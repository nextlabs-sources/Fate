/*  FIX ME - common.h and common.rul need to be refactored.  Way too many unrelated functions!.  Should also consider removing Destiny specific methods */
#ifndef COMMON_HEADER
	#define CANGOTONEXT_PROP_NAME "CanGoToNextDialog"

	export prototype AddCboEntry(HWND, STRING, STRING, STRING, NUMBER);
	export prototype void AllowNextDialog(HWND);
	export prototype WSTRING BrowseForFile (HWND);
	export prototype INT CopyFileFromBinaryTable(HWND, WSTRING, WSTRING);  
	export prototype INT CreateWindowsServiceAccount(HWND);    
	export prototype INT DeleteWindowsServiceAccount(HWND);
	export prototype void DenyNextDialog(HWND);
	export prototype INT DisableAutomaticReboot(HWND);
	export prototype INT DiscoverIcenetServersLocations(HWND, WSTRING, BYREF LIST);
	export prototype INT DiscoverMgmtServersLocations(HWND, WSTRING, BYREF LIST);
	export prototype INT DiscoverPolicyServersLocations(HWND, WSTRING, BYREF LIST);
	export prototype BOOL IsDigit(CHAR);
	export prototype BOOL IsFirstInstallation(HWND);
	export prototype BOOL IsLetter(CHAR);
	export prototype BOOL IsSilentInstallation(HWND);
	export prototype BOOL IsSpecialChar(CHAR);
	export prototype BOOL IsFullUninstallation(HWND);
	export prototype BOOL IsValidWindowsServiceUser(HWND, WSTRING, WSTRING);
	export prototype INT FindAvailablePort (HWND, INT);
	export prototype WSTRING GetCurrentDomainName(HWND, BOOL);
	export prototype WSTRING GetCurrentDomainPDC(HWND); 
	export prototype WSTRING GetCurrentHostSid(HWND);
	export prototype WSTRING GetCurrentHostName(HWND);
	export prototype WSTRING GetCurrentUserDisplayName(HWND);
	export prototype WSTRING GetCurrentUserFirstName(HWND);
	export prototype WSTRING GetCurrentUserLastName(HWND);
	export prototype WSTRING GetCurrentUserLoginName(HWND);
	export prototype WSTRING GetCurrentUserPrincipalName(HWND);
	export prototype WSTRING GetFileDirectory(HWND, WSTRING);
	export prototype WSTRING GetFileName(HWND, WSTRING);
	export prototype WSTRING GetInstallationDirNoSlash(HWND);
	export prototype NUMBER GetLastIndex(NUMBER, NUMBER);
	export prototype INT GetLoginDetails(WSTRING, BYREF WSTRING, BYREF WSTRING);
	export prototype WSTRING GetHostName(HWND, WSTRING);
	export prototype NUMBER GetNumCols(HWND);
	export prototype INT GetPortNumber(HWND, WSTRING, INT);
	export prototype WSTRING GetProperty(HWND, WSTRING);
	export prototype int LoadInstallerCommonDLL(HWND);
	export prototype WSTRING MakeString(HWND, INT);
	export prototype INT ShowModalDialog (HWND, WSTRING, WSTRING, INT);
	export prototype INT ShowYesNoDialog (HWND, WSTRING, WSTRING);
	export prototype INT SetProperty (HWND, WSTRING, WSTRING);
	export prototype int UnloadInstallerCommonDLL(HWND);
	export prototype INT ReplaceInFile(HWND, WSTRING, WSTRING, WSTRING, BOOL);
	export prototype INT ValidateFreePortNumber (HWND, WSTRING, WSTRING);
	export prototype INT ValidateLDAPConnection(HWND, WSTRING, INT, WSTRING, WSTRING, WSTRING, WSTRING, BYREF WSTRING);
	export prototype INT ValidateLocationInput (HWND, BYREF WSTRING, BYREF WSTRING, BYREF WSTRING, INT, BYREF STRING, BOOL);
	export prototype BOOL ValidatePassword (WSTRING);
	export prototype INT ValidatePasswords (WSTRING, WSTRING);
	export prototype INT ValidatePortNumber (HWND, BYREF WSTRING, BYREF WSTRING);

    /*
     *  The provided StreamFileFromBinary was not functioning as
     *  expected.  Therefore, I wrote my own version
     */
    export prototype INT StreamFileFromBinaryTable(HWND, STRING, STRING);
	//export prototype INT WaitForService (HWND, INT);
	
	//Exported function from installer common DLL
	export prototype INT installercommon.autoCreateServiceUser (BYREF WSTRING, BYREF WSTRING, BYREF WSTRING, BYREF WSTRING);
	export prototype INT installercommon.browseForFiles (BYREF WSTRING);
	export prototype INT installercommon.deleteServiceUserAccount (BYREF WSTRING);
	export prototype INT installercommon.discoverIcenetServers(BYREF WSTRING, INT);
	export prototype INT installercommon.discoverMgmtServers (BYREF WSTRING, INT);
	export prototype INT installercommon.discoverPolicyServers (BYREF WSTRING, INT);
	export prototype INT installercommon.findAvailablePort (INT);
	export prototype INT installercommon.findDomainControllers(BYREF WSTRING);
	export prototype INT installercommon.getCurrentUserDisplayName(BYREF WSTRING, INT);
	export prototype INT installercommon.getCurrentUserFirstName(BYREF WSTRING, INT);
	export prototype INT installercommon.getCurrentUserLastName(BYREF WSTRING, INT);
	export prototype INT installercommon.getCurrentUserLoginName(BYREF WSTRING, INT);
	export prototype INT installercommon.getCurrentUserPrincipalName(BYREF WSTRING, INT);
	export prototype INT installercommon.getDomainName(BYREF WSTRING, BOOL);
	export prototype INT installercommon.getHostName (BYREF WSTRING, BYREF WSTRING);
	export prototype INT installercommon.getHostSID (BYREF WSTRING, BYREF WSTRING);
	export prototype INT installercommon.getPortNumber (BYREF WSTRING, INT);
	export prototype INT installercommon.hashChallenge (BYREF WSTRING, BYREF WSTRING, BYREF INT);
	export prototype INT installercommon.showModalMessage(BYREF WSTRING, BYREF WSTRING, INT);
	export prototype INT installercommon.showYesNoDialog(BYREF WSTRING, BYREF WSTRING);
	export prototype INT installercommon.replaceInFile (BYREF WSTRING, BYREF WSTRING, BYREF WSTRING);
	export prototype INT installercommon.testLDAPConnection(BYREF WSTRING, INT, BYREF WSTRING, BYREF WSTRING, BYREF WSTRING, BYREF WSTRING, BYREF WSTRING);
	export prototype INT installercommon.validateAvailablePort(INT);
	export prototype INT installercommon.validateLocationInput (BYREF WSTRING, BYREF WSTRING, BYREF WSTRING, INT, BOOL);
	export prototype INT installercommon.validateServiceAccount (BYREF WSTRING, BYREF WSTRING, BYREF WSTRING); 
	//export prototype INT installercommon.waitForService (INT);
	#define INSTALLER_COMMON_DLL_NAME  "installercommon.dll"
	
	#define DEFAULT_WEB_APPLICATION_PORT 443
	#define DEFAULT_WEB_SERVICE_PORT 8443

#endif
#define COMMON_HEADER
