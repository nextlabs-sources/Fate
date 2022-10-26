/*========================PDPEval.h=========================================*
 *                                                                          *
 * All sources, binaries and HTML pages (C) copyright 2006 by Blue Jungle., *
 * Redwood City CA, Ownership remains with Blue Jungle Inc,                 * 
 * All rights reserved worldwide.                                           *
 *                                                                          * 
 * Note   : This file includes the declarations of the exported APIs of PDP *
 *          service EVAL module.                                            *
 *==========================================================================*/

#ifndef __CE_PDPEVAL_H
#define __CE_PDPEVAL_H

#include <vector>
#include "nlthread.h"
#include "CEsdk.h"

CEResult_t PDP_CEEVALUATE_CheckMetadata (jobject g_servStub,
					 jclass g_serverStubClass,
					 JavaVM *PDP_jvm,
					 std::vector<void *> &inputArgs,
					 nlsocket serverSfd);

CEResult_t PDP_CEEVALUATE_CheckMulti (jobject g_servStub,
                                      jclass g_serverStubClass,
                                      JavaVM *PDP_jvm,
                                      std::vector<void *> &inputArgs,
                                      nlsocket serverSfd);

#if defined (WIN32) || defined(_WIN64)
CEResult_t PDP_CEEVALUATE_CheckMetaFromKIF(jobject g_servStub,
					   jclass g_serverStubClass,
					   JavaVM *PDP_jvm,
					   NL_KIF_QUEUE_ITEM *kitem);
#endif
#endif
