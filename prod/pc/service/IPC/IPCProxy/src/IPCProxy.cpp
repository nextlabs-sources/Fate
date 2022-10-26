// All sources, binaries and HTML pages (C) copyright 2004 by Blue Jungle Inc., 
// Redwood City CA, Ownership remains with Blue Jungle Inc, 
// All rights reserved worldwide. 

/**
 * 
 * @author fuad
 * @version $Id$:
 */

#include "StdAfx.h"
#include <windows.h>
#include <wincrypt.h>
#include "..\..\shared\src\globals.h"
#include "..\..\shared\src\ilog.h"
#include "..\..\shared\src\logfactory.h"
#include "..\..\shared\src\securityattributesfactory.h"
#include "..\..\..\..\..\common\include\dsipc.h"
#include "ipcconstants.h"
#include "ipcproxy.h"

#define REQUEST_OPEN_TAG _T("<request>")
#define REQUEST_CLOSE_TAG _T("</request>")
#define METHOD_OPEN_TAG _T("<method name=\"%s\">")
#define METHOD_CLOSE_TAG _T("</method>")
#define PARAM_TAG _T("<param value=\"%s\"/>")
#define UNINIT_TIMEOUT 500
#define REQUEST_TIMEOUT 5000
#define SERVER_RESPONSE_TIMEOUT 4000

//Uncomment this line to get some tracing
//#define DEBUG_LOG

#ifdef DEBUG_LOG
static BOOL LogMsg (LPCTSTR pMsg)
{

	DWORD dwErr = GetLastError();
	HWND hwnd = FindWindow (_T("DC_MESSAGE_LOGGER_WND_CLASS"), NULL);
	if ( !hwnd )
	{
		SetLastError (dwErr);
		return FALSE;
	}

	COPYDATASTRUCT copydata;
	copydata.lpData = (LPVOID) pMsg;
	copydata.cbData = (pMsg) ? ((_tcslen(pMsg) + 1) * sizeof(TCHAR)) : 0;
	copydata.dwData = 0;
	SendMessageW (hwnd, WM_COPYDATA, (WPARAM) NULL, (LPARAM) &copydata);
	SetLastError (dwErr);
	return TRUE;
}
#endif

IPCProxy::IPCProxy(void)
{
    m_bInitialized = false;
    m_hMapFile = NULL;
    m_sharedMem = NULL;
    m_hSendEvent = NULL;
    m_hReceiveEvent = NULL;
    m_hMutex = NULL;
    m_pLog = LogFactory::GetLogger();
}

/**
 * Calls uninit to disconnect session and releases OS handles
 */
IPCProxy::~IPCProxy(void)
{
    Uninit ();

    if (m_sharedMem != NULL)
    {
        ::UnmapViewOfFile (m_sharedMem);
    }
    if (m_hMapFile != NULL)
    {
        ::CloseHandle (m_hMapFile);
    }
    if (m_hSendEvent != NULL)
    {
        ::CloseHandle (m_hSendEvent);
    }
    if (m_hReceiveEvent != NULL)
    {
        ::CloseHandle (m_hReceiveEvent);
    }
    if (m_hMutex != NULL)
    {
        ::CloseHandle (m_hMutex);
    }
}

static void GetUniquePrefix(TCHAR *uniqueName, size_t size)
{

    // This might appear to be really heavyweight, but it's the only
    // thing that seems to work reliably.  We need a unique id to send
    // to the PC.  The combination of thread and timestamp won't work,
    // because the timestamp isn't fine-grained enough.  rand() is
    // part of the crt and if we have that statically linked we could
    // get the same random sequence if we happen to call it from two
    // different dlls from the same thread within the time granularity
    // of timestamp (about 16ms).  Maybe.  We could either use a
    // high-res timestamp or roll out the big guns....

    // If all else fails...
    unsigned int extraEntropy = rand();

    HCRYPTPROV hCryptProv = NULL;

    if(!CryptAcquireContext(&hCryptProv, NULL, NULL, PROV_RSA_FULL, CRYPT_NEWKEYSET))
    {
        DWORD err = GetLastError();
        if (err == NTE_EXISTS)
        {
            CryptReleaseContext(hCryptProv, 0);
            if (!CryptAcquireContext(&hCryptProv, NULL, NULL, PROV_RSA_FULL, 0))
            {		
                hCryptProv = NULL;
            }
        } 
        else
        {
            hCryptProv = NULL;
        }
    }

    if (hCryptProv != NULL)
    {
        if (!CryptGenRandom(hCryptProv, sizeof(unsigned int)/sizeof(BYTE), (BYTE *)&extraEntropy)) 
        {
            extraEntropy = rand();
        }
        
        CryptReleaseContext(hCryptProv, 0);
    }

    // uniqueName is constructed by concatenating the current process id 
    // with a timestamp and some extra entropy
    // the timestamp only has about 16ms resolution, so it's not enough by itself
    // as two successive calls in the same thread can give the same value
    _sntprintf_s (uniqueName, size, _TRUNCATE, _T("%s%d-%d-%u"), 
               GLOBAL_NAME_PREFIX,
               ::GetCurrentThreadId(),
               ::GetTickCount(),
               extraEntropy);

    return;
}

/**
 * Sends handshake request to server to set up the session.
 * 
 * @param requestHandlerClassName
 *            class name of IPC request handler.
 */
bool IPCProxy::Init (const TCHAR* requestHandlerClassName)
{
    bool bRetVal = false;
    HANDLE hMapFile = NULL;
    TCHAR* objectNameBuffer = NULL;
    TCHAR* handshakeSharedMem = NULL;
    HANDLE hSendEvent = 0;
    HANDLE hReceiveEvent = 0;
    HANDLE hMutex = 0;
    TCHAR uniqueName [100];
    TCHAR* sendEventName = NULL;
    TCHAR* receiveEventName = NULL;
    TCHAR* mutexName = NULL;

    if (m_bInitialized) 
    {
        return (false);
    }

    size_t nameLength = _tcslen (requestHandlerClassName);
    objectNameBuffer = new TCHAR[GLOBAL_NAME_PREFIX_LENGTH + nameLength + _tcslen(MUTEX_SUFFIX) + 1]; // mutex is the longest suffix. This string will be used for all objects
    
    //get shared memory handle
    _tcsncpy_s (objectNameBuffer, GLOBAL_NAME_PREFIX_LENGTH + nameLength + _tcslen(MUTEX_SUFFIX) + 1, GLOBAL_NAME_PREFIX, _TRUNCATE);
    _tcsncat_s (objectNameBuffer, GLOBAL_NAME_PREFIX_LENGTH + nameLength + _tcslen(MUTEX_SUFFIX) + 1, requestHandlerClassName, _TRUNCATE);
    _tcsncat_s (objectNameBuffer, GLOBAL_NAME_PREFIX_LENGTH + nameLength + _tcslen(MUTEX_SUFFIX) + 1, SHARED_MEMORY_SUFFIX, _TRUNCATE);
    hMapFile = OpenFileMapping(FILE_MAP_WRITE, // read/write permission 
                               FALSE,               // Do not inherit the name 
                               objectNameBuffer);// of the mapping object. 

    if (hMapFile == NULL)
    {
        //handshake failed.
        bRetVal = false;
        m_pLog->Error (_T("Handshake failed: OpenFileMapping did not return a handle"));
        
        goto label_fail;
    }

    handshakeSharedMem = (TCHAR*) MapViewOfFile(hMapFile, // handle to mapping object 
                                                FILE_MAP_ALL_ACCESS,               // read/write permission 
                                                0,                                 // max. object size 
                                                0,                                 // size of hFile 
                                                0);                                // map entire file 
    
    if (handshakeSharedMem == NULL)
    {
        //handshake failed.
        bRetVal = false;
        m_pLog->Error (_T("Handshake failed: MapViewOfFile failed"));
        goto label_fail;
    }

    //get send events
    _tcsncpy_s (objectNameBuffer, GLOBAL_NAME_PREFIX_LENGTH + nameLength + _tcslen(MUTEX_SUFFIX) + 1, GLOBAL_NAME_PREFIX, _TRUNCATE);
    _tcsncat_s (objectNameBuffer, GLOBAL_NAME_PREFIX_LENGTH + nameLength + _tcslen(MUTEX_SUFFIX) + 1, requestHandlerClassName, _TRUNCATE);
    _tcsncat_s (objectNameBuffer, GLOBAL_NAME_PREFIX_LENGTH + nameLength + _tcslen(MUTEX_SUFFIX) + 1, RECEIVE_EVENT_SUFFIX, _TRUNCATE); //send event for proxy is receive for stub
    hSendEvent = ::OpenEvent(SYNCHRONIZE | EVENT_MODIFY_STATE, FALSE, objectNameBuffer);
    if (hSendEvent == NULL)
    {
        //handshake failed.
        bRetVal = false;
        m_pLog->Error (_T("Handshake failed: OpenEvent failed for Send Event"));
        goto label_fail;
    }

    //get receive events
    _tcsncpy_s (objectNameBuffer, GLOBAL_NAME_PREFIX_LENGTH + nameLength + _tcslen(MUTEX_SUFFIX) + 1, GLOBAL_NAME_PREFIX, _TRUNCATE);
    _tcsncat_s (objectNameBuffer, GLOBAL_NAME_PREFIX_LENGTH + nameLength + _tcslen(MUTEX_SUFFIX) + 1, requestHandlerClassName, _TRUNCATE);
    _tcsncat_s (objectNameBuffer, GLOBAL_NAME_PREFIX_LENGTH + nameLength + _tcslen(MUTEX_SUFFIX) + 1, SEND_EVENT_SUFFIX, _TRUNCATE); //receive event for proxy is send for stub
    hReceiveEvent = ::OpenEvent(SYNCHRONIZE | EVENT_MODIFY_STATE, FALSE, objectNameBuffer);
    if (hReceiveEvent == NULL)
    {
        //handshake failed.
        bRetVal = false;
        m_pLog->Error (_T("Handshake failed: OpenEvent failed for Receive Event"));
        goto label_fail;
    }

    //get mutex
    _tcsncpy_s (objectNameBuffer, GLOBAL_NAME_PREFIX_LENGTH + nameLength + _tcslen(MUTEX_SUFFIX) + 1, GLOBAL_NAME_PREFIX, _TRUNCATE);
    _tcsncat_s (objectNameBuffer, GLOBAL_NAME_PREFIX_LENGTH + nameLength + _tcslen(MUTEX_SUFFIX) + 1, requestHandlerClassName, _TRUNCATE);
    _tcsncat_s (objectNameBuffer, GLOBAL_NAME_PREFIX_LENGTH + nameLength + _tcslen(MUTEX_SUFFIX) + 1, MUTEX_SUFFIX, _TRUNCATE);
    hMutex = ::OpenMutex (SYNCHRONIZE | MUTEX_MODIFY_STATE, FALSE, objectNameBuffer);

    if (hMutex == NULL)
    {
        //handshake failed.
        bRetVal = false;
        m_pLog->Error (_T("Handshake failed: OpenMutex failed"));
        goto label_fail;
    }

    GetUniquePrefix(uniqueName, _countof(uniqueName));

    nameLength = _tcslen (uniqueName);
    m_sharedMemName = new TCHAR[nameLength + _tcslen(SHARED_MEMORY_SUFFIX) + 1];
    sendEventName = new TCHAR[nameLength + _tcslen(SEND_EVENT_SUFFIX) + 1];
    receiveEventName = new TCHAR[nameLength + _tcslen(RECEIVE_EVENT_SUFFIX) + 1];
    mutexName = new TCHAR[nameLength + _tcslen(MUTEX_SUFFIX) + 1];

    _tcsncpy_s (m_sharedMemName, nameLength + _tcslen(SHARED_MEMORY_SUFFIX) + 1, uniqueName, _TRUNCATE);
    _tcsncat_s (m_sharedMemName, nameLength + _tcslen(SHARED_MEMORY_SUFFIX) + 1, SHARED_MEMORY_SUFFIX, _TRUNCATE);
    _tcsncpy_s (sendEventName, nameLength + _tcslen(SEND_EVENT_SUFFIX) + 1, uniqueName, _TRUNCATE);
    _tcsncat_s (sendEventName, nameLength + _tcslen(SEND_EVENT_SUFFIX) + 1, SEND_EVENT_SUFFIX, _TRUNCATE);
    _tcsncpy_s (receiveEventName, nameLength + _tcslen(RECEIVE_EVENT_SUFFIX) + 1, uniqueName, _TRUNCATE);
    _tcsncat_s (receiveEventName, nameLength + _tcslen(RECEIVE_EVENT_SUFFIX) + 1, RECEIVE_EVENT_SUFFIX, _TRUNCATE);
    _tcsncpy_s (mutexName, nameLength + _tcslen(MUTEX_SUFFIX) + 1, uniqueName, _TRUNCATE);
    _tcsncat_s (mutexName, nameLength + _tcslen(MUTEX_SUFFIX) + 1, MUTEX_SUFFIX, _TRUNCATE);


    GetSharedMemMapping();

    m_hSendEvent = ::CreateEvent (SecurityAttributesFactory::GetSecurityAttributes(), FALSE, FALSE, sendEventName);

    if (m_hSendEvent == 0) {
        TCHAR buf[256];
        _sntprintf_s(buf, _countof(buf), _TRUNCATE, _T("Unable to open event %s (err = %d)\n"), sendEventName, ::GetLastError());
        m_pLog->Error(buf);
        bRetVal = false;
        goto label_fail;
    }

    m_hReceiveEvent = ::CreateEvent (SecurityAttributesFactory::GetSecurityAttributes(), FALSE, FALSE, receiveEventName);
    if (m_hReceiveEvent == 0) {
        TCHAR buf[256];
        _sntprintf_s(buf, _countof(buf), _TRUNCATE, _T("Unable to open event %s (err = %d)\n"), receiveEventName, ::GetLastError());
        m_pLog->Error(buf);
        bRetVal = false;
        goto label_fail;
    }

    m_hMutex = ::CreateMutex (SecurityAttributesFactory::GetSecurityAttributes(), FALSE, mutexName);
    if (m_hMutex == 0) {
        TCHAR buf[256];
        _sntprintf_s(buf, _countof(buf), _TRUNCATE, _T("Unable to open mutex %s (err = %d)\n"), mutexName, ::GetLastError());
        m_pLog->Error(buf);
        bRetVal = false;
        goto label_fail;
    }

    //Start handshake
    //get mutex
    if(::WaitForSingleObject (hMutex, REQUEST_TIMEOUT) == WAIT_TIMEOUT) {
        //time out to get mutex; exit
        bRetVal=false;
        goto label_fail;
    } 
    _stprintf (handshakeSharedMem, HANDSHAKE_STR, 
               CONNECT, 
               m_sharedMemName,
               receiveEventName, 
               sendEventName);

    if (m_pLog->IsInfoEnabled ())
    {
        TCHAR log[CHANNEL_SIZE + 100];
        _sntprintf_s (log, _countof(log), _TRUNCATE, _T("Handshake Request Sent: %s\n"), handshakeSharedMem);
        m_pLog->Info (log);
    }

    ::SetEvent (hSendEvent);

    // Technically we should wait here until the server gives us a
    // response.  A timeout leaves the channel in an in-between state,
    // where both sides think that they can use it.  In reality, if
    // the server doesn't give us a response in a few seconds then
    // something has gone wrong and it never will.  Rather than spin our
    // wheels, holding the server mutex, we should just give up.
    if (::WaitForSingleObject (hReceiveEvent, SERVER_RESPONSE_TIMEOUT) == WAIT_TIMEOUT) {
        ::ReleaseMutex(hMutex);
        TCHAR log[CHANNEL_SIZE + 100];
        _sntprintf_s(log, _countof(log), _TRUNCATE, _T("Never got response for handshake request: %s\n"), handshakeSharedMem);
        m_pLog->Error(log);
        bRetVal = false;
        goto label_fail;
    }
    ::ResetEvent(hReceiveEvent);
    ::ReleaseMutex(hMutex);
    //end handshake

    m_bInitialized = true;
    bRetVal = true;

 label_fail:
    delete[] (sendEventName);
    delete[] (receiveEventName);
    delete[] (mutexName);
    delete[] (objectNameBuffer);

    if (handshakeSharedMem != NULL)
    {
        ::UnmapViewOfFile (handshakeSharedMem);
        handshakeSharedMem = NULL;
    }
    if (hMapFile != NULL)
    {
        ::CloseHandle (hMapFile);
        hMapFile = 0;
    }
    if (hSendEvent != 0)
    {
        ::CloseHandle (hSendEvent);
        hSendEvent = 0;
    }
    if (hReceiveEvent != 0)
    {
        ::CloseHandle (hReceiveEvent);
        hReceiveEvent = 0;
    }
    if (hMutex != 0)
    {
        ::CloseHandle (hMutex);
        hMutex = 0;
    }

    return (bRetVal);

}

/**
 * sends method invocation request using IPC mechanism to server 
 * and returns the response
 * 
 * @param pRequest
 *            request object
 * @param pResponse
 *            pointer to response object
 * @return true if invocation is successful, false
 */
bool IPCProxy::Invoke (IPCREQUEST* pRequest, IPCREQUEST* pResponse)
{
    return Invoke (pRequest, pResponse, REQUEST_TIMEOUT);
}

/**
 * sends method invocation request using IPC mechanism to server 
 * and returns the response
 * 
 * @param pRequest
 *            request object
 * @param pResponse
 *            pointer to response object
 * @param timeout
 *            timeout in milliseconds. If we dont get a response 
 *            by this time, timeout and return false.
 * @return true if invocation is successful, false
 */
bool IPCProxy::Invoke (IPCREQUEST* pRequest, IPCREQUEST* pResponse, UINT timeout)
{
#ifdef DEBUG_LOG
    LogMsg(_T("In invoke"));
#endif
    if (!m_bInitialized) 
    {
#ifdef DEBUG_LOG
        LogMsg(_T("Not initialized"));
#endif
        m_pLog->Error (_T("Invoke failed: IPCProxy not initialized"));
        return (false);
    }

#ifdef DEBUG_LOG
    LogMsg(_T("Waiting for single object"));
#endif
    if (::WaitForSingleObject (m_hMutex, timeout) == WAIT_TIMEOUT)
    {
#ifdef DEBUG_LOG
        LogMsg(_T("Timeout waiting for mutex!"));
#endif
        m_pLog->Error (_T("Invoke failed: Timeout"));
        return (false);
    }

#ifdef DEBUG_LOG
    LogMsg(_T("Setting shared memory"));
    if (pRequest == NULL)
    {
        LogMsg(_T("Request is NULL"));
    }
    if (m_sharedMem == NULL)
    {
        LogMsg(_T("m_sharedMem is NULL"));
    }
    TCHAR msg[8192];
    _snwprintf_s(msg, 8192, _TRUNCATE, _T("Request size is %d"), pRequest->ulSize);
    LogMsg(msg);
#endif

    if (m_sharedMem == 0) {
        // Perhaps it's available now
        GetSharedMemMapping();

        if (m_sharedMem == 0) {
            // Or not.  We'll try again next time
			::ReleaseMutex (m_hMutex);
            return false;
        }
    }


    memcpy (m_sharedMem, pRequest, pRequest->ulSize);

    if (m_pLog->IsInfoEnabled ())
    {
        TCHAR *log = new TCHAR[CHANNEL_SIZE + 100];
        _sntprintf_s (log, CHANNEL_SIZE + 100, _TRUNCATE, _T("Thread %d: Request Sent: %s\n"), ::GetCurrentThreadId(), m_sharedMem);
        m_pLog->Info (log);
        delete[] (log);
    }
#ifdef DEBUG_LOG
    LogMsg(_T("Setting event"));
#endif
    ::SetEvent (m_hSendEvent);
#ifdef DEBUG_LOG
    LogMsg(_T("Waiting for receive event"));
#endif
    if (::WaitForSingleObject (m_hReceiveEvent, timeout) == WAIT_TIMEOUT)
    {
#ifdef DEBUG_LOG
        LogMsg(_T("Timeout waiting for receive event!"));
#endif
        ::ReleaseMutex (m_hMutex);
        m_pLog->Error (_T("Invoke failed: Timeout"));
        return (false);
    }
#ifdef DEBUG_LOG
    LogMsg(_T("Copy response"));
#endif
    memcpy (pResponse, m_sharedMem,
            min(pResponse->ulSize, ((IPCREQUEST*)m_sharedMem)->ulSize));
#ifdef DEBUG_LOG
    LogMsg(_T("releasing mutex"));
#endif
    ::ReleaseMutex (m_hMutex);

    if (m_pLog->IsInfoEnabled ())
    {
        TCHAR log[100];
        _sntprintf_s (log, _countof(log), _TRUNCATE, _T("Thread %d: Received Response: ?\n"), ::GetCurrentThreadId());
        m_pLog->Info (log);
    }

    return (true);
}


/**
 * send disconnect request to the server.
 */
void IPCProxy::Uninit()
{
    if (m_bInitialized) 
    {
        //Send disconnect request to server
        if (::WaitForSingleObject (m_hMutex, UNINIT_TIMEOUT) != WAIT_TIMEOUT)
        {
            IPCREQUEST* pRequest = (IPCREQUEST*) m_sharedMem;
            memset (pRequest, 0, sizeof (IPCREQUEST));
            pRequest->ulSize = sizeof (IPCREQUEST);
            _tcsncpy_s (pRequest->methodName, _countof(pRequest->methodName), DISCONNECT, _TRUNCATE);
            ::SetEvent (m_hSendEvent);
            ::WaitForSingleObject (m_hReceiveEvent, UNINIT_TIMEOUT);
            ::ReleaseMutex (m_hMutex);
        }
        m_bInitialized = false;
    }
}

bool IPCProxy::GetSharedMemMapping()
{
    m_hMapFile = OpenFileMapping(FILE_MAP_WRITE, FALSE, m_sharedMemName);

    if (m_hMapFile == NULL) 
    {
        return false;
    }


    m_sharedMem = (TCHAR*) MapViewOfFile(m_hMapFile, // handle to mapping object 
                                         FILE_MAP_ALL_ACCESS,               // read/write permission 
                                         0,                                 // max. object size 
                                         0,                                 // size of hFile 
                                         0);                                // map entire file 

    if (m_sharedMem == NULL)
    {
        return false;
    }

    return true;
}
