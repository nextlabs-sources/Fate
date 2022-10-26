
#pragma once
// msoObj.cpp : Implementation of CmsoObj

#include "stdafx.h"
#include "msoObj.h"
#include "../outlook/outlookObj.h"
#include "../common/policy.h"
#include "../common/log.h"
#include "../outlook/ItemEventDisp.h"
#include <boost/algorithm/string.hpp>
#pragma warning(push)
#pragma warning(disable: 4267)
#include "nlexcept.h"
#pragma warning(pop)
#include "../outlook/outlookUtilities.h"
extern COutlookObj*    g_pOutlookObj;
extern HINSTANCE       g_hInstance;
extern _ext_AppType    g_eAppType;
extern void exception_cb( NLEXCEPT_CBINFO* cb_info );

extern std::wstring g_strOETempFolder;

// CmsoObj
STDMETHODIMP CmsoObj::OnConnection(LPDISPATCH Application, ext_ConnectMode ConnectMode, LPDISPATCH AddInInst, SAFEARRAY * * custom)
{
    DP((L"CmsoObj::OnConnection\n"));
    switch(g_eAppType)
    {
    case apptWord:
        return S_OK;
    case apptExcel:
        return S_OK;
    case apptPPT:
        return S_OK;
    case apptOutlook:
		if(PolicyCommunicator::InitSDK())
		{
			DP((L"msoObj::OnConnection! Init SDK okay!\n"));
			CEventLog::RegEventSource();

			return g_pOutlookObj->OnConnection(Application, ConnectMode, AddInInst, custom);
		}
		else
		{
			DP((L"msoObj::OnConnection! Fail to init SDK, won't perform the pep functionality!\n"));
			return S_OK;
		}
    default:
        return S_OK;
    }
}

STDMETHODIMP CmsoObj::OnStartupComplete(SAFEARRAY * * custom)
{
	UNREFERENCED_PARAMETER(custom); 
	DP((L"Enter OnStartupComplete.\n"));

	////clear OE load time history, to make sure that Outlook didn't disable OE for the performance reason.
	const wchar_t* szOEPerformanceKey =  NULL;
#if defined WSO2K13
	szOEPerformanceKey = L"Software\\Microsoft\\Office\\15.0\\Outlook\\Addins\\msoPEP.msoObj.1";
#elif defined WSO2K16
    szOEPerformanceKey = L"Software\\Microsoft\\Office\\16.0\\Outlook\\Addins\\msoPEP.msoObj.1";
#endif

	if(szOEPerformanceKey)
	{
		HKEY hKeyOEPerformance = NULL;
		long lResult = RegOpenKeyEx(HKEY_CURRENT_USER, szOEPerformanceKey, 0, KEY_ALL_ACCESS, &hKeyOEPerformance);
		if( (lResult== ERROR_SUCCESS) && (hKeyOEPerformance!=NULL) )
		{
			DP((L"Clear load time value.\n"));
			const BYTE pValue[6*4]= {0};
			RegSetValueEx(hKeyOEPerformance, L"1", 0, REG_BINARY, pValue, 6*4);

			RegCloseKey(hKeyOEPerformance);
			hKeyOEPerformance= NULL;
		}
	} 

	return	S_OK;
}

STDMETHODIMP CmsoObj::OnDisconnection(ext_DisconnectMode RemoveMode, SAFEARRAY * * custom)
{
	HRESULT hr = S_OK;

    switch(g_eAppType)
    {
    case apptWord:
        return S_OK;
    case apptExcel:
        return S_OK;
    case apptPPT:
        return S_OK;
    case apptOutlook:
		if(RemoveMode == ext_dm_UserClosed)
		{
			// reactive our add-in
			NLComAddIn& theIns = NLComAddIn::Instance();
			theIns.ReActiveAddIn();
		}
		else
		{
			hr = g_pOutlookObj->OnDisconnection(RemoveMode, custom);
			PolicyCommunicator::DeinitSDK();
			CEventLog::DelEventSource();
		}
		return hr;
    default:
        return S_OK;
    }
}

extern BOOL InsertInspEventDisp(HWND hWnd);
#define NLBUFFER_LENGTH 512
BOOL CALLBACK CmsoObj::EnumMailWinProc( HWND hwnd, LPARAM lParam )
{
	UNREFERENCED_PARAMETER(lParam);
	if(hwnd != NULL)
	{
		DWORD dwProcessId = GetCurrentProcessId();
		DWORD dwHwndProcId = 0;
		GetWindowThreadProcessId(hwnd, &dwHwndProcId);

		if (dwHwndProcId == dwProcessId)
		{
			wchar_t wszCaption[NLBUFFER_LENGTH] = {0};
			wchar_t wszClassName[NLBUFFER_LENGTH] = {0};
			::GetClassNameW( hwnd, wszClassName, NLBUFFER_LENGTH);

			if (wszClassName[0]!= L'\0' && 0 == _wcsicmp(DEST_CLASS_NAME, wszClassName))
			{
				::GetWindowTextW(hwnd, wszCaption, NLBUFFER_LENGTH);
				if (wszCaption[0]!= L'\0' && boost::algorithm::icontains(wszCaption, L" - Message"))
				{
					InsertInspEventDisp(hwnd);
					return FALSE;
				}
			}
		}
	}
	return TRUE;
}

STDMETHODIMP CmsoObj::Invoke(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pdispparams, VARIANT* pvarResult, EXCEPINFO* pexcepinfo, UINT* puArgErr) 
{
	HRESULT hr = S_OK;

	UNREFERENCED_PARAMETER(riid);
	UNREFERENCED_PARAMETER(lcid);
	UNREFERENCED_PARAMETER(wFlags);
	UNREFERENCED_PARAMETER(pvarResult); 
	UNREFERENCED_PARAMETER(pexcepinfo);
	UNREFERENCED_PARAMETER(puArgErr);
	//__try
	//{
		if	(pdispparams && DISPATCH_METHOD==wFlags)
		{
			switch	(dispidMember)
			{
			case 0xf002:    // item send pdispparams->rgvarg[1].pdispVal, pdispparams->rgvarg[0].pboolVal
				{
					hr =  g_pOutlookObj->OnItemSend(  pdispparams->rgvarg[1].pdispVal, pdispparams->rgvarg[0].pboolVal) ;
				}
				break;
 			case 0xf007:   //disconnect
				
 				if (g_pOutlookObj->IsMeetingRespondChangeTo0())
				{			
					OLUtilities::SetDeleteWhenRespond(1);
				}
 				break;
#if defined(WSO2K13) || defined(WSO2K16)
			case 0xf006: //startup for outlook2013 launched by 3rd party app 
				{
					std::wstring wstrCmd = GetCommandLineW();
					if (boost::algorithm::iends_with(wstrCmd, L"-Embedding") //Excel
						||boost::algorithm::icontains(wstrCmd, L"simplemapi"))//ppt, word
					{
						EnumWindows(EnumMailWinProc, NULL);
					}
				}
				break;
#endif
			default:
				hr = DISP_E_MEMBERNOTFOUND;
				break;
			}
		}
		else
		{
			hr = DISP_E_PARAMNOTFOUND;
		}
	//}
	//__except( NLEXCEPT_FILTER_EX2(NULL,exception_cb) )
	//{
	//	/* empty */
	//	;
	//}

	return	hr;
}

//////////////////////////////////////////////////////////////////////////

NLComAddIn::NLComAddIn(void)
{
	m_bLoadItself=false;
}

NLComAddIn::~NLComAddIn(void)
{
}


NLComAddIn& NLComAddIn::Instance()
{
	static NLComAddIn theNLComAddIn;
	return theNLComAddIn;
}

bool NLComAddIn::ActiveOEInHKURegEntry()
{
    HRESULT hr = S_OK;
    DWORD   dwDisposition = 0;
    HKEY    hKeyOutlook = NULL;
    HKEY    hKeyAddin = NULL;
    LONG    lResult = 0;
    DWORD   dwVal = 0;
    char    szVal[MAX_PATH];    memset(szVal, 0, sizeof(szVal));
    WCHAR   wzKeyName[MAX_PATH]; memset(wzKeyName, 0, sizeof(wzKeyName));

    try
    {
        _snwprintf_s(wzKeyName, MAX_PATH, _TRUNCATE, L"SOFTWARE\\Microsoft\\Office\\%s\\Addins", L"Outlook");
        lResult = RegOpenKeyEx(HKEY_CURRENT_USER, wzKeyName, 0, KEY_ALL_ACCESS, &hKeyOutlook);
        if (ERROR_SUCCESS != lResult)     // get office/outlook key
        {
            DP((L"RegisterDll::Fail to open key: %s\n", wzKeyName));
            hr = E_UNEXPECTED;
            throw;
        }
        lResult = RegCreateKeyEx(hKeyOutlook, L"msoPEP.msoObj.1", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKeyAddin, &dwDisposition);
        if (ERROR_SUCCESS != lResult)
        {
            DP((L"RegisterDll::Fail to open key: msoPEP.msoObj.1\n"));
            hr = E_UNEXPECTED;
            throw;
        }

        dwVal = 3;
        RegSetValueEx(hKeyAddin, L"LoadBehavior", 0, REG_DWORD, (const BYTE*)&dwVal, sizeof(DWORD));
    }
    catch (...)
    {
        if (NULL != hKeyAddin) RegCloseKey(hKeyAddin);
        if (NULL != hKeyOutlook) RegCloseKey(hKeyOutlook);

        hKeyAddin = NULL;
        hKeyOutlook = NULL;
        return false;
    }

    return true;
}

bool NLComAddIn::ReConnect(CComPtr<IDispatch> pObject)
{
    Sleep(1000);
    if (pObject == NULL || pObject.p == NULL)   return false;
    Outlook::_ApplicationPtr pOutlook(pObject.p);
    Office::COMAddInsPtr pAddIns = NULL;
	HRESULT hr = pOutlook->get_COMAddIns(&pAddIns);
    if (FAILED(hr)) return false;
    long lCount = 0;
    hr = pAddIns->get_Count(&lCount);
    if (FAILED(hr)) return false;
    for (long i = 1; i <= lCount; i++)
    {
		CComVariant varIndex(i);
        Office::COMAddInPtr pAddIn = NULL;
        hr = pAddIns->Item(&varIndex, &pAddIn);
        if (SUCCEEDED(hr))
        {
            CComBSTR bstrName;
            hr = pAddIn->get_Description(&bstrName);
            if (_wcsicmp(bstrName.m_str, L"Enforcer for Microsoft Outlook ") == 0)
            {
                for (int a = 0; a < 2; a++)
                {
                     hr = pAddIn->put_Connect(VARIANT_TRUE);
                    if (SUCCEEDED(hr))  return true;
                    Sleep(1000);
                }
                break;
            }
        }
    }
    return false;
}

bool NLComAddIn::IsItemContentChanged(CComPtr<IDispatch> pdisp)
{
    DISPPARAMS dispparams;
    UINT uArgErr;
    HRESULT hr = S_OK;
    CComVariant varDirty;
    dispparams.rgvarg = 0;
    dispparams.cArgs = 0;
    dispparams.cNamedArgs = 0;
    dispparams.rgdispidNamedArgs = NULL;
    hr = pdisp->Invoke(0xF024,
        IID_NULL,
        LOCALE_SYSTEM_DEFAULT,
        DISPATCH_METHOD | DISPATCH_PROPERTYGET,
        &dispparams,
        &varDirty,
        NULL,
        &uArgErr);
    return SUCCEEDED(hr) && varDirty.bVal;
}

void  NLComAddIn::ActiveAddIn( void* pArguments )
{
    if (!IsUserAnAdmin())
    {
        const wchar_t * pMsg = L"All emails will be saved and closed since you disabled Outlook Enforcer.";
        MessageBoxW(GetActiveWindow(), pMsg, L"Waining", MB_OK | MB_ICONERROR);
    }

	UNREFERENCED_PARAMETER(pArguments);

	//deal with cancel add-in
    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

	NLComAddIn& theIns = NLComAddIn::Instance();

	CComPtr<IDispatch> pAppObj = theIns.GetOEObject();
	if(pAppObj == NULL)	
	{
		::CoUninitialize();
		_endthread();
		return ;
	}

    bool bConnect = false;
    if(IsUserAnAdmin()) bConnect = ReConnect(pAppObj);
    if (!bConnect)
    {
		Sleep(500);
        theIns.QuitApp(pAppObj);
    }
	::CoUninitialize();
	_endthread();
	return ;
} 


CComPtr<IDispatch> NLComAddIn::GetOEObject()
{
	CLSID clsid;
	CComPtr<IDispatch>  pApplication = NULL;
	HRESULT hr = S_OK;
	hr = CLSIDFromProgID(L"Outlook.Application", &clsid);  
	if(FAILED(hr))	return NULL;
	CComPtr<IUnknown> pUnknown = NULL;
	hr = GetActiveObject(clsid,NULL,(IUnknown**)&pUnknown);
	if(SUCCEEDED(hr))   hr = pUnknown->QueryInterface(IID_IDispatch, (void**)&pApplication);
	return pApplication;
}

void NLComAddIn::QuitApp(CComPtr<IDispatch> pDisp)
{
	if(pDisp != NULL && pDisp.p != NULL)
	{
        _ApplicationPtr spApp(pDisp.p);
        _InspectorsPtr pInspectors;
        HRESULT hr = spApp->get_Inspectors(&pInspectors);
        long lCount = 0;
        hr = pInspectors->get_Count(&lCount);
        for (long l = 1; l <= lCount; l++)
        {
            _InspectorPtr pInspector;
            hr = pInspectors->Item(CComVariant(1), &pInspector);
            if (FAILED(hr)) continue;
            CComPtr<IDispatch> pDispItem = NULL;
            hr = pInspector->get_CurrentItem((IDispatch**)&pDispItem);
            if (FAILED(hr)) continue;
            CComQIPtr<_MailItem> pItem(pDispItem);
			hr = pItem->Save();
            hr = pItem->Close(olSave);
        }
        ActiveOEInHKURegEntry();
        hr = spApp->Quit();
        if (FAILED(hr))
        {
            TerminateProcess(GetCurrentProcess(), 0);
        }
	}
}

void NLComAddIn::ReActiveAddIn()
{
	// load it self
	if(!m_bLoadItself)
	{
		wchar_t szPath[MAX_PATH]={0};
		GetModuleFileNameW(g_hInstance,szPath,MAX_PATH);
		HMODULE hMod = LoadLibrary(szPath);
		if(hMod != NULL) m_bLoadItself = true;	
	}
	if(m_bLoadItself)
	{
		_beginthread(ActiveAddIn,NULL,NULL);
	}
}
