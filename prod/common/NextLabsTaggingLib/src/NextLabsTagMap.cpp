/****************************************************************************************
 *
 * NextLabs Tagging STL Map
 *
 * Feb 2012 by HK
 ***************************************************************************************/
#include <windows.h>
#include <string>
#include <iostream>
#include <list>

#include <tchar.h>
#include <strsafe.h>
#include <aclapi.h>
#include <stdio.h>

#include "NextLabsTagMap.h"
#include <assert.h>

// import keys/values from wstring array
// input:
//	   itemData is array of wide strings
//     dataSize is number of chars in size
// output:
//     numKeys is number of keys
int NL_Tagging_Map::import_keys(wchar_t* itemData, size_t dataSize, size_t *numKeys)
{
	assert( dataSize % sizeof(wchar_t) == 0 );
	wchar_t* key	= NULL;
	wchar_t* value	= NULL;
	wchar_t* itemTmp = itemData;
	size_t len = 0;
    size_t noItems;

    unsigned long numStrings = 0;
    unsigned int i;

    // Count the number of strings.
    for (i = 0; i < dataSize / sizeof(wchar_t); i++)
    {
      if (itemData[i] == L'\0')
      {
		numStrings++;
      }
    }

	assert( numStrings % 2 == 0 );
	noItems = numStrings/2;
	for( size_t j=0; j<noItems; j++)
	{
		key = itemTmp;
        len = wcslen(key);
		itemTmp += len + 1;
		value = itemTmp;
		len = wcslen(value);
		itemTmp += len +1;
		assert(key);
		assert(value);
		add_key(key, value);    
	}
	*numKeys = noItems;
	return 0;
}
// export keys/values to wstring array
// input:
// output:
//	   itemData is array of wide strings
//     noItems is number of keys
//     dataSize is size of array in terms of chars
int NL_Tagging_Map::export_keys(wchar_t* itemData, size_t* noItems, size_t *dataSize)
{
	size_t keylen, vallen;
	*noItems = items.size();
	size_t tmpTagSize = (size_t) itemData;
    for( map<wstring,wstring, cmp_wstr>::iterator ii=items.begin(); ii!=items.end(); ++ii)
    {
		keylen = wcslen((*ii).first.c_str());
		vallen = wcslen((*ii).second.c_str());
		wcsncpy_s(itemData, keylen + 1, (*ii).first.c_str(), _TRUNCATE);
		itemData += keylen+1;
		wcsncpy_s(itemData, vallen + 1, (*ii).second.c_str(), _TRUNCATE);
		itemData += vallen+1;
    }
	// get number of chars
	*dataSize = (size_t) itemData - tmpTagSize ;
	return 0;
}
int NL_Tagging_Map::add_key(const wchar_t* key, const wchar_t* value)
{
	assert(key);
	assert(value);
	std::map<wstring, wstring, cmp_wstr>::iterator it = items.find(wstring(key));
	if( it == items.end() )
	{
		// there is no same key: it is new key
		items.insert(std::pair<wstring,wstring>(key,value));
	} else
	{
		// found same key
		if( wcscmp(it->second.c_str(),value) != 0)
		{
			// new value
			it->second = wstring(value);
		} 
	}
	return 0;
}

int NL_Tagging_Map::delete_key(const wchar_t* key)
{
	assert(key);
	std::map<wstring, wstring, cmp_wstr>::iterator it = items.find(wstring(key));
	if( it != items.end() )
	{
		items.erase(it);
	} else
	{
		return 1; // not found
	}
	return 0;
}
// for debug
int NL_Tagging_Map::print_keys(wchar_t* itemData, size_t noItems)
{
	wchar_t* key	= NULL;
	wchar_t* value	= NULL;
	wchar_t* itemTmp = itemData;
	size_t len = (size_t) -1;
	printf("\n\tidx:\tkey\tvalue");
	for( size_t i=0;i<noItems;i++)
	{
		itemTmp += len +1;
		key = itemTmp;
        len = wcslen(key);
		itemTmp += len + 1;
		value = itemTmp;
		len = wcslen(value);
		assert(key);
		assert(value);
		printf("\n\t%d:\t%ws\t%ws", i, key, value);  
	}
	return 0;
}