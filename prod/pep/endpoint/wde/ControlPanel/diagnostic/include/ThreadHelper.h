#pragma once

#include "MutexHelper.h"

/*

each CThreadHelper object is a thread.

*/
class CThreadHelper
{
public:

	typedef DWORD  (WINAPI *ThreadProcType)(LPVOID lpParameter);


	/*
	
	parameter:

	pThreadProc		--	user specified thread function.
	pParam				--	user specified lpParameter in user specified thread function.
	
	*/
	CThreadHelper(ThreadProcType pThreadProc, PVOID pParam);


	/*
	
	you should query running status by calling BOOL IsRunning();

	you can ONLY delete CThreadHelper object when it is not running.
	
	
	*/
	virtual ~CThreadHelper(void);

	
	/*
	
	create a suspended thread.
	
	*/
	BOOL Create();


	/*
	
	resume the created suspended thread.
	
	*/
	BOOL Resume();



	/*

	kill the thread.


	kill thread is a very dangerous calling, it will cause:

	If the thread owns a critical section, the critical section will not be released. 
	If the thread is allocating memory from the heap, the heap lock will not be released. 
	If the thread is executing certain kernel32 calls when it is terminated, the kernel32 state for the thread's process could be inconsistent. 
	If the thread is manipulating the global state of a shared DLL, the state of the DLL could be destroyed, affecting other users of the DLL. 


	return value:

	true	--	kill succeed
	false	--	can't kill or the thread is already exit before try to kill

	*/
	BOOL Kill();


	/*
	
	query if thread is running.

	return :

	true	--	running
	false	--	not running
	
	*/
	BOOL IsRunning();

private:

	//	user defined thread function and thread function parameter
	ThreadProcType m_pThreadProc;
	PVOID m_pParam;

	//	this mutex is target to keep every method as a atomic operation
	Mutex m_Mutex;

	/*
	
	our thread function.

	call user defined thread function in our thread function.
	
	*/	
	static DWORD WINAPI my_ThreadProc(LPVOID lpParameter);


	//	thread handle
	HANDLE 	m_handle;
	//	thread id
	DWORD m_tid;
	//	is thread running
	BOOL m_bRunning;


	HANDLE m_event;
};
