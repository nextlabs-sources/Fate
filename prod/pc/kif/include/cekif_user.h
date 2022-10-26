// Author : Jack Zhang
// Date   : 06/06/2007
// Note   : userspace KIF definition
// 

#include <queue>
#include <vector>
#include <map>
using namespace std;


#include "nlstrings.h"
#include "nlthread.h"


#ifndef KIF_USER_H
#define KIF_USER_H

//information needed to identify a kernel module/device
//   (can be extended to accommodate more types of identifier
typedef struct kif_identifier_tag{
  nlint        type;  
  nlint        socketid;    //e.g. netlink ID
  nlDescriptor handle;      //e.g. minifilter port  
  nlchar       *devpath;    //device path
  nlint        devpathlen;  
  nlint        tamperproof; //indicating whether this driver is tamper proof driver or not   1: yes  other: no
  nlchar       desc[32];    //description
} NL_KIF_IDENTIFIER;

//functions that all kinds of kif components need to implement
typedef struct kif_operations_tag{
  nlDescriptor (*open)(NL_KIF_IDENTIFIER &);
  nlint (*recv_data)(nlDescriptor, nlchar *,nlint);
  nlint (*send_data)(nlDescriptor, const nlchar*, nlint);
  nlint (*close)(nlDescriptor);
}NL_KIF_OPERATIONS;

typedef struct kif_connection_tag{
  nlchar            name[32];       //description
  NL_KIF_OPERATIONS kif_ops;  //supported operations
  nlDescriptor      handle;  //connection handle, fd in Linux
  nlthread_t        tid;  //id of kif thread for this connection
}NL_KIF_CONNECTION;

typedef struct kif_queue_item_tag{
  nlint                    index;  //identifier of the connection
  NL_KIF_POLICY_REQUEST    req;
}NL_KIF_QUEUE_ITEM,*PNL_KIF_QUEUE_ITEM;
  

//KIF exported functions
CEResult_t NL_KIF_Probe(const nlchar*);
CEResult_t NL_KIF_Register(NL_KIF_IDENTIFIER &kidentifier, NL_KIF_OPERATIONS &ops, nlint &index);
CEResult_t NL_KIF_Initialize();
CEResult_t NL_KIF_GetNextKernelRequests(nlint &index,NL_KIF_POLICY_REQUEST &request);
CEResult_t NL_KIF_SendKernelResponse(nlint index,const NL_KIF_POLICY_RESPONSE &response);
CEResult_t NL_KIF_Close(nlint index);
CEResult_t NL_KIF_RequestFree(NL_KIF_POLICY_REQUEST &request);
CEResult_t NL_KIF_Shutdown();

#endif  //KIF_USER_H
