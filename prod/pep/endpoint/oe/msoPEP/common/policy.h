

#ifndef _COMMON_POLICY_H_
#define _COMMON_POLICY_H_
#include "SDKSupport.h"

#include "../outlook/ParameterForMulQuery.h"
#include "../outlook/QueryPCInfo.h"

class PolicyCommunicator
{
public:
    virtual ~PolicyCommunicator();

    static PolicyCommunicator* CreateInstance();
    void Release();

	BOOL QueryPolicy(_In_ CQueryPCInfo& queryInfo, _Out_ BOOL &bConnectSuccess);

    BOOL QueryOutlookPolicy(MAILTYPE mtMailType,
                            LPCWSTR wzAttachment,
							LPCWSTR wzResolvedPath,
                            LPCWSTR wzClientIdPropertyName,
                            LPCWSTR wzClientId,
                            STRINGLIST& pwzRecipients,
                            STRINGLIST& pwzDenyRecipients,
                            CEEnforcement_t* pEnforcement);
    BOOL QueryOutlookPolicyEx(MAILTYPE mtMailType,
		LPCWSTR wzAttachment,
		LPCWSTR wzResolvedPath,
		STRINGLIST& pwzRecipients,
		STRINGLIST& pwzDenyRecipients,
		CEEnforcement_t* pEnforcement,
		std::vector<std::pair<std::wstring,std::wstring>>& attrs);
			
    BOOL LogDecision(LPCWSTR cookie,
                     CEResponse_t userResponse, 
                     CEAttributes* optAttributes);


	BOOL PolicyCommunicator::QueryOutlookPreviewPolicy(LPCWSTR AttachmentPath);

	void SetMultiplyFlag();
	
public:
	static CEHandle	   m_sdk;
	static BOOL				   m_bSDK;
	static CELogging::Handle	   m_ceLogging;
	static int m_nQueryPCTimeout;
	static BOOL InitSDK();
	static VOID DeinitSDK();

	static WCHAR	m_wzSenderName[128];
	static wstring  m_strID;

	static void SetRecipients(STRINGLIST &listRecipients);
	static wstring GetRecipientsInfo();
	static void SetAttachmentSrcPath(ATTACHMENTLIST &listAttachments);
	static wstring GetAttachmentSrcPath();

	static STRINGLIST      m_listRecipients;
	static ATTACHMENTLIST  m_listAttachments;
	static bool WriteReportLog(const wchar_t* ObName,const std::vector<std::wstring>& value);
	static void SetLogID(const wchar_t* strID);

	static const wchar_t* GetUserSID() {return m_wzSID;}
	static const wchar_t* GetUserName() {return m_wzUserName;}
	static const wchar_t* GetHostName() {return m_wzHostName;}
	static const wchar_t* GetAppName() {return m_wzAppName;}
	static const wchar_t* GetAppPath() {return m_wzAppPath;}


protected:
    PolicyCommunicator();

    static void  InitData();
    static void  GetUserInfo(LPWSTR wzSid, int nSize, LPWSTR UserName, int UserNameLen);
    static DWORD GetIp(string& strIP);
	static int IPToValue(const string& strIP);

    //
    BOOL Connect2PolicyServer();
    void Disconnect2PolicyServer();
    BOOL QueryOutlookPolicySingle(CEAction_t operation, CEString Attachment, CEAttributes* psourceAttributes, LPCWSTR wzRecipient);

private:
    static BOOL                m_bFirstInit;
    static int                 m_nRef;
    static PolicyCommunicator* m_pThis;

    static WCHAR    m_wzSID[128];
    static WCHAR    m_wzUserName[128];
	
    static WCHAR    m_wzHostName[128];
    static WCHAR    m_wzAppName[];
    static WCHAR    m_wzAppPath[MAX_PATH];
    static CEHandle m_connectHandle;
    static ULONG    m_ulIp;
	wstring			m_strTickTime; // this parameter for multiply query. we will determine multiply query, we just send pc one. 
	static string   m_strIp;
public: 
	COPParaMeterCERequest m_opparamtercerequest;


private:
	BOOL  GetAccountSid(LPCWSTR AccountName,WCHAR *wzSid, size_t nSize);
};

#endif