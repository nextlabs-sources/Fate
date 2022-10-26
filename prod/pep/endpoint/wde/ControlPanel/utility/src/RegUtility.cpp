#include "StdAfx.h"
#include "RegUtility.h"

CRegUtility::CRegUtility(void)
{
}

CRegUtility::~CRegUtility(void)
{
}


BOOL CRegUtility::OpenEDPMKey(HKEY* pKey)
{
	//	try to open it first
	LONG rstatus = RegOpenKeyExW(HKEY_CURRENT_USER,EDPM_REG_DIR,0,KEY_SET_VALUE | KEY_QUERY_VALUE,pKey);
	if (rstatus == ERROR_SUCCESS)
	{
		//	succeed
		//g_log.Log(CELOG_DEBUG, L"OpenEDPMKey with set value permission succeed\n");
		return TRUE;
	}
	

	//	there is no EDP Manager setting key, we need to create one
	HKEY hSoftwareRoot = NULL;
	rstatus = RegOpenKeyExW(HKEY_CURRENT_USER,EDPM_REG_SOFTWARE_ROOT_DIR,0,KEY_CREATE_SUB_KEY,&hSoftwareRoot);
	if (rstatus != ERROR_SUCCESS)
	{
		//	failed, check reason
		if (rstatus == ERROR_ACCESS_DENIED)
		{
			g_log.Log(CELOG_DEBUG, L"try to open //Software failed, error code: no permission\n");
		}
		else
		{
			g_log.Log(CELOG_DEBUG, L"try to open //Software failed, error code %d\n", rstatus);
		}

		//	try to get permission for creating sub key failed
		//	we have to return.
		return FALSE;
	}

	//	create \\Software\\Nextlabs
	HKEY hNextlabsRoot = NULL;
	rstatus = RegCreateKeyEx(
		hSoftwareRoot,
		EDPM_REG_NXTLABS,
		0,
		0,
		0,
		KEY_CREATE_SUB_KEY,
		NULL,
		&hNextlabsRoot,
		NULL);

	if (rstatus != ERROR_SUCCESS)
	{
		//	failed, check reason
		if (rstatus == ERROR_ACCESS_DENIED)
		{
			g_log.Log(CELOG_DEBUG, L"try to create //Software//Nextlabs failed, error code: no permission\n");
		}
		else
		{
			g_log.Log(CELOG_DEBUG, L"try to create //Software//Nextlabs failed, error code %d\n", rstatus);
		}

		//	try to get permission for creating sub key failed
		//	we have to return.
		return FALSE;
	}

	//	create Software\\Nextlabs\\EDP Manager 
	rstatus = RegCreateKeyEx(
		hNextlabsRoot,
		EDPM_REG_SUBROOT,
		0,
		0,
		0,
		KEY_SET_VALUE,
		NULL,
		pKey,
		NULL);

	if( rstatus != ERROR_SUCCESS )
	{
		g_log.Log(CELOG_DEBUG, L"try to creating sub key failed, error code %d\n", rstatus);
		return FALSE;
	}

	return TRUE;
}


BOOL CRegUtility::CloseEDPMKey(HKEY* pKey)
{
	RegCloseKey(*pKey);

	return TRUE;
}
