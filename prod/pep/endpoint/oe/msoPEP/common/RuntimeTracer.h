/**
*  @file CRuntimeTracer is used to trace the runtime info and record how much time of the function spend
*  @details
*       Use NLTRACER_SETRUNTIMEINFO to set the runtime info, this macro only add the info into cache
*       Use NLTRACER_RECORDRUNTIMEINFO to set the last runtime info and record the logs into log file.
*       We can use NLTRACER_SETRUNTIMEINFO to add many times and NLTRACER_RECORDRUNTIMEINFO will compute the total spend times for every even pair.
*  E.g:
*       CRuntimeTracer::InitBasicInfo();                    // Init global info
*       void MainLogicFunction()
*       {
*           CRuntimeTracer theRuntimeTracer;                    // New an object
*
*           NLTRACER_SETRUNTIMEINFO(theRuntimeTracer, NULL);    // A point, set
*           GetSomeInitInfo();  // logic
*           NLTRACER_SETRUNTIMEINFO(theRuntimeTracer, NULL);    // B point, set

*           PopupMessageBox()   // UI, user op

*           NLTRACER_SETRUNTIMEINFO(theRuntimeTracer, NULL);    // C point, set
*           Process();          // logic
*           FreeResource()
*           NLTRACER_RECORDRUNTIMEINFO(theRuntimeTracer, L"Main logic end", 5);  // D point, record
*       }
*       For this code, total time spend is: (B-A) + (D-C)
*
*  @author NextLabs::Kim
*/

#ifndef NLDEBUG_RUNTIMETRACER_H_
#define NLDEBUG_RUNTIMETRACER_H_

#include <string>
#include <vector>

#include <windows.h>

#define NLTRACER_CREATEINSTANCE(kbAutoRecord, kbSetInfoImmediately, pwchExternInfo)  CRuntimeTracer theRuntimeTracer(kbAutoRecord, kbSetInfoImmediately, __FILEW__, __LINE__, pwchExternInfo, g_kdwLimitSeconds)
#define NLTRACER_SETRUNTIMEINFO(pwchExternInfo)                                      theRuntimeTracer.SetRuntimeInfo(__FILEW__, __LINE__, pwchExternInfo)
#define NLTRACER_RECORDRUNTIMEINFO(pwchExternInfo)                                   theRuntimeTracer.RecordRuntimeInfo(__FILEW__, __LINE__, pwchExternInfo, g_kdwLimitSeconds)

extern const DWORD g_kdwLimitSeconds;

union UNNL_MYFILETIME
{
    DWORD64 dw64Time;
    FILETIME stuFTTime;

    UNNL_MYFILETIME(_In_ const DWORD64 kdwParamTime = 0) : dw64Time(kdwParamTime) {}
};

struct STUNL_RUNTIMEINFO
{
    std::wstring wstrFileName;
    unsigned int unLine;
    UNNL_MYFILETIME unFTUncTime;
    SYSTEMTIME stuSTUncTime;
    std::wstring wstrExternInfo;

    STUNL_RUNTIMEINFO(const std::wstring kwstrParamFileName = L"", const unsigned int kunParamLine = 0, const UNNL_MYFILETIME kstuParamFTUncTime = 0, const std::wstring kwstrParamExternInfo = L"")
        : wstrFileName(kwstrParamFileName), unLine(kunParamLine), unFTUncTime(kstuParamFTUncTime), wstrExternInfo(kwstrParamExternInfo) 
    {
        memset(&stuSTUncTime, 0, sizeof(stuSTUncTime));
        if (0 != unFTUncTime.dw64Time)
        {
            SystemTimeToFileTime(&stuSTUncTime, &unFTUncTime.stuFTTime);
        }
    }

    STUNL_RUNTIMEINFO(_In_ const STUNL_RUNTIMEINFO& kstuRuntimeInfo)
    {
        Copy(kstuRuntimeInfo);
    }

    void Copy(_In_ const STUNL_RUNTIMEINFO& kstuRuntimeInfo)
    {
        this->wstrFileName = kstuRuntimeInfo.wstrFileName;
        this->unLine = kstuRuntimeInfo.unLine;
        this->unFTUncTime = kstuRuntimeInfo.unFTUncTime;
        this->stuSTUncTime = kstuRuntimeInfo.stuSTUncTime;
        this->wstrExternInfo = kstuRuntimeInfo.wstrExternInfo;
    }
};

class CRuntimeTracer
{
public:
    CRuntimeTracer(_In_ const bool kbAutoRecord = false, _In_ const bool kbSetInfoImmediately = false, _In_ const wchar_t* kpwchFileName = NULL, _In_ const unsigned int kunLine = 0, _In_ const wchar_t* kpwchExternInfo = NULL, _In_ const DWORD kdwLimitSeconds = 0);
    ~CRuntimeTracer();

public:
    static void InitBasicInfo(); // Init folder and file.

public:
    void SetRuntimeInfo(_In_ const std::wstring& kwstrFileName, _In_ const unsigned int kunLine, _In_ const wchar_t* kpwchExternInfo);
    void RecordRuntimeInfo(_In_ const std::wstring& kwstrFileName, _In_ const unsigned int kunLine, _In_ const wchar_t* kpwchExternInfo, _In_ const DWORD kdwLimitSeconds); // kdwLimitSeconds, a time, unit is second

private:
    void RecordLogs(_In_ const DWORD kdwLimitSeconds) throw(); // kdwLimitSeconds, a time, unit is second

private:
    static void InitBaseOELogTempFolder();
    static void InitCurUniqueOELogTempFolder();

private:
    std::vector<STUNL_RUNTIMEINFO> m_vecRuntimeInfo;
    bool m_bAutoRecord;
    std::wstring m_kwstrFileName;
    unsigned int m_kunLine;
    std::wstring m_wstrExternInfo;
    DWORD m_kdwLimitSeconds;

private:
    static std::wstring s_wstrSysTempFolder;
    static std::wstring s_wstrFolder;
    const static DWORD  s_kdwTimeLimitUnit;             // Unit, second
    const static DWORD  s_kdwMaxFileSize;               // if the log file over than this max size, need used a new one.
    const static unsigned int s_kunMaxFilesCount;       // if the log files count over in the folder s_wstrFolder, need used a new folder.
    const static unsigned int s_kunMaxSubItemCount;   // if the log temp folders count over in the folder s_wstrSysTempFolder, need used a new folder.
};

#endif /**< NLDEBUG_RUNTIMETRACER_H_ */