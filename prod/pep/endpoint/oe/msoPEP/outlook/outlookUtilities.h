
#pragma once
#ifndef _OUTLOOK_UTILITIES_H_
#define _OUTLOOK_UTILITIES_H_



#include <Winldap.h>
#include <tchar.h>
#include <Sddl.h>
#include <ExDisp.h>
#include <ExDispId.h>
#include <MAPI.h>
#include <MAPIUTIL.H>
#include <MAPIX.H>
#include <set>
#include <vector>
#include <string>
#include <../common/NLType.h>
using namespace std;

typedef struct tagEncryptTag
{
	wstring strTagName;
	wstring strTagValue;
}ENCRYPTTAG;

void ReadTag(const wstring&file, vector<pair<wstring,wstring>>& tags );
bool IsNxlFormat(const WCHAR * filename);
void ReadSummaryTag(const wstring&file, vector<pair<wstring,wstring>>& tags );
class OLUtilities
{
public:
#ifdef _DEBUG
		   static void PrintLastError(LPCWSTR str);
#endif /* _DEBUG */
    static void GetSenderAddr(CComPtr<IDispatch> spMailItem, wchar_t* wzSenderAddr, int maxSize);
    static void GetMailSubject(CComPtr<IDispatch> spMailItem, wchar_t* wzMailSubject, int maxSize);
    static int  GetMailRecipients(CComPtr<IDispatch> spMailItem);
	static int  SetMailRecipients(CComPtr<IDispatch> spMailItem, STRINGLIST& listRecipients);
    static int  GetMailRecipients(CComPtr<IDispatch> spMailItem, STRINGLIST& listRecipients);
	static int  GetOriginalRecipientCount(CComPtr<IDispatch> spMailItem);
	static int  ResetMailRecipients(CComPtr<IDispatch> spMailItem, const STRINGLIST& listRecipients);
    /*
	Modified by chellee for the bug 7562, Temperory
	static BOOL RemoveMailRecipient(CComPtr<IDispatch> spMailItem, int nAttachIndex);
	*/
	//-----------------------------------------------------------------------------------------------
	static BOOL RemoveMailRecipient(CComPtr<IDispatch> spMailItem, std::wstring szRecipient);
	static BOOL FindRecip_FromRecipientList( CComPtr<Outlook::AddressEntry> spAddrEntry, std::wstring szRecipient,INT debugIndentLevel);
    //-----------------------------------------------------------------------------------------------
    static void ExpandAddressEntry(CComPtr<Outlook::AddressEntry> spAddrEntry, STRINGLIST& listRecipients);
    static void ExpandAddressEntry(CComPtr<Outlook::AddressEntry> spAddrEntry, STRINGLIST& listRecipients, int debugIndentLevel);
    static HRESULT ExResolveName( IAddrBook* pAddrBook, LPWSTR lpszName, LPADRLIST *lpAdrList );
	 static void GetSmtpAddressByDisplayName(CComPtr<Outlook::_Application> spApp, LPCWSTR wzDispName, LPWSTR wzSmtpAddress, int maxSize);
    static void GetSmtpAddressByDisplayName2(CComPtr<IMAPISession> lpSession, LPCWSTR wzDispName, LPWSTR wzSmtpAddress, int maxSize);
	static void GetSmtpAddressFromOfficeContact(CComPtr<Outlook::_Application> spApp, LPCWSTR wzDispName, LPWSTR wzSmtpAddress, int maxSize);

    static BOOL GetOutlookTempFolder(std::wstring& strTempFolder, int nVersion);
    static int  GetAllAttachments(CComPtr<IDispatch> spMailItem, ATTACHMENTLIST& listAttachment, BOOL bNeedSave = TRUE);
	static BOOL IsIgnoredAttach(CComPtr<Outlook::Attachment>   spAttachment);
	static BOOL NeedReAttach(LPCWSTR pwzFullName);
	static BOOL CheckReattach(CComPtr<IDispatch> spMailItem, BOOL bReattaAll, BOOL bNeedSave = TRUE);
    static void ClearAttachmentsList(ATTACHMENTLIST& listAttachment);
	static void ActiveMailWindow(CComPtr<IDispatch> spMailItem,VARIANT_BOOL	&isWordMail);
    static BOOL PrependMailBody(CComPtr<IDispatch> spMailItem, LPCWSTR wzPrependText);
	static BOOL SetMailBody(CComPtr<IDispatch> spMailItem, LPCWSTR wzNewBody);
	static bool PreAppendHtml(std::wstring& wstrHtml, const std::wstring& wstrPrepend);
    static BOOL AppendMailBody(CComPtr<IDispatch> spMailItem, LPCWSTR wzAppendText);
    static BOOL PrependMailSubject(CComPtr<IDispatch> spMailItem, LPCWSTR wzPrependText);
	static BOOL SetMailSubject(CComPtr<IDispatch> spMailItem, LPCWSTR wzNewSubject);
    static BOOL GetAttachmentByIndex(CComPtr<IDispatch> spMailItem, int nAttachIndex/*base on 0*/, Outlook::Attachment** ppAttachment);
    static BOOL WriteTagMessageToAttachment(CComPtr<IDispatch> spMailItem, int nAttachIndex/*base on 0*/, LPCWSTR wzTagMessage);
    static BOOL ReadTagMessageToAttachment(CComPtr<IDispatch> spMailItem, int nAttachIndex/*base on 0*/, LPWSTR wzTagMessage, int nTagMessageLen);
    static BOOL RemoveAttachment(CComPtr<IDispatch> spMailItem, int nAttachIndex/*base on 0*/);
    static BOOL AppendFileLinkToMail(CComPtr<IDispatch> spMailItem, LPCWSTR lpwzText, LPCWSTR lpwzLink);

    static std::wstring GetAttachFileName(CComPtr<Outlook::Attachment> Attachment);
	static BOOL ApplyNXLEncrypt(_In_ wstring& strExEFullPath, _In_  wstring& strCommandLine,_In_ wstring& strEncryptFile);
	static BOOL ApplyInterActiveNXLEncrypt(_In_ wstring& strExEFullPath,_In_ wstring& strEncryptFile);
	static BOOL ApplyAutoNXLEncrypt(_In_ wstring& strExEFullPath,_In_ wstring& strEncryptFile,vector<ENCRYPTTAG>& vetTagInfo);
	static void WaitProcess(HANDLE hProcess);
    static BOOL SetAttachmentLongFileName(CComPtr<Outlook::Attachment> spAttachment, LPCWSTR wzLongFileName);
    static BOOL GetAttachmentLongFileName(CComPtr<Outlook::Attachment> spAttachment, LPWSTR wzLongFileName, int nBufLen);
    static BOOL SetAttachmentLongPathName(CComPtr<Outlook::Attachment> spAttachment, LPCWSTR wzLongFileName);
	static BOOL GetAttachmentLongPathName(CComPtr<Outlook::Attachment> spAttachment, LPWSTR wzLongFileName, int nBufLen);
	static BOOL SetAttachmentDispName(CComPtr<Outlook::Attachment> spAttachment, LPCWSTR wzDispName);
	static BOOL GetAttachmentDispName(CComPtr<Outlook::Attachment> spAttachment, LPWSTR wzDispName, int nBufLen);
		
	static HRESULT AutoWrap(int autoType, VARIANT *pvResult, CComPtr<IDispatch> pDisp, LPOLESTR ptName, int cArgs...);
	static BOOL CreateMulDirectory(wstring & strFilePath);
	/*
	add by chellee on  25/9/2008: 2:15 PM
	for the file is not have temp file.
	*/
	static INT CheckAndReattachSpecFile( CComPtr<IDispatch> spMailItem,
										CComPtr<Outlook::Attachments>	spAttachments,
										 CComPtr<Outlook::Attachment>	spAttachment,
										 wchar_t *i_pszFileName, 
										 wchar_t *i_pszDispName,
										 const UINT i_CurIndex ) ;
	/*added by chelle on 08/11/2010: for support appointment, share ,task
	*/
	static BOOL CheckGetMailItemType( CComPtr<IDispatch> pDisp, ITEM_TYPE& itemType ) ;
	static BOOL CheckReattach4TaskRequestItem(CComPtr<IDispatch> spMailItem,CComPtr<IDispatch> taskItem) ;
#ifdef XXXXXWSO2K7
	static HRESULT CreateWordInstance(LPVOID* pWordInst);
	static HRESULT CreateExcelInstance(LPVOID* pExcelInst);
	static HRESULT CreatePwptInstance(LPVOID* pPwptInst);
#endif
	static BOOL IsWordFile(LPCWSTR pwzFile);
	static BOOL IsRTFFile(LPCWSTR pwzFile);
	static BOOL IsExcelFile(LPCWSTR pwzFile);
	static BOOL IsPwptFile(LPCWSTR pwzFile);
	static BOOL IsExcelTemplate(LPCWSTR pwzFile);
	static BOOL IsPwptTemplateFile(LPCWSTR pwzFile);
	static BOOL IsWordTemplate(LPCWSTR pwzFile);
	static BOOL IsPDFFile( LPCWSTR pwzFile ) ;
	static BOOL	NeedReattachFileType(LPCWSTR pwzFile);

    static wstring newGUID();
    static wstring CreateTempFolder(BOOL bGetLongPath); // return end with '\'
	static wstring CreateOETempFolder(BOOL bGetLongPath); // return end with '\'
    
    static wstring NLGetLongFilePath(const wstring& kwstrFileShortPath);    // return end with '\', only used for folder
    static wstring NLGetSysTempFilePath(const BOOL kbGetLongPath);          // return end with '\'
    static wstring GetNewFileName(_In_ const wstring& kwstrRealSrc, _In_ const wstring& kwstrTempPath);
    static long GetFolderItemsCount(OlDefaultFolders emFolderType, bool bOnlyGetUnReadItems);    // return items count under the current folder, return -1, function failed
    
    static BOOL IsInReAttachFolder(const wstring& kwstrFilePath);
    static BOOL IsInPATempFolder(const wstring& kwstrFilePath);
    static BOOL IsInOETempFolder(const wstring& kwstrFilePath);

    static BOOL IsInSpecifiedFolder(const wstring& kwstrFilePath, const wstring& kwstrSpecifyFolder);

#ifdef XXXXXWSO2K7
	static CComPtr<Word::_Document>    OpenWordDoc(Word::_wordApplication** ppWdApp, LPCWSTR pwzFile);
	static CComPtr<Excel::_Workbook>   OpenExcelDoc(Excel::_excelApplication** ppExcApp, LPCWSTR pwzFile);
	static CComPtr<PPT::_Presentation> OpenPwptDoc(PPT::_pptApplication** ppPwptApp, LPCWSTR pwzFile);
#endif

	static void GetFQDN(LPCWSTR hostname, LPWSTR fqdn, int nSize);

	static CComPtr<IDispatch> GetPropPointer(CComPtr<IDispatch> pProps, LPCWSTR pwzName);
	static BOOL GetDocumentPropValue(CComPtr<IDispatch> pProps, LPCWSTR pwzName, LPWSTR pwzValue, int nSize);
	static BOOL SetDocumentPropValue(CComPtr<IDispatch> pProps, LPCWSTR pwzName, LPCWSTR pwzValue);
	static BOOL CreateDocumentProp(CComPtr<IDispatch> pProps, LPCWSTR pwzName, LPCWSTR pwzValue);
    static void OLUtilities::ParseStringList(LPCWSTR pwzStringList, LPCWSTR pwzDelimiter, STRINGLIST& vStrings);
    static BOOL OLUtilities::IsStringInListIgnoreCase(LPCWSTR pwzStringList, LPCWSTR pwzDelimiter, LPCWSTR pwzString);
	//static BOOL SetCustomProperties(LPDISPATCH pDisp, LPCWSTR pwzName, LPCWSTR pwzValue);
	//static BOOL GetCustomProperties(LPDISPATCH pDisp, LPCWSTR pwzName, LPWSTR pwzValue, int nSize);
	static std::wstring GetCommonComponentsDir() ;

	static std::string MyWideCharToMultipleByte(const std::wstring & strValue);
	static std::wstring MyMultipleByteToWideChar(const std::string & strValue);

	static BOOL IfAppUserInDomain();
	static BOOL GetAppUserInfo(std::wstring& wstrAppUserName, std::wstring& wstrAppUserDomain);

    static wstring DeleteHtmlFormat(_In_ const wstring &strMsgIn);

public:
	static CComPtr<IDispatch> GetCurrentItem() ;
	static void SetCurrentItem(CComPtr<IDispatch> spItem ) ;
	static BOOL IsExcelFile(const std::wstring& wstrFileName);
	static BOOL SetDeleteWhenRespond(int nDWR);
	static EMNLOE_OUTLOOKVERSION GetVersionNumber();

	static void GetFileLastModifyTime(const WCHAR* wzFileName, WCHAR* wzDateTime, int BufSize);
	static time_t WinTime2JavaTime(SYSTEMTIME* pSysTime);
	
};


#define HEADER_NAME_NEXTLABS L"X-NextLabs"
#define HEADER_NEXTLABS_TAG_SEPARATOR ':'

// A mail header parser, such as a .EML format file
// https://tools.ietf.org/html/rfc5322#section-2.2.3
struct MsgHeaderParser{
#define WSTR_CRLF L"\r\n"



typedef std::vector< std::pair<std::wstring,std::wstring> > HEADER_PAIRS;

	/// <param name="pwzHeaderContent">The message to extract headers from. Does not need the body part. Needs the empty headers end line.</param>
	MsgHeaderParser(LPCWSTR pwzHeaderContent)
		: pwzNextLineOffset(pwzHeaderContent)
	{}

	/// <summary>
	/// Method that takes a full message and extract the headers from it.
	/// </summary>
	/// <returns>A collection of Name and Value pairs of headers</returns>
	void ExtractHeaders(HEADER_PAIRS& headerPairs)
	{
		std::wstring headerName; // header name
		std::wstring headerValue; // the header value may be continued on the next line

		// Read until all headers have ended.
		// The headers ends when an empty line is encountered
		// An empty message might actually not have an empty line, in which
		// case the headers end with null value.
		while (ReadLine())
		{
			SeparateHeaderNameAndValue(headerName, headerValue);

			// Keep advancing until we would hit next header
			// This if for handling multi line headers
			while (IsMoreLinesInHeaderValue())
			{
				// Unfolding is accomplished by simply removing any CRLF
				// that is immediately followed by WSP
				// This was done using ReadLine (it discards CRLF)
				// See http://tools.ietf.org/html/rfc822#section-3.1.1 for more information
				if(ReadLine())
				{
					// Simply append the line just read to the header value
					headerValue.append(pwzLineOffset, pwzLineCRLF);
				}
			}
			
			// Now we have the name and full value. Add it
			boost::trim(headerValue);
			headerPairs.push_back(std::make_pair<>(headerName, headerValue));
		}
	}

	// \param arr is a separated by ':' string, e.g. L"x-PR:X-ITAR:X-EAR"
	// \param item is a substring to search, e.g. L"X-eAr"
	static bool StringifiedArrayContains(const std::wstring& arr, const std::wstring& item, wchar_t separatorChar = HEADER_NEXTLABS_TAG_SEPARATOR)
	{
		typedef const boost::iterator_range<std::wstring::const_iterator> StringRange;
		//auto found = boost::ifind_first(StringRange(arr.begin(), arr.end()), StringRange(item.begin(), item.end()));
		StringRange found = boost::ifind_first(arr, item);
		if(!found.empty())
		{
			if (found.begin() == arr.begin() || separatorChar == *(found.begin() - 1))
			{
				if (found.end() == arr.end() || separatorChar == found.end().operator*())
				{
#ifdef SAM_PRINT_TEST
					wprintf(L"[V]%s exactly contains %s at boundary or separators\n", arr.c_str(), item.c_str());
					int offset = found.begin() - arr.begin();
					wprintf(L"[V]%*c\n", offset + 1, '^');
#endif
					return true;
				}
			}
		}
#if 0
		std::wstring::size_type pos = arr.find(item); // only case sensitive
		if (std::wstring::npos != pos)
		{
			if (0 == pos || separatorChar == arr[pos - 1])
			{
				if (separatorChar == arr[pos + item.length()] || arr.length() == pos + item.length())
				{
					return true;
				}
			}	
		}
#endif
#ifdef SAM_PRINT_TEST
		wprintf(L"[X]%s cannot exactly match %s at boundary or separators\n", arr.c_str(), item.c_str());
#endif
		return false;
	}

	// Filter the header pairs that is specified in #headerKeys or #HEADER_NAME_NEXTLABS (excluding the pairs with an empty value
	// that are not allowed to PC query, @see #CE_RESULT_EMPTY_ATTR_VALUE)
	static std::wstring FilterHeader(const HEADER_PAIRS& srcHeaderPairs, const std::set<std::wstring>& headerKeys, HEADER_PAIRS &dstHeaderPairs, HEADER_PAIRS &dstNextlabsHeaderPairs)
	{
		struct 
		{
			bool operator()(const std::pair<std::wstring, std::wstring>& itPair) const
			{
				return boost::iequals(itPair.first, HEADER_NAME_NEXTLABS);
			}
		}xNextlabs_Header_matcher;

		std::wstring nextlabsHeaderKeys;
		HEADER_PAIRS::const_iterator itPair = std::find_if(srcHeaderPairs.begin(), srcHeaderPairs.end(), xNextlabs_Header_matcher);
		if (srcHeaderPairs.end() != itPair)
		{
			nextlabsHeaderKeys = itPair->second;
		}

		for( HEADER_PAIRS::const_iterator itPair = srcHeaderPairs.begin();itPair != srcHeaderPairs.end(); ++itPair)
		{
			if ( !itPair->second.empty())
			{
				if (headerKeys.end() != headerKeys.find(itPair->first))
				{
					dstHeaderPairs.push_back(*itPair);
				}else if (StringifiedArrayContains(nextlabsHeaderKeys, itPair->first))
				{
					dstHeaderPairs.push_back(*itPair);
					dstNextlabsHeaderPairs.push_back(*itPair);
				}
			}
		}
		return nextlabsHeaderKeys;
	}

#ifdef SAM_PRINT_TEST
	void TestNextlabsHeader()
	{
		HEADER_PAIRS headerPairs;
		HEADER_PAIRS nextlabsHeaderPairs;

		//C++11
		headerPairs.emplace_back(std::make_pair(L"Accept-Language",  L"en-US"       ));
		headerPairs.emplace_back(std::make_pair(L"Content-Language", L"en-US"       ));
		headerPairs.emplace_back(std::make_pair(L"X-MS-Has-Attach",  L"yes"         ));
		headerPairs.emplace_back(std::make_pair(L"x-nextlabs",       L"X-ITAR:X-EAR"));
		headerPairs.emplace_back(std::make_pair(L"x-itar",           L"itar-1"      ));
		headerPairs.emplace_back(std::make_pair(L"x-ear",            L"EAR-1"       ));
		headerPairs.emplace_back(std::make_pair(L"MIME-Version",     L"1.0"         ));

		FilterNextLabsHeader(headerPairs, nextlabsHeaderPairs);

		wprintf(L"\n\n");

		std::wstring nextlabsTag = L"x-PR:X-ITAR:X-EAR";
		std::wstring test;
		test = L"X-ITA"; StringifiedArrayContains(nextlabsTag, test);
		test = L"-ITAR"; StringifiedArrayContains(nextlabsTag, test);
		test = L"Z-ITAR"; StringifiedArrayContains(nextlabsTag, test);
		test = L"x-ITAR"; StringifiedArrayContains(nextlabsTag, test);
		test = L"X-PR"; StringifiedArrayContains(nextlabsTag, test);
		test = L"X-PR:"; StringifiedArrayContains(nextlabsTag, test);
		test = L":X-ITAR"; StringifiedArrayContains(nextlabsTag, test);
	}
#else
#define TestNextlabsHeader()
#endif

protected:
	LPCWSTR pwzNextLineOffset;
	LPCWSTR pwzLineOffset;
	LPCWSTR pwzLineCRLF;

	/// <summary>
	/// Separate a full header line into a header name and a header value.
	/// </summary>
	void SeparateHeaderNameAndValue(std::wstring &sKey, std::wstring& sVal)
	{
		LPCWSTR pwzColon = pwzLineOffset;
		while (pwzColon < pwzLineCRLF)
		{
			if(':' == *pwzColon)
			{
				sKey.assign(pwzLineOffset, pwzColon);
				sVal.assign(pwzColon + 1, pwzLineCRLF);
				return;
			}
			pwzColon++;
		}
		sKey.assign(pwzLineOffset, pwzLineCRLF);
		sVal = L"";
	}

	bool ReadLine()
	{
		if (NULL != pwzNextLineOffset)
		{
			pwzLineOffset = pwzNextLineOffset;
			pwzLineCRLF = wcsstr(pwzLineOffset, WSTR_CRLF);
			if (NULL == pwzLineCRLF) // not find CRLF
			{
				pwzLineCRLF = pwzLineOffset + wcslen(pwzLineOffset);
				pwzNextLineOffset = NULL;
			}else // find CRLF
			{
				// set it to start position of the next line
				pwzNextLineOffset = pwzLineCRLF + wcslen(WSTR_CRLF);
			}
			return NULL != pwzLineOffset;
		}else{
			return false;
		}
	}

	/// <summary>
	/// Check if the next line is part of the current header value we are parsing by
	/// peeking on the next character of the.<br/>
	/// This should only be called while parsing headers.
	/// </summary>
	/// <returns><see langword="true"/> if multi-line header. <see langword="false"/> otherwise</returns>
	bool IsMoreLinesInHeaderValue()
	{
		if (NULL == pwzNextLineOffset)
			return false;

		wchar_t startChar = (wchar_t) *pwzNextLineOffset;

		// A multi line header must have a whitespace character on the next line if it is to be continued
		return startChar == ' ' || startChar == '\t';
	}
};

//// #include <iostream>
//void testSplitXHeaders()
//{
//	//x-myproperty: Use PropertyAccessor.SetProperty and PS_INTERNET_HEADERS, Added\r\n
//	//by Sam
//	LPCWSTR pwzXHeaders = 
//		L"Received: from sv-exchdb02.company.com (10.30.0.97) by \r\n"
//		L" sv-exchdb01.company.com (10.30.0.96) with Microsoft SMTP Server (TLS) id \r\n"
//		L" 15.0.1236.3 via Mailbox Transport; Sun, 26 Feb 2017 17:08:24 -0800 \r\n"
//		L"Received: from sv-exchdb03.company.com (10.30.0.100) by \r\n"
//		L" sv-exchdb02.company.com (10.30.0.97) with Microsoft SMTP Server (TLS) id \r\n"
//		L" 15.0.1236.3; Sun, 26 Feb 2017 17:08:22 -0800 \r\n"
//		L"Received: from sv-exchdb03.company.com ([2266::6688:db6f:cbfb:930b]) by \r\n"
//		L" SV-EXCHDB03.company.com ([2266::6688:db6f:cbfb:930b%16]) with mapi id \r\n"
//		L" 15.00.1236.000; Sun, 26 Feb 2017 17:08:22 -0800 \r\n"
//		L"Content-Type: application/ms-tnef; name=\"winmail.dat\" \r\n"
//		L"Content-Transfer-Encoding: binary \r\n"
//		L"From: \"Abe\" <Abe@company.com> \r\n"
//		L"To: \"Sam\" <Sam@company.com>, \"Tom\" \r\n"
//		L"        <Tom@company.com> \r\n"
//		L"Subject: FW: My Documents \r\n"
//		L"Thread-Topic: My Documents \r\n"
//		L"Thread-Index: AdKQlUZlG2iY+it7RUCchBCcAO16QwAAIb7Q \r\n"
//		L"Date: Sun, 26 Feb 2017 17:08:22 -0800 \r\n"
//		L"Message-ID: <71af5db9fe1a441eae4dce9309f8c5f3@SV-EXCHDB03.company.com> \r\n"
//		L"Accept-Language: en-US \r\n"
//		L"Content-Language: en-US \r\n"
//		L"X-MS-Has-Attach: yes \r\n"
//		L"X-MS-Exchange-Organization-SCL: -1 \r\n"
//		L"X-MS-TNEF-Correlator: <71af5db9fe1a441eae4dce9309f8c5f3@SV-EXCHDB03.company.com> \r\n"
//		L"=x-myproperty: Use PropertyAccessor.SetProperty and PS_INTERNET_HEADERS, Added\r\n"
//		L" by Sam\r\n"
//		L"MIME-Version: 1.0 \r\n"
//		L"X-MS-Exchange-Transport-FromEntityHeader: Hosted \r\n"
//		L"X-MS-Exchange-Organization-MessageDirectionality: Originating \r\n"
//		L"X-MS-Exchange-Organization-AuthSource: SV-EXCHDB03.company.com \r\n"
//		L"X-MS-Exchange-Organization-AuthAs: Internal \r\n"
//		L"X-MS-Exchange-Organization-AuthMechanism: 04 \r\n"
//		L"X-Originating-IP: [10.30.50.155] \r\n"
//		L"X-MS-Exchange-Organization-Network-Message-Id: f53b8075-697d-4635-df94-08d45ead2031 \r\n"
//		L"Return-Path: Abe@company.com \r\n"
//		L"X-MS-Exchange-Organization-AVStamp-Enterprise: 1.0 \r\n";
//
//	std::vector< std::pair<std::wstring,std::wstring> > xheaderPairs;
//	//parseXHeaderPairs(pwzXHeaders, xheaderPairs);
//	MsgHeaderParser(pwzXHeaders).ExtractHeaders(xheaderPairs);
//
//	for(auto it = xheaderPairs.cbegin(); it != xheaderPairs.cend(); ++it)
//	{
//		std::wcout << it->first << L"|" << it->second << std::endl;
//	}
//}


#endif