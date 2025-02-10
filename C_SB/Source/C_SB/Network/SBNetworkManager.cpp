#include "SBNetworkManager.h"
#include "C_SB.h"
#include "Sockets.h"

#include "Common/TcpSocketBuilder.h"
#include "Serialization/ArrayWriter.h"
#include "SocketSubsystem.h"

#include "PacketSession.h"
#include "ServerPacketHandler.h"

#include "SBDataTable.h"
#include "Engine/AssetManager.h"

#include "Component/SBMovementComponent.h"

#include "ServerHttpPacketHandler.h"
#include "Utils.h"

void USBNetworkManager::Deinitialize()
{
	Super::Deinitialize();
	
	if (GameServerSession)
	{
		GameServerSession->Owner = nullptr;
	}
}

void USBNetworkManager::ConnectionToGameServer()
{
	Socket = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateSocket(TEXT("Stream"), TEXT("Client Socket"));

	FIPv4Address Ip;
	FIPv4Address::Parse(IpAddress, Ip);

	TSharedRef<FInternetAddr> InternetAddr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
	InternetAddr->SetIp(Ip.Value);
	InternetAddr->SetPort(Port);

	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Connecting To Server...")));

	bool bConnected = Socket->Connect(*InternetAddr);

	if (bConnected)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Connection Success")));

		// 수신 작업 실행
		GameServerSession = MakeXShared<PacketSession>(this, Socket);
		GameServerSession->OnConnected();
		GameServerSession->Run();
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Connection Failed")));
	}
	
}

void USBNetworkManager::DisconnectFromGameServer()
{
	if (Socket == nullptr || GameServerSession == nullptr)
		return;

	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Disconnecting to Server...")));

	Protocol::C_LEAVE_GAME LeavePkt;
	SEND_C_PACKET(GameServerSession, LeavePkt);

	/*if (Socket)
	{
		ISocketSubsystem* socketSubsystem = ISocketSubsystem::Get();
		socketSubsystem->DestroySocket(Socket);
		Socket = nullptr;
	}*/
}

void USBNetworkManager::HandleRecvPackets()
{
	if (Socket == nullptr || GameServerSession == nullptr)
		return;

	GameServerSession->HandleRecvPackets();
}

void USBNetworkManager::HandleSpawn(const Protocol::ObjectInfo& Info, const bool bIsMine, const bool bAsync)
{
	if (Socket == nullptr || GameServerSession == nullptr)
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
	if (Socket == nullptr || GameServerSession == nullptr)
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
	if (Socket == nullptr || GameServerSession == nullptr)
		return;

	auto* World = GetWorld();
	if (World == nullptr)
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
