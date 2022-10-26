/***********************************************************************
 *
 * Compliant Enterprise Logging - Test Application
 *
 **********************************************************************/

#include <stdio.h>
#include <iostream>
#include <windows.h>
#include <process.h>
#include <cassert>
#include "nlthread.h"
#include "celog.h"

using namespace std;

/* Required libraries */
#pragma comment(lib,"advapi32.lib")

#define CELOG_CUR_MODULE L"CELogTest"
#define CELOG_CUR_FILE CELOG_FILEPATH_PROD_COMMON_CELOG2_TEST_CELOG_TEST_CPP

typedef enum
{
  ME_ZERO, ME_ONE, ME_TWO, ME_THREE
} my_enum_t;

typedef struct
{
  char unused;
} my_struct_t;



void Pause(void)
{
  char buf[512];

  printf("Press <Enter> to continue ... ");
  gets_s(buf);
}



//
// CELOG_XXX_ENTER / CELOG_XXX_RETURN_YYY log speed test
//

int SmallFuncNoLog(void)
{
  volatile int i = 0;
  i++;
  return 13579;
}

int SmallFuncEnterLeaveLog(void)
{
  CELOG_ENTER;
  volatile int i = 0;
  i++;
  CELOG_RETURN_VAL(13579);
}

int EnterLeaveLogSpeedTest(void)
{
  nextlabs::high_resolution_timer timer;
  unsigned int numTimes;
  unsigned int i;
  unsigned long long noLogAvgTimePs, enterLeaveLogAvgTimePs;

  numTimes = 1000000000;
  timer.start();
  for (i = 0; i < numTimes; i++)
  {
    SmallFuncNoLog();
  }
  timer.stop();
  noLogAvgTimePs = (unsigned long long)
    (timer.diff() * 1000000000 / numTimes + 0.5);
  printf("SmallFuncNoLog: numTimes=%d, average=%llups\n", numTimes,
         noLogAvgTimePs);

  numTimes = 100000;
  timer.start();
  for (i = 0; i < numTimes; i++)
  {
    SmallFuncEnterLeaveLog();
  }
  timer.stop();
  enterLeaveLogAvgTimePs = (unsigned long long)
    (timer.diff() * 1000000000 / numTimes + 0.5);
  printf("SmallFuncEnterLeaveLog: numTimes=%d, average=%llups\n", numTimes,
         enterLeaveLogAvgTimePs);

  printf("Enter/Leave log average overhead is %llups\n",
         enterLeaveLogAvgTimePs - noLogAvgTimePs);

  return 0;
}



//
// Log level test
//

_Check_return_
int LogLevelTest(void)
{
  CELOG_ENTER;
  int ret;

  ret = CELOG_LOG(CELOG_CRITICAL, L"This is CRITICAL.\n");
  if (ret < 0)
  {
    fprintf(stderr, "CELOG_LOG failed!");
    CELOG_RETURN_VAL(ret);
  }

  ret = CELOG_LOG(CELOG_ERROR, L"This is ERROR.\n");
  if (ret < 0)
  {
    fprintf(stderr, "CELOG_LOG failed!");
    CELOG_RETURN_VAL(ret);
  }

  ret = CELOG_LOG(CELOG_WARNING, L"This is WARNING.\n");
  if (ret < 0)
  {
    fprintf(stderr, "CELOG_LOG failed!");
    CELOG_RETURN_VAL(ret);
  }

  ret = CELOG_LOG(CELOG_INFO, L"This is INFO.\n");
  if (ret < 0)
  {
    fprintf(stderr, "CELOG_LOG failed!");
    CELOG_RETURN_VAL(ret);
  }

  ret = CELOG_LOG(CELOG_TRACE, L"This is TRACE.\n");
  if (ret < 0)
  {
    fprintf(stderr, "CELOG_LOG failed!");
    CELOG_RETURN_VAL(ret);
  }

  ret = CELOG_LOG(CELOG_DEBUG, L"This is DEBUG.\n");
  if (ret < 0)
  {
    fprintf(stderr, "CELOG_LOG failed!");
    CELOG_RETURN_VAL(ret);
  }

  ret = CELOG_LOG(CELOG_DUMP, L"This is DUMP.\n");
  if (ret < 0)
  {
    fprintf(stderr, "CELOG_LOG failed!");
    CELOG_RETURN_VAL(ret);
  }

  ret = CELOG_LOG((celog_level_t) (CELOG_DUMP + 1),
                  L"This is invalid log level.  This line should never be printed\n");
  if (ret >= 0)
  {
    fprintf(stderr, "CELOG_LOG should fail but doesn't!");
    CELOG_RETURN_VAL(-1);
  }

  CELOG_RETURN_VAL(0);
}



//
// Variable args test
//

_Check_return_
int VarArgsTest(void)
{
  CELOG_ENTER;
  int ret;

  ret = CELOG_LOG(CELOG_INFO, L"This is line one.\n");
  printf("CELOG_LOG returned %d\n", ret);
  if (ret < 0)
  {
    fprintf(stderr, "CELOG_LOG failed!");
    CELOG_RETURN_VAL(ret);
  }

  ret = CELOG_LOG(CELOG_INFO, L"%s %s %s %d!\n", L"This", L"is", L"line", 2);
  printf("CELOG_LOG returned %d\n", ret);
  if (ret < 0)
  {
    fprintf(stderr, "CELOG_LOG failed!");
    CELOG_RETURN_VAL(ret);
  }

  CELOG_RETURN_VAL(0);
}



//
// Message-too-long test
//

int logOneMsgA(size_t logMsgLen)
{
  CELOG_ENTER;
  char *logMsg = new char[logMsgLen+1];

  for (size_t i = 0; i < logMsgLen; i++)
  {
    logMsg[i] = 'A' + i % 10;
  }
  logMsg[logMsgLen] = '\0';

  CELOG_RETURN_VAL(CELOG_LOGA(CELOG_CRITICAL, "len=%d, str=\"%s\"\n",
                              logMsgLen, logMsg));
}

int logOneMsgW(size_t logMsgLen)
{
  CELOG_ENTER;
  wchar_t *logMsg = new wchar_t[logMsgLen+1];

  for (size_t i = 0; i < logMsgLen; i++)
  {
    logMsg[i] = L'A' + i % 10;
  }
  logMsg[logMsgLen] = L'\0';

  CELOG_RETURN_VAL(CELOG_LOGW(CELOG_CRITICAL, L"len=%d, str=\"%s\"\n",
                              logMsgLen, logMsg));
}

_Check_return_
int MessageTooLongTest(void)
{
  CELOG_ENTER;
  size_t logMsgLen;
  const size_t start = 90, end = 70;

  for (logMsgLen = CELOG_MAX_LINE_LEN - start;
       logMsgLen <= CELOG_MAX_LINE_LEN - end;
       logMsgLen++)
  {
    logOneMsgA(logMsgLen);
    logOneMsgW(logMsgLen);
  }

  // A short message should be printed fine.

  if (logOneMsgA(CELOG_MAX_LINE_LEN - start) < 0)
  {
    fprintf(stderr, "CELOG_LOGA failed!");
    CELOG_RETURN_VAL(-1);
  }

  if (logOneMsgW(CELOG_MAX_LINE_LEN - start) < 0)
  {
    fprintf(stderr, "CELOG_LOGW failed!");
    CELOG_RETURN_VAL(-1);
  }

  // A long message should be printed fine, and error should be returned.

  if (logOneMsgA(CELOG_MAX_LINE_LEN - end) >= 0)
  {
    fprintf(stderr, "CELOG_LOGA should have returned error but did not!");
    CELOG_RETURN_VAL(-1);
  }

  if (logOneMsgW(CELOG_MAX_LINE_LEN - end) >= 0)
  {
    fprintf(stderr, "CELOG_LOGW should have returned error but did not!");
    CELOG_RETURN_VAL(-1);
  }

  CELOG_RETURN_VAL(0);
}



//
// Mixed SBCS/DBCS format and args test
//

_Check_return_
int MixedSbcsDbcsFmtArgsTest(void)
{
  CELOG_ENTER;

  // SBCS format with SBCS string args.
  if (CELOG_LOGA(CELOG_INFO, "This is SBCS format with %s string args.\n",
                 "SBCS") < 0)
  {
    fprintf(stderr, "CELOG_LOG failed!");
    CELOG_RETURN_VAL(-1);
  }

  // SBCS format with DBCS string args.
  if (CELOG_LOGA(CELOG_INFO, "This is SBCS format with %ls string args.\n",
                 L"DBCS") < 0)
  {
    fprintf(stderr, "CELOG_LOG failed!");
    CELOG_RETURN_VAL(-1);
  }

  // DBCS format with SBCS string args.
  if (CELOG_LOG(CELOG_INFO, L"This is DBCS format with %hs string args.\n",
                "SBCS") < 0)
  {
    fprintf(stderr, "CELOG_LOG failed!");
    CELOG_RETURN_VAL(-1);
  }

  // DBCS format with DBCS string args.
  if (CELOG_LOG(CELOG_INFO, L"This is DBCS format with %s string args.\n",
                L"DBCS") < 0)
  {
    fprintf(stderr, "CELOG_LOG failed!");
    CELOG_RETURN_VAL(-1);
  }

  CELOG_RETURN_VAL(0);
}



//
// Multi-threaded test
//

unsigned int __stdcall printLogThread(_In_ void *pParam)
{
  CELOG_ENTER;
  for (int i = 0; i < 100; i++)
  {
    if (CELOG_LOG(CELOG_DEBUG, L"This is thread %d in the test.\n",
                  * (int *) pParam) < 0)
    {
      printf("CELOG_LOG failed\n");
      CELOG_RETURN_VAL(1);
    }
  }

  CELOG_RETURN_VAL(0);
}

void MultiThreadedTest(void)
{
  CELOG_ENTER;

  nlthread_t tid1, tid2, tid3;
  int one=1, two=2, three=3;
  nlthread_create(&tid1, printLogThread, &one);
  nlthread_create(&tid2, printLogThread, &two);
  nlthread_create(&tid3, printLogThread, &three);
  nlthread_join(tid1);
  nlthread_join(tid2);
  nlthread_join(tid3);

  CELOG_RETURN;
}



//
// C++ exception test
//

_Check_return_
std::wstring CppExceptionFunc(_In_ int cond)
{
  CELOG_ENTER;

  if (cond == 1)
  {
    Sleep(123);
    CELOG_RETURN_VAL(L"one");
  }

  if (cond == 2)
  {
    Sleep(456);
    throw std::exception();
  }

  Sleep(789);
  CELOG_RETURN_VAL(L"not one or two");
}

void CppExceptionTest(void)
{
  CELOG_ENTER;
  std::wstring ret;

  ret = CppExceptionFunc(1);
  wprintf(L"CppExceptionFunc returns %s\n", ret.c_str());

  try
  {
    ret = CppExceptionFunc(2);
    wprintf(L"CppExceptionFunc returns %s\n", ret.c_str());
  }
  catch (...)
  {
    wprintf(L"CppexceptionFunc has caused an exception\n");
  }

  ret = CppExceptionFunc(3);
  wprintf(L"CppExceptionFunct returns %s\n", ret.c_str());

  CELOG_RETURN;
}



//
// SEH test
//

_Check_return_
int SEHFunc(_In_ int cond)
{
  CELOG_ENTER;

  switch (cond)
  {
  case 1:
    Sleep(234);
    CELOG_RETURN_VAL(cond * 100);
  case 2:
    Sleep(567);
    RaiseException(1, 0, 0, NULL);
    break;
  }
  
  Sleep(890);
  CELOG_RETURN_VAL(cond * 100);
}

_Check_return_
DWORD Filter(void)
{
  CELOG_ENTER;

  printf("Filter here\n");
  CELOG_RETURN_VAL(EXCEPTION_EXECUTE_HANDLER);
}

void SEHTest(void)
{
  CELOG_SEH_ENTER;
  int ret;

  ret = SEHFunc(1);
  wprintf(L"SEHFunc returns %d\n", ret);

  __try
  {
    ret = SEHFunc(2);
    wprintf(L"SEHFunc returns %d\n", ret);
  }
  __except(Filter())
  {
    wprintf(L"SEHFunc has caused an exception\n");
  }

  ret = SEHFunc(3);
  wprintf(L"SEHFunc returns %d\n", ret);

  CELOG_SEH_RETURN;
}



//
// Normal return value test
//

void NormalTestRetVoid(void)
{
  CELOG_ENTER;
  CELOG_RETURN;
}

void NormalTestRetVoidExp(void)
{
  CELOG_ENTER;
  CELOG_RETURN_VOIDEXP(NormalTestRetVoid());
}

bool NormalTestRetBool(void)
{
  CELOG_ENTER;
  CELOG_RETURN_VAL(true);
}

short NormalTestRetShort(void)
{
  CELOG_ENTER;
  CELOG_RETURN_VAL((short) (1 + 2 * 3));
}

int NormalTestRetInt(void)
{
  CELOG_ENTER;
  CELOG_RETURN_VAL(1 + 2 * 3);
}

long NormalTestRetLong(void)
{
  CELOG_ENTER;
  CELOG_RETURN_VAL(1L + 2 * 3);
}

long long NormalTestRetLongLong(void)
{
  CELOG_ENTER;
  CELOG_RETURN_VAL(1LL + 2 * 3);
}

unsigned short NormalTestRetUShort(void)
{
  CELOG_ENTER;
  CELOG_RETURN_VAL((unsigned short)0x0123);
}

unsigned int NormalTestRetUInt(void)
{
  CELOG_ENTER;
  CELOG_RETURN_VAL(0x01230000U | 0x00000ABCU);
}

unsigned long NormalTestRetULong(void)
{
  CELOG_ENTER;
  CELOG_RETURN_VAL(0x01230000UL | 0x00000ABCUL);
}

unsigned long long NormalTestRetULongLong(void)
{
  CELOG_ENTER;
  CELOG_RETURN_VAL(0x0456000000000000ULL | 0x0000000000000DEFULL);
}

_Ret_opt_z_
char *NormalTestRetCharPtr(bool returnNull)
{
  CELOG_ENTER;
  CELOG_RETURN_VAL(returnNull ? NULL : "This is a char *.");
}

_Ret_opt_z_
wchar_t *NormalTestRetWcharPtr(bool returnNull)
{
  CELOG_ENTER;
  CELOG_RETURN_VAL(returnNull ? NULL : L"This is a wchar_t *.");
}

std::string NormalTestRetStr(void)
{
  CELOG_ENTER;
  CELOG_RETURN_VAL(std::string("This is a string."));
}

std::wstring NormalTestRetWstr(void)
{
  CELOG_ENTER;
  CELOG_RETURN_VAL(std::wstring(L"This is a wstring."));
}

_Ret_opt_
void *NormalTestRetVoidPtr(bool returnNull)
{
  CELOG_ENTER;
  int x;
  CELOG_RETURN_VAL(returnNull ? NULL : &x + 1);
}

my_enum_t NormalTestRetEnum(void)
{
  CELOG_ENUM_ENTER(my_enum_t);
  CELOG_ENUM_RETURN_VAL(ME_THREE);
}

_Ret_opt_
my_struct_t *NormalTestRetNonVoidPtr(bool returnNull)
{
  CELOG_PTR_ENTER(my_struct_t *);
  my_struct_t s;
  CELOG_PTR_RETURN_VAL(returnNull ? NULL : &s + 1);
}

my_struct_t NormalTestRetUnsupportedType(void)
{
  CELOG_ENTER;
  my_struct_t s = {};
  CELOG_RETURN_VAL_NOPRINT(s);
}

void NormalTestRet(void)
{
  CELOG_ENTER;

  NormalTestRetVoid();
  NormalTestRetVoidExp();
  NormalTestRetBool();
  NormalTestRetShort();
  NormalTestRetInt();
  NormalTestRetLong();
  NormalTestRetLongLong();
  NormalTestRetUShort();
  NormalTestRetUInt();
  NormalTestRetULong();
  NormalTestRetULongLong();
  NormalTestRetCharPtr(false);
  NormalTestRetCharPtr(true);
  NormalTestRetWcharPtr(false);
  NormalTestRetWcharPtr(true);
  NormalTestRetStr();
  NormalTestRetWstr();
  NormalTestRetVoidPtr(false);
  NormalTestRetVoidPtr(true);
  NormalTestRetEnum();
  NormalTestRetNonVoidPtr(false);
  NormalTestRetNonVoidPtr(true);
  NormalTestRetUnsupportedType();

  CELOG_RETURN;
}



//
// SEH return value test
//

void SEHTestRetVoid(void)
{
  CELOG_SEH_ENTER;
  CELOG_SEH_RETURN;
}

void SEHTestRetVoidExp(void)
{
  CELOG_SEH_ENTER;
  CELOG_SEH_RETURN_VOIDEXP(SEHTestRetVoid());
}

bool SEHTestRetBool(void)
{
  CELOG_SEH_ENTER;
  CELOG_SEH_RETURN_VAL(false);
}

short SEHTestRetShort(void)
{
  CELOG_SEH_ENTER;
  CELOG_SEH_RETURN_VAL((short) (1 + 2 * 3));
}

int SEHTestRetInt(void)
{
  CELOG_SEH_ENTER;
  CELOG_SEH_RETURN_VAL(1 + 2 * 3);
}

long SEHTestRetLong(void)
{
  CELOG_SEH_ENTER;
  CELOG_SEH_RETURN_VAL(1L + 2 * 3);
}

long long SEHTestRetLongLong(void)
{
  CELOG_SEH_ENTER;
  CELOG_SEH_RETURN_VAL(1LL + 2 * 3);
}

unsigned short SEHTestRetUShort(void)
{
  CELOG_SEH_ENTER;
  CELOG_SEH_RETURN_VAL((unsigned short)0x0123);
}

unsigned int SEHTestRetUInt(void)
{
  CELOG_SEH_ENTER;
  CELOG_SEH_RETURN_VAL(0x01230000U | 0x00000ABCU);
}

unsigned long SEHTestRetULong(void)
{
  CELOG_SEH_ENTER;
  CELOG_SEH_RETURN_VAL(0x01230000UL | 0x00000ABCUL);
}

unsigned long long SEHTestRetULongLong(void)
{
  CELOG_SEH_ENTER;
  CELOG_SEH_RETURN_VAL(0x0456000000000000ULL | 0x0000000000000DEFULL);
}

_Ret_opt_z_
char *SEHTestRetCharPtr(bool returnNull)
{
  CELOG_SEH_ENTER;
  CELOG_SEH_RETURN_VAL(returnNull ? NULL : "This is a char *.");
}

_Ret_opt_z_
wchar_t *SEHTestRetWcharPtr(bool returnNull)
{
  CELOG_SEH_ENTER;
  CELOG_SEH_RETURN_VAL(returnNull ? NULL : L"This is a wchar_t *.");
}

std::string SEHTestRetStr(void)
{
  CELOG_SEH_ENTER;
  CELOG_SEH_RETURN_VAL(std::string("This is a string."));
}

std::wstring SEHTestRetWstr(void)
{
  CELOG_SEH_ENTER;
  CELOG_SEH_RETURN_VAL(std::wstring(L"This is a wstring."));
}

_Ret_opt_
void *SEHTestRetVoidPtr(bool returnNull)
{
  CELOG_SEH_ENTER;
  int x;
  CELOG_SEH_RETURN_VAL(returnNull ? NULL : &x + 1);
}

my_enum_t SEHTestRetEnum(void)
{
  CELOG_SEH_ENUM_ENTER(my_enum_t);
  CELOG_SEH_ENUM_RETURN_VAL(ME_THREE);
}

_Ret_opt_
my_struct_t *SEHTestRetNonVoidPtr(bool returnNull)
{
  CELOG_SEH_PTR_ENTER(my_struct_t *);
  my_struct_t s;
  CELOG_SEH_PTR_RETURN_VAL(returnNull ? NULL : &s + 1);
}

my_struct_t SEHTestRetUnsupportedType(void)
{
  CELOG_SEH_ENTER;
  my_struct_t s = {};
  CELOG_SEH_RETURN_VAL_NOPRINT(s);
}

void SEHTestRet(void)
{
  CELOG_ENTER;

  SEHTestRetVoid();
  SEHTestRetVoidExp();
  SEHTestRetBool();
  SEHTestRetShort();
  SEHTestRetInt();
  SEHTestRetLong();
  SEHTestRetLongLong();
  SEHTestRetUShort();
  SEHTestRetUInt();
  SEHTestRetULong();
  SEHTestRetULongLong();
  SEHTestRetCharPtr(false);
  SEHTestRetCharPtr(true);
  SEHTestRetWcharPtr(false);
  SEHTestRetWcharPtr(true);
  SEHTestRetStr();
  SEHTestRetWstr();
  SEHTestRetVoidPtr(false);
  SEHTestRetVoidPtr(true);
  SEHTestRetEnum();
  SEHTestRetNonVoidPtr(false);
  SEHTestRetNonVoidPtr(true);
  SEHTestRetUnsupportedType();

  CELOG_RETURN;
}

// test of :
// 1, Log files Rotation
// 2, Deleting old log files
// 3, Changing the setting on the fly
bool ReadKey( const wchar_t* in_key , void* in_value , size_t in_value_size )
{
    if( in_key == NULL || in_value == NULL )
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return false;
    }

    /* Parse out key path and key name */
    std::wstring key_path(in_key);
    std::wstring::size_type i = key_path.find_last_of(L"\\//");
    if( i == std::wstring::npos )
    {
        return false;
    }
    std::wstring key_name(key_path,i+1); // capture key name
    key_path.erase(i);                   // trim tail which includes key name.

    HKEY hKey = NULL;
    LONG rstatus = RegOpenKeyExW(HKEY_LOCAL_MACHINE,key_path.c_str(),0,KEY_QUERY_VALUE,&hKey);
    if( rstatus != ERROR_SUCCESS )
    {
        return false;
    }

    DWORD result_size = (DWORD)in_value_size;
    rstatus = RegQueryValueExW(hKey,key_name.c_str(),NULL,NULL,(LPBYTE)in_value,&result_size);
    RegCloseKey(hKey);
    if( rstatus != ERROR_SUCCESS )
    {
        return false;
    }
    SetLastError(ERROR_SUCCESS);
    return true;
}/* ReadKey */

static bool ReadKey( const wchar_t* in_key , wchar_t* in_value , size_t in_value_count )
{
    return ReadKey(in_key,(void*)in_value,in_value_count*sizeof(wchar_t));
}

void RotateDelChangeTest()
{
    unsigned long logFileNum=0;
    unsigned long logFileSize=0;
    wstring path(L"C:\\Program Files\\NextLabs\\diags\\logs\\CELogTest.exe_");
    wstringstream filename;
// get the file number and file size from reg
    if( !ReadKey(L"SOFTWARE\\NextLabs\\Logs\\NumOfLogFiles", &logFileNum, sizeof(DWORD))  )
    {
        logFileNum=10;
    }
    if( !ReadKey(L"SOFTWARE\\NextLabs\\Logs\\LogFileSize", &logFileSize, sizeof(DWORD))  )
    {
        logFileSize=1048576;
    }   
    long maxLogs=logFileNum * logFileSize/500;
    cout<<"Begin to test log file Rotation"<<endl;
    Pause();
    for(int i=0;i< maxLogs*1.5;++i)
    {   
        if(!LogLevelTest())
            continue;
    }
    for(unsigned long i=0;i<logFileNum;i+=2)
    {
        filename.str(L"");
        filename<<path<<_getpid()<<".log."<<i;
        printf("The filename is : %ls \n",filename.str().c_str());
        errno_t ret=_wremove(filename.str().c_str()) ;
        cout<<"ret is :"<<ret<<endl;
    }
    cout<<"Creating some missing files"<<endl;
    Pause();
    
    cout<<"Begin to test Changing on the fly and deleting old files"<<endl;
    Pause();
    for(int i=0;i< maxLogs*1.5;++i)
    {
        if(!LogLevelTest())
            continue;
    }
    cout<<"Please change the setting in the reg"<<endl;
    Pause();
    for(int i=0;i< maxLogs*1.5;++i)
    {
        if(!LogLevelTest())
            continue;
    }
    cout<<"Please check the differences now"<<endl;
    Pause();
}


int main(void)
{
  CELOG_ENTER;

  printf("CELogTest start, PID is %d\n", _getpid());

  // Log level test
  if (LogLevelTest() != 0)
  {
    CELOG_RETURN_VAL(-1);
  }

  // Variable args test
  if (VarArgsTest() != 0)
  {
    CELOG_RETURN_VAL(-1);
  }

  // Message-too-long test
  if (MessageTooLongTest() != 0)
  {
    CELOG_RETURN_VAL(-1);
  }

  // Mixed SBCS/DBCS format and args test
  if (MixedSbcsDbcsFmtArgsTest() != 0)
  {
    CELOG_RETURN_VAL(-1);
  }

  // Multi-threaded test
  MultiThreadedTest();

  // C++ exception test
  CppExceptionTest();

  // SEH test
  SEHTest();

  // Normal return value test
  NormalTestRet();

  // SEH return value test
  SEHTestRet();
  
  // test log file Rotation
  RotateDelChangeTest(); 
  
  // Enter/Leave log speed test
  EnterLeaveLogSpeedTest();

  printf("CELogTest end\n");
  
  CELOG_RETURN_VAL(0);
}/* main */
