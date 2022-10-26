/*
 * Common.h 
 * Author: Helen Friedland
 * All sources, binaries and HTML pages (C) copyright 2004 by Blue Jungle Inc., 
 * Redwood City CA, Ownership remains with Blue Jungle Inc, 
 * All rights reserved worldwide. 
 */


#if !defined(_COMMON_H_)
#define _COMMON_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// 'identifier' : identifier was truncated to 'number' 
// characters in the debug information
#pragma warning(disable:4786)
#pragma warning(disable : 4244) /* Ignore warnings for conversion from '__w64 int' to 'long', possible loss of data */
#pragma warning(disable : 4267) /* Ignore warnings for conversion from 'size_t' to 'long', possible loss of data */
#pragma warning(disable : 4231) 
#pragma warning(disable : 4251)
#pragma warning(disable : 4996)
#pragma warning(disable : 4311)


//---------------------------------------------------------------------------
//
// Includes
//
//---------------------------------------------------------------------------

#ifndef _AFXDLL
#include <windows.h>
#endif

#include <tchar.h>

#endif // !defined(_COMMON_H_)

//--------------------- End of the file -------------------------------------
