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
#include "brain.h"
#include "globals.h"
#include "ipcconstants.h"
#include "ipcmessagechannel.h"
#include "iipcrequesthandler.h"
#include "ipcstub.h"
#include "requestdispatcher.h"

#define METHOD_OPEN_TAG _T("<method name=\"")
#define SHOW_NOTIFICATION_STR L"ShowNotification"
#define ACTION_ALLOW_EMAIL L"AEMAIL"
#define ACTION_DENY_EMAIL L"DEMAIL"

RequestDispatcher::RequestDispatcher(void)
{
    m_pRequestQueue = NULL;
    m_hQueueMutex = NULL;
    m_hQueueEvent = NULL;
    m_pRequestHandler = NULL;
    m_bStopped = true;

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
    
    TRACE (1, _T("Thread %d: IPC Request Received: %s\n"), ::GetCurrentThreadId (), request.methodName);

    if (_tcscmp (request.methodName, DISCONNECT) == 0)
    {
        // signal event before closing event handles.
        ::SetEvent (pMessageChannel->GetSendEvent());
        m_pStub->RemoveChannel (pMessageChannel->GetReceiveEvent ());
    }
    else
    {
		//if popub is show for outlook enforce , ignore it modify be bear 2016.5.13
		//parm[0] time ,for example Fri May 13 12:25:03 CST 2016
		//parm[1] result+action , for example AOPEN DOPEN
		//parm[2] checked file path
		//parm[3] alert message content
		if(_wcsicmp (request.methodName, SHOW_NOTIFICATION_STR) == 0 )
		{
			if((_wcsicmp(request.params[1],ACTION_ALLOW_EMAIL)!=0) && (_wcsicmp(request.params[1],ACTION_DENY_EMAIL)!=0))
			{
				m_pRequestHandler->Invoke (request, (IPCREQUEST*) pMessageChannel->GetSharedMemory ());
			}
			else
			{
				TRACE (1, _T("This Popub is for Outlook Enforce , Ignore it"));
			}
		}
        ::SetEvent (pMessageChannel->GetSendEvent());
    }
}
