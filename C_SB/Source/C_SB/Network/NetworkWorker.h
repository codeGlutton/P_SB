#pragma once

#include "CoreMinimal.h"
#include "Types.h"
#include "HAL/Runnable.h"
#include "HAL/RunnableThread.h"

DECLARE_LOG_CATEGORY_EXTERN(LogNetWorker, Log, All);

class FSocket;
class SendBuffer;

/* 네트워크 송수신 대기를 위한 전용 스레드 객체 */

/************************
		RecvWorker
*************************/

class C_SB_API RecvWorker : public FRunnable
{
public:
	RecvWorker(FSocket* Socket, PacketSessionRef Session);
	~RecvWorker();

public:
	virtual bool				Init() override;
	virtual uint32				Run() override;
	virtual void				Exit() override;

	void						Destroy();
	FRunnableThread* const		GetThread() { return _Thread; }

private:
	bool						ReceivePacket(TArray<uint8>& OutPacket);
	bool						ReceiveDesiredBytes(uint8* Results, int32 Size, OUT int32& RecvSize);

protected:
	FRunnableThread*			_Thread = nullptr;
	bool						_bRunning = true;
	FSocket*					_Socket;
	TWeakPtr<PacketSession>		_SessionRef;
};

/************************
		SendWorker
*************************/

class C_SB_API SendWorker : public FRunnable
{
public:
	SendWorker(FSocket* Socket, PacketSessionRef Session);
	~SendWorker();

	virtual bool				Init() override;
	virtual uint32				Run() override;
	virtual void				Exit() override;

	bool						SendPacket(SendBufferRef SendBuffer);

	void						Destroy();
	FRunnableThread* const		GetThread() { return _Thread; }

private:
	bool						SendDesiredBytes(const uint8* Buffer, int32 Size);

protected:
	FRunnableThread*			_Thread = nullptr;
	bool						_bRunning = true;
	FSocket*					_Socket;
	TWeakPtr<PacketSession>		_SessionRef;
};