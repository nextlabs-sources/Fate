// All sources, binaries and HTML pages (C) copyright 2004 by Blue Jungle Inc., 
// Redwood City CA, Ownership remains with Blue Jungle Inc, 
// All rights reserved worldwide. 

#ifndef _globals_h_
#define _globals_h_

#include <string.h>
#include <vector>
#include <list>
#include <map>

// C RunTime Header Files
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>

// Windows Header Files:
#include <windows.h>

//define tstring
typedef std::basic_string<TCHAR, 
        std::char_traits<TCHAR>, 
        std::allocator<TCHAR> > 
        tstring;

typedef std::vector<tstring> StringVector;
typedef std::vector<void*> PtrVector;
typedef std::list<void*> PtrList;
typedef std::map<void*, void*> PtrToPtrMap;


#endif