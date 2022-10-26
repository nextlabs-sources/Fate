#ifndef DROP_TARGET_PROXY
#define DROP_TARGET_PROXY

#include <oleidl.h>
#include "boost/noncopyable.hpp"

namespace nextlabsoe
{
class DropTargetProxy : boost::noncopyable, public IDropTarget
{
public:
    DropTargetProxy(HWND hwnd, IDropTarget *pTarget);
    ~DropTargetProxy(void);

    // IUnknown
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void **ppvObject);
    virtual ULONG STDMETHODCALLTYPE AddRef(void);
    virtual ULONG STDMETHODCALLTYPE Release(void);

    // IDropTarget
    virtual HRESULT STDMETHODCALLTYPE DragEnter(IDataObject *pDataObject, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
    virtual HRESULT STDMETHODCALLTYPE DragLeave(void);
    virtual HRESULT STDMETHODCALLTYPE DragOver(DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
    virtual HRESULT STDMETHODCALLTYPE Drop(IDataObject *pDataObject, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);

public:
    // DropTargetProxy
    HWND		hWnd_;
    IDropTarget *ptarget_;

private:
    ULONG referenceCount_;	// Reference count
};
}

#endif