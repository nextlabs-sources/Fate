
#ifndef __NUDF_STRING_HPP__
#define __NUDF_STRING_HPP__

#include <string>
#include <vector>

#include <nudf\regex.hpp>
#include <nudf\time.hpp>


#include <boost\algorithm\string.hpp>






namespace nudf {
namespace string {


//
// string buffer
//
template <typename T>
class tempstr
{
public:
    tempstr(std::basic_string<T>& str, size_t len) : _s(str)
    {
        // ctor
        _buf.resize(len+1, 0);
    }


    ~tempstr()
    {
        _s = std::basic_string<T>(&_buf[0]);      // copy to string passed by ref at construction
    }

    // auto conversion to serve as windows function parameter
    inline operator T* () throw() {return (&_buf[0]);}

private:
    // No copy allowed
    tempstr(tempstr<T>& c) {}
    // No assignment allowed
    tempstr& operator= (const tempstr<T>& c) {return *this;}

private:
    std::basic_string<T>&   _s;
    std::vector<T>          _buf;
};


//
//  Format Check
//

template<class T>
bool IsInteger(_In_ const std::basic_string<T>& s)
{
    // Regex:   ^[-+]?[0-9]+$
    // Example: -199 or 18
    const char* sz = "^[-+]?[0-9]+$";
    const wchar_t* wz = L"^[-+]?[0-9]+$";
    const T* p = (sizeof(wchar_t) == sizeof(T)) ? ((const T*)wz):((const T*)sz);
    if(s.length() == 0) {
        return false;
    }
    return nudf::util::regex::Match<T>(s, p, false);
}

template<class T>
bool IsFloat(_In_ const std::basic_string<T>& s)
{
    // Regex:   ^[-+]?[0-9]*\.[0-9]+$
    // Example: -132.354
    const char* sz = "^[-+]?[0-9]*\\.[0-9]+$";
    const wchar_t* wz = L"^[-+]?[0-9]*\\.[0-9]+$";
    const T* p = (sizeof(wchar_t) == sizeof(T)) ? ((const T*)wz):((const T*)sz);
    if(s.length() == 0) {
        return false;
    }
    return nudf::util::regex::Match<T>(s, p, false);
}

template<class T>
bool IsDecimal(_In_ const std::basic_string<T>& s)
{
    // Regex:   ^[-+]?[0-9]*\.?[0-9]*$
    // Example: 100 or 4.758
    const char* sz = "^[-+]?[0-9]*\\.?[0-9]*$";
    const wchar_t* wz = L"^[-+]?[0-9]*\\.?[0-9]*$";
    const T* p = (sizeof(wchar_t) == sizeof(T)) ? ((const T*)wz):((const T*)sz);
    if(s.length() == 0) {
        return false;
    }
    return nudf::util::regex::Match<T>(s, p, false);
}

template<class T>
bool IsHex(_In_ const std::basic_string<T>& s)
{
    // Regex:   ^[0]?[xX]?[0-9a-fA-F]+$
    // Example: 007B493D or 0x007B493D
    const char* sz = "^[0]?[xX]?[0-9a-fA-F]+$";
    const wchar_t* wz = L"^[0]?[xX]?[0-9a-fA-F]+$";
    const T* p = (sizeof(wchar_t) == sizeof(T)) ? ((const T*)wz):((const T*)sz);
    if(s.length() == 0) {
        return false;
    }
    return nudf::util::regex::Match<T>(s, p, false);
}

template<class T>
bool IsBytesString(_In_ const std::basic_string<T>& s)
{
    // Regex:   ^[0-9a-fA-F]+$
    // Example: 007B493D or 0x007B493D
    const char* sz = "^[0-9a-fA-F]+$";
    const wchar_t* wz = L"^[0-9a-fA-F]+$";
    const T* p = (sizeof(wchar_t) == sizeof(T)) ? ((const T*)wz):((const T*)sz);
    if(s.length() == 0 || 0 != (s.length()%2)) {
        return false;
    }
    return nudf::util::regex::Match<T>(s, p, false);
}

template<class T>
bool IsDatetime(_In_ const std::basic_string<T>& s)
{
    // Regex:   ^\d{4}-\d{2}-\d{2}T\d{2}:\d{2}:\d{2}[Z]?$
    // Example: 2009-06-15T13:45:30 or 2009-06-15T13:45:30Z
    const char* sz = "^\\d{4}-\\d{2}-\\d{2}T\\d{2}:\\d{2}:\\d{2}[Z]?$";
    const wchar_t* wz = L"^\\d{4}-\\d{2}-\\d{2}T\\d{2}:\\d{2}:\\d{2}[Z]?$";
    const T* p = (sizeof(wchar_t) == sizeof(T)) ? ((const T*)wz):((const T*)sz);
    if(s.length() == 0) {
        return false;
    }
    return nudf::util::regex::Match<T>(s, p, false);
}

template<class T>
bool IsDatetimeEx(_In_ const std::basic_string<T>& s)
{
    // Regex:   ^\d{4}-\d{2}-\d{2}T\d{2}:\d{2}:\d{2}\.\d{3}[Z]?$
    // Example: 2009-06-15T13:45:30.452 or 2009-06-15T13:45:30.452Z
    const char* sz = "^\\d{4}-\\d{2}-\\d{2}T\\d{2}:\\d{2}:\\d{2}\\.\\d{3}[Z]?$";
    const wchar_t* wz = L"^\\d{4}-\\d{2}-\\d{2}T\\d{2}:\\d{2}:\\d{2}\\.\\d{3}[Z]?$";
    const T* p = (sizeof(wchar_t) == sizeof(T)) ? ((const T*)wz):((const T*)sz);
    if(s.length() == 0) {
        return false;
    }
    return nudf::util::regex::Match<T>(s, p, false);
}

template<class T>
bool IsDatetimeWithZone(_In_ const std::basic_string<T>& s)
{
    // Regex:   ^\d{4}-\d{2}-\d{2}T\d{2}:\d{2}:\d{2}\.\d{3}[+-]\d{2}:\d{2}$
    // Example: 2014-12-29T16:51:36.216-08:00
    const char* sz = "^\\d{4}-\\d{2}-\\d{2}T\\d{2}:\\d{2}:\\d{2}\\.\\d{3}[+-]\\d{2}:\\d{2}$";
    const wchar_t* wz = L"^\\d{4}-\\d{2}-\\d{2}T\\d{2}:\\d{2}:\\d{2}\\.\\d{3}[+-]\\d{2}:\\d{2}$";
    const T* p = (sizeof(wchar_t) == sizeof(T)) ? ((const T*)wz):((const T*)sz);
    if(s.length() == 0) {
        return false;
    }
    return nudf::util::regex::Match<T>(s, p, false);
}

template<class T>
bool IsDatetimeWithName(_In_ const std::basic_string<T>& s)
{
    // Regex:   ^[a-zA-Z]{3} \d{1,2}, \d{4} \d{1,2}:\d{1,2}:\d{1,2} [aApP][mM]$
    // Example: Jun 26, 2015 3:18:00 PM
    const char* sz = "^[a-zA-Z]{3} \\d{1,2}, \\d{4} \\d{1,2}:\\d{1,2}:\\d{1,2} [aApP][mM]$";
    const wchar_t* wz = L"^[a-zA-Z]{3} \\d{1,2}, \\d{4} \\d{1,2}:\\d{1,2}:\\d{1,2} [aApP][mM]$";
    const T* p = (sizeof(wchar_t) == sizeof(T)) ? ((const T*)wz):((const T*)sz);
    if(s.length() == 0) {
        return false;
    }
    return nudf::util::regex::Match<T>(s, p, false);
}

template<class T>
bool IsUTCDatetime(_In_ const std::basic_string<T>& s)
{
    // Regex:   ^\d{4}-\d{2}-\d{2}T\d{2}:\d{2}:\d{2}[Z]$
    // Example: 2009-06-15T13:45:30Z
    const char* sz = "^\\d{4}-\\d{2}-\\d{2}T\\d{2}:\\d{2}:\\d{2}[Z]$";
    const wchar_t* wz = L"^\\d{4}-\\d{2}-\\d{2}T\\d{2}:\\d{2}:\\d{2}[Z]$";
    const T* p = (sizeof(wchar_t) == sizeof(T)) ? ((const T*)wz):((const T*)sz);
    if(s.length() == 0) {
        return false;
    }
    return nudf::util::regex::Match<T>(s, p, false);
}

template<class T>
bool IsUTCDatetimeEx(_In_ const std::basic_string<T>& s)
{
    // Regex:   ^\d{4}-\d{2}-\d{2}T\d{2}:\d{2}:\d{2}\.\d{3}[Z]$
    // Example: 2009-06-15T13:45:30Z
    const char* sz = "^\\d{4}-\\d{2}-\\d{2}T\\d{2}:\\d{2}:\\d{2}\\.\\d{3}[Z]$";
    const wchar_t* wz = L"^\\d{4}-\\d{2}-\\d{2}T\\d{2}:\\d{2}:\\d{2}\\.\\d{3}[Z]$";
    const T* p = (sizeof(wchar_t) == sizeof(T)) ? ((const T*)wz):((const T*)sz);
    if(s.length() == 0) {
        return false;
    }
    return nudf::util::regex::Match<T>(s, p, false);
}

template<class T>
bool IsDosPath(const std::basic_string<T>& s)
{
    // Regex:   ^[a-zA-Z]:\\[^?:*\"><|]*$
    // Example: C:\Program Files\Office\15\Winword.exe
    const char* sz = "^[a-zA-Z]:\\\\[^?:*\"><|]*$";
    const wchar_t* wz = L"^[a-zA-Z]:\\\\[^?:*\"><|]*$";
    const T* p = (sizeof(wchar_t) == sizeof(T)) ? ((const T*)wz):((const T*)sz);
    if(s.length() == 0) {
        return false;
    }
    return nudf::util::regex::Match<T>(s, p, false);
}

template<class T>
bool IsGlobalDosPath(const std::basic_string<T>& s)
{
    // Regex:   ^\\\?\?\\[a-zA-Z]:\\[^?:*\"><|]*$
    // Example: \??\C:\Program Files\Office\15\Winword.exe
    const char* sz = "^\\\\\\?\\?\\\\[a-zA-Z]:\\\\[^?:*\"><|]*$";
    const wchar_t* wz = L"^\\\\\\?\\?\\\\[a-zA-Z]:\\\\[^?:*\"><|]*$";
    const T* p = (sizeof(wchar_t) == sizeof(T)) ? ((const T*)wz):((const T*)sz);
    if(s.length() == 0) {
        return false;
    }
    return nudf::util::regex::Match<T>(s, p, false);
}

template<class T>
bool IsUncPath(const std::basic_string<T>& s)
{
    // Regex:   ^\\\\[^?:*"><|]*$
    // Example: \\nextlabs.com\share\data\design.docx
    const char* sz = "^\\\\\\\\[^?:*\"><|]*$";
    const wchar_t* wz = L"^\\\\\\\\[^?:*\"><|]*$";
    const T* p = (sizeof(wchar_t) == sizeof(T)) ? ((const T*)wz):((const T*)sz);
    if(s.length() == 0) {
        return false;
    }
    return nudf::util::regex::Match<T>(s, p, false);
}

template<class T>
bool IsGlobalUncPath(const std::basic_string<T>& s)
{
    // Regex:   ^\\\?\?\\[uU][nN][cC]\\[^?:*"><|]*$
    // Example: \??\UNC\nextlabs.com\share\data\design.docx
    const char* sz = "^\\\\\\?\\?\\\\[uU][nN][cC]\\\\[^?:*\"><|]*$";
    const wchar_t* wz = L"^\\\\\\?\\?\\\\[uU][nN][cC]\\\\[^?:*\"><|]*$";
    const T* p = (sizeof(wchar_t) == sizeof(T)) ? ((const T*)wz):((const T*)sz);
    if(s.length() == 0) {
        return false;
    }
    return nudf::util::regex::Match<T>(s, p, false);
}

template<class T>
bool IsFileName(const std::basic_string<T>& s)
{
    // Regex:   ^[^\/?:*\"><|]*$
    // Example: design.docx or temp
    const char* sz = "^[^\\/?:*\"><|]*$";
    const wchar_t* wz = L"^[^\\/?:*\"><|]*$";
    const T* p = (sizeof(wchar_t) == sizeof(T)) ? ((const T*)wz):((const T*)sz);
    if(s.length() == 0) {
        return false;
    }
    return nudf::util::regex::Match<T>(s, p, false);
}

template<class T>
bool IsEmailAddress(const std::basic_string<T>& s)
{
    // Regex:   ^[-._0-9a-zA-Z]*@[0-9a-zA-Z]*\.[0-9a-zA-Z]*$
    // Example: Gavin.Ye@nextlabs.com
    const char* sz = "^[-._0-9a-zA-Z]*@[0-9a-zA-Z]*\\.[0-9a-zA-Z]*$";
    const wchar_t* wz = L"^[-._0-9a-zA-Z]*@[0-9a-zA-Z]*\\.[0-9a-zA-Z]*$";
    const T* p = (sizeof(wchar_t) == sizeof(T)) ? ((const T*)wz):((const T*)sz);
    if(s.length() == 0) {
        return false;
    }
    return nudf::util::regex::Match<T>(s, p, false);
}

template<class T>
bool IsIpv4Address(const std::basic_string<T>& s)
{
    // Regex:   ^\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}$
    // Example: 192.168.0.14
    const char* sz = "^\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}$";
    const wchar_t* wz = L"^\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}$";
    const T* p = (sizeof(wchar_t) == sizeof(T)) ? ((const T*)wz):((const T*)sz);
    if(s.length() == 0) {
        return false;
    }
    return nudf::util::regex::Match<T>(s, p, false);
}

//
//  Convert
//

template<class T>
bool ToInt(const std::basic_string<T>& s, int* v)
{
    *v = 0;
    if(!IsInteger(s)) {
        return false;
    }
    std::wstring ls(s.begin(), s.end());
    if(ls.c_str()[0]==((T)'+'))
        ls = ls.substr(1);
    return (1 == swscanf_s(ls.c_str(), L"%d", v));
}

template<class T>
bool ToInt64(const std::basic_string<T>& s, __int64* v)
{
    *v = 0;
    if(!IsInteger(s)) {
        return false;
    }
    std::wstring ls(s.begin(), s.end());
    if(ls.c_str()[0]==((T)'+'))
        ls = ls.substr(1);
    return (1 == swscanf_s(ls.c_str(), L"%I64d", v));
}

template<class T>
bool ToUint(const std::basic_string<T>& s, unsigned int* v)
{
    *v = 0;
    if(!IsHex(s)) {
        return false;
    }
    std::wstring ls(s.begin(), s.end());
    std::transform(ls.begin(), ls.end(), ls.begin(), tolower);
    if(ls.c_str()[0]==L'0' && ls.c_str()[1]==L'x')
        ls = ls.substr(2);
    return (1 == swscanf_s(ls.c_str(), L"%x", v));
}

template<class T>
bool ToUint64(const std::basic_string<T>& s, unsigned __int64* v)
{
    *v = 0;
    if(!IsHex(s)) {
        return false;
    }
    std::wstring ls(s.begin(), s.end());
    std::transform(ls.begin(), ls.end(), ls.begin(), tolower);
    if(ls.c_str()[0]==L'0' && ls.c_str()[1]==L'x')
        ls = ls.substr(2);
    return (1 == swscanf_s(ls.c_str(), L"%I64x", v));
}

template<class T>
bool ToFloat(const std::basic_string<T>& s, float* v)
{
    *v = 0;
    if(!IsHex(s)) {
        return false;
    }
    return (1 == swscanf_s(s.c_str(), L"%f", v));
}

template<class T>
bool ToDouble(const std::basic_string<T>& s, double* v)
{
    *v = 0;
    if(!IsHex(s)) {
        return false;
    }
    return (1 == swscanf_s(s.c_str(), L"%f", v));
}

template<class T>
bool ToBoolean(const std::basic_string<T>& s, bool* v)
{
    *v = false;
    std::wstring ls(s.begin(), s.end());
    std::transform(ls.begin(), ls.end(), ls.begin(), tolower);
    if(0 == wcscmp(ls.c_str(), L"true") || 0 == wcscmp(ls.c_str(), L"yes") || 0 == wcscmp(ls.c_str(), L"on") || 0 == wcscmp(ls.c_str(), L"1")) {
        *v = true;
        return true;
    }
    else if(0 == wcscmp(ls.c_str(), L"false") || 0 == wcscmp(ls.c_str(), L"no") || 0 == wcscmp(ls.c_str(), L"off") || 0 == wcscmp(ls.c_str(), L"0")) {
        *v = false;
        return true;
    }
    else {
        return false;
    }
}

template<class T>
bool ToSystemTime(const std::basic_string<T>& s, SYSTEMTIME* v, bool* utc)
{
    memset(v, 0, sizeof(SYSTEMTIME));
    std::wstring ls(s.begin(), s.end());
    WORD nMilliseconds = 0;
    std::transform(ls.begin(), ls.end(), ls.begin(), toupper);

    if(NULL != utc) {
        *utc = false;
    }

    if(IsDatetime<T>(s)) {
        // YYYY-MM-DDTHH:MM:SS
        if(L'Z' == ls.c_str()[ls.length()-1]) {
            if(NULL != utc) {
                *utc = true;
            }
            ls = ls.substr(0, ls.length()-1);
        }
        swscanf_s(ls.c_str(), L"%04hd-%02hd-%02hdT%02hd:%02hd:%02hd", &v->wYear, &v->wMonth, &v->wDay, &v->wHour, &v->wMinute, &v->wSecond);
    }
    else if(IsDatetimeEx<T>(s)) {
        // YYYY-MM-DDTHH:MM:SS.MMM
        if(L'Z' == ls.c_str()[ls.length()-1]) {
            if(NULL != utc) {
                *utc = true;
            }
            ls = ls.substr(0, ls.length()-1);
        }
        swscanf_s(ls.c_str(), L"%04hd-%02hd-%02hdT%02hd:%02hd:%02hd.%03d", &v->wYear, &v->wMonth, &v->wDay, &v->wHour, &v->wMinute, &v->wSecond, &nMilliseconds);
        v->wMilliseconds = nMilliseconds;
    }
    else if(IsDatetimeWithZone<T>(s)) {
        // Format: UTC+Zone
        // YYYY-MM-DDTHH:MM:SS.MMM[+|-]%02d:%02d
        if(NULL != utc) {
            *utc = true;
        }
        std::wstring wsDatetime = ls.substr(0, 23);
        std::wstring wsZone = ls.substr(23);
        SYSTEMTIME   lst = {0};

        nudf::time::CTimeZone zone(wsZone.c_str());
        
        memset(&lst, 0, sizeof(lst));
        swscanf_s(wsDatetime.c_str(), L"%04hd-%02hd-%02hdT%02hd:%02hd:%02hd.%03d", &lst.wYear, &lst.wMonth, &lst.wDay, &lst.wHour, &lst.wMinute, &lst.wSecond, &nMilliseconds);
        lst.wMilliseconds = (WORD)nMilliseconds;

        nudf::time::CTime t(&lst);
        t.ToUtcTime(&zone);
        t.ToSystemTime(v);
    }
    else if(IsDatetimeWithName<T>(s)) {
        // Format: MMM DD, YYYY HH:MM:SS [AM|PM]
        // Jun 16, 2015 3:18:00 PM
        WCHAR        wzMon[16] = {0};
        WCHAR        wzType[16] = {0};
        WORD          nYear=0, nDay=0, nHour=0, nMinute=0, nSecond=0;
        swscanf_s(s.c_str(), L"%3s %d, %d %d:%d:%d %2s", wzMon, 16, &nDay, &nYear, &nHour, &nMinute, &nSecond, wzType, 16);
        v->wDay    = nDay;
        v->wYear   = nYear;
        v->wHour   = nHour;
        v->wMinute = nMinute;
        v->wSecond = nSecond;
        if(0 == _wcsicmp(wzMon, L"Jan")) {
            v->wMonth = 1;
        }
        else if(0 == _wcsicmp(wzMon, L"Feb")) {
            v->wMonth = 2;
        }
        else if(0 == _wcsicmp(wzMon, L"Mar")) {
            v->wMonth = 3;
        }
        else if(0 == _wcsicmp(wzMon, L"Apr")) {
            v->wMonth = 4;
        }
        else if(0 == _wcsicmp(wzMon, L"May")) {
            v->wMonth = 5;
        }
        else if(0 == _wcsicmp(wzMon, L"Jun")) {
            v->wMonth = 6;
        }
        else if(0 == _wcsicmp(wzMon, L"Jul")) {
            v->wMonth = 7;
        }
        else if(0 == _wcsicmp(wzMon, L"Aug")) {
            v->wMonth = 8;
        }
        else if(0 == _wcsicmp(wzMon, L"Sep")) {
            v->wMonth = 9;
        }
        else if(0 == _wcsicmp(wzMon, L"Oct")) {
            v->wMonth = 10;
        }
        else if(0 == _wcsicmp(wzMon, L"Nov")) {
            v->wMonth = 11;
        }
        else if(0 == _wcsicmp(wzMon, L"Dec")) {
            v->wMonth = 12;
        }
        else {
            return false;
        }
        if(0 == _wcsicmp(wzType, L"AM")) {
            ; // NOTHING
        }
        else if(0 == _wcsicmp(wzType, L"PM")) {
            if(v->wHour == 0 || v->wHour > 12) {
                return false;
            }
            if(v->wHour < 12) {
                v->wHour += 12;
            }
        }
        else {
            return false;
        }
        if(v->wMinute >= 60 || v->wSecond >= 60) {
            return false;
        }
    }
    else {
        return false;
    }

    return true;
}

template<class T>
bool ToSystemTime(const std::basic_string<T>& s, FILETIME* v, bool* utc)
{
    SYSTEMTIME st = {0};
    if(!ToSystemTime(s, &st, utc)) {
        return false;
    }
    return SystemTimeToFileTime(&st, v) ? true : false;
}

template<class T>
bool HexCharToUchar(T c, unsigned char*p)
{
    wchar_t wc = (wchar_t)c;
    if(wc >= L'0' && wc <= L'9') {
        *p = (UCHAR)(wc - L'0');
        return true;
    }
    else if(wc >= L'A' && wc <= L'F') {
        *p = (UCHAR)(10 + (wc - L'A'));
        return true;
    }
    else if(wc >= L'a' && wc <= L'f') {
        *p = (UCHAR)(10 + (wc - L'a'));
        return true;
    }
    else {
        *p = 0x0;
        return false;
    }
}

template<class T>
bool ToBytes(const std::basic_string<T>& s, std::vector<unsigned char>& v)
{
    if(s.length() == 0) {
        return false;
    }

    unsigned long len = (unsigned long)(((s.length()%2) == 0) ? s.length() : (s.length()-1));
    const T* sByte = s.c_str();
    while(0 != len) {
        unsigned char hi = 0;
        unsigned char lo = 0;
        if(!HexCharToUchar<T>(sByte[0], &hi)) {
            return false;
        }
        if(!HexCharToUchar<T>(sByte[1], &lo)) {
            return false;
        }
        hi = (hi << 4) + lo;
        v.push_back(hi);

        // move to next
        sByte += 2;
        len -= 2;
    }

    return true;
}

template<class T>
std::basic_string<T> FromGuid(const GUID* guid, bool brace)
{
    char szGuid[128] = {0};
    std::string s;

    sprintf_s(szGuid, brace ? "{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}" : "%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X",
        guid->Data1, guid->Data2, guid->Data3, guid->Data4[0], guid->Data4[1], guid->Data4[2], guid->Data4[3],
        guid->Data4[4], guid->Data4[5], guid->Data4[6], guid->Data4[7]);
    s = szGuid;
    return std::basic_string<T>(s.begin(), s.end());
}

template<class T>
std::basic_string<T> FromInt(int v, int wide=0)
{
    std::basic_string<T> s;
    const wchar_t* digits[] = {L"0", L"1", L"2", L"3", L"4", L"5", L"6", L"7", L"8", L"9"};

    if(0 == v) {
        wide = (0 == wide) ? 1 : wide;
        for(int i=0; i<wide; i++) {
            s += (const T*)(L"0");
        }
        return s;
    }

    bool minus = false;
    if(v < 0) {
        minus = true;
        v = abs(v);
    }

    do {
        s += (const T*)digits[v%10];
        v /= 10;
    } while (0 != v);
    while((int)s.length() < wide) {
        s += (const T*)(L"0");
    }
    if(minus) {
        s += (const T*)(L"-");
    }

    return std::basic_string<T>(s.rbegin(), s.rend());
}

template<class T>
std::basic_string<T> FromInt64(__int64 v, int wide=0)
{
    const wchar_t* digits[] = {L"0", L"1", L"2", L"3", L"4", L"5", L"6", L"7", L"8", L"9"};
    std::basic_string<T> s;

    if(0 == v) {
        wide = (0 == wide) ? 1 : wide;
        for(int i=0; i<wide; i++) {
            s += (const T*)(L"0");
        }
        return s;
    }

    bool minus = false;
    if(v < 0) {
        minus = true;
        v = abs((long)v);
    }

    do {
        s += (const T*)digits[v%10];
        v /= 10;
    } while (0 != v);
    while((int)s.length() < wide) {
        s += (const T*)(L"0");
    }
    if(minus) {
        s += (const T*)(L"-");
    }
    s.reserve();
    
    return std::basic_string<T>(s.rbegin(), s.rend());
}

template<class T>
std::basic_string<T> FromUint(unsigned int v)
{
    std::basic_string<T> s;
    s = (T)'0';
    s += (T)'x';
    s += FromByte<T>((unsigned char)((v >> 24) & 0xFF));
    s += FromByte<T>((unsigned char)((v >> 16) & 0xFF));
    s += FromByte<T>((unsigned char)((v >> 8) & 0xFF));
    s += FromByte<T>((unsigned char)(v & 0xFF));
    return s;
}

template<class T>
std::basic_string<T> FromUint64(unsigned __int64 v)
{
    std::basic_string<T> s;
    s = (T)'0';
    s += (T)'x';
    s += FromByte<T>((unsigned char)((v >> 56) & 0xFF));
    s += FromByte<T>((unsigned char)((v >> 48) & 0xFF));
    s += FromByte<T>((unsigned char)((v >> 40) & 0xFF));
    s += FromByte<T>((unsigned char)((v >> 32) & 0xFF));
    s += FromByte<T>((unsigned char)((v >> 24) & 0xFF));
    s += FromByte<T>((unsigned char)((v >> 16) & 0xFF));
    s += FromByte<T>((unsigned char)((v >> 8) & 0xFF));
    s += FromByte<T>((unsigned char)(v & 0xFF));
    return s;
}

template<class T>
std::basic_string<T> FromFloat(float v)
{
    std::basic_string<T> s;
    wchar_t ws[128] = {0};
    const wchar_t* p = ws;
    swprintf_s(ws, 128, L"%.3f", v);
    while(L'\0' != *p) {
        s.append(p, 1);
        p++;
    }
    return s;
}

template<class T>
std::basic_string<T> FromByte(unsigned char v)
{
    const char digits[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
    T buf[3] = {0, 0, 0};
    buf[0] = (T)digits[(v >> 4) & 0xF];
    buf[1] = (T)digits[v & 0xF];
    return buf;
}

template<class T>
std::basic_string<T> FromBytes(const unsigned char* v, unsigned long size)
{
    std::basic_string<T> s;
    if(0 == size || NULL == v) {
        return s;
    }
    for(unsigned long i=0; i<size; i++) {
        s += FromByte<T>(v[i]);
    }
    return s;
}

template<class T>
std::basic_string<T> FromBoolean(bool v)
{
    int i = 0;
    char* p = v ? "true" : "false";
    T buf[6] = {0, 0, 0, 0, 0, 0};
    while(0 != *p) {
        buf[i++] = (T)(*(p++));
    }
    return buf;
}

template<class T>
std::basic_string<T> FromSystemTime(const SYSTEMTIME* v, bool utc=true)
{
    // YYYY-MM-DDTHH:MM:SSZ
    std::basic_string<T> s;
    s = FromInt<T>(v->wYear, 4);
    s += (const T*)L"-";
    s += FromInt<T>(v->wMonth, 2);
    s += (const T*)L"-";
    s += FromInt<T>(v->wDay, 2);
    s += (const T*)L"T";
    s += FromInt<T>(v->wHour, 2);
    s += (const T*)L":";
    s += FromInt<T>(v->wMinute, 2);
    s += (const T*)L":";
    s += FromInt<T>(v->wSecond, 2);
    if(utc) {
        s += (const T*)L"Z";
    }
    return s;
}

template<class T>
std::basic_string<T> FromSystemTimeEx(const SYSTEMTIME* v, bool utc=true)
{
    // YYYY-MM-DDTHH:MM:SSZ
    std::basic_string<T> s;
    s = FromInt<T>(v->wYear, 4);
    s += (const T*)L"-";
    s += FromInt<T>(v->wMonth, 2);
    s += (const T*)L"-";
    s += FromInt<T>(v->wDay, 2);
    s += (const T*)L"T";
    s += FromInt<T>(v->wHour, 2);
    s += (const T*)L":";
    s += FromInt<T>(v->wMinute, 2);
    s += (const T*)L":";
    s += FromInt<T>(v->wSecond, 2);
    s += (const T*)L".";
    s += FromInt<T>(v->wMilliseconds, 3);
    if(utc) {
        s += (const T*)L"Z";
    }
    return s;
}

template<class T>
std::basic_string<T> FromSystemTime(const FILETIME* v, bool utc=true)
{
    SYSTEMTIME st = {0};
    FileTimeToSystemTime(v, &st);
    return FromSystemTime<T>(&st, utc);
}

template<class T>
std::basic_string<T> FromSystemTimeEx(const FILETIME* v, bool utc=true)
{
    SYSTEMTIME st = {0};
    FileTimeToSystemTime(v, &st);
    return FromSystemTimeEx<T>(&st, utc);
}

template<class T>
std::basic_string<T> TrimLeft(const std::basic_string<T>& s)
{
    return nudf::util::regex::TrimLeft<T>(s, ((T)L' '), false);
}

template<class T>
std::basic_string<T> TrimRight(const std::basic_string<T>& s)
{
    return nudf::util::regex::TrimRight<T>(s, ((T)L' '), false);
}

template<class T>
std::basic_string<T> Trim(const std::basic_string<T>& s)
{
    return nudf::util::regex::TrimAll<T>(s, ((T)L' '), false);
}

template<class T>
void Split(const std::basic_string<T>& s, T c, std::vector<std::basic_string<T>>& list)
{
    std::basic_string<T> ls(s);
    std::basic_string<T>::size_type pos;
    do {
        pos = ls.find_first_of(c);
        if(std::basic_string<T>::npos == pos) {
            if(!ls.empty()) {
                boost::algorithm::trim(ls);
                if (!ls.empty()) {
                    list.push_back(ls);
                }
            }
        }
        else {
            std::basic_string<T> component = ls.substr(0, pos);
            ls = ls.substr(pos+1);
            boost::algorithm::trim(component);
            boost::algorithm::trim(ls);
            if(!component.empty()) {
                list.push_back(component);
            }
        }
    } while(!ls.empty() && std::basic_string<T>::npos != pos);
}

template<class T>
void Split(const std::basic_string<T>& s, const std::basic_string<T>& c, std::vector<std::basic_string<T>>& list)
{
    std::basic_string<T> ls(s);
    std::basic_string<T>::size_type pos;
    do {
        pos = ls.find(c);
        if(std::basic_string<T>::npos == pos) {
            if(!ls.empty()) {
                list.push_back(ls);
            }
        }
        else {
            std::basic_string<T> component = ls.substr(0, pos);
            ls = ls.substr(pos+c.length());
            if(!component.empty()) {
                list.push_back(component);
            }
        }
    } while(!ls.empty() && std::basic_string<T>::npos != pos);
}

template<class T>
void Split(const std::basic_string<T>& s, T c, std::vector<int>& list)
{
    std::basic_string<T> ls = Trim<T>(s);
    std::vector<std::basic_string<T>> slist;
    Split<T>(ls, c, slist);
    for(std::vector<std::basic_string<T>>::iterator it=slist.begin(); it!=slist.end(); ++it) {
        int val = 0;
        if(ToInt(*it, &val)) {
            list.push_back(val);
        }
    }
}

template<class T>
void Split(const std::basic_string<T>& s, T c, std::vector<__int64>& list)
{
    std::basic_string<T> ls = Trim<T>(s);
    std::vector<std::basic_string<T>> slist;
    Split<T>(ls, c, slist);
    for(std::vector<std::basic_string<T>>::iterator it=slist.begin(); it!=slist.end(); ++it) {
        __int64 val = 0;
        if(ToInt64(*it, &val)) {
            list.push_back(val);
        }
    }
}

template<class T>
void Split(const std::basic_string<T>& s, T c, std::vector<unsigned int>& list)
{
    std::basic_string<T> ls = Trim<T>(s);
    std::vector<std::basic_string<T>> slist;
    Split<T>(ls, c, slist);
    for(std::vector<std::basic_string<T>>::iterator it=slist.begin(); it!=slist.end(); ++it) {
        unsigned int val = 0;
        if(ToUint(*it, &val)) {
            list.push_back(val);
        }
    }
}

template<class T>
void Split(const std::basic_string<T>& s, T c, std::vector<unsigned __int64>& list)
{
    std::basic_string<T> ls = Trim<T>(s);
    std::vector<std::basic_string<T>> slist;
    Split<T>(ls, c, slist);
    for(std::vector<std::basic_string<T>>::iterator it=slist.begin(); it!=slist.end(); ++it) {
        unsigned __int64 val = 0;
        if(ToUint64(*it, &val)) {
            list.push_back(val);
        }
    }
}


    
}   // namespace nudf::string
}   // namespace nudf

#endif

