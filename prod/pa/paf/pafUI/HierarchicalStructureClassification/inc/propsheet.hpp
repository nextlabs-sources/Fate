

#ifndef __NXRM_COMMONUI_PROPSHEET_HPP__
#define __NXRM_COMMONUI_PROPSHEET_HPP__

#include <Windows.h>
#include <Prsht.h>


class CPropSheet
{
public:
    CPropSheet();
    virtual ~CPropSheet();

protected:
    virtual INT_PTR Create(_In_opt_ HWND hParent, _In_opt_ LPCWSTR wzIcon, _In_opt_ LPCWSTR wzCaption, _In_ LPCPROPSHEETPAGEW pPages, _In_ UINT nPages, _In_ UINT nStartPage);

private:
    PROPSHEETHEADERW    _psh;
    HWND                _pshwnd;
};


#endif