#ifndef _SPI_H__
#define _SPI_H__

#include "instlsp.h"
#include "lspdef.h"

int DisplayLsp(int nCatalog, bool bDetail);
int UninstallLsp(int nCatalog, GUID guidProvider);
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
        );
int WSPStartupInit(
				   WORD                wVersion,
				   LPWSPDATA           lpWSPData,
				   LPWSAPROTOCOL_INFOW lpProtocolInfo,
				   WSPUPCALLTABLE      UpCallTable,
				   LPWSPPROC_TABLE     lpProcTable,
				   GUID				   guid);
void* FindSocketContextPtr(SOCKET s);

SOCKET WSPSocketInit( int                 af,
				int                 type,
				int                 protocol,
				LPWSAPROTOCOL_INFOW lpProtocolInfo,
				GROUP               g,
				DWORD               dwFlags,
				LPINT               lpErrno);


void SPI_Init();
void SPI_Uninit();

#endif