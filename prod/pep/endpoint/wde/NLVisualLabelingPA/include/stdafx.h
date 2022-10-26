// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"


// TODO: reference additional headers your program requires here
// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the PA_LABELING_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// PA_LABELING_API functions as being imported from a DLL, whereas this DLL sees symbols
#ifdef PA_LABELING_EXPORTS
#define PA_LABELING_API __declspec(dllexport)
#else
#define PA_LABELING_API __declspec(dllimport)
#endif
// Windows Header Files:
#include <windows.h>


#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // some CString constructors will be explicit

#include <atlbase.h>
#include <atlstr.h>

extern HMODULE g_hModule;

#include "PABase.h"
using namespace PABase;
// TODO: reference additional headers your program requires here

//#include "ylib\log.h"
#include <string>
#include <vector>
using std::wstring;
using std::pair;
using std::vector;

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>


#include "NLVisualLabelingPA.h"

#if defined(MSO2K3)

	//#include "../import/2k3/msaddndr.tlh"

	#include "../import/2k3/mso.tlh"
	using namespace Office;

	#include "../import/2k3/vbe6ext.tlh"
	using namespace VBE6;

	#include "../import/2k3/msword.tlh"
	using namespace Word;

	#include "../import/2k3/excel.tlh"
	using namespace Excel;

	#include "../import/2k3/msppt.tlh"
	using namespace PPT;

#elif defined(MSO2K7)

	//#include "../import/2k7/msaddndr.tlh"

	#include "../import/2k7/mso.tlh"
	using namespace Office;

	#include "../import/2k7/vbe6ext.tlh"
	using namespace VBE6;

	#include "../import/2k7/msword.tlh"
	using namespace Word;

	#include "../import/2k7/excel.tlh"
	using namespace Excel;

	#include "../import/2k7/msppt.tlh"
	using namespace PPT;

#else
	//Error NOSUPPORTED_VERSION
#endif


