#ifndef __NEXTLABS_COMMUNICATOR_H__
#define __NEXTLABS_COMMUNICATOR_H__
#include "resource.h"       // main symbols

#include <atlhost.h>
#pragma warning(push)
#pragma warning(disable: 6387)
#include "atlimage.h"
#pragma warning(pop)
#include "ImageBase.h"
/*
create a template panel for the windows implement
*/
//template<class T1>
class CBaseWindow:
	public CWindowImpl< CBaseWindow,CWindow >,
	public CImageBase
{
public:
	CBaseWindow() {} ;
	virtual ~CBaseWindow() {
		m_imgBackground.Destroy() ;
		//m_imgBackground.ReleaseDC() ;
	} ;
public :
	HRESULT SetBackGround( CImage _imgBackground )
	{
		HRESULT hr = S_OK ;
		m_imgBackground = _imgBackground ;
		return hr ;
	};
	HRESULT SetBackGround(  HINSTANCE _hInst,INT _IID )
	{
		_IID;	//for warning C4100
		HRESULT hr = S_OK ;
		//m_imgBackground = _imgBackground ;
		if( m_imgBackground.IsNull() )
		{
			this->InitImage( &m_imgBackground, _hInst,IDR_PNG_BACKGROUND ) ;
		}
		if( m_imgIdleIcon.IsNull() )
		{
			this->InitImage( &m_imgIdleIcon, _hInst, IDR_IDLE_ICON ) ;
		}
		return hr ;
	};
	//	HRESULT CreateWindow( 
public :
	virtual BOOL ProcessWindowMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
		LRESULT& lResult, DWORD dwMsgMapID)
	{
		switch(uMsg)
		{
		case WM_ERASEBKGND:
			return OnEraseBackground( hWnd, uMsg, wParam, lParam, lResult, dwMsgMapID ) ;
		default:
			break ;
		}; 
		return 0 ;
	} ;
protected:
	virtual BOOL OnEraseBackground( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
		LRESULT& lResult, DWORD dwMsgMapID ) 
	{
		dwMsgMapID; //for warning C4100
		lResult;	//for warning C4100
		lParam;		//for warning C4100
		wParam;		//for warning C4100
		uMsg;		//for warning C4100
		hWnd;		//for warning C4100

		BOOL bRet = FALSE ;
		if( m_imgBackground.IsNull() )
		{
			return bRet ;
		}
		HDC hdc = this->GetDC() ;
		if( hdc == NULL ) 
		{
			return bRet ;
		}
		

		RECT rc ;
		::GetWindowRect( m_hWnd, &rc ) ;
		INT iWidth = rc.right -rc.left ;
		INT iHeight = rc.bottom- rc.top ;
		bRet = m_imgBackground.StretchBlt( hdc,0,0, iWidth,iHeight, SRCCOPY  ) ;

		bRet = m_imgIdleIcon.BitBlt( hdc, (iWidth -m_imgIdleIcon.GetWidth())/2 ,iHeight-m_imgIdleIcon.GetHeight() -5,SRCCOPY ) ;
		ReleaseDC( hdc ) ;
		bRet = TRUE ;
		return bRet ;
	};
private:
	CImage m_imgBackground ;
	CImage m_imgIdleIcon ;
	//HWND m_hWnd ;
};
#endif