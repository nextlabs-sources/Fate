#ifndef _SFTPLIB_IMPLEMENT_H__
#define _SFTPLIB_IMPLEMENT_H__

#include "automap.hpp"
#include <atlcomcli.h>
/*
Implement for the Smart FTP
*/
class CSftplibImpl: public CAutoMap
{
public:
	CSftplibImpl(const IDispatch * i_AppDisp = NULL ) 
	{
		m_pDisp = const_cast<IDispatch *>(i_AppDisp) ; 
	} ;
	~CSftplibImpl(){} ;
public:
	HRESULT GetHost( IDispatch * i_AppDisp, std::wstring& strHost )
	{
		HRESULT hr = S_OK ;
		CComVariant vaRst;
		if(	 i_AppDisp    == NULL )
		{
		   i_AppDisp = m_pDisp ;
		}
		if(i_AppDisp)
		{
			hr = AutoWrap(DISPATCH_PROPERTYGET,&vaRst,i_AppDisp,L"Host",0);
			if( vaRst.vt == VT_BSTR )
			{
			   BSTR bstr = vaRst.bstrVal ;
			   strHost  = bstr ;
			}
		}
		return hr ;

	};
	HRESULT GetPort( IDispatch * i_AppDisp, LONG &lport ) 
	{
		HRESULT hr = S_OK ;
		CComVariant vaRst;
		if(	 i_AppDisp    == NULL )
		{
		   i_AppDisp = m_pDisp ;
		}
		if(i_AppDisp)
		{
			hr = AutoWrap(DISPATCH_PROPERTYGET,&vaRst,i_AppDisp,L"Port",0);
			if( SUCCEEDED( hr))
			{
			   lport  = vaRst.lVal ;
			}
		}
		return hr ;
	};
	HRESULT GetLastPath( IDispatch * i_AppDisp, std::wstring& strPath ) 
	{
		HRESULT hr = S_OK ;
		CComVariant vaRst;
		if(	 i_AppDisp    == NULL )
		{
		   i_AppDisp = m_pDisp ;
		}
		if(i_AppDisp)
		{
			std::wstring strHost ;
			GetHost( i_AppDisp,strHost ) ;
			hr = AutoWrap(DISPATCH_PROPERTYGET,&vaRst,i_AppDisp,L"LastPath",0);
			if( vaRst.vt == VT_BSTR )
			{
			   BSTR bstr = vaRst.bstrVal ;
			   std::wstring temp  =  bstr ;
			   strPath = L"FTP://"+strHost+ temp ;
			}
		}
		return hr ;
	};
	HRESULT GetUserName( IDispatch * i_AppDisp, std::wstring& strUsrName ) 
	{
		HRESULT hr = S_OK ;
		CComVariant vaRst;
		if(	 i_AppDisp    == NULL )
		{
			i_AppDisp = m_pDisp ;
		}
		if(i_AppDisp)
		{
			hr = AutoWrap(DISPATCH_PROPERTYGET,&vaRst,i_AppDisp,L"Username",0);
			if( vaRst.vt == VT_BSTR )
			{
				BSTR bstr = vaRst.bstrVal ;
				strUsrName  = bstr ;
			}
		}
		return hr ;
	};
private:
	IDispatch * m_pDisp ;
}	;

#endif