#include "StdAfx.h"
#include "QueryPCInfo.h"
#include "eframework/platform/cesdk_loader.hpp"
#include "eframework/platform/cesdk_attributes.hpp"
#include "../common/policy.h"
#include <fstream>
#include "../outlook/DataType.h"
#include "../ca/caadapter.h"
#include "outlookUtilities.h"
#include "../outlook/MailItemUtility.h"
#include "strsafe.h"

extern nextlabs::cesdk_loader cesdk;
extern std::wstring g_strOETempFolder;

CQueryPCInfo::CQueryPCInfo(void)
{
}

CQueryPCInfo::~CQueryPCInfo(void)
{
}

void CQueryPCInfo::SetTickTime()
{
	wchar_t strTime[MAX_PATH+1] = {0};
	StringCchPrintf(strTime,MAX_PATH,L"%d",::GetTickCount());
	m_strTickTime = strTime;
}

bool CQueryPCInfo::ConstructRequestFromEmailData(CSendEmailData& emailData,CComPtr<IDispatch> MailItem)
{
	//set tick time
    SetTickTime();

	//Construct reqeust for "EMAIL" action
	ConstructRequestForEmailAction(emailData, MailItem);

	//Construct request for "RECEIVE" action
	ConstructRequestForReceiveAction(emailData,MailItem);

	//Construct request for "RECEIVE" action for each recipient with empty resource and attribute
	ConstructRequestForReceiveActionOnEachRecipient(emailData,MailItem);

	return true;
}


bool CQueryPCInfo::ConstructRequestForEmailAction(CSendEmailData& emailData, CComPtr<IDispatch> olItem)
{
	//Operation value
	const CEString ceOperation = cesdk.fns.CEM_AllocateString(L"EMAIL");
	m_releaseResource.m_vecCEstring.push_back(ceOperation);

	//Recipients
	const STRINGLIST& refVecRecipients = emailData.GetRecipientsData().GetRealRecipients();
	const CEint32 numRecipients = (CEint32)refVecRecipients.size();
	CEString  *recipients  = new CEString[numRecipients];
	for (int i=0; i<numRecipients; i++)
	{
		recipients[i] = cesdk.fns.CEM_AllocateString(CharLowerW(const_cast<wchar_t *>(refVecRecipients[i].c_str())));
	}
	m_releaseResource.m_vecRecipientsAddr.push_back(recipients);
	
    //User and the attribute
	CEUser* user = new CEUser;
	user->userID   = cesdk.fns.CEM_AllocateString(PolicyCommunicator::GetUserSID());
	user->userName = cesdk.fns.CEM_AllocateString(CharLowerW(PolicyCommunicator::m_wzSenderName));
	m_releaseResource.m_vecUser.push_back(user);

	CEAttributes* UserAttributes = new CEAttributes;
	memset(UserAttributes, 0,sizeof(CEAttributes));
	UserAttributes->count = 1;
	UserAttributes->attrs = new CEAttribute;
	if(NULL != UserAttributes->attrs)
	{
		UserAttributes->attrs->key = cesdk.fns.CEM_AllocateString(L"OUTLOOK_USER");
		UserAttributes->attrs->value = cesdk.fns.CEM_AllocateString(L"SENDER");
	}
	m_releaseResource.m_vecUserAttributes.push_back(UserAttributes);

	//Application
	CEApplication* app = new CEApplication;
	app->appURL    = cesdk.fns.CEM_AllocateString(L"");
	app->appName   = cesdk.fns.CEM_AllocateString(PolicyCommunicator::GetAppName());
	app->appPath   = cesdk.fns.CEM_AllocateString(PolicyCommunicator::GetAppPath());
	m_releaseResource.m_vecApp.push_back(app);

	//subject request
	CEResource* pSourceSubject = NULL;
	CEAttributes* pSourceAttributeSubject = NULL;
	CreateSource(SOUCETYPE_SUBJECT, &emailData.GetSubjectData(), &pSourceSubject, &pSourceAttributeSubject, olItem);
    int nRequestIndex = AddRequest(ceOperation, user, UserAttributes, recipients, numRecipients, pSourceSubject, pSourceAttributeSubject);
	emailData.GetSubjectData().SetRecordPos(nRequestIndex);
	logd(L"[ConstructRequestForEmailAction]SOUCETYPE_SUBJECT RequestIndex = %d", nRequestIndex);

	//body request
	CEResource* pSourceBody = NULL;
	CEAttributes* pSourceAttributeBody = NULL;
	CreateSource(SOUCETYPE_BODY, &emailData.GetBodyData(), &pSourceBody, &pSourceAttributeBody, olItem);
	nRequestIndex = AddRequest(ceOperation, user, UserAttributes, recipients, numRecipients, pSourceBody, pSourceAttributeBody);
	emailData.GetBodyData().SetRecordPos(nRequestIndex);
	logd(L"[ConstructRequestForEmailAction]SOUCETYPE_BODY RequestIndex = %d", nRequestIndex);

	//Construct an message header request
	logd(L"[ConstructRequestForEmailAction]emailData.GetInheritHeader() = %d", emailData.GetInheritHeader());
	if (emailData.GetInheritHeader())
	{
		//If reach here, this method should be second called. When inheriting its original headers, we also consider that in the 
		//first request process, some header related obligations can be executed so need to merge new tags to inherited headers
		emailData.GetMessageHeader().InheritHeaderIfNeeded(emailData.ExtraHeadersNeedInheriting());

		CEResource* pSourceXHeader = NULL;
		CEAttributes* pSourceAttributeXHeader = NULL;
		CreateSource(SOUCETYPE_X_HEADER, &emailData.GetMessageHeader(), &pSourceXHeader, &pSourceAttributeXHeader, olItem);
		nRequestIndex = AddRequest(ceOperation, user, UserAttributes, recipients, numRecipients, pSourceXHeader, pSourceAttributeXHeader);
		emailData.GetMessageHeader().SetRecordPos(nRequestIndex);
		logd(L"[ConstructRequestForEmailAction]SOUCETYPE_X_HEADER RequestIndex = %d", nRequestIndex);
	}

	//attachment request
	std::vector<CAttachmentData>& refVecAttachmentData = emailData.GetAttachmentData(); 
	std::vector<CAttachmentData>::iterator itAttachment = refVecAttachmentData.begin();
	int nUnremovedAttachmentCount = 0;
	while (itAttachment != refVecAttachmentData.end())
	{
		if (!itAttachment->IsAttachmentRemoved())
		{
			CEResource* pSourceAttach = NULL;
			CEAttributes* pSourceAttributeAttach = NULL;
			SourceDataForAttachment sourceData = {emailData, *itAttachment};
			CreateSource(SOUCETYPE_ATTACHMENT, &sourceData, &pSourceAttach, &pSourceAttributeAttach, olItem);
			nRequestIndex = AddRequest(ceOperation, user, UserAttributes, recipients, numRecipients, pSourceAttach, pSourceAttributeAttach);
			itAttachment->SetRecordPos(nRequestIndex);
			++nUnremovedAttachmentCount;
		}

		itAttachment++;
	}
	emailData.SetHasUnremovedAttachment(nUnremovedAttachmentCount);
   return true;
}


bool CQueryPCInfo::CreateSource(EM_SOURCETYPE sourceType, void* pSourceData, CEResource** ppCEResource, CEAttributes** ppSourceAttr, CComPtr<IDispatch> MailItem /* = NULL */)
{
	if (NULL==pSourceData)
	{
		return false;
	}
	std::wstring wsEmailType = L"other"; 
	ITEM_TYPE eEmailType = DEFAULT;
	OLUtilities::CheckGetMailItemType(MailItem, eEmailType);
	if (eEmailType == MAIL_ITEM){
		wsEmailType = L"email";
	}else if(eEmailType == APPOINTMENT_ITEM || eEmailType == MEETING_ITEM){
		wsEmailType = L"meeting";
	}else if(eEmailType == TASK_ITEM || eEmailType == TASK_REQUEST_ITEM){
		wsEmailType = L"task";
		}
	
	std::vector<std::pair<std::wstring,std::wstring>> attrs;
	logd(L"[CreateSource]email type=%s", wsEmailType.c_str());
	if (wsEmailType != L"other"){
		std::pair<std::wstring,std::wstring> attrEmailType(L"emailtype",wsEmailType);
		attrs.push_back(attrEmailType);
	}
	//get source path
	std::wstring wstrSourcePath;
	if (sourceType == SOUCETYPE_SUBJECT)
	{
		wstrSourcePath = CreateTempFileForQueryPC(((CSubjectData*)pSourceData)->GetTempSubject().c_str(), L".txt");

		std::pair<std::wstring,std::wstring> attrContentType(NL_CA_CONTENTTYPE,NL_CA_CONTENTTYPE_SUBJECT);
		attrs.push_back(attrContentType);

		m_releaseResource.m_vecFilePath.push_back(wstrSourcePath);
	}
	else if (sourceType==SOUCETYPE_BODY)
	{
		wstrSourcePath = CreateTempFileForQueryPC(((CBodyData*)pSourceData)->GetTempBody().c_str(), L".txt");

		std::pair<std::wstring,std::wstring> attrContentType(NL_CA_CONTENTTYPE,NL_CA_CONTENTTYPE_BODY);
		attrs.push_back(attrContentType);

		m_releaseResource.m_vecFilePath.push_back(wstrSourcePath);
	}
	else if(SOUCETYPE_X_HEADER == sourceType)
	{
		logd(L"[CreateSource]SOUCETYPE_X_HEADER == sourceType");
		CMessageHeader *pMessageHeader = (CMessageHeader*)pSourceData;
		wstrSourcePath = CreateTempFileForQueryPC(pMessageHeader->GetContent().c_str(), L".txt");

		attrs.push_back(std::make_pair<wstring,wstring>(NL_CA_CONTENTTYPE, L"Email X-header"));
		pMessageHeader->PutTagsInAttributes(attrs);

		CMessageHeader::DebugPrintPairs(attrs, L"[CreateSource]X-Header Attribute");

		m_releaseResource.m_vecFilePath.push_back(wstrSourcePath);
	}
	else if(SOUCETYPE_RECIPIENT == sourceType)
	{
		SourceDataForRecipient* pSrcDataForRecipient = (SourceDataForRecipient*)pSourceData;
		logd(L"[CreateSource] SOUCETYPE_RECIPIENT == sourceType, HasUnremovedAttachment=%d", pSrcDataForRecipient->emailData.HasUnremovedAttachment());
		wstrSourcePath = CreateTempFileForQueryPC(L"", L".txt");
		attrs.push_back(std::make_pair<wstring,wstring>(NL_CA_CONTENTTYPE, L"Email Recipient"));

		// Need to send XHeader when no attachment request is sen, see Bug 45872 
		if(!pSrcDataForRecipient->emailData.HasUnremovedAttachment())
		{
			pSrcDataForRecipient->emailData.GetMessageHeader().PutTagsInAttributes(attrs);
		}
	}
	else
	{
		logd(L"[CreateSource] sourceType == SOUCETYPE_ATTACHMENT");
		SourceDataForAttachment* pSrcData = (SourceDataForAttachment*)pSourceData;
		CAttachmentData* pAttachmentData = & pSrcData->data;
		wstrSourcePath = pAttachmentData->GetTempPath();
		attrs.push_back(pair<wstring,wstring>(L"SourcePath",pAttachmentData->GetSourcePath()));
		const VECTOR_TAGPAIR& TagInfo = pAttachmentData->GetHCTagAlreadTagged();
		for (VECTOR_TAGPAIR::const_iterator itHCTag = TagInfo.begin(); itHCTag != TagInfo.end(); ++itHCTag)
		{
			CMessageHeader::PutTagInAttributes(*itHCTag, attrs);
		}

		pAttachmentData->ExtractSpecificTags(attrs, MailItem);

		//attrs.insert(attrs.end(), TagInfo.begin(),TagInfo.end());

		// Need to include XHeader (BTW, except for file tags, XHeader tags also are provided as a query criteria), see Bug 45872 
		pSrcData->emailData.GetMessageHeader().PutTagsInAttributes(attrs);
	}

	//create source
	CEResource *source = cesdk.fns.CEM_CreateResource(wstrSourcePath.c_str(), L"fso");
	m_releaseResource.m_vecResource.push_back(source);
	*ppCEResource = source;

	//create source attribute
    CEAttributes    *sourceAttributes = new CEAttributes;   
	memset(sourceAttributes, 0,sizeof(CEAttributes));

	WCHAR wzLastModifyDate[MAX_PATH+1]= L"123456789";
	OLUtilities::GetFileLastModifyTime(wstrSourcePath.c_str(), wzLastModifyDate, MAX_PATH);

	attrs.push_back(pair<wstring,wstring>(L"OUTLOOK_SENDTIME",m_strTickTime));
	attrs.push_back(pair<wstring,wstring>(L"ce::nocache",L"yes"));

	const CEint32	numAttrs=(CEint32)attrs.size();
	CEint32 numAttrsIndex=0;
	if((sourceType == SOUCETYPE_SUBJECT) || (sourceType == SOUCETYPE_BODY))
	{
		sourceAttributes->count = 2+numAttrs;
		sourceAttributes->attrs = new CEAttribute[sourceAttributes->count];
		if(NULL != sourceAttributes->attrs)
		{
			for(numAttrsIndex=0;numAttrsIndex<numAttrs;numAttrsIndex++)
			{
				sourceAttributes->attrs[numAttrsIndex].key	=cesdk.fns.CEM_AllocateString(attrs[numAttrsIndex].first.c_str());
				sourceAttributes->attrs[numAttrsIndex].value	=cesdk.fns.CEM_AllocateString(attrs[numAttrsIndex].second.c_str());
			}
			sourceAttributes->attrs[numAttrs+0].key   = cesdk.fns.CEM_AllocateString(L"modified_date");
			sourceAttributes->attrs[numAttrs+0].value = cesdk.fns.CEM_AllocateString(wzLastModifyDate);

			sourceAttributes->attrs[numAttrs+1].key   = cesdk.fns.CEM_AllocateString(L"resolved_name");
			sourceAttributes->attrs[numAttrs+1].value = cesdk.fns.CEM_AllocateString(wstrSourcePath.c_str());
		}
	}
	else
	{	
		
		bool bCount = false;

		sourceAttributes->count = 2 + numAttrs;

		sourceAttributes->attrs = new CEAttribute[sourceAttributes->count];
		if(NULL != sourceAttributes->attrs)
		{
			for(numAttrsIndex=0;numAttrsIndex<numAttrs;numAttrsIndex++)
			{
				sourceAttributes->attrs[numAttrsIndex].key=cesdk.fns.CEM_AllocateString(attrs[numAttrsIndex].first.c_str());
				sourceAttributes->attrs[numAttrsIndex].value=cesdk.fns.CEM_AllocateString(attrs[numAttrsIndex].second.c_str());
			}

			sourceAttributes->attrs[numAttrs].key   = cesdk.fns.CEM_AllocateString(L"modified_date");
			sourceAttributes->attrs[numAttrs].value = cesdk.fns.CEM_AllocateString(wzLastModifyDate);
			sourceAttributes->attrs[numAttrs+1].key   = cesdk.fns.CEM_AllocateString(L"resolved_name");
			sourceAttributes->attrs[numAttrs+1].value = cesdk.fns.CEM_AllocateString(wstrSourcePath.c_str());

			if (bCount)
			{
				sourceAttributes->attrs[numAttrs+3].key   = cesdk.fns.CEM_AllocateString(L"CE::nocache");
				sourceAttributes->attrs[numAttrs+3].value = cesdk.fns.CEM_AllocateString(L"yes");
			}
		}
	}

	m_releaseResource.m_vecSourceAttributes.push_back(sourceAttributes);
	*ppSourceAttr = sourceAttributes;
	return true;
}

//the return value is the index of the request, this is also the index of the enforcement;
int CQueryPCInfo::AddRequest(CEString ceOperator, CEUser* pCEUser, CEAttributes* pUserAttributes, CEString* pArrayRecipients,
							  CEint32 numRecipients, CEResource* pSource, CEAttributes* pSourceAttribute)
{
	//set request
	COPParaMeterCERequest request;
	request.SetOperation(&ceOperator);
	request.SetUser(&pCEUser);
	request.SetUserAttribute(&pUserAttributes);
	request.SetRecipients(&pArrayRecipients);
	request.SetNumRecipients(numRecipients);
	request.SetCEResource(&pSource);
	request.SetSourceAttribute(&pSourceAttribute);
	m_vecCERequest.push_back(request.m_CERequest);

	//enforcement
	CEEnforcement_t enf;
	enf.result = CEAllow;
	enf.obligation = NULL;
	m_vecEnforcement.push_back(enf);

	//
	return (int)m_vecEnforcement.size()-1;
}



std::wstring CQueryPCInfo::CreateTempFileForQueryPC(const wchar_t* pBuffer, const wchar_t* wszExt)
{
    if (pBuffer == NULL)    return L"";
	std::wstring wstrFileName;
	wchar_t wzFileName[MAX_PATH] = {0};
    DWORD dwBufLen = wcslen(pBuffer);
	if(GetTempFileName(g_strOETempFolder.c_str(),L"NLCA",0,wzFileName))
	{
		::DeleteFile(wzFileName);
		wstrFileName=wzFileName;
		wstrFileName.append(wszExt);
		
		//create the file
		std::fstream file(wstrFileName.c_str(), std::ios_base::out);
		if ( (dwBufLen<=0) || (dwBufLen>0xff000000) )
		{
			file.close();//without write anything to the file, fstream will create a empty file
			return wstrFileName;
		}
		

        int nLen = WideCharToMultiByte(CP_ACP, 0, pBuffer, dwBufLen, NULL, 0, NULL, NULL);
        if (nLen == 0)  return L"";

        char* pBuf = new char[nLen + 1];
        if (!pBuf)    return L"";
        memset(pBuf, 0, nLen + 1);
        nLen = WideCharToMultiByte(CP_ACP, 0, pBuffer, dwBufLen, pBuf, nLen, NULL, NULL);
        if (nLen > 0)
        {
            if (file.is_open())
            {
                file.write(pBuf, nLen);
                file.close();
            }

        }

        if (pBuf)
        {
            delete[]pBuf;
            pBuf = NULL;
        }

		return wstrFileName;
	}
	else
	{
		DP((L"CQueryPCInfo::CreateTempFileForQueryPC failed to create temp file.\n"));
		return L"";
	}
}

void CQueryPCInfo::GetMeetingSender(CSendEmailData& emailData,CComPtr<IDispatch> MailItem,wstring& strCurrentSender)
{
	CComPtr<Outlook::_Account> spAccount = 0;
	BOOL bSuccess = FALSE;
	HRESULT hr = S_OK;

	CComPtr<Outlook::_MeetingItem> spCurMeetItem = 0;
	CComPtr<Outlook::_AppointmentItem> spCurAppItem = 0;
	if (emailData.GetEmailType() == APPOINTMENT_ITEM)
	{

		hr = MailItem->QueryInterface(Outlook::IID__AppointmentItem, (void**)&spCurAppItem);
		if(SUCCEEDED(hr) && spCurAppItem)
		{	
			//hr = spCurAppItem->GetOrganizer();
			hr = spCurAppItem->get_SendUsingAccount(&spAccount);
			if (SUCCEEDED(hr) && spAccount)
			{
				bSuccess = TRUE;
			}
		}
	}
	else
	{
		hr = MailItem->QueryInterface(Outlook::IID__MeetingItem, (void**)&spCurMeetItem);
		if(SUCCEEDED(hr) && spCurMeetItem)
		{	
			hr = spCurMeetItem->get_SendUsingAccount(&spAccount);
			if (SUCCEEDED(hr) && spAccount)
			{
				bSuccess = TRUE;
			}
		}
	}

	if (bSuccess)
	{
		CComBSTR btrDisplayName;
		hr = spAccount->get_DisplayName(&btrDisplayName);
		if (SUCCEEDED(hr) && btrDisplayName.m_str != NULL)
		{
			strCurrentSender = btrDisplayName.m_str;
			logd(L"[CQueryPCInfo::GetMeetingSender]get_DisplayName successed.");
		}
		else
		{
			CComBSTR btrSmtpAddress;
			hr = spAccount->get_SmtpAddress(&btrSmtpAddress);
			if (SUCCEEDED(hr) && NULL != btrSmtpAddress.m_str)
			{
				strCurrentSender = btrSmtpAddress.m_str;
			}
			else
			{
				CComPtr<Outlook::Recipient>    spRecipient = 0;
				CComPtr<Outlook::AddressEntry>  spAddrEntry = 0;
				hr = spAccount->get_CurrentUser(&spRecipient);

				if (SUCCEEDED(hr))
				{
					hr = spRecipient->get_AddressEntry(&spAddrEntry);
					if (S_OK==hr && spAddrEntry)
					{
						STRINGLIST listRecipients;
						OLUtilities::ExpandAddressEntry(spAddrEntry, listRecipients);
						if (listRecipients.size() > 0)
						{
							strCurrentSender = listRecipients[0];
						}
					}
				}
			}
		}
	}

}

bool CQueryPCInfo::ConstructRequestForReceiveAction(CSendEmailData& emailData,CComPtr<IDispatch> MailItem)
{
	//Operation value
	const CEString ceOperation = cesdk.fns.CEM_AllocateString(L"EMAIL");
	m_releaseResource.m_vecCEstring.push_back(ceOperation);

	//Application
	CEApplication* app = new CEApplication;
	app->appURL    = cesdk.fns.CEM_AllocateString(L"");
	app->appName   = cesdk.fns.CEM_AllocateString(PolicyCommunicator::GetAppName());
	app->appPath   = cesdk.fns.CEM_AllocateString(PolicyCommunicator::GetAppPath());
	m_releaseResource.m_vecApp.push_back(app);

	//attachment request
	std::vector<CAttachmentData>& refVecAttachmentData = emailData.GetAttachmentData(); 
	std::vector<CAttachmentData>::iterator itAttachment = refVecAttachmentData.begin();
	while (itAttachment != refVecAttachmentData.end())
	{
		if (!itAttachment->IsAttachmentRemoved())
		{
			CEResource* pResourceAttach = NULL;
			CEAttributes* pResourceAttachAttr = NULL;
			SourceDataForAttachment sourceData = {emailData, *itAttachment};
			CreateSource(SOUCETYPE_ATTACHMENT, &sourceData, &pResourceAttach, &pResourceAttachAttr, MailItem);

			//construct request for every recipients
			const STRINGLIST& refLstRecipients = emailData.GetRecipientsData().GetRealRecipients();

			
			STRINGLIST LstRecipients;

			if ((emailData.GetEmailType() == MEETING_ITEM || emailData.GetEmailType() == APPOINTMENT_ITEM) && !emailData.GetForward())
			{
				wstring strCurrentSender = L"";

				//GetMeetingSender(emailData,MailItem,strCurrentSender);
				strCurrentSender = emailData.GetSender();
				logd(L"[ConstructRequestForReceiveAction]strCurrentSender = %s",strCurrentSender.c_str());
				if (strCurrentSender.empty())
				{
					logd(L"[meeting||appointment]Current sender is empty.");
					LstRecipients = refLstRecipients;
				}
				else
				{
					logd(L"[meeting||appointment]Current sender = %s.", strCurrentSender.c_str());
					for (size_t index = 0; index < refLstRecipients.size(); index++)
					{
						logd(L"[ConstructRequestForReceiveAction]refLstRecipients[%d]=%s", index, refLstRecipients[index].c_str());
						if (_wcsicmp(refLstRecipients[index].c_str(),strCurrentSender.c_str()) == 0)
						{	
							emailData.GetRecipientsData().SetSenderInRecipint(strCurrentSender);//add by OE 8.5, we can't know why

							//we add filter when do reject until silent and rich user alert; we need a recipients to send this request, so push_back it too 
							LstRecipients.push_back(refLstRecipients[index]);//when email type=meeting or appoint,the api get_recipients will get both recipients and sender
							continue;
						}
						LstRecipients.push_back(refLstRecipients[index]);
					}
				}
				logd(L"[ConstructRequestForReceiveAction]meeting/appointment LstRecipients.size()=%d",LstRecipients.size());
				//not remove recipient event it is equal sender, so annotate the code
				//if (0 == LstRecipients.size()){
					//LstRecipients.push_back(L"nxl@nextlabs.com");//when the meeting recipient is none, set a fake recipient
				//}
			}
			else
			{
				LstRecipients = refLstRecipients;
			}

			emailData.GetRecipientsData().SetSendPCRecipients(LstRecipients);

			
			for (size_t nReceiverIndex =0; nReceiverIndex<LstRecipients.size(); nReceiverIndex++)
			{
				
				//Recipients
				const CEint32 numRecipients = (CEint32)1;
				CEString  *recipients  = new CEString[numRecipients];			
				recipients[0] = cesdk.fns.CEM_AllocateString((const_cast<wchar_t *>(LstRecipients[nReceiverIndex].c_str())));
				m_releaseResource.m_vecRecipientsAddr.push_back(recipients);

				//User and the attribute
				CEUser* user = new CEUser;
				//user->userID   = cesdk.fns.CEM_AllocateString(LstRecipients[nReceiverIndex].c_str());
				//user->userName = cesdk.fns.CEM_AllocateString((LstRecipients[nReceiverIndex].c_str()));
				user->userID   = cesdk.fns.CEM_AllocateString(PolicyCommunicator::GetUserSID());
				user->userName = cesdk.fns.CEM_AllocateString(CharLowerW(PolicyCommunicator::m_wzSenderName));
				m_releaseResource.m_vecUser.push_back(user);

				CEAttributes* UserAttributes = new CEAttributes;
				memset(UserAttributes, 0,sizeof(CEAttributes));
				UserAttributes->count = 1;
				UserAttributes->attrs = new CEAttribute;
				if(NULL != UserAttributes->attrs)
				{
					UserAttributes->attrs->key = cesdk.fns.CEM_AllocateString(L"OUTLOOK_USER");
					UserAttributes->attrs->value = cesdk.fns.CEM_AllocateString(L"RECIPIENT");
					//UserAttributes->attrs->value = cesdk.fns.CEM_AllocateString(L"SENDER");
				}
				m_releaseResource.m_vecUserAttributes.push_back(UserAttributes);

				int nRecordPos = AddRequest(ceOperation, user, UserAttributes, recipients, numRecipients, pResourceAttach, pResourceAttachAttr);
				itAttachment->AddRecordPosForReceiveAction(nRecordPos);
				itAttachment->AddReceiverIndexForReceiveAction((int)nReceiverIndex);
			}

		}
		itAttachment++;
	}

	return true;
}

//Construct request for "RECEIVE" action for each recipient with fake resource and attribute
bool CQueryPCInfo::ConstructRequestForReceiveActionOnEachRecipient(CSendEmailData& emailData, CComPtr<IDispatch> MailItem)
{
	//Operation value
	const CEString ceOperation = cesdk.fns.CEM_AllocateString(L"EMAIL");
	m_releaseResource.m_vecCEstring.push_back(ceOperation);

	//Application
	CEApplication* app = new CEApplication;
	app->appURL    = cesdk.fns.CEM_AllocateString(L"");
	app->appName   = cesdk.fns.CEM_AllocateString(PolicyCommunicator::GetAppName());
	app->appPath   = cesdk.fns.CEM_AllocateString(PolicyCommunicator::GetAppPath());
	m_releaseResource.m_vecApp.push_back(app);

	//Recipient request for every single one.
	std::vector<int> &vecRecordPosForRecipients = emailData.GetRecordPosForRecipients();
	
	CEResource* pResForRecipient = NULL;
	CEAttributes* pResAttrForRecipient = NULL;
	SourceDataForRecipient sourceData = {emailData, vecRecordPosForRecipients};
	CreateSource(SOUCETYPE_RECIPIENT, &sourceData, &pResForRecipient, &pResAttrForRecipient, MailItem);

	//construct request for every recipients
	const STRINGLIST& refLstRecipients = emailData.GetRecipientsData().GetRealRecipients();
	vecRecordPosForRecipients.resize(refLstRecipients.size());

	logd(L"[ConstructRequestForReceiveActionOnEachRecipient] Count=%d", refLstRecipients.size());

	for (size_t nReceiverIndex =0; nReceiverIndex < refLstRecipients.size(); nReceiverIndex++)
	{
		logd(L"[ConstructRequestForReceiveActionOnEachRecipient] index=%d", nReceiverIndex);
		//Recipients but only one
		const CEint32 numRecipients = (CEint32)1;
		CEString  *recipients  = new CEString[numRecipients];			
		recipients[0] = cesdk.fns.CEM_AllocateString((const_cast<wchar_t *>(refLstRecipients[nReceiverIndex].c_str())));
		m_releaseResource.m_vecRecipientsAddr.push_back(recipients);

		//User and the attribute
		CEUser* user = new CEUser;
		//user->userID   = cesdk.fns.CEM_AllocateString(LstRecipients[nReceiverIndex].c_str());
		//user->userName = cesdk.fns.CEM_AllocateString((LstRecipients[nReceiverIndex].c_str()));
		user->userID   = cesdk.fns.CEM_AllocateString(PolicyCommunicator::GetUserSID());
		user->userName = cesdk.fns.CEM_AllocateString(CharLowerW(PolicyCommunicator::m_wzSenderName));
		m_releaseResource.m_vecUser.push_back(user);

		CEAttributes* UserAttributes = new CEAttributes;
		memset(UserAttributes, 0,sizeof(CEAttributes));
		UserAttributes->count = 1;
		UserAttributes->attrs = new CEAttribute;
		if(NULL != UserAttributes->attrs)
		{
			UserAttributes->attrs->key = cesdk.fns.CEM_AllocateString(L"OUTLOOK_USER");
			UserAttributes->attrs->value = cesdk.fns.CEM_AllocateString(L"RECIPIENT");
			//UserAttributes->attrs->value = cesdk.fns.CEM_AllocateString(L"SENDER");
		}
		m_releaseResource.m_vecUserAttributes.push_back(UserAttributes);

		int nRecordPos = AddRequest(ceOperation, user, UserAttributes, recipients, numRecipients, pResForRecipient, pResAttrForRecipient);
		vecRecordPosForRecipients[nReceiverIndex] = nRecordPos;
	}

	return true;
}
