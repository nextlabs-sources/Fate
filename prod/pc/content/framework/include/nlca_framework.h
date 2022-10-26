#ifndef __NLCA_FRAMEWORK_HPP__
#define __NLCA_FRAMEWORK_HPP__

#include "nlca_rpc.hpp"
#include "nlca_rpc_server.hpp"
#include "nlca_threadpool.hpp"

//void SetNLCAThreadPoolLog(CELog *lg);
//void SetContentAnalysisRPCLog(CELog *lg);

NLCA::ThreadPool *NLCA_ThreadPool_Initialize(int numThreads);
bool NLCA_ThreadPool_Shutdown(NLCA::ThreadPool *);
void NLCA_ThreadPool_AddTask(NLCA::ThreadPool *, NLCA::Client *c);
bool NLCA_ThreadPool_CancelTask(NLCA::ThreadPool *tp, NLCA::Client *c);
bool NLCA_RPCServer_Deserialization(const char *in, int inSize, 
				   NLCA::ContentAnalysisRPCTask **out);
NLCA::Client *NLCA_InitializeAClient();
void NLCA_FreeAClient(NLCA::Client *c);
#endif //__NLCA_framework_H__
