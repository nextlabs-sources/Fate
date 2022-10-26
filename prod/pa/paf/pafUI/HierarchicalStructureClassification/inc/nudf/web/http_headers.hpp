

#ifndef __NUDF_HTTP_HEADERS_HPP__
#define __NUDF_HTTP_HEADERS_HPP__

#include <map>
#include <memory>
#include <string>
#include <vector>
#include <functional>

#include <nudf\web\date_time.hpp>

namespace NX {
namespace web {
namespace http {


/// <summary>
/// Binds an individual reference to a string value.
/// </summary>
/// <typeparam name="key_type">The type of string value.</typeparam>
/// <typeparam name="_t">The type of the value to bind to.</typeparam>
/// <param name="text">The string value.</param>
/// <param name="ref">The value to bind to.</param>
/// <returns><c>true</c> if the binding succeeds, <c>false</c> otherwise.</returns>
template<typename key_type, typename _t>
bool bind(const key_type &text, _t &ref) // const
{
    std::wistringstream iss(text);
    iss >> ref;
    if (iss.fail() || !iss.eof()) {
        return false;
    }

    return true;
}

/// <summary>
/// Binds an individual reference to a string value.
/// This specialization is need because <c>istringstream::&gt;&gt;</c> delimits on whitespace.
/// </summary>
/// <typeparam name="key_type">The type of the string value.</typeparam>
/// <param name="text">The string value.</param>
/// <param name="ref">The value to bind to.</param>
/// <returns><c>true</c> if the binding succeeds, <c>false</c> otherwise.</returns>
template <typename key_type>
bool bind(const key_type &text, std::wstring &ref) //const
{
    ref = text;
    return true;
}

/// <summary>
/// Represents HTTP headers, acts like a map.
/// </summary>
class http_headers
{
public:
    /// Function object to perform case insensitive comparison of wstrings.
    struct _case_insensitive_cmp
    {
        bool operator()(const std::wstring &str1, const std::wstring &str2) const
        {
            return _wcsicmp(str1.c_str(), str2.c_str()) < 0;
        }
    };

    /// <summary>
    /// STL-style typedefs
    /// </summary>
    typedef std::map<std::wstring, std::wstring, _case_insensitive_cmp>::value_type value_type;
    typedef std::map<std::wstring, std::wstring, _case_insensitive_cmp>::key_type key_type;
    typedef std::map<std::wstring, std::wstring, _case_insensitive_cmp>::key_compare key_compare;
    typedef std::map<std::wstring, std::wstring, _case_insensitive_cmp>::allocator_type allocator_type;
    typedef std::map<std::wstring, std::wstring, _case_insensitive_cmp>::size_type size_type;
    typedef std::map<std::wstring, std::wstring, _case_insensitive_cmp>::difference_type difference_type;
    typedef std::map<std::wstring, std::wstring, _case_insensitive_cmp>::pointer pointer;
    typedef std::map<std::wstring, std::wstring, _case_insensitive_cmp>::const_pointer const_pointer;
    typedef std::map<std::wstring, std::wstring, _case_insensitive_cmp>::reference reference;
    typedef std::map<std::wstring, std::wstring, _case_insensitive_cmp>::const_reference const_reference;
    typedef std::map<std::wstring, std::wstring, _case_insensitive_cmp>::iterator iterator;
    typedef std::map<std::wstring, std::wstring, _case_insensitive_cmp>::const_iterator const_iterator;
    typedef std::map<std::wstring, std::wstring, _case_insensitive_cmp>::reverse_iterator reverse_iterator;
    typedef std::map<std::wstring, std::wstring, _case_insensitive_cmp>::const_reverse_iterator const_reverse_iterator;

    /// <summary>
    /// Constructs an empty set of HTTP headers.
    /// </summary>
    http_headers() {}

    /// <summary>
    /// Copy constructor.
    /// </summary>
    /// <param name="other">An <c>http_headers</c> object to copy from.</param>
    http_headers(const http_headers &other) : m_headers(other.m_headers) {}

    /// <summary>
    /// Assignment operator.
    /// </summary>
    /// <param name="other">An <c>http_headers</c> object to copy from.</param>
    http_headers &operator=(const http_headers &other)
    {
        if(this != &other) {
            m_headers = other.m_headers;
        }
        return *this;
    }

    /// <summary>
    /// Move constructor.
    /// </summary>
    /// <param name="other">An <c>http_headers</c> object to move.</param>
    http_headers(http_headers &&other) : m_headers(std::move(other.m_headers)) {}

    /// <summary>
    /// Move assignment operator.
    /// </summary>
    /// <param name="other">An <c>http_headers</c> object to move.</param>
    http_headers &operator=(http_headers &&other)
    {
        if(this != &other) {
            m_headers = std::move(other.m_headers);
        }
        return *this;
    }

    /// <summary>
    /// Adds a header field using the '&lt;&lt;' operator.
    /// </summary>
    /// <param name="name">The name of the header field.</param>
    /// <param name="value">The value of the header field.</param>
    /// <remark>If the header field exists, the value will be combined as comma separated string.</remark>
    template<typename _t1>
    void add(const key_type& name, const _t1& value)
    {
        if (has(name)) {
            m_headers[name] =  m_headers[name].append(L", " + NX::utility::conversions::print_string(value));
        }
        else {
            m_headers[name] = NX::utility::conversions::print_string(value);
        }
    }

    /// <summary>
    /// Removes a header field.
    /// </summary>
    /// <param name="name">The name of the header field.</param>
    void remove(const key_type& name)
    {
        m_headers.erase(name);
    }

    /// <summary>
    /// Removes all elements from the hearders
    /// </summary>
    void clear() { m_headers.clear(); }

    /// <summary>
    /// Checks if there is a header with the given key.
    /// </summary>
    /// <param name="name">The name of the header field.</param>
    /// <returns><c>true</c> if there is a header with the given name, <c>false</c> otherwise.</returns>
    bool has(const key_type& name) const { return m_headers.find(name) != m_headers.end(); }

    /// <summary>
    /// Returns the number of header fields.
    /// </summary>
    /// <returns>Number of header fields.</returns>
    size_type size() const { return m_headers.size(); }

    /// <summary>
    /// Tests to see if there are any header fields.
    /// </summary>
    /// <returns><c>true</c> if there are no headers, <c>false</c> otherwise.</returns>
    bool empty() const { return m_headers.empty(); }

    /// <summary>
    /// Returns a reference to header field with given name, if there is no header field one is inserted.
    /// </summary>
    std::wstring & operator[](const key_type &name) { return m_headers[name]; }

    /// <summary>
    /// Checks if a header field exists with given name and returns an iterator if found. Otherwise
    /// and iterator to end is returned.
    /// </summary>
    /// <param name="name">The name of the header field.</param>
    /// <returns>An iterator to where the HTTP header is found.</returns>
    iterator find(const key_type &name) { return m_headers.find(name); }
    const_iterator find(const key_type &name) const { return m_headers.find(name); }

    /// <summary>
    /// Attempts to match a header field with the given name using the '>>' operator.
    /// </summary>
    /// <param name="name">The name of the header field.</param>
    /// <param name="value">The value of the header field.</param>
    /// <returns><c>true</c> if header field was found and successfully stored in value parameter.</returns>
    template<typename _t1>
    bool match(const key_type &name, _t1 &value) const
    {
        auto iter = m_headers.find(name);
        if (iter != m_headers.end()) {
            // Check to see if doesn't have a value.
            if(iter->second.empty()) {
                http::bind(iter->second, value);
                return true;
            }
            return http::bind(iter->second, value);
        }
        else {
            return false;
        }
    }

    /// <summary>
    /// Returns an iterator refering to the first header field.
    /// </summary>
    /// <returns>An iterator to the beginning of the HTTP headers</returns>
    iterator begin() { return m_headers.begin(); }
    const_iterator begin() const { return m_headers.begin(); }

    /// <summary>
    /// Returns an iterator referring to the past-the-end header field.
    /// </summary>
            /// <returns>An iterator to the element past the end of the HTTP headers</returns>
    iterator end() { return m_headers.end(); }
    const_iterator end() const { return m_headers.end(); }

    /// <summary>
    /// Gets the content length of the message.
    /// </summary>
    /// <returns>The length of the content.</returns>
    unsigned __int64 content_length() const;

    /// <summary>
    /// Sets the content length of the message.
    /// </summary>
    /// <param name="length">The length of the content.</param>
    void set_content_length(unsigned __int64 length);

    /// <summary>
    /// Gets the content type of the message.
    /// </summary>
    /// <returns>The content type of the body.</returns>
    std::wstring content_type() const;

    /// <summary>
    /// Sets the content type of the message.
    /// </summary>
    /// <param name="type">The content type of the body.</param>
    void set_content_type(std::wstring type);

    /// <summary>
    /// Gets the cache control header of the message.
    /// </summary>
    /// <returns>The cache control header value.</returns>
    std::wstring cache_control() const;

    /// <summary>
    /// Sets the cache control header of the message.
    /// </summary>
    /// <param name="control">The cache control header value.</param>
    void set_cache_control(std::wstring control);

    /// <summary>
    /// Gets the date header of the message.
    /// </summary>
    /// <returns>The date header value.</returns>
    std::wstring date() const;

    /// <summary>
    /// Sets the date header of the message.
    /// </summary>
    /// <param name="date">The date header value.</param>
    void set_date(const NX::utility::datetime& date);

private:

    // Headers are stored in a map with case insensitive key.
    std::map<std::wstring, std::wstring, _case_insensitive_cmp> m_headers;
};


}
}
}


#endif