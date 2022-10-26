/***************************************************************************************************
 *
 * Ignored application policy decision
 *
 **************************************************************************************************/

#ifndef __POLICY_MONITOR_APPLICATION_HPP__
#define __POLICY_MONITOR_APPLICATION_HPP__

#include <windows.h>
#include <cassert>

#include "boost/utility.hpp"

#include "eframework/os/operating_system.hpp"
#include "eframework/platform/policy_controller.hpp"
#include "eframework/policy/policy_query.hpp"
#include "eframework/policy/policy_connection.hpp"

namespace nextlabs
{

  /** policy_monitor_application
   *
   *  \brief Ignored application policy decision.
   */
  class policy_monitor_application : boost::noncopyable
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
    static bool is_ignored( _In_z_ const wchar_t* in_image_path ) throw()
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

      nextlabs::policy_connection pconn;
      nextlabs::policy_query pquery;

      if( pconn.connect() == false )
      {
	return false;
      }

      /* Action and image path are required */
      pquery.set_action(L"ce::monitor_application");
      pquery.set_application(in_image_path);

      bool result = pconn.query(pquery);
      pconn.disconnect();

      /* Ignore this application if (1) the evaluation cannot occur OR (2) the policy
       * says that "monitor application" is *not* allowed (denied).
       */
      if( result == false || pquery.is_allow() == false )
      {
	return true;
      }
      return false;
    }/* is_ignored */

  };/* policy_monitor_application */

}/* namespace nextlabs */

#endif /* __POLICY_MONITOR_APPLICATION_HPP__ */
