/*============================nlse_test.cpp=================================*
 *                                                                          *
 * All sources, binaries and HTML pages (C) copyright 2006 by NextLabs,     *
 * San Mateo CA, Ownership remains with NextLabs Inc,                       * 
 * All rights reserved worldwide.                                           *
 *                                                                          * 
 * Author : Heidi Zhou                                                      *
 * Date   : 11/28/2009                                                      *
 * Note   : Test nlse driver and plugin                                     *
 *==========================================================================*/
#define WINVER _WIN32_WINNT_WINXP
#define _WIN32_WINNT _WIN32_WINNT_WINXP
#define NTDDI_VERSION NTDDI_WINXPSP2

#include <Windows.h>
#include <list>
#include <vector>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <winsock2.h>
#include <cassert>
#include <fltuser.h>
#include <tchar.h>
#include <WinIoCtl.h>
#include <Sddl.h>
#include "NLSECommon.h"
#include "NLSELib.h"

extern "C" __declspec(dllexport) int PluginUnload( void* in_context );
extern "C" __declspec(dllexport) int PluginEntry( void** in_context );

int main()
{
  void* pluginContext;
  int ret;

  ret=PluginEntry(&pluginContext);
  
  if(ret == 0) {
    printf("PluginEntry failed\n");
    return 0;
  }

  getchar();

  ret=PluginUnload(pluginContext);
  if(ret == 0) {
    printf("PluginUnload failed\n");
  }
}
