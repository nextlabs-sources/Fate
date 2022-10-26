#include "../include/ReaderToolsWndProcess.h"
#include "AdobeXI.h"

extern const char* ACTION_COPY;
extern const char* ACTION_CONVERT; 
extern const char* ACTION_SEND;

namespace AdobeXI
{

	CReaderToolsWndProcess theReaderToolsWndProc;

	CReaderToolsWndProcess::CReaderToolsWndProcess(void)
	{
		InitializeCriticalSection( &m_csWndTools );
	}

	CReaderToolsWndProcess::~CReaderToolsWndProcess(void)
	{
		DeleteCriticalSection( &m_csWndTools );
	}

	/**add a HWND of adobe Tools Window.*/
	void CReaderToolsWndProcess::AddToolsWnd(HWND hWnd)
	{
		EnterCriticalSection( &m_csWndTools );

		std::vector<HWND>::iterator itExist = std::find(m_hWndTools.begin(), m_hWndTools.end(), hWnd);
		if (itExist == m_hWndTools.end())
		{
			m_hWndTools.push_back(hWnd);
		}

		LeaveCriticalSection( &m_csWndTools );
	}

	/**delete a HWND of adobe Tools Window, when we received WM_DESTORY*/
	void CReaderToolsWndProcess::DeleteToolsWnd(HWND hWnd)
	{
		EnterCriticalSection( &m_csWndTools );

		std::vector<HWND>::iterator itExist = std::find(m_hWndTools.begin(), m_hWndTools.end(), hWnd);
		if (itExist != m_hWndTools.end())
		{
			m_hWndTools.erase(itExist);
			OutputDebugStringW(L"deletetoolswnd\n");
		}

		LeaveCriticalSection( &m_csWndTools );
	}

	bool CReaderToolsWndProcess::IsToolsWnd(HWND hWnd)
	{
		EnterCriticalSection( &m_csWndTools );

		std::vector<HWND>::iterator itExist = std::find(m_hWndTools.begin(), m_hWndTools.end(), hWnd);
		bool bIsToolsWnd = itExist != m_hWndTools.end();

		LeaveCriticalSection( &m_csWndTools );
        

		return bIsToolsWnd;

	}

	/**process tools wnd message*/
	/**if we processed this message return true*/
	bool CReaderToolsWndProcess::ProcessMsg(MSG* pMsg)
	{
		bool bProcessed = false;
		if( pMsg->message == WM_LBUTTONUP )
		{
			int  xPos = LOWORD(pMsg->lParam); 
			int  yPos = HIWORD(pMsg->lParam); 

			string output_file;
		
			bool bRet = getCurrentPDFPath(output_file);

			bool bConvertAllow = true;
			bool bSendAllow = true;
			string strCovertAction = ACTION_CONVERT;
			string strSendAction = ACTION_SEND;
		
			PDDoc pDoc = NULL;
			if (bRet)
			{
				pDoc = AdobeXI::CAdobeXITool::GetCurrentFilePDDoc(output_file);
			}
			else
			{
				AdobeXI::CAdobeXITool::IsExistOneDenyActionPath(strCovertAction, bConvertAllow,output_file,pDoc);
				AdobeXI::CAdobeXITool::IsExistOneDenyActionPath(strSendAction, bSendAllow,output_file,pDoc);
			}


			if(!output_file.empty() && PtInConvertBtn(pMsg->hwnd, xPos, yPos))
			{
				/**if user clicked convert btn but this file can't have convert right. we deny it.*/
				/**we deny it by modify this message to WM_USER*/
				bConvertAllow = AdobeXI::CAdobeXITool::DoActionOnLine(strCovertAction,CE_NOISE_LEVEL_USER_ACTION,output_file, pDoc);
				if (!bConvertAllow)
				{
					pMsg->message = WM_USER; 
					bProcessed = true;
				}
				
			}
			/*	else if( PtInCreatePDFBtn(pMsg->hwnd, xPos, yPos) )
			{
			pMsg->message = WM_USER;
			bProcessed = true;
			} */
			else if(!output_file.empty()&&PtInSendFileBtn(pMsg->hwnd, xPos, yPos) )
			{
				bSendAllow = AdobeXI::CAdobeXITool::DoActionOnLine(strSendAction,CE_NOISE_LEVEL_USER_ACTION,output_file, pDoc);
				if (!bSendAllow)
				{
					pMsg->message = WM_USER;
					bProcessed = true;
				}
			}
		}
		else if (pMsg->message == WM_DESTROY)
		{
			DeleteToolsWnd(pMsg->hwnd);
		}


		return bProcessed;
	}
   
	/**scan if the hdc exist a line of clr1 or clr2*/
	bool CReaderToolsWndProcess::ScanRect(HDC hdc, int nX, int nY, int nCX, int nCY, COLORREF clr1, COLORREF clr2)
	{
		COLORREF clrLeft = ::GetPixel(hdc, nX-1, nY);
		COLORREF clrRight = ::GetPixel(hdc, nX+nCX, nY);
		if( clrLeft==clr1 || clrRight==clr1 ||
			clrLeft==clr2 || clrRight==clr2 )
		{
			return false;
		}

		//scan X
		for(int nOffsetX=nX; nOffsetX<nX+nCX; nOffsetX++)
		{
			if((::GetPixel(hdc,nOffsetX, nY) != clr1) &&
				(::GetPixel(hdc,nOffsetX, nY) != clr2)
				)
				return false;


		}

		return true;

	}

	bool CReaderToolsWndProcess::FindBtn(HWND hWnd, int nX, int nY, std::vector<int>& vXStarts, int nCX, int nCY, COLORREF clr1, COLORREF clr2, RECT *pRect)
	{
		bool bFindBtn = false;

		RECT rcClient;
		GetClientRect(hWnd, &rcClient );

		RECT rcScanRgn = rcClient;
		rcScanRgn.left = 49;
		rcScanRgn.right = 178;

		//
		POINT ptMouse;
		ptMouse.x = nX;
		ptMouse.y = nY;
		if( !::PtInRect(&rcScanRgn, ptMouse ) )
		{
			//	OutputDebugStringW(L"not in scan rgn\n");
			return false;
		}


		//src dc
		HDC hdcSrc = GetDC(hWnd);

		std::vector<int>::iterator itXStart = vXStarts.begin();
		while( itXStart != vXStarts.end() )
		{
			int iSrcX = *itXStart;
			for (int iSrcY=rcScanRgn.top; iSrcY<rcScanRgn.bottom; iSrcY++)
			{
				bFindBtn =ScanRect(hdcSrc, iSrcX , iSrcY, nCX, nCY, clr1, clr2);

				if(bFindBtn)
				{
					pRect->left = iSrcX; pRect->right = iSrcX + nCX;
					pRect->top = iSrcY; pRect->bottom = iSrcY + nCY; 

					break;
				} 
			}

			if(bFindBtn)
			{
				break;
			}

			itXStart++;

		}

		//
		::ReleaseDC(hWnd, hdcSrc);

		return bFindBtn;
	}

	bool CReaderToolsWndProcess::PtInBtnRect(HWND hwnd, int nX, int nY, RECT* pRect)
	{
		::InflateRect(pRect, 2, 2);

		POINT ptMouse2;
		ptMouse2.x = nX;
		ptMouse2.y = nY;
		return PtInRect(pRect, ptMouse2) ;
	}

	bool CReaderToolsWndProcess::PtInConvertBtn(HWND hWnd, int nX, int nY )
	{
		RECT rcBtn;
		std::vector<int> vXStarts;
		vXStarts.push_back(57);
		vXStarts.push_back(65);
		bool bFindConvertBtn = FindBtn(hWnd, nX, nY, vXStarts, 101, 26, RGB(51,51,51), RGB(141,141,141), &rcBtn);

		if( bFindConvertBtn )
		{
			return PtInBtnRect(hWnd, nX, nY, &rcBtn);
		}

		return false;
	}

	bool CReaderToolsWndProcess::PtInCreatePDFBtn(HWND hWnd, int nX, int nY )
	{
		RECT rcBtn;
		std::vector<int> vXStarts;
		vXStarts.push_back(52);
		vXStarts.push_back(60);
		bool bFindConvertBtn = FindBtn(hWnd, nX, nY, vXStarts,111, 26, RGB(51,51,51), RGB(141,141,141), &rcBtn);

		if( bFindConvertBtn )
		{
			return PtInBtnRect(hWnd, nX, nY, &rcBtn);
		}

		return false;
	}

	bool CReaderToolsWndProcess::PtInSendFileBtn(HWND hWnd, int nX, int nY )
	{
		RECT rcBtn;
		std::vector<int> vXStarts;
		vXStarts.push_back(59);
		vXStarts.push_back(67);
		bool bFindConvertBtn = FindBtn(hWnd, nX, nY, vXStarts, 97, 26, RGB(51,51,51), RGB(141,141,141), &rcBtn);

		if( bFindConvertBtn )
		{
			return PtInBtnRect(hWnd, nX, nY, &rcBtn);
		}

		return false;
	}

	bool CReaderToolsWndProcess::PtInStoreFileBtn(HWND hWnd, int nX, int nY )
	{
		RECT rcBtn;
		std::vector<int> vXStarts;
		vXStarts.push_back(74);
		vXStarts.push_back(81);
		bool bFindConvertBtn = FindBtn(hWnd, nX, nY,vXStarts, 68, 30, RGB(51,51,51), RGB(141,141,141), &rcBtn);

		if( bFindConvertBtn )
		{	
			return PtInBtnRect(hWnd, nX, nY, &rcBtn);
		}
		return false;
	}

}