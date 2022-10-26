//
// All sources, binaries and HTML pages (C) copyright 2006 by Blue Jungle., 
// Redwood City CA, Ownership remains with Blue Jungle Inc, 
// All rights reserved worldwide. 
// 
// Author : Dominic Lam
// Date   : 5/23/2006
// Note   : This file is to match the constants and defines from the Java code
//          - IPCConstants.java 
//          - CMRequestHandler.java
//          Where it mostly model the code based on Window
// 
//  The origin of the file is called IPCConstants, but it's a misnomer
//  since we aren't using IPC in the new stuff.  So, it's now called
//  JavaConstant because this file share the constants between Java/Native
// 
// Best viewed by 80-column terminal
// <-------------------------------------------------------------------------->

#ifndef JAVACONSTANTS_H
#define JAVACONSTANTS_H

// This is for backward compatibility with the existing Window/Java code

typedef enum {
  IPC_WAIT_FAILED   = -1,        // Generic fail
  IPC_WAIT_SUCCESS  = 0x0,       // Wait successfully
  IPC_WAIT_ABANDOED = 0x80,      // Mutex is released by a different thread
  IPC_WAIT_TIMEOUT  = 0x102      // Wait timed out
} IPC_waitResult_t;

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
#define STOP_AGENT_VERIFY           "StopAgentVerify"
#define GET_STOP_AGENT_CHALLENGE    "getShutdownChallenge"
#define CM_REQUEST_HANDLER_CLASS    "com.bluejungle.destiny.agent.controlmanager.CMRequestHandler"
#define STOP_PASSWORD_ARG_INDEX     0
#define STOP_USER_ARG_INDEX         1
#define STOP_HOST_ARG_INDEX         2

//  Query the Policy engine 
#define QUERY_DECISION_ENGINE       _T("queryDecisionEngine")
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


//[jzhang 110706]  Noise Level (to match the definition from java side)
#define LOG_LEVEL_SYSTEM            1
#define LOG_LEVEL_APPLICATION       2
#define LOG_LEVEL_USER              3


// Java Classes that we need to call into the JVM

#define MAX_JVM_OPTIONS 20      // Maxiumium number of options to JVM
#define MAX_APP_OPTIONS 20      // Maxiumium number of options to app (CM)

// Java Classes to be used
#define CM_MAIN_CLASS     "com/bluejungle/destiny/agent/controlmanager/ControlMngr"
#define CM_EVENTMAN_CLASS "com/bluejungle/destiny/agent/controlmanager/SCMEventManager"
#define CONTROLMGR_STUB_CLASS "com/bluejungle/destiny/agent/controlmanager/ControlManagerStub"

// Java functions to be used

#define CM_MAIN_MAIN_M              "main"
#define CM_MAIN_ISINITIALIZED_M     "isInitialized"
#define CM_EVENTMAN_GETINSTANCE_M   "getInstance"
#define CM_EVENTMAN_DISPATCH_M      "dispatchSCMEvent"
#define CM_STUB_GETINSTANCE_M       "getInstance"
#define CM_GETSERVERSTUB_M          "getServerStub"
#define SERVER_STUB_RUN_M           "run"
#define SERVER_STUB_EVAL_M          "doPolicyEvaluation"
#define SERVER_STUB_MULTI_EVAL_M    "doMultiPolicyEvaluation"
#define CM_HANDLE_LOGON_EVENT_M     "handleLogonEvent"
#define CM_HANDLE_LOGOFF_EVENT_M    "handleLogoffEvent"

// Argument to Java Control Module
#define FILESERVER_ARG              "FileServer"

/* Make sure this table is in sync with the CEsdk.h action enumeration */
/* The actions and corresponding strings need to be sync in the following
   files: JavaConstans.h, PDPEval.cpp, eval.cpp, CEsdk.h. */
//action strings shared by C++ and Java
#define CE_ACTION_STRING_READ                    _T("OPEN")
#define CE_ACTION_STRING_DELETE                  _T("DELETE")
#define CE_ACTION_STRING_MOVE                    _T("MOVE")
#define CE_ACTION_STRING_COPY                    _T("COPY")
#define CE_ACTION_STRING_WRITE                   _T("EDIT")
#define CE_ACTION_STRING_RENAME                  _T("RENAME")
#define CE_ACTION_STRING_CHANGE_ATTR_FILE        _T("CHANGE_ATTRIBUTES")
#define CE_ACTION_STRING_CHANGE_SEC_FILE         _T("CHANGE_SECURITY")
#define CE_ACTION_STRING_PRINT_FILE              _T("PRINT")
#define CE_ACTION_STRING_PASTE_FILE              _T("PASTE")
#define CE_ACTION_STRING_EMAIL_FILE              _T("EMAIL")
#define CE_ACTION_STRING_IM_FILE                 _T("IM")
#define CE_ACTION_STRING_EXPORT                  _T("EXPORT")
#define CE_ACTION_STRING_IMPORT                  _T("IMPORT")
#define CE_ACTION_STRING_UPLOAD                  _T("UPLOAD")
#define CE_ACTION_STRING_DOWNLOAD                _T("DOWNLOAD")
#define CE_ACTION_STRING_CHECKIN                 _T("CHECKIN")
#define CE_ACTION_STRING_CHECKOUT                _T("CHECKOUT")
#define CE_ACTION_STRING_ATTACH                  _T("ATTACH") // Well, well, well, the policy is thinking upload and attach 
                                                              // is the same, which according to PM is not.  So, we hide this
                                                              // mess away from the 3rd party developer
//On Java side, "UPLOAD" is not treated as valid action. This is really mess!
#define CE_ACTION_STRING_RUN                      _T("RUN")
#define CE_ACTION_STRING_REPLY                    _T("REPLY")
#define CE_ACTION_STRING_FORWARD                  _T("FORWARD")
#define CE_ACTION_STRING_NEW_EMAIL                _T("NEW_EMAIL")
#define CE_ACTION_STRING_MEETING                  _T("MEETING")
#define CE_ACTION_STRING_AVD                      _T("AVDCALL")
#define CE_ACTION_STRING_PROCESS_TERMINATE        _T("PROC_TERMINATE")
#define CE_ACTION_STRING_WM_SHARE                 _T("SHARE")
#define CE_ACTION_STRING_WM_RECORD                _T("RECORD")
#define CE_ACTION_STRING_WM_QUESTION              _T("QUESTION")
#define CE_ACTION_STRING_WM_VOICE                 _T("VOICE")
#define CE_ACTION_STRING_WM_VIDEO                 _T("VIDEO")
#define CE_ACTION_STRING_WM_JOIN                  _T("JOIN")

#endif  /* JAVACONSTANTS_H */
