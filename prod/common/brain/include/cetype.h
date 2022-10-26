#ifndef __CE_TYPE_H
#define __CE_TYPE_H

#include <string>
#include <stddef.h>
#include "nlstrings.h"
#include "nlthread.h"
#include "nltypes.h"

/*CEString*/
struct _CEString {
  nlchar *buf;
  size_t length;

  _CEString () { buf=NULL;}
  ~_CEString () { if(buf) delete [] buf;}
};
/*CEString end*/

#define NON_EMPTY_CESTRING(s) ((s) != NULL  && (s)->buf != NULL && (s)->length!=0)
#define EMPTY_CESTRING(s) (!NON_EMPTY_CESTRING(s))

/*CEHandle*/
struct _CEHandle {
  nlchar *type;    // Plug-in (enforcer) type
  nlchar *appName; //The name of application
  nlchar *binaryPath; //The path to the application
  nlchar *userName; //Name to identify a user in the application
  nlchar * userID; //Unique ID to identify a user in the application
  int hostIPAddress; //host ip address

  nlsocket clientSfd; //Socket descriptor on PEP side
  nlsocket serverSfd; //Socket descriptor on PDP side

  /* PEP specific members */
  nlthread_t tHandle; //thread handle
  unsigned long tID;  //thread ID, on Linux this is same as tHandle
  unsigned long long sessionID;  // Session ID: the PEP handle on server side (64 bits)
  nlchar *fingerprint;
  /* PEP specific members end */
  
  /* PDP specific members */
  /* PDP specific members end */

  _CEHandle () {appName=binaryPath=userName=userID=fingerprint=NULL;}

};
/*CEHandle end*/

/*CE software version*/
enum {CESDK_VERSION_MAJOR=2, 
      CESDK_VERSION_MINOR=0, 
      CESDK_VERSION_PATCH=0,
      CESDK_VERSION=(((CESDK_VERSION_MAJOR) << 24) + ((CESDK_VERSION_MINOR) << 8) + (CESDK_VERSION_PATCH))};
/*CE software version end*/

#endif //End of #ifndef __CE_TYPE_H
