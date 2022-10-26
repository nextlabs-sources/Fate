/****************************************************************************************
 *
 * NextLabs Uninstall Cleanup Utility Tool
 *
 * January 2012 
 ***************************************************************************************/
#ifndef __NLFILECLEANUP_H__
#define __NLFILECLEANUP_H__

#include <windows.h>
#include <dbghelp.h>
#include "nl_baseCleanup.h"

class FileCleanup : public CleanupBase
{
public:
	FileCleanup(FILE* fp, bool flag) : CleanupBase(fp, flag) {};
	~FileCleanup(){};
public:
	virtual int createItems(itemList&);
	virtual int deleteItems(itemList&);
	virtual int createItem(wstring);
	virtual int deleteItem(const wstring);

private:
	bool traverse( __in const wchar_t* path  );
	int IsSubpathOfExceptionPaths(const wstring deletestr);
	int getExceptionItems(itemList& items);
	int IsSubpathOfExceptionPath(const wstring deletepath, const wstring exceptionpath);
	int remove_recursive( __in const wchar_t* path );
};
#endif /*  __FILECLEANUP_H__  */