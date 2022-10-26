#include "StdAfx.h"
#include "OBEmail.h"

#include "../../approver/include/emailprocess.h"
#include "rdPasswd.h"

//#ifdef OUTLOOK_2007
//#import "../../import/2k7/mso.dll" rename_namespace("OutLook") rename("RGB","MSORGB") 
//#import "../../import/2k7/MSOUTL.OLB" rename_namespace("OutLook") rename("CopyFile","_CopyFile")
//#else
//#import "../../import/2k3/mso.dll" rename_namespace("OutLook") rename("RGB","MSORGB") 
//#import "../../import/2k3/MSOUTL.OLB" rename_namespace("OutLook") rename("CopyFile","_CopyFile")
//#endif
////using namespace Office;
//using namespace OutLook;


COBEmail::COBEmail(void)
{
	CoInitialize(NULL);
}

COBEmail::~COBEmail(void)
{
	CoUninitialize();
}

HRESULT AutoWrap(int autoType, VARIANT *pvResult, CComPtr<IDispatch> pDisp, LPOLESTR ptName, int cArgs...)
{
	// Variables used...
	DISPPARAMS  dp          = { NULL, NULL, 0, 0 };
	DISPID      dispidNamed = DISPID_PROPERTYPUT;
	DISPID      dispID;
	HRESULT     hr;
	char        szName[200];

	// Convert down to ANSI
	WideCharToMultiByte(CP_ACP, 0, ptName, -1, szName, 200, NULL, NULL);

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
}
bool COBEmail::SendEmail(BaseArgumentFlex & baseArguFlex)
{
	bool bRet=true;
	ApprovalEmail ae;

	int iPosBegin=0,iPosEnd=0;
	while(iPosEnd!=-1)
	{
		iPosEnd=(int)baseArguFlex.wstrApprovers.find(L";",iPosBegin,1);
		std::wstring wstrApprover=baseArguFlex.wstrApprovers.substr(iPosBegin,iPosEnd-iPosBegin);
		iPosBegin=iPosEnd+1;
		if(wstrApprover.length())
			ae.add_Approver(wstrApprover.c_str());
	}

	ae.put_RequesterName(baseArguFlex.wstrRequesterDisplayName.c_str());
	ae.put_RequesterAddress(baseArguFlex.wstrRequesterAddress.c_str());
	ae.put_RequesterSid(baseArguFlex.wstrCurSID.c_str());
	ae.put_Purpose(baseArguFlex.wstrPurpose.c_str());
	ae.put_FtpServer(baseArguFlex.wstrFtpDir.c_str());
	ae.put_FtpUser(baseArguFlex.wstrFtpUser.c_str());
	ae.put_FtpPasswd(baseArguFlex.wstrFtpPasswd.c_str());
	ae.put_OrigSubject(baseArguFlex.wstrSubject.c_str ());
	CPasswdGenerator passwdGen;
	std::wstring strPasswd=passwdGen.Generator();
	ae.put_EncryptPasswd(strPasswd.c_str());
	//ae.put_FtpPort(baseArguFlex.w
	iPosBegin=0,iPosEnd=0;
	while(iPosEnd!=-1)
	{
		iPosEnd=(int)baseArguFlex.wstrRecipients.find(L";",iPosBegin,1);
		std::wstring wstrRecipient=baseArguFlex.wstrRecipients.substr(iPosBegin,iPosEnd-iPosBegin);
		iPosBegin=iPosEnd+1;
		if(wstrRecipient.length())
			ae.add_Recipient(wstrRecipient.c_str());
	}
	
	ae.put_Subject(L"Approval request for email documents");
	for(size_t n=0;n<baseArguFlex.vFiles.size();n++)
	{
		ae.add_File(baseArguFlex.vFiles[n].c_str());
	}

	baseArguFlex.vFiles.clear();

	const std::wstring& strMail = ae.Compose();
	CLog::WriteLog(L"Mail content",strMail.c_str());

	const std::wstring& strSubject = ae.get_Subject();
	CLog::WriteLog(L"Mail subject",strSubject.c_str());

	const std::wstring& strTo = baseArguFlex.wstrApprovers;
	CLog::WriteLog(L"Approver(s) ",strTo.c_str());
	if(baseArguFlex.wstrApprovers.find(L",",0,1)!=std::wstring::npos||
		baseArguFlex.wstrApprovers.find(L":",0,1)!=std::wstring::npos||
		baseArguFlex.wstrApprovers.find(L"&",0,1)!=std::wstring::npos)
	{
		CLog::WriteLog(L"Approver string include specific character like ',',':' or '&'. The approver string",strTo.c_str());
		return false;
	}
#ifdef _DEBUG
	std::wstring strMsg = L"Mail will be send to ";
	strMsg += strTo;
	MessageBox(NULL,strMsg.c_str(),L"Debug Msg",MB_OK);
	CLog::WriteLog(L"Mail will be send to",strTo.c_str());
#endif

//	std::wstring strCC = g_BaseArgument.wstrCurAddress ;
//	strCC += L";";
//	strCC += g_BaseArgument.wstrArchivalID;
//#ifdef _DEBUG
//	strMsg = L"Mail will be CC to ";
//	strMsg += strCC;
//	MessageBox(NULL,strMsg.c_str(),L"Debug Msg",MB_OK);
//	CLog::WriteLog(L"Mail CC to",strCC.c_str());
//#endif



	CComBSTR varSubject(strSubject.c_str());

	CComBSTR varHTMLBody(strMail.c_str());

	CComBSTR varTo(strTo.c_str());

//	CComBSTR varCC(strCC.c_str());

//////////////////////////////////////////////////////////////////////////
	#if 1
	{
		HRESULT hr = S_OK;
		CComPtr<IUnknown> pUnknown=NULL;
		CComPtr<IDispatch> pOutlook = NULL;

		CLSID clsid;
		wchar_t* pwstrSid=L"Outlook.Application";
		(void)::CLSIDFromProgID(pwstrSid,&clsid);
//		LPOLESTR strFunc = L"CreateItem";
		hr = CoCreateInstance(clsid,NULL,CLSCTX_LOCAL_SERVER,IID_IUnknown,reinterpret_cast<void**>(&pUnknown));
		if(SUCCEEDED(hr))
		{
			hr = pUnknown->QueryInterface( IID_IDispatch,(void**)&pOutlook);

			VARIANT var;
			var.vt = VT_I4;
			var.lVal = 0;

			CComVariant vaResult;
			AutoWrap(DISPATCH_METHOD,&vaResult,pOutlook,L"CreateItem",1,var);
			CComPtr<IDispatch> pItem = vaResult.pdispVal;

			var.lVal = 2;
            vaResult.Clear();
			AutoWrap(DISPATCH_PROPERTYPUT,&vaResult,pItem,L"BodyFormat",1,var);

			VARIANT param;
			param.vt = VT_BSTR;
			param.bstrVal = SysAllocString(strSubject.c_str());
            vaResult.Clear();
			AutoWrap(DISPATCH_PROPERTYPUT,&vaResult,pItem,L"Subject",1,param);
			::SysFreeString(param.bstrVal);

			param.bstrVal = SysAllocString(strMail.c_str());
            vaResult.Clear();
			AutoWrap(DISPATCH_PROPERTYPUT,&vaResult,pItem,L"HTMLBody",1,param);
			::SysFreeString(param.bstrVal);

			param.bstrVal = SysAllocString(strTo.c_str());
            vaResult.Clear();
			AutoWrap(DISPATCH_PROPERTYPUT,&vaResult,pItem,L"To",1,param);
			::SysFreeString(param.bstrVal);

			/*param.bstrVal = SysAllocString(strCC.c_str());
			AutoWrap(DISPATCH_PROPERTYPUT,&vaResult,pItem,L"CC",1,param);
			::SysFreeString(param.bstrVal);*/

            vaResult.Clear();
			hr = AutoWrap(0,&vaResult,pItem,L"Send",0);	
		}
		else
		{
			CLog::WriteLog(L"Fail to create COM instance",L"");
		}
		return SUCCEEDED(hr)?true:false;
	}
#endif 
	return bRet;
}
bool COBEmail::SendEmail()
{

	ApprovalEmail   ae;
	for(size_t i=0;i<g_BaseArgument.approverVector.size();i++)
		ae.add_Approver(g_BaseArgument.approverVector[i].first.c_str());

	ae.put_RequestType(g_BaseArgument.wstrApprType.c_str());

	ae.put_RequesterName(g_BaseArgument.wstrCurUserName.c_str());

	ae.put_RequesterAddress(g_BaseArgument.wstrCurAddress.c_str());

	ae.put_ArchiverAddress(g_BaseArgument.wstrArchivalID.c_str());

	ae.put_RequesterSid(g_BaseArgument.wstrCurSID.c_str());

	ae.put_ApprovalDirectory(g_BaseArgument.wstrApprDir.c_str());

	ae.put_Customer(g_BaseArgument.wstrCustomer.c_str());

	ae.put_CustomerKey(g_BaseArgument.wstrCusetomerKey.c_str());

	if(!g_BaseArgument.wstrCusetomerKey.empty())
	{
		
		CLog::WriteLog(L"The passed encrypt parameter is asymmetric, the value is: \t",g_BaseArgument.wstrCusetomerKey.c_str());
	}
	if(!g_BaseArgument.wstrEncryptionPwd.empty())
	{
		CLog::WriteLog(L"The passed encrypt parameter is symmetric, the value is: \t",g_BaseArgument.wstrEncryptionPwd.c_str());
	}
	//OutputDebugStringW(L"\r\n");

	ae.put_EncryptPasswd(g_BaseArgument.wstrEncryptionPwd.c_str());

	ae.put_Purpose(g_BaseArgument.wstrPurpose.c_str());

	ae.add_Recipient(g_BaseArgument.wstrRecipientUser.c_str());


	for(size_t n=0;n<g_BaseArgument.vecFile.size();n++)
	{
		ae.add_File(g_BaseArgument.vecFile[n].first.c_str(),g_BaseArgument.vecFile[n].second.c_str());
	}

	// add by Tonny at 2008/9/11 for bug 6529
	// we should clear the g_BaseArgument's file vector
	g_BaseArgument.vecFile.clear();


	const std::wstring& strMail = ae.Compose();
	CLog::WriteLog(L"Mail content",strMail.c_str());

	const std::wstring& strSubject = ae.get_Subject();
	CLog::WriteLog(L"Mail subject",strSubject.c_str());

	const std::wstring& strTo = g_BaseArgument.wstrApprEMail;
#ifdef _DEBUG
	std::wstring strMsg = L"Mail will be send to ";
	strMsg += strTo;
	MessageBox(NULL,strMsg.c_str(),L"Debug Msg",MB_OK);
	CLog::WriteLog(L"Mail will be send to",strTo.c_str());
#endif

	std::wstring strCC = g_BaseArgument.wstrCurAddress ;
	strCC += L";";
	strCC += g_BaseArgument.wstrArchivalID;
#ifdef _DEBUG
	strMsg = L"Mail will be CC to ";
	strMsg += strCC;
	MessageBox(NULL,strMsg.c_str(),L"Debug Msg",MB_OK);
	CLog::WriteLog(L"Mail CC to",strCC.c_str());
#endif



	CComBSTR varSubject(strSubject.c_str());

	CComBSTR varHTMLBody(strMail.c_str());

	CComBSTR varTo(strTo.c_str());

	CComBSTR varCC(strCC.c_str());

//////////////////////////////////////////////////////////////////////////
#if 1
	{
		HRESULT hr = S_OK;
		CComPtr<IUnknown> pUnknown=NULL;
		CComPtr<IDispatch> pOutlook = NULL;

		CLSID clsid;
		wchar_t* pwstrSid=L"Outlook.Application";
		(void)::CLSIDFromProgID(pwstrSid,&clsid);
//		LPOLESTR strFunc = L"CreateItem";
		hr = CoCreateInstance(clsid,NULL,CLSCTX_LOCAL_SERVER,IID_IUnknown,reinterpret_cast<void**>(&pUnknown));
		if(SUCCEEDED(hr))
		{
			hr = pUnknown->QueryInterface( IID_IDispatch,(void**)&pOutlook);

			VARIANT var;
			var.vt = VT_I4;
			var.lVal = 0;

			CComVariant vaResult;
			AutoWrap(DISPATCH_METHOD,&vaResult,pOutlook,L"CreateItem",1,var);
			CComPtr<IDispatch> pItem = vaResult.pdispVal;

			var.lVal = 2;
            vaResult.Clear();
			AutoWrap(DISPATCH_PROPERTYPUT,&vaResult,pItem,L"BodyFormat",1,var);

			VARIANT param;
			param.vt = VT_BSTR;
			param.bstrVal = SysAllocString(strSubject.c_str());
            vaResult.Clear();
			AutoWrap(DISPATCH_PROPERTYPUT,&vaResult,pItem,L"Subject",1,param);
			::SysFreeString(param.bstrVal);

			param.bstrVal = SysAllocString(strMail.c_str());
            vaResult.Clear();
			AutoWrap(DISPATCH_PROPERTYPUT,&vaResult,pItem,L"HTMLBody",1,param);
			::SysFreeString(param.bstrVal);

			param.bstrVal = SysAllocString(strTo.c_str());
            vaResult.Clear();
			AutoWrap(DISPATCH_PROPERTYPUT,&vaResult,pItem,L"To",1,param);
			::SysFreeString(param.bstrVal);

			param.bstrVal = SysAllocString(strCC.c_str());
            vaResult.Clear();
			AutoWrap(DISPATCH_PROPERTYPUT,&vaResult,pItem,L"CC",1,param);
			::SysFreeString(param.bstrVal);

            vaResult.Clear();
			hr = AutoWrap(0,&vaResult,pItem,L"Send",0);	
		}
		return SUCCEEDED(hr)?true:false;
	}
#endif 

}
