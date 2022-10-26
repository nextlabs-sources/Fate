/*========================PDPEval.h=========================================*
 *                                                                          *
 * All sources, binaries and HTML pages (C) copyright 2006 by Blue Jungle., *
 * Redwood City CA, Ownership remains with Blue Jungle Inc,                 * 
 * All rights reserved worldwide.                                           *
 *                                                                          * 
 * Note   : This file includes the declarations of the exported APIs of PDP *
 *          service EVAL module.                                            *
 *==========================================================================*/

#ifndef __CE_PDPPRIVATE_H
#define __CE_PDPPRIVATE_H

#include <vector>
#include "CEsdk.h"

CEResult_t PDP_CEP_StopPDP(JavaVM *PDP_jvm,
			   jobject cmStub,
			   jclass cmStubClass,
			   std::vector<void *> &inputArgs,
			   std::vector<void *> &outArgs);

#endif
