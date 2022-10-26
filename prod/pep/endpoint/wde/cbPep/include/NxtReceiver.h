#pragma once

#include <shobjidl.h>

class CNxtReceiver: public IFileOperationProgressSink
{
public:
	CNxtReceiver(void);
	~CNxtReceiver(void);

	// IUnknown
	IFACEMETHODIMP QueryInterface(REFIID riid, void **ppv)
	{
		static const QITAB qit[] =
		{
			QITABENT(CNxtReceiver, IFileOperationProgressSink),
			{0},
		};
		return QISearch(this, qit, riid, ppv);
	}

	IFACEMETHODIMP_(ULONG) AddRef()
	{
		return InterlockedIncrement(&_cRef);
	}

	IFACEMETHODIMP_(ULONG) Release()
	{
		ULONG cRef = InterlockedDecrement(&_cRef);
		if (0 == cRef)
		{
			delete this;
		}
		return cRef;
	}

//implement all functions of IFileOperationProgressSink
	IFACEMETHODIMP StartOperations()
	{
		return S_OK;
	}
	IFACEMETHODIMP FinishOperations(HRESULT /*hrResult*/)
	{
		return S_OK;
	}
	IFACEMETHODIMP PreRenameItem(DWORD /*dwFlags*/, IShellItem * /*psiItem*/, PCWSTR /*pszNewName*/)
	{
		return S_OK;
	}
	IFACEMETHODIMP PostRenameItem(DWORD /*dwFlags*/, IShellItem * /*psiItem*/, PCWSTR /*pszNewName*/, HRESULT /*hrRename*/, IShellItem * /*psiNewlyCreated*/)
	{
		return S_OK;
	}
	IFACEMETHODIMP PreMoveItem(DWORD dwFlags, IShellItem * psiItem, IShellItem * psiDestinationFolder, PCWSTR pszNewName);

	IFACEMETHODIMP PostMoveItem(DWORD dwFlags, IShellItem * psiItem,
		IShellItem * psiDestinationFolder, PCWSTR pszNewName, HRESULT hrNewName, IShellItem * psiNewlyCreated);

	IFACEMETHODIMP PreCopyItem(DWORD dwFlags, IShellItem * psiItem,
		IShellItem * psiDestinationFolder, PCWSTR pszNewName);

	IFACEMETHODIMP PostCopyItem(DWORD dwFlags, IShellItem *psiItem,
		IShellItem *psiDestinationFolder, PCWSTR pwszNewName, HRESULT hrCopy,
		IShellItem *psiNewlyCreated);

	IFACEMETHODIMP PreDeleteItem(DWORD /*dwFlags*/, IShellItem * /*psiItem*/)
	{
		return S_OK;
	}
	IFACEMETHODIMP PostDeleteItem(DWORD /*dwFlags*/, IShellItem * /*psiItem*/, HRESULT /*hrDelete*/, IShellItem * /*psiNewlyCreated*/)
	{
		return S_OK;
	}
	IFACEMETHODIMP PreNewItem(DWORD /*dwFlags*/, IShellItem * /*psiDestinationFolder*/, PCWSTR /*pszNewName*/)
	{
		return S_OK;
	}
	IFACEMETHODIMP PostNewItem(DWORD /*dwFlags*/, IShellItem * /*psiDestinationFolder*/,
		PCWSTR /*pszNewName*/, PCWSTR /*pszTemplateName*/, DWORD /*dwFileAttributes*/, HRESULT /*hrNew*/, IShellItem * /*psiNewItem*/)
	{
		return S_OK;
	}
	IFACEMETHODIMP UpdateProgress(UINT /*iWorkTotal*/, UINT /*iWorkSoFar*/)
	{
		return S_OK;
	}
	IFACEMETHODIMP ResetTimer()
	{
		return S_OK;
	}
	IFACEMETHODIMP PauseTimer()
	{
		return S_OK;
	}
	IFACEMETHODIMP ResumeTimer()
	{
		return S_OK;
	}
	
protected:
	long   _cRef;
};
