#include "ole.h"
#include <string.h>
#include <gsf/gsf-input-stdio.h>
#include <gsf/gsf-infile.h>
#include <gsf/gsf-infile-msole.h>
#include <gsf/gsf-msole-utils.h>
#include <gsf/gsf-docprop-vector.h>

#include <gsf/gsf-output-stdio.h>
#include <gsf/gsf-outfile.h>
#include <gsf/gsf-outfile-msole.h>
#include <errno.h>
#include <gsf/gsf-utils.h>

// for CELog2
#include "NLCELog.h"
#define  CELOG_CUR_FILE static_cast<celog_filepathint_t>(EMNLFILEINTEGER_OLECPP)

namespace NSLinuxTagLib
{
const char OLEFileTagging::OLEATTR_NOOLE_ERRMSG[]="No OLE2 signature";
const char OLEFileTagging::OLEATTR_STREAM_NAME[]="\05DocumentSummaryInformation";   
const char OLEFileTagging::OLEATTR_MSODATASTORE_NAME[]="MsoDataStore";   

GsfInfile* 	OLEFileTagging::GetGsfInfile(GsfInput*input,GError**err)
{
	input = gsf_input_uncompress (input);
	GsfInfile* infile = gsf_infile_msole_new (input, err);
	return infile;	
}

GsfOutfile* 	OLEFileTagging::GetGsfOutfile(GsfOutput*output,GError**err)
{
	return  gsf_outfile_msole_new (output);
}
BOOL OLEFileTagging::IsOLE(const char*pchFileName)
{
	GsfInput  *input;
	GError *err=NULL;
	
	input = gsf_input_stdio_new (pchFileName, &err);
	
	if (input == NULL)
	{
		g_return_val_if_fail (err != NULL, FALSE);
		//g_warning ("'%s' error: %s",pchFileName, err->message);
		g_error_free (err);
		return FALSE;
	}
	
	input = gsf_input_uncompress (input);
	
	GsfInfile* infile = gsf_infile_msole_new (input, &err);
	g_object_unref (G_OBJECT (input));

	if (infile == NULL) 
	{
		BOOL bRet = FALSE;
		g_return_val_if_fail (err != NULL, FALSE);
		//g_warning ("'%s' Not an OLE file: %s", pchFileName, err->message);
		if (strstr ( err->message, OLEATTR_NOOLE_ERRMSG))
			bRet = FALSE;
		g_error_free (err);
		return bRet;
	}
	g_object_unref (G_OBJECT (infile));
	return TRUE;
}
OLEFileTagging::OLEFileTagging()
{
	gsf_init ();
}
OLEFileTagging::~OLEFileTagging()
{
	gsf_shutdown ();
}
BOOL OLEFileTagging::GetPropValue (GsfDocProp const *prop,char*value, size_t size)
{
	GValue const *val = gsf_doc_prop_get_val  (prop);
	char *tmp;
	if(prop==NULL)
		return FALSE;
	if (VAL_IS_GSF_DOCPROP_VECTOR ((GValue *)val)) 
		return FALSE;
	else
	{
		tmp = g_strdup_value_contents (val);
		if(tmp&&tmp[0]=='\"'&&strlen(tmp)>1)
			strncpy_s(value,size,tmp+1, _TRUNCATE);
		else
			strncpy_s(value,size,tmp,_TRUNCATE);
		g_free (tmp);
		return TRUE;
	}
}
void OLEFileTagging::ForeachGetMetaData(char const *name,GsfDocProp const *prop, ResourceAttributes *pAttr)
{
	const char* pLink=gsf_doc_prop_get_link (prop);
	if (pLink != NULL)
		AddAttribute(pAttr,name,pLink);
	else
	{
		char chValue[512]="";
		if(GetPropValue(prop,chValue, 512)==TRUE)
			AddAttribute(pAttr,name,chValue);
	}
	
	return;	
}
BOOL OLEFileTagging::CloneStorage (GsfInfile *in, GsfOutfile *out,ResourceAttributes *pAttr,bool bRemove)
{
	GsfInput 	*new_input;
	GsfOutput 	*new_output;
	gboolean 	is_dir;
	GError    	*err = NULL;
	int i;
	const gchar *name;

	for (i = 0 ; i < gsf_infile_num_children (in) ; i++) 
	{
		new_input = gsf_infile_child_by_index (in, i);
		/* In theory, if new_file is a regular file (not directory),
		 * it should be GsfInput only, not GsfInfile, as it is not
		 * structured.  However, having each Infile define a 2nd class
		 * that only inherited from Input was cumbersome.  So in
		 * practice, the convention is that new_input is always
		 * GsfInfile, but regular file is distinguished by having -1
		 * children.
		 */
		
		is_dir = GSF_IS_INFILE (new_input) &&
			gsf_infile_num_children (GSF_INFILE (new_input)) >= 0;
		name = gsf_infile_name_by_index(in, i);
		//if(strcmp(name,OLEATTR_MSODATASTORE_NAME)==0)
		//{
		//	//Ignore the "MsoDataStore" storage. the libgsf has a bug when it handle 
		//	//"MsoDataStore" stream.It will corrupt the office document. Currently, we
		//	//found that there will be a "MsoDataStore" storage when download a office 
		//	//document from SharePoint.
		//}
		//else
		{
			new_output = gsf_outfile_new_child  (out,name,is_dir);
			if(strcmp(name,OLEATTR_STREAM_NAME)==0)
			{
				GsfDocMetaData * meta_data=gsf_doc_meta_data_new();
				gsf_msole_metadata_read(new_input, meta_data);
				//printf("before insert .....\n"); 
				//gsf_doc_meta_dump (meta_data);
				long lIdx=0,lCount=GetAttributeCount(pAttr);
				if(bRemove==true)
				{
					for(lIdx=0;lIdx<lCount;lIdx++)
					{
						const char*pName=GetAttributeName(pAttr,lIdx);
						if(pName&&strlen(pName))
							gsf_doc_meta_data_remove(meta_data,pName);
					}
				}
				else
				{
					for(lIdx=0;lIdx<lCount;lIdx++)
					{
						const char*pName  = GetAttributeName(pAttr,lIdx); 
						const char*pValue = GetAttributeValue(pAttr,lIdx); 
						char* chAttrName=(char*)malloc(strlen(pName)+1);
						strncpy_s(chAttrName,strlen(pName)+1,pName,_TRUNCATE);
						if(pName&&strlen(pName)&&pValue&&strlen(pValue))
						{           
							GsfDocProp*pProp=gsf_doc_prop_new (chAttrName);
							GValue*value=g_value_init(g_new0(GValue,1),G_TYPE_STRING);
							g_value_set_string(value,pValue);
							gsf_doc_prop_set_val(pProp,value);
							gsf_doc_meta_data_store  (meta_data,pProp);
							//GValue*value=g_value_init(g_new0(GValue,1),G_TYPE_STRING);
							//g_value_set_string(value,pValue);
							//gsf_doc_meta_data_insert(meta_data,strdup(pName),value);
						}
					}
				}
				gsf_msole_metadata_write(new_output,meta_data,true);
				//printf("after insert.....\n");
				//gsf_doc_meta_dump (meta_data);
				g_object_unref(G_OBJECT(meta_data));
			}
			else
			{
				if(CloneOLE (new_input, new_output,pAttr,bRemove)==FALSE)
					return FALSE;
			}
		}
		
	}
	return TRUE;
	/* An observation: when you think about the explanation to is_dir
	 * above, you realize that clone_dir is called even for regular files.
	 * But nothing bad happens, as the loop is never entered.
	 */
}
BOOL OLEFileTagging::CloneOLE (GsfInput *input, GsfOutput *output,ResourceAttributes *pAttr,bool bRemove)
{
	BOOL bRet=TRUE;
	if (gsf_input_size (input) > 0)
	{
		//guint8 const *data;
		//size_t len;
		gsf_input_seek(input,0,G_SEEK_SET);
		gsf_output_seek(output,0,G_SEEK_SET);
		if(gsf_input_copy(input,output)==FALSE)
			return FALSE;
		//while ((len = gsf_input_remaining (input)) > 0) 
		//{
		//	/* copy in odd sized chunks to exercise system */
		//	if (len > 314)
		//		len = 314;
		//	if (NULL == (data = gsf_input_read (input, len, NULL))) 
		//	{
		//		g_warning ("error reading ?");
		//		return FALSE;
		//	}
		//	if (!gsf_output_write (output, len, data)) 
		//	{
		//		g_warning ("error writing ?");
		//		return FALSE;
		//	}
		//}
	} 
	else if (GSF_IS_INFILE(input))
		bRet=CloneStorage (GSF_INFILE(input), GSF_OUTFILE(output),pAttr,bRemove);
		

	gsf_output_close (output);
	g_object_unref (G_OBJECT (output));
	g_object_unref (G_OBJECT (input));
	return bRet;
}
BOOL OLEFileTagging::GetTags(void*/*pluginData*/,const char *pchFileName, ResourceAttributes *pAttr)
{NLCELOG_ENTER
	GsfInput  *stream;
	GsfInfile *infile;
	GError    *err = NULL;
		
	infile = FileTagging::GetGsfInfile(pchFileName,&err);
	
	if (infile == NULL) 
		NLCELOG_RETURN_VAL( FALSE )
	
	stream = gsf_infile_child_by_name (infile, OLEATTR_STREAM_NAME);
	if (stream != NULL) 
	{
		GsfDocMetaData *meta_data = gsf_doc_meta_data_new ();

		err = gsf_msole_metadata_read (stream, meta_data);
		if (err != NULL) 
		{
			g_warning ("'%s' error: %s", pchFileName, err->message);
			g_error_free (err);
			err = NULL;
		} 
		else
		{
			//gsf_doc_meta_dump (meta_data);
			if(GetAttributeCount(pAttr))
			{
				for(int i=0;i<GetAttributeCount(pAttr);i++)
				{
					const char*pName=GetAttributeName(pAttr,i);
					if(pName&&strlen(pName))
					{
						GsfDocProp * prop=gsf_doc_meta_data_lookup(meta_data,pName);
						char chValue[512]="";
						if(prop&&GetPropValue(prop,chValue, 512)==TRUE)
							SetAttributeValue(pAttr,i,chValue);
					}
				}
			}
			else
				gsf_doc_meta_data_foreach(meta_data,(GHFunc)ForeachGetMetaData,pAttr);
		}

		g_object_unref (G_OBJECT(meta_data));
		g_object_unref (G_OBJECT (stream));
	}
	g_object_unref (G_OBJECT (infile));

	NLCELOG_RETURN_VAL( TRUE )
}
BOOL OLEFileTagging::SetTags(void*/*pluginData*/,const char *pchFileName, ResourceAttributes *pAttr)
{NLCELOG_ENTER
	NLCELOG_RETURN_VAL( ModifyTags(NULL,pchFileName,pAttr,false) )	
}
BOOL OLEFileTagging::RemoveTags(void*/*pluginData*/,const char *pchFileName, ResourceAttributes *pAttr)
{NLCELOG_ENTER
	NLCELOG_RETURN_VAL( ModifyTags(NULL,pchFileName,pAttr,true) )
}
BOOL OLEFileTagging::ModifyTags(void*/*pluginData*/,const char *pchFileName, ResourceAttributes *pAttr, bool bRemove)
{NLCELOG_ENTER
	GsfInfile  *infile;
	GsfOutput  *output;
	GsfOutfile *outfile;
	GError    *err = NULL;

	infile = FileTagging::GetGsfInfile(pchFileName,&err);

	if (infile == NULL) 
	{
		g_return_val_if_fail (err != NULL, 1);
		g_error_free (err);
		NLCELOG_RETURN_VAL( FALSE )
	}
	std::string strNewFileName,strFullPath;
	strNewFileName=pchFileName;
	std::string::size_type pos=strNewFileName.rfind("/");
	if(pos!=std::string::npos)
		strFullPath=strNewFileName.substr(0,pos+1);
	else
		strFullPath="";

	if(NSLinuxTagLib::Utils::GetTempFile(pchFileName,strNewFileName)==FALSE)
	{
		g_object_unref (G_OBJECT (infile));
		NLCELOG_RETURN_VAL( FALSE )
	}

	strFullPath+=strNewFileName;
	outfile = FileTagging::GetGsfOutfile(strFullPath.c_str(),&err);
	
	if(CloneOLE (GSF_INPUT (infile), GSF_OUTPUT (outfile),pAttr,bRemove)==FALSE)
	{
		remove(strFullPath.c_str());
		NLCELOG_RETURN_VAL( FALSE )
	}
	else
	{
		int iRet=rename(strFullPath.c_str(),pchFileName);
		if(iRet)
		{
			printf("rename failed(%s->%s). errno=%d\n",strFullPath.c_str(),pchFileName,errno);
			perror("rename error");
			if(iRet==EEXIST)//If the file already exist, then adopt another approach
			{
				std::string strBackName=pchFileName;
				strBackName+=".bak";
				iRet=rename(pchFileName,strBackName.c_str());//backup the source file first
				if(iRet)
				{//failed for backup, then return FALSE and remove the one tag changed
					printf("rename for backup file failed. errno=%d\n",errno);
					perror("rename for backup file error");
					remove(strFullPath.c_str());
					NLCELOG_RETURN_VAL( FALSE )
				}
				else
				{//success. then rename the one tag changed to the source
					iRet=rename(strFullPath.c_str(),pchFileName);
					if(iRet)
					{//failed, then rename the backup file to back
						printf("rename failed again. errno=%d\n",errno);
						perror("rename error again");
						rename(strBackName.c_str(),pchFileName);
						remove(strFullPath.c_str());
						NLCELOG_RETURN_VAL( FALSE )
					}
					else
					{//succeess. then remove the backup file
						remove(strBackName.c_str());
						NLCELOG_RETURN_VAL( TRUE )
					}
				}
			}
			else
			{//other type error: remove the one with tag changed and return FALSE
				remove(strFullPath.c_str());
				NLCELOG_RETURN_VAL( FALSE )
			}
		}
	}

	NLCELOG_RETURN_VAL( TRUE )
}

}
