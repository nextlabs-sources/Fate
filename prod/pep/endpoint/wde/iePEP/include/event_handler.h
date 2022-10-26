
#pragma once
#ifndef _EVENT_HANDLER_H_
#define _EVENT_HANDLER_H_
#pragma warning(disable: 4584 4996)
#include "base_event_handler.h"
#include <vector>

#pragma warning(push)
#pragma warning(disable: 4512 4244) 
#include <boost/smart_ptr.hpp> 
#pragma warning(pop)


typedef std::vector<std::wstring>   _STRINGLIST;

class HTMLFormEventHandler : public base_event_handler<&__uuidof(HTMLFormElementEvents2)>
{
public:
    HTMLFormEventHandler(CComPtr<IHTMLFormElement> spFormElem, CComPtr<IWebBrowser2> spRelatedWebBrowser2, LPVOID spParentHandler):m_spFormElem(spFormElem), m_spRelatedWebBrowser2(spRelatedWebBrowser2), m_spParentHandler(spParentHandler)
    {
    };
    ~HTMLFormEventHandler()
    {
    }
    HRESULT OnInvoke(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pdispparams, VARIANT* pvarResult, EXCEPINFO* pexcepinfo, UINT* puArgErr);

protected:
    void OnSubmit();
private:
    CComPtr<IHTMLFormElement>       m_spFormElem;
    CComPtr<IWebBrowser2>           m_spRelatedWebBrowser2;
    LPVOID                  m_spParentHandler;
};
typedef std::vector<boost::shared_ptr<HTMLFormEventHandler>>    _HTMLFORMEVENTHANDLERS;

class HTMLDocumentEventHandler : public base_event_handler<&__uuidof(HTMLDocumentEvents)>
{
public:
    HTMLDocumentEventHandler(CComQIPtr<IHTMLDocument2> spHTMLDocument2, CComPtr<IWebBrowser2> spRelatedWebBrowser2, LPVOID spParentHandler):m_spHTMLDocument2(spHTMLDocument2), m_spRelatedWebBrowser2(spRelatedWebBrowser2), m_spParentHandler(spParentHandler){};
    ~HTMLDocumentEventHandler()
    {
        if(NULL!=m_spHTMLDocument2 && NULL!=m_spRelatedWebBrowser2)
        {
            m_HtmlFormHandlers.clear();
        }
    }

    void StopBeforeQuit()
    {
        if(NULL != m_spRelatedWebBrowser2)
        {
            m_spRelatedWebBrowser2->Stop();
        }
    }
    
    HRESULT OnInvoke(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pdispparams, VARIANT* pvarResult, EXCEPINFO* pexcepinfo, UINT* puArgErr);

    inline CComPtr<IWebBrowser2> GetRelatedWebBrowser2(){return m_spRelatedWebBrowser2;};
    inline CComQIPtr<IHTMLDocument2> GetRelatedDocument2(){return m_spHTMLDocument2;};
    void SinkFormEvent();
private:
    CComPtr<IWebBrowser2>           m_spRelatedWebBrowser2;
    CComQIPtr<IHTMLDocument2>         m_spHTMLDocument2;
    _HTMLFORMEVENTHANDLERS  m_HtmlFormHandlers;
    LPVOID                  m_spParentHandler;
};
typedef std::vector<boost::shared_ptr<HTMLDocumentEventHandler>>    _HTMLDOCUMENTEVENTHANDLERS;

class WebBrowserEventHandler : public base_event_handler<&__uuidof(DWebBrowserEvents2)>
{
public:
    WebBrowserEventHandler(CComPtr<IWebBrowser2> spWebBrowser2):m_spWebBrowser2(spWebBrowser2),m_bAvtive(FALSE)
    {
        ::InitializeCriticalSection(&m_htmlDocHandlerCS);
    }
    ~WebBrowserEventHandler()
    {
        ::DeleteCriticalSection(&m_htmlDocHandlerCS);
    }
    HRESULT OnInvoke(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pdispparams, VARIANT* pvarResult, EXCEPINFO* pexcepinfo, UINT* puArgErr);

    void OnBeforeNavigate2(LPCWSTR wzUrl, CComPtr<IWebBrowser2> spRelatedWebBrowser2,VARIANT_BOOL* pCancel);
    void OnNavigateComplete2();
    void OnNewWindow2();
    void OnQuit();
    void OnDocumentComplete(CComPtr<IWebBrowser2> spRelatedWebBrowser2);
	void OnNavigateComplete2(LPCWSTR wzUrl, CComPtr<IWebBrowser2> spRelatedWebBrowser2);

    void CleanHtmlDocumentHandler(CComPtr<IWebBrowser2> spTopSite);

    _STRINGLIST                 m_vDenyUrlList;

private:
    CComPtr<IWebBrowser2>               m_spWebBrowser2;
    _HTMLDOCUMENTEVENTHANDLERS  m_HtmlDocumentHandlers;
    CRITICAL_SECTION            m_htmlDocHandlerCS;
    std::wstring                m_preNavUrl;
    std::wstring                m_siteUrl;
    BOOL                        m_bAvtive;
};


#endif