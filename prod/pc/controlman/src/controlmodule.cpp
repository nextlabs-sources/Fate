//
// All sources, binaries and HTML pages (C) copyright 2006 by Blue Jungle Inc. 
// San Mateo CA, Ownership remains with Blue Jungle Inc, 
// All rights reserved worldwide. 
//
// Dominic Lam
// 4/26/2006
//
// First attempt to load the JVM on linux

// <-------------------------------------------------------------------------->

#include <iostream>
#include <string>
#include <jni.h>

#include <sys/types.h>
#include <sys/stat.h>

#include "brain.h"
#include "osServLinux.h"
#include "JavaConstants.h"
#include "ce_syslistener.h"

using namespace std;

static jobject  g_scmEventMgr = NULL;
static JavaVM * jvm = NULL;


#define MAX_JVM_OPTIONS 20      // Maxiumium number of options to JVM
#define MAX_APP_OPTIONS 20      // Maxiumium number of options to app (CM)

void startShield();

// ---------------------------------------------------------------------------
// Function to create a JVM for some Java class to run
// Arguments
//   - options     : options to the JVM
//   - options_cnt : number of options
// 
// Returns
//   - NULL : Fail to create the JVM
//   -      : JVM env pointer
// ---------------------------------------------------------------------------


JNIEnv * invokeJVM (char ** options, int options_cnt)
{
  JNIEnv       * jenv;
  JavaVMInitArgs vm_args;
  JavaVMOption   joptions[MAX_JVM_OPTIONS];
  jint           rc;
  int            i;

  memset (joptions, 0x0, sizeof(joptions));
  
  for (i = 0; i < options_cnt; i++) {
    joptions[i].optionString = options[i];
  }
  joptions[i].optionString    = "-verbose:jni";

  vm_args.version             = JNI_VERSION_1_2;
  vm_args.options             = joptions;
  vm_args.nOptions            = i;
  vm_args.ignoreUnrecognized  = JNI_FALSE;

  rc = JNI_CreateJavaVM (&jvm, (void **) &jenv, &vm_args);

  if (rc < 0) {
    cerr << "Cannot create Java VM." << endl;
    return NULL;
  }
  return jenv;
}

// ---------------------------------------------------------------------------
// Function to start the JVM for the Control Module
// Arguments
//   - args     : arguments to the Control Module
//   - args_cnt : number of argruments to the Control Module
// 
// Returns
//   - -1       : Fail to start the JVM Control Module
//   -  0       : Control Module started, but fail to initialized
//   -  1       : Control Module started and initialized successfully
// ---------------------------------------------------------------------------

int startControlMgr (JNIEnv *env, char ** args, int args_cnt) 
{
  jclass       agentMainClass;
  jclass       agentEventManClass;
  jmethodID    mainMethod;
  jmethodID    initMethod;
  jmethodID    getInstanceMethod;
  jobjectArray jargs;
  char         buf[1024]; 

  // Get all the necessary classes first
  agentMainClass     = env->FindClass (CM_MAIN_CLASS);
  agentEventManClass = env->FindClass (CM_EVENTMAN_CLASS);

  if (!agentMainClass || !agentEventManClass) {
    if (env->ExceptionOccurred()) {
      env->ExceptionDescribe();
    }
    cerr << "Failed to load the event class" << endl;
    return -1;
  }

  // Get all the methods from the classes 
  _snprintf_s (buf, 1024, _TRUNCATE, "()L%s;", CM_EVENTMAN_CLASS);

  mainMethod        = env->GetStaticMethodID (agentMainClass,     CM_MAIN_MAIN_M,          "([Ljava/lang/String;)V");
  initMethod        = env->GetStaticMethodID (agentMainClass,     CM_MAIN_ISINITIALIZED_M, "()Z");
  getInstanceMethod = env->GetStaticMethodID (agentEventManClass, CM_EVENTMAN_GETINSTANCE_M,   buf);
                                              
  if (!mainMethod || !initMethod || !getInstanceMethod) {
    cerr << "Failed to find the methods." << endl;
    return -1;
  }

  // Get all the necessary stuffs for calling the methods

  jargs = env->NewObjectArray (args_cnt, env->FindClass ("java/lang/String"), NULL);

  if (jargs == NULL) {
    cerr << "Cannot allocate the arguments to run main" << endl;
    return -1;
  }

  for (int i = 0; i < args_cnt; i++) {
    
    jstring jarg = env->NewStringUTF (args[i]);

    if (jarg == NULL) {
      cerr << "Failed to build the arg list" << endl;
      return -1;
    }
    env->SetObjectArrayElement (jargs, i, jarg);
  }

  // Get an instance for the Event Manager
  g_scmEventMgr = env->NewGlobalRef (env->CallStaticObjectMethod 
                                     (agentEventManClass, getInstanceMethod));

  if (g_scmEventMgr == NULL) {
    cerr << "Cannot get an Instance from SCM EventMgr" << endl;
  }

  // Calling entry point of the control manager
  env->CallStaticVoidMethod  (agentMainClass, mainMethod, jargs);
  
  // Check if it has been initialized successfully
  if (! env->CallStaticBooleanMethod (agentMainClass, initMethod)) {
    cerr << "Agent failed to initialized" << endl;
    return false;
  }

  cerr << "Agent has started and initialized successfully" << endl;
  return true;
}

int stopControlMgr (char ** app_opts, int app_opts_cnt, 
                    char ** jvm_opts, int jvm_opts_cnt) 
{

  for (int i = 0; i < app_opts_cnt; i++) {
    free (app_opts[i]);
  }

  for (int i = 0; i < jvm_opts_cnt; i++) {
    free (jvm_opts[i]);
  }

  return true;
}


// ---------------------------------------------------------------------------
// Usage print out of the control module
// ---------------------------------------------------------------------------

void print_usage (char * program)
{
  cout << "Usage: " << program << endl;
  cout << "   -h                  : Display this message"   << endl;
  cout << "   -d <name>=<value>   : JVM properties"         << endl;   
  cout << "   -x debug flags      : JVM properties"         << endl;
  cout << "   -a                  : Control Module args"    << endl; 
  cout << "   -w work directory   : Working directory"      << endl;
  cout << "   -f                  : File Server Agent mode" << endl;
  // Not printing this out, treat out as undocumented feature.
  // cout << "   -i                  : interactive mode, not as deamon" << endl;
}

// ---------------------------------------------------------------------------
// Entry point for the Control Module of the Agent
// ---------------------------------------------------------------------------

int am_I_running(const char* lckpath)
{
  char lckfile[256];
  _snprintf_s(lckfile,256, _TRUNCATE, "%s/%s",lckpath,BJ_LCK_FILE);
  FILE *fp =  fopen(lckfile,"r");
  if(fp)
    {
      fclose(fp);
      return 1;
    }
  return 0;
}

int main(int argc, char ** argv)
{
  JNIEnv * env = NULL;
  int    c;
  string s;
  string workdir;
  char * jvm_options [MAX_JVM_OPTIONS];
  char * app_options [MAX_APP_OPTIONS];
  int    jvm_options_cnt = 0;
  int    app_options_cnt = 0;
  bool   file_agent  = false;
  bool   daemon_mode = true;
  bjSemaphore   * stop_agent_sem = NULL;
  BJ_waitResult_t waitResult     = BJ_WAIT_FAILED;

  // parse the arguments

  while ((c = getopt (argc, argv, "ifhd:x:w:a:")) != -1) {

    switch (c) {
    case 'h':
      print_usage (argv[0]);
      break;

    case 'f':
      file_agent = true;
      break;

    case 'd':
      s = "-D" + string(optarg);
      jvm_options[jvm_options_cnt++] = strdup (s.c_str());
      break;

    case 'x':
      s = "-X" + string(optarg);
      jvm_options[jvm_options_cnt++] = strdup (s.c_str());
      break;

    case 'w':
      workdir = string (optarg);
      break;

    case 'a':
      s = string(optarg);
      app_options[app_options_cnt++] = strdup (s.c_str());
      break;

    case 'i':
      daemon_mode = false;
      break;
      
    default:
      cout << "Bad arguments.  Program aborted." << endl;
      exit (1);
    }
  }

  for (int i = 0; i < jvm_options_cnt; i++) {
    cout << "option " << i << " = " << jvm_options[i] << endl;
  }
  for (int i = 0; i < app_options_cnt; i++) {
    cout << "applic " << i << " = " << app_options[i] << endl;
  }
  cout << "work dir is " << workdir << endl;

  if(am_I_running(workdir.c_str()))
    {
      cerr << "Another instance is running\n" << endl;
      exit(0);  //return 0 to prevent failed message print in "service"
    }


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
  
  stop_agent_sem = new bjSemaphore();
  if (stop_agent_sem == NULL) {
    cerr << "Fatal error: Cannot create the synchronization." << endl;
    exit (1);
  }
  
  if (stop_agent_sem->create(STOP_AGENT_EVENT, 0) != BJ_OK) {
    cerr << "Fatal error: Cannot create the synchronization." << endl;
    exit (1);
  }
  
  // Creates the Java VM
  env = invokeJVM(jvm_options, jvm_options_cnt);

  if (env == NULL) {
    cerr << "Fail to invoke JVM." << endl;
    exit (1);
  }

  startControlMgr (env, app_options, app_options_cnt);

  //jzhang 031307  Adding failover feature
  char lckfile[256];
  _snprintf_s(lckfile,256, _TRUNCATE, "%s/%s",workdir.c_str(),BJ_LCK_FILE);
  printf("%s\n",lckfile);
  FILE *fp =  fopen(lckfile,"w+");
  if(fp)
    {
      fprintf(fp, "%d", getpid());
      fclose(fp);
    }
  

  //Added by jzhang for tamperproof
  startShield();
  cout << "Shield applied." <<endl;


  waitResult = stop_agent_sem->take ();

  if (waitResult == BJ_WAIT_SUCCESS) {
    cout << "Stop request agent from Java!" << endl;
    stopControlMgr (app_options, app_options_cnt, 
                    jvm_options, jvm_options_cnt);
  } else {
    cout << "Error from stop semaphore" << endl;
  }
  
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

  delete (stop_agent_sem);
  cout << "end" << endl;
  exit (0);
}

// ---------------------------------------------------------------------------
// Simple Utility function to pass an Event from agent/controlManager
//   to the Control Center via the Java - scmEventManager
//  - jvm     : Java Virtual Machine
//  - env     : JNI environment
//  - eventId : Event ID to be passed 
//  
// ---------------------------------------------------------------------------

static void passSCMEvent (JavaVM * jvm, JNIEnv *env, int eventId)
{
  jint      jeventId;
  jclass    agentEventManClass;
  jmethodID dispatchEventMethod;

  jeventId = (jint) eventId;
  
  jvm->AttachCurrentThread ((void **)&env, NULL);

  agentEventManClass  = env->GetObjectClass (g_scmEventMgr);

  if (agentEventManClass == NULL) {
    cerr << "Weird.  It shouldn't fail to get the eventMan class" << endl;
    return;
  }
  
  dispatchEventMethod = env->GetMethodID (agentEventManClass,
                                          CM_EVENTMAN_DISPATCH_M, "(I)V");

  if (dispatchEventMethod == NULL) {
    cerr << "Cannot retrieve the dispatch method." << endl;
    return;
  }

  env->CallVoidMethod (g_scmEventMgr, dispatchEventMethod, jeventId);

  jvm->DetachCurrentThread();

  return;
  
}

