// stdafx.cpp : source file that includes just the standard includes
// HookDll.pch will be the pre-compiled header
// stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"
#include "MsgHook.h"
nextlabs::recursion_control hook_control;
// TODO: reference any additional headers you need in STDAFX.H
// and not in this file
void WriteLog(const char* strFilePath,const char* strContent)
{
    //return;
	FILE* fp = NULL;
	fopen_s(&fp,strFilePath,"a+b");
	if(fp != NULL)
	{
		fwrite(strContent,1,strlen(strContent),fp);
		fwrite("\r\n",1,2,fp);
		fclose(fp);
	}
}

void WriteLog(const TCHAR* strFilePath,const TCHAR* strContent)
{
    //return;
    FILE* fp = NULL;
    _wfopen_s( &fp, (strFilePath), L"a+b" );
    if(fp != NULL)
    {
        fwrite(strContent,1,_tcslen(strContent),fp);
        fwrite("\r\n",1,2,fp);
        fclose(fp);
    }
}

bool OutPutLog(const char* strContext,const int nDebugOutPut,const char* strPath)
{
	nDebugOutPut ;
	DPA((strContext));
	//if(nDebugOutPut == 0)	
	WriteLog(strPath,strContext);
	return true;
}

bool OutPutLog(const TCHAR* strContext,const int nDebugOutPut,const TCHAR* strPath)
{
	nDebugOutPut ;
    WriteLog( strPath,strContext);
    return true;
}

bool IsExecute( const TCHAR* pMsg )
{
    //return true;
    HWND hCurrentWnd = FindWindow( L"PWFrameWrapperConsoleWindow", 0/*L"PWConsole.exe"*/ );
    return IDYES == MessageBox( hCurrentWnd, pMsg, L"Continue Execute?", MB_YESNO/*|MB_APPLMODAL */);
}

bool AllowToContinue( const TCHAR* pMsg )
{
    //return true;
    HWND hCurrentWnd = FindWindow( L"PWFrameWrapperConsoleWindow", 0/*L"PWConsole.exe"*/ );
    return IDYES == MessageBox( hCurrentWnd, pMsg, L"Allow to continue?", MB_YESNO/*|MB_APPLMODAL */);
}

#include <deque>
Mutex gMutexString;
std::deque<std::wstring> gStrQue;
std::wstring strInfoTile;

void ShowInfoFunc( void* pVoid )
{
	pVoid ;
    std::wstring strTmp;
    {
    MUTEX _mutex_(&gMutexString);
    strTmp = gStrQue.front();
    
    gStrQue.pop_front();
    }

    const TCHAR* pStr = strTmp.c_str();
    MessageBox( 0, /*(TCHAR*)pVoid*/pStr, strInfoTile.c_str(), MB_OK/*|MB_APPLMODAL */); 
}

typedef void (*ShowFunc)(void* pVoid);

bool _ShowInfo( const TCHAR* pMsg, ShowFunc pFunc )
{
    MUTEX _mutex_(&gMutexString);
    gStrQue.push_back( pMsg );
    //return true;
    //HWND hCurrentWnd = FindWindow( L"DV2ControlHost", 0);//L"IEXPLORE" );
    _beginthread( pFunc, 0, (void*)pMsg );
    return true;    
}

void _AllowShare( void* pVoid )
{
	pVoid ;
    std::wstring strTmp;
    {
        MUTEX _mutex_(&gMutexString);
        strTmp = gStrQue.front();

        gStrQue.pop_front();
    }

    const TCHAR* pStr = strTmp.c_str();
    CMsgHook::GetInstance()->SetAllowShare( IDYES == MessageBox( 0, /*(TCHAR*)pVoid*/pStr, strInfoTile.c_str(), MB_YESNO/*|MB_APPLMODAL */) );
}

void AllowShare()
{
    strInfoTile = TEXT( "Allow to Share the Following?" ); 
    _ShowInfo( TEXT("Whiteboard, Text Page, Poll Page, Web Page ?"), _AllowShare );
}

void _AllowChat( void* pVoid )
{
	pVoid;
    std::wstring strTmp;
    {
        MUTEX _mutex_(&gMutexString);
        strTmp = gStrQue.front();

        gStrQue.pop_front();
    }

    const TCHAR* pStr = strTmp.c_str();
    CMsgHook::GetInstance()->SetAllowChat( IDYES == MessageBox( 0, /*(TCHAR*)pVoid*/pStr, strInfoTile.c_str(), MB_YESNO/*|MB_APPLMODAL */) ); 
}

void AllowChat()
{
    strInfoTile = TEXT( "Allow to Chat?" ); 
    _ShowInfo( TEXT(""), _AllowChat );
}


bool ShowInfo( const TCHAR* pMsg )
{
    //OutPutLog( pMsg );
    
    return true;
    strInfoTile = TEXT( "Information" );
    return _ShowInfo( pMsg, ShowInfoFunc );    
}



bool DemoShowInfo(  const TCHAR* pMsg, const TCHAR* pInfoBox )
{
    strInfoTile = pInfoBox;
    return _ShowInfo( pMsg, ShowInfoFunc );    
}

inline HWND GetWindowHwd()
{
    static HWND ahWnd = 0;

    if( ahWnd == 0 )
    {
        ahWnd = FindWindow( L"PWFrameWrapperConsoleWindow", 0 );
    }
    return ahWnd;
}

#include "Policy.h"
bool MsgBoxAllowOrDeny( const TCHAR* pMsg, const TCHAR* pMsgBoxInfo, const TCHAR* pstrAttachment,wchar_t* action )
{
	return CPolicy::CreateInstance()->QueryLiveMeetingPolicy( pstrAttachment,action );
    return IDYES == MessageBox( GetWindowHwd(), pMsg, pMsgBoxInfo, MB_YESNO/*|MB_APPLMODAL */);
}

bool DoEvaluate( const TCHAR* pstrRes, wchar_t*  action )
{
	DPW((L"Evaluation Information:Srouce:[%s]\r\n,Action:[%s]", pstrRes, action)) ;
	return CPolicy::CreateInstance()->QueryLiveMeetingPolicy( pstrRes,action );
}
bool DoAppEvaluate( const TCHAR* pMagic,const TCHAR* pstrRes, wchar_t*  action,const wchar_t* pszDest )
{
	DP((L"Magic: %s\r\n PstrRes:[%s]\r\n action:[%s]\r\n",pMagic, pstrRes, action )) ;
	if( pszDest!= NULL )
	{
		DP((L"Destination path:[%s]\r\n",pszDest )) ;
	}
	return	CPolicy::CreateInstance()->QueryLiveMeetingPolicyWithAppProp( pMagic,pstrRes, action,0 ) ;
}

static TCHAR strProcessName[1024] = {0};
BOOL CALLBACK EnumWindowsProc(          HWND hwnd,
                              LPARAM lParam
                              )
{
    DWORD dwProcessID = 0;

    GetWindowThreadProcessId( hwnd, &dwProcessID );

    if( dwProcessID == (DWORD)(lParam) )
    {
        GetWindowText( hwnd, strProcessName, 1024 );
        return false;
    }
    return true;
}

const TCHAR* GetNameWithoutPath( const TCHAR* pstrPath, DWORD nLen )
{
    if( !nLen || !pstrPath )
    {
        return 0;
    }

    const TCHAR* pEnd = pstrPath + nLen;

    while( pEnd >= pstrPath && *pEnd != TEXT( '\\' ) )
    {
        --pEnd;
    }
    if( *pEnd == TEXT('\\') )
    {
        ++pEnd;
    }

    return pEnd;
}

const TCHAR* GetProcessName( UINT32 nProcessId )
{
    //EnumWindows(
    //    EnumWindowsProc, // callback function
    //    (LPARAM)nProcessId // application-defined value
    //    );
    //DWORD nStrLen = wcslen(strProcessName);
    //if( !nStrLen )
    {
		HANDLE hProcess = OpenProcess(
			PROCESS_QUERY_INFORMATION,
			FALSE, nProcessId );

		DWORD nStrLen =0;
		nStrLen = GetProcessImageFileName ( hProcess, strProcessName, 1024 );
        //nStrLen = GetModuleBaseName( hProcess, 0, strProcessName, 1024 );
        //nStrLen = 1024;
        //QueryFullProcessImageName( hProcess, 0, strProcessName, &nStrLen );
        //nStrLen = GetProcessImageFileName( hProcess, strProcessName, 1024 );
        //return GetNameWithoutPath( strProcessName, nStrLen );
    }
    return strProcessName;
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
	if( *filename == 0 )//only call GetModuleFileNameW at the first time. Kevin 20089-8-20
	{
		GetModuleFileNameW(hMod, filename, 1024);
	}

	if( *filename != 0 )
	{
		memcpy(lpszProcessName, filename, nLen * sizeof(wchar_t));
		return TRUE;
	}

	return FALSE;
}
BOOL IsProcess(LPCWSTR lpProcessName)
{
	if(!lpProcessName)
	{
		return FALSE;
	}

	wchar_t filename[1024] = {0};
	GetModuleName(filename, 1023, NULL);

	wchar_t* p = wcsrchr(filename, '\\');

	if(p && *(p + 1) != '\0')
	{
		if(_wcsicmp(p + 1, lpProcessName) == 0)
			return TRUE;
	}

	return FALSE;

}
