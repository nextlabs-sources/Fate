#ifndef __API_HOOK_H__
#define __API_HOOK_H__
/*
Maintain the API Hooking
*/
#define SECURITY_WIN32

#include <list>
#include "HookRes.h"
#include <string>
#include "Wininet.h"
#include <Winsock2.h>
#include "Security.h"
#include "timeout_list.hpp"
#include <Shellapi.h>

class CAPIHook
{
public:
	CAPIHook(){}
	~CAPIHook(){}

public:
	BOOL StartHook();
	VOID EndHook();

private:
	BOOL  HookAPIByInfoList();
	PVOID GetExportFunc_ByID( const wchar_t *pszModuleName, DWORD i_dID ) ;
	PVOID GetFuncAddr_ByOffset( DWORD i_dOffset, char* i_pszModuleName ) ;

public:
	typedef HANDLE (WINAPI* CreateFileWType)(
		LPCWSTR lpFileName,
		DWORD dwDesiredAccess,
		DWORD dwShareMode,
		LPSECURITY_ATTRIBUTES lpSecurityAttributes,
		DWORD dwCreationDisposition,
		DWORD dwFlagsAndAttributes,
		HANDLE hTemplateFile);


	typedef BOOL (WINAPI* WriteFileType)(
		HANDLE hFile,
		LPCVOID lpBuffer,
		DWORD nNumberOfBytesToWrite,
		LPDWORD lpNumberOfBytesWritten,
		LPOVERLAPPED lpOverlapped);

	typedef BOOL (WINAPI* CloseHandleType)(HANDLE hObject);
	typedef BOOL (WINAPI* DeleteFileWType)(   LPCWSTR lpFileName ); 

	typedef BOOL (WINAPI *ReadFileType)(
		HANDLE hFile,
		LPVOID lpBuffer,
		DWORD nNumberOfBytesToRead,
		LPDWORD lpNumberOfBytesRead,
		LPOVERLAPPED lpOverlapped
		);
	typedef SECURITY_STATUS (WINAPI* EncryptMessageType)( PVOID phContext,ULONG fQOP, PVOID pMessage, ULONG MessageSeqNo );
	typedef SECURITY_STATUS (WINAPI* DecryptMessageType)(PVOID phContext,  PVOID pMessage,  ULONG MessageSeqNo,  PVOID pfQOP );



	typedef BOOL (WINAPI *CopyFileType)(
										_In_          LPCWSTR lpExistingFileName,
										_In_          LPCWSTR lpNewFileName,
										_In_          BOOL bFailIfExists
										);
	typedef BOOL (WINAPI *CopyFileExWType)(
								_In_          LPCWSTR lpExistingFileName,
								_In_          LPCWSTR lpNewFileName,
								_In_          LPPROGRESS_ROUTINE lpProgressRoutine,
								_In_          LPVOID lpData,
								_In_          LPBOOL pbCancel,
								_In_          DWORD dwCopyFlags
								);

	typedef BOOL (WINAPI *MoveFileWType)(
								_In_          LPCWSTR lpExistingFileName,
								_In_          LPCWSTR lpNewFileName
								);

	typedef BOOL (WINAPI *MoveFileExWType)(
								_In_          LPCWSTR lpExistingFileName,
								_In_          LPCWSTR lpNewFileName,
								_In_          DWORD dwFlags
								);

	typedef int (WINAPIV * PK11_CipherOpType)(void *context, unsigned char * out, int *outlen, 
											int maxout, unsigned char *in, int inlen);

	typedef int (WINAPIV * PK11_DigestFinalType)(void *context, unsigned char *data, 
											unsigned int *outLen, unsigned int length);

	typedef BOOL (WINAPI * MoveFileWithProgressWType)(
		_In_          LPCWSTR lpExistingFileName,
		_In_          LPCWSTR lpNewFileName,
		_In_          LPPROGRESS_ROUTINE lpProgressRoutine,
		_In_          LPVOID lpData,
		_In_          DWORD dwFlags
		);

	typedef int (WINAPI * SHFileOperationWType)(
									LPSHFILEOPSTRUCT lpFileOp
									);

	typedef HANDLE (WINAPI* CreateFileAType)(
		LPCSTR lpFileName,
		DWORD dwDesiredAccess,
		DWORD dwShareMode,
		LPSECURITY_ATTRIBUTES lpSecurityAttributes,
		DWORD dwCreationDisposition,
		DWORD dwFlagsAndAttributes,
		HANDLE hTemplateFile);
public:
	static CreateFileWType real_CreateFileW;
	static	ReadFileType real_ReadFile ;
	static WriteFileType   real_WriteFile;
	static CloseHandleType real_CloseHandle;
	static HRESULT*   real_CoCreateInstance  ;
	static EncryptMessageType	real_EncryptMessage ;
	static DecryptMessageType	real_DecryptMessage ;
	static CopyFileType real_CopyFileW;
	static CopyFileExWType real_CopyFileExW;
	static MoveFileWType real_MoveFileW;
	static MoveFileExWType real_MoveFileExW;

	static PK11_CipherOpType real_PK11_CipherOp;
	static PK11_DigestFinalType real_PK11_DigestFinal;
	
	static MoveFileWithProgressWType real_MoveFileWithProgressW;

	static SHFileOperationWType real_SHFileOperationW;

	static CreateFileAType real_CreateFileA;
public:

	static BOOL WINAPI try_DeleteFileW(   LPCWSTR lpFileName ) ;
	static BOOL WINAPI myDeleteFileW(   LPCWSTR lpFileName ) ;

	static HANDLE WINAPI try_CreateFileW(
		LPCWSTR lpFileName,
		DWORD dwDesiredAccess,
		DWORD dwShareMode,
		LPSECURITY_ATTRIBUTES lpSecurityAttributes,
		DWORD dwCreationDisposition,
		DWORD dwFlagsAndAttributes,
		HANDLE hTemplateFile);
	static HANDLE WINAPI MyCreateFileW(
		LPCWSTR lpFileName,
		DWORD dwDesiredAccess,
		DWORD dwShareMode,
		LPSECURITY_ATTRIBUTES lpSecurityAttributes,
		DWORD dwCreationDisposition,
		DWORD dwFlagsAndAttributes,
		HANDLE hTemplateFile);

	/*static HANDLE WINAPI try_CreateFileA(
		LPCSTR lpFileName,
		DWORD dwDesiredAccess,
		DWORD dwShareMode,
		LPSECURITY_ATTRIBUTES lpSecurityAttributes,
		DWORD dwCreationDisposition,
		DWORD dwFlagsAndAttributes,
		HANDLE hTemplateFile);

	static HANDLE WINAPI MyCreateFileA(
		LPCSTR lpFileName,
		DWORD dwDesiredAccess,
		DWORD dwShareMode,
		LPSECURITY_ATTRIBUTES lpSecurityAttributes,
		DWORD dwCreationDisposition,
		DWORD dwFlagsAndAttributes,
		HANDLE hTemplateFile);*/

	static BOOL WINAPI try_CloseHandle(HANDLE hObject);
	static BOOL WINAPI MyCloseHandle(HANDLE hObject);

	static BOOL WINAPI try_WriteFile(
		HANDLE hFile,
		LPCVOID lpBuffer,
		DWORD nNumberOfBytesToWrite,
		LPDWORD lpNumberOfBytesWritten,
		LPOVERLAPPED lpOverlapped);
	static BOOL WINAPI MyWriteFile(
		HANDLE hFile,
		LPCVOID lpBuffer,
		DWORD nNumberOfBytesToWrite,
		LPDWORD lpNumberOfBytesWritten,
		LPOVERLAPPED lpOverlapped);


	static BOOL WINAPI try_ReadFile(
		HANDLE hFile,
		LPVOID lpBuffer,
		DWORD nNumberOfBytesToRead,
		LPDWORD lpNumberOfBytesRead,
		LPOVERLAPPED lpOverlapped
		);
	static BOOL WINAPI my_ReadFile(
		HANDLE hFile,
		LPVOID lpBuffer,
		DWORD nNumberOfBytesToRead,
		LPDWORD lpNumberOfBytesRead,
		LPOVERLAPPED lpOverlapped
		);
	static SECURITY_STATUS WINAPI try_EncryptMessage( PVOID phContext,ULONG fQOP, PVOID pMessage, ULONG MessageSeqNo );
	static SECURITY_STATUS WINAPI my_EncryptMessage( PVOID phContext,ULONG fQOP, PVOID pMessage, ULONG MessageSeqNo );
	static SECURITY_STATUS WINAPI try_DecryptMessage(PVOID phContext,  PVOID pMessage,  ULONG MessageSeqNo,  PVOID pfQOP );
	static SECURITY_STATUS WINAPI my_DecryptMessage(PVOID phContext,  PVOID pMessage,  ULONG MessageSeqNo,  PVOID pfQOP );

	static BOOL WINAPI try_CopyFileW(
								_In_          LPCWSTR lpExistingFileName,
								_In_          LPCWSTR lpNewFileName,
								_In_          BOOL bFailIfExists
								);

	static BOOL WINAPI my_CopyFileW(
								_In_          LPCWSTR lpExistingFileName,
								_In_          LPCWSTR lpNewFileName,
								_In_          BOOL bFailIfExists
								);

	static BOOL WINAPI try_CopyFileExW(
										_In_          LPCWSTR lpExistingFileName,
										_In_          LPCWSTR lpNewFileName,
										_In_          LPPROGRESS_ROUTINE lpProgressRoutine,
										_In_          LPVOID lpData,
										_In_          LPBOOL pbCancel,
										_In_          DWORD dwCopyFlags
										);

	static BOOL WINAPI my_CopyFileExW(
										_In_          LPCWSTR lpExistingFileName,
										_In_          LPCWSTR lpNewFileName,
										_In_          LPPROGRESS_ROUTINE lpProgressRoutine,
										_In_          LPVOID lpData,
										_In_          LPBOOL pbCancel,
										_In_          DWORD dwCopyFlags
										);

	static BOOL WINAPI my_MoveFileExW(
									_In_          LPCWSTR lpExistingFileName,
									_In_          LPCWSTR lpNewFileName,
									_In_          DWORD dwFlags
									);
	static BOOL WINAPI try_MoveFileExW(
									_In_          LPCWSTR lpExistingFileName,
									_In_          LPCWSTR lpNewFileName,
									_In_          DWORD dwFlags
									);

	static BOOL WINAPI my_MoveFileW(
									_In_          LPCWSTR lpExistingFileName,
									_In_          LPCWSTR lpNewFileName
									);
	static BOOL WINAPI try_MoveFileW(
									_In_          LPCWSTR lpExistingFileName,
									_In_          LPCWSTR lpNewFileName
									);

	static int WINAPIV my_PK11_CipherOp(void *context, unsigned char * out, int *outlen, 
									int maxout, unsigned char *in, int inlen);
	static int WINAPIV try_PK11_CipherOp(void *context, unsigned char * out, int *outlen, 
									int maxout, unsigned char *in, int inlen);

	static int WINAPIV try_PK11_DigestFinal(void *context, unsigned char *data, 
										unsigned int *outLen, unsigned int length);
	static int WINAPIV my_PK11_DigestFinal(void *context, unsigned char *data, 
										unsigned int *outLen, unsigned int length);

	static BOOL WINAPI try_MoveFileWithProgressW(
		_In_          LPCWSTR lpExistingFileName,
		_In_          LPCWSTR lpNewFileName,
		_In_          LPPROGRESS_ROUTINE lpProgressRoutine,
		_In_          LPVOID lpData,
		_In_          DWORD dwFlags
		);

	static BOOL WINAPI my_MoveFileWithProgressW(
		_In_          LPCWSTR lpExistingFileName,
		_In_          LPCWSTR lpNewFileName,
		_In_          LPPROGRESS_ROUTINE lpProgressRoutine,
		_In_          LPVOID lpData,
		_In_          DWORD dwFlags
		);

	static int WINAPI try_SHFileOperationW(
								LPSHFILEOPSTRUCT lpFileOp
								);
	
	static int WINAPI my_SHFileOperationW(
								LPSHFILEOPSTRUCT lpFileOp
								);

	static HANDLE WINAPI try_CreateFileA(
		LPCSTR lpFileName,
		DWORD dwDesiredAccess,
		DWORD dwShareMode,
		LPSECURITY_ATTRIBUTES lpSecurityAttributes,
		DWORD dwCreationDisposition,
		DWORD dwFlagsAndAttributes,
		HANDLE hTemplateFile);

private:
	
	static BOOL HookComInterface(UINT _iTID, std::list<APIRES::HOOKAPIINFO> &hookAPIInfo, LONG_PTR pOriginFunc, LONG_PTR  pmyFunc ,PVOID pObjectAddress) ;
	//static HRESULT GetUnknownPointer( LPVOID pIDispatch, REFIID refID , LPUNKNOWN &lpUnknown ) ;
	static DWORD GetVersionNumber( std::wstring szModuleName, std::wstring szKeyName ) ;
	 BOOL HookModuleAPI(  LPCSTR pszModule,  LPCSTR pszFuncName,  PVOID  pCallbackFunc,  PVOID  *pNextHook ) ;
	static BOOL HookCodeByAddr(  PVOID  pCode,  PVOID  pCallbackFunc,  PVOID  *pNextHook ) ;
	 BOOL UnHookModuleAPI(   PVOID  *pNextHook );
	 BOOL UnHookCodeByAddr(  PVOID  *pNextHook );

	 //	used by firefox or chrome.
	 //	this is doing evaluation in the second phase of download file.
	 //	in the first phase, file is downloaded into temp folder as a temp file,
	 //	in the second phase, temp file is moved to the real destination folder.
	 //	return value:
	 //	true means allow, false means denied
	 static BOOL MovePhaseEval(const wstring & srcMoveFile, const wstring & dstMoveFile);

	 static CTimeoutList m_listFireFox_Download;
	 static string m_strLastDigest;

};
#endif