#pragma once

#include <vector>
#include <string>

using namespace std;

typedef struct struComponentInfo
{
	wchar_t szComponentName[200];
	wchar_t szInfo[MAX_PATH];
}COMPONENTINFO, *LPCOMPONENTINFO;

class CInstalledComptMgr
{
public:

	


	CInstalledComptMgr(void)
	{
	}

	virtual ~CInstalledComptMgr(void)
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
	virtual BOOL GetComponentsInstallDir( LPCOMPONENTINFO pInstallDirs, int& nCount) = 0;


	/*
	
	parameter:

	pair<wstring, wstring>	--	installed component name, such as L"Windows Desktop Enforcer", last updated (installed) date of the component.

	vector<>	--	all name and date pairs of installed components installed on current machine.

	return result:

	true	--	get these information succeed
	false	--	get these information failed
	
	*/
	virtual BOOL GetComponentsLastUpdatedDate( LPCOMPONENTINFO pLastUpdatedDates, int& nCount) = 0;


	/*

	parameter:

	pair<wstring, wstring>	--	installed component name, such as L"Windows Desktop Enforcer", version of the component.

	vector<>	--	all name and version pairs of installed components installed on current machine.

	return result:

	true	--	get these information succeed
	false	--	get these information failed

	*/
	virtual BOOL GetComponentsVersion( LPCOMPONENTINFO pVersions, int& nCount) = 0;
};
