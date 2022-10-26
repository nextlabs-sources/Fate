/**************************************************************************************************
 *
 * NextLabs Kernel Log (NL KLOG)
 *
 *************************************************************************************************/

#ifndef __NL_KLOG_H__
#define __NL_KLOG_H__

#include <ntstatus.h>
#include <wdm.h>

/** NL_KLOG_LEVEL
 *
 *  \brief Log/trace level which follow SYSLOG levels.
 */
typedef enum
{
  NL_KLOG_LEVEL_EMERG   = 0 ,
  NL_KLOG_LEVEL_ALERT   = 1 ,
  NL_KLOG_LEVEL_CRIT    = 2 ,
  NL_KLOG_LEVEL_ERR     = 3 ,
  NL_KLOG_LEVEL_WARNING = 4 ,
  NL_KLOG_LEVEL_NOTICE  = 5 ,
  NL_KLOG_LEVEL_INFO    = 6 ,
  NL_KLOG_LEVEL_DEBUG   = 7 ,

  /* User defined levels [8,999] */
  NL_KLOG_LEVEL_USER    = 8,
  NL_KLOG_LEVEL_MAX     = 999
} NL_KLOG_LEVEL;

/** NL_KLOG_FILTER
 *
 *  \brief Callback type to filter messages.
 *
 *  \return TRUE if the message should be processed, otherwise FALSE.  If
 *          the message is not processed it is dropped.
 */
typedef BOOLEAN (*NL_KLOG_FILTER)( NL_KLOG_LEVEL , PCHAR );

/** NL_KLOG_OPTIONS
 *
 *  \brief Options use for constructing an instance of NL_KLOG.
 */
typedef struct
{
  ULONG size;                   /* Size of this structure.  This must be set by the caller
				 * of NL_KLOG_Initialize.
				 * This field is required for NL_KLOG_Initialize.
				 */
  ULONG options;                /* Options.  This field is reserved. */
  NL_KLOG_LEVEL level;          /* Default threshold level.  Calls to NL_KLOG_Log with log
				 * level greather than this will be dropped/filtered.  For
				 * example, a default level of NL_KLOG_LEVEL_ERROR will
				 * filter NL_KLOG_LEVEL_DEBUG.  This field is required for
				 * NL_KLOG_Initialize.
				 */
  ULONG queue_size;             /* Queue size (maximum).  A value of 0 indicates 100.
				 * This field is optional for NL_KLOG_Initialize.
				 */
  NL_KLOG_FILTER filter;        /* Filter messages from the kernel log.  This value must be
				 * initialized to NULL or a valid callback.  This callback
				 * will always be called at PASSIVE_LEVEL.
				 */

} NL_KLOG_OPTIONS, *PNL_KLOG_OPTIONS;

/** NL_KLOG
 *
 *  \brief NextLabs Kernel Log
 */
typedef struct
{
  PVOID p_impl;   /* Opaque implementation */
} NL_KLOG, *PNL_KLOG;

/** NL_KLOG_Initialize
 *
 *  \brief Initialize an instance of NL_KLOG.
 *
 *  \param in_klog (in) Log instance.
 *  \param in_ops (in)  Options for log instance construction.
 *
 *  \return STATUS_SUCCESS on success, otherwise a STATUS_XXX error.
 *
 *  \notes IRQL == PASSIVE_LEVEL.  If this log instance will be used at DISPATCH_LEVEL,
 *         then the log instance must be accessable at that IRQL - i.e. be non-pagable.
 *
 *  \sa NL_KLOG_Shutdown
 */
__drv_requiresIRQL(PASSIVE_LEVEL)
__checkReturn
NTSTATUS NL_KLOG_Initialize( __out PNL_KLOG in_klog ,
			     __in PNL_KLOG_OPTIONS in_ops );

/** NL_KLOG_Shutdown
 *
 *  \brief Shutdown/teardown the NL_KLOG instance.  After an instance has been shutdown
 *         is should not be used by any NL_KLOX_XXX methods.
 *
 *  \param in_klog (in) Log instance.  This parameter must not be NULL.
 *
 *  \return STATUS_SUCCESS on success, otherwise a STATUS_XXX error.
 *
 *  \notes IRQL == PASSIVE_LEVEL
 *
 *  \sa NL_KLOG_Initialize
 */
__drv_requiresIRQL(PASSIVE_LEVEL)
__checkReturn
NTSTATUS NL_KLOG_Shutdown( __in PNL_KLOG in_klog );

/** NL_KLOG_Log
 *
 *  \brief Log a message to the given NL_KLOG instance.  Format support is similar to
 *         KdPrint.  See documentation for that method for details.
 *
 *  \param in_klog (in)   Log instance.  This parameter must not be NULL.
 *  \param in_level (in)  Log level NL_KLOG_LEVEL_XXX.
 *  \param in_format (in) Print format.
 *
 *  \return STATUS_SUCCESS on success, otherwise a STATUS_XXX error.  The return value of
 *          STATUS_INSUFFICIENT_RESOURCES may indicate that the queue limit has been reached
 *          and this message was dropped.
 *
 *  \notes IRQL <= DISPATCH_LEVEL.  Formatted string must not be used when IRQL is
 *         not PASSIVE_LEVEL.
 *
 *  \sa NL_KLOG_LEVEL, NL_KLOG_Initialize, NL_KLOG_Shutdown
 */
__drv_maxIRQL(DISPATCH_LEVEL)
NTSTATUS NL_KLOG_Log( __in PNL_KLOG in_klog ,
		      __in NL_KLOG_LEVEL in_level ,
		      __in __format_string PCHAR in_format ,
		      ... );

/** NL_KLOG_SetLevel
 *
 *  \brief Set threshold level for the given log instance.
 *
 *  \param in_klog (in)   Log instance.  This parameter must not be NULL.
 *  \param in_level (in)  Log level NL_KLOG_LEVEL_XXX.
 *
 *  \return STATUS_SUCCESS on success, otherwise a STATUS_XXX error.
 *
 *  \notes Any IRQL provided the log instances is accessable at the current level.
 *
 *  \sa NL_KLOG_LEVEL, NL_KLOG_Initialize, NL_KLOG_Shutdown
 */
__drv_maxIRQL(DISPATCH_LEVEL)
__checkReturn
NTSTATUS NL_KLOG_SetLevel( __in PNL_KLOG in_klog ,
			   __in NL_KLOG_LEVEL in_level );

/** NL_KLOG_SetFile
 *
 *  \brief Initialize an instance of NL_KLOG.
 *
 *  \param in_klog (in)          An initialized log instance.
 *  \param in_log_file (in)      Path to the log file.
 *  \param in_log_file_size (in) Log file size in bytes.
 *
 *  \return STATUS_SUCCESS on success, otherwise a STATUS_XXX error.
 *
 *  \notes IRQL <= DISPATCH_LEVEL.  The log file created as a result will be
 *         closed when the NL_KLOG instance is shutdown.
 *
 *  \sa NL_KLOG_Initialize, NL_KLOG_Shutdown
 */
__drv_maxIRQL(DISPATCH_LEVEL)
__checkReturn
NTSTATUS NL_KLOG_SetFile( __in PNL_KLOG in_klog ,
			  __in PUNICODE_STRING in_log_file ,
			  __in ULONG in_log_file_size );

#define KLOG_STRINGIFY(x) #x
#define KLOG_TOSTRING(x) KLOG_STRINGIFY(x)
#define KLOG_AT __FILE__ ":" __FUNCTION__ "." KLOG_TOSTRING(__LINE__) ": "
#define KLOG_TRACE(mylog,level,fmt,...) NL_KLOG_Log(mylog,level,KLOG_AT##fmt,__VA_ARGS__)

#if defined(NL_KLOG_USE_KDPRINT)
#  undef KLOG_TRACE
#  define KLOG_TRACE(mylog,level,...) KdPrint(__VA_ARGS__)
#endif

#endif /* __NL_KLOG_H__ */
