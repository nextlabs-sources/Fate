// ***************************************************************
//  StringConvert.h           version: 1.0.0  ·  date: 2010-01-10
//  -------------------------------------------------------------
//  facilitates ANSI/WIDE strings from ANSI code
//  -------------------------------------------------------------
//  Copyright (C) 1999 - 2010 www.madshi.net, All Rights Reserved
// ***************************************************************

#ifndef _STRINGCONVERT_H
#define _STRINGCONVERT_H

class toChar
{
   private:
      BOOL mAutoDelete;
      LPSTR mpBuffer;

   public:

      toChar(LPCWSTR text, bool autoDelete = true) : mAutoDelete(autoDelete)
      {
         ASSERT(text);
         size_t len = wcslen(text)+1;
         mpBuffer = new CHAR[len];
         wcstombs(mpBuffer, text, len);
      }

      toChar(LPCSTR text, bool autoDelete = true) : mAutoDelete(autoDelete)
      {
         ASSERT(text);
         size_t len = strlen(text) + 1;
         mpBuffer = new CHAR[len];
         strcpy(mpBuffer, text);
      }

      ~toChar()
      {
         if(mAutoDelete)
         {
            ASSERT(mpBuffer);
            delete[] mpBuffer;
         }
      }

      LPSTR GetBuffer()
      {
         return (LPSTR)mpBuffer;
      }

      operator LPSTR()
      {
         ASSERT(mpBuffer);
         return (LPSTR)mpBuffer;
      }

      operator LPCSTR()
      {
         ASSERT(mpBuffer);
         return (LPCSTR)mpBuffer;
      }
};


class toWide
{
   private:
      BOOL mAutoDelete;
      LPWSTR mpBuffer;

   public:
      toWide(LPCWSTR text, bool autoDelete = true) : mAutoDelete(autoDelete)
      {
         ASSERT(text);
         size_t len = wcslen(text)+1;
         mpBuffer = new WCHAR[len];
         wcscpy(mpBuffer, text);
      }

      toWide(LPCSTR text, bool autoDelete = true) : mAutoDelete(autoDelete)
      {
         ASSERT(text);
         size_t len = strlen(text)+1;
         mpBuffer = new WCHAR[len];
         mbstowcs(mpBuffer, text, len);
      }

      ~toWide()
      {
         if( mAutoDelete )
         {
            ASSERT(mpBuffer);
            delete[] mpBuffer;
         }
      }

      LPWSTR GetBuffer()
      {
         return (LPWSTR)mpBuffer;
      }

      operator LPWSTR()
      {
         ASSERT(mpBuffer);
         return (LPWSTR)mpBuffer;
      }

      operator LPCWSTR()
      {
         ASSERT(mpBuffer);
         return (LPCWSTR)mpBuffer;
      }

};


/*
//=============================================================================
// class _totchar
// This class converts a TCHAR string to a new TCHAR string.
// Memory is allocated/deallocated using new/delete
//=============================================================================

class _totchar {
private:
  BOOL m_bAutoDelete;
  LPTSTR m_tszBuffer;

public:
  _totchar(LPCSTR szText, BOOL bAutoDelete = TRUE)
  {
    m_bAutoDelete = bAutoDelete;
    _ASSERTE(szText);
    int nLen = strlen(szText) + 1;
    m_tszBuffer = new TCHAR [nLen];
    #if defined(UNICODE) || defined(_UNICODE)
    mbstowcs(m_tszBuffer, szText, nLen);
    #else
    strcpy(m_tszBuffer, szText);
    #endif
  }
  _totchar(LPCWSTR wszText, BOOL bAutoDelete = TRUE)
  {
    m_bAutoDelete = bAutoDelete;
    _ASSERTE(wszText);
    int nLen = wcslen(wszText) + 1;
    m_tszBuffer = new TCHAR [nLen];
    #if defined(UNICODE) || defined(_UNICODE)
    wcscpy(m_tszBuffer, wszText);
    #else
    wcstombs(m_tszBuffer, wszText, nLen);
    #endif
  }
  ~_totchar()
  {
    if (m_bAutoDelete) {
      _ASSERTE(m_tszBuffer);
      delete [] m_tszBuffer;
    }
  }
  operator LPTSTR()
  {
    _ASSERTE(m_tszBuffer);
    return (LPTSTR) m_tszBuffer;
  }
  operator LPCTSTR()
  {
    _ASSERTE(m_tszBuffer);
    return (LPCTSTR) m_tszBuffer;
  }
};



  Following can be useful when cracking COM

//=============================================================================
// class _cochar
// This class converts either WCHAR or CHAR string to a new CHAR string.
// Memory is allocated/deallocated using CoTaskMemAlloc/CoTaskMemFree.
//=============================================================================

class _cochar {
private:
  BOOL m_bAutoDelete;
  LPSTR m_szBuffer;

public:
  _cochar(LPCWSTR wszText, BOOL bAutoDelete = TRUE)
  {
    m_bAutoDelete = bAutoDelete;
    _ASSERTE(wszText);
    int nLen = wcslen(wszText)+1;
    m_szBuffer = (LPSTR)::CoTaskMemAlloc(nLen * sizeof(CHAR));
    wcstombs(m_szBuffer, wszText, nLen);
  }
  _cochar(LPCSTR szText, BOOL bAutoDelete = TRUE)
  {
    m_bAutoDelete = bAutoDelete;
    _ASSERTE(szText);
    int nLen = strlen(szText) + 1;
    m_szBuffer = (LPSTR)::CoTaskMemAlloc(nLen * sizeof(CHAR));
    strcpy(m_szBuffer, szText);
  }
  ~_cochar()
  {
    if (m_bAutoDelete)
      ::CoTaskMemFree(m_szBuffer);
  }
  operator LPSTR()
  {
    return (LPSTR)m_szBuffer;
  }
  operator LPCSTR()
  {
    return (LPCSTR)m_szBuffer;
  }
};

//=============================================================================
// class _towchar
// This class converts either WCHAR or CHAR string to a new WCHAR string.
// Memory is allocated/deallocated using CoTaskMemAlloc/CoTaskMemFree
//=============================================================================

class _cowchar {
private:
  BOOL m_bAutoDelete;
  LPWSTR m_wszBuffer;

public:
  _cowchar(LPCWSTR wszText, BOOL bAutoDelete = TRUE)
  {
    m_bAutoDelete = bAutoDelete;
    _ASSERTE(wszText);
    int nLen = wcslen(wszText)+1;
    m_wszBuffer = (LPWSTR)::CoTaskMemAlloc(nLen * sizeof(WCHAR));
    wcscpy(m_wszBuffer, wszText);
  }
  _cowchar(LPCSTR szText, BOOL bAutoDelete = TRUE)
  {
    m_bAutoDelete = bAutoDelete;
    _ASSERTE(szText);
    int nLen = strlen(szText) + 1;
    m_wszBuffer = (LPWSTR)::CoTaskMemAlloc(nLen * sizeof (WCHAR));
    mbstowcs(m_wszBuffer, szText, nLen);
  }
  ~_cowchar()
  {
    if (m_bAutoDelete)
      ::CoTaskMemFree(m_wszBuffer);
  }
  operator LPWSTR()
  {
    return (LPWSTR)m_wszBuffer;
  }
  operator LPCWSTR()
  {
    return (LPCWSTR)m_wszBuffer;
  }
};

//=============================================================================
// class _cotchar
// This class converts a TCHAR string to a new TCHAR string.
// Memory is allocated/deallocated using CoTaskMemAlloc/CoTaskMemFree
//=============================================================================

class _cotchar {
private:
  BOOL m_bAutoDelete;
  LPTSTR m_tszBuffer;

public:
  _cotchar(LPCSTR szText, BOOL bAutoDelete = TRUE)
  {
    m_bAutoDelete = bAutoDelete;
    _ASSERTE(szText);
    int nLen = strlen(szText) + 1;
    m_tszBuffer = (LPTSTR)::CoTaskMemAlloc(nLen * sizeof(TCHAR));
    #if defined(UNICODE) || defined(_UNICODE)
    mbstowcs(m_tszBuffer, szText, nLen);
    #else
    strcpy(m_tszBuffer, szText);
    #endif
  }
  _cotchar(LPCWSTR wszText, BOOL bAutoDelete = TRUE)
  {
    m_bAutoDelete = bAutoDelete;
    _ASSERTE(wszText);
    int nLen = wcslen(wszText) + 1;
    m_tszBuffer = (LPTSTR)::CoTaskMemAlloc(nLen * sizeof(TCHAR));
    #if defined(UNICODE) || defined(_UNICODE)
    wcscpy(m_tszBuffer, wszText);
    #else
    wcstombs(m_tszBuffer, wszText, nLen);
    #endif
  }
  ~_cotchar()
  {
    if (m_bAutoDelete)
      ::CoTaskMemFree(m_tszBuffer);
  }
  operator LPTSTR()
  {
    return (LPTSTR) m_tszBuffer;
  }
  operator LPCTSTR()
  {
    return (LPCTSTR) m_tszBuffer;
  }
};
*/
#endif
