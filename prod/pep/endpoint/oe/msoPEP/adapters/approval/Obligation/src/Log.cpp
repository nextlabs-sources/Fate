#include <stdafx.h>
#include "log.h"
#include <string>
const wchar_t* wtrReturn =L"\r\n";
bool CLog::WriteLog(const wchar_t* wstrName ,const wchar_t* wstrValue)
{
	WCHAR wzModuleName[MAX_PATH+1];memset(wzModuleName,0,sizeof(wzModuleName));
	DWORD dwRet=::GetModuleFileName(NULL,wzModuleName,MAX_PATH);
	std::wstring strLogFile;
	if(dwRet==0)
	{
		strLogFile = L".\\co.log";
	}
	else
	{
		strLogFile=wzModuleName;
		std::wstring::size_type pos=strLogFile.rfind(L"\\");
		if(pos!=std::wstring::npos)
		{
			std::wstring strTemp=strLogFile.substr(0,pos);
			strLogFile=strTemp;
			strLogFile+=L"\\co.log";
		}
		else
		{
			strLogFile+=L"\\co.log";
		}
	}

	
	FILE* fp=NULL;
	errno_t err = _wfopen_s(&fp,strLogFile.c_str(),L"a+b");
	if(err ==0 && fp != NULL)
	{
		std::wstring strLine = wstrName;
		strLine += L" is:>>>>>>>>>>>>>>>>>>>>>>>>\t";
		strLine += wstrValue;
		strLine += L"\r\n";
		fwrite(strLine.c_str(),sizeof(wchar_t),strLine.length(),fp);
		fclose(fp);
	}
	return true;
}