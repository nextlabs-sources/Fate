

#include <Windows.h>
#include <Winhttp.h>

#include <array>


#include <nudf\web\error_category.hpp>
#include <nudf\web\conversions.hpp>
#include <nudf\web\json.hpp>



using namespace NX;
using namespace NX::utility;
using namespace NX::utility::conversions;


    
std::unique_ptr<NX::utility::details::scoped_c_thread_locale::xplat_locale, void(*)(NX::utility::details::scoped_c_thread_locale::xplat_locale *)> g_c_locale(nullptr, [](NX::utility::details::scoped_c_thread_locale::xplat_locale *){});

class locale_once
{
public:
    locale_once() : _called(false)
    {
        ::InitializeCriticalSection(&_cs);
        call_once();
    }

    ~locale_once()
    {
        ::DeleteCriticalSection(&_cs);
    }

private:
    void call_once()
    {
        bool first_time = false;
        ::EnterCriticalSection(&_cs);
        if(!_called) {
            _called = true;
            first_time = true;
        }
        ::LeaveCriticalSection(&_cs);

        if(!first_time) {
            return;
        }

        NX::utility::details::scoped_c_thread_locale::xplat_locale *clocale = new NX::utility::details::scoped_c_thread_locale::xplat_locale();

        *clocale = _create_locale(LC_ALL, "C");
        if (clocale == nullptr)
        {
            throw std::runtime_error("Unable to create 'C' locale.");
        }
        auto deleter = [](NX::utility::details::scoped_c_thread_locale::xplat_locale *clocale)
        {
            _free_locale(*clocale);
        };
        g_c_locale = std::unique_ptr<NX::utility::details::scoped_c_thread_locale::xplat_locale, void(*)(NX::utility::details::scoped_c_thread_locale::xplat_locale *)>(clocale, deleter);
    }

private:
    CRITICAL_SECTION    _cs;
    bool                _called;
};


NX::utility::details::scoped_c_thread_locale::xplat_locale NX::utility::details::scoped_c_thread_locale::c_locale()
{
    static locale_once g_c_localeFlag;
    return *g_c_locale;
}

NX::utility::details::scoped_c_thread_locale::scoped_c_thread_locale()
    : m_prevLocale(), m_prevThreadSetting(-1)
{
    char *prevLocale = setlocale(LC_ALL, nullptr);
    if (prevLocale == nullptr)
    {
        throw std::runtime_error("Unable to retrieve current locale.");
    }

    if (std::strcmp(prevLocale, "C") != 0)
    {
        m_prevLocale = prevLocale;
        m_prevThreadSetting = _configthreadlocale(_ENABLE_PER_THREAD_LOCALE);
        if (m_prevThreadSetting == -1)
        {
            throw std::runtime_error("Unable to enable per thread locale.");
        }
        if (setlocale(LC_ALL, "C") == nullptr)
        {
             _configthreadlocale(m_prevThreadSetting);
             throw std::runtime_error("Unable to set locale");
        }
    }
}

NX::utility::details::scoped_c_thread_locale::~scoped_c_thread_locale()
{
    if (m_prevThreadSetting != -1)
    {
        setlocale(LC_ALL, m_prevLocale.c_str());
        _configthreadlocale(m_prevThreadSetting);
    }
}


std::wstring __cdecl NX::utility::conversions::utf8_to_utf16(const std::string &s)
{
    if(s.empty())
    {
        return std::wstring();
    }

    // first find the size
    int size = ::MultiByteToWideChar(
        CP_UTF8, // convert to utf-8
        MB_ERR_INVALID_CHARS, // fail if any characters can't be translated
        s.c_str(),
        (int)s.size(),
        nullptr, 0); // must be null for utf8

    if (size == 0)
    {
        throw NX::utility::create_system_error(GetLastError());
    }

    std::wstring buffer;
    buffer.resize(size);

    // now call again to format the string
    const int result = ::MultiByteToWideChar(
        CP_UTF8, // convert to utf-8
        MB_ERR_INVALID_CHARS, // fail if any characters can't be translated
        s.c_str(),
        (int)s.size(),
        &buffer[0], size); // must be null for utf8

    if (result != size)
    {
        throw NX::utility::create_system_error(GetLastError());
    }

    return buffer;
}

std::string __cdecl NX::utility::conversions::utf16_to_utf8(const std::wstring &w)
{
    if(w.empty())
    {
        return std::string();
    }

    // first find the size
    const int size = ::WideCharToMultiByte(
        CP_UTF8, // convert to utf-8
#if _WIN32_WINNT >= _WIN32_WINNT_VISTA
        WC_ERR_INVALID_CHARS, // fail if any characters can't be translated
#else
        0, // ERROR_INVALID_FLAGS is not supported in XP, set this dwFlags to 0
#endif // _WIN32_WINNT >= _WIN32_WINNT_VISTA
        w.c_str(),
        (int)w.size(),
        nullptr, 0, // find the size required
        nullptr, nullptr); // must be null for utf8

    if (size == 0)
    {
        throw NX::utility::create_system_error(GetLastError());
    }

    std::string buffer;
    buffer.resize(size);

    // now call again to format the string
    const int result = ::WideCharToMultiByte(
        CP_UTF8, // convert to utf-8
#if _WIN32_WINNT >= _WIN32_WINNT_VISTA
        WC_ERR_INVALID_CHARS, // fail if any characters can't be translated
#else
        0, // ERROR_INVALID_FLAGS is not supported in XP, set this dwFlags to 0
#endif // _WIN32_WINNT >= _WIN32_WINNT_VISTA
        w.c_str(),
        (int)w.size(),
        &buffer[0], size,
        nullptr, nullptr); // must be null for utf8

    if (result != size)
    {
        throw NX::utility::create_system_error(GetLastError());
    }

    return buffer;
}

std::wstring __cdecl NX::utility::conversions::usascii_to_utf16(const std::string &s)
{
    if(s.empty())
    {
        return std::wstring();
    }

    int size = ::MultiByteToWideChar(
        20127, // convert from us-ascii
        MB_ERR_INVALID_CHARS, // fail if any characters can't be translated
        s.c_str(),
        (int)s.size(),
        nullptr, 0);

    if (size == 0)
    {
        throw NX::utility::create_system_error(GetLastError());
    }

    // this length includes the terminating null
    std::wstring buffer;
    buffer.resize(size);

    // now call again to format the string
    int result = ::MultiByteToWideChar(
        20127, // convert from us-ascii
        MB_ERR_INVALID_CHARS, // fail if any characters can't be translated
        s.c_str(),
        (int)s.size(),
        &buffer[0], size);

    if (result != size)
    {
        throw NX::utility::create_system_error(GetLastError());
    }

    return buffer;
}

std::wstring __cdecl NX::utility::conversions::latin1_to_utf16(const std::string &s)
{
    if(s.empty())
    {
        return std::wstring();
    }

    int size = ::MultiByteToWideChar(
        28591, // convert from Latin1
        MB_ERR_INVALID_CHARS, // fail if any characters can't be translated
        s.c_str(),
        (int)s.size(),
        nullptr, 0);

    if (size == 0)
    {
        throw NX::utility::create_system_error(GetLastError());
    }

    // this length includes the terminating null
    std::wstring buffer;
    buffer.resize(size);

    // now call again to format the string
    int result = ::MultiByteToWideChar(
        28591, // convert from Latin1
        MB_ERR_INVALID_CHARS, // fail if any characters can't be translated
        s.c_str(),
        (int)s.size(),
        &buffer[0], size);

    if (result != size)
    {
        throw NX::utility::create_system_error(GetLastError());
    }

    return buffer;
}

std::wstring __cdecl NX::utility::conversions::default_code_page_to_utf16(const std::string &s)
{
    if(s.empty())
    {
        return std::wstring();
    }

    // First have to convert to UTF-16.
    int size = ::MultiByteToWideChar(
        CP_ACP, // convert from Windows system default
        MB_ERR_INVALID_CHARS, // fail if any characters can't be translated
        s.c_str(),
        (int)s.size(),
        nullptr, 0);
    if (size == 0)
    {
        throw NX::utility::create_system_error(GetLastError());
    }

    // this length includes the terminating null
    std::wstring buffer;
    buffer.resize(size);

    // now call again to format the string
    int result = ::MultiByteToWideChar(
        CP_ACP, // convert from Windows system default
        MB_ERR_INVALID_CHARS, // fail if any characters can't be translated
        s.c_str(),
        (int)s.size(),
        &buffer[0], size);
    if(result == size)
    {
        return buffer;
    }
    else
    {
        throw NX::utility::create_system_error(GetLastError());
    }
}

std::wstring __cdecl NX::utility::conversions::to_string(std::wstring &&s)
{
    return std::move(s);
}

std::wstring __cdecl conversions::to_string(std::string &&s)
{
    return utf8_to_utf16(std::move(s));
}

std::wstring __cdecl conversions::to_string(const std::wstring &s)
{
    return s;
}

std::wstring __cdecl conversions::to_string(const std::string &s)
{
    return utf8_to_utf16(s);
}

std::string __cdecl conversions::to_utf8string(std::string value) { return std::move(value); }

std::string __cdecl conversions::to_utf8string(const std::wstring &value) { return utf16_to_utf8(value); }

std::wstring __cdecl conversions::to_utf16string(const std::string &value) { return utf8_to_utf16(value); }

std::wstring __cdecl conversions::to_utf16string(std::wstring value) { return std::move(value); }

static bool is_digit(wchar_t c) { return (c >= L'0' && c <= L'9'); }
static bool is_not_digit(wchar_t c) { return (c < L'0' || c > L'9'); }


const std::wstring nonce_generator::c_allowed_chars(L"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789");

std::wstring nonce_generator::generate()
{
    std::uniform_int_distribution<> distr(0, static_cast<int>(c_allowed_chars.length() - 1));
    std::wstring result;
    result.reserve(length());
    std::generate_n(std::back_inserter(result), length(), [&]() { return c_allowed_chars[distr(m_random)]; } );
    return result;
}


//
//  base 64
//

#define _USE_INTERNAL_BASE64_
std::vector<unsigned char> _from_base64(const std::wstring& str);
std::wstring _to_base64(const unsigned char *ptr, size_t size);

std::vector<unsigned char> __cdecl conversions::from_base64(const std::wstring& str)
{
    return _from_base64(str);
}

std::wstring __cdecl conversions::to_base64(const std::vector<unsigned char>& input)
{
    if (input.size() == 0)
    {
        // return empty string
        return std::wstring();
    }

    return _to_base64(&input[0], input.size());
}

std::wstring __cdecl conversions::to_base64(uint64_t input)
{
    return _to_base64(reinterpret_cast<const unsigned char*>(&input), sizeof(input));
}


#if defined(_USE_INTERNAL_BASE64_)
static const char* _base64_enctbl = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
const std::array<unsigned char, 128> _base64_dectbl =
{ { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,  62, 255, 255, 255,  63,
52,  53,  54,  55,  56,  57,  58,  59,  60,  61, 255, 255, 255, 254, 255, 255,
255,  0,    1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,
15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25, 255, 255, 255, 255, 255,
255,  26,  27,  28,  29,  30,  31,  32,  33,  34,  35,  36,  37,  38,  39,  40,
41,  42,  43,  44,  45,  46,  47,  48,  49,  50,  51, 255, 255, 255, 255, 255 } };

struct _triple_byte
{
    unsigned char _1_1 : 2;
    unsigned char _0 : 6;
    unsigned char _2_1 : 4;
    unsigned char _1_2 : 4;
    unsigned char _3 : 6;
    unsigned char _2_2 : 2;
};

//
// A note on the implementation of BASE64 encoding and decoding:
//
// This is a fairly basic and naive implementation; there is probably a lot of room for
// performance improvement, as well as for adding options such as support for URI-safe base64,
// ignoring CRLF, relaxed validation rules, etc. The decoder is currently pretty strict.
//

std::vector<unsigned char> _from_base64(const std::wstring& input)
{
    std::vector<unsigned char> result;

    if (input.empty())
        return result;

    size_t padding = 0;

    // Validation
    {
        auto size = input.size();

        if ((size % 4) != 0)
        {
            throw std::runtime_error("length of base64 string is not an even multiple of 4");
        }

        for (auto iter = input.begin(); iter != input.end(); ++iter, --size)
        {
            const auto ch = *iter;
            if (ch < 0)
            {
                throw std::runtime_error("invalid character found in base64 string");
            }
            const size_t ch_sz = static_cast<size_t>(ch);
            if (ch_sz >= _base64_dectbl.size() || _base64_dectbl[ch_sz] == 255)
            {
                throw std::runtime_error("invalid character found in base64 string");
            }
            if (_base64_dectbl[ch_sz] == 254)
            {
                padding++;
                // padding only at the end
                if (size > 2)
                {
                    throw std::runtime_error("invalid padding character found in base64 string");
                }
                if (size == 2)
                {
                    const auto ch2 = *(iter + 1);
                    if (ch2 < 0)
                    {
                        throw std::runtime_error("invalid padding character found in base64 string");
                    }
                    const size_t ch2_sz = static_cast<size_t>(ch2);
                    if (ch2_sz >= _base64_dectbl.size() || _base64_dectbl[ch2_sz] != 254)
                    {
                        throw std::runtime_error("invalid padding character found in base64 string");
                    }
                }
            }
        }
    }


    auto size = input.size();
    const wchar_t* ptr = &input[0];

    auto outsz = (size / 4) * 3;
    outsz -= padding;

    result.resize(outsz);

    size_t idx = 0;
    for (; size > 4; ++idx)
    {
        unsigned char target[3];
        memset(target, 0, sizeof(target));
        _triple_byte* record = reinterpret_cast<_triple_byte*>(target);

        unsigned char val0 = _base64_dectbl[ptr[0]];
        unsigned char val1 = _base64_dectbl[ptr[1]];
        unsigned char val2 = _base64_dectbl[ptr[2]];
        unsigned char val3 = _base64_dectbl[ptr[3]];

        record->_0 = val0;
        record->_1_1 = val1 >> 4;
        result[idx] = target[0];

        record->_1_2 = val1 & 0xF;
        record->_2_1 = val2 >> 2;
        result[++idx] = target[1];

        record->_2_2 = val2 & 0x3;
        record->_3 = val3 & 0x3F;
        result[++idx] = target[2];

        ptr += 4;
        size -= 4;
    }

    // Handle the last four bytes separately, to avoid having the conditional statements
    // in all the iterations (a performance issue).

    {
        unsigned char target[3];
        memset(target, 0, sizeof(target));
        _triple_byte* record = reinterpret_cast<_triple_byte*>(target);

        unsigned char val0 = _base64_dectbl[ptr[0]];
        unsigned char val1 = _base64_dectbl[ptr[1]];
        unsigned char val2 = _base64_dectbl[ptr[2]];
        unsigned char val3 = _base64_dectbl[ptr[3]];

        record->_0 = val0;
        record->_1_1 = val1 >> 4;
        result[idx] = target[0];

        record->_1_2 = val1 & 0xF;
        if (val2 != 254)
        {
            record->_2_1 = val2 >> 2;
            result[++idx] = target[1];
        }
        else
        {
            // There shouldn't be any information (ones) in the unused bits,
            if (record->_1_2 != 0)
            {
                throw std::runtime_error("Invalid end of base64 string");
            }
            return result;
        }

        record->_2_2 = val2 & 0x3;
        if (val3 != 254)
        {
            record->_3 = val3 & 0x3F;
            result[++idx] = target[2];
        }
        else
        {
            // There shouldn't be any information (ones) in the unused bits.
            if (record->_2_2 != 0)
            {
                throw std::runtime_error("Invalid end of base64 string");
            }
            return result;
        }
    }

    return result;
}

std::wstring _to_base64(const unsigned char *ptr, size_t size)
{
    std::wstring result;

    for (; size >= 3; )
    {
        const _triple_byte* record = reinterpret_cast<const _triple_byte*>(ptr);
        unsigned char idx0 = record->_0;
        unsigned char idx1 = (record->_1_1 << 4) | record->_1_2;
        unsigned char idx2 = (record->_2_1 << 2) | record->_2_2;
        unsigned char idx3 = record->_3;
        result.push_back(wchar_t(_base64_enctbl[idx0]));
        result.push_back(wchar_t(_base64_enctbl[idx1]));
        result.push_back(wchar_t(_base64_enctbl[idx2]));
        result.push_back(wchar_t(_base64_enctbl[idx3]));
        size -= 3;
        ptr += 3;
    }
    switch (size)
    {
    case 1:
    {
        const _triple_byte* record = reinterpret_cast<const _triple_byte*>(ptr);
        unsigned char idx0 = record->_0;
        unsigned char idx1 = (record->_1_1 << 4);
        result.push_back(wchar_t(_base64_enctbl[idx0]));
        result.push_back(wchar_t(_base64_enctbl[idx1]));
        result.push_back('=');
        result.push_back('=');
        break;
    }
    case 2:
    {
        const _triple_byte* record = reinterpret_cast<const _triple_byte*>(ptr);
        unsigned char idx0 = record->_0;
        unsigned char idx1 = (record->_1_1 << 4) | record->_1_2;
        unsigned char idx2 = (record->_2_1 << 2);
        result.push_back(wchar_t(_base64_enctbl[idx0]));
        result.push_back(wchar_t(_base64_enctbl[idx1]));
        result.push_back(wchar_t(_base64_enctbl[idx2]));
        result.push_back('=');
        break;
    }
    }
    return result;
}
#endif  // #if defined(_USE_INTERNAL_BASE64_)
