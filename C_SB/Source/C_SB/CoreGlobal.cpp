#include "CoreGlobal.h"
#include "C_SB.h"

Memory*				GMemory = nullptr;
SendBufferManager*	GSendBufferManager = nullptr;

class CoreGlobal
{
public:
	CoreGlobal()
	{
		GMemory = new Memory();
		GSendBufferManager = new SendBufferManager();
	}

	~CoreGlobal()
	{
		delete GMemory;
		delete GSendBufferManager;
	}
} GCoreGlocal;
