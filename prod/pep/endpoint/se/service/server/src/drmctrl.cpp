

#include <windows.h>
#include <iostream>
#include <fstream>
#include "drmctrl.hpp"


const unsigned char UTF8BOM[3] = {0xEF, 0xBB, 0xBF};
const unsigned char LINEEND[2] = {0x0D, 0x0A};

const std::wstring SE_SHARED_KEYRING_KEYW(L"SharedKeyRing=");
const std::wstring SE_DRM_PATH_KEYW(L"DRMDir=");
const std::wstring SE_NONDRM_PATH_KEYW(L"NONDRMDIR=");
const std::wstring SE_FWDRM_PATH_KEYW(L"DRMDirFW=");
const std::string  SE_SHARED_KEYRING_KEYA("SharedKeyRing=");
const std::string  SE_DRM_PATH_KEYA("DRMDir=");
const std::string  SE_NONDRM_PATH_KEYA("NONDRMDIR=");
const std::string  SE_FWDRM_PATH_KEYA("DRMDirFW=");


CDRMCtrl::CDRMCtrl()
{
    ::InitializeCriticalSection(&m_drmpaths_lock);
    ::InitializeCriticalSection(&m_nondrmpaths_lock);
    ::InitializeCriticalSection(&m_fwdrmpaths_lock);
}

CDRMCtrl::~CDRMCtrl()
{
    Close();
    ::DeleteCriticalSection(&m_drmpaths_lock);
    ::DeleteCriticalSection(&m_nondrmpaths_lock);
    ::DeleteCriticalSection(&m_fwdrmpaths_lock);
}

BOOL CDRMCtrl::Open(_In_ LPCWSTR wzFile)
{
    std::ifstream ifile;

    if(!EnsureFileExist(wzFile))
        return FALSE;

    m_file = wzFile;
    ifile.open(wzFile);
    if(!ifile.is_open()) return FALSE;

    // Clear old data
    ::EnterCriticalSection(&m_drmpaths_lock);
    m_drmpaths.clear();
    ::LeaveCriticalSection(&m_drmpaths_lock);
    ::EnterCriticalSection(&m_fwdrmpaths_lock);
    m_fwdrmpaths.clear();
    ::LeaveCriticalSection(&m_fwdrmpaths_lock);

    while(!ifile.eof())
    {
        std::string     strLine;
        std::wstring    wstrLine;
        std::getline(ifile, strLine);

        if(!Utf8ToWide(strLine, wstrLine))
            continue;

        if(boost::algorithm::istarts_with(wstrLine, SE_SHARED_KEYRING_KEYW))
        {
            wstrLine = wstrLine.substr(SE_SHARED_KEYRING_KEYW.length());
            m_skn = wstrLine;
        }
        else if(boost::algorithm::istarts_with(wstrLine, SE_DRM_PATH_KEYW))
        {
            wstrLine = wstrLine.substr(SE_DRM_PATH_KEYW.length());
            ::EnterCriticalSection(&m_drmpaths_lock);
            m_drmpaths.push_back(wstrLine);
            ::LeaveCriticalSection(&m_drmpaths_lock);
        }
        else if(boost::algorithm::istarts_with(wstrLine, SE_NONDRM_PATH_KEYW))
        {
            wstrLine = wstrLine.substr(SE_NONDRM_PATH_KEYW.length());
            ::EnterCriticalSection(&m_nondrmpaths_lock);
            m_nondrmpaths.push_back(wstrLine);
            ::LeaveCriticalSection(&m_nondrmpaths_lock);
        }
        else if(boost::algorithm::istarts_with(wstrLine, SE_FWDRM_PATH_KEYW))
        {
            wstrLine = wstrLine.substr(SE_FWDRM_PATH_KEYW.length());
            ::EnterCriticalSection(&m_fwdrmpaths_lock);
            m_fwdrmpaths.push_back(wstrLine);
            ::LeaveCriticalSection(&m_fwdrmpaths_lock);
        }
        else
        {
            continue;
        }
    }

    ifile.close();
    return TRUE;
}

void CDRMCtrl::Close()
{
    m_file = L"";
    m_skn  = L"";
    ::EnterCriticalSection(&m_drmpaths_lock);
    m_drmpaths.clear();
    ::LeaveCriticalSection(&m_drmpaths_lock);
    ::EnterCriticalSection(&m_nondrmpaths_lock);
    m_nondrmpaths.clear();
    ::LeaveCriticalSection(&m_nondrmpaths_lock);
    ::EnterCriticalSection(&m_fwdrmpaths_lock);
    m_fwdrmpaths.clear();
    ::LeaveCriticalSection(&m_fwdrmpaths_lock);
}

BOOL CDRMCtrl::SetSharedKeyRing(_In_ LPCWSTR wzKeyRing, _In_ BOOL AutoCommit)
{
    m_skn = wzKeyRing;
    if(AutoCommit) return Commit();
    return TRUE;
}

BOOL CDRMCtrl::Commit()
{
    BOOL bRet = TRUE;
    std::string strUtf8Line;
    HANDLE      hFile = INVALID_HANDLE_VALUE;
    std::list<std::wstring>::iterator it;

    // Open file
    hFile = ::CreateFileW(m_file.c_str(), GENERIC_READ|GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if(INVALID_HANDLE_VALUE == hFile)
        return FALSE;

    // Set file size to 0
    SetFilePointer(hFile, 0, 0, FILE_BEGIN);
    SetEndOfFile(hFile);

    // Write shared key ring
    if(m_skn.length() > 0)
    {
        strUtf8Line = BuildSharedKeyRingLine(m_skn);
        bRet = WriteLine(hFile, strUtf8Line);
        if(!bRet) goto _exit;
    }

    // Write DRM Path
    ::EnterCriticalSection(&m_drmpaths_lock);
    for(it=m_drmpaths.begin(); it!=m_drmpaths.end(); ++it)
    {
        strUtf8Line = BuildDrmPathLine(*it);
        bRet = WriteLine(hFile, strUtf8Line);
        if(!bRet) break;
    }
    ::LeaveCriticalSection(&m_drmpaths_lock);
    if(!bRet) goto _exit;

    // Write NonDRM Path
    ::EnterCriticalSection(&m_nondrmpaths_lock);
    for(it=m_nondrmpaths.begin(); it!=m_nondrmpaths.end(); ++it)
    {
        strUtf8Line = BuildNonDrmPathLine(*it);
        bRet = WriteLine(hFile, strUtf8Line);
        if(!bRet) break;
    }
    ::LeaveCriticalSection(&m_nondrmpaths_lock);
    if(!bRet) goto _exit;

    // Write FastWrite DRM Path
    ::EnterCriticalSection(&m_fwdrmpaths_lock);
    for(it=m_fwdrmpaths.begin(); it!=m_fwdrmpaths.end(); ++it)
    {
        strUtf8Line = BuildFwDrmPathLine(*it);
        bRet = WriteLine(hFile, strUtf8Line);
        if(!bRet) break;
    }
    ::LeaveCriticalSection(&m_fwdrmpaths_lock);

_exit:
    if(hFile != INVALID_HANDLE_VALUE) CloseHandle(hFile);
    return bRet;
}

BOOL CDRMCtrl::AddPath(_In_ LPCWSTR wzPath, _In_ BOOL FastWrite, _In_ BOOL AutoCommit)
{
    BOOL bRet = FALSE;
    std::wstring wstrPath(wzPath);
	BOOL bPathHasInclude=FALSE;

    // Check confliction
    if(FastWrite)
    {
        ::EnterCriticalSection(&m_fwdrmpaths_lock);
        if(!ConflictPath(wstrPath, m_drmpaths) && !ExistPath(wstrPath, m_fwdrmpaths))
		{
            bRet = AddPathToList(wstrPath, m_fwdrmpaths);
		}
		else	bPathHasInclude = TRUE;
        ::LeaveCriticalSection(&m_fwdrmpaths_lock);
    }
    else
    {
        ::EnterCriticalSection(&m_drmpaths_lock);
		if(!ConflictPath(wstrPath, m_fwdrmpaths) && !ExistPath(wstrPath, m_drmpaths))
		{
            bRet = AddPathToList(wstrPath, m_drmpaths);
		}
		else	bPathHasInclude = TRUE;
        ::LeaveCriticalSection(&m_drmpaths_lock);
    }
	// for bug 14885
	// if the path's parent or some path has been add to list ,we don't need to do anything.
	if(bPathHasInclude)	return bPathHasInclude;

    if(bRet && AutoCommit) bRet = Commit();
    return bRet;
}

BOOL CDRMCtrl::RemovePath(_In_ LPCWSTR wzPath, _In_ BOOL FastWrite, _In_ BOOL AutoCommit)
{
    BOOL bRet = FALSE;
    std::wstring wstrPath(wzPath);
    if(FastWrite)
    {        
        ::EnterCriticalSection(&m_fwdrmpaths_lock);
        bRet = RemovePathFromList(wstrPath, m_fwdrmpaths);
        ::LeaveCriticalSection(&m_fwdrmpaths_lock);
    }
    else
    {
        ::EnterCriticalSection(&m_drmpaths_lock);
        bRet = RemovePathFromList(wstrPath, m_drmpaths);
        ::LeaveCriticalSection(&m_drmpaths_lock);
    }

    if(bRet && AutoCommit) bRet = Commit();
    return bRet;
}

BOOL CDRMCtrl::HaveUtf8Bom(_In_ LPCWSTR wzFile)
{
    BOOL   bRet  = FALSE;
    HANDLE hFile = INVALID_HANDLE_VALUE;
    char   Bom[3]= {0, 0, 0};
    DWORD  dwRead= 0;

    hFile = ::CreateFileW(wzFile, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if(INVALID_HANDLE_VALUE == hFile)
        return FALSE;

    if(!ReadFile(hFile, Bom, 3, &dwRead, NULL))
        goto _exit;
    if(0 == memcmp(Bom, UTF8BOM, 3))
        bRet = TRUE;

_exit:
    if(hFile != INVALID_HANDLE_VALUE) CloseHandle(hFile);
    return bRet;
}

BOOL CDRMCtrl::RemoveUtf8Bom(_In_ LPCWSTR wzFile)
{
    BOOL   bRet     = FALSE;
    HANDLE hFile    = INVALID_HANDLE_VALUE;
    char   Bom[3]   = {0, 0, 0};
    DWORD  dwSize   = 0;
    DWORD  dwRead   = 0;
    DWORD  dwWritten= 0;
    DWORD  dwOffset = 0;
    char*  pbData   = NULL;
    const  DWORD ReadBlockSize = 65536;    // 64K

    hFile = ::CreateFileW(wzFile, GENERIC_READ|GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if(INVALID_HANDLE_VALUE == hFile)
        return FALSE;

    // Get file size
    dwSize = GetFileSize(hFile, NULL);

    // Check if there is UTF8 Bom
    if(dwSize < 3)
        goto _exit;
    if(!ReadFile(hFile, Bom, 3, &dwRead, NULL) || 3!=dwRead)
        goto _exit;
    if(0 != memcmp(Bom, UTF8BOM, 3))
        goto _exit;

    __try
    {
        pbData = new char[ReadBlockSize];
        if(NULL==pbData)
            __leave;

        // Remove this Bom
        dwOffset = 3;   // Ignore the Bom
        while(dwOffset < dwSize)
        {
            SetFilePointer(hFile, dwOffset, 0, FILE_BEGIN);

            // Read data
            if(ReadFile(hFile, pbData, ReadBlockSize, &dwRead, NULL))
            {
                SetFilePointer(hFile, dwOffset-3, 0, FILE_BEGIN);
                WriteFile(hFile, pbData, dwRead, &dwWritten, NULL);
            }

            // Reach end of file
            if(dwRead < ReadBlockSize)
                break;

            // Move to next block
            dwOffset += ReadBlockSize;
        }

        // Reset file size
        SetFilePointer(hFile, (dwSize-3), 0, FILE_BEGIN);
        SetEndOfFile(hFile);
        bRet = TRUE;
    }
    __finally
    {
        if(pbData) delete []pbData;
    }

_exit:
    if(hFile != INVALID_HANDLE_VALUE) CloseHandle(hFile);
    return bRet;
}

BOOL CDRMCtrl::WideToUtf8(_In_ const std::wstring& strLine, _Out_ std::string& strUtf8Line)
{
    int nLen = 0;
    char* szBuf = NULL;

    // Get UTF8 Length
    nLen = WideCharToMultiByte( CP_UTF8, 0, strLine.c_str(), -1, NULL, 0, NULL, NULL);

    // Allocate UTF8 Buffer
    szBuf = new char[nLen+1];
    if(NULL == szBuf)
        return FALSE;
    memset(szBuf, 0, sizeof(szBuf));

    // Convert    
    nLen = WideCharToMultiByte( CP_UTF8, 0, strLine.c_str(), -1, szBuf, nLen+1, NULL, NULL);
    if(0 == nLen)
    {
        delete []szBuf;
        return FALSE;
    }

    // Succeed
    strUtf8Line = szBuf;
    delete []szBuf;
    return TRUE;
}

BOOL CDRMCtrl::Utf8ToWide(_In_ const std::string& strUtf8Line, _Out_ std::wstring& strLine)
{
    int nLen = 0;
    WCHAR* wzBuf = NULL;

    if(strUtf8Line.length() <= 0)
        return TRUE;    

    // Get UTF8 Length
    nLen = MultiByteToWideChar(CP_UTF8, 0, strUtf8Line.c_str(), -1, NULL, 0);
    if(0 == nLen)
        return FALSE;

    wzBuf = new WCHAR[nLen+1];
    if(NULL == wzBuf)
        return FALSE;
    memset(wzBuf, 0, sizeof(wzBuf));
    nLen = MultiByteToWideChar(CP_UTF8, 0, strUtf8Line.c_str(), -1, wzBuf, nLen+1);
    if(0 == nLen)
    {
        delete []wzBuf; wzBuf=NULL;
        return FALSE;
    }

    // Succeed
    strLine = wzBuf;
    delete []wzBuf; wzBuf=NULL;
    return TRUE;
}

BOOL CDRMCtrl::ConflictPath(_In_ const std::wstring& wstrPath, _In_ const std::list<std::wstring>& Paths)
{
    std::list<std::wstring>::const_iterator it;
    for(it=Paths.begin(); it!=Paths.end(); ++it)
    {
        std::wstring drmpath = *it;
        std::wstring inpath  = wstrPath;
        if(!boost::algorithm::iends_with(drmpath, L"\\")) drmpath += L"\\";
        if(!boost::algorithm::iends_with(inpath, L"\\"))  inpath  += L"\\";

        if( boost::algorithm::istarts_with(drmpath, inpath) )  return TRUE;
        if( boost::algorithm::istarts_with(inpath,  drmpath) ) return TRUE;
    }
    return FALSE;
}

BOOL CDRMCtrl::ExistPath(_In_ const std::wstring& wstrPath, _In_ const std::list<std::wstring>& Paths)
{
    std::list<std::wstring>::const_iterator it;
    for(it=Paths.begin(); it!=Paths.end(); ++it)
    {
        std::wstring drmpath = *it;
        std::wstring inpath  = wstrPath;
        if(!boost::algorithm::iends_with(drmpath, L"\\")) drmpath += L"\\";
        if(!boost::algorithm::iends_with(inpath, L"\\"))  inpath  += L"\\";

        if( boost::algorithm::istarts_with(inpath, drmpath) ) return TRUE;
    }
    return FALSE;
}

BOOL CDRMCtrl::AddPathToList(_In_ const std::wstring& wstrPath, _In_ std::list<std::wstring>& Paths)
{
    // Remove all exiting sub path
    RemovePathFromList(wstrPath, Paths);
    // Add it
    Paths.push_back(wstrPath);
    return TRUE;
}

BOOL CDRMCtrl::RemovePathFromList(_In_ const std::wstring& wstrPath, _In_ std::list<std::wstring>& Paths)
{
    std::list<std::wstring>::const_iterator it;
    BOOL bRet = TRUE;
    Paths.remove_if(is_sub(wstrPath.c_str()));
    return bRet;
}

std::string CDRMCtrl::BuildSharedKeyRingLine(const std::wstring& wstrKeyRing)
{
    std::string strOut = SE_SHARED_KEYRING_KEYA;
    std::string strUtf8;

    if(!WideToUtf8(wstrKeyRing, strUtf8))
        return "";

    strOut += strUtf8;
    return strOut;
}

std::string CDRMCtrl::BuildDrmPathLine(const std::wstring& wstrPath)
{
    std::string strOut = SE_DRM_PATH_KEYA;
    std::string strUtf8;

    if(!WideToUtf8(wstrPath, strUtf8))
        return "";

    strOut += strUtf8;
    return strOut;
}

std::string CDRMCtrl::BuildNonDrmPathLine(const std::wstring& wstrPath)
{
    std::string strOut = SE_NONDRM_PATH_KEYA;
    std::string strUtf8;

    if(!WideToUtf8(wstrPath, strUtf8))
        return "";

    strOut += strUtf8;
    return strOut;
}

std::string CDRMCtrl::BuildFwDrmPathLine(const std::wstring& wstrPath)
{
    std::string strOut = SE_FWDRM_PATH_KEYA;
    std::string strUtf8;

    if(!WideToUtf8(wstrPath, strUtf8))
        return "";

    strOut += strUtf8;
    return strOut;
}

BOOL CDRMCtrl::WriteLine(HANDLE hFile, const std::string& strLine)
{
    DWORD dwWritten = 0;

    // Write "\0D\0A" first to make sure this is a new line
    WriteFile(hFile, LINEEND, 2, &dwWritten, NULL);
    return WriteFile(hFile, strLine.c_str(), static_cast<DWORD>(strLine.length()), &dwWritten, NULL);
}

BOOL CDRMCtrl::EnsureFileExist(LPCWSTR wzFile)
{
    HANDLE hFile = INVALID_HANDLE_VALUE;
    hFile = ::CreateFileW(wzFile, GENERIC_READ|GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if(INVALID_HANDLE_VALUE == hFile)
        return FALSE;
    CloseHandle(hFile);
    return TRUE;
}