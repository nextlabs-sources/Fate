// All sources, binaries and HTML pages (C) copyright 2004 by Blue Jungle Inc., 
// Redwood City CA, Ownership remains with Blue Jungle Inc, 
// All rights reserved worldwide. 

/**
 * The RequestDispatcher waits on the request queue. When the queue has an entry and we 
 * can obtain a lock on the mutex, pop the queue and handle the request 
 * 
 * @version $Id:
 *          //depot/main/Destiny/main/src/etc/eclipse/destiny-code-templates.xml#2 $:
 * 
 */

#include "StdAfx.h"
#include "..\..\shared\src\globals.h"
#include "..\..\shared\src\ilog.h"
#include "..\..\shared\src\logfactory.h"
#include "..\..\ipcproxy\src\ipcconstants.h"
#include "ipcmessagechannel.h"
#include "iipcrequesthandler.h"
#include "ipcstub.h"
#include "requestdispatcher.h"

#define METHOD_OPEN_TAG _T("<method name=\"")

RequestDispatcher::RequestDispatcher(void)
{
    m_pRequestQueue = NULL;
    m_hQueueMutex = NULL;
    m_hQueueEvent = NULL;
    m_pRequestHandler = NULL;
    m_bStopped = true;

    m_pLog = LogFactory::GetLogger();
}

RequestDispatcher::~RequestDispatcher(void)
{
    delete m_pRequestHandler;
}

/**
 * Wait on the request queue. When the queue has an entry and we 
 * can obtain a lock on the mutex, pop the queue and handle the 
 * request by calling DispatchRequest()
 * 
 */
void RequestDispatcher::Run ()
{
    DWORD waitResult = 0;
    m_bStopped = false;
    while (!m_bStopped) 
    {
        while (::WaitForSingleObject (m_hQueueMutex, TIMEOUT) == WAIT_TIMEOUT)
        {
            if (m_bStopped)
            {
                break;
            }
        }
        while (!m_bStopped && m_pRequestQueue->empty()) 
        {
            // release, wait for event, lock: This code is emulating java's wait() method.
            ::ReleaseMutex (m_hQueueMutex);
            waitResult = ::WaitForSingleObject (m_hQueueEvent, TIMEOUT);
            if (waitResult == WAIT_TIMEOUT && m_bStopped)
            {
                break;
            }

            while (::WaitForSingleObject (m_hQueueMutex, TIMEOUT) == WAIT_TIMEOUT)
            {
                if (m_bStopped)
                {
                    break;
                }
            }
        }
        
        if (!m_bStopped)
        {
            if (m_pRequestQueue->size() == 0)
            {
                _tprintf (_T("\n\nWHAT??!?!?!?!\n\n"));
            }
            IPCMessageChannel* pMessageChannel = (IPCMessageChannel*) (*m_pRequestQueue->begin());
            m_pRequestQueue->pop_front();

            ::ReleaseMutex (m_hQueueMutex);

            DispatchRequest (pMessageChannel);
        }
    }
}

/**
 * Force request dispatcher to return from its loop
 */
void RequestDispatcher::Stop()
{
    m_bStopped = true;
}

/**
 * If this is a DISCONNECT request, close the channel
 * If this is an invocation request, parse the request to determine 
 * the method and input parameters, and call invoke on m_pRequestHandler
 * Convert the result params to the response string, write the string back 
 * to the shared memory location and set the send event to signal the caller
 * to continue
 * 
 * @param pMessageChannel
 *            pointer to IPCMessageChannel instance that corresponds to the 
 *            shared memory location containing the request.
 */
void RequestDispatcher::DispatchRequest (IPCMessageChannel* pMessageChannel)
{
    IPCREQUEST request;
    memcpy (&request, pMessageChannel->GetSharedMemory (), sizeof (IPCREQUEST));
    memset (pMessageChannel->GetSharedMemory (), 0, sizeof (IPCREQUEST));
    ((IPCREQUEST*) pMessageChannel->GetSharedMemory ())->ulSize = sizeof (IPCREQUEST);
    
    if (m_pLog->IsInfoEnabled ())
    {
        TCHAR log[CHANNEL_SIZE + 100];
        _stprintf (log, _T("Thread %d: IPC Request Received: %s\n"), ::GetCurrentThreadId (), request.methodName);
        m_pLog->Info (log);
    }

    if (_tcscmp (request.methodName, DISCONNECT) == 0)
    {
        // signal event before closing event handles.
        ::SetEvent (pMessageChannel->GetSendEvent());
        m_pStub->RemoveChannel (pMessageChannel->GetReceiveEvent ());
    }
    else
    {
        m_pRequestHandler->Invoke (request, (IPCREQUEST*) pMessageChannel->GetSharedMemory ());

        ::SetEvent (pMessageChannel->GetSendEvent());
    }
}
