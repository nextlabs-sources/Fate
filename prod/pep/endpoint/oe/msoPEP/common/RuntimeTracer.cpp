/** precompile header and resource header files */
#include "stdafx.h"
/** current class declare header files */
#include "RuntimeTracer.h"
/** C system header files */

/** C++ system header files */
#include <iostream>
#include <fstream> 
/** Platform header files */
#include <shlobj.h>
#include <shlwapi.h>
/** Third part library header files */
/** boost */

/** Other project header files */

/** Current project header files */
#include "CommonTools.h"
#include "log.h"

const DWORD g_kdwLimitSeconds = 2;

std::wstring CRuntimeTracer::s_wstrSysTempFolder = L"";
std::wstring CRuntimeTracer::s_wstrFolder = L"";
const DWORD  CRuntimeTracer::s_kdwTimeLimitUnit = 1 * 1000 * 1000 * 10; // second
const DWORD  CRuntimeTracer::s_kdwMaxFileSize = 2 * 1024 * 1024;     // 2M, log file max size is 2M
const unsigned int CRuntimeTracer::s_kunMaxFilesCount = 1000;   // 1k, log files in one log folder max count is 1k
const unsigned int CRuntimeTracer::s_kunMaxSubItemCount = 100;

CRuntimeTracer::CRuntimeTracer(_In_ const bool kbAutoRecord, _In_ const bool kbSetInfoImmediately, _In_ const wchar_t* kpwchFileName, _In_ const unsigned int kunLine, _In_ const wchar_t* kpwchExternInfo, _In_ const DWORD kdwLimitSeconds)
                    : m_bAutoRecord(kbAutoRecord), m_kwstrFileName((NULL==kpwchFileName)?L"":kpwchFileName), m_kunLine(kunLine), m_wstrExternInfo((NULL==kpwchExternInfo)?L"":kpwchExternInfo), m_kdwLimitSeconds(kdwLimitSeconds)
{
    if (kbSetInfoImmediately)
    {
        SetRuntimeInfo(m_kwstrFileName, m_kunLine, m_wstrExternInfo.c_str());
    }
}

CRuntimeTracer::~CRuntimeTracer()
{
    if (m_bAutoRecord)
    {
        RecordRuntimeInfo(m_kwstrFileName, m_kunLine, (L"(AutoRecord):" + m_wstrExternInfo).c_str(), m_kdwLimitSeconds);
    }
}

void CRuntimeTracer::InitBasicInfo()
{
    InitBaseOELogTempFolder();
}

void CRuntimeTracer::SetRuntimeInfo(_In_ const std::wstring& kwstrFileName, _In_ const unsigned int kunLine, _In_ const wchar_t* kpwchExternInfo)
{
    STUNL_RUNTIMEINFO stuRuntimeInfo(GetFileName(kwstrFileName), kunLine);
    GetSystemTime(&stuRuntimeInfo.stuSTUncTime);
    SystemTimeToFileTime(&stuRuntimeInfo.stuSTUncTime, &stuRuntimeInfo.unFTUncTime.stuFTTime);
    if (NULL != kpwchExternInfo)
    {
        stuRuntimeInfo.wstrExternInfo = kpwchExternInfo;
    }
    m_vecRuntimeInfo.push_back(stuRuntimeInfo);
}

void CRuntimeTracer::RecordRuntimeInfo(_In_ const std::wstring& kwstrFileName, _In_ const unsigned int kunLine, _In_ const wchar_t* kpwchExternInfo, _In_ const DWORD kdwLimitSeconds)
{
    SetRuntimeInfo(kwstrFileName, kunLine, kpwchExternInfo);
    RecordLogs(kdwLimitSeconds);
    m_bAutoRecord = false;
}

void CRuntimeTracer::RecordLogs(_In_ const DWORD kdwLimitSeconds)
{
    UNNL_MYFILETIME unFileTime = 0;
    size_t stVesSize = m_vecRuntimeInfo.size();
    size_t stEvenLen = stVesSize - (stVesSize % 2);
    for (size_t stIndex = 0; stIndex < stEvenLen; stIndex += 2)
    {
        unFileTime.dw64Time += (m_vecRuntimeInfo[stIndex+1].unFTUncTime.dw64Time - m_vecRuntimeInfo[stIndex].unFTUncTime.dw64Time);
    }

    if (unFileTime.dw64Time >= (kdwLimitSeconds * s_kdwTimeLimitUnit))
    {
        // Write logs into file
        DWORD dwThreadID = GetCurrentThreadId();
        wchar_t wszFileName[128] = { 0 };
        swprintf_s(wszFileName, 127, L"OE%u", dwThreadID);
        std::wstring wstrFileFullPath = s_wstrFolder + wszFileName + L".log";

        std::wfstream wfstreamFileOb(wstrFileFullPath.c_str(), std::ios::app | std::ios::in);
        if (wfstreamFileOb.is_open())
        {
            wchar_t wszLog[2048] = { 0 };
            swprintf_s(wszLog, 2047, L"Brief: %s:%d:TotalTime:[%u](UTC time, unit:100ns)", m_vecRuntimeInfo[stVesSize - 1].wstrFileName.c_str(), m_vecRuntimeInfo[stVesSize - 1].unLine, unFileTime.dw64Time);
            wfstreamFileOb << wszLog << std::endl << L"Details" << std::endl;
            for (size_t stIndex = 0; stIndex < stVesSize; ++stIndex)
            {
                STUNL_RUNTIMEINFO& stuRuntimeInfo = m_vecRuntimeInfo[stIndex];
                std::wstring wstrSTTime = ConverSysTimeToString(stuRuntimeInfo.stuSTUncTime);
                swprintf_s(wszLog, 2047, L"\t%u:%s:%u:Time:[%s(%u-%u)]", stIndex, stuRuntimeInfo.wstrFileName.c_str(), stuRuntimeInfo.unLine, wstrSTTime.c_str(), stuRuntimeInfo.unFTUncTime.stuFTTime.dwHighDateTime, stuRuntimeInfo.unFTUncTime.stuFTTime.dwLowDateTime);
                wfstreamFileOb << wszLog << std::endl;
                if (!stuRuntimeInfo.wstrExternInfo.empty())
                {
                    wfstreamFileOb << L"\t\tExternInfo:" << stuRuntimeInfo.wstrExternInfo.c_str() << std::endl;
                }
            }

            wfstreamFileOb.seekg(0, std::ios::end);
            DWORD64 dwFileCount = wfstreamFileOb.tellg();   // use DWORD64 to avoid a waring
            wfstreamFileOb.close();

            if (dwFileCount >= s_kdwMaxFileSize)
            {
                std::wstring wstrNewFileFullPath = s_wstrFolder + wszFileName + L"(" + NLNewGUID() + L").log";
                MoveFileW(wstrFileFullPath.c_str(), wstrNewFileFullPath.c_str());
            }
        }
    }
}

//////////////////////////////Independence tools////////////////////////////////////////////
void CRuntimeTracer::InitBaseOELogTempFolder()
{
    if (s_wstrSysTempFolder.empty() || (!PathFileExistsW(s_wstrSysTempFolder.c_str())))
    {
        s_wstrSysTempFolder = GetNLTempFileFolder(emTempLocationOverTimeLog);
    }
    DeleteEmptyFolderInTheFolder(s_wstrSysTempFolder);
    unsigned int unsunItemCounts = GetItemCountInTheFolder(s_wstrSysTempFolder);
    if (unsunItemCounts >= s_kunMaxSubItemCount)
    {
        DeleteFolderOrFile(s_wstrSysTempFolder, false);
        CreateDirectoryW(s_wstrSysTempFolder.c_str(), NULL);
    }
    InitCurUniqueOELogTempFolder();
}

void CRuntimeTracer::InitCurUniqueOELogTempFolder()
{
    bool bNeedCreateNewTempFolder = (s_wstrFolder.empty() || (!PathFileExistsW(s_wstrFolder.c_str())));
    if (!bNeedCreateNewTempFolder)
    {
        unsigned int unsunItemCounts = GetItemCountInTheFolder(s_wstrFolder);
        bNeedCreateNewTempFolder = (unsunItemCounts >= s_kunMaxFilesCount);
    }
    if (bNeedCreateNewTempFolder)
    {
        s_wstrFolder = s_wstrSysTempFolder + NLNewGUID() + L"\\";
        CreateDirectoryW(s_wstrFolder.c_str(), NULL);
    }
}