#ifndef _COVERLAYWND_H_
#define _COVERLAYWND_H_

#include "NLVLViewPrint.h"
#include "Auxiliary.h"
#include "OvelayWndMgr.h"
#include "VLObligation.h"
#include <windows.h>
#include <string>
using namespace std;

class OverlayInfo{
public:
	OverlayInfo():m_hView(NULL),m_bUpdateMap(false),m_bRepairFlag(false){}
	void SetoverlayViewMapSize(HWND view,bool update=true,bool repair = false)
	{
		m_hView = view;
		m_bUpdateMap = update;
		m_bRepairFlag = repair;
	}
	void SetoverlayMapSizeFlag(bool update = false,bool repair = true)
	{
		m_bUpdateMap = update;
		m_bRepairFlag = repair;
	}
	HWND GetOIView(void)
	{
		return m_hView;
	}
	void SetOIUpdateMap(bool update)
	{
		m_bUpdateMap = update;
	}
	bool GetOIUpdateMap(void)
	{
		return m_bUpdateMap;
	}
	void SetOIRepairFlag(bool repair)
	{
		m_bRepairFlag = repair;
	}
	bool GetOIRepairFlag(void)
	{
		return m_bRepairFlag;
	}
private:
	HWND	m_hView;		//this window indicate bitmap size.
	bool	m_bUpdateMap;
	bool	m_bRepairFlag;	//this flag tell if need to repair bitmap size.
};
typedef struct _judelation
{
	bool	b_OverlayIsUnder;	//true: overlay is under of frame ; false: overlay is top of frame.
	HWND	h_Overlay;
	HWND	h_Frame;
}JUDELATION;
class COverlayWnd
{
public:
	COverlayWnd(void);
	~COverlayWnd(void);
public:
	static OverlayError CreateOverLayWnd(__in const NM_VLObligation::VisualLabelingInfo& theInfo,__in HWND hParent);
	void ReSize(HWND hwnd=NULL,bool topmost = true);
	bool IsMoved(void);
	void InitOverLayInfo(__in const NM_VLObligation::VisualLabelingInfo& theInfo,__in HWND hView);
	void SethViewWnd(const HWND& hView);
	HWND GethViewWnd(void);
	void SetstrText(const wstring& strText);
	wstring GetstrText(void);
	void SetdwFirstLineSize(const DWORD& dwFirstLineSize);
	DWORD GetdwFirstLineSize(void);
	void SetdwOtherLineSize(const DWORD& dwOtherLineSize);
	DWORD GetdwOtherLineSize(void);
	void Setcolor(const Color& color);
	Color Getcolor(void);
	void SetstrFont(const wstring& strFont);
	wstring GetstrFont(void);
	void SetdwLeftMargin(const DWORD& leftmargin);
	DWORD GetdwLeftMargin(void);
	void SetdwTopMargin(const DWORD& TopMargin);
	DWORD GetdwTopMargin(void);
	void SetdwHorSpace(const DWORD& HorSpace);
	DWORD GetdwHorSpace(void);
	void SetdwVerSpace(const DWORD& VerSpace);
	DWORD GetdwVerSpace(void);
	void SetFontBold(const bool& flag);
	bool GetFontBold(void);
	void SetstrFilePath(const wstring& strFilePath);
	wstring GetstrFilePath(void);
	void SetIsRepeat(const bool& flag);
	bool GetIsRepeat(void);
	void SetdwFontColor(const DWORD& FontColor);
	DWORD GetdwFontColor(void);
	void SetdwTransparency(const DWORD& Transparency);
	DWORD GetdwTransparency(void);
	void SetstrPlacement(const wstring& strPlacement);
	wstring GetstrPlacement(void);
	void SetViewProc(const WNDPROC& viewproc);
	WNDPROC GetViewProc(void);
	void SethOverlayWnd(const HWND& hoverlay);
	HWND GethOverlayWnd(void);
	void SetstTextInfo(const TextInfo& testinfo);
	TextInfo GetstTextInfo(void);
	void SetstrTextValue(const wstring& textValue);
	wstring GetstrTextValue(void);

	void SethForWnd(HWND hfor);
	HWND GethForWnd(void);
	void SethPrevWnd(HWND hfor);
	HWND GethPrevWnd(void);
	void SetbPrevVisibleStatus(bool b);
	bool GetbPrevVisibleStatus(void);

	void SethIfExistPrintView(HWND hwnd);
	bool IsUnderFrame(void);
	
	bool CheckViewToCloseOverlayForAdobe();
	bool CheckAdobeIsClosing();
	bool FullScreenForAdobe3D(HWND hForeWnd);
private:
	void DesignedWindows(wchar_t clsName[],HINSTANCE hInstance);
	static LRESULT APIENTRY WaterMarkProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam );
	static void CreateOLThreadFun(LPVOID lParam);
	static LRESULT CALLBACK ViewWndProc(  HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
	static BOOL CALLBACK EnumFrameProc(HWND hwnd,LPARAM lParam);
	void OtherLineStringToVecter(const wstring& otherLine,vector<wstring>& vecString);//change other line string to vector.
	int ComputeWidth(HDC hdc,const wstring& str,int heigth);//compute the str's width.
	int ComputeHeith(HDC hdc,const wstring& str,int width);
	BOOL UpdateDisplayEx(HWND overlay);
	void RepairEdge(const HWND& hView,POINT& point,DWORD& dwWidth,DWORD& dwHeigth);
	void ControlResize(HWND hForeWnd,bool bVisible);
	void PrintView(void);	//for print view
	bool IsTopChild(void);	//judge if is top child window.
	HWND UpdateForegoundWindow(void);	//update m_hForWnd and return Foreground window
	bool DetectAndDooverlayForPrintView(void);	//detect if is visible , and do overlay if necessary. return m_hViewWnd visible status.
	void ClearhIfExistPrintView(HWND hwnd);
	/*
	**	the function only deal with protect view FullScreen overlay case.
	**	other cases full page overlay will be triggered by  slidshowbegin event.
	**	in: current foreground window;
	**	Return Value: true indicate exist related full page window; false indicate don't exist related reated full page window
	*/
	void ProtectFullScreen(HWND foreWnd);
	/*
	**	brief: Check full page window caption if is related to overlay's view window. if true , zoom overlay .
	**	in: full page window 
	*/
	bool SetFullScreenStatus(HWND hFullScreen);
	bool CheckFrameHadShowOverlay(HWND hFrameWnd);
	/*
	**	brief: EXCEL2007 print view.
	*/
	bool DoPrintViewEXCEL2007(void);
	BOOL CreatePPTProtectModeFullScreenOverlay(HWND hFullScreenWnd);
	BOOL CheckFullScreenHadOverlay(HWND hFullScreenWnd);
	/*
	ppt had multiply documents, so we will check which view is need.
	we will get overlay information by view.
	*/
	void GetRightView(HWND hView);
	void DoProtectModeExtendFullScreen();
	/*
	hFindFullScreen may be is not Top. 
	so we must check the point on screen to get right handle.
	*/
	bool WndIsFullScreen(HWND hWnd,HWND& hTopFullScreen);
	BOOL UpdateDisplay(void);
	OverlayError DrawOverlayInfo(__in HDC &hdcMemory);
	void ComputeTxTOnBitmapPos(__in  DWORD bmWidth,__in  DWORD bmHeigh,__in  Gdiplus::REAL TxTWidth,__in  Gdiplus::REAL TxTHeight,__out vector<PointF> &vecTxTPos);

	/*
	Get hwnd of full screen window in all Split screen window 
	*/
	void GetAllDisplayMonitorsFullScreenHWND(vector<HWND> & vechFullScreenWnd);
	void ComputeSingleTxTPostion(__in const wstring& strFirstLine,Gdiplus::REAL FirstLineSize,
		__in const wstring& strOtherLine,Gdiplus::REAL OtherLineSize,
		__in const StringFormat &strformat,__in const Gdiplus::FontFamily &fontFamily,__in FontStyle emFontStyle,
		__out vector<pair<wstring,PointF>> &vecOtherLineInfo,__out Gdiplus::REAL &Width,__out Gdiplus::REAL &Height);
	void DrawSingleOverlay(__inout Graphics &graphics,__inout GraphicsPath &path,__in PointF Pos,__in const wstring& strFirstLine,__in Gdiplus::REAL FirstLineSize,__in Gdiplus::REAL OtherLineSize,__in const SolidBrush &blackBrush,
		__in const StringFormat &strformat,__in const Gdiplus::FontFamily &fontFamily,__in FontStyle emFontStyle,__in vector<pair<wstring,PointF>> &vecOtherLineInfo);
	void ComputeCenterTxTPos(__in DWORD bmWidth,__in DWORD bmHeigh,__in Gdiplus::REAL TxTWidth,__in Gdiplus::REAL TxTHeight,__out PointF &CenterPos);

	/*
	Get hwnd of full screen window which is show in the top in a rect 
	*/
	HWND GetTopFullScreenHWND(const RECT& FullScreenRect);

	/*
	Get rect area in all Split screen window 
	*/
	void GetAllDisplayMonitorsRect(vector<RECT> &vecMonitorRect);
	static BOOL CALLBACK COverlayWnd::MyInfoEnumProc(_In_  HMONITOR hMonitor,_In_  HDC hdcMonitor,_In_  LPRECT lprcMonitor,_In_  LPARAM dwData);
private:;
	CAuxiliary m_Auxi;
	HWND    	m_hViewWnd;
	wstring 	m_strText;
	DWORD   	m_dwFirstLineSize;
	DWORD   	m_dwOtherLineSize;
	Color		m_color;
	wstring 	m_strFont;
	DWORD   	m_dwLeftMargin;
	DWORD   	m_dwTopMargin;
	DWORD   	m_dwHorSpace;
	DWORD   	m_dwVerSpace;
	bool    	m_bFontBold;
	wstring 	m_strFilePath;
	bool        m_bIsRepeat;
	DWORD       m_dwFontColor;
	DWORD       m_dwTransparency;
	wstring     m_strPlacement;
	WNDPROC     m_ViewProc;
	HWND		m_hOverlayWnd;
	RECT        m_ViewRect;
	TextInfo	m_stTextInfo;
	wstring		m_strTextValue;

	HWND		m_hForWnd;			//cache the latest Frame window that is related to view window.
	HWND		m_hPrevWnd;			//cache previous WM_TIMER foreground window
	bool		m_bPrevVisibleStatus; //cache previous WM_TIMER view window's visible status,Used to judge if click FILE button
	RECT        m_ParentViewRect;	//cache the view's parent window's RECT.
	HWND		m_hIfExistPrintView;//cache view window when click FILE.
	bool        m_bIsAdobeUseFrameSize;
	HWND        m_hCacheView;
	map<HWND,bool> m_mapIsOverlayFullScreen;
	UINT		m_msg;				//cache the latest WM_SIZE message in a cycle 
	WPARAM		m_msgWparam;		//cache the latest WM_SIZE message's param.
	bool		m_bExistWM_SIZE;	//true:exist WM_SIZE message , false: don't exist WM_SIZE message.
	OverlayInfo m_overlayInfo;
	bool		m_IfScreen;
};

#endif
