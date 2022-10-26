// outlookImpl.cpp : Implementation of CoutlookImpl

#include "stdafx.h"
#pragma warning(push)
#pragma warning(disable: 4278)
#pragma warning(disable: 4337)
#include "outlookImpl.h"
#pragma warning(pop)


// CoutlookImpl


STDMETHODIMP CoutlookImpl::OnAddInsUpdate(SAFEARRAY * * custom)
{
	// TODO: Add your implementation code here

	return S_OK;
}

STDMETHODIMP CoutlookImpl::OnBeginShutdown(SAFEARRAY * * custom)
{
	// TODO: Add your implementation code here

	return S_OK;
}

STDMETHODIMP CoutlookImpl::OnStartupComplete(SAFEARRAY * * custom) 
{
	HRESULT hr = S_OK  ;
	return hr ;
}
STDMETHODIMP CoutlookImpl::OnConnection(LPDISPATCH Application, ext_ConnectMode ConnectMode, LPDISPATCH AddInInst, SAFEARRAY * * custom)
{
	HRESULT hr = S_OK ;
	return hr;
}
STDMETHODIMP CoutlookImpl::OnDisconnection(ext_DisconnectMode RemoveMode, SAFEARRAY * * custom)
{
	HRESULT hr = S_OK ;
	return hr;
}
STDMETHODIMP CoutlookImpl::Invoke(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pdispparams, VARIANT* pvarResult, EXCEPINFO* pexcepinfo, UINT* puArgErr) 
{
	HRESULT hr = S_OK;

	return	hr;
}