

#ifndef __NUDF_ENCODING_HPP__
#define __NUDF_ENCODING_HPP__


#include <Wincrypt.h>
#include <string>
#include <vector>

namespace nudf
{
namespace util
{
namespace encoding
{

template<typename T>
std::basic_string<T> Base64Encode(_In_ const void* buf, _In_ unsigned long size)  throw()
{
    unsigned long cb = ((size +1) / 2) * 3 + 8;
    std::vector<char> outbuf;
    outbuf.resize(cb, 0);

    if(!CryptBinaryToStringA((const unsigned char*)buf, size, CRYPT_STRING_BASE64|CRYPT_STRING_NOCRLF, (LPSTR)&outbuf[0], &cb) || 0 == cb) {
        return std::basic_string<T>();
    }

    std::string s(&outbuf[0], cb);
    return std::basic_string<T>(s.begin(), s.end());
}

std::string Base64EncodeForX509(_In_ const void* buf, _In_ unsigned long size) throw();

template<typename T>
bool Base64Decode(_In_ const std::basic_string<T>& s, _Out_ std::vector<unsigned char>& buf) throw()
{
    unsigned long size = 0;
    std::string ls(s.begin(), s.end());
    if(!CryptStringToBinaryA(ls.c_str(), 0, CRYPT_STRING_BASE64_ANY, NULL, &size, NULL, NULL) || 0 == size) {
        return false;
    }
    buf.resize(size, 0x0);
    if(!CryptStringToBinaryA(ls.c_str(), 0, CRYPT_STRING_BASE64_ANY, &buf[0], &size, NULL, NULL)) {
        buf.clear();
        return false;
    }
    return true;
}

template<typename T>
bool Base64Decode(_In_ const std::basic_string<T>& s, _Out_ std::vector<unsigned char>& buf, _In_ bool with_header) throw()
{
    unsigned long size = 0;
    std::string ls(s.begin(), s.end());
    if(!CryptStringToBinaryA(ls.c_str(), 0, with_header ? CRYPT_STRING_BASE64HEADER : CRYPT_STRING_BASE64, NULL, &size, NULL, NULL) || 0 == size) {
        return false;
    }
    buf.resize(size, 0x0);
    if(!CryptStringToBinaryA(ls.c_str(), 0, with_header ? CRYPT_STRING_BASE64HEADER : CRYPT_STRING_BASE64, &buf[0], &size, NULL, NULL)) {
        buf.clear();
        return false;
    }
    return true;
}



}   // namespace nudf::util::encoding
}   // namespace nudf::util
}   // namespace nudf




#endif  // #ifndef __NUDF_ENCODING_HPP__