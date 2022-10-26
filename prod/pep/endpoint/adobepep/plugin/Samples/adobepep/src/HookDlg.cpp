//File is copied from //depot/Fate/dev/Emerald_Adobepep_6.5 branch

#include <string>
#include <map>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#pragma warning(push)
#pragma warning(disable: 4819)
#include "PIHeaders.h"
#pragma warning(pop)

#include "policy.h"
#include "utilities.h"
#include "Send.h"

#pragma warning(push)
#pragma warning(disable: 4510 4512 4610)
#include "HookDlg.h"
#pragma warning(pop)

#pragma warning(push)
#pragma warning(disable: 4819)
#include "madCHook_helper.h"
#pragma warning(pop)

using namespace boost;

static CRITICAL_SECTION _cs;
static bool _bIsCapturedByCInternalHookDlg=false;

BOOL CALLBACK EnumAllChildProc(HWND hwnd,LPARAM lParam)  
{
	// true for continue enmu,false for stop and find the OK button
	char wndTittle[MAX_PATH]={0},wndCls[MAX_PATH]={0};	
	GetWindowTextA(hwnd,wndTittle,MAX_PATH-1);
	GetClassNameA(hwnd,wndCls,MAX_PATH-1);

	stEnumParam* param=(stEnumParam*)lParam;
	switch (param->enumFlag)
	{
	case Flag_WndCls:
		{
			if (iequals(param->pszWndCls,wndCls))
			{
				param->hDestWnd=hwnd;
				return false;
			}
			break;
		}
	case Flag_WndTxt:
		{
			if (iequals(param->pszWndtxt,wndTittle))
			{
				param->hDestWnd=hwnd;
				return false;
			}
			break;
		}
	case Flag_WndTxt_And_WndCls:
		{
			if (iequals(param->pszWndtxt,wndTittle) && iequals(param->pszWndCls,wndCls))
			{
				param->hDestWnd=hwnd;
				return false;
			}
			break;
		}
	}

	return true;
}

typedef LRESULT (_stdcall *SendMessageWFunc)(HWND hWnd,UINT Msg,WPARAM wParam,LPARAM lParam	);
SendMessageWFunc next_SendMessageW=NULL;
LRESULT _stdcall mySendMessageW(HWND hWnd,UINT Msg,WPARAM wParam,LPARAM lParam)
{
	bool isSign=false;

	EnterCriticalSection(&_cs);
	isSign=_bIsCapturedByCInternalHookDlg;
	LeaveCriticalSection(&_cs);

	if (isSign)
	{
		CInternalHookDlg::Instance()->FilterMsgAndInvokePolicyContorller(hWnd,Msg,wParam,lParam);
		return next_SendMessageW(hWnd,Msg,wParam,lParam);
		
	}

	return next_SendMessageW(hWnd,Msg,wParam,lParam);
}

typedef LRESULT (_stdcall *PostMessageWFunc)(HWND hWnd,UINT Msg,WPARAM wParam,LPARAM lParam	);
PostMessageWFunc next_PostMessageW=NULL;
LRESULT _stdcall myPostMessageW(HWND hWnd,UINT Msg,WPARAM wParam,LPARAM lParam)
{
	bool isSign=false;
	
	EnterCriticalSection(&_cs);
	isSign=_bIsCapturedByCInternalHookDlg;
	LeaveCriticalSection(&_cs);
	
	if (isSign)
	{
		CInternalHookDlg::Instance()->FilterMsgAndInvokePolicyContorller(hWnd,Msg,wParam,lParam);
		return next_PostMessageW(hWnd,Msg,wParam,lParam);
	}

	return next_PostMessageW(hWnd,Msg,wParam,lParam);
}

CInternalHookDlg* CInternalHookDlg::_Instance = NULL;
CInternalHookDlg* CInternalHookDlg::Instance()
{
	if (0 == _Instance)
	{	
		_Instance=new CInternalHookDlg;
		//The Hook must false,until Called BuildRTE;
		_bIsCapturedByCInternalHookDlg=false;
		InitializeCriticalSection(&_cs);
		HookAPI("user32.dll","SendMessageW",mySendMessageW,(PVOID*)&next_SendMessageW);
		HookAPI("user32.dll","PostMessageW",myPostMessageW,(PVOID*)&next_PostMessageW);
	}

	return _Instance;
}

bool CInternalHookDlg::FilterMsgAndInvokePolicyContorller(HWND& hWnd,UINT& Msg,WPARAM& wParam,LPARAM& lParam)
{
	bool bModified=false;
	if (Msg==WM_COMMAND)
	{
		if ( (hWnd==m_hDlgWnd && wParam==1) ||											//default push button
			(hWnd==m_hDlgWnd && wParam==0 && lParam==(LPARAM)m_hOKButton) ||			//focus on Ok_btn and press Enter
			(hWnd==m_hParentForOkBtn && wParam==0 && lParam==(LPARAM)m_hOKButton)		//bn_click , accelerator key 
			) 
		{
			if (iequals(HD_STR_SPLIT_DOCUMENT_11,m_strDlgTitle.c_str()) || iequals(HD_STR_DISTRIBUTE_FORM,m_strDlgTitle.c_str()) || iequals(HD_STR_EXTRACT_PAGES,m_strDlgTitle.c_str()))
			{
				std::string strCurPDF;
				getCurrentPDFPath(strCurPDF);
				
				if (!strCurPDF.empty())
				{
					bool bdeny=false;
					CPolicy::GetInstance()->queryLocalSourceAndDoTagObligation(strCurPDF.c_str(),bdeny,"CONVERT");
					if (bdeny)
					{
						Msg=WM_NULL;
						bModified=true;
						return bModified;
					}
				}
			}
		}
	}

	return bModified;
}


HWND CInternalHookDlg::GetOKButtonHandle()
{
	stEnumParam param={m_strOkBtnTitle.c_str(),"Button",Flag_WndTxt_And_WndCls,NULL};
	EnumChildWindows(m_hDlgWnd,EnumAllChildProc,(LPARAM)&param);
	
	if (m_isFormDistribute && param.hDestWnd == NULL)
	{
		param.pszWndtxt=HD_STR_CONTINUEBTN_DISTRIBUTE_FORM;
		EnumChildWindows(m_hDlgWnd,EnumAllChildProc,(LPARAM)&param);	
	}
	
	return param.hDestWnd?param.hDestWnd:(HWND)INVALID_HANDLE_VALUE;
}

HWND CInternalHookDlg::GetDestCntrlHandle()
{
	return NULL;
}

bool CInternalHookDlg::GetOkButtonTitle()
{
	if(iequals(HD_STR_DISTRIBUTE_FORM,m_strDlgTitle.c_str()))
	{
		extern AVTVersionNumPart MajorVersion;

		if (10 == MajorVersion || 9 == MajorVersion )
		{
			m_strOkBtnTitle.assign(HD_STR_OKBTN_DISTRIBUTE_FORM_V9_V10);
			return true;
		}
		if (11 == MajorVersion)
		{
			m_isFormDistribute = true;
		}
		m_strOkBtnTitle.assign(HD_STR_OKBTN_DISTRIBUTE_FORM);

		return true;
	}

	if(iequals(HD_STR_EXTRACT_PAGES,m_strDlgTitle.c_str()))
	{
		m_strOkBtnTitle.assign(HD_STR_OKBTN_EXTRACT_PAGES);
		return true;
	}


	if(iequals(HD_STR_SPLIT_DOCUMENT_11,m_strDlgTitle.c_str()))
	{
		m_strOkBtnTitle.assign(HD_STR_OKBTN_SPLIT_DOCUMENT_11);
		return true;
	}
	return false;
}
	
bool CInternalHookDlg::BuildRTE(HWND hWnd,char* szWndTittle)
{
	m_hDlgWnd=hWnd;
	m_strDlgTitle.assign(szWndTittle);
	GetOkButtonTitle();//to Find handle of OkBtn,need title name and class name.
	m_hOKButton=GetOKButtonHandle();

	m_hParentForOkBtn=GetParent(m_hOKButton);
	m_hDestCntrl=GetDestCntrlHandle();

	return true;
}

void CInternalHookDlg::ClearRTE()
{
	m_hDlgWnd=NULL;
	m_strDlgTitle.clear();
	m_hOKButton=NULL;
	m_strOkBtnTitle.clear();
	m_hParentForOkBtn=NULL;
	m_hDestCntrl=NULL;
	m_isFormDistribute=false;
}


//this Module's Entry Point
void OnSpecificShowWindow(HWND hWnd,char* szWndTittle,int nCmdShow)  //Specific means this funcion just handle spec title 
{
	//only occur for specific Wnd's Show_Cmd
	if(NULL!=szWndTittle && (0 == strcmp(szWndTittle,HD_STR_SPLIT_DOCUMENT_11) || 0 == strcmp(szWndTittle,HD_STR_DISTRIBUTE_FORM) || 0 == strcmp(szWndTittle,HD_STR_EXTRACT_PAGES)))
	{
		switch (nCmdShow)
		{
		case SW_SHOW:
			{
				CInternalHookDlg::Instance()->BuildRTE(hWnd,szWndTittle);

				EnterCriticalSection(&_cs);
				_bIsCapturedByCInternalHookDlg=true;
				LeaveCriticalSection(&_cs);
				
				break;
			}
		case SW_HIDE:
			{
				EnterCriticalSection(&_cs);
				_bIsCapturedByCInternalHookDlg=false;
				LeaveCriticalSection(&_cs);
				
				CInternalHookDlg::Instance()->ClearRTE();//

				break;
			}
		}
	}
}

