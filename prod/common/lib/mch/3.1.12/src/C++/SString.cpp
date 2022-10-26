// ***************************************************************
//  SString.cpp               version: 1.0.0  ·  date: 2010-01-10
//  -------------------------------------------------------------
//  simple string class
//  -------------------------------------------------------------
//  Copyright (C) 1999 - 2010 www.madshi.net, All Rights Reserved
// ***************************************************************

#define _CRT_SECURE_NO_DEPRECATE
#define _CRT_NON_CONFORMING_SWPRINTFS

#include "SystemIncludes.h"
#include "Systems.h"
#include "SystemsInternal.h"

// Allocate a bit extra so simple SString manipulations will not always require
// reallocation
const int EXTRA_MEMORY_LENGTH = 25;

// Internal allocation functions
static void *Allocate(size_t length)
{
  void *p = HeapAlloc(GetProcessHeap(), (HEAP_GENERATE_EXCEPTIONS | HEAP_ZERO_MEMORY), length);
  return p;
}

static BOOL Deallocate(void *p)
{
  return ((!p) || (HeapFree(GetProcessHeap(), 0, p)));
}

// The SDK conversion calls have heavy synchronization around code pages.
// Is that the problem?
// MultiByteToWideChar that is hopefully safe/good enough for hooking etc.
// mbSString is multi byte SString
// cbMultiByte is number of bytes of multi byte SString to convert
// wSString is target buffer
// cchWideChar is size of target buffer, if 0 will return required length of conversion
// returns number of Wide characters processed
static int osMultiByteToWideChar(LPCSTR mbSString, int cbMultiByte, LPWSTR wSString, int cchWideChar)
{
  int result = 0;
  if ((mbSString == NULL) || (mbSString == (LPCSTR) wSString))
  {
    SetLastError(ERROR_INVALID_PARAMETER);
    return result;
  }
  if ((wSString == NULL) && (cchWideChar != 0))
  {
    SetLastError(ERROR_INVALID_PARAMETER);
    return result;
  }

  // cbMultiByte is BYTES, not characters
  if (cbMultiByte == -1)
  {
    // length + null terminator
    // To skip null terminator, pass in length alone
    cbMultiByte = (int) strlen(mbSString) + 1;
  }
  int num_wchars = 0;

  while ((cbMultiByte > 0) && ((num_wchars < cchWideChar) || (cchWideChar == 0)))
  {
    if (cchWideChar != 0)
      wSString[num_wchars] = (wchar_t) *mbSString;

    cbMultiByte--;
    num_wchars++;
    mbSString++;
  }
  if (cbMultiByte != 0)
  {
    SetLastError(ERROR_INSUFFICIENT_BUFFER);
    result = 0;
  }
  else
    result = num_wchars;

  return result;
}

static int osWideCharToMultiByte(LPCWSTR wSString, int cchWideChar, LPSTR mbSString, int cbMultiByte)
{
  int result = 0;

  if ((wSString == NULL) || (wSString == (LPCWSTR) mbSString))
  {
    SetLastError(ERROR_INVALID_PARAMETER);
    return result;
  }
  if ((mbSString == NULL) && (cbMultiByte != 0))
  {
    SetLastError(ERROR_INVALID_PARAMETER);
    return result;
  }

  if (cchWideChar == -1)
  {
    cchWideChar = (int) wcslen(wSString) + 1;
  }

  int num_bytes = 0;
  while ((cchWideChar > 0) && ((num_bytes < cbMultiByte) || (cbMultiByte == 0)))
  {
    if (cbMultiByte != 0)
    {
      if (num_bytes + 1 > cbMultiByte)
      {
        // Not enough room
        break;
      }
      mbSString[num_bytes] = ((char *) wSString)[(num_bytes) * 2];
      num_bytes++;
    }
    cchWideChar--;
  }
  if (cchWideChar != 0)
  {
    SetLastError(ERROR_INSUFFICIENT_BUFFER);
    result = 0;
  }
  else
  {
    result = num_bytes;
  }
  return result;
}

// --------------- Static Implementation --------------------

void SString::DeallocateBuffer(wchar_t *buffer)
{
  Deallocate(buffer);
}

int SString::MultiByteToWideChar(LPCSTR mbSString, int cbMultiByte, LPWSTR wSString, int cchWideChar)
{
  return osMultiByteToWideChar(mbSString, cbMultiByte, wSString, cchWideChar);
}

int SString::WideCharToMultiByte(LPCWSTR wSString, int cchWideChar, LPSTR mbSString, int cbMultiByte)
{
  return osWideCharToMultiByte(wSString, cchWideChar, mbSString, cbMultiByte);
}

// --------------- Constructors --------------------

// Constructor that creates a NULL SString.
SString::SString()
{
  mpData = NULL;
  mBufferLength = 0;
  mAllocLength = EXTRA_MEMORY_LENGTH;
  mDeleteBufferOnDestruct = TRUE;
  mpEndPos = mpData;
  mpAnsiData = NULL;
}

// Constructor that creates a SString from a constant wide char array.
SString::SString(LPCWSTR SString, size_t count)
{
  // Calculate total amount to allocate.
  mAllocLength = EXTRA_MEMORY_LENGTH;

  // Support for passing a non-null terminated string with character count
  size_t length = count;
  if ((length == 0) && (SString))
    length = wcslen(SString);
  // size_t length = _tcslen(SString);

  mBufferLength = length + 1 + mAllocLength;
  mpData = (wchar_t *) Allocate(mBufferLength * sizeof(wchar_t));

  if (length)
    wcsncpy(mpData, SString, length);
  mpData[length] = L'\0';
  // wcscpy(mpData , SString);

  mpEndPos = mpData + length;
  mDeleteBufferOnDestruct = TRUE;
  mpAnsiData = NULL;
}

#ifdef UNICODE
  // Constructor that creates a SString from a constant ANSI char array.
  SString::SString(LPCSTR SString, size_t count)
  {
    // Calculate total amount to allocate.
    mAllocLength = EXTRA_MEMORY_LENGTH;

    // Support for passing a non-null terminated string with character count
    size_t length = count;
    if ((length == 0) && (SString))
      length = strlen(SString);

    // character count + NULL character + extra
    mBufferLength = length + 1 + mAllocLength;
    mpData = (wchar_t *) Allocate(mBufferLength * sizeof(wchar_t));

    if (length)
      osMultiByteToWideChar(SString, (int) length, mpData, (int) length);
    mpData[length] = L'\0';

    mpEndPos = mpData + length;
    mDeleteBufferOnDestruct = TRUE;
    mpAnsiData = NULL;
  }
#endif // UNICODE

// Constructor that creates a class that may not delete the
// buffer when the destructor is called.
SString::SString(bool bDeleteBufferOnDestruct)
{
  mpData = NULL;
  mBufferLength = 0;
  mAllocLength = EXTRA_MEMORY_LENGTH;
  mDeleteBufferOnDestruct = bDeleteBufferOnDestruct;
  mpEndPos = mpData;
  mpAnsiData = NULL;
}

// Copy constructor.
SString::SString(const SString& SString)
{
  mAllocLength = SString.mAllocLength;
  if (SString.mpData == NULL)
  {
    mpData = NULL;
    mpEndPos = mpData;
  }
  else
  {
    size_t length = wcslen(SString.mpData);
    mBufferLength = length + 1 + mAllocLength;
    mpData = (wchar_t *) Allocate(mBufferLength * sizeof(wchar_t));
    wcscpy(mpData , SString.mpData);
    mAllocLength = SString.mAllocLength;
    mDeleteBufferOnDestruct = TRUE;
    mpEndPos = mpData + length;
  }
  mpAnsiData = NULL;
}

// Destructor.
SString::~SString()
{
  if (mDeleteBufferOnDestruct)
  {
    Deallocate(mpData);
    mpData = NULL;
    mpEndPos = NULL;
    mBufferLength = 0;
  }
  if (mpAnsiData)
  {
    Deallocate(mpAnsiData);
    mpAnsiData = NULL;
  }
}

// --------------- Operators --------------------

// Equality operators. All comparisons are case sensitive.

bool operator == (const SString& s, const SString& t)
{
  return wcscmp(s.mpData, t.mpData) == 0;
}

bool operator == (const SString& s, const wchar_t *t)
{
  return wcscmp(s, t) == 0;
}

// Inequality operator. 
bool operator != (const SString& s , const SString& t)
{
  return wcscmp(s.mpData, t.mpData) != 0;
}

bool operator != (const SString& s, const wchar_t *t)
{
  return wcscmp(s, t) != 0;
}

// Assignment operator.

SString& SString::operator = (const SString& t)
{
  mAllocLength = t.mAllocLength;
  // If t is empty, fix up this.
  if (t.mBufferLength == 0)
  {
    if (mBufferLength != 0)
    {
      mpData[0] = L'\0';
      mpEndPos = mpData;
    }
  }
  else
  {
    size_t length = t.Length();

    // Is the new SString larger than buffer already allocated?
    if (length >= mBufferLength)
    {
      if (mpData != NULL)
        Deallocate(mpData);

      mBufferLength = length + 1 + mAllocLength;
      mpData = (wchar_t *) Allocate(mBufferLength * sizeof(wchar_t));
    }
    wcscpy(mpData, t.mpData);
    mpEndPos = mpData + length;
  }
  return *this;
}

SString& SString::operator = (LPCWSTR SString)
{
  // If SString is empty, fix up this.
  if ((SString == NULL) || (SString[0] == L'\0'))
  {
    if (mBufferLength != 0)
    {
      mpData[0] = L'\0';
      mpEndPos = mpData;
    }
  }
  else
  {
    size_t length = wcslen(SString);

    // Is the new string larger than buffer already allocated?
    if (length >= mBufferLength)
    {
      if (mpData != NULL)
        Deallocate(mpData);

      mBufferLength = length + 1 + mAllocLength;
      mpData = (wchar_t *) Allocate(mBufferLength * sizeof(wchar_t));
    }

    wcscpy(mpData, SString);
    mpEndPos = mpData + length;
  }
  return *this;
}

SString& SString::operator = (const wchar_t c)
{
  if (mBufferLength == 0)
  {
    mBufferLength = mAllocLength + 1;
    mpData = (wchar_t *) Allocate(mBufferLength * sizeof(wchar_t));
  }
  // Replace the data.
  mpData[0] = c;
  mpData[1] = L'\0';
  mpEndPos = mpData + 1;
  return *this;
}

#ifdef UNICODE
  SString& SString::operator = (const char *SString)
  {
    // If SString is empty, fix up this.
    if ((SString == NULL) || (SString[0] == '\0'))
    {
      if (mBufferLength != 0)
      {
        mpData[0] = L'\0';
        mpEndPos = mpData;
      }
    }
    else
    {
      size_t length = strlen(SString);

      // Is the new SString larger than buffer already allocated?
      if (length >= mBufferLength)
      {
        if (mpData != NULL)
        {
          Deallocate(mpData);
        }

        mBufferLength = length + 1 + mAllocLength;
        mpData = (wchar_t *) Allocate(mBufferLength * sizeof(wchar_t));
      }

      int d = osMultiByteToWideChar(SString, -1, mpData, (int) length + 1);
      if (d == 0)
      {
        // Process failure
        Deallocate(mpData);
        mpData = NULL;
        length = 0;
      }
      mpEndPos = mpData + length;
    }
    return *this;
  }
#endif  // UNICODE

// cast operator.
SString::operator const wchar_t * () const
{
  return (const wchar_t *) mpData;
}

// cast operator
SString::operator char * ()
{
  if (mpAnsiData)
  {
    Deallocate(mpAnsiData);
    mpAnsiData = NULL;
  }
  size_t length = Length() + 1;
  mpAnsiData = (char *) Allocate(length * sizeof(char));
  if (osWideCharToMultiByte(mpData, -1, mpAnsiData, (int) length) != (int) length)
  {
    Deallocate(mpAnsiData);
    mpAnsiData = NULL;
  }
  return mpAnsiData;
}

SString operator + (const SString& s, const SString& t)
{
  if (s.Length() == 0)
  {
    return t;
  }
  else if (t.Length() == 0)
  {
    return s;
  }

  SString szNew;

  if (s.mAllocLength > t.mAllocLength)
  {
    szNew.mAllocLength = s.mAllocLength;
  }
  else
  {
    szNew.mAllocLength = t.mAllocLength;
  }

  size_t iSLen = s.Length();
  size_t iTLen = t.Length();

  szNew.mBufferLength = iSLen + iTLen + 1 + szNew.mAllocLength;
  szNew.mpData = (wchar_t *) Allocate(szNew.mBufferLength * sizeof(wchar_t));

  wcscpy(szNew.mpData, s.mpData);
  szNew.mpEndPos = szNew.mpData + iSLen;

  wcscpy(szNew.mpEndPos, t.mpData);
  szNew.mpEndPos = szNew.mpEndPos + iTLen;

  return szNew;
}

// Append a string to an SString without changing the SString
SString operator + (const SString& s, const wchar_t *s1)
{
  if (s1 == NULL)
  {
    return s;
  }
  return (s + SString(s1));
}

SString& SString::operator += (const SString& s)
{
  if (s.Length() == 0)
  {
    return *this;
  }

  if (mpData == NULL)
  {
    *this = s;
    return *this;
  }

  size_t iSLen = s.Length();
  size_t iThisLen = Length();

  if (mBufferLength > iSLen + iThisLen)
  {
    wcscpy(mpEndPos, s.mpData);
    mpEndPos += iSLen;
  }
  else
  {
    mBufferLength = iThisLen + iSLen + 1 + mAllocLength;

    wchar_t *szBuff = (wchar_t *) Allocate(mBufferLength * sizeof(wchar_t));

    wcscpy(szBuff , mpData);
    mpEndPos = szBuff + iThisLen;

    wcscpy(mpEndPos, s.mpData);
    mpEndPos += iSLen;

    Deallocate(mpData);

    mpData = szBuff;
  }
  return *this;
}

SString& SString::operator += (const wchar_t * s)
{
  if (s == NULL)
  {
    return *this;
  }

  if (mpData == NULL)
  {
    *this = s;
    return *this;
  }

  size_t iSLen = wcslen(s);
  size_t iThisLen = Length();

  if (mBufferLength > iSLen + iThisLen)
  {
    wcscpy(mpEndPos, s);
    mpEndPos += iSLen;
  }
  else
  {
    mBufferLength = iThisLen + iSLen + 1 + mAllocLength;

    wchar_t * szBuff = (wchar_t *) Allocate(mBufferLength * sizeof(wchar_t));

    wcscpy(szBuff, mpData);
    mpEndPos = szBuff + iThisLen;
    wcscpy(mpEndPos, s);
    mpEndPos += iSLen;

    Deallocate(mpData);
    mpData = szBuff;

    mpData = szBuff;
  }
  return *this;
}

SString& SString::operator += (const wchar_t c)
{
  if (mpData == NULL)
  {
    mBufferLength = mAllocLength;
    mpData = (wchar_t *) Allocate(mBufferLength * sizeof(wchar_t));
    mpData[0] = c;
    mpData[1] = L'\0';
    mpEndPos = mpData + 1;
  }
  else
  {
    size_t iLen = Length();

    if (mBufferLength - 1 > iLen)
    {
      mpData[iLen] = c;
      mpData[iLen + 1] = L'\0';
      mpEndPos++;
    }
    else
    {
      mBufferLength = iLen + 2 + mAllocLength;

      wchar_t *szBuff = (wchar_t *) Allocate(mBufferLength * sizeof(wchar_t));

      wcscpy(szBuff, mpData);
      szBuff[iLen] = c;
      szBuff[iLen + 1] = L'\0';
      Deallocate(mpData);
      mpData = szBuff;
      mpEndPos = mpData + 1;
    }
  }
  return *this;
}

#ifdef UNICODE
  SString& SString::operator += (const char * SString)
  {
    if (SString == NULL)
    {
      return *this;
    }

    if (mpData == NULL)
    {
      *this = SString;
      return *this;
    }

    size_t length = strlen(SString);
    size_t thisLength = Length();

    if (mBufferLength <= length + thisLength)
    {
      mBufferLength = thisLength + length + 1 + mAllocLength;

      wchar_t *buffer = (wchar_t *) Allocate(mBufferLength * sizeof(wchar_t));

      wcscpy(buffer, mpData);
      Deallocate(mpData);
      mpData = buffer;
      mpEndPos = mpData + thisLength;
    }

    int d = osMultiByteToWideChar(SString, -1, mpEndPos, (int) length + 1);
    if (d == 0)
    {
      // Process failure
      Deallocate(mpData);
      mpData = NULL;
      length = 0;
    }

    mpEndPos += length;
    return *this;
  }
#endif

wchar_t & SString::operator [] (int iIndex)
{
  return mpData[iIndex];
}

// --------------------- Properties --------------------------

// Length function.

size_t SString::Length() const
{
  if (mpData == NULL)
  {
    return 0;
  }
  return mpEndPos - mpData;
}

// Is empty function.
bool SString::IsEmpty()
{
  return mpEndPos == mpData;
}

// ---------------------- Implementation ----------------------

void SString::ToLower()
{
  if (mpData != NULL)
  {
    _wcslwr(mpData);
  }
}

void SString::TrimTrailingWhiteSpace(void)
{
  if (mpData != NULL)
  {
    wchar_t * pEnd = mpEndPos - 1;
    while (iswspace(*pEnd))
    {
      pEnd--;
      if (pEnd == mpData)
      {
        pEnd--;
        break;
      }
    }
    pEnd++;
    *pEnd = L'\0';
    mpEndPos = pEnd;
  }
}

// Clears any data.
void SString::Empty(void)
{
  if (mpData != NULL)
  {
    Deallocate(mpData);
  }
  mpData = NULL;
  mBufferLength = 0;
  mpEndPos = mpData;
}

// Safely puts a NULL at the beginning of the SString.
void SString::NullString(void)
{
  if (mpData != NULL)
  {
    mpData[0] = L'\0';
  }
  mpEndPos = mpData;
}

// A printf style filler.
int SString::Format(const wchar_t * szFmt, ...)
{
  if (mBufferLength < 1024)
  {
    wchar_t * szTempData = (wchar_t *) Allocate(1024 * sizeof(wchar_t));

    if (mpData != NULL)
    {
      Deallocate(mpData);
    }
    mpData = szTempData;
    mBufferLength = 1024;
  }

  va_list marker;

  va_start(marker, szFmt);

  int iRet = vswprintf_s(mpData, mBufferLength, szFmt, marker);

  va_end(marker);

  // Send end position
  size_t length = wcslen(mpData);
  mpEndPos = &mpData[length];
  return iRet;
}

void SString::SetBufferLen(size_t newSize)
{
  if (mBufferLength < newSize)
  {
    wchar_t *buffer = (wchar_t *) Allocate(newSize * sizeof(wchar_t));
    wcsncpy(buffer, mpData, mBufferLength * sizeof(wchar_t));
    mBufferLength = newSize;

    size_t l = mpEndPos - mpData;

    if (mpData != NULL)
      Deallocate(mpData);

    mpData = buffer;
    mpEndPos = (wchar_t *) &mpData[l];
  }
}

wchar_t *SString::GetBuffer(void)
{
  return mpData;
}

int SString::SetMinumumAllocationLength(int iNewLen)
{
  int iRet = mAllocLength;
  mAllocLength = iNewLen;
  return iRet;
}

bool SString::DoNotDeleteBuffer(bool bDeleteBuffer)
{
  bool bRet = mDeleteBufferOnDestruct;
  mDeleteBufferOnDestruct = bDeleteBuffer;
  return bRet;
}

int SString::PosStr(LPCWSTR subStr, int fromPos, int toPos)
{
  if (fromPos >= (int) this->Length())
    return -1;

  wchar_t *pStart = wcsstr((wchar_t *) &(*this)[fromPos], subStr);
  if (pStart == NULL)
    return -1;
  int pos = (int) (pStart - mpData);
  if (pos > toPos)
    return -1;
  return pos;
}

// delimeter is character between each SString.
wchar_t *SString::SubStr(int index, wchar_t delimeter)
{
  SString result(false);
  if ((Length() > 1) && (index != -1))
  {
    int i;
    int end = -1;
    int start = -1;

    for (i = 0; i < index + 1; i++)
    {
      start = end + 1;
      if (start > (int) this->Length())
        break;   // ran out of space to look

      wchar_t *subStrEnd = wcschr((wchar_t *) &(mpData[start]), delimeter);
      if (subStrEnd == NULL)
      {
        // Didn't find a delimeter, so we go to end
        end = (int) this->Length();
      }
      else
        end = (int) (subStrEnd - this->GetBuffer());

    }
    if ((i == index + 1) && (end != start))
    {
      result.SetBufferLen(end - start + 1);
      wcsncpy((wchar_t *) &(result)[0], (wchar_t *) &(mpData[start]), end - start);
    }
  }
  return result.GetBuffer();
}

int SString::SubStrCount(wchar_t delimeter)
{
  int result = 0;
  if (Length() > 0)
  {
    result = 1;
    for (int i = 0; i < (int) this->Length(); i++)
    {
      if (mpData[i] == delimeter)
        result++;
    }
  }
  return result;
}

// **************************************************************
// Replaces all occurrences of a SString with another SString
// Returns true if at least one occurrence replaced
// **************************************************************
#pragma warning(disable: 4127)
bool SString::Replace(LPCWSTR replaceThis, LPCWSTR withThis)
{
  if (mpData == NULL)
  {
    return false;
  }

  bool result = false;
  while (true)
  {
    int replacePosition = PosStr(replaceThis);
    if (replacePosition == -1)
      break;

    size_t replaceLen = wcslen(replaceThis);
    size_t withLen = wcslen(withThis);
    size_t dLen = withLen - replaceLen;

    size_t newSize = this->Length() + dLen + 1;
    wchar_t *szBuff = (wchar_t *) Allocate(newSize * sizeof(wchar_t));

    // Copy segment before replace SString 
    wcsncpy(szBuff, mpData, replacePosition);

    // Copy withThis to Replace Position
    wcsncpy(&szBuff[replacePosition], withThis, withLen);

    size_t lengthToCopy = Length() - replacePosition - replaceLen;
    if (lengthToCopy > 0)
      wcsncpy(&szBuff[replacePosition + withLen], &mpData[replacePosition + replaceLen], lengthToCopy);

    szBuff[replacePosition + withLen + lengthToCopy] = L'\0';

    mBufferLength = newSize;
    Deallocate(mpData);
    mpData = szBuff;

    size_t length = wcslen(mpData);
    mpEndPos = &mpData[length];

    result = true;
  }
  return result;
}
