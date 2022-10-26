// All sources, binaries and HTML pages (C) copyright 2004 by Blue Jungle Inc.,

// Redwood City CA,

// Ownership remains with Blue Jungle Inc, All rights reserved worldwide.
#include "dstypes.h"
#include "dskerneltypes.h"
#include "dsifs.h"
#include "dsutils.h"
#include "dstdicifs.h"
#include "dskerneldebug.h"
#include "actions.h"

ULONG ntohl (ULONG netlong)
{
	ULONG result = 0;
	((char *)&result)[0] = ((char *)&netlong)[3];
	((char *)&result)[1] = ((char *)&netlong)[2];
	((char *)&result)[2] = ((char *)&netlong)[1];
	((char *)&result)[3] = ((char *)&netlong)[0];
	return result;
}

USHORT ntohs (USHORT netshort)
{
	USHORT result = 0;
	((char *)&result)[0] = ((char *)&netshort)[1];
	((char *)&result)[1] = ((char *)&netshort)[0];
	return result;
}

BOOL DSK_szTerminateString(char *szBuffer, ULONG ulBufLength, ULONG ulStringLength)
{
	if (szBuffer == NULL || ulBufLength < 2 || ulStringLength == 0 || 
		ulBufLength <= ulStringLength || ulStringLength > MAX_NAME_LENGTH )
	{
		DP(DSDERR,("DSK_szTerminateString<ERROR>Invalid parameters\n"));
		return FALSE;
	}
	szBuffer[ulStringLength] = 0x00;
	return TRUE;
}

BOOL DSK_wzTerminateString(WCHAR *wzBuffer, ULONG ulBufLength, ULONG ulStringLength)
{
	if (wzBuffer == NULL || ulBufLength < 2 || ulStringLength == 0 || 
		ulBufLength <= ulStringLength || ulStringLength > MAX_NAME_LENGTH)
	{
		DP(DSDERR,("DSK_wzTerminateString<ERROR>Invalid parameters\n"));
		return FALSE;
	}
	wzBuffer[ulStringLength] = 0x0000;
	return TRUE;
}

BOOL DSK_szCopyStringWithTerminate(char *szDst, ULONG ulDstSize, char *szSrc, ULONG ulSrcSize )
{
	if (szDst == NULL || ulDstSize < 2 || szSrc == NULL || ulSrcSize == 0 || 
		ulSrcSize >= ulDstSize || ulSrcSize > MAX_NAME_LENGTH)
	{
		DP(DSDERR,("DSK_szCopyStringWithTerminate<ERROR>Invalid parameters\n"));
		return FALSE;
	}
	memset((void *)szDst, 0, ulDstSize);
	memcpy(szDst, szSrc, ulSrcSize);
	szDst[ulSrcSize] = 0;
	return TRUE;
}

BOOL DSK_wzCopyStringWithTerminate(WCHAR *wzDst, ULONG ulDstSize, wchar_t *wzSrc, ULONG ulSrcSize)
{
	if (wzDst == NULL || ulDstSize < 2 || wzSrc == NULL || ulSrcSize == 0 || 
		ulSrcSize >= ulDstSize || ulSrcSize > MAX_NAME_LENGTH)
	{
		DP(DSDERR,("DSK_wzCopyStringWithTerminate<ERROR>Invalid parameters\n"));
		return FALSE;
	}
	memset((void *)wzDst, 0, ulDstSize * 2);
	memcpy((void *)wzDst, (void *)wzSrc, ulSrcSize * 2);
	wzDst[ulSrcSize] = 0;
	return TRUE;
}

BOOL GetStringSize(IN char *pszString, IN ULONG ulMaxSearch, OUT PULONG pulStringSize)
{
	ULONG ulStringSize = 0;
	while(TRUE)
	{
		if (pszString[ulStringSize] == 0)
			break;
		ulStringSize++;
		if (ulStringSize >= ulMaxSearch)
		{
			DP(DSDERR,("GetStringSize<ERR> get pass max string search\n"));
			return FALSE;
		}
	}
	*pulStringSize = ulStringSize;
	return TRUE;
}


BOOL GetWStringSize(IN WCHAR *pwzString, IN ULONG ulMaxSearch, OUT PULONG pulStringSize)
{
	ULONG ulStringSize = 0;
	while(TRUE)
	{
		if (pwzString[ulStringSize] == 0)
			break;
		ulStringSize++;
		if (ulStringSize >= ulMaxSearch)
		{
			DP(DSDERR,("GetWStringSize<ERR> get pass max string search\n"));
			return FALSE;
		}
	}
	*pulStringSize = ulStringSize;
	return TRUE;
}


BOOL DSK_InitSpinLock (PBJ_SPIN_LOCK pLock, ULONG ulLockType)
{
	if (pLock == NULL)
	{
		DP(DSDERR,("DSK_InitSpinLock::pLock is NULL\n"));
		return FALSE;
	}
	if (ulLockType > DS_DISPATCH_LEVEL_LOCK)
	{
		DP(DSDERR,("DSK_InitSpinLock::Invalid ulLockType = %d\n", ulLockType));
		return FALSE;
	}

	pLock->ulLockLevel = ulLockType;
#ifdef WIN32
	switch (pLock->ulLockLevel)
	{
	case DS_DISPATCH_LEVEL_LOCK:
		KeInitializeSpinLock(&pLock->kLock);
		break;
	case DS_PASSIVE_LEVEL_LOCK:
	case DS_APC_LEVEL_LOCK:
		ExInitializeFastMutex(&pLock->fFastMutex);
		break;
	default:
		break;
	}
#elif LINUX
	pLock->kLock = SPIN_LOCK_UNLOCKED;
#endif	//ifdef WIN32...elif LINUX...endif
	return TRUE;
}

BOOL DSK_SpinLock(BJ_SPIN_LOCK *pLock)
{
	if (pLock == NULL)
	{
		DP(DSDERR,("DSK_SpinLock::pLock is NULL\n"));
		return FALSE;
	}
#ifdef WIN32
	switch(pLock->ulLockLevel)
	{
	case DS_DISPATCH_LEVEL_LOCK:
		KeAcquireSpinLock(&pLock->kLock, &pLock->kIRQSave);	
		break;
	case DS_PASSIVE_LEVEL_LOCK:
	case DS_APC_LEVEL_LOCK:
		ExAcquireFastMutex(&pLock->fFastMutex);
		break;
	default:
		break;
	}
#elif LINUX
	switch(pLock->ulLockLevel)
	{
	case DS_DISPATCH_LEVEL_LOCK:
		spin_lock_irqsave(&pLock->kLock, pLock->lock_flags);
		break;
	case DS_PASSIVE_LEVEL_LOCK:
	case DS_APC_LEVEL_LOCK:
		spin_lock(&pLock->kLock);
		break;
	default:
		break;
	}

#endif	//ifdef WIN32...elif LINUX...endif
	return TRUE;
}

BOOL DSK_SpinUnlock(BJ_SPIN_LOCK *pLock)
{
	if (pLock == NULL)
	{
		DP(DSDERR,("DSK_SpinUnlock::pLock or pIRQSave is NULL\n"));
		return FALSE;
	}
#ifdef WIN32
	switch(pLock->ulLockLevel)
	{
	case DS_DISPATCH_LEVEL_LOCK:
		KeReleaseSpinLock(&pLock->kLock, pLock->kIRQSave);	
		break;
	case DS_PASSIVE_LEVEL_LOCK:
	case DS_APC_LEVEL_LOCK:
		ExReleaseFastMutex(&pLock->fFastMutex);
		break;
	default:
		break;
	}
#elif LINUX
	switch(pLock->ulLockLevel)
	{
	case DS_DISPATCH_LEVEL_LOCK:
		spin_unlock_irqrestore(&pLock->kLock, pLock->lock_flags);
		break;
	case DS_PASSIVE_LEVEL_LOCK:
	case DS_APC_LEVEL_LOCK:
		spin_unlock(&pLock->kLock);
		break;
	default:
		break;
	}
#endif	//ifdef WIN32...elif LINUX...endif
	return TRUE;
}


PVOID
DSK_GetListObjectHead(PDS_LIST_ENTRY pQList, PBJ_SPIN_LOCK pLock, ULONG ulObjectType)
{
	PDS_LIST_ENTRY	pListEntry;
	PVOID pObject = NULL;
	
	if (pQList == NULL)
	{
		DP(DSDERR,("DSK_GetSharedObjectHead<ERR>Invalid parameter received\n"));
		return NULL;
	}
	if (pLock) DSK_SpinLock(pLock);
	if (IS_DS_LIST_EMPTY(pQList))
	{
		DP(DSDINFO,("DSK_GetSharedObjectHead<INFO>pQList empty\n"));
		if (pLock) DSK_SpinUnlock(pLock);
		return NULL;
	}
	pListEntry = DS_GET_LIST_HEAD(pQList);

	switch (ulObjectType)
	{
		case DS_IFS_TDI_SHARED_OBJECT:
			pObject = DS_GET_LIST_ITEM_PTR(pListEntry, IFS_TDI_SHARED_OBJECT, dsObjectEntry);
			break;
		case DS_IFS_IPC_OBJECT:
			pObject = DS_GET_LIST_ITEM_PTR(pListEntry, IPC_SLOT, dsObjectEntry);
			break;
		case DS_IFS_FILECTX_OBJECT:			
			pObject = DS_GET_LIST_ITEM_PTR(pListEntry, FILE_OBJECT_CONTEXT, dsObjectEntry);
			break;
		case DS_IFS_COMPLETIONCTX_OBJECT:			
                        pObject = DS_GET_LIST_ITEM_PTR(pListEntry, COMPLETION_CONTEXT, dsObjectEntry);
			break;
		default:
			pObject = NULL;
			break;
	}

	if (pLock) DSK_SpinUnlock(pLock);

	return pObject;
}

PVOID
DSK_GetListObjectTail(PDS_LIST_ENTRY pQList, PBJ_SPIN_LOCK pLock, ULONG ulObjectType)
{
	PDS_LIST_ENTRY	pListEntry;
	PVOID pObject = NULL;
	
	if (pQList == NULL)
	{
		DP(DSDERR,("DSK_GetSharedObjectTail<ERR>Invalid parameter received\n"));
		return NULL;
	}
	if (pLock) DSK_SpinLock(pLock);
	if (IS_DS_LIST_EMPTY(pQList))
	{
		DP(DSDINFO,("DSK_GetSharedObjectTail<INFO>pQList empty\n"));
		if (pLock) DSK_SpinUnlock(pLock);
		return NULL;
	}

	pListEntry = DS_GET_LIST_TAIL(pQList);

	switch (ulObjectType)
	{
		case DS_IFS_TDI_SHARED_OBJECT:
			pObject = DS_GET_LIST_ITEM_PTR(pListEntry, IFS_TDI_SHARED_OBJECT, dsObjectEntry);
			break;
		case DS_IFS_IPC_OBJECT:
			pObject = DS_GET_LIST_ITEM_PTR(pListEntry, IPC_SLOT, dsObjectEntry);
			break;
		case DS_IFS_FILECTX_OBJECT:			
			pObject = DS_GET_LIST_ITEM_PTR(pListEntry, FILE_OBJECT_CONTEXT, dsObjectEntry);
			break;
		case DS_IFS_COMPLETIONCTX_OBJECT:			
                        pObject = DS_GET_LIST_ITEM_PTR(pListEntry, COMPLETION_CONTEXT, dsObjectEntry);
			break;
		default:
			pObject = NULL;
			break;
	}

	if (pLock) DSK_SpinUnlock(pLock);

	return pObject;
}

PVOID
DSK_GetNextListObjectHead(PDS_LIST_ENTRY pQList, PBJ_SPIN_LOCK pLock, 
						  int iNumOfNexts, BOOL fRemoveSharedObject, ULONG ulObjectType)
{
	int i;
	PVOID	pObject;
	PDS_LIST_ENTRY			pListEntry;

	if (pQList == NULL || iNumOfNexts <= 0 || iNumOfNexts >= INITIAL_SHARED_NUM_OBJECT)
	{
		DP(DSDERR,("DSK_GetNextShareObjectHead<ERR>Invalid parameter received\n"));
		return NULL;
	}
	if (pLock) DSK_SpinLock(pLock);
	if (IS_DS_LIST_EMPTY(pQList))
	{
		DP(DSDERR,("SK_GetNextShareObjectHead<ERR>pQList empty\n"));
		if (pLock) DSK_SpinUnlock(pLock);
		return NULL;
	}

	pListEntry = DS_GET_LIST_HEAD(pQList);
	//Don't need to spinlock here, if we spinlock, we'll spinlock earlier in this function.
	if (pListEntry == NULL)
	{
		DP(DSDERR,("DSK_GetNextShareObjectHead<ERR>pListEntry head is NULL\n"));
		if (pLock) DSK_SpinUnlock(pLock);
		return NULL;
	}
	
	for (i = 1; i < iNumOfNexts; i++)
	{
		if (pListEntry == NULL)
		{
			DP(DSDERR,("DSK_GetNextShareObjectHead<ERR>Looking for pListEntry is NULL\n"));
			if (pLock) DSK_SpinUnlock(pLock);
			return NULL;
		}

		if (pListEntry == NULL)
		{
			DP(DSDERR,("Unexpected NULL pListEntry 1\n"));
			return NULL;
		}

		pListEntry = pListEntry->pFLink;	
	}

	if (pListEntry == NULL)
	{
		DP(DSDERR,("Unexpected NULL pListEntry 2\n"));
		return NULL;
	}

	if (fRemoveSharedObject)
	{
		DS_REMOVE_LIST_ENTRY(pListEntry);
	}

	switch (ulObjectType)
	{
		case DS_IFS_TDI_SHARED_OBJECT:
			pObject = DS_GET_LIST_ITEM_PTR(pListEntry, IFS_TDI_SHARED_OBJECT, dsObjectEntry);
			break;
		case DS_IFS_IPC_OBJECT:
			pObject = DS_GET_LIST_ITEM_PTR(pListEntry, IPC_SLOT, dsObjectEntry);
			break;
		case DS_IFS_FILECTX_OBJECT:			
			pObject = DS_GET_LIST_ITEM_PTR(pListEntry, FILE_OBJECT_CONTEXT, dsObjectEntry);
			break;
		case DS_IFS_COMPLETIONCTX_OBJECT:			
                        pObject = DS_GET_LIST_ITEM_PTR(pListEntry, COMPLETION_CONTEXT, dsObjectEntry);
			break;
		default:
			pObject = NULL;
			break;
	}

	return pObject;
}


PVOID
DSK_RemoveListObjectHead(PDS_LIST_ENTRY pQList, PBJ_SPIN_LOCK pLock, PULONG pulObjectInQ, ULONG ulObjectType)
{
	PDS_LIST_ENTRY	pListEntry;
	PVOID pObject = NULL;
		
	if (pQList == NULL)
	{
		DP(DSDERR,("DSK_RemoveSharedObjectHead<ERR>Invalid parameter received\n"));
		return NULL;
	}

	if (pLock) DSK_SpinLock(pLock);
	if (IS_DS_LIST_EMPTY(pQList))
	{
		DP(DSDERR,("DSK_RemoveSharedObjectHead<ERR>pQList empty\n"));
		if (pLock) DSK_SpinUnlock(pLock);
		return NULL;
	}
	pListEntry = DS_REMOVE_HEAD_LIST(pQList);
        
	switch (ulObjectType)
	{
		case DS_IFS_TDI_SHARED_OBJECT:
			pObject = DS_GET_LIST_ITEM_PTR(pListEntry, IFS_TDI_SHARED_OBJECT, dsObjectEntry);
			break;
		case DS_IFS_IPC_OBJECT:
			pObject = DS_GET_LIST_ITEM_PTR(pListEntry, IPC_SLOT, dsObjectEntry);
			break;
		case DS_IFS_FILECTX_OBJECT:			
			pObject = DS_GET_LIST_ITEM_PTR(pListEntry, FILE_OBJECT_CONTEXT, dsObjectEntry);
			break;
		case DS_IFS_COMPLETIONCTX_OBJECT:			
                        pObject = DS_GET_LIST_ITEM_PTR(pListEntry, COMPLETION_CONTEXT, dsObjectEntry);
			break;
		default:
			pObject = NULL;
			break;
	}

	if (pulObjectInQ) ((ULONG)*pulObjectInQ)--;
	if (pLock) DSK_SpinUnlock(pLock);

	return pObject;
}

PVOID
DSK_RemoveListObjectTail(PDS_LIST_ENTRY pQList, PBJ_SPIN_LOCK pLock, PULONG pulObjectInQ, ULONG ulObjectType)
{
	PDS_LIST_ENTRY	pListEntry;
	PVOID pObject = NULL;
	
	if (pQList == NULL)
	{
		DP(DSDERR,("DSK_RemoveSharedObjectTail<ERR>Invalid parameter received\n"));
		return NULL;
	}

	if (pLock) DSK_SpinLock(pLock);
	if (IS_DS_LIST_EMPTY(pQList))
	{
		DP(DSDERR,("DSK_RemoveSharedObjectTail<ERR>pQList empty\n"));
		if (pLock) DSK_SpinUnlock(pLock);
		return NULL;
	}
	pListEntry = DS_REMOVE_TAIL_LIST(pQList);

	switch (ulObjectType)
	{
		case DS_IFS_TDI_SHARED_OBJECT:
			pObject = DS_GET_LIST_ITEM_PTR(pListEntry, IFS_TDI_SHARED_OBJECT, dsObjectEntry);
			break;
		case DS_IFS_IPC_OBJECT:
			pObject = DS_GET_LIST_ITEM_PTR(pListEntry, IPC_SLOT, dsObjectEntry);
			break;
		case DS_IFS_FILECTX_OBJECT:			
			pObject = DS_GET_LIST_ITEM_PTR(pListEntry, FILE_OBJECT_CONTEXT, dsObjectEntry);
			break;
		case DS_IFS_COMPLETIONCTX_OBJECT:			
                        pObject = DS_GET_LIST_ITEM_PTR(pListEntry, COMPLETION_CONTEXT, dsObjectEntry);
			break;
		default:
			pObject = NULL;
			break;
	}
        

	if (pulObjectInQ) ((ULONG)*pulObjectInQ) --;
	if (pLock) DSK_SpinUnlock(pLock);

	return pObject;
}

BOOL 
DSK_InsertListObjectHead(PDS_LIST_ENTRY pQList, PDS_LIST_ENTRY pdsObjectEntry,
						   PBJ_SPIN_LOCK pLock, PULONG pulObjectInQ)
{

	if (pQList == NULL || pdsObjectEntry == NULL)
	{
		DP(DSDERR,("DSK_InsertSharedObjectHead<ERR>Invalid parameter received\n"));
		return FALSE;
	}

	if (pLock) DSK_SpinLock(pLock);
        DS_INSERT_HEAD_LIST(pQList, pdsObjectEntry);
	if (pulObjectInQ) ((ULONG)*pulObjectInQ)++;
	if (pLock) DSK_SpinUnlock(pLock);

	return TRUE;
}

BOOL 
DSK_InsertListObjectTail(PDS_LIST_ENTRY pQList, PDS_LIST_ENTRY pdsObjectEntry, 
						   PBJ_SPIN_LOCK pLock, PULONG pulObjectInQ)
{
	if (pQList == NULL || pdsObjectEntry == NULL)
	{
		DP(DSDERR,("DSK_InsertSharedObjectTail<ERR>Invalid parameter received\n"));
		return FALSE;
	}

	if (pLock) DSK_SpinLock(pLock);
	DS_INSERT_TAIL_LIST(pQList, pdsObjectEntry);
	if (pulObjectInQ) ((ULONG)*pulObjectInQ)++;
	if (pLock) DSK_SpinUnlock(pLock);

	return TRUE;
}

BOOL DSK_RecoverIFSTDISharedQ(PIFS_TDI_SHAREDMEM pIFSTDISharedMem)
{
	int i, iNumTries;
	PIFS_TDI_SHARED_OBJECT pSharedObject = NULL;
	if (pIFSTDISharedMem == NULL)
	{
		DP(DSDERR,("DSK_RecoverIFSTDISharedQ<ERROR>Invalid parameter\n"));
		return FALSE;
	}	
	DSK_SpinLock(&pIFSTDISharedMem->Lock);
	for (i = 0; i < (int)pIFSTDISharedMem->ulTotalNumSharedObjects; i++)
	{
		pSharedObject =  DSK_RemoveListObjectHead(&pIFSTDISharedMem->dsBusyQ, 
							            NULL,
							            &pIFSTDISharedMem->ulObjectsInBusyQ,
							            DS_IFS_TDI_SHARED_OBJECT);
		if (pSharedObject == NULL)
			break;

	        DP(DSDNOLOG, ("Removing '%S' from the busy list head (Recovery)\n", pSharedObject->wzFileName));
		pSharedObject->ulIFSTDIState = TDI_INITIALIZED;
	        DP(DSDNOLOG, ("Inserting '%S' to the free list tail (6)\n", pSharedObject->wzFileName));
		DSK_InsertListObjectTail(&pIFSTDISharedMem->dsFreeQ,
		&pSharedObject->dsObjectEntry, NULL,
		&pIFSTDISharedMem->ulObjectsInFreeQ);
	}
	DSK_SpinUnlock(&pIFSTDISharedMem->Lock);
	return TRUE;
}



BOOL DSK_GetOSVersion(PULONG pulMajorVersion, PULONG pulMinorVersion,
					  PULONG pulBuildNumber, PVOID pContext)
{
	pContext = pContext;	//stop compiling warnings
#ifdef WIN32
	return PsGetVersion(pulMajorVersion, pulMinorVersion, 
						pulBuildNumber, (PUNICODE_STRING)pContext);
#elif LINUX
	//TBD
#endif	//#ifdef WIN32 ... #elif LINUX ... 
	return TRUE;
}


BOOL DSK_GetTDIInfo(IN PIFS_TDI_SHARED_OBJECT pTDISharedMem, OUT PFILE_OBJECT_CONTEXT pIFSFileCtx)
{
	if (pTDISharedMem == NULL || pIFSFileCtx == NULL)
	{
		DP(DSDERR,("DSK_GetTDIInfo<ERROR> NULL parameters received\n"));
		return FALSE;
	}
	pIFSFileCtx->ulLocalAddress = pTDISharedMem->ulLocalIPAddress;
	pIFSFileCtx->ulRemoteAddress = pTDISharedMem->ulRemoteIPAddress;
	pIFSFileCtx->ulTID = pTDISharedMem->ulTID;
	pIFSFileCtx->ulUID = pTDISharedMem->ulPID;
	pIFSFileCtx->ulPID = pTDISharedMem->ulPID;
	wcscpy(pIFSFileCtx->wzFileName,pTDISharedMem->wzFileName);
	pIFSFileCtx->ulFileNameLength = pTDISharedMem->ulFileNameLength;
	return TRUE;
}

BOOL LogSMBHeader(IN PVOID pData,IN ULONG ulDataSize)
{
	PSMB_HEADER pSMBHeader;	
	DP(DSDINFO,("LogSMBHeader ...\n"));

	if (ulDataSize < sizeof(SMB_HEADER))
	{
		DP(DSDERR,("LogSMBHeader<ERROR> Size for SMB header is too small\n"));
		return FALSE;
	}
	
	pSMBHeader = (PSMB_HEADER)pData;

	if (*(PULONG)pSMBHeader->Protocol != SMB_HEADER_PROTOCOL)
	{
		DP(DSDERR,("LogSMBHeader<ERROR> Not a SMB Header\n"));
		return FALSE;
	}

	DP(DSDNOLOG,("SMB->Header = %s\n",pSMBHeader->Protocol));
	DP(DSDINFO,("SMB->Command = 0x%x\n",pSMBHeader->Command));
	DP(DSDINFO,("SMB->ErrorClass = 0x%x\n",pSMBHeader->ErrorClass));
	DP(DSDINFO,("SMB->Flags = 0x%x\n",pSMBHeader->Flags));
	DP(DSDINFO,("SMB->Pid = 0x%x = %d\n",pSMBHeader->Pid, pSMBHeader->Pid));

	switch (pSMBHeader->Command)
	{
		case SMB_COM_CREATE_DIRECTORY:
			DP(DSDINFO,("SMB_COM_CREATE_DIRECTORY\n"));
			break;
		case SMB_COM_DELETE_DIRECTORY:
			DP(DSDINFO,("SMB_COM_DELETE_DIRECTORY\n"));
			break;
		case SMB_COM_OPEN:
			DP(DSDINFO,("SMB_COM_OPEN\n"));
			break;
		case SMB_COM_CREATE:		
			DP(DSDINFO,("SMB_COM_CREATE\n"));
			break;
		case SMB_COM_NT_CREATE_ANDX:
		{
			char *pString;
			pString = (char *)((char *)pData + sizeof(SMB_HEADER) + sizeof (REQ_NT_CREATE_ANDX));
			
			//DP(DSDINFO,("SBMCreate.Command = 0x%x  FileName=%S\n",pSMBHeader->Command, pString));
			DP(DSDINFO,("SBMCreate.Command = 0x%x\n",pSMBHeader->Command));
			DP(DSDINFO,("SBMCreate.ErrorClass = 0x%x\n",pSMBHeader->ErrorClass));
			DP(DSDINFO,("SBMCreate.Reserved = 0x%x\n",pSMBHeader->Reserved));
			DP(DSDINFO,("SBMCreate.Error = 0x%x\n",pSMBHeader->Error));
			DP(DSDINFO,("SBMCreate.Flags = 0x%x\n",pSMBHeader->Flags));
			DP(DSDINFO,("SBMCreate.Flags2 = 0x%x\n",pSMBHeader->Flags2));
			DP(DSDINFO,("SBMCreate.Reserved2 = 0x%x\n",pSMBHeader->Reserved2));
			DP(DSDINFO,("SMBCreate.Key = 0x%x\n",pSMBHeader->Key));
			DP(DSDINFO,("SBMCreate.PidHigh = 0x%x\n",pSMBHeader->PidHigh));
			DP(DSDINFO,("SBMCreate.Tid = 0x%x\n",pSMBHeader->Tid));
			DP(DSDINFO,("SBMCreate.Pid = 0x%x\n",pSMBHeader->Pid));
			DP(DSDINFO,("SBMCreate.Uid = 0x%x\n",pSMBHeader->Uid));
			DP(DSDINFO,("SBMCreate.Mid = 0x%x\n",pSMBHeader->Mid));
		}
			break;
		case SMB_COM_CLOSE:
		{
			PUSHORT pusFid;
			pusFid = (PUSHORT)((PUCHAR)pData + sizeof(SMB_HEADER) + 1 /*Offset or something*/);
			DP(DSDINFO,("SMB_COM_CLOSE: Fid=0x%x\n",*pusFid));
		}
			break;
		case SMB_COM_FLUSH:
			DP(DSDINFO,("SMB_COM_FLUSH\n"));
			break;
		case SMB_COM_DELETE:
			DP(DSDINFO,("SMB_COM_DELETE\n"));
			break;
		case SMB_COM_RENAME:
			DP(DSDINFO,("SMB_COM_RENAME\n"));
			break;
		case SMB_COM_WRITE_ANDX:
		{
			REQ_NT_WRITE_ANDX *pWritenX;
			pWritenX = (REQ_NT_WRITE_ANDX *)((PUCHAR)pData + sizeof(SMB_HEADER));
			DP(DSDINFO,("pWritenX->WordCount = 0x%x\n",pWritenX->WordCount));
			DP(DSDINFO,("pWritenX->AndXCommand = 0x%x\n",pWritenX->AndXCommand));
			DP(DSDINFO,("pWritenX->AndXOffset = 0x%x\n",pWritenX->AndXOffset));
			DP(DSDINFO,("pWritenX->Fid = 0x%x\n",pWritenX->Fid));
			DP(DSDINFO,("pWritenX->Offset = 0x%x\n",pWritenX->Offset));
			DP(DSDINFO,("pWritenX->WriteMode = 0x%x\n",pWritenX->WriteMode));
			DP(DSDINFO,("pWritenX->Remaining = 0x%x\n",pWritenX->Remaining));
			DP(DSDINFO,("pWritenX->DataLengthHigh = 0x%x\n",pWritenX->DataLengthHigh));
			DP(DSDINFO,("pWritenX->DataLength = 0x%x\n",pWritenX->DataLength));
			DP(DSDINFO,("pWritenX->DataOffset = 0x%x\n",pWritenX->DataOffset));
			DP(DSDINFO,("pWritenX->OffsetHigh = 0x%x\n",pWritenX->OffsetHigh));
			DP(DSDINFO,("pWritenX->ByteCount = 0x%x\n",pWritenX->ByteCount));
		}
			break;
		case SMB_COM_READ_ANDX:
		{
			REQ_READ_ANDX *pReadnX;
			pReadnX = (REQ_READ_ANDX *)((PUCHAR)pData + sizeof(SMB_HEADER));
			DP(DSDINFO,("pReadnX->WordCount = 0x%x\n",pReadnX->WordCount));
			DP(DSDINFO,("pReadnX->AndXCommand = 0x%x\n",pReadnX->AndXCommand));
			DP(DSDINFO,("pReadnX->AndXReserved = 0x%x\n",pReadnX->AndXReserved));
			DP(DSDINFO,("pReadnX->Fid = 0x%x\n",pReadnX->Fid));
			DP(DSDINFO,("pReadnX->Offset = 0x%x\n",pReadnX->Offset));
			DP(DSDINFO,("pReadnX->MaxCount = 0x%x\n",pReadnX->MaxCount));
			DP(DSDINFO,("pReadnX->MinCount = 0x%x\n",pReadnX->MinCount));
			DP(DSDINFO,("pReadnX->Timeout = 0x%x\n",pReadnX->Timeout));
			DP(DSDINFO,("pReadnX->Remaining = 0x%x\n",pReadnX->Remaining));
			DP(DSDINFO,("pReadnX->ByteCount = 0x%x\n",pReadnX->ByteCount));
		}
			break;
		default:
			break;
	}
	return TRUE;
}
BOOL FillSharedIFSTDIMem(PIFS_TDI_SHAREDMEM pIFSTDISharedMem,
						 ULONG ulRemoteAddr, ULONG ulLocalAddr,
						 PSMB_HEADER pSMBHeader, WCHAR *wzFileName, ULONG ulFileNameLength,
                                                 ULONG ulAction)
{
	PIFS_TDI_SHARED_OBJECT	pIFSTDISharedObject = NULL;

	if (pIFSTDISharedMem == NULL || pSMBHeader == NULL || wzFileName == NULL)
	{
		DP(DSDERR,("FillSharedIFSTDIMem NULL parameters received\n"));
		return FALSE;
	}

	if (ulFileNameLength <= 1)
	{
		DP(DSDNOLOG,("FileName length too short, let it pass\n"));
		return FALSE;
	}

	if (ulFileNameLength >= MAX_NAME_LENGTH)
	{
		DP(DSDWRN,("FileNameLength too large = %d, truncate to %d instead SMB->Command=0x%x\n", \
				   ulFileNameLength, MAX_NAME_LENGTH - 1, pSMBHeader->Command));
		ulFileNameLength = MAX_NAME_LENGTH - 1;		
	}

	//Secondary screening.  Eliminate services and file that not going to IFS driver.
	//This may be too slow and CPU consume.  Have to run VNTune to see what going on here
	if (wcsstr(wzFileName,L"Thumbs.db") ||
		wcsstr(wzFileName,L"\\wkssvc") ||
		wcsstr(wzFileName,L"\\srvsvc") ||
		wcsstr(wzFileName,L"\\winreg") ||
		wcsstr(wzFileName,L"\\spoolss") ||
		wcsstr(wzFileName,L"\\dcom") ||
		wcsstr(wzFileName,L"\\drsuapi") ||
		wcsstr(wzFileName,L"\\echo") ||
		wcsstr(wzFileName,L"\\epmapper") ||
		wcsstr(wzFileName,L"\\netlogon") ||
		wcsstr(wzFileName,L"\\samr") ||
		wcsstr(wzFileName,L"\\lsarpc"))	
	{
		DP(DSDNOLOG,("Not a real file, we should not check these\n"));
		return FALSE;
	}

	pIFSTDISharedObject = DSK_RemoveListObjectHead(&pIFSTDISharedMem->dsFreeQ, 
												   &pIFSTDISharedMem->Lock,
												   &pIFSTDISharedMem->ulObjectsInFreeQ,
												   DS_IFS_TDI_SHARED_OBJECT);

	//We have some of options here.
	//1.  Currently we do the easiest option: recover the Q and allow the a crack in the defense for a short time.
	//2.  An algorithm that figure out either we can continue to either expand or do option 1
	//3.  Exam to see if we if there is an err in the system and take appropriate work of option 1 and option 2
	if (pIFSTDISharedObject == NULL)
	{
                DP(DSDERR,("DSK_RecoverIFSTDISharedQ<ERROR>Something wrong, we have to recover the entire share Q's\n"));
		if (DSK_RecoverIFSTDISharedQ(pIFSTDISharedMem) == FALSE)
		{
			DP(DSDERR,("FillSharedIFSTDIMem<ERROR>Ran out of free Q and still not able to recover\n"));
			return FALSE;
		}

		pIFSTDISharedObject = DSK_RemoveListObjectHead(&pIFSTDISharedMem->dsFreeQ, 
													   &pIFSTDISharedMem->Lock,
													   &pIFSTDISharedMem->ulObjectsInFreeQ,
													   DS_IFS_TDI_SHARED_OBJECT);
		if (!pIFSTDISharedObject)
		{
			DP(DSDERR,("FillSharedIFSTDIMem<ERROR>Able to recover, but still cannot get buffer in FreeQ\n"));
			return FALSE;
		}
	}

	if (pIFSTDISharedObject->ulIFSTDIState == TDI_PUT)
	{
		DP(DSDERR,("...Unexpected IFSTDI state = TDI_PUT received...Remote Address = 0x%x\n", ulRemoteAddr));
	}

	DSK_wzCopyStringWithTerminate(pIFSTDISharedObject->wzFileName, MAX_NAME_LENGTH, 
								  wzFileName, ulFileNameLength);

	pIFSTDISharedObject->ulFileNameLength = ulFileNameLength;
	pIFSTDISharedObject->ulIFSTDIState = TDI_PUT;
	pIFSTDISharedObject->ulLocalIPAddress = ulLocalAddr;
	pIFSTDISharedObject->ulRemoteIPAddress = ulRemoteAddr;
        pIFSTDISharedObject->ulAction = ulAction;
	pIFSTDISharedObject->ulPID = pSMBHeader->Pid;
	pIFSTDISharedObject->ulTID = pSMBHeader->Tid;
	pIFSTDISharedObject->ulUID = pSMBHeader->Uid;
        pIFSTDISharedObject->ulUserSIDLength = 0;
        pIFSTDISharedObject->ulReserve = 0;

        DP(DSDNOLOG,("     FillSharedIFSTDIMem::File = %S ",pIFSTDISharedObject->wzFileName));
        DP(DSDNOLOG,("TDI Inserting '%S' to the busy list tail (8)\n", pIFSTDISharedObject->wzFileName));
	DSK_InsertListObjectTail(&pIFSTDISharedMem->dsBusyQ, &pIFSTDISharedObject->dsObjectEntry, 
						     &pIFSTDISharedMem->Lock, &pIFSTDISharedMem->ulObjectsInBusyQ);
	return TRUE;
}

BOOL ScreenSMBHeaderAndFillData(IN PVOID pData,IN ULONG ulDataSize,
								IN ULONG ulRemoteAddr, IN ULONG ulLocalAddr,
								IN OUT PIFS_TDI_SHAREDMEM pIFSTDISharedMem)
{
	//Return FALSE mean let this operation pass without putting info for IFS driver to get.
	//Return TRUE mean get infomation from this buffer and pass it to IFS driver
	BOOL		bReturn = FALSE;	//default, let it pass

	PSMB_HEADER	pSMBHeader = (PSMB_HEADER)pData;
	
	//Bad SMB header, let it pass
	if (ulDataSize < sizeof(SMB_HEADER) || pSMBHeader == NULL)
		goto ScreenSMBHeaderExit;

	if (pIFSTDISharedMem == NULL)
	{
		DP(DSDERR,("NULL pIFSTDISharedMem received...\n"));
		goto ScreenSMBHeaderExit;
	}

	//If can't determine the remote address, let it pass
	if (ulRemoteAddr == 0)
		goto ScreenSMBHeaderExit; 

	//Screening for commands that we need to check, the rest
	//of other commands let them pass through...
	if (pSMBHeader->Command != SMB_COM_NT_CREATE_ANDX &&
		pSMBHeader->Command != SMB_COM_CREATE &&
		pSMBHeader->Command != SMB_COM_CREATE_DIRECTORY &&
		pSMBHeader->Command != SMB_COM_DELETE_DIRECTORY &&
		pSMBHeader->Command != SMB_COM_OPEN &&
		pSMBHeader->Command != SMB_COM_OPEN_ANDX &&
		pSMBHeader->Command != SMB_COM_OPEN_PRINT_FILE &&
		pSMBHeader->Command != SMB_COM_RENAME &&
		pSMBHeader->Command != SMB_COM_DELETE &&
#ifdef IPFLT
		pSMBHeader->Command != SMB_COM_TREE_DISCONNECT &&
		pSMBHeader->Command != SMB_COM_TREE_CONNECT_ANDX &&
#endif //#ifdef IPFLT
		pSMBHeader->Command != SMB_COM_TRANSACTION2)
		goto ScreenSMBHeaderExit;

	//Now for each indivdual command, we'll have to screen and exam the fields 
	//of each different structure to determine if this operation will be pass 
	//to the IFS filter or not.  If the operation pass to the IFS, there wont
	//be a DSK_SpinUnlock in the TDI, but the IFS filter driver will responsible
	//for the DSK_SpinUnlock ...
	switch(pSMBHeader->Command)
	{
		case SMB_COM_NT_CREATE_ANDX:
		{
			REQ_NT_CREATE_ANDX *pSMBCreateX;
			WCHAR *wzFileName;
			ULONG ulFileNameLength;

			pSMBCreateX = (REQ_NT_CREATE_ANDX *)((char *)pData + sizeof(SMB_HEADER));
			wzFileName = (PWCHAR)((char *)pData + sizeof(SMB_HEADER) + sizeof(REQ_NT_CREATE_ANDX));
			ulFileNameLength = pSMBCreateX->NameLength >> 1;

			//Cannot use Normal file case here because on copy, the create 
			//open for the file open without the normal file flag set
			//if (pSMBCreateX->FileAttributes & 0x80) //Normal file case

			//0x00400000 is undoccument.  Look like it something only happen in Windows.
			//A normal file won't have this bit set.  Most(not all)system services will
			//have this bit set.  Secondary screening will be in FillSharedIFSTDIMem function
			//if ((pSMBCreateX->CreateOptions & 0x00400000) == 0)
			{
				if (FillSharedIFSTDIMem(pIFSTDISharedMem, ulRemoteAddr, ulLocalAddr,
										pSMBHeader, wzFileName, ulFileNameLength, 0) == FALSE)
				{
					DP(DSDNOLOG,("TDI cannot fill sharedmem, let this pass FileName = %S\n", wzFileName));
					bReturn = FALSE;
					goto ScreenSMBHeaderExit;
				}
				else	
				{   
					bReturn = TRUE;					
					DP(DSDDEV,("TDI:CREATE IP=0x%x(%d)\n", ulRemoteAddr,(ulRemoteAddr & 0xff)));	
					goto ScreenSMBHeaderExit;
				}
				
			}
			goto ScreenSMBHeaderExit;
		}

		case SMB_COM_RENAME:
		{
			ULONG ulFileNameSize = 0;
			ULONG ulMaxSearch;
			WCHAR *pwzFileName;
			REQ_RENAME *pSMBRename;
			pSMBRename = (REQ_RENAME *)((char *)pData + sizeof(SMB_HEADER));
			ulMaxSearch = (pSMBRename->ByteCount) >> 1;
			ulMaxSearch -= 2;	//Take away the null terminate string and the two BufferFormats fields
			pwzFileName = (WCHAR *)(pSMBRename->Buffer + 1);
			while(TRUE)
			{
				if (pwzFileName[ulFileNameSize] == 0)
					break;
				ulFileNameSize++;
				if (ulFileNameSize >= ulMaxSearch)
				{
                                    DP(DSDERR,("    TDI:RENAME cannot find FileNameLength, let this pass bytecount:%d, sizeofREQ_RENAME:%d\n", pSMBRename->ByteCount, sizeof(REQ_RENAME)));
					bReturn = FALSE;
					goto ScreenSMBHeaderExit;
				}
			}

			if (FillSharedIFSTDIMem(pIFSTDISharedMem, ulRemoteAddr, ulLocalAddr,
									pSMBHeader, pwzFileName, ulFileNameSize, RENAME_ACTION) == FALSE)
			{
				DP(DSDNOLOG,("    TDI:RENAME cannot fill sharedmem, let this pass FileName = %S\n", pwzFileName));
				bReturn = FALSE;
				goto ScreenSMBHeaderExit;
			}
			else	
			{   
				bReturn = TRUE;
					
				DP(DSDDEV,("TDI:RENAME IP=0x%x(%d)\n", ulRemoteAddr,(ulRemoteAddr & 0xff)));	
				goto ScreenSMBHeaderExit;
			}
			goto ScreenSMBHeaderExit;
		}

		case SMB_COM_DELETE:
		{
			WCHAR			*pwzFileName;
			ULONG			ulFileNameSize;
			REQ_DELETE		*pReqDelete = NULL;
			pReqDelete = (REQ_DELETE *)((char *)pData + sizeof(SMB_HEADER));

			if (pReqDelete == NULL)
			{
				DP(DSDERR,("SMB_COM_DELETE<ERR>pReqDelete is NULL\n"));
				goto ScreenSMBHeaderExit;
			}
			
			pwzFileName = (WCHAR *)(pReqDelete->FileName);
			if (GetWStringSize(pwzFileName, MAX_NAME_LENGTH, &ulFileNameSize) == FALSE)
			{
				DP(DSDERR,("SMB_COM_DELETE<ERR>Get String size error\n"));
				goto ScreenSMBHeaderExit;
			}

			if (FillSharedIFSTDIMem(pIFSTDISharedMem, ulRemoteAddr, ulLocalAddr,
									pSMBHeader, pwzFileName, ulFileNameSize, DELETE_ACTION) == FALSE)
			{
				DP(DSDNOLOG,("    TDI:SMB_COM_DELETE cannot fill sharedmem, let this pass FileName = %S\n", pwzFileName));
				goto ScreenSMBHeaderExit;
			}
			else	
			{   
				bReturn = TRUE;				
				DP(DSDDEV,("TDI:SMB_COM_DELETE IP=0x%x(%d)\n", ulRemoteAddr,(ulRemoteAddr & 0xff)));	
				goto ScreenSMBHeaderExit;
			}

			goto ScreenSMBHeaderExit;


		}

		case SMB_COM_TRANSACTION2:
		{	
			//Use SubCommand TRANS2_QUERY_PATH_INFORMATION to enforce Open/Query Directory enforcement
			WCHAR			*pwzFileName;
			ULONG			ulFileNameSize;
			REQ_TRANSACTION	*pReqTransaction = NULL;
			REQ_QUERY_PATH_INFORMATION	*pQueryPathInfo = NULL;
			pReqTransaction = (REQ_TRANSACTION *)((char *)pData + sizeof(SMB_HEADER));

			if (pReqTransaction && pReqTransaction->Buffer[0] == TRANS2_QUERY_PATH_INFORMATION)
				pQueryPathInfo = (REQ_QUERY_PATH_INFORMATION *)((char *)pData + sizeof(SMB_HEADER) + sizeof(REQ_TRANSACTION) + 6);
			else
				goto ScreenSMBHeaderExit;

			if (pQueryPathInfo == NULL)
			{
				DP(DSDERR,("SMB_COM_TRANSACTION2<ERR>pQueryPathInfo is NULL\n"));
				goto ScreenSMBHeaderExit;
			}

			pwzFileName = (WCHAR *)(pQueryPathInfo->Buffer);

			if (GetWStringSize((WCHAR *)pQueryPathInfo->Buffer, MAX_NAME_LENGTH, &ulFileNameSize) == FALSE)
			{
				DP(DSDERR,("SMB_COM_TRANSACTION2<ERR>Get String size error\n"));
				goto ScreenSMBHeaderExit;
			}

			if (FillSharedIFSTDIMem(pIFSTDISharedMem, ulRemoteAddr, ulLocalAddr,
									pSMBHeader, pwzFileName, ulFileNameSize, 0) == FALSE)
			{
				DP(DSDNOLOG,("    TDI:SMB_COM_TRANSACTION2 cannot fill sharedmem, let this pass FileName = %S\n", pwzFileName));
				goto ScreenSMBHeaderExit;
			}
			else	
			{   
				bReturn = TRUE;				
				DP(DSDDEV,("TDI:SMB_COM_TRANSACTION2 IP=0x%x(%d)\n", ulRemoteAddr,(ulRemoteAddr & 0xff)));	
				goto ScreenSMBHeaderExit;
			}

			goto ScreenSMBHeaderExit;

//This is unuse block of code for handle TRANS2_FIND_FIRST2 .
#if 0	//hnx:begin
/*
			ULONG			ulFileNameSize;
			WCHAR			*pwzFileName;
			REQ_TRANSACTION	*pReqTransaction = NULL;
			REQ_FIND_FIRST2 *pReqFirst2 = NULL;

			DP(DSDDEV,("SMB_COM_TRANSACTION2\n"));
			pReqTransaction = (REQ_TRANSACTION *)((char *)pData + sizeof(SMB_HEADER));

			if (pReqTransaction && pReqTransaction->Buffer[0] == TRANS2_FIND_FIRST2)
				pReqFirst2 = (REQ_FIND_FIRST2 *)((char *)pData + sizeof(SMB_HEADER) + sizeof(REQ_TRANSACTION) + 6);
			else
				goto ScreenSMBHeaderExit;
			
			if (pReqFirst2 == NULL)
			{
				DP(DSDERR,("SMB_COM_TRANSACTION2<ERR>pReqFirst2 is NULL\n"));
				goto ScreenSMBHeaderExit;
			}

			pwzFileName = (WCHAR *)(pReqFirst2->Buffer);

			if (GetWStringSize((WCHAR *)pReqFirst2->Buffer, MAX_NAME_LENGTH, &ulFileNameSize) == FALSE)
			{
				DP(DSDERR,("SMB_COM_TRANSACTION2<ERR>Get String size error\n"));
				goto ScreenSMBHeaderExit;
			}

			if (FillSharedIFSTDIMem(pIFSTDISharedMem, ulRemoteAddr, ulLocalAddr,
									pSMBHeader, pwzFileName, ulFileNameSize) == FALSE)
			{
				DP(DSDERR,("    TDI:SMB_COM_TRANSACTION2 cannot fill sharedmem, let this pass\n"));
				goto ScreenSMBHeaderExit;
			}
			else	
			{   
				bReturn = TRUE;				
				DP(DSDNOLOG,("    TDI:FIRST_FIND2 IP=0x%x(%d) File = %S\n", \
							ulRemoteAddr,(ulRemoteAddr & 0xff), pwzFileName));	
				goto ScreenSMBHeaderExit;
			}
*/
#endif	//hnx:end
		}

#ifdef IPFLT
		case SMB_COM_TREE_DISCONNECT:
		{
			WCHAR	*pwzShareName;
			REQ_TREE_DISCONNECT *pTreeDis = NULL;
			DP(DSDDEV,("SMB_COM_TREE_DISCONNECT\n"));
			pTreeDis = (REQ_TREE_DISCONNECT *)((char *)pData + sizeof(SMB_HEADER));
			if ((pSMBHeader->Flags & 0x80) == 0x00)
			{
				DP(DSDINFO,("Disconnect*****************************->Tid = 0x%x\n",pSMBHeader->Tid));
			}
			goto ScreenSMBHeaderExit;
		}

		case SMB_COM_TREE_CONNECT_ANDX:
		{
			WCHAR	*pwzShareName;
			REQ_TREE_CONNECT_ANDX *pTreeConn = NULL;
			DP(DSDDEV,("SMB_COM_TREE_CONNECT_ANDX\n"));
			pTreeConn = (REQ_TREE_CONNECT_ANDX *)((char *)pData + sizeof(SMB_HEADER));
			if ((pSMBHeader->Flags & 0x80) == 0x00)
			{
				pwzShareName = (WCHAR *)((char *)pTreeConn->Buffer + 1);
				DP(DSDNOLOG,("Connect----------------------->ShareName=%S\n", pwzShareName));
			}
			goto ScreenSMBHeaderExit;
		}
#endif	//#ifdef IPFLT

		case SMB_COM_CREATE:
		{
			DP(DSDDEV,("SMB_COM_CREATE\n"));
			goto ScreenSMBHeaderExit;
		}
		case SMB_COM_CREATE_DIRECTORY:
		{
			DP(DSDDEV,("SMB_COM_DIRECTORY\n"));
			goto ScreenSMBHeaderExit;
		}
		case SMB_COM_DELETE_DIRECTORY:
		{
                    //bug 1619: This code should work if we fix PREQ_DELETE_DIRECTORY
			//WCHAR			*pwzFileName;
			//ULONG			ulFileNameSize;
			//PREQ_DELETE_DIRECTORY   pReqDelete = NULL;
			//pReqDelete = (PREQ_DELETE_DIRECTORY)((char *)pData + sizeof(SMB_HEADER));

			//if (pReqDelete == NULL)
			//{
			//	DP(DSDERR,("SMB_COM_DELETE_DIRECTORY<ERR>pReqDelete is NULL\n"));
			//	goto ScreenSMBHeaderExit;
			//}
			//
			//pwzFileName = (WCHAR *)(pReqDelete->Buffer);
			//if (GetWStringSize(pwzFileName, MAX_NAME_LENGTH, &ulFileNameSize) == FALSE)
			//{
			//	DP(DSDERR,("SMB_COM_DELETE_DIRECTORY<ERR>Get String size error\n"));
			//	goto ScreenSMBHeaderExit;
			//}

                        //DP (DSDWRN, ("delete dir: %S, count: %d", pwzFileName, pReqDelete->WordCount));

			//if (FillSharedIFSTDIMem(pIFSTDISharedMem, ulRemoteAddr, ulLocalAddr,
			//						pSMBHeader, pwzFileName, ulFileNameSize, DELETE_ACTION) == FALSE)
			//{
			//	DP(DSDNOLOG,("    TDI:SMB_COM_DELETE_DIRECTORY cannot fill sharedmem, let this pass FileName = %S\n", pwzFileName));
			//	goto ScreenSMBHeaderExit;
			//}
			//else	
			//{   
			//	bReturn = TRUE;				
			//	DP(DSDDEV,("TDI:SMB_COM_DELETE_DIRECTORY IP=0x%x(%d)\n", ulRemoteAddr,(ulRemoteAddr & 0xff)));	
			//	goto ScreenSMBHeaderExit;
			//}

			goto ScreenSMBHeaderExit;
		}
		case SMB_COM_OPEN:
		{
			DP(DSDDEV,("SMB_COM_OPEN\n"));
			goto ScreenSMBHeaderExit;
		}
		case SMB_COM_OPEN_ANDX:
		{
			DP(DSDDEV,("SMB_COM_OPEN_ANDX\n"));
			goto ScreenSMBHeaderExit;
		}
		case SMB_COM_OPEN_PRINT_FILE:
		{
			DP(DSDDEV,("SMB_COM_OPEN_PRINT_FILE\n"));
			goto ScreenSMBHeaderExit;
		}
	}
	
ScreenSMBHeaderExit:
	return bReturn;
}