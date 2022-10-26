#pragma once

//	pre-declarations for interfaces
class CLogMgr;
class CPCStatusMgr;
class CInstalledComptMgr;
class CNotifyMgr;

enum _CEResult_t;

#include <vector>
#include <string>

#include <assert.h>
#include "utilities.h"
#include "EDPMgrUtilities.h"
#include "CEsdk.h"
#include "LogMgr.h"
#include "PCStatusMgr.h"
#include "InstalledComptMgr.h"
#include "NotifySettingMgr.h"
#include "DlgMgr.h"
#include "SysUtils.h"

using namespace std;

#define MAX_ENFORCER_COUNT	30

/*


this class has interfaces got from edpmgrutility.dll.

edpmgrutility.dll is a very important utility dll, 

to see exported functions of it, refer to edpmgrapi.h/edpmgrutility.def in edpmgrutility project.

*/
class CEDPMUtilities
{
private:
	CEDPMUtilities(void)
	{
		m_hLib = NULL;
		m_bInited = FALSE;

		m_pLogMgr = NULL;
		m_pPCMgr = NULL;
		m_pInstalledComptMgr = NULL;
		m_pNotifyMgr = NULL;
		m_pDlgMgr = NULL;
	}

	~CEDPMUtilities(void)
	{
		Uninit();
	}


	CEDPMUtilities(const CEDPMUtilities& );
	void operator = (const CEDPMUtilities& );



	/*
	
	load edpmgrutility.dll under the same module of edp manager in default.

	get needed utilities manager interface from the loaded dll.

	
	
	parameter:

	pszDllPath --	useless now, load depmgrutility.dll under the same module of edp manager in default.
	


	return value:

	true -- ok
	false -- failed


	*/
	BOOL Init(wchar_t* pszDllPath = NULL)
	{
		pszDllPath;
		if (m_bInited)
		{
			//	only load once
			return TRUE;
		}

		//	load library
		wstring installPath;
		if (!edp_manager::CCommonUtilities::GetComponentInstallPath(installPath))
		{
			return FALSE;
		}

#ifdef _WIN64
		installPath += wstring(L"bin\\edpmgrutility.dll");
#else
		installPath += wstring(L"bin\\edpmgrutility32.dll");
#endif

		HMODULE hLib = LoadLibraryW(installPath.c_str());

		wchar_t buf[512] = {0};
		_snwprintf_s(buf, 512, _TRUNCATE, L"Tries to load %s\r\n", installPath.c_str());
		//OutputDebugStringW(buf);

		if (!hLib)
		{
			//	load failed
			return FALSE;
		}

		//	get process address
		GetLogMgrType pfGetLogMgr = (GetLogMgrType)GetProcAddress(hLib, "GetLogMgr");
		GetPCStatusMgrType pfGetPCStatusMgr = (GetPCStatusMgrType)GetProcAddress(hLib, "GetPCStatusMgr");
		GetInstalledComptMgrType pfGetInstalledComptMgr = (GetInstalledComptMgrType)GetProcAddress(hLib, "GetInstalledComptMgr");
		GetNotifyMgrType pfGetNotifyMgr = (GetNotifyMgrType)GetProcAddress(hLib, "GetNotifyMgr");
		GetDlgMgrType pfGetDlgMgr = (GetDlgMgrType)GetProcAddress(hLib, "GetDlgMgr");

		//	check if get process address success
		if (!pfGetLogMgr || !pfGetPCStatusMgr || !pfGetInstalledComptMgr || !pfGetNotifyMgr || !pfGetDlgMgr)
		{
			//	GetProcAddress failed
			FreeLibrary(hLib);
			return FALSE;	
		}

		//	get interface
		CLogMgr* pLogMgr = NULL;
		CPCStatusMgr* pPCMgr = NULL;
		CInstalledComptMgr* pInstCompMgr = NULL;
		CNotifyMgr* pNotifyMgr = NULL;
		CDlgMgr* pDlgMgr = NULL;
		pfGetLogMgr(&pLogMgr);
		pfGetPCStatusMgr(&pPCMgr);
		pfGetInstalledComptMgr(&pInstCompMgr);
		pfGetNotifyMgr(&pNotifyMgr);
		pfGetDlgMgr(&pDlgMgr);

		//	check if get interface success
		if(!pLogMgr || !pPCMgr || !pInstCompMgr || !pNotifyMgr || !pDlgMgr)
		{
			//	get interface failed
			FreeLibrary(hLib);
			return FALSE;
		}

		//	get success
		m_bInited = TRUE;
		m_hLib = hLib;
		m_pLogMgr = pLogMgr;
		m_pPCMgr = pPCMgr;
		m_pInstalledComptMgr = pInstCompMgr;
		m_pNotifyMgr = pNotifyMgr;
		m_pDlgMgr = pDlgMgr;


		memset(buf, 0, sizeof(buf));
		_snwprintf_s(buf, 512, _TRUNCATE, L"CEDPMUtilities::Init() successfully!\r\n");
		//OutputDebugStringW(buf);

		return TRUE;
	}



	/*

	unload depmgrutility.dll

	get needed utilities manager interface from the loaded dll.


	return value:

	true -- ok
	false -- failed


	*/
	BOOL Uninit()
	{
		if (m_hLib)
		{
			FreeLibrary(m_hLib);
		}

		m_hLib = NULL;
		m_bInited = FALSE;

		m_pLogMgr = NULL;
		m_pPCMgr = NULL;
		m_pInstalledComptMgr = NULL;
		m_pNotifyMgr = NULL;

		return TRUE;
	}



public:

	static CEDPMUtilities& GetInstance()
	{
		static CEDPMUtilities edpUtilities;

		edpUtilities.Init();

		return edpUtilities;
	}


	/*
	
	check status of edp manager verbose log.

	return value:

	true -- get ok
	false -- error happened


	parameter:

	bOn -- enabled if is true or disabled if is false

	*/
	BOOL IsVerboseLogOn(BOOL& bOn)
	{
		if (m_pLogMgr)
		{
			if (!m_pLogMgr->GetVerlogStatus(bOn))
			{
				//	error
				return FALSE;
			}

			return TRUE;
		}

		//	unexpected case.
		return FALSE;
	}



	/*

	set status of edp manager verbose log.

	return value:

	true -- set ok
	false -- failed to set

	*/
	BOOL SetVerboseLogStatus(BOOL bOn)
	{
		assert(m_pLogMgr);

		if (m_pLogMgr)
		{
			BOOL res = m_pLogMgr->SetVerlogStatus(bOn);
			
			return res;
		}

		//	unexpected case.
		return FALSE;
	}


	/*

	check running status of policy controller.

	return value:

	true -- running
	false -- not running

	*/
	BOOL IsPCRunning()
	{
		if (m_pPCMgr)
		{
			return m_pPCMgr->IsPCRunning();
		}

		//	unexpected case.
		return FALSE;
	}

	/*
	
	stop PC.

	parameter:

	pszPWD	--		password for Enterprise DLP
	
	return value:

	0	--	stop successfully

	else --		error code

	*/
	_CEResult_t StopPC(wchar_t* pszPWD)
	{
		assert(m_pPCMgr);

		if (m_pPCMgr)
		{
			return m_pPCMgr->StopPC(pszPWD);
		}

		//	unexpected case
		return CE_RESULT_NOT_SUPPORTED;
	}


	/*
	
	start PC.
	
	return value:

	true	--	start successfully

	false	 --		failed

	*/
	BOOL StartPC()
	{
		assert(m_pPCMgr);

		if (m_pPCMgr)
		{
			BOOL res = m_pPCMgr->StartPC();
		
			return res;
		}

		//	unexpected case
		return FALSE;
	}


	/*
	
	get installed components' name and installed dir pair.

	parameter:

	pair<wstring, wstring>	--	installed component name, such as L"Windows Desktop Enforcer", install dir of the component

	vector<>	--	all name and dir pairs of installed components installed on current machine.

	return result:

	true	--	get these information succeed
	false	--	get these information failed
	
	*/


	BOOL GetComponentsInstallDir( vector< pair<wstring, wstring> >  & vInstallDirs )
	{
		assert(m_pInstalledComptMgr);

		if (m_pInstalledComptMgr)
		{
			LPCOMPONENTINFO pInstallDirs = new COMPONENTINFO[MAX_ENFORCER_COUNT];
			if(pInstallDirs)
			{
				int nCount = MAX_ENFORCER_COUNT;
				BOOL bRet = m_pInstalledComptMgr->GetComponentsInstallDir(pInstallDirs, nCount);
				if(!bRet)
				{//Failed, maybe we need a larger buffer.
					delete []pInstallDirs;
					nCount += 5;
					pInstallDirs = new COMPONENTINFO[nCount];
					if(pInstallDirs)
					{
						bRet = m_pInstalledComptMgr->GetComponentsInstallDir(pInstallDirs, nCount);//try again.
					}	 
				}

				if(bRet)
				{
					//Copy the information to "input" parameter
					for(int i = 0; i < nCount; i++)
					{
						pair<wstring, wstring> info;
						info.first = pInstallDirs[i].szComponentName;
						info.second = pInstallDirs[i].szInfo;

						vInstallDirs.push_back(info);
					}
				}
				delete []pInstallDirs;
				pInstallDirs = NULL;

				return bRet;
			}
			
		}

		//	unexpected case
		return FALSE;
	}




	/*

	parameter:

	pair<wstring, wstring>	--	installed component name, such as L"Windows Desktop Enforcer", last updated (installed) date of the component.

	vector<>	--	all name and date pairs of installed components installed on current machine.

	return result:

	true	--	get these information succeed
	false	--	get these information failed

	*/
	BOOL GetComponentsLastUpdatedDate( vector< pair<wstring, wstring> >  & vLastUpdatedDates)
	{
		assert(m_pInstalledComptMgr);

		if (m_pInstalledComptMgr)
		{
			LPCOMPONENTINFO pLastUpdateDate = new COMPONENTINFO[MAX_ENFORCER_COUNT];
			if(pLastUpdateDate)
			{
				int nCount = MAX_ENFORCER_COUNT;
				BOOL bRet = m_pInstalledComptMgr->GetComponentsLastUpdatedDate(pLastUpdateDate, nCount);
				if(!bRet)
				{//Failed, maybe we need a larger buffer.
					delete []pLastUpdateDate;
					nCount += 5;
					pLastUpdateDate = new COMPONENTINFO[nCount];
					if(pLastUpdateDate)
					{
						bRet = m_pInstalledComptMgr->GetComponentsLastUpdatedDate(pLastUpdateDate, nCount);//try again.
					}	 
				}

				if(bRet)
				{
					//Copy the information to "input" parameter
					for(int i = 0; i < nCount; i++)
					{
						pair<wstring, wstring> info;
						info.first = pLastUpdateDate[i].szComponentName;
						info.second = pLastUpdateDate[i].szInfo;

						vLastUpdatedDates.push_back(info);
					}
				}
				delete []pLastUpdateDate;
				pLastUpdateDate = NULL;

				return bRet;
			}
			
		}

		//	unexpected case
		return FALSE;
	}


	/*

	parameter:

	pair<wstring, wstring>	--	installed component name, such as L"Windows Desktop Enforcer", version of the component.

	vector<>	--	all name and version pairs of installed components installed on current machine.

	return result:

	true	--	get these information succeed
	false	--	get these information failed

	*/
	BOOL GetComponentsVersion( vector< pair<wstring, wstring> >  & vVersion)
	{
		assert(m_pInstalledComptMgr);

		if (m_pInstalledComptMgr)
		{
			LPCOMPONENTINFO pVersions = new COMPONENTINFO[MAX_ENFORCER_COUNT];
			if(pVersions)
			{
				int nCount = MAX_ENFORCER_COUNT;
				BOOL bRet = m_pInstalledComptMgr->GetComponentsVersion(pVersions, nCount);
				if(!bRet)
				{//Failed, maybe we need a larger buffer.
					delete []pVersions;
					nCount += 5;
					pVersions = new COMPONENTINFO[nCount];
					if(pVersions)
					{
						bRet = m_pInstalledComptMgr->GetComponentsVersion(pVersions, nCount);//try again.
					}	 
				}

				if(bRet)
				{
					//Copy the information to "input" parameter
					for(int i = 0; i < nCount; i++)
					{
						pair<wstring, wstring> info;
						info.first = pVersions[i].szComponentName;
						info.second = pVersions[i].szInfo;

						vVersion.push_back(info);
					}
				}
				delete []pVersions;
				pVersions = NULL;

				return bRet;
			}
	
		}

		//	unexpected case
		return FALSE;
	}


	/*

	set and get notification display level

	there are three levels	--	display all, display blocking only, display none.

	return value:

	true	--	operation success
	false	--	unexpected error

	*/
	typedef enum
	{
		E_ALL,
		E_BLOCK_ONLY,
		E_NONE
	}NOTIFY_DIS_LEVEL;
	BOOL SetNotifyDisplayLever(NOTIFY_DIS_LEVEL eLevel)
	{
		assert(m_pNotifyMgr);

		if (m_pNotifyMgr)
		{
			return m_pNotifyMgr->SetNotifyDisplayLever( (CNotifyMgr::NOTIFY_DIS_LEVEL)eLevel );
		}

		//	unexpected case
		return FALSE;
	}

	BOOL GetNotifyDisplayLever(NOTIFY_DIS_LEVEL& eLevel)
	{
		assert(m_pNotifyMgr);

		if (m_pNotifyMgr)
		{
			return m_pNotifyMgr->GetNotifyDisplayLever( (CNotifyMgr::NOTIFY_DIS_LEVEL&)eLevel );
		}

		//	unexpected case
		return FALSE;
	}


	/*

	set and get notification display duration

	there are four levels	--	user close, 15 seconds, 30 seconds and 45 seconds.

	return value:

	true	--	operation success
	false	--	unexpected error

	*/
	typedef enum
	{
		E_USER_CLOSE,
		E_15_SECONDS,
		E_30_SECONDS,
		E_45_SECONDS
	}NOTIFY_DIS_DURATION;
	BOOL SetNotifyDuration(NOTIFY_DIS_DURATION eDuration)
	{
		assert(m_pNotifyMgr);

		if (m_pNotifyMgr)
		{
			return m_pNotifyMgr->SetNotifyDisplayDuration( (CNotifyMgr::NOTIFY_DIS_DURATION)eDuration );
		}

		//	unexpected case
		return FALSE;
	}

	BOOL GetNotifyDuration(NOTIFY_DIS_DURATION& eDuration)
	{
		assert(m_pNotifyMgr);

		if (m_pNotifyMgr)
		{
			return m_pNotifyMgr->GetNotifyDisplayDuration( (CNotifyMgr::NOTIFY_DIS_DURATION&)eDuration );
		}

		//	unexpected case
		return FALSE;
	}

	
	/*

	add notify history.

	history max number is controlled by \\Software\\Nextlabs\\EDP Manager\\MaxNotifyCache in registry, 
	default value is 300 is no registry.

	all notification is added into notifymgr, notifymgr manage notification history.

	*/
	typedef struct  
	{
		ULONG ulSize;
		WCHAR methodName [64];
		WCHAR params [7][256];
	}NOTIFY_INFO;
	BOOL AddNotifyToHistory(NOTIFY_INFO& notify)
	{
		assert(m_pNotifyMgr);

		if (m_pNotifyMgr)
		{
			return m_pNotifyMgr->AddNotify( (CNotifyMgr::NOTIFY_INFO&)notify );
		}

		//	unexpected case
		return FALSE;
	}


	/*

	get notify history.

	*/	
	
	typedef std::vector< NotifyInfo > NotificationVector;
	BOOL GetNotifyHistory(NotificationVector& notifyArray)
	{
		assert(m_pNotifyMgr);

		if (m_pNotifyMgr)
		{
			int nCount = m_pNotifyMgr->GetNotifyCount();

			NotifyInfo* pNotifies = new NotifyInfo[nCount];
			if(pNotifies)
			{
				BOOL bRet = m_pNotifyMgr->GetNotifyHistory(pNotifies, nCount);

				for(int i = 0; i < nCount; i++)
				{
					NotifyInfo info;
					memset(&info, 0, sizeof(NotifyInfo));

					wcsncpy_s(info.action, sizeof(info.action)/sizeof(wchar_t), pNotifies[i].action, _TRUNCATE);
					wcsncpy_s(info.file, sizeof(info.file)/sizeof(wchar_t), pNotifies[i].file, _TRUNCATE);
					wcsncpy_s(info.message, sizeof(info.message)/sizeof(wchar_t), pNotifies[i].message, _TRUNCATE);
					wcsncpy_s(info.time, sizeof(info.time)/sizeof(wchar_t), pNotifies[i].time, _TRUNCATE);
					info.enforcement = pNotifies[i].enforcement;
					
					notifyArray.push_back(info);
				}

				delete []pNotifies;
				pNotifies = NULL;

				return bRet;
			}
			
		}

		//	unexpected case
		return FALSE;
	}

	int GetNotificationCount() const
	{
		int nCount = 0;
		if (m_pNotifyMgr)
		{
			nCount = m_pNotifyMgr->GetNotifyCount();
		}
		return nCount;
	}

	/*
	
	register modaless dialog handle for keyboard input
	
	*/
	void RegDlgHandleForKB(HWND* pWnds, int nCount)
	{
		if (!m_pDlgMgr)
		{
			return;
		}
	
		return m_pDlgMgr->RegDlgHandleForKB(pWnds, nCount);
	}

	/*
	
	get dialog handles which need be special processed in winMain's message loop
	
	*/

	BOOL GetRegDlgHandleForKB(HWND* pHwnds, DWORD& dwCount)
	{
		if (!m_pDlgMgr)
		{
			return FALSE;
		}

		return m_pDlgMgr->GetDlgHandleForKB(pHwnds, dwCount);
	}


private:
	//	only init once
	BOOL m_bInited;

	//	handle to dll who contain interfaces
	HMODULE m_hLib;
	

	//	interfaces
	CLogMgr* m_pLogMgr;
	CPCStatusMgr* m_pPCMgr;
	CInstalledComptMgr* m_pInstalledComptMgr;
	CNotifyMgr* m_pNotifyMgr;
	CDlgMgr* m_pDlgMgr;


	//	exported function of the dll to get each interface
	typedef BOOL (*GetLogMgrType)(CLogMgr** ppILogMgr);
	typedef BOOL (*GetPCStatusMgrType)(CPCStatusMgr** ppIPCStatusMgr);
	typedef BOOL (*GetInstalledComptMgrType)(CInstalledComptMgr** ppInstalledComptMgr);
	typedef BOOL (*GetNotifyMgrType)(CNotifyMgr** ppNotifyMgr);
	typedef BOOL (*GetDlgMgrType)(CDlgMgr** ppDlgMgr);

public:
	/*

	show no permission modal dialog.

	*/
	BOOL ShowNoPermission_StartPC()
	{
		m_pDlgMgr->ShowModalDlg(CDlgMgr::E_NO_PERMISSION_START_PC);

		return TRUE;
	}

	BOOL ShowNoPermission_SetVerboseLog()
	{
		m_pDlgMgr->ShowModalDlg(CDlgMgr::E_NO_PERMISSION_SET_LOG);

		return TRUE;
	}

	//Release INLEDPManager, so that user needs to type in the user name and password to elevate the privilege again.
	void ResetUACInstance()
	{
		assert(m_pPCMgr);

		if (m_pPCMgr)
		{
			m_pPCMgr->ResetUAC();
		}
	}
};
