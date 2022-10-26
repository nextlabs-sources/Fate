#ifndef TAMPER_PRIVATE_H
#define TAMPER_PRIVATE_H

#if defined (WIN32) || defined (_WIN64)
#include <Winsock2.h>
#pragma warning(push)
#pragma warning(disable : 6386)
#include <ws2tcpip.h>
#pragma warning(pop)
#endif
#if defined (Linux) || defined (Darwin)
//#include <iostream>
//#include <fstream>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif
#include "brain.h"
#include "nlthread.h"
#include "CEsdk.h"

using namespace std;

class TamperPolicy
{
private:
  CEHandle ceHandle;         //connection handle to policy controller
  struct in_addr  hostIPAddr; //Host ip address
  nlstring processName;       //the current process name including path
  nlthread_mutex_t connMutex; //The mutext to synchronize connection setting up
  bool bInitFailed;           //true, initialization failed; everything allowed
  bool bConnInProgress;       //Avoid recursive call of CECONN_Initalize

  //Set up the connection to policy controller
  bool SetupConnection();

public:
  //Constructor
  TamperPolicy(); 

  //Destructor
  ~TamperPolicy();

  //Do policy evaluation. If allow, returns true.
  bool evaluation(CEAction_t action, 
		  const nlchar *from, 
		  const nlchar *to=NULL);

};

#endif //TAMPER_PRIVATE_H
