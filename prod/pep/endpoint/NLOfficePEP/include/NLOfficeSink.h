// NxtOfficeSink.h : Declaration of the CNxtOfficeSink

#pragma once
#include "resource.h"       // main symbols
#include "NlOfficePEP.h"
#include "NLOfficePEP_Comm.h"
#include "NLHookAPI.h"
#include "OfficeListener.h"

#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Single-threaded COM objects are not properly supported on Windows CE platform, such as the Windows Mobile platforms that do not include full DCOM support. Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support creating single-thread COM object's and allow use of it's single-threaded COM object implementations. The threading model in your rgs file was set to 'Free' as that is the only threading model supported in non DCOM Windows CE platforms."
#endif

// CNxtOfficeSink
class ATL_NO_VTABLE CNxtOfficeSink :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CNxtOfficeSink, &CLSID_NxtOfficeSink>,
	public IDispatchImpl<INxtOfficeSink, &IID_INxtOfficeSink, &LIBID_NxtOfficePEPLib, /*wMajor =*/ 1, /*wMinor =*/ 0>,
	public IDispatchImpl<IRibbonExtensibility, &__uuidof(IRibbonExtensibility), &LIBID_Office, /* wMajor = */ 2, /* wMinor = */ 4>,
	public IDispatchImpl<_IDTExtensibility2, &__uuidof(_IDTExtensibility2), &LIBID_AddInDesignerObjects, /* wMajor = */ 1>
{
public:
	CNxtOfficeSink();
	~CNxtOfficeSink();

public:
	DECLARE_REGISTRY_RESOURCEID(IDR_NXTOFFICESINK)

	BEGIN_COM_MAP(CNxtOfficeSink)
		COM_INTERFACE_ENTRY(INxtOfficeSink)
		COM_INTERFACE_ENTRY(_IDTExtensibility2)
		COM_INTERFACE_ENTRY2(IDispatch, INxtOfficeSink)
		COM_INTERFACE_ENTRY(IRibbonExtensibility)
	END_COM_MAP()

	DECLARE_PROTECT_FINAL_CONSTRUCT()
	
	// _IDTExtensibility2 Methods
public:
	// Office add-in entry
	STDMETHOD(OnConnection)( LPDISPATCH Application, ext_ConnectMode ConnectMode, LPDISPATCH AddInInst, SAFEARRAY * * custom);
	STDMETHOD(OnDisconnection)( ext_DisconnectMode RemoveMode, SAFEARRAY * * custom );
	STDMETHOD(OnAddInsUpdate)(SAFEARRAY * * custom){ return E_NOTIMPL; }
	STDMETHOD(OnStartupComplete)(SAFEARRAY * * custom){ return E_NOTIMPL; }
	STDMETHOD(OnBeginShutdown)(SAFEARRAY * * custom){ return E_NOTIMPL; }
	// IRibbonExtensibility
	STDMETHOD(GetCustomUI)(BSTR RibbonID, BSTR * RibbonXml);
	
	HRESULT FinalConstruct();
	void FinalRelease();

private:
	static BOOL CALLBACK EnumChildProc(HWND hwnd, LPARAM lParam);
	static BOOL CALLBACK EnumThreadWndProc(HWND hwnd, LPARAM lParam);

private:
	void LoadCBModule();
	void UnloadCBModule();

private:
	void CNxtOfficeSink::DoFakeEva();

	wstring	GetOfficeCommandXml();
	//wstring	NLAddNextLabsButton();

private:
	// These functions register for XML ribbon event.  
	//STDMETHOD(OnClickButton)(IDispatch* pCtrl);
	//STDMETHOD(GetImage)(IDispatch* pCtrl, IPictureDisp** pImage);

	STDMETHOD(FileSendAsAttachmentAction)(IDispatch* RibbonControl, VARIANT* cancel);
	STDMETHOD(FileSaveAction)(IDispatch* RibbonControl, VARIANT* cancel);
	STDMETHOD(FileSaveAsAction)(IDispatch* RibbonControl, VARIANT* cancel);
	STDMETHOD(FileEmailAsPdfEmailAttachmentAction)(IDispatch* RibbonControl, VARIANT* cancel);
	STDMETHOD(FileEmailAsXpsEmailAttachmentAction)(IDispatch* RibbonControl, VARIANT* cancel);
	STDMETHOD(FileInternetFaxAction)(IDispatch* RibbonControl, VARIANT* cancel);

	STDMETHOD(OleObjectctInsertAction)(IDispatch* RibbonControl, VARIANT* cancel);
	STDMETHOD(TextFromFileInsertAction)(IDispatch* RibbonControl, VARIANT* cancel);
	STDMETHOD(FileSaveToDocumentManagementServerAction)(IDispatch* RibbonControl, VARIANT* cancel);
	STDMETHOD(FileNewBlogPostAction)(IDispatch* RibbonControl, VARIANT* cancel);
	STDMETHOD(GetExtDataFromAccess)(IDispatch* RibbonControl, VARIANT* cancel);	
	STDMETHOD(GetExtDataFromWeb)(IDispatch* RibbonControl, VARIANT* cancel);	
	STDMETHOD(GetExtDataFromText)(IDispatch* RibbonControl, VARIANT* cancel);	
	STDMETHOD(CopyAsPictureAction)(IDispatch* RibbonControl, VARIANT* cancel);		
	STDMETHOD(SheetMoveOrCopyAction)(IDispatch* RibbonControl, VARIANT* cancel);

	STDMETHOD(GetExtDataFromExistConn)(IDispatch* RibbonControl, VARIANT* cancel);			
	STDMETHOD(FileCreateDocumentWorkspaceAction)(IDispatch* RibbonControl, VARIANT_BOOL *pvarfPressed, VARIANT* cancel);
	STDMETHOD(AdvancedFilePropertiesAction)(IDispatch* RibbonControl, VARIANT* cancel);
	STDMETHOD(ScreenClippingAction)(IDispatch* RibbonControl, VARIANT* cancel);
	STDMETHOD(CutAction)(IDispatch* RibbonControl, VARIANT* cancel);
	STDMETHOD(PasteAction)(IDispatch* RibbonControl, VARIANT* cancel);
	STDMETHOD(CopyAction)(IDispatch* RibbonControl, VARIANT* cancel);
	STDMETHOD(FilePrintQuickAction)(IDispatch* RibbonControl, VARIANT* cancel);

	STDMETHOD(FileSaveAsWordDocxAction)(IDispatch* RibbonControl, VARIANT* cancel);
	STDMETHOD(FileSaveAsWordDotxAction)(IDispatch* RibbonControl, VARIANT* cancel);
	STDMETHOD(FileSaveAsWord97_2003Action)(IDispatch* RibbonControl, VARIANT* cancel);
	STDMETHOD(FileSaveAsPdfOrXpsAction)(IDispatch* RibbonControl, VARIANT* cancel);
	STDMETHOD(FileSaveAsOtherFormatsAction)(IDispatch* RibbonControl, VARIANT* cancel);
	STDMETHOD(FilePropertiesAction)(IDispatch* RibbonControl, VARIANT_BOOL *pvarfPressed, VARIANT* cancel);
	STDMETHOD(FilePrintAction)(IDispatch* RibbonControl, VARIANT* cancel);
	STDMETHOD(FileSaveAsWordOpenDocumentTextAction)(IDispatch* RibbonControl, VARIANT* cancel);

	STDMETHOD(FileSaveAsExcelXlsxAction)(IDispatch* RibbonControl, VARIANT* cancel);
	STDMETHOD(FileSaveAsExcelXlsxMacroAction)(IDispatch* RibbonControl, VARIANT* cancel);
	STDMETHOD(FileSaveAsExcel97_2003Action)(IDispatch* RibbonControl, VARIANT* cancel);
	STDMETHOD(FilePublishExcelServicesAction)(IDispatch* RibbonControl, VARIANT* cancel);
	STDMETHOD(FileSaveAsExcelXlsbAction)(IDispatch* RibbonControl, VARIANT* cancel);
	STDMETHOD(FileSaveAsExcelOpenDocumentSpreadsheetAction)(IDispatch* RibbonControl, VARIANT* cancel);
	
	STDMETHOD(FileSaveAsPowerPointPpsxAction)(IDispatch* RibbonControl, VARIANT* cancel);
	STDMETHOD(FileSaveAsPowerPointPptxAction)(IDispatch* RibbonControl, VARIANT* cancel);
	STDMETHOD(FileSaveAsPowerPointOpenDocumentPresentationAction)(IDispatch* RibbonControl, VARIANT* cancel);
	STDMETHOD(FileSaveAsPowerPoint97_2003Action)(IDispatch* RibbonControl, VARIANT* cancel);
	STDMETHOD(FilePackageForCDAction)(IDispatch* RibbonControl, VARIANT* cancel);
	STDMETHOD(FilePublishSlidesAction)(IDispatch* RibbonControl, VARIANT* cancel);
	STDMETHOD(CreateHandoutsInWordAction)(IDispatch* RibbonControl, VARIANT* cancel);
	STDMETHOD(PictureInsertFromFilePowerPointAction)(IDispatch* RibbonControl, VARIANT* cancel);
	STDMETHOD(PictureInsertFromFileAction)(IDispatch* RibbonControl, VARIANT* cancel);
	STDMETHOD(ExportToVideoAction)(IDispatch* RibbonControl, VARIANT* cancel);
	STDMETHOD(VideoInsert2PPT2010)(IDispatch* RibbonControl, VARIANT* cancel);
	STDMETHOD(AudioInsert2PPT2010)(IDispatch* RibbonControl, VARIANT* cancel);
	STDMETHOD(DuplicateSelectedSlidesAction)(IDispatch* RibbonControl, VARIANT* cancel);
	STDMETHOD(BroadcastSlideShowAction)(IDispatch* RibbonControl, VARIANT* cancel);

	STDMETHOD(WindowNewAction)(IDispatch* RibbonControl, VARIANT* cancel);
	STDMETHOD(FilePrintPreviewAction)(IDispatch* RibbonControl, VARIANT* cancel);
	STDMETHOD(PrintPreviewCloseAction)(IDispatch* RibbonControl, VARIANT* cancel);

	STDMETHOD(ZoomCurrent100Action)(IDispatch* RibbonControl, VARIANT* cancel);
	STDMETHOD(ZoomOnePageAction)(IDispatch* RibbonControl, VARIANT* cancel);
	STDMETHOD(ZoomTwoPagesAction)(IDispatch* RibbonControl, VARIANT* cancel);
	STDMETHOD(ZoomPageWidthAction)(IDispatch* RibbonControl, VARIANT* cancel);

private:
	COfficeListener* m_pOfficeListener;
	HMODULE	m_hCBHandle;
};

OBJECT_ENTRY_AUTO(__uuidof(NxtOfficeSink), CNxtOfficeSink)
