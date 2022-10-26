// Author : Jack Zhang
// Date   : 06/06/2007
// Note   : Common logic for KIF
// 
#include "brain.h"
#include "CEsdk.h"

//for user mode
#define  NL_KIF_USER
#include "cekif.h"
#include "cekif_user.h"

#ifdef Linux
#include <dlfcn.h>
#include <dirent.h>
#endif

#if defined (WIN32) || defined (_WIN64)
#endif

#define DATA_BUFSIZE 4096

namespace{
  map<nlint,NL_KIF_CONNECTION> g_kconn_map;                   //current list of kernel module specific information
  nlint   gIndex = 0;
  nlint   tpIndex = -1;   //index of tamper proof driver
  nlthread_mutex_t   indexlock;

  //global queue
  queue<NL_KIF_QUEUE_ITEM> g_kif_queue;  //the queue holding incoming requests
  nlthread_mutex_t  g_kqueuelock;         //queue lock
  nlthread_sem_t   g_semEnqueued;         //semaphore for incoming request notification

  //Variables to stop TransCtrl
  nlthread_mutex_t  stopMutex; //mutex for notifying the stop of kif
  bool bStopped;    //stop flag 

  //thread that handles client connection and data transfer
  static void* kif_transport_thread(void* arg)
  {
    nlint index = *(static_cast<nlint *>(arg));
    delete arg;

    NL_KIF_CONNECTION kconn = g_kconn_map[index];
    nlchar *recvbuf = new nlchar[DATA_BUFSIZE];
    NL_KIF_QUEUE_ITEM      qitem;
    NL_KIF_POLICY_REQUEST  request;

    qitem.index = index;
    
    while(1)
      {
	nlint ret = kconn.kif_ops.recv_data(kconn.handle,recvbuf,DATA_BUFSIZE);
	//process received data (de-package)
	if(ret == 0)   continue;
	if(ret == -1)  break;

	
	PNL_KIF_TRANSPORT_HDR header = reinterpret_cast<PNL_KIF_TRANSPORT_HDR>(recvbuf);

	switch(header->kt_type)
	  {
	  case NL_KIF_TYPE_REQUEST:
	    unmarshal_request(reinterpret_cast<nlchar*>(reinterpret_cast<char*>(recvbuf)+sizeof(NL_KIF_TRANSPORT_HDR)),ret-sizeof(NL_KIF_TRANSPORT_HDR),&request);
	    //got a request, enqueue it
	    qitem.req = request;
	    nlthread_mutex_lock(&g_kqueuelock);
	    g_kif_queue.push(qitem);
	    nlthread_mutex_unlock(&g_kqueuelock);
	    nlthread_sem_post(&g_semEnqueued);
	    break;
	  }
      }

    delete[] recvbuf;
    nlthread_end();
    return NULL;
  }

}//namespace


//SDK functions start here

/**
 *  Probe all the components need to be loaded 
 *  
 *  @param  [in]  path      the path contains loadable KIF components
 *  @return                 the result status
 */
CEResult_t NL_KIF_Probe(const nlchar* path)
{
  //enumerate all dynamic libraries in given path
#ifdef Linux
  struct dirent *dirp;
  DIR           *dp;
  void          *handle;

  if((dp = opendir(path))==NULL)
    return CE_RESULT_GENERAL_FAILED;
  while((dirp = readdir(dp)) != NULL){
    printf("Probing %s ...\n",dirp->d_name);
    nlint len = strlen(dirp->d_name);
    if((strstr(dirp->d_name,"libkif_")==dirp->d_name) && 
       dirp->d_name[len-2] == 's' && dirp->d_name[len-1] == 'o')
      {
	//start to load
	char libpath[512];
	_snprintf_s(libpath,512, _TRUNCATE, "%s/%s",path,dirp->d_name);
	handle = dlopen(libpath,RTLD_NOW);
	if(!handle)
	  {
	    printf("loading %s failed(%s).\n",libpath,dlerror());
	  }
	else
	  {
	    printf("%s loaded.\n",libpath);
	  }
      }
  }
#endif

#if defined (WIN32) || defined (_WIN64)
  HANDLE searchHandle;
  WIN32_FIND_DATA findData;
  HINSTANCE hDLL;

  searchHandle = FindFirstFile(path, &findData);

  do {
    if((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
      //Not a directory
      size_t len=nlstrlen(findData.cFileName);
      if(nlstrstr(findData.cFileName, _T("libkif_")) &&
	 nlstrcmp(&(findData.cFileName[len-4]), _T(".dll"))==0) {
	//This is the KIF component. Start to load it
	nlchar libPath[512];
	nlsprintf(libPath, 512, _T("%s\\%s"),path,findData.cFileName);
	hDLL = LoadLibrary(libPath);
	if(hDLL==NULL) {
	  wprintf(L"loading %s failed.\n",libPath);
	} else {
	  wprintf(L"%s loaded.\n",libPath);
	}
      }
    }
  } while (FindNextFile(searchHandle, &findData));
#endif

  return CE_RESULT_SUCCESS;
}

/**
 *  Given identifier and operations, register the KIF component
 *  
 *  @param  [in]  kidentifier     the identification information of the kernel module
 *  @param  [in]  ops             the standard operations implemented to handle general kif calls
 *  @param  [out] index           the index used to identify this connection 
 *  @return                       the result status
 */
CEResult_t NL_KIF_Register(NL_KIF_IDENTIFIER &kidentifier, NL_KIF_OPERATIONS &ops, nlint &index)
{
  NL_KIF_CONNECTION kconn;

  nlsprintf(kconn.name, 23, kidentifier.desc,nlstrlen(kidentifier.desc));
  kconn.kif_ops = ops;
  kconn.handle = kconn.kif_ops.open(kidentifier);

#if defined (WIN32) || defined (_WIN64)
  if(kconn.handle==NULL)
#else
  if(kconn.handle==-1)
#endif
    return CE_RESULT_GENERAL_FAILED;

  nlprintf(L"registering KIF component \"%s\" , handle = %p\n",kconn.name,kconn.handle);

  //put into vector
  nlthread_mutex_lock(&indexlock);
  index = gIndex++;
  nlthread_mutex_unlock(&indexlock);
  g_kconn_map[index] = kconn;

  if(kidentifier.tamperproof==1)
    tpIndex = index;

  return CE_RESULT_SUCCESS;
}

/**
 *  Initialize a connection to the kernel module, start a loop to put incoming requests into the queue
 *  
 *  @return                 the result status
 */
CEResult_t NL_KIF_Initialize()
{
  nlthread_t tid;

  nlthread_mutex_init(&g_kqueuelock);
  //On Mac, we should call nlthread_sem_close to free the memory of 
  //g_semEqueued. However, to simplify semphore handling, sem_close
  //is not called since g_semEqueued will last through the whole
  //life of PDPMan.
  nlthread_sem_init(&g_semEnqueued,0);
  nlthread_mutex_init(&indexlock);
  nlthread_mutex_init(&stopMutex);
  bStopped=false;

  //initiate the transport thread for each KIF component
  map<nlint,NL_KIF_CONNECTION>::iterator  kconn_itor;
  nlint index;

  for(kconn_itor=g_kconn_map.begin();kconn_itor!=g_kconn_map.end();
      kconn_itor++) {
    index = (*kconn_itor).first;
    nlint *arg = new nlint;
    *arg = index;
    nlthread_create(&tid,(nlthread_func)&kif_transport_thread,(void *)arg);
    kconn_itor->second.tid=tid;
  }

  return CE_RESULT_SUCCESS;
}

/**
 *  Retrieve at most a certain number of requests from the internal queue
 *  
 *  @param  [in]  handle    connection identifier
 *  @param  [in]  peek      expected number of requests to retrieve
 *  @param  [out] outlist   list of retrieved kernel requests
 *  @return                 the result status
 */

CEResult_t NL_KIF_GetNextKernelRequests(nlint &index,NL_KIF_POLICY_REQUEST &request)
{
  NL_KIF_QUEUE_ITEM    qitem;
  
  //If the stop flag is on, skip fetching requests and return
  nlthread_mutex_lock(&stopMutex);
  if(bStopped) {
    nlthread_mutex_unlock(&stopMutex);
    TRACE(0, _T("KIF has stopped\n"));
    return CE_RESULT_SUCCESS;
  }
  nlthread_mutex_unlock(&stopMutex);

  //FIXME: the checking queue if empty and going to wait is not atomic. 
  //FIXME: This can cause wait forever. We should use conditional wait.
  //Fetch the request
  nlthread_mutex_lock(&g_kqueuelock);
  if(g_kif_queue.empty()) {
    nlthread_mutex_unlock(&g_kqueuelock);
    //wait for incoming request
    while(!nlthread_sem_wait(&g_semEnqueued));
    //Wake up from waiting
  } else 
    nlthread_mutex_unlock(&g_kqueuelock);

  //If the stop flag is on, ignore the request and return
  nlthread_mutex_lock(&stopMutex);
  if(bStopped) {
    nlthread_mutex_unlock(&stopMutex);
    TRACE(0, _T("KIF stopped\n"));
    return CE_RESULT_SUCCESS;
  }
  nlthread_mutex_unlock(&stopMutex);
  
  //Fetch the request
  nlthread_mutex_lock(&g_kqueuelock);
  if(!g_kif_queue.empty())
  {
      qitem = g_kif_queue.front();

      index = qitem.index;
      request = qitem.req;

      g_kif_queue.pop();
  }
  nlthread_mutex_unlock(&g_kqueuelock);

  return CE_RESULT_SUCCESS;
}


/**
 *  Send response to the kernel module
 *  
 *  @param  [in]  handle    connection identifier
 *  @param  [in]  response  policy decision
 *  @return                 the result status
 */

CEResult_t NL_KIF_SendKernelResponse(nlint index,const NL_KIF_POLICY_RESPONSE &response)
{
  NL_KIF_CONNECTION kconn = g_kconn_map[index];
  NL_KIF_TRANSPORT_HDR hdr;
  nlint  sz = sizeof(NL_KIF_TRANSPORT_HDR) + sizeof(NL_KIF_POLICY_RESPONSE);
  nlchar *outbuf = (nlchar*)malloc(sz);
  CEResult_t  res;
  nlint  ret;

  hdr.kt_type = NL_KIF_TYPE_RESPONSE;
  hdr.kt_payloadsize = sizeof(NL_KIF_POLICY_RESPONSE);

  memcpy(outbuf,&hdr,sizeof(NL_KIF_TRANSPORT_HDR));
  memcpy(reinterpret_cast<nlchar*>(reinterpret_cast<char*>(outbuf)+sizeof(NL_KIF_TRANSPORT_HDR)),&response,
	 sizeof(NL_KIF_POLICY_RESPONSE));

  // 7/30/2010 Nao & Alan
  // Use of kconn, which comes from g_kconn_map, seems dangerous.
  // If NL_KIF_Shutdown() clears g_kconn_map right before this line, doesn't send_data() crash?
  // I won't change this code for now, as it does not seem to cause a crash.
  // But we should revisit this if we see a crash.

  ret = kconn.kif_ops.send_data(kconn.handle,outbuf,sz);
  if( -1 == ret)
    {
      res = CE_RESULT_GENERAL_FAILED;
      goto clean_and_return;
    }

  res = CE_RESULT_SUCCESS;
    
 clean_and_return:
  free(outbuf);
  return res;
}


/**
 *  Shutdown all kernel connections
 *  
 *  @param 
 *  @return                 the result status
 */

CEResult_t NL_KIF_Shutdown()
{
  map<nlint,NL_KIF_CONNECTION>::iterator  kconn_itor;
  NL_KIF_CONNECTION kconn;


  //Close each opened kif component
  for(kconn_itor=g_kconn_map.begin();kconn_itor!=g_kconn_map.end();kconn_itor++)
    {
      kconn = (*kconn_itor).second;
      if(kconn.kif_ops.close(kconn.handle))
	return CE_RESULT_GENERAL_FAILED;
    }

  //Wait for each kif transport thread to exit
  for(kconn_itor=g_kconn_map.begin();kconn_itor!=g_kconn_map.end();
      kconn_itor++) {
    kconn = (*kconn_itor).second;
    nlthread_join(kconn.tid);    
  }

  g_kconn_map.clear();

  //Flag kif to stop and wake up kif from waiting request
  nlthread_mutex_lock(&stopMutex);
  bStopped=true;
  nlthread_mutex_unlock(&stopMutex);
  nlthread_sem_post(&g_semEnqueued);
  
  return CE_RESULT_SUCCESS;
}

/**
 *  Shutdown one kernel connection
 *  
 *  @param  [in]  handle    connection identifier
 *  @return                 the result status
 */

CEResult_t NL_KIF_Close(nlint index)
{
  if(g_kconn_map.find(index) == g_kconn_map.end()) {
    return CE_RESULT_SUCCESS;
  }
    
  NL_KIF_CONNECTION kconn = g_kconn_map[index];
  if(kconn.kif_ops.close(kconn.handle))
    return CE_RESULT_GENERAL_FAILED;
  g_kconn_map.erase(index);   //clear this element from map

  return CE_RESULT_SUCCESS;
}



/**
 *  deallocate the memory allocated inside NL_KIF_POLICY_REQUEST
 *  
 *  @param  [in]  request   NL_KIF_POLICY_REQUEST structure needs to be freed
 *  @return                 the result status
 */
 
CEResult_t NL_KIF_RequestFree(NL_KIF_POLICY_REQUEST &request)
{
  destroy_KIF_POLICY_REQUEST(&request);
  return CE_RESULT_SUCCESS;
}

/**
 *  send pid to tamper proof driver 
 *  
 *  @param  [in]  pid   Process ID
 *  @return             result status
 */
 
CEResult_t NL_KIF_ProtectPID(nlint pid)
{
  //TODO: assembly a package and call send_data to tamper proof 
  return CE_RESULT_SUCCESS;
}


/**
 *  send path to tamper proof driver 
 *  
 *  @param  [in]  path  buffer containing the target path
 *  @param  [in]  len   buffer len
 *  @return             result status
 */
 
CEResult_t NL_KIF_ProtectPath(const nlchar* path, int len)
{
  //TODO: assembly a package and call send_data to tamper proof 
  return CE_RESULT_SUCCESS;
}
