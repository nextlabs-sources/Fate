/*
 * Created on Apr 21, 2010
 *
 * All sources, binaries and HTML pages (C) copyright 2010 by NextLabs Inc.,
 * San Mateo CA, Ownership remains with NextLabs Inc, All rights reserved
 * worldwide.
 */

#define NLMODULE SEC
#define NLTRACELEVEL 3

#include "brain.h"
#include "cetype.h"
#include "marshal.h"
#include "PEPMan.h"

static const int timeout = 5 * 1000;  // 5 seconds

CEResult_t CESEC_MakeProcessTrusted(CEHandle handle, CEString password)
{
    try
    {
        std::vector<void *> args;
        nlchar reqIDStr[100];
        nlsprintf(reqIDStr, _countof(reqIDStr), _T("%lu+%lu"),
                  nlthread_selfID(),
                  PEPMAN_GetUniqueRequestID());
        
        nlstring reqId(reqIDStr);
        CEString reqIDArg = CEM_AllocateString(reqIDStr);

#if defined(WIN32) || defined(_WIN64)
        // See CEPrivate.cpp for the reason behind adding "CE" at the beginning of the password
        nlstring prefix(_T("CE"));
        prefix+=CEM_GetString(password);
        CEString paddedPassword = CEM_AllocateString(prefix.c_str());
#else
        CEString paddedPassword = CEM_AllocateString(CEM_GetString(password));
#endif
        CEint32 processID = GetCurrentProcessId();
        args.push_back(reqIDArg);
        args.push_back(&processID);
        args.push_back(paddedPassword);
        
        size_t reqLen;
        char *packed=Marshal_PackReqFunc(_T("CESEC_MakeProcessTrusted"), args, reqLen);
        
        if (packed == NULL) {
            CEM_FreeString(reqIDArg);
            return CE_RESULT_GENERAL_FAILED;
        }
        
        nlstring reqFunc(100, ' ');
        CEResult_t reqResult;
        std::vector<void *> reqOut;
        reqOut.reserve(RPC_MAX_NUM_ARGUMENTS);
        
        CEResult_t result = PEPMAN_RPCCall(reqId, packed, reqLen, handle,
                                           reqFunc, reqResult, reqOut,
                                           timeout);
     
        Marshal_PackFree(packed);

        Marshal_UnPackFree(_T("CESEC_MakeProcessTrusted"), reqOut, false);

        CEM_FreeString(reqIDArg);
    
        return result != CE_RESULT_SUCCESS ? result : reqResult;
    }
    catch (...)
    {
        return CE_RESULT_GENERAL_FAILED;
    }
}
