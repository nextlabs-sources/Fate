/*************************************************************************
 *
 * Compliant Enterprise Logging
 *
 * Implementation file for logging manager interface described in celog_mgr.h
 *
 ************************************************************************/

#include <stdio.h>
#include <errno.h>
#include <sstream>
#include <time.h>
#include <sys/timeb.h>
#include <process.h>
#include <io.h>
#include <regex>
#include <boost/filesystem.hpp>
#include <boost/system/error_code.hpp>
#include "nlconfig.hpp"
#include "eframework/auto_disable/auto_disable.hpp"
#include "celog.h"
#include "celog_mgr.h"



static const wchar_t* levelTable[] =
{
  L"CRITICAL",  // CELOG_CRITICAL
  L"ERROR",     // CELOG_ERROR
  L"WARNING",   // CELOG_WARNING
  L"INFO",      // CELOG_INFO
  L"TRACE",     // CELOG_TRACE
  L"DEBUG",     // CELOG_DEBUG
  L"DUMP"       // CELOG_DUMP
};


CELogMgr mgr;
static nextlabs::recursion_control log_control;
static BOOL g_bThreadExit = TRUE;

CELogMgr::CELogMgr(void)
{
  logMutex_ = NULL;
  logFile_ = NULL;
  isDebugProcess_ =false;
  timeTOAccessReg_=0;
  config_.logFileSize=1048576;
  config_.logFileNum=10;
  config_.logFileKeptDays=10;
  config_.logEnabled=1;
}

void CELogMgr::PrintTimeZone(void)
{
  if(logFile_==NULL)
    return;
  TIME_ZONE_INFORMATION tziTest;
  DWORD dwRet;
  dwRet = GetTimeZoneInformation(&tziTest);
 
  if(dwRet == TIME_ZONE_ID_STANDARD || dwRet == TIME_ZONE_ID_UNKNOWN) 
  {  
     fputws(L"Local time zone is " , logFile_);
     fputws(tziTest.StandardName  , logFile_);
     fputws(L".  Timestamps below are in UTC." , logFile_);    
     fputws(L"\n" , logFile_);    
  }
  else if( dwRet == TIME_ZONE_ID_DAYLIGHT )
  {
     fputws(L"Local TimeZone is " , logFile_);
     fputws(tziTest.DaylightName  , logFile_);
     fputws(L",Time Stamps below are in UTC " , logFile_);    
     fputws(L"\n" , logFile_); 
  }
}

void CELogMgr::GetRotationInfo(void)
{
  //////////////////////////////////////////////////
  //     Get the log file setting information
  /////////////////////////////////////////////////
  // get the numbers of log files
  if(NLConfig::ReadKey(L"SOFTWARE\\NextLabs\\Logs\\NumOfLogFiles",&config_.logFileNum, sizeof(config_.logFileNum) )  )
  {
    if(config_.logFileNum<2)
      config_.logFileNum=2;
    if(config_.logFileNum>100)
      config_.logFileNum=100;
  }
  // get the size of a log file
  if(NLConfig::ReadKey(L"SOFTWARE\\NextLabs\\Logs\\LogFileSize",&config_.logFileSize,sizeof(config_.logFileSize)) )
  {
    if(config_.logFileSize<128000)
      config_.logFileSize=128000;
    if(config_.logFileSize>2147483648)
      config_.logFileSize=2147483648;
  }
}

void CELogMgr::GetDeletionOldLogInfo(void)
{
  if(NLConfig::ReadKey(L"SOFTWARE\\NextLabs\\Logs\\NumOfDaysToKeep",&config_.logFileKeptDays, sizeof(config_.logFileKeptDays) )  )
  {
    if(config_.logFileKeptDays<1)
         config_.logFileKeptDays=1;
    if(config_.logFileKeptDays>2147483647)
         config_.logFileKeptDays=2147483647;
  }

}

void CELogMgr::GetLogEnableInfo(void)
{
  if(! NLConfig::ReadKey(L"SOFTWARE\\NextLabs\\Logs\\LogEnabled",&config_.logEnabled , sizeof(config_.logEnabled ) )  )
  {
    config_.logEnabled=1;
  }

}

void CELogMgr::ReadConfig(void)
{
  int perfLogModeInt, logLevelInt, debugModeInt;

  if (NLConfig::ReadKey(L"SOFTWARE\\NextLabs\\Logs\\PerfLogMode",
                        &perfLogModeInt))
  {
    config_.perfLogMode = (perfLogModeInt != 0);
  }
  else
  {
    config_.perfLogMode = false;
  }

  if (config_.perfLogMode)
  {
    // Force log level to be INFO.
    config_.logLevel = CELOG_INFO;
  }
  else
  {
    // Read log level from config.
    if (NLConfig::ReadKey(L"SOFTWARE\\NextLabs\\Logs\\LogLevel", &logLevelInt))
    {
      config_.logLevel = logLevelInt > CELOG_DUMP ?
        CELOG_DUMP : (celog_level_t) logLevelInt;
    }
    else
    {
      config_.logLevel = CELOG_WARNING;
    }
  }

  if (NLConfig::ReadKey(L"SOFTWARE\\NextLabs\\Logs\\DebugMode", &debugModeInt))
  {
    config_.debugMode = (debugModeInt != 0);
  }
  else
  {
    config_.debugMode = false;
  }
  GetRotationInfo();
  GetLogEnableInfo();
}

void CELogMgr::ReadNxDir(void)
{
  // Get NextLabs installation path.
  WCHAR nlse_root[MAX_PATH] = {0};

  if (!NLConfig::ReadKey(L"SOFTWARE\\NextLabs\\Compliant Enterprise\\Policy Controller\\InstallDir",
                         nlse_root, _countof(nlse_root)))
  {
    wcsncpy_s(nlse_root,_countof(nlse_root),L"C:\\Program Files\\NextLabs\\",_TRUNCATE);
  }
  std::wstringstream logFilePath;

  logFilePath << nlse_root;
  nxPath_=logFilePath.str();
}

_Check_return_
bool CELogMgr::IsKnownDebugger(_In_z_ const WCHAR *ProcessName)
{
    const WCHAR *DebugValueOne=L"windbg.exe";
    const WCHAR *DebugValueTwo=L"dbgview.exe";
    if(_wcsicmp(DebugValueOne,ProcessName)==0)
        return true;
    else
        return (_wcsicmp(DebugValueTwo,ProcessName)==0 );
}

_Check_return_
bool CELogMgr::IsPc(_In_z_ const WCHAR *ProcessName)
{
    const WCHAR pcExeName[] = L"cepdpman.exe";
    return _wcsicmp(pcExeName, ProcessName) == 0;
}

void CELogMgr::FindEXEName(void)
{
  // Get name of the .exe in this process.
  WCHAR exeFullPath[MAX_PATH];
  const WCHAR *exeName;

  if (GetModuleFileNameW(NULL, exeFullPath, _countof(exeFullPath)) == 0)
  {
    exeFullPath[0] = L'\0';
  }

  exeName = wcsrchr(exeFullPath, L'\\');
  if (exeName == NULL)
  {
    exeName = exeFullPath;
  }
  else
  {
    exeName++;  //we want to start from the character after the '\'.
  }
  exeName_= wstring(exeName);
}

void CELogMgr::FindModuleName(void)
{
  // Get name of the module containing CELog3 in this process.  This is one of
  // the following:
  // 1. name of the .exe in this process (if the .exe uses CELog3)
  // 2. name of a DLL in this process (if the .exe loads one or more DLLs that
  //    use CELog3)
  //
  // Note that #1 and #2 can both exist in the same process at the same time.
  // In addition, #2 can exist multiple times in the same process at the same
  // time.

  HMODULE hModule;
  WCHAR moduleFullPath[MAX_PATH];
  const WCHAR *moduleName;
  static const WCHAR dummy;

  if (!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                          GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                          &dummy, &hModule) ||
      GetModuleFileNameW(hModule, moduleFullPath, _countof(moduleFullPath))
      == 0)
  {
    moduleFullPath[0] = L'\0';
  }

  moduleName = wcsrchr(moduleFullPath, L'\\');
  if (moduleName == NULL)
  {
    moduleName = moduleFullPath;
  }
  else
  {
    moduleName++;  //we want to start from the character after the '\'.
  }

  moduleName_ = wstring(moduleName);
}

bool CELogMgr::OpenLogFile(void)
{
  // Construct filename using executable name and PID.
  std::wstringstream logFilePath;

  logFilePath << nxPath_ << "diags\\logs\\" << exeName_  << '_';
  if (exeName_ != moduleName_)
  {
    logFilePath << moduleName_ << '_';
  }
  logFilePath << _getpid() << ".log.";

  // Get NextLabs log file path.
  path_=logFilePath.str(); 
  
  logFilePath.str(L"");
  logFilePath << path_ <<"0";

  // For some reason, _wfopen_s() returns EPERM instead of 0 to us even when
  // it can successfully create a new file or open an existing file.  So we
  // just ignore EPERM here.

  logFile_ = _wfsopen(logFilePath.str().c_str(),L"wt", _SH_DENYWR );
  PrintTimeZone();

  return logFile_!=NULL;
}

void CELogMgr::CloseLogFile(void)
{
  if (logFile_ != NULL)
  {
    fclose(logFile_);
    logFile_ = NULL;
  }
}

static unsigned __stdcall DeleteOldLogFileThread(void *pMgr)
{
  SetThreadPriority(GetCurrentThread(), THREAD_MODE_BACKGROUND_BEGIN);
  ((CELogMgr *)pMgr)->DeleteOldLogFileReal();
  SetThreadPriority(GetCurrentThread(), THREAD_MODE_BACKGROUND_END);

  g_bThreadExit = TRUE;
  return 0;
}

// boost is used in this function
void CELogMgr::DeleteOldLogFileReal(void)
{
    wstring logDir=nxPath_+L"diags\\logs\\";
    boost::filesystem::wpath logPath(logDir);

    if ( boost::filesystem::exists(logPath) && boost::filesystem::is_directory(logPath))
    {
        boost::filesystem::wrecursive_directory_iterator it(logPath);
        boost::filesystem::wrecursive_directory_iterator endit;
        while(it != endit && !deleteOldLogFileThreadStopFlag_)
       {
            if (is_regular_file(*it))
            {
                // this regular expression is not part of the boost library, it is part of the C++ library
                wstring fileName(it->path().filename() );
                tr1::wregex resExe(L"(.*)([Ee][Xx][Ee]_)([[:digit:]]{1,})(\\.)log(\\.)([[:digit:]]{1,})");
                tr1::wregex resCom(L"(.*)([Cc][Oo][Mm]_)([[:digit:]]{1,})(\\.)log(\\.)([[:digit:]]{1,})");
        
                if (regex_match(fileName,resExe ) || regex_match(fileName,resCom ))
                {
                   time_t fileLastChange = boost::filesystem::last_write_time( it->path() ) ;
                   // get current time
                   time_t currentTime;
                   time(&currentTime); 
                  
                   if(difftime(currentTime,fileLastChange) > 24*60*60*config_.logFileKeptDays )     
                   {
                        // if the time is older then our setting,delete it.
                        try
                        {
                            boost::filesystem::remove(it->path());
                        }
                        catch (exception e)
                        {
                        // if a file can not be deleted at the time, forget it 
                                ++it;
                                continue;               
                        }
                   }
                }
            }
            ++it;
        }
    }
}

void CELogMgr::EnsureDeleteOldLogFiles(void)
{
  if (deleteOldLogFileThreadSpawned_)
    return;

  deleteOldLogFileThreadSpawned_ = true;        // Assume that the thread can
                                                // be spawned successfully
                                                // below.

  deleteOldLogFileThreadStopFlag_ = false;
  g_bThreadExit = FALSE;
  deleteOldLogFileThreadHandle_ = (HANDLE)
    _beginthreadex(NULL, 0, &DeleteOldLogFileThread, &mgr, 0, NULL);
}

_Check_return_
bool CELogMgr::Init(void)
{
  if (!nlthread_mutex_init(&logMutex_))
  {
    return false;
  }
  time(&timeTOAccessReg_); 
  ReadConfig();
  GetDeletionOldLogInfo();
 
  // Get NextLabs installation path.
  ReadNxDir();
  // Get the name of current process.
  FindEXEName();
  // Get the name of current module.
  FindModuleName();
  // If current process name is equal to debugView.exe or WinDbg, set debugMode to 0 
  if(IsKnownDebugger(exeName_.c_str()))
  {
    isDebugProcess_= true; 
  }

  isPcProcess_ = IsPc(exeName_.c_str());

  deleteOldLogFileThreadSpawned_ = false;
  deleteOldLogFileThreadHandle_ = NULL;
  
  return OpenLogFile();
}

void CELogMgr::Destroy(void)
{
  if (deleteOldLogFileThreadHandle_ != NULL)
  {
    deleteOldLogFileThreadStopFlag_ = true;
    int index = 0;
    // at most wait 2 seconds. 
    while (g_bThreadExit != TRUE && index < 4)
    {
      Sleep(500);
      ++index;
    }
    CloseHandle(deleteOldLogFileThreadHandle_);
    deleteOldLogFileThreadHandle_ = NULL;
  }

  CloseLogFile();

  nlthread_mutex_destroy(&logMutex_);
}

_Check_return_
bool CELogMgr::IsPerfLogMode(void)
{
  if (!nlthread_mutex_lock(&logMutex_))
  {
    return false;
  }

  bool ret = config_.perfLogMode;
  nlthread_mutex_unlock(&logMutex_);
  return ret;
}

int CELogMgr::Log(_In_          celog_filepathint_t fileInt,
                  _In_          int line,
                  _In_z_        const wchar_t *mod,
                  _In_          celog_level_t level,
                  _In_z_ _Printf_format_string_ const char *fmt,
                  _In_          va_list argPtr)
{
  return LogReal(fileInt, line, mod, level, fmt, NULL, argPtr);
}

int CELogMgr::Log(_In_          celog_filepathint_t fileInt,
                  _In_          int line,
                  _In_z_        const wchar_t *mod,
                  _In_          celog_level_t level,
                  _In_z_ _Printf_format_string_ const wchar_t *fmt,
                  _In_          va_list argPtr)
{
  return LogReal(fileInt, line, mod, level, NULL, fmt, argPtr);
}

bool CELogMgr::CheckRotation(void)
{
  // checks the file size of log.0
  __int64 sizeOfLog0;
  std::wstringstream logFileNextPath;
  std::wstringstream logFilePath;

  int attempt=0;
  sizeOfLog0= _filelengthi64(  _fileno(logFile_ ) );

  // check if the file size accross the boundry
  // The buffer size about to flush into the file is about 4K, need to calculate that too.
  if(sizeOfLog0+4096>=config_.logFileSize)
  {
    // close the current file
    CloseLogFile();

    // check the last existing log file
    unsigned long lastFile=config_.logFileNum-1;
    std::wstringstream numBuf;
    errno_t err = 0;
    do
    {
      numBuf.str(L"");
      numBuf<<path_<<lastFile;

      if ((err = _waccess_s( numBuf.str().c_str(), 0 )) == 0 )
      {
        // file exist
        break;
      }
      lastFile--;
    }while(lastFile!=0);

    // rotate files
    // if the file tried to be deleted can not be deleted, , the next one to be rotated will be deleted
    bool lastLogFlag=true;
    do
    {
      if(lastFile==config_.logFileNum-1)
      {
        if(lastFile==0)
        {
          // the setting of logFileNum is 1
          break;
        }
        else
        {
          lastFile--;
          continue;
        }
      }
      logFilePath.str(L"");
      logFilePath << path_<<lastFile;

      logFileNextPath.str(L"");
      logFileNextPath << path_<<(lastFile+1);
      // delete the last file
      attempt=0;
      while(lastLogFlag==true && _wremove(logFileNextPath.str().c_str()) != 0)
      {
        if(errno == ENOENT )    // file not exist
        {
          break;
        }
        if(++attempt>=3)
        {
          lastLogFlag=false;
          break;
        }
      }
      if(lastLogFlag ==false)
      {
        // file can not be removed
        lastLogFlag=true;
        lastFile--;
        continue;
      }
      else
      {
        attempt=0;
        while(_wrename(logFilePath.str().c_str(), logFileNextPath.str().c_str())!=0 )
        {
          if(++attempt>=3)
          {
            break;
          }
        }
      }
      if(lastFile==0)
        break;
      lastFile--;
    }while(true);

    //create log.0 file, if existed, destroy its original contents
    logFilePath.str(L"");
    logFilePath << path_<<"0";

    if( (logFile_=_wfsopen(logFilePath.str().c_str(),L"wt", _SH_DENYWR ) )!=NULL )
    {
      PrintTimeZone();
      return true;
    }
    else
    {
      logFile_=NULL;
      return false;
     }
  }
  return true;
}


_Check_return_
int CELogMgr::LogReal(_In_              celog_filepathint_t fileInt,
                      _In_              int line,
                      _In_z_            const wchar_t *mod,
                      _In_              celog_level_t level,
                      _In_opt_z_ _Printf_format_string_ const char *sbFmt,
                      _In_opt_z_ _Printf_format_string_ const wchar_t *dbFmt,
                      _In_              va_list argPtr)
{
  if (log_control.is_thread_disabled())
  {
    return 0;
  }

  nextlabs::recursion_control_auto auto_disable(log_control);

  // Make sure that one and only one format string is passed.
  assert((sbFmt == NULL) != (dbFmt == NULL));
  if ((sbFmt == NULL) == (dbFmt == NULL))
  {
    return -1;
  }

  // Return error if level of this msg is invalid.
  if (level >= _countof(levelTable))
  {
    return -1;
  }

  if (!nlthread_mutex_lock(&logMutex_))
  {
    return -1;
  }
   
  time_t currentTime;
  time(&currentTime); 
  if(difftime(currentTime,timeTOAccessReg_)>5)
  { 
    timeTOAccessReg_=currentTime;
    ReadConfig();
  }

  if (isPcProcess_)
  {
    EnsureDeleteOldLogFiles();
  }
   
  // Skip printing if logEnable is set to 0 .
  if(config_.logEnabled==0)
  {
    return nlthread_mutex_unlock(&logMutex_) ? 0 : -1;   
  }
  
  // Skip printing if level of this msg is higher than current log level.
  if (level > config_.logLevel)
  {
    return nlthread_mutex_unlock(&logMutex_) ? 0 : -1;
  }
  
    
  if(logFile_==NULL)
  {
    std::wstringstream logFilePath;
    logFilePath.str(L"");
    logFilePath << path_<<"0";
    if( (logFile_ = _wfsopen(logFilePath.str().c_str(),L"wt", _SH_DENYWR ) )==NULL)
    {
       nlthread_mutex_unlock(&logMutex_);
       return -1;
    }
    else
        PrintTimeZone();
  }
  
  if(! CheckRotation())
  {
    nlthread_mutex_unlock(&logMutex_);
    return -1;
  }

  // Get UTC time.
  struct __timeb64 timeb;
  struct tm _tm;

  if (_ftime64_s(&timeb) != 0 || _gmtime64_s(&_tm, &timeb.time) != 0)
  {
    nlthread_mutex_unlock(&logMutex_);
    return -1;
  }


  // Construct timestamp string "YYYY/MM/DD HH:MM:SS.mmmUTC".
  // 1. Construct "YYYY/MM/DD HH:MM:SS" (19 chars).
  // 2. Append ".mmmUTC" (7 chars).
  wchar_t timestampStr[19 + 7 + 1];     // "YYYY/MM/DD HH:MM:SS.mmmUTC"

  wcsftime(timestampStr, 19 + 1, L"%Y/%m/%d %H:%M:%S", &_tm);
  swprintf_s(&timestampStr[19], 7 + 1, L".%03huUTC", timeb.millitm);

  int ret1, ret2, ret3;

  if (sbFmt != NULL)
  {
    // Construct the SBCS log line.
    char sbcsLogLine[CELOG_MAX_LINE_LEN];

    ret1 = sprintf_s(sbcsLogLine, _countof(sbcsLogLine),
                     "%ls P%d T%lu F%d L%d %ls %ls: ",
                     timestampStr, _getpid(), nlthread_selfID(), fileInt,
                     line, mod, levelTable[level]);
    if (ret1 == -1)
    {
      nlthread_mutex_unlock(&logMutex_);
      return -1;
    }

    ret2 = _vsnprintf_s(sbcsLogLine + ret1, _countof(sbcsLogLine) - ret1,
                        _TRUNCATE, sbFmt, argPtr);

    // If _vsnprintf_s() returned any error value other than -1 (i.e. "string
    // truncated"), return error here.
    if (ret2 < -1)
    {
      nlthread_mutex_unlock(&logMutex_);
      return -1;
    }

    // Print the log line.
    ret3 = fputs(sbcsLogLine, logFile_);

    if (config_.debugMode && !isDebugProcess_ )
    {
      OutputDebugStringA(sbcsLogLine);
    }
  }
  else
  {
    // Construct the DBCS log line.
    wchar_t dbcsLogLine[CELOG_MAX_LINE_LEN];

    ret1 = swprintf_s(dbcsLogLine, _countof(dbcsLogLine),
                      L"%ls P%d T%lu F%d L%d %ls %ls: ",
                      timestampStr, _getpid(), nlthread_selfID(), fileInt,
                      line, mod, levelTable[level]);
    if (ret1 == -1)
    {
      nlthread_mutex_unlock(&logMutex_);
      return -1;
    }

    ret2 = _vsnwprintf_s(dbcsLogLine + ret1, _countof(dbcsLogLine) - ret1,
                         _TRUNCATE, dbFmt, argPtr);

    // If _vsnwprintf_s() returned any error value other than -1 (i.e. "string
    // truncated"), return error here.
    if (ret2 < -1)
    {
      nlthread_mutex_unlock(&logMutex_);
      return -1;
    }

    // Print the log line.
    ret3 = fputws(dbcsLogLine, logFile_);

    if (config_.debugMode && !isDebugProcess_)
    {
      OutputDebugStringW(dbcsLogLine);
    }
  }

  bool ret4 = nlthread_mutex_unlock(&logMutex_);

  // If string was not truncated, return number of chars pritned.
  // If string was truncated or any other error occurred, return error.
  return (ret2 >= 0 && ret3 >= 0 && ret4) ? (ret1 + ret2) : -1;
}




