// All sources, binaries and HTML pages (C) copyright 2004 by Blue Jungle Inc., 
// Redwood City CA, Ownership remains with Blue Jungle Inc, 
// All rights reserved worldwide. 

#ifndef _RequestDispatcher_h_
#define _RequestDispatcher_h_

class IPCStub;
class IPCMessageChannel;

/**
 * The RequestDispatcher waits on the request queue. When the queue has an entry and we 
 * can obtain a lock on the mutex, pop the queue and handle the request 
 * 
 * @version $Id:
 *          //depot/main/Destiny/main/src/etc/eclipse/destiny-code-templates.xml#2 $:
 * 
 */
class RequestDispatcher
{
public:
    RequestDispatcher(void);
    ~RequestDispatcher(void);

    /**
     * Wait on the request queue. When the queue has an entry and we 
     * can obtain a lock on the mutex, pop the queue and handle the 
     * request by calling DispatchRequest()
     * 
     */
    void Run ();

    /**
     * Force request dispatcher to return from its loop
     */
    void Stop ();

    void SetRequestQueue (PtrList* pRequestQueue) {m_pRequestQueue = pRequestQueue;}
    void SetQueueMutex (HANDLE hQueueMutex) {m_hQueueMutex = hQueueMutex;}
    void SetQueueEvent (HANDLE hQueueEvent) {m_hQueueEvent = hQueueEvent;}
    void SetRequestHandler (IIPCRequestHandler* pRequestHandler) {m_pRequestHandler = pRequestHandler;}
    void SetIPCStub (IPCStub* pStub) {m_pStub = pStub;}

private:
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
    void DispatchRequest (IPCMessageChannel* pMessageChannel);

private:
    PtrList* m_pRequestQueue;
    HANDLE m_hQueueMutex;
    HANDLE m_hQueueEvent;
    IIPCRequestHandler* m_pRequestHandler;
    IPCStub* m_pStub;
    bool m_bStopped;
    
    // ILog* m_pLog;

};

#endif
