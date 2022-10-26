#include "stdafx.h"
#include "backthread.h"
#include "ftpcli.h"
#include "log.h"
#include <wininet.h>
extern HINSTANCE   g_hInstance;
HRESULT SendMail(CComPtr<Outlook::_olApplication> spApp, LPCWSTR pwzTo, LPCWSTR pwzCC, LPCWSTR pwzSubject, LPCWSTR pwzHTMLBody)
{
	//Sleep(2*60*1000);
	DP((L"Send email to %s (subject:%s)",pwzTo,pwzSubject));
    HRESULT             hr = S_OK;
    CComPtr<IDispatch>  spDisp;
	hr = spApp->CreateItem(/*OlItemType::*/Outlook::olMailItem, &spDisp);
    if(FAILED(hr) || spDisp==0)
        return hr;
    CComQIPtr<Outlook::_MailItem>   spMail(spDisp);
    ATLASSERT(spMail);
	//spApp->AddRef();
	long lCodePage=0;
	hr=spMail->get_InternetCodepage(&lCodePage);
	hr=spMail->put_InternetCodepage((long)65001);//set code page to Unicode UTF8
    CComBSTR varSubject(pwzSubject);
    CComBSTR varHTMLBody(pwzHTMLBody);
    CComBSTR varTo(pwzTo);
    CComBSTR varCC(pwzCC);
	hr = spMail->put_BodyFormat(/*OlBodyFormat::*/Outlook::olFormatHTML);
	if(FAILED(hr))
		return hr;
    hr = spMail->put_Subject(varSubject);
	if(FAILED(hr))
		return hr;
    hr = spMail->put_HTMLBody(varHTMLBody);
	if(FAILED(hr))
		return hr;
    hr = spMail->put_To(varTo);
	if(FAILED(hr))
		return hr;
    if(NULL!=pwzCC && 0<wcslen(pwzCC))
        hr = spMail->put_CC(varCC);
	DP((L"Before send email....spMail->Send():Recipients:%s,CC:%s",pwzTo,pwzCC));
	hr=spMail->Save();
    hr = spMail->Send();
	//hr = spMail->put_InternetCodepage(lCodePage);
	DP((L"After send email....spMail->Send():hr=%x",hr));
    return hr;
}

std::wstring MakeOriginalRequest(ApprovalEmail& ae)
{
   std::wstring strOriginalRequest = L"";
    RecipientVector::const_iterator itr;
	std::vector<std::wstring>::const_iterator      itf;
    const std::wstring& strRequesterAddr = ae.get_RequesterAddress();
    const std::wstring& strRequesterName = ae.get_RequesterName();
    
    const std::wstring& strPurpose = ae.get_Purpose();
    const std::vector<std::wstring>& vFiles = ae.get_vecFiles();
    const RecipientVector& vRecipients = ae.get_Recipients();
    // <-- Original Request
    strOriginalRequest += L"<table width=\"610\" style=\"font-family:Arial\" style=\"font-size:15\">\r\n<tr>\r\n";
    strOriginalRequest += L"  <tr>\r\n";
    strOriginalRequest += L"    <td valign=\"center\" align=\"left\"><HR><BR><B>Original Request from You:</B></td>\r\n";
    strOriginalRequest += L"  </tr>\r\n";
    strOriginalRequest += L"</table>\r\n";

    strOriginalRequest += L"<TABLE width=\"610\" style=\"font-family:Arial\" style=\"font-size:13\">\r\n";
    // Type
    // add "pre information"
    strOriginalRequest += L"  <tr>\r\n";
    strOriginalRequest += L"    <td colspan=\"2\" valign=\"top\" align=\"left\">&nbsp;&nbsp;&nbsp;&nbsp;Please approve sharing of following files requested by ";
    strOriginalRequest += L"<a href=mailto:"; strOriginalRequest += strRequesterAddr; strOriginalRequest+=L">"; strOriginalRequest += strRequesterName; strOriginalRequest += L"</a>";
    /*strOriginalRequest += L" using "; strOriginalRequest += strRequestType; */strOriginalRequest += L"<br>&nbsp;<br></td>\r\n";
    strOriginalRequest += L"  </tr>\r\n";

    // Subject
    strOriginalRequest += L"  <tr>\r\n";
    strOriginalRequest += L"    <td valign=\"top\" align=\"left\" width=\"10%\"><b>Subject:</b></td>\r\n";
	strOriginalRequest += L"    <td valign=\"top\" align=\"left\">";  strOriginalRequest += ae.get_OrigSubject(); strOriginalRequest += L"<br>&nbsp;<br></td>\r\n";
    strOriginalRequest += L"  </tr>\r\n";
    // Purpose
    strOriginalRequest += L"  <tr>\r\n";
    strOriginalRequest += L"    <td valign=\"top\" align=\"left\"><b>Content:</b></td>\r\n";
    strOriginalRequest += L"    <td valign=\"top\" align=\"left\"><pre>";  strOriginalRequest += strPurpose; strOriginalRequest += L"</pre></td>\r\n";
    strOriginalRequest += L"  </tr>\r\n";
    // Recipients
    strOriginalRequest += L"  <tr>\r\n";
    strOriginalRequest += L"    <td valign=\"top\" align=\"left\"><b>Recipients:</b></td>\r\n";
    strOriginalRequest += L"    <td valign=\"top\" align=\"left\">";
    for(itr = vRecipients.begin(); itr!=vRecipients.end(); ++itr)
    {
        strOriginalRequest += (*itr).c_str(); strOriginalRequest += L"<br>";
    }
    strOriginalRequest += L"&nbsp;<br></td>\r\n";
    strOriginalRequest += L"  </tr>\r\n";
    // Files
    strOriginalRequest += L"  <tr>\r\n";
    strOriginalRequest += L"    <td valign=\"top\" align=\"left\"><b>Files:</b></td>\r\n";
    strOriginalRequest += L"    <td valign=\"top\" align=\"left\">\r\n";
	
    for(itf = vFiles.begin(); itf!=vFiles.end(); ++itf)
    {
        
        strOriginalRequest += L"      <a href=\"";
        strOriginalRequest += (*itf).c_str();
        strOriginalRequest += L"\">";
        strOriginalRequest += (*itf).c_str();//pwzFileName?(pwzFileName+1):((*itf).first.c_str());
        strOriginalRequest += L"</a><br>\r\n";
    }
    strOriginalRequest += L"    &nbsp;<br></td>\r\n";
    strOriginalRequest += L"  </tr>\r\n";

    // Original Request End -->
    strOriginalRequest += L"</TABLE>\r\n";

    return strOriginalRequest;
}

void HTMLEncode(std::wstring & strIn,std::wstring&strOut)
{
	strOut=L"";
	std::wstring::iterator it;
	
	for(it=strIn.begin();it!=strIn.end();it++)
	{
		if((*it)>=0x100)
		{
			char chBuff[12];
			int iCnt=::WideCharToMultiByte(CP_UTF8/*CP_ACP*/,0,&(*it),1,chBuff,12,NULL,NULL);
			
			for(int i=0;i<iCnt;i++)
			{
				WCHAR szTemp[16]=L"";
				_snwprintf_s(szTemp,16, _TRUNCATE, L"%%%X",(unsigned char)chBuff[i]);
				strOut+=szTemp;				

			}
		}
		else
			strOut+=(*it);
	}
	return;
}
std::wstring ComposeApproveMail(ApprovalEmail& ae, ApproveFilesVector& vAFiles)
{
     std::wstring strMail = L"";
    const std::wstring strOriginalRequest = MakeOriginalRequest(ae);
    ApproveFilesVector::iterator    itaf;

    // Title
    strMail += L"<html charset=\"utf-8\" CodePage=65001>\r\n";
    strMail += L"<table width=\"610\" style=\"font-family:Arial\" style=\"font-size:19\">\r\n<tr>\r\n";
    strMail += L"  <tr>\r\n";
    strMail += L"    <td valign=\"center\" align=\"left\" height=\"50\"><b>The request is approved</b></td>\r\n";
    strMail += L"  </tr>\r\n";
    strMail += L"</table>\r\n";

 //   // Information
 //   strMail += L"<table width=\"610\" style=\"font-family:Arial\" style=\"font-size:15\">\r\n<tr>\r\n";
 //   strMail += L"  <tr>\r\n";
 //   strMail += L"    <td valign=\"center\" align=\"left\">&nbsp;&nbsp;&nbsp;&nbsp;Your request has been approved by approver.\
 //               Following file(s) can be shared with recipient(s).</td>\r\n";
 //   strMail += L"  </tr>\r\n";
 //   // Approved files
 //   strMail += L"  <tr>\r\n";
 //   strMail += L"    <td valign=\"center\" align=\"left\">\r\n";
	//std::vector<std::wstring>& vecFiles=m_pae->get_vecFiles();
	//for(itaf = vecFiles.begin(); itaf!=vecFiles.end(); ++itaf)
 //   {
 //       LPCWSTR pwzFileName = wcsrchr((*itaf).c_str(), L'\\');
 //       strMail += L"      <a href=\"";
 //       strMail += (*itaf).c_str(); strMail += L"\">";
 //       strMail += (*itaf).c_str();//pwzFileName?(pwzFileName+1):((*itaf).c_str()); 
	//	strMail += L"</a><br>\r\n";
 //   }
 //   strMail += L"    </td>\r\n";
 //   strMail += L"  </tr>\r\n";
 //   strMail += L"</table>\r\n";

	// Information
    strMail += L"<table width=\"610\" style=\"font-family:Arial\" style=\"font-size:15\">\r\n<tr>\r\n";
    strMail += L"  <tr>\r\n";
	strMail += L"    <td valign=\"center\" align=\"left\">&nbsp;&nbsp;&nbsp;&nbsp;Please send the following link to the recipient(s) in the request below:</td>\r\n";
    strMail += L"  </tr>\r\n";
    // Uploaded files
    strMail += L"  <tr>\r\n";
    strMail += L"    <td valign=\"center\" align=\"left\">\r\n";
	//std::vector<std::wstring>& vecFiles=m_pae->get_vecFiles();
	for(itaf = vAFiles.begin(); itaf!=vAFiles.end(); ++itaf)
    {
        
        strMail += L"      <a href=\"";
		std::wstring strHTMLEncoded;
		HTMLEncode(*itaf,strHTMLEncoded);
		strMail += strHTMLEncoded.c_str()/*(*itaf).c_str()*/; strMail += L"\">";
        strMail += (*itaf).c_str();//pwzFileName?(pwzFileName+1):((*itaf).c_str()); 
		strMail += L"</a><br>\r\n";
    }
    strMail += L"    </td>\r\n";
    strMail += L"  </tr>\r\n";
    strMail += L"</table>\r\n";
    // Original Request
    strMail += strOriginalRequest;
    strMail += L"</html>\r\n";

    return strMail;
}

unsigned int __stdcall ZipFtpBackThread(void* lpParam)
{
	HRESULT						hr=S_OK;
	BackThreadParam*			pThreadParam=(BackThreadParam*)lpParam;
	ApprovalEmail*				pae=pThreadParam->pae;
	CComPtr<Outlook::_olApplication>	pApp=NULL;
	CComPtr<Outlook::_MailItem>			pItem=NULL;
	ApproveFilesVector			vAFiles;

	if(pThreadParam&&pThreadParam->pItemStream)
	{
		hr = CoGetInterfaceAndReleaseStream(pThreadParam->pItemStream, __uuidof(Outlook::_MailItem), (LPVOID*)&pItem);
		_ASSERT(SUCCEEDED(hr) && pItem);
	}
	
	if(pThreadParam&&pThreadParam->pAppStream)
	{
		hr = CoGetInterfaceAndReleaseStream(pThreadParam->pAppStream, __uuidof(Outlook::_olApplication), (LPVOID*)&pApp);
		_ASSERT(SUCCEEDED(hr) && pItem);
	}
	{
		/*CPasswdGenerator passGen;
		std::wstring strMailTest=passGen.Generator();
		strMailTest +=pae->Compose();
		CComBSTR bstrMailTest(strMailTest.c_str());
		CComPtr<Outlook::_MailItem> pItemTest=NULL;
		HRESULT hrTest = CoGetInterfaceAndReleaseStream(pThreadParam->pItemStream, __uuidof(Outlook::_MailItem), (LPVOID*)&pItemTest);
		_ASSERT(SUCCEEDED(hrTest) && pItemTest);
		hr=pItemTest->put_HTMLBody(bstrMailTest);
		pItemTest->Save();

		delete pae;
		delete lpParam;
		_endthread();
		return 0;*/
		
		/*CComPtr<Outlook::_olApplication> pAppTest;
		hr = CoGetInterfaceAndReleaseStream(pThreadParam->pAppStream, __uuidof(Outlook::_olApplication), (LPVOID*)&pAppTest);
		_ASSERT(SUCCEEDED(hr) && pAppTest);
		HRESULT    hr=SendMail(pAppTest, L"john.tyler@lab01.cn.nextlabs.com", NULL, L"test from Approver", L"Can you see this message?");
		return 0;*/
	}

	CComBSTR bstrSubject;
	
	bstrSubject=L"[Uploading to FTP] Approval request for email documents";
#pragma warning(push)
#pragma warning(disable: 6011)
	hr=pItem->put_Subject(bstrSubject);
#pragma warning(pop)
	hr=pItem->Save();

	std::wstring strPasswd=pae->get_EncryptPasswd();
	
	CZipEncryptor zipEncryptor(g_hInstance);

	//b. Upload file to FTP server
	WCHAR wzRemoteFile[1024]=L"",wzEncryptedFileName[MAX_PATH*2]=L"";
	unsigned int i=0;
	bool bUpload,bFTPFailed=false;
	for(i=0;i<pae->get_vecFiles().size();i++)
	{
		if(zipEncryptor.Encrypt(pae->get_vecFiles()[i].c_str(),wzEncryptedFileName,strPasswd.c_str())==S_OK)
		{
			bUpload=FtpUpload(pae->get_FtpServer().c_str(),
								pae->get_FtpUser().c_str(),
								pae->get_FtpPasswd().c_str(),
								pae->get_FtpPort(),
								wzEncryptedFileName,//ae.get_vecFiles()[i].c_str(),
								wzRemoteFile);
			if(bUpload)
			{
				vAFiles.push_back(wzRemoteFile);
			}
			else
			{
				std::wstring strErrMsg=L"Fail to upload file ";
				strErrMsg+=pae->get_vecFiles()[i];
				strErrMsg+=L" to ";
				strErrMsg+=pae->get_FtpServer();
				CLog::WriteLog(L"Error",strErrMsg.c_str());
				bFTPFailed=true;
			}
			ZeroMemory(wzRemoteFile,sizeof(wzRemoteFile));
		}
		else
		{
			std::wstring strErrMsg=L"Fail to encrypt file ";
			strErrMsg+=pae->get_vecFiles()[i];
			strErrMsg+=L" with zip";
			strErrMsg+=pae->get_FtpServer();
			CLog::WriteLog(L"Error",strErrMsg.c_str());
			bFTPFailed=true;
		}
	}
	
	if(bFTPFailed==true)
	{
		bstrSubject=L"[Approval Upload Failed] Approval request for email documents";
		hr=pItem->put_Subject(bstrSubject);
		hr=pItem->Save();
		delete pae;
		delete lpParam;
		return 0;
	}
	const std::wstring& strApprovalMail = ComposeApproveMail(*pae, vAFiles);
    std::wstring strTo;
	pae->get_Approvers(strTo); //ae.get_RequesterAddress();
    const std::wstring& strCC = pae->get_ArchiverAddress();
    hr=SendMail(pApp, strTo.c_str(), strCC.c_str(), APPROVAL_EMAIL_SUBJECT, strApprovalMail.c_str());
	if(SUCCEEDED(hr))
	{
		const std::wstring strOriginalRequest = MakeOriginalRequest(*pae);
		std::wstring strMail = L"<table width=\"610\" style=\"font-family:Arial\" style=\"font-size:19\">\r\n<tr>\r\n";
		strMail += L"  <tr>\r\n";
		strMail += L"    <td valign=\"center\" align=\"left\" height=\"50\"><b>Your request is approved. The request approver will send the FTP URL to the desired recipient(s).</b></td>\r\n";
		strMail += L"  </tr>\r\n";
		strMail += L"</table>\r\n";
		strMail += strOriginalRequest;
		std::wstring strApprovedTo=pae->get_RequesterAddress();
		hr=SendMail(pApp,strApprovedTo.c_str(),strCC.c_str(),APPROVAL_EMAIL_SUBJECT,strMail.c_str());
		if(SUCCEEDED(hr))
		{
			strMail = L"<pre>";
			strMail +=pae->get_Purpose();
			strMail +=L"</pre>";

			strMail +=L"<table width=\"610\" style=\"font-family:Arial\" style=\"font-size:19\">\r\n<tr>\r\n";
			strMail += L"  <tr>\r\n";
			strMail += L"    <td valign=\"center\" align=\"left\" height=\"50\"><HR><BR><b>The following files have been made available to you on the Flextronics FTP server.</b></td>\r\n";
			strMail += L"  </tr>\r\n";
			strMail += L"</table>\r\n";

			strMail += L"<table width=\"610\" style=\"font-family:Arial\" style=\"font-size:15\">\r\n<tr>\r\n";
			// Uploaded files
			strMail += L"  <tr>\r\n";
			strMail += L"    <td valign=\"center\" align=\"left\">\r\n";
			//std::vector<std::wstring>& vecFiles=m_pae->get_vecFiles();
			ApproveFilesVector::iterator    itaf;
			for(itaf = vAFiles.begin(); itaf!=vAFiles.end(); ++itaf)
			{
				
				strMail += L"      <a href=\"";
				std::wstring strHTMLEncoded;
				HTMLEncode(*itaf,strHTMLEncoded);
				strMail += strHTMLEncoded.c_str()/*(*itaf).c_str()*/; strMail += L"\">";
				strMail += (*itaf).c_str();//pwzFileName?(pwzFileName+1):((*itaf).c_str()); 
				strMail += L"</a><br>\r\n";
			}
			strMail += L"    </td>\r\n";
			strMail += L"  </tr>\r\n";
			strMail += L"</table>\r\n";
			const RecipientVector& vRecipients = pae->get_Recipients();
			RecipientVector::const_iterator itr;
			strTo=L"";
			for(itr = vRecipients.begin(); itr!=vRecipients.end(); ++itr)
			{
				strTo += (*itr).c_str(); strTo += L";";
			}
			//Password and Regards information
			strMail +=L"<table width=\"610\" style=\"font-family:Arial\" style=\"font-size:14\">\r\n<tr>\r\n";
			strMail += L"  <tr>\r\n";
			strMail += L"    <td valign=\"center\" align=\"left\" height=\"50\"><b>You will need to use the password \"";
			strMail += pae->get_EncryptPasswd();
			strMail += L"\" to open them.</b></td>\r\n";
			strMail += L"  </tr>\r\n";
			strMail += L"  <tr>\r\n";
			strMail += L"    <td valign=\"center\" align=\"left\" height=\"50\"><b>Regards</b></td>\r\n";
			strMail += L"  </tr>\r\n";
			
			std::wstring strRegardsName;
			/*lUserName=sizeof(wstrUserName);
			ZeroMemory(wstrUserName,sizeof(wstrUserName));
			bName=::GetUserNameEx(NameDisplay,wstrUserName,&lUserName);*/
			GetCurrentUser(*pApp,strRegardsName,true);
			if(strRegardsName.length()==0)
			{
				strRegardsName=L"Your Sincerely";
			}
			strMail += L"  <tr>\r\n";
			strMail += L"    <td valign=\"center\" align=\"left\" height=\"50\"><b>";
			strMail += strRegardsName;
			strMail += L"</b></td>\r\n";
			strMail += L"  </tr>\r\n";
			strMail += L"</table>\r\n";
			std::wstring strApproverRequester=L"";
			pae->get_Approvers(strApproverRequester); 
			//strApproverRequester +=ae.get_RequesterAddress();
			hr=SendMail(pApp,strTo.c_str(),strApproverRequester.c_str(),pae->get_OrigSubject().c_str()/*L"Document Sharing from Flextronics"*/,strMail.c_str());
		}
		else
		{
			DP((L"Failed to send email back to the requester.Requester:%s",strApprovedTo.c_str()));
		}
	}
	else
	{
		DP((L"Failed to send approval email to the approver(s).To:%s, CC:%s",strTo.c_str(),strCC.c_str()));
	}
	bstrSubject=L"[Approved] Approval request for email documents";
	hr=pItem->put_Subject(bstrSubject);
	hr=pItem->Save();
//_Exit_Clear:
	delete pae;
	delete lpParam;
	_endthread();
	return 0;
}