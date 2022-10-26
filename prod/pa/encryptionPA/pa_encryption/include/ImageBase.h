#ifndef __NXTLABS_CHELLEE_IMAGE_BASE__
#define __NXTLABS_CHELLEE_IMAGE_BASE__
/*
define the basic image implement.
*/
#include "atlimage.h"
//#include "global.h"
class CImageBase 
{
public:
	CImageBase(){} ;
	virtual ~CImageBase(){} ;
public:
	/*
	Initialize the image 
	*/
	 void InitImage( CImage* _img, HINSTANCE _hInst,  UINT nIDResource ) 
	{
		//_img.LoadFromResource( _hInst, nIDResource ) ;
		GetResourceStream(_hInst, _img, MAKEINTRESOURCE( nIDResource ) , L"PNG" ) ;
	};
	 void ReleaseImage( CImage _img ) 
	{
		if( !_img.IsNull() )
		{
			_img.ReleaseDC() ;
		}
	}
	 UINT GetResourceStream( HINSTANCE hInst,CImage* pImg, LPCWSTR lpName, LPCWSTR lpType ) 
	{
		UINT iRet = 0 ;
		HRSRC hResource = ::FindResource(/*AfxGetInstanceHandle()*/hInst, lpName,lpType);
		if (!hResource)
		{
			DWORD derr = 0 ;
			derr = ::GetLastError() ;
			return false;
		}

		DWORD imageSize = ::SizeofResource(hInst, hResource);
		if (!imageSize)
			return false;

		//const void* pResourceData = ::LockResource(::LoadResource(hInst, 
		//	hResource));
		//for warning C6309
		HGLOBAL htemp = ::LoadResource(hInst,hResource);
		const void* pResourceData = NULL;
		if (htemp)
			pResourceData = ::LockResource(htemp);

		if (!pResourceData)
			return false;

		m_hBuffer  = ::GlobalAlloc(GMEM_MOVEABLE, imageSize);
		if (m_hBuffer)
		{
			void* pBuffer = ::GlobalLock(m_hBuffer);
			if (pBuffer)
			{
#pragma warning( push ) 
#pragma warning( disable : 6386 )
				CopyMemory(pBuffer, pResourceData, imageSize);
#pragma warning( pop ) 

				CComPtr<IStream> pStream = NULL;
				//Create the image stream..
				if (::CreateStreamOnHGlobal(m_hBuffer, FALSE, &pStream) == S_OK)
				{
					pImg->Load( pStream ) ;
					//DWORD derr = ::GetLastError() ;	//for warning C4189
					if( pImg->IsNull() )
					{
						iRet = 1 ;
						//	MessageBox( 0,0,0,0 ) ;
					}

				}
				::GlobalUnlock(m_hBuffer);
			}
			if( m_hBuffer )
			{
				::GlobalFree(m_hBuffer);
			}
			m_hBuffer = NULL;
		}
		return iRet ;
	};
private :
	 HGLOBAL m_hBuffer;
};
#endif