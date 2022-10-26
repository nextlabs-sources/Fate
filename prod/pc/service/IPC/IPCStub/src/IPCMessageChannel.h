// All sources, binaries and HTML pages (C) copyright 2004 by Blue Jungle Inc., 
// Redwood City CA, Ownership remains with Blue Jungle Inc, 
// All rights reserved worldwide. 

#ifndef _IPCMessageChannel_h_
#define _IPCMessageChannel_h_

/**
 * Each instance of this class represents one channel of communication between
 * client and server IPC processes. Whenever a client comes up, it performs a
 * handshake by providing a shared memory location and two events. The shared
 * memory is used communication and the two events are used to synchronize
 * requests. An instance of this class is created to represent the channel.
 * 
 * @author fuad
 * @version $Id:
 *          //depot/main/Destiny/main/src/etc/eclipse/destiny-code-templates.xml#2 $:
 */
class IPCMessageChannel
{
public:

    IPCMessageChannel(HANDLE hFileMapping, TCHAR* pSharedMemory, HANDLE hSendEvent, HANDLE hReceiveEvent);

    ~IPCMessageChannel(void);

    void Close ();

private:
    HANDLE m_hFileMapping;
    TCHAR* m_pSharedMemory;
    HANDLE m_hSendEvent;
    HANDLE m_hReceiveEvent;
public:
    TCHAR* GetSharedMemory(void);
    HANDLE GetSendEvent ();
    HANDLE GetReceiveEvent ();
};

#endif