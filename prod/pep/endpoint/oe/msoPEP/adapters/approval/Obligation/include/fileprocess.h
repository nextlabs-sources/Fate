

#pragma once
#ifndef _FILE_PROCESS_H_
#define _FILE_PROCESS_H_
#include <string>

#if(_WIN32_WINNT < 0x0500)
#undef _WIN32_WINNT
#define _WIN32_WINNT    0x0500
#include <Sddl.h>
#else
#include <Sddl.h>
#endif

#pragma comment(lib, "Advapi32")

class FileProcess
{
public:
    FileProcess(){}
    ~FileProcess(){}

    static std::wstring ReplaceSubStr(std::wstring& strIn, LPCWSTR pwzSub, LPCWSTR pwzNew)
    {
        std::wstring::size_type stPos;
        std::wstring strSub = pwzSub;

        stPos = strIn.find(strSub);
        while(stPos != std::wstring::npos)
        {
            strIn.replace(stPos, strSub.size(), pwzNew);
            stPos = strIn.find(strSub, stPos);
        }

        return strIn;
    }
    static std::wstring ComposeFolderNameWithTimeStamp(LPCWSTR pwzUserName)
    {
        // Replace L" " with L"_"
        std::wstring strFolderName = pwzUserName;
        ReplaceSubStr(strFolderName, L" ", L"_");
        strFolderName += L"_";

        SYSTEMTIME  st;
        GetLocalTime(&st);

        // LocalTime: 20080101101010xxxx
        WCHAR wzLocalTime[19]; memset(wzLocalTime, 0, sizeof(wzLocalTime));
        _snwprintf_s(wzLocalTime, 19, _TRUNCATE, L"%04d%02d%02d%02d%02d%02d%04d", st.wYear, st.wMonth, st.wDay,
            st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);

        strFolderName += wzLocalTime;
        return strFolderName;
    }
	// add by toxyboy for copy file to another path,if two file have the same name ,add [1] at the end
	static bool CopyFileToDest(LPCWSTR wstrSrc,LPCWSTR wstrDest)
	{
		// dest path,exclude the file name
		bool bExist=false;
		int	 nNumb = 0;
		CFileFind finder;
		std::wstring strDestPath = wstrDest;
		size_t nPos = strDestPath.find_last_of(L"\\");
		std::wstring strFileName = strDestPath.substr(nPos+1,strDestPath.length()-nPos-1);
		strDestPath = strDestPath.substr(0,nPos+1);

		std::wstring strfindpath = strDestPath;
		strfindpath += L"*.*";
		BOOL bFind = finder.FindFile(strfindpath.c_str());
		if(!bFind)
		{
			finder.Close();
			return false;
		}
		while(bFind)
		{
			bFind = finder.FindNextFile();
			if(!finder.IsDots() && !finder.IsDirectory())
			{
				CString strTempFile = finder.GetFileName();
				if(strTempFile.CompareNoCase(strFileName.c_str())==0)
				{
					strDestPath += finder.GetFileTitle();
					std::wstring strNewFile = strDestPath;
					strNewFile += L"[1]";
					strNewFile += strTempFile.Right(4);
					MoveFile(finder.GetFilePath(),strNewFile.c_str());
					strDestPath += L"[2]";
					strDestPath += strTempFile.Right(4);
					bExist = true;
					break;
				}
				if(strTempFile.GetLength() < 7)	continue;
				int nLeft = strTempFile.Find(L'[');
				int nRight = strTempFile.Find(L']');
				if(nRight< nLeft || nLeft == -1 || 
					nRight == -1)	continue;

				CString strTempFileName = strTempFile.Left(nLeft);
				strTempFileName += strTempFile.Right(4);

				CString strMid = strTempFile.Mid(nLeft+1,nRight-nLeft-1);
				if(strTempFileName.CompareNoCase(strFileName.c_str())==0)
				{
					int nTemp = _wtoi(strMid);
					if(nTemp>10000||nTemp<1)	continue;
					if(nTemp>nNumb)	nNumb = nTemp;
					bExist = true;
				}
			}
		}

		if(bExist)
		{
			if(nNumb != 0)
			{
				strDestPath += strFileName.substr(0,strFileName.length()-4);
				strDestPath += L"[";
				CString strNum ;
				strNum.Format(L"%d",++nNumb);
				strDestPath += strNum;
				strDestPath += L"]";
				strDestPath += strFileName.substr(strFileName.length()-4,4);			
			}
		}
		else
		{
			strDestPath = wstrDest;
		}
#ifdef _DEBUG
		std::wstring strMsg = L"Copy source file :";
		strMsg += wstrSrc;
		strMsg += L" to ";
		strMsg += strDestPath;
		MessageBox(NULL,strMsg.c_str(),L"Debug Msg",MB_OK);
#endif

		DWORD   dwPermissions = 0;
		YLIB::AccessPermissionList apl;

		dwPermissions = GENERIC_ALL; //GENERIC_READ|GENERIC_WRITE;
		// add ACL information for current user to copy file into there
		apl.push_back(YLIB::COMMON::smart_ptr<YLIB::AccessPermission>(new YLIB::AccessPermission(dwPermissions)));
		dwPermissions = GENERIC_READ;

		// add ACL information for manager User read file
		for(size_t n=0;n<g_BaseArgument.approverVector.size();n++)
			apl.push_back(YLIB::COMMON::smart_ptr<YLIB::AccessPermission>(new YLIB::AccessPermission(g_BaseArgument.approverVector[n].second.c_str(),dwPermissions)));
		// add ACL information for request user read file from there
		apl.push_back(YLIB::COMMON::smart_ptr<YLIB::AccessPermission>(new YLIB::AccessPermission(g_BaseArgument.wstrCurSID.c_str(), dwPermissions)));

		// Create security attributes object
		YLIB::SecurityAttributesObject sa;
		sa.put_SecurityAttributes(apl);
		if(!YLIB::SecurityUtility::SecureCopyW(wstrSrc, strDestPath.c_str(), sa.get_SecurityAttributes(), FALSE))	return false;
		return true;
	}
};

#endif
