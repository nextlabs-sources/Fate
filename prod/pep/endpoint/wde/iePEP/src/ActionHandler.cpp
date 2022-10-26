#include "ActionHandler.h"
#include "celog.h"

#define CELOG_CUR_MODULE L"iePEP"
#define CELOG_CUR_FILE CELOG_FILEPATH_PROD_PEP_ENDPOINT_WDE_IEPEP_SRC_NL_ActionHandler_CPP
//////////////////////////////////////////////////////////////////////////
//CActionHandler

extern unsigned int g_ip;

CActionHandler::CActionHandler ( )
{
	
}

CActionHandler::CActionHandler ( const CActionHandler& CInstance )
{

}

CActionHandler& CActionHandler::operator= ( const CActionHandler& CInstance )
{
	if ( this != &CInstance )
	{

	}

	return *this;
}

CActionHandler::~CActionHandler ( )
{
	
}

CActionHandler* CActionHandler::GetInstance ( )
{

	if ( IEVersion9 == GetIEVersion() || IEVersion10 == GetIEVersion() || IEVersion11 == GetIEVersion())
	{
		return new CIEDefaultActionHandler;
	}

	return NULL;
}

CActionHandler::IEVersionType CActionHandler::GetIEVersion ( )
{
	IEVersionType IEVersion = IEVersionNULL;

	HKEY hKey = NULL;

	LONG lResult = RegOpenKeyExW ( HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Internet Explorer", 0, KEY_QUERY_VALUE, &hKey );

	if ( ERROR_SUCCESS == lResult )
	{;
		WCHAR Buffer[MAX_PATH] = { 0 };
		DWORD cbBuffer = MAX_PATH * sizeof WCHAR;

		lResult = RegQueryValueExW ( hKey, L"Version", 0, NULL, (LPBYTE)Buffer, &cbBuffer );
		
		if ( ERROR_SUCCESS == lResult )	
		{
			int iVersion = _wtoi ( Buffer );

			switch ( iVersion )
			{
			case 6:
				IEVersion = IEVersion6;
				break;

			case 7:
				IEVersion = IEVersion7;
				break;

			case 8:
				IEVersion = IEVersion8;
				break;

			case 9:
				IEVersion = IEVersion9;
				break;

			case 10:
				IEVersion = IEVersion10;
				break;

			case 11:
				IEVersion = IEVersion11;
				break;
			
			default:
				IEVersion = IEVersionNULL;
				break;
			}
		}

		RegCloseKey ( hKey );
	}

	return IEVersion;
}

BOOL CActionHandler::GetCanonicalName ( std::wstring& CanonicalName, const std::wstring& FileName )
{
	CELOG_LOG(CELOG_DUMP, L" The Parameters are: CanonicalName=%ls, FileName=%ls \n", CanonicalName.c_str(), FileName.c_str());

	CanonicalName.clear();

	//is a local file
	if ( 0 == _wcsnicmp(L"file:///", FileName.c_str(), wcslen(L"file:///") ) )
	{
		CanonicalName = FileName.substr( wcslen(L"file:///") );
		boost::replace_all(CanonicalName,L"/",L"\\");
	}
	// is a UNC file
	else if ( 0 == _wcsnicmp(L"file://", FileName.c_str(), wcslen(L"file://") ) )
	{
		CanonicalName = FileName.substr( wcslen(L"file:") );
		boost::replace_all(CanonicalName,L"/",L"\\");
	}
	else
	{
		CanonicalName = FileName;
	}
	 
	return TRUE;
}

std::wstring CActionHandler::FormatPath(const std::wstring& path)
{
	CELOG_LOG(CELOG_DUMP, L" The Parameters are: path=%ls \n", path.c_str());

	std::wstring ret = path;

	if (boost::algorithm::istarts_with(path, L"http") || boost::algorithm::istarts_with(path, L"//")
		|| boost::algorithm::istarts_with(path, L"\\\\"))
	{
		ConvertURLCharacterW(ret);
	}
    CELOG_LOG(CELOG_DUMP, L"Local variables are: ret=%ls \n", ret.c_str() );

	return ret;
}

//////////////////////////////////////////////////////////////////////////
//CIEDefaultActionHandler

CIEDefaultActionHandler::CIEDefaultActionHandler ( )
{

}

CIEDefaultActionHandler::CIEDefaultActionHandler ( const CIEDefaultActionHandler& CInstance )
{

}

CIEDefaultActionHandler& CIEDefaultActionHandler::operator= ( const CIEDefaultActionHandler& CInstance )
{
	if ( this != &CInstance )
	{

	}

	return *this;
}

CIEDefaultActionHandler::~CIEDefaultActionHandler ( )
{
  
}

BOOL CIEDefaultActionHandler::OpenAction ( const WCHAR* Url )
{
    CELOG_LOG(CELOG_DUMP, L" The Parameters are: Url=%ls \n",  (Url) );

	if ( NULL != Url )
	{
		//Handle all urls, include local file path, UNC path and common url
		if ( 0 != _wcsicmp ( Url, L"about:Tabs" ) && 0 != _wcsicmp ( Url, L"about:blank" ) )
		{		
			std::wstring strUrl = Url;

			strUrl = FormatPath(strUrl);
		
			boost::shared_ptr<nextlabs::comm_base> ptr = nextlabs::comm_helper::CreateComm(nextlabs::type_cesdk, &cesdk_context_instance);
			nextlabs::eval_parms parm;
			parm.SetAction(L"OPEN");
			parm.SetSrc(strUrl.c_str());
			parm.SetIp(g_ip);

			if(ptr->Query(&parm))
			{
				if(ptr->IsDenied())
				{

					return FALSE;
				}
			}	
		}	
	}

	return TRUE;
}

BOOL CIEDefaultActionHandler::PrintAction ( const DOCINFOW* lpdi, const WCHAR* ActiveUrl )
{
	CELOG_LOG(CELOG_DUMP, L" The Parameters are: lpdi=%p, ActiveUrl=%ls \n", lpdi, (ActiveUrl));

	std::wstring CanonicalName;

	boost::shared_ptr<nextlabs::comm_base> ptr = nextlabs::comm_helper::CreateComm(nextlabs::type_cesdk, &cesdk_context_instance);

	if ( NULL != lpdi && NULL != lpdi->lpszDocName )
	{
		GetCanonicalName ( CanonicalName, lpdi->lpszDocName );

		CanonicalName = FormatPath(CanonicalName);

		nextlabs::eval_parms parm;
		parm.SetAction(L"PRINT");
		parm.SetSrc(CanonicalName.c_str());
		parm.SetIp(g_ip);

		//do evaluation for document
		if(ptr->Query(&parm))
		{
			if(ptr->IsDenied())
			{
				CELOG_LOG(CELOG_DUMP, L"Local variables are: CanonicalName=%ls \n", CanonicalName.c_str() );
				return FALSE;
			}
			
			//if allow, do evaluation for active url again
			if ( NULL != ActiveUrl )
			{
				GetCanonicalName ( CanonicalName, ActiveUrl );

				CanonicalName = FormatPath(CanonicalName);

				parm.SetSrc(CanonicalName.c_str());
				parm.SetIp(g_ip);

				if(ptr->Query(&parm))
				{
					if(ptr->IsDenied())
					{
						return FALSE;
					}
				}
			}
		}
	}
	
	return TRUE;
} 

BOOL CIEDefaultActionHandler::PrintAction(const WCHAR* activeUrl)
{
	CELOG_LOG(CELOG_DUMP, L" The Parameters are: activeUrl=%ls \n", activeUrl);

	std::wstring CanonicalName;

	boost::shared_ptr<nextlabs::comm_base> ptr = nextlabs::comm_helper::CreateComm(nextlabs::type_cesdk, &cesdk_context_instance);

	//if allow, do evaluation for active url again
	if ( NULL != activeUrl )
	{
		nextlabs::eval_parms parm;
		parm.SetAction(L"PRINT");

		GetCanonicalName ( CanonicalName, activeUrl );

		CanonicalName = FormatPath(CanonicalName);

		parm.SetSrc(CanonicalName.c_str());
		parm.SetIp(g_ip);

		if(ptr->Query(&parm))
		{
			if(ptr->IsDenied())
			{
				return FALSE;
			}
		}
	}

	return TRUE;
}

BOOL CIEDefaultActionHandler::UploadAction ( const LPOPENFILENAME lcpofn, const std::wstring& UploadUrl )
{
	CELOG_LOG(CELOG_DUMP, L" The Parameters are: lcpofn=%p, UploadUrl=%ls \n", lcpofn,UploadUrl.c_str());

	if ( NULL != lcpofn && NULL != lcpofn->lpstrFile && !UploadUrl.empty() )
	{
		if ( NULL == lcpofn->lpstrTitle || 0 != _wcsicmp ( L"Windows Internet Explorer", lcpofn->lpstrTitle ) )
		{
            if (!UploadAction(lcpofn->lpstrFile, UploadUrl) )
            {
                return FALSE;
            }
		}
	}

	return TRUE;
}


BOOL CIEDefaultActionHandler::UploadAction(const std::wstring& src, const std::wstring& dest)
{
    boost::shared_ptr<nextlabs::comm_base> ptr = nextlabs::comm_helper::CreateComm(nextlabs::type_cesdk, &cesdk_context_instance);

    nextlabs::eval_parms parm;
    parm.SetAction(L"SEND");
    parm.SetSrc(src.c_str());
    parm.SetTarget(dest.c_str());
	parm.SetIp(g_ip);

    if(ptr->Query(&parm) && ptr->IsDenied())
    {
        return FALSE;
    }

    return TRUE;
}

BOOL CIEDefaultActionHandler::PasteAction(const std::wstring& src, const std::wstring& dest)
{
    boost::shared_ptr<nextlabs::comm_base> ptr = nextlabs::comm_helper::CreateComm(nextlabs::type_cesdk, &cesdk_context_instance);

	std::wstring path;
	GetCanonicalName(path, src);
    nextlabs::eval_parms parm;
    parm.SetAction(L"PASTE");
	parm.SetSrc(path.c_str());
    parm.SetTarget(dest.c_str());
	parm.SetIp(g_ip);

    if(ptr->Query(&parm) && ptr->IsDenied())
    {
        return FALSE;
    }

    return TRUE;
}