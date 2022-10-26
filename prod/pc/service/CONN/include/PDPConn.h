/*========================PDPConn.h=========================================*
 *                                                                          *
 * All sources, binaries and HTML pages (C) copyright 2006 by Blue Jungle., *
 * Redwood City CA, Ownership remains with Blue Jungle Inc,                 * 
 * All rights reserved worldwide.                                           *
 *                                                                          * 
 * Note   : This file includes the declarations of the exported APIs of PDP *
 *          service CONN module.                                            *
 *==========================================================================*/

#ifndef __CE_PDPCONN_H
#define __CE_PDPCONN_H

#include <vector>
#include "CEsdk.h"

CEResult_t PDP_CECONN_Initialize (nlsocket sockid,
				  JavaVM *PDP_jvm,
				  jobject PDP_servStub,
				  unsigned long long *inScopeCEHandleWrapper,
				  std::vector<void *> &inputArgs,
				  std::vector<void *> &replyArgs);

CEResult_t PDP_CECONN_Close (std::vector<void *> &inputArgs,
			     std::vector<void *> &replyArgs);

void PDP_CECONN_FreeHandle(CEHandle handle);
#endif
