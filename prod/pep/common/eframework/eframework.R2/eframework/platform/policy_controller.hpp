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

  class policy_controller
  {

    public:

      /** is_up
       *
       *  \brief Determine if the Policy Controller is running.
       *  \return true if the Policy Controller is running, otherwise false.
       */
      static bool is_up(void)
      {
	HANDLE hPIDFileMapping = OpenFileMappingA(FILE_MAP_WRITE,FALSE,"Global\\b67546e2-6dc7-4d07-aa8a-e1647d29d4d7");
	bool result = false;
	if( hPIDFileMapping != NULL )
	{
	  result = true;
	  CloseHandle(hPIDFileMapping);
	}
	return result;
      }/* IsPolicyControllerUp */

  };/* policy_controller */

}/* namespace nextlabs */

#endif /* __POLICY_CONTROLLER_HPP__ */
