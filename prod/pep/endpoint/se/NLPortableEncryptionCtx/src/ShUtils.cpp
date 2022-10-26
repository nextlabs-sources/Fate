#include "stdafx.h"
#include "resource.h"
#include "ShUtils.h"
#include "errno.h"
#include <direct.h>
#include <io.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string>
#include "nlconfig.hpp"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CRITICAL_SECTION g_critSectionBreak;

////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////
void ShMsgPump()
{
	// if we do MFC stuff in an exported fn, call this first!
	AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

	DWORD dInitTime = GetTickCount();
	MSG m_msgCur;                   // current message
	CWinApp	*pWinApp = AfxGetApp();   
	while (::PeekMessage(&m_msgCur, NULL, NULL, NULL, PM_NOREMOVE)  &&
			(GetTickCount() - dInitTime < 200) )
	{
		pWinApp->PumpMessage();
	}
}

////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////
int WideCharToLocal(LPTSTR pLocal, LPWSTR pWide, DWORD dwChars)
{
	*pLocal = 0;

	#ifdef UNICODE
	lstrcpyn(pLocal, pWide, dwChars);
	#else
	WideCharToMultiByte( CP_ACP, 
						 0, 
						 pWide, 
						 -1, 
						 pLocal, 
						 dwChars, 
						 NULL, 
						 NULL);
	#endif

	return lstrlen(pLocal);
}

////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////
int LocalToWideChar(LPWSTR pWide, LPTSTR pLocal, DWORD dwChars)
{
	*pWide = 0;

	#ifdef UNICODE
	lstrcpyn(pWide, pLocal, dwChars);
	#else
	MultiByteToWideChar( CP_ACP, 
						 0, 
						 pLocal, 
						 -1, 
						 pWide, 
						 dwChars); 
	#endif

	return lstrlenW(pWide);
}

////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////
STDMETHODIMP CreateShellExtMenu(HMENU hMenu,
                                         UINT indexMenu,
                                         UINT idCmdFirst,
                                         UINT idCmdLast,
                                         UINT uFlags,
                                         HBITMAP hbmp,
										 BOOL bWrap)
{
   UINT idCmd = idCmdFirst;

   BOOL bAppendItems=TRUE;


   HINSTANCE hInst = AfxGetInstanceHandle();

   if ((uFlags & 0x000F) == CMF_NORMAL)  //Check == here, since CMF_NORMAL=0
   {
       ;
   }
   else
   if (uFlags & CMF_VERBSONLY)
   {
       ;
   }
   else
   if (uFlags & CMF_EXPLORE)
   {
       ;
   }
   else if (uFlags & CMF_DEFAULTONLY)
   {
      bAppendItems = FALSE;
   }
   else
   {
      bAppendItems = FALSE;
   }

   BOOL bPopUp = TRUE;

   if (bAppendItems)
   {
	   HMENU hParentMenu;

      // add popup
      if (bPopUp)
      {
         hParentMenu = ::CreateMenu();
      }
      else
      {
         // or not
         hParentMenu = hMenu;
      }

      if (hParentMenu) 
      {
         // pop-up title
         if (bPopUp)
         {
			#pragma warning(push)
			#pragma warning(disable:4311)
			InsertMenu(hMenu, indexMenu++, MF_POPUP|MF_BYPOSITION, reinterpret_cast<UINT>(hParentMenu), SHELLEXNAME);
			#pragma warning(pop)
            SetMenuItemBitmaps(hMenu, indexMenu-1, MF_BITMAP | MF_BYPOSITION, hbmp, hbmp);
         }

		 if (!bWrap)
		 {
			InsertMenuW(hParentMenu, indexMenu++, MF_STRING|MF_BYPOSITION, idCmd++, L"Unwrap");
			InsertMenuW(hParentMenu, indexMenu++, MF_STRING|MF_BYPOSITION, idCmd++, L"Unwrap...");
		 }
		 else
		 {
			InsertMenuW(hParentMenu, indexMenu++, MF_STRING|MF_BYPOSITION, idCmd++, L"Wrap");
		 }

      }
                       
      return ResultFromShort(idCmd-idCmdFirst); //Must return number of menu
      //items we added.
   }
   return NOERROR;
}

////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////
STDMETHODIMP GetSelectedFiles(LPCITEMIDLIST pIDFolder,
                                   CComPtr<IDataObject> &pDataObj,
                                   CStringArray &csaPaths)
{
   // get these paths into a CStringArray
   csaPaths.RemoveAll();

   // fetch all of the file names we're supposed to operate on
   if (pDataObj) 
   {
	   STGMEDIUM medium;
	   FORMATETC fe = { CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};


	   HRESULT hr = pDataObj->GetData (&fe, &medium);
	   if (FAILED (hr))
	   {
		   return E_FAIL;
	   }

	   // buffer to receive filenames
	   WCHAR path[MAX_PATH];

	   // how many are there?
	   UINT fileCount = DragQueryFileW((HDROP)medium.hGlobal, 0xFFFFFFFF, path, MAX_PATH);

	   if (fileCount>0)
       {
           // avoid wasting mem when this thing gets filled in
           csaPaths.SetSize(fileCount);

           // stash the paths in our CStringArray
           for (UINT i=0;i<fileCount;i++) 
           {
               // clear old path
               memset(path, 0, sizeof(path));
               // fetch new path
               if (DragQueryFileW((HDROP)medium.hGlobal, i, path, MAX_PATH)) 
               {
                   csaPaths.SetAt(i, path);
               }
           }

           csaPaths.FreeExtra();
       }

      // free our path memory - we have the info in our CStringArray
      ReleaseStgMedium(&medium);
   }

   return NOERROR;
}

std::string MyWideCharToMultipleByte(const std::wstring & strValue)
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

std::wstring MyMultipleByteToWideChar(const std::string & strValue)
{
	int nLen = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, strValue.c_str(), (int)strValue.length(), NULL, 0 );
	DWORD dError = 0 ;
	if( nLen == 0 )
	{
		dError = ::GetLastError() ;
		if( dError == ERROR_INVALID_FLAGS )
		{
			nLen = MultiByteToWideChar(CP_ACP, 0, strValue.c_str(), (int)strValue.length(), NULL, 0 );
		}
	}
	wchar_t* pBuf = new wchar_t[nLen + 1];
	if(!pBuf)
		return L"";

	memset(pBuf, 0, (nLen + 1) * sizeof(wchar_t));
	MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, strValue.c_str(), (int)strValue.length(), pBuf, nLen);
	dError = ::GetLastError() ;
	if( dError == ERROR_INVALID_FLAGS )
	{
		MultiByteToWideChar(CP_ACP, 0, strValue.c_str(), (int)strValue.length(), pBuf, nLen);
	}
	std::wstring strResult(pBuf);

	if(pBuf)
	{
		delete []pBuf;
		pBuf = NULL;
	}
	return strResult;
}

std::wstring GetCommonComponentsDir()
{
	wchar_t szDir[MAX_PATH + 1] = {0};
	if(NLConfig::ReadKey(L"SOFTWARE\\NextLabs\\CommonLibraries\\InstallDir", szDir, MAX_PATH))
	{
		wchar_t* pTemp = wcsrchr(szDir, L'\\');
		if ( pTemp && !( * ( pTemp + 1 ) ) )
		{
			*pTemp = 0;
		}
#ifdef _WIN64
		wcsncat_s(szDir, MAX_PATH, L"\\bin64\\", _TRUNCATE);
#else
		wcsncat_s(szDir, MAX_PATH, L"\\bin32\\", _TRUNCATE);
#endif
		return szDir;
	}

	return L"";
}
