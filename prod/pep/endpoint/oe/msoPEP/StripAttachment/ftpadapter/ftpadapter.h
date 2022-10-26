#ifndef __FTPADAPTER_H__
#define __FTPADAPTER_H__
#include <string>
#include <vector>
#include <winreg.h>
#include "adaptercomm.h"
#include <atlcomcli.h>
#include "log.h"
#include "../ftpcli/ftpcli.h"
#define FTPADAPTER_OBLIGATION_NAME	L"FTPADAPTER"
#define FTPADAPTER_INI_FILENAME		L"ftpadapter.ini"

typedef struct ATTACHMENTPARAM
{
	std::wstring strRemotePath;
	std::wstring strFileName;
	std::wstring strPassword;
};

class FTPAdapter
{
private:
	static const WCHAR OBLIGATION_ATTRNAME_FTPSERVER[];
	static const WCHAR OBLIGATION_ATTRNAME_FTPUSER[];
	static const WCHAR OBLIGATION_ATTRNAME_FTPPASSWORD[];
	static const WCHAR OBLIGATION_ATTRNAME_LOCATION[];
	static const WCHAR OBLIGATION_ATTRVALUE_LOCATION_TOP[];
	static const WCHAR OBLIGATION_ATTRVALUE_LOCATION_BOTTOM[];

	static const WCHAR OBLIGATION_ATTRNAME_TEXT[];
	static const WCHAR OBLIGATION_ATTRVALUE_TEXT[];

	static const WCHAR OBLIGATION_ATTRNAME_LINKFORMAT[];
	static const WCHAR OBLIGATION_ATTRVALUE_LINKFORMAT_LONG[];
	static const WCHAR OBLIGATION_ATTRVALUE_LINKFORMAT_SHORT[];

	static const WCHAR FTPADAPTER_PLACEHOLDER_FILENAME[];
	static const WCHAR FTPADAPTER_PLACEHOLDER_LINK[];
	static const WCHAR SEPARATOR_STRING[];

	// Add by Tonny for Tyco of GlobalScope ftp server
	static const WCHAR OBLIGATION_ATTRNAME_EFTEXPIRATIONDATE[];
	static const WCHAR OBLIGATION_ATTRNAME_EFTFTPSITE[];
	static const WCHAR OBLIGATION_ATTRNAME_EFTFTPUSERSETTINGSTEMPLATE[];
	static const WCHAR OBLIGATION_ATTRNAME_EFTADMINPORT[];
	static const WCHAR OBLIGATION_ATTRNAME_PWD_SUBJECT[];
	static const WCHAR OBLIGATION_ATTRNAME_PWD_BODY[];
	static const WCHAR OBLIGATION_SUBJECT_PATTERN[];
	static const WCHAR FTP_USERNAME[];
	static const WCHAR OBLIGATION_RETURN[];
	static const WCHAR OBLIGATION_SEPARATE_PASSWORD[];
	static const WCHAR OBLIGATION_ERRORMESSAGE_SENDER[];
	static const WCHAR OBLIGATION_ERRORMESSAGE_RECIPIENTS[];


public:
	FTPAdapter():m_pItem(NULL),m_pAttachments(NULL),m_pAdapter(NULL),hFtpCli(NULL),fpFtpUploadEx(NULL),m_bIsHtml(false),
	fpEFTUpload(NULL){};
	FTPAdapter(IDispatch*pItem,AdapterCommon::Attachments* pAtts,AdapterCommon::Adapter* pAdapter)
		:m_pItem(pItem),m_pAttachments(pAtts),m_pAdapter(pAdapter),hFtpCli(NULL),fpFtpUploadEx(NULL),m_bIsHtml(false),
		fpEFTUpload(NULL){};
	bool IsHtmlBody(){return m_bIsHtml;};
	BOOL Init();
	BOOL UploadAll();
	BOOL UploadAllEx(wchar_t** pwchObligationTopMessage,int &iTopMsgLength,wchar_t** pwchObligationBottomMessage,int &iBottomMsgLength);
	BOOL UploadOne(AdapterCommon::Attachment *pAtt);
private:
	
private:
	
	HMODULE						hFtpCli;
	FPFtpUploadEx				fpFtpUploadEx;
	IDispatch*					m_pItem;
	AdapterCommon::Attachments*	m_pAttachments;
	AdapterCommon::Adapter*		m_pAdapter;
	bool						m_bIsHtml;
	std::wstring				m_strTopMessageText;
	std::wstring				m_strBottomMessageText;

	FuncEFTFtpUpload				fpEFTUpload;

	std::vector<ATTACHMENTPARAM> m_vAttachments;
	std::wstring m_strPwdMailSubject;
	std::wstring m_strPwdMailBody;
	std::wstring m_strErrorMsgForSender;
	std::wstring m_strErrorMsgForRecipients;
	std::vector<std::wstring> m_vFailedAttachments;
};
const WCHAR FTPAdapter::OBLIGATION_ATTRNAME_FTPSERVER[]=L"FTP Server";
const WCHAR FTPAdapter::OBLIGATION_ATTRNAME_FTPUSER[]=L"User";
const WCHAR FTPAdapter::OBLIGATION_ATTRNAME_FTPPASSWORD[]=L"Password";
const WCHAR FTPAdapter::OBLIGATION_ATTRNAME_LOCATION[]=L"Location";
const WCHAR FTPAdapter::OBLIGATION_ATTRVALUE_LOCATION_TOP[]=L"Top";
const WCHAR FTPAdapter::OBLIGATION_ATTRVALUE_LOCATION_BOTTOM[]=L"Bottom";
const WCHAR FTPAdapter::OBLIGATION_ATTRNAME_TEXT[]=L"Text";
const WCHAR FTPAdapter::OBLIGATION_ATTRVALUE_TEXT[]=L"The attachments [filename] to this message have been removed for security purpose and made available at the following location:[link]";
const WCHAR FTPAdapter::OBLIGATION_ATTRNAME_LINKFORMAT[]=L"Link Format";
const WCHAR FTPAdapter::OBLIGATION_ATTRVALUE_LINKFORMAT_LONG[]=L"Long";
const WCHAR FTPAdapter::OBLIGATION_ATTRVALUE_LINKFORMAT_SHORT[]=L"Short";
const WCHAR FTPAdapter::FTPADAPTER_PLACEHOLDER_FILENAME[]=L"[filename]";
const WCHAR FTPAdapter::FTPADAPTER_PLACEHOLDER_LINK[]=L"[link]";
const WCHAR FTPAdapter::SEPARATOR_STRING[]=L"==========================================================================";
const WCHAR FTPAdapter::OBLIGATION_ATTRNAME_PWD_SUBJECT[]=L"Subject";
const WCHAR FTPAdapter::OBLIGATION_ATTRNAME_PWD_BODY[]=L"Body";
const WCHAR FTPAdapter::OBLIGATION_SUBJECT_PATTERN[]=L"[Title of Original Email]";
const WCHAR FTPAdapter::OBLIGATION_RETURN[]=L"[Return]";
const WCHAR FTPAdapter::FTP_USERNAME[]=L"(Login: %s)";
const WCHAR FTPAdapter::OBLIGATION_SEPARATE_PASSWORD[]=L"Separate Password in Email";
const WCHAR FTPAdapter::OBLIGATION_ERRORMESSAGE_SENDER[]=L"Error Message For Sender";
const WCHAR FTPAdapter::OBLIGATION_ERRORMESSAGE_RECIPIENTS[]=L"Error Message For Recipients";


const WCHAR FTPAdapter::OBLIGATION_ATTRNAME_EFTEXPIRATIONDATE[]=L"EFT User Expiry Date (0...65536)";
const WCHAR FTPAdapter::OBLIGATION_ATTRNAME_EFTFTPSITE[]=L"EFT Site Name";
const WCHAR FTPAdapter::OBLIGATION_ATTRNAME_EFTFTPUSERSETTINGSTEMPLATE[]=L"EFT User Settings Template";
const WCHAR FTPAdapter::OBLIGATION_ATTRNAME_EFTADMINPORT[]=L"EFT Admin Port";



STDAPI RepositoryUpload(IDispatch*pItem,AdapterCommon::Attachments* pAtts);
STDAPI RepositoryUploadEx(IDispatch*pItem,AdapterCommon::Attachments* pAtts,wchar_t** pwchObligationTopMessage,int &iTopMsgLength,wchar_t** pwchObligationBottomMessage,int &iBottomMsgLength);
STDAPI ReleaseRepositoryUploadExPWCH(wchar_t* pwch,bool bIsArray);
#endif //__FTPADAPTER_H__
