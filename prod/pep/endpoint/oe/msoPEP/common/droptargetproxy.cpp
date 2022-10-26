#include "stdafx.h"
#include "dropTargetProxy.h"

#include "AttachmentFileMgr.h"
#include "log.h"

namespace nextlabsoe
{
    DropTargetProxy::DropTargetProxy(HWND hwnd, IDropTarget *pTarget/*, CBaseEventProviderContext *pContext,  DNDWinClassAction winClassAction*/) : referenceCount_(0)
    {
        hWnd_ = hwnd;
        ptarget_ = pTarget;
        /*pContext_ = pContext;*/
        ptarget_->AddRef();
        /*winClassAction_ = winClassAction;*/
    }

    DropTargetProxy::~DropTargetProxy(void)
    {
        ptarget_->Release();
    }

    // IUnknown
    HRESULT DropTargetProxy::QueryInterface(REFIID iid, void **ppvObject) 
    {
        if (iid == IID_IDropTarget || iid == IID_IUnknown)
        {
            *ppvObject = static_cast<IDropTarget*>(this);
        }
        else
        {
            *ppvObject = 0;
            return E_NOINTERFACE;
        }

        AddRef();
        return S_OK;
    }

    ULONG DropTargetProxy::AddRef(void)
    {
        return ++referenceCount_; 
    }

    ULONG DropTargetProxy::Release(void) 
    {
        ULONG tmp = --referenceCount_;
        if (!tmp) 
        {
            delete this;
        }
        return tmp;
    }

    // IDropTarget
    HRESULT DropTargetProxy::DragEnter(IDataObject * pDataObject, DWORD grfKeyState,
        POINTL pt, DWORD * pdwEffect)
    {
        return ptarget_->DragEnter(pDataObject, grfKeyState, pt, pdwEffect);
    }

    HRESULT DropTargetProxy::DragLeave(void) 
    {
        return ptarget_->DragLeave();
    }

    HRESULT DropTargetProxy::DragOver(DWORD grfKeyState, POINTL pt, DWORD * pdwEffect)
    {
        return ptarget_->DragOver(grfKeyState, pt, pdwEffect);
    }

    HRESULT DropTargetProxy::Drop(IDataObject *pDataObject, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
    {NLONLY_DEBUG
        unsigned long ulCurThreadID = GetCurrentThreadId();
        NLPRINT_DEBUGVIEWLOG(L"Current thread ID:[%d]\n", ulCurThreadID);

        CAttachmentFileMgr& theAttachmentFileMgrIns = CAttachmentFileMgr::GetInstance();
        theAttachmentFileMgrIns.AddSourceFilesInDropedCache(ulCurThreadID, pDataObject);

        HRESULT hr = ptarget_->Drop(pDataObject, grfKeyState, pt, pdwEffect);
        
        // Check cache
        // In Drop function it will invoke CopyFileW.
        // We hook this function and get source file path from cache
        // After Drop function success return, this cache must be empty
        size_t stCacheFileCount = theAttachmentFileMgrIns.GetDropSrcFilesCount(ulCurThreadID);
        if (0 != stCacheFileCount)
        {
            NLPRINT_DEBUGVIEWLOG(L"!!!Attention: the file cache is not empty after Drop function return.");
        }

        NLPRINT_DEBUGVIEWLOG(L"Current thread ID:[%d]\n", GetCurrentThreadId());
        return hr;
    }
}