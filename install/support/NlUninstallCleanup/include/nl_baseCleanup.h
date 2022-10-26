/****************************************************************************************
 *
 * NextLabs Uninstall Cleanup Utility Tool
 * Cleanup Base Class
 * January 2012 
 ***************************************************************************************/
#ifndef __NLCLEANUPBASE_H__
#define __NLCLEANUPBASE_H__

#include <windows.h>
#include <dbghelp.h>
#include "nl_cleanup.h"

class CleanupBase
{
public:
	CleanupBase(FILE* fp, bool flag) : m_File(fp), m_Delete(flag)
	{
		if( flag )
			wcscpy_s(msg,20, L" ");
		else wcscpy_s(msg,20, L"-- Will Be");
		m_numErrors = 0;
	};
	~CleanupBase(){};
public:
	virtual int createItems(itemList&);
	virtual int deleteItems(itemList&);

	virtual int createItem(wstring);
	virtual int deleteItem(const wstring);
	void fprintfLastError(DWORD lastErr);
	void getLastErrorStr(DWORD lastErr, WCHAR *errstr);
protected:
	FILE* m_File;  // log file
	bool m_Delete; // real deletion flag
	wchar_t msg[20]; // default message
	WCHAR errStr[1024];	

	itemList m_deletepath;   // 
	itemList m_exceptions; //
	int m_numErrors;  // number of errors
};
#endif /*  __NLCLEANUPBASE_H__  */