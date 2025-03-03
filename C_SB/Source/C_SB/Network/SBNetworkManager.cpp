#include "SBNetworkManager.h"
#include "C_SB.h"

#include "PacketSession.h"
#include "ServerPacketHandler.h"

#include "SBDataTable.h"
#include "Engine/AssetManager.h"

#include "Component/SBMovementComponent.h"

#include "ServerHttpPacketHandler.h"

#include "Utils.h"
#include "ByteConverters.h"

DEFINE_LOG_CATEGORY(LogNetManager);

/*************************
	FSBServerSelectInfo
*************************/

FSBServerSelectInfo::FSBServerSelectInfo(const Protocol::ServerSelectInfo& ServerSeclectInfo) : Dencity(ServerSeclectInfo.density())
{
	Utils::UTF8To16(ServerSeclectInfo.name(), Name);
}

FSBServerSelectInfo::FSBServerSelectInfo(Protocol::ServerSelectInfo&& ServerSeclectInfo) : Dencity(ServerSeclectInfo.density())
{
	Utils::UTF8To16(ServerSeclectInfo.name(), Name);
}

/*************************
	FSBPlayerSelectInfo
*************************/

FSBPlayerSelectInfo::FSBPlayerSelectInfo(const FString& Name, const int32& CostumeSetting)
{
	CostumeSettingConvertor Converter(CostumeSetting);
	ManagerCostume = Converter.manager;
	TeamUniform = Converter.uniform;
	TeamShoes = Converter.shoes;
	TeamAccessories = Converter.accessories;
}

FSBPlayerSelectInfo::FSBPlayerSelectInfo(FString&& Name, int32&& CostumeSetting)
{
	CostumeSettingConvertor Converter(MoveTemp(CostumeSetting));
	ManagerCostume = Converter.manager;
	TeamUniform = Converter.uniform;
	TeamShoes = Converter.shoes;
	TeamAccessories = Converter.accessories;
}

FSBPlayerSelectInfo::FSBPlayerSelectInfo(const Protocol::PlayerSelectInfo& PlayerSeclectInfo)
{
	Utils::UTF8To16(PlayerSeclectInfo.name(), Name);
	CostumeSettingConvertor Converter(PlayerSeclectInfo.costume_setting());
	ManagerCostume = Converter.manager;
	TeamUniform = Converter.uniform;
	TeamShoes = Converter.shoes;
	TeamAccessories = Converter.accessories;
}

FSBPlayerSelectInfo::FSBPlayerSelectInfo(Protocol::PlayerSelectInfo&& PlayerSeclectInfo)
{
	Utils::UTF8To16(PlayerSeclectInfo.name(), Name);
	CostumeSettingConvertor Converter(PlayerSeclectInfo.costume_setting());
	ManagerCostume = Converter.manager;
	TeamUniform = Converter.uniform;
	TeamShoes = Converter.shoes;
	TeamAccessories = Converter.accessories;
}

/*************************
	 USBNetworkManager
*************************/

USBNetworkManager::USBNetworkManager() :
	_bWaitToConnect(false)
{
	GameServerSession = MakeXShared<PacketSession>();
}

void USBNetworkManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	_LoginPkt = xnew<Protocol::C_LOGIN>();
}

void USBNetworkManager::Deinitialize()
{
	Super::Deinitialize();

	xdelete(_LoginPkt);
	_LoginPkt = nullptr;
}

void USBNetworkManager::ConnectToGameServer(int32 ServerIndex)
{
	if (_bWaitToConnect == true)
	{
		UE_LOG(LogNetManager, Warning, TEXT("New connection cannot be attempted while waiting for a server packet"));
		return;
	}
	_bWaitToConnect = true;

	check(_ServerInfos.Num() > ServerIndex);

	FString SocketError;
	UE_LOG(LogNetManager, Log, TEXT("Connecting To Server"));
	if (GameServerSession->Connect(_ServerInfos[ServerIndex], OUT SocketError))
	{
		UE_LOG(LogNetManager, Log, TEXT("Connection Success"));
		_ServerInfos.Empty();

		// 수신 작업 실행
		GameServerSession->Run();

		if (_LoginPkt != nullptr)
		{
			SEND_C_PACKET(GameServerSession, *_LoginPkt);
		}
	}
	else
	{
		UE_LOG(LogNetManager, Warning, TEXT("Connection Failed. ESocketErrorCode: %s"), *SocketError);
		_bWaitToConnect = false;
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

bool USBNetworkManager::HasVerifiedConnection()
{
	return GameServerSession->State >= PacketSession::EState::VERIFIED;
}

bool USBNetworkManager::IsWaitingToConnect()
{
	return _bWaitToConnect;
}

void USBNetworkManager::GetCurrentServerInfo(FSBServerSelectInfo& CurrentServerInfo)
{
	CurrentServerInfo = FSBServerSelectInfo(GameServerSession->GetCurrentServerInfo());
}

void USBNetworkManager::GetServerInfos(TArray<FSBServerSelectInfo>& ServerInfos)
{
	for (auto& server : _ServerInfos)
	{
		ServerInfos.Add(FSBServerSelectInfo(server));
	}
}

void USBNetworkManager::GetPlayerInfos(TArray<FSBPlayerSelectInfo>& PlayerInfos)
{
	for (auto& player : _PlayerInfos)
	{
		PlayerInfos.Add(FSBPlayerSelectInfo(player));
	}
}

void USBNetworkManager::HandleReadyToConnect(const Protocol::RES_LOGIN_ACCOUNT& HttpLoginPkt)
{
	if (GameServerSession == nullptr)
		return;

	_LoginPkt->set_account_id(HttpLoginPkt.account_id());
	_LoginPkt->set_token_value(HttpLoginPkt.token_value());

	UpdateServerInfos(HttpLoginPkt.server_list());
	OnServerListResRecved.Broadcast();
}

void USBNetworkManager::HandleLogin(const Protocol::S_LOGIN& LoginPkt)
{
	switch (LoginPkt.result())
	{
	case Protocol::LOGIN_RESULT_SUCCESS:
	{
		GameServerSession->State = PacketSession::EState::VERIFIED;
		UpdatePlayerInfos(LoginPkt.players());
		OnLoginPktRecved.Broadcast(true, TEXT(""));
		break;
	}
	case Protocol::LOGIN_RESULT_ERROR_FULL_SERVER:
	{
		GameServerSession->Disconnect();
		OnLoginPktRecved.Broadcast(false, TEXT("서버 인원 초과"));
		break;
	}
	case Protocol::LOGIN_RESULT_ERROR_INVALID_TOKEN:
	{
		GameServerSession->Disconnect();
		OnLoginPktRecved.Broadcast(false, TEXT("잘못된 토큰"));
		_LoginPkt->Clear();
		break;
	}
	case Protocol::LOGIN_RESULT_ERROR_ACCOUNT_EXIST:
	{
		GameServerSession->Disconnect();
		OnLoginPktRecved.Broadcast(false, TEXT("중복 로그인"));
		_LoginPkt->Clear();
		break;
	}
	}

	_bWaitToConnect = false;
}

void USBNetworkManager::HandleRefreshServer(const Protocol::RES_RECHECK_SERVER& HttpCheckPkt)
{
	if (GameServerSession == nullptr)
		return;

	UpdateServerInfos(HttpCheckPkt.server_list());
	OnServerListResRecved.Broadcast();
}

void USBNetworkManager::HandlePingPong(const Protocol::S_PING& PingPkt)
{
	GameServerSession->Rtt = PingPkt.rtt();

	{
		Protocol::C_PING PingPongPkt;
		SEND_C_PACKET(GameServerSession, PingPongPkt);
	}
}

void USBNetworkManager::HandleSpawn(const Protocol::ObjectInfo& Info, const bool bIsMine, const bool bAsync)
{
	if (GameServerSession == nullptr)
		return;

	const uint64 ObjectId = Info.object_base_info().object_id();
	if (Objects.Find(ObjectId) != nullptr)
		return;

	if (bIsMine)
	{
		ASBPlayer* Player = Cast<ASBPlayer>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));
		if (Player == nullptr)
			return;

		MyPlayer = Player;
		MyPlayer->SetInfo(Info);
		Objects.Add(Info.object_base_info().object_id(), Player);
	}
	else
	{
		USBDataTable* DataTable = GetGameInstance()->GetSubsystem<USBDataTable>();
		const FSBTableRow* FindRow = DataTable->FindTableRow(Info.object_base_info().object_id());
		if (FindRow == nullptr)
			return;
		auto& AssetManager = UAssetManager::Get();
		if (bAsync == false || AssetManager.GetStreamableManager().IsAsyncLoadComplete(FindRow->ClientBasePath.ToSoftObjectPath()))
		{
			UClass* ClassType = FindRow->ClientBasePath.LoadSynchronous();
			SpawnObjectDeferred(Info, ClassType);
			return;
		}
		else
		{
			AssetManager.GetStreamableManager().RequestAsyncLoad(FindRow->ClientBasePath.ToSoftObjectPath(), FStreamableDelegate::CreateLambda([Info, FindRow, this]()
				{
					UClass* ClassType = FindRow->ClientBasePath.Get();
					SpawnObjectDeferred(Info, ClassType);
				}
			));
		}
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

void USBNetworkManager::GetLoginInfo(OUT int32& AccountId, OUT std::string& TokenValue)
{
	AccountId = _LoginPkt->account_id();
	TokenValue = _LoginPkt->token_value();
}

void USBNetworkManager::SpawnObjectDeferred(const Protocol::ObjectInfo& Info, UClass* ObjectClass)
{
	auto* World = GetWorld();
	if (World == nullptr)
		return;

	FTransform SpawnTransform(FRotator(0., Info.pos_info().yaw(), 0.), FVector(Info.pos_info().x(), Info.pos_info().y(), 0.));

	// 초기 정보 적용

	ISBObject* Object = World->SpawnActorDeferred<ISBObject>(ObjectClass, SpawnTransform, nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
	if (Object == nullptr)
		return;
	Object->SetInfo(Info);

	// 초기 움직임 적용

	AActor* Actor = Cast<AActor>(Object);
	if (USBMovementComponent* SBMoveComp = Actor->FindComponentByClass<USBMovementComponent>())
	{
		SBMoveComp->AddInitMoveRecord(Info.pos_info(), Info.move_state());
	}
	Actor->FinishSpawning(SpawnTransform, true);

	Objects.Add(Info.object_base_info().object_id(), Object);

	return;
}

void USBNetworkManager::UpdateServerInfos(const google::protobuf::RepeatedPtrField<Protocol::ServerSelectInfo>& ServerInfos)
{
	_ServerInfos.Empty();
	for (auto& ServerInfo : ServerInfos)
	{
		_ServerInfos.Add(ServerInfo);
	}
}

void USBNetworkManager::UpdatePlayerInfos(const google::protobuf::RepeatedPtrField<Protocol::PlayerSelectInfo>& PlayerInfos)
{
	_PlayerInfos.Empty();
	for (auto& PlayerInfo : PlayerInfos)
	{
		_PlayerInfos.Add(PlayerInfo);
	}
}
