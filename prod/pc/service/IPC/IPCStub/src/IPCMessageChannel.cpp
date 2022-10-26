// All sources, binaries and HTML pages (C) copyright 2004 by Blue Jungle Inc., 
// Redwood City CA, Ownership remains with Blue Jungle Inc, 
// All rights reserved worldwide. 

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

#include "StdAfx.h"
#include "..\..\shared\src\globals.h"
#include "..\..\shared\src\ilog.h"
#include "..\..\shared\src\logfactory.h"
#include "ipcmessagechannel.h"

IPCMessageChannel::IPCMessageChannel(HANDLE hFileMapping, TCHAR* pSharedMemory, HANDLE hSendEvent, HANDLE hReceiveEvent)
{
    ILog* pLog = LogFactory::GetLogger ();

    if (hFileMapping != 0)
    {
        m_hFileMapping = hFileMapping;
    }
    else 
    {
        pLog->Error (_T("Invalid file mapping"));
    }
    if (pSharedMemory)
    {
        m_pSharedMemory = pSharedMemory;
    }
    else 
    {
        pLog->Error (_T("Shared Memory Pointer is NULL"));
    }
    if (hSendEvent != 0)
    {
    m_hSendEvent = hSendEvent;
    }
    else 
    {
        pLog->Error (_T("Invalid Send Event"));
    }
    if (hReceiveEvent != 0)
    {
        m_hReceiveEvent = hReceiveEvent;
    }
    else 
    {
        pLog->Error (_T("Invalid Receive Event"));
    }
}

IPCMessageChannel::~IPCMessageChannel(void)
{
}

/**
 * Return shared memory pointer
 */
TCHAR* IPCMessageChannel::GetSharedMemory(void)
{
    return m_pSharedMemory;
}

/**
 * Return send event handle
 */
HANDLE IPCMessageChannel::GetSendEvent(void)
{
    return m_hSendEvent;
}

/**
 * Return receive event handle
 */
HANDLE IPCMessageChannel::GetReceiveEvent(void)
{
    return m_hReceiveEvent;
}

/**
 * Closes all OS handles for the message channel
 */
void IPCMessageChannel::Close ()
{
    ::UnmapViewOfFile (m_pSharedMemory);
    ::CloseHandle (m_hFileMapping);
    ::CloseHandle (m_hSendEvent);
    ::CloseHandle (m_hReceiveEvent);
}
