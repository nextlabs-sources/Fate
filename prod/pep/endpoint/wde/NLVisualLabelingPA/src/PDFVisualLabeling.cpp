#include "stdafx.h"
#include "VLObligation.h"
#include "PDFVisualLabeling.h"
#include "Utility.h"

using namespace NM_VLObligation;

#ifdef _M_IX86
	#define VIEW_MODULE_NAME	L"NLVLViewPrint32.dll"
#elif defined (_M_AMD64) || (_WIN64)
	#define VIEW_MODULE_NAME	L"NLVLViewPrint.dll"
#endif

#define VIEW_EXPORTFUNC_NAME	"ViewVisualLabeling"
typedef int (*P_ViewVisualLabeling)(const wstring& strFilePath, const VisualLabelingInfo& theInfo,HWND hParent);
static HMODULE g_hViewLibMod = NULL;
static P_ViewVisualLabeling g_pViewFunc=NULL;
wstring	g_strModuleParentPath=L"";

INT InvokeViewVL(const wstring& strFilePath, const VisualLabelingInfo& theInfo,HWND hParent)
{
	if(g_strModuleParentPath.empty())
	{
		wchar_t wzPath[MAX_PATH+1]={0};
		DWORD dwLen = GetModuleFileNameW(g_hModule,wzPath,MAX_PATH);
		DWORD i=1;
		while(i++ < dwLen)
		{
			if(wzPath[dwLen-i] == L'\\')	
			{
				wzPath[dwLen-i]=L'\0';
				break;
			}
		}
		g_strModuleParentPath = wzPath;
	}
	if(g_pViewFunc == NULL)
	{
		if(g_hViewLibMod == NULL)	
		{
			wstring strDllPath = g_strModuleParentPath + L"\\";
			strDllPath += VIEW_MODULE_NAME;
			g_hViewLibMod = LoadLibraryW(strDllPath.c_str());
			if(g_hViewLibMod == NULL)
			{
				LogOut(true,L"Load Library:%s failed...",VIEW_MODULE_NAME);
			}
		}
		if(g_hViewLibMod != NULL)
		{
			g_pViewFunc = (P_ViewVisualLabeling)GetProcAddress(g_hViewLibMod,VIEW_EXPORTFUNC_NAME);
			if(g_pViewFunc == NULL)
			{
				LogOut(true,L"GetProcAddress:%s failed...",VIEW_EXPORTFUNC_NAME);
			}
		}
	}
	if(g_pViewFunc != NULL)
	{
		return g_pViewFunc(strFilePath,theInfo,hParent);
	}
	return 0;
}


bool CPDFVL::DoVisualLabeling(const wstring& strFilePath, const PABase::ACTION& theAction,const VisualLabelingInfo& newInfo,HWND hView)
{
	switch(theAction)
	{
	case AT_READ:
		{
			// invoke floating window module 
			InvokeViewVL(strFilePath,newInfo,hView);
		}
		break;
	case AT_PERSISTED:
		{
			
		}
		break;
	default:
		break;
	}
	return true;
}