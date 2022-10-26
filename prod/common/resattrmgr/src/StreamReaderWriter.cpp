#ifndef UNICODE
#define UNICODE 1
#endif
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#include "StreamReaderWriter.h"

// for CELog2
#include "NLCELog.h"
#define  CELOG_CUR_FILE static_cast<celog_filepathint_t>(EMNLFILEINTEGER_STREAMREADERWRITERCPP)

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// HEAD
//
// return code type
//
typedef INT NXT_NTSTATUS;

//
// Check for success
//
#define NT_SUCCESS(Status) ((NXT_NTSTATUS)(Status) >= 0)

//
// The NT return codes we care about
//
#define STATUS_BUFFER_OVERFLOW           ((NXT_NTSTATUS)0x80000005L)

//--------------------------------------------------------------------
//     N T F S C O N T R O L F I L E   D E F I N I T I O N S
//--------------------------------------------------------------------

//
// Prototype for NtFsControlFile and data structures
// used in its definition
//

//
// Io Status block (see NTDDK.H)
//
typedef struct _IO_STATUS_BLOCK {
    NXT_NTSTATUS Status;
    ULONG Information;
} IO_STATUS_BLOCK, *PIO_STATUS_BLOCK;


//
// Apc Routine (see NTDDK.H)
//
typedef VOID (*PIO_APC_ROUTINE) (
                                 PVOID ApcContext,
                                 PIO_STATUS_BLOCK IoStatusBlock,
                                 ULONG Reserved
                                 );

//
// File information classes (see NTDDK.H)
//
typedef enum _FILE_INFORMATION_CLASS {
    // end_wdm
    FileDirectoryInformation       = 1,
    FileFullDirectoryInformation, // 2
    FileBothDirectoryInformation, // 3
    FileBasicInformation,         // 4  wdm
    FileStandardInformation,      // 5  wdm
    FileInternalInformation,      // 6
    FileEaInformation,            // 7
    FileAccessInformation,        // 8
    FileNameInformation,          // 9
    FileRenameInformation,        // 10
    FileLinkInformation,          // 11
    FileNamesInformation,         // 12
    FileDispositionInformation,   // 13
    FilePositionInformation,      // 14 wdm
    FileFullEaInformation,        // 15
    FileModeInformation,          // 16
    FileAlignmentInformation,     // 17
    FileAllInformation,           // 18
    FileAllocationInformation,    // 19
    FileEndOfFileInformation,     // 20 wdm
    FileAlternateNameInformation, // 21
    FileStreamInformation,        // 22
    FilePipeInformation,          // 23
    FilePipeLocalInformation,     // 24
    FilePipeRemoteInformation,    // 25
    FileMailslotQueryInformation, // 26
    FileMailslotSetInformation,   // 27
    FileCompressionInformation,   // 28
    FileObjectIdInformation,      // 29
    FileCompletionInformation,    // 30
    FileMoveClusterInformation,   // 31
    FileQuotaInformation,         // 32
    FileReparsePointInformation,  // 33
    FileNetworkOpenInformation,   // 34
    FileAttributeTagInformation,  // 35
    FileTrackingInformation,      // 36
    FileMaximumInformation
    // begin_wdm
} FILE_INFORMATION_CLASS, *PFILE_INFORMATION_CLASS;


//
// Streams information
//
#pragma pack(4)
typedef struct {
    ULONG    	        NextEntry;
    ULONG    	        NameLength;
    LARGE_INTEGER    	Size;
    LARGE_INTEGER    	AllocationSize;
    USHORT    	        Name[1];
} FILE_STREAM_INFORMATION, *PFILE_STREAM_INFORMATION;
#pragma pack()

typedef NXT_NTSTATUS (__stdcall *PNtQueryInformationFile)(
    IN HANDLE FileHandle,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    OUT PVOID FileInformation,
    IN ULONG Length,
    IN FILE_INFORMATION_CLASS FileInformationClass
    );

typedef NXT_NTSTATUS (__stdcall *PNtSetInformationFile)(
    IN HANDLE FileHandle,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN PVOID FileInformation,
    IN ULONG Length,
    IN FILE_INFORMATION_CLASS FileInformationClass
    );
static BOOL                    bEnableTokenPrivilege    = FALSE;

static PNtQueryInformationFile NtQueryInformationFile;
static PNtSetInformationFile   NtSetInformationFile;

BOOL EnableTokenPrivilege( PTCHAR PrivilegeName );
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//
//----------------------------------------------------------------------
//
// EnableTokenPrivilege
//
// Enables the load driver privilege
//
//----------------------------------------------------------------------
BOOL EnableTokenPrivilege( PTCHAR PrivilegeName )
{
    TOKEN_PRIVILEGES tp;
    LUID luid;
    HANDLE	hToken;
    TOKEN_PRIVILEGES tpPrevious;
    DWORD cbPrevious=sizeof(TOKEN_PRIVILEGES);

    //
    // Get debug privilege
    //
    if(!OpenProcessToken( GetCurrentProcess(),
        TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
        &hToken )) {

            return FALSE;
        }

        if(!LookupPrivilegeValue( NULL, PrivilegeName, &luid )) return FALSE;

        //
        // first pass.  get current privilege setting
        //
        tp.PrivilegeCount           = 1;
        tp.Privileges[0].Luid       = luid;
        tp.Privileges[0].Attributes = 0;

        AdjustTokenPrivileges(
            hToken,
            FALSE,
            &tp,
            sizeof(TOKEN_PRIVILEGES),
            &tpPrevious,
            &cbPrevious
            );

        if (GetLastError() != ERROR_SUCCESS) return FALSE;

        //
        // second pass.  set privilege based on previous setting
        //
        tpPrevious.PrivilegeCount       = 1;
        tpPrevious.Privileges[0].Luid   = luid;
        tpPrevious.Privileges[0].Attributes |= (SE_PRIVILEGE_ENABLED);
        AdjustTokenPrivileges(
            hToken,
            FALSE,
            &tpPrevious,
            cbPrevious,
            NULL,
            NULL
            );

        return GetLastError() == ERROR_SUCCESS;
}

CStreamReaderWriter::CStreamReaderWriter()
{
    m_pCreateFileW  = NULL;
    m_pCloseHandle  = NULL;
    m_pReadFile     = NULL;
    m_pWriteFile    = NULL;
}

BOOL CStreamReaderWriter::CheckKernelFunctionsValid()
{
    if(NULL == NtQueryInformationFile)
    {
		HMODULE hModule = GetModuleHandle(L"ntdll.dll");
		if(hModule != NULL)
		{
			NtQueryInformationFile = (PNtQueryInformationFile) GetProcAddress( hModule, "NtQueryInformationFile" );
		}
		if(!NtQueryInformationFile)
    {
#ifdef _DEBUG
        wprintf(L"\nCould not find NtQueryInformationFile entry point in NTDLL.DLL\n");
#endif
        return FALSE;
    }
    }
    if(NULL == NtSetInformationFile)
    {
		HMODULE hModule = GetModuleHandle(L"ntdll.dll");
		if(hModule != NULL)
		{
			NtSetInformationFile = (PNtSetInformationFile) GetProcAddress( hModule, "NtSetInformationFile" );
		}
		if(!NtSetInformationFile)
    {
#ifdef _DEBUG
        wprintf(L"\nCould not find NtSetInformationFile entry point in NTDLL.DLL\n");
#endif
        return FALSE;
    }
    }

    if(!bEnableTokenPrivilege)
        bEnableTokenPrivilege = EnableTokenPrivilege(SE_BACKUP_NAME);

    return TRUE;
}

BOOL CStreamReaderWriter::CheckFunctionsValid()
{
	HMODULE hModule = GetModuleHandle(L"Kernel32.dll");
	if(hModule == NULL)
	{
        return FALSE;
	}

    if(NULL==m_pCreateFileW)
	{
		m_pCreateFileW=(PCreateFileW)GetProcAddress( hModule, "CreateFileW" );
		if(!m_pCreateFileW)
		{
        return FALSE;
		}
	}
    if(NULL==m_pCloseHandle)
	{
		m_pCloseHandle=(PCloseHandle)GetProcAddress( hModule, "CloseHandle" );
		if(!m_pCloseHandle)
		{
        return FALSE;
		}
	}
    if(NULL==m_pReadFile)
	{
		m_pReadFile=(PReadFile)GetProcAddress( hModule, "ReadFile" );
		if(!m_pReadFile)
		{
        return FALSE;
		}
	}
    if(NULL==m_pWriteFile)
	{
		m_pWriteFile=(PWriteFile)GetProcAddress( hModule, "WriteFile" );
		if(!m_pWriteFile)
		{
	        return FALSE;
		}
	}

    return TRUE;
}

PWCHAR* CStreamReaderWriter::EnumStream(IN const WCHAR* FileName, IN OUT DWORD* pdwStreamNameCount)
{
    PFILE_STREAM_INFORMATION  streamInfo, streamInfoPtr;
    ULONG    streamInfoSize = 0;
    NXT_NTSTATUS status;
    HANDLE   fileHandle;
    IO_STATUS_BLOCK ioStatus;
	ioStatus.Information = 0;
    WCHAR*  pTruncat=0;

    PWCHAR* StreamNameBuffer = 0;
    *pdwStreamNameCount = 0;
    PWCHAR tmpBuffer[1024]; memset(tmpBuffer, 0, sizeof(tmpBuffer));

    // make sure we have inited the data
    if(!CheckKernelFunctionsValid() || !CheckFunctionsValid())
    {
        return StreamNameBuffer;
    }

    //
    // Open the file
    //
    fileHandle = m_pCreateFileW( FileName, GENERIC_READ,
        FILE_SHARE_READ|FILE_SHARE_WRITE, NULL,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS, 0 );
    if( fileHandle == INVALID_HANDLE_VALUE )
    {
        return StreamNameBuffer;
    }

    streamInfoSize = 16384;
    streamInfo = (PFILE_STREAM_INFORMATION)malloc( streamInfoSize );
    // Need to initialize it or the string compare below will overrun
    memset (streamInfo, 0x0, streamInfoSize);
    status = STATUS_BUFFER_OVERFLOW;
    while( status == STATUS_BUFFER_OVERFLOW )
    {
        status = NtQueryInformationFile( fileHandle, &ioStatus,
            streamInfo, streamInfoSize,
            FileStreamInformation );
        if( status == STATUS_BUFFER_OVERFLOW )
        {
            free( streamInfo );
            streamInfoSize += 16384;
            streamInfo = (PFILE_STREAM_INFORMATION)malloc( streamInfoSize );
        }
        else
        {
            break;
        }
    }
#ifdef _WIN64
	if( NT_SUCCESS( status ) && streamInfo && streamInfo->NameLength != 0)
#else
    if( NT_SUCCESS( status ) && ioStatus.Information )
#endif
    {
        streamInfoPtr = streamInfo;
        for(;;)
        {
	    if(0 != wcscmp((WCHAR*)streamInfoPtr->Name, L"::$DATA"))
            {
                tmpBuffer[(*pdwStreamNameCount)] = new WCHAR[(streamInfoPtr->NameLength/2)+1];
                memset(tmpBuffer[(*pdwStreamNameCount)], 0, sizeof(tmpBuffer[(*pdwStreamNameCount)]));
                if(L':' == streamInfoPtr->Name[0])
                    memcpy( tmpBuffer[(*pdwStreamNameCount)], streamInfoPtr->Name+1, streamInfoPtr->NameLength-2 );
                else
                    memcpy( tmpBuffer[(*pdwStreamNameCount)], streamInfoPtr->Name, streamInfoPtr->NameLength );
                pTruncat = wcsstr(tmpBuffer[(*pdwStreamNameCount)], L":$DATA");
                if(pTruncat) *pTruncat = 0;
                (*pdwStreamNameCount)++;
            }


            if( !streamInfoPtr->NextEntry ) break;
            streamInfoPtr = (PFILE_STREAM_INFORMATION) ((char *) streamInfoPtr + streamInfoPtr->NextEntry );
        }

        if(0 < *pdwStreamNameCount)
        {
            StreamNameBuffer = new PWCHAR[(*pdwStreamNameCount)];
            memcpy(StreamNameBuffer, tmpBuffer, (*pdwStreamNameCount)*sizeof(WCHAR*));
        }
    }
#ifdef _DEBUG
    else
    {
        wprintf(L"[ReadStream] Fail to NtQueryInformationFile");
    }
#endif

    free( streamInfo );
    m_pCloseHandle( fileHandle );

    return StreamNameBuffer;
}

void CStreamReaderWriter::ReleaseStreamNameBuffer(IN PWCHAR* StreamNameBuffer, IN DWORD dwStreamNameCount)
{
    if( IsBadReadPtr(StreamNameBuffer, dwStreamNameCount*sizeof(PWCHAR)) )
        return;

    int i=0;
    for(i=0; i<(int)dwStreamNameCount; i++)
    {
        if(0 != StreamNameBuffer[i]) delete [](StreamNameBuffer[i]);
    }
    delete []StreamNameBuffer;
}

HANDLE CStreamReaderWriter::CreateStream(IN const WCHAR* FileName, IN const WCHAR* StreamName, IN BOOL ReadOnly, IN BOOL CreateNew)
{
	size_t size = wcslen(FileName) + wcslen(StreamName) + 2;
    WCHAR *fullStreamName = new WCHAR[size];
    wcsncpy_s(fullStreamName, size, FileName, _TRUNCATE);
    wcsncat_s(fullStreamName, size, L":", _TRUNCATE);
    wcsncat_s(fullStreamName, size, StreamName, _TRUNCATE);

    if(!CheckFunctionsValid())
    {
        delete[] fullStreamName;
        return INVALID_HANDLE_VALUE;
    }

    HANDLE ret = m_pCreateFileW(fullStreamName,
                                ReadOnly?GENERIC_READ:(GENERIC_READ|GENERIC_WRITE),
                                ReadOnly?FILE_SHARE_READ:(FILE_SHARE_READ|FILE_SHARE_WRITE),
                                NULL,
                                CreateNew?CREATE_ALWAYS:OPEN_EXISTING,
                                0, NULL);

    delete[] fullStreamName;
    return ret;
}

BOOL CStreamReaderWriter::WriteStreamWithWideChar(IN const WCHAR* FileName, IN const WCHAR* StreamName, IN const WCHAR* StreamData, OUT DWORD* pdwWriteSize)
{
    BOOL bRet = FALSE;

    EMEncodeType emDefaultEncodeType = EncodeBaseInfoMgr::GetInstance().GetDefaultEncodingType();
    if ((emEncode_Unicode == emDefaultEncodeType) || (emEncode_Unicode_BigEnd == emDefaultEncodeType))
    {
        bRet = InnerWriteStream(FileName, StreamName, StreamData, pdwWriteSize, NULL, 0);
    }
    else
    {
        EncodeSteamMgr obEncodeSteamMgr;
        obEncodeSteamMgr.SetEncodeStream((byte*)StreamData, *pdwWriteSize, emEncode_Unicode);

        size_t stByteLength = 0;
        byte* pByteStream = obEncodeSteamMgr.GetSpeicifyEncodeByteStream(emDefaultEncodeType, false, false, &stByteLength);
        bRet = InnerWriteStream(FileName, StreamName, pByteStream, (DWORD*)&stByteLength, NULL, 0);
        *pdwWriteSize = stByteLength;

        obEncodeSteamMgr.FreeResource((void**)(&pByteStream));
    }
    return bRet;
}

BOOL CStreamReaderWriter::InnerWriteStream(IN const WCHAR* FileName, IN const WCHAR* StreamName, IN const void* StreamData, IN OUT DWORD* pdwWriteSize, _In_opt_ const byte* pByteBomHeader, _In_ int nBomHeaderLength)
{NLCELOG_ENTER  
    HANDLE handleStream = CreateStream(FileName, StreamName, FALSE, TRUE);
    if(INVALID_HANDLE_VALUE == handleStream)
    {
        NLCELOG_RETURN_VAL( FALSE )
    }

    DWORD   dwWritedBOM    = 0;
    DWORD   dwWritedStream = 0;

    BOOL bRet = TRUE;
    if (NULL != pByteBomHeader)
    {
        bRet = m_pWriteFile(handleStream, pByteBomHeader, nBomHeaderLength, &dwWritedBOM, NULL);
    }

    if (bRet)
    {
        bRet = m_pWriteFile(handleStream, StreamData, *pdwWriteSize, &dwWritedStream, NULL);
    }
    *pdwWriteSize = dwWritedBOM + dwWritedStream;
    m_pCloseHandle(handleStream);

    NLCELOG_RETURN_VAL( bRet )
}

BOOL CStreamReaderWriter::ReadStream(IN const WCHAR* FileName, IN const WCHAR* StreamName, _Inout_ EncodeSteamMgr& obEncodeSteamMgrInout)
{
    DWORD dwRead = 0;
    // Pass in NULL to get the length of the stream (value)
    InnerReadStream(FileName, StreamName, NULL, &dwRead);

    // String terminator
    dwRead += 2;
    char *pchValue = new char[dwRead];
    memset(pchValue, '\0', dwRead);

    BOOL bRet = InnerReadStream(FileName, StreamName, pchValue, &dwRead);
    if (bRet)
    {
        obEncodeSteamMgrInout.SetEncodeStream((byte*)pchValue, dwRead, emEncode_Unknown);
    }

    delete[] pchValue;
    pchValue = NULL;

    return bRet;
}

BOOL CStreamReaderWriter::InnerReadStream(IN const WCHAR* FileName, IN const WCHAR* StreamName, IN void* StreamData, IN OUT DWORD* pdwReadSize)
{NLCELOG_ENTER
    HANDLE  handleStream= INVALID_HANDLE_VALUE;
    BOOL    bRet        = FALSE;
    DWORD   dwToRead    = *pdwReadSize - 2;   // 2 bytes for string termination
    DWORD   dwRead      = 0;

    if (StreamData == NULL)
    {
        // NULL pointer?  Return the length of the stream
        size_t size = wcslen(FileName) + wcslen(StreamName) + 2;
        WCHAR *fullStreamName = new WCHAR[size];
        wcsncpy_s(fullStreamName, size, FileName, _TRUNCATE);
        wcsncat_s(fullStreamName, size, L":", _TRUNCATE);
        wcsncat_s(fullStreamName, size, StreamName, _TRUNCATE);

        WIN32_FILE_ATTRIBUTE_DATA attrData;
        if (GetFileAttributesEx(fullStreamName, GetFileExInfoStandard, &attrData) == 0) {
            *pdwReadSize = 128;
        } else {
            // Attribute values can't be 4GB in length, so we don't need to check the high word
            *pdwReadSize = attrData.nFileSizeLow;
        }

        delete [] fullStreamName;
        NLCELOG_RETURN_VAL( TRUE )
    }

    char   *pStreamData = (char*)StreamData;
    memset(StreamData, 0, *pdwReadSize);

    handleStream = CreateStream(FileName, StreamName, TRUE, FALSE);
    if(INVALID_HANDLE_VALUE == handleStream)
    {
        NLCELOG_RETURN_VAL( FALSE )
    }

    bRet = m_pReadFile(handleStream, (void *)StreamData, dwToRead, &dwRead, NULL);
    if(bRet && dwRead>0)
    {
        pStreamData[dwRead]  = 0;
        pStreamData[dwRead+1]= 0;
    }

    *pdwReadSize = dwRead;
    m_pCloseHandle(handleStream);
    NLCELOG_RETURN_VAL( bRet )
}

BOOL CStreamReaderWriter::RemoveStream(IN const WCHAR* FileName, IN const WCHAR* StreamName)
{NLCELOG_ENTER
    if(!FileName || !StreamName)
        NLCELOG_RETURN_VAL( FALSE )

		size_t size = wcslen(FileName) + wcslen(StreamName) + 2;
    WCHAR *fullStreamName = new WCHAR[size ];

    wcsncpy_s(fullStreamName, size, FileName, _TRUNCATE);
    wcsncat_s(fullStreamName, size, L":", _TRUNCATE);
    wcsncat_s(fullStreamName, size, StreamName, _TRUNCATE);

    BOOL res = DeleteFile(fullStreamName);
    delete[] fullStreamName;

    NLCELOG_RETURN_VAL( res )
}
