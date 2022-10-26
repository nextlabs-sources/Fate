// WDE_CA.h : main header file for the WDE_CA DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols
#include <msi.h>
#include <Msiquery.h.>


// CWDE_CAApp
// See WDE_CA.cpp for the implementation of this class
//

class CWDE_CAApp : public CWinApp
{
public:
	CWDE_CAApp();

// Overrides
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
UINT fnSayHello(MSIHANDLE hInstall);
UINT fnShutdown20ComplianceAgentService(MSIHANDLE hInstall);
UINT fnQueryServiceStatus(MSIHANDLE hInstall, TCHAR * tchServiceName,DWORD * dwServiceStatus);
void fnWriteToInstallerLogFile(MSIHANDLE hInstall, TCHAR * tchMessage);
UINT fnControlService(MSIHANDLE hInstall,TCHAR * tchServiceName, DWORD dwControl);
UINT fnStream20AgentControllerToTemp( MSIHANDLE hInstall);
UINT fnClose20ComplianceAgentNotify(MSIHANDLE hInstall);
UINT fnFind20ComplianceAgentNotify(MSIHANDLE hInstall);
UINT fnFindDestiny20(MSIHANDLE hInstall);
UINT fnCleanup20TempFiles(MSIHANDLE hInstall);
UINT fnDelete20EnforcerDriver(MSIHANDLE hInstall);
UINT fnInstallEnforcerDriver(MSIHANDLE hInstall);
UINT fnBackupEnforcerDriver(MSIHANDLE hInstall);
UINT fnRestoreEnforcerDriver(MSIHANDLE hInstall);
UINT fnStartEnforcerDriver(MSIHANDLE hInstall);
UINT fnStopEnforcerDriver(MSIHANDLE hInstall);
UINT fnDeleteEnforcerDriver(MSIHANDLE hInstall);
