
#ifndef __NXRM_RESOURCE_H__
#define __NXRM_RESOURCE_H__

/* --------------------------------------------------------
 HEADER SECTION
*/



/* ------------------------------------------------------------------
 MESSAGE DEFINITION SECTION
*/




/***************************************
  GENERAL
 ***************************************/


//
//  Values are 32 bit values laid out as follows:
//
//   3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
//   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
//  +---+-+-+-----------------------+-------------------------------+
//  |Sev|C|R|     Facility          |               Code            |
//  +---+-+-+-----------------------+-------------------------------+
//
//  where
//
//      Sev - is the severity code
//
//          00 - Success
//          01 - Informational
//          10 - Warning
//          11 - Error
//
//      C - is the Customer code flag
//
//      R - is a reserved bit
//
//      Facility - is the facility code
//
//      Code - is the facility's status code
//
//
// Define the facility codes
//
#define FACILITY_SYSTEM                  0x0
#define FACILITY_STUBS                   0x3
#define FACILITY_RUNTIME                 0x2
#define FACILITY_IO_ERROR_CODE           0x4


//
// Define the severity codes
//
#define STATUS_SEVERITY_WARNING          0x2
#define STATUS_SEVERITY_SUCCESS          0x0
#define STATUS_SEVERITY_INFORMATIONAL    0x1
#define STATUS_SEVERITY_ERROR            0x3


//
// MessageId: IDS_PRODUCT_NAME
//
// MessageText:
//
// NextLabs Rights Management%0
//
#define IDS_PRODUCT_NAME                 ((long)0x00000000L)





/***************************************
  RIGHTS
 ***************************************/


//
// MessageId: IDS_RIGHT_VIEW
//
// MessageText:
//
// View%0
//
#define IDS_RIGHT_VIEW                   ((long)0x00000001L)

//
// MessageId: IDS_RIGHT_EDIT
//
// MessageText:
//
// Edit%0
//
#define IDS_RIGHT_EDIT                   ((long)0x00000002L)

//
// MessageId: IDS_RIGHT_PRINT
//
// MessageText:
//
// Print%0
//
#define IDS_RIGHT_PRINT                  ((long)0x00000003L)

//
// MessageId: IDS_RIGHT_COPY
//
// MessageText:
//
// Copy%0
//
#define IDS_RIGHT_COPY                   ((long)0x00000004L)

//
// MessageId: IDS_RIGHT_EXTRACT
//
// MessageText:
//
// Extract%0
//
#define IDS_RIGHT_EXTRACT                ((long)0x00000005L)

//
// MessageId: IDS_RIGHT_ANNOTATE
//
// MessageText:
//
// Annotate%0
//
#define IDS_RIGHT_ANNOTATE               ((long)0x00000006L)

//
// MessageId: IDS_RIGHT_DECRYPT
//
// MessageText:
//
// Remove Protection%0
//
#define IDS_RIGHT_DECRYPT                ((long)0x00000007L)

//
// MessageId: IDS_RIGHT_SCREENCAP
//
// MessageText:
//
// Screen Capture%0
//
#define IDS_RIGHT_SCREENCAP              ((long)0x00000008L)

//
// MessageId: IDS_RIGHT_SEND
//
// MessageText:
//
// Send%0
//
#define IDS_RIGHT_SEND                   ((long)0x00000009L)

//
// MessageId: IDS_RIGHT_CONVERT
//
// MessageText:
//
// Convert%0
//
#define IDS_RIGHT_CONVERT                ((long)0x0000000AL)

//
// MessageId: IDS_RIGHT_CLASSIFY
//
// MessageText:
//
// Classify%0
//
#define IDS_RIGHT_CLASSIFY               ((long)0x0000000BL)

//
// MessageId: IDS_RIGHT_ASSIGNRIGHTS
//
// MessageText:
//
// Assign Right%0
//
#define IDS_RIGHT_ASSIGNRIGHTS           ((long)0x0000000CL)

//
// MessageId: IDS_RIGHT_EDITRIGHTS
//
// MessageText:
//
// Edit Right%0
//
#define IDS_RIGHT_EDITRIGHTS             ((long)0x0000000DL)





/***************************************
  OPERATION
 ***************************************/


//
// MessageId: IDS_OPERATION_DEFAULT
//
// MessageText:
//
// access%0
//
#define IDS_OPERATION_DEFAULT            ((long)0x0000000EL)

//
// MessageId: IDS_OPERATION_SAVE
//
// MessageText:
//
// edit%0
//
#define IDS_OPERATION_SAVE               ((long)0x0000000FL)

//
// MessageId: IDS_OPERATION_OVERWRITE
//
// MessageText:
//
// save a copy of%0
//
#define IDS_OPERATION_OVERWRITE          ((long)0x00000010L)

//
// MessageId: IDS_OPERATION_PRINT
//
// MessageText:
//
// print%0
//
#define IDS_OPERATION_PRINT              ((long)0x00000011L)

//
// MessageId: IDS_OPERATION_OPEN
//
// MessageText:
//
// view%0
//
#define IDS_OPERATION_OPEN               ((long)0x00000012L)

//
// MessageId: IDS_OPERATION_INSERT
//
// MessageText:
//
// insert%0
//
#define IDS_OPERATION_INSERT             ((long)0x00000013L)

//
// MessageId: IDS_OPERATION_EMAIL
//
// MessageText:
//
// email%0
//
#define IDS_OPERATION_EMAIL              ((long)0x00000014L)

//
// MessageId: IDS_OPERATION_EXPORT
//
// MessageText:
//
// export%0
//
#define IDS_OPERATION_EXPORT             ((long)0x00000015L)





/***************************************
  NOTIFICATION
 ***************************************/


//
// MessageId: IDS_NOTIFY_USER_LOGON
//
// MessageText:
//
// User %%s has logged in%0
//
#define IDS_NOTIFY_USER_LOGON            ((long)0x00000016L)

//
// MessageId: IDS_NOTIFY_USER_LOGON_FAILURE
//
// MessageText:
//
// User %%s was unable to log in due to error %%d (%%s)%0
//
#define IDS_NOTIFY_USER_LOGON_FAILURE    ((long)0x00000017L)

//
// MessageId: IDS_NOTIFY_LOGON_ERROR_EMPTY_NAME
//
// MessageText:
//
// empty user name%0
//
#define IDS_NOTIFY_LOGON_ERROR_EMPTY_NAME ((long)0x00000018L)

//
// MessageId: IDS_NOTIFY_LOGON_ERROR_EMPTY_PASSWORD
//
// MessageText:
//
// empty password%0
//
#define IDS_NOTIFY_LOGON_ERROR_EMPTY_PASSWORD ((long)0x00000019L)

//
// MessageId: IDS_NOTIFY_LOGON_ERROR_INVALID_PASSWORD
//
// MessageText:
//
// wrong password%0
//
#define IDS_NOTIFY_LOGON_ERROR_INVALID_PASSWORD ((long)0x0000001AL)

//
// MessageId: IDS_NOTIFY_LOGON_ERROR_NOT_AUTHENTICATED
//
// MessageText:
//
// authentication failed - wrong user name or password%0
//
#define IDS_NOTIFY_LOGON_ERROR_NOT_AUTHENTICATED ((long)0x0000001BL)

//
// MessageId: IDS_NOTIFY_LOGON_ERROR_INVALID_DOMAIN
//
// MessageText:
//
// unrecognized domain name%0
//
#define IDS_NOTIFY_LOGON_ERROR_INVALID_DOMAIN ((long)0x0000001CL)

//
// MessageId: IDS_NOTIFY_LOGON_ERROR_UNKNOWN
//
// MessageText:
//
// unknown error%0
//
#define IDS_NOTIFY_LOGON_ERROR_UNKNOWN   ((long)0x0000001DL)

//
// MessageId: IDS_NOTIFY_LOGON_REQUIRED
//
// MessageText:
//
// This document is protected by NextLabs Rights Management.%nTo access this document, you must log in.%0
//
#define IDS_NOTIFY_LOGON_REQUIRED        ((long)0x0000001EL)

//
// MessageId: IDS_NOTIFY_USER_LOGOUT
//
// MessageText:
//
// User (%%s) has logged out%0
//
#define IDS_NOTIFY_USER_LOGOUT           ((long)0x0000001FL)

//
// MessageId: IDS_NOTIFY_LOGON_EXPIRED
//
// MessageText:
//
// Current session expired, please log in again.%0
//
#define IDS_NOTIFY_LOGON_EXPIRED         ((long)0x00000020L)

//
// MessageId: IDS_NOTIFY_AUTHORIZED
//
// MessageText:
//
// Rights granted:%0
//
#define IDS_NOTIFY_AUTHORIZED            ((long)0x00000021L)

//
// MessageId: IDS_NOTIFY_AUTHORIZE_FAILURE
//
// MessageText:
//
// You are not authorized to access this file%0
//
#define IDS_NOTIFY_AUTHORIZE_FAILURE     ((long)0x00000022L)

//
// MessageId: IDS_NOTIFY_OPERATION_DENIED
//
// MessageText:
//
// You are not authorized to %%s this file (%%s)%0
//
#define IDS_NOTIFY_OPERATION_DENIED      ((long)0x00000023L)

//
// MessageId: IDS_NOTIFY_OPERATION_SAVEAS_DENIED
//
// MessageText:
//
// This operation is not supported. File (%%s) cannot be saved to the specified location%0
//
#define IDS_NOTIFY_OPERATION_SAVEAS_DENIED ((long)0x00000024L)

//
// MessageId: IDS_NOTIFY_ADOBE_PLUGIN_NOT_READY
//
// MessageText:
//
// Rights Management for Adobe is not ready, please wait...%0
//
#define IDS_NOTIFY_ADOBE_PLUGIN_NOT_READY ((long)0x00000025L)

//
// MessageId: IDS_NOTIFY_READONLY_MODE
//
// MessageText:
//
// You are not authorized to edit this file (%%s)%0
//
#define IDS_NOTIFY_READONLY_MODE         ((long)0x00000026L)

//
// MessageId: IDS_NOTIFY_UPDATING_POLICY
//
// MessageText:
//
// Checking for latest policy...%0
//
#define IDS_NOTIFY_UPDATING_POLICY       ((long)0x00000027L)

//
// MessageId: IDS_NOTIFY_POLICY_UPDATED
//
// MessageText:
//
// Policy updated%0
//
#define IDS_NOTIFY_POLICY_UPDATED        ((long)0x00000028L)

//
// MessageId: IDS_NOTIFY_REGISTERING
//
// MessageText:
//
// Registering to Rights Management Server...%0
//
#define IDS_NOTIFY_REGISTERING           ((long)0x00000029L)

//
// MessageId: IDS_NOTIFY_REGISTERED
//
// MessageText:
//
// Registeration to Rights Management Server completed%0
//
#define IDS_NOTIFY_REGISTERED            ((long)0x0000002AL)

//
// MessageId: IDS_NOTIFY_DBGLOG_COLLECTING
//
// MessageText:
//
// Collecting debug logs...%0
//
#define IDS_NOTIFY_DBGLOG_COLLECTING     ((long)0x0000002BL)

//
// MessageId: IDS_NOTIFY_DBGLOG_COLLECTED
//
// MessageText:
//
// A zip file (%%s) containing the debug logs has been saved to your desktop%0
//
#define IDS_NOTIFY_DBGLOG_COLLECTED      ((long)0x0000002CL)

//
// MessageId: IDS_NOTIFY_DBGLOG_COLLECT_FAILED
//
// MessageText:
//
// Unable to collect debug logs%0
//
#define IDS_NOTIFY_DBGLOG_COLLECT_FAILED ((long)0x0000002DL)





/***************************************
  UI: GENERAL
 ***************************************/


//
// MessageId: IDS_BUTTON_OK
//
// MessageText:
//
// OK%0
//
#define IDS_BUTTON_OK                    ((long)0x0000002EL)

//
// MessageId: IDS_BUTTON_CANCEL
//
// MessageText:
//
// Cancel%0
//
#define IDS_BUTTON_CANCEL                ((long)0x0000002FL)

//
// MessageId: IDS_BUTTON_APPLY
//
// MessageText:
//
// Apply%0
//
#define IDS_BUTTON_APPLY                 ((long)0x00000030L)





/***************************************
  UI: Context Menu
 ***************************************/


//
// MessageId: IDS_MENU_PROTECT
//
// MessageText:
//
// Protect...%0
//
#define IDS_MENU_PROTECT                 ((long)0x00000031L)

//
// MessageId: IDS_MENU_REMOVEPROTECT
//
// MessageText:
//
// Remove Protection%0
//
#define IDS_MENU_REMOVEPROTECT           ((long)0x00000032L)

//
// MessageId: IDS_MENU_CHECKPERMISSION
//
// MessageText:
//
// Check Permissions...%0
//
#define IDS_MENU_CHECKPERMISSION         ((long)0x00000033L)

//
// MessageId: IDS_MENU_CLASSIFY
//
// MessageText:
//
// Classify...%0
//
#define IDS_MENU_CLASSIFY                ((long)0x00000034L)

//
// MessageId: IDS_MENU_HELP
//
// MessageText:
//
// Help%0
//
#define IDS_MENU_HELP                    ((long)0x00000035L)





/***************************************
  UI: TRAY
 ***************************************/


//
// MessageId: IDS_TRAY_TIP
//
// MessageText:
//
// NextLabs Rights Management%nPolicy: %%s%nUser: %%s%0
//
#define IDS_TRAY_TIP                     ((long)0x00000036L)

//
// MessageId: IDS_TRAY_MENU_OPEN_RMC
//
// MessageText:
//
// NextLabs Rights Management...%0
//
#define IDS_TRAY_MENU_OPEN_RMC           ((long)0x00000037L)

//
// MessageId: IDS_TRAY_MENU_UPDATE_POLICY
//
// MessageText:
//
// Update Rights Management Policy%0
//
#define IDS_TRAY_MENU_UPDATE_POLICY      ((long)0x00000038L)

//
// MessageId: IDS_TRAY_MENU_ENABLE_DEBUG
//
// MessageText:
//
// Enable Debug%0
//
#define IDS_TRAY_MENU_ENABLE_DEBUG       ((long)0x00000039L)

//
// MessageId: IDS_TRAY_MENU_DISABLE_DEBUG
//
// MessageText:
//
// Disable Debug%0
//
#define IDS_TRAY_MENU_DISABLE_DEBUG      ((long)0x0000003AL)

//
// MessageId: IDS_TRAY_MENU_COLLECT_LOGS
//
// MessageText:
//
// Collect Debug Logs...%0
//
#define IDS_TRAY_MENU_COLLECT_LOGS       ((long)0x0000003BL)

//
// MessageId: IDS_TRAY_MENU_HELP
//
// MessageText:
//
// Help...%0
//
#define IDS_TRAY_MENU_HELP               ((long)0x0000003CL)

//
// MessageId: IDS_TRAY_MENU_ABOUT
//
// MessageText:
//
// About...%0
//
#define IDS_TRAY_MENU_ABOUT              ((long)0x0000003DL)

//
// MessageId: IDS_TRAY_MENU_LOGIN
//
// MessageText:
//
// Log in...%0
//
#define IDS_TRAY_MENU_LOGIN              ((long)0x0000003EL)

//
// MessageId: IDS_TRAY_MENU_LOGOUT
//
// MessageText:
//
// Log Out%0
//
#define IDS_TRAY_MENU_LOGOUT             ((long)0x0000003FL)

//
// MessageId: IDS_TRAY_MENU_EXIT
//
// MessageText:
//
// Exit%0
//
#define IDS_TRAY_MENU_EXIT               ((long)0x00000040L)





/***************************************
  UI: MAIN
 ***************************************/


//
// MessageId: IDS_MAIN_TITLE
//
// MessageText:
//
// RIGHTS MANAGEMENT%0
//
#define IDS_MAIN_TITLE                   ((long)0x00000041L)

//
// MessageId: IDS_MAIN_STATE_CONNECTED
//
// MessageText:
//
// Connected%0
//
#define IDS_MAIN_STATE_CONNECTED         ((long)0x00000042L)

//
// MessageId: IDS_MAIN_STATE_DISCONNECTED
//
// MessageText:
//
// Disconnected%0
//
#define IDS_MAIN_STATE_DISCONNECTED      ((long)0x00000043L)

//
// MessageId: IDS_MAIN_LABEL_POLICY
//
// MessageText:
//
// Policy%0
//
#define IDS_MAIN_LABEL_POLICY            ((long)0x00000044L)

//
// MessageId: IDS_MAIN_LABEL_LAST_UPDATE
//
// MessageText:
//
// Last updated on%0
//
#define IDS_MAIN_LABEL_LAST_UPDATE       ((long)0x00000045L)

//
// MessageId: IDS_MAIN_CLOSE_TIP
//
// MessageText:
//
// Close Window%0
//
#define IDS_MAIN_CLOSE_TIP               ((long)0x00000046L)

//
// MessageId: IDS_MAIN_REFRESH_TIP
//
// MessageText:
//
// Update Policy%0
//
#define IDS_MAIN_REFRESH_TIP             ((long)0x00000047L)

//
// MessageId: IDS_MAIN_SETTING_TIP
//
// MessageText:
//
// Settings%0
//
#define IDS_MAIN_SETTING_TIP             ((long)0x00000048L)

//
// MessageId: IDS_MAIN_POPUP_LABEL
//
// MessageText:
//
// Show user notifications%0
//
#define IDS_MAIN_POPUP_LABEL             ((long)0x00000049L)





/***************************************
  UI: PERMISSION DIALOG
 ***************************************/


//
// MessageId: IDS_PROPDLG_TAB_PERMISSIONS
//
// MessageText:
//
// Permissions%0
//
#define IDS_PROPDLG_TAB_PERMISSIONS      ((long)0x0000004AL)

//
// MessageId: IDS_PROPDLG_LABEL_DOC
//
// MessageText:
//
// Document%0
//
#define IDS_PROPDLG_LABEL_DOC            ((long)0x0000004BL)

//
// MessageId: IDS_PROPDLG_LABEL_PERMISSION
//
// MessageText:
//
// Permissions for current user%0
//
#define IDS_PROPDLG_LABEL_PERMISSION     ((long)0x0000004CL)

//
// MessageId: IDS_PROPDLG_COL_PERMISSIONS
//
// MessageText:
//
// Permissions%0
//
#define IDS_PROPDLG_COL_PERMISSIONS      ((long)0x0000004DL)

//
// MessageId: IDS_PROPDLG_COL_STATUS
//
// MessageText:
//
// Status%0
//
#define IDS_PROPDLG_COL_STATUS           ((long)0x0000004EL)





/***************************************
  UI: CLASSIFICATION DIALOG
 ***************************************/


//
// MessageId: IDS_PROPDLG_TAB_CLASSIFICATION
//
// MessageText:
//
// Classification%0
//
#define IDS_PROPDLG_TAB_CLASSIFICATION   ((long)0x0000004FL)

//
// MessageId: IDS_PROPDLG_LABEL_LOCATION
//
// MessageText:
//
// Location:%0
//
#define IDS_PROPDLG_LABEL_LOCATION       ((long)0x00000050L)

//
// MessageId: IDS_PROPDLG_LABEL_CLASSIFICATION
//
// MessageText:
//
// Classification Labels%0
//
#define IDS_PROPDLG_LABEL_CLASSIFICATION ((long)0x00000051L)

//
// MessageId: IDS_PROPDLG_COL_NAME
//
// MessageText:
//
// Name%0
//
#define IDS_PROPDLG_COL_NAME             ((long)0x00000052L)

//
// MessageId: IDS_PROPDLG_COL_VALUE
//
// MessageText:
//
// Value%0
//
#define IDS_PROPDLG_COL_VALUE            ((long)0x00000053L)





/***************************************
  UI: CLASSIFY DIALOG
 ***************************************/


//
// MessageId: IDS_CLASSIFYDLG_INFO
//
// MessageText:
//
// Classify the document you want to protect%0
//
#define IDS_CLASSIFYDLG_INFO             ((long)0x00000054L)





/***************************************
  ERROR MESSAGE
 ***************************************/


//
// MessageId: IDS_ERROR_INSUFFICIENT_DISK_SPACE
//
// MessageText:
//
// Insufficient disk space%0
//
#define IDS_ERROR_INSUFFICIENT_DISK_SPACE ((long)0xC0020000L)

//
// MessageId: IDS_MSG_UNRECOGNIZED_KEY_ID
//
// MessageText:
//
// File (%%s) has an unrecognized encryption key ID: %%s%0
//
#define IDS_MSG_UNRECOGNIZED_KEY_ID      ((long)0xC0020001L)

//
// MessageId: IDS_MSG_INVALID_KEY
//
// MessageText:
//
// Invalid encryption key%0
//
#define IDS_MSG_INVALID_KEY              ((long)0xC0020002L)

//
// MessageId: IDS_MSG_INCORRECT_SIGNATURE
//
// MessageText:
//
// Incorrect signature%0
//
#define IDS_MSG_INCORRECT_SIGNATURE      ((long)0xC0020003L)

//
// MessageId: IDS_MSG_NO_SIGNATURE
//
// MessageText:
//
// No signature%0
//
#define IDS_MSG_NO_SIGNATURE             ((long)0xC0020004L)




#endif    // #ifndef __NXRM_RESOURCE_H__