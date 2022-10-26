

#ifndef _COMMON_LOG_H_ 
#define _COMMON_LOG_H_ 

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

// Show debug

#ifdef _UNICODE
#define DP(x)   CCommonLog::PrintLogW x
#else
#define DP(x)   CCommonLog::PrintLogA x
#endif


#endif