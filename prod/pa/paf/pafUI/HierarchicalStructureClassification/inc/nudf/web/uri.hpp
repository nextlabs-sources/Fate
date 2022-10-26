

#ifndef __NUDF_URI_HPP__
#define __NUDF_URI_HPP__

#include <map>
#include <memory>
#include <string>
#include <sstream>
#include <vector>
#include <functional>

namespace NX {
namespace web {


namespace details
{

struct uri_components
{
    uri_components() : m_path(L"/"), m_port(-1)
    {}

    uri_components(const uri_components &other) :
        m_scheme(other.m_scheme),
        m_host(other.m_host),
        m_user_info(other.m_user_info),
        m_path(other.m_path),
        m_query(other.m_query),
        m_fragment(other.m_fragment),
        m_port(other.m_port)
    {}

    uri_components & operator=(const uri_components &other)
    {
        if (this != &other) {
            m_scheme = other.m_scheme;
            m_host = other.m_host;
            m_user_info = other.m_user_info;
            m_path = other.m_path;
            m_query = other.m_query;
            m_fragment = other.m_fragment;
            m_port = other.m_port;
        }
        return *this;
    }

    uri_components(uri_components &&other) throw() :
        m_scheme(std::move(other.m_scheme)),
        m_host(std::move(other.m_host)),
        m_user_info(std::move(other.m_user_info)),
        m_path(std::move(other.m_path)),
        m_query(std::move(other.m_query)),
        m_fragment(std::move(other.m_fragment)),
        m_port(other.m_port)
    {}

    uri_components & operator=(uri_components &&other) throw()
    {
        if (this != &other) {
            m_scheme = std::move(other.m_scheme);
            m_host = std::move(other.m_host);
            m_user_info = std::move(other.m_user_info);
            m_path = std::move(other.m_path);
            m_query = std::move(other.m_query);
            m_fragment = std::move(other.m_fragment);
            m_port = other.m_port;
        }
        return *this;
    }

    std::wstring join();

    std::wstring m_scheme;
    std::wstring m_host;
    std::wstring m_user_info;
    std::wstring m_path;
    std::wstring m_query;
    std::wstring m_fragment;
    int          m_port;
};


namespace uri_parser
{

/// <summary>
/// Parses the uri, attempting to determine its validity.
///
/// This function accepts both uris ('http://msn.com') and uri relative-references ('path1/path2?query')
/// </summary>
bool validate(const std::wstring &encoded_string);

/// <summary>
/// Parses the uri, setting each provided string to the value of that component. Components
/// that are not part of the provided text are set to the empty string. Component strings
/// DO NOT contain their beginning or ending delimiters.
///
/// This function accepts both uris ('http://msn.com') and uri relative-references ('path1/path2?query')
/// </summary>
bool parse(const std::wstring &encoded_string, uri_components &components);

/// <summary>
/// Unreserved characters are those that are allowed in a URI but do not have a reserved purpose. They include:
/// - A-Z
/// - a-z
/// - 0-9
/// - '-' (hyphen)
/// - '.' (period)
/// - '_' (underscore)
/// - '~' (tilde)
/// </summary>
inline bool is_unreserved(int c)
{
    return ((c>='A' && c<='z') || (c>='a' && c<='z') || (c>='0' && c<='9') || c == '-' || c == '.' || c == '_' || c == '~');
}

/// <summary>
/// General delimiters serve as the delimiters between different uri components.
/// General delimiters include:
/// - All of these :/?#[]@
/// </summary>
inline bool is_gen_delim(int c)
{
    return c == ':' || c == '/' || c == '?' || c == '#' || c == '[' || c == ']' || c == '@';
}

/// <summary>
/// Subdelimiters are those characters that may have a defined meaning within component
/// of a uri for a particular scheme. They do not serve as delimiters in any case between
/// uri segments. sub_delimiters include:
/// - All of these !$&'()*+,;=
/// </summary>
inline bool is_sub_delim(int c)
{
    switch (c)
    {
    case '!':
    case '$':
    case '&':
    case '\'':
    case '(':
    case ')':
    case '*':
    case '+':
    case ',':
    case ';':
    case '=':
        return true;
    default:
        return false;
    }
}

/// <summary>
/// Reserved characters includes the general delimiters and sub delimiters. Some characters
/// are neither reserved nor unreserved, and must be percent-encoded.
/// </summary>
inline bool is_reserved(int c)
{
    return is_gen_delim(c) || is_sub_delim(c);
}

/// <summary>
/// Legal characters in the scheme portion include:
/// - Any alphanumeric character
/// - '+' (plus)
/// - '-' (hyphen)
/// - '.' (period)
///
/// Note that the scheme must BEGIN with an alpha character.
/// </summary>
inline bool is_scheme_character(int c)
{
    return (c>='A' && c<='z') || (c>='a' && c<='z') || (c>='0' && c<='9') || c == '+' || c == '-' || c == '.';
}

/// <summary>
/// Legal characters in the user information portion include:
/// - Any unreserved character
/// - The percent character ('%'), and thus any percent-endcoded octet
/// - The sub-delimiters
/// - ':' (colon)
/// </summary>
inline bool is_user_info_character(int c)
{
    return is_unreserved(c) || is_sub_delim(c) || c == '%' || c == ':';
}

/// <summary>
/// Legal characters in the host portion include:
/// - Any unreserved character
/// - The percent character ('%'), and thus any percent-endcoded octet
/// - The sub-delimiters
/// - ':' (colon)
/// - '[' (open bracket)
/// - ']' (close bracket)
/// </summary>
inline bool is_host_character(int c)
{
    return is_unreserved(c) || is_sub_delim(c) || c == '%' || c == ':' || c == '[' || c == ']';
}

/// <summary>
/// Legal characters in the authority portion include:
/// - Any unreserved character
/// - The percent character ('%'), and thus any percent-endcoded octet
/// - The sub-delimiters
/// - ':' (colon)
///
/// Note that we don't currently support:
/// - IPv6 addresses (requires '[]')
/// </summary>
inline bool is_authority_character(int c)
{
    return is_unreserved(c) || is_sub_delim(c) || c == '%' || c == '@' || c == ':';
}

/// <summary>
/// Legal characters in the path portion include:
/// - Any unreserved character
/// - The percent character ('%'), and thus any percent-endcoded octet
/// - The sub-delimiters
/// - ':' (colon)
/// - '@' (ampersand)
/// </summary>
inline bool is_path_character(int c)
{
    return is_unreserved(c) || is_sub_delim(c) || c == '%' || c == '/' || c == ':' || c == '@';
}

/// <summary>
/// Legal characters in the query portion include:
/// - Any path character
/// - '?' (question mark)
/// </summary>
inline bool is_query_character(int c)
{
    return is_path_character(c) || c == '?';
}

/// <summary>
/// Legal characters in the fragment portion include:
/// - Any path character
/// - '?' (question mark)
/// </summary>
inline bool is_fragment_character(int c)
{
    // this is intentional, they have the same set of legal characters
    return is_query_character(c);
}

/// <summary>
/// Parses the uri, setting the given pointers to locations inside the given buffer.
/// 'encoded' is expected to point to an encoded zero-terminated string containing a uri
/// </summary>
bool inner_parse( const wchar_t *encoded,
                  const wchar_t **scheme_begin, const wchar_t **scheme_end,
                  const wchar_t **uinfo_begin, const wchar_t **uinfo_end,
                  const wchar_t **host_begin, const wchar_t **host_end,
                  _Out_ int *port,
                  const wchar_t **path_begin, const wchar_t **path_end,
                  const wchar_t **query_begin, const wchar_t **query_end,
                  const wchar_t **fragment_begin, const wchar_t **fragment_end);

}   // namespace uri_parser

}   // namespace details


/// <summary>
/// A single exception type to represent errors in parsing, encoding, and decoding URIs.
/// </summary>
class uri_exception : public std::exception
{
public:
    uri_exception(std::string msg) : m_msg(std::move(msg)) {}

    ~uri_exception() throw() {}

    const char* what() const throw()
    {
        return m_msg.c_str();
    }

private:
    std::string m_msg;
};



/// <summary>
/// A flexible, protocol independent URI implementation.
///
/// URI instances are immutable. Querying the various fields on an emtpy URI will return empty strings. Querying
/// various diagnostic members on an empty URI will return false.
/// </summary>
/// <remarks>
/// This implementation accepts both URIs ('http://msn.com/path') and URI relative-references
/// ('/path?query#frag').
///
/// This implementation does not provide any scheme-specific handling -- an example of this
/// would be the following: 'http://path1/path'. This is a valid URI, but it's not a valid
/// http-uri -- that is, it's syntactically correct but does not conform to the requirements
/// of the http scheme (http requires a host).
/// We could provide this by allowing a pluggable 'scheme' policy-class, which would provide
/// extra capability for validating and canonicalizing a URI according to scheme, and would
/// introduce a layer of type-safety for URIs of differing schemes, and thus differing semantics.
///
/// One issue with implementing a scheme-independent URI facility is that of comparing for equality.
/// For instance, these URIs are considered equal 'http://msn.com', 'http://msn.com:80'. That is --
/// the 'default' port can be either omitted or explicit. Since we don't have a way to map a scheme
/// to it's default port, we don't have a way to know these are equal. This is just one of a class of
/// issues with regard to scheme-specific behavior.
/// </remarks>
class uri
{
public:

    /// <summary>
    /// The various components of a URI. This enum is used to indicate which
    /// URI component is being encoded to the encode_uri_component. This allows
    /// specific encoding to be performed.
    ///
    /// Scheme and port don't allow '%' so they don't need to be encoded.
    /// </summary>
    class components
    {
    public:
        enum component
        {
            user_info,
            host,
            path,
            query,
            fragment,
            full_uri
        };
    };

    /// <summary>
    /// Encodes a URI component according to RFC 3986.
    /// Note if a full URI is specified instead of an individual URI component all
    /// characters not in the unreserved set are escaped.
    /// </summary>
    /// <param name="raw">The URI as a string.</param>
    /// <returns>The encoded string.</returns>
    static std::wstring __cdecl encode_uri(const std::wstring &raw, uri::components::component = components::full_uri);

    /// <summary>
    /// Encodes a string by converting all characters except for RFC 3986 unreserved characters to their
    /// hexadecimal representation.
    /// </summary>
    /// <param name="utf8data">The UTF-8 string data.</param>
    /// <returns>The encoded string.</returns>
    static std::wstring __cdecl encode_data_string(const std::wstring &utf8data);

    /// <summary>
    /// Decodes an encoded string.
    /// </summary>
    /// <param name="encoded">The URI as a string.</param>
    /// <returns>The decoded string.</returns>
    static std::wstring __cdecl decode(const std::wstring &encoded);

    /// <summary>
    /// Splits a path into its hierarchical components.
    /// </summary>
    /// <param name="path">The path as a string</param>
    /// <returns>A <c>std::vector&lt;std::wstring&gt;</c> containing the segments in the path.</returns>
    static std::vector<std::wstring> __cdecl split_path(const std::wstring &path);

    /// <summary>
    /// Splits a query into its key-value components.
    /// </summary>
    /// <param name="query">The query string</param>
    /// <returns>A <c>std::map&lt;std::wstring, std::wstring&gt;</c> containing the key-value components of the query.</returns>
    static std::map<std::wstring, std::wstring> __cdecl split_query(const std::wstring &query);

    /// <summary>
    /// Validates a string as a URI.
    /// </summary>
    /// <param name="uri_string">The URI string to be validated.</param>
    /// <returns><c>true</c> if the given string represents a valid URI, <c>false</c> otherwise.</returns>
    static bool __cdecl validate(const std::wstring &uri_string);

    /// <summary>
    /// Creates an empty uri
    /// </summary>
    uri() { m_uri = L"/";};

    /// <summary>
    /// Creates a URI from the given encoded string. This will throw an exception if the string
    /// does not contain a valid URI. Use uri::validate if processing user-input.
    /// </summary>
    /// <param name="uri_string">A pointer to an encoded string to create the URI instance.</param>
    uri(const wchar_t *uri_string);

    /// <summary>
    /// Creates a URI from the given encoded string. This will throw an exception if the string
    /// does not contain a valid URI. Use uri::validate if processing user-input.
    /// </summary>
    /// <param name="uri_string">An encoded URI string to create the URI instance.</param>
    uri(const std::wstring &uri_string);

    /// <summary>
    /// Copy constructor.
    /// </summary>
    uri(const uri &other) :
        m_uri(other.m_uri),
        m_components(other.m_components)
    {}

    /// <summary>
    /// Copy assignment operator.
    /// </summary>
    uri & operator=(const uri &other)
    {
        if (this != &other) {
            m_uri = other.m_uri;
            m_components = other.m_components;
        }
        return *this;
    }

    /// <summary>
    /// Move constructor.
    /// </summary>
    uri(uri &&other) throw() :
        m_uri(std::move(other.m_uri)),
        m_components(std::move(other.m_components))
    {}

    /// <summary>
    /// Move assignment operator
    /// </summary>
    uri & operator=(uri &&other) throw()
    {
        if (this != &other) {
            m_uri = std::move(other.m_uri);
            m_components = std::move(other.m_components);
        }
        return *this;
    }

    /// <summary>
    /// Get the scheme component of the URI as an encoded string.
    /// </summary>
    /// <returns>The URI scheme as a string.</returns>
    const std::wstring &scheme() const { return m_components.m_scheme; }

    /// <summary>
    /// Get the user information component of the URI as an encoded string.
    /// </summary>
    /// <returns>The URI user information as a string.</returns>
    const std::wstring &user_info() const { return m_components.m_user_info; }

    /// <summary>
    /// Get the host component of the URI as an encoded string.
    /// </summary>
    /// <returns>The URI host as a string.</returns>
    const std::wstring &host() const { return m_components.m_host; }

    /// <summary>
    /// Get the port component of the URI. Returns -1 if no port is specified.
    /// </summary>
    /// <returns>The URI port as an integer.</returns>
    int port() const { return m_components.m_port; }

    /// <summary>
    /// Get the path component of the URI as an encoded string.
    /// </summary>
    /// <returns>The URI path as a string.</returns>
    const std::wstring &path() const { return m_components.m_path; }

    /// <summary>
    /// Get the query component of the URI as an encoded string.
    /// </summary>
    /// <returns>The URI query as a string.</returns>
    const std::wstring &query() const { return m_components.m_query; }

    /// <summary>
    /// Get the fragment component of the URI as an encoded string.
    /// </summary>
    /// <returns>The URI fragment as a string.</returns>
    const std::wstring &fragment() const { return m_components.m_fragment; }

    /// <summary>
    /// Creates a new uri object with the same authority portion as this one, omitting the resource and query portions.
    /// </summary>
    /// <returns>The new uri object with the same authority.</returns>
    uri authority() const;

    /// <summary>
    /// Gets the path, query, and fragment portion of this uri, which may be empty.
    /// </summary>
    /// <returns>The new URI object with the path, query and fragment portion of this URI.</returns>
    uri resource() const;

    /// <summary>
    /// An empty URI specifies no components, and serves as a default value
    /// </summary>
    bool is_empty() const
    {
        return this->m_uri.empty() || this->m_uri == L"/";
    }

    /// <summary>
    /// A loopback URI is one which refers to a hostname or ip address with meaning only on the local machine.
    /// </summary>
    /// <remarks>
    /// Examples include "locahost", or ip addresses in the loopback range (127.0.0.0/24).
    /// </remarks>
    /// <returns><c>true</c> if this URI references the local host, <c>false</c> otherwise.</returns>
    bool is_host_loopback() const
    {
        return !is_empty() && ((host() == L"localhost") || (host().size() > 4 && host().substr(0,4) == L"127."));
    }

    /// <summary>
    /// A wildcard URI is one which refers to all hostnames that resolve to the local machine (using the * or +)
    /// </summary>
    /// <example>
    /// http://*:80
    /// </example>
    bool is_host_wildcard() const
    {
        return !is_empty() && (this->host() == L"*" || this->host() == L"+");
    }

    /// <summary>
    /// A portable URI is one with a hostname that can be resolved globally (used from another machine).
    /// </summary>
    /// <returns><c>true</c> if this URI can be resolved globally (used from another machine), <c>false</c> otherwise.</returns>
    /// <remarks>
    /// The hostname "localhost" is a reserved name that is guaranteed to resolve to the local machine,
    /// and cannot be used for inter-machine communication. Likewise the hostnames "*" and "+" on Windows
    /// represent wildcards, and do not map to a resolvable address.
    /// </remarks>
    bool is_host_portable() const
    {
        return !(is_empty() || is_host_loopback() || is_host_wildcard());
    }

    // <summary>
    /// A default port is one where the port is unspecified, and will be determined by the operating system.
    /// The choice of default port may be dictated by the scheme (http -> 80) or not.
    /// </summary>
    /// <returns><c>true</c> if this URI instance has a default port, <c>false</c> otherwise.</returns>
    bool is_port_default() const
    {
        return !is_empty() && this->port() == 0;
    }

    /// <summary>
    /// An "authority" URI is one with only a scheme, optional userinfo, hostname, and (optional) port.
    /// </summary>
    /// <returns><c>true</c> if this is an "authority" URI, <c>false</c> otherwise.</returns>
    bool is_authority() const
    {
        return !is_empty() && is_path_empty() && query().empty() && fragment().empty();
    }

    /// <summary>
    /// Returns whether the other URI has the same authority as this one
    /// </summary>
    /// <param name="other">The URI to compare the authority with.</param>
    /// <returns><c>true</c> if both the URI's have the same authority, <c>false</c> otherwise.</returns>
    bool has_same_authority(const uri &other) const
    {
        return !is_empty() && this->authority() == other.authority();
    }

    /// <summary>
    /// Returns whether the path portion of this URI is empty
    /// </summary>
    /// <returns><c>true</c> if the path portion of this URI is empty, <c>false</c> otherwise.</returns>
    bool is_path_empty() const
    {
        return path().empty() || path() == L"/";
    }

    /// <summary>
    /// Returns the full (encoded) URI as a string.
    /// </summary>
     /// <returns>The full encoded URI string.</returns>
    std::wstring to_string() const
    {
        return m_uri;
    }

    bool operator == (const uri &other) const;

    bool operator < (const uri &other) const
    {
        return m_uri < other.m_uri;
    }

    bool operator != (const uri &other) const
    {
        return !(this->operator == (other));
    }

private:
    friend class uri_builder;

    // Encodes all characters not in given set determined by given function.
    static std::wstring encode_impl(const std::wstring &raw, const std::function<bool(int)>& should_encode);

    std::wstring m_uri;
    details::uri_components m_components;
};



/// <summary>
/// Builder for constructing URIs incrementally.
/// </summary>
class uri_builder
{
public:

    /// <summary>
    /// Creates a builder with an initially empty URI.
    /// </summary>
    uri_builder() {}

    /// <summary>
    /// Creates a builder with a existing URI object.
    /// </summary>
    /// <param name="uri_str">Encoded string containing the URI.</param>
    uri_builder(const NX::web::uri &uri_str): m_uri(uri_str.m_components) {}

    /// <summary>
    /// Get the scheme component of the URI as an encoded string.
    /// </summary>
    /// <returns>The URI scheme as a string.</returns>
    const std::wstring &scheme() const { return m_uri.m_scheme; }

    /// <summary>
    /// Get the user information component of the URI as an encoded string.
    /// </summary>
    /// <returns>The URI user information as a string.</returns>
    const std::wstring &user_info() const { return m_uri.m_user_info; }

    /// <summary>
    /// Get the host component of the URI as an encoded string.
    /// </summary>
    /// <returns>The URI host as a string.</returns>
    const std::wstring &host() const { return m_uri.m_host; }

    /// <summary>
    /// Get the port component of the URI. Returns -1 if no port is specified.
    /// </summary>
    /// <returns>The URI port as an integer.</returns>
    int port() const { return m_uri.m_port; }

    /// <summary>
    /// Get the path component of the URI as an encoded string.
    /// </summary>
    /// <returns>The URI path as a string.</returns>
    const std::wstring &path() const { return m_uri.m_path; }

    /// <summary>
    /// Get the query component of the URI as an encoded string.
    /// </summary>
    /// <returns>The URI query as a string.</returns>
    const std::wstring &query() const { return m_uri.m_query; }

    /// <summary>
    /// Get the fragment component of the URI as an encoded string.
    /// </summary>
    /// <returns>The URI fragment as a string.</returns>
    const std::wstring &fragment() const { return m_uri.m_fragment; }

    /// <summary>
    /// Set the scheme of the URI.
    /// </summary>
    /// <param name="scheme">Uri scheme.</param>
    /// <returns>A reference to this <c>uri_builder</c> to support chaining.</returns>
    uri_builder & set_scheme(const std::wstring &scheme)
    {
        m_uri.m_scheme = scheme;
        return *this;
    }

    /// <summary>
    /// Set the user info component of the URI.
    /// </summary>
    /// <param name="user_info">User info as a decoded string.</param>
    /// <param name="do_encoding">Specify whether to apply URI encoding to the given string.</param>
    /// <returns>A reference to this <c>uri_builder</c> to support chaining.</returns>
    uri_builder & set_user_info(const std::wstring &user_info, bool do_encoding = false)
    {
        m_uri.m_user_info = do_encoding ? uri::encode_uri(user_info, uri::components::user_info) : user_info;
        return *this;
    }

    /// <summary>
    /// Set the host component of the URI.
    /// </summary>
    /// <param name="host">Host as a decoded string.</param>
    /// <param name="do_encoding">Specify whether to apply URI encoding to the given string.</param>
    /// <returns>A reference to this <c>uri_builder</c> to support chaining.</returns>
    uri_builder & set_host(const std::wstring &host, bool do_encoding = false)
    {
        m_uri.m_host = do_encoding ? uri::encode_uri(host, uri::components::host) : host;
        return *this;
    }

    /// <summary>
    /// Set the port component of the URI.
    /// </summary>
    /// <param name="port">Port as an integer.</param>
    /// <returns>A reference to this <c>uri_builder</c> to support chaining.</returns>
    uri_builder & set_port(int port)
    {
        m_uri.m_port = port;
        return *this;
    }

    /// <summary>
    /// Set the port component of the URI.
    /// </summary>
    /// <param name="port">Port as a string.</param>
    /// <returns>A reference to this <c>uri_builder</c> to support chaining.</returns>
    /// <remarks>When string can't be converted to an integer the port is left unchanged.</remarks>
    uri_builder & set_port(const std::wstring &port)
    {
        std::wistringstream portStream(port);
        int port_tmp;
        portStream >> port_tmp;
        if(portStream.fail() || portStream.bad()) {
            throw std::invalid_argument("invalid port argument, must be non empty string containing integer value");
        }
        m_uri.m_port = port_tmp;
        return *this;
    }

    /// <summary>
    /// Set the path component of the URI.
    /// </summary>
    /// <param name="path">Path as a decoded string.</param>
    /// <param name="do_encoding">Specify whether to apply URI encoding to the given string.</param>
    /// <returns>A reference to this <c>uri_builder</c> to support chaining.</returns>
    uri_builder & set_path(const std::wstring &path, bool do_encoding = false)
    {
        m_uri.m_path = do_encoding ? uri::encode_uri(path, uri::components::path) : path;
        return *this;
    }


    /// <summary>
    /// Set the query component of the URI.
    /// </summary>
    /// <param name="query">Query as a decoded string.</param>
    /// <param name="do_encoding">Specify whether apply URI encoding to the given string.</param>
    /// <returns>A reference to this <c>uri_builder</c> to support chaining.</returns>
    uri_builder & set_query(const std::wstring &query, bool do_encoding = false)
    {
        m_uri.m_query = do_encoding ? uri::encode_uri(query, uri::components::query) : query;
        return *this;
    }

    /// <summary>
    /// Set the fragment component of the URI.
    /// </summary>
    /// <param name="fragment">Fragment as a decoded string.</param>
    /// <param name="do_encoding">Specify whether to apply URI encoding to the given string.</param>
    /// <returns>A reference to this <c>uri_builder</c> to support chaining.</returns>
    uri_builder & set_fragment(const std::wstring &fragment, bool do_encoding = false)
    {
        m_uri.m_fragment = do_encoding ? uri::encode_uri(fragment, uri::components::fragment) : fragment;
        return *this;
    }

    /// <summary>
    /// Clears all components of the underlying URI in this uri_builder.
    /// </summary>
    void clear()
    {
        m_uri = details::uri_components();
    }

    /// <summary>
    /// Appends another path to the path of this uri_builder.
    /// </summary>
    /// <param name="path">Path to append as a already encoded string.</param>
    /// <param name="do_encoding">Specify whether to apply URI encoding to the given string.</param>
    /// <returns>A reference to this uri_builder to support chaining.</returns>
    uri_builder &append_path(const std::wstring &path, bool do_encoding = false);

    /// <summary>
    /// Appends another query to the query of this uri_builder.
    /// </summary>
    /// <param name="query">Query to append as a decoded string.</param>
    /// <param name="do_encoding">Specify whether to apply URI encoding to the given string.</param>
    /// <returns>A reference to this uri_builder to support chaining.</returns>
    uri_builder &append_query(const std::wstring &query, bool do_encoding = false);

    /// <summary>
    /// Appends an relative uri (Path, Query and fragment) at the end of the current uri.
    /// </summary>
    /// <param name="relative_uri">The relative uri to append.</param>
    /// <returns>A reference to this uri_builder to support chaining.</returns>
    uri_builder &append(const uri &relative_uri);

    /// <summary>
    /// Appends another query to the query of this uri_builder, encoding it first. This overload is useful when building a query segment of
    /// the form "element=10", where the right hand side of the query is stored as a type other than a string, for instance, an integral type.
    /// </summary>
    /// <param name="name">The name portion of the query string</param>
    /// <param name="value">The value portion of the query string</param>
    /// <returns>A reference to this uri_builder to support chaining.</returns>
    template<typename T>
    uri_builder &append_query(std::wstring name, const T &value, bool do_encoding = true)
    {
        utility::ostringstream_t ss;
        ss << name << _XPLATSTR("=") << value;
        return append_query(ss.str(), do_encoding);
    }

    /// <summary>
    /// Combine and validate the URI components into a encoded string. An exception will be thrown if the URI is invalid.
    /// </summary>
    /// <returns>The created URI as a string.</returns>
    std::wstring to_string();

    /// <summary>
    /// Combine and validate the URI components into a URI class instance. An exception will be thrown if the URI is invalid.
    /// </summary>
    /// <returns>The create URI as a URI class instance.</returns>
    uri to_uri();

    /// <summary>
    /// Validate the generated URI from all existing components of this uri_builder.
    /// </summary>
    /// <returns>Whether the URI is valid.</returns>
    bool is_valid();

private:
    details::uri_components m_uri;
};



}   // namespace web
}   // namespace NX



#endif