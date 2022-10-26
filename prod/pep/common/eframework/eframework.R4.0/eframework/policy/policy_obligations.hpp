/**********************************************************************************************
 *
 * policy_obligations.hpp
 *
 *
 *********************************************************************************************/

#ifndef __EFRAMEWORK_POLICY_OBLIGATIONS_HPP__
#define __EFRAMEWORK_POLICY_OBLIGATIONS_HPP__

#include <windows.h>
#include <cstdio>
#include <cassert>
#include <string>
#include <list>
#include <map>

namespace nextlabs
{
  /********************************************************************************************
   *
   * Obligation Abstraction
   *
   *   An obligation set is a collection of obligations.  Each obligation is a set of options
   *   which are key values, the obligation name and the policy it is associated with.
   *
   *   Class               Method
   *   ---------------------------------------------------------------------------------
   *   obligations
   *      |
   *   obligation          GetObligationValue - Read the string value of an obligation.
   *      |                IsObligationSet    - Does the obligation exist?
   *      |
   *   obligation_option
   *
   *******************************************************************************************/

  class policy_obligation;

  /** obligation_option
   *
   *  Key-value pair which represents an option to an obligatoin.
   */
  typedef std::pair<std::wstring,std::wstring> policy_obligation_option;

  /** obligation_options
   *
   *  A set of obligation_option objects.
   */
  typedef std::list<policy_obligation_option> policy_obligation_options;

  typedef std::list<policy_obligation> policy_obligation_list;

  /** obligation
   *
   *  obligation object which contains all key-value pairs for an obligation
   *  and the policy which it blongs to.
   */
  class policy_obligation
  {
    public:

      /** get_obligation_value
       *
       *  \brief Retreive the value of an obligation key.
       *  \return true when the value exist and the value is obtained, otherwise false.
       */
      bool get_obligation_value( _In_z_  const wchar_t* ob_key ,
				 _Out_ std::wstring& ob_value ) const
      {
	assert( ob_key != NULL );
	if( ob_key == NULL )
	{
	  return false;
	}
	for( policy_obligation_options::const_iterator it = options.begin() ; it != options.end() ; ++it )
	{
	  if( it->first == ob_key )
	  {
	    ob_value = it->second;
	    return true;
	  }
	}
	return false;
      }/* get_obligation_value */

      /** IsobligationSet
       *
       *  \brief Determine if an obligation is set.
       *  \return true if the key exists, otherwise false.
       */
      bool is_obligation_set( _In_z_ const wchar_t* ob_key ) const
      {
	assert( ob_key != NULL );
	if( ob_key == NULL )
	{
	  return false;
	}
	for( policy_obligation_options::const_iterator it = options.begin() ; it != options.end() ; ++it )
	{
	  if( it->first == ob_key )
	  {
	    return true;
	  }
	}
	return false;
      }/* is_obligation_set */

      _Ret_z_ const wchar_t* get_name(void) const
      {
	return name.c_str();
      }/* get_policy */

      _Ret_z_ const wchar_t* get_policy(void) const
      {
	return policy.c_str();
      }/* get_policy */

      _Ret_ const nextlabs::policy_obligation_options& get_options(void) const
      {
	return options;
      }/* get_options */

      std::wstring name;         /* obligation Name */
      std::wstring policy;       /* Policy Name */
      nextlabs::policy_obligation_options options; /* Set of obligation options */
  };/* policy_obligation */

  /** policy_obligations
   *
   *  Set of obligations.
   */
  class policy_obligations
  {
    public:

      /** assign
       *
       *  \brief Assign obligations from an attributes (CEAttributes_t) structure.
       */
      void assign(_In_ const std::list< std::pair<std::wstring,std::wstring> >& obligations )
      {
	WCHAR temp_key[128] = {0};
	std::wstring wsvalue;
	int num_obs = 0;

	wsvalue.reserve(256);  /* avoid excessive allocations */

	/********************************************************************************
	 * obligation Count
	 *******************************************************************************/
	if( get_value(obligations,L"CE_ATTR_OBLIGATION_COUNT",wsvalue) == false &&
	    wsvalue.size() == 0 )
	{
	  return;
	}

	num_obs = _wtoi(wsvalue.c_str());
	if( num_obs <= 0 )   /* Are there obligations? */
	{
	  return;
	}
	/********************************************************************************
	 * Extract num_obs obligations or die trying.
	 *******************************************************************************/
	bool result = true;
	for( int i = 1 ; i <= num_obs ; ++i ) /* obligations [1,n] not [0,n-1] */
        {
	  nextlabs::policy_obligation ob;          /* obligation */
	  int num_values = 0;     /* Values to read from the obligation */

	  _snwprintf_s(temp_key,_countof(temp_key), _TRUNCATE,L"CE_ATTR_OBLIGATION_NAME:%d",i);           /* Name */
	  result &= get_value(obligations,temp_key,ob.name); 
	  _snwprintf_s(temp_key,_countof(temp_key), _TRUNCATE,L"CE_ATTR_OBLIGATION_POLICY:%d",i);         /* Policy */
	  result &= get_value(obligations,temp_key,ob.policy);
	  _snwprintf_s(temp_key,_countof(temp_key), _TRUNCATE,L"CE_ATTR_OBLIGATION_NUMVALUES:%d",i);      /* # of Values */
	  result &= get_value(obligations,temp_key,wsvalue);
	  num_values = _wtoi(wsvalue.c_str());

	  if( result != true )
	  {
	    continue;
	  }

	  /********************************************************************************
	   * Extract Values or "options" of the obligation in pairs {key,value}
	   *******************************************************************************/
	  result = true;
	  for( int j = 1 ; j <= num_values ; j += 2 ) /* obligations [1,n] not [0,n-1] */
	  {
	    nextlabs::policy_obligation_option option;
	    _snwprintf_s(temp_key,_countof(temp_key), _TRUNCATE,L"CE_ATTR_OBLIGATION_VALUE:%d:%d",i,j);    /* Key */
	    get_value(obligations,temp_key,option.first);
	    _snwprintf_s(temp_key,_countof(temp_key), _TRUNCATE,L"CE_ATTR_OBLIGATION_VALUE:%d:%d",i,j+1);  /* Value */
	    get_value(obligations,temp_key,option.second);
	    ob.options.push_back(option);                                   /* Add Option */
	  }/* for j */
	  obs.push_back(ob);               /* Add obligation to obligation list */
	}/* for i */
      }/* assign */

      /** begin
       *
       *  \brief First iterator of the obligations.
       */
      policy_obligation_list::const_iterator begin(void) const
      {
	return obs.begin();
      }/* begin */

      /** end
       *
       *  \brief Last iterator of the obligations.
       */
      policy_obligation_list::const_iterator end(void) const
      {
	return obs.end();
      }/* end */

      /** is_obligation_set
       *
       *  \brief Determine if an obligation exists, or is set.
       */
      bool is_obligation_set( _In_z_ const wchar_t* in_name ) const
      {
	if( find(in_name) != obs.end() )
	{
	  return true;
	}
	return false;
      }/* is_obligation_set */

      /** find
       *
       *  \brief Find an obligation in the set of obligations.
       */
      _Ret_ policy_obligation_list::const_iterator find(_In_opt_ const wchar_t* in_name ) const
      {
	assert( in_name != NULL );
	policy_obligation_list::const_iterator it;
	for( it = obs.begin() ; it != obs.end() ; ++it )
	{
	  if( it->name == in_name )
	  {
	    break;
	  }
	}
	return it;
      }/* find */

      /** get_obligations
       *
       *  \brief Retrieve the obligation set.
       */
      _Ret_ const policy_obligation_list& get_obligations(void)
      {
	return obs;
      }/* get_obligations */

    private:

      /** get_value
       *
       *  \brief Extract the value of a key in the CEAttributes structure.  If the value of the
       *         option is NULL the out value (in_value) for the caller will be an empty string.
       *         For this reason, it is not possible to distinguish an empty and NULL string
       *         value in an obligation structure.
       *
       *  \return true if the value is found and assigned to the callers
       *          string object, otherwise false.
       */
       bool get_value(_In_ const std::list< std::pair<std::wstring,std::wstring> >& obligations ,
		       _In_z_ const wchar_t* in_key ,
		       _Out_ std::wstring& in_value )
      {
	assert( in_key != NULL );

	if( in_key == NULL )
	{
	  return false;
	}

	/* find the key's value */
	std::list< std::pair<std::wstring,std::wstring> >::const_iterator it;
	for( it = obligations.begin() ; it != obligations.end() ; ++it )
	{
	  if( in_key == it->first )
	  {
	    in_value.assign(it->second);
	    return true;
	  }
	}

	return false;
      }/* get_value */

      policy_obligation_list obs;

  };/* class policy_obligations */

}/* namespace nextlabs */

#endif /* __EFRAMEWORK_POLICY_OBLIGATIONS_HPP__ */
