/******************************************************************************
 *
 * NLCA Service Client
 *
 * The purpose of this program is to test the NLCAService interface which permits
 * request for content analysis from a client (such as the Policy Controller) to
 * the NLCA service.
 *
 ******************************************************************************/

#include <windows.h>
#include <tchar.h>
#include <strsafe.h>
#include "nlca_client.h"

#define MAX_THREADS 1
#define BUF_SIZE 255

namespace {
using namespace std;
std::vector<wstring> fle;
int max_file=1*MAX_PATH; //1*260
CRITICAL_SECTION displayMutex; /*client mutex*/

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
       (ffd.cFileName[0] >= L'A' && ffd.cFileName[0] <= L'Z') ||
       (ffd.cFileName[0] >= L'0' && ffd.cFileName[0] <= L'9')) {
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

DWORD WINAPI MyThreadFunction( LPVOID lpParam ) 
{ 
  const wchar_t* fNameStr=(const wchar_t *)lpParam;

  std::list<NLCA::Expression *> expression_list;
  NLCA::ExpressionCCN exp;
  exp.SetExpression(L"(\\d{16})|(\\d{4}.\\d{4}.\\d{4}.\\d{4})");  // 16 digit
  //exp.SetExpression(L"confidential");
  exp.SetRequiredMatchCount(1);
  expression_list.push_back(&exp); 


  if(NLCAService_SearchFile(fNameStr,GetCurrentProcessId(),
			    expression_list, 5000)) {
    EnterCriticalSection(&displayMutex);
    printf("==Analyze Result (%d)==\n", GetCurrentThreadId());
    printf("Match\n");
    wprintf_s(L"File: %s\n", fNameStr);
  } else {
    EnterCriticalSection(&displayMutex);
    printf("==Analyze Result (%d)==\n", GetCurrentThreadId());
    printf("Not match\n");
    wprintf_s(L"File: %s\n", fNameStr);
  }
  std::list<NLCA::Expression *>::iterator it=expression_list.begin();
  std::list<NLCA::Expression *>::iterator eit=expression_list.end();
  for(; it!=eit; it++)
    (*(*it)).display();
  LeaveCriticalSection(&displayMutex);

  return 0; 
}
}

int wmain( int argc , wchar_t** argv )
{
  if(!(argc==3 || argc==2 || argc==4)) {
    printf("nlca_client [<dir|file> <dir|file-name>] [stop_nlca_service] \n");
    printf("            dir-name: e.g. c:\\temp\\test-folder\n");
    return 1;
  }


  if(argc == 2 && wcscmp(argv[1], L"stop_nlca_service")==0) {
    printf("Send stop\n");
    NLCAService_Stop();
    return 0;
  }

  InitializeCriticalSection(&displayMutex);

  DWORD   dwThreadIdArray[MAX_THREADS];
  HANDLE  hThreadArray[MAX_THREADS]; 
  
  if(wcscmp(argv[1], L"dir")==0) {
    wstring sDir;
    sDir=argv[2];
    sDir+=L"\\*";
    GetAllFilesUnderDir(argv[2], sDir.c_str());
    max_file=fle.size();
    printf("max_file %d\n", max_file);
  } else {
    fle.push_back(argv[2]);
    max_file=1;
  }
    
  double r;
  int index;
  for(int i=0; i<MAX_THREADS; i++) {
    r = ((double)rand() / ((double)(RAND_MAX)+(double)(1)) );
    index =(int)(r * max_file);    
    hThreadArray[i] = CreateThread( 
				   NULL,  // default security attributes
				   0,    // use default stack size  
				   MyThreadFunction,    // thread function name
				   (LPVOID)fle[index].c_str(), //argument 
				   0,       // use default creation flags 
				   &dwThreadIdArray[i]); //thread identifier 


        // Check the return value for success.
        // If CreateThread fails, terminate execution. 
        // This will automatically clean up threads and memory. 
    if (hThreadArray[i] == NULL) {
      printf("Can create thread: %d\n", GetLastError());
      break;
    }
  }

  // Wait until all threads have terminated.
  WaitForMultipleObjects(MAX_THREADS, hThreadArray, TRUE, INFINITE);

  // Close all thread handles and free memory allocations.
  for(int i=0; i<MAX_THREADS; i++) {
    CloseHandle(hThreadArray[i]);
  }

  if(argc >= 4 && wcscmp(argv[3], L"stop_nlca_service")==0) {
    printf("Send stop\n");
    NLCAService_Stop();
  }
  DeleteCriticalSection(&displayMutex);
  return 0;
}/* main */
