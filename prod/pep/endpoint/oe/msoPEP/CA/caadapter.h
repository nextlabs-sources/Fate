#ifndef __H_CAADAPTER_H__
#define __H_CAADAPTER_H__

#include "nl_content_analysis.hpp"

#define NL_CA_CONTENTTYPE			L"ContentType"
#define NL_CA_CONTENTTYPE_SUBJECT	L"Email Subject"
#define NL_CA_CONTENTTYPE_BODY		L"Email Body"
#define	NL_CA_REDACTIONOBLIGATION_KEYWORD	L"Keyword(s)"

class CARegExpression
{
public:
	CARegExpression(){m_MatchCount=0;};
	std::wstring	m_RegexType;    // Opaque name used to construct NLCA::Expression instance
	std::wstring	m_Regex;		// Regular expression literal string (i.e. "\d{16}")
	size_t			m_MatchCount;   // Match count required for redaction
};
typedef std::vector<CARegExpression*> CARegExpressions;

class CAAdapter
{
public:
	
	CAAdapter(){};
	BOOL RedactBuffer( std::wstring& text_to_redact , CARegExpressions& regExps , bool& text_changed );
	BOOL RedactBufferBody( std::wstring& text_to_redact , CARegExpressions& regExps , bool& text_changed ,map<wstring,wstring>& mapReplaceBuf);

};

#endif //__H_CAADAPTER_H__
