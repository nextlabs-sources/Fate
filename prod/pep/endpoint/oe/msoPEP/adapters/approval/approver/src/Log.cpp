#include <stdafx.h>
#include "log.h"
#include <string>
#include <io.h>
const wchar_t* wtrReturn =L"\r\n";
extern HINSTANCE   g_hInstance;
bool CLog::WriteLog(const wchar_t* wstrName ,const wchar_t* wstrValue)
{
	
	WCHAR wzModuleName[MAX_PATH+1];memset(wzModuleName,0,sizeof(wzModuleName));
	DWORD dwRet=::GetModuleFileName(g_hInstance,wzModuleName,MAX_PATH);
	std::wstring strLogFile;
	if(dwRet==0)
	{
		strLogFile = L".\\ce_approver.log";
	}
	else
	{
		strLogFile=wzModuleName;
		std::wstring::size_type pos=strLogFile.rfind(L"\\");
		if(pos!=std::wstring::npos)
		{
			std::wstring strTemp=strLogFile.substr(0,pos);
			strLogFile=strTemp;
			strLogFile+=L"\\ce_approver.log";
		}
		else
		{
			strLogFile+=L"\\ce_approver.log";
		}
	}
	FILE* fp=NULL;
	errno_t err = _wfopen_s(&fp,strLogFile.c_str(),L"a+b");

	if(err ==0 && fp != NULL)
	{
		fseek(fp,0,SEEK_END);
		long size=ftell(fp);
		if(size>5*1024*1024)
		{
			fclose(fp);
			fp=NULL;
			err=_wfopen_s(&fp,strLogFile.c_str(),L"w+b");
			if(!(err==0&&fp!=NULL))
				return true;
		}
		std::wstring strLine = wstrName;
		if(wcslen(wstrValue))
		{
			strLine += L" is:>>>>>\t";
			strLine += wstrValue;
		}
		strLine += L"\r\n";
		fwrite(strLine.c_str(),sizeof(wchar_t),strLine.length(),fp);
		fclose(fp);
	}
	return true;
}