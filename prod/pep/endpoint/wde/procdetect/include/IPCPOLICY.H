
#pragma once
#ifndef PD_IPCPOLICY_H
#define PD_IPCPOLICY_H
#include <ntifs.h>

#define MAX_ACTION_NAME_SIZE	32

#define MAX_NAME_LENGTH		    512
#define MAX_SID_LENGTH		    64
#define MAX_PATH_LENGTH		    512
#define MAX_DRIVE_LETTERS	    26

#define PROCDETECT_POOL_TAG     'dpln'

/* Prototype for PsDereferencePrimaryToken which cannot be imported at runtime
   by Win2K since the kernel does not export it.
*/
typedef PACCESS_TOKEN (*PsDereferencePrimaryToken_fn_t)(PEPROCESS);

extern
NTSTATUS
GetSID(
       IN HANDLE ProcessHandle,
       IN PVOID pReceiveSID, 
       IN TOKEN_INFORMATION_CLASS TokenInformationClass,
       IN OUT PULONG pulSIDLength, 
       OUT PULONG pulSIDAttribute
       );

extern
NTSTATUS
GetProcessUserAccount(
                      HANDLE ProcessId,
                      PUNICODE_STRING UserAccount
                      );

#endif
