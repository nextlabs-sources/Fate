//**************************************************
//Using CAutoMap to auto wrap the objcet fucntion  *
//Get the DispIID by Property /Method Name		   *
//**************************************************


#ifndef __NEXTLABS_CHELLEE_COM_AUTOMATION_H__
#define __NEXTLABS_CHELLEE_COM_AUTOMATION_H__



class CAutoMap 
{
public:
	CAutoMap() {} ;
	virtual ~CAutoMap() {} ;
public:
	//***************************************
	//cArgs...is inverted sequence			*
	//autoTyp:								*
	//DISPATCH_METHOD,						*
	//DISPATCH_PROPERTYGET,					*
	//DISPATCH_PROPERTYPUT,					*
	//DISPATCH_PROPERTYPUTREF				*
	//***************************************

	static HRESULT AutoWrap(int autoType, VARIANT *pvResult, IDispatch *pDisp, LPOLESTR ptName, int cArgs...) 
	{
		// Variables used...
		DISPPARAMS  dp          = { NULL, NULL, 0, 0 };
		DISPID      dispidNamed = DISPID_PROPERTYPUT;
		DISPID      dispID;
		HRESULT     hr;
		char        szName[300] = {0};

		// Convert down to ANSI
		WideCharToMultiByte(CP_ACP, 0, ptName, -1, szName, 256, NULL, NULL);

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
		try
		{
			hr = pDisp->Invoke(dispID, IID_NULL, LOCALE_SYSTEM_DEFAULT, (WORD)autoType, &dp, pvResult, NULL, NULL);
		}
		catch (...)
		{
			hr = E_FAIL;
		}

		delete [] pArgs;
		return hr;
	};

	//**************************************
	//Get the Disp IID by Name			*
	//**************************************
	static HRESULT GetDispIDByName( DISPID &_iDispID,IDispatch *pDisp, LPOLESTR ptName )
	{
		HRESULT hr = S_OK ;
		hr = pDisp->GetIDsOfNames(IID_NULL, &ptName, 1, LOCALE_USER_DEFAULT, &_iDispID);
		return hr;
	};
protected:
private:
} ;
#endif
