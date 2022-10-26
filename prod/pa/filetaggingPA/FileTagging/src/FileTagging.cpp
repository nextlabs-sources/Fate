// FileTagging.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "gdiplusinit.h"
#include "UIInterface.h"
#include "Utils.h"
#include "GSControls.h"


#ifdef _WIN64
#define PAF_DLL_NAME							L"pafUI.dll"
#define RESATTRLIB_DLL_NAME						L"resattrlib.dll"
#define RESATTRMGR_DLL_NAME						L"resattrmgr.dll"
#define OFFICE_2K7_DLL_NAME						L"tag_office2k7.dll"
#else
#define PAF_DLL_NAME							L"pafUI32.dll"
#define RESATTRLIB_DLL_NAME						L"resattrlib32.dll"
#define RESATTRMGR_DLL_NAME						L"resattrmgr32.dll"
#define OFFICE_2K7_DLL_NAME						L"tag_office2k732.dll"
#endif
#define ZLIB_DLL_NAME							L"zlib1.dll"
#define FREETYPE_DLL_NAME						L"freetype6.dll"
#define LIBTIFF_DLL_NAME						L"libtiff.dll"
#define PODOFO_DLL_NAME							L"PoDoFoLib.dll"

UINT g_MainWindowMsgID;
HINSTANCE g_hPafDLL = NULL;
HINSTANCE g_hInstance = NULL;
std::vector<HMODULE> g_vModules;

#ifdef _MANAGED
#pragma managed(push, off)
#endif

ULONG_PTR g_gdiplusToken;
BOOL g_bLoadByOE = FALSE;
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID /*lpReserved*/
					 )
{
	switch(ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		{
			BOOL bInit = FALSE;
			if(!bInit)
			{
		//		Gdiplus::GdiplusStartupInput gdiplusStartupInput;
				
				// Initialize GDI+.    
		//		Gdiplus::GdiplusStartup(&g_gdiplusToken, &gdiplusStartupInput, NULL) ;//remove gdiplus

				g_MainWindowMsgID = RegisterWindowMessage( PAF_UI_MAINWINDOW_STATUS ) ;

				InitCommonControls();
				InitializeGSControls();

				std::wstring strPAFPath = std::wstring(PAF_DLL_NAME);
				if(!g_hPafDLL)
				{
					std::wstring strPath = GetInstallPath(hModule);
					strPath.append(strPAFPath);

					g_hPafDLL = LoadLibraryW(strPath.c_str());
					if(!g_hPafDLL)
					{
						DP((L"Can't load paf DLL: %s, Error Code: %d\r\n", strPath.c_str(), GetLastError()));
					}
					else
						DP((L"Load paf DLL successfully! %s\r\n", strPath.c_str()));
				}

				g_hInstance = GetModuleHandle(FILETAGGING_DLL_NAME);

				//Preload all dependencies DLLs
				static std::wstring szDLLs[7] = 
											{
												ZLIB_DLL_NAME,
												FREETYPE_DLL_NAME,
												LIBTIFF_DLL_NAME,
												PODOFO_DLL_NAME,
												OFFICE_2K7_DLL_NAME,
												RESATTRLIB_DLL_NAME,
												RESATTRMGR_DLL_NAME
											};
				std::wstring strInstallPath = GetInstallPath(hModule);
				for(int i = 0; i < 7; i++)
				{
					std::wstring strDllPath;
					strDllPath.append(strInstallPath);
					strDllPath.append(szDLLs[i]);
					HMODULE hDll = LoadLibraryW(strDllPath.c_str());
					DP((L"LoadLibrary %s, handle: %d\r\n", strDllPath.c_str(), hDll));
					if(hDll)
						g_vModules.push_back(hDll);
				}

				//check if this DLL is load by OE, we have different behavior in different module.
				g_bLoadByOE = IsLoadByOE();

				bInit = TRUE;
			}
			break;
		}

	case DLL_PROCESS_DETACH:
		{
			BOOL bShutdown = FALSE;
			if(!bShutdown)
			{
			//	Gdiplus::GdiplusShutdown(g_gdiplusToken);

				UninitializeGSControls();

				if(g_hPafDLL)
				{
					FreeLibrary(g_hPafDLL);
					g_hPafDLL = NULL;
				}

				std::vector<HMODULE>::iterator itr;
				for(itr = g_vModules.begin(); itr != g_vModules.end(); itr++)
				{
					HMODULE hDLL= *itr;
					if(hDLL)
						FreeLibrary(hDLL);
				}

				bShutdown = TRUE;
			}
			break;
		}
	}
	
    return TRUE;
}

#ifdef _MANAGED
#pragma managed(pop)
#endif

