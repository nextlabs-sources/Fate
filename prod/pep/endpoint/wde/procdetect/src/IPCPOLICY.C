
#include <ntifs.h>
#include <ntddk.h>
#include <wdm.h>
#include "stdio.h"

#include "IPCPOLICY.H"

extern int gPoolType;

#define MAX_SID_LENGTH  64
#ifndef WINSID_IDENTIFIER_AUTHORITY_DEFINED
#define WINSID_IDENTIFIER_AUTHORITY_DEFINED
typedef struct _WINSID_IDENTIFIER_AUTHORITY
{
    UCHAR Value[6];
} WINSID_IDENTIFIER_AUTHORITY, *PWINSID_IDENTIFIER_AUTHORITY;
#endif

#ifndef WINSID_DEFINED
#define WINSID_DEFINED
typedef struct _WINSID
{
    UCHAR Revision;
    UCHAR SubAuthorityCount;
    WINSID_IDENTIFIER_AUTHORITY IdentifierAuthority;
    ULONG SubAuthority[1]; // Actually SubAuthorityCount entries
} WINSID, *PWINSID;
#endif

/************************************************************************/
/*      Routines Prototypes                                             */
/************************************************************************/
NTSTATUS
NTAPI
ZwOpenProcessToken(
                   IN HANDLE  ProcessHandle,
                   IN ACCESS_MASK  DesiredAccess,
                   OUT PHANDLE  TokenHandle
                   );

BOOLEAN
StringizeSid(
             IN PSID pSid,
             OUT PCHAR pszSid,
             IN OUT ULONG SidLength
             );

NTSTATUS
GetSID(
       IN HANDLE ProcessHandle,
       IN PVOID pReceiveSID, 
       IN TOKEN_INFORMATION_CLASS TokenInformationClass,
       IN OUT PULONG pulSIDLength, 
       OUT PULONG pulSIDAttribute
       );

NTSTATUS
GetProcessUserAccount(
                      HANDLE ProcessId,
                      PUNICODE_STRING UserAccount
                      );

/************************************************************************/
/*  Routines                                                            */
/************************************************************************/
BOOLEAN
StringizeSid(
             __in PSID pSid,
             __in_ecount_z(SidLength) PCHAR pszSid,
             __in ULONG SidLength
             )
{
    PWINSID                      pWinSid = (PWINSID)pSid;
    WINSID_IDENTIFIER_AUTHORITY *pia     = NULL;
    ULONG i, dwSACount;
    ULONG dwSidSize;

    RtlZeroMemory(pszSid,SidLength);
    if (!pSid) return FALSE;
    if (!RtlValidSid(pSid)) return FALSE;

    pia = &pWinSid->IdentifierAuthority;
    dwSACount = pWinSid->SubAuthorityCount;

    // Compute the buffer length.
    // S-SID_REVISION- + IdentifierAuthority- + sub authorities- + NULL
    dwSidSize = (15 + 12 + (12 * dwSACount) + 1) * sizeof(CHAR);
    if(SidLength<dwSidSize) return FALSE;

    _snprintf_s(pszSid, SidLength, _TRUNCATE, "S-%u-", pWinSid->Revision);

    // Add Sid identifier authority
    if (pia->Value[0] || pia->Value[1])
    {
        _snprintf_s(pszSid + strlen(pszSid), SidLength - strlen(pszSid), _TRUNCATE,
            "0x%02x%02x%02x%02x%02x%02x",
            pia->Value[0], pia->Value[1], pia->Value[2],
            pia->Value[3], pia->Value[4], pia->Value[5]);
    }
    else
    {
        _snprintf_s(pszSid + strlen(pszSid), SidLength - strlen(pszSid), _TRUNCATE, "%lu",
            (ULONG)(pia->Value[5] ) +
            (ULONG)(pia->Value[4] << 8) +
            (ULONG)(pia->Value[3] << 16) +
            (ULONG)(pia->Value[2] << 24));
    }

    // and the sub authorities
    for (i = 0; i < dwSACount; ++i)
    {
        _snprintf_s(pszSid + strlen(pszSid), SidLength - strlen(pszSid), _TRUNCATE, "-%lu", pWinSid->SubAuthority[i]);
    }

    return TRUE;
}

__checkReturn NTSTATUS
GetSID(
       __in HANDLE ProcessHandle,
       __in PVOID pReceiveSID, 
       __in TOKEN_INFORMATION_CLASS TokenInformationClass,
       __inout PULONG pulSIDLength, 
       __out_opt PULONG pulSIDAttribute
       )
{
    NTSTATUS             status;
    HANDLE               TokenHandle = NULL;
    WCHAR               *Buffer = NULL;
    TOKEN_USER          *tkUser;
    TOKEN_PRIMARY_GROUP *tkPrimaryGroup;
    TOKEN_OWNER         *tkOwner;
    ULONG                ReturnLength;
    PSID                 pSid   = NULL;
    CHAR                *pszSid = NULL;

    Buffer = ExAllocatePoolWithTag(gPoolType,512,PROCDETECT_POOL_TAG);
    if(!Buffer) 
    {
        KdPrint(("GetSID! Fail to allocate 512 buffer\n"));
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto GETSID_EXIT;
    }

    pszSid = ExAllocatePoolWithTag(gPoolType,128,PROCDETECT_POOL_TAG);
    if(!pszSid)
    {
        KdPrint(("GetSID! Fail to allocate 128 buffer\n"));
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto GETSID_EXIT;
    }

    status = ZwOpenProcessToken(ProcessHandle, TOKEN_READ,	&TokenHandle);
    if(NT_SUCCESS(status))
    {
        // Get token
        switch(TokenInformationClass)
        {
        case TokenUser:	
            status = ZwQueryInformationToken(TokenHandle, TokenInformationClass, Buffer, 512, &ReturnLength);
            if(!NT_SUCCESS(status))
            {
                KdPrint(("GetSID! Fail to ZwQueryInformationToken :: TokenUser\n"));
                goto GETSID_EXIT;
            }
            tkUser = (TOKEN_USER*)Buffer;
            pSid = tkUser->User.Sid;
            break;
        case TokenPrimaryGroup:	
            status = ZwQueryInformationToken(TokenHandle, TokenInformationClass, Buffer, 512, &ReturnLength);
            if(!NT_SUCCESS(status)) 
            {
                KdPrint(("GetSID! Fail to ZwQueryInformationToken :: TokenPrimaryGroup\n"));
                goto GETSID_EXIT;
            }
            tkPrimaryGroup = (TOKEN_PRIMARY_GROUP*)Buffer;
            pSid = tkPrimaryGroup->PrimaryGroup;
            break;
        case TokenOwner: 
            status = ZwQueryInformationToken(TokenHandle, TokenInformationClass, Buffer,  512, &ReturnLength);
            if(!NT_SUCCESS(status)) 
            {
                KdPrint(("GetSID! Fail to ZwQueryInformationToken :: TokenOwner\n"));
                goto GETSID_EXIT;
            }
            tkOwner = (TOKEN_OWNER*)Buffer;
            pSid = tkOwner->Owner; 
            break;
        default:
            KdPrint(("GetSID! Fail to ZwQueryInformationToken :: unknown TokenInformationClass\n"));
            status = STATUS_UNSUCCESSFUL;
            goto GETSID_EXIT;
        }

        if(!NT_SUCCESS(status))
        {
            KdPrint(("GetSID! Fail to NtOpenProcessToken\n"));
            status = STATUS_UNSUCCESSFUL;
            goto GETSID_EXIT;
        }

        if (RtlValidSid(pSid) == FALSE)
        {
            KdPrint(("GetSID! Not a valid SID\n"));
            status = STATUS_UNSUCCESSFUL;
            goto GETSID_EXIT;
        }	

        if ( RtlLengthSid(pSid) > MAX_SID_LENGTH)
        {
            KdPrint(("GetSID! SID length is too long\n"));
            status = STATUS_INSUFFICIENT_RESOURCES;
            goto GETSID_EXIT;
        }

        {
            // Convert pSid->wszSid
            ANSI_STRING     SID_A;
            UNICODE_STRING  SID_U;
            if(StringizeSid(pSid,pszSid,128))
            {
                // convert ANSI to UNICODE
                RtlInitAnsiString(&SID_A,pszSid);	
                if(RtlAnsiStringToUnicodeString(&SID_U,&SID_A,TRUE)==STATUS_SUCCESS)
                {
                    *pulSIDLength = min(SID_U.Length, *pulSIDLength);
                    RtlCopyMemory(pReceiveSID,SID_U.Buffer, *pulSIDLength);
                    RtlFreeUnicodeString(&SID_U);
                    status = STATUS_SUCCESS;
                }
                else
                {
                    KdPrint(("GetSid! Fail to RtlAnsiStringToUnicodeString!\n"));
                }
            }
            else
            {
                KdPrint(("GetSid! Fail to stringlize the sid!\n"));
            }
        }
    }
    else
    {
        KdPrint(("GetSid! Fail to open process token 0x%08x!", status));
    }

GETSID_EXIT:
	if (TokenHandle)
	{
		ZwClose(TokenHandle);
	}
	
    if(Buffer) ExFreePool(Buffer);
    if(pszSid) ExFreePool(pszSid);
    return status;
}

__checkReturn NTSTATUS
GetProcessUserAccount(
                      __in HANDLE ProcessId,
                      __out PUNICODE_STRING UserAccount
                      )
{
    NTSTATUS            Status;
    PACCESS_TOKEN       AccessToken;
    LUID                Luid;
    PSecurityUserData   UserInformation = NULL;
    PEPROCESS           PEProcess       = NULL;
#define SYSTEMACCOUNT_LOW       999
#define SYSTEMACCOUNT_HIGH      0
#define SYSTEMUSER_NAME         L"SYSTEM"
#define UNKNOWNUSER_NAME        L"UNKNOWN"

    // initialize the UserAccount
    UserAccount->Length = 14;
    RtlCopyMemory(UserAccount->Buffer, UNKNOWNUSER_NAME, 14);
    UserAccount->Buffer[7] = 0;

    // It is only valid under passive level
    if(KeGetCurrentIrql() != PASSIVE_LEVEL)
    {
        return STATUS_UNSUCCESSFUL;;
    }

    Status = PsLookupProcessByProcessId(ProcessId, (PEPROCESS*)&PEProcess);
    if(NT_SUCCESS(Status))
    {
        AccessToken = PsReferencePrimaryToken(PEProcess);
        Status = SeQueryAuthenticationIdToken(AccessToken, &Luid);

	ObDereferenceObject(PEProcess); /* decrement reference count for process handle */
	PsDereferencePrimaryToken(AccessToken);

        if(NT_SUCCESS(Status))
        {
            if(SYSTEMACCOUNT_HIGH==Luid.HighPart && SYSTEMACCOUNT_LOW==Luid.LowPart)
            {
                // It is a system user
                UserAccount->Length = 12;
                RtlCopyMemory(UserAccount->Buffer, SYSTEMUSER_NAME, 12);
                UserAccount->Buffer[6] = 0;
                Status = STATUS_SUCCESS;
            }
            else
            {
                Status = GetSecurityUserInfo(&Luid, UNDERSTANDS_LONG_NAMES, &UserInformation);
                if(NT_SUCCESS(Status) && UserInformation)
                {
                    RtlCopyUnicodeString(UserAccount, &UserInformation->UserName);
                    LsaFreeReturnBuffer(UserInformation);
                    Status = STATUS_SUCCESS;
                }
                else
                {
                    KdPrint(("GetProcessUserAccount! Fail to GetSecurityUserInfo, Status=0x%x\n", Status));
                }
            }
        }
        else
        {
            KdPrint(("GetProcessUserAccount! Fail to SeQueryAuthenticationIdToken, Status=0x%x\n", Status));
        }
    }
    else
    {
        KdPrint(("GetProcessUserAccount! Fail to PsLookupProcessByProcessId, Status=0x%x\n", Status));
    }

    KdPrint(("GetProcessUserAccount! UserAccount=%wZ\n", UserAccount));
    return Status;
}
