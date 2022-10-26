#include "FileAttributeReaderWriter.h"
#include "nxlAttrs.h"
#include "pdfattrs.h"
#include "tiffattrs.h"
#include "oleAttrs.h"
#include "Office2k7_attrs.h"
#include "ntfsAttrs.h"

// for CELog2
#include "NLCELog.h"
#define  CELOG_CUR_FILE static_cast<celog_filepathint_t>(EMNLFILEINTEGER_FILEATTRIBUTEWRITERCPP)

// There have been reports of UNICODE not working, but my experiments indicate otherwise
#define USE_UNICODE_FOR_VALUE

namespace GenericNextLabsTagging {
    BOOL SetFileCustomAttributes(const WCHAR *filename,
                                 ResourceAttributes *attrs, TagType iType )
    {NLCELOG_ENTER
		
        if (attrs == NULL || GetAttributeCount(attrs) == 0)
        {
            // No attributes to write == success
            NLCELOG_RETURN_VAL( TRUE )
        }

        if (GetFileAttributes(filename) == 0xFFFFFFFF)
        {
            // file doesn't exist
            NLCELOG_RETURN_VAL( FALSE )
        }

				if ( TagTypeNTFS == iType )
				{
						NLCELOG_RETURN_VAL( SetNTFSFileProps(filename, attrs) )
				}
		
        if (IsNXLFile(filename))
        {
            NLCELOG_RETURN_VAL( SetNXLFileProps(filename, attrs) == 0 )
        }

        if (IsPDFFile(filename)) 
		{
            NLCELOG_RETURN_VAL( SetPDFFileProps(filename, attrs) )
        }
		else if(IsTIFFFile(filename))//Support tiff files Kevin Zhou 2008-8-27
		{
				WIN32_FILE_ATTRIBUTE_DATA FileAttribute = { 0 };

				if ( GetFileAttributesExW ( filename, GetFileExInfoStandard, &FileAttribute ) && 0 == FileAttribute.nFileSizeLow && 0 == FileAttribute.nFileSizeHigh )
				{
					NLCELOG_RETURN_VAL( SetNTFSFileProps ( filename, attrs ) )
				}

				NLCELOG_RETURN_VAL( SetTIFFFileProps(filename, attrs) )
		}
		else if(IsOLEFile(filename))
		{
				NLCELOG_RETURN_VAL( FAILED(SetOLEFileProps(filename, attrs))? FALSE: TRUE )
		}
		else if(IsOffice2k7FileType(filename))
		{
				NLCELOG_RETURN_VAL( ( 0 != SetO2K7FileProps(filename, attrs)? FALSE: TRUE) )
		}
		else
        {
            NLCELOG_RETURN_VAL( SetNTFSFileProps(filename, attrs) )
        }
    }
}
