/*============================nlca_rpc.cpp==================================*
 *                                                                          *
 * All sources, binaries and HTML pages (C) copyright 2008 by NextLabs,     *
 * San Mateo CA, Ownership remains with NextLabs Inc,                       * 
 * All rights reserved worldwide.                                           *
 *                                                                          * 
 * Author : Heidi Zhou Nolan                                                *
 * Date   : 10/28/2008                                                      *
 * Note   : NLCA RPC implementation.                                        *
 *==========================================================================*/

#include <windows.h>
#include <sddl.h>
#include <aclapi.h>

#include "nlca_rpc_server.hpp"
#include "nl_content_cracker_strings.hpp"
#include "nl_content_cracker_ifilter.hpp"
#include "celog.h"
using namespace NLCA;

namespace {
CELog *calog=NULL;
}

bool ContentAnalysisRPCTask::HandleCommand(Client *client)
{
  bool result = false; /* result of this method */
  ContentCracker* cracker = NULL;
  std::wstring file;
  std::list<NLCA::SerializableExpression>::iterator it;
  std::list<NLCA::SerializableExpression>::iterator eit;
  NLCA::ExpressionList::const_iterator rxit;
  NLCA::ExpressionList::const_iterator erxit;
  NLCA::Expressions rx;
  bool matched = false;

  //calog->Log(CELOG_DEBUG, _T("===Enter Handle Command==\n"));  

  //Try to impersonate CA RPC caller
  if(!ImpersonateCARPCCaller()) {
    //Cracker has to be created by CA rpc caller
  }

  //Get the content cracker
  EnterCriticalSection(&client->_cs);
  //Note: only helding mutex: client's
  cracker=CreateContentCracker(client);
  if(cracker == NULL) {
    LeaveCriticalSection(&client->_cs);
    //Revert to default process owner (i.e. SYSTEM)
    RevertToSelf();
    return false;
  }
  LeaveCriticalSection(&client->_cs);  

  //Convert std::list<NLCA::SerializableExpression>
  //to std::list<NLCA::Expression *>
  NLCA::Expression *exp = NULL;
  std::list<NLCA::SerializableExpression> &sExps=_rpc->_ca.GetExpressions();
  it=sExps.begin();
  eit=sExps.end();
  for(; it!=eit; it++) {
    exp=NLCA::ConstructExpression((*it).GetType());
    if(exp) {
      (*it).CreateExpressionObject((exp));
      rx.Add(exp);
    }
  }

  //Search File
  //DWORD start_time = GetTickCount(); /* start time */

  result = NLCA::SearchFile(*cracker,rx);  // search file
  //DWORD runtime = GetTickCount() - start_time;

  //delete the content cracker
  EnterCriticalSection(&client->_cs);
  //Note: only helding mutex: client's
  DeleteContentCracker(client);
  LeaveCriticalSection(&client->_cs);  

  //Revert to default process owner (i.e. SYSTEM)
  RevertToSelf();

  //std::cout << "Runtime " << runtime << " ms\n";

  //Reconstruct serializable expression list with updates 
  //from NLCA::AnalyzeFile
  const NLCA::ExpressionList &expressions=rx.GetExpressions();
  rxit=expressions.begin();
  erxit=expressions.end();
  for(; rxit != erxit; ++rxit ) {
    exp=(*rxit);
    it=sExps.begin();
    eit=sExps.end();
    for(; it!=eit; ++it)
    {
      if(exp->GetType() == (*it).GetType()) {
	const std::wstring &exp1=exp->RegexString();
	const std::wstring &exp2=(*it).RegexString();
	if(exp1 == exp2) {
	  if(exp->RequiredMatchCount() == (*it).RequiredMatchCount()) {
	    //Find the expression; assign the result
	    (*it).SetMatch(exp->MatchCount());
	    matched = true;
	    break;
	  }//RequiredMatchCount ==
	}//RegexString ==	      
      }//ExpType ==
    }
  }    

  _rpc->_ca.SetMatchResult(matched);

  //cracker is not deleted at here
  //To synchronize with cancelling task, cracker is deleted when the task
  //is removed from threadpool 
  return true;
}/* ContentAnalysisRPC::HandleCommand */

bool ContentAnalysisRPCTask::SendResponse(SOCKET &s)
{
  /*********************************************************************
   * Serialize response
   ********************************************************************/
  std::string serialized;
  unsigned long bytes = 0;
  NLCAService::Serialization((*_rpc), serialized, bytes);

  if(bytes == 0)
    return false; 

  //Sending RPC response
  int bytes_sent, nbytes;
  nbytes = htonl(bytes); // byte order for network
  bytes_sent=send(s,(char*)&nbytes, sizeof(nbytes),0);//Send payload size
  if( bytes_sent != sizeof(nbytes) ) {
#if 0
    calog->Log(CELOG_ERR,
	      _T("Failed to send response payload size %d %d err=%d\n"),
	      bytes_sent,sizeof(nbytes), WSAGetLastError());
#endif
  } else {
    bytes_sent=send(s,(char*)serialized.c_str(),bytes,0);// Send payload
    if( static_cast<unsigned long>(bytes_sent) != bytes ) {
#if 0
      calog->Log(CELOG_ERR,_T("Failed to send response payload err=%d\n"), 
	    WSAGetLastError());
#endif
    }
  }
  return true;
}

bool ContentAnalysisRPCTask::Run(void * pParam)
{
  Client *c=reinterpret_cast<Client *>(pParam);
  //calog->Log(CELOG_DEBUG, _T("===RPC REQUEST==\n"));
  //display(*calog);
  HandleCommand(c);
  //calog->Log(CELOG_DEBUG, _T("===RPC RESPONSE==\n"));
  //display(*calog);

  //Sending response
  EnterCriticalSection(&c->_cs);
  if(c->bAbort==false)
    SendResponse(c->s);
  LeaveCriticalSection(&c->_cs);
  return true;
}

void ContentAnalysisRPCTask::DeleteContentCracker(Client *client)
{
  if(client->event) {
    ///Cracker is assign as event of threadpool client's
    DeleteTaskEvent(client->event); 
    client->event=NULL;
  }
}

ContentCracker *ContentAnalysisRPCTask::CreateContentCracker(Client *client)
{
  //Note: helding client's mutex
  if(client->bAbort) {
    //Before cracker is created, cancel happens. Abort.
    return NULL;
  }

  //Cracker will be stored under client not rpc because cracker won't
  //be sent over socket. Cracker will be assign to client's event since
  //content analysis can only be cancelled throught it. 
  if(client->event==NULL) {
    ContentCracker* cracker = NULL;
    Content &content=_rpc->_ca.GetContent();
    std::wstring file;

    content.GetDetails(file);
    if( content.GetType() != NLCA::Content::FILE || file.empty() == true ) {
      return NULL;
    }

    /***********************************************************************
     * Construct a cracker instance for this file.  Attempt IFilter first
     * then raw mode.
     **********************************************************************/
    try {
      cracker = new NLContentCracker_IFilter(file.c_str());
    } catch(...) {
      /* temp */
    }

    if( cracker == NULL ){
      try {
	cracker = new NLContentCracker_Strings(file.c_str(), true);
      } catch(...) {
	cracker = NULL;
      }
    }
    if( cracker == NULL ) {
      return NULL;
    }
    client->event=(LPVOID)cracker;
  }

  return reinterpret_cast<ContentCracker *>(client->event);
}

//Impersonate CA RPC Caller
bool ContentAnalysisRPCTask::ImpersonateCARPCCaller()
{
  /******************************************************************************
   * Impersonate user before cracker instance is created.  This order is required
   * due to file permissions such as EFS where the SYSTEM user may not have
   * permissions to open an NTFS encrypted file that was encrypted by another
   * user.
   *****************************************************************************/
  //fprintf(stdout, "Impersonate SID: PID %d\n", _rpc->pid);

  BOOL rv;
  HANDLE hToken = NULL;
  HANDLE hTokenDup = NULL;
  HANDLE ph = NULL;
  bool bResult=false;

  ph = OpenProcess(PROCESS_QUERY_INFORMATION,FALSE,_rpc->pid);
  if( ph == NULL )
  {
    goto ImpersonateProcessUser_complete;
  }

  rv = OpenProcessToken(ph,TOKEN_READ|TOKEN_DUPLICATE,&hToken);
  CloseHandle(ph);
  if( rv != TRUE )
  {
    goto ImpersonateProcessUser_complete;
  }

  rv = DuplicateTokenEx(hToken,
		 TOKEN_IMPERSONATE|TOKEN_READ|TOKEN_ASSIGN_PRIMARY|TOKEN_DUPLICATE,
			NULL,SecurityImpersonation,TokenPrimary,&hTokenDup);
  CloseHandle(hToken);
  if( rv != TRUE ) {
    goto ImpersonateProcessUser_complete;
  }

  // If user cannot be impersonated don't fail.  Try to scan the file anyway.
  bResult=true;
  rv = ImpersonateLoggedOnUser(hTokenDup);
  CloseHandle(hTokenDup);
  if( rv != TRUE ) {
  }

 ImpersonateProcessUser_complete:

  return bResult;
}//ContentAnalysisRPCTask::ImpersonateCARPCCaller

bool ContentAnalysisRPCTask::Cancel(void * pParam)
{
  Client *c=reinterpret_cast<Client *>(pParam);

  EnterCriticalSection(&c->_cs);
  c->bAbort=true;
  //Note: helding two mutex (in-order): 1. threadpool's 2. client's
  ContentCracker *cracker=reinterpret_cast<ContentCracker *>(c->event);
  if(cracker != NULL) {
    if(cracker->Cancel()==false)
    {
      //calog->Log(CELOG_ERR, _T("Cancel (socket=0x%x) failed\n"), c->s);
    }
      
  } 
  c->s=NULL;
  LeaveCriticalSection(&c->_cs);
  return true;
}

void SetContentAnalysisRPCLog(CELog *lg)
{
  calog=lg;
}

bool NLCA_RPCServer_Deserialization(const char *in, int inSize, 
				   NLCA::ContentAnalysisRPCTask **out)
{
  *out=new ContentAnalysisRPCTask;
  ContentAnalysisRPC *r=new ContentAnalysisRPC;
  (*out)->SetRPC(r);
  return NLCAService::Deserialization(in, inSize, *r);
}
