#include <windows.h>
#include "AdobeXI.h"
#include <vector>
#include "strsafe.h"
#include "boost\algorithm\string.hpp"
 
const char* ACTION_SEND = "SEND";
const char* ACTION_CONVERT = "CONVERT";
const char* DEST_FILE = "https://www.adobe.com";  
const wchar_t* TITLE_SAVETO_ACROBAT = L"Save &To Acrobat.com...";
const wchar_t* SEND_MAIL_PATH = L"plug_ins\\SendMail.api";
const wchar_t* CONTROL_CLASS_NAME = L"ToolbarWindow32";
const wchar_t* CONTROL_CAPTION = L"Save to Online Account";
const wchar_t* WINDOW_TITLE = L"Save As";
const char* FUNCTION_NAME = "SendExec";


namespace AdobeXI
{
	_AVMenuAddMenuItem  CAdobeXITool::next_AVMenuAddMenuItem = NULL;
	_SendMail CAdobeXITool::next_SendMail = NULL;
	map<string,FILESTOREINFOR> CAdobeXITool::m_mapFileRight;
    string CAdobeXITool::m_CurrentSaveAsPath = "";

	bool CAdobeXITool::DoActionOnLine(_In_ const string& strAction,_In_ CENoiseLevel_t EmNosie,_In_ const string& strFilePath, _In_ PDDoc pdDoc)
	{
		string output_file;
		string strUpperAction = strAction;
		std::transform(strUpperAction.begin(), strUpperAction.end(), strUpperAction.begin(), towupper);
		if (strFilePath.empty())
		{
			bool bRet = getCurrentPDFPath(output_file);
			if (!bRet)
			{
				return false;
			}
		}
		else
		{
			output_file = strFilePath;
		}
		
		CPolicy* ins_policy = CPolicy::GetInstance();
		bool bdeny = false;		
		vector<pair<wstring,wstring>> w_obligation_tags_dest;				
		if (boost::algorithm::iequals(ACTION_SEND,strUpperAction.c_str()) || boost::algorithm::iequals(ACTION_CONVERT,strUpperAction.c_str()))
		{
			if (pdDoc != NULL && (boost::algorithm::istarts_with(output_file, "http://") || boost::algorithm::istarts_with(output_file, "https://"))) 
			{
				ins_policy->queryHttpSource(output_file.c_str(),pdDoc,bdeny,strUpperAction,false,false,EmNosie);
			}
			else if (boost::algorithm::istarts_with(output_file,"Acrobat.com:"))
			{
				ins_policy->QueryAcrobatCom(strUpperAction,output_file,bdeny,pdDoc,EmNosie);
			}
			else
			{
				ins_policy->queryLocalSourceAndDoTagObligation(output_file.c_str(),bdeny,strUpperAction,NULL,EmNosie, true);
			}
			
		}
	
		return !bdeny;
	}

	
	void CAdobeXITool::myAVMenuAddMenuItem(AVMenu menu, AVMenuItem menuItem, AVMenuIndex menuItemIndex)
	{
		ASText title = ASTextNew();
		AVMenuItemGetTitleAsASText( menuItem, title);
		if (title != NULL)
		{
			std::wstring wstrTitle = reinterpret_cast<const wchar_t*>(ASTextGetUnicode(title));	
			if (_wcsicmp(wstrTitle.c_str(),TITLE_SAVETO_ACROBAT) == 0)
			{
				AVMenuItemSetComputeEnabledProc(menuItem, ASCallbackCreateProto(AVComputeEnabledProc, MenuSaveToAcrobatEnabled), NULL);
			}
		}
		ASTextDestroy(title);

		return next_AVMenuAddMenuItem(menu,menuItem,menuItemIndex);
	}

	void CAdobeXITool::SetFileRightOnlineToCache(_In_ const string& strFilePath,_In_ DWORD dwRight,_In_ PDDoc pDoc)
	{
		SWRLock lock;
		AutoSWRLocker write(lock, true);
		FILESTOREINFOR stInfo;
		stInfo.dwRight = dwRight;
		stInfo.pPDFDoc = pDoc;

		m_mapFileRight[strFilePath] = stInfo;

	}
	bool CAdobeXITool::GetFileRightOnlineFromCache(_In_ const string& strFilePath,_Out_ DWORD& dwRight)
	{
		SWRLock lock;
		AutoSWRLocker Read(lock, false);
		dwRight = 0;
		map<string,FILESTOREINFOR>::iterator itor = m_mapFileRight.find(strFilePath);
		if (itor != m_mapFileRight.end())
		{
			dwRight = itor->second.dwRight;
			return true;
		}
		return false;
	}

	PDDoc CAdobeXITool::GetCurrentFilePDDoc(_In_ const string& strFilePath)
	{
		PDDoc pDoc = NULL;
		if (boost::algorithm::istarts_with(strFilePath, "http://") || boost::algorithm::istarts_with(strFilePath, "https://")) 
		{
			SWRLock lock;
			AutoSWRLocker Read(lock, false);
			map<string,FILESTOREINFOR>::iterator itor = m_mapFileRight.find(strFilePath);
			if (itor != m_mapFileRight.end())
			{
				pDoc = itor->second.pPDFDoc;
			}
		}
		return pDoc;
	}

	void CAdobeXITool::DelAllFileRightOnline()
	{
		SWRLock lock;
		AutoSWRLocker Read(lock, false);
		m_mapFileRight.clear();
	}

	 bool CAdobeXITool::IsAllowActionOnline(_In_ const string& strFilePath, _In_ string& strAction, _Out_ bool& bAllow)
	 {
		 bAllow = false;
		 DWORD dwRightValue = 0;
		 bool bFind = GetFileRightOnlineFromCache(strFilePath,dwRightValue);
		 if (bFind)
		 { 
			 if (boost::algorithm::iequals(strAction.c_str(),ACTION_SEND))
			 {
				 if (dwRightValue&ADOBEXI_SEND_RIGHT)
				 {
					 bAllow = true;
				 }
			 }
			 else if (boost::algorithm::iequals(strAction.c_str(),ACTION_CONVERT))
			 {
				 if (dwRightValue&ADOBEXI_CONVERT_RIGHT)
				 { 
					 bAllow = true;
				 }
			 }
		 }
		 else  
		 {			
			 return false;
		 }
		 return true;
	 }

	 void CAdobeXITool::IsExistOneDenyActionPath(_In_ string& strAction,_Out_ bool& bAllow,_Out_ string& strFilePath,_Out_ PDDoc &pDoc)
	 {
		 SWRLock lock;
		 AutoSWRLocker Read(lock, false);
		 bAllow = true;
		 strFilePath = "";
		 pDoc = NULL;
		 if (boost::algorithm::iequals(strAction.c_str(),ACTION_SEND))
		 {
			 map<string,FILESTOREINFOR>::iterator itor = m_mapFileRight.begin();
			 for (; itor != m_mapFileRight.end(); ++itor)
			 {

				if(!(itor->second.dwRight & ADOBEXI_SEND_RIGHT))
				{
					strFilePath = itor->first;
					pDoc = itor->second.pPDFDoc;
				
					bAllow = false;
					break;
				}
			 }
		 }
		 else if (boost::algorithm::iequals(strAction.c_str(),ACTION_CONVERT))
		 {
			 map<string,FILESTOREINFOR>::iterator itor = m_mapFileRight.begin();
			 for (; itor != m_mapFileRight.end(); ++itor)
			 {
				 
				 if(!(itor->second.dwRight & ADOBEXI_CONVERT_RIGHT))
				 {
					 strFilePath = itor->first;
					 pDoc = itor->second.pPDFDoc;
					 bAllow = false;
					 break;
				 }
			 }
		 }
		
	 }
	 
	 DWORD CAdobeXITool::GetRightValueOnline(_In_ string& strAction,_In_ bool bAllow)
	 { 
		 if (boost::algorithm::iequals(strAction.c_str(),ACTION_SEND) && bAllow)
		 {
			return ADOBEXI_SEND_RIGHT;
		 }
		 else if (boost::algorithm::iequals(strAction.c_str(),ACTION_CONVERT) && bAllow)
		 {
			return ADOBEXI_CONVERT_RIGHT;
		 }

		 return 0;
	 }

	 void CAdobeXITool::SetFileOnLineRightValue(_In_ const string& strFilePath, _In_ DWORD dwSend, _In_ DWORD dwConvert,_In_ PDDoc pDdoc)
	 {
		 DWORD dwRightValue = dwSend + dwConvert;
		 SetFileRightOnlineToCache(strFilePath,dwRightValue,pDdoc);
	 }

	 ACCB1 ASBool ACCB2 CAdobeXITool::MenuSaveToAcrobatEnabled(void  *data)
	 {	
		 string output_file;
		 bool bAllow = true;
		 bool bRet = getCurrentPDFPath(output_file);
		 string strAction = ACTION_SEND;
		 if (bRet)
		 {
			 bool bSuc = IsAllowActionOnline(output_file,strAction, bAllow);
			 if (!bSuc)
			 {
				bAllow = true;
			 }
		 }
		 else
		 {	
			 string strFilePath;
			 PDDoc pDoc;
			 IsExistOneDenyActionPath(strAction, bAllow,strFilePath,pDoc);
		 }	

		 return bAllow;
	 }

	 void CAdobeXITool::QueryFileOnlineRight(_In_ const string& strFilePath,_In_ PDDoc pdDoc)
	 {
		 bool bSendAllow = DoActionOnLine(ACTION_SEND,CE_NOISE_LEVEL_MAX,strFilePath,pdDoc);
		 string strSendAction = ACTION_SEND;
		 DWORD dwSendRightValue = 0;
		 dwSendRightValue = GetRightValueOnline(strSendAction,bSendAllow);

		 bool bConvertAllow = DoActionOnLine(ACTION_CONVERT,CE_NOISE_LEVEL_MAX,strFilePath,pdDoc);
		 string strConvertAction = ACTION_CONVERT;
		 DWORD dwConvertRightValue = 0;
		 dwConvertRightValue = GetRightValueOnline(strConvertAction,bConvertAllow);

		 SetFileOnLineRightValue(strFilePath, dwSendRightValue, dwConvertRightValue,pdDoc);
	 }


	 
	 char* CAdobeXITool::CheckEmailFuncAddress()
	 {
		 static const int nscSendExecOffset = 0x84;
		 static const int nscMaxOffset = 0x200;
		 BYTE pFuncBin[16] = { 0x55,0x8B,0xEC,0xA1,0x84,0xFA,0x91,0x2A,0x56,0x6A,0x00,0x68,0x18,0x4D,0x8D,0x2A};
		 wchar_t szModule[512] = { 0 };
		 GetModuleFileName(NULL, szModule, 512);
		 wchar_t* pPos = wcsrchr(szModule, L'\\');
		 if (pPos == NULL)   return NULL;
		 pPos[1] = L'\0';
		 wcscat_s(szModule, SEND_MAIL_PATH);
		 HMODULE hLib = LoadLibraryW(szModule);
		 if (hLib != NULL)
		 {
			 char* pAddress = (char*)GetProcAddress(hLib, FUNCTION_NAME);
			 if (pAddress != NULL && pAddress > (char*)nscMaxOffset)
			 {
				 char *pFunc = pAddress - nscSendExecOffset;
				 while (pFunc + 0x100 >= pAddress)
				 {
					 if (memcmp(pFunc, pFuncBin, 4) == 0 && memcmp(pFunc+8,pFuncBin+8,4)==0)
					 {
						 return pFunc;
					 }
					 pFunc--;
				 }
			 }
		 }
		 return NULL;
	 }
	 void CAdobeXITool::mySendMail(DWORD dwP1, DWORD dwP2)
	 {
		 string output_file;
		 bool bAllow = true;
		 bool bRet = getCurrentPDFPath(output_file);
		 string strAction = ACTION_SEND;
		 PDDoc pDoc = NULL;
		 if (bRet)
		 {
	
			 pDoc = GetCurrentFilePDDoc(output_file);
			 bAllow = DoActionOnLine(strAction,CE_NOISE_LEVEL_USER_ACTION,output_file, pDoc);
		 }
		 else
		 {
			 AdobeXI::CAdobeXITool::IsExistOneDenyActionPath(strAction, bAllow,output_file,pDoc);
			 if (!bAllow)
			 {
				  bAllow = DoActionOnLine(strAction,CE_NOISE_LEVEL_USER_ACTION,output_file,pDoc);
			 }
			
		 }
	
		 if (bAllow)
		 { 
			 return next_SendMail(dwP1, dwP2);
		 }
	 } 


     bool CAdobeXITool::IsGreyOnlineWndOnSaveAsDlg()
     {
         bool bAllow = true;
         string strCurrentPath = GetCurrentSaveAsPath();
         string strAction = ACTION_SEND;
         if (!strCurrentPath.empty())
         {
             bool bSuc = IsAllowActionOnline(strCurrentPath,strAction, bAllow);
             if (!bSuc)
             {
                 bAllow = true;
             }
         }
         else
         {
			string strFilePath; 
			PDDoc pDoc;
            IsExistOneDenyActionPath(strAction, bAllow,strFilePath,pDoc);
         }

         return !bAllow;
     }

     void CAdobeXITool::SetCurrentSaveAsPath(_In_ const string& strFilePath)
     {
         m_CurrentSaveAsPath = strFilePath;
     }

     string CAdobeXITool::GetCurrentSaveAsPath()
     {
         return m_CurrentSaveAsPath;
     }


	void CAdobeXITool::GetCurrentUserID(char **UserSID)
	{
		HANDLE hToken;
		HANDLE hCurrentProcess = GetCurrentProcess();
		if (::OpenProcessToken(hCurrentProcess, TOKEN_READ, &hToken))
		{
			DWORD dwLen = NULL;
			//Pass NULL to get the right buffer size
			::GetTokenInformation(hToken, TokenUser, NULL, 0, &dwLen);
			if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
			{
				TOKEN_USER* pTokenUser = (PTOKEN_USER)(new char[dwLen]);
				if (NULL != pTokenUser)
				{
					if (::GetTokenInformation(hToken, TokenUser, pTokenUser, dwLen, &dwLen))
					{
						::ConvertSidToStringSidA(pTokenUser->User.Sid, UserSID);
					}
					delete []((char *)pTokenUser);
				}
			}
			CloseHandle(hToken);
		}
		CloseHandle(hCurrentProcess);
	}

	bool CAdobeXITool::IsiProtectedViewValueGreaterThanZero(AVTVersionNumPart major, bool bAcrobat)
	{
		bool bRet = false;

		char *pUserId = NULL;
		GetCurrentUserID(&pUserId);
		if (NULL == pUserId)
		{
			return bRet;
		}

		char kpchAdobeFourProcessFlagKey[1024] = { 0 };
		if (bAcrobat)
		{
			StringCchPrintfA(kpchAdobeFourProcessFlagKey, 1024, "%s\\Software\\Adobe\\Adobe Acrobat\\%d.0\\TrustManager", pUserId, major);
		}
		else
		{
			StringCchPrintfA(kpchAdobeFourProcessFlagKey, 1024, "%s\\Software\\Adobe\\Acrobat Reader\\%d.0\\TrustManager", pUserId, major);
		}

		static const char* kpwchiProtectedView = "iProtectedView"; /**< 0: didn't show message , just open it.  */

		HKEY hkey = NULL;		
		LONG ret = RegOpenKeyExA(HKEY_USERS, kpchAdobeFourProcessFlagKey, 0,KEY_QUERY_VALUE, &hkey);
		if (ERROR_SUCCESS == ret)
		{
			DWORD dwDataSize = sizeof(DWORD);
			DWORD value = 0;
			ret = RegQueryValueExA(hkey, kpwchiProtectedView, NULL, NULL, reinterpret_cast<LPBYTE>(&value), &dwDataSize);
			if (ret == ERROR_SUCCESS)
			{
				if(value > 0)
					bRet = true;
			}
			RegCloseKey(hkey);
			hkey = NULL;
		}
		return bRet;

	}

	 SWRLock::SWRLock(void)
	 { 
		 InitializeSRWLock(&lock_); 
	 }
	 SWRLock::~SWRLock()
	 {

	 }
	 void SWRLock::Lock(bool write)
	 {
		if (write)
		{
			::AcquireSRWLockExclusive(&lock_);
		}
		else
		{
			::AcquireSRWLockShared(&lock_);
		}
	 }

	 void SWRLock::Unlock(bool write)
	 {
		if (write)
		{
			::ReleaseSRWLockExclusive(&lock_);
		}
		else
		{
			::ReleaseSRWLockShared(&lock_);
		}
	 }

	 AutoSWRLocker::AutoSWRLocker(SWRLock& lock, bool write) :lock_(lock), write_(write)
	 {
		 lock_.Lock(write_);
	 }
	 AutoSWRLocker::~AutoSWRLocker()
	 {
		 lock_.Unlock(write_);
	 }
}
