#include "stdafx.h"

#include "NLAction.h"
#include "NLOfficePEP_Comm.h"
#include "NLObMgr.h"


#include "TalkWithSCE.h"


#pragma warning( push )
#pragma warning( disable: 4819 )
#include "madCHook_helper.h"
#pragma warning( pop )


#include "dllmain.h"

extern BOOL g_bDebugMode_L1;
extern BOOL g_bDebugMode_L2;
extern bool g_bFourceLogDefault;
extern CRITICAL_SECTION showbubbleCriticalSection;

//
//	CNxtOfficePEPModule impl
//





void CNxtOfficePEPModule::OnDllAttach(HMODULE hDll)
{
	g_hInstance = hDll;
	hPepDll_ = hDll;
    
	InitializeMadCHook();
    InitializeCriticalSection(&showbubbleCriticalSection);
}

void CNxtOfficePEPModule::OnDllDetach()
{
	// all golden tags will synchronize here
    DeleteCriticalSection(&showbubbleCriticalSection);

	CNLObMgr& theObMgrIns = CNLObMgr::NLGetInstance();
	theObMgrIns.NLStartSynchronizeGoldenTags(true);

	FinalizeMadCHook();
}

//
// very ugly code, special for SE ,in OnAppConnect
//
void DoFakeEva()
{
	static bool s_bFakeEva = false;
	if (!s_bFakeEva)
	{
		nextlabs::Obligations obs;
		CNxtSDK::DoEvaluation(
			L"c:\\foo.txt", 
			kOA_OPEN, 
			obs, 
			NULL, 
			NULL, 
			CE_NOISE_LEVEL_APPLICATION);

		s_bFakeEva = true;
	}
}

HRESULT CNxtOfficePEPModule::OnAppConnect(LPDISPATCH Application)
{
	if (NULL == Application){
		return E_FAIL;
	}

	_configDeploy();


	app_ = Application;
	type_ = getType();
	ver_ = getVersion(Application);
	bconnected_ = true;


	// initialize the action module, save application IDispatch
	CNLAction& theActionIns = CNLAction::NLGetInstance(); theActionIns;


	// special for SE
		// curVer for BAE, make it comment
	//DoFakeEva();

	//add for send result to sce for screencapture
	boost::thread t1(&TalkWithSCE::StartServerThread, &TalkWithSCE::GetInstance());

	return S_OK;
}

void CNxtOfficePEPModule::_configDeploy()
{
	const PWCHAR fini = L"officepep.ini";

	if (hPepDll_ == NULL){
		return;
	}

	WCHAR path[MAX_PATH] = { 0 };
	if (0 == ::GetModuleFileNameW(hPepDll_, path, MAX_PATH)){
		return;
	}

	PWCHAR pfname = ::PathFindFileNameW(path);
	if (path == pfname){
		return;
	}
	//wcscpy(pfname, config);
	wcscpy_s(pfname, wcslen(fini)+1, fini);

	utils::CIniFile cfg(path);
	if (!cfg.FileExist()){
		// path is not exist
		return;
	}
	// get cgf vars from officepep.ini
	BOOL bOutputDebug = cfg.ReadBool(L"Debug", L"enableOutputDebug", FALSE);
	BOOL bInterceptOutEval = cfg.ReadBool(L"Debug", L"interceptOutEval", FALSE);
	g_bDebugMode_L1 = bOutputDebug;
	g_bDebugMode_L2 = bInterceptOutEval;

	return;
}