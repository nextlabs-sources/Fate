
#ifndef __EFRAMEWORK_CESDK_STRING_HPP__
#define __EFRAMEWORK_CESDK_STRING_HPP__

#include "CEsdk.h"
#include "cetype.h"

namespace nextlabs
{

  class cesdk_cem
  {
  public:
    static const wchar_t* CEM_GetString(CEString cestr)
    {
      if( cestr == NULL )
      {
	return NULL;
      }
      return (((_CEString*)cestr)->buf);
    }/* CEM_GetString */
  };/* class cesdk_cem */

}/* namespace nextlabs */

#endif /* __EFRAMEWORK_CESDK_STRING_HPP__ */
