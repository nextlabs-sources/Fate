#include "stdafx.h"
#include "configure.h"
#include <fstream>
#include <algorithm>
#pragma comment(lib, "shlwapi.lib") 


CComfigureImpl::CComfigureImpl()
{
	hComfigureFile = NULL ;
} ;
CComfigureImpl::CComfigureImpl(const wchar_t *pszFileName,const wchar_t* pszPath )
{
	strFileName = const_cast<wchar_t*>(pszFileName) ;
	strPath = const_cast<wchar_t*>( pszPath ) ;
	hComfigureFile = NULL ;
} ;
CComfigureImpl::~CComfigureImpl()
{
	if( hComfigureFile )
	{
		CloseHandle( hComfigureFile ) ;
	}
} ;
/*
If the file name and path has supported by the constructure function, It can be set as NULL
*/
BOOL CComfigureImpl::CheckConfigureFile( const wchar_t* pszFileName, const  wchar_t* pszPath  ) 
{
	BOOL bRet = FALSE ;
	if( (pszFileName == NULL )&&(strFileName.empty() ))
	{   
		/*
		Configure file name is NULL, check the parameter is OK...
		*/
		return bRet ;
	}
	if( pszFileName )
	{
		strFileName = pszFileName ; 
	}
	if( (pszPath == NULL )&&(strPath.empty() ))
	{   
		/*
		Configure file path is NULL, 
		Open the file in the current path
		*/			 
		std::wstring strCurPath ;
		
		wchar_t buffer[MAX_PATH * 2 + 1] = {0};
		GetModuleName(buffer, MAX_PATH * 2, g_hMod);
		wchar_t* p = (wchar_t*)wcsrchr(buffer, '\\');
		if(p)
		{
			*(p + 1) = '\0';
		}
		strCurPath = std::wstring(buffer);

		if( strCurPath.empty() )
		{
			return bRet ;
		}
		strCurPath = strCurPath + strFileName ;
		bRet = GetINIInformation( strCurPath ) ;
		return bRet ;
	}
	if( pszPath )
	{
		strPath = pszPath    ;
	}
	if( PathIsRelative( strPath.c_str() ) )
	{
		std::wstring strCurPath ;
		wchar_t buffer[MAX_PATH * 2 + 1] = {0};
		GetModuleName(buffer, MAX_PATH * 2, g_hMod);
		wchar_t* p = (wchar_t*)wcsrchr(buffer, '\\');
		if(p)
		{
			*(p + 1) = '\0';
		}
		strCurPath = std::wstring(buffer) ;

		if( strCurPath.empty() )
		{
			return bRet ;
		}
		wchar_t lpszDest[MAX_PATH+1]= {0};;
		if( PathCombineW( lpszDest,   strCurPath.c_str() ,strPath.c_str() ) != NULL )
		{
			if( lpszDest )
			{
				strPath = lpszDest;
				std::wstring strTemp = strPath+ L"\\"+ strFileName ;
				bRet = GetINIInformation( strTemp ) ;
			}
		}
		return bRet ;
	}
	std::wstring strTemp = strPath+ strFileName ;
	bRet = GetINIInformation( strTemp ) ;
	return bRet ;
};

BOOL CComfigureImpl::ImplementConfigureFile( std::wstring strFullName )
{
	BOOL bRet = TRUE ;
	struct _stat64i32 fileStatus;
	if(_wstat(strFullName.c_str(), &fileStatus) == -1)
	{
		bRet = FALSE ;
	}
	return bRet ;
};

BOOL CComfigureImpl::GetINIInformation( std::wstring strFileName) 
{
	BOOL bRet = TRUE ;
	if( (bRet = ImplementConfigureFile(	 strFileName )) == TRUE)
	{
		std::wifstream file ;
		file.open(strFileName.c_str());
		if(!file)
		{
			return FALSE ;
		}
		if( file.fail() )
		{
			return FALSE ;
		}
		wchar_t buffer[BUFFER_SIZE] = {0} ;
		while( !file.eof())
		{
			file.getline(  buffer, BUFFER_SIZE )  ;
			if( wcsstr( buffer, L"=" ) )
			{
				size_t buffLen = wcslen(buffer) ; 
				if((buffLen > 0) && buffer[buffLen-1] == '\r') {
					buffer[buffLen-1] = '\0';
				}
				std::wstring temp = buffer ;
				std::wstring::size_type index = temp.find( '=' ) ;
				if( index != std::wstring::npos) 
				{
					INIINFORM info ;
					info.first = temp.substr(0, index) ;
					transform(   info.first.begin(), info.first.end(), info.first.begin(), toupper    ) ;
					info.second = temp.substr(index + 1)	 ;
					transform(   info.second.begin(), info.second.end(), info.second.begin(), toupper    ) ;
					m_IniInform.push_back(info) ; 
				}
			}
		}
		file.close() ;
	}
	return  bRet ;
}

BOOL CComfigureImpl::CheckHookAll(VOID)
{
	BOOL bRet = FALSE ;
	std::list<INIINFORM>::iterator iter = m_IniInform.begin() ;
	for( ; iter != m_IniInform.end() ; iter ++ )
	{
		std::wstring::size_type index = (*iter).first.find( L"HOOKALL" ) ; 
		if( index != std::wstring::npos) 
		{
			std::wstring::size_type idx = (*iter).second.find( L"NO" ) ; 
			if( idx != std::wstring::npos )
			{
				bRet = FALSE ;
			}
			else
			{
				bRet = TRUE ;
			}
			break ;
		}
	}
	return bRet ;
}
BOOL CComfigureImpl::IsSupportApp( std::wstring strApp )
{
	BOOL bRet = FALSE ;
	std::list<INIINFORM>::iterator iter = m_IniInform.begin() ;
	for( ; iter != m_IniInform.end() ; iter ++ )
	{
		std::wstring::size_type index = (*iter).first.find( L"HOOK " ) ; 
		if( index != std::wstring::npos) 
		{
			transform(   strApp.begin(), strApp.end(), strApp.begin(), toupper    ) ;
			std::wstring::size_type idx = (*iter).second.find( strApp ) ; 
			if( idx != std::wstring::npos) 
			{
				bRet = TRUE ;
			}
			else
			{
				bRet = FALSE ;
			}
			break ;
		}
	}
	return bRet ;
}
BOOL CComfigureImpl::IsHookEmpty() 
{
	BOOL bRet = TRUE ;
	std::list<INIINFORM>::iterator iter = m_IniInform.begin() ;
	for( ; iter != m_IniInform.end() ; iter ++ )
	{
		std::wstring::size_type index = (*iter).first.find( L"HOOK " ) ; 
		if( index != std::wstring::npos) 
		{
			bRet = (*iter).second.empty() ;
			break ;
		}
	}
	return bRet ;
}
BOOL CComfigureImpl::IsNotSupportApp( std::wstring strApp )
{
   	BOOL bRet = FALSE ;
	std::list<INIINFORM>::iterator iter = m_IniInform.begin() ;
	for( ; iter != m_IniInform.end() ; iter ++ )
	{
		std::wstring::size_type index = (*iter).first.find( L"NOT HOOK" ) ; 
		if( index != std::wstring::npos) 
		{
			transform(   strApp.begin(), strApp.end(), strApp.begin(), toupper    ) ;
			std::wstring::size_type idx = (*iter).second.find( strApp ) ; 
			if( idx != std::wstring::npos) 
			{
				bRet = TRUE ;
			}
			else
			{
				bRet = FALSE ;
			}
			break ;
		}
	}
	return bRet ;
}
