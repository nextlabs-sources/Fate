


#ifndef _COMMON_LOG_H_ 
#define _COMMON_LOG_H_ 

#include <string>
#include <vector>

#include "celog.h"

#define EVENTLOG_ERROR_INIT_ID    16461
#define EVENTLOG_ERROR_QUERY_ID   16462
#define EVENTLOG_INFO_ID          16363

class CCommonLog
{
public:
    CCommonLog();
    virtual ~CCommonLog();

    static void Initialize(void);

    static void PrintLogA(const char* _Fmt, ...);
	static void PrintLogA(int lvl, const char* _Fmt, ...);
	static void VPrintLogA(int lvl, const char* _Fmt, va_list _ArgList);

    static void PrintLogW(const wchar_t* _Fmt, ...);
	static void PrintLogW(int lvl, const wchar_t* _Fmt, ...);
	static void VPrintLogW(int lvl, const wchar_t* _Fmt, va_list _ArgList);

	static void PrintW(const wchar_t* _Fmt, ...);

protected:
private:
};

class CEventLog
{
public:
	CEventLog();
	virtual ~CEventLog();
	static void RegEventSource();
	static void DelEventSource();
	static void WriteEventLog(const wchar_t *strLog, WORD LogType, DWORD LgID);
	static HANDLE m_hEventLog;
	static bool  m_bSetlog;
	static DWORD m_dwTime;
	static void SetInitTime();

};

// Show debug
#ifdef _UNICODE

#define DP(x)   CCommonLog::PrintLogW x

#define llog(lvl, fmt, ...) CCommonLog::PrintLogW(lvl, fmt, ##__VA_ARGS__)
#define logd(fmt, ...) CCommonLog::PrintLogW(CELOG_DEBUG, fmt, ##__VA_ARGS__)
#define logi(fmt, ...) CCommonLog::PrintLogW(CELOG_INFO, fmt, ##__VA_ARGS__)
#define logw(fmt, ...) CCommonLog::PrintLogW(CELOG_WARNING, fmt, ##__VA_ARGS__)
#define loge(fmt, ...) CCommonLog::PrintLogW(CELOG_ERR, fmt, ##__VA_ARGS__)

#else

#define DP(x)   CCommonLog::PrintLogA x

#define llog(lvl, fmt, ...) CCommonLog::PrintLogA(lvl, fmt, ##__VA_ARGS__)
#define logd(fmt, ...) CCommonLog::PrintLogA(CELOG_DEBUG, fmt, ##__VA_ARGS__)
#define logi(fmt, ...) CCommonLog::PrintLogA(CELOG_INFO, fmt, ##__VA_ARGS__)
#define logw(fmt, ...) CCommonLog::PrintLogA(CELOG_WARNING, fmt, ##__VA_ARGS__)
#define loge(fmt, ...) CCommonLog::PrintLogA(CELOG_ERR, fmt, ##__VA_ARGS__)

#endif

#define DPA(x)	CCommonLog::PrintLogA x
#define DPW(x)	CCommonLog::PrintLogW x


////////////////////////////Only debug begin//////////////////////////////////////////////
/** Debug compile information */
#define chSTR2(x) #x
#define chSTR(x)  chSTR2(x)

#if 0
#define chMSG(desc) message("")
#else
#define chMSG(desc) message( __FILE__ "(" chSTR(__LINE__) "):" #desc )
#endif

/** Trace */
#define NLONLY_DEBUG nldebug::CFunTraceLog OnlyForDebugLog( __FUNCTIONW__, __LINE__ );

/** Debug log */
#define NLPRINT_DEBUGVIEWLOGEX( bOutputDebugLog, ... ) nldebug::NLPrintLogW(bOutputDebugLog,    L"\t %s:[%s:%s:%d]>:", nldebug::g_pKwchDebugLogHeader, __FUNCTIONW__, __FILEW__, __LINE__), nldebug::NLPrintLogW(bOutputDebugLog,    __VA_ARGS__)
#define NLPRINT_DEBUGVIEWLOG( ... )                    nldebug::NLPrintLogW(nldebug::g_kbOutputDebugLog, L"\t %s:[%s:%s:%d]>:", nldebug::g_pKwchDebugLogHeader, __FUNCTIONW__, __FILEW__, __LINE__), nldebug::NLPrintLogW(nldebug::g_kbOutputDebugLog, __VA_ARGS__)

#define NLPRINT_TAGPAIRLOG(vecTagPair, pwchBeginFlag, pwchEndFlag) nldebug::NLPrintLogW(nldebug::g_kbOutputDebugLog, L"\t %s:[%s:%d]>:", nldebug::g_pKwchDebugLogHeader, __FUNCTIONW__, __LINE__), nldebug::NLPrintTagPairW(vecTagPair, pwchBeginFlag, pwchEndFlag)

#define NLMESSAGEBOXW nldebug::NLMessageBoxW

namespace nldebug
{

    extern const wchar_t* g_pKwchDebugLogHeader;
    extern const bool g_kbOutputDebugLog;
    extern const bool g_kbOutputEventViewLog;
    extern const bool g_kbForceOutputDebugLog;

    class CFunTraceLog
    {
    public:
        CFunTraceLog(_In_ const wchar_t* const pkwchFuncName, _In_ const unsigned int unlLine);
        ~CFunTraceLog();
    private:
        std::wstring m_wstrFuncName;
        unsigned int m_unlStartLine;
    };

    wchar_t* NLConvertBoolToString(_In_ const bool bIsTtrue);

    void NLPrintLogW(_In_ const bool bOutputDebugLog, _In_ const wchar_t* _Fmt, ...);

    void NLPrintTagPairW(_In_ const std::vector<std::pair<std::wstring, std::wstring>>& kvecTagPair, _In_opt_ const wchar_t* pkwchBeginFlag = NULL, _In_opt_ const wchar_t* pkwchEndFlag = NULL);

    void NLOutputDebugStringW(_In_ wchar_t* pwchDebugLog);

    /** Debug message box */
    int NLMessageBoxW(_In_opt_ const wchar_t* pkwchText, _In_opt_ const wchar_t* pkwchCaption = L"KIM_TEST", _In_ unsigned int uType = MB_OK|MB_TOPMOST);

} /* namespace nldebug */

////////////////////////////Only debug end//////////////////////////////////////////////

#endif