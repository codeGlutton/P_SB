#include "SBDataTable.h"
#include "C_SB.h"

const FSBTableRow* USBDataTable::FindTableRow(uint64 ObjectId)
{
    ObjectId /= 0x1000000000000ull;
    if (auto FindTable = _Tables.Find(ObjectId / 1000ull))
        return (*FindTable)->FindRow<FSBTableRow>(FName(FString::FromInt(ObjectId)), FString(""));
    return nullptr;
}
