#ifndef __FTPADAPTER_H__
#define __FTPADAPTER_H__
#include <string>
#include <vector>
#include <winreg.h>
#include "adaptercomm.h"
#include <atlcomcli.h>
#include "log.h"
//#include "../ftpcli/ftpcli.h"
#define APPROVALADAPTER_OBLIGATION_NAME	L"APPROVALADAPTER"
#define APPROVALADAPTER_INI_FILENAME		L"approvaladapter.ini"


class ApprovalAdapter
{
public:
	static const WCHAR OBLIGATION_ATTRNAME_SOURCE[];
	static const WCHAR OBLIGATION_ATTRNAME_USER[];
	static const WCHAR OBLIGATION_ATTRNAME_APPROVERS[];
	static const WCHAR OBLIGATION_ATTRNAME_RECIPIENTS[];
	static const WCHAR OBLIGATION_ATTRNAME_FTPDIR[];
	static const WCHAR OBLIGATION_ATTRNAME_FTPUSER[];

	static const WCHAR OBLIGATION_ATTRNAME_FTPPASSWD[];
public:
	ApprovalAdapter(){};
	ApprovalAdapter(CComPtr<IDispatch> pItem,AdapterCommon::Attachments* pAtts,AdapterCommon::Adapter* pAdapter):
		m_pItem(pItem),m_pAttachments(pAtts),m_pAdapter(pAdapter){};
	bool IsHtmlBody(){return m_bIsHtml;};
	BOOL AttachWithApprovalObligation(AdapterCommon::Attachment *pAtt,AdapterCommon::Obligation& obligation);
	//BOOL Init();
	//BOOL UploadAll();
	//BOOL UploadOne(AdapterCommon::Attachment *pAtt);
private:
	
private:
	
	HMODULE						hFtpCli;
	//FPFtpUpload					fpFtpUpload;
	CComPtr<IDispatch>					m_pItem;
	AdapterCommon::Attachments*	m_pAttachments;
	AdapterCommon::Adapter*		m_pAdapter;
	bool						m_bIsHtml;

};
const WCHAR ApprovalAdapter::OBLIGATION_ATTRNAME_SOURCE[]=L"-Source";
const WCHAR ApprovalAdapter::OBLIGATION_ATTRNAME_USER[]=L"-user";
const WCHAR ApprovalAdapter::OBLIGATION_ATTRNAME_APPROVERS[]=L"-approvers";
const WCHAR ApprovalAdapter::OBLIGATION_ATTRNAME_RECIPIENTS[]=L"-recipients";
const WCHAR ApprovalAdapter::OBLIGATION_ATTRNAME_FTPDIR[]=L"-ftpdir";
const WCHAR ApprovalAdapter::OBLIGATION_ATTRNAME_FTPUSER[]=L"-ftpuser";
const WCHAR ApprovalAdapter::OBLIGATION_ATTRNAME_FTPPASSWD[]=L"-ftppasswd";



STDAPI RepositoryUpload(CComPtr<IDispatch> pItem,AdapterCommon::Attachments* pAtts);

#endif //__FTPADAPTER_H__
