#include "FileAttributeReaderWriter.h"
#include "nxlAttrs.h"
#include "pdfattrs.h"
#include "tiffattrs.h"
#include "oleAttrs.h"
#include "Office2k7_attrs.h"
#include "ntfsAttrs.h"

// for CELog2
#include "NLCELog.h"
#define  CELOG_CUR_FILE static_cast<celog_filepathint_t>(EMNLFILEINTEGER_FILEATTRIBUTEREMOVECPP)

#define USE_UNICODE_FOR_VALUE

namespace GenericNextLabsTagging
{
	BOOL RemoveFileCustomAttributes(const WCHAR *name, ResourceAttributes *attrs)
	{NLCELOG_ENTER
		
		if(!name || !attrs)
			NLCELOG_RETURN_VAL(FALSE)

		if (IsNXLFile(name))
		{
			NLCELOG_RETURN_VAL(RemoveNXLFileProps(name, attrs) == 0)
		}

		if(IsTIFFFile(name))
		{
			NLCELOG_RETURN_VAL(RemoveTIFFFileProps(name, attrs));
			
		}
		else if (IsPDFFile(name))
		{
			NLCELOG_RETURN_VAL( RemovePDFFileProps(name, attrs) )
			
		}
		else if(IsOLEFile(name))
		{
			NLCELOG_RETURN_VAL( FAILED(RemoveOLEFileProps(name, attrs))? FALSE: TRUE )
		}
		else if(IsOffice2k7FileType(name))
		{
			NLCELOG_RETURN_VAL( (0 != RemoveO2K7FileProps(name, attrs)? FALSE: TRUE) )
		}
		else
		{
			NLCELOG_RETURN_VAL( RemoveNTFSFileProps(name, attrs) )
		}
	}
}
