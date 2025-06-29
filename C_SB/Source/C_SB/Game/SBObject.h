#pragma once

#include "C_SB.h"
#include "SBTableRow.h"
#include "UObject/Interface.h"
#include "SBObject.generated.h"

UINTERFACE(MinimalAPI)
class USBObject : public UInterface
{
	GENERATED_BODY()
};

class C_SB_API ISBObject
{
	GENERATED_BODY()

public:
	virtual void							GetInfo(OUT Protocol::ObjectInfo& OutInfo) = 0;
	virtual void							SetInfo(const Protocol::ObjectInfo& Info) = 0;
	virtual void							ApplyPos(const Protocol::PosInfo& InPos) = 0;
	virtual void							ApplyTableRow(const FSBSpawnableTableRow& TableRow) = 0;

	virtual const Protocol::ObjectBaseInfo* GetObjectBaseInfo() = 0;
	virtual const Protocol::PosInfo*		GetPosInfo() = 0;

	bool									IsMyPlayer();
};
