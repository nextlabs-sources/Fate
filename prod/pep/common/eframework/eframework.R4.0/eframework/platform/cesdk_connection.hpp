/***************************************************************************************************
 *
 * CE SDK Connection Management
 *
 **************************************************************************************************/

#ifndef __CESDK_CONNECTION_HPP__
#define __CESDK_CONNECTION_HPP__

#include <cassert>

#include "boost/utility.hpp"

#include "CEsdk.h"

#include "eframework/platform/policy_controller.hpp"
#include "eframework/platform/cesdk_loader.hpp"

namespace nextlabs
{

  /** cesdk_connection
   *
   *  \brief SDK Connection abstraction
   */
  class cesdk_connection : boost::noncopyable
  {
    public:

      cesdk_connection(void) :
	connected(false),
	last_error(CE_RESULT_SUCCESS),
	timeout(5000),
	app_path()
      {
	memset(&connHandle,0x00,sizeof(connHandle));
      }

      void set_sdk( _In_ nextlabs::cesdk_loader* in_cesdk ) throw()
      {
	assert( in_cesdk != NULL );
	cesdk = in_cesdk;
      }/* set_sdk */

      /** set_timeout
       *
       *  \brief Set the CE SDK timeout in milliseconds.
       *  \sa connect, disconnect
       */
      void set_timeout( _In_ size_t in_timeout ) throw()
      {
	timeout = (CEint32)in_timeout;
      }/* set_timeout */

      /** connect
       *
       *  \brief Establish a connection to the Policy Controller.
       *  \return true if connection is successful, otherwise false.
       *  \sa get_last_error, set_timeout
       */
      _Check_return_ bool connect(void) throw()
      {
	CEApplication app;
	CEUser user;

	memset(&app,0x00,sizeof(CEApplication));
	memset(&user,0x00,sizeof(CEUser));

	app.appPath = cesdk->fns.CEM_AllocateString(app_path.c_str());
	for (int i = 0; i < 2; i++)
	{
		last_error = cesdk->fns.CECONN_Initialize(app,user,NULL,&connHandle,timeout);
		if( last_error == CE_RESULT_SUCCESS )
		{
			connected = true;
			break;
		}
		else
		{
			if(2 == i)
			{
				break;
			}

			if (last_error == CE_RESULT_THREAD_NOT_INITIALIZED)
			{
				Sleep(200);
			}
			else
			{
				break;
			}
		}
	}

	cesdk->fns.CEM_FreeString(app.appPath);
	return last_error == CE_RESULT_SUCCESS;
      }/* connect */

      /** set_app_path
       *
       *  \brief Set the application path for the current connection.  The default path
       *         is empty.
       */
      void set_app_path( _In_z_ const wchar_t* in_app_path )
      {
	assert( in_app_path != NULL );
	app_path.assign(in_app_path);
      }/* set_app_path */

      /** get_connection_handle
       *
       *  \brief Return the connection handle.
       *  \return CEHandle of current connection.
       */
      CEHandle& get_connection_handle(void) throw()
      {
	return connHandle;
      }/* get_connection_handle */

      /** disconnect
       *
       *  \brief Disconnect from the Policy Controller.
       */
      bool disconnect(void) throw()
      {
	last_error = cesdk->fns.CECONN_Close(connHandle,timeout);
	memset(&connHandle,0x00,sizeof(connHandle));
	connected = false;
	return last_error == CE_RESULT_SUCCESS;
      }/* disconnect */

      /** is_connected
       *
       *  \brief Determine if the connection to the Policy Controller is active.
       *  \return true if connected to the Policy Controller, otherwise false.
       */
      bool is_connected(void) const throw()
      {
	return connected;
      }/* is_connected */

      /** get_last_error
       *
       *  \brief Return the last CE SDK error.
       */
      CEResult_t get_last_error(void) const throw()
      {
	return last_error;
      }/* get_last_error */

    private:

      nextlabs::cesdk_loader* cesdk;  /* CE SDK pointers */
      CEHandle connHandle;            /* Current connection */
      bool connected;                 /* Currently connected? */
      CEResult_t last_error;          /* Last CE SDK error */
      CEint32 timeout;                /* Timeout */
      std::wstring app_path;          /* Application path */

  };/* cesdk_connection */

}/* namespace nextlabs */

#endif /* __CESDK_CONNECTION_HPP__ */
