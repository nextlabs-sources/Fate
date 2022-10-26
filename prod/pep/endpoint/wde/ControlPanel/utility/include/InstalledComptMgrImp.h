#pragma once
#include "installedcomptmgr.h"

class CInstalledComptMgrImp :
	public CInstalledComptMgr
{
public:
	CInstalledComptMgrImp(void);
	virtual ~CInstalledComptMgrImp(void);

	/*

	parameter:

	pair<wstring, wstring>	--	installed component name, such as L"Windows Desktop Enforcer", install dir of the component

	vector<>	--	all name and dir pairs of installed components installed on current machine.

	return result:

	true	--	get these information succeed
	false	--	get these information failed

	*/
	virtual BOOL GetComponentsInstallDir( LPCOMPONENTINFO pInstallDirs, int& nCount);



	/*

	parameter:

	pair<wstring, wstring>	--	installed component name, such as L"Windows Desktop Enforcer", last updated (installed) date of the component.

	vector<>	--	all name and date pairs of installed components installed on current machine.

	return result:

	true	--	get these information succeed
	false	--	get these information failed

	*/
	virtual BOOL GetComponentsLastUpdatedDate( LPCOMPONENTINFO pLastUpdatedDates, int& nCount);



	/*

	parameter:

	pair<wstring, wstring>	--	installed component name, such as L"Windows Desktop Enforcer", version of the component.

	vector<>	--	all name and version pairs of installed components installed on current machine.

	return result:

	true	--	get these information succeed
	false	--	get these information failed

	*/
	virtual BOOL GetComponentsVersion( LPCOMPONENTINFO pVersions, int& nCount);


private:

	/*

	parameter:

	pair<wstring, wstring>	--	installed component name, such as L"Windows Desktop Enforcer", value string of the keyName parameter in registry of the component.

	vector<>	--	all name and value string pairs of installed components installed on current machine.

	keyName	--		key name under each component, such as InstallDir, ProductVersion, ProductCode

	return result:

	true	--	get these information succeed
	false	--	get these information failed

	*/
	BOOL TraverseProductRegistry(vector< pair<wstring, wstring> >  & vComponentInfo, const wstring & keyName);


	/*
	
	enum all sub name under \\nextlabs, such as: Compliant Enterprise, Enterprise DLP,	.....

	return result:

	true	--	get these information succeed
	false	--	get these information failed

	*/
	BOOL EnumProductLine( vector< wstring >  & vProductLines );

	/*
	enumerate all the products under our product keys, like: HKEY_LOCAL_MACHINE\SOFTWARE\NextLabs\Compliant Enterprise
	*/
	BOOL EnumProducts(vector< pair<wstring, wstring> >  & vComponentInfo, const wstring & keyName, const wstring& registryKey);

	int GetComponentsInstallDir(vector< pair<wstring, wstring> >  & vInstallDirs);
};
