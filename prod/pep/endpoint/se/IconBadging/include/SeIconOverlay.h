// SeIconOverlay.h : Declaration of the CSeIconOverlay

#pragma once
#include "resource.h"       // main symbols
#include "IconBadging.h"
#include <shlobj.h>
#include <string>
#include <vector>


#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Single-threaded COM objects are not properly supported on Windows CE platform, such as the Windows Mobile platforms that do not include full DCOM support. Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support creating single-thread COM object's and allow use of it's single-threaded COM object implementations. The threading model in your rgs file was set to 'Free' as that is the only threading model supported in non DCOM Windows CE platforms."
#endif

class CDRMPathReader
{
public:
    CDRMPathReader();
    virtual ~CDRMPathReader();

    BOOL IsEncryptedDirectory(_In_ LPCWSTR wzDir);

protected:
    BOOL GetDRMConfigFile();
    BOOL IsFileChanged();
    BOOL NeedToUpdate();
    BOOL UpdateDRMLists();

private:
    std::wstring m_strConfigFile;
    std::vector<std::wstring> m_vecDRMs;
    CRITICAL_SECTION    m_cs;
    FILETIME    m_ftWrite;
    SYSTEMTIME  m_stLastUpdate;
};



// CSeIconOverlay

class ATL_NO_VTABLE CSeIconOverlay :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CSeIconOverlay, &CLSID_SeIconOverlay>,
    public IShellIconOverlayIdentifier,
	public IDispatchImpl<ISeIconOverlay, &IID_ISeIconOverlay, &LIBID_IconBadgingLib, /*wMajor =*/ 1, /*wMinor =*/ 0>
{
public:
	CSeIconOverlay()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_SEICONOVERLAY)


BEGIN_COM_MAP(CSeIconOverlay)
	COM_INTERFACE_ENTRY(ISeIconOverlay)
	COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(IShellIconOverlayIdentifier)
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
	// IShellIconOverlayIdentifier Methods
	STDMETHOD(GetOverlayInfo)(LPWSTR pwszIconFile, int cchMax,int *pIndex,DWORD* pdwFlags);
	STDMETHOD(GetPriority)(int* pPriority);
	STDMETHOD(IsMemberOf)(LPCWSTR pwszPath,DWORD dwAttrib);

private:
    CDRMPathReader  g_drm;
};

OBJECT_ENTRY_AUTO(__uuidof(SeIconOverlay), CSeIconOverlay)
