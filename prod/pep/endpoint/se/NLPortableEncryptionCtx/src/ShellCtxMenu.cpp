#include "stdafx.h"
#include "resource.h"
#include "priv.h"
#include "ShellExt.h"
#include "NLPortableEncryptionCtx.h"

#include <shlobj.h>
#include <string>
#include <shlguid.h>
#include <fstream>
#include "io.h"

#include <boost/algorithm/string.hpp>

// utilities
#include "ShUtils.h"
#include "FileProcess.h"
#include "nl_sysenc_lib.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

EXTERN_C IMAGE_DOS_HEADER __ImageBase;

const WCHAR GENPASSWORD_CONFIG_FILENAME[]=L"NXGeneratePwdCtx.config";

/*----------------------------------------------------------------

  FUNCTION: CShellExt::InvokeCommand(LPCMINVOKECOMMANDINFO)

  PURPOSE: Called by the shell after the user has selected on of the
           menu items that was added in QueryContextMenu(). This is 
           the function where you get to do your real work!!!

  PARAMETERS:
    lpcmi - Pointer to an CMINVOKECOMMANDINFO structure

  RETURN VALUE:


  COMMENTS:
 
----------------------------------------------------------------*/

std::wstring GetTargetFolder()
{
    BROWSEINFOW     bi; 
    LPITEMIDLIST    pidl = NULL;
    WCHAR           szDisplayName[MAX_PATH];
    WCHAR           szOutPathName[MAX_PATH];
    std::wstring    strOut;

    memset(&bi, 0, sizeof(bi)); 
    memset(szDisplayName, 0, sizeof(szDisplayName)); 
    memset(szOutPathName, 0, sizeof(szOutPathName));

    // Get
    bi.hwndOwner        =   NULL; 
    bi.pidlRoot         =   NULL; 
    bi.pszDisplayName   =   szDisplayName; 
    bi.lpszTitle        =   L"Select a Directory for Unwrapped File:"; 
    bi.ulFlags          =   BIF_RETURNONLYFSDIRS;
    bi.lParam           =   NULL; 
    bi.iImage           =   0;
    pidl = SHBrowseForFolderW(&bi);
    if (NULL != pidl)
    {
        SHGetPathFromIDList(pidl,szOutPathName);
        strOut = szOutPathName;
        if(!strOut.empty() && !boost::algorithm::ends_with(strOut, L"\\"))
            strOut += L"\\";
    }

    return strOut;
}

std::wstring GetSourceFileName(_In_ LPCWSTR wzSource)
{
    const WCHAR* pos = wcsrchr(wzSource, L'\\');
    if(NULL != pos)
        return (pos+1);
    else
        return L"";
}

std::wstring BuildParameter(_In_ LPCWSTR wzSource, _In_opt_ LPCWSTR wzTarget, BOOL bWrap)
{
    std::wstring strParam;

    strParam  = L"/path \"";
    strParam += wzSource;
    strParam += bWrap?L"\" --wrap":L"\" --unwrap";
    if(wzTarget)
    {
        strParam  += L" --output \"";
        strParam  += wzTarget;
        strParam  += L"\"";
    }

    return strParam;
}

std::wstring GetAppDir()
{
    WCHAR   wzPath[MAX_PATH+1]  = {0};
    HMODULE hMod                = NULL;

    hMod = reinterpret_cast<HMODULE>(&__ImageBase);
    GetModuleFileNameW(hMod, wzPath, MAX_PATH);
    wchar_t* p = wcsrchr(wzPath, '\\');
    if(p) *p = '\0';

    return wzPath;
}

STDMETHODIMP CShellExt::InvokeCommand(LPCMINVOKECOMMANDINFO lpcmi)
{
	// look at all the MFC stuff in here! call this to allow us to 
	// not blow up when we try to use it.
	AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

	HINSTANCE hInst = AfxGetInstanceHandle();

	HWND hParentWnd = lpcmi->hwnd;
                                      
	HRESULT hr = NOERROR;

    //If HIWORD(lpcmi->lpVerb) then we have been called programmatically
    //and lpVerb is a command that should be invoked.  Otherwise, the shell
    //has called us, and LOWORD(lpcmi->lpVerb) is the menu ID the user has
    //selected.  Actually, it's (menu ID - idCmdFirst) from QueryContextMenu().
	UINT idCmd = 0;

    std::wstring strSrc = m_csaPaths.GetAt(0).GetString();
    std::wstring strSrcName = GetSourceFileName(strSrc.c_str());
  //  std::wstring strParams;

    if (!HIWORD(lpcmi->lpVerb)) 
    {
        idCmd = LOWORD(lpcmi->lpVerb);

        // process it
        switch (idCmd) 
        {
        case 1: // operation 2
            {
                if(boost::algorithm::iends_with(strSrc, L".nxl"))
                {
                    std::wstring strOut = GetTargetFolder();
                    if(!strOut.empty())
                        strOut += strSrcName;
                    else
                        strOut = strSrc;
                    strOut = strOut.substr(0, strOut.length()-4);

                    std::wstring strParams = BuildParameter(strSrc.c_str(), strOut.c_str(), FALSE);
                    if(!strParams.empty())
                    {
                        std::wstring strDir = GetAppDir();
                        ShellExecuteW( NULL, L"open", L"NLPortableEncryption.exe", strParams.c_str(), strDir.c_str(), SW_SHOWNORMAL);
                    }
                }
                else
                {
                    MessageBoxW(NULL, L"This file is not a valid NextLabs portable file!", L"NextLabs Portable Encryption", MB_OK);
                }
            }
            break;

        case 0: // operation 1 and others
        default:
            {
                BOOL bWrap = true;
                if(boost::algorithm::iends_with(strSrc, L".nxl"))
                    bWrap = FALSE;

                std::wstring strParams = BuildParameter(strSrc.c_str(), NULL, bWrap);
                if(!strParams.empty())
                {
                    std::wstring strDir = GetAppDir();
                    ShellExecuteW(NULL, L"open", L"NLPortableEncryption.exe", strParams.c_str(), strDir.c_str(), SW_SHOWNORMAL);
                }
            }
            break;
        }	// switch on command
    }

   return hr;
}


////////////////////////////////////////////////////////////////////////
//
//  FUNCTION: CShellExt::GetCommandString(...)
//
//  PURPOSE: Retrieve various text strinsg associated with the context menu
//
//	Param			Type			Use
//	-----			----			---
//	idCmd			UINT			ID of the command
//	uFlags			UINT			which type of info are we requesting
//	reserved		UINT *			must be NULL
//	pszName			LPSTR			output buffer
//	cchMax			UINT			max chars to copy to pszName
//
////////////////////////////////////////////////////////////////////////

#ifdef _WIN64
STDMETHODIMP CShellExt::GetCommandString(UINT_PTR idCmd,
										 UINT uFlags,
										 UINT FAR *reserved,
										 LPSTR pszName,
										 UINT cchMax)
#else
STDMETHODIMP CShellExt::GetCommandString(UINT idCmd,
                                         UINT uFlags,
                                         UINT FAR *reserved,
                                         LPSTR pszName,
                                         UINT cchMax)
#endif
{	
	AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

	HINSTANCE hInst = AfxGetInstanceHandle();

	switch (uFlags) 
   {
	case GCS_HELPTEXT:		// fetch help text for display at the bottom of the 
							// explorer window
//#pragma warning( push )
//#pragma warning( disable : 4065 4996 )
            strncpy_s(pszName, cchMax, "NextLabs", _TRUNCATE);
//#pragma warning( pop )
		break;

	case GCS_VALIDATE:
		break;

	case GCS_VERB:			// language-independent command name for the menu item 
//#pragma warning( push )
//#pragma warning( disable : 4065 4996 )
            strncpy_s(pszName, cchMax, "NextLabs", _TRUNCATE);
//#pragma warning( pop )
		break;
	}
    return NOERROR;
}

///////////////////////////////////////////////////////////////////////////
//
//  FUNCTION: CShellExt::QueryContextMenu(HMENU, UINT, UINT, UINT, UINT)
//
//  PURPOSE: Called by the shell just before the context menu is displayed.
//           This is where you add your specific menu items.
//
//  PARAMETERS:
//    hMenu      - Handle to the context menu
//    indexMenu  - Index of where to begin inserting menu items
//    idCmdFirst - Lowest value for new menu ID's
//    idCmtLast  - Highest value for new menu ID's
//    uFlags     - Specifies the context of the menu event
//
//  RETURN VALUE:
//
//
//  COMMENTS:
//
///////////////////////////////////////////////////////////////////////////
typedef BOOL (__stdcall *SE_IsEncryptedType)( _In_ const wchar_t* path );

STDMETHODIMP CShellExt::QueryContextMenu(HMENU hMenu,
                                         UINT indexMenu,
                                         UINT idCmdFirst,
                                         UINT idCmdLast,
                                         UINT uFlags)
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

   std::wstring strFile = m_csaPaths.GetAt(0).GetString();

   if (SE_IsEncrypted(strFile.c_str()))
   {
       if(boost::algorithm::iends_with(strFile, L".nxl"))
	   {
		   return CreateShellExtMenu(hMenu, indexMenu, idCmdFirst, idCmdLast, uFlags, (HBITMAP)m_menuBmp.GetSafeHandle(), FALSE);
	   }
       
       return CreateShellExtMenu(hMenu, indexMenu, idCmdFirst, idCmdLast, uFlags, (HBITMAP)m_menuBmp.GetSafeHandle(), TRUE);
   }   
		
   return S_OK;
}

void CShellExt::ReadConfigFile()
{
	int i = 0;
	int index = 0; 
	int index2 = 0;

	// get path
	HMODULE hM = reinterpret_cast<HMODULE>(&__ImageBase);
	wchar_t szBuffer[MAX_PATH] = {0};
	GetModuleFileNameW(hM, szBuffer, MAX_PATH);
	wchar_t* p = wcsrchr(szBuffer, '\\');
	if(p)
		*p = '\0';
	else
	{
		p=wcsrchr(szBuffer,'/');
		if(p)
			*p='\0';
	}

	WCHAR strFilename[MAX_PATH];
	wcsncpy_s(strFilename,MAX_PATH,szBuffer, _TRUNCATE);
	wcsncat_s(strFilename,MAX_PATH,L"\\", _TRUNCATE);
	wcsncat_s(strFilename,MAX_PATH,GENPASSWORD_CONFIG_FILENAME, _TRUNCATE);
	std::ifstream infile;
	infile.open (strFilename);
	if(infile.fail())
		MessageBoxW(NULL, L"Error Reading NextLabs Config file", L"Error", NULL);
	else
	{	//first line only
	
		char configStr[MAX_PATH];
		if (infile.is_open()) 
		{
			infile.getline(configStr,MAX_PATH);
//			infile >> configStr;
			strncpy_s(m_TargetPath,MAX_PATH, configStr, _TRUNCATE);
		}
	}

	infile.close(); 
}