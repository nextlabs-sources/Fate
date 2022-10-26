// All sources, binaries and HTML pages (C) copyright 2004 by Blue Jungle Inc., 
// Redwood City CA, Ownership remains with Blue Jungle Inc, 
// All rights reserved worldwide. 

/**
 * 
 * IPCStub test program
 * 
 * @author fuad
 * @version $Id:
 *          //depot/main/Destiny/main/src/etc/eclipse/destiny-code-templates.xml#2 $:
 *  
 */
#include "stdafx.h"
#include "..\..\shared\src\globals.h"
#include "..\..\ipcstub\src\ipcstub.h"
#include "..\..\ipcstub\src\iipcrequesthandler.h"
#include "StubTestRequestHandler.h"

DWORD WINAPI ThreadProc(LPVOID lpStub)
{
   ((IPCStub*) lpStub)->Run();

   ::ExitThread (0);
}

/**
 * @param argc
 *            not used
 * @param argv
 *            not used
 * 
 * Starts IPCStub and initializes with StubTestRequestHandler and waits for requests
 */
int _tmain(int argc, _TCHAR* argv[])
{
    IPCStub* pStub = new IPCStub ();

    IIPCRequestHandler* ppHandlerArray [10];

    for (int i = 0; i < 10; i++)
    {
        ppHandlerArray [i] = new StubTestRequestHandler ();
    }

    pStub->Init (ppHandlerArray, 10);

    HANDLE hThread = ::CreateThread (NULL, 0, ThreadProc, (LPVOID) pStub, 0, NULL);       

    TCHAR buf [100];
    _tscanf (_T("%s"), buf);

    _tprintf (_T("Stopping Stub...\n"));
    pStub->Stop();
    ::WaitForSingleObject (hThread, INFINITE);
    _tprintf (_T("Stub terminated successfully.\n"));

    return 0;
}

