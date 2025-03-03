#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

#include "Http.h"

#include "OnlineSubsystemGoogleEx.h"
#include "OnlineSubsystemGoogleExModule.h"

#undef ONLINE_LOG_PREFIX
#define ONLINE_LOG_PREFIX TEXT("GoogleEx: ")

#ifndef GOOGLE_EX_SUBSYSTEM
const FName GOOGLE_EX_SUBSYSTEM(TEXT("GOOGLEEX"));
#endif
