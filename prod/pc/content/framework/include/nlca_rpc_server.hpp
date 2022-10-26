
#ifndef __NLCA_RPC_SERVER_HPP__
#define __NLCA_RPC_SERVER_HPP__

#include <winsock2.h>
#include <windows.h>
#include <sstream>

#pragma warning(push)
#pragma warning(disable: 4512)
#pragma warning(disable: 6326 6334)
#  include <boost/regex.hpp>
#  include <boost/archive/text_oarchive.hpp>
#  include <boost/archive/text_iarchive.hpp>
#pragma warning(pop)

#include "nl_content_analysis.hpp"
#include "nlca_serialization.hpp"
#include "brain.h"
#include "nlca_threadpool.hpp"
#include "nlca_rpc.hpp"

namespace NLCA {
/* ContentAnalysisRPC
 *
 * \brief Content analysis RPC task data structure
 *
 */
class ContentAnalysisRPCTask : public TASK
{
private:
  NLCA::ContentAnalysisRPC *_rpc;

  bool HandleCommand(Client *client);
  bool SendResponse(SOCKET &s);    
  ContentCracker *CreateContentCracker(Client *client);
  void DeleteContentCracker(Client *client);
  bool ImpersonateCARPCCaller();
public:
  ContentAnalysisRPCTask():_rpc(NULL){}; 
  ~ContentAnalysisRPCTask(){if(_rpc) delete _rpc;};

  //access member function
  void SetRPC(NLCA::ContentAnalysisRPC *r) {_rpc=r;}
  NLCA::ContentAnalysisRPC *GetRPC(){return _rpc;}

  bool Run(void * pParam);
  bool Cancel(void * pParam);
  TASK *CloneTask() {
    if(_rpc==NULL) {
      ContentAnalysisRPCTask *t= new ContentAnalysisRPCTask();
      return t;
    }

    ContentAnalysisRPCTask *t= new ContentAnalysisRPCTask();
    if(_rpc->type != ContentAnalysisRPC::DoContentAnalysis) {
      ContentAnalysisRPC *c=new (std::nothrow) ContentAnalysisRPC(_rpc->type);
      c->pid=_rpc->pid;
      t->_rpc=c;
      return t;
    }
    
    ContentAnalysisRPC *c=new (std::nothrow) ContentAnalysisRPC(_rpc->_ca, 
						 _rpc->pid, 
						 _rpc->type);
    t->_rpc=c;
    return t;      
  }

  void DeleteTaskEvent(LPVOID event) {
    if(event)
      delete (ContentCracker*) event;
  }
}; //ContentAnalysisRPCTask
}

#endif /* __NLCA_RPC_SEVER_HPP__ */
