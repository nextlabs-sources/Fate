

#ifndef _NL_DEVCON_INSTALL_HPP_
#define _NL_DEVCON_INSTALL_HPP_

int cmdInstall(__in LPCWSTR wzInfPath, __in LPCWSTR wzHwId);
int cmdUninstall(__in LPCWSTR wzHwId);

#endif