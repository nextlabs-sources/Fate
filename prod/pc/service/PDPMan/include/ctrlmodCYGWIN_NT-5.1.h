//
// All sources, binaries and HTML pages (C) copyright 2007 by NextLabs Inc, 
// Redwood City CA, Ownership remains with Blue Jungle Inc, 
// All rights reserved worldwide. 
// 
// Date   : 6/19/2007
// Note   : Integrating the old code to the new platform for the  
//          tamper-resistance
// 

#ifndef _CTRLMODCYGWIN_NT_5_1_H
#define _CTRLMODCYGWIN_NT_5_1_H

// PDPman, things that we may want to config in the future

namespace ctrlmod_key {
//Constant
#define SZ_EVENT_VIEWER_SOURCE   _T("Compliant Enterprise")
#define EVENT_VIEWER_KEY         _T("SYSTEM\\CurrentControlSet\\Services\\Eventlog\\Application\\Compliant Enterprise")
#define EVENT_MESSAGE_DLL        _T("public_bin\\EventMessages.dll")

// internal name of the service
#define SZSERVICENAME            _T("ComplianceAgentService")
// displayed name of the service
#define SZSERVICEDISPLAYNAME     _T("Compliance Agent Service")
// list of service dependencies - "dep1\0dep2\0\0"
#define SZDEPENDENCIES           _T("\0\0")
// Service TYPE
#define SERVICESTARTTYPE         SERVICE_AUTO_START 

// Path to Parameter Key
#define SZPARAMKEY               "SYSTEM\\CurrentControlSet\\Services\\ComplianceAgentService\\Parameters"
#define SZPARAMKEY_TEMPLATE      "SYSTEM\\CurrentControlSet\\Services\\%s\\Parameters"
// name of the executable
#define SZAPPNAME               _T("pdpman")
}

namespace ctrlmod_win {
VOID AddToMessageLog(LPTSTR lpszMsg);

}


#endif
