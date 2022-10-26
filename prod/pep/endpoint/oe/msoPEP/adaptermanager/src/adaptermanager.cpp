// adaptermanager.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "adaptermanager.h"


#ifdef _MANAGED
#pragma managed(push, off)
#endif
HINSTANCE g_hInstance;
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		{
			g_hInstance=hModule;
			break;
		}
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
    return TRUE;
}

#ifdef _MANAGED
#pragma managed(pop)
#endif


static BOOL GetModuleBaseName(std::wstring& wstrModuleBaseName)
{
	WCHAR wzModuleFileName[MAX_PATH+1];memset(wzModuleFileName,0,sizeof(wzModuleFileName));
	DWORD dwRet=GetModuleFileName(g_hInstance,wzModuleFileName,MAX_PATH);
	
	if(dwRet==0||GetLastError() == ERROR_INSUFFICIENT_BUFFER)
		return FALSE;
	std::wstring wstrTemp=wzModuleFileName;
	std::wstring::size_type pos=wstrTemp.rfind(L'/');
	if(pos==std::wstring::npos)
	{
		pos=wstrTemp.rfind(L'\\');
		if(pos==std::wstring::npos)
			return FALSE;
	}
	
	wstrModuleBaseName=wstrTemp.substr(0,pos);
	return TRUE;
}
STDAPI RepositoryAdaptersManager(WCHAR* wzEnforcerKeyName,CComPtr<IDispatch> pItem,AdapterCommon::Attachments* pAtts)
{
	AdapterManager adapterMgr(pItem,pAtts);
	adapterMgr.Init(wzEnforcerKeyName);
	BOOL bRet=adapterMgr.Do();
	//if do adapter false return E_FAIL
	return bRet==TRUE?S_OK:E_FAIL;
}
STDAPI RepositoryAdaptersManagerEx(WCHAR* wzEnforcerKeyName,CComPtr<IDispatch> pItem,AdapterCommon::Attachments* pAtts,wchar_t** pwchObligationTopMessage,int &iTopMsgLength,wchar_t** pwchObligationBottomMessage,int &iBottomMsgLength)
{
	AdapterManager adapterMgr=AdapterManager(pItem,pAtts);
	adapterMgr.Init(wzEnforcerKeyName);
	BOOL bRet=adapterMgr.DoEx(pwchObligationTopMessage,iTopMsgLength,pwchObligationBottomMessage,iBottomMsgLength); 
	return bRet==TRUE?S_OK:E_FAIL;
}
STDAPI ReleaseRepositoryAdaptersResource(wchar_t * pwch,bool bIsArray)
{
	if (pwch!=NULL)
	{
		if(bIsArray)
		{
			delete[] pwch;
		}
		else
		{
			delete pwch;
		}
	}
	return true;
}