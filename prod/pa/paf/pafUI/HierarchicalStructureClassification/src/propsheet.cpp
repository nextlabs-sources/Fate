

#include <Windows.h>
#include <assert.h>

#include <list>

#include "commonui.hpp"
#include "dlgtemplate.hpp"
#include "propsheet.hpp"

extern HINSTANCE g_hInst;

CPropSheet::CPropSheet() : _pshwnd(NULL)
{
    memset(&_psh, 0, sizeof(_psh));
    _psh.dwSize = sizeof(PROPSHEETHEADERW);
}

CPropSheet::~CPropSheet()
{
}

INT_PTR CPropSheet::Create(_In_opt_ HWND hParent, _In_opt_ LPCWSTR wzIcon, _In_opt_ LPCWSTR wzCaption, _In_ LPCPROPSHEETPAGEW pPages, _In_ UINT nPages, _In_ UINT nStartPage)
{
    _pshwnd = NULL;
    memset(&_psh, 0, sizeof(_psh));
    _psh.dwSize = sizeof(PROPSHEETHEADERW);
    _psh.dwFlags = PSH_PROPSHEETPAGE | PSH_NOCONTEXTHELP | PSH_PROPTITLE;
    _psh.hwndParent = hParent;
    _psh.hInstance  = g_hInst;
    if(NULL != wzCaption && L'\0' != wzCaption[0]) {
        _psh.dwFlags |= PSH_PROPTITLE;
        _psh.pszCaption = wzCaption;
    }
    if(NULL != wzIcon && L'\0' != wzIcon[0]) {
        _psh.dwFlags |= PSH_USEICONID;
        _psh.pszIcon    = wzIcon;
    }
    _psh.nPages      = nPages;
    _psh.ppsp        = pPages;
    if(nStartPage >= nPages) {
        nStartPage = 0;
    }
    _psh.nStartPage  = nStartPage;

    return PropertySheetW(&_psh);
}