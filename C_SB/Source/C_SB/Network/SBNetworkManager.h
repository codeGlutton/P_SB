#pragma once

#include "CoreMinimal.h"
#include "Types.h"

#include "Protocol.pb.h"
#include "HttpProtocol.pb.h"
#include "SBPlayer.h"

#include "Kismet/GameplayStatics.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "SBNetworkManager.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogNetManager, Log, All);

/*************************
	FSBServerSelectInfo
*************************/

/**
 * 서버 선택창에 표기될 일부 서버 정보 구조체
 */
USTRUCT(BlueprintType)
struct C_SB_API FSBServerSelectInfo
{
	GENERATED_USTRUCT_BODY()
public:
	FSBServerSelectInfo() : Name(), Dencity(0.f) {}
	FSBServerSelectInfo(const FString& Name,const float& Dencity) : Name(Name), Dencity(Dencity) {}
	FSBServerSelectInfo(FString&& Name,float&& Dencity) : Name(MoveTemp(Name)), Dencity(MoveTemp(Dencity)) {}
	FSBServerSelectInfo(const Protocol::ServerSelectInfo& ServerSeclectInfo);
	FSBServerSelectInfo(Protocol::ServerSelectInfo&& ServerSeclectInfo);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	FString Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	float Dencity;
};

/*************************
	FSBPlayerSelectInfo
*************************/

/**
 * 캐릭터 선택창에 표기될 일부 캐릭터 정보 구조체
 */
USTRUCT(BlueprintType)
struct C_SB_API FSBPlayerSelectInfo
{
	GENERATED_USTRUCT_BODY()
public:
	FSBPlayerSelectInfo() : Name(), ManagerCostume(0), TeamUniform(0), TeamShoes(0), TeamAccessories(0) {}
	FSBPlayerSelectInfo(const FString& Name, const uint8& ManagerCostume, const uint8& TeamUniform, const uint8& TeamShoes, const uint8& TeamAccessories) : 
		Name(Name), 
		ManagerCostume(ManagerCostume), 
		TeamUniform(TeamUniform),
		TeamShoes(TeamShoes),
		TeamAccessories(TeamAccessories)
	{
	}
	FSBPlayerSelectInfo(FString&& Name, uint8&& ManagerCostume, uint8&& TeamUniform, uint8&& TeamShoes, uint8&& TeamAccessories) : 
		Name(MoveTemp(Name)),
		ManagerCostume(MoveTemp(ManagerCostume)),
		TeamUniform(MoveTemp(TeamUniform)),
		TeamShoes(MoveTemp(TeamShoes)),
		TeamAccessories(MoveTemp(TeamAccessories))
	{
	}
	FSBPlayerSelectInfo(const FString& Name, const int32& CostumeSetting);
	FSBPlayerSelectInfo(FString&& Name, int32&& CostumeSetting);
	FSBPlayerSelectInfo(const Protocol::PlayerSelectInfo& PlayerSeclectInfo);
	FSBPlayerSelectInfo(Protocol::PlayerSelectInfo&& PlayerSeclectInfo);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	FString Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	uint8 ManagerCostume;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	uint8 TeamUniform;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	uint8 TeamShoes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	uint8 TeamAccessories;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnServerListResRecved);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLoginPktRecved, bool, bIsSuccess, FString, ErrStr);

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
	virtual void						Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void						Deinitialize() override;
	
public:
	/**
	 * TCP 게임 서버로 Connect 패킷 전송
	 */
	UFUNCTION(BlueprintCallable)
	void								ConnectToGameServer(int32 ServerIndex);
	
	/**
	 * TCP 게임 서버로 Disconnect 패킷 전송
	 */
	UFUNCTION(BlueprintCallable)
	void								DisconnectFromGameServer();
	
	/**
	 * TCP 게임 서버에서 받은 패킷 큐를 모두 실행
	 */
	UFUNCTION(BlueprintCallable)
	void								HandleRecvPackets();
	
public:
	/**
	 * TCP 게임 서버와 연결 및 허가 여부 확인
	 * @return 연결 및 허가 여부
	 */
	UFUNCTION(BlueprintCallable)
	bool								HasVerifiedConnection();

	/**
	 * TCP 게임 서버와 연결을 대기중인지 확인
	 * @return 연결 대기 여부
	 */
	UFUNCTION(BlueprintCallable)
	bool								IsWaitingToConnect();

	/**
	 * 현재 서버의 이름과 복잡도가 담긴 간단한 정보 추출
	 * @param CurrentServerInfo 현재 서버의 간단한 정보
	 */
	UFUNCTION(BlueprintCallable)
	void								GetCurrentServerInfo(FSBServerSelectInfo& CurrentServerInfo);

	/**
	 * 접속가능한 모든 서버의 이름과 복잡도가 담긴 간단한 정보들 추출
	 * @param ServerInfos 접속가능한 서버들의 간단한 정보 배열
	 */
	UFUNCTION(BlueprintCallable)
	void								GetServerInfos(TArray<FSBServerSelectInfo>& ServerInfos);

	/**
	 * 사용가능한 모든 Player 이름과 개별 세팅이 담긴 간단한 정보들 추출
	 * @param PlayerInfos 사용가능한 Player들의 간단한 정보 배열
	 */
	UFUNCTION(BlueprintCallable)
	void								GetPlayerInfos(TArray<FSBPlayerSelectInfo>& PlayerInfos);

public:
	void								HandleReadyToConnect(const Protocol::RES_LOGIN_ACCOUNT& HttpLoginPkt);

	void								HandleLogin(const Protocol::S_LOGIN& LoginPkt);
	void								HandleRefreshServer(const Protocol::RES_RECHECK_SERVER& HttpCheckPkt);

	void								HandlePingPong(const Protocol::S_PING& PingPkt);

	void								HandleSpawn(const Protocol::ObjectInfo& Info, const bool bIsMine, const bool bAsync = true);
	void								HandleSpawn(const Protocol::S_CHANGE_MAP& ChangeMapPkt);
	void								HandleSpawn(const Protocol::S_SPAWN& SpawnPkt);

	void								HandleDespawn(const uint64& ObjectId);
	void								HandleDespawn(const Protocol::S_DESPAWN& DespawnPkt);

	void								HandleMove(const Protocol::S_MOVE& MovePkt);

	void								HandleLeave();

	void								GetLoginInfo(OUT int32& AccountId, OUT std::string& TokenValue);

protected:
	void								SpawnObjectDeferred(const Protocol::ObjectInfo& Info, UClass* ObjectClass);
	
	void								UpdateServerInfos(const google::protobuf::RepeatedPtrField<Protocol::ServerSelectInfo>& ServerInfos);
	void								UpdatePlayerInfos(const google::protobuf::RepeatedPtrField<Protocol::PlayerSelectInfo>& PlayerInfos);

public:
	PacketSessionRef					GameServerSession;

public:
	ASBPlayer*							MyPlayer;
	TMap<uint64, ISBObject*>			Objects;

public:
	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnServerListResRecved				OnServerListResRecved;

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnLoginPktRecved					OnLoginPktRecved;

private:
	Protocol::C_LOGIN*					_LoginPkt;

	bool								_bWaitToConnect;

	// 임시적으로 보관되는 접속가능한 서버 정보 배열
	TArray<Protocol::ServerSelectInfo>	_ServerInfos;

	// 임시적으로 보관되는 선택가능한 Player 정보 배열
	TArray<Protocol::PlayerSelectInfo>	_PlayerInfos;
};