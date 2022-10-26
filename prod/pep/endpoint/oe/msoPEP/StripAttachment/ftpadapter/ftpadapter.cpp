// ftpadapter.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "ftpadapter.h"
#include <stdio.h>
#include "log.h"
#include "adaptercomm.h"
#include "celog.h"
#include "celog_policy_windbg.hpp"
#include "celog_policy_file.hpp"
#include "eframework/auto_disable/auto_disable.hpp"
#include "zipencryptor.h"
#include "boost\algorithm\string.hpp"
#include "MailUtility.h"
#include <process.h>


nextlabs::recursion_control mso_hook_control;
#ifdef _MANAGED
#pragma managed(push, off)
#endif
HINSTANCE g_hInstance;
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		{
		g_hInstance=hModule;
		break;
		}
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
    return TRUE;
}

#ifdef _MANAGED
#pragma managed(pop)
#endif

static void URICoding(std::wstring& strURI)
{
	boost::replace_all(strURI, L" ", L"%20");
}

static BOOL GetModuleBaseName(std::wstring& wstrModuleBaseName)
{
	WCHAR wzModuleFileName[MAX_PATH+1];memset(wzModuleFileName,0,sizeof(wzModuleFileName));
	DWORD dwRet=GetModuleFileName(g_hInstance,wzModuleFileName,MAX_PATH);
	
	if(dwRet==0||GetLastError() == ERROR_INSUFFICIENT_BUFFER)
		return S_FALSE;
	std::wstring wstrTemp=wzModuleFileName;
	std::wstring::size_type pos=wstrTemp.rfind(L'/');
	if(pos==std::wstring::npos)
	{
		pos=wstrTemp.rfind(L'\\');
		if(pos==std::wstring::npos)
			return S_FALSE;
	}
	
	wstrModuleBaseName=wstrTemp.substr(0,pos);
	return TRUE;
}

AdapterCommon::Adapter* GetAdapter()
{
	WCHAR wzModuleFileName[MAX_PATH+1];memset(wzModuleFileName,0,sizeof(wzModuleFileName));
	DWORD dwRet=GetModuleFileName(g_hInstance,wzModuleFileName,MAX_PATH);
	
	if(dwRet==0||GetLastError() == ERROR_INSUFFICIENT_BUFFER)
		return NULL;

	std::wstring wstrIniFile;
	if(GetModuleBaseName(wstrIniFile)==FALSE)
		return NULL;

	wstrIniFile +=L"\\";
	wstrIniFile +=FTPADAPTER_INI_FILENAME;
	return new AdapterCommon::Adapter(wstrIniFile.c_str(),wzModuleFileName,FTPADAPTER_OBLIGATION_NAME);
}
STDAPI DllRegisterServer(void)
{
	AdapterCommon::Adapter* pAdapter=GetAdapter();
	if(pAdapter==NULL)
		return S_FALSE;

	BOOL bRet=pAdapter->Register();

	delete pAdapter;
	if(bRet==FALSE)
		return S_FALSE;
	return S_OK;
}
/////////////////////////////////////////////////////

STDAPI DllUnregisterServer(void)
{
	AdapterCommon::Adapter* pAdapter=GetAdapter();
	if(pAdapter==NULL)
		return S_FALSE;

	BOOL bRet=pAdapter->UnRegister();

	delete pAdapter;
	if(bRet==FALSE)
		return S_FALSE;
	return S_OK;
}
BOOL FTPAdapter::UploadOne(AdapterCommon::Attachment *pAtt)
{
	if(fpFtpUploadEx==NULL)
	{
		DP((L"The function point of FtpUpload is null"));
		return FALSE;
	}
	WCHAR wzRemoteFile[1024];memset(wzRemoteFile,0,sizeof(wzRemoteFile));
	HRESULT hr=S_OK;
#ifdef _DEBUG
	{
		AdapterCommon::Obligation ftpOb;
		ftpOb.SetName(m_pAdapter->GetObligationName().c_str());
		ftpOb.AddAttribute(AdapterCommon::Attribute(this->OBLIGATION_ATTRNAME_FTPSERVER,L"lab01-srvu01"));
		ftpOb.AddAttribute(AdapterCommon::Attribute(this->OBLIGATION_ATTRNAME_FTPUSER,L"kaka"));
		ftpOb.AddAttribute(AdapterCommon::Attribute(this->OBLIGATION_ATTRNAME_FTPPASSWORD,L"123456"));
		ftpOb.AddAttribute(AdapterCommon::Attribute(this->OBLIGATION_ATTRNAME_LOCATION,OBLIGATION_ATTRVALUE_LOCATION_TOP));
		ftpOb.AddAttribute(AdapterCommon::Attribute(this->OBLIGATION_ATTRNAME_TEXT,OBLIGATION_ATTRVALUE_TEXT));
		ftpOb.AddAttribute(AdapterCommon::Attribute(this->OBLIGATION_ATTRNAME_LINKFORMAT,OBLIGATION_ATTRVALUE_LINKFORMAT_SHORT));
		pAtt->AddObligation(ftpOb);

	}
#endif
	std::wstring wstrFtpServer,wstrUser,wstrPasswd,wstrSrc=pAtt->GetTempPath();
	std::wstring wstrExpirationDate=L"",wstrFtpSite=L"",wstrUserTemplate=L"",wstrAdminPort=L"1100";
	WORD	wEFTServerPort = 1100;
	size_t iIndex=0,obCount=pAtt->Count();
	if(obCount==0)
		return TRUE;
	for(iIndex=0;iIndex<obCount;iIndex++)
	{
		hr=S_OK;
		AdapterCommon::Obligation ob=pAtt->Item((int)iIndex);	
		if(m_pAdapter->GetObligationName()==ob.GetName())
		{
			std::wstring strPasswd,strPasswdLine, strSeparatePwd;

			if(ob.FindAttribute(this->OBLIGATION_ATTRNAME_FTPSERVER,wstrFtpServer)==false)
				return FALSE;
			if(ob.FindAttribute(this->OBLIGATION_ATTRNAME_FTPUSER,wstrUser)==false)
				return FALSE;
			if(ob.FindAttribute(this->OBLIGATION_ATTRNAME_FTPPASSWORD,wstrPasswd)==false)
				return FALSE;

			// add by Tonny to check if upload to EFT ftp server
			ob.FindAttribute(this->OBLIGATION_ATTRNAME_EFTEXPIRATIONDATE,wstrExpirationDate);
			ob.FindAttribute(this->OBLIGATION_ATTRNAME_EFTFTPSITE,wstrFtpSite);
			ob.FindAttribute(this->OBLIGATION_ATTRNAME_EFTFTPUSERSETTINGSTEMPLATE,wstrUserTemplate);
			ob.FindAttribute(this->OBLIGATION_ATTRNAME_EFTADMINPORT,wstrAdminPort);

			ob.FindAttribute(this->OBLIGATION_SEPARATE_PASSWORD, strSeparatePwd);

			if(m_strErrorMsgForSender.empty() || m_strErrorMsgForRecipients.empty())
			{
				ob.FindAttribute(this->OBLIGATION_ERRORMESSAGE_SENDER, m_strErrorMsgForSender);
				ob.FindAttribute(this->OBLIGATION_ERRORMESSAGE_RECIPIENTS, m_strErrorMsgForRecipients);
			}

#if 1
			static wchar_t szlog[512]={0};
			wsprintfW(szlog,L"[In Ftp adapter module:] Admin Port is [%s],ExpirationDate is: [%s], Ftp Site is: [%s],User template is: [%s].......\n"
				,wstrAdminPort.c_str(),wstrExpirationDate.c_str(),wstrFtpSite.c_str(),wstrUserTemplate.c_str());
			OutputDebugString(szlog);
#endif

			bool bEFTUpload = false;
			if(!wstrFtpSite.empty() && !wstrUserTemplate.empty())	bEFTUpload = true;

			wstring strFTPLoginUserName;
			//hr = fpFtpUpload(wstrFtpServer.c_str(),wstrUser.c_str(),wstrPasswd.c_str(),21,wstrSrc.c_str(),wzRemoteFile);
			if(bEFTUpload)
			{
				if(wstrSrc.empty())	return FALSE;
				if(fpEFTUpload==NULL)
					fpEFTUpload=(FuncEFTFtpUpload)GetProcAddress(hFtpCli,"EFTFtpUpload");
#if 0
				wsprintfW(szlog,L"[In Ftp adapter module:] server is :[%s],user is:[%s],pwd is:[%s],Src is:[%s],ExpDate is: [%s], Ftp Site is: [%s],User template is: [%s].......\n"
					,wstrFtpServer.c_str(),wstrUser.c_str(),wstrPasswd.c_str(),wstrSrc.c_str(),
					wstrExpirationDate.c_str(),wstrFtpSite.c_str(),wstrUserTemplate.c_str());
				OutputDebugString(szlog);
#endif
				if(fpEFTUpload != NULL)
				{
					hr = E_FAIL;
					wEFTServerPort = _wtoi(wstrAdminPort.c_str());
					hr = fpEFTUpload(wstrFtpServer.c_str(),wstrUser.c_str(),wstrPasswd.c_str(),wEFTServerPort,wstrSrc.c_str(),wzRemoteFile,
						FTPCLI_RENAME_APPENDNUMBER,wstrExpirationDate.c_str(),wstrFtpSite.c_str(),wstrUserTemplate.c_str());

					if(hr!=S_OK)
					{//upload failed, 
						std::wstring::size_type nIndex = wstrSrc.rfind(L"\\");
						std::wstring strFileName;
						if( nIndex != std::wstring::npos)
							strFileName = wstrSrc.substr(nIndex + 1);
						else
							strFileName = wstrSrc;

						m_vFailedAttachments.push_back(strFileName);

						wchar_t buf[1024] = {0};
						swprintf_s(buf, 1024, L"FTP faild: %s\n", strFileName.c_str());
						OutputDebugString(buf);
					}

					//extract PASSWORD 
					if(!strSeparatePwd.empty() && wcsicmp(strSeparatePwd.c_str(), L"Yes") == 0)
					{
						std::wstring strRemote(wzRemoteFile);
						std::wstring::size_type nEnd = strRemote.find(L"@");
						
						if (nEnd != std::wstring::npos)
						{
							std::wstring::size_type nStart = strRemote.rfind(L":", nEnd);
							if (nStart != std::wstring::npos)
							{
								ATTACHMENTPARAM param;
								param.strPassword = strRemote.substr(nStart + 1, nEnd - nStart - 1);
								param.strRemotePath = strRemote.replace(nStart, nEnd - nStart, L"");

								std::wstring::size_type nIndex = wstrSrc.rfind(L"\\");
								if( nIndex != std::wstring::npos)
									param.strFileName = wstrSrc.substr(nIndex + 1);
								else
									param.strFileName = wstrSrc;

								m_vAttachments.push_back(param);

								//remove PASSWORD in link
								memset(wzRemoteFile,0,sizeof(wzRemoteFile));
								wcsncpy_s(wzRemoteFile, sizeof(wzRemoteFile) / sizeof(wchar_t), param.strRemotePath.c_str(), _TRUNCATE);

								//get FTP user name
								nEnd = nStart;
								nStart = strRemote.rfind(L"//", nEnd);

								if(nStart != wstring::npos)
								{
									std::wstring strUserName = strRemote.substr(nStart + 2, nEnd - nStart -2);

									wchar_t buf[1024] = {0};
									swprintf_s(buf, sizeof(buf) / sizeof(wchar_t) - 1, this->FTP_USERNAME, strUserName.c_str());
									strFTPLoginUserName = wstring(buf);
								}
							}
						}
					}
				}
			}
			else
			{
				CZipEncryptor zipEncryptor(g_hInstance);
				CPasswdGenerator passGen;
				strPasswd=passGen.Generator();
				
				WCHAR wzEncryptedFileName[MAX_PATH*2]=L"";
				if(zipEncryptor.Encrypt(wstrSrc.c_str(),wzEncryptedFileName,strPasswd.c_str())==S_OK)
				{
					hr=fpFtpUploadEx(wstrFtpServer.c_str(),
								wstrUser.c_str(),
								wstrPasswd.c_str(),
								21,
								wzEncryptedFileName,//ae.get_vecFiles()[i].c_str(),
								wzRemoteFile,
								FTPCLI_RENAME_APPENDNUMBER);
					strPasswdLine=L"(The password for the encrypted .zip file is \"";
					strPasswdLine+=strPasswd;
					strPasswdLine+=L"\")";
				}
				else
				{
					hr=fpFtpUploadEx(wstrFtpServer.c_str(),
								wstrUser.c_str(),
								wstrPasswd.c_str(),
								21,
								wstrSrc.c_str(),//ae.get_vecFiles()[i].c_str(),
								wzRemoteFile,
								FTPCLI_RENAME_APPENDNUMBER);
				}
				if(wzEncryptedFileName[0]!=0)
					DeleteFile(wzEncryptedFileName);
			}
			
			pAtt->SetStripFlag(true);

			if(hr!=S_OK)
				return FALSE;
			else
			{
				pAtt->SetReturnPath(wzRemoteFile);
				std::wstring	wstrLocation(OBLIGATION_ATTRVALUE_LOCATION_BOTTOM),
								wstrText(OBLIGATION_ATTRVALUE_TEXT),
								wstrLinkFormat(m_bIsHtml==true?OBLIGATION_ATTRVALUE_LINKFORMAT_LONG:OBLIGATION_ATTRVALUE_LINKFORMAT_SHORT);
				ob.FindAttribute(this->OBLIGATION_ATTRNAME_LOCATION,wstrLocation);
				ob.FindAttribute(this->OBLIGATION_ATTRNAME_TEXT,wstrText);
				ob.FindAttribute(this->OBLIGATION_ATTRNAME_LINKFORMAT,wstrLinkFormat);

				if(m_strPwdMailSubject.empty() || m_strPwdMailBody.empty())
				{
					ob.FindAttribute(this->OBLIGATION_ATTRNAME_PWD_SUBJECT, m_strPwdMailSubject);
					ob.FindAttribute(this->OBLIGATION_ATTRNAME_PWD_BODY, m_strPwdMailBody);
				}


				std::wstring wstrSrcNameOnly=pAtt->GetSrcPath();
				std::wstring::size_type pos=wstrSrcNameOnly.rfind(L"\\");
				if(pos != std::wstring::npos)
					wstrSrcNameOnly=wstrSrcNameOnly.substr(pos+1);
				else
				{
					pos=wstrSrc.rfind(L"/");
					if(pos != std::wstring::npos)
						wstrSrcNameOnly=wstrSrcNameOnly.substr(pos+1);
				}
				//Replace "[filename]" with the name of the file
				AdapterCommon::StringReplace(wstrText,FTPADAPTER_PLACEHOLDER_FILENAME,wstrSrcNameOnly);
				
				std::wstring wstrLink;

				if(this->IsHtmlBody()&&wstrLinkFormat==OBLIGATION_ATTRVALUE_LINKFORMAT_SHORT)
				{
					wstrLink  = L"<a href=\"";
					wstrLink += wzRemoteFile;
					wstrLink += L"\">";
					wstrLink += wstrSrcNameOnly;
					wstrLink += L"</a>";
				}
				else
				{
					std::wstring strURILink(wzRemoteFile);
					URICoding(strURILink);

					wstrLink  = L" ";//L"<";
					wstrLink += strURILink;
					wstrLink += L" " ;
					//wstrLink += L">";
				}

				if(!strFTPLoginUserName.empty())
				{
					wstrLink.append(L" ");
					wstrLink.append(strFTPLoginUserName);
				}

				//Replace "[link]" with the return remote path of FTP upload
				AdapterCommon::StringReplace(wstrText,FTPADAPTER_PLACEHOLDER_LINK,wstrLink);
				
				

				//std::wstring wstrBody;
				/*if(this->IsHtmlBody())
					hr=AdapterCommon::GetStringPropertyFromObject(m_pItem,L"HTMLBody",wstrBody);
				else
					hr=AdapterCommon::GetStringPropertyFromObject(m_pItem,L"Body",wstrBody);
				
				if(FAILED(hr))
					return TRUE;*/
				
				std::wstring wstrNewLine;
				if(this->IsHtmlBody())
					wstrNewLine=L"<br>";
				else
					wstrNewLine=L"\r\n";
				if(wstrLocation==OBLIGATION_ATTRVALUE_LOCATION_TOP)
				{
					//wstrNewBody += wstrBody;
					m_strTopMessageText+=wstrNewLine;
					m_strTopMessageText+=SEPARATOR_STRING;
					m_strTopMessageText+=wstrNewLine;
					m_strTopMessageText+= wstrText;
					m_strTopMessageText+=strPasswdLine;
					//m_strTopMessageText+=wstrNewBody;
				}
				else //Bottom
				{
					m_strBottomMessageText+=wstrNewLine;
					m_strBottomMessageText+=SEPARATOR_STRING;
					m_strBottomMessageText+=wstrNewLine;
					m_strBottomMessageText+= wstrText;
					m_strBottomMessageText+=strPasswdLine;

				}
				m_strTopMessageText+=wstrNewLine;
				m_strBottomMessageText+=wstrNewLine;

				/*if(this->IsHtmlBody())
					hr=AdapterCommon::PutStringPropertyToObject(m_pItem,L"HTMLBody",wstrNewBody.c_str());
				else
					hr=AdapterCommon::PutStringPropertyToObject(m_pItem,L"Body",wstrNewBody.c_str());
				if(FAILED(hr))
					return FALSE;
				CComVariant varResult;
				hr=AdapterCommon::AutoWrap(DISPATCH_METHOD,&varResult,m_pItem,L"Save",0);
				if(FAILED(hr))
					return FALSE;*/
				return TRUE;
			}
		}
		else
			continue;
		

	}
	
	return TRUE;
	
}
BOOL FTPAdapter::UploadAll()
{
	if(hFtpCli==NULL)
	{
		#ifndef _WIN64 
		hFtpCli=LoadLibrary(L"ftpcli32.dll");
		#else
		hFtpCli=LoadLibrary(L"ftpcli.dll");
		#endif
	}
	m_vAttachments.clear();
	m_strPwdMailSubject.clear();
	m_strPwdMailBody.clear();
	m_strErrorMsgForSender.clear();
	m_strErrorMsgForRecipients.clear();
	m_vFailedAttachments.clear();

	size_t iIndex=0,iCount=m_pAttachments->Count();
	BOOL   bFailure=FALSE;
	for(iIndex=0;iIndex<iCount;iIndex++)
	{
		AdapterCommon::Attachment& pAtt=(m_pAttachments->Item(iIndex));
		if(UploadOne(&pAtt)==FALSE)
		{
			DP((L"FTP Adapter:upload %s[%s] failed",pAtt.GetTempPath().c_str(),pAtt.GetSrcPath().c_str()));
			bFailure=TRUE;
		}
		
	}

	//Send a 2nd mail to user for PASSWORDs
	std::wstring strEnder = L"<br>";
	int nMailType = 0;
	if (!this->IsHtmlBody())
	{
		strEnder = L"\r\n";
		nMailType = 1;
	}

	if (m_vAttachments.size() > 0 )
	{
		std::wstring strBody;
		strBody.append(this->SEPARATOR_STRING);
		strBody.append(strEnder);
		

		//replace the subject
		std::wstring strSubject = CMailUtility::GetSubject(m_pItem);
		std::wstring::size_type nStart;
		while( (nStart = m_strPwdMailBody.find(this->OBLIGATION_SUBJECT_PATTERN)) != std::wstring::npos)
			m_strPwdMailBody = m_strPwdMailBody.replace(nStart, wcslen(this->OBLIGATION_SUBJECT_PATTERN), strSubject);

		//replace the [Return]
		while( (nStart = m_strPwdMailBody.find(this->OBLIGATION_RETURN)) != std::wstring::npos)
		{
			m_strPwdMailBody.replace(nStart, wcslen(this->OBLIGATION_RETURN), strEnder);
		}

		strBody.append(m_strPwdMailBody);

		strBody.append(strEnder);
		strBody.append(strEnder);

		
		for (std::vector<ATTACHMENTPARAM>::size_type i = 0; i < m_vAttachments.size(); i++)
		{
			const ATTACHMENTPARAM& param = m_vAttachments[i];
			
			wchar_t buf[1024] = {0};
			swprintf_s(buf, sizeof(buf)/sizeof(wchar_t) - 1, L"Document: %s%sPassword: %s%s%s", param.strFileName.c_str(), strEnder.c_str(), param.strPassword.c_str(), strEnder.c_str(), strEnder.c_str() );
			
			strBody.append(buf);
		}
		
		strBody.append(this->SEPARATOR_STRING);
		wstring strRecipients = CMailUtility::GetRecipients(m_pItem);
	//	DP((L"Try to send PASSWORD mail to: %s, body: %s\n", strRecipients.c_str(), strBody.c_str()));

		//Force to load ftpadapter.dll again. adaptermanager will free FtpAdapter, but we have a thread here to send "PASSWORD mail", so it will generate an exception if the DLL was free before thread ends
#ifndef _WIN64 
		HMODULE hFTPAdapter = GetModuleHandleW(L"ftpadapter32.dll");
#else
		HMODULE hFTPAdapter = GetModuleHandleW(L"ftpadapter.dll");
#endif
		wchar_t buf[MAX_PATH + 1] = {0};
		GetModuleFileNameW(hFTPAdapter, buf, MAX_PATH);

		if(wcslen(buf) > 0)
		{
			HMODULE h = LoadLibraryW(buf);
			DP((L"Loaded ftpadater again: %s, %x\r\n", buf, h));
			if (h)
			{
				//try to create a thread to send PASSWORD mail
				MAILPARAM* pParam = new MAILPARAM;
				pParam->strSubject = m_strPwdMailSubject;
				pParam->strBody = strBody;
				pParam->strTo = strRecipients;
				pParam->strCC = L"";
				pParam->nMailType = nMailType;

				CreateThread(NULL, 0, SendMailThread, (LPVOID)pParam, 0, NULL);
			}
			
		}

		

	//  CMailUtility::SendMail(m_strPwdMailSubject, strBody, strRecipients, L"", nMailType);
	}
	

	HRESULT hr=S_OK;
	std::wstring strMsg,wstrBody,wstrNewBody,strNewLine;
	std::wstring strSpanBegin,strSpanEnd;
	if(this->IsHtmlBody())
	{
		strNewLine=L"<br>";
		strSpanBegin=L"<span style='font-size:11.0pt;font-family:\"Calibri\"'>";
		strSpanEnd=L"</span>";
		hr=AdapterCommon::GetStringPropertyFromObject(m_pItem,L"HTMLBody",wstrBody);
	}
	else
	{
		strNewLine=L"\r\n";
		hr=AdapterCommon::GetStringPropertyFromObject(m_pItem,L"Body",wstrBody);
	}
	if(FAILED(hr))
		return TRUE;
	if(bFailure==TRUE)
	{
		HRESULT hr=S_OK;

		std::wstring strErrorMsg = L"One or more attachment(s) failed to upload to the designated storage location.";
		std::wstring strEnder = L"\r\n";
		if(this->IsHtmlBody())
			strEnder = L"<br>";

		strErrorMsg.append(strEnder);
		if(m_vFailedAttachments.size() > 0)//there are attachments which failed to FTP
		{
			//Pop up an box for sender
			std::wstring strTemp = m_strErrorMsgForSender;
			strTemp.append(L"\r\n\r\n");

			std::wstring::size_type nStart = 0;
			while( (nStart = strTemp.find(this->OBLIGATION_SUBJECT_PATTERN)) != std::wstring::npos)
				strTemp = strTemp.replace(nStart, wcslen(this->OBLIGATION_SUBJECT_PATTERN), CMailUtility::GetSubject(m_pItem));

			for (int i = 0; i < m_vFailedAttachments.size(); i++)
			{
				strTemp.append(m_vFailedAttachments[i] + L"\r\n");
			}
			MessageBoxW(::GetForegroundWindow(), strTemp.c_str(), L"FTP failed", MB_OK | MB_ICONERROR);

			strErrorMsg += m_strErrorMsgForRecipients;
			
		}

		if(this->IsHtmlBody())
		{
			strMsg =strSpanBegin;
			strMsg+=L"<hr>";
			strMsg+=strErrorMsg;
			strMsg+=L"<hr>";
			strMsg+=strSpanEnd;
		}
		else
		{
			strMsg =SEPARATOR_STRING;
			strMsg+=strNewLine;
			strMsg+=strErrorMsg;
			//strMsg+=L"Please make sure the designated storage location is reachable from the sender's host and re-send the attachment(s).\r\n";
			strMsg+=SEPARATOR_STRING;
			strMsg+=strNewLine;
		}

		
	}
	if(m_strTopMessageText.length()>wcslen(SEPARATOR_STRING))
	{
		m_strTopMessageText+=SEPARATOR_STRING;
		m_strTopMessageText+=strNewLine;
	}
	if(m_strBottomMessageText.length()>wcslen(SEPARATOR_STRING))
	{
		m_strBottomMessageText+=SEPARATOR_STRING;
		m_strBottomMessageText+=strNewLine;
	}
	
	wstrNewBody=strMsg;
	wstrNewBody+=strSpanBegin;
	wstrNewBody+=m_strTopMessageText;
	wstrNewBody+=strSpanEnd;
	wstrNewBody+=wstrBody;
	wstrNewBody+=strSpanBegin;
	wstrNewBody+=m_strBottomMessageText;
	wstrNewBody+=strSpanEnd;

	if(this->IsHtmlBody())
		hr=AdapterCommon::PutStringPropertyToObject(m_pItem,L"HTMLBody",wstrNewBody.c_str());
	else
		hr=AdapterCommon::PutStringPropertyToObject(m_pItem,L"Body",wstrNewBody.c_str());
	if(FAILED(hr))
		return FALSE;
	CComVariant varResult;
	hr=AdapterCommon::AutoWrap(DISPATCH_METHOD,&varResult,m_pItem,L"Save",0);
	if(FAILED(hr))
		return FALSE;
	
	return TRUE;
}
BOOL FTPAdapter::UploadAllEx(wchar_t** pwchObligationTopMessage,int &iTopMsgLength,wchar_t** pwchObligationBottomMessage,int &iBottomMsgLength)
{
	if(hFtpCli==NULL)
	{
		hFtpCli=LoadLibrary(FTPCLI_OUTPUT_NAME);		
	}

	m_vAttachments.clear();
	m_strPwdMailSubject.clear();
	m_strPwdMailBody.clear();
	m_strErrorMsgForSender.clear();
	m_strErrorMsgForRecipients.clear();
	m_vFailedAttachments.clear();

	size_t iIndex=0,iCount=m_pAttachments->Count();
	BOOL   bFailure=FALSE;
	for(iIndex=0;iIndex<iCount;iIndex++)
	{
		AdapterCommon::Attachment& pAtt=(m_pAttachments->Item(iIndex));
		if(UploadOne(&pAtt)==FALSE)
		{
			wchar_t wstrTempPath[MAX_PATH]={0};  
			GetTempPath(MAX_PATH, wstrTempPath); 
			SYSTEMTIME sys;
			GetLocalTime(&sys);
			WCHAR currTime[32]={0};
			wsprintf(currTime,L"_%4d%02d%02d-%02d%02d%02d-%03d",sys.wYear,sys.wMonth,sys.wDay,sys.wHour,sys.wMinute,sys.wSecond,sys.wMilliseconds);
			wchar_t wstrNewFolder[MAX_PATH]={0};
			swprintf_s(wstrNewFolder,L"%s%s",wstrTempPath,currTime);

			std::wstring wstrSrcNameOnly=pAtt.GetTempPath();
			std::wstring::size_type pos=wstrSrcNameOnly.rfind(L"\\");
			if(pos != std::wstring::npos)
			{
				wstrSrcNameOnly=wstrSrcNameOnly.substr(pos+1);
			}
			else
			{
				pos=pAtt.GetSrcPath().rfind(L"/");
				if(pos != std::wstring::npos)
				{
					wstrSrcNameOnly=wstrSrcNameOnly.substr(pos+1);
				}
			}
			if(CreateDirectory(wstrNewFolder,NULL))
			{
				swprintf_s(wstrNewFolder,L"%s\\%s",wstrNewFolder,wstrSrcNameOnly.c_str());
				if(CopyFile(pAtt.GetTempPath().c_str(),wstrNewFolder,true))
				{
					wchar_t wstrErrorMsg[MAX_PATH]={0};
					swprintf_s(wstrErrorMsg,L"Upload File To FTP Server Error , Backup Attachment to Folder %s",wstrNewFolder);
					HWND hWindow = ::GetActiveWindow();
					if(NULL==hWindow)
					{
						hWindow=::GetDesktopWindow();
					}
					MessageBoxW(hWindow,wstrErrorMsg , L"FS failed", MB_OK | MB_ICONERROR);		
				}
			}

			DP((L"FTP Adapter:upload %s[%s] failed",pAtt.GetTempPath().c_str(),pAtt.GetSrcPath().c_str()));
			bFailure=TRUE;
		}

	}

	//Send a 2nd mail to user for PASSWORDs
	std::wstring strEnder = L"<br>";
	int nMailType = 0;
	if (!this->IsHtmlBody())
	{
		strEnder = L"\r\n";
		nMailType = 1;
	}

	if (m_vAttachments.size() > 0 )
	{
		std::wstring strBody;
		strBody.append(this->SEPARATOR_STRING);
		strBody.append(strEnder);


		//replace the subject
		std::wstring strSubject = CMailUtility::GetSubject(m_pItem);
		std::wstring::size_type nStart;
		while( (nStart = m_strPwdMailBody.find(this->OBLIGATION_SUBJECT_PATTERN)) != std::wstring::npos)
			m_strPwdMailBody = m_strPwdMailBody.replace(nStart, wcslen(this->OBLIGATION_SUBJECT_PATTERN), strSubject);

		//replace the [Return]
		while( (nStart = m_strPwdMailBody.find(this->OBLIGATION_RETURN)) != std::wstring::npos)
		{
			m_strPwdMailBody.replace(nStart, wcslen(this->OBLIGATION_RETURN), strEnder);
		}

		strBody.append(m_strPwdMailBody);

		strBody.append(strEnder);
		strBody.append(strEnder);


		for (std::vector<ATTACHMENTPARAM>::size_type i = 0; i < m_vAttachments.size(); i++)
		{
			const ATTACHMENTPARAM& param = m_vAttachments[i];

			wchar_t buf[1024] = {0};
			swprintf_s(buf, sizeof(buf)/sizeof(wchar_t) - 1, L"Document: %s%sPassword: %s%s%s", param.strFileName.c_str(), strEnder.c_str(), param.strPassword.c_str(), strEnder.c_str(), strEnder.c_str() );

			strBody.append(buf);
		}

		strBody.append(this->SEPARATOR_STRING);
		wstring strRecipients = CMailUtility::GetRecipients(m_pItem);
		//	DP((L"Try to send PASSWORD mail to: %s, body: %s\n", strRecipients.c_str(), strBody.c_str()));

		//Force to load ftpadapter.dll again. adaptermanager will free FtpAdapter, but we have a thread here to send "PASSWORD mail", so it will generate an exception if the DLL was free before thread ends
		
		#ifndef _WIN64 
		HMODULE hFTPAdapter = GetModuleHandleW(L"ftpadapter32.dll");
		#else
		HMODULE hFTPAdapter = GetModuleHandleW(L"ftpadapter.dll");
		#endif
		wchar_t buf[MAX_PATH + 1] = {0};
		GetModuleFileNameW(hFTPAdapter, buf, MAX_PATH);

		if(wcslen(buf) > 0)
		{
			HMODULE h = LoadLibraryW(buf);
			DP((L"Loaded ftpadater again: %s, %x\r\n", buf, h));
			if (h)
			{
				//try to create a thread to send PASSWORD mail
				MAILPARAM* pParam = new MAILPARAM;
				pParam->strSubject = m_strPwdMailSubject;
				pParam->strBody = strBody;
				pParam->strTo = strRecipients;
				pParam->strCC = L"";
				pParam->nMailType = nMailType;

				CreateThread(NULL, 0, SendMailThread, (LPVOID)pParam, 0, NULL);
			}

		}



		//  CMailUtility::SendMail(m_strPwdMailSubject, strBody, strRecipients, L"", nMailType);
	}


	HRESULT hr=S_OK;
	std::wstring strMsg,wstrBody,wstrNewBody,strNewLine;
	std::wstring strSpanBegin,strSpanEnd;
	if(this->IsHtmlBody())
	{
		strNewLine=L"<br>";
		strSpanBegin=L"<span style='font-size:11.0pt;font-family:\"Calibri\"'>";
		strSpanEnd=L"</span>";
		hr=AdapterCommon::GetStringPropertyFromObject(m_pItem,L"HTMLBody",wstrBody);
	}
	else
	{
		strNewLine=L"\r\n";
		hr=AdapterCommon::GetStringPropertyFromObject(m_pItem,L"Body",wstrBody);
	}
	if(FAILED(hr))
		return TRUE;
	if(bFailure==TRUE)
	{
		HRESULT hr=S_OK;

		std::wstring strErrorMsg = L"One or more attachment(s) failed to upload to the designated storage location.";
		std::wstring strEnder = L"\r\n";
		if(this->IsHtmlBody())
			strEnder = L"<br>";

		strErrorMsg.append(strEnder);
		
		if(this->IsHtmlBody())
		{
			strMsg =strSpanBegin;
			strMsg+=L"<hr>";
			strMsg+=strErrorMsg;
			strMsg+=L"<hr>";
			strMsg+=strSpanEnd;
		}
		else
		{
			strMsg =SEPARATOR_STRING;
			strMsg+=strNewLine;
			strMsg+=strErrorMsg;
			//strMsg+=L"Please make sure the designated storage location is reachable from the sender's host and re-send the attachment(s).\r\n";
			strMsg+=SEPARATOR_STRING;
			strMsg+=strNewLine;
		}
		m_strBottomMessageText+=strMsg;

	}
	if(m_strTopMessageText.length()>wcslen(SEPARATOR_STRING))
	{
		m_strTopMessageText+=SEPARATOR_STRING;
		m_strTopMessageText+=strNewLine;
	}
	if(m_strBottomMessageText.length()>wcslen(SEPARATOR_STRING))
	{
		m_strBottomMessageText+=SEPARATOR_STRING;
		m_strBottomMessageText+=strNewLine;
	}
	iTopMsgLength= ((strSpanBegin+m_strTopMessageText+strSpanEnd).length())+1;
	iBottomMsgLength=((strSpanBegin+m_strBottomMessageText+strSpanEnd).length())+1;
	*pwchObligationTopMessage=new wchar_t[iTopMsgLength];
	wmemset(*pwchObligationTopMessage, 0, iTopMsgLength);
	*pwchObligationBottomMessage=new wchar_t[iBottomMsgLength];
	wmemset(*pwchObligationBottomMessage,0,iBottomMsgLength);
	swprintf_s(*pwchObligationTopMessage, iTopMsgLength, L"%s", (strSpanBegin+m_strTopMessageText+strSpanEnd).c_str());
	swprintf_s(*pwchObligationBottomMessage, iBottomMsgLength, L"%s", (strSpanBegin+m_strBottomMessageText+strSpanEnd).c_str());
	
	return TRUE;
}
BOOL FTPAdapter::Init()
{
	if(hFtpCli==NULL)
	{
		std::wstring wstrModuleBasename;
		if(GetModuleBaseName(wstrModuleBasename)==FALSE)
			return FALSE;

		wstrModuleBasename+=L"\\";
		wstrModuleBasename+=FTPCLI_OUTPUT_NAME;
		hFtpCli=LoadLibrary(wstrModuleBasename.c_str());
	}
	if(hFtpCli==NULL)
		return FALSE;

	if(fpFtpUploadEx==NULL)
		fpFtpUploadEx=(FPFtpUploadEx)GetProcAddress(hFtpCli,"FtpUploadEx");
	
	if(fpFtpUploadEx==NULL)
		return FALSE;

	if(m_pItem)
	{
		CComVariant varResult;
		HRESULT hr=AdapterCommon::AutoWrap(DISPATCH_PROPERTYGET,&varResult,m_pItem,L"BodyFormat",0);
		if(SUCCEEDED(hr)&&(varResult.intVal==2||varResult.intVal==3))
			m_bIsHtml=true;
	}

	return TRUE;
}
STDAPI RepositoryUpload(IDispatch*pItem,AdapterCommon::Attachments* pAtts)
{
	HRESULT hr=S_OK;
	if(pAtts->Count()==1&&pAtts->Item(0).GetSrcPath()==L"C:\\No_attachment.ice")
		return S_OK;

	AdapterCommon::Adapter* pAdapter=GetAdapter();
	if(pAdapter==NULL)
	{
		DP((L"Failed to Get Adapter in RepositoryUpload"));
		return S_FALSE;
	}
	
	FTPAdapter ftpAdapter(pItem,pAtts,pAdapter);
	if(ftpAdapter.Init()==FALSE)
	{
		DP((L"Failed to Init for FTP adapter"));
		return S_FALSE;
	}
	BOOL bRet=ftpAdapter.UploadAll();

	delete pAdapter;
	if(bRet==TRUE)
		return S_OK;
	DP((L"RepositoryUpload of FTP Adapter failed"));
	return S_FALSE;
}
STDAPI RepositoryUploadEx(IDispatch*pItem,AdapterCommon::Attachments* pAtts,wchar_t** pwchObligationTopMessage,int &iTopMsgLength,wchar_t** pwchObligationBottomMessage,int &iBottomMsgLength)
{
	HRESULT hr=S_OK;
	if(pAtts->Count()==1&&pAtts->Item(0).GetSrcPath()==L"C:\\No_attachment.ice")
	{
		return S_OK;
	}
	AdapterCommon::Adapter* pAdapter=GetAdapter();
	if(pAdapter==NULL)
	{
		DP((L"Failed to Get Adapter in RepositoryUpload"));
		return S_FALSE;
	}

	FTPAdapter ftpAdapter(pItem,pAtts,pAdapter);
	if(ftpAdapter.Init()==FALSE)
	{
		DP((L"Failed to Init for FTP adapter"));
		return S_FALSE;
	}
	BOOL bRet=ftpAdapter.UploadAllEx(pwchObligationTopMessage,iTopMsgLength,pwchObligationBottomMessage,iBottomMsgLength);
	delete pAdapter;
	if(bRet==TRUE)
	{
		return S_OK;
	}
	DP((L"RepositoryUpload of FTP Adapter failed"));
	return S_FALSE;
}
STDAPI ReleaseRepositoryUploadExPWCH(wchar_t * pwch,bool bIsArray)
{
	if(pwch!=NULL)
	{
		if(bIsArray)
		{
			delete[] pwch;
		}
		else
		{
			delete pwch;
		}
		
	}
	return true;
}