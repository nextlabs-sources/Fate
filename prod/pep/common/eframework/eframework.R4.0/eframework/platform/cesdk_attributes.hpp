/**********************************************************************************************
 *
 * cesdk_attributes.hpp
 *
 * Interface to simplify CE SDK type CEAttributes.
 *
 *********************************************************************************************/

#ifndef __CESDK_ATTRIBUTES_HPP__
#define __CESDK_ATTRIBUTES_HPP__

#include <cassert>
#include <cstdlib>

#include "CEsdk.h"

#include "eframework/platform/cesdk_loader.hpp"

namespace nextlabs
{

  /** cesdk_attributes
   *
   *  \brief Abstraction for CEAttributes management.
   */
  class cesdk_attributes
  {

    public:

    /** create
     *
     *  \brief Create a CEAttributes instance.
     *  \return Pointer to a CEAttributes instance.
     *  \sa destroy
     */
    _Check_return_ static CEAttributes* create(void) throw()
    {
      CEAttributes* attrs = (CEAttributes*)malloc( sizeof(CEAttributes) );
      if( attrs != NULL )
      {
	memset(attrs,0x00,sizeof(CEAttributes));
	attrs->attrs = NULL;
	attrs->count = 0;
      }
      return attrs;
    }/* create */

    /** add
     *
     *  \brief Add an attribute to a CEAttributes instance.
     *
     *  \param cesdk (in)        nextlabs::sdk_loader instance.
     *  \param in_attrs (in-out) CEAttributes instance.
     *  \param in_key (in)       Key name.
     *  \param in_value (in)     Value of key.
     *
     *  \return true on success, otherwise false.
     *
     *  \sa create, destroy
     */
    _Check_return_ static bool add(_In_ const nextlabs::cesdk_loader& cesdk ,
				   _Inout_ CEAttributes* in_attrs ,
				   _In_z_ const wchar_t* in_key ,
				   _In_z_ const wchar_t* in_value ) throw()
    {
      assert( in_attrs != NULL && in_key != NULL && in_value != NULL );
      CEAttribute* new_attr  = (CEAttribute*)realloc(in_attrs->attrs,(in_attrs->count+1)*sizeof(CEAttribute));
      if( new_attr == NULL )
      {
	return false;
      }
      in_attrs->attrs = new_attr;
      in_attrs->count++;
      in_attrs->attrs[in_attrs->count-1].key   = cesdk.fns.CEM_AllocateString(in_key);
      in_attrs->attrs[in_attrs->count-1].value = cesdk.fns.CEM_AllocateString(in_value);
      return true;
    }/* add */

    /** destroy
     *
     *  \brief Destroy a CEAttributes instance.
     *
     *  \param cesdk (in)    nextlabs::cesdk_loader instance.
     *  \param in_attrs (in) CEAttributes instance.
     *
     *  \sa create
     */
    static void destroy(_In_ const nextlabs::cesdk_loader& cesdk ,
			 _In_ CEAttributes* in_attrs ) throw()
    {
      assert( in_attrs != NULL );
      if( in_attrs != NULL )
      {
	for( int i = 0 ; i < in_attrs->count ; i++ )
	{
	  cesdk.fns.CEM_FreeString(in_attrs->attrs[i].key);
	  cesdk.fns.CEM_FreeString(in_attrs->attrs[i].value);
	}
	if( in_attrs->attrs != NULL )
	{
	  free(in_attrs->attrs);
	}
	free(in_attrs);
      }
    }/* destroy */

  };/* cesdk_attributes */

};/* namespace nextlabs */

#endif /* __CESDK_ATTRIBUTES_HPP__ */
