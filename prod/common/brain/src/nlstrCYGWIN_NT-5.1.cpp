//
// All sources, binaries and HTML pages (C) copyright 2007 by NextLabs Inc, 
// San Mateo CA, Ownership remains with NextLabs Inc, 
// All rights reserved worldwide. 
// 
// Author : Dominic Lam
// Date   : 1/4/2007
// Note   : This is a OS abstraction layer for the String type
//
// Best viewed by 80-column terminal
// <-------------------------------------------------------------------------->

#include "nlstrings.h"
#include "brain.h"

nlchar * nlstrcat (nlchar *dest, const nlchar *src)
{
#pragma warning( push )
#pragma warning( disable : 4996 )
  return _tcscat (dest, src);
#pragma warning( pop )
}

bool nlstrcat_s (_Inout_z_cap_(len) nlchar *dest, _In_ size_t len, _In_z_ const nlchar *src)
{
  return (_tcscat_s (dest, len, src) == 0);
}

nlchar * nlstrncat  (nlchar *dest, const nlchar *src, size_t len)
{
#pragma warning( push )
#pragma warning( disable : 4996 )
  return _tcsncat (dest, src, len);
#pragma warning( pop )
}

bool nlstrncat_s  (_Inout_z_cap_(destLen) nlchar *dest, size_t destLen, _In_z_ const nlchar *src, _In_ size_t len)
{
  return (_tcsncat_s (dest, destLen, src, len) == 0);
}

_Check_return_ _Ret_opt_z_ nlchar    *  nlstrchr   (_In_z_ const nlchar * str, _In_ nlint32 c)
{
#pragma warning(push)
#pragma warning(disable: 4244)	
  return (nlchar *) _tcschr (str, c);
#pragma warning(pop)
}

_Check_return_ _Ret_opt_z_ nlchar * nlstrrchr (_In_z_ const nlchar * str, _In_ nlint32 c)
{
#pragma warning(push)
#pragma warning(disable: 4244)	
  return (nlchar *) _tcsrchr(str, c);
#pragma warning(pop)
}

_Check_return_ nlint32      nlstrcmp   (_In_z_ const nlchar * s1, _In_z_ const nlchar * s2)
{
  return _tcscmp (s1, s2);
}

_Check_return_ nlint32 nlstrncmp (_In_z_ const nlchar * s1, _In_z_ const nlchar * s2, _In_ size_t len)
{
  return _tcsncmp (s1, s2, len);
}

_Check_return_ nlint32 nlstricmp (_In_z_ const nlchar * s1, _In_z_ const nlchar * s2)
{
  return _tcsicmp (s1, s2);
}

_Check_return_ nlint32 nlstrnicmp (_In_z_ const nlchar * s1, _In_z_ const nlchar * s2, _In_ size_t len)
{
  return _tcsnicmp (s1, s2, len);
}

nlchar * nlstrcpy (nlchar *dest, const nlchar *src)
{
#pragma warning(push)
#pragma warning(disable: 4996)	
  return _tcscpy (dest, src);
#pragma warning(pop)
}

bool nlstrcpy_s (_Out_z_cap_(len) nlchar *dest, _In_ size_t len, _In_z_ const nlchar *src)
{
  return (_tcscpy_s (dest, len, src) == 0);
}

nlchar * nlstrncpy (nlchar *dest, const nlchar *src, size_t len)
{
#pragma warning(push)
#pragma warning(disable: 4996)	
  return _tcsncpy (dest, src, len);
#pragma warning(pop)
}

bool nlstrncpy_s(_Out_z_cap_(destLen) nlchar *dest, _In_ size_t destLen, _In_z_ const nlchar *src, _In_ size_t len)
{
  return (_tcsncpy_s (dest, destLen, src, len) == 0);
}

_Check_return_ _Ret_opt_z_ nlchar * nlstrdup (_In_z_ const nlchar * str)
{
  return _tcsdup (str);
}

_Check_return_ size_t nlstrlen (_In_z_ const nlchar * str)
{
    return _tcslen (str);
}

_Check_return_ size_t nlstrblen (_In_z_ const nlchar * str)
{
    return _tcslen (str) * sizeof(TCHAR);
}

_Check_return_ _Ret_opt_z_ nlchar * nlstrstr (_In_z_ const nlchar * str, _In_z_ const nlchar *substr)
{
  return (nlchar *) _tcsstr (str, substr);
}

//Convert nlchar * to char *
//"ascii_buf_size" is the size of buffer "ascii_buf"
bool nlstrtoascii(_In_z_ const nlchar *nl_str, _Out_z_bytecap_(ascii_buf_size) char *ascii_buf, _In_ unsigned int ascii_buf_size)
{
    size_t byte_size=nlstrblen(nl_str);
    if(byte_size >= ascii_buf_size)  {
      *ascii_buf = '\0';
      return false; //The buffer "ascii_str" is not big enough
    }
    
    if(byte_size == nlstrlen(nl_str)) {
        //not unicode string
        strncpy_s(ascii_buf, ascii_buf_size, (char *)nl_str, _TRUNCATE);
    } else 
        if (ascii_buf_size > INT_MAX)
          ascii_buf_size = INT_MAX;
        ::WideCharToMultiByte(CP_ACP, 0, nl_str, -1, ascii_buf, 
                              (int) ascii_buf_size, NULL, NULL);
    return true;  
}

//This function copies the source string to destination string base on
//the local machine platform.
//On Windows, the supported unicode standard is UTF-16; 
//on Linux, UTF-8 is supported. 
//If the current platform is Linux,
//-- if the source string is unicode (it has to be UTF-16), we convert 
//   the source to UTF-8.
//-- if the source string is ascii string, we just simply call strcpy.
//If the current platform is Windows and support unicode, 
//-- if the source string is not UTF-16 (which has to be UTF-8), 
//   we convert the source to UTF-16. 
bool nlstrplatformcpy(_Out_z_cap_(des_len) nlchar *dest, _In_z_count_(src_len) const nlchar *source, 
                      _In_ int &src_len, _In_ int src_byte_len, _In_ int des_len)
{
  if(src_len >= des_len) {
    *dest = '\0';
    return false;
  }

#ifdef UNICODE
  if(src_byte_len != src_len) {
    //The source string is in UTF-16 format; 
    nlstrncpy_s(dest, des_len, source, src_len);
    dest[src_len]='\0';
  } else {
    //Convert it from UTF-8 to Windows UTF-16
    int numWC=::MultiByteToWideChar(CP_UTF8, 0, (char *)source, 
				    src_len, dest, des_len);
    dest[numWC]='\0';
  }
#else
  if(src_byte_len != src_len) {
    //The source string is in unicode format (which has to be UTF-8); 
    //convert it to ascii 
    ::WideCharToMultiByte(CP_ACP, 0, source, src_len, dest, 
			  des_len, NULL, NULL);
    dest[src_len]='\0';
  } else {
    nlstrncpy_s(dest, des_len, source, src_len);
    dest[src_len]='\0';
  }  
#endif
  
  return true;
}

/* Convert a string to lower case. The destLength is the length of*/
/* destination string buffer; the lenght of source string doesn't */
/* include the terminal NULL.                                     */
void nlstrntolow(_Out_z_cap_(destLength) nlchar *dest, _In_ size_t destLength,
		 _In_z_count_(srcLength) const nlchar *src, _In_ size_t srcLength)
{
  //NULL dest string, do nothing
  if(dest == NULL || destLength <= 0)
    return;
  
  //NULL source string, do nothing
  if(src == NULL || srcLength <= 0) {
    *dest = '\0';
    return;
  }


  //The destination buffer is not big enough, do nothing
  if(destLength <=srcLength)
    return;

  for( int i=0; i<(int)srcLength; i++) {
    if(src[i]<='Z' && src[i]>='A')
      dest[i]='a'+(src[i]-'A');
    else
      dest[i]=src[i];
  }

  //redundant if-clause added here just to keep Veracode happy
  if (srcLength < destLength)
    dest[srcLength]='\0';
  return;
}
