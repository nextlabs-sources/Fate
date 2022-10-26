#include "dropTargetProxy.h"

#include "contentstorage.h"

#pragma warning(push)
#pragma warning(disable:6011)
#include <boost/algorithm/string.hpp>
#include <boost/scoped_ptr.hpp>
#pragma warning(pop)

namespace nextlabs
{
    DropTargetProxy::DropTargetProxy(HWND hwnd, IDropTarget *pTarget, CBaseEventProviderContext *pContext,  DNDWinClassAction winClassAction) : referenceCount_(0)
    {
        hWnd_ = hwnd;
        ptarget_ = pTarget;
        pContext_ = pContext;
        ptarget_->AddRef();
        winClassAction_ = winClassAction;
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

    HRESULT DropTargetProxy::DragLeave(void) {
        return ptarget_->DragLeave();
    }

    HRESULT DropTargetProxy::DragOver(DWORD grfKeyState, POINTL pt, DWORD * pdwEffect)
    {
        return ptarget_->DragOver(grfKeyState, pt, pdwEffect);
    }

    HRESULT DropTargetProxy::Drop(IDataObject *pDataObject, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
    {
        std::list<std::wstring> dropedFiles;
        OutputDebugString(L"enter DropTargetProxy::Drop functioin");
        if (pDataObject == NULL)
        {
            return ptarget_->Drop(pDataObject, grfKeyState, pt, pdwEffect);
        }

        BOOL rt = pContext_->finishDrop(pDataObject, winClassAction_);
        if (FALSE == rt)
        {
            ptarget_->DragLeave();
            return E_UNEXPECTED;
        }
        return ptarget_->Drop(pDataObject, grfKeyState, pt, pdwEffect);
    }
}