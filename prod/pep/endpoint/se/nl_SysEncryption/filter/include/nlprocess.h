

#pragma once
#ifndef __NL_PROCESS_H__
#define __NL_PROCESS_H__



__drv_requiresIRQL(PASSIVE_LEVEL)
NTSTATUS
ProcessMgrCreate(
                 );

VOID
ProcessMgrClose(
                );

VOID
NLUpdateProcessInfo(
                    __in HANDLE ProcessId,
                    __in ULONG Trusted
                    );

ULONG
NLGetProcessInfo(
                 __in HANDLE ProcessId
                 );

#endif