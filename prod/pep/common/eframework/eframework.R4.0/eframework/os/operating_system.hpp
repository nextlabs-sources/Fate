
#ifndef __EFRAMEWORK_OPERATING_SYSTEM_HPP__
#define __EFRAMEWORK_OPERATING_SYSTEM_HPP__

#include <windows.h>

namespace nextlabs
{

  class os
  {

  public:

    /** is_system_process
     *
     *  \brief Determine if the current process is a SYSTEM process.
     *  \return true if the current process is a SYSTEM process, otherwise false.
     */
    static bool is_system_process(void) throw()
    {
      return is_system_process(GetCurrentProcessId());
    }/* is_system_process */

    /** is_system_process
     *
     *  \brief Determine if the current process is a SYSTEM process.
     *
     *  \param in_pid (in) Process ID (PID) of the process to check.
     *
     *  \return true if the current process is a SYSTEM process, otherwise false.
     */
    static bool is_system_process(_In_ DWORD in_pid ) throw()
    {
      DWORD dwErrCode		= 0;
      LPSTR AcctName		= NULL;
      LPSTR DomainName	= NULL;
      DWORD dwAcctName	= 1, dwDomainName = 1;
      SID_NAME_USE eUse	= SidTypeUnknown;
      HANDLE hProcess		= NULL;
      BOOL bSystemProcess = TRUE;
      HANDLE hToken = NULL;
      PTOKEN_USER pTokenUser = NULL;;
      DWORD dwLen = 0;

      hProcess = OpenProcess(PROCESS_ALL_ACCESS,FALSE,in_pid);
      if (!hProcess)
      {
	goto cleanup; 
      }

      if (!OpenProcessToken (hProcess, TOKEN_READ, &hToken))
      {
        goto cleanup; 
      }

      GetTokenInformation (hToken,TokenUser,NULL,0,&dwLen);
      pTokenUser = (PTOKEN_USER) GlobalAlloc(GMEM_FIXED,dwLen);
      // Check pTokenUser for GlobalAlloc error condition.
      if (pTokenUser == NULL) 
      {
        goto cleanup;      
      }
      if (!GetTokenInformation(hToken,TokenUser,pTokenUser,dwLen,&dwLen))
      {
        goto cleanup; 
      }

      // First call to LookupAccountSid to get the buffer sizes.
      dwErrCode = LookupAccountSidA(NULL,pTokenUser->User.Sid,AcctName,(LPDWORD)&dwAcctName,
				    DomainName,(LPDWORD)&dwDomainName,&eUse);
      if (dwErrCode != ERROR_SUCCESS)
      {
        goto cleanup;
      }

      // Reallocate memory for the buffers.
      AcctName = (char *)GlobalAlloc(GMEM_FIXED,dwAcctName);
      if (AcctName == NULL) 
      {
        goto cleanup;      
      }

      DomainName = (char *)GlobalAlloc(GMEM_FIXED,dwDomainName);
      if (DomainName == NULL) 
      {
        goto cleanup; 
      }

      // Second call to LookupAccountSid to get the account name.
      dwErrCode = LookupAccountSidA(NULL,
				    pTokenUser->User.Sid,          // security identifier
				    AcctName,                      // account name buffer
				    (LPDWORD)&dwAcctName,          // size of account name buffer 
				    DomainName,                    // domain name
				    (LPDWORD)&dwDomainName,        // size of domain name buffer
				    &eUse);                        // SID type

      if( _stricmp(AcctName,"SYSTEM") == 0 && _stricmp(DomainName,"NT AUTHORITY") == 0 )
      {
        goto cleanup; // this is a process running under the SYSTEM account
      }

      // none of the conditions met -- this is not a system process
      bSystemProcess = FALSE; 

    cleanup:
      if (hProcess != NULL)
      {
        CloseHandle (hProcess);
      }
      if (pTokenUser != NULL)
      {
	GlobalFree (pTokenUser);
      }
      if (AcctName != NULL)
      {
        GlobalFree (AcctName);
      }
      if (DomainName != NULL)
      {
        GlobalFree (DomainName);
      }

      return bSystemProcess;
    }/* is_system_process */

    /** is_process_x86
     *
     *  \brief Determine if the current process is x86?
     *
     *  \return true if the process is x86, otherwise false.
     */
    static bool is_process_x86(void) throw()
    {
      return !is_process_x64();
    }/* is_process_x86 */

    /** is_process_x64
     *
     *  \brief Determine if the current process is x64?
     *
     *  \return true if the process is x64, otherwise false.
     */
    static bool is_process_x64(void) throw()
    {
#if defined (_WIN64) || (_AMD64_)
      return true;
#else
      return false;
#endif
    }/* is_process_x86 */

  };/* class os */
}/* namespace nextlabs */

#endif /* __EFRAMEWORK_OPERATING_SYSTEM_HPP__ */
