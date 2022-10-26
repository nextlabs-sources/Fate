#pragma once
#include <windows.h>
#include <map>
#include <string>
#include <process.h>
#include <new.h>
#include <signal.h>
#include <exception>
#include <atldef.h>
#include "CritSec.h"
using std::wstring;


#define CR_INST_ALL_POSSIBLE_HANDLERS          0x1FFF //!< Install all possible exception handlers.
#define CR_INST_CRT_EXCEPTION_HANDLERS         0x1FFE //!< Install exception handlers for the linked CRT module.



/* This structure contains pointer to the exception handlers for a thread.*/
struct ThreadExceptionHandlers
{
    ThreadExceptionHandlers()
    {
        m_prevTerm = NULL;
        m_prevUnexp = NULL;
        m_prevSigFPE = NULL;
        m_prevSigILL = NULL;
        m_prevSigSEGV = NULL;
    }

    terminate_handler m_prevTerm;        // Previous terminate handler   
    unexpected_handler m_prevUnexp;      // Previous unexpected handler
    void(__cdecl *m_prevSigFPE)(int);   // Previous FPE handler
    void(__cdecl *m_prevSigILL)(int);   // Previous SIGILL handler
    void(__cdecl *m_prevSigSEGV)(int);  // Previous illegal storage access handler
};


typedef struct tagCR_EXCEPTION_INFO
{
    WORD cb;                   //!< Size of this structure in bytes; should be initialized before using.
    PEXCEPTION_POINTERS pexcptrs; //!< Exception pointers.
    int exctype;               //!< Exception type.
    DWORD code;                //!< Code of SEH exception.
    unsigned int fpe_subcode;  //!< Floating point exception subcode.
    const wchar_t* expression; //!< Assertion expression.
    const wchar_t* function;   //!< Function in which assertion happened.
    const wchar_t* file;       //!< File in which assertion happened.
    unsigned int line;         //!< Line number.
    BOOL bManual;              //!< Flag telling if the error report is generated manually or not.
    HANDLE hSenderProcess;     //!< Handle to the CrashSender.exe process.
    DWORD dwThreadID;
}
CR_EXCEPTION_INFO;

typedef CR_EXCEPTION_INFO* PCR_EXCEPTION_INFO;

class CExceptionHandler
{
private:
    CExceptionHandler();
    ~CExceptionHandler();
public:
    static void InitInstance();
    static CExceptionHandler* CExceptionHandler::GetInstance();
    static void ReleaseInstance();
private:
    // grasp dump file from a new thread
    void AyncWork(const CR_EXCEPTION_INFO& ei);
    
    BOOL CreateMiniDump(const wchar_t* strWorkFolder);
    BOOL CExceptionHandler::SetDumpPrivileges();

    // get current time as file path
    void GetUniqueDumpPath(__out wstring& strPath);

    // create log file
    void CreateLogFile(const wstring& strWorkFolder);

    void ZipFiles(const wstring& strWorkFolder);
//////////////////////////////////////////////////////////////////////////
public:

    /* Exception handler functions. */
    // Structured exception handler (SEH handler)
    static LONG WINAPI SehHandler(__in PEXCEPTION_POINTERS pExceptionPtrs);
    static DWORD WINAPI StackOverflowThreadFunction(LPVOID threadParameter);
    // C++ terminate handler
    static void __cdecl TerminateHandler();
    // C++ unexpected handler
    static void __cdecl UnexpectedHandler();

#if _MSC_VER>=1300
    // C++ pure virtual call handler
    static void __cdecl PureCallHandler();
#endif 

#if _MSC_VER>=1300 && _MSC_VER<1400
    // Buffer overrun handler (deprecated in newest versions of Visual C++).
    static void __cdecl SecurityHandler(int code, void *x);
#endif

#if _MSC_VER>=1400
    // C++ Invalid parameter handler.
    static void __cdecl InvalidParameterHandler(const wchar_t* expression,
        const wchar_t* function, const wchar_t* file, unsigned int line, uintptr_t pReserved);
#endif

#if _MSC_VER>=1300
    // C++ new operator fault (memory exhaustion) handler
    static int __cdecl NewHandler(size_t);
#endif

    // Signal handlers
    static void SigabrtHandler(int);
    static void SigfpeHandler(int /*code*/, int subcode);
    static void SigintHandler(int);
    static void SigillHandler(int);
    static void SigsegvHandler(int);
    static void SigtermHandler(int);

    /* Crash report generation methods */

    // Collects current state of CPU registers.
    void GetExceptionPointers(
        DWORD dwExceptionCode,
        EXCEPTION_POINTERS* pExceptionPointers);


#if _MSC_VER>=1300
    _purecall_handler m_prevPurec;   // Previous pure virtual call exception filter.
    _PNH m_prevNewHandler; // Previous new operator exception filter.
#endif

#if _MSC_VER>=1400
    _invalid_parameter_handler m_prevInvpar; // Previous invalid parameter exception filter.
#endif

#if _MSC_VER>=1300 && _MSC_VER<1400
    _secerr_handler_func m_prevSec; // Previous security exception filter.
#endif

    void(__cdecl *m_prevSigABRT)(int); // Previous SIGABRT handler.  
    void(__cdecl *m_prevSigINT)(int);  // Previous SIGINT handler.
    void(__cdecl *m_prevSigTERM)(int); // Previous SIGTERM handler.

    void CExceptionHandler::CrashLock(BOOL bLock);
    int SetProcessExceptionHandlers(DWORD dwFlags = CR_INST_ALL_POSSIBLE_HANDLERS);
    int CExceptionHandler::UnSetThreadExceptionHandlers();
    int CExceptionHandler::SetThreadExceptionHandlers(DWORD dwFlags);
    int CExceptionHandler::UnSetProcessExceptionHandlers();


    static int WINAPI ExceptionFilter(unsigned int code, struct _EXCEPTION_POINTERS* ep);
    static void WINAPI SetExpInfo(char* szFile, int nLine);

//////////////////////////////////////////////////////////////////////////
private:
    
    // Previous SEH exception filter.
    LPTOP_LEVEL_EXCEPTION_FILTER  m_oldSehHandler;

    wchar_t m_szCodeMsg[1024];
    wchar_t m_szDisplayMsg[1024];
    BOOL m_bContinueExecution;
    CR_EXCEPTION_INFO m_crEI;
    wchar_t m_szReportDir[512];                 // a folder folder full path, end with '\\'
    const unsigned int m_kunMaxSubItemCount;

    static CExceptionHandler* m_pInstance;

    CCritSec m_csThreadExceptionHandlers; // Synchronization lock for m_ThreadExceptionHandlers.
    std::map<DWORD, ThreadExceptionHandlers> m_ThreadExceptionHandlers;
    CCritSec m_csCrashLock;        // Critical section used to synchronize thread access to this object. 
};

