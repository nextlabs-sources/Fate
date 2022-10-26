#ifndef _COVERLAYWNDMGR_H_
#define _COVERLAYWNDMGR_H_


#include <string>
#include <map>
#include <vector>
using namespace std;

#include "NLVLViewPrint.h"
#include "OverlayWnd.h"
class COverlayWnd;
class COvelayWndMgr 
{
private:
	COvelayWndMgr ();
	~COvelayWndMgr ();
private:
	map<HWND,COverlayWnd*>	m_mapWnd;
	CRITICAL_SECTION		m_theWnd;
	CAuxiliary				m_Auxi;
public:
	static COvelayWndMgr& GetInstance();
	//get a COverlayWnd object according to view handle
	COverlayWnd* GetOverLayInfoFromView(__in const HWND& hView);
	//get a COverlayWnd object according to overlay handle
	COverlayWnd* GetOverLayInfoFromOverlayWnd(__in const HWND& hOverlay);
	//get a COverlayWnd object according to file path
	COverlayWnd* GetOverLayInfoFromPath(__in const wstring& strFilePath);
	//add a record 
	void AddOverLayInfoEx(const HWND& hWnd,COverlayWnd* pOverlay);
	void DeletOverLay(const HWND& hView);
	//clear map cach
	void DeleteAllOverLayInfo(void);
	//judge if exist a record according to  view handle
	bool ExistOverLayInfo(__in const HWND& hView);
	bool AddOverlayAndViewProc(__in const HWND& hWnd,__in const HWND& hOverlay,__in const WNDPROC & ViewProc);
	void SetPrintView(HWND hPrintView,bool bFlag);  //bFlag:true:cache FullpageUIHost  false:clear cach

	bool CheckAdobeFrameWndHadVisibleView(HWND hFrameWnd);
	
public:
	CRITICAL_SECTION m_GlobalSec;
	ULONG_PTR  m_gdiplusToken;
};

#endif