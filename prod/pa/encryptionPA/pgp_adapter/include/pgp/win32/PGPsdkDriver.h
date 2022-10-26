/*____________________________________________________________________________
	Copyright (C) 2002 PGP Corporation
	All rights reserved.

	pgpUtilityDriverWin32.h - header for callers of Win32 PGP Utility Driver


	$Id$

____________________________________________________________________________*/

#ifndef _pgpUtilityDriverWin32_h
#define _pgpUtilityDriverWin32_h

#if ! PGP_WIN32
#error this file should only be used for PGP_WIN32
#endif

#ifndef PGPSDK_DRIVER
#define PGPSDK_DRIVER 0
#endif

#define kPGPUDMaxVersionStringLength			80
#define kPGPUDMaxCacheIndexLength				64
#define kPGPUDMaxCacheDataLength				64

/* driver-specific error codes */
#define kPGPUDError_NoErr						0x0000
#define kPGPUDError_UndefinedOperation			0x0001
#define kPGPUDError_BadParams					0x0002
#define kPGPUDError_MemAllocError				0x0003
#define kPGPUDError_BufferTooSmall				0x0004
#define kPGPUDError_DriverUninitialized			0x0005

#define kPGPUDError_MemLockError				0x0101
#define kPGPUDError_MemUnlockError				0x0102
#define kPGPUDError_LockListError				0x0103

#define kPGPUDError_WipePending					0x0201

#define kPGPUDError_ItemNotFound				0x0301


/* virtual memory paging in Win32 (Intel) uses 4K byte pages (2^12) */
#define WIN32PAGESIZE	12

/* Define the various device type values.  Note that values used by Microsoft
   Corporation are in the range 0-32767, and 32768-65535 are reserved for use
   by customers. */

#define FILE_DEVICE_PGPUTILITY					0x00008001

/* Macro definition for defining IOCTL and FSCTL function control codes.  Note
   that function codes 0-2047 are reserved for Microsoft Corporation, and
   2048-4095 are reserved for customers. */

#define PGPUTILITY_IOCTL_INDEX					0x800

/* For defining our own private IOCTLs */
#define METHOD_BUFFERED							0
#define FILE_ANY_ACCESS							0

#define CTL_CODE( DeviceType, Function, Method, Access ) (                 \
    ((DeviceType) << 16) | ((Access) << 14) | ((Function) << 2) | (Method) \
)

/*____________________________________________________________________________

	Generic driver functions

	The calling Win32 application should initialize a PGPGENERICSTRUCT with
	an operation code and then pass the address of the structure to the driver
	via a call to DeviceIOControl.

	Currently these generic operations are supported:

		kPGPUDOperation_QueryVersion
			the driver will copy a null-terminated version string to ucBuffer.
		kPGPUDOperation_QueryStatus
			the driver will return status flags in ulFlags.

	Possible return values of ulError :
		kPGPUDError_NoErr
		kPGPUDError_UndefinedOperation


	Example:
		PGPGENERICSTRUCT	pgs;
		DWORD				dw;

		pgs.ulOperation = kPGPUDOperation_QueryVersion;
		bDIOreturn = DeviceIoControl(	hPGPUtilityDriver,
										IOCTL_PGPUTIL_GENERIC,
										&pgs,
										sizeof( pgs ),
										&pgs,
										sizeof( pgs ),
										&dw,
										NULL );
		if ( !bDIOreturn )
		{
			// this is an error communicating with the driver
		}

		else
		{
			if ( pgs.ulError != kPGPUDError_NoErr )
			{
				// this is an error internal to the driver
			}
			else
			{
				printf ("\nDriver version : %s\n", pgs.ucBuffer);
			}
		}

____________________________________________________________________________*/

#define IOCTL_PGPUTIL_GENERIC			CTL_CODE(FILE_DEVICE_PGPUTILITY, \
                                                 PGPUTILITY_IOCTL_INDEX,  \
                                                 METHOD_BUFFERED,     \
                                                 FILE_ANY_ACCESS)

#define kPGPUDOperation_QueryVersion			0x0000
#define kPGPUDOperation_QueryStatus				0x0001

#define kPGPUDFlag_MemlockInitialized			0x0001
#define kPGPUDFlag_WipeDeleteInitialized		0x0002
#define kPGPUDFlag_KeyboardHookInstalled		0x0004
#define kPGPUDFlag_MouseHookInstalled			0x0008
#define kPGPUDFlag_KeyboardHookCalled			0x0010
#define kPGPUDFlag_MouseHookCalled				0x0020
#define kPGPUDFlag_InactivityTimerRunning		0x0040

typedef struct {
	ULONG	ulOperation;
	ULONG	ulError;
	ULONG	ulFlags;
	UCHAR	ucBuffer[kPGPUDMaxVersionStringLength];
} PGPGENERICSTRUCT, *PPGPGENERICSTRUCT;


/*____________________________________________________________________________

	Memory locking functions

	This kernel mode device driver is used to lock memory pages into RAM
	so that they will not get copied to the system page file.  This
	functionality is desired when allocating buffers that will contain
	sensitive information (e.g. passwords) which could present a security
	hazard if copied to the page file.

	The calling application should initialize a PGPMEMLOCKSTRUCT and pass
	the address of the structure to the driver via a call to DeviceIoControl.

	Currently these memory locking operations are supported:

		kPGPUDOperation_LockMemory
			The driver will lock the memory, preventing it from being paged
			out to the paging file.  The calling application must set these
			values in the PGPMEMLOCKSTRUCT :

			pMem		- address of memory block to be locked
			ulNumBytes	- number of bytes in memory block to be locked

		kPGPUDOperation_UnlockMemory
			The driver will unlock the specified memory block.  The calling
			application must set these values in the PGPMEMLOCKSTRUCT :

			pMem		- address of memory block to be unlocked
			ulNumBytes	- number of bytes in memory block to be unlocked

  	Possible return values of ulError :
		kPGPUDError_NoErr
		kPGPUDError_UndefinedOperation
		kPGPUDError_DriverUninitialized
		kPGPUDError_MemLockError
		kPGPUDError_MemUnlockError
		kPGPUDError_LockListError
		kPGPUDError_MemAllocError


	Example :

		PGPMEMLOCKSTRUCT	pmls;
		DWORD				dw;
		BOOL				bDIOreturn;
		PVOID				pMem;
		ULONG				ulNumBytes;

		// allocate the memory
		ulNumBytes = number_of_bytes_to_allocate;
		pMem = VirtualAlloc ( NULL, ulNumBytes, MEM_COMMIT, PAGE_READWRITE );

		// lock the memory
		pmls.ulOperation	= kPGPUDOperation_LockMemory
		pmls.pMem			= pMem;
		pmls.ulNumBytes		= ulNumBytes;

		bDIOreturn = DeviceIoControl(	hPGPUtilityDriver,
										IOCTL_PGPUTIL_MEMLOCK,
										&pmls,
										sizeof( pmls ),
										&pmls,
										sizeof( pmls ),
										&dw,
										NULL );
		if ( !bDIOreturn )
		{
			// this is an error communicating with the driver
		}

		else
		{
			if ( mls.ulError != kPGPUDError_NoErr )
			{
				// this is an error internal to the driver
			}
			else
			{
				// successfully locked!
			}
		}


		// unlocking the memory
		pmls.ulOperation	= kPGPUDOperation_UnlockMemory
		pmls.pMem			= pMem;
		pmls.ulNumBytes		= ulNumBytes;
		bDIOreturn = DeviceIoControl(	hPGPUtilityDriver,
										IOCTL_PGPUTIL_MEMLOCK,
										&pmls,
										sizeof( pmls ),
										&pmls,
										sizeof( pmls ),
										&dw,
										NULL );
		if ( !bDIOreturn )
		{
			// this is an error communicating with the driver
		}

		else
		{
			if ( mls.ulError != kPGPUDError_NoErr )
			{
				// this is an error internal to the driver
			}
			else
			{
				// successfully unlocked!
			}
		}

		// free the memory
		VirtualFree( pMem, 0, MEM_RELEASE );


____________________________________________________________________________*/

#define IOCTL_PGPUTIL_MEMLOCK			CTL_CODE(FILE_DEVICE_PGPUTILITY,  \
                                                 PGPUTILITY_IOCTL_INDEX+1,\
                                                 METHOD_BUFFERED,     \
                                                 FILE_ANY_ACCESS)

#define kPGPUDOperation_LockMemory				0x0000
#define kPGPUDOperation_UnlockMemory			0x0001

#if PGP_WIN32
#pragma pack(8)
#endif
typedef struct {
	ULONG	ulOperation;
	ULONG	ulError;
	PVOID	pMem;
	ULONG	ulNumBytes;
} PGPMEMLOCKSTRUCT, *PPGPMEMLOCKSTRUCT;

#ifdef _WIN64
typedef struct {
	ULONG	ulOperation;
	ULONG	ulError;
	VOID * POINTER_32	pMem;
	ULONG	ulNumBytes;
} PGPMEMLOCKSTRUCT_32, *PPGPMEMLOCKSTRUCT_32;
#endif

#if PGP_WIN32
#pragma pack()
#endif



/*____________________________________________________________________________

	Keyboard/mouse entropy functions

	These functions are used to request entropy from the driver.  The driver
	continuously monitors keyboard and mouse events and derives cryptographic
	quality entropy from them.

	The calling application should initialize a PGPENTROPYSTRUCT and pass the
	address of the structure to the driver via a call to DeviceIoControl.

	Currently these entropy operations are supported:

		kPGPUDOperation_QueryEntropy
			The driver will inform the caller of the number of bits of entropy
			currently present in the entropy pool.  The number of bits will
			be returned in ulEntropyBits.

		kPGPUDOperation_GetEntropy
			The driver will attempt to provide the requested amount of entropy
			to the caller.  The driver will always fill the buffer with the
			requested number of random bytes and will return, in ulEntropyBits,
			the amount of entropy provided.  The caller should set these
			parameters in the PGPENTROPYSTRUCT :

			ulBufferLength		- length of buffer (in bytes) to fill with
								  random data


  	Possible return values of ulError :
		kPGPUDError_NoErr
		kPGPUDError_UndefinedOperation


	Example:

		PGPENTROPYSTRUCT*		ppes;
		DWORD					dw;
		INT						iNumberOfBytes;
		INT						iStructSize;

		iNumberOfBytes	= max_number_of_bytes_of_entropy_needed;
		iStructSize		= sizeof(PGPENTROPYSTRUCT) + iNumberOfBytes -1;

		ppes = malloc (iStructSize);
		ppes->ulOperation = kPGPUDOperation_QueryEntropy;

		bDIOreturn = DeviceIoControl(	hPGPUtilityDriver,
										IOCTL_PGPUTIL_ENTROPY,
										ppes,
										iStructSize,
										ppes,
										iStructSize,
										&dw,
										NULL );
		if ( !bDIOreturn )
		{
			// this is an error communicating with the driver
		}

		else
		{
			if ( pgs.ulError != kPGPUDError_NoErr )
			{
				// this is an error internal to the driver
			}
			else
			{
				// driver successfully queried, now get the random data
				if (ppes->ulEntropyBits >= bits_needed)
				{
					ppes->ulOperation		= kPGPUDOperation_GetEntropy;
					ppes->ulBufferLength	= bits_needed/8 +1;

					bDIOreturn = DeviceIoControl(	hPGPUtilityDriver,
													IOCTL_PGPUTIL_ENTROPY,
													ppes,
													iStructSize,
													ppes,
													iStructSize,
													&dw,
													NULL );

					if ( !bDIOreturn )
					{
						// this is an error communicating with the driver
					}

					else
					{
						if ( pgs.ulError != kPGPUDError_NoErr )
						{
							// this is an error internal to the driver
						}
						else
						{
							// see if we really got the data
							if (ppes->ulEntropyBits >= bits_needed)
							{
								// success!
							}
						}
					}
				}
			}
		}

____________________________________________________________________________*/


#define IOCTL_PGPUTIL_ENTROPY			CTL_CODE(FILE_DEVICE_PGPUTILITY, \
                                                 PGPUTILITY_IOCTL_INDEX+2,  \
                                                 METHOD_BUFFERED,     \
                                                 FILE_ANY_ACCESS)

#define kPGPUDOperation_QueryEntropy			0x0000
#define kPGPUDOperation_GetEntropy				0x0001

#if PGP_WIN32
#pragma pack(8)
#endif
typedef struct {
	ULONG	ulOperation;
	ULONG	ulError;
	ULONG	ulEntropyBits;
	ULONG	ulBufferLength;
	UCHAR	ucEntropyBuffer[1];
} PGPENTROPYSTRUCT, *PPGPENTROPYSTRUCT;
#if PGP_WIN32
#pragma pack()
#endif

/*____________________________________________________________________________

	Passkey cache functions

	These functions are used to cache passkeys for signing and decryption.

	The calling application should initialize a PGPCACHESTRUCT and pass the
	address of the structure to the driver via a call to DeviceIoControl.

	Currently these cache operations are supported:

		kPGPUDOperation_PurgeCache
			The driver will purge the contents of the specified cache.  The
			caller must specify the cache type (kPGPUDCache_Signing or
			kPGPUDCache_Decryption) in ulCache.

		kPGPUDOperation_SetCacheValue
			The driver will insert the index and data into the specified cache
			and begin timing the cache entry using the specified time value.
			After the specified number of seconds have elapsed without the
			entry having been accessed, the entry will be removed from the
			cache.  The caller must specify these values :

			ulCache			- cache to use (kPGPUDCache_Signing or
							  kPGPUDCache_Decryption)
			ulSeconds		- cache entry duration
			ulIndexLength	- length of cache index
			ucIndex			- buffer of data to use as index
			ulDataLength	- passkey length
			ucData			- passkey data buffer

		kPGPUDOperation_QueryCacheValue
			The driver will search the specified cache for the specified index
			value.  If a match is found, the data from the cache is copied to
			the ucData buffer and ulDataLength is set.  The caller must specify
			these values :

			ulCache			- cache to search (kPGPUDCache_Signing or
							  kPGPUDCache_Decryption)
			ulIndexLength	- length of cache index
			ucIndex			- buffer of data to use as index

  	Possible return values of ulError :
		kPGPUDError_NoErr
		kPGPUDError_UndefinedOperation
		kPGPUDError_ItemNotFound
		kPGPUDError_MemAllocError


	Example:

		struct {
			INT			keyalg;
			PGPKeyID	keyid;
		} datastruct;

		PGPCACHESTRUCT		pcs;
		DWORD				dw;

		// add entry to cache
		pcs.ulOperation		= kPGPUDOperation_SetCacheValue;
		pcs.ulCache			= kPGPUDCache_Signing;
		pcs.ulSeconds		= duration;
		pcs.ulIndexLength	= sizeof(datastruct);
		memcpy (&pcs.ucIndex, &datastruct, sizeof(datastruct));
		ulDataLength		= passkeylength;
		memcpy (&pcs.ucData, pPasskey, passkeylength);

		bDIOreturn = DeviceIoControl(	hPGPUtilityDriver,
										IOCTL_PGPUTIL_CACHE,
										&pcs,
										sizeof( pcs ),
										&pcs,
										sizeof( pcs ),
										&dw,
										NULL );
		if ( !bDIOreturn )
		{
			// this is an error communicating with the driver
		}

		else
		{
			if ( pgs.ulError != kPGPUDError_NoErr )
			{
				// this is an error internal to the driver
			}
			else
			{
				// entry added to cache successfully
			}
		}


		// retrieve entry from cache
		pcs.ulOperation		= kPGPUDOperation_QueryCacheValue;
		pcs.ulCache			= kPGPUDCache_Signing;
		pcs.ulIndexLength	= sizeof(datastruct);
		memcpy (&pcs.ucIndex, &datastruct, sizeof(datastruct));

		bDIOreturn = DeviceIoControl(	hPGPUtilityDriver,
										IOCTL_PGPUTIL_CACHE,
										&pcs,
										sizeof( pcs ),
										&pcs,
										sizeof( pcs ),
										&dw,
										NULL );
		if ( !bDIOreturn )
		{
			// this is an error communicating with the driver
		}

		else
		{
			if ( pgs.ulError != kPGPUDError_NoErr )
			{
				// this is an error internal to the driver
			}
			else
			{
				// entry found in cache, copy data
				memcpy (pPasskey, &pcs.ucData, pcs.ulDataLength);
			}
		}

____________________________________________________________________________*/

#define IOCTL_PGPUTIL_CACHE				CTL_CODE(FILE_DEVICE_PGPUTILITY, \
                                                 PGPUTILITY_IOCTL_INDEX+3,  \
                                                 METHOD_BUFFERED,     \
                                                 FILE_ANY_ACCESS)

#define kPGPUDOperation_PurgeCache				0x0000
#define kPGPUDOperation_SetCacheValue			0x0001
#define kPGPUDOperation_QueryCacheValue			0x0002

#define kPGPUDCache_Signing						0x0000
#define kPGPUDCache_Decryption					0x0001

#if PGP_WIN32
#pragma pack(8)
#endif
typedef struct {
	ULONG	ulOperation;
	ULONG	ulError;
	ULONG	ulCache;
	ULONG	ulSeconds;
	ULONG	ulIndexLength;
	UCHAR	ucIndex[kPGPUDMaxCacheIndexLength];
	ULONG	ulDataLength;
	UCHAR	ucData[kPGPUDMaxCacheDataLength];
} PGPCACHESTRUCT, *PPGPCACHESTRUCT;
#if PGP_WIN32
#pragma pack()
#endif


/*____________________________________________________________________________

	Keyboard/mouse inactivity timer functions

	These functions are used to set inactivity timer callback functions.  When
	the specified time has elapsed without any keyboard key presses or mouse
	button presses, the callback function is called by the driver.

	IMPORTANT: A callback function can only be set by another driver, not by
	a Win32 application!

	The calling driver should initialize a PGPINACTIVITYSTRUCT and pass the
	address of the structure to the driver via a call to IoCallDriver (WinNT)
	or via calls to Get_DDB/Directed_Sys_Control (Win9x).

	Notes:
	1) under NT, the driver responds to an IRP_MJ_INTERNAL_DEVICE_CONTROL
	function code.  Therefore, set the InternalDeviceIoControl parameter of
	IoBuildDeviceIoControlRequest to TRUE.
	2) under 9x, when calling Directed_Sys_Control, the address of the
	PGPINACTIVITYSTRUCT should be passed in the ESI register.

	Currently these inactivity operations are supported:

		kPGPUDOperation_CreateInactivityTimer
			The driver will create a new inactivity timer and add it to its
			list of timers.  The caller should set these parameters in
			the PGPINACTIVITYSTRUCT :

			ulFlags		- set to kPGPUDFlag_RepeatingInactivityTimer if
						  timer is to repeat itself.  Otherwise timer destroys
						  itself after calling the callback function.
			ulSeconds	- inactivity timeout value in seconds
			callback	- PGPInactivityCallbackFunc to call if ulSeconds of
						  inactivity elapse
			ulUserVal	- value which will be passed back to the callback
						  function for use the by the caller

			A handle to the newly created timer is returned to the caller in
			ulTimerHandle.  This handle is used in any subsequent calls to
			destroy the timer (see below).  The timeout value of a timer cannot
			be changed.  The timer should be destroyed and new one created if
			a new timeout value is needed.

		kPGPUDOperation_DestroyInactivityTimer
			The driver will destroy the timer specified by the ulTimerHandle
			parameter.

  	Possible return values of ulError :
		kPGPUDError_NoErr
		kPGPUDError_UndefinedOperation
		kPGPUDError_ItemNotFound
		kPGPUDError_MemAllocError


	Example 1 (using the PGPsdk driver from WinNT kernel mode driver) :

		#define PGPUTIL_DEVICE_NAME		L"\\Device\\PGPsdkDriver"

		PDEVICE_OBJECT			g_pdevUtil			= NULL;
		ULONG					g_ulTimerHandle;
		MYCONTEXTSTRUCT			g_myContext;

		VOID myCallbackFunction (ULONG ulUserVal)
		{
			MYCONTEXTSTRUCT*	pContext = (MYCONTEXTSTRUCT*)ulUserVal;

			// do whatever we need to do when the user is inactive
		}

		static NTSTATUS
		sSendIRP (
			PVOID		pbuf,
			ULONG		lenbuf)
		{
			UNICODE_STRING 		szDeviceName;
			PFILE_OBJECT 		pfo;

			KEVENT 				event;
			NTSTATUS 			ntstatus;

			PIRP 				pIRP;
			IO_STATUS_BLOCK 	ioStatus;

			if (g_pdevUtil == NULL)
			{
				// get a DeviceObject for the utility driver.
				RtlInitUnicodeString (&szDeviceName, PGPUTIL_DEVICE_NAME);
				ntstatus = IoGetDeviceObjectPointer (&szDeviceName,
							FILE_ALL_ACCESS, &pfo, &g_pdevUtil);

				if (!NT_SUCCESS (ntstatus))
				{
					return ntstatus;
				}

				// increment the pointer ref count
				ntstatus = ObReferenceObjectByPointer (g_pdevUtil,
							FILE_ALL_ACCESS, NULL, KernelMode);

				// decrement the ref count to the unused file object
				ObDereferenceObject (pfo);
			}

			// Create notification event object to be used to signal the
			// request completion.
			KeInitializeEvent (&event, NotificationEvent, FALSE);

			// Build the synchronous request to be sent to the utility driver
			// to perform the request.  Allocate an IRP to issue the port internal
			// device control call.
			pIRP = IoBuildDeviceIoControlRequest (IOCTL_PGPUTIL_INACTIVITY,
						g_pdevUtil, pbuf, lenbuf, pbuf, lenbuf,
						TRUE, &event, &ioStatus);

			// Call the utility driver to act on the supplied buffer.
			// If the returned status is PENDING, wait for the request to complete.
			ntstatus = IoCallDriver (g_pdevUtil, pIRP);

			if (ntstatus == STATUS_PENDING)
			{
				(VOID)KeWaitForSingleObject (&event, Suspended, KernelMode,
												FALSE, NULL);
			}
			else
			{
				ioStatus.Status = ntstatus;
			}
			return(ioStatus.Status);
		}

		StartInactivityTimer (VOID)
		{
			PGPINACTIVITYSTRUCT		pis;
			DWORD					dw;

			pis.ulOperation = kPGPUDOperation_CreateInactivityTimer;
			pis.ulFlags		= kPGPUDFlag_RepeatingInactivityTimer;
			pis.ulSeconds	= 60;	// callback if one minute of inactivity
			pis.callback	= myCallbackFunction;
			pis.ulUserVal	= (ULONG)&g_myContext;

			if (!NT_SUCCESS (sSendIRP (&pis, sizeof(pis))))
			{
				// this is an error communicating with the driver
			}

			else
			{
				if ( pgs.ulError != kPGPUDError_NoErr )
				{
					// this is an error internal to the driver
				}
				else
				{
					// timer successfully created, save handle
					g_ulTimerHandle = pis.ulTimerHandle;
				}
			}
		}

		StopInactivityTimer (VOID)
		{
			PGPINACTIVITYSTRUCT		pis;

			pis.ulOperation = kPGPUDOperation_DestroyInactivityTimer;
			pis.ulTimerHandle = g_ulTimerHandle;

			if (!NT_SUCCESS (sSendIRP (&pis, sizeof(pis))))
			{
				// this is an error communicating with the driver
			}

			else
			{
				if ( pgs.ulError != kPGPUDError_NoErr )
				{
					// this is an error internal to the driver
				}
				else
				{
					// timer successfully destroyed
				}
			}
		}


	Example 2 (Using the PGPsdk driver from Win9x VxD) :

		struct VxD_Desc_Block*	g_pDDB				= NULL;
		ULONG					g_ulTimerHandle;
		MYCONTEXTSTRUCT			g_myContext;

		VOID myCallbackFunction (ULONG ulUserVal)
		{
			MYCONTEXTSTRUCT*	pContext = (MYCONTEXTSTRUCT*)ulUserVal;

			// do whatever we need to do when the user is inactive
		}

		StartInactivityTimer (VOID)
		{
			PGPINACTIVITYSTRUCT		pis;
			DWORD					dw;

			pis.ulOperation = kPGPUDOperation_CreateInactivityTimer;
			pis.ulFlags		= kPGPUDFlag_RepeatingInactivityTimer;
			pis.ulSeconds	= 60;	// callback if one minute of inactivity,
									// set to whatever you want
			pis.callback	= myCallbackFunction;
			pis.ulUserVal	= (ULONG)&g_myContext;

			if (g_pDDB == NULL)
				g_pDDB = Get_DDB (0, "PGPSDK  ");

			if (g_pDDB == NULL)
			{
				error finding driver ...
			}
			else
			{
				Directed_Sys_Control (g_pDDB, kPGPUDControl_Inactivity,
											0, 0, (ULONG)&pis, 0);

				if ( pgs.ulError != kPGPUDError_NoErr )
				{
					// this is an error internal to the driver
				}
				else
				{
					// timer successfully created, save handle
					g_ulTimerHandle = pis.ulTimerHandle;
				}
			}
		}

		StopInactivityTimer (VOID)
		{
			PGPINACTIVITYSTRUCT		pis;

			pis.ulOperation = kPGPUDOperation_DestroyInactivityTimer;
			pis.ulTimerHandle = g_ulTimerHandle;

			if (g_pDDB == NULL)
				g_pDDB = Get_DDB (0, "PGPSDK  ");

			if (g_pDDB == NULL)
			{
				error finding driver ...
			}
			else
			{
				Directed_Sys_Control (g_pDDB, kPGPUDControl_Inactivity,
											0, 0, (ULONG)&pis, 0);

				if ( pgs.ulError != kPGPUDError_NoErr )
				{
					// this is an error internal to the driver
				}
				else
				{
					// timer successfully destroyed
				}
			}
		}

____________________________________________________________________________*/


#define IOCTL_PGPUTIL_INACTIVITY		CTL_CODE(FILE_DEVICE_PGPUTILITY, \
                                                 PGPUTILITY_IOCTL_INDEX+4,  \
                                                 METHOD_BUFFERED,     \
                                                 FILE_ANY_ACCESS)

/* normally defined in VMM.h */
#ifndef BEGIN_RESERVED_PRIVATE_SYSTEM_CONTROL
#define BEGIN_RESERVED_PRIVATE_SYSTEM_CONTROL	0x70000000
#endif //BEGIN_RESERVED_PRIVATE_SYSTEM_CONTROL

#define kPGPUDControl_Inactivity	BEGIN_RESERVED_PRIVATE_SYSTEM_CONTROL +1

#define kPGPUDOperation_CreateInactivityTimer	0x0000
#define kPGPUDOperation_DestroyInactivityTimer	0x0001

#define kPGPUDFlag_RepeatingInactivityTimer		0x0001

typedef VOID (* PGPInactivityCallbackFunc)(ULONG);

#if PGP_WIN32
#pragma pack(8)
#endif
typedef struct
{
	ULONG	ulOperation;
	ULONG	ulError;
	ULONG	ulFlags;
	ULONG	ulSeconds;
	ULONG	ulTimerHandle;

	PGPInactivityCallbackFunc	callback;
	ULONG						ulUserValue;
} PGPINACTIVITYSTRUCT, *PPGPINACTIVITYSTRUCT;
#if PGP_WIN32
#pragma pack()
#endif

/*____________________________________________________________________________

	File Wipe-on-Delete functions

	These functions are used to get notifications from the Windows File System
	when it is about to delete a file.  This allows us to automatically wipe
	files before they are deleted.

	The user should set up a new thread that is suspended via
	WaitForSingleObjectEx. This thread is then registered with the
	driver by calling kPGPUDOperation_RegisterThread. To start the
	scheduling of wipe on deletes, use a kPGPUDOperation_RegisterCallback
	call to register the address of the user function that should
	be called for deletes. The parameter passed to the user function
	is a kernel level pointer. To translate it into a filename, the
	user must pass in a pointer to a PGPWDTRAPRECORD structure that contains
	the kernel address plus some memory in which to store the filename
	via a kPGPUDOperation_MapMemory call. Once the filename is
	obtained within the user mode callback function, the file wiping
	can proceed normally in user mode. When the wiping is done,
	the user application must call kPGPUDOperation_WipingDone, again
	with a pointer to a PGPWDTRAPRECORD structure so that internal
	memory can be freed and the driver can clean up.

	The callback function should have the form:

	void __stdcall UserCallback(void *keFileRecord)

	where the keFileRecord is a pointer in kernel mode memory which must
	be translated using kPGPUDOperation_MapMemory.

	The calling application should initialize a PGPWIPEDELETESTRUCT and pass
	the address of the structure to the driver via a call to DeviceIoControl.

	Currently these wipe-on-delete operations are supported:

		kPGPUDOperation_RegisterThread
			The user must do this call first and register a suspended
			thread with the driver, since this is the user thread
			that will actually be scheduled to do the callback. There
			are no parameters to be set, since the thread will be
			determined by the calling thread.

		kPGPUDOperation_RegisterCallback
			The driver will store the pointer to the callback function.  The
			callback function will be notified when the file system is about
			to delete a file.  The calling application must set these
			values in the PGPWIPEDELETESTRUCT :

			usCallback - address of user mode callback function

 		kPGPUDOperation_MapMemory
			This operation is used to translate the kernel address
			received by the callback to an actual filename accessible
			by the user application. The calling application sends
			in the kernel pointer and memory to receive the filename
			in the PGPWIPEDELETESTRUCT. The calling application must
			set these values in PGPWIPEDELETESTRUCT :

			keFileRecord - Value received by user callback from driver

			and receives these values back in the same structure

			usFileName - Actual filename in userspace

		kPGPUDOperation_WipingDone
			This is sent to the driver when the application has finished
			wiping the file which the driver previously indicated was ready
			for deletion.  The calling application must set these value
			in PGPWIPEDELETESTRUCT :

			keFileRecord - Value received by user callback from driver

  	Possible return values of ulError :
		kPGPUDError_NoErr
		kPGPUDError_WipePending
		kPGPUDError_UndefinedOperation
		kPGPUDError_DriverUninitialized


	Example :

		See PTwipedl.c in PGPtray source code

____________________________________________________________________________*/


#define IOCTL_PGPUTIL_WIPEDELETE		CTL_CODE(FILE_DEVICE_PGPUTILITY, \
                                                 PGPUTILITY_IOCTL_INDEX+5,  \
                                                 METHOD_BUFFERED,     \
                                                 FILE_ANY_ACCESS)

#define kPGPUDOperation_RegisterCallback		0x0000
#define kPGPUDOperation_RegisterThread			0x0001
#define kPGPUDOperation_WipingDone				0x0002
#define kPGPUDOperation_MapMemory				0x0003
#define kPGPUDOperation_RegisterFlags			0x0004
#define kPGPUDOperation_VolFlush				0x0008

#define WDFLAG_NOOP								0x0000
#define WDFLAG_WIPEFILES						0x0001

typedef VOID (* PGPWipeDeleteCallbackFunc)(void *);

#if PGP_WIN32
#pragma pack(8)
#endif
typedef struct
{
	ULONG	ulOperation;
	ULONG	ulError;

	PGPWipeDeleteCallbackFunc	usCallback;
	ULONG						ulFlags;
	VOID*						keFileRecord;
	CHAR						usFileName[1024]; // UTF8 for binary compatibility with 8x
	ULONG						ulVolume;
} PGPWIPEDELETESTRUCT, *PPGPWIPEDELETESTRUCT;
#if PGP_WIN32
#pragma pack()
#endif

/* ****************************************************/
#define IOCTL_PGPUTIL_WDELOCKDELETE		CTL_CODE(FILE_DEVICE_PGPUTILITY, \
                                                 PGPUTILITY_IOCTL_INDEX+6,  \
                                                 METHOD_BUFFERED,     \
                                                 FILE_ANY_ACCESS)

#define kPGPwdeInstrumentFileLock	0x0000
#define kPGPwdeInstrumentFileUnlock	0x0001
#define kPGPwdeInstrumentFileDelete	0x0002
#define kPGPwdePlacementFileLock	0x0003
#define kPGPwdePlacementFileUnlock	0x0004
#define kPGPwdePlacementFileDelete	0x0005

#if PGP_WIN32
#pragma pack(8)
#endif
typedef struct
{
	ULONG	ulOperation;
	ULONG	ulError;
	ULONG	ulVolumeNumber;
} PGPWDEINSTRUMENTFILESTRUCT, *PPGPWDEINSTRUMENTFILESTRUCT;
#if PGP_WIN32
#pragma pack()
#endif

/* ****************************************************/

#if ! PGPSDK_DRIVER
PGPBoolean	pgpSDKDriverIsInstalled(void);
PGPBoolean	pgpSDKEntropyDriverIsRunning(void);
PGPBoolean	pgpSDKMouseEntropyDriverIsWorking(void);
PGPBoolean	pgpSDKKeyboardEntropyDriverIsWorking(void);
PGPUInt32	pgpDriverRandomPoolGetEntropy(void);
PGPError	pgpDriverRandomPoolGetBytesEntropy(void *buf, PGPSize len);
PGPError	pgpSDKInitDriver(void);
#endif

#endif //_pgpUtilityDriverWin32_h




