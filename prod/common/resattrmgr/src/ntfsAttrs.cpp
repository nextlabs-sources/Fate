#include "FileAttributeReaderWriter.h"
#include "StreamReaderWriter.h"
#include "ntfsAttrs.h"

// for CELog2
#include "NLCELog.h"
#define  CELOG_CUR_FILE static_cast<celog_filepathint_t>(EMNLFILEINTEGER_NTFSATTRSCPP)

//
// Internal variables
//

static CStreamReaderWriter g_StreamReaderWriter;



//
// Internal functions
//

static CStreamReaderWriter* GetStreamFunctions()
{
    InitializeNTFSStreamFunctions();
    return &g_StreamReaderWriter;
}



//
// Exported functions
//

void InitializeNTFSStreamFunctions(void)
{
    static BOOL initialized = FALSE;

    if (!initialized)
    {
        g_StreamReaderWriter.setCreateFileW (CreateFileW);
        g_StreamReaderWriter.setCloseHandle (CloseHandle);
        g_StreamReaderWriter.setReadFile    (ReadFile);
        g_StreamReaderWriter.setWriteFile   (WriteFile);
        initialized = TRUE;
    }
}

BOOL IsNTFSFile(LPCWSTR pszFileName)
{
    // Currently we return TRUE for all files, and let the stream functions in
    // xxxNTFSFileProps() return error on files on file systems that don't
    // support named streams.
    //
    // We can potentially change this to call GetVolumeInformation() and check
    // for either file system name being "NTFS" (which will support only NTFS)
    // or FILE_NAMED_STREAMS being set (which will also support other file
    // systems that support named streams.)  Logically it makes sense, but
    // this would have a small performance hit for files on NTFS which is the
    // case for most files, since this would be an extra check to make.
    return TRUE;
}

BOOL GetNTFSFileProps(const WCHAR *filename, ResourceAttributes *attrs)
{NLCELOG_ENTER
    PWCHAR *pStreamBuffer = NULL;
    DWORD streamAttrCount=0;
    DWORD count = 0;

    // Now look at the NTFS stream attributes
    pStreamBuffer = g_StreamReaderWriter.EnumStream (filename, &count);

    for (unsigned int i = 0; i < count; ++i)
    {
        if(GenericNextLabsTagging::TagExists(attrs, pStreamBuffer[i]))
        {
            continue;
        }

        EncodeSteamMgr obEncodeSteamMgr;
        BOOL bRet = g_StreamReaderWriter.ReadStream(filename, pStreamBuffer[i], obEncodeSteamMgr);
        if (bRet)
        {
            wchar_t* pwchBuffer = obEncodeSteamMgr.GetWideCharBuffer(false, true, NULL);
            if (NULL == pwchBuffer)
            {
                GenericNextLabsTagging::AddKeyValueHelperW(attrs, pStreamBuffer[i], L"");
            }
            else
            {
                // ReadStream terminated the string for me
                GenericNextLabsTagging::AddKeyValueHelperW(attrs, pStreamBuffer[i], pwchBuffer);
                obEncodeSteamMgr.FreeResource((void**)(&pwchBuffer));
            }
        }
    }

    g_StreamReaderWriter.ReleaseStreamNameBuffer (pStreamBuffer, streamAttrCount);

    return TRUE;
}

BOOL SetNTFSFileProps(const WCHAR *filename, ResourceAttributes *attrs)
{NLCELOG_ENTER
    CStreamReaderWriter *streamWriter = GetStreamFunctions();

    int size = GetAttributeCount(attrs);
    for (int i = 0; i < size; ++i)
    {
        const WCHAR *key = (WCHAR *)GetAttributeName(attrs, i);
        const WCHAR *value = (WCHAR *)GetAttributeValue(attrs, i);

        DWORD dwWriteSize = (DWORD)((wcslen(value) + 1) * sizeof(WCHAR));
        if (!streamWriter->WriteStreamWithWideChar(filename, key, value, &dwWriteSize))
        {
            // Oh well
            NLCELOG_RETURN_VAL( FALSE )
        }
    }

    NLCELOG_RETURN_VAL( TRUE )
}

BOOL RemoveNTFSFileProps(const WCHAR *filename, ResourceAttributes *attrs)
{NLCELOG_ENTER
        CStreamReaderWriter *streamRemove = GetStreamFunctions();
        if(!streamRemove)
                NLCELOG_RETURN_VAL(  FALSE )

        int size = GetAttributeCount(attrs);
        for (int i = 0; i < size; ++i)
        {
                const WCHAR *key = (WCHAR *)GetAttributeName(attrs, i);

                if(!streamRemove->RemoveStream(filename, key))
                        NLCELOG_RETURN_VAL(  FALSE )
        }

       NLCELOG_RETURN_VAL( TRUE )
}
