

#ifndef _OUTLOOK_EVENTDISP_H_
#define _OUTLOOK_EVENTDISP_H_

/************************************************************************/
/* CLASS CBaseAppEventDisp                                              */
/************************************************************************/
class ATL_NO_VTABLE CBaseAppEventDisp : 
    public CComObjectRoot,//Ex<CComSingleThreadModel>,
    public IDispatch
{
public:
    CBaseAppEventDisp();
    virtual ~CBaseAppEventDisp();

    BEGIN_COM_MAP(CBaseAppEventDisp)
        COM_INTERFACE_ENTRY(IDispatch)
        COM_INTERFACE_ENTRY_IID(__uuidof(Outlook::olApplicationEvents), IDispatch)
    END_COM_MAP()

    STDMETHOD(GetTypeInfoCount)(UINT* pctinfo){return	E_NOTIMPL;};
    STDMETHOD(GetTypeInfo)(UINT itinfo, LCID lcid, ITypeInfo** pptinfo){return	E_NOTIMPL;};
    STDMETHOD(GetIDsOfNames)(REFIID riid, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgdispid){return	E_NOTIMPL;};
    STDMETHOD(Invoke)(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pdispparams, VARIANT* pvarResult, EXCEPINFO* pexcepinfo, UINT* puArgErr);

protected:
    void OnItemSend(CComPtr<IDispatch> Item, VARIANT_BOOL* Cancel);
    void OnNewMail();
    void OnReminder(CComPtr<IDispatch> Item);
    void OnOptionsPagesAdd(PropertyPages* Pages);
    void OnStartup();
    void OnQuit();
};

/************************************************************************/
/* CLASS CBaseExplEventDisp                                             */
/************************************************************************/
class ATL_NO_VTABLE CBaseExplEventDisp : 
    public CComObjectRoot,//Ex<CComSingleThreadModel>,
    public IDispatch
{
public:
    CBaseExplEventDisp();
    virtual ~CBaseExplEventDisp();

    BEGIN_COM_MAP(CBaseExplEventDisp)
        COM_INTERFACE_ENTRY(IDispatch)
        COM_INTERFACE_ENTRY_IID(__uuidof(Outlook::ExplorerEvents), IDispatch)
    END_COM_MAP()

    STDMETHOD(GetTypeInfoCount)(UINT* pctinfo){return	E_NOTIMPL;};
    STDMETHOD(GetTypeInfo)(UINT itinfo, LCID lcid, ITypeInfo** pptinfo){return	E_NOTIMPL;};
    STDMETHOD(GetIDsOfNames)(REFIID riid, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgdispid){return	E_NOTIMPL;};
    STDMETHOD(Invoke)(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pdispparams, VARIANT* pvarResult, EXCEPINFO* pexcepinfo, UINT* puArgErr);

protected:
    void OnActivate();
    void OnFolderSwitch();
    void OnBeforeFolderSwitch(CComPtr<IDispatch> NewFolder, VARIANT_BOOL* Cancel);
    void OnViewSwitch();
    void OnBeforeViewSwitch(VARIANT NewView, VARIANT_BOOL* Cancel);
    void OnDeactivate();
    void OnSelectionChange();
    void OnClose();
};


/************************************************************************/
/* CLASS CBaseInspsEventDisp                                            */
/************************************************************************/
class ATL_NO_VTABLE CBaseInspsEventDisp : 
    public CComObjectRoot,//Ex<CComSingleThreadModel>,
    public IDispatch
{
public:
    CBaseInspsEventDisp();
    virtual ~CBaseInspsEventDisp();

    BEGIN_COM_MAP(CBaseInspsEventDisp)
        COM_INTERFACE_ENTRY(IDispatch)
        COM_INTERFACE_ENTRY_IID(__uuidof(Outlook::InspectorsEvents), IDispatch)
    END_COM_MAP()

    STDMETHOD(GetTypeInfoCount)(UINT* pctinfo){return	E_NOTIMPL;};
    STDMETHOD(GetTypeInfo)(UINT itinfo, LCID lcid, ITypeInfo** pptinfo){return	E_NOTIMPL;};
    STDMETHOD(GetIDsOfNames)(REFIID riid, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgdispid){return	E_NOTIMPL;};
    STDMETHOD(Invoke)(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pdispparams, VARIANT* pvarResult, EXCEPINFO* pexcepinfo, UINT* puArgErr);

protected:
    void OnNewInspector(CComQIPtr<Outlook::_Inspector> Inspector);
};


/************************************************************************/
/* CLASS CBaseInspEventDisp                                             */
/************************************************************************/
class ATL_NO_VTABLE CBaseInspEventDisp : 
    public CComObjectRoot,//Ex<CComSingleThreadModel>,
    public IDispatch
{
public:
    CBaseInspEventDisp();
    virtual ~CBaseInspEventDisp();

    BEGIN_COM_MAP(CBaseInspEventDisp)
        COM_INTERFACE_ENTRY(IDispatch)
        COM_INTERFACE_ENTRY_IID(__uuidof(Outlook::InspectorEvents), IDispatch)
    END_COM_MAP()

    STDMETHOD(GetTypeInfoCount)(UINT* pctinfo){return	E_NOTIMPL;};
    STDMETHOD(GetTypeInfo)(UINT itinfo, LCID lcid, ITypeInfo** pptinfo){return	E_NOTIMPL;};
    STDMETHOD(GetIDsOfNames)(REFIID riid, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgdispid){return	E_NOTIMPL;};
    STDMETHOD(Invoke)(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pdispparams, VARIANT* pvarResult, EXCEPINFO* pexcepinfo, UINT* puArgErr);

protected:
    void OnActivate();
    void OnDeactivate();
    void OnClose();

private:
    CComPtr<Outlook::_Inspector>    m_spInsp;
    DWORD                   m_dwInspCookie;
    DWORD                   m_dwItemCookie;
};


/************************************************************************/
/* CLASS CBaseItemEventDisp                                             */
/************************************************************************/
class ATL_NO_VTABLE CBaseItemEventDisp : 
    public CComObjectRoot,//Ex<CComSingleThreadModel>,
    public IDispatch
{
public:
    CBaseItemEventDisp();
    virtual ~CBaseItemEventDisp();

    BEGIN_COM_MAP(CBaseItemEventDisp)
        COM_INTERFACE_ENTRY(IDispatch)
        COM_INTERFACE_ENTRY_IID(__uuidof(Outlook::ItemEvents), IDispatch)
    END_COM_MAP()

    STDMETHOD(GetTypeInfoCount)(UINT* pctinfo){return	E_NOTIMPL;};
    STDMETHOD(GetTypeInfo)(UINT itinfo, LCID lcid, ITypeInfo** pptinfo){return	E_NOTIMPL;};
    STDMETHOD(GetIDsOfNames)(REFIID riid, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgdispid){return	E_NOTIMPL;};
    STDMETHOD(Invoke)(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pdispparams, VARIANT* pvarResult, EXCEPINFO* pexcepinfo, UINT* puArgErr);

protected:
    void OnOpen(VARIANT_BOOL* Cancel);
    void OnCustomAction(CComPtr<IDispatch> Action, CComPtr<IDispatch> Response, VARIANT_BOOL* Cancel);
    void OnCustomPropertyChange(BSTR Name);
    void OnForward(CComPtr<IDispatch> Forward, VARIANT_BOOL* Cancel);
    void OnClose(VARIANT_BOOL* Cancel);
    void OnPropertyChange(BSTR Name);
    void OnRead();
    void OnReply(CComPtr<IDispatch> Response, VARIANT_BOOL* Cancel);
    void OnReplyAll(CComPtr<IDispatch> Response, VARIANT_BOOL* Cancel);
    void OnSend(VARIANT_BOOL* Cancel);
    void OnWrite(VARIANT_BOOL* Cancel);
    void OnBeforeCheckNames(VARIANT_BOOL* Cancel);
    void OnAttachmentAdd(Attachment* Attachment);
    void OnAttachmentRead(Attachment* Attachment);
    void OnBeforeAttachmentSave(Attachment* Attachment, VARIANT_BOOL* Cancel);
};


#endif