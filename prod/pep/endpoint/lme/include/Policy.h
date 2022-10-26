#pragma once
#include <list>
#include <map>
#include <vector>

//#include "eframework.R2\eframework\platform\cesdk_attributes.hpp"

#define TIMEOUT_TIME                    50000
#define MAX_SRC_PATH_LENGTH				1024

static enum MAILTYPE{NEWMAIL=0, REPLYMAIL, FWMAIL, EXISTINGMAIL, OTHERSEND};
static enum _ext_AppType {apptUnknown=0, apptWord, apptExcel, apptPPT, apptOutlook};
static enum _ext_AttachType{attachWord=0, attachExcel, attachPPT, attachOthers};

typedef struct _AttachmentData
{
    _ext_AttachType type;
    wchar_t         dispname[MAX_PATH];
    wchar_t         src[MAX_SRC_PATH_LENGTH];
    wchar_t			resolved[MAX_SRC_PATH_LENGTH];
    wchar_t         temp[MAX_SRC_PATH_LENGTH];
    BOOL            hdr;
    wchar_t         hdrurl[MAX_SRC_PATH_LENGTH];
    int				iIndex;//the index in attachments collection object.
}AttachmentData, *LpAttachmentData;
typedef std::vector<LpAttachmentData>   ATTACHMENTLIST;
typedef std::vector<std::wstring>       STRINGLIST;





class CPolicy
{
public:
	static CPolicy* CreateInstance();
	void Release();
	bool LoadSDK() ;
public:
	BOOL QueryLiveMeetingPolicy( const wchar_t* pstrAttachment ,wchar_t* action) ;

    BOOL QueryLiveMeetingPolicy(  const wchar_t* pstrAttachment,wchar_t* action, STRINGLIST* pAttendees  )  ;
	virtual BOOL QueryPolicy( wchar_t* action, LPCWSTR wzAttachment, std::map<std::wstring, std::wstring>& mapAttributes, CEEnforcement_t& pEnforcement, STRINGLIST& pwzRecipients) ;
	BOOL QueryLiveMeetingPolicyWithAppProp(   const wchar_t* pstrAttachment,const wchar_t* pstrApp, wchar_t* action, STRINGLIST* pAttendees,const wchar_t* pszDest=NULL  ) ;
	BOOL QueryPolicyWithApp( wchar_t* action, LPCWSTR wzAttachment, std::map<std::wstring, std::wstring>& mapAttributes,LPCWSTR wzApplication, std::map<std::wstring, std::wstring>& appAttributes, CEEnforcement_t& pEnforcement,STRINGLIST& pwzRecipients,const wchar_t* pszDest=NULL ) ;
	BOOL Connect2PolicyServer() ;
	void Disconnect2PolicyServer()   ;

private:
	static DWORD GetLocalIP() ;
	static BOOL GetWindowUserInfo(  wchar_t *pszSid, INT iBufSize, wchar_t* pszUserName, INT inbufLen ) ;
	static void GetFQDN(wchar_t* hostname, wchar_t* fqdn, int nSize) ;
	BOOL GetFileLastModifiedTime( const std::wstring & strFileName, std::wstring& strLastModifiedTime);
	BOOL SetAttributes(const std::map<std::wstring, std::wstring>& mapAttributes, CEAttributes *pAttribute);
private:
	CPolicy() ;
	virtual ~CPolicy() ;
	static	VOID InitLocalInfo(void) ;
private:

	static BOOL                m_bFirstInit;
	static int                 m_nRef;
	static CPolicy*			   m_pThis;


#ifndef SID_LEN
#define	 SID_LEN  128
#endif
	static wchar_t    m_wzSID[SID_LEN];
	static wchar_t    m_wzUserName[SID_LEN];
	static wchar_t    m_wzHostName[SID_LEN];
	static wchar_t    m_wzAppName[MAX_PATH];
	static wchar_t    m_wzAppPath[MAX_PATH];
	static CEHandle   m_connectHandle;
	static ULONG      m_ulIp;
private:
	nextlabs::cesdk_loader m_cesdk;
};
