#define WINVER _WIN32_WINNT_WINXP
#pragma warning( push )
#pragma warning( disable : 4005 )
#define _WIN32_WINNT _WIN32_WINNT_WINXP
#pragma warning( pop )
#define NTDDI_VERSION NTDDI_WINXPSP2

#include <windows.h>

#include "AltDataStream.h"
#include "celog.h"



#define CELOG_CUR_MODULE L"NLSELib"
#define CELOG_CUR_FILE \
  CELOG_FILEPATH_PROD_PEP_ENDPOINT_SE_NL_SYSENC_LIB_SRC_ALTDATASTREAM_CPP



/** NxtFindFirstStream
 *
 *  \brief Get the first NTFS Alternate Data Stream of the file.
 *
 *  \param lpFileName (in)			Path to file.
 *  \param lpFindStreamData (out)	The Alternate Data Stream information structure.
 *  \param lpContext (out)			Pointer to a variable that pointer to an internal data structure.
 *
 *  \return INVALID_HANDLE_VALUE on failure. It needs to be released by calling CloseHandle() at last.
 */
extern "C"
HANDLE NxtFindFirstStream(
    _In_	LPCWSTR lpFileName,
    _Out_	LPVOID lpFindStreamData,
	_Out_	LPVOID *lpContext
	)
{
	CELOG_ENTER;
	HANDLE hFindStream = INVALID_HANDLE_VALUE;
	PNXT_FIND_STREAM_DATA pFindStreamData = (PNXT_FIND_STREAM_DATA)lpFindStreamData;

	if (lpFileName == NULL || lpFindStreamData == NULL || lpContext == NULL)
		CELOG_RETURN_VAL(hFindStream);

	//ZeroMemory(pFindStreamData, sizeof(NXT_FIND_STREAM_DATA));

	hFindStream = CreateFile( lpFileName, 
						GENERIC_READ, 
						FILE_SHARE_READ, 
						NULL, 
						OPEN_EXISTING, 
						FILE_FLAG_BACKUP_SEMANTICS, 
						NULL );

	if(hFindStream == INVALID_HANDLE_VALUE)
	{
		CELOG_RETURN_VAL(hFindStream);
	}

	//BYTE				baData[sizeof(WIN32_STREAM_ID) + sizeof(TCHAR)];
	WIN32_STREAM_ID		StrmId;
	DWORD				dwBytesToRead;
	DWORD				dwBytesRead;
	BOOL				bResult = TRUE;
	LARGE_INTEGER		liDataSize;
	DWORD				dwNameSize = 0;	
	DWORD				dwLowByteSeeked;
	DWORD				dwHighByteSeeked;

	*lpContext = NULL;

	while (dwNameSize == 0)
	{
		dwBytesToRead = FIELD_OFFSET(WIN32_STREAM_ID, cStreamName);
		ZeroMemory((void*)&StrmId, sizeof(StrmId));

		// read the header part
		bResult = BackupRead( hFindStream, 
						(LPBYTE)&StrmId,
						dwBytesToRead,
						&dwBytesRead,
						FALSE, // am I done?
						FALSE,
						lpContext );
		if (dwBytesRead == 0)
		{
			bResult = FALSE;
			break;
		}
		else
		{
			// saving the header info
			//pStrmId = (LPWIN32_STREAM_ID)baData;
			dwNameSize = StrmId.dwStreamNameSize;

			liDataSize = StrmId.Size;
			
			pFindStreamData->StreamSize = StrmId.Size;


			if(dwNameSize > 0)
			{
				//CopyMemory(pFindStreamData->cStreamName, pStrmId->cStreamName, 6);

				dwBytesToRead = dwNameSize;
				LPBYTE lpBuffer = new BYTE[dwBytesToRead];
				
				if(!BackupRead(	hFindStream, 
								lpBuffer,
								dwBytesToRead,
								&dwBytesRead,
								FALSE, // am I done?
								FALSE,
								lpContext ) )
				{
					bResult = FALSE;
				}			
				
				if (dwBytesToRead > sizeof(WCHAR)*(MAX_PATH + 32))
					dwBytesToRead = sizeof(WCHAR)*(MAX_PATH + 32);
				CopyMemory((pFindStreamData->cStreamName), lpBuffer, dwBytesToRead);
				delete[] lpBuffer;
			}
			pFindStreamData->cStreamName[dwNameSize / sizeof(WCHAR)] = L'\0';

			// Skip the remain stream data
			if(liDataSize.LowPart != 0 || liDataSize.HighPart != 0)
			{
				dwLowByteSeeked = 0;
				dwHighByteSeeked = 0;

				BackupSeek(hFindStream, 
						 liDataSize.LowPart,
						 liDataSize.HighPart,
						 &dwLowByteSeeked,
						 &dwHighByteSeeked,
						 lpContext);
			}
		}
	}

	if (!bResult)
	{
		CloseHandle(hFindStream);
		hFindStream = INVALID_HANDLE_VALUE;
		*lpContext = NULL;
	}

	CELOG_RETURN_VAL(hFindStream);
}

/** NxtFindNextStream
 *
 *  \brief Get the next NTFS Alternate Data Stream of the file.
 *
 *  \param lpFileName (in)			Path to file.
 *  \param lpFindStreamData (out)	The Alternate Data Stream information structure.
 *  \param lpContext (out)			Pointer to a variable that pointer to an internal data structure.
 *
 *  \return Non-zero on success, otherwise return 0.
 */
BOOL NxtFindNextStream(
    _In_	HANDLE hFindStream,
    _Out_	LPVOID lpFindStreamData,
	_Inout_	LPVOID *lpContext
	)
{
	CELOG_ENTER;
	//BYTE				baData[sizeof(WIN32_STREAM_ID) + sizeof(TCHAR)];
	WIN32_STREAM_ID	StrmId;
	DWORD				dwBytesToRead;
	DWORD				dwBytesRead;
	BOOL				bResult = TRUE;
	LARGE_INTEGER		liDataSize;
	DWORD				dwNameSize = 0;	
	DWORD				dwLowByteSeeked;
	DWORD				dwHighByteSeeked;

	PNXT_FIND_STREAM_DATA pFindStreamData = (PNXT_FIND_STREAM_DATA)lpFindStreamData;

	if (hFindStream == INVALID_HANDLE_VALUE || lpFindStreamData == NULL || lpContext == NULL)
		CELOG_RETURN_VAL(FALSE);

	while (dwNameSize == 0)
	{
		dwBytesToRead = FIELD_OFFSET(WIN32_STREAM_ID, cStreamName);
		ZeroMemory((void *)&StrmId, sizeof(StrmId));

		// read the header part
		bResult = BackupRead( hFindStream, 
						(LPBYTE)&StrmId,
						dwBytesToRead,
						&dwBytesRead,
						FALSE, // am I done?
						FALSE,
						lpContext );
		if (dwBytesRead == 0)
		{
			bResult = FALSE;
			break;
		}
		else
		{
			// saving the header info

			dwNameSize = StrmId.dwStreamNameSize;
			liDataSize = StrmId.Size;
			pFindStreamData->StreamSize = StrmId.Size;

			if(dwNameSize > 0)
			{
				//CopyMemory(pFindStreamData->cStreamName, pStrmId->cStreamName, 6);

				dwBytesToRead = dwNameSize;
				LPBYTE lpBuffer = new BYTE[dwBytesToRead];
				
				if(!BackupRead(	hFindStream, 
								lpBuffer,
								dwBytesToRead,
								&dwBytesRead,
								FALSE, // am I done?
								FALSE,
								lpContext ) )
				{
					bResult = FALSE;
				}			
				
				if (dwBytesToRead > sizeof(WCHAR)*(MAX_PATH + 32))
					dwBytesToRead = sizeof(WCHAR)*(MAX_PATH + 32);
				CopyMemory((pFindStreamData->cStreamName), lpBuffer, dwBytesToRead);
				delete[] lpBuffer;
			}
			pFindStreamData->cStreamName[dwNameSize / sizeof(WCHAR)] = L'\0';

			// Skip the remain stream data
			if(liDataSize.LowPart != 0 || liDataSize.HighPart != 0)
			{
				dwLowByteSeeked = 0;
				dwHighByteSeeked = 0;

				BackupSeek(hFindStream, 
						 liDataSize.LowPart,
						 liDataSize.HighPart,
						 &dwLowByteSeeked,
						 &dwHighByteSeeked,
						 lpContext);
			}
		}
	}

	CELOG_RETURN_VAL(bResult);
}

/** NxtWriteAltDataStream
 *
 *  \brief Get the next NTFS Alternate Data Stream of the file.
 *
 *  \param inFileName (in)			Path to file.
 *  \param streamName (in)			The Alternate Data Stream name.
 *  \param lpDataBuffer (in)		ADS data content need to be written.
 *  \param dwDataLen (in)			ADS data content length.
 *
 *  \return Non-zero on success, otherwise return 0.
 */
extern "C"
BOOL NxtWriteAltDataStream(
    _In_	LPCWSTR inFileName,
    _In_	LPCWSTR streamName,
	_In_	LPVOID	lpDataBuffer,
	_In_	DWORD	dwDataLen
	)
{
	CELOG_ENTER;
	BOOL ret = FALSE;
	WCHAR outFileStreamName[MAX_PATH*2 + 36]; memset(outFileStreamName, 0, sizeof(outFileStreamName));
	HANDLE hOutFileStream = INVALID_HANDLE_VALUE;
	DWORD dwBytesWritten = 0;
	DWORD dwTotalBytesWritten = 0;

	if (inFileName == NULL || streamName == NULL || lpDataBuffer == NULL || dwDataLen == 0)
		CELOG_RETURN_VAL(FALSE);

	_snwprintf_s(outFileStreamName, MAX_PATH*2 + 36, _TRUNCATE, L"%s%s", inFileName, streamName);

	hOutFileStream = CreateFileW( outFileStreamName, 
						GENERIC_WRITE, 
						FILE_SHARE_WRITE, 
						NULL, 
						CREATE_ALWAYS, 
						0, 
						NULL );
	if(hOutFileStream != INVALID_HANDLE_VALUE)
	{
		while (dwTotalBytesWritten < dwDataLen)
		{
			WriteFile(hOutFileStream, (LPCVOID)((LPBYTE)lpDataBuffer+dwTotalBytesWritten), 
				dwDataLen-dwTotalBytesWritten, &dwBytesWritten, 0);
			dwTotalBytesWritten += dwBytesWritten;
		}

		CloseHandle(hOutFileStream);
		ret = TRUE;
	}

	CELOG_RETURN_VAL(ret);
}

/** NxtCopyNtfsDataStream
 *
 * \brief Copy the specified Alternate Data Stream from source  
 * \brief file to the destination file.
 *
 *  \param inFileName (in)			Path of the source file.
 *  \param outFileName (in)			Path of the destination file.
 *  \param streamName (in)			The Alternate Data Stream name.
 *
 *  \return Non-zero on success, otherwise return 0.
 */
BOOL NxtCopyNtfsDataStream(
    _In_	LPCWSTR inFileName,
    _In_	LPCWSTR outFileName,
	_In_	LPCWSTR streamName
	)
{
	CELOG_ENTER;
	BOOL ret = TRUE;
	WCHAR inFileStreamName[MAX_PATH*2 + 36]; memset(inFileStreamName, 0, sizeof(inFileStreamName));
	WCHAR outFileStreamName[MAX_PATH*2 + 36]; memset(outFileStreamName, 0, sizeof(outFileStreamName));
	BYTE cDataBuffer[8*1024]; 
	DWORD dwBytesToRead = 8*1024;
	DWORD dwBytesRead = 0;
	HANDLE hInFileStream = INVALID_HANDLE_VALUE;
	HANDLE hOutFileStream = INVALID_HANDLE_VALUE;

	if (inFileName == NULL || outFileName == NULL || streamName == NULL)
		CELOG_RETURN_VAL(FALSE);

	//CELOG_LOG(CELOG_DEBUG, L"NLSE!doCopyNtfsDataStream: Copy stream %s from %s to %s.\n", streamName, inFileName, outFileName);

	_snwprintf_s(inFileStreamName, MAX_PATH*2 + 36, _TRUNCATE, L"%s%s", inFileName, streamName);
	_snwprintf_s(outFileStreamName, MAX_PATH*2 + 36, _TRUNCATE, L"%s%s", outFileName, streamName);

	hInFileStream = CreateFileW( inFileStreamName, 
						GENERIC_READ, 
						FILE_SHARE_READ, 
						NULL, 
						OPEN_EXISTING, 
						0, 
						NULL );
	if(hInFileStream == INVALID_HANDLE_VALUE)
	{
		ret = FALSE;
		goto exit;
	}

	hOutFileStream = CreateFileW( outFileStreamName, 
						GENERIC_WRITE, 
						FILE_SHARE_WRITE, 
						NULL, 
						CREATE_ALWAYS, 
						0, 
						NULL );
	if(hOutFileStream == INVALID_HANDLE_VALUE)
	{
		ret = FALSE;
		goto exit;
	}

	do
	{
		memset(cDataBuffer, 0, sizeof(cDataBuffer));
#pragma warning(push)
#pragma warning(disable: 6031)
		ReadFile(hInFileStream, (LPVOID)cDataBuffer, dwBytesToRead, &dwBytesRead, NULL);
#pragma warning(pop)
		if (dwBytesRead != 0)
		{
			DWORD dwBytesWritten = 0;
			WriteFile(hOutFileStream, (LPCVOID)cDataBuffer, dwBytesRead, &dwBytesWritten, 0);
		}
	} while(dwBytesRead != 0);

exit:
	if (hInFileStream != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hInFileStream);
	}

	if (hOutFileStream != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hOutFileStream);
	}

	CELOG_RETURN_VAL(ret);
}

/** NxtCopyNtfsDataStreams
 *
 * \brief Copy the Alternate Data Streams from source file to  
 * \brief the destination file.
 *
 *  \param inFileName (in)			Path of the source file.
 *  \param outFileName (in)			Path of the destination file.
 *
 *  \return Non-zero on success, otherwise return 0.
 */
BOOL NxtCopyNtfsDataStreams(
    _In_	LPCWSTR inFileName,
    _In_	LPCWSTR outFileName
	)
{
	CELOG_ENTER;
	BOOL ret = TRUE;
	NXT_FIND_STREAM_DATA FindStreamData;
	memset((LPVOID)&FindStreamData, 0, sizeof(FindStreamData));
	LPVOID lpContext = NULL;

	if (inFileName == NULL || outFileName == NULL)
		CELOG_RETURN_VAL(FALSE);
	CELOG_LOG(CELOG_DEBUG, L"NxtCopyNtfsDataStreams..........Files:");
	CELOG_LOG(CELOG_DEBUG, inFileName);
	CELOG_LOG(CELOG_DEBUG, outFileName);

	HANDLE hFindStream = NxtFindFirstStream(inFileName, (LPVOID)&FindStreamData, &lpContext);
	if (hFindStream == INVALID_HANDLE_VALUE)
	{
		ret = FALSE;
		goto exit;
	}

	do
	{
		CELOG_LOG(CELOG_DEBUG, L"Copy Stream..........:");
		CELOG_LOG(CELOG_DEBUG, FindStreamData.cStreamName);
		ret = NxtCopyNtfsDataStream(inFileName, outFileName, FindStreamData.cStreamName);
		if (!ret)	break;

		memset((LPVOID)&FindStreamData, 0, sizeof(FindStreamData));
	} while (NxtFindNextStream(hFindStream, (LPVOID)&FindStreamData, &lpContext));
	

exit:
	if (hFindStream != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hFindStream);
	}

	CELOG_RETURN_VAL(ret);
}