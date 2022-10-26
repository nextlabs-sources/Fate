#include "stdafx.h"
#include "MsgHook.h"
#include "PartDB.h"
#include "Policy.h"
#include <algorithm>


INSTANCE_DEFINE( CHookedCreateWindow );
#ifndef HDM_FIRST
#define HDM_FIRST               0x1200 
#endif
#ifndef HDM_GETITEMW
#define HDM_GETITEMW            (HDM_FIRST + 11)
#endif
#ifndef TTM_GETTOOLINFOW
#define TTM_GETTOOLINFOW        (WM_USER + 53)
#endif
CRITICAL_SECTION CMsgHook::s_csMsgHook ;

void CHookedCreateWindow::Hook( void* pCreateWindow )
{
	pCreateWindow ;
    //gDetourTransactionBegin();
   // gDetourUpdateThread(GetCurrentThread());
    //gDetourAttach(&(PVOID&)Old_CreateWindowExW, Hooked_CreateWindowExW);
    //gDetourAttach(&(PVOID&)Old_ShowWindow, Hooked_ShowWindow);
    //gDetourAttach(&(PVOID&)Old_RegisterClassW, Hooked_RegisterClassW);
    //gDetourAttach(&(PVOID&)Old_RegisterClassExW, Hooked_RegisterClassExW);
   // gDetourTransactionCommit();
}
BOOL CALLBACK EnumChildProc(HWND hwndChild, LPARAM lParam) 
{ 
	lParam ;
	/*wchar_t szBuf[MAX_PATH] = {0}  ;
	::GetWindowTextW( hwndChild, szBuf,MAX_PATH ) ;
	::DPW(( szBuf )) ;
	if( _wcsnicmp( szBuf,L"Chat", wcslen( L"Chat") ) != 0 )
	{
		::DPW(( L"Exist ")) ;
		return TRUE ;
	}*/
    wchar_t szClassBuf[MAX_PATH] = {0}  ;
	GetClassNameW( hwndChild, szClassBuf,MAX_PATH ) ;
	::DPW(( szClassBuf ) );
	if( wcslen( L"WTL_CrossFadeButton") != wcslen(szClassBuf )  || _wcsnicmp( szClassBuf,L"WTL_CrossFadeButton", wcslen(szClassBuf )) != 0 )
	{
		::DPW(( L"Exit two" )) ;
		return TRUE ;
	}
	CMsgHook::m_IsQAChat =  TRUE ;
    return FALSE;
}
INSTANCE_DEFINE( CMsgHook );

const int ID_SHARE_WHITEBOARD  = 22069;
const int ID_SHARE_POLLPAGE    = 22072;
const int ID_SHARE_TEXTPAGE    = 22071;
const int ID_SHARE_WEBPAGE     = 22070;
const int ID_SHARE_REMOTEDESTOP = 22100;
const int ID_SHARE_UPLOADFILE  = 26066;
const int ID_RAISE_YOUR_HAND   = 27043;
const int ID_INVITE            = 26314;
const int ID_ASK               = 24112;
const int ID_ASK1              = 24114;
const int ID_ASK3              = 24017;
const int ID_SHARED_NOTEs      = 1312;
const int ID_SHARE_VIDEO        = 32018;
const int ID_SHARE_VIDEO1       = 32082;
const int ID_SHARE_VIDEO2        = 1000;
const int ID_HANDOUTS           = 1427;
const int ID_MUTE_AUDIO      = 26309;
const int ID_MUTE_AUDIO1      = 26300;
const int ID_MUTE_AUDIO2      = 100;
//const int ID_MICROPHONE_AUDIO3      = 32016;
const int ID_COMMAND_AUDIO1      = 32011;
//const int ID_COMMAND_AUDIO2      = 32012;
const int ID_RECORD_1          = 0x567;
const int ID_RECORD_2          = 0x607; 
//const int ID_CHAT               = 26301;
//const int ID_CHAT1              = 21003;//21002
//const int ID_CHAT2              = 21002;
const int ID_CHAT3 = 0x1ed;
const int ID_CHAT4 = 0x5DCE	;
const int ID_CHAT5 = 0x5209	;
const int ID_CHAT6 = 0x5DD0	;

//const int ID_TEXT               = WM_USER + 7246;
//const int ID_TEXT              = WM_USER + 3111;
//const int ID_TEXT              = WM_USER + 3112;
//const int ID_TEXT              = WM_USER + 3076;
//const int ID_TEXT              = WM_USER + 3086; ;
//const int ID_TEXT              = HDM_FIRST+11;
//const int ID_TEXT              = WM_USER+3103;
//const int ID_TEXT              = WM_USER+3167;
//const int ID_TEXT              = WM_USER+3147; //****************
//const int ID_TEXT =  WM_USER+3127;
//const int ID_TEXT =  WM_USER+3187;
//const int ID_TEXT =  WM_USER+3116;
//const int ID_TEXT =  WM_USER+3103;
//const int ID_TEXT =  WM_USER+315;
//const int ID_TEXT =  0;//0x1ef;//HDM_GETITEMW;
//const int ID_TEXT =  WM_USER+3090;
//const int ID_TEXT =  0xc1ec;//WM_USER+3090;
//const int ID_TEXT =  WM_USER+3148;
//const int ID_TEXT =  WM_USER+3149;
//const int ID_TEXT              = WM_USER+5;
//const int ID_TEXT              = WM_USER+7;
//const int ID_TEXT              = WM_USER+21;
//const int ID_TEXT              = WM_USER+8;
//const int ID_TEXT              = WM_USER+7696;
//const int ID_TEXT              = WM_SHOWWINDOW; 
const int ID_TEXT              = WM_SIZE; 
//const int ID_TEXT              = WM_CREATE; 
//const int ID_TEXT = 0;//WM_WINDOWPOSCHANGING;
//const int ID_TEXT = WM_CREATE;
//const int ID_TEXT              = LB_GETITEMDATA;
//const int ID_TEXT              = WM_NOTIFY;
//const int ID_TEXT = WM_USER+31867;
//const int ID_TEXT = WM_USER+31843;

static bool bShareNotesFirstTime = TRUE;
static bool bShareNotesOpen = false;
std::list<THREADHOOKLINK> CMsgHook::s_pThreadHookLink;
BOOL CMsgHook::m_IsQAChat = FALSE ;

BOOL  ImplementQAChatWindow( HWND hwnd )
{
	BOOL bRet = FALSE;
	if( IsWindow(hwnd) == TRUE )
	{
		HWND hparent = hwnd ;
		//hparent =::GetParent(hparent);
		HWND hChild = NULL;
		wchar_t szBuf[MAX_PATH] = {0} ;
		::GetWindowTextW( hparent,szBuf, MAX_PATH ) ;
		DPW((L"Window Text :[%s]",szBuf)) ;
		::GetClassNameW( hparent,szBuf, MAX_PATH ) ;
		DPW((L"Window Class name :[%s]",szBuf)) ;
		hChild = ::FindWindowExW(hparent,hChild,  L"WTL_CrossFadeButton", NULL ) ;
		while(hChild != NULL )
		{
			if( hChild != NULL )
			{
				CoInitialize(NULL) ;
				CComPtr<IAccessible> pIAcc = NULL ;

				if ( SUCCEEDED( AccessibleObjectFromWindow( hChild,OBJID_WINDOW , IID_IAccessible,(void**)&pIAcc ) ) )
				{
					if( pIAcc!=NULL )
					{
						std::wstring sRet ;
						BSTR bStr = NULL ;
						VARIANT VT ;
						VariantInit( &VT ) ;
						VT.vt = VT_I4;
						VT.lVal = CHILDID_SELF ;
						if ( !FAILED(pIAcc->get_accName( VT, &bStr ) ) )
						{
							if ( bStr != NULL )
							{
								sRet = bStr ;
								if( sRet.length()>=5 )
								{
									std::wstring temp = L"Reply to All" ;
									if( _wcsnicmp( sRet.c_str(),temp.c_str(),temp.length() ) == 0 )
									{
										if( !DoEvaluate( LME_MAGIC_STRING,L"QUESTION" ) )//!MsgBoxAllowOrDeny( CPartDB::GetInstance()->GetPresenterAttendeeInfo().c_str(), L"Allow to Raise Your Hand?" )) 
										{
											bRet = TRUE;
										}
										break ;
									}
									else
									{
										if(_wcsnicmp( sRet.c_str(),L"Reply",temp.length() ) == 0 )
										{
											if( !DoEvaluate(LME_MAGIC_STRING,L"IM") )
											{     
												bRet = TRUE;
											}
											break ;
										}
									}
								}
							}
							::SysFreeString( bStr ) ;
						}

					}
				}
				CoUninitialize() ;
			}
			hChild = ::FindWindowExW(hparent,hChild,  L"WTL_CrossFadeButton", NULL ) ;
		}
	}
	return bRet ;
}
BOOL ImplementChatWindow( HWND hwnd )
{
	BOOL bRet = TRUE ;
	if( IsWindow(hwnd) == TRUE )
	{
		HWND hparent = hwnd ;
		wchar_t szWndName[MAX_PATH]= {0} ;
		std::wstring title  ;
		OutputDebugStringW( L"==================step4========================" ) ;

		GetWindowTextW(hparent,szWndName,MAX_PATH ) ;
		title = szWndName ;
		std::transform(  title.begin(), title.end(), title.begin(), toupper    ) ; 
		OutputDebugStringW(title.c_str() ) ;
		OutputDebugStringW( L"==================step5========================" ) ;
		if( title.find(L"CHAT" ) !=std::string.npos )
		{
			if( wcslen( szWndName)!= 0 )
			{
				std::wstring::size_type pos = title.rfind( L" - ") ;
				if( pos != std::string.npos )
				{
					std::wstring receiptName = title.substr(0,pos) ;
					OutputDebugStringW( L"==========================================" ) ;
					OutputDebugStringW( receiptName.c_str() ) ;
					receiptName = CPartDB::GetInstance()->GetPresenterInfoByName(receiptName) ;
					OutputDebugStringW( receiptName.c_str() ) ;
					STRINGLIST recipients ;
					recipients.push_back(receiptName) ;
					bRet =	CPolicy::CreateInstance()->QueryLiveMeetingPolicy( LME_MAGIC_STRING,L"IM", &recipients );
				}
			}
		}
		else
		{
			ZeroMemory( szWndName, MAX_PATH*sizeof(wchar_t) ) ;
		}
	}
	return bRet ;
}

LRESULT CALLBACK CMsgHook::WindowProc(          HWND hwnd,
                                      UINT uMsg,
                                      WPARAM wParam,
                                      LPARAM lParam
                                      )
{
//    bool bContinue = false;

    static bool bShared = false;

    LONG nID = GetWindowLong(hwnd, GWL_ID );
    TCHAR strClassName[64] = {0};

	
    if( ID_TEXT == uMsg  )//|| WM_SETFOCUS == uMsg )
    {
        if( 0 )// bShared )
        {
            bShared = !bShared;
        }
        else
        {
            static bool bAsk = true;

            //static DWORD dwTick = GetTickCount();

            if( 1 )//bAsk )
            {
                GetClassName(hwnd, strClassName, 64 );

               // extern CEvaluator gEva; 
              //  CCEResult aCEResult;

                if( //wcscmp(strClassName, L"WTLCasement") == 0 || 
                    //wcscmp( strClassName, L"WTLProjection" ) == 0 ||
                    wcscmp( strClassName, L"PWWebSlide" ) == 0 ||
                    wcscmp( strClassName, L"AtlAxWin80" ) == 0 ||
                    wcscmp( strClassName, L"Shell Embedding" ) == 0 ||
                    wcscmp( strClassName, L"Shell DocObject View" ) == 0 ||
                    wcscmp( strClassName, L"Internet Explorer_Server" ) == 0 )
                {
					if(  !DoAppEvaluate( LME_MAGIC_STRING ,L"WHITEBOARD",L"SHARE") ) 
                    {
                        MoveWindow( hwnd, 0, 0, 0, 0, FALSE );
                    }
                }
                else if( wcscmp( strClassName, L"PWAnnoViewer" ) == 0 )
                {
					if(  !DoAppEvaluate( LME_MAGIC_STRING ,L"WHITEBOARD",L"SHARE") ) 
                    {
                        MoveWindow( hwnd, 0, 0, 0, 0, FALSE );
                    }
                }
                
                else if( wcscmp( strClassName, L"PWPollSlide" ) == 0 )
                {
					if(  !DoAppEvaluate( LME_MAGIC_STRING ,L"POLL",L"SHARE") ) 
                    {
                        MoveWindow( hwnd, 0, 0, 0, 0, FALSE );
                    }
                }            
                else if( wcscmp( strClassName, L"PWTextSlide" ) == 0 || ( nID == 0xffffffff && wcscmp( strClassName, L"WTLEdit" ) == 0 ) )
                {  
					if(  !DoAppEvaluate( LME_MAGIC_STRING ,L"TEXT",L"SHARE") ) 
                    {
                        MoveWindow( hwnd, 0, 0, 0, 0, FALSE );
                    }
                  
                }
                else if( wcscmp( strClassName, L"WTL_OwnerDrawScrollBar" ) == 0 )
                {
                    TCHAR strClassName2[64] = {0};

                    GetClassName( GetParent(GetParent(GetParent(hwnd))), strClassName2, 64 );
                    if( wcscmp( strClassName2, L"WTLProjection" ) == 0 )
                    {
                    
						if(  !DoAppEvaluate( LME_MAGIC_STRING ,L"TEXT",L"SHARE") ) 
                        {
                            MoveWindow( hwnd, 0, 0, 0, 0, FALSE );
                        }
                       
                    }
                }
            }
        }
    }
	

    if( nID == 0x592 ) //Handle Share Notes
    {
        if( uMsg == WM_LBUTTONUP )
        {
            static bool bOpen = false;

            if( !bOpen )
            {
				if(  !DoAppEvaluate( LME_MAGIC_STRING ,L"NOTES",L"SHARE") ) 
                {   
                    if( WNDPROC pOrgFunc = GetInstance()->GetOrgProcFunc(hwnd) )
                    {      
                        lParam = 0xfff0fff;   
                        pOrgFunc( hwnd, WM_MOUSEMOVE, wParam, lParam );                        
                    }
                    //return TRUE;
                }
                else
                {
                    bOpen = !bOpen;
                    if( WNDPROC pOrgFunc = GetInstance()->GetOrgProcFunc(hwnd) )
                    {
                        pOrgFunc( hwnd, WM_LBUTTONDOWN, wParam, lParam );
                        return pOrgFunc( hwnd, uMsg, wParam, lParam );
                    }
                }
            }
            else
            {
                bOpen = !bOpen;
            }
        }
    }

    if( nID == 0x593 ) //Handle Handouts
    {
        if( uMsg == WM_LBUTTONUP )
        {
            static bool bOpen = false;

            if( !bOpen )
            {
				if(  !DoAppEvaluate( LME_MAGIC_STRING ,L"HANDOUTS",L"SHARE") ) 
                {    
                    if( WNDPROC pOrgFunc = GetInstance()->GetOrgProcFunc(hwnd) )
                    {
                        lParam = 0xfff0fff;   
                        pOrgFunc( hwnd, WM_MOUSEMOVE, wParam, lParam );       
                    }
                }
                else
                {
                    bOpen = !bOpen;
                    if( WNDPROC pOrgFunc = GetInstance()->GetOrgProcFunc(hwnd) )
                    {
                        pOrgFunc( hwnd, WM_LBUTTONDOWN, wParam, lParam );
                        return pOrgFunc( hwnd, uMsg, wParam, lParam );
                    }
                }
            }
            else
            {
                bOpen = !bOpen;
            }
        }
    }


    if( nID == 0x5e32 ) //Handle Ask
    {
        TCHAR pCaption[1024] = {0};
        GetWindowText( hwnd, pCaption, 1024 );
        if( uMsg == WM_LBUTTONUP )
        {
            if( ( GetWindowLong(hwnd, GWL_STYLE) & WS_MAXIMIZEBOX ) && _wcsnicmp( pCaption, L"Ask", wcslen(pCaption) ) == 0 ) 
            {
               // if( !DoEvaluate( TEXT("[LM_Question]"),L"QUESTION" ) )//!MsgBoxAllowOrDeny( CPartDB::GetInstance()->GetPresenterAttendeeInfo().c_str(), L"Allow to Ask?" )) 
				if( !DoEvaluate( LME_MAGIC_STRING,L"QUESTION" ) )
                {     
                     if( WNDPROC pOrgFunc = GetInstance()->GetOrgProcFunc(hwnd) )
                    {
                        lParam = 0xfff0fff;   
                        pOrgFunc( hwnd, WM_MOUSEMOVE, wParam, lParam );       
                    }
                }
                else
                {
                    if( WNDPROC pOrgFunc = GetInstance()->GetOrgProcFunc(hwnd) )
                    {
                        pOrgFunc( hwnd, WM_LBUTTONDOWN, wParam, lParam );
                        return pOrgFunc( hwnd, uMsg, wParam, lParam );
                    }
                }
            }
        }
    }
    /*if( hwnd == GetInstance()->GetWndAsk() )
    {
    return TRUE;
    }*/

    switch( uMsg )
    {
    case WM_COMMAND:
        {
            switch( LOWORD(wParam) )
            {
            case ID_SHARE_WHITEBOARD:
                {
                    static bool bFirstTime = TRUE;
                    //if( bFirstTime ) {bFirstTime = false; break;}
                    bShared = true;
                   // if( !DoEvaluate( TEXT("[LM_Whiteboard]"),L"SHARE" ) )//!MsgBoxAllowOrDeny( CPartDB::GetInstance()->GetPresenterAttendeeInfo().c_str(), L"Allow to share White Board?" )) 
					if(  !DoAppEvaluate( LME_MAGIC_STRING ,L"WHITEBOARD",L"SHARE") ) 
                    {                        
                        return TRUE;
                    }
                    bFirstTime = TRUE;
                }
                break;
            case ID_SHARE_POLLPAGE:
                {
                    static bool bFirstTime = TRUE;
                    //if( bFirstTime ) {bFirstTime = false; break;}
                    bShared = true;
                    //if( !DoEvaluate( TEXT("[LM_PollPage]"),L"SHARE" ) )//!MsgBoxAllowOrDeny( CPartDB::GetInstance()->GetPresenterAttendeeInfo().c_str(), L"Allow to share Poll Page?" )) 
						if(  !DoAppEvaluate(LME_MAGIC_STRING ,L"POLL",L"SHARE") ) 
                    {                       
                        return TRUE;
                    }
                    bFirstTime = TRUE;
                }
                break;
            case ID_SHARE_TEXTPAGE:
                {
                    static bool bFirstTime = TRUE;
                    //if( bFirstTime ) {bFirstTime = false; break;}
                    bShared = true;
                    //if( !DoEvaluate( TEXT("[LM_TextPage]"),L"SHARE" ) )//!MsgBoxAllowOrDeny( CPartDB::GetInstance()->GetPresenterAttendeeInfo().c_str(), L"Allow to share Text Page?" )) 
					if(  !DoAppEvaluate( LME_MAGIC_STRING ,L"TEXT",L"SHARE") ) 
                    {                        
                        return TRUE;
                    }
                    bFirstTime = TRUE;
                }
                break;
            case ID_SHARE_WEBPAGE:
                {
                    static bool bFirstTime = TRUE;
                    if( bFirstTime ) {bFirstTime = false; break;}
                    bShared = true;
                    //if( !DoEvaluate( TEXT("[LM_Whiteboard]"),L"SHARE" ) )//!MsgBoxAllowOrDeny ( CPartDB::GetInstance()->GetPresenterAttendeeInfo().c_str(), L"Allow to share Web Page?" )) 
					if(  !DoAppEvaluate( LME_MAGIC_STRING ,L"WEB",L"SHARE") ) 
                    {                        
                        return TRUE;
                    }
                    bFirstTime = TRUE;
                }
                break;
            case ID_SHARE_REMOTEDESTOP:
				{
					 static bool bFirstTime = TRUE;
                    if( bFirstTime ) {bFirstTime = false; break;}
                    bShared = true;
                    //if( !DoEvaluate( TEXT("[LM_Whiteboard]"),L"SHARE" ) )//!MsgBoxAllowOrDeny ( CPartDB::GetInstance()->GetPresenterAttendeeInfo().c_str(), L"Allow to share Web Page?" )) 
					if(  !DoAppEvaluate( LME_MAGIC_STRING ,L"DESKTOP",L"SHARE") ) 
                    {                        
                        return TRUE;
                    }
                    bFirstTime = TRUE;
                }
                break;

            case ID_RECORD_1:
            case ID_RECORD_2:
                {
                    if( !DoEvaluate( LME_MAGIC_STRING,L"RECORD" ) )////!MsgBoxAllowOrDeny( CPartDB::GetInstance()->GetPresenterAttendeeInfo().c_str(), L"Allow to Start Record?" )) 
                    {
                        return TRUE;
                    }
                }
                break;    

            case ID_RAISE_YOUR_HAND:
                {
					DPW(( L"Question" )) ;
                    if( !DoEvaluate( LME_MAGIC_STRING,L"QUESTION" ) )//!MsgBoxAllowOrDeny( CPartDB::GetInstance()->GetPresenterAttendeeInfo().c_str(), L"Allow to Raise Your Hand?" )) 
                    {
                        return TRUE;
                    }
                }
                break;

            case ID_ASK1:
			case ID_ASK3:
                {
					::DPW(( L"Question 111" )) ;
                    /*static int nCount = 0;
                    if( nCount < 2 ) { ++nCount; break; }*/
                    if( !DoEvaluate( LME_MAGIC_STRING,L"QUESTION" ) )//!MsgBoxAllowOrDeny( CPartDB::GetInstance()->GetPresenterAttendeeInfo().c_str(), L"Allow to Raise Your Hand?" )) 
                    {
                        //nCount = 0;
                        return TRUE;
                    }
                    //nCount = 0;
                }
                break;

			case ID_MUTE_AUDIO:
			case ID_MUTE_AUDIO1:
			case ID_MUTE_AUDIO2:
			//case ID_MICROPHONE_AUDIO3:
				{
					if( !DoEvaluate( LME_MAGIC_STRING,L"VOICE" ) )
					{                        
						return TRUE;
					}
				}
                break;
			case ID_COMMAND_AUDIO1:
			//case ID_COMMAND_AUDIO2:
				{
			/*		static bool bFirstTime = TRUE;
					static bool bOpen = false;
					if( bFirstTime ) {bFirstTime = false; break;}
					if( !bOpen )
					{	*/
						if( !DoEvaluate( LME_MAGIC_STRING,L"VOICE" ) )
						{                        
							return TRUE;
						}
					//	bOpen = !bOpen;
					/*}
					else
					{
						bOpen = !bOpen;
					}

					bFirstTime = TRUE;*/
				}break ;
            case ID_SHARED_NOTEs:
                {                    
                    if( bShareNotesOpen ){ bShareNotesOpen = false; break; }
                    if( bShareNotesFirstTime ) 
                    {bShareNotesFirstTime = false; break;}
                    if( !bShareNotesOpen )
                    {
                        if(  !DoAppEvaluate( LME_MAGIC_STRING ,L"NOTES",L"SHARE") )  //if( !MsgBoxAllowOrDeny( CPartDB::GetInstance()->GetPresenterAttendeeInfo().c_str(), L"Allow to open Shared Notes?" )) 
                        {                        
                            return TRUE;
                        }
                        bShareNotesOpen = !bShareNotesOpen;
                    }

                    bShareNotesFirstTime = TRUE;
                }
				break;
			case ID_SHARE_VIDEO2:
				{
					wchar_t szBuf[MAX_PATH] = {0} ;
					HWND hParent = ::GetParent( hwnd ) ;
					if( hParent == NULL )
					{
						break ;
					}
					::GetWindowTextW( hParent,szBuf,MAX_PATH ) ;
					DPW((L"Window Text:[%s]",szBuf)) ;
					if( _wcsnicmp(szBuf, L"voice & video", MAX_PATH) ==0 )
					{
						if( !DoEvaluate( LME_MAGIC_STRING,L"VIDEO" ) )//!MsgBoxAllowOrDeny( CPartDB::GetInstance()->GetPresenterAttendeeInfo().c_str(), L"Allow to open Video?" )) 
						{   
							lParam = -1;
							return TRUE;
						}
					}
					break ;
				}
            case ID_SHARE_VIDEO1:
            case ID_SHARE_VIDEO:
                {
                    static bool bFirstTime = TRUE;
                    static bool bOpen = false;
                    if( bFirstTime ) {bFirstTime = false; break;}
                    if( !bOpen )
                    {
                        if( !DoEvaluate( LME_MAGIC_STRING,L"VIDEO" ) )//!MsgBoxAllowOrDeny( CPartDB::GetInstance()->GetPresenterAttendeeInfo().c_str(), L"Allow to open Video?" )) 
                        {                        
                            return TRUE;
                        }
                        bOpen = !bOpen;
                    }
                    else
                    {
                        bOpen = !bOpen;
                    }
                    bFirstTime = TRUE;
                }
                break;

            case ID_HANDOUTS:
                {
                    static bool bFirstTime = TRUE;
                    if( bFirstTime ) {bFirstTime = false; break;}
                  //  if( !DoEvaluate( TEXT("[LM_Handouts]"),L"SHARE" ) )//!MsgBoxAllowOrDeny( CPartDB::GetInstance()->GetPresenterAttendeeInfo().c_str(), L"Allow to open HandOuts?" )) 
					if(  !DoAppEvaluate( LME_MAGIC_STRING ,L"HANDOUTS",L"SHARE") ) 
                    {                        
                        return TRUE;
                    }
                    bFirstTime = TRUE;
                }
                break;
            case ID_INVITE:
                {
                    static bool bFirstTime = TRUE;
                    if( bFirstTime ) {bFirstTime = false; break;}
                    if( !DoEvaluate(  LME_MAGIC_STRING,L"MEETING" ) )//!MsgBoxAllowOrDeny( CPartDB::GetInstance()->GetPresenterAttendeeInfo().c_str(), L"Allow to open HandOuts?" )) 
                    {                        
                        return TRUE;
                    }
                    bFirstTime = TRUE;
				}
				break;
			case ID_CHAT6:
				{
					OutputDebugStringW( L"==================step1========================" ) ;
					wchar_t szWindowName[MAX_PATH]= {0} ;
					if( ImplementQAChatWindow(hwnd) == TRUE)
					{
						return TRUE;
					}
						::wsprintf( szWindowName,L"MSG ID:%d,LPARAM:%d",uMsg, LOWORD( wParam)) ;
					OutputDebugStringW( szWindowName ) ;
					OutputDebugStringW( L"==================end========================" ) ;
				}break ;
			case ID_CHAT5:
				{

					if( ImplementChatWindow( hwnd) == FALSE )
					{
						return TRUE ;
					}

				}
				break ;
			case ID_CHAT4:
				{
					EnumChildWindows(hwnd, EnumChildProc, NULL); 
					if(m_IsQAChat == TRUE )
					{
						OutputDebugStringW( L"m_IsQAChat" ) ;
						// if( !DoEvaluate( TEXT("[LM_Question]"),L"QUESTION" ) )//!MsgBoxAllowOrDeny( CPartDB::GetInstance()->GetPresenterAttendeeInfo().c_str(), L"Allow to Ask?" )) 
						if( !DoEvaluate(LME_MAGIC_STRING,L"IM") )
						{     
							lParam = -1;
							return TRUE ;
						}
						else
						{
							if( WNDPROC pOrgFunc = GetInstance()->GetOrgProcFunc(hwnd) )
							{
								pOrgFunc( hwnd, WM_LBUTTONDOWN, wParam, lParam );
								m_IsQAChat = FALSE ;
								return pOrgFunc( hwnd, uMsg, wParam, lParam );
							}
						}
					}
					m_IsQAChat = FALSE ;
				}
				break ;
			}     

        }
        break;        
    default:
        {

        }
        break;
    }

    if( WNDPROC pOrgFunc = GetInstance()->GetOrgProcFunc(hwnd) )
    {
        return pOrgFunc( hwnd, uMsg, wParam, lParam );
    }
	return TRUE ;
}

BOOL CMsgHook::DoUnHookMSG(DWORD dThreadID)
{
	::EnterCriticalSection(&CMsgHook::s_csMsgHook);
	for ( std::list<THREADHOOKLINK>::iterator iter = s_pThreadHookLink.begin() ; iter != s_pThreadHookLink.end() ; iter++ )
	{
		if ((*iter).ThreadID == dThreadID)
		{
			if( (*iter).HookhdCallBack != NULL )
			UnhookWindowsHookEx((*iter).HookhdCallBack);
			if( (*iter).HookhdMouse != NULL )
				UnhookWindowsHookEx((*iter).HookhdMouse);
			if( (*iter).HookMSG != NULL )
				UnhookWindowsHookEx((*iter).HookMSG);

			s_pThreadHookLink.erase( iter) ;
			::LeaveCriticalSection(&CMsgHook::s_csMsgHook);
			return TRUE;
		}
	}
	::LeaveCriticalSection(&CMsgHook::s_csMsgHook);
	return FALSE;
}
BOOL CMsgHook::SetMsgHook(DWORD dwThreadId) 
{
    BOOL bOk = FALSE;
	if (dwThreadId != 0 && HasHooked(dwThreadId) == FALSE ) 
	{
        // Make sure that the hook is not already installed.
        //chASSERT(m_hHook == NULL);

        // Save our thread ID in a shared variable so that our GetMsgProc
        // function can post a message back to the thread when the server
        // window has been created.
        m_dwThreadId = dwThreadId;

        // Install the hook on the specified thread
       // m_hHook = SetWindowsHookEx(WH_GETMESSAGE, GetMsgProc1, m_hInstDll, dwThreadId);

		//m_hHook =  SetWindowsHookEx(WH_MSGFILTER, GetMsgProc, m_hInstDll,    dwThreadId);
		HHOOK  hMouse = SetWindowsHookEx(WH_MOUSE, MouseProc, m_hInstDll,    dwThreadId);
		m_hHook = hMouse ;
		HHOOK  hCallBack =   SetWindowsHookEx(WH_CALLWNDPROC, CallWndProc, m_hInstDll,  dwThreadId);
		if( m_hHook != NULL ||  hMouse!= NULL||hCallBack!=NULL)
		{
			DPW((L"SetWindowsHookEx  success:[%d]",dwThreadId)) ;
			THREADHOOKLINK aLink;
			aLink.ThreadID = dwThreadId;
			aLink.HookMSG = m_hHook;
			aLink.HookMSG = hMouse ;
			aLink.HookhdCallBack = hCallBack ;
			::EnterCriticalSection(&CMsgHook::s_csMsgHook);
			s_pThreadHookLink.push_back(aLink);	
			::LeaveCriticalSection(&CMsgHook::s_csMsgHook);
			//CHookedCreateWindow::GetInstance()->Hook( CreateWindowExW );
			bOk = (m_hHook != NULL);

			if (bOk) 
			{
				// The hook was installed successfully; force a benign message to
				// the thread's queue so that the hook function gets called.
				bOk = PostThreadMessage(dwThreadId, WM_NULL, 0, 0);
			}   
		}
    } 
    else 
    {
        // Make sure that a hook has been installed.
        //chASSERT(m_hHook != NULL);
        //bOk = UnhookWindowsHookEx(m_hHook);
       // m_hHook = NULL;
    }     
    return(bOk);
}
HHOOK CMsgHook::GetHookedCallBackHandle(DWORD dThreadID)
{
	for ( std::list<THREADHOOKLINK>::iterator iter = s_pThreadHookLink.begin() ; iter != s_pThreadHookLink.end() ; iter++ )
	{
		if ((*iter).ThreadID == dThreadID)
		{
			return (*iter).HookhdCallBack;
		}
	}
	return NULL ;
}
HHOOK  CMsgHook::GetHookedMsgHandle(DWORD dThreadID)
{
	for ( std::list<THREADHOOKLINK>::iterator iter = s_pThreadHookLink.begin() ; iter != s_pThreadHookLink.end() ; iter++ )
	{
		if ((*iter).ThreadID == dThreadID)
		{
			return (*iter).HookMSG;
		}
	}
	return NULL ;
}
HHOOK  CMsgHook::GetHookedMouseHandle(DWORD dThreadID)
{
	for ( std::list<THREADHOOKLINK>::iterator iter = s_pThreadHookLink.begin() ; iter != s_pThreadHookLink.end() ; iter++ )
	{
		if ((*iter).ThreadID == dThreadID)
		{
			return (*iter).HookhdMouse;
		}
	}
	return NULL ;
}
BOOL CMsgHook::HasHooked(DWORD dThreadID)
{
	for ( std::list<THREADHOOKLINK>::iterator iter = s_pThreadHookLink.begin() ; iter != s_pThreadHookLink.end() ; iter++ )
	{
		if ((*iter).ThreadID == dThreadID)
		{
			return TRUE;
		}
	}
	return FALSE;
}
void CMsgHook::SubWndPorc( HWND hWnd )
{
    if( m_mpProcSub.find( hWnd ) == m_mpProcSub.end() )//!GetInstance()->GetOrgProcFunc() )
    {
        //;//GetInstance()->SetOrgProcFunc( (WNDPROC)SetWindowLongPtr( pCw->hwnd, GWLP_WNDPROC, (LONG)(void*)WindowProc) );
#pragma warning(push)
#pragma warning(disable: 4311 4312)
        m_mpProcSub.insert( std::make_pair(hWnd, (WNDPROC)SetWindowLongPtr( hWnd, GWLP_WNDPROC, (LONG)(PVOID)WindowProc) ) );
#pragma warning(pop)
    }
}

void CMsgHook::SubWndPorc( HWND hWnd, WNDPROC pWndProc )
{
    std::map< HWND, WNDPROC >::iterator it = m_mpProcSub.find( hWnd );
    if( it != m_mpProcSub.end() )//!GetInstance()->GetOrgProcFunc() )
    {
#pragma warning(push)
#pragma warning(disable: 4311 4312)
        (*it).second = (WNDPROC)SetWindowLongPtr( hWnd, GWLP_WNDPROC, (LONG)(PVOID)pWndProc );
#pragma warning(pop)
    }
}

LRESULT CALLBACK CMsgHook::MouseProc(int nCode, WPARAM wParam, LPARAM lParam) 
{
    if (nCode < 0)  // do not process message 
        return CallNextHookEx(GetInstance()->GetHookHandle(), nCode, wParam, lParam); 


    MOUSEHOOKSTRUCT* pMouse = (MOUSEHOOKSTRUCT*)lParam;    

    if( wParam == WM_LBUTTONUP || wParam == WM_LBUTTONDBLCLK )
    {
		wchar_t szBuf[MAX_PATH]={0} ;
		::GetWindowText( pMouse->hwnd,szBuf,MAX_PATH) ;
        LONG nID = GetWindowLong(pMouse->hwnd, GWL_ID );
		if( nID == ID_CHAT6 )
		{
			if( _wcsnicmp( szBuf,L"reply",wcslen(szBuf) ) == 0 )
			{
				ImplementQAChatWindow( pMouse->hwnd ) ;
			}
		}
        if( nID == 0x5e32  || nID == 0x593 || nID == 0x592|| nID == 0x5209)
        {
            GetInstance()->SubWndPorc( pMouse->hwnd );
        }
    }

    if( wParam == WM_LBUTTONDBLCLK )
    {
        LONG nID = GetWindowLong(pMouse->hwnd, GWL_ID );
        if( nID == 0x3FC )
        {
            GetInstance()->SubWndPorc( pMouse->hwnd );
        }
    }


	return CallNextHookEx(GetInstance()->GetHookHandle(), nCode, wParam, lParam); 
}


LRESULT WINAPI CMsgHook::CallWndProc(int nCode, WPARAM wParam, LPARAM lParam)
{

	/*HHOOK curHook = CMsgHook::GetHookedCallBackHandle(GetCurrentThreadId()) ;
	if( curHook == NULL )
	{
		return S_OK ;
	}*/
	if (nCode < 0)  // do not process message 
		return CallNextHookEx(GetInstance()->GetHookHandle(), nCode, wParam, lParam); 

	//    bool bContinue = true;
	switch (nCode) 
	{ 
	case HC_ACTION:
		{
			PCWPSTRUCT pCw = (PCWPSTRUCT)lParam;            
			if( pCw!=NULL&& HandleMsg( pCw->hwnd, pCw->wParam, pCw->message ) )
			{
				//return CMsgHook::WindowProc( pCw->hwnd, LOWORD(pCw->message),pCw->wParam,pCw->lParam ) ;
				GetInstance()->SubWndPorc( pCw->hwnd );

				return S_OK;
			}
		}
		break; 

	default: 
		break; 
	} 

	return CallNextHookEx(GetInstance()->GetHookHandle(), nCode, wParam, lParam); 
}

//LRESULT WINAPI CMsgHook::GetMsgProc1(int nCode, WPARAM wParam, LPARAM lParam)
//{
//    if (nCode < 0) // do not process message 
//        return CallNextHookEx(GetInstance()->GetHookHandle(), nCode, 
//        wParam, lParam); 
//
//    switch (nCode) 
//    { 
//    case HC_ACTION: 
//        switch (wParam) 
//        { 
//        case PM_REMOVE:
//            {
//                Dlg_Proc( (HWND)GetInstance()->GetHookHandle(), nCode, wParam, lParam );
//            } 
//            break; 
//
//        case PM_NOREMOVE:
//            {
//                Dlg_Proc( (HWND)GetInstance()->GetHookHandle(), nCode, wParam, lParam );
//            } 
//            break; 
//
//        default:
//            {
//                // TODO: write error handler
//            } 
//            break; 
//        } 
//
//
//    default: 
//        break; 
//    } 
//
//    return CallNextHookEx(GetInstance()->GetHookHandle(), nCode, wParam, lParam); 
//}


LRESULT WINAPI CMsgHook::GetMsgProc(int nCode, WPARAM wParam, LPARAM lParam) 
{
	//HHOOK curHook = CMsgHook::GetHookedMouseHandle(GetCurrentThreadId()) ;
	//	if( curHook == NULL )
	//	{
	//		return S_OK ;
	//	}
    static BOOL bFirstTime = TRUE;

    if (bFirstTime) 
    {
        // The DLL just got injected.
        bFirstTime = FALSE;

        // Uncomment the line below to invoke the debugger
        // on the process that just got the injected DLL.
        // ForceDebugBreak();

        // Create the DIPS Server window to handle the client request.
        //CreateDialog(g_hInstDll, MAKEINTRESOURCE(IDD_DIPS), NULL, Dlg_Proc);

        // Tell the DIPS application that the server is up
        // and ready to handle requests.
        PostThreadMessage( GetInstance()->GetThreadId(), WM_NULL, 0, 0);
    }

    if (nCode < 0)  // do not process message 
    {
        return CallNextHookEx(GetInstance()->GetHookHandle(), nCode, wParam, lParam); 
    }


    
    switch (nCode) 
    { 
    case HC_ACTION: 
        switch (wParam) 
        { 
        case PM_REMOVE:
            {
               
            } 
            break; 

        case PM_NOREMOVE:
            {
               MSG* msg = (MSG*)lParam ;
			   if( IsWindow( msg->hwnd ) && msg->message == BM_SETSTYLE  )
			   {
				     //LOWORD( wParam) |BS_PUSHBUTTON
				   wchar_t szBuf[MAX_PATH]= {0} ;
				   ::GetClassName(  msg->hwnd,szBuf,MAX_PATH) ;
				   ::OutputDebugStringW(szBuf) ;
				   if( ::_wcsnicmp( szBuf,L"WTL_CrossFadeButton",wcslen(L"WTL_CrossFadeButton") )==0 )
				   {
					   ::OutputDebugStringW(L"Chat window click............") ;
				   }
			   }
            } 
            break; 

        default:
            {
                // TODO: write error handler
            } 
            break; 
        } 
	}

    return( CallNextHookEx( GetInstance()->GetHookHandle(), nCode, wParam, lParam) );
}

bool CMsgHook::HandleMsg( HWND hwnd, WPARAM wParam, UINT nMsg )
{
//    UINT nHiSel = HIWORD (wParam);
    UINT nSelection = LOWORD (wParam);

    if( nMsg == ID_CHAT3 )
    {
        return true;
    }

    if( nMsg == HDM_GETITEMW )
    {
        TCHAR strName[64] = {0};
        GetClassName( GetParent(hwnd), strName, 64 );
        if( wcscmp( strName, L"PWSuperTreeCtrl" ) == 0 )
        {
            return true;
        }
        return false;
    }

    if( nMsg == TTM_GETTOOLINFOW )
    {
        return true;
    }

    if( LB_SETCURSEL == nMsg )
    {
        return true;
    }

    if( nMsg == WM_TIMER || nMsg == WM_MOUSEMOVE )
    {
        return false;
    }

   /* if( (nMsg == 0x2111 ) && ID_ASK == nSelection )
    {

        return true;
    }*/

    UINT nMsgLow = LOWORD(nMsg);

    if( nMsgLow == ID_TEXT )
    {
        LONG nID = GetWindowLong(hwnd, GWL_ID );

        TCHAR strClassName[64] = {0};

        GetClassName(hwnd, strClassName, 64 );

        if( //wcscmp( strClassName, L"WTLProjection" ) == 0 ||
            wcscmp( strClassName, L"PWWebSlide" ) == 0 ||
            wcscmp( strClassName, L"AtlAxWin80" ) == 0 ||
            wcscmp( strClassName, L"Shell Embedding" ) == 0 ||
            wcscmp( strClassName, L"Shell DocObject View" ) == 0 ||
            wcscmp( strClassName, L"Internet Explorer_Server" ) == 0 ||
            wcscmp( strClassName, L"PWAnnoViewer" ) == 0 ||
            wcscmp( strClassName, L"PWPollSlide" ) == 0 ||
            wcscmp( strClassName, L"PWTextSlide" ) == 0 || ( nID == 0xffffffff && wcscmp( strClassName, L"WTLEdit" ) == 0 ) 
            )
        {
            return true;
        }
        else if( wcscmp( strClassName, L"WTL_OwnerDrawScrollBar" ) == 0 )
        {
            TCHAR strClassName2[64] = {0};

            GetClassName( GetParent(GetParent(GetParent(hwnd))), strClassName2, 64 );
            if( wcscmp( strClassName2, L"WTLProjection" ) == 0 )
            {
                return true;
            }
        }
    }

    if( /*nMsg < WM_USER &&*/ nMsgLow != WM_COMMAND )
    {
        return false;
    }    

    /*if( ( LOWORD(nMsg) != WM_COMMAND ) && ( LOWORD(nMsg) != WM_LBUTTONDOWN ) )
    {
    return false;
    }  */  

    char strEvent[64] = {0};

    sprintf_s( strEvent, 64, "nSelection: %d, Msg: %x\n", nSelection, nMsg );

    DPA(( strEvent ));

    
    //return false;
    if( ID_SHARE_WHITEBOARD == nSelection ||
        ID_SHARE_WEBPAGE == nSelection ||
		ID_SHARE_REMOTEDESTOP == nSelection||
        ID_SHARE_POLLPAGE == nSelection ||
        ID_SHARE_TEXTPAGE == nSelection ||
        ID_SHARE_UPLOADFILE == nSelection ||
        ID_RECORD_1 == nSelection ||
        ID_RECORD_2 == nSelection ||
        ID_RAISE_YOUR_HAND == nSelection ||
        ID_MUTE_AUDIO == nSelection ||
        ID_MUTE_AUDIO1 == nSelection ||
        ID_MUTE_AUDIO2 == nSelection ||
        ID_SHARED_NOTEs == nSelection ||
        ID_SHARE_VIDEO == nSelection ||
        ID_SHARE_VIDEO1 == nSelection ||
		ID_SHARE_VIDEO2	== nSelection ||
        ID_HANDOUTS == nSelection ||
        ID_INVITE == nSelection || 
		ID_CHAT4 ==nSelection||
		ID_CHAT5 ==nSelection||
		ID_CHAT6 ==nSelection||
		ID_ASK1 ==nSelection||
		ID_ASK3 ==nSelection||
		ID_COMMAND_AUDIO1 ==nSelection)
		//ID_MICROPHONE_AUDIO3 == nSelection ||
		//||		ID_COMMAND_AUDIO2==nSelection|| ID_CHAT1 || ID_CHAT2 )
    {
        return true;
    }    

    return false;
    //return false;
}


//INT_PTR WINAPI CMsgHook::Dlg_Proc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) 
//{
//	hWnd ;
//	uMsg ;
//	wParam ;
//	lParam ;
////    PMSG pMsg = (PMSG)lParam;
//
//    //HandleMsg( pMsg->wParam, pMsg->message );
//    return(FALSE);
//}

void CMsgHook::AddMsgHandler( const MsgHandler& aMsgHandler )
{
    std::map< UINT32, std::map< UINT32, SubMsgHandler > >::iterator 
        it = m_mpMsgHandler.find( aMsgHandler.m_uMsgId );
    
    if( it == m_mpMsgHandler.end() )
    {
        std::map< UINT32, SubMsgHandler > aMap;
        aMap.insert( std::make_pair( aMsgHandler.m_uSubMsgId, aMsgHandler.m_SubMsgHandler ) );
        m_mpMsgHandler[aMsgHandler.m_uSubMsgId] = aMap;
    }
    else
    {
        //std::map< UINT32, std::pair< std::vector<UINT32>, FuncWindowProc > >& aMap = (*it).second;
        //std::map< UINT32, std::pair< std::vector<UINT32>, FuncWindowProc > >::iterator it1 = aMap.find( aMsgHandler.m_uSubMsgId );
        //if( it1 == aMap.end() )
        {
            (*it).second.insert( std::make_pair( aMsgHandler.m_uSubMsgId, aMsgHandler.m_SubMsgHandler ) );
        } 
    }
}

