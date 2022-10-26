#include "StdAfx.h"
#include "NxtMgr.h"
#include <boost/algorithm/string.hpp>
#include "boost/smart_ptr.hpp"
#include "boost/filesystem.hpp"
#include "FileOperationHooking.h"
#include "Hooking.h"
#include "nl_sysenc_lib.h"
#include "celog.h"

#pragma warning(push)
#pragma warning(disable:6334 6011 4996 4189 4100 4819)
#include "boost\format.hpp"
#pragma warning(pop)

const int MAX_PATHLEN = 1024;
const wchar_t COPYSUFFIX[] = L" - Copy";
const wchar_t LOCALENCRYPTED_HINT[] = L"This action is not permitted. \nPlease use Windows Explorer to perform the action.";

bool g_bWarningBox = false;
extern CELog cbPepLog;

class CWarningBoxParam
{
public:
	CWarningBoxParam(LPCWSTR title, LPCWSTR content)
	{
		wcsncpy_s(szTitle, _countof(szTitle), title, _TRUNCATE);
		wcsncpy_s(szContent, _countof(szContent), content, _TRUNCATE);
	}

	wchar_t szTitle[100];
	wchar_t szContent[1024];
};

void WarningBoxThread(void* pParam)
{
	if (!pParam)
	{
		return;
	}

	g_bWarningBox = true;
	CWarningBoxParam* p = (CWarningBoxParam*)pParam;
	MessageBoxW(::GetForegroundWindow(), p->szContent, p->szTitle, MB_ICONWARNING | MB_OK);

	delete p;

	g_bWarningBox = false;
}

BOOL GetOSInfo(DWORD& dwMajor, DWORD& dwMinor)
{
	static DWORD sMajor = 0;
	static DWORD sMinor = 0;

	if(sMajor == 0 && sMinor == 0)
	{
		OSVERSIONINFOEX osvi;
		BOOL bOsVersionInfoEx;

		// Try calling GetVersionEx using the OSVERSIONINFOEX structure.
		//
		// If that fails, try using the OSVERSIONINFO structure.

		ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
		osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

		bOsVersionInfoEx = GetVersionEx ((OSVERSIONINFO *) &osvi);
		if( !bOsVersionInfoEx )
		{
			// If OSVERSIONINFOEX doesn't work, try OSVERSIONINFO.

			osvi.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
			if (! GetVersionEx ( (OSVERSIONINFO *) &osvi) ) 
				return FALSE;
		}

		sMajor = osvi.dwMajorVersion;
		sMinor = osvi.dwMinorVersion;

	}


	//5,0 win2k, 5,1 winxp
	dwMajor = sMajor;
	dwMinor = sMinor;

	return TRUE;

}

BOOL IsWin7()
{
	static DWORD dwMajor = 0;
	static DWORD dwMinor = 0;
	if (dwMajor == 0 && dwMinor == 0)
	{
		GetOSInfo(dwMajor, dwMinor);
	}
	
	return dwMajor >= 6? TRUE: FALSE;
}

CNxtMgr::CNxtMgr(void):m_bInitialized(false)
{
}

CNxtMgr::~CNxtMgr(void)
{
}

CNxtMgr* CNxtMgr::Instance()
{
	static CNxtMgr mgr;
	return &mgr;
}

void CNxtMgr::Init()
{
	if (!m_bInitialized)
	{
		CFileOperationHooking::GetInstance();//force to create the instance here.
		CHooking* inst = CHooking::GetInstance();
		inst->StartHook();

		m_bInitialized = true;
	}
}

void CNxtMgr::Uninit()
{
	if(m_bInitialized)
	{
		CHooking* inst = CHooking::GetInstance();
		inst->EndHook();

		m_bInitialized = false;
	}
}

bool CNxtMgr::IsInitialized()
{
	return m_bInitialized;
}

std::wstring CNxtMgr::GetCurModuleName()
{
	static wchar_t buf[1024] = {0};
	if(wcslen(buf) == 0)
		GetModuleFileNameW(NULL, buf, 1024);

	return std::wstring(buf);
}

std::string CNxtMgr::MyWideCharToMultipleByte(const std::wstring & strValue)
{
	int nLen = WideCharToMultiByte(CP_ACP, 0, strValue.c_str(), (int)strValue.length(), NULL, 0, NULL, NULL); 

	char* pBuf = new char[nLen + 1];
	if(!pBuf)
		return "";
	memset(pBuf, 0, nLen +1);
	nLen = WideCharToMultiByte(CP_ACP, 0, strValue.c_str(), (int)strValue.length(), pBuf, nLen, NULL, NULL); 

	std::string strResult(pBuf);

	if(pBuf)
	{
		delete []pBuf;
		pBuf = NULL;
	}
	return strResult;
}

void CNxtMgr::PopupWarningBox(LPCWSTR title, LPCWSTR content)
{
	if (!title || !content || g_bWarningBox)
	{
		return;
	}

	CWarningBoxParam* data = new CWarningBoxParam(title, content);
	_beginthread(WarningBoxThread, 0, data);
}

bool CNxtMgr::NeedBlock()
{
	static wchar_t buf[1024] = {0};
	if(wcslen(buf) == 0)
		GetModuleFileNameW(NULL, buf, 1024);


	bool b = boost::algorithm::iends_with(std::wstring(buf), L"explorer.exe");
	return !b;
}

bool CNxtMgr::FileExists(LPCWSTR pszFileName)
{
	if( pszFileName && _waccess( pszFileName, 0 ) == 0 ) { 
		return true;
	} else { 
		return false; 
	} 

}

bool CNxtMgr::IsLocalPath(LPCWSTR pszFileName)
{
	if (pszFileName == NULL)
	{
		return false;
	}

	bool bRet = false;
	if (wcslen(pszFileName) >= 3 && pszFileName[1] == ':')
	{
		wchar_t drive[3] = {0};
		wcsncpy_s(drive, 3, pszFileName, _TRUNCATE);

		if (GetDriveTypeW(drive) == DRIVE_FIXED)
		{
			bRet = true;
		}
	}


	std::wstring temp = boost::str(boost::wformat(L"IsLocalFile? ret: %d, path: %s\n") % bRet % pszFileName);
	cbPepLog.Log(CELOG_DEBUG,L"%s",temp.c_str());
	//OutputDebugStringW(temp.c_str());

	return bRet;
};

bool CNxtMgr::FindFirstNumber(const std::wstring& strFileName, std::wstring::size_type& start, std::wstring::size_type& end)
{
	bool ret = false;
	std::wstring::size_type index1 = strFileName.find('(');
	while (index1 != std::wstring::npos)
	{
		std::wstring::size_type index2 = strFileName.find(')', index1);

		if (index2 == std::wstring::npos)
		{
			ret = false;
			break;
		}

		std::wstring substr = strFileName.substr(index1 + 1, index2 - index1 -1);

		bool bAllNumber = true;
		for (std::string::size_type m = 0; m < substr.length(); m++)
		{
			if (!(substr[m] >= '0' && substr[m] <= '9'))
			{
				bAllNumber = false;
				break;
			}
		}

		if (bAllNumber)
		{
			start = index1;
			end = index2;

			ret = true;
			break;
		}

		index1 = strFileName.find('(', index1 + 1);
	}

	return ret;
}

std::wstring CNxtMgr::GetPossiblePathWithSameFolder(LPCWSTR pszFilePath)
{
	if (!pszFilePath || wcsrchr(pszFilePath, '\\') == 0)
	{
		return L"";
	}

	boost::filesystem::wpath Path(pszFilePath);
	std::wstring::size_type index = 0;
	std::wstring strFileName = Path.filename();
	std::wstring strFolder = Path.parent_path().native_directory_string() + L"\\";

	std::wstring part1 = strFileName;
	std::wstring part2 = L"";

	index = strFileName.rfind('.');
	if (index != std::wstring::npos)
	{
		part1 = strFileName.substr(0, index);
		part2 = strFileName.substr(index);
	}

	//try to make a new file name "xxx - Copy.xxx"
	std::wstring strNewFileName = part1 + std::wstring(COPYSUFFIX) + part2;
	std::wstring strNewPath = strFolder + strNewFileName;

	bool bHandled = false;
	if (FileExists(strNewPath.c_str()))
	{
		std::wstring::size_type start, end;
		if (FindFirstNumber(strNewFileName, start, end))//it means the file name contains a (number) already.
		{
			bHandled = true;
			int number = ::_wtoi(strNewFileName.substr(start + 1, end - start - 1).c_str());
			for (int i = 0; i < 10000; i++)
			{
				number++;
				std::wstring::size_type len = end - start + 4;
				wchar_t* FileNameIndex = new wchar_t[len];
				memset(FileNameIndex, 0, sizeof(wchar_t) * len);
				_snwprintf_s(FileNameIndex, len, _TRUNCATE, L"%d", number);
				
				strNewFileName.replace(start + 1, end - start -1, FileNameIndex);

				delete []FileNameIndex;

				strNewPath = strFolder + strNewFileName;

				if (!FileExists(strNewPath.c_str()))
				{
					break;
				}

			}
		}
	}


	//file name doesn't contain (number), then add to end, like "xxx - Copy (1).xxx"
	for (int i = 2; !bHandled && i < 10000; i++)
	{
		if (!FileExists(strNewPath.c_str()))
		{
			break;
		}

		wchar_t buf[100] = {0};
		_snwprintf_s(buf, 100, _TRUNCATE, L" (%d)", i);

		strNewFileName = part1 + COPYSUFFIX + buf + part2;
		strNewPath = strFolder + strNewFileName;

	}

	return strNewPath;
}

std::wstring CNxtMgr::GetPossiblePath(LPCWSTR pszFilePath)
{
	//windows XP doesn't support this mode.
	if (!pszFilePath || !IsWin7() )
	{
		return L"";
	}

	std::wstring path(pszFilePath);
	boost::algorithm::replace_all(path, L"/", L"\\");
	std::wstring::size_type index = path.rfind('\\');
	if (index == std::wstring::npos)
	{
		return L"";
	}

	std::wstring filename = path.substr(index + 1);//get file name
	std::wstring folder = path.substr(0, index + 1);//get the folder of file
	std::wstring newFilePath;

	std::wstring::size_type start; 
	std::wstring::size_type end;

	bool bHandled = false;
	if (FindFirstNumber(filename, start, end))//it means there is a pair of (), in (), there is a number
	{
		bHandled = true;
		int number = ::_wtoi(filename.substr(start + 1, end - start - 1).c_str());
		for (int i = 0; i < 10000; i++)
		{
			number++;
			std::wstring::size_type len = end - start + 4;
			wchar_t* FileNameIndex = new wchar_t[len];
			memset(FileNameIndex, 0, sizeof(wchar_t) * len);
			_snwprintf_s(FileNameIndex, len, _TRUNCATE, L"%d", number);

			std::wstring newFileName = filename;
			newFileName.replace(start + 1, end - start -1, FileNameIndex);

			delete []FileNameIndex;

			newFilePath = folder + newFileName;

			if (!FileExists(newFilePath.c_str()))
			{
				break;
			}

		}
	}

	
	if(!bHandled)
	{//file name doesn't contain "()"
		std::wstring::size_type finder = filename.rfind('.');
		std::wstring part1 = filename;
		std::wstring part2 = L"";

		if (finder != std::wstring::npos)
		{
			part1 = filename.substr(0, finder);
			part2 = filename.substr(finder);
		}

		int number = 2;
		for (int i = 0; i < 10000; i++)
		{
			wchar_t buf[50] = {0};
			_snwprintf_s(buf, 50, _TRUNCATE, L" (%d)", number);

			std::wstring newFileName = part1 + buf + part2;

			newFilePath = folder + newFileName;

			if (!FileExists((folder + newFileName).c_str()))
			{
				break;
			}
			number++;
		}
	}

	return newFilePath;
}

bool CNxtMgr::IsFile(IShellItem* psiItem)
{
	if (!psiItem)
	{
		return false;
	}

	SFGAOF attr = 0;
	HRESULT hr = psiItem->GetAttributes(SFGAO_FOLDER, &attr);
	if (SUCCEEDED(hr) && attr != SFGAO_FOLDER)
	{
		return true;
	}

	return false;
}

bool CNxtMgr::IsLocalEncrypted(IShellItem* item)
{
	bool ret = false;
	if (item)
	{
		PWSTR pszPath = NULL;
		HRESULT hr = item->GetDisplayName(SIGDN_FILESYSPATH, &pszPath);
		if (SUCCEEDED(hr) && pszPath)
		{
			ret = IsLocalEncrypted(pszPath);
			CoTaskMemFree(pszPath);

		}

	}

	return ret;
}

bool CNxtMgr::IsLocalEncrypted(LPCWSTR pszFileName)
{
	if (pszFileName && (!boost::algorithm::iends_with(pszFileName, L".nxl") && SE_IsEncrypted(pszFileName)))
	{
		return true;
	}

	return false;
}

bool CNxtMgr::PreHandleEncryption(LPCWSTR src, LPCWSTR dest)
{
	if (!src || !dest || !IsLocalEncrypted(src))
	{
		return true;
	}

	bool bRet = true;
	if (IsLocalPath(dest))
	{
		if (!FileExists(dest))
		{
			//call SE function to mark the newly created file should be encrypted.
			std::wstring temp = boost::str(boost::wformat(L"Need to call SE function to mark the newly created file should be encrypted. path: %s\n") % dest);
			cbPepLog.Log(CELOG_DEBUG,L"%s",temp.c_str());
			//OutputDebugStringW(temp.c_str());

			//added for bug 24589: if the passed-in file name is a short file name, try to change it to long path
			std::wstring strPath(dest);		
			size_t nPos = strPath.find_first_of(L'\\',strPath.rfind(L'~'));
			if (nPos != std::wstring::npos)
			{
				wchar_t szLong[1024] = {0};
				if (GetLongPathNameW(strPath.substr(0,nPos).c_str(),szLong,1024) > 0)
					strPath = szLong + strPath.substr(nPos);
			}

			SE_MarkFileAsDRMOneShot(strPath.c_str());
		}
		else
		{//the dest file exists already, need to find the possible path after user clicks "copy, but keep both"
			boost::filesystem::wpath pathSrc(src);
			boost::filesystem::wpath pathDest(dest);

			boost::filesystem::wpath parentSrc = pathSrc.parent_path();
			boost::filesystem::wpath parentDest = pathDest.parent_path();


			std::wstring newFilePath;
			if (boost::filesystem::equivalent(parentSrc, parentDest))
			{
				newFilePath = GetPossiblePathWithSameFolder(dest);
			}
			else
			{
				newFilePath = GetPossiblePath(dest);
			}


			if (!newFilePath.empty())
			{//call SE function
				std::wstring temp = boost::str(boost::wformat(L"Need to call SE function to mark the newly created file should be encrypted(dest exists). path: %s\n") % newFilePath.c_str());
				cbPepLog.Log(CELOG_DEBUG,L"%s",temp.c_str());
				//OutputDebugStringW(temp.c_str());

				SE_MarkFileAsDRMOneShot(newFilePath.c_str());
			}
			
		}

	}

	if (!IsLocalPath(dest))//block the action if the dest is not local
	{
		PopupWarningBox(L"Warning", LOCALENCRYPTED_HINT);
		bRet = false;
	}

	return bRet;
}

bool CNxtMgr::PreHandleEncryption(IShellItem * src, IShellItem * dest)
{
	bool bRet = true;
	if (IsFile(src))//only care files NOT folders
	{
		PWSTR srcpath = NULL;
		HRESULT hr = src->GetDisplayName(SIGDN_FILESYSPATH, &srcpath);
		if (SUCCEEDED(hr))//only care local-encrypted files
		{
			wchar_t* p = wcsrchr(srcpath, '\\');
			if (p)
			{
				boost::scoped_ptr<wchar_t> filename(new wchar_t[MAX_PATHLEN]);
				wcsncpy_s(filename.get(), MAX_PATHLEN, p + 1, _TRUNCATE);//get file name

				PWSTR folder = NULL;
				hr = dest->GetDisplayName(SIGDN_FILESYSPATH, &folder);//get dest folder path
				if (SUCCEEDED(hr))
				{
					//combine the full path of destination
					std::wstring destpath = boost::str(boost::wformat(L"%s\\%s") % folder % filename.get());

					bRet = PreHandleEncryption(srcpath, destpath.c_str());

					CoTaskMemFree(folder);
				}
			}

			CoTaskMemFree(srcpath);
		}
	}

	return bRet;
}


void CNxtMgr::PostHandleEncryption(IShellItem * src, IShellItem * dest, LPCWSTR pszNewName)
{
	if (!src || !dest)
	{
		return;
	}

	PWSTR pszDest = NULL;
	HRESULT hr = dest->GetDisplayName(SIGDN_FILESYSPATH, &pszDest);
	if (SUCCEEDED(hr) && pszDest)
	{
		if (IsFile(src))
		{
			//get dest file path
			std::wstring strDest = boost::str(boost::wformat(L"%s\\%s") % pszDest % pszNewName);

			PWSTR szSrc = NULL;
			hr = src->GetDisplayName(SIGDN_FILESYSPATH, &szSrc);
			if (SUCCEEDED(hr) && szSrc)
			{
				PostHandleEncryption(szSrc, strDest.c_str());

				CoTaskMemFree(szSrc);
			}

		}

		CoTaskMemFree(pszDest);
	}

}

void CNxtMgr::PostHandleEncryption(LPCWSTR src, LPCWSTR dest)
{
	if (src && dest)
	{
		if (IsLocalEncrypted(src) && !IsLocalEncrypted(dest))
		{//the source file is local-encrypted, though the dest is NOT local-encrypted, 
			//then, we need to encrypt the dest file here.
			//call SE function to encrypt dest file.
			SE_EncryptFile(dest);
		}
	}
}

CNxtAeroPeek::CNxtAeroPeek(void):m_hEvent(NULL),m_hThread(NULL),m_dwProcessId(0),m_bNeedCreateThread(false)
{
	
}

CNxtAeroPeek::~CNxtAeroPeek(void)
{
}

CNxtAeroPeek& CNxtAeroPeek::GInstance()
{
	static CNxtAeroPeek mgr;
	return mgr;
}

void CNxtAeroPeek::Load()
{
	std::wstring currentExePath = L"";
	std::wstring parentExePath = L"";
	const std::wstring AdobeRedExeName = L"AcroRd32.exe";
	
	m_dwProcessId = GetCurrentProcessId();
	DWORD dwParentProcessId = GetParentProcessID(m_dwProcessId,currentExePath);
	if(dwParentProcessId)	GetParentProcessID(dwParentProcessId,parentExePath);

	if(CompareStringOrdinal(parentExePath.c_str(),parentExePath.length(),AdobeRedExeName.c_str(),AdobeRedExeName.length(),true) == CSTR_EQUAL)
	{ 
		m_bNeedCreateThread = false;
		return ;		
	}
	else
	{
		m_bNeedCreateThread = true;		
	}

 	m_hEvent = CreateEventW(NULL,TRUE,FALSE,NULL);
 	if(GetLastError() == ERROR_ALREADY_EXISTS || m_hEvent == NULL)	return;
	
	m_hThread = (HANDLE)_beginthreadex(NULL,NULL,StartThread,this,CREATE_SUSPENDED,NULL);
	if(m_hThread != INVALID_HANDLE_VALUE)
	{
		SetThreadPriority(m_hThread,THREAD_PRIORITY_BELOW_NORMAL);
		ResumeThread(m_hThread);
	}
	else
	{
		CloseHandle( m_hEvent );
		m_hEvent = NULL;
	}
}

void CNxtAeroPeek::UnLoad()
{
	if(!m_bNeedCreateThread)		return;
	if(m_hThread != INVALID_HANDLE_VALUE)
	{
		SetEvent(m_hEvent);
		::WaitForSingleObject(m_hThread,3000);
		CloseHandle(m_hThread);
		m_hThread = NULL;	
		CloseHandle(m_hEvent);
		m_hEvent = NULL;
	}
}

unsigned CNxtAeroPeek::StartThread(PVOID lParam)
{
	CNxtAeroPeek *pThis = (CNxtAeroPeek*)lParam;
	while(true)
	{
		HWND hOverlayWnd = FindWindow(TEXT("WaterMarkWindows"),NULL);
		pThis->DisableAeroPeek( NULL != hOverlayWnd );
		DWORD dwTime = WaitForSingleObject(pThis->m_hEvent,500);
		if(dwTime == WAIT_OBJECT_0 || dwTime == WAIT_FAILED)	
			break;
	}
	pThis->DisableAeroPeek(false);
	_endthreadex(0);
	return 0;
}

void CNxtAeroPeek::DisableAeroPeek(bool bDisable)
{
	// check if HKEY_CURRENT_USER\Software\Microsoft\Windows\DWM\EnableAeroPeek value,
	// if value is 1 reset to 0, otherwise, do nothing.
	HKEY hCurKey = NULL,hSubKey = NULL;
	LONG lRet = RegOpenCurrentUser(KEY_SET_VALUE,&hCurKey);
	if(lRet == ERROR_SUCCESS)	
	{
		static const wchar_t* strKeyPath = L"Software\\Microsoft\\Windows\\DWM";
		lRet = RegOpenKey(hCurKey,strKeyPath,&hSubKey);
		if(lRet == ERROR_SUCCESS)
		{
			DWORD dwData = 0;
			DWORD dwSize = sizeof(DWORD);
			lRet = RegQueryValueEx(hSubKey, L"EnableAeroPeek", NULL,NULL,(LPBYTE)&dwData,&dwSize);
			if(bDisable)
			{
				if(lRet != ERROR_SUCCESS || dwData != 0)
				{
					dwData = 0;
					if(NULL == FindWindow(L"LivePreview",NULL))
						lRet = RegSetValueExW(hSubKey,L"EnableAeroPeek",0,REG_DWORD,(BYTE*)&dwData,sizeof(dwData));
				}
			}
			else
			{
				if(lRet != ERROR_SUCCESS || dwData != 1)
				{
					dwData = 1;
					if(NULL == FindWindow(L"LivePreview",NULL))
						lRet = RegSetValueExW(hSubKey,L"EnableAeroPeek",0,REG_DWORD,(BYTE*)&dwData,sizeof(dwData));
				}
			}
			RegCloseKey(hSubKey);
		}
		RegCloseKey(hCurKey);
	}
}

DWORD CNxtAeroPeek::GetParentProcessID(DWORD dwProcessID,std::wstring& exePath)
{
	DWORD dwParentID = 0;
	HANDLE hProcessSnap = NULL;
	PROCESSENTRY32 pe32;

	// Take a snapshot of all processes in the system.
	hProcessSnap = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );
	if( hProcessSnap == INVALID_HANDLE_VALUE )
	{
		return( FALSE );
	}

	// Set the size of the structure before using it.
	pe32.dwSize = sizeof( PROCESSENTRY32 );

	// Retrieve information about the first process,
	// and exit if unsuccessful
	if( !Process32First( hProcessSnap, &pe32 ) )
	{
		CloseHandle( hProcessSnap );    // Must clean up the
		//   snapshot object!
		return( FALSE );
	}

	do
	{
		if(dwProcessID == pe32.th32ProcessID )
		{
			dwParentID = pe32.th32ParentProcessID;
			exePath	= pe32.szExeFile;
		}
	} while( Process32Next( hProcessSnap, &pe32 ) );

	CloseHandle( hProcessSnap );
	return dwParentID;
}
