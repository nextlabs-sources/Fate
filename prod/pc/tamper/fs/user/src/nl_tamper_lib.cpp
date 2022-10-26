#define WINVER _WIN32_WINNT_WINXP
#ifdef _WIN32_WINNT
#undef _WIN32_WINNT
#endif
#define _WIN32_WINNT _WIN32_WINNT_WINXP
#define NTDDI_VERSION NTDDI_WINXPSP2

#include <windows.h>
#include <cstdio>
#include <cassert>
#include <fltuser.h>
#include <tchar.h>
#include <stdlib.h>

#include "nl_tamper.h"
#include "nl_tamper_lib.h"

//Convert a wide-character string to lowercase.
static void wcstoupper(WCHAR *inout, size_t inout_size )
{
  size_t i;
  if(inout == NULL || inout_size == 0)
	return; //do nothing
  if(inout[0]==L'\0')
	return; //do nothing

  WCHAR buf[MAX_PATH * 2];
  unsigned int endIndex=MAX_PATH * 2-1;
  for(i=0; inout[i] != L'\0' && i < endIndex && i < inout_size; i++) {
	if(inout[i] >= L'a' && inout[i] <= L'z')
	  buf[i]=L'A'+(inout[i]-L'a');
	else
	  buf[i]=inout[i];
  }
  buf[i]=L'\0';
  wcsncpy_s(inout, inout_size, buf, _TRUNCATE);
  //KdPrint(("NLTamper: wstrtolow: %ws convert to %ws\n", inout, buf));
}

static int WriteCommand( NLFiltCommand* cmd )
{
  HRESULT hr;
  HANDLE port;
  int result = 0;

  hr = FilterConnectCommunicationPort(NLTAMPER_PORT_NAME,0,NULL,0,NULL,&port);
  if( !IS_ERROR(hr) )
  {
    DWORD result_size;
    hr = FilterSendMessage(port,cmd,sizeof(*cmd),NULL,0,&result_size);
    if( !IS_ERROR(hr) )
    {
      result = 1;
    }
  }

  if( port != INVALID_HANDLE_VALUE )
  {
    CloseHandle(port);
  }

  return result;
}/* WriteCommand */

void init_cmd( NLFiltCommand* cmd )
{
  memset(cmd,0x00,sizeof(NLFiltCommand));
  cmd->policy.flags = NL_TAMPER_FLAG_NONE;
  cmd->policy.fsm.pid = GetCurrentProcessId();
}/* init_cmd */

int NLTamper_ProtectFile( const WCHAR* fname , int flags )
{
  NLFiltCommand cmd;
  init_cmd(&cmd);
  cmd.type      = NL_FILT_COMMAND_ADD_POLICY;
  cmd.policy.type   = NL_FILT_POLICY_PROTECT_FILE;
  cmd.policy.flags  = flags;
  wcsncpy_s(cmd.policy.fsm.fname,NLFILT_MAX_PATH, fname, _TRUNCATE);
  NLTamper_TranslatePath(cmd.policy.fsm.fname,_countof(cmd.policy.fsm.fname));
  wcstoupper(cmd.policy.fsm.fname,_countof(cmd.policy.fsm.fname));
  WriteCommand(&cmd);
  return 0;
}/* NLTamper_ProtectFile */

int NLTamper_ExemptProcessName( const WCHAR* pname )
{
  NLFiltCommand cmd;
  init_cmd(&cmd);
  cmd.type = NL_FILT_COMMAND_ADD_POLICY;
  cmd.policy.type = NL_FILT_POLICY_EXEMPT_PROCESS_NAME;
  cmd.policy.flags  = NL_TAMPER_FLAG_EXEMPT;
  wcsncpy_s(cmd.policy.fsm.pname,NLFILT_MAX_PATH, pname, _TRUNCATE);
  if(wcsstr(pname, L":\\"))
    NLTamper_TranslatePath(cmd.policy.fsm.pname,_countof(cmd.policy.fsm.pname));
  WriteCommand(&cmd);
  return 0;
}/* NLTamper_ExemptProcessName */

int NLTamper_ExemptProcessId( ULONG pid )
{
  NLFiltCommand cmd;
  init_cmd(&cmd);
  cmd.type = NL_FILT_COMMAND_ADD_POLICY;
  cmd.policy.type = NL_FILT_POLICY_EXEMPT_PROCESS_ID;
  cmd.policy.flags  = NL_TAMPER_FLAG_EXEMPT;
  cmd.policy.fsm.pid = pid;
  WriteCommand(&cmd);
  return 0;
}/* NLTamper_ExemptProcessId */

int NLTamper_FileExemptProcessName( const WCHAR* pname, const WCHAR* fname )
{
  NLFiltCommand cmd;
  init_cmd(&cmd);
  cmd.type = NL_FILT_COMMAND_ADD_POLICY;
  cmd.policy.type = NL_FILT_POLICY_FILE_EXEMPT_PROCESS_NAME;
  cmd.policy.flags  = NL_TAMPER_FLAG_EXEMPT;
  wcsncpy_s(cmd.policy.fsm.pname,NLFILT_MAX_PATH, pname, _TRUNCATE);
  if(wcsstr(pname, L":\\"))
    NLTamper_TranslatePath(cmd.policy.fsm.pname,_countof(cmd.policy.fsm.pname));
  wcsncpy_s(cmd.policy.fsm.fname,NLFILT_MAX_PATH, fname, _TRUNCATE);
  NLTamper_TranslatePath(cmd.policy.fsm.fname,_countof(cmd.policy.fsm.fname));
  wcstoupper(cmd.policy.fsm.fname,_countof(cmd.policy.fsm.fname)); 
  WriteCommand(&cmd);
  return 0;
}/* NLTamper_ExemptProcessName */


int NLTamper_TranslatePath( WCHAR* path , int path_size )
{
  WCHAR buf[MAX_PATH * 2] = {0};
  PFILTER_VOLUME_BASIC_INFORMATION vi = (PFILTER_VOLUME_BASIC_INFORMATION)buf;
  DWORD vi_size = sizeof(buf);
  HANDLE vhandle;
  HRESULT hr;
  int new_path_size = 0;

  hr = FilterVolumeFindFirst(FilterVolumeBasicInformation,vi,vi_size,&vi_size,&vhandle);
  if( hr != S_OK )
  {
    return 0;
  }

  for( ; SUCCEEDED(hr) ; )
  {
    WCHAR letter[8] = {0}, vname[MAX_PATH] = {0};

    memcpy(vname,vi->FilterVolumeName,min(sizeof(vname),vi->FilterVolumeNameLength));

    hr = FilterGetDosName(vname,letter,_countof(letter));
    if( SUCCEEDED(hr) )
    {
      if( iswalpha(path[0]) && towlower(letter[0]) == towlower(path[0]) )
      {
	new_path_size = _snwprintf_s(buf,
				     _countof(buf), _TRUNCATE, 
				     L"%s%s",vname,path + 2);
	wcsncpy_s(path,path_size, buf,_TRUNCATE);
      }
      else if( path[0] == L'\\' && 
	       wcsncmp(vname,path,vi->FilterVolumeNameLength/sizeof(WCHAR)) == 0 )
      {
	new_path_size = _snwprintf_s(buf, 
				   _countof(buf), _TRUNCATE,L"%s%s",
				   letter,
				   path + vi->FilterVolumeNameLength/sizeof(WCHAR));
	wcsncpy_s(path,path_size, buf,_TRUNCATE);
      }
    }
    vi_size = sizeof(buf);
    hr = FilterVolumeFindNext(vhandle,FilterVolumeBasicInformation,vi,vi_size,&vi_size);
  }/* for */
  FilterVolumeFindClose(vhandle);
  return new_path_size;
}/* NLTamper_TranslatePath */
