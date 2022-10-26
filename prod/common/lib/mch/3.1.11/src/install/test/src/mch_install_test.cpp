
#include <windows.h>
#include <cstdio>
#include <cstdlib>

extern "C"
int nextlabs_install(void);

extern "C"
int nextlabs_uninstall(void);

int main( int argc , char** argv )
{

  if( argc != 2 ||
      ( strcmp(argv[1],"install") && strcmp(argv[1],"remove") ) )
  {
    fprintf(stdout, "usage: mch_install_test [install|remove]\n");
    return 1;
  }

  int rv = -1;
  if( strcmp(argv[1],"install") == 0 )
  {
    fprintf(stdout, "installing\n");
    rv = nextlabs_install();
  }

  if( strcmp(argv[1],"remove") == 0 )
  {
    fprintf(stdout, "removing\n");
    rv = nextlabs_uninstall();
  }

  return rv;
}/* main */
