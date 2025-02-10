#pragma once

#include "CoreMinimal.h"
#include "Types.h"

#include "Protocol.pb.h"
#include "SBPlayer.h"

#include "Kismet/GameplayStatics.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "SBNetworkManager.generated.h"

UCLASS()
class C_SB_API USBNetworkManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	virtual void					Deinitialize() override;
	
	/**
	 * TCP 게임 서버로 Connect 패킷 전송
	 */
	UFUNCTION(BlueprintCallable)
	void							ConnectionToGameServer();
	
	/**
	 * TCP 게임 서버로 Disconnect 패킷 전송
	 */
	UFUNCTION(BlueprintCallable)
	void							DisconnectFromGameServer();
	
	/**
	 * TCP 게임 서버에서 받은 패킷 큐를 모두 실행
	 */
	UFUNCTION(BlueprintCallable)
	void							HandleRecvPackets();

public:
	void							HandleSpawn(const Protocol::ObjectInfo& Info, const bool bIsMine, const bool bAsync = true);

	void							HandleSpawn(const Protocol::S_CHANGE_MAP& ChangeMapPkt);
	void							HandleSpawn(const Protocol::S_SPAWN& SpawnPkt);

	void							HandleDespawn(const uint64& ObjectId);
	void							HandleDespawn(const Protocol::S_DESPAWN& DespawnPkt);

	void							HandleMove(const Protocol::S_MOVE& MovePkt);

protected:
	void							SpawnObjectDeferred(const Protocol::ObjectInfo& Info, UClass* ObjectClass);

public:
	class FSocket*					Socket;
	FString							IpAddress = TEXT("127.0.0.1");
	int16							Port = 7777;
	PacketSessionRef				GameServerSession;
	float							Rtt;

public:
	ASBPlayer*						MyPlayer;
	TMap<uint64, ISBObject*>		Objects;

	uint64							NextMapId; // TODO = 로비 기본값
};