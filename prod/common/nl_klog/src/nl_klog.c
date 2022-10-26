/**************************************************************************************************
 *
 * NextLabs Kernel Log (NL KLOG)
 *
 *************************************************************************************************/

#include <ntifs.h>
#include <wdm.h>
#include <ntstrsafe.h>
#include <limits.h>

#include "nl_klog.h"

/* When NL_KLOG_DEBUG_MODE is defined there will be verbose logging for NL KLOG.
 *
 * NL_KLOG_KdPrint is KdPrint when NL_KLOG_DEBUG_MODE is not defined.
 */
//#define NL_KLOG_DEBUG_MODE
#ifdef NL_KLOG_DEBUG_MODE
#  define NL_KLOG_KdPrint KdPrint
#else
#  define NL_KLOG_KdPrint
#endif

#define NL_KLOG_POOL_TAG         '0kln'  /* KLOG object */
#define NL_KLOG_MESSAGE_POOL_TAG '1kln'  /* KLOG message objects */

/** NL_KLOG_INTERNAL
 *
 *  \brief Implementation of NL_KLOG.
 */
typedef struct
{
  HANDLE th;                    /* worker thread handle */
  KEVENT cancel_event;          /* signal to cancel worker thread */

  UNICODE_STRING log_file;      /* log file */
  ULONG log_file_size;          /* log file size in bytes */
  KEVENT log_file_event;        /* log file event - signaled when log file set */
  HANDLE fh;                    /* log file handle */

  /* Message queue */
  LIST_ENTRY queue;             /* queue */
  KSPIN_LOCK qlock;             /* spin lock to protect queue */
  KEVENT queue_event;           /* signal to process a message */
  ULONG queue_size;             /* current queue size */
  ULONG queue_size_max;         /* maximum queue size */

  NL_KLOG_LEVEL level;          /* threshold level */

  NL_KLOG_FILTER filter;        /* Filter messages from the kernel log.
				 */

} NL_KLOG_INTERNAL, *PNL_KLOG_INTERNAL;

/** NL_KLOG_MESSAGE
 *
 *  \brief Message payload.
 */
typedef struct
{
  LIST_ENTRY list_node;     /* Node */
  NL_KLOG_LEVEL level;      /* Log Level */
  LONG  message_size;       /* Message size allocated in bytes */
  PCHAR message;            /* Message allocated from non-paged pool */
} NL_KLOG_MESSAGE, *PNL_KLOG_MESSAGE;

#define MESSAGE_SIZE 1024

DRIVER_INITIALIZE DriverEntry;

/** NL_KLOG_AllocateMessage
 *
 *  \brief Allocate an instance of a klog message.  The message will be
 *         allocated from the NL_KLOG_MESSAGE_POOL_TAG pool.  Members
 *         message and message_size are assigned if the allocation is
 *         successful.
 *
 *         The message is allocated from the non-paged pool.
 */
__checkReturn
static PNL_KLOG_MESSAGE NL_KLOG_AllocateMessage(void)
{
  PNL_KLOG_MESSAGE msg = NULL;
  msg = (PNL_KLOG_MESSAGE)ExAllocatePoolWithTag(NonPagedPool,
						sizeof(NL_KLOG_MESSAGE) + MESSAGE_SIZE,
						NL_KLOG_MESSAGE_POOL_TAG);
  if( msg != NULL )
  {
    /* Set message to allocated buffer after list header */
    msg->message = (PCHAR)( ((PUCHAR)msg) + sizeof(NL_KLOG_MESSAGE) );
    msg->message_size = MESSAGE_SIZE;
  }
  return msg;
}/* NL_KLOG_AllocateMessage */

/** NL_KLOG_FreeMessage
 *
 *  \brief Free an instance of a klog message.
 */
static void NL_KLOG_FreeMessage( __in PNL_KLOG_MESSAGE msg )
{
  ASSERT( msg != NULL );
  RtlSecureZeroMemory(msg,sizeof(NL_KLOG_MESSAGE));  /* zero out message for security */
  ExFreePoolWithTag(msg,NL_KLOG_MESSAGE_POOL_TAG);   /* free message */
}/* NL_KLOG_FreeMessage */

NTSTATUS DriverEntry( __in struct _DRIVER_OBJECT *DriverObject,
		      __in PUNICODE_STRING RegistryPath )
{
  return STATUS_SUCCESS;
}/* DriverEntry */

NTSTATUS DllInitialize( __in PUNICODE_STRING RegistryPath )
{
  return STATUS_SUCCESS;
}/* DllInitialize */

NTSTATUS DllUnload(void)
{
  return STATUS_SUCCESS;
}/* DllUnload */

KSTART_ROUTINE NL_KLOG_WorkerTread;

/** NL_KLOG_WorkerThread
 *
 *  Process messages which are queued by calling NL_KLOG_Log until a cancel
 *  signal is set.
 */
VOID NL_KLOG_WorkerTread( __in PVOID in_context )
{
  NTSTATUS status;
  PNL_KLOG klog = (PNL_KLOG)in_context; /* log instance */
  PNL_KLOG_INTERNAL klogi = NULL;       /* internal log instance */

  ASSERT( in_context != NULL );

  klogi = (NL_KLOG_INTERNAL*)klog->p_impl;

  for( ; ; )
  {
    /* Wait for either (1) cancel, (2) message signal, or (3) log file  */
    PVOID objs[] = { (PVOID)&klogi->cancel_event ,
		     (PVOID)&klogi->queue_event ,    
		     (PVOID)&klogi->log_file_event };

    NL_KLOG_KdPrint(("NL_KLOG_WorkerTread: waiting for event\n"));
    status = KeWaitForMultipleObjects(3,objs,WaitAny,Executive,KernelMode,FALSE,NULL,NULL);

    /* Cancel signal? */
    if( status == STATUS_WAIT_0 )
    {
      PNL_KLOG_MESSAGE msg = NULL;  /* current message */
      KLOCK_QUEUE_HANDLE qlock_qh;  /* in-queue spin lock */

      NL_KLOG_KdPrint(("NL_KLOG_WorkerTread: cancel signal\n"));
      NL_KLOG_KdPrint(("NL_KLOG_WorkerTread: draining queue\n"));
      KeAcquireInStackQueuedSpinLock(&klogi->qlock,&qlock_qh);
      while( IsListEmpty(&klogi->queue) == FALSE )
      {
	msg = (PNL_KLOG_MESSAGE)RemoveHeadList(&klogi->queue);
	klogi->queue_size--;
	RtlSecureZeroMemory(msg,sizeof(NL_KLOG_MESSAGE));  /* zero out message for security */
	ExFreePoolWithTag(msg,NL_KLOG_MESSAGE_POOL_TAG);
      }
      KeReleaseInStackQueuedSpinLock(&qlock_qh);
      NL_KLOG_KdPrint(("NL_KLOG_WorkerTread: draining queue complete\n"));

      break;
    }/* cancel? */

    /* The log file was set */
    if( status == STATUS_WAIT_2 )
    {
      OBJECT_ATTRIBUTES oa;             /* log file attributes */
      IO_STATUS_BLOCK iosb;             /* log file io status */

      RtlSecureZeroMemory(&oa,sizeof(OBJECT_ATTRIBUTES));
      RtlSecureZeroMemory(&iosb,sizeof(IO_STATUS_BLOCK));
      InitializeObjectAttributes(&oa,&klogi->log_file,OBJ_INHERIT,NULL,NULL);

      if( klogi->fh != NULL )
      {
	NL_KLOG_KdPrint(("NL_KLOG_WorkerTread: closing existing file\n"));
	ZwClose(klogi->fh);
	klogi->fh = NULL;
      }

      /* Create log file - open existing or create */
      NL_KLOG_KdPrint(("NL_KLOG_WorkerTread: creating log file %wZ\n",&klogi->log_file));
      status = ZwCreateFile(&klogi->fh,GENERIC_READ | GENERIC_WRITE | SYNCHRONIZE ,
			    &oa,&iosb,NULL,
			    FILE_ATTRIBUTE_NORMAL,
			    FILE_SHARE_READ | FILE_SHARE_WRITE,
			    FILE_OPEN_IF,
			    FILE_RANDOM_ACCESS |                 /* CreateOptions */
			    FILE_SYNCHRONOUS_IO_NONALERT |
			    FILE_WRITE_THROUGH,
			    NULL,0);
      if( !NT_SUCCESS(status) )
      {
	NL_KLOG_KdPrint(("NL_KLOG_WorkerTread: ZwCreateFile failed (0x%x)\n",status));
	klogi->fh = NULL;
      }
    }

    /* Process message list.  The queue signal is reset after the wait is satisfied. */
    NL_KLOG_KdPrint(("NL_KLOG_WorkerTread: processing messages\n"));
    for( ; ; )
    {
      PNL_KLOG_MESSAGE msg = NULL;  /* current message */
      IO_STATUS_BLOCK iosb;         /* io status */
      size_t buf_size = 0;          /* current message size */
      FILE_STANDARD_INFORMATION fi; /* file information for size */
      KLOCK_QUEUE_HANDLE qlock_qh;  /* in-queue spin lock */

      /* pop queue */
      KeAcquireInStackQueuedSpinLock(&klogi->qlock,&qlock_qh);
      if( IsListEmpty(&klogi->queue) == FALSE )
      {
	msg = (PNL_KLOG_MESSAGE)RemoveHeadList(&klogi->queue);
	klogi->queue_size--;
      }
      KeReleaseInStackQueuedSpinLock(&qlock_qh);

      if( msg == NULL ) /* NULL is empty list */
      {
	NL_KLOG_KdPrint(("NL_KLOG_WorkerTread: empty list\n"));
	break;
      }

      /* Should this message be dropped? */
      if( klogi->filter != NULL &&
	  klogi->filter(msg->level,msg->message) == FALSE )
      {
	NL_KLOG_KdPrint(("NL_KLOG_WorkerTread: filter decision - drop\n"));
	NL_KLOG_FreeMessage(msg);
	continue;
      }

      NL_KLOG_KdPrint(("NL_KLOG_WorkerTread: file handle = 0x%x\n",klogi->fh));
      if( klogi->fh == NULL )
      {
	NL_KLOG_KdPrint(("NL_KLOG_WorkerTread: no file set\n"));
	NL_KLOG_FreeMessage(msg);
	continue;
      }

      /* Determine if log file size has been exceeded */
      status = ZwQueryInformationFile(klogi->fh,&iosb,(PVOID)&fi,sizeof(fi),FileStandardInformation);
      if( !NT_SUCCESS(status) )
      {
	NL_KLOG_KdPrint(("NL_KLOG_WorkerTread: ZwQueryInformationFile failed (0x%x)\n",status));
      }
      else
      {
	/* If the file exceeds the maximum log size set truncate the file and reset the file position. */
	if( fi.EndOfFile.QuadPart >= klogi->log_file_size )
	{
	  FILE_END_OF_FILE_INFORMATION eof_info;
	  FILE_POSITION_INFORMATION fp_info;

	  NL_KLOG_KdPrint(("NL_KLOG_WorkerTread: exceeded log size\n"));

	  /* Truncate */
	  eof_info.EndOfFile.QuadPart = 0;
	  status = ZwSetInformationFile(klogi->fh,&iosb,&eof_info,sizeof(eof_info),FileEndOfFileInformation);
	  if( !NT_SUCCESS(status) )
	  {
	    NL_KLOG_KdPrint(("NL_KLOG_WorkerTread: ZwSetInformationFile failed  for truncation (0x%x)\n",status));
	  }

	  /* Rewind */
	  fp_info.CurrentByteOffset.QuadPart = 0;
	  status = ZwSetInformationFile(klogi->fh,&iosb,&fp_info,sizeof(fp_info),FilePositionInformation);
	  if( !NT_SUCCESS(status) )
	  {
	    NL_KLOG_KdPrint(("NL_KLOG_WorkerTread: ZwSetInformationFile failed for rewind (0x%x)\n",status));
	  }
	}
      }

      buf_size = strlen(msg->message);
      status = ZwWriteFile(klogi->fh,NULL,NULL,NULL,&iosb,msg->message,buf_size,NULL,NULL);
      if( !NT_SUCCESS(status) )
      {
	NL_KLOG_KdPrint(("NL_KLOG_WorkerTread: ZwWriteFile failed (0x%x)\n",status));
      }

      NL_KLOG_FreeMessage(msg);
    }
  }/* for */

  NL_KLOG_KdPrint(("NL_KLOG_WorkerTread: complete\n"));
  PsTerminateSystemThread(STATUS_SUCCESS);

}/* NL_KLOG_WorkerTread */

__drv_requiresIRQL(PASSIVE_LEVEL)
__checkReturn
NTSTATUS NL_KLOG_Initialize( __out PNL_KLOG klog ,
			     __in NL_KLOG_OPTIONS* ops )
{
  NTSTATUS status;
  PNL_KLOG_INTERNAL klogi = NULL;   /* log instance */

  ASSERT( KeGetCurrentIrql() == PASSIVE_LEVEL );
  ASSERT( klog != NULL );
  ASSERT( ops != NULL );

  if( klog == NULL || ops == NULL )
  {
    NL_KLOG_KdPrint(("NL_KLOG_Initialize: invalid parameter\n"));
    return STATUS_INVALID_PARAMETER;
  }

  NL_KLOG_KdPrint(("NL_KLOG_Initialize: initializing\n"));

  /* The caller was compiled with a different size of NL_KLOG_OPTIONS */
  if( ops->size != sizeof(NL_KLOG_OPTIONS) )
  {
    return STATUS_INVALID_PARAMETER;
  }

  RtlSecureZeroMemory(klog,sizeof(NL_KLOG));

  /* Allocate internal structure */
  klog->p_impl = ExAllocatePoolWithTag(NonPagedPool,sizeof(NL_KLOG_INTERNAL),NL_KLOG_POOL_TAG);
  if( klog->p_impl == NULL )
  {
    NL_KLOG_KdPrint(("NL_KLOG_Initialize: allocation failed\n"));
    return STATUS_INSUFFICIENT_RESOURCES;
  }

  klogi = (NL_KLOG_INTERNAL*)klog->p_impl;

  /* Initialize log instance */
  RtlSecureZeroMemory(klog->p_impl,sizeof(NL_KLOG_INTERNAL));        /* zero out */
  KeInitializeEvent(&klogi->cancel_event,NotificationEvent,FALSE);   /* init cancel event */
  KeInitializeEvent(&klogi->queue_event,SynchronizationEvent,FALSE); /* init queue event */

  KeInitializeEvent(&klogi->log_file_event,SynchronizationEvent,FALSE); /* init log file event */
  klogi->fh = NULL;                                                     /* log file unset */

  KeInitializeSpinLock(&klogi->qlock);                               /* init spin lock */
  InitializeListHead(&klogi->queue);                                 /* init message queue */
  klogi->level = ops->level;                                         /* default threshold to debug */
  klogi->filter = ops->filter;                                       /* filter callback */
  klogi->queue_size = 0L;                                            /* current queue size  */
  if( ops->queue_size > 0 )
  {
    klogi->queue_size_max = ops->queue_size;                         /* max queue size */
  }

  /* Create and start worker thread */
  status = PsCreateSystemThread(&klogi->th,GENERIC_ALL,NULL,NULL,NULL,NL_KLOG_WorkerTread,klog);
  if( !NT_SUCCESS(status) )
  {
    NL_KLOG_KdPrint(("NL_KLOG_Initialize: failed to create worker thread\n"));
    ExFreePoolWithTag(klog->p_impl,NL_KLOG_POOL_TAG);
    goto NL_KLOG_Initialize_complete;
  }

 NL_KLOG_Initialize_complete:

  NL_KLOG_KdPrint(("NL_KLOG_Initialize: complete (0x%x)\n",status));

  return status;
}/* NL_KLOG_Initialize */

__drv_requiresIRQL(PASSIVE_LEVEL)
__checkReturn
NTSTATUS NL_KLOG_Shutdown( __in PNL_KLOG in_klog )
{
  NTSTATUS status;
  PNL_KLOG_INTERNAL klogi = NULL; /* internal log instance */
  PKTHREAD pkt = NULL;            /* kernel thread handle */

  ASSERT( KeGetCurrentIrql() == PASSIVE_LEVEL );
  ASSERT( in_klog != NULL );

  if( in_klog == NULL )
  {
    NL_KLOG_KdPrint(("NL_KLOG_Shutdown: invalid parameter\n"));
    return STATUS_INVALID_PARAMETER;
  }

  NL_KLOG_KdPrint(("NL_KLOG_Shutdown: starting shutdown\n"));

  klogi = (NL_KLOG_INTERNAL*)in_klog->p_impl;

  /* Signal worker thread to terminate */
  NL_KLOG_KdPrint(("NL_KLOG_Shutdown: signal cancel event\n"));
  KeSetEvent(&klogi->cancel_event,0,TRUE);

  NL_KLOG_KdPrint(("NL_KLOG_Shutdown: waiting for worker\n"));

  /* Synchronization and free requires kernel object rather than handle */
  status = ObReferenceObjectByHandle(klogi->th,GENERIC_ALL,NULL,KernelMode,&pkt,NULL);
  if( !NT_SUCCESS(status) )
  {
    NL_KLOG_KdPrint(("NL_KLOG_Shutdown: ObReferenceObjectByHandle failed (0x%x)\n",status));
  }

  /* Wait for the worker thread to teardown */
  status = KeWaitForSingleObject(pkt,Executive,KernelMode,FALSE,NULL);
  NL_KLOG_KdPrint(("NL_KLOG_Shutdown: thread wait result 0x%x\n",status));
  ZwClose(pkt);

  ObDereferenceObject((PVOID)pkt);   /* deref due to ObReferenceObjectByHandle  */
  if( klogi->fh != NULL )
  {
    ZwClose(klogi->fh);              /* close log file handle */
  }

  if( klogi->log_file.Buffer != NULL )
  {
    ExFreePoolWithTag(klogi->log_file.Buffer,NL_KLOG_POOL_TAG);
  }

  NL_KLOG_KdPrint(("NL_KLOG_Shutdown: tearing down log instance\n"));
  RtlSecureZeroMemory(klogi,sizeof(NL_KLOG_INTERNAL));
  ExFreePoolWithTag(klogi,NL_KLOG_POOL_TAG); 
  RtlSecureZeroMemory(in_klog,sizeof(NL_KLOG)); 

  NL_KLOG_KdPrint(("NL_KLOG_Shutdown: complete\n"));

  return STATUS_SUCCESS;
}/* NL_KLOG_Shutdown */

__drv_maxIRQL(DISPATCH_LEVEL)
NTSTATUS NL_KLOG_Log( __in PNL_KLOG in_klog ,
		      __in NL_KLOG_LEVEL in_level ,
		      __in __format_string PCHAR in_format ,
		      ... )
{
  NTSTATUS status;
  PNL_KLOG_INTERNAL klogi = NULL; /* internal log instance */
  PNL_KLOG_MESSAGE msg = NULL;    /* message */
  va_list va;                     /* variable args */
  BOOLEAN drop_message = FALSE;   /* drop due to queue size? */
  LARGE_INTEGER timestamp_gmt;    /* current time GMT */
  LARGE_INTEGER timestamp;        /* current time local time */
  TIME_FIELDS timestamp_fields;   /* time parts - i.e. day, hour, etc. */
  KLOCK_QUEUE_HANDLE qlock_qh;    /* in-queue spin lock */
  typedef struct
  {
    const char* name;  /* level name (i.e. "DEBUG") */
    size_t name_len;   /* length of string */
  } level_table_t;
  static const level_table_t level_table[] =
    {
      { "EMERGENCY" , sizeof("EMERGENCY")  },    /* NL_KLOG_LEVEL_EMERG   */
      { "ALERT"     , sizeof("ALERT")      },    /* NL_KLOG_LEVEL_ALERT   */
      {	"CRITICAL"  , sizeof("CRITICAL")   },    /* NL_KLOG_LEVEL_CRIT    */
      { "ERROR"     , sizeof("ERROR")      },    /* NL_KLOG_LEVEL_ERR     */
      { "WARNING"   , sizeof("WARNING")    },    /* NL_KLOG_LEVEL_WARNING */
      { "NOTICE"    , sizeof("NOTICE")     },    /* NL_KLOG_LEVEL_NOTICE  */
      { "INFO"      , sizeof("INFO")       },    /* NL_KLOG_LEVEL_INFO    */
      { "DEBUG"     , sizeof("DEBUG")      },    /* NL_KLOG_LEVEL_DEBUG   */
      { "USER"      , sizeof("USER")       }     /* NL_KLOG_LEVEL_USER    */
    };

  ASSERT( KeGetCurrentIrql() <= DISPATCH_LEVEL );
  ASSERT( in_klog != NULL );
  ASSERT( in_format != NULL );

  if( in_klog == NULL || in_format == NULL )
  {
    NL_KLOG_KdPrint(("NL_KLOG_Log: invalid parameter\n"));
    return STATUS_INVALID_PARAMETER;
  }

  klogi = (NL_KLOG_INTERNAL*)in_klog->p_impl;

  /* Does threshold filter this message? */
  if( in_level > klogi->level )
  {
    NL_KLOG_KdPrint(("NL_KLOG_Log: dropping due to threshold\n"));
    return STATUS_SUCCESS;
  }

  /* If the queue size is exhaused per maximum queue size */
  KeAcquireInStackQueuedSpinLock(&klogi->qlock,&qlock_qh);
  if( klogi->queue_size >= klogi->queue_size_max )
  {
    drop_message = TRUE;
  }
  KeReleaseInStackQueuedSpinLock(&qlock_qh);

  if( drop_message == TRUE )
  {
    NL_KLOG_KdPrint(("NL_KLOG_Log: dropping due to queue size limit\n"));
    return STATUS_INSUFFICIENT_RESOURCES;
  }

  /* Commit message to queue */
  msg = NL_KLOG_AllocateMessage();
  if( msg == NULL )
  {
    NL_KLOG_KdPrint(("NL_KLOG_Log: ExAllocatePoolWithTag failed\n"));
    return STATUS_INSUFFICIENT_RESOURCES;
  }

  /* Store log level */
  msg->level = in_level;

  /* Add timestamp */
  KeQuerySystemTime(&timestamp_gmt);
  ExSystemTimeToLocalTime(&timestamp_gmt,&timestamp);
  RtlTimeToTimeFields(&timestamp,&timestamp_fields);

  /* If the trace level exceeds the default levels [CRIT,DEBUG] then set the level
   * to USER to get the name.
   */
  if( in_level > NL_KLOG_LEVEL_DEBUG )
  {
    in_level = NL_KLOG_LEVEL_USER;
  }

  /* 17 digits 2/2/4 2:2:2:3 and 8 separators */
  status = RtlStringCbPrintfA(msg->message,
			      msg->message_size,
			      "%02d/%02d/%04d %02d:%02d:%02d.%03d: %s ",
			      timestamp_fields.Month,
			      timestamp_fields.Day,
			      timestamp_fields.Year,
			      timestamp_fields.Hour,
			      timestamp_fields.Minute,
			      timestamp_fields.Second,
			      timestamp_fields.Milliseconds,
			      level_table[in_level].name);

  va_start(va,in_format);
  status = RtlStringCbVPrintfA(msg->message + (17 + 8 + level_table[in_level].name_len),
			       msg->message_size - (17 + 8 + level_table[in_level].name_len),
			       in_format,va);
  va_end(va);

  /* If formatted print succeeded push the message, otherwise fail it. */
  if( NT_SUCCESS(status) )
  {
    /* push message on queue */
    KeAcquireInStackQueuedSpinLock(&klogi->qlock,&qlock_qh);
    InsertTailList(&klogi->queue,(PLIST_ENTRY)msg);
    klogi->queue_size++;
    KeReleaseInStackQueuedSpinLock(&qlock_qh);

    KeSetEvent(&klogi->queue_event,0,FALSE);           /* signal worker thread */
  }
  else
  {
    ExFreePoolWithTag(msg,NL_KLOG_MESSAGE_POOL_TAG);   /* free message */
  }

  return status;
}/* NL_KLOG_Log */

__drv_maxIRQL(DISPATCH_LEVEL)
__checkReturn
NTSTATUS NL_KLOG_SetLevel( __in PNL_KLOG in_klog ,
			   __in NL_KLOG_LEVEL in_level )
{
  NTSTATUS status;
  PNL_KLOG_INTERNAL klogi = NULL;       /* internal log instance */

  ASSERT( in_klog != NULL );

  if( in_klog == NULL )
  {
    NL_KLOG_KdPrint(("NL_KLOG_Log: invalid parameter\n"));
    return STATUS_INVALID_PARAMETER;
  }

  klogi = (NL_KLOG_INTERNAL*)in_klog->p_impl;
  klogi->level = in_level;

  return STATUS_SUCCESS;
}/* NL_KLOG_SetLevel */

__drv_maxIRQL(DISPATCH_LEVEL)
__checkReturn
NTSTATUS NL_KLOG_SetFile( __in PNL_KLOG in_klog ,
			  __in PUNICODE_STRING in_log_file ,
			  __in ULONG in_log_file_size )
{
  OBJECT_ATTRIBUTES oa;             /* log file attributes */
  IO_STATUS_BLOCK iosb;             /* log file io status */
  PNL_KLOG_INTERNAL klogi = NULL;   /* internal log instance */

  ASSERT( KeGetCurrentIrql() <= DISPATCH_LEVEL );
  ASSERT( in_klog != NULL );
  ASSERT( in_log_file != NULL );

  if( in_klog == NULL || in_klog->p_impl == NULL || in_log_file == NULL )
  {
    NL_KLOG_KdPrint(("NL_KLOG_SetFile: invalid parameter\n"));
    return STATUS_INVALID_PARAMETER;
  }

  NL_KLOG_KdPrint(("NL_KLOG_SetFile: %wZ\n",in_log_file));

  klogi = (NL_KLOG_INTERNAL*)in_klog->p_impl;

  /* allocate unicode string */
  RtlCopyMemory(&klogi->log_file,in_log_file,sizeof(klogi->log_file));
  klogi->log_file.Buffer = (PWSTR)ExAllocatePoolWithTag(PagedPool,in_log_file->MaximumLength,
							NL_KLOG_POOL_TAG);
  if( klogi->log_file.Buffer == NULL )
  {
    return STATUS_INSUFFICIENT_RESOURCES;
  }
  RtlCopyMemory(klogi->log_file.Buffer,in_log_file->Buffer,in_log_file->MaximumLength);
  klogi->log_file_size = in_log_file_size;

  NL_KLOG_KdPrint(("NL_KLOG_SetFile: signal\n"));
  KeSetEvent(&klogi->log_file_event,0,FALSE);

  return STATUS_SUCCESS;
}/* NL_KLOG_SetFile */
