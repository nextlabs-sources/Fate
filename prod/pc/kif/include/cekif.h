
/* All sources, binaries and HTML pages (C) copyright 2006 by Blue Jungle Inc.,
 * Redwood City CA,
 * Ownership remains with Blue Jungle Inc, All rights reserved worldwide.
 *
 * Note: This file defines the interfaces between the kernel and user
 *       It export the kernel defines / struct etc available to the user mode
 */

// Author : Jack Zhang
// Date   : 06/06/2007
// Note   : Common header file for KIF
// 

#ifndef KIF_H
#define KIF_H 

//for time being, delay the use of nlstrings.h
#include "nltypes.h"

#ifdef  NL_KIF_USER

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#define nlmalloc    malloc
#define nlfree      free
#define nlprint     printf

#elif defined(NL_KIF_LINUX_KERNEL)

#define nlmalloc(size)  kmalloc(size,GFP_ATOMIC)
#define nlfree(buf)     kfree(buf)
#define nlprint         printk

#endif


//define package type here
enum kif_pkgtype {
  NL_KIF_TYPE_REQUEST  = 0,
  NL_KIF_TYPE_RESPONSE  = 1,
  NL_KIF_TYPE_COMMAND_FILTER_PID = 2,
  NL_KIF_TYPE_COMMAND_FILTER_PATH = 3};

//fixed length header
typedef struct kif_transport_hdr_tag{
  nlint32   kt_version;   //version
  nlint32   kt_type;      //type
  nlint32  kt_payloadsize;   //payload size
  nlint32  kt_reserve;   //reserved
}NL_KIF_TRANSPORT_HDR,*PNL_KIF_TRANSPORT_HDR;

typedef struct kif_string_tag{
  nlint   size;    //buffer length, including the null-terminated character (in number of nlchar)
  nlchar*  content;   //platform dependent, null-terminated, defined in nlstrings.h (Linux single byte, Windows double-byte)
}NL_KIF_STRING, *PNL_KIF_STRING;

typedef struct tagCORE_SID
{
  NL_KIF_STRING aUserSID;
  nlint       ulUserSIDAttribute;
  NL_KIF_STRING aOwnerSID;
  NL_KIF_STRING aGroupSID;
  nlint       ulLinuxUid;
}CORE_SID,*PCORE_SID;

typedef struct tagFILE_INFO
{
  nlint  hFileHandle;
  nlint  ulHighCreationTime;
  nlint  ulLowCreationTime;
  nlint  ulHighLastAccessTime;
  nlint  ulLowLastAccessTime;
  nlint  ulHighLastWriteTime;
  nlint  ulLowLastWriteTime;
} FILE_INFO,*PFILE_INFO;

typedef struct kernel_policy_request_tag{
  nlint           index;   //UniqueIndex
  nlint           pid;   //process id
  NL_KIF_STRING   fromfile;   //from-file name
  NL_KIF_STRING   fromfileEQ;  //equivalent from-file name
  NL_KIF_STRING   tofile;     //to-file name
  NL_KIF_STRING   tofileEQ;     //to-file name equivalent name
  FILE_INFO       fromfileinfo;
  nlint           action;    //action code
  NL_KIF_STRING   hostname;   //host name
  nlint           ipaddr;  //IP address
  NL_KIF_STRING   appname;  //Application name
  nlint           performOB;  //true when obligations are mandated
  CORE_SID        coreSID;     //user SID information
  nlint           noiselevel;   //3-default   2-unknown    1-noise
}NL_KIF_POLICY_REQUEST,*PNL_KIF_POLICY_REQUEST;

typedef struct kernel_policy_response_tag{
  nlint         index;   //UniqueIndex
  nlint         allow;    
}NL_KIF_POLICY_RESPONSE,*PNL_KIF_POLICY_RESPONSE;


/***************Utility functions*******************/

static inline void destroy_KIF_STRING(NL_KIF_STRING *kstr)
{
  if(kstr->size)
    nlfree(kstr->content);
}

static inline void destroy_KIF_POLICY_REQUEST(PNL_KIF_POLICY_REQUEST pReq)
{
  destroy_KIF_STRING(&(pReq->fromfile));
  destroy_KIF_STRING(&(pReq->fromfileEQ));
  destroy_KIF_STRING(&(pReq->tofile));
  destroy_KIF_STRING(&(pReq->tofileEQ));
  destroy_KIF_STRING(&(pReq->hostname));
  destroy_KIF_STRING(&(pReq->appname));
}

//each NL_KIF_STRING must come from this constructor
//   @param:   size             should be the strlen result
static inline void construct_KIF_STRING(nlchar* buf, int size,NL_KIF_STRING *kstr)
{
  kstr->size = size + 1;  //including null
  kstr->content = (nlchar*) nlmalloc(kstr->size * sizeof(nlchar));
  memcpy(kstr->content,buf,kstr->size);
  //nlprint("building KIF_STRING length=%d, content='%s'\n",kstr->size,kstr->content);
}

//append the expanded buffer (size + content) to a buffer
//return the total number of bytes appended
//!!!! buf must have enough space 
static inline int kif_explode_KIF_STRING(nlchar **buf,const NL_KIF_STRING s)
{
  int totalsize = sizeof(s.size)+s.size*sizeof(nlchar);
  memcpy(*buf,&(s.size),sizeof(s.size));
  memcpy((reinterpret_cast<char*>(*buf))+sizeof(s.size),s.content,s.size*sizeof(nlchar));
  *buf = reinterpret_cast<nlchar*>((reinterpret_cast<char*>(*buf)) + totalsize);  //move forward
  return totalsize;
}
//for nlint
static inline int kif_explode_nlint(nlchar **buf,const nlint value)
{
  int totalsize = sizeof(nlint);
  memcpy(*buf,&value,totalsize);
  *buf = reinterpret_cast<nlchar*>((reinterpret_cast<char*>(*buf)) + totalsize);  //move forward
  return totalsize;
}
//for FILE_INFO
static inline int kif_explode_FILE_INFO(nlchar **buf,const FILE_INFO value)
{
  int totalsize = sizeof(FILE_INFO);
  memcpy(*buf,&value,totalsize);
  *buf = reinterpret_cast<nlchar*>((reinterpret_cast<char*>(*buf)) + totalsize);  //move forward
  return totalsize;
}
//for CORE_SID
static inline int kif_explode_CORE_SID(nlchar **buf,const CORE_SID value)
{
  int totalsize = kif_explode_KIF_STRING(buf,value.aUserSID)+
    kif_explode_nlint(buf,value.ulUserSIDAttribute)+
    kif_explode_KIF_STRING(buf,value.aOwnerSID)+
    kif_explode_KIF_STRING(buf,value.aGroupSID)+
    kif_explode_nlint(buf,value.ulLinuxUid);
  return totalsize;
}

//calculate the needed space to accomodate the request
static inline int sizeof_request(const NL_KIF_POLICY_REQUEST req){
  return sizeof(req.index)+
    sizeof(req.pid) +
    req.fromfile.size + sizeof(req.fromfile.size) +
    req.fromfileEQ.size + sizeof(req.fromfileEQ.size) +
    req.tofile.size + sizeof(req.tofile.size) +
    req.tofileEQ.size + sizeof(req.tofileEQ.size) +
    sizeof(FILE_INFO) +
    sizeof(req.action) +
    req.hostname.size + sizeof(req.hostname.size) +
    sizeof(req.ipaddr) +
    req.appname.size + sizeof(req.appname.size) +
    sizeof(req.performOB) +
    sizeof(CORE_SID) +
    sizeof(req.noiselevel);
}

//marshal the reqeust into a chunk of bytes (may need to be exported)
static inline int marshal_request(nlchar *buf,int size, const NL_KIF_POLICY_REQUEST req)
{
  int cnt = 0;
  if((cnt+=kif_explode_nlint(&buf,req.index))>size ||
     (cnt+=kif_explode_nlint(&buf,req.pid))>size ||
     (cnt+=kif_explode_KIF_STRING(&buf,req.fromfile))>size ||
     (cnt+=kif_explode_KIF_STRING(&buf,req.fromfileEQ))>size ||
     (cnt+=kif_explode_KIF_STRING(&buf,req.tofile))>size ||
     (cnt+=kif_explode_KIF_STRING(&buf,req.tofileEQ))>size ||
     (cnt+=kif_explode_FILE_INFO(&buf,req.fromfileinfo))>size ||
     (cnt+=kif_explode_nlint(&buf,req.action))>size ||
     (cnt+=kif_explode_KIF_STRING(&buf,req.hostname))>size ||
     (cnt+=kif_explode_nlint(&buf,req.ipaddr))>size ||
     (cnt+=kif_explode_KIF_STRING(&buf,req.appname))>size ||
     (cnt+=kif_explode_nlint(&buf,req.performOB))>size ||
     (cnt+=kif_explode_CORE_SID(&buf,req.coreSID))>size ||
     (cnt+=kif_explode_nlint(&buf,req.noiselevel))>size)   //buf is pointing at the end at this moment
    return -1;
  
  return  cnt;
}

////////////end of marshalling

////////////Unmarshalling begins here

//put the content of buf into values, adjust the buf pointer
//return the number of bytes processed
static inline int kif_collapse_KIF_STRING(nlchar **buf,NL_KIF_STRING *s)
{
  int totalsize = 0;
  memcpy(&(s->size),*buf,sizeof(s->size));
  totalsize = sizeof(s->size)+s->size*sizeof(nlchar);
  s->content = (nlchar*)nlmalloc(s->size * sizeof(nlchar));
  memcpy(s->content,reinterpret_cast<char*>(*buf)+sizeof(s->size),s->size*sizeof(nlchar));

  //nlprint("collapse_KIF_STRING got %s\n",s->content);

  *buf = reinterpret_cast<nlchar*>((reinterpret_cast<char*>(*buf)) + totalsize);  //move forward
  return totalsize;
}
//for nlint
static inline int kif_collapse_nlint(nlchar **buf,nlint *value)
{
  int totalsize = sizeof(nlint);
  memcpy(value,*buf,totalsize);
  *buf = reinterpret_cast<nlchar*>((reinterpret_cast<char*>(*buf)) + totalsize);  //move forward
  return totalsize;
}
//for FILE_INFO
static inline int kif_collapse_FILE_INFO(nlchar **buf,FILE_INFO *value)
{
  int totalsize = sizeof(FILE_INFO);
  memcpy(value,*buf,totalsize);
  *buf = reinterpret_cast<nlchar*>((reinterpret_cast<char*>(*buf)) + totalsize);  //move forward
  return totalsize;
}
//for CORE_SID
static inline int kif_collapse_CORE_SID(nlchar **buf,CORE_SID *value)
{
  int totalsize = kif_collapse_KIF_STRING(buf,&(value->aUserSID))+
    kif_collapse_nlint(buf,&(value->ulUserSIDAttribute))+
    kif_collapse_KIF_STRING(buf,&(value->aOwnerSID))+
    kif_collapse_KIF_STRING(buf,&(value->aGroupSID))+
    kif_collapse_nlint(buf,&(value->ulLinuxUid));
  return totalsize;
}

//marshal the reqeust into a chunk of bytes
static inline int unmarshal_request(nlchar *buf,int size, NL_KIF_POLICY_REQUEST *req)
{
  int cnt = 0;
  if((cnt+=kif_collapse_nlint(&buf,&req->index))>size ||
     (cnt+=kif_collapse_nlint(&buf,&req->pid))>size ||
     (cnt+=kif_collapse_KIF_STRING(&buf,&req->fromfile))>size ||
     (cnt+=kif_collapse_KIF_STRING(&buf,&req->fromfileEQ))>size ||
     (cnt+=kif_collapse_KIF_STRING(&buf,&req->tofile))>size ||
     (cnt+=kif_collapse_KIF_STRING(&buf,&req->tofileEQ))>size ||
     (cnt+=kif_collapse_FILE_INFO(&buf,&req->fromfileinfo))>size ||
     (cnt+=kif_collapse_nlint(&buf,&req->action))>size ||
     (cnt+=kif_collapse_KIF_STRING(&buf,&req->hostname))>size ||
     (cnt+=kif_collapse_nlint(&buf,&req->ipaddr))>size ||
     (cnt+=kif_collapse_KIF_STRING(&buf,&req->appname))>size ||
     (cnt+=kif_collapse_nlint(&buf,&req->performOB))>size ||
     (cnt+=kif_collapse_CORE_SID(&buf,&req->coreSID))>size ||
     (cnt+=kif_collapse_nlint(&buf,&req->noiselevel))>size)   //buf is pointing at the end at this moment
    return -1;
  
  return  cnt;
}
////////////end of unmarshalling  

/***********************End of utility functions*******************/

#endif /* KIF_H*/
