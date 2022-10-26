/****************************************************************************************
 *
 * NextLabs Tagging STL Map
 *
 * Feb 2012 by HK
 ***************************************************************************************/
#ifndef __NEXTLABSTAGMAP_H__
#define __NEXTLABSTAGMAP_H__

#include <string.h>
#include <iostream>
#include <map>
#include <utility>
#include <windows.h>
#include <dbghelp.h>
using namespace std;

struct cmp_wstr 
{
	bool operator()(std::wstring a, std::wstring b) const
   {
      //return cmp(a.c_str(), b.c_str()) < 0;
	  return std::wcscmp(a.c_str(), b.c_str()) > 0;
   }
};

class NL_Tagging_Map
{
public:
	NL_Tagging_Map(void) {};
	~NL_Tagging_Map(){};
public:
	int import_keys(wchar_t* itemData, size_t dataSize, size_t *numKeys);
	int export_keys(wchar_t* itemData, size_t* noItems, size_t *tagSize);
	int add_key(const wchar_t* key, const wchar_t* value);
	int delete_key(const wchar_t* key);
	int print_keys(wchar_t* itemData, size_t noItems);
private:
	map<wstring, wstring, cmp_wstr> items;
};
#endif /*  __NEXTLABSTAGMAP_H__  */