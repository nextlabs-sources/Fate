#include "StdAfx.h"
#include "OverlayWnd.h"
#include <process.h>  


#define ONSIZE_MSG	WM_USER+1
#define TIMERID1	WM_USER+3
#define WM_OVPRINT	WM_USER+4
#define WM_OVPRINTCLOSE	WM_USER+5
#define	WM_TOPTOP		WM_USER+6
#define WM_SETFORGROUND WM_USER+7

#define MYCOLOR(dwfontcolor) RGB((BYTE)((((DWORD)dwfontcolor)&0x00FF0000)>>16),(BYTE)((((DWORD)dwfontcolor)&0x0000FF00)>>8),(BYTE)(((DWORD)dwfontcolor)&0x000000FF))

COverlayWnd::COverlayWnd(void):m_hForWnd(NULL),m_hPrevWnd(NULL),m_bPrevVisibleStatus(false),m_hIfExistPrintView(NULL),m_bExistWM_SIZE(false)
{
	m_ViewRect.top = 0;
	m_ViewRect.bottom = 0;
	m_ViewRect.left = 0;
	m_ViewRect.right = 0;
	m_ParentViewRect.top = 0;
	m_ParentViewRect.bottom = 0;
	m_ParentViewRect.left = 0;
	m_ParentViewRect.right = 0;
	m_bIsAdobeUseFrameSize = false;
	m_hCacheView = NULL;
	m_IfScreen = false;
}

COverlayWnd::~COverlayWnd(void)
{
	m_mapIsOverlayFullScreen.clear();
}


BOOL CALLBACK ViewBrotherProc(HWND hwnd, LPARAM lParam) 
{ 
	HWND hView = reinterpret_cast<HWND>(lParam);
	if (hView == hwnd)	return TRUE;

	wchar_t strCaption[MAX_PATH] = {0};
	::GetWindowText(hwnd,strCaption,MAX_PATH);

	if(0 == _wcsicmp(strCaption,L"Microsoft Word Document"))
	{
		COvelayWndMgr &OverlayWndMgr = COvelayWndMgr::GetInstance();
		
		//Overlay had exist. so we don't need do overlay
		COverlayWnd* pNeedOverLayInfo= OverlayWndMgr.GetOverLayInfoFromView(hwnd);
		if (pNeedOverLayInfo != NULL)	return TRUE;
	
		//get overlay information.
		COverlayWnd* pOverLayInfo= OverlayWndMgr.GetOverLayInfoFromView(hView);
		if(pOverLayInfo == NULL)	return TRUE;

		//create overlay 
		NM_VLObligation::VisualLabelingInfo theInfo;
						
		theInfo.dwFontSize1		= pOverLayInfo->GetdwFirstLineSize();
		theInfo.dwFontSize2		= pOverLayInfo->GetdwOtherLineSize();		
		theInfo.dwLeftMargin	= pOverLayInfo->GetdwLeftMargin();
		theInfo.dwTopMargin		= pOverLayInfo->GetdwTopMargin();
		theInfo.dwHorSpace		= pOverLayInfo->GetdwHorSpace();
		theInfo.dwVerSpace		= pOverLayInfo->GetdwVerSpace();
		theInfo.bFontBold		= pOverLayInfo->GetFontBold();
		theInfo.dwFontColor     = pOverLayInfo->GetdwFontColor();
		theInfo.dwTransparency  = pOverLayInfo->GetdwTransparency();				

		StringCchPrintf(theInfo.strText,512,L"%s",pOverLayInfo->GetstrText().c_str());
		StringCchPrintf(theInfo.strFontName,64,L"%s",pOverLayInfo->GetstrFont().c_str());
		StringCchPrintf(theInfo.strFilePath,2048,L"%s",pOverLayInfo->GetstrFilePath().c_str());
		StringCchPrintf(theInfo.strPlacement,64,L"%s",pOverLayInfo->GetstrPlacement().c_str());
		StringCchPrintf(theInfo.strTextValue,1024,L"%s",pOverLayInfo->GetstrTextValue().c_str());
		COverlayWnd::CreateOverLayWnd(theInfo, hwnd);
	}
	return TRUE;
}


void COverlayWnd::ClearhIfExistPrintView(HWND hwnd)
{
	CAuxiliary theAu;
	COvelayWndMgr &OverlayWndMgr = COvelayWndMgr::GetInstance();
	emProcessType theAppType = theAu.GetProgressType();
	if(theAppType == WORD_TYPE || theAppType == EXCEL_TYPE|| theAppType == PPT_TYPE)
	{
		wchar_t className[MAX_PATH] = {0};
		GetClassName(hwnd,className,MAX_PATH);
		if(wcscmp(className,L"FullpageUIHost")==0)
		{
			OverlayWndMgr.SetPrintView(hwnd,false);
		}
	}
}
LRESULT CALLBACK COverlayWnd::ViewWndProc(  HWND hwnd,
												UINT uMsg,
												WPARAM wParam,
												LPARAM lParam
												)
{
	COvelayWndMgr &OverlayWndMgr = COvelayWndMgr::GetInstance();
	COverlayWnd* pOverLayInfo = OverlayWndMgr.GetOverLayInfoFromView(hwnd);

	if(NULL == pOverLayInfo)
		return DefWindowProc(hwnd,uMsg,wParam,lParam);
	WNDPROC thePorc = pOverLayInfo->GetViewProc();
	switch(uMsg)
	{
	case WM_SIZE:
		{
			SendMessage(pOverLayInfo->GethOverlayWnd(),ONSIZE_MSG,wParam,reinterpret_cast<LPARAM>(hwnd));
			if(pOverLayInfo->m_Auxi.GetProgressType() == WORD_TYPE)
			{
				::EnumChildWindows(::GetParent(hwnd),ViewBrotherProc,reinterpret_cast<LPARAM>(hwnd));
			}
		}
		break;
	case WM_NCCALCSIZE:		//only deal with WwB Size change 
		PostMessage(pOverLayInfo->GethOverlayWnd(),ONSIZE_MSG,NULL,NULL);
		break;
	case WM_DESTROY:
		{
			//only deal with clear m_hIfExistPrintView when destroy FullPageUIHost window
			pOverLayInfo->ClearhIfExistPrintView(hwnd);
			::SendMessage(pOverLayInfo->GethOverlayWnd(),uMsg,wParam,lParam);
		}
		break;
	case WM_OVPRINT:
	case WM_OVPRINTCLOSE:
		{
			PostMessage(pOverLayInfo->GethOverlayWnd(),uMsg,NULL,NULL);
		}
		break;
	}
	return CallWindowProc(thePorc,hwnd,uMsg,wParam,lParam);
}



void COverlayWnd::CreateOLThreadFun(LPVOID lParam)
{
	HWND hView = static_cast<HWND>(lParam);
	wchar_t strlog[1024] = {0};
	bool bRet = true;
	COvelayWndMgr &OverlayWndMgr = COvelayWndMgr::GetInstance();
	COverlayWnd* pOverLayInfo = OverlayWndMgr.GetOverLayInfoFromView(hView);
	if (pOverLayInfo == NULL)
	{
		StringCchPrintf(strlog,1024,L"GetOverLayInfoFromView fail, the view handle is [%p]\n",hView);
		::OutputDebugString(strlog);
		goto ExitOverlayThread;
	}
	
	HINSTANCE hInstance = GetModuleHandle(NULL);
	wchar_t clsName[]= L"WaterMarkWindows";
	pOverLayInfo->DesignedWindows(clsName,hInstance);
	//caption file path
	HWND hOverLay = NULL;
	if(pOverLayInfo->m_IfScreen)
	{
		hOverLay = CreateWindowExW(/*WS_EX_TOPMOST|*/WS_EX_TRANSPARENT|WS_EX_LAYERED|WS_EX_TOOLWINDOW,
			clsName,L"WaterMarkWindows",WS_POPUP|WS_MAXIMIZE,0,0,10,10,
			NULL,NULL,hInstance,NULL);
		//set window type
		DWORD dwStyle = GetWindowLongPtrW(hOverLay,GWL_STYLE);
		dwStyle &= ~WS_VISIBLE;
		SetWindowLongPtrW(hOverLay,GWL_STYLE,dwStyle);
	}
	else
	{
		RECT viewRect;
		GetWindowRect(hView,&viewRect);
		hOverLay = CreateWindowExW(WS_EX_TOPMOST|WS_EX_TRANSPARENT|WS_EX_LAYERED|WS_EX_TOOLWINDOW,
			clsName,NULL,NULL ,viewRect.left,viewRect.top,viewRect.right-viewRect.left,viewRect.bottom-viewRect.top,
			NULL,NULL,hInstance,NULL);

		//set window type
		DWORD dwStyle = static_cast<DWORD>(GetWindowLongPtrW(hOverLay,GWL_STYLE));
		dwStyle &= ~WS_VISIBLE;dwStyle &= ~WS_CAPTION;
		SetWindowLongPtrW(hOverLay,GWL_STYLE,dwStyle);
		DWORD dwStyleEx = static_cast<DWORD>(GetWindowLongPtrW(hOverLay,GWL_EXSTYLE));
		dwStyleEx &= ~WS_EX_WINDOWEDGE;dwStyleEx &= ~WS_EX_CLIENTEDGE;
		SetWindowLongPtrW(hOverLay,GWL_EXSTYLE,dwStyleEx);

		if(pOverLayInfo->m_dwTransparency<1)
			pOverLayInfo->m_dwTransparency = 0;
		else if(pOverLayInfo->m_dwTransparency >100)
			pOverLayInfo->m_dwTransparency = 100;
		BYTE alph = static_cast<BYTE>((255 * (100-pOverLayInfo->m_dwTransparency)) / 100);

		if(pOverLayInfo->m_dwFontColor==0xFFFFFF)
		{
			SetLayeredWindowAttributes(hOverLay, RGB(0,0,0),alph, LWA_ALPHA|LWA_COLORKEY);
		}
		else
		{
			SetLayeredWindowAttributes(hOverLay, RGB(255,255,255),alph, LWA_ALPHA|LWA_COLORKEY);
		}
	}

	bool bPPTFullScreen = true;
	WNDPROC ViewProc = reinterpret_cast<WNDPROC>(::SetWindowLongPtrW(hView,GWLP_WNDPROC,(LONG_PTR)ViewWndProc));

	if (ViewProc == NULL)
	{
		wchar_t strClsName[MAX_PATH] = {0};
		GetClassName(hView,strClsName,MAX_PATH);
		if (_wcsicmp(strClsName,L"screenClass") == 0)
		{
			DWORD dwCurProcId = GetCurrentProcessId();
			DWORD dwFullScreenWndProcId = 0;
			GetWindowThreadProcessId(hView,&dwFullScreenWndProcId);
			if (CAuxiliary::GetParentProcessID(dwFullScreenWndProcId) == dwCurProcId)
			{
				bPPTFullScreen = false;
			}
		}
		if (bPPTFullScreen)
		{
		    StringCchPrintf(strlog,1024,L"set window log fail!, the view handle is [%p]\n",hView);
			::OutputDebugString(strlog);
			goto ExitOverlayThread;
		}
	}

	bRet = OverlayWndMgr.AddOverlayAndViewProc(hView,hOverLay,ViewProc);
	if (!bRet)
	{
		StringCchPrintf(strlog,1024,L"AddOverlayAndViewProc fail, the view handle is [%p]\n",hView);
		::OutputDebugString(strlog);
		goto ExitOverlayThread;
	}

	if(pOverLayInfo->m_IfScreen)
		pOverLayInfo->UpdateDisplay();
	else
	{
		pOverLayInfo->UpdateDisplayEx(hOverLay);
		ShowWindow(hOverLay,SW_SHOWNOACTIVATE);
		UpdateWindow(hOverLay);
		SetWindowRgn(hOverLay,NULL,true);
	}

	SetTimer(hOverLay,TIMERID1,250,NULL);

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

ExitOverlayThread:
	StringCchPrintf(strlog,1024,L"end the overlay thread, the thread ID is [%d] @@\n",::GetCurrentThreadId());
	::OutputDebugString(strlog);
	
	OverlayWndMgr.DeletOverLay(hView);
	_endthread();
}

OverlayError COverlayWnd::CreateOverLayWnd(const NM_VLObligation::VisualLabelingInfo& theInfo,HWND hParent)
{
	COverlayWnd* pOverLayInfo = new COverlayWnd;
	pOverLayInfo->InitOverLayInfo(theInfo,hParent);
	COvelayWndMgr &OverlayWndMgr = COvelayWndMgr::GetInstance();
	OverlayWndMgr.AddOverLayInfoEx(hParent,pOverLayInfo);
	wchar_t strlog[MAX_PATH+1] = {0};
	GetClassName(hParent,strlog,MAX_PATH);
	if(_wcsicmp(strlog,L"screenClass")==0)
	{
		pOverLayInfo->m_IfScreen = true;
	}
	_beginthread(CreateOLThreadFun,NULL,hParent);
	return NLOverlaySuccess;
}

void COverlayWnd::DesignedWindows(wchar_t clsName[],HINSTANCE hInstance)
{
	WNDCLASS wc;
	wc.style =  CS_VREDRAW | CS_HREDRAW ;
	wc.lpfnWndProc = WaterMarkProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	if(m_dwFontColor==0xFFFFFF)
	{
		wc.hbrBackground = (HBRUSH)(GetStockObject(BLACK_BRUSH));
	}
	else
	{
		wc.hbrBackground = (HBRUSH)(GetStockObject(WHITE_BRUSH));
	}
	wc.hIcon = NULL;
	wc.hCursor = NULL;
	wc.lpszMenuName = NULL;
	wc.lpszClassName = clsName;

	RegisterClass(&wc);
}

void COverlayWnd::InitOverLayInfo(__in const NM_VLObligation::VisualLabelingInfo& theInfo,__in HWND hView)
{
	SethViewWnd(hView);
	m_overlayInfo.SetoverlayViewMapSize(hView);

	SetstrText(theInfo.strText);	
	SetdwFirstLineSize(theInfo.dwFontSize1);
	SetdwOtherLineSize(theInfo.dwFontSize2);
	SetstrFont(theInfo.strFontName);
	SetdwLeftMargin(theInfo.dwLeftMargin);
	SetdwTopMargin(theInfo.dwTopMargin);
	SetdwHorSpace(theInfo.dwHorSpace);
	SetdwVerSpace(theInfo.dwVerSpace);
	SetFontBold(theInfo.bFontBold);
	SetstrFilePath(theInfo.strFilePath);
	SetdwFontColor(theInfo.dwFontColor);
	SetdwTransparency(theInfo.dwTransparency);
	SetstrPlacement(theInfo.strPlacement);
	SetstrTextValue(theInfo.strTextValue);
	
	CAuxiliary   aux;
	Setcolor(aux.ChooseFontColor(theInfo.dwFontColor,theInfo.dwTransparency));
	SetIsRepeat(aux.IsRepeat(theInfo.strPlacement));
	TextInfo linTextInfo;
	aux.GetUserHostHome(theInfo.strTextValue,linTextInfo);
	SetstTextInfo(linTextInfo);
}

bool COverlayWnd::IsTopChild(void)
{
	bool bRet = false;
	CAuxiliary theAu;
	emProcessType theAppType = theAu.GetProgressType();
	switch(theAppType)
	{
	case WORD_TYPE:
		{
			HWND hWwF = FindWindowEx(m_hForWnd,NULL,L"_WwF",L"");
			wchar_t className[MAX_PATH] = {0};
			GetClassName(m_hViewWnd,className,MAX_PATH);
			if(wcscmp(className,L"_WwG")==0)
			{
				if(GetParent(m_hViewWnd)==GetWindow(hWwF,GW_CHILD))	bRet = true;
			}
			else
			{
				if(m_hViewWnd == GetWindow(hWwF,GW_CHILD))	bRet = true;
			}
		}
		break;
	case EXCEL_TYPE:
		{
			HWND hXLDESK = FindWindowEx(m_hForWnd,NULL,L"XLDESK",L"");
			if(m_hViewWnd == GetWindow(hXLDESK,GW_CHILD))	bRet = true;
		}
		break;
	case PPT_TYPE:
		bRet = true;
		break;
	default:
		break;
	}
	return bRet;
}
void COverlayWnd::PrintView(void)
{
	HWND hFullpageUIHost = FindWindowEx(m_hForWnd,NULL,L"FullpageUIHost",L"");

	if(!hFullpageUIHost || !IsTopChild())
		return ;
	NM_VLObligation::VisualLabelingInfo theInfo;

	theInfo.dwFontSize1		= GetdwFirstLineSize();
	theInfo.dwFontSize2		= GetdwOtherLineSize();		
	theInfo.dwLeftMargin	= GetdwLeftMargin();
	theInfo.dwTopMargin		= GetdwTopMargin();
	theInfo.dwHorSpace		= GetdwHorSpace();
	theInfo.dwVerSpace		= GetdwVerSpace();
	theInfo.bFontBold		= GetFontBold();
	theInfo.dwFontColor     = GetdwFontColor();
	theInfo.dwTransparency  = GetdwTransparency();				

	StringCchPrintf(theInfo.strText,512,L"%s",GetstrText().c_str());
	StringCchPrintf(theInfo.strFontName,64,L"%s",GetstrFont().c_str());
	StringCchPrintf(theInfo.strFilePath,2048,L"%s",GetstrFilePath().c_str());
	StringCchPrintf(theInfo.strPlacement,64,L"%s",GetstrPlacement().c_str());
	StringCchPrintf(theInfo.strTextValue,1024,L"%s",GetstrTextValue().c_str());

	COvelayWndMgr &OverLayIns = COvelayWndMgr::GetInstance();
	EnterCriticalSection(&OverLayIns.m_GlobalSec);
	if(!OverLayIns.ExistOverLayInfo(hFullpageUIHost))
	{
		COverlayWnd::CreateOverLayWnd(theInfo, hFullpageUIHost);
	}
	OverLayIns.SetPrintView(hFullpageUIHost,true);
	LeaveCriticalSection(&OverLayIns.m_GlobalSec);
}
HWND COverlayWnd::UpdateForegoundWindow(void)
{
	//foreground window
	HWND hCurrentForeWnd = ::GetForegroundWindow();
	//update m_hForeWnd
	if(IsChild(hCurrentForeWnd,GethViewWnd())|| hCurrentForeWnd == GethViewWnd())
	{
		SethForWnd(hCurrentForeWnd);			
	}
	//for pps/ppsm... file dialog
	if(!m_hForWnd)
	{
		wchar_t diaClass[MAX_PATH] = {0};
		GetClassName(m_hViewWnd,diaClass,MAX_PATH);
		if(wcscmp(diaClass,L"screenClass") ==0)
		{
			m_hForWnd = m_hViewWnd;
		}
	}
	//for adobe opened in IE
	if(!m_hForWnd)
		m_hForWnd = GetAncestor(m_hViewWnd,GA_ROOT);
	return hCurrentForeWnd;
}
bool COverlayWnd::DetectAndDooverlayForPrintView(void)
{
	//detect view window if visible
	bool bVisible = IsWindowVisible(GethViewWnd());
	CAuxiliary theAu;
	if(theAu.GetProgressType() == WORD_TYPE ||
		theAu.GetProgressType() == EXCEL_TYPE || 
		theAu.GetProgressType() == PPT_TYPE )
	{
		if(!m_hIfExistPrintView && !bVisible )	
		{
			PrintView();
		}
	}
	return bVisible;
}

void COverlayWnd::ControlResize(HWND hForeWnd,bool bVisible)
{
	//this code is resolve word print case (when click print , IsWindowVisible api return true.)
	CAuxiliary theAu;
	HWND hFullpageUIHost = NULL;
	if(theAu.GetProgressType() == WORD_TYPE )
		hFullpageUIHost = FindWindowEx(m_hForWnd,NULL,L"FullpageUIHost",L"");
    BOOL bIsFullPageUIHostVisible = TRUE;
    if (hFullpageUIHost)
    {
        bIsFullPageUIHostVisible = IsWindowVisible(hFullpageUIHost);
    }

	if((bVisible && !hFullpageUIHost) || (bVisible && hFullpageUIHost==m_hViewWnd) || (bVisible && !bIsFullPageUIHostVisible)) //if window is visible and don't active "FILE" button
	{
        bool condition = (hForeWnd == m_hOverlayWnd) && (m_hPrevWnd == m_hForWnd);  // fix bug28610, avoid to switch theirselves between ForWnd and overlay window.
		//view window hide->visible ,we need Resize overlay once
		if(!condition && (hForeWnd != GethPrevWnd() || !GetbPrevVisibleStatus()))
		{
			bool brelationship = false;
			HWND hlind = hForeWnd;
			while(hlind)
			{
				hlind = GetWindow(hlind,GW_OWNER);
				if(hlind == GethForWnd() && hlind)
				{
					brelationship = true;
					break;
				}
			}
			if(brelationship || hForeWnd == GethForWnd())
			{
				m_overlayInfo.SetoverlayMapSizeFlag();
			}
			else
			{
				emProcessType pType = m_Auxi.GetProgressType();
				if( (pType == WORD_TYPE || pType == PPT_TYPE) && !IsWindowVisible(m_hOverlayWnd))
				{
					if(m_Auxi.GetAppVer() == 2010)
					{
						ShowWindow(m_hOverlayWnd,SW_SHOWNA);
						PostMessage(m_hOverlayWnd,WM_TOPTOP,true,NULL);
					}
				}
				SetWindowPos(m_hOverlayWnd,HWND_NOTOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
				if(!SetWindowPos(hForeWnd,HWND_TOP,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE))
					::OutputDebugStringW(L"**bring windows to top isn't ok\n");
			}
		}
	}
	else
	{
		bool bHide = true;		
		if(CheckAdobeIsClosing())
		{
			bHide = false;
		}
		if (bHide)
		{
			ShowWindow(GethOverlayWnd(),SW_HIDE);
			bVisible = false;
		}
	}
	//update m_bHidePrevStatus and m_hForWnd
	SetbPrevVisibleStatus(bVisible);
	SethPrevWnd(hForeWnd);
}
/*
**	brief: Check full page window caption if is related to overlay's view window. if true , zoom overlay .
**	in: full page window 
*/
BOOL COverlayWnd::CheckFullScreenHadOverlay(HWND hFullScreenWnd)
{
	COvelayWndMgr &OverlayWndMgr = COvelayWndMgr::GetInstance();

	//Overlay had exist. so we don't need do overlay
	COverlayWnd* pNeedOverLayInfo= OverlayWndMgr.GetOverLayInfoFromView(hFullScreenWnd);
	if (pNeedOverLayInfo != NULL)	
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

BOOL COverlayWnd::CreatePPTProtectModeFullScreenOverlay(HWND hFullScreenWnd)
{
	COvelayWndMgr &OverlayWndMgr = COvelayWndMgr::GetInstance();

	//Overlay had exist. so we don't need do overlay
	COverlayWnd* pNeedOverLayInfo= OverlayWndMgr.GetOverLayInfoFromView(hFullScreenWnd);
	if (pNeedOverLayInfo != NULL)	
	{
		return TRUE;
	}

	//get overlay information.
	COverlayWnd* pOverLayInfo= OverlayWndMgr.GetOverLayInfoFromView(m_hViewWnd);
	if(pOverLayInfo == NULL)	
	{
		return TRUE;
	}

	//create overlay 
	NM_VLObligation::VisualLabelingInfo theInfo;

	theInfo.dwFontSize1		= pOverLayInfo->GetdwFirstLineSize();
	theInfo.dwFontSize2		= pOverLayInfo->GetdwOtherLineSize();		
	theInfo.dwLeftMargin	= pOverLayInfo->GetdwLeftMargin();
	theInfo.dwTopMargin		= pOverLayInfo->GetdwTopMargin();
	theInfo.dwHorSpace		= pOverLayInfo->GetdwHorSpace();
	theInfo.dwVerSpace		= pOverLayInfo->GetdwVerSpace();
	theInfo.bFontBold		= pOverLayInfo->GetFontBold();
	theInfo.dwFontColor     = pOverLayInfo->GetdwFontColor();
	theInfo.dwTransparency  = pOverLayInfo->GetdwTransparency();				

	StringCchPrintf(theInfo.strText,512,L"%s",pOverLayInfo->GetstrText().c_str());
	StringCchPrintf(theInfo.strFontName,64,L"%s",pOverLayInfo->GetstrFont().c_str());
	StringCchPrintf(theInfo.strFilePath,2048,L"%s",pOverLayInfo->GetstrFilePath().c_str());
	StringCchPrintf(theInfo.strPlacement,64,L"%s",pOverLayInfo->GetstrPlacement().c_str());
	StringCchPrintf(theInfo.strTextValue,1024,L"%s",pOverLayInfo->GetstrTextValue().c_str());
	COverlayWnd::CreateOverLayWnd(theInfo, hFullScreenWnd);
	return FALSE;
}



bool COverlayWnd::WndIsFullScreen(HWND hWnd,HWND& hTopFullScreen)
{
	wchar_t strClsName[MAX_PATH] = {0};
	hTopFullScreen = ::GetAncestor(hWnd,GA_ROOTOWNER);
	GetClassName(hTopFullScreen,strClsName,MAX_PATH);

	if (_wcsicmp(strClsName,L"screenClass") == 0)
	{
		return true;
	}
	return false;
}

BOOL CALLBACK COverlayWnd::MyInfoEnumProc(
							 _In_  HMONITOR hMonitor,
							 _In_  HDC hdcMonitor,
							 _In_  LPRECT lprcMonitor,
							 _In_  LPARAM dwData
							 )
{
	((vector<RECT> *)dwData)->push_back(*lprcMonitor);
	return TRUE;
}


void COverlayWnd::GetAllDisplayMonitorsRect(vector<RECT> &vecMonitorRect)
{
	EnumDisplayMonitors(NULL, NULL, MyInfoEnumProc, (LPARAM)&vecMonitorRect);  
}

HWND COverlayWnd::GetTopFullScreenHWND(const RECT& FullScreenRect)
{
	POINT Left_Top     = {FullScreenRect.left + 10,FullScreenRect.top + 10};
	POINT Left_Bottom  = {FullScreenRect.left + 10,FullScreenRect.bottom - 10};
	POINT right_Top    = {FullScreenRect.right - 10,FullScreenRect.top + 10};
	POINT right_Bottom = {FullScreenRect.right - 10,FullScreenRect.bottom - 10};
	POINT center_point = {(FullScreenRect.left + FullScreenRect.right)/2,(FullScreenRect.top + FullScreenRect.bottom)/2};

	HWND hTopFullScreen = NULL;
	HWND hWnd1 = WindowFromPoint(Left_Top);
	if (WndIsFullScreen(hWnd1,hTopFullScreen))
	{
		return hTopFullScreen;
	}

	HWND hWnd2 = WindowFromPoint(Left_Bottom);
	if (WndIsFullScreen(hWnd2,hTopFullScreen))
	{
		return hTopFullScreen;
	}

	HWND hWnd3 = WindowFromPoint(right_Top);
	if (WndIsFullScreen(hWnd3,hTopFullScreen))
	{
		return hTopFullScreen;
	}

	HWND hWnd4 = WindowFromPoint(right_Bottom);
	if (WndIsFullScreen(hWnd4,hTopFullScreen))
	{
		return hTopFullScreen;
	}

	HWND hWnd5 = WindowFromPoint(center_point);
	if (WndIsFullScreen(hWnd5,hTopFullScreen))
	{
		return hTopFullScreen;
	}

	return NULL;
}

void COverlayWnd::GetAllDisplayMonitorsFullScreenHWND(vector<HWND> & vechFullScreenWnd)
{
	vector<RECT> vecMonitorRect;
	GetAllDisplayMonitorsRect(vecMonitorRect);
	vector<RECT>::iterator itor;
	HWND hTemp = NULL;
	for (itor = vecMonitorRect.begin(); itor != vecMonitorRect.end(); itor++)
	{
		hTemp = GetTopFullScreenHWND(*itor);
		if (hTemp != NULL)
		{
			vechFullScreenWnd.push_back(hTemp);
		}
	}
}


void COverlayWnd::DoProtectModeExtendFullScreen()
{
	vector<HWND> vechFullScreenWnd;
	GetAllDisplayMonitorsFullScreenHWND(vechFullScreenWnd);
	if (vechFullScreenWnd.empty())
	{
		return ;
	}

	vector<HWND>::iterator itor;
	for (itor = vechFullScreenWnd.begin(); itor != vechFullScreenWnd.end(); itor++)
	{
		BOOL bExistOverlay = CheckFullScreenHadOverlay(*itor);
		if (!bExistOverlay)
		{
			GetRightView(*itor);
		}
	}
}

void COverlayWnd::GetRightView(HWND hTopFullScreen)
{
	wchar_t screenClassTitle[MAX_PATH] = {0};
	GetWindowText(hTopFullScreen,screenClassTitle,MAX_PATH);
	wstring fullPageCaption = screenClassTitle;
	wchar_t strForeTitle[MAX_PATH] = {0};
	GetWindowText(m_hForWnd,strForeTitle,MAX_PATH);
	wstring foreWndCapton = strForeTitle;
	CAuxiliary::GetFileNameFromCaption(fullPageCaption);
	CAuxiliary::GetFileNameFromCaption(foreWndCapton);

	if(_wcsicmp(fullPageCaption.c_str(),foreWndCapton.c_str())==0)
	{
		CreatePPTProtectModeFullScreenOverlay(hTopFullScreen);
	}
}
/*
**	the function only deal with protect view FullScreen overlay case.
**	other cases full page overlay will be triggered by  slidshowbegin event.
**	in: current foreground window;
**	Return Value: true indicate exist related full page window; false indicate don't exist related reated full page window
*/
void COverlayWnd::ProtectFullScreen(HWND foreWnd)
{
	wchar_t strCaption[MAX_PATH]={0};
	GetWindowText(GetParent(GetParent(m_hViewWnd)),strCaption,MAX_PATH);
	if(boost::algorithm::icontains(strCaption,L"[Protected View]"))
	{
		if(m_Auxi.GetProgressType() == PPT_TYPE)
		{
			DoProtectModeExtendFullScreen();
		}
	}
}
LRESULT APIENTRY COverlayWnd::WaterMarkProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam )
{
	COvelayWndMgr &OverlayWndMgr = COvelayWndMgr::GetInstance();
	COverlayWnd* pOverLayInfo = OverlayWndMgr.GetOverLayInfoFromOverlayWnd(hWnd);

	if(NULL == pOverLayInfo)	
		return DefWindowProc(hWnd,message,wParam,lParam);
	switch (message)
	{
	case WM_TIMER:
		{
			CAuxiliary::DisableAeroPeek();

			{	//ONSIZE_MSG message process
				//Here deal with the latest WM_SIZE message between previous WM_TIMER and this WM_TIMER
				if(pOverLayInfo->m_bExistWM_SIZE && IsWindowVisible(hWnd))
				{	
					pOverLayInfo->m_overlayInfo.SetoverlayMapSizeFlag(true,true);
					if(pOverLayInfo->m_msgWparam == SIZE_MINIMIZED)	ShowWindow(hWnd,SW_HIDE);
					pOverLayInfo->m_bExistWM_SIZE = false;
					goto ProcessMapSize;
				}		
			}

			HWND hForegroundWindow = pOverLayInfo->UpdateForegoundWindow();
		
		    if (pOverLayInfo->CheckViewToCloseOverlayForAdobe())	break;
			if(pOverLayInfo->FullScreenForAdobe3D(hForegroundWindow))	goto ProcessMapSize;
			
			//detect PPT protect view and do full screen overlay
			pOverLayInfo->ProtectFullScreen(hForegroundWindow);

			/*because ppt full screen in protect mode, so the full screen is in other process
			we can hook the view window process,so the view proc is null. we check view isn't window 
			we will close the overlay window.*/
			if (pOverLayInfo->m_ViewProc == NULL&&
				!IsWindow(pOverLayInfo->m_hViewWnd))
			{
				/*
				from the test, we found the code don't enter, but the overlay thread had exit. it may be forced exit by office 
				but we add this code in order to safe.
				*/
				KillTimer(hWnd,NULL);
				PostQuitMessage(NULL);
			}
			
			//move
			if(pOverLayInfo->IsMoved())
			{
				if(pOverLayInfo->m_overlayInfo.GetOIView() == pOverLayInfo->m_hViewWnd)
					pOverLayInfo->m_overlayInfo.SetoverlayMapSizeFlag();
				else
					pOverLayInfo->m_overlayInfo.SetoverlayMapSizeFlag(true,true);
				goto ProcessMapSize;
			}
			bool bVisible = pOverLayInfo->DetectAndDooverlayForPrintView();
			
			//control Resize times , Set window topmost or non topmost , and call Resize
			pOverLayInfo->ControlResize(hForegroundWindow,bVisible);
			if(pOverLayInfo->IsUnderFrame())
			{
				SetWindowPos(pOverLayInfo->m_hOverlayWnd,HWND_TOPMOST,0,0,0,0,SWP_NOMOVE|SWP_SHOWWINDOW|SWP_NOSIZE);
			}			
ProcessMapSize:
			//here we update map or resize window by relevant parameter
			if(pOverLayInfo->m_overlayInfo.GetOIUpdateMap())
			{
				if(pOverLayInfo->m_IfScreen)		pOverLayInfo->UpdateDisplay();
				else	pOverLayInfo->UpdateDisplayEx(hWnd); 
				pOverLayInfo->m_overlayInfo.SetOIUpdateMap(false);
			}
			if(pOverLayInfo->m_overlayInfo.GetOIRepairFlag())
			{
				pOverLayInfo->ReSize();
				pOverLayInfo->m_overlayInfo.SetOIRepairFlag(false);
			}
		}
		break;
	case WM_DESTROY:
		KillTimer(hWnd,NULL);
		PostQuitMessage(NULL);
		break;
	case ONSIZE_MSG:
		{
			//In ProtectView FullPage case , we don't deal with WM_SIZE message.
			pOverLayInfo->m_bExistWM_SIZE = true;
			pOverLayInfo->m_msg = message;
			pOverLayInfo->m_msgWparam = wParam;
		}
		break;
	case WM_PAINT:
		{
			if(pOverLayInfo->m_IfScreen)	pOverLayInfo->UpdateDisplay();
			else	pOverLayInfo->UpdateDisplayEx(hWnd); 
		}
		break;
	case WM_DISPLAYCHANGE:
		{
			if(pOverLayInfo->m_IfScreen)	pOverLayInfo->UpdateDisplay();
			else	pOverLayInfo->UpdateDisplayEx(hWnd); 
		}
		break;
	case WM_OVPRINT:
		{
			pOverLayInfo->m_hCacheView = GetParent(pOverLayInfo->m_hViewWnd);
			pOverLayInfo->m_overlayInfo.SetoverlayViewMapSize(pOverLayInfo->m_hCacheView,true,true);
		}
		break;
	case WM_OVPRINTCLOSE:
		{
			pOverLayInfo->m_overlayInfo.SetoverlayViewMapSize(pOverLayInfo->m_hViewWnd,true,true);
			pOverLayInfo->m_hCacheView = NULL;
		}
		break;
	case WM_TOPTOP:
		{
			HWND hCurrentWnd = GetForegroundWindow();
			if(IsWindow(hCurrentWnd))
			{
				if((bool)wParam)
				{
					if(hCurrentWnd != pOverLayInfo->m_hForWnd)
					{
						SetWindowPos(pOverLayInfo->m_hOverlayWnd,HWND_NOTOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
						PostMessage(hWnd,WM_TOPTOP,false,NULL);
					}
				}
				SetWindowPos(hCurrentWnd,HWND_TOP,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
			}
			else
				PostMessage(hWnd,WM_SETFORGROUND,NULL,NULL);
		}
		break;
	case WM_SETFORGROUND:
		{
			HWND hCurrentWnd = GetForegroundWindow();
			if(hCurrentWnd == pOverLayInfo->m_hOverlayWnd)
				SetForegroundWindow(pOverLayInfo->m_hForWnd);
		}
		break;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}
void COverlayWnd::RepairEdge(const HWND& hView,POINT& point,DWORD& dwWidth,DWORD& dwHeight)
{
	//word:_WwG,_WwB excel:EXCEL7 PPT:PPTFrameClass,childClass
	RECT lRect,lRectEdge;//lRectEdge word:_WwF,excel:XLDESK,PPT:
	GetWindowRect(hView,&lRect);
	GetWindowRect(hView,&lRectEdge);
	wchar_t className[MAX_PATH] = {0};
	GetClassName(hView,className,MAX_PATH);
	HWND hLin = NULL,hLin2 = NULL;
	if(wcscmp(className,L"_WwG")==0)
	{
		hLin =GetParent(hView);
		if(hLin != NULL)	hLin2 =GetParent(hLin);
		if(hLin2 != NULL)	GetWindowRect(hLin2,&lRectEdge);
	}
	else if(wcscmp(className,L"_WwB")==0)
	{
		hLin =GetParent(hView);
		if(hLin != NULL)	GetWindowRect(hLin,&lRectEdge);
	}
	else if(_wcsicmp(className,L"EXCEL7")==0 ||
		_wcsicmp(className,L"mdiClass")==0 ||
		_wcsicmp(className,L"FullpageUIHost")==0)
	{
		GetWindowRect(m_hForWnd,&lRectEdge);
	}
	if(lRect.top<lRectEdge.top)
		lRect.top=lRectEdge.top;
	if(lRect.bottom>lRectEdge.bottom)
		lRect.bottom=lRectEdge.bottom;
	if(lRect.left<lRectEdge.left)
		lRect.left=lRectEdge.left;
	if(lRect.right>lRectEdge.right)
		lRect.right=lRectEdge.right;
	point.x=lRect.left;
	point.y=lRect.top;
	dwWidth=lRect.right-lRect.left;
	dwHeight=lRect.bottom-lRect.top;
}
void COverlayWnd::ReSize(HWND hwnd,bool topmost)
{
	emProcessType processType = m_Auxi.GetProgressType();
	if(processType == ADOBE_TYPE)
	{
		if (m_bIsAdobeUseFrameSize)
		{
			hwnd = GethForWnd();
		}
	}
	else if(processType == WORD_TYPE)
	{
		if(m_Auxi.GetAppVer()==2010)
		{
			if(m_hForWnd != GetForegroundWindow())	topmost = false;
		}
	}

	if(hwnd == NULL)	hwnd = m_overlayInfo.GetOIView();
	POINT pUL;
	DWORD dwWidth,dwHeight;
	RepairEdge(hwnd,pUL,dwWidth,dwHeight);

	ShowWindow(m_hOverlayWnd,SW_SHOW);
	::UpdateWindow(m_hOverlayWnd);
	SetWindowPos(GethOverlayWnd(),topmost?HWND_TOPMOST:HWND_NOTOPMOST,pUL.x,pUL.y,dwWidth,dwHeight,SWP_SHOWWINDOW|SWP_NOACTIVATE);		
	if(!topmost)	PostMessage(m_hOverlayWnd,WM_TOPTOP,false,NULL);
	m_bIsAdobeUseFrameSize = false;
}

bool COverlayWnd::IsMoved(void)
{
	bool bMove = false;
	RECT ViewRect,ParentViewRect;
	GetWindowRect(m_overlayInfo.GetOIView(),&ViewRect);
	if(m_ViewRect.top != ViewRect.top|| m_ViewRect.bottom != ViewRect.bottom || m_ViewRect.left != ViewRect.left || m_ViewRect.right != ViewRect.right)
	{
		m_ViewRect = ViewRect;
		bMove = true;
	}
	HWND hParent = GetParent(GetParent(m_overlayInfo.GetOIView()));
	if(hParent)
	{
		GetWindowRect(hParent,&ParentViewRect);
		if(m_ParentViewRect.top != ParentViewRect.top|| m_ParentViewRect.bottom != ParentViewRect.bottom || m_ParentViewRect.left != ParentViewRect.left || m_ParentViewRect.right != ParentViewRect.right)
		{
			m_ParentViewRect = ParentViewRect;
			bMove = true;
		}
	}
	return bMove;
}

void COverlayWnd::SethViewWnd(const HWND& hView)
{
	m_hViewWnd = hView;
}
HWND COverlayWnd::GethViewWnd(void)
{
	return m_hViewWnd;
}
void COverlayWnd::SetstrText(const wstring& strText)
{
	m_strText = strText;
}
wstring COverlayWnd::GetstrText(void)
{
	return m_strText;
}
void COverlayWnd::SetdwFirstLineSize(const DWORD& dwFirstLineSize)
{
	m_dwFirstLineSize = dwFirstLineSize;
}
DWORD COverlayWnd::GetdwFirstLineSize(void)
{
	return m_dwFirstLineSize;
}
void COverlayWnd::SetdwOtherLineSize(const DWORD& dwOtherLineSize)
{
	m_dwOtherLineSize = dwOtherLineSize;
}
DWORD COverlayWnd::GetdwOtherLineSize(void)
{
	return m_dwOtherLineSize;
}
void COverlayWnd::Setcolor(const Color& color)
{
	m_color = color;
}
Color COverlayWnd::Getcolor(void)
{
	return m_color;
}
void COverlayWnd::SetstrFont(const wstring& strFont)
{
	m_strFont = strFont;
}
wstring COverlayWnd::GetstrFont(void)
{
	return m_strFont;
}
void COverlayWnd::SetdwLeftMargin(const DWORD& leftmargin)
{
	m_dwLeftMargin = leftmargin;
}
DWORD COverlayWnd::GetdwLeftMargin(void)
{
	return m_dwLeftMargin;
}
void COverlayWnd::SetdwTopMargin(const DWORD& TopMargin)
{
	m_dwTopMargin = TopMargin;
}
DWORD COverlayWnd::GetdwTopMargin(void)
{
	return m_dwTopMargin;
}
void COverlayWnd::SetdwHorSpace(const DWORD& HorSpace)
{
	m_dwHorSpace = HorSpace;
}
DWORD COverlayWnd::GetdwHorSpace(void)
{
	return m_dwHorSpace;
}
void COverlayWnd::SetdwVerSpace(const DWORD& VerSpace)
{
	m_dwVerSpace = VerSpace;
}
DWORD COverlayWnd::GetdwVerSpace(void)
{
	return m_dwVerSpace;
}
void COverlayWnd::SetFontBold(const bool& flag)
{
	m_bFontBold = flag;
}
bool COverlayWnd::GetFontBold(void)
{
	return m_bFontBold;
}
void COverlayWnd::SetstrFilePath(const wstring& strFilePath)
{
	m_strFilePath = strFilePath;
}
wstring COverlayWnd::GetstrFilePath(void)
{
	return m_strFilePath;
}
void COverlayWnd::SetIsRepeat(const bool& flag)
{
	m_bIsRepeat = flag;
}
bool COverlayWnd::GetIsRepeat(void)
{
	return m_bIsRepeat;
}
void COverlayWnd::SetdwFontColor(const DWORD& FontColor)
{
	m_dwFontColor = FontColor;
}
DWORD COverlayWnd::GetdwFontColor(void)
{
	return m_dwFontColor;
}
void COverlayWnd::SetdwTransparency(const DWORD& Transparency)
{
	m_dwTransparency = Transparency;
}
DWORD COverlayWnd::GetdwTransparency(void)
{
	return m_dwTransparency;
}
void COverlayWnd::SetstrPlacement(const wstring& strPlacement)
{
	m_strPlacement = strPlacement;
}
wstring COverlayWnd::GetstrPlacement(void)
{
	return m_strPlacement;
}
void COverlayWnd::SetViewProc(const WNDPROC& viewproc)
{
	m_ViewProc = viewproc;
}
WNDPROC COverlayWnd::GetViewProc(void)
{
	return m_ViewProc;
}
void COverlayWnd::SethOverlayWnd(const HWND& hoverlay)
{
	m_hOverlayWnd = hoverlay;
}
HWND COverlayWnd::GethOverlayWnd(void)
{
	return m_hOverlayWnd;
}
void COverlayWnd::SetstTextInfo(const TextInfo& testinfo)
{
	m_stTextInfo.strHostHome = testinfo.strHostHome;
	m_stTextInfo.strUser = testinfo.strUser;
}
TextInfo COverlayWnd::GetstTextInfo(void)
{
	return m_stTextInfo;
}
void COverlayWnd::SetstrTextValue(const wstring& textValue)
{
	m_strTextValue = textValue;
}
wstring COverlayWnd::GetstrTextValue(void)
{
	return m_strTextValue;
}
void COverlayWnd::SethForWnd(HWND hfor)
{
	m_hForWnd = hfor;
}
HWND COverlayWnd::GethForWnd(void)
{
	return m_hForWnd;
}
void COverlayWnd::SethPrevWnd(HWND hfor)
{
	m_hPrevWnd = hfor;
}
HWND COverlayWnd::GethPrevWnd(void)
{
	return m_hPrevWnd;
}
void COverlayWnd::SetbPrevVisibleStatus(bool b)
{
	m_bPrevVisibleStatus = b;
}
bool COverlayWnd::GetbPrevVisibleStatus(void)
{
	return m_bPrevVisibleStatus;
}
void COverlayWnd::SethIfExistPrintView(HWND hwnd)
{
	m_hIfExistPrintView = hwnd;
}
/*
	if first enum overlay,set IsUnder flag false, otherwise set true;
*/
BOOL CALLBACK COverlayWnd::EnumFrameProc(HWND hwnd,LPARAM lParam)
{
	JUDELATION* onemember = (JUDELATION*)lParam;
	if(hwnd == onemember->h_Frame)
	{
		onemember->b_OverlayIsUnder = true;
		return false;
	}
	else if(hwnd == onemember->h_Overlay)
	{
		onemember->b_OverlayIsUnder = false;
		return false;
	}
	return true;
}
/*
	judge overlay window whether is under Frame window.
return value: true(overlay window is under of frame window),otherwise return false;
*/
bool COverlayWnd::IsUnderFrame(void)
{
	JUDELATION oneme;
	oneme.h_Overlay = m_hOverlayWnd;
	oneme.h_Frame = m_hForWnd;
	EnumWindows(EnumFrameProc,(LPARAM)&oneme);
	return  oneme.b_OverlayIsUnder;
}


/*
adobe can not get wm_destory message in window process.
so we will check view window is window.
if the view window is not window, we will close overlay window.
*/
bool COverlayWnd::CheckViewToCloseOverlayForAdobe()
{
	bool bRet = false;
	if (m_Auxi.GetProgressType() == ADOBE_ADDIN_TYPE ||m_Auxi.GetProgressType() ==  ADOBE_TYPE)
	{
		HWND hViewWnd  = GethViewWnd();
		
		//screen view  close, adobe does not close.
		WCHAR strWinCaption[MAX_PATH] = {0};
		::GetWindowText(hViewWnd,strWinCaption,MAX_PATH);
		if (_wcsicmp(strWinCaption,L"Full Screen Window") == 0 )	return bRet;
	
		BOOL bViewIsWindow = IsWindow(hViewWnd);
		if (!bViewIsWindow)
		{
			bRet = true;
			//::GetWindowText(m_hForWnd,strWinCaption,MAX_PATH);
			KillTimer(GethOverlayWnd(),NULL);
			PostQuitMessage(NULL);
		}
	}
	return bRet;
}
/*
if the frame window is visible.
the view window is not visible.
we will check other view is or not visible in same frame window.
if all view window is not visible.
we think the adobe is closing document.
so we don't hide overlay.
*/
bool COverlayWnd::CheckAdobeIsClosing()
{
	bool bIsClosing = false;
	if (m_Auxi.GetProgressType() == ADOBE_TYPE)
	{
		HWND hFrameWnd = GethForWnd();
		if (IsWindow(hFrameWnd))
		{
			wchar_t strForeClsName[MAX_PATH]= {0};
			::GetClassName(hFrameWnd,strForeClsName,MAX_PATH);
			if (_wcsicmp(strForeClsName,L"AcrobatSDIWindow") == 0)
			{
				if (IsWindowVisible(hFrameWnd))
				{
					COvelayWndMgr & mgr = COvelayWndMgr::GetInstance();
					bool bIsOneVisible = mgr.CheckAdobeFrameWndHadVisibleView(hFrameWnd);
					if (!bIsOneVisible)
					{
						bIsClosing = true;
						m_bIsAdobeUseFrameSize = true;
					}
				}
			}

		}
	}
	return bIsClosing;
}

/*
bug for 23104
change the view to change the size.
*/
bool COverlayWnd::FullScreenForAdobe3D(HWND hForeWnd)
{
	bool bReturn = false;
	if (hForeWnd == NULL)	return bReturn;
	if ((m_Auxi.GetProgressType() == ADOBE_ADDIN_TYPE || m_Auxi.GetProgressType() == ADOBE_TYPE)
		&&m_Auxi.GetOpenType(m_hForWnd) == IE_OPNE_TYPE)
	{
		WCHAR strTxT[MAX_PATH] = {0};
		::GetWindowText(hForeWnd,strTxT,MAX_PATH);
		if (_wcsicmp(strTxT,L"Full Screen Window") == 0)
		{
				if (SetFullScreenStatus(hForeWnd))
				{
					m_hCacheView = hForeWnd;
					m_overlayInfo.SetoverlayViewMapSize(hForeWnd,true,true);
					bReturn = true;
				}
		}
		else
		{
			if (m_hCacheView != NULL)
			{
				// window is unvisible, the full screen is closed
				BOOL bRet = IsWindowVisible(m_hCacheView);
				if (!bRet)
				{	
					//reback
					m_overlayInfo.SetoverlayViewMapSize(m_hViewWnd,true,true);
					m_hCacheView = NULL;
					bReturn = true;
				}
			}
		}
	}
	return bReturn;
}

bool COverlayWnd::CheckFrameHadShowOverlay(HWND hFrameWnd)
{
	bool bRet = false;
	if (m_hForWnd == hFrameWnd)
	{
		bRet = IsWindowVisible(m_hViewWnd);
	}
	return bRet;
}



bool COverlayWnd::SetFullScreenStatus(HWND hFullScreen)
{
	bool bFullScreenMatchOverlay = false;
	if (hFullScreen != NULL)
	{
		map<HWND,bool>::iterator itor = m_mapIsOverlayFullScreen.find(hFullScreen);
		if (itor == m_mapIsOverlayFullScreen.end())
		{
			//check 
			HWND hFrameWindow = ::GetAncestor(hFullScreen,GA_ROOTOWNER);
			if (hFrameWindow != NULL)
			{
				bFullScreenMatchOverlay = CheckFrameHadShowOverlay(hFrameWindow);
			}
			m_mapIsOverlayFullScreen[hFullScreen] = bFullScreenMatchOverlay;
		}
	}
	return bFullScreenMatchOverlay;
}
void COverlayWnd::OtherLineStringToVecter(const wstring& otherLine,vector<wstring>& vecString)
{
	TextInfo linInfo;
	wstring linString = otherLine;
	while(true)
	{
		m_Auxi.GetUserHostHome(linString,linInfo);
		vecString.push_back(linInfo.strUser);
		if(_wcsicmp(linInfo.strHostHome.c_str(),L"")==0)
			break;
		else
			linString = linInfo.strHostHome;		
	}
}
int COverlayWnd::ComputeWidth(HDC hdc,const wstring& str,int heigth)
{
	RECT rect;
	rect.top=0;rect.left=0;rect.bottom=heigth;rect.right=1;
	DrawText(hdc,str.c_str(),-1,&rect,DT_CALCRECT);
	return static_cast<int>(rect.right-rect.left);
}
int COverlayWnd::ComputeHeith(HDC hdc,const wstring& str,int width)
{
	RECT rect;
	rect.top=0;rect.left=0;rect.bottom=1;rect.right=width;
	DrawText(hdc,str.c_str(),-1,&rect,DT_CALCRECT);
	return static_cast<int>(rect.bottom-rect.top);
}
BOOL COverlayWnd::UpdateDisplayEx(HWND overlay)
{
	HFONT hFont1 = CreateFont(m_dwFirstLineSize,0,0,0,m_bFontBold?FW_BOLD:FW_NORMAL,false,false,false,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH,m_strFont.c_str());
	HFONT hFont2 = CreateFont(m_dwOtherLineSize,0,0,0,m_bFontBold?FW_BOLD:FW_NORMAL,false,false,false,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH,m_strFont.c_str());
	PAINTSTRUCT ps ; 
	HDC hdc = BeginPaint(overlay, &ps) ;
	SetBkMode(hdc,TRANSPARENT);
	SetTextColor(hdc,MYCOLOR(m_dwFontColor));

	int firstLineWidth = 0,firstLineHeith = 0,SecondLineWidth=0,secondLineHeith = 0;
	//get overlay window RECT
	RECT overlayRect;
	GetWindowRect(overlay,&overlayRect);
	int overlayWidth = overlayRect.right - overlayRect.left;
	int overlayHeigth = overlayRect.bottom - overlayRect.top;

	//change  m_stTextInfo.strHostHome to vecter
	vector<wstring> vecOtherLineText;
	OtherLineStringToVecter(m_stTextInfo.strHostHome,vecOtherLineText);

	HGDIOBJ hold = SelectObject(hdc,hFont1);
	firstLineWidth = ComputeWidth(hdc,m_stTextInfo.strUser,overlayHeigth);
	firstLineHeith = ComputeHeith(hdc,m_stTextInfo.strUser,overlayWidth);

	int x = m_dwLeftMargin;
	int y = m_dwTopMargin;
	int maxWidth = firstLineWidth;
	bool bIsNeedComputeMaxWidth = true;
	if(m_bIsRepeat)
	{
		while(y<overlayHeigth)
		{
			x = m_dwLeftMargin;
			while(x<overlayWidth)
			{
				SelectObject(hdc,hFont1);
				TextOut(hdc,x,y,m_stTextInfo.strUser.c_str(),static_cast<int>(m_stTextInfo.strUser.length()));
				y += firstLineHeith;
				SelectObject(hdc,hFont2);
				for(vector<wstring>::const_iterator it = vecOtherLineText.begin();it != vecOtherLineText.end();it++)
				{
					if(bIsNeedComputeMaxWidth)
					{
						SecondLineWidth = ComputeWidth(hdc,*it,overlayHeigth);
						if(secondLineHeith==0)	secondLineHeith = ComputeHeith(hdc,*it,overlayWidth);
						if(SecondLineWidth>maxWidth)	maxWidth = SecondLineWidth;
					}
					TextOut(hdc,x,y,(*it).c_str(),static_cast<int>((*it).length()));
					y += secondLineHeith;		
				}
				bIsNeedComputeMaxWidth = false;
				x += maxWidth + m_dwHorSpace;
				y -= static_cast<int>(firstLineHeith + secondLineHeith * vecOtherLineText.size());
			}
			y += static_cast<int>(firstLineHeith + secondLineHeith * vecOtherLineText.size() + m_dwVerSpace);
		}
	}
	else
	{
		x = (overlayWidth-firstLineWidth)/2;
		y = static_cast<int>((overlayHeigth-firstLineHeith-secondLineHeith * vecOtherLineText.size())/2);
		SelectObject(hdc,hFont1);
		TextOut(hdc,x,y,m_stTextInfo.strUser.c_str(),static_cast<int>(m_stTextInfo.strUser.length()));
		SelectObject(hdc,hFont2);
		y += firstLineHeith;
		for(vector<wstring>::const_iterator it = vecOtherLineText.begin();it != vecOtherLineText.end();it++)
		{
			SecondLineWidth = ComputeWidth(hdc,*it,overlayHeigth);
			x = (overlayWidth - SecondLineWidth)/2;
			if(secondLineHeith==0)	secondLineHeith = ComputeHeith(hdc,*it,overlayWidth);
			TextOut(hdc,x,y,(*it).c_str(),static_cast<int>((*it).length()));
			y += secondLineHeith;
		}
	}
	if(hold)	SelectObject(hdc,hold);
	DeleteObject(hFont1);
	DeleteObject(hFont2);
	EndPaint (overlay, &ps) ;
	return true;
}
BOOL COverlayWnd::UpdateDisplay(void)
{
	RECT viewRect;
	GetWindowRect(m_overlayInfo.GetOIView(),&viewRect);
	DWORD dwWeight = viewRect.right - viewRect.left;
	DWORD dwHeight = viewRect.bottom - viewRect.top;
	SIZE sizeWindow = {dwWeight, dwHeight};
	HDC hDC = ::GetDC(GethOverlayWnd());
	HDC hdcMemory = CreateCompatibleDC(hDC);

	BITMAPINFOHEADER stBmpInfoHeader = { 0 };   
	int nBytesPerLine = ((sizeWindow.cx * 32 + 31) & (~31)) >> 3;
	stBmpInfoHeader.biSize = sizeof(BITMAPINFOHEADER);   
	stBmpInfoHeader.biWidth = sizeWindow.cx;   
	stBmpInfoHeader.biHeight = sizeWindow.cy;   
	stBmpInfoHeader.biPlanes = 1;   
	stBmpInfoHeader.biBitCount = 32;   
	stBmpInfoHeader.biCompression = BI_RGB;   
	stBmpInfoHeader.biClrUsed = 0;   
	stBmpInfoHeader.biSizeImage = nBytesPerLine * sizeWindow.cy;   

	PVOID pvBits = NULL;   
	HBITMAP hbmpMem = ::CreateDIBSection(NULL, (PBITMAPINFO)&stBmpInfoHeader, DIB_RGB_COLORS, &pvBits, NULL, 0);
	if (hbmpMem == NULL)
	{
		::DeleteDC(hdcMemory);
		::ReleaseDC(GethOverlayWnd(), hDC);
		return FALSE;
	}

	memset( pvBits, 0, sizeWindow.cx * 4 * sizeWindow.cy);
	HGDIOBJ hbmpOld = ::SelectObject( hdcMemory, hbmpMem);
	DrawOverlayInfo(hdcMemory);
	POINT ptSrc = { 0, 0};
	BLENDFUNCTION stBlend = {AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};
	::UpdateLayeredWindow(GethOverlayWnd(), hDC, NULL, &sizeWindow, hdcMemory, &ptSrc, 0, &stBlend, ULW_ALPHA);
	::SelectObject( hdcMemory, hbmpOld);
	::DeleteObject(hbmpMem); 
	::DeleteDC(hdcMemory);
	::ReleaseDC(GethOverlayWnd(), hDC);
	return TRUE;
}

void COverlayWnd::ComputeSingleTxTPostion(__in const wstring& strFirstLine,Gdiplus::REAL FirstLineSize,
										  __in const wstring& strOtherLine,Gdiplus::REAL OtherLineSize,
										  __in const StringFormat &strformat,__in const Gdiplus::FontFamily &fontFamily,__in FontStyle emFontStyle,
										  __out vector<pair<wstring,PointF>> &vecOtherLineInfo,__out Gdiplus::REAL &Width,__out Gdiplus::REAL &Height)
{
	GraphicsPath path;
	RectF PathRect1;
	RectF PathRect2;
	PointF origin(0,0);
	size_t lNpos = 0;

	path.AddString(strFirstLine.c_str(),static_cast<int>(strFirstLine.length()),&fontFamily,emFontStyle,FirstLineSize,origin,&strformat);	
	path.GetBounds(&PathRect1);
	path.Reset();

	Width = PathRect1.Width;
	Height = PathRect1.Height;
	origin.Y = PathRect1.Height;

	if (strOtherLine.empty())
	{
		return ;
	}

	wstring strTemp = strOtherLine;
	wstring strPrint = L"";
	for(;;)
	{
		lNpos = strTemp.find(L"\\n");
		if(lNpos == wstring::npos)
		{
			strPrint = strTemp;
			path.AddString(strPrint.c_str(),static_cast<int>(strPrint.length()),&fontFamily,emFontStyle,OtherLineSize,origin,&strformat);
			path.GetBounds(&PathRect2);
			path.Reset();

			if(Width < PathRect2.Width ) Width = PathRect2.Width;
			Height = origin.Y + PathRect2.Height;
			vecOtherLineInfo.push_back(pair<wstring,PointF>(strPrint,origin));
			break;
		}
		else
		{
			strPrint = strTemp.substr(0,lNpos);
			strTemp = strTemp.substr(lNpos + 2);

			path.AddString(strPrint.c_str(),static_cast<int>(strPrint.length()),&fontFamily,emFontStyle,OtherLineSize,origin,&strformat);	
			path.GetBounds(&PathRect2);
			path.Reset();

			if(Width < PathRect2.Width ) Width = PathRect2.Width;
			vecOtherLineInfo.push_back(pair<wstring,PointF>(strPrint,origin));

			origin.Y +=  PathRect2.Height;
		}
	}

}

void COverlayWnd::ComputeTxTOnBitmapPos(__in DWORD bmWidth,__in  DWORD bmHeigh,__in  Gdiplus::REAL TxTWidth,__in  Gdiplus::REAL TxTHeight,__out vector<PointF> &vecTxTPos)
{
	PointF origin;
	DWORD dwLeftPos = m_dwLeftMargin;
	DWORD dwTopPos = m_dwTopMargin; 

	while(dwTopPos < bmHeigh)
	{
		while(dwLeftPos < bmWidth)
		{
			origin.X = static_cast<Gdiplus::REAL>(dwLeftPos);
			origin.Y = static_cast<Gdiplus::REAL>(dwTopPos);
			vecTxTPos.push_back(origin);
			dwLeftPos += static_cast<DWORD>(TxTWidth) + m_dwHorSpace;
		}
		dwLeftPos = m_dwLeftMargin;
		dwTopPos += static_cast<DWORD>(TxTHeight) + m_dwVerSpace;
	}
}

void COverlayWnd::DrawSingleOverlay(__inout Graphics &graphics,__inout GraphicsPath &path,__in PointF Pos,__in const wstring& strFirstLine,__in Gdiplus::REAL FirstLineSize,__in Gdiplus::REAL OtherLineSize,__in const SolidBrush &blackBrush,
									__in const StringFormat &strformat,__in const Gdiplus::FontFamily &fontFamily,__in FontStyle emFontStyle,__in vector<pair<wstring,PointF>> &vecOtherLineInfo)
{

	//first line
	PointF origin(Pos);

	path.AddString(strFirstLine.c_str(),static_cast<int>(strFirstLine.length()),&fontFamily,emFontStyle,FirstLineSize,origin,&strformat);	
	graphics.FillPath(&blackBrush,&path);
	path.Reset();

	//other line
	vector<pair<wstring,PointF>>::iterator itor;
	for (itor = vecOtherLineInfo.begin(); itor != vecOtherLineInfo.end(); itor++)
	{
		origin = itor->second + Pos;
		path.AddString(itor->first.c_str(),static_cast<int>(itor->first.length()),&fontFamily,emFontStyle,OtherLineSize,origin,&strformat);	
		graphics.FillPath(&blackBrush,&path);
		path.Reset();
	}

}

void COverlayWnd::ComputeCenterTxTPos(__in DWORD bmWidth,__in DWORD bmHeigh,__in Gdiplus::REAL TxTWidth,__in Gdiplus::REAL TxTHeight,__out PointF &CenterPos)
{
	CenterPos.X = (bmWidth - TxTWidth)/2;
	CenterPos.Y = (bmHeigh - TxTHeight)/2;
}


OverlayError COverlayWnd::DrawOverlayInfo(__in HDC &hdcMemory)
{
	RECT OverLayRect;
	GetWindowRect(m_overlayInfo.GetOIView(),&OverLayRect);
	DWORD bmWidth = OverLayRect.right - OverLayRect.left;
	DWORD bmHeigh = OverLayRect.bottom - OverLayRect.top;

	Graphics graphics(hdcMemory);
	StringFormat strformat;
	GraphicsPath path;
	graphics.SetSmoothingMode(SmoothingModeHighQuality);


	Gdiplus::REAL FirstLineSize = static_cast<Gdiplus::REAL>(GetdwFirstLineSize());
	Gdiplus::REAL OtherLineSize = static_cast<Gdiplus::REAL>(GetdwOtherLineSize());

	FontStyle emFontStyle = m_bFontBold?FontStyleBold:FontStyleRegular;
	Gdiplus::FontFamily fontFamily(m_strFont.c_str());

	vector<pair<wstring,PointF>> vecOtherLineInfo;
	Gdiplus::REAL TxTWidth = 0;
	Gdiplus::REAL TxTHeight = 0;
	ComputeSingleTxTPostion(m_stTextInfo.strUser,FirstLineSize,m_stTextInfo.strHostHome,OtherLineSize,strformat, fontFamily,emFontStyle,
		vecOtherLineInfo,TxTWidth,TxTHeight);

	SolidBrush blackBrush(m_color);
	if (m_bIsRepeat)
	{
		vector<PointF> vecTxTPos;
		ComputeTxTOnBitmapPos( bmWidth,bmHeigh,TxTWidth,TxTHeight,vecTxTPos);
		vector<PointF>::iterator itor;
		for (itor = vecTxTPos.begin(); itor != vecTxTPos.end(); itor++)
		{
			DrawSingleOverlay(graphics,path,*itor,m_stTextInfo.strUser,FirstLineSize,OtherLineSize,blackBrush,
				strformat,fontFamily,emFontStyle,vecOtherLineInfo);
		}
	}
	else
	{
		PointF CenterPos(0,0);
		ComputeCenterTxTPos(bmWidth,bmHeigh,TxTWidth,TxTHeight,CenterPos);
		DrawSingleOverlay(graphics,path,CenterPos,m_stTextInfo.strUser,FirstLineSize,OtherLineSize,blackBrush,
			strformat,fontFamily,emFontStyle,vecOtherLineInfo);
	}
	graphics.ReleaseHDC(hdcMemory);
	return NLOverlaySuccess;
}
