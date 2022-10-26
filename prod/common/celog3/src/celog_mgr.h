/*************************************************************************
 *
 * Compliant Enterprise Logging
 *
 * Declarations for internal logging manager interface.
 *
 ************************************************************************/

#ifndef __CELOG_MGR_H__
#define __CELOG_MGR_H__



#include <stdio.h>
#include <iostream>
#include <sstream>
#include "nlthread.h"


using namespace std;



class CELogMgr
{
public:
  CELogMgr(void);
  _Check_return_
  bool Init(void);

  void Destroy(void);

  _Check_return_
  bool IsPerfLogMode(void);

  int Log(_In_          celog_filepathint_t fileInt,
          _In_          int line,
          _In_z_        const wchar_t *mod,
          _In_          celog_level_t level,
          _In_z_ _Printf_format_string_ const char *fmt,
          _In_          va_list argPtr);

  int Log(_In_          celog_filepathint_t fileInt,
          _In_          int line,
          _In_z_        const wchar_t *mod,
          _In_          celog_level_t level,
          _In_z_ _Printf_format_string_ const wchar_t *fmt,
          _In_          va_list argPtr);

public:
  void DeleteOldLogFileReal(void);

private:
  void ReadConfig(void);
  bool CheckRotation(void); // to check and do the log file rotation
  void GetRotationInfo(void); // to pull out the log info from register
  void EnsureDeleteOldLogFiles(void);// to delete the old log files
  void GetDeletionOldLogInfo(void); // to pull out the log info from register
  void GetLogEnableInfo(void); // to pull out out the info of log enable setting
  void ReadNxDir(void); //to Get NextLabs installation path
  void FindEXEName(void) ; // to find out the name of current process
  void FindModuleName(void);    // to find out the name of current module
  _Check_return_
  static bool IsKnownDebugger(_In_z_ const WCHAR *ProcessName); // to check if current process is identical to the debug process   
  _Check_return_
  static bool IsPc(_In_z_ const WCHAR *ProcessName); // to check if current process is the PC process
  bool OpenLogFile(void);
  void PrintTimeZone(void);
  void CloseLogFile(void);

  _Check_return_
  int LogReal(_In_              celog_filepathint_t fileInt,
              _In_              int line,
              _In_z_            const wchar_t *mod,
              _In_              celog_level_t level,
              _In_opt_z_ _Printf_format_string_ const char *sbFmt,
              _In_opt_z_ _Printf_format_string_ const wchar_t *dbFmt,
              _In_              va_list argPtr);

private:
  nlthread_mutex_t logMutex_;
  FILE *logFile_;
  wstring path_;		  // path of the log file
  wstring nxPath_;       // NextLabs installation path 
  time_t timeTOAccessReg_;  // Last time to access the reg to get the info
  bool isDebugProcess_; // If the current process a debug process
  bool isPcProcess_; // If the current process is the PC process
  wstring exeName_;   // the name of current process
  wstring moduleName_;  // the name of current module

  bool deleteOldLogFileThreadSpawned_;  // true if the thread has every been
                                        // spawned since module initialization
  HANDLE deleteOldLogFileThreadHandle_;
  bool deleteOldLogFileThreadStopFlag_;
  
  struct {
    celog_level_t       logLevel;
    bool                debugMode;
    bool                perfLogMode;
    unsigned long       logFileSize; // log file setting from register
    unsigned long       logFileNum;  // number of log files setting from register
    unsigned long       logFileKeptDays; // number of days to keep the log file
    unsigned long       logEnabled; // log enable setting from register
  } config_;
};

#endif /* __CELOG_MGR_H__ */
