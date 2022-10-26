

#ifndef __NUDF_HTTP_CLIENT_HPP__
#define __NUDF_HTTP_CLIENT_HPP__

#include <map>
#include <memory>
#include <string>
#include <vector>
#include <functional>

#include <winhttp.h>

#include <nudf\web\conversions.hpp>
#include <nudf\web\date_time.hpp>
#include <nudf\web\http_constant.hpp>
#include <nudf\web\http_helpers.hpp>
#include <nudf\web\http_msg.hpp>
#include <nudf\web\containerstream.hpp>
#include <nudf\web\pplxtasks.hpp>


// define native handle
namespace NX { namespace web { namespace http { namespace client {
typedef void* native_handle;
}}}}


namespace NX {
namespace web {
namespace http {
namespace client {



    
// credentials and web_proxy class has been moved from web::http::client namespace to web namespace.
// The below using declarations ensure we dont break existing code.
// Please use the web::credentials and web::web_proxy class going forward.
using NX::web::credentials;
using NX::web::web_proxy;


namespace details {
class winhttp_client;
}

/// <summary>
/// HTTP client configuration class, used to set the possible configuration options
/// used to create an http_client instance.
/// </summary>
class http_client_config
{
public:
    http_client_config() :
        m_guarantee_order(false),
        m_timeout(NX::utility::seconds(30)),
        m_chunksize(0)
        , m_validate_certificates(false)
        , m_set_user_nativehandle_options([](native_handle)->void{})
        , m_buffer_request(false)
        , m_secure_protos(WINHTTP_FLAG_SECURE_PROTOCOL_SSL3| WINHTTP_FLAG_SECURE_PROTOCOL_TLS1| WINHTTP_FLAG_SECURE_PROTOCOL_TLS1_1|WINHTTP_FLAG_SECURE_PROTOCOL_TLS1_2)
    {
    }
    
    /// <summary>
    /// Get the web proxy object
    /// </summary>
    /// <returns>A reference to the web proxy object.</returns>
    const web_proxy& proxy() const
    {
        return m_proxy;
    }

    /// <summary>
    /// Set the web proxy object
    /// </summary>
    /// <param name="proxy">A reference to the web proxy object.</param>
    void set_proxy(web_proxy proxy)
    {
        m_proxy = std::move(proxy);
    }

    /// <summary>
    /// Get the client credentials
    /// </summary>
    /// <returns>A reference to the client credentials.</returns>
    const http::client::credentials& credentials() const
    {
        return m_credentials;
    }

    /// <summary>
    /// Set the client credentials
    /// </summary>
    /// <param name="cred">A reference to the client credentials.</param>
    void set_credentials(const http::client::credentials& cred)
    {
        m_credentials = cred;
    }

    unsigned long secure_protocols() const throw()
    {
        return m_secure_protos;
    }

    void set_secure_protocols(unsigned long protos) throw()
    {
        m_secure_protos = (protos & (WINHTTP_FLAG_SECURE_PROTOCOL_SSL2
                                     | WINHTTP_FLAG_SECURE_PROTOCOL_SSL3
                                     | WINHTTP_FLAG_SECURE_PROTOCOL_TLS1
                                     | WINHTTP_FLAG_SECURE_PROTOCOL_TLS1_1
                                     | WINHTTP_FLAG_SECURE_PROTOCOL_TLS1_2));
    }

    bool support_protocol_ssl2() const throw()
    {
        return (WINHTTP_FLAG_SECURE_PROTOCOL_SSL2 == (m_secure_protos & WINHTTP_FLAG_SECURE_PROTOCOL_SSL2));
    }
    bool support_protocol_ssl3() const throw()
    {
        return (WINHTTP_FLAG_SECURE_PROTOCOL_SSL3 == (m_secure_protos & WINHTTP_FLAG_SECURE_PROTOCOL_SSL3));
    }
    bool support_protocol_tls1() const throw()
    {
        return (WINHTTP_FLAG_SECURE_PROTOCOL_TLS1 == (m_secure_protos & WINHTTP_FLAG_SECURE_PROTOCOL_TLS1));
    }
    bool support_protocol_tls11() const throw()
    {
        return (WINHTTP_FLAG_SECURE_PROTOCOL_TLS1_1 == (m_secure_protos & WINHTTP_FLAG_SECURE_PROTOCOL_TLS1_1));
    }
    bool support_protocol_tls12() const throw()
    {
        return (WINHTTP_FLAG_SECURE_PROTOCOL_TLS1_2 == (m_secure_protos & WINHTTP_FLAG_SECURE_PROTOCOL_TLS1_2));
    }

    /// <summary>
    /// Get the 'guarantee order' property
    /// </summary>
    /// <returns>The value of the property.</returns>
    bool guarantee_order() const
    {
        return m_guarantee_order;
    }

    /// <summary>
    /// Set the 'guarantee order' property
    /// </summary>
    /// <param name="guarantee_order">The value of the property.</param>
    void set_guarantee_order(bool guarantee_order)
    {
        m_guarantee_order = guarantee_order;
    }

    /// <summary>
    /// Get the timeout
    /// </summary>
    /// <returns>The timeout (in seconds) used for each send and receive operation on the client.</returns>
    utility::seconds timeout() const
    {
        return m_timeout;
    }

    /// <summary>
    /// Set the timeout
    /// </summary>
    /// <param name="timeout">The timeout (in seconds) used for each send and receive operation on the client.</param>
    void set_timeout(const utility::seconds &timeout)
    {
        m_timeout = timeout;
    }

    /// <summary>
    /// Get the client chunk size.
    /// </summary>
    /// <returns>The internal buffer size used by the http client when sending and receiving data from the network.</returns>
    size_t chunksize() const
    {
        return m_chunksize == 0 ? 64 * 1024 : m_chunksize;
    }

    /// <summary>
    /// Sets the client chunk size.
    /// </summary>
    /// <param name="size">The internal buffer size used by the http client when sending and receiving data from the network.</param>
    /// <remarks>This is a hint -- an implementation may disregard the setting and use some other chunk size.</remarks>
    void set_chunksize(size_t size)
    {
        m_chunksize = size;
    }

    /// <summary>
    /// Returns true if the default chunk size is in use.
    /// <remarks>If true, implementations are allowed to choose whatever size is best.</remarks>
    /// </summary>
    /// <returns>True if default, false if set by user.</returns>
    bool is_default_chunksize() const
    {
        return m_chunksize == 0;
    }

    /// <summary>
    /// Gets the server certificate validation property.
    /// </summary>
    /// <returns>True if certificates are to be verified, false otherwise.</returns>
    bool validate_certificates() const
    {
        return m_validate_certificates;
    }

    /// <summary>
    /// Sets the server certificate validation property.
    /// </summary>
    /// <param name="validate_certs">False to turn ignore all server certificate validation errors, true otherwise.</param>
    /// <remarks>Note ignoring certificate errors can be dangerous and should be done with caution.</remarks>
    void set_validate_certificates(bool validate_certs)
    {
        m_validate_certificates = validate_certs;
    }

    /// <summary>
    /// Checks if request data buffering is turned on, the default is off.
    /// </summary>
    /// <returns>True if buffering is enabled, false otherwise</returns>
    bool buffer_request() const
    {
        return m_buffer_request;
    }

    /// <summary>
    /// Sets the request buffering property.
    /// If true, in cases where the request body/stream doesn't support seeking the request data will be buffered.
    /// This can help in situations where an authentication challenge might be expected.
    /// </summary>
    /// <param name="buffer_request">True to turn on buffer, false otherwise.</param>
    /// <remarks>Please note there is a performance cost due to copying the request data.</remarks>
    void set_buffer_request(bool buffer_request)
    {
        m_buffer_request = buffer_request;
    }

    /// <summary>
    /// Sets a callback to enable custom setting of winhttp options
    /// </summary>
    /// <param name="callback">A user callback allowing for customization of the request</param>
    void set_nativehandle_options(const std::function<void(native_handle)> &callback)
    {
         m_set_user_nativehandle_options = callback;
    }

private:
    NX::web::web_proxy m_proxy;
    NX::web::credentials m_credentials;

    // Whether or not to guarantee ordering, i.e. only using one underlying TCP connection.
    bool m_guarantee_order;

    NX::utility::seconds m_timeout;
    size_t m_chunksize;

    // IXmlHttpRequest2 doesn't allow configuration of certificate verification.
    bool m_validate_certificates;

    // Secure Protocols used
    unsigned long m_secure_protos;

    std::function<void(native_handle)> m_set_user_nativehandle_options;

    bool m_buffer_request;
    friend class details::winhttp_client;

    /// <summary>
    /// Invokes a user callback to allow for customization of the requst
    /// </summary>
    /// <param name="handle">The internal http_request handle</param>
    /// <returns>True if users set WinHttp/IXAMLHttpRequest2 options correctly, false otherwise.</returns>
    void call_user_nativehandle_options(native_handle handle) const
    {
         m_set_user_nativehandle_options(handle);
    }
};

/// <summary>
/// HTTP client class, used to maintain a connection to an HTTP service for an extended session.
/// </summary>
class http_client
{
public:
    /// <summary>
    /// Creates a new http_client connected to specified uri.
    /// </summary>
    /// <param name="base_uri">A string representation of the base uri to be used for all requests. Must start with either "http://" or "https://"</param>
    http_client(uri base_uri);

    /// <summary>
    /// Creates a new http_client connected to specified uri.
    /// </summary>
    /// <param name="base_uri">A string representation of the base uri to be used for all requests. Must start with either "http://" or "https://"</param>
    /// <param name="client_config">The http client configuration object containing the possible configuration options to intitialize the <c>http_client</c>. </param>
    http_client(uri base_uri, http_client_config client_config);

    /// <summary>
    /// Note the destructor doesn't necessarily close the connection and release resources.
    /// The connection is reference counted with the http_responses.
    /// </summary>
    ~http_client() throw() {}

    /// <summary>
    /// Gets the base uri
    /// </summary>
    /// <returns>
    /// A base uri initialized in constructor
    /// </returns>
    const uri& base_uri() const;

    /// <summary>
    /// Get client configuration object
    /// </summary>
    /// <returns>A reference to the client configuration object.</returns>
    const http_client_config& client_config() const;

    /// <summary>
    /// Adds an HTTP pipeline stage to the client.
    /// </summary>
    /// <param name="handler">A function object representing the pipeline stage.</param>
    void add_handler(std::function<pplx::task<http_response>(http_request, std::shared_ptr<http_pipeline_stage>)> handler)
    {
        m_pipeline->append(std::make_shared<NX::web::http::details::function_pipeline_wrapper>(handler));
    }

    /// <summary>
    /// Adds an HTTP pipeline stage to the client.
    /// </summary>
    /// <param name="stage">A shared pointer to a pipeline stage.</param>
    void add_handler(const std::shared_ptr<http_pipeline_stage> &stage)
    {
        m_pipeline->append(stage);
    }

    /// <summary>
    /// Asynchronously sends an HTTP request.
    /// </summary>
    /// <param name="request">Request to send.</param>
    /// <param name="token">Cancellation token for cancellation of this request operation.</param>
    /// <returns>An asynchronous operation that is completed once a response from the request is received.</returns>
    pplx::task<http_response> request(http_request request, const pplx::cancellation_token &token = pplx::cancellation_token::none());

    /// <summary>
    /// Asynchronously sends an HTTP request.
    /// </summary>
    /// <param name="mtd">HTTP request method.</param>
    /// <param name="token">Cancellation token for cancellation of this request operation.</param>
    /// <returns>An asynchronous operation that is completed once a response from the request is received.</returns>
    pplx::task<http_response> request(const method &mtd, const pplx::cancellation_token &token = pplx::cancellation_token::none())
    {
        http_request msg(mtd);
        return request(msg, token);
    }

    /// <summary>
    /// Asynchronously sends an HTTP request.
    /// </summary>
    /// <param name="mtd">HTTP request method.</param>
    /// <param name="path_query_fragment">String containing the path, query, and fragment, relative to the http_client's base URI.</param>
    /// <param name="token">Cancellation token for cancellation of this request operation.</param>
    /// <returns>An asynchronous operation that is completed once a response from the request is received.</returns>
    pplx::task<http_response> request(
        const method &mtd,
        const std::wstring &path_query_fragment,
        const pplx::cancellation_token &token = pplx::cancellation_token::none())
    {
        http_request msg(mtd);
        msg.set_request_uri(path_query_fragment);
        return request(msg, token);
    }

    /// <summary>
    /// Asynchronously sends an HTTP request.
    /// </summary>
    /// <param name="mtd">HTTP request method.</param>
    /// <param name="path_query_fragment">String containing the path, query, and fragment, relative to the http_client's base URI.</param>
    /// <param name="body_data">The data to be used as the message body, represented using the json object library.</param>
    /// <param name="token">Cancellation token for cancellation of this request operation.</param>
    /// <returns>An asynchronous operation that is completed once a response from the request is received.</returns>
    pplx::task<http_response> request(
        const method &mtd,
        const std::wstring &path_query_fragment,
        const json::value &body_data,
        const pplx::cancellation_token &token = pplx::cancellation_token::none())
    {
        http_request msg(mtd);
        msg.set_request_uri(path_query_fragment);
        msg.set_body(body_data);
        return request(msg, token);
    }

    /// <summary>
    /// Asynchronously sends an HTTP request with a string body. Assumes the
    /// character encoding of the string is UTF-8.
    /// </summary>
    /// <param name="mtd">HTTP request method.</param>
    /// <param name="path_query_fragment">String containing the path, query, and fragment, relative to the http_client's base URI.</param>
    /// <param name="content_type">A string holding the MIME type of the message body.</param>
    /// <param name="body_data">String containing the text to use in the message body.</param>
    /// <param name="token">Cancellation token for cancellation of this request operation.</param>
    /// <returns>An asynchronous operation that is completed once a response from the request is received.</returns>
    pplx::task<http_response> request(
        const method &mtd,
        const std::string &path_query_fragment,
        const std::string &body_data,
        const std::string &content_type = "text/plain; charset=utf-8",
        const pplx::cancellation_token &token = pplx::cancellation_token::none())
    {
        http_request msg(mtd);
        msg.set_request_uri(NX::utility::conversions::to_string(path_query_fragment));
        msg.set_body(body_data, content_type);
        return request(msg, token);
    }

    /// <summary>
    /// Asynchronously sends an HTTP request with a string body. Assumes the
    /// character encoding of the string is UTF-8.
    /// </summary>
    /// <param name="mtd">HTTP request method.</param>
    /// <param name="path_query_fragment">String containing the path, query, and fragment, relative to the http_client's base URI.</param>
    /// <param name="content_type">A string holding the MIME type of the message body.</param>
    /// <param name="body_data">String containing the text to use in the message body.</param>
    /// <param name="token">Cancellation token for cancellation of this request operation.</param>
    /// <returns>An asynchronous operation that is completed once a response from the request is received.</returns>
    pplx::task<http_response> request(
        const method &mtd,
        const std::string &path_query_fragment,
        std::string &&body_data,
        const std::string &content_type = "text/plain; charset=utf-8",
        const pplx::cancellation_token &token = pplx::cancellation_token::none())
    {
        http_request msg(mtd);
        msg.set_request_uri(NX::utility::conversions::to_string(path_query_fragment));
        msg.set_body(std::move(body_data), content_type);
        return request(msg, token);
    }

    /// <summary>
    /// Asynchronously sends an HTTP request with a string body. Assumes the
    /// character encoding of the string is UTF-16 will perform conversion to UTF-8.
    /// </summary>
    /// <param name="mtd">HTTP request method.</param>
    /// <param name="path_query_fragment">String containing the path, query, and fragment, relative to the http_client's base URI.</param>
    /// <param name="content_type">A string holding the MIME type of the message body.</param>
    /// <param name="body_data">String containing the text to use in the message body.</param>
    /// <param name="token">Cancellation token for cancellation of this request operation.</param>
    /// <returns>An asynchronous operation that is completed once a response from the request is received.</returns>
    pplx::task<http_response> request(
        const method &mtd,
        const std::wstring &path_query_fragment,
        const std::wstring &body_data,
        const std::wstring &content_type = NX::utility::conversions::to_utf16string("text/plain"),
        const pplx::cancellation_token &token = pplx::cancellation_token::none())
    {
        NX::web::http::http_request msg(mtd);
        msg.set_request_uri(NX::utility::conversions::to_string(path_query_fragment));
        msg.set_body(body_data, content_type);
        return request(msg, token);
    }

    /// <summary>
    /// Asynchronously sends an HTTP request with a string body. Assumes the
    /// character encoding of the string is UTF-8.
    /// </summary>
    /// <param name="mtd">HTTP request method.</param>
    /// <param name="path_query_fragment">String containing the path, query, and fragment, relative to the http_client's base URI.</param>
    /// <param name="body_data">String containing the text to use in the message body.</param>
    /// <param name="token">Cancellation token for cancellation of this request operation.</param>
    /// <returns>An asynchronous operation that is completed once a response from the request is received.</returns>
    pplx::task<http_response> request(
        const method &mtd,
        const std::string &path_query_fragment,
        const std::string &body_data,
        const pplx::cancellation_token &token)
    {
        return request(mtd, path_query_fragment, body_data, "text/plain; charset=utf-8", token);
    }

    /// <summary>
    /// Asynchronously sends an HTTP request with a string body. Assumes the
    /// character encoding of the string is UTF-8.
    /// </summary>
    /// <param name="mtd">HTTP request method.</param>
    /// <param name="path_query_fragment">String containing the path, query, and fragment, relative to the http_client's base URI.</param>
    /// <param name="body_data">String containing the text to use in the message body.</param>
    /// <param name="token">Cancellation token for cancellation of this request operation.</param>
    /// <returns>An asynchronous operation that is completed once a response from the request is received.</returns>
    pplx::task<http_response> request(
        const method &mtd,
        const std::string &path_query_fragment,
        std::string &&body_data,
        const pplx::cancellation_token &token)
    {
        http_request msg(mtd);
        msg.set_request_uri(NX::utility::conversions::to_string(path_query_fragment));
        msg.set_body(std::move(body_data), "text/plain; charset=utf-8");
        return request(msg, token);
    }

    /// <summary>
    /// Asynchronously sends an HTTP request with a string body. Assumes
    /// the character encoding of the string is UTF-16 will perform conversion to UTF-8.
    /// </summary>
    /// <param name="mtd">HTTP request method.</param>
    /// <param name="path_query_fragment">String containing the path, query, and fragment, relative to the http_client's base URI.</param>
    /// <param name="body_data">String containing the text to use in the message body.</param>
    /// <param name="token">Cancellation token for cancellation of this request operation.</param>
    /// <returns>An asynchronous operation that is completed once a response from the request is received.</returns>
    pplx::task<http_response> request(
        const method &mtd,
        const std::wstring &path_query_fragment,
        const std::wstring &body_data,
        const pplx::cancellation_token &token)
    {
        return request(mtd, path_query_fragment, body_data, NX::utility::conversions::to_utf16string("text/plain"), token);
    }

#if !defined (__cplusplus_winrt)
    /// <summary>
    /// Asynchronously sends an HTTP request.
    /// </summary>
    /// <param name="mtd">HTTP request method.</param>
    /// <param name="path_query_fragment">String containing the path, query, and fragment, relative to the http_client's base URI.</param>
    /// <param name="body">An asynchronous stream representing the body data.</param>
    /// <param name="content_type">A string holding the MIME type of the message body.</param>
    /// <param name="token">Cancellation token for cancellation of this request operation.</param>
    /// <returns>A task that is completed once a response from the request is received.</returns>
    pplx::task<http_response> request(
        const method &mtd,
        const std::wstring &path_query_fragment,
        const concurrency::streams::istream &body,
        const std::wstring &content_type = L"application/octet-stream",
        const pplx::cancellation_token &token = pplx::cancellation_token::none())
    {
        http_request msg(mtd);
        msg.set_request_uri(path_query_fragment);
        msg.set_body(body, content_type);
        return request(msg, token);
    }

    /// <summary>
    /// Asynchronously sends an HTTP request.
    /// </summary>
    /// <param name="mtd">HTTP request method.</param>
    /// <param name="path_query_fragment">String containing the path, query, and fragment, relative to the http_client's base URI.</param>
    /// <param name="body">An asynchronous stream representing the body data.</param>
    /// <param name="token">Cancellation token for cancellation of this request operation.</param>
    /// <returns>A task that is completed once a response from the request is received.</returns>
    pplx::task<http_response> request(
        const method &mtd,
        const std::wstring &path_query_fragment,
        const concurrency::streams::istream &body,
        const pplx::cancellation_token &token)
    {
        return request(mtd, path_query_fragment, body, L"application/octet-stream", token);
    }
#endif // __cplusplus_winrt

    /// <summary>
    /// Asynchronously sends an HTTP request.
    /// </summary>
    /// <param name="mtd">HTTP request method.</param>
    /// <param name="path_query_fragment">String containing the path, query, and fragment, relative to the http_client's base URI.</param>
    /// <param name="body">An asynchronous stream representing the body data.</param>
    /// <param name="content_length">Size of the message body.</param>
    /// <param name="content_type">A string holding the MIME type of the message body.</param>
    /// <param name="token">Cancellation token for cancellation of this request operation.</param>
    /// <returns>A task that is completed once a response from the request is received.</returns>
    /// <remarks>Winrt requires to provide content_length.</remarks>
    pplx::task<http_response> request(
        const method &mtd,
        const std::wstring &path_query_fragment,
        const concurrency::streams::istream &body,
        size_t content_length,
        const std::wstring &content_type = L"application/octet-stream",
        const pplx::cancellation_token &token = pplx::cancellation_token::none())
    {
        http_request msg(mtd);
        msg.set_request_uri(path_query_fragment);
        msg.set_body(body, content_length, content_type);
        return request(msg, token);
    }

    /// <summary>
    /// Asynchronously sends an HTTP request.
    /// </summary>
    /// <param name="mtd">HTTP request method.</param>
    /// <param name="path_query_fragment">String containing the path, query, and fragment, relative to the http_client's base URI.</param>
    /// <param name="body">An asynchronous stream representing the body data.</param>
    /// <param name="content_length">Size of the message body.</param>
    /// <param name="token">Cancellation token for cancellation of this request operation.</param>
    /// <returns>A task that is completed once a response from the request is received.</returns>
    /// <remarks>Winrt requires to provide content_length.</remarks>
    pplx::task<http_response> request(
        const method &mtd,
        const std::wstring &path_query_fragment,
        const concurrency::streams::istream &body,
        size_t content_length,
        const pplx::cancellation_token &token)
    {
        return request(mtd, path_query_fragment, body, content_length, L"application/octet-stream", token);
    }

private:

    void build_pipeline(uri base_uri, http_client_config client_config);

    std::shared_ptr<NX::web::http::http_pipeline> m_pipeline;
};


}   // namespace client
}   // namespace http
}   // namespace web
}   // namespace NX


#endif