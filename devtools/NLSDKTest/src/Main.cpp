// Defines the entry point for the console application.
//

#include "..\include\\Main.h"

CEHandle sdk_connHandle;
nextlabs::cesdk_loader cesdk;

WCHAR application[MAX_PATH] = { 0 };

WCHAR* pszUserName;
WCHAR* pszUserSID;

long volatile SuccessfulCount = 0;
long volatile  FailedCount = 0;

int wmain ( int argc, WCHAR* argv[] )
{
	//Print help message
	if ( argc == 1 || ( 0 == _wcsicmp ( argv[1], L"-h" ) || 0 == _wcsicmp ( argv[1], L"--help" ) || 0 == _wcsicmp ( argv[1], L"/?" ) ) )
	{
		PrintUsage ( );
		return 0;
	}

	//If call SDK in single thread
	if ( 0 == _wcsicmp ( argv[1], L"--single" ) || 0 == _wcsicmp ( argv[1], L"-s" ) )
	{
		CallInSingleThread ( );
		printf ( "Successful count:%ld, failed count:%ld, total count:%ld \n", SuccessfulCount, FailedCount, SuccessfulCount + FailedCount  );
		printf ( "Completed \n" );
	}
	//call SDK in multi-thread
	else if ( 0 == _wcsicmp ( argv[1], L"--multi" ) || 0 == _wcsicmp ( argv[1], L"-m" ) )
	{
		CallInMultiThread ( );
		printf ( "Successful count:%ld, failed count:%ld, total count:%ld \n", SuccessfulCount, FailedCount, SuccessfulCount + FailedCount  );
		printf ( "Completed \n" );
	}
	else
	{
		PrintUsage ( );
	}

	return 0;
}

//Print help message
void PrintUsage ( )
{
	printf ( "Test SDK Utility \n" );
	printf ( "usage: TestSDK  <command> \n" );
	printf ( "command     Specify single thread or multi-thread calling SDK \n" );
	printf ( "      --single/-s             calling SDK in single thread \n" );
	printf ( "      --multi/-m              calling SDK in multi-thread \n" );
}

//call SDK in single thread
BOOL CallInSingleThread ( )
{
	Initial();

	for ( int i = 0; i < 100; i++ )
	{
		DoEvaluation();
	}

	Uninitial();

	return TRUE;
}

//call SDK in multi-thread
BOOL CallInMultiThread ( )
{
	Initial ( FALSE );

	HANDLE hThread[1000] = { 0 };

	for ( int i = 0; i < 1000; i++ )
	{
		hThread[i] = CreateThread ( NULL, 0, CallSDK, NULL, 0, NULL );
	}

	for ( int i = 0; i < 1000; i++ )
	{
		WaitForSingleObject(hThread[i], INFINITE);
		CloseHandle(hThread[i]);
	}

	Uninitial ( FALSE );

	return TRUE;
}

//call SDK
DWORD WINAPI CallSDK ( LPVOID )
{
	DoEvaluation ( FALSE );

	return 0;
}

//initial
BOOL Initial ( BOOL bSingle )
{
	GetModuleFileNameW ( NULL, application, MAX_PATH );

	cesdk.load ( L"C:\\Program Files\\NextLabs\\Policy Controller\\bin" );

	GetUserInfo (&pszUserName,&pszUserSID);

	if ( bSingle )
	{
		CEApplication app = { 0 };
		CEUser user = { 0 };   

		app.appPath = cesdk.fns.CEM_AllocateString ( application );

		user.userName = cesdk.fns.CEM_AllocateString(pszUserName);
		user.userID = cesdk.fns.CEM_AllocateString(pszUserSID);

		cesdk.fns.CECONN_Initialize ( app, user, NULL, &sdk_connHandle, 1000 );

		cesdk.fns.CEM_FreeString(app.appPath);
		if(NULL!=user.userName) cesdk.fns.CEM_FreeString(user.userName);
		if(NULL!=user.userID) cesdk.fns.CEM_FreeString(user.userID);
	}

	return TRUE;
}

//uninitial
BOOL Uninitial ( BOOL bSingle )
{
	if ( bSingle )
	{
		cesdk.fns.CECONN_Close ( sdk_connHandle, 30000 );
	}

	if ( NULL != pszUserName)
	{
		delete []pszUserName;
	}

	if ( NULL != pszUserSID)
	{
		delete []pszUserSID;
	}

	cesdk.unload ( );

	return TRUE;
}

//do evaluation
BOOL DoEvaluation ( BOOL bSingle )
{
	CEString ce_string_action_name = cesdk.fns.CEM_AllocateString(L"COPY");

	std::wstring final_source(L"c:\\kaka\\test.txt");  
	CEResource* res_source = cesdk.fns.CEM_CreateResourceW(final_source.c_str(),L"fso");

	std::wstring final_target(L"c:\\test\\test.txt");             
	CEResource* res_target = cesdk.fns.CEM_CreateResourceW(final_target.c_str(),L"fso");

	CEAttributes* in_source_attrs = nextlabs::cesdk_attributes::create();
	nextlabs::cesdk_attributes::add(cesdk,in_source_attrs,L"ce::request_cache_hint",L"yes");
	nextlabs::cesdk_attributes::add(cesdk,in_source_attrs,L"ce::filesystemcheck",L"no");

	CEAttributes* in_target_attrs = nextlabs::cesdk_attributes::create();
	nextlabs::cesdk_attributes::add(cesdk,in_target_attrs,L"ce::filesystemcheck",L"no");

	CEUser user = { 0 };   
	user.userName = cesdk.fns.CEM_AllocateString(pszUserName);
	user.userID = cesdk.fns.CEM_AllocateString(pszUserSID);

	CEApplication app = { 0 };
	app.appPath = cesdk.fns.CEM_AllocateString ( application );

	CEEnforcement_t enforcement;

	CEResult_t result = CE_RESULT_SUCCESS;

	if ( bSingle )
	{
		result = cesdk.fns.CEEVALUATE_CheckResources(sdk_connHandle, ce_string_action_name, res_source, in_source_attrs, res_target, in_target_attrs, &user, NULL, &app, NULL, NULL, 0, 0, CETrue, CE_NOISE_LEVEL_USER_ACTION, &enforcement, 30000 );       
	}
	else
	{
		CEApplication applocal = { 0 };
		CEUser userlocal = { 0 };   

		applocal.appPath = cesdk.fns.CEM_AllocateString ( application );

		userlocal.userName = cesdk.fns.CEM_AllocateString(pszUserName);
		userlocal.userID = cesdk.fns.CEM_AllocateString(pszUserSID);

		CEHandle sdk_connHandlelocal;

		CEResult_t resultlocal = cesdk.fns.CECONN_Initialize ( applocal, userlocal, NULL, &sdk_connHandlelocal, 30000 );
	
		if ( CE_RESULT_SUCCESS != resultlocal )
		{
			printf ( "Create handle, error code:%d \n", resultlocal );
		}

		result = cesdk.fns.CEEVALUATE_CheckResources(sdk_connHandlelocal, ce_string_action_name, res_source, in_source_attrs, res_target, in_target_attrs, &user, NULL, &app, NULL, NULL, 0, 0, CETrue, CE_NOISE_LEVEL_USER_ACTION, &enforcement, 30000 );    

		cesdk.fns.CEM_FreeString(applocal.appPath);
		if(NULL!=userlocal.userName) cesdk.fns.CEM_FreeString(userlocal.userName);
		if(NULL!=userlocal.userID) cesdk.fns.CEM_FreeString(userlocal.userID);

		cesdk.fns.CECONN_Close ( sdk_connHandlelocal, 30000 );
	}


	if ( CE_RESULT_SUCCESS == result )
	{
		InterlockedIncrement ( &SuccessfulCount );
	}
	else
	{
		InterlockedIncrement ( &FailedCount );
		printf ( "Error code:%d \n", result );
	}

	cesdk.fns.CEM_FreeString(ce_string_action_name);

	cesdk.fns.CEM_FreeResource(res_source);
	cesdk.fns.CEM_FreeResource(res_target);

	nextlabs::cesdk_attributes::destroy(cesdk,in_source_attrs);
    nextlabs::cesdk_attributes::destroy(cesdk,in_target_attrs);

	if(NULL!=user.userName) cesdk.fns.CEM_FreeString(user.userName);
	if(NULL!=user.userID) cesdk.fns.CEM_FreeString(user.userID);

    cesdk.fns.CEM_FreeString(app.appPath);

	cesdk.fns.CEEVALUATE_FreeEnforcement(enforcement);

	return TRUE;
}

//Get the user name of the current process
BOOL GetUserInfo ( WCHAR **pszUserName,WCHAR **pszUserSID)
{
	if(NULL==pszUserName || NULL==pszUserSID)
		return FALSE;

	HANDLE hToken;
	HANDLE hCurrentProcess = GetCurrentProcess();
	BOOL bSIDFound = FALSE;
	if(::OpenProcessToken(hCurrentProcess, TOKEN_READ, &hToken))
	{
		DWORD dwLen = NULL;
		::GetTokenInformation(hToken, TokenUser, NULL, 0, &dwLen);
		if(GetLastError() == ERROR_INSUFFICIENT_BUFFER)
		{
			PTOKEN_USER pTokenUser = (PTOKEN_USER) malloc (dwLen);
			if(NULL!=pTokenUser )
			{
				if(::GetTokenInformation(hToken, TokenUser, pTokenUser, dwLen, &dwLen))
				{
					DWORD dwSidLen = ::GetLengthSid(pTokenUser->User.Sid);
					PSID pSid = (PSID)malloc(dwSidLen);
					if (NULL!=pSid ) 
					{
						if(::CopySid(dwSidLen, pSid, pTokenUser->User.Sid))
						{
							TCHAR* pszSID = NULL;
							::ConvertSidToStringSid((PSID) pSid, &pszSID);
							*pszUserSID = (TCHAR*) malloc (sizeof(TCHAR) * (_tcslen (pszSID) + 1));
							if(NULL != *pszUserSID)
							{
								_tcscpy (*pszUserSID, pszSID);
								::LocalFree (pszSID);
								bSIDFound = TRUE;
							}
						}
						free (pSid);
					}
				}
				free (pTokenUser);
			}
		}
		CloseHandle(hToken);
	}
	CloseHandle(hCurrentProcess);
	if (!bSIDFound)
	{
		*pszUserSID = (TCHAR*) malloc (sizeof (TCHAR) * (_tcslen(_T("InvalidUserName"))+1));
		if(NULL != *pszUserSID)
		{
			_tcscpy (*pszUserSID, _T("InvalidUserName"));
			*pszUserName = (TCHAR*) malloc (sizeof (TCHAR) * (_tcslen(_T("InvalidUserName"))+1));
			if(NULL != *pszUserName)
			{
				_tcscpy (*pszUserName, _T("InvalidUserName"));
			}
		}
	}
	else
	{
		std::wstring uname;
		GetUserNameFromSID(*pszUserSID,uname);
		*pszUserName = (TCHAR*)malloc(1024);
		if(NULL != *pszUserName)
		{
			wcsncpy_s(*pszUserName,1024/sizeof(wchar_t),uname.c_str(),_TRUNCATE);
		}
	}

	return bSIDFound;
}

//Get user name from SID
BOOL GetUserNameFromSID( const wchar_t* user_sid , std::wstring& user )
{
	WCHAR user_name[512] = {0};
	WCHAR user_domain[512] = {0};
	PSID sid = (PSID)NULL;
	BOOL rv = FALSE;

	if( ConvertStringSidToSidW(user_sid,&sid) != FALSE )
	{
		SID_NAME_USE name_use;
		DWORD user_name_size = _countof(user_name);
		DWORD user_domain_size = _countof(user_domain);

		rv = LookupAccountSidW(NULL,sid,user_name,&user_name_size,user_domain,&user_domain_size,&name_use);

		LocalFree(sid);
	}
	if( rv != FALSE ) 
	{
		user.clear();
		user = user_domain;
		user += L"\\";
		user += user_name;
		return TRUE;
	}
	return FALSE;
}