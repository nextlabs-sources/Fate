/**********************************************************************************
 *
 * NL Tamper Resistance File System Filter Driver
 *
 * This driver handles IRP_MJ_CREATE and matches the file information to any
 * configured policies that have been set by user-mode.
 *
 *********************************************************************************/

#ifdef _AMD64_
#define _X86AMD64_
#endif

#include <fltKernel.h>
#include <wdm.h>

NTSTATUS ZwQueryInformationProcess( HANDLE ProcessHandle,
				    PROCESSINFOCLASS ProcessInformationClass,
				    PVOID ProcessInformation,
				    ULONG ProcessInformationLength,
				    PULONG ReturnLength );

#include "..\inc\nl_tamper.h"

#define NL_TAMPER_POOLTAG 'nltp'
#define NLFILT_MAX_POLICY 36
#define FILTER_FILEPATH_MIM_LENGTH 25

/* NLFILT_MAX_WORK_QUEUE_SIZE
 *
 * The work queue size for user-mode notification must not exceed this value.  When user-mode
 * does not accept/read events from this driver the work queue will contain the events just
 * before user-mode stops accepting/reading events.
 */
#define NLFILT_MAX_WORK_QUEUE_SIZE 32

#ifndef MIN
  #define MIN(a,b) ( (a) < (b) ? (a) : (b) )
#endif

/** PolicyEntry_t
 *
 *  Policy representation.
 */
typedef struct
{
  LIST_ENTRY list;
  NLFiltPolicy policy;
} PolicyEntry_t;

/** NotifyEntry_t
 *
 *  Information for user-mode notification.
 */
typedef struct
{
  LIST_ENTRY list;
  fs_msg_t fsm;      /* File information */
} NotifyEntry_t;

/** NL_TAMPER_GLOBAL_DATA
 *
 *  Global handles and state.
 */
typedef struct
{
  PFLT_FILTER FilterHandle;      /* filter handle */
  PFLT_PORT FilterPort;          /* filter comm port */
  PFLT_PORT ClientPort;          /* client comm port */
  int connected;                 /* client connected? */
  int policyCount;               /* the number of policy */
  LIST_ENTRY policy_list;        /* Policy list */
  FAST_MUTEX list_mutex;         /* Policy list mutext */
  LONG work_queue_size;          /* Work queue size */
	int bStoppable;
} NL_TAMPER_GLOBAL_DATA;

static NL_TAMPER_GLOBAL_DATA Globals;

DRIVER_INITIALIZE DriverEntry;
NTSTATUS DriverEntry ( PDRIVER_OBJECT DriverObject,
		       PUNICODE_STRING RegistryPath );
static FLT_PREOP_CALLBACK_STATUS PreCreateOp( PFLT_CALLBACK_DATA data,
					      PCFLT_RELATED_OBJECTS FltObjects,
					      PVOID *CompletionContext );
static void DisplayPolicy( const PolicyEntry_t* p );
static NTSTATUS FilterDriverUnload( FLT_FILTER_UNLOAD_FLAGS Flags );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(PAGE, PreCreateOp)
#pragma alloc_text(PAGE, DisplayPolicy)
#endif



/** EvaluateEvent
 *
 *  \brief Evaluate event to determine what how the filter should respond.
 *  \param ne (in) Event information in the form of a NotifyEntry_t structure.
 *  \return Flags to indicate behavior of filter.
 */
__checkReturn
static int EvaluateEvent( __in const NotifyEntry_t* ne )
{
  int flags = NL_TAMPER_FLAG_NONE;
  PLIST_ENTRY node = NULL;
  PLIST_ENTRY fExemptProc=NULL;
  int bFound=0;
  UNICODE_STRING policyFileNameUpcase;
  UNICODE_STRING fileName;
  UNICODE_STRING fileNameUpcase;

  USHORT ScannedLastBackslash = 0;
  USHORT ComparedPolicyStart = 0;
  USHORT ComparedFileStart = 0;
  USHORT PolicyComparedEndPlace = 0;
  USHORT i=0;
  USHORT j=0;
  BOOLEAN PrefixStringEqual = TRUE;
  BOOLEAN HasBackslash = FALSE;	

  ASSERT( ne != NULL );

  RtlInitUnicodeString(&fileName, ne->fsm.fname);
  RtlUpcaseUnicodeString(&fileNameUpcase, &fileName, TRUE);
 
  /* Process policy must be evaluated first */
  ExAcquireFastMutex(&Globals.list_mutex);
  for( node = Globals.policy_list.Flink ;
       node != &Globals.policy_list ;
       node = node->Flink )
  {
    PolicyEntry_t* pe = (PolicyEntry_t*)node;
    ASSERT( pe != NULL );

    switch( pe->policy.type )
    {
	case NL_FILT_POLICY_PROTECT_FILE:
		{
			RtlInitUnicodeString(&policyFileNameUpcase, pe->policy.fsm.fname);
			if(FILTER_FILEPATH_MIM_LENGTH < fileNameUpcase.Length / sizeof(WCHAR) && FILTER_FILEPATH_MIM_LENGTH < policyFileNameUpcase.Length / sizeof(WCHAR)) {				
				ScannedLastBackslash = 0;
				ComparedPolicyStart = 0;
				ComparedFileStart = 0;
				PrefixStringEqual = TRUE;

				for(i = 0; i < fileNameUpcase.Length / sizeof(WCHAR); i++)
				{
					if(L'\\' == fileNameUpcase.Buffer[i])
					{
						ScannedLastBackslash = i;
					}
					else if(L'~' == fileNameUpcase.Buffer[i])
					{
						//Whether the name is short
						if(7 == i - ScannedLastBackslash)
						{
							if((fileNameUpcase.Length > (i + 2) * sizeof(WCHAR) && L'\\' == fileNameUpcase.Buffer[i + 2]) || fileNameUpcase.Length == (i + 2) * sizeof(WCHAR)
								&& fileNameUpcase.Buffer[i + 1] != L'\\')
							{
								PolicyComparedEndPlace = ComparedPolicyStart + (i - ComparedFileStart) + 1;

								if(PolicyComparedEndPlace * sizeof(WCHAR) < policyFileNameUpcase.Length 
									&& policyFileNameUpcase.Buffer[PolicyComparedEndPlace] != L'\\' 
									&& policyFileNameUpcase.Buffer[PolicyComparedEndPlace + 1] != L'\\' 
									&& policyFileNameUpcase.Buffer[PolicyComparedEndPlace - 1] != L'\\')
								{
									if(RtlEqualMemory(policyFileNameUpcase.Buffer + ComparedPolicyStart, fileNameUpcase.Buffer + ComparedFileStart, (i - ComparedFileStart) * sizeof(WCHAR)))
									{
										HasBackslash = FALSE;

										for(j = PolicyComparedEndPlace + 2; j <= policyFileNameUpcase.Length / sizeof(WCHAR); j++)
										{
											if(policyFileNameUpcase.Buffer[j] == L'\\')
											{
												HasBackslash = TRUE;
												ComparedPolicyStart = j;
												break;
											}
										}

										if(!HasBackslash || fileNameUpcase.Length == (i + 2) * sizeof(WCHAR))
										{										
											if(!HasBackslash)
											{
												flags |= pe->policy.flags;
											}

											PrefixStringEqual = FALSE;
											break;
										}

										ComparedFileStart = i + 2;
										ScannedLastBackslash = ComparedFileStart;
										i += 2;
									}
									else
									{
										PrefixStringEqual = FALSE;
										break;
									}
								}
								else
								{
									PrefixStringEqual = FALSE;
									break;
								}
							}
							else if((i + 6) * sizeof(WCHAR) == fileNameUpcase.Length 
								&& fileNameUpcase.Buffer[i + 1] != L'\\' 
								&& fileNameUpcase.Buffer[i + 2] == L'.' 
								&& fileNameUpcase.Buffer[i + 3] != L'\\' 
								&& fileNameUpcase.Buffer[i + 4] != L'\\' 
								&& fileNameUpcase.Buffer[i + 5] != L'\\')
							{
								if ((ComparedPolicyStart + (i - ComparedFileStart) + 5) * sizeof(WCHAR) < policyFileNameUpcase.Length)
								{
									if (RtlEqualMemory(policyFileNameUpcase.Buffer + ComparedPolicyStart, fileNameUpcase.Buffer + ComparedFileStart, (i - ComparedFileStart) * sizeof(WCHAR)))
									{
										flags |= pe->policy.flags;
									}
								}

								PrefixStringEqual = FALSE;
								break;
							}
						}
					}
				}

				if (PrefixStringEqual)
				{
					if(fileNameUpcase.Length - ComparedFileStart * sizeof(WCHAR) >= policyFileNameUpcase.Length - ComparedPolicyStart * sizeof(WCHAR)
						&& RtlEqualMemory(policyFileNameUpcase.Buffer + ComparedPolicyStart, fileNameUpcase.Buffer + ComparedFileStart, policyFileNameUpcase.Length - ComparedPolicyStart * sizeof(WCHAR)))
					{
						flags |= pe->policy.flags;
					}
				}
			}
			break;
		}
      case NL_FILT_POLICY_FILE_EXEMPT_PROCESS_NAME:
      {
  /****************************************************************************
   * TBD: Once we find the offical way to get full path name of I/O request
   * process, we will complete this functionality. 
   ***************************************************************************/
		/*RtlInitUnicodeString(&policyFileNameUpcase, pe->policy.fsm.fname);
		if(RtlEqualMemory(policyFileNameUpcase->Buffer, fileNameUpcase->Buffer, policyFileNameUpcase->Length)) {
		  if( wcsstr(ne->fsm.pname,pe->policy.fsm.pname) != NULL ) {
			flags |= pe->policy.flags; 
			KdPrint(("NLTamper: Exempt Process: %04d %ws for file %ws\n", ne->fsm.pid, ne->fsm.pname, ne->fsm.fname));
			ExReleaseFastMutex(&Globals.list_mutex);
			RtlFreeUnicodeString(&fileNameUpcase);
			return flags; //short-circuit any deny operation
		  }
		}*/ 
		break;
      }
     case NL_FILT_POLICY_EXEMPT_PROCESS_ID:
      {
		if( pe->policy.fsm.pid == ne->fsm.pid ) {
		  flags |= pe->policy.flags;
		  ExReleaseFastMutex(&Globals.list_mutex);
		  RtlFreeUnicodeString(&fileNameUpcase);
		  return flags; //short-circuit any deny operation
		}
		break;
      }
      case NL_FILT_POLICY_EXEMPT_PROCESS_NAME:
      {
  /****************************************************************************
   * TBD: Once we find the offical way to get full path name of I/O request
   * process, we will complete this functionality. 
   ***************************************************************************/
		/*if( wcsstr(ne->fsm.pname, pe->policy.fsm.pname) != NULL ) {
		  flags |= pe->policy.flags;
		  //KdPrint(("NLTamper: Exempt Process: %04d %ws for policy %ws\n", ne->fsm.pid, ne->fsm.pname, pe->policy.fsm.pname));
		  ExReleaseFastMutex(&Globals.list_mutex);
		  RtlFreeUnicodeString(&fileNameUpcase);
		  return flags; //short-circuit any deny operation
		}*/
		break;
      }
	}/* switch */
  }/* for */
  ExReleaseFastMutex(&Globals.list_mutex);
  RtlFreeUnicodeString(&fileNameUpcase);
  return flags;
}/* EvaluateEvent */

/** wi_worker
 *
 *  Deferred queue work item worker callback.  PreOperation pends I/O to this callback.
 */
static VOID wi_worker( __in PFLT_GENERIC_WORKITEM work_item ,
		       __in PFLT_FILTER filter ,
		       __in PVOID context )
{
  NotifyEntry_t* ne = (NotifyEntry_t*)context;

  ASSERT( work_item != NULL && context != NULL );

  //KdPrint(("NLTamper: NOTIFY Process: %04d %ws\n", ne->fsm.pid, ne->fsm.pname));
  KdPrint(("NLTamper: NOTIFY Process: %04d\n", ne->fsm.pid));
  KdPrint(("NLTamper:        File   : %ws\n", ne->fsm.fname));

  if( Globals.connected )
  {
    LARGE_INTEGER li;
    const int timeout_ms = 10000; /* 10 second */
    NTSTATUS status;

    li.QuadPart = -((LONGLONG)timeout_ms * 10 * 1000);  /* (-) is time from now! */
    status = FltSendMessage(Globals.FilterHandle,&Globals.ClientPort,
			    &ne->fsm,sizeof(ne->fsm),NULL,NULL,&li);
    if( status == STATUS_TIMEOUT )
    {
      KdPrint(("NLTamper: nl_tamper_worker_thread: FltSendMessage timeout\n"));
    }
  }
  ExFreePoolWithTag(ne,NL_TAMPER_POOLTAG);
  FltFreeGenericWorkItem(work_item);
  InterlockedDecrement(&Globals.work_queue_size); // complete work item reduce queue size
}/* wi_worker */

/** PreCreateOp
 *
 *  \brief Pre-IRP_MJ_CREATE callback.  This filters the IPR_MJ_CREATE command to prevent
 *         access to restricted files.  No operations are pended here, each I/O either is
 *         allowed or denied based on a policy that has been set by user-mode.
 *
 *  \return Either (1) FLT_PREOP_SUCCESS_NO_CALLBACK
 *                     The I/O is passed to the next filter and/or down the stack.
 *                 (2) FLT_PREOP_COMPLETE when operation is denied.
 *                     The I/O is completed by this filter.  It is not passed to the next
 *                     filter or down the stack.
 */
__checkReturn static FLT_PREOP_CALLBACK_STATUS PreCreateOp(__inout PFLT_CALLBACK_DATA data,
					      __in PCFLT_RELATED_OBJECTS FltObjects,
					      __in PVOID *CompletionContext )
{
  PolicyEntry_t* policy = NULL;
  PFLT_FILE_NAME_INFORMATION fni = NULL;
  ULONG pid;
  FLT_PREOP_CALLBACK_STATUS cb_status = FLT_PREOP_SUCCESS_NO_CALLBACK;
  int flags = NL_TAMPER_FLAG_NONE;
  PEPROCESS eproc = NULL;
  NTSTATUS status;
  HANDLE handle;
  ULONG out_size;
  PUNICODE_STRING ustring = NULL;
  NotifyEntry_t* ne = NULL;

  PAGED_CODE();

  if( Globals.connected == 0 )
  {
    return FLT_PREOP_SUCCESS_NO_CALLBACK;
  }

  /* Non-IRP operations pass */
  if( FLT_IS_IRP_OPERATION(data) == FALSE )
  {
    return FLT_PREOP_SUCCESS_NO_CALLBACK;
  }

  if( FltGetFileNameInformation(data,FLT_FILE_NAME_OPENED|FLT_FILE_NAME_QUERY_DEFAULT,&fni) != STATUS_SUCCESS )
  {
    return FLT_PREOP_SUCCESS_NO_CALLBACK;
  }

#pragma prefast(disable:28197, "Possibly leaking memory 'ne'.")
  ne = (NotifyEntry_t*)ExAllocatePoolWithTag(PagedPool,
					     sizeof(NotifyEntry_t),
					     NL_TAMPER_POOLTAG);
#pragma prefast(enable:28197, "re-enable this warning")
  if( ne == NULL )
  {
    KdPrint(("NLTamper: ExAllocatePoolWithTag failed\n"));
    FltReleaseFileNameInformation(fni);
    data->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
    data->IoStatus.Information = 0;
    return FLT_PREOP_COMPLETE;
  }
  RtlZeroMemory(ne,sizeof(NotifyEntry_t));

  /****************************************************************************
   * Retrieve context information for IRP_MJ_CREATE event.  This includes PID,
   * image file path of originating process, and file name being accessed.
   ***************************************************************************/
  pid   = FltGetRequestorProcessId(data);    /* Process ID */
  eproc = FltGetRequestorProcess(data);      /* Process structure */
  if( eproc == NULL )
  {
    KdPrint(("NLTamper: FltGetRequestorProcess NULL\n"));
    FltReleaseFileNameInformation(fni);
    goto PreCreateOp_complete;
  }

  status = ObOpenObjectByPointer(eproc,OBJ_KERNEL_HANDLE,NULL,0,NULL,KernelMode,&handle);
  if( !NT_SUCCESS(status) )
  {
    KdPrint(("NLTamper: ObOpenObjectByPointer failed (0x%x)\n",status));
    FltReleaseFileNameInformation(fni);
    goto PreCreateOp_complete;
  }

  /* Get the required size for the name info buffer */
  status = ZwQueryInformationProcess(handle,ProcessImageFileName,NULL,0,&out_size);
  if( status != STATUS_INFO_LENGTH_MISMATCH )
  {
    KdPrint(("NLTamper: ZwQueryInformationProcess failed\n"));
    ZwClose(handle);
    FltReleaseFileNameInformation(fni);
    data->IoStatus.Status = status;
    data->IoStatus.Information = 0;
    cb_status = FLT_PREOP_COMPLETE;
    goto PreCreateOp_complete;
  }

  /* Allocate the actual name info buffer */
  ustring = ExAllocatePoolWithTag(PagedPool,out_size,NL_TAMPER_POOLTAG);
  if( ustring == NULL )
  {
    KdPrint(("NLTamper: ExAllocatePoolWithTag failed\n"));
    ZwClose(handle);
    FltReleaseFileNameInformation(fni);
    data->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
    data->IoStatus.Information = 0;
    cb_status = FLT_PREOP_COMPLETE;
    goto PreCreateOp_complete;
  }

  status = ZwQueryInformationProcess(handle,ProcessImageFileName,ustring,out_size,NULL); 
  ZwClose(handle);
  if( !NT_SUCCESS(status) )
  {
    KdPrint(("NLTamper: ZwQueryInformationProcess failed (0x%x)\n",status));
    //goto PreCreateOp_complete;
  } else {
	//copy process name if query is successful
	RtlCopyMemory(ne->fsm.pname,ustring->Buffer,MIN(sizeof(ne->fsm.pname),ustring->Length));
  }


  /* Copy file name, access and process ID */
  ne->fsm.pid = pid;
  RtlCopyMemory(ne->fsm.fname,fni->Name.Buffer,MIN(sizeof(ne->fsm.fname),fni->Name.Length));
  ne->fsm.access=data->Iopb->Parameters.Create.SecurityContext->DesiredAccess;

  /*****************************************************************************
   * Determine flags for the given event.  Perform actions based on 'flags' of
   * policies that match the event.
   ****************************************************************************/
  flags = EvaluateEvent(ne);

  FltReleaseFileNameInformation(fni);

  /* Ignore operation.  This must come first to short-circuit any potential deny.
   */
  if( flags & NL_TAMPER_FLAG_EXEMPT ) {
    goto PreCreateOp_complete;
  }

  /* Deny operation.  Deny occurs in two forms: (1) deny any access and (2) deny modify
   * access.  Modify access comes in the form of write, write attributes, append, and
   * delete.
   */
  if( flags & NL_TAMPER_FLAG_DENY )
  {
    KdPrint(("DesiredAccess 0x%x\n",data->Iopb->Parameters.Create.SecurityContext->DesiredAccess));
	if( flags & NL_TAMPER_FLAG_DENY_ALL )  {          /* deny all access? */
      data->IoStatus.Status = STATUS_ACCESS_DENIED;  /* setup I/O for completion */
      data->IoStatus.Information = 0;
      cb_status = FLT_PREOP_COMPLETE;
	} else if(flags & NL_TAMPER_FLAG_DENY_WRITE) {
      /* Evaluate IRP_MJ_CREATE flags to determine if the file may be modified
         by the requested access privilege. deny modify access? */
      ACCESS_MASK waccess = 0x0;             /* empty default */
      ACCESS_MASK is_waccess = 0x0;          /* empty default */

      waccess = FILE_WRITE_ACCESS|FILE_WRITE_EA|FILE_APPEND_DATA|FILE_WRITE_ATTRIBUTES|DELETE|FILE_WRITE_DATA;
      is_waccess = data->Iopb->Parameters.Create.SecurityContext->DesiredAccess & waccess;

	  if( is_waccess ) {/* access required can modify? */
		data->IoStatus.Status = STATUS_ACCESS_DENIED;
		data->IoStatus.Information = 0;
		cb_status = FLT_PREOP_COMPLETE;
      }
	}

	if(flags & NL_TAMPER_FLAG_NOTIFY && data->IoStatus.Status == STATUS_ACCESS_DENIED) {
	  /*It needs to notify user-mode for action deny*/
	  /* Notify user-mode.  Create a work-item and queue it for transmission to the
	   * connected user-mode thread.
       */
#if 0
	  PFLT_GENERIC_WORKITEM wi = FltAllocateGenericWorkItem();
      if( wi != NULL ) {

	        /* The queue is bound to [0,NLFILT_MAX_WORK_QUEUE_SIZE], so when outside of this bound
		 * the event is dropped.  This would happen if user-mode stops accepting/reading events.
		 */
	        LONG qsize = InterlockedIncrement(&Globals.work_queue_size);
		if( qsize < NLFILT_MAX_WORK_QUEUE_SIZE )
		{
		  status = FltQueueGenericWorkItem(wi,
						   (PVOID)FltObjects->Instance,
						   (PFLT_GENERIC_WORKITEM_ROUTINE)wi_worker,
						   DelayedWorkQueue,(PVOID)ne);
		  if( !NT_SUCCESS(status) ) {
		    FltFreeGenericWorkItem(wi);
		  } else {
		    ne = NULL;  /* wi_worker owns memory */
		  }
		}// if qsize bound exceeded
		else
		{
		  InterlockedDecrement(&Globals.work_queue_size);
		}
	  } 
#else
	  BOOLEAN bDecrement = FALSE;
	  LONG qsize = InterlockedIncrement(&Globals.work_queue_size);
	  if( qsize < NLFILT_MAX_WORK_QUEUE_SIZE )
	  {
		  PFLT_GENERIC_WORKITEM wi = FltAllocateGenericWorkItem();
		  if( wi != NULL ) 
		  {
			  status = FltQueueGenericWorkItem(wi,
				  (PVOID)FltObjects->Instance,
				  (PFLT_GENERIC_WORKITEM_ROUTINE)wi_worker,
				  DelayedWorkQueue,(PVOID)ne);
			  if( !NT_SUCCESS(status) ) 
			  {
				  FltFreeGenericWorkItem(wi);
				  bDecrement = TRUE;
			  } 
			  else 
			  {
				  ne = NULL;  /* wi_worker owns memory */
			  }
		  }
		  else	bDecrement = TRUE;
	  }
	  else	bDecrement = TRUE;

	  if(bDecrement)	InterlockedDecrement(&Globals.work_queue_size);
#endif
	} /*flags & NL_TAMPER_FLAG_NOTIFY && data->IoStatus.Status == STATUS_ACCESS_DENIED*/
  }/* NL_TAMPER_FLAG_DENY */

 PreCreateOp_complete:

  if( ustring != NULL )
  {
    ExFreePoolWithTag(ustring,NL_TAMPER_POOLTAG);
  }

  /* Non-NULL value means memory ownership did not go to wi_worker, so the object
     must be freed here.
  */
  if( ne != NULL )
  {
    ExFreePoolWithTag(ne,NL_TAMPER_POOLTAG);
  }

  return cb_status;
}/* PreCreateOp */

/* DisplayPolicy
 *
 * IRQL <= APC
 */
static void DisplayPolicy( __in const PolicyEntry_t* p )
{
  ASSERT( p != NULL );
  PAGED_CODE();

  switch( p->policy.type )
  {
    case NL_FILT_POLICY_PROTECT_FILE:
      KdPrint(("NLTamper: Policy: NL_FILT_POLICY_PROTECT_FILE: %ws (0x%x)\n",
	       p->policy.fsm.fname,p->policy.flags));
      break;
    case NL_FILT_POLICY_EXEMPT_PROCESS_ID:
      KdPrint(("NLTamper: Policy: NL_FILT_POLICY_PROCESS_ID: Process ID %d (0x%x)\n",
	       p->policy.fsm.pid,p->policy.flags));
      break;
    case NL_FILT_POLICY_EXEMPT_PROCESS_NAME:
      /*KdPrint(("NLTamper: Policy: NL_FILT_POLICY_EXEMPT_PROCESS_NAME: %ws (0x%x)\n",
	       p->policy.fsm.pname,p->policy.flags));*/
      break;
    case NL_FILT_POLICY_FILE_EXEMPT_PROCESS_NAME:
      /*KdPrint(("NLTamper: Policy: NL_FILT_POLICY_FILE_EXEMPT_PROCESS_NAME: %ws %ws(0x%x)\n",
	       p->policy.fsm.pname,p->policy.fsm.fname,p->policy.flags));*/
      break;
  }
}/* DisplayPolicy */

/** ClientMessage
 *
 *  \brief Handle commands from user-model FilterXXX calls.
 */
static NTSTATUS ClientMessage( __in PVOID ConnectionCookie,
			       __in_bcount(InputBufferSize) PVOID InputBuffer,
			       __in ULONG InputBufferSize,
			       __out_bcount_full(OutputBufferSize) PVOID OutputBuffer,
			       __in ULONG OutputBufferSize,
			       __out PULONG ReturnOutputBufferLength )
{
  NTSTATUS status = STATUS_SUCCESS;
  NLFiltCommand* cmd = NULL;
  PolicyEntry_t* pe = NULL;

	//	new comment, 2011-5-3
	//	I have added a new type for stoppable 
	if( InputBuffer && InputBufferSize == sizeof(NLFltUnlaodControl) )
	{
		NLFltUnlaodControl* pUnloadControl = (NLFltUnlaodControl*)InputBuffer;
		Globals.bStoppable = pUnloadControl->bStoppable;
		KdPrint(("NLTamper: have set stoppable flag in driver [%d]\n", Globals.bStoppable));
		return STATUS_SUCCESS;
	}

  if( InputBuffer == NULL || InputBufferSize != sizeof(NLFiltCommand) )
  {
    return STATUS_INVALID_PARAMETER;
  }

  cmd = (NLFiltCommand*)InputBuffer;

  switch( cmd->type )
  {
    case NL_FILT_COMMAND_ADD_POLICY:
    {
      pe = (PolicyEntry_t*)ExAllocatePoolWithTag(PagedPool,sizeof(PolicyEntry_t),NL_TAMPER_POOLTAG);

      if( pe == NULL )
      {
	status = STATUS_INSUFFICIENT_RESOURCES;
	goto ClientMessage_done;
      }

      RtlCopyMemory(&pe->policy,&cmd->policy,sizeof(pe->policy));  /* copy to new instance */
      ExAcquireFastMutex(&Globals.list_mutex);
	  if(Globals.policyCount < NLFILT_MAX_POLICY) {
	    InsertTailList(&Globals.policy_list,(PLIST_ENTRY)pe);        /* add to the policy list */
	    Globals.policyCount++;
	  } else {
		//Exceed the policy limit
		//Any new policy is not going to be added
		ExFreePoolWithTag(pe,NL_TAMPER_POOLTAG);
		status=STATUS_INSUFFICIENT_RESOURCES;
		KdPrint(("NLTamper: Exceed policy limit(%d)\n",NLFILT_MAX_POLICY));
	  }
      ExReleaseFastMutex(&Globals.list_mutex);
      DisplayPolicy(pe);
      break;
    }
  }/* switch */

 ClientMessage_done:
  return status;
}/* ClientMessage */

static NTSTATUS ClientConnect(__in PFLT_PORT ClientPort,
			      __in PVOID ServerPortCookie,
			      __in_bcount(SizeOfContext) PVOID ConnectionContext,
			      __in ULONG SizeOfContext,
			      __out PVOID *ConnectionCookie )
{
  if( Globals.connected == 0 )
  {
    Globals.ClientPort = ClientPort;
  }
  Globals.connected++;
	KdPrint(("NLTamper: calling ClientConnect: connected handle [%d]\n", Globals.connected));
  return STATUS_SUCCESS;
}/* ClientConnect */

static VOID ClientDisconnect( PVOID ConnectionCookie )
{
	KdPrint(("NLTamper: start to call ClientDisconnect: connected handle [%d]\n", Globals.connected));
  Globals.connected--;
  if(Globals.connected == 0) {
   /****************************************************************
    * Clear the policy list
    ***************************************************************/
    ExAcquireFastMutex(&Globals.list_mutex);
    while( IsListEmpty(&Globals.policy_list) == FALSE )  /* Free policy list */
    {
      PLIST_ENTRY node = RemoveHeadList(&Globals.policy_list);
      ExFreePoolWithTag(node,NL_TAMPER_POOLTAG);
    }
    Globals.policyCount=0;
    ExReleaseFastMutex(&Globals.list_mutex);	
  }
}/* ClientDisconnect */

#ifdef ALLOC_PRAGMA
  #pragma alloc_text(INIT,DriverEntry)  /* remove from memory after init */
#endif

static NTSTATUS InstanceSetup( __in PCFLT_RELATED_OBJECTS FltObjects,
			       __in FLT_INSTANCE_SETUP_FLAGS Flags,
			       __in DEVICE_TYPE VolumeDeviceType,
			       __in FLT_FILESYSTEM_TYPE VolumeFilesystemType )
{
  if( VolumeDeviceType != FILE_DEVICE_DISK_FILE_SYSTEM ) /* Only want disks */
  {
    return STATUS_FLT_DO_NOT_ATTACH;
  }
  return STATUS_SUCCESS;
}/* InstanceSetup */

NTSTATUS DriverEntry(__in PDRIVER_OBJECT DriverObject,
		      __in PUNICODE_STRING RegistryPath )
{
  FLT_OPERATION_REGISTRATION_FLAGS flags =
    FLTFL_OPERATION_REGISTRATION_SKIP_CACHED_IO |
    FLTFL_OPERATION_REGISTRATION_SKIP_PAGING_IO;

  FLT_OPERATION_REGISTRATION callbacks[] =
    {
      { IRP_MJ_CREATE , flags , (PFLT_PRE_OPERATION_CALLBACK)PreCreateOp , NULL },
      { IRP_MJ_OPERATION_END }
    };

  FLT_REGISTRATION filterRegistration =
    {
      sizeof(FLT_REGISTRATION),      //  Size
      FLT_REGISTRATION_VERSION,      //  Version
			FLTFL_REGISTRATION_DO_NOT_SUPPORT_SERVICE_STOP,                             //  Flags, disable mandatory unload
      NULL,                          //  Context
      callbacks,                     //  Operation callbacks
      (PFLT_FILTER_UNLOAD_CALLBACK)FilterDriverUnload,  //  Filters unload routine
      (PFLT_INSTANCE_SETUP_CALLBACK)InstanceSetup, //  InstanceSetup routine
      NULL,NULL,NULL,NULL,NULL,NULL 
    };

  OBJECT_ATTRIBUTES ObjectAttributes;
  NTSTATUS status;
  PSECURITY_DESCRIPTOR sd = NULL;
  OBJECT_ATTRIBUTES oa;
  UNICODE_STRING uniString;

  RtlSecureZeroMemory(&Globals,sizeof(Globals));
	Globals.bStoppable = 0;
  Globals.work_queue_size = 0L;
  ExInitializeFastMutex(&Globals.list_mutex);
  InitializeListHead((PLIST_ENTRY)&Globals.policy_list);

  InitializeObjectAttributes(&ObjectAttributes,NULL,OBJ_KERNEL_HANDLE,NULL,NULL);

  status = FltRegisterFilter(DriverObject,&filterRegistration,&Globals.FilterHandle);
  if( !NT_SUCCESS(status) )
  {
    KdPrint(("NLTamper: FltRegisterFilter failed (0x%x)\n",status));
    return status;
  }

  status  = FltBuildDefaultSecurityDescriptor(&sd,FLT_PORT_ALL_ACCESS);
  if( !NT_SUCCESS(status) )
  {
    KdPrint(("NLTamper: FltBuildDefaultSecurityDescriptor failed (0x%x)\n",status));
    FltUnregisterFilter(Globals.FilterHandle);
    goto DriverEntry_complete;
  }

  RtlInitUnicodeString(&uniString,NLTAMPER_PORT_NAME);
  InitializeObjectAttributes(&oa,&uniString,OBJ_KERNEL_HANDLE|OBJ_CASE_INSENSITIVE,NULL,sd);
  status = FltCreateCommunicationPort(Globals.FilterHandle,
				      &Globals.FilterPort,&oa,NULL,
				      (PFLT_CONNECT_NOTIFY)ClientConnect,
				      (PFLT_DISCONNECT_NOTIFY)ClientDisconnect,
				      (PFLT_MESSAGE_NOTIFY)ClientMessage,10);
  FltFreeSecurityDescriptor(sd);
  if( !NT_SUCCESS(status) )
  {
    KdPrint(("NLTamper: FltCreateCommunicationPort failed (0x%x)\n",status));
    FltUnregisterFilter(Globals.FilterHandle);
    return status;
  }

  status = FltStartFiltering(Globals.FilterHandle);
  if( !NT_SUCCESS(status) )
  {
    KdPrint(("NLTamper: FltStartFiltering failed (0x%x)\n",status));
    FltUnregisterFilter(Globals.FilterHandle);
    return status;
  }

 DriverEntry_complete:
  return status;
}/* DriverEntry */

/** FilterDriverUnload
 *
 *  Stop filtering and release all claimed resources.
 */
static NTSTATUS FilterDriverUnload(__in FLT_FILTER_UNLOAD_FLAGS Flags )
{
  NTSTATUS status;

	if (Globals.bStoppable == 0)
	{
		KdPrint(("NLTamper: FilterDriverUnload: don't allow unload because stoppable flag is not enabled\n"));
		return STATUS_FLT_DO_NOT_DETACH;
	}
	

  if( Globals.connected )
  {
    KdPrint(("NLTamper: FilterDriverUnload: FltCloseClientPort\n"));
    FltCloseClientPort(Globals.FilterHandle,&Globals.ClientPort);
  }

  KdPrint(("NLTamper: FilterDriverUnload: FltCloseCommunicationPort\n"));
  FltCloseCommunicationPort(Globals.FilterPort);
  Globals.FilterPort = NULL;

  /* Unregister of filter should block until all pending I/O is completed
   * by the filter.
   */
  FltUnregisterFilter(Globals.FilterHandle);
  Globals.FilterHandle = NULL;

  /****************************************************************
   * Clear the lists
   ***************************************************************/
  ExAcquireFastMutex(&Globals.list_mutex);
  while( IsListEmpty(&Globals.policy_list) == FALSE )  /* Free policy list */
  {
    PLIST_ENTRY node = RemoveHeadList(&Globals.policy_list);
    ExFreePoolWithTag(node,NL_TAMPER_POOLTAG);
  }
  Globals.policyCount=0;
  ExReleaseFastMutex(&Globals.list_mutex);

  return STATUS_SUCCESS;
}/* FilterDriverUnload */
