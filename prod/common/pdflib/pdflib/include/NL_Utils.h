#pragma once
#include <string>
#include <map>


using namespace std;

class CUtils
{
public:
	CUtils(void);
	~CUtils(void);

	static void RemoveCharAtFrontAndBack(string& strParam, char cChar);

	//Remove all non-number chars at front. after this function, the first char is always 0-9.
	static void RemoveInvalidCharAtFront(string& strValue);

	static void Encode(string& strText);
	static void Decode(string& strText);
	static bool SplitTags(const string& strTags, const string& strOurTags, map<string, string>& mapTags, map<string, string>& mapOurTags);
	static bool SplitNextlabsTag(const string& strTags, map<string, string>& mapTags);
	static bool AnalyzeMetaDataInfo(const string& strInfo, map<string, string>& mapTags);
	static bool InterceptString(_In_ const string& strOrg,_In_ const string& strStart, _In_ const string& strEnd, _Out_ string& strIntercept);
	static void SetDocumentTag(_In_ const string& strInfo,_In_ const string& strTagName,_In_ const string& strStart,_In_ const string& strEnd,_Out_ map<string, string>& mapTags);
	static string RemoveInvalidChar(char *ch, string::size_type len);
	static void CheckBufWideChar(string &strBuf);
	/*
	format:
	/ab (\(aa\))
	we need to find the last ")"
	*/
	static int FindEnderOfTagvalue(const string& strText);

	static string m_strTagValueSeparator;
	static string m_strTagSeparator;
};
