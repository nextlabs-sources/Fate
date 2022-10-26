// approverobj.cpp : Implementation of Capproverobj

#include "stdafx.h"
#include "approverobj.h"
#include "olHandler.h"

extern HINSTANCE                      g_hInstance;
static YLIB::COMMON::smart_ptr<OlInspectorsHandler> g_spInspectorsHandler;

// Capproverobj
STDMETHODIMP Capproverobj::OnConnection(LPDISPATCH Application, ext_ConnectMode ConnectMode, LPDISPATCH AddInInst, SAFEARRAY * * custom)
{
	DP((L"Capproverobj::OnConnection\n"));
    CComQIPtr <Outlook::_olApplication>   spApp(Application);
    CComPtr<Outlook::_Explorer>         spExp;
    CComPtr<Outlook::_Inspectors>       spInspectors;
    CComPtr<IUnknown>                   spUnknown;
    HRESULT                             hr = S_OK;

    hr = spApp->ActiveExplorer(&spExp);
    if(FAILED(hr))
        return S_OK;
    hr = spApp->get_Inspectors(&spInspectors);
    if(FAILED(hr))
        return S_OK;

    g_spInspectorsHandler = YLIB::COMMON::smart_ptr<OlInspectorsHandler>(new OlInspectorsHandler);
	DP((L"OnConnection RegisterEventDispatch"));
    if(!g_spInspectorsHandler->RegisterEventDispatch(spInspectors))
		DP((L"olEnforcer::OnConnection fail to register inspectors event handler\n"));

    return S_OK;
}

STDMETHODIMP Capproverobj::OnDisconnection(ext_DisconnectMode RemoveMode, SAFEARRAY * * custom)
{
	DP((L"Capproverobj::OnDisconnection\n"));
    if(g_spInspectorsHandler.get())
        g_spInspectorsHandler->UnregisterEventDispatch();

    return S_OK;
}