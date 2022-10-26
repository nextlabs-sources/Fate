#include "StdAfx.h"
#include "PAMngr.h"
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include "MailPropertyTool.h"
#include "outlookUtilities.h"

CMailPropertyTool::CMailPropertyTool(void)
{
}

CMailPropertyTool::~CMailPropertyTool(void)
{
}

HRESULT CMailPropertyTool::SetAttachmentProperty(CComPtr<Outlook::Attachment> spAttachment, const wchar_t* wszPropName, const wchar_t* wszPropValue)
{
	CComPtr<Outlook::_PropertyAccessor> pPropAccess;
	HRESULT hr = spAttachment->get_PropertyAccessor(&pPropAccess);
	if (SUCCEEDED(hr) && pPropAccess)
	{
         hr = SetProperty(pPropAccess, wszPropName, wszPropValue);
	}

	return hr;
}

std::wstring CMailPropertyTool::GetAttachmentProperty(CComPtr<Outlook::Attachment> spAttachment, const wchar_t* wszPropName)
{
	std::wstring wstrValue;
	CComPtr<Outlook::_PropertyAccessor> pPropAccess;
	HRESULT hr = spAttachment->get_PropertyAccessor(&pPropAccess);
	if (SUCCEEDED(hr) && pPropAccess)
	{
	    wstrValue = GetProperty(pPropAccess, wszPropName);
	}

	return wstrValue;
}

void CMailPropertyTool::SetHCTagToAttachmentProperty(CComPtr<Outlook::Attachment> spAttachment, const std::vector<std::pair<std::wstring,std::wstring>>& vectHCTagInfo)
{		
	std::wstring strHCTag = GetTagsAsString(vectHCTagInfo);
	SetAttachmentProperty(spAttachment, PROP_NAME_HCTAG, strHCTag.c_str());
}

void CMailPropertyTool::GetHCTagFromAttachmentProperty(CComPtr<Outlook::Attachment> spAttachment, std::vector<std::pair<std::wstring,std::wstring>>& vectHCTagInfo)
{
	vectHCTagInfo.clear();

	std::wstring wstrTags = GetAttachmentProperty(spAttachment, PROP_NAME_HCTAG);
    GetTagsAsVectory(wstrTags, vectHCTagInfo);
}

std::wstring CMailPropertyTool::GetTagsAsString(const std::vector<std::pair<std::wstring,std::wstring>>& vectHCTagInfo)
{
	std::wstring wstrTagReturn;
	std::vector<std::pair<std::wstring,std::wstring>>::const_iterator itTag = vectHCTagInfo.begin();
	while (itTag != vectHCTagInfo.end())
	{
		std::wstring wstrTag = itTag->first.c_str();
		wstrTag += TAG_PROP_SEPRATOR_KEYVALUE;
		wstrTag += itTag->second.c_str();
		wstrTag += TAG_PROP_SEPRATOR_TAGS;

		wstrTagReturn += wstrTag;

		itTag++;
	}

	return wstrTagReturn;
}

void CMailPropertyTool::GetTagsAsVectory(const std::wstring& wstrTags, std::vector<std::pair<std::wstring,std::wstring>>& vectHCTagInfo)
{
	std::vector<std::wstring> vectTags;
	boost::split(vectTags, wstrTags,boost::is_any_of(TAG_PROP_SEPRATOR_TAGS));

	std::vector<std::wstring>::iterator itTag = vectTags.begin();
	while (itTag != vectTags.end())
	{
		std::wstring wstrTag = *itTag;
		int nPos = wstrTag.find(TAG_PROP_SEPRATOR_KEYVALUE);
		if (nPos!=std::wstring::npos)
		{
			std::wstring wstrKey = wstrTag.substr(0, nPos);
			std::wstring wstrValue = wstrTag.substr(nPos+1);

			vectHCTagInfo.push_back(std::pair<std::wstring,std::wstring>(wstrKey, wstrValue));
		}

		itTag++;
	}
}

HRESULT CMailPropertyTool::MakeSafeBSTRArrayVariant(OUT VARIANT *dst, IN const std::vector<std::pair<std::wstring, std::wstring> >& src, const wchar_t* pwzElementSuffix /*= L""*/)
{
	HRESULT hr = S_OK;
	int nCounter = src.size();
	if(nCounter)
	{
		SAFEARRAYBOUND  Bound;
		Bound.lLbound   = 0;
		Bound.cElements = nCounter;
		SAFEARRAY* pSafeArrayOfBSTRs = SafeArrayCreate(VT_BSTR, 1, &Bound);
		if(pSafeArrayOfBSTRs == NULL)
			return E_OUTOFMEMORY;

		BSTR* pBSTRs=NULL;

		dst->vt = VT_ARRAY | VT_BSTR; 
		dst->parray = pSafeArrayOfBSTRs;

		hr = SafeArrayAccessData(pSafeArrayOfBSTRs, (void**)&pBSTRs);
		if (SUCCEEDED(hr) && pBSTRs)
		{
			std::wstring wsElement(pwzElementSuffix);
			std::wstring::size_type nMarkOff = wsElement.length();
			std::wstring::const_iterator nMarkPosition = wsElement.end();
			while(--nCounter >= 0)
			{
				// Override all characters begins at the position specified by #nMarkOff until the end of the (right) string
				// If this is greater than right string's length, this will be truncated because the second parameter #npos.
				wsElement.replace(nMarkOff, std::wstring::npos, src[nCounter].first);
				// std::wstring& tmp = wsElement.replace(nMarkOff, std::wstring::npos, L"");
				// wprintf(L"%s\n", wsElement.c_str());
				pBSTRs[nCounter] = W2BSTR(wsElement.c_str());
			}
			hr = SafeArrayUnaccessData(pSafeArrayOfBSTRs);
		}
	}else // @TODO 
	{ 
		// CComVariant(_In_ const SAFEARRAY *pSrc) if pSrc is NULL, {VT_ERROR, E_INVALIDARG}
		// How to represents a valid value like @C#`Object[] objects = null`?
		// dst                       hr = ::VariantClear(dst)
		//{VT_ARRAY | VT_NULL, NULL} hr = 0x80020008 Bad variable type.
		//{VT_ARRAY, NULL}           hr = 0x80020008 Bad variable type.
		//{VT_NULL, NULL}            hr = 0 S_OK The operation completed successfully. 
		dst->vt = VT_NULL;
		dst->parray = NULL;
	}
	return hr;
}

HRESULT CMailPropertyTool::DelAttachmentProperty(CComPtr<Outlook::Attachment> spAttachment, const wchar_t* wszPropName)
{
#if 1

	//currettly we found even if we delete the property. the property still exist on the email at receiver side.
	return SetAttachmentProperty(spAttachment, wszPropName, L""); 

#else 
	CComPtr<Outlook::_PropertyAccessor> pPropAccess;
	HRESULT hr = spAttachment->get_PropertyAccessor(&pPropAccess);
	if (SUCCEEDED(hr) && pPropAccess)
	{
		std::wstring wstrSchmepName = PROP_NAME_ROOT;
		wstrSchmepName += wszPropName;
		CComBSTR bstrScheName = wstrSchmepName.c_str();
		hr = pPropAccess->DeleteProperty(bstrScheName);

		DP((L"testx DelAttachmentProperty name=%s, HRESULT=0x%x\n", wszPropName, hr ));
	}

	return hr;
#endif
}

HRESULT CMailPropertyTool::SetProperty(CComPtr<Outlook::_PropertyAccessor> propAccessor, const wchar_t* wszPropName, const wchar_t* wszPropValue)
{
	CComVariant varValueString = wszPropValue;

	std::wstring wstrSchmepName = PROP_NAME_ROOT;
	wstrSchmepName += wszPropName;
	CComBSTR bstrScheName = wstrSchmepName.c_str();
	HRESULT hr = propAccessor->SetProperty(bstrScheName, varValueString);

	DP((L"testx CMailPropertyTool::SetProperty name=%s, value=%s, HRESULT=0x%x\n", wszPropName, wszPropValue, hr ));
	return hr;
}

std::wstring CMailPropertyTool::GetProperty(CComPtr<Outlook::_PropertyAccessor> propAccessor, const wchar_t* wszPropName)
{
	std::wstring wstrValue;
	std::wstring wstrSchmepName = PROP_NAME_ROOT;
	wstrSchmepName += wszPropName;
	CComBSTR bstrScheName = wstrSchmepName.c_str();

	CComVariant varPropValue;
	HRESULT hr = propAccessor->GetProperty(bstrScheName, &varPropValue);

	if (SUCCEEDED(hr) && varPropValue.bstrVal)
	{
		wstrValue = varPropValue.bstrVal;
	}

	DP((L"testx GetAttachmentProperty name=%s, value=%s, HRESULT=0x%x\n", wszPropName, wstrValue.c_str(), hr ));

	return wstrValue;
}

CComPtr<Outlook::_PropertyAccessor> CMailPropertyTool::GetPropAccessorFromMailItem(CComPtr<IDispatch> lpDisp)
{
	CComPtr<Outlook::_PropertyAccessor> pPropAccessor=NULL;

	CComPtr<Outlook::_MailItem> spCurMailItem = 0;
	HRESULT hr = lpDisp->QueryInterface(Outlook::IID__MailItem, (void**)&spCurMailItem);
	if(SUCCEEDED(hr) && spCurMailItem)
	{
		hr = spCurMailItem->get_PropertyAccessor(&pPropAccessor);
	    return pPropAccessor;
	}


	CComPtr<Outlook::_AppointmentItem> spCurAppItem = 0;
	hr = lpDisp->QueryInterface(Outlook::IID__AppointmentItem, (void**)&spCurAppItem);
	if(SUCCEEDED(hr) && spCurAppItem)
	{
		hr = spCurAppItem->get_PropertyAccessor(&pPropAccessor);
		return pPropAccessor;
	}

	CComPtr<Outlook::_TaskItem> spCurTaskItem = 0;
	hr = lpDisp->QueryInterface(Outlook::IID__TaskItem, (void**)&spCurTaskItem);
	if(SUCCEEDED(hr) && spCurTaskItem)
	{
		hr = spCurTaskItem->get_PropertyAccessor(&pPropAccessor);
		return pPropAccessor;
	}

	CComPtr<Outlook::_TaskRequestItem> spCurTaskReqItem = 0;
	hr = lpDisp->QueryInterface(Outlook::IID__TaskRequestItem, (void**)&spCurTaskReqItem);
	if(SUCCEEDED(hr) && spCurTaskReqItem)
	{
		hr = spCurTaskReqItem->get_PropertyAccessor(&pPropAccessor);
		return pPropAccessor;
	}

	CComPtr<Outlook::_MeetingItem> spCurMeetItem = 0;
	hr = lpDisp->QueryInterface(Outlook::IID__MeetingItem, (void**)&spCurMeetItem);
	if(SUCCEEDED(hr) && spCurMeetItem)
	{
		hr = spCurMeetItem->get_PropertyAccessor(&pPropAccessor);
		return pPropAccessor;
	}
#if ((defined WSO2K16) || (defined WSO2K13) || (defined WSO2K10))
	//add sharing item, bug 42622
	CComPtr<Outlook::_SharingItem> spCurSharingItem = 0;
	hr = lpDisp->QueryInterface(Outlook::IID__SharingItem, (void**)&spCurSharingItem);
	if(SUCCEEDED(hr) && spCurSharingItem)
	{
		hr = spCurSharingItem->get_PropertyAccessor(&pPropAccessor);
		return pPropAccessor;
	}
#endif
	return NULL;
}


void CMailPropertyTool::DeleteMessageHeaderPairs(CComPtr<IDispatch> dspMailItem, std::vector<std::pair<std::wstring, std::wstring>> & vecTagPairs)
{
	if (!vecTagPairs.empty())
	{
		CComPtr<Outlook::_PropertyAccessor> spPropAcc = GetPropAccessorFromMailItem(dspMailItem);
		if (NULL != spPropAcc)
		{
			HRESULT hr;
			CComVariant varSchemaNamesNeedDeleting, varDeletingResult;
			LPCWSTR pwzHeaderSuffix = L"http://schemas.microsoft.com/mapi/string/{00020386-0000-0000-C000-000000000046}/";
			hr = MakeSafeBSTRArrayVariant(&varSchemaNamesNeedDeleting, vecTagPairs, pwzHeaderSuffix);
			hr = spPropAcc->DeleteProperties(&varSchemaNamesNeedDeleting, &varDeletingResult);
		}
	}
}

std::wstring CMailPropertyTool::GetMailProperty(CComPtr<IDispatch> dspMail, const wchar_t* wszPropName)
{
	std::wstring wstrValue;
	CComPtr<Outlook::_PropertyAccessor> pPropAccess = GetPropAccessorFromMailItem(dspMail);
	if (pPropAccess!=NULL)
	{
     wstrValue = GetProperty(pPropAccess, wszPropName);
	}
	return wstrValue;
}

HRESULT CMailPropertyTool::SetMailProperty(CComPtr<IDispatch> dspMail, const wchar_t* wszPropName, const wchar_t* wszPropValue)
{
	HRESULT hr = S_FALSE;
	CComPtr<Outlook::_PropertyAccessor> pPropAccess = GetPropAccessorFromMailItem(dspMail);
	if (pPropAccess!=NULL)
	{
		hr = SetProperty(pPropAccess, wszPropName, wszPropValue);
	}

	return hr;
}

 bool CMailPropertyTool::OESupportMailType(ITEM_TYPE emEmailType)
{
	switch(emEmailType)
	{
	case MAIL_ITEM:	
	case MEETING_ITEM:	
	case APPOINTMENT_ITEM:	
	case TASK_ITEM:
	case TASK_REQUEST_ITEM:
	case SHARE_ITEM:
		logd(L"The mail type = %d\n", emEmailType);
		return true;
	case DEFAULT:
		loge(L"The mail type not support.\n");
		return false;	
	}
}
 
 HRESULT CMailPropertyTool::PutMessageHeaderPairs(CComPtr<IDispatch> spItem, const std::vector<std::pair<std::wstring,std::wstring> >& vecHeaderPairs, BOOL abortIfFails)
{
	if (0 < vecHeaderPairs.size())
	{
		CComPtr<Outlook::_PropertyAccessor> spPropAccessor = CMailPropertyTool::GetPropAccessorFromMailItem(spItem);
		if (NULL != spPropAccessor)
		{
			logd(L"[PutMessageHeaderPairs]Prepare to SetProperty"); 

			HRESULT hr;
			std::vector<std::pair<std::wstring,std::wstring> >::const_iterator itHeader; 
			std::wstring sNextlabsTag; // used to track the Nextlabs proprietary header tag
			
			std::wstring sHeaderName(L"http://schemas.microsoft.com/mapi/string/{00020386-0000-0000-C000-000000000046}/"); //init base schema name 
			std::wstring::size_type nMarkPosition = sHeaderName.size();

			// @TODO optimize it: spPropAccessor->SetProperties(MakeSafeBSTRArrayVariant(), MakeSafeBSTRArrayVariant());
			for (itHeader = vecHeaderPairs.begin();  itHeader != vecHeaderPairs.end(); ++itHeader)
			{	
				LPCWSTR pwzHeaderName = itHeader->first.c_str();
				LPCWSTR pwzHeaderValue = itHeader->second.c_str();
				//suffix the given name to get a full property name
				sHeaderName.replace(nMarkPosition, std::wstring::npos, itHeader->first); 

				hr = spPropAccessor->SetProperty(CComBSTR(sHeaderName.c_str()), CComVariant(pwzHeaderValue));

				// Note, here printed pwzHeaderName don't prefix base schema name 
				logd(L"[PutMessageHeaderPairs]HeaderPair(%#x): \"%s\"=\"%s\"", hr, pwzHeaderName, pwzHeaderValue); 
				if (FAILED(hr))
				{
					if (abortIfFails)
					{
						return E_ABORT;
					}
				}else
				{
					// Track the Nextlabs proprietary mail transport header names
					if (!sNextlabsTag.empty())
					{
						sNextlabsTag += (wchar_t)HEADER_NEXTLABS_TAG_SEPARATOR;
					}
					sNextlabsTag.append(pwzHeaderName);
				}
			}

			// Also put the Nextlabs proprietary mail transport header names as a header pair. e.g. "X-Nextlabs: X-PR:X-ITAR:X-EAR"
			if (!sNextlabsTag.empty())
			{
				//suffix the given name to get a full property name
				sHeaderName.replace(nMarkPosition, std::wstring::npos, HEADER_NAME_NEXTLABS);

				hr = spPropAccessor->SetProperty(CComBSTR(sHeaderName.c_str()), CComVariant(sNextlabsTag.c_str()));

				// Note, here printed pwzHeaderName don't prefix base schema name 
				logd(L"[PutMessageHeaderPairs]HeaderPair(%#x): \"%s\"=\"%s\"", hr, HEADER_NAME_NEXTLABS, sNextlabsTag.c_str()); 
			}
			return S_OK;
		}else
		{
			loge(L"[PutMessageHeaderPairs]Failed to get PropertyAccessor\n"); 
		}
	}else{
		logi(L"[PutMessageHeaderPairs]size = %d\n", vecHeaderPairs.size()); 
	}
	return S_FALSE;
}

ATL::CComPtr<Outlook::UserProperties> CMailPropertyTool::GetUserPropertiesFromItem(CComPtr<IDispatch> spDispItem)
{
	CComPtr<Outlook::UserProperties> spUserProperties=NULL;

	CComPtr<Outlook::_MailItem> spCurMailItem = 0;
	HRESULT hr = spDispItem->QueryInterface(Outlook::IID__MailItem, (void**)&spCurMailItem);
	if(SUCCEEDED(hr) && spCurMailItem)
	{
		spCurMailItem->get_UserProperties(&spUserProperties);
		return spUserProperties;
	}

	CComPtr<Outlook::_AppointmentItem> spAppointmentItem = 0;
	hr = spDispItem->QueryInterface(Outlook::IID__AppointmentItem, (void**)&spAppointmentItem);
	if(SUCCEEDED(hr) && spAppointmentItem)
	{
		hr = spAppointmentItem->get_UserProperties(&spUserProperties);
		return spUserProperties;
	}

	CComPtr<Outlook::_TaskItem> spTaskItem = 0;
	hr = spDispItem->QueryInterface(Outlook::IID__TaskItem, (void**)&spTaskItem);
	if(SUCCEEDED(hr) && spTaskItem)
	{
		hr = spTaskItem->get_UserProperties(&spUserProperties);
		return spUserProperties;
	}

	CComPtr<Outlook::_TaskRequestItem> spTaskReqItem = 0;
	hr = spDispItem->QueryInterface(Outlook::IID__TaskRequestItem, (void**)&spTaskReqItem);
	if(SUCCEEDED(hr) && spTaskReqItem)
	{
		hr = spTaskReqItem->get_UserProperties(&spUserProperties);
		return spUserProperties;
	}

	CComPtr<Outlook::_MeetingItem> spMeetItem = 0;
	hr = spDispItem->QueryInterface(Outlook::IID__MeetingItem, (void**)&spMeetItem);
	if(SUCCEEDED(hr) && spMeetItem)
	{
		hr = spMeetItem->get_UserProperties(&spUserProperties);
		return spUserProperties;
	}
	return NULL;
}

HRESULT CMailPropertyTool::FindUserProperty(CComPtr<Outlook::UserProperties> spUserProperties, const wchar_t* wszName, std::wstring &sValue)
{
	ATL::CComBSTR sbsPropertyName(wszName);
	ATL::CComPtr<Outlook::UserProperty> spUserProperty;
	
	HRESULT hr = spUserProperties->Find(sbsPropertyName, CComVariant(VARIANT_TRUE), &spUserProperty);

	if (FAILED(hr) || NULL == spUserProperty)
	{
		loge(L"[FindUserProperty]UserProperties.Find return %#x\n", hr);
		return hr;
	}
	CComVariant varValue;
	hr = spUserProperty->get_Value(&varValue);
	if (FAILED(hr))
	{
		loge(L"[FindUserProperty]UserProperties.get_Value return %#x\n", hr);
	}
	if (VT_BSTR == varValue.vt)
	{
		sValue = varValue.bstrVal;
	}
	return hr;
}

HRESULT CMailPropertyTool::AddUserProperty(CComPtr<Outlook::UserProperties> spUserProperties, const wchar_t* wszName, const wchar_t* wszValue)
{
	ATL::CComBSTR sbsPropertyName(wszName);
	ATL::CComVariant addToFolderField(VARIANT_TRUE), varOptional((long)DISP_E_PARAMNOTFOUND, VT_ERROR);
	ATL::CComPtr<Outlook::UserProperty> spUserProperty;
	HRESULT hr = spUserProperties->Add(sbsPropertyName, Outlook::OlUserPropertyType::olText, addToFolderField, varOptional, &spUserProperty);
	if (FAILED(hr) || NULL == spUserProperty)
	{
		loge(L"[AddUserProperty]UserProperties.Add return %#x\n", hr);
		return hr;
	}
	hr = spUserProperty->put_Value(CComVariant(wszValue));
	if (FAILED(hr))
	{
		loge(L"[AddUserProperty]UserProperties.put_Value return %#x\n", hr);
	}
	return hr;
}
