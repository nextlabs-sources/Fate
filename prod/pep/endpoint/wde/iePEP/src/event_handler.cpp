#include "stdafx.h"
#include <windows.h>
#include "celog.h"

#pragma warning( push )
#pragma warning( disable : 6011 )
#  include <boost/algorithm/string.hpp>
#pragma warning(pop)

extern void ConvertURLCharacterW(std::wstring& strUrl);

#include "event_handler.h"
#include "utils.h"
#include "EvaForPDF.h"
#include "ActionHandler.h"
#include "iepmapi.h"
#define SafeRelease(x)  if(x){x->Release();x=NULL;}

#define DISPID_WINDOWSTATECHANGED 283   // the windows's visibility or enabled state has changed

#define CELOG_CUR_MODULE L"iePEP"
#define CELOG_CUR_FILE CELOG_FILEPATH_PROD_PEP_ENDPOINT_WDE_IEPEP_SRC_EVENT_HANDLER_CPP

#ifndef __docobj_h__
typedef enum
{
    OLECMDIDF_WINDOWSTATE_USERVISIBLE        = 0x00000001,
    OLECMDIDF_WINDOWSTATE_ENABLED            = 0x00000002,
    OLECMDIDF_WINDOWSTATE_USERVISIBLE_VALID  = 0x00010000,
    OLECMDIDF_WINDOWSTATE_ENABLED_VALID      = 0x00020000,
} OLECMDID_WINDOWSTATE_FLAG;
#endif


std::wstring        g_activeAnchorUrl;
std::wstring        g_activeAnchorName;

std::wstring        g_strDownloadUrl;
std::wstring        g_strUploadUrl;
CPDF_Manager	g_thePDFEva ;

extern void HookShowModalDialog(CComQIPtr<IHTMLDocument2> spDocument2, BOOL bHook);
extern  BOOL DisableInput(CComQIPtr<IHTMLDocument2> spDoc);
extern VOID KeepIePepEnabled();

extern CActionHandler* pActionHandler;
extern HINSTANCE g_hInstance;
static const GUID FOLDERID_InternetCache = {0x352481E8, 0x33BE, 0x4251, {0xBA, 0x85, 0x60, 0x07, 0xCA, 0xED, 0xCF, 0x9D}};

LPCTSTR SHARED_MUTEX_NAME = L"5AA70967-8661-424F-BA00-978CBC35A6DE";
HANDLE gSharedFileMutex;

static bool writeURLToTmpFile(std::wstring url)
{
	gSharedFileMutex = OpenMutex(MUTEX_ALL_ACCESS, false, SHARED_MUTEX_NAME);
	BOOL bProtectedMode = FALSE;
	HRESULT hISProtectedMode = IEIsProtectedModeProcess(&bProtectedMode);

	if (FAILED(hISProtectedMode) && !bProtectedMode)
	{
		return false;
	}

	LPWSTR pwszCacheDir = NULL;
	HRESULT hIEGetPath = IEGetWriteableFolderPath(FOLDERID_InternetCache, &pwszCacheDir);

	if (SUCCEEDED(hIEGetPath))
	{
		DWORD dwResult = WaitForSingleObject(gSharedFileMutex, INFINITE);
		std::wstring tempFile;
		tempFile.append(pwszCacheDir);
		tempFile.append(L"\\nxIETemp.tmp");
		HANDLE hFileHandle = CreateFile(tempFile.c_str(), GENERIC_ALL, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFileHandle == INVALID_HANDLE_VALUE)
		{
			return false;
		}
		DWORD dwBytesWritten = 0; 

		BOOL bSuccess = WriteFile(hFileHandle, url.c_str(), (wcslen(url.c_str()) + 1)*sizeof(WCHAR), &dwBytesWritten, NULL);
		if (!bSuccess)
		{
			CloseHandle(hFileHandle);
			return false;
		}
		CoTaskMemFree(pwszCacheDir);
		CloseHandle(hFileHandle);
		switch (dwResult)
		{
		case WAIT_OBJECT_0:
			if (!ReleaseMutex(gSharedFileMutex))
			{
				DP((L"Release gSharedFileMutex error %d", GetLastError()));
			}
			break;
		default:
			DP((L"Wait error %d", GetLastError()));
			return false;
		}
	}
	return true;
}




//////////////////////////////////////////////////////////////////////////
// Detour Submit
//////////////////////////////////////////////////////////////////////////

static BOOL HasInputFileInForm(CComPtr<IHTMLElement> spParentElem, _STRINGLIST& vStrList)
{
	CELOG_LOG(CELOG_DUMP, L" The Parameters are: vStrList=%p \n", &vStrList);

    BOOL    bHasInputFile = FALSE;
    HRESULT hr = S_OK;
    CComPtr<IDispatch>  spDispElemCollection = NULL;
    CComPtr<IHTMLElementCollection> spElems  = NULL;
    long    lElems = 0;
    long    i      = 0;

    hr = spParentElem->get_children(&spDispElemCollection);
    if(FAILED(hr) || !spDispElemCollection)
        goto NO_CHILD_EXIT;

    hr = spDispElemCollection->QueryInterface(IID_IHTMLElementCollection, (void**)&spElems);
    if(FAILED(hr) || !spElems)
        goto NO_CHILD_EXIT;

    hr = spElems->get_length(&lElems);
    if(FAILED(hr) || 0==lElems)
        goto NO_CHILD_EXIT;

    // HAS CHILD
    for (i=0; i<lElems; i++)
    {
        CComVariant   varIndex(i);
        CComPtr<IDispatch>    spDispElem = 0;
        hr = spElems->item(varIndex, varIndex, (IDispatch**)&spDispElem);
        if(SUCCEEDED(hr) && spDispElem)
        {
            CComPtr<IHTMLElement>  spElem = 0;
            hr = spDispElem->QueryInterface(IID_IHTMLElement, (void**)&spElem);
            if(SUCCEEDED(hr) && spElem)
            {
                CComPtr<IHTMLInputFileElement>  spInputFileElem = 0;
                hr = spElem->QueryInterface(IID_IHTMLInputFileElement, (void**)&spInputFileElem);
                if(SUCCEEDED(hr) && spInputFileElem)
                {
                    bHasInputFile = TRUE;
                    BSTR bstrValue = 0;
                    hr = spInputFileElem->get_value(&bstrValue);
                    if(SUCCEEDED(hr) && bstrValue)
                    {
                        std::wstring strTemp(bstrValue);
                        ConvertURLCharacterW(strTemp);
                        vStrList.push_back(strTemp);
                    }
                }
                else
                {
                    if(HasInputFileInForm(spElem, vStrList))
                        bHasInputFile = TRUE;
                }
            }
        }
    }

NO_CHILD_EXIT:
	CELOG_LOG(CELOG_DUMP, L"Local variables are: bHasInputFile=%s, hr=0X%08X  \n", bHasInputFile ? L"TRUE":L"FALSE",hr  );

    return bHasInputFile;
}

//class HTMLFormEventHandler
HRESULT HTMLFormEventHandler::OnInvoke(DISPID dispidMember, const IID &riid, LCID lcid, WORD wFlags, DISPPARAMS *pdispparams, VARIANT *pvarResult, EXCEPINFO *pexcepinfo, UINT *puArgErr)
{
	CELOG_LOG(CELOG_DUMP, L" The Parameters are: dispidMember=0X%08X, riid=%p, lcid=%lu, wFlags=%u, pdispparams=%p, pvarResult=%p, pexcepinfo=%p, puArgErr=%p, puArgErr=%p \n", dispidMember,&riid,lcid, wFlags, pdispparams,pvarResult,pexcepinfo,puArgErr,  (puArgErr) );

    UNREFERENCED_PARAMETER(puArgErr);
    UNREFERENCED_PARAMETER(pexcepinfo);
    UNREFERENCED_PARAMETER(pvarResult);
    UNREFERENCED_PARAMETER(lcid);
    UNREFERENCED_PARAMETER(riid);

    HRESULT hr = S_OK;

    if	(pdispparams && DISPATCH_METHOD==wFlags)
    {
        switch	(dispidMember)
        {
        case DISPID_HTMLFORMELEMENTEVENTS2_ONSUBMIT:
            OnSubmit();
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
	CELOG_LOG(CELOG_DUMP, L"Local variables are: hr=0X%08X \n", hr );

    return hr;
}

void HTMLFormEventHandler::OnSubmit()
{
    std::wstring    strUploadUrl(g_strUploadUrl);
    DP((L"DISPID_HTMLFORMELEMENTEVENTS2_ONSUBMIT! upload url is %s\n", strUploadUrl.c_str()));

    _STRINGLIST vStrList;
    vStrList.clear();

    HRESULT         hr = S_OK;
    CComPtr<IHTMLElement>   spElem = 0;
    hr = m_spFormElem->QueryInterface(IID_IHTMLElement, (void**)&spElem);
    if(SUCCEEDED(hr) && spElem)
    {
        if(HasInputFileInForm(spElem, vStrList))
        {
            BOOL    bAllow = TRUE;

            for (_STRINGLIST::iterator it=vStrList.begin(); it!=vStrList.end(); ++it)
            {
                if ( NULL != pActionHandler && !pActionHandler->UploadAction ( (*it).c_str(), strUploadUrl.c_str() ))
                {
                    bAllow = FALSE;
                }
            }

            if(!bAllow)
            {
                // Block this page
                BSTR    bstrFormAction = 0;
                hr = m_spFormElem->get_action(&bstrFormAction);
                if(SUCCEEDED(hr) && bstrFormAction)
                {
                    std::wstring strDenyUrl(bstrFormAction);
                    WebBrowserEventHandler* pWebHandler = (WebBrowserEventHandler*)m_spParentHandler;
                    if(pWebHandler)
                    {
                        DP((L"    Push Deny URL: %s\n", strDenyUrl.c_str()));
                        pWebHandler->m_vDenyUrlList.push_back(strDenyUrl);
                    }
                    SysFreeString(bstrFormAction);
                }
            }
        }
    }
	CELOG_LOG(CELOG_DUMP, L"Local variables are: vStrList=%p, hr=0X%08X \n", &vStrList,hr );

}


//class HTMLDocumentEventHandler
HRESULT HTMLDocumentEventHandler::OnInvoke(DISPID dispidMember, const IID &riid, LCID lcid, WORD wFlags, DISPPARAMS *pdispparams, VARIANT *pvarResult, EXCEPINFO *pexcepinfo, UINT *puArgErr)
{
	CELOG_LOG(CELOG_DUMP, L" The Parameters are: dispidMember=0X%08X, riid=%p, lcid=%lu, wFlags=%u, pdispparams=%p, pvarResult=%p, pexcepinfo=%p, puArgErr=%p, puArgErr=%p \n", dispidMember, &riid,lcid,wFlags,pdispparams,pvarResult,pexcepinfo,puArgErr,  (puArgErr));

    UNREFERENCED_PARAMETER(puArgErr);
    UNREFERENCED_PARAMETER(pexcepinfo);
    UNREFERENCED_PARAMETER(pvarResult);
    UNREFERENCED_PARAMETER(lcid);
    UNREFERENCED_PARAMETER(riid);

    HRESULT hr = S_OK;

    if	(pdispparams && DISPATCH_METHOD==wFlags)
    {
        switch	(dispidMember)
        {
        case DISPID_HTMLDOCUMENTEVENTS_ONMOUSEDOWN:
            DP((L"DISPID_HTMLDOCUMENTEVENTS_ONMOUSEDOWN\n"));
            g_activeAnchorUrl = L"";
            g_activeAnchorName = L"";
            break;
        case DISPID_HTMLDOCUMENTEVENTS_ONCLICK:
        case DISPID_HTMLDOCUMENTEVENTS_ONCONTEXTMENU:
            if(NULL!=m_spHTMLDocument2 && NULL!=m_spRelatedWebBrowser2)
            {
                DP((L"DISPID_HTMLDOCUMENTEVENTS_ONCLICK or DISPID_HTMLDOCUMENTEVENTS_ONCONTEXTMENU\n"));
                CComPtr<IHTMLElement>   spActiveElem = 0;
                g_activeAnchorUrl = L"";
                g_activeAnchorName = L"";
                hr = m_spHTMLDocument2->get_activeElement(&spActiveElem);
                if(SUCCEEDED(hr) && spActiveElem.p)
                {
                    CComPtr<IHTMLAnchorElement> spActiveAnchor = 0;
                    hr = spActiveElem->QueryInterface(IID_IHTMLAnchorElement, (void**)&spActiveAnchor);
                    if(SUCCEEDED(hr) && spActiveAnchor.p)
                    {
                        BSTR bstrAnchorUrl = 0;
                        hr = spActiveAnchor->get_href(&bstrAnchorUrl);
                        if(SUCCEEDED(hr) && bstrAnchorUrl)
                        {
                            g_activeAnchorUrl = bstrAnchorUrl;
                            ConvertURLCharacterW(g_activeAnchorUrl);
                            DP((L"  Active Anchor URL:: %s\n", g_activeAnchorUrl.c_str()));
                            SysFreeString(bstrAnchorUrl);
                        }

                        BSTR    bstrInnerText = 0;
                        hr = spActiveElem->get_innerText(&bstrInnerText);
                        if(SUCCEEDED(hr) && bstrInnerText)
                        {
                            g_activeAnchorName = bstrInnerText;
                            ConvertURLCharacterW(g_activeAnchorName);
                            DP((L"  Active Anchor Name:: %s\n", g_activeAnchorName.c_str()));
                            SysFreeString(bstrInnerText);
                        }
                    }
                }
            }
            break;
        case DISPID_HTMLDOCUMENTEVENTS_ONSTOP:
            // Fires when the user clicks the Stop button or leaves the Web page.
            // We need to release HtmlDocumentEvent Sink Object when we leave the page
            if(NULL!=m_spHTMLDocument2 && NULL!=m_spRelatedWebBrowser2)
            {
                UnregisterEventDispatch();
                DP((L"HTMLDocumentEventHandler! HTMLDocumentEvent () is UnSinked.\n"));
                m_HtmlFormHandlers.clear();
                m_spRelatedWebBrowser2 = NULL;
            }
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
    CELOG_LOG(CELOG_DUMP, L"Local variables are: hr=0X%08X \n", hr );

    return hr;
}

void HTMLDocumentEventHandler::SinkFormEvent()
{
    HRESULT                 hr = S_OK;
    CComPtr<IHTMLElementCollection> spForms = NULL;

    if(NULL==m_spHTMLDocument2)
	{
		CELOG_LOG(CELOG_DUMP, L"Local variables are: hr=0X%08X  \n", hr );
        return;
	}
    hr = m_spHTMLDocument2->get_forms(&spForms);
    if(SUCCEEDED(hr) && spForms)
    {
        long    lForms = 0;
        long    i      = 0;
        hr = spForms->get_length(&lForms);
        if(SUCCEEDED(hr) && lForms)
        {
            for(i=0; i<lForms; i++)
            {
                CComVariant varIndex(i);
                CComPtr<IDispatch>        spDispForm= 0;
                CComQIPtr<IUnknown,&IID_IUnknown> spUnknown ;
                hr = spForms->item(varIndex, varIndex, (IDispatch**)&spDispForm);
                if(SUCCEEDED(hr) && spDispForm)
                {
                    hr = spDispForm->QueryInterface(IID_IUnknown, (void**)&spUnknown);
                    if(SUCCEEDED(hr) && spUnknown)
                    {
						/************************************************************************************
						Since we will disable edit element of "input:file" control, forbid this operation for now. 
						Another issue: for win2K+IE6.0, the address of API fire_onsubmit is different with IE7.0,
						So need to handle independently, or it will crash in Detour_CFormElementSubmit.

						Kevin Zhou 2008-4-28
						************************************************************************************/

                        BOOL    bIsPostForm = FALSE;
                        BSTR    bstrFormName = 0;
			     		CComQIPtr<IHTMLFormElement> spForm(spUnknown);		
						if (spForm == NULL)
						{
							return;
						}
                        hr = spForm->get_name(&bstrFormName);
                        if(SUCCEEDED(hr) && bstrFormName)
                        {
                            SysFreeString(bstrFormName);
                        }

                        BSTR    bstrFormMethod = 0;
                        hr = spForm->get_method(&bstrFormMethod);
                        if(SUCCEEDED(hr) && bstrFormMethod)
                        {
                            if(0 == _wcsicmp(bstrFormMethod, L"POST"))
                            {
                                bIsPostForm = TRUE;
                            }
                            SysFreeString(bstrFormMethod);
                        }

                        BSTR    bstrFormAction = 0;
                        hr = spForm->get_action(&bstrFormAction);
                        if(SUCCEEDED(hr) && bstrFormAction)
                        {
                            SysFreeString(bstrFormAction);
                        }

                        // Sink this form's event
                        boost::shared_ptr<HTMLFormEventHandler> spHTMLFormEventHandler(new HTMLFormEventHandler(spForm, m_spRelatedWebBrowser2, m_spParentHandler));
                        if(bIsPostForm && spHTMLFormEventHandler->RegisterEventDispatch(spUnknown))
                        {
                        }
                        else
                        {
                            if(bIsPostForm)
                            {
                                DP((L"Fail to sink form event (0x%X)\n", spForm));
                            }
                        }
                        m_HtmlFormHandlers.push_back(spHTMLFormEventHandler);
                    }
                }
                else
                {
                    DP((L"Fail to get Form %d\n", i));
                }
            }
        }
    }
    CELOG_LOG(CELOG_DUMP, L"Local variables are: hr=0X%08X  \n", hr );

}


// class WebBrowserEventHandler
HRESULT WebBrowserEventHandler::OnInvoke(DISPID dispidMember, const IID &riid, LCID lcid, WORD wFlags, DISPPARAMS *pdispparams, VARIANT *pvarResult, EXCEPINFO *pexcepinfo, UINT *puArgErr)
{
	CELOG_LOG(CELOG_DUMP, L" The Parameters are: dispidMember=0X%08X, riid=%p, lcid=%lu, wFlags=%u, pdispparams=%p, pvarResult=%p, pexcepinfo=%p, puArgErr=%p  \n", dispidMember, &riid,lcid, wFlags,pdispparams,pvarResult,pexcepinfo,puArgErr);

    UNREFERENCED_PARAMETER(puArgErr);
    UNREFERENCED_PARAMETER(pexcepinfo);
    UNREFERENCED_PARAMETER(pvarResult);
    UNREFERENCED_PARAMETER(lcid);
    UNREFERENCED_PARAMETER(riid);

    HRESULT hr = S_OK;
    LPWSTR  pwzUrl = NULL;

    if	(pdispparams && DISPATCH_METHOD==wFlags)
    {
        switch	(dispidMember)
        {
        case DISPID_BEFORENAVIGATE2:            //250   // hyperlink clicked on
            {
                CComVariant varURL(*pdispparams->rgvarg[5].pvarVal);
                varURL.ChangeType(VT_BSTR);
                pwzUrl = OLE2W(varURL.bstrVal);
                OnBeforeNavigate2(pwzUrl, (IWebBrowser2*)pdispparams->rgvarg[6].pdispVal, (VARIANT_BOOL*)pdispparams->rgvarg[0].pboolVal);
            }
            break;
        case DISPID_NEWWINDOW2:                 //251
            break;
        case DISPID_NAVIGATECOMPLETE2:          //252
			{
				CComVariant varURL(*pdispparams->rgvarg[0].pvarVal);
				varURL.ChangeType(VT_BSTR);
				pwzUrl = OLE2W(varURL.bstrVal);
				OnNavigateComplete2(pwzUrl, (IWebBrowser2*)pdispparams->rgvarg[1].pdispVal);
			}
            break;
        case DISPID_ONQUIT:                     //253
            OnQuit();
            break;
        case DISPID_DOCUMENTCOMPLETE:           //259 new document goes ReadyState_Complete
            OnDocumentComplete((IWebBrowser2*)pdispparams->rgvarg[1].pdispVal);
            break;
        case DISPID_ONVISIBLE:
            {
                VARIANT_BOOL bVisible = (VARIANT_BOOL)pdispparams->rgvarg[0].pboolVal;
                bVisible;   // Avoid unreference warning (DP won't be called in release version)
                DP((L"WebBrowserEvent! DISPID_ONVISIBLE(%p): %s\n", m_spWebBrowser2, bVisible?L"Show":L"Hide"));
            }
            break;
        case DISPID_FILEDOWNLOAD:               //270 Fired to indicate the File Download dialog is opening
            g_strDownloadUrl = m_preNavUrl;
            DP((L"WebBrowserEvent! DISPID_FILEDOWNLOAD(%p): %s\n", m_spWebBrowser2, m_preNavUrl.c_str()));
            break;
        case DISPID_WINDOWSTATECHANGED:
            {
                DWORD dwValidFlagMask = pdispparams->rgvarg[0].lVal;
                DWORD dwFlags         = pdispparams->rgvarg[1].lVal;
                if(dwValidFlagMask & OLECMDIDF_WINDOWSTATE_USERVISIBLE)
                {
                    DP((L"WebBrowserEvent! DISPID_WINDOWSTATECHANGED(%p): %s\n", m_spWebBrowser2, (dwFlags&OLECMDIDF_WINDOWSTATE_USERVISIBLE)?L"Show":L"Hide"));
                }
                if(dwValidFlagMask & OLECMDIDF_WINDOWSTATE_ENABLED)
                {
                    DP((L"WebBrowserEvent! DISPID_WINDOWSTATECHANGED(%p): %s\n", m_spWebBrowser2, (dwFlags&OLECMDIDF_WINDOWSTATE_ENABLED)?L"Enabled":L"Disabled"));
                    if(dwFlags&OLECMDIDF_WINDOWSTATE_USERVISIBLE)
                    {
                        CComBSTR strURL;
                        m_bAvtive = TRUE;
                        g_strUploadUrl = m_siteUrl;
                        hr = m_spWebBrowser2->get_LocationURL(&strURL);
                        if(SUCCEEDED(hr) && strURL.m_str)
                        {
                            m_siteUrl = strURL.m_str;
                            if(m_siteUrl.empty() ||
                                0==_wcsicmp(m_siteUrl.c_str(), L"about:Tabs") ||
                                0==_wcsicmp(m_siteUrl.c_str(), L"about:blank"))
                            {
                                m_siteUrl = L"";
                                g_strUploadUrl = L"";
                            }
                            else
                            {
                                g_strUploadUrl = m_siteUrl;
								writeURLToTmpFile(g_strUploadUrl.c_str());
                            }
                        }
						// here we cache the current active web site.
						g_strPDFDocumentPath = g_strUploadUrl;
						g_thePDFEva.CacheCurrentPDFPath(g_strPDFDocumentPath);
                        DP((L"Active URL = %s\n", g_strUploadUrl.c_str()));
                    }
                    else
                    {
                        m_bAvtive = FALSE;
                    }
                }
            }
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
    CELOG_LOG(CELOG_DUMP, L"Local variables are: hr=0X%08X \n", hr );

    return hr;
}

static int CompareUrl(LPCWSTR wzNav, LPCWSTR wzDeny)
{
	CELOG_LOG(CELOG_DUMP, L" The Parameters are: wzNav=%ls, wzDeny=%ls \n",  (wzNav),  (wzDeny));

    if(0 != wcsnicmp(wzDeny, L"HTTP://", wcslen(L"HTTP://")))
    {
        if(NULL != wcsstr(wzNav, wzDeny))
            return 0;
        else
            return 1;
    }
    else
    {
        return _wcsicmp(wzNav, wzDeny);
    }
}
static void Navigate2DenyPage(CComPtr<IWebBrowser2> spWebBrowser2)
{
	static std::wstring strDenyPage;
    if (strDenyPage.empty())
    {
		 WCHAR denyPage[500] = {0};
         GetModuleFileName(g_hInstance, denyPage, 500); 
		 strDenyPage.append(denyPage);
		 int index = strDenyPage.rfind(L"\\");
		 strDenyPage = strDenyPage.substr(0, index);
		 strDenyPage.append(L"\\ce_deny.html");
    }

	/*CComVariant url(strDenyPage.c_str());*/
	//for bug 35772, navigate to local file the second tab would not do enter BEFORE_NAVGATION event.
	CComVariant url(L"about:blank");
	HRESULT hr = spWebBrowser2->Navigate2(&url, NULL, NULL, NULL, NULL);
	if(!SUCCEEDED(hr))
    {
		DP((L"Navigate2DenyPage failed, errorcode:%d", hr));
    }
}
void WebBrowserEventHandler::OnBeforeNavigate2(LPCWSTR wzUrl, CComPtr<IWebBrowser2> spRelatedWebBrowser2, VARIANT_BOOL* pCancel)
{
	CELOG_LOG(CELOG_ERROR, L" The Parameters are: wzUrl=%ls, spRelatedWebBrowser2=%p, pCancel=%p  \n",  (wzUrl), &spRelatedWebBrowser2, pCancel);

	if ( NULL != pActionHandler )
	{
		if ( !pActionHandler->OpenAction ( wzUrl ) )
		{
			*pCancel = TRUE;
			Navigate2DenyPage(spRelatedWebBrowser2);
			return;
		}
	}
}

void WebBrowserEventHandler::OnQuit()
{
    DP((L"WebBrowserEventHandler::DISPID_QUIT"));
    KeepIePepEnabled();
    CleanHtmlDocumentHandler(m_spWebBrowser2);
    SELF_DELETE();
}
void WebBrowserEventHandler::OnDocumentComplete(CComPtr<IWebBrowser2> spRelatedWebBrowser2)
{
	CELOG_LOG(CELOG_DUMP, L" The Parameters are: spRelatedWebBrowser2=%p \n", &spRelatedWebBrowser2 );

    HRESULT hr = S_OK;
    CComPtr<IDispatch>  spDispDocument2 = NULL;
    CComQIPtr<IUnknown,&IID_IUnknown>     spUnknown ;

    hr = spRelatedWebBrowser2->get_Document(&spDispDocument2);
    if(SUCCEEDED(hr) && spDispDocument2)
    {
        hr = spDispDocument2->QueryInterface(IID_IUnknown, (void**)&spUnknown);

        if(SUCCEEDED(hr) && spUnknown)
        {
            CComQIPtr<IHTMLDocument2> spDocument2 (spUnknown);
			if (spDocument2 == NULL)
			{
				return;
			}
            HookShowModalDialog(spDocument2, TRUE);
            boost::shared_ptr<HTMLDocumentEventHandler> spDocumentHandler(new HTMLDocumentEventHandler(spDocument2, spRelatedWebBrowser2, (LPVOID)this));                        
            if(spDocumentHandler->RegisterEventDispatch(spUnknown))
            {
            }
            else
            {
                DP((L"Fail to sink document event (0x%X)\n", spDocument2));
            }
            ::EnterCriticalSection(&m_htmlDocHandlerCS);
            m_HtmlDocumentHandlers.push_back(spDocumentHandler);
            ::LeaveCriticalSection(&m_htmlDocHandlerCS); 

			//disable the edit control of "file" element on IE6.0 kevin zhou 2008-4-28
			DisableInput(spDocument2);
		
        }
    }
    CELOG_LOG(CELOG_DUMP, L"Local variables are: hr=0X%08X \n", hr );

}

void WebBrowserEventHandler::CleanHtmlDocumentHandler(CComPtr<IWebBrowser2> spTopSite)
{
    ::EnterCriticalSection(&m_htmlDocHandlerCS);
    for (_HTMLDOCUMENTEVENTHANDLERS::iterator it=m_HtmlDocumentHandlers.begin();
        it!=m_HtmlDocumentHandlers.end(); ++it)
    {
        if(spTopSite != (*it)->GetRelatedWebBrowser2())
            (*it)->StopBeforeQuit();
        (*it)->UnregisterEventDispatch();
    }
    m_HtmlDocumentHandlers.clear();
    ::LeaveCriticalSection(&m_htmlDocHandlerCS);    
}

void WebBrowserEventHandler::OnNavigateComplete2(LPCWSTR wzUrl, CComPtr<IWebBrowser2> spRelatedWebBrowser2)
{
	if ( NULL != pActionHandler )
	{
		if ( !pActionHandler->OpenAction ( wzUrl ) )
		{
			Navigate2DenyPage(spRelatedWebBrowser2);
			return;
		}
	}
	return;
}
