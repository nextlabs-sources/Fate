#include "stdafx.h"
#include "Auxiliary.h"
#include <tlhelp32.h>

emProcessType CAuxiliary::m_emProcType = UNKNOWN_TYPE;
em0penType    CAuxiliary::m_emOpenType = UNKNOW_OPEN_TYPE;
int			  CAuxiliary::m_version	   = 0;


CAuxiliary::CAuxiliary()
{
}
CAuxiliary::~CAuxiliary()
{

}

Color CAuxiliary::ChooseFontColor(DWORD dwColor,long lTransParent)
{
	BYTE lR = static_cast<BYTE>((dwColor & 0xff0000) >> 16);
	BYTE lG = static_cast<BYTE>((dwColor & 0x00ff00) >> 8);
	BYTE lB = static_cast<BYTE>(dwColor & 0x0000ff);

	if(lTransParent < 0)	lTransParent = 0;
	else if (lTransParent > 100)	lTransParent = 255;
	else	lTransParent = lTransParent * 255 / 100;

	lTransParent = 255 - lTransParent;

	return Color((BYTE)lTransParent,lR, lG, lB);
}

void CAuxiliary::GetUserHostHome(__in const wstring & strText,__out TextInfo& theTextInfo)
{
	wstring::size_type pos = strText.find(L"\\n");
	if(pos == wstring::npos)
	{
		theTextInfo.strUser = strText;
		theTextInfo.strHostHome = L"";		
	}
	else
	{
		theTextInfo.strUser = strText.substr(0,pos);
		theTextInfo.strHostHome = strText.substr(pos + 2);		
	}
}
bool CAuxiliary::IsRepeat(__in const wstring &strPlacement)
{
	if(_wcsicmp(REPEAT_SHOW_TEXT,strPlacement.c_str()) != 0)
		return false;
	else
		return true;
}

//	Process related function
emProcessType CAuxiliary::GetProgressType()
{
	if(CAuxiliary::m_emProcType != UNKNOWN_TYPE)	return m_emProcType;
	
	HMODULE  hModel = ::GetModuleHandle(NULL);
	wchar_t strModeName[MAX_PATH] = {0};
	if(hModel != NULL)
	{
		::GetModuleFileName(hModel,strModeName,MAX_PATH);
	}
	else
	{
		return CAuxiliary::m_emProcType;
	}

	wstring strModeExEName = strModeName;
	if(!strModeExEName.empty())
	{
		wstring strExeName = GetFileName(strModeExEName);
		if(_wcsicmp(strExeName.c_str(),L"WINWORD") == 0) 
		{
			CAuxiliary::m_emProcType = WORD_TYPE;
		}
		else if(_wcsicmp(strExeName.c_str(),L"POWERPNT") == 0) 
		{
			CAuxiliary::m_emProcType = PPT_TYPE;
		}
		else if(_wcsicmp(strExeName.c_str(),L"EXCEL") == 0) 
		{
			CAuxiliary::m_emProcType = EXCEL_TYPE;
		}
		else if(_wcsicmp(strExeName.c_str(),L"AcroRd32") == 0 || _wcsicmp(strExeName.c_str(),L"Acrobat") == 0)
		{
			CAuxiliary::m_emProcType =  ADOBE_TYPE;
		}
		else if(_wcsicmp(strExeName.c_str(),L"iexplore") == 0 )
		{
			CAuxiliary::m_emProcType =  ADOBE_ADDIN_TYPE;
		}

	}

	return CAuxiliary::m_emProcType;
}

int CAuxiliary::GetAppVer()
{
	if(CAuxiliary::m_version != 0)	return m_version;

	HMODULE  hModel = ::GetModuleHandle(NULL);
	wchar_t strModeName[MAX_PATH] = {0};
	if(hModel != NULL)
	{
		::GetModuleFileName(hModel,strModeName,MAX_PATH);
	}
	else
	{
		OutputDebugString(L"Get Model Handle fail!\n");
	}

	wstring strModule = strModeName;
	std::wstring::size_type pos = strModule.rfind('\\');
	if(pos != std::wstring::npos)
	{
		strModule = strModule.substr(0,pos);
	}

	if(boost::algorithm::iends_with(strModule,L"Office14"))
		CAuxiliary::m_version = 2010;
	else if(boost::algorithm::iends_with(strModule,L"Office12"))
		CAuxiliary::m_version = 2007;
	else
		CAuxiliary::m_version = 2003;
	
	return CAuxiliary::m_version;
}


//	Process related function
em0penType CAuxiliary::GetOpenType(HWND hForeWnd)
{
	if(CAuxiliary::m_emOpenType != UNKNOW_OPEN_TYPE)	return m_emOpenType;

	if (hForeWnd != NULL)
	{
		WCHAR strClsName[MAX_PATH] = {0};
		GetClassName(hForeWnd,strClsName,MAX_PATH);
		if (_wcsicmp(strClsName,L"IEFrame") == 0)
		{
			CAuxiliary::m_emOpenType = IE_OPNE_TYPE; 
		}
		else
		{
			CAuxiliary::m_emOpenType = APP_OPEN_TYPE; 
		}
	}
	return CAuxiliary::m_emOpenType;
}


wstring CAuxiliary::GetFileName(const wstring& str)
{
	if(str.empty())
	{
		return str;
	}
	std::wstring::size_type pos, len, dotpos;
	char cslash = '\\';
	pos = str.find('/');
	if(pos != std::wstring::npos)
	{
		cslash = '/';
	}
	pos = str.rfind(cslash);
	if(pos == std::wstring::npos)
	{
		len = str.length();
		dotpos = str.rfind('.');
		if(dotpos == std::wstring::npos)
		{
			return str;
		}
		return str.substr(0,dotpos);
	}
	len = str.length();
	dotpos = str.rfind('.');
	if((dotpos == std::wstring::npos)||(dotpos < pos))
	{
		return str.substr(pos + 1,len - pos);
	}
	return str.substr(pos + 1,dotpos - pos - 1);
}

void CAuxiliary::DisableAeroPeek(bool bDisable)
{
	// check if HKEY_CURRENT_USER\Software\Microsoft\Windows\DWM\EnableAeroPeek value,
	// if value is 1 reset to 0, otherwise, do nothing.
	HKEY hCurKey = NULL,hSubKey = NULL;
	LONG lRet = RegOpenCurrentUser(KEY_SET_VALUE,&hCurKey);
	if(lRet == ERROR_SUCCESS)	
	{
		static const wchar_t* strKeyPath = L"Software\\Microsoft\\Windows\\DWM";
		lRet = RegOpenKey(hCurKey,strKeyPath,&hSubKey);
		if(lRet == ERROR_SUCCESS)
		{
			DWORD dwData = 0;
			DWORD dwSize = sizeof(DWORD);
			lRet = RegQueryValueEx(hSubKey, L"EnableAeroPeek", NULL,NULL,(LPBYTE)&dwData,&dwSize);
			if(bDisable)
			{
				if(lRet != ERROR_SUCCESS || dwData != 0)
				{
					dwData = 0;
					if(NULL == FindWindow(L"LivePreview",NULL))
						lRet = RegSetValueExW(hSubKey,L"EnableAeroPeek",0,REG_DWORD,(BYTE*)&dwData,sizeof(dwData));
				}
			}
			else
			{
				if(lRet != ERROR_SUCCESS || dwData != 1)
				{
					dwData = 1;
					if(NULL == FindWindow(L"LivePreview",NULL))
						lRet = RegSetValueExW(hSubKey,L"EnableAeroPeek",0,REG_DWORD,(BYTE*)&dwData,sizeof(dwData));
				}
			}

			RegCloseKey(hSubKey);
		}
		RegCloseKey(hCurKey);
	}
}
/*
**	extract file name from caption content.
**	now this function only for protected view case : PPTFrameClass and screenClass.
*/
void CAuxiliary::GetFileNameFromCaption(__inout wstring& str)
{
	if(boost::algorithm::istarts_with(str,L"PowerPoint Slide Show - ")||
	   boost::algorithm::istarts_with(str,L"PowerPoint Presenter View - "))
	{
		boost::algorithm::replace_all(str,L"PowerPoint Slide Show - ",L"");
		boost::algorithm::replace_all(str,L"PowerPoint Presenter View - ",L"");
		boost::algorithm::replace_all(str,L" [Protected View]",L"");
		str = str.substr(1,str.length()-1);
	}
	else
	{
		size_t nPose = str.rfind(L"-");
		if(nPose != std::wstring::npos)
		{
			str = str.substr(0,nPose-1);
			boost::algorithm::replace_all(str,L" [Protected View]",L"");
		}		
	}

	size_t dotPos = str.rfind(L".");
	if(dotPos != std::wstring::npos)
	{
		str = str.substr(0,dotPos);
	}
}


DWORD CAuxiliary::GetParentProcessID(DWORD dwProcessID)
{
	DWORD dwParentID = 0;
	HANDLE hProcessSnap = NULL;
	PROCESSENTRY32 pe32;

	// Take a snapshot of all processes in the system.
	hProcessSnap = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );
	if( hProcessSnap == INVALID_HANDLE_VALUE )
	{
		return( FALSE );
	}

	// Set the size of the structure before using it.
	pe32.dwSize = sizeof( PROCESSENTRY32 );

	// Retrieve information about the first process,
	// and exit if unsuccessful
	if( !Process32First( hProcessSnap, &pe32 ) )
	{
		CloseHandle( hProcessSnap );    // Must clean up the
		//   snapshot object!
		return( FALSE );
	}

	do
	{
		if(dwProcessID == pe32.th32ProcessID )
		{
			dwParentID = pe32.th32ParentProcessID;
		}
	} while( Process32Next( hProcessSnap, &pe32 ) );

	CloseHandle( hProcessSnap );
	return dwParentID;
}