/*
 * Created on Apr 21, 2010
 *
 * All sources, binaries and HTML pages (C) copyright 2010 by NextLabs Inc.,
 * San Mateo CA, Ownership remains with NextLabs Inc, All rights reserved
 * worldwide.
 */
#include "brain.h"
#include "cesdk.h"
#include "cetype.h"
#include "marshal.h"
#include "PEPMan.h"

int main(int argc, char **argv)
{
  CEResult_t ret      = CE_RESULT_GENERAL_FAILED;
  CEHandle   handle   = NULL;
  CEString   appName  = NULL;
  CEString   password = NULL;

  appName = CEM_AllocateString(_T("test_cesec"));

  CEApplication app;
  CEUser user;
  app.appName = appName;
  app.appPath = NULL;
  user.userName = NULL;
  user.userID = NULL;

  ret = CECONN_Initialize(app, user, NULL, &handle, CE_INFINITE);

  if (ret != CE_RESULT_SUCCESS) {
      TRACE(0, _T("CECONN_Initialize failed, error %d\n"), ret);
      return 0;
  }

  // This is actually rather annoying.  We'd like to make a call to a function
  // that will deny if we are not trusted, but these are not easily accessible
  // from this code.  I guess all we can do is make sure this function succeeds

  password = CEM_AllocateString(_T("password"));

  ret = CESEC_MakeProcessTrusted(handle, password);

  if (ret != CE_RESULT_SUCCESS) {
      TRACE(0, _T("CESEC_MakeProcessTrusted failed, error %d\n"), ret);
      return 0;
  }

  CECONN_Close(handle, CE_INFINITE);
}
