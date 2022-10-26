#pragma once
#include "DataType.h"
#include "../outlook/ParameterForMulQuery.h"
#include "eframework/platform/cesdk_attributes.hpp"

class CQueryPCInfo
{
public:
	CQueryPCInfo(void);
	~CQueryPCInfo(void);
    bool ConstructRequestFromEmailData(CSendEmailData& emailData,CComPtr<IDispatch> MailItem);
	CERequest* GetRequest() {return &m_vecCERequest[0]; }
	size_t GetRequestNumber() { return m_vecCERequest.size(); }
	CEEnforcement_t* GetEnforcement() { return &m_vecEnforcement[0]; }

protected:
	enum EM_SOURCETYPE{SOUCETYPE_SUBJECT=1, SOUCETYPE_BODY=2, SOUCETYPE_ATTACHMENT=3, SOUCETYPE_X_HEADER=4, SOUCETYPE_RECIPIENT=5};


protected:
	bool ConstructRequestForEmailAction(CSendEmailData& emailData,CComPtr<IDispatch> MailItem);
	bool ConstructRequestForReceiveAction(CSendEmailData& emailData,CComPtr<IDispatch> MailItem);
	bool ConstructRequestForReceiveActionOnEachRecipient(CSendEmailData& emailData, CComPtr<IDispatch> MailItem);
	bool CreateSource(EM_SOURCETYPE sourceType, void* pSourceData, CEResource** ppCEResource, CEAttributes** ppSourceAttr, CComPtr<IDispatch> MailItem = NULL);
	int AddRequest(CEString ceOperator, CEUser* pCEUser, CEAttributes* pUserAttributes, CEString* pArrayRecipients, CEint32 numRecipients, CEResource* pSource, CEAttributes* pSourceAttribute);

	void GetMeetingSender(CSendEmailData& emailData,CComPtr<IDispatch> MailItem,wstring& strCurrentSender);
protected:
	std::wstring CreateTempFileForQueryPC(const wchar_t* pBuffer, const wchar_t* wszExt);
    void SetTickTime();

private:
	std::vector<CERequest>						m_vecCERequest;
	std::vector<CEEnforcement_t>					m_vecEnforcement;

	ReleaseResource m_releaseResource;

	std::wstring m_strTickTime;
};

template<typename T = void*> struct SourceDataT{
	CSendEmailData& emailData;
	T data;
};

typedef SourceDataT<CAttachmentData&> SourceDataForAttachment;
typedef SourceDataT< std::vector<int>& > SourceDataForRecipient;