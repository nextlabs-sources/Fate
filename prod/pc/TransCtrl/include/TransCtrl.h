/*========================TransCtrl.h=======================================*
 *                                                                          *
 * All sources, binaries and HTML pages (C) copyright 2006 by Blue Jungle., *
 * Redwood City CA, Ownership remains with Blue Jungle Inc,                 * 
 * All rights reserved worldwide.                                           *
 *                                                                          * 
 * Author : Heidi Zhou                                                      *
 * Date   : 5/30/2007                                                       *
 * Note   : exported class and data structure of module TransCtrl.          *
 *==========================================================================*/
#ifndef __CE_TRANSCTRL_H
#define __CE_TRANSCTRL_H

//This is temperoal definition until KIF is ready
/*#include "nlstrings.h"

#define NL_KIF_STRING nlstring
#define nlint int
#define ku32 int
#define NL_KIF_CORE_SID int

using namespace std;

struct tagFILE_INFO
{
  ku32  hFileHandle;
  ku32  ulHighCreationTime;
  ku32  ulLowCreationTime;
  ku32  ulHighLastAccessTime;
  ku32  ulLowLastAccessTime;
  ku32  ulHighLastWriteTime;
  ku32  ulLowLastWriteTime;
};
typedef struct tagFILE_INFO NL_KIF_FILE_INFO;

typedef struct kernel_policy_request_tag
{
  nlint         index;   //UniqueIndex
  nlint         pid;   //process id

  NL_KIF_STRING    fromfile;   //from-file name
  NL_KIF_STRING    fromfileEQ;  //equivalent from-file name
  NL_KIF_STRING    tofile;     //to-file name
  NL_KIF_STRING    tofileEQ;     //to-file name equivalent name
  NL_KIF_FILE_INFO fromfileinfo;
  NL_KIF_STRING   action;    //action string
  NL_KIF_STRING   hostname;   //host name
  nlint         ipaddr;  //IP address
  NL_KIF_STRING   appname;  //Application name
  nlint         performOB;  //true when obligations are mandated
  NL_KIF_CORE_SID     coreSID;     //user SID information
  nlint         noiselevel;   //3-default   2-unknown    1-noise
} NL_KIF_POLICY_REQUEST,*NL_PKIF_POLICY_REQUEST;

typedef struct NL_KIF_QUEUE_ITEM
{
  nlint index;
  NL_KIF_POLICY_REQUEST req;
} NL_KIF_QUEUE_ITEM;
*/

struct NL_TRANSCTRL_QUEUE_ITEM
{
  enum SOURCE {KIF, SOCKET};

  SOURCE source;
  void *item;
};

/* =======================NL_TRANSCTRL_Serv_Initialize====================*
 * Initialize transport control module.                                   *
 *                                                                        *
 * Parameters:                                                            *
 * =======================================================================*/
CEResult_t NL_TRANSCTRL_Serv_Initialize();

/* =======================NL_TRANSCTRL_Serv_GetNextRequest================*
 * Get the next request from dataQ.                                       *
 *                                                                        *
 * Parameters:                                                            *
 * =======================================================================*/
CEResult_t NL_TRANSCTRL_Serv_GetNextRequest(NL_TRANSCTRL_QUEUE_ITEM **req);

/* =======================NL_TRANSCTRL_MemoryFree=========================*
 * Free the memory allocated in this module                               *
 *                                                                        *
 * Parameters:                                                            *
 * =======================================================================*/
void NL_TRANSCTRL_MemoryFree(NL_TRANSCTRL_QUEUE_ITEM *req);

/* =======================NL_TRANSCTRL_SHUTDOWN===========================*
 * Shutdown the two threads KifListenThread & SocketListenThread, of      *
 * TransCtrl.                                                             *
 *                                                                        *
 * Parameters:                                                            *
 * =======================================================================*/
void NL_TRANSCTRL_Shutdown();

#endif /* end of TRANSCTRL.h*/
