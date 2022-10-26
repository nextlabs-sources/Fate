#include "contentstorage.h"

namespace
{
    const std::string kStorageClipboard = "STORAGECLIPBOARD";
    const std::string kDragDropContent  = "STORAGEDRAGDROPCONTENT";
}  // ns anonymous


namespace nextlabs
{

HMODULE CContextStorage::m_hNLStorage = NULL;
nextlabs::nl_CacheData CContextStorage::m_fnCacheData = NULL;
nextlabs::nl_GetData CContextStorage::m_fnGetData = NULL;
nextlabs::nl_FreeMem CContextStorage::m_fnFreeMem = NULL;

const wchar_t* CContextStorage::m_basepepContent = L"basepep_content";

std::wstring CContextStorage::GetCommonComponentsDir()
{
	wchar_t szDir[MAX_PATH + 1] = {0};
	if(NLConfig::ReadKey(L"SOFTWARE\\NextLabs\\CommonLibraries\\InstallDir", szDir, MAX_PATH))
	{
#ifdef _M_IX86
		wcsncat_s(szDir, MAX_PATH, L"\\bin32", _TRUNCATE);
#else
		wcsncat_s(szDir, MAX_PATH, L"\\bin64", _TRUNCATE);
#endif
		return szDir;
	}
	return L"";
}

bool CContextStorage::Init()
{
	if (m_hNLStorage != NULL)
	{
		return true;
	}

	std::wstring strCommonPath = GetCommonComponentsDir();

#ifdef _WIN64
	std::wstring strLib = strCommonPath + L"\\nlcommonlib.dll";
#else
	std::wstring strLib = strCommonPath + L"\\nlcommonlib32.dll";
#endif

	m_hNLStorage = LoadLibraryW(strLib.c_str());
	if (m_hNLStorage)
	{
		m_fnCacheData = (nextlabs::nl_CacheData)GetProcAddress(m_hNLStorage, "nl_CacheData");
		m_fnGetData = (nextlabs::nl_GetData)GetProcAddress(m_hNLStorage, "nl_GetData");
		m_fnFreeMem = (nextlabs::nl_FreeMem)GetProcAddress(m_hNLStorage, "nl_FreeMem");
	}

	if(m_hNLStorage == NULL || m_fnCacheData == NULL || m_fnGetData == NULL || m_fnFreeMem == NULL)	
	{
		return false;
	}

	return true;
}

bool CContextStorage::StoreClipboardInfo(const std::wstring& info)
{
    std::string sInfo = from_wstr(info);
    return Set(kStorageClipboard, sInfo);
}

bool CContextStorage::GetClipboardInfo(std::wstring& info)
{
    std::string sInfo;
    if (!Get(kStorageClipboard, sInfo))
    {
        return false;
    }
    
    info = from_str(sInfo);
    return true;
}

bool CContextStorage::StoreDragDropContentFileInfo(const std::wstring& filePath)
{
    std::string sInfo = from_wstr(filePath);
    return Set(kDragDropContent, sInfo);
}

bool CContextStorage::GetDragDropContentFileInfo(std::wstring& filePath)
{
    std::string sFilePath;
    if (!Get(kDragDropContent, sFilePath))
    {
        return false;
    }

    filePath = from_str(sFilePath);
    return true;
}

bool CContextStorage::Set(const std::string& key, const std::string& value)
{
	if (!Init())
	{
		return false;
	}
	
	nextlabs::cache_key _key;
	_key.set_value(m_basepepContent, (const unsigned char*)key.c_str(), key.length());

	int err = m_fnCacheData(&_key, (const unsigned char*)value.c_str(), value.length());
	if (err == ERR_SUCCESS)
	{
		return true;
	}

	return false;
}

bool CContextStorage::Get(const std::string& key, std::string& value)
{
	if (!Init())
	{
		return false;
	}

	nextlabs::cache_key _key;
	_key.set_value(m_basepepContent, (const unsigned char*)key.c_str(), key.length());

	unsigned char* buf = NULL;
	unsigned int len = 0;

	int err = m_fnGetData(&_key, false, &buf, &len);
	if(err != ERR_SUCCESS || buf == NULL)
	{
		return false;
	}
	
	value.assign((char*)buf, len);

	m_fnFreeMem(buf);

	return true;
}

}