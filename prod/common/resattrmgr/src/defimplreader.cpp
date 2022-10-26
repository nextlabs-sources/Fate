#if defined(_WIN32) || defined(_WIN64)
#include "FileAttributeReaderWriter.h"
#include "pdf_comment_attr.h"
#include "oleAttrs.h"
#include "ntfsAttrs.h"
#else
#error "Implemented for WINDOWS only"
#endif

#include "defimplreader.h"
#include "Office2k7_attrs.h"
#include "longtag.h"

// for CELog2
#include "NLCELog.h"
#define CELOG_CUR_FILE static_cast<celog_filepathint_t>(EMNLFILEINTEGER_DEFIMPLREADERCPP)

int DefImpl_Initialize(void **pluginData)
{
    InitializeNTFSStreamFunctions();
    InitializePDFTaggingMethod();
    *pluginData = NULL;
    return 0;
}

int DefImpl_Deinitialize(void * /*pluginData*/)
{
    return 0;
}

int DefImpl_GetAttributes(void * /*pluginData*/, const WCHAR *resourceName, ResourceAttributes *attrs, TagType iType )
{NLCELOG_ENTER
	for (int i=0;i<GetAttributeCount(attrs);i++)
	{
		fwprintf(stderr, L"param to store output attrs when reading tag contains %d tags:\n", GetAttributeCount(attrs));
		fwprintf(stderr, L"%s %s\n", GetAttributeName(attrs,i),GetAttributeValue(attrs,i));
	}



	//read existing 
	ResourceAttributes* existing_attrs;

	AllocAttributes(&existing_attrs);
    int res=GenericNextLabsTagging::GetFileCustomAttributes(resourceName, existing_attrs, iType );

	//no office file, no long tag
	if(!IsOLEFile(resourceName) && !IsOffice2k7FileType(resourceName))
	{
		//split multi values
		CMultiValueOperation::split(existing_attrs,attrs);
		FreeAttributes(existing_attrs);
		NLCELOG_RETURN_VAL(res)
	}
	else
	{
		//combine summary,summary.1,summary.2
		ResourceAttributes* combined_attrs;

		AllocAttributes(&combined_attrs);
		CLongTagOperation::combine(existing_attrs,combined_attrs);

		//split multi values
		CMultiValueOperation::split(combined_attrs,attrs);

		FreeAttributes(combined_attrs);
		FreeAttributes(existing_attrs);
		NLCELOG_RETURN_VAL(res)
	}
}


int Convert4GetAttributes(ResourceAttributes *attrs, ResourceAttributes* existing_attrs)
{

	//combine summary,summary.1,summary.2
	ResourceAttributes* combined_attrs;
	AllocAttributes(&combined_attrs);
	CLongTagOperation::combine(existing_attrs,combined_attrs);

	//split multi values
	CMultiValueOperation::split(combined_attrs,attrs);

	FreeAttributes(combined_attrs);

	return TRUE;
}

int DefImpl_SetAttributes(void * /*pluginData*/, const WCHAR *resourceName, ResourceAttributes *attrs, TagType iType)
{NLCELOG_ENTER

	//for all file format, check tag name, if tag name is too long, return fail, tag name length should not exceed MAX_TAG_NAME_LENGTH characters
	if (CLengthMax::is_tagname_too_long(attrs))
	{
		//0 is error code
		fwprintf(stderr, L"tag name length too long, must not exceed %d\n", MAX_TAG_NAME_LENGTH);
		return 0;
	}

	//merge multi values 
	ResourceAttributes* merged_attrs;

	AllocAttributes(&merged_attrs);
	CMultiValueOperation::merge(attrs,merged_attrs);

	//for all file format, check tag value, the total tag value length should not exceed MAX_TAG_VALUE_LENGTH characters
	//test case, if the qa=figo|lifi|...|wendy exceed 4096, we should truncate it to 4096
	CLengthMax::truncate_tagvalue(merged_attrs);

	//no office file, no long tag
	if ( TagTypeNTFS == iType )
	{
		BOOL res= GenericNextLabsTagging::SetFileCustomAttributes(resourceName, merged_attrs, TagTypeNTFS);
		FreeAttributes(merged_attrs);
		NLCELOG_RETURN_VAL(res)
	}
	else
	{
		if ( (!IsOLEFile(resourceName) && !IsOffice2k7FileType(resourceName)) )
		{
			BOOL res= GenericNextLabsTagging::SetFileCustomAttributes(resourceName, merged_attrs, TagTypeDefault);
			FreeAttributes(merged_attrs);
			NLCELOG_RETURN_VAL(res)
		}
		else
		{
#if 0   // comment by Tonny at 5/4/2016, it waste too much time .
			//office file, long tag
			//pre_add for office, split long value, remove old long value
			ResourceAttributes* attrs_to_set;
			AllocAttributes(&attrs_to_set);
			CLongTagOperation::pre_add(merged_attrs,resourceName,attrs_to_set);

			BOOL res= GenericNextLabsTagging::SetFileCustomAttributes(resourceName, attrs_to_set, TagTypeDefault);
			FreeAttributes(attrs_to_set);
#else
            BOOL res = GenericNextLabsTagging::SetFileCustomAttributes(resourceName, merged_attrs, TagTypeDefault);
#endif
			FreeAttributes(merged_attrs);
			NLCELOG_RETURN_VAL(res)
		}
	}
}

int Convert4SetAttributes(ResourceAttributes* attrs_to_set, ResourceAttributes* merged_attrs)
{	
	CLongTagOperation::pre_add(merged_attrs,attrs_to_set);
	return 1;
}

int DefImpl_RemoveAttributes(void * /*pluginData*/, const WCHAR *resourceName, ResourceAttributes *attrs)
{NLCELOG_ENTER

	//merge multi values 
	ResourceAttributes* merged_attrs;

	AllocAttributes(&merged_attrs);
	CMultiValueOperation::merge(attrs,merged_attrs);

	//no office file, no long tag
	if(!IsOLEFile(resourceName) && !IsOffice2k7FileType(resourceName))
	{
		int res=GenericNextLabsTagging::RemoveFileCustomAttributes(resourceName, merged_attrs);
		FreeAttributes(merged_attrs);
		NLCELOG_RETURN_VAL(res)
	}
	else
	{	
		//office, long tag, not only remove summary, need to remove summary,summary.1
		int res=CLongTagOperation::remove(merged_attrs,resourceName);
		FreeAttributes(merged_attrs);
		NLCELOG_RETURN_VAL(res)
	}
}

//convert tags stored in PDF file to tag format PC understands.
//this function is used by AdobePEP.
//AdobePEP read tag sometimes using native adobe SDK, so it needs convert the RAW format tags to format PC understands
//the RAW tag format for OFFICE file and NON Office file are different, so this function only used for NON Office file
//merged_attrs is output
int Convert_Raw_2_PC_For_Non_Office_Imp(ResourceAttributes *raw_attrs,ResourceAttributes* PC_attrs)
{
	//for all file format, check tag name, if tag name is too long, return fail, tag name length should not exceed MAX_TAG_NAME_LENGTH characters
	if (CLengthMax::is_tagname_too_long(raw_attrs))
	{
		//0 is error code
		fwprintf(stderr, L"tag name length too long, must not exceed %d\n", MAX_TAG_NAME_LENGTH);
		return 0;
	}

	//merge multi values 
	CMultiValueOperation::merge(raw_attrs,PC_attrs);

	//for all file format, check tag value, the total tag value length should not exceed MAX_TAG_VALUE_LENGTH characters
	//test case, if the qa=figo|lifi|...|wendy exceed 4096, we should truncate it to 4096
	CLengthMax::truncate_tagvalue(PC_attrs);

	return 0;
}

//convert tags stored in PDF file to tag format PC understands.
//this function is used by AdobePEP.
//AdobePEP read tag sometimes using native adobe SDK, so it needs convert the RAW format tags to format PC understands
//the RAW tag format for OFFICE file and NON Office file are different, so this function only used for NON Office file
//merged_attrs is output
int Convert_PC_2_RAW_For_Non_Office_Imp(ResourceAttributes *PC_attrs,ResourceAttributes* raw_attrs)
{
	//split multi values
	CMultiValueOperation::split(PC_attrs,raw_attrs);
	return 0;	
}
