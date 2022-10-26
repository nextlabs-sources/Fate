// ShellExt.h : Declaration of the CShellExt

#pragma once
#include "resource.h"       // main symbols

#include "windows.h"
#include "TagView.h"
#include <ShObjIdl.h>
#include <ShlObj.h>
#include <list>

#define FILETAGGING_DLL_NAME L"pa_filetagging.dll"

#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Single-threaded COM objects are not properly supported on Windows CE platform, such as the Windows Mobile platforms that do not include full DCOM support. Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support creating single-thread COM object's and allow use of it's single-threaded COM object implementations. The threading model in your rgs file was set to 'Free' as that is the only threading model supported in non DCOM Windows CE platforms."
#endif


// CShellExt

class ATL_NO_VTABLE CShellExt :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CShellExt, &CLSID_ShellExt>,
	public IDispatchImpl<IShellExt, &IID_IShellExt, &LIBID_TagViewLib, /*wMajor =*/ 1, /*wMinor =*/ 0>,
	public IShellExtInit,
	public IContextMenu
{
public:
	CShellExt()
	{
		
	}

DECLARE_REGISTRY_RESOURCEID(IDR_SHELLEXT)


BEGIN_COM_MAP(CShellExt)
	COM_INTERFACE_ENTRY(IShellExt)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(IShellExtInit)
	COM_INTERFACE_ENTRY(IContextMenu)
END_COM_MAP()



	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease()
	{
	}

public:
	STDMETHODIMP Initialize(LPCITEMIDLIST pidlFolder, IDataObject *pdtobj, HKEY hkeyProgID);
	STDMETHODIMP QueryContextMenu(HMENU hmenu, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags);
	STDMETHODIMP GetCommandString(UINT idCmd, UINT uFlags, UINT *pwReserved, LPSTR pszName, UINT cchMax);  
	STDMETHODIMP InvokeCommand(LPCMINVOKECOMMANDINFO pici);

protected:

	std::list<std::wstring> m_listFiles;
	
};

OBJECT_ENTRY_AUTO(__uuidof(ShellExt), CShellExt)
