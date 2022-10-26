#pragma once
//#include "stdafx.h"
#include <list>
#include "ItemEventDisp.h"
#include "../outlook/outlookObj.h"

extern COutlookObj* g_pOutlookObj;

class MailItemUtility
{
public:
	static HRESULT QueryItemInterface(CComPtr<IDispatch> lpDisp,void **ppv)
	{
		HRESULT hr =S_FALSE ;
		if( lpDisp != NULL )
		{
			
			hr = lpDisp->QueryInterface(Outlook::IID__MailItem,ppv);
			if(SUCCEEDED(hr) && ppv)
			{
				DPW((L"IID__MailItem")) ;
				return hr ;
			}
			hr = lpDisp->QueryInterface(Outlook::IID__AppointmentItem,ppv);
			if(SUCCEEDED(hr) && ppv)
			{
				DPW((L"IID__AppointmentItem")) ;
				return hr ;
			}
			hr = lpDisp->QueryInterface(Outlook::IID__TaskItem, ppv);
			if(SUCCEEDED(hr) && ppv)
			{
				DPW((L"IID__TaskItem")) ;
				return hr ;
			}

			hr = lpDisp->QueryInterface(Outlook::IID__TaskRequestItem, ppv);
			if(SUCCEEDED(hr) && ppv)
			{
				DPW((L"IID__TaskRequestItem")) ;
				return hr ;
			}
			hr = lpDisp->QueryInterface(Outlook::IID__MeetingItem, ppv);
			if(SUCCEEDED(hr) && ppv)
			{
				DPW((L"IID__MeetingRequestItem")) ;
				return hr ;
			}
 #if ((defined WSO2K16) || (defined WSO2K13) || (defined WSO2K10))
			//bug 42622
			hr = lpDisp->QueryInterface(Outlook::IID__SharingItem, ppv);
			if(SUCCEEDED(hr) && ppv)
			{
				return hr ;
			}
#endif
		}
		return hr ;
	}
	static HRESULT get_SenderEmailType (CComPtr<IDispatch> lpDisp, BSTR * SenderEmailType )
	{
		HRESULT hr =S_FALSE ;
		if( lpDisp != NULL )
		{
			CComPtr<Outlook::_MailItem> spCurMailItem = 0;
			hr = lpDisp->QueryInterface(Outlook::IID__MailItem, (void**)&spCurMailItem);
			if(SUCCEEDED(hr) && spCurMailItem)
			{
				spCurMailItem->get_SenderEmailType( SenderEmailType ) ;
				return hr ;
			}
			CComPtr<Outlook::_MeetingItem> spCurMeetItem = 0;
			hr = lpDisp->QueryInterface(Outlook::IID__MeetingItem, (void**)&spCurMeetItem);
			if(SUCCEEDED(hr) && spCurMeetItem)
			{
				spCurMeetItem->get_SenderEmailType( SenderEmailType ) ;
				return hr ;
			}
			/*	CComPtr<Outlook::_AppointmentItem> spCurAppItem = 0;
			DPW((L"---------***************enter*********-------------------------")) ;
			hr = lpDisp->QueryInterface(Outlook::IID__AppointmentItem, (void**)&spCurAppItem);
			if(SUCCEEDED(hr) && spCurAppItem)
			{

			DPW((L"---------************************-------------------------")) ;
			hr = spCurAppItem->get_SenderEmailType( SenderEmailType ) ;

			return hr ;
			}*/
			//CComPtr<Outlook::_TaskItem> spCurTaskItem = 0;
			//hr = lpDisp->QueryInterface(Outlook::IID__TaskItem, (void**)&spCurTaskItem);
			//if(SUCCEEDED(hr) && spCurTaskItem)
			//{
			//	hr = spCurTaskItem->get_SenderEmailType( SenderEmailType ) ;

			//	return hr ;
			//}
			/*	CComPtr<Outlook::_TaskRequestItem> spCurTaskReqItem = 0;
			hr = lpDisp->QueryInterface(Outlook::IID__TaskRequestItem, (void**)&spCurTaskReqItem);
			if(SUCCEEDED(hr) && spCurTaskReqItem)
			{
			hr =spCurTaskReqItem->get_SenderEmailType( SenderEmailType ) ;

			return hr ;
			}*/
#if ((defined WSO2K16) || (defined WSO2K13) || (defined WSO2K10))
//bug 42622
//#ifdef WSO2K7
			CComPtr<Outlook::_SharingItem> spCurShareItem = 0;
			hr = lpDisp->QueryInterface(Outlook::IID__SharingItem, (void**)&spCurShareItem);
			if(SUCCEEDED(hr) && spCurShareItem)
			{
				hr = spCurShareItem->get_SenderEmailType( SenderEmailType ) ;

				return hr ;
			}
#endif
		}
		return hr ;
	}
	static HRESULT get_SenderEmailAddress(CComPtr<IDispatch> lpDisp, BSTR * SenderEmailAddress )
	{
		HRESULT hr =S_FALSE ;
		if( lpDisp != NULL )
		{
			CComPtr<Outlook::_MailItem> spCurMailItem = 0;
			hr = lpDisp->QueryInterface(Outlook::IID__MailItem, (void**)&spCurMailItem);
			if(SUCCEEDED(hr) && spCurMailItem)
			{
				hr =spCurMailItem->get_SenderEmailAddress( SenderEmailAddress ) ;
				return hr ;
			}
			CComPtr<Outlook::_MeetingItem> spCurMeetItem = 0;
			hr = lpDisp->QueryInterface(Outlook::IID__MeetingItem, (void**)&spCurMeetItem);
			if(SUCCEEDED(hr) && spCurMeetItem)
			{
				hr =spCurMeetItem->get_SenderEmailAddress( SenderEmailAddress ) ;
				return hr ;
			}
			/*	DPW((L"11111111111111111enter11111111111111111111111")) ;
			CComPtr<Outlook::_MeetingItem> spCurAppItem = 0;
			hr = lpDisp->QueryInterface(Outlook::IID__MeetingItem, (void**)&spCurAppItem);
			if(SUCCEEDED(hr) && spCurAppItem)
			{
			DPW((L"1111111111111111111111111111111111111111")) ;
			hr = spCurAppItem->get_SenderEmailAddress( SenderEmailAddress ) ;
			return hr ;
			}*/
			/*CComPtr<Outlook::_TaskItem> spCurTaskItem = 0;
			hr = lpDisp->QueryInterface(Outlook::IID__TaskItem, (void**)&spCurTaskItem);
			if(SUCCEEDED(hr) && spCurTaskItem)
			{
			hr = spCurTaskItem->get_SenderEmailAddress( SenderEmailAddress ) ;
			return hr ;
			}*/
			/*	CComPtr<Outlook::_TaskRequestItem> spCurTaskReqItem = 0;
			hr = lpDisp->QueryInterface(Outlook::IID__TaskRequestItem, (void**)&spCurTaskReqItem);
			if(SUCCEEDED(hr) && spCurTaskReqItem)
			{
			hr =spCurTaskReqItem->get_SenderEmailAddress( SenderEmailAddress ) ;

			return hr ;
			}*/
#if ((defined WSO2K16) || (defined WSO2K13) || (defined WSO2K10))
//add sharing item, bug 42622
//#ifdef WSO2K7
			CComPtr<Outlook::_SharingItem> spCurShareItem = 0;
			hr = lpDisp->QueryInterface(Outlook::IID__SharingItem, (void**)&spCurShareItem);
			if(SUCCEEDED(hr) && spCurShareItem)
			{
				hr = spCurShareItem->get_SenderEmailAddress( SenderEmailAddress ) ;
				return hr ;
			}
#endif
		}
		return hr ;
	}
	static HRESULT get_Session(CComPtr<IDispatch> lpDisp,struct _NameSpace * * Session  )
	{
		HRESULT hr =S_FALSE ;
		if( lpDisp != NULL )
		{
			CComPtr<Outlook::_MailItem> spCurMailItem = 0;
			hr = lpDisp->QueryInterface(Outlook::IID__MailItem, (void**)&spCurMailItem);
			if(SUCCEEDED(hr) && spCurMailItem)
			{
				spCurMailItem->get_Session( Session ) ;
				return hr ;
			}
			CComPtr<Outlook::_AppointmentItem> spCurAppItem = 0;
			hr = lpDisp->QueryInterface(Outlook::IID__AppointmentItem, (void**)&spCurAppItem);
			if(SUCCEEDED(hr) && spCurAppItem)
			{
				hr = spCurAppItem->get_Session( Session ) ;
				return hr ;
			}
			CComPtr<Outlook::_TaskItem> spCurTaskItem = 0;
			hr = lpDisp->QueryInterface(Outlook::IID__TaskItem, (void**)&spCurTaskItem);
			if(SUCCEEDED(hr) && spCurTaskItem)
			{
				hr = spCurTaskItem->get_Session( Session ) ;
				return hr ;
			}
			CComPtr<Outlook::_TaskRequestItem> spCurTaskReqItem = 0;
			hr = lpDisp->QueryInterface(Outlook::IID__TaskRequestItem, (void**)&spCurTaskReqItem);
			if(SUCCEEDED(hr) && spCurTaskReqItem)
			{
				hr = spCurTaskReqItem->get_Session( Session ) ;

				return hr ;
			}
			CComPtr<Outlook::_MeetingItem> spCurMeetItem = 0;
			hr = lpDisp->QueryInterface(Outlook::IID__MeetingItem, (void**)&spCurMeetItem);
			if(SUCCEEDED(hr) && spCurMeetItem)
			{
					hr = spCurMeetItem->get_Session( Session ) ;

				return hr ;
			}
#if ((defined WSO2K16) || (defined WSO2K13) || (defined WSO2K10))
//add sharing item, bug 42622
//#ifdef WSO2K7
			CComPtr<Outlook::_SharingItem> spCurShareItem = 0;
			hr = lpDisp->QueryInterface(Outlook::IID__SharingItem, (void**)&spCurShareItem);
			if(SUCCEEDED(hr) && spCurShareItem)
			{
				hr = spCurShareItem->get_Session( Session ) ;
				return hr ;
			}
#endif
		}
		return hr ;
	}
	static HRESULT get_MAPIOBJECT(CComPtr<IDispatch> lpDisp, IUnknown * * MAPIOBJECT )
	{
		HRESULT hr =S_FALSE ;
		if( lpDisp != NULL )
		{
			CComPtr<Outlook::_MailItem> spCurMailItem = 0;
			hr = lpDisp->QueryInterface(Outlook::IID__MailItem, (void**)&spCurMailItem);
			if(SUCCEEDED(hr) && spCurMailItem)
			{
				spCurMailItem->get_MAPIOBJECT( MAPIOBJECT ) ;
				return hr ;
			}
			CComPtr<Outlook::_AppointmentItem> spCurAppItem = 0;
			hr = lpDisp->QueryInterface(Outlook::IID__AppointmentItem, (void**)&spCurAppItem);
			if(SUCCEEDED(hr) && spCurAppItem)
			{
				hr = spCurAppItem->get_MAPIOBJECT( MAPIOBJECT ) ;
				return hr ;
			}
			CComPtr<Outlook::_TaskItem> spCurTaskItem = 0;
			hr = lpDisp->QueryInterface(Outlook::IID__TaskItem, (void**)&spCurTaskItem);
			if(SUCCEEDED(hr) && spCurTaskItem)
			{
				hr = spCurTaskItem->get_MAPIOBJECT( MAPIOBJECT ) ;
				return hr ;
			}
			CComPtr<Outlook::_TaskRequestItem> spCurTaskReqItem = 0;
			hr = lpDisp->QueryInterface(Outlook::IID__TaskRequestItem, (void**)&spCurTaskReqItem);
			if(SUCCEEDED(hr) && spCurTaskReqItem)
			{
				hr = spCurTaskReqItem->get_MAPIOBJECT( MAPIOBJECT ) ;

				return hr ;
			}
			CComPtr<Outlook::_MeetingItem> spCurMeetItem = 0;
			hr = lpDisp->QueryInterface(Outlook::IID__MeetingItem, (void**)&spCurMeetItem);
			if(SUCCEEDED(hr) && spCurMeetItem)
			{
					hr = spCurMeetItem->get_MAPIOBJECT( MAPIOBJECT ) ;

				return hr ;
			}
#if ((defined WSO2K16) || (defined WSO2K13) || (defined WSO2K10))
//add sharing item, bug 42622
//#ifdef WSO2K7
			CComPtr<Outlook::_SharingItem> spCurShareItem = 0;
			hr = lpDisp->QueryInterface(Outlook::IID__SharingItem, (void**)&spCurShareItem);
			if(SUCCEEDED(hr) && spCurShareItem)
			{
				hr = spCurShareItem->get_MAPIOBJECT( MAPIOBJECT ) ;
				return hr ;
			}
#endif
		}
		return hr ;
	}
	static HRESULT get_SenderName(CComPtr<IDispatch> lpDisp, BSTR * SenderName )
	{
		HRESULT hr =S_FALSE ;
		if( lpDisp != NULL )
		{
			CComPtr<Outlook::_MailItem> spCurMailItem = 0;
			hr = lpDisp->QueryInterface(Outlook::IID__MailItem, (void**)&spCurMailItem);
			if(SUCCEEDED(hr) && spCurMailItem)
			{
				hr = spCurMailItem->get_SenderName( SenderName ) ;
				return hr ;
			}
			CComPtr<Outlook::_MeetingItem> spCurMeetItem = 0;
			hr = lpDisp->QueryInterface(Outlook::IID__MeetingItem, (void**)&spCurMeetItem);
			if(SUCCEEDED(hr) && spCurMeetItem)
			{
					hr = spCurMeetItem->get_SenderName( SenderName ) ;
				return hr ;
			}
			//CComPtr<Outlook::_AppointmentItem> spCurAppItem = 0;
			//hr = lpDisp->QueryInterface(Outlook::IID__AppointmentItem, (void**)&spCurAppItem);
			//if(SUCCEEDED(hr) && spCurAppItem)
			//{
			//	hr = spCurAppItem->get_SenderName( SenderName ) ;

			//	return hr ;
			//}
			//CComPtr<Outlook::_TaskItem> spCurTaskItem = 0;
			//hr = lpDisp->QueryInterface(Outlook::IID__TaskItem, (void**)&spCurTaskItem);
			//if(SUCCEEDED(hr) && spCurTaskItem)
			//{
			//	hr = spCurTaskItem->get_SenderName( SenderName ) ;

			//	return hr ;
			//}
			/*	CComPtr<Outlook::_TaskRequestItem> spCurTaskReqItem = 0;
			hr = lpDisp->QueryInterface(Outlook::IID__TaskRequestItem, (void**)&spCurTaskReqItem);
			if(SUCCEEDED(hr) && spCurTaskReqItem)
			{
			hr=spCurTaskReqItem->get_SenderName( SenderName ) ;

			return hr ;
			}*/
#if ((defined WSO2K16) || (defined WSO2K13) || (defined WSO2K10))
//add sharing item, bug 42622
//#ifdef WSO2K7
			CComPtr<Outlook::_SharingItem> spCurShareItem = 0;
			hr = lpDisp->QueryInterface(Outlook::IID__SharingItem, (void**)&spCurShareItem);
			if(SUCCEEDED(hr) && spCurShareItem)
			{
				hr = spCurShareItem->get_SenderName( SenderName ) ;

				return hr ;
			}
#endif
		}
		return hr ;
	}
	//	static HRESULT get_Subject(CComPtr<IDispatch> lpDisp, BSTR * Subject )
	//	{
	//		HRESULT hr =S_FALSE ;
	//		if( lpDisp != NULL )
	//		{
	//			CComPtr<Outlook::_MailItem> spCurMailItem = 0;
	//			hr = lpDisp->QueryInterface(Outlook::IID__MailItem, (void**)&spCurMailItem);
	//			if(SUCCEEDED(hr) && spCurMailItem)
	//			{
	//				spCurMailItem->get_Subject( Subject ) ;
	//				return hr ;
	//			}
	//			CComPtr<Outlook::_AppointmentItem> spCurAppItem = 0;
	//			hr = lpDisp->QueryInterface(Outlook::IID__AppointmentItem, (void**)&spCurAppItem);
	//			if(SUCCEEDED(hr) && spCurAppItem)
	//			{
	//				hr = spCurAppItem->get_Subject( Subject ) ;
	//
	//				return hr ;
	//			}
	//			CComPtr<Outlook::_TaskItem> spCurTaskItem = 0;
	//			hr = lpDisp->QueryInterface(Outlook::IID__TaskItem, (void**)&spCurTaskItem);
	//			if(SUCCEEDED(hr) && spCurTaskItem)
	//			{
	//				hr = spCurTaskItem->get_Subject( Subject ) ;
	//
	//				return hr ;
	//			}
	//#ifdef WSO2K7
	//			CComPtr<Outlook::_SharingItem> spCurShareItem = 0;
	//			hr = lpDisp->QueryInterface(Outlook::IID__SharingItem, (void**)&spCurShareItem);
	//			if(SUCCEEDED(hr) && spCurShareItem)
	//			{
	//				hr = spCurShareItem->get_Subject( Subject ) ;
	//
	//				return hr ;
	//			}
	//#endif
	//		}
	//		return hr ;
	//	}
	
	static HRESULT get_Recipients(CComPtr<IDispatch> lpDisp,struct Recipients * * Recipients  )
	{
		HRESULT hr =S_FALSE ;
		if( lpDisp != NULL )
		{
			CComPtr<Outlook::_MailItem> spCurMailItem = 0;
			hr = lpDisp->QueryInterface(Outlook::IID__MailItem, (void**)&spCurMailItem);
			if(SUCCEEDED(hr) && spCurMailItem)
			{
				spCurMailItem->get_Recipients( Recipients ) ;
				return hr ;
			}
			CComPtr<Outlook::_AppointmentItem> spCurAppItem = 0;//can get organiger
			hr = lpDisp->QueryInterface(Outlook::IID__AppointmentItem, (void**)&spCurAppItem);
			if(SUCCEEDED(hr) && spCurAppItem)
			{
				hr = spCurAppItem->get_Recipients( Recipients ) ;
				
				return hr ;
			}
			CComPtr<Outlook::_TaskItem> spCurTaskItem = 0;
			hr = lpDisp->QueryInterface(Outlook::IID__TaskItem, (void**)&spCurTaskItem);
			if(SUCCEEDED(hr) && spCurTaskItem)
			{
				hr = spCurTaskItem->get_Recipients( Recipients);
				
				return hr ;
			}
			CComPtr<Outlook::_TaskRequestItem> spCurTaskReqItem = 0;
			hr = lpDisp->QueryInterface(Outlook::IID__TaskRequestItem, (void**)&spCurTaskReqItem);
			if(SUCCEEDED(hr) && spCurTaskReqItem)
			{
				hr = spCurTaskReqItem->GetAssociatedTask( TRUE,&spCurTaskItem ) ;//FALSE to TRUE bug 41900
				if(SUCCEEDED(hr) && spCurTaskItem)
				{
					hr = spCurTaskItem->get_Recipients( Recipients);
				}

				return hr ;
			}
			CComPtr<Outlook::_MeetingItem> spCurMeetItem = 0;//can't get organiger
			hr = lpDisp->QueryInterface(Outlook::IID__MeetingItem, (void**)&spCurMeetItem);
			if(SUCCEEDED(hr) && spCurMeetItem)
			{
				hr = spCurMeetItem->get_Recipients( Recipients);
				
				return hr ;
			}
#if ((defined WSO2K16) || (defined WSO2K13) || (defined WSO2K10))
//add sharing item, bug 42622
//#ifdef WSO2K7
			CComPtr<Outlook::_SharingItem> spCurShareItem = 0;
			hr = lpDisp->QueryInterface(Outlook::IID__SharingItem, (void**)&spCurShareItem);
			if(SUCCEEDED(hr) && spCurShareItem)
			{
				hr = spCurShareItem->get_Recipients( Recipients ) ;

				return hr ;
			}
#endif
		}
		return hr ;
	}
	static HRESULT DeleteMailItem(CComPtr<IDispatch> lpDisp)
	{
		HRESULT hr = S_FALSE;
		if (lpDisp != NULL)
		{
			CComPtr<Outlook::_MailItem> spCurMailItem = 0;
			hr = lpDisp->QueryInterface(Outlook::IID__MailItem, (void**)&spCurMailItem);
			if (SUCCEEDED(hr) && spCurMailItem)
			{
				hr = spCurMailItem->Delete();
			}
		}
		return hr;
	}
	static HRESULT Save(CComPtr<IDispatch> lpDisp)
	{
		HRESULT hr =S_FALSE ;
		if( lpDisp != NULL )
		{
			CComPtr<Outlook::_MailItem> spCurMailItem = 0;
			hr = lpDisp->QueryInterface(Outlook::IID__MailItem, (void**)&spCurMailItem);
			if(SUCCEEDED(hr) && spCurMailItem)
			{
				spCurMailItem->Save(  ) ;
				logi(L"=====>normal Email save.\n");
				return hr ;
			}
			CComPtr<Outlook::_AppointmentItem> spCurAppItem = 0;
			hr = lpDisp->QueryInterface(Outlook::IID__AppointmentItem, (void**)&spCurAppItem);
			if(SUCCEEDED(hr) && spCurAppItem)
			{
				g_pOutlookObj->SetAutoCloseMeetingSaveTipWindow(TRUE);
			    hr = spCurAppItem->Save(  ) ;
				g_pOutlookObj->SetAutoCloseMeetingSaveTipWindow(FALSE);
				return hr ;
			}
			
			CComPtr<Outlook::_TaskItem> spCurTaskItem = 0;
			hr = lpDisp->QueryInterface(Outlook::IID__TaskItem, (void**)&spCurTaskItem);
			logd(L"[Save]hr = %#x, spCurTaskItem@%p", hr, spCurTaskItem);
			if(SUCCEEDED(hr) && spCurTaskItem)
			{
				hr = spCurTaskItem->Save(  ) ;

				return hr ;
			}

			// It seems not allowed to save #TaskRequestItem within the OnSend event for the action
			// "Assign Task" since the receiver will cannot open the task inside.
			/*
			CComPtr<Outlook::_TaskRequestItem> spCurTaskReqItem = 0;
			hr = lpDisp->QueryInterface(Outlook::IID__TaskRequestItem, (void**)&spCurTaskReqItem);
			if(SUCCEEDED(hr) && spCurTaskReqItem)
			{
				hr = spCurTaskReqItem->Save(  ) ;

				return hr ;
			}
			*/

			CComPtr<Outlook::_MeetingItem> spCurMeetItem = 0;
			hr = lpDisp->QueryInterface(Outlook::IID__MeetingItem, (void**)&spCurMeetItem);
			if(SUCCEEDED(hr) && spCurMeetItem)
			{
				hr = spCurMeetItem->Save(  ) ;

				return hr ;
			}
			
			// @Ray add for note 2017/2/28
			CComPtr<Outlook::_NoteItem> spCurNoteItem = 0;
			hr = lpDisp->QueryInterface(Outlook::IID__NoteItem, (void**)&spCurNoteItem);
			if (SUCCEEDED(hr) && spCurNoteItem)
			{
				hr = spCurNoteItem->Save(  ) ;

				return hr ;
			}
#if ((defined WSO2K16) || (defined WSO2K13) || (defined WSO2K10))
//add sharing item, bug 42622
//#ifdef WSO2K7
			CComPtr<Outlook::_SharingItem> spCurShareItem = 0;
			hr = lpDisp->QueryInterface(Outlook::IID__SharingItem, (void**)&spCurShareItem);
			if(SUCCEEDED(hr) && spCurShareItem)
			{
				hr = spCurShareItem->Save(  ) ;

				return hr ;
			}
#endif
		}
		return hr ;
	}
	static HRESULT get_Attachments(CComPtr<IDispatch> lpDisp ,struct Attachments * * Attachments, bool bNeedAssociate )
	{
		HRESULT hr =S_FALSE ;
		if( lpDisp != NULL )
		{
			CComPtr<Outlook::_MailItem> spCurMailItem = 0;
			hr = lpDisp->QueryInterface(Outlook::IID__MailItem, (void**)&spCurMailItem);
			if(SUCCEEDED(hr) && spCurMailItem)
			{
				spCurMailItem->get_Attachments(Attachments) ;
				return hr ;
			}
			CComPtr<Outlook::_AppointmentItem> spCurAppItem = 0;
			hr = lpDisp->QueryInterface(Outlook::IID__AppointmentItem, (void**)&spCurAppItem);
			if(SUCCEEDED(hr) && spCurAppItem)
			{
				hr = spCurAppItem->get_Attachments( Attachments ) ;

				return hr ;
			}
			CComPtr<Outlook::_TaskItem> spCurTaskItem = 0;
			hr = lpDisp->QueryInterface(Outlook::IID__TaskItem, (void**)&spCurTaskItem);
			if(SUCCEEDED(hr) && spCurTaskItem)
			{
				hr = spCurTaskItem->get_Attachments(Attachments) ;

				return hr ;
			}
			CComPtr<Outlook::_TaskRequestItem> spCurTaskReqItem = 0;
			hr = lpDisp->QueryInterface(Outlook::IID__TaskRequestItem, (void**)&spCurTaskReqItem);
			if(SUCCEEDED(hr) && spCurTaskReqItem)
			{
				if (bNeedAssociate){
					logd(L"[get_Attachments]need associate!");
					hr = spCurTaskReqItem->GetAssociatedTask( TRUE,&spCurTaskItem ) ;
					if(SUCCEEDED(hr) && spCurTaskItem)
					{
						logd(L"[get_Attachments]associate succeed, get attachments!");
						hr = spCurTaskItem->get_Attachments(Attachments) ;
					}
				}
				else//assign task, firstly get attachment, don't need to associate
				hr = spCurTaskReqItem->get_Attachments(Attachments) ;
				return hr ;
			}
			CComPtr<Outlook::_MeetingItem> spCurMeetItem = 0;
			hr = lpDisp->QueryInterface(Outlook::IID__MeetingItem, (void**)&spCurMeetItem);
			if(SUCCEEDED(hr) && spCurMeetItem)
			{
				hr = spCurMeetItem->get_Attachments(Attachments) ;

				return hr ;
			}
#if ((defined WSO2K16) || (defined WSO2K13) || (defined WSO2K10))
//add sharing item, bug 42622
//#ifdef WSO2K7
			CComPtr<Outlook::_SharingItem> spCurShareItem = 0;
			hr = lpDisp->QueryInterface(Outlook::IID__SharingItem, (void**)&spCurShareItem);
			if(SUCCEEDED(hr) && spCurShareItem)
			{
				hr = spCurShareItem->get_Attachments(Attachments) ;

				return hr ;
			}
#endif
		}
		return hr ;
	}
	static HRESULT get_GetInspector(CComPtr<IDispatch> lpDisp , struct _Inspector * * GetInspector  )
	{
		HRESULT hr =S_FALSE ;
		if( lpDisp != NULL )
		{
			CComPtr<Outlook::_MailItem> spCurMailItem = 0;
			hr = lpDisp->QueryInterface(Outlook::IID__MailItem, (void**)&spCurMailItem);
			if(SUCCEEDED(hr) && spCurMailItem)
			{
				spCurMailItem->get_GetInspector(GetInspector) ;
				return hr ;
			}
			CComPtr<Outlook::_AppointmentItem> spCurAppItem = 0;
			hr = lpDisp->QueryInterface(Outlook::IID__AppointmentItem, (void**)&spCurAppItem);
			if(SUCCEEDED(hr) && spCurAppItem)
			{
				hr = spCurAppItem->get_GetInspector( GetInspector ) ;

				return hr ;
			}
			CComPtr<Outlook::_TaskItem> spCurTaskItem = 0;
			hr = lpDisp->QueryInterface(Outlook::IID__TaskItem, (void**)&spCurTaskItem);
			if(SUCCEEDED(hr) && spCurTaskItem)
			{
				hr = spCurTaskItem->get_GetInspector(GetInspector) ;

				return hr ;
			}
			CComPtr<Outlook::_TaskRequestItem> spCurTaskReqItem = 0;
			hr = lpDisp->QueryInterface(Outlook::IID__TaskRequestItem, (void**)&spCurTaskReqItem);
			if(SUCCEEDED(hr) && spCurTaskReqItem)
			{
				hr = spCurTaskReqItem->get_GetInspector(GetInspector) ;

				return hr ;
			}
			CComPtr<Outlook::_MeetingItem> spCurMeetItem = 0;
			hr = lpDisp->QueryInterface(Outlook::IID__MeetingItem, (void**)&spCurMeetItem);
			if(SUCCEEDED(hr) && spCurMeetItem)
			{
				hr = spCurMeetItem->get_GetInspector(GetInspector) ;

				return hr ;
			}
#if ((defined WSO2K16) || (defined WSO2K13) || (defined WSO2K10))
			//add sharing item, bug 42622
//#ifdef WSO2K7
			CComPtr<Outlook::_SharingItem> spCurShareItem = 0;
			hr = lpDisp->QueryInterface(Outlook::IID__SharingItem, (void**)&spCurShareItem);
			if(SUCCEEDED(hr) && spCurShareItem)
			{
				hr = spCurShareItem->get_GetInspector(GetInspector) ;

				return hr ;
			}
#endif
		}
		return hr ;
	}
	static HRESULT get_HTMLBody(CComPtr<IDispatch> lpDisp , BSTR * HTMLBody  )
	{
		HRESULT hr =S_FALSE ;
		if( lpDisp != NULL )
		{
			CComPtr<Outlook::_MailItem> spCurMailItem = 0;
			hr = lpDisp->QueryInterface(Outlook::IID__MailItem, (void**)&spCurMailItem);
			if(SUCCEEDED(hr) && spCurMailItem)
			{
				spCurMailItem->get_HTMLBody(HTMLBody) ;
				return hr ;
			}
			CComPtr<Outlook::_AppointmentItem> spCurAppItem = 0;
			hr = lpDisp->QueryInterface(Outlook::IID__AppointmentItem, (void**)&spCurAppItem);
			if(SUCCEEDED(hr) && spCurAppItem)
			{
				hr = spCurAppItem->get_Body( HTMLBody ) ;
				return hr ;
			}
			CComPtr<Outlook::_TaskItem> spCurTaskItem = 0;
			hr = lpDisp->QueryInterface(Outlook::IID__TaskItem, (void**)&spCurTaskItem);
			if(SUCCEEDED(hr) && spCurTaskItem)
			{
				hr = spCurTaskItem->get_Body(HTMLBody) ;
				return hr ;
			}
			CComPtr<Outlook::_MeetingItem> spCurMeetItem = 0;
			hr = lpDisp->QueryInterface(Outlook::IID__MeetingItem, (void**)&spCurMeetItem);
			if(SUCCEEDED(hr) && spCurMeetItem)
			{
				hr = spCurMeetItem->get_Body(HTMLBody) ;

				return hr ;
			}
#if ((defined WSO2K16) || (defined WSO2K13) || (defined WSO2K10))
			//add sharing item, bug 42622
//#ifdef WSO2K7
			CComPtr<Outlook::_SharingItem> spCurShareItem = 0;
			hr = lpDisp->QueryInterface(Outlook::IID__SharingItem, (void**)&spCurShareItem);
			if(SUCCEEDED(hr) && spCurShareItem)
			{
				hr = spCurShareItem->get_Body(HTMLBody) ;
				return hr ;
			}
#endif
		}
		return hr ;
	}
	static HRESULT put_HTMLBody(CComPtr<IDispatch> lpDisp , BSTR  HTMLBody  )
	{
		HRESULT hr =S_FALSE ;
		if( lpDisp != NULL )
		{
			CComPtr<Outlook::_MailItem> spCurMailItem = 0;
			hr = lpDisp->QueryInterface(Outlook::IID__MailItem, (void**)&spCurMailItem);
			if(SUCCEEDED(hr) && spCurMailItem)
			{
				spCurMailItem->put_HTMLBody(HTMLBody) ;
				return hr ;
			}
			CComPtr<Outlook::_AppointmentItem> spCurAppItem = 0;
			hr = lpDisp->QueryInterface(Outlook::IID__AppointmentItem, (void**)&spCurAppItem);
			if(SUCCEEDED(hr) && spCurAppItem)
			{
				hr = spCurAppItem->put_Body( HTMLBody ) ;
				return hr ;
			}
			CComPtr<Outlook::_TaskItem> spCurTaskItem = 0;
			hr = lpDisp->QueryInterface(Outlook::IID__TaskItem, (void**)&spCurTaskItem);
			if(SUCCEEDED(hr) && spCurTaskItem)
			{
				hr = spCurTaskItem->put_Body(HTMLBody) ;
				return hr ;
			}
			CComPtr<Outlook::_MeetingItem> spCurMeetItem = 0;
			hr = lpDisp->QueryInterface(Outlook::IID__MeetingItem, (void**)&spCurMeetItem);
			if(SUCCEEDED(hr) && spCurMeetItem)
			{
				hr = spCurMeetItem->put_Body(HTMLBody) ;
				if(SUCCEEDED(hr))
				{
					hr=spCurMeetItem->Save();
					if(FAILED(hr))
					{
						DP((L"put_HTMLBody MeetingItem FAILED HRESULT:%x",hr));
					}
				}
				return hr ;
			}
#if ((defined WSO2K16) || (defined WSO2K13) || (defined WSO2K10))
			//add sharing item, bug 42622
//#ifdef WSO2K7
			CComPtr<Outlook::_SharingItem> spCurShareItem = 0;
			hr = lpDisp->QueryInterface(Outlook::IID__SharingItem, (void**)&spCurShareItem);
			if(SUCCEEDED(hr) && spCurShareItem)
			{
				hr = spCurShareItem->put_Body(HTMLBody) ;
				return hr ;
			}
#endif
		}
		return hr ;
	}
	static HRESULT get_BodyFormat(CComPtr<IDispatch> lpDisp ,  enum OlBodyFormat * BodyFormat  )
	{
		HRESULT hr =S_FALSE ;
		if( lpDisp != NULL )
		{
			CComPtr<Outlook::_MailItem> spCurMailItem = 0;
			hr = lpDisp->QueryInterface(Outlook::IID__MailItem, (void**)&spCurMailItem);
			if(SUCCEEDED(hr) && spCurMailItem)
			{
				spCurMailItem->get_BodyFormat(BodyFormat) ;
				return hr ;
			}
			
#if ((defined WSO2K16) || (defined WSO2K13) || (defined WSO2K10))
			//add sharing item, bug 42622
			CComPtr<Outlook::_SharingItem> spCurShareItem = 0;
			hr = lpDisp->QueryInterface(Outlook::IID__SharingItem, (void**)&spCurShareItem);
			if(SUCCEEDED(hr) && spCurShareItem)
			{
			
				hr = spCurShareItem->get_BodyFormat(BodyFormat) ;
			
				return hr ;
			}
#endif

			CComPtr<Outlook::_PostItem> spCurPostItem = 0;
			hr = lpDisp->QueryInterface(Outlook::IID__PostItem, (void**)&spCurPostItem);
			if(SUCCEEDED(hr) && spCurPostItem)
			{	
				hr = spCurPostItem->get_BodyFormat(BodyFormat) ;
				return hr ;
			}

			CComPtr<Outlook::_AppointmentItem> spCurAppointmentItem = 0;
			hr = lpDisp->QueryInterface(Outlook::IID__AppointmentItem, (void**)&spCurAppointmentItem);
			if(SUCCEEDED(hr) && spCurAppointmentItem)
			{
				* BodyFormat = olFormatUnspecified;
				return hr ;
			}

			CComPtr<Outlook::_MeetingItem> spCurMeetItem = 0;
			hr = lpDisp->QueryInterface(Outlook::IID__MeetingItem, (void**)&spCurMeetItem);
			if(SUCCEEDED(hr) && spCurMeetItem)
			{
				* BodyFormat = olFormatUnspecified;
				return hr ;
			}

			CComPtr<Outlook::_TaskItem> spCurTaskItem = 0;
			hr = lpDisp->QueryInterface(Outlook::IID__TaskItem, (void**)&spCurTaskItem);
			if(SUCCEEDED(hr) && spCurTaskItem)
			{
				* BodyFormat = olFormatUnspecified;
				return hr ;
			}

			CComPtr<Outlook::_TaskRequestItem> spCurTaskReqItem = 0;
			hr = lpDisp->QueryInterface(Outlook::IID__TaskRequestItem, (void**)&spCurTaskReqItem);
			if(SUCCEEDED(hr) && spCurTaskReqItem)
			{
				* BodyFormat = olFormatUnspecified;
				return hr ;
			}

		}
		return hr ;
	}
	static HRESULT get_Subject(CComPtr<IDispatch> lpDisp ,  BSTR * Subject )
	{
		HRESULT hr =S_FALSE ;
		if( lpDisp != NULL )
		{
			CComPtr<Outlook::_MailItem> spCurMailItem = 0;
			hr = lpDisp->QueryInterface(Outlook::IID__MailItem, (void**)&spCurMailItem);
			if(SUCCEEDED(hr) && spCurMailItem)
			{
				spCurMailItem->get_Subject(Subject) ;
				return hr ;
			}
			CComPtr<Outlook::_AppointmentItem> spCurAppItem = 0;
			hr = lpDisp->QueryInterface(Outlook::IID__AppointmentItem, (void**)&spCurAppItem);
			if(SUCCEEDED(hr) && spCurAppItem)
			{
				hr = spCurAppItem->get_Subject( Subject ) ;

				return hr ;
			}
			CComPtr<Outlook::_TaskItem> spCurTaskItem = 0;
			hr = lpDisp->QueryInterface(Outlook::IID__TaskItem, (void**)&spCurTaskItem);
			if(SUCCEEDED(hr) && spCurTaskItem)
			{
				hr = spCurTaskItem->get_Subject(Subject) ;

				return hr ;
			}
			CComPtr<Outlook::_TaskRequestItem> spCurTaskReqItem = 0;
			hr = lpDisp->QueryInterface(Outlook::IID__TaskRequestItem, (void**)&spCurTaskReqItem);
			if(SUCCEEDED(hr) && spCurTaskReqItem)
			{
				hr = spCurTaskReqItem->get_Subject(Subject) ;

				return hr ;
			}
			CComPtr<Outlook::_MeetingItem> spCurMeetItem = 0;
			hr = lpDisp->QueryInterface(Outlook::IID__MeetingItem, (void**)&spCurMeetItem);
			if(SUCCEEDED(hr) && spCurMeetItem)
			{
				hr = spCurMeetItem->get_Subject(Subject) ;

				return hr ;
			}
			// @Ray  add for Notes.  2017/2/28   
			CComPtr<Outlook::_NoteItem> spCurNoteItem = 0;
			hr = lpDisp->QueryInterface(Outlook::IID__NoteItem, (void**)&spCurNoteItem);
			if(SUCCEEDED(hr) && spCurNoteItem)
			{
				hr = spCurNoteItem->get_Subject(Subject) ;

				return hr ;
			}
#if ((defined WSO2K16) || (defined WSO2K13) || (defined WSO2K10))
			//add sharing item, bug 42622
			CComPtr<Outlook::_SharingItem> spCurSharingItem = 0;
			hr = lpDisp->QueryInterface(Outlook::IID__SharingItem, (void**)&spCurSharingItem);
			if(SUCCEEDED(hr) && spCurSharingItem)
			{
				hr = spCurSharingItem->get_Subject(Subject) ;

				return hr ;
			}
#endif
#ifdef WSO2K7
			CComPtr<Outlook::_SharingItem> spCurShareItem = 0;
			hr = lpDisp->QueryInterface(Outlook::IID__SharingItem, (void**)&spCurShareItem);
			if(SUCCEEDED(hr) && spCurShareItem)
			{
				hr = spCurShareItem->get_Subject(Subject) ;

				return hr ;
			}
#endif
		}
		return hr ;
	}
	static HRESULT put_Subject(CComPtr<IDispatch> lpDisp ,  BSTR  Subject )
	{
		HRESULT hr =S_FALSE ;
		if( lpDisp != NULL )
		{
			CComPtr<Outlook::_MailItem> spCurMailItem = 0;
			hr = lpDisp->QueryInterface(Outlook::IID__MailItem, (void**)&spCurMailItem);
			if(SUCCEEDED(hr) && spCurMailItem)
			{
				spCurMailItem->put_Subject(Subject) ;
				return hr ;
			}
			CComPtr<Outlook::_AppointmentItem> spCurAppItem = 0;
			hr = lpDisp->QueryInterface(Outlook::IID__AppointmentItem, (void**)&spCurAppItem);
			if(SUCCEEDED(hr) && spCurAppItem)
			{
				hr = spCurAppItem->put_Subject( Subject ) ;

				return hr ;
			}
			CComPtr<Outlook::_TaskItem> spCurTaskItem = 0;
			hr = lpDisp->QueryInterface(Outlook::IID__TaskItem, (void**)&spCurTaskItem);
			if(SUCCEEDED(hr) && spCurTaskItem)
			{
				hr = spCurTaskItem->put_Subject(Subject) ;
				logi(L"set assign task subject,hr = 0x%x\n",hr);
				return hr ;
			}
			CComPtr<Outlook::_TaskRequestItem> spCurTaskReqItem = 0;
			hr = lpDisp->QueryInterface(Outlook::IID__TaskRequestItem, (void**)&spCurTaskReqItem);
			if(SUCCEEDED(hr) && spCurTaskReqItem)
			{
				hr = spCurTaskReqItem->put_Subject(Subject) ;

				return hr ;
			}
			CComPtr<Outlook::_MeetingItem> spCurMeetItem = 0;
			hr = lpDisp->QueryInterface(Outlook::IID__MeetingItem, (void**)&spCurMeetItem);
			if(SUCCEEDED(hr) && spCurMeetItem)
			{
				hr = spCurMeetItem->put_Subject(Subject) ;

				return hr ;
			}
#if ((defined WSO2K16) || (defined WSO2K13) || (defined WSO2K10))
			//add sharing item, bug 42622
//#ifdef WSO2K7
			CComPtr<Outlook::_SharingItem> spCurShareItem = 0;
			hr = lpDisp->QueryInterface(Outlook::IID__SharingItem, (void**)&spCurShareItem);
			if(SUCCEEDED(hr) && spCurShareItem)
			{
				hr = spCurShareItem->put_Subject(Subject) ;

				return hr ;
			}
#endif
		}
		return hr ;
	}
	static HRESULT get_olApplication(CComPtr<IDispatch> lpDisp , struct Outlook::_Application * * olApplication  )
	{
		HRESULT hr =S_FALSE ;
		if( lpDisp != NULL )
		{
			CComPtr<Outlook::_MailItem> spCurMailItem = 0;
			hr = lpDisp->QueryInterface(Outlook::IID__MailItem, (void**)&spCurMailItem);
			if(SUCCEEDED(hr) && spCurMailItem)
			{
				spCurMailItem->get_Application(olApplication) ;
				return hr ;
			}
			CComPtr<Outlook::_AppointmentItem> spCurAppItem = 0;
			hr = lpDisp->QueryInterface(Outlook::IID__AppointmentItem, (void**)&spCurAppItem);
			if(SUCCEEDED(hr) && spCurAppItem)
			{
				spCurAppItem->get_Application(olApplication) ;
				return hr ;
			}
			CComPtr<Outlook::_TaskItem> spCurTaskItem = 0;
			hr = lpDisp->QueryInterface(Outlook::IID__TaskItem, (void**)&spCurTaskItem);
			if(SUCCEEDED(hr) && spCurTaskItem)
			{
				spCurTaskItem->get_Application(olApplication) ;
				return hr ;
			}
			CComPtr<Outlook::_TaskRequestItem> spCurTaskReqItem = 0;
			hr = lpDisp->QueryInterface(Outlook::IID__TaskRequestItem, (void**)&spCurTaskReqItem);
			if(SUCCEEDED(hr) && spCurTaskReqItem)
			{
				spCurTaskReqItem->get_Application(olApplication) ;
				return hr ;
			}
			CComPtr<Outlook::_MeetingItem> spCurMeetItem = 0;
			hr = lpDisp->QueryInterface(Outlook::IID__MeetingItem, (void**)&spCurMeetItem);
			if(SUCCEEDED(hr) && spCurMeetItem)
			{
				spCurMeetItem->get_Application(olApplication) ;

				return hr ;
			}
#if ((defined WSO2K16) || (defined WSO2K13) || (defined WSO2K10))
			//add sharing item, bug 42622
//#ifdef WSO2K7
			CComPtr<Outlook::_SharingItem> spCurShareItem = 0;
			hr = lpDisp->QueryInterface(Outlook::IID__SharingItem, (void**)&spCurShareItem);
			if(SUCCEEDED(hr) && spCurShareItem)
			{
				hr = spCurShareItem->get_Application(olApplication) ;

				return hr ;
			}
#endif
		}
		return hr ;
	}
	static HRESULT SaveAs(CComPtr<IDispatch> lpDisp , BSTR Path, VARIANT Type = vtMissing  )
	{
		HRESULT hr =S_FALSE ;
		if( lpDisp != NULL )
		{
			CComPtr<Outlook::_MailItem> spCurMailItem = 0;
			hr = lpDisp->QueryInterface(Outlook::IID__MailItem, (void**)&spCurMailItem);
			if(SUCCEEDED(hr) && spCurMailItem)
			{
				spCurMailItem->SaveAs(Path,Type) ;
				return hr ;
			}
			CComPtr<Outlook::_AppointmentItem> spCurAppItem = 0;
			hr = lpDisp->QueryInterface(Outlook::IID__AppointmentItem, (void**)&spCurAppItem);
			if(SUCCEEDED(hr) && spCurAppItem)
			{
				hr = spCurAppItem->SaveAs( Path,Type ) ;

				return hr ;
			}
			CComPtr<Outlook::_TaskItem> spCurTaskItem = 0;
			hr = lpDisp->QueryInterface(Outlook::IID__TaskItem, (void**)&spCurTaskItem);
			if(SUCCEEDED(hr) && spCurTaskItem)
			{

				DP((L"_TaskItem save as")) ;
				hr = spCurTaskItem->SaveAs(Path,Type) ;

				return hr ;
			}
			CComPtr<Outlook::_TaskRequestItem> spCurTaskReqItem = 0;
			hr = lpDisp->QueryInterface(Outlook::IID__TaskRequestItem, (void**)&spCurTaskReqItem);
			if(SUCCEEDED(hr) && spCurTaskReqItem)
			{
				DP((L"_TaskRequestItem save as")) ;

				hr = spCurTaskReqItem->SaveAs(Path,Type) ;
				return hr ;
			}
			CComPtr<Outlook::_MeetingItem> spCurMeetItem = 0;
			hr = lpDisp->QueryInterface(Outlook::IID__MeetingItem, (void**)&spCurMeetItem);
			if(SUCCEEDED(hr) && spCurMeetItem)
			{
				hr = spCurMeetItem->SaveAs(Path,Type) ;

				return hr ;
			}
#if ((defined WSO2K16) || (defined WSO2K13) || (defined WSO2K10))
			//add sharing item, bug 42622
//#ifdef WSO2K7
			CComPtr<Outlook::_SharingItem> spCurShareItem = 0;
			hr = lpDisp->QueryInterface(Outlook::IID__SharingItem, (void**)&spCurShareItem);
			if(SUCCEEDED(hr) && spCurShareItem)
			{
				hr = spCurShareItem->SaveAs(Path,Type) ;

				return hr ;
			}
#endif
		}
		return hr ;
	}
	static HRESULT get_Body(CComPtr<IDispatch> lpDisp ,BSTR * Body )
	{
		HRESULT hr =S_FALSE ;
		if( lpDisp != NULL )
		{
			CComPtr<Outlook::_MailItem> spCurMailItem = 0;
			hr = lpDisp->QueryInterface(Outlook::IID__MailItem, (void**)&spCurMailItem);
			if(SUCCEEDED(hr) && spCurMailItem)
			{
				spCurMailItem->get_Body(Body) ;
				return hr ;
			}
			CComPtr<Outlook::_AppointmentItem> spCurAppItem = 0;
			hr = lpDisp->QueryInterface(Outlook::IID__AppointmentItem, (void**)&spCurAppItem);
			if(SUCCEEDED(hr) && spCurAppItem)
			{
				hr = spCurAppItem->get_Body( Body ) ;

				return hr ;
			}
			CComPtr<Outlook::_TaskItem> spCurTaskItem = 0;
			hr = lpDisp->QueryInterface(Outlook::IID__TaskItem, (void**)&spCurTaskItem);
			if(SUCCEEDED(hr) && spCurTaskItem)
			{
				hr = spCurTaskItem->get_Body(Body) ;

				return hr ;
			}
			CComPtr<Outlook::_TaskRequestItem> spCurTaskReqItem = 0;
			hr = lpDisp->QueryInterface(Outlook::IID__TaskRequestItem, (void**)&spCurTaskReqItem);
			if(SUCCEEDED(hr) && spCurTaskReqItem)
			{
				DPW((L"spCurTaskReqItem request item")) ;
				spCurTaskReqItem->get_Body( Body) ;
				return hr ;
			}
			CComPtr<Outlook::_MeetingItem> spCurMeetItem = 0;
			hr = lpDisp->QueryInterface(Outlook::IID__MeetingItem, (void**)&spCurMeetItem);
			if(SUCCEEDED(hr) && spCurMeetItem)
			{
				hr = spCurMeetItem->get_Body( Body) ;

				return hr ;
			}
			// @Ray add for Notes 2017/2/28
			CComPtr<Outlook::_NoteItem> spCurNoteItem = 0;
			hr = lpDisp->QueryInterface(Outlook::IID__NoteItem, (void**)&spCurNoteItem);
			if(SUCCEEDED(hr) && spCurMeetItem)
			{
				hr = spCurNoteItem->get_Body( Body) ;

				return hr ;
			}
#if ((defined WSO2K16) || (defined WSO2K13) || (defined WSO2K10))
			//add sharing item, bug 42622
//#ifdef WSO2K7
			CComPtr<Outlook::_SharingItem> spCurShareItem = 0;
			hr = lpDisp->QueryInterface(Outlook::IID__SharingItem, (void**)&spCurShareItem);
			if(SUCCEEDED(hr) && spCurShareItem)
			{
				hr = spCurShareItem->get_Body(Body) ;

				return hr ;
			}
#endif
		}
		return hr ;
	}
	static HRESULT put_Body(CComPtr<IDispatch> lpDisp ,BSTR  Body )
	{
		HRESULT hr =S_FALSE ;
		if( lpDisp != NULL )
		{
			CComPtr<Outlook::_MailItem> spCurMailItem = 0;
			hr = lpDisp->QueryInterface(Outlook::IID__MailItem, (void**)&spCurMailItem);
			if(SUCCEEDED(hr) && spCurMailItem)
			{
				spCurMailItem->put_Body(Body) ;
				return hr ;
			}
			CComPtr<Outlook::_AppointmentItem> spCurAppItem = 0;
			hr = lpDisp->QueryInterface(Outlook::IID__AppointmentItem, (void**)&spCurAppItem);
			if(SUCCEEDED(hr) && spCurAppItem)
			{
				hr = spCurAppItem->put_Body( Body ) ;

				return hr ;
			}
			CComPtr<Outlook::_TaskItem> spCurTaskItem = 0;
			hr = lpDisp->QueryInterface(Outlook::IID__TaskItem, (void**)&spCurTaskItem);
			if(SUCCEEDED(hr) && spCurTaskItem)
			{
				hr = spCurTaskItem->put_Body(Body) ;

				return hr ;
			}
			CComPtr<Outlook::_TaskRequestItem> spCurTaskReqItem = 0;
			hr = lpDisp->QueryInterface(Outlook::IID__TaskRequestItem, (void**)&spCurTaskReqItem);
			if(SUCCEEDED(hr) && spCurTaskReqItem)
			{
				hr = spCurTaskReqItem->put_Body(Body) ;
				return hr ;
			}
			CComPtr<Outlook::_MeetingItem> spCurMeetItem = 0;
			hr = lpDisp->QueryInterface(Outlook::IID__MeetingItem, (void**)&spCurMeetItem);
			if(SUCCEEDED(hr) && spCurMeetItem)
			{
				hr = spCurMeetItem->put_Body(Body) ;
				if(SUCCEEDED(hr))
				{
					hr=spCurMeetItem->Save();
					if(FAILED(hr))
					{
						DP((L"put_Body MeetingItem FAILED HRESULT:%x",hr));
					}
				}
				return hr ;
			}
			// @Ray add for Note. 2017/3/1
			CComPtr<Outlook::_NoteItem> spCurNoteItem = 0;
			hr = lpDisp->QueryInterface(Outlook::IID__NoteItem, (void**)&spCurNoteItem);
			if(SUCCEEDED(hr) && spCurNoteItem)
			{
				hr = spCurNoteItem->put_Body(Body) ;

				return hr ;
			}
#if ((defined WSO2K16) || (defined WSO2K13) || (defined WSO2K10))
			//add sharing item, bug 42622
//#ifdef WSO2K7
			CComPtr<Outlook::_SharingItem> spCurShareItem = 0;
			hr = lpDisp->QueryInterface(Outlook::IID__SharingItem, (void**)&spCurShareItem);
			if(SUCCEEDED(hr) && spCurShareItem)
			{
				hr = spCurShareItem->put_Body(Body) ;

				return hr ;
			}
#endif
		}
		return hr ;
	}
};
using namespace std;

struct _ItemEventMap
{

	CComPtr<IDispatch> spItem;
	CComObject<CItemEventDisp>* spItemEventMap ;
};

class CItemEventList
{
public:
	CItemEventList()
	{

		::InitializeCriticalSection(&m_csList);
	}
	~CItemEventList()
	{
		m_list.clear();
		::DeleteCriticalSection(&m_csList);
	}

	void AddItem( CComPtr<IDispatch> spItem,  CComObject<CItemEventDisp>* spItemEvent )
	{
		DeleteItem(spItem);//remove the item first.

		_ItemEventMap item;
		item.spItem = spItem ;
		item.spItemEventMap = spItemEvent ;
		EnterCriticalSection(&m_csList);
		m_list.push_back(item);//add the new item at back.
		LeaveCriticalSection(&m_csList);
	}
	CComObject<CItemEventDisp>* FindItem(const ATL::CComPtr<IDispatch> spItem)
	{
		EnterCriticalSection(&m_csList);
		for(std::list<_ItemEventMap>::iterator itr = m_list.begin(); itr != m_list.end(); )
		{

			DP((L"SpItem:[%d],[%d]",(*itr).spItem,*(*itr).spItem)) ;
			if((*itr).spItem!=NULL && ((*itr).spItem == spItem))//remove the item if it was found.
			{
				LeaveCriticalSection(&m_csList);
				return (*itr).spItemEventMap ;
			}
			itr++;
		}
		LeaveCriticalSection(&m_csList);

		return NULL;
	}

	BOOL ContainKey(const CComPtr<IDispatch> spItem, bool bDelete)
	{
		BOOL bSuccess = FALSE;

		EnterCriticalSection(&m_csList);

		for(std::list<_ItemEventMap>::iterator itr = m_list.begin(); itr != m_list.end(); )
		{

			if((*itr).spItem!=NULL && ((*itr).spItem ==spItem))//remove the item if it was found.
			{
				bSuccess = TRUE;
				if(bDelete )
				{
					m_list.erase(itr);
				}
				break;
			}
			itr++;
		}
		LeaveCriticalSection(&m_csList);

		return bSuccess;
	}

	void DeleteItem(const CComPtr<IDispatch> spItem)
	{

		EnterCriticalSection(&m_csList);

		for(std::list<_ItemEventMap>::iterator itr = m_list.begin(); itr != m_list.end(); )
		{
		if((*itr).spItem!=NULL && ((*itr).spItem ==spItem))//remove the item if it was found.
			{
				m_list.erase(itr);
				break;
			}
			itr++;
		}

		LeaveCriticalSection(&m_csList);
	}

protected:
	std::list<_ItemEventMap> m_list;
	CRITICAL_SECTION m_csList;
};
