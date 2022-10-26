/*========================PDPProtect.h======================================*
 *                                                                          *
 * All sources, binaries and HTML pages (C) copyright 2006 by Blue Jungle., *
 * Redwood City CA, Ownership remains with Blue Jungle Inc,                 * 
 * All rights reserved worldwide.                                           *
 *                                                                          * 
 * Note   : This file includes the declarations of the exported APIs of PDP *
 *          service CEProtect module.                                       *
 *==========================================================================*/

#ifndef __CE_PDPPROTECT_H
#define __CE_PDPPROTECT_H

#include <vector>
#include "CEsdk.h"

CEResult_t PDP_CEPROTECT_RegKeyGuard_Init(bool bDesktop);

void PDP_CEPROTECT_RegKeyGuard_Run();

void PDP_CEPROTECT_RegKeyGuard_Stop();

CEResult_t PDP_CEPROTECT_LockKey(std::vector<void *> &inputArgs,
				 std::vector<void *> &replyArgs);
CEResult_t PDP_CEPROTECT_UnlockKey(std::vector<void *> &inputArgs,
				 std::vector<void *> &replyArgs);
#endif
