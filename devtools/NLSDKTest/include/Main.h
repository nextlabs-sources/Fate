#pragma once

#include <windows.h>
#include <Sddl.h>
#include <tchar.h>

#include "eframework\platform\cesdk_loader.hpp"
#include "eframework\platform\cesdk_attributes.hpp"

//////////////////////////////////////////////////////////////////////////

//Print help message
void PrintUsage ( );

//////////////////////////////////////////////////////////////////////////

//call SDK in single thread
BOOL CallInSingleThread ( ); 

//call SDK in multi-thread
BOOL CallInMultiThread ( ); 

//call SDK
DWORD WINAPI CallSDK ( LPVOID lpParameter );

//////////////////////////////////////////////////////////////////////////

//initial
BOOL Initial ( BOOL bSingle = true );

//uninitial
BOOL Uninitial ( BOOL bSingle = true );

//do evaluation
BOOL DoEvaluation ( BOOL bSingle = true );

//Get the user name of the current process
BOOL GetUserInfo ( WCHAR **pszUserName,WCHAR **pszUserSID );

//Get user name from SID
BOOL GetUserNameFromSID( const wchar_t* user_sid , std::wstring& user );

//////////////////////////////////////////////////////////////////////////