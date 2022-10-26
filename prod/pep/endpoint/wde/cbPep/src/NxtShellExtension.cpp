// NxtShellExtension.cpp : Implementation of CNxtShellExtension

#include "stdafx.h"
#include "NxtShellExtension.h"
#include <ShellAPI.h>
#include <commctrl.h>
#include <string>
#include <list>
#include <atlwin.h> 
#include "celog.h"
const wchar_t BLOCK_HINT[] = L"This action is not permitted. \nPlease use Windows Explorer to perform the action.";

#include "policy.h"

const int kPathLen = 1024;
extern CELog cbPepLog;

// CNxtShellExtension
HRESULT CNxtShellExtension::Initialize(LPCITEMIDLIST pidlFolder, IDataObject *pdtobj, HKEY hkeyProgID)
{
	CNxtMgr* mgr = CNxtMgr::Instance();
	if( false == boost::algorithm::iends_with(mgr->GetCurModuleName(), L"explorer.exe") )
	{
		return S_OK;
	}

	TCHAR szFile [kPathLen];
	UINT uNumFiles;
	HDROP hdrop;
	FORMATETC etc = { CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
	STGMEDIUM stg = { TYMED_HGLOBAL };

	if ( FAILED( pdtobj->GetData (&etc, &stg)))
		return E_INVALIDARG;


	hdrop = (HDROP)GlobalLock ( stg.hGlobal );
	if ( NULL == hdrop )
	{
		ReleaseStgMedium ( &stg );
		return E_INVALIDARG;
	}


	uNumFiles = DragQueryFile ( hdrop, 0xFFFFFFFF, NULL, 0 );

	for ( UINT uFile = 0; uFile < uNumFiles; uFile++ )
	{
		if ( 0 == DragQueryFile ( hdrop, 
			uFile, szFile, kPathLen ))
			continue;

		m_pFileList.push_back ( szFile );
	} // end for 



	GlobalUnlock ( stg.hGlobal );
	ReleaseStgMedium ( &stg );
	return S_OK;
}

HRESULT CNxtShellExtension::QueryContextMenu(HMENU hmenu, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags)
{
	CNxtMgr* mgr = CNxtMgr::Instance();
	if(mgr->IsInitialized() && mgr->NeedBlock())
	{
		if (hmenu != NULL)
		{
			::DestroyMenu(hmenu);
		}
		cbPepLog.Log(CELOG_DEBUG,L"cbPep, block context menu!\n");
		//OutputDebugStringW(L"cbPep, block context menu!\n");
		return S_FALSE;
	}
	else
	{
		if(!(boost::algorithm::iends_with(mgr->GetCurModuleName(), L"explorer.exe") && hmenu && uFlags))
		{
			return S_OK;
		}

		cbPepLog.Log(CELOG_DEBUG,L"cbPep, not inited or don't need to block, but need to check send to of explore context menu!\n");
		//OutputDebugStringW(L"cbPep, not inited or don't need to block, but need to check send to of explore context menu!\n");

		bool bdeny=false;
		bool bHaveQueriedFile=false;
		string_list::const_iterator it, itEnd;
		for ( it = m_pFileList.begin(), itEnd = m_pFileList.end(); it != itEnd; it++ )
		{
			cbPepLog.Log(CELOG_DEBUG,L"%s",it->c_str());
			//OutputDebugStringW(it->c_str());

			wstring filepath(*it);
			boost::algorithm::replace_all(filepath, L"/", L"\\");

			DWORD dwattr=GetFileAttributes(it->c_str());
			if(FILE_ATTRIBUTE_DIRECTORY==(dwattr&FILE_ATTRIBUTE_DIRECTORY))
			{
				cbPepLog.Log(CELOG_DEBUG,"this is folder");
				//OutputDebugStringA("this is folder");
			}
			else
			{
				if (bHaveQueriedFile)
				{
					continue;
				}

				bHaveQueriedFile=true;

				wstring::size_type pos=filepath.rfind(L'\\');
				if (pos!=wstring::npos)
				{
					filepath=filepath.substr(0,pos);
				}
			}
			
			string strfilepath = mgr->MyWideCharToMultipleByte(filepath);

			strfilepath+="\\";

			CPolicy* ins=CPolicy::GetInstance();
			ins->query("SEND", 
				strfilepath, 
				bdeny);

			if (bdeny)
			{
				break;
			}
		}

		if(bdeny)
		{
			int count=GetMenuItemCount(hmenu);
			for (int i=0;i<count;i++)
			{
				wchar_t stringtxt[256]={0};
				GetMenuString(hmenu,i,stringtxt,255,MF_BYPOSITION);

				if (stringtxt[0])
				{
					if (!wcscmp(stringtxt,L"Se&nd To") || !wcscmp(stringtxt,L"Se&nd to") || _wcsicmp(stringtxt, L"·¢ËÍµ½") == 0)
					{
						EnableMenuItem(hmenu,i,MF_BYPOSITION|MF_GRAYED);
						break;
					}
				}
			}
		}
	}

	return S_OK;
}

HRESULT CNxtShellExtension::GetCommandString( 
	_In_  UINT_PTR idCmd,
	_In_  UINT uType,
	/* _Reserved_ */  UINT *pReserved,
	_Out_cap_(cchMax)  LPSTR pszName,
	_In_  UINT cchMax)
{
	idCmd;uType;pReserved;pszName;cchMax;
	return S_OK;
}

HRESULT CNxtShellExtension::InvokeCommand(LPCMINVOKECOMMANDINFO )
{
	return S_OK;
}

UINT CNxtShellExtension::CopyCallback(HWND /*hwnd*/, UINT /*wFunc*/, UINT /*wFlags*/, LPCWSTR pszSrcFile, DWORD /*dwSrcAttribs*/, LPCWSTR pszDestFile, DWORD /*dwDestAttribs*/)
{
	wchar_t temp[1025] = {0};
	wchar_t tempfolder[1025] = {0};
	GetTempPathW(1024, temp);
	GetLongPathNameW(temp, tempfolder, 1024);

	std::wstring log = boost::str(boost::wformat(L"CopyCallBack, source: %s, dest: %s, temp folder: %s\n") % (pszSrcFile?pszSrcFile: L"NULL") % (pszDestFile?pszDestFile: L"NULL") % tempfolder);
	cbPepLog.Log(CELOG_DEBUG,L"%s",log.c_str());
	//OutputDebugStringW(log.c_str());

	HANDLE pub = GetModuleHandle(L"Publisher.dll");

	if (pub && 
		pszSrcFile && 
		boost::algorithm::icontains(pszSrcFile, L"\\data") &&
		((pszDestFile != NULL && wcslen(pszDestFile) == 0) || pszDestFile == NULL)
	   )
	{
		cbPepLog.Log(CELOG_DEBUG,L"Ingore for Adobe presenter\n");
		//OutputDebugStringW(L"Ingore for Adobe presenter\n");
		return IDYES;
	}

	if(pszSrcFile != NULL && 
		(wcsstr(pszSrcFile, L"CEZIP_") != NULL || boost::algorithm::istarts_with(pszSrcFile, tempfolder)) && 
		( (pszDestFile != NULL && wcslen(pszDestFile) == 0) || pszDestFile == NULL)
		)
	{
		log = boost::str(boost::wformat(L"CopyCallback, ignored folder: %s\n") % (pszSrcFile? pszSrcFile: L"NULL"));
		cbPepLog.Log(CELOG_DEBUG,L"%s",log.c_str());
		//OutputDebugStringW(log.c_str());
		return IDYES;//fix the problem, CreateDPFfile with office2007, bug18239
	}

	CNxtMgr* mgr = CNxtMgr::Instance();
	if(mgr->IsInitialized() && mgr->NeedBlock())
	{
		log = boost::str(boost::wformat(L"CopyCallBack, blocked, source: %s, dest: %s\n")  % (pszSrcFile?pszSrcFile: L"NULL") % (pszDestFile?pszDestFile: L"NULL"));
		cbPepLog.Log(CELOG_DEBUG,L"%s",log.c_str());
		//OutputDebugStringW(log.c_str());

		mgr->PopupWarningBox(L"Warning", BLOCK_HINT);
		
		return IDCANCEL;
	}
	else
	{
		return IDYES;
	}
}


