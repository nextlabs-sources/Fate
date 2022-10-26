// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently,
// but are changed infrequently

#pragma once

#ifndef STRICT
#define STRICT
#endif

// Modify the following defines if you have to target a platform prior to the ones specified below.
// Refer to MSDN for the latest info on corresponding values for different platforms.
#ifndef WINVER				// Allow use of features specific to Windows XP or later.
#define WINVER 0x0501		// Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT 0x0502	// Change this to the appropriate value to target other versions of Windows.
#endif						

#ifndef _WIN32_WINDOWS		// Allow use of features specific to Windows 98 or later.
#define _WIN32_WINDOWS 0x0410 // Change this to the appropriate value to target Windows Me or later.
#endif

#ifndef _WIN32_IE			// Allow use of features specific to IE 6.0 or later.
#define _WIN32_IE 0x0600	// Change this to the appropriate value to target other versions of IE.
#endif

#define _ATL_APARTMENT_THREADED
#define _ATL_NO_AUTOMATIC_NAMESPACE

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// some CString constructors will be explicit


// Disable boost warning 6011
#pragma warning(disable : 4996 4584 4819) //4278 
#include "resource.h"
#pragma warning(push)
#pragma warning(disable : 6387 6386) // Disable warning C6387 in atlhost.h
#include <atlbase.h>
#pragma warning(pop)
#include <atlcom.h>
#include <string>
#include <vector>
#include <Sddl.h>
#include <WinSock2.h>

#pragma warning(push)
#pragma warning(disable: 6386)
#include <Ws2tcpip.h>
#pragma warning(pop)

#include <shlobj.h>

using namespace ATL;

/************************************************************************/
/* GLOBAL INCLUDE                                                       */
/************************************************************************/
#include "../common/log.h"
#include "eframework/platform/cesdk_obligations.hpp"
#include "CEsdk.h"
#include "resattrmgr.h"
/************************************************************************/
/* GLOBAL STRUCTURE                                                     */
/************************************************************************/
static enum MAILTYPE{NEWMAIL=0, REPLYMAIL, FWMAIL, EXISTINGMAIL, OTHERSEND};
static enum ITEM_TYPE {DEFAULT=0,MAIL_ITEM=1,APPOINTMENT_ITEM=2,TASK_ITEM = 3,TASK_REQUEST_ITEM=4,SHARE_ITEM=5,MEETING_ITEM=6,NOTE_ITEM=7,};
static enum _ext_AppType {apptUnknown=0, apptWord, apptExcel, apptPPT, apptOutlook};
static enum _ext_AttachType{attachWord=0, attachExcel, attachPPT, attachOthers};
#define MAX_SRC_PATH_LENGTH				1024
#define MAX_MAILADDR_LENGTH             512
#define TEMP_MAGIC_NAME_FMT             L"%s#TEMP&REAL#%s"
#define TEMP_MAGIC_NAME                 L"#TEMP&REAL#"
#define OBLIGATION_NAME_ID              L"CE_ATTR_OBLIGATION_NAME"
#define OBLIGATION_NUMVALUES_ID         L"CE_ATTR_OBLIGATION_NUMVALUES"
#define OBLIGATION_VALUE_ID             L"CE_ATTR_OBLIGATION_VALUE"

// Obligation Commands
#define OBLIGATION_HDR                  L"HDR"
#define OBLIGATION_PREPEND_BODY         L"PREPEND_BODY"
#define OBLIGATION_APEND_BODY           L"APEND_BODY"
#define OBLIGATION_APEND_KEY			L"Apend"
#define OBLIGATION_PREPEND_SUBJECT      L"PREPEND_SUBJECT"
#define OBLIGATION_TAG_MESSAGE          L"TAG_MESSAGE"
#define OBLIGATION_STORE_ATTACHMENTS    L"STORE_ATTACHMENTS"



#define OBLIGATION_INTEGRATED_RIGHTS_MANAGEMENT_AUTOMATIC    L"OUTLOOK_INTEGRATED_RIGHTS_MANAGEMENT_AUTOMATIC"
#define OBLIGATION_INTEGRATED_RIGHTS_MANAGEMENT_INTERACTIVE  L"OUTLOOK_INTEGRATED_RIGHTS_MANAGEMENT_INTERACTIVE"

#define OCP_REALPATH					L"CE_REALPATH"
#define OCP_TEMPPATH					L"CE_TEMPPATH"

#define CE_SYSTEMFOLDER					L"CEOutlookAddin"
#define CE_OL_TEMP_FOLDER				L"CE_OLTEMP_FOLDER"

#define WORD_INSPECT_0					L"Comments, Revisions, Versions and Annotations"
#define WORD_INSPECT_1					L"Document Properties And Personal Information"
#define EXEC_INSPECT_0					L"Comments and Annotations"
#define EXEC_INSPECT_1					L"Document Properties And Personal Information"
#define PWPT_INSPECT_0					L"Comments and Annotations"
#define PWPT_INSPECT_1					L"Document Properties And Personal Information"

#define DEST_CLASS_NAME     L"rctrl_renwnd32"
#define DEST_WND_STYLE      0x02cf0000
#define DEST_WND_EX_STYLE   0x00040000

// Should we create office instance every time?
#define CREATE_OFFICE_INSTANCE_EVERYTIME	1


typedef struct _AttachmentData
{
    _ext_AttachType type;
    wchar_t         dispname[MAX_SRC_PATH_LENGTH];
    wchar_t         src[MAX_SRC_PATH_LENGTH];
	wchar_t			resolved[MAX_SRC_PATH_LENGTH];
    wchar_t         temp[MAX_SRC_PATH_LENGTH];
	/*
	added by chellee on 1-9-2008 for the extension of encryption
	*/
	wchar_t			Origtemp[MAX_SRC_PATH_LENGTH]	;

    // Domain Mismatch obligation
    BOOL            dm;
    wchar_t         dmUrl[MAX_SRC_PATH_LENGTH];
    wchar_t         dmRecipients[1024];///size needs to be dynamic
    wchar_t         dmClientName[1024];///size needs to be dynamic

    // Multiple Client obligation
    BOOL            mc;
    wchar_t         mcUrl[MAX_SRC_PATH_LENGTH];
    wchar_t         mcRecipients[1024];///size needs to be dynamic

    // Missing Tag obligation
    BOOL            mt;
    wchar_t         mtUrl[MAX_SRC_PATH_LENGTH];
    wchar_t         mtPropertyName[1024];   // Name of custom property for
                                            // client ID
    wchar_t         mtClientNames[1024];///size needs to be dynamic
    wchar_t         mtClientIds[1024];///size needs to be dynamic
    wchar_t         mtNotClientRelatedId[1024];///size needs to be dynamic
    wchar_t         mtClientIdChosen[1024];///size needs to be dynamic

    // Internal Use Only obligation
    BOOL            iuo;
    wchar_t         iuoUrl[MAX_SRC_PATH_LENGTH];

    // External Recipient obligation
    BOOL            er;
    wchar_t         erUrl[MAX_SRC_PATH_LENGTH];
    wchar_t         erRecipients[1024];///size needs to be dynamic

    // Hidden Data Removal obligation
    BOOL            hdr;
    wchar_t         hdrUrl[MAX_SRC_PATH_LENGTH];

    // Log Decision obligation
    BOOL            ld;
    wchar_t         ldCookie[1024];///size needs to be dynamic

	// Check Recipient License obligation
	BOOL            rl;
	wchar_t         rlUrl[MAX_SRC_PATH_LENGTH];
	wchar_t         rlRecipients[4096];///size needs to be dynamic


	INT			iIndex;//the index in attachments collection object.

	BOOL		bIgnored;//determine if the current attachment is ignored
}AttachmentData, *LpAttachmentData;
typedef struct _tagUPDATEITEM
{
   std::wstring _strTempSrcName ;
   BOOL _isChanged ;
}UPDATEITEM, *PUPDATEITEM ;
typedef std::vector< UPDATEITEM > ListUpdate ;
typedef std::vector<LpAttachmentData>   ATTACHMENTLIST;
typedef std::vector<std::wstring>       STRINGLIST;
//////////////////////////////////////////////////////////////////////////
#ifndef __RESATTR_FP__
#define __RESATTR_FP__
typedef int (*CreateAttributeManagerType)(ResourceAttributeManager **mgr);
typedef int (*AllocAttributesType)(ResourceAttributes **attrs);
typedef int (*ReadResourceAttributesWType)(ResourceAttributeManager *mgr, const WCHAR *filename, ResourceAttributes *attrs);
typedef int (*GetAttributeCountType)(const ResourceAttributes *attrs);
typedef void (*FreeAttributesType)(ResourceAttributes *attrs);
typedef void (*CloseAttributeManagerType)(ResourceAttributeManager *mgr);
typedef void (*AddAttributeWType)(ResourceAttributes *attrs, const WCHAR *name, const WCHAR *value);
typedef const WCHAR *(*GetAttributeNameType)(const ResourceAttributes *attrs, int index);
typedef const WCHAR * (*GetAttributeValueType)(const ResourceAttributes *attrs, int index);
typedef int (*WriteResourceAttributesWType)(ResourceAttributeManager *mgr, const WCHAR *filename, ResourceAttributes *attrs);
typedef bool (*IsNxlFormatType)(const WCHAR *name);

typedef int (*ReadResrcSummaryAttrType)(ResourceAttributeManager *mgr, const WCHAR *filename, ResourceAttributes *attrs);

extern CreateAttributeManagerType glfCreateAttributeManager ;
extern AllocAttributesType glfAllocAttributes ;
extern ReadResourceAttributesWType glfReadResourceAttributesW ;
extern GetAttributeCountType glfGetAttributeCount ;
extern FreeAttributesType glfFreeAttributes ;
extern CloseAttributeManagerType glfCloseAttributeManager;
extern AddAttributeWType glfAddAttributeW ;
extern GetAttributeNameType glfGetAttributeName ;
extern GetAttributeValueType glfGetAttributeValue;
extern WriteResourceAttributesWType glfWriteResourceAttributesW ;
extern IsNxlFormatType glfIsNxlFormat ;
extern ReadResrcSummaryAttrType glfReadResrcSummaryAttr ;

#endif //__RESATTR_FP__
/************************************************************************/
/* Add Office Import FOR OFFICE 2007                                    */
/************************************************************************/
#ifdef WSO2K7
#include "../import/2k7/mso.tlh"
using namespace Office;

#include "../import/2k7/msaddndr.tlh"

#include "../import/2k7/vbe6ext.tlh"
using namespace VBE6;

#include "../import/2k7/msword.tlh"
//using namespace Word;

#include "../import/2k7/excel.tlh"
using namespace Excel;

#include "../import/2k7/msppt.tlh"
using namespace PPT;

#include "../import/2k7/msoutl.tlh"
using namespace Outlook;
#endif


/************************************************************************/
/* Add Office Import FOR OFFICE 2003                                    */
/************************************************************************/
#ifdef WSO2K3
#include "../import/2k3/mso.tlh"
using namespace Office;

#include "../import/2k3/msaddndr.tlh"
#include "../import/2k3/vbe6ext.tlh"
using namespace VBE6;

#include "../import/2k3/msword.tlh"
//using namespace Word;

#include "../import/2k3/excel.tlh"
using namespace Excel;

#include "../import/2k3/msppt.tlh"
using namespace PPT;

#include "../import/2k3/msoutl.tlh"
using namespace Outlook;
#endif

/************************************************************************/


#ifdef WSO2K10
#include "../import/2010/mso.tlh"
using namespace Office;
#include "../import/2010/msaddndr.tlh"
#include "../import/2010/msoutl.tlh"
using namespace Outlook;
#endif

#ifdef WSO2K13
#include "../import/2010/mso.tlh"
using namespace Office;
#include "../import/2013/msaddndr.tlh"
#include "../import/2013/msoutl.tlh"
using namespace Outlook;

#endif

#ifdef WSO2K16
#include "../import/2016/mso.tlh"
using namespace Office;
#include "../import/2016/msaddndr.tlh"
#include "../import/2016/msoutl.tlh"
using namespace Outlook;
#endif

#include "../import/2010/vbe6ext.tlh"
using namespace VBIDE;
#include "../import/2010/msword.tlh"


#include "../outlook/ConfigManager.h"

//#include <DispatchMessage.h>
