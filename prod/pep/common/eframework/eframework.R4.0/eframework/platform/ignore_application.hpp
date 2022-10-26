/***************************************************************************************************
 *
 * Ignored application policy decision
 *
 **************************************************************************************************/

#ifndef __IGNORE_APPLICATION_HPP__
#define __IGNORE_APPLICATION_HPP__

#include <windows.h>
#include <cassert>
#include <tchar.h>

#include "boost/utility.hpp"

#include "eframework/os/operating_system.hpp"
#include "eframework/platform/policy_controller.hpp"

namespace nextlabs
{

  /** application
   *
   *  \brief Ignored application policy decision.
   */
  class application : boost::noncopyable
  {
    public:

    /** is_ignored
     *
     *  \brief Determine if the current process should be ignored.
     *  \return If the process should be ignored true is returned, otherwise false is returned.
     *          If GetLastError does not return ERROR_SUCCESS, then is_ignored has failed.
     */
    static bool is_ignored(void) throw()
    {
      /* determine current image */
      wchar_t image_path[MAX_PATH] = {0};
      GetModuleFileNameW(NULL,image_path,MAX_PATH);
      return is_ignored(image_path);
    }/* is_ignored */

    /** is_ignored
     *
     *  \brief Determine if the current process should be ignored.  If the Policy Controller is
     *         not running the application will be ignored - the return value will be true.
     *
     *  \param in_image_path (in) The full image path to the executable.
     *
     *  \return If the process should be ignored true is returned, otherwise false is returned.
     *          If GetLastError does not return ERROR_SUCCESS, then is_ignored has failed.
     */
    static bool is_ignored( _In_opt_z_ const wchar_t* in_image_path ) throw()
    {
      assert( in_image_path );
      if( in_image_path == NULL )
      {
	SetLastError(ERROR_INVALID_PARAMETER);
	return false;
      }

      /* If the Policy Controller is down avoid trying to connect up using IPC channel. */
      if( nextlabs::policy_controller::is_up() == false )
      {
	SetLastError(ERROR_SUCCESS);
	return true;
      }

      /* @@ TBD */

      return false;
    }/* is_ignored */

    private:

  };/* application */

}/* namespace nextlabs */

#endif /* __IGNORE_APPLICATION_HPP__ */
