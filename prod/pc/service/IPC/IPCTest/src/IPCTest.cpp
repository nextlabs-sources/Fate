// All sources, binaries and HTML pages (C) copyright 2004 by Blue Jungle Inc., 
// Redwood City CA, Ownership remains with Blue Jungle Inc, 
// All rights reserved worldwide. 

/**
 * 
 * IPCProxy test program
 * 
 * @see #_tmain() for usage instructions
 * 
 * @author fuad
 * @version $Id:
 *          //depot/main/Destiny/main/src/etc/eclipse/destiny-code-templates.xml#2 $:
 *  
 */

#include "stdafx.h"
#include "..\..\shared\src\globals.h"
#include "..\..\IPCProxy\src\IPCProxy.h"
#include "..\..\..\..\..\common\include\dsipc.h"


#define IPC_TEST_REQUEST_HANDLER _T("com.bluejungle.destiny.agent.ipc.tests.TestRequestHandler")
#define IPC_TEST_REQUEST_HANDLER_CPP _T("StubTestRequestHandler")
#define NUM_ITERATIONS 100
#define NUM_THREADS 63
#define TEST_TIMEOUT 200000

DWORD WINAPI ThreadProc(LPVOID pHandlerName)
{
    IPCProxy* pProxy = new IPCProxy();
    pProxy->Init((TCHAR*) pHandlerName);

    StringVector inputParams;
    TCHAR* s1 = _T("param1");
    TCHAR* s2 = _T("param2");
    inputParams.push_back(s1);
    inputParams.push_back(s2);

    for (int i = 0; i < NUM_ITERATIONS; i++)
    {
        IPCREQUEST request, response;
        memset (&request, 0, sizeof (IPCREQUEST));
        request.ulSize = sizeof (IPCREQUEST);
        _tcsncpy_s (request.methodName, _countof(request.methodName), _T("method1"), _TRUNCATE); 
        _tcsncpy_s (request.params[0], _countof(request.params[0]), _T("param1"), _TRUNCATE); 
        _tcsncpy_s (request.params[1], _countof(request.params[1]), _T("param2"), _TRUNCATE); 

        pProxy->Invoke(&request, &response);

        // result params are expected to be retval1 and retval2
        if (_tcscmp (response.params[0], _T("retval1")) != 0 ||
            _tcscmp (response.params[1], _T("retval2")) != 0 )
        {
            delete (pProxy);
            pProxy = new IPCProxy();
            pProxy->Init((TCHAR*) pHandlerName);
            _tprintf (_T("ERROR: C++ IPC Proxy Test Failed"));
        }
        else
        {
            _tprintf (_T("Thread %d: Invocation successful.\n"), ::GetCurrentThreadId());
        }
    }

    delete (pProxy);
    ::ExitThread (0);
}

/**
 * This program assumes that a Stub is running already an waiting for requests.
 * For java stub, see com.bluejungle.destiny.agent.ipc.tests.IPCRequestHandlerTest
 * For C++ stub, see project IPCStubTest
 * 
 * The main function creates NUM_THREADS threads and call ThreadProc for each. 
 * ThreadProc creates an instance of IPCProxy and makes NUM_ITERATIONS calls 
 * on the proxy before returning. It also checks that the response of the invocation
 * is as expected.
 * 
 * @param argc
 *            number of args. should be 2
 * @param argv
 *            argv[1] specifies whether we are running the java or the cpp proxy.
 *            should be java to run against the java proxy using 
 *            com.bluejungle.destiny.agent.ipc.tests.TestRequestHandler as the handler class
 *            otherwise we will run against the c++ proxy using 
 *            StubTestRequestHandler as the handler class
 * 
 */
int _tmain(int argc, _TCHAR* argv[])
{
    HANDLE threadArray [NUM_THREADS];
    if (argc != 2)
    {
        _tprintf (_T("Usage: ipctest [java|cpp]\n\n"));
        exit (1);
    }

    TCHAR* pHandlerName = NULL;
    if (_tcscmp (argv[1], _T("java")) == 0)
    {
        pHandlerName = IPC_TEST_REQUEST_HANDLER;
    }
    else
    {
        pHandlerName = IPC_TEST_REQUEST_HANDLER_CPP;
    }

    for (int i=0; i < NUM_THREADS; i++)
    {
        threadArray[i] = ::CreateThread (NULL, 0, ThreadProc, (LPVOID) pHandlerName, 0, NULL);
    }

    if (::WaitForMultipleObjects (NUM_THREADS, threadArray, TRUE, TEST_TIMEOUT) == WAIT_TIMEOUT)
    {
        _tprintf (_T("ERROR: Timeout"));
        return (1); //error
    }
    else
    {
        return 0;
    }
}

