#ifndef __TAGLIB_FSYSEA_H__
#define __TAGLIB_FSYSEA_H__

#include "resattrlib.h"
#include "utils.h"
#include "tag.h"

namespace NSLinuxTagLib
{
	class FSysEAFileTagging:public FileTagging
	{
	private:
		/*
		opcode can be ATTR_OP_GET,ATTR_OP_SET and ATTR_OP_REMOVE
		*/
		BOOL ManipulateFSysEA(void*pluginData,const char*pchFileName,ResourceAttributes *pAttr,int opcode);
	
	public:
		FSysEAFileTagging();
		~FSysEAFileTagging();
		BOOL 		GetTags(void*/*pluginData*/,const char *filename, ResourceAttributes *attrs);
		BOOL 		SetTags(void*/*pluginData*/,const char *filename, ResourceAttributes *attrs);
		BOOL 		RemoveTags(void*/*pluginData*/,const char *filename, ResourceAttributes *attrs);
	
	};
	
}



#endif

