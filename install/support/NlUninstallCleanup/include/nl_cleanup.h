/****************************************************************************************
 *
 * NextLabs Uninstall Cleanup Utility Tool
 *
 * Main Header
 *
 * Phase 1:   January 2012 by H.K.
 *    File and directory, Service, and Registry Cleanup
 * 
 ***************************************************************************************/
#ifndef __NLCLEANUP_H__
#define __NLCLEANUP_H__

#include <windows.h>
#include <dbghelp.h>
#include <vector>

using namespace std;

enum CleanType {
	CleanService = 0,
	CleanRegistry,
	CleanFileDir,

	// add more
	CleanLast
};
#define MAX_LENGTH 1024

#define CLEANUP_SUCCESS 0
#define CLEANUP_CFG_FILE_READ_ERROR 1   // stop
#define CLEANUP_CFG_FILE_PARSE_ERROR 2  // stop
#define CLEANUP_INTERNAL_ERROR 3        // stop
#define CLEANUP_REMOVAL_ERROR 4         // insufficient permission

typedef std::vector<wstring> itemList;
typedef std::vector<wstring>::iterator itemListIter;
typedef std::vector<wstring>::const_iterator itemListConstIter;

struct ItemsList
{
	itemList filepaths;  // get file or directory path list
	itemList services;   // service list
	itemList registries; // registry list
    // ...
	// add more
};

// this is a test
// #define TEST_DETAIL
#endif /*  __NLCLEANUP_H__  */