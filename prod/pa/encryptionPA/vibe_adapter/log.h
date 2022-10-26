

#ifndef _COMMON_LOG_H_ 
#define _COMMON_LOG_H_ 

#ifdef WIN32
class CCommonLog
{
public:
    CCommonLog();
    virtual ~CCommonLog();

    static void PrintLogA(const char* _Fmt, ...);
    static void PrintLogW(const wchar_t* _Fmt, ...);

protected:
private:
};

void NL_Trace(const char *file, int line, const char *proc, const char *fmt, ...);

// Show debug
//#ifdef _DEBUG
#define DPA(x)	CCommonLog::PrintLogA x
#define DPW(x)	CCommonLog::PrintLogW x

#ifdef _UNICODE
#define DP(x)   CCommonLog::PrintLogW x
#else
#define DP(x)   CCommonLog::PrintLogA x
#endif
//#else
//#define DP(x)
//#endif
#else
#define DPA(x) printf x
//#define DP(x) printf x
#endif

#endif