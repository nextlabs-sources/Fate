#pragma once

#include <ShObjIdl.h>
#include <map>
#include "common.h"

typedef HRESULT ( STDMETHODCALLTYPE *f_CopyItems )( 
	IFileOperation * This,
	IUnknown *punkItems,
	IShellItem *psiDestinationFolder);
typedef HRESULT ( STDMETHODCALLTYPE *f_PerformOperations )( 
	IFileOperation * This);


class CFileOperationHooking
{
public:
	static CFileOperationHooking* GetInstance();
	bool Hook(IFileOperation* pObject);
protected:
	CFileOperationHooking(void);
	~CFileOperationHooking(void);


	static HRESULT WINAPI MyPerformOperations(IFileOperation * This);

protected:
	static std::map<LPVOID, LPVOID> m_mapHooks;//the key is the original address of the function, the value is the next_func.

};
