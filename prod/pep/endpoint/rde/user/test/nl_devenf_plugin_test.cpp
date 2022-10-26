#include <stdio.h>

extern "C" __declspec(dllexport) int PluginEntry( void** in_context );

void main()
{
  void *context;

  PluginEntry(&context);

  getchar();
}


