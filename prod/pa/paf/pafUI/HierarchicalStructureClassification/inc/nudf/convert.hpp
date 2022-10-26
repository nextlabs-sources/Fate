
#ifndef _NUDF_CONVERT_HPP__
#define _NUDF_CONVERT_HPP__

#include <string>
#include <vector>

namespace nudf {
namespace util {
namespace convert {

__forceinline
void SwapBytes(_Inout_ void* pv, _In_ size_t n)
{
    unsigned char *p = (unsigned char*)pv;
    size_t lo, hi;
    for(lo=0, hi=n-1; hi>lo; lo++, hi--) {
        unsigned char tmp = p[lo];
        p[lo] = p[hi];
        p[hi] = tmp;
    }
}

__forceinline
void ReverseMemCopy(void* pbDest, const void* pbSource, size_t cb)
{
    for (size_t i = 0; i < cb; i++) {
        ((unsigned char*)pbDest)[cb - 1 - i] = ((unsigned char*)pbSource)[i];
    }
}


template<typename T>
T ConvertEndian(_Inout_ T v)
{
    nudf::util::convert::SwapBytes(&v, sizeof(T));
    return v;
}

__forceinline
std::string Utf16ToUtf8(_In_ const std::wstring& str) throw()
{
    std::vector<CHAR> buf;
    buf.resize(str.length()*3+1, 0);
    ::WideCharToMultiByte(CP_UTF8, 0, str.c_str(), (int)str.length(), &buf[0], (int)buf.size(), NULL, NULL);
    return (&buf[0]);
}

__forceinline
std::wstring Utf8ToUtf16(_In_ const std::string& str) throw()
{
    std::vector<WCHAR> buf;
    buf.resize(str.length()*2+1, 0);
    ::MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.length(), &buf[0], (int)buf.size());
    return (&buf[0]);
}

template <typename T>
void MultiStringsToVector(_In_ const T* sStrings, _Out_ std::vector<std::basic_string<T>>& vStrings)
{
    vStrings.clear();
    while(0 != sStrings[0]) {
        // Get current string
        std::basic_string<T> s = sStrings;
        // Move to Next
        sStrings += (s.length() + 1);
        vStrings.push_back(sStrings);
    }
}

template <typename T>
void VectorToMultiStrings(_In_ const std::vector<std::basic_string<T>>& vStrings, _Out_ std::vector<T>& sStrings)
{
    sStrings.clear();
    for(std::vector<std::basic_string<T>>::const_iterator it=vStrings.begin(); it!=vStrings.end(); ++it) {
        if((*it).empty()) {
            continue;
        }
        sStrings.insert(sStrings.end(), (*it).begin(), (*it).end());
        sStrings.push_back(((T)0));
    }
    sStrings.push_back(((T)0));
}

template <typename T>
void MultiStringsToPairVector(_In_ const T* sStrings, _Out_ std::vector<std::pair<std::basic_string<T>,std::basic_string<T>>>& vStrings, _In_ const T cSeparator)
{
    vStrings.clear();
    while(0 != sStrings[0]) {
        // Get current string
        std::basic_string<T> s = sStrings;
        // Move to Next
        sStrings += (s.length() + 1);
        // Parse this pair
        std::basic_string<T>::size_type pos = s.find_first_of(cSeparator);
        if(std::basic_string<T>::npos == pos) {
            continue;
        }
        // Insert Pair
        vStrings.push_back(std::pair<std::basic_string<T>,std::basic_string<T>>(s.substr(0, pos), s.substr(pos+1)));
    }
}

template <typename T>
void PairVectorToMultiStrings(_In_ const std::vector<std::pair<std::basic_string<T>,std::basic_string<T>>>& vStrings, _Out_ std::vector<T>& sStrings, _In_ const T cSeparator)
{
    sStrings.clear();
    for(std::vector<std::pair<std::basic_string<T>,std::basic_string<T>>>::const_iterator it=vStrings.begin(); it!=vStrings.end(); ++it) {
        if((*it).first.empty()) {
            continue;
        }
        std::basic_string<T> s = (*it).first;
        s += cSeparator;
        s += (*it).second;
        sStrings.insert(sStrings.end(), s.begin(), s.end());
        sStrings.push_back(((T)0));
    }
    sStrings.push_back(((T)0));
}

    
}   // namespace nudf::util::convert
}   // namespace nudf::util
}   // namespace nudf

#endif  // _NUDF_REGEX_HPP__