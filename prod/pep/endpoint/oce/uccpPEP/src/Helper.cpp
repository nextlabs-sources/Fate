#include "stdafx.h"
#include "Helper.h"


#include "celog_policy_windbg.hpp"
#include "celog_policy_file.hpp"
CELog g_log;

// return value doesn't include '\'
// input c:\abc\1.exe
// output c:\abc
bool PathName2Path(char* szPathName,char Path[MAX_PATH])
{
	memset(Path,0,MAX_PATH);

	size_t Length = strlen(szPathName);
	if(!Length) return false;		

	for(size_t i=0;i<Length;++i)
	{
		if(szPathName[Length-1-i] == '\\')
		{
			memcpy(Path,szPathName,Length-i-1);
			return true;
		}
	}
	return false;
}

bool PathName2Path(wchar_t* wszPathName,wchar_t Path[MAX_PATH])
{
	memset(Path,0,MAX_PATH*sizeof(wchar_t));

	size_t Length = wcslen(wszPathName);
	if(!Length) return false;	

	for(size_t i=0;i<Length;++i)
	{
		if(wszPathName[Length-1-i] == '\\')
		{
			memcpy(Path,wszPathName,(Length-i-1)*sizeof(wchar_t));
			return true;
		}
	}
	return false;
}

void GetSessionTypeString(WCHAR SessionType[256],enum UCC_SESSION_TYPE enSessionType)
{
	switch(enSessionType)
	{
	case UCCST_INSTANT_MESSAGING:
		wcscpy_s(SessionType,256,L"UCCST_INSTANT_MESSAGING");
		break;
	case UCCST_AUDIO_VIDEO:
		wcscpy_s(SessionType,256,L"UCCST_AUDIO_VIDEO");
		break;
	case UCCST_CONFERENCE:
		wcscpy_s(SessionType,256,L"UCCST_CONFERENCE");
		break;
	case UCCST_APPLICATION:
		wcscpy_s(SessionType,256,L"UCCST_APPLICATION");
		break;
	case UCCST_APPLICATION_SHARING:
		wcscpy_s(SessionType,256,L"UCCST_APPLICATION_SHARING");
		break;
	default:
		wcscpy_s(SessionType,256,L"UNKNOWN");
		break;
	}
	return;
}




BOOL GetModuleName(LPWSTR lpszProcessName, int nLen, HMODULE hMod)

{

	if(!lpszProcessName || nLen > 1024)

		return FALSE;



	if(hMod)

	{

		DWORD dwCount = GetModuleFileNameW(hMod, lpszProcessName, nLen);

		return dwCount > 0 ? TRUE: FALSE;

	}



	static wchar_t filename[1025] = {0};

	if( wcslen(filename) == 0)//only call GetModuleFileNameW at the first time. Kevin 20089-8-20

	{

		GetModuleFileNameW(hMod, filename, 1024);

	}



	if(wcslen(filename) > 0)

	{

		memcpy(lpszProcessName, filename, nLen * sizeof(wchar_t));

		return TRUE;

	}



	return FALSE;

}


std::string MyWideCharToMultipleByte(const std::wstring strValue)

{

	int nLen = WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK, strValue.c_str(), static_cast<int>(strValue.length()), NULL, 0, NULL, NULL); 



	char* pBuf = new char[nLen + 1];

	if(!pBuf)

		return "";

	memset(pBuf, 0, nLen +1);

	nLen = WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK, strValue.c_str(),  static_cast<int>(strValue.length()), pBuf, nLen, NULL, NULL); 



	std::string strResult(pBuf);



	if(pBuf)

	{

		delete []pBuf;

		pBuf = NULL;

	}

	return strResult;

}


BOOL InitLog()

{

	// If debug mode is enabled write to log file as well

	if( NLConfig::IsDebugMode() == true )

	{

		/* Generate a path using the image name of the current process.  Set log policy for DebugView

		* and file on log instance.  Path will be [NextLabs]/.. Enforcer/diags/logs/.

		*/

		wchar_t image_name[MAX_PATH * 2 + 1] = {0};



		if( !GetModuleName(image_name, MAX_PATH * 2, NULL) )

		{

			return FALSE;

		}

		wchar_t* image_name_ptr = wcsrchr(image_name,'\\'); // get just image name w/o full path

		if( image_name_ptr == NULL )

		{

			image_name_ptr = image_name;                   // when failed use default image name

		}

		else

		{

			image_name_ptr++;                              // move past '\\'

		}



		wchar_t oce_install_path[MAX_PATH] = {0};

		if( NLConfig::GetComponentInstallPath(L"Enterprise DLP\\Office Communicator Enforcer",oce_install_path,_countof(oce_install_path)) == true )

		{

			wchar_t log_file[MAX_PATH * 3 + 1] = {0};

			swprintf_s(log_file,_countof(log_file),L"%s\\diags\\logs\\%s.txt",oce_install_path,image_name_ptr);

			std::string strLogPath = MyWideCharToMultipleByte(log_file);

			g_log.SetPolicy( new CELogPolicy_File(strLogPath.c_str()) );

		}


		g_log.SetPolicy( new CELogPolicy_WinDbg() ); // output to DebugView

		g_log.Enable();                              // enable log

		g_log.SetLevel(CELOG_DEBUG);                 // log threshold to debug level

	}

	return TRUE;

}

