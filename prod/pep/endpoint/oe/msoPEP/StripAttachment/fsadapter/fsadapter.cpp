// fsadapter.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "fsadapter.h"
#include <stdio.h>
#include <string>
#include "log.h"
#include "adaptercomm.h"
#include "boost/algorithm/string.hpp"
#include <MAPI.h>
#include <MAPIUTIL.H>
#include <MAPIX.H>
#include "WinAD.h"
#pragma comment(lib, "mapi32")
#pragma comment(lib, "atlsd.lib")

#ifdef _MANAGED
#pragma managed(push, off)
#endif

#ifndef  PR_SMTP_ADDRESS
#define  PR_SMTP_ADDRESS 0x39FE001E
#endif
HINSTANCE g_hInstance;
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		{
		g_hInstance=hModule;
		break;
		}
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
    return TRUE;
}

#ifdef _MANAGED
#pragma managed(pop)
#endif

/*
function: Convert to URI format if necessary.
for example:
\\hz-ts02\John tyler to file://hz-ts02/John tyler
c:\John tyler to file://c:/John tyler
*/
static void CheckPath(std::wstring& strPath)
{
	if(strPath.length() < 3)
	{
		return;
	}

	if( (strPath[0] == '\\' && strPath[1] == '\\') ||
		strPath[1] == ':')
	{
		if(strPath[0] == '\\' && strPath[1] == '\\')
		{
			strPath = strPath.substr(2, strPath.length() - 2);
		}

		boost::replace_all(strPath, L"\\", L"/");
		boost::replace_all(strPath, L" ", L"%20");

		strPath = L"file://" + strPath;
	}
}

static HRESULT ExResolveName( IAddrBook* pAddrBook, LPWSTR lpszName, LPADRLIST *lpAdrList )
{
	// NOTE: Callers of this function MUST release lpAdrList when done
	// with it using MAPIFreeBuffer.
	HRESULT   hRes = S_OK;
	LPADRLIST pAdrList = NULL;

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
	hRes = pAddrBook->ResolveName( NULL, MAPI_UNICODE, NULL, pAdrList );
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

void GetSmtpAddressByDisplayName2(IMAPISession* lpSession, LPCWSTR wzDispName, LPWSTR wzSmtpAddress, int maxSize)
{
	HRESULT hr = S_OK;
	LPADRBOOK       lpAdrBook=0;
	IMessage*       lpMessage=0;
	ULONG           ulObjType = 0;
	LPMAILUSER      lpMailUser = 0;
	LPADRLIST       lpAdrList = 0;
	int             i = 0;

	hr = lpSession->OpenAddressBook(0,NULL,0,&lpAdrBook);
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
					IMAPIProp*   lpAddress = 0;
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
							mbstowcs(wzSmtpAddress, pValue->Value.lpszA, maxSize);
							DP((L"Find Smtp address = %s\n", wzSmtpAddress));
						}
						lpAddress->Release();
					}
					break;
				}

			}
			MAPIFreeBuffer(lpAdrList->aEntries[0].rgPropVals);
			MAPIFreeBuffer(lpAdrList);
			lpAdrList = NULL;
		}
		lpAdrBook->Release();
		lpAdrBook = NULL;
	}
}
void GetSmtpAddressByDisplayName(/*Outlook::_olApplication* spApp*/IDispatch*spApp, LPCWSTR wzDispName, LPWSTR wzSmtpAddress, int maxSize)
{
	HRESULT hr = S_OK;
	LPADRBOOK       lpAdrBook=0;
	LPMAPISESSION   lpMapiSession=0;
	IMessage*       lpMessage=0;
	ULONG           ulObjType = 0;
	LPMAILUSER      lpMailUser = 0;
	LPADRLIST       lpAdrList = 0;
	int             i = 0;

	CComVariant varSession;
	hr=AdapterCommon::AutoWrap(DISPATCH_PROPERTYGET,&varSession,spApp,L"Session",0);
	if(S_OK == hr && varSession.pdispVal)
	{
		CComVariant varMAPISession;
		hr=AdapterCommon::AutoWrap(DISPATCH_PROPERTYGET,&varSession,spApp,L"MAPIOBJECT",0);
		if(S_OK==hr && varMAPISession.pdispVal)
		{
			lpMapiSession=(LPMAPISESSION)varMAPISession.pdispVal;
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
							IMAPIProp*   lpAddress = 0;
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
									mbstowcs(wzSmtpAddress, pValue->Value.lpszA, maxSize);
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
				lpAdrBook->Release();
				lpAdrBook = NULL;
			}
			lpMapiSession = NULL;
		}
	}
}

void GetSmtpAddressFromOfficeContact(IDispatch* spApp, LPCWSTR wzDispName, LPWSTR wzSmtpAddress, int maxSize)
{
	HRESULT	hr = S_OK;
	CComVariant nsName=L"MAPI";

	CComVariant varNameSpace;
	hr=AdapterCommon::AutoWrap(DISPATCH_METHOD,&varNameSpace,spApp,L"GetNamespace",1,nsName);

	if( FAILED(hr) || varNameSpace.pdispVal == NULL )
	{
		return;
	}

	CComVariant varFolder;
	CComVariant varFolderType=10;//olFolderContacts

	hr=AdapterCommon::AutoWrap(DISPATCH_METHOD,&varFolder,varNameSpace.pdispVal,L"GetDefaultFolder",1,varFolderType);
	if( FAILED(hr) || varFolder.pdispVal == NULL )
	{
		return;
	}

	CComVariant varContactItems;
	hr=AdapterCommon::AutoWrap(DISPATCH_PROPERTYGET,&varContactItems,varFolder.pdispVal,L"Items",0);
	if( SUCCEEDED(hr) && varContactItems.pdispVal != NULL )
	{
		/* EmailX attributes count [1,3] */
		for( int i = 1 ; i < 4 ; i++ )
		{
			CComVariant varContactItem;
			/* Query the contacts with the email display name to fine the contact item. */
			WCHAR query[512];
			swprintf(query,sizeof(query),
				L"[Email%dDisplayName] = \"%s\"",i,wzDispName);
			CComVariant varQuery=query;
			hr=AdapterCommon::AutoWrap(DISPATCH_METHOD,&varContactItem,varContactItems.pdispVal,L"Find",1,varQuery);
			if( FAILED(hr) || varContactItem.pdispVal == NULL )
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
			WCHAR email_display_name[512];    /* EmailXDisplayName */
			WCHAR email_address[512];         /* EmailXAddress */
			WCHAR email_address_type[512];    /* EmailXAddressType */

			swprintf(email_display_name,sizeof(email_display_name),L"Email%dDisplayName",i); /* EmailXDisplayName */
			swprintf(email_address,sizeof(email_address),L"Email%dAddress",i);               /* EmailXAddress */
			swprintf(email_address_type,sizeof(email_address_type),L"Email%dAddressType",i); /* EmailXAddressType */

			hr = AdapterCommon::AutoWrap(DISPATCH_PROPERTYGET, &varDispName, varContactItem.pdispVal, email_display_name, 0);

			if(SUCCEEDED(hr) && VT_BSTR==varDispName.vt && varDispName.bstrVal && !IsBadReadPtr(varDispName.bstrVal, 2))
			{
				if (0 == wcscmp(varDispName.bstrVal, wzDispName))
				{
					hr = AdapterCommon::AutoWrap(DISPATCH_PROPERTYGET, &varEmailAddress, varContactItem.pdispVal, email_address, 0);
					if(SUCCEEDED(hr) && VT_BSTR==varEmailAddress.vt && varEmailAddress.bstrVal && !IsBadReadPtr(varEmailAddress.bstrVal, 2))
					{
						hr = AdapterCommon::AutoWrap(DISPATCH_PROPERTYGET, &varEmailAddressType, varContactItem.pdispVal, email_address_type, 0);
						if(SUCCEEDED(hr) && VT_BSTR==varEmailAddressType.vt && varEmailAddressType.bstrVal && !IsBadReadPtr(varEmailAddressType.bstrVal, 2))
						{
							if(0 == _wcsicmp(L"SMTP", varEmailAddressType.bstrVal))
							{
								wcsncpy(wzSmtpAddress, varEmailAddress.bstrVal, maxSize);
							}
							else if(0 == _wcsicmp(L"EX", varEmailAddressType.bstrVal))
							{
								GetSmtpAddressByDisplayName(spApp, varDispName.bstrVal, wzSmtpAddress, maxSize);
							}
						}
					}
				}
			}
		}/* for i */
	}
}
//
// Note: Since this is a recursive function, don't use big local variables so
// that we won't overflow the stack.  Allocate all your big variables on the
// heap instead.
//
void ExpandAddressEntry(IDispatch* spAddrEntry,
						std::wstring& strAddress,bool bDisplayName=false)
{
	HRESULT                             hr = S_OK;
	CComPtr<IDispatch>    spMemberAddrEntries = 0;
	
	CComVariant varName;
	hr=AdapterCommon::AutoWrap(DISPATCH_PROPERTYGET,&varName,spAddrEntry,L"Name",0);
	if(bDisplayName == true)
	{
		strAddress = varName.bstrVal;
		return;
	}


	//OlDisplayType olDispType = olUser;
	//hr = spAddrEntry->get_DisplayType(&olDispType);

	// Only expand private distribution list.  Do not expand global
	// distribution list.
	//if(olPrivateDistList==olDispType)
	//{
	//	hr = spAddrEntry->get_Members(&spMemberAddrEntries);

	//	if(S_OK==hr && spMemberAddrEntries)
	//	{
	//		long nMemberCount;

	//		hr = spMemberAddrEntries->get_Count(&nMemberCount);
	//		doIndent(debugIndentLevel);
	//		DP((L"nMemberCount=%ld\n", nMemberCount));

	//		for(long i=0; i<nMemberCount; i++)
	//		{
	//			CComPtr<Outlook::AddressEntry> spMemberAddrEntry;
	//			VARIANT vi; vi.vt = VT_INT; vi.intVal = i+1;

	//			hr = spMemberAddrEntries->Item(vi, &spMemberAddrEntry);

	//			if(S_OK==hr && spMemberAddrEntry)
	//			{
	//				// Recurse.
	//				ExpandAddressEntry(spMemberAddrEntry, listRecipients,
	//					debugIndentLevel + 1);
	//			}
	//		}
	//	}
	//}
	//else
	{
		wchar_t* wzSmtpAddress = new wchar_t[256];
		CComPtr<IDispatch>	spApp = NULL;

		wzSmtpAddress[0] = L'\0';

		{
			CComVariant varAddress;
			CComVariant varType;
			hr=AdapterCommon::AutoWrap(DISPATCH_PROPERTYGET,&varType,spAddrEntry,L"Type",0);
			if(SUCCEEDED(hr))
			{
				if(_wcsnicmp(varType.bstrVal,L"SMTP",4)==0)
				{
					hr=AdapterCommon::AutoWrap(DISPATCH_PROPERTYGET,&varAddress,spAddrEntry,L"Address",0);
					if(SUCCEEDED(hr)&&varAddress.bstrVal)
					{
						if(wcsrchr(varAddress.bstrVal, L'@'))
							wcscpy_s(wzSmtpAddress,255,varAddress.bstrVal);

					}
				}
				/*else if(_wcsnicmp(bstrType, L"EX",2)==0)
				{
					BSTR bstrName;
					spAddrEntry->get_Name(&bstrName);
					::SysFreeString(bstrName);
					spAddrEntry->get_Address(&bstrAddress);
					spAddrEntry->Details();
				}*/
				//::SysFreeString(bstrType);
			}
			if(wzSmtpAddress[0]==0)
			{
				CComPtr<IMAPISession> pSession = NULL;
				HRESULT hr = MAPILogonEx(NULL,NULL,NULL,MAPI_ALLOW_OTHERS,&pSession);
				if(pSession)
				{
					DP((L"OLUtilities::ExpandAddressEntry  Succeeed to get MAPI sesstion\n"));
					GetSmtpAddressByDisplayName2(pSession, varName.bstrVal, wzSmtpAddress, 256);
					pSession->Logoff(NULL,NULL,NULL);
				}
				else
				{
					DP((L"OLUtilities::ExpandAddressEntry  Fail to get MAPI sesstion\n"));
				}
			}
		}

		if(0 == wzSmtpAddress[0])
		{
			CComVariant varAddress;
			CComVariant varType;
			hr=AdapterCommon::AutoWrap(DISPATCH_PROPERTYGET,&varType,spAddrEntry,L"Type",0);
			if(SUCCEEDED(hr))
			{
				if(_wcsnicmp(varType.bstrVal,L"SMTP",4)==0)
				{
					hr=AdapterCommon::AutoWrap(DISPATCH_PROPERTYGET,&varAddress,spAddrEntry,L"Address",0);
					if(SUCCEEDED(hr))
					{
						if(wcsrchr(varAddress.bstrVal, L'@'))
							wcscpy_s(wzSmtpAddress,255,varAddress.bstrVal);
					}
				}
			}

			if(0==wzSmtpAddress[0])
			{
				CComVariant varApp;
				hr=AdapterCommon::AutoWrap(DISPATCH_PROPERTYGET,&varApp,spAddrEntry,L"Application",0);
				if(SUCCEEDED(hr) && varApp.pdispVal)
				{
					OutputDebugString(L"Get email address by Office Contact!");
					GetSmtpAddressFromOfficeContact(varApp.pdispVal, varName.bstrVal, wzSmtpAddress, 256);				
				}
			}
		}

		if(0 ==wzSmtpAddress[0]||wcsrchr(wzSmtpAddress, L'@')==0)
		{
			OutputDebugString(L"Get email address by AD!");
			CWinAD theWinAD;
			std::wstring strKeyWord = L"(displayName=";
			wchar_t wsuser[256]=L"\0";
			if(SUCCEEDED(hr)&&varName.bstrVal)
			{
				strKeyWord += varName.bstrVal;
				strKeyWord += L")";
				std::wstring strEmailAddr,strSid;
				if(!theWinAD.SearchUserInfo(strEmailAddr,strSid,strKeyWord.c_str()))
				{
					OutputDebugString(L"Lookup the user SID and Email failed!");
				}
				else
				{
					wcscpy_s(wzSmtpAddress,255,strEmailAddr.c_str());
				}
			}
		}

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
			strAddress=pAddr;
			DP((L"wzSmtpAddress=%s\n", pAddr));
		}
		else
		{
			if(varName.bstrVal[0]==L'\'')
			{
				USES_CONVERSION;
				LPWSTR pAddr = W2OLE(varName.bstrVal);
				pAddr++;
				int len=(int)wcslen(pAddr);
				if(len>0&&pAddr[len-1]==L'\'')
					pAddr[len-1]=0;
				SysReAllocString(&varName.bstrVal,pAddr);
			}
			strAddress=varName.bstrVal;
			DP((L"wzSmtpAddress=%s\n", varName.bstrVal));
		}

		delete []wzSmtpAddress;
	}
}

static BOOL GetSenderName(IDispatch*pMailItem,std::wstring&strSenderName)
{
	HRESULT hr = S_OK;

	if(0 == pMailItem) return FALSE;
	BOOL bRet=FALSE;
	CComVariant varSession;
	hr=AdapterCommon::AutoWrap(DISPATCH_PROPERTYGET,&varSession,pMailItem,L"Session",0);
	if(SUCCEEDED(hr)&&varSession.pdispVal)
	{
		CComVariant varCurrentUser;
		hr=AdapterCommon::AutoWrap(DISPATCH_PROPERTYGET,&varCurrentUser,varSession.pdispVal,L"CurrentUser",0);
		if(SUCCEEDED(hr)&&varCurrentUser.pdispVal)
		{
			CComVariant varAddressEntry;
			hr=AdapterCommon::AutoWrap(DISPATCH_PROPERTYGET,&varAddressEntry,varCurrentUser.pdispVal,L"AddressEntry",0);
			if(SUCCEEDED(hr)&&varAddressEntry.pdispVal)
			{
				CComVariant varAddr;
				hr=AdapterCommon::AutoWrap(DISPATCH_PROPERTYGET,&varAddr,varAddressEntry.pdispVal,L"Name",0);
				if(SUCCEEDED(hr)&&varAddr.bstrVal)
				{
					strSenderName=varAddr.bstrVal;
					bRet=TRUE;
				}
				else
				{
					strSenderName=L"";
					bRet=FALSE;
				}
			}
		}				
	}
	return bRet;
}
static void GetSenderAddr(IDispatch* pMailItem, std::wstring& strSenderAddr)//wchar_t* wzSenderAddr, int maxSize)
{
	HRESULT hr = S_OK;
	if(0 == pMailItem) return;

	CComVariant varName,varType;
	hr=AdapterCommon::AutoWrap(DISPATCH_PROPERTYGET,&varType,pMailItem,L"SenderEmailType",0);
	if(SUCCEEDED(hr) && varType.bstrVal)
	{
		if(0 ==  _wcsicmp(varType.bstrVal, L"SMTP"))
		{
			hr=AdapterCommon::AutoWrap(DISPATCH_PROPERTYGET,&varName,pMailItem,L"SenderEmailAddress",0);
			if(SUCCEEDED(hr))
			{
				strSenderAddr=varName.bstrVal;
			}
		}
		else if(0 == _wcsicmp(varType.bstrVal, L"EX"))
		{
			LPADRBOOK       lpAdrBook=0;
			LPMAPISESSION   lpMapiSession=0;
			IMessage*       lpMessage=0;
			ULONG           ulObjType = 0;
			LPMAILUSER      lpMailUser = 0;

			ULONG       ulCount = 0;
			SPropValue  *pValue;
			SPropTagArray spta; memset(&spta, 0, sizeof(SPropTagArray));

			CComVariant varSession;
			hr=AdapterCommon::AutoWrap(DISPATCH_PROPERTYGET,&varSession,pMailItem,L"Session",0);
			if(S_OK == hr && varSession.pdispVal)
			{
				CComVariant varMAPISession;
				hr=AdapterCommon::AutoWrap(DISPATCH_PROPERTYGET,&varMAPISession,varSession.pdispVal,L"MAPIOBJECT",0);
				if(S_OK==hr && varMAPISession.pdispVal)
				{
					lpMapiSession=(LPMAPISESSION)varMAPISession.pdispVal;
					hr = lpMapiSession->OpenAddressBook(0,NULL,0,&lpAdrBook);
					if(S_OK==hr && lpAdrBook)
					{
						CComVariant varMessage;
						hr=AdapterCommon::AutoWrap(DISPATCH_PROPERTYGET,&varMessage,pMailItem,L"MAPIOBJECT",0);
						if(S_OK == hr&& varMessage.pdispVal)
						{
							lpMessage=(IMessage*)varMessage.pdispVal;
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
									if(S_OK == hr&& pValue)
									{
										WCHAR wzSmtpAddress[1024]; 
										memset(wzSmtpAddress, 0, sizeof(wzSmtpAddress));
										mbstowcs(wzSmtpAddress, pValue->Value.lpszA, 1023);
										strSenderAddr=wzSmtpAddress;
									}
									lpMailUser->Release();
								}
							}
						}
						lpAdrBook->Release();
					}
				}
			}
		//	else
		//	{
		//		hr = spMailItem->get_SenderName(&bstrName);
		//		if(SUCCEEDED(hr))
		//		{
		//			strSenderAddr=wzSenderAddr;//wcsncpy(wzSenderAddr, bstrName, maxSize);
		//			SysFreeString(bstrName);
		//		}
		//	}
		}
		else
		{
			CComVariant varSession;
			hr=AdapterCommon::AutoWrap(DISPATCH_PROPERTYGET,&varSession,pMailItem,L"Session",0);
			if(SUCCEEDED(hr)&&varSession.pdispVal)
			{
				CComVariant varCurrentUser;
				hr=AdapterCommon::AutoWrap(DISPATCH_PROPERTYGET,&varCurrentUser,varSession.pdispVal,L"CurrentUser",0);
				if(SUCCEEDED(hr)&&varCurrentUser.pdispVal)
				{
					CComVariant varAddressEntry;
					hr=AdapterCommon::AutoWrap(DISPATCH_PROPERTYGET,&varAddressEntry,varCurrentUser.pdispVal,L"AddressEntry",0);
					if(SUCCEEDED(hr)&&varAddressEntry.pdispVal)
						ExpandAddressEntry(varAddressEntry.pdispVal,strSenderAddr);
				}				
			}
		}
	}
}
typedef std::vector<std::wstring>       STRINGLIST;
static int  GetMailRecipients(IDispatch* spMailItem, STRINGLIST& listRecipients)
{
	HRESULT                      hr;
	int nValidCount=0;
	int i = 0;

	CComVariant varRecipients;
	hr=AdapterCommon::AutoWrap(DISPATCH_PROPERTYGET,&varRecipients,spMailItem,L"Recipients",0);
	if(FAILED(hr)||varRecipients.pdispVal==NULL) return 0;

	CComVariant varCount=0;
	hr=AdapterCommon::AutoWrap(DISPATCH_PROPERTYGET,&varCount,varRecipients.pdispVal,L"Count",0);
	for(i=0; i<varCount.lVal; i++)
	{
		VARIANT vi; vi.vt = VT_INT; vi.intVal = i+1;
		CComVariant varIndex=vi;

		CComVariant varRecipient,varAddrEntry;

		hr=AdapterCommon::AutoWrap(DISPATCH_PROPERTYGET,&varRecipient,varRecipients.pdispVal,L"Item",1,varIndex);
		if(FAILED(hr)||varRecipient.pdispVal==NULL)
			continue;

		hr=AdapterCommon::AutoWrap(DISPATCH_PROPERTYGET,&varAddrEntry,varRecipient.pdispVal,L"AddressEntry",0);
		if (S_OK==hr && varAddrEntry.pdispVal)
		{
			std::wstring strAddr;
			ExpandAddressEntry(varAddrEntry.pdispVal, strAddr);
			if(strAddr.length())
			{
				listRecipients.push_back(strAddr);
				nValidCount++;
			}
		}
	}

	return nValidCount ;
}
typedef enum _tagQUERYTYPE
{
	DISPLAY_NAME,
	SMTP_ADDRESS
}QUERYTYPE;
BOOL SetAceForUser(PACL pacl,DWORD accessMask,const WCHAR*wzDisplayName,QUERYTYPE type)
{
	std::wstring strKeyWord ;
	switch(type)
	{
	case SMTP_ADDRESS:
		strKeyWord= L"(mail=";
		break;
	case DISPLAY_NAME:
	default:
		strKeyWord= L"(displayName=";
		break;
	}
	
	wchar_t wsuser[256]=L"\0";
	strKeyWord += wzDisplayName;
	strKeyWord += L")";
	std::wstring strEmailAddr,strSid;
	CWinAD theWinAD;
	if(!theWinAD.SearchUserInfo(strEmailAddr,strSid,strKeyWord.c_str()))
	{
		OutputDebugString(L"Lookup the user SID and Email failed!");
		return FALSE;
	}
	else
	{
		PSID userSid;
		if(::ConvertStringSidToSid(strSid.c_str(),&userSid))
			AddAccessAllowedAce(pacl,ACL_REVISION,accessMask,userSid);
		else
			return FALSE;
	}
	return TRUE;
}
static BOOL GetModuleBaseName(std::wstring& wstrModuleBaseName)
{
	WCHAR wzModuleFileName[MAX_PATH+1];memset(wzModuleFileName,0,sizeof(wzModuleFileName));
	DWORD dwRet=GetModuleFileName(g_hInstance,wzModuleFileName,MAX_PATH);
	
	if(dwRet==0||GetLastError() == ERROR_INSUFFICIENT_BUFFER)
		return S_FALSE;
	std::wstring wstrTemp=wzModuleFileName;
	std::wstring::size_type pos=wstrTemp.rfind(L'/');
	if(pos==std::wstring::npos)
	{
		pos=wstrTemp.rfind(L'\\');
		if(pos==std::wstring::npos)
			return S_FALSE;
	}
	
	wstrModuleBaseName=wstrTemp.substr(0,pos);
	return TRUE;
}

AdapterCommon::Adapter* GetAdapter()
{
	WCHAR wzModuleFileName[MAX_PATH+1];memset(wzModuleFileName,0,sizeof(wzModuleFileName));
	DWORD dwRet=GetModuleFileName(g_hInstance,wzModuleFileName,MAX_PATH);
	
	if(dwRet==0||GetLastError() == ERROR_INSUFFICIENT_BUFFER)
		return NULL;

	std::wstring wstrIniFile;
	if(GetModuleBaseName(wstrIniFile)==FALSE)
		return NULL;

	wstrIniFile +=L"\\";
	wstrIniFile +=FSADAPTER_INI_FILENAME;
	return new AdapterCommon::Adapter(wstrIniFile.c_str(),wzModuleFileName,FSADAPTER_OBLIGATION_NAME);
}
STDAPI DllRegisterServer(void)
{
	AdapterCommon::Adapter* pAdapter=GetAdapter();
	if(pAdapter==NULL)
		return S_FALSE;

	BOOL bRet=pAdapter->Register();

	delete pAdapter;
	if(bRet==FALSE)
		return S_FALSE;
	return S_OK;
}
/////////////////////////////////////////////////////

STDAPI DllUnregisterServer(void)
{
	AdapterCommon::Adapter* pAdapter=GetAdapter();
	if(pAdapter==NULL)
		return S_FALSE;

	BOOL bRet=pAdapter->UnRegister();

	delete pAdapter;
	if(bRet==FALSE)
		return S_FALSE;
	return S_OK;
}
BOOL FileServerAdapter::UploadOne(AdapterCommon::Attachment *pAtt)
{
	HRESULT hr=S_OK;
#ifdef _DEBUG
	{
		AdapterCommon::Obligation fsOb;
		fsOb.SetName(m_pAdapter->GetObligationName().c_str());
		fsOb.AddAttribute(AdapterCommon::Attribute(this->OBLIGATION_ATTRNAME_FILESERVER,L"\\\\lab01-sd01\\fsAdapterShared"));
		/*fsOb.AddAttribute(AdapterCommon::Attribute(this->OBLIGATION_ATTRNAME_USER,L"jjin"));
		fsOb.AddAttribute(AdapterCommon::Attribute(this->OBLIGATION_ATTRNAME_PASSWORD,L"jjin"));
		fsOb.AddAttribute(AdapterCommon::Attribute(this->OBLIGATION_ATTRNAME_DOMAIN,L"nxtcn"));*/
		fsOb.AddAttribute(AdapterCommon::Attribute(this->OBLIGATION_ATTRNAME_LOCATION,OBLIGATION_ATTRVALUE_LOCATION_TOP));
		fsOb.AddAttribute(AdapterCommon::Attribute(this->OBLIGATION_ATTRNAME_TEXT,OBLIGATION_ATTRVALUE_TEXT));
		fsOb.AddAttribute(AdapterCommon::Attribute(this->OBLIGATION_ATTRNAME_LINKFORMAT,OBLIGATION_ATTRVALUE_LINKFORMAT_SHORT));
		pAtt->AddObligation(fsOb);

	}
#endif
	std::wstring wstrFileServer,wstrUser,wstrPasswd,wstrDomain,wstrSrc=pAtt->GetTempPath();
	size_t iIndex=0,obCount=pAtt->Count();
	if(obCount==0)
		return TRUE;
	for(iIndex=0;iIndex<obCount;iIndex++)
	{
		hr=S_OK;
		AdapterCommon::Obligation ob=pAtt->Item((int)iIndex);	
		if(m_pAdapter->GetObligationName()==ob.GetName())
		{
			if(ob.FindAttribute(this->OBLIGATION_ATTRNAME_FILESERVER,wstrFileServer)==false)
				return FALSE;
			/*if(ob.FindAttribute(this->OBLIGATION_ATTRNAME_USER,wstrUser)==false)
				return FALSE;
			if(ob.FindAttribute(this->OBLIGATION_ATTRNAME_PASSWORD,wstrPasswd)==false)
				return FALSE;
			if(ob.FindAttribute(this->OBLIGATION_ATTRNAME_DOMAIN,wstrDomain)==false)
				return FALSE;*/
			
			BOOL bRet=0;
			DWORD dwLE=0;
			/*HANDLE hToken;
			bRet=::LogonUser(wstrUser.c_str(),wstrDomain.c_str(),wstrPasswd.c_str(),LOGON32_LOGON_NEW_CREDENTIALS,LOGON32_PROVIDER_DEFAULT,&hToken);
			
			if(bRet==0||hToken==NULL)
			{
				dwLE=GetLastError();
				return dwLE;
			}

			bRet=::ImpersonateLoggedOnUser(hToken);
			if(bRet==0)
			{
				CloseHandle(hToken);
				dwLE=GetLastError();
				return dwLE;
			}
			else*/
			{
				std::wstring	wstrLocation(OBLIGATION_ATTRVALUE_LOCATION_BOTTOM),
								wstrText(OBLIGATION_ATTRVALUE_TEXT),
								wstrLinkFormat(m_bIsHtml==true?OBLIGATION_ATTRVALUE_LINKFORMAT_LONG:OBLIGATION_ATTRVALUE_LINKFORMAT_SHORT);
				ob.FindAttribute(this->OBLIGATION_ATTRNAME_LOCATION,wstrLocation);
				ob.FindAttribute(this->OBLIGATION_ATTRNAME_TEXT,wstrText);
				ob.FindAttribute(this->OBLIGATION_ATTRNAME_LINKFORMAT,wstrLinkFormat);
				std::wstring wstrSrcNameOnly=pAtt->GetSrcPath();
				std::wstring::size_type pos=wstrSrcNameOnly.rfind(L"\\");
				if(pos != std::wstring::npos)
					wstrSrcNameOnly=wstrSrcNameOnly.substr(pos+1);
				else
				{
					pos=wstrSrc.rfind(L"/");
					if(pos != std::wstring::npos)
						wstrSrcNameOnly=wstrSrcNameOnly.substr(pos+1);
				}
				
				std::wstring strNewFolder=wstrFileServer;
				if(wstrFileServer[wstrFileServer.length()-1]!='\\'||wstrFileServer[wstrFileServer.length()-1]!='/')
					strNewFolder+=L"\\";

				std::wstring strSenderName;
				if(GetSenderName(m_pItem,strSenderName)==FALSE)
					return FALSE;
				strNewFolder+=strSenderName;
				SYSTEMTIME sys;
				GetLocalTime(&sys);
				WCHAR currTime[32]=L"";
				wsprintf(currTime,L"_%4d%02d%02d-%02d%02d%02d-%03d",sys.wYear,sys.wMonth,sys.wDay,sys.wHour,sys.wMinute,sys.wSecond,sys.wMilliseconds);
				strNewFolder+=currTime;
				{
					SECURITY_ATTRIBUTES sa;
					SECURITY_DESCRIPTOR sd;
					
					BYTE aclBuffer[1024];
					PACL pacl=(PACL)&aclBuffer;

					InitializeSecurityDescriptor(&sd,SECURITY_DESCRIPTOR_REVISION);

					InitializeAcl(pacl,sizeof(aclBuffer),ACL_REVISION);

					//Set ACE for the sender
					if(SetAceForUser(pacl,GENERIC_ALL,strSenderName.c_str(),DISPLAY_NAME)==FALSE)
						return FALSE;
					
					//Set ACEs for the recipient(s)
					STRINGLIST listRecipients;
					GetMailRecipients(m_pItem,listRecipients);
					for(int i=0;i<listRecipients.size();i++)
					{
						if(SetAceForUser(pacl,GENERIC_READ,listRecipients[i].c_str(),SMTP_ADDRESS)==FALSE)
							break;
					}
					SetSecurityDescriptorDacl(&sd,TRUE,pacl,FALSE);
					sa.nLength=sizeof(SECURITY_ATTRIBUTES);
					sa.bInheritHandle=FALSE;
					sa.lpSecurityDescriptor=&sd;

					bRet=0;
					if(CreateDirectory(strNewFolder.c_str(),&sa)==0)
						return FALSE;
					else
					{
						strNewFolder+=L"\\";
						strNewFolder+=wstrSrcNameOnly;
						DP((L"Create file: %s\n", strNewFolder.c_str()));
						HANDLE hFile=CreateFile(strNewFolder.c_str(),0,0,&sa,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
						if(hFile!=INVALID_HANDLE_VALUE)
						{
							CloseHandle(hFile);
							pAtt->SetReturnPath((wchar_t*)strNewFolder.c_str());
							bRet=CopyFile(wstrSrc.c_str(),strNewFolder.c_str(),FALSE);
							DP((L"CopyFile, src: %s, dest: %s\n", wstrSrc.c_str(),strNewFolder.c_str()));
							if(bRet==0)
								return FALSE;
						}
						else
							return FALSE;
					}
				}
				dwLE=GetLastError();
				/*CloseHandle(hToken);
				::RevertToSelf();*/

				std::wstring wstrNewBody;
				if(bRet==0)
					return FALSE;
				else
				{
					pAtt->SetStripFlag(true);
					//Replace "[filename]" with the name of the file
					AdapterCommon::StringReplace(wstrText,FSADAPTER_PLACEHOLDER_FILENAME,wstrSrcNameOnly);
					
					std::wstring wstrLink;

					if(this->IsHtmlBody()&&wstrLinkFormat==OBLIGATION_ATTRVALUE_LINKFORMAT_SHORT)
					{
						wstrLink  = L"<a href=\"";
						wstrLink += strNewFolder;
						wstrLink += L"\">";
						wstrLink += wstrSrcNameOnly;
						wstrLink += L"</a>";
					}
					else
					{
						CheckPath(strNewFolder);//Convert to URI 

						wstrLink  = L" ";//L"<";
						wstrLink += strNewFolder;
						wstrLink += L" " ;
						//wstrLink += L">";
					}
					//Replace "[link]" with the return remote path of File Server upload
					AdapterCommon::StringReplace(wstrText,FSADAPTER_PLACEHOLDER_LINK,wstrLink);
				}
					
				std::wstring strNewLine;
				std::wstring wstrNewLine;
				if(this->IsHtmlBody())
					wstrNewLine=L"<br>";
				else
					wstrNewLine=L"\r\n";
				
				//if(FAILED(hr))
				//	return TRUE;
				
				if(wstrLocation==OBLIGATION_ATTRVALUE_LOCATION_TOP)
				{
					m_strTopMessageText+=wstrNewLine;
					m_strTopMessageText+=SEPARATOR_STRING;
					m_strTopMessageText+=wstrNewLine;
					m_strTopMessageText+= wstrText;
				}
				else //Bottom
				{
					m_strBottomMessageText+=wstrNewLine;
					m_strBottomMessageText+=SEPARATOR_STRING;
					m_strBottomMessageText+=wstrNewLine;
					m_strBottomMessageText+= wstrText;

				}
				m_strTopMessageText+=wstrNewLine;
				m_strBottomMessageText+=wstrNewLine;
				/*if(this->IsHtmlBody())
					hr=AdapterCommon::PutStringPropertyToObject(m_pItem,L"HTMLBody",wstrNewBody.c_str());
				else
					hr=AdapterCommon::PutStringPropertyToObject(m_pItem,L"Body",wstrNewBody.c_str());
				if(FAILED(hr))
					return FALSE;
				CComVariant varResult;
				hr=AdapterCommon::AutoWrap(DISPATCH_METHOD,&varResult,m_pItem,L"Save",0);
				if(FAILED(hr))
					return FALSE;*/
				return TRUE;
				
			}
		}
		else
			continue;
		

	}
	
	return TRUE;
	
}
BOOL FileServerAdapter::UploadAll()
{
	size_t iIndex=0,iCount=m_pAttachments->Count();
	BOOL   bFailure=FALSE;
	for(iIndex=0;iIndex<iCount;iIndex++)
	{
		AdapterCommon::Attachment& pAtt=(m_pAttachments->Item(iIndex));
		if(UploadOne(&pAtt)==FALSE)
		{
			DP((L"File Server Adapter:upload %s[%s] failed",pAtt.GetTempPath().c_str(),pAtt.GetSrcPath().c_str()));
			bFailure=TRUE;
		}
		
	}
	HRESULT hr=S_OK;
	std::wstring strMsg,wstrBody,wstrNewBody,strNewLine;
	std::wstring strSpanBegin,strSpanEnd;
	if(this->IsHtmlBody())
	{
		strNewLine=L"<br>";
		strSpanBegin=L"<span style='font-size:11.0pt;font-family:\"Calibri\"'>";
		strSpanEnd=L"</span>";
		hr=AdapterCommon::GetStringPropertyFromObject(m_pItem,L"HTMLBody",wstrBody);
	}
	else
	{
		strNewLine=L"\r\n";
		hr=AdapterCommon::GetStringPropertyFromObject(m_pItem,L"Body",wstrBody);
	}
	if(FAILED(hr))
		return TRUE;

	if(bFailure==TRUE)
	{
		if(this->IsHtmlBody())
		{
			strMsg =strSpanBegin;
			strMsg+=L"<hr>";
			strMsg+=L"One or more attachment(s) failed to upload to the designated storage location.<br>";
			//strMsg+=L"Please make sure the designated storage location is reachable from the sender's host and re-send the attachment(s).<br>";
			strMsg+=L"<hr>";
			strMsg+=strSpanEnd;
		}
		else
		{
			strMsg =SEPARATOR_STRING;
			strMsg+=strNewLine;
			strMsg+=L"One or more attachment(s) failed to upload to the designated storage location.\r\n";
			//strMsg+=L"Please make sure the designated storage location is reachable from the sender's host and re-send the attachment(s).\r\n";
			strMsg+=SEPARATOR_STRING;
			strMsg+=strNewLine;
		}
		//wstrNewBody=strMsg+wstrBody;
	}
	if(m_strTopMessageText.length()>wcslen(SEPARATOR_STRING))
	{
		m_strTopMessageText+=SEPARATOR_STRING;
		m_strTopMessageText+=strNewLine;
	}
	if(m_strBottomMessageText.length()>wcslen(SEPARATOR_STRING))
	{
		m_strBottomMessageText+=SEPARATOR_STRING;
		m_strBottomMessageText+=strNewLine;
	}

	wstrNewBody=strMsg;
	wstrNewBody+=strSpanBegin;
	wstrNewBody+=m_strTopMessageText;
	wstrNewBody+=strSpanEnd;
	wstrNewBody+=wstrBody;
	wstrNewBody+=strSpanBegin;
	wstrNewBody+=m_strBottomMessageText;
	wstrNewBody+=strSpanEnd;

	if(this->IsHtmlBody())
		hr=AdapterCommon::PutStringPropertyToObject(m_pItem,L"HTMLBody",wstrNewBody.c_str());
	else
		hr=AdapterCommon::PutStringPropertyToObject(m_pItem,L"Body",wstrNewBody.c_str());
	if(FAILED(hr))
		return FALSE;
	CComVariant varResult;
	hr=AdapterCommon::AutoWrap(DISPATCH_METHOD,&varResult,m_pItem,L"Save",0);
	if(FAILED(hr))
		return FALSE;
	return TRUE;
}
BOOL FileServerAdapter::UploadAllEx(wchar_t** pwchObligationTopMessage,int &iTopMsgLength,wchar_t** pwchObligationBottomMessage,int &iBottomMsgLength)
{
	size_t iIndex=0,iCount=m_pAttachments->Count();
	BOOL   bFailure=FALSE;
	for(iIndex=0;iIndex<iCount;iIndex++)
	{
		AdapterCommon::Attachment& pAtt=(m_pAttachments->Item(iIndex));
		if(UploadOne(&pAtt)==FALSE)
		{
			wchar_t wstrTempPath[MAX_PATH]={0};  
			GetTempPath(MAX_PATH, wstrTempPath); 
			SYSTEMTIME sys;
			GetLocalTime(&sys);
			WCHAR currTime[32]={0};
			wsprintf(currTime,L"_%4d%02d%02d-%02d%02d%02d-%03d",sys.wYear,sys.wMonth,sys.wDay,sys.wHour,sys.wMinute,sys.wSecond,sys.wMilliseconds);
			wchar_t wstrNewFolder[MAX_PATH]={0};
			swprintf_s(wstrNewFolder,L"%s%s",wstrTempPath,currTime);

			std::wstring wstrSrcNameOnly=pAtt.GetTempPath();
			std::wstring::size_type pos=wstrSrcNameOnly.rfind(L"\\");
			if(pos != std::wstring::npos)
			{
				wstrSrcNameOnly=wstrSrcNameOnly.substr(pos+1);
			}
			else
			{
				pos=pAtt.GetSrcPath().rfind(L"/");
				if(pos != std::wstring::npos)
				{
					wstrSrcNameOnly=wstrSrcNameOnly.substr(pos+1);
				}
			}
			if(CreateDirectory(wstrNewFolder,NULL))
			{
				swprintf_s(wstrNewFolder,L"%s\\%s",wstrNewFolder,wstrSrcNameOnly.c_str());
				if(CopyFile(pAtt.GetTempPath().c_str(),wstrNewFolder,true))
				{
					wchar_t wstrErrorMsg[MAX_PATH]={0};
					swprintf_s(wstrErrorMsg,L"Upload File To File Server Error , Backup Attachment to Folder %s",wstrNewFolder);
					HWND hWindow = ::GetActiveWindow();
					if(NULL==hWindow)
					{
						hWindow=::GetDesktopWindow();
					}
					MessageBoxW(hWindow,wstrErrorMsg , L"FS failed", MB_OK | MB_ICONERROR);
				}
			}
			DP((L"File Server Adapter:upload %s[%s] failed",pAtt.GetTempPath().c_str(),pAtt.GetSrcPath().c_str()));
			bFailure=TRUE;
		}

	}
	HRESULT hr=S_OK;
	std::wstring strMsg,wstrBody,wstrNewBody,strNewLine;
	std::wstring strSpanBegin,strSpanEnd;
	if(this->IsHtmlBody())
	{
		strNewLine=L"<br>";
		strSpanBegin=L"<span style='font-size:11.0pt;font-family:\"Calibri\"'>";
		strSpanEnd=L"</span>";
		hr=AdapterCommon::GetStringPropertyFromObject(m_pItem,L"HTMLBody",wstrBody);
	}
	else
	{
		strNewLine=L"\r\n";
		hr=AdapterCommon::GetStringPropertyFromObject(m_pItem,L"Body",wstrBody);
	}
	if(FAILED(hr))
		return TRUE;

	if(bFailure==TRUE)
	{
		if(this->IsHtmlBody())
		{
			strMsg =strSpanBegin;
			strMsg+=L"<hr>";
			strMsg+=L"One or more attachment(s) failed to upload to the designated storage location.<br>";
			//strMsg+=L"Please make sure the designated storage location is reachable from the sender's host and re-send the attachment(s).<br>";
			strMsg+=L"<hr>";
			strMsg+=strSpanEnd;
		}
		else
		{
			strMsg =SEPARATOR_STRING;
			strMsg+=strNewLine;
			strMsg+=L"One or more attachment(s) failed to upload to the designated storage location.\r\n";
			//strMsg+=L"Please make sure the designated storage location is reachable from the sender's host and re-send the attachment(s).\r\n";
			strMsg+=SEPARATOR_STRING;
			strMsg+=strNewLine;
		}
		//wstrNewBody=strMsg+wstrBody;
		m_strBottomMessageText+=strMsg;
	}
	if(m_strTopMessageText.length()>wcslen(SEPARATOR_STRING))
	{
		m_strTopMessageText+=SEPARATOR_STRING;
		m_strTopMessageText+=strNewLine;
	}
	if(m_strBottomMessageText.length()>wcslen(SEPARATOR_STRING))
	{
		m_strBottomMessageText+=SEPARATOR_STRING;
		m_strBottomMessageText+=strNewLine;
	}
	
	iTopMsgLength= ((strSpanBegin+m_strTopMessageText+strSpanEnd).length())+1;
	iBottomMsgLength=((strSpanBegin+m_strBottomMessageText+strSpanEnd).length())+1;
	*pwchObligationTopMessage=new wchar_t[iTopMsgLength];
	wmemset(*pwchObligationTopMessage, 0, iTopMsgLength);
	*pwchObligationBottomMessage=new wchar_t[iBottomMsgLength];
	wmemset(*pwchObligationBottomMessage,0,iBottomMsgLength);
	swprintf_s(*pwchObligationTopMessage, iTopMsgLength, L"%s", (strSpanBegin+m_strTopMessageText+strSpanEnd).c_str());
	swprintf_s(*pwchObligationBottomMessage, iBottomMsgLength, L"%s", (strSpanBegin+m_strBottomMessageText+strSpanEnd).c_str());
	return TRUE;
}
BOOL FileServerAdapter::Init()
{
	if(m_pItem)
	{
		CComVariant varResult;
		HRESULT hr=AdapterCommon::AutoWrap(DISPATCH_PROPERTYGET,&varResult,m_pItem,L"BodyFormat",0);
		if(SUCCEEDED(hr)&&(varResult.intVal==2||varResult.intVal==3))
			m_bIsHtml=true;
	}

	return TRUE;
}
STDAPI RepositoryUpload(IDispatch*pItem,AdapterCommon::Attachments* pAtts)
{
	HRESULT hr=S_OK;
	if(pAtts->Count()==1&&pAtts->Item(0).GetSrcPath()==L"C:\\No_attachment.ice")
		return S_OK;
	AdapterCommon::Adapter* pAdapter=GetAdapter();
	if(pAdapter==NULL)
	{
		DP((L"Failed to Get Adapter in RepositoryUpload"));
		return S_FALSE;
	}
	
	FileServerAdapter fsAdapter(pItem,pAtts,pAdapter);
	if(fsAdapter.Init()==FALSE)
	{
		DP((L"Failed to Init for File Server adapter"));
		return S_FALSE;
	}
	BOOL bRet=fsAdapter.UploadAll();

	delete pAdapter;
	if(bRet==TRUE)
		return S_OK;
	DP((L"RepositoryUpload of SharePont Upload Adapter failed"));
	return S_FALSE;
}
STDAPI RepositoryUploadEx(IDispatch*pItem,AdapterCommon::Attachments* pAtts,wchar_t** pwchObligationTopMessage,int &iTopMsgLength,wchar_t** pwchObligationBottomMessage,int &iBottomMsgLength)
{
	HRESULT hr=S_OK;
	if(pAtts->Count()==1&&pAtts->Item(0).GetSrcPath()==L"C:\\No_attachment.ice")
		return S_OK;
	AdapterCommon::Adapter* pAdapter=GetAdapter();
	if(pAdapter==NULL)
	{
		DP((L"Failed to Get Adapter in RepositoryUpload"));
		return S_FALSE;
	}

	FileServerAdapter fsAdapter(pItem,pAtts,pAdapter);
	if(fsAdapter.Init()==FALSE)
	{
		DP((L"Failed to Init for File Server adapter"));
		return S_FALSE;
	}
	BOOL bRet=fsAdapter.UploadAllEx(pwchObligationTopMessage,iTopMsgLength,pwchObligationBottomMessage,iBottomMsgLength);

	delete pAdapter;
	if(bRet==TRUE)
		return S_OK;
	DP((L"RepositoryUpload of SharePont Upload Adapter failed"));
	return S_FALSE;
}
STDAPI ReleaseRepositoryUploadExPWCH(wchar_t * pwch,bool bIsArray)
{
	if(pwch!=NULL)
	{
		if(bIsArray)
		{
			delete[] pwch;
		}
		else
		{
			delete pwch;
		}
	}
	return true;
}