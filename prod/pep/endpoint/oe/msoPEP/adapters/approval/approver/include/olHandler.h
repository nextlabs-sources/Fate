
#pragma once
#ifndef _OL_APROVER_HANDLER_H_
#define _OL_APROVER_HANDLER_H_
#include "../YLIB/base_event_handler.h"
#include "emailprocess.h"
#include "stdafx.h"
#include "multiftp.h"
namespace YLIB
{

}

typedef std::vector<std::wstring>   ApproveFilesVector;
#define APPROVAL_EMAIL_SUBJECT      L"Your Request Is Approved"
#define REJECT_EMAIL_SUBJECT        L"Your Request Is Rejected"
#define APPROVAL_EMAIL_FLAG         L"<P><font face=\"Arial\" size=\"5\" color=\"red\">This Request Has Been Approved</font></P>\r\n"
HRESULT GetCurrentUser(Outlook::_olApplication& olApp,std::wstring & strSenderAddress,bool bDisplayName=false);
enum ButtonType{BTP_REJECT=0, BTP_APPROVE, BTP_UPLOAD, BTP_COSTUB};
class OlCommandBarButtonHandler : public base_event_handler<&__uuidof(Office::_CommandBarButtonEvents)>
{
public:
    OlCommandBarButtonHandler(CComPtr<Outlook::_Inspector> spInspector, DWORD dwType):m_spInspector(spInspector), m_dwType(dwType),m_pae(NULL){};
    virtual ~OlCommandBarButtonHandler(){};
    HRESULT OnInvoke(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pdispparams, VARIANT* pvarResult, EXCEPINFO* pexcepinfo, UINT* puArgErr);
	void SetApprovalEmail(ApprovalEmail*ae){m_pae=ae;};
	void SetFtpSite(CFtpSite*pSite){m_pSite=pSite;};
	void SetMatchType(CMultiFTPManager::MATCH_TYPE matchType){m_matchType=matchType;};

	
protected:
    void OnClick(IN CommandBarButton* Ctrl, IN OUT VARIANT_BOOL* CancelDefault);
    void OnApprove(ApprovalEmail& ae);
    void OnReject(ApprovalEmail& ae);
	void OnUpload(ApprovalEmail& ae);
#ifdef _DEBUG
    void OnCOStub();
#endif

    HRESULT SendMail(CComPtr<Outlook::_olApplication> spApp, LPCWSTR pwzTo, LPCWSTR pwzCC, LPCWSTR pwzSubject, LPCWSTR pwzHTMLBody);
    std::wstring ComposeRejectMail(ApprovalEmail& ae);
    std::wstring ComposeApproveMail(ApprovalEmail& ae, ApproveFilesVector& vAFiles);
    std::wstring ComposeRequestMailStub();
    std::wstring MakeOriginalRequest(ApprovalEmail& ae);

private:
    DWORD                   m_dwType;       // 0:Reject, 1:Approve, 
    CComPtr<Outlook::_Inspector>    m_spInspector;
	ApprovalEmail*			m_pae;
	CFtpSite*				m_pSite;
	CMultiFTPManager::MATCH_TYPE					m_matchType;
};

class OlInspectorHandler : public base_event_handler<&__uuidof(Outlook::InspectorEvents)>
{
public:
    OlInspectorHandler(CComPtr<Outlook::_Inspector> spInspector):m_spInspector(spInspector),m_pae(NULL){};
    virtual ~OlInspectorHandler(){if(m_pae)delete m_pae;if(m_ftpSite)delete m_ftpSite;};
    HRESULT OnInvoke(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pdispparams, VARIANT* pvarResult, EXCEPINFO* pexcepinfo, UINT* puArgErr);

public:
    BOOL AddApprovalToolbar(BOOL bRequestMail,BOOL bFtped,BOOL bApproved,bool bMisMatched=false);
	void SetApprovalEmail(ApprovalEmail*ae){m_pae=ae;}
	void SetMatchType(CMultiFTPManager::MATCH_TYPE matchType){m_matchType=matchType;};
	void SetFtpSite(CFtpSite*pSite){m_ftpSite=pSite;};

protected:
    void OnActivate();
    void OnDeactivate();
    void OnClose();

protected:
    BOOL GetToolbarButtons(Office::_CommandBars* spCmdBars);
    BOOL CreateToolbarButtons(Office::_CommandBars* spCmdBars);

private:
	CFtpSite*							  m_ftpSite;
	CMultiFTPManager::MATCH_TYPE		  m_matchType;
	ApprovalEmail*						  m_pae;
    CComPtr<Outlook::_Inspector>          m_spInspector;
    CComPtr<Office::_CommandBarButton>    m_spBtnApprove;
    CComPtr<Office::_CommandBarButton>    m_spBtnReject;
    YLIB::COMMON::smart_ptr<OlCommandBarButtonHandler>  m_spApproveHandler;
    YLIB::COMMON::smart_ptr<OlCommandBarButtonHandler>  m_spRejectHandler;

	CComPtr<Office::_CommandBarButton>    m_spBtnUpload;
    YLIB::COMMON::smart_ptr<OlCommandBarButtonHandler>  m_spUploadHandler;

#ifdef _DEBUG
    CComPtr<Office::_CommandBarButton>    m_spBtnCOStub;
    YLIB::COMMON::smart_ptr<OlCommandBarButtonHandler>  m_spCOStubHandler;
#endif
};

class OlInspectorsHandler : public base_event_handler<&__uuidof(Outlook::InspectorsEvents)>
{
public:
    OlInspectorsHandler(){};
    virtual ~OlInspectorsHandler(){};
    HRESULT OnInvoke(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pdispparams, VARIANT* pvarResult, EXCEPINFO* pexcepinfo, UINT* puArgErr);

protected:
    void OnNewInspector(CComQIPtr<Outlook::_Inspector> Inspector);

protected:
    BOOL IsApprovalRequest(CComPtr<Outlook::_Inspector> Inspector,BOOL* bFtped=NULL, BOOL* bApproved=NULL);
};
#endif