#ifndef __ADAPTERCOMM_H__
#define __ADAPTERCOMM_H__

#include <vector>
#include <string>
#include <atlcomcli.h>
#include "obligation.h"
#include "attachment.h"
#include "adapter.h"

namespace AdapterCommon
{
	namespace{
	HRESULT AutoWrap(int autoType, VARIANT *pvResult, CComPtr<IDispatch> pDisp, LPOLESTR ptName, int cArgs...)
	{
		// Variables used...
		DISPPARAMS  dp          = { NULL, NULL, 0, 0 };
		DISPID      dispidNamed = DISPID_PROPERTYPUT;
		DISPID      dispID;
		HRESULT     hr;
		char        szName[MAX_PATH];

		// Convert down to ANSI
		WideCharToMultiByte(CP_ACP, 0, ptName, -1, szName, MAX_PATH, NULL, NULL);

		// Get DISPID for name passed...
		hr = pDisp->GetIDsOfNames(IID_NULL, &ptName, 1, LOCALE_USER_DEFAULT, &dispID);
		if(FAILED(hr))
		{
			return hr;
		}

		// Allocate memory for arguments...
		VARIANT *pArgs = new VARIANT[cArgs+1];

		// Extract arguments...
		// Begin variable-argument list...
		va_list marker;
		va_start(marker, cArgs);
		for(int i=0; i<cArgs; i++) {
			pArgs[i] = va_arg(marker, VARIANT);
		}
		// End variable-argument section...
		va_end(marker);

		// Build DISPPARAMS
		dp.cArgs = cArgs;
		dp.rgvarg = pArgs;

		// Handle special-case for property-puts!
		if(autoType & DISPATCH_PROPERTYPUT)
		{
			dp.cNamedArgs = 1;
			dp.rgdispidNamedArgs = &dispidNamed;
		}

		// Make the call!
		hr = pDisp->Invoke(dispID, IID_NULL, LOCALE_SYSTEM_DEFAULT, (WORD)autoType, &dp, pvResult, NULL, NULL);
		if(FAILED(hr))
		{
			return hr;
		}
		delete [] pArgs;
		return hr;
	};

	BOOL GetStringPropertyFromObject(CComPtr<IDispatch> pObject,WCHAR* wzPropName,std::wstring & strPropValue)
	{
		CComVariant varResult;
		HRESULT hr=AutoWrap(DISPATCH_PROPERTYGET,&varResult,pObject,wzPropName,0);
		if(FAILED(hr)||varResult.vt!=VT_BSTR||(varResult.vt==VT_BSTR&&varResult.bstrVal==NULL))
			return FALSE;
		strPropValue=varResult.bstrVal;
		return TRUE;
	};
	BOOL PutStringPropertyToObject(CComPtr<IDispatch> pObject,WCHAR* wzPropName,const WCHAR* wzPropValue)
	{
		CComVariant varResult;
		BSTR bstr=SysAllocString(wzPropValue);
		VARIANT vPropValue;
		vPropValue.vt=VT_BSTR;
		vPropValue.bstrVal=bstr;
		HRESULT hr=AutoWrap(DISPATCH_PROPERTYPUT,&varResult,pObject,wzPropName,1,vPropValue);
		if(FAILED(hr))
			return FALSE;
		return TRUE;
	};
	void StringReplace(std::wstring &strBig,const std::wstring &strSrc,const std::wstring &strDest)
	{
		std::wstring::size_type pos=0;
		std::wstring::size_type srclen=strSrc.size();
		std::wstring::size_type destlen=strDest.size();
		while((pos=strBig.find(strSrc,pos))!=std::wstring::npos)
		{
			strBig.replace(pos,srclen,strDest);
			pos+=destlen;
		}
	};
	BOOL StringReplaceTail(std::wstring &strBig,const std::wstring&strSrc,const std::wstring &strDest)
	{
		std::wstring::size_type pos=strBig.size()-strSrc.size();
		if(pos<0||pos>0x80000000)
			return FALSE;
		std::wstring::size_type srclen=strSrc.size();
		std::wstring strTail=strBig.substr(pos);
		if(strTail==strSrc)
		{
			strBig.replace(pos,srclen,strDest);
			return TRUE;
		}
		return FALSE;
	};
	}
	
}

#endif //__ADAPTERCOMM_H__


