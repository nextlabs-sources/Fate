#pragma  once

typedef HRESULT (__stdcall* AppInvokeFunc )
( 
 PVOID pThis,
 /* [in] */ DISPID dispIdMember,
 /* [in] */ REFIID riid,
 /* [in] */ LCID lcid,
 /* [in] */ WORD wFlags,
 /* [out][in] */ DISPPARAMS *pDispParams,
 /* [out] */ VARIANT *pVarResult,
 /* [out] */ EXCEPINFO *pExcepInfo,
 /* [out] */ UINT *puArgErr
 ) ;

class CHookedCollAppShare: public CHookBase
{
    INSTANCE_DECLARE( CHookedCollAppShare );

public:

    void Hook( void* pAppShare );

public:    

    // hook ICollaborateAppShare
	    static HRESULT __stdcall my_AppShare_Scraper(ICollaborateAppShare* pAppShare,ICollaborateScraper** varScraper);
    static HRESULT __stdcall my_AppShare_ScraperFile(ICollaborateAppShare* pAppShare,ICollaborateScraperFile** varScraperFile);
    static HRESULT __stdcall my_AppShare_Viewer(ICollaborateAppShare* pAppShare,ICollaborateViewer** varViewer);
    static HRESULT __stdcall my_AppShare_GetSnapshot(ICollaborateAppShare* pAppShare,tagRECT* prcRegion, LMC_AppShare::wireHBITMAP* phbmpCaptured);
    //////////////////////////////////////////////////////////////////////////
    static HRESULT __stdcall New_AppShare_Scraper(ICollaborateAppShare* pAppShare,ICollaborateScraper** varScraper);
    static HRESULT __stdcall New_AppShare_ScraperFile(ICollaborateAppShare* pAppShare,ICollaborateScraperFile** varScraperFile);
    static HRESULT __stdcall New_AppShare_Viewer(ICollaborateAppShare* pAppShare,ICollaborateViewer** varViewer);
    static HRESULT __stdcall New_AppShare_GetSnapshot(ICollaborateAppShare* pAppShare,tagRECT* prcRegion, LMC_AppShare::wireHBITMAP* phbmpCaptured);

    typedef HRESULT (__stdcall* Old_AppShare_Scraper)(ICollaborateAppShare* pAppShare,ICollaborateScraper** varScraper);
    typedef HRESULT (__stdcall* Old_AppShare_ScraperFile)(ICollaborateAppShare* pAppShare,ICollaborateScraperFile** varScraperFile);
    typedef HRESULT (__stdcall* Old_AppShare_Viewer)(ICollaborateAppShare* pAppShare,ICollaborateViewer** varViewer);
    typedef HRESULT (__stdcall* Old_AppShare_GetSnapshot)(ICollaborateAppShare* pAppShare,tagRECT* prcRegion, LMC_AppShare::wireHBITMAP* phbmpCaptured);

    typedef HRESULT (__stdcall* AppShareOnNotifyFunc) (
        void* pThis,
    /*[in]*/ struct IBaseEvent * pEvent );

    typedef HRESULT (__stdcall* AppShareGet_AnnotateFunc ) ( void* pThis,
        /*[out,retval]*/ VARIANT_BOOL  varAnnotate );
    typedef HRESULT (__stdcall* AppSharePut_AnnotateFunc ) ( void* pThis,
        /*[in]*/ VARIANT_BOOL varAnnotate );

    static HRESULT __stdcall NewAppShareOnNotify (
        ICollaborateAppShare* pThis,
    /*[in]*/ struct IBaseEvent * pEvent );

	 static HRESULT __stdcall myAppShareOnNotify (
        ICollaborateAppShare* pThis,
    /*[in]*/ struct IBaseEvent * pEvent );
    static HRESULT __stdcall NewAppShareInvoke( ICollaborateAppShare* pThis,
        /* [in] */ DISPID dispIdMember,
        /* [in] */ REFIID riid,
        /* [in] */ LCID lcid,
        /* [in] */ WORD wFlags,
        /* [out][in] */ DISPPARAMS *pDispParams,
        /* [out] */ VARIANT *pVarResult,
        /* [out] */ EXCEPINFO *pExcepInfo,
        /* [out] */ UINT *puArgErr ) ;
};

//////////////////////////////////////////////////////////////////////////

// Hook ICollaborateScraperFile
//////////////////////////////////////////////////////////////////////////
//class CHookedCollScraperFile: public CHookBase
//{
//    INSTANCE_DECLARE( CHookedCollScraperFile );
//
//public:
//
//    void Hook( void* pScraperFile );
//
//public:    
//
//    typedef HRESULT (__stdcall* OnNotifyFunc) (
//        void* pThis,
//    /*[in]*/ struct IBaseEvent * pEvent );
//
//    typedef HRESULT (__stdcall* Old_ScraperFile_get_Share)(ICollaborateScraperFile*pScraperFile,LMC_AppShare::ShareStateType* varShare);
//    static HRESULT __stdcall New_ScraperFile_get_Share(ICollaborateScraperFile*pScraperFile,LMC_AppShare::ShareStateType* varShare);
//
//    typedef HRESULT (__stdcall* Old_ScraperFile_put_Share)(ICollaborateScraperFile*pScraperFile,LMC_AppShare::ShareStateType varShare);
//    static HRESULT __stdcall New_ScraperFile_put_Share(ICollaborateScraperFile*pScraperFile,LMC_AppShare::ShareStateType varShare);
//
//
//    typedef HRESULT  (__stdcall* Old_ScraperFile_Init)(
//        ICollaborateScraperFile* pScraperFile,
//        IUnknown* pStream, 
//        BSTR strFile, 
//        double dblSpeed, 
//        unsigned long nTimeToPlay);
//    static HRESULT  __stdcall New_ScraperFile_Init(
//        ICollaborateScraperFile* pScraperFile,
//        IUnknown* pStream, 
//        BSTR strFile, 
//        double dblSpeed, 
//        unsigned long nTimeToPlay);
//
//    typedef HRESULT (__stdcall* Old_ScraperFile_Uninit)(ICollaborateScraperFile* pScraperFile,IErrorInfo* pErrorInfo);
//    static HRESULT __stdcall New_ScraperFile_Uninit(ICollaborateScraperFile* pScraperFile,IErrorInfo* pErrorInfo);
//
//    typedef HRESULT (__stdcall* Old_ScraperFile_SendQosPacket)(ICollaborateScraperFile* pScraperFile,unsigned long PacketId);
//    static HRESULT __stdcall New_ScraperFile_SendQosPacket(ICollaborateScraperFile* pScraperFile,unsigned long PacketId);
//
//    static HRESULT __stdcall NewScraperFileInvoke( ICollaborateScraperFile* pThis,
//        /* [in] */ DISPID dispIdMember,
//        /* [in] */ REFIID riid,
//        /* [in] */ LCID lcid,
//        /* [in] */ WORD wFlags,
//        /* [out][in] */ DISPPARAMS *pDispParams,
//        /* [out] */ VARIANT *pVarResult,
//        /* [out] */ EXCEPINFO *pExcepInfo,
//        /* [out] */ UINT *puArgErr ) ;
//
//    static HRESULT __stdcall NewScraperFileOnNotify (
//        ICollaborateScraperFile* pScraperFile,
//    /*[in]*/ struct IBaseEvent * pEvent );
//
//    static HRESULT __stdcall NewScraperFilePut_Annotate ( void* pThis,
//        /*[in]*/ VARIANT_BOOL varAnnotate );
//
//    static HRESULT __stdcall NewScraperFileGet_Annotate ( void* pThis,
//        /*[in]*/ VARIANT_BOOL varAnnotate );
//};

//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//class CHookedCollScraper: public CHookBase
//{
//    INSTANCE_DECLARE( CHookedCollScraper );
//
//public:
//
//    void Hook( void* pScraper );
//
//public:    
//
//    typedef HRESULT (__stdcall* Old_Scraper_get_Share)(ICollaborateScraper*pScraper,LMC_AppShare::ShareStateType* varShare);
//    static HRESULT __stdcall New_Scraper_get_Share(ICollaborateScraper*pScraper,LMC_AppShare::ShareStateType* varShare);
//
//    typedef HRESULT (__stdcall* Old_Scraper_put_Share)(ICollaborateScraper*pScraper,LMC_AppShare::ShareStateType varShare);
//    static HRESULT __stdcall New_Scraper_put_Share(ICollaborateScraper*pScraper,LMC_AppShare::ShareStateType varShare);
//
//    typedef HRESULT (__stdcall* Old_CollScraper_Init)(ICollaborateScraper*pScraper,
//        IUnknown* pStream);
//    static HRESULT __stdcall New_CollScraper_Init(ICollaborateScraper*pScraper,
//        IUnknown* pStream);
//
//    typedef HRESULT (__stdcall* Old_Put_Annotate )( ICollaborateScraper*pScraper,
//        /*[in]*/ VARIANT_BOOL varAnnotate ) ;
//    static HRESULT __stdcall NewScraperPut_Annotate ( ICollaborateScraper*pScraper,
//        /*[in]*/ VARIANT_BOOL varAnnotate );
//
//    typedef HRESULT (__stdcall* Old_Get_Annotate) ( ICollaborateScraper*pScraper,
//        /*[out,retval]*/ VARIANT_BOOL * varAnnotate );
//    static HRESULT __stdcall NewScraperGet_Annotate ( ICollaborateScraper*pScraper,
//        /*[in]*/ VARIANT_BOOL* varAnnotate );
//
//    static HRESULT __stdcall NewScraperInvoke( ICollaborateScraper* pThis,
//        /* [in] */ DISPID dispIdMember,
//        /* [in] */ REFIID riid,
//        /* [in] */ LCID lcid,
//        /* [in] */ WORD wFlags,
//        /* [out][in] */ DISPPARAMS *pDispParams,
//        /* [out] */ VARIANT *pVarResult,
//        /* [out] */ EXCEPINFO *pExcepInfo,
//        /* [out] */ UINT *puArgErr ) ;
//};

//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//class CHookedCollViewer: public CHookBase
//{
//    INSTANCE_DECLARE( CHookedCollViewer );
//
//public:
//
//    void Hook( void* pViewer );
//
//public:    
//
//    typedef HRESULT (__stdcall* Old_CollViewer_Init)(ICollaborateViewer*pViewer,
//        IUnknown* pStream);
//    static HRESULT __stdcall New_CollViewer_Init(ICollaborateViewer*pViewer,
//        IUnknown* pStream);
//
//    typedef HRESULT (__stdcall *CollViewer_Get_ViewerPanelFunc ) (
//        ICollaborateViewer*pViewer,
//    /*[out,retval]*/ struct ICollaborateViewerPanel * * varViewerPanel );
//
//    static HRESULT __stdcall New_CollViewer_Get_ViewerPanel (
//        ICollaborateViewer*pViewer,
//    /*[out,retval]*/ struct ICollaborateViewerPanel * * varViewerPanel );
//
//    static HRESULT __stdcall NewViewerInvoke( ICollaborateViewer* pThis,
//        /* [in] */ DISPID dispIdMember,
//        /* [in] */ REFIID riid,
//        /* [in] */ LCID lcid,
//        /* [in] */ WORD wFlags,
//        /* [out][in] */ DISPPARAMS *pDispParams,
//        /* [out] */ VARIANT *pVarResult,
//        /* [out] */ EXCEPINFO *pExcepInfo,
//        /* [out] */ UINT *puArgErr ) ;
//};
//////////////////////////////////////////////////////////////////////////



//class CHookedViewerPanel : CHookBase
//{
//    INSTANCE_DECLARE( CHookedViewerPanel );
//
//public:
//
//    void Hook( void* pViewerPanel );
//public:
//
//    typedef HRESULT (__stdcall* Func_Init ) ( ICollaborateViewerPanel* pViewerPanel,
//        /*[in]*/ wireHWND hWndParent );
//    typedef HRESULT (__stdcall* Func_get_State ) ( ICollaborateViewerPanel* pViewerPanel,
//        /*[out,retval]*/ enum LMC_AppShare::ShareStateType * varState );
//    typedef HRESULT (__stdcall* Func_put_State ) ( ICollaborateViewerPanel* pViewerPanel,
//        /*[in]*/ enum LMC_AppShare::ShareStateType varState );
//    typedef HRESULT (__stdcall* Func_get_Handle ) ( ICollaborateViewerPanel* pViewerPanel,
//        /*[out,retval]*/ wireHWND * varHandle );
//    typedef HRESULT (__stdcall* Func_get_AutoPan ) ( ICollaborateViewerPanel* pViewerPanel,
//        /*[out,retval]*/ VARIANT_BOOL * varAutoPan );
//    typedef HRESULT (__stdcall* Func_put_AutoPan ) ( ICollaborateViewerPanel* pViewerPanel,
//        /*[in]*/ VARIANT_BOOL varAutoPan );
//    typedef HRESULT (__stdcall* Func_get_Control ) ( ICollaborateViewerPanel* pViewerPanel,
//        /*[out,retval]*/ VARIANT_BOOL * varControl );
//    typedef HRESULT (__stdcall* Func_put_Control ) ( ICollaborateViewerPanel* pViewerPanel,
//        /*[in]*/ VARIANT_BOOL varControl );
//    typedef HRESULT (__stdcall* Func_get_Scale ) ( ICollaborateViewerPanel* pViewerPanel,
//        /*[out,retval]*/ VARIANT_BOOL * varScale );
//    typedef HRESULT (__stdcall* Func_put_Scale ) ( ICollaborateViewerPanel* pViewerPanel,
//        /*[in]*/ VARIANT_BOOL varScale );
//    typedef HRESULT (__stdcall* Func_get_AspectRatio ) ( ICollaborateViewerPanel* pViewerPanel,
//        /*[out,retval]*/ double * varAspectRatio ) ;
//    typedef HRESULT (__stdcall* Func_get_ViewerSnapshotBuffer ) ( ICollaborateViewerPanel* pViewerPanel,
//    /*[out,retval]*/ struct ICollaborateViewerBuffer * * varViewerSnapshotBuffer );
//
//    static HRESULT __stdcall Hooked_Init ( ICollaborateViewerPanel* pViewerPanel,
//        /*[in]*/ wireHWND hWndParent );
//    static HRESULT __stdcall Hooked_get_State ( ICollaborateViewerPanel* pViewerPanel,
//        /*[out,retval]*/ enum LMC_AppShare::ShareStateType * varState );
//    static HRESULT __stdcall Hooked_put_State ( ICollaborateViewerPanel* pViewerPanel,
//        /*[in]*/ enum LMC_AppShare::ShareStateType varState );
//    static HRESULT __stdcall Hooked_get_Handle ( ICollaborateViewerPanel* pViewerPanel,
//        /*[out,retval]*/ wireHWND * varHandle );
//    static HRESULT __stdcall Hooked_get_AutoPan ( ICollaborateViewerPanel* pViewerPanel,
//        /*[out,retval]*/ VARIANT_BOOL * varAutoPan );
//    static HRESULT __stdcall Hooked_put_AutoPan ( ICollaborateViewerPanel* pViewerPanel,
//        /*[in]*/ VARIANT_BOOL varAutoPan );
//    static HRESULT __stdcall Hooked_get_Control ( ICollaborateViewerPanel* pViewerPanel,
//        /*[out,retval]*/ VARIANT_BOOL * varControl );
//    static HRESULT __stdcall Hooked_put_Control ( ICollaborateViewerPanel* pViewerPanel,
//        /*[in]*/ VARIANT_BOOL varControl );
//    static HRESULT __stdcall Hooked_get_Scale ( ICollaborateViewerPanel* pViewerPanel,
//        /*[out,retval]*/ VARIANT_BOOL * varScale );
//    static HRESULT __stdcall Hooked_put_Scale ( ICollaborateViewerPanel* pViewerPanel,
//        /*[in]*/ VARIANT_BOOL varScale );
//    static HRESULT __stdcall Hooked_get_AspectRatio ( ICollaborateViewerPanel* pViewerPanel,
//        /*[out,retval]*/ double * varAspectRatio ) ;
//    static HRESULT __stdcall Hooked_get_ViewerSnapshotBuffer ( ICollaborateViewerPanel* pViewerPanel,
//    /*[out,retval]*/ struct ICollaborateViewerBuffer * * varViewerSnapshotBuffer );
//};
