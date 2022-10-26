#pragma once

/*
Implement the file of "FTPE.ini"
The format of this file:
[Scope]
HookAll = NO
Hook = SmartFTP, Firefox, 
Not hook = CuteFTP

Detail Implementation:
1.	The value of ¡°HookAll¡± should be ¡°YES¡± or ¡°NO¡±. The values of ¡°hook¡±, ¡°Not hook¡± are process names. Use comma as the separator for multiple process names.
2.	FTPE will impact all processes if FTPE.ini doesn¡¯t exist
3.	If the ¡°HookAll¡± is YES, FTPE will impact all processes. The ¡°Hook¡± and ¡°Not hook¡± are useless.
4.	FTPE will use ¡°Hook¡± if the value of ¡°Hook¡± is not empty, otherwise it will try to use the value of ¡°Not hook¡±. It means only one (¡°Hook¡± or ¡°Not hook¡±) value works at one time.
*/
#include <string>
#include <list>
#include "FilterRes.h"
#include "shlwapi.h"
/*
Key & value
*/
typedef std::pair<std::wstring, std::wstring> INIINFORM;

class CComfigureImpl
{
public:
	CComfigureImpl();
	CComfigureImpl(const wchar_t *pszFileName,const wchar_t* pszPath = NULL);
	virtual ~CComfigureImpl();
public:
	/*
	If the file name and path has supported by the constructure function, It can be set as NULL
	*/
	BOOL CheckConfigureFile( const wchar_t* pszFileName = NULL, const  wchar_t* pszPath = NULL );
	/*
	Check HookAll or not.
	*/
	BOOL CheckHookAll(VOID) ;
	/*
	Check Support Application
	*/
	BOOL IsSupportApp( std::wstring strApp ) ;
	/*
	Check is not Support Application
	*/
	BOOL IsNotSupportApp( std::wstring strApp ) ;
	/*
	Check hook item is empty
	*/
	BOOL IsHookEmpty() ;
protected:
	BOOL CComfigureImpl::ImplementConfigureFile( std::wstring strFullName ) ;
	BOOL GetINIInformation( std::wstring strFileName) ;
private:
	std::wstring strFileName ;
	std::wstring strPath ;
	HANDLE hComfigureFile ;
	std::list<INIINFORM> m_IniInform ;
	const static LONG BUFFER_SIZE = 2048  ;
};
