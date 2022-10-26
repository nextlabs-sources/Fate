/*++

Module Name:

  NLSEStream.c

Abstract:
  Encryption Stream for system encryption in kernel mode

Environment:

    Kernel mode

--*/
#include "NLSEStruct.h"
#include "NLSEUtility.h"
#include "NLSEDrmpathList.h"

__checkReturn
NTSTATUS NLSEIsDirectoryEncryptedByDrmPathList( __in PCFLT_RELATED_OBJECTS FltObjects,
				   __in PCUNICODE_STRING in_path ,
				   __in BOOLEAN* out_result )
{
  return NLSEDrmPathListCheck(in_path, out_result);
}/* NLSEIsDirectoryEncryptedByDrmPathList */

__checkReturn
NTSTATUS NLSEIsDirectoryEncrypted( __in PCFLT_RELATED_OBJECTS FltObjects,
				   __in PUNICODE_STRING in_path ,
				   __in BOOLEAN* out_result )
{
  return NLSEIsDirectoryEncryptedByDrmPathList(FltObjects, in_path, out_result);
}/* NLSEIsDirectoryEncrypted */

__checkReturn
NTSTATUS NLSEIsPathEncrypted( __in PCFLT_RELATED_OBJECTS FltObjects,
			      __in PUNICODE_STRING in_path ,
			      __out BOOLEAN* out_result )
{
  UNICODE_STRING path;
  PWCHAR p = NULL;
  LONG i = 0;
  NTSTATUS status = STATUS_SUCCESS;

  ASSERT( in_path != NULL );
  ASSERT( out_result != NULL );

  RtlCopyMemory(&path,in_path,sizeof(path));
  path.Buffer = (PWSTR)ExAllocatePoolWithTag(NonPagedPool,in_path->MaximumLength,NLSE_CHECKDIRENCATTR_TAG);
  if( path.Buffer == NULL )
  {
    return STATUS_INSUFFICIENT_RESOURCES;
  }
  RtlCopyMemory(path.Buffer,in_path->Buffer,in_path->Length);

  *out_result = FALSE; /* Default is not an encrypted path */
  if (path.Length == 0)
  {
    goto _exit;
  }
  for( ; ; )
  {
    BOOLEAN curr_enc = FALSE;
    /* Traverse the string for a slash and truncate including the slash.  The UNICODE_STRING
     * must also be updated after truncation.
     */

    p = path.Buffer + (path.Length / sizeof(WCHAR)) - 1;
    while( p != path.Buffer )
    {
      if( *p == L'\\' )
      {
	*p = (WCHAR)NULL;
	break;
      }
      p--;
    }/* while */
    path.Length = (p - path.Buffer) * sizeof(WCHAR);

    /* Are we are root? */
    if( p == path.Buffer) // || path.Length <= sizeof(L"\\??\\c:"))
    {
      break;
    }

    /* If the current directory is encrypted, then stop the search.  Otherwise walk to
     * the root to determine if the current path is encrypted.
     */
    status = NLSEIsDirectoryEncrypted(FltObjects,&path,&curr_enc);
    if( !NT_SUCCESS(status) )
    {
      break;
    }
    if( curr_enc == TRUE )
    {
      *out_result = TRUE;
      break;
    }

  }/* for */

_exit:
  ExFreePool(path.Buffer);

  return status;
}/* NLSEIsPathEncrypted */
