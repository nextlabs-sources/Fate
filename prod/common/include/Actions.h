/*
* Actions.h
* Author: Fuad Rashid
* All sources, binaries and HTML pages (C) copyright 2004 by Blue Jungle Inc., 
* Redwood City CA, Ownership remains with Blue Jungle Inc, 
* All rights reserved worldwide. 
*/

#ifndef _ACTIONS_H_
#define _ACTIONS_H_

#define MAX_ACTION_NAME_SIZE	32

#define OPEN_NAME               L"OPEN"
#define EDIT_NAME               L"EDIT"
#define DELETE_NAME             L"DELETE"
#define READ_NAME               L"READ"
#define WRITE_NAME              L"WRITE"
#define CLOSE_NAME              L"CLOSE"
#define RENAME_NAME             L"RENAME"
#define CREATE_NEW_NAME         L"EDIT"
#define CHANGE_PROPERTIES_NAME  L"CHANGE_ATTRIBUTES"
#define CHANGE_SECURITY_NAME    L"CHANGE_SECURITY"
#define EDIT_COPY_NAME          L"EDIT_COPY"
#define SEND_IM_NAME            L"IM"
#define CUT_PASTE_NAME          L"CUT_PASTE"
#define COPY_PASTE_NAME         L"PASTE"
#define BATCH_NAME              L"BATCH"
#define BURN_NAME               L"BURN"
#define PRINT_NAME              L"PRINT"
#define COPY_NAME               L"COPY"
#define MOVE_NAME               L"MOVE"
#define SHARE_NAME              L"SHARE"
#define EMAIL_NAME              L"EMAIL"
#define EMBED_NAME              L"EMBED"
#define STOP_AGENT_NAME         L"STOP_AGENT"
#define EXECUTE_NAME		L"EXECUTE"
#define UNKNOWN_NAME            L"UNKNOWN"

//The following action table has to be consistant with CEAction in CEsdk.h
#define OPEN_READ_ACTION         1
#define READ_ACTION              1
#define DELETE_ACTION            2
#define MOVE_ACTION              3
#define COPY_ACTION              4
#define OPEN_READ_WRITE_ACTION   1
#define WRITE_ACTION             5
#define RENAME_ACTION            6
#define CHANGE_PROPERTIES_ACTION 7
#define CHANGE_SECURITY_ACTION   8
#define CUT_PASTE_ACTION         10
#define COPY_PASTE_ACTION        10
#define EXECUTE_ACTION           18
#define CREATE_NEW_ACTION        5
#define SAVE_AS_ACTION           5
#define SEND_IM_ACTION           12
#define EMAIL_ACTION             11
#define OPEN_DIR_ACTION          1
#define PRINT_ACTION             9
//The following actions are not defined in CE_Action but defined in PF
#define EMBED_ACTION             0x400000
#define STOP_AGENT_ACTION        0x800000
//The following actions are not defined
#define CLOSE_ACTION             0x0020
#define BATCH_ACTION             0x4000
#define BURN_ACTION              0x8000
#define SHARE_ACTION             0x80000

// Definitions for Linux removable device and network enforcers, for Aeroflex Demo
#define MOUNT_ACTION				0x2000000
#define NETWORK_ACCESS				0x4000000

//for ulAllow
#define DENY                    1
#define ALLOW                   2

//for ulAllowType
#define NOT_WATCHED             1 //allow all operations on file for all users/all actions
#define WATCH_NEXT_OP           2 // send next op to policy engine
#define ALLOW_UNTIL_CLOSE       3 // ignore all subsequent actions of the same type for the open file

#endif
