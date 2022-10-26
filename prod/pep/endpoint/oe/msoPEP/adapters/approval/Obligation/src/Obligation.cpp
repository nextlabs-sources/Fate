     // Obligation.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "fileprocess.h"
#include "Obligation.h"
#include "FtpDlg.h"
#include "EMailDlg.h"
#include "Removable.h"
#include "WinAD.h"
#define SECURITY_WIN32
#include <security.h>

#include <secext.h>
#include <shlwapi.h>

#define PARSECODE	L"\""

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

const WCHAR CObligationApp::PARA_SOURCE[]=L"-source";
const WCHAR CObligationApp::PARA_USER[]=L"-user";
const WCHAR CObligationApp::PARA_RECIPIENTS[]=L"-recipients";
const WCHAR CObligationApp::PARA_APPROVERS[]=L"-approvers";
const WCHAR CObligationApp::PARA_FTPDIR[]=L"-ftpdir";
const WCHAR CObligationApp::PARA_FTPUSER[]=L"-ftpuser";
const WCHAR CObligationApp::PARA_FTPPASSWD[]=L"-ftppasswd";
const WCHAR CObligationApp::PARA_MESSAGEFILE[]=L"-messagefile";
// CObligationApp

BEGIN_MESSAGE_MAP(CObligationApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CObligationApp construction

CObligationApp::CObligationApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CObligationApp object

CObligationApp theApp;
BaseArgument g_BaseArgument;


// CObligationApp initialization


 const WCHAR* gMutexName = L"NextLabs_FileObligation";
 HANDLE gMutex = NULL;

const wchar_t* g_strFtpType=L"FTP";
const wchar_t* g_strEmailType=L"Email";
const wchar_t* g_strReMedia=L"Removable Media";

BOOL CObligationApp::InitInstance()
{
	gMutex = ::CreateMutexW(NULL, FALSE, gMutexName);   
	if(GetLastError() == ERROR_ALREADY_EXISTS )   
	{   
		return FALSE;   
	}  
	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

	AfxEnableControlContainer();

	SetRegistryKey(_T("Local AppWizard-Generated Applications"));


	LPWSTR wstrCommandLine = ::GetCommandLineW();
	std::wstring strLine = wstrCommandLine;
	//CLog::WriteLog(L"the command line ",strLine.c_str());

#ifdef _DEBUG
	std::wstring strMsg = L"the argument list is:\t";
	strMsg += strLine;
	MessageBox(NULL,strMsg.c_str(),L"Debug Msg",MB_OK);
#endif
	
	//////////////////////////////////////////////////////////////////////////
	//# Maybe the value contain any spaces , so value will be enclosed in quotes. (reference co.pdf Page 139)
	//# eg: if the type is FTP 
	//strLine = L"Co.exe -encpasswd \"hello123\" -archive \"jimmy@nextlabs.com\" -quardir \"\\\\10.86.32.102\\share\\Quarantine\" -appdir \"\\\\10.86.32.102\\share\\Approval\" -user \"dhe\" -source \"E:/outlook_proj/Co parameters.doc\" -apptype \"FtP\"";
	
	//strLine = L"Co.exe  -recipient toxyboy;tonny -project \"Motorla_razer3;Kodak_cam4x\" -approver \"jimmy.carter@lab01.cn.nextlabs.com\" -archive \"jimmy.carter@lab01.cn.nextlabs.com\" -encpasswd \"hello123\" -quardir \"\\\\cn.nextlabs.com\\share\\users\\flex\\quarantine\" -appdir \"\\\\cn.nextlabs.com\\share\\users\\flex\\Approval\" -user \"dhe\" -source \"E:/outlook_proj/Co parameters.doc\" -apptype \"Email\"";
	
	
	//strLine = L"co.exe -archive \"gavin.ye@nextlabs.com\" -quardir \"\\\\cn.nextlabs.com\\share\\users\\flex\\quarantine\" -appdir \"\\\\cn.nextlabs.com\\share\\users\\flex\\approval\"  -apptype \"fTp\" -user \"dhe\" -source \"E:/outlook_proj/test.txt\"";

	//strLine = L"co.exe -recipient toxyboy;tonny -project \"Motorla_razer3;Kodak_cam4x\" -approver \"gavin.ye@cn.nextlabs.com;jim.jin@cn.nextlabs.com\" -pubkey \"\\\\demoserver20.demo20.nextlabs.com\\share\\key\" -encpasswd \"hello123\" -archive \"gavin.ye@nextlabs.com\" -quardir \"\\\\cn.nextlabs.com\\share\\users\\flex\\quarantine\" -appdir \"\\\\cn.nextlabs.com\\share\\users\\flex\\approval\"  -apptype \"EmaIl\" -source \"E:/outlook_proj/test.txt\" -user \"dhe\"";
	
	//strLine = L"co2003.exe -project \"Motorla_razer3\" -source \"c:\\log.txt\" -approver \"george.washington@demo20.nextlabs.com\" -pubkey \"\\\\demoserver20.demo20.nextlabs.com\\share\\key\" -quardir \"\\\\demo20-ftp\\Share\\Customer\\Shared_Information\\Motorla\\Quarantine\\Removable_Media\" -appdir \"\\\\demo20-ftp\\Share\\Customer\\Shared_Information\\Motorla\\Approved\\Removable_Media\"  -apptype \"Removable Media\" -user \"Banker Adams\"";
	//Requester's Email  is:john.adams@demo20.nextlabs.com";
	//strLine = L" -source $CESource -user $CEUser -recipient -project Motorola -approver jimmy.carter@demo20.nextlabs.com -quardir \\demo20-ftp\Share\Customer\Shared_Information\Motorla\Quarantine\Removable_Media -appdir \\demo20-ftp\Share\Customer\Shared_Information\Motorla\Approved\Removable_Media -apptype Removable Media -archive -pubkey \\demo20-ftp\Share\Customer\Shared_Information\Motorla\key -encpasswd";
	////# eg: if the type is Email
	//strLine = L"Co.exe -recipient toxyboy;tonny -project \"Motorla_razer3;Kodak_cam4x\" -approver \"gavin.ye@cn.nextlabs.com\" -archive \"gavin.ye@cn.nextlabs.com\" -pubkey \"\\\\demoserver20.demo20.nextlabs.com\\share\\key\" -quardir \"";
	//strLine += L"\\\\10.86.32.102\\share\\Quarantine\" -appdir \"\\\\10.86.32.102\\share\\Approval\"  -apptype \"fTp\" -source \"E:/outlook_proj/Co parameters.doc\" -user \"dhe\"";

	////# eg: if the type is Removable Media
	//strLine =L"Co.exe -project \"Motorla_razer3;Kodak_cam4x\" -approver \"jimmy.carter@lab01.cn.nextlabs.com;John.Tyler@lab01.cn.nextlabs.com\" -pubkey \"\\\\10.86.32.102\\share\\key\\gye_pub_key.cer\" -archive \"jimmy.carter@lab01.cn.nextlabs.com\" -quardir \"";
	//strLine +=   L"\\\\10.86.32.102\\share\\Quarantine\" -appdir \"\\\\10.86.32.102\\share\\Approval\"  -apptype \"Removable mEdia\" -source \"E:/outlook_proj/Co parameters.doc\" -user \"dhe\"";

	//strLine = L"-project \"Gena\" -approver \"lyndon.johnson@qapf.qalab01.nextlabs.com\" -pubkey \"\\\\pf-cc1\\Flex\\key\" ";
	//strLine += L"-encpasswd \"hello123\" -archive \"ljohnson@qapf.qalab01.nextlabs.com\" -quardir \"\\\\pf-cc1\\Flex\\quarantine\" -appdir \"\\\\pf-cc1\\Flex\\approval\" ";
	//strLine += L"-apptype \"Removable Medi\" -user \"John Tyler\"";
	//////////////////////////////////////////////////////////////////////////

	//strLine = L"co.exe -recipient \"vishi@yahoo.com\" -project \"Motorla_razer3\" -pubkey\"\\\\adlab-sps03\\amat-data\\Workflow\\Public Key\\testkey.asc\" -approver \"adlab-next02@amat.com\"  -archive \"john.tyler@nextlabs.com\" -quardir "; 
	//strLine += L"\"\\\\adlab-sps03\\amat-data\\Workflow\\QUAR\" -appdir \"\\\\adlab-sps03\\amat-data\\Workflow\\Approved\" -apptype \"EMail\" -user \"adlab-next01\" -source \"c:/time.txt\"";

	LPWSTR* argv;
	int argc=0;
	BaseArgumentFlex baseArgumentFlex;
	//ZeroMemory(&baseArgumentFlex,sizeof(baseArgumentFlex));
	argv=::CommandLineToArgvW(::GetCommandLineW(),&argc);
	if(NULL==argv) return FALSE;
	
	//bool bTrue = ParseCommandLineFlex(strLine.c_str());
	bool bTrue = ParseCommandLineFlex(argc,argv,baseArgumentFlex);
	if(!bTrue)		return FALSE;

	//bTrue = CheckAvailable();
	//if(!bTrue)	return FALSE;
	WCHAR wstrUserName[256] = {0};
	ULONG lUserName=_countof(wstrUserName);
	ZeroMemory(wstrUserName,sizeof(wstrUserName));
	BOOL bName=::GetUserNameEx(NameUserPrincipal,wstrUserName,&lUserName);
	if(bName)
		baseArgumentFlex.wstrRequesterAddress=wstrUserName;
	
	bName=::GetUserNameEx(NameDisplay,wstrUserName,&lUserName);
	if(bName)
		baseArgumentFlex.wstrRequesterDisplayName=wstrUserName;
	//bTrue = DoWork();
	{
		CWinAD theWinAD;
		std::wstring strKeyWord = L"(sAMAccountName=";
		wchar_t wsuser[256]=L"\0";
		DWORD dwlen = 256;
		::GetUserName(wsuser,&dwlen);
		baseArgumentFlex.wstrCurUserName = wsuser;
		strKeyWord += wsuser;	//g_BaseArgument.wstrCurUserName;
		strKeyWord += L")";
		if(!theWinAD.SearchUserInfo(baseArgumentFlex.wstrRequesterAddress,baseArgumentFlex.wstrCurSID,strKeyWord.c_str()))
		{
			//std::wstring strMsg = L"Get Requester SID failed by (sAMAccountName = ";
			//strMsg += g_BaseArgument.wstrCurUserName;
			std::wstring strMsg = L"\"-user ";
			strMsg += baseArgumentFlex.wstrCurUserName;
			strMsg += L" \" could not be determined.Please contact the policy administrator";
			MessageBoxW(NULL,strMsg.c_str(),L"Error",MB_OK);
			CLog::WriteLog(L"Lookup the user SID and Email address failed! the user",wsuser);
			return false;
		}
	}
	bTrue=DoWork(baseArgumentFlex);
	LocalFree(argv);
	return bTrue;
}
bool CObligationApp::ParseCommandLineFlex(int argc,LPWSTR*argv,BaseArgumentFlex & baseArguFlex)
{
	bool bRet=true;
	std::wstring wstrCmd;
	int i=1;
	for(i=1;i<argc;i++)
	{
		if(_wcsnicmp(argv[i],PARA_SOURCE,wcslen(PARA_SOURCE))==0)
		{
			if(argv[i+1][0]==L'-')
				continue;
			WCHAR wzSourceTemp[1024*4]=L"";
			baseArguFlex.wstrSource=argv[++i];
			int len=(int)baseArguFlex.wstrSource.length();
			/*if(argv[i][0]==L'/'&&argv[i][1]==L'/')
			{*/
				wcsncpy_s(wzSourceTemp,_countof(wzSourceTemp),argv[i], _TRUNCATE);
				int j=0;
				for(j=0;j<(int)baseArguFlex.wstrSource.length();j++)
				{
					if(wzSourceTemp[j]==L'/')
						wzSourceTemp[j]=L'\\';
				}
				baseArguFlex.wstrSource=wzSourceTemp;
			/*}*/
#define WDE_ADDITIONAL_STRING_AT_END	L"\\*.*"
			wcsncpy_s(wzSourceTemp,_countof(wzSourceTemp),baseArguFlex.wstrSource.c_str(), _TRUNCATE);
			if(_wcsnicmp(wzSourceTemp+len-wcslen(WDE_ADDITIONAL_STRING_AT_END),
					   WDE_ADDITIONAL_STRING_AT_END,
					   wcslen(WDE_ADDITIONAL_STRING_AT_END))==0)
			{
				wzSourceTemp[len-wcslen(WDE_ADDITIONAL_STRING_AT_END)]=0;
				baseArguFlex.wstrSource=wzSourceTemp;
			}
			wstrCmd+=PARA_SOURCE;
			wstrCmd+=L" ";
			wstrCmd+=baseArguFlex.wstrSource;
		}
		else if(_wcsnicmp(argv[i],PARA_USER,wcslen(PARA_USER))==0)
		{
			baseArguFlex.wstrUser=argv[++i];
			baseArguFlex.wstrRequesterAddress=baseArguFlex.wstrUser;
			wstrCmd+=PARA_USER;
			wstrCmd+=L" ";
			wstrCmd+=baseArguFlex.wstrRequesterAddress;
		}
		else if(_wcsnicmp(argv[i],PARA_APPROVERS,wcslen(PARA_APPROVERS))==0)
		{
			baseArguFlex.wstrApprovers=argv[++i];
			wstrCmd+=PARA_APPROVERS;
			wstrCmd+=L" ";
			wstrCmd+=baseArguFlex.wstrApprovers;
		}
		else if(_wcsnicmp(argv[i],PARA_FTPDIR,wcslen(PARA_FTPDIR))==0)
		{
			baseArguFlex.wstrFtpDir=argv[++i];
			wstrCmd+=PARA_FTPDIR;
			wstrCmd+=L" ";
			wstrCmd+=baseArguFlex.wstrFtpDir;
		}
		else if(_wcsnicmp(argv[i],PARA_FTPPASSWD,wcslen(PARA_FTPPASSWD))==0)
		{
			baseArguFlex.wstrFtpPasswd=argv[++i];
			wstrCmd+=PARA_FTPPASSWD;
			wstrCmd+=L" ******";
		}
		else if(_wcsnicmp(argv[i],PARA_FTPUSER,wcslen(PARA_FTPUSER))==0)
		{
			baseArguFlex.wstrFtpUser=argv[++i];
			wstrCmd+=PARA_FTPUSER;
			wstrCmd+=L" ***";
		}
		else if(_wcsnicmp(argv[i],PARA_RECIPIENTS,wcslen(PARA_RECIPIENTS))==0)
		{
			baseArguFlex.wstrRecipients=argv[++i];
			wstrCmd+=PARA_RECIPIENTS;
			wstrCmd+=L" ";
			wstrCmd+=baseArguFlex.wstrRecipients;
			
		}
		else if(_wcsnicmp(argv[i],PARA_MESSAGEFILE,wcslen(PARA_MESSAGEFILE))==0)
		{
			baseArguFlex.wstrMessageFile=argv[++i];
			wstrCmd+=PARA_MESSAGEFILE;
			wstrCmd+=L" ";
			wstrCmd+=baseArguFlex.wstrMessageFile;
		}
		else
		{
			if(argv[i][0]!=L'-')
				bRet=false;
		}
			
	}
	CLog::WriteLog(L"Command Line",wstrCmd.c_str());

	return bRet;
}
// Parse CommandLine and initialize the Base Argument in according to CommandLine
bool CObligationApp::ParseCommandLine(const wchar_t* pCommandLine)
{
	std::wstring strLine(pCommandLine);
	std::wstring::size_type nstart = 0;
	std::wstring::size_type  nend = 0;

	// quarantine dir
	const wchar_t* wstrname=L"-quardir \"";
	if((nstart = strLine.find(wstrname)) == std::wstring::npos)	
	{
		MessageBox(NULL,L"Quarantine directory parse failed!",L"Error",MB_OK);
		CLog::WriteLog(L"Quarantine directory parse failed! the command line",strLine.c_str());
		return false;
	}
	nstart += wcslen(wstrname);
	nend =strLine.find(PARSECODE,nstart);
	if(nend == std::wstring::npos)
		g_BaseArgument.wstrQuarantineDir = strLine.substr(nstart,strLine.length()-nstart);
	else 
		g_BaseArgument.wstrQuarantineDir = strLine.substr(nstart,nend-nstart);
	if(g_BaseArgument.wstrQuarantineDir.empty())
	{
		MessageBox(NULL,L"Quarantine directory is empty!",L"Error",MB_OK);
		CLog::WriteLog(L"Error",L"Quarantine directory is empty!");
		return false;
	}

	// app dir
	wstrname = L"-appdir \"";
	if((nstart = strLine.find(wstrname)) == std::wstring::npos)	
	{
		MessageBox(NULL,L"Approval directory parse failed!",L"Error",MB_OK);
		CLog::WriteLog(L"Approval directory parse failed! the command line",strLine.c_str());
		return false;
	}
	nstart += wcslen(wstrname);
	nend = strLine.find(PARSECODE,nstart);
	if(nend == std::wstring::npos)
		g_BaseArgument.wstrApprDir = strLine.substr(nstart,strLine.length()-nstart);
	else 
		g_BaseArgument.wstrApprDir = strLine.substr(nstart,nend-nstart);
	if(g_BaseArgument.wstrApprDir.empty())
	{
		MessageBox(NULL,L"Approval directory is empty!",L"Error",MB_OK);
		CLog::WriteLog(L"Error",L"Approval directory is empty!");
		return false;
	}

	// archival ID
	wstrname = L"-archive \"";
	if((nstart = strLine.find(wstrname)) != std::wstring::npos)	
	{
		nstart += wcslen(wstrname);
		nend =strLine.find(PARSECODE,nstart);
		if(nend == std::wstring::npos)
			g_BaseArgument.wstrArchivalID = strLine.substr(nstart,strLine.length()-nstart);
		else 
			g_BaseArgument.wstrArchivalID = strLine.substr(nstart,nend-nstart);
	}

	// app type
	wstrname = L"-apptype \"";
	if((nstart = strLine.find(wstrname)) == std::wstring::npos)	
	{
		MessageBox(NULL,L"App type parse failed!",L"Error",MB_OK);
		CLog::WriteLog(L"App type parse failed! the command line",strLine.c_str());
		return false;
	}
	nstart += wcslen(wstrname);
	nend =strLine.find(PARSECODE,nstart);
	if(nend == std::wstring::npos)
		g_BaseArgument.wstrApprType = strLine.substr(nstart,strLine.length()-nstart);
	else 
		g_BaseArgument.wstrApprType = strLine.substr(nstart,nend-nstart);
	if(g_BaseArgument.wstrApprType.empty())
	{
		MessageBox(NULL,L"App type is empty!",L"Error",MB_OK);
		CLog::WriteLog(L"Error",L"App type is empty!");
		return false;
	}

	// source file
	wstrname = L"-source \"";
	if((nstart = strLine.find(wstrname)) == std::wstring::npos)	
	{
		MessageBox(NULL,L"Source file parse failed!",L"Error",MB_OK);
		CLog::WriteLog(L"Source file parse failed! the command line",strLine.c_str());
		return false;
	}
	nstart += wcslen(wstrname);
	nend = strLine.find(PARSECODE,nstart);
	if(nend == std::wstring::npos)
		g_BaseArgument.wstrDenyFile = strLine.substr(nstart,strLine.length()-nstart);
	else 
		g_BaseArgument.wstrDenyFile = strLine.substr(nstart,nend-nstart);
	if(g_BaseArgument.wstrDenyFile.empty())
	{
		MessageBox(NULL,L"Source file is empty!",L"Error",MB_OK);
		CLog::WriteLog(L"Error",L"Source file is empty!");
		return false;
	}

	for (unsigned int i=0; i < g_BaseArgument.wstrDenyFile.length(); i++)
	{
		if(g_BaseArgument.wstrDenyFile[i] == L'/')
			g_BaseArgument.wstrDenyFile[i]=L'\\'; 
	}	

	// user name
	wstrname = L"-user \"";
	if((nstart = strLine.find(wstrname)) == std::wstring::npos)	
	{
		MessageBox(NULL,L"User name parse failed!",L"Error",MB_OK);
		CLog::WriteLog(L"User name parse failed! the command line",strLine.c_str());
		return false;
	}
	nstart += wcslen(wstrname);
	nend =strLine.find(PARSECODE,nstart);
	if(nend == std::wstring::npos)
		g_BaseArgument.wstrCurUserName = strLine.substr(nstart,strLine.length()-nstart);
	else 	
		g_BaseArgument.wstrCurUserName = strLine.substr(nstart,nend-nstart);
	if(g_BaseArgument.wstrCurUserName.empty())
	{
		MessageBox(NULL,L"User name is empty!",L"Error",MB_OK);
		CLog::WriteLog(L"Error",L"User name is empty!");
		return false;
	}

	// get user email and user SID by user name

	CWinAD theWinAD;
	std::wstring strKeyWord = L"(sAMAccountName=";
	wchar_t wsuser[256]=L"\0";
	DWORD dwlen = 256;
	::GetUserName(wsuser,&dwlen);
	g_BaseArgument.wstrCurUserName = wsuser;
	strKeyWord += wsuser;	//g_BaseArgument.wstrCurUserName;
	strKeyWord += L")";
	if(!theWinAD.SearchUserInfo(g_BaseArgument.wstrCurAddress,g_BaseArgument.wstrCurSID,strKeyWord.c_str()))
	{
		//std::wstring strMsg = L"Get Requester SID failed by (sAMAccountName = ";
		//strMsg += g_BaseArgument.wstrCurUserName;
		std::wstring strMsg = L"\"-user ";
		strMsg += g_BaseArgument.wstrCurUserName;
		strMsg += L" \" could not be determined.Please contact the policy administrator";
		MessageBoxW(NULL,strMsg.c_str(),L"Error",MB_OK);
		CLog::WriteLog(L"Lookup the user SID and Email failed! the user",wsuser);
		return false;
	}

#ifdef _DEBUG
	std::wstring strMsg = L"User login name is :";
	strMsg += wsuser;
	//strMsg += g_BaseArgument.wstrCurUserName;
	strMsg += L"\t User EMail is:";
	strMsg += g_BaseArgument.wstrCurAddress;
	strMsg += L"\t User SID is:";
	strMsg += g_BaseArgument.wstrCurSID;
	MessageBox(NULL,strMsg.c_str(),L"Debug Msg",MB_OK);
#endif

	CLog::WriteLog(L"Requester's Email ",g_BaseArgument.wstrCurAddress.c_str());
	CLog::WriteLog(L"Requester's SID ",g_BaseArgument.wstrCurSID.c_str());

	if(_wcsicmp(g_BaseArgument.wstrApprType.c_str(),g_strReMedia) == 0)	// Removable Media
	{
		// customer
		wstrname = L"-project \"";
		if((nstart = strLine.find(wstrname)) == std::wstring::npos)	
		{
			MessageBox(NULL,L"App type is RemovableMedia ,project is not exist!",L"Error",MB_OK);
			CLog::WriteLog(L"Project parse failed! the command line",strLine.c_str());
			return false;
		}
		nstart += wcslen(wstrname);
		nend =strLine.find(PARSECODE,nstart);
		if(nend == std::wstring::npos)
			g_BaseArgument.wstrCustomer = strLine.substr(nstart,strLine.length()-nstart);
		else 
			g_BaseArgument.wstrCustomer = strLine.substr(nstart ,nend-nstart);
		if(g_BaseArgument.wstrCustomer.empty())
		{
			MessageBox(NULL,L"Project is empty!",L"Error",MB_OK);
			CLog::WriteLog(L"Error",L"Project is empty!");
			return false;
		}

		// approver email address
		wstrname = L"-approver \"";
		if((nstart = strLine.find(wstrname)) == std::wstring::npos)	
		{
			MessageBox(NULL,L"App type is RemovableMedia ,approver is not exist!",L"Error",MB_OK);
			CLog::WriteLog(L"approver parse failed! the command line",strLine.c_str());
			return false;
		}
		nstart += wcslen(wstrname);
		nend =strLine.find(PARSECODE,nstart);
		if(nend == std::wstring::npos)
			g_BaseArgument.wstrApprEMail = strLine.substr(nstart,strLine.length()-nstart);
		else 
			g_BaseArgument.wstrApprEMail = strLine.substr(nstart ,nend-nstart);
		if(g_BaseArgument.wstrApprEMail.empty())
		{
			MessageBox(NULL,L"approver is empty!",L"Error",MB_OK);
			CLog::WriteLog(L"Error",L"approver is empty!");
			return false;
		}	
		// here we need parse to get severel approver email
		std::wstring wstrApproverEmail,wstrApproverSID; 
		std::wstring::size_type start=0,end=0;
		while(true)
		{
			end = g_BaseArgument.wstrApprEMail.find(L';',start);
			if(end == std::wstring::npos)	
				wstrApproverEmail = g_BaseArgument.wstrApprEMail.substr(start,g_BaseArgument.wstrApprEMail.length()-start);
			else
				wstrApproverEmail = g_BaseArgument.wstrApprEMail.substr(start,end-start);
			strKeyWord=L"(mail=";
			strKeyWord += wstrApproverEmail;
			strKeyWord += L")";	
			if(!theWinAD.SearchUserInfo(wstrApproverEmail,wstrApproverSID,strKeyWord.c_str()))
			{
				std::wstring strError = L"\"-approver ";
				strError += wstrApproverEmail;
				strError += L"\" could not be determined.Please contact the policy administrator";
				MessageBoxW(NULL,L"",L"Error",MB_OK);
				CLog::WriteLog(L"Get Approver's SID failed! the Approver's EMail Address",wstrApproverEmail.c_str());
				return false;
			}
#ifdef _DEBUG
			std::wstring strMsg = L"Get Approver SID by Email.Approver email is:";
			strMsg += wstrApproverEmail;
			strMsg += L"\t SID is:";
			strMsg += wstrApproverSID;
			MessageBox(NULL,strMsg.c_str(),L"Debug Msg",MB_OK);
#endif
			CLog::WriteLog(L"Approver's Email ",wstrApproverEmail.c_str());
			CLog::WriteLog(L"Approver's SID ",wstrApproverSID.c_str());

			g_BaseArgument.approverVector.push_back(FilePair(wstrApproverEmail,wstrApproverSID));
			if(end == std::wstring::npos)	break;
			start = end+1;
		}

		// customer key or pwd
		wstrname = L"-pubkey \"";
		if((nstart = strLine.find(wstrname)) != std::wstring::npos)
		{
			nstart += wcslen(wstrname);
			nend =strLine.find(PARSECODE,nstart);
			if(nend == std::wstring::npos)
				g_BaseArgument.wstrCusetomerKey = strLine.substr(nstart,strLine.length()-nstart);
			else 
				g_BaseArgument.wstrCusetomerKey = strLine.substr(nstart,nend-nstart);

		}
		else if((nstart = strLine.find(L"-encpasswd \"")) != std::wstring::npos)	
		{
			wstrname = L"-encpasswd \"";
			nstart += wcslen(wstrname);
			nend =strLine.find(PARSECODE,nstart);
			if(nend == std::wstring::npos)
				g_BaseArgument.wstrEncryptionPwd = strLine.substr(nstart,strLine.length()-nstart);
			else 
				g_BaseArgument.wstrEncryptionPwd = strLine.substr(nstart,nend-nstart);
		}
		else 
		{
			MessageBoxW(NULL,L"The data should be protected using encryption for approval. It needs public key or encryption password. Please contact your policy admin",L"Error",MB_OK);
			CLog::WriteLog(L"Get public key or encryption failed! the command line ",strLine.c_str());
			return false;
		}
	}
	else if(_wcsicmp(g_BaseArgument.wstrApprType.c_str(),g_strFtpType) == 0)
	{
		// approver email address
		wstrname = L"-approver \"";
		if((nstart = strLine.find(wstrname)) == std::wstring::npos)	
		{
			// approver email and approver SID by config file
			if(!theWinAD.GetManagerInfo(g_BaseArgument.wstrApprEMail,g_BaseArgument.wstrApprSid))
			{
#ifdef _DEBUG
				MessageBox(NULL,L"Please confirm AD include the manager of the current login user's!",L"",MB_OK);
#endif
				MessageBox(NULL,L"Get Approver's SID and EMail Failed,Please pass all the information used so that it could be debugged. It may be due to your access rights. Please contact your admin",L"Error",MB_OK);
				CLog::WriteLog(L"Error",L"Get Approver SID and EMail failed!");
				return false;
			}

#ifdef _DEBUG
			std::wstring strMsg = L"Get approver email and SID by configure.The Email is:";
			strMsg += g_BaseArgument.wstrApprEMail;
			strMsg += L"\t The SID is:";
			strMsg += g_BaseArgument.wstrApprSid;
			MessageBox(NULL,strMsg.c_str(),L"Debug Msg",MB_OK);
#endif
			CLog::WriteLog(L"Approver's Email ",g_BaseArgument.wstrApprEMail.c_str());
			CLog::WriteLog(L"Approver's SID ",g_BaseArgument.wstrApprSid.c_str());
			g_BaseArgument.approverVector.push_back(FilePair(g_BaseArgument.wstrApprEMail,g_BaseArgument.wstrApprSid));
		}
		else
		{
			nstart += wcslen(wstrname);
			nend =strLine.find(PARSECODE,nstart);
			if(nend == std::wstring::npos)
				g_BaseArgument.wstrApprEMail = strLine.substr(nstart,strLine.length()-nstart);
			else 
				g_BaseArgument.wstrApprEMail = strLine.substr(nstart ,nend-nstart);
			if(g_BaseArgument.wstrApprEMail.empty())
			{
				MessageBox(NULL,L"approver is empty!",L"Error",MB_OK);
				CLog::WriteLog(L"Error",L"approver is empty!");
				return false;
			}	
			// here we need parse to get severel approver email
			std::wstring wstrApproverEmail,wstrApproverSID; 
			std::wstring::size_type start=0,end=0;
			while(true)
			{
				end = g_BaseArgument.wstrApprEMail.find(L';',start);
				if(end == std::wstring::npos)	
					wstrApproverEmail = g_BaseArgument.wstrApprEMail.substr(start,g_BaseArgument.wstrApprEMail.length()-start);
				else
					wstrApproverEmail = g_BaseArgument.wstrApprEMail.substr(start,end-start);
				strKeyWord=L"(mail=";
				strKeyWord += wstrApproverEmail;
				strKeyWord += L")";	
				if(!theWinAD.SearchUserInfo(wstrApproverEmail,wstrApproverSID,strKeyWord.c_str()))
				{
					std::wstring strError = L"\"-approver ";
					strError += wstrApproverEmail;
					strError += L"\" could not be determined.Please contact the policy administrator";
					MessageBoxW(NULL,L"",L"Error",MB_OK);
					CLog::WriteLog(L"Get Approver's SID failed! the Approver's EMail Address",wstrApproverEmail.c_str());
					return false;
				}
#ifdef _DEBUG
				std::wstring strMsg = L"Get Approver SID by Email.Approver email is:";
				strMsg += wstrApproverEmail;
				strMsg += L"\t SID is:";
				strMsg += wstrApproverSID;
				MessageBox(NULL,strMsg.c_str(),L"Debug Msg",MB_OK);
#endif
				CLog::WriteLog(L"Approver's Email ",wstrApproverEmail.c_str());
				CLog::WriteLog(L"Approver's SID ",wstrApproverSID.c_str());

				g_BaseArgument.approverVector.push_back(FilePair(wstrApproverEmail,wstrApproverSID));
				if(end == std::wstring::npos)	break;
				start = end+1;
			}
		}
// add at 10-20 for all of the type support asymetic
// start from here
#if 0
		// pwd
		wstrname = L"-encpasswd \"";
		if((nstart = strLine.find(wstrname)) != std::wstring::npos)	
		{
			nstart += wcslen(wstrname);
			nend =strLine.find(PARSECODE,nstart);
			if(nend == std::wstring::npos)
				g_BaseArgument.wstrEncryptionPwd = strLine.substr(nstart,strLine.length()-nstart);
			else 
				g_BaseArgument.wstrEncryptionPwd = strLine.substr(nstart,nend-nstart);
		}
#endif

		// customer key or pwd
		wstrname = L"-pubkey \"";
		if((nstart = strLine.find(wstrname)) != std::wstring::npos)
		{
			nstart += wcslen(wstrname);
			nend =strLine.find(PARSECODE,nstart);
			if(nend == std::wstring::npos)
				g_BaseArgument.wstrCusetomerKey = strLine.substr(nstart,strLine.length()-nstart);
			else 
				g_BaseArgument.wstrCusetomerKey = strLine.substr(nstart,nend-nstart);
			g_BaseArgument.wstrEncryptionPwd.clear();
		}
		else if((nstart = strLine.find(L"-encpasswd \"")) != std::wstring::npos)	
		{
			wstrname = L"-encpasswd \"";
			nstart += wcslen(wstrname);
			nend =strLine.find(PARSECODE,nstart);
			if(nend == std::wstring::npos)
				g_BaseArgument.wstrEncryptionPwd = strLine.substr(nstart,strLine.length()-nstart);
			else 
				g_BaseArgument.wstrEncryptionPwd = strLine.substr(nstart,nend-nstart);
			g_BaseArgument.wstrCusetomerKey.clear();
		}
		else 
		{
			MessageBoxW(NULL,L"The data should be protected using encryption for approval. It needs public key or encryption password. Please contact your policy admin",L"Error",MB_OK);
			CLog::WriteLog(L"Get public key or encryption failed! the command line ",strLine.c_str());
			return false;
		}
// end at here

	}
	else if(_wcsicmp(g_BaseArgument.wstrApprType.c_str(),g_strEmailType)==0)
	{
		//recipient		
		wstrname = L"-recipient ";
		if((nstart = strLine.find(wstrname)) == std::wstring::npos)	
		{
			MessageBox(NULL,L"App type is EMail ,recipient is not exist!",L"Error",MB_OK);
			CLog::WriteLog(L"recipient parse failed! the command line ",strLine.c_str());
			return false;
		}
		nstart += wcslen(wstrname);
		CLog::WriteLog(L"PC bug, please note ",L"the recipient's value doesn't included by quote!");
		nend =strLine.find(L" -",nstart);
		if(nend == std::wstring::npos)
			g_BaseArgument.wstrRecipientUser = strLine.substr(nstart,strLine.length()-nstart);
		else 
			g_BaseArgument.wstrRecipientUser = strLine.substr(nstart,nend-nstart);	
		if(g_BaseArgument.wstrRecipientUser.empty())
		{
			MessageBox(NULL,L"recipient is empty!",L"Error",MB_OK);
			CLog::WriteLog(L"Error",L"recipient is empty!");
			return false;
		}

		// customer
		wstrname = L"-project \"";
		if((nstart = strLine.find(wstrname)) == std::wstring::npos)	
		{
			MessageBox(NULL,L"App type is EMail ,project is not exist!",L"Error",MB_OK);
			CLog::WriteLog(L"project parse failed! the command line ",strLine.c_str());
			return false;
		}
		nstart += wcslen(wstrname);
		nend =strLine.find(PARSECODE,nstart);
		if(nend == std::wstring::npos)
			g_BaseArgument.wstrCustomer = strLine.substr(nstart,strLine.length()-nstart);
		else 
			g_BaseArgument.wstrCustomer = strLine.substr(nstart ,nend-nstart);
		if(g_BaseArgument.wstrCustomer.empty())
		{
			MessageBox(NULL,L"project is empty!",L"Error",MB_OK);
			CLog::WriteLog(L"Error",L"project is empty!");
			return false;
		}

		// approver email address
		wstrname = L"-approver \"";
		if((nstart = strLine.find(wstrname)) == std::wstring::npos)	
		{
			MessageBox(NULL,L"App type is EMail ,approver is not exist!",L"Error",MB_OK);
			CLog::WriteLog(L"approver parse failed! the command line ",strLine.c_str());
			return false;
		}
		nstart += wcslen(wstrname);
		nend =strLine.find(PARSECODE,nstart);
		if(nend == std::wstring::npos)
			g_BaseArgument.wstrApprEMail = strLine.substr(nstart,strLine.length()-nstart);
		else 
			g_BaseArgument.wstrApprEMail = strLine.substr(nstart ,nend-nstart);
		if(g_BaseArgument.wstrApprEMail.empty())
		{
			MessageBox(NULL,L"approver is empty!",L"Error",MB_OK);
			CLog::WriteLog(L"Error",L"approver is empty!");
			return false;
		}

		std::wstring wstrApproverEmail,wstrApproverSID; 
		std::wstring::size_type start=0,end=0;
		while(true)
		{
			end = g_BaseArgument.wstrApprEMail.find(L';',start);
			if(end == std::wstring::npos)	
				wstrApproverEmail = g_BaseArgument.wstrApprEMail.substr(start,g_BaseArgument.wstrApprEMail.length()-start);
			else
				wstrApproverEmail = g_BaseArgument.wstrApprEMail.substr(start,end-start);
			strKeyWord=L"(mail=";
			strKeyWord += wstrApproverEmail;
			strKeyWord += L")";	
			if(!theWinAD.SearchUserInfo(wstrApproverEmail,wstrApproverSID,strKeyWord.c_str()))
			{
				std::wstring strError = L"\"-approver ";
				strError += wstrApproverEmail;
				strError += L"\" could not be determined.Please contact the policy administrator";
				MessageBoxW(NULL,L"",L"Error",MB_OK);
				CLog::WriteLog(L"Get Approver's SID failed! the Approver's EMail Address",wstrApproverEmail.c_str());
				return false;
			}
#ifdef _DEBUG
			std::wstring strMsg = L"Get Approver SID by Email.Approver email is:";
			strMsg += wstrApproverEmail;
			strMsg += L"\t SID is:";
			strMsg += wstrApproverSID;
			MessageBox(NULL,strMsg.c_str(),L"Debug Msg",MB_OK);
#endif
			CLog::WriteLog(L"Approver's Email ",wstrApproverEmail.c_str());
			CLog::WriteLog(L"Approver's SID ",wstrApproverSID.c_str());

			g_BaseArgument.approverVector.push_back(FilePair(wstrApproverEmail,wstrApproverSID));
			if(end == std::wstring::npos)	break;
			start = end+1;
		}

		// add at 10-20 for all of the type support asymetic
		// start from here
#if 0
		// pwd
		wstrname = L"-encpasswd \"";
		if((nstart = strLine.find(wstrname)) != std::wstring::npos)	
		{
			nstart += wcslen(wstrname);
			nend =strLine.find(PARSECODE,nstart);
			if(nend == std::wstring::npos)
				g_BaseArgument.wstrEncryptionPwd = strLine.substr(nstart,strLine.length()-nstart);
			else 
				g_BaseArgument.wstrEncryptionPwd = strLine.substr(nstart,nend-nstart);
		}
#endif

		// customer key or pwd
		wstrname = L"-pubkey \"";
		if((nstart = strLine.find(wstrname)) != std::wstring::npos)
		{
			nstart += wcslen(wstrname);
			nend =strLine.find(PARSECODE,nstart);
			if(nend == std::wstring::npos)
				g_BaseArgument.wstrCusetomerKey = strLine.substr(nstart,strLine.length()-nstart);
			else 
				g_BaseArgument.wstrCusetomerKey = strLine.substr(nstart,nend-nstart);
			g_BaseArgument.wstrEncryptionPwd.clear();

		}
		else if((nstart = strLine.find(L"-encpasswd \"")) != std::wstring::npos)	
		{
			wstrname = L"-encpasswd \"";
			nstart += wcslen(wstrname);
			nend =strLine.find(PARSECODE,nstart);
			if(nend == std::wstring::npos)
				g_BaseArgument.wstrEncryptionPwd = strLine.substr(nstart,strLine.length()-nstart);
			else 
				g_BaseArgument.wstrEncryptionPwd = strLine.substr(nstart,nend-nstart);
			g_BaseArgument.wstrCusetomerKey.clear();
		}
		else 
		{
			MessageBoxW(NULL,L"The data should be protected using encryption for approval. It needs public key or encryption password. Please contact your policy admin",L"Error",MB_OK);
			CLog::WriteLog(L"Get public key or encryption failed! the command line ",strLine.c_str());
			return false;
		}
		// end at here
	}
	return true;
}

// check the common available for all of the type
bool CObligationApp::CheckAvailable(void)
{
	// check whether Approval directory exists
	if(!PathFileExistsW(g_BaseArgument.wstrApprDir.c_str()))
	{
		MessageBox(NULL,L"Approved directory not exists. Please contact your policy administrator",L"Error",MB_OK);
		CLog::WriteLog(L"Error",L"Approved directory not exists. Please contact your policy administrator ");		
		return false;
	}

	// check Quarantine directory exists and the user can write to that directory

	if(!PathFileExistsW(g_BaseArgument.wstrQuarantineDir.c_str()) )
	{
		MessageBox(NULL,L"Quarantine directory no exists . Please contact your policy administrator",L"Error",MB_OK);
		CLog::WriteLog(L"Error",L"Quarantine directory no exists. Please contact your policy administrator ");
		return false;
	}
	else
	{
		g_BaseArgument.wstrQuarantineDir += L"\\";
		std::wstring strfile = FileProcess::ComposeFolderNameWithTimeStamp(g_BaseArgument.wstrCurUserName.c_str());
		g_BaseArgument.wstrQuarantineDir += strfile;

		if(!PathFileExistsW(g_BaseArgument.wstrQuarantineDir.c_str()))
		{
			// create acl 
			//////////////////////////////////////////////////////////////////////////
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

			if(::CreateDirectoryW(g_BaseArgument.wstrQuarantineDir.c_str(),sa.get_SecurityAttributes()) == 0)
			{
				wchar_t strMsg[512]={0};
				_snwprintf_s(strMsg,512, _TRUNCATE,L"Unable to access quarantine directory at \"%ws\" . Please contact your policy administrator",g_BaseArgument.wstrQuarantineDir.c_str());
				MessageBox(NULL,strMsg,L"Error",MB_OK);
				CLog::WriteLog(L"Error",strMsg);
				return false;
			}
#ifdef _DEBUG
			std::wstring strMsg = L"Create quarantine directory ";
			strMsg += g_BaseArgument.wstrQuarantineDir;
			strMsg += L" at here!";
			MessageBox(NULL,strMsg.c_str(),L"Debug Msg",MB_OK);
#endif
		}
	}
	return true;
}

bool CObligationApp::DoWork(BaseArgumentFlex &baseArgumentFlex)
{
	COBEmail theEmail;
	CFtpDlg theDlg(&theEmail);
	theDlg.SetBaseArgument(baseArgumentFlex);
	theDlg.DoModal();
	return true;
}
bool CObligationApp::DoWork(void)
{
	COBEmail theEmail;
	CFtpDlg theDlg(&theEmail);
	theDlg.DoModal();
	
	//if(_wcsicmp(g_BaseArgument.wstrApprType.c_str(),g_strFtpType) ==0 )
	//{
	//	// show ftp dlg for input argument
	//	// we need get approver from ad
	//	CFtpDlg theDlg(&theEmail);
	//	theDlg.DoModal();
	//}
	//else if(_wcsicmp(g_BaseArgument.wstrApprType.c_str(),g_strEmailType)== 0)
	//{
	//	// show EMail dlg for input argument
	//	CEMailDlg theDlg(&theEmail);
	//	theDlg.DoModal();
	//}
	//else if(_wcsicmp(g_BaseArgument.wstrApprType.c_str(),g_strReMedia) == 0 )
	//{
	//	// show Removable media dlg
	//	CRemovable theDlg(&theEmail);
	//	theDlg.DoModal();
	//}
	return true;
}
