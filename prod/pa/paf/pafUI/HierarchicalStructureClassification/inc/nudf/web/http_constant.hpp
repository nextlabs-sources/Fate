

#ifndef __NUDF_HTTP_CONSTANT_HPP__
#define __NUDF_HTTP_CONSTANT_HPP__

#include <map>
#include <memory>
#include <string>
#include <vector>
#include <functional>

namespace NX {
namespace web {
namespace http {

namespace message_direction
{
    /// <summary>
    /// Enumeration used to denote the direction of a message: a request with a body is
    /// an upload, a response with a body is a download.
    /// </summary>
    enum direction {
        upload,
        download
    };
}

typedef  std::wstring   method;
typedef  std::wstring   reason_phrase;
typedef unsigned short  status_code;
typedef std::function<void(message_direction::direction, unsigned __int64)> progress_handler;

class methods
{
public:
    const static method GET;
    const static method POST;
    const static method PUT;
    const static method DEL;
    const static method HEAD;
    const static method OPTIONS;
    const static method TRCE;
    const static method CONNECT;
    const static method MERGE;
    const static method PATCH;
};

class status_codes
{
public:
    const static status_code Continue               = 100;
    const static status_code SwitchingProtocols     = 101;
    const static status_code OK                     = 200;
    const static status_code Created                = 201;
    const static status_code Accepted               = 202;
    const static status_code NonAuthInfo            = 203;
    const static status_code NoContent              = 204;
    const static status_code ResetContent           = 205;
    const static status_code PartialContent         = 206;
    const static status_code MultipleChoices        = 300;
    const static status_code MovedPermanently       = 301;
    const static status_code Found                  = 302;
    const static status_code SeeOther               = 303;
    const static status_code NotModified            = 304;
    const static status_code UseProxy               = 305;
    const static status_code TemporaryRedirect      = 307;
    const static status_code BadRequest             = 400;
    const static status_code Unauthorized           = 401;
    const static status_code PaymentRequired        = 402;
    const static status_code Forbidden              = 403;
    const static status_code NotFound               = 404;
    const static status_code MethodNotAllowed       = 405;
    const static status_code NotAcceptable          = 406;
    const static status_code ProxyAuthRequired      = 407;
    const static status_code RequestTimeout         = 408;
    const static status_code Conflict               = 409;
    const static status_code Gone                   = 410;
    const static status_code LengthRequired         = 411;
    const static status_code PreconditionFailed     = 412;
    const static status_code RequestEntityTooLarge  = 413;
    const static status_code RequestUriTooLarge     = 414;
    const static status_code UnsupportedMediaType   = 415;
    const static status_code RangeNotSatisfiable    = 416;
    const static status_code ExpectationFailed      = 417;
    const static status_code InternalError          = 500;
    const static status_code NotImplemented         = 501;
    const static status_code BadGateway             = 502;
    const static status_code ServiceUnavailable     = 503;
    const static status_code GatewayTimeout         = 504;
    const static status_code HttpVersionNotSupported= 505;
};

struct http_status_to_phrase
{
    unsigned short id;
    reason_phrase phrase;
};

class header_names
{
public:
    const static std::wstring accept;
    const static std::wstring accept_charset;
    const static std::wstring accept_encoding;
    const static std::wstring accept_language;
    const static std::wstring accept_ranges;
    const static std::wstring age;
    const static std::wstring allow;
    const static std::wstring authorization;
    const static std::wstring cache_control;
    const static std::wstring connection;
    const static std::wstring content_encoding;
    const static std::wstring content_language;
    const static std::wstring content_length;
    const static std::wstring content_location;
    const static std::wstring content_md5;
    const static std::wstring content_range;
    const static std::wstring content_type;
    const static std::wstring date;
    const static std::wstring etag;
    const static std::wstring expect;
    const static std::wstring expires;
    const static std::wstring from;
    const static std::wstring host;
    const static std::wstring if_match;
    const static std::wstring if_modified_since;
    const static std::wstring if_none_match;
    const static std::wstring if_range;
    const static std::wstring if_unmodified_since;
    const static std::wstring last_modified;
    const static std::wstring location;
    const static std::wstring max_forwards;
    const static std::wstring pragma;
    const static std::wstring proxy_authenticate;
    const static std::wstring proxy_authorization;
    const static std::wstring range;
    const static std::wstring referer;
    const static std::wstring retry_after;
    const static std::wstring server;
    const static std::wstring te;
    const static std::wstring trailer;
    const static std::wstring transfer_encoding;
    const static std::wstring upgrade;
    const static std::wstring user_agent;
    const static std::wstring vary;
    const static std::wstring via;
    const static std::wstring warning;
    const static std::wstring www_authenticate;
};


class mime_types
{
public:
    const static std::wstring application_atom_xml;
    const static std::wstring application_http;
    const static std::wstring application_javascript;
    const static std::wstring application_json;
    const static std::wstring application_xjson;
    const static std::wstring application_octetstream;
    const static std::wstring application_x_www_form_urlencoded;
    const static std::wstring application_xjavascript;
    const static std::wstring application_xml;
    const static std::wstring message_http;
    const static std::wstring text;
    const static std::wstring text_javascript;
    const static std::wstring text_json;
    const static std::wstring text_plain;
    const static std::wstring text_plain_utf16;
    const static std::wstring text_plain_utf16le;
    const static std::wstring text_plain_utf8;
    const static std::wstring text_xjavascript;
    const static std::wstring text_xjson;
};

class charset_types
{
public:
    const static std::wstring ascii;
    const static std::wstring usascii;
    const static std::wstring latin1;
    const static std::wstring utf8;
    const static std::wstring utf16;
    const static std::wstring utf16le;
    const static std::wstring utf16be;
};

typedef std::wstring oauth1_method;
class oauth1_methods
{
public:
    const static std::wstring hmac_sha1;
    const static std::wstring plaintext;
};

class oauth1_strings
{
public:
    const static std::wstring callback;
    const static std::wstring callback_confirmed;
    const static std::wstring consumer_key;
    const static std::wstring nonce;
    const static std::wstring realm;
    const static std::wstring signature;
    const static std::wstring signature_method;
    const static std::wstring timestamp;
    const static std::wstring token;
    const static std::wstring token_secret;
    const static std::wstring verifier;
    const static std::wstring version;
};

class oauth2_strings
{
public:
    const static std::wstring access_token;
    const static std::wstring authorization_code;
    const static std::wstring bearer;
    const static std::wstring client_id;
    const static std::wstring client_secret;
    const static std::wstring code;
    const static std::wstring expires_in;
    const static std::wstring grant_type;
    const static std::wstring redirect_uri;
    const static std::wstring refresh_token;
    const static std::wstring response_type;
    const static std::wstring scope;
    const static std::wstring state;
    const static std::wstring token;
    const static std::wstring token_type;
};



}   // namespace http
}   // namespace web
}   // namespace NX


#endif