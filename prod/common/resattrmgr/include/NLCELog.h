#pragma once

#include "celog.h"								

#if 0
#define chMSG(desc) message( "*" )
#else
#define chMSG(desc) message( __FILE__ "(" chSTR(__LINE__) "):" #desc )
#endif

///////////////////////////CELog2///////////////////////////////////////////////
// the current module name
#define CELOG_CUR_MODULE L"NLRESATTRMGR"

/*
*	if you want to use the CELog2 first you should define the file integer like this:
*
* NLCELOG_FILEINTEGERDEFINE_NOWARING
* #define  CELOG_CUR_FILE static_cast<celog_filepathint_t>(EMNLFILEINTEGER_XXX)
*/

// every file should has a file integer
enum 
{
	EMNLFILEINTEGER_STDAFX = 1,	
	
	// .CPP implement
	EMNLFILEINTEGER_ATTRMGRCPP,
	EMNLFILEINTEGER_BASE64CPP,	
	EMNLFILEINTEGER_DEFIMPLREADERCPP,	
	EMNLFILEINTEGER_FILEATTRIBUTEREADERCPP,		
	EMNLFILEINTEGER_FILEATTRIBUTEREMOVECPP,
	EMNLFILEINTEGER_FILEATTRIBUTEWRITERCPP,
	EMNLFILEINTEGER_FSYSEACPP,		
	EMNLFILEINTEGER_NLCELOGCPP,	
	EMNLFILEINTEGER_NTFSATTRSCPP,
	EMNLFILEINTEGER_NXLATTRSCPP, 
	EMNLFILEINTEGER_OFFICE2K7_ATTRSCPP,
	EMNLFILEINTEGER_OLECPP,			
	EMNLFILEINTEGER_OLEATTRSCPP,			
	EMNLFILEINTEGER_OOXMLCPP,						
	EMNLFILEINTEGER_PDFCPP,					
	EMNLFILEINTEGER_PDF_COMMENT_ATTRCPP,				
	EMNLFILEINTEGER_PDFATTRSCPP,				
	EMNLFILEINTEGER_RESATTRMGRCPP,				
	EMNLFILEINTEGER_STREAMREADERWRITERCPP,							
	EMNLFILEINTEGER_TIFFATTRSCPP, 
	EMNLFILEINTEGER_UTILSCPP,
	
	// .H implement 
	EMNLFILEINTEGER_BASE64H,				
	EMNLFILEINTEGER_DEFIMPLREADERH,				
	EMNLFILEINTEGER_FILEATTRIBUTEREADERWRITERH,						
	EMNLFILEINTEGER_FSYSEAH,					
	EMNLFILEINTEGER_LONGTAGH,
	EMNLFILEINTEGER_NLCELOGH,
	EMNLFILEINTEGER_NTFSATTRSH,
	EMNLFILEINTEGER_NXLATTRSH,
	EMNLFILEINTEGER_OFFICE2K7_ATTRSH,
	EMNLFILEINTEGER_OLEH,
	EMNLFILEINTEGER_OLEATTRSH,
	EMNLFILEINTEGER_OOXMLH,
	EMNLFILEINTEGER_PDFH,
	EMNLFILEINTEGER_PDF_COMMENT_ATTRH,
	EMNLFILEINTEGER_PDFATTRSH,
	EMNLFILEINTEGER_RESATTRMGRH,
	EMNLFILEINTEGER_STREAMREADERWRITERH,
	EMNLFILEINTEGER_TAGH,
	EMNLFILEINTEGER_TAG_OFFICE2K7H,
	EMNLFILEINTEGER_TIFFATTRSH,
	EMNLFILEINTEGER_UTILSH,
	EMNLFILEINTEGER_WFSE_PC_ENV_DECLAREH
};

////////////////////////////For CELog2 define//////////////////////////////////////////////

// Debug level
#define	NLCELOG_CRITICAL	CELOG_CRITICAL
#define	NLCELOG_ERROR			CELOG_ERROR
#define	NLCELOG_WARNING		CELOG_WARNING
#define	NLCELOG_INFO			CELOG_INFO
#define	NLCELOG_TRACE			CELOG_TRACE
#define	NLCELOG_DEBUG			CELOG_DEBUG
#define	NLCELOG_DUMP			CELOG_DUMP

#define NLCELOG_LOG CELOG_LOG

// Debug string
#define NLCELOG_DEBUGLOG( ... ) CELOG_LOG( NLCELOG_DEBUG, __VA_ARGS__ )

//
// External macros for function entering and leaving.  These should be used
// for:
// - functions that do not handle any exceptions.
// - functions that handle C++ exceptions.
//

// Function entering
#define NLCELOG_ENTER CELOG_ENTER;

// Function returning void
#define NLCELOG_RETURN CELOG_RETURN;

// Function returning a value whose type is supported
#define NLCELOG_RETURN_VAL(val) CELOG_RETURN_VAL(val);

// Function returning a value whose type is not supported
#define NLCELOG_RETURN_VAL_NOPRINT(val) CELOG_RETURN_VAL_NOPRINT(val);


//
// External macros for function entering and leaving.  These should be used
// for:
// - functions that handle Structured Exceptions (SEH).
//

// Function entering
#define NLCELOG_SEH_ENTER CELOG_SEH_ENTER;

// Function returning void
#define NLCELOG_SEH_RETURN CELOG_SEH_RETURN;

// Function returning a value whose type is supported
#define NLCELOG_SEH_RETURN_VAL(val) CELOG_SEH_RETURN_VAL(val);

// Function returning a value whose type is not supported
#define NLCELOG_SEH_RETURN_VAL_NOPRINT(val) CELOG_SEH_RETURN_VAL_NOPRINT(val);
	
///////////////////////////end///////////////////////////////////////////////

