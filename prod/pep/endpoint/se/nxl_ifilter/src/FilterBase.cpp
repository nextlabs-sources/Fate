#include <boost/algorithm/string.hpp>
#include <fstream>
#include <string>
#include "FilterBase.h"
#include "nlconfig.hpp"
#include "nl_sysenc_lib.h"
#include "NextLabsEncryption_Types.h"
#include "NTQuery.h"

#include <winsock2.h>
#include <Ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")

using std::wstring;

#ifdef _X86_
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.VC90.CRT' version='9.0.21022.8' processorArchitecture='x86' publicKeyToken='1fc8b3b9a1e18e3b' language='*'\"") 
#else ifdef _AMD64_
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.VC90.CRT' version='9.0.21022.8' processorArchitecture='amd64' publicKeyToken='1fc8b3b9a1e18e3b' language='*'\"") 
#endif


typedef int(*SE_DecryptNXLFile)(const wchar_t* pSrc, const wchar_t* pDest, bool bFailIfExists);
static SE_DecryptNXLFile	gSE_DecryptNXLFile = NULL;
static HMODULE	ghMode = NULL;


static HINSTANCE hinstSELib = 0;


static std::wstring GetCommonComponentsDir();
static bool initExtDLL();
static bool GetTempDir(std::wstring &tempPath);
static bool UnWrapFile( LPCWSTR wzSrc, LPCWSTR wzDest, bool SwitchToLocalKey);

HRESULT CFilterBase::Init(ULONG grfFlags, ULONG cAttributes, const FULLPROPSPEC *aAttributes, ULONG *pFlags)
{
	return m_pFilt->Init(grfFlags, cAttributes, aAttributes,  pFlags);
}

HRESULT CFilterBase::GetChunk(STAT_CHUNK *pStat)
{
	return  m_pFilt->GetChunk(pStat);
}

HRESULT CFilterBase::GetText(ULONG *pcwcBuffer, WCHAR *awcBuffer)
{
	return m_pFilt->GetText(pcwcBuffer, awcBuffer);
}

HRESULT CFilterBase::GetValue(PROPVARIANT **ppPropValue)
{
	return m_pFilt->GetValue(ppPropValue);
}

HRESULT CFilterBase::GetClassID( CLSID * pClassID )
{
    *pClassID = CLSID_NXLFILTER;
    return S_OK;
}

HRESULT CFilterBase::IsDirty()
{
	return E_NOTIMPL;
}

HRESULT CFilterBase::Save( LPCWSTR pszFileName, BOOL fRemember )
{
	pszFileName = pszFileName;
	fRemember = fRemember;
	return E_NOTIMPL;
}

HRESULT CFilterBase::SaveCompleted( LPCWSTR pszFileName )
{
	pszFileName = pszFileName;
	return E_NOTIMPL;
}

HRESULT CFilterBase::GetCurFile( LPWSTR  * ppszFileName )
{
	ppszFileName = ppszFileName;
	return E_NOTIMPL;
}
//////////////////////////////////////////////////////////////////////////
bool GetNXLFileExt(LPCWSTR wzFile, std::wstring& strOrigFile)
{
	const int nOffset = 0x800;
	const wchar_t* szFileExt = L"$FileExt";
	HANDLE hFile = CreateFileW(wzFile, GENERIC_READ, NULL, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		DWORD dwPtr = SetFilePointer(hFile, nOffset, NULL, FILE_BEGIN);
		if (dwPtr != INVALID_SET_FILE_POINTER)
		{
			wchar_t szBuf[513] = { 0 };
			DWORD dwRead = 0;
			if (ReadFile(hFile, (char*)szBuf, 1024, &dwRead, NULL))
			{
				if (_wcsnicmp(szBuf, szFileExt, wcslen(szFileExt)) == 0)
				{
					strOrigFile = szBuf + (wcslen(szFileExt) + 1);
				}
			}
			CloseHandle(hFile);
		}
	}
	if (strOrigFile.empty())	return false;
	return true;
}

bool GetFileExt(const char* pBuf, const int nBufLen, std::wstring& strExt)
{
	if (nBufLen < 0x800 + 100)	return false;
	const int nOffset = 0x800;
	wchar_t* pExt = (wchar_t*)(pBuf + nOffset);
	const wchar_t* szFileExt = L"$FileExt=";
	if (_wcsnicmp(pExt, szFileExt, wcslen(szFileExt)) == 0)
	{
		strExt = pExt + wcslen(szFileExt);
		return true;
	}
	return false;
}
bool GetUniquedFileName(wstring& strFilePath)
{
	wstring strTempFolder = L"";
	wchar_t szLog[1024] = { 0 };
	if (!GetTempDir(strTempFolder))
	{
		wsprintfW(szLog, L"NXLFilter-> GetTempDir failed, last error is %d.\n", GetLastError());
		OutputDebugStringW(szLog);
		return false;
	}
	wchar_t szTempFilePath[1052] = { 0 };
	if (!GetTempFileName(strTempFolder.c_str(), NULL, 0, szTempFilePath))
	{
		wsprintfW(szLog, L"NXLFilter-> GetTempFileName failed, last error is %d.\n", GetLastError());
		OutputDebugStringW(szLog);
		return false;
	}
	strFilePath = szTempFilePath;
	return true;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CFilterBase::Load( LPCWSTR pszFileName, DWORD dwMode)
{
	wchar_t szLog[1024] = { 0 };
	if (m_pFilt != NULL)
	{
		m_pFilt.Release();
		m_pFilt = NULL;
	}

	HRESULT hr = E_UNEXPECTED;
	if( !initExtDLL() )
	{
		OutputDebugStringW(L"NXLFilter-> initExtDLL failed.\n");
		return hr;
	}
	wstring strDst = L"";
	wstring strExt = L"";
	if (!GetNXLFileExt(pszFileName, strExt) || strExt.find(L'.') == wstring::npos || boost::algorithm::iends_with(strExt, ".nxl"))
	{
		OutputDebugStringW(L"NXLFilter-> Get File Extension of GetNXLFileExt failed.\n");
		return E_UNEXPECTED;
	}
	GetUniquedFileName(strDst);
	strDst += strExt;
	if (!UnWrapFile(pszFileName, strDst.c_str(), false))	return hr;
	#pragma warning(push)
	#pragma warning(disable:6309 6387)
	hr = LoadIFilter(strDst.c_str(), NULL, (void **)&m_pFilt);
	#pragma warning(pop)
	DeleteFileW(strDst.c_str());
	return hr; 
}

HRESULT CFilterBase::Save(IStream *pStm, BOOL fClearDirty)
{ 
	pStm = pStm;
	fClearDirty = fClearDirty;
	return E_NOTIMPL;
}

HRESULT CFilterBase::GetSizeMax(ULARGE_INTEGER *pcbSize)
{ 
	pcbSize = pcbSize;
	return E_NOTIMPL;
}

HRESULT CFilterBase::Load(IStream *pStm)
{
	wchar_t szLog[1024] = { 0 };
	if (m_pFilt != NULL)
	{
		m_pFilt.Release();
		m_pFilt = NULL;
	}

	STATSTG pstatstg = { 0 };
	DWORD grfStatFlag = STATFLAG_DEFAULT;
	HRESULT hr = pStm->Stat(&pstatstg, grfStatFlag);
	if (FAILED(hr) || pstatstg.cbSize.QuadPart < 1)
	{
		OutputDebugStringW(L"NXLFilter-> Read stream state failed.\n");
		return E_FAIL;
	}
	DWORD dwBufLen = pstatstg.cbSize.QuadPart;
	char* szBuffer = new char[dwBufLen];
	ZeroMemory(szBuffer, dwBufLen);
	hr = pStm->Read(szBuffer, dwBufLen, NULL);
	if (FAILED(hr))
	{
		OutputDebugStringW(L"NXLFilter-> Read stream data failed.\n");
		return E_FAIL;
	}
	std::wstring strExt = L"";
	if (!GetFileExt(szBuffer, dwBufLen, strExt) || boost::algorithm::iends_with(strExt, ".nxl") || strExt.find(L'.') == wstring::npos)
	{
		OutputDebugStringW(L"NXLFilter-> Get File Extension failed.\n");
		return E_FAIL;
	}
	wstring strsrc = L"";
	GetUniquedFileName(strsrc);
	strsrc += L".nxl";
	HANDLE hFile = CreateFile(strsrc.c_str(), GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		DWORD dwWriteLen = 0;
		WriteFile(hFile, szBuffer, dwBufLen, &dwWriteLen, NULL);
		CloseHandle(hFile);
	}
	else
	{
		wsprintfW(szLog, L"NXLFilter-> Create file of %s failed, last error is %d.\n", strsrc.c_str(), GetLastError());
		OutputDebugStringW(szLog);
		delete[] szBuffer;
		return E_FAIL;
	}
	delete[] szBuffer;
	if (!initExtDLL())	return E_FAIL;
	wstring strdst = L"";
	GetUniquedFileName(strdst);
	strdst += strExt;
	bool bRet = UnWrapFile(strsrc.c_str(), strdst.c_str(), false);
	if (bRet)
	{
#pragma warning(push)
#pragma warning(disable:6309 6387)
		hr = LoadIFilter(strdst.c_str(), NULL, (void **)&m_pFilt);
#pragma warning(pop)
	}
	DeleteFileW(strsrc.c_str());
	DeleteFileW(strdst.c_str());
	return hr;
}

std::wstring GetCommonComponentsDir()
{
	wchar_t szDir[MAX_PATH] = {0};
	if(NLConfig::ReadKey(L"SOFTWARE\\NextLabs\\CommonLibraries\\InstallDir", szDir, MAX_PATH))
	{
#ifdef _WIN64
		wcsncat_s(szDir, MAX_PATH, L"bin64", _TRUNCATE);
#endif
		return szDir;
	}

	return L"";
}

bool initExtDLL()
{
	if (ghMode != NULL && hinstSELib != NULL)	return true;
	//TO get installpath from register
	std::wstring wstrInstallPath = GetCommonComponentsDir();
	if(!boost::algorithm::iends_with(wstrInstallPath, "\\"))
	{
        wstrInstallPath += L"\\";
	}
	std::wstring strNxlWrapper = wstrInstallPath ;
#ifdef _WIN64
	strNxlWrapper += L"NxlWrapper.dll";
#endif 
	if (ghMode == NULL)
	{
		ghMode = LoadLibraryW(strNxlWrapper.c_str());
		if (ghMode == NULL)
		{
			wchar_t szLog[1024] = { 0 };
			wsprintfW(szLog, L"NXLFilter--------->Load lib of [%s] failed, error code is: %d.\n", strNxlWrapper.c_str(), GetLastError());
			OutputDebugStringW(szLog);
		}
		else
		{
			gSE_DecryptNXLFile = (SE_DecryptNXLFile)GetProcAddress(ghMode, "DecryptNXLFile");
			if (gSE_DecryptNXLFile == NULL)
			{
				OutputDebugStringW(L"Get proc address of DecryptNXLFile failed.\n");
				return false;
			}
		}
	}
	return true;
}

bool GetTempDir(std::wstring &tempPath)
{
    WCHAR wzTempDir[1025] = {0};
	GetTempPathW(1024, wzTempDir);
    tempPath = wzTempDir;
    if(tempPath.empty())
    {
        tempPath = L"C:\\Users\\{EFD15EFF-FED6-45CC-83C4-4DAAA4CAAC9C}\\";
        if(INVALID_FILE_ATTRIBUTES == GetFileAttributesW(tempPath.c_str()))
        {
            if(!::CreateDirectoryW(tempPath.c_str(), NULL))
            {
                return false;
            }
        }
    }
    return true;
}

bool UnWrapFile( LPCWSTR wzSrc, LPCWSTR wzDest, bool SwitchToLocalKey)
{
    bool bRet = false;
	int nRet = gSE_DecryptNXLFile(wzSrc, wzDest, false);
	if (nRet == 0)	bRet = true;
	else
	{
		wchar_t szLog[1024] = { 0 };
		wsprintfW(szLog, L"NXLFilter-> Decrypt file with NXLWrapper from %s to %s failed, the error code is %d.\n", wzSrc, wzDest, nRet);
		OutputDebugStringW(szLog);
	}
    return bRet;
}



