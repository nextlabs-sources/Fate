#ifndef DROP_TARGET_PROXY
#define DROP_TARGET_PROXY

#include "baseeventprovidercontext.h"
#include "oledragndrop.h"

#include <oleidl.h>

namespace nextlabs
{
class CBaseEventProviderContext;
class DropTargetProxy : boost::noncopyable, public IDropTarget
{
public:
	DropTargetProxy(HWND hwnd, IDropTarget *pTarget, CBaseEventProviderContext *pContext,  DNDWinClassAction winClassAction);
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

	// DropTargetProxy
	HWND		hWnd_;
    IDropTarget *ptarget_;
private:
	
	CBaseEventProviderContext *pContext_;
	ULONG referenceCount_;	// Reference count
	DNDWinClassAction winClassAction_;
};

}

#endif