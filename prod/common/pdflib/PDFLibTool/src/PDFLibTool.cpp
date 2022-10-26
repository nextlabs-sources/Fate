// PDFLibTool.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <Windows.h>
#include <string>
#include <map>

typedef int (*PDF_Func)(const char* pszFileName, void* pParam, int nCount);
typedef int (*PDF_ReadFunc)(const char* pszFileName, void** pParam, int* nCount);
typedef bool (*PDF_IsPDFFileType)(const char* pszFileName);

HMODULE g_hMod;

using namespace std;

struct TAG 
{
	char szTagName[1025];
	char szTagValue[4097];
};

void help()
{
	printf("PDFLib help command:\r\nPDFLibTool.exe -r -f \"c:\\a.pdf\" Show all tags\r\nPDFLibTool.exe -a -f \"c:\\a.pdf\" -t \"itar:yes;class:private\" Add tags\r\nPDFLibTool.exe -d -f \"c:\\a.pdf\" -t \"itar:yes;class:private\" Delete tags\r\n");
}

std::string MyWideCharToMultipleByte(const std::wstring & strValue)
{
	int nLen = WideCharToMultiByte(CP_ACP, 0, strValue.c_str(), (int)strValue.length(), NULL, 0, NULL, NULL); 

	char* pBuf = new char[nLen + 1];
	if(!pBuf)
		return "";
	memset(pBuf, 0, nLen +1);
	nLen = WideCharToMultiByte(CP_ACP, 0, strValue.c_str(), (int)strValue.length(), pBuf, nLen, NULL, NULL); 

	std::string strResult(pBuf);

	if(pBuf)
	{
		delete []pBuf;
		pBuf = NULL;
	}
	return strResult;
}

void PDFLib_ReadTag(char* pszFileName)
{
	PDF_ReadFunc lfRead = (PDF_ReadFunc)GetProcAddress(g_hMod, "PDF_ReadTags");
	if(lfRead)
	{
		TAG * pTags = NULL;
		int nTagCount = 0;
		DWORD dwStart = GetTickCount();
		
		lfRead(pszFileName, (void**)&pTags, &nTagCount);

		printf("calling PDF_ReadTags used: %d ms\r\n", GetTickCount() - dwStart);

		printf("Read tag from file: %s\r\n", pszFileName);
		for(int i = 0; i < nTagCount; i++)
		{
			printf("%s: %s\r\n", pTags[i].szTagName, pTags[i].szTagValue);
		}

		delete [] pTags;
		pTags = NULL;
		
	}
}

void PDF_AddTags(char* pszFileName, const char* pszTags)
{
	string strTags(pszTags);
	
	string::size_type nIndex;
	
	map<string, string> mapTags;
	while((nIndex = strTags.find(";")) != string::npos)
	{
		string::size_type nStart = strTags.find(":");
		if(nStart != string::npos)
		{
			string strTagName = strTags.substr(0, nStart);
			string strTagValue = strTags.substr(nStart + 1, nIndex - nStart - 1);

			mapTags[strTagName] = strTagValue;
		}
		strTags = strTags.substr(nIndex + 1, strTags.length() - 1);
	}

	if(mapTags.empty())
	{
		printf("Failed to parse tags, check the format please.\r\nFormat: tagname:tagvalue;\r\nLike:\r\nitar:yes;class:private;\r\n");
		return;
	}

	TAG* pTags = new TAG[mapTags.size()];
	if(pTags)
	{
		memset(pTags, 0, sizeof(TAG) * mapTags.size());

		map<string, string>::iterator itr;
		int index = 0;
		for(itr = mapTags.begin(); itr != mapTags.end(); itr++)
		{
			strncpy_s(pTags[index].szTagName, sizeof(pTags[index].szTagName), (*itr).first.c_str(), _TRUNCATE);
			strncpy_s(pTags[index].szTagValue, sizeof(pTags[index].szTagValue), (*itr).second.c_str(), _TRUNCATE);

			index++;
		}

		PDF_Func lfAdd = (PDF_Func)GetProcAddress(g_hMod, "PDF_AddTags");

		if(lfAdd)
		{
			lfAdd(pszFileName, pTags, (int)mapTags.size());
		}

		delete []pTags;
		pTags = NULL;
	}

}

void PDF_DeleteTags(char* pszFileName, const char* pszTags)
{
	string strTags(pszTags);

	string::size_type nIndex;

	map<string, string> mapTags;
	while((nIndex = strTags.find(";")) != string::npos)
	{
		string::size_type nStart = strTags.find(":");
		if(nStart != string::npos)
		{
			string strTagName = strTags.substr(0, nStart);
			string strTagValue = strTags.substr(nStart + 1, nIndex - nStart - 1);

			mapTags[strTagName] = strTagValue;
		}
		strTags = strTags.substr(nIndex + 1, strTags.length() - 1);
	}

	if(mapTags.empty())
	{
		printf("Failed to parse tags, check the format please.\r\nFormat: tagname:tagvalue;\r\nLike:\r\nitar:yes;class:private;\r\n");
		return;
	}

	TAG* pTags = new TAG[mapTags.size()];
	if(pTags)
	{
		memset(pTags, 0, sizeof(TAG) * mapTags.size());

		map<string, string>::iterator itr;
		int index = 0;
		for(itr = mapTags.begin(); itr != mapTags.end(); itr++)
		{
			strncpy_s(pTags[index].szTagName, sizeof(pTags[index].szTagName), (*itr).first.c_str(), _TRUNCATE);
			strncpy_s(pTags[index].szTagValue, sizeof(pTags[index].szTagValue), (*itr).second.c_str(), _TRUNCATE);

			index++;
		}

		PDF_Func lfDelete= (PDF_Func)GetProcAddress(g_hMod, "PDF_DeleteTags");

		if(lfDelete)
		{
			lfDelete(pszFileName, pTags, (int)mapTags.size());
		}

		delete []pTags;
		pTags = NULL;
	}

}

int _tmain(int argc, _TCHAR* argv[])
{
#ifdef _WIN64
	g_hMod = (HMODULE)LoadLibraryW(L"PDFLib.dll");
#else
	g_hMod = (HMODULE)LoadLibraryW(L"PDFLib32.dll");
#endif

	if(!g_hMod)
	{
		printf("Can't load PDFLib.dll, check if it is there(including Podofo DLLs)\r\n");
		return 0;
	}

	DWORD dwStart = GetTickCount();

	if( argc == 4)
	{
		string strFilePath = MyWideCharToMultipleByte(wstring(argv[3]));

		if(_wcsicmp(argv[2], L"-f") ==0)
		{
			PDFLib_ReadTag((char*)strFilePath.c_str());
		}

		
	}
	else if(argc == 6)
	{
		string strFilePath = MyWideCharToMultipleByte(wstring(argv[3]));

		string strTags = MyWideCharToMultipleByte(wstring(argv[5]));

		if(_wcsicmp(argv[1], L"-a") == 0)
			PDF_AddTags((char*)strFilePath.c_str(), strTags.c_str());
		else if(_wcsicmp(argv[1], L"-d") == 0)
			PDF_DeleteTags((char*)strFilePath.c_str(), strTags.c_str());
	}
	else if(argc == 2)
	{
		PDF_IsPDFFileType lfIsPDFFile = (PDF_IsPDFFileType)GetProcAddress(g_hMod, "PDF_IsPDFFile");
		if(lfIsPDFFile)
		{
			string strFilePath = MyWideCharToMultipleByte(wstring(argv[1]));
			bool bRet = lfIsPDFFile(strFilePath.c_str());
			if(bRet)
				printf("file %s is a PDF file\r\n", strFilePath.c_str());
			else
				printf("file %s is not a PDF file\r\n", strFilePath.c_str());
		}
	}
	else
	{
		help();
	}

	printf("used %d ms\r\n", GetTickCount() - dwStart);
	return 0;
}

