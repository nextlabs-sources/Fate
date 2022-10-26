
#ifndef __PCS_IDL_HPP__
#define __PCS_IDL_HPP__

#include <windows.h>
#include "CEsdk.h"
#include "pcs_rpc_packer.hpp"

extern "C" CEResult_t FakeServiceInvoke(CEHandle h, CEString serviceName, CEString fmt, void **request, void ***response, CEint32 timeout);
extern "C" CEResult_t FakeServiceResponseFree(void **response);

typedef struct
{
  char file[MAX_PATH];
} SEFiles;

#endif /* __PCS_IDL_HPP__ */
