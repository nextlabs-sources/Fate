#include "stdafx.h"
#include "utils.h"
#include "NLOfficePEP_Comm.h"
#include "NLObMgr.h"		
#include "dllmain.h"
#include "NLAction.h"

////////////////////////////CELog2//////////////////////////////////////////////
// for CELog2 we should define: CELOG_CUR_FILE
#define CELOG_CUR_FILE static_cast<celog_filepathint_t>(CELOG_FILEPATH_OFFICEPEP_MIN + EMNLFILEINTEGER_NLACTION)
//////////////////////////////////////////////////////////////////////////

CNLAction::CNLAction(void):m_process(NULL)
{
	if (NULL == m_process)
	{
		m_process = new CNLProcess();
	}
}

CNLAction::~CNLAction(void)
{
	UnInitialize();
}

CNLAction& CNLAction::NLGetInstance()
{
	static CNLAction theInstance;
	return theInstance;
}

void CNLAction::Initialize()
{
	if (NULL == m_process)
	{
		m_process = new CNLProcess();
	}
}

void CNLAction::UnInitialize( )
{
	if ( NULL != m_process )
	{
		delete m_process;
		m_process = NULL;
	}
}

/*
* SEH, it doesn't handle the C++ object destructor. we can not use STUNLOFFICE_RESULT as the return value. So here we use a number to replace it.
*	NLCELOG_SEH_RETURN_VAL can not support my return value enum EMNLOFFICE_RESULT
*/
ActionResult CNLAction::NLSetEventForXMLRibbon(_Inout_ STUNLOFFICE_RIBBONEVENT* pribbonevent )
{
	__try
	{
		return NLSetEventCommon( (void*)pribbonevent, kFireByRibbonUI );
	}
	__except ( EXCEPTION_ACCESS_VIOLATION == GetExceptionCode() ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH )	// Warning: C6320: Exception-filter expression is the constant EXCEPTION_EXECUTE_HANDLE, might mask exceptions that were not intended to be handled 
	{
		NLPRINT_DEBUGLOG( L"!!!!!!Exceptions, when we do XML ribbon event \n" );
		return kRtException;
	}
}

ActionResult CNLAction::NLSetEventForCOMNotify( _Inout_ NotifyEvent* pnotifyevent )
{
	__try
	{
		return NLSetEventCommon((void*)pnotifyevent, kFireByComNotify );
	}
	__except ( EXCEPTION_ACCESS_VIOLATION == GetExceptionCode() ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH )
	{
		NLPRINT_DEBUGLOG( L"!!!!!!Exceptions, when we do COM notify \n" );
		return kRtException;
	}
}

ActionResult CNLAction::NLSetEventForHookEvent( _Inout_ HookEvent*	phookevent )
{
	__try
	{
		return NLSetEventCommon((void*)phookevent, kFireByHookAPI );
	}
	__except ( EXCEPTION_ACCESS_VIOLATION == GetExceptionCode() ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH )
	{
		NLPRINT_DEBUGLOG( L"!!!!!!Exceptions, when we do HOOK event \n" );
		return kRtException;
	}
}

ActionResult CNLAction::NLSetEventCommon(void* pEventStruct, const EventType& emEventTriggerPoint )
{
	NLCELOG_ENUM_ENTER( ActionResult )
	// check parameter
	if ( NULL == pEventStruct )
	{
		NLCELOG_ENUM_RETURN_VAL( kRtFailed )
	}

	// 1. release hook event
	// 2. set event
	ProcessResult result( kFSSuccess, kPSAllow,  kOA_Unknown, VARIANT_FALSE, __FUNCTIONW__, __LINE__ );
	switch ( emEventTriggerPoint )
	{
	case kFireByRibbonUI:
		{
			STUNLOFFICE_RIBBONEVENT* pStuOfficeRibbonEvent = (STUNLOFFICE_RIBBONEVENT*)pEventStruct;
			result = NLSetEventFromXMLRibbonButton( pStuOfficeRibbonEvent->rbEvent, pStuOfficeRibbonEvent->unOfficeEventExtraType );
			if ( kFSSuccess != result.kFuncStat || kPSDeny == result.kPolicyStat )
			{
				pStuOfficeRibbonEvent->bVariantCancel = VARIANT_TRUE;	// deny the event
			}
			else
			{
				pStuOfficeRibbonEvent->bVariantCancel = result.vbCancel;
			}
		}
		break;
	case kFireByComNotify:
		{
			bool bIsCloseEvent = false;
			NotifyEvent* pNotifyEvent = (NotifyEvent*)pEventStruct;
			switch ( pep::appType())
			{
			case kAppWord:				
				if (NULL != m_process)
				{
					result = m_process->NLProcessComNotifyWord(pNotifyEvent->pDoc, pNotifyEvent->eventflag.fword, pNotifyEvent->bVariantSaveAsUI);
				}
				bIsCloseEvent = (kWdDocBeforeClose == pNotifyEvent->eventflag.fword);
				break;

			case kAppExcel:				
				if (NULL != m_process)
				{
					result = m_process->NLProcessComNotifyExcel(pNotifyEvent->pDoc, pNotifyEvent->eventflag.fexcel, pNotifyEvent->bVariantSaveAsUI);
				}
				bIsCloseEvent = (emExcelClose == pNotifyEvent->eventflag.fexcel);
				break;

			case kAppPPT:				
				if (NULL != m_process)
				{
					result = m_process->NLProcessComNotifyPpt(pNotifyEvent->pDoc, pNotifyEvent->eventflag.fppt, pNotifyEvent->bVariantSaveAsUI);
				}
				bIsCloseEvent = (emPPTBeforeClose == pNotifyEvent->eventflag.fppt);
				break;

			default:
				break;
			}
		
			pNotifyEvent->bVariantCancel = result.vbCancel;
			NLPRINT_DEBUGLOG( L"variant cancel:[%d] \n", pNotifyEvent->bVariantCancel );
			if ( kFSSuccess != result.kFuncStat || kPSDeny == result.kPolicyStat )
			{
				// here if we process close event failed, we allow the file close
				// for other event if we process failed we deny it and prevent the default work follow.
				if ( bIsCloseEvent )
				{
					pNotifyEvent->bVariantCancel = VARIANT_FALSE;	// allow the file close
				} 
				else
				{
					pNotifyEvent->bVariantCancel = VARIANT_TRUE;	// deny
				}
			}
			NLPRINT_DEBUGLOG( L"variant cancel:[%d] \n", pNotifyEvent->bVariantCancel );
		}
		break;
	case kFireByHookAPI:
		{
			HookEvent* pHookEvent = (HookEvent*)pEventStruct;
			if ( NULL != m_process )
			{
				result = m_process->NLProcessHookEvent( *pHookEvent );
			}
		}
		break;
	default:
		break;
	}
	
	ActionResult emOfficeResult = kFSSuccess != result.kFuncStat ? kRtFailed : ( kPSDeny == result.kPolicyStat ? kRtPCDeny : kRtPCAllow );
	NLCELOG_ENUM_RETURN_VAL( emOfficeResult )
}

ProcessResult CNLAction::NLSetEventFromXMLRibbonButton( _In_ const RibbonEvent& emRibbonEvent, _In_ const UNNLOFFICE_EVENTEXTRAFLAG& unOfficeEventExtraType )
{
	ProcessResult stuResult( kFSSuccess, kPSAllow,  kOA_Unknown, VARIANT_FALSE, __FUNCTIONW__, __LINE__ );
	OfficeAction emAction = kOA_Unknown;
	STUNLOFFICE_EVENTSTATUS stuEventStatus;
	stuEventStatus.stuCurRibbonEvent.rbEvent = emRibbonEvent;
	stuEventStatus.stuCurRibbonEvent.unOfficeEventExtraType = unOfficeEventExtraType;
	
	NLPRINT_DEBUGLOG( L" ***************** The ribbon button event is:[%d], union:[%d] \n", emRibbonEvent, unOfficeEventExtraType.emExtraDesFileType );
	switch ( emRibbonEvent )
	{
	case 	kRibbonSave:
		{
			emAction = kOA_EDIT;
		}
		break;
	case	kRibbonSaveAs:
		{
			emAction = kOA_COPY;
		}
		break;
	case	kRibbonConvert:	// save as XPS or PDF
		{
			emAction = kOA_CONVERT;
		}
		break;	
	case	kRibbonSend:
	case	kRibbonPublish:
		{
			emAction = kOA_SEND;
		}
		break;
	case	kRibbonPasteContent:
		{
			emAction = kOA_PASTE;
		}
		break;
	case	kRibbonViewNewWindow:		// View->NewWindow, used for overlay
	case	kRibbonViewZoom:					// View->Zoom, used for overlay
		{
			// This is not a event, need to do others for overlay
#pragma chMSG( "This is zoom and new window event, old it works for overlay and may be it is no used for the new overlay\n" )
		}
		break;
	case kRibbonExcelDataTab:
	case kRibbonInsert:		// DataInsert/Insert: are convert action and convert action doesn't care destination path
		{
			emAction = kOA_INSERT;
		}
		break;
	case	kRibbonNextLabsRibbonRightProtected:	// for NextLabs define ribbon button: right protect 
		{
//#pragma chMSG( "This is nextlabs ribbon button click event" )
		}
		break;
	default:
		break;
	}
	
	if ( NULL != m_process )
	{
		stuResult = m_process->HandleFacet( emAction, kFireByRibbonUI, stuEventStatus );
	}
	return stuResult;
}


