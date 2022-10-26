#include "stdafx.h"
#include <windows.h>
#include <dbghelp.h>
#include <ShlObj.h>
#include <time.h>
#include <strsafe.h>
#include "ExceptionHandler.h"
#include "../common/CommonTools.h"

// Exception types used in CR_EXCEPTION_INFO::exctype structure member.
#define CR_SEH_EXCEPTION                0    //!< SEH exception.
#define CR_CPP_TERMINATE_CALL           1    //!< C++ terminate() call.
#define CR_CPP_UNEXPECTED_CALL          2    //!< C++ unexpected() call.
#define CR_CPP_PURE_CALL                3    //!< C++ pure virtual function call (VS .NET and later).
#define CR_CPP_NEW_OPERATOR_ERROR       4    //!< C++ new operator fault (VS .NET and later).
#define CR_CPP_SECURITY_ERROR           5    //!< Buffer overrun error (VS .NET only).
#define CR_CPP_INVALID_PARAMETER        6    //!< Invalid parameter exception (VS 2005 and later).
#define CR_CPP_SIGABRT                  7    //!< C++ SIGABRT signal (abort).
#define CR_CPP_SIGFPE                   8    //!< C++ SIGFPE signal (flotating point exception).
#define CR_CPP_SIGILL                   9    //!< C++ SIGILL signal (illegal instruction).
#define CR_CPP_SIGINT                   10   //!< C++ SIGINT signal (CTRL+C).
#define CR_CPP_SIGSEGV                  11   //!< C++ SIGSEGV signal (invalid storage access).
#define CR_CPP_SIGTERM                  12   //!< C++ SIGTERM signal (termination request).


#define CR_INST_STRUCTURED_EXCEPTION_HANDLER      0x1 //!< Install SEH handler (deprecated name, use \ref CR_INST_SEH_EXCEPTION_HANDLER instead).
#define CR_INST_SEH_EXCEPTION_HANDLER             0x1 //!< Install SEH handler.
#define CR_INST_TERMINATE_HANDLER                 0x2 //!< Install terminate handler.
#define CR_INST_UNEXPECTED_HANDLER                0x4 //!< Install unexpected handler.
#define CR_INST_PURE_CALL_HANDLER                 0x8 //!< Install pure call handler (VS .NET and later).
#define CR_INST_NEW_OPERATOR_ERROR_HANDLER       0x10 //!< Install new operator error handler (VS .NET and later).
#define CR_INST_SECURITY_ERROR_HANDLER           0x20 //!< Install security error handler (VS .NET and later).
#define CR_INST_INVALID_PARAMETER_HANDLER        0x40 //!< Install invalid parameter handler (VS 2005 and later).
#define CR_INST_SIGABRT_HANDLER                  0x80 //!< Install SIGABRT signal handler.
#define CR_INST_SIGFPE_HANDLER                  0x100 //!< Install SIGFPE signal handler.   
#define CR_INST_SIGILL_HANDLER                  0x200 //!< Install SIGILL signal handler.  
#define CR_INST_SIGINT_HANDLER                  0x400 //!< Install SIGINT signal handler.  
#define CR_INST_SIGSEGV_HANDLER                 0x800 //!< Install SIGSEGV signal handler.
#define CR_INST_SIGTERM_HANDLER                0x1000 //!< Install SIGTERM signal handler.  


#ifndef _AddressOfReturnAddress

// Taken from: http://msdn.microsoft.com/en-us/library/s975zw7k(VS.71).aspx
#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

// _ReturnAddress and _AddressOfReturnAddress should be prototyped before use 
EXTERNC void * _AddressOfReturnAddress(void);
EXTERNC void * _ReturnAddress(void);

#endif 


CExceptionHandler* CExceptionHandler::m_pInstance = NULL;
CExceptionHandler::CExceptionHandler() :m_bContinueExecution(TRUE), m_kunMaxSubItemCount(10)
{
    ZeroMemory(m_szCodeMsg, 1024 * 2);
    ZeroMemory(m_szReportDir, 1024);
    ZeroMemory(m_szDisplayMsg, 2048);

    std::wstring wstrDumpFolder = GetNLTempFileFolder(emTempLocationDump);
    unsigned int unsunItemCounts = GetItemCountInTheFolder(wstrDumpFolder);
    if (unsunItemCounts >= m_kunMaxSubItemCount)
    {
        DeleteFolderOrFile(wstrDumpFolder, false);
        CreateDirectoryW(wstrDumpFolder.c_str(), NULL);
    }
    swprintf_s(m_szReportDir, 511, L"%s", wstrDumpFolder.c_str());
}


CExceptionHandler::~CExceptionHandler()
{
}

CExceptionHandler* CExceptionHandler::GetInstance()
{
    if (m_pInstance == NULL)    m_pInstance = new CExceptionHandler();
    return m_pInstance;
}

void CExceptionHandler::InitInstance()
{
    CExceptionHandler* pCrashHandler = CExceptionHandler::GetInstance();
}

void CExceptionHandler::ReleaseInstance()
{
    if (NULL != m_pInstance)
    {
        delete m_pInstance;
        m_pInstance = NULL;
    }
}

LONG WINAPI RedirectedSetUnhandledExceptionFilter(EXCEPTION_POINTERS * /*ExceptionInfo*/)
{
    // When the CRT calls SetUnhandledExceptionFilter with NULL parameter
    // our handler will not get removed.
    return 0;
}

int CExceptionHandler::SetProcessExceptionHandlers(DWORD dwFlags)
{
    static bool gbInit = false;
    if (gbInit) return 0;
    gbInit = true;

    // installed
    if ((dwFlags&CR_INST_ALL_POSSIBLE_HANDLERS) == 0)
        dwFlags |= CR_INST_ALL_POSSIBLE_HANDLERS;

    if (dwFlags&CR_INST_STRUCTURED_EXCEPTION_HANDLER)
    {
        // Install top-level SEH handler
        m_oldSehHandler = SetUnhandledExceptionFilter(SehHandler);
    }

    _set_error_mode(_OUT_TO_STDERR);

#if _MSC_VER>=1300
    if (dwFlags&CR_INST_PURE_CALL_HANDLER)
    {
        // Catch pure virtual function calls.
        // Because there is one _purecall_handler for the whole process, 
        // calling this function immediately impacts all threads. The last 
        // caller on any thread sets the handler. 
        // http://msdn.microsoft.com/en-us/library/t296ys27.aspx
        m_prevPurec = _set_purecall_handler(PureCallHandler);
    }

    if (dwFlags&CR_INST_NEW_OPERATOR_ERROR_HANDLER)
    {
        // Catch new operator memory allocation exceptions
        _set_new_mode(1); // Force malloc() to call new handler too
        m_prevNewHandler = _set_new_handler(NewHandler);
    }
#endif

#if _MSC_VER>=1400
    if (dwFlags&CR_INST_INVALID_PARAMETER_HANDLER)
    {
        // Catch invalid parameter exceptions.
        m_prevInvpar = _set_invalid_parameter_handler(InvalidParameterHandler);
    }
#endif


#if _MSC_VER>=1300 && _MSC_VER<1400    
    if (dwFlags&CR_INST_SECURITY_ERROR_HANDLER)
    {
        // Catch buffer overrun exceptions
        // The _set_security_error_handler is deprecated in VC8 C++ run time library
        m_prevSec = _set_security_error_handler(SecurityHandler);
    }
#endif

    // Set up C++ signal handlers


    if (dwFlags&CR_INST_SIGABRT_HANDLER)
    {
#if _MSC_VER>=1400  
        _set_abort_behavior(_CALL_REPORTFAULT, _CALL_REPORTFAULT);
#endif
        // Catch an abnormal program termination
        m_prevSigABRT = signal(SIGABRT, SigabrtHandler);
    }

    if (dwFlags&CR_INST_SIGINT_HANDLER)
    {
        // Catch illegal instruction handler
        m_prevSigINT = signal(SIGINT, SigintHandler);
    }

    if (dwFlags&CR_INST_TERMINATE_HANDLER)
    {
        // Catch a termination request
        m_prevSigTERM = signal(SIGTERM, SigtermHandler);
    }

    return 0;
}

// Unsets exception pointers that work on per-process basis
int CExceptionHandler::UnSetProcessExceptionHandlers()
{
    // Unset all previously set handlers

#if _MSC_VER>=1300
    if (m_prevPurec != NULL)
        _set_purecall_handler(m_prevPurec);

    if (m_prevNewHandler != NULL)
        _set_new_handler(m_prevNewHandler);
#endif

#if _MSC_VER>=1400
    if (m_prevInvpar != NULL)
        _set_invalid_parameter_handler(m_prevInvpar);
#endif //_MSC_VER>=1400  

#if _MSC_VER>=1300 && _MSC_VER<1400    
    if (m_prevSec != NULL)
        _set_security_error_handler(m_prevSec);
#endif //_MSC_VER<1400

    if (m_prevSigABRT != NULL)
        signal(SIGABRT, m_prevSigABRT);

    if (m_prevSigINT != NULL)
        signal(SIGINT, m_prevSigINT);

    if (m_prevSigTERM != NULL)
        signal(SIGTERM, m_prevSigTERM);

    return 0;
}

// Installs C++ exception handlers that function on per-thread basis
int CExceptionHandler::SetThreadExceptionHandlers(DWORD dwFlags)
{
    // If 0 is specified as dwFlags, assume all available exception handlers should be
    // installed  
    if ((dwFlags&CR_INST_ALL_POSSIBLE_HANDLERS) == 0)
        dwFlags |= CR_INST_ALL_POSSIBLE_HANDLERS;

    // Get current thread ID.
    DWORD dwThreadId = GetCurrentThreadId();

    // Lock the critical section.
    CAutoLock lock(&m_csThreadExceptionHandlers);

    // Try and find our thread ID in the list of threads.
    std::map<DWORD, ThreadExceptionHandlers>::iterator it =
        m_ThreadExceptionHandlers.find(dwThreadId);

    if (it != m_ThreadExceptionHandlers.end())
    {
        // handlers are already set for the thread    
        //crSetErrorMsg(_T("Can't install handlers for current thread twice."));
        return 1; // failed
    }

    ThreadExceptionHandlers handlers;

    if (dwFlags&CR_INST_TERMINATE_HANDLER)
    {
        // Catch terminate() calls. 
        // In a multithreaded environment, terminate functions are maintained 
        // separately for each thread. Each new thread needs to install its own 
        // terminate function. Thus, each thread is in charge of its own termination handling.
        // http://msdn.microsoft.com/en-us/library/t6fk7h29.aspx
        handlers.m_prevTerm = set_terminate(TerminateHandler);
    }

    if (dwFlags&CR_INST_UNEXPECTED_HANDLER)
    {
        // Catch unexpected() calls.
        // In a multithreaded environment, unexpected functions are maintained 
        // separately for each thread. Each new thread needs to install its own 
        // unexpected function. Thus, each thread is in charge of its own unexpected handling.
        // http://msdn.microsoft.com/en-us/library/h46t5b69.aspx  
        handlers.m_prevUnexp = set_unexpected(UnexpectedHandler);
    }

    if (dwFlags&CR_INST_SIGFPE_HANDLER)
    {
        // Catch a floating point error
        typedef void(*sigh)(int);
        handlers.m_prevSigFPE = signal(SIGFPE, (sigh)SigfpeHandler);
    }


    if (dwFlags&CR_INST_SIGILL_HANDLER)
    {
        // Catch an illegal instruction
        handlers.m_prevSigILL = signal(SIGILL, SigillHandler);
    }

    if (dwFlags&CR_INST_SIGSEGV_HANDLER)
    {
        // Catch illegal storage access errors
        handlers.m_prevSigSEGV = signal(SIGSEGV, SigsegvHandler);
    }

    // Insert the structure to the list of handlers  
    m_ThreadExceptionHandlers[dwThreadId] = handlers;

    // OK.
    //crSetErrorMsg(_T("Success."));
    return 0;
}

// Unsets exception handlers for the current thread
int CExceptionHandler::UnSetThreadExceptionHandlers()
{
    //crSetErrorMsg(_T("Unspecified error."));

    DWORD dwThreadId = GetCurrentThreadId();

    CAutoLock lock(&m_csThreadExceptionHandlers);

    std::map<DWORD, ThreadExceptionHandlers>::iterator it =
        m_ThreadExceptionHandlers.find(dwThreadId);

    if (it == m_ThreadExceptionHandlers.end())
    {
        // No exception handlers were installed for the caller thread?    
        //crSetErrorMsg(_T("Crash handler wasn't previously installed for current thread."));
        return 1;
    }

    ThreadExceptionHandlers* handlers = &(it->second);

    if (handlers->m_prevTerm != NULL)
        set_terminate(handlers->m_prevTerm);

    if (handlers->m_prevUnexp != NULL)
        set_unexpected(handlers->m_prevUnexp);

    if (handlers->m_prevSigFPE != NULL)
        signal(SIGFPE, handlers->m_prevSigFPE);

    if (handlers->m_prevSigILL != NULL)
        signal(SIGINT, handlers->m_prevSigILL);

    if (handlers->m_prevSigSEGV != NULL)
        signal(SIGSEGV, handlers->m_prevSigSEGV);

    // Remove from the list
    m_ThreadExceptionHandlers.erase(it);

    // OK.
    //crSetErrorMsg(_T("Success."));
    return 0;
}

// The following code gets exception pointers using a workaround found in CRT code.
void CExceptionHandler::GetExceptionPointers(DWORD dwExceptionCode,
    EXCEPTION_POINTERS* pExceptionPointers)
{
    // The following code was taken from VC++ 8.0 CRT (invarg.c: line 104)

    CONTEXT ContextRecord;
    memset(&ContextRecord, 0, sizeof(CONTEXT));

#ifdef _X86_

    __asm {
        mov dword ptr[ContextRecord.Eax], eax
            mov dword ptr[ContextRecord.Ecx], ecx
            mov dword ptr[ContextRecord.Edx], edx
            mov dword ptr[ContextRecord.Ebx], ebx
            mov dword ptr[ContextRecord.Esi], esi
            mov dword ptr[ContextRecord.Edi], edi
            mov word ptr[ContextRecord.SegSs], ss
            mov word ptr[ContextRecord.SegCs], cs
            mov word ptr[ContextRecord.SegDs], ds
            mov word ptr[ContextRecord.SegEs], es
            mov word ptr[ContextRecord.SegFs], fs
            mov word ptr[ContextRecord.SegGs], gs
            pushfd
            pop[ContextRecord.EFlags]
    }

    ContextRecord.ContextFlags = CONTEXT_CONTROL;
#pragma warning(push)
#pragma warning(disable:4311)
    ContextRecord.Eip = (ULONG)_ReturnAddress();
    ContextRecord.Esp = (ULONG)_AddressOfReturnAddress();
#pragma warning(pop)
    ContextRecord.Ebp = *((ULONG *)_AddressOfReturnAddress() - 1);

#elif defined (_IA64_) || defined (_AMD64_)

    /* Need to fill up the Context in IA64 and AMD64. */
    RtlCaptureContext(&ContextRecord);

#else  /* defined (_IA64_) || defined (_AMD64_) */

    ZeroMemory(&ContextRecord, sizeof(ContextRecord));

#endif  /* defined (_IA64_) || defined (_AMD64_) */

    memcpy(pExceptionPointers->ContextRecord, &ContextRecord, sizeof(CONTEXT));

    ZeroMemory(pExceptionPointers->ExceptionRecord, sizeof(EXCEPTION_RECORD));

    pExceptionPointers->ExceptionRecord->ExceptionCode = dwExceptionCode;
    pExceptionPointers->ExceptionRecord->ExceptionAddress = _ReturnAddress();
}


// Acquires the crash lock. Other threads that may crash while we are 
// inside of a crash handler function, will wait until we unlock.
void CExceptionHandler::CrashLock(BOOL bLock)
{
    if (bLock)
        m_csCrashLock.Lock();
    else
        m_csCrashLock.Unlock();
}
//////////////////////////////////////////////////////////////////////////

void printlog(int line)
{
#if _DEBUG
    NLPRINT_DEBUGVIEWLOG(L"=========================> enter to line of %d, thread id is 0x%x.\n", line, GetCurrentThreadId());
#endif
}
int WINAPI CExceptionHandler::ExceptionFilter(unsigned int code, struct _EXCEPTION_POINTERS* ep)
{
    printlog(__LINE__);
    CExceptionHandler* pCrashHandler = CExceptionHandler::GetInstance();
    ATLASSERT(pCrashHandler != NULL);

    if (pCrashHandler == NULL)
    {
        return EXCEPTION_CONTINUE_SEARCH;
    }

    // it can't be handled , so we kill the process
    if (code == EXCEPTION_STACK_OVERFLOW)
    {
        HANDLE thread = ::CreateThread(0, 0,
            &StackOverflowThreadFunction, ep, 0, 0);
        ::WaitForSingleObject(thread, INFINITE);
        ::CloseHandle(thread);
        // Terminate process
        TerminateProcess(GetCurrentProcess(), 1);
    }
    else
    {
        CR_EXCEPTION_INFO ei;
        memset(&ei, 0, sizeof(CR_EXCEPTION_INFO));
        ei.cb = sizeof(CR_EXCEPTION_INFO);
        ei.exctype = CR_SEH_EXCEPTION;
        ei.pexcptrs = ep;
        ei.code = code;

        pCrashHandler->AyncWork(ei);
    }

    return EXCEPTION_EXECUTE_HANDLER;
}

// Structured exception handler (SEH)
LONG WINAPI CExceptionHandler::SehHandler(PEXCEPTION_POINTERS pExceptionPtrs)
{
    printlog(__LINE__);
    CExceptionHandler* pCrashHandler = CExceptionHandler::GetInstance();
    ATLASSERT(pCrashHandler != NULL);
    if (pCrashHandler != NULL)
    {
        // Acquire lock to avoid other threads (if exist) to crash while we are 
        // inside. 
        pCrashHandler->CrashLock(TRUE);

        CR_EXCEPTION_INFO ei;
        memset(&ei, 0, sizeof(CR_EXCEPTION_INFO));
        ei.cb = sizeof(CR_EXCEPTION_INFO);
        ei.exctype = CR_SEH_EXCEPTION;
        ei.pexcptrs = pExceptionPtrs;
        ei.code = 0;
        if (pExceptionPtrs != NULL && pExceptionPtrs->ExceptionRecord != NULL)   ei.code = pExceptionPtrs->ExceptionRecord->ExceptionCode;


        pCrashHandler->AyncWork(ei);
        // Free lock
        pCrashHandler->CrashLock(FALSE);
    }

    // Unreacheable code  
    return EXCEPTION_EXECUTE_HANDLER;
}

//Vojtech: Based on martin.bis...@gmail.com comment in
// http://groups.google.com/group/crashrpt/browse_thread/thread/a1dbcc56acb58b27/fbd0151dd8e26daf?lnk=gst&q=stack+overflow#fbd0151dd8e26daf
// Thread procedure doing the dump for stack overflow.
DWORD WINAPI CExceptionHandler::StackOverflowThreadFunction(LPVOID lpParameter)
{
    printlog(__LINE__);
    PEXCEPTION_POINTERS pExceptionPtrs =
        reinterpret_cast<PEXCEPTION_POINTERS>(lpParameter);

    CExceptionHandler *pCrashHandler = CExceptionHandler::GetInstance ();
    ATLASSERT(pCrashHandler != NULL);

    if (pCrashHandler != NULL)
    {
        // Acquire lock to avoid other threads (if exist) to crash while we	are inside.
        pCrashHandler->CrashLock(TRUE);

        pCrashHandler->m_bContinueExecution = FALSE;

        CR_EXCEPTION_INFO ei;
        memset(&ei, 0, sizeof(CR_EXCEPTION_INFO));
        ei.cb = sizeof(CR_EXCEPTION_INFO);
        ei.exctype = CR_SEH_EXCEPTION;
        ei.pexcptrs = pExceptionPtrs;
        ei.code = 0;
        if(pExceptionPtrs != NULL && pExceptionPtrs->ExceptionRecord != NULL)   ei.code = pExceptionPtrs->ExceptionRecord->ExceptionCode;
        pCrashHandler->AyncWork(ei);

        if (!pCrashHandler->m_bContinueExecution)
        {
            // Terminate process
            TerminateProcess(GetCurrentProcess(), 1);
        }

        // Free lock
        pCrashHandler->CrashLock(FALSE);
    }

    return 0;
}

// CRT terminate() call handler
void __cdecl CExceptionHandler::TerminateHandler()
{
    printlog(__LINE__);
    // Abnormal program termination (terminate() function was called)

    CExceptionHandler* pCrashHandler = CExceptionHandler::GetInstance();
    ATLASSERT(pCrashHandler != NULL);

    if (pCrashHandler != NULL)
    {
        // Acquire lock to avoid other threads (if exist) to crash while we are 
        // inside. We do not unlock, because process is to be terminated.
        pCrashHandler->CrashLock(TRUE);

        pCrashHandler->m_bContinueExecution = FALSE;

        CR_EXCEPTION_INFO ei;
        memset(&ei, 0, sizeof(CR_EXCEPTION_INFO));
        ei.cb = sizeof(CR_EXCEPTION_INFO);
        ei.exctype = CR_CPP_TERMINATE_CALL;

        // Generate crash report
        pCrashHandler->AyncWork(ei);

        if (!pCrashHandler->m_bContinueExecution)
        {
            // Terminate process
            TerminateProcess(GetCurrentProcess(), 1);
        }

        // Free lock
        pCrashHandler->CrashLock(FALSE);
    }
}

// CRT unexpected() call handler
void __cdecl CExceptionHandler::UnexpectedHandler()
{
    printlog(__LINE__);
    // Unexpected error (unexpected() function was called)

    CExceptionHandler* pCrashHandler = CExceptionHandler::GetInstance();
    ATLASSERT(pCrashHandler != NULL);

    if (pCrashHandler != NULL)
    {
        // Acquire lock to avoid other threads (if exist) to crash while we are 
        // inside. We do not unlock, because process is to be terminated.
        pCrashHandler->CrashLock(TRUE);

        pCrashHandler->m_bContinueExecution = FALSE;

        CR_EXCEPTION_INFO ei;
        memset(&ei, 0, sizeof(CR_EXCEPTION_INFO));
        ei.cb = sizeof(CR_EXCEPTION_INFO);
        ei.exctype = CR_CPP_UNEXPECTED_CALL;

        // Generate crash report
        pCrashHandler->AyncWork(ei);

        if (!pCrashHandler->m_bContinueExecution)
        {
            // Terminate process
            TerminateProcess(GetCurrentProcess(), 1);
        }

        // Free lock
        pCrashHandler->CrashLock(FALSE);
    }
}

// CRT Pure virtual method call handler
#if _MSC_VER>=1300
void __cdecl CExceptionHandler::PureCallHandler()
{
    printlog(__LINE__);
    // Pure virtual function call

    CExceptionHandler* pCrashHandler = CExceptionHandler::GetInstance();
    ATLASSERT(pCrashHandler != NULL);

    if (pCrashHandler != NULL)
    {
        // Acquire lock to avoid other threads (if exist) to crash while we are 
        // inside. We do not unlock, because process is to be terminated.
        pCrashHandler->CrashLock(TRUE);

        // Treat this type of crash critical by default
        pCrashHandler->m_bContinueExecution = FALSE;

        // Fill in the exception info
        CR_EXCEPTION_INFO ei;
        memset(&ei, 0, sizeof(CR_EXCEPTION_INFO));
        ei.cb = sizeof(CR_EXCEPTION_INFO);
        ei.exctype = CR_CPP_PURE_CALL;

        // Generate error report.
        pCrashHandler->AyncWork(ei);

        if (!pCrashHandler->m_bContinueExecution)
        {
            // Terminate process
            TerminateProcess(GetCurrentProcess(), 1);
        }

        // Free lock
        pCrashHandler->CrashLock(FALSE);
    }
}
#endif

// CRT buffer overrun handler. Available in CRT 7.1 only
#if _MSC_VER>=1300 && _MSC_VER<1400
void __cdecl CExceptionHandler::SecurityHandler(int code, void *x)
{
    // Security error (buffer overrun).
    printlog(__LINE__);
    UNREFERENCED_PARAMETER(code);
    UNREFERENCED_PARAMETER(x);

    CExceptionHandler* pCrashHandler = CExceptionHandler::GetInstance();
    ATLASSERT(pCrashHandler != NULL);

    if (pCrashHandler != NULL)
    {
        // Acquire lock to avoid other threads (if exist) to crash while we are 
        // inside. We do not unlock, because process is to be terminated.
        pCrashHandler->CrashLock(TRUE);

        // Treat this type of crash critical by default
        pCrashHandler->m_bContinueExecution = FALSE;

        // Fill in the exception info
        CR_EXCEPTION_INFO ei;
        memset(&ei, 0, sizeof(CR_EXCEPTION_INFO));
        ei.cb = sizeof(CR_EXCEPTION_INFO);
        ei.exctype = CR_CPP_SECURITY_ERROR;

        pCrashHandler->AyncWork(ei);

        if (!pCrashHandler->m_bContinueExecution)
        {
            // Terminate process
            TerminateProcess(GetCurrentProcess(), 1);
        }

        // Free lock
        pCrashHandler->CrashLock(FALSE);
    }
}
#endif 

// CRT invalid parameter handler
#if _MSC_VER>=1400
void __cdecl CExceptionHandler::InvalidParameterHandler(
    const wchar_t* expression,
    const wchar_t* function,
    const wchar_t* file,
    unsigned int line,
    uintptr_t pReserved)
{
    pReserved;
    printlog(__LINE__);
    // Invalid parameter exception

    CExceptionHandler* pCrashHandler = CExceptionHandler::GetInstance();
    ATLASSERT(pCrashHandler != NULL);

    if (pCrashHandler != NULL)
    {
        // Acquire lock to avoid other threads (if exist) to crash while we are 
        // inside. We do not unlock, because process is to be terminated.
        pCrashHandler->CrashLock(TRUE);

        // Treat this type of crash critical by default
        pCrashHandler->m_bContinueExecution = FALSE;

        // Fill in the exception info
        CR_EXCEPTION_INFO ei;
        memset(&ei, 0, sizeof(CR_EXCEPTION_INFO));
        ei.cb = sizeof(CR_EXCEPTION_INFO);
        ei.exctype = CR_CPP_INVALID_PARAMETER;
        ei.expression = expression;
        ei.function = function;
        ei.file = file;
        ei.line = line;

        pCrashHandler->AyncWork(ei);

        if (!pCrashHandler->m_bContinueExecution)
        {
            // Terminate process
            TerminateProcess(GetCurrentProcess(), 1);
        }

        // Free lock
        pCrashHandler->CrashLock(FALSE);
    }
}
#endif

// CRT new operator fault handler
#if _MSC_VER>=1300
int __cdecl CExceptionHandler::NewHandler(size_t)
{
    // 'new' operator memory allocation exception
    printlog(__LINE__);
    CExceptionHandler* pCrashHandler = CExceptionHandler::GetInstance ();
    ATLASSERT(pCrashHandler != NULL);

    if (pCrashHandler != NULL)
    {
        // Acquire lock to avoid other threads (if exist) to crash while we are 
        // inside. We do not unlock, because process is to be terminated.
        pCrashHandler->CrashLock(TRUE);

        // Treat this type of crash critical by default
        pCrashHandler->m_bContinueExecution = FALSE;

        // Fill in the exception info
        CR_EXCEPTION_INFO ei;
        memset(&ei, 0, sizeof(CR_EXCEPTION_INFO));
        ei.cb = sizeof(CR_EXCEPTION_INFO);
        ei.exctype = CR_CPP_NEW_OPERATOR_ERROR;
        ei.pexcptrs = NULL;

        // Generate error report.
        pCrashHandler->AyncWork(ei);

        if (!pCrashHandler->m_bContinueExecution)
        {
            // Terminate process
            TerminateProcess(GetCurrentProcess(), 1);
        }

        // Free lock
        pCrashHandler->CrashLock(FALSE);
    }

    // Unreacheable code
    return 0;
}
#endif //_MSC_VER>=1300

// CRT SIGABRT signal handler
void CExceptionHandler::SigabrtHandler(int)
{
    // Caught SIGABRT C++ signal
    printlog(__LINE__);
    CExceptionHandler* pCrashHandler = CExceptionHandler::GetInstance();
    ATLASSERT(pCrashHandler != NULL);

    if (pCrashHandler != NULL)
    {
        // Acquire lock to avoid other threads (if exist) to crash while we are 
        // inside. We do not unlock, because process is to be terminated.
        pCrashHandler->CrashLock(TRUE);

        // Treat this type of crash critical by default
        pCrashHandler->m_bContinueExecution = FALSE;

        // Fill in the exception info
        CR_EXCEPTION_INFO ei;
        memset(&ei, 0, sizeof(CR_EXCEPTION_INFO));
        ei.cb = sizeof(CR_EXCEPTION_INFO);
        ei.exctype = CR_CPP_SIGABRT;

        pCrashHandler->AyncWork(ei);

        if (!pCrashHandler->m_bContinueExecution)
        {
            // Terminate process
            TerminateProcess(GetCurrentProcess(), 1);
        }

        // Free lock
        pCrashHandler->CrashLock(FALSE);
    }
}

// CRT SIGFPE signal handler
void CExceptionHandler::SigfpeHandler(int /*code*/, int subcode)
{
    // Floating point exception (SIGFPE)
    printlog(__LINE__);
    CExceptionHandler* pCrashHandler = CExceptionHandler::GetInstance ();
    ATLASSERT(pCrashHandler != NULL);

    if (pCrashHandler != NULL)
    {
        // Acquire lock to avoid other threads (if exist) to crash while we are 
        // inside. We do not unlock, because process is to be terminated.
        pCrashHandler->CrashLock(TRUE);

        // Treat this type of crash critical by default
        pCrashHandler->m_bContinueExecution = FALSE;

        // Fill in the exception info
        CR_EXCEPTION_INFO ei;
        memset(&ei, 0, sizeof(CR_EXCEPTION_INFO));
        ei.cb = sizeof(CR_EXCEPTION_INFO);
        ei.exctype = CR_CPP_SIGFPE;
        ei.pexcptrs = (PEXCEPTION_POINTERS)_pxcptinfoptrs;
        ei.fpe_subcode = subcode;

        //Generate crash report.
        pCrashHandler->AyncWork(ei);

        if (!pCrashHandler->m_bContinueExecution)
        {
            // Terminate process
            TerminateProcess(GetCurrentProcess(), 1);
        }

        // Free lock
        pCrashHandler->CrashLock(FALSE);
    }
}

// CRT sigill signal handler
void CExceptionHandler::SigillHandler(int)
{
    // Illegal instruction (SIGILL)
    printlog(__LINE__);
    CExceptionHandler* pCrashHandler = CExceptionHandler::GetInstance();
    ATLASSERT(pCrashHandler != NULL);

    if (pCrashHandler != NULL)
    {
        // Acquire lock to avoid other threads (if exist) to crash while we are 
        // inside. We do not unlock, because process is to be terminated.
        pCrashHandler->CrashLock(TRUE);

        // Treat this type of crash critical by default
        pCrashHandler->m_bContinueExecution = FALSE;

        // Fill in the exception info
        CR_EXCEPTION_INFO ei;
        memset(&ei, 0, sizeof(CR_EXCEPTION_INFO));
        ei.cb = sizeof(CR_EXCEPTION_INFO);
        ei.exctype = CR_CPP_SIGILL;

        // Generate crash report
        pCrashHandler->AyncWork(ei);

        if (!pCrashHandler->m_bContinueExecution)
        {
            // Terminate process
            TerminateProcess(GetCurrentProcess(), 1);
        }

        // Free lock
        pCrashHandler->CrashLock(FALSE);
    }
}

// CRT sigint signal handler
void CExceptionHandler::SigintHandler(int)
{
    // Interruption (SIGINT)
    printlog(__LINE__);
    CExceptionHandler* pCrashHandler = CExceptionHandler::GetInstance();
    ATLASSERT(pCrashHandler != NULL);

    if (pCrashHandler != NULL)
    {
        // Acquire lock to avoid other threads (if exist) to crash while we are 
        // inside. We do not unlock, because process is to be terminated.
        pCrashHandler->CrashLock(TRUE);

        // Treat this type of crash critical by default
        pCrashHandler->m_bContinueExecution = FALSE;

        // Fill in the exception info
        CR_EXCEPTION_INFO ei;
        memset(&ei, 0, sizeof(CR_EXCEPTION_INFO));
        ei.cb = sizeof(CR_EXCEPTION_INFO);
        ei.exctype = CR_CPP_SIGINT;

        // Generate crash report.
        pCrashHandler->AyncWork(ei);

        if (!pCrashHandler->m_bContinueExecution)
        {
            // Terminate process
            TerminateProcess(GetCurrentProcess(), 1);
        }

        // Free lock
        pCrashHandler->CrashLock(FALSE);
    }
}

// CRT SIGSEGV signal handler
void CExceptionHandler::SigsegvHandler(int)
{
    // Invalid storage access (SIGSEGV)
    printlog(__LINE__);
    CExceptionHandler* pCrashHandler = CExceptionHandler::GetInstance();

    if (pCrashHandler != NULL)
    {
        // Acquire lock to avoid other threads (if exist) to crash while we are 
        // inside. We do not unlock, because process is to be terminated.
        pCrashHandler->CrashLock(TRUE);

        // Treat this type of crash critical by default
        pCrashHandler->m_bContinueExecution = FALSE;

        // Fill in exception info
        CR_EXCEPTION_INFO ei;
        memset(&ei, 0, sizeof(CR_EXCEPTION_INFO));
        ei.cb = sizeof(CR_EXCEPTION_INFO);
        ei.exctype = CR_CPP_SIGSEGV;
        ei.pexcptrs = (PEXCEPTION_POINTERS)_pxcptinfoptrs;

        // Generate crash report
        pCrashHandler->AyncWork(ei);

        if (!pCrashHandler->m_bContinueExecution)
        {
            // Terminate process
            TerminateProcess(GetCurrentProcess(), 1);
        }

        // Free lock
        pCrashHandler->CrashLock(FALSE);
    }
}

// CRT SIGTERM signal handler
void CExceptionHandler::SigtermHandler(int)
{
    // Termination request (SIGTERM)
    printlog(__LINE__);
    CExceptionHandler* pCrashHandler = CExceptionHandler::GetInstance();
    ATLASSERT(pCrashHandler != NULL);

    if (pCrashHandler != NULL)
    {
        // Acquire lock to avoid other threads (if exist) to crash while we are 
        // inside. We do not unlock, because process is to be terminated.
        pCrashHandler->CrashLock(TRUE);

        // Treat this type of crash critical by default
        pCrashHandler->m_bContinueExecution = FALSE;

        // Fill in the exception info
        CR_EXCEPTION_INFO ei;
        memset(&ei, 0, sizeof(CR_EXCEPTION_INFO));
        ei.cb = sizeof(CR_EXCEPTION_INFO);
        ei.exctype = CR_CPP_SIGTERM;

        // Generate crash report
        pCrashHandler->AyncWork(ei);

        if (!pCrashHandler->m_bContinueExecution)
        {
            // Terminate process
            TerminateProcess(GetCurrentProcess(), 1);
        }

        // Free lock
        pCrashHandler->CrashLock(FALSE);
    }
}

BOOL CExceptionHandler::SetDumpPrivileges()
{
    // This method is used to have the current process be able to call MiniDumpWriteDump
    // This code was taken from:
    // http://social.msdn.microsoft.com/Forums/en-US/vcgeneral/thread/f54658a4-65d2-4196-8543-7e71f3ece4b6/


    BOOL       fSuccess = FALSE;
    HANDLE      TokenHandle = NULL;
    TOKEN_PRIVILEGES TokenPrivileges;

    if (!OpenProcessToken(GetCurrentProcess(),
        TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
        &TokenHandle))
    {
        goto Cleanup;
    }

    TokenPrivileges.PrivilegeCount = 1;

    if (!LookupPrivilegeValue(NULL,
        SE_DEBUG_NAME,
        &TokenPrivileges.Privileges[0].Luid))
    {
        goto Cleanup;
    }

    TokenPrivileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    //Add privileges here.
    if (!AdjustTokenPrivileges(TokenHandle,
        FALSE,
        &TokenPrivileges,
        sizeof(TokenPrivileges),
        NULL,
        NULL))
    {
        goto Cleanup;
    }

    fSuccess = TRUE;

Cleanup:

    if (TokenHandle)
    {
        CloseHandle(TokenHandle);
    }

    return fSuccess;
}
void CExceptionHandler::GetUniqueDumpPath(__out wstring& strPath)
{
    // Get system time in UTC format

    time_t cur_time;
    time(&cur_time);
    wchar_t szDateTime[64] = { 0 };

#if _MSC_VER<1400
    struct tm* timeinfo = gmtime(&cur_time);
    wcsftime(szDateTime, 64, "%Y-%m-%dT%H-%M-%SZ", timeinfo);
#else
    struct tm timeinfo;
    gmtime_s(&timeinfo, &cur_time);
    wcsftime(szDateTime, 64, L"%Y-%m-%dT%H-%M-%SZ", &timeinfo);
#endif

    strPath = szDateTime;
    return ;
}
BOOL CExceptionHandler::CreateMiniDump(const wchar_t* strWorkFolder)
{
    BOOL bStatus = FALSE;
    HANDLE hFile = NULL;
    MINIDUMP_EXCEPTION_INFORMATION mei;
    MINIDUMP_CALLBACK_INFORMATION mci;

    // Load dbghelp.dll
    HMODULE hDbgHelp = LoadLibrary(L"dbghelp.dll");
    if (hDbgHelp == NULL)   return FALSE;

    // Try to adjust process privilegies to be able to generate minidumps.
    SetDumpPrivileges();

    wstring strDumpPath = strWorkFolder;
    strDumpPath += L"\\Outlook.dmp";
    // Create the minidump file
    hFile = CreateFile(
        strDumpPath.c_str(),
        GENERIC_WRITE,
        0,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL);

    // Check if file has been created
    if (hFile == INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }

    // Set valid dbghelp API version  
    typedef LPAPI_VERSION(WINAPI* LPIMAGEHLPAPIVERSIONEX)(LPAPI_VERSION AppVersion);
    LPIMAGEHLPAPIVERSIONEX lpImagehlpApiVersionEx =
        (LPIMAGEHLPAPIVERSIONEX)GetProcAddress(hDbgHelp, "ImagehlpApiVersionEx");
    ATLASSERT(lpImagehlpApiVersionEx != NULL);
    if (lpImagehlpApiVersionEx != NULL)
    {
        API_VERSION CompiledApiVer;
        CompiledApiVer.MajorVersion = 6;
        CompiledApiVer.MinorVersion = 1;
        CompiledApiVer.Revision = 11;
        CompiledApiVer.Reserved = 0;
        LPAPI_VERSION pActualApiVer = lpImagehlpApiVersionEx(&CompiledApiVer);
        pActualApiVer;
        ATLASSERT(CompiledApiVer.MajorVersion == pActualApiVer->MajorVersion);
        ATLASSERT(CompiledApiVer.MinorVersion == pActualApiVer->MinorVersion);
        ATLASSERT(CompiledApiVer.Revision == pActualApiVer->Revision);
    }

    // Write minidump to the file
    mei.ThreadId = m_crEI.dwThreadID;
    mei.ExceptionPointers = m_crEI.pexcptrs;
    mei.ClientPointers = FALSE;
    mci.CallbackRoutine = NULL;
    mci.CallbackParam = NULL;

    typedef BOOL(WINAPI *LPMINIDUMPWRITEDUMP)(
        HANDLE hProcess,
        DWORD ProcessId,
        HANDLE hFile,
        MINIDUMP_TYPE DumpType,
        CONST PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
        CONST PMINIDUMP_USER_STREAM_INFORMATION UserEncoderParam,
        CONST PMINIDUMP_CALLBACK_INFORMATION CallbackParam);

    // Get address of MiniDumpWirteDump function
    LPMINIDUMPWRITEDUMP pfnMiniDumpWriteDump =
        (LPMINIDUMPWRITEDUMP)GetProcAddress(hDbgHelp, "MiniDumpWriteDump");
    if (!pfnMiniDumpWriteDump)
    {
        goto cleanup;
    }
    DWORD dwPID = GetCurrentProcessId();


    // Now actually write the minidump
    BOOL bWriteDump = pfnMiniDumpWriteDump(
        GetCurrentProcess(),
        dwPID,
        hFile,
        MiniDumpWithFullMemory,
        &mei,
        NULL,
        &mci);

    // Check result
    if (!bWriteDump)
    {
        goto cleanup;
    }

    // Update progress
    bStatus = TRUE;

cleanup:

    // Close file
    if (hFile)
        CloseHandle(hFile);
    if (!bStatus)
    {
        DeleteFileW(strDumpPath.c_str());
    }

    // Unload dbghelp.dll
    if (hDbgHelp)
        FreeLibrary(hDbgHelp);

    return bStatus;
}
void CExceptionHandler::CreateLogFile(const wstring& strWorkFolder)
{
    wchar_t* pExcType = L"Error Type";
    switch (m_crEI.exctype)
    {
    case CR_SEH_EXCEPTION:
        pExcType = L"CR_SEH_EXCEPTION";
        break;
    case CR_CPP_TERMINATE_CALL:
        pExcType = L"CR_CPP_TERMINATE_CALL";
        break;
    case CR_CPP_UNEXPECTED_CALL:
        pExcType = L"CR_CPP_UNEXPECTED_CALL";
        break;
    case CR_CPP_PURE_CALL:
        pExcType = L"CR_CPP_PURE_CALL";
        break;
    case CR_CPP_NEW_OPERATOR_ERROR:
        pExcType = L"CR_CPP_NEW_OPERATOR_ERROR";
        break;
    case  CR_CPP_SECURITY_ERROR:
        pExcType = L"CR_CPP_SECURITY_ERROR";
        break;
    case CR_CPP_INVALID_PARAMETER:
        pExcType = L"CR_CPP_INVALID_PARAMETER";
        break;
    case CR_CPP_SIGABRT:
        pExcType = L"CR_CPP_SIGABRT";
        break;
    case CR_CPP_SIGFPE:
        pExcType = L"CR_CPP_SIGFPE";
        break;
    case CR_CPP_SIGILL:
        pExcType = L"CR_CPP_SIGILL";
        break;
    case CR_CPP_SIGINT:
        pExcType = L"CR_CPP_SIGINT";
        break;
    case CR_CPP_SIGSEGV:
        pExcType = L"CR_CPP_SIGSEGV";
        break;
    case CR_CPP_SIGTERM:
        pExcType = L"CR_CPP_SIGTERM";
        break;
    default:
        pExcType = L"NULL";
    }
    wchar_t szLog[2046] = { 0 };
    StringCchPrintfW(szLog, 2046, L"OE's version is: 8.3.0\nException type is:%s\nException code is:0x%x\nFile is:%s\nFunction is:%s\nLine is:%d\nExpression is:%s\nCode Message:%s",
        pExcType, m_crEI.code, m_crEI.file!=NULL?m_crEI.file:L"", m_crEI.function!=NULL?m_crEI.function:L"",
        m_crEI.line, m_crEI.expression!=NULL?m_crEI.expression:L"", m_szCodeMsg);


    wstring strFile = strWorkFolder + L"\\Outlook.log";
    HANDLE hFile = CreateFileW(strFile.c_str(), // file name
        GENERIC_WRITE,        // open for write 
        0,                    // do not share 
        NULL,                 // default security 
        CREATE_ALWAYS,        // overwrite existing
        FILE_ATTRIBUTE_NORMAL,// normal file 
        NULL);                // no template 
    if (hFile != INVALID_HANDLE_VALUE)
    {
		DWORD dwByteWrite= 0;
        WriteFile(hFile, szLog, wcslen(szLog)*2, &dwByteWrite, NULL);
        CloseHandle(hFile);
    }
}

void CExceptionHandler::ZipFiles(const wstring& strWorkFolder)
{
    strWorkFolder;
}
void CExceptionHandler::AyncWork(const CR_EXCEPTION_INFO& ei)
{
    EXCEPTION_RECORD ExceptionRecord;
    CONTEXT ContextRecord;
    EXCEPTION_POINTERS ExceptionPointers;
    ExceptionPointers.ExceptionRecord = &ExceptionRecord;
    ExceptionPointers.ContextRecord = &ContextRecord;

    memset(&m_crEI, 0, sizeof(CR_EXCEPTION_INFO));
    m_crEI.cb = sizeof(CR_EXCEPTION_INFO);
    m_crEI.bManual = ei.bManual;
    m_crEI.code = ei.code;
    m_crEI.exctype = ei.exctype;
    m_crEI.expression = ei.expression;
    m_crEI.file = ei.file;
    m_crEI.fpe_subcode = ei.fpe_subcode;
    m_crEI.function = ei.function;
    m_crEI.hSenderProcess = ei.hSenderProcess;
    m_crEI.line = ei.line;
    m_crEI.pexcptrs = ei.pexcptrs;
    // Get exception pointers if they were not provided by the caller. 
    if (m_crEI.pexcptrs == NULL)
    {
        GetExceptionPointers(m_crEI.code, &ExceptionPointers);
        m_crEI.pexcptrs = &ExceptionPointers;
    }
    m_crEI.dwThreadID = GetCurrentThreadId();

    // Generate unique folder for exception info
    wstring strUniqueFolder;
    GetUniqueDumpPath(strUniqueFolder);

    wchar_t szExceptionFolder[1024] = { 0 };
    StringCchPrintfW(szExceptionFolder, 1024, L"%s%s", m_szReportDir, strUniqueFolder.c_str());
    CreateDirectoryW(szExceptionFolder, NULL);

    if (ei.code != EXCEPTION_STACK_OVERFLOW)    CreateLogFile(szExceptionFolder);
    if (CreateMiniDump(szExceptionFolder))
    {
        const wchar_t const* pMsg = L"The application has saved the exception log details at \"%s\". Please Zip this folder and send to NextLabs for troubleshooting.";
        StringCchPrintfW(m_szDisplayMsg, 1024, pMsg, szExceptionFolder);
    }
    MessageBoxW(NULL, m_szDisplayMsg, L"Error Report", MB_OK | MB_ICONERROR);   
}

void CExceptionHandler::SetExpInfo(char* szFile, int nLine)
{
    CExceptionHandler* pCrashHandler = CExceptionHandler::GetInstance();
    ATLASSERT(pCrashHandler != NULL);

    if (pCrashHandler != NULL)
    {

        DWORD nNum = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szFile, -1, NULL, 0);
        wchar_t*  wzTempFile = new wchar_t[nNum];
        ZeroMemory(wzTempFile, nNum*sizeof(wchar_t));
        MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szFile, (int)strlen(szFile), wzTempFile, nNum);

        // Acquire lock to avoid other threads (if exist) to crash while we are 
        // inside. We do not unlock, because process is to be terminated.
        pCrashHandler->CrashLock(TRUE);

        StringCchPrintfW(pCrashHandler->m_szCodeMsg, 1024, L"Exception happen at file of %s and line of %d.\n", wzTempFile, nLine);
        // Free lock
        pCrashHandler->CrashLock(FALSE);
        delete[] wzTempFile;
    }
}
//////////////////////////////////////////////////////////////////////////