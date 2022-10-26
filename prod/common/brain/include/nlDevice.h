#ifndef _NL_DEVICE_H
#define _NL_DEVICE_H
#include <string>
#include <stddef.h>
#include <hash_map>
#include <Winioctl.h>
#include "nltypes.h"
#include "nlthread.h"

//using namespace std;

#if defined (WIN32) || defined (_WIN64)
class DeviceDetc
{
private:
  std::nlstring cdBurningFolder;
  nlthread_cs_t csRMCache;
  stdext::hash_map<std::nlstring, BOOL> removableMediaCache;

  STORAGE_BUS_TYPE GetBusType(const nlchar disk);
  BOOL IsRemovableBusType(const nlchar *driver);

public:
  DeviceDetc(void);
  ~DeviceDetc(void);

  BOOL IsRemovableMedia (const nlchar *fileName);
};

#endif //end ifdef WIN32 || _WIN64

#endif
