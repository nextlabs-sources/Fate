// ftpadapter.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "approvaladapter.h"
#include <stdio.h>
#include "log.h"
#include "adaptercomm.h"
#include <shlobj.h>
#include <fstream>

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
		//Adapters adapters;
		//adapters.Init(L"Outlook Enforcer");
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

static BOOL GetModuleBaseName(std::wstring& wstrModuleBaseName)
{
	WCHAR wzModuleFileName[MAX_PATH+1];memset(wzModuleFileName,0,sizeof(wzModuleFileName));
	DWORD dwRet=GetModuleFileName(g_hInstance,wzModuleFileName,MAX_PATH);
	
	if(dwRet==0||GetLastError() == ERROR_INSUFFICIENT_BUFFER)
		return FALSE;
	std::wstring wstrTemp=wzModuleFileName;
	std::wstring::size_type pos=wstrTemp.rfind(L'/');
	if(pos==std::wstring::npos)
	{
		pos=wstrTemp.rfind(L'\\');
		if(pos==std::wstring::npos)
			return FALSE;
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
	wstrIniFile +=APPROVALADAPTER_INI_FILENAME;
	return new AdapterCommon::Adapter(wstrIniFile.c_str(),wzModuleFileName,APPROVALADAPTER_OBLIGATION_NAME);
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
#include "celog.h"
#include "celog_policy_windbg.hpp"
#include "celog_policy_file.hpp"
#include "eframework/auto_disable/auto_disable.hpp"
nextlabs::recursion_control mso_hook_control;
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
/*
BOOL ApprovalAdapter::UploadOne(AdapterCommon::Attachment *pAtt)
{
	if(fpFtpUpload==NULL)
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
		ftpOb.AddAttribute(AdapterCommon::Attribute(this->OBLIGATION_ATTRNAME_FTPSERVER,L"lab01-sps07"));
		ftpOb.AddAttribute(AdapterCommon::Attribute(this->OBLIGATION_ATTRNAME_FTPUSER,L"jjin"));
		ftpOb.AddAttribute(AdapterCommon::Attribute(this->OBLIGATION_ATTRNAME_FTPPASSWORD,L"daxiea!110"));
		ftpOb.AddAttribute(AdapterCommon::Attribute(this->OBLIGATION_ATTRNAME_LOCATION,OBLIGATION_ATTRVALUE_LOCATION_TOP));
		ftpOb.AddAttribute(AdapterCommon::Attribute(this->OBLIGATION_ATTRNAME_TEXT,OBLIGATION_ATTRVALUE_TEXT));
		ftpOb.AddAttribute(AdapterCommon::Attribute(this->OBLIGATION_ATTRNAME_LINKFORMAT,OBLIGATION_ATTRVALUE_LINKFORMAT_LONG));
		pAtt->AddObligation(ftpOb);

	}
#endif
	std::wstring wstrFtpServer,wstrUser,wstrPasswd,wstrSrc=pAtt->GetTempPath();
	size_t iIndex=0,obCount=pAtt->Count();
	if(obCount==0)
		return TRUE;
	for(iIndex=0;iIndex<obCount;iIndex++)
	{
		hr=S_OK;
		AdapterCommon::Obligation ob=pAtt->Item(iIndex);	
		if(m_pAdapter->GetObligationName()==ob.GetName())
		{
			if(ob.FindAttribute(this->OBLIGATION_ATTRNAME_FTPSERVER,wstrFtpServer)==false)
				return FALSE;
			if(ob.FindAttribute(this->OBLIGATION_ATTRNAME_FTPUSER,wstrUser)==false)
				return FALSE;
			if(ob.FindAttribute(this->OBLIGATION_ATTRNAME_FTPPASSWORD,wstrPasswd)==false)
				return FALSE;

			hr = fpFtpUpload(wstrFtpServer.c_str(),wstrUser.c_str(),wstrPasswd.c_str(),21,wstrSrc.c_str(),wzRemoteFile);
			if(hr!=S_OK)
				return FALSE;
			else
			{
				pAtt->SetReturnPath(wzRemoteFile);
				pAtt->SetStripFlag(true);
				std::wstring	wstrLocation(OBLIGATION_ATTRVALUE_LOCATION_BOTTOM),
								wstrText(OBLIGATION_ATTRVALUE_TEXT),
								wstrLinkFormat(m_bIsHtml==true?OBLIGATION_ATTRVALUE_LINKFORMAT_LONG:OBLIGATION_ATTRVALUE_LINKFORMAT_SHORT);
				ob.FindAttribute(this->OBLIGATION_ATTRNAME_LOCATION,wstrLocation);
				ob.FindAttribute(this->OBLIGATION_ATTRNAME_TEXT,wstrText);
				ob.FindAttribute(this->OBLIGATION_ATTRNAME_LINKFORMAT,wstrLinkFormat);
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
					wstrLink  = L"";//L"<";
					wstrLink += wzRemoteFile;
					//wstrLink += L">";
				}
				//Replace "[link]" with the return remote path of FTP upload
				AdapterCommon::StringReplace(wstrText,FTPADAPTER_PLACEHOLDER_LINK,wstrLink);
				
				std::wstring wstrBody;
				if(this->IsHtmlBody())
					hr=AdapterCommon::GetStringPropertyFromObject(m_pItem,L"HTMLBody",wstrBody);
				else
					hr=AdapterCommon::GetStringPropertyFromObject(m_pItem,L"Body",wstrBody);
				
				if(FAILED(hr))
					return TRUE;
				std::wstring wstrNewBody=L"\r\n";;
				if(wstrLocation!=OBLIGATION_ATTRVALUE_LOCATION_TOP)
				{
					wstrNewBody += wstrBody;
					wstrNewBody += wstrText;
				}
				else //Bottom
				{
					wstrNewBody += wstrText;
					wstrNewBody += L"\r\n";
					wstrNewBody += wstrBody;
				}
				wstrNewBody+=L"\r\n";
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
		}
		else
			continue;
		

	}
	
	return TRUE;
	
}
BOOL FTPAdapter::UploadAll()
{
	if(hFtpCli==NULL)
		hFtpCli=LoadLibrary(L"ftpcli.dll");

	size_t iIndex=0,iCount=m_pAttachments->Count();

	for(iIndex=0;iIndex<iCount;iIndex++)
	{
		AdapterCommon::Attachment& pAtt=(m_pAttachments->Item(iIndex));
		if(UploadOne(&pAtt)==FALSE)
		{
			DP((L"FTP Adapter:upload %s[%s] failed",pAtt.GetTempPath().c_str(),pAtt.GetSrcPath().c_str()));
			return FALSE;
		}
		
	}

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

	if(fpFtpUpload==NULL)
		fpFtpUpload=(FPFtpUpload)GetProcAddress(hFtpCli,"FtpUpload");
	
	if(fpFtpUpload==NULL)
		return FALSE;

	if(m_pItem)
	{
		CComVariant varResult;
		HRESULT hr=AdapterCommon::AutoWrap(DISPATCH_PROPERTYGET,&varResult,m_pItem,L"BodyFormat",0);
		if(SUCCEEDED(hr)&&varResult.intVal==2)
			m_bIsHtml=true;
	}

	return TRUE;
}
*/
BOOL ApprovalAdapter::AttachWithApprovalObligation(AdapterCommon::Attachment *pAtt,AdapterCommon::Obligation& obligation)
{
	HRESULT hr=S_OK;
#ifdef _DEBUG
	{
		AdapterCommon::Obligation ftpOb;
		ftpOb.SetName(m_pAdapter->GetObligationName().c_str());
		ftpOb.AddAttribute(AdapterCommon::Attribute(this->OBLIGATION_ATTRNAME_SOURCE,L"//hz-ts02/upload/jjin/test.txt"));
		ftpOb.AddAttribute(AdapterCommon::Attribute(this->OBLIGATION_ATTRNAME_USER,L"jjin"));
		ftpOb.AddAttribute(AdapterCommon::Attribute(this->OBLIGATION_ATTRNAME_APPROVERS,L"john.tyler@lab01.cn.nextlabs.com;jimmy.carter@lab01.cn.nextlabs.com"));
		ftpOb.AddAttribute(AdapterCommon::Attribute(this->OBLIGATION_ATTRNAME_RECIPIENTS,L"john.tyler@lab01.cn.nextlabs.com;james.polk@lab01.cn.nextlabs.com"));
		ftpOb.AddAttribute(AdapterCommon::Attribute(this->OBLIGATION_ATTRNAME_FTPDIR,L"lab01-sps07/approved"));
		ftpOb.AddAttribute(AdapterCommon::Attribute(this->OBLIGATION_ATTRNAME_FTPUSER,L"jjin"));
		ftpOb.AddAttribute(AdapterCommon::Attribute(this->OBLIGATION_ATTRNAME_FTPPASSWD,L"daxiea!110"));
		pAtt->AddObligation(ftpOb);

	}
#endif
	size_t iIndex=0,obCount=pAtt->Count();
	if(obCount==0)
		return FALSE;
	for(iIndex=0;iIndex<obCount;iIndex++)
	{
		hr=S_OK;
		AdapterCommon::Obligation ob=pAtt->Item((int)iIndex);	
		if(m_pAdapter->GetObligationName()==ob.GetName())
		{
			obligation=ob;
			return TRUE;
			/*if(ob.FindAttribute(this->OBLIGATION_ATTRNAME_FTPSERVER,wstrFtpServer)==false)
				return FALSE;
			if(ob.FindAttribute(this->OBLIGATION_ATTRNAME_FTPUSER,wstrUser)==false)
				return FALSE;
			if(ob.FindAttribute(this->OBLIGATION_ATTRNAME_FTPPASSWORD,wstrPasswd)==false)
				return FALSE;*/
		}
	}
	return FALSE;
}
STDAPI RepositoryUpload(CComPtr<IDispatch> pItem,AdapterCommon::Attachments* pAtts)
{
	HRESULT hr=S_OK;
	

	AdapterCommon::Adapter* pAdapter=GetAdapter();
	if(pAdapter==NULL)
	{
		DP((L"Failed to Get Adapter in RepositoryUpload"));
		return S_FALSE;
	}
	
	ApprovalAdapter approvalAdapter(pItem,pAtts,pAdapter);
	/*if(ftpAdapter.Init()==FALSE)
	{
		DP((L"Failed to Init for FTP adapter"));
		return S_FALSE;
	}
	BOOL bRet=ftpAdapter.UploadAll();
	*/
	size_t iIndex=0,iCount=pAtts->Count();
	AdapterCommon::Obligation ob;
	for(iIndex=0;iIndex<iCount;iIndex++)
	{
		AdapterCommon::Attachment& pAtt=(pAtts->Item(iIndex));
		if(approvalAdapter.AttachWithApprovalObligation(&pAtt,ob)==TRUE)
		{
			DP((L"Find approval obligation on attachment %s[SourcePath=%s]",pAtt.GetTempPath().c_str(),pAtt.GetSrcPath().c_str()));
			break;
		}
	}
	if(iIndex==iCount)
		return S_OK;

	std::wstring strSource;
	for(iIndex=0;iIndex<iCount;iIndex++)
	{
		AdapterCommon::Attachment& pAtt=(pAtts->Item(iIndex));
		if(iIndex)
			strSource += L";";
		strSource += pAtt.GetSrcPath();		
	}
	//generate the temp file in which the email message information stored
	std::wstring strPathForMessageFile;
	WCHAR wzTargetPath[MAX_PATH+1]=L"";
	if(SHGetSpecialFolderPath(NULL, wzTargetPath, CSIDL_PERSONAL, FALSE)==TRUE)
	{
		strPathForMessageFile=wzTargetPath;
	}
	else
	{
		::GetCurrentDirectory(MAX_PATH,wzTargetPath);
		strPathForMessageFile=wzTargetPath;
	}
	GUID guid;
	WCHAR guidStr[128]=L"";
	hr=CoCreateGuid(&guid);
	::StringFromGUID2(guid,guidStr,128);
	strPathForMessageFile +=L"\\";
	strPathForMessageFile +=guidStr;

	HANDLE hOpenFile=(HANDLE)CreateFile(strPathForMessageFile.c_str(),GENERIC_READ|GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
	if(hOpenFile==INVALID_HANDLE_VALUE)
	{
		strPathForMessageFile=L"";
		DP((L"Can't create file for message file which will result in the Subject/Content fields blank on co.exe's UI"));
	}
	else
	{
		CComVariant varSubject,varBody;
		std::wstring strSubject,strBody;
		hr=AdapterCommon::AutoWrap(DISPATCH_PROPERTYGET,&varSubject,pItem,L"Subject",0);
		hr=AdapterCommon::AutoWrap(DISPATCH_PROPERTYGET,&varBody,pItem,L"Body",0);
		strSubject=varSubject.bstrVal;
		strSubject+= L"\n";
		strBody=varBody.bstrVal;
		//Convert Wide char to multibyte char
		int iMinSize=::WideCharToMultiByte(CP_UTF8,NULL,strSubject.c_str(),-1,NULL,0,NULL,FALSE);
		char *pchSubject=new char[iMinSize+1];
		::ZeroMemory(pchSubject,iMinSize+1);
		::WideCharToMultiByte(CP_UTF8,NULL,strSubject.c_str(),-1,pchSubject,iMinSize,NULL,FALSE);
		DWORD dwSize=iMinSize;
		::WriteFile(hOpenFile,pchSubject,dwSize-1,&dwSize,NULL);
		delete[] pchSubject;pchSubject=NULL;

		iMinSize=::WideCharToMultiByte(CP_UTF8,NULL,strBody.c_str(),-1,NULL,0,NULL,FALSE);
		char *pchBody=new char[iMinSize+1];
		::ZeroMemory(pchBody,iMinSize+1);
		::WideCharToMultiByte(CP_UTF8,NULL,strBody.c_str(),-1,pchBody,iMinSize,NULL,FALSE);
		dwSize=iMinSize;
		::WriteFile(hOpenFile,pchBody,dwSize,&dwSize,NULL);
		delete[] pchBody;pchBody=NULL;
		
		CloseHandle(hOpenFile);

	}
	//open file
	/*std::wofstream fo(strPathForMessageFile.c_str());
	if(fo.fail())
	{
		strPathForMessageFile=L"";
		fo.clear();
	}
	else
	{
		CComVariant varSubject,varBody;
		std::wstring strSubject,strBody;
		hr=AdapterCommon::AutoWrap(DISPATCH_PROPERTYGET,&varSubject,pItem,L"Subject",0);
		hr=AdapterCommon::AutoWrap(DISPATCH_PROPERTYGET,&varBody,pItem,L"Body",0);
		strSubject=varSubject.bstrVal;
		strSubject+= L"\n";
		strBody=varBody.bstrVal;
		std::wstring::iterator itBody;
		for(itBody=strBody.begin();itBody!=strBody.end();itBody++)
		{
			if(*itBody==8216||*itBody==8217)
				*itBody=0x27;
		}
		fo.write(strSubject.c_str(),strSubject.length());
		size_t len=strBody.length();
		fo.write(strBody.c_str(),len);
		fo.close();
		fo.clear();
	}*/

	WCHAR wzModuleFileName[MAX_PATH+1];memset(wzModuleFileName,0,sizeof(wzModuleFileName));
	DWORD dwRet=GetModuleFileName(g_hInstance,wzModuleFileName,MAX_PATH);
	
	if(dwRet==0||GetLastError() == ERROR_INSUFFICIENT_BUFFER)
		return S_FALSE;

	std::wstring strCmdLine,strCmdLineTemp;
	if(GetModuleBaseName(strCmdLineTemp)==FALSE)
		return S_FALSE;

	WCHAR wzCmdLine[3*1024];memset(wzCmdLine,0,sizeof(wzCmdLine));
	std::wstring strAttValue;
	strCmdLine  = L"\"";
	strCmdLine +=strCmdLineTemp;
	strCmdLine +=L"\\co.exe";
	strCmdLine +=L"\"";
	//-Source
	/*if(ob.FindAttribute(ApprovalAdapter::OBLIGATION_ATTRNAME_SOURCE,strAttValue)==true)
	{*/
	if(strSource.length())
	{
		strCmdLine += L" -Source \"";
		strCmdLine += strSource;
		strCmdLine += L"\"";
	}
	/*}
	else
		return S_OK;*/

	//-messagefile
	if(strPathForMessageFile.length())
	{
		strCmdLine += L" -messagefile \"";
		strCmdLine += strPathForMessageFile;
		strCmdLine += L"\"";
	}

	//-user
	if(ob.FindAttribute(ApprovalAdapter::OBLIGATION_ATTRNAME_USER,strAttValue)==true)
	{
		strCmdLine += L" -user ";
		strCmdLine += strAttValue;
	}
	else
		return S_OK;

	
	//-approvers
	if(ob.FindAttribute(ApprovalAdapter::OBLIGATION_ATTRNAME_APPROVERS,strAttValue)==true)
	{
		strCmdLine += L" -approvers ";
		strCmdLine += strAttValue;
	}
	else
		return S_OK;

	//-recipients
	if(ob.FindAttribute(ApprovalAdapter::OBLIGATION_ATTRNAME_RECIPIENTS,strAttValue)==true)
	{
		strCmdLine += L" -recipients ";
		strCmdLine += strAttValue;
	}
	else
		return S_OK;

	//-ftpdir
	if(ob.FindAttribute(ApprovalAdapter::OBLIGATION_ATTRNAME_FTPDIR,strAttValue)==true)
	{
		strCmdLine += L" -ftpdir \"";
		strCmdLine += strAttValue;
		strCmdLine += L"\"";
	}
	else
		return S_OK;

	//-ftpuser
	if(ob.FindAttribute(ApprovalAdapter::OBLIGATION_ATTRNAME_FTPUSER,strAttValue)==true)
	{
		strCmdLine += L" -ftpuser ";
		strCmdLine += strAttValue;
	}
	else
		return S_OK;

	//-ftppasswd
	if(ob.FindAttribute(ApprovalAdapter::OBLIGATION_ATTRNAME_FTPPASSWD,strAttValue)==true)
	{
		strCmdLine += L" -ftppasswd ";
		strCmdLine += strAttValue;
	}
	else
		return S_OK;
	
	DP((L"command line:%s",strCmdLine.c_str()));
	wcsncpy_s(wzCmdLine, _countof(wzCmdLine), strCmdLine.c_str(), _TRUNCATE);
	STARTUPINFO startupInfo;
	memset(&startupInfo,0,sizeof(startupInfo));
	startupInfo.cb=sizeof(startupInfo);
	startupInfo.wShowWindow=SW_SHOWNORMAL;
	PROCESS_INFORMATION processInfo;
#pragma warning(push)
#pragma warning(disable: 6335 6053)
	::CreateProcess(NULL,wzCmdLine,NULL,NULL,FALSE,NORMAL_PRIORITY_CLASS,NULL,NULL,&startupInfo,&processInfo);
#pragma warning(pop)
	delete pAdapter;
	
	return S_FALSE;
}