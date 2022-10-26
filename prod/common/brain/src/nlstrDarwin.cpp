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

#include <errno.h>
#include "nlstrings.h"
#include <strings.h>
#include "brain.h"
#include <iconv.h>



namespace {
//Convert UTF-16 to UTF-8  
int ConvertToUTF8(const nlchar *inBuf, nlchar *outBuf, int inBufLen,
		   int outBufLen)
{
  iconv_t cd;                     /* conversion descriptor */
  size_t ret_val;                 
  nlchar *in=(nlchar *)inBuf;
  size_t inLen=inBufLen;
  size_t outLen=outBufLen;
  size_t convertLen;

  /* Initiate conversion -- get conversion descriptor */
  if ((cd = iconv_open("UTF-8",  "UTF-16")) == (iconv_t)-1) {
    TRACE(0, _T("iconv_open failed due to: errno=%d\n"), errno);
    return -1;
  }

  ret_val = iconv(cd, (const nlchar **)(&in), &inLen, &outBuf, &outLen);
  if(ret_val == -1) {
    TRACE(0, _T("iconv failed due to: errno=%d inLen=%d %d\n"), errno, inBufLen, outBufLen);
    return -1;
  } else
    convertLen=outBufLen-outLen;

  /* end conversion & get rid of the conversion table */
  if (iconv_close(cd) == -1) {
    TRACE(0, _T("iconv_close failed due to: errno=%d\n"), errno);
    return -1;
  }
  return convertLen;
}
}

nlchar * nlstrcat (nlchar * dest, const nlchar * src)
{
  return strcat(dest, src);
}

nlchar * nlstrncat  (nlchar *dest, const nlchar *src, nluint32 len)
{
  return strncat (dest, src, len);
}

nlchar * nlstrchr (const nlchar * str, nlint32 c)
{
  return strchr (str, c);
}

nlchar *  nlstrrchr (const nlchar * str, nlint32 c)
{
  return strrchr (str, c);
}

nlint32 nlstrcmp (const nlchar * s1, const nlchar * s2)
{
  return strcmp (s1, s2);
}

nlint32 nlstrncmp (const nlchar * s1, const nlchar * s2, nluint32 len)
{
  return strncmp (s1, s2, len);
}

nlint32 nlstricmp (const nlchar * s1, const nlchar * s2)
{
  return strcasecmp (s1, s2);
}

nlint32 nlstrnicmp (const nlchar * s1, const nlchar * s2, nluint32 len)
{
  return strncasecmp (s1, s2, len);
}

nlchar * nlstrcpy (nlchar *dest, const nlchar *src)
{
  return strcpy (dest, src);
}
nlchar * nlstrncpy (nlchar *dest, const nlchar *src, nluint32 len)
{
  return strncpy (dest, src, len);
}

nlchar * nlstrdup (const nlchar * str)
{
  return strdup (str);
}

nluint32 nlstrlen (const nlchar * str)
{
  return strlen(str);
}

nluint32 nlstrblen (const nlchar * str)
{
  return strlen(str);
}

nlchar * nlstrstr (const nlchar * str, const nlchar *substr)
{
  return strstr(str, substr);
}

//Convert nlchar * to char *
//"ascii_buf_size" is the size of buffer "ascii_buf"
bool nlstrtoascii(const nlchar *nl_str, char *ascii_buf, 
		 unsigned int ascii_buf_size)
{
  nluint32 byte_size=nlstrblen(nl_str);
  if(byte_size >= ascii_buf_size)  
    return false; //The buffer "ascii_str" is not big enough

  strncpy_s(ascii_buf, ascii_buf_size, nl_str, _TRUNCATE);

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
bool nlstrplatformcpy(nlchar *dest, const nlchar *source, int &src_len, 
		      int src_byte_len, int des_len) 
{
  if(src_len >= des_len)
    return false;

  if(src_byte_len != src_len) {
    //The source string is in non-ascii format( which has to be UTF-16 
    //currently ), convert it to UTF-8 by taking the first byte from each 
    //character.
    int convertLen=ConvertToUTF8(source, dest, src_byte_len,des_len);
    if(convertLen==-1) 
      return false;
    dest[convertLen]='\0';
    src_len=convertLen;
  } else {
    nlstrncpy(dest, source, src_len);
    dest[src_len]='\0';
  }
  return true;
}


/* Convert a string to lower case. The destLength is the length of*/
/* destination string buffer; the lenght of source string doesn't */
/* include the terminal NULL.                                     */
void nlstrntolow(nlchar *dest, nluint32 destLength,
		 const nlchar *src, nluint32 srcLength)
{
  //NULL source string, do nothing
  if(src == NULL || srcLength <= 0)
    return;

  //NULL dest string, do nothing
  if(dest == NULL || destLength <= 0)
    return;

  //The destination buffer is not big enough, do nothing
  if(destLenght <=srcLength)
    return;

  for(nluint32 i=0; i<srcLenght; i++) {
    if(src[i]<='Z' && src[i]>='A')
      dest[i]='a'+(src[i]-'A');
    else
      dest[i]=src[i];
  }
  dest[srcLength]='\0';
  return;
}
