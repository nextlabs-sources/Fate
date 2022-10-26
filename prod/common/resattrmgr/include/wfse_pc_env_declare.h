#pragma once


//	this is used for windows file server enforcer
//	because of a bug 15805, we need those transaction function
typedef HANDLE
(APIENTRY* _CreateTransaction) (
								IN LPSECURITY_ATTRIBUTES lpTransactionAttributes OPTIONAL,
								IN LPGUID UOW OPTIONAL,
								IN DWORD CreateOptions OPTIONAL,
								IN DWORD IsolationLevel OPTIONAL,
								IN DWORD IsolationFlags OPTIONAL,
								IN DWORD Timeout OPTIONAL,
								_In_opt_ LPWSTR Description
								);

typedef WINBASEAPI
BOOL
(WINAPI* _CopyFileTransactedW)(
							   _In_     LPCWSTR lpExistingFileName,
							   _In_     LPCWSTR lpNewFileName,
							   _In_opt_ LPPROGRESS_ROUTINE lpProgressRoutine,
							   _In_opt_ LPVOID lpData,
							   _In_opt_ LPBOOL pbCancel,
							   _In_     DWORD dwCopyFlags,
							   _In_     HANDLE hTransaction
							   );

typedef BOOL
(APIENTRY* _CommitTransaction) (
								IN HANDLE TransactionHandle
								);


//	used to indicate if the env is wfse pc env
typedef enum
{
	WFSE_PC_UNSET,
	WFSE_PC_NO,
	WFSE_PC_YES
}WFSE_PC_STATUS;