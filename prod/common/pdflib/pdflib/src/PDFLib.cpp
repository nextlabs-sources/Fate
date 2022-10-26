// PDFLib.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "NL_PDFMgr.h"
#include "NL_Utils.h"

struct TAG 
{
	char szTagName[1025];
	char szTagValue[4097];
};

class MyPdfTokenizer: public PdfTokenizer
{
public:
	MyPdfTokenizer(const char* buf, size_t len): PdfTokenizer(buf, len)
	{

	}
	void MyReadString(PdfVariant& rVariant, PdfEncrypt* encrypt)
	{
		ReadString(rVariant, encrypt);
	}
};

static string EncodeName(const string& src)
{
	PdfName _name(src);
	int len = static_cast<int>((_name.GetLength() + 1) * 3);
	char* buf = new char[len];
	memset(buf, 0, len * sizeof(char));
	PdfOutputDevice* device = new PdfOutputDevice(buf, len);
	_name.Write(device);

	buf[len - 1] = '\0';
	string strResult(buf);

	delete device;
	delete []buf;

	if (strResult.length() >= 2)
	{
		strResult = strResult.substr(1);
	}

	return strResult;
}

static string EncodeValue(const string& src)
{
	//this is very interesting, if src is an empty string, then pass it to PdfString, the lenght of PdfString is -2
	//maybe this is a bug of podofo. anyway, here we can return the value directly since empty string doesnt need to encode.
	if (src.length() == 0)
	{
		return src;
	}
	
	PdfString _value(src);
	int len = static_cast<int> ((_value.GetLength() + 1) * 2);
	char* buf = new char[len];
	memset(buf, 0, len * sizeof(char));
	PdfOutputDevice* device = new PdfOutputDevice(buf, len);
	_value.Write(device);
	buf[len - 1] = '\0';
	string strResult(buf);

	delete device;
	delete []buf;

	if (strResult.length() >= 3)
	{
		strResult = strResult.substr(1, strResult.length() - 2);
	}

	return strResult;
}

static string DecodeName(const string& src)
{
	return PdfName::FromEscaped(src).GetName();
}

static string DecodeValue(const string& src)
{
	PdfVariant v;
	MyPdfTokenizer t = MyPdfTokenizer(src.c_str(), src.length());
	t.MyReadString(v, NULL);

	string strResult;

	if (v.GetDataType() == ePdfDataType_String)
	{
		PdfString ps = v.GetString();
		const char* result = ps.GetString();
		pdf_long len = ps.GetLength();

		if (result != NULL)
		{
			strResult = string(result, len);
		}
	}
	
	return strResult;
}

int PDF_AddTags(const char* pszFileName, void* pParam, int nCount)
{
	if(!pszFileName || !pParam)
		return -1;

	TAG* pTags = (TAG*)pParam;
	map<string, string> mapTags;
	for(int i = 0; i < nCount; i++)
	{
		pTags[i].szTagName[sizeof(pTags[i].szTagName)/sizeof(char) - 1] = '\0';
		pTags[i].szTagValue[sizeof(pTags[i].szTagValue)/sizeof(char) - 1] = '\0';

		//encode to PDF format
		string tagName = EncodeName(pTags[i].szTagName);
		string tagValue = EncodeValue(pTags[i].szTagValue);

		mapTags[tagName] = tagValue;
	}

	CPDFMgr mgr;
	if(mgr.AddTags(pszFileName, mapTags))
		return 0;

	return -1;
}


int PDF_ReadTags(const char* pszFileName, void** pParam, int* nCount)
{
	if(!pszFileName)
		return -1;

	CPDFMgr mgr;

	map<string, string> mapAllTags;
	
	if(mgr.GetTags(pszFileName, mapAllTags))
	{
		if(nCount == NULL || pParam == NULL)
			return (int)mapAllTags.size();

		*nCount = (int) mapAllTags.size();
		*pParam = new TAG[*nCount];
		if(!(*pParam) )
			return -2;
		
		memset(*pParam, 0, sizeof(TAG) * (*nCount));
		TAG* pTags = (TAG*)*pParam;
		int nIndex = 0;
		for(map<string, string>::iterator itr = mapAllTags.begin(); itr != mapAllTags.end(); itr++)
		{
			string tagName = DecodeName((*itr).first);
			string tagValue = DecodeValue((*itr).second);

			strncpy_s(pTags[nIndex].szTagName, sizeof(pTags[nIndex].szTagName)/sizeof(char), tagName.c_str(), _TRUNCATE);
			strncpy_s(pTags[nIndex].szTagValue, sizeof(pTags[nIndex].szTagValue)/sizeof(char), tagValue.c_str(), _TRUNCATE);

			nIndex++;
		}
		return 0;
	}
	return -1;
}


int PDF_ReadSummaryTags(const char* pszFileName, void** pParam, int* nCount)
{
	if(!pszFileName)
		return -1;

	CPDFMgr mgr;

	map<string, string> mapAllTags;

	if(mgr.GetSummaryTags(pszFileName, mapAllTags))
	{
		if(nCount == NULL || pParam == NULL)
			return (int)mapAllTags.size();

		*nCount = (int) mapAllTags.size();
		*pParam = new TAG[*nCount];
		if(!(*pParam) )
			return -2;

		memset(*pParam, 0, sizeof(TAG) * (*nCount));
		TAG* pTags = (TAG*)*pParam;
		int nIndex = 0;
		for(map<string, string>::iterator itr = mapAllTags.begin(); itr != mapAllTags.end(); itr++)
		{
			string tagName = DecodeName((*itr).first);
			string tagValue = DecodeValue((*itr).second);

			strncpy_s(pTags[nIndex].szTagName, sizeof(pTags[nIndex].szTagName)/sizeof(char), tagName.c_str(), _TRUNCATE);
			strncpy_s(pTags[nIndex].szTagValue, sizeof(pTags[nIndex].szTagValue)/sizeof(char), tagValue.c_str(), _TRUNCATE);

			nIndex++;
		}
		return 0;
	}
	return -1;
}

int PDF_DeleteTags(const char* pszFileName, void* pParam, int nCount)
{
	if(!pszFileName || !pParam)
		return -1;

	TAG* pTags = (TAG*)pParam;
	map<string, string> mapTags;
	for(int i = 0; i < nCount; i++)
	{
		pTags[i].szTagName[sizeof(pTags[i].szTagName)/sizeof(char) - 1] = '\0';
		pTags[i].szTagValue[sizeof(pTags[i].szTagValue)/sizeof(char) - 1] = '\0';

		string tagName = EncodeName(pTags[i].szTagName);
		string tagValue = EncodeValue(pTags[i].szTagValue);

		mapTags[tagName] = tagValue;
	}

	CPDFMgr mgr;
	if(mgr.DeleteTags(pszFileName, mapTags))
		return 0;

	return -1;
}

bool PDF_IsPDFFile(const char* pszFileName)
{
	try
	{
		CPDFParser* pParser = new CPDFParser( pszFileName );

		bool bPDF = false;
		if(pParser)
		{
			bPDF = pParser->IsPDFFile();

			delete pParser;
			pParser = NULL;
		}

		return bPDF;
	}
	catch( const PoDoFo::PdfError & eCode )
	{
		printf("Exception, failed to determine if the current file is PDF file. error: %d\r\n", eCode.GetError());
	}

	return false;
}

//parse the PDF tagging format of 5.5
int PDF_ParseTags(LPCSTR pszRawData, void** pOut, int* nCount)
{
	if (pszRawData == NULL || pOut == NULL || nCount == NULL )
	{
		return -1;
	}

	std::string TagValue(pszRawData);
	TagValue = "(" + TagValue + ")";
	map<string, string> mapTags;
	CUtils::SplitNextlabsTag(TagValue, mapTags);

	if (mapTags.size() > 0)
	{
		*nCount = static_cast<int>(mapTags.size());
		TAG* pTags = new TAG[*nCount];
		if(pOut) *pOut = pTags;

		int i = 0;
		for(map<string, string>::iterator itr = mapTags.begin(); itr != mapTags.end(); itr++)
		{
			memset(&(pTags[i]), 0, sizeof(TAG));

			strncpy_s(pTags[i].szTagName, itr->first.c_str(), _TRUNCATE);
			strncpy_s(pTags[i].szTagValue, itr->second.c_str(), _TRUNCATE);
			i++;
		}
		return 0;
	}

	return -1;
}

void PDF_FreeMem(TAG* pTags)
{
	if (pTags)
	{
		delete []pTags;
	}

}
