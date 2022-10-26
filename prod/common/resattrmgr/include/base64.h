#ifndef _BASE64_H
#define _BASE64_H
 
#ifdef _WIN32
#pragma warning(disable:4514)
#endif
 
#include "StdAfx.h"
#include <string>
/** \defgroup util Utilities */
 
/** Base64 encode/decode.
    \ingroup util */
class Base64
{
public:
    static inline bool is_base64(unsigned char c) {
        return (isalnum(c) || (c == '+') || (c == '/'));};
    std::string base64_encode(unsigned char const* , unsigned int len);
    std::string base64_encode(std::string const& s);
    std::string base64_decode(unsigned char const* , unsigned int len);
    std::string base64_decode(std::string const& s);
};
 
#endif // _BASE64_H
  
  