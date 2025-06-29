#pragma once

#include "CoreMinimal.h"
#include "CoreNetwork.h"
#include "Types.h"
#include "Wrappers.h"

#include "SBTableRow.h"

#include "Protocol.pb.h"
#include "HttpProtocol.pb.h"
#include "SBPlayer.h"

#include "Kismet/GameplayStatics.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "SBNetworkManager.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogNetManager, Log, All);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTcpPktRecved, bool, bIsSuccess, FString, ErrStr);

/**
 * Tcp 패킷이 어떤 카테고리의 정보를 다루는지
 */
UENUM(BlueprintType, meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class ESBTcpPktTypeFlag : uint8
{
	NONE = 0 UMETA(DisplayName = "None"),

	ENTER_FLOW = 1 << 0 UMETA(DisplayName = "Enter flow")
};

 /*************************
	  USBNetworkManager
 *************************/

UCLASS()
class C_SB_API USBNetworkManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	USBNetworkManager();
	
public:
	/* BP로 웹 서버에 요청 보내기 */

	void								ConnectToGameServer(const Protocol::ServerInfo& ServerInfo);
	void								DisconnectFromGameServer();

	/**
	 * TCP 게임 서버에서 받은 패킷 큐를 모두 실행
	 */
	UFUNCTION(BlueprintCallable)
	void								HandleRecvPackets();

	/**
	 * 새로운 플레이어 생성 Tcp 패킷 전송
	 * @param NewPlayerInfo 새 플레이어 정보
	 */
	UFUNCTION(BlueprintCallable)
	void								CreateNewPlayer(const FSBPlayerSelectInfo& NewPlayerInfo);

	/**
	 * 기존 플레이어 삭제 Tcp 패킷 전송
	 * @param PlayerDbId 삭제할 플레이어 id
	 */
	UFUNCTION(BlueprintCallable)
	void								DeletePlayer(const int32& PlayerDbId);

	/**
	 * 기존 플레이어 선택 Tcp 패킷 전송
	 * @param PlayerDbId 선택할 플레이어 id
	 */
	UFUNCTION(BlueprintCallable)
	void								SelectPlayer(const int32& PlayerDbId);
	
public:
	/* 상태 출력 함수 */

	/**
	 * TCP 게임 서버와 연결 및 허가 여부 확인
	 * @return 연결 및 허가 여부
	 */
	UFUNCTION(BlueprintCallable)
	bool								HasVerifiedConnection();
	/**
	 * 어떤 응답 패킷 종류들을 대기중인지
	 * @return 응답 패킷 Flags
	 */
	UFUNCTION(BlueprintCallable)
	ESBTcpPktTypeFlag					GetWaitingTcpTypeFlags() { return _WaitingPktTypeFlags; }
	/**
	 * 특정 패킷 종류가 대기중인지
	 * @param TcpPktTypeFlag 원하는 패킷 종류
	 * @return 응답 중인지 여부
	 */
	UFUNCTION(BlueprintCallable)
	bool								IsWaitingSpecificPkt(const ESBTcpPktTypeFlag TcpPktTypeFlag);
	/**
	 * 현재 서버의 이름과 복잡도가 담긴 간단한 정보 추출
	 * @param CurrentServerInfo 현재 서버의 간단한 정보
	 */
	UFUNCTION(BlueprintCallable)
	void								GetCurrentServerInfo(FSBServerSelectInfo& CurrentServerInfo);
	/**
	 * 사용가능한 모든 Player 이름과 개별 세팅이 담긴 간단한 정보들 추출
	 * @param PlayerInfos 사용가능한 Player들의 간단한 정보 배열
	 */
	UFUNCTION(BlueprintCallable)
	void								GetPlayerInfos(TArray<FSBPlayerSelectInfo>& PlayerInfos);

public:
	/* 성공 시 응답 처리 */

	void								HandleReadyToConnect(const Protocol::RES_LOGIN_ACCOUNT& HttpLoginPkt);
	void								HandleLogin(const Protocol::S_LOGIN& LoginPkt);
	void								HandleChangePlayer(const Protocol::S_CREATE_PLAYER& CreatePkt);
	void								HandleChangePlayer(const Protocol::S_DELETE_PLAYER& DeletePkt);
	void								HandlePingPong(const Protocol::S_PING& PingPkt);
	void								HandleSpawn(const Protocol::S_CHANGE_MAP& ChangeMapPkt);
	void								HandleSpawn(const Protocol::S_SPAWN& SpawnPkt);
	void								HandleDespawn(const Protocol::S_DESPAWN& DespawnPkt);
	void								HandleMove(const Protocol::S_MOVE& MovePkt);
	void								HandleLeave();

	/* 실패 시 응답 처리 */

	void								ErrorFromLoginPkt(FString ErrStr, bool NeedToClearToken = false);
	void								ErrorFromCreatePkt(FString ErrStr);
	void								ErrorFromDeletePkt(FString ErrStr);
	void								ErrorFromChangeMapPkt(FString ErrStr);

private:
	void								HandleSpawn(const Protocol::ObjectInfo& Info, const bool bIsMine, const bool bAsync = true);
	void								HandleDespawn(const uint64& ObjectId);

	bool								CanSendTcpPkt(const ESBTcpPktTypeFlag PktType);
	void								HandleTcpPkt(const FOnTcpPktRecved& InDelegate, const ESBTcpPktTypeFlag PktType);
	void								ErrorFromTcpPkt(const FString& ErrStr, const FOnTcpPktRecved& InDelegate, const ESBTcpPktTypeFlag PktType);

	void								UpdateSeletablePlayerInfos(const google::protobuf::RepeatedPtrField<Protocol::PlayerSelectInfo>& PlayerInfos);

public:
	/* 블루프린트 Notify */

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnTcpPktRecved						OnLoginPktRecved;
	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnTcpPktRecved						OnCreatePlayerPktRecved;
	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnTcpPktRecved						OnDeletePlayerPktRecved;
	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnTcpPktRecved						OnChangeMapPktRecved;

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnChangeReqPktFlags				OnChangeReqPktFlags;

public:
	PacketSessionRef					GameServerSession;

	ASBPlayer*							MyPlayer;
	TMap<uint64, ISBObject*>			Objects;

private:
	// 응답 대기 중인 Tcp 패킷 타입들
	// (같은 카테고리 패킷에 대해서 다중 요청이 불가)
	ESBTcpPktTypeFlag					_WaitingPktTypeFlags;

	// 임시적으로 보관되는 선택가능한 Player 정보 배열
	TArray<Protocol::PlayerSelectInfo>	_OtherPlayerInfos;
};