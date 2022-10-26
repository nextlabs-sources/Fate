

#ifndef __NUDF_HTTP_HELPERS_HPP__
#define __NUDF_HTTP_HELPERS_HPP__

#include <Wincrypt.h>

#include <map>
#include <memory>
#include <string>
#include <vector>
#include <functional>

#include <nudf\web\uri.hpp>
#include <nudf\web\error_category.hpp>

namespace NX {
namespace web {
namespace http {

namespace client {
namespace details {
    class winhttp_client;
}
}

namespace details
{


 /// <summary>
 /// Determines whether or not the given content type is 'textual' according the feature specifications.
 /// </summary>
 bool is_content_type_textual(const std::wstring &content_type);

 /// <summary>
 /// Determines whether or not the given content type is JSON according the feature specifications.
 /// </summary>
 bool is_content_type_json(const std::wstring &content_type);

 /// <summary>
 /// Parses the given Content-Type header value to get out actual content type and charset.
 /// If the charset isn't specified the default charset for the content type will be set.
 /// </summary>
 void parse_content_type_and_charset(const std::wstring &content_type, std::wstring &content, std::wstring &charset);

 /// <summary>
 /// Gets the default charset for given content type. If the MIME type is not textual or recognized Latin1 will be returned.
 /// </summary>
 std::wstring get_default_charset(const std::wstring &content_type);

 /// <summary>
 /// Helper functions to convert a series of bytes from a charset to utf-8 or utf-16.
 /// These APIs deal with checking for and handling byte order marker (BOM).
 /// </summary>
 std::wstring convert_utf16_to_string_t(std::wstring src);
 std::wstring convert_utf16_to_utf16(std::wstring src);
 std::string convert_utf16_to_utf8(std::wstring src);
 std::wstring convert_utf16le_to_string_t(std::wstring src, bool erase_bom);
 std::string convert_utf16le_to_utf8(std::wstring src, bool erase_bom);
 std::wstring convert_utf16be_to_string_t(std::wstring src, bool erase_bom);
 std::string convert_utf16be_to_utf8(std::wstring src, bool erase_bom);
 std::wstring convert_utf16be_to_utf16le(std::wstring src, bool erase_bom);

 // simple helper functions to trim whitespace.
 void ltrim_whitespace(std::wstring &str);
 void rtrim_whitespace(std::wstring &str);
 void trim_whitespace(std::wstring &str);

 bool validate_method(const std::wstring& method);


 namespace chunked_encoding
 {
     // Transfer-Encoding: chunked support
     static const size_t additional_encoding_space = 12;
     static const size_t data_offset               = additional_encoding_space-2;

     // Add the data necessary for properly sending data with transfer-encoding: chunked.
     //
     // There are up to 12 additional bytes needed for each chunk:
     //
     // The last chunk requires 5 bytes, and is fixed.
     // All other chunks require up to 8 bytes for the length, and four for the two CRLF
     // delimiters.
     //
     size_t add_chunked_delimiters(_Out_writes_ (buffer_size) uint8_t *data, _In_ size_t buffer_size, size_t bytes_read);
 }

} // namespace details

}   // namespace http


namespace details {
    
class zero_memory_deleter
{
public:
    void operator()(std::wstring *data) const
    {
        SecureZeroMemory(
            const_cast<std::wstring::value_type *>(data->data()),
            data->size() * sizeof(std::wstring::value_type));
        delete data;
    }
};
typedef std::unique_ptr<std::wstring, zero_memory_deleter> plaintext_string;

class win32_encryption
{
public:
    win32_encryption() {}
    win32_encryption(const std::wstring &data)
    {
        const auto dataNumBytes = data.size() * sizeof(std::wstring::value_type);
        m_buffer.resize(dataNumBytes);
        memcpy_s(m_buffer.data(), m_buffer.size(), data.c_str(), dataNumBytes);

        // Buffer must be a multiple of CRYPTPROTECTMEMORY_BLOCK_SIZE
        const auto mod = m_buffer.size() % CRYPTPROTECTMEMORY_BLOCK_SIZE;
        if (mod != 0) {
            m_buffer.resize(m_buffer.size() + CRYPTPROTECTMEMORY_BLOCK_SIZE - mod);
        }
        if (!CryptProtectMemory(m_buffer.data(), static_cast<DWORD>(m_buffer.size()), CRYPTPROTECTMEMORY_SAME_PROCESS)) {
            throw NX::utility::create_system_error(GetLastError());
        }
    }
    ~win32_encryption()
    {
        SecureZeroMemory(m_buffer.data(), m_buffer.size());
    }
    plaintext_string decrypt() const
    {
        // Copy the buffer and decrypt to avoid having to re-encrypt.
        auto data = plaintext_string(new std::wstring(reinterpret_cast<const std::wstring::value_type *>(m_buffer.data()), m_buffer.size() / 2));
        if (!CryptUnprotectMemory(
            const_cast<std::wstring::value_type *>(data->c_str()),
            static_cast<DWORD>(m_buffer.size()),
            CRYPTPROTECTMEMORY_SAME_PROCESS))
        {
            throw NX::utility::create_system_error(GetLastError());
        }
        data->resize(m_numCharacters);
        return std::move(data);
    }
private:
    std::vector<char> m_buffer;
    size_t m_numCharacters;
};
}   // namespace details



/// <summary>
/// Represents a set of user credentials (user name and password) to be used
/// for authentication.
/// </summary>
class credentials
{
public:
    /// <summary>
    /// Constructs an empty set of credentials without a user name or password.
    /// </summary>
    credentials() {}

    /// <summary>
    /// Constructs credentials from given user name and password.
    /// </summary>
    /// <param name="username">User name as a string.</param>
    /// <param name="password">Password as a string.</param>
    credentials(std::wstring username, const std::wstring &password) :
        m_username(std::move(username))
        , m_password(password)
    {}

    /// <summary>
    /// The user name associated with the credentials.
    /// </summary>
    /// <returns>A string containing the user name.</returns>
    const std::wstring &username() const { return m_username; }

    /// <summary>
    /// The password for the user name associated with the credentials.
    /// </summary>
    /// <returns>A string containing the password.</returns>
    std::wstring password() const
    {
        return std::wstring(*m_password.decrypt());
    }

    /// <summary>
    /// Checks if credentials have been set
    /// </summary>
    /// <returns><c>true</c> if user name and password is set, <c>false</c> otherwise.</returns>
    bool is_set() const { return !m_username.empty(); }

private:
    friend class NX::web::http::client::details::winhttp_client;

    details::plaintext_string decrypt() const
    {
        return m_password.decrypt();
    }

    std::wstring m_username;
    details::win32_encryption m_password;
};

/// <summary>
/// web_proxy represents the concept of the web proxy, which can be auto-discovered,
/// disabled, or specified explicitly by the user.
/// </summary>
class web_proxy
{
    enum web_proxy_mode_internal{ use_default_, use_auto_discovery_, disabled_, user_provided_ };
public:
    enum web_proxy_mode{ use_default = use_default_, use_auto_discovery = use_auto_discovery_, disabled  = disabled_};

    /// <summary>
    /// Constructs a proxy with the default settings.
    /// </summary>
    web_proxy() : m_address(L""), m_mode(use_default_) {}

    /// <summary>
    /// Creates a proxy with specified mode.
    /// </summary>
    /// <param name="mode">Mode to use.</param>
    web_proxy( web_proxy_mode mode ) : m_address(L""), m_mode(static_cast<web_proxy_mode_internal>(mode)) {}

    /// <summary>
    /// Creates a proxy explicitly with provided address.
    /// </summary>
    /// <param name="address">Proxy URI to use.</param>
    web_proxy( uri address ) : m_address(address), m_mode(user_provided_) {}

    /// <summary>
    /// Gets this proxy's URI address. Returns an empty URI if not explicitly set by user.
    /// </summary>
    /// <returns>A reference to this proxy's URI.</returns>
    const uri& address() const { return m_address; }

    /// <summary>
    /// Gets the credentials used for authentication with this proxy.
    /// </summary>
    /// <returns>Credentials to for this proxy.</returns>
    const web::credentials& credentials() const { return m_credentials; }

    /// <summary>
    /// Sets the credentials to use for authentication with this proxy.
    /// </summary>
    /// <param name="cred">Credentials to use for this proxy.</param>
    void set_credentials(web::credentials cred) {
        if( m_mode == disabled_ )
        {
            throw std::invalid_argument("Cannot attach credentials to a disabled proxy");
        }
        m_credentials = std::move(cred);
    }

    /// <summary>
    /// Checks if this proxy was constructed with default settings.
    /// </summary>
    /// <returns>True if default, false otherwise.</param>
    bool is_default() const { return m_mode == use_default_; }

    /// <summary>
    /// Checks if using a proxy is disabled.
    /// </summary>
    /// <returns>True if disabled, false otherwise.</returns>
    bool is_disabled() const { return m_mode == disabled_; }

    /// <summary>
    /// Checks if the auto discovery protocol, WPAD, is to be used.
    /// </summary>
    /// <returns>True if auto discovery enabled, false otherwise.</returns>
    bool is_auto_discovery() const { return m_mode == use_auto_discovery_; }

    /// <summary>
    /// Checks if a proxy address is explicitly specified by the user.
    /// </summary>
    /// <returns>True if a proxy address was explicitly specified, false otherwise.</returns>
    bool is_specified() const { return m_mode == user_provided_; }

private:
    web::uri m_address;
    web_proxy_mode_internal m_mode;
    web::credentials m_credentials;
};


}   // namespace web
}   // namespace NX


#endif