/*==========================eval_test.cpp===================================*
 *                                                                          *
 * All sources, binaries and HTML pages (C) copyright 2006 by Blue Nextlabs,*
 * Redwood City CA, Ownership remains with Blue Jungle Inc,                 * 
 * All rights reserved worldwide.                                           *
 *                                                                          * 
 * Author : Heidi Zhou                                                      *
 * Date   : 1/22/2007                                                       *
 * Note   : Test SDK CEEVALUATE_XXX APIs.                                   *
 *==========================================================================*/
#include <time.h>
#if defined (WIN32) || defined (_WIN64)
#include <Winsock2.h>
#pragma warning(push)
#pragma warning(disable : 6386)
#include <ws2tcpip.h>
#pragma warning(pop)
#endif
#include <iostream>
#if defined (Linux) || defined (Darwin)
#include <fstream>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif
#include <errno.h>
#include "brain.h"
#include "cetype.h"
#include "CEsdk.h"
#include "transport.h"
#include "nlthread.h"
#include "marshal.h"

namespace {
  enum {MAX_ATTRIBUTES=1, MAX_REQUEST_PER_THREAD=1,MAX_SUB_THREADS=1, 
        FLAG_DONE=1, FLAG_RUNNING=2, FLAG_JOINED=4};

  struct child {
    int flag;
    nlthread_t tid;
    int index;
  } childs[MAX_SUB_THREADS];

  int numDone;
  nlthread_mutex_t numDone_mutex;
  nlthread_cond_t numDone_cond;

  CEString pdpHost=NULL;
  CEString sourceString=CEM_AllocateString(_T("dummySource"));
  
  bool bServerReady=false;

  void TestCheckResource(CEHandle handle, const CEint32 ipNumber,
                         const CEint32 numRecipients, CEString *recipients,
                         CEAttributes *dummyAttrs)
  {
    CEEnforcement_t enf;
    CEString operation=CEM_AllocateString(_T("action_test_checkresource"));
    CEResource *source=CEM_CreateResource(_T("source_name_test_checkresource"),
                                          _T("fso"));
    CEResource *target=CEM_CreateResource(_T("target_name_test_checkresource"),
                                          _T("portal"));
    CEResult_t res;

    if(source == NULL || target == NULL || operation == NULL) {
      TRACE(0, _T("Failed to create input parameters\n"));
      return;
    }

    res=CEEVALUATE_CheckResources(handle,
                                  operation,
                                  source,
                                  dummyAttrs, //from attrs
                                  target,
                                  dummyAttrs, //target attrs
                                  NULL, //user
                                  dummyAttrs, //user attrs
                                  NULL, //application
                                  dummyAttrs, //application attrs
                                  recipients, //recipients
                                  numRecipients, //number of recipients
                                  ipNumber,
                                  CETrue, //performObligation
                                  CE_NOISE_LEVEL_MIN, //noise level
                                  &enf,
                                  15000 //timeout in milliseconds
                                  );
			     
    if(res == CE_RESULT_SUCCESS) {
      wprintf(L"CEEVALUATE_CheckResources result: %s\n", 
              (enf.result==CEDeny)?L"deny":L"allow");
      if(enf.obligation) {
        for(int i=0; i<enf.obligation->count; i++) {
          wprintf(L"obligation %d: key=%s; value=%s\n",
                  i,
                  (enf.obligation->attrs[i].key)->buf,
                  (enf.obligation->attrs[i].value)->buf);
        }
      }
      //Free enforcement
      CEEVALUATE_FreeEnforcement(enf);
    } else {
      wprintf(L"CEEVALUATE_CheckResources failed result: %d\n",res);
    }

    CEM_FreeString(operation);
    CEM_FreeResource(source);
    CEM_FreeResource(target);
  }

  void TestCheckResourcesEx(CEHandle handle) {
    CEApplication app = {0};
    app.appName = CEM_AllocateString(L"P");
    app.appPath = CEM_AllocateString(L"c:/foo.exe");

    CEUser bob = {0};
    bob.userName = CEM_AllocateString(L"bob");

    CEUser kathy = {0};
    kathy.userName = CEM_AllocateString(L"kathy");

    CEString *recipients = new CEString[2];
    recipients[0] = CEM_AllocateString(L"manoj@foo.com");
    recipients[1] = CEM_AllocateString(L"yan@bar.com");

    CERequest *requests = new CERequest[2];

    CEAttributes source1attrs;
    source1attrs.count = 2;
    source1attrs.attrs = new CEAttribute[source1attrs.count];
    source1attrs.attrs[0].key = CEM_AllocateString(L"key1");
    source1attrs.attrs[0].value = CEM_AllocateString(L"value1");
    source1attrs.attrs[1].key = CEM_AllocateString(L"key2");
    source1attrs.attrs[1].value = CEM_AllocateString(L"value2");

    requests[0].operation = CEM_AllocateString(L"DELETE");
    requests[0].source = CEM_CreateResource(L"c:/file1.txt", L"fso");
    requests[0].sourceAttributes = &source1attrs;
    requests[0].target = NULL;
    requests[0].targetAttributes = NULL;
    requests[0].user = &bob;
    requests[0].userAttributes = NULL;
    requests[0].app = NULL;
    requests[0].appAttributes = NULL;
    requests[0].recipients = recipients;
    requests[0].numRecipients = 2;
    requests[0].additionalAttributes = NULL;
    requests[0].numAdditionalAttributes = 0;
    requests[0].performObligation = CETrue;
    requests[0].noiseLevel = CE_NOISE_LEVEL_USER_ACTION;

    requests[1].operation = CEM_AllocateString(L"RENAME");
    requests[1].source = CEM_CreateResource(L"c:/file2.java", L"fso");
    requests[1].sourceAttributes = &source1attrs;
    requests[1].target = CEM_CreateResource(L"c:/file3.java", L"fso");
    requests[1].targetAttributes = NULL;
    requests[1].user = &kathy;
    requests[1].userAttributes = NULL;
    requests[1].app = NULL;
    requests[1].appAttributes = NULL;
    requests[1].recipients = NULL;
    requests[1].numRecipients = 0;
    requests[1].additionalAttributes = NULL;
    requests[1].numAdditionalAttributes = 0;
    requests[1].performObligation = CETrue;
    requests[1].noiseLevel = CE_NOISE_LEVEL_USER_ACTION;

    CEEnforcement_t enf[2];

    std::cout << "About to call check resources" << std::endl;

    CEResult_t res = CEEVALUATE_CheckResourcesEx(handle,
                                                 requests, 2,
                                                 NULL, CEFalse,
                                                 0,
                                                 enf,
                                                 10000);

    std::cout << "Return value was " << res << std::endl;

    if (res == CE_RESULT_SUCCESS)
    {
      for (int i = 0; i < 2; ++i)
      {
        std::cout << "Response " << (i+1) << " was " << enf[i].result << std::endl;
      
        std::cout << "Obligations" << std::endl;
      
        for (int j = 0; j < enf[i].obligation->count; ++j)
        {
          std::wcout << enf[i].obligation->attrs[j].key->buf << L"=" << enf[i].obligation->attrs[j].value->buf << std::endl;
        }

        CEEVALUATE_FreeEnforcement(enf[i]);
      }

    }
  }


  bool GetPdpHost(char *p)
  {
#ifdef WIN32
    nlchar tmp[1024];
    ::MultiByteToWideChar(CP_ACP, 0, p, -1, tmp, _countof(tmp));
    pdpHost=CEM_AllocateString(tmp);
#else
    pdpHost=CEM_AllocateString(p);
#endif
    return true;
  }

  bool GetSourceString(char *f)
  {
#ifdef WIN32
    HANDLE hFile; 
    nlchar tmp[1024];
    DWORD nBytesRead;

    ::MultiByteToWideChar(CP_ACP, 0, f, -1, tmp, _countof(tmp));
    hFile = CreateFile(tmp,    // file to open
                       GENERIC_READ,          // open for reading
                       FILE_SHARE_READ,       // share for reading
                       NULL,                  // default security
                       OPEN_EXISTING,         // existing file only
                       FILE_ATTRIBUTE_NORMAL, // normal file
                       NULL);                 // no attr. template
 
    if (hFile == INVALID_HANDLE_VALUE) { 
      TRACE(0, _T("File %s doesn't exit\n"), tmp);
      return false;
    }  
    // Attempt a synchronous read operation.
    memset(tmp, 0, 1024*sizeof(nlchar));
    bool bResult = ReadFile(hFile, 
                            tmp, 
                            1023, 
                            &nBytesRead, 
                            NULL) ; 
    // Check for the end of file. 
    if (bResult &&  nBytesRead == 0) { 
      // This is the end of the file. 
      printf("File %s is empty\n", f);    
      return false;
    } 
    sourceString=CEM_AllocateString(tmp);
    return true;
#else
    ifstream ifs(f);
    if(!ifs.good()) {
      TRACE(0, _T("File %s doesn't exit\n"), f);
      return false;
    }
    nlchar line[1024];
    memset(line, 0, 1024*sizeof(nlchar));
    while(ifs.getline(line, 1024)) {
      sourceString=CEM_AllocateString(line);
      break;
    }
    if(sourceString == NULL) {
      printf("File %s is empty\n", f);    
      return false;
    }
    return true;
#endif
  }

  bool GetHostIPAddr(struct in_addr &hostIPAddr)
  {
    char hostName[1024];
    struct addrinfo hints, *res;
    int err;

    if(gethostname(hostName, 1024) != 0) {
      TRACE(0, _T("Failed to get host name: error=%d\n"), errno);
      return false;
    }

    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_INET;

    if ((err = getaddrinfo(hostName, NULL, &hints, &res)) != 0) {
      TRACE(0, _T("Failed to get host ip: error=%d\n"), err);
      return false;
    }

    hostIPAddr = ((struct sockaddr_in *)(res->ai_addr))->sin_addr;

    //printf("ip address : %s %lu\n", inet_ntoa(hostIPAddr), hostIPAddr.s_addr);

    freeaddrinfo(res);

    return true;  
  }
}

extern "C" void *EVALTestThread(void *arg)
{
  struct child *cptr=(struct child *)arg;
  CEString appName=CEM_AllocateString(_T("ceevaltest"));
  CEString binaryPath=CEM_AllocateString(_T("./ceevaltest.exe"));
  CEString userName=CEM_AllocateString(_T("heidi"));
  CEString dummyString=CEM_AllocateString(_T("dummy_string"));
  CEHandle handle;
  CEUser user;
  CEApplication app;

  user.userName=NULL;
  user.userID=NULL;
  app.appPath=binaryPath;
  app.appName=appName;
  //app.appPath=NULL;
  //app.appName=NULL;

  CEString *recipients= new CEString[2];
  recipients[0]=CEM_AllocateString(_T("Hzhou@nextlabs.com"));
  recipients[1]=CEM_AllocateString(_T("heidi.zhou@Nextlabs.com"));

  CEString type = CEM_AllocateString(_T("eval_test"));
  CEResult_t res=CECONN_Initialize(app, user, pdpHost, &handle, 10000);

  if(res == CE_RESULT_SUCCESS) {
    CEEnforcement_t enf;
    CEString key=CEM_AllocateString(_T("modified_date"));
    CEString val=CEM_AllocateString(_T("1234567"));
    CEAttribute attr[MAX_ATTRIBUTES];
    CEAttributes dummyAttrs;  
    for(int i=0; i<MAX_ATTRIBUTES; i++) {
      attr[i].key = key;
      attr[i].value = val;
    }
    dummyAttrs.count=MAX_ATTRIBUTES;
    dummyAttrs.attrs=attr;

    struct in_addr  hostIPAddr;
    GetHostIPAddr(hostIPAddr);
    //TRACE(0, _T("Call CEEVALUATE_Check...%s\n\n"), sourceString->buf);

    user.userID=CEM_AllocateString(handle->userID);
    
    TestCheckResource(handle, (CEint32)(ntohl(hostIPAddr.s_addr)), 
		      2, recipients, &dummyAttrs);
		      

    TestCheckResourcesEx(handle);

    for(int j=0; j<MAX_REQUEST_PER_THREAD; j++) {
      res=CEEVALUATE_CheckPortal(handle, 
				 CE_ACTION_ATTACH,
				 sourceString,
				 &dummyAttrs, //from attrs
				 NULL, //to attrs
				 NULL, //to attrs
				 (CEint32)(ntohl(hostIPAddr.s_addr)),//ip
				 &user,
				 CETrue, //performObligation 
				 CE_NOISE_LEVEL_MIN, //noise level
				 &enf,
				 15000); //timeout in milliseconds
      
      if(res == CE_RESULT_SUCCESS) {
	TRACE(5, _T("CheckPortal result: %s\n"), 
	      (enf.result==CEDeny)?_T("deny"):_T("allow"));
	if(enf.obligation) {
	  for(int i=0; i<enf.obligation->count; i++) {
	    TRACE(0, 
		  _T("obligation %d: key=%s; value=%s\n"),
		  i,
		  (enf.obligation->attrs[i].key)->buf,
		  (enf.obligation->attrs[i].value)->buf);
	  }
	}
	//Free enforcement
	CEEVALUATE_FreeEnforcement(enf);
      } else
	TRACE(0, _T("CECONN_CheckPortal failed: errorno=%d\n"), res);
	
      //Test CheckFile
      res=CEEVALUATE_CheckFile(handle, 
			       CE_ACTION_WRITE,
			       sourceString,
			       &dummyAttrs, //from attrs
			       dummyString,//to attrs
			       NULL, //to attrs
			       (CEint32)((hostIPAddr.s_addr)),//ip address
			       NULL,
			       NULL,
			       CETrue, //performObligation 
			       CE_NOISE_LEVEL_MIN, //noise level
			       &enf,
			       15000); //timeout in milliseconds
      
      if(res == CE_RESULT_SUCCESS) {
	TRACE(0, _T("CheckFile result: %s\n"), 
	      (enf.result==CEDeny)?_T("deny"):_T("allow"));
	if(enf.obligation) {
	  for(int i=0; i<enf.obligation->count; i++) {
	    TRACE(0, 
		  _T("obligation %d: key=%s; value=%s\n"),
		  i,
		  (enf.obligation->attrs[i].key)->buf,
		  (enf.obligation->attrs[i].value)->buf);
	  }
	}
	//Free enforcement
	CEEVALUATE_FreeEnforcement(enf);
      } else
	TRACE(0, _T("CECONN_CheckFile failed: errorno=%d\n"), res);

      //Test CEEVALUATE_CheckMessageAttachment
      res=CEEVALUATE_CheckMessageAttachment(handle, 
					    CE_ACTION_IM_FILE,
					    sourceString,
					    &dummyAttrs, //from attrs
					    2,
					    recipients, //&userName,
                                            (CEint32)((hostIPAddr.s_addr)),//ip address
					    NULL, //user
					    &dummyAttrs, //user attributes
					    NULL, //application
					    &dummyAttrs, //application attributes
					    CETrue, //performObligation 
					    CE_NOISE_LEVEL_MIN, //noise level
					    &enf,
					    15000); //timeout in milliseconds
      
      if(res == CE_RESULT_SUCCESS) {
	TRACE(0, _T("CEEVALUATE_CheckMessageAttachment result: %s\n"), 
	      (enf.result==CEDeny)?_T("deny"):_T("allow"));
	if(enf.obligation) {
	  for(int i=0; i<enf.obligation->count; i++) {
	    TRACE(0, 
		  _T("obligation %d: key=%s; value=%s\n"),
		  i,
		  (enf.obligation->attrs[i].key)->buf,
		  (enf.obligation->attrs[i].value)->buf);
	  }
	}
	//Free enforcement
	CEEVALUATE_FreeEnforcement(enf);
      } else
	TRACE(0, _T("CEEVALUATE_CheckMessageAttachment failed: errorno=%d\n"), 
	      res);
    }

    TRACE(4, _T("Call CECONN_Close\n"));
    CECONN_Close(handle, 10000); 
    CEM_FreeString(key);
    CEM_FreeString(val);  
    CEM_FreeString(recipients[0]);
    CEM_FreeString(recipients[1]);
    delete [] recipients;

    if(user.userID)
      CEM_FreeString(user.userID);

  } else
    TRACE(1, _T("CECONN_Initalization failed: errorno=%d\n"), res);
    
  CEM_FreeString(appName);
  CEM_FreeString(binaryPath);
  CEM_FreeString(userName);
  CEM_FreeString(dummyString);
  CEM_FreeString(type);
  nlthread_mutex_lock(&numDone_mutex);
  cptr->flag=FLAG_DONE;
  numDone++;  
  nlthread_cond_signal(&numDone_cond);
  nlthread_mutex_unlock(&numDone_mutex);
  nlthread_end();
  return NULL;
}


int main(int argc, char **argv)
{
  int result;
  nlthread_t tid;
  int numLeftChilds;
  nlthread_timeout timeout;
  bool btimedout;
  
  nlthread_mutex_init(&numDone_mutex);
  nlthread_cond_init(&numDone_cond);

  double start_time=NL_GetCurrentTimeInMillisec();

  for(int i=0; i<MAX_SUB_THREADS; i++) {
    result=nlthread_create(&tid, (nlthread_func)(&EVALTestThread), &childs[i]);
    if(!result) {
      TRACE(0, _T("Can't create no.%d thread\n"), i);
      exit(1);
    }
    childs[i].tid=tid;
    childs[i].index=i;
  }

  numLeftChilds=MAX_SUB_THREADS;

  while(numLeftChilds > 0) {
    TRACE(4, _T("Waiting for any one out of %d child threads done.\n"), 
	  numLeftChilds);
    for(int i=0; i<MAX_SUB_THREADS; i++) {
      if(childs[i].flag & FLAG_DONE)
	continue;
      if(childs[i].flag & FLAG_JOINED)
	continue;
    }
    nlthread_maketimeout(&timeout, 60, 0);
    nlthread_mutex_lock(&numDone_mutex);
    if(numDone == 0) 
      nlthread_cond_timedwait(&numDone_cond, &numDone_mutex, &timeout, 
			      &btimedout);
  
    for(int i=0; i<MAX_SUB_THREADS; i++) {
      if(childs[i].flag & FLAG_DONE) {
	nlthread_join(childs[i].tid);
      
	childs[i].flag=FLAG_JOINED;
	numDone--;
	numLeftChilds--;
	TRACE(4, _T("No.%d child thread %lu done\n"), i, childs[i].tid);
      } 
    }
    nlthread_mutex_unlock(&numDone_mutex);
  }
  nlthread_mutex_destroy(&numDone_mutex);
  nlthread_cond_destroy(&numDone_cond);
  double t2=NL_GetCurrentTimeInMillisec();
  double total_exec_time=t2-start_time;
  TRACE(0, _T("Execution time %f seconds\n "), 
	total_exec_time/1000.0);

  if(pdpHost) 
    CEM_FreeString(pdpHost);
  if(sourceString)
    CEM_FreeString(sourceString);    
    
  return 0;
}
