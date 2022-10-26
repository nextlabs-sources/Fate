
#pragma once

#include "stdafx.h"


#define INITGUID
#define USES_IID_IMessage
#define USES_IID_IMAPITable
#define USES_IID_IAttachment

#include <initguid.h>

#include <Windows.h>
#include "outlookUtilities.h"
#include <algorithm> //add by jjm 
#include <string>
#include <map>
#include "timeout_list.hpp"
#include "../AD/WinAD.h"
#pragma warning(push)
#pragma warning(disable: 6334)
#include <boost/algorithm/string.hpp>
#pragma warning(pop)
#include "MailItemUtility.h"
#pragma comment(lib, "Wldap32.lib")
#pragma comment(lib, "Advapi32")
#pragma comment(lib, "mapi32")
#include ".\..\Common\Hook.h"
#pragma warning(push)
#pragma warning(disable: 4267)
#include "nlconfig.hpp"
#pragma warning(pop)

#include "Shlwapi.h"
#pragma comment(lib,"Shlwapi.lib")

#include "msopath.hpp"
#include "strsafe.h"
#include "../common/log.h"
#include "../common/NLType.h"

#define  PR_SMTP_ADDRESS 0x39FE001E

#ifndef PR_ATTACH_CONTENT_ID
#define PR_ATTACH_CONTENT_ID (PROP_TAG(PT_TSTRING,0x3712))
#endif

#ifndef PR_ATTACH_CONTENT_LOCATION
#define PR_ATTACH_CONTENT_LOCATION (PROP_TAG(PT_TSTRING,0x3713))
#endif


CTimeoutList g_cacheSMTPAddress(0xFFFFFFFF);
extern CComPtr<IMAPISession> pSession;

#ifdef XXXXXWSO2K7

extern Word::_wordApplication*		g_spWordApp;
extern Excel::_excelApplication*	g_spExcelApp;
extern PPT::_pptApplication*		g_spPPTApp;

#endif
extern std::wstring g_strOETempFolder;
#ifdef _DEBUG
void OLUtilities::PrintLastError(LPCWSTR str)
{
	DWORD lastErr = GetLastError();
	WCHAR errStr[1024];

	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_MAX_WIDTH_MASK,
		NULL, lastErr, 0, errStr, 1024, NULL);
	DP((L"%s failed, error code %lu, \"%s\"\n", str, lastErr, errStr));
}
#endif /* _DEBUG */

//////////////////////////////////////////////////////////////////////////

void OLUtilities::GetSenderAddr(CComPtr<IDispatch> spMailItem, wchar_t* wzSenderAddr, int maxSize)
{
	HRESULT hr = S_OK;
	BSTR    bstrName;
	BSTR    bstrType;

	if(0 == spMailItem) return;

	hr = MailItemUtility::get_SenderEmailType(spMailItem,&bstrType);
	if(SUCCEEDED(hr) && bstrType)
	{
		if(0 ==  _wcsicmp(bstrType, L"SMTP"))
		{
			hr = MailItemUtility::get_SenderEmailAddress(spMailItem,&bstrName);
			if(SUCCEEDED(hr))
			{
				wcsncpy_s(wzSenderAddr, maxSize, bstrName, _TRUNCATE);
				SysFreeString(bstrName);
			}
		}
		else if(0 == _wcsicmp(bstrType, L"EX"))
		{
			//PR_SENDER_ENTRYID
			CComPtr<IAddrBook>      lpAdrBook=0;
			CComPtr<IMAPISession>   lpMapiSession=0;
			CComPtr<IMessage>       lpMessage=0;
			ULONG           ulObjType = 0;
			CComPtr<IMailUser>      lpMailUser = 0;
			CComPtr<Outlook::_NameSpace>    lpOlSession;

			ULONG       ulCount = 0;
			SPropValue  *pValue;
			SPropTagArray spta; memset(&spta, 0, sizeof(SPropTagArray));

			hr = MailItemUtility::get_Session(spMailItem,&lpOlSession);
			if(S_OK == hr && lpOlSession)
			{
				hr = lpOlSession->get_MAPIOBJECT((IUnknown**)&lpMapiSession);
				if(S_OK==hr && lpMapiSession)
				{
					hr = lpMapiSession->OpenAddressBook(0,NULL,0,&lpAdrBook);
					if(S_OK==hr && lpAdrBook)
					{
						hr =MailItemUtility::get_MAPIOBJECT(spMailItem,(IUnknown**)&lpMessage);
						if(S_OK == hr && lpMessage)
						{
							spta.cValues = 1;
							spta.aulPropTag[0] = PR_SENDER_ENTRYID;
							hr = lpMessage->GetProps(&spta, MAPI_UNICODE, &ulCount, &pValue);
							if(S_OK==hr && pValue)
							{
								hr = lpAdrBook->OpenEntry(pValue->Value.bin.cb, (LPENTRYID)(pValue->Value.bin.lpb), NULL, 0, &ulObjType, (LPUNKNOWN*)&lpMailUser);
								if(S_OK==hr && lpMailUser)
								{
									pValue = 0;
									memset(&spta, 0, sizeof(SPropTagArray));
									ulCount = 0;
									spta.cValues = 1;
									spta.aulPropTag[0] = PR_SMTP_ADDRESS;
									hr = lpMailUser->GetProps(&spta, MAPI_UNICODE, &ulCount, &pValue);
									if(S_OK == hr && pValue)
									{
										size_t size = 0;
										mbstowcs_s(&size, wzSenderAddr, maxSize, pValue->Value.lpszA, _TRUNCATE);
									}
								}
							}
						}
					}
				}
			}
			else
			{
				hr =MailItemUtility::get_SenderName(spMailItem,&bstrName);
				if(SUCCEEDED(hr))
				{
					wcsncpy_s(wzSenderAddr, maxSize, bstrName, _TRUNCATE);
					SysFreeString(bstrName);
				}
			}
		}
		SysFreeString(bstrType);
	}
}

void OLUtilities::GetMailSubject(CComPtr<IDispatch> spMailItem, wchar_t* wzMailSubject, int maxSize)
{
	if(0 == spMailItem) return;

	BSTR    bstrName;
	HRESULT hr = S_OK;

	hr = MailItemUtility::get_Subject(spMailItem,&bstrName);
	if(SUCCEEDED(hr) && bstrName)
	{
		wcsncpy_s(wzMailSubject, maxSize, bstrName, _TRUNCATE);
		SysFreeString(bstrName);
	}
}

int  OLUtilities::GetMailRecipients(CComPtr<IDispatch> spMailItem)
{
	HRESULT hr;
	CComPtr<Outlook::Recipients> spRecipients;
	int                          nRecipientCount=0;

	hr = MailItemUtility::get_Recipients(spMailItem,&spRecipients);
	if(FAILED(hr)) return 0;

	hr = spRecipients->get_Count((long*)&nRecipientCount);
	if(FAILED(hr)) return 0;

	return nRecipientCount;
}

int  OLUtilities::SetMailRecipients(CComPtr<IDispatch> spMailItem, STRINGLIST& listRecipients)
{
	int				iIndex=0,iCount=0;
	int				nRecipientCount=0;
	STRINGLIST		strRecipient;
	HRESULT			hr;
	CComPtr<Outlook::Recipients> spRecipients;
	hr = MailItemUtility::get_Recipients(spMailItem,&spRecipients);
	if(FAILED(hr)) return 0;
	spRecipients->get_Count((long*)&nRecipientCount);
	for(iIndex=0;iIndex<nRecipientCount;iIndex++)
	{
		strRecipient.clear();

		VARIANT vi; vi.vt = VT_INT; vi.intVal = iIndex+1;
		CComPtr<Outlook::Recipient>    spRecipient = 0;
		CComPtr<Outlook::AddressEntry>  spAddrEntry = 0;

		hr = spRecipients->Item(vi, &spRecipient);
		if(FAILED(hr))
			continue;

		hr = spRecipient->get_AddressEntry(&spAddrEntry);
		if (S_OK==hr && spAddrEntry)
		{
			ExpandAddressEntry(spAddrEntry, strRecipient);
		}

		if(std::find(listRecipients.begin(),listRecipients.end(),strRecipient[0].c_str())==listRecipients.end())
			spRecipients->Remove(iIndex+1);

		iCount++;
	}
	//strRecipient.clear();
	//OLUtilities::GetMailRecipients(spMailItem,strRecipient);

	return iCount;

}

int  OLUtilities::GetOriginalRecipientCount(CComPtr<IDispatch> spMailItem)
{
	HRESULT                      hr;
	CComPtr<Outlook::Recipients> spRecipients;
	int nRecipientCount=0;

	hr = MailItemUtility::get_Recipients(spMailItem,&spRecipients);
	if(FAILED(hr)) return 0;

	spRecipients->get_Count((long*)&nRecipientCount);
	return nRecipientCount;
}
int  OLUtilities::ResetMailRecipients(CComPtr<IDispatch> spMailItem,const STRINGLIST& listRecipients)
{
	HRESULT                         hr  = S_OK;
	int                             i   = 0, nRecipientCount = 0 ;
	CComPtr<Outlook::Recipients>    spRecipients;

	hr = MailItemUtility::get_Recipients(spMailItem,&spRecipients);
	if(SUCCEEDED(hr))
	{
		spRecipients->get_Count((long*)&nRecipientCount);
		DP((L"Check count:[%d]\n",nRecipientCount));
		for(i=nRecipientCount-1; i>=0; i--)
		{
			hr = spRecipients->Remove(i+1);
			if(SUCCEEDED(hr))
				continue;
			return 0;
		}
		CComPtr<Outlook::Recipient>    spRecipient;
		for(i=0;i<static_cast<int>(listRecipients.size());i++)
		{
			CComBSTR addr(listRecipients[i].c_str());
			hr=spRecipients->Add(addr,&spRecipient);
			if(spRecipient)
			{
				VARIANT_BOOL varResult;
				hr=spRecipient->Resolve(&varResult);
			}
		}
		VARIANT_BOOL varResolved;
		spRecipients->get_Count((long*)&nRecipientCount);
		for(i=nRecipientCount-1; i>=0; i--)
		{
			VARIANT vi; vi.vt = VT_INT; vi.intVal = i+1;
			hr=spRecipients->Item(vi,&spRecipient);
			if(SUCCEEDED(hr)&&spRecipient)
			{
				hr=spRecipient->get_Resolved(&varResolved);
				if(SUCCEEDED(hr)&&varResolved==VARIANT_FALSE)
					hr = spRecipients->Remove(i+1);
			}
		}
	}
	

	return static_cast<int>(listRecipients.size());

}
int  OLUtilities::GetMailRecipients(CComPtr<IDispatch> spMailItem, STRINGLIST& listRecipients)
{
	HRESULT                      hr;
	CComPtr<Outlook::Recipients> spRecipients;
	int nRecipientCount=0, nValidCount=0;
	int i = 0;

	hr = MailItemUtility::get_Recipients(spMailItem,&spRecipients);
	if(FAILED(hr)){
		loge(L"[GetMailRecipients]get_Recipients failed");
		return 0;
	}

	wchar_t strlog[MAX_PATH] = {0};
	DWORD dwStart = GetTickCount();

	spRecipients->get_Count((long*)&nRecipientCount);
	for(i=0; i<nRecipientCount; i++)
	{
		VARIANT vi; vi.vt = VT_INT; vi.intVal = i+1;
		CComPtr<Outlook::Recipient>    spRecipient = 0;
		CComPtr<Outlook::AddressEntry>  spAddrEntry = 0;

		hr = spRecipients->Item(vi, &spRecipient);
		if(FAILED(hr)){
			loge(L"[GetMailRecipients]spRecipients->Item failed, continue.");
			continue;
		}

		hr = spRecipient->get_AddressEntry(&spAddrEntry);
		if (S_OK==hr && spAddrEntry)
		{
			ExpandAddressEntry(spAddrEntry, listRecipients);
		}

		nValidCount++;
	}

	StringCchPrintf(strlog,MAX_PATH,L"GetMailRecipients Spend time is [%d] @@",GetTickCount() - dwStart);
	CEventLog::WriteEventLog(strlog,EVENTLOG_INFORMATION_TYPE,EVENTLOG_INFO_ID);
	/*
	Modified by chellee on 26/11/2008: 8:51
	For the Bug:7563
	return nValidCount;
	*/
	return (INT)listRecipients.size() ;
}

void doIndent(int debugIndentLevel)
{
	for (int i = 0; i < debugIndentLevel; i++)
	{
		DP((L"\t"));
	}
}


BOOL OLUtilities::FindRecip_FromRecipientList( CComPtr<Outlook::AddressEntry> spAddrEntry, std::wstring szRecipient,INT debugIndentLevel)
{
	BOOL bRet = FALSE ; 
	/*
	TRUE: The recipient is in the mail address....
	FALSE:Currrent recipient or disttribute needn't removed
	*/
	HRESULT                             hr = S_OK;
	CComPtr<Outlook::AddressEntries>    spMemberAddrEntries = 0;

	BSTR bstrName;
	hr = spAddrEntry->get_Name(&bstrName);
	doIndent(debugIndentLevel); DP((L"--------------------\n"));
	doIndent(debugIndentLevel); DP((L"Name=%s\n", bstrName));

	OlDisplayType olDispType = olUser;
	hr = spAddrEntry->get_DisplayType(&olDispType);
	doIndent(debugIndentLevel); DP((L"DisplayType=%d\n", olDispType));

	// Only expand private distribution list.  Do not expand global
	// distribution list.
	if(olPrivateDistList==olDispType)
	{
		hr = spAddrEntry->get_Members(&spMemberAddrEntries);

		if(S_OK==hr && spMemberAddrEntries)
		{
			long nMemberCount;

			hr = spMemberAddrEntries->get_Count(&nMemberCount);
			doIndent(debugIndentLevel);
			DP((L"nMemberCount=%ld\n", nMemberCount));

			for(long i=0; i<nMemberCount; i++)
			{
				CComPtr<Outlook::AddressEntry> spMemberAddrEntry;
				VARIANT vi; vi.vt = VT_INT; vi.intVal = i+1;

				hr = spMemberAddrEntries->Item(vi, &spMemberAddrEntry);

				if(S_OK==hr && spMemberAddrEntry)
				{
					// Recurse.
					bRet = FindRecip_FromRecipientList(spMemberAddrEntry, szRecipient,
						debugIndentLevel + 1);
					if( bRet == TRUE )
					{
						break ;
					}
				}
			}
		}
	}
	else
	{
		wchar_t* wzSmtpAddress = new wchar_t[MAX_MAILADDR_LENGTH];
		CComPtr<Outlook::_Application>	spApp = NULL;

		wzSmtpAddress[0] = L'\0';

		wstring strSMTPAddress;
		if(g_cacheSMTPAddress.FindItem(bstrName, strSMTPAddress) )
		{
			wcsncpy_s(wzSmtpAddress, MAX_MAILADDR_LENGTH, strSMTPAddress.c_str(), _TRUNCATE);
			DP((L"FindRecip_FromRecipientList::Get the SMTP address from cache: %s", wzSmtpAddress));
		}
		else
		{
			BSTR bstrAddress;
			BSTR bstrType;
			hr=spAddrEntry->get_Type(&bstrType);
			if(SUCCEEDED(hr))
			{
				if(_wcsnicmp(bstrType,L"SMTP",4)==0)
				{
					hr = spAddrEntry->get_Address(&bstrAddress);
					if(SUCCEEDED(hr)&&bstrAddress)
					{
						if(wcsrchr(bstrAddress, L'@'))
							wcsncpy_s(wzSmtpAddress,MAX_MAILADDR_LENGTH,bstrAddress, _TRUNCATE);
						::SysFreeString(bstrAddress);

					}
				}
				else if (_wcsnicmp(bstrType, L"EX", 2) == 0)
				{
					DP((L"testx begin get address Through EX type\n"));
					CComPtr<Outlook::_ExchangeUser> pIExUser = NULL;
					hr = spAddrEntry->GetExchangeUser(&pIExUser);
					if (SUCCEEDED(hr) && pIExUser)
					{
						BSTR bstrPriSmtpAddr = NULL;
						pIExUser->get_PrimarySmtpAddress(&bstrPriSmtpAddr);
						if (bstrPriSmtpAddr)
						{
							DP((L"testx bstrAddress smtp:%s\n", bstrPriSmtpAddr));
							wcsncpy_s(wzSmtpAddress, MAX_MAILADDR_LENGTH, bstrPriSmtpAddr, _TRUNCATE);
							::SysFreeString(bstrPriSmtpAddr);
							bstrPriSmtpAddr = NULL;
						}
					}
				}
				::SysFreeString(bstrType);
			}
			if(wzSmtpAddress[0]==0)
			{
				hr = S_OK;
				if(pSession == NULL)
				{
					hr = MAPILogonEx(NULL,NULL,NULL, MAPI_ALLOW_OTHERS | MAPI_EXTENDED,&pSession);
				}
				if(S_OK == hr && pSession)
				{
					DP((L"OLUtilities::ExpandAddressEntry  Succeeed to get MAPI sesstion\n"));
					GetSmtpAddressByDisplayName2(pSession, bstrName, wzSmtpAddress, MAX_MAILADDR_LENGTH);
					g_cacheSMTPAddress.AddItem(bstrName, wzSmtpAddress);
				//	pSession->Logoff(NULL,NULL,NULL);
				}
				else
				{
					DP((L"OLUtilities::ExpandAddressEntry  Fail to get MAPI sesstion\n"));
				}
				
			}
		}

		if(0 == wzSmtpAddress[0])
		{
			BSTR bstrAddress;
			BSTR bstrType;
			hr=spAddrEntry->get_Type(&bstrType);
			if(SUCCEEDED(hr))
			{
				if(_wcsnicmp(bstrType,L"SMTP",4)==0)
				{
					hr = spAddrEntry->get_Address(&bstrAddress);
					if(SUCCEEDED(hr))
					{
						if(wcsrchr(bstrAddress, L'@'))
							wcsncpy_s(wzSmtpAddress,MAX_MAILADDR_LENGTH,bstrAddress, _TRUNCATE);
						::SysFreeString(bstrAddress);
					}
				}
				::SysFreeString(bstrType);
			}

			if(0==wzSmtpAddress[0])
			{
				hr = spAddrEntry->get_Application(&spApp);
				if(SUCCEEDED(hr) && spApp)
				{
					GetSmtpAddressFromOfficeContact(spApp, bstrName, wzSmtpAddress, MAX_MAILADDR_LENGTH);				
				}
			}
		}
		doIndent(debugIndentLevel);

		if(0 != wzSmtpAddress[0])
		{
			wchar_t	*pAddr=wzSmtpAddress;
			if(wzSmtpAddress[0]==L'\'')
			{
				pAddr++;
				int len=(int)wcslen(pAddr);
				if(len>0&&pAddr[len-1]==L'\'')
					pAddr[len-1]=0;
			}
			DP((L"Compare and remove the mail recipient: Current:[%s];Mapper[%s]\n", pAddr,szRecipient.c_str()));
			
			if (_wcsicmp(pAddr,szRecipient.c_str()) == 0)
			{
				bRet =	TRUE ;
			}
			
			DP((L"wzSmtpAddress=%s\n", pAddr));
		}
		else
		{
			if(bstrName[0]==L'\'')
			{
				USES_CONVERSION;
				LPWSTR pAddr = W2OLE(bstrName);
				pAddr++;
				int len=(int)wcslen(pAddr);
				if(len>0&&pAddr[len-1]==L'\'')
					pAddr[len-1]=0;
				SysReAllocString(&bstrName,pAddr);
			}
			DP((L"Compare and remove the mail recipient: bstrName:[%s];Mapper[%s]\n", bstrName,szRecipient.c_str()));
			

			if (_wcsicmp(bstrName,szRecipient.c_str()) == 0)
			{
				bRet =	TRUE ;
			}
			DP((L"wzSmtpAddress=%s\n", bstrName));
		}

		delete []wzSmtpAddress;
	}

	SysFreeString(bstrName);
	return bRet ;
}
BOOL OLUtilities::RemoveMailRecipient(CComPtr<IDispatch> spMailItem,
									  std::wstring szRecipient)
{
	HRESULT                         hr  = S_OK;
	BOOL                            bRet= FALSE, bNeedRemoved = FALSE ;
	int                             i   = 0, nRecipientCount = 0 ;
	CComPtr<Outlook::Recipients>    spRecipients;

	hr = MailItemUtility::get_Recipients(spMailItem,&spRecipients);
	if(SUCCEEDED(hr))
	{
LOOPREMOVE:
		spRecipients->get_Count((long*)&nRecipientCount);
		DP((L"Check count:[%d]\n",nRecipientCount));
		for(i=0; i<nRecipientCount; i++)
		{
			VARIANT vi; vi.vt = VT_INT; vi.intVal = i+1;
			CComPtr<Outlook::Recipient>    spRecipient = 0;
			CComPtr<Outlook::AddressEntry>  spAddrEntry = 0;

			hr = spRecipients->Item(vi, &spRecipient);
			DP((L"Get Item:Error Code [%d]",hr));
			if(FAILED(hr))
				continue;
		   DP((L"Loop number:[%d]\n",i+1));
			hr = spRecipient->get_AddressEntry(&spAddrEntry);
			if (S_OK==hr && spAddrEntry)
			{
				bNeedRemoved =  FindRecip_FromRecipientList(spAddrEntry, szRecipient,0);
			}

			if( bNeedRemoved == TRUE )
			{

				//i = 0 ;
				DP((L"Need removed\n"));
				hr = spRecipients->Remove(i+1);
				goto LOOPREMOVE ;
			}
			//nValidCount++;
		}

	}
	if( hr== S_OK )
	{
		bRet = TRUE ;
	}
	return bRet;
}
//--------------------------------------------------------------------------
void OLUtilities::ExpandAddressEntry(CComPtr<Outlook::AddressEntry> spAddrEntry,
									 STRINGLIST& listRecipients)
{
	ExpandAddressEntry(spAddrEntry, listRecipients, 0);
}

//
// Note: Since this is a recursive function, don't use big local variables so
// that we won't overflow the stack.  Allocate all your big variables on the
// heap instead.
//

void OLUtilities::ExpandAddressEntry(CComPtr<Outlook::AddressEntry> spAddrEntry,
									 STRINGLIST& listRecipients,
									 int debugIndentLevel)
{
	HRESULT                             hr = S_OK;
	CComPtr<Outlook::AddressEntries>    spMemberAddrEntries = 0;
	wchar_t strlog[MAX_PATH] = {0};
	DWORD dwStart = GetTickCount();
 
	BSTR bstrName=NULL;
	hr = spAddrEntry->get_Name(&bstrName);
    if (FAILED(hr) || bstrName == NULL) return;

	OlDisplayType olDispType = olUser;
	hr = spAddrEntry->get_DisplayType(&olDispType);

	// Only expand private distribution list.  Do not expand global
	// distribution list.
	if((olPrivateDistList==olDispType) /*||(olDistList==olDispType)*/)
	{
		hr = spAddrEntry->get_Members(&spMemberAddrEntries);

		if(S_OK==hr && spMemberAddrEntries)
		{
			long nMemberCount;
			hr = spMemberAddrEntries->get_Count(&nMemberCount);
			doIndent(debugIndentLevel);
			for(long i=0; i<nMemberCount; i++)
			{
				CComPtr<Outlook::AddressEntry> spMemberAddrEntry;
				VARIANT vi; vi.vt = VT_INT; vi.intVal = i+1;
				hr = spMemberAddrEntries->Item(vi, &spMemberAddrEntry);
				if(S_OK==hr && spMemberAddrEntry)
				{
					// Recurse.
					ExpandAddressEntry(spMemberAddrEntry, listRecipients,
						debugIndentLevel + 1);
				}
			}
		}
	}
	else
	{
		wchar_t* wzSmtpAddress = new wchar_t[MAX_MAILADDR_LENGTH];
        wzSmtpAddress[0] = L'\0';
        wstring strSMTPAddress;
        if (g_cacheSMTPAddress.FindItem(bstrName, strSMTPAddress))
        {
            wcsncpy_s(wzSmtpAddress, MAX_MAILADDR_LENGTH, strSMTPAddress.c_str(), _TRUNCATE);
        }
        else
        {
            BSTR bstrAddress=NULL;
            BSTR bstrType=NULL;
            hr = spAddrEntry->get_Type(&bstrType);
            if (SUCCEEDED(hr) && bstrType != NULL)
            {
                if (_wcsnicmp(bstrType, L"SMTP", 4) == 0)
                {
                    hr = spAddrEntry->get_Address(&bstrAddress);
                    if (SUCCEEDED(hr) && bstrAddress)
                    {
                        if (wcsrchr(bstrAddress, L'@'))
                            wcsncpy_s(wzSmtpAddress, MAX_MAILADDR_LENGTH, bstrAddress, _TRUNCATE);
                        ::SysFreeString(bstrAddress);

                    }
                }
                else if (_wcsnicmp(bstrType, L"EX", 2) == 0)
                {
                    CComPtr<Outlook::_ExchangeUser> pIExUser = NULL;
                    hr = spAddrEntry->GetExchangeUser(&pIExUser);
                    if (SUCCEEDED(hr) && pIExUser)
                    {
                        BSTR bstrPriSmtpAddr = NULL;
                        pIExUser->get_PrimarySmtpAddress(&bstrPriSmtpAddr);
                        if (bstrPriSmtpAddr)
                        {
                            wcsncpy_s(wzSmtpAddress, MAX_MAILADDR_LENGTH, bstrPriSmtpAddr, _TRUNCATE);
                            ::SysFreeString(bstrPriSmtpAddr);
                            bstrPriSmtpAddr = NULL;
                        }
                    }
                }
                ::SysFreeString(bstrType);
            }

            if (wzSmtpAddress[0] == 0)
            {
                hr = S_OK;
                if (pSession == NULL)
                {
                    hr = MAPILogonEx(NULL, NULL, NULL, MAPI_EXTENDED | MAPI_ALLOW_OTHERS, &pSession);
                }
                if (S_OK == hr && pSession)
                {
                    GetSmtpAddressByDisplayName2(pSession, bstrName, wzSmtpAddress, MAX_MAILADDR_LENGTH);
                    g_cacheSMTPAddress.AddItem(bstrName, wzSmtpAddress);
                }
            }
        }

		if(0 == wzSmtpAddress[0])
		{
            CComPtr<Outlook::_Application>	spApp = NULL;
			hr = spAddrEntry->get_Application(&spApp);
			if(SUCCEEDED(hr) && spApp)
			{
				GetSmtpAddressFromOfficeContact(spApp, bstrName, wzSmtpAddress, MAX_MAILADDR_LENGTH);
			}
		}
		if(0 ==wzSmtpAddress[0]||wcsrchr(wzSmtpAddress, L'@')==0)
		{
			if(IfAppUserInDomain())
			{
				CWinAD theWinAD;
				std::wstring strKeyWord = L"(displayName=";
				BSTR bstrName_l=NULL;
				hr=spAddrEntry->get_Name(&bstrName_l);
				if(SUCCEEDED(hr)&&bstrName_l)
				{
					strKeyWord += bstrName_l;	//g_BaseArgument.wstrCurUserName;
					::SysFreeString(bstrName_l);
					strKeyWord += L")";
					std::wstring strEmailAddr,strSid;
                    if (theWinAD.SearchUserInfo(strEmailAddr, strSid, strKeyWord.c_str()))
					{
						wcsncpy_s(wzSmtpAddress,255,strEmailAddr.c_str(), _TRUNCATE);
					}
				}
			}
		}
		doIndent(debugIndentLevel);

		if(0 != wzSmtpAddress[0])
		{
			wchar_t	*pAddr=wzSmtpAddress;
			if(wzSmtpAddress[0]==L'\'')
			{
				pAddr++;
				int len=(int)wcslen(pAddr);
				if(len>0&&pAddr[len-1]==L'\'')
					pAddr[len-1]=0;
			}
			listRecipients.push_back(pAddr);
		}
		else
		{
			if(bstrName[0]==L'\'')
			{
				USES_CONVERSION;
				LPWSTR pAddr = W2OLE(bstrName);
				pAddr++;
				int len=(int)wcslen(pAddr);
				if(len>0&&pAddr[len-1]==L'\'')
					pAddr[len-1]=0;
				SysReAllocString(&bstrName,pAddr);
			}
			listRecipients.push_back(bstrName);
		}
        delete[]wzSmtpAddress;
	}
	SysFreeString(bstrName);

    DWORD dwTime = GetTickCount() - dwStart;
    StringCchPrintf(strlog, MAX_PATH, L"------------> ExpandAddressEntry spend time is [%d].\n", dwTime);
    if (dwTime > 1000)
    {
        NLPRINT_DEBUGVIEWLOG(L"%s", strlog);
    }
	CEventLog::WriteEventLog(strlog,EVENTLOG_INFORMATION_TYPE,EVENTLOG_INFO_ID);
}

HRESULT OLUtilities::ExResolveName( IAddrBook* pAddrBook, LPWSTR lpszName, LPADRLIST *lpAdrList )
{
	// NOTE: Callers of this function MUST release lpAdrList when done
	// with it using MAPIFreeBuffer.
	HRESULT   hRes = S_OK;
	LPADRLIST pAdrList = NULL;

	DP((L"Display Name is %s\n", lpszName));

	// Allocate memory for new SRowSet structure.
	hRes = MAPIAllocateBuffer(CbNewSRowSet(1),(LPVOID*) &pAdrList);

	// If memory allocation fails, quit.
	if ( FAILED ( hRes ) )
	{
		DP((L"First allocate failed!\n"));
		return hRes;
	}

	// Zero out allocated memory.
	ZeroMemory ( pAdrList, CbNewSRowSet(1));

	// Allocate memory for SPropValue structure that indicates what
	// recipient properties will be set. NUM_RECIP_PROPS == 5.
	hRes = MAPIAllocateBuffer( 1 * sizeof(SPropValue), (LPVOID*) &(pAdrList->aEntries[0].rgPropVals));

	// If memory allocation fails, quit.
	if ( FAILED ( hRes ) )
	{
		DP((L"Second allocate failed!\n"));
		MAPIFreeBuffer(pAdrList);
		return hRes;
	}

	// Zero out allocated memory.
	ZeroMemory ( pAdrList->aEntries[0].rgPropVals, sizeof(SPropValue) );

	// How many recipients.
	pAdrList->cEntries = 1;

	// How many properties per recipient.
	pAdrList->aEntries[0].cValues    = 1;

	// Set the SPropValue members == the desired values.
	pAdrList->aEntries[0].rgPropVals[0].ulPropTag = PR_DISPLAY_NAME;
	pAdrList->aEntries[0].rgPropVals[0].Value.lpszW =  lpszName;


	// ResolveName is kind enough to redimension the ADRLIST that we
	// pass to it and give us back a fully qualified ADRLIST structure
	// that contains all the recipient information the address book
	// decided to give us back.
#pragma warning(push)
#pragma warning(disable: 6309)
	hRes = pAddrBook->ResolveName( NULL, MAPI_UNICODE, NULL, pAdrList );
#pragma warning(pop)
	if ( SUCCEEDED(hRes) )
	{
		*lpAdrList = pAdrList;
		DP((L"Resolve name OK!\n"));
	}
	else
	{
		MAPIFreeBuffer(pAdrList->aEntries[0].rgPropVals);
		MAPIFreeBuffer(pAdrList);
		DP((L"Fail to resolve name!\n"));
	}

	return hRes;
}

void OLUtilities::GetSmtpAddressByDisplayName2(CComPtr<IMAPISession> lpSession, LPCWSTR wzDispName, LPWSTR wzSmtpAddress, int maxSize)
{
	HRESULT hr = S_OK;
	CComPtr<IAddrBook>       lpAdrBook=0;
	ULONG           ulObjType = 0;
	LPADRLIST       lpAdrList = 0;
	int             i = 0;

	hr = lpSession->OpenAddressBook(0,NULL,0,&lpAdrBook);

	if(hr != S_OK)
	{
		DP((L"Failed to call OpenAddressBook, maybe the MAPI session was disconnected, try to re-connect\n"));
		hr = MAPILogonEx(NULL,NULL,NULL,MAPI_EXTENDED | MAPI_ALLOW_OTHERS,&pSession);
		if(hr == S_OK && pSession)
		{
			DP((L"try to open address book again\n"));
			lpSession = pSession;
			hr = lpSession->OpenAddressBook(0,NULL,0,&lpAdrBook);
		}
	}

	if(S_OK==hr && lpAdrBook)
	{
		// search address book
		hr = ExResolveName(lpAdrBook, (LPWSTR)wzDispName, &lpAdrList);
		DP((L"resolve name finished"));
		if(SUCCEEDED(hr))
		{
			for (i=0; i<(int)lpAdrList->aEntries->cValues; i++ )
			{
				if ( PR_ENTRYID == lpAdrList -> aEntries -> rgPropVals[i].ulPropTag )
				{
					// These next two assignments are unnecessary but make
					// the code more readable below.
					//OlAddressEntryUserType
					CComPtr<IMAPIProp>   lpAddress = 0;
					
					hr = lpAdrBook->OpenEntry(lpAdrList->aEntries->rgPropVals[i].Value.bin.cb,
						(LPENTRYID)(lpAdrList->aEntries->rgPropVals[i].Value.bin.lpb),
						NULL,
						0,
						&ulObjType,
						(LPUNKNOWN*)&lpAddress);
					
					if(SUCCEEDED(hr) && lpAddress)
					{
						ULONG         ulCount=0;
						SPropValue   *pValue;
						SPropTagArray spta; memset(&spta, 0, sizeof(SPropTagArray));
						spta.cValues = 1;
						spta.aulPropTag[0] = PR_SMTP_ADDRESS;
						
						hr = lpAddress->GetProps(&spta, MAPI_UNICODE, &ulCount, &pValue);
						
						if(S_OK == hr && pValue)
						{
							size_t size = 0;
							mbstowcs_s(&size, wzSmtpAddress, maxSize, pValue->Value.lpszA, _TRUNCATE);
							DP((L"Find Smtp address = %s\n", wzSmtpAddress));
						}
					}
					break;
				}

			}
			MAPIFreeBuffer(lpAdrList->aEntries[0].rgPropVals);
			MAPIFreeBuffer(lpAdrList);
			lpAdrList = NULL;
		}
	}
}

void OLUtilities::GetSmtpAddressByDisplayName(CComPtr<Outlook::_Application> spApp, LPCWSTR wzDispName, LPWSTR wzSmtpAddress, int maxSize)
{
	HRESULT hr = S_OK;
	CComPtr<IAddrBook>       lpAdrBook=0;
	CComPtr<IMAPISession>   lpMapiSession=0;
	ULONG           ulObjType = 0;
	CComPtr<Outlook::_NameSpace>    lpOlSession;
	LPADRLIST       lpAdrList = 0;
	int             i = 0;

	hr = spApp->get_Session(&lpOlSession);
	if(S_OK == hr && lpOlSession)
	{
		hr = lpOlSession->get_MAPIOBJECT((IUnknown**)&lpMapiSession);
		if(S_OK==hr && lpMapiSession)
		{
			hr = lpMapiSession->OpenAddressBook(0,NULL,0,&lpAdrBook);
			if(S_OK==hr && lpAdrBook)
			{
				// search address book
				hr = ExResolveName(lpAdrBook, (LPWSTR)wzDispName, &lpAdrList);
				if(SUCCEEDED(hr))
				{
					for (i=0; i<(int)lpAdrList->aEntries->cValues; i++ )
					{
						if ( PR_ENTRYID == lpAdrList -> aEntries -> rgPropVals[i].ulPropTag )
						{
							// These next two assignments are unnecessary but make
							// the code more readable below.
							//OlAddressEntryUserType
							CComPtr<IMAPIProp>   lpAddress = 0;
							hr = lpAdrBook->OpenEntry(lpAdrList->aEntries->rgPropVals[i].Value.bin.cb,
								(LPENTRYID)(lpAdrList->aEntries->rgPropVals[i].Value.bin.lpb),
								NULL,
								0,
								&ulObjType,
								(LPUNKNOWN*)&lpAddress);
							if(SUCCEEDED(hr) && lpAddress)
							{
								ULONG         ulCount=0;
								SPropValue   *pValue;
								SPropTagArray spta; memset(&spta, 0, sizeof(SPropTagArray));
								spta.cValues = 1;
								spta.aulPropTag[0] = PR_SMTP_ADDRESS;
								hr = lpAddress->GetProps(&spta, MAPI_UNICODE, &ulCount, &pValue);
								if(S_OK == hr && pValue)
								{
									size_t size = 0;
									mbstowcs_s(&size, wzSmtpAddress, maxSize, pValue->Value.lpszA, _TRUNCATE);
									DP((L"Find Smtp address = %s\n", wzSmtpAddress));
								}
							}
							break;
						}

					}
					MAPIFreeBuffer(lpAdrList->aEntries[0].rgPropVals);
					MAPIFreeBuffer(lpAdrList);
					lpAdrList = NULL;
				}
			}
		}
	}
}

void OLUtilities::GetSmtpAddressFromOfficeContact(CComPtr<Outlook::_Application> spApp, LPCWSTR wzDispName, LPWSTR wzSmtpAddress, int maxSize)
{
	HRESULT	hr = S_OK;
	CComBSTR bstrMAPI(L"MAPI");
	CComPtr<Outlook::_NameSpace>	ns;

	hr = spApp->GetNamespace(bstrMAPI, &ns);

	if( FAILED(hr) || ns == NULL )
	{
		return;
	}

	CComPtr<Outlook::MAPIFolder> spFolder = NULL;
	hr = ns->GetDefaultFolder(olFolderContacts, &spFolder);

	if( FAILED(hr) || spFolder == NULL )
	{
		return;
	}

	CComPtr<Outlook::_Items> spContactItems = NULL;
	hr = spFolder->get_Items(&spContactItems);

	if( SUCCEEDED(hr) && spContactItems != NULL )
	{
        WCHAR query[512] = { 0 };
        WCHAR email_display_name[512] = { 0 };    /* EmailXDisplayName */
        WCHAR email_address[512] = { 0 };         /* EmailXAddress */
        WCHAR email_address_type[512] = { 0 };    /* EmailXAddressType */
		/* EmailX attributes count [1,3] */
		for( int i = 1 ; i < 4 ; i++ )
		{
			CComPtr<Outlook::Recipient> spContactItem = NULL;
            ZeroMemory(query, 512 * sizeof(WCHAR));
			/* Query the contacts with the email display name to fine the contact item. */
			_snwprintf_s(query,_countof(query), _TRUNCATE,
				L"[Email%dDisplayName] = \"%s\"",i,wzDispName);

			hr = spContactItems->Find(query,(LPDISPATCH*)&spContactItem);
			if( FAILED(hr) || spContactItem == NULL )
			{
				/* There is not a contact match. */
				continue;
			}

			/* The display name exists.  spContactItem is a pointer to that item.  Find its
			attributes such that it can be translated for policy evaluation.  Each attribute
			name is generated based on the numeric index i.
			*/
			DP((L"OLUtilities::GetSmtpAddressFromOfficeContact: found %s\n",wzDispName));

			CComVariant varDispName;
			CComVariant varEmailAddress;
			CComVariant varEmailAddressType;

            ZeroMemory(email_display_name, 512 * sizeof(WCHAR));    /* EmailXDisplayName */
            ZeroMemory(email_address, 512 * sizeof(WCHAR));         /* EmailXAddress */
            ZeroMemory(email_address_type, 512 * sizeof(WCHAR));    /* EmailXAddressType */

			_snwprintf_s(email_display_name,_countof(email_display_name), _TRUNCATE,L"Email%dDisplayName",i); /* EmailXDisplayName */
			_snwprintf_s(email_address,_countof(email_address), _TRUNCATE,L"Email%dAddress",i);               /* EmailXAddress */
			_snwprintf_s(email_address_type,_countof(email_address_type), _TRUNCATE,L"Email%dAddressType",i); /* EmailXAddressType */

            CComPtr<IDispatch> spDisp = spContactItem;
			hr = AutoWrap(DISPATCH_PROPERTYGET, &varDispName, spDisp, email_display_name, 0);

			if(SUCCEEDED(hr) && VT_BSTR==varDispName.vt && varDispName.bstrVal && !IsBadReadPtr(varDispName.bstrVal, 2))
			{
				if (0 == _wcsicmp(varDispName.bstrVal, wzDispName))
				{
					hr = AutoWrap(DISPATCH_PROPERTYGET, &varEmailAddress, spDisp, email_address, 0);
					if(SUCCEEDED(hr) && VT_BSTR==varEmailAddress.vt && varEmailAddress.bstrVal && !IsBadReadPtr(varEmailAddress.bstrVal, 2))
					{
						hr = AutoWrap(DISPATCH_PROPERTYGET, &varEmailAddressType, spDisp, email_address_type, 0);
						if(SUCCEEDED(hr) && VT_BSTR==varEmailAddressType.vt && varEmailAddressType.bstrVal && !IsBadReadPtr(varEmailAddressType.bstrVal, 2))
						{
							if(0 == _wcsicmp(L"SMTP", varEmailAddressType.bstrVal))
							{
								wcsncpy_s(wzSmtpAddress, maxSize, varEmailAddress.bstrVal, _TRUNCATE);
							}
							else if(0 == _wcsicmp(L"EX", varEmailAddressType.bstrVal))
							{
								OLUtilities::GetSmtpAddressByDisplayName(spApp, varDispName.bstrVal, wzSmtpAddress, maxSize);
							}
						}
					}
				}
			}
		}/* for i */
	}
}/* OLUtilities::GetSmtpAddressFromOfficeContact */

BOOL OLUtilities::GetOutlookTempFolder(std::wstring& strTempFolder, int nVersion)
{
	HKEY hKeySecurity   = NULL;
 	char* szTempFolder = NULL;
	WCHAR* wzTempFolder = NULL;

	LONG lResult = 0;
	DWORD dwType = 0;
	DWORD dwSize = MAX_PATH;
    WCHAR wzKeyName[MAX_PATH] = { 0 };   
	_snwprintf_s(wzKeyName, MAX_PATH, _TRUNCATE, L"Software\\Microsoft\\Office\\%d.0\\Outlook\\Security", nVersion);

	lResult = RegOpenKeyExW(HKEY_CURRENT_USER,wzKeyName, 0, KEY_ALL_ACCESS, &hKeySecurity);
	if(ERROR_SUCCESS != lResult)
	{
		::ZeroMemory( wzKeyName, MAX_PATH*sizeof(wchar_t) ) ;
		_snwprintf_s(wzKeyName, MAX_PATH, _TRUNCATE, L"Software\\Wow6432Node\\Microsoft\\Office\\%d.0\\Outlook\\Security", nVersion);

		lResult = RegOpenKeyExW(HKEY_CURRENT_USER,wzKeyName, 0, KEY_ALL_ACCESS, &hKeySecurity);
		if(ERROR_SUCCESS != lResult)
		{
			return FALSE;
		}
	}

	if (ERROR_SUCCESS != RegQueryValueExA(hKeySecurity, "OutlookSecureTempFolder", NULL, &dwType, NULL, &dwSize))
	{
		RegCloseKey(hKeySecurity);
		return FALSE;
	}

	szTempFolder = new char[dwSize];
	ZeroMemory(szTempFolder, dwSize*sizeof(char));

	if(ERROR_SUCCESS != RegQueryValueExA(hKeySecurity, "OutlookSecureTempFolder", 0, &dwType, (LPBYTE)szTempFolder, &dwSize) || '\0'==szTempFolder[0])
	{
		RegCloseKey(hKeySecurity);
		delete[] szTempFolder;
		return FALSE;
	}

	RegCloseKey(hKeySecurity);
	_strupr(szTempFolder);
	int nPathLen = (int)strlen(szTempFolder);
	if('\\' != szTempFolder[nPathLen-1]) strncat_s(szTempFolder, MAX_PATH, "\\", _TRUNCATE);

	DWORD nNum = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szTempFolder, -1, NULL, 0);
    wzTempFolder = new WCHAR[nNum];
	ZeroMemory(wzTempFolder, nNum*sizeof(WCHAR));
	MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szTempFolder, (int)strlen(szTempFolder), wzTempFolder, nNum);
	strTempFolder = wzTempFolder;
	
	delete[] wzTempFolder;
	delete[] szTempFolder;
	return TRUE;
}
/*
added by chellee for the file don't have temporary path.
on 25/9/2008. 2:21 PM.
*/
//---------------------------------------------------------------------------------
BOOL OLUtilities::CheckAndReattachSpecFile(CComPtr<IDispatch> spMailItem,
										   CComPtr<Outlook::Attachments>	spAttachments ,
										   CComPtr<Outlook::Attachment>	spAttachment,
										   wchar_t *i_pszFileName,
										   wchar_t *i_pszDispName,
										   const UINT i_CurIndex )
{
	BOOL iRet = FALSE  ;
	if(	(  spAttachment == NULL) || ( i_pszFileName == NULL ) )
	{
		return iRet ;
	}
	LPCWSTR pSuffix = wcsrchr(i_pszDispName, L'.');
	if(NULL == pSuffix) 
	{
		return iRet;
	}
	if(0 == _wcsicmp(pSuffix, L".png") || 0 == _wcsicmp(pSuffix, L".jpg") || 
		0 == _wcsicmp(pSuffix, L".bmp")|| 0 == _wcsicmp(pSuffix, L".jpeg")|| 
		0 == _wcsicmp(pSuffix, L".ico")|| 0 == _wcsicmp(pSuffix, L".gif"))
	{
		return iRet ;
	}
	if(!PathIsRelative( i_pszFileName ) ) 
	{
		return iRet ;
	}
	DP((L"Need Reattach: File name  [%s]\n", i_pszFileName));
	SYSTEMTIME	sysTime;	memset(&sysTime, 0, sizeof(SYSTEMTIME));
	GetLocalTime(&sysTime);
	int nLen = 0;
	std::wstring strTempPath ;
	WCHAR wzTempAttachmentPath[MAX_PATH];	memset(wzTempAttachmentPath, 0, sizeof(wzTempAttachmentPath));
	WCHAR wzTempAttachmentPath2[MAX_PATH];	memset(wzTempAttachmentPath2, 0, sizeof(wzTempAttachmentPath2));

	// Get folder path
	_snwprintf_s(wzTempAttachmentPath, MAX_PATH, _TRUNCATE, L"%s%04d%02d%02d_%02d%02d%02d_%d\\",
		g_strOETempFolder.c_str(),
		sysTime.wYear, sysTime.wMonth, sysTime.wDay,
		sysTime.wHour, sysTime.wMinute, sysTime.wSecond, sysTime.wMilliseconds);
	if( !CreateDirectoryW(wzTempAttachmentPath, NULL) && ERROR_ALREADY_EXISTS!=GetLastError() )
		return iRet;

	nLen = (int)wcslen(wzTempAttachmentPath);
	if(nLen>0 && L'\\'!=wzTempAttachmentPath[nLen-1])
		wcsncat_s(wzTempAttachmentPath, MAX_PATH, L"\\", _TRUNCATE);
	_snwprintf_s(wzTempAttachmentPath2, MAX_PATH, _TRUNCATE, L"%s%d", wzTempAttachmentPath, i_CurIndex);
	if( !CreateDirectoryW(wzTempAttachmentPath2, NULL) && ERROR_ALREADY_EXISTS!=GetLastError() )
		return iRet;

	strTempPath = wzTempAttachmentPath2;
	strTempPath.append(L"\\");
	strTempPath.append(i_pszDispName);
	CComBSTR                varTempFile;
	varTempFile = strTempPath.c_str();
	HRESULT hr = spAttachment->SaveAsFile(varTempFile);
	if (FAILED(hr))
	{
		DP((L"Reattach No source File: Fail save to the temp file: %s\n", strTempPath.c_str()));
		goto ERROR_END;
	}
	DP((L"Reattach No source File:Save to the temp file: %s\n", strTempPath.c_str()));

	// Delete this attachment from Attachments
	hr = spAttachment->Delete();
    DP((L"  CheckAndReattachSpecFile: %s to remove attachment %d, %s\n", SUCCEEDED(hr)?L"Succeed":L"Fail", i_CurIndex, i_pszDispName));
	if (FAILED(hr))
	{
		DP((L"Reattach No source File:Fail delete the attachment(%d): %s\n", i_CurIndex, varTempFile));
		goto ERROR_END;
	}

	hr=MailItemUtility::Save(spMailItem);
	if (FAILED(hr))
	{
		DP((L"Reattach No source File:Fail save mail item the attachment(%d): %s\n", i_CurIndex, varTempFile));
		goto ERROR_END;
	}
	DP((L"Reattach No source File:Delete the attachment(%d): %s\n", i_CurIndex, varTempFile));

	{
		CComVariant varSource(strTempPath.c_str());
		CComVariant varOptional((long)DISP_E_PARAMNOTFOUND, VT_ERROR);
		CComPtr<Outlook::Attachment>	spNewAttach = 0;
		// Re-attach the attachment
        hr = spAttachments->Add(varSource, varOptional, varOptional, varOptional, &spNewAttach);
        DP((L"  CheckAndReattachSpecFile: %s (0x%08x) to add attachment %s\n", SUCCEEDED(hr)?L"Succeed":L"Fail", hr, strTempPath.c_str()));
		if(SUCCEEDED(hr)&&spNewAttach)
		{
			hr=MailItemUtility::Save(spMailItem);
			iRet = TRUE ;
		}
		SysFreeString(varTempFile); 
		::wcsncpy_s( i_pszFileName, MAX_PATH, strTempPath.c_str(), _TRUNCATE ) ;
	}

ERROR_END:
	return iRet ;
}



BOOL OLUtilities::NeedReAttach(LPCWSTR pwzFullName)
{
	if(!pwzFullName)
	{
		return FALSE;
	}

	LPCWSTR pExt = wcsrchr(pwzFullName, L'.');
	
	if(pExt &&
		(0 == _wcsicmp(pExt, L".png") || 0 == _wcsicmp(pExt, L".jpg") || 
		0 == _wcsicmp(pExt, L".bmp")|| 0 == _wcsicmp(pExt, L".jpeg")|| 
		0 == _wcsicmp(pExt, L".ico")|| 0 == _wcsicmp(pExt, L".gif"))
		)
	{
		return FALSE ;
	}

	return PathIsRelative( pwzFullName );
}

BOOL MakeTempPath(LPWSTR pwzPath, int nBufLen, LPCWSTR pwzFileName, int nIndex)
{
	if(!pwzPath || !pwzFileName)
	{
		return FALSE;
	}

	SYSTEMTIME	sysTime;	memset(&sysTime, 0, sizeof(SYSTEMTIME));
	GetLocalTime(&sysTime);
	int nLen = 0;

	wchar_t wzTempAttachmentPath[MAX_PATH * 2] = {0};
	wchar_t wzTempPath[MAX_PATH * 2] = {0};	

	// Get folder path
	_snwprintf_s(wzTempAttachmentPath, MAX_PATH * 2, _TRUNCATE, L"%s%04d%02d%02d_%02d%02d%02d_%d\\",
		g_strOETempFolder.c_str(),
		sysTime.wYear, sysTime.wMonth, sysTime.wDay,
		sysTime.wHour, sysTime.wMinute, sysTime.wSecond, sysTime.wMilliseconds);

	DPW((L"Try to create directory: %s", wzTempAttachmentPath));
	if( !CreateDirectoryW(wzTempAttachmentPath, NULL) && ERROR_ALREADY_EXISTS!=GetLastError() )
	{
		DPW((L"Failed to create directory: %s, error code: %d", wzTempAttachmentPath, GetLastError()));
		return FALSE;
	}

	nLen = (int)wcslen(wzTempAttachmentPath);
	if(nLen > 0 && L'\\'!=wzTempAttachmentPath[nLen-1])
	{
		wcsncat_s(wzTempAttachmentPath, MAX_PATH * 2, L"\\", _TRUNCATE);
	}

	_snwprintf_s(wzTempPath, MAX_PATH * 2, _TRUNCATE, L"%s%d", wzTempAttachmentPath, nIndex);
	if( !CreateDirectoryW(wzTempPath, NULL) && ERROR_ALREADY_EXISTS != GetLastError() )
	{
		return FALSE;
	}

	//generate the temp file path
	wcsncat_s(wzTempPath, MAX_PATH * 2, L"\\", _TRUNCATE);
	wcsncat_s(wzTempPath, MAX_PATH * 2, (WCHAR*)pwzFileName, _TRUNCATE);
	wcsncpy_s(pwzPath, nBufLen, wzTempPath, _TRUNCATE);

	DPW((L"The local temp path for reattach: %s", pwzPath));

	return TRUE;
}

BOOL OLUtilities::IsIgnoredAttach(CComPtr<Outlook::Attachment>   spAttachment)
{
	CComPtr<IUnknown> pUnk = NULL;
	CComPtr<IAttach> pIAttach = NULL;


	//try to get MAPI interface from OOM (Outlook object model)
	BOOL bIsIgnored = FALSE;
	if(SUCCEEDED(spAttachment->get_MAPIOBJECT( &pUnk )))
	{
		if( SUCCEEDED( pUnk->QueryInterface(IID_IAttachment, (void**) &pIAttach) ))
		{
			ULONG rgTags[]       = {2, PR_ATTACH_CONTENT_ID, PR_ATTACH_CONTENT_LOCATION};
			SPropValue * rgprops = NULL;
			ULONG cCount         = 0;

			DP((L"[IsIgnoredAttach]Succeeded to get IAttach interface for attachment from OOM Outlook::Attachment.\n"));
			if(SUCCEEDED(pIAttach->GetProps((LPSPropTagArray) rgTags, MAPI_UNICODE, &cCount, &rgprops)))//Try to get the properties of the current attachment
			{
				DP((L"Succeeded to get properties for attachment, property count: %d\n", cCount));

				LONG lIndex = -1;
				spAttachment->get_Index(&lIndex);
				BSTR    bstrDisplayName;
				HRESULT hr = spAttachment->get_DisplayName(&bstrDisplayName);
				wchar_t szDisplayName[MAX_PATH] = {0};
				if(SUCCEEDED(hr) && NULL!= bstrDisplayName)
				{
					wcsncpy_s(szDisplayName, MAX_PATH, bstrDisplayName, _TRUNCATE);
					SysFreeString(bstrDisplayName);
				}

				if(boost::algorithm::icontains(szDisplayName, L"(Device independent bitmap)") || boost::algorithm::icontains(szDisplayName, L"picture (metafile)"))
				{
					DP((L"Ingore this attachment since it's diplay name contains \"(Device independent bitmap)\". Maybe this attachment is an embedded picture in the mail body with \"rich text\" mode. %s\n", szDisplayName));
					bIsIgnored = TRUE;
				}

				if (rgprops[0].ulPropTag == PR_ATTACH_CONTENT_ID || rgprops[0].ulPropTag == PR_ATTACH_CONTENT_LOCATION)
				{//Check if the current attachment is a "embedded attachment", we don't need to handle "embedded attachments".
					BSTR    bstrFileName;
					hr = spAttachment->get_DisplayName(&bstrFileName);
					wchar_t szFileName[MAX_PATH] = {0};
					if(SUCCEEDED(hr) && NULL!= bstrFileName)
					{
						wcsncpy_s(szFileName, MAX_PATH, bstrFileName, _TRUNCATE);
						SysFreeString(bstrFileName);
					}

					DP((L"This is an embedded attachment, index: %d, file name: %s\n", lIndex, szFileName));
					bIsIgnored = TRUE;
				}

                if(NULL != rgprops)
                {
                    MAPIFreeBuffer(rgprops);
                    rgprops = NULL;
                }
			}
		}
	}

	return bIsIgnored;
}

BOOL OLUtilities::CheckReattach4TaskRequestItem(CComPtr<IDispatch> spMailItem,CComPtr<IDispatch> taskItem)
{
	HRESULT                         hr = S_OK;
	int                             i = 0, nAttchmentCount = 0;
	CComPtr<Outlook::Attachments>   spAttachments;

	MailItemUtility::Save(spMailItem);

	hr = MailItemUtility::get_Attachments(spMailItem,&spAttachments,TRUE);
	if(SUCCEEDED(hr))
	{
		spAttachments->get_Count((long*)&nAttchmentCount);
		std::list<std::wstring> vReattach;
		if( nAttchmentCount >0)
		{
			CComPtr<Outlook::Attachment>   spAttachment;
			VARIANT vi; vi.vt = VT_INT; vi.intVal = 1;

			hr = spAttachments->Item(vi, &spAttachment);
			if(SUCCEEDED(hr))
			{
				wchar_t szFileName[MAX_PATH] = {0};
                std::wstring strDisplayFileName;
                strDisplayFileName = OLUtilities::GetAttachFileName(spAttachment);
                if(!strDisplayFileName.empty())
                {
                    wcsncpy_s(szFileName, MAX_PATH, strDisplayFileName.c_str(), _TRUNCATE);
                }

				DP((L"CheckReattach4TaskRequestItemCheck attachment file name: %s\n", szFileName));

				if(IsIgnoredAttach(spAttachment))//We don't need to handle "embedded attachment", 
				{
					return FALSE ;
				}

				BOOL bNeedReattach = TRUE;

                WCHAR wzCombinePath[1024] = { 0 };
                OLUtilities::GetAttachmentDispName(spAttachment, wzCombinePath, 1023);
                DP((L"CheckReattach4TaskRequestItemGET magic path: %s\n", wzCombinePath));
                LPCWSTR pwzMagicFlag = wcsstr(wzCombinePath, TEMP_MAGIC_NAME);

                if (pwzMagicFlag)
                {
                    LPCWSTR pwzRealPath = pwzMagicFlag + wcslen(TEMP_MAGIC_NAME);

                    if (!NeedReAttach(pwzRealPath))
                    {
                        bNeedReattach = FALSE;
                    }
                }
	

				if(bNeedReattach )
				{
					wchar_t szTempPath[MAX_PATH * 2] = {0};
					if(szFileName[0] != '\0' && MakeTempPath(szTempPath, MAX_PATH * 2, szFileName, i))
					{
						CComBSTR  varTempFile;
						varTempFile = szTempPath;
						hr =MailItemUtility::SaveAs( taskItem,varTempFile);//Save the attachment to local
						if(SUCCEEDED(hr))
						{
                            hr = spAttachment->Delete();
							if( SUCCEEDED( hr ))
							{
								MailItemUtility::Save(spMailItem);
								vReattach.push_front(szTempPath);
							}
						}
					}
				}
			}
		}

		//Reattach
		std::list<std::wstring>::iterator itr;
		for(itr = vReattach.begin(); itr != vReattach.end(); itr++)
		{
            CComVariant varSource(itr->c_str());
			CComVariant varOptional((long)DISP_E_PARAMNOTFOUND, VT_ERROR);
			CComPtr<Outlook::Attachment>	spNewAttach = 0;
			// Re-attach the attachment
			MailItemUtility::Save(spMailItem);
			hr = spAttachments->Add(varSource, varOptional, varOptional, varOptional, &spNewAttach);
			if(SUCCEEDED(hr)&&spNewAttach)
			{
				MailItemUtility::Save(spMailItem);
			}
		}
	}
	return TRUE;
}
BOOL OLUtilities::CheckReattach(CComPtr<IDispatch> spMailItem, BOOL bReattaAll, BOOL bNeedSave)
{
	HRESULT                         hr = S_OK;
	int                             i = 0, nAttchmentCount = 0;
	CComPtr<Outlook::Attachments>   spAttachments;

	if (bNeedSave)
		MailItemUtility::Save(spMailItem);

	hr = MailItemUtility::get_Attachments(spMailItem,&spAttachments,TRUE);
	if(SUCCEEDED(hr))
	{
		spAttachments->get_Count((long*)&nAttchmentCount);
		std::list<std::wstring> vReattach;
		for(i = nAttchmentCount; i > 0; i--)
		{
			CComPtr<Outlook::Attachment>   spAttachment;
			VARIANT vi; vi.vt = VT_INT; vi.intVal = i;

			hr = spAttachments->Item(vi, &spAttachment);
			if(SUCCEEDED(hr))
            {
                std::wstring strDisplayFileName;
                wchar_t szFileName[MAX_PATH] = {0};

                strDisplayFileName = OLUtilities::GetAttachFileName(spAttachment);
                if(!strDisplayFileName.empty())
                {
                    wcsncpy_s(szFileName, MAX_PATH, strDisplayFileName.c_str(), _TRUNCATE);
                }

				if(IsIgnoredAttach(spAttachment))//We don't need to handle "embedded attachment", 
				{
					continue;
				}

				BOOL bNeedReattach = TRUE;

				if(!bReattaAll)
				{
					WCHAR wzCombinePath[1024] = {0};
					OLUtilities::GetAttachmentDispName(spAttachment, wzCombinePath, 1023);
					DP((L"GET magic path: %s\n", wzCombinePath));
					LPCWSTR pwzMagicFlag = wcsstr(wzCombinePath, TEMP_MAGIC_NAME);

					if(pwzMagicFlag)
					{
						LPCWSTR pwzRealPath = pwzMagicFlag + wcslen(TEMP_MAGIC_NAME);

						if(!NeedReAttach(pwzRealPath))
						{
							bNeedReattach = FALSE;
						}
					}
				}

				if(bNeedReattach || bReattaAll)
				{//Need reattach

					wchar_t szTempPath[MAX_PATH * 2] = {0};
					if(szFileName[0] != '\0' && MakeTempPath(szTempPath, MAX_PATH * 2, strDisplayFileName.c_str(), i))
					{
						CComBSTR  varTempFile;
						varTempFile = szTempPath;

						hr = spAttachment->SaveAsFile(varTempFile);//Save the attachment to local
						if(SUCCEEDED(hr))
						{
                            hr = spAttachment->Delete();
							if( SUCCEEDED( hr ))
							{
								if (bNeedSave)  MailItemUtility::Save(spMailItem);
								vReattach.push_front(szTempPath);
							}
						}
					}
				}
			}
		}
		

		//Reattach
		std::list<std::wstring>::iterator itr;
		
		for(itr = vReattach.begin(); itr != vReattach.end(); itr++)
		{
			CComVariant varSource(itr->c_str());
			CComVariant varOptional((long)DISP_E_PARAMNOTFOUND, VT_ERROR);
			CComPtr<Outlook::Attachment>	spNewAttach = 0;
			// Re-attach the attachment
			if (bNeedSave)
 				MailItemUtility::Save(spMailItem);
			hr = spAttachments->Add(varSource, varOptional, varOptional, varOptional, &spNewAttach);
			if(SUCCEEDED(hr)&&spNewAttach)
			{
				if (bNeedSave)
					MailItemUtility::Save(spMailItem);
			}	
		}
	}
	return TRUE;
}

//---------------------------------------------------------------------------------
std::wstring OLUtilities::GetAttachFileName(CComPtr<Outlook::Attachment> Attachment)
{
	HRESULT         hr = S_OK;
    CComBSTR        bstrDispName;
    std::wstring    strFileName;

	hr = Attachment->get_DisplayName(&bstrDispName);
	if(SUCCEEDED(hr) && NULL!=bstrDispName.m_str)
        strFileName = bstrDispName.m_str;

    if(boost::algorithm::icontains(strFileName, TEMP_MAGIC_NAME))
    {
        // The display name is NextLabs Special string
        // Try to get file name from it
        CDisplayName dispName(strFileName.c_str());
		strFileName = dispName.GetFileName();
    }
    else
    {
        // Otherwise, we should use get_FileName here
        CComBSTR        bstrFileName;
        hr = Attachment->get_FileName(&bstrFileName);
        if(SUCCEEDED(hr) && NULL!=bstrFileName.m_str)
            strFileName = bstrFileName.m_str;
    }

    return strFileName;
}

int  OLUtilities::GetAllAttachments(CComPtr<IDispatch> spMailItem, ATTACHMENTLIST& listAttachment, BOOL bNeedSave)
{
	CheckReattach(spMailItem, FALSE, bNeedSave);

	HRESULT                         hr = S_OK;
	int                             i = 0, nAttchmentCount = 0;
	CComPtr<Outlook::Attachments>   spAttachments;
	
	if (bNeedSave)
		MailItemUtility::Save(spMailItem);

	listAttachment.clear();

	hr = MailItemUtility::get_Attachments(spMailItem,&spAttachments,TRUE);
	if(SUCCEEDED(hr))
	{
		spAttachments->get_Count((long*)&nAttchmentCount);
		for(i=0; i<nAttchmentCount; i++)
		{
			DP((L"\n[Start] Check an attachment, index: %d\n", i));
			CComPtr<Outlook::Attachment>   spAttachment;
			VARIANT vi; vi.vt = VT_INT; vi.intVal = i+1;

			hr = spAttachments->Item(vi, &spAttachment);
			if(SUCCEEDED(hr))
			{
				LpAttachmentData lpAttachmentData = new AttachmentData;
				if(lpAttachmentData)
                {
                    memset(lpAttachmentData, 0, sizeof(AttachmentData));
                    lpAttachmentData->iIndex = i+1;

					/************************************************************************/
					/*  Get Display Name                                                    */
					/************************************************************************/
					if(IsIgnoredAttach(spAttachment))
					{
						lpAttachmentData->bIgnored = TRUE;
					}
					else
					{
						lpAttachmentData->bIgnored = FALSE;
					}

                    std::wstring wstrFileName = OLUtilities::GetAttachFileName(spAttachment);
                    if(!wstrFileName.empty())
					{
                        wcsncpy_s(lpAttachmentData->dispname, MAX_SRC_PATH_LENGTH, wstrFileName.c_str(), _TRUNCATE);
					}
					else
					{
						wcsncpy_s(lpAttachmentData->dispname,MAX_SRC_PATH_LENGTH, L"", _TRUNCATE);
					}
                    
					/************************************************************************/
					/*  Get temp file path                                                  */
					/************************************************************************/
					WCHAR* pwzMagicFlag = 0;
					WCHAR* pwzRealPath  = 0;
					WCHAR wzCombinePath[1024];	memset(wzCombinePath, 0, sizeof(wzCombinePath));
					OLUtilities::GetAttachmentDispName(spAttachment, wzCombinePath, 1023);
					pwzMagicFlag = wcsstr(wzCombinePath, TEMP_MAGIC_NAME);

					BOOL bIsOverflow = FALSE;

					if ( 0xFF == wcslen ( wzCombinePath ) )
					{
						bIsOverflow = TRUE;	
					}
					
					BOOL bDummy = FALSE;
					if(!pwzMagicFlag)
					{
						memset(wzCombinePath, 0, sizeof(wzCombinePath));
						_snwprintf_s(wzCombinePath, sizeof(wzCombinePath)/sizeof(wchar_t), _TRUNCATE, TEMP_MAGIC_NAME_FMT, L"c:\\dummy.tmp", lpAttachmentData->dispname );
						DP((L"Can't get magic string, make a dummy here, %s\n", wzCombinePath));
						pwzMagicFlag = wcsstr(wzCombinePath, TEMP_MAGIC_NAME);
						bDummy = TRUE;
					}
					
					pwzRealPath = pwzMagicFlag + wcslen(TEMP_MAGIC_NAME);
						
						//---------------------------------------------------------------------------------
					*pwzMagicFlag = NULL;

					
						//---------------------------------------------------------------------------------
						
					
					//---------------------------------------------------------------------------------
					//Code for bug 104
					//added by casablanca  -- for bug104
					//if temp file does not exists, Save As a new one.
					if(!bDummy)
					{
						FILE *mFile = NULL;
						char *fname = new char[wcslen(wzCombinePath)+1];
						size_t size = 0;
						wcstombs_s(&size, fname, wcslen(wzCombinePath)+1, wzCombinePath, _TRUNCATE);
						mFile = fopen( fname, "r" );
						if (mFile == NULL)
						{
							spAttachment->SaveAsFile( wzCombinePath );
						}
						else
						{
							fclose( mFile );
						} 
                        delete[] fname;
                        fname = NULL;
					}
					//end
					//end

					//Context
					wcsncpy_s(lpAttachmentData->temp, MAX_SRC_PATH_LENGTH, wzCombinePath, _TRUNCATE);
					DP((L"[GetAllAttachments] Attachment%d_temp: %s\n", i, lpAttachmentData->temp));

					if ( bIsOverflow )
					{
						pwzRealPath = lpAttachmentData->temp;
					}

					/************************************************************************/
					/*  Get real file path                                                  */
					/************************************************************************/
					if( IsWordFile(lpAttachmentData->temp) )
					{
						lpAttachmentData->type = attachWord;
#ifdef XXXXXWSO2K7
						// try to get from custom properties
						CComPtr<Word::_Document> spWd = OpenWordDoc(&g_spWordApp, lpAttachmentData->temp);
						if(NULL != spWd)
						{
							hr = spWd->get_CustomDocumentProperties(&lpCUDisp);
							if(SUCCEEDED(hr) && lpCUDisp)
							{
								GetDocumentPropValue(lpCUDisp, OCP_REALPATH, lpAttachmentData->src, MAX_PATH);
								lpCUDisp->Release();
							}
							spWd->Close();
						}
#else
						if(pwzRealPath)
							wcsncpy_s(lpAttachmentData->src, MAX_SRC_PATH_LENGTH, pwzRealPath, _TRUNCATE);
#endif

						if(0 == lpAttachmentData->src[0])
						{
							if(NULL != pwzRealPath && 0 != *pwzRealPath)
								wcsncpy_s(lpAttachmentData->src, MAX_SRC_PATH_LENGTH, pwzRealPath, _TRUNCATE);
							else
								wcsncpy_s(lpAttachmentData->src, MAX_SRC_PATH_LENGTH, lpAttachmentData->dispname, _TRUNCATE);
						}
					}
					else if( IsExcelFile(lpAttachmentData->temp) )
					{
						lpAttachmentData->type = attachExcel;
						// try to get from custom properties
#ifdef XXXXXWSO2K7
						CComPtr<Excel::_Workbook> spExcl = OpenExcelDoc(&g_spExcelApp, lpAttachmentData->temp);
						if(NULL != spExcl)
						{
							hr = spExcl->get_CustomDocumentProperties(&lpCUDisp);
							if(SUCCEEDED(hr) && lpCUDisp)
							{
								GetDocumentPropValue(lpCUDisp, OCP_REALPATH, lpAttachmentData->src, MAX_PATH);
								lpCUDisp->Release();
							}
							spExcl->Close();
						}
#else
						if(pwzRealPath)
							wcsncpy_s(lpAttachmentData->src, MAX_SRC_PATH_LENGTH, pwzRealPath, _TRUNCATE);
#endif

						if(0 == lpAttachmentData->src[0])
						{
							if(NULL != pwzRealPath && 0 != *pwzRealPath)
								wcsncpy_s(lpAttachmentData->src, MAX_SRC_PATH_LENGTH, pwzRealPath, _TRUNCATE);
							else
								wcsncpy_s(lpAttachmentData->src, MAX_SRC_PATH_LENGTH, lpAttachmentData->dispname, _TRUNCATE);
						}
					}
					else if( IsPwptFile(lpAttachmentData->temp) )
					{
						lpAttachmentData->type = attachPPT;
						// try to get from custom properties
#ifdef XXXXXXXWSO2K7
						CComPtr<PPT::_Presentation> spPwpt = OpenPwptDoc(&g_spPPTApp, lpAttachmentData->temp);
						if(NULL != spPwpt)
						{
							hr = spPwpt->get_CustomDocumentProperties(&lpCUDisp);
							if(SUCCEEDED(hr) && lpCUDisp)
							{
								GetDocumentPropValue(lpCUDisp, OCP_REALPATH, lpAttachmentData->src, MAX_PATH);
								lpCUDisp->Release();
							}
							spPwpt->Close();
						}
#else
						if(pwzRealPath)
							wcsncpy_s(lpAttachmentData->src, MAX_SRC_PATH_LENGTH, pwzRealPath, _TRUNCATE);
#endif

						if(0 == lpAttachmentData->src[0])
						{
							if(NULL != pwzRealPath && 0 != *pwzRealPath)
								wcsncpy_s(lpAttachmentData->src, MAX_SRC_PATH_LENGTH, pwzRealPath, _TRUNCATE);
							else
								wcsncpy_s(lpAttachmentData->src, MAX_SRC_PATH_LENGTH, lpAttachmentData->dispname, _TRUNCATE);
						}
					}
					else
					{
						lpAttachmentData->type = attachOthers;
						if(NULL != pwzRealPath && 0 != *pwzRealPath)
							wcsncpy_s(lpAttachmentData->src, MAX_SRC_PATH_LENGTH, pwzRealPath, _TRUNCATE);
						else
							wcsncpy_s(lpAttachmentData->src, MAX_SRC_PATH_LENGTH, lpAttachmentData->dispname, _TRUNCATE);
					}
					DP((L"[GetAllAttachments] Attachment%d_src, ignored: %d, file name:  %s\n", i, lpAttachmentData->bIgnored, lpAttachmentData->src));

					/************************************************************************/
					/*  Insert into list                                                    */
					/************************************************************************/
					listAttachment.push_back(lpAttachmentData);
				}

			}
			DP((L"\n[End] Check an attachment, index: %d\n", i));
		}
	}
	
	return (int)(listAttachment.size());
}

void OLUtilities::ClearAttachmentsList(ATTACHMENTLIST& listAttachment)
{
	int i=0, nSize = (int)listAttachment.size();
	for (i=0; i<nSize; i++)
	{
		LpAttachmentData lpAttachmentData = (LpAttachmentData)listAttachment[i];
		if(lpAttachmentData) delete lpAttachmentData;
	}
	listAttachment.clear();
}

void OLUtilities::ActiveMailWindow(CComPtr<IDispatch> spMailItem,VARIANT_BOOL &isWordMail)
{
	HRESULT              hr     = S_OK;
	CComPtr<Outlook::_Inspector> spInsp;

	if(spMailItem)
	{
		hr = MailItemUtility::get_GetInspector(spMailItem,&spInsp);
		if(SUCCEEDED(hr) && spInsp)
		{
			spInsp->Activate();
			spInsp->IsWordMail(&isWordMail);
		}
	}
}

BOOL OLUtilities::PrependMailBody(CComPtr<IDispatch> spMailItem, LPCWSTR wzPrependText)
{
	bool bHtml = false;
	OlBodyFormat bodyformat = olFormatUnspecified;
	if(SUCCEEDED(MailItemUtility::get_BodyFormat(spMailItem,&bodyformat)))
	{		
		if (bodyformat == olFormatHTML)
		{
			bHtml = true;
		}
	}



	HRESULT  hr   = S_OK;
	BOOL     bRet = FALSE;

	std::wstring wstrPrependText(wzPrependText);
	if (bHtml)
	{
		boost::algorithm::replace_all(wstrPrependText, L"\\n", L"<br>");
		wstrPrependText.append(L"<br>");
	}
	else
	{
		boost::algorithm::replace_all(wstrPrependText, L"\\n", L"\n");
		wstrPrependText.append(L"\n");
	}

    CComBSTR bstrOldBody;
	hr =MailItemUtility::get_HTMLBody(spMailItem,&bstrOldBody);

	if(SUCCEEDED(hr) && bstrOldBody.m_str )
	{
		CComBSTR bstrBody;
		if(bHtml)
		{
			std::wstring strOldBody = bstrOldBody.m_str;
			PreAppendHtml(strOldBody, wstrPrependText);
			bstrBody.Append(strOldBody.c_str());
		}
		else
		{
			bstrBody.Append(wstrPrependText.c_str());
			bstrBody.Append(bstrOldBody);
		}

		hr =MailItemUtility::put_HTMLBody(spMailItem,bstrBody);
		if(SUCCEEDED(hr)) bRet = TRUE;
	}

	return bRet;
}

BOOL OLUtilities::AppendMailBody(CComPtr<IDispatch> spMailItem, LPCWSTR wzAppendText)
{
	bool bHtml = false;
	OlBodyFormat bodyformat = olFormatUnspecified;
	if(SUCCEEDED(MailItemUtility::get_BodyFormat(spMailItem,&bodyformat)))
	{
		if(bodyformat == olFormatRichText)//don't do "append body" for rich text mode
		{
			DP((L"Ignore \"append body\" for rich text mode.\n"));
			return FALSE;
		}
		else if (bodyformat == olFormatHTML)
		{
			bHtml = true;
		}
	}

	CComBSTR bstrBody;
	HRESULT  hr   = S_OK;
	BOOL     bRet = FALSE;

	hr = MailItemUtility::get_HTMLBody(spMailItem,&bstrBody);
	if(SUCCEEDED(hr))
	{
		std::wstring wstrAppendText;
		if (!bHtml) 
		{
			wstrAppendText = L"\n";
		}
		wstrAppendText.append(wzAppendText);
		if (bHtml)
		{
			boost::algorithm::replace_all(wstrAppendText, L"\\n", L"<br>");
			wstrAppendText.append(L"<br>");
		}
		else
		{
			boost::algorithm::replace_all(wstrAppendText, L"\\n", L"\n");
		}

		hr = bstrBody.Append(wstrAppendText.c_str());
		hr = MailItemUtility ::put_HTMLBody(spMailItem,bstrBody);
		if(SUCCEEDED(hr)) bRet = TRUE;
	}

	return bRet;
}

BOOL OLUtilities::SetMailBody(CComPtr<IDispatch> spMailItem, LPCWSTR wzNewBody)
{
	OlBodyFormat bodyformat = olFormatUnspecified;
	if(SUCCEEDED(MailItemUtility::get_BodyFormat(spMailItem,&bodyformat)))
	{
		if(bodyformat == olFormatRichText)
		{
			DP((L"Ignore \"body\" for rich text mode.\n"));
			return FALSE;
		}
		else if (bodyformat == olFormatHTML)
		{
			CComBSTR bstrBody(wzNewBody);
			HRESULT	hr = MailItemUtility::put_HTMLBody(spMailItem,bstrBody);
			return (SUCCEEDED(hr));
		}
		else
		{
			CComBSTR bstrBody(wzNewBody);
			HRESULT	hr = MailItemUtility::put_Body(spMailItem,bstrBody);
			return (SUCCEEDED(hr));
		}
	}
    return FALSE;
}

BOOL OLUtilities::SetMailSubject(CComPtr<IDispatch> spMailItem, LPCWSTR wzNewSubject)
{
	logi(L"set mail subject = %s\n", wzNewSubject);
	CComBSTR bstrSubejct(wzNewSubject);
	HRESULT	hr = MailItemUtility::put_Subject(spMailItem,bstrSubejct);
	return (SUCCEEDED(hr));
}

BOOL OLUtilities::PrependMailSubject(CComPtr<IDispatch> spMailItem, LPCWSTR wzPrependText)
{
	CComBSTR bstrSubejct;
	CComBSTR bstrOldSubejct;
	HRESULT  hr   = S_OK;
	BOOL     bRet = FALSE;

	hr = bstrSubejct.Append(wzPrependText);
	hr =MailItemUtility::get_Subject(spMailItem,&bstrOldSubejct);
	if(SUCCEEDED(hr))
	{
		hr = bstrSubejct.Append(bstrOldSubejct);
		hr = MailItemUtility::put_Subject(spMailItem,bstrSubejct);
		if(SUCCEEDED(hr)) bRet = TRUE;
	}

	return bRet;
}

BOOL OLUtilities::GetAttachmentByIndex(CComPtr<IDispatch> spMailItem, int nAttachIndex/*base on 0*/, Outlook::Attachment** ppAttachment)
{
	HRESULT                         hr  = S_OK;
	BOOL                            bRet= FALSE;
	int                             nAttchmentCount = 0;
	CComPtr<Outlook::Attachments>   spAttachments;

	hr = MailItemUtility::get_Attachments(spMailItem,&spAttachments,TRUE);
	if(SUCCEEDED(hr))
	{
		spAttachments->get_Count((long*)&nAttchmentCount);
		if(nAttachIndex < nAttchmentCount)
		{
			VARIANT vi; vi.vt = VT_INT; vi.intVal = nAttachIndex+1;
			hr = spAttachments->Item(vi, ppAttachment);
			if(SUCCEEDED(hr) && ppAttachment)
			{
				bRet = TRUE;
			}
		}
	}

	return bRet;
}

BOOL OLUtilities::WriteTagMessageToAttachment(CComPtr<IDispatch> spMailItem, int nAttachIndex, LPCWSTR wzTagMessage)
{
	HRESULT                         hr  = S_OK;
	BOOL                            bRet= FALSE;
	CComPtr<Outlook::Attachment>    spAttachment;

	if(GetAttachmentByIndex(spMailItem, nAttachIndex, &spAttachment))
	{
		CComPtr<IMAPIProp>  lpAttach=0;
		spAttachment->get_MAPIOBJECT((IUnknown**)&lpAttach);
		if(S_OK==hr && lpAttach)
		{
			ULONG         ulCount=1;
			SPropValue    PropValue;

			PropValue.ulPropTag    = PR_ATTACH_TAG;
			PropValue.Value.bin.cb = (int)(sizeof(WCHAR)*(wcslen(wzTagMessage)+1));
			PropValue.Value.bin.lpb= (BYTE*)wzTagMessage;
			hr = lpAttach->SetProps(ulCount, &PropValue, NULL);
			if(SUCCEEDED(hr)) bRet = TRUE;
		}

	}

	return bRet;
}

BOOL OLUtilities::ReadTagMessageToAttachment(CComPtr<IDispatch> spMailItem, int nAttachIndex, LPWSTR wzTagMessage, int nTagMessageLen)
{
	HRESULT                         hr  = S_OK;
	BOOL                            bRet= FALSE;
	CComPtr<Outlook::Attachment>    spAttachment;

	if(GetAttachmentByIndex(spMailItem, nAttachIndex, &spAttachment))
	{
		CComPtr<IMAPIProp>  lpAttach=0;
		spAttachment->get_MAPIOBJECT((IUnknown**)&lpAttach);
		if(S_OK==hr && lpAttach)
		{
			ULONG         ulCount=0;
			SPropValue   *pValue;
			SPropTagArray spta; memset(&spta, 0, sizeof(SPropTagArray));
			spta.cValues = 1;
			spta.aulPropTag[0] = PR_ATTACH_TAG;
			hr = lpAttach->GetProps(&spta, MAPI_UNICODE, &ulCount, &pValue);
			if(S_OK == hr && pValue)
			{
				ULONG ulCopyLength = min(pValue->Value.bin.cb, sizeof(WCHAR)*nTagMessageLen);
				memcpy(wzTagMessage, pValue->Value.bin.lpb, ulCopyLength);
				bRet = TRUE;
			}
		}
	}

	return bRet;
}


BOOL OLUtilities::SetAttachmentLongFileName(CComPtr<Outlook::Attachment> spAttachment, LPCWSTR wzLongFileName)
{
	HRESULT     hr      = S_OK;
	BOOL        bRet    = FALSE;
	CComPtr<IMAPIProp>  lpAttach= 0;

	spAttachment->get_MAPIOBJECT((IUnknown**)&lpAttach);
	if(S_OK==hr && lpAttach)
	{
		ULONG         ulCount=1;
		SPropValue    PropValue;

		PropValue.ulPropTag    = 0x3707001F;//PR_ATTACH_LONG_FILENAME;
		PropValue.Value.lpszW  = (WCHAR*)wzLongFileName;
		hr = lpAttach->SetProps(ulCount, &PropValue, NULL);
		if(SUCCEEDED(hr))
		{
			bRet = TRUE;
		}
	}

	return bRet;
}

BOOL OLUtilities::GetAttachmentLongFileName(CComPtr<Outlook::Attachment> spAttachment, LPWSTR wzLongFileName, int nBufLen)
{
	HRESULT     hr      = S_OK;
	BOOL        bRet    = FALSE;
	CComPtr<IMAPIProp>  lpAttach= 0;

	hr = spAttachment->get_MAPIOBJECT((IUnknown**)&lpAttach);
	if(S_OK==hr && lpAttach)
	{
		ULONG         ulCount=0;
		SPropValue   *pValue;
		SPropTagArray spta; memset(&spta, 0, sizeof(SPropTagArray));
		spta.cValues = 1;
		spta.aulPropTag[0] = 0x3707001F;//PR_ATTACH_LONG_FILENAME;
		hr = lpAttach->GetProps(&spta, MAPI_UNICODE, &ulCount, &pValue);
		if(S_OK == hr && pValue)
		{
			if(pValue->Value.lpszW) wcsncpy_s(wzLongFileName, nBufLen, pValue->Value.lpszW, _TRUNCATE);
			bRet = TRUE;
		}
	}

	return bRet;
}

BOOL OLUtilities::SetAttachmentLongPathName(CComPtr<Outlook::Attachment> spAttachment, LPCWSTR wzLongPathName)
{
	HRESULT     hr      = S_OK;
	BOOL        bRet    = FALSE;
	CComPtr<IMAPIProp>  lpAttach= 0;

	hr = spAttachment->get_MAPIOBJECT((IUnknown**)&lpAttach);
	if(S_OK==hr && lpAttach)
	{
		ULONG         ulCount=1;
		SPropValue    PropValue;

		PropValue.ulPropTag    = 0x370D001F;//PR_ATTACH_LONG_PATHNAME;
		PropValue.Value.lpszW  = (WCHAR*)wzLongPathName;
		hr = lpAttach->SetProps(ulCount, &PropValue, NULL);
		if(SUCCEEDED(hr))
		{
			bRet = TRUE;
		}
	}

	return bRet;
}

BOOL OLUtilities::GetAttachmentLongPathName(CComPtr<Outlook::Attachment> spAttachment, LPWSTR wzLongPathName, int nBufLen)
{
	HRESULT     hr      = S_OK;
	BOOL        bRet    = FALSE;
	CComPtr<IMAPIProp>  lpAttach= 0;

	hr = spAttachment->get_MAPIOBJECT((IUnknown**)&lpAttach);
	if(S_OK==hr && lpAttach)
	{
		ULONG         ulCount=0;
		SPropValue   *pValue;
		SPropTagArray spta; memset(&spta, 0, sizeof(SPropTagArray));
		spta.cValues = 1;
		spta.aulPropTag[0] = 0x370D001F;//PR_ATTACH_LONG_PATHNAME;
		hr = lpAttach->GetProps(&spta, MAPI_UNICODE, &ulCount, &pValue);
		if(S_OK == hr && pValue)
		{
			if(pValue->Value.lpszW) wcsncpy_s(wzLongPathName, nBufLen, pValue->Value.lpszW, _TRUNCATE);
			bRet = TRUE;
		}
	}

	return bRet;
}


BOOL OLUtilities::SetAttachmentDispName(CComPtr<Outlook::Attachment> spAttachment, LPCWSTR wzDispName)
{
	HRESULT	hr = S_OK;
	if(NULL == spAttachment)
	{
		DP((L"SetAttachmentDispName:: Invalid attachment pointer!\n"));
		return FALSE;
	}

	BSTR	bstrDispName = SysAllocString(wzDispName);
	if(bstrDispName)
	{
		hr = spAttachment->put_DisplayName(bstrDispName);
		SysFreeString(bstrDispName);
		if(SUCCEEDED(hr))
		{
			return TRUE;
		}
	}

	return FALSE;
}

BOOL OLUtilities::GetAttachmentDispName(CComPtr<Outlook::Attachment> spAttachment, LPWSTR wzDispName, int nBufLen)
{
	HRESULT	hr = S_OK;
	if(NULL == spAttachment)
		return FALSE;

	BSTR	bstrDispName = 0;;
	hr = spAttachment->get_DisplayName(&bstrDispName);
	if(SUCCEEDED(hr) && bstrDispName)
	{
		wcsncpy_s(wzDispName, nBufLen, bstrDispName, _TRUNCATE);
		SysFreeString(bstrDispName);
		return TRUE;
	}

	return FALSE;
}

BOOL OLUtilities::RemoveAttachment(CComPtr<IDispatch> spMailItem, int nAttachIndex/*base on 0*/)
{
	HRESULT                         hr  = S_OK;
	BOOL                            bRet= FALSE;
	int                             nAttchmentCount = 0;
	CComPtr<Outlook::Attachments>   spAttachments;

	hr = MailItemUtility::get_Attachments(spMailItem,&spAttachments,TRUE);
	if(SUCCEEDED(hr))
	{
		spAttachments->get_Count((long*)&nAttchmentCount);
		if(nAttachIndex < nAttchmentCount)
		{
			hr = spAttachments->Remove(nAttachIndex+1);
			if(SUCCEEDED(hr)) bRet = TRUE;
		}
	}

	return bRet;
}

BOOL OLUtilities::AppendFileLinkToMail(CComPtr<IDispatch> spMailItem, LPCWSTR lpwzText, LPCWSTR lpwzLink)
{
	std::wstring strAppendString = L"<P><A HREF=\"";
	strAppendString.append(lpwzLink);
	strAppendString.append(L"\"> ");
	strAppendString.append(lpwzText);
	strAppendString.append(L" </A>");

	return OLUtilities::AppendMailBody(spMailItem, strAppendString.c_str());
}

HRESULT OLUtilities::AutoWrap(int autoType, VARIANT *pvResult, CComPtr<IDispatch>pDisp, LPOLESTR ptName, int cArgs...)
{
	// Variables used...
	DISPPARAMS  dp          = { NULL, NULL, 0, 0 };
	DISPID      dispidNamed = DISPID_PROPERTYPUT;
	DISPID      dispID;
	HRESULT     hr;
	char        szName[MAX_PATH+1];

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
}

#ifdef XXXXXWSO2K7
HRESULT OLUtilities::CreateWordInstance(LPVOID* pWordInst)
{
	HRESULT hr = CoCreateInstance(Word::CLSID_wordApplication, NULL, CLSCTX_LOCAL_SERVER, Word::IID__wordApplication, pWordInst);
	if(FAILED(hr)) return hr;
	if(NULL == pWordInst) return S_FALSE;
	return hr;
}
HRESULT OLUtilities::CreateExcelInstance(LPVOID* pExcelInst)
{
	HRESULT hr = CoCreateInstance(Excel::CLSID_excelApplication, NULL, CLSCTX_LOCAL_SERVER, Excel::IID__excelApplication, pExcelInst);
	if(FAILED(hr)) return hr;
	if(NULL == pExcelInst) return S_FALSE;
	return hr;
}
HRESULT OLUtilities::CreatePwptInstance(LPVOID* pPwptInst)
{
	HRESULT hr = CoCreateInstance(PPT::CLSID_pptApplication, NULL, CLSCTX_LOCAL_SERVER, PPT::IID__pptApplication, pPwptInst);
	if(FAILED(hr)) return hr;
	if(NULL == pPwptInst) return S_FALSE;
	return hr;
}
#endif // XXXXXWSO2K7
BOOL OLUtilities::IsRTFFile(LPCWSTR pwzFile)
{
	LPCWSTR pSuffix = wcsrchr(pwzFile, L'.');
	if(NULL == pSuffix) return FALSE;

	if(0 == _wcsicmp(pSuffix, L".RTF"))

		return TRUE;

	return FALSE;
}
BOOL OLUtilities::IsExcelTemplate(LPCWSTR pwzFile)
{
	LPCWSTR pSuffix = wcsrchr(pwzFile, L'.');
	if(NULL == pSuffix) return FALSE;

	if(0 == _wcsicmp(pSuffix, L".XLT")
		/*
		Removed by chellee
		*/
		//#ifdef WSO2K7
		||
		0 == _wcsicmp(pSuffix, L".XLTX") ||
		0 == _wcsicmp(pSuffix, L".XLTM")||
//for 2007 File
#if defined(WSO2K13) || defined(WSO2K16)
		0 == _wcsicmp(pSuffix, L".XLS")  ||
#endif
		0 == _wcsicmp(pSuffix, L".XLSX") ||
		0 == _wcsicmp(pSuffix, L".XLSM") ||
		0 == _wcsicmp(pSuffix, L".XLAM") ||
		0 == _wcsicmp(pSuffix, L".XLSB") 
		/*
		Removed by chellee
		//#endif
		*/
		)

		return TRUE;

	return FALSE;
}
BOOL OLUtilities::IsWordFile(LPCWSTR pwzFile)
{
	LPCWSTR pSuffix = wcsrchr(pwzFile, L'.');
	if(NULL == pSuffix) return FALSE;

	if(0 == _wcsicmp(pSuffix, L".DOC") ||
		0 == _wcsicmp(pSuffix, L".DOT")

#ifdef WSO2K7
		|| 0 == _wcsicmp(pSuffix, L".DOCX") ||
		0 == _wcsicmp(pSuffix, L".DOCM") ||
		0 == _wcsicmp(pSuffix, L".DOTX") ||
		0 == _wcsicmp(pSuffix, L".RTF")
#endif

		)
		return TRUE;

	return FALSE;
}
BOOL OLUtilities::IsWordTemplate(LPCWSTR pwzFile)
{
	LPCWSTR pSuffix = wcsrchr(pwzFile, L'.');
	if(NULL == pSuffix) return FALSE;

	if(0 == _wcsicmp(pSuffix, L".DOT")
		/*
		Removed by chellee
		#ifdef WSO2K7
		*/
		|| 0 == _wcsicmp(pSuffix, L".DOTM") ||
		0 == _wcsicmp(pSuffix, L".DOTX")	|| 
// for 2007 File
		0 == _wcsicmp(pSuffix, L".DOCX") ||
		0 == _wcsicmp(pSuffix, L".DOCM") ||
		0 == _wcsicmp(pSuffix, L".RTF")
		/*
		Removed by chellee
		#endif
		*/
		)
		return TRUE;

	return FALSE;
}
BOOL OLUtilities::IsExcelFile(LPCWSTR pwzFile)
{
	LPCWSTR pSuffix = wcsrchr(pwzFile, L'.');
	if(NULL == pSuffix) return FALSE;

	if(0 == _wcsicmp(pSuffix, L".XLS") ||
		0 == _wcsicmp(pSuffix, L".XLT")

#ifdef WSO2K7

		|| 0 == _wcsicmp(pSuffix, L".XLSX") ||
		0 == _wcsicmp(pSuffix, L".XLSM") ||
		0 == _wcsicmp(pSuffix, L".XLSB") ||
		0 == _wcsicmp(pSuffix, L".XLTX") ||
		0 == _wcsicmp(pSuffix, L".XLTM")

#endif

		)
		return TRUE;

	return FALSE;
}
BOOL OLUtilities::IsPwptFile(LPCWSTR pwzFile)
{
	LPCWSTR pSuffix = wcsrchr(pwzFile, L'.');
	if(NULL == pSuffix) return FALSE;

	if(0 == _wcsicmp(pSuffix, L".PPT") ||
		0 == _wcsicmp(pSuffix, L".POT")
#ifdef WSO2K7

		|| 0 == _wcsicmp(pSuffix, L".PPTX") ||
		0 == _wcsicmp(pSuffix, L".PPTM") ||
		0 == _wcsicmp(pSuffix, L".POTX") ||
		0 == _wcsicmp(pSuffix, L".POTM")

#endif

		)
		return TRUE;

	return FALSE;
}
BOOL OLUtilities::IsPwptTemplateFile(LPCWSTR pwzFile)
{
	LPCWSTR pSuffix = wcsrchr(pwzFile, L'.');
	if(NULL == pSuffix) return FALSE;

	if(
		0 == _wcsicmp(pSuffix, L".POT")
		/*
		Removed by chellee
		#ifdef WSO2K7
		*/
		||
		0 == _wcsicmp(pSuffix, L".POTX") ||
		0 == _wcsicmp(pSuffix, L".POTM")
// for 2007 File
		|| 0 == _wcsicmp(pSuffix, L".PPTX") ||
		0 == _wcsicmp(pSuffix, L".PPTM") ||
		0 == _wcsicmp(pSuffix, L".PPAM") ||
		0 == _wcsicmp(pSuffix, L".PPSM")	||
		0 == _wcsicmp(pSuffix, L".PPSX")	||
		0 == _wcsicmp(pSuffix, L".SLDX")	||
		0 == _wcsicmp(pSuffix, L".SLDM")	||
		0 == _wcsicmp(pSuffix, L".THMX")
		/*
		Removed by chellee
		#endif
		*/
		)
		return TRUE;

	return FALSE;
}
/*
added by chellee for the BUG 7563, The pdf should be reattched...
on 26/11/2008: 9:26 in CDC
*/
//-----------------------------------------------------------------
BOOL OLUtilities::IsPDFFile( LPCWSTR pwzFile ) 
{
   LPCWSTR pSuffix = wcsrchr(pwzFile, L'.');
	if(NULL == pSuffix) return FALSE;
	
	if(	0 == _wcsicmp(pSuffix, L".pdf") )
	{
		return TRUE ;
	}
	return FALSE ;
}
//------------------------------------------------------------------
BOOL OLUtilities ::NeedReattachFileType(LPCWSTR pwzFile)
{
	if(OLUtilities::IsRTFFile(pwzFile)||	 
		OLUtilities::IsExcelTemplate(pwzFile)||
		OLUtilities::IsPwptTemplateFile(pwzFile)||
		OLUtilities::IsWordTemplate(pwzFile) ||
		OLUtilities::IsPDFFile( pwzFile ))
		return TRUE;
	return FALSE;

}

#ifdef XXXXXWSO2K7
Word::_Document*    OLUtilities::OpenWordDoc(Word::_wordApplication** ppWdApp, LPCWSTR pwzFile)
{
	HRESULT				hr = S_OK;
	CComPtr<Word::_Document>	spWdDoc = NULL;
	CComPtr<Word::Documents>    spDocuments  = NULL;
	if(NULL == ppWdApp) return NULL;

	if(NULL == *ppWdApp)
	{
		hr = OLUtilities::CreateWordInstance((void**)ppWdApp);
		if(FAILED(hr) || NULL==*ppWdApp)
			return NULL;
	}

	hr = (*ppWdApp)->get_Documents(&spDocuments);
	if(SUCCEEDED(hr) && spDocuments)
	{
		CComVariant FileName(pwzFile);
		CComVariant varZero((short)0);
		CComVariant covOptional((long)DISP_E_PARAMNOTFOUND, VT_ERROR);
		CComVariant varTrue((short)TRUE);
		CComVariant varFalse((short)FALSE);
		hr = spDocuments->Open( &FileName,      // File
			&varFalse,      // ConfirmConversions
			&varFalse,      // ReadOnly
			&varFalse,      // AddToRecentFiles
			&covOptional,   // PasswordDocument
			&covOptional,   // PasswordTemplate
			&varFalse,      // Revert
			&covOptional,   // WritePasswordDocument
			&covOptional,   // WritePasswordTemplate
			&covOptional,   // Format
			&covOptional,   // Encoding
			&varFalse,      // Visible
			&covOptional,   // OpenAndRepair
			&varZero,       // DocumentDirection
			&covOptional,   // NoEncodingDialog
			&covOptional,   // XMLTransform
			&spWdDoc);
		if(FAILED(hr))
		{
			spWdDoc = NULL;
		}
	}

	return spWdDoc;
}
CComPtr<Excel::_Workbook>   OLUtilities::OpenExcelDoc(Excel::_excelApplication** ppExcApp, LPCWSTR pwzFile)
{
	HRESULT				hr = S_OK;
	CComPtr<Excel::_Workbook>	spWbk = NULL;
	CComPtr<Excel::Workbooks>   spWorkbooks  = 0;
	if(NULL == ppExcApp) return NULL;

	if(NULL == *ppExcApp)
	{
		hr = OLUtilities::CreateExcelInstance((void**)ppExcApp);
		if(FAILED(hr) || NULL==ppExcApp)
			return NULL;
	}

	hr = (*ppExcApp)->get_Workbooks(&spWorkbooks);
	if(SUCCEEDED(hr) && spWorkbooks)
	{
		CComBSTR FileName(pwzFile);
		CComVariant covOptional((long)DISP_E_PARAMNOTFOUND, VT_ERROR);
		hr = spWorkbooks->Open( FileName,       // FileName
			covOptional,    // UpdateLinks
			covOptional,    // ReadOnly
			covOptional,    // Format
			covOptional,    // Password
			covOptional,    // WriteResPassword
			covOptional,    // IgnoreReadOnlyRecommended
			covOptional,    // Origin
			covOptional,    // Delimiter
			covOptional,    // Editable
			covOptional,    // Notify
			covOptional,    // Converter
			covOptional,    // AddToMru
			covOptional,    // Local
			covOptional,    // CorruptLoad
			0,
			&spWbk);
		if(FAILED(hr))
		{
			spWbk = NULL;
		}
	}

	return spWbk;
}
CComPtr<PPT::_Presentation> OLUtilities::OpenPwptDoc(PPT::_pptApplication** ppPwptApp, LPCWSTR pwzFile)
{
	HRESULT				hr = S_OK;
	CComPtr<PPT::_Presentation>	spPresent = NULL;
	CComPtr<PPT::Presentations> spPresents  = 0;
	if(NULL == ppPwptApp) return NULL;

	if(NULL == *ppPwptApp)
	{
		hr = OLUtilities::CreatePwptInstance((void**)ppPwptApp);
		if(FAILED(hr) || NULL==*ppPwptApp)
			return NULL;
	}

	hr = (*ppPwptApp)->get_Presentations(&spPresents);
	if(SUCCEEDED(hr) && spPresents)
	{
		CComBSTR    FileName(pwzFile);
		CComVariant covOptional((long)DISP_E_PARAMNOTFOUND, VT_ERROR);
		hr = spPresents->Open( FileName,       // FileName
			msoFalse,    // ReadOnly
			msoFalse,    // Untitled
			msoFalse,    // WithWindow
			&spPresent);
		if(FAILED(hr))
		{
			spPresent = NULL;
		}
	}

	return spPresent;
}
#endif // XXXXXWSO2K7

BOOL OLUtilities::CreateDocumentProp(CComPtr<IDispatch> pProps, LPCWSTR pwzName, LPCWSTR pwzValue)
{
	HRESULT  hr = S_OK;
	VARIANT parm1, parm2, parm3, parm4;
	parm1.vt = VT_BSTR;
	parm1.bstrVal = SysAllocString(pwzName);
	parm2.vt = VT_BOOL;
	parm2.boolVal = false;
	parm3.vt = VT_I4;
	parm3.lVal = msoPropertyTypeString; //msoPropertyTypeNumber = 1
	parm4.vt = VT_BSTR;
	parm4.bstrVal = SysAllocString(pwzValue);

	CComVariant varProp;
	hr = AutoWrap(DISPATCH_METHOD|DISPATCH_PROPERTYGET, &varProp, pProps, L"Add", 4, parm4,parm3, parm2, parm1);
	if(SUCCEEDED(hr) && varProp.pdispVal)
	{
		CComVariant varResult;
		WCHAR wzPropName[MAX_PATH]; memset(&wzPropName, 0, sizeof(wzPropName));
		AutoWrap(DISPATCH_PROPERTYGET, &varResult, varProp.pdispVal, L"Name", 0);
		if(varResult.bstrVal)
			wcsncpy_s(wzPropName, MAX_PATH, varResult.bstrVal, _TRUNCATE);
	}
	SysFreeString(parm1.bstrVal);
	SysFreeString(parm4.bstrVal);

	return SUCCEEDED(hr)?TRUE:FALSE;
}

BOOL OLUtilities::SetDocumentPropValue(CComPtr<IDispatch> pProps, LPCWSTR pwzName, LPCWSTR pwzValue)
{
	HRESULT hr   = S_OK;
	BOOL    bSet = FALSE;

	// Looking for existing prop
	CComPtr<IDispatch>	pProp = GetPropPointer(pProps, pwzName);
	if( pProp )
	{
		VARIANT parm1;
		parm1.vt = VT_BSTR;
		parm1.bstrVal = SysAllocString(pwzValue);
		hr = AutoWrap(DISPATCH_PROPERTYPUT, NULL, pProp, L"Value", 1, parm1);
		SysFreeString(parm1.bstrVal);
	}
	else
	{
		bSet = CreateDocumentProp(pProps, pwzName, pwzValue);
	}

	return bSet;
}

BOOL OLUtilities::GetDocumentPropValue(CComPtr<IDispatch> pProps, LPCWSTR pwzName, LPWSTR pwzValue, int nSize)
{
	BOOL    bGet = FALSE;
	CComVariant result; 
	VARIANT x; x.vt = VT_BSTR; x.bstrVal = ::SysAllocString(pwzName);
	AutoWrap(DISPATCH_PROPERTYGET, &result, pProps, L"Item", 1, x);
	CComPtr<IDispatch> pProp = result.pdispVal;
	SysFreeString(x.bstrVal);

	if(pProp)
	{
	    result.Clear();
		AutoWrap(DISPATCH_PROPERTYGET, &result, pProp, L"Value", 0);
		if(result.bstrVal)
		{
			bGet = TRUE;
			wcsncpy_s(pwzValue, nSize, result.bstrVal, _TRUNCATE);
		}
	}

	return bGet;
}

CComPtr<IDispatch> OLUtilities::GetPropPointer(CComPtr<IDispatch> pCustomProps, LPCWSTR pwzName)
{
	HRESULT hr = S_OK;
	CComVariant result;
	VARIANT x; x.vt = VT_BSTR; x.bstrVal = ::SysAllocString(pwzName);
	hr = AutoWrap(DISPATCH_PROPERTYGET, &result, pCustomProps, L"Item", 1, x);
	return result.pdispVal;
}

void OLUtilities::GetFQDN(LPCWSTR hostname, LPWSTR fqdn, int nSize)
{
	/*******************************************************************************************************
	Since win2k doesn't support GetAddrInfoW, we cant use this on win2k platform. Or it will fail when 
	registers this dll.
	Here, I replace it with API gethostbyname, it works on 2k and xp....

	kevin 2008-4-15
	/*******************************************************************************************************/

	char szHostName[1001] = {0};
	WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK, hostname, (int)wcslen(hostname), szHostName, 1000, NULL, NULL);

	hostent* hostinfo;
	hostinfo = gethostbyname(szHostName);
	if(hostinfo && hostinfo->h_name)
	{
		MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, hostinfo->h_name, (int)strlen(hostinfo->h_name), fqdn, nSize);
		DP((L"[GetFQDN] OK! %s\n", fqdn));
	}
	else
	{
		wcsncpy_s(fqdn, nSize, hostname, _TRUNCATE);
		DP((L"[GetFQDN] Fail! %s\n", hostname));
	}
}

void OLUtilities::ParseStringList(LPCWSTR pwzStringList, LPCWSTR pwzDelimiter,
								  STRINGLIST& vStrings)
{
	const WCHAR* pwzStart = pwzStringList;
	const WCHAR* pwzEnd   = wcsstr(pwzStart, pwzDelimiter);

	do 
	{
		if(NULL==pwzEnd)
		{
			vStrings.push_back(pwzStart);
			break;
		}

		if(pwzEnd != pwzStart)
		{
			std::wstring strTemp(pwzStart, (pwzEnd-pwzStart));
			vStrings.push_back(strTemp);
		}

		pwzStart = ++pwzEnd;
		pwzEnd = wcsstr(pwzStart, pwzDelimiter);
	} while(1);
}

BOOL OLUtilities::IsStringInListIgnoreCase(LPCWSTR pwzStringList,
										   LPCWSTR pwzDelimiter,
										   LPCWSTR pwzString)
{
	STRINGLIST vStrings;
	ParseStringList(pwzStringList, pwzDelimiter, vStrings);

	for (int i = 0; i < (int)vStrings.size(); i++)
	{
		if (_wcsicmp(pwzString, vStrings[i].c_str()) == 0)
		{
			return TRUE;
		}
	}

	return FALSE;
}
BOOL OLUtilities::CheckGetMailItemType(CComPtr<IDispatch> lpDisp, ITEM_TYPE& itemType ) 
{
	BOOL bRet = FALSE ;
	if( lpDisp == NULL )
	{
		return bRet ;
	}
	CComPtr<IDispatch> spCurMailItem = 0;
	HRESULT hr = lpDisp->QueryInterface(Outlook::IID__MailItem, (void**)&spCurMailItem);
	if(SUCCEEDED(hr) && spCurMailItem)
	{
		itemType = MAIL_ITEM ;
		return TRUE ;
	}
	CComPtr<Outlook::_MeetingItem> spCurMeetItem = 0;
	hr = lpDisp->QueryInterface(Outlook::IID__MeetingItem, (void**)&spCurMeetItem);
	if(SUCCEEDED(hr) && spCurMeetItem)
	{
		itemType = MEETING_ITEM ;
		return TRUE ;
	}
	CComPtr<Outlook::_AppointmentItem> spCurAppItem = 0;
	hr = lpDisp->QueryInterface(Outlook::IID__AppointmentItem, (void**)&spCurAppItem);
	if(SUCCEEDED(hr) && spCurAppItem)
	{
		itemType = APPOINTMENT_ITEM ;
		return TRUE ;
	}
	CComPtr<Outlook::_TaskItem> spCurTaskItem = 0;
	hr = lpDisp->QueryInterface(Outlook::IID__TaskItem, (void**)&spCurTaskItem);
	if(SUCCEEDED(hr) && spCurTaskItem)
	{
		itemType = TASK_ITEM ;
		return TRUE ;
	}
	CComPtr<Outlook::_TaskRequestItem> spCurTaskReqItem = 0;
	hr = lpDisp->QueryInterface(Outlook::IID__TaskRequestItem, (void**)&spCurTaskReqItem);
	if(SUCCEEDED(hr) && spCurTaskReqItem)
	{
		itemType = TASK_REQUEST_ITEM ;
		return TRUE ;
	}
	CComPtr<Outlook::_NoteItem> spCurNoteItem = 0;
	hr = lpDisp->QueryInterface(Outlook::IID__NoteItem, (void**)&spCurNoteItem);
	if(SUCCEEDED(hr) && spCurNoteItem)
	{
		itemType = NOTE_ITEM ;
		return TRUE ;
	}
#if ((defined WSO2K16) || (defined WSO2K13) || (defined WSO2K10))
	//add sharing item, bug 42622
	//#ifdef WSO2K7
	CComPtr<Outlook::_SharingItem> spCurSharingItem = 0;
	hr = lpDisp->QueryInterface(Outlook::IID__SharingItem, (void**)&spCurSharingItem);
	if(SUCCEEDED(hr) && spCurSharingItem)
	{
		itemType = SHARE_ITEM;
		return TRUE ;
	}
#endif
	return bRet ;
}
std::wstring OLUtilities::GetCommonComponentsDir()
{
	wchar_t szDir[MAX_PATH] = {0};
	if(NLConfig::ReadKey(L"SOFTWARE\\NextLabs\\CommonLibraries\\InstallDir", szDir, MAX_PATH))
	{
#ifdef _WIN64
		wcsncat_s(szDir, MAX_PATH, L"\\bin64\\", _TRUNCATE);
#else
		wcsncat_s(szDir, MAX_PATH, L"\\bin32\\", _TRUNCATE);
#endif
		return szDir;
	}
	else
	{
		if(NLConfig::ReadKey(L"SOFTWARE\\Wow6432Node\\NextLabs\\CommonLibraries\\InstallDir", szDir, MAX_PATH))
		{
#ifdef _WIN64
			wcsncat_s(szDir, MAX_PATH, L"\\bin64\\", _TRUNCATE);
#else
			wcsncat_s(szDir, MAX_PATH, L"\\bin32\\", _TRUNCATE);
#endif
			return szDir;
		}
	}

	return L"";
}

BOOL OLUtilities::IsExcelFile(const wstring& wstrFileName)
{
	if(boost::algorithm::iends_with(wstrFileName,L"xlsx")  ||
		boost::algorithm::iends_with(wstrFileName,L"xlsm") ||
		boost::algorithm::iends_with(wstrFileName,L"xlsb") ||
		boost::algorithm::iends_with(wstrFileName,L"xls")  ||
		boost::algorithm::iends_with(wstrFileName,L"xltx") ||
		boost::algorithm::iends_with(wstrFileName,L"xltm") ||
		boost::algorithm::iends_with(wstrFileName,L"xlt")  ||
		boost::algorithm::iends_with(wstrFileName,L"xlam") ||
		boost::algorithm::iends_with(wstrFileName,L"xla") )
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}
EMNLOE_OUTLOOKVERSION OLUtilities::GetVersionNumber()
{
#ifdef WSO2K16
	return OUTLOOK_VER_NUM_2016;
#endif
#ifdef WSO2K13
	return OUTLOOK_VER_NUM_2013;
#endif 
#ifdef WSO2K10
	return OUTLOOK_VER_NUM_2010;
#endif 
#ifdef WSO2K7
	return OUTLOOK_VER_NUM_2007;
#endif 
#ifdef WSO2K3
	return OUTLOOK_VER_NUM_2003;
#endif 
	return OUTLOOK_VER_NUM_2013;
}
/*
return: means the key value has been changed from 1 to 0.
*/
BOOL OLUtilities::SetDeleteWhenRespond(int nDWR)
{
	LSTATUS	lResult	= 0;
	HKEY    hKey    = NULL;
	DWORD   dwType  = REG_DWORD;
	DWORD   cbValue = 4;
	DWORD   dwValue = 0;
	BOOL	bRet = FALSE;

	WCHAR wzKeyName[MAX_PATH]={0};
	_snwprintf_s(wzKeyName, MAX_PATH, _TRUNCATE, L"Software\\Microsoft\\Office\\%d.0\\Outlook\\Preferences", GetVersionNumber());
	lResult = RegOpenKeyExW(HKEY_CURRENT_USER, wzKeyName, 0, KEY_ALL_ACCESS, &hKey);
	
	if(ERROR_SUCCESS == lResult)
	{
		lResult = RegQueryValueExW(hKey, L"DeleteWhenRespond", NULL, &dwType, (LPBYTE)&dwValue, &cbValue);
        if( ERROR_SUCCESS == lResult && REG_DWORD==dwType && 4==cbValue)
        {			
			if (nDWR != (int)dwValue)
			{
				lResult = RegSetValueExW(hKey, L"DeleteWhenRespond", 0, REG_DWORD, (LPBYTE)&nDWR, 4);
				if (ERROR_SUCCESS == lResult)
				{
					if (nDWR == 0)
						bRet = TRUE;
				}
			}
        }
		else if(ERROR_FILE_NOT_FOUND == lResult)
		{
			lResult = RegSetValueEx(hKey, L"DeleteWhenRespond", 0, REG_DWORD, (const BYTE*)&dwValue, sizeof(DWORD));
			if (ERROR_SUCCESS == lResult)
				bRet = TRUE;
		}

		RegCloseKey(hKey);
    }
	return bRet;
}

BOOL OLUtilities::ApplyInterActiveNXLEncrypt(_In_ wstring& strExEFullPath,_In_ wstring& strEncryptFile)
{
	wstring strProtectCommand = L" protect \"";
	wstring strCommandLine = strProtectCommand + strEncryptFile + L"\"";
	return ApplyNXLEncrypt(strExEFullPath,strCommandLine,strEncryptFile);
}

BOOL OLUtilities::ApplyAutoNXLEncrypt(_In_ wstring& strExEFullPath,_In_ wstring& strEncryptFile,vector<ENCRYPTTAG>& vetTagInfo)
{
	wstring strProtectCommand = L" protect \"";
	wstring strCommandLine = strProtectCommand + strEncryptFile + L"\"";
	wstring strAddTagCommand = L" /s ";
	strCommandLine += strAddTagCommand;
	wstring strAddTagFlag = L" /t ";
	vector<ENCRYPTTAG>::iterator itor;
	for (itor = vetTagInfo.begin(); itor != vetTagInfo.end(); itor++)
	{
		strCommandLine += strAddTagFlag;
		strCommandLine += L"\"";
		strCommandLine += (*itor).strTagName;
		strCommandLine += L"=";
		strCommandLine += (*itor).strTagValue;
		strCommandLine += L"\"";
	}
	return ApplyNXLEncrypt(strExEFullPath,strCommandLine,strEncryptFile);
}




void OLUtilities::WaitProcess(HANDLE hProcess)
{

	MSG msg;
	while (1)
	{ 
		switch(MsgWaitForMultipleObjects(1,&hProcess,TRUE,50,QS_ALLINPUT))
		{
		case WAIT_OBJECT_0:
			{
				return;
			}
			break;
		case WAIT_TIMEOUT:
			{
				while(PeekMessage(&msg,NULL,0,0,PM_REMOVE))
				{
					// we will deal with all message, except exit oe process message
					if(!(msg.message == WM_SYSCOMMAND || msg.message == WM_LBUTTONDOWN))
					{
						TranslateMessage(&msg);
						DispatchMessage(&msg);
					}
				}
			}
			break;
		default:
			break;
		}
	}
}


BOOL OLUtilities::ApplyNXLEncrypt(_In_ wstring& strExEFullPath, _In_ wstring& strCommandLine,_In_ wstring& strEncryptFile)
{
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory( &si, sizeof(si) );
	si.cb = sizeof(si);
	ZeroMemory( &pi, sizeof(pi) );
	BOOL bSuccess = ::CreateProcess(strExEFullPath.c_str(),
		const_cast<wchar_t*>(strCommandLine.c_str()),
		NULL,
		NULL,
		FALSE,
		0,
		NULL,
		NULL,
		&si,
		&pi);
	if (!bSuccess)
	{
		int nError = ::GetLastError();
		NLPRINT_DEBUGVIEWLOG(L"Create process nxrmconv.exe fail,the exe path is [%s],commandline is [%s] error value is [%d]\n",strExEFullPath.c_str(),strCommandLine.c_str(),nError);
		return FALSE;
	}

	// Wait until process exits.
	WaitProcess( pi.hProcess);
	
	// Close process and thread handles. 
	CloseHandle( pi.hProcess );
	CloseHandle( pi.hThread );

	//check the attach file encrypt? if the file doesn't encrypt, oe thinks user had select cancel button or encrypt fail!
	if (!IsNxlFormat(strEncryptFile.c_str()))
	{
		NLPRINT_DEBUGVIEWLOG(L"The [%s] file encrypt fail!\n",strEncryptFile.c_str());
		bSuccess = FALSE;
	}
	return bSuccess;
}



BOOL OLUtilities::CreateMulDirectory(wstring & strFilePath)
{
	if (strFilePath.empty())
	{
		return FALSE;
	}
	BOOL bRet = PathFileExists(strFilePath.c_str());
	if (!bRet)
	{
		size_t nPos = strFilePath.rfind(L"\\");
		if (nPos != wstring::npos)
		{
			wstring strTempPath = strFilePath.substr(0,nPos);
			if (CreateMulDirectory(strTempPath))
			{
				::CreateDirectory(strFilePath.c_str(),NULL);
			}
		}
		else
		{
			return FALSE;
		}
	}
	return TRUE;
}


std::string OLUtilities::MyWideCharToMultipleByte(const std::wstring & strValue)
{
	int nLen = WideCharToMultiByte(CP_ACP, 0, strValue.c_str(), (int)strValue.length(), NULL, 0, NULL, NULL); 

	char* pBuf = new char[nLen + 1];
	if(!pBuf)
		return "";
	memset(pBuf, 0, nLen +1);
	nLen = WideCharToMultiByte(CP_ACP, 0, strValue.c_str(), (int)strValue.length(), pBuf, nLen, NULL, NULL); 

	std::string strResult(pBuf);

	if(pBuf)
	{
		delete []pBuf;
		pBuf = NULL;
	}
	return strResult;
}

std::wstring OLUtilities::MyMultipleByteToWideChar(const std::string & strValue)
{
	int nLen = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, strValue.c_str(), (int)strValue.length(), NULL, 0 );
	DWORD dError = 0 ;
	if( nLen == 0 )
	{
		dError = ::GetLastError() ;
		if( dError == ERROR_INVALID_FLAGS )
		{
			nLen = MultiByteToWideChar(CP_ACP, 0, strValue.c_str(), (int)strValue.length(), NULL, 0 );
		}
	}
	wchar_t* pBuf = new wchar_t[nLen + 1];
	if(!pBuf)
		return L"";

	memset(pBuf, 0, (nLen + 1) * sizeof(wchar_t));
	MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, strValue.c_str(), (int)strValue.length(), pBuf, nLen);
	dError = ::GetLastError() ;
	if( dError == ERROR_INVALID_FLAGS )
	{
		MultiByteToWideChar(CP_ACP, 0, strValue.c_str(), (int)strValue.length(), pBuf, nLen);
	}
	std::wstring strResult(pBuf);

	if(pBuf)
	{
		delete []pBuf;
		pBuf = NULL;
	}
	return strResult;
}

bool OLUtilities::PreAppendHtml(std::wstring& wstrHtml, const std::wstring& wstrPrepend)
{
	bool bHtmlFormat = true;
	//find body position, and append after <body>
	const wchar_t* szBodyBegin = L"<body";
	const wchar_t* szBodyEnd = L">";

	size_t nPosBodyBegin = wstrHtml.find(szBodyBegin);
	if(nPosBodyBegin != std::wstring::npos)
	{
		size_t nPosBodyEnd = wstrHtml.find(szBodyEnd, nPosBodyBegin);
		if(nPosBodyEnd != std::wstring::npos)
		{
			wstrHtml.insert(nPosBodyEnd + 1, wstrPrepend);
		}
	}
	else
	{
		bHtmlFormat = false;
	}
	return bHtmlFormat;
}

BOOL OLUtilities::IfAppUserInDomain()
{
	static int nAppUserInDomain = -1;
    if(nAppUserInDomain!=-1)
	{
		return nAppUserInDomain;
	}

   //get host name
	const int nHostNameLen = 500;
	char szHostName[nHostNameLen+1] = { 0 };
	int nResult = gethostname(szHostName, nHostNameLen);
	if(nResult == SOCKET_ERROR)
	{
		DP((L"IfAppUserInDomain failed, gethostname failed, last error=0x%x\n", GetLastError()));
		nAppUserInDomain = TRUE;
		return nAppUserInDomain;
	}

	//GetFQDN
	std::string strHostNameA = szHostName;
	std::wstring strHostNameW = OLUtilities::MyMultipleByteToWideChar(strHostNameA);
	const int nFQDNLen = 1000;
	wchar_t szFQDN[nFQDNLen+1] = {0};
	OLUtilities::GetFQDN(strHostNameW.c_str(), szFQDN, nFQDNLen);
	if(szFQDN[0]==0)
	{
		DP((L"IfAppUserInDomain failed, GetFQDN failed, last error=0x%x\n", GetLastError()));
		nAppUserInDomain = TRUE;
		return nAppUserInDomain;
	}
	std::transform(szFQDN,szFQDN + wcslen(szFQDN),szFQDN,::tolower);
	DP((L"IfAppUserInDomain GetFQDN success: fqdn=%s\n", szFQDN));

	const wchar_t* pDomainSplit = L".";
	if(wcsstr(szFQDN,pDomainSplit)==NULL)
	{
        //if the fqdn didn't have format: AAA.BBB.CCC, the mechine is not join a domain
		nAppUserInDomain = FALSE;
		return nAppUserInDomain;
	}
	//if the mechine joined a domain, we need check if the Current user a domain user or a local user
	std::wstring wstrAppUserName;
	std::wstring wstrAppUserDomain;
	BOOL bResult = GetAppUserInfo(wstrAppUserName, wstrAppUserDomain);
	if(bResult)
	{
		std::transform(wstrAppUserDomain.begin(),wstrAppUserDomain.end(),wstrAppUserDomain.begin(),::tolower);
		DP((L"GetAppUserInfo wstrAppUserDomain=%s\n", wstrAppUserDomain.c_str()));
		std::wstring strDomainFindKey = L".";
		strDomainFindKey += wstrAppUserDomain;
		nAppUserInDomain = (wcsstr(szFQDN, strDomainFindKey.c_str())  != NULL);
		DP((L"App user in Domain:%s.\n", nAppUserInDomain ? L"True" : L"False"));
		return nAppUserInDomain;
	}
   
	return TRUE;
}

BOOL OLUtilities::GetAppUserInfo(std::wstring& strUserName, std::wstring& strUserDomain)
{
	//open process token
	HANDLE hProcessToken = NULL;
	BOOL bOpenToken = OpenProcessToken(GetCurrentProcess(), TOKEN_READ,  &hProcessToken);
	if ((!bOpenToken) || (NULL == hProcessToken))
	{
		DP((L"Open ThreadToken failed, last error=0x%x\n", GetLastError()));
		return false;
	}

	//get user information from token
	DWORD dwLen = 0;
	void* pTokenInfo = NULL;
	BOOL bGetTokenInfo = GetTokenInformation(hProcessToken, TokenUser, NULL, 0, &dwLen);
	if(0==dwLen)
	{
	   CloseHandle(hProcessToken);
	   DP((L"GetTokenInformation failed 1, last error=0x%x\n", GetLastError()));
	   return false;
	}

	pTokenInfo = new BYTE[dwLen];
	memset(pTokenInfo, 0, dwLen);
	bGetTokenInfo = GetTokenInformation(hProcessToken, TokenUser, pTokenInfo, dwLen, &dwLen);
	if (!bGetTokenInfo)
	{
		DP((L"GetTokenInformation failed 2, last error=0x%x\n", GetLastError()));
		CloseHandle(hProcessToken);
		delete[] pTokenInfo;
		return false;
	}
	TOKEN_USER* pTokenUser = (TOKEN_USER*)pTokenInfo;
	

	//get user infomation
	wchar_t userName[100] = { 0 };
	DWORD dwUserNameLen = 100;
	wchar_t domain[1000] = { 0 };
	DWORD dwDomainLen = 1000;
	SID_NAME_USE nameUse;
	BOOL bGetAccount = LookupAccountSidW(NULL, pTokenUser->User.Sid, userName, &dwUserNameLen, domain, &dwDomainLen, &nameUse);
    strUserName = userName;
	strUserDomain = domain;

	//clean
	CloseHandle(hProcessToken);
	delete[] pTokenInfo;
	pTokenInfo = NULL;

	return bGetAccount;
	
}

wstring OLUtilities::newGUID()
{
    wchar_t wszGuid[65] = {0};
    GUID guid = {0};
    HRESULT hr = ::CoCreateGuid(&guid);
    if (SUCCEEDED(hr))
    {
        swprintf_s(wszGuid, 64, L"{%08X-%04X-%04x-%02X%02X-%02X%02X%02X%02X%02X%02X}"
            , guid.Data1
            , guid.Data2
            , guid.Data3
            , guid.Data4[0], guid.Data4[1]
            , guid.Data4[2], guid.Data4[3], guid.Data4[4], guid.Data4[5]
            , guid.Data4[6], guid.Data4[7]
        );
        return wszGuid;
    }
    return L"";
}

wstring OLUtilities::NLGetLongFilePath(const wstring& kwstrFileShortPath)
{
    /** check parameters */
    if (kwstrFileShortPath.empty())
    {
        return L"";
    }

    /** Get long file path */
    wstring wstrFileLongPath(L"");

    wchar_t* pwchLongTempPath = new wchar_t[1024];
    memset(pwchLongTempPath, 0, 1024);
    DWORD dwBufferLength = GetLongPathNameW(kwstrFileShortPath.c_str(), pwchLongTempPath, 1023);
    if (1023 < dwBufferLength)
    {
        /** the buffer is too small and get long path again */
        delete[] pwchLongTempPath;

        pwchLongTempPath = new wchar_t[dwBufferLength + 1];
        GetLongPathNameW(kwstrFileShortPath.c_str(), pwchLongTempPath, dwBufferLength);
    }

    if (NULL != pwchLongTempPath)
    {
        wstrFileLongPath = pwchLongTempPath;
        delete[] pwchLongTempPath;
    }

    return wstrFileLongPath;
}

wstring OLUtilities::NLGetSysTempFilePath(const BOOL kbGetLongPath)
{
    wstring wstrTempFilePath(L"");

    wchar_t wszTempFilePath[1024] = { 0 };
    DWORD dwRet = GetTempPath(1023, wszTempFilePath);
    if (0 != dwRet)
    {
        /** Get long temp path */
        if (kbGetLongPath)
        {
            wstrTempFilePath = NLGetLongFilePath(wszTempFilePath);
            if (wstrTempFilePath.empty())
            {
                wstrTempFilePath = wszTempFilePath;
            }
        }
        else
        {
            wstrTempFilePath = wszTempFilePath;
        }
    }
    return wstrTempFilePath;
}

wstring OLUtilities::CreateTempFolder(BOOL bGetLongPath)
{
    BOOL bCreate = FALSE;
    wstring wstrFolder = NLGetSysTempFilePath(bGetLongPath);
    if (!wstrFolder.empty())
    {
        SetLastError(ERROR_SUCCESS);
        wstrFolder += CE_SYSTEMFOLDER;
        wstrFolder += L"\\";
        if(CreateDirectoryW(wstrFolder.c_str(), NULL) || (ERROR_ALREADY_EXISTS == GetLastError()))
        {
            wstrFolder += OLUtilities::newGUID() + L"\\";
            bCreate = CreateDirectoryW(wstrFolder.c_str(), NULL);
        }
    }
    return bCreate ? wstrFolder : L"";
}

wstring OLUtilities::CreateOETempFolder(BOOL bGetLongPath)
{
	BOOL bCreate = FALSE;
	wstring wstrFolder = NLGetSysTempFilePath(bGetLongPath);
	if (!wstrFolder.empty())
	{
		wstrFolder += CE_SYSTEMFOLDER;
		wstrFolder += L'\\';
		if(CreateDirectoryW(wstrFolder.c_str(), NULL) || (ERROR_ALREADY_EXISTS == GetLastError()))
		{
			bCreate = TRUE;
		}
	}
	return bCreate ? wstrFolder : L"";

}

wstring OLUtilities::GetNewFileName(_In_ const wstring& kwstrRealSrc, _In_ const wstring& kwstrTempPath)
{
    if (kwstrRealSrc.empty())
    {
        return kwstrRealSrc;
    }
    CPath cRealSrcPath(kwstrRealSrc);
    wstring wstrDefaultRetFileName = cRealSrcPath.GetFileName();

    if (kwstrTempPath.empty())
    {
        return wstrDefaultRetFileName;
    }
    CPath cTempPath(kwstrTempPath);
    
    if (0 != _wcsicmp(cTempPath.GetFileSuffix().c_str(), cRealSrcPath.GetFileSuffix().c_str()))
    {
        wstrDefaultRetFileName += L"." + cTempPath.GetFileSuffix();
    }
    return wstrDefaultRetFileName;
}

long OLUtilities::GetFolderItemsCount(OlDefaultFolders emFolderType, bool bOnlyGetUnReadItems)
{
    long lItemCount = -1;
    // Using MAPI to check folder.
    if (NULL != g_pOutlookObj)
    {
        // Get outlook application object
		CComPtr<Outlook::_Application> spApp = g_pOutlookObj->get_App();
        if (NULL != spApp)
        {
            CComPtr<_NameSpace> spNameSpace = NULL;
            HRESULT hr = spApp->get_Session(&spNameSpace);

            if ((SUCCEEDED(hr)) && (NULL != spNameSpace))
            {
                CComPtr<MAPIFolder> spFolder = NULL;
                hr = spNameSpace->GetDefaultFolder(emFolderType, &spFolder);

                if ((SUCCEEDED(hr)) && (NULL != spFolder))
                {
                    if (bOnlyGetUnReadItems)
                    {
                        hr = spFolder->get_UnReadItemCount(&lItemCount);
                    }
                    else
                    {
                        CComPtr<_Items> spItems = NULL;
                        hr = spFolder->get_Items(&spItems);

                        if ((SUCCEEDED(hr)) && (NULL != spItems))
                        {
                            hr = spItems->get_Count(&lItemCount);
                        }
                    }

                    if (FAILED(hr))
                    {
                        lItemCount = -1;
                    }
                }
            }
        }
    }
    else
    {
        DP((L"g_pOutlookObj is NULL \n"));
    }
    return lItemCount;
}

BOOL OLUtilities::IsInReAttachFolder(const wstring& kwstrFilePath)
{
    BOOL bRet = FALSE;
    wstring wstrFolder = NLGetSysTempFilePath(FALSE);
    if (!wstrFolder.empty())
    {
        wstrFolder += CE_SYSTEMFOLDER;
        wstrFolder += L"\\";
        bRet = OLUtilities::IsInSpecifiedFolder(kwstrFilePath, wstrFolder);
    }
    return bRet;
}

BOOL OLUtilities::IsInPATempFolder(const wstring& kwstrFilePath)
{
    BOOL bRet = FALSE;
    wstring wstrFolder = NLGetSysTempFilePath(FALSE);
    if (!wstrFolder.empty())
    {
        wstrFolder += L"PA\\";
        bRet = OLUtilities::IsInSpecifiedFolder(kwstrFilePath, wstrFolder);
    }
    return bRet;
}

BOOL OLUtilities::IsInOETempFolder(const wstring& kwstrFilePath)
{
    BOOL bRet = FALSE;
    if (!g_strOETempFolder.empty())
    {
        bRet = OLUtilities::IsInSpecifiedFolder(kwstrFilePath, g_strOETempFolder);
    }
    return bRet;
}

BOOL OLUtilities::IsInSpecifiedFolder(const wstring& kwstrFilePath, const wstring& kwstrSpecifyFolder)
{
    bool bRet = (0 == _wcsnicmp(kwstrFilePath.c_str(), kwstrSpecifyFolder.c_str(), kwstrFilePath.length()));
    if (!bRet)
    {
        // Log path check
        if (!CPath::IsFullPath(kwstrFilePath))
        {
            return FALSE;
        }

        CPath cFilePath(kwstrFilePath);
        wstring wstrFileFolder = cFilePath.GetParentDir();
        if (wstrFileFolder.empty())
        {
            return FALSE;
        }

        if (L'\\' != wstrFileFolder[wstrFileFolder.length() - 1])
        {
            wstrFileFolder += L"\\";
        }

        wstring wstrLongFileFolderPath = NLGetLongFilePath(wstrFileFolder);
        wstring wstrLongFolderPath = NLGetLongFilePath(kwstrSpecifyFolder);
        if (wstrLongFileFolderPath.empty() || wstrLongFolderPath.empty())
        {
            return FALSE;   // folder not exist or not permission to check it.
        }

        wstring wstrLongFilePath = wstrLongFileFolderPath + cFilePath.GetFileName();
        bRet = (0 == _wcsnicmp(wstrLongFolderPath.c_str(), wstrLongFilePath.c_str(), wstrLongFolderPath.length()));
    }
    DP((L"FilePath:[%s] in folder:[%s] is [%s]\n", kwstrFilePath.c_str(), kwstrSpecifyFolder.c_str(), bRet ? L"TRUE" : L"FALSE"));
    return bRet;
}

void  OLUtilities::GetFileLastModifyTime(const WCHAR* wzFileName, WCHAR* wzDateTime, int BufSize)
{
	HANDLE hFile = INVALID_HANDLE_VALUE;
	memset(wzDateTime, 0, BufSize*sizeof(WCHAR));

	hFile = CreateFile(wzFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(INVALID_HANDLE_VALUE != hFile)
	{
		FILETIME ftLastModify;  memset(&ftLastModify, 0, sizeof(FILETIME));
		if(GetFileTime(hFile, NULL, NULL, &ftLastModify))
		{
			FILETIME ftLocalLastModify;  memset(&ftLocalLastModify, 0, sizeof(FILETIME));
			SYSTEMTIME stModifyTime;    memset(&stModifyTime, 0, sizeof(SYSTEMTIME));
			// We should pass UTC time, so don't convert it to local time
			//FileTimeToLocalFileTime(&ftLastModify, &ftLocalLastModify);
			if(FileTimeToSystemTime(&ftLastModify/*ftLocalLastModify*/, &stModifyTime))
			{
				time_t tmModify;
				DP((L"%04d-%02d-%02d %02d:%02d:%02d",
					stModifyTime.wYear,
					stModifyTime.wMonth,
					stModifyTime.wDay,
					stModifyTime.wHour,
					stModifyTime.wMinute,
					stModifyTime.wSecond));
				tmModify = WinTime2JavaTime(&stModifyTime);
				_snwprintf_s(wzDateTime, BufSize, _TRUNCATE, L"%I64d", tmModify);
				DP((L"ModifyString is %s\n", wzDateTime));
			}
		}
		else
		{
			DP((L"[GetFileLastModifyTime] Fail to get file time\n"));
		}
		CloseHandle(hFile);
	}
	else
	{
		DP((L"[GetFileLastModifyTime] Fail to open file '%s'\n    Last error = %d\n", wzFileName, GetLastError()));
	}
}

time_t OLUtilities::WinTime2JavaTime(SYSTEMTIME* pSysTime)
{
	time_t rtTime = 0;
	tm     rtTM;

	rtTM.tm_year = pSysTime->wYear - 1900;
	rtTM.tm_mon  = pSysTime->wMonth - 1;
	rtTM.tm_mday = pSysTime->wDay;
	rtTM.tm_hour = pSysTime->wHour;
	rtTM.tm_min  = pSysTime->wMinute;
	rtTM.tm_sec  = pSysTime->wSecond;
	rtTM.tm_wday = pSysTime->wDayOfWeek;
	rtTM.tm_isdst = -1;     // Let CRT Lib compute whether DST is in effect,
	// assuming US rules for DST.
	rtTime = mktime(&rtTM); // get the second from Jan. 1, 1970

	if (rtTime == (time_t) -1)
	{
		if (pSysTime->wYear <= 1970)
		{
			// Underflow.  Return the lowest number possible.
			rtTime = (time_t) 0;
		}
		else
		{
			// Overflow.  Return the highest number possible.
			rtTime = (time_t) _I64_MAX;
		}
	}
	else
	{
		rtTime*= 1000;          // get millisecond
	}

	return rtTime;
}

wstring OLUtilities::DeleteHtmlFormat(_In_ const wstring &strMsgIn)
{
    wstring strMsgOut = L"";
    bool bInHtml=false;
    for(int i = 0; i < strMsgIn.length(); i++) 
    { 
        if((strMsgIn[i] != '<') && (strMsgIn.c_str()[i] != '>'))
        {
            if(!bInHtml)
            {
                strMsgOut += strMsgIn[i];
            }
        }
        else if(strMsgIn[i] == '<') 
        { 
            bInHtml=true;
        } 
        else if(strMsgIn.c_str()[i]=='>')
        {
            bInHtml=false;
        }
    }
    return strMsgOut;
}
