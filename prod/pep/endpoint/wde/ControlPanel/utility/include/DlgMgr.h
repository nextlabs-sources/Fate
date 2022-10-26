#pragma once

#include <windows.h>
#include <vector>



class CDlgMgr
{
public:

	CDlgMgr(void)
	{
	}

	virtual ~CDlgMgr(void)
	{
	}


	/*

	show modal dialog.

	*/

	typedef enum
	{
		E_NO_PERMISSION_START_PC,
		E_NO_PERMISSION_SET_LOG
	}MODAL_DLG_ID;
	virtual BOOL ShowModalDlg(MODAL_DLG_ID eDlgId) = 0;




	/*
	
	registry modaless dialog' handle. as KeyBoard event can't be sent to modaless dialog default, 
	so if user want his dialog' controls can respond to keyboard input, user need to registry the dialog' handle using this method.

	you can refer to method GetDlgHandleForKB for more information.

	note:
	1, if you want a dialog( we call it dialog A)' control can respond to keyboard input, you need registry both A's handle and its parent dialog' handle.
	2, the parent dialog' handle must behind the A' handle in vecHWnds
	*/
	virtual void RegDlgHandleForKB(HWND* pWnds, int nCount) = 0;

	
	/*
	
	get all handles reg by RegDlgHandleForKB, this function is usually called by winMain, in winMain' message loop.



	parameters:
	phWnds	--	array for HWND
	dwCount	--	size of the array, for output, it is the actual value of hwnd

	return value:
	false	--	the memory is not enough to carry all handles out. user should allocate more memories and call again.
	true	--	succeed.
	
	*/
	virtual BOOL GetDlgHandleForKB(HWND* phWnds, DWORD& dwCount) = 0;
};
