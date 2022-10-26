#ifndef __MAINFRAME_UI_INTERFACE_H__
#define __MAINFRAME_UI_INTERFACE_H__



namespace pafUI
{
	/*
	define the callback function for the button
	PVOID: 
		It is the pointer which the PEP wants to push it.
	status:
		The common dialog will set this value of current dialog status.
	_hParent:
		It is the handle of common dialog.
	*/
	typedef void (CALLBACK* ONCLICKNEXTBT)( PVOID ,LONG status, HWND _hParent ) ;
	typedef void (CALLBACK* ONCLICKCANCELBT)(PVOID, HWND _hParent) ;
	/*
	use these string to get message ID by this fucntion of.
	UINT ID = RegisterWindowMessage( PAF_UI_MAINWINDOW_STATUS ) ;
	Post this message ID to the main window.
	*/
	/*
		Such as:
		UINT ID = RegisterWindowMessage( PAF_UI_MAINWINDOW_STATUS ) ;
		::PostMessage( hWnd, ID, (WPARAM)DLGSTATUS::DS_NEXT, (LPARAM)string ) ;
		
	*/
#define PAF_UI_MAINWINDOW_STATUS		L"Dialog Status"

//#define PAF_UI_MAINWINDOW_NEXT		L"NEXT STATUS"
	/*
	BTSTATUS
	WPARAM:
		0: Enable.
		1: Disable.
	string :
	 it will show in the button.
	Such as:
		UINT ID = RegisterWindowMessage( PAF_UI_MAINWINDOW_STATUS ) ;
		::PostMessage( hWnd, ID, (WPARAM)BTSTATUS::BT_ENABLE, (LPARAM)string ) ;
	LPARAM: should define same rules to check which button will changed.
	*/
#define PAF_UI_BTTONSATUS			L"Button Status"
	/*
	define the status value of the button click...
	*/
	typedef enum _tagDLGSTATUS
	{
		DS_NEXT,
		DS_OK,
		DS_FINISHED,
		DS_SENDMAIL,
		DS_DEFAULT
	}*PDLGSTATUS,DLGSTATUS ;

	typedef enum _tagBTSTATUS
	{
		BT_ENABLE,
		BT_DISABLE,
		BT_HIDE
	}*PBTSTATUS,BTSTATUS  ;

	/*
	Register the message to End the progress bar...
	UINT iMsg = ::RegisterWindowMessage( PAF_UI_END_PROGRESSBAR );
	::SendMessage( hParent,iMsg,0,0 ) ;
	*/
#define PAF_UI_END_PROGRESSBAR	L"End ProgressBar"

};

extern "C"
{
	typedef LONG UI_STATUS  ;
	/*
	Add a method to GetParentWindow before create the subwindow of PA.;
	*/
	UI_STATUS WINAPI GetParentWindow(HWND &_hWnd,const HWND _hParent = NULL) ;
	/*
	Create the main window of the frame.
	_hChildWnd:
		This is the window's handle of the PA...
	_bIsModel:
		Check if the dialog is model or modeless.
	_statu:
		Set the first status of the dialog.
	_hParent:
		If this window has a parent.
	_pszTitle:
		The title string of the dialog..
	_pszDescription:
		If there has a string to show in the panel,
	*/
	INT_PTR  WINAPI CreateMainFrame(		const HWND _hChildWnd = NULL ,
										const BOOL _bIsModel = TRUE, 
										const wchar_t* _pszHelpURL = NULL ,
										const wchar_t* _strBtName= NULL,
										const pafUI::BTSTATUS _BTStatu = pafUI::BT_ENABLE,
										const HWND _hParent = NULL,
										const wchar_t* _pszTitle = NULL, 
										const wchar_t* _pszDescription = NULL )  ;
	/*
		Change the Panel of PA....
	*/
	UI_STATUS WINAPI Change_PA_Panel(	
										const HWND _hParent = NULL,
										const HWND _hNewPanelWnd = NULL,
										const wchar_t* _pszHelpURL = NULL ,
										const wchar_t* _pszTitle= NULL, 
										const wchar_t* _pszDescription = NULL ) ;
	/*
	Release the resource 
	*/
	UI_STATUS WINAPI ReleaseMainFrame( const HWND _hParent = NULL ) ;
	/*
	For the progress bar...
	Show & End Progress bar
	_pszTitleName:
				It is the pa's title,which will show the top in the panel of PA.
	_pszDescrption:
				It is the description which will be show upper in the progress bar.
	*/
	UI_STATUS WINAPI ShowProgressBar(	
										const HWND _hParent = NULL,
										const wchar_t * _pszTitleName = NULL ,
										const wchar_t * _pszDescrption = NULL 
									);
	UI_STATUS WINAPI EndProgressBar( const HWND _hParent = NULL ) ;

	//UI_STATUS WINAPI ShowMainFrame( BOOL _bDoModel ) ;
	/*
	this is the call back function the UI wants to do....
	status:	0: Next
			1: OK
	void OnClickButton( pafUI::DLGSTATUS status ) ;
	*/
	UI_STATUS  WINAPI SetNEXT_OKCallBack( /*CALLBACK* pFunc*/pafUI::ONCLICKNEXTBT pFunc ,PVOID _pData,const HWND _hParent = NULL) ;
	/*
	on click the button of cancel.
	*/
	UI_STATUS  WINAPI SetCancelCallBack( /*CALLBACK* pFunc */pafUI::ONCLICKCANCELBT pFunc,PVOID _pData,const HWND _hParent = NULL) ;

	/*
	To get the last PA window's handle and its created time.
	*/
	UI_STATUS WINAPI GetLastPAWnd(HWND *phWnd, DWORD *pCreatedTime);

	/*
	Show Hierarchical Classification dialog
	*/
	UI_STATUS WINAPI ShowHierarchicalClassifyDlg(HWND hParentWnd, const wchar_t* pszFileName, bool bLastFile, const wchar_t* pszDescript,
		const wchar_t* pszXmlDefine, const wchar_t* pOldTags, wchar_t** szOutAddTagBuf, wchar_t** szOutDelTagBuf);

	/*
	Release buffer
	*/
	void WINAPI ReleaseBuffer(wchar_t* pBuf);

	const WCHAR g_kSepTags = 0x01;
	const WCHAR g_kSepTagVNameAndValue = 0x02;

};

#define ON_CLICK_OK_NEXT(  pFunc, result ) \
		result = SetNEXT_OKCallBack( pFunc ) ;


#define ON_CLICK_CANCEL ( pFunc, result ) \
		result = SetCancelCallBack( pFunc ) ;

#endif
