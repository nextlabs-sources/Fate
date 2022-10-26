#include "stdafx.h"
#include "NL_Utils.h"
#include <vector>
#include <boost/algorithm/string.hpp>

#pragma warning(push)
#pragma warning(disable: 4512 6011)
#include <podofo.h>
#include "NL_PDFParser.h"
#pragma warning(pop)

string CUtils::m_strTagValueSeparator(0x01, 1);
string CUtils::m_strTagSeparator(0x02, 1);

using namespace PoDoFo;

CUtils::CUtils(void)
{
}

CUtils::~CUtils(void)
{
}

/*
format:
/ab (\(aa\))
we need to find the last ")"
*/
int CUtils::FindEnderOfTagvalue(const string& strText)
{
	string::size_type nIndex = 0;
	while( ( nIndex = strText.find(")", nIndex + 1) ) != string::npos && nIndex > 0)
	{
		if (strText[nIndex - 1] != '\\')
			return (int)nIndex;
	}

	return -1;
}

void CUtils::Encode(string& strText)
{
	std::auto_ptr<PdfFilter> pFilter = PdfFilterFactory::Create( ePdfFilter_ASCIIHexDecode );
	
	char* pOut = NULL;
	pdf_long nOut = 0;
	pFilter->Encode(strText.c_str(), strText.length(), &pOut, &nOut);
	
	strText = string(pOut, nOut);

	if(pOut)
	{
		free(pOut);
		pOut = NULL;
	}
}

void CUtils::Decode(string& strText)
{
	std::auto_ptr<PdfFilter> pFilter = PdfFilterFactory::Create( ePdfFilter_ASCIIHexDecode );

	char* pOut = NULL;
	pdf_long nOut = 0;
	pFilter->Decode(strText.c_str(), strText.length(), &pOut, &nOut);

	strText = string(pOut, nOut);

	if(pOut)
	{
		free(pOut);
		pOut = NULL;
	}
}

string CUtils::RemoveInvalidChar(char *ch, string::size_type len)
{
	if (ch == NULL)
	{
		return "";
	}
	char *p = new char[len+1];
	memset(p,0,len+1);
	int j = 0;
	for(string::size_type m = 0; m < len;m++)
	{
		if(ch[m] < 128 && ch[m] > 0)
		{
			p[j] = ch[m];
			j++;
		}
	}
	string strTemp = p;
	delete []p;
	return strTemp;
}


/*
function:
if the buf contains wide char, we will remove the 00 char. else we do nothing.
eg: buf is [0xef,0xff,Ox00,Ox43, Ox00, Ox56], it will become [Ox43, Ox56]
parameter:
strBuf: _inout
*/
void CUtils::CheckBufWideChar(string &strBuf)
{
	char *buf = new char[strBuf.length() + 1];
	memset(buf,0,strBuf.length() + 1);
	memcpy(buf,strBuf.c_str(),strBuf.length());
	string strCmpTemp = buf;
	if (strCmpTemp.length() != strBuf.length())
	{
		strBuf = RemoveInvalidChar(buf,strBuf.length());
	}
	delete []buf;
}

bool CUtils::SplitTags(const string& strTags, const string& strOurTags, map<string, string>& mapTags, map<string, string>& mapOurTags)
{
	string::size_type nIndex = strTags.find("/");
	string strTemp = strTags;

	
	if(!strOurTags.empty())
	{
		SplitNextlabsTag(strOurTags, mapOurTags);
	}

	while(nIndex != string::npos)
	{
		string strLine;
		int nPos = FindEnderOfTagvalue(strTemp);
		if(nPos < 0)
		{
			break;
		}

		nIndex = strTemp.find("/", nPos + 1);
		if(nIndex != string::npos)
			strLine = strTemp.substr(0, nIndex);
		else
			strLine = strTemp;


		std::vector<std::string> vecTags;
		boost::split(vecTags, strLine, boost::is_any_of("\n"));
		std::vector<std::string>::iterator itTag = vecTags.begin();
		while (itTag != vecTags.end())
		{
			if (!itTag->empty())
			{
				strLine = *itTag;
				string::size_type nStart = strLine.find("(");
				string::size_type nEnd = strLine.rfind(")");
				if (nStart==string::npos || nEnd==string::npos)
				{
					nStart = strLine.find("<");
					nEnd = strLine.rfind(">");
				}

				string strTagName, strTagValue;
				if (nStart != string::npos && nEnd != string::npos)
				{
					strTagName = strLine.substr(0, nStart);
					RemoveCharAtFrontAndBack(strTagName, ' ');
					RemoveCharAtFrontAndBack(strTagName, '\r');
					RemoveCharAtFrontAndBack(strTagName, '\n');

					strTagValue = strLine.substr(nStart + 1, nEnd - nStart - 1);
					CheckBufWideChar(strTagValue);
					if (_stricmp(strTagName.c_str(), NEXTLABS_TAG) != 0)//We have parsed our tags at before.
					{
						if (!strTagName.empty())
						{
							if (strTagName[0] == '/')
								strTagName = strTagName.substr(1, strTagName.length() - 1);

							mapTags[strTagName] = strTagValue;
						}
					}
				}

			}
			
				itTag++;
		}
	
		if(nIndex != string::npos)
		{
			strTemp = strTemp.substr(nIndex, strTemp.length() - nIndex);
			nIndex = strTags.find("/");
		}
		else
			break;
	}
	return true;
}

bool CUtils::SplitNextlabsTag(const string& strTags, map<string, string>& mapTags)
{
	string strTemp = strTags;

	if(strTemp.empty())
		return false;

	string::size_type i1 = strTemp.find("(");
	string::size_type i2 = strTemp.rfind(")");
	if(i1 != string::npos && i2 != string::npos && i2 > i1)
	{
		strTemp = strTemp.substr(i1 + 1, i2 - i1 - 1);
	}

	string::size_type nIndex = strTemp.find(NEXTLABS_CHECKSUM);
	if(nIndex != string::npos)
	{//remove the flag
		strTemp = strTemp.substr(0, nIndex);
	}

	CUtils::Decode(strTemp);

	nIndex = strTemp.find(CUtils::m_strTagSeparator);
	while(nIndex != string::npos)
	{
		string tag = strTemp.substr(0, nIndex);
		
		string::size_type nStart = tag.find(CUtils::m_strTagValueSeparator);
		if(nStart != string::npos)
		{
			string tagname = tag.substr(0, nStart);
			string tagvalue = tag.substr(nStart + 1, nIndex - nStart - 1);
			mapTags[tagname] = tagvalue;
		}

		strTemp = strTemp.substr(nIndex + CUtils::m_strTagSeparator.length(), strTemp.length() - nIndex - CUtils::m_strTagSeparator.length());
		nIndex = strTemp.find(CUtils::m_strTagSeparator);
	}

	return true;
}

void CUtils::RemoveInvalidCharAtFront(string& strValue)
{
	//try to find the start position of entries
	for(string::size_type m = 0; m < strValue.length();)
	{
		if(strValue[m] >= '0' && strValue[m] <= '9')
		{
			break;
		}
		else
		{
			strValue = strValue.substr(1, strValue.length() - 1);
			m = 0;
		}
	}
}

void CUtils::RemoveCharAtFrontAndBack(string& strParam, char cChar)
{
	//Remove at front
	string::size_type i = 0;
	for(i = 0; i < strParam.length(); )
	{
		if(strParam[i] == cChar)
		{
			strParam = strParam.substr(1);
			i = 0;
			continue;
		}
		break;
	}

	//Remove at back
	for(i = strParam.length() - 1; strParam.length() > 0; )
	{
		if(strParam[i] == cChar)
		{
			strParam = strParam.substr(0, strParam.length() - 1);
			i = strParam.length() - 1;
			continue;
		}
		break;
	}
}


bool CUtils::InterceptString(_In_ const string& strOrg,_In_ const string& strStart, _In_ const string& strEnd, _Out_ string& strIntercept)
{
	strIntercept = "";
	if (strOrg.empty()||strStart.empty()||strEnd.empty())
	{
		return false;
	}
	size_t nStartPos = strOrg.find(strStart);
	size_t nEndPos = strOrg.find(strEnd);
	if (nStartPos == string::npos || nEndPos == string::npos)
	{
		return false;
	}

	size_t offSet = nStartPos + strStart.length();
	if (offSet > nEndPos)
	{
		return false;
	}

	strIntercept =  strOrg.substr(offSet,nEndPos - offSet);

	return true;
}

void CUtils::SetDocumentTag(_In_ const string& strInfo,_In_ const string& strTagName,_In_ const string& strStart,_In_ const string& strEnd,_Out_ map<string, string>& mapTags)
{
	string strIntercept = "";
	bool   bReSet = true;
	map<string, string>::iterator mapItor = mapTags.find(strTagName);
	if (mapItor != mapTags.end())
	{
		if (!mapItor->second.empty())
		{
			bReSet = false;
		}
	}
	if (bReSet)
	{
		if (InterceptString(strInfo,strStart,strEnd,strIntercept) && !strIntercept.empty())
		{
			mapTags[strTagName] = strIntercept;
		}
	}
}
bool CUtils::AnalyzeMetaDataInfo(const string& strInfo, map<string, string>& mapTags)
{
	string strTitle = "Title";
	string strTitleStart = "<pdf:Title>";
	string strTitleEnd   = "</pdf:Title>";
	SetDocumentTag(strInfo,strTitle,strTitleStart,strTitleEnd,mapTags);

	string strAutor = "Author";
	string strAutorStart = "<pdf:Author>";
	string strAutorEnd   = "</pdf:Author>";
	SetDocumentTag(strInfo,strAutor,strAutorStart,strAutorEnd,mapTags);
	
	string strSubject = "Subject";
	string strSubjectStart = "<pdf:Subject>";
	string strSubjectEnd   = "</pdf:Subject>";
	SetDocumentTag(strInfo,strSubject,strSubjectStart,strSubjectEnd,mapTags);


	string strKeywords = "Keywords";
	string strKeywordsStart = "<pdf:Keywords>";
	string strKeywordsEnd   = "</pdf:Keywords>";
	SetDocumentTag(strInfo,strKeywords,strKeywordsStart,strKeywordsEnd,mapTags);

	string strModDate = "ModDate";
	string strModDateStart = "<pdf:ModDate>";
	string strModDateEnd   = "</pdf:ModDate>";
	SetDocumentTag(strInfo,strModDate,strModDateStart,strModDateEnd,mapTags);

	
	string strCreationDate = "CreationDate";
	string strCreationDateStart = "<pdf:CreationDate>";
	string strCreationDateEnd   = "</pdf:CreationDate>";
	SetDocumentTag(strInfo,strCreationDate,strCreationDateStart,strCreationDateEnd,mapTags);
	

	/*this is File->Document Properties->Application*/
	string strCreator = "Creator";
	string strCreatorStart = "<pdf:Creator>";
	string strCreatorEnd   = "</pdf:Creator>";
	SetDocumentTag(strInfo,strCreator,strCreatorStart,strCreatorEnd,mapTags);
	
	string strProducer = "Producer";
	string strProducerStart = "<pdf:Producer>";
	string strProducerEnd   = "</pdf:Producer>";
	SetDocumentTag(strInfo,strProducer,strProducerStart,strProducerEnd,mapTags);

	return true;
}
