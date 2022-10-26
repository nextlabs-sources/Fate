#ifndef CESERVICE_H
#define CESERVICE_H

#include "cesdk.h"

CEResult_t ServiceInvoke(CEHandle h, CEString serviceName, CEString fmt, void **request, void ***response, CEint32 timeout);
CEResult_t ServiceResponseFree(void **response);
 
#endif
