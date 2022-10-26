
#ifndef __PCS_RPC_HPP__
#define __PCS_RPC_HPP__

#include <cstdlib>

#define PCS_RPC_VERSION_MAJOR 1
#define PCS_RPC_VERSION_MINOR 0

typedef void (*PCS_RPC_DISPATCH)( const char* in_string , char** out_string );

class pcs_rpc_request
{
public:
  static bool get_method( const char* in_string , char* out_method , size_t in_method_count )
  {
    const char* p = strstr(in_string,"method=");
    if( p == NULL )
      return false;

    p += _countof("method=") - 1;    /* move past key name */
    const char* q = strstr(p," ");
    if( q == NULL )
      return false;

    size_t count = q - p + 1;
    if( count > in_method_count )
    {
      SetLastError(ERROR_INSUFFICIENT_BUFFER);
      return false;
    }

    strncpy_s(out_method,count,p,_TRUNCATE);
    return true;
  }/* get_method */

};

#endif /* __PCS_RPC_HPP__ */
