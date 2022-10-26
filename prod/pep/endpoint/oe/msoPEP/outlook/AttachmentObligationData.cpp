#include "stdafx.h"
#include "AttachmentObligationData.h"
#include "nextlabs_feature_manager.hpp"
#include "nextlabs_features.h"
#include "../PAEx/OE_PAMngr.h"
#include "adaptermanager.h"
#include "../include/msopath.hpp"
#include "DlgOEWarningBox.h"
#include "strsafe.h"
#include "../common/policy.h"
#include "MailPropertyTool.h"


extern nextlabs::cesdk_loader cesdk;
extern nextlabs::feature_manager feat;
extern HINSTANCE	g_hTAG;
extern HINSTANCE	g_hODHD;
extern std::wstring g_strOETempFolder;
extern HINSTANCE	g_hENC;
extern HINSTANCE	g_hPE;	

///////////////TestCode////////////////
#include "../paui/assistantdlg.h"

typedef std::vector<std::wstring> RecipientVector;
typedef std::vector<std::pair<std::wstring,std::wstring>> AttachVector;
typedef std::vector<std::wstring> HelpUrlVector;
typedef long (*TFunHdrObligation)(HWND ,RecipientVector &,AttachVector &,HelpUrlVector &,VARIANT_BOOL) ;

#define OBLIGATION_STR_DELIM            L";"
#define NEXTLABS_NXL_TEMP_FOLDER        L"\\NextLabs\\cache\\"





CAttachmentObligationData::CAttachmentObligationData()
{
	
}


CAttachmentObligationData::~CAttachmentObligationData()
{
	
}

bool CAttachmentObligationData::ExcuteHCReportLogEx(CSendEmailData &SendEmailData)
{

	bool bRet = true;
	int nAttachmentNum = (int)SendEmailData.GetAttachmentData().size();
	for (int i = 0; i < nAttachmentNum; i++)
	{
		if (!SendEmailData.GetAttachmentData()[i].GetStrLogID().empty())
		{
			PolicyCommunicator::SetLogID(SendEmailData.GetAttachmentData()[i].GetStrLogID().c_str());
			vector<wstring> strInfo;
			wstring	strTemp = L"";
			SendEmailData.GetAttachmentData()[i].GetHCTagInfo(strTemp);
			strInfo.push_back(strTemp);
			PolicyCommunicator::WriteReportLog(L"OE_HIERARCHICAL_CLASSIFICATION",strInfo);
		}
	}
	return bRet;
}

// Return whether proceed or not: if false, a user presses cancel button, otherwise it presses proceed button or no warning obligation 
bool CAttachmentObligationData::ExcuteWarnMsgEx(CSendEmailData &SendEmailData)
{
	bool bRet = true;
	int nAttachmentNum = (int)SendEmailData.GetAttachmentData().size();
	bool bShowWarningMsg = true;
	int nProc = IDOK;
	for (int i = 0; i < nAttachmentNum; i++)
	{
		if (SendEmailData.GetAttachmentData()[i].IsNeedDoWarningMsgInfo())
		{
			if (bShowWarningMsg)
			{
				CDlgOEWarningBox dlgWarnBox(IDD_OE_WARNING_BOX, SendEmailData.GetAttachmentData()[i].GetWarningMsgInfo().strHeaderTxt.c_str(), SendEmailData.GetAttachmentData()[i].GetWarningMsgInfo().strDisplayTxt.c_str(), 
					SendEmailData.GetAttachmentData()[i].GetWarningMsgInfo().strProceedBtnLabel.c_str(), SendEmailData.GetAttachmentData()[i].GetWarningMsgInfo().strCancelBtnLabel.c_str());
				nProc = dlgWarnBox.DoModal(SendEmailData.GetWnd());
				bShowWarningMsg = false;
			}

			PolicyCommunicator::SetLogID(SendEmailData.GetAttachmentData()[i].GetWarningMsgInfo().strLogID.c_str());
			vector<wstring> strInfo;
			if (nProc == IDOK)
			{
				ExcuteAuditLogEx(SendEmailData,i,true);
			}
			else
			{
				ExcuteAuditLogEx(SendEmailData,i,false);
				bRet = false;
			}
		}
	}
	return bRet;
}

bool CAttachmentObligationData::ExcuteAuditLogEx(CSendEmailData &SendEmailData,int nAttachmentPos,bool bOK)
{
	if(SendEmailData.GetAttachmentData()[nAttachmentPos].GetWarningMsgInfo().strLogID.empty())
	{
		return true;
	}

	PolicyCommunicator::SetLogID(SendEmailData.GetAttachmentData()[nAttachmentPos].GetWarningMsgInfo().strLogID.c_str());
	vector<wstring> strInfo;
	wstring	strTemp = L"";

	if (bOK)
	{
		strTemp = L"User Warning Decision: Proceed \n";
	}
	else
	{
		strTemp = L"User Warning Decision: Cancel \n";
	}
	
	//SendEmailData.GetAttachmentData()[nAttachmentPos].GetAuditLogInfo(strTemp);
	//SendEmailData.GetBodyData().GetAuditLogInfo(strTemp);
	//SendEmailData.GetSubjectData().GetAuditLogInfo(strTemp);
	//SendEmailData.GetRecipientsData().GetAuditLogInfo(strTemp);
	
	strInfo.push_back(strTemp);
	PolicyCommunicator::WriteReportLog(L"OE_WARNING_MSG_PROCEED_CANCEL",strInfo);
	
	return true;
}


bool CAttachmentObligationData::GetRejectUnlessSilentInfoFromObl(CEAttributes *obligation,int ObligationPos,int nAttachmentPos,CSendEmailData &SendEmailData,int nRecipientNum)
{
    UNREFERENCED_PARAMETER(nRecipientNum);
    UNREFERENCED_PARAMETER(SendEmailData);
    UNREFERENCED_PARAMETER(nAttachmentPos);
	
	//if recipient == sender, we will not display the recipient
	//if (SendEmailData.IsEqualSender(SendEmailData.GetRecipientsData().GetSendPCRecipients()[nRecipientNum]))
	//{
		//SendEmailData.SetAllow(True);
		//SendEmailData.DeleteOrganigerForMeeting(SendEmailData);
		//return true;
	//}
	if (obligation->count >  ObligationPos + 5)//Òì³£±£»¤
	{
		const TCHAR* pTemp3 = NULL;
		const TCHAR* pTemp5 = NULL;

		pTemp3 = cesdk.fns.CEM_GetString(obligation->attrs[ObligationPos + 3].value);
		pTemp5 = cesdk.fns.CEM_GetString(obligation->attrs[ObligationPos + 5].value);
		if (pTemp3 == NULL)
		{
			return false;
		}
		
		if (pTemp3 == '\0')
		{
			SendEmailData.m_OnlyAttachmentData.m_rlInfo.bAllowOverride = FALSE;
		}
		else
		{
			SendEmailData.m_OnlyAttachmentData.m_rlInfo.bAllowOverride = (_wcsicmp(pTemp3,L"NO") == 0)?FALSE:TRUE;
		}

		if (pTemp5 == NULL)
		{
			SendEmailData.m_OnlyAttachmentData.m_rlInfo.strMessage = L"";
		}
		else
		{
			SendEmailData.m_OnlyAttachmentData.m_rlInfo.strMessage = pTemp5;
		}

		SendEmailData.GetAttachmentData()[nAttachmentPos].rl = TRUE;
		
		
		wstring strRecipient = SendEmailData.GetRecipientsData().GetSendPCRecipients()[nRecipientNum];
		SendEmailData.GetRecipientsData().AddShowDenyRecipients(strRecipient);
		SendEmailData.m_OnlyAttachmentData.m_RecordTempRecipientsForCrl += strRecipient;
		SendEmailData.m_OnlyAttachmentData.m_RecordTempRecipientsForCrl += OBLIGATION_STR_DELIM;
		wcsncpy_s(SendEmailData.GetAttachmentData()[nAttachmentPos].rlRecipients,sizeof(SendEmailData.GetAttachmentData()[nAttachmentPos].rlRecipients)/sizeof(wchar_t),SendEmailData.m_OnlyAttachmentData.m_RecordTempRecipientsForCrl.c_str(), _TRUNCATE);
		
		SendEmailData.m_OnlyAttachmentData.m_bNeedCrl = TRUE;
		return true;
	}
	return false;
}

bool CAttachmentObligationData::GetMailAttributeParsingFromObl(CEAttributes *obligation,int ObligationPos,CSendEmailData &SendEmailData)
{
	if (obligation->count >  ObligationPos + 11)
	{
		const TCHAR* pTemp3 = NULL;
		const TCHAR* pTemp5 = NULL;
		const TCHAR* pTemp7 = NULL;
		const TCHAR* pTemp9 = NULL;
		const TCHAR* pTemp11 = NULL;

		pTemp3 = cesdk.fns.CEM_GetString(obligation->attrs[ObligationPos + 3].value);
		pTemp5 = cesdk.fns.CEM_GetString(obligation->attrs[ObligationPos + 5].value);
		pTemp7 = cesdk.fns.CEM_GetString(obligation->attrs[ObligationPos + 7].value);
		pTemp9 = cesdk.fns.CEM_GetString(obligation->attrs[ObligationPos + 9].value);
		pTemp11 = cesdk.fns.CEM_GetString(obligation->attrs[ObligationPos + 11].value);
		if (pTemp3 == NULL || pTemp5 == NULL||pTemp3 == NULL || pTemp9 == NULL||pTemp11 == NULL)
		{
			return false;
		}


		if (pTemp9 != '\0')
		{
			SendEmailData.m_OnlyAttachmentData.m_mapInfo.ulMin = _wtol( pTemp9 ) ;
			if (SendEmailData.m_OnlyAttachmentData.m_mapInfo.ulMin > (DWORD)SendEmailData.GetRecipientsData().GetRealRecipentsNum())
			{
				return false;
			}
		}
		else
		{
			SendEmailData.m_OnlyAttachmentData.m_mapInfo.ulMin = 0;
		}
		
		
		const static std::wstring strWarn = L"Warn";
		
		if (pTemp11 != '\0')
		{
			SendEmailData.m_OnlyAttachmentData.m_mapInfo.ulMax = _wtol( pTemp11 ) ;
			if(SendEmailData.m_OnlyAttachmentData.m_mapInfo.ulMax >=  (DWORD)SendEmailData.GetRecipientsData().GetRealRecipentsNum())
			{
				if( _wcsnicmp( pTemp7,strWarn.c_str() ,strWarn.length()) == 0)
				{
					SendEmailData.m_OnlyAttachmentData.m_mapInfo.msgType =  L"Warning" ;
					SendEmailData.m_OnlyAttachmentData.m_mapInfo.url = ( pTemp5 == '\0'? OBLIGATION_URL_DEFAULT: pTemp5);
					SendEmailData.m_OnlyAttachmentData.m_mapInfo.strMessage = (pTemp3 == '\0'? L"Warning for the Mail recipients counts": pTemp3);
					SendEmailData.m_OnlyAttachmentData.m_mapInfo.valid = TRUE;
					SendEmailData.m_OnlyAttachmentData.m_mapInfo.bIsBlock = FALSE ;
					SendEmailData.m_OnlyAttachmentData.m_bNeedMailap = TRUE;
					return true;
				}
				else
				{

					SendEmailData.m_OnlyAttachmentData.m_mapInfo.msgType =  L"Blocked" ;
					SendEmailData.m_OnlyAttachmentData.m_mapInfo.url = ( pTemp5 == '\0'? OBLIGATION_URL_DEFAULT: pTemp5);
					SendEmailData.m_OnlyAttachmentData.m_mapInfo.strMessage = (pTemp3 == '\0'? L"Block for the Mail recipients counts": pTemp3);
					SendEmailData.m_OnlyAttachmentData.m_mapInfo.valid = TRUE;
					SendEmailData.m_OnlyAttachmentData.m_mapInfo.bIsBlock = TRUE ;
					SendEmailData.m_OnlyAttachmentData.m_bNeedMailap = TRUE;
					return true ;
				}
			}
		}
		else
		{
			SendEmailData.m_OnlyAttachmentData.m_mapInfo.ulMax = 4096  ;
		}


		if( (SendEmailData.m_OnlyAttachmentData.m_mapInfo.ulMin <= (DWORD)SendEmailData.GetRecipientsData().GetRealRecipentsNum()  )&&(SendEmailData.m_OnlyAttachmentData.m_mapInfo.ulMax >= (DWORD)SendEmailData.GetRecipientsData().GetRealRecipentsNum() ) )
		{
			if( _wcsnicmp(pTemp7,strWarn.c_str() ,strWarn.length()) == 0)
			{
				SendEmailData.m_OnlyAttachmentData.m_mapInfo.msgType =  L"Warning" ;
				SendEmailData.m_OnlyAttachmentData.m_mapInfo.url = ( pTemp5 == '\0'? OBLIGATION_URL_DEFAULT: pTemp5);
				SendEmailData.m_OnlyAttachmentData.m_mapInfo.strMessage = (pTemp3 == '\0'? L"Warning for the Mail recipients counts": pTemp3);
				SendEmailData.m_OnlyAttachmentData.m_mapInfo.valid = TRUE;
				SendEmailData.m_OnlyAttachmentData.m_mapInfo.bIsBlock = FALSE ;
				SendEmailData.m_OnlyAttachmentData.m_bNeedMailap = TRUE;
				return true;
			}
			else
			{
				SendEmailData.m_OnlyAttachmentData.m_mapInfo.msgType =  L"Blocked" ;
				SendEmailData.m_OnlyAttachmentData.m_mapInfo.url = ( pTemp5 == '\0'? OBLIGATION_URL_DEFAULT: pTemp5);
				SendEmailData.m_OnlyAttachmentData.m_mapInfo.strMessage = (pTemp3 == '\0'? L"Block for the Mail recipients counts": pTemp3);
				SendEmailData.m_OnlyAttachmentData.m_mapInfo.valid = TRUE;
				SendEmailData.m_OnlyAttachmentData.m_mapInfo.bIsBlock = TRUE ;
				SendEmailData.m_OnlyAttachmentData.m_bNeedMailap = TRUE;
				return true ;
			}
		}
		return true;
	}
	return false;
}



bool CAttachmentObligationData::GetMailNotiFromObl(CEAttributes *obligation,int ObligationPos,CSendEmailData &SendEmailData)
{
	if (obligation->count >  ObligationPos + 15)
	{
		const TCHAR* pTemp3 = NULL;
		const TCHAR* pTemp5 = NULL;
		const TCHAR* pTemp7 = NULL;
		const TCHAR* pTemp9 = NULL;
		const TCHAR* pTemp11 = NULL;
		const TCHAR* pTemp13 = NULL;
		const TCHAR* pTemp15 = NULL;


		pTemp3 = cesdk.fns.CEM_GetString(obligation->attrs[ObligationPos + 3].value); //Result
		pTemp5 = cesdk.fns.CEM_GetString(obligation->attrs[ObligationPos + 5].value); //MinRecipients
		pTemp7 = cesdk.fns.CEM_GetString(obligation->attrs[ObligationPos + 7].value); //MaxRecipients
		pTemp9 = cesdk.fns.CEM_GetString(obligation->attrs[ObligationPos + 9].value);  //Violation
		pTemp11 = cesdk.fns.CEM_GetString(obligation->attrs[ObligationPos + 11].value); //WarningMessage
		pTemp13 = cesdk.fns.CEM_GetString(obligation->attrs[ObligationPos + 13].value); //LogId
		pTemp15 = cesdk.fns.CEM_GetString(obligation->attrs[ObligationPos + 15].value); //Help URL
		if (pTemp3 == NULL || pTemp5 == NULL||pTemp3 == NULL || pTemp9 == NULL||pTemp11 == NULL || pTemp13== NULL || pTemp15 == NULL)
		{
			return false;
		}

		if(pTemp5 != '\0')
		{
			SendEmailData.m_OnlyAttachmentData.m_mailNotiInfo.ulMin = _wtoi(pTemp5);
		}
		else
		{
			SendEmailData.m_OnlyAttachmentData.m_mailNotiInfo.ulMin = 0;
		}

		if(pTemp7 != '\0')
		{
			SendEmailData.m_OnlyAttachmentData.m_mailNotiInfo.ulMax = _wtoi(pTemp7);
		}
		else
		{
			SendEmailData.m_OnlyAttachmentData.m_mailNotiInfo.ulMax = 1024;
		}

		SendEmailData.m_OnlyAttachmentData.m_mailNotiInfo.strWarnType = pTemp3;
		SendEmailData.m_OnlyAttachmentData.m_mailNotiInfo.strViolation = pTemp9;
		SendEmailData.m_OnlyAttachmentData.m_mailNotiInfo.strWMsg = pTemp11;
		SendEmailData.m_OnlyAttachmentData.m_mailNotiInfo.strLogID = pTemp13;
		SendEmailData.m_OnlyAttachmentData.m_mailNotiInfo.strUrl = pTemp15;

		if(SendEmailData.m_OnlyAttachmentData.m_mailNotiInfo.ulMax >= (DWORD)SendEmailData.GetRecipientsData().GetRealRecipentsNum() && SendEmailData.m_OnlyAttachmentData.m_mailNotiInfo.ulMin <= (DWORD)SendEmailData.GetRecipientsData().GetRealRecipentsNum() && !SendEmailData.m_OnlyAttachmentData.m_mailNotiInfo.strLogID.empty() &&
			!SendEmailData.m_OnlyAttachmentData.m_mailNotiInfo.strViolation.empty() && !SendEmailData.m_OnlyAttachmentData.m_mailNotiInfo.strWMsg.empty() && !SendEmailData.m_OnlyAttachmentData.m_mailNotiInfo.strWarnType.empty())
		{
			SendEmailData.m_OnlyAttachmentData.m_bNeedMailNoti = TRUE;
			return true;
		}
		else
		{
			return false;
		}

	}
	return false;
}


bool CAttachmentObligationData::GetInteUseOnlyFromObl(CEAttributes *obligation,int ObligationPos,int nAttachmentPos,CSendEmailData &SendEmailData)
{

	if (obligation->count >  ObligationPos + 3)
	{
		const TCHAR* pTemp3 = NULL;

		pTemp3 = cesdk.fns.CEM_GetString(obligation->attrs[ObligationPos + 3].value); //Help URL
		
		if (pTemp3 == NULL)
		{
			return false;
		}
		SendEmailData.m_OnlyAttachmentData.m_IUOInfo.valid = TRUE;
		SendEmailData.m_OnlyAttachmentData.m_IUOInfo.url = pTemp3;
		if (SendEmailData.GetAttachmentData().size() > (DWORD)nAttachmentPos )
		{
			SendEmailData.GetAttachmentData()[nAttachmentPos].iuo = TRUE;
			size_t len = sizeof(SendEmailData.GetAttachmentData()[nAttachmentPos].iuoUrl)/sizeof(wchar_t);
			wcsncpy_s(SendEmailData.GetAttachmentData()[nAttachmentPos].iuoUrl,len,SendEmailData.m_OnlyAttachmentData.m_IUOInfo.url.c_str(),_TRUNCATE);
			SendEmailData.m_OnlyAttachmentData.m_bNeedIuo = TRUE;
			return true;
		}
		
	}
	return false;
}



bool CAttachmentObligationData::GetVerifyRecipientFromObl(CEAttributes *obligation,int ObligationPos,int nAttachmentPos,CSendEmailData &SendEmailData)
{

	SendEmailData.GetAttachmentData()[nAttachmentPos].er = FALSE;

	if (obligation->count >  ObligationPos + 5)
	{
		const TCHAR* pTemp3 = NULL;
		const TCHAR* pTemp5 = NULL;

		pTemp5 = cesdk.fns.CEM_GetString(obligation->attrs[ObligationPos + 5].value); //Offending Recipients
		pTemp3 = cesdk.fns.CEM_GetString(obligation->attrs[ObligationPos + 3].value); //Help URL
		if (pTemp3 == NULL || pTemp5 == NULL)
		{
			return false;
		}
		SendEmailData.m_OnlyAttachmentData.m_VerifyRecpientsInfo.valid = TRUE;

		if (pTemp5 == '\0')
		{
			SendEmailData.m_OnlyAttachmentData.m_VerifyRecpientsInfo.recipients = L"Missing arg \"Offending Recipients\" from ER obligation";
		}
		else
		{
			SendEmailData.m_OnlyAttachmentData.m_VerifyRecpientsInfo.recipients = pTemp5;
		}

		SendEmailData.m_OnlyAttachmentData.m_VerifyRecpientsInfo.url = pTemp3;
		SendEmailData.m_OnlyAttachmentData.m_bNeedEr=TRUE;
		SendEmailData.GetAttachmentData()[nAttachmentPos].er = TRUE;
		size_t len = sizeof(SendEmailData.GetAttachmentData()[nAttachmentPos].erUrl)/sizeof(wchar_t);
		size_t Relen = sizeof(SendEmailData.GetAttachmentData()[nAttachmentPos].erRecipients)/sizeof(wchar_t);
		wcsncpy_s(SendEmailData.GetAttachmentData()[nAttachmentPos].erUrl,len,SendEmailData.m_OnlyAttachmentData.m_VerifyRecpientsInfo.url.c_str(),_TRUNCATE);
		wcsncpy_s(SendEmailData.GetAttachmentData()[nAttachmentPos].erRecipients,Relen,SendEmailData.m_OnlyAttachmentData.m_VerifyRecpientsInfo.recipients.c_str(),_TRUNCATE);
		
		return true;
	}
	return false;
}




bool CAttachmentObligationData::GetMissTagFromObl(CEAttributes *obligation,int ObligationPos,int nAttachmentPos,CSendEmailData &SendEmailData)
{
	if (obligation->count >  ObligationPos + 9)
	{
		const TCHAR* pTemp3 = NULL;
		const TCHAR* pTemp5 = NULL;
		const TCHAR* pTemp7 = NULL;
		const TCHAR* pTemp9 = NULL;
		

		pTemp3 = cesdk.fns.CEM_GetString(obligation->attrs[ObligationPos + 3].value); //Help URL
		pTemp5 = cesdk.fns.CEM_GetString(obligation->attrs[ObligationPos + 5].value); //Tag Name
		pTemp7 = cesdk.fns.CEM_GetString(obligation->attrs[ObligationPos + 7].value); //Clients
		pTemp9 = cesdk.fns.CEM_GetString(obligation->attrs[ObligationPos + 9].value);  //Unknown Id
	
		if (pTemp3 == NULL || pTemp5 == NULL||pTemp7 == NULL || pTemp9 == NULL)
		{
			return false;
		}

		if (pTemp3 == '\0')
		{
			SendEmailData.m_OnlyAttachmentData.m_MissingTagInfo.url = L""; 
		}
		else
		{
			SendEmailData.m_OnlyAttachmentData.m_MissingTagInfo.url = pTemp3;
		}

		if (pTemp5 == '\0')
		{
			SendEmailData.m_OnlyAttachmentData.m_MissingTagInfo.propertyName = L"Missing arg \" tag name \" from MT obligation";
		}
		else
		{
			SendEmailData.m_OnlyAttachmentData.m_MissingTagInfo.propertyName = pTemp5;
		}


		if (pTemp7 == '\0')
		{
			SendEmailData.m_OnlyAttachmentData.m_MissingTagInfo.clientNames = L"";
		}
		else
		{
			SendEmailData.m_OnlyAttachmentData.m_MissingTagInfo.clientNames = pTemp7;
		}

		if (pTemp9 == '\0')
		{
			SendEmailData.m_OnlyAttachmentData.m_MissingTagInfo.clientIds = L"Missing arg \" Unknown Id \" from MT obligation";
		}
		else
		{
			SendEmailData.m_OnlyAttachmentData.m_MissingTagInfo.clientIds = pTemp9;
		}

		SendEmailData.m_OnlyAttachmentData.m_bNeedMt=TRUE;
	
		wcsncpy_s(SendEmailData.GetAttachmentData()[nAttachmentPos].mtUrl,sizeof(SendEmailData.GetAttachmentData()[nAttachmentPos].mtUrl)/sizeof(wchar_t),SendEmailData.m_OnlyAttachmentData.m_MissingTagInfo.url.c_str(),_TRUNCATE);

		wcsncpy_s(SendEmailData.GetAttachmentData()[nAttachmentPos].mtPropertyName,sizeof(SendEmailData.GetAttachmentData()[nAttachmentPos].mtPropertyName)/sizeof(wchar_t),SendEmailData.m_OnlyAttachmentData.m_MissingTagInfo.propertyName.c_str(),_TRUNCATE);

		wcsncpy_s(SendEmailData.GetAttachmentData()[nAttachmentPos].mtClientIds,sizeof(SendEmailData.GetAttachmentData()[nAttachmentPos].mtClientIds)/sizeof(wchar_t),SendEmailData.m_OnlyAttachmentData.m_MissingTagInfo.clientIds.c_str(),_TRUNCATE);

		wcsncpy_s(SendEmailData.GetAttachmentData()[nAttachmentPos].mtNotClientRelatedId,sizeof(SendEmailData.GetAttachmentData()[nAttachmentPos].mtNotClientRelatedId)/sizeof(wchar_t),SendEmailData.m_OnlyAttachmentData.m_MissingTagInfo.notClientRelatedId.c_str(),_TRUNCATE);
		
		return true;
	}
	return false;
}





bool CAttachmentObligationData::GetMulClientFromObl(CEAttributes *obligation,int ObligationPos,int nAttachmentPos,CSendEmailData &SendEmailData)
{
	SendEmailData.GetAttachmentData()[nAttachmentPos].mc = FALSE;
	if (obligation->count >  ObligationPos + 5)
	{
		const TCHAR* pTemp3 = NULL;
		const TCHAR* pTemp5 = NULL;
		pTemp3 = cesdk.fns.CEM_GetString(obligation->attrs[ObligationPos + 3].value); //Help URL
		pTemp5 = cesdk.fns.CEM_GetString(obligation->attrs[ObligationPos + 5].value); //Tag Name
	
		if (pTemp3 == NULL || pTemp5 == NULL)
		{
			return false;
		}

		if (pTemp3 == '\0')
		{
			SendEmailData.m_OnlyAttachmentData.m_mcInfo.url = L""; 
		}
		else
		{
			SendEmailData.m_OnlyAttachmentData.m_mcInfo.url = pTemp3;
		}

		if (pTemp5 == '\0')
		{
			SendEmailData.m_OnlyAttachmentData.m_mcInfo.recipients = L"Missing arg \" Recipients \" from MC obligation";
		}
		else
		{
			SendEmailData.m_OnlyAttachmentData.m_mcInfo.recipients = pTemp5;
		}
		SendEmailData.m_OnlyAttachmentData.m_bNeedMc = TRUE;
		wcsncpy_s(SendEmailData.GetAttachmentData()[nAttachmentPos].mcUrl,sizeof(SendEmailData.GetAttachmentData()[nAttachmentPos].mcUrl)/sizeof(wchar_t),SendEmailData.m_OnlyAttachmentData.m_mcInfo.url.c_str(),_TRUNCATE);

		wcsncpy_s(SendEmailData.GetAttachmentData()[nAttachmentPos].mcRecipients,sizeof (SendEmailData.GetAttachmentData()[nAttachmentPos].mcRecipients)/sizeof(wchar_t),SendEmailData.m_OnlyAttachmentData.m_mcInfo.recipients.c_str(),_TRUNCATE);
		SendEmailData.GetAttachmentData()[nAttachmentPos].mc = TRUE;
		return true;
	}
	return false;
}


bool CAttachmentObligationData::GetDomainMismatchFromObl(CEAttributes *obligation,int ObligationPos,int nAttachmentPos,CSendEmailData &SendEmailData)
{
	
	SendEmailData.GetAttachmentData()[nAttachmentPos].dm = FALSE;

	if (obligation->count >  ObligationPos + 7)
	{
		const TCHAR* pTemp3 = NULL;
		const TCHAR* pTemp5 = NULL;
		const TCHAR* pTemp7 = NULL;

		pTemp3 = cesdk.fns.CEM_GetString(obligation->attrs[ObligationPos + 3].value); //Help URL
		pTemp5 = cesdk.fns.CEM_GetString(obligation->attrs[ObligationPos + 5].value); //Offending Recipients
		pTemp7 = cesdk.fns.CEM_GetString(obligation->attrs[ObligationPos + 7].value); //Resource Client Name
	
		if (pTemp3 == NULL || pTemp5 == NULL||pTemp7 == NULL)
		{
			return false;
		}

		if (pTemp3 == '\0')
		{
			SendEmailData.m_OnlyAttachmentData.m_dmInfo.url = L""; 
		}
		else
		{
			SendEmailData.m_OnlyAttachmentData.m_dmInfo.url = pTemp3;
		}

		if (pTemp5 == '\0')
		{
			SendEmailData.m_OnlyAttachmentData.m_dmInfo.recipients = L"Missing arg \" Offending Recipients \" from DM obligation";
		}
		else
		{
			SendEmailData.m_OnlyAttachmentData.m_dmInfo.recipients = pTemp5;
		}

		if (pTemp7 == '\0')
		{
			SendEmailData.m_OnlyAttachmentData.m_dmInfo.clientName = L"Missing arg \" Resource Client Name \" from DM obligation";
		}
		else
		{
			SendEmailData.m_OnlyAttachmentData.m_dmInfo.clientName = pTemp7;
		}

		SendEmailData.m_OnlyAttachmentData.m_bNeedDm=TRUE;

		wcsncpy_s(SendEmailData.GetAttachmentData()[nAttachmentPos].dmUrl,sizeof (SendEmailData.GetAttachmentData()[nAttachmentPos].dmUrl)/sizeof(wchar_t),SendEmailData.m_OnlyAttachmentData.m_dmInfo.url.c_str(),_TRUNCATE);
		wcsncpy_s(SendEmailData.GetAttachmentData()[nAttachmentPos].dmRecipients,sizeof (SendEmailData.GetAttachmentData()[nAttachmentPos].dmRecipients)/sizeof(wchar_t),SendEmailData.m_OnlyAttachmentData.m_dmInfo.recipients.c_str(),_TRUNCATE);
		wcsncpy_s(SendEmailData.GetAttachmentData()[nAttachmentPos].dmClientName,sizeof (SendEmailData.GetAttachmentData()[nAttachmentPos].dmClientName)/sizeof(wchar_t),SendEmailData.m_OnlyAttachmentData.m_dmInfo.clientName.c_str(),_TRUNCATE);

		return true;
	}
	return false;
}

bool CAttachmentObligationData::GetHDRFromObl(CEAttributes *obligation,int ObligationPos,int nAttachmentPos,CSendEmailData &SendEmailData)
{
	
	SendEmailData.GetAttachmentData()[nAttachmentPos].hdr = FALSE;

	if (obligation->count >  ObligationPos + 3)
	{
		SendEmailData.m_OnlyAttachmentData.m_bNeedHdr=TRUE;
		SendEmailData.GetAttachmentData()[nAttachmentPos].hdr = TRUE;
	    const wchar_t* wszHelpUrl = cesdk.fns.CEM_GetString(obligation->attrs[ObligationPos + 3].value); //Help URL
		if (wszHelpUrl)
		{
			wcsncpy_s(SendEmailData.GetAttachmentData()[nAttachmentPos].hdrUrl,
				sizeof (SendEmailData.GetAttachmentData()[nAttachmentPos].hdrUrl)/sizeof(wchar_t),
				wszHelpUrl,_TRUNCATE);
		}

		return true;
	}
	return false;
}

bool CAttachmentObligationData::GetFileCustomInterTagFromObl(CEAttributes *obligation,int ObligationPos,VECTOR_TAGPAIR &vecTag)
{
	if (obligation->count >  ObligationPos + 15)
	{
		const TCHAR* pTemp3 = NULL;
		const TCHAR* pTemp5 = NULL;
	
		pTemp3 = cesdk.fns.CEM_GetString(obligation->attrs[ObligationPos + 3].value);
		pTemp5 = cesdk.fns.CEM_GetString(obligation->attrs[ObligationPos + 5].value);

		if (pTemp3 == NULL || pTemp5 == NULL )
		{
			return false;
		}
		
		vecTag.push_back(pair<wstring,wstring>(pTemp3,pTemp5));
		return true;

	}
	return false;
}

bool CAttachmentObligationData::GetWarningMsgFromObl(CEAttributes *obligation,int ObligationPos,int AttachmentNum,CSendEmailData &SendEmailData)
{
	if (obligation->count >  ObligationPos + 11)
	{
		const TCHAR* pTemp3 = NULL;
		const TCHAR* pTemp5 = NULL;
		const TCHAR* pTemp7 = NULL;
		const TCHAR* pTemp9 = NULL;
		const TCHAR* pTemp11 = NULL;

        
		pTemp3 = cesdk.fns.CEM_GetString(obligation->attrs[ObligationPos + 3].value);
		pTemp5 = cesdk.fns.CEM_GetString(obligation->attrs[ObligationPos + 5].value);
		pTemp7 = cesdk.fns.CEM_GetString(obligation->attrs[ObligationPos + 7].value);
		pTemp9 = cesdk.fns.CEM_GetString(obligation->attrs[ObligationPos + 9].value);
		pTemp11 = cesdk.fns.CEM_GetString(obligation->attrs[ObligationPos + 11].value);

		if (pTemp11 == NULL)
		{
			return false;
		}
		
		WARNING_MSG_INFO info;
		info.bDoWarningMsg = true;
		info.strHeaderTxt = (pTemp3 == NULL)? L"":pTemp3;
		info.strDisplayTxt = (pTemp5 == NULL)? L"":pTemp5;
		info.strProceedBtnLabel = (pTemp7 == NULL)? L"Process":pTemp7;
		info.strCancelBtnLabel = (pTemp9 == NULL)? L"Cancel":pTemp9;
		info.strLogID =  pTemp11;
		SendEmailData.GetAttachmentData()[AttachmentNum].SetWaringMsgInfo(info);
		return true;
	}
	return false;
}

bool CAttachmentObligationData::GetRichAlertMsgFromObl(CEAttributes *obligation,int ObligationPos,CSendEmailData &SendEmailData,bool bAllow)
{
    UNREFERENCED_PARAMETER(bAllow);
	if (obligation->count >  ObligationPos + 7)
	{
		const TCHAR* pHeaderTxtForDeny  = cesdk.fns.CEM_GetString(obligation->attrs[ObligationPos + 3].value);
		const TCHAR* pHeaderTxtForAllow  = cesdk.fns.CEM_GetString(obligation->attrs[ObligationPos + 5].value);
		const TCHAR* pAlertMsg  = cesdk.fns.CEM_GetString(obligation->attrs[ObligationPos + 7].value);
		if ((pHeaderTxtForDeny==NULL) || (pHeaderTxtForAllow==NULL) || (pAlertMsg==NULL))
		{
			return false;
		}      
		SendEmailData.SetRichAlertMsgData(pHeaderTxtForAllow, pHeaderTxtForDeny, pAlertMsg);     
		return true;
	}
	return false;
}

bool CAttachmentObligationData::GetAlertInfoFromObl(CEAttributes *obligation,int ObligationPos,CSendEmailData &SendEmailData,bool bAllow)
{
	if (obligation->count >  ObligationPos + 3)
	{
		const TCHAR* pTemp3 = NULL;
		pTemp3 = cesdk.fns.CEM_GetString(obligation->attrs[ObligationPos + 3].value);

		if (pTemp3 == NULL)
		{
			return false;
		}

		wstring strBubble = pTemp3;
		if (!strBubble.empty())
		{
			wstring strAllow = bAllow?L"Allow: ":L"Deny: ";
			strBubble = strAllow + strBubble + L"\n";
			SendEmailData.SetAlertInfo(strBubble,strAllow);
		}
		return true;
	}
	return false;
	
}




bool CAttachmentObligationData::GetFileCustomAutoTagFromObl(CEAttributes *obligation,int ObligationPos,VECTOR_TAGPAIR &vecTag)
{
	if (obligation->count >  ObligationPos + 15)
	{
		const TCHAR* pTemp3 = NULL;
		const TCHAR* pTemp5 = NULL;

		pTemp3 = cesdk.fns.CEM_GetString(obligation->attrs[ObligationPos + 3].value);
		pTemp5 = cesdk.fns.CEM_GetString(obligation->attrs[ObligationPos + 5].value);


		if (pTemp3 == NULL || pTemp5 == NULL )
		{
			return false;
		}
		
		vecTag.push_back(pair<wstring,wstring>(pTemp3,pTemp5));
		return true;

	}
	return false;
}


bool CAttachmentObligationData::GetNxlAutoTagFromObl(CEAttributes *obligation,int ObligationPos,CSendEmailData &SendEmailData,int nAttachmentPos)
{
	if (obligation->count >  ObligationPos + 5)
	{
		const TCHAR* pTemp3 = NULL;
		const TCHAR* pTemp5 = NULL;

		pTemp3 = cesdk.fns.CEM_GetString(obligation->attrs[ObligationPos + 3].value);
		pTemp5 = cesdk.fns.CEM_GetString(obligation->attrs[ObligationPos + 5].value);


		if (pTemp3 == NULL || pTemp5 == NULL )
		{
			return false;
		}
	
		VECTOR_TAGPAIR vec;
		vec.push_back(pair<wstring,wstring>(pTemp3,pTemp5));
		wstring strSourcePath = SendEmailData.GetAttachmentData()[nAttachmentPos].GetSourcePath();
		SendEmailData.GetAttachmentData()[nAttachmentPos].GetAttachmentTagData().SetTempAddedAutoTagNxlTag(vec);
		SendEmailData.GetAttachmentData()[nAttachmentPos].SetAutoNxl(TRUE);
		return true;

	}
	return false;
}


bool CAttachmentObligationData::GetPrependBodyFromObl(CEAttributes *obligation,int ObligationPos,wstring &strBuf)
{
	if (obligation->count >  ObligationPos + 2)
	{
		const TCHAR* pTemp2 = NULL;
		
		pTemp2 = cesdk.fns.CEM_GetString(obligation->attrs[ObligationPos + 2].value);
		

		if (pTemp2 == NULL)
		{
			return false;
		}
		strBuf = pTemp2;
		return true;

	}
	return false;
}

bool CAttachmentObligationData::GetAppendBodyFromObl(CEAttributes *obligation,int ObligationPos,wstring &strBuf)
{
	if (obligation->count >  ObligationPos + 3)
	{
		const TCHAR* pTemp3 = NULL;

		pTemp3 = cesdk.fns.CEM_GetString(obligation->attrs[ObligationPos +3].value);


		if (pTemp3 == NULL)
		{
			return false;
		}
		strBuf = pTemp3;
		return true;

	}
	return false;
}



bool CAttachmentObligationData::GetPrependSubjectFromObl(CEAttributes *obligation,int ObligationPos,wstring &strBuf)
{
	if (obligation->count >  ObligationPos + 2)
	{
		const TCHAR* pTemp2 = NULL;

		pTemp2 = cesdk.fns.CEM_GetString(obligation->attrs[ObligationPos + 2].value);

		if (pTemp2 == NULL)
		{
			return false;
		}
		strBuf = pTemp2;
		return true;

	}
	return false;
}

bool CAttachmentObligationData::GetStripFSFromObl(CEAttributes *obligation,int ObligationPos,CSendEmailData &SendEmailData,AdapterCommon::Attachment& AdapterAttachment)
{
	if (obligation->count >  ObligationPos + 9)
	{
		const TCHAR* pTempObligationName = cesdk.fns.CEM_GetString(obligation->attrs[ObligationPos].value);//FSADAPTER
		const TCHAR* pTempFileServer = cesdk.fns.CEM_GetString(obligation->attrs[ObligationPos + 2].value);//File Server
		const TCHAR* pTempFileServerValue = cesdk.fns.CEM_GetString(obligation->attrs[ObligationPos + 3].value);//\\hz-ts03.nextlabs.com\transfer\bear\stripAttachment
		const TCHAR* pTempLocation = cesdk.fns.CEM_GetString(obligation->attrs[ObligationPos + 4].value);// Location
		const TCHAR* pTempLocationValue = cesdk.fns.CEM_GetString(obligation->attrs[ObligationPos + 5].value);// Bottom
		const TCHAR* pTempText = cesdk.fns.CEM_GetString(obligation->attrs[ObligationPos + 6].value);// Text
		const TCHAR* pTempTextValue = cesdk.fns.CEM_GetString(obligation->attrs[ObligationPos + 7].value);// The attachments [filename] to this message have been removed for security purpose and made available at the following location:[link].
		const TCHAR* pTempLinkFormat = cesdk.fns.CEM_GetString(obligation->attrs[ObligationPos + 8].value);//Link Format
		const TCHAR* pTempLinkFormatValue = cesdk.fns.CEM_GetString(obligation->attrs[ObligationPos + 9].value);// Long


		if (pTempObligationName == NULL || 
			pTempFileServer == NULL || 
			pTempFileServerValue == NULL || 
			pTempLocation == NULL || 
			pTempLocationValue == NULL || 
			pTempText == NULL || 
			pTempTextValue == NULL || 
			pTempLinkFormat == NULL||
			pTempLinkFormatValue == NULL)
		{
			return false;
		}
		AdapterCommon::Obligation ob;
		
		ob.SetName(pTempObligationName);
		ob.AddAttribute(AdapterCommon::Attribute(pTempFileServer,pTempFileServerValue));
		ob.AddAttribute(AdapterCommon::Attribute(pTempLocation,pTempLocationValue));
		ob.AddAttribute(AdapterCommon::Attribute(pTempText,pTempTextValue));
		ob.AddAttribute(AdapterCommon::Attribute(pTempLinkFormat,pTempLinkFormatValue));
		
		SendEmailData.m_OnlyAttachmentData.m_bNeedStripAttachment = TRUE;

		AdapterAttachment.AddObligation(ob);
	}
	return true;
}
bool CAttachmentObligationData::GetStripFTPFromObl(CEAttributes *obligation,int ObligationPos,CSendEmailData &SendEmailData,AdapterCommon::Attachment& AdapterAttachment)
{
	if (obligation->count >  ObligationPos + 27)
	{
		const TCHAR* pTemp0 = cesdk.fns.CEM_GetString(obligation->attrs[ObligationPos].value);//FTPADAPTER
		const TCHAR* pTemp2 = cesdk.fns.CEM_GetString(obligation->attrs[ObligationPos + 2].value);//FTP Server
		const TCHAR* pTemp3 = cesdk.fns.CEM_GetString(obligation->attrs[ObligationPos + 3].value);//10.23.56.103/test
		const TCHAR* pTemp4 = cesdk.fns.CEM_GetString(obligation->attrs[ObligationPos + 4].value);// User
		const TCHAR* pTemp5 = cesdk.fns.CEM_GetString(obligation->attrs[ObligationPos + 5].value);// test
		const TCHAR* pTemp6 = cesdk.fns.CEM_GetString(obligation->attrs[ObligationPos + 6].value);// Password
		const TCHAR* pTemp7 = cesdk.fns.CEM_GetString(obligation->attrs[ObligationPos + 7].value);// 123blue!
		const TCHAR* pTemp8 = cesdk.fns.CEM_GetString(obligation->attrs[ObligationPos + 8].value);// Location
		const TCHAR* pTemp9 = cesdk.fns.CEM_GetString(obligation->attrs[ObligationPos + 9].value);// Bottom
		const TCHAR* pTemp10 = cesdk.fns.CEM_GetString(obligation->attrs[ObligationPos + 10].value);// Text
		const TCHAR* pTemp11 = cesdk.fns.CEM_GetString(obligation->attrs[ObligationPos + 11].value);// The attachments [filename] to this message have been removed for security purpose and made available at the following location:[link].
		const TCHAR* pTemp12 = cesdk.fns.CEM_GetString(obligation->attrs[ObligationPos + 12].value);//Link Format
		const TCHAR* pTemp13 = cesdk.fns.CEM_GetString(obligation->attrs[ObligationPos + 13].value);// Long
		const TCHAR* pTemp14 = cesdk.fns.CEM_GetString(obligation->attrs[ObligationPos + 14].value);// Subject
		const TCHAR* pTemp15 = cesdk.fns.CEM_GetString(obligation->attrs[ObligationPos + 15].value);//SubjectValue
		const TCHAR* pTemp16 = cesdk.fns.CEM_GetString(obligation->attrs[ObligationPos + 16].value);//Body
		const TCHAR* pTemp17 = cesdk.fns.CEM_GetString(obligation->attrs[ObligationPos + 17].value);//body value
		const TCHAR* pTemp18 = cesdk.fns.CEM_GetString(obligation->attrs[ObligationPos + 18].value);// Separate Password in Email
		const TCHAR* pTemp19 = cesdk.fns.CEM_GetString(obligation->attrs[ObligationPos + 19].value);//No
		const TCHAR* pTemp20 = cesdk.fns.CEM_GetString(obligation->attrs[ObligationPos + 20].value);//EFT Admin Port
		const TCHAR* pTemp21 = cesdk.fns.CEM_GetString(obligation->attrs[ObligationPos + 21].value);//port valye
		const TCHAR* pTemp22 = cesdk.fns.CEM_GetString(obligation->attrs[ObligationPos + 22].value);// EFT User Expiry Date (0...65536)
		const TCHAR* pTemp23 = cesdk.fns.CEM_GetString(obligation->attrs[ObligationPos + 23].value);//expiryDate Value
		const TCHAR* pTemp24 = cesdk.fns.CEM_GetString(obligation->attrs[ObligationPos + 24].value);// EFT Site Name
		const TCHAR* pTemp25 = cesdk.fns.CEM_GetString(obligation->attrs[ObligationPos + 25].value);//site value
		const TCHAR* pTemp26=cesdk.fns.CEM_GetString(obligation->attrs[ObligationPos + 26].value);//EFT User Settings Template
		const TCHAR* pTemp27=cesdk.fns.CEM_GetString(obligation->attrs[ObligationPos + 27].value);//Default Settings

		if (pTemp0 == NULL || pTemp2 == NULL || pTemp3 == NULL || pTemp4 == NULL || 
			pTemp6 == NULL || pTemp8 == NULL || pTemp9 == NULL || pTemp10 == NULL||
			pTemp11 == NULL || pTemp12 == NULL || pTemp13 == NULL || pTemp14 == NULL || 
			pTemp16 == NULL || pTemp18 == NULL || pTemp19 == NULL || pTemp20 == NULL ||
		    pTemp22 == NULL || pTemp24 == NULL||pTemp26==NULL)
		{
			return false;
		}

		AdapterCommon::Obligation ob;
		
		ob.SetName(pTemp0);
		ob.AddAttribute(AdapterCommon::Attribute(pTemp2,pTemp3));
		if(pTemp5!=NULL)
			ob.AddAttribute(AdapterCommon::Attribute(pTemp4,pTemp5));
		else
			ob.AddAttribute(AdapterCommon::Attribute(pTemp4,L""));
		if(pTemp7!=NULL)
			ob.AddAttribute(AdapterCommon::Attribute(pTemp6,pTemp7));
		else
			ob.AddAttribute(AdapterCommon::Attribute(pTemp6,L""));
		ob.AddAttribute(AdapterCommon::Attribute(pTemp8,pTemp9));
		ob.AddAttribute(AdapterCommon::Attribute(pTemp10,pTemp11));
		ob.AddAttribute(AdapterCommon::Attribute(pTemp12,pTemp13));
		if(pTemp15!=NULL)
			ob.AddAttribute(AdapterCommon::Attribute(pTemp14,pTemp15));
		else
			ob.AddAttribute(AdapterCommon::Attribute(pTemp14,L""));
		if(pTemp17!=NULL)
			ob.AddAttribute(AdapterCommon::Attribute(pTemp16,pTemp17));
		else
			ob.AddAttribute(AdapterCommon::Attribute(pTemp16,L""));
		ob.AddAttribute(AdapterCommon::Attribute(pTemp18,pTemp19));
		if(pTemp21!=NULL)
			ob.AddAttribute(AdapterCommon::Attribute(pTemp20,pTemp21));
		else
			ob.AddAttribute(AdapterCommon::Attribute(pTemp20,L""));
		if(pTemp23!=NULL)
			ob.AddAttribute(AdapterCommon::Attribute(pTemp22,pTemp23));
		else
			ob.AddAttribute(AdapterCommon::Attribute(pTemp22,L""));
		if(pTemp25!=NULL)
			ob.AddAttribute(AdapterCommon::Attribute(pTemp24,pTemp25));
		else
			ob.AddAttribute(AdapterCommon::Attribute(pTemp24,L""));
		SendEmailData.m_OnlyAttachmentData.m_bNeedStripAttachment = TRUE;
		if(pTemp27!=NULL)
			ob.AddAttribute(AdapterCommon::Attribute(pTemp26,pTemp27));
		else
			ob.AddAttribute(AdapterCommon::Attribute(pTemp26,L""));
		AdapterAttachment.AddObligation(ob);
	}
	return true;
}
int CAttachmentObligationData::GetContentReactionFromObl(CEAttributes *obligation,int ObligationPos,CAOblInfo& ca)
{NLONLY_DEBUG
	if (obligation->count >  ObligationPos + 5)
	{
		const TCHAR* pTemp3 = NULL;
		const TCHAR* pTemp5 = NULL;
		pTemp3 = cesdk.fns.CEM_GetString(obligation->attrs[ObligationPos + 3].value);
		pTemp5 = cesdk.fns.CEM_GetString(obligation->attrs[ObligationPos + 5].value);

		if (pTemp3 == NULL || pTemp5 == NULL || pTemp3 == '\0')
		{
			return 0;
		}
		
		ca.SetCount(_wtol(pTemp3));
        NLPRINT_DEBUGVIEWLOG(L"CA count:[%d]\n", ca.GetCount());

		if (0 == wcsncmp(CONTENTREDACTION_EMAILADDRESS_STR, pTemp5, wcslen(CONTENTREDACTION_EMAILADDRESS_STR)))
		{
			if (ca.GetCAType() == emBODY)
			{
				return CONTENTREDACTION_EMAILADDRESS_BODY;
			}
			else
			{
				return CONTENTREDACTION_EMAILADDRESS_SUBJECT;
			}
		}
		
		if (0 == wcsncmp(L"CCN", pTemp5, wcslen(L"CCN")))
		{
			if (ca.GetCAType() == emBODY)
			{
				return CONTENTREDACTION_CREDITCARDNUM_BODY;
			}
			else
			{
				return CONTENTREDACTION_CREDITCARDNUM_SUBJECT;
			}
		}

		if (0 == wcsncmp(CONTENTREDACTION_CURRENCYVALUE_STR, pTemp5, wcslen(CONTENTREDACTION_CURRENCYVALUE_STR)))
		{
			if (ca.GetCAType() == emBODY)
			{
				return CONTENTREDACTION_CURRENCYVALUE_BODY;
			}
			else
			{
				return CONTENTREDACTION_CURRENCYVALUE_SUBJECT;
			}
		}
		

		if (0 == wcsncmp(CONTENTREDACTION_PHONENUMBER_STR, pTemp5, wcslen(CONTENTREDACTION_PHONENUMBER_STR)))
		{
			if (ca.GetCAType() == emBODY)
			{
				return CONTENTREDACTION_PHONENUMBER_BODY;
			}
			else
			{
				return CONTENTREDACTION_PHONENUMBER_SUBJECT;
			}
		}

		if (0 == wcsncmp(L"SSN", pTemp5, wcslen(L"SSN")))
		{
			if (ca.GetCAType() == emBODY)
			{
				return CONTENTREDACTION_SOCIALSECURITYNUM_BODY;
			}
			else
			{
				return CONTENTREDACTION_SOCIALSECURITYNUM_SUBJECT;
			}
		}

		if (0 == wcsncmp(CONTENTREDACTION_IPV4Address_STR, pTemp5, wcslen(CONTENTREDACTION_IPV4Address_STR)))
		{
			if (ca.GetCAType() == emBODY)
			{
				return CONTENTREDACTION_IPV4Address_BODY;
			}
			else
			{
				return CONTENTREDACTION_IPV4Address_SUBJECT;
			}
		}

		if (0 == wcsncmp(CONTENTREDACTION_DOB_STR, pTemp5, wcslen(CONTENTREDACTION_DOB_STR)))
		{
			if (ca.GetCAType() == emBODY)
			{
				return CONTENTREDACTION_DOB_BODY;
			}
			else
			{
				return CONTENTREDACTION_DOB_SUBJECT;
			}
		}

		if (0 == wcsncmp(CONTENTREDACTION_MAILINGADDRESS_STR, pTemp5, wcslen(CONTENTREDACTION_MAILINGADDRESS_STR)))
		{
			if (ca.GetCAType() == emBODY)
			{
				return CONTENTREDACTION_MAILINGADDRESS_BODY;
			}
			else
			{
				return CONTENTREDACTION_MAILINGADDRESS_SUBJECT;
			}
		}
		
		if (0 == wcsncmp(CONTENTREDACTION_KEYWORD_STR, pTemp5, wcslen(CONTENTREDACTION_KEYWORD_STR)))
		{
			if (obligation->count >  ObligationPos + 7)
			{
				const TCHAR* pTemp7 = NULL;
				pTemp7 = cesdk.fns.CEM_GetString(obligation->attrs[ObligationPos + 7].value);

				if (pTemp7 == NULL || pTemp7 == '\0')
				{
					return 0;
				}
				wstring strValue = pTemp7;
				ca.SetValue(strValue);
				
				if (ca.GetCAType() == emBODY)
				{
					return CONTENTREDACTION_KEYWORD_BODY;
				}
				else
				{
					return CONTENTREDACTION_KEYWORD_SUBJECT;
				}
			}
		}
	}
	return 0;
}


bool CAttachmentObligationData::GetRichUserMsg(CEAttributes *obligation,int ObligationPos,CSendEmailData &SendEmailData)
{
	if (obligation->count >  ObligationPos + 5)
	{
		const TCHAR* pTemp3 = NULL;
		const TCHAR* pTemp5 = NULL;

		pTemp3 = cesdk.fns.CEM_GetString(obligation->attrs[ObligationPos + 3].value);
		pTemp5 = cesdk.fns.CEM_GetString(obligation->attrs[ObligationPos + 5].value);

		if (pTemp3 == NULL || pTemp5 == NULL )
		{
			return false;
		}
		SendEmailData.m_OnlyAttachmentData.m_strURMsgTxt = pTemp3;
		SendEmailData.m_OnlyAttachmentData.m_strURMsgTimeOut = pTemp5;
		SendEmailData.m_OnlyAttachmentData.m_bNeedURMsg = TRUE;
		return true;
	}
	return false;
}

EXCUTEOBLACTION CAttachmentObligationData::ExcuteRichUserMsg(CSendEmailData &SendEmailData)
{
	if (SendEmailData.m_OnlyAttachmentData.m_bNeedURMsg)
	{
		m_NotificationBubble.ShowBubble(SendEmailData.m_OnlyAttachmentData.m_strURMsgTxt.c_str(),1000*_wtol(SendEmailData.m_OnlyAttachmentData.m_strURMsgTimeOut.c_str()));
	}
	return EXCUTESUCCESS;
}

EXCUTEOBLACTION CAttachmentObligationData::ExcutePrependSubject(CSendEmailData &SendEmailData)
{
	map<wstring,bool>& mapPrependSubject = SendEmailData.GetSubjectData().GetPrependSubject();
	wstring strTempSubject;
	for (map<wstring,bool>::iterator itor = mapPrependSubject.begin(); itor != mapPrependSubject.end(); itor++)
	{
		if (!itor->second)
		{
			strTempSubject = itor->first + SendEmailData.GetSubjectData().GetTempSubject();
			SendEmailData.GetSubjectData().SetTempSubject(strTempSubject.c_str());
			SendEmailData.GetSubjectData().SetSubjectChanged(true);
			SendEmailData.SetEvalAgain(true);
			itor->second = true;
		}
	}
	return EXCUTESUCCESS;
}
EXCUTEOBLACTION CAttachmentObligationData::ExcutePrependBody(CSendEmailData &SendEmailData)
{
	map<wstring,bool>& mapPrependBody = SendEmailData.GetBodyData().GetPrependBody();
	wstring strTempBody;
	wstring strPrependBody;
	bool bFormat = false;
	for (map<wstring,bool>::iterator itor = mapPrependBody.begin(); itor != mapPrependBody.end(); itor++)
	{
		if (!itor->second)
		{
			strPrependBody = itor->first;
			strTempBody = SendEmailData.GetBodyData().GetTempBody();
			if (SendEmailData.GetBodyData().GetBodyFormat() == olFormatHTML)
			{
				boost::algorithm::replace_all(strPrependBody, L"\\n", L"<br>");
				strPrependBody.append(L"<br>");
			    bFormat = OLUtilities::PreAppendHtml(strTempBody, strPrependBody);
				if (!bFormat)
				{
					strTempBody = strPrependBody + strTempBody;
				}
			}
			else
			{
				boost::algorithm::replace_all(strPrependBody, L"\\n", L"\n");
				strPrependBody.append(L"\n");
				strTempBody = strPrependBody + strTempBody;
			}
			
			SendEmailData.GetBodyData().SetTempBody(strTempBody.c_str());
			SendEmailData.GetBodyData().SetBodyChanged(true);
			SendEmailData.SetEvalAgain(true);
			itor->second = true;
		}
	}
	return EXCUTESUCCESS;
}
EXCUTEOBLACTION CAttachmentObligationData::ExcuteAppendBody(CSendEmailData &SendEmailData)
{
	map<wstring,bool>& mapAppendBody = SendEmailData.GetBodyData().GetAppendBody();
	wstring strTempBody;
	wstring strApendBody;
	bool bFirstLine = true;
	wstring strHtmlWrap = L"<br>";
	wstring strNormalWrap = L"\n";
	for (map<wstring,bool>::iterator itor = mapAppendBody.begin(); itor != mapAppendBody.end(); itor++)
	{
		if (!itor->second)
		{
			strApendBody = itor->first;
			if (SendEmailData.GetBodyData().GetBodyFormat() == olFormatHTML)
			{
				boost::algorithm::replace_all(strApendBody, L"\\n", L"<br>");
				if (bFirstLine)
				{
					strApendBody =strHtmlWrap + strApendBody;
					bFirstLine = false;
				}
				strApendBody.append(L"<br>");
			}
			else
			{
				boost::algorithm::replace_all(strApendBody, L"\\n", L"\n");
				if (bFirstLine)
				{
					strApendBody =strNormalWrap + strApendBody;
					bFirstLine = false;
				}
				strApendBody.append(L"\n");
				
			}

			strTempBody = SendEmailData.GetBodyData().GetTempBody() + strApendBody;
			SendEmailData.GetBodyData().SetTempBody(strTempBody.c_str());
			SendEmailData.GetBodyData().SetBodyChanged(true);
			SendEmailData.SetEvalAgain(true);
			itor->second = true;
		}
	}
	return EXCUTESUCCESS;
}


EXCUTEOBLACTION CAttachmentObligationData::ExcuteContentReactionBody(CSendEmailData &SendEmailData)
{NLONLY_DEBUG
	CARegExpressions regExps;
	CARegExpression camailAddress;
	CARegExpression caCreditCardNum;
	CARegExpression caCurrencyValue;
	CARegExpression caPhoneNum;
	CARegExpression caSocialSecurityNum;
	CARegExpression caIPAddress;
	CARegExpression caEmailAddress;
	CARegExpression caDOB;
	CAAdapter adapter;
	

	int nType = SendEmailData.GetBodyData().GetExistObligationType();
    NLPRINT_DEBUGVIEWLOG(L"Olbigation type:[0x%x]\n", nType);
	if (nType & CONTENTREDACTION_EMAILADDRESS_BODY)
	{
		if(!SendEmailData.GetBodyData().GetCAOblBodyData().GetEmailAddress_body().GetDone())
		{
			camailAddress.m_MatchCount = SendEmailData.GetBodyData().GetCAOblBodyData().GetEmailAddress_body().GetCount();
			camailAddress.m_Regex = CONTENTREDACTION_EMAILADDRESS_STR;
			SendEmailData.GetBodyData().GetCAOblBodyData().GetEmailAddress_body().SetDone(true);
			regExps.push_back(&camailAddress);
		}
	}


	if (nType & CONTENTREDACTION_CREDITCARDNUM_BODY)
	{
		if(!SendEmailData.GetBodyData().GetCAOblBodyData().GetCreditCardNum_body().GetDone())
		{
			caCreditCardNum.m_MatchCount = SendEmailData.GetBodyData().GetCAOblBodyData().GetCreditCardNum_body().GetCount();
			caCreditCardNum.m_Regex = CONTENTREDACTION_CREDITCARDNUM_STR;
			caCreditCardNum.m_RegexType = L"CCN";
			SendEmailData.GetBodyData().GetCAOblBodyData().GetCreditCardNum_body().SetDone(true);
			regExps.push_back(&caCreditCardNum);
		}
	}
	
	if (nType & CONTENTREDACTION_CURRENCYVALUE_BODY)
	{
		if(!SendEmailData.GetBodyData().GetCAOblBodyData().GetCurrencyValue_body().GetDone())
		{
			caCurrencyValue.m_MatchCount = SendEmailData.GetBodyData().GetCAOblBodyData().GetCurrencyValue_body().GetCount();
			caCurrencyValue.m_Regex = CONTENTREDACTION_CURRENCYVALUE_STR;
			SendEmailData.GetBodyData().GetCAOblBodyData().GetCurrencyValue_body().SetDone(true);
			regExps.push_back(&caCurrencyValue);
		}
	}

	if (nType & CONTENTREDACTION_PHONENUMBER_BODY)
	{
		if(!SendEmailData.GetBodyData().GetCAOblBodyData().GetPhoneNum_body().GetDone())
		{
			caPhoneNum.m_MatchCount = SendEmailData.GetBodyData().GetCAOblBodyData().GetPhoneNum_body().GetCount();
			caPhoneNum.m_Regex = CONTENTREDACTION_PHONENUMBER_STR;
			SendEmailData.GetBodyData().GetCAOblBodyData().GetPhoneNum_body().SetDone(true);
			regExps.push_back(&caPhoneNum);
		}
	}

	if (nType & CONTENTREDACTION_SOCIALSECURITYNUM_BODY)
	{
		if(!SendEmailData.GetBodyData().GetCAOblBodyData().GetSocialSecurityNum_body().GetDone())
		{
			caSocialSecurityNum.m_MatchCount = SendEmailData.GetBodyData().GetCAOblBodyData().GetSocialSecurityNum_body().GetCount();
			caSocialSecurityNum.m_Regex = CONTENTREDACTION_SOCIALSECURITYNUM_STR;
			caSocialSecurityNum.m_RegexType = L"SSN";
			SendEmailData.GetBodyData().GetCAOblBodyData().GetSocialSecurityNum_body().SetDone(true);
			regExps.push_back(&caSocialSecurityNum);
		}
	}

	if (nType & CONTENTREDACTION_IPV4Address_BODY)
	{
		if(!SendEmailData.GetBodyData().GetCAOblBodyData().GetIPAddress_body().GetDone())
		{
			caIPAddress.m_MatchCount = SendEmailData.GetBodyData().GetCAOblBodyData().GetIPAddress_body().GetCount();
			caIPAddress.m_Regex = CONTENTREDACTION_IPV4Address_STR;
			SendEmailData.GetBodyData().GetCAOblBodyData().GetIPAddress_body().SetDone(true);
			regExps.push_back(&caIPAddress);
		}
	}

	if (nType & CONTENTREDACTION_MAILINGADDRESS_BODY)
	{
		if(!SendEmailData.GetBodyData().GetCAOblBodyData().GetMailingAdd_body().GetDone())
		{
			caEmailAddress.m_MatchCount = SendEmailData.GetBodyData().GetCAOblBodyData().GetMailingAdd_body().GetCount();
			caEmailAddress.m_Regex = CONTENTREDACTION_MAILINGADDRESS_STR;
			SendEmailData.GetBodyData().GetCAOblBodyData().GetMailingAdd_body().SetDone(true);
			regExps.push_back(&caEmailAddress);
		}
	}

	if (nType & CONTENTREDACTION_DOB_BODY)
	{
		if(!SendEmailData.GetBodyData().GetCAOblBodyData().GetDOB_body().GetDone())
		{
			caDOB.m_MatchCount = SendEmailData.GetBodyData().GetCAOblBodyData().GetDOB_body().GetCount();
			caDOB.m_Regex = CONTENTREDACTION_DOB_STR;
			SendEmailData.GetBodyData().GetCAOblBodyData().GetDOB_body().SetDone(true);
			regExps.push_back(&caDOB);
		}
	}

	
	
	vector<CAOblInfo> vecCA = SendEmailData.GetBodyData().GetCAOblBodyData().GetKeyWord_body();
	size_t nKeyWordSize = vecCA.size();
	vector<CARegExpression> vecCAKeyWord(nKeyWordSize);

	if (nType & CONTENTREDACTION_KEYWORD_BODY)
	{
		for (DWORD i = 0; i < nKeyWordSize; i++)
		{
			if (!vecCA[i].GetDone())
			{
				vecCAKeyWord[i].m_MatchCount = vecCA[i].GetCount();
				vecCAKeyWord[i].m_RegexType = NL_CA_REDACTIONOBLIGATION_KEYWORD;
				vecCAKeyWord[i].m_Regex = vecCA[i].GetValue();
				vecCA[i].SetDone(true);
				regExps.push_back(&vecCAKeyWord[i]);	
			}
		}
	}

	if (regExps.size() > 0)
	{
		wstring strTemp = SendEmailData.GetBodyData().GetTempBody();
		if (!strTemp.empty())
		{
			bool bChanged = false;
			map<wstring,wstring> & mapReplaceInfo = SendEmailData.GetBodyData().GetReplaceInfo();
			if(adapter.RedactBufferBody( strTemp, regExps , bChanged, mapReplaceInfo )==TRUE&&bChanged==true)
			{
				SendEmailData.GetBodyData().SetBodyChanged(true);
				SendEmailData.GetBodyData().SetTempBody(strTemp.c_str());
				SendEmailData.SetEvalAgain(true);
			}
		}
	}
	return EXCUTESUCCESS;
}

EXCUTEOBLACTION CAttachmentObligationData::ExcuteContentReactionSubject(CSendEmailData &SendEmailData)
{NLONLY_DEBUG
	CARegExpressions regExps;
	CARegExpression camailAddress;
	CARegExpression caCreditCardNum;
	CARegExpression caCurrencyValue;
	CARegExpression caPhoneNum;
	CARegExpression caSocialSecurityNum;
	CARegExpression caIPAddress;
	CARegExpression caEmailAddress;
	CARegExpression caDOB;
	CAAdapter adapter;


	int nType = SendEmailData.GetSubjectData().GetExistObligationType();
    NLPRINT_DEBUGVIEWLOG(L"nType:[%d]\n", nType);
	if (nType & CONTENTREDACTION_EMAILADDRESS_SUBJECT)
	{
		if(!SendEmailData.GetSubjectData().GetCAOblSubjectData().GetEmailAddress_suject().GetDone())
		{
			camailAddress.m_MatchCount = SendEmailData.GetSubjectData().GetCAOblSubjectData().GetEmailAddress_suject().GetCount();
			camailAddress.m_Regex = CONTENTREDACTION_EMAILADDRESS_STR;
			SendEmailData.GetSubjectData().GetCAOblSubjectData().GetEmailAddress_suject().SetDone(true);
			regExps.push_back(&camailAddress);
		}
	}


	if (nType & CONTENTREDACTION_CREDITCARDNUM_SUBJECT)
	{
		if(!SendEmailData.GetSubjectData().GetCAOblSubjectData().GetCreditCardNum_suject().GetDone())
		{
			caCreditCardNum.m_MatchCount = SendEmailData.GetSubjectData().GetCAOblSubjectData().GetCreditCardNum_suject().GetCount();
			caCreditCardNum.m_Regex = CONTENTREDACTION_CREDITCARDNUM_STR;
			caCreditCardNum.m_RegexType = L"CCN";
			SendEmailData.GetSubjectData().GetCAOblSubjectData().GetCreditCardNum_suject().SetDone(true);
			regExps.push_back(&caCreditCardNum);
		}
	}

	if (nType & CONTENTREDACTION_CURRENCYVALUE_SUBJECT)
	{
		if(!SendEmailData.GetSubjectData().GetCAOblSubjectData().GetCurrencyValue_suject().GetDone())
		{
			caCurrencyValue.m_MatchCount = SendEmailData.GetSubjectData().GetCAOblSubjectData().GetCurrencyValue_suject().GetCount();
			caCurrencyValue.m_Regex = CONTENTREDACTION_CURRENCYVALUE_STR;
			SendEmailData.GetSubjectData().GetCAOblSubjectData().GetCurrencyValue_suject().SetDone(true);
			regExps.push_back(&caCurrencyValue);
		}
	}

	if (nType & CONTENTREDACTION_PHONENUMBER_SUBJECT)
	{
		if(!SendEmailData.GetSubjectData().GetCAOblSubjectData().GetPhoneNum_suject().GetDone())
		{
			caPhoneNum.m_MatchCount = SendEmailData.GetSubjectData().GetCAOblSubjectData().GetPhoneNum_suject().GetCount();
			caPhoneNum.m_Regex = CONTENTREDACTION_PHONENUMBER_STR;
			SendEmailData.GetSubjectData().GetCAOblSubjectData().GetPhoneNum_suject().SetDone(true);
			regExps.push_back(&caPhoneNum);
		}
	}

	if (nType & CONTENTREDACTION_SOCIALSECURITYNUM_SUBJECT)
	{
		if(!SendEmailData.GetSubjectData().GetCAOblSubjectData().GetSocialSecurityNum_suject().GetDone())
		{
			caSocialSecurityNum.m_MatchCount = SendEmailData.GetSubjectData().GetCAOblSubjectData().GetSocialSecurityNum_suject().GetCount();
			caSocialSecurityNum.m_Regex = CONTENTREDACTION_SOCIALSECURITYNUM_STR;
			caSocialSecurityNum.m_RegexType = L"SSN";
			SendEmailData.GetSubjectData().GetCAOblSubjectData().GetSocialSecurityNum_suject().SetDone(true);
			regExps.push_back(&caSocialSecurityNum);
		}
	}

	if (nType & CONTENTREDACTION_IPV4Address_SUBJECT)
	{
		if(!SendEmailData.GetSubjectData().GetCAOblSubjectData().GetIPAddress_suject().GetDone())
		{
			caIPAddress.m_MatchCount = SendEmailData.GetSubjectData().GetCAOblSubjectData().GetIPAddress_suject().GetCount();
			caIPAddress.m_Regex = CONTENTREDACTION_IPV4Address_STR;
			SendEmailData.GetSubjectData().GetCAOblSubjectData().GetIPAddress_suject().SetDone(true);
			regExps.push_back(&caIPAddress);
		}
	}

	if (nType & CONTENTREDACTION_MAILINGADDRESS_SUBJECT)
	{
		if(!SendEmailData.GetSubjectData().GetCAOblSubjectData().GetMailingAdd_suject().GetDone())
		{
			caEmailAddress.m_MatchCount = SendEmailData.GetSubjectData().GetCAOblSubjectData().GetMailingAdd_suject().GetCount();
			caEmailAddress.m_Regex = CONTENTREDACTION_MAILINGADDRESS_STR;
			SendEmailData.GetSubjectData().GetCAOblSubjectData().GetMailingAdd_suject().SetDone(true);
			regExps.push_back(&caEmailAddress);
		}
	}

	if (nType & CONTENTREDACTION_DOB_SUBJECT)
	{
		if(!SendEmailData.GetSubjectData().GetCAOblSubjectData().GetDOB_suject().GetDone())
		{
			caDOB.m_MatchCount = SendEmailData.GetSubjectData().GetCAOblSubjectData().GetDOB_suject().GetCount();
			caDOB.m_Regex = CONTENTREDACTION_DOB_STR;
			SendEmailData.GetSubjectData().GetCAOblSubjectData().GetDOB_suject().SetDone(true);
			regExps.push_back(&caDOB);
		}
	}



	vector<CAOblInfo> vecCA = SendEmailData.GetSubjectData().GetCAOblSubjectData().GetKeyWord_suject();
	size_t nKeyWordSize = vecCA.size();
	vector<CARegExpression> vecCAKeyWord(nKeyWordSize);

	if (nType & CONTENTREDACTION_KEYWORD_SUBJECT)
	{
		for (DWORD i = 0; i < nKeyWordSize; i++)
		{
			if (!vecCA[i].GetDone())
			{
				vecCAKeyWord[i].m_MatchCount = vecCA[i].GetCount();
				vecCAKeyWord[i].m_RegexType = NL_CA_REDACTIONOBLIGATION_KEYWORD;
				vecCAKeyWord[i].m_Regex = vecCA[i].GetValue();
				vecCA[i].SetDone(true);
				regExps.push_back(&vecCAKeyWord[i]);	
			}
		}
	}

	if (regExps.size() > 0)
	{
		wstring strTemp = SendEmailData.GetSubjectData().GetTempSubject();
		if (!strTemp.empty())
		{
			bool bChanged = false;
			if(adapter.RedactBuffer( strTemp, regExps , bChanged )==TRUE&&bChanged==true)
			{
				SendEmailData.GetSubjectData().SetSubjectChanged(true);
				SendEmailData.GetSubjectData().SetTempSubject(strTemp.c_str());
				SendEmailData.SetEvalAgain(true);
			}
		}
	}
	return EXCUTESUCCESS;
}






EXCUTEOBLACTION CAttachmentObligationData::ExcuteMailNotificationEx(CSendEmailData &SendEmailData)
{
	if(SendEmailData.m_OnlyAttachmentData.m_bNeedMailNoti)
	{
		CYeledDlg theDlg(SendEmailData.m_OnlyAttachmentData.m_mailNotiInfo);
		theDlg.DoNotification(SendEmailData.GetWnd());
		if(!theDlg.IsSend())
		{
			return EXCUTE_CANCEL_MAYBE_LOG_DESIGN;	
		}
		SendEmailData.SetEvalAgain(true);
	}
	return EXCUTESUCCESS;
}


EXCUTEOBLACTION CAttachmentObligationData::ExcuteInternetUseOnlyEx(CSendEmailData &SendEmailData)
{
	if (SendEmailData.m_OnlyAttachmentData.m_bNeedIuo)
	{
		CInterAssistDlg iuoDlg;
		iuoDlg.SetLastFlag();

		// We use the Help URL from the first matching attachment only.
		BOOL bHasHelpUrl = FALSE;

		for (DWORD i = 0; i < SendEmailData.GetAttachmentData().size(); i++)
		{
			if (SendEmailData.GetAttachmentData()[i].iuo)
			{
				iuoDlg.AddInterItemData(SendEmailData.GetAttachmentData()[i].GetSourcePath().c_str());

				if (!bHasHelpUrl && SendEmailData.GetAttachmentData()[i].iuoUrl[0] != L'\0')
				{
					iuoDlg.SetHelpUrl(SendEmailData.GetAttachmentData()[i].iuoUrl);
					bHasHelpUrl = TRUE;
				}
			}
		}

		iuoDlg.DoModal(SendEmailData.GetWnd());
		SendEmailData.SetEvalAgain(true);
	}
	return EXCUTESUCCESS;
}



EXCUTEOBLACTION CAttachmentObligationData::ExcuteHelpURLFirstMatchAttachmentEx(CSendEmailData &SendEmailData)
{
	if (SendEmailData.m_OnlyAttachmentData.m_bNeedMt)
	{
		// We use the Help URL from the first matching attachment only.
		BOOL bHasHelpUrl = FALSE;
		CTagAssistDlgEx::CONTEXT context = 
		{
			&(SendEmailData.GetAttachmentData())
		};
		CTagAssistDlgEx mtDlg(&context);
		if (!(SendEmailData.m_OnlyAttachmentData.m_bNeedDm || SendEmailData.m_OnlyAttachmentData.m_bNeedMc || SendEmailData.m_OnlyAttachmentData.m_bNeedEr) && !SendEmailData.m_OnlyAttachmentData.m_bNeedHdr)
			mtDlg.SetLastFlag();
		// add data
		for (DWORD i = 0; i<SendEmailData.GetAttachmentData().size(); i++)
		{
		
			if (SendEmailData.GetAttachmentData()[i].mt)
			{
				std::wstring tmp;

				tmp = TAG_NOT_CLIENT_RELATED;
				if (wcslen(SendEmailData.GetAttachmentData()[i].mtClientNames) > 0)
				{
					tmp += OBLIGATION_STR_DELIM;
					tmp += SendEmailData.GetAttachmentData()[i].mtClientNames;
				}
				mtDlg.AddTagItemData(SendEmailData.GetAttachmentData()[i].GetSourcePath().c_str(), tmp.c_str());
				if (!bHasHelpUrl && SendEmailData.GetAttachmentData()[i].mtUrl[0] != L'\0')
				{
					/*
					Modified by chellee for the bug "?" BUG ID:10200 
					*/
					mtDlg.SetHelpUrl(SendEmailData.GetAttachmentData()[i].mtUrl);
					bHasHelpUrl = TRUE;
				}
			}
		}

		// Show dialog
		LRESULT lResult = mtDlg.DoModal(SendEmailData.GetWnd());
		if (IDOK == lResult)
		{
			// Re-check policies.
			return EXCUTE_CHECK_POLICY;
		}
		else
		{
			return EXCUTE_CANCEL_MAYBE_LOG_DESIGN;
		}
	}
	return EXCUTESUCCESS;
}

EXCUTEOBLACTION CAttachmentObligationData::ExcuteEmailVerifyEx(CSendEmailData &SendEmailData)
{
	if(SendEmailData.m_OnlyAttachmentData.m_bNeedDm || SendEmailData.m_OnlyAttachmentData.m_bNeedMc
		||SendEmailData.m_OnlyAttachmentData.m_bNeedEr|| SendEmailData.m_OnlyAttachmentData.m_bNeedCrl)
	{

		std::vector<int> listRecipientIndices;
		std::vector<BOOL> listRecipientDmFlags;
		std::vector<BOOL> listRecipientMcFlags;
		STRINGLIST		  TemplistRecipients;
		int				  nTempRecipients;
		BOOL              bExistDefaultRecipients = FALSE;
		BOOL			  bSenderInShowDenyReci = FALSE;
		BOOL			  bSenderInRecipients = FALSE;// for all email type 
		
		if (SendEmailData.m_OnlyAttachmentData.m_bNeedCrl)
		{
			TemplistRecipients = SendEmailData.GetRecipientsData().GetShowDenyRecipients();
			nTempRecipients = (int)SendEmailData.GetRecipientsData().GetShowDenyRecipients().size();

            // fix the bug of 35450 if there are duplicated recipient.

            const STRINGLIST& vecRealList1 = SendEmailData.GetRecipientsData().GetRealRecipients();
            STRINGLIST vecRealList2 = SendEmailData.GetRecipientsData().GetRealRecipients();
            int nRealNumb = 0;
            for (STRINGLIST::const_iterator it = vecRealList1.begin(); it != vecRealList1.end();it++)
            {
                int nFindTwice = 0;
                for (STRINGLIST::iterator itt = vecRealList2.begin(); itt != vecRealList2.end(); )
                {
                    if (_wcsicmp(it->c_str(), itt->c_str()) == 0 && nFindTwice++>0)
                    {
                        itt = vecRealList2.erase(itt);
                    }
                    else itt++;
                }
            }
            nRealNumb = (int)vecRealList2.size();
			std::wstring strCurrentSender = SendEmailData.GetSender();
			for (STRINGLIST::const_iterator it = vecRealList2.begin(); it != vecRealList2.end();)
			{
				if (_wcsicmp(it->c_str(), strCurrentSender.c_str()) == 0)
				{
					bSenderInRecipients = TRUE;
					logd(L"[ExcuteEmailVerifyEx]sender in real recipients.");
					break;
				}
				else
					it++;
			}
			for (STRINGLIST::const_iterator it = TemplistRecipients.begin(); it != TemplistRecipients.end();)
			{
				if (_wcsicmp(it->c_str(), strCurrentSender.c_str()) == 0)
				{
					bSenderInShowDenyReci = TRUE;
					logd(L"[ExcuteEmailVerifyEx]sender in show deny list.");
					break;
				}
				else
					it++;
			}

			//SendEmailData.GetRecipientsData().GetSenderInRecipient() only be avaiable when email type is meeting or appointment
			//if (!(SendEmailData.GetRecipientsData().GetSenderInRecipient().empty()) && !bSenderInShowDenyReci)//sender in recipients but not match "reject until silent"
			if (bSenderInRecipients && !bSenderInShowDenyReci)//sender in recipients but not match "reject until silent"
			{
				if (nTempRecipients+1 < nRealNumb) bExistDefaultRecipients = TRUE;//besides the sender(also is recipient), if other recipients are all in reject list,"send email" button will be gray
			}
			else
			{
				if (nTempRecipients < nRealNumb)
				{
					bExistDefaultRecipients = TRUE;//"send email" not gray
				}
			}
           
			/*
				send to sender himself, and himself is the only recipient; the default action: set allow=true, allow to send to himself
			*/
			if ( 1 == nTempRecipients && SendEmailData.IsEqualSender(TemplistRecipients[0]) )
			{
				if (!SendEmailData.m_OnlyAttachmentData.m_bNeedCrl)
				{
					SendEmailData.SetEvalAgain(true);
				}
				else
				{
					logd(L"[ExcuteEmailVerifyEx]Reset the SendEmailData allow value(old_%d) to true", SendEmailData.GetAllow());
					SendEmailData.SetAllow(true);
					SendEmailData.AllowOfRejectUnlessSilent(true);
				}
				return EXCUTESUCCESS;
			}
			else
			{
				for (STRINGLIST::const_iterator it = TemplistRecipients.begin(); it != TemplistRecipients.end();)
				{
					if (_wcsicmp(it->c_str(), strCurrentSender.c_str()) == 0)
					{
						it = TemplistRecipients.erase(it);//do not show it in reject until silent list
						logd(L"[ExcuteEmailVerifyEx]erase show deny recipient which equals to sender.");
					}
					else
						it++;
				}
				nTempRecipients = (int)TemplistRecipients.size();
			}
		}
		else
		{
			TemplistRecipients = SendEmailData.GetRecipientsData().GetRealRecipients();
			nTempRecipients = (int)SendEmailData.GetRecipientsData().GetRealRecipients().size();
		}

		int nAttachments = (int)SendEmailData.GetAttachmentData().size();

		CDmAssistDlgEx::CONTEXT context = {
			&TemplistRecipients,
			&nTempRecipients,
			&listRecipientIndices,
			&listRecipientDmFlags,
			&listRecipientMcFlags,
			&SendEmailData.GetAttachmentData(),
			&nAttachments,
			&SendEmailData.m_OnlyAttachmentData.m_bNeedHdr,
			SendEmailData.m_OnlyAttachmentData.m_bNeedCrl,
			SendEmailData.m_OnlyAttachmentData.m_rlInfo.strMessage,
			SendEmailData.m_OnlyAttachmentData.m_rlInfo.bAllowOverride,
			bExistDefaultRecipients,//false:for reject until silent ui, "send email" button is gray;
			&SendEmailData
		};
		logd(L"[ExcuteEmailVerifyEx]nTempRecipients=%d",nTempRecipients);
		logd(L"[ExcuteEmailVerifyEx]SendEmailData.m_OnlyAttachmentData.m_bNeedCrl=%d",SendEmailData.m_OnlyAttachmentData.m_bNeedCrl);
		logd(L"[ExcuteEmailVerifyEx]SendEmailData.m_OnlyAttachmentData.m_rlInfo.bAllowOverride=%d",SendEmailData.m_OnlyAttachmentData.m_rlInfo.bAllowOverride);
		logd(L"[ExcuteEmailVerifyEx]bExistDefaultRecipients=%d",bExistDefaultRecipients);

		CDmAssistDlgEx dmDlg(&context);
		if (!SendEmailData.m_OnlyAttachmentData.m_bNeedHdr)          // No other dialog, set the Button to OK
			dmDlg.SetLastFlag();

		BOOL bHasHelpUrl = FALSE;

		if(SendEmailData.m_OnlyAttachmentData.m_bNeedCrl)
		{
			dmDlg.SetHelpUrl(L"http://www.nextlabs.com");
		}


		for (int i = 0; i < nTempRecipients; i++)
		{
			BOOL bRecipientFound = FALSE;
			BOOL bDomainMismatch = FALSE;
			BOOL bMultiClient    = FALSE;
			ATTACHMENTINFO info;

			for (int j = 0; j < nAttachments; j++)
			{
				//LpAttachmentData lpAttachmentData = m_listAttachments[j];

				if (SendEmailData.GetAttachmentData()[j].dm &&
					OLUtilities::IsStringInListIgnoreCase
					(SendEmailData.GetAttachmentData()[j].dmRecipients, OBLIGATION_STR_DELIM,
					TemplistRecipients[i].c_str()))
				{
					bRecipientFound = TRUE;
					bDomainMismatch = TRUE;
					info.push_back(std::pair<std::wstring, std::wstring>
						(SendEmailData.GetAttachmentData()[j].GetDispName(),
						SendEmailData.GetAttachmentData()[j].dmClientName));

					if (!bHasHelpUrl && SendEmailData.GetAttachmentData()[j].dmUrl[0] != L'\0')
					{
						dmDlg.SetHelpUrl(SendEmailData.GetAttachmentData()[j].dmUrl);
						bHasHelpUrl = TRUE;
					}
				}

				if (SendEmailData.GetAttachmentData()[j].mc &&
					OLUtilities::IsStringInListIgnoreCase
					(SendEmailData.GetAttachmentData()[j].mcRecipients, OBLIGATION_STR_DELIM,
					TemplistRecipients[i].c_str()))
				{
					bRecipientFound = TRUE;
					bMultiClient    = TRUE;

					if (!bHasHelpUrl && SendEmailData.GetAttachmentData()[j].mcUrl[0] != L'\0')
					{
						dmDlg.SetHelpUrl(SendEmailData.GetAttachmentData()[j].mcUrl);
						bHasHelpUrl = TRUE;
					}
				}

				if (SendEmailData.GetAttachmentData()[j].er &&
					OLUtilities::IsStringInListIgnoreCase
					(SendEmailData.GetAttachmentData()[j].erRecipients, OBLIGATION_STR_DELIM,
					TemplistRecipients[i].c_str()))
				{
					bRecipientFound = TRUE;

					if (!bHasHelpUrl && SendEmailData.GetAttachmentData()[j].erUrl[0] != L'\0')
					{
						dmDlg.SetHelpUrl(SendEmailData.GetAttachmentData()[j].erUrl);
						bHasHelpUrl = TRUE;
					}
				}

				if (!SendEmailData.m_OnlyAttachmentData.m_bNeedCrl)
				{
					SendEmailData.GetAttachmentData()[j].rl = FALSE;
				}
				
				if (SendEmailData.GetAttachmentData()[j].rl &&
					OLUtilities::IsStringInListIgnoreCase
					(SendEmailData.GetAttachmentData()[j].rlRecipients, OBLIGATION_STR_DELIM,
					TemplistRecipients[i].c_str()))
				{
					bRecipientFound = TRUE;
					if (!bHasHelpUrl && SendEmailData.GetAttachmentData()[j].rlUrl[0] != L'\0')
					{
						dmDlg.SetHelpUrl(SendEmailData.GetAttachmentData()[j].rlUrl);
						bHasHelpUrl = TRUE;
					}
				}
			}

			if (bRecipientFound)
			{
				BOOL bHasExisted = FALSE ;
				if( !listRecipientIndices.empty() )
				{
					std::vector<int>::iterator  itor = listRecipientIndices.begin() ;
					for( itor ; itor!= listRecipientIndices.end(); itor ++ )
					{
						DP((L"Verify if the multi-clients has exist: Current[%s],Mapper[%s]",TemplistRecipients[i].c_str(), TemplistRecipients[(*itor)].c_str())) ;
						if(  TemplistRecipients[(*itor)].compare(  TemplistRecipients[i].c_str() ) == 0 )
						{
							DP((L"Success mapped")) ;
							bHasExisted = TRUE ;
							break  ;
						}
					}
				}
				//--------------------------------------------------------------------------------------------
				if( bHasExisted == FALSE )
				{
					dmDlg.AddDmItemData(bMultiClient, bDomainMismatch, SendEmailData.m_OnlyAttachmentData.m_rlInfo.bAllowOverride,TemplistRecipients[i].c_str(), info);
					listRecipientIndices.push_back(i);
					listRecipientDmFlags.push_back(bDomainMismatch);
					listRecipientMcFlags.push_back(bMultiClient);
				}
			}
		}
		
		LRESULT lResult = dmDlg.DoModal(SendEmailData.GetWnd());

		if (IDCANCEL == lResult)
		{
			
			return EXCUTE_CANCEL_MAYBE_LOG_DESIGN;
		}
		else
		{
			if (!SendEmailData.m_OnlyAttachmentData.m_bNeedCrl)
			{
				SendEmailData.SetEvalAgain(true);
			}
			else
			{
				logd(L"[ExcuteEmailVerifyEx]Reset the SendEmailData allow value(old_%d) to true", SendEmailData.GetAllow());
				SendEmailData.SetAllow(true);
				SendEmailData.AllowOfRejectUnlessSilent(true);
			}
		}
	}
	return EXCUTESUCCESS;
}


EXCUTEOBLACTION CAttachmentObligationData::ExcuteEmailHDREx(CSendEmailData &SendEmailData)
{
	EXCUTEOBLACTION RetValue = EXCUTESUCCESS;

	if(SendEmailData.m_OnlyAttachmentData.m_bNeedHdr)
	{
		// pop up HDR inspector dialog

		if(g_hODHD)
		{
			AttachVector     vecAttachments;
			long lHdrRet=TRUE;
			HelpUrlVector		 listHelpUrls;
			TFunHdrObligation pFunHdrObligation;
			std::vector<int>  vectAttachmentIndex; //used to record which attachment have HDR done.
			std::vector<BOOL> vectAttachmentUpdate;
		
			for (size_t i = 0; i < SendEmailData.GetAttachmentData().size(); i++ )
			{
				if (!SendEmailData.GetAttachmentData()[i].IsAttachmentRemoved())
				{
					if (SendEmailData.GetAttachmentData()[i].hdr)
					{
						std::pair<std::wstring,std::wstring> oneAttachments(SendEmailData.GetAttachmentData()[i].GetSourcePath(),SendEmailData.GetAttachmentData()[i].GetTempPath());
						vecAttachments.push_back(oneAttachments);
						listHelpUrls.push_back(SendEmailData.GetAttachmentData()[i].hdrUrl);

						//record 
						vectAttachmentIndex.push_back((int)i);
						vectAttachmentUpdate.push_back(TRUE);
					}
				}
			}

			if (!vecAttachments.empty())
			{
				pFunHdrObligation=(TFunHdrObligation)GetProcAddress(g_hODHD,"HDRObligation");
				if(pFunHdrObligation)
				{
					lHdrRet=pFunHdrObligation(SendEmailData.GetWnd(),(RecipientVector &)SendEmailData.GetRecipientsData().GetRealRecipients(),vecAttachments,listHelpUrls,SendEmailData.m_OnlyAttachmentData.m_bIsWordMail);
				
					if(lHdrRet==FALSE)
					{
						RetValue = EXCUTE_CANCEL_MAYBE_LOG_DESIGN;
					}
					else
					{
						SendEmailData.SetEvalAgain(true);
						SendEmailData.SetAttachmentUpdated(&vectAttachmentIndex[0], &vectAttachmentUpdate[0], vectAttachmentIndex.size() );
					}
				}				
			}
		}
		else
		{
			DP((L"No ODHD module loaded!\n"));
		}

	}
	return RetValue;
}


/*
*/
//////////////////////////////////////////////////////////////////////////
BOOL FileTypeSupport(LPCWSTR wszFileName)
{
    const wchar_t* szSupportTypes[] = { L".docx", L".doc", L".dot", L".docm", L".dotx", L".dotm",
        L".xlsx", L".xls", L".xlsm", L".xlt", L".xltm", L".xltx",
        L".pptx", L".ppt", L".pot", L".potx", L".potm", L".potx", L".pptm",
        L".pdf",
    };

    if (NULL != wszFileName)
    {
        size_t nFileNameLen = wcslen(wszFileName);
        for (int i = 0; i < sizeof(szSupportTypes) / sizeof(szSupportTypes[0]); i++)
        {
            const wchar_t* pFileType = szSupportTypes[i];
            size_t nFileTypeLen = wcslen(pFileType);

            if ((nFileNameLen>nFileTypeLen) &&
                (_wcsicmp(wszFileName + nFileNameLen - nFileTypeLen, pFileType) == 0))
            {
                return TRUE;
            }
        }

    }

    return FALSE;
}

//////////////////////////////////////////////////////////////////////////
// Add internet headers(contains existing headers originated from a received mail if it's a mail to reply to sender, reply to all or forward and tags (increase or override existing header))
EXCUTEOBLACTION CAttachmentObligationData::ExcuteHeaderTaggingEx(CSendEmailData &SendEmailData,PolicyCommunicator* pPolicyCommunicator,COE_PAMngr& paMngr, CComPtr<IDispatch> dspMailItem)
{	
	int nUpdatedCount;
	EXCUTEOBLACTION excuteOblaction = EXCUTESUCCESS;
	CMessageHeader& msgHeader = SendEmailData.GetMessageHeader();

	// Changes? e.g. AutoHeaderTag: x-foo=yes, UIHeaderTag: x-foo=no;  --> changes = 1+1 ==> 1

	MsgHeaderParser::HEADER_PAIRS& nextlabsPairs = msgHeader.GetNextlabsHeaderPairs();

	// ## First process Automatic X-Header Tagging

	// Merge auto-tag pairs into the header pairs
	nUpdatedCount = msgHeader.GetTagPairs().size();
	if (0 < nUpdatedCount)
	{
		CMessageHeader::MergePairs(msgHeader.GetTagPairs(), nextlabsPairs);
	}else
	{
		logd(L"[ExcuteHeaderTaggingEx]No auto-tag on header");
	}
	//logd(L"After merging auto-tag pairs into the header pairs of nextlabs");
	//CMessageHeader::DebugPrintPairs(nextlabsPairs, L"Nextlabs X- Pair");

	// ## Then, process X-Header Hierarchical Classification
	if (msgHeader.NeedHCTagging())
	{
		logd(L"[ExcuteHeaderTaggingEx]Excute header tagging with UI");

		LONG lRet=0;
		vector<std::tr1::shared_ptr<HCADDTAGINFO>>  VecHCInfo;
		VecHCInfo.reserve(1);
	
#ifdef _DEBUG
		std::wstring temp;
		paMngr.GetTempFolder(temp); //e.g. "C:\Users\ADMINI~1.QAP\AppData\Local\Temp\PA\"
		logi(L"[ExcuteHeaderTaggingEx]m_szTempFoder=%s, objList.size()=%d, paMngr@%p", temp.c_str(), paMngr.GetPaParam()->objList.size(), &paMngr);
#endif

		lRet = msgHeader.GetPAMngr()->DoFileTagging_OE( g_hTAG,(PVOID)pPolicyCommunicator, PABase::AT_SENDMAIL, TRUE,L"Send E-mail",SendEmailData.GetWnd(),false,&VecHCInfo) ;
	
		msgHeader.SetHCTagged();
		//logi(L"[ExcuteHeaderTaggingEx]The number of HCADDTAGINFO is %d", VecHCInfo.size());

		if( lRet == PA_USER_CANCEL || lRet == PA_ERROR)
		{
			//user select cancel button
			logw(L"[ExcuteHeaderTaggingEx]user select cancel button or tag error!");
			excuteOblaction = EXCUTE_CANCEL_MAYBE_LOG_DESIGN;
		}else
		{
			std::vector<std::pair<std::wstring,std::wstring> > &vecHCTagPairs = VecHCInfo[0]->vecTagInfo;
			nUpdatedCount += vecHCTagPairs.size();
			// Merge HC-tag pairs into the nextlabs header pairs
			int nUIChanges = CMessageHeader::MergePairs(vecHCTagPairs, nextlabsPairs);

			logd(L"[ExcuteHeaderTaggingEx]After executing header tagging with UI, changes=%d, tags = %d", nUIChanges, vecHCTagPairs.size());

			//logd(L"After merging hc-tag pairs into the header pairs of nextlabs");
			//CMessageHeader::DebugPrintPairs(nextlabsPairs, L"Nextlabs X- Pair");
		}
	}

	if (0 < nUpdatedCount)
	{
		// synchronize the nextlabs vector to the headers vector so as to QueryPC again
		if(0 < CMessageHeader::MergePairs( nextlabsPairs, msgHeader.GetHeaderPairs() ) )
		{
			logd(L"[ExcuteHeaderTaggingEx]EnableEvalAgainForHeader");
			SendEmailData.EnableEvalAgainForHeader();
		}
	}

	return excuteOblaction;
}

//////////////////////////////////////////////////////////////////////////
EXCUTEOBLACTION CAttachmentObligationData::ExcuteFileTaggingEx(CSendEmailData &SendEmailData,PolicyCommunicator* pPolicyCommunicator,COE_PAMngr& paMngr, CComPtr<IDispatch> dspMailItem)
{
	
	if (SendEmailData.m_OnlyAttachmentData.m_bNeedAutoFileTagging||SendEmailData.m_OnlyAttachmentData.m_bNeedHCFileTagging
		||SendEmailData.m_OnlyAttachmentData.m_bNeedInterFileTagging)
	{
	
		LONG lRet=0;
		const int nAttachmentNum = (int)SendEmailData.GetAttachmentData().size();
		vector<std::tr1::shared_ptr<HCADDTAGINFO>>  VecHCInfo;
		VecHCInfo.reserve(nAttachmentNum + 1);

		lRet = paMngr.DoFileTagging_OE( g_hTAG,(PVOID)pPolicyCommunicator, PABase::AT_SENDMAIL, paMngr.CheckIsLastPA( PA_Mngr::OB_TAG),L"Send E-mail",SendEmailData.GetWnd(),false,&VecHCInfo) ;

		CComPtr<Outlook::Attachments> attachments = NULL;
		MailItemUtility::get_Attachments(dspMailItem, &attachments,TRUE);
		
		for (size_t t = 0; t < VecHCInfo.size(); t++)
		{
			for (int i = 0; i < nAttachmentNum; i++)
			{
				if (_wcsicmp(VecHCInfo[t]->strDstPath.c_str(),SendEmailData.GetAttachmentData()[i].GetTempPath().c_str()) == 0)
				{
					CAttachmentData& attachmentData = SendEmailData.GetAttachmentData()[i];
					attachmentData.GetAttachmentTagData().GetHCTagInfo().SetAddHCTag(VecHCInfo[t]->vecTagInfo);
					attachmentData.SetStrLogID(VecHCInfo[t]->strLogID);
  
					if (!VecHCInfo[t]->vecTagInfo.empty())
					{
						if (attachments!=NULL)
						{
							CComPtr<Outlook::Attachment> pAttachment;
							CComVariant varAttachIndex = attachmentData.GetOriginalAttachIndex();
							attachments->Item(varAttachIndex, &pAttachment);
							if (pAttachment)
							{
								//added tagged information into mail property, so that we will not do it second time.
								CMailPropertyTool::SetHCTagToAttachmentProperty(pAttachment, VecHCInfo[t]->vecTagInfo);

								attachmentData.SetHCTagAlreadTagged(VecHCInfo[t]->vecTagInfo); 
								VECTOR_TAGPAIR::iterator it;
								for(it = VecHCInfo[t]->vecTagInfo.begin(); it != VecHCInfo[t]->vecTagInfo.end(); it++){
									logd(L"[ExcuteFileTaggingEx]HC tag key = %s", it->first.c_str());
									logd(L"[ExcuteFileTaggingEx]HC tag value = %s", it->second.c_str());
								}
								//update last modify time for temp file.
								attachmentData.UpdateLastModifyTimePropertyForTempFile(pAttachment);
							}
						}
					}
			
					break;
				}
			}
		}
		
		
		if( lRet == PA_USER_CANCEL || lRet == PA_ERROR)
		{
			//user select cancel button
			logd((L"user select cancel button or tag error!\n"));
			return EXCUTE_CANCEL_MAYBE_LOG_DESIGN;
		}
		else
		{
			for (int iAttachIndex = 0; iAttachIndex < nAttachmentNum; iAttachIndex++)
			{
				
				int nExistOblType = SendEmailData.GetAttachmentData()[iAttachIndex].GetExistObligationType();
				CComPtr<Outlook::Attachment> pAttachment;
				CComVariant varAttachIndex =  SendEmailData.GetAttachmentData()[iAttachIndex].GetOriginalAttachIndex();
				attachments->Item(varAttachIndex, &pAttachment);

                if ((nExistOblType&LOOP_OBLIGATION_FILE_TAGGING) ||
					(nExistOblType&LOOP_OBLIGATION_FILE_TAGGING_AUTO))
				{
					SendEmailData.SetAttachmentUpdated(iAttachIndex, TRUE);
					SendEmailData.GetAttachmentData()[iAttachIndex].SetAttachmentUpdateProperty(pAttachment);
					logd(L"[ExcuteFileTaggingEx]autofiletagging set attachment[%d] updated.", iAttachIndex);

				}
                else if (nExistOblType&LOOP_OBLIGATION_FILE_OHC)
                {
                    if (FileTypeSupport(SendEmailData.GetAttachmentData()[iAttachIndex].GetTempPath().c_str()))
                    {
                        SendEmailData.SetAttachmentUpdated(iAttachIndex, TRUE);
						SendEmailData.GetAttachmentData()[iAttachIndex].SetAttachmentUpdateProperty(pAttachment);
						logd(L"[ExcuteFileTaggingEx]hcfiletagging set attachment[%d] updated.", iAttachIndex);
                    }
                }
			}

			logd(L"[ExcuteFileTaggingEx]SetEvalAgain");
			SendEmailData.SetEvalAgain(true);
		}
	}

	return EXCUTESUCCESS;

}


void CAttachmentObligationData::UpdateTempFilePath(CSendEmailData &SendEmailData,COE_PAMngr& paMngr)
{
	size_t nAttachmentPos = SendEmailData.GetAttachmentData().size();
	for (size_t i = 0; i < nAttachmentPos; i++)
	{
		if (SendEmailData.GetAttachmentData()[i].IsAttachmentRemoved())
		{
			continue;
		}
		BOOL bNeedUpdate = FALSE;
		wchar_t strNewTempPath[1024] = {0}; 
		paMngr.QueryRetName_bySrc(SendEmailData.GetAttachmentData()[i].GetTempPath().c_str(),bNeedUpdate,strNewTempPath);
		if (bNeedUpdate&&strNewTempPath != '\0')
		{
			SendEmailData.GetAttachmentData()[i].SetTempPath(strNewTempPath);
			SendEmailData.SetAttachmentUpdated((int)i, TRUE);
		}
	}
}


EXCUTEOBLACTION CAttachmentObligationData::ExcuteNXLEncrptyEx(CSendEmailData &SendEmailData)
{
	if( SendEmailData.GetAttachmentData().size() != 0  && SendEmailData.m_OnlyAttachmentData.m_bNeedRMC)
	{
		//Get nextlabs temp folder
		wstring fullTempPath = L"";
		wchar_t szPath[MAX_PATH]; 
		ZeroMemory(szPath, MAX_PATH); 
		SHGetSpecialFolderPath(NULL, szPath,  CSIDL_LOCAL_APPDATA, FALSE);
		wstring strNextLabsTempFolder(szPath);
		strNextLabsTempFolder += NEXTLABS_NXL_TEMP_FOLDER;
		std::wstring strTempFolder = L"";

		//Get exe excute path.
		wchar_t szDir[MAX_PATH] = {0};
		if(!NLConfig::ReadKey(L"SYSTEM\\CurrentControlSet\\services\\nxrmserv\\ImagePath", szDir, MAX_PATH))
		{
			MessageBox(NULL,L"You must install NextLabs RMC to apply rights-protection to the email attachment before sending the email.\n",L"Nextlabs Enforcer For Microsoft Outlook",MB_OK);
			return EXCUTE_CANCEL_MAYBE_LOG_DESIGN;
		}

		wstring strExEFullPath(szDir);
		boost::algorithm::replace_all(strExEFullPath, L"\"", L"");
		boost::algorithm::replace_all(strExEFullPath, L"nxrmserv.exe", L"nxrmconv.exe");


		for (DWORD i = 0; i < SendEmailData.GetAttachmentData().size(); i++)
		{
			vector<ENCRYPTTAG> vecTag;
			if(SendEmailData.GetAttachmentData()[i].GetAutoNxl())
			{
				wstring strFilePath = SendEmailData.GetAttachmentData()[i].GetSourcePath();
				VECTOR_TAGPAIR vecNxlTag = SendEmailData.GetAttachmentData()[i].GetAttachmentTagData().GetTempAddedAutoTagNxlTag();
				VECTOR_TAGPAIR::iterator itor;
				ENCRYPTTAG tagInfo;
				for (itor = vecNxlTag.begin(); itor != vecNxlTag.end(); itor++)
				{
					tagInfo.strTagName = itor->first;
					tagInfo.strTagValue = itor->second;
					vecTag.push_back(tagInfo);
				}
			} 

			if (SendEmailData.GetAttachmentData()[i].GetAutoNxl() || SendEmailData.GetAttachmentData()[i].GetInterActiveNxl())
			{
				if (IsNxlFormat(SendEmailData.GetAttachmentData()[i].GetTempPath().c_str()))
				{
					DP((L"[%s] temp file is encrypt file, we don't do encrypt file\n", SendEmailData.GetAttachmentData()[i].GetTempPath().c_str()));
					continue;
				}

				//Copy the temp file to nextlabs temp folder
				CDisplayName dispName(SendEmailData.GetAttachmentData()[i].GetSourcePath().c_str());
				fullTempPath = strNextLabsTempFolder + dispName.GetFileName();

				OLUtilities::CreateMulDirectory(strTempFolder);
				::CopyFile(SendEmailData.GetAttachmentData()[i].GetTempPath().c_str(),fullTempPath.c_str(),FALSE);

				wstring strFilePath(SendEmailData.GetAttachmentData()[i].GetTempPath().c_str());
				if (SendEmailData.GetAttachmentData()[i].GetInterActiveNxl())
				{
					if(!OLUtilities::ApplyInterActiveNXLEncrypt(strExEFullPath,strFilePath))
					{
                        const wchar_t* pMsg = L"Since the application could not apply rights-protection, email could not be sent.";
                        MessageBox(NULL, pMsg, L"Nextlabs Enforcer For Microsoft Outlook", MB_OK);
                        return EXCUTE_CANCEL_MAYBE_LOG_DESIGN;
					}
					SendEmailData.SetEvalAgain(true);
					SendEmailData.GetAttachmentData()[i].SetTempPath(strFilePath.c_str());
					SendEmailData.SetAttachmentUpdated(i, TRUE);
					SendEmailData.GetAttachmentData()[i].SetNxlSuccessful(TRUE);
					logd(L"[ExcuteNXLEncrptyEx]Attachment_%d, InterActiveNXL %s successful", i, strFilePath.c_str());
				}
				if (SendEmailData.GetAttachmentData()[i].GetAutoNxl())
				{
					if (!OLUtilities::ApplyAutoNXLEncrypt(strExEFullPath,strFilePath,vecTag))
					{
                        const wchar_t* pMsg = L"Since the application could not apply rights-protection, email could not be sent.";
                        MessageBox(NULL, pMsg, L"Nextlabs Enforcer For Microsoft Outlook", MB_OK);
                        return EXCUTE_CANCEL_MAYBE_LOG_DESIGN;
					}
					SendEmailData.SetEvalAgain(true);
					SendEmailData.GetAttachmentData()[i].SetTempPath(strFilePath.c_str());
					SendEmailData.SetAttachmentUpdated(i, TRUE);
					SendEmailData.GetAttachmentData()[i].SetNxlSuccessful(TRUE);
					logd(L"[ExcuteNXLEncrptyEx]Attachment_%d, AutoNxl %s successful", i, strFilePath.c_str());
				}
			}
		}
	}
	return EXCUTESUCCESS;
}


EXCUTEOBLACTION CAttachmentObligationData::ExcuteEncryptionEx(CSendEmailData &SendEmailData,PolicyCommunicator* pPolicyCommunicator,COE_PAMngr& paMngr)
{
	/*
	supporting do the encryption
	*/
	
	LONG lRet=0;
	lRet = paMngr.DoEcnryption( g_hENC,(PVOID)pPolicyCommunicator, PABase::AT_SENDMAIL, TRUE, L"Send E-mail",SendEmailData.GetWnd() ) ;
	if( lRet != 0 )
	{
		return EXCUTE_CANCEL_MAYBE_LOG_DESIGN;
	}

	if (SendEmailData.m_OnlyAttachmentData.m_bNeedZIP || SendEmailData.m_OnlyAttachmentData.m_bNeedPGP)
	{
		UpdateTempFilePath(SendEmailData,paMngr);
		SendEmailData.SetEvalAgain(true);
	}
	return EXCUTESUCCESS;
}

EXCUTEOBLACTION CAttachmentObligationData::ExcutePortableEncryptionEx(CSendEmailData &SendEmailData,PolicyCommunicator* pPolicyCommunicator,COE_PAMngr& paMngr)
{

	LONG lRet=0;
	lRet = paMngr.DoPortableEncryption( g_hPE,(PVOID)pPolicyCommunicator, PABase::AT_SENDMAIL, FALSE, TRUE, L"Send E-mail",SendEmailData.GetWnd() ) ;
	if( lRet != 0 )
	{
		return EXCUTE_CANCEL_MAYBE_LOG_DESIGN;
	}
	
	return EXCUTESUCCESS;
}

EXCUTEOBLACTION CAttachmentObligationData::ExcuteEmailStrIPAttachmentEx(CSendEmailData &SendEmailData,CComPtr<IDispatch> dspMailItem,COE_PAMngr& paMngr)
{
	if(SendEmailData.m_OnlyAttachmentData.m_bNeedStripAttachment)
	{
		HRESULT hr= S_OK;
		wchar_t* pwchTopMsg = NULL;
		wchar_t* pwchBottomMsg = NULL;
		int iTopMessageLength=0;
		int iBottomMessageLength=0;
		hr = RepositoryAdaptersManagerEx(L"Outlook Enforcer",dspMailItem,&(SendEmailData.m_AttachmentOblData->m_adapterAttachments),&pwchTopMsg,iTopMessageLength,&pwchBottomMsg,iBottomMessageLength);

		if(SUCCEEDED(hr))
		{	
			
			wstring wstrTempBody=SendEmailData.GetBodyData().GetTempBody();
			wstring wstrTopMsg(pwchTopMsg);
			wstring wstrBottomMsg(pwchBottomMsg);
			SendEmailData.GetBodyData().SetStripAttachmentTopMessage(wstrTopMsg);
			SendEmailData.GetBodyData().SetStripAttachmentBottomMessage(wstrBottomMsg);
			SendEmailData.GetBodyData().SetBodyChanged(true);

			
			ReleaseRepositoryAdaptersResource(pwchTopMsg,true);
			ReleaseRepositoryAdaptersResource(pwchBottomMsg,true);
			DWORD AttachmentPos = 0;
			for (AttachmentPos = 0; AttachmentPos < SendEmailData.GetAttachmentData().size();AttachmentPos++)
			{
				if (SendEmailData.GetAttachmentData()[AttachmentPos].IsAttachmentRemoved())
				{
					continue;
				}
				DWORD dwAdpterAttamentPos=0;
				for(dwAdpterAttamentPos=0;dwAdpterAttamentPos<SendEmailData.m_AttachmentOblData->m_adapterAttachments.Count();dwAdpterAttamentPos++)
				{
					AdapterCommon::Attachment& att= SendEmailData.m_AttachmentOblData->m_adapterAttachments.Item(dwAdpterAttamentPos);
					if(att.GetRemovedFlag()==true&&0==wcscmp(att.GetTempPath().c_str(),SendEmailData.GetAttachmentData()[AttachmentPos].GetTempPath().c_str()))
					{
						SendEmailData.GetAttachmentData()[AttachmentPos].SetAttachmentRemoved(true);
						SendEmailData.SetAttachmentUpdated(AttachmentPos, TRUE);
						SendEmailData.SetEvalAgain(true);
					}
				}
			}
			PABase::PA_PARAM* pPAParam=paMngr.GetPaParam();
			PABase::OBJECTINFOLIST& paObjList=pPAParam->objList;
			PABase::OBJECTINFOLIST::iterator itPAObjList;
			for(itPAObjList=paObjList.begin();itPAObjList!=paObjList.end();)
			{
				PABase::OBJECTINFO& objInfo=(*itPAObjList);
				bool bDelete=FALSE;
				for(DWORD k=0;k<SendEmailData.m_AttachmentOblData->m_adapterAttachments.Count();k++)
				{
					AdapterCommon::Attachment& att=SendEmailData.m_AttachmentOblData->m_adapterAttachments.Item(k);
					if(att.GetRemovedFlag()==true&&wcscmp(att.GetTempPath().c_str(),objInfo.strSrcName.c_str())==0)
					{
						itPAObjList=paObjList.erase(itPAObjList);
						bDelete=TRUE;
						if(paObjList.size()==0)
						{
							return EXCUTESUCCESS;
						}
						else
						{
							break;
						}
					}

				}
				if(paObjList.size()==0)
				{
					return EXCUTESUCCESS;
				}
				if(!bDelete)
				{
					itPAObjList++;
				}
			}
			return EXCUTESUCCESS;
		}
	}
	return EXCUTESUCCESS;
}
