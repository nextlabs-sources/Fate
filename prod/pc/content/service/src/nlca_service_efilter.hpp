
#ifndef __NLCA_SERVICE_EFILTER_HPP_
#define __NLCA_SERVICE_EFILTER_HPP_

#include <windows.h>
#include <dbghelp.h>

LONG WINAPI efilter( LPEXCEPTION_POINTERS exception_pointers );

#endif /* __NLCA_SERVICE_EFILTER_HPP_ */
