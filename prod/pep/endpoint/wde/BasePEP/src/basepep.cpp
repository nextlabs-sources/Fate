#include <iostream>
#include "basepep.h"
#include <boost\algorithm\string.hpp>

#include "shellexplorercontext.h"
#include "cbcontext.h"
#include "genericcontext.h"
#include "iecontext.h"
#include "disablescreencapturecontext.h"
#include "dllhostcontextt.h"
#include "gsexplorercontext.h"
#include "chromecontextt.h"

extern nextlabs::CRuntimeContext* gContext_;
extern HANDLE gSharedFileMutex;
extern LPCTSTR SHARED_MUTEX_NAME;
namespace nextlabs
{

BOOL CBasePep::InitContext()
{
    // build Context by building-policies
    if (!gContext_)
    {
        if (boost::iequals(L"explorer.exe", exe_.Name())){
            gContext_ = new CShellExploerContext;
            //OutputDebugStringW(L"create CShellExploerContext instance");
        }
		else if (boost::iequals(L"AcroRd32.exe", exe_.Name()) || boost::iequals(L"Acrobat.exe", exe_.Name())
			     || boost::iequals(L"EXCEL.EXE", exe_.Name()) || boost::iequals(L"WINWORD.EXE", exe_.Name()) || boost::iequals(L"POWERPNT.EXE", exe_.Name()))
		{
			gContext_ = new CCBContext;
			//OutputDebugStringW(L"create CCBContext instance");
		}
		else if(boost::iequals(L"dwm.exe", exe_.Name()) || boost::iequals(L"nlsce.exe", exe_.Name()))
		{
			gContext_ = new CDisableScreenCaptureContext;
		}
        else if(boost::iequals(L"iexplore.exe", exe_.Name()))
        {
            gContext_ = new CIEContext;
            gSharedFileMutex = CreateMutex(NULL, FALSE, SHARED_MUTEX_NAME);
            if (gSharedFileMutex)
            {
                priorityHelper::SetObjectToLowIntegrity(gSharedFileMutex);
            }
        }
		else if(boost::iequals(L"dllhost.exe", exe_.Name()))
		{
			gContext_ = new CDllHostContext;
		}
		else if(boost::iequals(L"gsexplorer.exe", exe_.Name()))
		{
			gContext_ = new CGSExplorerContext;
		}
		else if(boost::iequals(L"chrome.exe", exe_.Name()) ||
			boost::iequals(L"firefox.exe", exe_.Name()))
		{
			gContext_ = new CChromeContext;
		}
        else
        {
            gContext_ = new CGenericContext;
            //OutputDebugStringW(L"create CGenericContext instance");
        }

        gContext_->Init();
    }

	pContext_ = gContext_;
	
	return pContext_? TRUE: FALSE;

}

void CBasePep::DeinitContext()
{
	if(pContext_){
		pContext_->Deinit();
		delete pContext_;
		pContext_ = NULL;
	}
}

BOOL CBasePep::OnProcessAttach()
{
	CDllModule::OnProcessAttach();

	// the filter policy may be INI file , or entry from registry
	if(_FilterPolicyCheck())
	{
		return FALSE; // do not instantiate CBasePep
	}
	// init context;
	BOOL rtContext = InitContext();

	return rtContext;
}

BOOL CBasePep::OnProcessDetach()
{
	DeinitContext();
	return TRUE;
}

BOOL CBasePep::_FilterPolicyCheck()
{
    bool bIs_ignored = nextlabs::policy_monitor_application::is_ignored();
    if (bIs_ignored)
    {
        //OutputDebugStringW(L"ignored by policy");
        return TRUE;
    }
	
    return FALSE;
}

}  // ns nextlabs
