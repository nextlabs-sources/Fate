/***************************************************************************************************
 *
 * Policy Connection
 *
 **************************************************************************************************/

#ifndef __EFRAMEWORK_POLICY_CONNECTION_HPP__
#define __EFRAMEWORK_POLICY_CONNECTION_HPP__

#include <cassert>
#include <boost/utility.hpp>

#pragma warning( push )
#pragma warning( disable : 6011 4996)
#  include <boost/algorithm/string.hpp>
#pragma warning( pop )

#include <eframework/timer/timer_high_resolution.hpp>
#include <eframework/platform/policy_controller.hpp>
#include <eframework/policy/policy_query.hpp>

#include "nlcc.h"
#include "nlcc_ioctl.h"
#include "nlcc_ulib.h"

#include <windows.h>
#include <sddl.h>

namespace nextlabs
{
  /** policy_connection
   *
   *  \brief Connection abstraction to the PDP.
   */
  class policy_connection : boost::noncopyable
  {
    public:

      policy_connection(void) :
	connected(false),
	timeout(5000),
	app_path(),
	user_id()
      {
	memset(&nlcc_handle,0x00,sizeof(nlcc_handle));
	pid = GetCurrentProcessId();
      }/* policy_connection */

      /** set_timeout
       *
       *  \brief Set the default timeout in milliseconds.
       *  \sa connect, disconnect
       */
      void set_timeout( _In_ size_t in_timeout ) throw()
      {
	timeout = (int)in_timeout;
      }/* set_timeout */

      /** connect
       *
       *  \brief Establish a connection to the Policy Controller.
       *  \return true if connection is successful, otherwise false.
       *  \sa disconnect, set_timeout
       */
      _Check_return_ bool connect(void) throw()
      {
	if( NLCC_UOpen(&nlcc_handle) != 0 )
	{
	  return false;
	}
	return true;
      }/* connect */

      /** set_app_path
       *
       *  \brief Set the application path for the current connection.  The default path
       *         is empty.
       */
      void set_app_path( _In_ const wchar_t* in_app_path )
      {
	assert( in_app_path != NULL );
	app_path.assign(in_app_path);
      }/* set_app_path */

      /** disconnect
       *
       *  \brief Disconnect from the Policy Controller.
       *  \sa connect
       */
      bool disconnect(void) throw()
      {
	NLCC_UClose(&nlcc_handle);
	connected = false;
	return true;
      }/* disconnect */

      /** is_connected
       *
       *  \brief Determine if the connection to the Policy Controller is active.
       *  \return true if connected to the Policy Controller, otherwise false.
       */
      _Ret_ bool is_connected(void) const throw()
      {
	return connected;
      }/* is_connected */

      /** query
       *
       *  \brief Perofrm a query of the given object against the given connected PDP.

       *  \return true if the query is successful, otherwise false.  When query fails the last
       *          error should be checked.  It may take one of the following values:
       *
       *          ERROR_TIMEOUT           - Timeout expired while waiting for a PDP decision.
       *          ERROR_SERVICE_NOT_FOUND - PDP is not available.
       */
      bool query(_Inout_ nextlabs::policy_query& in_query )
      {
	PNLCC_QUERY query_objects = NULL;
	PNLCC_QUERY request = NULL;
	PNLCC_QUERY response = NULL;
	BOOL rv;
	bool result = false;
	std::list< std::pair<std::wstring,std::wstring> >::iterator it;
	std::list< std::pair<std::wstring,std::wstring> > obligations;
	std::wstring ob;
	std::wstring curr_user_id;
	std::wstring pszUserSID;

	/* If Policy Controller is down do not attempt to connect */
	if( nextlabs::policy_controller::is_up() == false )
	{
	  in_query.deny = false;
	  SetLastError(ERROR_SERVICE_NOT_FOUND);
	  return false;
	}

	assert( in_query.action.empty() == false );

	/* Allocate space for request and response objects */
	query_objects = (PNLCC_QUERY)malloc( sizeof(NLCC_QUERY) * 2 );
	if( query_objects == NULL )
	{
	  result = false;
	  goto query_complete;
	}

	request  = &query_objects[0];  /* request buffer */
	response = &query_objects[1];  /* response buffer */

	/* Initialize request and query objects */
	NLCC_UInitializeQuery(request);
	NLCC_UInitializeQuery(response);

	request->info.request.ip = 0;
	request->info.request.pid = pid;
	request->info.request.event_level = in_query.event_level;

	NLCC_UAddAttribute(request,L"action",in_query.action.c_str());

	HANDLE hToken;
	HANDLE hCurrentProcess = GetCurrentProcess();
	if( OpenProcessToken(hCurrentProcess,TOKEN_READ,&hToken) )
	{
	  DWORD dwLen = NULL;
	  //Pass NULL to get the right buffer size
	  GetTokenInformation(hToken, TokenUser, NULL, 0, &dwLen);
	  if(GetLastError() == ERROR_INSUFFICIENT_BUFFER)
	  {
	    PTOKEN_USER pTokenUser = (PTOKEN_USER) malloc (dwLen);
	    if(NULL!=pTokenUser )
	    {
	      if( GetTokenInformation(hToken,TokenUser,pTokenUser,dwLen,&dwLen) )
	      {
		DWORD dwSidLen = GetLengthSid(pTokenUser->User.Sid);
		PSID pSid = (PSID)malloc(dwSidLen);
		if (NULL!=pSid ) 
                {
		  if( CopySid(dwSidLen, pSid, pTokenUser->User.Sid) )
		  {
		    wchar_t* pszSID = NULL;
		    ConvertSidToStringSidW((PSID) pSid, &pszSID);
		    pszUserSID.assign(pszSID);
		    LocalFree (pszSID);
		  }
		  free (pSid);
		}
	      }
	      free (pTokenUser);
	    }
	  }
	  CloseHandle(hToken);
	}
	CloseHandle(hCurrentProcess);

	/* If the connection user_id is unset, then take the current process user's ID */
	if( user_id.empty() == true )
	{
	  user_id.assign(pszUserSID);
	}

	/* If the user ID is unset, then use the derived connections user ID. */
	if( in_query.user_id.empty() == true )
	{
	  curr_user_id.assign(user_id);
	}
	else
	{
	  curr_user_id.assign(in_query.user_id);
	}

	NLCC_UAddAttribute(request,L"user_id",curr_user_id.c_str());
	NLCC_UAddAttribute(request,L"host",L"malta");
	NLCC_UAddAttribute(request,L"application",in_query.app_path.c_str());

	/* Reousrces (source and target) */
	NLCC_UAddAttribute(request,L"source_name",in_query.source_name.c_str());
	NLCC_UAddAttribute(request,L"source_type",in_query.source_type.c_str());

	/* Request a cache hint from the PDP */
	NLCC_UAddAttribute(request,L"source_attr_ce::request_cache_hint",L"yes");

	/* A concrete file cannot contain '*' */
	if( boost::algorithm::contains(in_query.source_name,L"*") == true )
	{
	  NLCC_UAddAttribute(request,L"source_attr_ce::filesystemcheck",L"no");
	}

	/* If there is a target set its name and attribute */
	if( in_query.target_name.empty() == false )
	{
	  NLCC_UAddAttribute(request,L"target_name",in_query.target_name.c_str());
	  NLCC_UAddAttribute(request,L"target_type",in_query.target_type.c_str());
	}

	/* Source resource attributes.  Attributes must be prefixed with 'source_attr_' */
	for( it = in_query.source_attrs.begin() ; it != in_query.source_attrs.end() ; ++it )
	{
	  std::wstring key_name;
	  key_name.append(L"source_attr_");
	  key_name.append(it->first);
	  NLCC_UAddAttribute(request,key_name.c_str(),it->second.c_str());
	}

	/* Target resource attributes.  Attributes must be prefixed with 'target_attr_' */
	for( it = in_query.target_attrs.begin() ; it != in_query.target_attrs.end() ; ++it )
	{
	  std::wstring key_name;
	  key_name.append(L"target_attr_");
	  key_name.append(it->first);
	  NLCC_UAddAttribute(request,key_name.c_str(),it->second.c_str());
	}

	/* Perform query with timing */
	in_query.timer.start();
	rv = NLCC_UQuery(&nlcc_handle,request,response,timeout);
	in_query.timer.stop();

	in_query.deny = false;
	if( rv == 0 )
	{
	  result = true;
	  if( response->info.response.allow == 0 )
	  {
	    in_query.deny = true;
	  }
	}

	/* assign the obligations */
	for( size_t i = 0 ; ; i += 2 )
	{
	  const wchar_t* temp = NULL;
	  const wchar_t* key = NULL;
	  const wchar_t* value = NULL;

	  /* key (index)*/
	  if( NLCC_UGetAttributeByIndex(response,i,&temp,&key) != 0 )
	  {
	    break;
	  }

	  /* value (index + 1) */
	  if( NLCC_UGetAttributeByIndex(response,i + 1,&temp,&value) != 0 )
	  {
	    break;
	  }
	  obligations.push_back( std::pair<std::wstring,std::wstring>(key,value) );
	}
	in_query.obligations.assign(obligations);

      query_complete:

	if( query_objects != NULL )
	  free(query_objects);

	return result;
      }/* query */

    private:

      bool connected;                 /* Currently connected? */
      long pid;                       /* Process ID */
      int timeout;                    /* Timeout */
      std::wstring app_path;          /* Application path */
      std::wstring user_id;           /* User ID */
      NLCC_HANDLE nlcc_handle;        /* NLCC handle */

  };/* policy_connection */
}/* nextlabs */

#endif /* __EFRAMEWORK_POLICY_CONNECTION_HPP__ */
