#include "stdafx.h"
#include "MailItemUtility.h"
#include "outlookUtilities.h"
#include "../common/AttachmentFileMgr.h"
#include "../common/CommonTools.h"
#include "DataType.h"
#include "MailPropertyTool.h"

#include <set>

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/trim.hpp>

//implement for CSubjectData
bool CSubjectData::GetSubjectFromMailItem(CComPtr<IDispatch> dspMailItem)
{
	CComBSTR bstrSubject;
	HRESULT hr=MailItemUtility::get_Subject(dspMailItem,&bstrSubject);
	if (SUCCEEDED(hr))
	{
		if (bstrSubject)
		{
		  m_wstrOriginSubject = bstrSubject;
		}

		return true;
	}
	else
	{
		DP((L"CSubjectData::GetSubjectFromMailItem failed.\n"));
	}

	return false;
}

void CSubjectData::GetAuditLogInfo(wstring & strLogInfo)
{
	if (!m_mapPrependSubject.empty())
	{
		wstring strPreSubject;
		map<wstring,bool>::iterator itor;
		for (itor = m_mapPrependSubject.begin(); itor != m_mapPrependSubject.end(); itor++)
		{
			if (!strPreSubject.empty())
			{
				strPreSubject = strPreSubject + L";";
			}
			strPreSubject += itor->first;
		}
		strLogInfo += L"Prepend Email Subject obligation add text: ";
		strLogInfo += strPreSubject;
		strLogInfo += L" \n";	
	}
}


void CRecipientData::GetAuditLogInfo(wstring& strLogInfo)
{
	

	STRINGLIST::iterator USRItar;
	wstring strUSRName;
	for (USRItar = m_lstOriginRecipients.begin(); USRItar != m_lstOriginRecipients.end(); USRItar++)
	{
		if (!strUSRName.empty())
		{
			strUSRName += L";";
		}
		strUSRName += *USRItar;
	}
	wstring strTempAddRecipient = L"User added recipient: ";
	strTempAddRecipient += strUSRName;
	strTempAddRecipient = L"\n";
	strLogInfo += strTempAddRecipient;

	wstring strRMRName;
	STRINGLIST::iterator RMRItar;
	for (RMRItar = m_listRmRecipients.begin(); RMRItar != m_listRmRecipients.end(); RMRItar++)
	{
		if (!strRMRName.empty())
		{
			strRMRName += L";";
		}
		strRMRName += *RMRItar;
	}
	if(!strRMRName.empty())
	{
		wstring strTempRemRcpet = L"User removed recipient: ";
		strTempRemRcpet += strUSRName;
		strTempRemRcpet = L"\n";
		strLogInfo += strTempRemRcpet;
	}
}

void CBodyData::GetAuditLogInfo(wstring & strLogInfo)
{
	if (!m_mapAppendBody.empty())
	{
		wstring strAppendBody;
		map<wstring,bool>::iterator itor;
		for (itor = m_mapAppendBody.begin(); itor != m_mapAppendBody.end(); itor++)
		{
			if (!strAppendBody.empty())
			{
				strAppendBody = strAppendBody + L";";
			}
			strAppendBody += itor->first;
		}

		strLogInfo += L"Append Email Body obligation add text: ";
		strLogInfo += strAppendBody;
		strLogInfo += L" \n";
	}


	if (!m_mapPrependBody.empty())
	{
		wstring strPreBody;
		map<wstring,bool>::iterator itor;
		for (itor = m_mapPrependBody.begin(); itor != m_mapPrependBody.end(); itor++)
		{
			if (!strPreBody.empty())
			{
				strPreBody = strPreBody + L";";
			}
			strPreBody += itor->first;
		}

		strLogInfo += L"Prepend Email Body obligation add text: ";
		strLogInfo += strPreBody;
		strLogInfo += L" \n";
	}
}

bool CBodyData::ReplaceParagraphsText(CComPtr<Word::_Document>& wordDoc)
{

	wordDoc->Select();
	CComPtr<Word::_wordApplication> spWordApp = NULL;
	
	HRESULT hr = wordDoc->get_wordApplication(&spWordApp);
	if (SUCCEEDED(hr) && spWordApp)
	{
		
		CComPtr<Word::wordSelection> spWordSelection = NULL;

		hr = spWordApp->get_wordSelection(&spWordSelection);
		if (SUCCEEDED(hr) && spWordSelection)
		{
			map<wstring,wstring>::iterator hypermapItor;
			for(hypermapItor = m_mapReplaceInfo.begin(); hypermapItor != m_mapReplaceInfo.end(); hypermapItor++)
			{
				CComPtr<Word::wordHyperlinks> spHyperlinks;
				spWordSelection->get_wordHyperlinks(&spHyperlinks);
				long HyperlinkNum;
				spHyperlinks->get_Count(&HyperlinkNum);
				for (long j = 0; j < HyperlinkNum; j++)
				{
					CComPtr<Word::wordHyperlink> spHyperlink;
					CComVariant varNum(j+1);
					hr = spHyperlinks->Item(&varNum,&spHyperlink);
					if (SUCCEEDED(hr))
					{

						CComBSTR bstrCurrent;
						hr = spHyperlink->get_TextToDisplay(&bstrCurrent);
						if (SUCCEEDED(hr) && bstrCurrent.m_str != NULL)
						{
							wstring strCurDisplayText(bstrCurrent.m_str);
							if (_wcsicmp(strCurDisplayText.c_str(),hypermapItor->first.c_str()) == 0)
							{
								CComBSTR bstrReplaceText(hypermapItor->second.c_str());
								spHyperlink->put_Address(bstrReplaceText);
								spHyperlink->put_TextToDisplay(bstrReplaceText);
							}

						}

					}
				}
			}
			


			map<wstring,wstring>::iterator mapItor;
			for(mapItor = m_mapReplaceInfo.begin(); mapItor != m_mapReplaceInfo.end(); mapItor++)
			{
				CComPtr<Word::Find> spFind;
				hr = spWordSelection->get_Find(&spFind);
				if (SUCCEEDED(hr))
				{
					spFind->ClearFormatting();
					CComPtr<Word::Replacement> spReplacement;
					hr = spFind->get_Replacement(&spReplacement);
					if (SUCCEEDED(hr))
					{
						spReplacement->ClearFormatting();

						CComBSTR bstrFindText(mapItor->first.c_str());

						hr = spFind->put_Text(bstrFindText);

						CComBSTR bstrReplaceText((mapItor->second.c_str()));

						spReplacement->put_Text(bstrReplaceText);

						spFind->put_Forward(VARIANT_TRUE);

						spFind->put_Wrap(Word::WdFindWrap::wdFindContinue);

						spFind->put_Format(VARIANT_FALSE);

						spFind->put_MatchCase(VARIANT_FALSE);

						spFind->put_MatchWholeWord(VARIANT_FALSE);

						spFind->put_MatchWildcards(VARIANT_FALSE);

						spFind->put_MatchSoundsLike(VARIANT_FALSE);

						spFind->put_MatchAllWordForms(VARIANT_FALSE);

						spFind->put_MatchByte(VARIANT_FALSE);


						VARIANT_BOOL varBOOL;
						CComVariant var(Word::WdReplace::wdReplaceAll);
						hr = spFind->Execute(&vtMissing,&vtMissing,&vtMissing,&vtMissing,
							&vtMissing,&vtMissing,&vtMissing,&vtMissing,&vtMissing,&vtMissing,
							&var,&vtMissing,&vtMissing,&vtMissing,&vtMissing,&varBOOL);


					}
				}

			}

		}
	}

	return true;
}



//implement for CBodyData
bool CBodyData::GetBodyFromMailItem(CComPtr<IDispatch> dspMailItem)
{
	//get body format
	HRESULT hr=MailItemUtility::get_BodyFormat(dspMailItem,&m_bodyFormat);
	if(FAILED(hr))
		m_bodyFormat=olFormatUnspecified;

	//get body data
    CComBSTR bstrBody;
	if(m_bodyFormat==olFormatHTML)
	{
		hr=MailItemUtility::get_HTMLBody(dspMailItem,&bstrBody);
	}
	else
	{
		hr=MailItemUtility::get_Body(dspMailItem,&bstrBody);
	}

	if (SUCCEEDED(hr))
	{
		if (bstrBody)
		{
			m_wstrOriginBody = bstrBody;
		}
		return true;
	}
	else
	{
		DP((L"CBodyData::GetBodyFromMailItem failed.\n"));
	}
	return false;
}




//implement for CRecipientsData
bool CRecipientData::GetRecipientsFromMailItem(CComPtr<IDispatch> dspMailItem)
{
   OLUtilities::GetMailRecipients(dspMailItem, m_lstOriginRecipients);
   SetRealRecipients(m_lstOriginRecipients);
   for (STRINGLIST::iterator itt = m_lstOriginRecipients.begin(); itt != m_lstOriginRecipients.end(); )
   {
	   logd(L"[GetRecipientsFromMailItem]Real recipient = %s", itt->c_str());
	   itt++;
   }
   return true;
}


void CRecipientData::AddShowDenyRecipients(wstring& strRecipient)
{
	STRINGLIST::iterator itor ;
	bool bFind = false;
	for (itor = m_lstShowDenyRecipients.begin(); itor != m_lstShowDenyRecipients.end();itor ++)
	{
		if (_wcsicmp((*itor).c_str(),strRecipient.c_str()) == 0)
		{
			bFind = true;
			break;
		}
	}
	if (!bFind)
	{
		m_lstShowDenyRecipients.push_back(strRecipient);
	}
}

void CRecipientData::UpdateRealRecipients()
{
	//remove
	STRINGLIST::iterator itRemoveReceiver = m_listRmRecipients.begin();
	while (itRemoveReceiver != m_listRmRecipients.end())
	{
		STRINGLIST::iterator itRealReceiver = m_lstRealRecipients.begin();
		while (itRealReceiver != m_lstRealRecipients.end())
		{
			if (_wcsicmp(itRealReceiver->c_str(), itRemoveReceiver->c_str())==0)
			{
				m_lstRealRecipients.erase(itRealReceiver);
				break;
			}
			itRealReceiver++;
		}
		itRemoveReceiver++;
	}

	//clear m_listRmRecipients
	//m_listRmRecipients.clear();
}



//implement for CSendEmailData
bool CSendEmailData::HasAttachmentUpdated()
{
	std::vector<CAttachmentData>::iterator itAttach = m_vecAttachData.begin();
	while (itAttach != m_vecAttachData.end())
	{
		if (itAttach->IsAttachmentUpdated())
		{
			return true;
		}
		itAttach++;
	}
	return false;
}

void CSendEmailData::ResetInformation()
{
	//reset receiver action information for every attachment
	std::vector<CAttachmentData>::iterator itAttachData = m_vecAttachData.begin();
	while (itAttachData != m_vecAttachData.end())
	{
		itAttachData->ClearReceiverIndexForReceiveAction();
		itAttachData->ClearRecordPosForReceiveAction();
		itAttachData++;
	}
}

void CSendEmailData::SetRichAlertMsgData(const wchar_t* wszHeaderTextAllow, const wchar_t* wszHeaderTextDeny, const wchar_t* wszAlertMsg)
{
	if (wszHeaderTextAllow)
	{
		m_richAlertMsgData.m_strHeaderTextOnAllow = wszHeaderTextAllow;
	}

	if (wszHeaderTextDeny)
	{
		m_richAlertMsgData.m_strHeaderTextOnDeny = wszHeaderTextDeny;
	}

	if (wszAlertMsg)
	{
		if (m_richAlertMsgData.m_strAlertMsg.find(wszAlertMsg)==std::wstring::npos)
		{
			m_richAlertMsgData.m_strAlertMsg += wszAlertMsg;
			m_richAlertMsgData.m_strAlertMsg += L"<br>";
		}		
	}

}

BOOL CSendEmailData::GetBubbleMsgTxT(bool bIsAllow,wstring& strHeader, wstring & strBubbleMsg)
{
	if (m_mapAlertInfo.empty())
	{
		return FALSE;
	}

	if (!bIsAllow)
	{
		strHeader = theCfg[L"Alert Message"][L"HeaderTextOnDeny"];
		if (strHeader.empty())
		{
			strHeader = L"<br><br><br>This email couldn't be sent because of the following policies:";
		}
	}
	else
	{
		strHeader = theCfg[L"Alert Message"][L"HeaderTextOnAllow"];
		if (strHeader.empty())
		{
			strHeader = L"<br><br><br>This email is recorded because of the following policies:";
		}
	}
	wstring strNotification = L"";
	map<wstring,wstring>::iterator mapItor;
	for (mapItor = m_mapAlertInfo.begin(); mapItor != m_mapAlertInfo.end(); mapItor++)
	{
		if (boost::algorithm::iequals(mapItor->second,L"Allow: "))
		{
			strNotification = strNotification + mapItor->first;
		}
		else
		{
			strNotification = mapItor->first + strNotification;
		}

	}
	strBubbleMsg += strNotification;
	return TRUE;
}

void CSendEmailData::SetAttachmentUpdated(int nAttachIndex[], BOOL bUpdateValue[], size_t nCount)
{
    for (size_t i=0; i<nCount; i++)
    {
		SetAttachmentUpdated(nAttachIndex[i], bUpdateValue[i] );
		logd(L"[BatchSetAttachmentUpdated]attachment[%d] update value=%d.", i, bUpdateValue[i]);
    }
}

void CSendEmailData::SetAttachmentUpdated(int nAttachIndex, BOOL bUpdateValue)
{
	if ((nAttachIndex>=0) && (nAttachIndex<(int)m_vecAttachData.size()))
	{
		m_vecAttachData[nAttachIndex].SetAttachmentUpdated(bUpdateValue==TRUE?true:false);
		logd(L"[SingleSetAttachmentUpdated]attachment[%d] update value=%d.", nAttachIndex, bUpdateValue);
	}
}

bool CSendEmailData::HasAttachmentRemoved()
{
	std::vector<CAttachmentData>::iterator itAttach = m_vecAttachData.begin();
	while (itAttach != m_vecAttachData.end())
	{
		if (itAttach->IsAttachmentRemoved())
		{
			return true;
		}
		itAttach++;
	}
	return false;
}

bool CSendEmailData::GetDataFromMailItem(CComPtr<IDispatch> dspMailItem, const std::map<long,std::wstring>& map3thAppAttachmentPath, ITEM_TYPE origEmailType,bool bNeedAssociate)
{
	bool bSuccess = false;

	OLUtilities::CheckGetMailItemType(dspMailItem,m_EmailType);
	//if ( (m_EmailType != MAIL_ITEM) && (m_EmailType != MEETING_ITEM) && (m_EmailType != APPOINTMENT_ITEM))
	if (!CMailPropertyTool::OESupportMailType(m_EmailType))
	{
		return bSuccess;	
	}

    //subject
    bSuccess =  m_subjectData.GetSubjectFromMailItem(dspMailItem);
	if (bSuccess)
	{
		logi(L"====>subject = %s\n", m_subjectData.GetOriginSubject().c_str());
		m_subjectData.SetTempSubject(m_subjectData.GetOriginSubject().c_str());
	}

	//body
	bSuccess = m_bodyData.GetBodyFromMailItem(dspMailItem);
	if (bSuccess)
	{
		//logi(L"====>body = %s", m_bodyData.GetOriginBody().c_str());
		m_bodyData.SetTempBody(m_bodyData.GetOriginBody().c_str());
	}

	// message header
	bSuccess = m_messageHeader.GetHeaderFromMailItem(dspMailItem);

	//recipient
    m_recipientsData.GetRecipientsFromMailItem(dspMailItem);
	
	//attachment
    GetAttachmentFromMailItem(dspMailItem, map3thAppAttachmentPath, origEmailType,bNeedAssociate);


	return true;

}
bool CSendEmailData::IsEqualSender(std::wstring wsRecipient)
{
	wstring strCurrentSender = L"";
	strCurrentSender = this->GetSender();
	if (_wcsicmp(wsRecipient.c_str(), strCurrentSender.c_str()) == 0)
		return true;
	return false;
}
#if 0
void  CSendEmailData::DeleteOrganigerForMeeting(CSendEmailData& emailData)
{
	wstring strCurrentSender = L"";
	strCurrentSender = emailData.GetSender();
	logd(L"[DeleteOrganigerForMeeting]Current sender = %s.", strCurrentSender.c_str());
	if (strCurrentSender.empty())
	{
		return ;
	}
	else
	{
		for (vector<std::wstring>::iterator it = emailData.GetRecipientsData().GetRealRecipients().begin();it != emailData.GetRecipientsData().GetRealRecipients().end();)
		{
			if(_wcsicmp(it->c_str(), strCurrentSender.c_str()) == 0)
			{
				it=emailData.GetRecipientsData().GetRealRecipients().erase(it);
				logd(L"[DeleteOrganigerForMeeting]erase Organiger.");
				//break;//usually organiger is the first one in the recipients list
			}
			else
				it++;
		}
		return;
	}
}
#endif

CAttachmentData* CSendEmailData::GetAttachmentDataByIndex(long nIndex)
{
	std::vector<CAttachmentData>::iterator itAttachment = m_vecAttachData.begin();
	while (itAttachment != m_vecAttachData.end())
	{
		if (itAttachment->GetOriginalAttachIndex() == nIndex )
		{
			return &(*itAttachment);
		}
		itAttachment++;
	}
	return NULL;
}

std::wstring CAttachmentData::GetPreOutlookLastModifyTime(CComPtr<Outlook::Attachment> spAttachment, const std::wstring& kstrSourcePath, int nOnsendTimes)
{
    std::wstring wstrPreOutlookLastModifyTime = L"";
    if (1 == nOnsendTimes)
    {
        SYSTEMTIME stuSourceFileLastModifyTime = { 0 };
        if (!kstrSourcePath.empty())
        {
            GetFileSystemTime(kstrSourcePath, &stuSourceFileLastModifyTime, 1);
        }
        else
        {
            NLPRINT_DEBUGVIEWLOG(L"!!!Serious error, pre file path is empty");
        }
        wstrPreOutlookLastModifyTime = ConverSysTimeToString(stuSourceFileLastModifyTime);
        CMailPropertyTool::SetAttachmentProperty(spAttachment, PROP_NAME_PREOUTLOOKTEMPLASTMODIFYTIME, wstrPreOutlookLastModifyTime.c_str());
    }
    else
    {
        wstrPreOutlookLastModifyTime = CMailPropertyTool::GetAttachmentProperty(spAttachment, PROP_NAME_PREOUTLOOKTEMPLASTMODIFYTIME);
    }
    return wstrPreOutlookLastModifyTime;
}
bool CAttachmentData::GetOutlookTempModifyFlagAndUpdate(CComPtr<Outlook::Attachment> spAttachment)
{NLONLY_DEBUG
	const std::wstring kstrTempPath = GetTempPath();
	const std::wstring kstrSourcePath = GetSourcePath();
	const int nOnsendTimes = GetAttachmentOnSendTimes();
    bool bModified = false;
    if (!kstrTempPath.empty())
    {
        if (IfNeedCheckModifyFlag(kstrTempPath.c_str()))
        {
            SYSTEMTIME stuCurTempFileLastModifyTime = { 0 };
            bool bGetSysTime = GetFileSystemTime(kstrTempPath, &stuCurTempFileLastModifyTime, 1);
            if (bGetSysTime)
            {
                std::wstring wstrPreTempFileLastModifyTime = GetPreOutlookLastModifyTime(spAttachment, kstrSourcePath, nOnsendTimes);
                SYSTEMTIME stuPreTempFileLastModifyTime = ConvertStringToSysTime(wstrPreTempFileLastModifyTime);
                bModified = !IsSameSysTime(stuCurTempFileLastModifyTime, stuPreTempFileLastModifyTime);
                if (bModified)
                {
                    // Update
                    m_strTempFileLastModifyTimeDelay= ConverSysTimeToString(stuCurTempFileLastModifyTime);

                     //delay write last modify time to doing tag successed  
					//CMailPropertyTool::SetAttachmentProperty(spAttachment, PROP_NAME_PREOUTLOOKTEMPLASTMODIFYTIME, wstrCurTempFileLastModifyTime.c_str());
                }
                if ((1 == nOnsendTimes) && bModified)
                {
                    bModified = !IsSameBinaryFile(kstrSourcePath, kstrTempPath);
                }

            }
            else
            {
                NLPRINT_DEBUGVIEWLOG(L"!!!Error, get outlook temp:[%s] last modify time failed\n", kstrTempPath.c_str());
            }
        }
    }
    else
    {
        NLPRINT_DEBUGVIEWLOG(L"!!!Serious error, Outlook temp file path is empty");
    }
    return bModified;
}

void CAttachmentData::UpdateLastModifyTimePropertyForTempFile(CComPtr<Outlook::Attachment> spAttachment)
{
	if (!m_strTempFileLastModifyTimeDelay.empty())
	{
        NLPRINT_DEBUGVIEWLOG(L"testx on UpdateLastModifyTimePropertyForTempFile time=%s\n", m_strTempFileLastModifyTimeDelay.c_str() );
		CMailPropertyTool::SetAttachmentProperty(spAttachment, PROP_NAME_PREOUTLOOKTEMPLASTMODIFYTIME, m_strTempFileLastModifyTimeDelay.c_str());
		m_strTempFileLastModifyTimeDelay.clear();
	}	
}

int CSendEmailData::GetAttachmentFromMailItem(CComPtr<IDispatch> dspMailItem, const std::map<long,std::wstring>& map3thAppAttachmentPath, ITEM_TYPE origEmailType, bool bNeedAssociate)
{
	logd(L"[GetAttachmentFromMailItem]dspMailItem=%x", dspMailItem);
	CComPtr<Outlook::Attachments> spAttachments = NULL;

	HRESULT hr = MailItemUtility::get_Attachments(dspMailItem, &spAttachments,bNeedAssociate);
	if (SUCCEEDED(hr) && (NULL != spAttachments))
	{

		CAttachmentFileMgr& theAttachmentFileMgr = CAttachmentFileMgr::GetInstance();

		long lAttachCount = 0; 
		hr = spAttachments->get_Count(&lAttachCount);
		logd(L"[GetAttachmentFromMailItem]attachment count = %d\n",lAttachCount);
		if (SUCCEEDED(hr) && lAttachCount > 0)
		{
			for (long lCurIndex = lAttachCount; lCurIndex > 0; lCurIndex--)
			{
				CComPtr<Outlook::Attachment> spAttachment = NULL;
				hr = spAttachments->Item(CComVariant(lCurIndex), &spAttachment);

				if (SUCCEEDED(hr) && (NULL!=spAttachment))
				{
					// Get file path
					std::wstring wstrRealSourceFileFullPath  = L"";
					std::wstring wstrOETempFileFullPath = L"";
					bool bRet = theAttachmentFileMgr.GetAttachmentFilePath(spAttachment, wstrRealSourceFileFullPath, wstrOETempFileFullPath, emAttachmentFromUnknown, origEmailType);
					if (bRet && (!wstrOETempFileFullPath.empty()))
					{
						logd(L"Attachment %d wstrOETempFileFullPath %s",lCurIndex, wstrOETempFileFullPath.c_str());
						if (wstrRealSourceFileFullPath.empty())
						{
							std::map<long,std::wstring>::const_iterator itAttachPath = map3thAppAttachmentPath.find(lCurIndex);
							if (itAttachPath != map3thAppAttachmentPath.end() )
							{		
								wstrRealSourceFileFullPath = itAttachPath->second; 
								logd(L"This attach is added by 3th-part app, the source path is:%s\n", wstrRealSourceFileFullPath.c_str());
							}
							else
							{
								wstrRealSourceFileFullPath = wstrOETempFileFullPath;
								logd(L"Maybe it is a forward email. we set the source and dest path to the same one, path:%s\n", wstrRealSourceFileFullPath.c_str());
							}
						}
						logd(L"Attachment %d wstrRealSourceFileFullPath %s",lCurIndex, wstrRealSourceFileFullPath.c_str());
						CAttachmentData attachData(wstrRealSourceFileFullPath.c_str(), wstrOETempFileFullPath.c_str(),lCurIndex);
						
						//get tag information tagged by last time
						VECTOR_TAGPAIR vecTag;
						CMailPropertyTool::GetHCTagFromAttachmentProperty(spAttachment, vecTag);
						attachData.SetHCTagAlreadTagged(vecTag); 
						VECTOR_TAGPAIR::iterator it;
						for(it = vecTag.begin(); it != vecTag.end(); it++){
							logd(L"[GetAttachmentFromMailItem]HC tag key = %s", it->first.c_str());
							logd(L"[GetAttachmentFromMailItem]HC tag value = %s", it->second.c_str());
						}
						attachData.IncreaseAttachmentOnsendTimes(spAttachment);
						
						//CComBSTR sbsDisplayName;
						//spAttachment->get_DisplayName(&sbsDisplayName);
						//if(0 < sbsDisplayName.Length())
						//{
						//	attachData.SetDispName(std::wstring(sbsDisplayName));
						//}else
						//{
						//	attachData.SetDispName(GetFileName(wstrRealSourceFileFullPath));
						//}

						CComBSTR sbsFileName;
						hr = spAttachment->get_FileName(&sbsFileName);
						if(0 < sbsFileName.Length())
						{
							attachData.FileName(std::wstring(sbsFileName));
						}else
						{
							attachData.FileName(GetFileName(wstrRealSourceFileFullPath));
						}

						m_vecAttachData.push_back(attachData);

					}
					else
					{
						//can't get path from CAttachmentFileMgr, maybe this item was opened by 3th-part.
						logd(L"Get Attachment from theAttachmentFileMgr.GetAttachmentFilePath failed.");
					}
				}
			}
		}
	}

	return (int)m_vecAttachData.size();
}

void CSendEmailData::SetExistObligationTypeForAllAttachment(int nOblType)
{
	std::vector<CAttachmentData>::iterator itAttach = m_vecAttachData.begin();
	while (itAttach != m_vecAttachData.end())
	{
		itAttach->SetExistObligationType(nOblType);
		itAttach++;
	}
}

void CSendEmailData::PutAlertMessages(CSendEmailData &emailData)
{
	GetDeniedAttachmentMessages().put(emailData.GetDeniedAttachmentMessages());
	GetAllowedAttachmentMessages().put(emailData.GetAllowedAttachmentMessages());
	GetDeniedRecipientMessages().put(emailData.GetDeniedRecipientMessages());
	GetAllowedRecipientMessages().put(emailData.GetAllowedRecipientMessages());
	GetDeniedAttachmentRecipientMessages().put(emailData.GetDeniedAttachmentRecipientMessages());
	GetAllowedAttachmentRecipientMessages().put(emailData.GetAllowedAttachmentRecipientMessages());
	GetDeniedOtherMessages().put(emailData.GetDeniedOtherMessages());
	GetAllowedOtherMessages().put(emailData.GetAllowedOtherMessages());
}

void CSendEmailData::ExecuteInheritHeaderObliation()
{
	logd(L"[ExecuteInheritHeaderObliation]Entry: m_bInheritHeader=%d, m_bNeedToInheritHeader=%d, m_bEvalAgainForHeader=%d"
		, m_bInheritHeader, m_bNeedToInheritHeader, m_bEvalAgainForHeader);
	// If originally not InheritHeader but receive the InheritHeader obligation and it requires to Inherit
	// it's true only when m_bInheritHeader = false and m_bNeedToInheritHeader = true
	m_bEvalAgainForHeader = !m_bInheritHeader && m_bNeedToInheritHeader;
	m_bInheritHeader = m_bNeedToInheritHeader;
}

// return true if a recipient detail item with the same policy to the attachment item #itAttItem is found in the Recipient Messages #recMessages, 
static BOOL FindItemWithTheSamePolicyInRecipientMessages(AlertMessages::AlertDetailItems& attItems, 
	AlertMessages::AlertDetailItems::iterator itAttItem, std::wstring& wsMessageText, AlertMessages::Iteratable& recMessages){
	std::wstring& wsPolicyName = itAttItem->Policy();

	for (AlertMessages::Iteratable::iterator itRecMsg = recMessages.begin(); itRecMsg != recMessages.end(); ++itRecMsg)
	{
		logd(L"[FindItemWithTheSamePolicyInRecipientMessages]recipient message: %s", itRecMsg->first.c_str());
		if (wsMessageText == itRecMsg->first)
		{
			AlertMessages::AlertDetailItems& recItems = itRecMsg->second;
			// Traverse the recipient detail item list to find the item with the same policy. If found, this attachment with the policy should be removed.
			for (AlertMessages::AlertDetailItems::iterator itRecItem = recItems.begin(); itRecItem != recItems.end(); ++itRecItem)
			{
				logd(L"[FindItemWithTheSamePolicyInRecipientMessages]recipient item(cmd_%d):%s for %s", itRecItem->QueryResCmd(), itRecItem->Item().c_str(), itRecItem->Policy().c_str());
				if(itRecItem->Policy() == wsPolicyName) //itRecItem->QueryResCmd() == RECIPIENT_COMMAND && 
				{
					logd(L"[FindItemWithTheSamePolicyInRecipientMessages]found recipient detail item: %s for %s", itAttItem->Item().c_str(), wsPolicyName.c_str());
					return TRUE;
				}
			}
		}
	}
	return FALSE;
}

static void RemoveAttachmentAlertMessages(AlertMessages::Iteratable& recMessages, AlertMessages::Iteratable& attMessages)
{
	for (AlertMessages::Iteratable::iterator itAttMsg = attMessages.begin(); itAttMsg != attMessages.end();)
	{
		std::wstring& wsMessageText = itAttMsg->first;
		AlertMessages::AlertDetailItems& attItems = itAttMsg->second;
		for (AlertMessages::AlertDetailItems::iterator itAttItem = attItems.begin(); itAttItem != attItems.end();)
		{
			logd(L"[RemoveAttachmentAlertMessages]Attachment item: %s for %s", itAttItem->Item().c_str(), itAttItem->Policy().c_str());
			if (!itAttItem->Item().empty()) // If detail item is empty, ignore to compare policy it comes from.
			{
				BOOL itAttItemUpdated = FindItemWithTheSamePolicyInRecipientMessages(attItems, itAttItem, wsMessageText, recMessages);
				if (itAttItemUpdated)
				{
					itAttItem = attItems.erase(itAttItem);
					continue;
				}
				
			}
			++itAttItem;
		}

		if (attItems.empty())
		{
			itAttMsg = attMessages.erase(itAttMsg);
		}else
		{
			++itAttMsg;
		}
	}
}

/*

	+--------+--------------------------------------+------------+------------+------------+------------+
	|        |                                      | Policy-1   | Policy-2   | Policy-3   | Policy-4   |
	+--------+--------------------------------------+------------+------------+------------+------------+
	|        | header/subject/body                  |            |            |            |            |
	|        +--------------------------------------+------------+------------+------------+------------+
	| Policy | recipient                            | configured |            | configured |            |
	|        +--------------------------------------+------------+------------+------------+------------+
	|        | document                             |            | configured | configured |            |
	+--------+--------------------------------------+------------+------------+------------+------------+
	|        | ATTACHMENT_RECIPIENT_AS_USER_COMMAND | matched    | matched    | matched    | matched    |
	|        +--------------------------------------+------------+------------+------------+------------+
	| Result | RECIPIENT_COMMAND                    | matched    |            |            | matched    |
	|        +--------------------------------------+------------+------------+------------+------------+
	|        | other                                |            |            |            | matched    |
	+--------+--------------------------------------+------------+------------+------------+------------+

	If a policy is like Policy-1 of the above table, the means only recipient condition is configured.
	If this policy is matched, in fact only recipient is matched, when #ClassifyObligationType called, ATTACHMENT_COMMAND also will be matched,
	it will cause a message about attachment file to be collected. Because of this, it should be removed.
*/
void CSendEmailData::RemoveTrivialAlertMessages()
{
	std::vector<std::pair<int, int>> intersectIndexes;
	AlertMessages::Iteratable& recAllowedMessages = GetAllowedRecipientMessages().GetMessages();
	if (!recAllowedMessages.empty())
	{
		AlertMessages::Iteratable& attAllowedMessages = GetAllowedAttachmentMessages().GetMessages();
		RemoveAttachmentAlertMessages(recAllowedMessages, attAllowedMessages);
	}

	AlertMessages::Iteratable& recDeniedMessages = GetDeniedRecipientMessages().GetMessages();
	if (!recDeniedMessages.empty())
	{
		AlertMessages::Iteratable& attDeniedMessages = GetDeniedAttachmentMessages().GetMessages();
		RemoveAttachmentAlertMessages(recDeniedMessages, attDeniedMessages);
	}
}

void CTagTypeInfo::SetAddHCTag(VECTOR_TAGPAIR& vecTags)
{
	if (m_vecAddHCTagInfo.size()==0)
	{
		m_vecAddHCTagInfo = vecTags;
	}
	else
	{
		VECTOR_TAGPAIR::const_iterator itor;
		for (itor = vecTags.begin(); itor != vecTags.end(); itor++)
		{
			bool bFind = false;
			VECTOR_TAGPAIR::iterator Olditor;
			for (Olditor = m_vecAddHCTagInfo.begin(); Olditor != m_vecAddHCTagInfo.end(); Olditor++)
			{
				if (_wcsicmp(itor->first.c_str(), Olditor->first.c_str())==0)
				{
					bFind = true;
					break;
				}
			}
			if (bFind)
			{
				Olditor->second = itor->second;
			}
			else
			{
				m_vecAddHCTagInfo.push_back(*itor);
			}
		}
	}
}

//CTagTypeInfo implement
void CTagTypeInfo::SetTag(const VECTOR_TAGPAIR& vecTags, TAG_POS tagPos)
{
	//find the old tag
	VECTOR_TAGPAIR& refOldTag = (tagPos==TAG_POS_NXL) ? m_vecNxlTagInfo : (tagPos==TAG_POS_CUSTOM ? m_vecCustomTagInfo : m_vecSummaryTagInfo);

	//merge tag
	if (refOldTag.size()==0)
	{
		refOldTag = vecTags;
	}
	else
	{
		VECTOR_TAGPAIR::const_iterator itNewTag = vecTags.begin();
		while (itNewTag != vecTags.end())
		{

			VECTOR_TAGPAIR::iterator itOldTag = refOldTag.begin();
			while (itOldTag != refOldTag.end())
			{
				if (_wcsicmp(itOldTag->first.c_str(), itNewTag->first.c_str())==0)
				{
					break;
				}
				itOldTag++;
			}

			if (itOldTag!=refOldTag.end())//tag exist
			{
                itOldTag->second += L"|";
				itOldTag->second += itNewTag->second;
			}
			else
			{
				refOldTag.push_back(*itNewTag);
			}

			itNewTag++;
		}
	}
   
}



BOOL CTagTypeInfo::IsExistSameTagValue(const wstring& strOrgTagValue, const wstring& strTagValue)
{
	if (strTagValue.empty())
	{
		return TRUE;
	}

	vector<wstring> vectemp;
	boost::algorithm::split(vectemp,strOrgTagValue,boost::algorithm::is_any_of(L"|"));
	for(size_t n=0;n<vectemp.size();n++)
	{
		wstring strtagvalue = vectemp[n];
		boost::algorithm::trim(strtagvalue);
		if (_wcsicmp(strtagvalue.c_str(),strTagValue.c_str()) == 0)
		{
			return TRUE;
		}
	}
	return FALSE;
}



BOOL CTagTypeInfo::IsTagExist(const VECTOR_TAGPAIR& vecTags, TAG_POS tagPos, BOOL bCheckKeyAndValue)
{
	//find the old tag
	const VECTOR_TAGPAIR& refExistTag = (tagPos==TAG_POS_NXL) ? m_vecNxlTagInfo : (tagPos==TAG_POS_CUSTOM ? m_vecCustomTagInfo : m_vecSummaryTagInfo);

	//
	bool bFind = false;
	VECTOR_TAGPAIR::const_iterator itTag = vecTags.begin();
	while (itTag != vecTags.end())
	{
		bFind = false;
		VECTOR_TAGPAIR::const_iterator itOldTags = refExistTag.begin();
		while (itOldTags != refExistTag.end())
		{
            BOOL bKeyExist = _wcsicmp(itOldTags->first.c_str(), itTag->first.c_str())==0;
		    if (bKeyExist)
		    {
				if (!bCheckKeyAndValue)
				{
					bFind = true;
					break;
				}
				else
				{

					BOOL bValueExist = _wcsicmp(itOldTags->second.c_str(), itTag->second.c_str())==0;
					if (!bValueExist)
					{
						if (IsExistSameTagValue(itOldTags->second,itTag->second))
						{
							bFind = true;
							break;
						}
					}
					else
					{
						bFind = true;
						break;
					}
				}
			
		    }
			
		
			itOldTags++;
		}
		if (!bFind)
		{
			return FALSE;
		}

		itTag++;
	}

	return TRUE;
}


CAOblInfo::CAOblInfo()
{
	m_strValue = L"";
	m_lCount = 0;
    m_strDisplayOblName = L"";
	m_bDone = false; 
}
CAOblInfo::~CAOblInfo()
{

}

CAOblInfo& CAOblInfo::operator=(const CAOblInfo& caInfo)
{
	if ( this == &caInfo )
	{
		return *this;
	}
	
	this->m_lCount = caInfo.m_lCount;
	this->m_strDisplayOblName = caInfo.m_strDisplayOblName;
	this->m_strValue = caInfo.m_strValue;
	return *this;
}


bool CAOblSubjectData::SetEmailAddress_suject(CAOblInfo &Info)
{
	return SetCAData(m_EmailAddress_suject,Info);
}

bool CAOblSubjectData::SetCreditCardNum_suject(CAOblInfo &Info)
{
	return SetCAData(m_CreditCardNum_suject,Info);

}

bool CAOblSubjectData::SetCurrencyValue_suject(CAOblInfo &Info)
{
	return SetCAData(m_CurrencyValue_suject,Info);
}

bool CAOblSubjectData::SetPhoneNum_suject(CAOblInfo &Info)
{
	return SetCAData(m_PhoneNum_suject,Info);
}

bool CAOblSubjectData::SetSocialSecurityNum_suject(CAOblInfo &Info)
{
	return SetCAData(m_SocialSecurityNum_suject,Info);
}

bool CAOblSubjectData::SetIPAddress_suject(CAOblInfo &Info)
{
	return SetCAData(m_IPAddress_suject,Info);
}

bool CAOblSubjectData::SetDOB_suject(CAOblInfo &Info)
{
	return SetCAData(m_DOB_suject,Info);
}

bool CAOblSubjectData::SetMailingAdd_suject(CAOblInfo &Info)
{
	return SetCAData(m_MailingAdd_suject,Info);
}

bool CAOblSubjectData::SetKeyWord_suject(CAOblInfo &Info)
{
	return SetKeyWord(m_vecKeyWord_suject,Info);
}



bool CAOblBodyData::SetEmailAddress_body(CAOblInfo &Info)
{
	return SetCAData(m_EmailAddress_body,Info);
}

bool CAOblBodyData::SetCreditCardNum_body(CAOblInfo &Info)
{
	return SetCAData(m_CreditCardNum_body,Info);

}

bool CAOblBodyData::SetCurrencyValue_body(CAOblInfo &Info)
{
	return SetCAData(m_CurrencyValue_body,Info);
}

bool CAOblBodyData::SetPhoneNum_body(CAOblInfo &Info)
{
	return SetCAData(m_PhoneNum_body,Info);
}

bool CAOblBodyData::SetSocialSecurityNum_body(CAOblInfo &Info)
{
	return SetCAData(m_SocialSecurityNum_body,Info);
}

bool CAOblBodyData::SetIPAddress_body(CAOblInfo &Info)
{
	return SetCAData(m_IPAddress_body,Info);
}

bool CAOblBodyData::SetDOB_body(CAOblInfo &Info)
{
	return SetCAData(m_DOB_body,Info);
}

bool CAOblBodyData::SetMailingAdd_body(CAOblInfo &Info)
{
	return SetCAData(m_MailingAdd_body,Info);
}

bool CAOblBodyData::SetKeyWord_body(CAOblInfo &Info)
{
	return SetKeyWord(m_vecKeyWord_body,Info);
}

bool CAOblData::SetCAData(CAOblInfo &DesInfo, CAOblInfo &SrcInfo)
{
	if (DesInfo.GetDone())
	{
		return false;
	}
	if (SrcInfo.GetCount() > 0)
	{
		if (DesInfo.GetCount() <= 0 || DesInfo.GetCount() > SrcInfo.GetCount())
		{
			DesInfo = SrcInfo;
			return true;
		}
	}
	return false;
}

/*
true means the data had been changed, else not
*/
bool CAOblData::SetKeyWord(vector<CAOblInfo>& vecOblInfo, CAOblInfo &Info)
{
	bool bRet = false;
	if (Info.GetCount() > 0)
	{
		vector<CAOblInfo>::iterator itor; 
		bool bFind = false;
		for(itor = vecOblInfo.begin(); itor != vecOblInfo.end(); itor++ )
		{
			if (_wcsicmp(Info.GetValue().c_str(),itor->GetValue().c_str()) == 0 )
			{
				if (itor->GetDone())
				{
					return false;
				}
				bFind = true;
				if (itor->GetCount() <= 0 || itor->GetCount() > Info.GetCount())
				{
					itor->SetCount(Info.GetCount());
					bRet = true;
				}
				break;
			}
		}
		if (!bFind)
		{
			vecOblInfo.push_back(Info);
			bRet = true;
		}
	}
	return bRet;
}



emObligationExistType CSubjectData::SetCASubjectOblData(CAOblInfo &caInfo,int nOblType)
{
	if(nOblType == CONTENTREDACTION_EMAILADDRESS_SUBJECT)
	{
		if (!m_casubjectdata.SetEmailAddress_suject(caInfo))
		{
			return emOBL_EXIST_SAME;
		}		
	}

	if(nOblType == CONTENTREDACTION_CREDITCARDNUM_SUBJECT)
	{
		if (!m_casubjectdata.SetCreditCardNum_suject(caInfo))
		{
			return emOBL_EXIST_SAME;
		}		
	}


	if(nOblType == CONTENTREDACTION_CURRENCYVALUE_SUBJECT)
	{
		if (!m_casubjectdata.SetCurrencyValue_suject(caInfo))
		{
			return emOBL_EXIST_SAME;
		}		
	}

	if(nOblType == CONTENTREDACTION_PHONENUMBER_SUBJECT)
	{
		if (!m_casubjectdata.SetPhoneNum_suject(caInfo))
		{
			return emOBL_EXIST_SAME;
		}		
	}

	if(nOblType == CONTENTREDACTION_SOCIALSECURITYNUM_SUBJECT)
	{
		if (!m_casubjectdata.SetSocialSecurityNum_suject(caInfo))
		{
			return emOBL_EXIST_SAME;
		}		
	}

	if(nOblType == CONTENTREDACTION_IPV4Address_SUBJECT)
	{
		if (!m_casubjectdata.SetIPAddress_suject(caInfo))
		{
			return emOBL_EXIST_SAME;
		}		
	}

	if(nOblType == CONTENTREDACTION_DOB_SUBJECT)
	{
		if (!m_casubjectdata.SetDOB_suject(caInfo))
		{
			return emOBL_EXIST_SAME;
		}		
	}

	if(nOblType == CONTENTREDACTION_MAILINGADDRESS_SUBJECT)
	{
		if (!m_casubjectdata.SetMailingAdd_suject(caInfo))
		{
			return emOBL_EXIST_SAME;
		}		
	}


	if(nOblType == CONTENTREDACTION_KEYWORD_SUBJECT)
	{
		if (!m_casubjectdata.SetKeyWord_suject(caInfo))
		{
			return emOBL_EXIST_SAME;
		}		
	}
	return emOBL_NOT_EXIST;
}


emObligationExistType CBodyData::SetCABodyOblData(CAOblInfo &caInfo,int nOblType)
{
	if(nOblType == CONTENTREDACTION_EMAILADDRESS_BODY)
	{
		if (!m_cabodydata.SetEmailAddress_body(caInfo))
		{
			return emOBL_EXIST_SAME;
		}		
	}

	if(nOblType == CONTENTREDACTION_CREDITCARDNUM_BODY)
	{
		if (!m_cabodydata.SetCreditCardNum_body(caInfo))
		{
			return emOBL_EXIST_SAME;
		}		
	}


	if(nOblType == CONTENTREDACTION_CURRENCYVALUE_BODY)
	{
		if (!m_cabodydata.SetCurrencyValue_body(caInfo))
		{
			return emOBL_EXIST_SAME;
		}		
	}

	if(nOblType == CONTENTREDACTION_PHONENUMBER_BODY)
	{
		if (!m_cabodydata.SetPhoneNum_body(caInfo))
		{
			return emOBL_EXIST_SAME;
		}		
	}

	if(nOblType == CONTENTREDACTION_SOCIALSECURITYNUM_BODY)
	{
		if (!m_cabodydata.SetSocialSecurityNum_body(caInfo))
		{
			return emOBL_EXIST_SAME;
		}		
	}

	if(nOblType == CONTENTREDACTION_IPV4Address_BODY)
	{
		if (!m_cabodydata.SetIPAddress_body(caInfo))
		{
			return emOBL_EXIST_SAME;
		}		
	}

	if(nOblType == CONTENTREDACTION_DOB_BODY)
	{
		if (!m_cabodydata.SetDOB_body(caInfo))
		{
			return emOBL_EXIST_SAME;
		}		
	}

	if(nOblType == CONTENTREDACTION_MAILINGADDRESS_BODY)
	{
		if (!m_cabodydata.SetMailingAdd_body(caInfo))
		{
			return emOBL_EXIST_SAME;
		}		
	}


	if(nOblType == CONTENTREDACTION_KEYWORD_BODY)
	{
		if (!m_cabodydata.SetKeyWord_body(caInfo))
		{
			return emOBL_EXIST_SAME;
		}		
	}
	return emOBL_NOT_EXIST;
}


void CAttachmentData::SetWaringMsgInfo(WARNING_MSG_INFO &WarningMsgInfo)
{
	logd(L"[CAttachmentData::SetWaringMsgInfo]Header=%s, Display=%s", WarningMsgInfo.strHeaderTxt.c_str(), WarningMsgInfo.strDisplayTxt.c_str());
	m_WarningMsgInfo.bDoWarningMsg = true;
	m_WarningMsgInfo.strDisplayTxt = WarningMsgInfo.strDisplayTxt;
	m_WarningMsgInfo.strHeaderTxt = WarningMsgInfo.strHeaderTxt;
	m_WarningMsgInfo.strProceedBtnLabel = WarningMsgInfo.strProceedBtnLabel;
	m_WarningMsgInfo.strCancelBtnLabel = WarningMsgInfo.strCancelBtnLabel;
	m_WarningMsgInfo.strLogID = WarningMsgInfo.strLogID;
}


void CAttachmentData::GetHCTagInfo(wstring& strLogInfo)
{
	if ((m_nExistObligationType & LOOP_OBLIGATION_FILE_OHC) != 0)
	{
		strLogInfo += OE_OBLIGATION_DESCRIPTION;
		strLogInfo += L"Tag information is below:\n";
		VECTOR_TAGPAIR TagInfo = m_TagData.GetHCTagInfo().GetAddHCTag();

		wstring strTag;
		VECTOR_TAGPAIR::iterator itor;
		for (itor = TagInfo.begin(); itor != TagInfo.end(); itor++)
		{
			if (_wcsicmp(itor->first.c_str(),L"NLLastModifyTime") == 0)
			{
				continue;
			}
			if (!strTag.empty())
			{
				strTag = strTag + L";\n";
			}
			strTag += itor->first;
			strTag = strTag + L"=";
			strTag += itor->second;
		}
		strLogInfo += strTag;
	}
}

void CAttachmentData::GetAuditLogInfo(wstring& strLogInfo)
{

	if ((m_nExistObligationType & LOOP_OBLIGATION_HDR) != 0)
	{
		strLogInfo += OE_OBLIGATION_DESCRIPTION;
		strLogInfo += L"Hidden Data Removal \n";
	}

	if ((m_nExistObligationType & LOOP_OBLIGATION_VERIFY_RECIPIENTS) != 0)
	{
		strLogInfo += OE_OBLIGATION_DESCRIPTION;
		strLogInfo += L"Verify Recipients \n";
	}

	if ((m_nExistObligationType & LOOP_OBLIGATION_MUTIPLE_CLIENT_CONFIGURATION) != 0)
	{
		strLogInfo += OE_OBLIGATION_DESCRIPTION;
		strLogInfo += L"Multiple Client Confirmation \n";
	}

	if ((m_nExistObligationType & LOOP_OBLIGATION_MISSING_TAG) != 0)
	{
		strLogInfo += OE_OBLIGATION_DESCRIPTION;
		strLogInfo += L"Missing Tag \n";
	}

	if ((m_nExistObligationType & LOOP_OBLIGATION_REJECT_UNLESS_SILENT_OVERRIDE) != 0)
	{
		strLogInfo += OE_OBLIGATION_DESCRIPTION;
		strLogInfo += L"Reject Unless Silent Override \n";
	}

	if ((m_nExistObligationType & LOOP_OBLIGATION_OIRM_AUTOMATIC) != 0)
	{
		strLogInfo += OE_OBLIGATION_DESCRIPTION;
		strLogInfo += L"Outlook Integrated Rights Management - automatic \n";
	}

	if ((m_nExistObligationType & LOOP_OBLIGATION_OIRM_INTERACTIVE) != 0)
	{
		strLogInfo += OE_OBLIGATION_DESCRIPTION;
		strLogInfo += L"Outlook Integrated Rights Management - Interactive \n";
	}
	
	if ((m_nExistObligationType & LOOP_OBLIGATION_ZIP_ENCRYTION) != 0)
	{
		strLogInfo += OE_OBLIGATION_DESCRIPTION;
		strLogInfo += L"Password Based Encryption \n";
	}

	if ((m_nExistObligationType & LOOP_OBLIGATION_PGP_ENCRYTION) != 0)
	{
		strLogInfo += OE_OBLIGATION_DESCRIPTION;
		strLogInfo += L"Identity Based Encryption \n";
	}

	if ((m_nExistObligationType & LOOP_OBLIGATION_STRIP_ATTACHMENT) != 0)
	{
		strLogInfo += OE_OBLIGATION_DESCRIPTION;
		strLogInfo +=L"Strip Attachments \n";
	}

	if ((m_nExistObligationType & LOOP_OBLIGATION_EMAIL_APPROVAL) != 0)
	{
		strLogInfo += OE_OBLIGATION_DESCRIPTION;
		strLogInfo += L"Email Approval \n";
	}

	if ((m_nExistObligationType & LOOP_OBLIGATION_INTERNAL_USE_ONLY) != 0)
	{
		strLogInfo += OE_OBLIGATION_DESCRIPTION;
		strLogInfo += L"Internal Use Only \n";
	}

	if ((m_nExistObligationType & LOOP_OBLIGATION_DOMAIN_MISMATCH_CONFIRMATION) != 0)
	{
		strLogInfo += OE_OBLIGATION_DESCRIPTION;
		strLogInfo +=L"Domain Mismatch Confirmation \n";
	}


	if ((m_nExistObligationType & LOOP_OBLIGATION_MAIL_ATTR_PARSING) != 0)
	{
		strLogInfo += OE_OBLIGATION_DESCRIPTION;
		strLogInfo += L"MAIL_ATTRIBUTE_PARSING \n";
	}


	if ((m_nExistObligationType & LOOP_OBLIGATION_FILE_OHC) != 0)
	{
		strLogInfo += OE_OBLIGATION_DESCRIPTION;
		strLogInfo += L"Outlook: Hierarchical Classification \n";
	}


	VECTOR_TAGPAIR vecTempAddedInteractiveCustomTag = GetAttachmentTagData().GetTempAddedInteractiveTagCustomTag();
	if(!vecTempAddedInteractiveCustomTag.empty())
	{
		wstring strTag;
		VECTOR_TAGPAIR::iterator itor;
		for(itor = vecTempAddedInteractiveCustomTag.begin(); itor != vecTempAddedInteractiveCustomTag.end();itor++)
		{
			if (!strTag.empty())
			{
				strTag = strTag + L";";
			}
			strTag += itor->first;
			strTag = strTag + L"=";
			strTag += itor->second;
		}
		strLogInfo += L"Interactive File Tagging Added Tag: ";
		strLogInfo += strTag;
		strLogInfo += L" \n";
	}

    VECTOR_TAGPAIR vecTempAddedAutoTagCustomTag = GetAttachmentTagData().GetTempAddedAutoTagCustomTag(); 
	if(!vecTempAddedAutoTagCustomTag.empty())
	{
		wstring strAutoTag;
		VECTOR_TAGPAIR::iterator itor;
		for(itor = vecTempAddedAutoTagCustomTag.begin(); itor != vecTempAddedAutoTagCustomTag.end();itor++)
		{
			if (!strAutoTag.empty())
			{
				strAutoTag = strAutoTag + L";";
			}
			strAutoTag += itor->first;
			strAutoTag = strAutoTag + L"=";
			strAutoTag += itor->second;
		}
		strLogInfo += L"Automatic File Tagging Added Tag: ";
		strLogInfo += strAutoTag;
		strLogInfo += L" \n";
	}

}

BOOL CAttachmentData::IsFileNameChangedAfterObligation()
{
	std::wstring wstrTempFileName = GetFileName(m_wstrTempPath);
	std::wstring wstrOrgFileName = GetFileName(m_wstrOrgTempPath);
	return _wcsicmp(wstrTempFileName.c_str(), wstrOrgFileName.c_str())!=0;
}

int CAttachmentData::IncreaseAttachmentOnsendTimes(CComPtr<Outlook::Attachment> spAttachment)
{
	m_nAttachmentSendTimes = GetAttachmentOnSendTimes(spAttachment);
	m_nAttachmentSendTimes++;
	SetAttachmentOnSendTimes(spAttachment, m_nAttachmentSendTimes);
	logi(L"attachment%#x,send times %d.",spAttachment, m_nAttachmentSendTimes);
	return m_nAttachmentSendTimes;
}

int CAttachmentData::GetAttachmentOnSendTimes(CComPtr<Outlook::Attachment> spAttachment)
{
	std::wstring wstrOnSendTimes = CMailPropertyTool::GetAttachmentProperty(spAttachment, PROP_NAME_ATTACHMENT_SENDTIMES);
	if (wstrOnSendTimes.size()>0)
	{
		return wcstol(wstrOnSendTimes.c_str(), NULL, 10);
	}
	else
	{
		logd(L"[GetAttachmentOnSendTimes] GetAttachmentOnSendTimes failed. default=0\n");
		return 0;
	}
}
bool CAttachmentData::GetAttachmentUpdateProperty(CComPtr<Outlook::Attachment> spAttachment)
{
	std::wstring wstrIsChanged = CMailPropertyTool::GetAttachmentProperty(spAttachment, PROP_NAME_ATTACHMENT_ISCHANGED);
	logd(L"[GetAttachmentUpdateProperty]value=%s", wstrIsChanged.c_str());
	std::wstring wstrValue = L"yes";
	if (0 == _wcsicmp(wstrIsChanged.c_str(), wstrValue.c_str()))
	{
		return true;
	}
	else
	{
		return false;
	}
}

void CAttachmentData::ExtractSpecificTags(std::vector<std::pair<std::wstring,std::wstring>>& attrs, CComPtr<IDispatch> MailItem)
{
	//logd(L"[ExtractSpecificTags]GetAttachmentByIndex 1-based #%d", GetOriginalAttachIndex());
	CComPtr<Outlook::Attachment> spAttachment = NULL;
	if(OLUtilities::GetAttachmentByIndex(MailItem, GetOriginalAttachIndex() - 1, &spAttachment))
	{
		HRESULT hr;

		// 1. file size

		long filesize = 0; //long: -2147483648 ~ 2147483647 
		hr = spAttachment->get_Size(&filesize);
		attrs.push_back( std::make_pair<>( L"size", boost::lexical_cast<std::wstring>( filesize ) ) );

		// 2. file full name
		// bug 44594 [oe8.5]can't deny send email by file fullname

		CComBSTR sbsFileName;
		std::wstring wsFileName;
		hr = spAttachment->get_FileName(&sbsFileName);
		if(0 < sbsFileName.Length())
		{
			wsFileName = sbsFileName;
		}else
		{
			//wsFileName = boost::filesystem::wpath(pAttachmentData->GetSourcePath()).filename(); // #include <boost/filesystem/path.hpp>
			//wsFileName = GetFileName(pAttachmentData->GetSourcePath()); // #include "../common/CommonTools.h"
			wsFileName = PathFindFileNameW(GetSourcePath().c_str()); // #include "Shlwapi.h"
		}
		attrs.push_back( std::make_pair<>( L"original_name", wsFileName ) );
		
		// 3. file extension name
		// bug 44592 [oe8.5]can't deny send email by file type

		// the address of the "." that precedes the extension within pszPath if an extension is found
		LPCWSTR pwsFileExt = PathFindExtensionW( wsFileName.c_str() );
		if (NULL != pwsFileExt && *pwsFileExt) // non-null and non-empty
		{
			//attrs.push_back( std::make_pair<>( L"type", boost::filesystem::extension( wsFileName ) ) ); // #include <boost/filesystem.hpp>
			attrs.push_back( std::make_pair<>( L"type",  pwsFileExt + 1) );
		}

		logd(L"[ExtractSpecificTags]File: size=%ld, name=%s, type=%s", filesize, wsFileName.c_str(), pwsFileExt);
	}else
	{
		logw(L"[ExtractSpecificTags]GetAttachmentByIndex at %d failed", GetOriginalAttachIndex());
	}
}

void CAttachmentData::SetAttachmentOnSendTimes(CComPtr<Outlook::Attachment> spAttachment, int nOnSendTimes)
{
	wchar_t wszOnSendTimes[100];
	wsprintfW(wszOnSendTimes, L"%d", nOnSendTimes);
	CMailPropertyTool::SetAttachmentProperty(spAttachment, PROP_NAME_ATTACHMENT_SENDTIMES, wszOnSendTimes);
}
void CAttachmentData::SetAttachmentUpdateProperty(CComPtr<Outlook::Attachment> spAttachment)
{
	wchar_t wszIsChanged[100];
	std::wstring wstrValue = L"yes";
	wsprintfW(wszIsChanged, L"%s", wstrValue.c_str());
	CMailPropertyTool::SetAttachmentProperty(spAttachment, PROP_NAME_ATTACHMENT_ISCHANGED, wszIsChanged);
}
int CMessageHeader::MergePairs(const VECTOR_TAGPAIR& from, VECTOR_TAGPAIR& to)
{
	logd(L"[MergePairs]ToSize=%d, FromSize=%d\n", to.size(), from.size());
	if (to.size()==0)
	{
		to = from;
		return from.size();
	}
	else
	{
		int changes = 0;
		VECTOR_TAGPAIR::const_iterator itor;
		for (itor = from.begin(); itor != from.end(); itor++)
		{
			bool bFind = false;
			VECTOR_TAGPAIR::iterator Olditor;
			for (Olditor = to.begin(); Olditor != to.end(); Olditor++)
			{
				if (_wcsicmp(itor->first.c_str(), Olditor->first.c_str())==0)
				{
					bFind = true;
					break;
				}
			}
			if (bFind)
			{
				logd(L"[MergePairs]Overwrite \"%s\"=\"%s\"'s value to \"%s\"", Olditor->first.c_str(), Olditor->second.c_str(), itor->second.c_str());
				
				if(Olditor->second != itor->second)
				{
					++changes;
					Olditor->second = itor->second;
				}
			}
			else
			{
				logd(L"[MergePairs]Incremental pair \"%s\"=\"%s\"", itor->first.c_str(), itor->second.c_str());
				to.push_back(*itor);
			}
		}
		return changes;
	}
}

void CMessageHeader::DebugPrintPairs(const VECTOR_TAGPAIR& pairs, const wchar_t *pszLabel /*= L"X-Nextlabs Header Pair"*/)
{
	int nIndex;
	std::vector< std::pair<std::wstring,std::wstring> >::const_iterator itPair;
	if (NULL == pszLabel)
	{
		pszLabel = L"DebugPrintPairs";
	}
	logd(L"%s:size=%d", pszLabel, pairs.size());
	for(nIndex = 0, itPair = pairs.begin(); itPair != pairs.end(); ++itPair, ++nIndex) 
	{
		logd(L"%s:%d \"%s\"=\"%s\"\n", pszLabel, nIndex, itPair->first.c_str(), itPair->second.c_str());
	}
}

bool CMessageHeader::GetHeaderFromMailItem(CComPtr<IDispatch> dspMailItem)
{
	HRESULT hr;

	// Read the transport message headers from the item by PropertyAccessor with the key #PR_TRANSPORT_MESSAGE_HEADERS_URL
	// If the item is created by parsing an attachment item that is an outlook item, it will be not empty if headers exist.
	// If empty in the previous step, reading headers by UserProperties with the key P_NAME_ORIGIN_MESSAGE_HEADER_W, it will be
	// empty if is created by pressing the New Mail button or the original item after Reply/replyAll/Forward has not headers.
	
	CComVariant varPropValue;

	CComPtr<Outlook::_PropertyAccessor> propAcc = CMailPropertyTool::GetPropAccessorFromMailItem(dspMailItem);
	hr = propAcc->GetProperty(CComBSTR(PR_TRANSPORT_MESSAGE_HEADERS_URL), &varPropValue);
	if (SUCCEEDED(hr) && varPropValue.bstrVal)
	{
		m_wstrHeader = varPropValue.bstrVal;
	}else
	{
		loge(L"[GetHeaderFromMailItem]PropertyAccessor.GetProperty, HRESULT=%#x", hr);
	}

	if (m_wstrHeader.empty())
	{
		ATL::CComVariant varUserProperties;
		hr = dspMailItem.GetPropertyByName(W2COLE(L"UserProperties"), &varUserProperties);
		if (FAILED(hr) || VT_DISPATCH != varUserProperties.vt)
		{
			loge(L"[GetHeaderFromMailItem] Find HRESULT=%#x, UserProperties@%#x\n", hr, varUserProperties.pdispVal);
			return false;
		}

		//UserProperty = NewItem.UserProperties.Find(L"OriginMessageHeader")
		ATL::CComPtr<Outlook::UserProperties> spUserProperties((Outlook::UserProperties*)varUserProperties.pdispVal);

		ATL::CComBSTR sbsPropertyName(P_NAME_ORIGIN_MESSAGE_HEADER_W);
		ATL::CComPtr<Outlook::UserProperty> spUserProperty;
		ATL::CComVariant varCustom(VARIANT_TRUE);
		hr = spUserProperties->Find(sbsPropertyName, varCustom, &spUserProperty);
		if (FAILED(hr) || NULL == spUserProperty)
		{
			logw(L"[GetHeaderFromMailItem] Find HRESULT=%#x, UserProperty@%#x\n", hr, spUserProperty);
			return false;
		}
		ATL::CComVariant varMessageHeader;
		hr = spUserProperty->get_Value(&varMessageHeader);
		if (FAILED(hr) || VT_BSTR != varMessageHeader.vt)
		{
			logw(L"[GetHeaderFromMailItem] get_Value HRESULT=%#x, BSTR@%#x\n", hr, (LPCWSTR)varMessageHeader.bstrVal);
			return false;
		}

		m_wstrHeader = (LPCWSTR)varMessageHeader.bstrVal;
	}
	//logi(L"\n\nGetHeaderFromMailItem: %s\n\n", m_wstrHeader.c_str());
	return true;
}

bool CMessageHeader::InheritHeaderIfNeeded(std::wstring& wsExtraHeaders)
{
	if (!m_bHeaderInherited)
	{
		// parse to retrieve the key-value pairs;
		MsgHeaderParser::HEADER_PAIRS allHeaderPairs;
		MsgHeaderParser(m_wstrHeader.c_str()).ExtractHeaders(allHeaderPairs);

		if (!allHeaderPairs.empty())
		{
			DebugPrintPairs(allHeaderPairs, L"AllHeaderPairs");
			DebugPrintPairs(m_vecHeaderPairs, L"XHeaderPairs");
			DebugPrintPairs(m_vecNextlabsHeaderParis, L"NextlabsXHeaderPairs");
			
			// Those headers in need of inheriting is specified by X-Nextlabs or in #wsExtraHeaders, which will be provided for querying PC.
			// Whereas only those headers with keys matching in X-Nextlabs values will be tagged.

			logd(L"[InheritHeaderIfNeeded]extraHeaderKeys = %s", wsExtraHeaders.c_str());

			std::set<std::wstring> extraHeaderKeys;
			boost::function<bool(wchar_t)> delimiterPred = boost::is_any_of(L",\t ");
			boost::algorithm::trim_if(wsExtraHeaders, delimiterPred); // could also use plain boost::trim
			boost::algorithm::split(extraHeaderKeys, wsExtraHeaders, delimiterPred, boost::token_compress_on);

			// OE is concerned only with the pairs specified by the key X-Nextlabs while writing or executing tagging obligation
			// Thus, finally the sent mail only holds the header pairs specified by the key X-Nextlabs because Outlook originally
			//  doesn't automatically add user customized x-headers.

			if (!m_vecNextlabsHeaderParis.empty()) // Have executed header related obligations
			{
				MsgHeaderParser::HEADER_PAIRS vecHeaderPairs, vecNextlabsHeaderParis;
				std::wstring nextlabsKeys = MsgHeaderParser::FilterHeader(allHeaderPairs, extraHeaderKeys, vecHeaderPairs, vecNextlabsHeaderParis);

				DebugPrintPairs(vecHeaderPairs, L"InheritedXHeaderPairs");
				logd(L"[InheritHeaderIfNeeded]nextlabs tag keys = %s", nextlabsKeys.c_str());

				CMessageHeader::MergePairs(m_vecHeaderPairs, vecHeaderPairs); // put (auto/HC) tags into the inherited header pairs
				m_vecHeaderPairs = vecHeaderPairs; // update header pairs

				CMessageHeader::MergePairs(m_vecNextlabsHeaderParis, vecNextlabsHeaderParis); // put (auto/HC) tags into the inherited header pairs
				m_vecNextlabsHeaderParis = vecNextlabsHeaderParis; // update nextlabs header pairs

				DebugPrintPairs(m_vecNextlabsHeaderParis, L"LatestNextlabsXHeaderPairs");
			}else
			{
				std::wstring nextlabsKeys = MsgHeaderParser::FilterHeader(allHeaderPairs, extraHeaderKeys, m_vecHeaderPairs, m_vecNextlabsHeaderParis);

				DebugPrintPairs(m_vecHeaderPairs, L"InheritedXHeaderPairs");
				logd(L"[InheritHeaderIfNeeded]nextlabs tag keys = %s", nextlabsKeys.c_str());
			}
		}else
		{
			logd(L"InheritHeaderIfNeeded: extract empty collection for HeaderPairs\n");
		}

		m_bHeaderInherited = true;
		return true;
	}
	return false;
}

HRESULT CMessageHeader::ResetNLIncrementalHeaderProperty(IDispatch *pItemDisp)
{
	HRESULT hr;
	// Retrieve the incremental pairs property that's set by OE. If exists, we need to get rid of it.
	CComPtr<Outlook::UserProperties> spUserProperties = CMailPropertyTool::GetUserPropertiesFromItem(pItemDisp);
	if (NULL != spUserProperties)
	{
		LPCWSTR pwszPropName = L"NLIncrementalHeaders";
		CComPtr<Outlook::UserProperty> spUserProperty;
		hr = spUserProperties->Find(ATL::CComBSTR(pwszPropName), CComVariant(VARIANT_TRUE), &spUserProperty);
		if (SUCCEEDED(hr) && NULL != spUserProperty)
		{
			logd(L"[ResetNLIncrementalHeaderProperty]This email was once processed by OE");

			ATL::CComVariant varUserPropertyValue;
			hr = spUserProperty->get_Value(&varUserPropertyValue);
			if (VT_BSTR == varUserPropertyValue.vt)
			{
				logd(L"[ResetNLIncrementalHeaderProperty]\"%s\"=\"%s\"", pwszPropName, (LPCWSTR)varUserPropertyValue.bstrVal);
			}
			hr = spUserProperty->Delete();
		}
	}
	return hr;
}

BOOL CMessageHeader::UpdateEmailHeaders(CComPtr<IDispatch> dspMailItem)
{
	// ## Finally put new header into this mail

	// @See [Commonly Used Property Sets - PS_INTERNET_HEADERS](https://msdn.microsoft.com/en-us/library/office/gg318108.aspx)
	// Put the header pairs into the mail property named PS_INTERNET_HEADERS {00020386-0000-0000-C000-000000000046}
	// When the mail is sent, the header pairs can be seen in File>Properties:internet headers or use the following powershell 
	// code(MUST select one item in an Outlook explorer window, two version property name that can output X-Header).
	// PS C:\Users\ssfang> 
	/*
	[Runtime.InteropServices.Marshal]::GetActiveObject('Outlook.Application').ActiveExplorer().Selection.`
	Item(1).PropertyAccessor.GetProperties(@('http://schemas.microsoft.com/mapi/proptag/0x007D001F'
	, 'http://schemas.microsoft.com/mapi/proptag/0x007d001e'))
	*/
	VECTOR_TAGPAIR& headeTags = m_vecHeaderPairs;
	if (0 < headeTags.size())
	{
		// @TODO we can indirectly use mail's PropertyAccessor to remember the tags(MUST consider that it can be persistent
		//  when a user saves the mail as draft so as to send it in the future)

		// Retrieve the incremental pairs tagged last time
		LPCWSTR pwszPropName = L"NLIncrementalHeaders";
		CComPtr<Outlook::UserProperties> spUserProperties = CMailPropertyTool::GetUserPropertiesFromItem(dspMailItem);
		std::wstring sTag;
		CMailPropertyTool::FindUserProperty(spUserProperties, pwszPropName, sTag);
		std::vector<std::pair<std::wstring,std::wstring> > vecLastHeaderTag;
		if (!sTag.empty())
		{
			CMailPropertyTool::GetTagsAsVectory(sTag, vecLastHeaderTag);
		}
		logi(L"[UpdateEmailHeaders]The incremental pairs last time(count=%d): %s", vecLastHeaderTag.size(), sTag.c_str());

		if (!vecLastHeaderTag.empty())
		{

			// Erase those in last tags but not in current tags
			// e.g. 
			//             | last tagPairs | current tagPairs |
			// ============+===============+==================+
			//             | x-foo:f1      | x-foo:f2         |
			//  + add      | x-bar:b1      |                  |
			//             |               | x-tux:q2         |
			// ------------+---------------+------------------+
			//  - del      |               | x-bar            |
			// ------------|---------------+------------------+
			//  x-nextlabs | x-foo:x-bar   | x-foo:x-tux      |
			// ------------+---------------+------------------+
			// 
			// std::vector<HEADER_PAIRS> nextlabsPairs = Mail.GetProperty("NL"); std::vector<HEADER_PAIRS> tagPairs = autoTags + HCTags;
			// Obtain a difference sets of tagPairs with respect to nextlabsPairs to remove those pairs tagged last time but not in the 
			// current pairs to tag because those pairs already have been putted into mail.
			for( std::vector<std::pair<std::wstring,std::wstring> >::iterator itLastTag = vecLastHeaderTag.begin();itLastTag != vecLastHeaderTag.end();)
			{
				// If it exists in the pairs tagged last time, need erase it from current pairs
				BOOL isIteratorUpdated = FALSE;
				std::vector<std::pair<std::wstring,std::wstring> >::iterator itLast = vecLastHeaderTag.begin();
				for( ;itLast != vecLastHeaderTag.end(); ++itLast)
				{
					if (itLast->first == itLastTag->first)
					{
						itLastTag = vecLastHeaderTag.erase(itLastTag); // The iterator #itLastTag is updated here
						isIteratorUpdated = TRUE;
						break;
					}
				}
				if (!isIteratorUpdated)
				{
					++itLastTag; // The iterator #itLastTag is updated here
				}
			}

			//Now, we can remove redundant tag pairs compared to the last
			if (!vecLastHeaderTag.empty())
			{
				CMailPropertyTool::DeleteMessageHeaderPairs(dspMailItem, vecLastHeaderTag);
			}
		}


		// Update nextlabs pairs in the item's UserProperty
		sTag = CMailPropertyTool::GetTagsAsString(headeTags);
		LPCWSTR pwzTag = sTag.c_str();
		logi(L"[UpdateEmailHeaders]The nextlabs header pairs(count=%d): %s", headeTags.size(), pwzTag);
		CMailPropertyTool::AddUserProperty(spUserProperties, pwszPropName, pwzTag);

		//Put new header into mail
		HRESULT hr = CMailPropertyTool::PutMessageHeaderPairs(dspMailItem, headeTags, boost::iequals(m_wsErrorAction, DECISION_ON_TAG_ERROR_BLOCK));
		if (E_ABORT == hr)
		{
			extern HMODULE g_hTAG;
			typedef int (*FnShowTagErrorBlockMessage)(HWND hParent, LPCWSTR szFileName, LPCWSTR szMessage);
			FnShowTagErrorBlockMessage pfFnShowTagErrorBlockMessage = (FnShowTagErrorBlockMessage)GetProcAddress(g_hTAG, "ShowTagErrorBlockMessage");

			pfFnShowTagErrorBlockMessage(NULL, L"Email Header", m_wsMessageIfBlcok.c_str());

			return FALSE;
		}
	}
	return TRUE;
}

void AlertMessages::put(LPCWSTR pwzAlertMsg, LPCWSTR pwzPolicy, const std::wstring& wsItem, EmQueryResCommand emQueryResCmd, int index /*= 0*/)
{
	Iteratable & vecMessages = m_messages;
	BOOL bAlertTextExist = FALSE;
	std::wstring wsPolicyName(NULL != pwzPolicy ? pwzPolicy : L"");
	const AlertDetailItem alertDetailItem = AlertDetailItem(wsItem, wsPolicyName, emQueryResCmd, index);
	const AlertDetailItem::Matcher itemMatcher = {wsItem.c_str(), index};
	for(Iteratable::iterator it = vecMessages.begin(); it != vecMessages.end(); ++it)
	{
		if (0 == it->first.compare(pwzAlertMsg))
		{
			bAlertTextExist = TRUE;

			if (!wsItem.empty()) // ignore empty item of a message
			{
				AlertDetailItems& objects = it->second;

				if (objects.end() == std::find_if(objects.begin(), objects.end(), itemMatcher))
				{
					logd(L"[AlertMessages::put]add a new detail item, cmd_%d, \"%s\":#%d\"%s\" for %s", emQueryResCmd, pwzAlertMsg, index, wsItem.c_str(), pwzPolicy);
					objects.push_back(alertDetailItem);
				}else
				{
					logw(L"[AlertMessages::put]do nothing for a duplicated item cmd_%d, \"%s\":#%d\"%s\" for %s", emQueryResCmd, pwzAlertMsg, index, wsItem.c_str(), pwzPolicy);
				}
			}else
			{
				logw(L"[AlertMessages::put]do nothing for an empty item cmd_%d, \"%s\" for %s", emQueryResCmd, pwzAlertMsg, pwzPolicy);
			}
		}
	}
	if (!bAlertTextExist)
	{
		logd(L"[AlertMessages::put]put a new message, \"%s\":#%d\"%s\" for %s", pwzAlertMsg, index, wsItem.c_str(), pwzPolicy);
		AlertDetailItems vecObjects;
		if (!wsItem.empty()) // ignore empty item of a message, meaning no detail item
		{
			vecObjects.push_back(alertDetailItem);
		}
		vecMessages.push_back(std::make_pair<>(pwzAlertMsg, vecObjects));
	}
}

void AlertMessages::put(const AlertMessages& alertMessages)
{
	Iteratable & vecDstMessages = m_messages;
	logd(L"[AlertMessages::put]put another(%d) to this(%d)", vecDstMessages.size(), alertMessages.GetMessages().size());

	struct
	{
		bool operator()(const AlertMessage& alertMsg) const
		{
			return alertMsg.first == *pText;
		}
		const std::wstring* pText;
	}matcher;

	for(Iteratable::const_iterator itSrcMsg = alertMessages.GetMessages().begin(); itSrcMsg != alertMessages.GetMessages().end(); ++itSrcMsg)
	{
		matcher.pText = &itSrcMsg->first;
		Iteratable::iterator itDstItems = std::find_if( vecDstMessages.begin(), vecDstMessages.end(), matcher );
		if(vecDstMessages.end() != itDstItems)
		{
			itDstItems->second.insert( itDstItems->second.end(), itSrcMsg->second.begin(), itSrcMsg->second.end() );
		}else
		{
			vecDstMessages.push_back( std::make_pair<>(itSrcMsg->first, itSrcMsg->second) );
		}
	}
}

void AlertMessages::StringifyAndAppendTo(std::wstring & wsMessage)
{
	Iteratable & vecMessages = m_messages;
	logd(L"[StringifyAndAppendTo] message amount is %d", vecMessages.size());
	for(Iteratable::iterator itAlertMsg = vecMessages.begin(); itAlertMsg != vecMessages.end(); ++itAlertMsg)
	{
		AlertMessage & message = *itAlertMsg;
		AlertDetailItems& objects = message.second;
		wsMessage += L"<b>";
		wsMessage.append(message.first);
		wsMessage += L"</b>";
		for (AlertDetailItems::iterator itObject = objects.begin(); objects.end() != itObject; ++itObject)
		{
			wsMessage += L"<br>  - ";
			wsMessage.append(itObject->Item());
		}
		wsMessage += L"<br>";
	}
}
