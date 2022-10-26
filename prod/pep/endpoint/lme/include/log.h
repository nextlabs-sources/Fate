

#ifndef _COMMON_LOG_H_ 
#define _COMMON_LOG_H_ 

class CCommonLog
{
public:
    CCommonLog();
    virtual ~CCommonLog();

    static void Initialize(void);
    static void PrintLogA(const char* _Fmt, ...);
    static void PrintLogW(const wchar_t* _Fmt, ...);

protected:
private:
};

#ifdef _UNICODE
#define DP(x)   CCommonLog::PrintLogW x
#else
#define DP(x)   CCommonLog::PrintLogA x
#endif

#define DPA(x)	CCommonLog::PrintLogA x
#define DPW(x)	CCommonLog::PrintLogW x
BOOL GetCurrentProcessName2(LPSTR lpszProcessName, int nLen, HMODULE hMod);
#endif /* _COMMON_LOG_H_  */
