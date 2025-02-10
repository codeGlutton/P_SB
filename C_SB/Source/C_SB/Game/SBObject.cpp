#include "SBObject.h"
#include "C_SB.h"
#include "SBMyPlayer.h"

bool ISBObject::IsMyPlayer()
{
	return Cast<ASBMyPlayer>(this) != nullptr;
}

