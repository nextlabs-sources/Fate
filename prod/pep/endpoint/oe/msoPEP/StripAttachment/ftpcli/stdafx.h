// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

// Modify the following defines if you have to target a platform prior to the ones specified below.
// Refer to MSDN for the latest info on corresponding values for different platforms.
#ifndef WINVER				// Allow use of features specific to Windows XP or later.
#define WINVER 0x0501		// Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target other versions of Windows.
#endif						

#ifndef _WIN32_WINDOWS		// Allow use of features specific to Windows 98 or later.
#define _WIN32_WINDOWS 0x0410 // Change this to the appropriate value to target Windows Me or later.
#endif

#ifndef _WIN32_IE			// Allow use of features specific to IE 6.0 or later.
#define _WIN32_IE 0x0600	// Change this to the appropriate value to target other versions of IE.
#endif

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// some CString constructors will be explicit

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers
#endif

#include <afx.h>
#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <iostream>
// Windows Header Files:
//#include <windows.h>


#include <string>
#include <vector>
#include "log.h"
#include "./YLIB/security.h"
#include <afx.h>
#include <afxinet.h>
#include "ftpcli.h"
class FTPClient
{
	CInternetSession*	m_pSession;
	CFtpConnection*		m_pConnection;
	FTPClient(){};
public:
	
	FTPClient(const WCHAR*wstrFTPServer, const WCHAR*wstrUserName,const WCHAR* wstrPasswd, int nFTPPort)
	{
		m_strFTPServer=wstrFTPServer;
		m_strUserName=wstrUserName;
		m_strPasswd=wstrPasswd;
		m_iPort=nFTPPort;
		m_pConnection=NULL;
		m_pSession=NULL;
		m_dwFlag=0;
		/*std::wstring strFTPServerTemp=wstrFTPServer;
		std::wstring strServer,strPort,strPath;
		m_strFTPServer=L"10.1.2.3:22";ParseFullFTPPath(strServer,strPort,strPath);
		m_strFTPServer=L"10.1.2.3";ParseFullFTPPath(strServer,strPort,strPath);
		m_strFTPServer=L"10.1.2.3:22/path";ParseFullFTPPath(strServer,strPort,strPath);
		m_strFTPServer=L"10.1.2.3/path";ParseFullFTPPath(strServer,strPort,strPath);
		m_strFTPServer=L"10.1.2.3:22\\path";ParseFullFTPPath(strServer,strPort,strPath);
		m_strFTPServer=L"10.1.2.3\\path";ParseFullFTPPath(strServer,strPort,strPath);
		m_strFTPServer=strFTPServerTemp;*/
		ParseFullFTPPath(m_strServer,m_strPort,m_strPath);
		OutputDebugString(L"After ParseFullFTPPath.....");
		OutputDebugString(m_strServer.c_str());
		OutputDebugString(m_strPath.c_str());
	}
	~FTPClient()
	{
		if(m_pConnection)
		{
			m_pConnection->Close();
			m_pConnection=NULL;
		}
		if(m_pSession)
			delete m_pSession;
	}
	void SetLocalFile(const WCHAR*pwzLocalFile){m_strLocalFile=pwzLocalFile;}
	void SetRemoteFile(const WCHAR*pwzRemoteFile){m_strRemoteFile=pwzRemoteFile;}
	void SetFlag(DWORD dwFlag){m_dwFlag=dwFlag;};
	void SetRequesterSid(const WCHAR*pwzRequesterSid){m_strRequesterSid=pwzRequesterSid;}
	bool Connect()
	{
		if(m_pSession==NULL)
		{
			std::wstring wstrAppAgent=L"EmailApproval";
			m_pSession=new CInternetSession(wstrAppAgent.c_str());
		}
		if(m_pSession==NULL)
		{
			DP((L"Failed to new CInternetSession object!"));
			return false;
		}
		m_pSession->SetOption(INTERNET_OPTION_CONNECT_TIMEOUT,10*1000);
		try
		{
			if(NULL==m_pConnection)
			{
				DP((L"Connect FTP server [%s] with %s/%s",m_strServer.c_str(),m_strUserName.c_str(),m_strPasswd.c_str()));
                // for bug 28596, Set bPassive true that customer active connect to ftp server.
                // Even if customer firewall on or off, it can work fine with this way.
				m_pConnection=m_pSession->GetFtpConnection(m_strServer.c_str(),m_strUserName.c_str(),m_strPasswd.c_str(),m_iPort, TRUE);
			}
		}
		catch(CInternetException *pEx)
		{
			WCHAR wzError[1024]=L"";
			if(pEx->GetErrorMessage(wzError,1024))
			{
				std::wstring wstrErr=L"Connect FTP server ";
				wstrErr+=m_strServer;
				wstrErr+=L" error.";
				wstrErr+=wzError;
				AfxMessageBox(wstrErr.c_str());
			}
			else
				AfxMessageBox(L"There was an exception when connect ftp server");
		}
		if(NULL==m_pConnection)
		{
			DP((L"Failed to new CFtpConnection object!"));
			return false;
		}
		return true;
	}
	BOOL GetAnNameUnexisting(std::wstring& origName,std::wstring&newName,int index)
	{
		if(origName.length()==0)
			return FALSE;
		if(index==0)
		{
			newName=origName;
			return TRUE;
		}
		WCHAR wzTemp[32]=L"";
		wsprintf(wzTemp,L"(%d)",index);
		std::wstring::size_type pos=origName.rfind(L".zip");
		if(pos==std::wstring::npos)
		{
			pos=origName.rfind(L".");
			if(pos==std::wstring::npos)
			{
				newName=origName;
				newName+=wzTemp;
			}
			else
			{
				std::wstring strExt=origName.substr(pos);
				newName=origName.substr(0,pos);
				newName+=wzTemp;
				newName+=strExt;
			}
		}
		else
		{
			std::wstring strTempOrigName=origName.substr(0,pos);
			pos=strTempOrigName.rfind(L".");
			if(pos==std::wstring::npos)
			{
				newName=strTempOrigName;
				newName+=wzTemp;
				newName+=L".zip";
			}
			else
			{
				std::wstring strExt=strTempOrigName.substr(pos);
				newName=strTempOrigName.substr(0,pos);
				newName+=wzTemp;
				newName+=strExt;
				newName+=L".zip";
			}
		}
		return TRUE;
	}
	bool Upload(WCHAR *wzRemoteFile)
	{
		if(_waccess(m_strLocalFile.c_str(),0))
			return false;
		bool bRet=true;
		//m_strLocalFile=L"C:/Jim/test.txt";
		// a. Prepare security
		
		/*DWORD                       dwPermissions = 0;
		YLIB::AccessPermissionList  apl;
		dwPermissions = GENERIC_ALL|STANDARD_RIGHTS_ALL|SPECIFIC_RIGHTS_ALL;
		apl.push_back(YLIB::COMMON::smart_ptr<YLIB::AccessPermission>(new YLIB::AccessPermission(dwPermissions)));
		dwPermissions = GENERIC_READ|GENERIC_WRITE|STANDARD_RIGHTS_ALL|SPECIFIC_RIGHTS_ALL;
		apl.push_back(YLIB::COMMON::smart_ptr<YLIB::AccessPermission>(new YLIB::AccessPermission(m_strRequesterSid.c_str(), dwPermissions)));
		YLIB::SecurityAttributesObject sa;
		if(!sa.put_SecurityAttributes(apl))
		{
			DP((L"Fail to put_SecurityAttributes\n!"));
		}*/

		//if(YLIB::SecurityUtility::SecureCopyW(m_strLocalFile.c_str(), L"C:/test.txt", sa.get_SecurityAttributes(), FALSE))
		std::wstring strRemoteFile=m_strLocalFile;
		std::wstring strRemoteFullPathLink=L"ftp://";
		strRemoteFullPathLink+=m_strUserName;
		strRemoteFullPathLink+=L":";
		strRemoteFullPathLink+=m_strPasswd;
		strRemoteFullPathLink+=L"@";
		strRemoteFullPathLink+=m_strServer;
		std::wstring::size_type iPos=m_strLocalFile.rfind(L"/");
		if(iPos!=-1)
				strRemoteFile=m_strLocalFile.substr(iPos+1);
		else
		{
			iPos=m_strLocalFile.rfind(L"\\");
			if(iPos!=-1)
				strRemoteFile=m_strLocalFile.substr(iPos+1);
		}
		try
		{
			strRemoteFullPathLink+=L"/";
			if(m_strPath.length())
			{
				strRemoteFullPathLink+=m_strPath;
				strRemoteFullPathLink+=L"/";
				DP((L"server path:%s",m_strPath.c_str()));
				bRet=m_pConnection->SetCurrentDirectory(m_strPath.c_str());
				
				if(bRet==0)
				{
					DP((L"Failed to Set current directory for FTP server to %s",m_strPath.c_str()));
					return false;
				}
			}
			//strRemoteFullPathLink+=strRemoteFile;
			/*std::wstring strNewLocal=L"C:\\obligations\\approver\\temp\\";
			strNewLocal+=strRemoteFile;
			CopyFile(m_strLocalFile.c_str(),strNewLocal.c_str(),false);*/
			DP((L"Begin to upload %s to %s.",m_strLocalFile.c_str(),strRemoteFile.c_str()));
			if(m_dwFlag&FTPCLI_RENAME_APPENDNUMBER)
			{
				{
					std::wstring strNew;

					std::wstring strTestOrig=L"a.doc";
					GetAnNameUnexisting(strTestOrig,strNew,10);

					strTestOrig=L"a.doc.zip";
					GetAnNameUnexisting(strTestOrig,strNew,10);

					strNew=L"";

				}
				CFtpFileFind finder(m_pConnection);
				BOOL bWorking=TRUE;
				int index=0;
				std::wstring strOrigName=strRemoteFile;
				while(bWorking)
				{
					GetAnNameUnexisting(strOrigName,strRemoteFile,index++);
					bWorking=finder.FindFile(strRemoteFile.c_str());
				}
			}
			strRemoteFullPathLink+=strRemoteFile;
			bRet=m_pConnection->PutFile(m_strLocalFile.c_str(),strRemoteFile.c_str());
			if(bRet)
			{
				DP((L"succeeded to upload[%s]!",m_strLocalFile.c_str()));
			}
			else
			{
				DP((L"failed to upload[%s][%s]!",m_strLocalFile.c_str(),strRemoteFile.c_str()));
			}
			
		}
		catch(CInternetException *pEx)
		{
			WCHAR wzError[1024]=L"";
			if(pEx->GetErrorMessage(wzError,1024))
			{
				std::wstring wstrErr=L"Connect FTP server ";
				wstrErr+=m_strServer;
				wstrErr+=L" error.";
				wstrErr+=wzError;
				AfxMessageBox(wstrErr.c_str());
			}
			else
				AfxMessageBox(L"There was an exception when upload file to ftp server.");
		}
		if(bRet)
		{
			wcscpy(wzRemoteFile,strRemoteFullPathLink.c_str());
			DP((L"The remote file is %s",wzRemoteFile));
		}
		DP((L"Return from FtpClient::Upload"));
		return bRet;
	}
protected:
	void ParseFullFTPPath(std::wstring &strServer,std::wstring & strPort,std::wstring& strPath)
	{
		/*WCHAR seps[]=L":/\\";
		WCHAR *token = wcstok(m_strFTPServer.c_str(),seps);
		while(token !=NULL)
		{
			token = wcstok(NULL,seps);
		}*/
		std::wstring::size_type iPosBegin=0,iPosEnd=0;
		
		iPosEnd=m_strFTPServer.find(L"/",0,1);
		if(iPosEnd!=-1)//  1.1.1.1:21/path   or 1.1.1.1/path
		{
			std::wstring str=m_strFTPServer.substr(iPosBegin,iPosEnd);
			strPath=m_strFTPServer.substr(iPosEnd+1);
			iPosEnd=str.find(L":",0,1);
			if(iPosEnd!=-1)//1.1.1.1:21/path
			{
				strServer=str.substr(0,iPosEnd);
				strPort=str.substr(iPosEnd+1);
			}
			else// 1.1.1.1/path
			{
				strServer=str;
				strPort=L"21";
			}
		}
		else
		{
			std::wstring str;
			iPosEnd=m_strFTPServer.find(L"\\",0,1);
			if(iPosEnd!=-1)//1.1.1.1:21\path or 1.1.1.1\path
			{
				str=m_strFTPServer.substr(iPosBegin,iPosEnd);
				strPath=m_strFTPServer.substr(iPosEnd+1);
				iPosEnd=str.find(L":",0,1);
				if(iPosEnd!=-1)// 1.1.1.1:21\path
				{
					strServer=str.substr(0,iPosEnd);
					strPort=str.substr(iPosEnd+1);
				}
				else//1.1.1.1\path
				{
					strServer=str;
					strPort=L"21";
				}
			}
			else//1.1.1.1:21 or 1.1.1.1
			{
				iPosEnd=m_strFTPServer.find(L":",0,1);
				if(iPosEnd!=-1)//1.1.1.1:21
				{
					strServer=m_strFTPServer.substr(0,iPosEnd);
					strPort=m_strFTPServer.substr(iPosEnd+1);
				}
				else//1.1.1.1
				{
					strServer=m_strFTPServer;
					strPort=L"21";
				}
			}
		}
	}
private:
	int m_iPort;
	DWORD m_dwFlag;
	std::wstring m_strFTPServer;
	std::wstring m_strServer;
	std::wstring m_strPort;
	std::wstring m_strPath;

	std::wstring m_strUserName;
	std::wstring m_strPasswd;
	std::wstring m_strLocalFile;
	std::wstring m_strRemoteFile;
	std::wstring m_strRequesterSid;

};

// TODO: reference additional headers your program requires here
