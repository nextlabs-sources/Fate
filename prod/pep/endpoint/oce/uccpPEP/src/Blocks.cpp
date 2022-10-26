#include "stdafx.h"
#include <BaseTsd.h >
#include <time.h>
#include <sddl.h>
#include <algorithm>
//#include "Helper.h"


#pragma warning(push)
#pragma warning(disable : 4100)
#include "brain.h"
#pragma warning(pop)


#include "Blocks.h"
#include "EvalCache.h"

#include "celog.h"
extern CELog g_log;

#include "eframework\platform\policy_controller.hpp"
#define DISCLAIMER_ONETIME			TRUE


namespace {

bool IsPCRunning()
{
	return nextlabs::policy_controller::is_up();
}

//Some utility functions scoped 
time_t WinTime2JavaTime(SYSTEMTIME* pSysTime)
{
    time_t rtTime = 0;
    tm     rtTM;

    rtTM.tm_year = pSysTime->wYear - 1900;
    rtTM.tm_mon  = pSysTime->wMonth - 1;
    rtTM.tm_mday = pSysTime->wDay;
    rtTM.tm_hour = pSysTime->wHour;
    rtTM.tm_min  = pSysTime->wMinute;
    rtTM.tm_sec  = pSysTime->wSecond;
    rtTM.tm_wday = pSysTime->wDayOfWeek;
    rtTM.tm_isdst = -1;     // Let CRT Lib compute whether DST is in effect,
                            // assuming US rules for DST.
    rtTime = mktime(&rtTM); // get the second from Jan. 1, 1970

    if (rtTime == (time_t) -1)
    {
        if (pSysTime->wYear <= 1970)
        {
            // Underflow.  Return the lowest number possible.
            rtTime = (time_t) 0;
        }
        else
        {
            // Overflow.  Return the highest number possible.
            rtTime = (time_t) _I64_MAX;
        }
    }
    else
    {
        rtTime*= 1000;          // get millisecond
    }

    return rtTime;
}

void GetFileLastModifyTime(const WCHAR* wzFileName, WCHAR* wzDateTime, int BufSize)
{
	HANDLE hFile = (HANDLE) -1;/*INVALID_HANDLE_VALUE;*/
    memset(wzDateTime, 0, BufSize*sizeof(WCHAR));

    hFile = CreateFile(wzFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if(/*INVALID_HANDLE_VALUE*/(HANDLE)-1 != hFile)
    {
        FILETIME ftLastModify;  memset(&ftLastModify, 0, sizeof(FILETIME));
        if(GetFileTime(hFile, NULL, NULL, &ftLastModify))
        {
            FILETIME ftLocalLastModify;  memset(&ftLocalLastModify, 0, sizeof(FILETIME));
            SYSTEMTIME stModifyTime;    memset(&stModifyTime, 0, sizeof(SYSTEMTIME));
            FileTimeToLocalFileTime(&ftLastModify, &ftLocalLastModify);
            if(FileTimeToSystemTime(&ftLocalLastModify, &stModifyTime))
            {
                time_t tmModify;
                tmModify = WinTime2JavaTime(&stModifyTime);
                swprintf(wzDateTime, BufSize, L"%I64d", tmModify);
            }
        }
        else
        {
            g_log.Log(CELOG_DEBUG, _T("[GetFileLastModifyTime] Fail to get file time\n"));
        }
        CloseHandle(hFile);
    }
    else
    {
		g_log.Log(CELOG_DEBUG, _T("[GetFileLastModifyTime] Fail to open file '%s':  Last error = %d\n"), 
			wzFileName, GetLastError());
    }
}

typedef struct WarnUserParam 
{
	std::wstring strMsg;
	bool bCancel;
}WARNUSERPARAM, *LPWARNUSERPARAM;

void WarnUserFunc( void* pArguments )
{
	if(!pArguments)
	{
		return;
	}

	LPWARNUSERPARAM pParam = (LPWARNUSERPARAM)pArguments;


	int ret=MessageBox(NULL, pParam->strMsg.c_str(),
		L"Compliant Enterprise Communicator Enforcer", 
		MB_ICONWARNING| MB_OKCANCEL|MB_TASKMODAL|MB_SETFOREGROUND);


	if (ret==IDCANCEL) 
	{
		//cancel button is selected. Action is denied
		pParam->bCancel = true;
	}

	return;
}

bool ShowWarnMessageBox(const WCHAR *msg) 
{
	std::wstring trimedMsg=L"";
	size_t len=wcslen(msg);
	bool bUserCancel=false;
	for(size_t i=0; i<len; i++) {
		if(i%80 != 0 || i==0)
			trimedMsg.append(&msg[i],1);
		else {
			if(msg[i] == L' ' || msg[i] == L'\t' || msg[i] == L'\n')
				trimedMsg+=L"\n";
			else {
				while(i<len) {
					if(msg[i] == L' ' || msg[i] == L'\t' || msg[i] == L'\n') {
						trimedMsg+=L"\n";
						break;
					}
					trimedMsg.append(&msg[i++],1);						
				}
			}
		}
	}

	WARNUSERPARAM param;
	param.strMsg = trimedMsg;
	param.bCancel = false;

	/*****************************************************************
	It will prevent OC to call "SendMessage" if we pop up dialog here.
	It seems the UI changed something. we can't find the root cause.
	We just create a thread to pop up the dialog, it can resolve this
	problem.
	*****************************************************************/
	HWND hWnd = GetForegroundWindow();
	HANDLE hThread = (HANDLE)_beginthread(WarnUserFunc, 0, &param);

	WaitForSingleObject(hThread, INFINITE);

	bUserCancel = param.bCancel;

	SetForegroundWindow(hWnd);

	return bUserCancel;
}
}

/*==========================================================================*
 * Member functions of class PolicyEval                                     *
 *==========================================================================*/
//PolicyEval constructor
PolicyEval::PolicyEval():endPointSip(NULL), oceHandle(NULL),
	currentWinLogonUserSid(NULL), bInitFailed(false), outstandWnd(NULL)
{
	disclaimerMsgMutex=CreateMutex(NULL, false, NULL);	
	fileInfoMapMutex=CreateMutex(NULL, false, NULL);
	sessionWindowMapMutex=CreateMutex(NULL, false, NULL);
	cachedCopyEvalMutex=CreateMutex(NULL, false, NULL);
	isIMFirstMsgSetMutex=CreateMutex(NULL, false, NULL);
	groupSetMutex=CreateMutex(NULL, false, NULL);
	incomingGroupSetMutex=CreateMutex(NULL, false, NULL);
	GetHostIPAddr();
	GetWinLogonUserSid();
}  

//PolicyEval destructor
PolicyEval::~PolicyEval()
{
	SysFreeString(endPointSip);
	endPointSip=NULL;

	if(currentWinLogonUserSid)
		LocalFree(currentWinLogonUserSid);
	

	CloseHandle(disclaimerMsgMutex);
	CloseHandle(fileInfoMapMutex);
	CloseHandle(sessionWindowMapMutex);
	CloseHandle(cachedCopyEvalMutex);
	CloseHandle(isIMFirstMsgSetMutex);
	CloseHandle(groupSetMutex);
	CloseHandle(incomingGroupSetMutex);

	/*****************************************************************************************
	we can't call these functions here, since it's possible that the SDK was unloaded already.
	OCE created a global variable of PolicyEval.
	System will call this dis-constructor very late.
	As I tested, it will be called after DllMain(DLL_PROCESS_DETACH). 
	So it's possible the SDK DLLs were unloaded by system already.

	In fact, OC will crash when we exit OC with below codes.

	******************************************************************************************/
#if 0
	if(oceHandle)
	{
		m_sdkLoader.fns.CECONN_Close(oceHandle, 10000); 
	}


	if(m_sdkLoader.is_loaded())
	{
		//	unload sdk
		m_sdkLoader.unload();
	}
#endif
}  

void PolicyEval::SetEndPointSip(BSTR in)
{
	if(endPointSip)
		SysFreeString(endPointSip);
	endPointSip = SysAllocString(in);
}

//Add open file info and action time into mapping
void PolicyEval::AddFileInfoMapping(DWORD tid, LPCWSTR fileName, HANDLE handle)
{
	tid;

	WaitForSingleObject(fileInfoMapMutex, INFINITE);
	stdext::hash_map<wstring, FileInfo>::iterator it=fileInfoMap.begin();
	std::vector<wstring> discardRecords;
	std::vector<wstring>::iterator dit;
	double currentTime=GetTickCount();

	//We first discard the old record
	for(; it!=fileInfoMap.end(); it++) {
		FileInfo *fi=&(it->second);
		if(currentTime-fi->openTime > 60000) //1 minute old record, discard it
			discardRecords.push_back(it->first);
	}
	for(dit=discardRecords.begin(); dit != discardRecords.end(); dit++) {
		it=fileInfoMap.find(*dit);
		if(it != fileInfoMap.end())
			fileInfoMap.erase(it);
	}
		
	//Then do adding
	if(wcsstr(fileName, L":\\")|| wcsstr(fileName, L"://") || wcsstr(fileName, L"\\") == fileName) {
		//we only are interested in the one with full path
		FileInfo fi;
		fi.fileName=fileName;
		fi.fileHandle=handle;
		fi.openTime=GetTickCount();
		wchar_t* file_name_ptr = wcsrchr((wchar_t*)fileName, '\\'); // get just file name w/o full path
		if(file_name_ptr == NULL)
			file_name_ptr = (wchar_t*)fileName; // only filename? or http://www.xxx.com/xxx.txt?
		else
			file_name_ptr++; // move past '\\'
		fileInfoMap[file_name_ptr] = fi;
	}
	ReleaseMutex(fileInfoMapMutex);
}

//Add a disclaimer message for a receipt
void PolicyEval::AddDisclaimer(BSTR receipt, const WCHAR *msg)
{
	WaitForSingleObject(disclaimerMsgMutex, INFINITE);
	stdext::hash_map<wstring, wstring>::iterator it=disclaimerMsgs.find(receipt);
	if(it == disclaimerMsgs.end())
		disclaimerMsgs[receipt]=msg;
	else {
		if(wcscmp(it->second.c_str(), msg)) {
			it->second+=L"\t\n";
			it->second+=msg;
		}
	}
	ReleaseMutex(disclaimerMsgMutex);
}

//Fetch a disclaimer message for a receipt if the msg exists and remove it from 
//the map after fetch it
void PolicyEval::FetchDisclaimer(BSTR participant, BSTR *msg)
{
	*msg=NULL;
	
	//We don't the current endpointsip, no disclaimer needs to fetched
	if(endPointSip==NULL)
		return;

	//If the current participant is the same as the endPointSip, no disclaimer
	//needs to be fetched.
	if(participant == endPointSip) 
		return;

	WaitForSingleObject(disclaimerMsgMutex, INFINITE);
	stdext::hash_map <wstring, wstring>::iterator it=disclaimerMsgs.find(participant);
	if(it != disclaimerMsgs.end()) {
		*msg=SysAllocString(disclaimerMsgs[participant].c_str());

		if(DISCLAIMER_ONETIME)
		{
		disclaimerMsgs.erase(it);
	}
	}
	ReleaseMutex(disclaimerMsgMutex);	
}

bool PolicyEval::SetupConnection()
{
	//The connection has been failed to setup; everything is allowed
	if(bInitFailed==true)
		return false;

	//The connection has been setup
	if(oceHandle != NULL)
		return true;

	if ( !m_sdkLoader.is_loaded() )
	{
		//	the sdk is not loaded yet,
		//	load sdk.
		//	first get the installer dir from regedit first
		wchar_t install_root[2048] = {0};
		bool ret = true;//NLConfig::GetComponentInstallPath( L"Enterprise DLP\\Office Communicator Enforcer", install_root, 2048 );
		//if ( !ret || !(*install_root) )
		//{
		//	//	get installer dir from regedit failed
		//	return false;
		//}

		//	load sdk from installer_root\bin
		//wsprintf(install_root, L"%s\\bin", install_root);
		wsprintf(install_root, L"%s\\bin32", L"C:\\Program Files\\NextLabs\\Common");
		ret = m_sdkLoader.load(install_root);
		if (!ret)
		{
			//	load failed
			return false;
		}
	}
	


	CEString binaryPath=NULL;
	CEUser user;
	CEApplication app;

	user.userName=NULL;
	user.userID=NULL;
	if(processName.empty())
		app.appPath=NULL;
	else {
		binaryPath = m_sdkLoader.fns.CEM_AllocateString(processName.c_str());
		app.appPath=binaryPath;
	}
	app.appName=NULL;
	
	CEResult_t res = m_sdkLoader.fns.CECONN_Initialize(app, user, NULL, &oceHandle, 40000);
	if(res != CE_RESULT_SUCCESS) {
		g_log.Log(CELOG_DEBUG, _T("CECONN_Initialize failed: errorno=%d\n"), res);
		oceHandle=NULL;
	}

	if(binaryPath)
	{
		m_sdkLoader.fns.CEM_FreeString(binaryPath);
	}
	return (res==CE_RESULT_SUCCESS)?TRUE:FALSE;
}

//Get host ip address
bool PolicyEval::GetHostIPAddr()
{
  char hostName[1024];
  struct addrinfo hints, *res;
  int err;
  WORD wVersionRequested;
  WSADATA wsaData;

  wVersionRequested = MAKEWORD( 2, 2 );
 
  err = WSAStartup( wVersionRequested, &wsaData );
  if ( err != 0 ) {
     /* Tell the user that we could not find a usable */
     /* WinSock DLL.                                  */
	 g_log.Log(CELOG_DEBUG, _T("Failed to WSAStartup: error=%d\n"), WSAGetLastError());
     return false;
  }   

  if(gethostname(hostName, 1024) != 0) {
    g_log.Log(CELOG_DEBUG, _T("Failed to get host name: error=%d\n"), errno);
	hostIPAddr.s_addr=123;
	return false;
  }

  memset(&hints, 0, sizeof(hints));
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_family = AF_INET;

  if ((err = getaddrinfo(hostName, NULL, &hints, &res)) != 0) {
    g_log.Log(CELOG_DEBUG, _T("Failed to get host ip: error=%d\n"), err);
	hostIPAddr.s_addr=123;
    return false;
  }

  hostIPAddr = ((struct sockaddr_in *)(res->ai_addr))->sin_addr;
  hostIPAddr.s_addr=ntohl(hostIPAddr.s_addr);
  //printf("ip address : %s %lu\n", inet_ntoa(hostIPAddr), hostIPAddr.s_addr);

  freeaddrinfo(res);

  return true;  
}

bool PolicyEval::GetWinLogonUserSid()
{
	HANDLE       hToken   = NULL;
	PTOKEN_USER  ptiUser  = NULL;
	DWORD        cbti     = 0;
	DWORD        lastErr;

	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
		lastErr = GetLastError();
                return false;
	}

	if (!GetTokenInformation(hToken, TokenUser, NULL, 0, &cbti)) {
		lastErr = GetLastError();
                return false;
	}
	ptiUser = (PTOKEN_USER) HeapAlloc(GetProcessHeap(), 0, cbti);

	if (!GetTokenInformation(hToken, TokenUser, ptiUser, cbti, &cbti)) {
		lastErr = GetLastError();
                return false;
	}

	ConvertSidToStringSidW(ptiUser->User.Sid, &currentWinLogonUserSid);
	return true;
}

//Parse the resulted obligation
//Handle disclaimer obligation
void PolicyEval::HandleDisclaimer(std::map<wstring, wstring> &obligations,
								  std::set<wstring> &disclaimerSet,
								  std::set<wstring> &disclaimerPreKeys)
{
	wstring disclaimerKeyStr;
	WCHAR disclaimerKey[1024];
	std::set<wstring>::iterator it;
	int obNum, obAttrNum;

	for(it=disclaimerPreKeys.begin(); it != disclaimerPreKeys.end(); it++) {
		disclaimerKeyStr=*it;
		wcsncpy_s(disclaimerKey, disclaimerKeyStr.c_str(), disclaimerKeyStr.length());
		swscanf_s(&disclaimerKey[wcslen(L"CE_ATTR_OBLIGATION_VALUE:")], 
			L"%d:%d", &obNum, &obAttrNum);
		swprintf(disclaimerKey, 1024, L"CE_ATTR_OBLIGATION_VALUE:%d:%d", obNum, obAttrNum+1);
		if(obligations.find(disclaimerKey) != obligations.end()) {
			swprintf(disclaimerKey, 1024, L"%s", obligations[disclaimerKey].c_str());
			if(disclaimerSet.find(disclaimerKey) == disclaimerSet.end()) {
				disclaimerSet.insert(disclaimerKey);
				g_log.Log(CELOG_DEBUG, _T("Get disclaimer %s \n"), disclaimerKey);
			}
		}
	}
}


//Handle warn obligation
void PolicyEval::HandleWarnObligation(std::map<wstring, wstring> &obligations,
									  std::set<wstring> &warnPreKeys,
									  std::set<wstring> &warnObligationSet,
									  int numParticipant,
									  bool &bPopupWindow,
									  bool &bUserCancel)
{
	std::set<wstring>::iterator it=warnPreKeys.begin();
	wstring warnKeyStr;
	wstring limitKeyStr;
	WCHAR strBuf[1024];
	int obNum, obAttrNum;

	g_log.Log(CELOG_DEBUG, L"Try to handle \"WarnUser Obligation\", participant count: %d \n", numParticipant);

	
	for(; it != warnPreKeys.end(); it++) {
		warnKeyStr=*it;
		wcsncpy_s(strBuf, warnKeyStr.c_str(), warnKeyStr.length());
		swscanf_s(&strBuf[wcslen(L"CE_ATTR_OBLIGATION_VALUE:")], L"%d:%d", &obNum, &obAttrNum);
		if(obAttrNum%2==0) {
			//this might be a real warn message, e.g. 
			//Obligation attrName: CE_ATTR_OBLIGATION_VALUE:1:1
			//Obligation attrValue: Warn Message
			//Obligation attrName: CE_ATTR_OBLIGATION_VALUE:1:2
			//Obligation attrValue: warn message#2
			continue;
		}
		if(obligations.find(strBuf) != obligations.end()) { 
			swprintf(strBuf, 1024, L"CE_ATTR_OBLIGATION_VALUE:%d:%d", obNum, obAttrNum+1);
			if(obligations.find(strBuf) != obligations.end()) { // Warn User message is not null
				swprintf(strBuf, 1024, L"%s", obligations[strBuf].c_str());
			}
			else { // Warn User message is null
				swprintf(strBuf, 1024, L"");
			}
			if(warnObligationSet.find(strBuf) == warnObligationSet.end()) {
				//This warn message has not been shown
				warnObligationSet.insert(strBuf);
				warnKeyStr=strBuf;
				swprintf(strBuf, 1024, L"CE_ATTR_OBLIGATION_VALUE:%d:%d", obNum, obAttrNum+2);
				if(obligations.find(strBuf) != obligations.end() &&
					obligations[strBuf] == L"Participant Limit") {
					//Find participant limit Key
					swprintf(strBuf, 1024, L"CE_ATTR_OBLIGATION_VALUE:%d:%d", obNum, obAttrNum+3);
					if(obligations.find(strBuf) != obligations.end()) {
						int limitParticipant=_wtoi(obligations[strBuf].c_str());
						if(limitParticipant < numParticipant) {
							//Over participant limits, display warn message
							if(ShowWarnMessageBox(warnKeyStr.c_str())) 
								bUserCancel=true;
							bPopupWindow=true;
						}
					} else {
						//No participant limit number defined. Warn message needs to be displayed
						if(ShowWarnMessageBox(warnKeyStr.c_str())) 
							bUserCancel=true;
						bPopupWindow=true;
					}
				} else {
					//Can't find participant limit Key
					//Warn message needs to be displayed
					if(ShowWarnMessageBox(warnKeyStr.c_str())) 
						bUserCancel=true;
					bPopupWindow=true;
				}
			} //end of if(warnObligationSet.find(strBuf) == warnObligationSet.end())
		}

		//If user chose "Cancel" at a warn obligation, exit
		if(bUserCancel)
			break;
	}//end of for loop
}
//Parse the disclaimer or/and warm message from policy evaluation result
void PolicyEval::ParseObligation(std::set<wstring> &disclaimerSet,
							   std::set<wstring> &warnObligationSet,
							   CEEnforcement_t *pEnforcement,
							   int numParticipant,
							   bool &bUserCancel,
							   bool &bPopupWindow,
							   bool bIgnoreDisclaimer)
{
	std::map<wstring, wstring> obligations;
	std::set<wstring> disclaimerPreKeys;
	std::set<wstring> warnPreKeys;
	const TCHAR *attrName;
	const TCHAR *attrValue;

	if(pEnforcement==NULL || pEnforcement->obligation==NULL)
		return;

	for(int i=0; i<pEnforcement->obligation->count; i++) {
        attrName = m_sdkLoader.fns.CEM_GetString(pEnforcement->obligation->attrs[i].key);
        attrValue = m_sdkLoader.fns.CEM_GetString(pEnforcement->obligation->attrs[i].value);
		if(attrName && attrValue) {
			obligations[attrName]=attrValue;
			g_log.Log(CELOG_DEBUG, _T("%d Obligation attrName: %s\nObligation attrValue: %s\n"), i, attrName, attrValue);			
			if(_tcsnicmp (attrValue, _T("Disclaimer message"), 
				_tcslen(_T("Disclaimer message")))==0)
				disclaimerPreKeys.insert(attrName);
			else if(_tcsnicmp (attrValue, _T("Warn message"), 
				_tcslen(_T("Warn message")))==0)
				warnPreKeys.insert(attrName);
		}
	}

	if(!bIgnoreDisclaimer && disclaimerPreKeys.size()>0)  
		HandleDisclaimer(obligations, disclaimerSet, disclaimerPreKeys);

	if(warnPreKeys.size()>0) 
		HandleWarnObligation(obligations,
							warnPreKeys,
							warnObligationSet,
							numParticipant,
							bPopupWindow,
							bUserCancel);


}

//Evaluate if the current endPointSIP can communicate with participant.
//If yes, return true; otherwise it is false
bool PolicyEval::EvalAddParticipant(BSTR participant, UCCP::UCC_SESSION_TYPE SessionType, bool bObligation)
{
	if(!IsPCRunning())
	{
		return true;
	}

	//Connection to policy controller has not been setup.
	if(oceHandle == NULL) {
		if(bInitFailed == false) {
			//Connection has not been setup, try to set it up
			SetupConnection();
			//setup connection failed, return true to allow
			if(bInitFailed == true)
				return true;
		} else //connection setup failed, return true to allow
			return true;
	}

	//We don't know the current endpointsip, return true to allow
	if(endPointSip==NULL)
		return true;

	//If the current participant is the same as the endPointSip, allow.
	wchar_t selfsip[256 + 1] = {0};
	wsprintf(selfsip, L"sip:%s", endPointSip);
	if(NULL != wcsstr(participant, selfsip))
		return true;

	int evaluationtime = 2;
	if(UCCST_APPLICATION_SHARING == SessionType)
	{
		evaluationtime = 1; // if session type is UCCST_APPLICATION_SHARING, the only one time evaluation
	}

	CEAction_t action=CE_ACTION_IM_FILE;
	CEUser sender;
	CEResult_t res;
    CEEnforcement_t enforcement;
	bool bAllow=true;
	CEString sourceString = m_sdkLoader.fns.CEM_AllocateString(_T("C:\\No_attachment.ice"));
	CEString *recipients= new CEString[2];
	recipients[0] = m_sdkLoader.fns.CEM_AllocateString(endPointSip);
	// if the participant have suffix params, should remove them, and they start with char ';'
	// ---------------------------------------------------------------------------------------
	wchar_t* pos = wcsstr(participant, _T(";"));
	if(NULL != pos)
	{
		BSTR participantsip = SysAllocStringLen(participant, static_cast<UINT>(pos - participant));
		recipients[1] = m_sdkLoader.fns.CEM_AllocateString(participantsip + wcslen(L"sip:"));
		SysFreeString(participantsip);

		pos = NULL;
	}
	else
		recipients[1] = m_sdkLoader.fns.CEM_AllocateString(participant + wcslen(L"sip:"));
	// ---------------------------------------------------------------------------------------
	
	std::set<wstring> disclaimerSet;
	std::set<wstring> warnObligations;
	bool bDummy;

	//Get correct action	
	switch(SessionType)
	{
	case UCCST_INSTANT_MESSAGING:
		//g_log.Log(CELOG_DEBUG, _T("UCCST_INSTANT_MESSAGING\n"));
		break;
	case UCCST_AUDIO_VIDEO:
		//g_log.Log(CELOG_DEBUG, _T("UCCST_AUDIO_VIDEO\n"));
		action=CE_ACTION_AVD;
		break;
	case UCCST_CONFERENCE:
		//g_log.Log(CELOG_DEBUG, _T("UCCST_CONFERENCE\n"));
		break;
	case UCCST_APPLICATION:
		//g_log.Log(CELOG_DEBUG, _T("UCCST_APPLICATION\n"));
		action=CE_ACTION_AVD;
		break;
	case UCCST_APPLICATION_SHARING: // Added By Jacky.Dong 2011-11-23
		//g_log.Log(CELOG_DEBUG, _T("CE_ACTION_WM_SHARE\n"));
		action = CE_ACTION_WM_SHARE; // Sharing
		break;
	default:
		//Unknown session type, allow
		return true;
	}

	//We still need to setup dummy attributes since we are using CheckMsgAttachment
    CEString key = m_sdkLoader.fns.CEM_AllocateString(_T("modified_date"));
    CEString val = m_sdkLoader.fns.CEM_AllocateString(_T("1234567"));
    CEAttribute dummyAttr[1];
    CEAttributes dummyAttrs;  
    dummyAttr[0].key = key;
    dummyAttr[0].value = val;
    dummyAttrs.count=1;
    dummyAttrs.attrs=dummyAttr;

	//do reciprocating evaluation
	for(int i=0; i < evaluationtime; i++) {
		sender.userName = m_sdkLoader.fns.CEM_AllocateString(m_sdkLoader.fns.CEM_GetString(recipients[i]));
		sender.userID = m_sdkLoader.fns.CEM_AllocateString(m_sdkLoader.fns.CEM_GetString(recipients[i]));

		//Do policy evaluation
		res = m_sdkLoader.fns.CEEVALUATE_CheckMessageAttachment(oceHandle, 
										action, 
										sourceString, //attachment source
										&dummyAttrs, //source attributes
										1, //number of receipients
										(i==0)?(&recipients[1]):(&recipients[0]), // recipients
										(CEint32)((hostIPAddr.s_addr)),//ip address
										&sender, //sender 
										NULL, //sender attributes
										NULL, //application
										NULL, //application attributes
										bObligation?CETrue:CEFalse, //performObligation 
										CE_NOISE_LEVEL_USER_ACTION, //noise level
										&enforcement, //returned enforcement
										OCE_EVALUATION_TIMEOUT); //timeout in milliseconds

		m_sdkLoader.fns.CEM_FreeString(sender.userName);
		m_sdkLoader.fns.CEM_FreeString(sender.userID);


		//Check the result
		if(res == CE_RESULT_SUCCESS) { 
			if(bAllow)
				bAllow=(enforcement.result==CEDeny)?false:true;

			//Parse Obligation
			bool bUserCancel=false;
			if(bAllow) 
				if(bObligation) {
					ParseObligation(disclaimerSet, warnObligations, &enforcement, 2, bUserCancel, bDummy);
					if(SessionType == UCCST_INSTANT_MESSAGING) {
						std::set<wstring>::iterator it=disclaimerSet.begin();
						for(; it!=disclaimerSet.end(); it++)
							AddDisclaimer(participant, it->c_str());
						disclaimerSet.clear();
					}
				}

			//Free enforcement
			m_sdkLoader.fns.CEEVALUATE_FreeEnforcement(enforcement);

			//The user chose to cancel the action
			if(bUserCancel) {
				bAllow=false;
				break;
			}

			g_log.Log(CELOG_DEBUG, L"\nOCE do evaluation (EvalAddParticipant): \nAction: %d (0 Instant Message, 1 Audio, 2 Conference, 3 Application, 4 Sharing)\nSender: %s\nRecepient: %s\nresult: %s\n", 
				SessionType, m_sdkLoader.fns.CEM_GetString(recipients[i]), m_sdkLoader.fns.CEM_GetString ( (i==0)?(recipients[1]):(recipients[0])), bAllow?L"Allow":L"Deny" );

		} else 
			g_log.Log(CELOG_DEBUG, _T("CEEVALUATE_CheckMessageAttachment failed: errorno=%d\n"), res);
	}

	

	//Cleaning up
    m_sdkLoader.fns.CEM_FreeString(key);
    m_sdkLoader.fns.CEM_FreeString(val);  
	m_sdkLoader.fns.CEM_FreeString(sourceString);
    m_sdkLoader.fns.CEM_FreeString(recipients[0]);
    m_sdkLoader.fns.CEM_FreeString(recipients[1]);
	delete [] recipients;

	//return result
	return bAllow; 
}

//Evaluate if the current endPointSip can transfer a file to the participant
//If yes, return true; otherwise it is false
bool PolicyEval::EvalTransferFile(BSTR participant, wchar_t *fileName)
{
	if(!IsPCRunning())
	{
		return true;
	}

	//Connection to policy controller has not been setup.
	if(oceHandle == NULL) {
		if(bInitFailed == false) {
			//Connection has not been setup, try to set it up
			SetupConnection();
			//setup connection failed, return true to allow
			if(bInitFailed == true)
				return true;
		} else //connection setup failed, return true to allow
			return true;
	}

	//We don't know the current endpointsip, return true to allow
	if(endPointSip==NULL)
		return true;

	//If the current participant is the same as the endPointSip, allow.
	if(wcsstr(participant,endPointSip) || participant == endPointSip) 
		return true;

	//Try to get file's name with full path
	wstring fileFullName = fileName;
	WaitForSingleObject(fileInfoMapMutex, INFINITE);
	stdext::hash_map<wstring, FileInfo>::iterator it = fileInfoMap.find(fileName);
	if(it != fileInfoMap.end())
	{
		FileInfo* fi = &(it->second);
		if(NULL != wcsstr(fi->fileName.c_str(), fileName))
		{
			fileFullName = fi->fileName;
			g_log.Log(CELOG_DEBUG, _T("TransferFile[%s]:%s, Current File Count: %d\n"), fileName, fi->fileName.c_str(), fileInfoMap.size());
		}
		fileInfoMap.erase(it); //Erase the record
	}
	ReleaseMutex(fileInfoMapMutex);

	CEAction_t action=CE_ACTION_IM_FILE;
	CEUser sender;
	CEResult_t res;
    CEEnforcement_t enforcement;
	CEString sourceString = m_sdkLoader.fns.CEM_AllocateString(fileFullName.c_str());
	g_log.Log(CELOG_DEBUG, L"Jacky TransferFile:%s\n", fileFullName.c_str());
	CEString *recipients= new CEString[1];
	recipients[0] = m_sdkLoader.fns.CEM_AllocateString(participant+wcslen(L"sip:"));
	sender.userID = m_sdkLoader.fns.CEM_AllocateString(endPointSip);
	sender.userName = m_sdkLoader.fns.CEM_AllocateString(endPointSip);

	//Get source attribute
	//Get source's last modify date
    WCHAR lastModifyDate[MAX_PATH+1]; 
	memset(lastModifyDate, 0, sizeof(lastModifyDate));
	GetFileLastModifyTime(fileFullName.c_str(), lastModifyDate, MAX_PATH);
    if(0 == lastModifyDate[0]) 
		wcscpy_s(lastModifyDate, L"123456789");
 	CEString modAttrKey = m_sdkLoader.fns.CEM_AllocateString(_T("modified_date"));
	CEString modAttrVal = m_sdkLoader.fns.CEM_AllocateString(lastModifyDate);
    CEAttribute sourceAttr[1];
    CEAttributes sourceAttrs;  
    sourceAttr[0].key = modAttrKey;
    sourceAttr[0].value = modAttrVal;
    sourceAttrs.count=1;
    sourceAttrs.attrs=sourceAttr;

	//Do policy evaluation
	res=m_sdkLoader.fns.CEEVALUATE_CheckMessageAttachment(oceHandle, 
										action, 
										sourceString, //attachment source
										&sourceAttrs, //source attributes
										1, //number of receipients
										recipients, // recipients
										(CEint32)((hostIPAddr.s_addr)),//ip address
										&sender, //sender 
										NULL, //sender attributes
										NULL, //application
										NULL, //application attributes
										CETrue, //performObligation 
										CE_NOISE_LEVEL_USER_ACTION, //noise level
										&enforcement, //returned enforcement
										OCE_EVALUATION_TIMEOUT); //timeout in milliseconds

	//Cleaning up
	m_sdkLoader.fns.CEM_FreeString(modAttrKey);
	m_sdkLoader.fns.CEM_FreeString(modAttrVal);
	m_sdkLoader.fns.CEM_FreeString(sourceString);
	m_sdkLoader.fns.CEM_FreeString(sender.userID);
	m_sdkLoader.fns.CEM_FreeString(sender.userName);
    m_sdkLoader.fns.CEM_FreeString(recipients[0]);
    delete [] recipients;

	//Check the result
	if(res == CE_RESULT_SUCCESS) {
		bool bAllow=(enforcement.result==CEDeny)?FALSE:TRUE;

		if(bAllow) {
			//Parse obligation
			bool bUserCancel=false;
			std::set<wstring> disclaimerSet;
			std::set<wstring> warnObligations;
			bool bDummy;
			ParseObligation(disclaimerSet, warnObligations, &enforcement, 2, bUserCancel, bDummy);

			std::set<wstring>::iterator it_disclaimerSet=disclaimerSet.begin();
			for(; it_disclaimerSet!=disclaimerSet.end(); it_disclaimerSet++)
			{
				AddDisclaimer(participant, it_disclaimerSet->c_str());
			}
			disclaimerSet.clear();

			if(bUserCancel)
				bAllow=false; //User chose to stop the whole action
		}

		g_log.Log(CELOG_DEBUG, L"\nOCE do evaluation (EvalTransferFile): \nAction: %d (0 Instant Message, 1 Audio, 2 conference, 3 application)\nresult: %s\n", 
			action, bAllow?L"Allow":L"Deny" );

		//Free enforcement
		m_sdkLoader.fns.CEEVALUATE_FreeEnforcement(enforcement);
		return bAllow;
	} else 
		g_log.Log(CELOG_DEBUG, _T("CEEVALUATE_CheckMessageAttachment failed: errorno=%d\n"), res);

	//Allow if anything wrong during evaluation
	return true;
}

//Evaluate if it is allowed to copy message, which is from this endPointSip to 
//its participants, to clipboard. If yes, return true; otherwise it is false
bool PolicyEval::EvalCopyAction()
{
	if(!IsPCRunning())
	{
		return true;
	}

	//Connection to policy controller has not been setup.
	if(oceHandle == NULL) {
		if(bInitFailed == false) {
			//Connection has not been setup, try to set it up
			SetupConnection();
			//setup connection failed, return true to allow
			if(bInitFailed == true)
				return true;
		} else //connection setup failed, return true to allow
			return true;
	}

	//We don't know the current endpointsip, return true to allow
	if(endPointSip==NULL)
	{
		g_log.Log(CELOG_DEBUG, _T("EvalCopyAction return as don't know the current endpointsip\n"));
		return true;
	}

	CComPtr<IUccCollection> pParticipants;
	CComQIPtr<IUccSessionParticipant> pParticipant;
	long numParticipants;
	CComPtr<IUccSession> pSession = NULL;

	//	this is added for bug 15906
	//	there is a outgoing conference session which may contain more than 2 participants,
	//	in this case, session pointer returned by GetSessionofWindow may don't have all participants in the conference
	//	instead, we need to check outgoing groupset.
	//	note that PASTE is not required for incoming session by PRD, so we don't consider incoming session case.
	WaitForSingleObject(groupSetMutex, INFINITE);
	std::map<HWND, GroupSessionInfo>::iterator it=groupSet.find(GetForegroundWindow());
	if( it != groupSet.end() ) 
	{
		CComPtr<IUccConferenceSession> pUccConfSession = it->second.confSession;
		if(pUccConfSession)
		{
			HRESULT qResult= pUccConfSession->QueryInterface(IID_IUccSession, (void **)&pSession);
			if(qResult == S_OK) 
			{
				g_log.Log(CELOG_DEBUG, _T("EvalCopyAction find session from groupSet\n"));
			}
		}
	}
	ReleaseMutex(groupSetMutex);
	
	if (!pSession)
	{
		//	only if we are not in outgoing conference session, then we can use old code
		g_log.Log(CELOG_DEBUG, _T("EvalCopyAction can't find session from groupSet, then try GetSessionofWindow\n"));
		pSession=GetSessionofWindow(GetForegroundWindow());
	}
		
	//If we can't get the session, return true to allow
	if(pSession == NULL) 
	{
		g_log.Log(CELOG_DEBUG, _T("EvalCopyAction return as can't find session from GetSessionofWindow\n"));
		return true;
	}

	//Get session participants
	pSession->get_Participants(&pParticipants );
	pParticipants->get_Count(&numParticipants);

	g_log.Log(CELOG_DEBUG, _T("EvalCopyAction pSession: %p, numParticipants: %d\n"), pSession, numParticipants);


	//No participant, return true to allow
	if(numParticipants <= 0) 
	{
		g_log.Log(CELOG_DEBUG, _T("EvalCopyAction return as participants number is 0\n"));
		return true;
	}

	CEAction_t action=CE_ACTION_PASTE_FILE;
	CEUser sender;
	CEResult_t res;
    CEEnforcement_t enforcement;
	int realNumParticipants = 0;
	CEString sourceString=m_sdkLoader.fns.CEM_AllocateString(_T("C:\\No_attachment.ice"));
	CEString *recipients= new CEString[numParticipants];

	//Assign sender
	sender.userID=m_sdkLoader.fns.CEM_AllocateString(endPointSip);
	sender.userName=m_sdkLoader.fns.CEM_AllocateString(endPointSip);

	for(int i=1; i<=numParticipants; i++) {
		CComVariant vtItem;
		vtItem.vt = VT_DISPATCH;
		pParticipants->get_Item(i, &vtItem);
		pParticipant = vtItem.pdispVal;
		BSTR bstrUri = NULL;IUccUri* pUri = NULL;
		HRESULT hr = pParticipant->get_Uri(&pUri);pUri->get_AddressOfRecord(&bstrUri);
		if(SUCCEEDED(hr)&&bstrUri) {
			//skip if the current participant is the same as the endPointSip
			if(!(wcsstr(bstrUri,endPointSip) || bstrUri == endPointSip)) {
				recipients[realNumParticipants] = m_sdkLoader.fns.CEM_AllocateString(bstrUri+wcslen(L"sip:"));
				realNumParticipants++;	
			}
		}
		SysFreeString(bstrUri);
	}

	//We still need to setup dummy attributes since we are using CheckMsgAttachment
    CEString key = m_sdkLoader.fns.CEM_AllocateString(_T("modified_date"));
    CEString val = m_sdkLoader.fns.CEM_AllocateString(_T("1234567"));
    CEAttribute dummyAttr[1];
    CEAttributes dummyAttrs;  
    dummyAttr[0].key = key;
    dummyAttr[0].value = val;
    dummyAttrs.count=1;
    dummyAttrs.attrs=dummyAttr;

	//Do policy evaluation
	res = m_sdkLoader.fns.CEEVALUATE_CheckMessageAttachment(oceHandle, 
										action, 
										sourceString, //attachment source
										&dummyAttrs, //source attributes
										realNumParticipants, //number of recipients
										recipients, //recipients
										(CEint32)((hostIPAddr.s_addr)),//ip address
										&sender, //sender 
										NULL, //sender attributes
										NULL, //application
										NULL, //application attributes
										CETrue, //performObligation 
										CE_NOISE_LEVEL_USER_ACTION, //noise level
										&enforcement, //returned enforcement
										OCE_EVALUATION_TIMEOUT); //timeout in milliseconds

	//Cleaning up
	m_sdkLoader.fns.CEM_FreeString(key);
	m_sdkLoader.fns.CEM_FreeString(val);
	m_sdkLoader.fns.CEM_FreeString(sourceString);
	m_sdkLoader.fns.CEM_FreeString(sender.userID);
	m_sdkLoader.fns.CEM_FreeString(sender.userName);
	for(int i=0; i<realNumParticipants; i++)
		m_sdkLoader.fns.CEM_FreeString(recipients[i]);
    delete [] recipients;

	//Check the result
	if(res == CE_RESULT_SUCCESS) 
	{
		bool bAllow=(enforcement.result==CEDeny)?FALSE:TRUE;

		if(bAllow) 
		{
			//Parse obligation
			bool bUserCancel=false;
			std::set<wstring> dummySet;
			bool bDummy;
			ParseObligation(dummySet, dummySet, &enforcement, realNumParticipants, bUserCancel, bDummy, true);
			if(bUserCancel)
				bAllow=false; //User chose to stop the whole action
		}

		g_log.Log(CELOG_DEBUG, L"\nOCE do evaluation (EvalCopyAction): \nAction: %d (0 Instant Message, 1 Audio, 2 conference, 3 application)\nresult: %s\n", 
			action, bAllow?L"Allow":L"Deny" );

		//Free enforcement
		m_sdkLoader.fns.CEEVALUATE_FreeEnforcement(enforcement);
		return bAllow;
	} 
	else 
	{
		g_log.Log(CELOG_DEBUG, _T("CEEVALUATE_CheckMessageAttachment failed: errorno=%d\n"), res);
	}

	//Allow if anything wrong during evaluation
	return true;
}

//Evaluate if livemeeting is allowed 
//If yes, return true; otherwise it is false
bool PolicyEval::EvalLivemeeting(wchar_t *confURI)
{
	if(!IsPCRunning())
	{
		return true;
	}

	//Connection to policy controller has not been setup.
	if(oceHandle == NULL) {
		if(bInitFailed == false) {
			//Connection has not been setup, try to set it up
			SetupConnection();
			//setup connection failed, return true to allow
			if(bInitFailed == true)
				return true;
		} else //connection setup failed, return true to allow
			return true;
	}

	bool bAllow=true;
	int realNumParticipants=0;
	CEAction_t action=CE_ACTION_MEETING;
	CEUser sender;
	CEResult_t res;
	CEEnforcement_t enforcement;
	CEBoolean bObligation=CETrue;
	std::set<wstring> disclaimerSet;
	std::set<wstring> warnObligation;
	std::set<wstring> &disclaimerSetRef=disclaimerSet;
	std::set<wstring> &warnObligationRef=warnObligation;
	WCHAR confMeetingID[1024];
	confMeetingID[0]=L'\0';



	
	std::vector<CEString> recipients;

	g_log.Log(CELOG_DEBUG, _T("Livemeeting evaluation: endpoint=%s confId=%s\n"), endPointSip, confURI);
	if(!wcsstr(confURI, endPointSip)) {
		//The livemeeting conference is not initiated by this endpoint.
		bool bCallSDKEval=false;
		bAllow=DoGroupNonIMEvalOnActiveIncomingSession(UCCST_APPLICATION, confURI, bCallSDKEval);
		if(bCallSDKEval) {
			//This is incoming group session of livemeeting
			return bAllow;
		}

		//This is peer-to-peer incoming group session of livemeeting
		bObligation=CEFalse;

		//Get recipents
		const wchar_t *endPos=wcsstr(confURI, L";gruu;opaque=app:conf:focus:id:");
		WCHAR confId[1024];
		wcsncpy_s(confId, confURI, endPos-confURI);
		confId[endPos-confURI]=L'\0';


		recipients.push_back( m_sdkLoader.fns.CEM_AllocateString( endPointSip ) );
		recipients.push_back( m_sdkLoader.fns.CEM_AllocateString( confId+wcslen( L"sip:" ) ) );
		realNumParticipants=2;
		
		
		g_log.Log(CELOG_DEBUG, _T("Livemeeting evaluation: 0=%s\n 1=%s\n"), endPointSip, confId);
		//We store a peer-to-peer conference session to incomingGroupSet at DoGroupEvalOnIncomingSession. 
		//So when livemeeting is launched for this session here and add more participant later, 
		//we can have a record to do policy evaluation.
		//Get conf meeting ID
		endPos+=wcslen(L";gruu;opaque=app:conf:focus:id:");
		size_t len=wcslen(endPos);
		wcsncpy_s(confMeetingID, endPos, len);
		confMeetingID[len]=L'\0';
	} else {
		WaitForSingleObject(groupSetMutex, INFINITE);
		std::map<HWND, GroupSessionInfo>::iterator it=groupSet.begin();
		for(; it!=groupSet.end(); it++) {
			//Find the conference record. 
			if(wcscmp(it->second.confURI.c_str(), confURI)==0) 
				break;		
		}
		if(it != groupSet.end() && it->second.numParticipants > 0) {
			//The conference record found. 
			//Get recipents
			std::map<wstring, GroupParticipant>::iterator pit=it->second.groupParticipants.begin();
			for(; pit!=it->second.groupParticipants.end(); pit++) {
				recipients.push_back( m_sdkLoader.fns.CEM_AllocateString( pit->first.c_str() + wcslen( L"sip:" ) ) );
				realNumParticipants++;
			}
			BSTR bstrUri=NULL;IUccUri* pUri = NULL;
			HRESULT hr = it->second.localParticipant->get_Uri(&pUri);pUri->get_AddressOfRecord(&bstrUri);
			if(SUCCEEDED(hr)&&bstrUri) {
				recipients.push_back( m_sdkLoader.fns.CEM_AllocateString( bstrUri + wcslen( L"sip:") ) );
				realNumParticipants++;
			}
			SysFreeString(bstrUri);
			disclaimerSetRef=it->second.groupDisclaimer.disclaimers;
			warnObligationRef=it->second.warnObligations;
		} else {
			ReleaseMutex(groupSetMutex);
			return bAllow;
		}
		ReleaseMutex(groupSetMutex);
	}

	CEString sourceString = m_sdkLoader.fns.CEM_AllocateString(_T("C:\\No_attachment.ice"));

	CEString *realRecipients= new CEString[realNumParticipants-1];

	//We still need to setup dummy attributes since we are using CheckMsgAttachment
	CEString key=m_sdkLoader.fns.CEM_AllocateString(_T("modified_date"));
	CEString val=m_sdkLoader.fns.CEM_AllocateString(_T("1234567"));
	CEAttribute dummyAttr[1];
	CEAttributes dummyAttrs;  
	dummyAttr[0].key = key;
	dummyAttr[0].value = val;
	dummyAttrs.count=1;
	dummyAttrs.attrs=dummyAttr;

	for(int i=0; i<realNumParticipants; i++) {
		//Assign sender
		sender.userID=m_sdkLoader.fns.CEM_AllocateString(m_sdkLoader.fns.CEM_GetString(recipients[i]));
		sender.userName=m_sdkLoader.fns.CEM_AllocateString(m_sdkLoader.fns.CEM_GetString(recipients[i]));

		//Assign recipients
		for(int j=0, count=0; j<realNumParticipants; j++) {
			if(j==i)
				continue;
			realRecipients[count++]=recipients[j];
		}

		//Call EVAL
		res=m_sdkLoader.fns.CEEVALUATE_CheckMessageAttachment(oceHandle, 
										action, 
										sourceString, //attachment source
										&dummyAttrs, //source attributes
										realNumParticipants-1, //number of recipients
										realRecipients, //recipients
										(CEint32)((hostIPAddr.s_addr)),//ip address
										&sender, //sender 
										NULL, //sender attributes
										NULL, //application
										NULL, //application attributes
										CETrue, //performObligation 
										CE_NOISE_LEVEL_USER_ACTION, //noise level
										&enforcement, //returned enforcement
										OCE_EVALUATION_TIMEOUT); //timeout in milliseconds
		m_sdkLoader.fns.CEM_FreeString(sender.userID);
		m_sdkLoader.fns.CEM_FreeString(sender.userName);
		
		//Check the result
		if(res == CE_RESULT_SUCCESS) {
			if(bAllow)
				bAllow=(enforcement.result==CEDeny)?FALSE:TRUE;

			//Get disclaimer
			bool bUserCancel=false;
			bool bDummy=false;
			if(bAllow) {
				if(bObligation)
					ParseObligation(disclaimerSetRef, warnObligationRef,
									&enforcement,realNumParticipants, bUserCancel, bDummy);
				g_log.Log(CELOG_DEBUG, _T("Livemeeting Allow: sender=%s\n"), m_sdkLoader.fns.CEM_GetString(recipients[i]));
			} else 
				g_log.Log(CELOG_DEBUG, _T("Livemeeting Deny: sender=%s\n"), m_sdkLoader.fns.CEM_GetString(recipients[i]));

			g_log.Log(CELOG_DEBUG, L"\nOCE do evaluation (EvalLivemeeting): \nAction: %d (0 Instant Message, 1 Audio, 2 conference, 3 application)\nresult: %s\n", 
				action, bAllow?L"Allow":L"Deny" );

			//Free enforcement
			m_sdkLoader.fns.CEEVALUATE_FreeEnforcement(enforcement);

			//User chose to cancel the whole action
			if(bUserCancel) {
				bAllow=false;
				break;
			}
		} else 
			g_log.Log(CELOG_DEBUG, _T("CEEVALUATE_CheckMessageAttachment failed: errorno=%d\n"), res);
	}
	//Cleaning up
	m_sdkLoader.fns.CEM_FreeString(key);
	m_sdkLoader.fns.CEM_FreeString(val);
	m_sdkLoader.fns.CEM_FreeString(sourceString);
	for(int i=0; i<realNumParticipants; i++)
		m_sdkLoader.fns.CEM_FreeString(recipients[i]);
	recipients.clear();
	delete [] realRecipients;

	if(bAllow) {
		//We store a peer-to-peer conference session to incomingGroupSet at DoGroupEvalOnIncomingSession. 
		//So when livemeeting is launched for this session here and add more participant later, 
		//we can have a record to do policy evaluation.
		//Get conf meeting ID
		if(wcslen(confMeetingID) > 0) {
			FindIncomingSessionAndSetType(confMeetingID, UCCST_APPLICATION);
		}		
	}

	//return policy evaluation result
	return bAllow;
}

//Add a new thread and session pair to sessionThreadMap
void PolicyEval::AddSessionWindowPair(HWND w, CComPtr<IUccSession> s)
{
	WaitForSingleObject(sessionWindowMapMutex, INFINITE);	
	stdext::hash_map<HWND, CComPtr<IUccSession>>::iterator it=sessionWindowMap.find(w);
	if(it == sessionWindowMap.end()) {
		sessionWindowMap[w]=s;
		g_log.Log(CELOG_DEBUG, _T("Add session %p for window %d\n"), s, w);
	} else if(it->second != s) {
		sessionWindowMap[w]=s;
		g_log.Log(CELOG_DEBUG, _T("Replace session %p for window %d\n"), s, w);
	}
	ReleaseMutex(sessionWindowMapMutex);
}

//Remove a thread and session pair from sessionThreadMap
void PolicyEval::RemoveSessionWindowPair(HWND w, CComPtr<IUccSession> s)
{
	WaitForSingleObject(sessionWindowMapMutex, INFINITE);
	stdext::hash_map<HWND, CComPtr<IUccSession>>::iterator it=sessionWindowMap.find(w);
	if(it != sessionWindowMap.end()) 
		if(it->second == s) { //We only erase it when the session matched
			sessionWindowMap.erase(it);	
			//g_log.Log(CELOG_DEBUG, _T("Remove session %p for thread %d\n"), s, threadID);
		}
	ReleaseMutex(sessionWindowMapMutex);
}

//Fetch the session of a thread
CComPtr<IUccSession> PolicyEval::GetSessionofWindow(HWND w)
{
	CComPtr<IUccSession> s=NULL;

	WaitForSingleObject(sessionWindowMapMutex, INFINITE);
	stdext::hash_map<HWND, CComPtr<IUccSession>>::iterator it=sessionWindowMap.find(w);
	if(it != sessionWindowMap.end()) 
		s=it->second; 
	ReleaseMutex(sessionWindowMapMutex);
	return s;
}

//Add a thread and its copy evaluation result pair to cachedCopyEvalResults
void PolicyEval::CacheCopyEvalResult(HWND hWnd, bool bAllow)
{
	WaitForSingleObject(cachedCopyEvalMutex, INFINITE);	
	EvalResult er;
	er.evalTime=GetTickCount();
	er.bAllow=bAllow;
	cachedCopyEvalResults[hWnd]=er;
	ReleaseMutex(cachedCopyEvalMutex);
}

//Remove a thread and its copy evaluation result pair 
void PolicyEval::RemoveCachedCopyEvalResults(HWND hWnd)
{
	WaitForSingleObject(cachedCopyEvalMutex, INFINITE);
	stdext::hash_map<HWND, EvalResult>::iterator it=cachedCopyEvalResults.find(hWnd);
	if(it != cachedCopyEvalResults.end()) 
		cachedCopyEvalResults.erase(it);
	ReleaseMutex(cachedCopyEvalMutex);
}

//Fetch the cached copy eval result of a thread
//When this function reture false, it means that the result is unknown
//If the last eval time is more than 10 sec ago, discard the result and return false;
bool PolicyEval::GetCachedCopyEvalResult(HWND hWnd, bool &bAllow)
{
	bool bUnknown=true;

	WaitForSingleObject(cachedCopyEvalMutex, INFINITE);
	stdext::hash_map<HWND, EvalResult>::iterator it=cachedCopyEvalResults.find(hWnd);
	if(it != cachedCopyEvalResults.end()) {
		double currentTime=GetTickCount();
		if((currentTime - it->second.evalTime) > 10000) {
			cachedCopyEvalResults.erase(it);
		} else {
			bAllow=it->second.bAllow;
			bUnknown=false;
		}
	}
	ReleaseMutex(cachedCopyEvalMutex);
	return bUnknown;
}
//Evaluate if it is allowed to send message among the recipients
bool PolicyEval::EvalSendMsgAction(CComPtr<IUccSession> pSession, wstring &disclaimer)
{
	if(!IsPCRunning())
	{
		return true;
	}

	//Connection to policy controller has not been setup.
	if(oceHandle == NULL) {
		if(bInitFailed == false) {
			//Connection has not been setup, try to set it up
			SetupConnection();
			//setup connection failed, return true to allow
			if(bInitFailed == true)
				return true;
		} else //connection setup failed, return true to allow
			return true;
	}

	CComPtr<IUccCollection> pParticipants; 
	CComQIPtr<IUccSessionParticipant> pParticipant;
	long numParticipants;
	bool bAllow=true;

	//Get session participants
	pSession->get_Participants(&pParticipants );
	pParticipants->get_Count(&numParticipants);

	//No participant, return true to allow
	if(numParticipants <= 0) {
		return true;
	}

	CEAction_t action=CE_ACTION_IM_FILE;
	CEUser sender;
	CEResult_t res;
    CEEnforcement_t enforcement;
	int realNumParticipants = 0;
	std::set<wstring> disclaimerSet;

	CEString sourceString=m_sdkLoader.fns.CEM_AllocateString(_T("C:\\No_attachment.ice"));
	CEString *recipients= new CEString[numParticipants];

	//Get recipients
	for(int i=1; i<=numParticipants; i++) {
		CComVariant vtItem;
		vtItem.vt = VT_DISPATCH;
		pParticipants->get_Item(i, &vtItem);
		pParticipant = vtItem.pdispVal;
		BSTR bstrUri = NULL;IUccUri* pUri = NULL;
		HRESULT hr = pParticipant->get_Uri(&pUri);pUri->get_AddressOfRecord(&bstrUri);
		if(SUCCEEDED(hr)&&bstrUri) {
			recipients[realNumParticipants]=m_sdkLoader.fns.CEM_AllocateString(bstrUri+wcslen(L"sip:"));
			realNumParticipants++;	
		}
		SysFreeString(bstrUri);
	}

	//We still need to setup dummy attributes since we are using CheckMsgAttachment
    CEString key=m_sdkLoader.fns.CEM_AllocateString(_T("modified_date"));
    CEString val=m_sdkLoader.fns.CEM_AllocateString(_T("1234567"));
    CEAttribute dummyAttr[1];
    CEAttributes dummyAttrs;  
    dummyAttr[0].key = key;
    dummyAttr[0].value = val;
    dummyAttrs.count=1;
    dummyAttrs.attrs=dummyAttr;

	for(int i=0; i<realNumParticipants; i++) {
		//Assign sender
		sender.userID=m_sdkLoader.fns.CEM_AllocateString(m_sdkLoader.fns.CEM_GetString(recipients[i]));
		sender.userName=m_sdkLoader.fns.CEM_AllocateString(m_sdkLoader.fns.CEM_GetString(recipients[i]));

		//Do policy evaluation
		res=m_sdkLoader.fns.CEEVALUATE_CheckMessageAttachment(oceHandle, 
										action, 
										sourceString, //attachment source
										&dummyAttrs, //source attributes
										realNumParticipants, //number of recipients
										recipients, //recipients
										(CEint32)((hostIPAddr.s_addr)),//ip address
										&sender, //sender 
										NULL, //sender attributes
										NULL, //application
										NULL, //application attributes
										CETrue, //performObligation 
										CE_NOISE_LEVEL_USER_ACTION, //noise level
										&enforcement, //returned enforcement
										OCE_EVALUATION_TIMEOUT); //timeout in milliseconds

		m_sdkLoader.fns.CEM_FreeString(sender.userID);
		m_sdkLoader.fns.CEM_FreeString(sender.userName);

		//Check the result
		if(res == CE_RESULT_SUCCESS) {
			if(bAllow)
				bAllow=(enforcement.result==CEDeny)?FALSE:TRUE;

			//Parse Obligation
			bool bUserCancel=false;
			bool bDummy=false;
			std::set<wstring> warnObligations;
			if(bAllow)
				ParseObligation(disclaimerSet, warnObligations, &enforcement, 
								realNumParticipants, bUserCancel, bDummy);

			g_log.Log(CELOG_DEBUG, L"\nOCE do evaluation (EvalSendMsgAction): \nAction: %d (0 Instant Message, 1 Audio, 2 conference, 3 application)\nresult: %s\n", 
				action, bAllow?L"Allow":L"Deny" );

			//Free enforcement
			m_sdkLoader.fns.CEEVALUATE_FreeEnforcement(enforcement);
			
			//User chose to cancel the action.  
			if(bUserCancel) {
				bAllow=false;
				break;
			}
		} else 
			g_log.Log(CELOG_DEBUG, _T("CEEVALUATE_CheckMessageAttachment failed: errorno=%d\n"), res);
	}

	//Cleaning up
	m_sdkLoader.fns.CEM_FreeString(key);
	m_sdkLoader.fns.CEM_FreeString(val);
	m_sdkLoader.fns.CEM_FreeString(sourceString);
	for(int i=0; i<realNumParticipants; i++)
		m_sdkLoader.fns.CEM_FreeString(recipients[i]);
    delete [] recipients;
	
	//Compose disclaimer
	std::set<wstring>::iterator it=disclaimerSet.begin();
	for(; it != disclaimerSet.end(); it++) 
		disclaimer+=*it+L"\n";

	return bAllow;
}

//Add to first IM session set
void PolicyEval::AddToIMFirstMsg(CComPtr<IUccInstantMessagingSession> pIMSession)
{
	WaitForSingleObject(isIMFirstMsgSetMutex, INFINITE);
	isIMFirstMsgSet.insert(pIMSession);
	ReleaseMutex(isIMFirstMsgSetMutex);

}

//Is IM First message
bool PolicyEval::IsIMSessionFirstMsg(CComPtr<IUccInstantMessagingSession> pIMSession)
{
	bool bRes=false;
	WaitForSingleObject(isIMFirstMsgSetMutex, INFINITE);
	std::set<CComPtr<IUccInstantMessagingSession>>::iterator it=isIMFirstMsgSet.find(pIMSession);
	if(it != isIMFirstMsgSet.end()) {
		bRes=true;

		g_log.Log(CELOG_DEBUG, L"IsIMSessionFirstMsg yes\n");

		if(DISCLAIMER_ONETIME)
		{
		isIMFirstMsgSet.erase(it);
	}
	}
	ReleaseMutex(isIMFirstMsgSetMutex);
	return bRes;
}

/*Group Chat Handling*/
//The procedure of invoking a group IM chat is
//1. Create A new session (type=UCCST_CONFERENCE) including local participant
//2. Call IUccConferenceSession::Enter. Here, we get the window handle (HWND),
//   the window title to get the number of participant, and create a new 
//   GroupdSessionInfo entry and add it to the groupSet.   
void PolicyEval::InitGroupSession(HWND winHandle, IUccConferenceSession *session,
								  int numParticipant, BSTR bstrConfURI)
{
	g_log.Log(CELOG_DEBUG, L"InitGroupSession, count: %d, window: %d\n", numParticipant, winHandle);

//	if(numParticipant<3) //This is not a group chat
//		return;
	WaitForSingleObject(groupSetMutex, INFINITE);
	std::map<HWND, GroupSessionInfo>::iterator it=groupSet.find(winHandle);
	if( it == groupSet.end()) {
		GroupSessionInfo gInfo;
		gInfo.bFristEval=true;
		gInfo.confSession=session;
		gInfo.numParticipants=numParticipant;
		gInfo.sessionType=UCCST_CONFERENCE;
		gInfo.winHandle=winHandle;
		gInfo.groupDisclaimer.sendMsgFunc=NULL;
		gInfo.groupDisclaimer.imSession=NULL;
		gInfo.groupDisclaimer.bSendMsg=false;
		gInfo.confURI=bstrConfURI;
		CComPtr<IUccSession> pIuccSession;
		HRESULT qResult= session->QueryInterface(IID_IUccSession, (void **)&pIuccSession);
		if(qResult == S_OK) {
			pIuccSession->get_LocalParticipant(&(gInfo.localParticipant));
		}

		//Add this session to groupSet
		groupSet[winHandle]=gInfo;
		outstandWnd=winHandle;

	}
	ReleaseMutex(groupSetMutex);
}

//3. Create a new session (type=UCCST_INSTANT_MESSAGE), add a participant as 
//"sip:<local-participant>;gruu;opaque=app:conf:chat:idxxx" and send message
//including "This is a multi-part message". Here, we store this function information
//to the GroupDisclaimer structure of this GroupSessionInfo. We will use this information
//to send out the disclaimer later.
void PolicyEval::ModifySessionType(HWND winHandle, UCC_SESSION_TYPE t)
{
	WaitForSingleObject(groupSetMutex, INFINITE);
	std::map<HWND, GroupSessionInfo>::iterator it=groupSet.find(winHandle);
	if( it != groupSet.end()) {
		it->second.sessionType=t;
		g_log.Log(CELOG_DEBUG, "Jacky Log ModifySessionType: Window %d, SessionType %d\n", winHandle, t);
	}
	ReleaseMutex(groupSetMutex);	
}

// Added By Jacky.Dong 2011-11-21
// ------------------------------
//4. Get SessionType By HWND
bool PolicyEval::GetSessionTypeByHWND(HWND winHandle, UCC_SESSION_TYPE* sessionType)
{
	bool res = false;

	WaitForSingleObject(groupSetMutex, INFINITE);
	std::map<HWND, GroupSessionInfo>::iterator it = groupSet.find(winHandle);
	if( it != groupSet.end()) {
		*sessionType = it->second.sessionType;
		g_log.Log(CELOG_DEBUG, "Jacky Log GetSessionTypeByHWND: Window %d, SessionType %d\n", winHandle, *sessionType);

		res = true;
	}
	ReleaseMutex(groupSetMutex);

	return res;
}
// ------------------------------

void PolicyEval::StoreGroupSendMsgFunc(HWND winHandle, FUNC_IM_SESSION_SENDMESSAGE f, 
								CComPtr<IUccInstantMessagingSession> imSession, 
								struct IUccOperationContext * pOperationContext)
{	
	g_log.Log(CELOG_DEBUG, L"StoreGroupSendMsgFunc, window: %d, imSession: %p\n", winHandle, imSession);

	WaitForSingleObject(groupSetMutex, INFINITE);
	std::map<HWND, GroupSessionInfo>::iterator it=groupSet.find(winHandle);
	if( it != groupSet.end()) {
		if(it->second.groupDisclaimer.imSession==NULL &&
			it->second.groupDisclaimer.sendMsgFunc==NULL) {
			//New group IM chat session
			it->second.groupDisclaimer.imSession=imSession;
			it->second.groupDisclaimer.pOperationContext=pOperationContext;
			it->second.groupDisclaimer.sendMsgFunc=f;
			it->second.sessionType=UCCST_INSTANT_MESSAGING;
		}
		/*if(it->second.groupDisclaimer.disclaimers.size()>0 &&
			it->second.groupDisclaimer.imSession != NULL &&
			it->second.groupDisclaimer.sendMsgFunc != NULL &&
			it->second.groupDisclaimer.bSendMsg) {
			//We have disclaimer to be sent
			wstring disclaimer=L"";
			it->second.groupDisclaimer.bSendMsg=false;
			for(std::set<wstring>::iterator dit=it->second.groupDisclaimer.disclaimers.begin();
				dit != it->second.groupDisclaimer.disclaimers.end(); dit++) {
				disclaimer+=(*dit)+L"\n";
			}
			BSTR msgType = SysAllocString(L"text/plain");
			BSTR msg=SysAllocString(disclaimer.c_str());
			it->second.groupDisclaimer.sendMsgFunc(
												it->second.groupDisclaimer.imSession,
												msgType,
												msg,
												it->second.groupDisclaimer.pOperationContext
												);
		}	*/
	}
	ReleaseMutex(groupSetMutex);
}

void PolicyEval::SendGroupDisclaimer(HWND winHandle, CComPtr<IUccInstantMessagingSession> s, 
					BSTR bstrContentType, 
					struct IUccOperationContext * pOperationContext)
{	
	bstrContentType;
	g_log.Log(CELOG_DEBUG, L"Try to send disclaimer for group\n");

	WaitForSingleObject(groupSetMutex, INFINITE);
	std::map<HWND, GroupSessionInfo>::iterator it=groupSet.find(winHandle);
	if( it != groupSet.end()) {
		g_log.Log(CELOG_DEBUG, L"Disclaimer size: %d, window: %d, imSession: %p, sendMsgFunc: %p, bSendMsg: %d\n", it->second.groupDisclaimer.disclaimers.size(), winHandle, it->second.groupDisclaimer.imSession, it->second.groupDisclaimer.sendMsgFunc, it->second.groupDisclaimer.bSendMsg);

		if(s != it->second.groupDisclaimer.imSession) 
			g_log.Log(CELOG_DEBUG, _T("IMPOSSIBLE: different IM session\n"));
		if(pOperationContext != it->second.groupDisclaimer.pOperationContext) 
			g_log.Log(CELOG_DEBUG, _T("Oops: different IM operation context\n"));
		if(it->second.groupDisclaimer.disclaimers.size()>0 &&
			it->second.groupDisclaimer.imSession != NULL &&
			it->second.groupDisclaimer.sendMsgFunc != NULL &&
			it->second.groupDisclaimer.bSendMsg) {
			//We have disclaimer to be sent
			g_log.Log(CELOG_DEBUG, _T("send disclaimer to group, SendGroupDisclaimer\n"));
			wstring disclaimer=L"";

			if(DISCLAIMER_ONETIME)
			{
			it->second.groupDisclaimer.bSendMsg=false;
			}

			for(std::set<wstring>::iterator dit=it->second.groupDisclaimer.disclaimers.begin();
				dit != it->second.groupDisclaimer.disclaimers.end(); dit++) {
				disclaimer+=(*dit)+L"\n";
			}
			BSTR msgType = SysAllocString(L"text/plain");
			BSTR msg=SysAllocString(disclaimer.c_str());
			it->second.groupDisclaimer.sendMsgFunc(
												s, //it->second.groupDisclaimer.imSession,
												msgType,
												msg,
												pOperationContext 
												//crash happen: it->second.groupDisclaimer.pOperationContext
												);
			SysFreeString(msgType);
			SysFreeString(msg);
		}	
	}
	ReleaseMutex(groupSetMutex);
}
//If a session's participant is calling CopyParticipantEndpoint, we check if this
//session is in group chat set. If yes, this participant is the existing allowed member 
//so we add this participant to this group 
void PolicyEval::AddExistingMemberToGroupChatSession(HWND &winHandle, CComPtr<IUccSessionParticipant> pParticipant)
{
	WaitForSingleObject(groupSetMutex, INFINITE);
	std::map<HWND, GroupSessionInfo>::iterator it=groupSet.find(winHandle);

	//It is not in the group set
	if( it == groupSet.end()) {
		ReleaseMutex(groupSetMutex);
		return;
	}

	VARIANT_BOOL bLocal=false;
	pParticipant->get_IsLocal(&bLocal);
	if(!bLocal) {
		BSTR bstrUri = NULL;IUccUri* pUri = NULL;
		pParticipant->get_Uri(&pUri);pUri->get_AddressOfRecord(&bstrUri);
		if(it->second.groupParticipants.find(bstrUri) == it->second.groupParticipants.end()) {
			GroupParticipant gP;
			gP.addParticipantFunc=NULL; //we don't need it for added Participant
			gP.id=bstrUri;
			gP.pOperationContext=NULL; //we don't need it for added Participant
			gP.pParticipant=pParticipant;
			gP.session=NULL; //we don't need it for added Participant
			gP.bAdded=true;
			(it->second.groupParticipants)[bstrUri]=gP;
		}
		SysFreeString(bstrUri);
	}
	ReleaseMutex(groupSetMutex);
}

//4. Create a new session (type=UCCST_APPLICATION) for each following participant and
//    call addParticipant to send out invitation. Here, we hold the call addparticipant 
//    and store the information to GroupParticipant of this GroupSessionInfo. At adding
//    the No. "numParticipant", we do group participant evaluation and send out the 
//    disclaimer if it exists.  
//    If this is the first time to do policy evaluation for this group and the result is
//    deny (because two (or more) of them can't talk to each other), we cancell the whole
//    conferce by calling IUccConferenceSession::Leave. 
//    If this is not the first time to do policy evaluation, we won't add the new
//    participant to the group. 
bool PolicyEval::IsInGroupChatSession(HWND &winHandle)
{
	bool bResult=false;
	WaitForSingleObject(groupSetMutex, INFINITE);
	if(outstandWnd) {
		winHandle=outstandWnd;
		bResult=true;
	} else {
		std::map<HWND, GroupSessionInfo>::iterator it=groupSet.find(winHandle);
		if( it != groupSet.end()) 
			bResult=true;
	}
	ReleaseMutex(groupSetMutex);
	return bResult;
}

void PolicyEval::AddParticipantToGroupChat(HWND winHandle, WCHAR *id, CComPtr<IUccSession> session, 
									struct IUccSessionParticipant * pParticipant,
									struct IUccOperationContext * pOperationContext,
									FUNC_SESSION_ADDPARTICIPANT addParticipantFunc,
									bool &bAddNow)
{	
	WaitForSingleObject(groupSetMutex, INFINITE);
	std::map<HWND, GroupSessionInfo>::iterator it=groupSet.find(winHandle);
	if( it != groupSet.end()) {
		// Added By Jacky.Dong 2011-11-21
		// ------------------------------
		wchar_t selfsip[256 + 1] = {0};
		wsprintf(selfsip, L"sip:%s", endPointSip);
		if(NULL != wcsstr(id, selfsip)) // if the participant is login-self, needn't add.
		{
			bAddNow = true;
		}
		// ------------------------------
		else
		{
			if(it->second.groupParticipants.find(id) == it->second.groupParticipants.end()) {
				GroupParticipant gP;
				gP.addParticipantFunc=addParticipantFunc;
				gP.id=id;
				gP.pOperationContext=pOperationContext;
				//gP.pParticipant=new IUccSessionParticipant(*pParticipant);
				gP.pParticipant=pParticipant;
				gP.session=session;
				gP.bAdded=false;
				it->second.groupParticipants[id]=gP;
				// Modified By Jacky.Dong 2011-11-21
				// ---------------------------------
				if(it->second.numParticipants <= (int)it->second.groupParticipants.size())
				{
					// adjust the value of numParticipants(which = groupParticipants.size() + 1), except the login-self.
					it->second.numParticipants = (int)(it->second.groupParticipants.size() + 1);
					bAddNow = false;
				}
				else
				{
					// if the group is not full, go on filling participant.
					bAddNow = true;
				}
				// ---------------------------------
				g_log.Log(CELOG_DEBUG, L"AddParticipantToGroupChat %s to groupchat, window handle: %d, session: %p\n", id, winHandle, session);
			} else 
				//This group session might be started from an existing peer-to-peer session. 
				//This participant might be in the previous session already (See 
				//InitGroupSession function). Call the real participant so OC can add the next one. 
				bAddNow=true;
		}
	}
	ReleaseMutex(groupSetMutex);
}
void PolicyEval::UpdateGroupParticipantNum(HWND winHandle, int num)
{	
	WaitForSingleObject(groupSetMutex, INFINITE);
	std::map<HWND, GroupSessionInfo>::iterator it=groupSet.find(winHandle);
	if( it != groupSet.end()) {
		if(it->second.numParticipants != num)
			it->second.numParticipants=num;
	}
	ReleaseMutex(groupSetMutex);
}
bool PolicyEval::IsTimeToDoEvaluation(HWND winHandle)
{	
	bool bResult=false;
	WaitForSingleObject(groupSetMutex, INFINITE);
	std::map<HWND, GroupSessionInfo>::iterator it=groupSet.find(winHandle);
	if( it != groupSet.end()) {
		g_log.Log(CELOG_DEBUG, _T("IsTimeToDoEvaluation: %d %d?\n"), it->second.numParticipants,
			it->second.groupParticipants.size());
		if( ( (DWORD)(it->second.numParticipants-1) ) == it->second.groupParticipants.size() ) {
			std::map<wstring, GroupParticipant>::iterator git;
			git=it->second.groupParticipants.begin();
			for(; git != it->second.groupParticipants.end(); git++) {
				if(!git->second.bAdded) {
					bResult=true;
					g_log.Log(CELOG_DEBUG, _T("TimeToDoEvaluation\n"));
					break;
				}
			}
		}
	}
	ReleaseMutex(groupSetMutex);
	return bResult;
}

bool PolicyEval::IsSessionTypeChanged(HWND winHandle, CComPtr<IUccSession> session, WCHAR *newParticipantId)
{
	bool bResult=false;
	WaitForSingleObject(groupSetMutex, INFINITE);
	std::map<HWND, GroupSessionInfo>::iterator it=groupSet.find(winHandle);
	if( it != groupSet.end()) {
		enum UCC_SESSION_TYPE newType;
		session->get_Type(&newType);
		if(newType != it->second.sessionType) {
			//session type changed
			//let's make sure that it is only session type changed
			if( (DWORD)(it->second.numParticipants-1) == it->second.groupParticipants.size() ) {
				//Let's make sure the new participant already exists
				bool bExist=false;
				std::map<wstring, GroupParticipant>::iterator pit=it->second.groupParticipants.begin();
				for(; pit!=it->second.groupParticipants.end(); pit++) {
					if(wcscmp(pit->first.c_str(), newParticipantId)==0)
						bExist=true;
				}
				if(bExist) {
					bResult=true;	
					it->second.sessionType=newType;
				}
			}
		}
	}
	ReleaseMutex(groupSetMutex);
	return bResult;
}

// Added By Jacky.Dong 2011-11-23
// ------------------------------
// Do Invite Evaluation
bool PolicyEval::DoInviteEvaluation(BSTR participant, enum UCC_SESSION_TYPE SessionType)
{
	if(!IsPCRunning())
	{
		return true;
	}

	// Connection to policy controller has not been setup.
	if(oceHandle == NULL)
	{
		if(bInitFailed == false)
		{
			// Connection has not been setup, try to set it up.
			SetupConnection();
			// Setup connection failed, return true to allow.
			if(bInitFailed == true)
				return true;
		} else // Connection setup failed, return true to allow.
			return true;
	}

	// We don't know the current endpointsip, return true to allow
	if(endPointSip == NULL)
		return true;

	// If the current participant is the same as the endPointSip, allow.
	wchar_t selfsip[256 + 1] = {0};
	wsprintf(selfsip, L"sip:%s", endPointSip);
	if(NULL != wcsstr(participant, selfsip))
		return true;

	// invite evaluation is unilateral, if the participant suffix with char ';', it means the invite evaluation occurs in receiver oce.
	if(NULL != wcsstr(participant, L";"))
		return true;

	bool bAllow = true;

	// Decide action
	CEAction_t action = CE_ACTION_MEETING;
	// Get sourceString
	CEString sourceString = m_sdkLoader.fns.CEM_AllocateString(_T("C:\\No_attachment.ice"));
	// We still need to setup dummy attributes since we are using CheckMsgAttachment
	CEString key = m_sdkLoader.fns.CEM_AllocateString(_T("modified_date"));
	CEString val = m_sdkLoader.fns.CEM_AllocateString(_T("1234567"));
	CEAttribute dummyAttr[1];
	CEAttributes dummyAttrs;  
	dummyAttr[0].key = key;
	dummyAttr[0].value = val;
	dummyAttrs.count = 1;
	dummyAttrs.attrs = dummyAttr;
	// Get recipents
	CEString* recipients = new CEString[2];
	recipients[0] = m_sdkLoader.fns.CEM_AllocateString(endPointSip);
	// if the participant have suffix params, should remove them, and they start with char ';'
	// ---------------------------------------------------------------------------------------
	wchar_t* pos = wcsstr(participant, _T(";"));
	if(NULL != pos)
	{
		BSTR participantsip = SysAllocStringLen(participant, static_cast<UINT>(pos - participant));
		recipients[1] = m_sdkLoader.fns.CEM_AllocateString(participantsip + wcslen(L"sip:"));
		SysFreeString(participantsip);

		pos = NULL;
	}
	else
		recipients[1] = m_sdkLoader.fns.CEM_AllocateString(participant + wcslen(L"sip:"));
	// ---------------------------------------------------------------------------------------

	std::set<wstring> disclaimerSet;
	std::set<wstring> warnObligations;
	bool bDummy;

	CEUser sender;
	CEResult_t res;
	CEEnforcement_t enforcement;

	// Do Invite Evaluation Start
	g_log.Log(CELOG_DEBUG, _T("OCE Invite Evaluation Start\n"));
	for(int i = 0; i < 1; i++) // for(int i = 0; i < 2; i++) // evaluation from bilateral to unilateral
	{
		sender.userName = m_sdkLoader.fns.CEM_AllocateString(m_sdkLoader.fns.CEM_GetString(recipients[i]));
		sender.userID = m_sdkLoader.fns.CEM_AllocateString(m_sdkLoader.fns.CEM_GetString(recipients[i]));

		res = m_sdkLoader.fns.CEEVALUATE_CheckMessageAttachment(
			oceHandle, 
			action, 
			sourceString, //attachment source
			&dummyAttrs, //source attributes
			1, //number of receipients
			(i == 0) ? (&recipients[1]) : (&recipients[0]), // recipients
			(CEint32)((hostIPAddr.s_addr)),//ip address
			&sender, //sender 
			NULL, //sender attributes
			NULL, //application
			NULL, //application attributes
			CETrue, //performObligation 
			CE_NOISE_LEVEL_USER_ACTION, //noise level
			&enforcement, //returned enforcement
			OCE_EVALUATION_TIMEOUT); //timeout in milliseconds

		m_sdkLoader.fns.CEM_FreeString(sender.userName);
		m_sdkLoader.fns.CEM_FreeString(sender.userID);

		// Check the result
		if(res == CE_RESULT_SUCCESS)
		{ 
			if(bAllow)
				bAllow = (enforcement.result == CEDeny) ? false : true;
			
			//Parse Obligation
			bool bUserCancel=false;
			if(bAllow) {
				ParseObligation(disclaimerSet, warnObligations, &enforcement, 2, bUserCancel, bDummy);
				if(SessionType == UCCST_INSTANT_MESSAGING) {
					std::set<wstring>::iterator it=disclaimerSet.begin();
					for(; it!=disclaimerSet.end(); it++)
						AddDisclaimer(participant, it->c_str());
					disclaimerSet.clear();
				}
			}
			
			//Free enforcement
			m_sdkLoader.fns.CEEVALUATE_FreeEnforcement(enforcement);

			//The user chose to cancel the action
			if(bUserCancel) {
				bAllow=false;
				break;
			}
			
			g_log.Log(CELOG_DEBUG, L"\nOCE do evaluation (DoInviteEvaluation): \nAction: %d (0 Instant Message, 1 Audio, 2 Conference, 3 Application, 4 Sharing)\nSender: %s\nRecepient: %s\nresult: %s\n", 
					SessionType, m_sdkLoader.fns.CEM_GetString(recipients[i]), m_sdkLoader.fns.CEM_GetString ( (i == 0) ? (recipients[1]) : (recipients[0])), bAllow ? L"Allow" : L"Deny" );
		} else 
			g_log.Log(CELOG_DEBUG, _T("CEEVALUATE_CheckMessageAttachment failed: errorno = %d\n"), res);
	}
	g_log.Log(CELOG_DEBUG, _T("OCE Invite Evaluation Over\n"));

	// Cleaning up
	m_sdkLoader.fns.CEM_FreeString(key);
	m_sdkLoader.fns.CEM_FreeString(val);  
	m_sdkLoader.fns.CEM_FreeString(sourceString);
	m_sdkLoader.fns.CEM_FreeString(recipients[0]);
	m_sdkLoader.fns.CEM_FreeString(recipients[1]);
	delete [] recipients;

	//return result
	return bAllow;
}
// ------------------------------

bool PolicyEval::DoGroupChatEvaluation(HWND winHandle, WCHAR *lastParticipant)
{
	if(!IsPCRunning())
	{
		return true;
	}

	//Connection to policy controller has not been setup.
	if(oceHandle == NULL) {
		if(bInitFailed == false) {
			//Connection has not been setup, try to set it up
			SetupConnection();
			//setup connection failed, return true to allow
			if(bInitFailed == true)
				return true;
		} else //connection setup failed, return true to allow
			return true;
	}

	// We don't know the current endpointsip, return true to allow
	if(endPointSip == NULL)
		return true;

	bool bPopupWindow=false; //If true, there is pop-up window for warning. 
	bool bAllow=true;

	WaitForSingleObject(groupSetMutex, INFINITE);
	if(outstandWnd==winHandle)
		outstandWnd=NULL; //This outstand session has been finished.
	std::map<HWND, GroupSessionInfo>::iterator it=groupSet.find(winHandle);

	if( it != groupSet.end() && it->second.numParticipants > 0) { 
		int realNumParticipants=0;
		CEAction_t action;
		CEUser sender;
		CEResult_t res;
		CEEnforcement_t enforcement;
		CEString sourceString=m_sdkLoader.fns.CEM_AllocateString(_T("C:\\No_attachment.ice"));
		CEString *recipients= new CEString[it->second.numParticipants+1];


		//Decide action
		if(it->second.sessionType == UCCST_INSTANT_MESSAGING ||
			it->second.sessionType == UCCST_CONFERENCE)
			action=CE_ACTION_IM_FILE;
		else if(it->second.sessionType==UCCST_AUDIO_VIDEO)
			action=CE_ACTION_AVD;
		// Sharing, Added By Jacky.Dong 2011-11-23
		else if(it->second.sessionType == UCCST_APPLICATION_SHARING)
			action = CE_ACTION_WM_SHARE;
		else 
			action=CE_ACTION_MEETING;

		//Get recipents
		BSTR bstrUri = NULL;IUccUri* pUri = NULL;
		HRESULT hr = it->second.localParticipant->get_Uri(&pUri);pUri->get_AddressOfRecord(&bstrUri);
		if(SUCCEEDED(hr)&&bstrUri) {
			recipients[realNumParticipants++]=m_sdkLoader.fns.CEM_AllocateString(bstrUri+wcslen(L"sip:"));
		}
		std::map<wstring, GroupParticipant>::iterator pit=it->second.groupParticipants.begin();
		for(; pit!=it->second.groupParticipants.end(); pit++)
		{
			// if the participant have suffix params, should remove them, and they start with char ';'
			// ---------------------------------------------------------------------------------------
			BSTR participant = SysAllocString(pit->first.c_str());
			wchar_t* pos = wcsstr(participant, _T(";"));
			if(NULL != pos)
			{
				BSTR participantsip = SysAllocStringLen(participant, static_cast<UINT>(pos - participant));
				recipients[realNumParticipants++] = m_sdkLoader.fns.CEM_AllocateString(participantsip + wcslen(L"sip:"));
				SysFreeString(participantsip);

				pos = NULL;
			}
			else
				recipients[realNumParticipants++] = m_sdkLoader.fns.CEM_AllocateString(participant + wcslen(L"sip:"));
			SysFreeString(participant);
			// ---------------------------------------------------------------------------------------
		}
		if(realNumParticipants == 1 && bstrUri && action == CE_ACTION_AVD)
		{
			m_sdkLoader.fns.CEM_FreeString(recipients[0]);
			delete [] recipients;
			recipients = new CEString[2];
			realNumParticipants = 0;
			recipients[realNumParticipants++] = m_sdkLoader.fns.CEM_AllocateString(L"dummy@dummy.com");
			recipients[realNumParticipants++]=m_sdkLoader.fns.CEM_AllocateString(bstrUri+wcslen(L"sip:"));
		}
		SysFreeString(bstrUri);

		CEString *realRecipients= new CEString[realNumParticipants-1];

		//We still need to setup dummy attributes since we are using CheckMsgAttachment
		CEString key=m_sdkLoader.fns.CEM_AllocateString(_T("modified_date"));
		CEString val=m_sdkLoader.fns.CEM_AllocateString(_T("1234567"));
		CEAttribute dummyAttr[1];
		CEAttributes dummyAttrs;  
		dummyAttr[0].key = key;
		dummyAttr[0].value = val;
		dummyAttrs.count=1;
		dummyAttrs.attrs=dummyAttr;

		g_log.Log(CELOG_DEBUG, _T("We are going to call Eval %d times (DoGroupChatEvaluation), action: %d\n"), realNumParticipants, action);
		for(int i = 0; i < realNumParticipants; i++)
		{
			// Modified By Jacky.Dong 2011-12-08
			std::wstring strTemp;
			if(CE_ACTION_WM_SHARE == action) // In Sharing evaluation, the mode is A->B,C,D,...
			{
				//Assign sender
				sender.userID = m_sdkLoader.fns.CEM_AllocateString(m_sdkLoader.fns.CEM_GetString(recipients[0]));
				sender.userName = m_sdkLoader.fns.CEM_AllocateString(m_sdkLoader.fns.CEM_GetString(recipients[0]));

				//Assign recipients
				for(int j = 1, count = 0; j < realNumParticipants; j++)
				{
					realRecipients[count++] = recipients[j];
					strTemp.append(m_sdkLoader.fns.CEM_GetString(recipients[j]));
					strTemp.append(L"\n");
				}
			}
			else // In other evaluation, the mode is A->B,C,D and B->A,C,D and C->A,B,D and D->A,B,C
			{
				//Assign sender
				sender.userID = m_sdkLoader.fns.CEM_AllocateString(m_sdkLoader.fns.CEM_GetString(recipients[i]));
				sender.userName = m_sdkLoader.fns.CEM_AllocateString(m_sdkLoader.fns.CEM_GetString(recipients[i]));

				//Assign recipients
				for(int j = 0, count = 0; j < realNumParticipants; j++)
				{
					if(j == i)
						continue;
					realRecipients[count++] = recipients[j];
					strTemp.append(m_sdkLoader.fns.CEM_GetString(recipients[j]));
					strTemp.append(L"\n");
				}
			}

			//Do policy evaluation
			res=m_sdkLoader.fns.CEEVALUATE_CheckMessageAttachment(oceHandle, 
										action, 
										sourceString, //attchment source
										&dummyAttrs, //source attributes
										realNumParticipants-1, //number of recipients
										realRecipients, //recipients
										(CEint32)((hostIPAddr.s_addr)),//ip address
										&sender, //sender 
										NULL, //sender attributes
										NULL, //application
										NULL, //application attributes
										CETrue, //(i==realNumParticipants-1)?CETrue:CEFalse, //performObligation 
										CE_NOISE_LEVEL_USER_ACTION, //noise level
										&enforcement, //returned enforcement
										OCE_EVALUATION_TIMEOUT); //timeout in milliseconds

			m_sdkLoader.fns.CEM_FreeString(sender.userID);
			m_sdkLoader.fns.CEM_FreeString(sender.userName);

			//Check the result
			if(res == CE_RESULT_SUCCESS) {
				if(bAllow)
					bAllow=(enforcement.result==CEDeny)?FALSE:TRUE;

				//Get disclaimer
				bool bUserCancel=false;
				if(bAllow) {
					ParseObligation(it->second.groupDisclaimer.disclaimers, 
									it->second.warnObligations,
									&enforcement,realNumParticipants, bUserCancel,
									bPopupWindow);
					g_log.Log(CELOG_DEBUG, _T("Allow: sender=%s\n"), m_sdkLoader.fns.CEM_GetString(recipients[i]));
				} else 
					g_log.Log(CELOG_DEBUG, _T("Deny: sender=%s\n"), m_sdkLoader.fns.CEM_GetString(recipients[i]));

				g_log.Log(CELOG_DEBUG, L"\nOCE do evaluation (DoGroupChatEvaluation): \nAction: %d (0 Instant Message, 1 Audio, 2 Conference, 3 Application, 4 Sharing)\nresult: %s\n", 
					it->second.sessionType, bAllow?L"Allow":L"Deny" );

				//Free enforcement
				m_sdkLoader.fns.CEEVALUATE_FreeEnforcement(enforcement);

				//User chose to cancel the whole action
				if(bUserCancel) {
					bAllow=false;
					break;
				}
			} else 
				g_log.Log(CELOG_DEBUG, _T("CEEVALUATE_CheckMessageAttachment failed: errorno=%d\n"), res);

			g_log.Log(CELOG_DEBUG, L"Do Query: \nAction: %d\nSender: %s\nRecipients: %sAllow: %d\n", action,  m_sdkLoader.fns.CEM_GetString(recipients[i]), strTemp.c_str(), bAllow);

			if(CE_ACTION_WM_SHARE == action) // Sharing evaluation, only one time evaluation, Added By Jacky.Dong 2011-12-08
				break;
		}
		
		if(bAllow)
		{
			if(action==CE_ACTION_IM_FILE)
			{
				it->second.groupDisclaimer.bSendMsg=true;

				g_log.Log(CELOG_DEBUG, L"Try to do disclaimer, DoGroupChatEvaluation. disclaimer size: %d,  imSession: %p, sendMsgFunc: %p, bPopupWindow: %d, window: %d\n", 
										it->second.groupDisclaimer.disclaimers.size(), it->second.groupDisclaimer.imSession, it->second.groupDisclaimer.sendMsgFunc, bPopupWindow, winHandle);

				if( !(lastParticipant && wcscmp(lastParticipant, L"dummyParticipant") == 0) )
				{//DoGroupChatEvaluation was called in My_NewConnPoint_Advise if the lastParticipant is "dummyParticipant". We don't need to handle the disclaimer for this case since My_NewConnPoint_Advise was used for incoming session. fix bug13539
					if(it->second.groupDisclaimer.disclaimers.size()>0 &&
						it->second.groupDisclaimer.imSession != NULL && 
						it->second.groupDisclaimer.sendMsgFunc != NULL ) 
					{	
							//We created a thread to pop up "warn user dialog".
							wstring disclaimer=L"";
							for(std::set<wstring>::iterator dit=it->second.groupDisclaimer.disclaimers.begin();
								dit != it->second.groupDisclaimer.disclaimers.end(); dit++) {
									disclaimer+=(*dit)+L"\n";
							}
							BSTR msgType = SysAllocString(L"text/plain");
							//BSTR msgType = SysAllocString(L"multipart/alternative;");
							BSTR msg=SysAllocString(disclaimer.c_str());
							it->second.groupDisclaimer.sendMsgFunc(
								it->second.groupDisclaimer.imSession,
								msgType,
								msg,
								//NULL
								it->second.groupDisclaimer.pOperationContext
								);
							SysFreeString(msgType);
							SysFreeString(msg);		

							if(DISCLAIMER_ONETIME)//only add disclaimer one time?	
							{
								it->second.groupDisclaimer.bSendMsg=false;						
							}
					}
				}
		
			}

			//Add all participants except the last one
			pit=it->second.groupParticipants.begin();
			for(; pit!=it->second.groupParticipants.end(); pit++) {
				if(!pit->second.bAdded) {
					if(_wcsicmp(pit->first.c_str(), lastParticipant)) {
						CComPtr<IUccSessionParticipant> pParticipant;
						BSTR  myBstr = SysAllocString(pit->first.c_str());
						IUccUri* pUri2 = NULL;
						pit->second.pParticipant->get_Uri(&pUri2);
						HRESULT result=pit->second.session->CreateParticipant(pUri2, NULL,
																&pParticipant);
						result=pit->second.addParticipantFunc(pit->second.session,
													pParticipant,
													NULL);
													//pit->second.pOperationContext);
						SysFreeString(myBstr);
						g_log.Log(CELOG_DEBUG, _T("DoGroupChatEvaluation: Calling addParticipant: %d\n"), 
							result);
					}
					pit->second.bAdded=true;
				}
			}
		}

		//Cleaning the records of warn obligation 
		//so the warn obligation can be revaluated as next evaluation when the group 
		//status is changed (e.g. adding more participant).  
		it->second.warnObligations.clear();

		//If this is first evaluation on the session and action denied, close the chat window.
		if(it->second.bFristEval)
		{
			if(!bAllow)
			{
				//We cached denied result to prevent trying to enter this conference again.
				//groupSet.erase(it);
				
				// if a participant get deny result, should remove from its group.
				std::map<wstring, GroupParticipant>::iterator it_participant = it->second.groupParticipants.find(lastParticipant);
				if(it_participant != it->second.groupParticipants.end())
				{
					it->second.groupParticipants.erase(it_participant);
					it->second.numParticipants--;
				}
			}
		}
		else
		{
			it->second.bLastEvalResult = bAllow;
		}

		//Cleaning up
		m_sdkLoader.fns.CEM_FreeString(key);
		m_sdkLoader.fns.CEM_FreeString(val);
		m_sdkLoader.fns.CEM_FreeString(sourceString);
		for(int i=0; i<realNumParticipants; i++)
			m_sdkLoader.fns.CEM_FreeString(recipients[i]);
		delete [] recipients;
		delete [] realRecipients;
	}
	ReleaseMutex(groupSetMutex);
	return bAllow;
}

bool PolicyEval::DoGroupChatEvaluation_V2(HWND winHandle, WCHAR *lastParticipant, bool& bCancel, IUccConferenceSession** ppConfSession, HWND* phCurrentWnd )
{
	if(!IsPCRunning())
	{
		return true;
	}

	//Connection to policy controller has not been setup.
	if(oceHandle == NULL) {
		if(bInitFailed == false) {
			//Connection has not been setup, try to set it up
			SetupConnection();
			//setup connection failed, return true to allow
			if(bInitFailed == true)
				return true;
		} else //connection setup failed, return true to allow
			return true;
	}

	// We don't know the current endpointsip, return true to allow
	if(endPointSip == NULL)
		return true;


	bCancel = FALSE;
	*ppConfSession = NULL;
	*phCurrentWnd = NULL;

	bool bPopupWindow=false; //If true, there is pop-up window for warning. 
	bool bAllow=true;

	WaitForSingleObject(groupSetMutex, INFINITE);
	if(outstandWnd==winHandle)
		outstandWnd=NULL; //This outstand session has been finished.
	std::map<HWND, GroupSessionInfo>::iterator it=groupSet.find(winHandle);

	if( it != groupSet.end() && it->second.numParticipants > 0) { 
		int realNumParticipants=0;
		CEAction_t action;
		CEUser sender;
		CEResult_t res;
		CEEnforcement_t enforcement;
		CEString sourceString=m_sdkLoader.fns.CEM_AllocateString(_T("C:\\No_attachment.ice"));
		CEString *recipients= new CEString[it->second.numParticipants+1];


		//Decide action
		if(it->second.sessionType == UCCST_INSTANT_MESSAGING ||
			it->second.sessionType == UCCST_CONFERENCE)
			action=CE_ACTION_IM_FILE;
		else if(it->second.sessionType==UCCST_AUDIO_VIDEO)
			action=CE_ACTION_AVD;
		// Sharing, Added By Jacky.Dong 2011-11-23
		else if(it->second.sessionType == UCCST_APPLICATION_SHARING)
			action = CE_ACTION_WM_SHARE;
		else 
			action=CE_ACTION_MEETING;

		//Get recipents
		BSTR bstrUri = NULL;IUccUri* pUri = NULL;
		HRESULT hr = it->second.localParticipant->get_Uri(&pUri);pUri->get_AddressOfRecord(&bstrUri);
		if(SUCCEEDED(hr)&&bstrUri) {
			recipients[realNumParticipants++]=m_sdkLoader.fns.CEM_AllocateString(bstrUri+wcslen(L"sip:"));
		}
		std::map<wstring, GroupParticipant>::iterator pit=it->second.groupParticipants.begin();
		for(; pit!=it->second.groupParticipants.end(); pit++)
		{
			// if the participant have suffix params, should remove them, and they start with char ';'
			// ---------------------------------------------------------------------------------------
			BSTR participant = SysAllocString(pit->first.c_str());
			wchar_t* pos = wcsstr(participant, _T(";"));
			if(NULL != pos)
			{
				BSTR participantsip = SysAllocStringLen(participant, static_cast<UINT>(pos - participant));
				recipients[realNumParticipants++] = m_sdkLoader.fns.CEM_AllocateString(participantsip + wcslen(L"sip:"));
				SysFreeString(participantsip);

				pos = NULL;
			}
			else
				recipients[realNumParticipants++] = m_sdkLoader.fns.CEM_AllocateString(participant + wcslen(L"sip:"));
			SysFreeString(participant);
			// ---------------------------------------------------------------------------------------
		}
		if(realNumParticipants == 1 && bstrUri && action == CE_ACTION_AVD)
		{
			m_sdkLoader.fns.CEM_FreeString(recipients[0]);
			delete [] recipients;
			recipients = new CEString[2];
			realNumParticipants = 0;
			recipients[realNumParticipants++] = m_sdkLoader.fns.CEM_AllocateString(L"dummy@dummy.com");
			recipients[realNumParticipants++]=m_sdkLoader.fns.CEM_AllocateString(bstrUri+wcslen(L"sip:"));
		}
		SysFreeString(bstrUri);

		CEString *realRecipients= new CEString[realNumParticipants-1];

		//We still need to setup dummy attributes since we are using CheckMsgAttachment
		CEString key=m_sdkLoader.fns.CEM_AllocateString(_T("modified_date"));
		CEString val=m_sdkLoader.fns.CEM_AllocateString(_T("1234567"));
		CEAttribute dummyAttr[1];
		CEAttributes dummyAttrs;  
		dummyAttr[0].key = key;
		dummyAttr[0].value = val;
		dummyAttrs.count=1;
		dummyAttrs.attrs=dummyAttr;

		g_log.Log(CELOG_DEBUG, _T("We are going to call Eval %d times (DoGroupChatEvaluation), action: %d\n"), realNumParticipants, action);
		for(int i = 0; i < realNumParticipants; i++)
		{
			// Modified By Jacky.Dong 2011-12-08
			std::wstring strTemp;
			if(CE_ACTION_WM_SHARE == action) // In Sharing evaluation, the mode is A->B,C,D,...
			{
				//Assign sender
				sender.userID = m_sdkLoader.fns.CEM_AllocateString(m_sdkLoader.fns.CEM_GetString(recipients[0]));
				sender.userName = m_sdkLoader.fns.CEM_AllocateString(m_sdkLoader.fns.CEM_GetString(recipients[0]));

				//Assign recipients
				for(int j = 1, count = 0; j < realNumParticipants; j++)
				{
					realRecipients[count++] = recipients[j];
					strTemp.append(m_sdkLoader.fns.CEM_GetString(recipients[j]));
					strTemp.append(L"\n");
				}
			}
			else // In other evaluation, the mode is A->B,C,D and B->A,C,D and C->A,B,D and D->A,B,C
			{
				//Assign sender
				sender.userID = m_sdkLoader.fns.CEM_AllocateString(m_sdkLoader.fns.CEM_GetString(recipients[i]));
				sender.userName = m_sdkLoader.fns.CEM_AllocateString(m_sdkLoader.fns.CEM_GetString(recipients[i]));

				//Assign recipients
				for(int j = 0, count = 0; j < realNumParticipants; j++)
				{
					if(j == i)
						continue;
					realRecipients[count++] = recipients[j];
					strTemp.append(m_sdkLoader.fns.CEM_GetString(recipients[j]));
					strTemp.append(L"\n");
				}
			}

			//Do policy evaluation
			res=m_sdkLoader.fns.CEEVALUATE_CheckMessageAttachment(oceHandle, 
										action, 
										sourceString, //attchment source
										&dummyAttrs, //source attributes
										realNumParticipants-1, //number of recipients
										realRecipients, //recipients
										(CEint32)((hostIPAddr.s_addr)),//ip address
										&sender, //sender 
										NULL, //sender attributes
										NULL, //application
										NULL, //application attributes
										CETrue, //(i==realNumParticipants-1)?CETrue:CEFalse, //performObligation 
										CE_NOISE_LEVEL_USER_ACTION, //noise level
										&enforcement, //returned enforcement
										OCE_EVALUATION_TIMEOUT); //timeout in milliseconds

			m_sdkLoader.fns.CEM_FreeString(sender.userID);
			m_sdkLoader.fns.CEM_FreeString(sender.userName);

			//Check the result
			if(res == CE_RESULT_SUCCESS) {
				if(bAllow)
					bAllow=(enforcement.result==CEDeny)?FALSE:TRUE;

				//Get disclaimer
				if(bAllow) 
				{
					// if the number of participants equals to the number of groupSet of the conference,
					// means that the last participant has joined in the conference, then do Obligation.
					if(it->second.numParticipants == realNumParticipants)
					{
						ParseObligation(it->second.groupDisclaimer.disclaimers,
							it->second.warnObligations,
							&enforcement,realNumParticipants, bCancel,
							bPopupWindow);
					}
					g_log.Log(CELOG_DEBUG, _T("Allow: sender=%s\n"), m_sdkLoader.fns.CEM_GetString(recipients[i]));
				} 
				else
				{
					g_log.Log(CELOG_DEBUG, _T("Deny: sender=%s\n"), m_sdkLoader.fns.CEM_GetString(recipients[i]));
				}

				g_log.Log(CELOG_DEBUG, L"\nOCE do evaluation (DoGroupChatEvaluation): \nAction: %d (0 Instant Message, 1 Audio, 2 Conference, 3 Application, 4 Sharing)\nresult: %s\n", 
					it->second.sessionType, bAllow?L"Allow":L"Deny" );

				//Free enforcement
				m_sdkLoader.fns.CEEVALUATE_FreeEnforcement(enforcement);

				//User chose to cancel the whole action
				if(bCancel) 
				{
					bAllow=false;
					*ppConfSession = it->second.confSession;
					*phCurrentWnd = it->second.winHandle;
					g_log.Log(CELOG_DEBUG, _T("user cancel!!!!!!!\n"));
					break;
				}
			} else 
				g_log.Log(CELOG_DEBUG, _T("CEEVALUATE_CheckMessageAttachment failed: errorno=%d\n"), res);

			g_log.Log(CELOG_DEBUG, L"Do Query: \nAction: %d\nSender: %s\nRecipients: %sAllow: %d\n", action,  m_sdkLoader.fns.CEM_GetString(recipients[i]), strTemp.c_str(), bAllow);

			if(CE_ACTION_WM_SHARE == action) // Sharing evaluation, only one time evaluation, Added By Jacky.Dong 2011-12-08
				break;
		}
		
		if(bAllow)
		{
			if(action==CE_ACTION_IM_FILE)
			{
				it->second.groupDisclaimer.bSendMsg=true;

				g_log.Log(CELOG_DEBUG, L"Try to do disclaimer, DoGroupChatEvaluation. disclaimer size: %d,  imSession: %p, sendMsgFunc: %p, bPopupWindow: %d, window: %d\n", 
										it->second.groupDisclaimer.disclaimers.size(), it->second.groupDisclaimer.imSession, it->second.groupDisclaimer.sendMsgFunc, bPopupWindow, winHandle);

				if( !(lastParticipant && wcscmp(lastParticipant, L"dummyParticipant") == 0) )
				{//DoGroupChatEvaluation was called in My_NewConnPoint_Advise if the lastParticipant is "dummyParticipant". We don't need to handle the disclaimer for this case since My_NewConnPoint_Advise was used for incoming session. fix bug13539
					if(it->second.groupDisclaimer.disclaimers.size()>0 &&
						it->second.groupDisclaimer.imSession != NULL && 
						it->second.groupDisclaimer.sendMsgFunc != NULL ) 
					{	
							//We created a thread to pop up "warn user dialog".
							wstring disclaimer=L"";
							for(std::set<wstring>::iterator dit=it->second.groupDisclaimer.disclaimers.begin();
								dit != it->second.groupDisclaimer.disclaimers.end(); dit++) {
									disclaimer+=(*dit)+L"\n";
							}
							BSTR msgType = SysAllocString(L"text/plain");
							//BSTR msgType = SysAllocString(L"multipart/alternative;");
							BSTR msg=SysAllocString(disclaimer.c_str());

							g_log.Log(CELOG_DEBUG, L"call sendMsgFunc in dogroupchatevaluation\n");							

							it->second.groupDisclaimer.sendMsgFunc(
								it->second.groupDisclaimer.imSession,
								msgType,
								msg,
								//NULL
								it->second.groupDisclaimer.pOperationContext
								);
							SysFreeString(msgType);
							SysFreeString(msg);		

							if(DISCLAIMER_ONETIME)//only add disclaimer one time?	
							{
								it->second.groupDisclaimer.bSendMsg=false;						
							}
					}
				}
		
			}

			//Add all participants except the last one
			pit=it->second.groupParticipants.begin();
			for(; pit!=it->second.groupParticipants.end(); pit++) {
				if(!pit->second.bAdded) {
					if(_wcsicmp(pit->first.c_str(), lastParticipant)) {
						CComPtr<IUccSessionParticipant> pParticipant;
						BSTR  myBstr = SysAllocString(pit->first.c_str());
						IUccUri* pUri2 = NULL;
						pit->second.pParticipant->get_Uri(&pUri2);
						HRESULT result=pit->second.session->CreateParticipant(pUri2, NULL,
																&pParticipant);
						result=pit->second.addParticipantFunc(pit->second.session,
													pParticipant,
													NULL);
													//pit->second.pOperationContext);
						SysFreeString(myBstr);
						g_log.Log(CELOG_DEBUG, _T("DoGroupChatEvaluation: Calling addParticipant: %d\n"), 
							result);
					}
					pit->second.bAdded=true;
				}
			}
		}

		//Cleaning the records of warn obligation 
		//so the warn obligation can be revaluated as next evaluation when the group 
		//status is changed (e.g. adding more participant).  
		it->second.warnObligations.clear();

		//If this is first evaluation on the session and action denied, close the chat window.
		if(it->second.bFristEval)
		{
			if(!bAllow)
			{
				//We cached denied result to prevent trying to enter this conference again.
				//groupSet.erase(it);
				
				// if a participant get deny result, should remove from its group.
				std::map<wstring, GroupParticipant>::iterator it_participant = it->second.groupParticipants.find(lastParticipant);
				if(it_participant != it->second.groupParticipants.end())
				{
					it->second.groupParticipants.erase(it_participant);
					it->second.numParticipants--;
				}
			}
		}
		else
		{
			it->second.bLastEvalResult = bAllow;
		}

		//Cleaning up
		m_sdkLoader.fns.CEM_FreeString(key);
		m_sdkLoader.fns.CEM_FreeString(val);
		m_sdkLoader.fns.CEM_FreeString(sourceString);
		for(int i=0; i<realNumParticipants; i++)
			m_sdkLoader.fns.CEM_FreeString(recipients[i]);
		delete [] recipients;
		delete [] realRecipients;
	}
	ReleaseMutex(groupSetMutex);
	return bAllow;
}
//5. When the group chat finished, call function IUccConferenceSession::Leave. Here,
//   we remove the record from groupSet.
void PolicyEval::RemoveGroupChat(HWND winHandle)
{
	WaitForSingleObject(groupSetMutex, INFINITE);
	if(outstandWnd==winHandle)
		outstandWnd=NULL; //The outstand session has been completed.
	std::map<HWND, GroupSessionInfo>::iterator it=groupSet.find(winHandle);
	if( it != groupSet.end()) 
	{
		g_log.Log(CELOG_DEBUG, L"RemoveGroupChat: winHandle=[%d]\n", winHandle);
		groupSet.erase(it);
	}
	ReleaseMutex(groupSetMutex);
}
bool PolicyEval::IsInOutgoingGroupSet(WCHAR *confMeetingID, long numParticipants) 
{
	bool bTrue=false;
	WaitForSingleObject(groupSetMutex, INFINITE);
	std::map<HWND, GroupSessionInfo>::iterator it=groupSet.begin();
	for(; it!=groupSet.end(); it++) {
		g_log.Log(CELOG_DEBUG, _T("IsInOutgoingGroupSet: %s id=%s numP=%d Pnum=%d\n"), 
			it->second.confURI.c_str(), confMeetingID, it->second.numParticipants, numParticipants);
		if(wcsstr(it->second.confURI.c_str(), confMeetingID)) {
			if(it->second.numParticipants == numParticipants) {	
				bTrue=true;
				break;
			}
		}
	}
	ReleaseMutex(groupSetMutex);
	return bTrue;
}
bool PolicyEval::IsInOutgoingGroupSet(WCHAR *confMeetingID) 
{
	bool bTrue=false;
	WaitForSingleObject(groupSetMutex, INFINITE);
	std::map<HWND, GroupSessionInfo>::iterator it=groupSet.begin();
	for(; it!=groupSet.end(); it++) {
		if(wcsstr(it->second.confURI.c_str(), confMeetingID)) {
			bTrue=true;
			break;
		}		
	}
	ReleaseMutex(groupSetMutex);
	return bTrue;
}
/*End--Group Chat Handling*/
/*........ Session*/
bool PolicyEval::DoGroupEvalOnIncomingSession(CComPtr<IUccSession> pSession)
{	
	if(!IsPCRunning())
	{
		return true;
	}

	//Connection to policy controller has not been setup.
	if(oceHandle == NULL) {
		if(bInitFailed == false) {
			//Connection has not been setup, try to set it up
			SetupConnection();
			//setup connection failed, return true to allow
			if(bInitFailed == true)
				return true;
		} else //connection setup failed, return true to allow
			return true;
	}

	UCC_SESSION_TYPE sessionType;
	pSession->get_Type(&sessionType);
	WCHAR confMeetingID[1024];
	confMeetingID[0]=L'\0';

	if(sessionType == UCCST_CONFERENCE) {
		//Get conference meeting ID
		CComPtr<IUccConferenceSession> pConf;
		HRESULT hr=pSession->QueryInterface(IID_IUccConferenceSession, (void **)(&pConf));
		if(hr==S_OK && pConf) {
			IUccReadOnlyPropertyCollection *pVal;
			pConf->get_Properties(&pVal);
			IUccProperty *pProperty;
			pVal->get_Property(UCCCSPID_CONFERENCE_DATA_URI, &pProperty);
			BSTR bstr;
			hr=pProperty->get_StringValue(&bstr);
			if(hr==S_OK && bstr) {
				const wchar_t *endPos=wcsstr(bstr, L";gruu;opaque=app:conf:meeting:id:");
				if(endPos) {
					endPos+=wcslen(L";gruu;opaque=app:conf:meeting:id:");
					size_t len=wcslen(endPos);
					wcsncpy_s(confMeetingID, endPos, len);
					confMeetingID[len]=L'\0';
					g_log.Log(CELOG_DEBUG, _T("Conference session(%p): UCCCSPID_CONFERENCE_DATA_URI=%s\n"), pSession, confMeetingID);
				}			
			}
		}
	}

	CComPtr<IUccCollection> pParticipants; 
	long numParticipants;

	pSession->get_Participants(&pParticipants );
	pParticipants->get_Count(&(numParticipants));
	WaitForSingleObject(incomingGroupSetMutex, INFINITE);
	std::map<CComPtr<IUccSession>, GroupSessionInfo>::iterator it=incomingGroupSet.end();

	if(numParticipants < 3) {
		if(sessionType == UCCST_CONFERENCE) {
			//If this one is already in outgoing group set "groupSet" (we won't worry about the 
			//number of participant changed to less than group number), 
			//we are not going to do policy evaluation to avoid redundant evaluations.
			if(IsInOutgoingGroupSet(confMeetingID)) {
				ReleaseMutex(incomingGroupSetMutex);	
				return true;
			}
			it=incomingGroupSet.find(pSession);
			if(it==incomingGroupSet.end()) {
				//We store this session to incomingGroupSet. So later when livemeeting is launched for 
				//this session and add more participant, we can have a record to do policy evaluation.
				GroupSessionInfo g;
				g.sessionType = UCCST_INSTANT_MESSAGING;
				if(wcslen(confMeetingID) > 0)
					g.confURI=confMeetingID;
				g.bLastEvalResult=true;
				incomingGroupSet[pSession]=g;
				ReleaseMutex(incomingGroupSetMutex);	
				return true;
			} else {//else: this session is recorded as incoming group session, so do evaluation.
				// Add By Jacky.Dong 2011-11-21
				// ----------------------------
				// Get Current Window's SessionType
				HWND hCurrentWin = GetForegroundWindow();
				if(true == GetSessionTypeByHWND(hCurrentWin, &it->second.sessionType))
				{
					g_log.Log(CELOG_DEBUG, "Jacky Log Invoke GetSessionTypeByHWND Success: Window %d, SessionType %d\n", it->second.winHandle, it->second.sessionType);
				}
				// ----------------------------
				if(numParticipants<2) {
					//Too few participants, skip evaluation.
					ReleaseMutex(incomingGroupSetMutex);	
					return true;
				}
			}
		} else {
			ReleaseMutex(incomingGroupSetMutex);	
			return true;
		}
	} else {
		if(sessionType == UCCST_CONFERENCE) {
			//If this one is already in outgoing group set "groupSet" and there is no changes on
			//number of participant, we are not going to do policy evaluation to avoid redundant
			//evaluations.
			if(IsInOutgoingGroupSet(confMeetingID, numParticipants)) {
				ReleaseMutex(incomingGroupSetMutex);	
				return true;
			}
		}
	}

	g_log.Log(CELOG_DEBUG, _T("DoGroupEvalOnIncomingSession\n"));
	int realNumParticipants=0;
	CEAction_t action=CE_ACTION_IM_FILE;
	CEUser sender;
	CEResult_t res;
	CEEnforcement_t enforcement;
	CEString sourceString=m_sdkLoader.fns.CEM_AllocateString(_T("C:\\No_attachment.ice"));
	bool bAllow=true;
	bool bAddToSet=false;


	//Check if the session already exists
	if(it == incomingGroupSet.end())
	{
		it=incomingGroupSet.find(pSession);
	}
	//If yes, check the group session real type
	if(it != incomingGroupSet.end()) 
	{
		if(it->second.sessionType == UCCST_AUDIO_VIDEO) 
		{
			g_log.Log(CELOG_DEBUG, _T("DoGroupIMEvalOnIncomingSession for AVD\n"));
			action=CE_ACTION_AVD;
		} 
		else if(it->second.sessionType == UCCST_APPLICATION) 
		{
			g_log.Log(CELOG_DEBUG, _T("DoGroupIMEvalOnIncomingSession for meeting\n"));
			action=CE_ACTION_MEETING;
		}
	} 
	else 
	{
		//Add to the incoming group session 
		bAddToSet=true;
		if(activeIncomingSessionType == UCCST_AUDIO_VIDEO) 
			action=CE_ACTION_AVD;
		else if(activeIncomingSessionType == UCCST_APPLICATION)
			action=CE_ACTION_MEETING;
	}

	//Get recipents
	std::vector<CEString> recipients;
	CComQIPtr<IUccSessionParticipant> pParticipant; 
	for(int i=1; i<=numParticipants; i++) {
		CComVariant vtItem;
		vtItem.vt = VT_DISPATCH;
		pParticipants->get_Item(i, &vtItem);
		pParticipant = vtItem.pdispVal;
		BSTR bstrUri = NULL;IUccUri* pUri = NULL;
		HRESULT hr = pParticipant->get_Uri(&pUri);pUri->get_AddressOfRecord(&bstrUri);
		if(SUCCEEDED(hr)&&bstrUri) {
			recipients.push_back( m_sdkLoader.fns.CEM_AllocateString( bstrUri + wcslen( L"sip:" ) ) );
			realNumParticipants++;
		}
		SysFreeString(bstrUri);
	}
	CEString *realRecipients= new CEString[realNumParticipants-1];

	//We still need to setup dummy attributes since we are using CheckMsgAttachment
	CEString key=m_sdkLoader.fns.CEM_AllocateString(_T("modified_date"));
	CEString val=m_sdkLoader.fns.CEM_AllocateString(_T("1234567"));
	CEAttribute dummyAttr[1];
	CEAttributes dummyAttrs;  
	dummyAttr[0].key = key;
	dummyAttr[0].value = val;
	dummyAttrs.count=1;
	dummyAttrs.attrs=dummyAttr;

	g_log.Log(CELOG_DEBUG, _T("We are going to call Eval %d times (DoGroupEvalOnIncomingSession)\n"), realNumParticipants);
	for(int i=0; i<realNumParticipants; i++) {
		//Assign sender
		sender.userID=m_sdkLoader.fns.CEM_AllocateString(m_sdkLoader.fns.CEM_GetString(recipients[i]));
		sender.userName=m_sdkLoader.fns.CEM_AllocateString(m_sdkLoader.fns.CEM_GetString(recipients[i]));

		//Assign recipients
		for(int j=0, count=0; j<realNumParticipants; j++) {
			if(j==i)
				continue;
			realRecipients[count++]=recipients[j];
		}

		//Do policy evaluation
		res=m_sdkLoader.fns.CEEVALUATE_CheckMessageAttachment(oceHandle, 
										action, 
										sourceString, //attachment source
										&dummyAttrs, //source attributes
										realNumParticipants-1, //number of recipients
										realRecipients, //recipients
										(CEint32)((hostIPAddr.s_addr)),//ip address
										&sender, //sender 
										NULL, //sender attributes
										NULL, //application
										NULL, //application attributes
										CETrue, //perform obligation 
										CE_NOISE_LEVEL_USER_ACTION, //noise level
										&enforcement, //returned enforcement
										OCE_EVALUATION_TIMEOUT); //timeout in milliseconds

		m_sdkLoader.fns.CEM_FreeString(sender.userID);
		m_sdkLoader.fns.CEM_FreeString(sender.userName);

		//Check the result
		if(res == CE_RESULT_SUCCESS) {
			if(bAllow)
				bAllow=(enforcement.result==CEDeny)?FALSE:TRUE;

			g_log.Log(CELOG_DEBUG, L"\nOCE do evaluation (DoGroupEvalOnIncomingSession): \nAction: %d (0 Instant Message, 1 Audio, 2 conference, 3 application)\nresult: %s\n", 
				action, bAllow?L"Allow":L"Deny" );

			//For incoming session, we don't handle disclaimer and warn obligation
			//Free enforcement
			m_sdkLoader.fns.CEEVALUATE_FreeEnforcement(enforcement);
		} else 
			g_log.Log(CELOG_DEBUG, _T("CEEVALUATE_CheckMessageAttachment failed: errorno=%d\n"), res);
	}

	//Cleaning up
	m_sdkLoader.fns.CEM_FreeString(key);
	m_sdkLoader.fns.CEM_FreeString(val);
	m_sdkLoader.fns.CEM_FreeString(sourceString);
	for(int i=0; i<realNumParticipants; i++)
		m_sdkLoader.fns.CEM_FreeString(recipients[i]);
	recipients.clear();
	delete [] realRecipients;

	if(bAllow) {
		if(bAddToSet) {
			if(activeIncomingSessionType == UCCST_INSTANT_MESSAGING) {
				//Add to incoming group set
				GroupSessionInfo g;
				g.sessionType = UCCST_INSTANT_MESSAGING;
				if(wcslen(confMeetingID) > 0)
					g.confURI=confMeetingID;
				g.bLastEvalResult=true;
				incomingGroupSet[pSession]=g;
				activeIncomingSession=pSession;
			}
		}
	} else {
		if(!bAddToSet) {
			//We cached denied result to prevent trying to enter this conference again.
			it->second.bLastEvalResult=false;
			//incomingGroupSet.erase(it);
		} else {
			//Add to incoming group set
			//So we can prevent entering this conference again.
			GroupSessionInfo g;
			g.sessionType = UCCST_INSTANT_MESSAGING;
			if(wcslen(confMeetingID) > 0)
				g.confURI=confMeetingID;
			g.bLastEvalResult=false;
			incomingGroupSet[pSession]=g;
		}
	}
	
	if(activeIncomingSessionType != UCCST_INSTANT_MESSAGING) {
		//This means that the current active session is not IM type and 
		//its initialization is done. 
		//This is for the case that a peer-to-peer AVDCALL change to group AVDCALL:
		//During initailization first add participant for conference leader; 
		//then call Adivce for each participant. 
		activeIncomingSessionType=UCCST_INSTANT_MESSAGING;
		activeIncomingSession=NULL;
	}

	ReleaseMutex(incomingGroupSetMutex);	

	return bAllow;
}
void PolicyEval::SetActiveGroupIncomingSesion(CComPtr<IUccSession> pSession)
{	
	//Connection to policy controller has not been setup.
	if(oceHandle == NULL) {
		if(bInitFailed == false) {
			//Connection has not been setup, try to set it up
			SetupConnection();
			//setup connection failed, return true to allow
			if(bInitFailed == true)
				return;
		} else //connection setup failed, return true to allow
			return;
	}

	if(pSession == NULL) {
		WaitForSingleObject(incomingGroupSetMutex, INFINITE);
		activeIncomingSession=pSession;
		ReleaseMutex(incomingGroupSetMutex);	
		return;
	}

	CComPtr<IUccCollection> pParticipants; 
	long numParticipants;

	pSession->get_Participants(&pParticipants );
	pParticipants->get_Count(&(numParticipants));

	if(numParticipants < 3) {
		ReleaseMutex(incomingGroupSetMutex);	
		return;
	}

	WaitForSingleObject(incomingGroupSetMutex, INFINITE);
	activeIncomingSession=pSession;
	ReleaseMutex(incomingGroupSetMutex);		

	return;
}
bool PolicyEval::DoGroupNonIMEvalOnActiveIncomingSession(UCC_SESSION_TYPE type, BSTR bstrConfURI, 
														 bool &bCallSDKEval)
{	
	if(!IsPCRunning() || !bstrConfURI)
	{
		return true;
	}

	//Connection to policy controller has not been setup.
	if(oceHandle == NULL) {
		if(bInitFailed == false) {
			//Connection has not been setup, try to set it up
			SetupConnection();
			//setup connection failed, return true to allow
			if(bInitFailed == true)
				return true;
		} else //connection setup failed, return true to allow
			return true;
	}

	WaitForSingleObject(incomingGroupSetMutex, INFINITE);
	std::map<CComPtr<IUccSession>, GroupSessionInfo>::iterator it=incomingGroupSet.end();
	WCHAR confMeetingID[1024];
	confMeetingID[0]=L'\0';

	g_log.Log(CELOG_DEBUG, L"DoGroupNonIMEvalOnActiveIncomingSession, activeIncomingSession: %x, Uri: %s, type: %d\n", activeIncomingSession, bstrConfURI, type);

	if(activeIncomingSession == NULL) {
		if(type == UCCST_AUDIO_VIDEO)
		{//fix bug13469
			if(!CheckURIWithoutIncomingSession(bstrConfURI))
			{
				return false;
			}
		}
		const wchar_t *endPos=wcsstr(bstrConfURI, L";gruu;opaque=app:conf:focus:id:");
		if(endPos != NULL)
			endPos+=wcslen(L";gruu;opaque=app:conf:focus:id:");
		else {
			endPos=wcsstr(bstrConfURI, L";gruu;opaque=app:conf:audio-video:id:");
			if(endPos != NULL)
				endPos+=wcslen(L";gruu;opaque=app:conf:audio-video:id:");
		}
		if(endPos != NULL) {
			size_t len=wcslen(endPos);
			wcsncpy_s(confMeetingID, endPos, len);
			confMeetingID[len]=L'\0';
			for(it=incomingGroupSet.begin(); it != incomingGroupSet.end(); it++) {
				if(wcscmp(confMeetingID, it->second.confURI.c_str()) == 0) {
					g_log.Log(CELOG_DEBUG, _T("This addparticipant session has the same conf-id(%s) as session(%p)\n"),
						confMeetingID, it->first);
					break;
				}			
			}
		}
		if(it == incomingGroupSet.end()) {
			//There is no active incoming session. So we record the type here.
			//That means that the current active session is non in IM type and
			//its initialization starts from here.
			//This is for the case that a peer-to-peer AVDCALL change to group AVDCALL:
			//During initailization first add participant for conference leader; 
			//then call Adivce for each participant. 
			activeIncomingSessionType=type;
			ReleaseMutex(incomingGroupSetMutex);	

			return true;
		}

	} else 
		it=incomingGroupSet.find(activeIncomingSession);

	if(it == incomingGroupSet.end()) {
		ReleaseMutex(incomingGroupSetMutex);	
		return true;
	}

	g_log.Log(CELOG_DEBUG, _T("DoGroupNonIMEvalOnIncomingSession\n"));
	bCallSDKEval=true;

	CComPtr<IUccCollection> pParticipants; 
	long numParticipants;
	int realNumParticipants=0;
	CEAction_t action=CE_ACTION_IM_FILE;
	CEUser sender;
	CEResult_t res;
	CEEnforcement_t enforcement;
	CEString sourceString; 
	bool bAllow=true;

	//Get number of participants
	it->first->get_Participants(&pParticipants );
	pParticipants->get_Count(&(numParticipants));
	if(numParticipants < 2) {
		//Only have one participant, doesn't need to policy evaluation
		ReleaseMutex(incomingGroupSetMutex);	
		return true;
	}

	//Assign source string
	sourceString=m_sdkLoader.fns.CEM_AllocateString(_T("C:\\No_attachment.ice"));

	//Check the group session real type
	it->second.sessionType = type;
	if(it->second.sessionType == UCCST_AUDIO_VIDEO) {
		g_log.Log(CELOG_DEBUG, _T("DoGroupNonIMEvalOnIncomingSession for AVD\n"));
		action=CE_ACTION_AVD;
	} else if(it->second.sessionType == UCCST_APPLICATION) {
		g_log.Log(CELOG_DEBUG, _T("DoGroupNonIMEvalOnIncomingSession for Meeting\n"));
		action=CE_ACTION_MEETING;
	}

	//Assign conf id
	//it->second.confURI=bstrConfURI;
	if(wcslen(confMeetingID)!=0)
		it->second.confURI=confMeetingID;
	else
		it->second.confURI=bstrConfURI;


	//Get recipents
	std::vector<CEString> recipients;
	CComQIPtr<IUccSessionParticipant> pParticipant; 
	for(int i=1; i<=numParticipants; i++) {
		CComVariant vtItem;
		vtItem.vt = VT_DISPATCH;
		pParticipants->get_Item(i, &vtItem);
		pParticipant = vtItem.pdispVal;
		BSTR bstrUri = NULL;IUccUri* pUri = NULL;
		HRESULT hr = pParticipant->get_Uri(&pUri);pUri->get_AddressOfRecord(&bstrUri);
		if(SUCCEEDED(hr)&&bstrUri) {
			recipients.push_back( m_sdkLoader.fns.CEM_AllocateString(bstrUri+wcslen(L"sip:")) );
			realNumParticipants++;
		}
		SysFreeString(bstrUri);
	}
	CEString *realRecipients= new CEString[realNumParticipants-1];

	//We still need to setup dummy attributes since we are using CheckMsgAttachment
	CEString key=m_sdkLoader.fns.CEM_AllocateString(_T("modified_date"));
	CEString val=m_sdkLoader.fns.CEM_AllocateString(_T("1234567"));
	CEAttribute dummyAttr[1];
	CEAttributes dummyAttrs;  
	dummyAttr[0].key = key;
	dummyAttr[0].value = val;
	dummyAttrs.count=1;
	dummyAttrs.attrs=dummyAttr;

	g_log.Log(CELOG_DEBUG, _T("We are going to call Eval %d times (DoGroupNonIMEvalOnActiveIncomingSession)\n"), realNumParticipants);
	for(int i=0; i<realNumParticipants; i++) {
		//Assign sender
		sender.userID=m_sdkLoader.fns.CEM_AllocateString(m_sdkLoader.fns.CEM_GetString(recipients[i]));
		sender.userName=m_sdkLoader.fns.CEM_AllocateString(m_sdkLoader.fns.CEM_GetString(recipients[i]));

		//Assign recipients
		for(int j=0, count=0; j<realNumParticipants; j++) {
			if(j==i)
				continue;
			realRecipients[count++]=recipients[j];
		}

		//Do policy evaluation
		res=m_sdkLoader.fns.CEEVALUATE_CheckMessageAttachment(oceHandle, 
										action, 
										sourceString, //attachment source
										&dummyAttrs, //source attributes
										realNumParticipants-1, //number of recipients
										realRecipients, //recipients
										(CEint32)((hostIPAddr.s_addr)),//ip address
										&sender, //sender 
										NULL, //sender attributes
										NULL, //application
										NULL, //application attributes
										CETrue, //perform obligation 
										CE_NOISE_LEVEL_USER_ACTION, //noise level
										&enforcement, //returned enforcement
										OCE_EVALUATION_TIMEOUT); //timeout in milliseconds

		m_sdkLoader.fns.CEM_FreeString(sender.userID);
		m_sdkLoader.fns.CEM_FreeString(sender.userName);

		//Check the result
		if(res == CE_RESULT_SUCCESS) {
			if(bAllow)
				bAllow=(enforcement.result==CEDeny)?FALSE:TRUE;

			g_log.Log(CELOG_DEBUG, L"\nOCE do evaluation (DoGroupNonIMEvalOnActiveIncomingSession): \nAction: %d (0 Instant Message, 1 Audio, 2 conference, 3 application)\nresult: %s\n", 
				action, bAllow?L"Allow":L"Deny" );

			//For incoming session, we don't handle disclaimer and warn obligation
			//Free enforcement
			m_sdkLoader.fns.CEEVALUATE_FreeEnforcement(enforcement);
		} else 
			g_log.Log(CELOG_DEBUG, _T("CEEVALUATE_CheckMessageAttachment failed: errorno=%d\n"), res);
	}

	//Cleaning up
	m_sdkLoader.fns.CEM_FreeString(key);
	m_sdkLoader.fns.CEM_FreeString(val);
	m_sdkLoader.fns.CEM_FreeString(sourceString);
	for(int i=0; i<realNumParticipants; i++)
		m_sdkLoader.fns.CEM_FreeString(recipients[i]);
	recipients.clear();
	delete [] realRecipients;

	if(!bAllow) {
		//This session is not interesting to us any more
		incomingGroupSet.erase(it);
	}

	//The active incoming session should be set back to NULL now.
	activeIncomingSession=NULL;

	ReleaseMutex(incomingGroupSetMutex);	
	return bAllow;
}
void PolicyEval::RemoveCachedIncommingGroupSession(CComPtr<IUccSession> pSession) 
{
	WaitForSingleObject(incomingGroupSetMutex, INFINITE);
	std::map<CComPtr<IUccSession>, GroupSessionInfo>::iterator it=incomingGroupSet.find(pSession);
	if(it != incomingGroupSet.end()) {
		g_log.Log(CELOG_DEBUG, _T("REMOVE INCOMING GSESSION %s\n"), pSession);
		incomingGroupSet.erase(it);
	}
	ReleaseMutex(incomingGroupSetMutex);	
}
//Find the incoming session based on conference meeting id, and set its real type.
void PolicyEval::FindIncomingSessionAndSetType(WCHAR *confMeetingID, UCC_SESSION_TYPE realType)
{
	WaitForSingleObject(incomingGroupSetMutex, INFINITE);
	std::map<CComPtr<IUccSession>, GroupSessionInfo>::iterator it=incomingGroupSet.begin();
	for(; it != incomingGroupSet.end(); it++) {
		if(wcscmp(confMeetingID, it->second.confURI.c_str()) == 0) {
			it->second.sessionType=realType;
			break;
		}
	}
	ReleaseMutex(incomingGroupSetMutex);	
}
//Find the incoming session based on conference meeting id, and Check its previous Eval Result.
//Return false if the incoming session is not found.
bool PolicyEval::FindIncomingSessionAndCheckEvalResult(WCHAR *confMeetingID, bool &bAllow)
{
	bool bFind=false;
	bAllow=true;
	WaitForSingleObject(incomingGroupSetMutex, INFINITE);
	std::map<CComPtr<IUccSession>, GroupSessionInfo>::iterator it=incomingGroupSet.begin();
	for(; it != incomingGroupSet.end(); it++) {
		if(wcscmp(confMeetingID, it->second.confURI.c_str()) == 0) {
			bFind=true;
			bAllow=it->second.bLastEvalResult;
			break;
		}
	}
	ReleaseMutex(incomingGroupSetMutex);	
	return bFind;
}/*End--Incoming Session*/


bool PolicyEval::CheckURIWithoutIncomingSession(LPCWSTR pszURI)
{
	if(!pszURI)
		return true;

	g_log.Log(CELOG_DEBUG, L"CheckURIWithoutIncomingSession, %s\n", pszURI);

	wstring strURI(pszURI);
	wstring strTemp(pszURI);
	std::transform(strTemp.begin(), strTemp.end(), strTemp.begin(), tolower);

	wstring::size_type nStart = strTemp.find(L"sip:");
	wstring::size_type nEnd = strTemp.find(L";gruu;opaque=app:conf:audio-video:id:");
	if( nStart != wstring::npos &&  nEnd != wstring::npos && nEnd > nStart)
	{
		strURI = strURI.substr(nStart + 4, nEnd - nStart - 4);

		CEAction_t action=CE_ACTION_AVD;
		CEUser sender;
		CEResult_t res;
		CEEnforcement_t enforcement;
		CEString sourceString; 
		bool bAllow=true;

		//Assign source string
		sourceString=m_sdkLoader.fns.CEM_AllocateString(_T("C:\\No_attachment.ice"));

		//Get recipents
		std::vector<CEString> recipients;

		// Modified By Jacky.Dong 2011-12-07
		// recipients.push_back(m_sdkLoader.fns.CEM_AllocateString(L"dummy@dummy.com"));
		recipients.push_back(m_sdkLoader.fns.CEM_AllocateString(endPointSip));
		recipients.push_back(m_sdkLoader.fns.CEM_AllocateString(strURI.c_str()));

		INT_PTR nCount = recipients.size();
		CEString *realRecipients= new CEString[nCount - 1];

		CEString key=m_sdkLoader.fns.CEM_AllocateString(_T("modified_date"));
		CEString val=m_sdkLoader.fns.CEM_AllocateString(_T("1234567"));
		CEAttribute dummyAttr[1];
		CEAttributes dummyAttrs;  
		dummyAttr[0].key = key;
		dummyAttr[0].value = val;
		dummyAttrs.count=1;
		dummyAttrs.attrs=dummyAttr;

		for(int i = 0; i < nCount; i++) 
		{
			//Assign sender
			sender.userID=m_sdkLoader.fns.CEM_AllocateString(m_sdkLoader.fns.CEM_GetString(recipients[i]));
			sender.userName=m_sdkLoader.fns.CEM_AllocateString(m_sdkLoader.fns.CEM_GetString(recipients[i]));

			//Assign recipients
			for(int j=0, count=0; j<nCount; j++) {
				if(j==i)
					continue;
				realRecipients[count++]=recipients[j];
			}

			//Do policy evaluation
			res=m_sdkLoader.fns.CEEVALUATE_CheckMessageAttachment(oceHandle, 
				action, 
				sourceString, //attachment source
				&dummyAttrs, //source attributes
				static_cast<CEint32>(nCount-1), //number of recipients
				realRecipients, //recipients
				(CEint32)((hostIPAddr.s_addr)),//ip address
				&sender, //sender 
				NULL, //sender attributes
				NULL, //application
				NULL, //application attributes
				CETrue, //perform obligation 
				CE_NOISE_LEVEL_USER_ACTION, //noise level
				&enforcement, //returned enforcement
				OCE_EVALUATION_TIMEOUT); //timeout in milliseconds

			m_sdkLoader.fns.CEM_FreeString(sender.userID);
			m_sdkLoader.fns.CEM_FreeString(sender.userName);

			//Check the result
			if(res == CE_RESULT_SUCCESS) {
				bAllow = (enforcement.result == CEDeny)? FALSE:TRUE;

				g_log.Log(CELOG_DEBUG, L"\nOCE do evaluation (CheckURIWithoutIncomingSession): \nAction: %d (0 Instant Message, 1 Audio, 2 conference, 3 application)\nresult: %s\n", 
					action, bAllow?L"Allow":L"Deny" );

				m_sdkLoader.fns.CEEVALUATE_FreeEnforcement(enforcement);
			}
			if(!bAllow)
			{
				break;
			}
		}

		for(int i=0; i<nCount; i++)
			m_sdkLoader.fns.CEM_FreeString(recipients[i]);

		delete []realRecipients;

		g_log.Log(CELOG_DEBUG, L"Do query for %s and %s, allow: %d\n", strURI.c_str(), L"dummy@dummy.com", bAllow);
		return bAllow;
	}

	return true;
}


//	comment by Ben, 2011-12-14
//	I don't want to modify existing code which was not written by me, and it's written in very old years, its logic is complicated, 
//	that's why I don't want to modify them, what I want to do is to add some code that will not affect existing code.
//	in the existing code, there are some evaluations already,
//	but I want to add some extra evaluation code using my own code, because existing code don't cover all cases,
//	e.g. bug 15769, the existing code will not do AVDCALL evaluation, that's why I add my code here.
//	here the general idea is, we store conference session, audio/video session of the same hwnd together in CLiveSessionWnd
//	then we have two cases:
//	case 1,
//	when conference session is advised, we check CLiveSessionWnd to see if we have or not have an audio/video session for the same hwnd.
//	if we have, then, we evaluate AVDCALL against the conference session, because conference session has all participants we need.
//	case 2,
//	on the other side, if an audio/video session is advised, 
//	we check CLiveSessionWnd to see if we have or not have conference session for the same hwnd.
//	if we have, then, we evaluate AVDCALL against the conference session, not the audio/video session, because conference session has all participants we need.
bool PolicyEval::DoEvalOnSession(CComPtr<IUccSession> pSession, CEAction_t action)
{
	g_log.Log(CELOG_DEBUG, _T("enter DoEvalOnSession\n"));

	if(!IsPCRunning())
	{
		return true;
	}

	// Connection to policy controller has not been setup.
	if(oceHandle == NULL)
	{
		if(bInitFailed == false)
		{
			// Connection has not been setup, try to set it up.
			SetupConnection();
			// Setup connection failed, return true to allow.
			if(bInitFailed == true)
				return true;
		} else // Connection setup failed, return true to allow.
			return true;
	}

	

	//	get Participants from input session
	CComPtr<IUccCollection> pParticipants; 
	long numParticipants;
	pSession->get_Participants(&pParticipants );
	pParticipants->get_Count(&(numParticipants));

	//	stop if only has one participant
	if (numParticipants <= 1)
	{
		g_log.Log(CELOG_DEBUG, _T("leave DoEvalOnSession because no enough participants\n"));
		return true;
	}
	
	wstring strParticipants;

	// fill recipients out
	std::vector<CEString> recipients;
	CComQIPtr<IUccSessionParticipant> pParticipant; 
	long realNumParticipants = 0;
	for(int i=1; i<=numParticipants; i++) 
	{
		//	get Participant one by one from Participants
		CComVariant vtItem;
		vtItem.vt = VT_DISPATCH;
		pParticipants->get_Item(i, &vtItem);
		pParticipant = vtItem.pdispVal;


		//	get address of record of the Participant
		BSTR bstrUri = NULL;
		IUccUri* pUri = NULL;
		HRESULT hr = pParticipant->get_Uri(&pUri);
		pUri->get_AddressOfRecord(&bstrUri);

		if(SUCCEEDED(hr)&&bstrUri) 
		{
			recipients.push_back( m_sdkLoader.fns.CEM_AllocateString( bstrUri + wcslen( L"sip:" ) ) );

			//	append all participant name to strParticipants, strParticipants will be used as part of eval_key later
			wstring strParticipant(bstrUri + wcslen( L"sip:" ));
			strParticipants += strParticipant;
			g_log.Log(CELOG_DEBUG, _T("[CEvalCache] append [%s] as part of eval_key\n"), strParticipant.c_str());

			realNumParticipants++;
		}
		SysFreeString(bstrUri);
	}

	CEString *realRecipients= new CEString[realNumParticipants-1];	

	//We still need to setup dummy attributes since we are using CheckMsgAttachment
	CEString key=m_sdkLoader.fns.CEM_AllocateString(_T("modified_date"));
	CEString val=m_sdkLoader.fns.CEM_AllocateString(_T("1234567"));
	CEAttribute dummyAttr[1];
	CEAttributes dummyAttrs;  
	dummyAttr[0].key = key;
	dummyAttr[0].value = val;
	dummyAttrs.count=1;
	dummyAttrs.attrs=dummyAttr;
	CEString sourceString = m_sdkLoader.fns.CEM_AllocateString(_T("C:\\No_attachment.ice"));

	g_log.Log(CELOG_DEBUG, _T("We are going to call Eval %d times (DoEvalOnSession)\n"), realNumParticipants);


	CEUser sender;
	CEEnforcement_t enforcement;
	bool bAllow=true;


	//	check cache first
	CEvalCache* ins = CEvalCache::GetInstance();
	wchar_t szAction[4] = {0};
	wsprintf(szAction, L"%d", action);
	wstring eval_key(szAction);
	eval_key += strParticipants;

	if(ins->query(eval_key))
	{
		//	we have a cache, don't query pdp, return allow directory
		goto exit;
	}

	for(int i=0; i<realNumParticipants; i++) 
	{
		//Assign sender
		sender.userID=m_sdkLoader.fns.CEM_AllocateString(m_sdkLoader.fns.CEM_GetString(recipients[i]));
		sender.userName=m_sdkLoader.fns.CEM_AllocateString(m_sdkLoader.fns.CEM_GetString(recipients[i]));

		//Assign recipients
		for(int j=0, count=0; j<realNumParticipants; j++) 
		{
			if(j==i)
				continue;
			realRecipients[count++]=recipients[j];
		}

		//Do policy evaluation
		CEResult_t res=m_sdkLoader.fns.CEEVALUATE_CheckMessageAttachment(oceHandle, 
			action, 
			sourceString, //attachment source
			&dummyAttrs, //source attributes
			realNumParticipants-1, //number of recipients
			realRecipients, //recipients
			(CEint32)((hostIPAddr.s_addr)),//ip address
			&sender, //sender 
			NULL, //sender attributes
			NULL, //application
			NULL, //application attributes
			CETrue, //perform obligation 
			CE_NOISE_LEVEL_USER_ACTION, //noise level
			&enforcement, //returned enforcement
			OCE_EVALUATION_TIMEOUT); //timeout in milliseconds

		m_sdkLoader.fns.CEM_FreeString(sender.userID);
		m_sdkLoader.fns.CEM_FreeString(sender.userName);

		//Check the result
		if(res == CE_RESULT_SUCCESS) 
		{
			if(bAllow)
				bAllow=(enforcement.result==CEDeny)?FALSE:TRUE;

			bool bUserCancel = false;
			if(bAllow)
			{
				//	parse obligation to check participant limit
				std::set<wstring> disclaimers_empty;
				std::set<wstring> warnObligations_empty;

				bool bPopupWindow = false;
				ParseObligation(disclaimers_empty,
					warnObligations_empty,
					&enforcement,realNumParticipants, bUserCancel,
					bPopupWindow);

				g_log.Log(CELOG_DEBUG, L"DoEvalOnSession: ParseObligation check participant limit result[%s].\n", bUserCancel ? L"cancel" : L"ok");
			}

			g_log.Log(CELOG_DEBUG, L"\nOCE do evaluation (DoEvalOnSession): \nAction: %d (0 Instant Message, 1 Audio, 2 conference, 3 application)\nresult: %s\n", 
				action, bAllow?L"Allow":L"Deny" );

			
			//Free enforcement
			m_sdkLoader.fns.CEEVALUATE_FreeEnforcement(enforcement);

			//User chose to cancel the whole action
			if(bUserCancel) 
			{
				g_log.Log(CELOG_DEBUG, L"DoEvalOnSession: user cancel\n");
				bAllow = false;
				break;
			}
		} 
		else 
		{
			g_log.Log(CELOG_DEBUG, _T("CEEVALUATE_CheckMessageAttachment failed: errorno=%d\n"), res);
		}
	}

exit:

	//	commit cache when it is "allow"
	if (bAllow)
	{
		ins->commit(eval_key);
	}

	//Cleaning up
	m_sdkLoader.fns.CEM_FreeString(key);
	m_sdkLoader.fns.CEM_FreeString(val);
	m_sdkLoader.fns.CEM_FreeString(sourceString);
	for(int i=0; i<realNumParticipants; i++)
	{
		m_sdkLoader.fns.CEM_FreeString(recipients[i]);
	}
	recipients.clear();
	delete [] realRecipients;

	return bAllow;
}

bool PolicyEval::DoShareEvalOnSession(CComPtr<IUccSession> pSession)
{
	g_log.Log(CELOG_DEBUG, _T("enter DoShareEvalOnSession\n"));

	if(!IsPCRunning())
	{
		return true;
	}

	// Connection to policy controller has not been setup.
	if(oceHandle == NULL)
	{
		if(bInitFailed == false)
		{
			// Connection has not been setup, try to set it up.
			SetupConnection();
			// Setup connection failed, return true to allow.
			if(bInitFailed == true)
				return true;
		} else // Connection setup failed, return true to allow.
			return true;
	}



	//	get Participants from input session
	CComPtr<IUccCollection> pParticipants; 
	long numParticipants;
	pSession->get_Participants(&pParticipants );
	pParticipants->get_Count(&(numParticipants));

	//	stop if only has one participant
	if (numParticipants <= 1)
	{
		g_log.Log(CELOG_DEBUG, _T("leave DoShareEvalOnSession because no enough participants\n"));
		return true;
	}	

	//	get local
	CComPtr<IUccSessionParticipant> localParticipant;
	pSession->get_LocalParticipant(&localParticipant);

	//Assign sender while fill all recipients out
	CEUser sender;
	std::vector<CEString> recipients;
	CComQIPtr<IUccSessionParticipant> pParticipant; 
	long realNumParticipants = 0;
	for(int i=1; i<=numParticipants; i++) 
	{
		//	get Participant one by one from Participants
		CComVariant vtItem;
		vtItem.vt = VT_DISPATCH;
		pParticipants->get_Item(i, &vtItem);
		pParticipant = vtItem.pdispVal;

		//	get address of record of the Participant
		BSTR bstrUri = NULL;
		IUccUri* pUri = NULL;
		HRESULT hr = pParticipant->get_Uri(&pUri);
		pUri->get_AddressOfRecord(&bstrUri);

		if(SUCCEEDED(hr)&&bstrUri) 
		{
			if (localParticipant == pParticipant)
			{
				sender.userID=m_sdkLoader.fns.CEM_AllocateString( bstrUri + wcslen( L"sip:" ) );
				sender.userName=m_sdkLoader.fns.CEM_AllocateString( bstrUri + wcslen( L"sip:" ) );
			}
			else
			{
				recipients.push_back( m_sdkLoader.fns.CEM_AllocateString( bstrUri + wcslen( L"sip:" ) ) );
				realNumParticipants++;
			}
		}
		SysFreeString(bstrUri);
	}


	//We still need to setup dummy attributes since we are using CheckMsgAttachment
	CEString key=m_sdkLoader.fns.CEM_AllocateString(_T("modified_date"));
	CEString val=m_sdkLoader.fns.CEM_AllocateString(_T("1234567"));
	CEAttribute dummyAttr[1];
	CEAttributes dummyAttrs;  
	dummyAttr[0].key = key;
	dummyAttr[0].value = val;
	dummyAttrs.count=1;
	dummyAttrs.attrs=dummyAttr;
	CEString sourceString = m_sdkLoader.fns.CEM_AllocateString(_T("C:\\No_attachment.ice"));

	//	assign recipients to array
	CEString *realRecipients= new CEString[realNumParticipants];	
	for(int count=0; count<realNumParticipants; ) 
	{
		realRecipients[count++]=recipients[count];
	}


	CEEnforcement_t enforcement;
	bool bAllow=true;

	//Do policy evaluation
	CEResult_t res=m_sdkLoader.fns.CEEVALUATE_CheckMessageAttachment(oceHandle, 
		CE_ACTION_WM_SHARE, 
		sourceString, //attachment source
		&dummyAttrs, //source attributes
		realNumParticipants, //number of recipients
		realRecipients, //recipients
		(CEint32)((hostIPAddr.s_addr)),//ip address
		&sender, //sender 
		NULL, //sender attributes
		NULL, //application
		NULL, //application attributes
		CETrue, //perform obligation 
		CE_NOISE_LEVEL_USER_ACTION, //noise level
		&enforcement, //returned enforcement
		OCE_EVALUATION_TIMEOUT); //timeout in milliseconds

	m_sdkLoader.fns.CEM_FreeString(sender.userID);
	m_sdkLoader.fns.CEM_FreeString(sender.userName);

	//Check the result
	if(res == CE_RESULT_SUCCESS) 
	{

		bAllow=(enforcement.result==CEDeny)?FALSE:TRUE;

		g_log.Log(CELOG_DEBUG, L"\nOCE do evaluation (DoShareEvalOnSession): \nresult: %s\n", 
			bAllow?L"Allow":L"Deny" );

		//For incoming session, we don't handle disclaimer and warn obligation
		//Free enforcement
		m_sdkLoader.fns.CEEVALUATE_FreeEnforcement(enforcement);
	} 
	else 
	{
		g_log.Log(CELOG_DEBUG, _T("CEEVALUATE_CheckMessageAttachment failed: errorno=%d\n"), res);
	}

	//Cleaning up
	m_sdkLoader.fns.CEM_FreeString(key);
	m_sdkLoader.fns.CEM_FreeString(val);
	m_sdkLoader.fns.CEM_FreeString(sourceString);
	for(int i=0; i<realNumParticipants; i++)
	{
		m_sdkLoader.fns.CEM_FreeString(recipients[i]);
	}
	recipients.clear();
	delete [] realRecipients;

	return bAllow;
}