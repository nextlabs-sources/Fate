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
#include <Wincrypt.h>
#include <security.h>
#include <wtypes.h>
#include <OAIdl.h>
#include "FTPEEval.h"
#include <Mswsock.h>
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
	BOOL DoHookSSL4FTPTE(void)	 ;
	BOOL DoHookSFTP4FTPTE(void) ;
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

	typedef HANDLE (WINAPI* CreateFileAType)(
		LPCSTR lpFileName,
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

	typedef HRESULT (WINAPI *CoCreateInstanceType)(
		REFCLSID rclsid,
		void* pUnkOuter, 
		DWORD dwClsContext, 
		REFIID riid,
		LPVOID * ppv);
	/*
	SmartFTP SFTP
	*/
	typedef HRESULT( WINAPI* DownloadFileType)(
		PVOID pthis,
		BSTR bstrRemoteFile, 
										VARIANT varLocalFile, 
										long nRemoteStartPosLo, 
										long nRemoteStartPosHi, 
										long nLocalStartPosLo, 
										long nLocalStartPosHi, 
		PVOID pError)  ;
	typedef HRESULT (WINAPI* DownloadFileExType)(
		PVOID pthis,
		BSTR bstrRemoteFile, 
	VARIANT varLocalFile, 
	ULONGLONG nRemoteStartPos, 
	ULONGLONG nLocalStartPos, 
	long dwCreateDeposition, 
		PVOID retval ) ;
	typedef HRESULT (WINAPI* UploadFileType)( 
		PVOID pthis,
		  VARIANT varLocalFile, 
									  BSTR bstrRemoteFile, 
									  long nLocalStartPosLo, 
									  long nLocalStartPosHi, 
									  long nRemoteStartPosLo, 
									  long nRemoteStartPosHi, 
		PVOID retval ) ;

	typedef HMODULE (WINAPI* LoadLibraryAType)(LPCSTR lpFileName);

	//// For SmartFtp FTPS
	typedef SECURITY_STATUS (__stdcall* EncryptMessageType)(
	  PCtxtHandle phContext,
	  ULONG fQOP,
	  PSecBufferDesc pMessage,
	  ULONG MessageSeqNo
	);

	typedef SECURITY_STATUS (__stdcall* DecryptMessageType)(
	  PCtxtHandle phContext,
	  PSecBufferDesc pMessage,
	  ULONG MessageSeqNo,
	  PULONG pfQOP
	);

	//// For CuteFtp FTPS
	typedef int (WINAPIV* SSL_ReadType)(/*SSL*/ void *s, void *buf,int num);
	typedef int (WINAPIV* SSL_WriteType)(/*SSL*/ void *s,const void *buf,int num);

	//// For CuteFtp SFTP
	typedef int (__cdecl* CreateSFTPConnectionType)(DWORD* pConnHandle, int arg_1, int arg_2, int arg_3, const char* user, const char* pwd, int arg_6, int arg_7, int arg_8, int arg_9, char *Src, int arg_11, int arg_12, const char* ip, __int16 port);
	typedef int (__cdecl* WriteSFTPFile2Type)(DWORD connHandle, const char* svrFilePath, HANDLE hFile, int arg_3, int arg_4);
	typedef int (__cdecl* ReadSFTPFileType)(DWORD connHandle, const char* svrFilePath, HANDLE hFile, int arg_3, int arg_4, DWORD dwFileSize, int arg_6);
	typedef int (__cdecl* CloseSFTPConnectionType)(DWORD connHandle);

	typedef BOOL (WINAPI* DeleteFileWType)(   LPCWSTR lpFileName ); 

	typedef BOOL (WINAPI* TransmitFileType)(
									SOCKET hSocket,
									HANDLE hFile,
									DWORD nNumberOfBytesToWrite,
									DWORD nNumberOfBytesPerSend,
									LPOVERLAPPED lpOverlapped,
									LPTRANSMIT_FILE_BUFFERS lpTransmitBuffers,
									DWORD dwFlags
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
	typedef BOOL (WINAPI *MoveFileExAType)(
								_In_          LPCSTR lpExistingFileName,
								_In_          LPCSTR lpNewFileName,
								_In_          DWORD dwFlags
								);
typedef int (WINAPI * SHFileOperationWType)(
									LPSHFILEOPSTRUCT lpFileOp
									);

public:
	static CreateFileWType real_CreateFileW;
	static CreateFileAType real_CreateFileA;
	static WriteFileType   real_WriteFile;
	static CloseHandleType real_CloseHandle;
	static LoadLibraryAType real_LoadLibraryA;
	static HRESULT* real_CoCreateInstance;

	//// For SmartFTP FTPS
	static EncryptMessageType real_EncryptMessage;
	static DecryptMessageType real_DecryptMessage;

	//// For CuteFtp FTPS
	static SSL_ReadType real_SSL_Read;
	static SSL_WriteType real_SSL_Write;

	//// For CuteFtp SFTP
	static CreateSFTPConnectionType real_CreateSFTPConnection;
	static WriteSFTPFile2Type real_WriteSFTPFile2;
	static ReadSFTPFileType real_ReadSFTPFile;
	static CloseSFTPConnectionType real_CloseSFTPConnection;

	static	DeleteFileWType real_DeleteFileW ;

	static TransmitFileType real_TransmitFile;

		static CopyFileExWType real_CopyFileExW;
	static MoveFileWType real_MoveFileW;
	static MoveFileExWType real_MoveFileExW;
	static MoveFileExAType real_MoveFileExA;
	static SHFileOperationWType real_SHFileOperationW;
public:
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
	static BOOL WINAPI my_MoveFileExA(
									_In_          LPCSTR lpExistingFileName,
									_In_          LPCSTR lpNewFileName,
									_In_          DWORD dwFlags
									);
	static BOOL WINAPI try_MoveFileExA(
									_In_          LPCSTR lpExistingFileName,
									_In_          LPCSTR lpNewFileName,
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
		static int WINAPI try_SHFileOperationW(
								LPSHFILEOPSTRUCT lpFileOp
								);
	
	static int WINAPI my_SHFileOperationW(
								LPSHFILEOPSTRUCT lpFileOp
								);
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

	static HANDLE WINAPI try_CreateFileA(
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
		HANDLE hTemplateFile);

	static HANDLE WINAPI MyCreateFileAW(
		LPCWSTR lpFileName,
		DWORD dwDesiredAccess,
		DWORD dwShareMode,
		LPSECURITY_ATTRIBUTES lpSecurityAttributes,
		DWORD dwCreationDisposition,
		DWORD dwFlagsAndAttributes,
		HANDLE hTemplateFile,
		BOOL bIsUnicode=TRUE,
		LPCSTR lpFileNameChar=NULL);

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

	static HRESULT WINAPI try_CoCreateInstance(
		REFCLSID rclsid,
		void* pUnkOuter, 
		DWORD dwClsContext,
		REFIID riid, 
		LPVOID * ppv);
	static HRESULT WINAPI MyCoCreateInstance(
		REFCLSID rclsid,
		void* pUnkOuter, 
		DWORD dwClsContext,
		REFIID riid, 
		LPVOID * ppv);
	/*
	Smart Sftp
	*/
	static HRESULT WINAPI retReal_DownloadFile(
		PVOID pthis,
		BSTR bstrRemoteFile, 
		VARIANT varLocalFile, 
		long nRemoteStartPosLo, 
		long nRemoteStartPosHi, 
		long nLocalStartPosLo, 
		long nLocalStartPosHi, 
		PVOID pError)  ;
	static HRESULT WINAPI try_DownloadFile(
		PVOID pthis,
		BSTR bstrRemoteFile, 
		VARIANT varLocalFile, 
		long nRemoteStartPosLo, 
		long nRemoteStartPosHi, 
		long nLocalStartPosLo, 
		long nLocalStartPosHi, 
		PVOID pError)  ;
	static HRESULT WINAPI myDownloadFile(
		PVOID pthis,
	BSTR bstrRemoteFile, 
										VARIANT varLocalFile, 
										long nRemoteStartPosLo, 
										long nRemoteStartPosHi, 
										long nLocalStartPosLo, 
										long nLocalStartPosHi, 
		PVOID pError)  ;
	static HRESULT WINAPI retReal_DownloadFileEx(
		PVOID pthis,
		BSTR bstrRemoteFile, 
		VARIANT varLocalFile, 
		ULONGLONG nRemoteStartPos, 
		ULONGLONG nLocalStartPos, 
		long dwCreateDeposition,  
		PVOID retval ) ;
	static HRESULT WINAPI try_DownloadFileEx(
		PVOID pthis,
		BSTR bstrRemoteFile, 
		VARIANT varLocalFile, 
		ULONGLONG nRemoteStartPos, 
		ULONGLONG nLocalStartPos, 
		long dwCreateDeposition,  
		PVOID retval ) ;
	static HRESULT WINAPI myDownloadFileEx(
		PVOID pthis,
	BSTR bstrRemoteFile, 
	VARIANT varLocalFile, 
	ULONGLONG nRemoteStartPos, 
	ULONGLONG nLocalStartPos, 
	long dwCreateDeposition,  
		PVOID retval ) ;
	static HRESULT WINAPI retReal_UploadFile(
		PVOID pthis,
		VARIANT varLocalFile, 
		BSTR bstrRemoteFile, 
		long nLocalStartPosLo, 
		long nLocalStartPosHi, 
		long nRemoteStartPosLo, 
		long nRemoteStartPosHi, 
		PVOID retval ) ;
	static HRESULT WINAPI try_UploadFile(
		PVOID pthis,
		VARIANT varLocalFile, 
		BSTR bstrRemoteFile, 
		long nLocalStartPosLo, 
		long nLocalStartPosHi, 
		long nRemoteStartPosLo, 
		long nRemoteStartPosHi, 
		PVOID retval ) ;
	static HRESULT WINAPI myUploadFile(
		PVOID pthis,
			  VARIANT varLocalFile, 
									  BSTR bstrRemoteFile, 
									  long nLocalStartPosLo, 
									  long nLocalStartPosHi, 
									  long nRemoteStartPosLo, 
									  long nRemoteStartPosHi, 
		PVOID retval ) ;

	static HMODULE WINAPI try_LoadLibraryA(LPCSTR lpFileName);
	static HMODULE WINAPI MyLoadLibraryA(LPCSTR lpFileName);

	//// For SmartFtp FTPS
	static SECURITY_STATUS __stdcall try_EncryptMessage(
		PCtxtHandle phContext,
		ULONG fQOP,
		PSecBufferDesc pMessage,
		ULONG MessageSeqNo
		);
	static SECURITY_STATUS __stdcall MyEncryptMessage(
	  PCtxtHandle phContext,
	  ULONG fQOP,
	  PSecBufferDesc pMessage,
	  ULONG MessageSeqNo
	);
	static SECURITY_STATUS __stdcall try_DecryptMessage(
		PCtxtHandle phContext,
		PSecBufferDesc pMessage,
		ULONG MessageSeqNo,
		PULONG pfQOP);
	static SECURITY_STATUS __stdcall MyDecryptMessage(
	  PCtxtHandle phContext,
	  PSecBufferDesc pMessage,
	  ULONG MessageSeqNo,
	  PULONG pfQOP);

	static BOOL WINAPI try_TransmitFile(
									SOCKET hSocket,
									HANDLE hFile,
									DWORD nNumberOfBytesToWrite,
									DWORD nNumberOfBytesPerSend,
									LPOVERLAPPED lpOverlapped,
									LPTRANSMIT_FILE_BUFFERS lpTransmitBuffers,
									DWORD dwFlags
									);

	static BOOL WINAPI my_TransmitFile(
									SOCKET hSocket,
									HANDLE hFile,
									DWORD nNumberOfBytesToWrite,
									DWORD nNumberOfBytesPerSend,
									LPOVERLAPPED lpOverlapped,
									LPTRANSMIT_FILE_BUFFERS lpTransmitBuffers,
									DWORD dwFlags
									);


	//// For CuteFtp FTPS
	static int WINAPIV MySSL_Read(/*SSL*/ void *s, void *buf,int num);
	static int WINAPIV MySSL_Write(/*SSL*/ void *s,const void *buf,int num);

	//// For CuteFtp SFTP
	static int __cdecl MyCreateSFTPConnection(DWORD* pConnHandle, int arg_1, int arg_2, int arg_3, const char* user, const char* pwd, int arg_6, int arg_7, int arg_8, int arg_9, char *Src, int arg_11, int arg_12, const char* ip, __int16 port);
	static int __cdecl MyWriteSFTPFile2(DWORD connHandle, const char* svrFilePath, HANDLE hFile, int arg_3, int arg_4);
	static int __cdecl MyReadSFTPFile(DWORD connHandle, const char* svrFilePath, HANDLE hFile, int arg_3, int arg_4, DWORD dwFileSize, int arg_6);
	static int __cdecl MyCloseSFTPConnection(DWORD connHandle);
private:
	static BOOL ImpSmartFtpSFTP( LPVOID pUnkOuter, LPVOID* ppv   ) ;
	static BOOL HookComInterface(UINT _iTID, std::list<APIRES::HOOKAPIINFO> &hookAPIInfo, LONG_PTR pOriginFunc, LONG_PTR  pmyFunc ,PVOID pObjectAddress) ;
	static BOOL MovePhaseEval(const std::wstring & srcMoveFile, const std::wstring & dstMoveFile);

private:
	 static std::list<APIRES::HOOKAPIINFO>	m_listSmartFtp_UpFile ;
	 static std::list<APIRES::HOOKAPIINFO>	m_listSmartFtp_DownFile ;
	 static std::list<APIRES::HOOKAPIINFO>	m_listSmartFtp_DownFileEx ;
	 static HRESULT GetUnknownPointer( LPVOID pIDispatch, REFIID refID , LPUNKNOWN &lpUnknown ) ;
	 static std::list<std::wstring>   m_listSmartSftpRemoveList ;
	 static DWORD GetVersionNumber( std::wstring szModuleName, std::wstring szKeyName ) ;
	static CPolicy * m_pPolicy ;
};
#endif