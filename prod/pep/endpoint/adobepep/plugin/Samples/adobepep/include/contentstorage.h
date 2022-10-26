#pragma once

#include "nlconfig.hpp"
#include "commonlib_helper.h"

class CContextStorage
{
public:
	CContextStorage(){

	}

public:
    bool StoreDragDropContentFileInfo(const std::string& filePath);
    bool GetDragDropContentFileInfo(std::string& filePath);

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


