#include "StdAfx.h"
#include <sddl.h>
#include <lm.h>
#include "globals.h"
#include "userutils.h"
#include <wchar.h>

UserUtils::UserUtils(void)
{
}

UserUtils::~UserUtils(void)
{
}

//////////////////////////////////////////////////////////////////////////////////
// Return the SID of the specified user. pszSID must be deallocated by caller
//
//////////////////////////////////////////////////////////////////////////////////
void UserUtils::GetUserSID(TCHAR* pszUserName, TCHAR **ppszSID)
{
    UCHAR          psnuType[2048];
    UCHAR          userSID[1024];
    DWORD          dwSIDBufSize=1024;
    TCHAR          lpszDomain[2048];
    DWORD          dwDomainLength = 250;

    if (LookupAccountName(NULL,
        pszUserName,
        userSID,
        &dwSIDBufSize,
        lpszDomain,
        &dwDomainLength,
        (PSID_NAME_USE) psnuType))
    {
        ::ConvertSidToStringSid((PSID) userSID, ppszSID);
    }
}

//////////////////////////////////////////////////////////////////////////////////
// Return array of SIDs for all current users. 
//
//////////////////////////////////////////////////////////////////////////////////
void GetCurrentUsers (StringVector& loggedInUsers)
{

    LPWKSTA_USER_INFO_1 pUserInfo = NULL;
    DWORD entriesRead = 0;
    DWORD totalEntries = 0;
    TCHAR userName[1024];
    TCHAR* pszSID = NULL;
    //DWORD foo = ERROR_MORE_DATA;
  
    ::NetWkstaUserEnum(NULL, 1, (LPBYTE*) &pUserInfo, 
				      MAX_PREFERRED_LENGTH, &entriesRead, 
				      &totalEntries, NULL);

   
    for (DWORD i = 0; i < entriesRead; i++){
        // dont add to list if this is a host and not a user. (hosts end with $)
        if (pUserInfo[i].wkui1_username[_tcslen(pUserInfo[i].wkui1_username) - 1] != '$')
        {
            _stprintf_s (userName, _T("%s\\%s"), pUserInfo[i].wkui1_logon_domain, pUserInfo[i].wkui1_username);
			UserUtils::GetUserSID (userName, &pszSID);
			//The SID string will be deleted by the caller
			loggedInUsers.push_back(pszSID);
        }
    }
    if (pUserInfo != NULL)
    {
        NetApiBufferFree((LPVOID) pUserInfo);
    }    
}
//////////////////////////////////////////////////////////////////////////////////
// Return array of SIDs for all users logged in now and sometimes before
//
//////////////////////////////////////////////////////////////////////////////////
#define PROFILE_LIST_KEY _T("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\ProfileList")
void UserUtils::GetLoggedInUsers (StringVector& users)
{
  HKEY hKey;

  if (::RegOpenKeyEx (HKEY_LOCAL_MACHINE, PROFILE_LIST_KEY, 0, KEY_READ, 
		      &hKey) == ERROR_SUCCESS) {
    TCHAR    achKey[256];   // buffer for subkey name
    DWORD    cbName;                   // size of name string 
    TCHAR    achClass[MAX_PATH] = TEXT("");  // buffer for class name 
    DWORD    cchClassName = MAX_PATH;  // size of class string 
    DWORD    cSubKeys=0;               // number of subkeys 
    DWORD    cbMaxSubKey;              // longest subkey size 
    DWORD    cchMaxClass;              // longest class string 
    DWORD    cValues;              // number of values for key 
    DWORD    cchMaxValue;          // longest value name 
    DWORD    cbMaxValueData;       // longest value data 
    DWORD    cbSecurityDescriptor; // size of security descriptor 
    FILETIME ftLastWriteTime;      // last write time 

    // Get the class name and the value count. 
    DWORD retCode = RegQueryInfoKey(hKey,  
				    achClass,
				    &cchClassName,
				    NULL,         
				    &cSubKeys,    
				    &cbMaxSubKey,            
				    &cchMaxClass,            
				    &cValues,                
				    &cchMaxValue,            
				    &cbMaxValueData,         
				    &cbSecurityDescriptor,   
				    &ftLastWriteTime);       
      
    TCHAR *retKeyStr=NULL;
    size_t retKeyStrLen;

    for (DWORD i=0; i<cSubKeys; i++) { 
      cbName = 256; //MAX_KEY_LENGTH
      retCode = RegEnumKeyEx(hKey, i,
			     achKey, 
			     &cbName, 
			     NULL, 
			     NULL, 
			     NULL, 
			     &ftLastWriteTime); 
      if (retCode == ERROR_SUCCESS) {	
	//The SID string will be deleted by the caller
	//Thus, we need to return heap allocated strings
	retKeyStrLen=_tcslen(achKey);
	retKeyStr=(TCHAR *)malloc((retKeyStrLen+1)*sizeof(TCHAR));
	if (retKeyStr != NULL) {
	  wcsncpy_s(retKeyStr, retKeyStrLen+1,achKey, _TRUNCATE);
	  users.push_back(retKeyStr);
	}
      }
    }      
    ::RegCloseKey (hKey);
  }
}
