// Author : Jack Zhang
// Date   : 06/06/2007
// Note   : unit test program for KIF / KIF use sample
// 
#include "brain.h"
#include "CEsdk.h"

#define  NL_KIF_USER
#include "cekif.h"
#include "cekif_user.h"

void dump_KIF_Request(NL_KIF_POLICY_REQUEST &req)
{
  printf("----> Incoming KIF request (index = %d)\n",req.index);
  printf("      ====  pid      = %d\n",req.pid);
  printf("      ====  fromfile = %s\n",req.fromfile.content);
  printf("      ====  tofile   = %s\n",req.tofile.content);
  printf("      ====  action   = %d\n",req.action);
  printf("      ====  appname  = %s\n",req.appname.content);
}

int main()
{
  CEResult_t  result;
  NL_KIF_POLICY_REQUEST  req;
  NL_KIF_POLICY_RESPONSE  response;
  //NL_KIF_POLICY_RESPONSE response;
  nlint       index;

  result = NL_KIF_Probe(_T("."));

  if(result != CE_RESULT_SUCCESS)
    goto failed;

  NL_KIF_Initialize();

  while((result = NL_KIF_GetNextKernelRequests(index, req)) == CE_RESULT_SUCCESS)
    {
      dump_KIF_Request(req);
      
      response.index = req.index;
      response.allow = 2;
      NL_KIF_SendKernelResponse(index,response);
      
      NL_KIF_RequestFree(req);
    }

  return 0;

 failed:
  printf("failed with error %d\n", result);
  return -1;
}
