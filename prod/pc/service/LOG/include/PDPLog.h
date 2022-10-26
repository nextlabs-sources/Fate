/*=========================PDPLog.h=========================================*
 *                                                                          *
 * All sources, binaries and HTML pages (C) copyright 2008 by NextLabs, Inc.*
 * San Mateo CA, Ownership remains with NextLabs Inc,                       * 
 * All rights reserved worldwide.                                           *
 *                                                                          * 
 * Note   : This file includes the declarations of the exported APIs of PDP *
 *          service LOG module.                                             *
 *==========================================================================*/

#ifndef __CE_PDPLOG_H
#define __CE_PDPLOG_H

#include <vector>
#include "CEsdk.h"

CEResult_t PDP_CELOG_LogDecision(JavaVM *PDP_jvm,
				 jobject cmStub,
				 jclass cmStubClass,
				 std::vector<void *> &inputArgs,
				 std::vector<void *> &outArgs);

CEResult_t PDP_CELOG_LogAssistantData(JavaVM *PDP_jvm,
				      jobject cmStub,
				      jclass cmStubClass,
				      std::vector<void *> &inputArgs,
				      std::vector<void *> &outArgs);
#endif
