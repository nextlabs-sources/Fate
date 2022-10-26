

#include "stdafx.h"
#include <MAPI.h>
#include <MAPIUTIL.H>
#include <MAPIX.H>
#include "olHandler.h"
#include "emailprocess.h"
#include "fileprocess.h"
#include "gpgUtility.h"
#include "../YLIB/security.h"
#include "ftpcli.h"
#ifndef SECURITY_WIN32
#define SECURITY_WIN32
#endif
#include <security.h>
#include <secext.h>
#include "log.h"
#include "../obligation/WinAD.h"

#include "backthread.h"
#include "multiftp.h"
//#include "ftpcli.h"
#pragma comment(lib, "mapi32")
#define  PR_SMTP_ADDRESS 0x39FE001E

extern HINSTANCE   g_hInstance;

static WCHAR* pwzCmdBarName  = L"CE Approval Toolbar";

static HRESULT SendMail(CComPtr<Outlook::_olApplication> spApp)
{
    HRESULT     hr = S_OK;

    CComPtr<IDispatch>    spDisp;
	hr = spApp->CreateItem(/*OlItemType::*/Outlook::olMailItem, &spDisp);
    if(FAILED(hr) || spDisp==0)
        return hr;
    CComQIPtr<Outlook::_MailItem>   spMail(spDisp);
    ATLASSERT(spMail);

    CComBSTR bstrEnailID;
    hr = spMail->get_EntryID(&bstrEnailID);
    if(SUCCEEDED(hr) && bstrEnailID!=0)
    {
        DP((L"E-Mail entry ID = %s\n", bstrEnailID.m_str));
    }

    ApprovalEmail   ae;
    ae.put_RequestType(CE_REQUEST_EMAILATTACHMENT);
    ae.put_RequesterName(L"Gavin Ye");
    ae.put_RequesterAddress(L"gye@bluejungle.com");
    ae.put_ArchiverAddress(L"gye@cn.nextlabs.com");
    ae.put_RequesterSid(L"S-1-5-21-1530755583-2104571598-3515320701-500");
    //ae.put_ApprovalAddress(L"gavin.ye@nextlabs.com");
    ae.put_ApprovalDirectory(L"\\\\saba\\approval\\");
    ae.put_Customer(L"Flextronix Inc.");
    ae.put_CustomerKey(L"\\\\saba\\Flextronix\\key.dat");
    ae.put_EncryptPasswd(L"abcd1234");
    ae.put_Purpose(L"A test for approval");
    ae.add_Recipient(L"jzhuang@nextlabs.com");
    ae.add_Recipient(L"dong.he@cn.nextlabs.com");
    ae.add_Recipient(L"jmjin@cn.nextlabs.com");
    ae.add_Recipient(L"ssong@cn.nextlabs.com");
    ae.add_File(L"c:\\deny\\1.doc", L"\\\\saba\\quarantine\\1.doc");
    ae.add_File(L"c:\\deny\\2.doc", L"\\\\saba\\quarantine\\2.doc");
    ae.add_File(L"c:\\deny\\3.doc", L"\\\\saba\\quarantine\\3.doc");
    ae.add_File(L"c:\\deny\\4.doc", L"\\\\saba\\quarantine\\4.doc");
    ae.add_File(L"c:\\deny\\5.doc", L"\\\\saba\\quarantine\\5.doc");
    const std::wstring& strMail = ae.Compose();
    const std::wstring& strSubject = ae.get_Subject();
    const std::wstring& strTo = ae.get_RequesterAddress();
    const std::wstring& strCC = ae.get_ArchiverAddress();

    CComBSTR varSubject(strSubject.c_str());
    CComBSTR varHTMLBody(strMail.c_str());
    CComBSTR varTo(strTo.c_str());
    CComBSTR varCC(strCC.c_str());
	hr = spMail->put_BodyFormat(/*OlBodyFormat::*/Outlook::olFormatHTML);
    hr = spMail->put_Subject(varSubject);
    hr = spMail->put_HTMLBody(varHTMLBody);
    hr = spMail->put_To(varTo);
    hr = spMail->put_CC(varCC);
    hr = spMail->Send();

    return hr;
}

/************************************************************************/
/* class OlCommandBarButtonHandler                                      */
/************************************************************************/
HRESULT OlCommandBarButtonHandler::OnInvoke(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pdispparams, VARIANT* pvarResult, EXCEPINFO* pexcepinfo, UINT* puArgErr)
{
    HRESULT hr = S_OK;

    if	(pdispparams && DISPATCH_METHOD==wFlags)
    {
        switch	(dispidMember)
        {
        case 0x00000001:    // Click CommandBar Button
            OnClick((CommandBarButton*)(pdispparams->rgvarg[1].pdispVal), (VARIANT_BOOL*)(pdispparams->rgvarg[0].pboolVal));
            break;
        default:
            hr = DISP_E_MEMBERNOTFOUND;
            break;
        }
    }
    else
    {
        hr = DISP_E_PARAMNOTFOUND;
    }

    return hr;
}

void OlCommandBarButtonHandler::OnClick(IN CommandBarButton* Ctrl, IN OUT VARIANT_BOOL* CancelDefault)
{
    HRESULT         hr = S_OK;
    if(NULL==m_spInspector) return;

    CComPtr<IDispatch>  spMailDisp;
    hr = m_spInspector->get_CurrentItem(&spMailDisp);
    if(FAILED(hr) || spMailDisp==0)
        return;

    CComQIPtr<Outlook::_MailItem> spMail(spMailDisp);
    if(spMail == 0)
        return;

    /*CComBSTR    bstrBody;
    hr = spMail->get_HTMLBody(&bstrBody);
    if (FAILED(hr) || bstrBody==0)
        return;

    ApprovalEmail   ae;*/
	BOOL bAeready = m_pae!=NULL;
#ifndef _DEBUG
    if(!bAeready) return;
#endif


    if (BTP_REJECT == m_dwType)
    {
        if(bAeready) OnReject(*m_pae);
		m_spInspector->Close(/*OlInspectorClose::*/Outlook::olDiscard);
    }
    else if (BTP_APPROVE == m_dwType)
    {
        if(bAeready) OnApprove(*m_pae);
		m_spInspector->Close(/*OlInspectorClose::*/Outlook::olSave);
    }
	else if(BTP_UPLOAD == m_dwType)
	{
		if(bAeready) OnUpload(*m_pae);
		m_spInspector->Close(/*OlInspectorClose::*/Outlook::olSave);
	}
#ifdef _DEBUG
    else if (BTP_COSTUB == m_dwType)
    {
        OnCOStub();
		m_spInspector->Close(/*OlInspectorClose::*/Outlook::olDiscard);
    }
#endif
    else
    {
        MessageBoxW(NULL, L"The button type is unknown!", L"Alert", MB_OK);
    }
}

static std::wstring GetCurrentAddress(Outlook::_MailItem* spMail)
{
    std::wstring strAddr = L"";
    HRESULT      hr = S_OK;
    CComPtr<Outlook::Recipients>    spRecipients;
    hr = spMail->get_Recipients(&spRecipients);
    if(SUCCEEDED(hr) && spRecipients)
    {
        long lCount = 0, i=0;
        hr = spRecipients->get_Count(&lCount);
        if(FAILED(hr)) return strAddr;
        for(i=0; i<lCount; i++)
        {
            CComVariant varIndex(i+1);
            CComPtr<Outlook::Recipient> spRecipient;
            hr = spRecipients->Item(varIndex, &spRecipient);
            if (SUCCEEDED(hr) && spRecipient)
            {
                long lType = 0;
                hr = spRecipient->get_Type(&lType);
                if(FAILED(hr)) continue;
				if(Outlook::olTo==lType)
                {
                    CComBSTR bstrAddr;
                    hr = spRecipient->get_Address(&bstrAddr);
                    if(SUCCEEDED(hr) && bstrAddr)
                    {
                        strAddr = bstrAddr;
                        if(NULL != wcsstr(strAddr.c_str(), L"@"))
                            return strAddr;
                    }
                    CComPtr<Outlook::AddressEntry>  spAddrEntry = 0;
                    hr = spRecipient->get_AddressEntry(&spAddrEntry);
                    if(SUCCEEDED(hr) && spAddrEntry)
                    {
                        CComPtr<IMAPIProp>  spAddress = 0;
                        //IMAPIProp*  spAddress=0;
                        hr = spAddrEntry->get_MAPIOBJECT((IUnknown**)&spAddress);
                        if(SUCCEEDED(hr) && spAddress)
                        {
                            ULONG         ulCount=0;
                            SPropValue   *pValue;
                            SPropTagArray spta; memset(&spta, 0, sizeof(SPropTagArray));
                            spta.cValues = 1;
                            spta.aulPropTag[0] = PR_SMTP_ADDRESS;
                            hr = spAddress->GetProps(&spta, MAPI_UNICODE, &ulCount, &pValue);
                            if(SUCCEEDED(hr) && pValue && pValue->Value.lpszA)
                            {
                                size_t  sizeRet = 0;
                                size_t  sizeInWords = MAX_PATH;
                                WCHAR wzSmtpAddress[MAX_PATH]; memset(wzSmtpAddress, 0, sizeof(wzSmtpAddress));
                                if(0 == mbstowcs_s(&sizeRet, wzSmtpAddress, sizeInWords, pValue->Value.lpszA, _TRUNCATE))
                                {
                                    strAddr = wzSmtpAddress;
                                    MAPIFreeBuffer(pValue);
                                    return strAddr;
                                }
                                MAPIFreeBuffer(pValue);
                            }
                            else
                            {
                                if(MAPI_W_ERRORS_RETURNED == hr)
                                {
                                    DP((L"Fail to get Props: err=MAPI_W_ERRORS_RETURNED(0x%08X)\n", hr));
                                }
                                else
                                {
                                    DP((L"Fail to get Props: err=0x%08X\n", hr));
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return strAddr;
}

static std::wstring GetFolder(LPCWSTR pwzPath)
{
    const WCHAR* pwzCursor = pwzPath;
    const WCHAR* pwzPos1   = 0;
    const WCHAR* pwzPos2   = 0;

    while (0 != *pwzCursor)
    {
        if(L'\\' == *pwzCursor)
        {
            pwzPos2 = pwzPos1;
            pwzPos1 = pwzCursor;
        }
        pwzCursor++;
    }

    if(NULL == pwzPos2) return L"";
    std::wstring strFolderName((pwzPos2+1), (pwzPos1-pwzPos2-1));
    return strFolderName;
}

static std::string Wstring2String(const WCHAR* pwzPath)
{
    BOOL    bUsedDefault = FALSE;
    char szPath[MAX_PATH+1];    memset(szPath, 0, sizeof(szPath));
    WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK, pwzPath, -1, szPath, MAX_PATH, "", &bUsedDefault);
    return szPath;
}
static std::wstring String2WString(const char* pszPath)
{
    WCHAR wzPath[MAX_PATH+1];    memset(wzPath, 0, sizeof(wzPath));
    MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, pszPath, -1, wzPath, MAX_PATH);
    return wzPath;
}
HRESULT AutoWrap(int autoType, VARIANT *pvResult, CComPtr<IDispatch> pDisp, LPOLESTR ptName, int cArgs...)
{
	// Variables used...
	DISPPARAMS  dp          = { NULL, NULL, 0, 0 };
	DISPID      dispidNamed = DISPID_PROPERTYPUT;
	DISPID      dispID;
	HRESULT     hr;
	char        szName[256];

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
	hr = pDisp->Invoke(dispID, IID_NULL, LOCALE_SYSTEM_DEFAULT, (WORD)autoType, &dp, pvResult, NULL, NULL);
	if(FAILED(hr))
	{
		return hr;
	}
	delete [] pArgs;
	return hr;
}
HRESULT ExResolveName( IAddrBook* pAddrBook, LPWSTR lpszName, LPADRLIST *lpAdrList )
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
#pragma warning(disable: 6309 6387)
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

void GetSmtpAddressByDisplayName2(CComPtr<IMAPISession> lpSession, LPCWSTR wzDispName, LPWSTR wzSmtpAddress, int maxSize)
{
	HRESULT hr = S_OK;
	CComPtr<IAddrBook>       lpAdrBook=0;
//	IMessage*       lpMessage=0;
	ULONG           ulObjType = 0;
//	LPMAILUSER      lpMailUser = 0;
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
void GetSmtpAddressByDisplayName(CComPtr<Outlook::_olApplication> spApp, LPCWSTR wzDispName, LPWSTR wzSmtpAddress, int maxSize)
{
	HRESULT hr = S_OK;
	CComPtr<IAddrBook>       lpAdrBook=0;
	CComPtr<IMAPISession>   lpMapiSession=0;
//	IMessage*       lpMessage=0;
	ULONG           ulObjType = 0;
	//LPMAILUSER      lpMailUser = 0;
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

void GetSmtpAddressFromOfficeContact(CComPtr<Outlook::_olApplication> spApp, LPCWSTR wzDispName, LPWSTR wzSmtpAddress, int maxSize)
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
	hr = ns->GetDefaultFolder(Outlook::OlDefaultFolders::olFolderContacts, &spFolder);

	if( FAILED(hr) || spFolder == NULL )
	{
		return;
	}

	CComPtr<Outlook::_Items> spContactItems = NULL;
	hr = spFolder->get_Items(&spContactItems);

	if( SUCCEEDED(hr) && spContactItems != NULL )
	{
		/* EmailX attributes count [1,3] */
		for( int i = 1 ; i < 4 ; i++ )
		{
			CComPtr<Outlook::_ContactItem> spContactItem = NULL;

			/* Query the contacts with the email display name to fine the contact item. */
			WCHAR query[512];
			_snwprintf_s(query,sizeof(query), _TRUNCATE,
				L"[Email%dDisplayName] = \"%s\"",i,wzDispName);

			spContactItem = NULL;
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
			WCHAR email_display_name[512];    /* EmailXDisplayName */
			WCHAR email_address[512];         /* EmailXAddress */
			WCHAR email_address_type[512];    /* EmailXAddressType */

			_snwprintf_s(email_display_name,_countof(email_display_name), _TRUNCATE,L"Email%dDisplayName",i); /* EmailXDisplayName */
			_snwprintf_s(email_address,_countof(email_address), _TRUNCATE,L"Email%dAddress",i);               /* EmailXAddress */
			_snwprintf_s(email_address_type,_countof(email_address_type), _TRUNCATE,L"Email%dAddressType",i); /* EmailXAddressType */

			hr = AutoWrap(DISPATCH_PROPERTYGET, &varDispName, spContactItem, email_display_name, 0);

			if(SUCCEEDED(hr) && VT_BSTR==varDispName.vt && varDispName.bstrVal && !IsBadReadPtr(varDispName.bstrVal, 2))
			{
				if (0 == wcscmp(varDispName.bstrVal, wzDispName))
				{
					hr = AutoWrap(DISPATCH_PROPERTYGET, &varEmailAddress, spContactItem, email_address, 0);
					if(SUCCEEDED(hr) && VT_BSTR==varEmailAddress.vt && varEmailAddress.bstrVal && !IsBadReadPtr(varEmailAddress.bstrVal, 2))
					{
						hr = AutoWrap(DISPATCH_PROPERTYGET, &varEmailAddressType, spContactItem, email_address_type, 0);
						if(SUCCEEDED(hr) && VT_BSTR==varEmailAddressType.vt && varEmailAddressType.bstrVal && !IsBadReadPtr(varEmailAddressType.bstrVal, 2))
						{
							if(0 == _wcsicmp(L"SMTP", varEmailAddressType.bstrVal))
							{
								wcsncpy_s(wzSmtpAddress, maxSize, varEmailAddress.bstrVal, _TRUNCATE);
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
void ExpandAddressEntry(CComPtr<Outlook::AddressEntry> spAddrEntry,
						std::wstring& strAddress,bool bDisplayName=false)
{
	HRESULT                             hr = S_OK;
	CComPtr<Outlook::AddressEntries>    spMemberAddrEntries = 0;

	BSTR bstrName;
	hr = spAddrEntry->get_Name(&bstrName);
	if(bDisplayName == true)
	{
		strAddress = bstrName;
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
		CComPtr<Outlook::_olApplication>	spApp = NULL;

		wzSmtpAddress[0] = L'\0';

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
							wcsncpy_s(wzSmtpAddress,255,bstrAddress, _TRUNCATE);
						::SysFreeString(bstrAddress);

					}
				}
				/*else if(wcsnicmp(bstrType, L"EX",2)==0)
				{
					BSTR bstrName;
					spAddrEntry->get_Name(&bstrName);
					::SysFreeString(bstrName);
					spAddrEntry->get_Address(&bstrAddress);
					spAddrEntry->Details();
				}*/
				::SysFreeString(bstrType);
			}
			if(wzSmtpAddress[0]==0)
			{
				CComPtr<IMAPISession> pSession = NULL;
				HRESULT hr = MAPILogonEx(NULL,NULL,NULL,MAPI_ALLOW_OTHERS,&pSession);
				if(pSession)
				{
					DP((L"OLUtilities::ExpandAddressEntry  Succeeed to get MAPI sesstion\n"));
					GetSmtpAddressByDisplayName2(pSession, bstrName, wzSmtpAddress, 256);
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
							wcsncpy_s(wzSmtpAddress,255,bstrAddress, _TRUNCATE);
						::SysFreeString(bstrAddress);
					}
				}
				::SysFreeString(bstrType);
			}

			if(0==wzSmtpAddress[0])
			{
				hr = spAddrEntry->get_olApplication(&spApp);
				if(SUCCEEDED(hr) && spApp)
				{
					DP((L"Get email address by Office Contact!"));
					GetSmtpAddressFromOfficeContact(spApp, bstrName, wzSmtpAddress, 256);				
				}
			}
		}

		if(0 ==wzSmtpAddress[0]||wcsrchr(wzSmtpAddress, L'@')==0)
		{
			DP((L"Get email address by AD!"));
			CWinAD theWinAD;
			//std::wstring strKeyWord = L"(sAMAccountName=";
			std::wstring strKeyWord = L"(displayName=";
//			wchar_t wsuser[256]=L"\0";
			BSTR bstrName_l=NULL;
			hr=spAddrEntry->get_Name(&bstrName_l);
			if(SUCCEEDED(hr)&&bstrName_l)
			{
				strKeyWord += bstrName_l;	//g_BaseArgument.wstrCurUserName;
				::SysFreeString(bstrName_l);
				strKeyWord += L")";
				std::wstring strEmailAddr,strSid;
				if(!theWinAD.SearchUserInfo(strEmailAddr,strSid,strKeyWord.c_str()))
				{
					//std::wstring strMsg = L"Get Requester SID failed by (sAMAccountName = ";
					//strMsg += g_BaseArgument.wstrCurUserName;
					DP((L"Lookup the user SID and Email failed!"));
				}
				else
				{
					wcsncpy_s(wzSmtpAddress,255,strEmailAddr.c_str(), _TRUNCATE);
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
			//listRecipients.push_back(pAddr);
			strAddress=pAddr;
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
			//listRecipients.push_back(bstrName);
			strAddress=bstrName;
			DP((L"wzSmtpAddress=%s\n", bstrName));
		}

		delete []wzSmtpAddress;
	}

	SysFreeString(bstrName);
}
HRESULT GetCurrentUser(Outlook::_olApplication& olApp,std::wstring & strSenderAddress,bool bDisplayName)
{
	HRESULT hr=S_OK;
	CComPtr<Outlook::_NameSpace> nsPtr=NULL;
	hr=olApp.GetNamespace(L"MAPI",&nsPtr);
	if(FAILED(hr)||nsPtr==NULL)
		return S_FALSE;

	CComPtr<Outlook::Recipient> recipientPtr=NULL;
	hr=nsPtr->get_CurrentUser(&recipientPtr);
	if(FAILED(hr)||recipientPtr==NULL)
		return S_FALSE;
	
	CComPtr<Outlook::AddressEntry> addressEntryPtr=NULL;
	hr=recipientPtr->get_AddressEntry(&addressEntryPtr);
	if(FAILED(hr)||addressEntryPtr==NULL)
		return S_FALSE;
	std::wstring strAddress;
	ExpandAddressEntry(addressEntryPtr,strAddress,bDisplayName);
	if(strAddress.length())
	{
		strSenderAddress=strAddress;
		return S_OK;
	}
	else
		return S_FALSE;
	return hr;

}
void OlCommandBarButtonHandler::OnUpload(ApprovalEmail& ae)
{
	
	HRESULT								hr = S_OK;
    ApproveFilesVector					vAFiles;
    FileVector::const_iterator			itf;
    CComPtr<Outlook::_olApplication>	spApp;
    hr = m_spInspector->get_olApplication(&spApp);
    if(FAILED(hr) || spApp==0)
        return;
    CComPtr<IDispatch>              spMailDisp;
    hr = m_spInspector->get_CurrentItem(&spMailDisp);
    if(FAILED(hr) || spMailDisp==0)
        return;
    CComQIPtr<Outlook::_MailItem>   spMail(spMailDisp);
    if(spMail==0)
        return;
 
	//Get the Outlook login account
	std::wstring strSenderAddress;
	if(GetCurrentUser(*spApp,strSenderAddress)==S_FALSE)
		strSenderAddress=L"";

	if(strSenderAddress.length())
	{
		const std::wstring& strRequester = ae.get_RequesterAddress();
		if(0 == _wcsicmp(strRequester.c_str(), strSenderAddress.c_str()))
		{
			MessageBoxW(NULL, L"You can't approve your own request!", L"Error", MB_OK);
#ifndef _DEBUG
			return;
#endif
		}
	}

	if(strSenderAddress.length())
	{
		if(ae.IsValidApprover(strSenderAddress.c_str())==FALSE)
		{
			MessageBoxW(NULL,L"You are not the validate approver for this request!",L"Wrong Approver",MB_OK);
#ifndef _DEBUG
			return ;
#endif
		}
	}
	struct BackThreadParam* pThreadParam=new BackThreadParam;
	if(pThreadParam)
	{
		//marshal Application interface;
		//pThreadParam->pAppOutlook=spApp;
		//pThreadParam->pAppOutlook->AddRef();
		LPSTREAM pStream = NULL;
		hr=CoMarshalInterThreadInterfaceInStream(__uuidof(Outlook::_olApplication), spApp, &pStream);
		if(SUCCEEDED(hr))
			pThreadParam->pAppStream=pStream;
		else
		{
			pThreadParam->pAppStream=NULL;
			MessageBox(NULL,L"Failed to marshal interface for application object. Please try again! \nIf this error keep happening, please contact your administrator!",L"Error",MB_OK);
			return;
		}
		//marshal Item interface
		//pThreadParam->pMailItem=spMail;
		//pThreadParam->pMailItem->AddRef();
		pStream = NULL;
		hr=CoMarshalInterThreadInterfaceInStream(__uuidof(Outlook::_MailItem), spMail, &pStream);
		if(SUCCEEDED(hr))
			pThreadParam->pItemStream=pStream;
		else
		{
			pThreadParam->pItemStream=NULL;
			MessageBox(NULL,L"Failed to marshal interface for Mail Item object. Please try again! \nIf this error keep happening, please contact your administrator!",L"Error",MB_OK);
			return;
		}

		pThreadParam->pae=new ApprovalEmail;
		if(pThreadParam->pae)
		{
			if(m_matchType==CMultiFTPManager::ONE)
			{
				ae.put_Subject(L"Approval request for email documents");
				ae.put_FtpServer(m_pSite->GetFtpServer().c_str());
				ae.put_FtpUser(m_pSite->GetFtpUser().c_str());
				ae.put_FtpPasswd(m_pSite->GetFtpPasswd().c_str());
				std::wstring strBody=ae.Compose();
				CComBSTR varHTMLBody(strBody.c_str());
				hr=spMail->put_HTMLBody(varHTMLBody);
				hr=spMail->Save();
			}
			*(pThreadParam->pae)=ae;
		}
		pThreadParam->ftpSite=*(this->m_pSite);
		pThreadParam->matchType=this->m_matchType;
	}
	
	CLog::WriteLog(L"Info:",L"strat to create thread ZipFtpBackThread");
	if(_beginthreadex(NULL,0,ZipFtpBackThread,pThreadParam,0,NULL)==NULL)
	{
		MessageBox(NULL,L"Failed to create background thread for Zip Encryption and FTP operation. The message windows will be locked for a while. Please wait!",L"Warnning",MB_OK);
	}
}
void OlCommandBarButtonHandler::OnApprove(ApprovalEmail& ae)
{
    HRESULT                           hr = S_OK;
    ApproveFilesVector                vAFiles;
    FileVector::const_iterator        itf;
    CComPtr<Outlook::_olApplication> spApp;
    hr = m_spInspector->get_olApplication(&spApp);
    if(FAILED(hr) || spApp==0)
        return;
    CComPtr<IDispatch>              spMailDisp;
    hr = m_spInspector->get_CurrentItem(&spMailDisp);
    if(FAILED(hr) || spMailDisp==0)
        return;
    CComQIPtr<Outlook::_MailItem>   spMail(spMailDisp);
    if(spMail==0)
        return;
 
	std::wstring strSenderAddress;
	if(GetCurrentUser(*spApp,strSenderAddress)==S_FALSE)
		strSenderAddress=L"";

	if(strSenderAddress.length())
	{
		const std::wstring& strRequester = ae.get_RequesterAddress();
		if(0 == _wcsicmp(strRequester.c_str(), strSenderAddress.c_str()))
		{
			MessageBoxW(NULL, L"You can't approve your own request!", L"Error", MB_OK);
#ifndef _DEBUG
			return;
#endif
		}
	}
	if(strSenderAddress.length())
	{
		if(ae.IsValidApprover(strSenderAddress.c_str())==FALSE)
		{
			MessageBoxW(NULL,L"You are not the validate approver for this request!",L"Wrong Approver",MB_OK);
#ifndef _DEBUG
			return ;
#endif
		}
	}

	struct BackThreadParam* pThreadParam=new BackThreadParam;
	if(pThreadParam)
	{
		//marshal Application interface;
		//pThreadParam->pAppOutlook=spApp;
		//pThreadParam->pAppOutlook->AddRef();
		LPSTREAM pStream = NULL;
		hr=CoMarshalInterThreadInterfaceInStream(__uuidof(Outlook::_olApplication), spApp, &pStream);
		if(SUCCEEDED(hr))
			pThreadParam->pAppStream=pStream;
		else
		{
			pThreadParam->pAppStream=NULL;
			MessageBox(NULL,L"Failed to marshal interface for application object. Please try again! \nIf this error keep happening, please contact your administrator!",L"Error",MB_OK);
			return;
		}
		//marshal Item interface
		//pThreadParam->pMailItem=spMail;
		//pThreadParam->pMailItem->AddRef();
		pStream = NULL;
		hr=CoMarshalInterThreadInterfaceInStream(__uuidof(Outlook::_MailItem), spMail, &pStream);
		if(SUCCEEDED(hr))
			pThreadParam->pItemStream=pStream;
		else
		{
			pThreadParam->pItemStream=NULL;
			MessageBox(NULL,L"Failed to marshal interface for Mail Item object. Please try again! \nIf this error keep happening, please contact your administrator!",L"Error",MB_OK);
			return;
		}

		pThreadParam->pae=new ApprovalEmail;
		if(pThreadParam->pae)
		{
			if(m_matchType==CMultiFTPManager::ONE)
			{
				ae.put_Subject(L"Approval request for email documents");
				ae.put_FtpServer(m_pSite->GetFtpServer().c_str());
				ae.put_FtpUser(m_pSite->GetFtpUser().c_str());
				ae.put_FtpPasswd(m_pSite->GetFtpPasswd().c_str());
				std::wstring strBody=ae.Compose();
				CComBSTR varHTMLBody(strBody.c_str());
				hr=spMail->put_HTMLBody(varHTMLBody);
				hr=spMail->Save();
			}
			*(pThreadParam->pae)=ae;
		}
		pThreadParam->ftpSite=*(this->m_pSite);
		pThreadParam->matchType=this->m_matchType;
	}
	else
	{
		MessageBox(NULL,L"There is critical error happen on your machine. Please contact your administrator!",L"Error",MB_OK);
		return;
	}
	
	CLog::WriteLog(L"Info:",L"strat to create thread ZipFtpBackThread");
	if(_beginthreadex(NULL,0,ZipFtpBackThread,pThreadParam,0,NULL)==NULL)
	{
		MessageBox(NULL,L"Failed to create background thread for Zip Encryption and FTP operation. The message windows will be locked for a while. Please wait!",L"Warnning",MB_OK);
	}
	
	DP((L"OnApprove finished"));
}

void OlCommandBarButtonHandler::OnReject(ApprovalEmail& ae)
{
	CLog::WriteLog(L"The request was rejected",L"");
    HRESULT                           hr = S_OK;
    CComPtr<Outlook::_olApplication>    spApp;
    hr = m_spInspector->get_olApplication(&spApp);
    if (FAILED(hr) || spApp==0)
        return;
    CComPtr<IDispatch>              spMailDisp;
    hr = m_spInspector->get_CurrentItem(&spMailDisp);
    if(FAILED(hr) || spMailDisp==0)
        return;
    CComQIPtr<Outlook::_MailItem>   spMail(spMailDisp);
    if(spMail==0)
        return;
    //const std::wstring& strAprover = ae.get_ApprovalAddress();
    GetCurrentAddress(spMail);
    /*DP((L"OnReject::Aprover:%s, CurrentUser:%s\n", strAprover.c_str(), strCurrent.c_str()));
    if(0 != _wcsicmp(strAprover.c_str(), strCurrent.c_str()))
    {
        MessageBoxW(NULL, L"You can't reject this request because you are not the approver!", L"Warning", MB_OK);
        return;
    }*/

//#ifndef _DEBUG
	/*WCHAR wstrUserName[256];
	ULONG lUserName=sizeof(wstrUserName);
	ZeroMemory(wstrUserName,sizeof(wstrUserName));
	BOOLEAN bName=::GetUserNameEx(NameUserPrincipal,wstrUserName,&lUserName);*/

	std::wstring strSenderAddress;
	if(GetCurrentUser(*spApp,strSenderAddress)==S_FALSE)
		strSenderAddress=L"";

	if(strSenderAddress.length())
	{
		const std::wstring& strRequester = ae.get_RequesterAddress();
		if(0 == _wcsicmp(strRequester.c_str(), strSenderAddress.c_str()))
		{
			MessageBoxW(NULL, L"You can't reject your own request!", L"Error", MB_OK);
#ifndef _DEBUG
			return;
#endif
		}
	}
	if(strSenderAddress.length())
	{
		if(ae.IsValidApprover(strSenderAddress.c_str())==FALSE)
		{
			MessageBoxW(NULL,L"You can't reject this request since you are not the validate approver for this request!",L"Wrong Approver",MB_OK);
			return ;
		}
	}
//#endif

    CComPtr<Outlook::_MailItem> spNewMail;
    CComPtr<IDispatch>          spNewMailDisp;
	hr = spApp->CreateItem(/*OlItemType::*/Outlook::olMailItem, &spNewMailDisp);
    if (SUCCEEDED(hr) && spNewMailDisp)
    {
        hr = spNewMailDisp->QueryInterface(__uuidof(Outlook::_MailItem), (void**)&spNewMail);
        if (SUCCEEDED(hr) && spNewMail)
        {
            const std::wstring& strTo = ae.get_RequesterAddress();
            const std::wstring& strCC = ae.get_ArchiverAddress();
            const std::wstring& strRejectMail =  ComposeRejectMail(ae);
            CComBSTR    bstrSubject(REJECT_EMAIL_SUBJECT);
            CComBSTR    bstrTo(strTo.c_str());
            CComBSTR    bstrCC(strCC.c_str());
            CComBSTR    bstrHTML(strRejectMail.c_str());
            spNewMail->put_Subject(bstrSubject);
            spNewMail->put_To(bstrTo);
            spNewMail->put_CC(bstrCC);
            spNewMail->put_HTMLBody(bstrHTML);
            spNewMail->Display();
        }
    }
}

#ifdef _DEBUG
void OlCommandBarButtonHandler::OnCOStub()
{
    HRESULT                           hr = S_OK;
    CComPtr<Outlook::_olApplication>    spApp;
    hr = m_spInspector->get_olApplication(&spApp);
    if (FAILED(hr) || spApp==0)
        return;

    const std::wstring& strRequestMail =  ComposeRequestMailStub();
    SendMail(spApp, L"gavin.ye@nextlabs.com", L"gye@bluejungle.com", L"Compliant Enforcer Approval Request", strRequestMail.c_str());
}
#endif

HRESULT OlCommandBarButtonHandler::SendMail(CComPtr<Outlook::_olApplication> spApp, LPCWSTR pwzTo, LPCWSTR pwzCC, LPCWSTR pwzSubject, LPCWSTR pwzHTMLBody)
{
    HRESULT             hr = S_OK;
    CComPtr<IDispatch>  spDisp;
	hr = spApp->CreateItem(/*OlItemType::*/Outlook::olMailItem, &spDisp);
    if(FAILED(hr) || spDisp==0)
        return hr;
    CComQIPtr<Outlook::_MailItem>   spMail(spDisp);
    ATLASSERT(spMail);

    CComBSTR varSubject(pwzSubject);
    CComBSTR varHTMLBody(pwzHTMLBody);
    CComBSTR varTo(pwzTo);
    CComBSTR varCC(pwzCC);
	hr = spMail->put_BodyFormat(/*OlBodyFormat::*/Outlook::olFormatHTML);
	if(FAILED(hr))
		return hr;
    hr = spMail->put_Subject(varSubject);
	if(FAILED(hr))
		return hr;
    hr = spMail->put_HTMLBody(varHTMLBody);
	if(FAILED(hr))
		return hr;
    hr = spMail->put_To(varTo);
	if(FAILED(hr))
		return hr;
    if(NULL!=pwzCC && 0<wcslen(pwzCC))
        hr = spMail->put_CC(varCC);
    hr = spMail->Send();
    return hr;
}

std::wstring OlCommandBarButtonHandler::MakeOriginalRequest(ApprovalEmail& ae)
{
    std::wstring strOriginalRequest = L"";
    RecipientVector::const_iterator itr;
	std::vector<std::wstring>::const_iterator      itf;
    const std::wstring& strRequesterAddr = ae.get_RequesterAddress();
    const std::wstring& strRequesterName = ae.get_RequesterName();
   
    const std::wstring& strPurpose = ae.get_Purpose();
    const std::vector<std::wstring>& vFiles = ae.get_vecFiles();
    const RecipientVector& vRecipients = ae.get_Recipients();
    // <-- Original Request
    strOriginalRequest += L"<table width=\"610\" style=\"font-family:Arial\" style=\"font-size:15\">\r\n<tr>\r\n";
    strOriginalRequest += L"  <tr>\r\n";
    strOriginalRequest += L"    <td valign=\"center\" align=\"left\"><HR><BR><B>Original Request from You:</B></td>\r\n";
    strOriginalRequest += L"  </tr>\r\n";
    strOriginalRequest += L"</table>\r\n";

    strOriginalRequest += L"<TABLE width=\"610\" style=\"font-family:Arial\" style=\"font-size:13\">\r\n";
    // Type
    // add "pre information"
    strOriginalRequest += L"  <tr>\r\n";
    strOriginalRequest += L"    <td colspan=\"2\" valign=\"top\" align=\"left\">&nbsp;&nbsp;&nbsp;&nbsp;Please approve sharing of following files requested by ";
    strOriginalRequest += L"<a href=mailto:"; strOriginalRequest += strRequesterAddr; strOriginalRequest+=L">"; strOriginalRequest += strRequesterName; strOriginalRequest += L"</a>";
    /*strOriginalRequest += L" using "; strOriginalRequest += strRequestType; */strOriginalRequest += L"<br>&nbsp;<br></td>\r\n";
    strOriginalRequest += L"  </tr>\r\n";

    // Subject
    strOriginalRequest += L"  <tr>\r\n";
    strOriginalRequest += L"    <td valign=\"top\" align=\"left\" width=\"10%\"><b>Subject:</b></td>\r\n";
	strOriginalRequest += L"    <td valign=\"top\" align=\"left\">";  strOriginalRequest += ae.get_OrigSubject(); strOriginalRequest += L"<br>&nbsp;<br></td>\r\n";
    strOriginalRequest += L"  </tr>\r\n";
    // Purpose
    strOriginalRequest += L"  <tr>\r\n";
    strOriginalRequest += L"    <td valign=\"top\" align=\"left\"><b>Content:</b></td>\r\n";
    strOriginalRequest += L"    <td valign=\"top\" align=\"left\"><pre>";  strOriginalRequest += strPurpose; strOriginalRequest += L"</pre></td>\r\n";
    strOriginalRequest += L"  </tr>\r\n";
    // Recipients
    strOriginalRequest += L"  <tr>\r\n";
    strOriginalRequest += L"    <td valign=\"top\" align=\"left\"><b>Recipients:</b></td>\r\n";
    strOriginalRequest += L"    <td valign=\"top\" align=\"left\">";
    for(itr = vRecipients.begin(); itr!=vRecipients.end(); ++itr)
    {
        strOriginalRequest += (*itr).c_str(); strOriginalRequest += L"<br>";
    }
    strOriginalRequest += L"&nbsp;<br></td>\r\n";
    strOriginalRequest += L"  </tr>\r\n";
    // Files
    strOriginalRequest += L"  <tr>\r\n";
    strOriginalRequest += L"    <td valign=\"top\" align=\"left\"><b>Files:</b></td>\r\n";
    strOriginalRequest += L"    <td valign=\"top\" align=\"left\">\r\n";
	
    for(itf = vFiles.begin(); itf!=vFiles.end(); ++itf)
    {
        
        strOriginalRequest += L"      <a href=\"";
        strOriginalRequest += (*itf).c_str();
        strOriginalRequest += L"\">";
        strOriginalRequest += (*itf).c_str();//pwzFileName?(pwzFileName+1):((*itf).first.c_str());
        strOriginalRequest += L"</a><br>\r\n";
    }
    strOriginalRequest += L"    &nbsp;<br></td>\r\n";
    strOriginalRequest += L"  </tr>\r\n";

    // Original Request End -->
    strOriginalRequest += L"</TABLE>\r\n";

    return strOriginalRequest;
}

std::wstring OlCommandBarButtonHandler::ComposeRejectMail(ApprovalEmail& ae)
{
    std::wstring strMail = L"";
    const std::wstring strOriginalRequest = MakeOriginalRequest(ae);

    // Title
    strMail += L"<table width=\"610\" style=\"font-family:Arial\" style=\"font-size:19\">\r\n<tr>\r\n";
    strMail += L"  <tr>\r\n";
    strMail += L"    <td valign=\"center\" align=\"left\" height=\"50\"><b>Your request is rejected</b></td>\r\n";
    strMail += L"  </tr>\r\n";
    strMail += L"</table>\r\n";

    // Information
    strMail += L"<table width=\"610\" style=\"font-family:Arial\" style=\"font-size:15\">\r\n<tr>\r\n";
    strMail += L"  <tr>\r\n";
    strMail += L"    <td valign=\"center\" align=\"left\">&nbsp;&nbsp;&nbsp;&nbsp;Your request has been rejected by approver.\
                Please check your request and contact approver to get detail information.</td>\r\n";
    strMail += L"  </tr>\r\n";
    strMail += L"</table>\r\n";

    // Original Request
    strMail += strOriginalRequest;

    return strMail;
}

std::wstring OlCommandBarButtonHandler::ComposeApproveMail(ApprovalEmail& ae, ApproveFilesVector& vAFiles)
{
    std::wstring strMail = L"";
    const std::wstring strOriginalRequest = MakeOriginalRequest(ae);
    ApproveFilesVector::iterator    itaf;

    // Title
    strMail += L"<table width=\"610\" style=\"font-family:Arial\" style=\"font-size:19\">\r\n<tr>\r\n";
    strMail += L"  <tr>\r\n";
    strMail += L"    <td valign=\"center\" align=\"left\" height=\"50\"><b>The request is approved</b></td>\r\n";
    strMail += L"  </tr>\r\n";
    strMail += L"</table>\r\n";

 //   // Information
 //   strMail += L"<table width=\"610\" style=\"font-family:Arial\" style=\"font-size:15\">\r\n<tr>\r\n";
 //   strMail += L"  <tr>\r\n";
 //   strMail += L"    <td valign=\"center\" align=\"left\">&nbsp;&nbsp;&nbsp;&nbsp;Your request has been approved by approver.\
 //               Following file(s) can be shared with recipient(s).</td>\r\n";
 //   strMail += L"  </tr>\r\n";
 //   // Approved files
 //   strMail += L"  <tr>\r\n";
 //   strMail += L"    <td valign=\"center\" align=\"left\">\r\n";
	//std::vector<std::wstring>& vecFiles=m_pae->get_vecFiles();
	//for(itaf = vecFiles.begin(); itaf!=vecFiles.end(); ++itaf)
 //   {
 //       LPCWSTR pwzFileName = wcsrchr((*itaf).c_str(), L'\\');
 //       strMail += L"      <a href=\"";
 //       strMail += (*itaf).c_str(); strMail += L"\">";
 //       strMail += (*itaf).c_str();//pwzFileName?(pwzFileName+1):((*itaf).c_str()); 
	//	strMail += L"</a><br>\r\n";
 //   }
 //   strMail += L"    </td>\r\n";
 //   strMail += L"  </tr>\r\n";
 //   strMail += L"</table>\r\n";

	// Information
    strMail += L"<table width=\"610\" style=\"font-family:Arial\" style=\"font-size:15\">\r\n<tr>\r\n";
    strMail += L"  <tr>\r\n";
	strMail += L"    <td valign=\"center\" align=\"left\">&nbsp;&nbsp;&nbsp;&nbsp;Please send the following link to the recipient(s) in the request below:</td>\r\n";
    strMail += L"  </tr>\r\n";
    // Uploaded files
    strMail += L"  <tr>\r\n";
    strMail += L"    <td valign=\"center\" align=\"left\">\r\n";
	//std::vector<std::wstring>& vecFiles=m_pae->get_vecFiles();
	for(itaf = vAFiles.begin(); itaf!=vAFiles.end(); ++itaf)
    {
        
        strMail += L"      <a href=\"";
        strMail += (*itaf).c_str(); strMail += L"\">";
        strMail += (*itaf).c_str();//pwzFileName?(pwzFileName+1):((*itaf).c_str()); 
		strMail += L"</a><br>\r\n";
    }
    strMail += L"    </td>\r\n";
    strMail += L"  </tr>\r\n";
    strMail += L"</table>\r\n";
    // Original Request
    strMail += strOriginalRequest;

    return strMail;
}

std::wstring OlCommandBarButtonHandler::ComposeRequestMailStub()
{
    ApprovalEmail   ae;
    ae.put_RequestType(CE_REQUEST_EMAILATTACHMENT);
    ae.put_RequesterName(L"Gavin Ye");
    ae.put_RequesterAddress(L"gavin.ye@nextlabs.com");
    ae.put_ArchiverAddress(L"gye@cn.nextlabs.com");
    ae.put_RequesterSid(L"S-1-5-21-1512634949-460182318-3030517407-1123");  // dong.he's domain SID
    //ae.put_ApprovalAddress(L"gavin.ye@nextlabs.com");
    ae.put_ApprovalDirectory(L"C:\\deny\\approval\\");
    ae.put_Customer(L"Flextronix Inc.");
    ae.put_CustomerKey(L"C:\\deny\\key\\gye.pub.cer");
    ae.put_EncryptPasswd(L"abcd1234");
    ae.put_Purpose(L"A test for approval");
    ae.add_Recipient(L"jzhuang@nextlabs.com");
    ae.add_Recipient(L"dong.he@cn.nextlabs.com");
    ae.add_Recipient(L"jmjin@cn.nextlabs.com");
    ae.add_Recipient(L"ssong@cn.nextlabs.com");
    ae.add_File(L"c:\\deny\\1.doc", L"C:\\deny\\quarantine\\gavin_ye_200810100101014444\\1.doc");
    ae.add_File(L"c:\\deny\\2.doc", L"C:\\deny\\quarantine\\gavin_ye_200810100101014444\\2.doc");
    ae.add_File(L"c:\\deny\\3.doc", L"C:\\deny\\quarantine\\gavin_ye_200810100101014444\\3.doc");
    ae.add_File(L"c:\\deny\\4.doc", L"C:\\deny\\quarantine\\gavin_ye_200810100101014444\\4.doc");
    ae.add_File(L"c:\\deny\\5.doc", L"C:\\deny\\quarantine\\gavin_ye_200810100101014444\\5.doc");
    return ae.Compose();
}

/************************************************************************/
/* class OlInspectorHandler                                             */
/************************************************************************/
HRESULT OlInspectorHandler::OnInvoke(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pdispparams, VARIANT* pvarResult, EXCEPINFO* pexcepinfo, UINT* puArgErr)
{
    HRESULT hr = S_OK;

    if	(pdispparams && DISPATCH_METHOD==wFlags)
    {
        switch	(dispidMember)
        {
        case 0xf001:    // Activate
            OnActivate();
            break;
        case 0xf006:    // Deactivate
            OnDeactivate();
            break;
        case 0xf008:    // Close
            OnClose();
            break;
        default:
            hr = DISP_E_MEMBERNOTFOUND;
            break;
        }
    }
    else
    {
        hr = DISP_E_PARAMNOTFOUND;
    }

    return hr;
}

void OlInspectorHandler::OnActivate()
{
}

void OlInspectorHandler::OnDeactivate()
{
}

void OlInspectorHandler::OnClose()
{
    if(m_spApproveHandler.get())
        m_spApproveHandler->UnregisterEventDispatch();
    if(m_spRejectHandler.get())
        m_spRejectHandler->UnregisterEventDispatch();
	if(m_spUploadHandler.get())
        m_spUploadHandler->UnregisterEventDispatch();
    UnregisterEventDispatch();
    delete this;
}

BOOL OlInspectorHandler::GetToolbarButtons(Office::_CommandBars* spCmdBars)
{
    HRESULT     hr = S_OK;
    int         i=0, nCmdBars = 0;

    hr = spCmdBars->get_Count(&nCmdBars);
    if(FAILED(hr) || 0==nCmdBars)
        return FALSE;

    for (i=0; i<nCmdBars; i++)
    {
        CComVariant varIndex(i+1);
        CComPtr<Office::CommandBar> spCmdBar = 0;
        hr = spCmdBars->get_Item(varIndex, &spCmdBar);
        if (FAILED(hr) || spCmdBar==0)
            continue;

        CComBSTR    bstrName;
        hr = spCmdBar->get_Name(&bstrName);
        if (FAILED(hr) || bstrName==0)
            continue;

        //DP((L"Commandbar Name = %s\n", bstrName));
        if(0 == _wcsicmp(pwzCmdBarName, bstrName))
        {
            CComPtr<Office::CommandBarControls> spBarControls;
            CComPtr< Office::CommandBarControl> spBarCtrlApprove; 
            CComPtr< Office::CommandBarControl> spBarCtrlReject;
			//CComPtr< Office::CommandBarControl> spBarCtrlUpload;
            hr = spCmdBar->get_Controls(&spBarControls);
            if (FAILED(hr) || spBarControls==0)
                return FALSE;

            CComVariant varIndex1(1);
            hr = spBarControls->get_Item(varIndex1, &spBarCtrlApprove);
            if (FAILED(hr) || spBarCtrlApprove==0)
                return FALSE;
            CComVariant varIndex2(2);
            hr = spBarControls->get_Item(varIndex2, &spBarCtrlReject);
            if (FAILED(hr) || spBarCtrlReject==0)
                return FALSE;

			/*CComVariant varIndex4(3);
            hr = spBarControls->get_Item(varIndex4, &spBarCtrlUpload);
            if (FAILED(hr) || spBarCtrlUpload==0)
                return FALSE;*/

#ifdef _DEBUG
            CComVariant varIndex3(3);
            CComPtr<Office::CommandBarControl> spBarCtrlCOStub;
            hr = spBarControls->get_Item(varIndex3, &spBarCtrlCOStub);
            if (FAILED(hr) || spBarCtrlCOStub==0)
                return FALSE;
            CComQIPtr<Office::_CommandBarButton> spCmdButtonCOStub(spBarCtrlCOStub);
            m_spBtnCOStub = spCmdButtonCOStub;
#endif

            CComQIPtr<Office::_CommandBarButton> spCmdButtonApprove(spBarCtrlApprove);
            CComQIPtr<Office::_CommandBarButton> spCmdButtonReject(spBarCtrlReject);
			//CComQIPtr<Office::_CommandBarButton> spCmdButtonUpload(spBarCtrlUpload);
            m_spBtnApprove = spCmdButtonApprove;
            m_spBtnReject  = spCmdButtonReject;
			//m_spBtnUpload  = spCmdButtonUpload;
            return TRUE;
        }
    }

    return FALSE;
}

BOOL OlInspectorHandler::CreateToolbarButtons(Office::_CommandBars* spCmdBars)
{
    HRESULT                             hr  =S_OK;
    CComPtr<Office::CommandBar>         spNewCmdBar;
    CComPtr<Office::CommandBarControls> spBarControls;
    CComPtr<Office::CommandBarControl>  spNewBarCtrlApprove; 
    CComPtr<Office::CommandBarControl>  spNewBarCtrlReject;
	//CComPtr<Office::CommandBarControl>  spNewBarCtrlUpload;
    HBITMAP                             hBtnBmp = NULL;
    
    // Add new toolbar
    CComVariant vName(pwzCmdBarName);
    CComVariant vPos(1);
    CComVariant vTemp(VARIANT_TRUE);
    CComVariant vEmpty(DISP_E_PARAMNOTFOUND, VT_ERROR);
    hr= spCmdBars->Add(vName, vPos, vEmpty, vTemp,&spNewCmdBar);
	if(FAILED(hr)||spNewCmdBar==NULL)
	{
		CLog::WriteLog(L"Error:",L"OlInspectorHandler: failed to add new toolbar as CE Approval Toolbar");
		return FALSE;
	}

    // Add new Bar Controls
    hr = spNewCmdBar->get_Controls(&spBarControls);
	if(FAILED(hr)||spBarControls==NULL)
	{
		CLog::WriteLog(L"Error",L"OlInspectorHandler: failed to add new bar controls");
		return FALSE;
	}
    ATLASSERT(spBarControls);
    CComVariant vToolBarType(msoControlButton);
    CComVariant vShow(VARIANT_TRUE);
    hr = spBarControls->Add(vToolBarType, vEmpty, vEmpty, vEmpty, vShow,&spNewBarCtrlApprove); 
	if(FAILED(hr)||spNewBarCtrlApprove==NULL)
	{
		CLog::WriteLog(L"Error",L"OlInspectorHandler: failed to add bar control for Approval");
		return FALSE;
	}
    ATLASSERT(spNewBarCtrlApprove);
    hr = spBarControls->Add(vToolBarType, vEmpty, vEmpty, vEmpty, vShow,&spNewBarCtrlReject);
	if(FAILED(hr)||spNewBarCtrlReject==NULL)
	{
		CLog::WriteLog(L"Error",L"OlInspectorHandler: failed to add bar control for Reject");
		return FALSE;
	}
    ATLASSERT(spNewBarCtrlReject);
	/*hr = spBarControls->Add(vToolBarType, vEmpty, vEmpty, vEmpty, vShow,&spNewBarCtrlUpload);
		if(FAILED(hr)||spNewBarCtrlUpload==NULL)
	{
		CLog::WriteLog(L"Error",L"OlInspectorHandler: failed to add bar control for Upload");
		return FALSE;
	}
    ATLASSERT(spNewBarCtrlUpload);*/

    // Set _CommandBarButton objects
    CComQIPtr<Office::_CommandBarButton> spCmdButtonApprove(spNewBarCtrlApprove);
    CComQIPtr<Office::_CommandBarButton> spCmdButtonReject(spNewBarCtrlReject);
	//CComQIPtr<Office::_CommandBarButton> spCmdButtonUpload(spNewBarCtrlUpload);
    ATLASSERT(spCmdButtonApprove);
    ATLASSERT(spCmdButtonReject);
	//ATLASSERT(spCmdButtonUpload);

    // Show Approve button
    hBtnBmp =(HBITMAP)::LoadImage(g_hInstance,MAKEINTRESOURCE(IDB_BITMAP1),IMAGE_BITMAP,0,0,LR_LOADMAP3DCOLORS);
    if(hBtnBmp==NULL)
	{
		CLog::WriteLog(L"Error",L"OlInspectorHandler: failed to load Approve button image");
	}
	::OpenClipboard(NULL);
    ::EmptyClipboard();
    ::SetClipboardData(CF_BITMAP, (HANDLE)hBtnBmp);
    ::CloseClipboard();
    spCmdButtonApprove->put_Style(Office::msoButtonIconAndCaption);
    hr = spCmdButtonApprove->PasteFace();
    if (FAILED(hr))
	{
		CLog::WriteLog(L"Error",L"OlInspectorHandler: failed to PasteFace for Approve button1");
		::OpenClipboard(NULL);
		::EmptyClipboard();
		::SetClipboardData(CF_BITMAP, (HANDLE)hBtnBmp);
		::CloseClipboard();
		spCmdButtonApprove->put_Style(Office::msoButtonIconAndCaption);
		hr = spCmdButtonApprove->PasteFace();
		if(FAILED(hr))
		{
			CLog::WriteLog(L"Error",L"OlInspectorHandler: failed to PasteFace for Approve button2");
	        //return FALSE;
		}
	}
	if (NULL != hBtnBmp)
	::DeleteObject(hBtnBmp);
    spCmdButtonApprove->put_Visible(VARIANT_TRUE); 
    spCmdButtonApprove->put_Caption(OLESTR("Approve")); 
    spCmdButtonApprove->put_Enabled(VARIANT_TRUE);
    spCmdButtonApprove->put_TooltipText(OLESTR("Approve")); 
    spCmdButtonApprove->put_Tag(OLESTR("Approve"));
    spNewCmdBar->put_Visible(VARIANT_TRUE);

    // Show Reject button
    hBtnBmp =(HBITMAP)::LoadImage(g_hInstance,MAKEINTRESOURCE(IDB_BITMAP2),IMAGE_BITMAP,0,0,LR_LOADMAP3DCOLORS);
    if(hBtnBmp==NULL)
	{
		CLog::WriteLog(L"Error",L"OlInspectorHandler: failed to load Reject button image");
	}
	::OpenClipboard(NULL);
    ::EmptyClipboard();
    ::SetClipboardData(CF_BITMAP, (HANDLE)hBtnBmp);
    ::CloseClipboard();
    spCmdButtonReject->put_Style(Office::msoButtonIconAndCaption);
    hr = spCmdButtonReject->PasteFace();
    if (FAILED(hr))
    {
		CLog::WriteLog(L"Error",L"OlInspectorHandler: failed to PasteFace for Reject button1");
		::OpenClipboard(NULL);
		::EmptyClipboard();
		::SetClipboardData(CF_BITMAP, (HANDLE)hBtnBmp);
		::CloseClipboard();
		spCmdButtonReject->put_Style(Office::msoButtonIconAndCaption);
		hr = spCmdButtonReject->PasteFace();
		if(FAILED(hr))
		{
			CLog::WriteLog(L"Error",L"OlInspectorHandler: failed to PasteFace for Reject button2");
			//return FALSE;
		}
	}
	if (NULL != hBtnBmp)
	::DeleteObject(hBtnBmp);
    spCmdButtonReject->put_Visible(VARIANT_TRUE); 
    spCmdButtonReject->put_Caption(OLESTR("Reject")); 
    spCmdButtonReject->put_Enabled(VARIANT_TRUE);
    spCmdButtonReject->put_TooltipText(OLESTR("Reject")); 
    spCmdButtonReject->put_Tag(OLESTR("Reject"));
    spNewCmdBar->put_Visible(VARIANT_TRUE);

	// Show Upload button
 //   hBtnBmp =(HBITMAP)::LoadImage(g_hInstance,MAKEINTRESOURCE(IDB_BITMAP4),IMAGE_BITMAP,0,0,LR_LOADMAP3DCOLORS);
 //   if(hBtnBmp==NULL)
	//{
	//	CLog::WriteLog(L"Error",L"OlInspectorHandler: failed to load Upload button image");
	//}
	//::OpenClipboard(NULL);
 //   ::EmptyClipboard();
 //   ::SetClipboardData(CF_BITMAP, (HANDLE)hBtnBmp);
 //   ::CloseClipboard();
 //   
 //   spCmdButtonUpload->put_Style(Office::msoButtonIconAndCaption);
 //   hr = spCmdButtonUpload->PasteFace();
 //   if (FAILED(hr))
 //   {
	//	CLog::WriteLog(L"Error",L"OlInspectorHandler: failed to PasteFace for Upload button1");
	//	::OpenClipboard(NULL);
	//	::EmptyClipboard();
	//	::SetClipboardData(CF_BITMAP, (HANDLE)hBtnBmp);
	//	::CloseClipboard();
	//	spCmdButtonUpload->put_Style(Office::msoButtonIconAndCaption);
	//	hr = spCmdButtonUpload->PasteFace();
	//	if(FAILED(hr))
	//	{
	//		CLog::WriteLog(L"Error",L"OlInspectorHandler: failed to PasteFace for Upload button2");
	//		//return FALSE;
	//	}
	//}
	//::DeleteObject(hBtnBmp);
 //   spCmdButtonUpload->put_Visible(VARIANT_TRUE); 
 //   spCmdButtonUpload->put_Caption(OLESTR("Upload to FTP")); 
 //   spCmdButtonUpload->put_Enabled(VARIANT_TRUE);
 //   spCmdButtonUpload->put_TooltipText(OLESTR("Upload to FTP")); 
 //   spCmdButtonUpload->put_Tag(OLESTR("Upload to FTP"));
 //   spNewCmdBar->put_Visible(VARIANT_TRUE);

    // Save these two buttons' objects
    m_spBtnApprove  = spCmdButtonApprove;
    m_spBtnReject   = spCmdButtonReject;
	//m_spBtnUpload   = spCmdButtonUpload;

#ifdef _DEBUG
    CComPtr<Office::CommandBarControl>  spNewBarCtrlCOStub;
    hr = spBarControls->Add(vToolBarType, vEmpty, vEmpty, vEmpty, vShow,&spNewBarCtrlCOStub);
    ATLASSERT(spNewBarCtrlCOStub);
    CComQIPtr<Office::_CommandBarButton> spCmdButtonCOStub(spNewBarCtrlCOStub);
    ATLASSERT(spCmdButtonCOStub);

    hBtnBmp =(HBITMAP)::LoadImage(g_hInstance,MAKEINTRESOURCE(IDB_BITMAP3),IMAGE_BITMAP,0,0,LR_LOADMAP3DCOLORS);
    if(hBtnBmp==NULL)
	{
		CLog::WriteLog(L"Error",L"OlInspectorHandler: failed to load COTest button image");
	}
	::OpenClipboard(NULL);
    ::EmptyClipboard();
    ::SetClipboardData(CF_BITMAP, (HANDLE)hBtnBmp);
    ::CloseClipboard();
    
    spCmdButtonCOStub->put_Style(Office::msoButtonIconAndCaption);
    hr = spCmdButtonCOStub->PasteFace();
    if (FAILED(hr))
    {
		CLog::WriteLog(L"Error",L"OlInspectorHandler: failed to PasteFace for COTest button1");
		::OpenClipboard(NULL);
		::EmptyClipboard();
		::SetClipboardData(CF_BITMAP, (HANDLE)hBtnBmp);
		::CloseClipboard();
		spCmdButtonCOStub->put_Style(Office::msoButtonIconAndCaption);
		hr = spCmdButtonCOStub->PasteFace();
		if(FAILED(hr))
		{
			CLog::WriteLog(L"Error",L"OlInspectorHandler: failed to PasteFace for COTest button2");
			//return FALSE;
		}
	}        
    ::DeleteObject(hBtnBmp);
    spCmdButtonCOStub->put_Visible(VARIANT_TRUE); 
    spCmdButtonCOStub->put_Caption(OLESTR("COTest")); 
    spCmdButtonCOStub->put_Enabled(VARIANT_TRUE);
    spCmdButtonCOStub->put_TooltipText(OLESTR("COTest")); 
    spCmdButtonCOStub->put_Tag(OLESTR("COTest"));
    spNewCmdBar->put_Visible(VARIANT_TRUE);
    m_spBtnCOStub   = spCmdButtonCOStub;
#endif

    return TRUE;
}

BOOL OlInspectorHandler::AddApprovalToolbar(BOOL bRequestMail,BOOL bFtped,BOOL bApproved,bool bMisMatched)
{
    HRESULT                             hr  =S_OK;
    CComPtr<Outlook::_Explorer>         spExplorer;
    CComPtr<Office::_CommandBars>       spCmdBars;

    hr = m_spInspector->get_CommandBars(&spCmdBars);
    if(FAILED(hr) || spCmdBars==0)
        return hr;

    ATLASSERT(spCmdBars);

    if(!GetToolbarButtons(spCmdBars))
        if(!CreateToolbarButtons(spCmdBars))
            return FALSE;

    if(bRequestMail)
    {
        // Dispatch button events
        m_spApproveHandler =  YLIB::COMMON::smart_ptr<OlCommandBarButtonHandler>(new OlCommandBarButtonHandler(m_spInspector, BTP_APPROVE));
        m_spApproveHandler->SetApprovalEmail(m_pae);
		m_spApproveHandler->SetFtpSite(this->m_ftpSite);
		m_spApproveHandler->SetMatchType(this->m_matchType);
		DP((L"AddApprovalToolbar RegisterEventDispatch for m_spApproveHandler"));
		m_spApproveHandler->RegisterEventDispatch(m_spBtnApprove);

        m_spRejectHandler  =  YLIB::COMMON::smart_ptr<OlCommandBarButtonHandler>(new OlCommandBarButtonHandler(m_spInspector, BTP_REJECT));
        m_spRejectHandler->SetApprovalEmail(m_pae);
		m_spRejectHandler->SetFtpSite(this->m_ftpSite);
		m_spRejectHandler->SetMatchType(this->m_matchType);
		DP((L"AddApprovalToolbar RegisterEventDispatch for m_spRejectHandler"));
		m_spRejectHandler->RegisterEventDispatch(m_spBtnReject);

		/*m_spUploadHandler =  YLIB::COMMON::smart_ptr<OlCommandBarButtonHandler>(new OlCommandBarButtonHandler(m_spInspector, BTP_UPLOAD));
        m_spUploadHandler->SetApprovalEmail(m_pae);
		m_spUploadHandler->SetFtpSite(this->m_ftpSite);
		m_spUploadHandler->SetMatchType(this->m_matchType);
		DP((L"AddApprovalToolbar RegisterEventDispatch for m_spUploadHandler"));
		m_spUploadHandler->RegisterEventDispatch(m_spBtnUpload);*/
		if(bMisMatched==true)
		{
			m_spBtnApprove->put_Enabled(FALSE);
			m_spBtnReject->put_Enabled(TRUE);
			//m_spBtnUpload->put_Enabled(FALSE);
		}
		else
		{
			if(bApproved==TRUE)
			{
				m_spBtnApprove->put_Enabled(FALSE);
				m_spBtnReject->put_Enabled(FALSE);
				//m_spBtnUpload->put_Enabled(FALSE);
			}
			else
			{
				m_spBtnReject->put_Enabled(TRUE);
				m_spBtnApprove->put_Enabled(TRUE);
			}
		}
    }
    else
    {
        m_spBtnApprove->put_Enabled(FALSE);
        m_spBtnReject->put_Enabled(FALSE);
		//m_spBtnUpload->put_Enabled(FALSE);
    }

#ifdef _DEBUG
    m_spCOStubHandler =  YLIB::COMMON::smart_ptr<OlCommandBarButtonHandler>(new OlCommandBarButtonHandler(m_spInspector, BTP_COSTUB));
    m_spCOStubHandler->RegisterEventDispatch(m_spBtnCOStub);
#endif

    return TRUE;
}

/************************************************************************/
/* class OlInspectorsHandler                                            */
/************************************************************************/
HRESULT OlInspectorsHandler::OnInvoke(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pdispparams, VARIANT* pvarResult, EXCEPINFO* pexcepinfo, UINT* puArgErr)
{
    HRESULT hr = S_OK;

    if	(pdispparams && DISPATCH_METHOD==wFlags)
    {
        switch	(dispidMember)
        {
        case 0xf001:    // NewInspector
        {
            CComQIPtr<IDispatch> spDispatch(pdispparams->rgvarg[0].pdispVal);
            CComQIPtr<Outlook::_Inspector> spInspector(spDispatch);
            OnNewInspector(spInspector);
            break;
        }
        default:
            hr = DISP_E_MEMBERNOTFOUND;
            break;
        }
    }
    else
    {
        hr = DISP_E_PARAMNOTFOUND;
    }

    return hr;
}

void OlInspectorsHandler::OnNewInspector(CComQIPtr<Outlook::_Inspector> Inspector)
{
    OlInspectorHandler*  pInspectorHandler = new OlInspectorHandler(Inspector);
    
    if(NULL == pInspectorHandler) return;
	CLog::WriteLog(L"New an email",L"");

    pInspectorHandler->RegisterEventDispatch(Inspector);

	//BOOL bRequest=IsApprovalRequest(Inspector);
	BOOL bFtped=FALSE,bApproved=FALSE;
	BOOL bRequest=IsApprovalRequest(Inspector,&bFtped,&bApproved) ;

	CMultiFTPManager::MATCH_TYPE matchType=CMultiFTPManager::NONE;
	CMultiFTPManager ftpManager(g_hInstance);
	if(bRequest)
	{
		HRESULT         hr = S_OK;
		if(NULL==Inspector) return;

		CComPtr<IDispatch>  spMailDisp;
		hr = Inspector->get_CurrentItem(&spMailDisp);
		if(FAILED(hr) || spMailDisp==0)
			return;

		CComQIPtr<Outlook::_MailItem> spMail(spMailDisp);
		if(spMail == 0)
			return;

		CComBSTR    bstrBody;
		hr = spMail->get_HTMLBody(&bstrBody);
		if (FAILED(hr) || bstrBody==0)
			return;
#pragma warning(push)
#pragma warning(disable: 6211)
		ApprovalEmail* p_ae=new ApprovalEmail;
#pragma warning(pop)
		p_ae->Parse(bstrBody);
		CFtpSite*pSite=new CFtpSite;
		
		std::vector<std::wstring> vecRec;
		const RecipientVector& vecAERec=p_ae->get_Recipients();
		RecipientVector::const_iterator it;
		for(it=vecAERec.begin();it!=vecAERec.end();it++)
		{
			vecRec.push_back((*it));
		}
		matchType=ftpManager.Match(vecRec,*pSite);
		
		{
			/*CFtpSite ftpSite;
			CMultiFTPManager ftpManager2(g_hInstance);
			std::wstring oneRecipient;
			std::vector<std::wstring> vecTest;
			oneRecipient=L"john.tyler@qapf.qalab01.nextlabs.com";vecTest.push_back(oneRecipient);
			oneRecipient=L"vishi.iyer@nextlabs.com";vecTest.push_back(oneRecipient);
			oneRecipient=L"robert.lin@nextlabs.com";vecTest.push_back(oneRecipient);
			CMultiFTPManager::MATCH_TYPE matchType=ftpManager2.Match(vecTest,ftpSite);
			vecTest.clear();
			oneRecipient=L"a@company1.com";vecTest.push_back(oneRecipient);
			oneRecipient=L"b@company2.com";vecTest.push_back(oneRecipient);
			matchType=ftpManager2.Match(vecTest,ftpSite);

			vecTest.clear();
			oneRecipient=L"a@company1.com";vecTest.push_back(oneRecipient);
			oneRecipient=L"b@company4.com";vecTest.push_back(oneRecipient);
			matchType=ftpManager2.Match(vecTest,ftpSite);

			vecTest.clear();
			oneRecipient=L"a@company3.com";vecTest.push_back(oneRecipient);
			oneRecipient=L"b@company4.com";vecTest.push_back(oneRecipient);
			matchType=ftpManager2.Match(vecTest,ftpSite);*/
		}
		
		pInspectorHandler->SetMatchType(matchType);
		
		pInspectorHandler->SetFtpSite(pSite);
		pInspectorHandler->SetApprovalEmail(p_ae);
	}
	
	if(ftpManager.IsMalFormed()==true)
		bRequest=FALSE;
	pInspectorHandler->AddApprovalToolbar(bRequest,bFtped,bApproved, matchType==CMultiFTPManager::MISMATCH?true:false);
	DP((L"000000000000000000000000000000000000000"));
}

BOOL OlInspectorsHandler::IsApprovalRequest(CComPtr<Outlook::_Inspector> Inspector,BOOL* bFtped, BOOL* bApproved)
{
    HRESULT         hr = S_OK;
    if(NULL==Inspector) return FALSE;

    CComPtr<IDispatch>  spMailDisp;
    hr = Inspector->get_CurrentItem(&spMailDisp);
    if(FAILED(hr) || spMailDisp==0)
        return FALSE;

    CComQIPtr<Outlook::_MailItem> spMail(spMailDisp);
    if(spMail == 0)
        return FALSE;

	DP((L"Before if(bFtped&&bApproved) in IsApprovalRequest"));
	if(bFtped&&bApproved)
	{
		CComBSTR	bstrSubject;
		hr=spMail->get_Subject(&bstrSubject);
		if(SUCCEEDED(hr)&&bstrSubject)
		{
			if(_wcsnicmp(bstrSubject,L"[Approved] ",11)==0||_wcsnicmp(bstrSubject,L"RE: [Approved] ",15)==0||_wcsnicmp(bstrSubject,L"FW: [Approved] ",15)==0)
				*bApproved=TRUE;
			else if(_wcsnicmp(bstrSubject,L"[Ready for Approval] ",21)==0||_wcsnicmp(bstrSubject,L"RE: [Ready for Approval] ",25)==0||_wcsnicmp(bstrSubject,L"FW: [Ready for Approval] ",25)==0)
				*bFtped=TRUE;
			else if(_wcsnicmp(bstrSubject,L"[Uploading to FTP] ",19)==0||_wcsnicmp(bstrSubject,L"RE: [Uploading to FTP] ",23)==0||_wcsnicmp(bstrSubject,L"FW: [Uploading to FTP] ",23)==0
				||_wcsnicmp(bstrSubject,L"[Approval Upload Failed] ",25)==0||_wcsnicmp(bstrSubject,L"RE: [Approval Upload Failed] ",29)==0||_wcsnicmp(bstrSubject,L"FW: [Approval Upload Failed] ",29)==0)
				*bFtped=FALSE;
			else
			{
				*bFtped=FALSE;
				*bApproved=FALSE;
			}
		}
	}
	DP((L"After if(bFtped&&bApproved) in IsApprovalRequest"));
    // CComBSTR    bstrSenderName;
    //hr = spMail->get_SenderName(&bstrSenderName);
    // if(FAILED(hr) || bstrSenderName.m_str==0 || bstrSenderName.m_str[0]==0)
    //   return FALSE;

    //enum OlBodyFormat
    //{
    //    olFormatUnspecified = 0,
    //    olFormatPlain = 1,
    //    olFormatHTML = 2,
    //    olFormatRichText = 3
    //};
	CComBSTR    bstrBody;
    hr = spMail->get_HTMLBody(&bstrBody);

	//DP((L"Email Body*****************\n%s",bstrBody.m_str));
    if (FAILED(hr) || bstrBody==0)
        return FALSE;

    //DP((L"\nEMail Body Start\n%s\nEMail Body End\n\n", bstrBody.m_str));

    ApprovalEmail   ae;
	BOOL bRet=ae.IsApprovalMail(bstrBody.m_str);
	
	if(bRet==FALSE)
	{
		CLog::WriteLog(L"This is not an Approval Request email",L"");
		CLog::WriteLog(L"Email Body",bstrBody.m_str);

	}

    return bRet;
}
