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
#include "Eval.h"
#include <OAIdl.h>
#include <Mswsock.h>

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
	static BOOL ImpSmartFtpSFTP( LPVOID pUnkOuter, LPVOID* ppv   ) ;
	static BOOL HookComInterface(UINT _iTID, std::list<APIRES::HOOKAPIINFO> &hookAPIInfo, LONG_PTR pOriginFunc, LONG_PTR  pmyFunc ,PVOID pObjectAddress) ;
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

	typedef HRESULT (WINAPI* UploadFileType)( 
		PVOID pthis,
		VARIANT varLocalFile, 
		BSTR bstrRemoteFile, 
		long nLocalStartPosLo, 
		long nLocalStartPosHi, 
		long nRemoteStartPosLo, 
		long nRemoteStartPosHi, 
		PVOID retval ) ;

	typedef BOOL (WINAPI* TransmitFileType)(
		SOCKET hSocket,
		HANDLE hFile,
		DWORD nNumberOfBytesToWrite,
		DWORD nNumberOfBytesPerSend,
		LPOVERLAPPED lpOverlapped,
		LPTRANSMIT_FILE_BUFFERS lpTransmitBuffers,
		DWORD dwFlags
		);

public:
	static CreateFileWType real_CreateFileW;
	static CreateFileAType real_CreateFileA;
	static CloseHandleType real_CloseHandle;
	static HRESULT* real_CoCreateInstance;

	static CPolicy *m_pEvaluaImp ;
	 static std::list<APIRES::HOOKAPIINFO>	m_listSmartFtp_UpFile ;

	 static TransmitFileType real_TransmitFile;
public:
	static HANDLE WINAPI MyCreateFileW(
		LPCWSTR lpFileName,
		DWORD dwDesiredAccess,
		DWORD dwShareMode,
		LPSECURITY_ATTRIBUTES lpSecurityAttributes,
		DWORD dwCreationDisposition,
		DWORD dwFlagsAndAttributes,
		HANDLE hTemplateFile);

	static HANDLE WINAPI try_MyCreateFileW(
		LPCWSTR lpFileName,
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

	static HANDLE WINAPI try_MyCreateFileA(
		LPCSTR lpFileName,
		DWORD dwDesiredAccess,
		DWORD dwShareMode,
		LPSECURITY_ATTRIBUTES lpSecurityAttributes,
		DWORD dwCreationDisposition,
		DWORD dwFlagsAndAttributes,
		HANDLE hTemplateFile);

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

	static BOOL WINAPI MyCloseHandle(HANDLE hObject);
	static BOOL WINAPI try_MyCloseHandle(HANDLE hObject);

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

};
#endif