// PA_Test.cpp : Implementation of WinMain


#include "stdafx.h"
#include "resource.h"
#include "PA_Test.h"
#include "PA_Demo.h"

class CPA_TestModule : public CAtlExeModuleT< CPA_TestModule >
{
public :
	DECLARE_LIBID(LIBID_PA_TestLib)
	DECLARE_REGISTRY_APPID_RESOURCEID(IDR_PA_TEST, "{2959302D-70D2-4502-8029-91FB9C3B9F03}")
};

CPA_TestModule _AtlModule;



//
extern "C" int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, 
								LPTSTR lpCmdLine, int nShowCmd)
{
	//MessageBox( 0,lpCmdLine,0,0 ) ;
	if((( lpCmdLine == NULL )|| wcslen(  lpCmdLine ) == 0 )||(wcscmp(lpCmdLine ,L"/RegServer") != 0 ) ) 
	{
		CPA_Demo paDemo ;
		paDemo.m_hInst =  hInstance ;
		paDemo.DoModal() ;
		paDemo.m_hInst = NULL ;

	}
    return _AtlModule.WinMain(nShowCmd);
}

