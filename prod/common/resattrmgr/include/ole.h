#ifndef __TAGLIB_OLE_H__
#define __TAGLIB_OLE_H__ 
#include "resattrlib.h"
#include "utils.h"
#include "tag.h"
#include <gsf/gsf-msole-utils.h>
namespace NSLinuxTagLib
{
	class OLEFileTagging:public FileTagging
	{
		static const char OLEATTR_NOOLE_ERRMSG[];
		static const char OLEATTR_STREAM_NAME[];
		static const char OLEATTR_MSODATASTORE_NAME[];
		
		static void ForeachGetMetaData(char const *name,GsfDocProp const *prop, ResourceAttributes *pAttr);
		static int 	GetPropValue (GsfDocProp const *prop,char*value, size_t size);
		static BOOL CloneOLE (GsfInput *input, GsfOutput *output,ResourceAttributes *pAttr,bool bRemove=false);
		static BOOL CloneStorage (GsfInfile *in, GsfOutfile *out,ResourceAttributes *pAttr,bool bRemove=false);
		/*
		Descirption:
			Set or Remove tag(s) from the file.
		Parameters:
			pchFileName: the full path of the file
			pAttr: Attribute(s) waht to set or remove
			bRemove: true indicate to remove tag(s) instead set.
		Return:
			TRUE: Success
			FALSE: Failed
		*/
		BOOL		ModifyTags(void*/*pluginData*/,const char *pchFileName, ResourceAttributes *pAttr, bool bRemove);
		
		GsfInfile* 	GetGsfInfile(GsfInput*input,GError**err);
		GsfOutfile* 	GetGsfOutfile(GsfOutput*output,GError**err);
	public:
		OLEFileTagging();
		~OLEFileTagging();
		static BOOL IsOLE(const char*pchFileName);	
		BOOL 		GetTags(void*/*pluginData*/,const char *filename, ResourceAttributes *attrs);
		BOOL 		SetTags(void*/*pluginData*/,const char *filename, ResourceAttributes *attrs);
		BOOL 		RemoveTags(void*/*pluginData*/,const char *filename, ResourceAttributes *attrs);
	};
	
}

#endif //__TAGLIB_OLE_H__

