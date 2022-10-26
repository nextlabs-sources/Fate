#include "StdAfx.h"
#include "InstalledComptMgrImp.h"

#include "nlconfig.hpp"
#include <algorithm>

typedef struct
{
	int major;           /* Major Version */
	int minor;           /* Minor Version */
	int maintenance;     /* Maintenance Version */
	int patch;           /* Patch Version */
	int build;           /* Build Number */
} NextLabs_ProductVersion_t;

wstring GetUpdatedTimeFromUninstallKey(const wstring& ProductCode)
{
	wstring ret;
	wstring entryBase = L"Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\";
	
	//	full entry name
	wstring fullEntry = entryBase + ProductCode;

	//	open key
	HKEY hKey = NULL;
	LONG rstatus = RegOpenKeyExW(HKEY_LOCAL_MACHINE,fullEntry.c_str(),0,KEY_QUERY_VALUE,&hKey);
	if( rstatus != ERROR_SUCCESS )
	{
		//	we should not return false,
		//	as this is acceptable that a component has no such key
		//	do nothing here.
#ifdef _WIN64
		//Try to L"Software\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\"
		wstring wow6432node = L"Software\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\";
		wstring fullEntry32 = wow6432node + ProductCode;
		LONG rt = RegOpenKeyExW(HKEY_LOCAL_MACHINE,fullEntry32.c_str(),0,KEY_QUERY_VALUE,&hKey);
		if(rt != ERROR_SUCCESS)
		{
			return ret;
		}
#else
		return ret;
#endif
	}
	//	opened, query key value
	wchar_t keyValue[1024] = {0};
	DWORD keyValueLen = 1024;
	rstatus = RegQueryValueExW(hKey,L"InstallDate",NULL,NULL,(LPBYTE)keyValue,&keyValueLen);
	//	close key
	RegCloseKey(hKey);
	ret.assign(keyValue);

	return ret;
}

BOOL NextLabs_GetProductVersion( _In_ const wchar_t* in_path ,
								_In_ NextLabs_ProductVersion_t* out_ver )
{
	if( in_path == NULL || out_ver == NULL )
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}
	memset(out_ver,0x00,sizeof(NextLabs_ProductVersion_t));
	FILE* fp = NULL;
	errno_t err = _wfopen_s(&fp,in_path,L"r");
	if( err != 0 || fp == NULL )
	{
		return FALSE;
	}

	char buf[512] = {0};
	size_t bytes_read;
	bytes_read = fread((void*)buf,sizeof(wchar_t),sizeof(buf),fp);
	if( bytes_read != sizeof(buf) && ferror(fp) )
	{
		fclose(fp);
		return FALSE;
	}
	fclose(fp);

	/* Expecting format of "v[Major].[Minor].[Maintenance].[Patch] ([Build]) */
	const char* p = strrchr(buf,'v');
	if( p == NULL )
	{
		SetLastError(ERROR_INVALID_DATA);
		return FALSE;
	}
	int nscan = 0;
	nscan = sscanf_s(p,"v%d.%d.%d.%d (%d)",
		&out_ver->major,&out_ver->minor,
		&out_ver->maintenance,&out_ver->patch,
		&out_ver->build);

	if( nscan != 5 )
	{
		SetLastError(ERROR_INVALID_DATA);
		return FALSE;
	}
	return TRUE;
}/* NextLabs_ReadProductVersion */

CInstalledComptMgrImp::CInstalledComptMgrImp(void)
{
}

CInstalledComptMgrImp::~CInstalledComptMgrImp(void)
{
}


/*

parameter:

pair<wstring, wstring>	--	installed component name, such as L"Windows Desktop Enforcer", install dir of the component

vector<>	--	all name and dir pairs of installed components installed on current machine.

return result:

true	--	get these information succeed
false	--	get these information failed

*/
BOOL CInstalledComptMgrImp::GetComponentsInstallDir( LPCOMPONENTINFO pInstallDirs, int& nCount)
{
	wstring keyName = L"InstallDir";

	vector< pair<wstring, wstring> > vInstallDirs;
	GetComponentsInstallDir(vInstallDirs);
	
	if((int)(vInstallDirs.size()) > nCount)
	{
		nCount = (int)vInstallDirs.size();
		return FALSE;
	}

	//	find PolicyController, its dir is special
	for (DWORD i = 0; i < vInstallDirs.size(); i++)
	{

		g_log.Log(CELOG_DEBUG, L"vInstallDirs [%s]: [%s]\n", vInstallDirs[i].first.c_str(),vInstallDirs[i].second.c_str());



		memset(&(pInstallDirs[i]), 0, sizeof(COMPONENTINFO));
		wcsncpy_s(pInstallDirs[i].szComponentName, sizeof(pInstallDirs[i].szComponentName)/sizeof(wchar_t), vInstallDirs[i].first.c_str(), _TRUNCATE);

		/*
		if (vInstallDirs[i].first == L"NextLabs Policy Controller")
		{
			vInstallDirs[i].second += wstring(L"Policy Controller\\");
		}*/

		wcsncpy_s(pInstallDirs[i].szInfo, sizeof(pInstallDirs[i].szInfo)/sizeof(wchar_t), vInstallDirs[i].second.c_str(), _TRUNCATE);
	}

	nCount = (int)vInstallDirs.size();
	return TRUE;
}


/*

parameter:

pair<wstring, wstring>	--	installed component name, such as L"Windows Desktop Enforcer", last updated (installed) date of the component.

vector<>	--	all name and date pairs of installed components installed on current machine.

return result:

true	--	get these information succeed
false	--	get these information failed

*/
BOOL CInstalledComptMgrImp::GetComponentsLastUpdatedDate( LPCOMPONENTINFO pLastUpdatedDates, int& nCount)
{
	wstring keyName = L"LastUpdated";

	vector< pair<wstring, wstring> > vLastUpdatedDates;

	BOOL res = TraverseProductRegistry(vLastUpdatedDates, keyName);
	if (!res)
	{
		g_log.Log(CELOG_DEBUG, L"No such key, %s\n", keyName);
		return FALSE;
	}

	if((int)(vLastUpdatedDates.size()) > nCount)
	{
		g_log.Log(CELOG_DEBUG, "Input parameter doesn't have enough memory, input: %d, needed: %d\n", nCount, vLastUpdatedDates.size());
		nCount = (int)vLastUpdatedDates.size();
		return FALSE;
	}

	for(unsigned i = 0; i < vLastUpdatedDates.size(); i ++)
	{
		memset(&(pLastUpdatedDates[i]), 0, sizeof(COMPONENTINFO));
		wcsncpy_s(pLastUpdatedDates[i].szComponentName, sizeof(pLastUpdatedDates[i].szComponentName)/sizeof(wchar_t), vLastUpdatedDates[i].first.c_str(), _TRUNCATE);

		wstring _value = vLastUpdatedDates[i].second;
		if (_value.length() > 0 && _value[0] == '{')//read a ProductCode instead of LastUpdated. the case is: use Control Panel to read 5.5 products
		{//here, Control Panel needs to get the updated time from "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall"
			_value = GetUpdatedTimeFromUninstallKey(_value);
			if (_value.length() == 8)
			{
				wstring year = _value.substr(0, 4);
				wstring month = _value.substr(4, 2);
				wstring date = _value.substr(6, 2);

				_value = month + wstring(L"/") + date + wstring(L"/") + year;
			}
		}
		wcsncpy_s(pLastUpdatedDates[i].szInfo, sizeof(pLastUpdatedDates[i].szInfo)/sizeof(wchar_t), _value.c_str(), _TRUNCATE);

		g_log.Log(CELOG_DEBUG, L"read last updated date, component name: %s, value: %s\n", pLastUpdatedDates[i].szComponentName, pLastUpdatedDates[i].szInfo);
	}

	nCount = static_cast<int> (vLastUpdatedDates.size());
	return TRUE;
}

/*

parameter:

pair<wstring, wstring>	--	installed component name, such as L"Windows Desktop Enforcer", version of the component.

vector<>	--	all name and version pairs of installed components installed on current machine.

return result:

true	--	get these information succeed
false	--	get these information failed

*/
BOOL CInstalledComptMgrImp::GetComponentsVersion( LPCOMPONENTINFO pVersions, int& nCount)
{
	wstring keyName = L"ProductVersion";

	vector< pair<wstring, wstring> >  vVersion;
	TraverseProductRegistry(vVersion, keyName);

	if((int)(vVersion.size()) > nCount)
	{
		nCount = static_cast<int>(vVersion.size());
		return FALSE;
	}

	for(unsigned i = 0; i < vVersion.size(); i ++)
	{
		memset(&(pVersions[i]), 0, sizeof(COMPONENTINFO));
		wcsncpy_s(pVersions[i].szComponentName, sizeof(pVersions[i].szComponentName)/sizeof(wchar_t), vVersion[i].first.c_str(), _TRUNCATE);
		wcsncpy_s(pVersions[i].szInfo, sizeof(pVersions[i].szInfo)/sizeof(wchar_t), vVersion[i].second.c_str(), _TRUNCATE);
	}

	nCount = static_cast<int>(vVersion.size());
	return TRUE;

}



BOOL CInstalledComptMgrImp::TraverseProductRegistry(vector< pair<wstring, wstring> >  & vComponentInfo, const wstring & keyName)
{
	BOOL bRet = EnumProducts(vComponentInfo, keyName, L"Software\\Nextlabs\\");

#ifdef _WIN64
	//We need to enumerate L"Software\\Wow6432Node\\Nextlabs\\" too
	vector< pair<wstring, wstring> > vInfo32;
	vInfo32.clear();
	bRet = EnumProducts(vInfo32, keyName, L"Software\\Wow6432Node\\Nextlabs\\");

	for(int i = 0; i < vInfo32.size(); i++)
	{
		vComponentInfo.push_back(pair<wstring, wstring>(vInfo32[i].first, vInfo32[i].second));
	}

#endif

	return bRet;
}

BOOL CInstalledComptMgrImp::EnumProducts(vector< pair<wstring, wstring> > & vComponentInfo, const wstring & keyName, const wstring& registryKey)
{
	//	check all key under \\Nextlabs,

	//	first, enum all sub name under \\nextlabs, such as:
	//	Compliant Enterprise, Enterprise DLP,	.....
	vector<wstring> vProductLines;
	if (!EnumProductLine(vProductLines))
	{
		g_log.Log(CELOG_DEBUG, L"error when try to enum all sub name under \\nextlabs\n");
		return FALSE;
	}
	
	//	enum all sub product name under each product line
	for (DWORD dwLineNo = 0; dwLineNo < vProductLines.size(); dwLineNo++)
	{
		//	first, open key like \\Nextlabs\\Compliant Enterprise
		wstring baseEntryName = registryKey + vProductLines[dwLineNo];
	HKEY hKey = NULL;
		LONG rstatus = RegOpenKeyExW(HKEY_LOCAL_MACHINE,baseEntryName.c_str(),0,KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS,&hKey);
	if( rstatus != ERROR_SUCCESS )
	{
			g_log.Log(CELOG_DEBUG, L"failed to open key for %s\n", baseEntryName.c_str());
			return FALSE;
	}

		//	second, query all sub key of the opened baseEntryName
	//	open each sub key, each sub key is a installed component
	for (DWORD i = 0; ; i++)
	{
		wchar_t subkeyname[256] = {0};
		DWORD subkeynameLen = 256;

		//	do query
		rstatus = RegEnumKeyEx(
			hKey,
			i,
			subkeyname,
			&subkeynameLen,
			NULL,
			NULL,
			NULL,
			NULL);
		if( rstatus != ERROR_SUCCESS )
		{
			//	failed to get sub key of \\Nextlabs,
			//	check if there is no more sub key of \\Nextlabs
			if (ERROR_NO_MORE_ITEMS == rstatus)
			{
					//	yes, there is no more sub key
				//	do free resource, then break to continue work flow.	
	
					g_log.Log(CELOG_DEBUG, L"no more product under %s, go to next line\n", baseEntryName.c_str());

				RegCloseKey(hKey);
				hKey = NULL;
					break;
			}
			else
			{
				//	no, there is something wrong
					g_log.Log(CELOG_DEBUG, L"error when try to enum under %s, error %d\n", baseEntryName.c_str(), rstatus);
				RegCloseKey(hKey);
				hKey = NULL;
				return FALSE;
			}
		}

		//	succeed to get a sub key, this is component name
			g_log.Log(CELOG_DEBUG, L"get a product %s under %s\n", subkeyname,baseEntryName.c_str());
		wstring compName(subkeyname);

			//	get full name in registry.
			wstring fullEntryName = baseEntryName + L"\\" + compName;

#ifdef _WIN64
		wstring strTemp = fullEntryName;
		std::transform(strTemp.begin(), strTemp.end(), strTemp.begin(), tolower);
		const wstring strPattern(L"software\\wow6432node\\");
		wstring::size_type nIndex = strTemp.find(strPattern);
		g_log.Log(CELOG_DEBUG, L"Check if %s exists in software\\nextlabs\r\n", fullEntryName.c_str());
		if(nIndex != wstring::npos)
		{//we need to check if this key exists in "Software\\Nextlabs\\"
			strTemp.replace(nIndex, strPattern.length(), L"software\\");

			HKEY hKeyTemp = NULL;
			LONG rt = RegOpenKeyExW(HKEY_LOCAL_MACHINE,strTemp.c_str(),0,KEY_QUERY_VALUE,&hKeyTemp);
			if(rt == ERROR_SUCCESS)
			{//find the same information in "Software\\Nextlabs\\", we don't need to handle it again.
				RegCloseKey(hKeyTemp);
				g_log.Log(CELOG_DEBUG, L"find %s in software\\nextlabs, we don't need to handle this key in wow6432. %s\r\n", fullEntryName.c_str(), strTemp.c_str());
				continue;
			}
		}
#endif
		
		//	open component entry to query key
		HKEY hComponentKey = NULL;
		rstatus = RegOpenKeyExW(HKEY_LOCAL_MACHINE,fullEntryName.c_str(),0,KEY_QUERY_VALUE,&hComponentKey);
		if( rstatus != ERROR_SUCCESS )
		{
				g_log.Log(CELOG_DEBUG, L"error when try to open %s, error %d\n", fullEntryName.c_str(), rstatus);
				return FALSE;
		}

		//	opened, query key value
		wchar_t keyValue[1024] = {0};
		DWORD keyValueLen = 1024;
		rstatus = RegQueryValueExW(hComponentKey,keyName.c_str(),NULL,NULL,(LPBYTE)keyValue,&keyValueLen);
		if( rstatus != ERROR_SUCCESS )
		{
			if(_wcsicmp(keyName.c_str(), L"LastUpdated") == 0)//if use control panel to read 5.5 products, control panel can't read "LastUpdated" since 5.5 doesn't have this key
			{
				g_log.Log(CELOG_DEBUG, "no LastUpdated, try to look up ProductCode\n");
				keyValueLen = 1024;
				RegQueryValueExW(hComponentKey,L"ProductCode",NULL,NULL,(LPBYTE)keyValue,&keyValueLen);//try to read "ProductCode" value
			}
		}

		//read product name
		const wchar_t keyProductName[] = L"ProductName";
		wchar_t valueProductName[1024] = {0};
		keyValueLen = 1024;
		rstatus = RegQueryValueExW(hComponentKey, keyProductName, NULL, NULL, (LPBYTE)valueProductName, &keyValueLen);
		g_log.Log(CELOG_DEBUG, L"Read product name: %s\n", valueProductName);


		//	close key
		RegCloseKey(hComponentKey);

		//	insert
		wstring strkeyValue(keyValue);
		wstring strKeyName;
		if (wcslen(valueProductName) > 0)
		{
			strKeyName = wstring(valueProductName);//Control Panel can read this info after 6.0 (including 6.0)
		}
		else
		{
			strKeyName = compName;//for 5.5
		}

		vComponentInfo.push_back( pair<wstring, wstring>(strKeyName, strkeyValue) );

			//	finish one, continue
			continue;
		}

		//	finish one, continue
		continue;
	}


	return TRUE;
}

BOOL CInstalledComptMgrImp::EnumProductLine( vector< wstring >  & vProductLines )
	{

	//	first, open nextlabs in registry
	HKEY hKey = NULL;
	LONG rstatus = RegOpenKeyExW(HKEY_LOCAL_MACHINE,L"Software\\Nextlabs",0,KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS,&hKey);
		if( rstatus != ERROR_SUCCESS )
		{
		g_log.Log(CELOG_DEBUG, L"can't open software\\nextlabs in registry\n");
		return false;
	}

	//	second, query all sub key of the opened \\Nextlabs
	//	open each sub key, each sub key is a installed component product line

	for (DWORD i = 0; ; i++)
	{
		wchar_t subkeyname[256] = {0};
		DWORD subkeynameLen = 256;

		//	do query
		rstatus = RegEnumKeyEx(
			hKey,
			i,
			subkeyname,
			&subkeynameLen,
			NULL,
			NULL,
			NULL,
			NULL);
		if( rstatus != ERROR_SUCCESS )
		{
			//	failed to get sub key of \\Nextlabs,
			//	check if there is no more sub key of \\Nextlabs
			if (ERROR_NO_MORE_ITEMS == rstatus)
			{
				//	yes, there is no more sub key of \\Nextlabs
				//	do free resource, then break to continue work flow.	

				g_log.Log(CELOG_DEBUG, L"no more subname under nextlabs in registry\n");
				RegCloseKey(hKey);
				hKey = NULL;
			return TRUE;
		}
			else
			{
				//	no, there is something wrong
				g_log.Log(CELOG_DEBUG, L"error %d when try to get a subname under nextlabs in registry\n", rstatus);
				RegCloseKey(hKey);
				hKey = NULL;
				return FALSE;
			}
		}


		//	succeed to get a sub key, this is product line name
		g_log.Log(CELOG_DEBUG, L"get a subname under nextlabs: %s in registry\n", subkeyname);
		wstring strProductLineName(subkeyname);
		vProductLines.push_back(strProductLineName);
	}

	//	here may not be reached
	if (hKey)
	{
		RegCloseKey(hKey);
		hKey = NULL;
	}

	return TRUE;
}

int CInstalledComptMgrImp::GetComponentsInstallDir(std::vector<pair<wstring,wstring> > &vInstallDirs)
{
	const wchar_t* szRootKey = L"SOFTWARE\\NextLabs\\Compliant Enterprise\\Policy Controller";
	HKEY hkeyRoot = NULL;
	LSTATUS rstatus = RegOpenKeyExW(HKEY_LOCAL_MACHINE,szRootKey,0,KEY_QUERY_VALUE,&hkeyRoot);
	if( (rstatus != ERROR_SUCCESS) || (hkeyRoot==NULL) )
	{
		return FALSE;
	}

	//Root Install Dir
	wchar_t szKeyValue[1024]={0};
	DWORD keyValueLen = 1024;
	rstatus = RegQueryValueExW(hkeyRoot, L"InstallDir", NULL, NULL, (LPBYTE)szKeyValue, &keyValueLen);
    if( rstatus == ERROR_SUCCESS)
	{
		vInstallDirs.push_back(std::pair<std::wstring,std::wstring>(L"RootDir", szKeyValue));
	}


	//PC
	memset(szKeyValue, 0, 1024*sizeof(szKeyValue[0]));
	keyValueLen = 1024;
    rstatus = RegQueryValueExW(hkeyRoot, L"PolicyControllerDir", NULL, NULL, (LPBYTE)szKeyValue, &keyValueLen);
    if( rstatus == ERROR_SUCCESS)
	{
		vInstallDirs.push_back(std::pair<std::wstring,std::wstring>(L"NextLabs Policy Controller", szKeyValue));
	}


	RegCloseKey(hkeyRoot);
	hkeyRoot=NULL;

	return vInstallDirs.size();
}
