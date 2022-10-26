// All sources, binaries and HTML pages (C) copyright 2004 by Blue Jungle Inc., 
// Redwood City CA, Ownership remains with Blue Jungle Inc, 
// All rights reserved worldwide. 

#ifndef _IPCStub_h_
#define _IPCStub_h_

class IIPCRequestHandler;
class RequestDispatcher;
class ILog;

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
class DllExport IPCStub
{
public:
    IPCStub(void);
    ~IPCStub(void);

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
    bool Init(IIPCRequestHandler** pRequestHandlerArray, int threadPoolSize);

    /**
     * Waits for events to be signaled.
     * 
     * An event being signaled specifies that a message is received. Messages
     * are dispatched based on the type of message. For handshake messages, call
     * performHandshake. For invoke request, add the corresponding
     * IPCMessageChannel instance to the requestQueue.
     */
    void Run (intptr_t eventBucketNumber = 0);

    /**
     * Stop threadpool and close handles for all OS objects
     */
    void Stop ();

    /**
     * Close handles for OS objects corresponding the communication channel
     * corresponding to the specified event. This method is called when the
     * event is abandoned or closed.
     * 
     * @param hReceiveEvent
     *            handle of receive event corresponding to connection that is being
     *            dropped.
     */
    void RemoveChannel (HANDLE hReceiveEvent);

private:
    HANDLE* GetEventArray(int& size, intptr_t bucketNumber);

    /**
     * reads the handshake message and completes the handshake so that
     * subsequent requests can be handled.
     * 
     * If a connection is being established, creates an instance of
     * IPCMessageChannel with the shared memory location and events specified
     * and add it to m_eventChannelMap. Add the receive event to the list of
     * events to wait on (m_eventList).
     *  
     */
    void PerformHandshake ();

    void Cleanup ();

private:
        HANDLE m_hSharedFileMapping;
        TCHAR* m_pSharedMemory;
        HANDLE m_hSendEvent;
        HANDLE m_hReceiveEvent;
        HANDLE m_hHandshakeMutex;

        PtrVector m_eventBucketList;
        //PtrVector m_eventList;
        HANDLE m_hEventListMutex;
        PtrToPtrMap m_eventChannelMap; 
        HANDLE* m_threadArray;
        RequestDispatcher** m_requestDispatcherArray;
        int m_threadPoolSize;
        PtrList m_requestQueue;
        HANDLE m_hQueueMutex;
        HANDLE m_hQueueEvent;

        bool m_bStopped;

        ILog* m_pLog;
};

#endif