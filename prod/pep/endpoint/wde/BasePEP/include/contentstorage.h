#ifndef __WDE_CONTENTSTORAGE_H__
#define __WDE_CONTENTSTORAGE_H__

#ifdef _MSC_VER
#pragma once
#else
#error "contentstorage.h only supports windows-compile"
#endif // _MSC_VER

#include <windows.h>
#include <string>

#include "nlconfig.hpp"
#include "commonlib_helper.h"

namespace nextlabs
{

class CContextStorage
{
public:
	CContextStorage(){

	}

public:
	bool StoreClipboardInfo(const std::wstring& info);
    bool GetClipboardInfo(std::wstring& info);

    bool StoreDragDropContentFileInfo(const std::wstring& filePath);
    bool GetDragDropContentFileInfo(std::wstring& filePath);

private:
	std::wstring GetCommonComponentsDir();
	bool Init();

private:
	static HMODULE m_hNLStorage;
	static nextlabs::nl_CacheData m_fnCacheData;
	static nextlabs::nl_GetData m_fnGetData;
	static nextlabs::nl_FreeMem m_fnFreeMem;

    bool Set(const std::string& key, const std::string& value);
    bool Get(const std::string& key, std::string& value);
private:
	static const wchar_t* m_basepepContent;
};

}  // ns nextlabs

#endif //__WDE_CONTENTSTORAGE_H__



