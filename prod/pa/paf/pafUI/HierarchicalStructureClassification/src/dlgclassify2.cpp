//#pragma comment(lib,"comctl32.lib")
//https://msdn.microsoft.com/en-us/library/windows/desktop/bb773175(v=vs.85).aspx
#define ISOLATION_AWARE_ENABLED 1
// Why it doesn't work. I have to add MANIFEST = $(BINDIR)/pafUI$(SUBTARGET_SUFFIX).dll.manifest in Makefile.inc
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")


#include <Windows.h>
#include <Windowsx.h>
#include <Commctrl.h>
#include <assert.h>

#include <list>

//#include <nudf\exception.hpp>

//#include "commonui.hpp"
#include <atlbase.h>
#include <atlcomcli.h>
#include "../resource.h"
#include "dlgtemplate.hpp"
#include "dlgclassify2.hpp"
#include <shellapi.h>
#include <fstream>
#include "nudf\user.hpp"
#include <vector>
#include <boost/algorithm/string.hpp>

void get_logon_user(std::wstring& userName, std::wstring& userSID)
{
	nudf::win::CUser user;
	user.GetProcessUser(GetCurrentProcess());

	userName = user.GetName();
	userSID = user.GetSid().GetSidStr();
}

CDlgClassify2::CDlgClassify2() : CDlgTemplate(IDD_DIALOG_CLASSIFY2), _hIcon(NULL), _hFontTitle0(NULL), _hFontTitle(NULL), _readonly(false)
{
}

CDlgClassify2::CDlgClassify2(_In_ const wchar_t* file, bool bLastFile, const wchar_t* title, _In_ const wchar_t* xml, _In_ const wchar_t* group, const std::vector<std::pair<std::wstring, std::wstring>>& oldTags):
CDlgTemplate(IDD_DIALOG_CLASSIFY2),
    _hIcon(NULL),
    _hFontTitle0(NULL),
    _hFontTitle(NULL),
    _readonly(false),
	m_bLastFile(bLastFile),
	m_strTitle(title)
{
	_listCtl.SetReadOnly(_readonly);
    SetFile(file);
    if (xml[0] == L'{') {
        SetJson(xml, group);
    }
    else {
        SetXml(xml, group, oldTags);
    }
}

CDlgClassify2::~CDlgClassify2()
{
    if(NULL != _hIcon) {
        DestroyIcon(_hIcon);
        _hIcon = NULL;
    }
    if(NULL != _hFontTitle0) {
        ::DeleteObject(_hFontTitle0);
        _hFontTitle0 = NULL;
    }
    if(NULL != _hFontTitle) {
        ::DeleteObject(_hFontTitle);
        _hFontTitle = NULL;
    }
    if(NULL != _tooltip) {
        ::DestroyWindow(_tooltip);
        _tooltip = NULL;
    }
}

void CDlgClassify2::SetXml(_In_ const std::wstring& xml, _In_ const std::wstring& group, const std::vector<std::pair<std::wstring, std::wstring>>& oldTags) throw()
{
	_clsdata.LoadFromXml(xml, group, oldTags);
}

void CDlgClassify2::SetJson(_In_ const std::wstring& ws, _In_ const std::wstring& group) throw()
{
    _clsdata.LoadFromJson(ws, group);
}

void CDlgClassify2::SetInitialData(_In_ const std::vector<std::pair<std::wstring,std::wstring>>& tags) throw()
{
    assert(!_clsdata.IsEmpty());
    _unMatchTags.clear();
    _finalTags.clear();
    for(int i=0; i<(int)tags.size(); i++) {
        if(!InitClassifyItem(tags[i].first, tags[i].second)) {
            _unMatchTags.push_back(tags[i]);
        }
    }
}

BOOL CDlgClassify2::InitClassifyItem(const std::wstring& name, const std::wstring& value)
{
    assert(!_clsdata.IsEmpty());
    std::vector<classify::CItem>& vItems = _clsdata.GetItemList();

    for(std::vector<classify::CItem>::iterator it=vItems.begin(); it!=vItems.end(); ++it) {

        if(0 == _wcsicmp(name.c_str(), (*it).GetName().c_str())) {

            std::vector<classify::CItemValue>& values = (*it).GetValues();
            for(int i=0; i<(int)values.size(); i++) {
                if(0 == _wcsicmp(value.c_str(), values[i].GetData().c_str())) {
                    // Good , found it
                    int nPriority = values[i].GetPriority();
                    if((*it).IsMultiSelection()) {
                        (*it).Select(i);
                    }
                    else {
                        std::vector<classify::CItemValue> newValues;
                        int nSelId = 0;
                        // Remove all values whose priority is lower than input value's
                        for(int j=0; j<(int)values.size(); j++) {
                            if(values[j].GetPriority() >= nPriority) {
                                newValues.push_back(values[j]);
                                if(0 == _wcsicmp(value.c_str(), values[j].GetData().c_str())) {
                                    nSelId = (int)newValues.size() - 1;
                                }
                            }
                        }
                        (*it).GetValues() = newValues;
                        (*it).Select(nSelId);
                        // We force this item to be mandatory because
                        (*it).SetMandatory(true);
                    }
                    return TRUE;
                }
            }
        }
    }
    return FALSE;
}

void CDlgClassify2::GetClassificationTags(_Out_ std::vector<std::pair<std::wstring,std::wstring>>& tags)
{
    tags = _finalTags;
}

VOID CDlgClassify2::GetDialogFontInfo(LOGFONTW* plFont)
{
    TEXTMETRIC tm = {0};
    HDC hDC = ::GetDC(GetDlgItem(GetHwnd(), IDC_EDIT_OBJECTNAME));
    HFONT hFont = GetWindowFont(GetDlgItem(GetHwnd(), IDC_EDIT_OBJECTNAME));
    
    HFONT hOldFont = (HFONT)::SelectObject(hDC, hFont);
    GetTextFaceW(hDC, 32, plFont->lfFaceName);
    GetTextMetricsW(hDC, &tm);
    ::SelectObject(hDC, hOldFont);
    
    plFont->lfHeight = tm.tmHeight;
    plFont->lfWidth = tm.tmAveCharWidth;
    plFont->lfEscapement = 0;
    plFont->lfOrientation = 0;
    plFont->lfWeight = tm.tmWeight;
    plFont->lfItalic = 0;
    plFont->lfUnderline = 0;
    plFont->lfStrikeOut = 0;
    plFont->lfCharSet = tm.tmCharSet;
    plFont->lfOutPrecision = OUT_DEFAULT_PRECIS;
    plFont->lfClipPrecision = CLIP_DEFAULT_PRECIS;
    plFont->lfQuality = PROOF_QUALITY;
    plFont->lfPitchAndFamily = tm.tmPitchAndFamily;
}

bool CDlgClassify2::FileExist(const wchar_t* pszFilePath)
{
	std::ifstream f(pszFilePath);
	if (f.good()) 
	{
		f.close();
		return true;
	}
	else
	{
		f.close();
		return false;
	}
}
BOOL CDlgClassify2::OnInitialize()
{
    SHFILEINFOW sfi = {0};
    LOGFONTW lFont = {0};

	if (_file.empty() || _clsdata.IsEmpty()) {
		return FALSE;
	}

	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(INITCOMMONCONTROLSEX);
	InitCtrls.dwICC = ICC_STANDARD_CLASSES | ICC_WIN95_CLASSES;
	BOOL bInitCtrl = InitCommonControlsEx(&InitCtrls);

	{
		HMODULE hDll;
		DWORD dwMajorVersion = 0;

		hDll = LoadLibrary(_T("COMCTL32.DLL"));
		if(hDll != NULL) {
			DLLGETVERSIONPROC fn_DllGetVersion;
			DLLVERSIONINFO vi;

			fn_DllGetVersion = (DLLGETVERSIONPROC) GetProcAddress(hDll, "DllGetVersion");
			if(fn_DllGetVersion != NULL) {
				vi.cbSize = sizeof(DLLVERSIONINFO);
				fn_DllGetVersion(&vi);
				dwMajorVersion = vi.dwMajorVersion;
			}
			FreeLibrary(hDll);
		}

		wchar_t sz[1024];
		wsprintfW(sz, L"[CDlgClassify2::OnInitialize]InitCommonControlsEx %s, GetLastError=%#x, COMCTL32 version=%d\n", 
			bInitCtrl ? L"successful" : L"failed", GetLastError(), dwMajorVersion);
		OutputDebugStringW(sz);
	}

	// Fix bug41906, see http://bugs.cn.nextlabs.com/show_bug.cgi?id=41906
	//[SHGetFileInfo(LPCTSTR pszPath, DWORD dwFileAttributes, SHFILEINFO *psfi, UINT cbFileInfo,UINT uFlags)]
	// (https://msdn.microsoft.com/en-us/library/windows/desktop/bb762179(v=vs.85).aspx)
	// pszPath 
	// If the uFlags parameter includes the SHGFI_USEFILEATTRIBUTES flag, this parameter does not have
	// to be a valid file name. The function will proceed as if the file exists with the specified name
	// and with the file attributes passed in the dwFileAttributes parameter. This allows you to obtain
	// information about a file type by passing just the extension for pszPath and passing 
	// FILE_ATTRIBUTE_NORMAL in dwFileAttributes.
	//uFlags
	// SHGFI_USEFILEATTRIBUTES (0x000000010)
	// Indicates that the function should not attempt to access the file specified by pszPath. 
	// Rather, it should act as if the file specified by pszPath exists with the file attributes
	// passed in dwFileAttributes. This flag cannot be combined with the SHGFI_ATTRIBUTES, 
	// SHGFI_EXETYPE, or SHGFI_PIDL flags.

	CoInitialize(NULL);
	DWORD_PTR hr = SHGetFileInfoW(0 == _file.compare(L"Email Header") ? L".msg" : _file.c_str(), 0, &sfi, sizeof(sfi), SHGFI_ICON | SHGFI_USEFILEATTRIBUTES);
	CoUninitialize();
	if (hr) {
		_hIcon = sfi.hIcon;
		Static_SetIcon(GetDlgItem(GetHwnd(), IDC_FILE_ICON), _hIcon);
	} 
	
	//just show file name in OE8.3
	std::wstring wstrFileName = _file;
	wchar_t wszFileName[1024+1] = { 0 };
	DWORD dwSize=GetModuleFileNameW(NULL, wszFileName, 1024);
	if ((dwSize>0) && (boost::algorithm::iends_with(wszFileName, L"outlook.exe")))
	{
		size_t nPos = _file.find_last_of(L'\\');
		if (nPos != std::wstring::npos)
		{
			wstrFileName = _file.substr(nPos + 1);
		}
	}
	SetWindowTextW(GetDlgItem(GetHwnd(), IDC_EDIT_OBJECTNAME), wstrFileName.c_str());
    
    GetDialogFontInfo(&lFont);
    
    _hFontTitle0 = CreateFontW(lFont.lfHeight,
                               0,
                               lFont.lfEscapement,
                               lFont.lfOrientation,
                               lFont.lfWeight + 150,
                               FALSE,
                               FALSE,
                               FALSE,
                               lFont.lfCharSet,
                               lFont.lfOutPrecision,
                               lFont.lfClipPrecision,
                               lFont.lfQuality,
                               lFont.lfPitchAndFamily,
                               lFont.lfFaceName);
    ::SendMessageW(GetDlgItem(GetHwnd(), IDC_STATIC_CLASSIFY_TITLE), WM_SETFONT, (WPARAM)_hFontTitle0, TRUE);

    _hFontTitle = CreateFontW( lFont.lfHeight,
                               0,
                               lFont.lfEscapement,
                               lFont.lfOrientation,
                               lFont.lfWeight + 150,
                               FALSE,
                               FALSE,
                               FALSE,
                               lFont.lfCharSet,
                               lFont.lfOutPrecision,
                               lFont.lfClipPrecision,
                               lFont.lfQuality,
                               lFont.lfPitchAndFamily,
                               lFont.lfFaceName);

    _tooltip = CreateWindowExW(0, L"tooltips_class32", NULL, 
             WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP, 
             CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 
             CW_USEDEFAULT, NULL, NULL, NULL, NULL);

    TOOLINFOW    toolinfo = {0};
    memset(&toolinfo, 0, sizeof(toolinfo));    
    toolinfo.cbSize = sizeof(toolinfo);
    toolinfo.uFlags = TTF_SUBCLASS | TTF_TRANSPARENT;
    toolinfo.hwnd = GetDlgItem(GetHwnd(), IDC_EDIT_OBJECTNAME);
    toolinfo.uId = IDC_EDIT_OBJECTNAME;
    toolinfo.hinst = NULL;
    toolinfo.lpszText = (LPWSTR)_file.c_str();
    toolinfo.lParam = NULL;
    GetClientRect(toolinfo.hwnd, &toolinfo.rect);
    SendMessageW(_tooltip, TTM_ADDTOOL, 0, (LPARAM)&toolinfo);


    // Init Summary List
    _listCtl.Attach(GetDlgItem(GetHwnd(), IDC_CLASSIFY_LIST));
    _listCtl.SetData(&_clsdata);
    _listCtl.SetTitleFont(_hFontTitle);
    _listCtl.Init();

	//Create Title window
	m_WndTitle.Attach(GetDlgItem(GetHwnd(), IDC_STATIC_CLASSIFY_TITLE));
	m_WndTitle.SetWindowText(m_strTitle.c_str());

	//set ok button text
	std::wstring wstrOkButtonText = m_bLastFile ? L"OK" : L"Next";
	SetDlgItemTextW(GetHwnd(), IDOK, wstrOkButtonText.c_str());

	//check if there is a show item. or if all the item is set to hidden
	//we must do it after _listCtl.Init(), because we will get data result from _listCtl;
	if (!HasShowItem(_clsdata))
	{
		::PostMessage(GetHwnd(), WM_COMMAND, MAKEWPARAM(IDOK, BN_CLICKED), 0);
	}
	else
	{
		::ShowWindow(GetHwnd(), SW_SHOW);
	}
	
    return TRUE;
}

BOOL CDlgClassify2::OnNotify(_In_ LPNMHDR lpnmhdr)
{
    if(IDC_SUMMARY_LIST == lpnmhdr->idFrom) {
       // HWND hList = GetDlgItem(GetHwnd(), IDC_SUMMARY_LIST);
        if(NM_CLICK == lpnmhdr->code) {
            LVHITTESTINFO itestinfo;

            assert(hList == lpnmhdr->hwndFrom);
            memset(&itestinfo, 0, sizeof(itestinfo));
            itestinfo.pt = ((LPNMITEMACTIVATE)lpnmhdr)->ptAction;
        }
    }
    else {
        ;
    }

    return CDlgTemplate::OnNotify(lpnmhdr);
}

BOOL CDlgClassify2::OnCommand(WORD notify, WORD id, HWND hwnd)
{
    return CDlgTemplate::OnCommand(notify, id, hwnd);
}

BOOL CDlgClassify2::OnOwnerDraw(DRAWITEMSTRUCT* lpDrawItemStruct)
{
    UNREFERENCED_PARAMETER(lpDrawItemStruct);
    if(IDC_CLASSIFY_LIST == lpDrawItemStruct->CtlID) {
        _listCtl.OnOwnerDraw(lpDrawItemStruct);
        return TRUE;
    }
	else if (lpDrawItemStruct->hwndItem == m_WndTitle.GetHWND() )
	{
		m_WndTitle.OnDrawItem(lpDrawItemStruct);
	}
    return FALSE;
}

BOOL CDlgClassify2::OnMeasureItem(MEASUREITEMSTRUCT* lpMeasureItemStruct)
{
    if(IDC_CLASSIFY_LIST == lpMeasureItemStruct->CtlID) {
        // Adjust list item size        
        _listCtl.OnMeasureItem(lpMeasureItemStruct);
        return TRUE;
    }
	

    return FALSE;
}

void CDlgClassify2::OnComboBoxSelChanged()
{
}

void CDlgClassify2::OnOk()
{
    // Make sure all the mandatory label s have value
    const classify::CItem* pItem = NULL;
    if(!_listCtl.MandatoryCheck(&pItem)) {
        std::wstring wsInfo = L"You must select a value for this label: ";
        wsInfo += pItem->GetDisplayName();
        MessageBoxW(GetHwnd(), wsInfo.c_str(), L"Error", MB_OK|MB_ICONEXCLAMATION);
        return;
    }

    // get & Return result
    _listCtl.GetResult(_finalTags);
    for(int i=0; i<(int)_unMatchTags.size(); i++) {
        _finalTags.push_back(_unMatchTags[i]);
    }
    CDlgTemplate::OnOk();
}

// check the Top Level of the Item to see if there is a item need to show
BOOL CDlgClassify2::HasShowItem(classify::CClassifyData& clsData)
{
	for (int i = 0; i < (int)clsData.GetEntryList().size(); i++)
	{
		classify::CItem* item = clsData.GetItem(clsData.GetEntryList()[i]);
		if ((item != NULL) &&(!item->IsHideUI()))
		{
			return TRUE;
		}
	}
	return FALSE;
}