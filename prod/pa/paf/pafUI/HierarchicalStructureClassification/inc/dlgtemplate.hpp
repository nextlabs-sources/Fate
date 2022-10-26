

#ifndef __NXRM_COMMONUI_DLGTEMPLATE_HPP__
#define __NXRM_COMMONUI_DLGTEMPLATE_HPP__

#ifndef WINVER				// Allow use of features specific to Windows XP or later.
#define WINVER 0x0600		// Change this to the appropriate value to target other versions of Windows.
#endif
#include <Windows.h>
#include <prsht.h>


class CDlgTemplate
{
public:
    CDlgTemplate();
    CDlgTemplate(_In_ UINT uDlgId);
    virtual ~CDlgTemplate();

    inline BOOL AutoRelease() const throw() {return _AutoRelease;}
    inline HWND GetHwnd() throw() {return _hWnd;}

    int DoModal(_In_opt_ HWND hParent);

    virtual DLGPROC GetDlgProc() throw() {return CDlgTemplate::DlgProc;}
    virtual BOOL OnInitialize() {return TRUE;}
    virtual void OnOk() {_nResult=IDOK;::EndDialog(_hWnd, IDOK);}
    virtual void OnCancel() {_nResult=IDCANCEL;::EndDialog(_hWnd, IDCANCEL);}
    virtual void OnDestroy() {}
    virtual BOOL OnClose() {return FALSE;}
    virtual BOOL OnCommand(WORD notify, WORD id, HWND hwnd) {UNREFERENCED_PARAMETER(notify);UNREFERENCED_PARAMETER(id);UNREFERENCED_PARAMETER(hwnd);return FALSE;}
    virtual BOOL OnSysCommand(WPARAM wParam, LPARAM lParam) {UNREFERENCED_PARAMETER(wParam);UNREFERENCED_PARAMETER(lParam);return FALSE;}
    virtual BOOL OnTimer(_In_ UINT id, _In_opt_ TIMERPROC proc) {UNREFERENCED_PARAMETER(id);UNREFERENCED_PARAMETER(proc);return FALSE;}
    virtual BOOL OnHelp(_In_ LPHELPINFO info) {UNREFERENCED_PARAMETER(info);return FALSE;}
    virtual BOOL OnNotify(_In_ LPNMHDR lpnmhdr) {UNREFERENCED_PARAMETER(lpnmhdr);return FALSE;}
    
    virtual BOOL OnLButtonDown(WPARAM wParam, int x, int y) {UNREFERENCED_PARAMETER(wParam);UNREFERENCED_PARAMETER(x);UNREFERENCED_PARAMETER(y);return FALSE;}
    virtual BOOL OnLButtonUp(WPARAM wParam, int x, int y) {UNREFERENCED_PARAMETER(wParam);UNREFERENCED_PARAMETER(x);UNREFERENCED_PARAMETER(y);return FALSE;}
    virtual BOOL OnLButtonDblClk(WPARAM wParam, int x, int y) {UNREFERENCED_PARAMETER(wParam);UNREFERENCED_PARAMETER(x);UNREFERENCED_PARAMETER(y);return FALSE;}
    virtual BOOL OnRButtonDown(WPARAM wParam, int x, int y) {UNREFERENCED_PARAMETER(wParam);UNREFERENCED_PARAMETER(x);UNREFERENCED_PARAMETER(y);return FALSE;}
    virtual BOOL OnRButtonUp(WPARAM wParam, int x, int y) {UNREFERENCED_PARAMETER(wParam);UNREFERENCED_PARAMETER(x);UNREFERENCED_PARAMETER(y);return FALSE;}
    virtual BOOL OnRButtonDblClk(WPARAM wParam, int x, int y) {UNREFERENCED_PARAMETER(wParam);UNREFERENCED_PARAMETER(x);UNREFERENCED_PARAMETER(y);return FALSE;}
    virtual BOOL OnMButtonDown(WPARAM wParam, int x, int y) {UNREFERENCED_PARAMETER(wParam);UNREFERENCED_PARAMETER(x);UNREFERENCED_PARAMETER(y);return FALSE;}
    virtual BOOL OnMButtonUp(WPARAM wParam, int x, int y) {UNREFERENCED_PARAMETER(wParam);UNREFERENCED_PARAMETER(x);UNREFERENCED_PARAMETER(y);return FALSE;}
    virtual BOOL OnMButtonDblClk(WPARAM wParam, int x, int y) {UNREFERENCED_PARAMETER(wParam);UNREFERENCED_PARAMETER(x);UNREFERENCED_PARAMETER(y);return FALSE;}

    virtual BOOL OnPaint() {return FALSE;}
    virtual BOOL OnEraseBkGnd(HDC hDC) {UNREFERENCED_PARAMETER(hDC);return FALSE;}
    virtual BOOL OnGetText(WCHAR* text, UINT size) {UNREFERENCED_PARAMETER(text);UNREFERENCED_PARAMETER(size);return FALSE;}
    virtual BOOL OnSetText(const WCHAR* text) {UNREFERENCED_PARAMETER(text);return FALSE;}

    virtual BOOL OnOwnerDraw(DRAWITEMSTRUCT* lpDrawItemStruct) {UNREFERENCED_PARAMETER(lpDrawItemStruct);return FALSE;}
    virtual BOOL OnMeasureItem(MEASUREITEMSTRUCT* lpMeasureItemStruct) {UNREFERENCED_PARAMETER(lpMeasureItemStruct);return FALSE;}

    virtual BOOL OnNcPaint(HRGN hRgn) {UNREFERENCED_PARAMETER(hRgn);return FALSE;}
    virtual LRESULT OnNcHitTest(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {return DefWindowProcW(hDlg, uMsg, wParam, lParam);}

protected:
    virtual INT_PTR MessageHandler(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    inline void SetHwnd(_In_ HWND hWnd) throw() {_hWnd = hWnd;}
    static INT_PTR WINAPI DlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    
private:
    HWND _hWnd;
    UINT _dlgId;
    BOOL _AutoRelease;
    int  _nResult;
};

class CPropPageDlgTemplate : public CDlgTemplate
{
public:
    CPropPageDlgTemplate() : CDlgTemplate() {memset(&_psp, 0, sizeof(_psp)); _psp.dwSize=sizeof(PROPSHEETPAGEW);}
    CPropPageDlgTemplate(BOOL AutoRelease) : CDlgTemplate(AutoRelease) {memset(&_psp, 0, sizeof(_psp)); _psp.dwSize=sizeof(PROPSHEETPAGEW);}
    virtual ~CPropPageDlgTemplate() {}
    virtual DLGPROC GetDlgProc() throw() {return CPropPageDlgTemplate::PropPageDlgProc;}

    virtual void OnPsnSetActive(_In_ LPPSHNOTIFY lppsn){};
    virtual void OnPsnKillActive(_In_ LPPSHNOTIFY lppsn){};
    virtual void OnPsnApply(_In_ LPPSHNOTIFY lppsn){};
    virtual void OnPsnReset(_In_ LPPSHNOTIFY lppsn){};
    virtual void OnPsnHelp(_In_ LPPSHNOTIFY lppsn){};
    virtual void OnPsnWizBack(_In_ LPPSHNOTIFY lppsn){};
    virtual void OnPsnWizNext(_In_ LPPSHNOTIFY lppsn){};
    virtual void OnPsnWizFinish(_In_ LPPSHNOTIFY lppsn){};
    virtual void OnPsnQueryCancel(_In_ LPPSHNOTIFY lppsn){};
    virtual void OnPsnGetObject(_In_ LPPSHNOTIFY lppsn){};
    virtual void OnPsnTranslateAccelerator(_In_ LPPSHNOTIFY lppsn){};
    virtual void OnPsnQueryInitialFocus(_In_ LPPSHNOTIFY lppsn){};

    operator PROPSHEETPAGEW& () throw() {return _psp;}
    operator const PROPSHEETPAGEW& () const throw() {return _psp;}
    const PROPSHEETPAGEW& GetPage() const throw() {return _psp;}
    PROPSHEETPAGEW& GetPage() throw() {return _psp;}

protected:
    virtual INT_PTR MessageHandler(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    static INT_PTR WINAPI PropPageDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

protected:    
    PROPSHEETPAGEW _psp; 
};


#endif