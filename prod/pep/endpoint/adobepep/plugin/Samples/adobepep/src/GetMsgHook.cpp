#include "../include/GetMsgHook.h"
#include "../include/ReaderToolsWndProcess.h"

namespace AdobeXI
{
	CGetMsgHook theGetMsgHook;

	CGetMsgHook::CGetMsgHook(void)
	{
		m_hGetMsgHook = NULL;
	}

	CGetMsgHook::~CGetMsgHook(void)
	{
		UnHookHandle();
	}

	/**return the handle of WH_GETMESSAGE*/
	HHOOK CGetMsgHook::GetHookHandle()
	{
		return m_hGetMsgHook;
	}

	void  CGetMsgHook::UnHookHandle()
	{
		if (m_hGetMsgHook != NULL )
		{
			UnhookWindowsHookEx(m_hGetMsgHook);
			m_hGetMsgHook = NULL;
		}
	}
 
	/**install a WindowsHook,WH_GETMESSAGE*/
	bool CGetMsgHook::InstallGetMsgHook()
	{
		if( m_hGetMsgHook == NULL )
		{
			m_hGetMsgHook = SetWindowsHookEx(WH_GETMESSAGE, GetMsgProcMy, NULL, ::GetCurrentThreadId() );
			if( NULL == m_hGetMsgHook)
			{
				//  wchar_t szMsg[200] = {0};
				//  wsprintf( szMsg, L"Set Hook error, getlatError=%d\n", GetLastError() );
				// OutputDebugStringW( szMsg );
			}
		}

		return NULL != m_hGetMsgHook;
	}

	LRESULT CALLBACK CGetMsgHook::GetMsgProcMy(_In_  int code,_In_  WPARAM wParam, _In_  LPARAM lParam )
	{
		if( code<0 )
		{
			return ::CallNextHookEx(NULL, code, wParam, lParam );
		}
		else
		{
			//
			MSG *pMsg = (MSG*)lParam;
			if( pMsg )
			{

				if( pMsg->message == WM_KEYDOWN )
				{
					//disable tab
					if( pMsg->wParam == VK_TAB )
					{
						pMsg->message = WM_USER;
						OutputDebugStringW( L"Disable Tab key\n");
						return 0;
					}
					else if((pMsg->wParam == 'W') || (pMsg->wParam == 'w'))
					{
						bool bCtrlDown =  (GetKeyState(VK_LCONTROL)&0x8000) || (GetKeyState(VK_RCONTROL)&0x8000);
						bool bShiftDown =  (GetKeyState(VK_LSHIFT)&0x8000) || (GetKeyState(VK_RSHIFT)&0x8000);
						if( bCtrlDown && bShiftDown )
						{
							pMsg->message = WM_USER;
							OutputDebugStringW( L"Disable Ctrl + Shift + W\n");
							return 0;
						}
					}
				}
				else if( theReaderToolsWndProc.IsToolsWnd(pMsg->hwnd) ) 
				{
					if(theReaderToolsWndProc.ProcessMsg( pMsg ) )
					{
						return 0;
					}
				}

			}

			return ::CallNextHookEx(NULL, code, wParam, lParam );
		}
	}

}



