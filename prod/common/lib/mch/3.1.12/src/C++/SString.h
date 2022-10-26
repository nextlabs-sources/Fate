// ***************************************************************
//  SString.h                 version: 1.0.0  ·  date: 2010-01-10
//  -------------------------------------------------------------
//  simple string class
//  -------------------------------------------------------------
//  Copyright (C) 1999 - 2010 www.madshi.net, All Rights Reserved
// ***************************************************************

#ifndef _SSTRING_H
#define _SSTRING_H

// A simple String class.
class SYSTEMS_API SString
{
  public:
    // Constructor that creates a NULL SString
    SString(void);
    // Constructor that creates a SString from a constant char array
    // if not NULL terminated, count indicates number of characters to copy
    SString(const wchar_t *string, size_t count = 0);

    #ifdef UNICODE
      // Constructor that creates a SString from a constant ANSI char array
      SString(LPCSTR SString, size_t count = 0);
    #endif

    // Constructor that creates a class that does not delete the
    // buffer when the constructor is called
    SString(bool bDeleteBufferOnDestruct);
    // Copy constructor
    SString(const SString& cInString);
    // Destructor
    ~SString(void);

    // Equality operator. All compares are case insensitive
    friend bool operator == (const SString& s, const SString& t);
    friend bool operator == (const SString& s, const wchar_t *SString);
    // Inequality operator. All compares are case insensitive
    friend bool operator != (const SString& s, const SString& t);
    friend bool operator != (const SString& s, const wchar_t *SString);

    // Assignement operator
    SString& operator = (const SString& t);
    SString& operator = (const wchar_t *string);
    SString& operator = (const wchar_t cChar);

    #ifdef UNICODE
      SString& operator = (LPCSTR ansiString);
    #endif 

    SString& operator += (const SString& s);
    SString& operator += (const wchar_t *s);
    SString& operator += (const wchar_t c);

    #ifdef UNICODE
      SString& operator += (const char * ansiStr);
    #endif

    // Length function
    size_t Length(void) const;

    // Operator to allow us to use the SString in API functions
    operator const wchar_t *(void) const;
    // Operator to return an ANSI SString
    operator char *(void);

    // Concatination operator
    friend SString operator +(const SString& s, const SString& t);
    friend SString operator +(const SString& s, wchar_t *s1);

    // Array operator
    wchar_t& operator [] (int index);

    // A check for empty SStrings
    bool IsEmpty(void);

    // Forces the SString to lowercase
    void ToLower(void);

    // Trims any trialing whitespace
    void TrimTrailingWhiteSpace(void);

    // Clears any data
    void Empty(void);

    // Safely puts a NULL at the beginning of the SString
    void NullString(void);

    // A printf style filler.
    int Format(const wchar_t * szFmt, ...);

    // Forces the buffer to a new length in characters
    // If the buffer is larger, the larger value is kept
    void SetBufferLen(size_t newSize);

    // Get's the raw data buffer so API functions can fill it
    // Make sure to set the size with SetBufferLen first!
    wchar_t *GetBuffer(void);

    // Allows you to set the minimum allocation length
    // This is convenient if you are going to be adding a lot to this SString
    int SetMinumumAllocationLength(int iNewLen);

    // Tells the instance NOT to delete the underlying buffer
    // You must call GetBuffer and call delete [] on it yourself!!
    bool DoNotDeleteBuffer(bool bDeleteBuffer);

    // Stuff added for DisAssembly
    bool Replace(LPCWSTR replaceThis, LPCWSTR withThis);
    int PosStr(LPCWSTR subStr, int fromPos = 0, int toPos = 32768);
    LPWSTR SubStr(int index, wchar_t delimeter = L'|');
    int SubStrCount(wchar_t delimeter = L'|');

    static int MultiByteToWideChar(LPCSTR mbSString, int cbMultiByte, LPWSTR wSString, int cchWideChar);
    static int WideCharToMultiByte(LPCWSTR wSString, int cchWideChar, LPSTR mbSString, int cbMultiByte);

    // Deallocates the buffer returned by GetBuffer
    // If DoNotDeleteBuffer is used, this must be called to delete the buffer
    static void DeallocateBuffer(wchar_t *buffer);

  private:
    // The data buffer
    wchar_t *mpData;
    char *mpAnsiData;
    // The pointer to the end of the current SString
    wchar_t *mpEndPos;
    // The total amount of memory
    size_t mBufferLength;
    // The extra allocation length
    int mAllocLength;
    // If true, the buffer is not deleted in the destructor
    // This property is not copied over when copying SStrings
    // You must specifically call DoNotDeleteBuffer or construct the SString
    // using the bool param constructor
    bool mDeleteBufferOnDestruct;
};

#endif