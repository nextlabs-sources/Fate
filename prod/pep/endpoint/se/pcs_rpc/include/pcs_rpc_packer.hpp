
#ifndef __PCS_IDL_PACKER_HPP__
#define __PCS_IDL_PACKER_HPP__

#include <windows.h>

#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <exception>
#include <string>

#include "CEsdk.h"
#include "pcs_rpc_version.hpp"

/** packer
 *
 *  \brief Pack types into memory and provide a method to codify
 *         to an ANSI string.
 *
 *  Format:
 *
 *      Each type is packed byte by byte with a prefixed header indicating the
 *      size, in bytes, of the type (payload).
 *
 *      [return=XY]payload=[size_in_bytes][payload][size_in_bytes][payload]....
 *
 */
class packer
{

  public:

    packer(void) :
      buf(NULL),
      buf_size(0),
      coded_string_cpp(),
      method(),
      service(),
      rv(CE_RESULT_SUCCESS)
    {
      /* empty */
    }/* packer */

   /** pack
     *
     *  \brief Pack an arbitrary type.
     */
    template <typename T>
    bool pack( _In_ const T* pack_type ) throw()
    {
      return pack_raw((void*)pack_type,sizeof(T));
    }/* pack */

    template <typename T>
    bool pack( _In_ T* pack_type ) throw()
    {
      return pack_raw((void*)pack_type,sizeof(T));
    }/* pack */

    bool pack( _In_ char* pack_type ) throw()
    {
      size_t len = strlen(pack_type) + sizeof(char); /* string + term */
      return pack_raw((void*)pack_type,len);
    }/* pack */

    bool pack( _In_ const char* pack_type ) throw()
    {
      size_t len = strlen(pack_type) + sizeof(char); /* string + term */
      return pack_raw((void*)pack_type,len);
    }/* pack */

    bool pack( _In_ wchar_t* pack_type ) throw()
    {
      size_t len = (wcslen(pack_type) * sizeof(wchar_t)) + sizeof(wchar_t); /* string + term */
      return pack_raw((void*)pack_type,len);
    }/* pack */

    bool pack( _In_ const wchar_t* pack_type ) throw()
    {
      size_t len = (wcslen(pack_type) * sizeof(wchar_t)) + sizeof(wchar_t); /* string + term */
      return pack_raw((void*)pack_type,len);
    }/* pack */

    /* Pack a data blob into the data member with the size header. */
    bool pack_raw( _In_bytecount_(size) void* data , size_t size ) throw()
    {
      pack_header(static_cast<int> (size));
      unsigned char* new_buf = NULL;
      new_buf = (unsigned char*)realloc( buf , buf_size + size );
      if( new_buf == NULL )
      {
	return false;
      }
      buf = new_buf;
      memcpy(buf + buf_size,data,size);

      buf_size += size;
      return true;
    }/* pack_raw */

    size_t size(void) const
    {
      return buf_size;
    }/* size */

    /** size_coded
     *
     *  \brief Determine the coded size of the data.
     */
    size_t size_coded(void) const
    {
      size_t size = 0;

      size += 512;                  /* header */
      size += buf_size * 2 + 1;     /* payload */

      return size;
    }/* size_coded */

    const std::string& get_coded_string(void)
    {
      char* cs = code_string();
      if( cs != NULL )
      {
	coded_string_cpp.assign(cs);
      }
      free(cs);
      return coded_string_cpp;
    }/* get_coded_string */

    void set_service( const char* in_service )
    {
      service.assign(in_service);
    }/* set_service */

    void set_method( const char* in_method )
    {
      method.assign(in_method);
    }/* set_method */

    void set_return( CEResult_t in_rv )
    {
      rv = in_rv;
    }/* set_return */

    CEResult_t get_return(void) const
    {
      return rv;
    }

    /** assign_code_string
     *
     *  \brief Assign a coded string to the packer so that data can be
     *         extracted.
     */
    void assign_code_string( _In_ const char* in_string )
    {
      assert( in_string );
      size_t num_bytes = sizeof(char) * strlen(in_string) + sizeof(char);
      delete buf;
      buf = (unsigned char*)malloc(num_bytes);
      if( buf == NULL )
      {
	throw std::bad_alloc();
      }
      memset(buf,0x00,num_bytes);

      /* Determine the payload is past the header */
      const char* payload = strstr(in_string,"payload=");
      if( payload == NULL )
      {
	return;
      }
      payload += strlen("payload=");

      size_t in_string_len = strlen(payload);
      buf_size = in_string_len / 2;
      char* p = (char*)buf;

      for( size_t i = 0 ; i < in_string_len ; i += 2 )
      {
	sscanf_s(payload + i,"%02x",p);
	p++;
      }

      payload = strstr(in_string,"return=");
      if( payload != NULL )
      {
	int byte = 0;
	sscanf_s(payload + strlen("return="),"%02x",&byte);
	byte = (char)byte;
	rv = (CEResult_t)byte;
      }

    }/* assign_code_string */

    void assign_code_string( _In_ const wchar_t* in_string )
    {
      assert( in_string != NULL );
      size_t temp_count = wcslen(in_string) + 1;
      char* temp = (char*)malloc(temp_count);
      if( temp == NULL )
      {
	throw std::bad_alloc();
      }
      memset(temp,0x00,temp_count);
      _snprintf_s(temp,temp_count, _TRUNCATE,"%ws",in_string);
      assign_code_string(temp);
      free(temp);
    }/* assign_code_string */

    /** at
     *
     *  \brief Address and size of the parameter at a location.
     */
    void* at( size_t loc , _In_ size_t* out_size )
    {
      unsigned char* p = buf;
      size_t curr_size = 0;
      size_t curr_loc = 0;

      if( buf == NULL )
      {
	return NULL;
      }

      /* Walk to the current blob by traversing the headers until
       * the correct parameter is located.
       */
      curr_size = *( (int*)buf ); // current packed payload size
      for( ; curr_loc < loc ; )
      {
	p += sizeof(int);         // move past curretn packed header
	p += curr_size;           // move past current packed payload
	curr_size = *( (int*)p ); // determine current packed size
	curr_loc++;
      }

      *out_size = curr_size;
      return (p + sizeof(int));
    }/* at */

    ~packer(void)
    {
      if( buf ) free(buf);
    }

  private:

    /** code_string
     *
     *  \brief Code the data into an ANSI string.
     */
    char* code_string(void) const
    {
      char temp[32] = {0};
      char* coded_string = NULL;
      size_t alloc_size = size_coded();
      coded_string = (char*)malloc( alloc_size * sizeof(char) );
      if( coded_string == NULL )
      {
	return NULL;
      }
      memset(coded_string,0x00,alloc_size);

      /* Generate header information including version, service, method, etc. */
      _snprintf_s(coded_string,alloc_size, _TRUNCATE,"pcs_idl version=%02d.%02d service=%s method=%s ",
		PCS_IDL_VERSION_MAJOR,PCS_IDL_VERSION_MINOR,
		service.c_str(), method.c_str());

      /* Code the return value */
      _snprintf_s(temp,_countof(temp), _TRUNCATE," return=%02x ",(unsigned char)rv);
      strncat_s(coded_string,alloc_size,temp, _TRUNCATE);

      /* Code the payload (parameters) */
      _snprintf_s(temp,_countof(temp), _TRUNCATE,"payload=");
      strncat_s(coded_string,alloc_size,temp, _TRUNCATE);

      /* codify the payload */
      for( size_t i = 0 ; i < buf_size ; i++ )
      {
	_snprintf_s(temp,_countof(temp), _TRUNCATE,"%02x",(unsigned char)buf[i]);
	strncat_s(coded_string,alloc_size,temp, _TRUNCATE);
      }

      return coded_string;
    }/* code_string */

    /* Pack the header into the current data buffer */
    bool pack_header( int in_size ) throw()
    {
      unsigned char* new_buf = NULL;
      new_buf = (unsigned char*)realloc( buf , buf_size + sizeof(int) );
      if( new_buf == NULL )
      {
	return false;
      }
      buf = new_buf;
      memcpy(buf + buf_size,&in_size,sizeof(int));
      buf_size += sizeof(int);
      return true;
    }/* pack_header */

    unsigned char* buf;   /* data */
    size_t buf_size;      /* data size in bytes */

    std::string method;
    std::string service;
    std::string coded_string_cpp;
    CEResult_t rv;

};/* packer */

#endif /* __PCS_IDL_PACKER_HPP__ */

