#include "pdf.h"
#include <string.h>
#include <iostream>
#include <errno.h>
#include "podofo.h"
#include "PdfError.h"
#include "pdfattrs.h"

// for CELog2
#include "NLCELog.h"
#define  CELOG_CUR_FILE static_cast<celog_filepathint_t>(EMNLFILEINTEGER_PDFCPP)

using namespace PoDoFo;
namespace NSLinuxTagLib
{
const char PDFFileTagging::PDF_EXT[]=".pdf";
BOOL PDFFileTagging::IsPDF(const char*pchFileName)
{
	char* pSuffix = strrchr(pchFileName, L'.');
    if(NULL == pSuffix) 
        return FALSE;
    
    if (0 == strcasecmp(pSuffix, PDF_EXT))
        return TRUE;
    
    return FALSE;
}

BOOL PDFFileTagging::GetTags(void*/*pluginData*/,const char *pchFileName, ResourceAttributes *pAttr)
{NLCELOG_ENTER
	BOOL bSuccess = TRUE;
    try
    {
        PdfMemDocument pdfDoc(pchFileName);

        PdfDictionary dictionary = pdfDoc.GetInfo()->GetObject()->GetDictionary();
   		//TKeyMap dictionaryKeys = dictionary.GetKeys();

        for (TKeyMap::const_iterator iter = dictionary.GetKeys().begin();
             iter != dictionary.GetKeys().end();
             ++iter)
        {
            PdfObject *obj = iter->second;

            if (obj && obj->IsString())
            {
                std::string key = (iter->first).GetName();
				
				if(key.length() > 15)//kevin 2008-10-30
					continue;
                // Perhaps this should be a set of keys to ignore
        		// if (key != "CreationDate" && key != "Creator" && key != "ModDate" && key != "Title" && key != "Author" && key != "Producer" && key != "Keywords")
         		// 	{
                    const char *value = obj->GetString().GetString();
                    AddAttributeA(pAttr, key.c_str(), value);
          		//  }
            }
        }

		bSuccess = TRUE;

    } catch (PdfError& err) {
        std::cout << "Caught an error!" << std::endl;
        err.PrintErrorMsg();
				NLCELOG_DEBUGLOG( L"Caught an error \n" );
		bSuccess = FALSE;
    }
	NLCELOG_RETURN_VAL( TRUE ); // why always return true ? ... 
}

PDFFileTagging::PDFFileTagging()
{
	//PdfError::EnableLogging(false);	
}
BOOL FindTagNameWithoutCaseSensitive(PdfDictionary& dic,const char* pszTagName, std::string& strTagName)
{
	if(!pszTagName)
		return FALSE;

	BOOL bSuccess = FALSE;
	try 
	{
		for (TKeyMap::const_iterator iter = dic.GetKeys().begin();
			iter != dic.GetKeys().end();
			++iter)
		{
			PdfObject *obj = iter->second;

			if (obj && obj->IsString())
			{
				std::string key = (iter->first).GetName();

				if(key.length() > 15)//kevin 2008-10-30
					continue;
				// Perhaps this should be a set of keys to ignore
				//if (key != "CreationDate" && key != "Creator" && key != "ModDate" && key != "Title" && key != "Author" && key != "Producer" && key != "Keywords")
				//{
					if(0 == strncasecmp(key.c_str(), pszTagName, key.length()))
					{
						strTagName = key;
						bSuccess = TRUE;
						break;
					}
				//}
			}
		}
	} 
	catch (PdfError& err) 
	{
		std::cout << "Caught an error!" << std::endl;
		err.PrintErrorMsg();
		bSuccess = FALSE;
	}
	return bSuccess;
}
BOOL PDFFileTagging::SetTags(void*pluginData,const char *pchFileName, ResourceAttributes *pAttr)
{
	return ModifyTags(pluginData,pchFileName,pAttr,false);
}
BOOL PDFFileTagging::ModifyTags(void*/*pluginData*/,const char *pchFileName, ResourceAttributes *pAttr,bool bRemove)
{NLCELOG_ENTER
	if (pAttr == NULL || GetAttributeCount(pAttr) <= 0)
    {
        NLCELOG_RETURN_VAL( FALSE )
    }

    std::string strNewFileName;
	if(NSLinuxTagLib::Utils::GetTempFile(pchFileName,strNewFileName)==FALSE)
		NLCELOG_RETURN_VAL( FALSE )
	BOOL bSuccess = TRUE;
    try 
    {
        PdfMemDocument pdfDoc(pchFileName);

        PdfDictionary& dictionary = pdfDoc.GetInfo()->GetObject()->GetDictionary();

        int count = GetAttributeCount(pAttr);

        for (int i = 0; i < count; ++i)
        {
            const char *attrName = GetAttributeName(pAttr, i);
			if(bRemove==true)
			{
				if(bSuccess==TRUE)
					bSuccess = dictionary.RemoveKey(PdfName(attrName));
			}
			else
			{
				//if(1)//added by kevin 008-11-12
				{
					std::string strTagName;
					if(FindTagNameWithoutCaseSensitive(dictionary, attrName, strTagName))
					{
						dictionary.RemoveKey(PdfName(strTagName.c_str()));
					}
				}
	
	            const char *attrValue = GetAttributeValue(pAttr, i);
	            dictionary.AddKey( PdfName(attrName), PdfString(attrValue) );
	        }
        }
        
        pdfDoc.Write(strNewFileName.c_str());
		bSuccess = TRUE;
    }
    catch (PdfError& err) 
    {
        std::cout << "Caught an error setting PDF tags.  Error code: " << err.GetError() << std::endl;
        err.PrintErrorMsg();
		bSuccess = FALSE;
    }
    
    if(bSuccess)
    {
        BOOL bRet = rename(strNewFileName.c_str(),pchFileName); //ReplaceFileA(mbFileName, mbNewFileName, NULL, REPLACEFILE_WRITE_THROUGH, 0, 0);
		
		if(bRet)
		{
			printf("Failed to ReplaceFileA (Add tags for PDF files) errno=%d",errno);
			remove(strNewFileName.c_str());
			NLCELOG_RETURN_VAL( FALSE )
		}
		
    }
    else
    	remove(strNewFileName.c_str());

    NLCELOG_RETURN_VAL( bSuccess )
}
BOOL PDFFileTagging::RemoveTags(void*pluginData,const char *pchFileName, ResourceAttributes *pAttr)
{
	return ModifyTags(pluginData,pchFileName,pAttr,true);
}		
PDFFileTagging2::PDFFileTagging2()
{
	//PdfError::EnableLogging(false);	
};
BOOL PDFFileTagging2::IsPDF(const char*pchFileName)
{
	return IsPDFFile(pchFileName);
}
BOOL PDFFileTagging2::GetTags(void*/*pluginData*/,const char *pchFileName, ResourceAttributes *pAttr)
{
	return GetPDFFileProps(pchFileName,pAttr);
}
BOOL PDFFileTagging2::SetTags(void*pluginData,const char *pchFileName, ResourceAttributes *pAttr)
{
	return SetPDFFileProps(pchFileName,pAttr);
}
BOOL PDFFileTagging2::RemoveTags(void*pluginData,const char *pchFileName, ResourceAttributes *pAttr)
{
	return RemovePDFFileProps(pchFileName,pAttr);
}

}

