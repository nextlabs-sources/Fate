#include <Windows.h>
#include <WindowSX.h>
#include <UxTheme.h>

#ifndef _GSCONTROLS_H_INCLUDED
#define _GSCONTROLS_H_INCLUDED

struct _ThemeHelper
{
	typedef BOOL(__stdcall *PFNISAPPTHEMED)();
	typedef HTHEME(__stdcall *PFNOPENTHEMEDATA)(HWND hwnd, LPCWSTR pszClassList);
	typedef HRESULT(__stdcall *PFNCLOSETHEMEDATA)(HTHEME hTheme);
	typedef HRESULT(__stdcall *PFNDRAWTHEMEBACKGROUND)(HTHEME hTheme, HDC hdc, 
		int iPartId, int iStateId, const RECT *pRect, OPTIONAL const RECT *pClipRect);
	typedef HRESULT(__stdcall *PFNGETTHEMEPARTSIZE)(HTHEME hTheme, HDC hdc, 
		int iPartId, int iStateId, RECT * pRect, enum THEMESIZE eSize, OUT SIZE *psz);
	typedef HRESULT(__stdcall *PFNGETTHEMEBACKGROUNDCONTENTRECT)(HTHEME hTheme, HDC hdc, 
		int iPartId, int iStateId, const RECT *pBoundingRect, RECT *pContentRect);

	static void* GetProc(LPCSTR szProc, void* pfnFail)
	{
		static HMODULE hThemeDll = LoadLibrary(TEXT("UxTheme.dll"));

		void* pRet = pfnFail;
		if (hThemeDll != NULL)
		{
			pRet = GetProcAddress(hThemeDll, szProc);
		}
		return pRet;
	}
	//IsAppThemed
	static BOOL __stdcall IsAppThemedFail()
	{
		return FALSE;
	}
	static BOOL IsAppThemed()
	{
		static PFNISAPPTHEMED pfnIsAppThemed = (PFNISAPPTHEMED)GetProc("IsAppThemed", IsAppThemedFail);
		return (*pfnIsAppThemed)();
	}
	//OpenThemeData
	static HTHEME __stdcall OpenThemeDataFail(HWND , LPCWSTR )
	{
		return NULL;
	}
	static HTHEME OpenThemeData(HWND hwnd, LPCWSTR pszClassList)
	{
		static PFNOPENTHEMEDATA pfnOpenThemeData = (PFNOPENTHEMEDATA)GetProc("OpenThemeData", OpenThemeDataFail);
		return (*pfnOpenThemeData)(hwnd, pszClassList);
	}
	//CloseThemeData
	static HRESULT __stdcall CloseThemeDataFail(HTHEME)
	{
		return E_FAIL;
	}
	static HRESULT CloseThemeData(HTHEME hTheme)
	{
		static PFNCLOSETHEMEDATA pfnCloseThemeData = (PFNCLOSETHEMEDATA)GetProc("CloseThemeData", CloseThemeDataFail);
		return (*pfnCloseThemeData)(hTheme);
	}
	//DrawThemeBackground
	static HRESULT __stdcall DrawThemeBackgroundFail(HTHEME, HDC, int, int, const RECT *, const RECT *)
	{
		return E_FAIL;
	}
	static HRESULT DrawThemeBackground(HTHEME hTheme, HDC hdc, 
		int iPartId, int iStateId, const RECT *pRect, const RECT *pClipRect)
	{
		static PFNDRAWTHEMEBACKGROUND pfnDrawThemeBackground = 
			(PFNDRAWTHEMEBACKGROUND)GetProc("DrawThemeBackground", DrawThemeBackgroundFail);
		return (*pfnDrawThemeBackground)(hTheme, hdc, iPartId, iStateId, pRect, pClipRect);
	}
	//GetThemePartSize
	static HRESULT __stdcall GetThemePartSizeFail(HTHEME, HDC, int, int, RECT *, enum THEMESIZE, SIZE *)
	{
		return E_FAIL;
	}
	static HRESULT GetThemePartSize(HTHEME hTheme, HDC hdc, 
		int iPartId, int iStateId, RECT * pRect, enum THEMESIZE eSize, SIZE *psz)
	{
		static PFNGETTHEMEPARTSIZE pfnGetThemePartSize = 
			(PFNGETTHEMEPARTSIZE)GetProc("GetThemePartSize", GetThemePartSizeFail);
		return (*pfnGetThemePartSize)(hTheme, hdc, iPartId, iStateId, pRect, eSize, psz);
	}
	//GetThemeBackgroundContentRect
	static HRESULT __stdcall GetThemeBackgroundContentRectFail(HTHEME, HDC, int, int, const RECT *, RECT *)
	{
		return E_FAIL;
	}
	static HRESULT GetThemeBackgroundContentRect(HTHEME hTheme, HDC hdc, 
		int iPartId, int iStateId, const RECT *pBoundingRect, RECT *pContentRect)
	{
		static PFNGETTHEMEBACKGROUNDCONTENTRECT pfnGetThemeBackgroundContentRect = 
			(PFNGETTHEMEBACKGROUNDCONTENTRECT)GetProc("GetThemeBackgroundContentRect", 
			GetThemeBackgroundContentRectFail);
		return (*pfnGetThemeBackgroundContentRect)(hTheme, hdc, iPartId, iStateId, pBoundingRect, pContentRect);
	}
};

/******************************* Control: NonProgressBar *******************************/
#define NONPROGRESS_CLASSA	"gs_nonprogressbar"
#define NONPROGRESS_CLASSW	L"gs_nonprogressbar"
#ifdef _UNICODE
#define NONPROGRESS_CLASS	NONPROGRESS_CLASSW
#else
#define NONPROGRESS_CLASS	NONPROGRESS_CLASSA
#endif

//NonProgressBar - Private Data Struct
typedef struct tagNPBDATA
{
	UINT	uAnimationValue;
	UINT	uSpeed;
	UINT	uNumberOfDots;
} NPBDATA, *LPNPBDATA;

//NonProgressBar - Messages
#define NPM_GETSPEED			WM_USER + 0
#define NPM_SETSPEED			WM_USER + 1
#define NPM_GETNUMBEROFDOTS		WM_USER + 2
#define NPM_SETNUMBEROFDOTS		WM_USER + 3

//NonProgressBar - Styles
#define NPS_NOANIMATED			WS_DISABLED
#define NPS_NOALLOWTHEMING		0x00008000L

//NonProgressBar - Macros
#define NonProgressBar_Create(hwndparent, doani, x, y, width, height)	CreateWindow(NONPROGRESS_CLASS, NULL, WS_CHILD|WS_VISIBLE|(doani ? 0 : WS_DISABLED), (int) x, (int) y, (int) width, (int) height, (HWND) hwndparent, NULL, NULL, NULL)
#define NonProgressBar_GetSpeed(hwnd)									(UINT) SendMessage(hwnd, NPM_GETSPEED, NULL, NULL)
#define NonProgressBar_SetSpeed(hwnd, milliseconds)						SendMessage(hwnd, NPM_SETSPEED, (WPARAM) milliseconds, NULL)
#define NonProgressBar_GetNumberOfDots(hwnd)							(UINT) SendMessage(hwnd, NPM_GETNUMBEROFDOTS, NULL, NULL)
#define NonProgressBar_SetNumberOfDots(hwnd, dots)						SendMessage(hwnd, NPM_SETNUMBEROFDOTS, (WPARAM) dots, NULL)
#define NonProgressBar_IsAnimationEnabled(hwnd)							IsWindowEnabled(hwnd)
#define NonProgressBar_EnableAnimation(hwnd, enable)					EnableWindow(hwnd, (BOOL) enable)

//NonProgressBar - Default Message Proc
LRESULT CALLBACK DefNonProgressBarCtrlProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LPNPBDATA		lpData;
	//WM_PAINT
    RECT			rc1, rc2;
	PAINTSTRUCT		ps;
	HTHEME			hTheme;
	SIZE			size;
	UINT			cont0;
	
	lpData = (LPNPBDATA) GetWindowLongPtr(hWnd, GWLP_USERDATA);
	switch (uMsg)
	{
	case WM_CREATE:
		lpData = new NPBDATA;
		SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR) lpData);
		NonProgressBar_SetNumberOfDots(hWnd, 5);
		NonProgressBar_SetSpeed(hWnd, 60);
		break;

	case WM_DESTROY:
		lpData = (LPNPBDATA) SetWindowLongPtr(hWnd, GWLP_USERDATA, NULL);
		delete lpData;
		break;

	case WM_PAINT:
		GetClientRect(hWnd, &rc1);
		BeginPaint(hWnd, &ps);
		hTheme = _ThemeHelper::OpenThemeData(hWnd, L"Progress");
		if ((hTheme)&&(_ThemeHelper::IsAppThemed())&&(!(GetWindowStyle(hWnd)&NPS_NOALLOWTHEMING)))
		{
			//Theme enabled drawing
			_ThemeHelper::DrawThemeBackground(hTheme, ps.hdc, 1, 1, &rc1, &ps.rcPaint);
			_ThemeHelper::GetThemeBackgroundContentRect(hTheme, ps.hdc, 1, 1, &rc1, &rc2);
			CopyRect(&rc1, &rc2);
			_ThemeHelper::GetThemePartSize(hTheme, ps.hdc, 3, 1, &rc2, TS_MIN, &size);
			SetRect(&rc2, rc1.left + 
				((size.cx + 2) * ((lpData->uAnimationValue - 1) * ((rc1.right - rc1.left) / size.cx) / 100)), 
				rc1.top, rc1.right, rc1.bottom);
			for (cont0=1; cont0<=lpData->uNumberOfDots; cont0++)
			{
				rc2.right = rc2.left + size.cx;
				if (rc2.right>rc1.right) {rc2.right = rc1.right;}
				if ((rc2.left>=rc1.left)&&(rc2.left<=rc1.right))
				{
					_ThemeHelper::DrawThemeBackground(hTheme, ps.hdc, 3, 1, &rc2, &ps.rcPaint);
				}
				rc2.left = rc2.left - (size.cx + 2);
			}
		}
		else
		{
			//Normal drawing
			FillRect(ps.hdc, &rc1, (HBRUSH) (COLOR_BTNFACE+1));
			DrawEdge(ps.hdc, &rc1, BDR_SUNKENOUTER, BF_RECT);
			SetRect(&rc1, rc1.left + 2, rc1.top + 2, rc1.right - 2, rc1.bottom - 2);
			size.cx = (rc1.bottom - rc1.top) * 3/4;
			SetRect(&rc2, rc1.left + ((size.cx + 2) * 
				((lpData->uAnimationValue - 1) * ((rc1.right - rc1.left) / size.cx) / 100)), 
				rc1.top, rc1.right, rc1.bottom);
			for (cont0=1; cont0<=lpData->uNumberOfDots; cont0++)
			{
				rc2.right = rc2.left + size.cx;
				if (rc2.right>rc1.right) {rc2.right = rc1.right;}
				if ((rc2.left>=rc1.left)&&(rc2.left<=rc1.right))
				{
					FillRect(ps.hdc, &rc2, (HBRUSH) (COLOR_HIGHLIGHT+1));
				}
				rc2.left = rc2.left - (size.cx + 2);
			}
		}
		EndPaint(hWnd, &ps);
		//Vuelve el contador a 0 una vez terminada la animación
		if (rc2.left > (int) (rc1.right + (size.cx + 2) * (10 + lpData->uNumberOfDots * 2)))
		{
			lpData->uAnimationValue = 0;
		}
		return 0;

	case WM_ENABLE:
		if (wParam) {SetTimer(hWnd, 0, lpData->uSpeed, NULL);}
		else
		{
			KillTimer(hWnd, 0);
			lpData->uAnimationValue = 0;
			RedrawWindow(hWnd, NULL, NULL, RDW_UPDATENOW|RDW_INVALIDATE);
		}
		break;
	
	case WM_TIMER:
		lpData->uAnimationValue++;
		RedrawWindow(hWnd, NULL, NULL, RDW_UPDATENOW|RDW_INVALIDATE);
		return TRUE;

	case NPM_GETSPEED:
		return lpData->uSpeed;

	case NPM_SETSPEED:
		lpData->uSpeed = (UINT) wParam;
		if (NonProgressBar_IsAnimationEnabled(hWnd)) {SendMessage(hWnd, WM_ENABLE, TRUE, NULL);}
		return TRUE;

	case NPM_GETNUMBEROFDOTS:
		return lpData->uNumberOfDots;

	case NPM_SETNUMBEROFDOTS:
		if ((wParam<1)||(wParam)>20) {return FALSE;}
		lpData->uNumberOfDots = (UINT) wParam;
		return TRUE;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

// ******************************* Common Initializacion Functions *******************************/
void CALLBACK InitializeGSControls()
{
	WNDCLASSEX wNonProgClass = {sizeof WNDCLASSEX, CS_GLOBALCLASS|CS_HREDRAW|CS_VREDRAW, 
		(WNDPROC) DefNonProgressBarCtrlProc, 0, 0, GetModuleHandle(NULL), NULL, NULL, NULL, NULL,
		NONPROGRESS_CLASS, NULL};
	RegisterClassEx(&wNonProgClass);
}

void CALLBACK UninitializeGSControls()
{
	HMODULE hModule = GetModuleHandle(NULL);
	if(hModule)
	{
		UnregisterClass(NONPROGRESS_CLASS, hModule);
	}
}

#endif //_GSCONTROLS_H_INCLUDED
