#ifndef __MAILATTACH_H__
#define __MAILATTACH_H__

#include "../common/policy.h"
#include "../outlook/outlookUtilities.h"
#include "../outlook/ParameterForMulQuery.h"
#include "../outlook/AttachmentObligationData.h"



class CAttachmentObligationData;

BOOL IsMsgFile(LPCWSTR pwzFile);

BOOL ExtractMsgAttachment(const wchar_t* pTopDir,
									   MAILTYPE m_mtMailType,
									   STRINGLIST& listRecipients,
									   LpAttachmentData attachMail,
									   CComPtr<IDispatch> pMail,
									   CEEnforcement_t *enforcer,
									   PolicyCommunicator* spPolicyCommunicator,
									   BOOL* bNeedNdr,
									   BOOL* bCancel,
									   AttachInMsgFormat** ppMsgAttach);


BOOL AddAttachsInMsg4Hdr(MSGATTACHLIST & listMsgAttach,ATTACHMENTLIST& hdrAttachList);
BOOL CheckHdrNeeded(LpAttachmentData lpAttachmentData,CEEnforcement_t *enforcer);
BOOL CreateTempMsgFile(const wchar_t* pTopDir,LpAttachmentData attachMail);
int PackMsgAttach(CComPtr<IDispatch> pMail,AttachInMsgFormat *pMsgAttach);


#endif //__MAILATTACH_H__