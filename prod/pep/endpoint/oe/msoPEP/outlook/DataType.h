#ifndef DATA_TYPE_H
#define DATA_TYPE_H

#include <Windows.h>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <algorithm>

#include "ParameterForMulQuery.h"

using namespace std;



#define  OE_OBLIGATION_DESCRIPTION L"Obligation Description: "

#define 	CONTENTREDACTION_EMAILADDRESS_SUBJECT						(1)
#define 	CONTENTREDACTION_CREDITCARDNUM_SUBJECT						(1<<1)
#define 	CONTENTREDACTION_CURRENCYVALUE_SUBJECT						(1<<2)
#define 	CONTENTREDACTION_PHONENUMBER_SUBJECT						(1<<3)
#define 	CONTENTREDACTION_SOCIALSECURITYNUM_SUBJECT					(1<<4)
#define 	CONTENTREDACTION_IPV4Address_SUBJECT 						(1<<5)
#define 	CONTENTREDACTION_DOB_SUBJECT								(1<<6)
#define 	CONTENTREDACTION_MAILINGADDRESS_SUBJECT						(1<<7)
#define 	CONTENTREDACTION_KEYWORD_SUBJECT							(1<<8)


#define 	CONTENTREDACTION_EMAILADDRESS_BODY							(1<<9)
#define 	CONTENTREDACTION_CREDITCARDNUM_BODY							(1<<10)
#define 	CONTENTREDACTION_CURRENCYVALUE_BODY							(1<<11)
#define 	CONTENTREDACTION_PHONENUMBER_BODY							(1<<12)
#define 	CONTENTREDACTION_SOCIALSECURITYNUM_BODY						(1<<13)
#define 	CONTENTREDACTION_IPV4Address_BODY 							(1<<14)
#define 	CONTENTREDACTION_DOB_BODY									(1<<15)
#define 	CONTENTREDACTION_MAILINGADDRESS_BODY						(1<<16)
#define 	CONTENTREDACTION_KEYWORD_BODY								(1<<17)



#define     LOOP_OBLIGATION_HDR								    (1) //
#define     LOOP_OBLIGATION_VERIFY_RECIPIENTS					(1<<1)
#define     LOOP_OBLIGATION_MUTIPLE_CLIENT_CONFIGURATION		(1<<2)
#define     LOOP_OBLIGATION_MISSING_TAG							(1<<3)
#define     LOOP_OBLIGATION_REJECT_UNLESS_SILENT_OVERRIDE		(1<<4)
#define     LOOP_OBLIGATION_OIRM_AUTOMATIC						(1<<5)
#define     LOOP_OBLIGATION_OIRM_INTERACTIVE					(1<<6)
#define     LOOP_OBLIGATION_ZIP_ENCRYTION						(1<<7)
#define     LOOP_OBLIGATION_PGP_ENCRYTION						(1<<8)
#define		LOOP_OBLIGATION_STRIP_ATTACHMENT					(1<<9)
#define     LOOP_OBLIGATION_EMAIL_APPROVAL						(1<<10)
#define     LOOP_OBLIGATION_INTERNAL_USE_ONLY					(1<<11)
#define     LOOP_OBLIGATION_DOMAIN_MISMATCH_CONFIRMATION		(1<<12)

#define     LOOP_OBLIGATION_FILE_TAGGING						(1<<13)

#define     LOOP_OBLIGATION_MAIL_ATTR_PARSING					(1<<17)
#define     LOOP_OBLIGATION_MAIL_NOTIFICATION					(1<<18)
#define     LOOP_OBLIGATION_LOGDECISION							(1<<19)
#define     LOOP_OBLIGATION_FILE_OHC							(1<<20)
#define     LOOP_OBLIGATION_FILE_TAGGING_AUTO					(1<<21)
#define     LOOP_OBLIGATION_FTPADAPTER							(1<<22)
#define     LOOP_OBLIGATION_FSADAPTER							(1<<23)
#define     LOOP_OBLIGATION_SPUPLOADADAPTER						(1<<24)

// LOOP_OBLIGATION_PREPEND_BODY, LOOP_OBLIGATION_APEND_BODY, LOOP_OBLIGATION_PREPEND_SUBJECT
#define     LOOP_OBLIGATION_PREPEND_BODY						(1<<25)
#define     LOOP_OBLIGATION_APEND_BODY							(1<<26)
#define     LOOP_OBLIGATION_PREPEND_SUBJECT						(1<<27)

#define  	CONTENTREDACTION_STR								L"NLCA_REDACTION"
#define  	CONTENTREDACTION_EMAILADDRESS_STR					L"\\b\\w(\\w|[.+#$!-])*@(\\w+\\.){1,3}\\w{2,6}\\b"
#define 	CONTENTREDACTION_CREDITCARDNUM_STR					L"\\b\\d{4}(\\s|[-]){0,1}\\d{4}(\\s|[-]){0,1}\\d{2}(\\s|[-]){0,1}\\d{2}(\\s|[-]){0,1}\\d{1,4}\\b"
#define 	CONTENTREDACTION_CURRENCYVALUE_STR					L"([\\x{0024}\\x{00a2}-\\x{00a5}\\x{20a1}-\\x{20cf}])(\\s)*((([-(]){0,1}\\d{1,3}([,.]\\d{3})*([,.]\\d{1,2}){0,1}[)]{0,1})|([,.]\\d{1,2}))"
#define 	CONTENTREDACTION_PHONENUMBER_STR					L"(([(]{0,1}\\d{3}([).-]|\\s)\\s{0,10}\\d{3}([-.]|\\s)\\d{4})|(\\b\\d{3}([.-]|\\s)\\d{4}))\\b"
#define 	CONTENTREDACTION_SOCIALSECURITYNUM_STR				L"\\b\\d{3}([- ]){0,1}\\d{2}([- ]){0,1}\\d{4}\\b"
#define 	CONTENTREDACTION_IPV4Address_STR 					L"\\b((2[0-4]\\d)|(25[0-5])|(1{0,1}\\d{1,2}))([.]((2[0-4]\\d)|(25[0-5])|(1{0,1}\\d{1,2}))){3}(/\\d{1,2}){0,1}\\b"
#define 	CONTENTREDACTION_DOB_STR							L"\\b\\d{1,2}\\s*/\\s*\\d{1,2}\\s*/\\s*(\\d{4}|\\d{2})|((Jan|Feb|Mar|Apr|May|Jun|Jul|Aug|Sep|Oct|Nov|Dec)\\w{0,6}(\\s)+\\d{1,2}(st|nd|rd|th){0,1}(\\s)*([,]){0,1}\\s*\\d{4})|(\\d{1,2}(st|nd|rd|th){0,1}(\\s)*(Jan|Feb|Mar|Apr|May|Jun|Jul|Aug|Sep|Oct|Nov|Dec)\\w{0,6}\\s*[,]{0,1}\\s*\\d{4})\\b"
#define 	CONTENTREDACTION_MAILINGADDRESS_STR					L"\\b(AL|AK|AS|AZ|AR|CA|CO|CT|DE|DC|FM|FL|GA|GU|HI|ID|IL|IN|IA|KS|KY|LA|ME|MH|MD|MA|MI|MN|MS|MO|MT|NE|NV|NH|NJ|NM|NY|NC|ND|MP|OH|OK|OR|PW|PA|PR|RI|SC|SD|TN|TX|UT|VT|VI|VA|WA|WV|WI|WY)(\\s)*\\d{5}(\\s|[-]\\d{4}){0,1}\\b"
#define 	CONTENTREDACTION_KEYWORD_STR						L"Keyword(s)"



#define OBLIGATION_INTEGRATED_RIGHTS_MANAGEMENT_AUTOMATIC_EXIST    L"OUTLOOK_INTEGRATED_RIGHTS_MANAGEMENT_AUTOMATIC_EXIST"
#define OBLIGATION_INTEGRATED_RIGHTS_MANAGEMENT_INTERACTIVE_EXIST  L"OUTLOOK_INTEGRATED_RIGHTS_MANAGEMENT_INTERACTIVE_EXIST"

#define OBLIGATION_PASSWORD_BASED_ENCRYPTION_EXIST				   L"PASSWORD_BASED_ENCRYPTION_EXIST"
#define OBLIGATION_PASSWORD_BASED_ENCRYPTION					   L"PASSWORD_BASED_ENCRYPTION"

#define OBLIGATION_IDENTITY_BASED_ENCRYPTION_EXIST				   L"IDENTITY_BASED_ENCRYPTION_EXIST"
#define OBLIGATION_IDENTITY_BASED_ENCRYPTION					   L"IDENTITY_BASED_ENCRYPTION"


#define OBLIGATION_FTPADAPTER								       L"FTPADAPTER"
#define OBLIGATION_FSADAPTER								       L"FSADAPTER"
#define OBLIGATION_SPUPLOADADAPTER							       L"SPUPLOADADAPTER"

#define OBLIGATION_FTPADAPTER_EXIST								   L"FTPADAPTER_EXIST"
#define OBLIGATION_FSADAPTER_EXIST								   L"FSADAPTER_EXIST"
#define OBLIGATION_SPUPLOADADAPTER_EXIST						   L"SPUPLOADADAPTER_EXIST"


/*
That MAPI property contains all the transport headers, including any x-headers.
For use with PropertyAccessor you'd use the DASL property value for PR_TRANSPORT_MESSAGE_HEADERS:
"http://schemas.microsoft.com/mapi/proptag/0x007D001F"
That's the DASL property tag, the equivalent of the MAPI hex property tag 0x007D001F.
     
You'd get that property as a string value and split the string on each newline. 
You'd then parse that line to see if it was an x-header.
     
All the Internet headers are stored in the received item as one long string. The property is 
PR_TRANSPORT_MESSAGE_HEADERS ("http://schemas.microsoft.com/mapi/proptag/0x007D001E") for ANSI (PT_STRING8) , 
or for Unicode it would be PR_TRANSPORT_MESSAGE_HEADERS_W ("http://schemas.microsoft.com/mapi/proptag/0x007D001F").
*/
#define PR_TRANSPORT_MESSAGE_HEADERS_URL L"http://schemas.microsoft.com/mapi/proptag/0x007D001F"

#define PR_INTERNET_TRANSPORT_HEADERS_URL L"http://schemas.microsoft.com/mapi/proptag/0x007d001e"

// 1.3.2 Commonly Used Property Sets https://msdn.microsoft.com/en-us/library/ee219487(v=exchg.80).aspx
//   In the property list that forms the main body of this document (section 2), each named property is listed
// with the GUID that describes its property set. Several of the more common property sets and their 
// functional areas are described in the following table.
// Area name  Property set name and GUID value
// Common     PS_PUBLIC_STRINGS {00020329-0000-0000-C000-000000000046}
// Email      PS_INTERNET_HEADERS {00020386-0000-0000-C000-000000000046}
#define PS_INTERNET_HEADERS_GUID L"{00020386-0000-0000-C000-000000000046}"
#define MESSAGE_HEADER_NAME_SUFFIX L"http://schemas.microsoft.com/mapi/string/" PS_INTERNET_HEADERS_GUID L"/"

#define PR_CONVERSATION_INDEX_URL L"http://schemas.microsoft.com/mapi/proptag/0x00710102"

#define P_NAME_ORIGIN_MESSAGE_HEADER_W L"OriginMessageHeader"

#define DECISION_ON_TAG_ERROR_BLOCK L"block"

class CAttachmentObligationData;
class COE_PAMngr;

enum emObligationExistType
{
	emOBL_EXIST,      //OE had done the obligation,but it may be not this obligation. eg: OE had done HDR by policy test1. but didn't done HDR by policy test2 
	emOBL_NOT_EXIST,  //OE didn't done the obligation
	emOBL_EXIST_SAME  //OE had same policy and same obligation
};
enum emCAType
{
	emBODY,
	emSubject
};

class CAOblInfo 
{
public:
	CAOblInfo();
	~CAOblInfo();
	CAOblInfo& operator=(const CAOblInfo &caInfo);
	wstring    GetValue(){return m_strValue;}
	void	   SetValue(wstring &strValue){m_strValue = strValue;}
	size_t     GetCount() {return m_lCount;}
	void       SetCount(size_t nCount) { m_lCount = nCount;}
	wstring    GetDisplayOblName(){return m_strDisplayOblName;} 
	void       SetCAType(emCAType type) {m_caType = type;}
	emCAType   GetCAType(){return m_caType;}
	bool       GetDone(){return m_bDone;}
	void       SetDone(bool bDone){m_bDone = bDone;} 

private:
	wstring    m_strValue;
	size_t     m_lCount;
	wstring    m_strDisplayOblName;
	emCAType   m_caType;
	bool       m_bDone; //mark this obligation is done.
};


class CAOblData
{

public: 
	CAOblData() {}
	virtual ~CAOblData(){}
	
	bool      SetCAData(CAOblInfo &DesInfo, CAOblInfo &SrcInfo);
	bool      SetKeyWord(vector<CAOblInfo>& vecOblInfo, CAOblInfo &Info);

};

class CAOblSubjectData: public CAOblData
{
public:
	CAOblSubjectData(){};
	~CAOblSubjectData(){};

	CAOblInfo& GetEmailAddress_suject() {return m_EmailAddress_suject;}
	bool      SetEmailAddress_suject(CAOblInfo &Info);
	CAOblInfo& GetCreditCardNum_suject() {return m_CreditCardNum_suject;} 
	bool	  SetCreditCardNum_suject(CAOblInfo &Info);
	CAOblInfo& GetCurrencyValue_suject() {return m_CurrencyValue_suject;}
	bool	  SetCurrencyValue_suject(CAOblInfo &Info);
	CAOblInfo& GetPhoneNum_suject()       {return m_PhoneNum_suject;}
	bool	  SetPhoneNum_suject(CAOblInfo &Info);
	CAOblInfo& GetSocialSecurityNum_suject() {return m_SocialSecurityNum_suject;}
	bool	  SetSocialSecurityNum_suject(CAOblInfo &Info);
	CAOblInfo& GetIPAddress_suject()  {return m_IPAddress_suject;}
	bool	  SetIPAddress_suject(CAOblInfo &Info);
	CAOblInfo& GetDOB_suject() {return m_DOB_suject;}
	bool	  SetDOB_suject(CAOblInfo &Info);
	CAOblInfo& GetMailingAdd_suject() {return m_MailingAdd_suject;};
	bool	  SetMailingAdd_suject(CAOblInfo &Info);
	vector<CAOblInfo>& GetKeyWord_suject(){return m_vecKeyWord_suject;}
	bool      SetKeyWord_suject(CAOblInfo &Info);
private:

	CAOblInfo m_EmailAddress_suject;
	CAOblInfo m_CreditCardNum_suject;
	CAOblInfo m_CurrencyValue_suject;
	CAOblInfo m_PhoneNum_suject;
	CAOblInfo m_SocialSecurityNum_suject;
	CAOblInfo m_IPAddress_suject;
	CAOblInfo m_DOB_suject;
	CAOblInfo m_MailingAdd_suject;
	vector<CAOblInfo> m_vecKeyWord_suject;
};


class CAOblBodyData: public CAOblData
{
public:
	CAOblBodyData(){};
	~CAOblBodyData(){};

	CAOblInfo& GetEmailAddress_body() {return m_EmailAddress_body;}
	bool      SetEmailAddress_body(CAOblInfo &Info);
	CAOblInfo& GetCreditCardNum_body() {return m_CreditCardNum_body;} 
	bool	  SetCreditCardNum_body(CAOblInfo &Info);
	CAOblInfo& GetCurrencyValue_body() {return m_CurrencyValue_body;}
	bool	  SetCurrencyValue_body(CAOblInfo &Info);
	CAOblInfo& GetPhoneNum_body()       {return m_PhoneNum_body;}
	bool	  SetPhoneNum_body(CAOblInfo &Info);
	CAOblInfo& GetSocialSecurityNum_body() {return m_SocialSecurityNum_body;}
	bool	  SetSocialSecurityNum_body(CAOblInfo &Info);
	CAOblInfo& GetIPAddress_body()  {return m_IPAddress_body;}
	bool	  SetIPAddress_body(CAOblInfo &Info);
	CAOblInfo& GetDOB_body() {return m_DOB_body;}
	bool	  SetDOB_body(CAOblInfo &Info);
	CAOblInfo& GetMailingAdd_body() {return m_MailingAdd_body;};
	bool	  SetMailingAdd_body(CAOblInfo &Info);
	vector<CAOblInfo>& GetKeyWord_body(){return m_vecKeyWord_body;}
	bool      SetKeyWord_body(CAOblInfo &Info);
private:

	CAOblInfo m_EmailAddress_body;
	CAOblInfo m_CreditCardNum_body;
	CAOblInfo m_CurrencyValue_body;
	CAOblInfo m_PhoneNum_body;
	CAOblInfo m_SocialSecurityNum_body;
	CAOblInfo m_IPAddress_body;
	CAOblInfo m_DOB_body;
	CAOblInfo m_MailingAdd_body;
	vector<CAOblInfo> m_vecKeyWord_body;
};



class CSubjectData
{
public:
	CSubjectData():m_nCurrentObligationType(0), m_nExistObligationType(0), m_nRecordPos(-1), m_bIsSubjectChanged(false){}

	const std::wstring& GetOriginSubject(){return m_wstrOriginSubject;}
	void SetOriginSubject(const wchar_t* wszSubject) { m_wstrOriginSubject = wszSubject; }
	const std::wstring& GetTempSubject(){ return m_wstrTempSubject;}
	void SetTempSubject(const wchar_t* wszTempSubject) {m_wstrTempSubject = wszTempSubject;}
	int GetCurrentObligationType() { return m_nCurrentObligationType;}
	void SetCurrentObligationType(int nCurObType) { m_nCurrentObligationType = m_nCurrentObligationType|nCurObType;}
	int GetExistObligationType() { return m_nExistObligationType;}
	void SetExistObligationType(int nExistObType ){ m_nExistObligationType=m_nExistObligationType|nExistObType;}
	int GetRecordPos() { return m_nRecordPos;}
	void SetRecordPos(int nRecordPos ){ m_nRecordPos= nRecordPos;}
	void SetSubjectChanged(bool bChange){ m_bIsSubjectChanged = bChange;}
	bool IsSubjectChanged(){ return m_bIsSubjectChanged;}
	void UpdateSubjectData() {}

	bool GetSubjectFromMailItem(CComPtr<IDispatch> dspMailItem);
	
	emObligationExistType SetCASubjectOblData(CAOblInfo &caInfo,int nOblType);
	CAOblSubjectData&  GetCAOblSubjectData(){return m_casubjectdata;}

	map<wstring,bool>& GetPrependSubject(){return m_mapPrependSubject;}
	void SetPrependSubject(wstring& strBuf,bool bDone){m_mapPrependSubject[strBuf] = bDone;}
	bool IsExistSamePrependSubject(wstring &strBuf){return (m_mapPrependSubject.find(strBuf) == m_mapPrependSubject.end())?false:true;}

	void GetAuditLogInfo(wstring & strLogInfo);
private:
	std::wstring m_wstrOriginSubject;
	std::wstring m_wstrTempSubject;
	int          m_nCurrentObligationType;
	int          m_nExistObligationType;
	int          m_nRecordPos;
	bool         m_bIsSubjectChanged;
	CAOblSubjectData m_casubjectdata;
	map<wstring,bool> m_mapPrependSubject; //prepend subject adds content, bool true means the subject content had added
};


class CBodyData
{
public:
	CBodyData():m_nCurrentObligationType(0), 
		m_nExistObligationType(0),m_nRecordPos(-1),m_bIsBodyChanged(false){}
	CBodyData(const wchar_t* wszBody):m_wstrOriginBody(wszBody),m_nCurrentObligationType(0), 
		m_nExistObligationType(0),m_nRecordPos(-1),m_bIsBodyChanged(false){}

	const std::wstring& GetOriginBody() { return m_wstrOriginBody;}
	void SetOriginBody(const wchar_t* wszBody){ m_wstrOriginBody=wszBody;}
	const std::wstring& GetTempBody() { return m_wstrTempBody;}
	void SetTempBody(const wchar_t* wszTempBody){ m_wstrTempBody=wszTempBody;}
	int GetCurrentObligationType() {return m_nCurrentObligationType;}
	void SetCurrentObligationType(int nCurObType){m_nCurrentObligationType=m_nCurrentObligationType|nCurObType;}
	int GetExistObligationType() {return m_nExistObligationType;}
	void SetExistObligationType(int nExistObType){ m_nExistObligationType= m_nExistObligationType|nExistObType;}
	int GetRecordPos() { return m_nRecordPos;}
	void SetRecordPos(int nRecordPos){ m_nRecordPos = nRecordPos;}
	bool IsBodyChanged() { return m_bIsBodyChanged;}
	void SetBodyChanged(bool bChange){ m_bIsBodyChanged= bChange;}

	map<wstring,bool> &GetAppendBody(){return m_mapAppendBody;}
	map<wstring,bool> &GetPrependBody(){return m_mapPrependBody;}
	
	void SetAppendBody(wstring& strBuf,bool bDone){m_mapAppendBody[strBuf] = bDone;}
	void SetPrependBody(wstring& strBuf,bool bDone){m_mapPrependBody[strBuf] = bDone;}

	bool IsExistSameAppendBody(wstring &strBuf){return (m_mapAppendBody.find(strBuf) == m_mapAppendBody.end())?false:true;}
	bool IsExistSamePrependBody(wstring &strBuf){return (m_mapPrependBody.find(strBuf) == m_mapPrependBody.end())?false:true;}

	void UpdateBodyData() {}

	emObligationExistType SetCABodyOblData(CAOblInfo &caInfo,int nOblType);
	CAOblBodyData&  GetCAOblBodyData(){return m_cabodydata;}

	void GetAuditLogInfo(wstring & strLogInfo);

	bool GetBodyFromMailItem(CComPtr<IDispatch> dspMailItem);
	Outlook::OlBodyFormat GetBodyFormat(){return m_bodyFormat;};

	const std::wstring& GetStripAttachmentTopMessage() { return m_strStripAttachmentTop;}
	const std::wstring& GetStripAttachmentBottomMessage() { return m_strStripAttachmentButton;}
	void SetStripAttachmentTopMessage(wstring& strMsg){m_strStripAttachmentTop=strMsg;}
	void SetStripAttachmentBottomMessage(wstring& strMsg){m_strStripAttachmentButton=strMsg;}


	bool ReplaceParagraphsText(CComPtr<Word::_Document>& wordDoc);
	
	map<wstring,wstring>& GetReplaceInfo(){return m_mapReplaceInfo;}


private:
	std::wstring m_wstrOriginBody;
	std::wstring m_wstrTempBody;
	int          m_nCurrentObligationType;
	int          m_nExistObligationType;
	int          m_nRecordPos;
	bool         m_bIsBodyChanged;
	CAOblBodyData      m_cabodydata;
	Outlook::OlBodyFormat m_bodyFormat;

	std::wstring m_strStripAttachmentTop;
	std::wstring m_strStripAttachmentButton;

	map<wstring,bool> m_mapAppendBody; //append body adds content, bool true means the body content had added
	map<wstring,bool> m_mapPrependBody;//prepend body adds content, bool true means the body content had added
	map<wstring,wstring>   m_mapReplaceInfo;


};

typedef std::vector<std::pair<std::wstring,std::wstring>> VECTOR_TAGPAIR;
enum TAG_POS{TAG_POS_NXL=1, TAG_POS_SUMMARY, TAG_POS_CUSTOM};

class CMessageHeader
{
public:
	CMessageHeader()
		:m_nRecordPos(-1)
		,m_bAutoTag(false)
		,m_HCTagState(DO_NOTHING)
		,m_bHeaderInherited(false)
	{
		 m_pPAMngr = NULL;
	}

	~CMessageHeader(){ ReleasePAMngr(); }

	std::wstring& GetContent() { return m_wstrHeader;}
	void SetContent(const wchar_t* wszContent){ m_wstrHeader = wszContent;}
	
	std::wstring& GetHCTempFile() { return m_wsHCTempFile;}
	void SetHCTempFile(const wchar_t* wszHCTempFile){ m_wsHCTempFile = wszHCTempFile;}

	int GetRecordPos() { return m_nRecordPos;}
	void SetRecordPos(int nRecordPos){ m_nRecordPos = nRecordPos;}

	bool NeedAutoTag() { return m_bAutoTag;}
	void SetAutoTag(bool value){ m_bAutoTag = value;}

	VECTOR_TAGPAIR& GetTagPairs() { return m_vecTagPair;}
	//Note: it will override existing pair values
	void  SetTagPairs(VECTOR_TAGPAIR& vecTag) { MergePairs(vecTag, m_vecTagPair);}

	bool HasHCTagging() { return m_HCTagState != DO_NOTHING;}
	bool NeedHCTagging() { return m_HCTagState == NEED_TAGGING;}
	void SetNeedHCTagging() { m_HCTagState = NEED_TAGGING;}
	void SetHCTagged() { m_HCTagState = TAGGED;}

	void SetPAMngr(COE_PAMngr *value){ m_pPAMngr = value; }
	COE_PAMngr *GetPAMngr(){return m_pPAMngr;}

	void ReleasePAMngr()
	{ 		
		if (NULL != m_pPAMngr)
		{
			delete m_pPAMngr;
			m_pPAMngr = NULL;
		} 
	}

	VECTOR_TAGPAIR& GetHeaderPairs() { return m_vecHeaderPairs;}
	void  SetHeaderPairs(VECTOR_TAGPAIR& vecTag) { MergePairs(vecTag, m_vecHeaderPairs); }

	VECTOR_TAGPAIR& GetNextlabsHeaderPairs() { return m_vecNextlabsHeaderParis;}

	bool GetHeaderFromMailItem(CComPtr<IDispatch> dspMailItem);
	bool InheritHeaderIfNeeded(std::wstring& wsExtraHeaders);

	// Returns the count of and changed tags including addition and modification
	static int MergePairs(const VECTOR_TAGPAIR& from, VECTOR_TAGPAIR& to);
	static void DebugPrintPairs(const VECTOR_TAGPAIR& pairs, const wchar_t *pszLabel = L"X-Nextlabs Header Pair");

	// Iterate through x-header tags and put each tag pair into another container, but if a multiple values tag exists, split it as multiple tags with the same name.
	void PutTagsInAttributes(VECTOR_TAGPAIR &attrs)
	{
		struct Matcher
		{
			bool operator()(const std::pair<std::wstring,std::wstring>& xtag) const
			{
				return boost::iequals(xtag.first, *lpwsName);
			}
			std::wstring * lpwsName;
		}matcher;

		std::wstring wstrXHeaderTagKeyPrefix = theCfg.GetXHeaderKeyPrefix();
		VECTOR_TAGPAIR tags = m_vecHeaderPairs; // All inherited  x-header pairs
		
		for (VECTOR_TAGPAIR::iterator itHeaderTag = tags.begin(); itHeaderTag != tags.end(); ++itHeaderTag)
		{
			matcher.lpwsName = &itHeaderTag->first;
			// check whether a tag is of nextlabs, if yes, check multiple values before putting, otherwise, directly put.
			if(m_vecNextlabsHeaderParis.end() != std::find_if(m_vecNextlabsHeaderParis.begin(), m_vecNextlabsHeaderParis.end(), matcher))
			{
				PutHeaderTagInAttributes(wstrXHeaderTagKeyPrefix, *itHeaderTag, attrs);
			}else
			{
				attrs.push_back( std::make_pair(wstrXHeaderTagKeyPrefix + itHeaderTag->first, itHeaderTag->second) );
			}
		}
	}

	// put a x-header tag pair into another container, but if the value of a tag supports multiple values, split it.
	// e.g.
	// tag(L"x-foo", L"1") --> (L"<prefix>x-foo", L"1"); 
	// tag(L"x-bar", L"yes|no|") --> (L"<prefix>x-bar", L"yes"), (L"<prefix>x-bar", L"no");
	// tag(L"x-vex", L"2||") --> (L"<prefix>x-vex", L"2")
	static void PutHeaderTagInAttributes(const std::wstring& tagKeyPrefix, const std::pair<std::wstring,std::wstring> &tag, VECTOR_TAGPAIR &attrs)
	{
		const wchar_t delimiter = L'|';
		const std::wstring value = tag.second; // It may be multiple-values separated by the delimiter.
		size_t start = 0U, end = value.find(delimiter);
		while (end != std::string::npos)
		{
			if (end > start)
			{
				attrs.push_back(std::make_pair(tagKeyPrefix + tag.first, value.substr(start, end - start)));
			}
			start = end + 1;
			end = value.find(delimiter, start);
		}
		if (start < value.length())
		{
			attrs.push_back(std::make_pair(tagKeyPrefix + tag.first, value.substr(start)));
		}
	}

	// put a x-header tag pair into another container, but if the value of a tag supports multiple values, split it.
	// e.g. tag(L"x-foo", L"1") -> (L"x-foo", L"1"); tag(L"x-bar", L"yes|no|") -> (L"x-bar", L"yes"), (L"x-bar", L"no"); tag(L"x-vex", L"2||") -> (L"x-vex", L"2")
	static void PutTagInAttributes(const std::pair<std::wstring,std::wstring> &tag, VECTOR_TAGPAIR &attrs)
	{
		const wchar_t delimiter = L'|';
		const std::wstring value = tag.second; // It may be multiple-values separated by the delimiter.
		size_t start = 0U, end = value.find(delimiter);
		while (end != std::string::npos)
		{
			if (end > start)
			{
				attrs.push_back(std::make_pair(tag.first, value.substr(start, end - start)));
			}
			start = end + 1;
			end = value.find(delimiter, start);
		}
		if (start < value.length())
		{
			attrs.push_back(std::make_pair(tag.first, value.substr(start)));
		}
	}

	/**
	 * When sending an email failed or canceled, we must remember some tags that are tagged by OE. Because if a user change the 
	 * email content, some header tagging obligations can be invalid, we need remove those relevant tags. Thus, in some cases,
	 * we have to remove those tags by what we have remembered.
	 * 
	 * As such, when an email sent successfully, some user properties will be put into it by OE. However, when Forward/Reply/ReplyAll
	 * this in some folder, such as, InBox, Sent Items, those properties still exist and can affect this time, MUST check clear it.
	 * 
	 * To inspect them, you can use the following PowerShell commands:
	 *  
	 *  $PRBase = 'http://schemas.microsoft.com/mapi/string/{00020386-0000-0000-C000-000000000046}/'
	 *  $olApp = [Runtime.InteropServices.Marshal]::GetActiveObject('Outlook.Application')
	 *  $olItem = $olApp.ActiveExplorer().Selection.Item(1)  # MUST select the first item in you outlook explorer window or change me
	 *  $olItem.PropertyAccessor.GetProperty($PRBase + 'x-nextlabs')
	 *  $olItem.UserProperties
	 *  $olItem.UserProperties.Find('NLIncrementalHeaders').Value
	 */
	static HRESULT ResetNLIncrementalHeaderProperty(IDispatch *pItemDisp);

	// OE just updates headers of NextLabs 
	BOOL UpdateEmailHeaders(CComPtr<IDispatch> dspMailItem);

	std::wstring ErrorAction() const { return m_wsErrorAction; }
	void ErrorAction(std::wstring val) { m_wsErrorAction = val; }
	std::wstring MessageIfBlcok() const { return m_wsMessageIfBlcok; }
	void MessageIfBlcok(std::wstring val) { m_wsMessageIfBlcok = val; }
private:
	// A long string contains all message header pairs
	std::wstring m_wstrHeader;

	// all pairs need to auto-tag, that is a merged result, after all obligations are classified.
	VECTOR_TAGPAIR m_vecTagPair; 

	// Reuse the file tagging implemention to do OB_XHEADER_HIERARCHICAL_CLASSIFICATION
	// It means to first create a new temporary file and append existing headers into it, then 
	// reuse the file tagging process, finally get file tags to apply to the mail header.
	std::wstring m_wsHCTempFile;

	//Save the HC obligation from the PC query result and pop up a dialog when executing the obligation.
	COE_PAMngr *m_pPAMngr;

	// Save initial parsed headers whose keys are specified in the Third-Party field of the obligation Inherit X-header or #HEADER_NAME_NEXTLABS
	// and can be updated after auto-tagging or HC-tagging so that keep them up-to-date when #IsEvalAgain. It's a superset of #m_vecNextlabsHeaderParis
	VECTOR_TAGPAIR m_vecHeaderPairs;

	// Hold the x-headers of NextLabs(initially it should be not empty only after Reply/ReplyAll/Forward)
	// and will be updated after auto-tagging or HC-tagging
	VECTOR_TAGPAIR m_vecNextlabsHeaderParis;

	// continue/block. Note that all header related obligations share this
	std::wstring m_wsErrorAction;
	//if #m_wsErrorAction is block, a error dialog will be popped up with this message, Note that all header related obligations share this
	std::wstring m_wsMessageIfBlcok;

	bool m_bHeaderInherited;
	// An index of requests to query PC.
	int m_nRecordPos; 
	//If at least one policy that contains a/more OB_XHEADER_AUTO obligation match
	bool m_bAutoTag;
	//If multiple policies that contain the OB_XHEADER_HIERARCHICAL_CLASSIFICATION obligation match, 
	//only first policy and its first obligation takes effect.
	enum TagState{
		DO_NOTHING, // no need tagg
		NEED_TAGGING, // need to tagg
		TAGGED //has tagged
	} m_HCTagState;
};

class CTagTypeInfo
{
public:
	CTagTypeInfo(){}
	CTagTypeInfo(const VECTOR_TAGPAIR& vecNxlTag, const VECTOR_TAGPAIR& vecSummaryTag, const VECTOR_TAGPAIR& vecCustomTag):
	  m_vecNxlTagInfo(vecNxlTag), m_vecSummaryTagInfo(vecSummaryTag), m_vecCustomTagInfo(vecCustomTag){}

	  const VECTOR_TAGPAIR& GetNxlTag() { return m_vecNxlTagInfo;}
	  const VECTOR_TAGPAIR& GetSummaryTag() { return m_vecSummaryTagInfo;}
	  const VECTOR_TAGPAIR& GetCustomTag() { return m_vecCustomTagInfo;}
	  const VECTOR_TAGPAIR& GetAddHCTag() { return m_vecAddHCTagInfo;}
	  void  SetAddHCTag(VECTOR_TAGPAIR& vecTags);
	  void SetTag(const VECTOR_TAGPAIR& vecTags, TAG_POS tagPos);
	  BOOL IsTagExist(const VECTOR_TAGPAIR& vecTags, TAG_POS tagPos, BOOL bCheckKeyAndValue);

private:
	
	BOOL IsExistSameTagValue(const wstring& strOrgTagValue, const wstring& strTagValue);
private:
	VECTOR_TAGPAIR m_vecNxlTagInfo;
	VECTOR_TAGPAIR m_vecSummaryTagInfo;
	VECTOR_TAGPAIR m_vecCustomTagInfo;
	VECTOR_TAGPAIR m_vecAddHCTagInfo;
};

class CAttachmentTagData
{
public:
	//set tag method
	void SetSourceOriginNxlTag(const VECTOR_TAGPAIR& vecTags){ m_sourceOriginTagInfo.SetTag(vecTags, TAG_POS_NXL); }
	void SetSourceOriginSummaryTag(const VECTOR_TAGPAIR& vecTags){ m_sourceOriginTagInfo.SetTag(vecTags, TAG_POS_SUMMARY); }
	void SetSourceOriginCustomTag(const VECTOR_TAGPAIR& vecTags){ m_sourceOriginTagInfo.SetTag(vecTags, TAG_POS_CUSTOM); }
	void SetTempOriginNxlTag(const VECTOR_TAGPAIR& vecTags){ m_tempOriginTagInfo.SetTag(vecTags, TAG_POS_NXL); }
	void SetTempOriginSummaryTag(const VECTOR_TAGPAIR& vecTags){ m_tempOriginTagInfo.SetTag(vecTags, TAG_POS_SUMMARY); }
	void SetTempOriginCustomTag(const VECTOR_TAGPAIR& vecTags){ m_tempOriginTagInfo.SetTag(vecTags, TAG_POS_CUSTOM); }
	void SetSourceAddedAutoTagNxlTag(const VECTOR_TAGPAIR& vecTags){m_sourceAddedTagInfoAutoTag.SetTag(vecTags, TAG_POS_NXL);}
	void SetSourceAddedAutoTagSummaryTag(const VECTOR_TAGPAIR& vecTags){m_sourceAddedTagInfoAutoTag.SetTag(vecTags, TAG_POS_SUMMARY);}
	void SetSourceAddedAutoTagCustomTag(const VECTOR_TAGPAIR& vecTags) {m_sourceAddedTagInfoAutoTag.SetTag(vecTags, TAG_POS_CUSTOM);}
	void SetTempAddedAutoTagNxlTag(const VECTOR_TAGPAIR& vecTags){m_tempAddedTagInfoAutoTag.SetTag(vecTags, TAG_POS_NXL);}
	void SetTempAddedAutoTagSummaryTag(const VECTOR_TAGPAIR& vecTags){m_tempAddedTagInfoAutoTag.SetTag(vecTags, TAG_POS_SUMMARY);}
	void SetTempAddedAutoTagCustomTag(const VECTOR_TAGPAIR& vecTags) {m_tempAddedTagInfoAutoTag.SetTag(vecTags, TAG_POS_CUSTOM);}
	void SetSourceAddedInteractiveTagNxlTag(const VECTOR_TAGPAIR& vecTags){m_sourceAddedTagInfoInteractiveTag.SetTag(vecTags, TAG_POS_NXL);}
	void SetSourceAddedInteractiveTagSummaryTag(const VECTOR_TAGPAIR& vecTags){m_sourceAddedTagInfoInteractiveTag.SetTag(vecTags, TAG_POS_SUMMARY);}
	void SetSourceAddedInteractiveTagCustomTag(const VECTOR_TAGPAIR& vecTags) {m_sourceAddedTagInfoInteractiveTag.SetTag(vecTags, TAG_POS_CUSTOM);}
	void SetTempAddedInteractiveTagNxlTag(const VECTOR_TAGPAIR& vecTags){m_tempAddedTagInfoInteractiveTag.SetTag(vecTags, TAG_POS_NXL);}
	void SetTempAddedInteractiveTagSummaryTag(const VECTOR_TAGPAIR& vecTags){m_tempAddedTagInfoInteractiveTag.SetTag(vecTags, TAG_POS_SUMMARY);}
	void SetTempAddedInteractiveTagCustomTag(const VECTOR_TAGPAIR& vecTags) {m_tempAddedTagInfoInteractiveTag.SetTag(vecTags, TAG_POS_CUSTOM);}
	void SetSourceCurrentNxlTag(const VECTOR_TAGPAIR& vecTags){m_sourceCurrentTagInfo.SetTag(vecTags, TAG_POS_NXL);}
	void SetSourceCurrentSummaryTag(const VECTOR_TAGPAIR& vecTags){m_sourceCurrentTagInfo.SetTag(vecTags, TAG_POS_SUMMARY);}
	void SetSourceCurrentCustomTag(const VECTOR_TAGPAIR& vecTags) {m_sourceCurrentTagInfo.SetTag(vecTags, TAG_POS_CUSTOM);}
	void SetTempCurrentNxlTag(const VECTOR_TAGPAIR& vecTags){m_tempCurrentTagInfo.SetTag(vecTags, TAG_POS_NXL);}
	void SetTempCurrentSummaryTag(const VECTOR_TAGPAIR& vecTags){m_tempCurrentTagInfo.SetTag(vecTags, TAG_POS_SUMMARY);}
	void SetTempCurrentCustomTag(const VECTOR_TAGPAIR& vecTags) {m_tempCurrentTagInfo.SetTag(vecTags, TAG_POS_CUSTOM);}

	//get tag
	VECTOR_TAGPAIR GetSourceOriginNxlTag(){ return m_sourceOriginTagInfo.GetNxlTag(); }
	VECTOR_TAGPAIR GetSourceOriginSummaryTag(){return  m_sourceOriginTagInfo.GetSummaryTag(); }
	VECTOR_TAGPAIR GetSourceOriginCustomTag(){ return m_sourceOriginTagInfo.GetCustomTag(); }
	VECTOR_TAGPAIR GetTempOriginNxlTag(){ return m_tempOriginTagInfo.GetNxlTag(); }
	VECTOR_TAGPAIR GetTempOriginSummaryTag(){return  m_tempOriginTagInfo.GetSummaryTag(); }
	VECTOR_TAGPAIR GetTempOriginCustomTag(){ return m_tempOriginTagInfo.GetCustomTag(); }
	VECTOR_TAGPAIR GetSourceAddedAutoTagNxlTag(){return m_sourceAddedTagInfoAutoTag.GetNxlTag();}
	VECTOR_TAGPAIR GetSourceAddedAutoTagSummaryTag(){return m_sourceAddedTagInfoAutoTag.GetSummaryTag();}
	VECTOR_TAGPAIR GetSourceAddedAutoTagCustomTag() {return m_sourceAddedTagInfoAutoTag.GetCustomTag();}
	VECTOR_TAGPAIR GetTempAddedAutoTagNxlTag(){return m_tempAddedTagInfoAutoTag.GetNxlTag();}
	VECTOR_TAGPAIR GetTempAddedAutoTagSummaryTag(){return m_tempAddedTagInfoAutoTag.GetSummaryTag();}
	VECTOR_TAGPAIR GetTempAddedAutoTagCustomTag() { return m_tempAddedTagInfoAutoTag.GetCustomTag();}
	VECTOR_TAGPAIR GetSourceAddedInteractiveTagNxlTag(){return m_sourceAddedTagInfoInteractiveTag.GetNxlTag();}
	VECTOR_TAGPAIR GetSourceAddedInteractiveTagSummaryTag(){return m_sourceAddedTagInfoInteractiveTag.GetSummaryTag();}
	VECTOR_TAGPAIR GetSourceAddedInteractiveTagCustomTag() {return m_sourceAddedTagInfoInteractiveTag.GetCustomTag();}
	VECTOR_TAGPAIR GetTempAddedInteractiveTagNxlTag(){return m_tempAddedTagInfoInteractiveTag.GetNxlTag();}
	VECTOR_TAGPAIR GetTempAddedInteractiveTagSummaryTag(){return m_tempAddedTagInfoInteractiveTag.GetSummaryTag();}
	VECTOR_TAGPAIR GetTempAddedInteractiveTagCustomTag() {return m_tempAddedTagInfoInteractiveTag.GetCustomTag();}
	VECTOR_TAGPAIR GetSourceCurrentNxlTag(){return m_sourceCurrentTagInfo.GetNxlTag();}
	VECTOR_TAGPAIR GetSourceCurrentSummaryTag(){return m_sourceCurrentTagInfo.GetSummaryTag();}
	VECTOR_TAGPAIR GetSourceCurrentCustomTag() {return m_sourceCurrentTagInfo.GetCustomTag();}
	VECTOR_TAGPAIR GetTempCurrentNxlTag(){return m_tempCurrentTagInfo.GetNxlTag();}
	VECTOR_TAGPAIR GetTempCurrentSummaryTag(){return m_tempCurrentTagInfo.GetSummaryTag();}
	VECTOR_TAGPAIR GetTempCurrentCustomTag() {return m_tempCurrentTagInfo.GetCustomTag();}


	//check tag exist
	BOOL IsSourceOriginNxlTagExist(const VECTOR_TAGPAIR& vecTags, BOOL bCheckKeyAndValue=TRUE){return m_sourceOriginTagInfo.IsTagExist(vecTags, TAG_POS_NXL, bCheckKeyAndValue);}
	BOOL IsSourceOriginSummaryTagExist(const VECTOR_TAGPAIR& vecTags, BOOL bCheckkeyAndValue=TRUE){return m_sourceOriginTagInfo.IsTagExist(vecTags, TAG_POS_SUMMARY, bCheckkeyAndValue);}
	BOOL IsSourceOriginCustomTagExist(const VECTOR_TAGPAIR& vecTags, BOOL bCheckKeyAndValue=TRUE){return m_sourceOriginTagInfo.IsTagExist(vecTags, TAG_POS_CUSTOM, bCheckKeyAndValue);}
	BOOL IsTempOriginNxlTagExist(const VECTOR_TAGPAIR& vecTags, BOOL bCheckKeyAndValue=TRUE){return m_tempOriginTagInfo.IsTagExist(vecTags, TAG_POS_NXL, bCheckKeyAndValue);}
	BOOL IsTempOriginSummaryTagExist(const VECTOR_TAGPAIR& vecTags, BOOL bCheckkeyAndValue=TRUE){return m_tempOriginTagInfo.IsTagExist(vecTags, TAG_POS_SUMMARY, bCheckkeyAndValue);}
	BOOL IsTempOriginCustomTagExist(const VECTOR_TAGPAIR& vecTags, BOOL bCheckKeyAndValue=TRUE){return m_tempOriginTagInfo.IsTagExist(vecTags, TAG_POS_CUSTOM, bCheckKeyAndValue);}
	BOOL IsSourceAddedAutoTagNxlTagExist(const VECTOR_TAGPAIR& vecTags, BOOL bCheckKeyAndValue=TRUE){return m_sourceAddedTagInfoAutoTag.IsTagExist(vecTags, TAG_POS_NXL, bCheckKeyAndValue);}
	BOOL IsSourceAddedAutoTagSummaryTagExist(const VECTOR_TAGPAIR& vecTags, BOOL bCheckkeyAndValue=TRUE){return m_sourceAddedTagInfoAutoTag.IsTagExist(vecTags, TAG_POS_SUMMARY, bCheckkeyAndValue);}
	BOOL IsSourceAddedAutoTagCustomTagExist(const VECTOR_TAGPAIR& vecTags, BOOL bCheckKeyAndValue=TRUE){return m_sourceAddedTagInfoAutoTag.IsTagExist(vecTags, TAG_POS_CUSTOM, bCheckKeyAndValue);}
	BOOL IsTempAddedAutoTagNxlTagExist(const VECTOR_TAGPAIR& vecTags, BOOL bCheckKeyAndValue=TRUE){return m_tempAddedTagInfoAutoTag.IsTagExist(vecTags, TAG_POS_NXL, bCheckKeyAndValue);}
	BOOL IsTempAddedAutoTagSummaryTagExist(const VECTOR_TAGPAIR& vecTags, BOOL bCheckkeyAndValue=TRUE){return m_tempAddedTagInfoAutoTag.IsTagExist(vecTags, TAG_POS_SUMMARY, bCheckkeyAndValue);}
	BOOL IsTempAddedAutoTagCustomTagExist(const VECTOR_TAGPAIR& vecTags, BOOL bCheckKeyAndValue=TRUE){return m_tempAddedTagInfoAutoTag.IsTagExist(vecTags, TAG_POS_CUSTOM, bCheckKeyAndValue);}
	BOOL IsSourceAddedInteractiveTagNxlTagExist(const VECTOR_TAGPAIR& vecTags, BOOL bCheckKeyAndValue=TRUE){return m_sourceAddedTagInfoInteractiveTag.IsTagExist(vecTags, TAG_POS_NXL, bCheckKeyAndValue);}
	BOOL IsSourceAddedInteractiveTagSummaryTagExist(const VECTOR_TAGPAIR& vecTags, BOOL bCheckkeyAndValue=TRUE){return m_sourceAddedTagInfoInteractiveTag.IsTagExist(vecTags, TAG_POS_SUMMARY, bCheckkeyAndValue);}
	BOOL IsSourceAddedInteractiveTagCustomTagExist(const VECTOR_TAGPAIR& vecTags, BOOL bCheckKeyAndValue=TRUE){return m_sourceAddedTagInfoInteractiveTag.IsTagExist(vecTags, TAG_POS_CUSTOM, bCheckKeyAndValue);}
	BOOL IsTempAddedInteractiveTagNxlTagExist(const VECTOR_TAGPAIR& vecTags, BOOL bCheckKeyAndValue=TRUE){return m_tempAddedTagInfoInteractiveTag.IsTagExist(vecTags, TAG_POS_NXL, bCheckKeyAndValue);}
	BOOL IsTempAddedInteractiveTagSummaryTagExist(const VECTOR_TAGPAIR& vecTags, BOOL bCheckkeyAndValue=TRUE){return m_tempAddedTagInfoInteractiveTag.IsTagExist(vecTags, TAG_POS_SUMMARY, bCheckkeyAndValue);}
	BOOL IsTempAddedInteractiveTagCustomTagExist(const VECTOR_TAGPAIR& vecTags, BOOL bCheckKeyAndValue=TRUE){return m_tempAddedTagInfoInteractiveTag.IsTagExist(vecTags, TAG_POS_CUSTOM, bCheckKeyAndValue);}
	BOOL IsSourceCurrentNxlTagExist(const VECTOR_TAGPAIR& vecTags, BOOL bCheckKeyAndValue=TRUE){return m_sourceCurrentTagInfo.IsTagExist(vecTags, TAG_POS_NXL, bCheckKeyAndValue);}
	BOOL IsSourceCurrentSummaryTagExist(const VECTOR_TAGPAIR& vecTags, BOOL bCheckkeyAndValue=TRUE){return m_sourceCurrentTagInfo.IsTagExist(vecTags, TAG_POS_SUMMARY, bCheckkeyAndValue);}
	BOOL IsSourceCurrentCustomTagExist(const VECTOR_TAGPAIR& vecTags, BOOL bCheckKeyAndValue=TRUE){return m_sourceCurrentTagInfo.IsTagExist(vecTags, TAG_POS_CUSTOM, bCheckKeyAndValue);}
	BOOL IsTempCurrentNxlTagExist(const VECTOR_TAGPAIR& vecTags, BOOL bCheckKeyAndValue=TRUE){return m_tempCurrentTagInfo.IsTagExist(vecTags, TAG_POS_NXL, bCheckKeyAndValue);}
	BOOL IsTempCurrentSummaryTagExist(const VECTOR_TAGPAIR& vecTags, BOOL bCheckkeyAndValue=TRUE){return m_tempCurrentTagInfo.IsTagExist(vecTags, TAG_POS_SUMMARY, bCheckkeyAndValue);}
	BOOL IsTempCurrentCustomTagExist(const VECTOR_TAGPAIR& vecTags, BOOL bCheckKeyAndValue=TRUE){return m_tempCurrentTagInfo.IsTagExist(vecTags, TAG_POS_CUSTOM, bCheckKeyAndValue);}


	
	CTagTypeInfo& GetHCTagInfo() {return m_HCTag;}
private:  
    CTagTypeInfo  m_sourceOriginTagInfo;
    CTagTypeInfo  m_tempOriginTagInfo;

	CTagTypeInfo  m_sourceAddedTagInfoAutoTag;
	CTagTypeInfo  m_tempAddedTagInfoAutoTag;
	CTagTypeInfo  m_sourceAddedTagInfoInteractiveTag;
	CTagTypeInfo  m_tempAddedTagInfoInteractiveTag;

	CTagTypeInfo  m_sourceCurrentTagInfo;
	CTagTypeInfo  m_tempCurrentTagInfo;

	CTagTypeInfo  m_HCTag;
};

typedef struct _WARNING_MSG_INFO
{
	wstring strHeaderTxt;
	wstring strDisplayTxt;
	wstring strProceedBtnLabel;
	wstring strCancelBtnLabel;
	wstring strLogID;
	bool    bDoWarningMsg;
	_WARNING_MSG_INFO()
	{
		bDoWarningMsg = false;
	}
}WARNING_MSG_INFO,*LPWARNING_MSG_INFO;

class CAttachmentData
{
public:
	CAttachmentData(const wchar_t* wszSourcePath, const wchar_t* wszTempPath,const long lOriginalAttachIndex):m_wstrSourcePath(wszSourcePath), m_wstrTempPath(wszTempPath),m_lOriginalAttachIndex(lOriginalAttachIndex),
		m_wstrOrgTempPath(wszTempPath), m_bAttachmentUpdated(false), m_bRemoved(false), m_nExistObligationType(0),
		m_bIgnored(FALSE),
		m_bNxlSuccessful(FALSE),
		m_bNeedAutoNxl(FALSE),
		m_bNeedInteractiveNxl(FALSE),
	hdr(FALSE){
			ZeroMemory(hdrUrl, MAX_SRC_PATH_LENGTH * sizeof(hdrUrl[0]) );
			m_nAttachmentSendTimes = 0;
	}
	const long GetOriginalAttachIndex(){return m_lOriginalAttachIndex;}
	const std::wstring& GetSourcePath() { return m_wstrSourcePath;}
	const std::wstring& GetTempPath() { return m_wstrTempPath;}
	const std::wstring& GetOrgTempPath() {return m_wstrOrgTempPath; }
	void SetTempPath(const wchar_t* wszTempPath){ m_wstrTempPath = wszTempPath;}
     
	int GetCurrentObligationType() { return m_nCurrentObligationType;}
	void SetCurrentObligationType(int nCurObType){m_nCurrentObligationType= m_nCurrentObligationType | nCurObType;}
	int GetExistObligationType() { return m_nExistObligationType;}
	void SetExistObligationType(int nExistObType){ m_nExistObligationType = m_nExistObligationType | nExistObType;}
	int GetRecordPos() { return m_nRecordPos;}
	void SetRecordPos(int nRecPos){ m_nRecordPos = nRecPos;}
	std::vector<int> GetRecordPosForReceiveAction() { return m_vecRecordPosForRecevieAction;}
	void AddRecordPosForReceiveAction(int nPos ){ m_vecRecordPosForRecevieAction.push_back(nPos);}
	void ClearRecordPosForReceiveAction() { m_vecRecordPosForRecevieAction.clear();}
	std::vector<int> GetReceiverIndexForReceiveAction() { return m_vecReceiverIndexForReceiveAction;}
	void AddReceiverIndexForReceiveAction(int nIndex) { m_vecReceiverIndexForReceiveAction.push_back(nIndex);}
	void ClearReceiverIndexForReceiveAction() { m_vecReceiverIndexForReceiveAction.clear();}
	bool IsAttachmentUpdated() {return m_bAttachmentUpdated;}
	void SetAttachmentUpdated(bool bChanged){ m_bAttachmentUpdated=bChanged;}
	bool IsAttachmentRemoved(){return m_bRemoved;}
	void SetAttachmentRemoved(bool bRemove){ m_bRemoved = bRemove; }
	CAttachmentTagData& GetAttachmentTagData() { return m_TagData; }

	void SetDispName(wstring &dispname) {m_dispname = dispname;}
	wstring GetDispName(){return m_dispname;}

	std::wstring FileName() const { return m_filename; }
	void FileName(std::wstring &filename) { m_filename = filename; }

	const WARNING_MSG_INFO &GetWarningMsgInfo(){return m_WarningMsgInfo;}
	void SetWaringMsgInfo(WARNING_MSG_INFO &WarningMsgInfo);
	bool IsNeedDoWarningMsgInfo(){return m_WarningMsgInfo.bDoWarningMsg;}
	void GetAuditLogInfo(wstring& strLogInfo);

	void GetHCTagInfo(wstring& strLogInfo);

	BOOL GetAutoNxl(){return m_bNeedAutoNxl;}
	void SetAutoNxl(BOOL bAutoNxl) {m_bNeedAutoNxl = bAutoNxl;}
	
	BOOL GetInterActiveNxl(){return m_bNeedInteractiveNxl;}
	void SetInterActiveNxl(BOOL bInterActiveNxl){m_bNeedInteractiveNxl = bInterActiveNxl;}

	BOOL IsNxlSuccessful() const { return m_bNxlSuccessful; }
	void SetNxlSuccessful(BOOL val) { m_bNxlSuccessful = val; }

	BOOL IsIgnored(){return m_bIgnored;}
	void SetIgnored(BOOL b) {m_bIgnored = b;}

	BOOL IsFileNameChangedAfterObligation();
	wstring & GetStrLogID(){return m_strLogID;}
	void    SetStrLogID(wstring& strlogID){m_strLogID = strlogID;}

    const VECTOR_TAGPAIR& GetHSTagPair()
    {
        return m_TagData.GetHCTagInfo().GetAddHCTag();
    }

	const VECTOR_TAGPAIR& GetHCTagAlreadTagged()
	 //VECTOR_TAGPAIR& GetHCTagAlreadTagged()
	{
		return m_vecHCTagAlreadTagged;
	}

	void SetHCTagAlreadTagged(const VECTOR_TAGPAIR& vecTag)
	{
		m_vecHCTagAlreadTagged = vecTag;
	}

   bool GetOutlookTempModifyFlagAndUpdate(CComPtr<Outlook::Attachment> spAttachment);
   int IncreaseAttachmentOnsendTimes(CComPtr<Outlook::Attachment> spAttachment);
   int GetAttachmentOnSendTimes() { return m_nAttachmentSendTimes; }
   void UpdateLastModifyTimePropertyForTempFile(CComPtr<Outlook::Attachment> spAttachment);
   /*
	why oe need to add 'Cngd' tag on changed attachments?
	for all oe previous versions, in this scene, attachments sent out will not be changed:
	firstly click on "send", do hc or other obligations that will change attachments, then click "cancel"(one of obligation provide),
	secondly click on "send" and match no policies or match all obligations that will not change attachments and send out the email.
	*/
	void SetAttachmentUpdateProperty(CComPtr<Outlook::Attachment> spAttachment);
	bool GetAttachmentUpdateProperty(CComPtr<Outlook::Attachment> spAttachment);

	// Put some specific tags into attributes, e.g. file size, file name, and file extension
	void ExtractSpecificTags(std::vector<std::pair<std::wstring,std::wstring>>& attrs, CComPtr<IDispatch> MailItem);

private:
    int  GetAttachmentOnSendTimes(CComPtr<Outlook::Attachment> spAttachment);
	void SetAttachmentOnSendTimes(CComPtr<Outlook::Attachment> spAttachment,int nOnSendTimes);
	
     std::wstring GetPreOutlookLastModifyTime(CComPtr<Outlook::Attachment> spAttachment, const std::wstring& kstrSourcePath, int nOnsendTimes);
private:
    std::wstring m_wstrSourcePath;
	std::wstring m_wstrTempPath;   //this temp path will be modified when doing obligations.
	std::wstring m_wstrOrgTempPath; //this temp path is not be modified durning obligations.
	long		 m_lOriginalAttachIndex;//this is this object map the orgainal attachment index
	int          m_nCurrentObligationType; //记录当前的Obligation的类型,每次queryPC 后都要初始化
	int          m_nExistObligationType; // 记录我们曾经从PC获得的Obligation的类型
	int          m_nRecordPos;
	std::vector<int> m_vecRecordPosForRecevieAction;
	std::vector<int> m_vecReceiverIndexForReceiveAction;

	CAttachmentTagData m_TagData;
	bool         m_bAttachmentUpdated;
	bool         m_bRemoved;
	std::wstring m_dispname; // e.g. xx_subject.msg is xx_subject, xx.doc is xx.doc, but its implemention is commented.
	std::wstring m_filename; // not like dispname, it always has file extension name, e.g. xx_subject.msg is xx_subject.msg, xx.doc is xx.doc
	WARNING_MSG_INFO m_WarningMsgInfo;

	wstring		m_strLogID;
	VECTOR_TAGPAIR m_vecHCTagAlreadTagged;
    int m_nAttachmentSendTimes;
	std::wstring m_strTempFileLastModifyTimeDelay;
public:
	// External Recipient obligation
	BOOL            er;
	wchar_t         erUrl[MAX_SRC_PATH_LENGTH];
	wchar_t         erRecipients[1024];///size needs to be dynamic

	// Internal Use Only obligation
	BOOL            iuo;
	wchar_t         iuoUrl[MAX_SRC_PATH_LENGTH];

	// Missing Tag obligation
	BOOL            mt;
	wchar_t         mtUrl[MAX_SRC_PATH_LENGTH];
	wchar_t         mtPropertyName[1024];   // Name of custom property for
	// client ID
	wchar_t         mtClientNames[1024];///size needs to be dynamic
	wchar_t         mtClientIds[1024];///size needs to be dynamic
	wchar_t         mtNotClientRelatedId[1024];///size needs to be dynamic
	wchar_t         mtClientIdChosen[1024];///size needs to be dynamic

	// Domain Mismatch obligation
	BOOL            dm;
	wchar_t         dmUrl[MAX_SRC_PATH_LENGTH];
	wchar_t         dmRecipients[1024];///size needs to be dynamic
	wchar_t         dmClientName[1024];///size needs to be dynamic

	// Check Recipient License obligation
	BOOL            rl;
	wchar_t         rlUrl[MAX_SRC_PATH_LENGTH];
	wchar_t         rlRecipients[4096];///size needs to be dynamic

	// Multiple Client obligation
	BOOL            mc;
	wchar_t         mcUrl[MAX_SRC_PATH_LENGTH];
	wchar_t         mcRecipients[1024];///size needs to be dynamic

	// Hidden Data Removal obligation
	BOOL            hdr;
	wchar_t         hdrUrl[MAX_SRC_PATH_LENGTH];

	BOOL m_bNeedInteractiveNxl;
	BOOL m_bNeedAutoNxl;
	BOOL m_bNxlSuccessful;
	BOOL m_bIgnored; //Bug 41984 [oe8.5]it will show many gif file in alert message when email calendar 

};

class CRecipientData
{
public:
	CRecipientData(): m_bRecipientsChanged(false){}

	const STRINGLIST& GetOriginRecipients() {return m_lstOriginRecipients;}
	STRINGLIST& GetRealRecipients(){ return m_lstRealRecipients; }
	
	void AddRealRecipients(wstring& strRecipient){m_lstRealRecipients.push_back(strRecipient);}
	void SetRealRecipients(STRINGLIST & RecipientsList){m_lstRealRecipients = RecipientsList;}
	int GetRealRecipentsNum(){return (int)m_lstRealRecipients.size();}

	void SetRecipientsChanged(bool bChanged){m_bRecipientsChanged=bChanged;}
	bool IsRecipientsChanged() { return m_bRecipientsChanged;}

	bool GetRecipientsFromMailItem(CComPtr<IDispatch> dspMailItem);

	STRINGLIST& GetShowDenyRecipients(){return m_lstShowDenyRecipients;}
	void AddShowDenyRecipients(wstring& strRecipient);
	


	STRINGLIST GetRmRecipients(){return m_listRmRecipients;}
	void AddRmRecipients(wstring& strRecipient){m_listRmRecipients.push_back(strRecipient);}

	void GetAuditLogInfo(wstring& strLogInfo);

	void UpdateRealRecipients();

	void SetSendPCRecipients(STRINGLIST &lstSendPCRecipients){m_lstSendPCRecipients = lstSendPCRecipients;}
	STRINGLIST& GetSendPCRecipients(){return m_lstSendPCRecipients;}

	wstring &GetSenderInRecipient(){return m_strSenderInRecipient;}
	void    SetSenderInRecipint(const wstring& strSenderInRecipient){ m_strSenderInRecipient = strSenderInRecipient;}
private:
	STRINGLIST m_lstOriginRecipients;
	STRINGLIST m_lstRealRecipients;

	STRINGLIST m_listRmRecipients;
	STRINGLIST m_lstShowDenyRecipients;
	STRINGLIST m_lstSendPCRecipients;
	wstring    m_strSenderInRecipient;
	bool m_bRecipientsChanged;
};


typedef struct _DOMAINMISMATCHINFO
{
	BOOL         valid;
	std::wstring url;
	std::wstring recipients;
	std::wstring clientName;
	_DOMAINMISMATCHINFO()
	{
		valid = FALSE;
	}
}DOMAINMISMATCHINFO, *LPDOMAINMISMATCHINFO;

typedef struct _MULTIPLECLIENTINFO
{
	BOOL         valid;
	std::wstring url;
	std::wstring recipients;
	_MULTIPLECLIENTINFO()
	{
		valid = FALSE;
	}
}MULTIPLECLIENTINFO, *LPMULTIPLECLIENTINFO;

typedef struct _MISSINGTAGINFO
{
	BOOL         valid;
	std::wstring url;
	std::wstring propertyName;
	std::wstring clientNames;
	std::wstring clientIds;
	std::wstring notClientRelatedId;
	BOOL         bObligationDone;
	_MISSINGTAGINFO()
	{
		valid = FALSE;
		bObligationDone = false;
	}
}MISSINGTAGINFO, *LPMISSINGTAGINFO;

typedef struct _INTERNALUSEONLYINFO
{
	BOOL         valid;
	std::wstring url;
	bool bObligationDone;
	_INTERNALUSEONLYINFO()
	{
		valid = FALSE;
		bObligationDone = false;
	}
}INTERNALUSEONLYINFO, *LPINTERNALUSEONLYINFO;

typedef struct _EXTERNALRECIPIENTINFO
{
	BOOL         valid;
	std::wstring url;
	std::wstring recipients;
	bool bObligationDone;
	_EXTERNALRECIPIENTINFO()
	{
		valid = FALSE;
		bObligationDone = false;
	}
}EXTERNALRECIPIENTINFO, *LPEXTERNALRECIPIENTINFO;

typedef struct _HIDDENDATAREMOVALINFO
{
	BOOL         valid;
	std::wstring url;
	bool bObligationDone;
	_HIDDENDATAREMOVALINFO()
	{
		bObligationDone = false;
		valid = FALSE;
	}
}HIDDENDATAREMOVALINFO, *LPHIDDENDATAREMOVALINFO;

typedef struct _LOGDECISIONINFO
{
	BOOL         valid;
	std::wstring cookie;
	_LOGDECISIONINFO()
	{
		valid = FALSE;
	}
}LOGDECISIONINFO, *LPLOGDECISIONINFO;

typedef struct _MAILATTRIBUTEPARSE
{
	BOOL valid ;
	BOOL bIsBlock ;
	std::wstring msgType ;
	std::wstring strMessage ;
	ULONG ulMin ;
	ULONG ulMax ;
	std::wstring url;
	bool bObligationDone; //true 表示这个obligation已经执行过。 
	_MAILATTRIBUTEPARSE()
	{
		bObligationDone = false;
		valid = FALSE;
		bIsBlock = FALSE;
	}
}MAILATTRIBUTEPARSE,*LPMAILATTRIBUTEPARSE ;


typedef struct _MULRECIPIENTLICENSEINFO
{
	BOOL         valid;
	std::wstring recipients;
	std::wstring licenses;
	std::wstring strMessage;
	BOOL		 bAllowOverride;
	BOOL bObligationDone;
	_MULRECIPIENTLICENSEINFO()
	{
		valid = FALSE;
		bObligationDone = false;
	}
}MULRECIPIENTLICENSEINFO, *LPMULRECIPIENTLICENSEINFO;
typedef struct _MAILNOTFIATTR
{
public:
	BOOL	IsValid;
	std::wstring strViolation;
	std::wstring strLogID;
	std::wstring strWMsg;
	std::wstring strJust;
	std::wstring strWarnType;
	std::wstring strUrl;
	BOOL bObligationDone;

	DWORD ulMin;
	DWORD ulMax;
	_MAILNOTFIATTR()
	{
		IsValid = FALSE;
		strViolation = L"";
		strLogID=L"";
		strWMsg=L"";
		strJust=L"";
		strWarnType=L"";
		ulMax=10240;
		ulMin=0;
		strUrl = L"http://www.nextlabs.com/";
		bObligationDone = FALSE;
	}

}MAILNOTIFIATTR;

class COnlyAttachmentData
{
public:
	COnlyAttachmentData(){}
	~COnlyAttachmentData(){}


	MAILATTRIBUTEPARSE m_mapInfo;
	MAILNOTIFIATTR	 m_mailNotiInfo;
	INTERNALUSEONLYINFO m_IUOInfo;
	EXTERNALRECIPIENTINFO m_VerifyRecpientsInfo;
	MISSINGTAGINFO      m_MissingTagInfo;
	MULTIPLECLIENTINFO  m_mcInfo;
	DOMAINMISMATCHINFO  m_dmInfo;
	MULRECIPIENTLICENSEINFO m_rlInfo;

	BOOL m_bNeedDm;
	BOOL m_bNeedMc;  
	BOOL m_bNeedIuo;
	BOOL m_bNeedEr;
	BOOL m_bNeedHdr;
	BOOL m_bNeedStripAttachment;
	BOOL m_bNeedLd;
	BOOL m_bNeedMt;
	BOOL m_bNeedMailap; // true 表示 我们当前的这次EVA 含有 这个OBLIGATION, 并且 当前的EVA 没有做过
	BOOL m_bNeedMailNoti;
	BOOL m_bEmptyAttachment;
	BOOL m_bNeedCrl;
	BOOL m_bNeedInterFileTagging;
	BOOL m_bNeedAutoFileTagging;
	BOOL m_bNeedHCFileTagging;
	BOOL m_bNeedZIP;
	BOOL m_bNeedPGP;
	VARIANT_BOOL m_bIsWordMail;
	wstring m_RecordTempRecipientsForCrl;
	wstring m_strURMsgTxt;
	wstring m_strURMsgTimeOut;
	BOOL m_bNeedURMsg;
	BOOL m_bNeedRMC;
	
};

class AlertDetailItem{
public:
	AlertDetailItem(const std::wstring& item, const std::wstring& policy, EmQueryResCommand emQueryResCmd, int index)
		:m_wsItem(item)
		,m_wsPolicy(policy)
		,m_nIndex(index)
	{
		m_emQueryResCmd = emQueryResCmd;
	}

	operator std::wstring&() {
		return static_cast<std::wstring&>(m_wsItem);
	}

	std::wstring& Item(){ return m_wsItem; }
	std::wstring& Policy(){ return m_wsPolicy; }
	const std::wstring& Item() const { return m_wsItem; }
	const std::wstring& Policy()const { return m_wsPolicy; }
	EmQueryResCommand QueryResCmd() const{ return m_emQueryResCmd; }
	int Index() const { return m_nIndex; }
	void Index(int val) { m_nIndex = val; }

	// For all recipient items: index is 0, match depends address
	// for an attachment item: it depends index and file name
	struct Matcher
	{
		bool operator()(const AlertDetailItem& item) const
		{
			return 0 == item.m_wsItem.compare(text) && item.m_nIndex == index;
		}
		LPCWSTR text;
		int index;
	};

private:
	EmQueryResCommand m_emQueryResCmd;
	//BOOL m_bAllowed;
	std::wstring m_wsItem; // recipient address or attachment file name
	std::wstring m_wsPolicy;
	int m_nIndex; // the position of the object within the collection #AttachmentData. 
};

// a wrapper for alert message, structure:
// x MessageText1
//    - DetailItem11
//    - DetailItem12
// x MessageText2
//    - DetailItem21
//    - DetailItem22
// ! MessageText3
class AlertMessages{
public:
	typedef std::vector<AlertDetailItem> AlertDetailItems;
	typedef std::pair<std::wstring, AlertDetailItems > AlertMessage;
	typedef std::vector<AlertMessage> Iteratable;

	// merge alert message text and it's detail items
	void put(LPCWSTR pwzAlertMsg, LPCWSTR pwzPolicy, const std::wstring& wsItem, EmQueryResCommand emQueryResCmd, int index = 0);

	void put(const AlertMessages& alertMessages);

	void StringifyAndAppendTo(std::wstring & wsMessage);

	Iteratable& GetMessages() { return m_messages; }
	const Iteratable& GetMessages() const { return m_messages; }
	void clear(){ m_messages.clear();}

private:
	Iteratable m_messages;
};

// a message wrapper class contains message text and detail items
struct AlertMsg
{
	// define the message type 
	enum{ ATTACHMENT, RECIPIENT, OTHER };

	std::wstring text; // message text of the Rich Alert Message obligation 
	std::vector<std::wstring> items; // detail items: recipient(s), attachment(s), or empty
	bool allowed;
	short msgType; // message type: attachment, recipient, other

	AlertMsg(const std::wstring& msg, bool allow, short type)
		: text(msg)
		, allowed(allow)
		, msgType(type)
	{
	}

	void pushItemIfNotExist(const std::wstring& item)
	{
		std::vector<std::wstring>::iterator it = std::find(items.begin(), items.end(), item);
		if( it == items.end())
		{
			items.push_back(item);
		}
	}

	void pushItem(const std::wstring& item)
	{
		items.push_back(item);
	}
};

// all message collection of #AlertMsg
class AlertMsgs 
{
public:
	AlertMsgs()
	{}

	AlertMsgs(const AlertMessages &one, bool allow, short type)
	{
		addMessages0(one, allow, type);
	}

	size_t size() const { return messages.size(); }

	void addMessages0(const AlertMessages &one, bool allow, short type)
	{
		// Directly add the first messages into #alertMsgs without any check
		for(AlertMessages::Iteratable::const_iterator itMsg = one.GetMessages().begin(); itMsg != one.GetMessages().end(); ++itMsg)
		{
			if (AlertMsg::OTHER == type && !allow)
			{
				logd(L"addMessage0: %s", itMsg->first.c_str());
			}
			AlertMsg &alertMsg = add(AlertMsg(itMsg->first, allow, type));

			const AlertMessages::AlertDetailItems& srcItems = itMsg->second;
			for (AlertMessages::AlertDetailItems::const_iterator itItem = srcItems.begin(); itItem != srcItems.end(); ++itItem)
			{
				alertMsg.pushItem(itItem->Item());
			}
		}
	}

	void addUniqueMessagesWithoutItems(const AlertMessages &one, bool allow = true)
	{
		for(AlertMessages::Iteratable::const_iterator itMsg = one.GetMessages().begin(); itMsg != one.GetMessages().end(); ++itMsg)
		{
			if (NULL == findByMessageText(itMsg->first))
			{
				messages.push_back(AlertMsg(itMsg->first, allow, AlertMsg::OTHER));
			}
		}
	}

	void addRecipientMessages(const AlertMessages &one, bool allow)
	{ 
		// disallow the same items for an recipient message
		addMessages(one, allow, AlertMsg::RECIPIENT, &AlertMsg::pushItemIfNotExist);
	}
	void addAttachmentMessages(const AlertMessages &one, bool allow)
	{ 
		// allow the same items for an attachment message because outlook allows to attach the same file twice.
		addMessages(one, allow, AlertMsg::ATTACHMENT, &AlertMsg::pushItem);
	}

	AlertMsg &add(const AlertMsg &msg)
	{
		messages.push_back(msg);
		return messages.back();
	}

	void add(const AlertMsgs &msgs)
	{
		messages.insert(messages.end(), msgs.messages.begin(), msgs.messages.end());
	}

	AlertMsg* findByMessageText(const std::wstring &text)
	{
		struct  
		{
			bool operator()(const AlertMsg& alertMsg) const
			{
				return alertMsg.text == text;
			}
			const std::wstring& text;
		}matcher = {text};

		std::vector<AlertMsg>::iterator itAlertMsg = std::find_if(messages.begin(), messages.end(), matcher);
		return messages.end() == itAlertMsg ? NULL : &*itAlertMsg;
	}

	std::vector<AlertMsg>& Messages() { return messages; }
	const std::vector<AlertMsg>& Messages() const { return messages; }

private:
	std::vector<AlertMsg> messages;
	//std::map<std::wstring, AlertMsg> msg2AlertMsgIndexes;

	void addMessages(const AlertMessages &other, bool allow, short type, void (AlertMsg::*push)(const std::wstring& item))
	{
		for(AlertMessages::Iteratable::const_iterator itMsg = other.GetMessages().begin(); itMsg != other.GetMessages().end(); ++itMsg)
		{
			// Need to check if message exists before adding the message into this.messages 
			AlertMsg *pAlertMsg = findByMessageText(itMsg->first);
			if (NULL == pAlertMsg)
			{
				//AlertMsg &alertMsg = messages.add(AlertMsg(itMsg->first)); pAlertMsg = &alertMsg;
				pAlertMsg = &(add(AlertMsg(itMsg->first, allow, type)));
			}

			const AlertMessages::AlertDetailItems& srcItems = itMsg->second;
			for (AlertMessages::AlertDetailItems::const_iterator itItem = srcItems.begin(); itItem != srcItems.end(); ++itItem)
			{
				(pAlertMsg->*push)(itItem->Item());
			}
		}
	}
};

// Message data for RichAlertMessage
class CAlertMessageData
{
public:
	CAlertMessageData()
		: hasAlertObligation(FALSE)
	{
	}

	BOOL hasAlertObligation;
	AlertMessages m_deniedAttachmentMessages;
	AlertMessages m_allowedAttachmentMessages;
	AlertMessages m_deniedRecipientMessages;
	AlertMessages m_allowedRecipientMessages;
	AlertMessages m_deniedAttachmentRecipientMessages;
	AlertMessages m_allowedAttachmentRecipientMessages;
	AlertMessages m_deniedOtherMessages;
	AlertMessages m_allowedOtherMessages;

	void clear()
	{ 
		m_deniedAttachmentMessages.clear();
		m_allowedAttachmentMessages.clear();
		m_deniedRecipientMessages.clear();
		m_allowedRecipientMessages.clear();
		m_deniedAttachmentRecipientMessages.clear();
		m_allowedAttachmentRecipientMessages.clear();
		m_deniedOtherMessages.clear();
		m_allowedOtherMessages.clear();
	}
};

class CRichAlertMsgData
{
public:
	std::wstring m_strHeaderTextOnDeny;
	std::wstring m_strHeaderTextOnAllow;
	std::wstring m_strAlertMsg;
};

class CSendEmailData
{
public:
	CSendEmailData(): m_bAllowBegin(false), m_bAllow(false),m_bIsEvalAgain(false),m_bIsForward(false), 
		m_bInheritHeader(false), m_bNeedToInheritHeader(false), m_bEvalAgainForHeader(false)
		, m_nRichAlerMessageEvalAgainTime(0), m_nEvalAgainTimes(0)
		, m_bAllowOfRejectUnlessSilent(false)
	{};

	const STRINGLIST& GetOriginRecipients(){return m_recipientsData.GetOriginRecipients();}
	CBodyData& GetBodyData() { return m_bodyData;}
	CSubjectData& GetSubjectData() { return m_subjectData;}
	CMessageHeader& GetMessageHeader() { return m_messageHeader;}
	std::vector<CAttachmentData>& GetAttachmentData() { return m_vecAttachData;}
	CAttachmentData* GetAttachmentDataByIndex(long nIndex);
	//void DeleteOrganigerForMeeting(CSendEmailData& emailData);
	bool IsEqualSender(std::wstring wsRecipient);
	void SetEvalAgain(bool bAgain){ m_bIsEvalAgain = bAgain;}
	bool IsEvalAgain() { return m_bIsEvalAgain || m_bEvalAgainForHeader;}
	bool IsSubjectChanged() {return m_subjectData.IsSubjectChanged();}
	bool IsBodyChanged() { return m_bodyData.IsBodyChanged();}
	bool IsRecipientsChanged() { return m_recipientsData.IsRecipientsChanged();}
	bool HasAttachmentUpdated();
	void SetAttachmentUpdated(int nAttachIndex[], BOOL bUpdateValue[], size_t nCount);
	void SetAttachmentUpdated(int nAttachIndex, BOOL bUpdateValue);

	bool HasAttachmentRemoved();
    void ResetInformation();
	bool GetDataFromMailItem(CComPtr<IDispatch> dspMailItem, const std::map<long,std::wstring>& map3thAppAttachmentPath,ITEM_TYPE origEmailType, bool bNeedAssociate);

	void SetAllowBegin(bool bAllow) {m_bAllowBegin = bAllow;}
	bool GetAllowBegin() const {return m_bAllowBegin;}

	void SetAllow(bool bAllow) {m_bAllow = bAllow;}
	bool GetAllow(){return m_bAllow;}

	HWND GetWnd(){return m_hWnd;}
	void SetWnd(HWND hwnd){m_hWnd = hwnd;}

	bool GetForward(){return m_bIsForward;}
	void SetForward(bool bForward){m_bIsForward = bForward;}


	wstring& GetSender(){return m_strSender;}
	void     SetSender(wstring strSender){m_strSender = strSender;}

	CRecipientData& GetRecipientsData(){return m_recipientsData;}
	COnlyAttachmentData m_OnlyAttachmentData;
	CAttachmentObligationData * m_AttachmentOblData;

	map<wstring,wstring> GetAlertInfo(){return m_mapAlertInfo;}
	void SetAlertInfo(wstring& strAlertInfo, wstring& strAllow){m_mapAlertInfo[strAlertInfo] = strAllow;}

	CRichAlertMsgData& GetRichAlertMsgData() { return m_richAlertMsgData;}
	void SetRichAlertMsgData(const wchar_t* wszHeaderTextAllow, const wchar_t* wszHeaderTextDeny, const wchar_t* wszAlertMsg);

	BOOL GetBubbleMsgTxT(bool bIsAllow,wstring& strHeader, wstring & strBubbleMsg);
	void SetExistObligationTypeForAllAttachment(int nOblType);

	ITEM_TYPE GetEmailType() {return m_EmailType;}
	void SetEmailType(ITEM_TYPE & Type){m_EmailType = Type;}

	void SetWordDoc(Word::_Document* pWordDoc){ m_spWordDoc = pWordDoc;};
	CComPtr<Word::_Document> GetWordDoc() { return m_spWordDoc; }

	std::vector<int>& GetRecordPosForRecipients() { return m_vecRecordPosForRecipients;}
	CAlertMessageData& GetAlertMessageData() { return m_alertMessageData;}
	AlertMessages& GetDeniedAttachmentMessages() { return m_alertMessageData.m_deniedAttachmentMessages;}
	AlertMessages& GetAllowedAttachmentMessages() { return m_alertMessageData.m_allowedAttachmentMessages;}
	AlertMessages& GetDeniedRecipientMessages() { return m_alertMessageData.m_deniedRecipientMessages;}
	AlertMessages& GetAllowedRecipientMessages() { return m_alertMessageData.m_allowedRecipientMessages;}
	AlertMessages& GetDeniedAttachmentRecipientMessages() { return m_alertMessageData.m_deniedAttachmentRecipientMessages;}
	AlertMessages& GetAllowedAttachmentRecipientMessages() { return m_alertMessageData.m_allowedAttachmentRecipientMessages;}
	AlertMessages& GetDeniedOtherMessages() { return m_alertMessageData.m_deniedOtherMessages;}
	AlertMessages& GetAllowedOtherMessages() { return m_alertMessageData.m_allowedOtherMessages;}
	void ClearAllAlertMessages(){ m_alertMessageData.clear(); }
	void PutAlertMessages(CSendEmailData &emailData);
	//void SaveMsgAlertMessageData(){m_alertMessageDataMsg = m_alertMessageData;}
	//void SetMsgAlertMessage(){m_alertMessageData = m_alertMessageDataMsg;}

	BOOL GetRichAlerMessageEvalAgainTime() const { return m_nRichAlerMessageEvalAgainTime; }
	void SetRichAlerMessageEvalAgainTime(UINT val) { m_nRichAlerMessageEvalAgainTime = val; }

	UINT GetEvaAgainTimes() const { return m_nEvalAgainTimes; };
	void IncreaseEvaAgainTimes() { ++m_nEvalAgainTimes; };

	void SetNeedToInheritHeader(bool b) {m_bNeedToInheritHeader = b;}
	bool GetNeedToInheritHeader() { return m_bNeedToInheritHeader; }
	
	bool GetInheritHeader() { return m_bInheritHeader; }
	void SetInheritHeader(bool b) { m_bInheritHeader = b; }
	void SetHeaderInherited(bool b) {m_bHeaderInherited = b;}
	bool GetHeaderInherited() { return m_bHeaderInherited; }

	std::wstring ExtraHeadersNeedInheriting() const { return m_wsExtraHeadersNeedInheriting; }
	void ExtraHeadersNeedInheriting(std::wstring val) { m_wsExtraHeadersNeedInheriting = val; }

	void EnableEvalAgainForHeader() { m_bEvalAgainForHeader = true;}

	void ExecuteInheritHeaderObliation();
	
	// Remove trivial attachment item messages when recipient and document condition is configured in a policy, 
	// if only recipient is matched, it will cause alert message to be verbose.
	void RemoveTrivialAlertMessages();

	bool AllowOfRejectUnlessSilent() const { return m_bAllowOfRejectUnlessSilent; }
	void AllowOfRejectUnlessSilent(bool val) { m_bAllowOfRejectUnlessSilent = val; }

	BOOL HasUnremovedAttachment() const { return m_hasAttachment; }
	void SetHasUnremovedAttachment(BOOL val) { m_hasAttachment = val; }
protected:
	int GetAttachmentFromMailItem(CComPtr<IDispatch> dspMailItem, const std::map<long,std::wstring>& map3thAppAttachmentPath, ITEM_TYPE origEmailType, bool bNeedAssociate); 

private:
	CSubjectData m_subjectData;
	CBodyData   m_bodyData;
	CMessageHeader m_messageHeader;
	CRecipientData m_recipientsData;
	std::vector<CAttachmentData> m_vecAttachData;
	map<wstring,wstring> m_mapAlertInfo;
	CRichAlertMsgData m_richAlertMsgData;
	wstring m_strSender;
	bool m_bIsEvalAgain;
	bool m_bAllowBegin; // the aggregated enforcement result
	bool m_bAllow; // It may be inconsistent with #m_bAllowBegin after executing some obligation (see CAttachmentObligationData#ExcuteEmailVerifyEx)
	HWND m_hWnd;
	ITEM_TYPE m_EmailType;
	bool      m_bIsForward;
	CComPtr<Word::_Document> m_spWordDoc;
	
	std::vector<int> m_vecRecordPosForRecipients;
	CAlertMessageData m_alertMessageData;
	//CAlertMessageData m_alertMessageDataMsg;//keep the msg format attachment's alertmesssage
	UINT m_nRichAlerMessageEvalAgainTime;
	UINT m_nEvalAgainTimes;

	// Whether to inherit the x-headers originated from the email that triggers the event Reply/ReplyAll/Forward, by default false
	// It mainly affect whether to send x-headers request to PC and to Classify obligations from its response
	// It may be changed when #ExcuteOblgation by assigning it to #m_bNeedToInheritHeader
	bool m_bInheritHeader;

	// It may be changed when #ClassifyObligation and may be reset when #ExcuteOblgation 
	bool m_bNeedToInheritHeader;

	bool m_bEvalAgainForHeader;

	// Indicates that header has been inherited
	bool m_bHeaderInherited;

	//Supports to inherit classifications defined by 3rd parties except for those headers with keys matching #NEXTLABS_HEADER.
	std::wstring m_wsExtraHeadersNeedInheriting;

	// a simple workaround for Bug 45672 - [oe8.5]can't send email out if do reject unless silent override and Inherit_XHeader together
	bool m_bAllowOfRejectUnlessSilent;

	BOOL m_hasAttachment; 
};


#endif