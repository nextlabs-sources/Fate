#pragma once
#include "Sink.h"

// ICollaborateSimple
//////////////////////////////////////////////////////////////////////////
class CHookedCollSimple: public CHookBase
{
    INSTANCE_DECLARE( CHookedCollSimple );

public:

    void Hook( void* pSimple );

public:

    typedef HRESULT (__stdcall* CollSimple_Old_Init) (ICollaborateSimple* pSimple,
        /*[in]*/ IUnknown * pNPWRC_Stream );
    static HRESULT __stdcall CollSimple_New_Init (ICollaborateSimple* pSimple,
        /*[in]*/ IUnknown * pNPWRC_Stream );

    typedef HRESULT (__stdcall* Old_CollSimple_get_Share) (ICollaborateSimple* pSimple,LMC_Coll::ShareStateType * varShare ) ;
    static HRESULT __stdcall New_CollSimple_get_Share (ICollaborateSimple* pSimple,LMC_Coll::ShareStateType * varShare ) ;	 
	static HRESULT __stdcall my_CollSimple_get_Share (ICollaborateSimple* pSimple,LMC_Coll::ShareStateType * varShare ) ;

    typedef HRESULT (__stdcall* Old_CollSimple_put_Share) (ICollaborateSimple* pSimple, LMC_Coll::ShareStateType varShare ) ;

    static HRESULT __stdcall New_CollSimple_put_Share (ICollaborateSimple* pSimple,LMC_Coll::ShareStateType varShare ) ;
	 static HRESULT __stdcall my_CollSimple_put_Share (ICollaborateSimple* pSimple,LMC_Coll::ShareStateType varShare ) ;


    typedef HRESULT (__stdcall* Old_AnnotateClear)(ICollaborateSimple* pSimple);
    static HRESULT __stdcall New_AnnotateClear(ICollaborateSimple* pSimple);
	static HRESULT __stdcall my_AnnotateClear(ICollaborateSimple* pSimple);

    typedef HRESULT (__stdcall* Old_CollSimple_get_ControlRequestAllowed)(ICollaborateSimple* pSimple,VARIANT_BOOL* varControlRequestAllowed);

    typedef HRESULT (__stdcall* Old_CollSimple_put_ControlRequestAllowed)(ICollaborateSimple* pSimple,VARIANT_BOOL varControlRequestAllowed);

    typedef HRESULT (__stdcall* Old_CollSimple_get_AutoAcceptControlRequest)(ICollaborateSimple* pSimple,VARIANT_BOOL* varAutoAcceptControlRequest);

    typedef HRESULT (__stdcall* Old_CollSimple_put_AutoAcceptControlRequest)(ICollaborateSimple* pSimple,VARIANT_BOOL varAutoAcceptControlRequest);

    static HRESULT __stdcall New_CollSimple_get_ControlRequestAllowed(ICollaborateSimple* pSimple,VARIANT_BOOL* varControlRequestAllowed);

    static HRESULT __stdcall New_CollSimple_put_ControlRequestAllowed(ICollaborateSimple* pSimple,VARIANT_BOOL varControlRequestAllowed);

    static HRESULT __stdcall New_CollSimple_get_AutoAcceptControlRequest(ICollaborateSimple* pSimple,VARIANT_BOOL* varAutoAcceptControlRequest);

    static HRESULT __stdcall New_CollSimple_put_AutoAcceptControlRequest(ICollaborateSimple* pSimple,VARIANT_BOOL varAutoAcceptControlRequest);


    typedef HRESULT (__stdcall* Old_InitViewer)(ICollaborateSimple* pSimple,IUnknown* pNPWRC_Stream);
    static HRESULT __stdcall New_InitViewer(ICollaborateSimple* pSimple,IUnknown* pNPWRC_Stream);


    typedef HRESULT (__stdcall* Old_ShareViewer)(ICollaborateSimple* pSimple,LMC_Coll::ShareStateType rhs);
    static HRESULT __stdcall New_ShareViewer(ICollaborateSimple* pSimple,LMC_Coll::ShareStateType rhs);

    typedef HRESULT (__stdcall* Old_OpenSnapshotTool)(ICollaborateSimple* pSimple);
    static HRESULT __stdcall New_OpenSnapshotTool(ICollaborateSimple* pSimple);

    typedef HRESULT (__stdcall* Old_SendQosPacket)(ICollaborateSimple* pSimple,unsigned long nPacketId);
    static HRESULT __stdcall New_SendQosPacket(ICollaborateSimple* pSimple,unsigned long nPacketId);

    typedef HRESULT (__stdcall* Old_CreateViewerPanel)(ICollaborateSimple* pSimple,
        long hWndSlide, 
        long* phWnd);
    static HRESULT __stdcall New_CreateViewerPanel(ICollaborateSimple* pSimple,
        long hWndSlide, 
        long* phWnd);

    typedef HRESULT (__stdcall* Put_PrimaryWindowFunc) (
        ICollaborateSimple* pSimple,
        /*[in]*/ long varPrimaryWindow );
    static HRESULT __stdcall NewPut_PrimaryWindow(
        ICollaborateSimple* pSimple,
        /*[in]*/ long varPrimaryWindow );

    typedef HRESULT (__stdcall* Put_PrimaryWindowPositionFunc) (
        ICollaborateSimple* pSimple,
        /*[in]*/ long nLeftX,
        /*[in]*/ long nTopY,
        /*[in]*/ long nRightX,
        /*[in]*/ long nBottomY ) ;

    static HRESULT __stdcall NewPut_PrimaryWindowPosition (
        ICollaborateSimple* pSimple,
        /*[in]*/ long nLeftX,
        /*[in]*/ long nTopY,
        /*[in]*/ long nRightX,
        /*[in]*/ long nBottomY ) ;

    typedef HRESULT (__stdcall* Get_PrimaryWindowPositionFunc) (
        ICollaborateSimple* pSimple,
        /*[in]*/ long* pnLeftX,
        /*[in]*/ long* pnTopY,
        /*[in]*/ long* pnRightX,
        /*[in]*/ long* pnBottomY ) ;

    static HRESULT __stdcall NewGet_PrimaryWindowPosition (
        ICollaborateSimple* pSimple,
        /*[in]*/ long* pnLeftX,
        /*[in]*/ long* pnTopY,
        /*[in]*/ long* pnRightX,
        /*[in]*/ long* pnBottomY ) ;

    typedef HRESULT (__stdcall* Put_ProcessIdFunc) (
        ICollaborateSimple* pSimple,
        /*[in]*/ unsigned long varProcessId );

    static HRESULT __stdcall NewPut_ProcessId (
        ICollaborateSimple* pSimple,
        /*[in]*/ unsigned long varProcessId );

    typedef HRESULT (__stdcall* Get_ProcessIdFunc) (
        ICollaborateSimple* pSimple,
        /*[out,retval]*/ unsigned long * varProcessId );

    static HRESULT __stdcall NewGet_ProcessId (
        ICollaborateSimple* pSimple,
        /*[out,retval]*/ unsigned long * varProcessId );

    static HRESULT __stdcall NewPut_Scale (
        ICollaborateSimple* pSimple,
        /*[in]*/ VARIANT_BOOL varScale ) ;
    typedef HRESULT (__stdcall *Put_ScaleFunc) (
        ICollaborateSimple* pSimple,
        /*[in]*/ VARIANT_BOOL varScale ) ;


    typedef HRESULT (_stdcall* GetViewerSnapshotBufferFunc) (
        ICollaborateSimple* pSimple,
    /*[out,retval]*/ struct ICollaborateViewerBuffer * * varViewerSnapshotBuffer );

    static HRESULT _stdcall NewGetViewerSnapshotBuffer (
        ICollaborateSimple* pSimple,
    /*[out,retval]*/ struct ICollaborateViewerBuffer * * varViewerSnapshotBuffer );

    static HRESULT __stdcall NewPut_Annotate ( ICollaborateSimple* pSimple,
        /*[in]*/ VARIANT_BOOL varAnnotate );

    typedef HRESULT (__stdcall* Put_AnnotateFunc )( ICollaborateSimple* pSimple,
        /*[in]*/ VARIANT_BOOL varAnnotate );


    typedef HRESULT (__stdcall* Get_AnnotateFunc )( ICollaborateSimple* pSimple,
        /*[in]*/ VARIANT_BOOL varAnnotate );

    static HRESULT __stdcall NewGet_Annotate ( ICollaborateSimple* pSimple,
        /*[in]*/ VARIANT_BOOL varAnnotate );

    typedef HRESULT (__stdcall* Get_PrimaryWindowTitleFunc) (
        ICollaborateSimple* pSimple,
        /*[out,retval]*/ BSTR * varPrimaryWindowTitle );
    typedef HRESULT (__stdcall* Put_PrimaryWindowTitleFunc) (
        ICollaborateSimple* pSimple,
        /*[in]*/ BSTR varPrimaryWindowTitle ) ;

    static HRESULT __stdcall NewGet_PrimaryWindowTitle(
        ICollaborateSimple* pSimple,
        /*[out,retval]*/ BSTR * varPrimaryWindowTitle );
    static HRESULT __stdcall NewPut_PrimaryWindowTitle (
        ICollaborateSimple* pSimple,
        /*[in]*/ BSTR varPrimaryWindowTitle ) ;

    typedef HRESULT (__stdcall* Func_get_HotKey )( ICollaborateSimple* pSimple,
        /*[out,retval]*/ unsigned int * varHotKey ) ;
    typedef HRESULT (__stdcall* Func_put_HotKey )( ICollaborateSimple* pSimple,
        /*[in]*/ unsigned int varHotKey ) ;

    static HRESULT __stdcall Hooked_get_HotKey ( ICollaborateSimple* pSimple,
        /*[out,retval]*/ unsigned int * varHotKey );
    static HRESULT __stdcall Hooked_put_HotKey ( ICollaborateSimple* pSimple,
        /*[in]*/ unsigned int varHotKey ) ;

    typedef HRESULT (__stdcall* Func_InitPlayback )( ICollaborateSimple* pSimple,
        /*[in]*/ IUnknown * pNPWRC_Stream,
        /*[in]*/ BSTR strFile,
        /*[in]*/ double dblSpeed,
        /*[in]*/ unsigned long nTimeToPlay );

    static HRESULT __stdcall Hooked_InitPlayback ( ICollaborateSimple* pSimple,
        /*[in]*/ IUnknown * pNPWRC_Stream,
        /*[in]*/ BSTR strFile,
        /*[in]*/ double dblSpeed,
        /*[in]*/ unsigned long nTimeToPlay );

    typedef /* [local] */ HRESULT (STDMETHODCALLTYPE* Func_Invoke )( ICollaborateSimple* pSimple,
        /* [in] */ DISPID dispIdMember,
        /* [in] */ REFIID riid,
        /* [in] */ LCID lcid,
        /* [in] */ WORD wFlags,
        /* [out][in] */ DISPPARAMS *pDispParams,
        /* [out] */ VARIANT *pVarResult,
        /* [out] */ EXCEPINFO *pExcepInfo,
        /* [out] */ UINT *puArgErr);

    static /* [local] */ HRESULT STDMETHODCALLTYPE Hooked_Invoke( ICollaborateSimple* pSimple,
        /* [in] */ DISPID dispIdMember,
        /* [in] */ REFIID riid,
        /* [in] */ LCID lcid,
        /* [in] */ WORD wFlags,
        /* [out][in] */ DISPPARAMS *pDispParams,
        /* [out] */ VARIANT *pVarResult,
        /* [out] */ EXCEPINFO *pExcepInfo,
        /* [out] */ UINT *puArgErr);

    typedef HRESULT (__stdcall* Func_OnNotify )( ICollaborateSimple* pSimple,
    /*[in]*/ struct IBaseEvent * pEvent ) ;

    static HRESULT __stdcall Hooked_OnNotify ( ICollaborateSimple* pSimple,
    /*[in]*/ struct IBaseEvent * pEvent ) ;








		static HRESULT __stdcall my_CollSimple_get_ControlRequestAllowed(ICollaborateSimple* pSimple,VARIANT_BOOL* varControlRequestAllowed);

    static HRESULT __stdcall my_CollSimple_put_ControlRequestAllowed(ICollaborateSimple* pSimple,VARIANT_BOOL varControlRequestAllowed);

    static HRESULT __stdcall my_CollSimple_get_AutoAcceptControlRequest(ICollaborateSimple* pSimple,VARIANT_BOOL* varAutoAcceptControlRequest);

    static HRESULT __stdcall my_CollSimple_put_AutoAcceptControlRequest(ICollaborateSimple* pSimple,VARIANT_BOOL varAutoAcceptControlRequest);
	  static HRESULT __stdcall my_InitViewer(ICollaborateSimple* pSimple,IUnknown* pNPWRC_Stream);

		    static HRESULT __stdcall my_ShareViewer(ICollaborateSimple* pSimple,LMC_Coll::ShareStateType rhs);
	   static HRESULT __stdcall my_OpenSnapshotTool(ICollaborateSimple* pSimple);
	  static HRESULT __stdcall my_SendQosPacket(ICollaborateSimple* pSimple,unsigned long nPacketId);
	     static HRESULT __stdcall my_CreateViewerPanel(ICollaborateSimple* pSimple,
        long hWndSlide, 
        long* phWnd);
		 static HRESULT __stdcall myPut_PrimaryWindow(
        ICollaborateSimple* pSimple,
        /*[in]*/ long varPrimaryWindow );
		   static HRESULT __stdcall myPut_PrimaryWindowPosition (
        ICollaborateSimple* pSimple,
        /*[in]*/ long nLeftX,
        /*[in]*/ long nTopY,
        /*[in]*/ long nRightX,
        /*[in]*/ long nBottomY ) ;
		   	 static HRESULT __stdcall myGet_PrimaryWindowPosition (
        ICollaborateSimple* pSimple,
        /*[in]*/ long* pnLeftX,
        /*[in]*/ long* pnTopY,
        /*[in]*/ long* pnRightX,
        /*[in]*/ long* pnBottomY ) ;
	 static HRESULT __stdcall myPut_ProcessId (
        ICollaborateSimple* pSimple,
        /*[in]*/ unsigned long varProcessId );
	 static HRESULT __stdcall myGet_ProcessId (
        ICollaborateSimple* pSimple,
        /*[out,retval]*/ unsigned long * varProcessId );
	   static HRESULT __stdcall myPut_Scale (
        ICollaborateSimple* pSimple,
        /*[in]*/ VARIANT_BOOL varScale ) ;

	    static HRESULT _stdcall myGetViewerSnapshotBuffer (
        ICollaborateSimple* pSimple,
    /*[out,retval]*/ struct ICollaborateViewerBuffer * * varViewerSnapshotBuffer );

    static HRESULT __stdcall myPut_Annotate ( ICollaborateSimple* pSimple,
        /*[in]*/ VARIANT_BOOL varAnnotate );
	  static HRESULT __stdcall myGet_Annotate ( ICollaborateSimple* pSimple,
        /*[in]*/ VARIANT_BOOL varAnnotate );
	   static HRESULT __stdcall myGet_PrimaryWindowTitle(
        ICollaborateSimple* pSimple,
        /*[out,retval]*/ BSTR * varPrimaryWindowTitle );
	   static HRESULT __stdcall myPut_PrimaryWindowTitle (
        ICollaborateSimple* pSimple,
        /*[in]*/ BSTR varPrimaryWindowTitle ) ;	  
	       static HRESULT __stdcall my_get_HotKey ( ICollaborateSimple* pSimple,
        /*[out,retval]*/ unsigned int * varHotKey );
    static HRESULT __stdcall my_put_HotKey ( ICollaborateSimple* pSimple,
        /*[in]*/ unsigned int varHotKey ) ;
	    static HRESULT __stdcall my_InitPlayback ( ICollaborateSimple* pSimple,
        /*[in]*/ IUnknown * pNPWRC_Stream,
        /*[in]*/ BSTR strFile,
        /*[in]*/ double dblSpeed,
        /*[in]*/ unsigned long nTimeToPlay );
		    static /* [local] */ HRESULT STDMETHODCALLTYPE my_Invoke( ICollaborateSimple* pSimple,
        /* [in] */ DISPID dispIdMember,
        /* [in] */ REFIID riid,
        /* [in] */ LCID lcid,
        /* [in] */ WORD wFlags,
        /* [out][in] */ DISPPARAMS *pDispParams,
        /* [out] */ VARIANT *pVarResult,
        /* [out] */ EXCEPINFO *pExcepInfo,
        /* [out] */ UINT *puArgErr);

    static HRESULT __stdcall my_OnNotify ( ICollaborateSimple* pSimple,
    /*[in]*/ struct IBaseEvent * pEvent ) ;






};
//////////////////////////////////////////////////////////////////////////