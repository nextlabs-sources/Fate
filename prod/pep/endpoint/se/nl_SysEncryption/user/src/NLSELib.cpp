#define WINVER _WIN32_WINNT_WINXP
#pragma warning( push )
#pragma warning( disable : 4005 )
#define _WIN32_WINNT _WIN32_WINNT_WINXP
#pragma warning( pop )
#define NTDDI_VERSION NTDDI_WINXPSP2

#include <windows.h>
#include <fltuser.h>
#include "NLSECommon.h"
#include "celog.h"

#define CELOG_CUR_MODULE L"NLSEPlugin"
#define CELOG_CUR_FILE \
  CELOG_FILEPATH_PROD_PEP_ENDPOINT_SE_NL_SYSENCRYPTION_USER_NLSELIB_CPP

//Send user mode command to kernel
static int WriteCommand( NLSE_USER_COMMAND * cmd )
{
  HRESULT           hr;
  HANDLE            port;
  NLSE_PORT_CONTEXT portCtx;
  int               result = 0;

  if(cmd->type == NLSE_USER_COMMAND_ENABLE_FILTER ||
     cmd->type == NLSE_USER_COMMAND_DISABLE_FILTER) {
    portCtx.portTag=NLSE_PORT_TAG_MAIN_CMD;
  } else {
    portCtx.portTag=NLSE_PORT_TAG_DRM;
  }
  hr = FilterConnectCommunicationPort(NLSE_PORT_NAME,0,
				      &portCtx,
				      sizeof(portCtx),
				      NULL,
				      &port);
  if( !IS_ERROR(hr) && port != NULL) {
    DWORD result_size;
    hr = FilterSendMessage(port,cmd,sizeof(*cmd),NULL,0,&result_size);
    if( !IS_ERROR(hr) ) {
      result = 1;
    }
  } else {
    CELOG_LOG(CELOG_CRITICAL, 
	     L"NLSELib!WriteCommand: ConnectCommunicationPort failed 0x%x\n", 
	     hr);
  }    

  if( port != INVALID_HANDLE_VALUE ) {
    CloseHandle(port);
  }

  if(result == 0) {
    CELOG_LOG(CELOG_ERROR, L"NLSELib: WriteCommand failed: 0x%x\n", hr);
  }

  return result;
}/*WriteCommand */

void init_cmd( NLSE_USER_COMMAND *cmd )
{
  memset(cmd,0x00,sizeof(NLSE_USER_COMMAND));
}/* init_cmd */

/** NLSEUserCmd_EnableFilter
 *
 *  \brief Enable NLSE filter driver functionality
 *
 *  \return TRUE on success.
 */
BOOLEAN NLSEUserCmd_EnableFilter()
{
  NLSE_USER_COMMAND cmd;

  init_cmd(&cmd);

  cmd.type=NLSE_USER_COMMAND_ENABLE_FILTER;

  if(WriteCommand(&cmd)==0) {
    //write command failed
    return FALSE;
  }

  return TRUE;
}

/** NLSEUserCmd_DisableFilter
 *
 *  \brief disable NLSE filter driver functionality
 *
 *  \return TRUE on success.
 */
BOOLEAN NLSEUserCmd_DisableFilter()
{
  NLSE_USER_COMMAND cmd;

  init_cmd(&cmd);

  cmd.type=NLSE_USER_COMMAND_DISABLE_FILTER;

  if(WriteCommand(&cmd) == 0) {
    //write command failed
    return FALSE;
  }

  return TRUE;
}
/** NLSEUserCmd_SetIgnoredProcessByPID
 *
 *  \brief tell kernel not to handle any i/o operations of a process by PID 
 *
 *  \return TRUE on success.
 */
BOOLEAN NLSEUserCmd_SetIgnoreProcessByPID(ULONG pid)
{
  NLSE_USER_COMMAND cmd;

  init_cmd(&cmd);

  //This also means this process is not DRM application; it
  //always get encrypted data of encrypted file
  cmd.type=NLSE_USER_COMMAND_SET_IGNORE_PROCESS_BY_PID;
  cmd.msg.pid=pid;

  if(WriteCommand(&cmd) == 0) {
    //write command failed
    return FALSE;
  }

  return TRUE;
}
/** NLSEUserCmd_SetIgnoredProcessByPID
 *
 *  \brief tell kernel not to ignore a process by PID 
 *
 *  \return TRUE on success.
 */
BOOLEAN NLSEUserCmd_UnsetIgnoreProcessByPID(ULONG pid)
{
  NLSE_USER_COMMAND cmd;

  init_cmd(&cmd);

  //Unset previous setting ignore process command
  cmd.type=NLSE_USER_COMMAND_UNSET_IGNORE_PROCESS_BY_PID;
  cmd.msg.pid=pid;

  if(WriteCommand(&cmd) == 0) {
    //write command failed
    return FALSE;
  }

  return TRUE;
}
