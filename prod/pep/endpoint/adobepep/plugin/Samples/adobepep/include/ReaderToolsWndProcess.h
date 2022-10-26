#pragma once

#include <windows.h>
#include <vector>

namespace AdobeXI
{
	class CReaderToolsWndProcess
	{
	public:
		CReaderToolsWndProcess(void);
		~CReaderToolsWndProcess(void);

	public:
		void AddToolsWnd(HWND hWnd);
		void DeleteToolsWnd(HWND hWnd);
		bool IsToolsWnd(HWND hWnd);
		bool ProcessMsg( MSG* pMsg);

	protected:
		bool ScanRect(HDC hdc, int nX, int nY, int nCX, int nCY, COLORREF clr1, COLORREF clr2);
		bool FindBtn(HWND hWnd, int nX, int nY, std::vector<int>& vXStarts, int nCX, int nCY, COLORREF clr1, COLORREF clr2, RECT *pRect);
		bool PtInBtnRect(HWND hwnd, int nX, int nY, RECT* pRect);
		bool PtInConvertBtn(HWND hWnd, int nX, int nY );
		bool PtInCreatePDFBtn(HWND hWnd, int nX, int nY );
		bool PtInSendFileBtn(HWND hWnd, int nX, int nY );
		bool PtInStoreFileBtn(HWND hWnd, int nX, int nY );

	protected:
		std::vector<HWND> m_hWndTools;
		CRITICAL_SECTION m_csWndTools;
	};


	extern CReaderToolsWndProcess theReaderToolsWndProc;
}