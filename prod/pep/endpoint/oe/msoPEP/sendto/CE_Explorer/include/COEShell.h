// COEShell.h : Declaration of the CCOEShell

#pragma once
#include "resource.h"       // main symbols

#include "CE_Explorer.h"
#include <ShObjIdl.h>
#include <tlhelp32.h>
#include <string>

#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Single-threaded COM objects are not properly supported on Windows CE platform, such as the Windows Mobile platforms that do not include full DCOM support. Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support creating single-thread COM object's and allow use of it's single-threaded COM object implementations. The threading model in your rgs file was set to 'Free' as that is the only threading model supported in non DCOM Windows CE platforms."
#endif



// CCOEShell
//struct __declspec(uuid("000214E4-0000-0000-C000-000000000046")) IContextMenu;
const WCHAR* gMutexName = L"NextLabs Explorer Plugin";
HANDLE gMutex = NULL;

//////////////////////////////////////////////////////////////////////////
DWORD GetExplorerProcessID()
{
	DWORD pID = 0;
	HANDLE hSnap=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);//创建系统快照
	PROCESSENTRY32 lPrs;
	ZeroMemory(&lPrs,sizeof(lPrs));
	lPrs.dwSize=sizeof(lPrs);
	Process32First(hSnap,&lPrs);//取得系统快照里第一个进程信息
	std::wstring strname = lPrs.szExeFile;
	wchar_t* pName =L"explorer.exe";
	if(strname.find(pName) != std::wstring::npos)//判断进程信息是否是explorer.exe
	{
		pID=lPrs.th32ProcessID;
		return pID;
	}
	while(1)
	{
		ZeroMemory(&lPrs,sizeof(lPrs));
		lPrs.dwSize=(&lPrs,sizeof(lPrs));
		if(!Process32Next(hSnap,&lPrs))//继续枚举进程信息
		{
			pID=(DWORD)-1;
			break;
		}
		strname = lPrs.szExeFile;
		if(strname.find(pName) != std::wstring::npos)//判断进程信息是否是explorer.exe
		{
			pID=lPrs.th32ProcessID;
			break;
		}
	}
	return pID;
}

BOOL WINAPI InjectDllW(DWORD dwProcessId,PCWSTR pszLibFile)
{
	BOOL fOk = FALSE;
	HANDLE hProcess = NULL,hThread = NULL;
	PWSTR pszLibFileRemote = NULL;
    HMODULE hModK32 = NULL;

	__try {
		hProcess = OpenProcess(
			PROCESS_CREATE_THREAD	|
			PROCESS_VM_OPERATION	|
			PROCESS_VM_WRITE,
			FALSE,dwProcessId);
		if(hProcess == NULL) __leave;

		int cch = 1 + lstrlenW(pszLibFile);
		int cb = cch*sizeof(WCHAR);

		pszLibFileRemote = (PWSTR)
			VirtualAllocEx(hProcess,NULL,cb,MEM_COMMIT,PAGE_READWRITE);
		if(pszLibFileRemote == NULL) __leave;

		if(!WriteProcessMemory(hProcess,pszLibFileRemote,
			(PVOID)pszLibFile,cb,NULL)) __leave;

        hModK32 = GetModuleHandleW(L"Kernel32");
        if(hModK32==NULL) __leave;
		PTHREAD_START_ROUTINE pfnThreadRtn = (PTHREAD_START_ROUTINE)
			GetProcAddress(hModK32,"LoadLibraryW");
		if(pfnThreadRtn==NULL) __leave;

        /************************************************************************/
        /* Reviewed by Gavin 4/9/2009 -- THE METHOD IS WRONG HERE               */
        /*                                                                      */
        /* This is not a correct way to inject DLL                              */
        /* Because you get "LoadLibraryW" address in your process               */
        /* But use it in another process -- it may cause crash                  */
        /* We need to re-implement it in next release                           */
        /************************************************************************/
		hThread = CreateRemoteThread(hProcess,NULL,0,
			pfnThreadRtn,pszLibFileRemote,NULL,NULL);
		if(hThread==NULL) __leave;

		WaitForSingleObject(hThread,INFINITE);
		fOk = TRUE;
	}
	__finally
	{
		if(pszLibFileRemote != NULL)
			VirtualFreeEx(hProcess,pszLibFileRemote,0,MEM_RELEASE);
		if(hThread!=NULL) CloseHandle(hThread);
		if(hProcess != NULL) CloseHandle(hProcess);
	}
	return fOk;

}

//////////////////////////////////////////////////////////////////////////
class ATL_NO_VTABLE CCOEShell :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CCOEShell, &CLSID_COEShell>,
	public IShellExtInit,
	public IContextMenu
{
public:
	CCOEShell()
	{
	}
	virtual ~CCOEShell()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_COESHELL)


BEGIN_COM_MAP(CCOEShell)
	COM_INTERFACE_ENTRY(IShellExtInit)
	COM_INTERFACE_ENTRY(IContextMenu)
END_COM_MAP()

public:
	// IShellExtInit
	STDMETHODIMP Initialize(LPCITEMIDLIST, LPDATAOBJECT, HKEY)
	{
		gMutex = ::CreateMutexW(NULL, FALSE, gMutexName);   
		if(GetLastError() == ERROR_ALREADY_EXISTS )   	return S_OK;  
		/* ************************************
		* Changed by Tonny to location dhook.dll
		* Data:5/9 2008
		*/
		wchar_t strPath[MAX_PATH]={0};
#if 1
		DWORD dwLen = ::GetModuleFileNameW(g_hInstance,strPath,MAX_PATH);
		while(dwLen > 0)
		{
			if(strPath[dwLen--] == L'\\')	
			{
				strPath[dwLen+1]=L'\0';
				break;
			}
		}

		if(sizeof(void*)==8)
			wcsncat_s(strPath,MAX_PATH,L"\\InjectExp.dll", _TRUNCATE);
		else
			wcsncat_s(strPath,MAX_PATH,L"\\InjectExp32.dll", _TRUNCATE);
#else
		::GetSystemDirectoryW(strPath,MAX_PATH);
		wcscat_s(strPath,MAX_PATH,L"\\InjectExp.dll");
#endif
		DWORD dwID = GetCurrentProcessId();
		//DWORD dwID = GetExplorerProcessID();
		InjectDllW(dwID,strPath);
		return S_OK;
	}
	STDMETHODIMP GetCommandString(UINT, UINT, UINT*, LPSTR, UINT)
	{
		return S_OK ;
	}
#ifdef _WIN64
	STDMETHODIMP GetCommandString(UINT_PTR, UINT,UINT*,LPSTR,UINT)
	{
		return S_OK;
	}
#endif
	STDMETHODIMP InvokeCommand(LPCMINVOKECOMMANDINFO)
	{
		return S_OK ;
	}
	STDMETHODIMP QueryContextMenu(HMENU, UINT, UINT, UINT, UINT)
	{
		return S_OK ;
	}
public:

};

OBJECT_ENTRY_AUTO(__uuidof(COEShell), CCOEShell)
