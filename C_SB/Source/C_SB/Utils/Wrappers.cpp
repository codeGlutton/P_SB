#include "Utils/Wrappers.h"
#include "C_SB.h"

FRegexPatternWrapper& FRegexPatternWrapper::operator=(const FRegexPattern& Other)
{
	Pattern = Other;

	return *this;
}

FRegexPatternWrapper& FRegexPatternWrapper::operator=(FRegexPattern&& Other)
{
	Pattern = MoveTemp(Other);
	
	return *this;
}
