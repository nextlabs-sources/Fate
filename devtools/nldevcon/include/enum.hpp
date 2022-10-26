
#pragma once
#ifndef _NL_DEVCON_ENUM_HPP_
#define _NL_DEVCON_ENUM_HPP_
#include "devregprop.hpp"

typedef struct _NLDEVCON_ENUM_RESTRICTIONS
{
    LPCWSTR ClassName;
    LPCWSTR ClassGuid;
    LPCWSTR HwId;
}NLDEVCON_ENUM_RESTRICTIONS, *PNLDEVCON_ENUM_RESTRICTIONS;
typedef const NLDEVCON_ENUM_RESTRICTIONS* LPCNLDEVCON_ENUM_RESTRICTIONS;

typedef int (*EnumCallbackFunc)(__in HDEVINFO Devs, __in PSP_DEVINFO_DATA DevInfo, __in const CDevRegProp& DevProps, __in DWORD Index, __in_opt LPVOID Context);

int cmdQuery(__in LPCNLDEVCON_ENUM_RESTRICTIONS Restrictions);
int EnumerateDevices(__in LPCNLDEVCON_ENUM_RESTRICTIONS Restrictions, __in DWORD Flags, __in EnumCallbackFunc Callback, __in_opt LPVOID Context);

#endif