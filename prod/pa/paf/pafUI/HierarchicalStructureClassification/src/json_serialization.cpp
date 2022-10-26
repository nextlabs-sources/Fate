

#include <Windows.h>
#include <stdio.h>


#include <nudf\web\json.hpp>
#include <nudf\web\conversions.hpp>


using namespace NX;
using namespace NX::web;
using namespace NX::web::json;

//
// JSON Serialization
//

void json::value::serialize(std::ostream& stream) const
{
    // This has better performance than writing directly to stream.
    std::string str;
    m_value->serialize_impl(str);
    stream << str;
}
void json::value::format(std::basic_string<wchar_t> &string) const
{
    m_value->format(string);
}

void json::value::serialize(std::wostream &stream) const
{
    // This has better performance than writing directly to stream.
    std::wstring str;
    m_value->serialize_impl(str);
    stream << str;
}

void json::value::format(std::basic_string<char>& string) const
{
    m_value->format(string);
}

template<typename CharType>
void json::details::append_escape_string(std::basic_string<CharType>& str, const std::basic_string<CharType>& escaped)
{
    for (auto iter = escaped.begin(); iter != escaped.end(); ++iter)
    {
        switch (*iter)
        {
            case '\"':
                str += '\\';
                str += '\"';
                break;
            case '\\':
                str += '\\';
                str += '\\';
                break;
            case '\b':
                str += '\\';
                str += 'b';
                break;
            case '\f':
                str += '\\';
                str += 'f';
                break;
            case '\r':
                str += '\\';
                str += 'r';
                break;
            case '\n':
                str += '\\';
                str += 'n';
                break;
            case '\t':
                str += '\\';
                str += 't';
                break;
            default:
                str += *iter;
        }
    }
}

void json::details::format_string(const std::wstring& key, std::wstring& str)
{
    str.push_back('"');
    append_escape_string(str, key);
    str.push_back('"');
}

void json::details::format_string(const std::wstring& key, std::string& str)
{
    str.push_back('"');
    append_escape_string(str, utility::conversions::to_utf8string(key));
    str.push_back('"');
}

void json::details::_String::format(std::basic_string<char>& str) const
{
    str.push_back('"');

    if(m_has_escape_char)
    {
        append_escape_string(str, utility::conversions::to_utf8string(m_string));
    }
    else
    {
        str.append(utility::conversions::to_utf8string(m_string));
    }

    str.push_back('"');
}

void json::details::_Number::format(std::basic_string<char>& stream) const
{
    if(m_number.m_type != number::type::double_type)
    {
        // #digits + 1 to avoid loss + 1 for the sign + 1 for null terminator.
        const size_t tempSize = std::numeric_limits<uint64_t>::digits10 + 3;
        char tempBuffer[tempSize];

        // This can be improved performance-wise if we implement our own routine
        if (m_number.m_type == number::type::signed_type)
            _i64toa_s(m_number.m_intval, tempBuffer, tempSize, 10);
        else
            _ui64toa_s(m_number.m_uintval, tempBuffer, tempSize, 10);

        const auto numChars = strnlen_s(tempBuffer, tempSize);
        stream.append(tempBuffer, numChars);
    }
    else
    {
        // #digits + 2 to avoid loss + 1 for the sign + 1 for decimal point + 5 for exponent (e+xxx) + 1 for null terminator
        const size_t tempSize = std::numeric_limits<double>::digits10 + 10;
        char tempBuffer[tempSize];

        const auto numChars = _sprintf_s_l(
            tempBuffer,
            tempSize,
            "%.*g",
            utility::details::scoped_c_thread_locale::c_locale(),
            std::numeric_limits<double>::digits10 + 2,
            m_number.m_value);
        stream.append(tempBuffer, numChars);
    }
}

void json::details::_String::format(std::basic_string<wchar_t>& str) const
{
    str.push_back(L'"');

    if(m_has_escape_char)
    {
        append_escape_string(str, m_string);
    }
    else
    {
        str.append(m_string);
    }

    str.push_back(L'"');
}

void json::details::_Number::format(std::basic_string<wchar_t>& stream) const
{
    if(m_number.m_type != number::type::double_type)
    {
        // #digits + 1 to avoid loss + 1 for the sign + 1 for null terminator.
        const size_t tempSize = std::numeric_limits<uint64_t>::digits10 + 3;
        wchar_t tempBuffer[tempSize];

        if (m_number.m_type == number::type::signed_type)
            _i64tow_s(m_number.m_intval, tempBuffer, tempSize, 10);
        else
            _ui64tow_s(m_number.m_uintval, tempBuffer, tempSize, 10);

        stream.append(tempBuffer, wcsnlen_s(tempBuffer, tempSize));
    }
    else
    {
        // #digits + 2 to avoid loss + 1 for the sign + 1 for decimal point + 5 for exponent (e+xxx) + 1 for null terminator
        const size_t tempSize = std::numeric_limits<double>::digits10 + 10;
        wchar_t tempBuffer[tempSize];
        const int numChars = _swprintf_s_l(
            tempBuffer,
            tempSize,
            L"%.*g",
            utility::details::scoped_c_thread_locale::c_locale(),
            std::numeric_limits<double>::digits10 + 2,
            m_number.m_value);
        stream.append(tempBuffer, numChars);
    }
}

std::wstring json::details::_String::as_string() const
{
    return m_string;
}

std::wstring json::value::as_string() const
{
    return m_value->as_string();
}

std::wstring json::value::serialize() const
{
    return m_value->to_string();
}
