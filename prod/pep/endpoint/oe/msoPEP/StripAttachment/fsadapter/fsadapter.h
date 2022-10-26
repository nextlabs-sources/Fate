#ifndef __FSADAPTER_H__
#define __FSADAPTER_H__
#include <string>
#include <vector>
#include <winreg.h>
#include "adaptercomm.h"
#include <atlcomcli.h>
#include "log.h"
#define FSADAPTER_OBLIGATION_NAME	L"FSADAPTER"
#define FSADAPTER_INI_FILENAME		L"fsadapter.ini"


class FileServerAdapter
{
private:
	static const WCHAR OBLIGATION_ATTRNAME_FILESERVER[];
	static const WCHAR OBLIGATION_ATTRNAME_USER[];
	static const WCHAR OBLIGATION_ATTRNAME_PASSWORD[];
	static const WCHAR OBLIGATION_ATTRNAME_DOMAIN[];
	static const WCHAR OBLIGATION_ATTRNAME_LOCATION[];
	static const WCHAR OBLIGATION_ATTRVALUE_LOCATION_TOP[];
	static const WCHAR OBLIGATION_ATTRVALUE_LOCATION_BOTTOM[];

	static const WCHAR OBLIGATION_ATTRNAME_TEXT[];
	static const WCHAR OBLIGATION_ATTRVALUE_TEXT[];

	static const WCHAR OBLIGATION_ATTRNAME_LINKFORMAT[];
	static const WCHAR OBLIGATION_ATTRVALUE_LINKFORMAT_LONG[];
	static const WCHAR OBLIGATION_ATTRVALUE_LINKFORMAT_SHORT[];

	static const WCHAR FSADAPTER_PLACEHOLDER_FILENAME[];
	static const WCHAR FSADAPTER_PLACEHOLDER_LINK[];
	static const WCHAR SEPARATOR_STRING[];
	

public:
	FileServerAdapter():m_pItem(NULL),m_pAttachments(NULL),m_pAdapter(NULL),m_bIsHtml(false){};
	FileServerAdapter(IDispatch*pItem,AdapterCommon::Attachments* pAtts,AdapterCommon::Adapter* pAdapter)
		:m_pItem(pItem),m_pAttachments(pAtts),m_pAdapter(pAdapter),m_bIsHtml(false){};
	bool IsHtmlBody(){return m_bIsHtml;};
	BOOL Init();
	BOOL UploadAll();
	BOOL UploadAllEx(wchar_t** pwchObligationTopMessage,int &iTopMsgLength,wchar_t** pwchObligationBottomMessage,int &iBottomMsgLength);
	BOOL UploadOne(AdapterCommon::Attachment *pAtt);
private:
	
private:
	IDispatch*					m_pItem;
	AdapterCommon::Attachments*	m_pAttachments;
	AdapterCommon::Adapter*		m_pAdapter;
	bool						m_bIsHtml;
	std::wstring				m_strTopMessageText;
	std::wstring				m_strBottomMessageText;

};
const WCHAR FileServerAdapter::OBLIGATION_ATTRNAME_FILESERVER[]=L"File Server";
//const WCHAR FileServerAdapter::OBLIGATION_ATTRNAME_USER[]=L"User";
//const WCHAR FileServerAdapter::OBLIGATION_ATTRNAME_PASSWORD[]=L"Password";
//const WCHAR FileServerAdapter::OBLIGATION_ATTRNAME_DOMAIN[]=L"Domain";
const WCHAR FileServerAdapter::OBLIGATION_ATTRNAME_LOCATION[]=L"Location";
const WCHAR FileServerAdapter::OBLIGATION_ATTRVALUE_LOCATION_TOP[]=L"Top";
const WCHAR FileServerAdapter::OBLIGATION_ATTRVALUE_LOCATION_BOTTOM[]=L"Bottom";
const WCHAR FileServerAdapter::OBLIGATION_ATTRNAME_TEXT[]=L"Text";
const WCHAR FileServerAdapter::OBLIGATION_ATTRVALUE_TEXT[]=L"The attachments [filename] to this message have been removed for security purpose and made available at the following location:[link]";
const WCHAR FileServerAdapter::OBLIGATION_ATTRNAME_LINKFORMAT[]=L"Link Format";
const WCHAR FileServerAdapter::OBLIGATION_ATTRVALUE_LINKFORMAT_LONG[]=L"Long";
const WCHAR FileServerAdapter::OBLIGATION_ATTRVALUE_LINKFORMAT_SHORT[]=L"Short";
const WCHAR FileServerAdapter::FSADAPTER_PLACEHOLDER_FILENAME[]=L"[filename]";
const WCHAR FileServerAdapter::FSADAPTER_PLACEHOLDER_LINK[]=L"[link]";
const WCHAR FileServerAdapter::SEPARATOR_STRING[]=L"==========================================================================";


STDAPI RepositoryUpload(IDispatch*pItem,AdapterCommon::Attachments* pAtts);
STDAPI RepositoryUploadEx(IDispatch*pItem,AdapterCommon::Attachments* pAtts,wchar_t** pwchObligationTopMessage,int &iTopMsgLength,wchar_t** pwchObligationBottomMessage,int &iBottomMsgLength);
STDAPI ReleaseRepositoryUploadExPWCH(wchar_t * pwch,bool bIsArry);
#endif //__FSADAPTER_H__
