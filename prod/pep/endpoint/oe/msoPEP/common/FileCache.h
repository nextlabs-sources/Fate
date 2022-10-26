

#pragma once

#ifndef _INCLUDE_FILECACHE
#define _INCLUDE_FILECACHE

#include <vector>
#include <string>

#define MAX_FILECACHE_SIZE      500

class CFileCache
{
public:
    CFileCache(bool bRealPath = true);
    virtual ~CFileCache();

    void AddFile(const WCHAR* wzFilePath);
    bool FindByName(const WCHAR* wzFileName, std::wstring& strFilePath,_In_ long nFileSize = 0);

    std::wstring GetTempFileBySourcePath(const std::wstring& wstrSourcePath);
protected:
    bool AtomOperator(int nAct, const WCHAR* wzParam1=NULL, WCHAR* wzParam2=NULL,_In_ long nFileSize = 0);
    // Add two functions to find path
    // Gavin Ye June2408
    bool FindRealPathByName(LPCWSTR pwzName, LPWSTR pwzPath, size_t size=MAX_PATH, _In_ long nFileSize = 0);
    bool FindTempPathByName(LPCWSTR pwzName, LPWSTR pwzPath, size_t size=MAX_PATH);
private:
    bool bActive;
    bool m_bRealPath;
    std::vector<std::wstring>   m_filecache;
    CRITICAL_SECTION            m_cs;
};

class CStringCache
{
public:
    CStringCache(unsigned int maxsize = 0);
    ~CStringCache();

    bool add(const WCHAR* str);
    bool remove(const WCHAR* str);
    bool find(const WCHAR* str);
protected:
    bool AtomOperator(int nAct, const WCHAR* str);
private:
    std::vector<std::wstring>   m_cache;
    unsigned int                m_cachesize;
    unsigned int                m_maxsize;
    CRITICAL_SECTION            m_cs;
};


class CMeetingItemCache
{
public:
	CMeetingItemCache();
	~CMeetingItemCache();
private: 
	CRITICAL_SECTION            m_cs;
	std::vector<std::pair<std::wstring, std::wstring>>		m_vecCache;
public:
	bool AddMIMember(const std::wstring& wstrEID, const std::wstring& wstrConvIndex);
	bool DeleteMIMember(const std::wstring& strEID);
	std::wstring GetEIDFromConvIndex(const std::wstring& wstrConvIndex);
};
#endif