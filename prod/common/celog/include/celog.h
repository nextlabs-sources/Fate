/**********************************************************************
 *
 * Compliant Enterprise Logging
 *
 * This interface describes a logging mechanism.
 *
 * There is both a logging class (CELog) and an instance of that class
 * in the form of a singletone (CELogS) for users to choose from.
 * Linkage for C programs is also supported.
 *
 * Log Interfaces
 *   C       celog() function
 *   C++     CELog class
 *   Common  TRACE() macro prototype is common for both C and C++
 *
 * The definition of UNICODE controls the behavior of the TRACE macro.
 * By default UNICODE is not defined.  There are ANSI and wide macros
 * provided regardless of UNICODE definition: TRACEA(), TRACEW().
 *
 * Behavior:
 *   UNICODE defined  : TRACE uses wchar* format and strings.
 *   UNICODE undefined: TRACE uses char* format and strings.
 *
 *********************************************************************/

#ifndef __CELOG_H__
#define __CELOG_H__

#if defined(_WIN32) || defined (_WIN64)
  #include <winsock2.h> // timeval
  #include <windows.h>
#else
  #include <pthread.h>
  #include "linux_win.h"	
#endif

#include <stdarg.h>
#include <cassert>
#include <wchar.h>
#include <time.h>
#include <sys/types.h>
#include <sys/timeb.h>

/* Support for exports */
#if defined(_WIN32) || defined(_WIN64)
  #define CELOG_EXPORT __declspec(dllexport) 
#else
  #define CELOG_EXPORT /* empty */
#endif

/** CELOG_MAX_MESSAGE_SIZE_CHARS
 *
 *  Maximum message size in characters of wide chars.
 */
#define CELOG_MAX_MESSAGE_SIZE_CHARS 2048

/** CELOG_MAX_MESSAGE_SIZE_BYTES
 *
 *  Maximum message size in bytes.
 */
#if defined(WIN32) || defined(_WIN64)
#define CELOG_MAX_MESSAGE_SIZE_BYTES (CELOG_MAX_MESSAGE_SIZE_CHARS * sizeof(WCHAR))
#else
#define CELOG_MAX_MESSAGE_SIZE_BYTES (CELOG_MAX_MESSAGE_SIZE_CHARS * sizeof(char))
#endif


/** CELOG Level
 *
 *  \brief Log level to match syslog.  See 'man syslog' for details.
 */
enum
{
  CELOG_EMERG     = 0 ,  /** Emergency */
  CELOG_ALERT     = 1 ,  /** Alert */
  CELOG_CRIT      = 2 ,  /** Critical */
  CELOG_ERR       = 3 ,  /** Error */
  CELOG_WARNING   = 4 ,  /** Warning */
  CELOG_NOTICE    = 5 ,  /** Notice */
  CELOG_INFO      = 6 ,  /** Info */
  CELOG_DEBUG     = 7 ,  /** Debug */
  CELOG_MAX              /** maximum marker */
};

#ifdef __cplusplus

#include <vector>

/** CELogPolicy
 *
 *  \brief Abstract base class for Policies of CELog class.  Derive
 *         from this class to define custom policies.  Derived classes
 *         must implement Log().
 *
 *  \notes The Log method must not throw any exception.
 */
class CELogPolicy
{
 public:
  virtual ~CELogPolicy(void) {}

  /** Log
   *
   *  \brief Derived classes must implement this method.
   *
   *  \param string (in) Log message.
   *
   *  \return Number of characters written to the log.
   */
  virtual int Log( const wchar_t* string ) = 0;
  virtual int Log( const char* string ) = 0;
};/* CELogPolicy */

/** CELog
 *
 *  \brief Class to provide logging facilities.
 */
class CELog
{
  public:
    CELog(void) :
       is_enabled(true),     /* enabled by default */
       level(CELOG_EMERG),   /* Default to emergency */
       policies()            /* Log policies */
     {
       /* Construct mutex object */
#if defined(_WIN32) || defined(_WIN64)
       InitializeCriticalSection(&mutex);
#else
       /* Default attributes imply non-recursive mutex. */
       pthread_mutex_init(&mutex,NULL);
#endif
     }/* CELog */

     ~CELog(void) throw()
     {
       std::vector<CELogPolicy*>::iterator it;
       this->Lock();
       for( it = policies.begin() ; it != policies.end() ; ++it )
       {
	 delete (*it);
       }
       this->Unlock();
       /* Destroy mutex object */
#if defined(_WIN32) || defined(_WIN64)
       DeleteCriticalSection(&mutex);
#else
       pthread_mutex_destroy(&mutex);
#endif
     }/* ~CELog */

     static const int DEFAULT_LEVEL = CELOG_INFO;

     /** Log
     *
     *  \brief Log a message.
     *
     *  \param lvl (in)  Level.
     *  \param file (in) File name of current source.
     *  \param line (in) Line of current source file.
     *  \param fmt (in)  Format.  See printf() man page.
     *
     *  \return Number of characters written to the log.  Negative number on failure.
     *  \note MT-safe.
     */
     int Log( int lvl,
	      const char* file,
	      int line,
	      const char* fmt,
	      ... ) throw()
     {
       assert( fmt != NULL );
       if( fmt == NULL )
       {
	 return -1; /* fail */
       }

       /* If log level is higher than log is set for, or if logging
	  is diabled - do nothing.
       */
       if( lvl > this->level || is_enabled == false )
       {
	 return -1;
       }

       char sline[CELOG_MAX_MESSAGE_SIZE_CHARS];
       char* p = sline;            /* cursor for log write */

#if defined (Linux)
       struct _timeb timebuffer;
       char timeline[32] = {0};
       _ftime64_s(&timebuffer);
       char* sTmpTime = ctime(&timebuffer.time);
       if (sTmpTime)
           strncpy_s(timeline, sizeof(timeline), sTmpTime, _TRUNCATE);
       if (sTmpTime)
       {
           timeline[strlen(timeline)-1] = (char)NULL;  /* remove newline */
           p += _snprintf_s(p, _countof(sline) - (p-sline), _TRUNCATE, "%.19s.%03hu %s: ",timeline,timebuffer.millitm,&timeline[20]);
       }

       /* log level in string form */
       p += _snprintf_s(p, _countof(sline) - (p-sline), _TRUNCATE, "%s ", LevelToString(lvl));
#endif

       /* File may be NULL to indicate there is not file/line information */
       if( file != NULL )
       {
		 p += _snprintf_s(p, _countof(sline) - (p-sline), _TRUNCATE, "%s [%d]: ", file, line);
       }

       va_list args;
       va_start(args,fmt);
       p += vsnprintf_s(p,_countof(sline) - (p-sline),_TRUNCATE,fmt,args);
       va_end(args);

#if defined(_WIN32) || defined (_WIN64)
       wchar_t wsline[CELOG_MAX_MESSAGE_SIZE_CHARS];
       size_t nchars = 0;
       errno_t err_val = mbstowcs_s(&nchars,wsline,_countof(wsline),sline,_TRUNCATE);
       if( err_val != 0 )
       {
           return -1; /* fail */
       }
       wsline[CELOG_MAX_MESSAGE_SIZE_CHARS - 1] = L'\0';

       /* Reduce to ::Log for 'wchar*' */
       return Log(lvl, L"%s", wsline);
#else
       int nchars = 0;

       /* iterate over polices for output */
       std::vector<CELogPolicy*>::iterator it;
       this->Lock();
       for( it = policies.begin() ; it != policies.end() ; ++it )
       {
	 nchars = (*it)->Log(sline);
       }
       this->Unlock();
       return nchars;
#endif
     }/* Log */

	  int Log( int lvl, const char* file, int line, const wchar_t* fmt, va_list argptr) throw()
	  {
		  assert( fmt != NULL );

		  /* If log level is higher than log is set for, or if logging
		  is diabled - do nothing.
		  */
		  if( lvl > this->level || is_enabled == false )
		  {
			  return -1;
		  }

		  wchar_t sline[CELOG_MAX_MESSAGE_SIZE_CHARS];
		  wchar_t* p = sline;            /* cursor for log write */

		  /* Add timestamp to log message */
		  struct _timeb timebuffer;
		  char timeline[32] = {0};
		  _ftime64_s(&timebuffer);
#if defined(_WIN32) || defined (_WIN64)
		  if( ctime_s(timeline,_countof(timeline),&timebuffer.time) == 0 )
#else
		  char* sTmpTime = ctime(&timebuffer.time);
		  if (sTmpTime)
			  strncpy_s(timeline, sizeof(timeline), sTmpTime, _TRUNCATE);
		  if (sTmpTime)
#endif
		  {
			  timeline[strlen(timeline)-1] = (char)NULL;  /* remove newline */
			  p += _snwprintf_s(p,_countof(sline), _TRUNCATE, L"%.19hs.%03hu %hs: ",timeline,timebuffer.millitm,&timeline[20]);
		  }

		  /* log level in string form */
		  p += _snwprintf_s(p, _countof(sline) - (p-sline), _TRUNCATE, L"%s ", LevelToStringW(lvl));

		  /* File may be NULL to indicate there is not file/line information */
		  if( file != NULL )
		  {
			  p += _snwprintf_s(p, _countof(sline) - (p-sline), _TRUNCATE, L"%hs [%d]: ", file, line);
		  }

		  // Due to enough buffer size for the prefixed timestamp and logging level string, _snwprintf_s always works well(not return -1).
		  // Here #argptr is unknown, so if truncated, it returns -1 and has a terminating null because using _TRUNCATE ensures this point.
		  int nbufsize = _countof(sline) - (p-sline);
		  // https://msdn.microsoft.com/en-us/library/d3xd30zz.aspx
		  //#sizeOfBuffer The size of the buffer for output, as the character count.
		  //#count Maximum number of characters to write (not including the terminating null), or _TRUNCATE.
		  //  vsnprintf_s,_vsnprintf_s and _vsnwprintf_s return the number of characters written, not including the terminating null, or a 
		  //negative value if an output error occurs. vsnprintf_s is identical to _vsnprintf_s. vsnprintf_s is included for compliance to 
		  //the ANSI standard. _vnsprintf is retained for backward compatibility.
		  //  Each of these functions takes a pointer to an argument list, then formats and writes up to count characters of the given data 
		  //to the memory pointed to by buffer and appends a terminating null.
		  //  If count is _TRUNCATE, then these functions write as much of the string as will fit in buffer while leaving room for a 
		  //terminating null. If the entire string (with terminating null) fits in buffer, then these functions return the number of 
		  //characters written (not including the terminating null); otherwise, these functions return -1 to indicate that truncation occurred.
		  int nchars = _vsnwprintf_s(p, nbufsize, nbufsize - 1, fmt, argptr);

		  // Ensure the last but not least character is a `\n`(0xa), a line feed symbol
		  if( -1 == nchars) // 
		  {
			  p = sline + _countof(sline) - 2;
			  *p = '\n';
		  }else if(nchars > 0)
		  {
			  // |<--------- nbufsize ------>
			  // |<--- nchars  --->| ? counts the terminating null
			  // |<- nchars ->| ? not counts the terminating null
			  // | ... ... ...|'\0'|....|....
			  // 
			  // | ... ... ...|'\n'|'\0'|....
			  if ('\n' != p[nchars - 1])
			  {
				  if((nchars + 1) < nbufsize)
				  {
					  p[nchars+1] = '\0';
					  p[nchars] = '\n';
				  }else
				  {
					  p[nchars - 1] = '\n';
				  }
			  }
		  }else
		  {
			  p[0] = '\n';
			  p[1] = '\0';
		  }

		  nchars = 0;
		  /* iterate over polices for output */
		  std::vector<CELogPolicy*>::iterator it;
		  this->Lock();
		  for( it = policies.begin() ; it != policies.end() ; ++it )
		  {
			  nchars = (*it)->Log(sline);
		  }
		  this->Unlock();
		  return nchars;
	  }

	  int Log( int lvl,
		  const char* file,
		  int line,
		  const wchar_t* fmt ,
		  ... ) throw()
	  {
		  assert( fmt != NULL );
		  va_list args;
		  va_start(args,fmt);
		  int nchars = Log(lvl, file, line, fmt, args);
		  va_end(args);
		  return nchars;
	  }/* Log */

     /** Wrappers for Log() with {file,line} support.
      */
     int Log( int lvl , const char* fmt , ... ) throw()
     {
       va_list args;
       va_start(args,fmt);
       char sline[CELOG_MAX_MESSAGE_SIZE_CHARS];
       vsnprintf_s(sline,sizeof(sline),_TRUNCATE,fmt,args);
       va_end(args);
       return Log(lvl,NULL,0,"%s",sline);
     }/* Log */

     int Log( int lvl , const wchar_t* fmt , ... ) throw()
     {
       va_list args;
       va_start(args,fmt);
       wchar_t sline[CELOG_MAX_MESSAGE_SIZE_CHARS];
       vswprintf(sline,_countof(sline),fmt,args);
       va_end(args);
       return Log(lvl,NULL,0,L"%s",sline);
     }/* Log */

     /** IsEnabled
      *
      *  \brief Determine if logging is enabled.
      *
      *  \return Status of logging.  True when the log is enabled, otherwise
      *          false.
      *  \sa Enable.
      */
     bool IsEnabled(void) const throw()
     {
       return is_enabled;
     }/* IsEnabled */

     /** Enable
      *
      *  \brief Enable logging.
      *
      *  \return None.
      *  \sa Disable, IsEnabled.
      */
     void Enable(void) throw()
     {
       is_enabled = true;
     }/* Enable */

     /** Disable
      *
      *  \brief Diable logging.
      *  \return None.
      *  \sa Enable
      */
     void Disable(void) throw()
     {
       is_enabled = false;
     }/* Disable */

     /** Level
      *
      *  \brief Determine the current logging threshold level.  The default
      *         threshold is CELOG_EMERG unless set with SetLevel.
      *
      *  \return Current logging threshold level.
      *  \sa SetLevel.
      */
     int Level(void) const throw()
     {
       return this->level;
     }/* Level */

     /** SetLevel
      *
      *  \brief Set the logging threshold level.  The default threshold is
      *         CELOG_EMERG unless set with SetLevel.
      *
      *  \param level (in) Logging threshold level.
      *
      *  \return Logging level before set.  Return -1 on error.
      *  \sa Level.
      */
     int SetLevel( int level ) throw()
     {
       assert( level >= CELOG_EMERG && level < CELOG_MAX );
       if( level < CELOG_EMERG || level >= CELOG_MAX )
       {
	 return -1;
       }

       /* Keep current level for return value */
       int current_level = this->level;
       this->level = level;
       return current_level;
     }/* SetLevel */

     /** SetLogPolicy
      *
      *  \brief Install a policy for logging.  After a call to SetPolicy
      *         the CELog instance owns the policy instance and will free
      *         it when the CELog instance is destructed.
      *
      *  \param policy (in) Set use of stderr logging.
      *
      *  \return None.
      */
     void SetPolicy( CELogPolicy* policy ) throw()
     {
       assert( policy != NULL );
       this->Lock();
       this->policies.push_back(policy);
       this->Unlock();
     }

  private:

     /* LevelToStringW
      *
      * Convert a level to its string meaning.
      */
     static const wchar_t* LevelToStringW( int level )
     {
       if( level < 0 || level >= CELOG_MAX )
       {
	 return L"UNKNOWN";
       }
       static const wchar_t* table[] =
	 {
	   L"EMERGENCY",  // CELOG_EMERG
	   L"ALERT",      // CELOG_ALERT
	   L"CRITICAL",   // CELOG_CRIT
	   L"ERROR",      // CELOG_ERR
	   L"WARNING",    // CELOG_WARNING
	   L"NOTICE",     // CELOG_NOTICE
	   L"INFO",       // CELOG_INFO
	   L"DEBUG"       // CELOG_DEBUG
	 };
       return table[level];
     }/* LevelToStringW */

     static const char* LevelToString( int level )
     {
       if( level < 0 || level >= CELOG_MAX )
       {
	 return "UNKNOWN";
       }
       static const char* table[] =
	 {
	   "EMERGENCY",  // CELOG_EMERG
	   "ALERT",      // CELOG_ALERT
	   "CRITICAL",   // CELOG_CRIT
	   "ERROR",      // CELOG_ERR
	   "WARNING",    // CELOG_WARNING
	   "NOTICE",     // CELOG_NOTICE
	   "INFO",       // CELOG_INFO
	   "DEBUG"       // CELOG_DEBUG
	 };
       return table[level];
     }/* LevelToStringW */

    /* CELog is a noncopyable object.  Constructors and assignments are
       prototyped.  They are not defined to prevent linking when copy is
       attempted.
    */
    CELog( const CELog& a );
    CELog& operator= (const CELog& log);

    /** Lock
     *
     *  \brief Lock CELog object instance.
     *  \return None.
     *  \sa Unlock.
     */
    void Lock(void) throw()
    {
#if defined(_WIN32) || defined(_WIN64)
      EnterCriticalSection(&mutex);
#else
      pthread_mutex_lock(&mutex);
#endif
    }

    /** Unlock
     *
     *  \brief Unlock CELog object instance.
     *  \return None.
     *  \sa Lock.
     */
    void Unlock(void) throw()
    {
#if defined(_WIN32) || defined(_WIN64)
      LeaveCriticalSection(&mutex);
#else
      pthread_mutex_unlock(&mutex);
#endif
    }/* Unlock */

    /** Mutex object used by Lock/Unlock */
#if defined(_WIN32) || defined(_WIN64)
    CRITICAL_SECTION mutex;
#else
    pthread_mutex_t mutex;
#endif

    bool is_enabled;     /** Log is enabled? */
    int level;           /** Log threshold level CELOG_XXX */
    std::vector<CELogPolicy*> policies; /** Policies */

};/* CELog */

/** CELogS
 *
 *  \brief CELog singleton.  Wraps CELog to provide singleton for TRACE macros.
 */
class CELogS
{
  public:

    /** Instance
     *
     *  \brief Return instance of singleton.  It may be created first.
     */
    CELOG_EXPORT static CELog* Instance(void);

    private:

    /* CELogS is a noncopyable object.  Constructors are prototyped
       but not defined to prevent linking when copy is attempted.
    */
    CELogS(void);
    CELogS( const CELogS& a );

    static CELog* log;

};/* CELogS */

#endif /* __cplusplus */

/** TRACE,TRACEA,TRACEW
 *
 *  \brief Macros for logging in C/C++.
 */

#undef TRACE
#undef TRACEA
#undef TRACEW

#ifdef __cplusplus
  /* C++ will handle wchar/char selection via overloading. */
  #define TRACE(lvl,fmt,...)         CELogS::Instance()->Log(lvl,__FILE__,__LINE__,fmt,##__VA_ARGS__)
  #define TRACEA(lvl,fmt,...)        CELogS::Instance()->Log(lvl,__FILE__,__LINE__,fmt,##__VA_ARGS__)
  #define TRACEW(lvl,fmt,...)        CELogS::Instance()->Log(lvl,__FILE__,__LINE__,fmt,##__VA_ARGS__)
#else
  /* C must specify whether wchar/char string are used by UNICODE definition. */
  #ifdef UNICODE
    #define TRACE(lvl,fmt,...) celogW(lvl,__FILE__,__LINE__,fmt,##__VA_ARGS__)
  #else
    #define TRACE(lvl,fmt,...) celog(lvl,__FILE__,__LINE__,fmt,##__VA_ARGS__)
  #endif
  #define TRACEA(lvl,fmt,...)  celog(lvl,__FILE__,__LINE__,fmt,##__VA_ARGS__)
  #define TRACEW(lvl,fmt,...)  celogW(lvl,__FILE__,__LINE__,fmt,##__VA_ARGS__)
#endif /* __cplusplus */

/** TRACE_ASSERT
 *
 *  \brief When the condition is false a log entry at level CELOG_EMERG is
 *         produced and assert() called.
 */
#define TRACE_ASSERT(condition,fmt,...) ( (condition) == 0 ? TRACE(CELOG_EMERG,fmt,__VA__ARGS__) , assert(condition) : ; )

/*************************************************************************************
 * C linkage support
 ************************************************************************************/

/** celog
 *
 *  \brief Logging with C linkage.
 *
 *  \param lvl (in)  Log level.
 *  \param file (in) File.
 *  \param line (in) Line of file.
 *  \param fmt (in)  Format.
 *
 *  \return Number of characters written to the log.
 */
extern "C" CELOG_EXPORT int  celog( int lvl,
				    const char* file,
				    int line,
				    const char* fmt,
				    ... );

/** celogW
 *
 *  \brief Logging with C linkage for wide character.
 *
 *  \param lvl (in)  Log level.
 *  \param file (in) File.
 *  \param line (in) Line of file.
 *  \param fmt (in)  Format in wide char format.
 *
 *  \return Number of characters written to the log.
 */
extern "C" CELOG_EXPORT int  celogW( int lvl,
				     const char* file,
				     int line,
				     const wchar_t* fmt,
				     ... );

/* See methods of CELog.  These are C linkage wrappers for CELog. */
extern "C" CELOG_EXPORT int  celog_IsEnabled(void);
extern "C" CELOG_EXPORT int  celog_Level(void);
extern "C" CELOG_EXPORT int  celog_SetLevel( int level );
extern "C" CELOG_EXPORT void celog_Enable(void);
extern "C" CELOG_EXPORT void celog_Disable(void);
extern "C" CELOG_EXPORT void celog_SetPolicy( int (*policy)(const wchar_t*) );

#endif /* __CELOG_H__ */
