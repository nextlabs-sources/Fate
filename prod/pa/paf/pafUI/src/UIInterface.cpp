#include "stdafx.h"
#include "UIInterface.h"
#include "mainFrameDlg.h"
#include "global.h"
#include <list>
#include "dlgclassify2.hpp"
/*
create and initialzie the window of main frame
*/
std::list<CMainFrame *> g_listMainFrame ;
/*
added by chellee 11/11/2008 for the	 release window
*/
//-------------------------------------------------------------------------
DWORD WINAPI ReleaseListWindow( PVOID pFlag)
{
	DWORD dRet = 0 ;
	BOOL bRemove = FALSE ;
	
	while( TRUE)
	{
		std::list<CMainFrame *>::iterator itor =  g_listMainFrame.begin() ;
		for( itor; itor != g_listMainFrame.end() ; itor ++ )
		{	
			if( !IsWindow( (*itor)->m_hWnd ) &&(*itor)->get_PAObjectPtr()!= pFlag )
			{
				g_listMainFrame.erase(itor)	 ;
				break ;
			} 
		}
		if(	(bRemove == FALSE))
		{
			if( itor == g_listMainFrame.end() ) 
			{
				return dRet = 1 ;
			}
		}
	}

	return dRet ;

}
//-------------------------------------------------------------------------
/*
Modified by chellee for the WDE parent window...
Right now the modification is :
The	 _hParent has been pushed at the set_Next_callback...
CreateMainFrame & Change_PA_Panel
Right now the _hParent is used to set a pointor as a flag(which will be set at the set_Next_callback( ...this(pData).. ) 
*/
INT_PTR  WINAPI CreateMainFrame(const HWND _hChildWnd ,
							  const BOOL _bIsModel ,
							  const wchar_t* _pszHelpURL,
							  const wchar_t* _strBtName ,
							  const pafUI::BTSTATUS _BTStatu ,
							  const HWND _hParent,
							  const wchar_t* _pszTitle, 
							  const wchar_t* _pszDescription  ) 
{	
    INT_PTR dRet = IDCANCEL ;
	BOOL bNeedNew = TRUE ;

	std::list<CMainFrame *>::iterator itor =  g_listMainFrame.begin() ;
	for( itor; itor !=	g_listMainFrame.end() ; itor ++ )
	{
		if( (*itor)->get_PAObjectPtr() == (PVOID)_hParent )
		{
			bNeedNew = FALSE ;
			dRet = (*itor)->CreateMainWindow(_hChildWnd,_bIsModel,_pszHelpURL,_strBtName,_BTStatu,_hParent,_pszTitle,  _pszDescription ) ;
			break ;
		}
	}
	if( bNeedNew == TRUE ) 
	{
		CMainFrame * MainFrame = new	 CMainFrame ;
		//MainFrame->m_hParent =_hParent ;
		dRet = MainFrame->CreateMainWindow(_hChildWnd,_bIsModel,_pszHelpURL,_strBtName,_BTStatu,_hParent,_pszTitle,  _pszDescription ) ;
		g_listMainFrame.push_back( MainFrame ) ;

	}
	return	dRet ;
}
/*
Finished all policy assist and show the finished window.

*/
UI_STATUS  WINAPI ReleaseMainFrame( const HWND _hParent ) 
{
	UI_STATUS status = 0 ;
	std::list<CMainFrame *>::iterator itor =  g_listMainFrame.begin() ;
	for( itor; itor !=	g_listMainFrame.end() ; itor ++ )
	{
		if( (*itor)->m_hParent == _hParent )
		{
			if( (*itor) ) 
			{
				if( ::IsWindow((*itor)->m_hWnd) )
					(*itor)->ReleaseMainWnd() ;
			}
			//delete (*itor) ;
			g_listMainFrame.erase(itor)	 ;
		}
	}
	return status ;

}
UI_STATUS  WINAPI SetNEXT_OKCallBack( /*CALLBACK* pFunc*/pafUI::ONCLICKNEXTBT pFunc, PVOID _pData,const HWND _hParent )
{
	UI_STATUS status = 0 ;
	BOOL bNeedNew = TRUE ;
	ReleaseListWindow( _pData ) ;
	std::list<CMainFrame *>::iterator itor =  g_listMainFrame.begin() ;
	for( itor; itor !=	g_listMainFrame.end() ; itor ++ )
	{
		if( (*itor)->get_PAObjectPtr() == _pData )
		{
			bNeedNew = FALSE ;
			(*itor)->SetNext_OKCallBack( pFunc,_pData  ) ;
			break ;
		}
	}
	if( bNeedNew == TRUE ) 
	{
		CMainFrame * MainFrame = new	 CMainFrame ;
		MainFrame->m_hParent =_hParent ;
		 MainFrame->SetNext_OKCallBack( pFunc,_pData  ) ;
		g_listMainFrame.push_back( MainFrame ) ;
	}
	return status ;
}
UI_STATUS  WINAPI SetCancelCallBack( /*CALLBACK* pFunc*/pafUI::ONCLICKCANCELBT pFunc, PVOID _pData,const HWND _hParent)
{
	UI_STATUS status = 0 ;
	BOOL bNeedNew = TRUE ;
	std::list<CMainFrame *>::iterator itor =  g_listMainFrame.begin() ;
	for( itor; itor !=	g_listMainFrame.end() ; itor ++ )
	{
		if( (*itor)->get_PAObjectPtr() == _pData )
		{
			bNeedNew = FALSE ;
			(*itor)->SetCancelCallBack( pFunc,  _pData) ;
			break ;
		}
	}
	if( bNeedNew == TRUE ) 
	{
		CMainFrame * MainFrame = new	 CMainFrame ;
		MainFrame->m_hParent =_hParent ;
		MainFrame->SetCancelCallBack( pFunc,  _pData) ;
		g_listMainFrame.push_back( MainFrame ) ;
	}
	return status ;
} 
UI_STATUS WINAPI Change_PA_Panel( 
								 const HWND _hParent,
								 const HWND _hNewPanelWnd ,
								 const wchar_t* _pszHelpURL,
								 const wchar_t* _pszTitle ,
								 const wchar_t* _pszDescription  ) 
{
	UI_STATUS status = 0 ; 

	std::list<CMainFrame *>::iterator itor =  g_listMainFrame.begin() ;
	for( itor; itor !=	g_listMainFrame.end() ; itor ++ )
	{
		if( (*itor)->get_PAObjectPtr() == (PVOID)_hParent )
		{
			(*itor)->Change_PA_Panel( _hNewPanelWnd, _pszHelpURL, _pszTitle,_pszDescription  ) ;

			break ;
		}
	}
	return status ;
}
UI_STATUS WINAPI GetParentWindow(HWND &_hWnd,const HWND _hParent)
{
	UI_STATUS status = 0 ; 
	BOOL bNeedNew = TRUE ;
	std::list<CMainFrame *>::iterator itor =  g_listMainFrame.begin() ;
	for( itor; itor !=	g_listMainFrame.end() ; itor ++ )
	{
		if( (*itor)->m_hParent == _hParent )
		{
			bNeedNew = FALSE ;
			_hWnd = (*itor)->GetWindowHandle() ;
			break ;
		}
	}
	if( bNeedNew == TRUE ) 
	{
		CMainFrame * MainFrame = new	 CMainFrame ;
		MainFrame->m_hParent =_hParent ;
		_hWnd =  MainFrame->GetWindowHandle() ;
		g_listMainFrame.push_back( MainFrame ) ;
	}
	return status ;
}

UI_STATUS WINAPI ShowProgressBar( const HWND _hParent, const wchar_t * _pszTitleName  ,const wchar_t * _pszDescrption )
{
	UI_STATUS statu = 0 ;
	std::list<CMainFrame *>::iterator itor =  g_listMainFrame.begin() ;
	for( itor; itor !=	g_listMainFrame.end() ; itor ++ )
	{
		if( (*itor)->m_hParent == _hParent )
		{

			(*itor)->ShowProgressPanel( _pszTitleName, _pszDescrption ) ;
		}
	}

	return statu ;
}
UI_STATUS WINAPI EndProgressBar( const HWND _hParent ) 
{
	UI_STATUS statu = 0 ;
	std::list<CMainFrame *>::iterator itor =  g_listMainFrame.begin() ;
	for( itor; itor !=	g_listMainFrame.end() ; itor ++ )
	{
		if( (*itor)->m_hParent == _hParent )
		{

			(*itor)->EndProgressPanel( ) ;
		}
	}
	return statu ;
}

UI_STATUS WINAPI GetLastPAWnd(HWND *phWnd, DWORD *pCreatedTime)
{
	UI_STATUS status = 0;

	if (!g_listMainFrame.empty())
	{
		CMainFrame *mainFrame = g_listMainFrame.back();
		if (phWnd != NULL)
			*phWnd = mainFrame->m_hWnd;
		if (pCreatedTime != NULL)
			*pCreatedTime = mainFrame->m_dwCreatedTime;
	}

	return status;
}

void ParseTagsFromString(const wchar_t* wszTags, std::vector<std::pair<std::wstring, std::wstring>>& vTags)
{
	if (wszTags)
	{
		std::wstring wstrTags = wszTags;
		size_t nCurrentPos = 0;
		while (nCurrentPos < wstrTags.length())
		{
			const size_t nPosTagSep = wstrTags.find(g_kSepTags, nCurrentPos);
			const std::wstring wstrTag = wstrTags.substr(nCurrentPos, nPosTagSep == std::wstring::npos ? nPosTagSep : (nPosTagSep - nCurrentPos));
			const size_t nPosTagValue = wstrTag.find(g_kSepTagVNameAndValue);
			if (nPosTagValue != std::wstring::npos)
			{
				std::wstring wstrName = wstrTag.substr(0, nPosTagValue);
				std::wstring wstrValue = wstrTag.substr(nPosTagValue + 1);
				vTags.push_back(std::pair<std::wstring, std::wstring>(wstrName, wstrValue));
			}

			//find next
			if (nPosTagSep==std::wstring::npos)
			{
				break;
			}
			else
			{
				nCurrentPos = nPosTagSep + 1;
			}
		}
	}	
}

UI_STATUS WINAPI ShowHierarchicalClassifyDlg(HWND hParentWnd, const wchar_t* pszFileName, bool bLastFile, const wchar_t* pszDescript,
	                                         const wchar_t* pszXmlDefine, const wchar_t* pOldTags, wchar_t** szOutAddTagBuf, wchar_t** szOutDelTagBuf)
{
	//format the exist file tags.
	std::vector<std::pair<std::wstring, std::wstring>> vOldTags;
	if (pOldTags!=NULL)
	{
		ParseTagsFromString(pOldTags, vOldTags);
	}
	
	//show dialog
	CDlgClassify2 dlgClassify(pszFileName, bLastFile, pszDescript, pszXmlDefine, L"default", vOldTags);
	int nRet = dlgClassify.DoModal(hParentWnd==NULL ? GetForegroundWindow() : hParentWnd);

	if (nRet==IDOK)
	{
		//get result
		std::vector<std::pair<std::wstring, std::wstring>> tags;
		dlgClassify.GetClassificationTags(tags);

		std::wstring strResult;
		std::vector<std::pair<std::wstring, std::wstring>>::iterator itTag = tags.begin();
		while (itTag != tags.end())
		{
			strResult += itTag->first;
			strResult += L"=";
			strResult += itTag->second;
			strResult += L";";
			itTag++;
		}

		wchar_t* pTag = new wchar_t[(wcslen(strResult.c_str()) + 1)*sizeof(wchar_t)];
		wcscpy_s(pTag, wcslen(strResult.c_str()) + 1, strResult.c_str());
		*szOutAddTagBuf = pTag;

		//get tag that need to be delete
		std::wstring wstrDelTags;
		std::vector<std::pair<std::wstring, std::wstring>>::iterator itOldTag = vOldTags.begin();
		while (itOldTag != vOldTags.end())
		{
			if (dlgClassify.TagExist(itOldTag->first.c_str()))
			{
				wstrDelTags += itOldTag->first;
				wstrDelTags += g_kSepTags;
			}
			itOldTag++;
		}
		wchar_t* pDelTag = new wchar_t[wstrDelTags.length() + 1];
		wcscpy_s(pDelTag, wstrDelTags.length() + 1, wstrDelTags.c_str());
		*szOutDelTagBuf = pDelTag;
		
	}
	return nRet;
}

void WINAPI ReleaseBuffer(wchar_t* pBuf)
{
	delete[] pBuf;
}
