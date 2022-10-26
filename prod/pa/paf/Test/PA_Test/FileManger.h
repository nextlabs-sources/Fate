#ifndef __FILE_MANAGER_H__
#define __FILE_MANAGER_H__

#include "Policy.h"
#include "PAMngr.h"

#define DO_OUTPUT_STRING( hWnd, szString ) \
	{\
	::SendMessage( hWnd, LB_ADDSTRING, NULL, (LPARAM)szString ) ;\
	LRESULT iCount =   SendMessage( hWnd, LB_GETCOUNT, NULL, NULL ) ;\
	SendMessage( hWnd, LB_SETCARETINDEX, (WPARAM)iCount, (LPARAM)FALSE ) ;\
}
typedef CEString (WINAPIV* CE_AllocStringType)( wchar_t * pszBuf ) ;
typedef CEResult_t (WINAPIV* CEM_FreeStringType) (CEString cestr) ;
class CFileManager
{
public:
	CFileManager(const HINSTANCE hInst = NULL)  ;
	~CFileManager() ;
public:
	BOOL DoAction_Copy( wchar_t *pszSrcFile, wchar_t * pszDestFile )  ;
	BOOL DoAction_Move( wchar_t *pszSrcFile, wchar_t * pszDestFile )  ;

	BOOL DoStub( wchar_t *pszSrcFile, wchar_t *pszDestFile )  ;
	VOID SetOutPutWnd( HWND hWnd ) ;
	
public:
	static PVOID m_fnGetCEString ;
	static PVOID m_fnAllocCEString ;
	static PVOID m_fnFreeCEString ;
private:
	HMODULE m_hEnc ;
	HMODULE m_hTag ;
	CPolicyComm m_policyComm ;
	HWND m_hOutWnd ; 
	HMODULE m_hCEType ;
	
};
#endif