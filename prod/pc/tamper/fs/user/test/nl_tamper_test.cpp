/*============================nl_tamper_test.cpp============================*
 *                                                                          *
 * All sources, binaries and HTML pages (C) copyright 2006 by NextLabs,     *
 * San Mateo CA, Ownership remains with NextLabs Inc,                       * 
 * All rights reserved worldwide.                                           *
 *                                                                          * 
 * Author : Heidi Zhou                                                      *
 * Date   : 9/28/2008                                                       *
 * Note   : Test nl_tamper driver and plugin                                *
 *==========================================================================*/
#define WINVER _WIN32_WINNT_WINXP
//#ifndef _WIN32_WINNT
#define _WIN32_WINNT _WIN32_WINNT_WINXP
//#endif
#define NTDDI_VERSION NTDDI_WINXPSP2

#include <Windows.h>
#include <list>
#include <vector>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <winsock2.h>
#include <cassert>
#include <fltuser.h>
#include <tchar.h>
#include <WinIoCtl.h>
#include <Sddl.h>
#include "nlthread.h"
#include "brain.h"
#include "nl_tamper.h"
#include "nl_tamper_lib.h"

//#include <strsafe.h>

namespace {
int max_file=4*MAX_PATH; //4*260
enum {FLAG_DONE=1, 
      FLAG_RUNNING=2, 
      FLAG_JOINED=4, 
      MAX_SUB_THREADS=30,
      MAX_ACCESS=2*MAX_PATH};

struct child {
  int flag;
  nlthread_t tid;
  int index;
} childs[MAX_SUB_THREADS];

int numDone;
nlthread_mutex_t numDone_mutex;
nlthread_cond_t numDone_cond;

using namespace std;

std::vector<wstring> fle;

void GetAllFilesUnderDir(const WCHAR *dirName, const WCHAR *searchableDir)
{
  //WCHAR dirName[]=L"C:\\";
  //WCHAR searchableDir[]=L"C:\\*";
  WIN32_FIND_DATA ffd;
  //LARGE_INTEGER filesize;
  WCHAR szDir[MAX_PATH];
  HANDLE hFind = INVALID_HANDLE_VALUE;
  DWORD dwError=0;

  // Find the first file in the directory.
  hFind = FindFirstFile(searchableDir, &ffd);
  if (INVALID_HANDLE_VALUE == hFind) {
    int e=GetLastError();
    wprintf(L"FindFirstFile failed %s: %d\n", searchableDir, e);
    return;
  } 
   
  wstring fullPath;
  // retrieve the directory 
  do {
    if((ffd.cFileName[0] >= L'a' && ffd.cFileName[0] <= L'z') ||
       (ffd.cFileName[0] >= L'A' && ffd.cFileName[0] <= L'Z')) {
      if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
        //not a directory
        fullPath=dirName;
	fullPath+=L"\\";
        fullPath+=ffd.cFileName;
        fle.push_back(fullPath);
      } else {
        if(fle.size() < max_file) {
	  wstring sDir;
	  fullPath=dirName;
	  fullPath+=L"\\";
          fullPath+=ffd.cFileName;
	  sDir=fullPath;
	  sDir+=L"\\*";
	  //wprintf(L"retrieve the directory %s\n", fullPath.c_str());
	  GetAllFilesUnderDir(fullPath.c_str(), sDir.c_str());
	}
      }
    }
    if(fle.size() >= max_file)
      break;
  } while (FindNextFile(hFind, &ffd) != 0);
 
   dwError = GetLastError();
   if (dwError != ERROR_NO_MORE_FILES) 
     wprintf(L"FindNextFile failed: %d\n", dwError);

   FindClose(hFind);
   return;
}

  /*void FetchTamperproofPolicy()
{
  HRESULT hr;
  HANDLE port;
  std::vector<NLFiltPolicy> policy;

  hr = FilterConnectCommunicationPort(NLTAMPER_PORT_NAME,0,NULL,0,NULL,&port);
  if( IS_ERROR(hr) ) {
	wprintf(L"nl_tamper_test: FilterConnectCommunicationPort failed: 0x%x\n", hr);
    return;
  }

  NLTamper_GetAllPolicy(policy);

  if(port != INVALID_HANDLE_VALUE ) {
    CloseHandle(context->port);
  }
  }*/
}

extern "C" void *FileAccessTest(void *arg)
{
  struct child *cptr=(struct child *)arg;
  double r;
  int index;
  FILE* fp;

  srand(cptr->index*1000);
  for(int i=0; i<MAX_ACCESS; i++) {
    r = ((double)rand() / ((double)(RAND_MAX)+(double)(1)) );
    index =(int)(r * max_file);
    fp = _wfopen(fle[index].c_str(),L"r");
    if( fp == NULL ) {
      wprintf(L"open file[%d] failed: %s.\n", index, fle[index].c_str());
      continue;
    }
    //wprintf(L"open file[%d]: %s.\n", index, fle[index].c_str());
    fclose(fp);
  }
  nlthread_mutex_lock(&numDone_mutex);
  cptr->flag=FLAG_DONE;
  numDone++;  
  nlthread_cond_signal(&numDone_cond);
  nlthread_mutex_unlock(&numDone_mutex);
  nlthread_end();

  return NULL;
}

int main()
{
  int result;
  nlthread_t tid;
  int numLeftChilds;
  nlthread_timeout timeout;
  bool btimedout;

  //FetchTamperproofPolicy();

  GetAllFilesUnderDir(L"C:\\Program Files", L"C:\\Program Files\\*");
  //GetAllFilesUnderDir(L"C:", L"C:\\*");
  
  for(int i=0; i<fle.size(); i++)
    //wprintf(L"%d file[%d]: %s\n", max_file, i, fle[i].c_str());

  nlthread_mutex_init(&numDone_mutex);
  nlthread_cond_init(&numDone_cond);

  double start_time=NL_GetCurrentTimeInMillisec();

  for(int i=0; i<MAX_SUB_THREADS; i++) {
    result=nlthread_create(&tid, (nlthread_func)(&FileAccessTest), &childs[i]);
    if(!result) {
      wprintf(L"Can't create no.%d thread\n", i);
      exit(1);
    }
    childs[i].tid=tid;
    childs[i].index=i;
  }

  numLeftChilds=MAX_SUB_THREADS;

  while(numLeftChilds > 0) {
    wprintf(L"Waiting for any one out of %d child threads done.\n", 
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
	wprintf(L"No.%d child thread %lu done\n", i, childs[i].tid);
      } 
    }
    nlthread_mutex_unlock(&numDone_mutex);
  }
  nlthread_mutex_destroy(&numDone_mutex);
  nlthread_cond_destroy(&numDone_cond);
  double t2=NL_GetCurrentTimeInMillisec();
  double total_exec_time=t2-start_time;
  wprintf(L"Execution time %f seconds\n ", total_exec_time/1000.0);

  return 0;
}
