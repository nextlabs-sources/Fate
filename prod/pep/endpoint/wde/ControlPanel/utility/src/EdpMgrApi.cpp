#include "stdafx.h"
#include "LogMgrImp.h"
#include "PCStatusMgrImp.h"
#include "InstalledComptMgrImp.h"
#include "NotifyMgrImp.h"
#include "DlgMgrImp.h"

CLogMgrImp logMgr;
CPCStatusMgrImp pcStatusMgr;
CInstalledComptMgrImp installedCompMgr;
CNotifyMgrImp notifyMgr;
CDlgMgrImp dlgMgr;

BOOL GetLogMgr(CLogMgr** ppILogMgr)
{
	if (!ppILogMgr)
	{
		return FALSE;
	}

	*ppILogMgr = &logMgr;

	if ( !(*ppILogMgr) )
	{
		return FALSE;
	}

	return TRUE;
}

BOOL GetPCStatusMgr(CPCStatusMgr** ppIPCStatusMgr)
{
	if (!ppIPCStatusMgr)
	{
		return FALSE;
	}

	*ppIPCStatusMgr = &pcStatusMgr;

	if ( !(*ppIPCStatusMgr) )
	{
		return FALSE;
	}

	return TRUE;
}

BOOL GetInstalledComptMgr(CInstalledComptMgr** ppInstalledComptMgr)
{
	if (!ppInstalledComptMgr)
	{
		return FALSE;
	}

	*ppInstalledComptMgr = &installedCompMgr;

	if ( !(*ppInstalledComptMgr) )
	{
		return FALSE;
	}

	return TRUE;
}

BOOL GetNotifyMgr(CNotifyMgr** ppNotifyMgr)
{
	if (!ppNotifyMgr)
	{
		return FALSE;
	}

	*ppNotifyMgr = &notifyMgr;

	if ( !(*ppNotifyMgr) )
	{
		return FALSE;
	}

	return TRUE;
}

BOOL GetDlgMgr(CDlgMgr** ppDlgMgr)
{
	if (!ppDlgMgr)
	{
		return FALSE;
	}

	*ppDlgMgr = &dlgMgr;

	if ( !(*ppDlgMgr) )
	{
		return FALSE;
	}

	return TRUE;
}