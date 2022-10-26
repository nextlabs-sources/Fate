#include "StdAfx.h"
#include "NxtReceiver.h"
#include "nl_sysenc_lib.h"
#include <string>
#include "NxtMgr.h"
#include "celog.h"

extern CELog cbPepLog;

CNxtReceiver::CNxtReceiver(void): _cRef(1)
{
}

CNxtReceiver::~CNxtReceiver(void)
{
}


HRESULT CNxtReceiver::PreCopyItem(DWORD /*dwFlags*/, IShellItem * psiItem, IShellItem * psiDestinationFolder, PCWSTR pszNewName)
{
	cbPepLog.Log(CELOG_DEBUG,L"enter PreCopyItem\n");
	//OutputDebugStringW(L"enter PreCopyItem\n");
	if (psiItem && psiDestinationFolder && (pszNewName == NULL || wcslen(pszNewName) == 0))//it means the destination file name is same to source file name
	{
		if(!CNxtMgr::Instance()->PreHandleEncryption(psiItem, psiDestinationFolder))
			return E_FAIL;
	}
	return S_OK;
}


HRESULT CNxtReceiver::PostCopyItem(DWORD /*dwFlags*/, IShellItem *psiItem, IShellItem *psiDestinationFolder, LPCWSTR pszNewName, HRESULT /*hrCopy*/, IShellItem * /*psiNewlyCreated*/)
{
	cbPepLog.Log(CELOG_DEBUG,L"enter PostCopyItem\n");
	//OutputDebugStringW(L"enter PostCopyItem\n");
	if (psiItem && psiDestinationFolder && pszNewName && wcslen(pszNewName) > 0)
	{
		CNxtMgr::Instance()->PostHandleEncryption(psiItem, psiDestinationFolder, pszNewName);
	}

	return S_OK;
}


HRESULT CNxtReceiver::PreMoveItem(DWORD /*dwFlags*/, IShellItem * psiItem, IShellItem * psiDestinationFolder, PCWSTR pszNewName)
{
	if (psiItem && psiDestinationFolder && (pszNewName == NULL || wcslen(pszNewName) == 0))//it means the destination file name is same to source file name
	{
		if(!CNxtMgr::Instance()->PreHandleEncryption(psiItem, psiDestinationFolder))
			return E_FAIL;
	}

	return S_OK;
}

HRESULT CNxtReceiver::PostMoveItem(DWORD /*dwFlags*/, IShellItem * psiItem, IShellItem * psiDestinationFolder, PCWSTR pszNewName, HRESULT /*hrNewName*/, IShellItem * /*psiNewlyCreated*/)
{
	if (psiItem && psiDestinationFolder && pszNewName && wcslen(pszNewName) > 0)
	{
		CNxtMgr::Instance()->PostHandleEncryption(psiItem, psiDestinationFolder, pszNewName);
	}

	return S_OK;
}