/****************************************************************************************
 *
 * NextLabs Uninstall Cleanup Utility Tool
 *
 * January 2012 
 ***************************************************************************************/
#ifndef __NLSERVICECLEANUP_H__
#define __NLSERVICECLEANUP_H__

#include <windows.h>
#include <dbghelp.h>
#include "nl_baseCleanup.h"

class ServiceCleanup : public CleanupBase
{
public:
	ServiceCleanup(FILE* fp, bool flag) : CleanupBase(fp, flag) {};
	~ServiceCleanup(){};
public:
	virtual int createItems(itemList&);
	virtual int deleteItems(itemList&);
};
#endif /*  __NLServiceCleanup_H__  */