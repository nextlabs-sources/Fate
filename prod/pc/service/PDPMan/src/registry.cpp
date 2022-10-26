/*
* registry.c    
* Author: Helen Friedland
* All sources, binaries and HTML pages (C) copyright 2004 by Blue Jungle Inc., 
* Redwood City CA, Ownership remains with Blue Jungle Inc, 
* All rights reserved worldwide. 
*
*/

#if defined (WIN32) || defined (_WIN64)

#include<windows.h>
#include<stdio.h>
#include<stdlib.h>
#include<winreg.h>


int getStringValue(LPBYTE lpVal, LPDWORD lpcbLen, HKEY hkRoot, LPCSTR lpszPath, LPSTR lpszValue)
{

    LONG result;
    HKEY hKey;

    DWORD dwType;

    result = RegOpenKeyExA(
        hkRoot,
        lpszPath,
        (DWORD)0,
        KEY_EXECUTE | KEY_QUERY_VALUE,
        (PHKEY)&hKey);

    if(result != ERROR_SUCCESS)
    {
        return 1;
    }

    result = RegQueryValueExA(
        hKey,
        lpszValue, 
        NULL, 
        (LPDWORD)&dwType, 
        lpVal, 
        lpcbLen);    

    RegCloseKey(hKey);

    return !(result == ERROR_SUCCESS && 
        (dwType == REG_SZ || dwType == REG_EXPAND_SZ));
}

int setStringValue(CONST BYTE *lpVal, DWORD cbLen, HKEY hkRoot, LPCSTR lpszPath, LPCSTR lpszValue)
{

    LONG result;
    HKEY hKey;

    DWORD dwType = REG_SZ;

    result = RegOpenKeyExA(
        hkRoot,
        lpszPath,
        (DWORD)0,
        KEY_WRITE,
        (PHKEY)&hKey);

    if(result != ERROR_SUCCESS)
    {
        return 1;
    }

    result = RegSetValueExA(
        hKey,
        lpszValue, 
        (DWORD)0, 
        dwType, 
        lpVal, 
        cbLen);    

    RegCloseKey(hKey);

    return !(result == ERROR_SUCCESS);
}

int makeNewKey(HKEY hkRoot, LPCSTR lpszPath)
{
    char *classname = "LocalSystem";

    LONG result;
    HKEY hKey;
    DWORD disposition;


    result = RegCreateKeyExA(
        hkRoot,
        lpszPath,
        (DWORD)0,
        classname,
        REG_OPTION_NON_VOLATILE,
        KEY_ALL_ACCESS,
        NULL,
        (PHKEY)&hKey,
        (LPDWORD) &disposition);

    if(result != ERROR_SUCCESS)
    {
        return 1;
    }


    RegCloseKey(hKey);

    return !(result == ERROR_SUCCESS);
}

#endif

