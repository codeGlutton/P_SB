#pragma once
#include "NetAddress.h"

// 소켓 작업을 위한 유틸성 정적 헬퍼 클래스
class SocketUtils
{
public:
	// 연결을 위한 함수 포인터들

	static LPFN_CONNECTEX		ConnectEx;
	static LPFN_DISCONNECTEX	DisconnectEx;
	static LPFN_ACCEPTEX		AcceptEx;

public:
	static void					Init();
	static void					Clear();

	static bool					BindWindowFunction(SOCKET socket, GUID guid, LPVOID* fn);
	static SOCKET				CreateSocket();

	// 종료 명령 이후 타임 아웃 사용여부
	static bool					SetLinger(SOCKET socket, uint16 onoff, uint16 linger);
	// (종류 후 재실행에서 발생) 같은 주소의 연결 소켓 접근 허용 여부
	static bool					SetReuseAddress(SOCKET socket, uint16 flag);
	static bool					SetRecvBufferSize(SOCKET socket, int32 size);
	static bool					SetSendBufferSize(SOCKET socket, int32 size);
	// 짧은 소켓의 전송을 방지하는 네이글 알고리즘 미 사용 여부
	static bool					SetTcpNoDelay(SOCKET socket, bool flag);
	// listen Socket 옵션을 그대로 복붙
	static bool					SetUpdateAcceptSocket(SOCKET socket, SOCKET listenSocket);

	static bool					Bind(SOCKET socket, NetAddress netAddr);
	static bool					BindAnyAddress(SOCKET socket, uint16 port);
	static bool					Listen(SOCKET socket, int32 backlog = SOMAXCONN);
	static void					Close(SOCKET& socket);
};

template<typename T>
static inline bool SetSocketOpt(SOCKET socket, int32 level, int32 optName, T optVal)
{
	return SOCKET_ERROR != ::setsockopt(socket, level, optName, reinterpret_cast<char*>(&optVal), sizeof(T));
}

