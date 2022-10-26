#pragma once

#define PROP_NAME_ROOT L"http://schemas.microsoft.com/mapi/string/{00020386-0000-0000-C000-000000000046}/"


#define PROP_NAME_HCTAG L"NextLabs_HCTag"
#define PROP_NAME_PREOUTLOOKTEMPLASTMODIFYTIME L"PreOutlookTempLastModifyTime"
#define PROP_NAME_MAIL_SENDTIMES L"MailSendTimes"
#define PROP_NAME_ATTACHMENT_SENDTIMES L"AttachmentSendTimes"
#define PROP_NAME_ATTACHMENT_ISCHANGED L"Cngd"//so that customer can set "IsChanged" tag


#define TAG_PROP_SEPRATOR_TAGS  L";"
#define TAG_PROP_SEPRATOR_KEYVALUE L"="

class CMailPropertyTool
{
public:
	CMailPropertyTool(void);
	~CMailPropertyTool(void);

public:
	static HRESULT SetAttachmentProperty(CComPtr<Outlook::Attachment> spAttachment, const wchar_t* wszPropName, const wchar_t* wszPropValue);
	static std::wstring GetAttachmentProperty(CComPtr<Outlook::Attachment> spAttachment, const wchar_t* wszPropName);
	static HRESULT DelAttachmentProperty(CComPtr<Outlook::Attachment> spAttachment, const wchar_t* wszPropName);
	static HRESULT SetMailProperty(CComPtr<IDispatch> dspMail, const wchar_t* wszPropName, const wchar_t* wszPropValue);

	static std::wstring GetMailProperty(CComPtr<IDispatch> dspMail, const wchar_t* wszPropName);

	static void SetHCTagToAttachmentProperty(CComPtr<Outlook::Attachment> spAttachment, const std::vector<std::pair<std::wstring,std::wstring>>& vectHCTagInfo);
    static void GetHCTagFromAttachmentProperty(CComPtr<Outlook::Attachment> spAttachment, std::vector<std::pair<std::wstring,std::wstring>>& vectHCTagInfo);

	static bool OESupportMailType(ITEM_TYPE emEmailType);
	static HRESULT PutMessageHeaderPairs(CComPtr<IDispatch> spItem, const std::vector<std::pair<std::wstring,std::wstring> >& vecHeaderPairs, BOOL abortIfFails); // set x-header
	
	static void DeleteMessageHeaderPairs(CComPtr<IDispatch> dspMailItem, std::vector<std::pair<std::wstring, std::wstring>> & vecTagPairs);

	static CComPtr<Outlook::UserProperties> GetUserPropertiesFromItem(CComPtr<IDispatch> spDispItem);
	static HRESULT FindUserProperty(CComPtr<Outlook::UserProperties> spUserProperties, const wchar_t* wszName, std::wstring &sValue);
	static HRESULT AddUserProperty(CComPtr<Outlook::UserProperties> spUserProperties, const wchar_t* wszName, const wchar_t* wszValue);

	static std::wstring GetTagsAsString(const std::vector<std::pair<std::wstring,std::wstring>>& vectHCTagInfo);
	static void GetTagsAsVectory(const std::wstring& wstrTags, std::vector<std::pair<std::wstring,std::wstring>>& vectHCTagInfo);

	// e.g. LPCWSTR pwzSchemasSuffix = L"http://schemas.microsoft.com/mapi/string/{00020386-0000-0000-C000-000000000046}/";
	// CComVariant var; std::vector<std::pair<std::wstring, std::wstring> > vec; MakeSafeBSTRArrayVariant(&var, vec);
	static HRESULT MakeSafeBSTRArrayVariant(OUT VARIANT *dst, IN const std::vector<std::pair<std::wstring, std::wstring> >& src, IN OPTIONAL const wchar_t* pwzElementSuffix = L"");

	static CComPtr<Outlook::_PropertyAccessor> GetPropAccessorFromMailItem(CComPtr<IDispatch> dspMailItem);

protected:

	static HRESULT SetProperty(CComPtr<Outlook::_PropertyAccessor> propAccessor, const wchar_t* wszPropName, const wchar_t* wszPropValue);
	static std::wstring GetProperty(CComPtr<Outlook::_PropertyAccessor> propAccessor, const wchar_t* wszPropName);
};
