/*
Defines the basic type for the PA,and the standard interface.
The PAF will use this interface to implement the PA.
And the type of the parameter are defined in the name space of PABase.
*/
#ifndef __NEXTLAB_PA_BASES_H__
#define __NEXTLAB_PA_BASES_H__

#include <string>
#include <list>

namespace PABase
{
	/*
	define the basic string.
	*/
	typedef std::basic_string< wchar_t > tString ;
	/*
	define the baisc attribute list.
	key:	This is the identification for the attribute item. It is reserved.
			Because the attribute has been parsed by the PEP, and the key also has been removed right now, 
			so it can be empty. And the PEP should not push the attribute of number to the list of obligation.
			Now:	For the attribute of obligation:
	value: It is the value of attribute, the PA just concern this data for an obligation.
	*/
	typedef struct _tagATTRIBUTE
	{
		tString strKey ;		// The identify
		tString strValue ;		// The key's value
	}*PATTRIBUTE, ATTRIBUTE;
	/*
	Type defines the list of attributes.
	*/
	typedef std::list<ATTRIBUTE> ATTRIBUTELIST ;
	/*
	define the recipient list
	*/
	typedef std::list<tString> RECIPIENTLIST ;
	/*
	define the obligation data
	The name of Obligation
		strOBName: This is the name of obligation, it identifies the type obligation. An PA can impelment multi-obligaiton and a file also can have some.
	The list of attribute
		The second is the list of attribute. This data is corresponding to the obligation and back from the PDP.
	*/
	typedef struct _tagOBLIGATION
	{
		tString  strOBName ;	    // It is the identify string for the obligation.
		ATTRIBUTELIST attrList ;	// It is a list of attributes for the obligation.
	}*POBLIGATION, OBLIGATION ;
	/*
	type define the list of obligation
	*/
	typedef std::list< OBLIGATION > OBLIGATIONLIST ;

	/*
	type define the structure for the object 
	The name of the original object...
		The original object should be the source for the PEP...
	The name of source object
		strSrcName: It should be the full path name(Includint Path & File Name ).
		It is the file which the PA wants to implement.
	The temporary file name.
		strTempName: It the the temporary file folder or file.
	The result file name
		strRetName:	If the PA create a new file, (If the flag of  bFileNameChanged is true, this string represents the new file name( Full Path ) .
	The list of obligation
		obList	: It is the list of obligation which the object has.
	The result value for the object.
		lPARet: It is a long value. We can define some of the result for the object. 
		Such as: 0: Ok ; 1: Cancel ; 2: Removed.
	If the file name has been changed.
		It tells the PEP if the file name has been changed by the PA. The next PA should be pushed the result file name (strRetName).
	*/
	typedef struct _tagOBJECTINFO
	{	
		tString			strDisplayName ;	// This is the display name(such as: display on the file tagging panel.if it is a file, this file will be the origial file full path.) 
		tString			strSrcName ;		// 1.Source Object name,( if it is a file, this name should be full path name.
		tString			strTempName ;		// 2.Temporary or Destinating Object name .
		tString			strDestName;			// Destination Object name,
		wchar_t			strRetName[MAX_PATH+1]	;	// If bFileNameChanged is TRUE, this represents the new file name( including the path ) 
		OBLIGATIONLIST	obList ;			// Obligation list.
		ULONG			lPARet ;			// default:0 ; 1: removed.
		BOOL			bFileNameChanged ;	/*The temproary file name will become the source file for the next policy assist.*/
	}*POBJECTINFO,OBJECTINFO ;
	/*
	type define the list type of object.
	*/
	typedef std::list< OBJECTINFO > OBJECTINFOLIST ;
	/*
		type define the log function type...
	*/

	typedef void (*LogFunc)(PVOID lpLogCtx, 
							tString &wstrlogIdentifier,
							tString &wstrAssistantName, 
							tString &wstrAssistantOptions, 
							tString &wstrAssistantDescription, 
							tString &wstrAssistantUserActions, 
							ATTRIBUTELIST &optAttributes);

	/*
	Enum define the action
	*/
	typedef enum _tagACTION
	{
		AT_COPY,			// For the WDE
		AT_MOVE,			// For the WDE
		AT_SENDMAIL,		// For the OE/IE
		AT_INSERT,			// File Insert
		AT_UPLOAD,			// for the FTP,sharepoint
		AT_DOWNLOAD,		// for the FTP,sharepoint
		AT_READ,			// for visual labeling
		AT_PERSISTED,		// for visual labeling
		AT_PRINT,			// for visual labeling
		AT_SAVEAS,			// For the WDE
		AT_DEFAULT
	}*PACTION,ACTION ;
	/*
	define the structure of parameter for the policy assist.
	*/
	typedef struct _tagPA_PARAM
	{
		tString			strSender	;		// The mail-sender.
		OBJECTINFOLIST  objList ;
		RECIPIENTLIST	recipList ;			// The list of recipient
		ACTION		_action ;				// The action which is done by the user.
		BOOL		_bIsLastPA	;			// The PEP should tell the PA if the current is last one.
		tString		_strLastButtonName ;	// The PEP should tell the PA the button name which show in the last dialog:such as "Send E-mail", or"OK"
		LogFunc		fLog	;				// The callback of log to sever.
		PVOID		lpLogCtx ;				// The pep manages this pointer's sturcture.
	}*PPA_PARAM ,PA_PARAM ;


	typedef LONG	PA_STATUS ;
	/*
	The error code should use the same copy.
	The value type defines as follows:
	*/
#define PA_SUCCESS			0 
#define PA_ERROR				-1
#define PA_USER_CANCEL      1

#define OEACTIONFORHSC_NAME L"OEActionForHSC"
#define OEACTIONFORHSC_FROCEDOING L"OEActionForHSC_ForceDoing"
#define OEACTIONFORHSC_FROCENOTDO L"OEActionForHSC_ForceNOTDO"
#define OEACTIONFORHSC_NOTHING    L"OEActionForHSC_NOTHING"

};
extern "C"
{
	/*
	PAPARAM
		[in]A pointer to the structure which is the PA¡¯s parameter.The item of the sturcture is the list of object right now.
	_hParentWnd
		[in] this window is the parent window for the message box and the dialog...
	*/
	PABase::PA_STATUS WINAPI DoPolicyAssistant(_In_ PABase::PA_PARAM &_iParam, _In_opt_ const HWND _hParentWnd = NULL, _In_ bool forceObligation = false ) ;

}
#endif
