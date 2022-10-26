
#ifndef __CE_SDK_PRIVATE_H
#define __CE_SDK_PRIVATE_H

#define  FUNCID_CONN_INITIALIZE_Q                  0x00020000     //request
#define  FUNCID_CONN_INITIALIZE_P                  0x00030000     //reply
#define  FUNCID_CONN_CLOSE_Q                       0x00040000     //request
#define  FUNCID_CONN_CLOSE_P                       0x00050000     //reply
#define  FUNCID_PROTECT_LOCKKEY_Q                  0x00060000     //request
#define  FUNCID_PROTECT_LOCKKEY_P                  0x00070000     //reply
#define  FUNCID_PROTECT_UNLOCKKEY_Q                0x00080000     //request
#define  FUNCID_PROTECT_UNLOCKKEY_P                0x00090000     //reply
#define  FUNCID_PROTECT_PROTECTFILE_Q              0x000A0000     //request
#define  FUNCID_PROTECT_PROTECTFILE_P              0x000B0000     //reply
#define  FUNCID_PROTECT_UNPROTECTFILE_Q            0x000C0000     //request
#define  FUNCID_PROTECT_UNPROTECTFILE_P            0x000D0000     //reply
#define  FUNCID_PROTECT_PROTECTPROCESS_Q           0x000E0000     //request
#define  FUNCID_PROTECT_PROTECTPROCESS_P           0x000F0000     //reply
#define  FUNCID_PROTECT_UNPROTECTPROCESS_Q         0x00100000     //request
#define  FUNCID_PROTECT_UNPROTECTPROCESS_P         0x00110000     //reply
#define  FUNCID_EVALUATE_CHECKMETADATA_Q           0x00120000     //request
#define  FUNCID_EVALUATE_CHECKMETADATA_P           0x00130000     //reply
#define  FUNCID_CONTEXT_GETSYSTEMPEP_Q             0x00140000     //request
#define  FUNCID_CONTEXT_GETSYSTEMPEP_P             0x00150000     //reply
#define  FUNCID_CONTEXT_IGNOREPEP_Q                0x00160000     //request
#define  FUNCID_CONTEXT_IGNOREPEP_P                0x00170000     //reply
#define  FUNCID_CONTEXT_GETFILEATTRIBUTES_Q        0x00180000     //request
#define  FUNCID_CONTEXT_GETFILEATTRIBUTES_P        0x00190000     //reply
#define  FUNCID_LOGGING_SETNOISELEVEL_Q            0x001A0000     //request
#define  FUNCID_LOGGING_SETNOISELEVEL_P            0x001B0000     //reply 
#define  FUNCID_LOGGING_GETNOISELEVEL_Q            0x001C0000     //request
#define  FUNCID_LOGGING_GETNOISELEVEL_P            0x001D0000     //reply 
#define  FUNCID_CEP_STOPPDP_Q                      0x001E0000     //request
#define  FUNCID_CEP_STOPPDP_P                      0x001F0000     //reply 
#define  FUNCID_CEP_GETCHALLENGE_Q                 0x00200000     //request
#define  FUNCID_CEP_GETCHALLENGE_P                 0x00210000     //reply 
#define  FUNCID_CELOG_LOGDECISION_Q                0x00220000     //request
#define  FUNCID_CELOG_LOGDECISION_P                0x00230000     //reply 
#define  FUNCID_CELOG_LOGASSISTANTDATA_Q           0x00240000     //request
#define  FUNCID_CELOG_LOGASSISTANTDATA_P           0x00250000     //reply 
#define  FUNCID_GENERIC_FUNCCALL_Q                 0x00260000     //request 
#define  FUNCID_GENERIC_FUNCCALL_P                 0x00270000     //reply 
#define  FUNCID_CESEC_MAKETRUSTED_Q                0x00280000     //request 
#define  FUNCID_CESEC_MAKETRUSTED_P                0x00290000     //reply 
#define  FUNCID_EVALUATE_CHECKMULTIRESOURCES_Q     0x00300000     //request
#define  FUNCID_EVALUATE_CHECKMULTIRESOURCES_P     0x00310000     //reply

typedef enum _CEProtectMode_t {
  CE_PROTECT_READ                         =0x0,
  CE_PROTECT_WRITE                        =0x1,
  CE_PROTECT_DELETE                       =0x2,
  CE_PROTECT_MOVE                         =0x4,
  CE_PROTECT_RENAME                       =0x8,
  CE_PROTECT_METADATA_CHANGE              =0x10,
  CE_PROTECT_KILL                         =0x20,
} CEProtectMode_t;

//The followings are moved from CEsdk.h in order to hide them until
//the implementations have been done
typedef enum _CEPEP_t {
  CE_PEP_CLASS_NONE                       = 0x0,
  CE_PEP_CLASS_KERNEL                     = 0x1,      
  CE_PEP_CLASS_FS                         = 0x2,
  CE_PEP_CLASS_NETWORK                    = 0x4,      
  CE_PEP_CLASS_API                        = 0x8,      
  CE_PEP_CLASS_APPLICATION                = 0x10
}CEPEP_t;

typedef struct _CEAttributes_Array {
  CEAttributes *attrs_array;
  CEint32      count;
} CEAttributes_Array;

#ifdef __cplusplus
extern "C" {
#endif
/* ------------------------------------------------------------------------
 * CEPROTECT_ProtectFile()
 *
 * Protect a file in the file system 
 * 
 * Arguments : handle     : connection handle from the CONN_initialize API
 * 
 *             filename   : Full path of the file that needs to be protected
 *
 *             protect    : Bit-mask of the protecting mode define in
 *                          CE_PROTECT_*
 *                          
 * ------------------------------------------------------------------------
 */ 


CEResult_t
CEPROTECT_ProtectFile (CEHandle handle, CEString filename, CEint32 protect);


/* ------------------------------------------------------------------------
 * CEPROTECT_UnprotectFile()
 *
 * Undo the protection of a file in the file system 
 * 
 * Arguments : handle     : connection handle from the CONN_initialize API
 * 
 *             filename   : Full path of the file that needs to be unprotected
 *
 * ------------------------------------------------------------------------
 */ 
CEResult_t
CEPROTECT_UnprotectFile (CEHandle handle, CEString filename);

/* ------------------------------------------------------------------------
 * PROTECT_ProtectProcess()
 *
 * Protect a process from being terminated from the system 
 * 
 * Arguments : handle     : connection handle from the CONN_initialize API
 * 
 *             processId  : Process ID to be protected 
 *
 * ------------------------------------------------------------------------
 */ 
CEResult_t
CEPROTECT_ProtectProcess (CEHandle handle, CEint32 processId);

/* ------------------------------------------------------------------------
 * PROTECT_UnProtectProcess()
 *
 * Undo the protection of a process from being terminated from the system 
 * 
 * Arguments : handle     : connection handle from the CONN_initialize API
 * 
 *             processId  : Process ID to be unprotected 
 *
 * ------------------------------------------------------------------------
 */ 

CEResult_t
CEPROTECT_UnProtectProcess (CEHandle handle, CEint32 processId);

/* ------------------------------------------------------------------------
 * CECONTEXT_GetSystemPEPs()
 *
 * Retrieve the classes of the PEP classes that are running in the system
 * 
 * Arguments : handle     : connection handle from the CONN_initialize API
 * 
 *             peps       : bit-mask of the CE_PEP_CLASS_* reflecting the
 *                          current running PEPs
 *             
 * ------------------------------------------------------------------------
 */ 

CEResult_t
CECONTEXT_GetSystemPEPs (CEHandle handle, CEint32 * peps);


/* ------------------------------------------------------------------------
 * CECONTEXT_GetFileContexts()
 *
 * Retrieve the context that are associated with the filename 
 * 
 * Arguments : handle     : connection handle from the CONN_initialize API
 * 
 *             filename   : Full path filename to retrieve the associated
 *                          context
 *
 * Output    : attributes : List of key/value pairs that are associate
 *                          to the supplied filename
 *             
 * ------------------------------------------------------------------------
 */ 


CEResult_t
CECONTEXT_GetFileAttributes (CEHandle handle, CEString filename, 
                           CEAttributes ** attributes);

/* ------------------------------------------------------------------------
 * CELOGGING_SetNoiseLevel()
 *
 * Setting the default noise level for this client PEP
 * 
 * Arguments : handle     : connection handle from the CONN_initialize API
 * 
 *             noiseLevel : Desire noise level for this client PEP
 *
 * ------------------------------------------------------------------------
 */ 

CEResult_t
CELOGGING_SetNoiseLevel (CEHandle handle, CENoiseLevel_t noiseLevel);

/* ------------------------------------------------------------------------
 * CELOGGING_GetNoiseLevel()
 *
 * Getting the current noise level for this client PEP
 * 
 * Arguments : handle     : connection handle from the CONN_initialize API
 * 
 * Output    : noiseLevel : Current noise level for this client PEP
 *
 * ------------------------------------------------------------------------
 */ 


CEResult_t
CELOGGING_GetNoiseLevel (CEHandle handle, CENoiseLevel_t *noiseLevel);


/* ------------------------------------------------------------------------
 * CELOGGING_LogOperation()
 *
 * Log an operation without performing the policy evaluation from the 
 * Policy Decision Point Server
 *
 * Arguments : handle     : connection handle from the CONN_initialize API
 * 
 *             action     : Action performed by the client application
 *             
 *             source     : Name of the resource that the action perform on
 *
 *             target     : Name of the resource that the action perform to
 *
 *             attributes : List of attributes associate with the request
 *
 *           noiseLevel   : Classification of the noise level of this action
 *
 * ------------------------------------------------------------------------
 */ 


CEResult_t
CELOGGING_LogOperation (CEHandle handle, CEAction_t action, 
			CEString source, CEString target,
			CEAttributes *   attributes,
			CENoiseLevel_t   noiseLevel);
#ifdef __cplusplus
}
#endif

#endif

