#ifndef __CONTROL_BASE_H__
#define __CONTROL_BASE_H__
/*
define the basic method for the control
*/
namespace ctrlBase
{
	class BaseCtrl
	{
	public :
		BaseCtrl() {} ;
		~BaseCtrl(){} ;
	public:

		virtual RECT countWndRectByText( HWND _hWnd ) 
		{
			RECT rcWnd = {0} ;
			wchar_t szBuf[MAX_PATH] = {0} ;
			if( _hWnd )
			{
				::GetWindowText( _hWnd,szBuf,MAX_PATH ) ;
			}
			return rcWnd ;
		};
	};
};
#endif