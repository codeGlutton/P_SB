#pragma once

/***********************
	   IocpObject
************************/

// Iocp를 이용하는 객체 본연의 데이터
class IocpObject : public std::enable_shared_from_this<IocpObject>
{
public:
	virtual HANDLE	GetHandle() abstract;
	virtual void	Dispatch(class IocpEvent* iocpEvent, int32 numOfBytes = 0) abstract;
};

/***********************
		IocpCore
************************/

class IocpCore
{
public:
	IocpCore();
	~IocpCore();

	HANDLE			GetHandle() { return _iocpHandle; }

	bool			Register(IocpObjectRef iocpObject);
	// 완료 포트 검사
	bool			Dispatch(uint32 timeoutMs = INFINITE);

private:
	HANDLE			_iocpHandle;
};