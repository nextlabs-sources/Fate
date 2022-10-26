#ifndef MSO_ATTACHMENTOBLIGATIONDATA_H_
#define MSO_ATTACHMENTOBLIGATIONDATA_H_

#include "ItemEventDisp.h"
#include "attachment.h"
#include "../PAEx/OE_PAMngr.h"
#include "mailAttach.h"
#include "../common/policy.h"
#include "../CA/caadapter.h"
typedef std::vector<OBLIGATIONSLIST> RECIPIENTOBLIGATIONSLIST;
enum EXCUTEOBLACTION
{
	EXCUTESUCCESS,
	EXCUTE_CANCEL_MAYBE_LOG_DESIGN,
	EXCUTE_MAYBE_LOG_DESIGN,
	EXCUTE_ZERO_PA_OBJECT_LIST,
	EXCUTE_CHECK_POLICY
};


class CAttachmentObligationData
{
public:
	CAttachmentObligationData();
	~CAttachmentObligationData();
public:
	CNotificationBubble m_NotificationBubble;
	AdapterCommon::Attachments m_adapterAttachments;
	
	EXCUTEOBLACTION ExcuteNXLEncrptyEx(CSendEmailData &SendEmailData);
	EXCUTEOBLACTION ExcutePrependSubject(CSendEmailData &SendEmailData);
	EXCUTEOBLACTION ExcutePrependBody(CSendEmailData &SendEmailData); 
	EXCUTEOBLACTION ExcuteAppendBody(CSendEmailData &SendEmailData);
	EXCUTEOBLACTION ExcuteContentReactionBody(CSendEmailData &SendEmailData);
	EXCUTEOBLACTION ExcuteContentReactionSubject(CSendEmailData &SendEmailData);
	EXCUTEOBLACTION ExcuteMailAttributeParsingEx(CSendEmailData & EmailData);
	EXCUTEOBLACTION ExcuteMailNotificationEx(CSendEmailData & EmailData);
	EXCUTEOBLACTION ExcuteInternetUseOnlyEx(CSendEmailData &SendEmailData);
	EXCUTEOBLACTION ExcuteEmailStrIPAttachmentEx(CSendEmailData &SendEmailData,CComPtr<IDispatch> dspMailItem,COE_PAMngr& paMngr);
	EXCUTEOBLACTION ExcuteHelpURLFirstMatchAttachmentEx(CSendEmailData &SendEmailData);
	EXCUTEOBLACTION ExcuteEmailVerifyEx(CSendEmailData &SendEmailData);
	EXCUTEOBLACTION ExcuteEmailHDREx(CSendEmailData &SendEmailData);
	EXCUTEOBLACTION ExcuteHeaderTaggingEx(CSendEmailData &SendEmailData,PolicyCommunicator* pPolicyCommunicator,COE_PAMngr& paMngr, CComPtr<IDispatch> dspMailItem);
	EXCUTEOBLACTION ExcuteFileTaggingEx(CSendEmailData &SendEmailData,PolicyCommunicator* pPolicyCommunicator,COE_PAMngr& paMngr, CComPtr<IDispatch> dspMailItem);

	EXCUTEOBLACTION ExcuteEncryptionEx(CSendEmailData &SendEmailData,PolicyCommunicator* pPolicyCommunicator,COE_PAMngr& paMngr);
	EXCUTEOBLACTION ExcutePortableEncryptionEx(CSendEmailData &SendEmailData,PolicyCommunicator* pPolicyCommunicator,COE_PAMngr& paMngr);
	EXCUTEOBLACTION ExcuteRichUserMsg(CSendEmailData &SendEmailData);

	bool ExcuteWarnMsgEx(CSendEmailData &SendEmailData);
	bool ExcuteAuditLogEx(CSendEmailData &SendEmailData,int nAttachmentPos,bool bOK);
	bool ExcuteHCReportLogEx(CSendEmailData &SendEmailData);

	bool GetRejectUnlessSilentInfoFromObl(CEAttributes *obligation,int ObligationPos,int nAttachmentPos,CSendEmailData &SendEmailData,int nRecipientNum);
	bool GetMailAttributeParsingFromObl(CEAttributes *obligation,int ObligationPos,CSendEmailData &SendEmailData);
	bool GetMailNotiFromObl(CEAttributes *obligation,int ObligationPos,CSendEmailData &SendEmailData);
	bool GetInteUseOnlyFromObl(CEAttributes *obligation,int ObligationPos,int nAttachmentPos,CSendEmailData &SendEmailData);
	bool GetVerifyRecipientFromObl(CEAttributes *obligation,int ObligationPos,int nAttachmentPos,CSendEmailData &SendEmailData);
	bool GetMissTagFromObl(CEAttributes *obligation,int ObligationPos,int nAttachmentPos,CSendEmailData &SendEmailData);
	bool GetMulClientFromObl(CEAttributes *obligation,int ObligationPos,int nAttachmentPos,CSendEmailData &SendEmailData);
	bool GetDomainMismatchFromObl(CEAttributes *obligation,int ObligationPos,int nAttachmentPos,CSendEmailData &SendEmailData);
	bool GetHDRFromObl(CEAttributes *obligation,int ObligationPos,int nAttachmentPos,CSendEmailData &SendEmailData);
	bool GetFileCustomInterTagFromObl(CEAttributes *obligation,int ObligationPos,VECTOR_TAGPAIR &vecTag);
	bool GetFileCustomAutoTagFromObl(CEAttributes *obligation,int ObligationPos,VECTOR_TAGPAIR &vecTag);
	bool GetNxlAutoTagFromObl(CEAttributes *obligation,int ObligationPos,CSendEmailData &SendEmailData,int nAttachmentPos);
	bool GetPrependBodyFromObl(CEAttributes *obligation,int ObligationPos,wstring &strBuf);
	bool GetAppendBodyFromObl(CEAttributes *obligation,int ObligationPos,wstring &strBuf);
	bool GetWarningMsgFromObl(CEAttributes *obligation,int ObligationPos,int AttachmentNum,CSendEmailData &SendEmailData);
	bool GetPrependSubjectFromObl(CEAttributes *obligation,int ObligationPos,wstring &strBuf);
	bool GetAlertInfoFromObl(CEAttributes *obligation,int ObligationPos,CSendEmailData &SendEmailData,bool bAllow);
	bool GetRichAlertMsgFromObl(CEAttributes *obligation,int ObligationPos,CSendEmailData &SendEmailData,bool bAllow);
	int  GetContentReactionFromObl(CEAttributes *obligation,int ObligationPos,CAOblInfo& ca);
	bool GetStripFTPFromObl(CEAttributes *obligation,int ObligationPos,CSendEmailData &SendEmailData, AdapterCommon::Attachment& AdapterAttachment);
	bool GetStripFSFromObl(CEAttributes *obligation,int ObligationPos,CSendEmailData &SendEmailData, AdapterCommon::Attachment& AdapterAttachment);
	bool GetRichUserMsg(CEAttributes *obligation,int ObligationPos,CSendEmailData &SendEmailData);
	void UpdateTempFilePath(CSendEmailData &SendEmailData,COE_PAMngr& paMngr);
};



#endif