#pragma once
#include <atlbase.h>

#define LOCAL_IP	"127.0.0.1"
#define LOCAL_PORT	18787
class CRegEdit
{
public:
	static bool WriteServicePort(const unsigned int& nport,const HKEY hKey=HKEY_LOCAL_MACHINE,
		const wchar_t* wstrpath=L"SYSTEM\\CurrentControlSet\\Services\\TransferService",
		const wchar_t* wstrkeyname=L"ServerPort")
	{
        UNREFERENCED_PARAMETER(nport);
		CRegKey theReg;
		LONG lRet = theReg.Open(hKey,wstrpath);
		if(lRet == ERROR_SUCCESS)
		{
			//theReg.SetDWORDValue(wstrkeyname,nport);
			theReg.SetDWORDValue(wstrkeyname,LOCAL_PORT);
			if(lRet == ERROR_SUCCESS)	return true;
		}
		return false;
	}
	static bool ReadServicePort(DWORD& nport,const HKEY hKey=HKEY_LOCAL_MACHINE,
		const wchar_t* wstrpath=L"SYSTEM\\CurrentControlSet\\Services\\TransferService",
		const wchar_t* wstrkeyname=L"ServerPort")
	{
		nport = LOCAL_PORT;
		return true;

		CRegKey theReg;
		LONG lRet = theReg.Open(hKey,wstrpath);
		if(lRet == ERROR_SUCCESS)
		{
			lRet = theReg.QueryDWORDValue(wstrkeyname,nport);
			if(lRet == ERROR_SUCCESS)	return true;
		}
		return false;
	}
};
