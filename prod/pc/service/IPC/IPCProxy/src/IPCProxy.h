// All sources, binaries and HTML pages (C) copyright 2004 by Blue Jungle Inc., 
// Redwood City CA, Ownership remains with Blue Jungle Inc, 
// All rights reserved worldwide. 

#ifndef _IPCProxy_h_
#define _IPCProxy_h_

#include "..\..\..\..\..\common\include\dsipc.h"

class ILog;

/**
 * IPCProxy implements the client side of the IPC Framework. Processes that need
 * to call methods on the IPC Stub (or server) need to instantiate IPCStub (or
 * subclass).
 * 
 * Users of this class must instantiate it and call init to pass in the request
 * handler class name before calling Invoke to invoke methods.
 * 
 * @author fuad
 * @version $Id:
 *          //depot/main/Destiny/main/src/etc/eclipse/destiny-code-templates.xml#2 $:
 */
class IPCProxy
{

public:
    IPCProxy(void);
    virtual ~IPCProxy(void);

    virtual bool Init (const TCHAR* requestHandlerClassName);

    bool Invoke (IPCREQUEST* pRequest, IPCREQUEST* pResponse);
    bool Invoke (IPCREQUEST* pRequest, IPCREQUEST* pResponse, UINT timeout);

protected:
    void Uninit ();


// Private Member Variables
private:
    bool   m_bInitialized;
    HANDLE m_hMapFile;
    TCHAR* m_sharedMem;
    TCHAR* m_sharedMemName;
    HANDLE m_hSendEvent;
    HANDLE m_hReceiveEvent;
    HANDLE m_hMutex;

    ILog* m_pLog;
    bool GetSharedMemMapping();
};

#endif
