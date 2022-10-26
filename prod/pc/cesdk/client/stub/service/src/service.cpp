/*==========================conn.cpp========================================*
 *                                                                          *
 * All sources, binaries and HTML pages (C) copyright 2010 by Nextlabs,     *
 * San Mateo CA, Ownership remains with NextLabs Inc,                       * 
 * All rights reserved worldwide.                                           *
 *                                                                          * 
 * Author : Alan Morgan                                                     *
 * Date   : 1/13/2010                                                       *
 * Note   : Implementations of SDK Service APIs.                            *
 *==========================================================================*/

#include <iostream>
#include "brain.h"
#include "pepman.h"
#include "cetype.h"
#include "marshal.h"
#include "nlconfig.hpp"

#if defined(WIN32)
#include "eframework/timer/timer_high_resolution.hpp"
#endif

#include "celog.h"
#include "celog_policy_windbg.hpp"

#include <vector>

static CELog serviceLog;

static void initializeLog() {
    static bool initialized = false;
    if (!initialized) {
        serviceLog.SetLevel(CELOG_INFO);
#if defined(WIN32) || defined(_WIN64)
        serviceLog.SetPolicy(new CELogPolicy_WinDbg());
#endif
        if (NLConfig::IsDebugMode()) {
            serviceLog.SetLevel(CELOG_DEBUG);
        }
        initialized=true;
    }
    return;
}

static int countArgs(const nlchar *fmt)
{
    int count = 0;
    while (*fmt) {
        switch (*fmt) {
            case '[':
                fmt++;
                if (*fmt == 'a') {
                    count+=2;
                }
            default:
                count++;
        }
        fmt++;
    }

    return count;
}

static CEResult_t CheckInputs(CEHandle h, CEString serviceName, CEString cefmt)
{
    if (h == NULL) {
        return CE_RESULT_NULL_CEHANDLE;
    }

    // Service name must be a valid, non-empty string
    if (serviceName == NULL || CEM_GetString(serviceName) == NULL || nlstrlen(CEM_GetString(serviceName)) == 0) {
        return CE_RESULT_INVALID_PARAMS;
    }

    // Nothing wrong with an empty format string
    if (cefmt == NULL || CEM_GetString(cefmt) == NULL) {
        return CE_RESULT_INVALID_PARAMS;
    }

    return CE_RESULT_SUCCESS;
}

CEResult_t ServiceInvoke(CEHandle h, CEString serviceName, CEString cefmt, void **request, void ***response, CEint32 timeout) {

#ifdef WIN32
    nextlabs::high_resolution_timer ht;
#endif
    initializeLog();

    CEResult_t check = CheckInputs(h, serviceName, cefmt);

    if (check != CE_RESULT_SUCCESS) {
        return check;
    }
                       
    // Build the data
    size_t requestLen;
    std::vector<void *> requestVec;

    const nlchar *fmt = CEM_GetString(cefmt);
    int numArgs = countArgs(fmt);

    // We modify the format string that we are given because we want to add three new arguments.
    // The first is an internal reqID, the second is an indicator that this is a service call
    // as opposed to something else that might be using the generic API, the third is the service
    // name that we are given.
    // Modifying the argument seems better than special casing the marshalling code.

    nlchar *fmtEx = new nlchar[nlstrlen(fmt) + 4];
    nlstrcpy_s(fmtEx, nlstrlen(fmt) + 4, L"sss");
    nlstrcat_s(fmtEx, nlstrlen(fmt) + 4, fmt);

    // The reqId
    nlchar reqIDStr[100];
    nlsprintf(reqIDStr, 100, _T("%lu+%lu"), nlthread_selfID(), PEPMAN_GetUniqueRequestID());
    CEString reqIDArg = CEM_AllocateString(reqIDStr);
    nlstring reqID(reqIDStr);
    requestVec.push_back(reqIDArg);

    // I am a service
    CEString iAmAService = CEM_AllocateString(L"SERVICEINVOKE");
    requestVec.push_back(iAmAService);

    // The service name
    requestVec.push_back(serviceName);

    // Everything else
    for (int i = 0; i < numArgs; i++) {
        requestVec.push_back(request[i]);
    }

    char *marshaledRequest = Marshal_PackReqGeneric(fmtEx, requestVec, requestLen);
    delete[] fmtEx;
    if (marshaledRequest == NULL) {
	CEM_FreeString(reqIDArg);
	CEM_FreeString(iAmAService);

        return CE_RESULT_INVALID_PARAMS;
    }

    // Send off
    nlstring reqFunc(100u,' ');
    CEResult_t reqResult;
    vector<void *>reqOut;
    // Very nasty.  Service calls can return a lot of data, but we
    // can't use the auto-resizing feature of C++ vectors because the
    // resizing will be done in a different dll and Windows doesn't
    // share heaps between dlls.  A solution would be to move to a
    // dynamically linked crt, but that is a work in progress.  Right
    // now we need to allocate "enough".  When we have a dynamic CRT
    // all the "reserve"s can be removed.
    reqOut.reserve(1024);
    CEResult_t res = PEPMAN_RPCCall(reqID, marshaledRequest, requestLen, h,
                                    reqFunc, reqResult, reqOut,
                                    timeout);
    Marshal_PackFree(marshaledRequest);
    if (res != CE_RESULT_SUCCESS) {
	CEM_FreeString(reqIDArg);
	CEM_FreeString(iAmAService);

        return res;
    }

    // 0: format
    // 1: reqId
    // 2..n: rest of data
    // We don't want the reqId.  We do want the format, but it includes the reqId, so we have to
    // adjust it
    // MSVC /analyze doesn't like us calling reqOut.size() here and below, and complains about a possible
    // buffer overrun. I have no idea why this shuts it up
    int reqOutSize = reqOut.size();
    *response = new void *[reqOutSize-1];

    CEString fmtWithReqId = (CEString)reqOut[0];
    // Use Marshal_AllocateCEString() instead of CE_AllocateString() because I want to create this memory buffer in cemarshal.dll
    // so that it can be freed by Marshal_FreeGeneric() later
    CEString fmtWithoutReqId = Marshal_AllocateCEString(CEM_GetString(fmtWithReqId)+1);
    // Use Marshal_FreeCEString() instead of CE_FreeString() because these memory buffers have been created in cemarshal.dll
    Marshal_FreeCEString(fmtWithReqId); // free the format string with reqId
    Marshal_FreeCEString((CEString)reqOut[1]); // free reqId

    (*response)[0] = fmtWithoutReqId;
    for (size_t i = 2; i < reqOutSize; i++)
    {
        (*response)[i-1] = reqOut[i];
    }
    
#ifdef WIN32
    ht.stop();
    serviceLog.Log(CELOG_DEBUG, _T("ServiceInvoke: reqId=%s service=%s, time=%f\n"),
                   reqIDArg->buf,
                   serviceName->buf,
                   ht.diff());
#endif

    // We can't delete the response vector here because it and response still share data
    CEM_FreeString(reqIDArg);
    CEM_FreeString(iAmAService);

    return reqResult;
}

CEResult_t ServiceResponseFree(void **response) {
    if (response == NULL) {
        return CE_RESULT_SUCCESS;
    }

    std::vector<void *> vresp;

    // The response data starts with a format string describing the rest of the data
    CEString fmt = (CEString)(*response);

    // The void** isn't null terminated or anything, so we need to use the format string to
    // get the length
    int numargs = countArgs(CEM_GetString(fmt));

    // Marshal_FreeGeneric wants a vector.  Note <= numargs because the actual arguments start
    // at 1, not 0
    for (int i = 0; i <= numargs; ++i)
    {
        vresp.push_back(response[i]);
    }

    CEResult_t res = Marshal_FreeGeneric(vresp);

    delete response;
    return res;
    
}
