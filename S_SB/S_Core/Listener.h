#pragma once
#include "IocpCore.h"
#include "NetAddress.h"

class AcceptEvent;
class ServerService;

// 소켓 AcceptEx 역할 Iocp 오브젝트
class Listener : public IocpObject
{
public:
	Listener() = default;
	~Listener();

public:
	/* 외부에서 사용 */

	// 내부 주소 등록
	bool					StartAccept(ServerServiceRef service);
	void					CloseSocket();

public:
	/* 인터페이스 구현 */

	virtual HANDLE			GetHandle() override;
	virtual void			Dispatch(class IocpEvent* iocpEvent, int32 numOfBytes = 0) override;

private:
	/* 수신 구현 */

	void					RegisterAccept(AcceptEvent* acceptEvent);
	void					ProcessAccept(AcceptEvent* acceptEvent);

protected:
	SOCKET					_socket = INVALID_SOCKET;
	xVector<AcceptEvent*>	_acceptEvents;
	ServerServiceRef		_service;
};

