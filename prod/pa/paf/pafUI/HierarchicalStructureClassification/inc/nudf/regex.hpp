

#ifndef _NUDF_REGEX_HPP__
#define _NUDF_REGEX_HPP__


#include <Windows.h>
#include <assert.h>
#include <string>
#include <regex>

namespace nudf {
namespace util {
namespace regex {

template <typename T>
std::basic_string<T> ReplaceAll(_In_ const std::basic_string<T>& source, _In_ const std::basic_string<T>& rgx_pattern, _In_ const std::basic_string<T>& replacement, _In_ bool ignorecase)
{
    std::tr1::basic_regex<T> rgx = std::tr1::basic_regex<T>(rgx_pattern.c_str(), ignorecase?std::tr1::regex_constants::icase : std::tr1::regex_constants::ECMAScript);
    std::basic_string<T> s = std::regex_replace(source, rgx, replacement);
    return s;
}

template <typename T>
std::basic_string<T> SimpleReplaceAll(_In_ const std::basic_string<T>& source, _In_ const std::basic_string<T>& str_pattern, _In_ const std::basic_string<T>& replacement, _In_ bool ignorecase)
{
    std::basic_string<T> rgx_pattern = NormalizePattern<T>(str_pattern);
    std::tr1::basic_regex<T> rgx = std::tr1::basic_regex<T>(rgx_pattern.c_str(), ignorecase?std::tr1::regex_constants::icase : std::tr1::regex_constants::ECMAScript);
    std::basic_string<T> s = std::regex_replace(source, rgx, replacement);
    return s;
}

template <typename T>
std::basic_string<T> TrimAll(_In_ const std::basic_string<T>& source, _In_ T ch, _In_ bool ignorecase)
{
    std::basic_string<T> replacement;
    std::basic_string<T> rgx_pattern;
    std::basic_string<T> str_pattern(&ch, 1);
    rgx_pattern = NormalizePattern<T>(str_pattern);
    rgx_pattern += (T*)L"*";
    std::tr1::basic_regex<T> rgx = std::tr1::basic_regex<T>(rgx_pattern.c_str(), ignorecase?std::tr1::regex_constants::icase : std::tr1::regex_constants::ECMAScript);
    std::basic_string<T> s = std::regex_replace(source, rgx, replacement);
    return s;
}

template <typename T>
std::basic_string<T> TrimLeft(_In_ const std::basic_string<T>& source, _In_ T ch, _In_ bool ignorecase)
{
    std::basic_string<T> replacement;
    std::basic_string<T> rgx_pattern((T*)L"^");
    std::basic_string<T> str_pattern(&ch, 1);
    rgx_pattern += NormalizePattern<T>(str_pattern);
    rgx_pattern += (T*)L"*";
    std::tr1::basic_regex<T> rgx = std::tr1::basic_regex<T>(rgx_pattern.c_str(), ignorecase?std::tr1::regex_constants::icase : std::tr1::regex_constants::ECMAScript);
    std::basic_string<T> s = std::regex_replace(source, rgx, replacement);
    return s;
}

template <typename T>
std::basic_string<T> TrimRight(_In_ const std::basic_string<T>& source, _In_ T ch, _In_ bool ignorecase)
{
    std::basic_string<T> replacement;
    std::basic_string<T> rgx_pattern;
    std::basic_string<T> str_pattern(&ch, 1);
    rgx_pattern = NormalizePattern<T>(str_pattern);
    rgx_pattern += (T*)L"*";
    rgx_pattern += (T*)L"$";
    std::tr1::basic_regex<T> rgx = std::tr1::basic_regex<T>(rgx_pattern.c_str(), ignorecase?std::tr1::regex_constants::icase : std::tr1::regex_constants::ECMAScript);
    std::basic_string<T> s = std::regex_replace(source, rgx, replacement);
    return s;
}

template <typename T>
std::basic_string<T> NormalizePattern(_In_ const std::basic_string<T>& str_pattern)
{
    std::basic_string<T> s;

    // Normalize pattern
    s = ReplaceAll<T>(str_pattern, L"\\\\", L"\\\\", false);
    s = ReplaceAll<T>(s, L"\\^",  L"\\^", false);
    s = ReplaceAll<T>(s, L"\\.",  L"\\.", false);
    s = ReplaceAll<T>(s, L"\\$",  L"\\$", false);
    s = ReplaceAll<T>(s, L"\\|",  L"\\|", false);
    s = ReplaceAll<T>(s, L"\\(",  L"\\(", false);
    s = ReplaceAll<T>(s, L"\\)",  L"\\)", false);
    s = ReplaceAll<T>(s, L"\\[",  L"\\[", false);
    s = ReplaceAll<T>(s, L"\\]",  L"\\]", false);
    s = ReplaceAll<T>(s, L"\\*",  L"\\*", false);
    s = ReplaceAll<T>(s, L"\\+",  L"\\+", false);
    s = ReplaceAll<T>(s, L"\\?",  L"\\?", false);
    s = ReplaceAll<T>(s, L"\\/",  L"\\/", false);

    return s;
}

template <typename T>
std::basic_string<T> WildcardsToRegex(_In_ const std::basic_string<T>& str_pattern)
{
    std::basic_string<T> s;

    // Normalize pattern
    s = NormalizePattern<T>(str_pattern);

    // convert wildcards
    s = ReplaceAll<T>(s, L"\\\\\\*\\\\\\*",  L".*", false);
    s = ReplaceAll<T>(s, L"\\\\\\*",  L"[^\\\\]*", false);
    s = ReplaceAll<T>(s, L"\\\\\\?",  L".", false);

    return s;
}

template <typename T>
std::basic_string<T> WildcardsToRegexEx(_In_ const std::basic_string<T>& str_pattern)
{
    std::basic_string<T> s;
    
    // Normalize pattern
    s = NormalizePattern<T>(str_pattern);

    // convert wildcards
    s = ReplaceAll<T>(s, L"\\\\\\*\\\\\\*",  L".*", false);
    s = ReplaceAll<T>(s, L"\\\\\\*",  L"[^\\\\]*", false);

    // also convert NextLabs wildcards
    s = ReplaceAll<T>(s, L"\\\\\\?d",  L"\\d", false);
    s = ReplaceAll<T>(s, L"\\\\\\?D",  L"\\d*", false);
    s = ReplaceAll<T>(s, L"\\\\\\?a",  L"\\w", false);
    s = ReplaceAll<T>(s, L"\\\\\\?A",  L"\\w*", false);
    s = ReplaceAll<T>(s, L"\\\\\\?c",  L"[^\\\\]", false);
    s = ReplaceAll<T>(s, L"\\\\\\?C",  L"[^\\\\]*", false);
    s = ReplaceAll<T>(s, L"\\\\\\?s",  L"\\s", false);
    s = ReplaceAll<T>(s, L"\\\\\\?S",  L"\\s*", false);
    s = ReplaceAll<T>(s, L"\\\\\\?",  L"\\.", false);
    s = ReplaceAll<T>(s, L"\\\\\\?\\\\\\?",  L"\\?", false);
    
    s = ReplaceAll<T>(s, L"!d",  L"\\D", false);
    s = ReplaceAll<T>(s, L"!D",  L"\\D*", false);
    s = ReplaceAll<T>(s, L"!a",  L"\\W", false);
    s = ReplaceAll<T>(s, L"!A",  L"\\W*", false);
    s = ReplaceAll<T>(s, L"!c",  L"\\S", false);
    s = ReplaceAll<T>(s, L"!s",  L"\\S", false);
    s = ReplaceAll<T>(s, L"!S",  L"\\S*", false);
    s = ReplaceAll<T>(s, L"!!",  L"!", false);

    return s;
}

template <typename T>
bool Match(_In_ const std::basic_string<T>& source, _In_ const std::basic_string<T>& rgx_pattern, _In_ bool ignorecase)
{
    std::tr1::basic_regex<T> rgx = std::tr1::basic_regex<T>(rgx_pattern.c_str(), ignorecase?std::tr1::regex_constants::icase : std::tr1::regex_constants::ECMAScript);
    return std::tr1::regex_match(source, rgx);
}
    
}   // namespace nudf::util::regex
}   // namespace nudf::util
}   // namespace nudf

#endif  // _NUDF_REGEX_HPP__