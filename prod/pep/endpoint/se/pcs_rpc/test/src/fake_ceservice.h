
#ifndef __FAKE_CESERVICE__H__
#define __FAKE_CESERVICE__H__

//#include <cstdlib>
#include <windows.h>
#include "CEsdk.h"

/* For interface details see actual ceservice.h */

extern "C"
CEResult_t FakeServiceInvoke(CEHandle h, CEString serviceName, CEString fmt, void **request, void ***response, CEint32 timeout);

extern "C"
CEResult_t FakeServiceResponseFree(void **response);

#endif /* __FAKE_CESERVICE__H__ */
