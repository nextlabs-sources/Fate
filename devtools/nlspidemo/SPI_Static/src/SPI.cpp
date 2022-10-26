// SPI.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "spi.h"


WSPUPCALLTABLE      gMainUpCallTable;   // Winsock upcall table
LPPROVIDER          gLayerInfo = NULL;  // Provider information for each layer under us
int                 gLayerCount = 0;    // Number of providers layered over
CRITICAL_SECTION    gCriticalSection;


static  GUID guidDummy2 = 
{ 0xf2d9e916, 0x1031, 0x400d, { 0xb5, 0xf1, 0xb8, 0xb4, 0xc4, 0xd6, 0xa8, 0x17 } };

static int  gStartupCount = 0;      // Global startup count (for every WSPStartup call)

void SetGUID(GUID id)
{
	gProviderGuid = id;
}


void SPI_Init()
{
	InitializeCriticalSection( &gCriticalSection );
}

void SPI_Uninit()
{
	DeleteCriticalSection( &gCriticalSection );
		
	LspDestroyHeap();
}

/*************************************************
function:
	Display SPI providers information
Parameters:
	int nCatalog
	0: show 32 bit providers
	1: show 64 bit providers
	2: show both 32 and 64 bit providers
*************************************************/
int DisplayLsp(int nCatalog, bool bDetail)
{
	int nRet = 0;
	LspCreateHeap( &nRet );

	WINSOCK_CATALOG eCatalog;
	if( nCatalog == 0)
		eCatalog = LspCatalog32Only;
	else if(nCatalog == 1)
		eCatalog = LspCatalog64Only;
	else
		eCatalog = LspCatalogBoth;

	BOOL bLayeredOnly = FALSE;
	// Print the 32-bit catalog
	if ( ( LspCatalogBoth == eCatalog ) || ( LspCatalog32Only == eCatalog ) )
	{
		printf( "\n\nWinsock 32-bit Catalog:\n" );
		printf( "=======================\n" );
		PrintProviders( LspCatalog32Only, bLayeredOnly, bDetail );
	}
	// Print the 64-bit catalog
	if ( ( LspCatalogBoth == eCatalog ) || ( LspCatalog64Only == eCatalog ) )
	{
		printf( "\n\nWinsock 64-bit Catalog:\n" );
		printf( "=======================\n" );
		PrintProviders( LspCatalog64Only, bLayeredOnly, bDetail );
	}


	LspDestroyHeap();
	return 0;
}

int UninstallLsp(int nCatalog, GUID guidProvider)
{
	int nRet = 0;
	LspCreateHeap( &nRet );

	WINSOCK_CATALOG eCatalog;
	if( nCatalog == 0)
		eCatalog = LspCatalog32Only;
	else if(nCatalog == 1)
		eCatalog = LspCatalog64Only;
	else
		eCatalog = LspCatalogBoth;

	nRet = DeinstallProvider(eCatalog, &guidProvider);
	
	LspDestroyHeap();
	return 0;
}

/*************************************************
function:
	Install a new lsp provider
Parameters:
	int nCatalog
	0: show 32 bit providers
	1: show 64 bit providers
	2: show both 32 and 64 bit providers
*************************************************/
int InstallNewLsp(
        int			   nCatalog,
		GUID		   guidProvider,
        __in_z char    *lpszLspName,
        __in_z char    *lpszLspPathAndFile,
        __in_z char    *lpszLspPathAndFile32,
        DWORD           dwCatalogIdArrayCount,
        DWORD          *pdwCatalogIdArray,
        BOOL            bIFSProvider,
        BOOL            bInstallOverAll
        )
{
	int nRet = 0;
	LspCreateHeap( &nRet );

	SetGUID(guidProvider);

	WINSOCK_CATALOG eCatalog;
	if( nCatalog == 0)
		eCatalog = LspCatalog32Only;
	else if(nCatalog == 1)
		eCatalog = LspCatalog64Only;
	else
		eCatalog = LspCatalogBoth;

	nRet = InstallLsp(
                eCatalog,
                lpszLspName,
                lpszLspPathAndFile,
                lpszLspPathAndFile32,
                dwCatalogIdArrayCount,
                pdwCatalogIdArray,
                bIFSProvider,
                bInstallOverAll
                );

	LspDestroyHeap();
	return nRet;
}


int RemoveProvider(GUID )
{
	/*
	pProtocolInfo = EnumerateProviders( Catalog, &iProtocolCount );
    if ( NULL == pProtocolInfo )
    {
        fprintf( stderr, "PrintProviders: Unable to enumerate catalog!\n" );
        goto cleanup;
    }

    for(i=0; i < iProtocolCount ;i++)*/

	return 0;
}




int WSPStartupInit(
				   WORD                wVersion,
				   LPWSPDATA           lpWSPData,
				   LPWSAPROTOCOL_INFOW lpProtocolInfo,
				   WSPUPCALLTABLE      UpCallTable,
				   LPWSPPROC_TABLE     lpProcTable,
				   GUID				   guid)
{
	EnterCriticalSection( &gCriticalSection );

	PROVIDER           *loadProvider = NULL;
	int                 Error = WSAEPROVIDERFAILEDINIT,rc;


	// The first time the startup is called, create our heap and allocate some
	//    data structures for tracking the LSP providers
	if ( 0 == gStartupCount )
	{
		SetGUID(guid);

		// Create the heap for all LSP allocations
		rc = LspCreateHeap( &Error );
		if ( SOCKET_ERROR == rc )
		{
			dbgprint("WSPStartup: LspCreateHeap failed: %d", Error );
			goto cleanup;
		}

		// Find this LSP's entries in the Winsock catalog and build a map of them
		rc = FindLspEntries( &gLayerInfo, &gLayerCount, &Error );
		if ( FALSE == rc )
		{
			dbgprint("WSPStartup: FindLspEntries failed: %d", Error );
			goto cleanup;
		}

		// Save off upcall table - this should be the same across all WSPStartup calls
		memcpy( &gMainUpCallTable, &UpCallTable, sizeof( gMainUpCallTable ) );

	}

	// Find the matching LSP provider for the requested protocol info passed in.
	//    This can either be an LSP layered over use or an entry belonging to this
	//    LSP. Note that the LSP startup gets called for each LSP layered protocol
	//    entry with a unique GUID. Because of this each layered protocol entry for
	//    the IFS LSP should be installed with its own unique GUID.
	loadProvider = FindMatchingLspEntryForProtocolInfo(
		lpProtocolInfo,
		gLayerInfo,
		gLayerCount,
		TRUE
		);
	if ( NULL == loadProvider )
	{
		dbgprint("WSPStartup: FindMatchingLspEntryForProtocolInfo failed!");
		ASSERT( 0 );
		goto cleanup;
	}

	// If this is the first time to "load" this particular provider, initialize
	//    the lower layer, etc.
	if ( 0 == loadProvider->StartupCount )
	{

		rc = InitializeProvider( loadProvider, wVersion, lpProtocolInfo, 
			UpCallTable, &Error );
		if ( SOCKET_ERROR == rc )
		{
			dbgprint("WSPStartup: InitializeProvider failed: %d", Error );
			goto cleanup;
		}

	}

	gStartupCount++;

	// Build the proc table to return to the caller
	memcpy( lpProcTable, &loadProvider->NextProcTable, sizeof( *lpProcTable ) );
	memcpy( lpWSPData, &loadProvider->WinsockVersion, sizeof( *lpWSPData ) );

	Error = NO_ERROR;
	OutputDebugStringW(L"SPI.DLL:: WSPStartup");
cleanup:

//	LspDestroyHeap();
	LeaveCriticalSection( &gCriticalSection );

	return Error;
}

void* FindSocketContextPtr(SOCKET s)
{
	SOCKET_CONTEXT *sockContext = NULL;

    //
    // Find our provider socket corresponding to this one
    //
    sockContext = FindSocketContext( s );

	return (void*)sockContext;
}

SOCKET WSPSocketInit( int                 af,
				int                 type,
				int                 protocol,
				LPWSAPROTOCOL_INFOW lpProtocolInfo,
				GROUP               g,
				DWORD               dwFlags,
				LPINT               lpErrno)
{
	WSAPROTOCOL_INFOW   InfoCopy = {0};
    SOCKET_CONTEXT     *sockContext = NULL;
    PROVIDER           *lowerProvider = NULL;
    SOCKET              nextProviderSocket = INVALID_SOCKET,
                        sret = INVALID_SOCKET;
    int                 rc;

    // Find the LSP entry which matches the given provider info
    lowerProvider = FindMatchingLspEntryForProtocolInfo(
            lpProtocolInfo,
            gLayerInfo,
            gLayerCount
            );
    if ( NULL == lowerProvider )
    {
        dbgprint("WSPSocket: FindMatchingLspEntryForProtocolInfo failed!" );
        goto cleanup;
    }

    if ( 0 == lowerProvider->StartupCount ) 
    {
        rc = InitializeProvider( lowerProvider, MAKEWORD(2,2), lpProtocolInfo, 
               gMainUpCallTable, lpErrno );
        if ( SOCKET_ERROR == rc )
        {
            dbgprint("WSPSocket: InitializeProvider failed: %d", *lpErrno);
            goto cleanup;
        }
    }

    // If the next layer is a base, substitute the provider structure with the
    //    base provider's
    if ( BASE_PROTOCOL == lowerProvider->NextProvider.ProtocolChain.ChainLen )
    {
        memcpy( &InfoCopy, &lowerProvider->NextProvider, sizeof( InfoCopy ) );
        InfoCopy.dwProviderReserved = lpProtocolInfo->dwProviderReserved;
        if (af == FROM_PROTOCOL_INFO)
            InfoCopy.iAddressFamily = lpProtocolInfo->iAddressFamily;
        if (type == FROM_PROTOCOL_INFO)
            InfoCopy.iSocketType = lpProtocolInfo->iSocketType;
        if (protocol == FROM_PROTOCOL_INFO)
            InfoCopy.iProtocol = lpProtocolInfo->iProtocol;
        
        lpProtocolInfo = &InfoCopy;
    }

    ASSERT( lowerProvider->NextProcTable.lpWSPSocket );

    //
    // Create the socket from the lower layer
    //
    nextProviderSocket = lowerProvider->NextProcTable.lpWSPSocket(
            af, 
            type, 
            protocol, 
            lpProtocolInfo,
            g, 
            dwFlags,
            lpErrno
            );
    if ( INVALID_SOCKET == nextProviderSocket )
    {
        dbgprint("WSPSocket: NextProcTable.WSPSocket failed: %d", *lpErrno);
        goto cleanup;
    }

    //
    // Create the context information to be associated with this socket
    //
    sockContext = CreateSocketContext(
            lowerProvider,
            nextProviderSocket,
            lpErrno
            );
    if ( NULL == sockContext )
    {
        dbgprint( "WSPSocket: CreateSocketContext failed: %d", *lpErrno );
        goto cleanup;
    }

    //
    // Associate ownership of this handle with our LSP
    //
    sret = gMainUpCallTable.lpWPUModifyIFSHandle(
            lowerProvider->LayerProvider.dwCatalogEntryId,
            nextProviderSocket,
            lpErrno
            );
    if ( INVALID_SOCKET == sret )
    {
        dbgprint( "WSPSocket: WPUModifyIFSHandle failed: %d", *lpErrno );
        goto cleanup;
    }

    ASSERT( sret == nextProviderSocket );

    return nextProviderSocket;

	
cleanup:

    // If an error occured close the socket if it was already created
    if ( ( NULL != sockContext ) && ( NULL != lowerProvider ) &&
         ( INVALID_SOCKET != nextProviderSocket ) )
    {
        rc = lowerProvider->NextProcTable.lpWSPCloseSocket(
                nextProviderSocket,
                lpErrno
                );
        if ( SOCKET_ERROR == rc )
        {
            dbgprint( "WSPSocket: WSPCloseSocket failed: %d", *lpErrno );
        }

    }
    if ( ( NULL != sockContext ) && ( NULL != lowerProvider ) )
        FreeSocketContext( lowerProvider, sockContext );

    return INVALID_SOCKET;
}

