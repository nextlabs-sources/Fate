#include "fsysea.h"
#include <attr/attributes.h>
#include <errno.h>
namespace NSLinuxTagLib
{
	
FSysEAFileTagging::FSysEAFileTagging(){};
FSysEAFileTagging::~FSysEAFileTagging(){};

BOOL 		FSysEAFileTagging::GetTags(void*pluginData,const char *filename, ResourceAttributes *attrs)
{
	return ManipulateFSysEA(pluginData,filename,attrs,ATTR_OP_GET);
}
BOOL 		FSysEAFileTagging::SetTags(void*pluginData,const char *filename, ResourceAttributes *attrs)
{
	return ManipulateFSysEA(pluginData,filename,attrs,ATTR_OP_SET);
}
BOOL 		FSysEAFileTagging::RemoveTags(void*pluginData,const char *filename, ResourceAttributes *attrs)
{
	return ManipulateFSysEA(pluginData,filename,attrs,ATTR_OP_REMOVE);
}

BOOL FSysEAFileTagging::ManipulateFSysEA(void*pluginData,const char*pchFileName,ResourceAttributes *pAttr,int opcode)
{
	if(ATTR_OP_GET!=opcode&&ATTR_OP_SET!=opcode&&ATTR_OP_REMOVE!=opcode)
		return FALSE;
	if(pchFileName==NULL||pAttr==NULL)
		return FALSE;
	
	if(ATTR_OP_GET==opcode&&GetAttributeCount(pAttr)==0)
	{
		char* pBuffer=new char[ATTR_MAX_VALUELEN];
		attrlist_cursor_t cursor;
		attrlist_t* pAttrList=(attrlist_t*)pBuffer;
		memset(&cursor,0,sizeof(cursor));
		do
		{
			memset(pBuffer,0,ATTR_MAX_VALUELEN);
			int iRet=attr_list(pchFileName,pBuffer,ATTR_MAX_VALUELEN,ATTR_DONTFOLLOW,&cursor);
			if(iRet!=0)
			{
				delete[] pBuffer;
				return FALSE;
			}
			
			for(int i=0;i<pAttrList->al_count;i++)
			{
				attrlist_ent_t* pattr_ent=ATTR_ENTRY(pAttrList, i);
				if(pattr_ent==NULL)
					continue;
				AddAttribute(pAttr,pattr_ent->a_name,"");
				//printf("Attribute %d:%s[%d]\n",i+1,pattr_ent->a_name,pattr_ent->a_valuelen);
			}
		}while(pAttrList->al_more);
		delete [] pBuffer;
		if(GetAttributeCount(pAttr)==0)
			return TRUE;
	}
	
	int iAttrCount=GetAttributeCount(pAttr);
	if(iAttrCount==0)
		return TRUE;
	
	BOOL bRet=TRUE;
	
	for(int i=0;i<iAttrCount;i++)
	{
		char attrvalue[1024]="";
		int ivaluelen=sizeof(attrvalue);
		switch(opcode)
		{
		case ATTR_OP_GET:
			if( attr_get (pchFileName,GetAttributeName(pAttr,i),attrvalue,& ivaluelen,ATTR_DONTFOLLOW)==0)
			{
				attrvalue[ivaluelen]=0;
				SetAttributeValue(pAttr,i,attrvalue);
			}
			else
			{
				if(errno!=ENODATA)
				{
					bRet=FALSE;
					perror("Tagging library:attr_get");
				}
			}
			break;
		case ATTR_OP_SET:
			if( attr_set (pchFileName, GetAttributeName(pAttr,i),
                   GetAttributeValue(pAttr,i),strlen(GetAttributeValue(pAttr,i)),
                   ATTR_DONTFOLLOW)!=0)
			{
				bRet=FALSE;
      	perror("Tagging library:attr_set:");
      }
			break;
		case ATTR_OP_REMOVE:
			if(attr_remove (pchFileName, GetAttributeName(pAttr,i), ATTR_DONTFOLLOW)!=0)
			{
				if(errno!=ENODATA)
				{
					bRet=FALSE;
					perror("Tagging library:attr_remove:");
				}
			}
			break;
		default:
			break;
		}
	}
	return bRet;
}
	
}


