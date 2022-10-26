#include <stdlib.h>
//#include "servstub.h"


using namespace std;

void ServiceStart(int argc, char** argv);

int main(int argc, char ** argv)
{

  //do not daemonize for testing
  //daemon(1,0);

  ServiceStart(argc, argv);
  exit (0);


  /*

  int    c;
  string s;
  string workdir;
  char * jvm_options [MAX_JVM_OPTIONS];
  char * app_options [MAX_APP_OPTIONS];
  int    jvm_options_cnt = 0;
  int    app_options_cnt = 0;
  bool   file_agent  = false;
  bool   daemon_mode = true;

  // parse the arguments


  if (!file_agent) {
    cerr << "Only file server agent is supported\n" << endl;
    print_usage(argv[0]);
    exit (1);
  }

  // Let's change the directory because the Java code is assuming that
  if (chdir (workdir.c_str()) != 0) {
    cerr << "Cannot cd to : " << workdir << endl;
    exit (1);
  }

  if (daemon_mode) {
    daemon(1, 0);
  }

  // Create the Stop Agent semaphore so that Java can tell us when to stop 
  //nlthread_mutex_init(&stop_mutex);
  //nlthread_cond_init(&stop_cond);
  
  // Creates the Java VM
  env = invokeJVM(jvm_options, jvm_options_cnt);

  if (env == NULL) {
    cerr << "Fail to invoke JVM." << endl;
    exit (1);
  }

  startControlMgr (app_options, app_options_cnt);

  //[011907 jzhang] refactoring for SDK

  //wait for the JVM to start and start a master thread to listen on socket
  //master(NULL);
  nlthread_t  tid;
  nlthread_create(&tid,&master,NULL);
  

  //Added by jzhang for tamperproof
  //startShield();
  //cout << "Shield applied." <<endl;

  //Wait for jvm to stop
  
  nlthread_mutex_lock(&stop_mutex);
  while(!jvmstopped)
    nlthread_cond_wait(&stop_cond,&stop_mutex);
  
  
  // The Java code in terms of stopping the agent is wrong.
  // It creates a big race-condition because it's giving out
  // the StopAgent event from the stopAgent method before 
  // sending the reply to the IPC-client. 
  // Basically once the event is set, the system can be
  // shutdown before the message even get a chance to
  // go out.

  // So, we will do a delay here, which is a poor
  // workaround on the problem.
  // It doesn't take time to cleanup actually
  
  cout << "Cleaning up, please wait..." << endl;
  sleep (5);

  cout << "end" << endl;
  exit (0);
  */
}
