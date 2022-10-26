#ifndef __NLCA_THREADPOOL_HPP__
#define __NLCA_THREADPOOL_HPP__

#include <windows.h>
#include <process.h>
#include <winsock2.h>
#include <vector>

#include <boost/utility.hpp>

namespace NLCA {
#define MAX_SEMA_COUNT 256

class ThreadPool;
class ContentAnalysisRPCTask;

class TASK {
public:
  virtual ~TASK(){};  
  TASK(){};
  virtual bool Run(void * pParam)=0;
  virtual bool Cancel(void *pParam)=0;
  virtual TASK *CloneTask()=0;

  //Only the drived class know the type of its event in order to
  //call the corresponding destructor
  virtual void DeleteTaskEvent(LPVOID event)=0;
};

/** Client
 *
 *  \brief NLCA client record.  
 */
class Client : public boost::noncopyable
{
private:
  TASK   *task;      /* client task   */
  SOCKET s;          /* client socket */
  LPVOID event;      /* event to client */
  bool   bAbort;     /* flag to abort if true */
  CRITICAL_SECTION _cs; /*client mutex*/
  
  friend class ThreadPool;
  friend class ContentAnalysisRPCTask;
public:
  Client():task(NULL), event(NULL), bAbort(false) {
    InitializeCriticalSection(&_cs);
  }

  ~Client() {
    DeleteCriticalSection(&_cs);
    if(task) delete task;
    bAbort=false;
  }

  //access member functions
  TASK *GetTask() {return task;}
  void SetTask(TASK *t) {task=t;}
  SOCKET GetSocket() {return s;}
  void SetSocket(SOCKET s_in) {s=s_in;}
  LPVOID GetEvent() {return event;}
  void SetEvent(LPVOID e) {event=e;}
  bool IsAbort() {return bAbort;}
  void SetAbort(bool a) {bAbort=a;}
  CRITICAL_SECTION *GetLock() {return &_cs;}
  
  //This is not an equal operator, it only checks if the client
  //are from the same socket
  bool IsSameClient(Client &c) {
    if(s == c.s)
      return true;
    return false;
  }
};/* Client */

class NLCAThread : public boost::noncopyable
{
private:
  HANDLE _t;
  Client *_client; //client running in this thread
  ThreadPool *_myThreadPool;

public:
  //friend static unsigned int __stdcall ThreadPool::WorkerProc(void *pParam);
  friend class ThreadPool;
  
  NLCAThread():_t(NULL), _client(NULL), _myThreadPool(NULL){}
  ~NLCAThread() {if(_t) CloseHandle(_t);}

  inline void SetThread(HANDLE t){_t=t;}
  inline void SetMyThreadPool(ThreadPool *p){_myThreadPool=p;}  
};

class ThreadPool : public boost::noncopyable
{
private:
  std::vector<NLCAThread *> _pool;
  std::vector<Client *> _tasks;
  CRITICAL_SECTION _cs;
  unsigned int _poolSize;
  HANDLE _taskCountSema;
  HANDLE _abortSignal;

  Client *FetchTask(NLCAThread *pWorker); //worker thread fetch task  
public:
  friend class NLCAThread;

  ThreadPool();
  ~ThreadPool();

  bool Initialize(int numThreads);
  bool Shutdown();
  void AddTask(Client *c);
  bool CancelTask(Client *c);

  static unsigned int __stdcall WorkerProc(void *pParam); 
};
}

#endif /* __NLCA_THREADPOOL_HPP__ */
