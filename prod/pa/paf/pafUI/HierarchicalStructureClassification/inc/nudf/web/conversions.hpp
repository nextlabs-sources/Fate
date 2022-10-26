

#ifndef __JSON_UTILS_HPP__
#define __JSON_UTILS_HPP__

#include <sstream>
#include <string>
#include <memory>
#include <vector>
//#include <chrono>
//#include <cstdint>
//#include <system_error>
#include <random>
#include <locale.h>

#include <nudf\web\date_time.hpp>

namespace NX
{
/// Various utilities for string conversions and date and time manipulation.
namespace utility
{

/// Functions for string conversions.
namespace conversions
{
/// <summary>
/// Converts a UTF-16 string to a UTF-8 string
/// </summary>
std::string __cdecl utf16_to_utf8(const std::wstring &w);

/// <summary>
/// Converts a UTF-8 string to a UTF-16
/// </summary>
std::wstring __cdecl utf8_to_utf16(const std::string &s);

/// <summary>
/// Converts a ASCII (us-ascii) string to a UTF-16 string.
/// </summary>
std::wstring __cdecl usascii_to_utf16(const std::string &s);

/// <summary>
/// Converts a Latin1 (iso-8859-1) string to a UTF-16 string.
/// </summary>
std::wstring __cdecl latin1_to_utf16(const std::string &s);

/// <summary>
/// Converts a string with the OS's default code page to a UTF-16 string.
/// </summary>
std::wstring __cdecl default_code_page_to_utf16(const std::string &s);

/// <summary>
/// Decode to string_t from either a utf-16 or utf-8 string
/// </summary>
std::wstring __cdecl to_string(std::string &&s);
std::wstring __cdecl to_string(std::wstring &&s);
std::wstring __cdecl to_string(const std::string &s);
std::wstring __cdecl to_string(const std::wstring &s);

/// <summary>
/// Decode to utf16 from either a narrow or wide string
/// </summary>
std::wstring __cdecl to_utf16string(const std::string &value);
std::wstring __cdecl to_utf16string(std::wstring value);

/// <summary>
/// Decode to UTF-8 from either a narrow or wide string.
/// </summary>
std::string __cdecl to_utf8string(std::string value);
std::string __cdecl to_utf8string(const std::wstring &value);

/// <summary>
/// Encode the given byte array into a base64 string
/// </summary>
std::wstring __cdecl to_base64(const std::vector<unsigned char>& data);

/// <summary>
/// Encode the given 8-byte integer into a base64 string
/// </summary>
std::wstring __cdecl to_base64(uint64_t data);

/// <summary>
/// Decode the given base64 string to a byte array
/// </summary>
std::vector<unsigned char> __cdecl from_base64(const std::wstring& str);

template <typename Source>
std::wstring print_string(const Source &val)
{
    std::wostringstream oss;
    oss << val;
    if (oss.bad())
        throw std::bad_cast();
    return oss.str();
}
template <typename Target>
Target scan_string(const std::wstring &str)
{
    Target t;
    utility::istringstream_t iss(str);
    iss >> t;
    if (iss.bad())
        throw std::bad_cast();
    return t;
}
}   // namespace conversions

namespace details
{
/// <summary>
/// Cross platform RAII container for setting thread local locale.
/// </summary>
class scoped_c_thread_locale
{
public:
    scoped_c_thread_locale();
    ~scoped_c_thread_locale();

    typedef _locale_t xplat_locale;

    static xplat_locale __cdecl c_locale();

private:
    std::string m_prevLocale;
    int m_prevThreadSetting;
    scoped_c_thread_locale(const scoped_c_thread_locale &);
    scoped_c_thread_locale & operator=(const scoped_c_thread_locale &);
};

/// <summary>
/// Our own implementation of alpha numeric instead of std::isalnum to avoid
/// taking global lock for performance reasons.
/// </summary>
inline bool __cdecl is_alnum(char ch)
{
    return (ch >= '0' && ch <= '9')
        || (ch >= 'A' && ch <= 'Z')
        || (ch >= 'a' && ch <= 'z');
}

/// <summary>
/// Simplistic implementation of make_unique. A better implementation would be based on variadic templates
/// and therefore not be compatible with Dev10.
/// </summary>
template <typename _Type>
std::unique_ptr<_Type> make_unique() {
    return std::unique_ptr<_Type>(new _Type());
}

template <typename _Type, typename _Arg1>
std::unique_ptr<_Type> make_unique(_Arg1&& arg1) {
    return std::unique_ptr<_Type>(new _Type(std::forward<_Arg1>(arg1)));
}

template <typename _Type, typename _Arg1, typename _Arg2>
std::unique_ptr<_Type> make_unique(_Arg1&& arg1, _Arg2&& arg2) {
    return std::unique_ptr<_Type>(new _Type(std::forward<_Arg1>(arg1), std::forward<_Arg2>(arg2)));
}

template <typename _Type, typename _Arg1, typename _Arg2, typename _Arg3>
std::unique_ptr<_Type> make_unique(_Arg1&& arg1, _Arg2&& arg2, _Arg3&& arg3) {
    return std::unique_ptr<_Type>(new _Type(std::forward<_Arg1>(arg1), std::forward<_Arg2>(arg2), std::forward<_Arg3>(arg3)));
}

/// <summary>
/// Cross platform utility function for performing case insensitive string comparision.
/// </summary>
/// <param name="left">First string to compare.</param>
/// <param name="right">Second strong to compare.</param>
/// <returns>true if the strings are equivalent, false otherwise</returns>
inline bool str_icmp(const std::wstring &left, const std::wstring &right)
{
    return _wcsicmp(left.c_str(), right.c_str()) == 0;
}
 
}   // namespace details



/// <summary>
/// Nonce string generator class.
/// </summary>
class nonce_generator
{
public:

    /// <summary>
    /// Define default nonce length.
    /// </summary>
    enum { default_length = 32 };

    /// <summary>
    /// Nonce generator constructor.
    /// </summary>
    /// <param name="length">Length of the generated nonce string.</param>
    nonce_generator(int length=default_length) :
        m_random(static_cast<unsigned int>(NX::utility::datetime::utc_timestamp())),
        m_length(length)
    {}

    /// <summary>
    /// Generate a nonce string containing random alphanumeric characters (A-Za-z0-9).
    /// Length of the generated string is set by length().
    /// </summary>
    /// <returns>The generated nonce string.</returns>
    std::wstring generate();

    /// <summary>
    /// Get length of generated nonce string.
    /// </summary>
    /// <returns>Nonce string length.</returns>
    int length() const { return m_length; }

    /// <summary>
    /// Set length of the generated nonce string.
    /// </summary>
    /// <param name="length">Lenght of nonce string.</param>
    void set_length(int length) { m_length = length; }

private:
    static const std::wstring c_allowed_chars;
    std::mt19937 m_random;
    int m_length;
};

} // namespace utility;

} // namespace NX;



#endif