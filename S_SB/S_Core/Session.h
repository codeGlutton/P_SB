#pragma once
#include "IocpCore.h"
#include "IocpEvent.h"
#include "NetAddress.h"
#include "RecvBuffer.h"

class Service;

/********************
	   Session
********************/

// 소켓 Send/Recv 역할 Iocp 오브젝트
class Session : public IocpObject
{
	friend class Listener;
	friend class IocpCore;
	friend class Service;

	enum
	{
		BUFFER_SIZE = 0x10000
	};

public:
	Session();
	virtual ~Session();

public:
	/* 외부 API */

	void						Send(SendBufferRef sendBuffer);
	// 서버끼리의 연동에 주로 사용
	bool						Connect();
	void						Disconnect(const WCHAR* cause);

	std::shared_ptr<Service>	GetService() { return _service.lock(); }
	void						SetService(std::shared_ptr<Service> service) { _service = service; }

public:
	/* 정보 데이터 */

	void						SetNetAddress(NetAddress address) { _netAddress = address; }
	NetAddress					GetNetAddress() { return _netAddress; }
	SOCKET						GetSocket() { return _socket; }
	bool						IsConnected() { return _connected; }
	SessionRef					GetSessionRef() { return std::static_pointer_cast<Session>(shared_from_this()); }

private:
	/* 인테페이스 구현 */

	virtual HANDLE				GetHandle() override;
	virtual void				Dispatch(class IocpEvent* iocpEvent, int32 numOfBytes = 0) override;

private:
	/* 전송 구현 */

	bool						RegisterConnect();
	bool						RegisterDisconnect();
	void						RegisterRecv();
	void						RegisterSend();

	void						ProcessConnect();
	void						ProcessDisconnect();
	void						ProcessRecv(int32 numOfBytes);
	void						ProcessSend(int32 numOfBytes);

	void						HandleError(int32 errorCode);

protected:
	/* 컨텐츠 코드에서 오버로딩(연결 타이밍에 실행요소) */
	
	virtual void				OnConnected() { }
	virtual int32				OnRecv(BYTE* buffer, int32 len) { return len; }
	virtual void				OnSend(int32 len) { }
	virtual void				OnDisconnected() { }

private:
	std::weak_ptr<Service>		_service;
	SOCKET						_socket = INVALID_SOCKET;
	NetAddress					_netAddress = {};
	Atomic<bool>				_connected = false;

private:
	USE_LOCK;

	/* 수신 관련 */

	RecvBuffer					_recvBuffer;
	
	/* 송신 관련 */
	
	// 다음번 전송 대기 버퍼들
	xQueue<SendBufferRef>		_sendQueue;
	// 현 세션에서 전송 진행 여부 (타 스레드의 동기를 위해 atomic)
	Atomic<bool>				_sendRegistered = false;

private:
	/* IocpEvent 재사용 */

	ConnectEvent				_connectEvent;
	DisconnectEvent				_disconnectEvent;
	RecvEvent					_recvEvent;
	// WSASend에 버퍼들을 몰아서 전송해주기 위해 재사용
	SendEvent					_sendEvent;
};

/********************
	PacketSession
********************/

struct PacketHeader
{
	// 헤더 포함 총 데이터 사이즈
	uint16 size;
	// 프로토콜 종류 (ex 로그인, 이동요청)
	uint16 id;
};

class PacketSession : public Session
{
public: 
	PacketSession();
	virtual ~PacketSession();

	PacketSessionRef			GetPacketSessionRef() { return std::static_pointer_cast<PacketSession>(shared_from_this()); }

protected:
	// 헤더 체크 후 0개 이상 패킷 조립 처리
	virtual int32				OnRecv(BYTE* buffer, int32 len) final;
	virtual void				OnRecvPacket(BYTE* buffer, int32 len) = 0;
};