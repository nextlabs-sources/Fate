// ShellExt.cpp : Implementation of CShellExt

#include "stdafx.h"
#include "ShellExt.h"
#include "resource.h"
#include <shellapi.h>

typedef void (*SHOWVIEWPANEL)(LPCWSTR pszFileName) ;

// CShellExt
extern HINSTANCE g_hInstance;
extern std::wstring g_strFileTaggingDllPath;

HRESULT CShellExt::Initialize(LPCITEMIDLIST pidlFolder, IDataObject *pdtobj, HKEY hkeyProgID)
{
	if(NULL == pdtobj){
		return E_INVALIDARG;
	}
	FORMATETC feFmtEtc = { CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
	STGMEDIUM mdmSTG = { TYMED_HGLOBAL };

	if(FAILED(pdtobj->GetData(&feFmtEtc, &mdmSTG)))
	{
		return E_INVALIDARG;
	}

	//get user droped file counts
	HDROP hDrop = (HDROP)GlobalLock(mdmSTG.hGlobal);
	// Make sure it worked.
	if(NULL == hDrop)
	{
		return E_INVALIDARG;
	}

	UINT uNumFiles = DragQueryFile( hDrop,
		static_cast<UINT>(-1), NULL, 0 );    
	if( 0 == uNumFiles )
	{ 
		return E_INVALIDARG; 
	}
	HRESULT hr = S_OK;
	// The author has encountered situations where
	
	// Loop through all the files that were selected.
	m_listFiles.clear();
	for(UINT i = 0; i < uNumFiles; i++)
	{
		//   MAX_PATH was a bit too small...
		WCHAR szFile[ MAX_PATH * 2 + 1 ] = {0};
		DragQueryFile( hDrop, i, szFile, MAX_PATH * 2 );
		// If the file name is a directory, silently skip
		//   We should not encounter this...
		if (::GetFileAttributes( szFile ) & FILE_ATTRIBUTE_DIRECTORY)
		{ 
			continue; 
		}
		// Add the file name to the end of the list.
		m_listFiles.push_back(szFile);
	//	OutputDebugStringW(szFile);
	}
	
	return hr;


}

HRESULT CShellExt::QueryContextMenu(HMENU hmenu, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags)
{
	
	if(!g_hInstance)
		{
			return E_INVALIDARG;
		}

		if(!(uFlags & CMF_DEFAULTONLY)){
			wchar_t szFileTagging[101] = {0};
			LoadStringW(g_hInstance, IDS_TAGGING_ASSISTANT, szFileTagging, 100);
	
			wchar_t szViewReset[101] = {0};
			LoadStringW(g_hInstance, IDS_VIEWRESET_TAGS, szViewReset, 100);
	
			HMENU hSubMenu = ::CreateMenu();
			if(!hSubMenu)
			{
				return E_INVALIDARG;
			}
	
			BOOL bRet = ::AppendMenuW(hSubMenu, MF_STRING, idCmdFirst, szViewReset);
	
			if(!bRet)
			{
				return E_INVALIDARG;
			}
	
			bRet = ::InsertMenuW(hmenu, indexMenu, MF_BYPOSITION | MF_POPUP, (UINT)hSubMenu, szFileTagging);
		
			if(!bRet)
			{
				return E_INVALIDARG;
			}
		
			return MAKE_HRESULT(SEVERITY_SUCCESS, 0, USHORT(1));
		}
		return MAKE_HRESULT(SEVERITY_SUCCESS, 0, USHORT(0));

	
	

}

HRESULT CShellExt::GetCommandString(UINT idCmd, UINT uFlags, UINT *pwReserved, LPSTR pszName, UINT cchMax)
{
	USES_CONVERSION;
	
	std::wstring strTips;

	switch (uFlags)
	{
	case GCS_VERB:
		break;
	case GCS_HELPTEXTW:
		switch (idCmd)
		{
		case 0:
			{
				wchar_t szHint[1001] = {0};
				LoadStringW(g_hInstance, IDS_HINT, szHint, 1000);
				strTips = std::wstring(szHint);
			}
			
			break;
		default:
			break;
		}
		if (strTips.length() > 0)
		{
			memcpy(pszName, strTips.c_str(), (strTips.length() + 1) * sizeof(wchar_t));
		}
		break;
	}

	return NOERROR;
}

HRESULT CShellExt::InvokeCommand(LPCMINVOKECOMMANDINFO pici)
{
	USES_CONVERSION;
//	int nFlag = HIWORD(pici->lpVerb);
	int nMnuId = LOWORD(pici->lpVerb);  

	if(m_listFiles.size() < 1)
		return E_INVALIDARG;

	std::wstring strFileName = *(m_listFiles.begin());

	if (SetCurrentDirectoryW(g_strFileTaggingDllPath.c_str()) == 0) 
	{
	}

	HMODULE hFileTagging = LoadLibraryW(FILETAGGING_DLL_NAME);
	if(!hFileTagging)
	{
		return E_INVALIDARG;
	}

	SHOWVIEWPANEL fpShow = (SHOWVIEWPANEL)GetProcAddress(hFileTagging, "ShowResetPanel");
	if(fpShow)
		fpShow(strFileName.c_str());
//	ShowResetPanel(strFileName.c_str());
	
//	if(hFileTagging)
//		FreeLibrary(hFileTagging);


	return NOERROR;
}
