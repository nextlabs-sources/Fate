/****************************************************************************************
 *
 * NextLabs Uninstall Cleanup Utility Tool
 *
 * January 2012 
 ***************************************************************************************/
#ifndef __NLREGISTRYCLEANUP_H__
#define __NLREGISTRYCLEANUP_H__

#include <windows.h>
#include <dbghelp.h>
#include "nl_baseCleanup.h"

class RegistryCleanup : public CleanupBase
{
public:
	RegistryCleanup(FILE* fp, bool flag) : CleanupBase(fp, flag) {};
	~RegistryCleanup(){};
public:
	virtual int createItems(itemList&);
	virtual int deleteItems(itemList&);

private:
	bool RegDelnodeRecurse (HKEY hKeyRoot, LPTSTR lpSubKey);
	void getRootKeySubkey(wstring&, wstring&, wstring);
	void QueryKey(HKEY hKey); 
	// getPredefined handle
	HKEY getPredefinedHandle(const std::wstring str );
};
#endif /*  __NLRegistryeCleanup_H__  */