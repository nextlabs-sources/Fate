
#pragma once
#include "stdafx.h"
#include <Windows.h>
#include "outlookEventDisp.h"
#include "../../../common/include/nlexcept.h"

/************************************************************************/
/* Outlook::Application Event Dispatch                                  */
/************************************************************************/
CBaseAppEventDisp::CBaseAppEventDisp()
{
}

CBaseAppEventDisp::~CBaseAppEventDisp()
{
}

STDMETHODIMP CBaseAppEventDisp::Invoke(DISPID dispidMember,
                                       REFIID riid,
                                       LCID lcid,
                                       WORD wFlags,
                                       DISPPARAMS* pdispparams,
                                       VARIANT* pvarResult,
                                       EXCEPINFO* pexcepinfo,
                                       UINT* puArgErr)
{
    HRESULT hr = S_OK;

    __try
    {

    if	(pdispparams && DISPATCH_METHOD==wFlags)
    {
        switch	(dispidMember)
        {
        case 0xf002:    // ItemSend
            OnItemSend(pdispparams->rgvarg[0].pdispVal, pdispparams->rgvarg[1].pboolVal);
            break;
        case 0xf003:    // NewMail
            OnNewMail();
            break;
        case 0xf004:    // Reminder
            OnReminder(pdispparams->rgvarg[0].pdispVal);
            break;
        case 0xf005:    // OptionsPagesAdd
            OnOptionsPagesAdd((Outlook::PropertyPages *)(pdispparams->rgvarg[0].pdispVal));
            break;
        case 0xf006:    // Startup
            OnStartup();
            break;
        case 0xf007:    // Quit
            OnQuit();
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
    }
    __except( NLEXCEPT_FILTER() )
    {

    }
    return hr;
}

void CBaseAppEventDisp::OnItemSend(CComPtr<IDispatch> Item, VARIANT_BOOL* Cancel)
{
}

void CBaseAppEventDisp::OnNewMail()
{
}

void CBaseAppEventDisp::OnReminder(CComPtr<IDispatch> Item)
{
}

void CBaseAppEventDisp::OnOptionsPagesAdd(PropertyPages* Pages)
{
}

void CBaseAppEventDisp::OnStartup()
{
}

void CBaseAppEventDisp::OnQuit()
{
}

/************************************************************************/
/* Outlook::Explorer Event Dispatch                                     */
/************************************************************************/
CBaseExplEventDisp::CBaseExplEventDisp()
{
}

CBaseExplEventDisp::~CBaseExplEventDisp()
{
}

STDMETHODIMP CBaseExplEventDisp::Invoke(DISPID dispidMember,
                                        REFIID riid,
                                        LCID lcid,
                                        WORD wFlags,
                                        DISPPARAMS* pdispparams,
                                        VARIANT* pvarResult,
                                        EXCEPINFO* pexcepinfo,
                                        UINT* puArgErr)
{
    HRESULT hr = S_OK;

    if	(pdispparams && DISPATCH_METHOD==wFlags)
    {
        switch	(dispidMember)
        {
        case 0xf001:    // Activate
            OnActivate();
            break;
        case 0xf002:    // FolderSwitch
            OnFolderSwitch();
            break;
        case 0xf003:    // BeforeFolderSwitch
            OnBeforeFolderSwitch(pdispparams->rgvarg[0].pdispVal, pdispparams->rgvarg[1].pboolVal);
            break;
        case 0xf004:    // ViewSwitch
            OnViewSwitch();
            break;
        case 0xf005:    // BeforeViewSwitch
            OnBeforeViewSwitch(pdispparams->rgvarg[0], pdispparams->rgvarg[1].pboolVal);
            break;
        case 0xf006:    // Deactivate
            OnDeactivate();
            break;
        case 0xf007:    // SelectionChange
            OnSelectionChange();
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

void CBaseExplEventDisp::OnActivate()
{
}

void CBaseExplEventDisp::OnFolderSwitch()
{
}

void CBaseExplEventDisp::OnBeforeFolderSwitch(CComPtr<IDispatch> NewFolder, VARIANT_BOOL* Cancel)
{
}

void CBaseExplEventDisp::OnViewSwitch()
{
}

void CBaseExplEventDisp::OnBeforeViewSwitch(VARIANT NewView, VARIANT_BOOL* Cancel)
{
}

void CBaseExplEventDisp::OnDeactivate()
{
}

void CBaseExplEventDisp::OnSelectionChange()
{
}

void CBaseExplEventDisp::OnClose()
{
}


/************************************************************************/
/* Outlook::Inspectors Event Dispatch                                     */
/************************************************************************/
CBaseInspsEventDisp::CBaseInspsEventDisp()
{
}

CBaseInspsEventDisp::~CBaseInspsEventDisp()
{
}

STDMETHODIMP CBaseInspsEventDisp::Invoke(DISPID dispidMember,
                                         REFIID riid,
                                         LCID lcid,
                                         WORD wFlags,
                                         DISPPARAMS* pdispparams,
                                         VARIANT* pvarResult,
                                         EXCEPINFO* pexcepinfo,
                                         UINT* puArgErr)
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

void CBaseInspsEventDisp::OnNewInspector(CComQIPtr<Outlook::_Inspector> Inspector)
{
    HRESULT hr = S_OK;
    CComObject<CBaseInspEventDisp>*	pInspDisp = NULL;
    CComPtr<IUnknown> m_pUnkInspDisp = 0;

    if(Inspector)
    {
        CComPtr<Outlook::_MailItem> spMailItem = 0;
        hr = Inspector->get_CurrentItem((IDispatch**)&spMailItem);
        if(SUCCEEDED(hr) && spMailItem)
        {
            DP((L"\n***\nMailItem = 0X%X\n", spMailItem));
        }
        else
        {
            DP((L"\n***\nFail to get mail item\n"));
        }

    }

    hr = CComObject<CBaseInspEventDisp>::CreateInstance(&pInspDisp);
    if(SUCCEEDED(hr) && pInspDisp)
    {
        DP((L"Inspector = 0X%X\nInspDisp = 0X%X\n", Inspector, pInspDisp));
        hr = pInspDisp->QueryInterface(IID_IUnknown, (void**)&m_pUnkInspDisp);
        if(SUCCEEDED(hr) && m_pUnkInspDisp)
        {
            DP((L"pUnknown = 0X%X", m_pUnkInspDisp));
        }
        DP((L"***\n"));
    }
}


/************************************************************************/
/* Outlook::Inspector Event Dispatch                                     */
/************************************************************************/
CBaseInspEventDisp::CBaseInspEventDisp()
{
    m_spInsp        = NULL;
    m_dwInspCookie  = NULL;
    m_dwItemCookie  = NULL;
}

CBaseInspEventDisp::~CBaseInspEventDisp()
{
}

STDMETHODIMP CBaseInspEventDisp::Invoke(DISPID dispidMember,
                                        REFIID riid,
                                        LCID lcid,
                                        WORD wFlags,
                                        DISPPARAMS* pdispparams,
                                        VARIANT* pvarResult,
                                        EXCEPINFO* pexcepinfo,
                                        UINT* puArgErr)
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

void CBaseInspEventDisp::OnActivate()
{
}

void CBaseInspEventDisp::OnDeactivate()
{
}

void CBaseInspEventDisp::OnClose()
{
}

/************************************************************************/
/* Outlook::Item Event Dispatch                                         */
/************************************************************************/
CBaseItemEventDisp::CBaseItemEventDisp()
{
}

CBaseItemEventDisp::~CBaseItemEventDisp()
{
}

STDMETHODIMP CBaseItemEventDisp::Invoke(DISPID dispidMember,
                                        REFIID riid,
                                        LCID lcid,
                                        WORD wFlags,
                                        DISPPARAMS* pdispparams,
                                        VARIANT* pvarResult,
                                        EXCEPINFO* pexcepinfo,
                                        UINT* puArgErr)
{
    HRESULT hr = S_OK;

    if	(pdispparams && DISPATCH_METHOD==wFlags)
    {
        switch	(dispidMember)
        {
        case 0xf001:    // Read
            OnRead();
            break;
        case 0xf002:    // Write
            OnWrite(pdispparams->rgvarg[0].pboolVal);
            break;
        case 0xf003:    // Open
            OnOpen(pdispparams->rgvarg[0].pboolVal);
            break;
        case 0xf004:    // Close
            OnClose(pdispparams->rgvarg[0].pboolVal);
            break;
        case 0xf005:    // Send
            OnSend(pdispparams->rgvarg[0].pboolVal);
            break;
        case 0xf006:    // CustomAction
            OnCustomAction(pdispparams->rgvarg[0].pdispVal,
                           pdispparams->rgvarg[1].pdispVal,
                           pdispparams->rgvarg[2].pboolVal);
            break;
        case 0xf008:    // CustomPropertyChange
            OnCustomPropertyChange(pdispparams->rgvarg[0].bstrVal);
            break;
        case 0xf009:    // PropertyChange
            OnPropertyChange(pdispparams->rgvarg[0].bstrVal);
            break;
        case 0xf00a:    // BeforeCheckNames
            OnBeforeCheckNames(pdispparams->rgvarg[0].pboolVal);
            break;
        case 0xf00b:    // AttachmentAdd
            OnAttachmentAdd((Outlook::Attachment *)(pdispparams->rgvarg[0].pdispVal));
            break;
        case 0xf00c:    // AttachmentRead
            OnAttachmentRead((Outlook::Attachment *)(pdispparams->rgvarg[0].pdispVal));
            break;
        case 0xf00d:    // BeforeAttachmentSave
            OnBeforeAttachmentSave((Outlook::Attachment *)(pdispparams->rgvarg[0].pdispVal),
                                   pdispparams->rgvarg[1].pboolVal);
            break;
        case 0xf466:    // Reply
            OnReply(pdispparams->rgvarg[1].pdispVal, pdispparams->rgvarg[0].pboolVal);
            break;
        case 0xf467:    // ReplyAll
            OnReplyAll(pdispparams->rgvarg[1].pdispVal, pdispparams->rgvarg[0].pboolVal);
            break;
        case 0xf468:    // Forward
            OnForward(pdispparams->rgvarg[1].pdispVal, pdispparams->rgvarg[0].pboolVal);
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

void CBaseItemEventDisp::OnOpen(VARIANT_BOOL* Cancel)
{
}

void CBaseItemEventDisp::OnCustomAction(CComPtr<IDispatch> Action, CComPtr<IDispatch> Response, VARIANT_BOOL* Cancel)
{
}

void CBaseItemEventDisp::OnCustomPropertyChange(BSTR Name)
{
}

void CBaseItemEventDisp::OnForward(CComPtr<IDispatch> Forward, VARIANT_BOOL* Cancel)
{
}

void CBaseItemEventDisp::OnClose(VARIANT_BOOL* Cancel)
{
}

void CBaseItemEventDisp::OnPropertyChange(BSTR Name)
{
}

void CBaseItemEventDisp::OnRead()
{
}

void CBaseItemEventDisp::OnReply(CComPtr<IDispatch> Response, VARIANT_BOOL* Cancel)
{
}

void CBaseItemEventDisp::OnReplyAll(CComPtr<IDispatch> Response, VARIANT_BOOL* Cancel)
{
}

void CBaseItemEventDisp::OnSend(VARIANT_BOOL* Cancel)
{
}

void CBaseItemEventDisp::OnWrite(VARIANT_BOOL* Cancel)
{
}

void CBaseItemEventDisp::OnBeforeCheckNames(VARIANT_BOOL* Cancel)
{
}

void CBaseItemEventDisp::OnAttachmentAdd(Attachment* Attachment)
{
}

void CBaseItemEventDisp::OnAttachmentRead(Attachment* Attachment)
{
}

void CBaseItemEventDisp::OnBeforeAttachmentSave(Attachment* Attachment, VARIANT_BOOL* Cancel)
{
}
