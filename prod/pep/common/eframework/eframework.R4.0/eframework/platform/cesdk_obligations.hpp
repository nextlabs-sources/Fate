/**********************************************************************************************
 *
 * obligations.hpp
 *
 * Interface to simplify SDK use.  Patters of SDK use are wrapped for rapid development.
 *
 *********************************************************************************************/

#ifndef __CESDK_OBLIGATIONS_HPP__
#define __CESDK_OBLIGATIONS_HPP__

#include <windows.h>
#include <cstdio>
#include <cassert>
#include <tchar.h>
#include <string>
#include <list>
#include <map>

#include "CEsdk.h"

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
   *   Obligations
   *      |
   *   Obligation          GetObligationValue - Read the string value of an obligation.
   *      |                IsObligationSet    - Does the obligation exist?
   *      |
   *   ObligationOption
   *
   *******************************************************************************************/

  /** ObligationOption
   *
   *  Key-value pair which represents an option to an obligatoin.
   */
  typedef std::pair<std::wstring,std::wstring> ObligationOption;

  /** ObligationOptions
   *
   *  A set of ObligationOption objects.
   */
  typedef std::list<ObligationOption>          ObligationOptions;

  /** Obligation
   *
   *  Obligation object which contains all key-value pairs for an obligation
   *  and the policy which it blongs to.
   */
  class Obligation
  {
    public:
      /** GetObligationValue
       *
       *  \brief Retreive the value of an obligation key.
       *  \return true when the value exist and the value is obtained, otherwise false.
       */
      bool GetObligationValue( _In_opt_z_  const wchar_t* ob_key ,
			       _Out_ std::wstring& ob_value ) const
      {
	assert( ob_key != NULL );
	if( ob_key == NULL )
	{
	  return false;
	}
	for( ObligationOptions::const_iterator it = options.begin() ; it != options.end() ; ++it )
	{
	  if( it->first == ob_key )
	  {
	    ob_value = it->second;
	    return true;
	  }
	}
	return false;
      }/* GetObligationValue */

      /** IsObligationSet
       *
       *  \brief Determine if an obligation is set.
       *  \return true if the key exists, otherwise false.
       */
      bool IsObligationSet( _In_opt_z_ const wchar_t* ob_key ) const
      {
	assert( ob_key != NULL );
	if( ob_key == NULL )
	{
	  return false;
	}
	for( ObligationOptions::const_iterator it = options.begin() ; it != options.end() ; ++it )
	{
	  if( it->first == ob_key )
	  {
	    return true;
	  }
	}
	return false;
      }/* IsObligationSet */

      std::wstring name;         /* Obligation Name */
      std::wstring policy;       /* Policy Name */
      ObligationOptions options; /* Set of obligation options */
  };/* Obligation */

  /** Obligations
   *
   *  Set of obligations.
   */
  class Obligations
  {
    public:

      /** GetValue
       *
       *  \brief Extract the value of a key in the CEAttributes structure.  If the value of the
       *         option is NULL the out value (in_value) for the caller will be an empty string.
       *         For this reason, it is not possible to distinguish an empty and NULL string
       *         value in an obligation structure.
       *
       *  \return true if the value is found and assigned to the callers
       *          string object, otherwise false.
       */
      static bool GetValue( _In_opt_ const WCHAR* (*pfn_CEM_GetString)(CEString) ,
			    _In_ CEAttributes& obligations ,
			    _In_opt_z_ const wchar_t* in_key ,
			    _Out_ std::wstring& in_value )
      {
	assert( pfn_CEM_GetString != NULL );
	assert( in_key != NULL );

	if( pfn_CEM_GetString == NULL || in_key == NULL )
	{
	  return false;
	}

	in_value.clear();
	for( int i = 0 ; i < obligations.count ; ++i )
	{
	  const WCHAR* key = pfn_CEM_GetString(obligations.attrs[i].key);
	  if( key == NULL || wcscmp(key,in_key) != 0 )
	  {
	    continue;
	  }

	  /* Extract value.  NULL value indicates empty which is handled by clearing
	   * the input string before iteration.
	   */
	  const WCHAR* value = pfn_CEM_GetString(obligations.attrs[i].value);
	  if( value != NULL )
	  {
	    in_value = value;
	  }
	  return true;
	}
	return false;
      }/* GetValue */

      /** Assign
       *
       *  \brief Assign obligations from an attributes (CEAttributes_t) structure.
       */
      void Assign( _In_opt_ const WCHAR* (*pfn_CEM_GetString)(CEString) ,
		   _In_ CEAttributes& obligations )
      {
	assert( pfn_CEM_GetString != NULL );
	if( pfn_CEM_GetString == NULL || obligations.attrs == NULL )
	{
	  return;
	}
	WCHAR temp_key[128];
	std::wstring wsvalue;
	int num_obs = 0;

	wsvalue.reserve(128);  /* avoid excessive allocations */

	/********************************************************************************
	 * Obligation Count
	 *******************************************************************************/
	_snwprintf_s(temp_key,_countof(temp_key), _TRUNCATE,L"CE_ATTR_OBLIGATION_COUNT");
	if( GetValue(pfn_CEM_GetString,obligations,temp_key,wsvalue) == false &&
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
	for( int i = 1 ; i <= num_obs ; ++i ) /* Obligations [1,n] not [0,n-1] */
        {
	  Obligation ob;          /* Obligation */
	  int num_values = 0;     /* Values to read from the obligation */

	  _snwprintf_s(temp_key,_countof(temp_key), _TRUNCATE,L"CE_ATTR_OBLIGATION_NAME:%d",i);           /* Name */
	  result &= GetValue(pfn_CEM_GetString,obligations,temp_key,ob.name); 
	  _snwprintf_s(temp_key,_countof(temp_key), _TRUNCATE,L"CE_ATTR_OBLIGATION_POLICY:%d",i);         /* Policy */
	  result &= GetValue(pfn_CEM_GetString,obligations,temp_key,ob.policy);
	  _snwprintf_s(temp_key,_countof(temp_key), _TRUNCATE,L"CE_ATTR_OBLIGATION_NUMVALUES:%d",i);      /* # of Values */
	  result &= GetValue(pfn_CEM_GetString,obligations,temp_key,wsvalue);
	  num_values = _wtoi(wsvalue.c_str());

	  if( result != true )
	  {
	    continue;
	  }

	  /********************************************************************************
	   * Extract Values or "options" of the obligation in pairs {key,value}
	   *******************************************************************************/
	  result = true;
	  for( int j = 1 ; j <= num_values ; j += 2 ) /* Obligations [1,n] not [0,n-1] */
	  {
	    ObligationOption option;
	    _snwprintf_s(temp_key,_countof(temp_key), _TRUNCATE,L"CE_ATTR_OBLIGATION_VALUE:%d:%d",i,j);    /* Key */
	    GetValue(pfn_CEM_GetString,obligations,temp_key,option.first);
	    _snwprintf_s(temp_key,_countof(temp_key), _TRUNCATE,L"CE_ATTR_OBLIGATION_VALUE:%d:%d",i,j+1);  /* Value */
	    GetValue(pfn_CEM_GetString,obligations,temp_key,option.second);
	    ob.options.push_back(option);                                   /* Add Option */
	  }/* for j */
	  obs.push_back(ob);               /* Add obligation to obligation list */
	}/* for i */
      }/* Assign */

      /** begin
       *
       *  \brief First iterator of the obligations.
       */
      std::list<Obligation>::const_iterator begin(void) const
      {
	return obs.begin();
      }/* begin */

      /** end
       *
       *  \brief Last iterator of the obligations.
       */
      std::list<Obligation>::const_iterator end(void) const
      {
	return obs.end();
      }/* end */

      /** IsObligationSet
       *
       *  \brief Determine if an obligation exists, or is set.
       */
      bool IsObligationSet( _In_opt_z_ const wchar_t* ob_name ) const
      {
	assert( ob_name != NULL );
	if( ob_name == NULL )
	{
	  return false;
	}
	std::list<Obligation>::const_iterator it;
	for( it = obs.begin() ; it != obs.end() ; ++it )
	{
	  if( it->name == ob_name )
	  {
	    return true;
	  }
	}
	return false;
      }/* IsObligationSet */

      const std::list<Obligation>& GetObligations(void)
      {
	return obs;
      }/* GetObligations */

    private:
      std::list<Obligation> obs;
  };/* class Obligations */

}/* namespace nextlabs */

#endif /* __CESDK_OBLIGATIONS_HPP__ */
