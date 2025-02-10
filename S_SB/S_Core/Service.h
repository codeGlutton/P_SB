#pragma once
#include <functional>
#include "NetAddress.h"
#include "IocpCore.h"
#include "Listener.h"

// 서비스할 대상
enum class ServiceType : uint8
{
	Server,
	Client
};

/***********************
		Service
***********************/

// 세션 생성 함수
using SessionFactory = std::function<SessionRef(void)>;

// iocp 세션 통신 서비스 API 클래스 
class Service : public std::enable_shared_from_this<Service>
{
public:
	Service(ServiceType type, NetAddress address, IocpCoreRef core, SessionFactory factory, int32 maxSeesionCount = 1);
	virtual ~Service();

	// 서비스 통신 시작 함수
	virtual bool		Start() abstract;
	bool				CanStart() { return _sessionFactory != nullptr; }

	virtual void		CloseService();
	// 지정 생성 함수 등록
	void				SetSessionFactory(SessionFactory factory) { _sessionFactory = factory; }

	// 지정 생성 함수를 통해 생성 후 반환 (ex 아직 Accept 받은 전용 소켓인 아닌 경우 사용)
	SessionRef			CreateSession();
	// 기존에 연결되어 있는 세션을 세트에 등록
	void				AddSession(SessionRef session);
	// 등록되어 있는 세션 세트에서 제거
	void				ReleaseSession(SessionRef session);
	int32				GetCurrentSessionCount() { return _sessionCount; }
	int32				GetMaxSessionCount() { return _maxSessionCount; }

public:
	ServiceType			GetServiceType() { return _type; }
	NetAddress			GetNetAddress() { return _netAddress; }
	IocpCoreRef&		GetIocpCore() { return _iocpCore; }

protected:
	USE_LOCK;

	ServiceType			_type;
	NetAddress			_netAddress = {};
	IocpCoreRef			_iocpCore;

	xSet<SessionRef>	_sessions;
	int32				_sessionCount = 0;
	int32				_maxSessionCount = 0;
	SessionFactory		_sessionFactory;
};

/***********************
	 ClientService
***********************/

// 테스트 용 c++ 클라이언트
class ClientService : public Service
{
public:
	ClientService(NetAddress targetAddress, IocpCoreRef core, SessionFactory factory, int32 maxSessionCount = 1);
	virtual ~ClientService() {}

	virtual bool		Start() override;
	void				Send(SendBufferRef sendBuffer) { WRITE_LOCK; if (!_sessions.empty()) { (*(_sessions.begin()))->Send(sendBuffer); } }
};

/***********************
	 ServerService
***********************/

class ServerService : public Service
{
public:
	ServerService(NetAddress address, IocpCoreRef core, SessionFactory factory, int32 maxSessionCount = 1);
	virtual ~ServerService() {}

	virtual bool		Start() override;
	virtual void		CloseService() override;

private:
	ListenerRef			_listener = nullptr;
};