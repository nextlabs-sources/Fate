//
// All sources, binaries and HTML pages (C) copyright 2006 by Blue Jungle., 
// Redwood City CA, Ownership remains with Blue Jungle Inc, 
// All rights reserved worldwide. 
// 
// Author : Dominic Lam
// Date   : 5/23/2006
// Note   : This file is to match the constants and defines from 
//          - IPCConstants.java 
//          - CMRequestHandler.java
//          Where it mostly model the code based on Window
// 
// $Id$
//
// Best viewed by 80-column terminal
// <-------------------------------------------------------------------------->

#ifndef IPCCONSTANTS_H
#define IPCCONSTANTS_H

// This is for backward compatibility with the existing Window/Java code

typedef enum {
  IPC_WAIT_FAILED   = -1,        // Generic fail
  IPC_WAIT_SUCCESS  = 0x0,       // Wait successfully
  IPC_WAIT_ABANDOED = 0x80,      // Mutex is released by a different thread
  IPC_WAIT_TIMEOUT  = 0x102      // Wait timed out
} IPC_waitResult_t;

// This is defined in brain.h already.  Redefining here for convenience
#ifndef BJ_WIDE_CHAR
#ifdef Linux
#define BJ_WIDE_CHAR
#else
#define BJ_WIDE_CHAR L
#endif
#endif


// Names for the IPC Shared structure

#define IPC_SHARED_MEMORY_SUFFIX    "SHAREDMEM"
#define IPC_RECEIVE_EVENT_SUFFIX    "RECEIVE_EVENT"
#define IPC_SEND_EVENT_SUFFIX       "SEND_EVENT"
#define IPC_MUTEX_SUFFIX            "HANDSHAKE_MUTEX"
#define IPC_CONNECT                 "CONNECT"
#define IPC_DISCONNECT              "DISCONNECT"
#define IPC_HANDSHAKE_FORMAT        "%s\n%s\n%s\n%s"

#define IPC_CHANNEL_SIZE            8192

// Name of the services. Must match CMRequestHandler definition

// Agent Event between Java and Native code
#define STOP_AGENT_EVENT            "d65e9670-b7fd-4f91-8228-0f5e4e1cd9a2"

// Stop the Agent
#define STOP_AGENT_SERVICE          "stopAgentService"
#define GET_STOP_AGENT_CHALLENGE    "getShutdownChallenge"
#define CM_REQUEST_HANDLER_CLASS    "com.bluejungle.destiny.agent.controlmanager.CMRequestHandler"
#define STOP_PASSWORD_ARG_INDEX     0
#define STOP_USER_ARG_INDEX         1
#define STOP_HOST_ARG_INDEX         2

//  Query the Policy engine 
#define QUERY_DECISION_ENGINE       BJ_WIDE_CHAR"queryDecisionEngine"
#define METHOD_NAME_INDEX           0
#define FROM_FILE_NAME_INDEX        1
#define ACTION_INDEX                2
#define SID_INDEX                   3
#define APPLICATION_NAME_INDEX      4
#define HOST_NAME_INDEX             5
#define TO_FILE_NAME_INDEX          6
#define IGNORE_OBLIGATION_INDEX     7
#define SHARE_NAME_INDEX            8
#define PID_INDEX                   9
#define LOG_LEVEL_INDEX             10
#define ALLOW_STR                   "ALLOW"
#define TRUE_STR                    "true"

// Agent type used by the AgentTypeEnumType
#define AGENT_TYPE_DESKTOP          0
#define AGENT_TYPE_FILESERVER       1

// Type of Socket communication
// Socket type for kernel communication
// This match the IPCLinuxKernelStub as well as the kernelInterface.h
#define BJ_KERNELPEP                31

#define BJID_GROUPID_PREFIX         "BJID_GROUPID"


//[jzhang 110706]  Noise Level (to match the definition from java side)
#define LOG_LEVEL_SYSTEM            1
#define LOG_LEVEL_APPLICATION       2
#define LOG_LEVEL_USER              3


#endif  /* IPCCONSTANTS_H */
