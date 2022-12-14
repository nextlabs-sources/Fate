// MyLoadMenu

#include "StdAfx.h"

#include "Common/StringConvert.h"

#include "Windows/Menu.h"
#include "Windows/Error.h"
#include "Windows/Clipboard.h"

#include "../../PropID.h"

#include "resource.h"
#include "App.h"
#include "AboutDialog.h"
#include "../Common/CompressCall.h"

#include "HelpUtils.h"
#include "LangUtils.h"
#include "PluginInterface.h"

static const UINT kOpenBookmarkMenuID = 730;
static const UINT kSetBookmarkMenuID = 740;

extern HINSTANCE g_hInstance;

static LPCWSTR kFMHelpTopic = L"FM/index.htm";

extern void OptionsDialog(HWND hwndOwner, HINSTANCE hInstance);

using namespace NWindows;

static const int kFileMenuIndex = 0;
static const int kEditMenuIndex = 1;
static const int kViewMenuIndex = 2;
static const int kBookmarksMenuIndex = kViewMenuIndex + 1;

struct CStringLangPair
{
  wchar_t *String;
  UINT32 LangID;
};

static CStringLangPair kStringLangPairs[] = 
{
  { L"&File",  0x03000102 },
  { L"&Edit",  0x03000103 },
  { L"&View",  0x03000104 },
  { L"&Bookmarks", 0x03000107 },
  { L"&Tools", 0x03000105 },
  { L"&Help",  0x03000106 },
};

UINT32 kAddToFavoritesLangID = 0x03000710;
UINT32 kToolbarsLangID = 0x03000451;

/*
static int FindStringLangItem(const UString &anItem)
{
  for (int i = 0; i < sizeof(kStringLangPairs) / 
      sizeof(kStringLangPairs[0]); i++)
    if (anItem.CompareNoCase(kStringLangPairs[i].String) == 0)
      return i;
  return -1;
}
*/

static CIDLangPair kIDLangPairs[] = 
{
  // File
  { IDM_FILE_OPEN, 0x03000210 },
  { IDM_FILE_OPEN_INSIDE, 0x03000211 },
  { IDM_FILE_OPEN_OUTSIDE, 0x03000212 },
  { IDM_FILE_VIEW, 0x03000220 },
  { IDM_FILE_EDIT, 0x03000221 },
  { IDM_RENAME, 0x03000230 },
  { IDM_COPY_TO, 0x03000231 },
  { IDM_MOVE_TO, 0x03000232 },
  { IDM_DELETE, 0x03000233 },
  { IDM_FILE_PROPERTIES, 0x03000240 },
  { IDM_FILE_COMMENT, 0x03000241 },
  { IDM_FILE_CRC, 0x03000242 },
  { IDM_FILE_SPLIT, 0x03000270 },
  { IDM_FILE_COMBINE, 0x03000271 },
  { IDM_CREATE_FOLDER, 0x03000250 },
  { IDM_CREATE_FILE, 0x03000251 },
  { IDCLOSE, 0x03000260 },

  // Edit
  { IDM_EDIT_CUT, 0x03000320 },
  { IDM_EDIT_COPY, 0x03000321 },
  { IDM_EDIT_PASTE, 0x03000322 },

  { IDM_SELECT_ALL, 0x03000330 },
  { IDM_DESELECT_ALL, 0x03000331 },
  { IDM_INVERT_SELECTION, 0x03000332 },
  { IDM_SELECT, 0x03000333 },
  { IDM_DESELECT, 0x03000334 },
  { IDM_SELECT_BY_TYPE, 0x03000335 },
  { IDM_DESELECT_BY_TYPE, 0x03000336 },

  { IDM_VIEW_LARGE_ICONS, 0x03000410 },
  { IDM_VIEW_SMALL_ICONS, 0x03000411 },
  { IDM_VIEW_LIST, 0x03000412 },
  { IDM_VIEW_DETAILS, 0x03000413 },

  { IDM_VIEW_ARANGE_BY_NAME, 0x02000204 },
  { IDM_VIEW_ARANGE_BY_TYPE, 0x02000214 },
  { IDM_VIEW_ARANGE_BY_DATE, 0x0200020C },
  { IDM_VIEW_ARANGE_BY_SIZE, 0x02000207 },
  { IDM_VIEW_ARANGE_NO_SORT, 0x03000420 },

  { IDM_OPEN_ROOT_FOLDER, 0x03000430 },
  { IDM_OPEN_PARENT_FOLDER, 0x03000431 },
  { IDM_FOLDERS_HISTORY, 0x03000432 },

  { IDM_VIEW_REFRESH, 0x03000440 },
  
  { IDM_VIEW_FLAT_VIEW, 0x03000449 },
  { IDM_VIEW_TWO_PANELS, 0x03000450 },
  { IDM_VIEW_ARCHIVE_TOOLBAR, 0x03000460 },
  { IDM_VIEW_STANDARD_TOOLBAR, 0x03000461 },
  { IDM_VIEW_TOOLBARS_LARGE_BUTTONS, 0x03000462 },
  { IDM_VIEW_TOOLBARS_SHOW_BUTTONS_TEXT, 0x03000463 },

  { IDM_OPTIONS, 0x03000510 },
  { IDM_BENCHMARK, 0x03000511 },
  
  { IDM_HELP_CONTENTS, 0x03000610 },
  { IDM_ABOUT, 0x03000620 }
};


static int FindLangItem(int ControlID)
{
  for (int i = 0; i < sizeof(kIDLangPairs) / sizeof(kIDLangPairs[0]); i++)
    if (kIDLangPairs[i].ControlID == ControlID)
      return i;
  return -1;
}


/*
static bool g_IsNew_fMask = true;

class CInit_fMask
{
public:
  CInit_fMask()
  {
    g_IsNew_fMask = false;
    OSVERSIONINFO vi;
    vi.dwOSVersionInfoSize = sizeof(vi);
    if (::GetVersionEx(&vi)) 
    {
      g_IsNew_fMask = (vi.dwMajorVersion > 4 || 
        (vi.dwMajorVersion == 4 && vi.dwMinorVersion > 0));
    }
    g_IsNew_fMask = false;
  }
} g_Init_fMask;

// it's hack for supporting Windows NT
// constants are from WinUser.h

#if(WINVER < 0x0500)
#define MIIM_STRING      0x00000040
#define MIIM_BITMAP      0x00000080
#define MIIM_FTYPE       0x00000100
#endif

static UINT Get_fMaskForString()
{
  return g_IsNew_fMask ? MIIM_STRING : MIIM_TYPE;
}

static UINT Get_fMaskForFTypeAndString()
{
  return g_IsNew_fMask ? (MIIM_STRING | MIIM_FTYPE) : MIIM_TYPE;
}
*/

static UINT Get_fMaskForString()
{
  return MIIM_TYPE;
}

static UINT Get_fMaskForFTypeAndString()
{
  return MIIM_TYPE;
}



static void MyChangeMenu(HMENU menuLoc, int level, int menuIndex)
{
  CMenu menu;
  menu.Attach(menuLoc);
  for (int i = 0; i < menu.GetItemCount(); i++)
  {
    CMenuItem item;
    item.fMask = Get_fMaskForString() | MIIM_SUBMENU | MIIM_ID;
    item.fType = MFT_STRING;
    if (menu.GetItem(i, true, item))
    {
      UString newString;
      if (item.hSubMenu)
      {
        if (level == 1 && menuIndex == kBookmarksMenuIndex)
          newString = LangString(kAddToFavoritesLangID);
        else
        {
          MyChangeMenu(item.hSubMenu, level + 1, i);
          if (level == 1 && menuIndex == kViewMenuIndex)
          {
            newString = LangString(kToolbarsLangID);
          }
          else
          {
            if (level == 0 && i < sizeof(kStringLangPairs) / 
              sizeof(kStringLangPairs[0]))
              newString = LangString(kStringLangPairs[i].LangID);
            else
              continue;
          }
        }
        if (newString.IsEmpty())
          continue;
      }
      else
      {
        int langPos = FindLangItem(item.wID);
        if (langPos < 0)
          continue;
        newString = LangString(kIDLangPairs[langPos].LangID);
        if (newString.IsEmpty())
          continue;
        UString shorcutString = item.StringValue;
        int tabPos = shorcutString.ReverseFind(wchar_t('\t'));
        if (tabPos >= 0)
          newString += shorcutString.Mid(tabPos);
      }
      {
        item.StringValue = newString;
        item.fMask = Get_fMaskForString();
        item.fType = MFT_STRING;
        menu.SetItem(i, true, item);
      }
    }
  }
}

CMenu g_FileMenu;

class CFileMenuDestroyer
{
public:
  ~CFileMenuDestroyer()
  {
    if ((HMENU)g_FileMenu != 0)
      g_FileMenu.Destroy();
  }
} g_FileMenuDestroyer;


void MyLoadMenu(HWND hWnd)
{
  if ((HMENU)g_FileMenu != 0)
    g_FileMenu.Destroy();
  HMENU oldMenu = ::GetMenu(hWnd);
  HMENU baseMenu = ::LoadMenu(g_hInstance, MAKEINTRESOURCE(IDM_MENU));
  ::SetMenu(hWnd, baseMenu);
  ::DestroyMenu(oldMenu);
  if (!g_LangID.IsEmpty())
  {
    HMENU menuOld = ::GetMenu(hWnd);
    MyChangeMenu(menuOld, 0, 0);
  }
  ::DrawMenuBar(hWnd);
}

extern HWND g_HWND;
void MyLoadMenu()
{
  MyLoadMenu(g_HWND);
}

static void CopyMenu(HMENU srcMenuSpec, HMENU destMenuSpec)
{
  CMenu srcMenu;
  srcMenu.Attach(srcMenuSpec);
  CMenu destMenu;
  destMenu.Attach(destMenuSpec);
  int startPos = 0;
  for (int i = 0; i < srcMenu.GetItemCount(); i++)
  {
    CMenuItem item;
    item.fMask = MIIM_STATE | MIIM_ID | Get_fMaskForFTypeAndString();
    item.fType = MFT_STRING;
    if (srcMenu.GetItem(i, true, item))
      if (destMenu.InsertItem(startPos, true, item))
        startPos++;
  }
}

void OnMenuActivating(HWND /* hWnd */, HMENU hMenu, int position)
{
  if (::GetSubMenu(::GetMenu(g_HWND), position) != hMenu)
    return;
  if (position == kFileMenuIndex)
  {
    if ((HMENU)g_FileMenu == 0)
    {
      g_FileMenu.CreatePopup();
      CopyMenu(hMenu, g_FileMenu);
    }
    CMenu menu;
    menu.Attach(hMenu);
    while (menu.GetItemCount() > 0)
    {
      if (!menu.RemoveItem(0, MF_BYPOSITION))
        break;
    }
    // CopyMenu(g_FileMenu, hMenu);
    g_App.GetFocusedPanel().CreateFileMenu(hMenu);
  }
  else if (position == kEditMenuIndex)
  {
    /*
    CMenu menu;
    menu.Attach(hMenu);
    menu.EnableItem(IDM_EDIT_CUT, MF_ENABLED);
    menu.EnableItem(IDM_EDIT_COPY, MF_ENABLED);
    menu.EnableItem(IDM_EDIT_PASTE, IsClipboardFormatAvailableHDROP() ? MF_ENABLED : MF_GRAYED);
    */
  }
  else if (position == kViewMenuIndex)
  {
    // View;
    CMenu menu;
    menu.Attach(hMenu);
    menu.CheckRadioItem(IDM_VIEW_LARGE_ICONS, IDM_VIEW_DETAILS, 
      IDM_VIEW_LARGE_ICONS + g_App.GetListViewMode(), MF_BYCOMMAND);
    menu.CheckItem(IDM_VIEW_TWO_PANELS, MF_BYCOMMAND |
        ((g_App.NumPanels == 2) ? MF_CHECKED : MF_UNCHECKED));
    menu.CheckItem(IDM_VIEW_FLAT_VIEW, MF_BYCOMMAND |
        ((g_App.GetFlatMode()) ? MF_CHECKED : MF_UNCHECKED));
    menu.CheckItem(IDM_VIEW_ARCHIVE_TOOLBAR, MF_BYCOMMAND |
        (g_App.ShowArchiveToolbar ? MF_CHECKED : MF_UNCHECKED));
    menu.CheckItem(IDM_VIEW_STANDARD_TOOLBAR, MF_BYCOMMAND |
        (g_App.ShowStandardToolbar ? MF_CHECKED : MF_UNCHECKED));
    menu.CheckItem(IDM_VIEW_TOOLBARS_LARGE_BUTTONS, MF_BYCOMMAND |
        (g_App.LargeButtons ? MF_CHECKED : MF_UNCHECKED));
    menu.CheckItem(IDM_VIEW_TOOLBARS_SHOW_BUTTONS_TEXT, MF_BYCOMMAND |
        (g_App.ShowButtonsLables ? MF_CHECKED : MF_UNCHECKED));
  }
  else if (position == kBookmarksMenuIndex)
  {
    CMenu menu;
    menu.Attach(hMenu);

    CMenu subMenu;
    subMenu.Attach(menu.GetSubMenu(0));
    while (subMenu.GetItemCount() > 0)
      subMenu.RemoveItem(subMenu.GetItemCount() - 1, MF_BYPOSITION);
    int i;
    for (i = 0; i < 10; i++)
    {
      UString s = LangString(IDS_BOOKMARK, 0x03000720);
      s += L" ";
      wchar_t c = (wchar_t)(L'0' + i);
      s += c;
      s += L"\tAlt+Shift+";
      s += c;
      subMenu.AppendItem(MF_STRING, kSetBookmarkMenuID + i, s);
    }

    while (menu.GetItemCount() > 2)
      menu.RemoveItem(menu.GetItemCount() - 1, MF_BYPOSITION);

    for (i = 0; i < 10; i++)
    {
      UString path = g_App.AppState.FastFolders.GetString(i);
      const int kMaxSize = 100;
      const int kFirstPartSize = kMaxSize / 2;
      if (path.Length() > kMaxSize)
      {
        path = path.Left(kFirstPartSize) + UString(L" ... ") +
          path.Right(kMaxSize - kFirstPartSize);
      }
      UString s = path;
      if (s.IsEmpty())
        s = L"-";
      s += L"\tAlt+";
      s += (wchar_t)(L'0' + i);
      menu.AppendItem(MF_STRING, kOpenBookmarkMenuID + i, s);
    }
  }
}

/*
It doesn't help
void OnMenuUnActivating(HWND hWnd, HMENU hMenu, int id)
{
  if (::GetSubMenu(::GetMenu(g_HWND), 0) != hMenu)
    return;
  // g_App.GetFocusedPanel()._contextMenu.Release();
}

void OnMenuUnActivating(HWND hWnd)
{
}
*/


void LoadFileMenu(HMENU hMenu, int startPos, bool /* forFileMode */, bool programMenu)
{
  {
    CMenu srcMenu;
    srcMenu.Attach(::GetSubMenu(::GetMenu(g_HWND), 0));
    if ((HMENU)g_FileMenu == 0)
    {
      g_FileMenu.CreatePopup();
      CopyMenu(srcMenu, g_FileMenu);
    }
  }

  CMenu destMenu;
  destMenu.Attach(hMenu);
  
  for (int i = 0; i < g_FileMenu.GetItemCount(); i++)
  {
    CMenuItem item;

    item.fMask = MIIM_STATE | MIIM_ID | Get_fMaskForFTypeAndString();
    item.fType = MFT_STRING;
    if (g_FileMenu.GetItem(i, true, item))
    {
      if (!programMenu)
        if (item.wID == IDCLOSE)
          continue;
      /*
      bool createItem = (item.wID == IDM_CREATE_FOLDER || item.wID == IDM_CREATE_FILE);
      if (forFileMode)
      {
        if (createItem)
         continue;
      }
      else
      {
        if (!createItem)
         continue;
      }
      */
      if (destMenu.InsertItem(startPos, true, item))
        startPos++;
    }
  }
  while (destMenu.GetItemCount() > 0)
  {
    CMenuItem item;
    item.fMask = MIIM_TYPE;
    item.fType = 0;
    // item.dwTypeData = 0;
    int lastIndex = destMenu.GetItemCount() - 1;
    if (!destMenu.GetItem(lastIndex, true, item))
      break;
    if(item.fType != MFT_SEPARATOR)
      break;
    if (!destMenu.RemoveItem(lastIndex, MF_BYPOSITION))
      break;
  }
}

bool ExecuteFileCommand(int id)
{
  if (id >= kPluginMenuStartID)
  {
    g_App.GetFocusedPanel().InvokePluginCommand(id);
    g_App.GetFocusedPanel()._sevenZipContextMenu.Release();
    g_App.GetFocusedPanel()._systemContextMenu.Release();
    return true;
  }

  switch (id)
  {
    // File
    case IDM_FILE_OPEN:
      g_App.OpenItem();
      break;
    case IDM_FILE_OPEN_INSIDE:
      g_App.OpenItemInside();
      break;
    case IDM_FILE_OPEN_OUTSIDE:
      g_App.OpenItemOutside();
      break;
    case IDM_FILE_VIEW:
      break;
    case IDM_FILE_EDIT:
      g_App.EditItem();
      break;
    case IDM_RENAME:
      g_App.Rename();
      break;
    case IDM_COPY_TO:
      g_App.CopyTo();
      break;
    case IDM_MOVE_TO:
      g_App.MoveTo();
      break;
    case IDM_DELETE:
    {
      bool shift = (::GetKeyState(VK_SHIFT) & 0x8000) != 0;
      g_App.Delete(!shift);
      break;
    }
    case IDM_FILE_CRC:
      g_App.CalculateCrc();
      break;
    case IDM_FILE_SPLIT:
      g_App.Split();
      break;
    case IDM_FILE_COMBINE:
      g_App.Combine();
      break;
    case IDM_FILE_PROPERTIES:
      g_App.Properties();
      break;
    case IDM_FILE_COMMENT:
      g_App.Comment();
      break;

    case IDM_CREATE_FOLDER:
      g_App.CreateFolder();
      break;
    case IDM_CREATE_FILE:
      g_App.CreateFile();
      break;
    default:
      return false;
  } 
  return true;
}

bool OnMenuCommand(HWND hWnd, int id)
{
  if (ExecuteFileCommand(id))
    return true;

  switch (id)
  {
    // File
    case IDCLOSE:
      SendMessage(hWnd, WM_ACTIVATE, MAKEWPARAM(WA_INACTIVE, 0), (LPARAM)hWnd);
      SendMessage (hWnd, WM_CLOSE, 0, 0);
      break;
    
    // Edit
    case IDM_EDIT_CUT:
      g_App.EditCut();
      break;
    case IDM_EDIT_COPY:
      g_App.EditCopy();
      break;
    case IDM_EDIT_PASTE:
      g_App.EditPaste();
      break;
    case IDM_SELECT_ALL:
      g_App.SelectAll(true);
      g_App.RefreshStatusBar();
      break;
    case IDM_DESELECT_ALL:
      g_App.SelectAll(false);
      g_App.RefreshStatusBar();
      break;
    case IDM_INVERT_SELECTION:
      g_App.InvertSelection();
      g_App.RefreshStatusBar();
      break;
    case IDM_SELECT:
      g_App.SelectSpec(true);
      g_App.RefreshStatusBar();
      break;
    case IDM_DESELECT:
      g_App.SelectSpec(false);
      g_App.RefreshStatusBar();
      break;
    case IDM_SELECT_BY_TYPE:
      g_App.SelectByType(true);
      g_App.RefreshStatusBar();
      break;
    case IDM_DESELECT_BY_TYPE:
      g_App.SelectByType(false);
      g_App.RefreshStatusBar();
      break;

    //View
    case IDM_VIEW_LARGE_ICONS:
    case IDM_VIEW_SMALL_ICONS:
    case IDM_VIEW_LIST:
    case IDM_VIEW_DETAILS:
    {
      UINT index = id - IDM_VIEW_LARGE_ICONS;
      if (index < 4)
      {
        g_App.SetListViewMode(index);
        /*
        CMenu menu;
        menu.Attach(::GetSubMenu(::GetMenu(hWnd), kViewMenuIndex));
        menu.CheckRadioItem(IDM_VIEW_LARGE_ICONS, IDM_VIEW_DETAILS, 
            id, MF_BYCOMMAND);
        */
      }
      break;
    }
    case IDM_VIEW_ARANGE_BY_NAME:
    {
      g_App.SortItemsWithPropID(kpidName);
      break;
    }
    case IDM_VIEW_ARANGE_BY_TYPE:
    {
      g_App.SortItemsWithPropID(kpidExtension);
      break;
    }
    case IDM_VIEW_ARANGE_BY_DATE:
    {
      g_App.SortItemsWithPropID(kpidLastWriteTime);
      break;
    }
    case IDM_VIEW_ARANGE_BY_SIZE:
    {
      g_App.SortItemsWithPropID(kpidSize);
      break;
    }
    case IDM_VIEW_ARANGE_NO_SORT:
    {
      g_App.SortItemsWithPropID(kpidNoProperty);
      break;
    }

    case IDM_OPEN_ROOT_FOLDER:
      g_App.OpenRootFolder();
      break;
    case IDM_OPEN_PARENT_FOLDER:
      g_App.OpenParentFolder();
      break;
    case IDM_FOLDERS_HISTORY:
      g_App.FoldersHistory();
      break;
    case IDM_VIEW_REFRESH:
      g_App.RefreshView();
      break;
    case IDM_VIEW_FLAT_VIEW:
      g_App.ChangeFlatMode();
      break;
    case IDM_VIEW_TWO_PANELS:
      g_App.SwitchOnOffOnePanel();
      break;
    case IDM_VIEW_STANDARD_TOOLBAR:
      g_App.SwitchStandardToolbar();
      break;
    case IDM_VIEW_ARCHIVE_TOOLBAR:
      g_App.SwitchArchiveToolbar();
      break;
    case IDM_VIEW_TOOLBARS_SHOW_BUTTONS_TEXT:
      g_App.SwitchButtonsLables();
      break;
    case IDM_VIEW_TOOLBARS_LARGE_BUTTONS:
      g_App.SwitchLargeButtons();
      break;

    // Tools
    case IDM_OPTIONS:
      OptionsDialog(hWnd, g_hInstance);
      break;
          
    case IDM_BENCHMARK:
      Benchmark();
      break;
    // Help
    case IDM_HELP_CONTENTS:
      ShowHelpWindow(NULL, kFMHelpTopic);
      break;
    case IDM_ABOUT:
    {
      CAboutDialog dialog;
      dialog.Create(hWnd);
      break;
    }
    default:
    {
      if (id >= kOpenBookmarkMenuID && id <= kOpenBookmarkMenuID + 9)
      {
        g_App.OpenBookmark(id - kOpenBookmarkMenuID);
        return true;
      }
      else if (id >= kSetBookmarkMenuID && id <= kSetBookmarkMenuID + 9)
      {
        g_App.SetBookmark(id - kSetBookmarkMenuID);
        return true;
      }
      return false;
    }
  }
  return true;
}

