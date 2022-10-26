// Installer_CA.h : main header file for the Installer_CA DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols
#include <msi.h>
#include <msiquery.h>
#include <afxtempl.h>

#define POLICY_LOCATION_PROP_NAME TEXT("POLICY_SERVER_LOCATION")
#define MACHINE_NAME_TOKEN 		TEXT("[MACHINE_NAME]")
#define NEED_DISCOVERY_PROP_NAME TEXT("_Need_Policy_Server_Discovery")
#define DEFAULT_POLICY_SERVER_PORT 8443
#define PORT_TOKEN 	TEXT("[DPS_PORT]")
#define COMBOBOX_INDEX_COLUMN 	2
#define MAX_RECORD 				20
#define MAX_KEY_LENGTH 255
#define MAX_VALUE_NAME 16383
#define DEFAULT_EMBEDDED_DB_PORT 5432
#define DEFAULT_SMTP_PORT 25
#define DEFAULT_WEB_APPLICATION_PORT 443
#define DEFAULT_WEB_SERVICE_PORT 8443
#define MAX_LOOKUP_RETRY 3
#define DISCOVER_SERVER_INSTANCES 1
#define DISCOVER_POLICY_SERVER_INSTANCES 2
#define DISCOVER_ICENET_SERVER_INSTANCES 3
#define MAX_SEARCH_TIME 5000
#define MAX_RECV_BUF_SIZE 500




// CInstaller_CAApp
// See Installer_CA.cpp for the implementation of this class
//

class CInstaller_CAApp : public CWinApp
{
public:
	CInstaller_CAApp();

// Overrides
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};

// list of whatever type of servers
CList<CString, CString&> listServers;


UINT fnCheckMajorUpgrade(MSIHANDLE hInstall);
bool fnPackUpgradeCode(TCHAR * tchUpgradeCode,TCHAR * tchPackedUpgradeCode);
bool fnGetUpgradeProductCode(UINT uKey,TCHAR * tchKey, TCHAR * tchProductCode);
bool fnUnpackProductCode(TCHAR * tchPackedProductCode, TCHAR * tchUnpackedProductCode);
void fnRetrieveProperty(MSIHANDLE hInstall,TCHAR * tchProperyName,TCHAR * tchPropertyValue );
void fnStoreProperty(MSIHANDLE hInstall,TCHAR * tchProperyName,TCHAR * tchPropertyValue );
void fnWriteToInstallerLogFile(MSIHANDLE hInstall, TCHAR * tchMessage);
bool fnReverse(CString * strBuffer);
bool fnReverseTwos(CString * strBuffer);
UINT fnDiscoverPolicyServerLocations(MSIHANDLE hInstall);
int fnGetLastIndex(MSIHANDLE hView,int iIndexPosition);
void fnAddComboBoxEntry(MSIHANDLE hView, TCHAR * tchProperty, TCHAR * tchText, TCHAR * tchValue, int iIndex);
int fnGetNumCols(MSIHANDLE hView);
UINT fnDiscoverServers (TCHAR tchServers, int iDiscoveryType, int iResultSize);
void fnDiscover(MSIHANDLE hInstall,int iDiscoveryType, int iMaxSearchTime);
bool fnPolicyServerParsed(MSIHANDLE hInstall,CString strListItem,CString * strBuffer);