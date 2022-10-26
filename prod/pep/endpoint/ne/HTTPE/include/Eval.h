#ifndef __FTPE_EVALUATION_H__
#define __FTPE_EVALUATION_H__
#include <list>
#include <map>
#include "CEsdk.h"

/*
define the basic attribute list.
key:	This is the identification for the attribute item. It is reserved.
Because the attribute has been parsed by the PEP, and the key also has been removed right now, 
so it can be empty. And the PEP should not push the attribute of number to the list of obligation.
Now:	For the attribute of obligation:
value: It is the value of attribute, the PA just concern this data for an obligation.
*/
typedef struct _tagATTRIBUTE
{
	std::wstring strKey ;		// The identify
	std::wstring strValue ;		// The key's value
}*PATTRIBUTE, ATTRIBUTE;
/*
Type defines the list of attributes.
*/
typedef std::list<ATTRIBUTE> ATTRIBUTELIST ;
/*
define the obligation data
The name of Obligation
strOBName: This is the name of obligation, it identifies the type obligation. An PA can impelment multi-obligaiton and a file also can have some.
The list of attribute
The second is the list of attribute. This data is corresponding to the obligation and back from the PDP.
*/
typedef struct _tagOBLIGATION
{
	std::wstring  strOBName ;	    // It is the identify string for the obligation.
	ATTRIBUTELIST attrList ;		// It is a list of attributes for the obligation.
}*POBLIGATION, OBLIGATION ;
/*
type define the list of obligation
*/
typedef std::list< OBLIGATION > OBLIGATIONLIST ;
#define OBLIGATION_NAME_ID              L"CE_ATTR_OBLIGATION_NAME"
#define OBLIGATION_NUMVALUES_ID         L"CE_ATTR_OBLIGATION_NUMVALUES"
#define OBLIGATION_VALUE_ID             L"CE_ATTR_OBLIGATION_VALUE"

#define REDIRECT_OBLIGATION_NAME		L"HTTP_REDIRECT"
#define REDIRECT_OBLIGATION_URL			L"Redirect_URL"

#define HTTP_HEADER_INJECTION_NAME		L"HTTP_HEADER_INJECTION"
#define HTTP_HEADER_INJECTION_KEY		L"Field_Key"
#define HTTP_HEADER_INJECTION_VALUE		L"Field_Value"

class CObligation
{
public:
	CObligation(void) ;
	~CObligation(void) ;
public:
	void PushObligationItem(const OBLIGATION& obligation)  ;
	ATTRIBUTELIST  GetAttributeListByName( const std::wstring & strOBName ) ;
	BOOL GetAttributeListByName2( const std::wstring & strOBName,  vector<ATTRIBUTELIST* >& vAttrs);
	INT GetObligationCount(VOID);/*{return (INT)m_listOb.size() ;}*/
	std::wstring GetAttrValueByName( const ATTRIBUTELIST& listAttr, const std::wstring & strAttrName ) ;
public:
	//for redirector obligation ....
	std::wstring GetRedirectURL(VOID) ;
	//for HTTP HEADER INJECTION
	BOOL GetHeaderInjectionData( vector<wstring> & vHeaderData  ) ;
	BOOL GetOBValue(const wstring& strOBName, const wstring& strKey, std::list<wstring>& strListValue);
private:
	 OBLIGATIONLIST m_listOb ;
};

typedef struct struEval_data
{
	std::wstring strSrc;
	std::wstring strDest;
	std::wstring strResourceType_Src;
	std::wstring strResourceType_Dest;
	DWORD		 dwPort;
}EVALDATA, *LPEVALDATA;

class CPolicy
{
public:
	static CPolicy* CreateInstance();
	void Release();
public:
	virtual BOOL QueryPolicy( const std::wstring&  strAction, LPVOID pEval_data, CEEnforcement_t& pEnforcement ) ;
	virtual BOOL QueryPolicy( const std::wstring& strAction, LPVOID pEval_data, std::map<std::wstring, std::wstring>& mapAttributes, CEEnforcement_t& pEnforcement ) ;
	BOOL Connect2PolicyServer() ;
	void Disconnect2PolicyServer()   ;
	BOOL LogDecision(LPCWSTR pszCookie, CEResponse_t userResponse, CEAttributes* optAttributes);
	BOOL LogObligationData(LPCWSTR pszLogID, LPCWSTR pszObligationName, CEAttributes *attributes);
protected:
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

	//	add for verdict cache
	CRITICAL_SECTION m_csVerdictCache;

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
public:
	static BOOL				   m_bSDK;
};
#endif