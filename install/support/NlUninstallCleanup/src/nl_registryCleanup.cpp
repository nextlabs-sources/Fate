/****************************************************************************************
 *
 * NextLabs Uninstall Cleanup Utility Tool
 *
 * Registry Class
 * January 2012 
 ***************************************************************************************/

#include <windows.h>
#include <string>
#include <iostream>
#include <list>

#include <tchar.h>
#include <strsafe.h>
#include <aclapi.h>
#include <stdio.h>

#include <stdio.h>
#include <strsafe.h>
#include <windows.h>
#include <stdio.h>
#include <tchar.h>

#define MAX_KEY_LENGTH 255
#define MAX_VALUE_NAME 16383

#include "nl_registryCleanup.h"

// create registry: not used at this time
// It will be used for unit testing
int RegistryCleanup::createItems(itemList& items)
{
	printf("\n create items in RegistryCleanup\n");
	itemListIter str = items.begin();
	for( ; str != items.end(); str++)
	{
		createItem(*str);
	}	
	return 0;
};
// delete multiple registries
int RegistryCleanup::deleteItems(itemList& items)
{
	if( items.size() == 0 ) return 0;
	fprintf(m_File, "\n\nDelete registries ...\n");
	printf("\n\nDelete registries ...\n");

    HKEY hTestKey;
	LONG lResult;
	HKEY hKeyRoot;
	std::wstring hKey;
	std::wstring subKey;
	itemListIter str = items.begin();
	for( ; str != items.end(); str++)
	{
		// get hkey and subKey from input string
		getRootKeySubkey(hKey, subKey, *str);

		TCHAR szDelKey[MAX_PATH*2];

		StringCchCopy (szDelKey, MAX_PATH, (LPTSTR) subKey.c_str() );
		// get predefined handle
		hKeyRoot = getPredefinedHandle(hKey); 

		if( (lResult = RegOpenKeyEx( hKeyRoot,  
				szDelKey,   // for RegOpenKeyEx,
				0,
				KEY_QUERY_VALUE,  //KEY_READ,
				&hTestKey) ) == ERROR_SUCCESS)
		{
			printf("\t%ls\\%ls ", hKey.c_str(), szDelKey );
			fprintf(m_File, "\t%ls\\%ls ", hKey.c_str(), szDelKey);

			if( m_Delete == false )
			{
				// simulation mode: just query -- dont delete
				QueryKey(hTestKey);
				printf(" -- Will Be Deleted\n");
				fprintf(m_File, " -- Will Be Deleted\n");
			} else
			{
				// delete registry physically
				bool flag = RegDelnodeRecurse(hKeyRoot, szDelKey);
				if( flag )
				{
					printf(" - Deleted sucsessfully\n");
					fprintf(m_File, " - Deleted sucsessfully\n");
				} else
				{
					m_numErrors++;
					printf(" Tried to delete but failed\n");
					fprintf(m_File, " Tried to delete but failed\n");
				}
			}
		}else
		{
			fprintf(m_File, "\t%ls\\%ls ", hKey.c_str(), szDelKey);
			if (lResult == ERROR_FILE_NOT_FOUND) {
				fprintf(m_File, "\t\t Key not found.\n");
			} 
			else {
				getLastErrorStr(lResult, errStr);
				printf("\t\t ERROR opening key. %ws\n ", errStr);
				fprintf(m_File, "\t\t ERROR opening key. %ws\n", errStr);
				//printLastError2(lResult);
				m_numErrors++;
			}
		}
		RegCloseKey(hTestKey);	
	}	
	return m_numErrors;	
};

//*************************************************************
//
//  RegDelnodeRecurse()
//
//  Purpose:    Deletes a registry key and all its subkeys / values.
//
//  Parameters: hKeyRoot    -   Root key
//              lpSubKey    -   SubKey to delete
//
//  Return:     TRUE if successful.
//              FALSE if an error occurs.
//
//*************************************************************
bool RegistryCleanup::RegDelnodeRecurse (HKEY hKeyRoot, LPTSTR lpSubKey)
{
    LPTSTR lpEnd;
    LONG lResult;
    DWORD dwSize;
    TCHAR szName[MAX_PATH];
    HKEY hKey = HKEY_LOCAL_MACHINE;
    FILETIME ftWrite;

    // First, see if we can delete the key without having
    // to recurse.
    if( m_Delete == true )
	{
		lResult = RegDeleteKey(hKeyRoot, lpSubKey);
		//printf("\n\t\t -- lResult %d from RegDeleteKey", lResult);
		if (lResult == ERROR_SUCCESS) 
		{
			printf("\n\t\t Key Deleted. ");
			fprintf(m_File, "\n\t\t Key Deleted. ");
			return TRUE;
		}
		else
		{
			getLastErrorStr(lResult, errStr);
			printf("\n\t\t -- ERROR Code %d from RegDeleteKey : %ws", lResult, errStr);
			//printLastError2(lResult);
		}
		lResult = RegOpenKeyEx (hKeyRoot, lpSubKey, 0, KEY_READ, &hKey);
		//printf("\n\t\t -- lResult %d from RegOpenKeyEx", lResult);
		if (lResult != ERROR_SUCCESS) 
		{
			if (lResult == ERROR_FILE_NOT_FOUND) {
				printf("Key not found. It is OK.\n");
				fprintf(m_File, "Key not found. It is OK.\n");
				return TRUE;
			} 
			else {
				getLastErrorStr(lResult, errStr);
				printf("ERROR opening key. %ws\n", errStr);
				fprintf(m_File, "ERROR opening key. %ws\n", errStr);
				//printLastError2(lResult);
				return FALSE;
			}
		} else
		{
			printf("\n\t\t RegOpenKeyEx Opened");
			fprintf(m_File, "\n\t\t RegOpenKeyEx Opened");
		}
	}
    // Check for an ending slash and add one if it is missing.

    lpEnd = lpSubKey + lstrlen(lpSubKey);

    if (*(lpEnd - 1) != TEXT('\\')) 
    {
        *lpEnd =  TEXT('\\');
        lpEnd++;
        *lpEnd =  TEXT('\0');
    }

    // Enumerate the keys

    dwSize = MAX_PATH;
    lResult = RegEnumKeyEx(hKey, 0, szName, &dwSize, NULL,
                           NULL, NULL, &ftWrite);

    if (lResult == ERROR_SUCCESS) 
    {
        do {

            StringCchCopy (lpEnd, MAX_PATH*2, szName);

            if (!RegDelnodeRecurse(hKeyRoot, lpSubKey)) {
                break;
            }

            dwSize = MAX_PATH;

            lResult = RegEnumKeyEx(hKey, 0, szName, &dwSize, NULL,
                                   NULL, NULL, &ftWrite);

        } while (lResult == ERROR_SUCCESS);
    } 

    lpEnd--;
    *lpEnd = TEXT('\0');

    RegCloseKey (hKey);

    // Try again to delete the key.

    lResult = RegDeleteKey(hKeyRoot, lpSubKey);

    if (lResult == ERROR_SUCCESS) 
        return TRUE;

    return FALSE;
}


// get header key and subkey from one full path key
void RegistryCleanup::getRootKeySubkey(std::wstring &hkey, std::wstring &subkey, std::wstring str)
{
	wchar_t buffer[MAX_LENGTH];
	wcscpy_s(buffer,MAX_LENGTH, str.c_str());
	wchar_t* ptr = buffer;
	int length = 0;

	while( *ptr != '\\' ) //strncmp(ptr, "\", 1) != 0 )
	{
		length++;
		ptr++;
	}
	*ptr = '\0';
	hkey = wstring(buffer);
	subkey = wstring(++ptr);
}



// QueryKey - Enumerates the subkeys of key and its associated values.
//     hKey - Key whose subkeys and values are to be enumerated.
void RegistryCleanup::QueryKey(HKEY hKey) 
{ 
    TCHAR    achKey[MAX_KEY_LENGTH];   // buffer for subkey name
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
 
    DWORD i, retCode; 
 
     
	// Fix Warning C6262: use heap instead of stack
	// dont use stack -- TCHAR  achValue[MAX_VALUE_NAME];
	TCHAR  *achValue;
    achValue = (TCHAR *) malloc( MAX_VALUE_NAME * sizeof(TCHAR));

    if (achValue == NULL) 
    {
		printf( "\\nERROR: memory allocation for Registry and key\n");
		exit(1);
    }

    DWORD cchValue = MAX_VALUE_NAME; 
 
    // Get the class name and the value count. 
    retCode = RegQueryInfoKey(
        hKey,                    // key handle 
        achClass,                // buffer for class name 
        &cchClassName,           // size of class string 
        NULL,                    // reserved 
        &cSubKeys,               // number of subkeys 
        &cbMaxSubKey,            // longest subkey size 
        &cchMaxClass,            // longest class string 
        &cValues,                // number of values for this key 
        &cchMaxValue,            // longest value name 
        &cbMaxValueData,         // longest value data 
        &cbSecurityDescriptor,   // security descriptor 
        &ftLastWriteTime);       // last write time 
 
    // Enumerate the subkeys, until RegEnumKeyEx fails.
    
    if (cSubKeys)
    {
        printf( "\n\t\t\t Number of deleted subkeys: %d ", cSubKeys);
        fprintf(m_File, "\n\t\t\t Number of deleted subkeys: %d ", cSubKeys);
        for (i=0; i<cSubKeys; i++) 
        { 
            cbName = MAX_KEY_LENGTH;
            retCode = RegEnumKeyEx(hKey, i,
                     achKey, 
                     &cbName, 
                     NULL, 
                     NULL, 
                     NULL, 
                     &ftLastWriteTime); 
            if (retCode == ERROR_SUCCESS) 
            {
                _tprintf(TEXT("(%d) %s\n"), i+1, achKey);
            }
        }
    } 
 
    // Enumerate the key values. 

    if (cValues) 
    {
        printf( "\tNumber of values: %d\n", cValues);

        for (i=0, retCode=ERROR_SUCCESS; i<cValues; i++) 
        { 
            cchValue = MAX_VALUE_NAME; 
            achValue[0] = '\0'; 
            retCode = RegEnumValue(hKey, i, 
                achValue, 
                &cchValue, 
                NULL, 
                NULL,
                NULL,
                NULL);
 
            if (retCode == ERROR_SUCCESS ) 
            { 
                _tprintf(TEXT("(%d) %s\n"), i+1, achValue); 
            } 
        }
    }
	// free memory
	if( achValue != NULL)
	{
        free(achValue);
	}
}

// get predefined handle
HKEY RegistryCleanup::getPredefinedHandle(const std::wstring str )
{
	//	predefined keys: 
	if( ( wcscmp(str.c_str(), L"HKLM") == 0 )
		|| ( wcscmp(str.c_str(), L"HKEY_LOCAL_MACHINE") == 0 ) )
	{
		return HKEY_LOCAL_MACHINE;
	} else
	if( ( wcscmp(str.c_str(), L"HKCR") == 0 )
		|| ( wcscmp(str.c_str(), L"HKEY_CLASSES_ROOT") == 0 ) ) 
	{
		return HKEY_CLASSES_ROOT;
	} else
	if( ( wcscmp(str.c_str(), L"HKCC") == 0 )
		|| ( wcscmp(str.c_str(), L"HKEY_CURRENT_CONFIG") == 0 ) )
	{
		return HKEY_CURRENT_CONFIG;
	} else
	if( ( wcscmp(str.c_str(), L"HKCU") == 0 )
		|| ( wcscmp(str.c_str(), L"HKEY_CURRENT_USER") == 0 ) )
	{
		return HKEY_CURRENT_USER;
	}  else
	if( ( wcscmp(str.c_str(), L"HKU") == 0 )
		|| ( wcscmp(str.c_str(), L"HKEY_USERS") == 0 ) )
	{
		return HKEY_USERS;
	} 
	printf("\n only predefined handle(key) is supported: %ls is not supported\n", str.c_str());
	exit(0);
}