// All sources, binaries and HTML pages (C) copyright 2004 by Blue Jungle Inc., 
// Redwood City CA, Ownership remains with Blue Jungle Inc, 
// All rights reserved worldwide. 

/**
 * 
 * @author fuad
 * @version $Id:
 *          //depot/main/Destiny/main/src/etc/eclipse/destiny-code-templates.xml#2 $:
 */

#ifndef _IPCCONSTANTS_H_
#define _IPCCONSTANTS_H_

#define CONNECT _T("CONNECT")
#define DISCONNECT _T("DISCONNECT")
#define HANDSHAKE_STR _T("%s\n%s\n%s\n%s")
#define SHARED_MEMORY_SUFFIX _T("SHAREDMEM")
#define SEND_EVENT_SUFFIX _T("SEND_EVENT")
#define RECEIVE_EVENT_SUFFIX _T("RECEIVE_EVENT")
#define MUTEX_SUFFIX _T("HANDSHAKE_MUTEX")
#define CHANNEL_SIZE 8192
#define TIMEOUT 1000
#define GLOBAL_NAME_PREFIX _T("Global\\")
#define GLOBAL_NAME_PREFIX_LENGTH 8

#define RESPONSE_OPEN_TAG _T("<response>")
#define RESPONSE_CLOSE_TAG _T("</response>")
#define PARAM_OPEN_TAG _T("<param value=\"")
#define PARAM_CLOSE_TAG _T("\"/>")

#endif