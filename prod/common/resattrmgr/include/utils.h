#ifndef __TAGLIB_UTILS_H__
#define __TAGLIB_UTILS_H__

#ifndef FALSE
#define FALSE	0
#endif

#ifndef TRUE
#define TRUE	1
#endif

#ifndef NULL
#define NULL 	0
#endif

typedef int BOOL;
#include "tag.h"
#include <string>
namespace NSLinuxTagLib
{
	class Utils
	{
		static const char TEMPFILE_TEMPLATE[];
	public:
		/*
		Description:
			Detect if a file is PDF or not.
		Parameters:
			pchFileName: The file name/full path of a file.
		Return:
			TRUE: If it is PDF document.
			Otherwise, it return FALSE		
		*/
		static BOOL IsPDF(const char*pchFileName);
		/*
		Description:
			Detect if a file is OLE document(Compound document) or not. For example, .doc,.xls.
		Parameters:
			pchFileName: The file name/full path of a file.
		Return:
			TRUE: If it is OLE document.
			Otherwise, it return FALSE		
		*/
		static BOOL IsOLE(const char*pchFileName);
		/*
		Description:
			Detect a file is Office Open XML format or not. For example, .docx,domx.
		Parameters:
			pchFileName: The file name/full path of a file.
		Return:
			TRUE: If it is Office Open XML format.
			Otherwise, it return FALSE		
		*/
		static BOOL IsOOXML(const char*pchFileName);
		/*
		Description:
			Create a concrete object which derived from FileTagging.Current it support PDF,OLE and OOXML.
		Parameters:
			pchFileName: The file name/full path of a file.
		Return:
			A pointer to the FileTagging object will be returned if the file type is supported.
			Otherwise, NULL will be returned.
		*/
		static FileTagging* CreateFileTagging(const char*pchFileName);
		/*
		*/
		static BOOL GetTempFile(const char*pchFileName,std::string& strTempFile);
	};	
	
}


#endif // __TAGLIB_UTILS_H__
