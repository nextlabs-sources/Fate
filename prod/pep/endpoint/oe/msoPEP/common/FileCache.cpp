#include "stdafx.h"

#include <Windows.h>
#include "FileCache.h"
#include "Hook.h"
#include "../outlook/mailAttach.h"
#include "../include/msopath.hpp"

//////////////////////////////////////////////////////////////////////////
CFileCache::CFileCache(bool bRealPath)
{
    InitializeCriticalSection(&m_cs);
    bActive     = true;
    m_bRealPath = bRealPath;
}

CFileCache::~CFileCache()
{
    bActive = false;
    AtomOperator(0);
    DeleteCriticalSection(&m_cs);
}

void CFileCache::AddFile(const WCHAR* wzFilePath)
{
    AtomOperator(1, wzFilePath);
}

bool CFileCache::FindByName(const WCHAR* wzFileName, std::wstring& strFilePath,_In_ long nFileSize)
{
    bool bRet;
    WCHAR wzFilePath[1024]; memset(wzFilePath, 0, sizeof(wzFilePath));
    bRet = AtomOperator(2, wzFileName, wzFilePath, nFileSize);
    if(bRet) strFilePath = wzFilePath;
    return bRet;
}

wstring CFileCache::GetTempFileBySourcePath(const wstring& wstrSourcePath)
{
    wstring wstrTempRet = L"";
    CPath cSourcePath(wstrSourcePath);
    CPath cTempCache;
    for (std::vector<std::wstring>::reverse_iterator it = m_filecache.rbegin(); it != m_filecache.rend(); ++it)
    {
        cTempCache.Set(*it);
        if (0 == _wcsicmp(cTempCache.GetFileName().c_str(), cSourcePath.GetFileName().c_str()))
        {
            wstrTempRet = (*it);
            break;
        }
    }
    return wstrTempRet;
}

bool CFileCache::AtomOperator(int nAct, const WCHAR* wzParam1, WCHAR* wzParam2, _In_ long nFileSize)
{
    bool bRet = false;
    EnterCriticalSection(&m_cs);
    switch(nAct)
    {
    case 0:
        m_filecache.clear();
        break;
    case 1:
        if(wzParam1)
        {
            std::wstring strPath = wzParam1;
            if(m_filecache.size() >= MAX_FILECACHE_SIZE)
                m_filecache.erase(m_filecache.begin());
            m_filecache.push_back(strPath);
            if(m_bRealPath)
            {
                //DP((L"Add real path: %s\n", strPath.c_str()));
            }
            bRet = true;
        }
        break;
    case 2:
        // use new function to find path
        // Gavin Ye June2408
        if(wzParam1 && 0<(int)m_filecache.size())
        {
            if(m_bRealPath)
            {
                bRet = FindRealPathByName(wzParam1, wzParam2, MAX_PATH,nFileSize);
            }
            else
            {
                bRet = FindTempPathByName(wzParam1, wzParam2);
            }
        }
        break;
    default:
        bRet = false;
        break;
    }
    LeaveCriticalSection(&m_cs);
    return bRet;
}

// Add function to find real path
// Gavin Ye June2408
bool CFileCache::FindRealPathByName(LPCWSTR pwzName, LPWSTR pwzPath, size_t size, _In_ long nFileSize)
{
    if(NULL == pwzName)
        return false;


	BOOL bIsMsgFile = IsMsgFile(pwzName);
    for(std::vector<std::wstring>::reverse_iterator it=m_filecache.rbegin();it!=m_filecache.rend(); ++it)
    {
        // Get File Name from Path
        const WCHAR* pFileName = wcsrchr((*it).c_str(), L'\\');
        if(NULL==pFileName) pFileName = (*it).c_str();
        else pFileName++;

        // Compare if the File Name is the same
        if(0 == wcscmp(pFileName, pwzName))
        {
			DWORD dwFileSize = 0;
			HANDLE hFile = CallNextCreateFileFunction((*it).c_str(),GENERIC_READ,
				FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL ,NULL);
			if (hFile != INVALID_HANDLE_VALUE)
			{
				dwFileSize = GetFileSize(hFile,NULL);
				CloseHandle(hFile);
			}
            
			if (dwFileSize == (DWORD)nFileSize || bIsMsgFile)
			{
				wcsncpy_s(pwzPath, size, (*it).c_str(), _TRUNCATE);
				return true;
			}
            //m_filecache.erase(it.base());
        }
    }

    return false;
}

// Add function to find temp path
// Gavin Ye June2408
bool CFileCache::FindTempPathByName(LPCWSTR pwzName, LPWSTR pwzPath, size_t size)
{
    std::wstring strResolvedName = L"";
    WCHAR wzName[MAX_PATH+1];
    WCHAR wcCharacter = 0;
    WCHAR *pwzPrefix=0, *pwzSuffix=0;
    int   nPrefixLen = 0;
    int i=0, j=0;

    WCHAR wzCacheName[MAX_PATH+1];
    WCHAR *pwzCachePrefix=0, *pwzCacheSuffix=0;
    int   nCachePrefixLen = 0;

    // Gavin Ye
    // remove '&', '=', '[', ']', ';', ','
    // it's very interesting that outlook will remove these characters when it create temp file from original name
    // so we have to do the same work manually
    memset(wzName, 0, sizeof(wzName));
    while (pwzName[i] && i<MAX_PATH)
    {
        wcCharacter = pwzName[i++];
        if(L'&'==wcCharacter || L'='==wcCharacter || L'['==wcCharacter || L']'==wcCharacter || L','==wcCharacter)
            continue;
        if(L';'==wcCharacter)   // outlook won't remove ';', it replaces it with ' '. Crazy M$
        {
            wzName[j++] = L' ';
            continue;
        }
		if(L'.'==wcCharacter)
		{
			if(wcschr(&pwzName[i],L'.'))
			{
				wzName[j++]=L' ';
				continue;
			}
		}
        wzName[j++] = wcCharacter;
    }
    strResolvedName= wzName;

    // Get Suffix and Prefix
    pwzPrefix = wzName;
    pwzSuffix = wcsrchr(wzName, L'.');
    if(NULL==pwzSuffix)
    {
        pwzSuffix = &wzName[j];  // wzName[j] is the end of string '\0'
    }
    else
    {
        *pwzSuffix = 0;
        pwzSuffix++;
    }
    nPrefixLen = (int)wcslen(pwzPrefix);

    // search in the cache
    for(std::vector<std::wstring>::reverse_iterator it=m_filecache.rbegin();it!=m_filecache.rend(); ++it)
    {
        // Get File Name from Path
        const WCHAR* pFileName = wcsrchr((*it).c_str(), L'\\');
        if(NULL==pFileName) pFileName = (*it).c_str();
        else pFileName++;

        // First, if the name is equal to resolved name?
        if(0 == _wcsicmp(strResolvedName.c_str(), pFileName))
        {
            wcsncpy_s(pwzPath, size, (*it).c_str(), _TRUNCATE);
            return true;    // OK, we get it!
        }

        // Create temp path name
        wcsncpy_s(wzCacheName, MAX_PATH, pFileName, _TRUNCATE);
        nCachePrefixLen = (int)wcslen(wzCacheName);

        // Get Suffix and Prefix
        pwzCachePrefix = wzCacheName;
        pwzCacheSuffix = wcsrchr(wzCacheName, L'.');
        if(NULL==pwzCacheSuffix)
        {
            pwzCacheSuffix = wzCacheName+nCachePrefixLen;
        }
        else
        {
            *pwzCacheSuffix = 0;
            pwzCacheSuffix++;
        }
        nCachePrefixLen = (int)wcslen(pwzCachePrefix);

        // Second, maybe the file name contain a number suffix
        // if original file name is a.doc, then new file name may like a (1).doc
        // A. compare suffix
        if(0!=_wcsicmp(pwzCacheSuffix, pwzSuffix))       // suffix is not equal
            continue;
        // B. compare prefix
        if(0 != _wcsnicmp(pwzCachePrefix, pwzPrefix, nPrefixLen))
            continue;                                   // Base prefix is not equal
        // Number suffix is like a (1).doc
        if(L' '!=pwzCachePrefix[nPrefixLen]             // number suffix should start with ' '
        || L'('!=pwzCachePrefix[nPrefixLen+1]           // then '('
        || L')'!=pwzCachePrefix[nCachePrefixLen-1]      // and end with ')'
            )
            continue;                                   // number suffix format is not correct
        // Now we think the name is equal
        wcsncpy_s(pwzPath, size, (*it).c_str(), _TRUNCATE);
        return true;    // OK, we get it!
    }

    return false;
}


//////////////////////////////////////////////////////////////////////////
CStringCache::CStringCache(unsigned int maxsize)
{
    m_cachesize = 0;
    m_maxsize = maxsize;
    InitializeCriticalSection(&m_cs);
}

CStringCache::~CStringCache()
{
    AtomOperator(0, NULL);
    DeleteCriticalSection(&m_cs);
}

bool CStringCache::add(const WCHAR* str)
{
    return AtomOperator(1, str);
}

bool CStringCache::find(const WCHAR* str)
{
    return AtomOperator(2, str);
}

bool CStringCache::remove(const WCHAR* str)
{
    return AtomOperator(3, str);
}

bool CStringCache::AtomOperator(int nAct, const WCHAR* str)
{
    bool bRet = false;
    bool bFind= false;
    if(nAct && !str) return false;
    unsigned int  usStrLen = str?(unsigned int)wcslen(str) : 0;
    unsigned int  i = 0;
    std::vector<std::wstring>::iterator it;

    EnterCriticalSection(&m_cs);

    switch(nAct)
    {
    case 0:
        m_cache.clear();
        break;
    case 1:
        for(i=0; i<m_cachesize; i++)
        {
            if(usStrLen==m_cache[i].length() && 0==wcscmp(str, m_cache[i].c_str()))
            {
                bFind = true;
                break;
            }
        }
        if(!bFind)
        {
            if(0!=m_maxsize && m_cachesize>=m_maxsize)
            {
                m_cache.erase(m_cache.begin());
                m_cache.push_back(str);
            }
            else
            {
                m_cache.push_back(str);
                m_cachesize++;
            }
            bRet = true;
        }
        break;
    case 2:
        for(i=0; i<m_cachesize; i++)
        {
            if(usStrLen==m_cache[i].length() && 0==wcscmp(str, m_cache[i].c_str()))
            {
                bFind = true;
                break;
            }
        }
        bRet = bFind;
        break;
    case 3:
        it=m_cache.begin();
        for(i=0; i<m_cachesize; i++)
        {
            if(usStrLen==m_cache[i].length() && 0==wcscmp(str, m_cache[i].c_str()))
            {
                bFind = true;
                m_cache.erase(it);
                break;
            }
            it++;
        }
        bRet = bFind;
        break;
    default:
        bRet = false;
        break;
    }
    LeaveCriticalSection(&m_cs);
    return bRet;
}

CMeetingItemCache::CMeetingItemCache()
{
	InitializeCriticalSection(&m_cs);
}

CMeetingItemCache::~CMeetingItemCache()
{
	DeleteCriticalSection(&m_cs);
}

bool CMeetingItemCache::AddMIMember(const std::wstring& wstrEID, const std::wstring& wstrConvIndex)
{
	bool bExisted = false;
	EnterCriticalSection(&m_cs);
	for (std::vector<std::pair<std::wstring, std::wstring>>::iterator it = m_vecCache.begin(); it!= m_vecCache.end(); it++)
	{
		if (it->first == wstrEID)
		{
			bExisted = true;
			break;
		}
	}
	if (!bExisted)
		m_vecCache.push_back(std::pair<std::wstring, std::wstring>(wstrEID,wstrConvIndex));
	LeaveCriticalSection(&m_cs);
	
	return true;
}

bool CMeetingItemCache::DeleteMIMember(const std::wstring& wstrEID)
{
	EnterCriticalSection(&m_cs);
	for (std::vector<std::pair<std::wstring, std::wstring>>::iterator it = m_vecCache.begin(); it!= m_vecCache.end(); it++)
	{
		if (it->first == wstrEID)
		{
			m_vecCache.erase(it);
			break;
		}
	}
	LeaveCriticalSection(&m_cs);
	
	return true;
}

std::wstring CMeetingItemCache::GetEIDFromConvIndex(const std::wstring& wstrConvIndex)
{
	std::wstring wstrRet;
	EnterCriticalSection(&m_cs);
	for (std::vector<std::pair<std::wstring, std::wstring>>::iterator it = m_vecCache.begin(); it!= m_vecCache.end(); it++)
	{		
		if (wstrConvIndex.find(it->second) != std::wstring::npos)
		{
			wstrRet = it->first; 
		}
	}
	LeaveCriticalSection(&m_cs);

	return wstrRet;
}