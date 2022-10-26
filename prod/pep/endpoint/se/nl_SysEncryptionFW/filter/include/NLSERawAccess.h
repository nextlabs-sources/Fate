/*++

Module Name:

    NLSERawAccess.h

Abstract:
    This is the header file defining the functions used by 
    the kernel mode filter driver implementing NLSE raw mode file accesses.

Environment:
    Kernel mode

--*/
#ifndef __NLSE_RAW_ACCESS_H__
#define __NLSE_RAW_ACCESS_H__

#include "NLSEStruct.h"

//Create or open a file in raw mode
VOID NLSECreateFileRaw(__in const PNLSE_MESSAGE msg);

//Read a file in raw mode
VOID NLSEReadFileRaw(__in const PNLSE_MESSAGE msg);

//Write to a file in raw mode
VOID NLSEWriteFileRaw(__in const PNLSE_MESSAGE msg);

//Close a file opened in raw mode
VOID NLSECloseFileRaw(__in const PNLSE_MESSAGE msg);

#endif
