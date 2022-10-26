#pragma once

#pragma warning(push)
#pragma warning(disable: 4819)
#include "madCHook_helper.h"
#pragma warning(pop)

#include <vector>
#include "NxtReceiver.h"

typedef struct {
	LPCSTR	dllName;
	LPCSTR	funcName;
	PVOID	newFunc;
	PVOID	*oldFunc;
} HookEntry;



class CHooking
{
public:
	static CHooking* GetInstance();
	bool StartHook();
	bool EndHook();

protected:
	static HookEntry HookTable[];


	static BOOL WINAPI MyCopyFileExW( LPCWSTR lpExistingFileName,
		LPCWSTR lpNewFileName,
		LPPROGRESS_ROUTINE lpProgressRoutine,
		LPVOID lpData,
		LPBOOL pbCancel,
		DWORD dwCopyFlags);

	static BOOL WINAPI MyMoveFileW(
		LPCWSTR lpExistingFileName,
		LPCWSTR lpNewFileName);

	static BOOL WINAPI MyMoveFileWithProgressW(
		_In_      LPCWSTR lpExistingFileName,
		_In_opt_  LPCWSTR lpNewFileName,
		_In_opt_  LPPROGRESS_ROUTINE lpProgressRoutine,
		_In_opt_  LPVOID lpData,
		_In_      DWORD dwFlags
		);



	static HRESULT WINAPI MyCoCreateInstance(
		REFCLSID rclsid,
		LPUNKNOWN pUnkOuter,
		DWORD dwClsContext,
		REFIID riid,
		LPVOID * ppv
		);

protected:
	CHooking(void);
	~CHooking(void);

};
