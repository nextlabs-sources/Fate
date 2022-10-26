#include <OAIdl.h>
#include <string>

using namespace std;

typedef struct MailParam
{
	wstring strSubject;
	wstring strBody;
	wstring strTo;
	wstring strCC;
	int		nMailType;
}MAILPARAM;

class CMailUtility
{
public:
	static HRESULT AutoWrap(int autoType, VARIANT *pvResult, IDispatch *pDisp, 
		LPOLESTR ptName, int cArgs...) 
	{
		// Begin variable-argument list...
		va_list marker;
		va_start(marker, cArgs);

		if(!pDisp) {
			return S_FALSE;
		}

		// Variables used...
		DISPPARAMS dp = { NULL, NULL, 0, 0 };
		DISPID dispidNamed = DISPID_PROPERTYPUT;
		DISPID dispID;
		HRESULT hr;
		
		char szName[200];

		// Convert down to ANSI
		WideCharToMultiByte(CP_ACP, 0, ptName, -1, szName, 256, NULL, NULL);

		// Get DISPID for name passed...
		hr = pDisp->GetIDsOfNames(IID_NULL, &ptName, 1, LOCALE_USER_DEFAULT, 
			&dispID);
		if(FAILED(hr)) {

			return hr;
		}

		// Allocate memory for arguments...
		VARIANT *pArgs = new VARIANT[cArgs+1];

		// Extract arguments...
		for(int i=0; i<cArgs; i++) {
			pArgs[i] = va_arg(marker, VARIANT);
		}

		// Build DISPPARAMS
		dp.cArgs = cArgs;
		dp.rgvarg = pArgs;

		// Handle special-case for property-puts!
		if(autoType & DISPATCH_PROPERTYPUT) {
			dp.cNamedArgs = 1;
			dp.rgdispidNamedArgs = &dispidNamed;
		}

		// Make the call!
		hr = pDisp->Invoke(dispID, IID_NULL, LOCALE_SYSTEM_DEFAULT, autoType, 
			&dp, pvResult, NULL, NULL);
		if(FAILED(hr)) {
			return hr;
		}
		// End variable-argument section...
		va_end(marker);

		delete [] pArgs;

		return hr;

	}



	static bool SendMail(wstring strSubject, wstring strBody, wstring strTo, wstring strCC, int nMailType)//nMailType, 0: HTML, 1: plaintext
	{
		HRESULT hr = S_OK;
		IUnknown* pUnknown=NULL;
		IDispatch* pOutlook = NULL;

		CLSID clsid;
		wchar_t* pwstrSid=L"Outlook.Application";
		::CLSIDFromProgID(pwstrSid,&clsid);
		LPOLESTR strFunc = L"CreateItem";
		hr = CoCreateInstance(clsid,NULL,CLSCTX_LOCAL_SERVER,IID_IUnknown,reinterpret_cast<void**>(&pUnknown));
		if(SUCCEEDED(hr))
		{
			hr = pUnknown->QueryInterface( IID_IDispatch,(void**)&pOutlook);
			pUnknown->Release();
			VARIANTARG vaResult;
			VariantInit( &vaResult );

			VARIANT var;
			var.vt = VT_I4;
			var.lVal = 0;

			AutoWrap(DISPATCH_METHOD,&vaResult,pOutlook,L"CreateItem",1,var);
			IDispatch* pItem = vaResult.pdispVal;

			var.lVal = 2;
			AutoWrap(DISPATCH_PROPERTYPUT,&vaResult,pItem,L"BodyFormat",1,var);

			VARIANT param;
			param.vt = VT_BSTR;
			param.bstrVal = SysAllocString(strSubject.c_str());
			AutoWrap(DISPATCH_PROPERTYPUT,&vaResult,pItem,L"Subject",1,param);
			::SysFreeString(param.bstrVal);

			param.bstrVal = SysAllocString(strBody.c_str());
			if(nMailType == 0)
				AutoWrap(DISPATCH_PROPERTYPUT,&vaResult,pItem,L"HTMLBody",1,param);
			else
				AutoWrap(DISPATCH_PROPERTYPUT,&vaResult,pItem,L"Body",1,param);
			::SysFreeString(param.bstrVal);

			param.bstrVal = SysAllocString(strTo.c_str());
			AutoWrap(DISPATCH_PROPERTYPUT,&vaResult,pItem,L"To",1,param);
			::SysFreeString(param.bstrVal);

			param.bstrVal = SysAllocString(strCC.c_str());
			AutoWrap(DISPATCH_PROPERTYPUT,&vaResult,pItem,L"CC",1,param);
			::SysFreeString(param.bstrVal);

			hr = AutoWrap(0,&vaResult,pItem,L"Send",0);	

			VariantClear(&vaResult);
		}
		return SUCCEEDED(hr)?true:false;
	}

	

	static wstring GetRecipients(IDispatch* pItem)
	{
		if(!pItem)
			return L"";

		VARIANT result;
		VariantInit(&result);

		wstring strRet;
		
		if(SUCCEEDED(AutoWrap(DISPATCH_PROPERTYGET, &result, pItem, L"Recipients", 0)) && result.vt == 9 && result.pdispVal)
		{
			IDispatch* pDisp = result.pdispVal;
			VariantInit(&result);
			if(SUCCEEDED(AutoWrap(DISPATCH_PROPERTYGET, &result, pDisp, L"Count", 0)) && result.intVal > 0)
			{
				int nCount = result.intVal;
				for (int i = 0; i < nCount; i++)
				{
					VARIANT index;
					VariantInit(&index);
					index.vt = 3;
					index.intVal = i + 1;
					if(SUCCEEDED(AutoWrap(DISPATCH_METHOD, &result, pDisp, L"Item", 1, index)) && result.vt == 9 && result.pdispVal)
					{
						IDispatch* pRecipient = result.pdispVal;
						VARIANT tempResult;
						VariantInit(&tempResult);
						if (SUCCEEDED(AutoWrap(DISPATCH_PROPERTYGET, &tempResult, pRecipient, L"Name", 0)) && tempResult.bstrVal)
						{
							strRet.append(tempResult.bstrVal);
							strRet.append(L";");
						}
						VariantClear(&tempResult);
						pRecipient->Release();
					}
					VariantClear(&index);
				}
			}
			pDisp->Release();
			

		}

		return strRet;

	}

	static wstring GetSubject(IDispatch* pItem)
	{
		if(!pItem)
			return L"";

		VARIANT result;
		VariantInit(&result);

		wstring strRet;
		if(SUCCEEDED(AutoWrap(DISPATCH_PROPERTYGET, &result, pItem, L"Subject", 0)))
		{
			strRet = result.bstrVal;
			SysFreeString(result.bstrVal);
		}

		return strRet;

	}
};

DWORD WINAPI SendMailThread(LPVOID pParam)
{
	if(!pParam)
		return 0;
	
	Sleep(2000);

	CoInitialize(NULL);
	MAILPARAM* p = (MAILPARAM*)pParam;
	CMailUtility::SendMail(p->strSubject, p->strBody, p->strTo, p->strCC, p->nMailType);
	
	delete p;
	CoUninitialize();

	return 0;
}