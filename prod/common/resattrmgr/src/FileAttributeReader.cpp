// Sample code from MSDN for reading the custom file attributes from the NT File system
// Incorporate to our code for adding BJ attributes to the files
#include <string>

using namespace std;

#include "FileAttributeReaderWriter.h"
#include "nxlAttrs.h"
#include "pdfattrs.h"
#include "tiffattrs.h"
#include "oleAttrs.h"
#include "Office2k7_attrs.h"
#include "ntfsAttrs.h"
#include "NxlFormatFile.h"

// for CELog2
#include "NLCELog.h"
#define  CELOG_CUR_FILE static_cast<celog_filepathint_t>(EMNLFILEINTEGER_FILEATTRIBUTEREADERCPP)

// Vishi wants client_id values split at commas
#define SPLIT_CLIENT_ID


#include "eframework\platform\windows_file_server_enforcer.hpp"


namespace GenericNextLabsTagging { 
    BOOL TagExists(ResourceAttributes *attrs, LPCWSTR pszTagName, BOOL bCaseSensitive)//used to check if the tag existes already in a ResourceAttributes object. kevin 2008-12-5
    {
        if(!attrs || !pszTagName)
            return FALSE;

        int size = GetAttributeCount(attrs);

        BOOL bExists = FALSE;
        for (int i = 0; i < size; ++i)
        {
            WCHAR *tagName = (WCHAR *)GetAttributeName(attrs, i);

            int nRet = 0;
            if(bCaseSensitive)
            {
                nRet = wcscmp(pszTagName, tagName);
            }
            else
            {
                nRet = _wcsicmp(pszTagName, tagName);
            }

            if(nRet == 0)
            {
                bExists = TRUE;
                break;
            }

        }
        return bExists;
    }

    // Returning one dimensional array of key/value pair
    // Count is the number of the tuple pairs
    BOOL GetFileCustomAttributes (LPCWSTR fileName, ResourceAttributes *attrs, TagType iType )
    {NLCELOG_ENTER
        if (fileName == NULL || attrs == NULL)
            NLCELOG_RETURN_VAL( FALSE )
 
        BOOL bRet = TRUE;
  
		if ( TagTypeNTFS == iType )
		{
			GetNTFSFileProps(fileName, attrs);
		}

		CNxlFormatFile nxlFile;

        // If the file is an NXL file, the attributes in its NXL header is the
        // "Gold" standard.  Don't add file-type specific attributes or NTFS
        // attributes afterwards.
        //
        // If the file is an NXL 1.0 file, the attributes in its NXL 1.0 stream
        // is *not* the "Gold" standard.  We need to add file-type specific
        // attributes and NTFS attributes afterwards.
        //
        //	only do this when this is not a WFSE PC
        //	this is because, if we are in wfse pc env,
        //	calling below function will cause hang, as below function will call ReadFile,
        //	we decide not affect PE/SE module code, and don't call to get those PE/SE tags
        //	when we are in wfse pc env
        if (!nextlabs::windows_fse::is_wfse_installed())
        {
            if (IsNXLFile(fileName))
            {
                bRet = (GetNXLFileProps(fileName, attrs) == 0);
                goto exit;
            }

            if (IsNXL10File(fileName))
            {
                bRet = (GetNXL10FileProps(fileName, attrs) == 0);
            }
        }

        // Check for file-type specific attributes
		
		if (nxlFile.IsNXLFormatFile(fileName))
		{
			::OutputDebugString(L"Enter NXL File Read Tag!\n");
			nxlFile.GetNXLFileProps(fileName,attrs);
		}
		else
		{
			if (IsPDFFile(fileName))
			{
				if (iType  == TagSummary)
				{
					bRet = GetPDFSummaryProps(fileName, attrs);
				}
				else
				{
					bRet = GetPDFFileProps(fileName, attrs); 
				}
			} 
			else if (IsTIFFFile(fileName))
			{
				bRet = GetTIFFFileProps(fileName, attrs);
			}
			else if(IsOLEFile(fileName))
			{
				if (iType  == TagSummary)
				{
					bRet = FAILED(GetOLEFileSummaryProps(fileName, attrs))? FALSE:TRUE;
				}
				else
				{
					bRet = FAILED(GetOLEFileProps(fileName, attrs))? FALSE:TRUE;
				}
			}
			else if(IsOffice2k7FileType(fileName))
			{
				if (iType  == TagSummary)
				{
					bRet = (0 != GetO2K7FileSummaryProps(fileName, attrs)? FALSE: TRUE);
				}
				else
				{
					bRet = (0 != GetO2K7FileProps(fileName, attrs)? FALSE: TRUE);
				}
			}

			if (IsNTFSFile(fileName))
			{
				// If error occurs while reading NTFS attributes, ignore the error
				// and just return the file-type specific attributes from above.
				GetNTFSFileProps(fileName, attrs);
			}

		}
		
       
exit:
        NLCELOG_RETURN_VAL( bRet )
    }


    void AddKeyValueHelperA(ResourceAttributes *attrs, const char *key, const char *value)
    {
        if(!attrs || !key || !value)
            return;
#ifdef SPLIT_CLIENT_ID
        size_t size = 0;
        mbstowcs_s(&size, NULL, 0, key, 0);
        WCHAR *wKey = new WCHAR[size];
        mbstowcs_s(&size, wKey, size, key, _TRUNCATE);
    
        mbstowcs_s(&size, NULL, 0, value, 0);
        WCHAR *wValue = new WCHAR[size];
        mbstowcs_s(&size, wValue, size, value, _TRUNCATE);

        AddKeyValueHelperW(attrs, wKey, wValue);
        delete[] wKey;
        delete[] wValue;
#else
        AddAttributeA(attrs, key, value);
#endif
    
    }

    void AddKeyValueHelperW(ResourceAttributes *attrs, const WCHAR *key, const WCHAR *value)
    {
        if(!attrs || !key || !value)
            return;
#ifdef SPLIT_CLIENT_ID
        if (wcscmp(key, L"client_id") == 0)
        {
            static WCHAR seps[] = L",";
            // wcstok is actually thread safe, according to MSDN
            WCHAR *mutable_value = _wcsdup(value);

            WCHAR *next_token = NULL;
            WCHAR *token = wcstok_s(mutable_value, seps, &next_token);

            while(token != NULL)
            {
                AddAttribute(attrs, key, token);
                token = wcstok_s(NULL, seps, &next_token);
            }

            free(mutable_value);
        }
        else
#endif
        {
            AddAttribute(attrs, key, value);
        }
    }
}


