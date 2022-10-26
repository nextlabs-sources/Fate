

#ifndef _OUTLOOK_OBJ_H_
#define _OUTLOOK_OBJ_H_
#include "baseObj.h"

#include <map>
using namespace std;


class CSinkObjMgr
{
public:
	CSinkObjMgr(){};
	~CSinkObjMgr(){};
public:
	map<CComPtr<IDispatch>,int> m_mapSinkExpObj;

	void UnSinkExpObj();
	
};

class COutlookObj : public CBaseObj
{
public:
    COutlookObj();
    virtual ~COutlookObj();

	HRESULT OnConnection(LPDISPATCH Application, int ConnectMode, LPDISPATCH AddInInst, SAFEARRAY * * custom);
	HRESULT OnDisconnection(int RemoveMode, SAFEARRAY * * custom);
	HRESULT OnItemSend(CComPtr<IDispatch> Response, VARIANT_BOOL* Cancel ) ;
	CComPtr<Outlook::_Application>	get_App(){return m_spApp.p;}
	CComPtr<Outlook::_Explorer>	get_Explorer(){return m_spExp.p;}
	HRESULT SinkAppEvent( CComPtr<IDispatch> lpAppDisp ) ;
	BOOL IsMeetingRespondChangeTo0() {return m_bMeetingRespondChangedTo0;}  
	void SetMeetingRespondChangeTo0(BOOL b) {m_bMeetingRespondChangedTo0 = b;} 
	void SetAutoCloseMeetingSaveTipWindow(BOOL bAutoClose);
	BOOL NeedAutoCloseMeetingSaveTipWindow();
	void WaitDelayInitFinish();

public:
	BOOL InitAttachmentFileManager();
	void DeleteOETempFolder();
	static DWORD WINAPI DelayInitProc(LPVOID lpParameter);

private:
	CComQIPtr<Outlook::_Application, &Outlook::IID__Application>        m_spApp;    //application
    CComQIPtr<Outlook::_Explorer, &Outlook::IID__Explorer>              m_spExp;    //explorer
    CComQIPtr<Outlook::_Inspectors, &Outlook::IID__Inspectors>          m_spInsps;  //inspectors
    CComPtr<IUnknown>   m_spInspsSinkObj;
    DWORD       m_dwInspsSinkCookie;
    BOOL        m_bSendAttachmentDirectly;
	CComPtr<IUnknown>   m_sSinkObj;
	BOOL		m_bMeetingRespondChangedTo0;
	CRITICAL_SECTION m_csAutoCloseMSTW;
    int         m_intSaveWindowThreadID;
	
	
	DWORD       m_Explorerscook;
	CComPtr<Outlook::_Explorers> m_spExplores;

	HANDLE m_hEventDelayInit;
public:
	CSinkObjMgr m_SinkObjMgr;

};





class CExplorerSink: public CComObjectRoot,public ExplorerEvents_10
{
	BEGIN_COM_MAP(CExplorerSink)
		COM_INTERFACE_ENTRY(IDispatch)
		COM_INTERFACE_ENTRY(ExplorerEvents_10)
	END_COM_MAP()
public:
	virtual ~CExplorerSink()
	{

	}

	CExplorerSink()
	{

	}

	STDMETHODIMP GetTypeInfoCount(__RPC__out UINT *pctinfo) 
	{
		UNREFERENCED_PARAMETER(pctinfo);
		return E_NOTIMPL;
	}
	STDMETHODIMP GetTypeInfo(UINT iTInfo, LCID lcid, __RPC__deref_out_opt ITypeInfo **ppTInfo) 
	{
		UNREFERENCED_PARAMETER(iTInfo);
		UNREFERENCED_PARAMETER(lcid);
		UNREFERENCED_PARAMETER(ppTInfo);
		return E_NOTIMPL;
	}
	STDMETHODIMP GetIDsOfNames(__RPC__in REFIID riid, __RPC__in_ecount_full(cNames) LPOLESTR *rgszNames,  UINT cNames, LCID lcid, __RPC__out_ecount_full(cNames) DISPID *rgDispId)
	{
		UNREFERENCED_PARAMETER(riid);
		UNREFERENCED_PARAMETER(rgszNames);
		UNREFERENCED_PARAMETER(cNames);
		UNREFERENCED_PARAMETER(lcid);
		UNREFERENCED_PARAMETER(rgDispId);
		return E_NOTIMPL;
	}
	STDMETHODIMP Invoke(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS *pDispParams, VARIANT *pVarResult, EXCEPINFO *pExcepInfo, UINT *puArgErr);
private:
	void OnSeletionChange();
};



class CExplorersSink: public CComObjectRoot,public ExplorersEvents
{
	BEGIN_COM_MAP(CExplorersSink)
		COM_INTERFACE_ENTRY(IDispatch)
		COM_INTERFACE_ENTRY(ExplorersEvents)
	END_COM_MAP()
public:
	virtual ~CExplorersSink()
	{

	}

	CExplorersSink()
	{

	}

	STDMETHODIMP GetTypeInfoCount(__RPC__out UINT *pctinfo) 
	{
		UNREFERENCED_PARAMETER(pctinfo);
		return E_NOTIMPL;
	}
	STDMETHODIMP GetTypeInfo(UINT iTInfo, LCID lcid, __RPC__deref_out_opt ITypeInfo **ppTInfo) 
	{
		UNREFERENCED_PARAMETER(iTInfo);
		UNREFERENCED_PARAMETER(lcid);
		UNREFERENCED_PARAMETER(ppTInfo);
		return E_NOTIMPL;
	}
	STDMETHODIMP GetIDsOfNames(__RPC__in REFIID riid, __RPC__in_ecount_full(cNames) LPOLESTR *rgszNames,  UINT cNames, LCID lcid, __RPC__out_ecount_full(cNames) DISPID *rgDispId)
	{
		UNREFERENCED_PARAMETER(riid);
		UNREFERENCED_PARAMETER(rgszNames);
		UNREFERENCED_PARAMETER(cNames);
		UNREFERENCED_PARAMETER(lcid);
		UNREFERENCED_PARAMETER(rgDispId);
		return E_NOTIMPL;
	}
	STDMETHODIMP Invoke(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS *pDispParams, VARIANT *pVarResult, EXCEPINFO *pExcepInfo, UINT *puArgErr);
private:
	void OnNewExplorer(CComPtr<IDispatch> pExp);
	
};



#endif