// All sources, binaries and HTML pages (C) copyright 2004 by Blue Jungle Inc., 
// Redwood City CA, Ownership remains with Blue Jungle Inc, 
// All rights reserved worldwide. 

/**
* The IPC Stub is the component that runs on the callee (or Server) component
* and is responsible for receiving incoming requests, dispatching them to the
* callee and then returning the result to the caller.
* 
* After instantiation, the user should call init and pass in an array of 
* request handler instances and the size of the threadpool. The number of 
* request handler instances in the array should be equal to the threadpool
* size
* 
* The request handler class is a custom class that implements the behavior of
* the IPC Stub. Method invocations are dispatched to an instance of the request
* handler class which is a subclass of IIPCRequestHandler.
* 
* For an example of usage, see the IPCStubTest project.
* 
* @version $Id:
*          //depot/main/Destiny/main/src/etc/eclipse/destiny-code-templates.xml#2 $:
*  
*/

#include "StdAfx.h"
#include "globals.h"
#include "securityattributesfactory.h"
#include "ipcconstants.h"
#include "iipcrequesthandler.h"
#include "requestdispatcher.h"
#include "ipcmessagechannel.h"
#include "ipcstub.h"

#include "brain.h"

DWORD WINAPI ThreadProc(LPVOID lpDispatcher)
{
    ((RequestDispatcher*) lpDispatcher)->Run();

    ::ExitThread (0);
}

struct StubThreadInfo
{
    IPCStub* m_pStub;
    intptr_t m_bucketNumber;
};

DWORD WINAPI StubThreadProc(LPVOID lpThreadInfo)
{
    StubThreadInfo* pInfo = (StubThreadInfo*) lpThreadInfo;
    pInfo->m_pStub->Run(pInfo->m_bucketNumber);
    delete (pInfo);
    ::ExitThread (0);
    //Doly- This is unreachable code
    //return (0);
}


IPCStub::IPCStub(void)
{
    m_hSharedFileMapping = NULL;
    m_pSharedMemory = NULL;
    m_hSendEvent = NULL;
    m_hReceiveEvent = NULL;
    m_hHandshakeMutex = NULL;
    m_hEventListMutex = 0;
    m_threadPoolSize = 0;
    m_hQueueMutex = 0;
    m_hQueueEvent = 0;
    m_bStopped = true;

}

IPCStub::~IPCStub(void)
{
}

/**
* Initialize the IPC Stub by creating the shared memory location, events
* and mutex that are used for the handshake and also creating threads and
* request handler instance as specified by the parameters.
* 
* @param pRequestHandlerArray
*            Array of request handler instances that will be used to handle requests 
*            The array should be allocated and deallocated by the caller.
*            The contained request handler instances must be allocated by the caller but
*            must not be deallocated. The objects will be deallocated when IPCStub 
*            is stopped. 
*            The array size should be equal to the threadPoolSize
* @param threadPoolSize
*            number of request handler threads to create
* @return true if init is successful, false otherwise
*/
bool IPCStub::Init(IIPCRequestHandler** pRequestHandlerArray, int threadPoolSize) 
{
    if (threadPoolSize < 0 || threadPoolSize > MAXIMUM_WAIT_OBJECTS) {
        return false;
    }

    //get name from request handler instance
    tstring requestHandlerClassName = pRequestHandlerArray[0]->GetName ();

    m_serverSharedMemoryFileName = GLOBAL_NAME_PREFIX;
    m_serverSharedMemoryFileName += requestHandlerClassName;
    m_serverSharedMemoryFileName += SHARED_MEMORY_SUFFIX;

    tstring sendEventName = GLOBAL_NAME_PREFIX;
    sendEventName += requestHandlerClassName;
    sendEventName += SEND_EVENT_SUFFIX;
    tstring receiveEventName = GLOBAL_NAME_PREFIX;
    receiveEventName += requestHandlerClassName;
    receiveEventName += RECEIVE_EVENT_SUFFIX;
    tstring mutexName = GLOBAL_NAME_PREFIX;
    mutexName += requestHandlerClassName;
    mutexName += MUTEX_SUFFIX;

    m_hSendEvent = ::CreateEvent (SecurityAttributesFactory::GetSecurityAttributes(), FALSE, FALSE, sendEventName.c_str());
    if (!m_hSendEvent) 
    {
        m_hSendEvent = ::OpenEvent(SYNCHRONIZE | EVENT_MODIFY_STATE, FALSE, sendEventName.c_str());
        if (m_hSendEvent)
        {
            ::ResetEvent (m_hSendEvent);
        } 
        else
        {
            //FAIL
        }
    }
    m_hReceiveEvent = ::CreateEvent (SecurityAttributesFactory::GetSecurityAttributes(), FALSE, FALSE, receiveEventName.c_str());
    if (!m_hReceiveEvent) 
    {
        m_hReceiveEvent = ::OpenEvent(SYNCHRONIZE | EVENT_MODIFY_STATE, FALSE, receiveEventName.c_str());
        if (m_hReceiveEvent)
        {
            ::ResetEvent (m_hReceiveEvent);
        } 
        else
        {
            //FAIL
        }
    }
    m_hHandshakeMutex = ::CreateMutex (SecurityAttributesFactory::GetSecurityAttributes(), FALSE, mutexName.c_str());
    if (!m_hHandshakeMutex) 
    {
        m_hHandshakeMutex = ::OpenMutex(SYNCHRONIZE | MUTEX_MODIFY_STATE, FALSE, mutexName.c_str());
    }

    //Create first bucket.
    PtrVector* pEventList = new PtrVector ();
    m_eventBucketList.push_back (pEventList);
    pEventList->push_back (m_hReceiveEvent);

    m_threadPoolSize = threadPoolSize;
    m_threadArray = new HANDLE [(size_t) threadPoolSize];
    m_requestDispatcherArray = new RequestDispatcher* [(size_t) threadPoolSize];

    m_hQueueMutex = ::CreateMutex (NULL, FALSE, NULL);
    m_hQueueEvent = ::CreateEvent (NULL, FALSE, FALSE, NULL);
    m_hEventListMutex = ::CreateMutex (NULL, FALSE, NULL);

    for (int i=0; i < m_threadPoolSize; i++)
    {
        RequestDispatcher* pDispatcher = new RequestDispatcher ();
        m_requestDispatcherArray[i] = pDispatcher;
        pDispatcher->SetIPCStub (this);
        pDispatcher->SetQueueMutex (m_hQueueMutex);
        pDispatcher->SetQueueEvent (m_hQueueEvent);
        pDispatcher->SetRequestQueue (&m_requestQueue);
        pDispatcher->SetRequestHandler (pRequestHandlerArray [i]);

        m_threadArray[i] = ::CreateThread (NULL, 0, ThreadProc, (LPVOID) pDispatcher, 0, NULL);       
    }

    return true;

}

/**
* Close handles for OS objects corresponding the communication channel
* corresponding to the specified event. This method is called when the
* event is abandoned or closed.
* 
* @param hReceiveEvent
*            handle of receive event corresponding to connection that is being
*            dropped.
*/
void IPCStub::RemoveChannel (HANDLE hReceiveEvent)
{
    TRACE (2, _T("Channel removed: %d\n"), hReceiveEvent);

    ::WaitForSingleObject (m_hEventListMutex, INFINITE);

    m_eventChannelMap.erase (hReceiveEvent);

    bool bFound = false;
    for (PtrVector::iterator iter = m_eventBucketList.begin (); !bFound && iter != m_eventBucketList.end (); iter++)
    {
        PtrVector* pEventList = (PtrVector*) *iter;
        for (PtrVector::iterator eventListIter = pEventList->begin (); eventListIter != pEventList->end (); eventListIter++)
        {
            if (((HANDLE) *eventListIter) == hReceiveEvent)
            {
                pEventList->erase (eventListIter);
                bFound = true;
                break;
            }
        }
    }

    ::ReleaseMutex (m_hEventListMutex);

}

/**
* Waits for events to be signaled. Waits on events in the bucket specified by
* eventBucketNumber. Each bucket can have a maximum of MAXIMUM_WAIT_OBJECTS
* events. When that number is exceeded, a new bucket is created.
* 
* An event being signaled specifies that a message is received. Messages
* are dispatched based on the type of message. For handshake messages, call
* performHandshake. For invoke request, add the corresponding
* IPCMessageChannel instance to the requestQueue.
*
* @param eventBucketNumber specifies the bucket of events to wait on.
* 
*/
void IPCStub::Run(intptr_t eventBucketNumber)
{
    m_bStopped = false;
    while (!m_bStopped)
    {
        int numEvents;
        HANDLE* pHandleArray = GetEventArray (numEvents, eventBucketNumber);

        if (numEvents == 0)
        {
            // Thread no longer needed. A new thread will be created by 
            // ::PerformHandshake() when an event is added to the associated bucket
            return;
        }

        DWORD event = ::WaitForMultipleObjects (numEvents, pHandleArray, FALSE, TIMEOUT);

        if (event == WAIT_TIMEOUT)
        {
            // Do nothing... This will be handled by the loop condition
        }
        else if (event == WAIT_FAILED)
        {
            TRACE (1, _T("WaitForMultipleObjects call failed: Error# %d\n"), ::GetLastError ());
        }
        else if (event >= WAIT_ABANDONED_0 && event < WAIT_ABANDONED_0 + numEvents)
        {
            RemoveChannel (pHandleArray[event - WAIT_ABANDONED_0]);
        }
        else if (eventBucketNumber == 0 && 
                 event - WAIT_OBJECT_0 < static_cast<DWORD>(numEvents) && 
                 pHandleArray[event - WAIT_OBJECT_0] == m_hReceiveEvent)
        {
            PerformHandshake();
        }
        else if (event - WAIT_OBJECT_0 < static_cast<DWORD>(numEvents) && event - WAIT_OBJECT_0 >= 0)
        {            
            PtrToPtrMap::iterator iter = m_eventChannelMap.find((void*)(pHandleArray[event - WAIT_OBJECT_0]));

            if (iter != m_eventChannelMap.end()) {
                IPCMessageChannel* pMessageChannel = (IPCMessageChannel*) iter->second;

                ::WaitForSingleObject (m_hQueueMutex, INFINITE);
                m_requestQueue.push_back (pMessageChannel);
                ::SetEvent (m_hQueueEvent);
                ::ReleaseMutex (m_hQueueMutex);
            }

        }
        else
        {
            TRACE (1, _T("WaitForMultipleObjects call returned unknown return value %d\n"), event);
        }

        if ((event != WAIT_FAILED) && (event != WAIT_TIMEOUT)) {
          ResetEvent (pHandleArray[event - WAIT_OBJECT_0]);
        }

        delete [] pHandleArray;
    }

    Cleanup ();
}

/**
* Stop threadpool and close handles for all OS objects
*/
void IPCStub::Cleanup ()
{
    for (int i = 0; i < m_threadPoolSize; i++)
    {
        m_requestDispatcherArray[i]->Stop ();
    }

    ::WaitForMultipleObjects (m_threadPoolSize, m_threadArray, TRUE, INFINITE);

    for (int i = 0; i < m_threadPoolSize; i++)
    {
        delete (m_requestDispatcherArray[i]);
        ::CloseHandle (m_threadArray[i]);
    }

    delete[] m_requestDispatcherArray;
    delete[] m_threadArray;

    ::UnmapViewOfFile (m_pSharedMemory);
    ::CloseHandle (m_hSharedFileMapping);
    ::CloseHandle (m_hSendEvent);
    ::CloseHandle (m_hReceiveEvent);
    ::CloseHandle (m_hHandshakeMutex);
    ::CloseHandle (m_hQueueEvent);
    ::CloseHandle (m_hQueueMutex);
    ::CloseHandle (m_hEventListMutex);

    for (PtrToPtrMap::iterator iter = m_eventChannelMap.begin (); 
        iter != m_eventChannelMap.end (); 
        iter++)
    {
        // Destructor of message channel closes all handles.
        delete ((IPCMessageChannel*) iter->second);
    }
    m_eventChannelMap.clear();
}

void IPCStub::Stop ()
{
    m_bStopped = true;
}


bool IPCStub::SetupSharedMemMapping()
{
    if (m_pSharedMemory == NULL)
    {
        m_hSharedFileMapping = ::OpenFileMapping (FILE_MAP_WRITE, FALSE, m_serverSharedMemoryFileName.c_str());
        
        if (m_hSharedFileMapping == NULL)
        {
            return false;
        }
        
        m_pSharedMemory = (TCHAR*) ::MapViewOfFile(m_hSharedFileMapping,
                                                   FILE_MAP_ALL_ACCESS,
                                                   0,
                                                   0,
                                                   0);
    }
    return true;
}
/**
* reads the handshake message and completes the handshake so that
* subsequent requests can be handled.
* 
* If a connection is being established, creates an instance of
* IPCMessageChannel with the shared memory location and events specified
* and add it to m_eventChannelMap. Add the receive event to the list of
* events to wait on. 
* The list of events are stored in buckets of size (MAXIMUM_WAIT_OBJECTS - 1)
* since the OS only supports waiting on MAXIMUM_WAIT_OBJECTS events.
*  
* If no existing bucket has any space remaining, a new bucket is created and 
* a new thread is created to wait on the events in the new bucket
* When a bucket becomes empty, thread exits. If an event is added to an existing
* but empty bucket, a new thread is also created since the original thread on that
* bucket will have exited.
* 
*/
void IPCStub::PerformHandshake ()
{
    intptr_t bucketIndex = -1;
    TCHAR *buf = new TCHAR[CHANNEL_SIZE];

    if (!SetupSharedMemMapping())
    {
        TRACE(1, _T("Unable to acquire shared memory mapping in PerformHandshake.  Exiting\n"));
        delete []buf;
        return;
    }

    _tcsncpy_s (buf, CHANNEL_SIZE, m_pSharedMemory, _TRUNCATE);

    TCHAR* pToken = NULL;
    WCHAR* next_token = NULL;
    pToken = _tcstok_s (buf, _T("\n"), &next_token);

    TRACE (1, _T("Handshake Request Received: \n%s\n"), buf);


    if (_tcscmp (pToken, CONNECT) == 0)
    {
        // shared memory filename
        pToken = _tcstok_s (NULL, _T("\n"), &next_token);
        if (!pToken)
        {
            TRACE (1, _T("Invalid handshake request..."));
            delete []buf;
            return;
        }
        HANDLE hFileMapping = ::OpenFileMapping (FILE_MAP_WRITE, // read/write permission 
            FALSE,               // Do not inherit the name 
            pToken);             // of the mapping object. 

        TCHAR* pSharedMem = (TCHAR*) ::MapViewOfFile (hFileMapping, // handle to mapping object 
            FILE_MAP_ALL_ACCESS,               // read/write permission 
            0,                                 // max. object size 
            0,                                 // size of hFile 
            0);                                // map entire file 	

        // send event name
        pToken = _tcstok_s (NULL, _T("\n"), &next_token);
        if (!pToken)
        {
            //TODO: log error
            delete []buf;
            return;
        }
        HANDLE hSendEvent = ::OpenEvent (SYNCHRONIZE | EVENT_MODIFY_STATE, FALSE, pToken);

        //receive event name
        pToken = _tcstok_s (NULL, _T("\n"), &next_token);
        if (!pToken)
        {
            //TODO: log error
            delete []buf;
            return;
        }
        HANDLE hReceiveEvent = ::OpenEvent (SYNCHRONIZE | EVENT_MODIFY_STATE, FALSE, pToken);

        IPCMessageChannel* pMessageChannel = new IPCMessageChannel (hFileMapping, pSharedMem, hSendEvent, hReceiveEvent);
        ::WaitForSingleObject (m_hEventListMutex, INFINITE);

        PtrVector* pEventList = NULL;
        for (unsigned int i = 0; i < m_eventBucketList.size (); i++)
        {
            pEventList = (PtrVector*) m_eventBucketList.at (i);
            if (pEventList->size () < MAXIMUM_WAIT_OBJECTS - 1)
            {
                bucketIndex = i;
                break;
            }
        }

        if (bucketIndex == -1)
        {
            // no bucket has space. Create new bucket
            pEventList = new (std::nothrow) PtrVector ();
            m_eventBucketList.push_back (pEventList);
            bucketIndex = m_eventBucketList.size () - 1;
        }

        pEventList->push_back (hReceiveEvent);
        m_eventChannelMap[hReceiveEvent] = (void*) pMessageChannel;

        if (pEventList->size () == 1)
        {
            // New thread is needed because a new event list was created or 
            // an event was added to an existing empty event list
            StubThreadInfo* pInfo = new StubThreadInfo;
            pInfo->m_pStub = this;
            pInfo->m_bucketNumber = bucketIndex;
            ::CreateThread (NULL, 0, StubThreadProc, (LPVOID) pInfo, 0, NULL);       
        }

        ::ReleaseMutex (m_hEventListMutex);

    }
    else
    {
        //TODO: log error
    }

    ::SetEvent (m_hSendEvent);

    delete []buf;
}


/**
* Returns the event list as an array to be used with WaitForMultipleObjects. 
* Array must be deallocated by caller.
*/
HANDLE* IPCStub::GetEventArray (int& size, intptr_t bucketNumber)
{
    ::WaitForSingleObject (m_hEventListMutex, INFINITE);
    PtrVector* pEventList = (PtrVector*) m_eventBucketList.at (bucketNumber);

    HANDLE* retval = new HANDLE [pEventList->size ()];
    int i = 0;
    for (PtrVector::iterator iter = pEventList->begin (); iter != pEventList->end (); iter++)
    {
        retval [i++] = (HANDLE) *iter;
    }

    ::ReleaseMutex (m_hEventListMutex);

	size = i;

    return (retval);
}
