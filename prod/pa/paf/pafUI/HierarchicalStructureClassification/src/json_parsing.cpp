
#include <Windows.h>

#include <vector>
#include <cstdlib>
#include <array>

#include <nudf\web\json.hpp>
#include <nudf\web\conversions.hpp>

#if defined(_MSC_VER)
#pragma warning(disable : 4127) // allow expressions like while(true) pass
#endif


using namespace NX;
using namespace NX::web;

std::array<signed char,128> _hexval = {{ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                         -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                         -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                          0,  1,  2,  3,  4,  5,  6,  7,  8,  9, -1, -1, -1, -1, -1, -1,
                                         -1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                         -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                         -1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                         -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 }};

namespace NX
{
namespace web
{
namespace json
{
namespace details
{

//
// JSON Parsing
//

template <typename Token>
__declspec(noreturn)
void CreateException(const Token &tk, const std::wstring &message)
{
    std::wostringstream os;
    os << L"* Line " << tk.start.m_line << L", Column " << tk.start.m_column << L" Syntax error: " << message;
    throw json::json_exception(os.str().c_str());
}

template <typename Token>
void SetErrorCode(Token &tk, json_error jsonErrorCode)
{
    tk.m_error = std::error_code(jsonErrorCode, json_error_category());
}

template <typename CharType>
class JSON_Parser
{
public:
    JSON_Parser()
        : m_currentLine(1),
          m_eof(std::char_traits<CharType>::eof()),
          m_currentColumn(1),
          m_currentParsingDepth(0)
    { }

    struct Location
    {
        size_t m_line;
        size_t m_column;
    };

    struct Token
    {
        enum Kind
        {
            TKN_EOF,

            TKN_OpenBrace,
            TKN_CloseBrace,
            TKN_OpenBracket,
            TKN_CloseBracket,
            TKN_Comma,
            TKN_Colon,
            TKN_StringLiteral,
            TKN_NumberLiteral,
            TKN_IntegerLiteral,
            TKN_BooleanLiteral,
            TKN_NullLiteral,
            TKN_Comment
        };

        Token() : kind(TKN_EOF) {}

        Kind kind;
        std::basic_string<CharType> string_val;

        typename JSON_Parser<CharType>::Location start;

        union
        {
            double double_val;
            int64_t int64_val;
            uint64_t uint64_val;
            bool boolean_val;
            bool has_unescape_symbol;
        };

        bool signed_number;

        std::error_code m_error;
    };

    void GetNextToken(Token &);

    NX::web::json::value ParseValue(typename JSON_Parser<CharType>::Token &first)
    {
#ifdef ENABLE_JSON_VALUE_VISUALIZER
        auto _value = _ParseValue(first);
        auto type = _value->type();
        return json::value(std::move(_value), type);
#else
        return json::value(_ParseValue(first));
#endif
    }

protected:
    virtual CharType NextCharacter() = 0;
    virtual CharType PeekCharacter() = 0;

    virtual bool CompleteComment(Token &token);
    virtual bool CompleteStringLiteral(Token &token);
    bool handle_unescape_char(Token &token);

private:

    bool CompleteNumberLiteral(CharType first, Token &token);
    bool ParseInt64(CharType first, uint64_t& value);
    bool CompleteKeywordTrue(Token &token);
    bool CompleteKeywordFalse(Token &token);
    bool CompleteKeywordNull(Token &token);
    std::unique_ptr<json::details::_Value> _ParseValue(typename JSON_Parser<CharType>::Token &first);
    std::unique_ptr<json::details::_Value> _ParseObject(typename JSON_Parser<CharType>::Token &tkn);
    std::unique_ptr<json::details::_Value> _ParseArray(typename JSON_Parser<CharType>::Token &tkn);

    JSON_Parser& operator=(const JSON_Parser&);

    CharType EatWhitespace();

    void CreateToken(typename JSON_Parser<CharType>::Token& tk, typename Token::Kind kind, Location &start)
    {
        tk.kind = kind;
        tk.start = start;
        tk.string_val.clear();
    }

    void CreateToken(typename JSON_Parser<CharType>::Token& tk, typename Token::Kind kind)
    {
        tk.kind = kind;
        tk.start.m_line = m_currentLine;
        tk.start.m_column = m_currentColumn;
        tk.string_val.clear();
    }

protected:

    size_t m_currentLine;
    const typename std::char_traits<CharType>::int_type m_eof;
    size_t m_currentColumn;
    size_t m_currentParsingDepth;

// The DEBUG macro is defined in XCode but we don't in our CMakeList
// so for now we will keep the same on debug and release. In the future
// this can be increase on release if necessary.
    static const size_t maxParsingDepth = 128;
};

template <typename CharType>
class JSON_StreamParser : public JSON_Parser<CharType>
    {
public:
    JSON_StreamParser(std::basic_istream<CharType> &stream)
        : m_streambuf(stream.rdbuf())
    {
    }

protected:

    virtual CharType NextCharacter();
    virtual CharType PeekCharacter();

private:
    typename std::basic_streambuf<CharType, std::char_traits<CharType>>* m_streambuf;
};

template <typename CharType>
class JSON_StringParser : public JSON_Parser<CharType>
{
public:
    JSON_StringParser(const std::basic_string<CharType>& string)
        : m_position(&string[0])
    {
        m_startpos = m_position;
        m_endpos = m_position+string.length();
    }

protected:

    virtual CharType NextCharacter();
    virtual CharType PeekCharacter();

    virtual bool CompleteComment(typename JSON_Parser<CharType>::Token &token);
    virtual bool CompleteStringLiteral(typename JSON_Parser<CharType>::Token &token);

private:
    bool finish_parsing_string_with_unescape_char(typename JSON_Parser<CharType>::Token &token);
    const CharType* m_position;
    const CharType* m_startpos;
    const CharType* m_endpos;
};


template <typename CharType>
CharType JSON_StreamParser<CharType>::NextCharacter()
{
    CharType ch = (CharType) m_streambuf->sbumpc();

    if (ch == '\n')
    {
        this->m_currentLine += 1;
        this->m_currentColumn = 0;
    }
    else
    {
        this->m_currentColumn += 1;
    }

    return (CharType)ch;
}

template <typename CharType>
CharType JSON_StreamParser<CharType>::PeekCharacter()
{
    return (CharType)m_streambuf->sgetc();
}

template <typename CharType>
CharType JSON_StringParser<CharType>::NextCharacter()
{
    if (*m_position == 0 || m_position == m_endpos)
        return (CharType)this->m_eof;

    CharType ch = *m_position;
    m_position += 1;

    if ( ch == '\n' )
    {
        this->m_currentLine += 1;
        this->m_currentColumn = 0;
    }
    else
    {
        this->m_currentColumn += 1;
    }

    return (CharType)ch;
}

template <typename CharType>
CharType JSON_StringParser<CharType>::PeekCharacter()
{
    if ( m_position == m_endpos ) return (CharType)this->m_eof;

    return (CharType)*m_position;
}

//
// Consume whitespace characters and return the first non-space character or EOF
//
template <typename CharType>
CharType JSON_Parser<CharType>::EatWhitespace()
{
   CharType ch = NextCharacter();

   while ( ch != this->m_eof && iswspace((int)ch) )
   {
       ch = NextCharacter();
   }

   return ch;
}

template <typename CharType>
bool JSON_Parser<CharType>::CompleteKeywordTrue(Token &token)
{
    if (NextCharacter() != 'r')
        return false;
    if (NextCharacter() != 'u')
        return false;
    if (NextCharacter() != 'e')
        return false;
    token.kind = Token::TKN_BooleanLiteral;
    token.boolean_val = true;
    return true;
}

template <typename CharType>
bool JSON_Parser<CharType>::CompleteKeywordFalse(Token &token)
{
    if (NextCharacter() != 'a')
        return false;
    if (NextCharacter() != 'l')
        return false;
    if (NextCharacter() != 's')
        return false;
    if (NextCharacter() != 'e')
        return false;
    token.kind = Token::TKN_BooleanLiteral;
    token.boolean_val = false;
    return true;
}

template <typename CharType>
bool JSON_Parser<CharType>::CompleteKeywordNull(Token &token)
{
    if (NextCharacter() != 'u')
        return false;
    if (NextCharacter() != 'l')
        return false;
    if (NextCharacter() != 'l')
        return false;
    token.kind = Token::TKN_NullLiteral;
    return true;
}

// Returns false only on overflow
template <typename CharType>
inline bool JSON_Parser<CharType>::ParseInt64(CharType first, uint64_t& value)
{
    value = first - '0';
    CharType ch = PeekCharacter();
    while (ch >= '0' && ch <= '9')
    {
        unsigned int next_digit = (unsigned int)(ch - '0');
        if (value > (ULLONG_MAX / 10) || (value == ULLONG_MAX/10 && next_digit > ULLONG_MAX%10))
            return false;

        NextCharacter();

        value *= 10;
        value += next_digit;
        ch = PeekCharacter();
    }
    return true;
}

// This namespace hides the x-plat helper functions
namespace
{
static int print_llu(char* ptr, size_t n, uint64_t val64)
{
    return _snprintf_s_l(ptr, n, _TRUNCATE, "%I64u", utility::details::scoped_c_thread_locale::c_locale(), val64);
}

static int print_llu(wchar_t* ptr, size_t n, uint64_t val64)
{
    return _snwprintf_s_l(ptr, n, _TRUNCATE, L"%I64u", utility::details::scoped_c_thread_locale::c_locale(), val64);
}
static double anystod(const char* str)
{
    return _strtod_l(str, nullptr, utility::details::scoped_c_thread_locale::c_locale());
}
static double anystod(const wchar_t* str)
{
    return _wcstod_l(str, nullptr, utility::details::scoped_c_thread_locale::c_locale());
}
}

template <typename CharType>
bool JSON_Parser<CharType>::CompleteNumberLiteral(CharType first, Token &token)
{
    bool minus_sign;

    if (first == '-')
    {
        minus_sign = true;

        first = NextCharacter();
    }
    else
    {
        minus_sign = false;
    }

    if (first < '0' || first > '9')
        return false;

    CharType ch = PeekCharacter();

    //Check for two (or more) zeros at the beginning
    if (first == '0' && ch == '0')
        return false;

    // Parse the number assuming its integer
    uint64_t val64;
    bool complete = ParseInt64(first, val64);

    ch = PeekCharacter();
    if (complete && ch!='.' && ch!='E' && ch!='e')
    {
        if (minus_sign)
        {
            if (val64 > static_cast<uint64_t>(1) << 63 )
            {
                // It is negative and cannot be represented in int64, so we resort to double
                token.double_val = 0 - static_cast<double>(val64);
                token.signed_number = true;
                token.kind = JSON_Parser<CharType>::Token::TKN_NumberLiteral;
                return true;
            }

            // It is negative, but fits into int64
            token.int64_val = 0 - static_cast<int64_t>(val64);
            token.kind = JSON_Parser<CharType>::Token::TKN_IntegerLiteral;
            token.signed_number = true;
            return true;
        }

        // It is positive so we use unsigned int64
        token.uint64_val = val64;
        token.kind = JSON_Parser<CharType>::Token::TKN_IntegerLiteral;
        token.signed_number = false;
        return true;
    }

    // Magic number 5 leaves room for decimal point, null terminator, etc (in most cases)
    ::std::vector<CharType> buf(::std::numeric_limits<uint64_t>::digits10 + 5);
    int count = print_llu(buf.data(), buf.size(), val64);
    _ASSERTE(count >= 0);
    _ASSERTE((size_t)count < buf.size());
    // Resize to cut off the null terminator
    buf.resize(count);

    bool decimal = false;

    while (ch != this->m_eof)
    {
        // Digit encountered?
        if (ch >= '0' && ch <= '9')
        {
            buf.push_back(ch);
            NextCharacter();
            ch = PeekCharacter();
        }

        // Decimal dot?
        else if (ch == '.')
        {
            if (decimal)
                return false;

            decimal = true;
            buf.push_back(ch);

            NextCharacter();
            ch = PeekCharacter();

            // Check that the following char is a digit
            if (ch < '0' || ch > '9')
            return false;

            buf.push_back(ch);
            NextCharacter();
            ch = PeekCharacter();
        }

        // Exponent?
        else if (ch == 'E' || ch == 'e')
        {
            buf.push_back(ch);
            NextCharacter();
            ch = PeekCharacter();

            // Check for the exponent sign
            if (ch == '+')
            {
                buf.push_back(ch);
                NextCharacter();
                ch = PeekCharacter();
            }
            else if (ch == '-')
            {
                buf.push_back(ch);
                NextCharacter();
                ch = PeekCharacter();
            }

            // First number of the exponent
            if (ch >= '0' && ch <= '9')
            {
                buf.push_back(ch);
                NextCharacter();
                ch = PeekCharacter();
            }
            else return false;

            // The rest of the exponent
            while (ch >= '0' && ch <= '9')
            {
                buf.push_back(ch);
                NextCharacter();
                ch = PeekCharacter();
            }

            // The peeked character is not a number, so we can break from the loop and construct the number
            break;
        }
        else
        {
            // Not expected number character?
            break;
        }
    };

    buf.push_back('\0');
    token.double_val = anystod(buf.data());
    if (minus_sign)
    {
        token.double_val = -token.double_val;
    }
    token.kind = (JSON_Parser<CharType>::Token::TKN_NumberLiteral);

    return true;
}

template <typename CharType>
bool JSON_Parser<CharType>::CompleteComment(Token &token)
{
    // We already found a '/' character as the first of a token -- what kind of comment is it?

    CharType ch = NextCharacter();

    if ( ch == this->m_eof || (ch != '/' && ch != '*') )
        return false;

    if ( ch == '/' )
    {
        // Line comment -- look for a newline or EOF to terminate.

        ch = NextCharacter();

        while ( ch != this->m_eof && ch != '\n')
        {
            ch = NextCharacter();
        }
    }
    else
    {
        // Block comment -- look for a terminating "*/" sequence.

        ch = NextCharacter();

        while ( true )
        {
            if ( ch == this->m_eof )
                return false;

            if ( ch == '*' )
            {
                CharType ch1 = PeekCharacter();

                if ( ch1 == this->m_eof )
                    return false;

                if ( ch1 == '/' )
                {
                    // Consume the character
                    NextCharacter();
                    break;
                }

                ch = ch1;
            }

            ch = NextCharacter();
        }
    }

    token.kind = Token::TKN_Comment;

    return true;
}

template <typename CharType>
bool JSON_StringParser<CharType>::CompleteComment(typename JSON_Parser<CharType>::Token &token)
{
    // This function is specialized for the string parser, since we can be slightly more
    // efficient in copying data from the input to the token: do a memcpy() rather than
    // one character at a time.

    CharType ch = JSON_StringParser<CharType>::NextCharacter();

    if ( ch == this->m_eof || (ch != '/' && ch != '*') )
        return false;

    if ( ch == '/' )
    {
        // Line comment -- look for a newline or EOF to terminate.

        ch = JSON_StringParser<CharType>::NextCharacter();

        while ( ch != this->m_eof && ch != '\n')
        {
            ch = JSON_StringParser<CharType>::NextCharacter();
        }
    }
    else
    {
        // Block comment -- look for a terminating "*/" sequence.

        ch = JSON_StringParser<CharType>::NextCharacter();

        while ( true )
        {
            if ( ch == this->m_eof )
                return false;

            if ( ch == '*' )
            {
                ch = JSON_StringParser<CharType>::PeekCharacter();

                if ( ch == this->m_eof )
                    return false;

                if ( ch == '/' )
                {
                    // Consume the character
                    JSON_StringParser<CharType>::NextCharacter();
                    break;
                }

            }

            ch = JSON_StringParser<CharType>::NextCharacter();
        }
    }

    token.kind = JSON_Parser<CharType>::Token::TKN_Comment;

    return true;
}

void convert_append_unicode_code_unit(JSON_Parser<wchar_t>::Token &token, wchar_t value)
{
    token.string_val.push_back(value);
}
void convert_append_unicode_code_unit(JSON_Parser<char>::Token &token, wchar_t value)
{
    std::wstring utf16(reinterpret_cast<wchar_t *>(&value), 1);
    token.string_val.append(::utility::conversions::utf16_to_utf8(utf16));
}

template <typename CharType>
inline bool JSON_Parser<CharType>::handle_unescape_char(Token &token)
{
    // This function converts unescaped character pairs (e.g. "\t") into their ASCII or Unicode representations (e.g. tab sign)
    // Also it handles \u + 4 hexadecimal digits
    CharType ch = NextCharacter();
    switch (ch)
    {
        case '\"':
            token.string_val.push_back('\"');
            return true;
        case '\\':
            token.string_val.push_back('\\');
            return true;
        case '/':
            token.string_val.push_back('/');
            return true;
        case 'b':
            token.string_val.push_back('\b');
            return true;
        case 'f':
            token.string_val.push_back('\f');
            return true;
        case 'r':
            token.string_val.push_back('\r');
            return true;
        case 'n':
            token.string_val.push_back('\n');
            return true;
        case 't':
            token.string_val.push_back('\t');
            return true;
        case 'u':
        {
            // A four-hexdigit Unicode character.
            // Transform into a 16 bit code point.
            int decoded = 0;
            for (int i = 0; i < 4; ++i)
            {
                ch = NextCharacter();
                int ch_int = static_cast<int>(ch);
                if (ch_int < 0 || ch_int > 127)
                    return false;

                const int isxdigitResult = _isxdigit_l(ch_int, utility::details::scoped_c_thread_locale::c_locale());
                if (!isxdigitResult)
                    return false;

                int val = _hexval[static_cast<size_t>(ch_int)];
                _ASSERTE(val != -1);

                // Add the input char to the decoded number
                decoded |= (val << (4 * (3 - i)));
            }

            // Construct the character based on the decoded number
			convert_append_unicode_code_unit(token, static_cast<wchar_t>(decoded));

            return true;
        }
        default:
            return false;
    }
}

template <typename CharType>
bool JSON_Parser<CharType>::CompleteStringLiteral(Token &token)
{
    CharType ch = NextCharacter();
    while ( ch != '"' )
    {
        if ( ch == '\\' )
        {
            handle_unescape_char(token);
        }
        else if (ch >= CharType(0x0) && ch < CharType(0x20))
        {
            return false;
        }
        else
        {
            if (ch == this->m_eof)
                return false;

            token.string_val.push_back(ch);
        }
        ch = NextCharacter();
    }

    if ( ch == '"' )
    {
        token.kind = Token::TKN_StringLiteral;
    }
    else
    {
        return false;
    }

    return true;
}

template <typename CharType>
bool JSON_StringParser<CharType>::CompleteStringLiteral(typename JSON_Parser<CharType>::Token &token)
{
    // This function is specialized for the string parser, since we can be slightly more
    // efficient in copying data from the input to the token: do a memcpy() rather than
    // one character at a time.

    auto start = m_position;
    token.has_unescape_symbol = false;

    CharType ch = JSON_StringParser<CharType>::NextCharacter();

    while (ch != '"')
    {
        if (ch == this->m_eof)
            return false;

        if (ch == '\\')
        {
            token.string_val.resize(m_position - start - 1);
            if (token.string_val.size() > 0)
                memcpy(&token.string_val[0], start, (m_position - start - 1)*sizeof(CharType));

            token.has_unescape_symbol = true;

            return finish_parsing_string_with_unescape_char(token);
        }
        else if (ch >= CharType(0x0) && ch < CharType(0x20))
        {
            return false;
        }

        ch = JSON_StringParser<CharType>::NextCharacter();
    }

    token.string_val.resize(m_position - start - 1);
    if (token.string_val.size() > 0)
        memcpy(&token.string_val[0], start, (m_position - start - 1)*sizeof(CharType));

    token.kind = JSON_Parser<CharType>::Token::TKN_StringLiteral;

    return true;
}

template <typename CharType>
bool JSON_StringParser<CharType>::finish_parsing_string_with_unescape_char(typename JSON_Parser<CharType>::Token &token)
{
    // This function handles parsing the string when an unescape character is encountered.
    // It is called once the part before the unescape char is copied to the token.string_val string

    CharType ch;

    if (!JSON_StringParser<CharType>::handle_unescape_char(token))
        return false;

    while ((ch = JSON_StringParser<CharType>::NextCharacter()) != '"')
    {
        if (ch == '\\')
        {
            if (!JSON_StringParser<CharType>::handle_unescape_char(token))
                return false;
        }
        else
        {
            if (ch == this->m_eof)
                return false;

            token.string_val.push_back(ch);
        }
    }

    token.kind = JSON_StringParser<CharType>::Token::TKN_StringLiteral;

    return true;
}

template <typename CharType>
void JSON_Parser<CharType>::GetNextToken(typename JSON_Parser<CharType>::Token& result)
{
try_again:
    CharType ch = EatWhitespace();

    CreateToken(result, Token::TKN_EOF);

    if (ch == this->m_eof) return;

    switch (ch)
    {
    case '{':
    case '[':
        {
            if(++m_currentParsingDepth > JSON_Parser<CharType>::maxParsingDepth)
            {
                SetErrorCode(result, json_error::nesting);
                break;
            }

            typename JSON_Parser<CharType>::Token::Kind tk = ch == '{' ? Token::TKN_OpenBrace : Token::TKN_OpenBracket;
            CreateToken(result, tk, result.start);
            break;
        }
    case '}':
    case ']':
        {
            if((signed int)(--m_currentParsingDepth) < 0)
            {
                SetErrorCode(result, json_error::mismatched_brances);
                break;
            }

            typename JSON_Parser<CharType>::Token::Kind tk = ch == '}' ? Token::TKN_CloseBrace : Token::TKN_CloseBracket;
            CreateToken(result, tk, result.start);
            break;
        }
    case ',':
        CreateToken(result, Token::TKN_Comma, result.start);
        break;

    case ':':
        CreateToken(result, Token::TKN_Colon, result.start);
        break;

    case 't':
        if (!CompleteKeywordTrue(result))
        {
            SetErrorCode(result, json_error::malformed_literal);
        }
        break;
    case 'f':
        if (!CompleteKeywordFalse(result))
        {
            SetErrorCode(result, json_error::malformed_literal);
        }
        break;
    case 'n':
        if (!CompleteKeywordNull(result))
        {
            SetErrorCode(result, json_error::malformed_literal);
        }
        break;
    case '/':
        if (!CompleteComment(result))
        {
            SetErrorCode(result, json_error::malformed_comment);
            break;
        }
        // For now, we're ignoring comments.
        goto try_again;
    case '"':
        if (!CompleteStringLiteral(result))
        {
            SetErrorCode(result, json_error::malformed_string_literal);
        }
        break;

    case '-':
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
        if (!CompleteNumberLiteral(ch, result))
        {
            SetErrorCode(result, json_error::malformed_numeric_literal);
        }
        break;
    default:
        SetErrorCode(result, json_error::malformed_token);
        break;
    }
}

template <typename CharType>
std::unique_ptr<json::details::_Value> JSON_Parser<CharType>::_ParseObject(typename JSON_Parser<CharType>::Token &tkn)
{
    auto obj = utility::details::make_unique<json::details::_Object>(g_keep_json_object_unsorted);
    auto& elems = obj->m_object.m_elements;

    GetNextToken(tkn);
    if (tkn.m_error) goto error;

    if (tkn.kind != JSON_Parser<CharType>::Token::TKN_CloseBrace)
    {
        while (true)
        {
            // State 1: New field or end of object, looking for field name or closing brace
            std::basic_string<CharType> fieldName;
            switch (tkn.kind)
            {
            case JSON_Parser<CharType>::Token::TKN_StringLiteral:
                fieldName = std::move(tkn.string_val);
                break;
            default:
                goto error;
            }

            GetNextToken(tkn);
            if (tkn.m_error) goto error;

            // State 2: Looking for a colon.
            if (tkn.kind != JSON_Parser<CharType>::Token::TKN_Colon) goto done;

            GetNextToken(tkn);
            if (tkn.m_error) goto error;

            // State 3: Looking for an expression.
#ifdef ENABLE_JSON_VALUE_VISUALIZER
            auto fieldValue = _ParseValue(tkn);
            auto type = fieldValue->type();
            elems.emplace_back(NX::utility::conversions::to_string(std::move(fieldName)), json::value(std::move(fieldValue), type));
#else
            elems.emplace_back(NX::utility::conversions::to_string(std::move(fieldName)), json::value(_ParseValue(tkn)));
#endif
            if (tkn.m_error) goto error;

            // State 4: Looking for a comma or a closing brace
            switch (tkn.kind)
            {
            case JSON_Parser<CharType>::Token::TKN_Comma:
                GetNextToken(tkn);
                if (tkn.m_error) goto error;
                break;
            case JSON_Parser<CharType>::Token::TKN_CloseBrace:
                goto done;
            default:
                goto error;
            }
        }
    }

done:
    GetNextToken(tkn);
    if (tkn.m_error) return utility::details::make_unique<json::details::_Null>();

    if (!g_keep_json_object_unsorted) {
        ::std::sort(elems.begin(), elems.end(), json::object::compare_pairs);
    }

    return std::move(obj);

error:
    if (!tkn.m_error)
    {
        SetErrorCode(tkn, json_error::malformed_object_literal);
    }
    return utility::details::make_unique<json::details::_Null>();
}

template <typename CharType>
std::unique_ptr<json::details::_Value> JSON_Parser<CharType>::_ParseArray(typename JSON_Parser<CharType>::Token &tkn)
{
    GetNextToken(tkn);
    if (tkn.m_error) return utility::details::make_unique<json::details::_Null>();

    auto result = utility::details::make_unique<json::details::_Array>();

    if (tkn.kind != JSON_Parser<CharType>::Token::TKN_CloseBracket)
    {
        while (true)
        {
            // State 1: Looking for an expression.
            result->m_array.m_elements.emplace_back(ParseValue(tkn));
            if (tkn.m_error) return utility::details::make_unique<json::details::_Null>();

            // State 4: Looking for a comma or a closing bracket
            switch (tkn.kind)
            {
            case JSON_Parser<CharType>::Token::TKN_Comma:
                GetNextToken(tkn);
                if (tkn.m_error) return utility::details::make_unique<json::details::_Null>();
                break;
            case JSON_Parser<CharType>::Token::TKN_CloseBracket:
                GetNextToken(tkn);
                if (tkn.m_error) return utility::details::make_unique<json::details::_Null>();
                return std::move(result);
            default:
                SetErrorCode(tkn, json_error::malformed_array_literal);
                return utility::details::make_unique<json::details::_Null>();
            }
        }
    }

    GetNextToken(tkn);
    if (tkn.m_error) return utility::details::make_unique<json::details::_Null>();

    return std::move(result);
}

template <typename CharType>
std::unique_ptr<json::details::_Value> JSON_Parser<CharType>::_ParseValue(typename JSON_Parser<CharType>::Token &tkn)
{
    switch (tkn.kind)
    {
        case JSON_Parser<CharType>::Token::TKN_OpenBrace:
            {
                return _ParseObject(tkn);
            }
        case JSON_Parser<CharType>::Token::TKN_OpenBracket:
            {
                return _ParseArray(tkn);
            }
        case JSON_Parser<CharType>::Token::TKN_StringLiteral:
            {
                auto value = utility::details::make_unique<json::details::_String>(std::move(tkn.string_val), tkn.has_unescape_symbol);
                GetNextToken(tkn);
                if (tkn.m_error) return utility::details::make_unique<json::details::_Null>();
                return std::move(value);
            }
        case JSON_Parser<CharType>::Token::TKN_IntegerLiteral:
            {
                std::unique_ptr<json::details::_Number> value;
                if (tkn.signed_number)
                    value = utility::details::make_unique<json::details::_Number>(tkn.int64_val);
                else
                    value = utility::details::make_unique<json::details::_Number>(tkn.uint64_val);

                GetNextToken(tkn);
                if (tkn.m_error) return utility::details::make_unique<json::details::_Null>();
                return std::move(value);
            }
        case JSON_Parser<CharType>::Token::TKN_NumberLiteral:
            {
                auto value = utility::details::make_unique<json::details::_Number>(tkn.double_val);
                GetNextToken(tkn);
                if (tkn.m_error) return utility::details::make_unique<json::details::_Null>();
                return std::move(value);
            }
        case JSON_Parser<CharType>::Token::TKN_BooleanLiteral:
            {
                auto value = utility::details::make_unique<json::details::_Boolean>(tkn.boolean_val);
                GetNextToken(tkn);
                if (tkn.m_error) return utility::details::make_unique<json::details::_Null>();
                return std::move(value);
            }
        case JSON_Parser<CharType>::Token::TKN_NullLiteral:
            {
                GetNextToken(tkn);
                // Returning a null value whether or not an error occurred.
                return utility::details::make_unique<json::details::_Null>();
            }
        default:
            {
                SetErrorCode(tkn, json_error::malformed_token);
                return utility::details::make_unique<json::details::_Null>();
            }
    }
}

}}}}

static NX::web::json::value _parse_stream(std::wistream &stream)
{
    NX::web::json::details::JSON_StreamParser<wchar_t> parser(stream);
    NX::web::json::details::JSON_Parser<wchar_t>::Token tkn;

    parser.GetNextToken(tkn);
    if (tkn.m_error)
    {
        json::details::CreateException(tkn, utility::conversions::to_string(tkn.m_error.message()));
    }

    auto value = parser.ParseValue(tkn);
    if (tkn.m_error)
    {
        json::details::CreateException(tkn, utility::conversions::to_string(tkn.m_error.message()));
    }
    else if (tkn.kind != json::details::JSON_Parser<wchar_t>::Token::TKN_EOF)
    {
        json::details::CreateException(tkn, L"Left-over characters in stream after parsing a JSON value");
    }
    return value;
}

static NX::web::json::value _parse_stream(std::wistream &stream, std::error_code& error)
{
    NX::web::json::details::JSON_StreamParser<wchar_t> parser(stream);
    NX::web::json::details::JSON_Parser<wchar_t>::Token tkn;

    parser.GetNextToken(tkn);
    if (tkn.m_error)
    {
        error = std::move(tkn.m_error);
        return json::value();
    }

    auto returnObject = parser.ParseValue(tkn);
    if (tkn.kind != json::details::JSON_Parser<wchar_t>::Token::TKN_EOF)
    {
        json::details::SetErrorCode(tkn, json::details::json_error::left_over_character_in_stream);
    }

    error = std::move(tkn.m_error);
    return returnObject;
}

static NX::web::json::value _parse_narrow_stream(std::istream &stream)
{
    json::details::JSON_StreamParser<char> parser(stream);
    json::details::JSON_StreamParser<char>::Token tkn;

    parser.GetNextToken(tkn);
    if (tkn.m_error)
    {
        json::details::CreateException(tkn, utility::conversions::to_string(tkn.m_error.message()));
    }

    auto value = parser.ParseValue(tkn);
    if (tkn.m_error)
    {
        json::details::CreateException(tkn, utility::conversions::to_string(tkn.m_error.message()));
    }
    else if (tkn.kind != json::details::JSON_Parser<char>::Token::TKN_EOF)
    {
        json::details::CreateException(tkn, L"Left-over characters in stream after parsing a JSON value");
    }
    return value;
}

static NX::web::json::value _parse_narrow_stream(std::istream &stream, std::error_code& error)
{
    NX::web::json::details::JSON_StreamParser<char> parser(stream);
    NX::web::json::details::JSON_StreamParser<char>::Token tkn;

    parser.GetNextToken(tkn);
    if (tkn.m_error)
    {
        error = std::move(tkn.m_error);
        return json::value();
    }

    auto returnObject = parser.ParseValue(tkn);
    if (tkn.kind != json::details::JSON_Parser<wchar_t>::Token::TKN_EOF)
    {
        returnObject = json::value();
        json::details::SetErrorCode(tkn, json::details::json_error::left_over_character_in_stream);
    }

    error = std::move(tkn.m_error);
    return returnObject;
}

NX::web::json::value NX::web::json::value::parse(const std::wstring& str)
{
    NX::web::json::details::JSON_StringParser<wchar_t> parser(str);
    NX::web::json::details::JSON_Parser<wchar_t>::Token tkn;

    parser.GetNextToken(tkn);
    if (tkn.m_error)
    {
        NX::web::json::details::CreateException(tkn, NX::utility::conversions::to_string(tkn.m_error.message()));
    }

    auto value = parser.ParseValue(tkn);
    if (tkn.m_error)
    {
        NX::web::json::details::CreateException(tkn, NX::utility::conversions::to_string(tkn.m_error.message()));
    }
    else if (tkn.kind != NX::web::json::details::JSON_Parser<wchar_t>::Token::TKN_EOF)
    {
    }
    return value;
}

NX::web::json::value NX::web::json::value::parse(const std::wstring& str, std::error_code& error)
{
    NX::web::json::details::JSON_StringParser<wchar_t> parser(str);
    NX::web::json::details::JSON_Parser<wchar_t>::Token tkn;

    parser.GetNextToken(tkn);
    if (tkn.m_error)
    {
        error = std::move(tkn.m_error);
        return json::value();
    }

    auto returnObject = parser.ParseValue(tkn);
    if (tkn.kind != json::details::JSON_Parser<wchar_t>::Token::TKN_EOF)
    {
        returnObject = json::value();
        NX::web::json::details::SetErrorCode(tkn, json::details::json_error::left_over_character_in_stream);
    }

    error = std::move(tkn.m_error);
    return returnObject;
}

NX::web::json::value NX::web::json::value::parse(std::wistream &stream)
{
    return _parse_stream(stream);
}

NX::web::json::value NX::web::json::value::parse(std::wistream &stream, std::error_code& error)
{
    return _parse_stream(stream, error);
}

NX::web::json::value NX::web::json::value::parse(std::istream& stream)
{
    return _parse_narrow_stream(stream);
}

NX::web::json::value NX::web::json::value::parse(std::istream& stream, std::error_code& error)
{
    return _parse_narrow_stream(stream, error);
}
