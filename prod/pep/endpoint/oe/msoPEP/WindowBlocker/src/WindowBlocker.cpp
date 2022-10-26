
#include <windows.h>
#include <string>
#pragma warning(push)
#pragma warning(disable: 6334)
#include <boost/algorithm/string.hpp>
#pragma warning(pop)
#include "nlexcept.h"
#pragma warning(disable : 4819)
#include "madCHook_helper.h"


static BOOL Init();
static VOID Clean();

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
    UNREFERENCED_PARAMETER(hModule);
    UNREFERENCED_PARAMETER(lpReserved);
    switch(ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        if(Init()) return FALSE;
        break;

    case DLL_PROCESS_DETACH:
        Clean();
        break;

    default:
        break;
    }

    return TRUE;
}


//
// Local Routines
//
class CPreCheck
{
public:
    CPreCheck()
    {
        WCHAR wzPath[MAX_PATH+1];
        memset(wzPath, 0, sizeof(wzPath));
        GetModuleFileNameW(NULL, wzPath, MAX_PATH);
        m_mod = wzPath;
    }

    ~CPreCheck()
    {
    }

    inline BOOL IsExcel(){return boost::algorithm::iends_with(m_mod, L"EXCEL.EXE");}

private:
    std::wstring    m_mod;
};

/* Exception state.  When 'exception_state' has a non-zero value an exception
* has occurred.
*/
static int exception_state = 0;
static CPreCheck    g_precheck;
static BOOL         g_ishooked = FALSE;

//////////////////////////////////////////////////////////////////////////
static int (WINAPI *NextDialogBoxIndirectParamW)(HINSTANCE hInstance, LPCDLGTEMPLATE hDialogTemplate, HWND hWndParent, DLGPROC lpDialogFunc, LPARAM dwInitParam)=0;
static int WINAPI try_NLDialogBoxIndirectParamW(HINSTANCE hInstance, LPCDLGTEMPLATE hDialogTemplate, HWND hWndParent, DLGPROC lpDialogFunc, LPARAM dwInitParam);
static int WINAPI NLDialogBoxIndirectParamW(HINSTANCE hInstance, LPCDLGTEMPLATE hDialogTemplate, HWND hWndParent, DLGPROC lpDialogFunc, LPARAM dwInitParam);

// Function to close top dialogbox
static LRESULT CloseExcelWarningDialog(LPARAM lParam);


//////////////////////////////////////////////////////////////////////////
BOOL Init()
{
    if(g_ishooked)
        return g_ishooked;
    if(g_precheck.IsExcel())
        return FALSE;

    __try
	{
        g_ishooked = HookAPI("User32.dll", "DialogBoxIndirectParamW", (PVOID)try_NLDialogBoxIndirectParamW, (PVOID*)&NextDialogBoxIndirectParamW, 0);
    }
	__except( NLEXCEPT_FILTER() )
	{
		/* empty */
        ;
	}

    return g_ishooked;
}

VOID Clean()
{
    if(g_ishooked)
    {
        if(UnhookAPI((PVOID*)&NextDialogBoxIndirectParamW)) g_ishooked = FALSE;
    }
}

int  WINAPI try_NLDialogBoxIndirectParamW(HINSTANCE hInstance, LPCDLGTEMPLATE hDialogTemplate, HWND hWndParent, DLGPROC lpDialogFunc, LPARAM dwInitParam)
{
	__try
	{
		return NLDialogBoxIndirectParamW(hInstance,hDialogTemplate,hWndParent,lpDialogFunc,dwInitParam);
	}
	__except( NLEXCEPT_FILTER_EX(&exception_state) )
	{
		/* empty */
        ;
	}
	return -1; /* fail */
}

int  WINAPI NLDialogBoxIndirectParamW(HINSTANCE hInstance, LPCDLGTEMPLATE hDialogTemplate, HWND hWndParent, DLGPROC lpDialogFunc, LPARAM dwInitParam)
{
    DLGTEMPLATE* pDlgTemplate   = (DLGTEMPLATE*)hDialogTemplate;

    if( NULL != pDlgTemplate
        && 0xFFFF0001 == pDlgTemplate->style
        && 0 == pDlgTemplate->dwExtendedStyle
        && 0 == pDlgTemplate->cdit
        && 0 == pDlgTemplate->x
        && 449 == pDlgTemplate->y
        && (5 == pDlgTemplate->cy || 4 == pDlgTemplate->cy)
        )
    {
        HANDLE hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)CloseExcelWarningDialog, 0, 0, NULL);
        if(NULL!=hThread) CloseHandle(hThread); hThread = NULL;
    }
    
    return NextDialogBoxIndirectParamW(hInstance, hDialogTemplate, hWndParent, lpDialogFunc, dwInitParam);
}

LRESULT CloseExcelWarningDialog(LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);

    for(int i=0; i<30; i++)
    {
        HWND hWnd = NULL;

        Sleep(50);
        hWnd = ::FindWindow(MAKEINTRESOURCEW(32770), L"Microsoft Excel");                       // For office 2003
        if(NULL==hWnd) hWnd = ::FindWindow(MAKEINTRESOURCEW(32770), L"Microsoft Office Excel"); // For office 2007
        if(NULL != hWnd)
        {
            HWND hWndOK = ::FindWindowExW(hWnd, NULL, NULL, L"OK");
            if(hWndOK)
            {
                UINT   uId    = GetDlgCtrlID(hWndOK);
                WPARAM wParam = (BN_CLICKED<<16)|(uId&0x0000FFFF);
                SendMessage(hWnd, WM_COMMAND, wParam, (LPARAM)hWndOK);
                return 0L;
            }
        }
    }
    return 0L;
}
