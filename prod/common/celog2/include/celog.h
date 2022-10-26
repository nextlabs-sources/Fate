/*************************************************************************
 *
 * Compliant Enterprise Logging
 *
 * Declarations for external logging interface.
 *
 * The library code is designed to satisfy the following mandates:
 * - No type-casting should be used in the library code.
 * - No type-casting should be required on the library users' code.
 * - No static warning should be caused by the library code.
 *
 ************************************************************************/

#ifndef __CELOG_H__
#define __CELOG_H__



#include <sstream>
#include "eframework/timer/timer_high_resolution.hpp"
#include "celog_filepathdefs.h"

#define CELOG_MAX_LINE_LEN      1024

typedef enum
{
  CELOG_CRITICAL        = 0,
  CELOG_ERROR           = 1,
  CELOG_WARNING         = 2,
  CELOG_INFO            = 3,
  CELOG_TRACE           = 4,
  CELOG_DEBUG           = 5,
  CELOG_DUMP            = 6
} celog_level_t;

_Check_return_
bool CELog_IsPerfLogMode(void);

 /** CELog_Log
 *
 *  \brief Log a message.
 *
 *  \param fileInt (in) Integer representing the path of source file.
 *  \param line (in)    Line number in source file.
 *  \param mod (in)     Module name.
 *  \param level (in)   Log level.
 *  \param fmt (in)     Format.  See printf() man page.
 *  \param ... (in)     Arguments to be formatted.
 *
 *  \return Number of characters written to the log.  Negative number on failure.
 *  \note MT-safe.  If message is too long, it is truncate and printed, and then -1 is returned.
 */
int CELog_LogA(_In_     celog_filepathint_t fileInt,
               _In_     int line,
               _In_z_   const wchar_t *mod,
               _In_     celog_level_t level,
               _In_z_ _Printf_format_string_ const char *fmt,
               _In_      ...);

int CELog_LogW(_In_     celog_filepathint_t fileInt,
               _In_     int line,
               _In_z_   const wchar_t *mod,
               _In_     celog_level_t level,
               _In_z_ _Printf_format_string_ const wchar_t *fmt,
               _In_     ...);

#ifdef UNICODE
#define CELog_Log       CELog_LogW
#else
#define CELog_Log       CELog_LogA
#endif /* UNICODE */

 /** CELOG_LOG
 *
 *  \brief Log a message.
 *
 *  \param level (in)   Log level.
 *  \param fmt (in)     Format.  See printf() man page.
 *  \param ... (in)     Arguments to be formatted.
 *
 *  \return Number of characters written to the log.  Negative number on failure.
 *  \note MT-safe.  If message is too long, it is truncate and printed, and then -1 is returned.
 */
#define CELOG_LOGA(level, fmt, ...)                                     \
    CELog_LogA(CELOG_CUR_FILE, __LINE__, CELOG_CUR_MODULE, level, fmt,  \
              ##__VA_ARGS__)

#define CELOG_LOGW(level, fmt, ...)                                     \
    CELog_LogW(CELOG_CUR_FILE, __LINE__, CELOG_CUR_MODULE, level, fmt,  \
              ##__VA_ARGS__)

#ifdef UNICODE
#define CELOG_LOG       CELOG_LOGW
#else
#define CELOG_LOG       CELOG_LOGA
#endif /* UNICODE */



namespace CELog
{
  //
  // Internal classes.  These classes should not be used externally.
  //
  // The class hierarchy is the following:
  //
  // LogHelper --------------------------------------------> ExceptionLogHelper
  //     |                                                           |
  //     |                                                           |
  //     +----> FuncInvoke -------> FuncInvokeWithException <--------+
  //     |                                                           |
  //     |                                                           |
  //     +----> FuncInvokeEnum ---> FuncInvokeEnumWithException <----+
  //     |                                                           |
  //     |                                                           |
  //     +----> FuncInvokePtr ----> FuncInvokePtrWithException <-----+

  //
  // LogHelper class
  //
  // This class implements the following:
  // - recording of context information (e.g. file, module, function)
  // - Enter log
  // - performance log
  //
  class LogHelper
  {
  public:
    LogHelper(_In_      celog_filepathint_t fileInt,
              _In_      int line,
              _In_z_    const wchar_t *mod,
              _In_z_    const wchar_t *fn) :
      fileInt_(fileInt),
      mod_(mod),
      fn_(fn)
    {
      CELog_Log(fileInt, line, mod, CELOG_TRACE, L"%s Enter\n", fn);
    }

  private:
    // Since this class has const member data, we declare an assignment
    // operator in order to avoid C4512 "assignment operator could not be
    // generated" warning.
    LogHelper& operator= (const LogHelper&);

  protected:
    _Check_return_
    static celog_level_t GetLeaveMsgLevel(void)
    {
      return CELog_IsPerfLogMode() ? CELOG_INFO : CELOG_TRACE;
    }

    _Check_return_
    std::wstring GetLeaveMsgElapsedTimeStr(void)
    {
      if (CELog_IsPerfLogMode())
      {
        std::wstringstream s;

        timer_.stop();
        s << L", elapsed time = "
          << (unsigned long long) (timer_.diff() * 1000 + 0.5)
          << L"us";

        return s.str();
      }
      else
      {
        return std::wstring();
      }
    }

  protected:
    const celog_filepathint_t fileInt_;

    // Cannot use std::wstring here since this class cannot have a destructor.
    const wchar_t * const mod_;
    const wchar_t * const fn_;

    nextlabs::high_resolution_timer timer_;
  }; /* LogHelper */

  //
  // ExceptionLogHelper class
  //
  // This class implements the following:
  // - Leave log upon C++ exception
  //
  class ExceptionLogHelper : virtual LogHelper
  {
  public:
    ExceptionLogHelper(_In_     celog_filepathint_t fileInt,
                       _In_     int line,
                       _In_z_   const wchar_t *mod,
                       _In_z_   const wchar_t *fn) :
      LogHelper(fileInt, line, mod, fn),
      retOccurred_(false)
    {
    }

  protected:
    ~ExceptionLogHelper()
    {
      if (!retOccurred_)
      {
        CELog_Log(fileInt_, 0,
                  // mod_ can't actually be NULL here, but we need the
                  // conditional here just to avoid warning C6387.
                  mod_ == NULL ? L"" : mod_,
                  GetLeaveMsgLevel(),
                  L"%s Leave (C++ exception occurred)%s\n", fn_,
                  GetLeaveMsgElapsedTimeStr().c_str());
      }
    }

  public:
    void RetOccurred(void)
    {
      retOccurred_ = true;
    }

  private:
    bool retOccurred_;
  }; /* ExceptionLogHelper */

  //
  // FuncInvoke class
  //
  // This class implements the following:
  // - Leave log upon normal return
  // - support for functions that handle SEH.
  // - support for functions returning:
  //   - basic type values
  //   - strings and string objects.
  //   - void pointers
  //
  class FuncInvoke : virtual public LogHelper
  {
  public:
    FuncInvoke(_In_     celog_filepathint_t fileInt,
               _In_     int line,
               _In_z_   const wchar_t *mod,
               _In_z_   const wchar_t *fn) :
      LogHelper(fileInt, line, mod, fn)
    {
    }

    void Ret(_In_ int line)
    {
      CELog_Log(fileInt_, line, mod_, GetLeaveMsgLevel(), L"%s Leave%s\n", fn_,
                GetLeaveMsgElapsedTimeStr().c_str());
    }

    #define DEF_FI_RET_SAL(sal, type, fmt, printValExp)                 \
      _Check_return_                                                    \
      type Ret(_In_ int line, sal type val)                             \
      {                                                                 \
        CELog_Log(fileInt_, line, mod_, GetLeaveMsgLevel(),             \
                  L"%s Leave, returning " fmt L"%s\n", fn_,             \
                  printValExp,                                          \
                  GetLeaveMsgElapsedTimeStr().c_str());                 \
        return val;                                                     \
      }

    #define DEF_FI_RET2_SAL(sal, type, fmt, printValExp)                \
      _Check_return_                                                    \
      type Ret(_In_ int line, sal type val)                             \
      {                                                                 \
        CELog_Log(fileInt_, line, mod_, GetLeaveMsgLevel(),             \
                  L"%s Leave, returning " fmt L"%s\n", fn_,             \
                  sizeof(type) * 2, printValExp,                        \
                  GetLeaveMsgElapsedTimeStr().c_str());                 \
        return val;                                                     \
      }

    #define DEF_FI_RET(type, fmt, printValExp)                          \
        DEF_FI_RET_SAL(_In_, type, fmt, printValExp)
    #define DEF_FI_RET_OPT(type, fmt, printValExp)                      \
        DEF_FI_RET_SAL(_In_opt_, type, fmt, printValExp)
    #define DEF_FI_RET_OPTZ(type, fmt, printValExp)                     \
        DEF_FI_RET_SAL(_In_opt_z_, type, fmt, printValExp)
    #define DEF_FI_RET2(type, fmt, printValExp)                         \
        DEF_FI_RET2_SAL(_In_, type, fmt, printValExp)
    #define DEF_FI_RET2_OPT(type, fmt, printValExp)                     \
        DEF_FI_RET2_SAL(_In_opt_, type, fmt, printValExp)
    #define DEF_FI_RET2_OPTZ(type, fmt, printValExp)                    \
        DEF_FI_RET2_SAL(_In_opt_z_, type, fmt, printValExp)

    DEF_FI_RET     (bool,               L"%s",          val?L"true":L"false");
    DEF_FI_RET     (short,              L"%hd",         val);
    DEF_FI_RET     (int,                L"%d",          val);
    DEF_FI_RET     (long,               L"%ld",         val);
    DEF_FI_RET     (long long,          L"%lld",        val);
    DEF_FI_RET2    (unsigned short,     L"0x%0*hX",     val);
    DEF_FI_RET2    (unsigned int,       L"0x%0*X",      val);
    DEF_FI_RET2    (unsigned long,      L"0x%0*lX",     val);
    DEF_FI_RET2    (unsigned long long, L"0x%0*llX",    val);
    DEF_FI_RET_OPTZ(char *,             L"\"%hs\"",     val);
    DEF_FI_RET_OPTZ(wchar_t *,          L"\"%ls\"",     val);
    DEF_FI_RET     (std::string,        L"\"%hs\"",     val.c_str());
    DEF_FI_RET     (std::wstring,       L"\"%ls\"",     val.c_str());
    DEF_FI_RET_OPT (void *,             L"0x%p",        val);
  }; /* FuncInvoke */

  //
  // FuncInvokeWithException class
  //
  // This class implements the following:
  // - Leave log upon normal return
  // - support for functions that don't handle exceptions or handle C++
  //   exceptions.
  // - support for functions returning:
  //   - basic type values
  //   - strings and string objects.
  //   - void pointers
  //
  class FuncInvokeWithException : public FuncInvoke, ExceptionLogHelper
  {
  public:
    FuncInvokeWithException(_In_        celog_filepathint_t fileInt,
                            _In_        int line,
                            _In_z_      const wchar_t *mod,
                            _In_z_      const wchar_t *fn) :
      LogHelper(fileInt, line, mod, fn),
      FuncInvoke(fileInt, line, mod, fn),
      ExceptionLogHelper(fileInt, line, mod, fn)
    {
    }

    void Ret(_In_ int line)
    {
      RetOccurred();
      return FuncInvoke::Ret(line);
    }

    #define DEF_FIWE_RET_SAL(sal, type)                                 \
      _Check_return_                                                    \
      type Ret(_In_ int line, sal type val)                             \
      {                                                                 \
        RetOccurred();                                                  \
        return FuncInvoke::Ret(line, val);                              \
      }

    #define DEF_FIWE_RET(type)          DEF_FIWE_RET_SAL(_In_, type)
    #define DEF_FIWE_RET_OPT(type)      DEF_FIWE_RET_SAL(_In_opt_, type)
    #define DEF_FIWE_RET_OPTZ(type)     DEF_FIWE_RET_SAL(_In_opt_z_, type)

    DEF_FIWE_RET     (bool);
    DEF_FIWE_RET     (short);
    DEF_FIWE_RET     (int);
    DEF_FIWE_RET     (long);
    DEF_FIWE_RET     (long long);
    DEF_FIWE_RET     (unsigned short);
    DEF_FIWE_RET     (unsigned int);
    DEF_FIWE_RET     (unsigned long);
    DEF_FIWE_RET     (unsigned long long);
    DEF_FIWE_RET_OPTZ(char *);
    DEF_FIWE_RET_OPTZ(wchar_t *);
    DEF_FIWE_RET     (std::string);
    DEF_FIWE_RET     (std::wstring);
    DEF_FIWE_RET_OPT (void *);
  }; /* FuncInvokeWithException */

  //
  // FuncInvokeEnum class
  //
  // This class implements the following:
  // - Leave log upon normal return
  // - support for functions that handle SEH.
  // - support for functions returning:
  //   - enum values
  //
  template <typename T>
  class FuncInvokeEnum : virtual public LogHelper
  {
  public:
    FuncInvokeEnum(_In_         celog_filepathint_t fileInt,
                   _In_         int line,
                   _In_z_       const wchar_t *mod,
                   _In_z_       const wchar_t *fn) :
      LogHelper(fileInt, line, mod, fn)
    {
    }

    _Check_return_
    T Ret(_In_ int line, _In_ T val)
    {
      CELog_Log(fileInt_, line, mod_, GetLeaveMsgLevel(),
                L"%s Leave, returning %d%s\n", fn_, val,
                GetLeaveMsgElapsedTimeStr().c_str());
      return val;
    }
  }; /* FuncInvokeEnum */

  //
  // FuncInvokeEnumWithException class
  //
  // This class implements the following:
  // - Leave log upon normal return
  // - support for functions that don't handle exceptions or handle C++
  //   exceptions.
  // - support for functions returning:
  //   - enum values
  //
  template <typename T>
  class FuncInvokeEnumWithException : public FuncInvokeEnum<T>,
           ExceptionLogHelper
  {
  public:
    FuncInvokeEnumWithException(_In_    celog_filepathint_t fileInt,
                                _In_    int line,
                                _In_z_  const wchar_t *mod,
                                _In_z_  const wchar_t *fn) :
      LogHelper(fileInt, line, mod, fn),
      FuncInvokeEnum(fileInt, line, mod, fn),
      ExceptionLogHelper(fileInt, line, mod, fn)
    {
    }

    _Check_return_
    T Ret(_In_ int line, _In_ T val)
    {
      RetOccurred();
      return FuncInvokeEnum::Ret(line, val);
    }
  }; /* FuncInvokeEnumWithException */

  //
  // FuncInvokePtr class
  //
  // This class implements the following:
  // - Leave log upon normal return
  // - support for functions that handle SEH.
  // - support for functions returning:
  //   - pointers to non-void types
  //
  template <typename T>
  class FuncInvokePtr : virtual public LogHelper
  {
  public:
    FuncInvokePtr(_In_          celog_filepathint_t fileInt,
                  _In_          int line,
                  _In_z_        const wchar_t *mod,
                  _In_z_        const wchar_t *fn) :
      LogHelper(fileInt, line, mod, fn)
    {
    }

    _Check_return_
    T Ret(_In_ int line, _In_opt_ T val)
    {
      CELog_Log(fileInt_, line, mod_, GetLeaveMsgLevel(),
                L"%s Leave, returning %p%s\n", fn_, val,
                GetLeaveMsgElapsedTimeStr().c_str());
      return val;
    }
  }; /* FuncInvokePtr */

  //
  // FuncInvokePtrWithException class
  //
  // This class implements the following:
  // - Leave log upon normal return
  // - support for functions that don't handle exceptions or handle C++
  //   exceptions.
  // - support for functions returning:
  //   - pointers to non-void types
  //
  template <typename T>
  class FuncInvokePtrWithException : public FuncInvokePtr<T>,
           ExceptionLogHelper
  {
  public:
    FuncInvokePtrWithException(_In_     celog_filepathint_t fileInt,
                               _In_     int line,
                               _In_z_   const wchar_t *mod,
                               _In_z_   const wchar_t *fn) :
      LogHelper(fileInt, line, mod, fn),
      FuncInvokePtr(fileInt, line, mod, fn),
      ExceptionLogHelper(fileInt, line, mod, fn)
    {
    }

    _Check_return_
    T Ret(_In_ int line, _In_opt_ T val)
    {
      RetOccurred();
      return FuncInvokePtr::Ret(line, val);
    }
  }; /* FuncInvokePtrWithException */
}



//
// External macros for function entering and leaving.  These should be used
// for:
// - functions that do not handle any exceptions.
// - functions that handle C++ exceptions.
//

// Entering
#define CELOG_ENTER                                                     \
  CELog::FuncInvokeWithException fiwe(CELOG_CUR_FILE, __LINE__,         \
  CELOG_CUR_MODULE, TEXT(__FUNCTION__))

// Returning void
#define CELOG_RETURN                                                    \
  fiwe.Ret(__LINE__); return

// Returning void expression
#define CELOG_RETURN_VOIDEXP(exp)                                       \
  (exp); return fiwe.Ret(__LINE__)

// Returning a value whose type is supported
#define CELOG_RETURN_VAL(val)                                           \
  return fiwe.Ret(__LINE__, (val))

// Returning a value whose type is not supported
#define CELOG_RETURN_VAL_NOPRINT(val)                                   \
  fiwe.Ret(__LINE__); return (val)

// Entering (for functions returning an enum value)
#define CELOG_ENUM_ENTER(type)                                          \
  CELog::FuncInvokeEnumWithException<type> fiewe(CELOG_CUR_FILE,        \
  __LINE__, CELOG_CUR_MODULE, TEXT(__FUNCTION__))

// Returning an enum value
#define CELOG_ENUM_RETURN_VAL(val)                                      \
  return fiewe.Ret(__LINE__, (val))

// Entering (for functions returning a pointer to non-void type)
#define CELOG_PTR_ENTER(type)                                           \
  CELog::FuncInvokePtrWithException<type> fipwe(CELOG_CUR_FILE,         \
  __LINE__, CELOG_CUR_MODULE, TEXT(__FUNCTION__))

// Returning a pointer to non-void type
#define CELOG_PTR_RETURN_VAL(val)                                       \
  return fipwe.Ret(__LINE__, (val))



//
// External macros for function entering and leaving.  These should be used
// for:
// - functions that handle Structured Exceptions (SEH).
//

// Entering
#define CELOG_SEH_ENTER                                                 \
  CELog::FuncInvoke fi(CELOG_CUR_FILE, __LINE__,                        \
  CELOG_CUR_MODULE, TEXT(__FUNCTION__))

// Returning void
#define CELOG_SEH_RETURN                                                \
  fi.Ret(__LINE__); return

// Returning void expression
#define CELOG_SEH_RETURN_VOIDEXP(exp)                                   \
  (exp); return fi.Ret(__LINE__)

// Returning a value whose type is supported
#define CELOG_SEH_RETURN_VAL(val)                                       \
  return fi.Ret(__LINE__, (val))

// Returning a value whose type is not supported
#define CELOG_SEH_RETURN_VAL_NOPRINT(val)                               \
  fi.Ret(__LINE__); return (val)

// Entering (for functions returning an enum value)
#define CELOG_SEH_ENUM_ENTER(type)                                      \
  CELog::FuncInvokeEnum<type> fie(CELOG_CUR_FILE, __LINE__,             \
  CELOG_CUR_MODULE, TEXT(__FUNCTION__))

// Returning an enum value
#define CELOG_SEH_ENUM_RETURN_VAL(val)                                  \
  return fie.Ret(__LINE__, (val))

// Entering (for functions returning a pointer to non-void type)
#define CELOG_SEH_PTR_ENTER(type)                                       \
  CELog::FuncInvokePtr<type> fip(CELOG_CUR_FILE, __LINE__,              \
  CELOG_CUR_MODULE, TEXT(__FUNCTION__))

// Returning a pointer to non-void type
#define CELOG_SEH_PTR_RETURN_VAL(val)                                   \
  return fip.Ret(__LINE__, (val))



#endif /* __CELOG_H__ */
