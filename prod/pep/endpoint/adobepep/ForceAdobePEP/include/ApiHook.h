#pragma once

#include <windows.h>
#include <winsock2.h>
#include <mapix.h>
#pragma warning(push)
#pragma warning(disable:4819)
#include "madCHook_helper.h"
#pragma warning(pop)


#include "eframework/policy/comm_helper.hpp"
#include "eframework/auto_disable/auto_disable.hpp"


//Hook Apis
void Hook ( );

//Unhook Apis
void Unhook ( );

//Hook Apis
void HookForOtherProcess ( );

//Unhook Apis
void UnhookForOtherProcess ( );