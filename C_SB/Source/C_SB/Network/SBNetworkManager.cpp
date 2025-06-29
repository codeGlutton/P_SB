#include "SBNetworkManager.h"
#include "C_SB.h"

#include "PacketSession.h"
#include "ServerPacketHandler.h"

#include "SBGameModeBase.h"
#include "Engine/AssetManager.h"

#include "Component/SBMovementComponent.h"

#include "ServerHttpPacketHandler.h"

#include "Utils.h"
#include "ObjectUtils.h"
#include "ByteConverters.h"
#include <google/protobuf/util/time_util.h>

DEFINE_LOG_CATEGORY(LogNetManager);

/*************************
	 USBNetworkManager
*************************/

USBNetworkManager::USBNetworkManager() :
	_WaitingPktTypeFlags(ESBTcpPktTypeFlag::NONE)
{
	GameServerSession = MakeXShared<PacketSession>();
}

void USBNetworkManager::ConnectToGameServer(const Protocol::ServerInfo& ServerInfo)
{
	if (CanSendTcpPkt(ESBTcpPktTypeFlag::ENTER_FLOW) == false)
		return;

	FString SocketError;
	UE_LOG(LogNetManager, Log, TEXT("Connecting To Server.."));
	if (GameServerSession->Connect(ServerInfo, OUT SocketError) == true)
	{
		UE_LOG(LogNetManager, Log, TEXT("Connection Success"));

		// 수신 작업 실행
		GameServerSession->Run();

		if (GameServerSession->GetLoginPkt().token_value().empty() == false)
		{
			SEND_C_PACKET(GameServerSession, *GameServerSession->GetMutableLoginPkt());
		}
	}
	else
	{
		UE_LOG(LogNetManager, Warning, TEXT("Connection Failed. ESocketErrorCode: %s"), *SocketError);
		ErrorFromLoginPkt(TEXT("잘못된 연결"));
	}
}

void USBNetworkManager::DisconnectFromGameServer()
{
	if (GameServerSession == nullptr)
		return;

	UE_LOG(LogNetManager, Log, TEXT("Disconnecting to Server"));
	{
		Protocol::C_LEAVE_GAME LeavePkt;
		SEND_C_PACKET(GameServerSession, LeavePkt);
	}
}

void USBNetworkManager::HandleRecvPackets()
{
	if (GameServerSession == nullptr)
		return;

	GameServerSession->HandleRecvPackets();
}

void USBNetworkManager::CreateNewPlayer(const FSBPlayerSelectInfo& NewPlayerInfo)
{
	if (CanSendTcpPkt(ESBTcpPktTypeFlag::ENTER_FLOW) == false)
		return;

	if (NewPlayerInfo.Name.IsEmpty() == true)
	{
		UE_LOG(LogNetManager, Warning, TEXT("Creating player has error becase of empty name"));
		ErrorFromCreatePkt(TEXT("필수 정보 미입력"));
		return;
	}

	CostumeSettingConvertor Convertor(StaticCast<uint32>(NewPlayerInfo.TeamUniform), StaticCast<uint32>(NewPlayerInfo.TeamShoes), StaticCast<uint32>(NewPlayerInfo.TeamAccessories));

	Protocol::C_CREATE_PLAYER CreatePlayerPkt;
	Protocol::PlayerSelectInfo* SelectInfo = CreatePlayerPkt.mutable_new_player();
	SelectInfo->set_player_db_id(NewPlayerInfo.PlayerDbId);
	{
		std::string Ut8Name;
		Utils::UTF16To8(NewPlayerInfo.Name, OUT Ut8Name);
		SelectInfo->set_name(Ut8Name);
	}
	SelectInfo->set_costume_setting(Convertor.bytes);
	SelectInfo->set_rank_score(0);
	SelectInfo->mutable_created_time()->CopyFrom(google::protobuf::util::TimeUtil::GetCurrentTime());

	UE_LOG(LogNetManager, Log, TEXT("Creating new player.."));
	SEND_C_PACKET(GameServerSession, CreatePlayerPkt);
}

void USBNetworkManager::DeletePlayer(const int32& PlayerDbId)
{
	if (CanSendTcpPkt(ESBTcpPktTypeFlag::ENTER_FLOW) == false)
		return;

	// 해당 id의 Player 탐색
	auto IsContained = _OtherPlayerInfos.ContainsByPredicate([PlayerDbId](const Protocol::PlayerSelectInfo& PlayerInfo) {
		return (PlayerInfo.player_db_id() == PlayerDbId) && (PlayerInfo.name().empty() == false);
	});

	// 없는 Player거나, 이미 삭제된 캐릭터인 경우
	if (IsContained == false)
	{
		UE_LOG(LogNetManager, Warning, TEXT("Deleting player has error becase of invalid player's db id"));
		ErrorFromDeletePkt(TEXT("존재하지 않는 캐릭터"));
		return;
	}

	Protocol::C_DELETE_PLAYER DeletePlayerPkt;
	DeletePlayerPkt.set_player_db_id(PlayerDbId);

	UE_LOG(LogNetManager, Log, TEXT("Selecting %d player.."), PlayerDbId);
	SEND_C_PACKET(GameServerSession, DeletePlayerPkt);
}

void USBNetworkManager::SelectPlayer(const int32& PlayerDbId)
{
	if (CanSendTcpPkt(ESBTcpPktTypeFlag::ENTER_FLOW) == false)
		return;

	// 해당 id의 Player 탐색
	auto* TargetPlayer = _OtherPlayerInfos.FindByPredicate([PlayerDbId](const Protocol::PlayerSelectInfo& PlayerInfo) {
		return PlayerInfo.player_db_id() == PlayerDbId;
		});

	// 없는 Player거나, 이미 삭제된 캐릭터인 경우
	if (TargetPlayer == nullptr || TargetPlayer->name().empty() == true)
	{
		UE_LOG(LogNetManager, Warning, TEXT("Deleting player has error becase of invalid player's db id"));
		ErrorFromChangeMapPkt(TEXT("존재하지 않는 캐릭터"));
		return;
	}

	Protocol::C_ENTER_GAME EnterGamePkt;
	EnterGamePkt.set_player_db_id(PlayerDbId);
}

bool USBNetworkManager::HasVerifiedConnection()
{
	return GameServerSession->State >= PacketSession::EState::VERIFIED;
}

bool USBNetworkManager::IsWaitingSpecificPkt(const ESBTcpPktTypeFlag TcpPktTypeFlag)
{
	return EnumHasAllFlags(_WaitingPktTypeFlags, TcpPktTypeFlag);
}

void USBNetworkManager::GetCurrentServerInfo(FSBServerSelectInfo& CurrentServerInfo)
{
	CurrentServerInfo = FSBServerSelectInfo(GameServerSession->GetCurrentServerInfo());
}

void USBNetworkManager::GetPlayerInfos(TArray<FSBPlayerSelectInfo>& PlayerInfos)
{
	for (auto& player : _OtherPlayerInfos)
	{
		PlayerInfos.Add(FSBPlayerSelectInfo(player));
	}
}

void USBNetworkManager::HandleReadyToConnect(const Protocol::RES_LOGIN_ACCOUNT& HttpLoginPkt)
{
	if (GameServerSession == nullptr)
		return;

	GameServerSession->GetMutableLoginPkt()->set_account_id(HttpLoginPkt.account_id());
	GameServerSession->GetMutableLoginPkt()->set_token_value(HttpLoginPkt.token_value());
}

void USBNetworkManager::HandleLogin(const Protocol::S_LOGIN& LoginPkt)
{
	if (GameServerSession == nullptr)
		return;
	GameServerSession->State = PacketSession::EState::VERIFIED;
	UpdateSeletablePlayerInfos(LoginPkt.players());

	HandleTcpPkt(OnLoginPktRecved, ESBTcpPktTypeFlag::ENTER_FLOW);
}

void USBNetworkManager::HandleChangePlayer(const Protocol::S_CREATE_PLAYER& CreatePkt)
{
	UpdateSeletablePlayerInfos(CreatePkt.players());
	HandleTcpPkt(OnCreatePlayerPktRecved, ESBTcpPktTypeFlag::ENTER_FLOW);
}

void USBNetworkManager::HandleChangePlayer(const Protocol::S_DELETE_PLAYER& DeletePkt)
{
	UpdateSeletablePlayerInfos(DeletePkt.players());
	HandleTcpPkt(OnDeletePlayerPktRecved, ESBTcpPktTypeFlag::ENTER_FLOW);
}

void USBNetworkManager::HandlePingPong(const Protocol::S_PING& PingPkt)
{
	if (GameServerSession == nullptr)
		return;
	GameServerSession->Rtt = PingPkt.rtt();

	{
		Protocol::C_PING PingPongPkt;
		SEND_C_PACKET(GameServerSession, PingPongPkt);
	}
}

void USBNetworkManager::HandleSpawn(const Protocol::S_CHANGE_MAP& ChangeMapPkt)
{
	HandleSpawn(ChangeMapPkt.object_info(), true);
}

void USBNetworkManager::HandleSpawn(const Protocol::S_SPAWN& SpawnPkt)
{
	for (auto& Object : SpawnPkt.object_infos())
	{
		HandleSpawn(Object, false);
	}
}

void USBNetworkManager::HandleDespawn(const Protocol::S_DESPAWN& DespawnPkt)
{
	for (auto& ObjectId : DespawnPkt.object_id())
	{
		HandleDespawn(ObjectId);
	}
}

void USBNetworkManager::HandleMove(const Protocol::S_MOVE& MovePkt)
{
	if (GameServerSession == nullptr)
		return;

	ISBObject** FindObject = Objects.Find(MovePkt.object_id());
	if (FindObject == nullptr)
		return;
	if ((*FindObject)->IsMyPlayer())
		return;

	AActor* FindActor = Cast<AActor>(*FindObject);
	if (FindActor == nullptr)
		return;
	if (USBMovementComponent* MoveComp = FindActor->GetComponentByClass<USBMovementComponent>())
	{
		MoveComp->AddRecord(MovePkt.pos_info(), MovePkt.dest_info(), MovePkt.move_state());
	}
}

void USBNetworkManager::HandleLeave()
{
	GameServerSession->Disconnect();
	UKismetSystemLibrary::QuitGame(GetWorld(), nullptr, EQuitPreference::Quit, false);
}

void USBNetworkManager::ErrorFromLoginPkt(FString ErrStr, bool NeedToClearToken)
{
	GameServerSession->Disconnect();
	ErrorFromTcpPkt(ErrStr, OnLoginPktRecved, ESBTcpPktTypeFlag::ENTER_FLOW);
	if (NeedToClearToken)
	{
		GameServerSession->GetMutableLoginPkt()->Clear();
	}
}

void USBNetworkManager::ErrorFromCreatePkt(FString ErrStr)
{
	ErrorFromTcpPkt(ErrStr, OnCreatePlayerPktRecved, ESBTcpPktTypeFlag::ENTER_FLOW);
}

void USBNetworkManager::ErrorFromDeletePkt(FString ErrStr)
{
	ErrorFromTcpPkt(ErrStr, OnDeletePlayerPktRecved, ESBTcpPktTypeFlag::ENTER_FLOW);
}

void USBNetworkManager::ErrorFromChangeMapPkt(FString ErrStr)
{
	ErrorFromTcpPkt(ErrStr, OnChangeMapPktRecved, ESBTcpPktTypeFlag::ENTER_FLOW);
}

void USBNetworkManager::HandleSpawn(const Protocol::ObjectInfo& Info, const bool bIsMine, const bool bAsync)
{
	if (GameServerSession == nullptr)
		return;

	const uint64 ObjectId = Info.object_base_info().object_id();
	if (Objects.Find(ObjectId) != nullptr)
		return;

	TObjectPtr<UDataTable>* DataTable = GetWorld()->GetAuthGameMode<ASBGameModeBase>()->SpawnableTables.Find(StaticCast<ESBTableRowType>(Info.object_base_info().object_id() / 1000ull));
	if (DataTable == nullptr)
		return;
	auto* TableRow = (*DataTable)->FindRow<FSBSpawnableTableRow>(FName(FString::FromInt(Info.object_base_info().object_id())), FString());
	if (TableRow == nullptr || TableRow->ClientBasePath.IsNull() == true)
		return;

	if (bIsMine)
	{
		ASBPlayer* Player = Cast<ASBPlayer>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));
		if (Player == nullptr)
			return;

		MyPlayer = Player;
		MyPlayer->SetInfo(Info);
		MyPlayer->ApplyTableRow(*TableRow);
		Objects.Add(Info.object_base_info().object_id(), Player);
	}
	else
	{
		auto& AssetManager = UAssetManager::Get();
		TArray<FSoftObjectPath> LoadItems{ TableRow->ClientBasePath.ToSoftObjectPath(), TableRow->ClientMesh.ToSoftObjectPath(), TableRow->ClientMaterial.ToSoftObjectPath() };
		if (bAsync == true)
		{
			AssetManager.GetStreamableManager().RequestAsyncLoad(LoadItems, FStreamableDelegate::CreateLambda([this, Info, TableRow]()
				{
					ISBObject* NewSBObject = ObjectUtils::SpawnSBObject(GetWorld(), Info, *TableRow);
					Objects.Add(Info.object_base_info().object_id(), NewSBObject);
				}
			));
		}
		else
		{
			auto Handle = AssetManager.GetStreamableManager().RequestSyncLoad(LoadItems);
			{
				ISBObject* NewSBObject = ObjectUtils::SpawnSBObject(GetWorld(), Info, *TableRow);
				Objects.Add(Info.object_base_info().object_id(), NewSBObject);
			}
		}
	}
}

void USBNetworkManager::HandleDespawn(const uint64& ObjectId)
{
	if (GameServerSession == nullptr)
		return;

	auto* World = GetWorld();
	if (World == nullptr)
		return;

	ISBObject** FindObject = Objects.Find(ObjectId);
	if (FindObject == nullptr)
		return;
	AActor* FindActor = Cast<AActor>(*FindObject);
	if (FindActor == nullptr)
		return;

	World->DestroyActor(FindActor);
}

bool USBNetworkManager::CanSendTcpPkt(const ESBTcpPktTypeFlag PktType)
{
	if (EnumHasAllFlags(_WaitingPktTypeFlags, PktType) == true)
	{
		UE_LOG(LogNetManager, Warning, TEXT("New connection cannot be attempted while waiting for a server packet"));
		return false;
	}
	EnumAddFlags(_WaitingPktTypeFlags, PktType);
	OnChangeReqPktFlags.Broadcast();
	return true;
}

void USBNetworkManager::HandleTcpPkt(const FOnTcpPktRecved& InDelegate, const ESBTcpPktTypeFlag PktType)
{
	UE_LOG(LogNetManager, Log, TEXT("Tcp pkt is successed"));
	InDelegate.Broadcast(true, TEXT(""));
	EnumRemoveFlags(_WaitingPktTypeFlags, PktType);
	OnChangeReqPktFlags.Broadcast();
}

void USBNetworkManager::ErrorFromTcpPkt(const FString& ErrStr, const FOnTcpPktRecved& InDelegate, const ESBTcpPktTypeFlag PktType)
{
	UE_LOG(LogNetManager, Log, TEXT("Tcp pkt is failed by %s"), *ErrStr);
	InDelegate.Broadcast(false, ErrStr);
	EnumRemoveFlags(_WaitingPktTypeFlags, PktType);
	OnChangeReqPktFlags.Broadcast();
}

void USBNetworkManager::UpdateSeletablePlayerInfos(const google::protobuf::RepeatedPtrField<Protocol::PlayerSelectInfo>& PlayerInfos)
{
	_OtherPlayerInfos.Empty();
	for (auto& PlayerInfo : PlayerInfos)
	{
		_OtherPlayerInfos.Add(PlayerInfo);
	}
	_OtherPlayerInfos.Sort([](const Protocol::PlayerSelectInfo& Op1, const Protocol::PlayerSelectInfo& Op2) {
		if (Op1.name().empty() == Op2.name().empty())
		{
			return !Op1.name().empty() && Op1.created_time().seconds() < Op2.created_time().seconds();
		}
		return !Op1.name().empty();
	});
}
