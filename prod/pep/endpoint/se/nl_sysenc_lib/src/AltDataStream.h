#ifndef __ALT_DATA_STREAM_H
#define __ALT_DATA_STREAM_H

typedef struct _NXT_FIND_STREAM_DATA {

    LARGE_INTEGER StreamSize;
    WCHAR cStreamName[ MAX_PATH + 36 ];

} NXT_FIND_STREAM_DATA, *PNXT_FIND_STREAM_DATA;


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
	);

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
extern "C"
BOOL NxtFindNextStream(
    _In_	HANDLE hFindStream,
    _Out_	LPVOID lpFindStreamData,
	_Inout_	LPVOID *lpContext
	);

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
	);

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
extern "C"
BOOL NxtCopyNtfsDataStream(
    _In_	LPCWSTR inFileName,
    _In_	LPCWSTR outFileName,
	_In_	LPCWSTR streamName
	);

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
extern "C"
BOOL NxtCopyNtfsDataStreams(
    _In_	LPCWSTR inFileName,
    _In_	LPCWSTR outFileName
	);

#endif //__ALT_DATA_STREAM_H