/***************************************************************************
 *
 * This program demonstrates the use of NLEXCEPT_FILTER macros available
 * through nlexcept.h interface file.
 *
 **************************************************************************/

#include <windows.h>
#include <cstdio>
#include <cstdlib>

#include "nlexcept.h" /* Use NLEXCEPT_FILTER macro */

/***************************************************************************
 * Always return 0xbaadf00d.
 **************************************************************************/
int foo( void* arg )
{
  return 0xbaadf00d;
}/* foo */

/***************************************************************************
 * Do some random stuff.
 **************************************************************************/
int bar( void* arg )
{
  int x = 0xdeadbeef;

  foo(arg);

  wchar_t* temp = (wchar_t*)malloc(4);
  swprintf(temp,32,L"x val = %d / %s\n",x,foo(&x));
  free(temp);
  temp = NULL;
  free(temp);
  return x;
}/* bar */

/**************************************************************************
 * My callback to do some stuff.
 *************************************************************************/
int my_callback( void* arg )
{
  __try
  {
    return bar(arg);
  }
  __except( NLEXCEPT_FILTER() )
  {
    /* empty */
  }
  return 0;
}/* my_callback */

/**************************************************************************
 * Simulate my_callback use.
 **************************************************************************/
int main(void)
{
  return my_callback(main);
}/* main */
