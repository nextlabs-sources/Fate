#ifndef __TAG_OLE_H__
#define __TAG_OLE_H__ 
#include "resattrlib.h"
#include "utils.h"
namespace NSLinuxTagLib
{
	//This is a totally new class. It will not re-use pdfattr.h/.cpp.
	class PDFFileTagging:public FileTagging
	{
		static const char PDF_EXT[];
		BOOL ModifyTags(void*/*pluginData*/,const char *pchFileName, ResourceAttributes *pAttr,bool bRemove);
	public:
		PDFFileTagging();
		static BOOL IsPDF(const char*pchFileName);
		BOOL 		GetTags(void*/*pluginData*/,const char *filename, ResourceAttributes *attrs);
		BOOL 		SetTags(void*/*pluginData*/,const char *filename, ResourceAttributes *attrs);
		BOOL 		RemoveTags(void*/*pluginData*/,const char *filename, ResourceAttributes *attrs);
	};
	
	class PDFFileTagging2:public FileTagging
	{
	public:
		PDFFileTagging2();
		static BOOL IsPDF(const char*pchFileName);
		BOOL 		GetTags(void* /*pluginData*/,const char *filename, ResourceAttributes *attrs);
		BOOL 		SetTags(void* /*pluginData*/,const char *filename, ResourceAttributes *attrs);
		BOOL 		RemoveTags(void* /*pluginData*/,const char *filename, ResourceAttributes *attrs);
		
	};
}

#endif //__TAG_OLE_H__

