#include "stdafx.h"
#include "utils.h"
#include "NLAction.h"
#include "NLOfficePEP_Comm.h"
#include "NLSecondaryThreadForPDFMaker.h"
#include "dllmain.h"
#include "NLObMgr.h"

#include "OfficeListener.h"

////////////////////////////CELog2//////////////////////////////////////////////
// for CELog2 we should define: CELOG_CUR_FILE
#define CELOG_CUR_FILE static_cast<celog_filepathint_t>(CELOG_FILEPATH_OFFICEPEP_MIN + EMNLFILEINTEGER_OFFICELISTENER)
//////////////////////////////////////////////////////////////////////////

//
// for office listner base
//
COfficeListener::COfficeListener()
{
	NLSECONDARYTHREADFORPDFMAKERINSTANCE.NLStartPDFMakerSecondaryThread();
}

COfficeListener::~COfficeListener(void)
{
	NLSECONDARYTHREADFORPDFMAKERINSTANCE.NLExitPDFMakerSecondaryThread();
}

//
// for word listener
//
CWordListener::CWordListener() : COfficeListener()
{

}

CWordListener::~CWordListener(void)
{

}

HRESULT CWordListener::OnInvoke(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pdispparams, 
	VARIANT* pvarResult, EXCEPINFO* pexcepinfo, UINT* puArgErr)
{
	// NLPRINT_DEBUGLOG(L" The Parameters are: dispidMember=%ld, riid=%p, lcid=%lu, pdispparams=%p, pvarResult=%p, pexcepinfo=%p, puArgErr=%u \n", dispidMember, &riid, lcid, pdispparams, pvarResult, pexcepinfo, puArgErr);

	NotifyEvent event((WordEvent)dispidMember, NULL, VARIANT_FALSE, VARIANT_FALSE);
	VARIANT_BOOL* pvalBoolCanvel = NULL;

	switch ((WordEvent)dispidMember)
	{
	case kWdStartup:	//void Startup();
		NLPRINT_DEBUGLOG(L"Invoke: the word com event is:[%s]:[0x%x] \n", L"kWdStartup", dispidMember);	
		break;

	case kWdNewDoc:		//void NewDocument([in] Document* Doc);
		NLPRINT_DEBUGLOG(L"Invoke: the word com event is:[%s]:[0x%x] \n", L"kWdNewDoc", dispidMember);
		event.pDoc = pdispparams->rgvarg[0].pdispVal;	
		break;

	case kWdDocChange:	//void DocumentChange();
		NLPRINT_DEBUGLOG(L"Invoke: the word com event is:[%s]:[0x%x] \n", L"kWdDocChange", dispidMember);
		break;

	case kWdDocOpen:	//void DocumentOpen([in] Document* Doc);	
		NLPRINT_DEBUGLOG(L"Invoke: the word com event is:[%s]:[0x%x] \n", L"kWdDocOpen", dispidMember);
		event.pDoc = pdispparams->rgvarg[0].pdispVal;

		break;

	case kWdDocBeforeClose:		// void DocumentBeforeClose([in] Document* Doc,[in, out] VARIANT_BOOL* Cancel);
		NLPRINT_DEBUGLOG(L"Invoke: the word com event is:[%s]:[0x%x] \n", L"kWdDocBeforeClose", dispidMember);
		event.pDoc = pdispparams->rgvarg[0].pdispVal;
		pvalBoolCanvel = pdispparams->rgvarg[1].pboolVal;	// variant cancel

		break;
	case kWdDocBeforeSave:		// void DocumentBeforeSave(	[in] Document* Doc,	[in] VARIANT_BOOL* SaveAsUI,[in, out] VARIANT_BOOL* Cancel);
		NLPRINT_DEBUGLOG(L"Invoke: the word com event is:[%s]:[0x%x] \n", L"kWdDocBeforeSave", dispidMember);
		event.pDoc = pdispparams->rgvarg[0].pdispVal;
		event.bVariantSaveAsUI = *(pdispparams->rgvarg[1].pboolVal);
		pvalBoolCanvel = pdispparams->rgvarg[2].pboolVal;	// variant cancel
		if (pvalBoolCanvel && *pvalBoolCanvel)
		{
			return S_OK;
		}
	
		break;
	case kWdProtectedViewWindowOpen:  	//void ProtectedViewWindowOpen([in] ProtectedViewWindow* PvWindow);
		NLPRINT_DEBUGLOG(L"Invoke: the word com event is:[%s]:[0x%x] \n", L"kWdProtectedViewWindowOpen", dispidMember);
		event.pDoc = pdispparams->rgvarg[0].pdispVal;
	
		break;

	case kWdProtectedViewWindowBeforeClose: //void ProtectedViewWindowBeforeClose([in] ProtectedViewWindow* PvWindow,[in] int CloseReason,[in, out] VARIANT_BOOL* Cancel);
		NLPRINT_DEBUGLOG(L"Invoke: the word com event is:[%s]:[0x%x] \n", L"emWordProtectedViewWindowBeforeClose", dispidMember);

		event.pDoc = pdispparams->rgvarg[0].pdispVal;
		pvalBoolCanvel = pdispparams->rgvarg[2].pboolVal;
	
		break;

	case kWdWndActivate: //void WindowActivate([in] Document* Doc,[in] Window* Wn);
		// cache the path in this event and used to API hook.!!!!!!!
		NLPRINT_DEBUGLOG(L"Invoke: the word com event is:[%s]:[0x%x] \n", L"kWdWndActivate", dispidMember);
		event.pDoc = pdispparams->rgvarg[0].pdispVal;

		break;
	default:
		NLPRINT_DEBUGLOG(L"Invoke: the word com event is [default]:[0x%x] \n", dispidMember);
		return S_OK;
	}

	CNLAction& theActionIns = CNLAction::NLGetInstance();
	theActionIns.NLSetEventForCOMNotify(&event);

	//NLPRINT_DEBUGLOG(L"variant cancel:[%d] \n", event.bVariantCancel);

	NULL == pvalBoolCanvel ? NULL : *pvalBoolCanvel = event.bVariantCancel;

	//NLPRINT_DEBUGLOG(L"variant cancel:[%d] \n", event.bVariantCancel);
	return S_OK;
}

bool CWordListener::NLSinkEvent(_In_ IDispatch* pApplication)
{
	return RegisterEventDispatch(pApplication) ? true : false;
}

bool CWordListener::NLUnSinkEvent()
{
	return UnregisterEventDispatch() ? true : false;
}





CExcelListener::CExcelListener() : COfficeListener()
{
	// for excel we need hook the keyboard and get ctrl+x/c
	HookKeyboardMsg();
}

CExcelListener::~CExcelListener(void)
{
	UnHookKeyboardMsg();
}

//////////////////////////////////////////////////////////////////////////
HRESULT CExcelListener::OnInvoke(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pvarResult, EXCEPINFO* pexcepinfo, UINT* puArgErr)
{
	NLPRINT_DEBUGLOG(L" The Parameters are: dispidMember=%ld, riid=%p, lcid=%lu, pDispParams=%p, pvarResult=%p, pexcepinfo=%p, puArgErr=%p \n", dispidMember, &riid, lcid, pDispParams, pvarResult, pexcepinfo, puArgErr);
	/*
	*	pdispparams->rgvarg[0].pdispVal: pDoc;
	* Before save:
	*		pdispparams->rgvarg[1].boolVal: bVariantSaveAsUI
	*		pdispparams->rgvarg[2].pboolVal: bVariantCancel, if this is VARANT_TRUE means deny the event
	*	Other:
	*		pdispparams->rgvarg[1].pboolVal: bVariantCancel
	*/

	NotifyEvent stuComNotifyEvent((ExcelEvent)dispidMember, NULL, VARIANT_FALSE, VARIANT_FALSE);
	VARIANT_BOOL* pvalBoolCanvel = NULL;

	if (NULL != pDispParams)
	{
		switch ((ExcelEvent)dispidMember)
		{
		case emExcelWindowsDeactivate:		
			NLPRINT_DEBUGLOG(L"excel event: emExcelWindowsDeactivate:[0x%x] \n", dispidMember);		
			break;

		case emProtectedViewOpen:		
			NLPRINT_DEBUGLOG(L"excel event: emProtectedViewOpen::[0x%x] \n", dispidMember);
			stuComNotifyEvent.pDoc = pDispParams->rgvarg[0].pdispVal;
		
			break;
		case emExcelProtectedViewWindowBeforeClose:		
			NLPRINT_DEBUGLOG(L"excel event: emExcelProtectedViewWindowBeforeClose::[0x%x] \n", dispidMember);
			stuComNotifyEvent.pDoc = pDispParams->rgvarg[0].pdispVal;
			pvalBoolCanvel = pDispParams->rgvarg[2].pboolVal;		
			break;

		case emExcelOpen:		
			NLPRINT_DEBUGLOG(L"excel event: emExcelOpen::[0x%x] \n", dispidMember);
			stuComNotifyEvent.pDoc = pDispParams->rgvarg[0].pdispVal;		
			break;
		case emExcelSave:
		{
			NLPRINT_DEBUGLOG(L"excel event: emExcelSave:[0x%x] \n", dispidMember);
			stuComNotifyEvent.pDoc = pDispParams->rgvarg[0].pdispVal;
			stuComNotifyEvent.bVariantSaveAsUI = pDispParams->rgvarg[1].boolVal;
			pvalBoolCanvel = pDispParams->rgvarg[2].pboolVal;
		}
			break;
		case emExcelPrint:
		{
			NLPRINT_DEBUGLOG(L"excel event: emExcelPrint:[0x%x], OnBeforePrint(pDispParams->rgvarg[0].pdispVal,pDispParams->rgvarg[1].pboolVal) \n", dispidMember);
			stuComNotifyEvent.pDoc = pDispParams->rgvarg[0].pdispVal;
			pvalBoolCanvel = pDispParams->rgvarg[1].pboolVal;
		}
			break;
		case emExcelClose:
		{
							 NLPRINT_DEBUGLOG(L"excel event: emExcelClose:[0x%x], DocumentClose(pDispParams->rgvarg[0].pdispVal,pDispParams->rgvarg[1].pboolVal) \n", dispidMember);

							 stuComNotifyEvent.pDoc = pDispParams->rgvarg[0].pdispVal;
							 pvalBoolCanvel = pDispParams->rgvarg[1].pboolVal;
		}
			break;
		case emExelWBActive:
			//#pragma chMSG( "What is this event means ? ...." )
			NLPRINT_DEBUGLOG(L"excel event: emExelWBActive:[0x%x] \n", dispidMember);
			break;
		case emExcelHyperLink:
			NLPRINT_DEBUGLOG(L"excel event: emExcelHyperLink:[0x%x] \n", dispidMember);		
			break;
		case emWindowsActive:		
			NLPRINT_DEBUGLOG(L"excel event: emWindowsActive:[0x%x] \n", dispidMember);		
			break;

		default:
			NLPRINT_DEBUGLOG(L"excel event: default:[0x%x] \n", dispidMember);
			return S_OK;
		}
	}

	CNLAction& theActionIns = CNLAction::NLGetInstance();
	theActionIns.NLSetEventForCOMNotify(&stuComNotifyEvent);

	NULL == pvalBoolCanvel ? NULL : *pvalBoolCanvel = stuComNotifyEvent.bVariantCancel;

	return S_OK;
}

bool CExcelListener::NLSinkEvent(_In_ IDispatch* pApplication)
{
	return RegisterEventDispatch(pApplication) ? true : false;
}

bool CExcelListener::NLUnSinkEvent()
{
	return UnregisterEventDispatch() ? true : false;
}

///////////////////////////////for hook key board///////////////////////////////////////////
void CExcelListener::HookKeyboardMsg()
{
	if (NULL == m_hHookKeyBoard)
	{
		m_hHookKeyBoard = SetWindowsHookExW(WH_KEYBOARD_LL, KeyboardProc, g_hInstance, 0);
	}
}

void CExcelListener::UnHookKeyboardMsg()
{
	if (NULL != m_hHookKeyBoard)
	{
		UnhookWindowsHookEx(m_hHookKeyBoard);
		m_hHookKeyBoard = NULL;
	}
}

HHOOK CExcelListener::m_hHookKeyBoard = NULL;
BOOL CExcelListener::m_sbNeedDeny = FALSE;
LRESULT CALLBACK CExcelListener::KeyboardProc(int code, WPARAM wParam, LPARAM lParam)
{
	PKBDLLHOOKSTRUCT  p = (PKBDLLHOOKSTRUCT)lParam;
	switch (wParam)
	{
	case WM_KEYUP:
	case WM_KEYDOWN:
	{
					   if (GetKeyState(VK_CONTROL) & 0x80000)
					   {
						   NLPRINT_DEBUGLOGEX(true, L" this is excel ctrl + operation: p->vkCode=[%d], C:[%d,%d], X:[%d,%d] \n", p->vkCode, 'C', 'c', 'X', 'x');

						   EMNLOFFICE_HOOKEVENT emOfficeHookEvent = emOfficeHookEventUnknown;
						   if ('C' == p->vkCode || 'c' == p->vkCode)
						   {
							   emOfficeHookEvent = emOfficeHookKeyBoardCtrlC;
						   }
						   else if ('X' == p->vkCode || 'x' == p->vkCode)
						   {
							   emOfficeHookEvent = emOfficeHookKeyBoardCtrlX;
						   }

						   if (emOfficeHookEventUnknown != emOfficeHookEvent)
						   {
							   // get the current excel process ID
							   static DWORD dwCurPID = 0;
							   if (0 == dwCurPID)
							   {
								   dwCurPID = GetCurrentProcessId();
							   }

							   // judge if user copy/cut the excel context
							   HWND hWnd = GetForegroundWindow();
							   if (NULL != hWnd)
							   {
								   DWORD dwProcessID = 0;
								   if (GetWindowThreadProcessId(hWnd, &dwProcessID))
								   {
									   CNLObMgr& theObMgrIns = CNLObMgr::NLGetInstance();

									   {
										   // just for debug
										   CComPtr<IDispatch> pDoc = getActiveDoc();
										   wstring wstrActFilePath;
										   getDocumentPath(wstrActFilePath, pDoc);
										   NLPRINT_DEBUGLOGEX(true, L"process ID:[%d], current active file path:[%s], file path from active doc:[%s], pDoc=[0x%x] \n", dwProcessID, theObMgrIns.NLGetCurActiveFilePath().c_str(), wstrActFilePath.c_str(), pDoc.p);
									   }

									   // For excel open in IE, if user select excel file and do ctrl+c/x we can get the open in IE flag, if user choose others we can get the real file path.
									   if (dwProcessID == dwCurPID || isOpenInIE())
									   {
										   NLPRINT_DEBUGLOGEX(true, L" this is excel ctrl+c/x operation and process [%s] PASTE action \n", theObMgrIns.NLGetCurActiveFilePath().c_str());

										   // open in IE or normal, user ctrl+x/c the excel context
										   HookEvent stuOfficeHookEvent(emOfficeHookEvent, emOfficeHookMethodByHookKeyBoard);
										   CNLAction& theActionIns = CNLAction::NLGetInstance();

										   if (WM_KEYDOWN == wParam)
										   {
											   BOOL bNeedDeny = (kRtPCDeny == theActionIns.NLSetEventForHookEvent(&stuOfficeHookEvent));
											   NLSetNeedDenyFlag(bNeedDeny);
											   if (bNeedDeny)
											   {
												   return code >= 0;
											   }
										   }
										   else if (WM_KEYUP == wParam)
										   {
											   if (NLGetNeedDenyFlag())
											   {
												   return code >= 0;
											   }
											   // revert the deny flag
											   NLSetNeedDenyFlag(FALSE);
										   }
									   }
								   }
							   }
						   }
					   }
	}
		break;
	case WM_SYSKEYDOWN:
	case WM_SYSKEYUP:
	default:
		break;
	}
	return CallNextHookEx(m_hHookKeyBoard, code, wParam, lParam);
}

void CExcelListener::NLSetNeedDenyFlag(_In_ BOOL bNeedDeny)
{
	InterlockedExchange(reinterpret_cast<LONG*>(&m_sbNeedDeny), static_cast<LONG>(bNeedDeny));
}

BOOL CExcelListener::NLGetNeedDenyFlag()
{
	return m_sbNeedDeny;
}
//////////////////////////////////////////////////////////////////////////



CPPTListener::CPPTListener() : COfficeListener()
{

}

CPPTListener::~CPPTListener(void)
{

}

//////////////////////////////////////////////////////////////////////////
HRESULT CPPTListener::OnInvoke(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pvarResult, EXCEPINFO* pexcepinfo, UINT* puArgErr)
{
	NLPRINT_DEBUGLOG(L" The Parameters are: dispidMember=%ld, riid=%p, lcid=%p, pDispParams=%p, pvarResult=%p, pexcepinfo=%p, puArgErr=%p \n", dispidMember, &riid, &lcid, pDispParams, pvarResult, pexcepinfo, puArgErr);
	/*
	*	pdispparams->rgvarg[0].pdispVal: pDoc;
	* Before save:
	*		pdispparams->rgvarg[1].boolVal: bVariantSaveAsUI
	*		pdispparams->rgvarg[2].pboolVal: bVariantCancel, if this is VARANT_TRUE means deny the event
	*	Other:
	*		pdispparams->rgvarg[1].pboolVal: bVariantCancel
	*/

	NotifyEvent stuComNotifyEvent((PPTEvent)dispidMember, NULL, VARIANT_FALSE, VARIANT_FALSE);
	VARIANT_BOOL* pvalBoolCanvel = NULL;

	HRESULT hr = S_OK;
	if (NULL != pDispParams && DISPATCH_METHOD == wFlags)
	{
		switch ((PPTEvent)dispidMember)
		{
		case emPPTAfterNewPresentation:
		{
										  NLPRINT_DEBUGLOG(L"PPT com event: emPPTAfterNewPresentation:[0x%x] \n", dispidMember);

										  stuComNotifyEvent.pDoc = pDispParams->rgvarg[0].pdispVal;
		}
			break;
		case emPPTProtectedViewWindowOpen:
		{
											 NLPRINT_DEBUGLOG(L"PPT com event: emPPTProtectedViewWindowOpen:[0x%x] \n", dispidMember);

											 stuComNotifyEvent.pDoc = pDispParams->rgvarg[0].pdispVal;
		}
			break;
		case emPPTAfterPresentationOpen:
		{
										   NLPRINT_DEBUGLOG(L"PPT com event: emPPTAfterPresentationOpen:[0x%x] \n", dispidMember);
										   stuComNotifyEvent.pDoc = pDispParams->rgvarg[0].pdispVal;
		}
			break;
		case emPPTPresentationOpen: // PresentationOpen, before open
		{
										NLPRINT_DEBUGLOG(L"PPT com event: emPPTPresentationOpen:[0x%x] \n", dispidMember);
										stuComNotifyEvent.pDoc = pDispParams->rgvarg[0].pdispVal;
		}
			break;
		case emPPTWinSelectionChange:
		{
										NLPRINT_DEBUGLOG(L"PPT com event: emPPTWinSelectionChange:[0x%x] \n", dispidMember);
		}
			break;
		case emPPTSlideShow:
		{
							   NLPRINT_DEBUGLOG(L"PPT com event: emPPTSlideShow:[0x%x] \n", dispidMember);

							   stuComNotifyEvent.pDoc = pDispParams->rgvarg[0].pdispVal;
		}
			break;
		case emPPTSlideShowNextClick:
		{
										NLPRINT_DEBUGLOG(L"PPT com event: emPPTSlideShowNextClick:[0x%x] \n", dispidMember);

										stuComNotifyEvent.pDoc = pDispParams->rgvarg[0].pdispVal;
		}
			break;
		case emPPTSave:
		{
						  NLPRINT_DEBUGLOG(L"PPT com event: emPPTSave:[0x%x] \n", dispidMember);

						  stuComNotifyEvent.pDoc = pDispParams->rgvarg[1].pdispVal;
						  pvalBoolCanvel = pDispParams->rgvarg[0].pboolVal;
		}
			break;
		case emPPTAferSaveAs:
		{
								NLPRINT_DEBUGLOG(L"PPT com event: emPPTAferSaveAs:[0x%x] \n", dispidMember);

								stuComNotifyEvent.pDoc = pDispParams->rgvarg[0].pdispVal;
		}
			break;
		case emPPTBeforeClose:
		{
								 NLPRINT_DEBUGLOG(L"PPT com event: emPPTBeforeClose:[0x%x] \n", dispidMember);

								 stuComNotifyEvent.pDoc = pDispParams->rgvarg[0].pdispVal;
		}
			break;
		case emPPTProtectedViewWindowBeforeClose:
		{
													NLPRINT_DEBUGLOG(L"PPT com event: emPPTBeforeClose:[0x%x] \n", dispidMember);

													stuComNotifyEvent.pDoc = pDispParams->rgvarg[2].pdispVal;
													pvalBoolCanvel = pDispParams->rgvarg[0].pboolVal;
		}
			break;
		case emPPTWindowActivate:
		{
									NLPRINT_DEBUGLOG(L"PPT com event: emPPTWindowActivate:[0x%x] \n", dispidMember);

									stuComNotifyEvent.pDoc = pDispParams->rgvarg[0].pdispVal;
		}
			break;
		case emPPTPrint:
		{
						   NLPRINT_DEBUGLOG(L"PPT com event: emPPTPrint:[0x%x] \n", dispidMember);

						   stuComNotifyEvent.pDoc = pDispParams->rgvarg[0].pdispVal;
		}
		default:
			NLPRINT_DEBUGLOG(L"PPT com event: default:[0x%x] \n", dispidMember);
			return hr;
		}

		CNLAction& theActionIns = CNLAction::NLGetInstance();
		theActionIns.NLSetEventForCOMNotify(&stuComNotifyEvent);

		NULL == pvalBoolCanvel ? NULL : *pvalBoolCanvel = stuComNotifyEvent.bVariantCancel;
	}
	else
	{
		NLPRINT_DEBUGLOG(L"PPT com event: hr = DISP_E_PARAMNOTFOUND \n");
		hr = DISP_E_PARAMNOTFOUND;
	}
	return hr;
}

bool CPPTListener::NLSinkEvent(_In_ IDispatch* pApplication)
{
	return RegisterEventDispatch(pApplication) ? true : false;
}
bool CPPTListener::NLUnSinkEvent()
{
	return UnregisterEventDispatch() ? true : false;
}
