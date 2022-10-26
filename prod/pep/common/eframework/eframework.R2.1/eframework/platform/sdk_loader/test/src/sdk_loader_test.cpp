
#include <windows.h>
#include <cassert>
#include <cstdlib>

#include <eframework/platform/policy_controller.hpp>
#include <eframework/platform/cesdk_loader.hpp>

int main(void)
{
  nextlabs::cesdk_loader cesdk;

  if( cesdk.load(L"C:\\Program Files\\Nextlabs\\Policy Controller\\bin") == false )
  {
    fprintf(stderr, "cannot load cesdk\n");
  }
  if( cesdk.is_loaded() == true )
  {
    fprintf(stdout, "loaded\n");
  }
  cesdk.unload();
  fprintf(stdout, "unloaded\n");

  if( cesdk.is_loaded() == true )
  {
    fprintf(stderr, "failed: is_loaded() true after unload\n");
    exit(1);
  }

  if( cesdk.load(L"C:\\no such path") == true )
  {
    fprintf(stderr, "failed: load() true from invalid path\n");
    exit(1); 
  }

  if( cesdk.is_loaded() == true )
  {
    fprintf(stderr, "failed: is_loaded() false after successful load().\n");
    exit(1);
  }

  return 0;
}/* main */
