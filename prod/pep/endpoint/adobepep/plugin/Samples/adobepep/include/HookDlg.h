//File is copied from //depot/Fate/dev/Emerald_Adobepep_6.5 branch

//"view->tools->pages" ->click "split document"
#define HD_STR_SPLIT_DOCUMENT_11		"Split Document"
#define HD_STR_OKBTN_SPLIT_DOCUMENT_11  "&OK"

#define HD_STR_DISTRIBUTE_FORM				"Distribute Form" 
#define HD_STR_OKBTN_DISTRIBUTE_FORM			"OK"
#define HD_STR_CONTINUEBTN_DISTRIBUTE_FORM		"Continue"
#define	HD_STR_OKBTN_DISTRIBUTE_FORM_V9_V10		"Next >"

#define HD_STR_EXTRACT_PAGES		"Extract Pages"
#define HD_STR_OKBTN_EXTRACT_PAGES  "OK"

//External Interface 
void OnSpecificShowWindow(HWND hDlgWnd,char* szWndTittle,int nCmdShow);


//InternalDlg class used for performing specific tasks.
class CInternalHookDlg
{
public:
	static CInternalHookDlg* Instance();
	bool BuildRTE(HWND hWnd,char* szWndTittle);
	void ClearRTE();

	//by current version , filter WM_COMMAND msg
	bool FilterMsgAndInvokePolicyContorller(HWND& hWnd,UINT& Msg,WPARAM& wParam,LPARAM& lParam);\

protected:
	CInternalHookDlg():m_hDlgWnd(NULL),m_hOKButton(NULL),m_hParentForOkBtn(NULL),m_hDestCntrl(NULL),m_isFormDistribute(FALSE){}
	CInternalHookDlg(CInternalHookDlg& cHookDlg);
	CInternalHookDlg& operator = (CInternalHookDlg& cHookDlg);

private:
	HWND GetOKButtonHandle(); //need enum all child wnds
	HWND GetDestCntrlHandle();//need enum all child wnds
	bool GetOkButtonTitle();
	
private:
	static CInternalHookDlg*  _Instance; //Singleton

	HWND	m_hDlgWnd;
	string	m_strDlgTitle;
	HWND	m_hOKButton;
	string  m_strOkBtnTitle;
	HWND	m_hParentForOkBtn;
	HWND	m_hDestCntrl;  //  a Control's window handle that holds the string for pc to evaluation.
	bool    m_isFormDistribute;
};

//helper struct used by EnumChildWindowHandles 
typedef enum _FlagEnumParam
{
	Flag_WndCls				=   0x01,
	Flag_WndTxt				=	0x10,
	Flag_WndTxt_And_WndCls	=	0x11,
}FlagEnumParam;

#pragma  warning(push)
#pragma  warning(disable:4510 4512 4610)
typedef struct tagEnumParam
{
	const char* pszWndtxt;	//in
	const char* const pszWndCls;	//in 
	FlagEnumParam	const enumFlag; //in
	HWND hDestWnd;	//out 
}stEnumParam;
#pragma  warning(pop)
