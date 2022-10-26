

#include <windows.h>
#include <Shellapi.h>
#include <string>
#include <vector>
#include <algorithm>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/trim.hpp>
#include "nl_sysenc_lib.h"
#include "NextLabs_Types.h"

#pragma comment(linker, "/SUBSYSTEM:WINDOWS")


#define DLG_TITLE   L"NextLabs Portable Encryption"


class CParamParser
{
public:
    CParamParser();
    virtual ~CParamParser();

    BOOL Parse(_In_ int argc, _In_z_ LPWSTR* argv);

public:
    inline BOOL Wrap(){return m_bWrap;}
    inline BOOL UnWrap(){return m_bUnWrap;}
    inline std::wstring Path(){return m_path;}
    inline std::wstring Output(){return m_output;}
    inline std::wstring Error(){return m_le;}

protected:
    std::wstring GetParameter(_In_ LPCWSTR wzParam);

private:
    std::wstring    m_path;
    std::wstring    m_output;
    std::wstring    m_le;
    BOOL            m_bWrap;
    BOOL            m_bUnWrap;
    BOOL            m_bInplace;
};

class CWrapMgr
{
public:
    CWrapMgr();
    virtual ~CWrapMgr();

public:
    BOOL Wrap(_In_ LPCWSTR wzSource, _In_ LPCWSTR wzTarget);
    BOOL UnWrap(_In_ LPCWSTR wzSource, _In_ LPCWSTR wzTarget);
};

int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine,
                   int nCmdShow)
{
    CParamParser    parser;
    CWrapMgr        wrapmgr;
    int             nRet    = 0;
	int             argc    = 0;
	LPWSTR          *argv   = NULL;
	argv = CommandLineToArgvW(GetCommandLineW(), &argc);

    if(NULL==argv || 0==argc)
    {
        MessageBoxW(NULL, L"Wrong parameters!", DLG_TITLE, MB_OK);
        return 1;
    }

//    MessageBoxW(NULL, L"Break ..", L"Debug", MB_OK);

    if(!parser.Parse(argc, argv))
    {
        LocalFree(argv);
        MessageBoxW(NULL, parser.Error().c_str(), DLG_TITLE, MB_OK);
        return 1;
    }

    // Parameters have already been parsed, free memory
    LocalFree(argv);

    std::wstring strInfo;
    if(parser.Wrap())
    {
#if _DEBUG
        strInfo = L"Wrap:\n  Source: ";
        strInfo += parser.Path();
        strInfo += L"\n  Target: ";
        strInfo += parser.Output();
        MessageBoxW(NULL, strInfo.c_str(), DLG_TITLE, MB_OK);
#endif

        if(!wrapmgr.Wrap(parser.Path().c_str(), parser.Output().c_str()))
            nRet = 1;
    }
    else
    {
#if _DEBUG
        strInfo = L"Unwrap:\n  Source: ";
        strInfo += parser.Path();
        strInfo += L"\n  Target: ";
        strInfo += parser.Output();
        MessageBoxW(NULL, strInfo.c_str(), DLG_TITLE, MB_OK);
#endif

        if(!wrapmgr.UnWrap(parser.Path().c_str(), parser.Output().c_str()))
            nRet = 1;
    }

    return nRet;
}


CParamParser::CParamParser():m_bWrap(FALSE),m_bUnWrap(FALSE),m_bInplace(FALSE)
{
}

CParamParser::~CParamParser()
{
}

std::wstring CParamParser::GetParameter(_In_ LPCWSTR wzParam)
{
    std::wstring strCmdName = wzParam;
    boost::algorithm::trim_left(strCmdName);
    boost::algorithm::trim_right(strCmdName);
    
    return strCmdName;
}

BOOL CParamParser::Parse(_In_ int argc, _In_z_ LPWSTR* argv)
{
    static const std::wstring PARAM_WRAP      (L"--WRAP");
    static const std::wstring PARAM_UNWRAP    (L"--UNWRAP");
    static const std::wstring PARAM_INPLACE   (L"--INPLACE");
    static const std::wstring PARAM_PATH      (L"/PATH");
    static const std::wstring PARAM_OUTPUT    (L"--OUTPUT");

    std::vector<std::wstring>   vecCmd;
    int                         i = 0;

    //
    // Loop to parse all the parameters
    //
    for(i=0; i<argc; i++)
    {
        std::wstring strParam = GetParameter(argv[i]);

		std::transform(strParam.begin(), strParam.end(), strParam.begin(), toupper);

        //
        if(strParam == PARAM_WRAP)
        {
            m_bWrap = TRUE;
        }
        else if(strParam == PARAM_UNWRAP)
        {
            m_bUnWrap = TRUE;
        }
        else if(strParam == PARAM_INPLACE)
        {
            m_bInplace = TRUE;
        }
        else if(boost::algorithm::starts_with(strParam, PARAM_PATH.c_str()))
        {
            if(strParam == PARAM_PATH)
            {
                // This parameter is "/PATH" command
                // Next parameter must be the path
                // So we need to get next parameter

                // If there is no next parameter, error
                if(argc <= ++i)
                {
                    m_le = L"Cannot find value for \"PATH\" command";
                    return FALSE;
                }

                m_path = GetParameter(argv[i]);
            }
            else
            {
                // This parameter starts with "/PATH"
                // And rest part of this parameter is the path
                m_path = strParam.substr(PARAM_PATH.length());
            }
        }
        else if(boost::algorithm::starts_with(strParam, PARAM_OUTPUT.c_str()))
        {
            if(strParam == PARAM_OUTPUT)
            {
                // This parameter is "/OUTPUT" command
                // Next parameter must be the output path
                // So we need to get next parameter

                // If there is no next parameter, error
                if(argc <= ++i)
                {
                    m_le = L"Cannot find value for \"OUTPUT\" command";
                    return FALSE;
                }

                m_output = GetParameter(argv[i]);
            }
            else
            {
                // This parameter starts with "/PATH"
                // And rest part this parameter is the output path
                m_output = strParam.substr(PARAM_OUTPUT.length());
            }
        }
        else
        {
            // unknown command, continue
            continue;
        }
    }

    //
    // Sanity check
    //
    if(m_bWrap && m_bUnWrap)
    {
        m_le = L"Command \"WRAP\" and \"UNWRAP\" cannot be used at the same time";
        return FALSE;
    }

    if(m_path.empty())
    {
        m_le = L"Cannot find PATH parameter";
        return FALSE;
    }

    if(m_bWrap)
    {
        m_output = m_path;
        m_output += L".nxl";
    }
    else
    {
        if(!boost::algorithm::iends_with(m_path, L".nxl"))
        {
            m_le = L"Source file is not a valid NextLabs portable file";
            return FALSE;
        }
        if(m_output.empty())
        {
            m_output = m_path.substr(0, m_path.length()-4);
        }
    }

    return TRUE;
}

CWrapMgr::CWrapMgr()
{
}

CWrapMgr::~CWrapMgr()
{
}

BOOL CWrapMgr::Wrap(_In_ LPCWSTR wzSource, _In_ LPCWSTR wzTarget)
{
    //
    // Sanity check
    //
	
/*    if(INVALID_FILE_ATTRIBUTES == GetFileAttributes(wzSource))
    {
        MessageBoxW(NULL, L"Source file doesn't exist or is not a valid file, fail to wrap it.", DLG_TITLE, MB_OK);
        return FALSE;
    }*/

    //
    // Add a dummy file-open operation to force WDE to send out policy
    // evaluation request, just to make this process trusted.  This way it
    // won't fail when it tries to get the file key of the local-encrypted
    // source file later.
    //
    HANDLE h;

    h = CreateFile(wzSource, FILE_READ_DATA,
                   FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
                   NULL, OPEN_EXISTING, 0, NULL);

    if (h != INVALID_HANDLE_VALUE)
    {
      CloseHandle(h);
    }

    if(!SE_IsEncrypted(wzSource))
    {
        MessageBoxW(NULL, L"Source file is not a valid NextLabs encrypted file, fail to wrap it.", DLG_TITLE, MB_OK);
        return FALSE;
    }

 /*   if(INVALID_FILE_ATTRIBUTES != GetFileAttributes(wzTarget))
    {
        if(IDNO == MessageBoxW(NULL, L"Destination already exists, do you want to overwrite it?", DLG_TITLE, MB_YESNO))
            return FALSE;
    }*/

    //
    // Wrap
    //
    if (!SE_WrapEncryptedFile (wzSource, wzTarget))
    {
        SE_DisplayErrorMessage(SE_GetLastError());
        return FALSE;
    }

	NextLabsFile_Header_t   hdrinfo = {0};

	if(SE_GetFileInfo(wzTarget, SE_FileInfo_NextLabs, &hdrinfo))
	{
		const WCHAR* p = wcsrchr(wzSource, '\\');
		if(p)
		{
			memset(hdrinfo.orig_file_name, 0, sizeof(hdrinfo.orig_file_name));
			wcsncpy_s(hdrinfo.orig_file_name, sizeof(hdrinfo.orig_file_name)/sizeof(wchar_t), p + 1, _TRUNCATE);
			SE_SetFileInfo(wzTarget, SE_FileInfo_NextLabs, &hdrinfo);
		}
	}

    return TRUE;
}

BOOL CWrapMgr::UnWrap(_In_ LPCWSTR wzSource, _In_ LPCWSTR wzTarget)
{
    std::wstring strSource(wzSource);

    //
    // Sanity check
    //
    if(INVALID_FILE_ATTRIBUTES == GetFileAttributes(wzSource))
    {
        MessageBoxW(NULL, L"Source file doesn't exist or is not a valid file, fail to unwrap it.", DLG_TITLE, MB_OK);
        return FALSE;
    }

    //
    // Add a dummy file-open operation to force WDE to send out policy
    // evaluation request, just to make this process trusted.  This way it
    // won't fail when it tries to get the file key of the wrapped
    // source file later.
    //
    HANDLE h;

    h = CreateFile(wzSource, FILE_READ_DATA,
                   FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
                   NULL, OPEN_EXISTING, 0, NULL);

    if (h != INVALID_HANDLE_VALUE)
    {
      CloseHandle(h);
    }

    if(!boost::algorithm::iends_with(strSource, L".nxl"))
    {
        MessageBoxW(NULL, L"Source file is not a valid NextLabs portable file, fail to unwrap it.", DLG_TITLE, MB_OK);
        return FALSE;
    }

    BOOL  bNotExist = FALSE;
    DWORD dwAttrs = GetFileAttributesW(wzTarget);
    if(INVALID_FILE_ATTRIBUTES == dwAttrs)
    {
        DWORD dwLastError = GetLastError();
        if(ERROR_FILE_NOT_FOUND==dwLastError ||
           ERROR_PATH_NOT_FOUND==dwLastError)
        {
            bNotExist = TRUE;
        }
    }
    if(!bNotExist)
    {
        if(IDNO == MessageBoxW(NULL, L"Destination already exists, do you want to overwrite it?", DLG_TITLE, MB_YESNO))
            return FALSE;

        if(!DeleteFileW(wzTarget))
        {
            MessageBoxW(NULL, L"Fail to overwrite existing file.", DLG_TITLE, MB_OK);
            return FALSE;
        }
    }

    //
    // UnWrap
    //
    if (!SE_UnwrapToEncryptedFile (wzSource, wzTarget, TRUE))
    {
        SE_DisplayErrorMessage(SE_GetLastError());
        return FALSE;
    }

    return TRUE;
}
