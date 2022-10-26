// TagViewMenu.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include <atlbase.h>


#define FILETAGGING_NEXTLABS_REGKEY				L"SOFTWARE\\Nextlabs"
#define FILETAGGING_TAGVIEW_REGKEY				L"SOFTWARE\\Nextlabs\\TagView"	
#define FILETAGGING_TAGVIEW_SUBKEY				L"TagView"
#define FILETAGGING_TAGVIEW_COUNT				L"count"



#ifdef _MANAGED
#pragma managed(push, off)
#endif

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
    return TRUE;
}

STDAPI DllRegisterServer(void)
{
	// registers object, typelib and all interfaces in typelib
	if ( 0 == (GetVersion() & 0x80000000UL) )
	{
		LONG lRet=0;
		DWORD dwCount=0;
		CRegKey keyCount;
		lRet=keyCount.Open(HKEY_LOCAL_MACHINE, FILETAGGING_TAGVIEW_REGKEY);
		if(ERROR_SUCCESS == lRet)
		{
			if(keyCount.QueryDWORDValue(FILETAGGING_TAGVIEW_COUNT, dwCount) == ERROR_SUCCESS)
			{
				if(dwCount <= 0)
					dwCount = 1;
				else
					dwCount++;
			}
			else
				dwCount = 1;
		}
		else if(lRet == ERROR_FILE_NOT_FOUND)
		{
			
			lRet = keyCount.Create(HKEY_LOCAL_MACHINE, FILETAGGING_TAGVIEW_REGKEY);
			if(lRet != ERROR_SUCCESS)
				return lRet;
			dwCount = 1;
		}
		else
			return lRet;

		lRet = keyCount.SetDWORDValue(FILETAGGING_TAGVIEW_COUNT, dwCount);
		if(lRet != ERROR_SUCCESS)
			return lRet;
		keyCount.Close();

		CRegKey theReg;
		lRet = theReg.Create(HKEY_CLASSES_ROOT,L"*\\shellex\\ContextMenuHandlers\\FileTagging");
		if(ERROR_SUCCESS == lRet)
		{
			lRet = theReg.SetStringValue(NULL,L"{E7F164F5-8E7C-4AAB-BC8D-96A396661BFA}");
			theReg.Close();
		}
		else
			return E_ACCESSDENIED;

	}
	// registers object, typelib and all interfaces in typelib
	
	return ERROR_SUCCESS;
}


// DllUnregisterServer - Removes entries from the system registry
STDAPI DllUnregisterServer(void)
{
	if ( 0 == (GetVersion() & 0x80000000UL) )
	{
		LONG lRet=0;
		CRegKey keyCount;
		lRet=keyCount.Open(HKEY_LOCAL_MACHINE,FILETAGGING_TAGVIEW_REGKEY);
		if(ERROR_SUCCESS == lRet)
		{
			DWORD dwCount=0;
			if(keyCount.QueryDWORDValue(FILETAGGING_TAGVIEW_COUNT,dwCount)==ERROR_SUCCESS)
			{
				dwCount--;
				if(dwCount>0)
				{
					if(keyCount.SetDWORDValue(FILETAGGING_TAGVIEW_COUNT,dwCount)!=ERROR_SUCCESS)
						return ERROR_CANTWRITE;
					else
						return 0;
				}
				else
				{
					CRegKey keyNL;
					lRet=keyNL.Open(HKEY_LOCAL_MACHINE,FILETAGGING_NEXTLABS_REGKEY);
					if(lRet!=ERROR_SUCCESS)
						return lRet;
					else
					{
						lRet=keyNL.DeleteSubKey(FILETAGGING_TAGVIEW_SUBKEY);
						if(lRet!=ERROR_SUCCESS)
							return lRet;
					}
				}
			}
			else
				return ERROR_CANTWRITE;
		}
		else
			return lRet;

		CRegKey theReg;
		lRet = theReg.Open(HKEY_CLASSES_ROOT,L"*\\shellex\\ContextMenuHandlers");
		if(lRet == ERROR_SUCCESS)
		{
			lRet = theReg.DeleteSubKey(L"FileTagging");
			theReg.Close();
		}

	}
	
	return ERROR_SUCCESS;
}

#ifdef _MANAGED
#pragma managed(pop)
#endif

