/*++

Module Name:

  NLSECrypto.c

Abstract:
  Calling crypto API for encryption/decryption in kernel mode. Also,
  handling CBC cypher block mode. 

Environment:

    Kernel mode

--*/
#include "NLSEStruct.h"
#include "NLSEUtility.h"
#include "nl_crypto.h"
#include "FileOpHelp.h"

//
//  Global variables
//
extern NLFSE_GLOBAL_DATA nlfseGlobal;
extern NL_KLOG nlseKLog;

BOOLEAN decrypt_buffer( __in_bcount(keyLen) char  *decryptKey, 
			       __in size_t keyLen,
			       __in ULONGLONG startOffset,
			       __inout PVOID cryptoBuffer, 
			       __in size_t cryptoSize
			       )

{
#if NLSE_DEBUG_DATA_VERIFY
  ULONG i = 0;
  PUCHAR p = (PUCHAR)cryptoBuffer;
  for( i = 0 ; i < cryptoSize ; i++ )
  {
    /* Padding value may be 0x00, so ignore that.  Looking for non-padding and non-know
     * data size.
     */
    if( (UCHAR)p[i] != 0 && (UCHAR)p[i] != (UCHAR)'b' )
    {
      KdPrint(("NLSE: NLSE_DEBUG_DATA_VERIFY: Verification failed (0x%X,0x%X,0x%X,0x%X)\n",
	       (ULONG_PTR)cryptoBuffer,       /* Base address of memory */
	       (ULONG_PTR)i,                  /* Offset of mismatch in this block */
	       (ULONG_PTR)startOffset,        /* Offset of this block in the file */
	       (ULONG_PTR)p[i]));
#if NLSE_DEBUG_DATA_VERIFY_BUGCHECK
      KeBugCheckEx(0xBEEFFEED,
		   (ULONG_PTR)cryptoBuffer,       /* Base address of memory */
		   (ULONG_PTR)i,                  /* Offset of mismatch in this block */
		   (ULONG_PTR)startOffset,        /* Offset of this block in the file */
		   (ULONG_PTR)p[i]);              /* Value of unexpected byte */
#endif
    }
  }
#elif NLSE_DEBUG_CRYPTO_PASSTHROUGH
  /* Do nothing*/
#else
  /* Allocate initial vector and use the current offset as its value */
  UCHAR ivec[NLSE_KEY_LENGTH_IN_BYTES];
  PUCHAR cryptoBufferThisRun;
  void *pCryptoKey;

  ASSERT( NLSE_KEY_LENGTH_IN_BYTES >= keyLen );

  /* Allocate and init crypto key */
  pCryptoKey = ExAllocatePoolWithTag(NonPagedPool,
                                     nlfseGlobal.crypto_ctx.crypto_key_size,
                                     NLSE_CRYPTOKEY_TAG);
  if (pCryptoKey == NULL)
  {
    return FALSE;
  }

  if (nlfseGlobal.crypto_ctx.init_crypto_key(&nlfseGlobal.crypto_ctx,
                                             decryptKey, keyLen, FALSE,
                                             pCryptoKey)
      != NL_CRYPTO_ERROR_SUCCESS)
  {
    ExFreePoolWithTag(pCryptoKey, NLSE_CRYPTOKEY_TAG);
    return FALSE;
  }

  RtlZeroMemory(ivec,keyLen);
  cryptoBufferThisRun = cryptoBuffer;

  while (cryptoSize > 0)
  {
    size_t cryptoSizeThisRun = min(cryptoSize, nlfseGlobal.cryptoBlockSize);

    RtlCopyMemory(ivec,&startOffset,sizeof(startOffset));
    nlfseGlobal.crypto_ctx.decrypt(&nlfseGlobal.crypto_ctx,   /* context */
                                   pCryptoKey,                /* key */
                                   ivec,                      /* ivec */
                                   cryptoBufferThisRun,   /* caller's buffer */
                                   cryptoSizeThisRun);
    cryptoBufferThisRun += cryptoSizeThisRun;
    startOffset += cryptoSizeThisRun;
    cryptoSize -= cryptoSizeThisRun;
  }

  RtlSecureZeroMemory(ivec,keyLen);

  /* Securely destroy and free crypto key */
  nlfseGlobal.crypto_ctx.destroy_crypto_key(&nlfseGlobal.crypto_ctx,
                                            pCryptoKey);
  ExFreePoolWithTag(pCryptoKey, NLSE_CRYPTOKEY_TAG);

#endif
  return TRUE;
}

BOOLEAN encrypt_buffer(__in_bcount(keyLen) char  *encryptKey, 
			       __in size_t keyLen,
			       __in ULONGLONG startOffset,
			       __inout PVOID cryptoBuffer, 
			       __in size_t cryptoSize
			       )
{
#if NLSE_DEBUG_DATA_VERIFY
  RtlFillMemory(cryptoBuffer,cryptoSize,(UCHAR)'b');
#elif NLSE_DEBUG_CRYPTO_PASSTHROUGH
  /* Do nothing */
#else
  /* Allocate initial vector and use the current offset as its value */
  UCHAR ivec[NLSE_KEY_LENGTH_IN_BYTES];
  PUCHAR cryptoBufferThisRun;
  void *pCryptoKey;

  ASSERT( NLSE_KEY_LENGTH_IN_BYTES >= keyLen );

  /* Allocate and init crypto key */
  pCryptoKey = ExAllocatePoolWithTag(NonPagedPool,
                                     nlfseGlobal.crypto_ctx.crypto_key_size,
                                     NLSE_CRYPTOKEY_TAG);
  if (pCryptoKey == NULL)
  {
    return FALSE;
  }

  if (nlfseGlobal.crypto_ctx.init_crypto_key(&nlfseGlobal.crypto_ctx,
                                             encryptKey, keyLen, TRUE,
                                             pCryptoKey)
      != NL_CRYPTO_ERROR_SUCCESS)
  {
    ExFreePoolWithTag(pCryptoKey, NLSE_CRYPTOKEY_TAG);
    return FALSE;
  }

  RtlZeroMemory(ivec,keyLen);
  cryptoBufferThisRun = cryptoBuffer;

  while (cryptoSize > 0)
  {
    size_t cryptoSizeThisRun = min(cryptoSize, nlfseGlobal.cryptoBlockSize);

    RtlCopyMemory(ivec,&startOffset,sizeof(startOffset));
    nlfseGlobal.crypto_ctx.encrypt(&nlfseGlobal.crypto_ctx,   /* context */
                                   pCryptoKey,                /* key */
                                   ivec,                      /* ivec */
                                   cryptoBufferThisRun,   /* caller's buffer */
                                   cryptoSizeThisRun);
    cryptoBufferThisRun += cryptoSizeThisRun;
    startOffset += cryptoSizeThisRun;
    cryptoSize -= cryptoSizeThisRun;
  }

  RtlSecureZeroMemory(ivec,keyLen);

  /* Securely destroy and free crypto key */
  nlfseGlobal.crypto_ctx.destroy_crypto_key(&nlfseGlobal.crypto_ctx,
                                            pCryptoKey);
  ExFreePoolWithTag(pCryptoKey, NLSE_CRYPTOKEY_TAG);

#endif
  return TRUE;
}

//Generate random data encryption key
VOID
NLSEGenDataEncryptionKey(
	__inout PNLFSE_ENCRYPT_EXTENSION EncryptExtension)
{
  ULONG key[4]={0}; //128 bit long

  ASSERT( NULL != EncryptExtension );

#if NLSE_DEBUG_FAKE_FILE_KEY

  key[0] = (ULONG)'z';
  key[1] = (ULONG)'z';
  key[2] = (ULONG)'z';
  key[3] = (ULONG)'z';

#else

  nlfseGlobal.crypto_ctx.rand(&nlfseGlobal.crypto_ctx, (int*)&key[0]); /* random ULONG w/o presisted key */
  nlfseGlobal.crypto_ctx.rand(&nlfseGlobal.crypto_ctx, (int*)&key[1]); /* random ULONG w/o presisted key */
  nlfseGlobal.crypto_ctx.rand(&nlfseGlobal.crypto_ctx, (int*)&key[2]); /* random ULONG w/o presisted key */
  nlfseGlobal.crypto_ctx.rand(&nlfseGlobal.crypto_ctx, (int*)&key[3]); /* random ULONG w/o presisted key */

#endif
  
  RtlCopyMemory(EncryptExtension->key, key, NLSE_KEY_LENGTH_IN_BYTES);
}

//encrypt encryption indicator
BOOLEAN NLSEEncryptIndicator(__inout PNLFSE_ENCRYPT_EXTENSION pExt,
			     __in    char                     *key,
			     __in    NLSE_KEY_ID              *keyID,
			     __in    char                     *keyRingName)
{
  RtlCopyMemory(&(pExt->pcKeyID),
		keyID,
		sizeof(NLSE_KEY_ID));
  RtlCopyMemory(pExt->pcKeyRingName,
		keyRingName,
		NLSE_KEY_RING_NAME_MAX_LEN*sizeof(char));
  encrypt_buffer(key, 
		 NLSE_KEY_LENGTH_IN_BYTES, //encryption key
		 0, //ivec
		 pExt->key, //buffer to be encrypted 
		 NLSE_KEY_LENGTH_IN_BYTES); 
  return TRUE;
}

//decrypt encryption indicator
BOOLEAN NLSEDecryptIndicator(__inout PNLFSE_ENCRYPT_EXTENSION pExt,
			     __in    char                     *key,
			     __in    size_t                   keyLen)
{
  decrypt_buffer(key, NLSE_KEY_LENGTH_IN_BYTES, //decryption key
		 0, //ivec
		 pExt->key, 
		 NLSE_KEY_LENGTH_IN_BYTES); //buffer to be decrypted  
  return TRUE;
}

BOOLEAN 
NLSECalculatePadding(__inout NLFSE_PPRE_2_POST_CONTEXT ctx,
		     __in PFILE_END_OF_FILE_INFORMATION endOfFile)
{
  LARGE_INTEGER paddingOffset;
  LARGE_INTEGER tmpOffset;
  ULONG         tmpLen, tmpLen1;
  KIRQL         irql; 

  //Check if it needs to record new padding 
  if((endOfFile->EndOfFile.QuadPart)%nlfseGlobal.cbcBlockSize == 0) {
    //aligned with cbc block size; no need padding
    ctx->paddingLen=0;
    ctx->paddingBuf = NULL;
    return TRUE;
  } else if (ctx->cryptoBuffer == NULL) {
    //no enough information for padding; no need for padding
    //this might be caused by the change of file status, e.g.
    //new write happens and new size after the corresponding 
    //io to this work item (ctx)
    return FALSE;
  }

  if(ctx->cryptoOffset.QuadPart > endOfFile->EndOfFile.QuadPart) {
    //the file size has been changed to much smaller size
    //than cryptoOffset. This work item's information for padding
    //is not relevant to the file any more. Return FALSE so no
    //need to update padding info in ADS
    return FALSE;
  }

  if((ctx->cryptoOffset.QuadPart+ctx->cryptoBufferSize.QuadPart) <
     endOfFile->EndOfFile.QuadPart) {
    //the file size has been changed to much larger size
    //than cryptoOffset+cryptoBufferSize. This work item's 
    //information for padding is not relevant to the file any more. 
    //Return FALSE so no need to update padding info in ADS
    return FALSE;
  }

  if((ctx->cryptoOffset.QuadPart+ctx->cryptoLen.QuadPart) <
     endOfFile->EndOfFile.QuadPart) {
    //the file size has been changed to much larger size
    //than cryptoOffset+cryptoLen. This work item's 
    //information for padding is not relevant to the file any more. 
    //Return FALSE so no need to update padding info in ADS
    return FALSE;
  }

  tmpOffset.QuadPart=ROUND_TO_SIZE(endOfFile->EndOfFile.QuadPart,
				   nlfseGlobal.cbcBlockSize);
  //padding length
  ctx->paddingLen=(ULONG)(tmpOffset.QuadPart-
			  endOfFile->EndOfFile.QuadPart);
  ctx->paddingBuf = ExAllocatePoolWithTag(NonPagedPool, 
					  ctx->paddingLen,
					  NLFSE_BUFFER_TAG);
  if(ctx->paddingBuf == NULL) {
    NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_ERR, 
		"NLSE!CalculatePadding: allocate %d bytes memory\n",
		ctx->paddingLen);
    return FALSE;
  }
  RtlZeroMemory(ctx->paddingBuf, ctx->paddingLen);
  paddingOffset.QuadPart=endOfFile->EndOfFile.QuadPart-
    ctx->cryptoOffset.QuadPart;
  if((ctx->cryptoOffset.QuadPart+ctx->cryptoLen.QuadPart)<
     tmpOffset.QuadPart) {
    //we need recorded padding information
    //Get per stream lock on encryption extension
    KeAcquireSpinLock(&ctx->streamCtx->encryptExtLock,
		      &irql);     
    if(ctx->streamCtx->encryptExt->paddingLen == 0) {
      //no padding information ??!!
      KeReleaseSpinLock(&ctx->streamCtx->encryptExtLock,
			irql);
      return FALSE;
    }
    //First, update padding with the buffer from file end to 
    //end of read in data
    tmpLen=(ULONG)(ctx->cryptoOffset.QuadPart+
		   ctx->cryptoLen.QuadPart-endOfFile->EndOfFile.QuadPart);
    RtlCopyMemory(ctx->paddingBuf,
		  &((PUCHAR)ctx->cryptoBuffer)[paddingOffset.QuadPart],
		  tmpLen);
    tmpLen1=(ULONG)(tmpOffset.QuadPart-
		    ctx->cryptoOffset.QuadPart-ctx->cryptoLen.QuadPart);
    if(ctx->streamCtx->encryptExt->paddingLen < tmpLen1) {
      //File status change; this work item's information is not relevant 
      //any more
      //Release per stream lock on encryption extension
      KeReleaseSpinLock(&ctx->streamCtx->encryptExtLock,
			irql);
      return FALSE;
    }
    RtlCopyMemory(&((PUCHAR)ctx->paddingBuf)[tmpLen],
		  &((PUCHAR)ctx->streamCtx->encryptExt->paddingData)[0],
		  tmpLen1);    
    //Release per stream lock on encryption extension
    KeReleaseSpinLock(&ctx->streamCtx->encryptExtLock,
		      irql);
    NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_DEBUG, 
		"NLSE:CalculatePadding:!!padding %d use recorded padding %d\n",
		ctx->paddingLen, tmpLen1);    
  } else if ((ctx->cryptoOffset.QuadPart+ctx->cryptoLen.QuadPart)>=
	     tmpOffset.QuadPart){
    //update padding with the buffer from file end to tmpOffset
    RtlCopyMemory(ctx->paddingBuf,
		  &((PUCHAR)ctx->cryptoBuffer)[paddingOffset.QuadPart],
		  ctx->paddingLen);		
    NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_DEBUG, 
		"NLSE:CalculatePadding:!!!record padding %d\n",
		ctx->paddingLen);    
  }

  return TRUE;
}

BOOLEAN
IsZeroBuffer(
             __in PUCHAR Buffer,
             __in ULONG Length
             )
{
    ULONG i=0;
    for (i=0; i<Length; i++)
    {
        if(0 != Buffer[i])
            return FALSE;
    }
    return TRUE;
}

//Do CBC cipher block encryption
//return false if the operation failed
BOOLEAN NLSEEncryptionAtPreWrite(__in PFLT_CALLBACK_DATA Data, 
				 __in PCFLT_RELATED_OBJECTS FltObjects,   
				 __in_opt NLFSE_PPRE_2_POST_CONTEXT p2pCtx,
				 __in ULONG writeLen,
				 __inout PUCHAR outBuf, 
				 __in PUCHAR inBuf)
{
    PFLT_IO_PARAMETER_BLOCK   iopb = Data->Iopb;//IO parameter block
    FILE_STANDARD_INFORMATION stFileInfo;
    LARGE_INTEGER             writeOffset;      //written data offset
    LARGE_INTEGER             readOffset;       //read offset  
    LARGE_INTEGER             tmpOffset;        //temporary offset
    NTSTATUS                  status;           //NT status
    FLT_IO_OPERATION_FLAGS    flags;            //FLT operation flags
    LARGE_INTEGER             bufferSize;       //size of buffer
    PVOID                     buffer=NULL;      //buffer
    ULONG                     bytesRead;        //bytes read
    NLFSE_PVOLUME_CONTEXT     volCtx;
    PNLFSE_ENCRYPT_EXTENSION  pExt;

    __try
    {
        //Based on OSROnline (http://www.osronline.com/showthread.cfm?link=176455),
        //paging i/o and non cached i/o are aligned with sector size, except 
        //for the last page the length is less than integral. 
        //Therefore, the write offset is aligned with sector size (min=512 bytes)
        //but might not with page size (4k) which is equivelant to cryto block
        //size. 
        if(p2pCtx == NULL)
            return FALSE;
        volCtx=p2pCtx->VolCtx;

        if(volCtx == NULL) 
            return FALSE;

        if(p2pCtx->streamCtx == NULL)
            return FALSE;

        if(p2pCtx->streamCtx->encryptExt == NULL)
            return FALSE;
        pExt=p2pCtx->streamCtx->encryptExt;

        //decide read offset in case that we need to read in some data
        //to align with crypto block size
        //first, be aligned with cypher block size 
        readOffset.QuadPart =  (iopb->Parameters.Write.ByteOffset.QuadPart/nlfseGlobal.cryptoBlockSize)*nlfseGlobal.cryptoBlockSize;

        //decide buffer size
        bufferSize.QuadPart =  iopb->Parameters.Write.ByteOffset.QuadPart - readOffset.QuadPart;
        bufferSize.QuadPart += writeLen;
        //round up to cypher block size (nlfseGlobal.cryptoBlockSize) (and volume sector (512))
        bufferSize.QuadPart =  ROUND_TO_SIZE(bufferSize.QuadPart, nlfseGlobal.cryptoBlockSize);

        //allocate read buffer
        buffer = FltAllocatePoolAlignedWithTag( FltObjects->Instance, NonPagedPool, (size_t)(bufferSize.QuadPart), NLFSE_BUFFER_TAG);
        if (NULL == buffer)
        {
            NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_ERR, "NLSE!EncryptionAtPreWrite: can't allocate %d memory\n", bufferSize.QuadPart);
            return FALSE;
        }
        RtlZeroMemory(buffer, (size_t)(bufferSize.QuadPart));    

        if(readOffset.QuadPart != iopb->Parameters.Write.ByteOffset.QuadPart)
        {
            //write offset is not aligned with crypto block size (4K)
            //we need to read in some data first, and decrypt it before
            //before encryption
            //1. read in data
            status = NLSESendSynchronousReadIO(FltObjects, readOffset, buffer, bufferSize);
            if(!NT_SUCCESS(status))
            {
                NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_ERR, ("NLSE!EncryptionAtPreWrite: failed to read in data\n"));
                FltFreePoolAlignedWithTag( FltObjects->Instance, buffer, NLFSE_BUFFER_TAG );    
                return FALSE;      
            }
            //Even if the read in bytes is not aligned with cbcBlockSize (16bytes),
            //we don't need to worry about it because the extra part should 
            //be overwritten by the new written data because the write has to 
            //be aligned with sector size which (512 bytes) is aligned with 
            //cbcBlockSize (16 bytes)
            //2. decrypt the data;
            if(!decrypt_buffer(pExt->key, NLSE_KEY_LENGTH_IN_BYTES, readOffset.QuadPart, &((PUCHAR)buffer)[0], (size_t)(bufferSize.QuadPart)))
            {
                NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_ERR, "NLSE!EncryptionAtPreWrite: can't decrypt %d bytes\n", bufferSize.QuadPart);
                FltFreePoolAlignedWithTag( FltObjects->Instance, buffer, NLFSE_BUFFER_TAG );    
                return FALSE;      
            }
            //copy the new written data to the buffer
            tmpOffset.QuadPart = (iopb->Parameters.Write.ByteOffset.QuadPart - readOffset.QuadPart);
            RtlCopyMemory(&((PUCHAR)buffer)[tmpOffset.QuadPart], inBuf, writeLen);      
        }
        else
        {    
            //overwrite buffer with the data to be written
            RtlCopyMemory(buffer, inBuf, writeLen);
        }

        //re-encrypte the data with the new written data
        //calling crypto api to encrypt data
        if(!encrypt_buffer(pExt->key, NLSE_KEY_LENGTH_IN_BYTES, readOffset.QuadPart, buffer, (size_t)(bufferSize.QuadPart)))
        {
                NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR, "NLSE!EncryptionAtPreWrite: can't decrypt %d bytes\n", bufferSize);
                FltFreePoolAlignedWithTag( FltObjects->Instance, buffer, NLFSE_BUFFER_TAG );    
                return FALSE;      
        }

        //copy the encrypted data to the output buffer
        if(readOffset.QuadPart != iopb->Parameters.Write.ByteOffset.QuadPart)
        {
            RtlCopyMemory(outBuf, &((PUCHAR)buffer)[tmpOffset.QuadPart], writeLen);
        }
        else
        {
            //write offset is aligned with crypto block 
            RtlCopyMemory(outBuf, buffer, writeLen);
        }

        //Assign data to p2pCtx for padding handling in worker thread
        //after successful write operation
        p2pCtx->cryptoOffset             = readOffset;
        p2pCtx->cryptoBufferSize.QuadPart= bufferSize.QuadPart;
        p2pCtx->cryptoBuffer             = buffer;
        p2pCtx->cryptoLen.QuadPart       = bufferSize.QuadPart;

        NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_DEBUG, ("NLSE!EncryptionAtPreWrite: succeed\n"));
        return TRUE;      

    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        return FALSE;
    }  
}/*NLSEEncryptionAtPreWrite*/
 
//Do CBC cipher block decryption
//return false if the operation failed
BOOLEAN NLSEDecryptionAtPostRead(__in PFLT_CALLBACK_DATA Data, 
				 __in PCFLT_RELATED_OBJECTS FltObjects,   
				 __in_opt  NLFSE_PPRE_2_POST_CONTEXT p2pCtx,
				 __inout PUCHAR outBuf, 
				 __in PUCHAR inBuf)
{
    PFLT_IO_PARAMETER_BLOCK      iopb = Data->Iopb; //IO parameter block
    LARGE_INTEGER                offset; //read offset
    LARGE_INTEGER                cryptoOffset; //crypto offset
    LARGE_INTEGER                startOffset; //initial vector for CBC
    NTSTATUS                     status; //NT status
    FLT_IO_OPERATION_FLAGS       flags;  //FLT operation flags
    size_t                       bufferSize; //size of buffer
    size_t                       cryptoSize; //size of crypto buffer
    FILE_STANDARD_INFORMATION    stFileInfo;
    OBJECT_ATTRIBUTES            objAttr;
    PFILE_OBJECT                 fileHandle;
    IO_STATUS_BLOCK              ioStatusBlock;
    HANDLE                       handle;
    ULONG                        bytesRead; //bytes really read
    BOOLEAN                      bNeedPadding=FALSE;
    KIRQL                        irql;

    __try
    {
        // Sanity check to avoid preFast warning 6011
        if(NULL==p2pCtx || NULL==p2pCtx->streamCtx || 
            NULL==p2pCtx->streamCtx->encryptExt)
        {
            NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_ERR, ("NLSE!DecryptionAtPostRead: Fail to decrypt because p2pCtx has NULL pointer\n"));
            return FALSE;  
        }

        //Get per stream lock on encryption extension
        KeAcquireSpinLock(&p2pCtx->streamCtx->encryptExtLock, &irql);    

        //Check if need to use padding data for decryption
        if(p2pCtx->realReadSize < p2pCtx->newSize)
        {
            //reach the end of file
            if(p2pCtx->realReadSize%nlfseGlobal.cbcBlockSize != 0)
            {
                //need to append padding buffer to the end of real read-in data
                if(p2pCtx->streamCtx->encryptExt && p2pCtx->streamCtx->encryptExt->paddingLen > 0 )
                {
                    //make sure inBuf is big enough
                    if(p2pCtx->newSize >= (p2pCtx->realReadSize+ p2pCtx->streamCtx->encryptExt->paddingLen))
                    {
                        //append padding buffer for decryption
                        RtlCopyMemory(&((PUCHAR)inBuf)[p2pCtx->realReadSize], p2pCtx->streamCtx->encryptExt->paddingData, p2pCtx->streamCtx->encryptExt->paddingLen);
                    }
                    else
                    {
                        //invoke bug check
                        //Release per stream lock on encryption extension
                        KeReleaseSpinLock(&p2pCtx->streamCtx->encryptExtLock, irql);    
                        NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_ERR, ("NLSE!DecryptionAtPostRead: inBuf too small\n"));
                        return FALSE;  
                    }
                }
                else
                {
                    NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_ERR, ("NLSE!DecryptionAtPostRead: padding is not correct\n"));
                }
            }
        }

        //Release per stream lock on encryption extension
        KeReleaseSpinLock(&p2pCtx->streamCtx->encryptExtLock, irql);    

        //decide crypto offset within read "buffer"
        //first, be aligned with cypher block size
        cryptoOffset=iopb->Parameters.Read.ByteOffset;
        cryptoOffset.QuadPart /= nlfseGlobal.cryptoBlockSize;
        cryptoOffset.QuadPart *= nlfseGlobal.cryptoBlockSize;
        startOffset=cryptoOffset;
        //second, get crypto offset  
        cryptoOffset.QuadPart = cryptoOffset.QuadPart-p2pCtx->newOffset.QuadPart;

        //calling crypto api to decrypt data
        if(p2pCtx->cryptoSize+cryptoOffset.QuadPart > p2pCtx->newSize)
        {
            NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_ERR, "NLSE!DecryptionAtPostRead: WRONG c=%d o=%d n=%d\n", p2pCtx->cryptoSize, cryptoOffset.QuadPart, p2pCtx->newSize);
            return FALSE;
        }
        if(!decrypt_buffer(p2pCtx->streamCtx->encryptExt->key, NLSE_KEY_LENGTH_IN_BYTES, startOffset.QuadPart, &((PUCHAR)inBuf)[cryptoOffset.QuadPart], p2pCtx->cryptoSize))
        {
                NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_ERR, "NLSE!DecryptionAtPostRead: can't decrypt %d bytes\n", p2pCtx->cryptoSize);
                return FALSE;      
        }

        //get decrypted data offset within "buffer"
        cryptoOffset=iopb->Parameters.Read.ByteOffset;
        cryptoOffset.QuadPart -= p2pCtx->newOffset.QuadPart;

        //assign decrypted data to output buffer
        RtlCopyMemory(outBuf, &(((PUCHAR)inBuf)[cryptoOffset.QuadPart]), (size_t)(Data->IoStatus.Information));

        //clean up
        NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_DEBUG, 
            "NLSE!DecryptionAtPostRead: succeed offset=%d read-length=%d real-length=%d file-size=%d\n",
            iopb->Parameters.Read.ByteOffset.QuadPart, (size_t)iopb->Parameters.Read.Length, (size_t)(Data->IoStatus.Information), p2pCtx->streamCtx->encryptExt->fileRealLength);
        return TRUE;    
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_ERR, ("NLSE!DecryptionAtPostRead: exception\n"));
        return FALSE;
    }  
}/*NLSEDecryptionAtPostRead*/

//Do decryption when the buffer is safe to use
FLT_POSTOP_CALLBACK_STATUS
NLSEPostReadDecryptionWhenSafe (
    __inout PFLT_CALLBACK_DATA Data,
    __in PCFLT_RELATED_OBJECTS FltObjects,
    __in PVOID CompletionContext,
    __in FLT_POST_OPERATION_FLAGS Flags
    )
/*++

Routine Description:

    We had an arbitrary users buffer without a MDL so we needed to get
    to a safe IRQL so we could lock it and then copy the data.

Arguments:

    Data - Pointer to the filter callbackData that is passed to us.

    FltObjects - Pointer to the FLT_RELATED_OBJECTS data structure containing
        opaque handles to this filter, instance, its associated volume and
        file object.

    CompletionContext - Contains state from our PreOperation callback

    Flags - Denotes whether the completion is successful or is being drained.

Return Value:

    FLT_POSTOP_FINISHED_PROCESSING - This is always returned.

--*/
{
  PFLT_IO_PARAMETER_BLOCK iopb = Data->Iopb;
  NLFSE_PPRE_2_POST_CONTEXT p2pCtx = CompletionContext;
  PVOID origBuf;
  NTSTATUS status;

  __try {
    UNREFERENCED_PARAMETER( FltObjects );
    UNREFERENCED_PARAMETER( Flags );
    ASSERT(Data->IoStatus.Information != 0);

    //set really read in data size
    p2pCtx->realReadSize=(ULONG)Data->IoStatus.Information;

    //reset to the orignal read length
    Data->IoStatus.Information=p2pCtx->originalLen;

    //
    //  This is some sort of user buffer without a MDL, lock the user buffer
    //  so we can access it.  This will create a MDL for it.
    //
    status = FltLockUserBuffer( Data );
    if (!NT_SUCCESS(status)) {
      NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR,
		  "NLSE!PostReadDecryptionWhenSafe: %wZ Could not lock user buffer, oldB=%p, status=%x\n",
		  &p2pCtx->VolCtx->Name,
		  iopb->Parameters.Read.ReadBuffer,
		  status);

      //
      //  If we can't lock the buffer, fail the operation
      //
      Data->IoStatus.Status = status;
      Data->IoStatus.Information = 0;
    } else {
      //
      //  Get a system address for this buffer.
      //
      origBuf = MmGetSystemAddressForMdlSafe( iopb->Parameters.Read.MdlAddress,
					      NormalPagePriority );
      if (origBuf == NULL) {
	NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR,
		    "NLSE!PostReadDecryptionWhenSafe: %wZ Failed to get system address for MDL: %p\n",
		    &p2pCtx->VolCtx->Name,
		    iopb->Parameters.Read.MdlAddress);

	//  If we couldn't get a SYSTEM buffer address, fail the operation
	Data->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
	Data->IoStatus.Information = 0;
      } else {
	//
	//  Copy the data back to the original buffer.  Note that we
	//  don't need a try/except because we will always have a system
	//  buffer address.
	//
	if(!NLSEDecryptionAtPostRead(Data, 
				     FltObjects, 
				     p2pCtx,
				     origBuf, 
				     p2pCtx->SwappedBuffer)) {
	  Data->IoStatus.Status = STATUS_UNSUCCESSFUL;
	  Data->IoStatus.Information = 0;
	}
	NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_DEBUG, 
		    "NLSE!PostReadDecryptionWhenSafe:decrypt:%d %wZ\n",
		    Data->IoStatus.Information,
		    &p2pCtx->streamCtx->FileName);
      }
    }

    //  Free allocated memory and release the volume context
    if(p2pCtx) {
      ExFreePoolWithTag( p2pCtx->SwappedBuffer, NLFSE_BUFFER_TAG );
      FltReleaseContext( p2pCtx->VolCtx );
      FltReleaseContext( p2pCtx->streamCtx );
      if(p2pCtx->instanceCtx) {
	FltReleaseContext(p2pCtx->instanceCtx);
      }
      ExFreeToNPagedLookasideList( &nlfseGlobal.pre2PostContextList,
				   p2pCtx );
    }
    return FLT_POSTOP_FINISHED_PROCESSING;
  } __except(EXCEPTION_EXECUTE_HANDLER) {
    return FLT_POSTOP_FINISHED_PROCESSING;
  }
}/*--NLSEPostReadDecryptionWhenSafe--*/

//Encrypt a file by reading and re-writing it; 
//and fill in the encryption extension buffer
NTSTATUS NLSEEncryptFile(__in  PFLT_FILTER                  Filter,
                         __in  PFLT_INSTANCE                Instance,
                         __in  NLFSE_PVOLUME_CONTEXT        volCtx,
                         __in  PUNICODE_STRING              fileName,
                         __out PNLFSE_ENCRYPT_EXTENSION     pEncryptExt)
{
    OBJECT_ATTRIBUTES            objAttr;
    IO_STATUS_BLOCK              ioStatusBlock;
    HANDLE                       handle = NULL;
    PFILE_OBJECT                 fileHandle = NULL;
    NTSTATUS                     status = STATUS_SUCCESS;
    PVOID                        buffer;
    size_t                       BufferSize=nlfseGlobal.cryptoBlockSize;
    FILE_POSITION_INFORMATION    posInfo;
    FILE_STANDARD_INFORMATION    stFileInfo;
    FILE_END_OF_FILE_INFORMATION endOfFile;
    FLT_IO_OPERATION_FLAGS       rwFlags;  //FLT operation flags
    LARGE_INTEGER                startOffset;
    PNLFSE_STREAM_CONTEXT        streamCtx = NULL;
    BOOLEAN                      ToEnd = FALSE;
	FILE_BASIC_INFORMATION      fbi             = {0};
	BOOLEAN						AttrModified = FALSE;

    if(KeGetCurrentIrql() != PASSIVE_LEVEL)
        return STATUS_INVALID_DEVICE_STATE;	

	status = FOH_GET_FILE_BASICINFO_BY_NAME(Filter,Instance,fileName,&fbi);
    if(NT_SUCCESS(status))
    {
		if(fbi.FileAttributes & FILE_ATTRIBUTE_READONLY)
		{
			status = FOH_REMOVE_READONLY_ATTRIBUTES(Filter,Instance,fileName);
			if(NT_SUCCESS(status))
			{
				AttrModified = TRUE;
			}
		}
    }

    //Initialization
    BufferSize=ROUND_TO_SIZE(BufferSize, volCtx->SectorSize);
    RtlZeroMemory(&stFileInfo, sizeof(stFileInfo));
    InitializeObjectAttributes(&objAttr,
        fileName,
        OBJ_KERNEL_HANDLE,
        NULL,
        NULL);

    //Open the file for encryption
    status = FltCreateFile(Filter,
        Instance,
        &handle,
        GENERIC_WRITE|GENERIC_READ|SYNCHRONIZE,
        &objAttr,
        &ioStatusBlock,
        0,
        FILE_ATTRIBUTE_NORMAL,
        FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE, 
        FILE_OPEN,
        FILE_COMPLETE_IF_OPLOCKED|FILE_SYNCHRONOUS_IO_NONALERT,
        NULL,
        0,
        IO_IGNORE_SHARE_ACCESS_CHECK);
    if (!NT_SUCCESS(status)) {
        NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_ERR, 
            "NLSE!EncryptFile: create failed 0x%x %wZ\n", 
            status, fileName);
        handle = NULL;
        goto _exit;
    }

    // Get file object from the handle
    status = ObReferenceObjectByHandle(handle,       //Handle
        0,            //DesiredAccess
        NULL,         //ObjectType
        KernelMode,   //AccessMode
        &fileHandle,  //File Handle
        NULL);

    if (!NT_SUCCESS(status)) {
        NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_ERR, 
            "NLSE!EncryptFile: reference obj failed 0x%x %wZ\n", 
            status, fileName);
        fileHandle = NULL;
        goto _exit;
    }

    //streamCtx = NLFSEFindExistingContext(volCtx, fileHandle);
	status = FltGetStreamContext( Instance,
                                  fileHandle,
                                  &streamCtx );
    if(!NT_SUCCESS(status) || NULL == streamCtx)
    {
        NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR, "NLSE!The file (%wZ) has no perstream context\n", fileName);
        status = STATUS_UNSUCCESSFUL;
        goto _exit;
    } 

    streamCtx->IgnoreWrite = TRUE;
    status = FltFlushBuffers(Instance,fileHandle);

    //Check if the file is deleted
    ExAcquireFastMutex(&streamCtx->deleteFlagLock);
    if(streamCtx->bDelete == TRUE) {
        //file is being deleted or has been deleted; 
        ExReleaseFastMutex(&streamCtx->deleteFlagLock);
        NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_DEBUG, 
            "NLSE!EncryptFile: File is pending for delete %wZ\n", 
            &streamCtx->FileName);	  
        status=STATUS_DELETE_PENDING;  
        goto _exit;
    }
    ExReleaseFastMutex(&streamCtx->deleteFlagLock);

    //Get file size
    status = FltQueryInformationFile(Instance, 
        fileHandle,
        &stFileInfo, 
        sizeof(FILE_STANDARD_INFORMATION), 
        FileStandardInformation, 
        NULL);
    if(!NT_SUCCESS(status)) {
        NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_ERR, 
            "NLFSE!EncryptFile: query file info failed: 0x%x %wZ\n", 
            status, fileName);
        goto _exit;
    } else if( stFileInfo.DeletePending == TRUE) {
        //file is pending for delete
        NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_DEBUG, 
            "NLSE!EncryptFile: File is pending for delete %wZ\n", 
            fileName);
        status=STATUS_DELETE_PENDING;  
        goto _exit;
    }

    pEncryptExt->fileRealLength = stFileInfo.EndOfFile.QuadPart; 
    endOfFile.EndOfFile.QuadPart = stFileInfo.EndOfFile.QuadPart; 
    NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_DEBUG, 
        "NLFSE!EncryptFile: size=%d %wZ\n", 
        pEncryptExt->fileRealLength, fileName);

    // allocate copy buffer
    buffer = FltAllocatePoolAlignedWithTag( Instance, 
        NonPagedPool,
        BufferSize, 
        NLFSE_BUFFER_TAG);
    if (NULL == buffer) {
        NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_ERR, 
            "NLFSE!EncryptFile: allocated copy buffer %d failed\n", 
            BufferSize);
        status = STATUS_INSUFFICIENT_RESOURCES; // Allocation must have failed.
        goto _exit;
    }

    rwFlags=FLTFL_IO_OPERATION_NON_CACHED;
    startOffset.QuadPart=0;
    while (TRUE) {
        ULONG bytesRead = 0;
        ULONG bytesWritten = 0;
        ULONG bytesWrite = 0;

        RtlZeroMemory(buffer, BufferSize);
        status = FltReadFile( Instance, 
            fileHandle,
            &startOffset,
            BufferSize, 
            buffer,
            rwFlags, 
            &bytesRead, 
            NULL, 
            NULL);	
        if (!NT_SUCCESS(status) || 0 == bytesRead)
        {		
            status = STATUS_SUCCESS;
            break;
        }

        //encrypt the data
        encrypt_buffer(pEncryptExt->key,
            NLSE_KEY_LENGTH_IN_BYTES,
            startOffset.QuadPart,
            buffer,
            BufferSize);
        if(bytesRead < BufferSize) 
        {
            //reach the end of file; record padding
            ToEnd = TRUE;
            if((bytesRead%nlfseGlobal.cbcBlockSize)!=0)
            {
                pEncryptExt->paddingLen=nlfseGlobal.cbcBlockSize;
                pEncryptExt->paddingLen-=(bytesRead%nlfseGlobal.cbcBlockSize);
                RtlZeroMemory(pEncryptExt->paddingData, NLSE_PADDING_DATA_LEN);
                RtlCopyMemory(pEncryptExt->paddingData, &((PUCHAR)buffer)[bytesRead], pEncryptExt->paddingLen);
            }
            else
            {
                pEncryptExt->paddingLen = 0;
                RtlZeroMemory(pEncryptExt->paddingData, NLSE_PADDING_DATA_LEN);
            }
        }
        else
        {
            if((startOffset.QuadPart + bytesRead) == endOfFile.EndOfFile.QuadPart)
            {
                ToEnd = TRUE;
            }
        }

        // write encrypted data back to file
        bytesWrite = bytesRead;
        status = FltWriteFile( Instance, 
            fileHandle,
            &startOffset,
            bytesWrite, 
            buffer,
            rwFlags,
            &bytesWritten,  
            NULL, NULL);
        if (!NT_SUCCESS(status) || bytesWritten != bytesWrite) {
            NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_ERR, 
                "NLFSE!EncryptFile: failed write s=0x%x w=%d r=%d %wZ\n", 
                status, bytesWritten, bytesWrite, fileName);
            break;
        }

        //advance start offset
        startOffset.QuadPart+=bytesRead;
        if(ToEnd) break;
    }

    // Flush data to disk
    status = FltFlushBuffers(Instance,fileHandle);

    //clean up
    FltFreePoolAlignedWithTag( Instance, buffer, NLFSE_BUFFER_TAG );

_exit:
	if(AttrModified)
	{
		FOH_SET_FILE_BASICINFO_BY_NAME(Filter, Instance, fileName, &fbi);
	}

    if(NULL != streamCtx)
    {
        streamCtx->IgnoreWrite = FALSE;
        FltReleaseContext(streamCtx);
    }
    if(NULL != fileHandle)
        ObDereferenceObject(fileHandle);
    if(NULL != handle)
        FltClose(handle);

    if(NT_SUCCESS(status))
    {
        NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_DEBUG,
            "NLSE!EncryptFile: succeed to encrypt file %wZ\n",
            fileName);   
    }
    else
    {
        NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_ERR,
            "NLSE!EncryptFile: fail to encrypt file %wZ\n",
            fileName);   
    }
    return status;
}/*NLSEEncryptFile*/
