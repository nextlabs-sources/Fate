
#ifndef _NUDF_UIHLP_HPP__
#define _NUDF_UIHLP_HPP__

#include <string>
#include <vector>

#include <nudf\shared\nxlfmt.h>
#include <nudf\exception.hpp>
#include <nudf\convert.hpp>

namespace nudf {
namespace util {

    
typedef BOOL (WINAPI* RMUINITIALIZE)();
typedef VOID (WINAPI* RMUDEINITIALIZE)();
typedef LONG (WINAPI* RMUSHOWALLPROPPAGES)(_In_opt_ HWND, _In_ LPCWSTR, _In_ PCNXL_HEADER, _In_ ULONGLONG, _In_opt_ LPCWSTR);
typedef LONG (WINAPI* RMUSHOWGENERALPROPPAGE)(_In_opt_ HWND, _In_ LPCWSTR, _In_ PCNXL_HEADER);
typedef LONG (WINAPI* RMUSHOWPERMISSIONPROPPAGE)(_In_opt_ HWND, _In_ LPCWSTR, _In_ const ULONGLONG, _Out_opt_ PULONGLONG);
typedef LONG (WINAPI* RMUSHOWCLASSIFYPROPPAGE)(_In_opt_ HWND, _In_ LPCWSTR, _In_opt_ LPCWSTR);
typedef LONG (WINAPI* RMUSHOWDETAILSPROPPAGE)(_In_opt_ HWND, _In_ LPCWSTR,_In_ const ULONGLONG, _In_opt_ LPCWSTR);
typedef LONG (WINAPI* RMUSHOWALLPROPPAGESSIMPLE)(_In_opt_ HWND, _In_ LPCWSTR, _In_ ULONGLONG);
typedef LONG (WINAPI* RMUSHOWGENERALPROPPAGESIMPLE)(_In_opt_ HWND, _In_ LPCWSTR);
typedef LONG (WINAPI* RMUSHOWCLASSIFYPROPPAGESIMPLE)(_In_opt_ HWND, _In_ LPCWSTR);
typedef LONG (WINAPI* RMUSHOWDETAILSPROPPAGESIMPLE)(_In_opt_ HWND, _In_ LPCWSTR, _In_ ULONGLONG);
typedef VOID (WINAPI* RMUFREERESOURCE)(_In_ PVOID);
typedef LONG (WINAPI* RMUSHOWCLASSIFYDIALOG)(_In_opt_ HWND, _In_ LPCWSTR, _In_ LPCWSTR, _In_opt_ LPCWSTR, _In_opt_ LPCWSTR, _In_ BOOL, _Out_ LPWSTR*);
typedef LONG (WINAPI* RMUSHOWCLASSIFYDIALOGEX)(_In_opt_ HWND, _In_ LPCWSTR, _In_ LPCWSTR, _In_opt_ LPCWSTR, _In_ ULONG, _In_opt_ LPCWSTR, _In_ BOOL, _Out_ LPWSTR*);
typedef LONG (WINAPI* RMUSHOWCLASSIFYDIALOG2)(_In_opt_ HWND, _In_ LPCWSTR, _In_ LPCWSTR, _Out_ LPWSTR*);
typedef LONG (WINAPI* RMUSHOWCLASSIFYDIALOGEX2)(_In_opt_ HWND, _In_ LPCWSTR, _In_ LPCWSTR, _In_ ULONG, _Out_ LPWSTR*);


class CRmuObject
{
public:
    CRmuObject() : _hDll(NULL),
                   _RmuInitialize(NULL),
                   _RmuDeinitialize(NULL),
                   _RmuShowAllPropPages(NULL),
                   _RmuShowGeneralPropPage(NULL),
                   _RmuShowPermissionPropPage(NULL),
                   _RmuShowClassifyPropPage(NULL),
                   _RmuShowDetailsPropPage(NULL),
                   _RmuShowAllPropPagesSimple(NULL),
                   _RmuShowGeneralPropPageSimple(NULL),
                   _RmuShowClassifyPropPageSimple(NULL),
                   _RmuShowDetailsPropPageSimple(NULL),
                   _RmuFreeResource(NULL),
                   _RmuShowClassifyDialog(NULL),
                   _RmuShowClassifyDialogEx(NULL),
                   _RmuShowClassifyDialog2(NULL),
                   _RmuShowClassifyDialogEx2(NULL)
    {
    }

    ~CRmuObject()
    {
        Clear();
    }


    void Initialize(_In_ LPCWSTR wzDll)
    {
        _hDll = ::LoadLibraryW(wzDll);
        if(NULL == _hDll) {
            throw WIN32ERROR();
        }

        _RmuInitialize = (RMUINITIALIZE)::GetProcAddress(_hDll, MAKEINTRESOURCEA(1));
        if(NULL == _RmuInitialize) {
            throw WIN32ERROR2(ERROR_INVALID_FUNCTION);
        }
        _RmuDeinitialize = (RMUDEINITIALIZE)::GetProcAddress(_hDll, MAKEINTRESOURCEA(2));
        if(NULL == _RmuDeinitialize) {
            throw WIN32ERROR2(ERROR_INVALID_FUNCTION);
        }
        _RmuShowAllPropPages = (RMUSHOWALLPROPPAGES)::GetProcAddress(_hDll, MAKEINTRESOURCEA(3));
        if(NULL == _RmuShowAllPropPages) {
            throw WIN32ERROR2(ERROR_INVALID_FUNCTION);
        }
        _RmuShowGeneralPropPage = (RMUSHOWGENERALPROPPAGE)::GetProcAddress(_hDll, MAKEINTRESOURCEA(4));
        if(NULL == _RmuShowGeneralPropPage) {
            throw WIN32ERROR2(ERROR_INVALID_FUNCTION);
        }
        _RmuShowPermissionPropPage = (RMUSHOWPERMISSIONPROPPAGE)::GetProcAddress(_hDll, MAKEINTRESOURCEA(5));
        if(NULL == _RmuShowPermissionPropPage) {
            throw WIN32ERROR2(ERROR_INVALID_FUNCTION);
        }
        _RmuShowClassifyPropPage = (RMUSHOWCLASSIFYPROPPAGE)::GetProcAddress(_hDll, MAKEINTRESOURCEA(6));
        if(NULL == _RmuShowClassifyPropPage) {
            throw WIN32ERROR2(ERROR_INVALID_FUNCTION);
        }
        _RmuShowDetailsPropPage = (RMUSHOWDETAILSPROPPAGE)::GetProcAddress(_hDll, MAKEINTRESOURCEA(7));
        if(NULL == _RmuShowDetailsPropPage) {
            throw WIN32ERROR2(ERROR_INVALID_FUNCTION);
        }
        _RmuShowAllPropPagesSimple = (RMUSHOWALLPROPPAGESSIMPLE)::GetProcAddress(_hDll, MAKEINTRESOURCEA(8));
        if(NULL == _RmuShowAllPropPagesSimple) {
            throw WIN32ERROR2(ERROR_INVALID_FUNCTION);
        }
        _RmuShowGeneralPropPageSimple = (RMUSHOWGENERALPROPPAGESIMPLE)::GetProcAddress(_hDll, MAKEINTRESOURCEA(9));
        if(NULL == _RmuShowGeneralPropPageSimple) {
            throw WIN32ERROR2(ERROR_INVALID_FUNCTION);
        }
        _RmuShowClassifyPropPageSimple = (RMUSHOWCLASSIFYPROPPAGESIMPLE)::GetProcAddress(_hDll, MAKEINTRESOURCEA(10));
        if(NULL == _RmuShowClassifyPropPageSimple) {
            throw WIN32ERROR2(ERROR_INVALID_FUNCTION);
        }
        _RmuShowDetailsPropPageSimple = (RMUSHOWDETAILSPROPPAGESIMPLE)::GetProcAddress(_hDll, MAKEINTRESOURCEA(11));
        if(NULL == _RmuShowDetailsPropPageSimple) {
            throw WIN32ERROR2(ERROR_INVALID_FUNCTION);
        }
        _RmuFreeResource = (RMUFREERESOURCE)::GetProcAddress(_hDll, MAKEINTRESOURCEA(12));
        if(NULL == _RmuFreeResource) {
            throw WIN32ERROR2(ERROR_INVALID_FUNCTION);
        }
        _RmuShowClassifyDialog = (RMUSHOWCLASSIFYDIALOG)::GetProcAddress(_hDll, MAKEINTRESOURCEA(13));
        if(NULL == _RmuShowClassifyDialog) {
            throw WIN32ERROR2(ERROR_INVALID_FUNCTION);
        }
        _RmuShowClassifyDialogEx = (RMUSHOWCLASSIFYDIALOGEX)::GetProcAddress(_hDll, MAKEINTRESOURCEA(14));
        if(NULL == _RmuShowClassifyDialog) {
            throw WIN32ERROR2(ERROR_INVALID_FUNCTION);
        }
        _RmuShowClassifyDialog2 = (RMUSHOWCLASSIFYDIALOG2)::GetProcAddress(_hDll, MAKEINTRESOURCEA(15));
        if(NULL == _RmuShowClassifyDialog) {
            throw WIN32ERROR2(ERROR_INVALID_FUNCTION);
        }
        _RmuShowClassifyDialogEx2 = (RMUSHOWCLASSIFYDIALOGEX2)::GetProcAddress(_hDll, MAKEINTRESOURCEA(16));
        if(NULL == _RmuShowClassifyDialog) {
            throw WIN32ERROR2(ERROR_INVALID_FUNCTION);
        }
    }

    void Clear()
    {
        _RmuInitialize = NULL;
        _RmuDeinitialize = NULL;
        _RmuShowAllPropPages = NULL;
        _RmuShowGeneralPropPage = NULL;
        _RmuShowPermissionPropPage = NULL;
        _RmuShowClassifyPropPage = NULL;
        _RmuShowDetailsPropPage = NULL;
        _RmuShowAllPropPagesSimple = NULL;
        _RmuShowGeneralPropPageSimple = NULL;
        _RmuShowClassifyPropPageSimple = NULL;
        _RmuShowDetailsPropPageSimple = NULL;
        _RmuFreeResource = NULL;
        _RmuShowClassifyDialog = NULL;
        _RmuShowClassifyDialogEx = NULL;
        _RmuShowClassifyDialog2 = NULL;
        _RmuShowClassifyDialogEx2 = NULL;
        if(NULL != _hDll) {
            FreeLibrary(_hDll);
            _hDll = NULL;
        }
    }

    BOOL RmuInitialize()
    {
        if(NULL == _RmuInitialize) {
            return FALSE;
        }
        return _RmuInitialize();
    }

    VOID RmuDeinitialize()
    {
        if(NULL == _RmuDeinitialize) {
            return;
        }
        _RmuDeinitialize();
    }

    LONG RmuShowAllPropPages(_In_opt_ HWND hParent, _In_ LPCWSTR wzFile, _In_ PCNXL_HEADER pHeader, _In_ ULONGLONG uRights, _In_ const std::vector<std::pair<std::wstring,std::wstring>>& classifydata)
    {
        if(NULL == _RmuShowAllPropPages) {
            return -1;
        }
        
        std::vector<wchar_t> vClassifyData;
        nudf::util::convert::PairVectorToMultiStrings<wchar_t>(classifydata, vClassifyData, L'=');
        
        return _RmuShowAllPropPages(hParent, wzFile, pHeader, uRights, vClassifyData.empty() ? NULL : (&vClassifyData[0]));
    }

    LONG RmuShowGeneralPropPage(_In_opt_ HWND hParent, _In_ LPCWSTR wzFile, _In_ PCNXL_HEADER pHeader)
    {
        if(NULL == _RmuShowGeneralPropPage) {
            return -1;
        }
        return _RmuShowGeneralPropPage(hParent, wzFile, pHeader);
    }

    LONG RmuShowPermissionPropPage(_In_opt_ HWND hParent, _In_ LPCWSTR wzFile, _In_ const ULONGLONG uRrights, _Out_opt_ PULONGLONG puNewRights)
    {
        if(NULL == _RmuShowPermissionPropPage) {
            return -1;
        }
        return _RmuShowPermissionPropPage(hParent, wzFile, uRrights, puNewRights);
    }

    LONG RmuShowClassifyPropPage(_In_opt_ HWND hParent, _In_ LPCWSTR wzFile, _In_ const std::vector<std::pair<std::wstring,std::wstring>>& classifydata)
    {
        if(NULL == _RmuShowClassifyPropPage) {
            return -1;
        }
        std::vector<wchar_t> vClassifyData;
        nudf::util::convert::PairVectorToMultiStrings<wchar_t>(classifydata, vClassifyData, L'=');
        return _RmuShowClassifyPropPage(hParent, wzFile, vClassifyData.empty() ? NULL : (&vClassifyData[0]));
    }

    LONG RmuShowDetailsPropPage(_In_opt_ HWND hParent, _In_ LPCWSTR wzFile, _In_ const ULONGLONG uRrights, _In_ const std::vector<std::pair<std::wstring,std::wstring>>& classifydata)
    {
        if(NULL == _RmuShowDetailsPropPage) {
            return -1;
        }
        std::vector<wchar_t> vClassifyData;
        nudf::util::convert::PairVectorToMultiStrings<wchar_t>(classifydata, vClassifyData, L'=');
        return _RmuShowDetailsPropPage(hParent, wzFile, uRrights, vClassifyData.empty() ? NULL : (&vClassifyData[0]));
    }

    LONG RmuShowAllPropPagesSimple(_In_opt_ HWND hParent, _In_ LPCWSTR wzFile, _In_ ULONGLONG uRights)
    {
        if(NULL == _RmuShowAllPropPagesSimple) {
            return -1;
        }
        return _RmuShowAllPropPagesSimple(hParent, wzFile, uRights);
    }

    LONG RmuShowGeneralPropPageSimple(_In_opt_ HWND hParent, _In_ LPCWSTR wzFile)
    {
        if(NULL == _RmuShowGeneralPropPageSimple) {
            return -1;
        }
        return _RmuShowGeneralPropPageSimple(hParent, wzFile);
    }

    LONG RmuShowClassifyPropPageSimple(_In_opt_ HWND hParent, _In_ LPCWSTR wzFile)
    {
        if(NULL == _RmuShowClassifyPropPageSimple) {
            return -1;
        }
        return _RmuShowClassifyPropPageSimple(hParent, wzFile);
    }

    LONG RmuShowDetailsPropPageSimple(_In_opt_ HWND hParent, _In_ LPCWSTR wzFile, _In_ ULONGLONG uRights)
    {
        if(NULL == _RmuShowDetailsPropPageSimple) {
            return -1;
        }
        return _RmuShowDetailsPropPageSimple(hParent, wzFile, uRights);
    }

    LONG RmuShowClassifyDialog(_In_opt_ HWND hParent, _In_ LPCWSTR wzFile, _In_ LPCWSTR wzXml, _In_opt_ LPCWSTR wzGroup, _In_ BOOL ReadOnly, _Inout_ std::vector<std::pair<std::wstring,std::wstring>>& tags)
    {
        LONG   lRet = 0;
        LPWSTR pwzTags = NULL;
        std::vector<WCHAR> init_tags;

        if(NULL == _RmuShowClassifyDialog || NULL == _RmuFreeResource) {
            return -1;
        }

        if(!tags.empty()) {
            PairToBuffer(tags, init_tags);
        }

        lRet =  _RmuShowClassifyDialog(hParent, wzFile, wzXml, wzGroup, init_tags.empty() ? NULL : (&init_tags[0]), ReadOnly, &pwzTags);
        if(0 == lRet && NULL != pwzTags) {
            BufferToPair(pwzTags, tags);
            _RmuFreeResource(pwzTags);
            pwzTags = NULL;
        }

        return lRet;
    }

    LONG RmuShowClassifyDialogEx(_In_opt_ HWND hParent, _In_ LPCWSTR wzFile, _In_ LPCWSTR wzXmlFile, _In_opt_ LPCWSTR wzGroup, _In_ ULONG langId, _In_ BOOL ReadOnly, _Inout_ std::vector<std::pair<std::wstring,std::wstring>>& tags)
    {
        LONG   lRet = 0;
        LPWSTR pwzTags = NULL;
        std::vector<WCHAR> init_tags;

        if(NULL == _RmuShowClassifyDialog || NULL == _RmuShowClassifyDialogEx || NULL == _RmuFreeResource) {
            return -1;
        }

        if(!tags.empty()) {
            PairToBuffer(tags, init_tags);
        }

        lRet =  _RmuShowClassifyDialogEx(hParent, wzFile, wzXmlFile, wzGroup, langId, init_tags.empty() ? NULL : (&init_tags[0]), ReadOnly, &pwzTags);
        if(0 == lRet && NULL != pwzTags) {
            BufferToPair(pwzTags, tags);
            _RmuFreeResource(pwzTags);
            pwzTags = NULL;
        }

        return lRet;
    }

    LONG RmuShowClassifyDialog2(_In_opt_ HWND hParent, _In_ LPCWSTR wzFile, _In_ LPCWSTR wzXml, _Out_ std::vector<std::pair<std::wstring,std::wstring>>& tags)
    {
        LONG   lRet = 0;
        LPWSTR pwzTags = NULL;
        if(NULL == _RmuShowClassifyDialog2 || NULL == _RmuFreeResource) {
            return -1;
        }

        lRet =  _RmuShowClassifyDialog2(hParent, wzFile, wzXml, &pwzTags);
        if(0 == lRet && NULL != pwzTags) {
            BufferToPair(pwzTags, tags);
            _RmuFreeResource(pwzTags);
            pwzTags = NULL;
        }

        return lRet;
    }

    LONG RmuShowClassifyDialogEx2(_In_opt_ HWND hParent, _In_ LPCWSTR wzFile, _In_ LPCWSTR wzXmlFile, _In_ ULONG langId, _Out_ std::vector<std::pair<std::wstring,std::wstring>>& tags)
    {
        LONG   lRet = 0;
        LPWSTR pwzTags = NULL;
        if(NULL == _RmuShowClassifyDialog2 || NULL == _RmuShowClassifyDialogEx2 || NULL == _RmuFreeResource) {
            return -1;
        }

        lRet =  _RmuShowClassifyDialogEx2(hParent, wzFile, wzXmlFile, langId, &pwzTags);
        if(0 == lRet && NULL != pwzTags) {
            BufferToPair(pwzTags, tags);
            _RmuFreeResource(pwzTags);
            pwzTags = NULL;
        }

        return lRet;
    }


public:
    static void PairToBuffer(_In_ const std::vector<std::pair<std::wstring,std::wstring>>& pairs, _Out_ std::vector<WCHAR>& buf)
    {
        buf.clear();
        for(int i=0; i<(int)pairs.size(); i++) {
            if(pairs[i].first.empty() || pairs[i].second.empty()) {
                continue;
            }
            std::wstring ws(pairs[i].first);
            ws += L"=";
            ws += pairs[i].second;
            for(int j=0; j<(int)ws.length(); j++) {
                buf.push_back(ws.c_str()[j]);
            }
            buf.push_back(L'\0');
        }
        if(!buf.empty()) {
            buf.push_back(L'\0');
        }
    }

    static void BufferToPair(_In_ LPCWSTR buf, _Out_ std::vector<std::pair<std::wstring,std::wstring>>& pairs)
    {
        pairs.clear();
        while(buf[0] != L'\0') {
            std::wstring wsPair(buf);
            buf += (wsPair.length() + 1);
            std::wstring name;
            std::wstring value;
            std::wstring::size_type pos = wsPair.find_first_of(L'=');
            if(pos == std::wstring::npos) {
                continue;
            }
            name = wsPair.substr(0, pos);
            value = wsPair.substr(pos+1);
            pairs.push_back(std::pair<std::wstring,std::wstring>(name,value));
        }
    }

private:
    HMODULE                         _hDll;
    RMUINITIALIZE                   _RmuInitialize;
    RMUDEINITIALIZE                 _RmuDeinitialize;
    RMUSHOWALLPROPPAGES             _RmuShowAllPropPages;
    RMUSHOWGENERALPROPPAGE          _RmuShowGeneralPropPage;
    RMUSHOWPERMISSIONPROPPAGE       _RmuShowPermissionPropPage;
    RMUSHOWCLASSIFYPROPPAGE         _RmuShowClassifyPropPage;
    RMUSHOWDETAILSPROPPAGE          _RmuShowDetailsPropPage;
    RMUSHOWALLPROPPAGESSIMPLE       _RmuShowAllPropPagesSimple;
    RMUSHOWGENERALPROPPAGESIMPLE    _RmuShowGeneralPropPageSimple;
    RMUSHOWCLASSIFYPROPPAGESIMPLE   _RmuShowClassifyPropPageSimple;
    RMUSHOWDETAILSPROPPAGESIMPLE    _RmuShowDetailsPropPageSimple;
    RMUFREERESOURCE                 _RmuFreeResource;
    RMUSHOWCLASSIFYDIALOG           _RmuShowClassifyDialog;
    RMUSHOWCLASSIFYDIALOGEX         _RmuShowClassifyDialogEx;
    RMUSHOWCLASSIFYDIALOG2          _RmuShowClassifyDialog2;
    RMUSHOWCLASSIFYDIALOGEX2        _RmuShowClassifyDialogEx2;
};

}
}



#endif  // _NUDF_UIHLP_HPP__
