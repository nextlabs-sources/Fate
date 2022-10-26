#pragma once

#include <vector>
#include <string>

using namespace std;

#include "MutexHelper.h"



/*

collect logs and zip it to specified location


*/

class CThreadHelper;
class CCollectZipLog
{
private:
	CCollectZipLog();
	~CCollectZipLog(void);
	CCollectZipLog(const CCollectZipLog&);
	void operator = (const CCollectZipLog&);


public:
	static CCollectZipLog& GetInstance();

	/*

	async function, when collect/zip is finished, will call callback -- pOnCompleted -- to notify user.

	collect logs and zip it to specified location. 



	parameter:

	pOnCompleted	--	callback called when collect/zip is completed.

	pwd			--		password string for decrypting bundle.

	pszLocation		--	the location is specified by pszLocation in construction.

	return result:

	true --  collect and zip is started;
	false -- failed to start


	*/

	typedef void (*OnCompletedType)(PVOID param, DWORD res);

	BOOL CollectAndZip(const wstring & pwd, wchar_t* pszLocation, OnCompletedType pOnCompleted, PVOID param);

	/*
	
	
	sync function. 
	
	return value:

	true	--		canceled
	false	--		can't be canceled
	
	*/
	BOOL Cancel();


	/*


	sync function. 


	parameter:

	status	--	refer to WORKING_STATUS

	return value:

	true	--		query succeed
	false	--		failed

	*/
	typedef enum
	{
		E_IDLE,
		E_COLLECT_AND_ZIPPING
	}WORKING_STATUS;
	BOOL QueryState(WORKING_STATUS& status);


	typedef enum
	{
		E_OK,
		E_ERROR
	}DIAGNSOTIC_ERROR_CODE;

private:

	
	/*

	copy folder

	pszSrc		 --	 source folder full name, its format is L"c:\\source", not L"c:\\source\\"
	pszDst		--	dest folder full name, its format is L"c:\\source", not L"c:\\source\\"

	*/

	static BOOL CopyFolder(const wchar_t* pszSrc, const wchar_t* pszDst);
	static BOOL CCollectZipLog::CopyFolderByShell(const wchar_t* pszSrc, const wchar_t* pszDst);

	//	zip file function exported from zip_adapter.dll
	typedef int	(*ZipFileType)(const vector<wstring>& vFileSrc, const wchar_t* strZipFile);

	
	
	//	be called when collect and zipping is finished
	OnCompletedType m_pOnCompletedFunc;
	PVOID m_pOnCompletedParam;

	//	used in collecting and zipping
	wstring m_pwd;
	wstring m_Location;

	//	thread object
	CThreadHelper* m_pThreadHelper;

	/*
	
	thread function to collect and zipping logs.
	
	*/
	static DWORD  WINAPI ThreadProc(LPVOID lpParameter);

	//	there are two case for the usage of this flag.
	//	
	//	1:
	//	set this flag to true if we already called m_pOnCompletedFunc to notify user collect and zipping have finished.
	//	this is used to sync user cancel operation and notifying user operation.
	//	when user try to cancel collect and zipping process while we already notified user completion event, we will tell
	//	user the cancel can't be succeed.
	//
	//	2:
	//	if user canceled collect and zipping process, we can't notify user collect and zipping is completed. 
	BOOL m_bCompletedOrCanceled;
	CRITICAL_SECTION m_completionOrCanceledCS;
};
