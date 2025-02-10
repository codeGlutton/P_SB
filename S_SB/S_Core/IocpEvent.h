#pragma once

class Session;

enum class EventType : uint8
{
	Connect,
	Disconnect,
	Accept,
	PreRecv,
	Recv,
	Send
};

/***********************
	   IocpEvemt
************************/

// Iocp를 이용하는 객체의 송수신 이벤트 정보
class IocpEvent : public OVERLAPPED
{
public:
	IocpEvent(EventType type);

	// OVERLAPPED 구조체 데이터 초기화
	void					Init();

public:
	EventType				eventType;
	IocpObjectRef			owner;
};

/***********************
	  ConnectEvent
************************/

class ConnectEvent : public IocpEvent
{
public:
	ConnectEvent() : IocpEvent(EventType::Connect) { }
};

/***********************
	 DisconnectEvent
************************/

class DisconnectEvent : public IocpEvent
{
public:
	DisconnectEvent() : IocpEvent(EventType::Disconnect) { }
};

/***********************
	   AcceptEvent
************************/

class AcceptEvent : public IocpEvent
{
public:
	AcceptEvent() : IocpEvent(EventType::Accept) { }

public:
	// 임시로 AcceptEx에서 전용소켓이 될 대상 소켓 저장
	SessionRef				session = nullptr;
};

/***********************
	    RecvEvent
************************/

class RecvEvent : public IocpEvent
{
public:
	RecvEvent() : IocpEvent(EventType::Recv) { }
};

/***********************
	    SendEvent
************************/

class SendEvent : public IocpEvent
{
public:
	SendEvent() : IocpEvent(EventType::Send) { }

public:
	xVector<SendBufferRef>	sendBuffers;
};
