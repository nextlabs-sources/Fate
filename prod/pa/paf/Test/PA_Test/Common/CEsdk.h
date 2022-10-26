//
// All sources, binaries and HTML pages (C) copyright 2007 by NextLabs Inc. 
// San Mateo CA, Ownership remains with NextLabs Inc, 
// All rights reserved worldwide. 
//
//
// NextLabs Compliant Enterprise SDK header file
// 
// <-------------------------------------------------------------------------->

#ifndef CESDK_H
#define CESDK_H

/* All the constants */
enum {CE_INFINITE = -1};

/* All the types and structure definition */

/**
 * Result.
 */
typedef enum _CEResult_t {
  CE_RESULT_SUCCESS                             =  0,  /**< Success */
  CE_RESULT_GENERAL_FAILED                      = -1,  /**< General failure */
  CE_RESULT_CONN_FAILED                         = -2,  /**< Connection failed */
  CE_RESULT_INVALID_PARAMS                      = -3,  /**< Invalid parameter(s) */
  CE_RESULT_VERSION_MISMATCH                    = -4,  /**< Version mismatch */
  CE_RESULT_FILE_NOT_PROTECTED                  = -5,  /**< File not protected */
  CE_RESULT_INVALID_PROCESS                     = -6,  /**< Invalid process */
  CE_RESULT_INVALID_COMBINATION                 = -7,  /**< Invalid combination */
  CE_RESULT_PERMISSION_DENIED                   = -8,  /**< Permission denied */
  CE_RESULT_FILE_NOT_FOUND                      = -9,  /**< File not found */
  CE_RESULT_FUNCTION_NOT_AVAILBLE               = -10, /**< Function not available */
  CE_RESULT_TIMEDOUT                            = -11, /**< Timed out */
  CE_RESULT_SHUTDOWN_FAILED                     = -12, /**< Shutdown failed */
  CE_RESULT_INVALID_ACTION_ENUM                 = -13, /**< */
  CE_RESULT_EMPTY_SOURCE                        = -14, /**< Empty source */
  CE_RESULT_MISSING_MODIFIED_DATE               = -15, /**< */
  CE_RESULT_NULL_CEHANDLE                       = -16, /**< NULL or bad connection handle */
  CE_RESULT_INVALID_EVAL_ACTION                 = -17, /**< */
  CE_RESULT_EMPTY_SOURCE_ATTR                   = -18, /**< */
  CE_RESULT_EMPTY_ATTR_KEY                      = -19, /**< */
  CE_RESULT_EMPTY_ATTR_VALUE                    = -20, /**< */
  CE_RESULT_EMPTY_PORTAL_USER                   = -21, /**< */
  CE_RESULT_EMPTY_PORTAL_USERID                 = -22, /**< */
  CE_RESULT_MISSING_TARGET                      = -23, /**< Missing target */
  CE_RESULT_PROTECTION_OBJECT_NOT_FOUND         = -24, /**< Object not found */
  CE_RESULT_NOT_SUPPORTED                       = -25  /**< Not supported */
} CEResult_t;

/**
 * KeyRoot
 */
typedef enum _CEKeyRoot_t {
  CE_KEYROOT_CLASSES_ROOT                       = 0, /**< Classes root */
  CE_KEYROOT_CURRENT_USER                       = 1, /**< Current user */
  CE_KEYROOT_LOCAL_MACHINE                      = 2, /**< Local machine */
  CE_KEYROOT_USERS                              = 3, /**< Users */
  CE_KEYROOT_CURRENT_CONFIG                     = 4  /**< Current configuration */
} CEKeyRoot_t;

/**
 * Boolean
 */
typedef enum _CEBoolean {
  CEFalse                                       = 0, /**< False */
  CETrue                                        = 1  /**< True */
} CEBoolean;

/**
 * Action
 */
typedef enum _CEAction_t {
  CE_ACTION_READ                                = 1,  /**< Read */
  CE_ACTION_DELETE                              = 2,  /**< Delete */
  CE_ACTION_MOVE                                = 3,  /**< Move */
  CE_ACTION_COPY                                = 4,  /**< Copy */
  CE_ACTION_WRITE                               = 5,  /**< Write */
  CE_ACTION_RENAME                              = 6,  /**< Rename */
  CE_ACTION_CHANGE_ATTR_FILE                    = 7,  /**< Change attribute */
  CE_ACTION_CHANGE_SEC_FILE                     = 8,  /**< Change security */
  CE_ACTION_PRINT_FILE                          = 9,  /**< Print File */
  CE_ACTION_PASTE_FILE                          = 10, /**< Past File */
  CE_ACTION_EMAIL_FILE                          = 11, /**< EMail File */
  CE_ACTION_IM_FILE                             = 12, /**< Instance Messege File */
  CE_ACTION_EXPORT                              = 13, /**< Export */
  CE_ACTION_IMPORT                              = 14, /**< Import */
  CE_ACTION_CHECKIN                             = 15, /**< Checkin */
  CE_ACTION_CHECKOUT                            = 16, /**< Checkout */
  CE_ACTION_ATTACH                              = 17, /**< Attach */
  CE_ACTION_RUN                                 = 18, /**< Run */
  CE_ACTION_REPLY                               = 19, /**< Reply */
  CE_ACTION_FORWARD                             = 20, /**< Forward */
  CE_ACTION_NEW_EMAIL                           = 21, /**< New Email */
  CE_ACTION_AVD                                 = 22, /**< AVD */
  CE_ACTION_MEETING                             = 23, /**< Meeting */
  CE_ACTION_PROCESS_TERMINATE                   = 24  /**< Terminate Process.  This action is reserved
						           and should not be used. */
} CEAction_t;

/**
 * Policy evaluation response.
 */
typedef enum _CEResponse_t {
  CEDeny                                        = 0, /**< Action denied */
  CEAllow                                       = 1  /**< Action allowed */
} CEResponse_t;

/**
 * Noise Level
 */
typedef enum _CENoiseLevel_t {
  CE_NOISE_LEVEL_MIN                            = 0, /**< Minimum */
  CE_NOISE_LEVEL_SYSTEM                         = 1, /**< System */
  CE_NOISE_LEVEL_APPLICATION                    = 2, /**< Application */
  CE_NOISE_LEVEL_USER_ACTION                    = 3, /**< User Action */
  CE_NOISE_LEVEL_MAX                            = 4  /**< Maximum */
} CENoiseLevel_t;

/**
 * Attributes
 */
#define CE_ATTR_CREATE_TIME             _T("created_date")
#define CE_ATTR_LASTACCESS_TIME         _T("access_date")
#define CE_ATTR_LASTWRITE_TIME          _T("modified_date")
#define CE_ATTR_OWNER_ID                _T("owner")
#define CE_ATTR_GROUP_ID                _T("owner_group")
#define CE_ATTR_FULLPATH                _T("CE_ATTR_FULLPATH")
#define CE_ATTR_WINDOW_HANDLE           _T("CE_ATTR_WINDOW_HANDLE")
#define CE_ATTR_LASTACTION              _T("CE_ATTR_LASTACTION")
#define CE_ATTR_POLICY_LASTUPDATE_TIME  _T("CE_ATTR_POLICY_LASTUPDATE_TIME")
#define CE_ATTR_OBLIGATION_COUNT        _T("CE_ATTR_OBLIGATION_COUNT")
#define CE_ATTR_OBLIGATION_NAME         _T("CE_ATTR_OBLIGATION_NAME")
#define CE_ATTR_OBLIGATION_POLICY       _T("CE_ATTR_OBLIGATION_POLICY")
#define CE_ATTR_OBLIGATION_VALUE        _T("CE_ATTR_OBLIGATION_VALUE")
#define CE_OBLIGATION_NOTIFY            _T("CE::NOTIFY")

typedef struct _CEString * CEString;
typedef struct _CEHandle * CEHandle;

/* int will be platform dependent */
typedef int CEint32;

/**
 * Attribute
 */
struct CEAttribute {
  CEString key;    /**< Attribute key */
  CEString value;  /**< Attribute value */
};
typedef struct CEAttribute CEAttribute;

/**
 * Attributes
 */
typedef struct _CEAttributes {
  CEAttribute * attrs;  /**< Attributes */
  CEint32       count;  /**< Number of attributes */
} CEAttributes;

/**
 * Enforcement
 */
typedef struct _CEEnforcement_t {
  CEResponse_t result;       /**< Result of policy evaluation */
  CEAttributes *obligation;  /**< Obligation */
} CEEnforcement_t;

typedef struct _CEUser {
  CEString userName; /**< Display name of the user */
  CEString userID;   /**< ID of the user, SID on Windows, UID on Unix */
} CEUser;

typedef struct _CEApplication {
  CEString appName; /**< Display name of the application */
  CEString appPath; /**< Full path to the execuatable of the application */
  CEString appURL;  /**< The url that a web application visits. */
} CEApplication;


/* API definitions */

#ifdef __cplusplus
extern "C" {
#endif

/*! CECONN_Initialize
 *
 * \brief Initializes the connection between the client to the Policy Decision Point Server.
 * 
 * \param app (in)                 The application assoicate with the client PEP.
 * \param user (in)                A user in the application.
 * \param pdpHostName (in)         The PDP host name. When NULL localhost is used.
 * \param connectHandle (out)      Handle.
 * \param timeout_in_millisec (in) Desirable timeout in milliseconds for this evaluation.
 * 
 * \return Result of initialization.  Failure indicates the PDP server is not available.
 *
 * \sa CECONN_Close
 */ 
CEResult_t
CECONN_Initialize (CEApplication app, 
		   CEUser user, 
		   CEString pdpHostName,
		   CEHandle * connectHandle,
		   CEint32 timeout_in_millisec); 


/*! CECONN_Close
 *
 * \brief Close the connection between the client and the Policy Decision
 *        Point Server.
 * 
 * \param handle (in)              Connection handle from the CONN_initialize API.
 * \param timeout_in_millisec (in) Desirable timeout in milliseconds for this RPC.
 * 
 * \sa CECONN_Initialize
 */ 
CEResult_t
CECONN_Close (CEHandle handle, CEint32 timeout_in_millisec);


/*! CECONN_DLL_Activate
 *
 * \brief Activate the connection between the dll client to the Policy Decision
 *        Point Server.
 * 
 * \param app (in)                 The application assoicate with the client PEP
 * \param user (in)                A user in the application.
 * \param pdpHostName (in)         The PDP host name. If it is NULL, it is
 *                                 going to connect to the server on local machine.
 * \param connectHandle (in/out)
 * \param timeout_in_millisec (in) Desirable timeout in milliseconds 
 *                                 for this RPC
 *
 */ 
CEResult_t
CECONN_DLL_Activate(CEApplication app, 
		    CEUser user, 
		    CEString pdpHostName,
		    CEHandle * connectHandle,
		    CEint32 timeout_in_millisec);


/*! CECONN_DLL_Deactivate
 *
 * Deactivate the connection between the dll client and the Policy Decision
 * Point Server.
 * 
 * \param handle (in)              Connection handle from the CECONN_Initialize API.
 * \param timeout_in_millisec (in) Desirable timeout in milliseconds for this evaluation.
 *
 * \sa CECONN_DLL_Activate
 */ 
CEResult_t 
CECONN_DLL_Deactivate (CEHandle handle, CEint32 timeout_in_millisec);


/*! CEEVALUATE_CheckPortal
 *
 * \brief Ask the Policy Decision Point Server to evaluate the operation on 
 *        portal resource.
 *
 * \param handle (in)              Handle from the CECONN_Initialize.
 * \param operation (in)           Operation on the file.
 * \param sourceURL (in)           The url of the source resource.
 * \param targetURL (in)           The url of the target resource.
 * \param sourceAttributes (in)    Associate attributes of the source.
 * \param targetAttributes (in)    Associate attributes of the target.
 * \param ipNumber (in)            The IP address of client machine.
 * \param user (in)                The user who access this URL.
 * \param performObligation (in)   Perform the obligation defined by the policy.
 * \param noiseLevel (in)          Desirable noise level to be used for this evaluation.
 * \param enforcement (out)        Result of the policy for enforcement.
 * \param timeout_in_millisec (in) Desirable timeout in milliseconds for this evaluation.
 *
 * \return Result of policy evaluation.
 *
 */ 
CEResult_t CEEVALUATE_CheckPortal(CEHandle handle, 
				  CEAction_t operation, 
				  CEString sourceURL, 
				  CEAttributes * sourceAttributes,
				  CEString targetURL,
				  CEAttributes * targetAttributes,
				  CEint32 ipNumber,
				  CEUser  *user,
				  CEBoolean performObligation,
				  CENoiseLevel_t noiseLevel,
				  CEEnforcement_t * enforcement,
				  CEint32 timeout_in_millisec);


/*! CEEVALUATE_CheckFile
 *
 * \brief Ask the Policy Decision Point Server to evaluate the operation on 
 *        portal resource.
 * 
 * \param handle (in)              Handle from the CECONN_Initialize.
 * \param operation (in)           Operation on the file.
 * \param sourceFullFileName (in)  The full name of the source resource
 * \param targetFullFileName (in)  The full name of the target resource
 * \param targetAttributes (in)    Associate attributes of the target.
 * \param performObligation (in)   Perform the obligation defined by the policy 
 *                                 (e.g. logging / email)
 * \param sourceAttributes (in)    Associate attributes of the source.  The key "modified_date"
 *                                 must be present.
 * \param noiseLevel (in)          Desirable noise level to be used for this evaluation.
 * \param ipNumber (in)            The IP address of client machine.
 * \param user (in)                The user who access this file.
 * \param app (in)                 The application which access this file
 * \param enforcement (in)         Result of the policy for enforcement.
 * \param timeout_in_millisec (in) Desirable timeout in milliseconds for 
 *                                 this evaluation.
 *
 * \return Result of policy evaluation.
 */ 
CEResult_t CEEVALUATE_CheckFile(CEHandle handle, 
				CEAction_t operation, 
				CEString sourceFullFileName, 
				CEAttributes * sourceAttributes,
				CEString targetFullFileName,
				CEAttributes * targetAttributes,
				CEint32 ipNumber,
				CEUser  *user,
				CEApplication *app,
				CEBoolean performObligation,
				CENoiseLevel_t noiseLevel,
				CEEnforcement_t * enforcement,
				CEint32 timeout_in_millisec);


/* !CEEVALUATE_CheckMessageAttachment()
 *
 * Ask the Policy Decision Point Server to evaluate the operation on 
 * sending message with attchment
 * handle (in)                     Handle from the CONN_Initialize()
 * operation (in)                  Operation on the message
 * sourceFullFileName (in)         The full name of the source resource
 * numRecipients (in)              The number of message recipients
 * recipients (in)                 The string array of message recipients
 * performObligation (in)          Perform the obligation defined by the policy 
 *                                 (e.g. logging / email)
 * \param sourceAttributes (in)    Associate attributes of the source
 * \param noiseLevel (in)          Desirable noise level to be used for this evaluation
 * \param ipNumber (in)            The ip address of client machine
 * \param user (in)                The user who access this file
 * \param app (in)                 The application which send the message
 * \param timeout_in_millisec (in) Desirable timeout in milliseconds for 
 *                                 this evaluation
 * Enforcement (out)               Result of the policy for enforcement
 */ 
CEResult_t 
CEEVALUATE_CheckMessageAttachment(CEHandle handle, 
  				  CEAction_t operation, 
  				  CEString sourceFullFileName, 
  				  CEAttributes * sourceAttributes,
  				  CEint32 numRecipients,
  				  CEString *recipients,
  				  CEint32 ipNumber,
  				  CEUser  *user,
  				  CEApplication *app,
 				  CEBoolean performObligation,
  				  CENoiseLevel_t noiseLevel,
  				  CEEnforcement_t * enforcement,
  				  CEint32 timeout_in_millisec);

/*! CEEVALUATE_FreeEnforcement
 *
 * \brief Basic service to free a CE-type CEEnforcement_t from the system.
 *
 * \param enforcement CE-type CEEnforcement_t to be freed.
 */ 
CEResult_t
CEEVALUATE_FreeEnforcement(CEEnforcement_t enforcement);


/*! CEM_AllocateString
 *
 * \brief Basic service to create a CE-type string from the system
 *
 * \param str Pointer to the original C standard string 
 *
 * \sa CEM_FreeString
 */ 
#if defined (Linux) || defined (Darwin)
CEString
CEM_AllocateString (const char * str);
#endif 

#if defined (WIN32) || defined (_WIN64)
CEString
CEM_AllocateString (const TCHAR * str);
#endif 


/*! CEM_FreeString
 *
 * \brief Basic service to free a CE-type string from the system
 *
 * \param cestr CE-type string to be freed
 *
 * \sa CEM_AllocateString
 */ 
CEResult_t
CEM_FreeString (CEString cestr);


/*! CEM_ReallocateString
 *
 * \brief Basic service to reallocate the existing string to a new string
 *
 * \param cestr CE-type string to be freed
 *
 */ 
#if defined (Linux) || defined (Darwin)
CEResult_t
CEM_ReallocateString (CEString cestr, const char * newstr);
#endif

#if defined (WIN32) || defined (_WIN64)
CEResult_t
CEM_ReallocateString (CEString cestr, const TCHAR * newstr);
#endif


/*! CEM_GetString
 *
 * \brief Basic service to retrieve the standard C string from the CE-type string
 *
 * \param cestr CE-type string to be retrieved
 *
 */ 
#if defined (Linux) || defined (Darwin)
const char * 
CEM_GetString (CEString cestr);
#endif
 
#if defined (WIN32) || defined (_WIN64)
const TCHAR *
CEM_GetString (CEString cestr);
#endif

/* ------------------------------------------------------------------------
 * CEPROTECT_LockKey()
 *
 * Protect a registry key from being modified by other process 
 * 
 * Arguments : handle     : connection handle from the CONN_initialize API
 * 
 *             root       : Root of the registry group
 *
 *             key        : Key to be protected in the format of 
 *                          level1\\level2\\level3\\key
 * 
 * Note      : Applicable only to Window platform
 * ------------------------------------------------------------------------
 */ 


CEResult_t
CEPROTECT_LockKey (CEHandle handle, CEKeyRoot_t root, CEString key);

/* ------------------------------------------------------------------------
 * CEPROTECT_UnlockKey()
 *
 * Undo the protect of a registry key in the system 
 * 
 * Arguments : handle     : connection handle from the CONN_initialize API
 * 
 *             root       : Root of the registry group
 *
 *             key        : Key to be protected in the format of 
 *                          level1\\level2\\level3\\key
 * 
 * Note      : Applicable only to Window platform
 * ------------------------------------------------------------------------
 */ 

CEResult_t
CEPROTECT_UnlockKey (CEHandle handle, CEKeyRoot_t root, CEString key);

/*! CELOGGING_LogDecision
 *
 * \brief This, in combination with LOGDECISION custom obligation, provides
 *        a way to log user decision. 
 * 
 * \param handle (in)          connection handle from the CONN_initialize API
 * \param cookie (in)          data returned through LOGDECISION obligation
 * \param userResponse (in)    CEAllow if the user chose to continue with the action,
 *                             CEDeny otherwise
 * \param optAttributes (in)   any additional attributes that should be added to the
 *                             log entry
 * \return Result of logging decision.
 *
 * \sa CELOGGING_LogDecision
 */ 
CEResult_t 
CELOGGING_LogDecision(CEHandle handle, 
		      CEString cookie, 
		      CEResponse_t userResponse, 
		      CEAttributes * optAttributes);

#ifdef __cplusplus
}
#endif

#endif  /* CESDK_H */

