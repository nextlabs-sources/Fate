/***************************************************************************************************
 *
 * Determine if the Policy Controller is up (running)
 *
 **************************************************************************************************/

#ifndef __POLICY_CONTROLLER_HPP__
#define __POLICY_CONTROLLER_HPP__

#include <windows.h>

namespace nextlabs
{

  /** POLICY_CONTROLLER_UUID
   *
   *  \brief Global identifier for shared memory where the Policy Controller stores
   *         its PID.
   */
  static const wchar_t* POLICY_CONTROLLER_UUID = L"Global\\b67546e2-6dc7-4d07-aa8a-e1647d29d4d7";

  class policy_controller
  {

    public:

      /** is_up
       *
       *  \brief Determine if the Policy Controller is running.
       *  \return true if the Policy Controller is running, otherwise false.
       *          To determine if an error occurred check last error status
       *          using GetLastError.
       */
      static bool is_up(void) throw()
      {
	SetLastError(ERROR_SUCCESS); /* Clear last error */
	HANDLE hPIDFileMapping = OpenFileMappingW(FILE_MAP_READ,FALSE,POLICY_CONTROLLER_UUID);
	bool result = false;
	if( hPIDFileMapping != NULL )
	{
	  result = true;
	  CloseHandle(hPIDFileMapping);
	}
	return result;
      }/* is_up */

      /** pid
       *
       *  \brief Retrieve the process ID of the Policy Controller.
       *  \return Non-zero PID on success.  Zero on failure.  Use GetLastError
       *          to determine what error occurred on failure.
       */
      static DWORD pid(void) throw()
      {
	SetLastError(ERROR_SUCCESS); /* Clear last error */
	DWORD out_pid = 0;
	HANDLE hPIDFileMapping = OpenFileMapping(FILE_MAP_READ,FALSE,POLICY_CONTROLLER_UUID); 
	if( hPIDFileMapping == NULL )
	{
	  return out_pid;
	}

	DWORD* pid = (DWORD*) MapViewOfFile(hPIDFileMapping,FILE_MAP_READ,0,0,0);
	if( pid != NULL )
	{
	  out_pid = *pid;
	  UnmapViewOfFile(pid);
	}
	CloseHandle(hPIDFileMapping);
	return out_pid;
      }/* pid */
    
  };/* policy_controller */

}/* namespace nextlabs */

#endif /* __POLICY_CONTROLLER_HPP__ */
