// ItemEventDisp.h : Declaration of the CItemEventDisp

#pragma once
#include "resource.h"       // main symbols

#include "msoPEP.h"
#include "InspEventDisp.h"
#include "dmDlg.h"
#include "tagDlg.h"
#include <hash_map>

#include "..\Yeled\YeledDlg.h"
#include "assistantdlg.h"
#include "Hyperlink.h"
#define		PRVIEW_EVENT		(1)
#define		READ_EVENT			(1<<1)
#define		WRITE_EVENT			(1<<2)
#define		SEND_EVENT			(1<<3)
#define		CLOSE_EVENT			(1<<4)
#define		REPLAY_EVENT		(1<<5)
#define		ALL_EVENTS			(0xff)
class CSendEmailData;
class COE_PAMngr;


#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Single-threaded COM objects are not properly supported on Windows CE platform, such as the Windows Mobile platforms that do not include full DCOM support. Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support creating single-thread COM object's and allow use of it's single-threaded COM object implementations. The threading model in your rgs file was set to 'Free' as that is the only threading model supported in non DCOM Windows CE platforms."
#endif


BOOL EndWith(std::wstring& strValue, const WCHAR wcValue);
void ResetDisplayNameForAttachments(BOOL bEmptyAttachment,int nAttachments,ATTACHMENTLIST&listAttachments,CComPtr<IDispatch> pMailItem );
void AddFakeAttachmentIfNeeded(ATTACHMENTLIST& listAttachments,
							   int& nAttachments,
							   BOOL& bEmptyAttachment);
void GetRealPath(LPCWSTR wzPath, LPWSTR wzRealPath, int cch);
BOOL IsMapped(LPCWSTR wzPath, LPWSTR wzRealPath, int cch);
BOOL IsLocalDrive(LPCWSTR wzPath);

class CDmAssistDlgEx : public CAssistantDialog
{
public:
	typedef struct _CONTEXT
	{
		STRINGLIST* pListRecipients;
		int* pnRecipients;
		std::vector<int>* pListRecipientIndices;
		std::vector<BOOL>* pListRecipientDmFlags;
		std::vector<BOOL>* pListRecipientMcFlags;
		vector<CAttachmentData>* pvecAttachmentData;
		int* pnAttachments;
		BOOL* pbNeedHdr;
		BOOL  bCheckRecipientLicense;
		std::wstring strMessage;
		BOOL  bSelectable;
		BOOL  bExistDefaultRecipients;
		CSendEmailData *emailData;
	}CONTEXT, *LPCONTEXT;

	CDmAssistDlgEx(LPCONTEXT pDmContext);

	void ProcessOK();
	void ProcessCancel();
	void ProcessItemRemove(int nItem);
	void ProcessItemCheck(int nItem, BOOL bChecked);
	void ProcessItemCbnSelect(int nItem, int nSel);
	void AddDmItemData(BOOL bMultiUser, BOOL bMultiClient, BOOL bSelectable, LPCWSTR pwzRecipient, STRINGPAIRVECTOR& pairString);


protected:
	void AddViewData();
	void ReallocWindows();

	std::vector<CDmData> m_dmData;
	LPCONTEXT            m_pDmContext;
	CHyperlink           m_HyperLink;

};


class CTagAssistDlgEx : public CAssistantDialog
{
public:
	typedef struct _CONTEXT
	{
		vector<CAttachmentData>* pvecAttachmentData;
	}CONTEXT, *LPCONTEXT;

	CTagAssistDlgEx(LPCONTEXT pTagContext);
	void ProcessOK();
	void ProcessCancel();
	void ProcessItemCbnSelect(int nItem, int nSel);
	void AddTagItemData(LPCWSTR pwzAttach, STRINGVECTOR& vecClients);
	void AddTagItemData(LPCWSTR pwzAttach, LPCWSTR pwzClients);
	static void ParseClients(LPCWSTR pwzClients, STRINGVECTOR& vClients);
protected:
	void AddViewData();
	void ReallocWindows();
	std::vector<CTagData>   m_tagData;
	LPCONTEXT               m_pTagContext;
};



class CMAttrParAssistDlg : public CAssistantDialog
{
public:
	CMAttrParAssistDlg();
	virtual ~CMAttrParAssistDlg();
	
	void SetMAttrParType( const BOOL& bIsBlock ,const STRINGLIST& listRecipients);
	void SetMAttrParMessage( const std::wstring strMsgType, const std::wstring strMsg );
	void AddDmItemData( LPCWSTR pwzRecipient/*, STRINGPAIRVECTOR& pairString*/);
protected:
	void AddViewData();
	void ReallocWindows();
	std::wstring m_MessageInfo;
	std::wstring m_strmsgType ;
	std::vector<CMapData> m_mapData;
	BOOL m_IsBlock ;//This message has warning or block
};
class CInterAssistDlg : public CAssistantDialog
{
public:
	CInterAssistDlg();
	virtual ~CInterAssistDlg();
	
	void AddInterItemData(LPCWSTR pwzAttach);
protected:
	void AddViewData();
	void ReallocWindows();
	STRINGVECTOR m_Files;
};



// CItemEventDisp
class ATL_NO_VTABLE CItemEventDisp :
    public CComObjectRootEx<CComSingleThreadModel>,
    public IDispatch,
	public CComCoClass<CItemEventDisp, &CLSID_ItemEventDisp>,
	public IDispatchImpl<IItemEventDisp, &IID_IItemEventDisp, &LIBID_msoPEPLib, /*wMajor =*/ 1, /*wMinor =*/ 0>
{
public:
	CItemEventDisp()
	{
		m_bCancel = FALSE ;
		m_PCCancel = FALSE ;
		m_bLaunchedBy3rdPart = FALSE;
		m_nEventNeedSink = ALL_EVENTS;//default monitor all events(PRVIEW_EVENT + READ_EVENT + WRITE_EVENT + REPLAY_EVENT + SEND_EVENT + CLOSE_EVENT)
	}

DECLARE_REGISTRY_RESOURCEID(IDR_ITEMEVENTDISP)


BEGIN_COM_MAP(CItemEventDisp)
	COM_INTERFACE_ENTRY(IItemEventDisp)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY_IID(__uuidof(Outlook::ItemEvents), IDispatch)
END_COM_MAP()



	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease()
	{
	}

public:
	int m_nEventNeedSink;
    STDMETHOD(GetTypeInfoCount)(UINT* pctinfo){UNREFERENCED_PARAMETER(pctinfo); return	E_NOTIMPL;};
    STDMETHOD(GetTypeInfo)(UINT itinfo, LCID lcid, ITypeInfo** pptinfo)
    {
        UNREFERENCED_PARAMETER(itinfo);
        UNREFERENCED_PARAMETER(lcid);
        UNREFERENCED_PARAMETER(pptinfo);
        return	E_NOTIMPL;
    };
    STDMETHOD(GetIDsOfNames)(REFIID riid, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgdispid)
    {
        UNREFERENCED_PARAMETER(riid);
        UNREFERENCED_PARAMETER(rgszNames);
        UNREFERENCED_PARAMETER(cNames);
        UNREFERENCED_PARAMETER(lcid);
        UNREFERENCED_PARAMETER(rgdispid);
        return	E_NOTIMPL;
    };
    STDMETHOD(Invoke)(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pdispparams, VARIANT* pvarResult, EXCEPINFO* pexcepinfo, UINT* puArgErr);

public:
    inline void SetMailItemPtr(CComPtr<IDispatch> spMailItem)
    {
        m_spMailItem = spMailItem;
    };
    void OnOpen();//VARIANT_BOOL* Cancel);
	void OnSend(VARIANT_BOOL* Cancel, bool bInline = false);
	void OnSendEx(VARIANT_BOOL* Cancel, bool bInline = false);
	void OnPreviewAttachment(IDispatch * lpDispItem,VARIANT_BOOL* Cancel);
	bool ParseAssignTaskItem(const wchar_t *pTaskItemPath, CSendEmailData *pTaskData, CComPtr<IDispatch> &taskItem, bool bInline, ITEM_TYPE origEmailType);

private:
    BOOL DoEnforcement(CSendEmailData *pOuterEmailData, CComPtr<IDispatch> spItem, VARIANT_BOOL* Cancel, bool bInline, ITEM_TYPE origEmailType);

protected:
    void OnCustomAction(CComPtr<IDispatch> Action, CComPtr<IDispatch> Response, VARIANT_BOOL* Cancel);
    void OnCustomPropertyChange(BSTR Name);
    void OnForward(CComPtr<IDispatch> Forward, VARIANT_BOOL* Cancel);
    void OnClose(VARIANT_BOOL* Cancel);
    void OnPropertyChange(BSTR Name);
    void OnRead();
    void OnReply(CComPtr<IDispatch> Response, VARIANT_BOOL* Cancel);
    void OnReplyAll(CComPtr<IDispatch> Response, VARIANT_BOOL* Cancel);
   
    void OnWrite(VARIANT_BOOL* Cancel);
    void OnBeforeCheckNames(VARIANT_BOOL* Cancel);
   
    void OnAttachmentRead(Attachment* Attachment);
    void OnBeforeAttachmentSave(Attachment* Attachment, VARIANT_BOOL* Cancel);

	
	void ReAddAttachmentForForwardMail();
	void GetAttachmentSourcePathFor3rdPartSend(LPCWSTR pwzSubject, BOOL bNeedSave = TRUE);
	 //bool	UpdateEmailContent(CSendEmailData & EmailData,CComPtr<IDispatch> dspMailItem, BOOL bInline);
	 bool ReplaceAssignTaskContent(CComPtr<IDispatch> dspTaskItem);//assign task request or task
private:
	void DoEmailApprovalForFlex(ATTACHMENTLIST&   listAttachments,CEAttributes *obligation);
    void CollectAllOETempFileInfo(CSendEmailData& emailData);
    void _OnSendEx(VARIANT_BOOL* Cancel, bool bInline = false);     // just for capture exception
	void ActionOnQueryPCFinish(CSendEmailData& emailData, VARIANT_BOOL* Cancel, bool bInline, bool bQueryResult);
    void SaveCurrentActiveWindow();

	void Init(CComPtr<IDispatch> dspMailItem,CSendEmailData &emailData,CAttachmentObligationData & AttachmentOblData, bool bInline);
	bool UpdateEmailContent(CSendEmailData & EmailData,CComPtr<IDispatch> dspMailItem, BOOL bInline);
	void SaveAttachmentDumb(CComPtr<IDispatch> dspMailItem);

	void SyncInformationBeforeUpdateContent(COE_PAMngr* paMngr, CSendEmailData* pEmailData);
    void ReAttachAllAttachmentForOtherSend();
	void DeleteCustomerPropertiesForAttachment(CComPtr<Outlook::Attachment> dspAttachment);
	void DeleteCustomerProperties(CComPtr<IDispatch> dspMailItem);
	int  GetMailSendTimes();
	void SetMailSendTimes(int nSendTimes);
	// copy the message header from the current item #m_spMailItem to a new item
	BOOL CopyMessageHeaderTo(IDispatch *pItemDisp);

	//void SaveNoteFileContent(CComPtr<IDispatch> dspNoteItem);   

	BOOL ExecuteRichAlertMessage(CSendEmailData *pEmailDataToHandle, VARIANT_BOOL* Cancel);

private:
    CComPtr<IDispatch>		m_spMailItem;
	CComPtr<IDispatch>		m_spSaveOrigMailItem;//1.save the original mail 2.used for assigning task work flow
	CSendEmailData*			m_origEmailData;
    MAILTYPE        m_mtMailType;
	VARIANT_BOOL			m_bCancel ;
	VARIANT_BOOL m_PCCancel ;
	BOOL m_bLaunchedBy3rdPart;

    std::list<std::wstring>  m_lstOETempFile; //for delete OE temp  cache

	//record source path for attachment send by 3th-part app
	//didn't use outlook::attachment* as the key, because the outlook::attachment* valaue for the attachment maybe changed.
	std::map<long, std::wstring> m_map3thAppAttachmentSourcePath;

public:
	BOOL GetLaunchedBy3rdPartFlag() {return m_bLaunchedBy3rdPart;}
	void SetLaunchedBy3rdPartFlag(BOOL bFlag) {m_bLaunchedBy3rdPart = bFlag;}


public:
    static HWND s_hCurActiveWindow;

};


typedef struct _OBLIGATION
{
    std::wstring              name;
    BOOL hasArgs;
    // "values" is used for obligations without arguements, while
    // "argNameValues" is used for obligations with arguments.  Too bad we
    // can't put the two into a union.
    std::vector<std::wstring> values;
    stdext::hash_map<std::wstring, std::wstring> argNameValues;

    _OBLIGATION()
    {
        name = L"";
        hasArgs = FALSE;
        values.clear();
        argNameValues.clear();
    }
}OBLIGATION, *LPOBLIGATION;
typedef std::vector<LPOBLIGATION>   OBLIGATIONS;


class CObligations
{
public:
	CObligations();
	virtual ~CObligations();

	int  GetObligations(CEAttributes *obligation);
    void FreeObligations();
    BOOL CheckHiddenDataRemoval(LPHIDDENDATAREMOVALINFO hdrInfo);
	
    OBLIGATIONS m_Obligations;
	static int ClassifyObligationType(CEEnforcement_t *enforcer,EmQueryResCommand emCommand, CSendEmailData & SendEmailData,int nAttachmentPos,int nRecipientNum,bool &bAllow, DWORD* pdwOblTypes = NULL);

};
typedef std::vector<CObligations*>   OBLIGATIONSLIST;

OBJECT_ENTRY_AUTO(__uuidof(ItemEventDisp), CItemEventDisp)


