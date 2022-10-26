/****************************************************************************************
 *
 * NextLabs Uninstall Cleanup Utility Tool
 *
 * January 2012 
 ***************************************************************************************/
#ifndef __NLCONFIGFILE_H__
#define __NLCONFIGFILE_H__

#include <windows.h>
#include <dbghelp.h>

#include "nl_baseCleanup.h"

class ConfigFile : public CleanupBase
{
public:
	ConfigFile(FILE* fp, bool flag) : CleanupBase(fp, flag) {};
	~ConfigFile(){};
public:
	int read_config_file(std::wstring filename, FILE* logFile, ItemsList&);
};
#endif /*  __NLCONFIGFILE_H__  */