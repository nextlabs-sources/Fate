// SPIDemo.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <string>
#include "../spi_static/include/spi.h"
#include <Windows.h>

using namespace std;


// {C9BFDE32-37A4-4048-BBB4-5DEC9F70B33D}
static const GUID MyguID = 
{ 0xc9bfde32, 0x37a4, 0x4048, { 0xbb, 0xb4, 0x5d, 0xec, 0x9f, 0x70, 0xb3, 0x39 } };

std::string MyWideCharToMultipleByte(const std::wstring & strValue)
{
	int nLen = WideCharToMultiByte(CP_ACP, 0, strValue.c_str(), strValue.length(), NULL, 0, NULL, NULL); 

	char* pBuf = new char[nLen + 1];
	if(!pBuf)
		return "";
	memset(pBuf, 0, nLen +1);
	nLen = WideCharToMultiByte(CP_ACP, 0, strValue.c_str(), strValue.length(), pBuf, nLen, NULL, NULL); 

	std::string strResult(pBuf);

	if(pBuf)
	{
		delete []pBuf;
		pBuf = NULL;
	}
	return strResult;
}


void ShowLsp(bool bDetail)
{
	DisplayLsp(2, bDetail);
}

void InstallLSP(wchar_t* pszLspName, wchar_t* pszFilePath)
{
	DWORD dwIDs[1];
	dwIDs[0] = 1001;
	string szProvider = MyWideCharToMultipleByte(wstring(pszFilePath));
	InstallNewLsp(0, MyguID, (char*)MyWideCharToMultipleByte(wstring(pszLspName)).c_str(), (char*)szProvider.c_str(), (char*)szProvider.c_str(), 1, dwIDs, TRUE, FALSE);
}

void UninstallLSP()
{
	UninstallLsp(0, MyguID);
}

void help()
{
	printf("Command:\r\n    SPITool.exe -show -d | -s: show all the providers.\r\n    SPITool.exe -del: delete our LSP.\r\n    SPITool.exe -install MyProvider c:\\httpe.dll: install the new LSP with the specified file\r\n");
}
int _tmain(int argc, _TCHAR* argv[])
{
	SPI_Init();
	if(argc == 3 && _wcsicmp(argv[1], L"-show") == 0)
	{
		if(_wcsicmp(argv[2], L"-d") == 0)
			ShowLsp(true);
		else
			ShowLsp(false);

	}
	else if(argc == 2 && _wcsicmp(argv[1], L"-del") == 0)
	{
		UninstallLSP();
	}
	else if(argc == 4 && _wcsicmp(argv[1], L"-install") == 0)
	{
		InstallLSP(argv[2], argv[3]);
	}
	else
	{
		help();
	}
	SPI_Uninit();
	return 0;
}

