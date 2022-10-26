/*++

Module Name:

NLSEComm.c

Abstract:
Process Communication between user mode and kernel

Environment:
Kernel mode

--*/
#include "NLSEStruct.h"
#include "NLSEUtility.h"
#include "NLSERawAccess.h"
#include "NLSEDrmPathList.h"
#include "NLSEDrmFileList.h"

//
//  Global variables
//
extern NLFSE_GLOBAL_DATA nlfseGlobal;
extern NL_KLOG nlseKLog;
extern ULONG nlseLoggingFlags;

//key management
//If the nlfseGlobal.currentPCKey is expired, send a request to get
//the current PC key. 
NTSTATUS
NLSEUpdateCurrentPCKey(
                       __in ULONG pid,
                       __in BOOLEAN UseCache
                       )
{
    LARGE_INTEGER                 currentTime;
    ULONG                         currentSecond;
    PNLSE_KERNEL_REQUEST          req=NULL;
    PNLSE_KERNEL_REQUEST_RESPONSE response=NULL;
    ULONG                         responseLen;
    NTSTATUS                      status = STATUS_SUCCESS;
    LARGE_INTEGER                 timeout;	
    PNLSE_PORT_CONTEXT            portCtx = NULL;
    STRING                        tmpString;

#if NLSE_DEBUG_FAKE_PC_KEY
    RtlFillMemory(nlfseGlobal.currentPCKey,NLE_KEY_LENGTH_IN_BYTES,'a');
    nlfseGlobal.hasCurrentPCKey = TRUE;
    return STATUS_SUCCESS;
#endif

#ifdef	NLSE_DEBUG_PERFORMANCE
	NLPERFORMANCE_COUNTER pfc;
	PfStart(
		&pfc
		);
#endif

    ASSERT( KeGetCurrentIrql() < DISPATCH_LEVEL );

    // get current time
    currentTime = GetCurrentTime();
    RtlTimeToSecondsSince1980(&currentTime,&currentSecond);

    // Use cache: Check if the cached PC key expired
    if(UseCache && (currentSecond <= nlfseGlobal.keyRefreshTime))
    {
        status = nlfseGlobal.hasCurrentPCKey?STATUS_SUCCESS:STATUS_GENERIC_COMMAND_FAILED;
        goto _exit;
    }

    // Otherwise, try to get PC key from Policy Controller
    req=ExAllocatePoolWithTag(PagedPool, sizeof(NLSE_KERNEL_REQUEST), NLSE_MESSAGE_TAG);
    if(req==NULL)
    {
        NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_ERR, "NLSE!UpdateCurrentPCKey (%d): failed to allocate request\n", pid);
        nlfseGlobal.keyRefreshTime = currentSecond + 1;
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto _exit;
    }
    responseLen=sizeof(FILTER_REPLY_HEADER)+sizeof(NLSE_MESSAGE);
    response=ExAllocatePoolWithTag(PagedPool, responseLen, NLSE_MESSAGE_TAG);
    if(response==NULL)
    {
        NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_ERR, "NLSE!UpdateCurrentPCKey (%d): failed to allocate response\n", pid);
        ExFreePoolWithTag(req, NLSE_MESSAGE_TAG);
        nlfseGlobal.keyRefreshTime = currentSecond + 1;
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto _exit;
    }
    RtlZeroMemory(response,responseLen);
    RtlZeroMemory(req,sizeof(NLSE_KERNEL_REQUEST));

    portCtx=&(nlfseGlobal.clientPorts[NLSE_PORT_MAIN_REQUEST_INDEX]);
    if(portCtx->port == NULL)
    {
        ExFreePoolWithTag(req, NLSE_MESSAGE_TAG);
        ExFreePoolWithTag(response, NLSE_MESSAGE_TAG);
        NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_ERR, "NLSE!UpdateCurrentPCKey (%d): main port not connected\n", pid);
        nlfseGlobal.keyRefreshTime = currentSecond + 1;
        status = STATUS_PORT_DISCONNECTED;
        goto _exit;
    }

    //compose request
    req->type=NLSE_KERNEL_REQ_GET_KEY;
    RtlInitString(&tmpString, NLE_KEY_RING_LOCAL);
    RtlCopyMemory(req->msg.keyRingName, tmpString.Buffer, tmpString.Length);
    req->msg.pid=pid;
    responseLen=sizeof(NLSE_MESSAGE);
    timeout.QuadPart=NLSE_DELAY_ONE_SECOND*5;

    //send request and get response
    status = FltSendMessage(nlfseGlobal.filterHandle,
        &((PFLT_PORT)portCtx->port),
        &req->type,
        sizeof(int)+sizeof(NLSE_MESSAGE),
        &response->msg,
        &responseLen,
        &timeout);
    if(status != STATUS_SUCCESS)
    {
        ExFreePoolWithTag(req, NLSE_MESSAGE_TAG);
        ExFreePoolWithTag(response, NLSE_MESSAGE_TAG);
        //NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_ERR, "NLSE!UpdateCurrentPCKey: FltSendMessage failed 0x%x\n", status);
        if(status == STATUS_TIMEOUT)
        {
            //Use a longer refresh time if the error is a timeout, because timeout
            //may be caused by the PC side being too busy.
            nlfseGlobal.keyRefreshTime = currentSecond + 20;
        }
        else
        {
            nlfseGlobal.keyRefreshTime = currentSecond + 1;
        }
        goto _exit;
    }

    if(responseLen == sizeof(NLSE_MESSAGE))
    {
        if(response->msg.result == NLSE_KERNEL_REQ_RESULT_SUCCESS)
        {
            RtlCopyMemory(&(nlfseGlobal.currentPCKeyID), &(response->msg.keyID), sizeof(NLSE_KEY_ID));
            RtlCopyMemory(nlfseGlobal.currentPCKey, response->msg.key, NLE_KEY_LENGTH_IN_BYTES);
            nlfseGlobal.hasCurrentPCKey = TRUE;
            nlfseGlobal.keyRefreshTime = currentSecond + NLSE_KEY_CACHE_TIME;
        }
        else
        {
            NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_ERR, "NLSE!UpdateCurrentPCKey (%d): Get key request failed %d\n", pid, response->msg.result);
            //Erase the stored current key regardless of the reason of failure
            //(comm error, no key exists in key ring, etc.)
            nlfseGlobal.hasCurrentPCKey = FALSE;
            nlfseGlobal.keyRefreshTime = currentSecond + 1;
            status = STATUS_GENERIC_COMMAND_FAILED;
        }
        ExFreePoolWithTag(req, NLSE_MESSAGE_TAG);
        ExFreePoolWithTag(response, NLSE_MESSAGE_TAG);
    }
    else
    {
        ExFreePoolWithTag(req, NLSE_MESSAGE_TAG);
        ExFreePoolWithTag(response, NLSE_MESSAGE_TAG);
        NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_ERR, "NLSE!UpdateCurrentPCKey (%d): wrong size response %d\n", pid, responseLen);
        nlfseGlobal.keyRefreshTime = currentSecond + 1;
        status = STATUS_INVALID_BUFFER_SIZE;
    }

_exit:

#ifdef	NLSE_DEBUG_PERFORMANCE
	PfEnd(
		&pfc
		);
	if (pfc.diff.QuadPart / 1000)
		KdPrint(("NLSEUpdateCurrentPCKey elasped time = %I64d milliseconds\n", pfc.diff.QuadPart / 1000));
#endif

    return status;
}

//Get the PC key using PC key ID
//return TRUE on success
BOOLEAN NLSEGetPCKeyByID(__in    char         *keyRingName,
                         __in    NLSE_KEY_ID  *keyID,
                         __in    ULONG        pid,
                         __inout char  *key)
{
    PNLSE_KERNEL_REQUEST          req=NULL;
    PNLSE_KERNEL_REQUEST_RESPONSE response=NULL;
    ULONG                         responseLen;
    NTSTATUS                      status;
    LARGE_INTEGER                 timeout;	
    PNLSE_PORT_CONTEXT            portCtx = NULL;

#if NLSE_DEBUG_FAKE_PC_KEY
    RtlFillMemory(key,NLE_KEY_LENGTH_IN_BYTES,'a');
    return TRUE;
#endif

#ifdef	NLSE_DEBUG_PERFORMANCE
	NLPERFORMANCE_COUNTER pfc;
	PfStart(
		&pfc
		);
#endif

    //Sending a request to user mode
    req=ExAllocatePoolWithTag(NonPagedPool,
        sizeof(NLSE_KERNEL_REQUEST),
        NLSE_MESSAGE_TAG);
    if(req==NULL)
    {
        NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_ERR, "NLSE!GetPCKeyByID (%d): failed to allocate request\n", pid);
        return FALSE;
    }
    responseLen=sizeof(FILTER_REPLY_HEADER)+sizeof(NLSE_MESSAGE);
    response=ExAllocatePoolWithTag(NonPagedPool,
        responseLen,
        NLSE_MESSAGE_TAG);
    if(response==NULL)
    {
        NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_ERR, "NLSE!GetPCKeyByID (%d): failed to allocate response\n", pid);
        ExFreePoolWithTag(req, NLSE_MESSAGE_TAG);
        return FALSE;
    }
    RtlZeroMemory(response,responseLen);
    RtlZeroMemory(req,sizeof(NLSE_KERNEL_REQUEST));

    portCtx=&(nlfseGlobal.clientPorts[NLSE_PORT_MAIN_REQUEST_INDEX]);
    if(portCtx->port == NULL)
    {
        ExFreePoolWithTag(req, NLSE_MESSAGE_TAG);
        ExFreePoolWithTag(response, NLSE_MESSAGE_TAG);
        NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_ERR, "NLSE!GetPCKeyByID (%d): main port not connected\n", pid);
        return FALSE;
    }

    //compose request
    req->type=NLSE_KERNEL_REQ_GET_KEY;
    RtlCopyMemory(req->msg.keyRingName, keyRingName, NLE_KEY_RING_NAME_MAX_LEN*sizeof(char));
    RtlCopyMemory(&(req->msg.keyID), keyID, sizeof(NLSE_KEY_ID));
    req->msg.pid=pid;
    responseLen=sizeof(NLSE_MESSAGE);
    req->header.ReplyLength=sizeof(NLSE_MESSAGE);
    timeout.QuadPart=NLSE_DELAY_ONE_SECOND*5;

    //send request and get response
    status = FltSendMessage(nlfseGlobal.filterHandle,
        &((PFLT_PORT)portCtx->port),
        &req->type,
        sizeof(int)+sizeof(NLSE_MESSAGE),
        &response->msg,
        &responseLen,
        &timeout);
    if(status != STATUS_SUCCESS)
    {
        ExFreePoolWithTag(req, NLSE_MESSAGE_TAG);
        ExFreePoolWithTag(response, NLSE_MESSAGE_TAG);
        NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_ERR, "NLSE!GetPCKeyByID (%d): FltSendMessage failed 0x%x\n", pid, status);

#ifdef	NLSE_DEBUG_PERFORMANCE
		PfEnd(
			&pfc
			);
		if (pfc.diff.QuadPart / 1000)
			KdPrint(("NLSEGetPCKeyByID elasped time = %I64d milliseconds\n", pfc.diff.QuadPart / 1000));
#endif

        return FALSE;
    }

    if(responseLen == sizeof(NLSE_MESSAGE))
    {
        if(response->msg.result == NLSE_KERNEL_REQ_RESULT_SUCCESS)
        {
            RtlCopyMemory(key, response->msg.key, NLE_KEY_LENGTH_IN_BYTES);
        }
        else
        {
            NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_ERR, "NLSE!GetPCKeyByID (%d): Get key request failed %d\n", pid, response->msg.result);
            ExFreePoolWithTag(req, NLSE_MESSAGE_TAG);
            ExFreePoolWithTag(response, NLSE_MESSAGE_TAG);

#ifdef	NLSE_DEBUG_PERFORMANCE
			PfEnd(
				&pfc
				);
			if (pfc.diff.QuadPart / 1000)
				KdPrint(("NLSEGetPCKeyByID elasped time = %I64d milliseconds\n", pfc.diff.QuadPart / 1000));
#endif

            return FALSE;
        }
    }
    else
    {
        ExFreePoolWithTag(req, NLSE_MESSAGE_TAG);
        ExFreePoolWithTag(response, NLSE_MESSAGE_TAG);
        NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_ERR, 
            "NLSE!GetPCKeyByID (%d): wrong size response %d %d\n",
            pid,
            responseLen,
            sizeof(NLSE_MESSAGE));

#ifdef	NLSE_DEBUG_PERFORMANCE
		PfEnd(
			&pfc
			);
		if (pfc.diff.QuadPart / 1000)
			KdPrint(("NLSEGetPCKeyByID elasped time = %I64d milliseconds\n", pfc.diff.QuadPart / 1000));
#endif

        return FALSE;
    }

#ifdef	NLSE_DEBUG_PERFORMANCE
	PfEnd(
		&pfc
		);
	if (pfc.diff.QuadPart / 1000)
		KdPrint(("NLSEGetPCKeyByID elasped time = %I64d milliseconds\n", pfc.diff.QuadPart / 1000));
#endif

    ExFreePoolWithTag(req, NLSE_MESSAGE_TAG);
    ExFreePoolWithTag(response, NLSE_MESSAGE_TAG);
    return TRUE;
}
//end - key management

#ifdef _WIN64

static VOID ThunkPortCtx(__in   const PNLSE_PORT_CONTEXT32 portCtx32,
                         __out  PNLSE_PORT_CONTEXT portCtx)
{
    portCtx->port = portCtx32->port;
    portCtx->portTag = portCtx32->portTag;
}

static BOOLEAN ThunkUserCommand(__in    const PNLSE_USER_COMMAND32 cmd32,
                                __out   PNLSE_USER_COMMAND cmd)
{
    ULONG i;

    cmd->type = cmd32->type;
    RtlCopyMemory(cmd->msg.fname, cmd32->msg.fname, sizeof cmd->msg.fname);
    cmd->msg.keyID = cmd32->msg.keyID;
    RtlCopyMemory(cmd->msg.keyRingName, cmd32->msg.keyRingName,
        sizeof cmd->msg.keyRingName);
    RtlCopyMemory(cmd->msg.key, cmd32->msg.key, sizeof cmd->msg.key);
    cmd->msg.pid = cmd32->msg.pid;

    switch (cmd32->type)
    {
    case NLSE_USER_COMMAND_CREATE_FILE_RAW:
        cmd->msg.params.createFileRaw.desiredAccess =
            cmd32->msg.params.createFileRaw.desiredAccess;
        cmd->msg.params.createFileRaw.fileAttributes =
            cmd32->msg.params.createFileRaw.fileAttributes;
        cmd->msg.params.createFileRaw.shareAccess =
            cmd32->msg.params.createFileRaw.shareAccess;
        cmd->msg.params.createFileRaw.createDisposition =
            cmd32->msg.params.createFileRaw.createDisposition;

        cmd->msg.params.createFileRaw.pHandle =
            ExAllocatePoolWithTag(PagedPool, sizeof(HANDLE), NLSE_MESSAGE_TAG);
        if (cmd->msg.params.createFileRaw.pHandle == NULL)
        {
            NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_ERR, "NLSE!ThunkUserCommand: failed to allocate buffer\n");
            return FALSE;
        }

        cmd->msg.params.createFileRaw.status =
            cmd32->msg.params.createFileRaw.status;
        break;

    case NLSE_USER_COMMAND_READ_FILE_RAW:
        cmd->msg.params.readFileRaw.handle =
            cmd32->msg.params.readFileRaw.handle;
        cmd->msg.params.readFileRaw.offset =
            cmd32->msg.params.readFileRaw.offset;
        cmd->msg.params.readFileRaw.len =
            cmd32->msg.params.readFileRaw.len;
        cmd->msg.params.readFileRaw.buf =
            cmd32->msg.params.readFileRaw.buf;
        cmd->msg.params.readFileRaw.bufSize =
            cmd32->msg.params.readFileRaw.bufSize;
        cmd->msg.params.readFileRaw.bytesRead =
            cmd32->msg.params.readFileRaw.bytesRead;
        cmd->msg.params.readFileRaw.status =
            cmd32->msg.params.readFileRaw.status;
        break;

    case NLSE_USER_COMMAND_WRITE_FILE_RAW:
        cmd->msg.params.writeFileRaw.handle =
            cmd32->msg.params.writeFileRaw.handle;
        cmd->msg.params.writeFileRaw.offset =
            cmd32->msg.params.writeFileRaw.offset;
        cmd->msg.params.writeFileRaw.len =
            cmd32->msg.params.writeFileRaw.len;
        cmd->msg.params.writeFileRaw.buf =
            cmd32->msg.params.writeFileRaw.buf;
        cmd->msg.params.writeFileRaw.bufSize =
            cmd32->msg.params.writeFileRaw.bufSize;
        cmd->msg.params.writeFileRaw.bytesWritten =
            cmd32->msg.params.writeFileRaw.bytesWritten;
        cmd->msg.params.writeFileRaw.status =
            cmd32->msg.params.writeFileRaw.status;
        break;

    case NLSE_USER_COMMAND_CLOSE_FILE_RAW:
        cmd->msg.params.closeFileRaw.handle =
            cmd32->msg.params.closeFileRaw.handle;
        cmd->msg.params.closeFileRaw.status =
            cmd32->msg.params.closeFileRaw.status;
        break;

    case NLSE_USER_COMMAND_SET_DRM_PATHS:
        cmd->msg.params.setDrmPaths.numPaths =
            cmd32->msg.params.setDrmPaths.numPaths;
        cmd->msg.params.setDrmPaths.paths =
            cmd32->msg.params.setDrmPaths.paths;
        break;
    }

    return TRUE;
}

static VOID DeThunkUserCommand(__in     const PNLSE_USER_COMMAND cmd,
                               __out    PNLSE_USER_COMMAND32 cmd32)
{
    cmd32->msg.result = cmd->msg.result;

    switch (cmd->type)
    {
    case NLSE_USER_COMMAND_CREATE_FILE_RAW:
        *cmd32->msg.params.createFileRaw.pHandle =
            (VOID * POINTER_32) *cmd->msg.params.createFileRaw.pHandle;
        ExFreePoolWithTag(cmd->msg.params.createFileRaw.pHandle, NLSE_MESSAGE_TAG);
        break;
    }
}

#endif /* _WIN64 */

/** ClientMessage
*
*  \brief Handle commands from user-model FilterXXX calls.
*/
NTSTATUS NLSEClientMessage( __in PVOID ConnectionCookie,
                           __in_opt PVOID InputBuffer,
                           __in ULONG InputBufferSize,
                           __out PVOID OutputBuffer,
                           __in ULONG OutputBufferSize,
                           __out PULONG ReturnOutputBufferLength )
{
    PNLSE_PORT_CONTEXT portCtx=(PNLSE_PORT_CONTEXT)ConnectionCookie;
    NTSTATUS           status = STATUS_SUCCESS;
    NLSE_USER_COMMAND  *cmd = NULL;
#ifdef _WIN64
    NLSE_USER_COMMAND  cmdBuf;
#endif /* WIN64 */

#ifdef	NLSE_DEBUG_PERFORMANCE
	NLPERFORMANCE_COUNTER pfc;
	PfStart(
		&pfc
		);
#endif

    // Sanity check
    if(NULL == portCtx)
        return STATUS_INVALID_PARAMETER;;

    if( InputBuffer == NULL )
    {
        NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_ERR, ("NLSE!NLSEClientMessage: input buffer is NULL\n"));
        return STATUS_INVALID_PARAMETER;
    }

#ifdef _WIN64
    RtlZeroMemory(&cmdBuf, sizeof cmdBuf);    // just to surpress PREfast warning
    if( IoIs32bitProcess(NULL) )
    {
        if( InputBufferSize != sizeof(NLSE_USER_COMMAND32) )
        {
            NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_ERR, ("NLSE!NLSEClientMessage: input buffer size is wrong\n"));
            return STATUS_INVALID_PARAMETER;
        }

        if (!ThunkUserCommand(InputBuffer, &cmdBuf))
        {
            return STATUS_INSUFFICIENT_RESOURCES;
        }
        cmd = &cmdBuf;
    }
    else
    {
#endif /* _WIN64 */
        if( InputBufferSize != sizeof(NLSE_USER_COMMAND) )
        {
            NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_ERR, ("NLSE!NLSEClientMessage: input buffer size is wrong\n"));
            return STATUS_INVALID_PARAMETER;
        }

        cmd = (PNLSE_USER_COMMAND)InputBuffer;
#ifdef _WIN64
    }
#endif /* _WIN64 */

    if(portCtx->portTag == NLSE_PORT_TAG_MAIN_CMD)
    {
        //command to main port 
        switch( cmd->type )
        {
        case NLSE_USER_COMMAND_ENABLE_FILTER:
            {
                ExAcquireFastMutex(&nlfseGlobal.enableStatusMutex);
                nlfseGlobal.bEnable=TRUE;
                ExReleaseFastMutex(&nlfseGlobal.enableStatusMutex);
                break;
            }
        case NLSE_USER_COMMAND_DISABLE_FILTER:
            {
                ExAcquireFastMutex(&nlfseGlobal.enableStatusMutex);
                nlfseGlobal.bEnable=FALSE;
                ExReleaseFastMutex(&nlfseGlobal.enableStatusMutex);
                break;
            }
        default:
            break;
        }/* switch */
    }
    else if(portCtx->portTag == NLSE_PORT_TAG_DRM)
    {
        //DRM command
        switch( cmd->type )
        {
        case NLSE_USER_COMMAND_SET_IGNORE_PROCESS_BY_PID:
            {
                break;
            }	
        case NLSE_USER_COMMAND_UNSET_IGNORE_PROCESS_BY_PID:
            {
                break;
            }	
        case NLSE_USER_COMMAND_SET_DRM_FILE:
            {
                break;
            }
        case NLSE_USER_COMMAND_CREATE_FILE_RAW:
            {
                NLSECreateFileRaw(&cmd->msg);
                break;
            }
        case NLSE_USER_COMMAND_READ_FILE_RAW:
            {
                NLSEReadFileRaw(&cmd->msg);
                break;
            }
        case NLSE_USER_COMMAND_WRITE_FILE_RAW:
            {
                NLSEWriteFileRaw(&cmd->msg);
                break;
            }
        case NLSE_USER_COMMAND_CLOSE_FILE_RAW:
            {
                NLSECloseFileRaw(&cmd->msg);
                break;
            }
        case NLSE_USER_COMMAND_SET_DRM_PATHS:
            {
                NTSTATUS NewStatus;
                UNICODE_STRING  ConfigFileName;


                status = NLSEDrmPathListSet(cmd->msg.params.setDrmPaths.numPaths,
                    cmd->msg.params.setDrmPaths.paths);

                // When DRM path need to be set, the configure must be changed
                // So we need to re-read the config file
                // Since we already get C: drive instance, so we can pass NULL as input Instance
                RtlInitUnicodeString(&ConfigFileName,
                                     L"\\??\\C:\\Program Files\\NextLabs\\System Encryption\\config\\SystemEncryption.cfg");
                NewStatus = NxReadNonDrmList(nlfseGlobal.filterHandle, NULL, &ConfigFileName);
                if(!NT_SUCCESS(NewStatus))
                {
                    KdPrint(("Fail to read NonDRMDirectory from config file\n"));
                }
                break;
            }
        case NLSE_USER_COMMAND_ADD_DRM_FILE_ONE_SHOT:
            {
                status = NLSEDrmFileOneShotListAdd(cmd->msg.fname,
                                                   cmd->msg.pid);
                break;
            }
        case NLSE_USER_COMMAND_REMOVE_DRM_FILE_ONE_SHOT:
            {
                status = NLSEDrmFileOneShotListRemove(cmd->msg.fname,
                                                      cmd->msg.pid);
                break;
            }
        case NLSE_USER_COMMAND_REMOVE_ALL_DRM_FILES_ONE_SHOT:
            {
                status = NLSEDrmFileOneShotListRemoveAll(cmd->msg.pid);
                break;
            }
        default:
            break;
        }/* switch */
    }

#ifdef _WIN64
    if( IoIs32bitProcess(NULL) )
    {
        DeThunkUserCommand(&cmdBuf, InputBuffer);
    }
#endif /* _WIN64 */

#ifdef	NLSE_DEBUG_PERFORMANCE
	PfEnd(
		&pfc
		);
	if (pfc.diff.QuadPart / 1000)
		KdPrint(("NLSEClientMessage elasped time = %I64d milliseconds\n", pfc.diff.QuadPart / 1000));
#endif

    return status;
}/* ClientMessage */

/** CheckAndSetDebugMode
*/
__drv_requiresIRQL(PASSIVE_LEVEL)
CheckAndSetDebugMode(VOID)
{
    OBJECT_ATTRIBUTES attributes;
    HANDLE driverRegKey;
    NTSTATUS status;
    ULONG resultLength;
    UNICODE_STRING valueName;
    UCHAR buffer[sizeof( KEY_VALUE_PARTIAL_INFORMATION ) + sizeof( LONG )] = {0};
    UNICODE_STRING nl_sw_path;
    ULONG debug_mode = 0;

    RtlInitUnicodeString(&nl_sw_path,L"\\Registry\\Machine\\Software\\NextLabs");
    InitializeObjectAttributes(&attributes,&nl_sw_path,OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL,NULL);
    status = ZwOpenKey(&driverRegKey,KEY_READ,&attributes);
    if( NT_SUCCESS(status) )
    {
        // Read the given value from the registry.
        RtlInitUnicodeString(&valueName,L"DebugMode");
        status = ZwQueryValueKey(driverRegKey,&valueName,
            KeyValuePartialInformation,
            buffer,sizeof(buffer),
            &resultLength);
        if( NT_SUCCESS(status) )
        {
            debug_mode = *((PULONG) &(((PKEY_VALUE_PARTIAL_INFORMATION)buffer)->Data));
        }
        ZwClose(driverRegKey);
    }

    if( debug_mode )
    {
        NL_KLOG_SetLevel(&nlseKLog,NL_KLOG_LEVEL_DEBUG);
        nlseLoggingFlags = NL_KLOG_LEVEL_DEBUG;
    }
    else
    {
        NL_KLOG_SetLevel(&nlseKLog,NL_KLOG_LEVEL_ERR);
        nlseLoggingFlags = NL_KLOG_LEVEL_ERR;
    }
}/* CheckAndSetDebugMode */

NTSTATUS NLSEClientConnect(__in PFLT_PORT ClientPort,
                           __in PVOID ServerPortCookie,
                           __in PVOID ConnectionContext,
                           __in ULONG SizeOfContext,
                           __out PVOID *ConnectionCookie )
{
    PNLSE_PORT_CONTEXT portCtx=(PNLSE_PORT_CONTEXT)ConnectionContext;
#ifdef _WIN64
    NLSE_PORT_CONTEXT portCtxBuf;
#endif /* _WIN64 */
    int                i;

    CheckAndSetDebugMode();

    //Sanity checking
    if(portCtx == NULL)
    {
        NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_ERR, 
            ("NLSE!NLSEClientConnect: connection context is NULL\n")); 
        return STATUS_INVALID_PARAMETER;
    }

#ifdef _WIN64
    if (IoIs32bitProcess(NULL))
    {
        PNLSE_PORT_CONTEXT32 portCtx32;

        if (SizeOfContext != sizeof(NLSE_PORT_CONTEXT32))
        {
            NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_ERR, ("NLSE!NLSEClientConnect: connection context size is wrong\n")); 
            return STATUS_INVALID_PARAMETER;
        }

        portCtx = &portCtxBuf;
        ThunkPortCtx(ConnectionContext, portCtx);
    } else {
#endif /* _WIN64 */
        if (SizeOfContext != sizeof(NLSE_PORT_CONTEXT))
        {
            NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_ERR, 
                ("NLSE!NLSEClientConnect: connection context size is wrong\n")); 
            return STATUS_INVALID_PARAMETER;
        }
#ifdef _WIN64
    }
#endif /* _WIN64 */

    if(portCtx->portTag != NLSE_PORT_TAG_MAIN_REQUEST &&
        portCtx->portTag != NLSE_PORT_TAG_MAIN_CMD &&
        portCtx->portTag != NLSE_PORT_TAG_DRM)
    {
            NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_ERR, ("NLSE!NLSEClientConnect: wrong port tag in context\n")); 
            return STATUS_INVALID_PARAMETER;
    }

    //assign port
    if(portCtx->portTag == NLSE_PORT_TAG_MAIN_REQUEST)
    {
        if(nlfseGlobal.clientPorts[NLSE_PORT_MAIN_REQUEST_INDEX].port != NULL)
        {
            NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_ERR, ("NLSE!ClientConnect: main request port has been connected\n"));
            return STATUS_INVALID_PARAMETER;
        }
        *ConnectionCookie=&(nlfseGlobal.clientPorts[NLSE_PORT_MAIN_REQUEST_INDEX]);
        nlfseGlobal.clientPorts[NLSE_PORT_MAIN_REQUEST_INDEX].port=ClientPort;
        nlfseGlobal.clientPorts[NLSE_PORT_MAIN_REQUEST_INDEX].portTag=NLSE_PORT_TAG_MAIN_REQUEST;
    }
    else if(portCtx->portTag == NLSE_PORT_TAG_MAIN_CMD)
    {
        if(nlfseGlobal.clientPorts[NLSE_PORT_MAIN_CMD_INDEX].port != NULL)
        {
            NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_ERR, ("NLSE!ClientConnect: main cmd port has been connected\n"));
            return STATUS_INVALID_PARAMETER;
        }
        *ConnectionCookie=&(nlfseGlobal.clientPorts[NLSE_PORT_MAIN_CMD_INDEX]);
        nlfseGlobal.clientPorts[NLSE_PORT_MAIN_CMD_INDEX].port=ClientPort;
        nlfseGlobal.clientPorts[NLSE_PORT_MAIN_CMD_INDEX].portTag=NLSE_PORT_TAG_MAIN_CMD;
    }
    else
    {
        for(i=NLSE_PORT_MAIN_CMD_INDEX+1; i<NLSE_MAX_PORT_CONNECTION; i++)
        {
            if(nlfseGlobal.clientPorts[i].port == NULL)
            {
                //find an unassigned port and assign it to the new connection
                nlfseGlobal.clientPorts[i].port = ClientPort;
                nlfseGlobal.clientPorts[i].portTag=NLSE_PORT_TAG_DRM;
                *ConnectionCookie=&(nlfseGlobal.clientPorts[i]);
                return STATUS_SUCCESS;
            }
        }

        // Too bad, we cannot find a valid connection
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    return STATUS_SUCCESS;
}/* ClientConnect */

VOID NLSEClientDisconnect( __in PVOID ConnectionCookie )
{
    PNLSE_PORT_CONTEXT portCtx=(PNLSE_PORT_CONTEXT)ConnectionCookie;
    int                i;

    // Sanity check
    if(NULL == portCtx)
        return;

    for(i=0; i<NLSE_MAX_PORT_CONNECTION; i++)
    {
        if(&nlfseGlobal.clientPorts[i] == portCtx)
        {
            //find the corresponding port context and free it
            FltCloseClientPort(nlfseGlobal.filterHandle, (PFLT_PORT *) &nlfseGlobal.clientPorts[i].port);
            break;
        }
    }
}/* ClientDisconnect */
