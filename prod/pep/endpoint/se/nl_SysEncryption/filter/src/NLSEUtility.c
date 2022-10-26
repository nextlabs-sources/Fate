/*++

Module Name:

    NFSEUtility.c

Abstract:
  Utility Functions for system encryption in Kernel mode

Environment:

    Kernel mode

--*/
#include "NLSEStruct.h"
#include "NLSEUtility.h"

//Global variables
extern NL_KLOG nlseKLog;

DECLARE_CONST_UNICODE_STRING(wildcardStr, L"*");

//Free an encrypt extension (a.k.a. encryption footer)
void
NLFSEFreeEncryptExtension(__out_opt PNLFSE_ENCRYPT_EXTENSION pExt)
{
  if(pExt == NULL) return;
  RtlSecureZeroMemory(pExt, sizeof(NLFSE_ENCRYPT_EXTENSION));
  ExFreePoolWithTag(pExt, NLFSE_BUFFER_TAG);
}

//Get the current time
LARGE_INTEGER 
GetCurrentTime()
{
  LARGE_INTEGER Ret;
  KeQuerySystemTime(&Ret);
  return Ret;
}

#define ToUpperCase(x)  (( (x)>0x60 && (x)<0x7B )?((x)-0x20):(x))

INT
NLCompareName(
            __in PCWCH Name1,
            __in USHORT Length1,
            __in PCWCH Name2,
            __in USHORT Length2
            )
{
    USHORT i      = 0;
    USHORT Length = 0;

    Length1 = Length1 / sizeof(WCHAR);
    Length2 = Length2 / sizeof(WCHAR);
    Length  = min(Length1, Length2);

    for(i=0; i<Length; i++)
    {
        WCHAR N1 = ToUpperCase(Name1[i]);
        WCHAR N2 = ToUpperCase(Name2[i]);
        if(N1 > N2) return 1;
        if(N1 < N2) return -1;
    }

    if(Length1 > Length2) return 1;
    else if(Length1 < Length2) return -1;
    else return 0;
}

INT
NLCompareNameCaseSensitive(
            __in PCWCH Name1,
            __in USHORT Length1,
            __in PCWCH Name2,
            __in USHORT Length2
            )
{
    USHORT i      = 0;
    USHORT Length = 0;
    int ret;

    Length1 = Length1 / sizeof(WCHAR);
    Length2 = Length2 / sizeof(WCHAR);
    Length  = min(Length1, Length2);

    ret = wcsncmp(Name1, Name2, Length);
    if (ret != 0) return ret;

    if(Length1 > Length2) return 1;
    else if(Length1 < Length2) return -1;
    else return 0;
}

VOID
NLUpcaseString(
               __out PWCH OutString,
               __in PCWCH InString,
               __in USHORT Length
               )
{
  USHORT i;

  for (i = 0; i < Length / sizeof(WCHAR); i++)
  {
    OutString[i] = ToUpperCase(InString[i]);
  }
}

//Check if the access from remote client
//If yes, return TRUE
BOOLEAN NLSEIsAccessFromRemote(__in_opt PIO_SECURITY_CONTEXT secCtx)
{
  BOOLEAN bRet=FALSE;
  PACCESS_STATE pAS = NULL;
  PACCESS_TOKEN pT = NULL; 
  PTOKEN_GROUPS tokenGroupsPtr=NULL;
  NTSTATUS status;
  ULONG index=0;

  //Sanity checking on input
  if(secCtx == NULL) {
    return bRet;
  }

  pAS=secCtx->AccessState;
  if(pAS != NULL && 
     pAS->SubjectSecurityContext.ClientToken != NULL) {
    pT=SeQuerySubjectContextToken(&pAS->SubjectSecurityContext);
    status = SeQueryInformationToken(pT,
				     TokenGroups,
				     &tokenGroupsPtr);
    // Check if we've got token groups information.
    if (NT_SUCCESS(status)) {
      // Go through all SIDs to see if there's network pseudo group ID
      for (index = 0; index < tokenGroupsPtr->GroupCount; ++index) {
	if (RtlEqualSid(SeExports->SeNetworkSid, 
			tokenGroupsPtr->Groups[index].Sid)) {
	  NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_DEBUG,
		      "NLSE!IsAccessFromRemote: Yes\n");
	  bRet=TRUE;
	  break;
	}
      }
    }
    if (tokenGroupsPtr) {
      ExFreePool(tokenGroupsPtr);
    }
  }

  return bRet;
}

__checkReturn
NTSTATUS NLSEBuildDosPath(
    __in  PCUNICODE_STRING FullPath,
    __in  PCUNICODE_STRING VolumeName,
    __in  PCUNICODE_STRING VolumeDosName,
    __out PUNICODE_STRING DosPath)
{
  NTSTATUS            Status      = STATUS_SUCCESS;
  UNICODE_STRING      PathVolName = {0};
  UNICODE_STRING      PathDirName = {0};

  DosPath->Buffer = NULL;

  // Check path
  if(VolumeName->Length >= FullPath->Length)
  {
    Status = STATUS_INVALID_PARAMETER;
    goto _exit;
  }
  PathVolName.Buffer  = FullPath->Buffer;
  PathVolName.Length  = VolumeName->Length;
  PathVolName.MaximumLength  = PathVolName.Length;
  if(0 != RtlCompareUnicodeString(VolumeName, &PathVolName, TRUE))
  {
    Status = STATUS_INVALID_PARAMETER;
    goto _exit;
  }
  PathDirName.Buffer = (PWCH) (((PUCHAR)FullPath->Buffer) + VolumeName->Length);
  PathDirName.Length = FullPath->Length - VolumeName->Length;
  PathDirName.MaximumLength  = PathDirName.Length;

  // Build Dos Name
  DosPath->Length = VolumeDosName->Length + PathDirName.Length;
  DosPath->MaximumLength = DosPath->Length + sizeof(WCHAR);
  DosPath->Buffer = (PWCH)ExAllocatePoolWithTag(NonPagedPool,
      DosPath->MaximumLength, NLFSE_BUFFER_TAG);
  if(NULL == DosPath->Buffer)
  {
    Status = STATUS_INSUFFICIENT_RESOURCES;
    goto _exit;
  }
  RtlZeroMemory(DosPath->Buffer, DosPath->MaximumLength);

  RtlCopyUnicodeString(DosPath, VolumeDosName);
  RtlAppendUnicodeStringToString(DosPath, &PathDirName);

_exit:
  return Status;
}

/** getNextPathComponent
 *
 *  \brief Get the next component in the path, starting at the passed index.
 *
 *  \param path (in)        Path from which to extract the next component
 *  \param index (inout)    Pointer to index.  On entry, it contains the index
 *                          at which the next compoment is to be extracted.
 *  \param component (out)  String to store the next component, or empty string
 *                          if the next component is empty (e.g.
 *                          C:\Dir1\Dir2\\Dir4).  Caller needs to free the
 *                          memory allocated for this string.
 *
 *  \return TRUE on success (including the case where the next component is
 *                          empty), otherwise FALSE.
 */
__checkReturn
static BOOLEAN getNextPathComponent(__in    PCUNICODE_STRING    path,
                                    __inout PUSHORT             index,
                                    __out   PUNICODE_STRING     component)
{
  USHORT i;

  if (*index >= path->Length / sizeof(WCHAR))
  {
    // Invalid parameter error.
    return FALSE;
  }

  for (i = *index; i < path->Length / sizeof(WCHAR); i++)
  {
    if (path->Buffer[i] == L'\\')
    {
      break;
    }
  }

  // We have reached either a '\' or the end of the string.  Return the
  // (maybe empty) component.

  if (i == *index)
  {
    // Component is empty.  Return empty component.
    RtlInitUnicodeString(component, NULL);
    return TRUE;
  }

  // Return non-empty component.
  component->Buffer = ExAllocatePoolWithTag(PagedPool,
                                            (i - *index) * sizeof(WCHAR),
                                            NLSE_PATHNAME_TAG);
  if (component->Buffer == NULL)
  {
    NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_ERR, 
                "NLSE!" __FUNCTION__ ": failed to allocate component\n");
    return FALSE;
  }
  component->Length = (i - *index) * sizeof(WCHAR);
  component->MaximumLength = component->Length;
  RtlCopyMemory(component->Buffer, &path->Buffer[*index],
                (i - *index) * sizeof(WCHAR));
  *index = i;

  return TRUE;
}

__checkReturn
NTSTATUS NLSEIsPathMatchingWildcardPath(
    __in  PCUNICODE_STRING      path,
    __in  PCUNICODE_STRING      wildcardPath,
    __in  BOOLEAN               caseInSensitive,
    __out PBOOLEAN              result)
{
  USHORT index1 = 0, index2 = 0;

  while (1)
  {
    BOOLEAN endReached1, endReached2;
    UNICODE_STRING component1, component2;
    BOOLEAN componentMatch;

    endReached1 = (index1 >= path->Length / sizeof(WCHAR));
    endReached2 = (index2 >= wildcardPath->Length / sizeof(WCHAR));

    if (endReached1 && endReached2)
    {
      // Both paths have reached the end.  The paths match.
      *result = TRUE;
      return STATUS_SUCCESS;
    }
    else if (endReached1 || endReached2)
    {
      // One, but not both, path has reached the end.  The paths don't match.
      *result = FALSE;
      return STATUS_SUCCESS;
    }

    // Neither paths have reached the end.  See if the components between the
    // two paths match.

    if (!getNextPathComponent(path, &index1, &component1))
    {
      // Error.
      return STATUS_INSUFFICIENT_RESOURCES;
    }

    if (!getNextPathComponent(wildcardPath, &index2, &component2))
    {
      // Error.
      if (component1.Buffer != NULL)
      {
        ExFreePoolWithTag(component1.Buffer, NLSE_PATHNAME_TAG);
      }
      return STATUS_INSUFFICIENT_RESOURCES;
    }

    if (RtlEqualUnicodeString(&component1, &component2, caseInSensitive))
    {
      // The two components are exact match.
      componentMatch = TRUE;
    }
    else if (RtlPrefixUnicodeString(&wildcardStr, &component2,caseInSensitive))
    {
      if (component1.Buffer == NULL && component2.Buffer == NULL)
      {
        componentMatch = TRUE;
      }
      else if (component1.Buffer == NULL || component2.Buffer == NULL)
      {
        componentMatch = FALSE;
      }
      else
      {
        // The two components may be in the form "hello.txt", "*lo.txt".
        // Compare the post-wildcard suffix "lo.txt" and see if they match.
        USHORT suffixIndex1, suffixIndex2, suffixLen;
        UNICODE_STRING suffix1, suffix2;

        suffixIndex2 = wildcardStr.Length / sizeof(WCHAR);
        suffixLen = component2.Length / sizeof(WCHAR) - suffixIndex2;
        suffixIndex1 = component1.Length / sizeof(WCHAR) - suffixLen;
        suffix1.Buffer = &component1.Buffer[suffixIndex1];
        suffix2.Buffer = &component2.Buffer[suffixIndex2];
        suffix1.Length = suffix1.MaximumLength = suffixLen * sizeof(WCHAR);
        suffix2.Length = suffix2.MaximumLength = suffixLen * sizeof(WCHAR);
        componentMatch = RtlEqualUnicodeString(&suffix1, &suffix2,
                                               caseInSensitive);
      }
    }
    else
    {
      componentMatch = FALSE;
    }

    // Free the components if they are not empty.
    if (component2.Buffer != NULL)
    {
      ExFreePoolWithTag(component2.Buffer, NLSE_PATHNAME_TAG);
    }
    if (component1.Buffer != NULL)
    {
      ExFreePoolWithTag(component1.Buffer, NLSE_PATHNAME_TAG);
    }

    if (!componentMatch)
    {
      // This component doesn't match.  The paths don't match.
      *result = FALSE;
      return STATUS_SUCCESS;
    }

    // Skip the component separator (either a '\' or the wchar after the last
    // wchar in the paths).
    index1++;
    index2++;
  }
}



#ifdef NLSE_DEBUG_PERFORMANCE

VOID
PfStart(
        __out PNLPERFORMANCE_COUNTER ppfc
        )
{
    ppfc->start = KeQueryPerformanceCounter(&ppfc->freq);
    ppfc->freq.QuadPart = ppfc->freq.QuadPart/1000000; // frequency per microseconds
}

/*
It gets time diff in microseconds
*/
VOID
PfEnd(
      __out PNLPERFORMANCE_COUNTER ppfc
      )
{
    ppfc->end = KeQueryPerformanceCounter(NULL);
    // Diff time is in microseconds.
    if(0 != ppfc->freq.QuadPart)
        ppfc->diff.QuadPart = (ppfc->end.QuadPart - ppfc->start.QuadPart)/ppfc->freq.QuadPart;
    else
        ppfc->diff.QuadPart = ppfc->end.QuadPart - ppfc->start.QuadPart;
}

#endif
