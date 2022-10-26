#include "StdAfx.h"
#include "OfficeShare.h"
#include "CollManager.h"
#include "CollDesktop.h"
#include "CollFrames.h"
#include "CollSimple.h"

/*
* ICollaborateManager inherited from IBaseObject
* So we needn't hook the IBaseObject's function,
* because they are attribute function
*/

INSTANCE_DEFINE( CHookedCollManager );

void CHookedCollManager::Hook( void* pManager )
{
    SubstituteOrgFuncWithNew( pManager, 13, (void*)New_get_Desktop);
    SubstituteOrgFuncWithNew( pManager, 14, (void*)New_get_Frames);
    SubstituteOrgFuncWithNew( pManager, 15, (void*)New_get_Simple);
    DoHook( pManager );
}
//////////////////////////////////////////////////////////////////////////
HRESULT __stdcall CHookedCollManager::New_get_Desktop (ICollaborateManager* pThis,struct ICollaborateDesktop * * varDesktop ) 
{
	HRESULT hr = E_NOTIMPL;
	if( LMEIsDisabled() == true )
	{
		Old_get_Desktop pFunc = (Old_get_Desktop)(GetInstance()->GetOrgFunc( (void*)pThis, New_get_Desktop ));
		if( pFunc )
		{
			return   pFunc( pThis, varDesktop );
		}

	}
	__try
	{
		return   my_get_Desktop( pThis, varDesktop );
	}
	__except(NLEXCEPT_FILTER_EX2(NULL,exception_cb))
	{
		;
	}
	return hr;
}
HRESULT __stdcall CHookedCollManager::my_get_Desktop (ICollaborateManager* pThis,struct ICollaborateDesktop * * varDesktop ) 
{	
	//nextlabs::recursion_control_auto auto_disable(hook_control);
	HRESULT hr = E_NOINTERFACE;
    Old_get_Desktop pFunc = (Old_get_Desktop)(GetInstance()->GetOrgFunc( (void*)pThis, New_get_Desktop ));
    if( !pFunc )
    {
        GetInstance()->Hook( pThis );
        pFunc = (Old_get_Desktop)(GetInstance()->GetOrgFunc( (void*)pThis, New_get_Desktop ));
    }

    if( pFunc )
    {
        hr = pFunc( pThis, varDesktop );
    }

	//////////////////////////////////////////////////////////////////////////
	if(SUCCEEDED(hr) && (*varDesktop) != NULL)
	{
		ICollaborateDesktop* pDesktop = (*varDesktop);
#if 0
		BSTR strType =NULL;
		pDesktop->get_TypeOf(&strType);
		if(SUCCEEDED(hr))
		{
			sprintf_s(strInfo,sizeof(strInfo),"Desktop %p 's type is:%ws !\r\n",pDesktop,strType);
			OutPutLog(strInfo);
		}
		ICollaborateWindows* pWindows=NULL;
		hr = pDesktop->get_Windows(&pWindows);
		pWindows->get_TypeOf(&strType);
		sprintf_s(strInfo,sizeof(strInfo),"Get Desktop here we get windows name:%ws!\r\n",strType);
		OutPutLog(strInfo);
		if(SUCCEEDED(hr))
		{
			long nCount = 0;
			hr = pWindows->get_Count(&nCount);
			if(SUCCEEDED(hr))
			{
				sprintf_s(strInfo,sizeof(strInfo),"Get Desktop here we get windows count:%d!\r\n",nCount);
				OutPutLog(strInfo);
			}
			IBaseObject* pObject = NULL;
			for(int i=0;i<nCount;i++)
			{
				hr = pWindows->get_Item(i,&pObject);
				if(SUCCEEDED(hr) && (pObject != NULL))
				{
					hr = pObject->get_TypeOf(&strType);
					sprintf_s(strInfo,sizeof(strInfo),"Get Desktop here we get windows item's name:%ws!\r\n",strType);
					OutPutLog(strInfo);
				}
			}
		}
		IBaseObject* pApp=NULL;
		hr = pDesktop->get_CollaborateApp(&pApp);
		if(SUCCEEDED(hr))
		{
			strType = NULL;
			hr =pApp->get_TypeOf(&strType);
			if(SUCCEEDED(hr))
			{
				sprintf_s(strInfo,sizeof(strInfo),"Get Desktop here we get app's name:%ws!\r\n",strType);
				OutPutLog(strInfo);
			}
		}
#endif 
		//HookDesktop(pDesktop);
        CHookedCollDesktop::GetInstance()->Hook( PVOID(pDesktop) );
		//hr = S_OK;
	}
	//////////////////////////////////////////////////////////////////////////

	return hr;
}
HRESULT __stdcall CHookedCollManager::New_get_Frames (ICollaborateManager* pThis,/*[out,retval]*/ struct ICollaborateFrames * * varFrames )
{
	HRESULT hr = E_NOTIMPL;
	if( LMEIsDisabled() == true )
	{
		Old_get_Frames pFunc = (Old_get_Frames)(GetInstance()->GetOrgFunc( (void*)pThis, New_get_Frames ));
		if( pFunc )
		{
			return   pFunc( pThis, varFrames );
		}

	}
	__try
	{
		return   my_get_Frames( pThis, varFrames );
	}
	__except(NLEXCEPT_FILTER_EX2(NULL,exception_cb))
	{
		;
	}
	return hr;
}
HRESULT __stdcall CHookedCollManager::my_get_Frames (ICollaborateManager* pThis,/*[out,retval]*/ struct ICollaborateFrames * * varFrames )
{	
	//nextlabs::recursion_control_auto auto_disable(hook_control);
	HRESULT hr = E_NOINTERFACE;
    Old_get_Frames pFunc = (Old_get_Frames)(GetInstance()->GetOrgFunc( (void*)pThis, New_get_Frames ));
    if( !pFunc )
    {
        GetInstance()->Hook( pThis );
        pFunc = (Old_get_Frames)(GetInstance()->GetOrgFunc( (void*)pThis, New_get_Frames ));
    }

    if( pFunc )
    {
        hr = pFunc( pThis, varFrames );
    }

	if(SUCCEEDED(hr) && (*varFrames) != NULL)
	{
		ICollaborateFrames* pFrames = (*varFrames);
		BSTR pType = NULL;
		hr = pFrames->get_TypeOf(&pType);
		
		long ncount = 0;
		hr = pFrames->get_Count(&ncount);
		
        CHookedCollFrames::GetInstance()->Hook( PVOID(pFrames) );
		hr = S_OK;
	}

	return hr;
}

HRESULT __stdcall CHookedCollManager::New_get_Simple (
								  ICollaborateManager* pThis,
								  struct ICollaborateSimple * * varSimple
								  ) 
{
	HRESULT hr = E_NOTIMPL;
	if( LMEIsDisabled() == true )
	{
		Old_get_Simple pFunc = (Old_get_Simple)(GetInstance()->GetOrgFunc( (void*)pThis, New_get_Simple ));
		if( pFunc )
		{
			return   pFunc( pThis, varSimple );
		}

	}
	__try
	{
		return   my_get_Simple( pThis, varSimple );
	}
	__except(NLEXCEPT_FILTER_EX2(NULL,exception_cb))
	{
		;
	}
	return hr;
}
HRESULT __stdcall CHookedCollManager::my_get_Simple (
	ICollaborateManager* pThis,
struct ICollaborateSimple * * varSimple
	) 
{	
	//nextlabs::recursion_control_auto auto_disable(hook_control);
	HRESULT hr = E_NOINTERFACE;
    Old_get_Simple pFunc = (Old_get_Simple)(GetInstance()->GetOrgFunc( (void*)pThis, New_get_Simple ));
    if( !pFunc )
    {
        GetInstance()->Hook( pThis );
        pFunc = (Old_get_Simple)(GetInstance()->GetOrgFunc( (void*)pThis, New_get_Simple ));
    }

    if( pFunc )
    {
        hr = pFunc( pThis, varSimple );
    }

	//////////////////////////////////////////////////////////////////////////
	// here we trace whether can we get some information from ICollaborateSimple object
	if(SUCCEEDED(hr) && (*varSimple) != NULL)
	{
		ICollaborateSimple* pSample = (*varSimple);

		BSTR pType = NULL;
		hr = pSample->get_TypeOf(&pType);
		
		ULONG nProcessId=0;
		hr = pSample->get_ProcessId(&nProcessId);

		LMC_Coll::ShareStateType varShare;
		hr = pSample->get_Share(&varShare);

		VARIANT_BOOL bAnnotate=VARIANT_TRUE;
		hr = pSample->get_Annotate(&bAnnotate);

		ULONG lAnnotatePen=0;
		hr = pSample->get_AnnotatePen(&lAnnotatePen);

		ULONG lAnnotateColor=0;
		hr = pSample->get_AnnotateColor(&lAnnotateColor);

		LMC_Coll::DesktopColor varColorQuality;
		hr = pSample->get_ColorQuality(&varColorQuality);

		long lPrimaryWindow=0;
		hr = pSample->get_PrimaryWindow(&lPrimaryWindow);

		BSTR bstrTitle=NULL;
		hr = pSample->get_PrimaryWindowTitle(&bstrTitle);


		VARIANT_BOOL varControlRequestAllowed=VARIANT_TRUE;
		hr = pSample->get_ControlRequestAllowed(&varControlRequestAllowed);

		long lLeft=0,lTop=0,lRight=0,lBottom=0;
		hr = pSample->get_PrimaryWindowPosition(&lLeft,&lTop,&lRight,&lBottom);

        CHookedCollSimple::GetInstance()->Hook( PVOID(pSample) );

		hr = S_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	return hr;
}
//////////////////////////////////////////////////////////////////////////