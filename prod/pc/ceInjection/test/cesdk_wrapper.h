
#ifndef __CESDK_WRAPPER_H__
#define __CESDK_WRAPPER_H__

#include <windows.h>
#include <tchar.h>

#include "CEsdk.h"

namespace cesdk_wrapper
{
  __declspec(dllexport) bool Init(void);
  __declspec(dllexport) bool Connect(void);
  __declspec(dllexport) bool Disconnect(void);
  __declspec(dllexport) bool Eval( CEAction_t action ,
				   const TCHAR* resource_from ,
				   const TCHAR* resource_to = L"" );
};

#endif /* __CESDK_WRAPPER_H__ */
