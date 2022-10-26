#include <cstdio>
#include <cassert>
#include <tchar.h>
#include <windows.h>
#include <wininet.h>

#include "CEAdapter.h"

#include "CEsdk.h"
#include "cesdk_wrapper.h"

BOOL STDAPICALLTYPE myFtpPutFileW( HINTERNET hConnect,
				   LPCWSTR lpszLocalFile,
				   LPCWSTR lpszNewRemoteFile,
				   DWORD dwFlags,
				   DWORD_PTR dwContext )
{
  adapter::SetAdapterState("wininet.dll","FtpPutFileW");

  if( cesdk_wrapper::Eval(CE_ACTION_COPY,lpszLocalFile,lpszNewRemoteFile) == false )
  {
    adapter::SetDenyReturnValue<BOOL>(FALSE);
  }
  return adapter::Next<BOOL>(hConnect,lpszLocalFile,lpszNewRemoteFile,dwFlags,dwContext);
}/* myFtpPutFileW */

HINTERNET STDAPICALLTYPE myInternetConnectW( HINTERNET hInternet,
					     LPCWSTR lpszServerName,
					     INTERNET_PORT nServerPort,
					     LPCWSTR lpszUserName,
					     LPCWSTR lpszPassword,
					     DWORD dwService,
					     DWORD dwFlags,
					     DWORD_PTR dwContext )
{
  adapter::SetAdapterState("wininet.dll","InternetConnectW");

  if( cesdk_wrapper::Eval(CE_ACTION_RUN,lpszServerName) == false )
  {
    adapter::SetDenyReturnValue<HINTERNET>(NULL);
  }

  return adapter::Next<HINTERNET>(hInternet,lpszServerName,nServerPort,
			      lpszUserName,lpszPassword,dwService,
			      dwFlags,dwContext);
}/* myInternetConnectW */

int AdapterEntry(void)
{
  fprintf(stderr, "adapter_winint: AdapterEntry\n");

  bool bresult;

  typedef struct
  {
    const char* lib;      // Library
    const char* fn_name;  // Function name
    void* fn_addr;        // Function address
  } hook;

  hook list[] =
    {
      //"wininet.dll", "InternetConnectW", myInternetConnectW,
      "wininet.dll", "FtpPutFileW",      myFtpPutFileW
    };

  for( int i = 0 ; i < sizeof(list) / sizeof(hook) ; i++ )
  {
    bresult = adapter::AddHook(list[i].lib,list[i].fn_name,list[i].fn_addr);

    if( bresult == false )
    {
      return -1;
    }
  }

  return 0;
}/* AdapterEntry */
