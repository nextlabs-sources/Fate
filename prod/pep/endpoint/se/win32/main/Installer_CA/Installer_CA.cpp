// Installer_CA.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "Installer_CA.h"
#include <winsock2.h>
#pragma warning( push )
#pragma warning( disable : 6386 )
#include <ws2tcpip.h>
#pragma warning( pop )
//#include "DiscoveryClient.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//
//	Note!
//
//		If this DLL is dynamically linked against the MFC
//		DLLs, any functions exported from this DLL which
//		call into MFC must have the AFX_MANAGE_STATE macro
//		added at the very beginning of the function.
//
//		For example:
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// normal function body here
//		}
//
//		It is very important that this macro appear in each
//		function, prior to any calls into MFC.  This means that
//		it must appear as the first statement within the 
//		function, even before any object variable declarations
//		as their constructors may generate calls into the MFC
//		DLL.
//
//		Please see MFC Technical Notes 33 and 58 for additional
//		details.
//

// CInstaller_CAApp

BEGIN_MESSAGE_MAP(CInstaller_CAApp, CWinApp)
END_MESSAGE_MAP()


// CInstaller_CAApp construction

CInstaller_CAApp::CInstaller_CAApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CInstaller_CAApp object

CInstaller_CAApp theApp;


// CInstaller_CAApp initialization

BOOL CInstaller_CAApp::InitInstance()
{
	CWinApp::InitInstance();

	return TRUE;
}



//////////////////////////////////////////////////////////////////////////////////
//	Function Name: fnCheckMajorUpgrade(MSIHANDLE hInstall)
//	Purpose: Checks if this is a major upgrade 
//	Author: Michael Byrns
//  History: Created 11 September 2007
//////////////////////////////////////////////////////////////////////////////////
UINT fnCheckMajorUpgrade(MSIHANDLE hInstall)
{
	HKEY hKey;
	TCHAR tchCUKey[MAX_PATH] = TEXT("Software\\Microsoft\\Installer\\UpgradeCodes\\");
	TCHAR tchLMKey[MAX_PATH] = TEXT("Software\\Classes\\Installer\\UpgradeCodes\\");
	TCHAR tchUpgradeCode[MAX_PATH];
	TCHAR tchProductCode[MAX_PATH];
	TCHAR tchUnpackedProductCode[MAX_PATH];
	TCHAR tchPackedUpgradeCode[MAX_PATH];
	TCHAR tchPackedProductCode[MAX_PATH];
	DWORD dwBufferSize;
	UINT uResult;
	bool bUpgradeCodeFound = false;


	dwBufferSize = MAX_PATH;
	uResult = MsiGetProperty(hInstall,TEXT("UpgradeCode"),tchUpgradeCode,&dwBufferSize);
	if (uResult != ERROR_SUCCESS )
		return ERROR_SUCCESS;

	if ( !fnPackUpgradeCode(tchUpgradeCode,tchPackedUpgradeCode))
		return ERROR_SUCCESS;

	dwBufferSize = MAX_PATH;
	uResult = MsiGetProperty(hInstall,TEXT("ProductCode"),tchProductCode,&dwBufferSize);
	if (uResult != ERROR_SUCCESS )
		return ERROR_SUCCESS;


	lstrcat(tchCUKey,tchPackedUpgradeCode);
	if (RegOpenKeyEx( HKEY_CURRENT_USER, tchCUKey, 0, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS)
	{
		bUpgradeCodeFound = true;
		RegCloseKey(hKey);
		fnGetUpgradeProductCode(1,tchCUKey,tchPackedProductCode);
	}

	lstrcat(tchLMKey,tchPackedUpgradeCode);
	if (RegOpenKeyEx( HKEY_LOCAL_MACHINE, tchLMKey, 0, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS)
	{
		fnGetUpgradeProductCode(2,tchLMKey,tchPackedProductCode);
		bUpgradeCodeFound = true;
		RegCloseKey(hKey);
	}

	if ( bUpgradeCodeFound )
	{
		if ( !fnUnpackProductCode(tchPackedProductCode, tchUnpackedProductCode))
			return ERROR_SUCCESS;
		if ( lstrcmp(tchUnpackedProductCode,tchProductCode) != 0  )
		{
			MsiSetProperty(hInstall,TEXT("MAJOR_UPGRADE"),TEXT("1"));
			fnStoreProperty(hInstall,TEXT("PropertyCode"),tchUnpackedProductCode );
			return ERROR_SUCCESS;
		}
		else 
		{
			return ERROR_SUCCESS;
		}
	}
	else 
	{
		return ERROR_SUCCESS;
	}

//	return ERROR_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////////
//	Function Name: fnPackUpgradeCode(TCHAR * tchUpgradeCode,TCHAR * tchPackedUpgradeCode)
//	Purpose: Converts Upgrade code to packed format for registry lookups
//	Author: Michael Byrns
//  History: Created 11 September 2007
//////////////////////////////////////////////////////////////////////////////////
bool fnPackUpgradeCode(TCHAR * tchUpgradeCode,TCHAR * tchPackedUpgradeCode)
{
	TCHAR * tchToken;
	TCHAR tchSeps[] = TEXT("-");
//	int iLength = lstrlen(tchUpgradeCode);
	CString strBuffer,strSubBuffer1,strSubBuffer2,strPackedProductCode;
	int iTokenCount = 0;

	// Establish string and get the first token: 
	TCHAR * nexttoken = NULL;

	tchToken = strtok_s( tchUpgradeCode, tchSeps, &nexttoken);
	while( tchToken != NULL )
   {
	   	strBuffer = tchToken;
		switch (iTokenCount)
		{
			case 0:
				strBuffer.Delete(0,1);
				fnReverse(&strBuffer);
				break;
			case 1:
				fnReverse(&strBuffer);
				break;
			case 2:
				fnReverse(&strBuffer);
				break;
			case 3:
				fnReverseTwos(&strBuffer);;
				break;
			case 4:
				strBuffer.Delete(strBuffer.GetLength()-1,1);
				fnReverseTwos(&strBuffer);;
				break;
		}
		iTokenCount++;
		strPackedProductCode += strBuffer;

		tchToken = strtok_s( NULL, tchSeps, &nexttoken); 
   }
   lstrcpy(tchPackedUpgradeCode,strPackedProductCode);

	return true;
}

//////////////////////////////////////////////////////////////////////////////////
//	Function Name: bool fnGetUpgradeProductCode(UINT uKey,TCHAR * tchKey, TCHAR * tchProductCode)
//	Purpose: Retrieve the product code under the upgrade code
//	Author: Michael Byrns
//  History: Created 11 September 2007
//////////////////////////////////////////////////////////////////////////////////
bool fnGetUpgradeProductCode(UINT uKey,TCHAR * tchKey, TCHAR * tchProductCode)
{
	tchProductCode[0] = '\0';
	int iIndex = 0;
	DWORD dwValueSize = MAX_PATH;
	HKEY hKey;

	if ( uKey == 1 )
	{
		if (RegOpenKeyEx( HKEY_CURRENT_USER, tchKey, 0, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS)
		{
			while (RegEnumValue(hKey, iIndex, 
						tchProductCode, 
						&dwValueSize, 
						NULL, 
						NULL,
						NULL,
						NULL) == ERROR_SUCCESS )
			{
				dwValueSize = MAX_PATH;
				iIndex++;
			}

			RegCloseKey(hKey);
		}
		else
			return false;
	}
	else
	{
		if (RegOpenKeyEx( HKEY_LOCAL_MACHINE, tchKey, 0, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS)
		{
			while (RegEnumValue(hKey, iIndex, 
						tchProductCode, 
						&dwValueSize, 
						NULL, 
						NULL,
						NULL,
						NULL) == ERROR_SUCCESS )
			{
				dwValueSize = MAX_PATH;
				iIndex++;
			}
			RegCloseKey(hKey);
		}
		else
			return false;
	}

	return true;
}
//////////////////////////////////////////////////////////////////////////////////
//	Function Name: bool fnUnpackProductCode(TCHAR * tchPackedProductCode, TCHAR * tchUnpackedProductCode)
//	Purpose: Convert a packed product code to unpacked for comparison
//	Author: Michael Byrns
//  History: Created 11 September 2007
//////////////////////////////////////////////////////////////////////////////////
bool fnUnpackProductCode(TCHAR * tchPackedProductCode, TCHAR * tchUnpackedProductCode)
{
	CString strBuf1,strBuf2,strUnPackedProductCode;
	strBuf1 = tchPackedProductCode;
	int iLength;
	// 0050 14B3 2081 E884 E91F B411 99E2 4004

	if ( strBuf1.GetLength() < 32 )
	{
		return false;
	}
	strBuf2 = strBuf1.Left(8);
	iLength = strBuf2.GetLength() - 1;
	for ( int iIndex = iLength; iIndex >= 0; iIndex--)
		strUnPackedProductCode += strBuf2.GetAt(iIndex);
	strUnPackedProductCode += TEXT("-");
 	strBuf1.Delete(0,8);

	for ( int iLoop = 0; iLoop < 2; iLoop++ )
	{
		strBuf2 = strBuf1.Left(4);
		iLength = strBuf2.GetLength() - 1;
		for ( int iIndex = iLength; iIndex >= 0; iIndex--)
			strUnPackedProductCode += strBuf2.GetAt(iIndex);
		strUnPackedProductCode += TEXT("-");
		strBuf1.Delete(0,4);
	}
	for ( int iLoop = 0; iLoop < 2; iLoop++ )
	{
		strUnPackedProductCode += strBuf1.GetAt(1);
		strUnPackedProductCode += strBuf1.GetAt(0);
		strBuf1.Delete(0,2);
	}

	strUnPackedProductCode += TEXT("-");

	for ( int iLoop = 0; iLoop < 6; iLoop++ )
	{
		strUnPackedProductCode += strBuf1.GetAt(1);
		strUnPackedProductCode += strBuf1.GetAt(0);
		strBuf1.Delete(0,2);
	}


	strUnPackedProductCode.Insert(0,TEXT("{"));
	strUnPackedProductCode += TEXT("}");

	lstrcpy(tchUnpackedProductCode,strUnPackedProductCode);

	return true;
}

//////////////////////////////////////////////////////////////////////////////////
//	Function Name: fnRetrieveProperty(MSIHANDLE hInstall,TCHAR * tchProperyName,TCHAR * tchPropertyValue)
//	Purpose: Retrieve a stored propery for use primarily in the upgrade
//	Author: Michael Byrns
//  History: Created 7 September 2007
//////////////////////////////////////////////////////////////////////////////////
void fnRetrieveProperty(MSIHANDLE hInstall,TCHAR * tchProperyName,TCHAR * tchPropertyValue )
{

	HKEY hKey;
	TCHAR tchKey[MAX_PATH] = TEXT("Software\\Classes\\Installer\\Properties\\");
	DWORD dwBufferSize,dwType;
	TCHAR tchProductCode[MAX_PATH];
	UINT uResult;

	dwBufferSize = MAX_PATH;
	uResult = MsiGetProperty(hInstall,TEXT("ProductCode"),tchProductCode,&dwBufferSize);

	if (uResult != ERROR_SUCCESS )
	{
		fnWriteToInstallerLogFile(hInstall, TEXT("fnRetrieveProperty MsiGetProperty[1] failed"));
		return;
	}
	lstrcat(tchKey,tchProductCode);

	if (RegOpenKeyEx( HKEY_LOCAL_MACHINE, tchKey, 0, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS)
	{
		dwBufferSize = MAX_PATH;
		if ( RegQueryValueEx( hKey, tchProperyName, 0, &dwType, (LPBYTE)tchPropertyValue, &dwBufferSize) != ERROR_SUCCESS)
			fnWriteToInstallerLogFile(hInstall, TEXT("fnRetrieveProperty RegQueryValueEx failed"));
		RegCloseKey(hKey);
	}
	else
		fnWriteToInstallerLogFile(hInstall, TEXT("fnRetrieveProperty RegOpenKeyEx failed"));
}

//////////////////////////////////////////////////////////////////////////////////
//	Function Name: fnStoreProperty(MSIHANDLE hInstall,TCHAR * tchProperyName,TCHAR * tchProperyValue)
//	Purpose: Stores a  property for use primarily in the upgrade
//	Author: Michael Byrns
//  History: Created 11 September 2007
//////////////////////////////////////////////////////////////////////////////////
void fnStoreProperty(MSIHANDLE hInstall,TCHAR * tchProperyName,TCHAR * tchPropertyValue )
{
	HKEY hKey;
	TCHAR tchKey[MAX_PATH + 100] = TEXT("Software\\Classes\\Installer\\Properties\\");//allocate enough memory for "lstrcat"
	DWORD  dwDisposition; 
	TCHAR tchProductCode[MAX_PATH];
	UINT uResult;
	DWORD dwBufferSize = MAX_PATH;

	uResult = MsiGetProperty(hInstall,TEXT("ProductCode"),tchProductCode,&dwBufferSize);

	if (uResult != ERROR_SUCCESS )
	{
		fnWriteToInstallerLogFile(hInstall, TEXT("fnStoreProperty MsiGetProperty[1] failed"));
		return;
	}
	lstrcat(tchKey,tchProductCode);


	if ( RegCreateKeyEx(HKEY_LOCAL_MACHINE,tchKey,0,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&hKey,&dwDisposition) == ERROR_SUCCESS)
	{
		if ( RegSetValueEx( hKey, tchProperyName, 0, REG_SZ, (LPBYTE)tchPropertyValue, lstrlen(tchPropertyValue)) != ERROR_SUCCESS)
			fnWriteToInstallerLogFile(hInstall, TEXT("fnStoreProperty RegSetValueEx failed"));
		RegCloseKey(hKey);
	}
	else
		fnWriteToInstallerLogFile(hInstall, TEXT("fnStoreProperty RegCreateKeyEx failed"));
}

//////////////////////////////////////////////////////////////////////////////////
//	Function Name: void fnWriteToInstallerLogFile(MSIHANDLE hInstall, TCHAR * tchMessage)
//	Purpose: Writes to the installer log file
//	Author: Michael Byrns
//  History: Created 1 October 2007
//////////////////////////////////////////////////////////////////////////////////
void fnWriteToInstallerLogFile(MSIHANDLE hInstall, TCHAR * tchMessage)
{
	PMSIHANDLE hRec;
	TCHAR tchBuffer[MAX_PATH];
	DWORD dwLength;
	TCHAR tchFullErrorMessage[MAX_PATH];

	lstrcpy(tchFullErrorMessage,TEXT("Custom Action Logging "));
	if (lstrlen(tchFullErrorMessage) + lstrlen(tchMessage) > MAX_PATH)
		return;
	else
		lstrcat(tchFullErrorMessage,tchMessage);
	dwLength = lstrlen(tchFullErrorMessage);
	if ( dwLength <= 0 )
		return;

	hRec = MsiCreateRecord(1);                  
	
	if (!hRec)
		return;

	MsiRecordSetString(hRec, 1, tchFullErrorMessage);        
	MsiRecordSetString(hRec, 0, TEXT("[1]"));       
	MsiFormatRecord(hInstall, hRec, tchBuffer, &dwLength); 
	MsiProcessMessage(hInstall,INSTALLMESSAGE_INFO, hRec);                 
}

//////////////////////////////////////////////////////////////////////////////////
//	Function Name: bool fnReverse(TCHAR * tchBuffer)
//	Purpose: Reverses the characters in a string
//	Author: Michael Byrns
//  History: Created 11 September 2007
//////////////////////////////////////////////////////////////////////////////////
bool fnReverse(CString * strBuffer)
{
	CString strBuf;
	int iLength = strBuffer->GetLength() - 1;
	for ( int iIndex = 0; iIndex <= iLength; iIndex++ )
		strBuf += strBuffer->GetAt(iLength - iIndex);

	*strBuffer = strBuf;
	
	return true;
}
//////////////////////////////////////////////////////////////////////////////////
//	Function Name: bool fnReverseTwos(CString * strBuffer)
//	Purpose: Reverses the characters in a string by twos 
//	Author: Michael Byrns
//  History: Created 11 September 2007
//////////////////////////////////////////////////////////////////////////////////
bool fnReverseTwos(CString * strBuffer)
{
	CString strBuf1,strBuf2;
	int iLength = strBuffer->GetLength() - 1;

	for ( int iIndex = 0; iIndex <= iLength/2; iIndex++ )
	{
		strBuf1 = strBuffer->Left(2);
		strBuf2 += strBuf1.GetAt(1);
		strBuf2 += strBuf1.GetAt(0);
		strBuffer->Delete(0,2);
	}
	*strBuffer = strBuf2;
	return true;
}

//////////////////////////////////////////////////////////////////////////////////
//	Function Name: fnGetLastIndex(MSIHANDLE hView,int iIndexPosition) 
//	Purpose: Returns the last index of the records for this view
//	Author: Michael Byrns
//  History: Created 7 September 2007
//////////////////////////////////////////////////////////////////////////////////
int fnGetLastIndex(MSIHANDLE hView,int iIndexPosition)
{
	UINT uResult;
	int iIndex, iNumOfCols,iTemp;
    uResult = MsiViewExecute(hView, NULL);
    iNumOfCols = fnGetNumCols(hView);
	PMSIHANDLE hRec = MsiCreateRecord(iNumOfCols);

    iIndex = 0;

    while (MsiViewFetch(hView,&hRec) != ERROR_NO_MORE_ITEMS)
	{
        iTemp = MsiRecordGetInteger(hRec, iIndexPosition);
        if (iTemp > iIndex)
			iIndex = iTemp;
	}
    return iIndex;
}
//////////////////////////////////////////////////////////////////////////////////
//	Function Name: fnAddComboBoxEntry(MSIHANDLE hView, TCHAR * tchProperty, 
//                                    TCHAR * tchText, TCHAR * tchValue, int iIndex)
//	Purpose: Temporarily modifies/updates the specified record
//	Author: Michael Byrns
//  History: Created 7 September 2007
//////////////////////////////////////////////////////////////////////////////////
void fnAddComboBoxEntry(MSIHANDLE hView, TCHAR * tchProperty, TCHAR * tchText, TCHAR * tchValue, int iIndex)
{
    PMSIHANDLE hRec = MsiCreateRecord(4);
    MsiRecordSetString(hRec,  1, tchProperty);
    MsiRecordSetInteger(hRec, 2, iIndex);
    MsiRecordSetString(hRec,  3, tchValue);
    MsiRecordSetString(hRec,  4, tchText);
    MsiViewModify(hView, MSIMODIFY_INSERT_TEMPORARY, hRec);

}

//////////////////////////////////////////////////////////////////////////////////
//	Function Name: fnGetNumCols(MSIHANDLE hView)
//	Purpose: Retrieves the number of columns of the current view
//	Author: Michael Byrns
//  History: Created 7 September 2007
//////////////////////////////////////////////////////////////////////////////////
int fnGetNumCols(MSIHANDLE hView)
{
	PMSIHANDLE hRec = MsiCreateRecord(MAX_RECORD);
	UINT uResult;
	int iRecVal = 1;
	int iColCtr = 0;
	uResult =  MsiViewGetColumnInfo(hView, MSICOLINFO_NAMES, &hRec);

	while (iRecVal != 0)
	{
       iColCtr += 1;
       iRecVal = MsiRecordDataSize(hRec, iColCtr);
	}

	iColCtr--;
	return iColCtr;
}
//////////////////////////////////////////////////////////////////////////////////
//	Function Name: fnDiscoverPolicyServerLocations(MSIHANDLE hInstall)
//	Purpose: Populates the combo box control with all available policy servers
//  Failure returns success to allow manual entry 
//	Author: Michael Byrns
//  History: Created 7 September 2007
//////////////////////////////////////////////////////////////////////////////////
UINT fnDiscoverPolicyServerLocations(MSIHANDLE hInstall)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	TCHAR tchSQLQuery[MAX_PATH] = TEXT("SELECT * FROM ComboBox WHERE Property='POLICY_SERVER_LOCATION'");
	//TCHAR tchServers[MAX_PATH] = TEXT("SAVAII:8443,SAVAIW:8442,SAVAIY:8441");
//	int iSize = 4096;
//	DWORD dwSize = MAX_PATH;
	UINT uResult;
	MSIHANDLE hDatabase;
	MSIHANDLE hView;
	int iIndex = 0;
	TCHAR tchPropertyName[] = TEXT("POLICY_SERVER_LOCATION");



	hDatabase = MsiGetActiveDatabase(hInstall);
	if (!hDatabase)
	{
		fnWriteToInstallerLogFile(hInstall, TEXT("fnDiscoverPolicyServerLocations MsiGetActiveDatabase failed"));
		return ERROR_SUCCESS;
	}

	uResult = MsiDatabaseOpenView(hDatabase, tchSQLQuery, &hView);
	if (uResult != ERROR_SUCCESS )
	{
		fnWriteToInstallerLogFile(hInstall, TEXT("fnDiscoverPolicyServerLocations MsiDatabaseOpenView failed"));
		MsiCloseHandle(hDatabase);
		return ERROR_SUCCESS;
	}

	iIndex = fnGetLastIndex(hView, COMBOBOX_INDEX_COLUMN);
	if ( iIndex != 0 )
	{
		fnWriteToInstallerLogFile(hInstall, TEXT("fnDiscoverPolicyServerLocations fnGetLastIndex failed"));
		MsiCloseHandle(hView);
		MsiCloseHandle(hDatabase);
		return ERROR_SUCCESS;
	}

	
	fnDiscover(hInstall,DISCOVER_POLICY_SERVER_INSTANCES, MAX_SEARCH_TIME);
	bool bFirstItem = true;
	TCHAR tchItem[MAX_PATH];
	if ( listServers.GetCount() > 0 )
	{
		POSITION pos = listServers.GetHeadPosition();
		for ( int i=0; i < listServers.GetCount(); i++ )
		{
			CString strBuffer;
			
			if ( fnPolicyServerParsed(hInstall,listServers.GetNext(pos),&strBuffer))
			{
				if ( bFirstItem )
				{
					MsiSetProperty(hInstall,TEXT("POLICY_SERVER_LOCATION"),strBuffer);
					bFirstItem = false;
				}
				lstrcpy(tchItem,strBuffer);
				fnAddComboBoxEntry(hView,tchPropertyName, tchItem, tchItem, iIndex);
				iIndex++;
			}
		}

		listServers.RemoveAll();
	}

			
  	MsiCloseHandle(hView);
	MsiCloseHandle(hDatabase);
	return ERROR_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////////
//	Function Name: void fnDiscover(MSIHANDLE hInstall,int iDiscoveryType, int iMaxSearchTime)
//                                    
//	Purpose: Performs the discovery via socket broadcast
//	Author: NextLabs, modified by Michael Byrns
//  History: Created 7 September 2007
//////////////////////////////////////////////////////////////////////////////////
void fnDiscover(MSIHANDLE hInstall,int iDiscoveryType, int iMaxSearchTime)
{
	SOCKET      sockClient;
    TCHAR		tchBroadcastAddr[] = TEXT("255.255.255.255");
    TCHAR       tchPort[] = TEXT("19888");
    ADDRINFO*   AI = NULL;
	WSADATA     wsaData;
	bool bWaitMore = true;

	if (WSAStartup(MAKEWORD(2,2), &wsaData) != NO_ERROR)
	{
		fnWriteToInstallerLogFile(hInstall, TEXT("fnDiscover WSAStartup failed"));
        return;
	}

    struct addrinfo aiHints;
  //  struct addrinfo *aiList = NULL;
    memset(&aiHints, 0, sizeof(aiHints));
    aiHints.ai_family = AF_INET;
    aiHints.ai_socktype = SOCK_DGRAM;
    aiHints.ai_protocol = IPPROTO_UDP;

//	int addrInfoResult = getaddrinfo(tchBroadcastAddr, tchPort, &aiHints, &AI);
#pragma warning(push)
#pragma warning(disable: 6011)
    sockClient = socket(AI->ai_family, SOCK_DGRAM, 0);
#pragma warning(pop)
	if (sockClient == INVALID_SOCKET) 
	{
		fnWriteToInstallerLogFile(hInstall, TEXT("fnDiscover INVALID_SOCKET"));
		WSACleanup();
		return;
	}

    int iTimeout = 50;
    int iOptLen = sizeof(int);
    BOOL bTrueVal = TRUE;
    int bOptLen = sizeof(BOOL);
    setsockopt(sockClient, SOL_SOCKET, SO_RCVTIMEO, (char*) &iTimeout, iOptLen);
    setsockopt(sockClient, SOL_SOCKET, SO_BROADCAST, (char*) &bTrueVal, bOptLen);

    //Broadcast the message
    struct sockaddr_in broadcast;
    broadcast.sin_family = AF_INET;
    broadcast.sin_addr.s_addr = inet_addr(tchBroadcastAddr);
#pragma warning(push)
#pragma warning(disable: 4244)
	broadcast.sin_port = htons(atoi(tchPort));
#pragma warning(pop)
	int broadcast_addr_len =  sizeof(struct sockaddr);
    //StringArray responseList;

    TCHAR* tchBroadcastRequest = new TCHAR[11];
    switch (iDiscoveryType)
    {
        case DISCOVER_POLICY_SERVER_INSTANCES:
            tchBroadcastRequest = "2,3,1,1.0P";
            break;
        case DISCOVER_SERVER_INSTANCES:
            tchBroadcastRequest = "2,3,1,1.0S";
            break;
        case DISCOVER_ICENET_SERVER_INSTANCES:
            tchBroadcastRequest = "2,3,1,1.0D";
            break;
        default:
            //Should never happen
            tchBroadcastRequest = "";
            break;
    }

    
    int iSendResult = sendto(sockClient, tchBroadcastRequest, 10, NULL, (sockaddr *)&broadcast, broadcast_addr_len);
    if (iSendResult == SOCKET_ERROR )
	{
		//int iSendResult = WSAGetLastError();
		fnWriteToInstallerLogFile(hInstall, TEXT("fnDiscover SOCKET_ERROR on sendto"));
		closesocket(sockClient);
		WSACleanup();
		return;
	}

    //Start collecting answers  
	
	time_t beginTime = time(0);
    while (bWaitMore)
    {
        TCHAR tchReceivedBuf[MAX_RECV_BUF_SIZE]= "";
        int iRecvResult = recvfrom(sockClient, tchReceivedBuf, MAX_RECV_BUF_SIZE, NULL, (sockaddr *)&broadcast, &broadcast_addr_len);
        if (iRecvResult > 0)
        {
			
			// must be more than 2 bytes
			if ( iRecvResult > 2 )
			{
				CString strBuffer = tchReceivedBuf; 
				listServers.AddHead(strBuffer);
				bWaitMore = false;
			}
		}
        else
        {
			int iConnectResult = WSAGetLastError();
	        switch (iConnectResult)
	        {
		        case WSAETIMEDOUT:
			        //Timeout occured, this is fine
                    bWaitMore = true;
			        break;
		        default:
					fnWriteToInstallerLogFile(hInstall, TEXT("fnDiscover SOCKET_ERROR on recvfrom"));
			        bWaitMore = false;
	        }
        }   
		time_t endTime = time(0);
        //Socket timeout value is negligible
        time_t elapsedSeconds = endTime - beginTime;
		TCHAR tchBuffer1[32];
	//	TCHAR tchBuffer2[32];
		_itoa_s((int)elapsedSeconds,tchBuffer1,10);

        if (elapsedSeconds > iMaxSearchTime/1000)
            bWaitMore = false;
	}

	

    closesocket(sockClient);
    WSACleanup();
}
//////////////////////////////////////////////////////////////////////////////////
//	Function Name: bool fnPolicyServerParsed(MSIHANDLE hInstall,CString strListItem,CString strBuffer)
//                                    
//	Purpose: Parses the list item and builds the string for combo box insertion
//	and sets two related properties
//	Author: Michael Byrns
//  History: Created 7 September 2007
//////////////////////////////////////////////////////////////////////////////////
bool fnPolicyServerParsed(MSIHANDLE hInstall,CString strListItem,CString * strBuffer)
{
	int iBuffer;
	int iTokenCount = 0;
	int iIndex = 0;
	CString strBuff;
	CString strPort;
	CString strHost;
	int iItem1 = 0,iItem2 = 0,iItem3 = 0,iItem4 = 0;
	TCHAR * tchToken;
	TCHAR tchSeps[] = TEXT(",");
	TCHAR tchBuffer[MAX_PATH];


	strBuff = strListItem.GetAt(0);
	iBuffer = atoi(strBuff);

	if ( iBuffer != 4 )
	{
		fnWriteToInstallerLogFile(hInstall, TEXT("fnPolicyServerParsed incorrect number of items returned.  Items not equal to 4."));
		return false;
	}

	TCHAR * NextToken = NULL;
	lstrcpy(tchBuffer,strListItem);
	// Establish string and get the first token: 
    tchToken = strtok_s( tchBuffer, tchSeps, &NextToken);

	while( tchToken != NULL )
    {		
		switch ( iTokenCount)
		{
			case 1: 
				iItem1 = atoi(tchToken);
				break;
			case 2: 
				iItem2 = atoi(tchToken);
				break;
			case 3: 
				iItem3 = atoi(tchToken);
				break;
			case 4: 
				iItem4 = atoi(tchToken);
				break;
			case 5: 
				strBuff = tchToken;
				break;
		}
		tchToken = strtok_s( NULL, tchSeps, &NextToken); 
		iTokenCount++;
	}
	for ( iIndex = 0; iIndex < strBuff.GetLength(); iIndex++)
	{
		if (( iIndex >= iItem1 ) && (iIndex <= iItem2))
				strHost += strBuff.GetAt(iIndex);

		if (( iIndex >= iItem1 + iItem2 + iItem3 ) && (iIndex <= strBuff.GetLength()))
				strPort += strBuff.GetAt(iIndex);

	}

	MsiSetProperty(hInstall,TEXT("PORTNUMBER"),strPort);
	MsiSetProperty(hInstall,TEXT("HOSTNAME"),strHost);

	*strBuffer += strHost;
	*strBuffer += TEXT(":");
	*strBuffer += strPort;

	return true;
}