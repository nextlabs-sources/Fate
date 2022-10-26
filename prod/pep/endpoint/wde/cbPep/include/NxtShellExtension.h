// NxtShellExtension.h : Declaration of the CNxtShellExtension

#pragma once
#include "resource.h"       // main symbols

#include "cbPep_i.h"
#include <ShObjIdl.h>
#include <shlobj.h>
#include <ShellAPI.h>
#include <commctrl.h>
#include <string>
#include <list>

#include "NxtMgr.h"

#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Single-threaded COM objects are not properly supported on Windows CE platform, such as the Windows Mobile platforms that do not include full DCOM support. Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support creating single-thread COM object's and allow use of it's single-threaded COM object implementations. The threading model in your rgs file was set to 'Free' as that is the only threading model supported in non DCOM Windows CE platforms."
#endif

typedef std::list<std::wstring> string_list; 

// CNxtShellExtension

class ATL_NO_VTABLE CNxtShellExtension :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CNxtShellExtension, &CLSID_NxtShellExtension>,
	public IDispatchImpl<INxtShellExtension, &IID_INxtShellExtension, &LIBID_cbPepLib, /*wMajor =*/ 1, /*wMinor =*/ 0>,
	public IShellExtInit,
	public IContextMenu,
	public ICopyHook
{
public:
	CNxtShellExtension()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_NXTSHELLEXTENSION)


BEGIN_COM_MAP(CNxtShellExtension)
	COM_INTERFACE_ENTRY(INxtShellExtension)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(IShellExtInit)
	COM_INTERFACE_ENTRY(IContextMenu)
	COM_INTERFACE_ENTRY(ICopyHook)
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
	//IShellExtInit
	STDMETHODIMP Initialize(LPCITEMIDLIST pidlFolder, IDataObject *pdtobj, HKEY hkeyProgID);

	//IContextMenu
	STDMETHODIMP QueryContextMenu(HMENU hmenu, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags);
	virtual HRESULT STDMETHODCALLTYPE GetCommandString( 
		/* [annotation][in] */ 
		_In_  UINT_PTR idCmd,
		/* [annotation][in] */ 
		_In_  UINT uType,
		/* [annotation][in] */ 
		/* _Reserved_ */  UINT *pReserved,
		/* [annotation][out] */ 
		_Out_cap_(cchMax)  LPSTR pszName,
		/* [annotation][in] */ 
		_In_  UINT cchMax);
	STDMETHODIMP InvokeCommand(LPCMINVOKECOMMANDINFO /*pici*/);

	//ICopyHook
	STDMETHOD_(UINT,CopyCallback) (THIS_ HWND hwnd, UINT wFunc, UINT wFlags, LPCWSTR pszSrcFile, DWORD dwSrcAttribs,
		LPCWSTR pszDestFile, DWORD dwDestAttribs);

	string_list m_pFileList;
};

OBJECT_ENTRY_AUTO(__uuidof(NxtShellExtension), CNxtShellExtension)


