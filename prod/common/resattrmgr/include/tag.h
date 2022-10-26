#ifndef __TAGLIB_TAG_H__
#define __TAGLIB_TAG_H__
#include "resattrlib.h"
#include "utils.h"
#include <gsf/gsf-input-stdio.h>
#include <gsf/gsf-infile.h>
#include <gsf/gsf-output-stdio.h>
#include <gsf/gsf-outfile.h>
namespace NSLinuxTagLib
{
	class FileTagging
	{
		virtual GsfInfile* 	GetGsfInfile(GsfInput*input,GError**err){return NULL;};
		virtual GsfOutfile* GetGsfOutfile(GsfOutput*output,GError**err){return NULL;};
	public:
		virtual BOOL 		GetTags(void*/*pluginData*/,const char *filename, ResourceAttributes *attrs)=0;
		virtual BOOL 		SetTags(void*/*pluginData*/,const char *filename, ResourceAttributes *attrs)=0;
		virtual BOOL 		RemoveTags(void*/*pluginData*/,const char *filename, ResourceAttributes *attrs)=0;
		
		GsfInfile* 	GetGsfInfile(const char* pchFileName, GError**err=NULL)
		{
			GsfInput  *input;
			GsfInfile *infile;
			GError* inErr,**ppErr;
			if(err==NULL)
				ppErr=&inErr;
			else
				ppErr=err;
			input = gsf_input_stdio_new (pchFileName, ppErr);
	
			if (input == NULL)
			{
				g_return_val_if_fail (*ppErr != NULL, NULL);
				g_warning ("'%s' error: %s",pchFileName, (*ppErr)->message);
				g_error_free (*ppErr);
				return NULL;
			}
			
			infile = GetGsfInfile (input, ppErr);
			g_object_unref (G_OBJECT (input));
			
			if (infile == NULL) 
			{
				g_return_val_if_fail (*ppErr != NULL, NULL);
				g_error_free (*ppErr);
				return NULL;
			}	
			return infile;
		}
		GsfOutfile* 	GetGsfOutfile(const char* pchFileName,GError**err=NULL)
		{
			GsfOutput  *output;
			GsfOutfile *outfile;
			GError* inErr,**ppErr;
			if(err==NULL)
				ppErr=&inErr;
			else
				ppErr=err;
			output = gsf_output_stdio_new (pchFileName, ppErr);
	
			if (output == NULL)
			{
				g_return_val_if_fail (*ppErr != NULL, NULL);
				g_warning ("'%s' error: %s",pchFileName, (*ppErr)->message);
				g_error_free (*ppErr);
				return NULL;
			}
			
			outfile = GetGsfOutfile (output, ppErr);
			g_object_unref (G_OBJECT (output));
			
			if (outfile == NULL) 
			{
				g_return_val_if_fail (*ppErr != NULL, NULL);
				g_error_free (*ppErr);
				return NULL;
			}	
			return outfile;
		}
	};
}

#endif //__TAGLIB_H__
