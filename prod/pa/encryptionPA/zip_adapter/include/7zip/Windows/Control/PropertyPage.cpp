// Windows/Control/PropertyPage.cpp

#include "StdAfx.h"

#include "Windows/Control/PropertyPage.h"
#ifndef _UNICODE
#include "../../Common/StringConvert.h"
#endif

extern HINSTANCE g_hInstance;
#ifndef _UNICODE
extern bool g_IsNT;
#endif

namespace NWindows {
namespace NControl {

INT_PTR APIENTRY ProperyPageProcedure(HWND dialogHWND, UINT message, 
    WPARAM wParam, LPARAM lParam)
{
  CDialog tempDialog(dialogHWND);
  if (message == WM_INITDIALOG)
    tempDialog.SetUserDataLongPtr(((PROPSHEETPAGE *)lParam)->lParam);
  CDialog *dialog = (CDialog *)(tempDialog.GetUserDataLongPtr());
  if (message == WM_INITDIALOG)
    dialog->Attach(dialogHWND);
  switch (message)
  {
    case WM_INITDIALOG:
      return dialog->OnInit();
    case WM_COMMAND:
      return dialog->OnCommand(wParam, lParam);
    case WM_NOTIFY:
      return dialog->OnNotify((UINT)wParam, (LPNMHDR) lParam);
  }
  if (dialog == NULL)
    return false;
  return dialog->OnMessage(message, wParam, lParam);
}

bool CPropertyPage::OnNotify(UINT /* controlID */, LPNMHDR lParam) 
{
  switch(lParam->code)
  {
    case PSN_APPLY:
      SetMsgResult(OnApply(LPPSHNOTIFY(lParam)));
      break;
    case PSN_KILLACTIVE:
      SetMsgResult(BoolToBOOL(OnKillActive(LPPSHNOTIFY(lParam))));
      break;
    case PSN_SETACTIVE:
      SetMsgResult(OnSetActive(LPPSHNOTIFY(lParam)));
      break;
    case PSN_RESET:
      OnReset(LPPSHNOTIFY(lParam));
      break;
    case PSN_HELP:
      OnNotifyHelp(LPPSHNOTIFY(lParam));
      break;
    default:
      return false;
  }
  return true;
}

INT_PTR MyPropertySheet(const CObjectVector<CPageInfo> &pagesInfo, HWND hwndParent, const UString &title)
{
  #ifndef _UNICODE
  AStringVector titles;
  #endif
  #ifndef _UNICODE
  CRecordVector<PROPSHEETPAGEA> pagesA;
  #endif
  CRecordVector<PROPSHEETPAGEW> pagesW;

  int i;
  #ifndef _UNICODE
  for (i = 0; i < pagesInfo.Size(); i++)
    titles.Add(GetSystemString(pagesInfo[i].Title));
  #endif

  for (i = 0; i < pagesInfo.Size(); i++)
  {
    const CPageInfo &pageInfo = pagesInfo[i];
    #ifndef _UNICODE
    {
      PROPSHEETPAGE page;
      page.dwSize = sizeof(page);
      page.dwFlags = PSP_HASHELP;
      page.hInstance = g_hInstance;
      page.pszTemplate = MAKEINTRESOURCE(pageInfo.ID);
      page.pszIcon = NULL;
      page.pfnDlgProc = NWindows::NControl::ProperyPageProcedure;
      
      if (titles[i].IsEmpty())
        page.pszTitle = NULL;
      else
      {
        page.dwFlags |= PSP_USETITLE;
        page.pszTitle = titles[i];
      }
      page.lParam = (LPARAM)pageInfo.Page;
      page.pfnCallback = NULL;
      pagesA.Add(page);
    }
    #endif
    {
      PROPSHEETPAGEW page;
      page.dwSize = sizeof(page);
      page.dwFlags = PSP_HASHELP;
      page.hInstance = g_hInstance;
      page.pszTemplate = MAKEINTRESOURCEW(pageInfo.ID);
      page.pszIcon = NULL;
      page.pfnDlgProc = NWindows::NControl::ProperyPageProcedure;
      
      if (pageInfo.Title.IsEmpty())
        page.pszTitle = NULL;
      else
      {
        page.dwFlags |= PSP_USETITLE;
        page.pszTitle = pageInfo.Title;
      }
      page.lParam = (LPARAM)pageInfo.Page;
      page.pfnCallback = NULL;
      pagesW.Add(page);
    }
  }

  #ifndef _UNICODE
  if (!g_IsNT)
  {
    PROPSHEETHEADER sheet;
    sheet.dwSize = sizeof(sheet);
    sheet.dwFlags = PSH_PROPSHEETPAGE;
    sheet.hwndParent = hwndParent;
    sheet.hInstance = g_hInstance;
    AString titleA = GetSystemString(title);
    sheet.pszCaption = titleA;
    sheet.nPages = pagesInfo.Size();
    sheet.nStartPage = 0;
    sheet.ppsp = &pagesA.Front();
    sheet.pfnCallback = NULL;
    return ::PropertySheetA(&sheet);
  }
  else
  #endif
  {
    PROPSHEETHEADERW sheet;
    sheet.dwSize = sizeof(sheet);
    sheet.dwFlags = PSH_PROPSHEETPAGE;
    sheet.hwndParent = hwndParent;
    sheet.hInstance = g_hInstance;
    sheet.pszCaption = title;
    sheet.nPages = pagesInfo.Size();
    sheet.nStartPage = 0;
    sheet.ppsp = &pagesW.Front();
    sheet.pfnCallback = NULL;
    return ::PropertySheetW(&sheet);
  }
}

}}
