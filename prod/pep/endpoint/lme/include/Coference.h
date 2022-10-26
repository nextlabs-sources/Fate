#pragma once

/*
*
*  IDispatch
*/
//////////////////////////////////////////////////////////////////////////
// IUnknown
typedef HRESULT (__stdcall *FUNC_IUNKNOWN_QUERYINTERFACE)(
	IUnknown* This,
	const IID & riid,
	void **ppvObj
	);



typedef ULONG (__stdcall* FUNC_IUNKNOWN_ADDREF)(
	IUnknown* This
	);

typedef ULONG (__stdcall* FUNC_IUNKNOWN_RELEASE)(
	IUnknown* This
	);

typedef HRESULT (__stdcall* Old_GetTypeInfoCount)(unsigned int* pctinfo);

typedef HRESULT (__stdcall* Old_GetTypeInfo)(
							 unsigned int itinfo, 
							 unsigned long lcid, 
							  void** pptinfo);

typedef HRESULT (__stdcall* Old_GetIDsOfNames)(
							   GUID* riid, 
							   char** rgszNames, 
							   unsigned int cNames, 
							   unsigned long lcid, 
							    long* rgdispid);

typedef HRESULT (__stdcall* Old_Invoke)(
						IDispatch* This,
						long dispidMember, 
						GUID* riid, 
						unsigned long lcid, 
						unsigned short wFlags, 
						DISPPARAMS* pdispparams, 
						 VARIANT* pvarResult, 
						 EXCEPINFO* pexcepinfo, 
						 unsigned int* puArgErr);



//////////////////////////////////////////////////////////////////////////


/*
*
*	ConferencingCenter
*/
//////////////////////////////////////////////////////////////////////////
class CHookedConfCenter: public CHookBase
{
    INSTANCE_DECLARE( CHookedConfCenter );

public:

    void Hook( void* pConfCenter );

public:

    static HRESULT __stdcall New_QueryInterface(
        IUnknown* This,
        const IID & riid,
        void **ppvObj
        );

    static HRESULT __stdcall New_Invoke(
        IDispatch* This,
        long dispidMember, 
        GUID* riid, 
        unsigned long lcid, 
        unsigned short wFlags, 
        DISPPARAMS* pdispparams, 
        VARIANT* pvarResult, 
        EXCEPINFO* pexcepinfo, 
        unsigned int* puArgErr);
	static HRESULT __stdcall my_QueryInterface(
        IUnknown* This,
        const IID & riid,
        void **ppvObj
        );

    static HRESULT __stdcall my_Invoke(
        IDispatch* This,
        long dispidMember, 
        GUID* riid, 
        unsigned long lcid, 
        unsigned short wFlags, 
        DISPPARAMS* pdispparams, 
        VARIANT* pvarResult, 
        EXCEPINFO* pexcepinfo, 
        unsigned int* puArgErr);

    typedef HRESULT (__stdcall* Old_SetClientName)( IConferencingCenter* pThis,
        BSTR bstrApplicationName );

    typedef HRESULT (__stdcall* Old_ShowConfigUI) ( IConferencingCenter* pThis);

    typedef HRESULT (__stdcall* Old_GetCombinedCapabilities) (IConferencingCenter* pThis,
        enum ConfCapabilities * pCapabilities );

    typedef HRESULT (__stdcall* Old_GetDefaultProvider) (
        IConferencingCenter* pThis,
        enum ConfCapabilities capability,
    struct IConferencingProvider * * ppProvider ) ;

    typedef HRESULT (__stdcall* Old_GetProviderByID)(
        IConferencingCenter* pThis,
        BSTR bstrProviderID,
    struct IConferencingProvider * * ppProvider );

    typedef HRESULT (__stdcall* Old_GetProviders) 
        (
        IConferencingCenter* pThis,
    struct IConferencingProviders * * pProviders 
        );

    typedef HRESULT (__stdcall* Old_GetConference )(
        IConferencingCenter* pThis,
        BSTR bstrProviderID,
        BSTR bstrConferenceID,
    struct IConference * * ppConference );

    typedef HRESULT (__stdcall* Old_CreateConferenceFromXML) (
        IConferencingCenter* pThis,
        BSTR bstrConferenceDataXML,
    struct IConference * * ppConference );

    typedef HRESULT (__stdcall* Old_SendInvitation) (
        IConferencingCenter* pThis,
    struct IConference * pConference,
        enum ConfInvitationType type,
        enum TextFormat format ) ;

    typedef HRESULT (__stdcall*  Old_Schedule) (
        IConferencingCenter* pThis,
    struct IConference * pConference,
        enum TextFormat format );

    typedef HRESULT (__stdcall* Old_Initialize) ( IConferencingCenter* pThis);
    typedef HRESULT (__stdcall* Old_ShutDown) ( IConferencingCenter* pThis);
    typedef HRESULT (__stdcall* Old_SetPreferredUILanguage) (IConferencingCenter* pThis,
        unsigned short langid );


    // start at here
    static HRESULT __stdcall New_SetClientName (
        IConferencingCenter* pThis,
        /*[in]*/ BSTR bstrApplicationName );
    static HRESULT __stdcall New_ShowConfigUI ( IConferencingCenter* pThis) ;
    static HRESULT __stdcall New_GetCombinedCapabilities (
        IConferencingCenter* pThis,
        /*[out,retval]*/ enum ConfCapabilities * pCapabilities ) ;
    static HRESULT __stdcall New_GetDefaultProvider (
        IConferencingCenter* pThis,
        /*[in]*/ enum ConfCapabilities capability,
    /*[out,retval]*/ struct IConferencingProvider * * ppProvider ) ;
    static HRESULT __stdcall New_GetProviderByID (
        IConferencingCenter* pThis,
        /*[in]*/ BSTR bstrProviderID,
    /*[out,retval]*/ struct IConferencingProvider * * ppProvider ) ;
    static HRESULT __stdcall New_GetProviders (
        IConferencingCenter* pThis,
    /*[out,retval]*/ struct IConferencingProviders * * pProviders ) ;
    static HRESULT __stdcall New_GetConference (
        IConferencingCenter* pThis,
        /*[in]*/ BSTR bstrProviderID,
        /*[in]*/ BSTR bstrConferenceID,
    /*[out,retval]*/ struct IConference * * ppConference ) ;
    static HRESULT __stdcall New_CreateConferenceFromXML (
        IConferencingCenter* pThis,
        /*[in]*/ BSTR bstrConferenceDataXML,
    /*[out,retval]*/ struct IConference * * ppConference ) ;
    static HRESULT __stdcall New_SendInvitation (
        IConferencingCenter* pThis,
    /*[in]*/ struct IConference * pConference,
        /*[in]*/ enum ConfInvitationType type,
        /*[in]*/ enum TextFormat format );
    static HRESULT __stdcall New_Schedule (
        IConferencingCenter* pThis,
    /*[in]*/ struct IConference * pConference,
        /*[in]*/ enum TextFormat format );
    static HRESULT __stdcall New_Initialize ( IConferencingCenter* pThis);
    static HRESULT __stdcall New_ShutDown ( IConferencingCenter* pThis);
    static HRESULT __stdcall New_SetPreferredUILanguage (
        IConferencingCenter* pThis,
        unsigned short langid ) ;

	 static HRESULT __stdcall my_SetClientName (
        IConferencingCenter* pThis,
        /*[in]*/ BSTR bstrApplicationName );
    static HRESULT __stdcall my_ShowConfigUI ( IConferencingCenter* pThis) ;
    static HRESULT __stdcall my_GetCombinedCapabilities (
        IConferencingCenter* pThis,
        /*[out,retval]*/ enum ConfCapabilities * pCapabilities ) ;
    static HRESULT __stdcall my_GetDefaultProvider (
        IConferencingCenter* pThis,
        /*[in]*/ enum ConfCapabilities capability,
    /*[out,retval]*/ struct IConferencingProvider * * ppProvider ) ;
    static HRESULT __stdcall my_GetProviderByID (
        IConferencingCenter* pThis,
        /*[in]*/ BSTR bstrProviderID,
    /*[out,retval]*/ struct IConferencingProvider * * ppProvider ) ;
    static HRESULT __stdcall my_GetProviders (
        IConferencingCenter* pThis,
    /*[out,retval]*/ struct IConferencingProviders * * pProviders ) ;
    static HRESULT __stdcall my_GetConference (
        IConferencingCenter* pThis,
        /*[in]*/ BSTR bstrProviderID,
        /*[in]*/ BSTR bstrConferenceID,
    /*[out,retval]*/ struct IConference * * ppConference ) ;
    static HRESULT __stdcall my_CreateConferenceFromXML (
        IConferencingCenter* pThis,
        /*[in]*/ BSTR bstrConferenceDataXML,
    /*[out,retval]*/ struct IConference * * ppConference ) ;
    static HRESULT __stdcall my_SendInvitation (
        IConferencingCenter* pThis,
    /*[in]*/ struct IConference * pConference,
        /*[in]*/ enum ConfInvitationType type,
        /*[in]*/ enum TextFormat format );
    static HRESULT __stdcall my_Schedule (
        IConferencingCenter* pThis,
    /*[in]*/ struct IConference * pConference,
        /*[in]*/ enum TextFormat format );
    static HRESULT __stdcall my_Initialize ( IConferencingCenter* pThis);
    static HRESULT __stdcall my_ShutDown ( IConferencingCenter* pThis);
    static HRESULT __stdcall my_SetPreferredUILanguage (
        IConferencingCenter* pThis,
        unsigned short langid ) ;
};
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
// just hook ConferencingProvider's key function
class CHookedConfProvider: public CHookBase
{
    INSTANCE_DECLARE( CHookedConfProvider );

public:

    void Hook( void* pConfProvider );

public:
    typedef HRESULT (__stdcall* Old_ConfProvider_ShowConfigUI) (IConferencingProvider* pProvider) ;
    static HRESULT __stdcall New_ConfProvider_ShowConfigUI (IConferencingProvider* pProvider );
	static HRESULT __stdcall my_ConfProvider_ShowConfigUI (IConferencingProvider* pProvider );

    typedef HRESULT (__stdcall* Old_ConfProvider_GetConference) (
        IConferencingProvider* pProvider,
        /*[in]*/ BSTR bstrConferenceID,
    /*[out,retval]*/ struct IConference * * ppConference );
    static HRESULT __stdcall New_ConfProvider_GetConference (
        IConferencingProvider* pProvider,
        /*[in]*/ BSTR bstrConferenceID,
    /*[out,retval]*/ struct IConference * * ppConference );
	static HRESULT __stdcall my_ConfProvider_GetConference (
        IConferencingProvider* pProvider,
        /*[in]*/ BSTR bstrConferenceID,
    /*[out,retval]*/ struct IConference * * ppConference );

    typedef HRESULT (__stdcall* Old_CreateConferenceInstance) (
        IConferencingProvider* pProvider,
        /*[in]*/ enum ConfCapabilities capability,
    /*[out,retval]*/ struct IConference * * ppConference );
    static HRESULT __stdcall New_CreateConferenceInstance (
        IConferencingProvider* pProvider,
        /*[in]*/ enum ConfCapabilities capability,
    /*[out,retval]*/ struct IConference * * ppConference );
	 static HRESULT __stdcall my_CreateConferenceInstance (
        IConferencingProvider* pProvider,
        /*[in]*/ enum ConfCapabilities capability,
    /*[out,retval]*/ struct IConference * * ppConference );

    typedef HRESULT (__stdcall* Old_GetAdHocConference) (
        IConferencingProvider* pProvider,
        /*[in]*/ enum ConfCapabilities capability,
    /*[out,retval]*/ struct IConference * * ppConference );
    static HRESULT __stdcall New_GetAdHocConference (
        IConferencingProvider* pProvider,
        /*[in]*/ enum ConfCapabilities capability,
    /*[out,retval]*/ struct IConference * * ppConference );

	  static HRESULT __stdcall my_GetAdHocConference (
        IConferencingProvider* pProvider,
        /*[in]*/ enum ConfCapabilities capability,
    /*[out,retval]*/ struct IConference * * ppConference );

};

//////////////////////////////////////////////////////////////////////////

// hook Conference's key interface
//////////////////////////////////////////////////////////////////////////
//class CHookedConf: public CHookBase
//{
//    INSTANCE_DECLARE( CHookedConf );
//
//public:
//
//    void Hook( void* pConfProvider );
//public:
//    typedef HRESULT (__stdcall* Old_Conference_ShowConfigUI) (IConference* pConference);
//    static HRESULT __stdcall New_Conference_ShowConfigUI (IConference* pConference);
//
//    typedef HRESULT (__stdcall* Old_SetPropertiesXML) (
//        IConference* pConference,
//        /*[in]*/ BSTR bstrPropertiesXML,
//        /*[in]*/ VARIANT_BOOL fPartial );
//
//    static HRESULT __stdcall New_SetPropertiesXML (
//        IConference* pConference,
//        /*[in]*/ BSTR bstrPropertiesXML,
//        /*[in]*/ VARIANT_BOOL fPartial );
//
//
//    typedef HRESULT (__stdcall* Old_GetJoinPropertiesXML) (IConference* pConference,
//        /*[out,retval]*/ BSTR * pbstrJoinPropertiesXML );
//    static HRESULT __stdcall New_GetJoinPropertiesXML (IConference* pConference,
//        /*[out,retval]*/ BSTR * pbstrJoinPropertiesXML );
//
//
//    typedef HRESULT (__stdcall* Old_RefreshOnlineProperties) (IConference* pConference );
//    static HRESULT __stdcall New_RefreshOnlineProperties (IConference* pConference );
//
//    typedef HRESULT (__stdcall* Old_Commit) (IConference* pConference,
//        /*[in]*/ VARIANT_BOOL fBlindUpdate ) ;
//    static HRESULT __stdcall New_Commit (IConference* pConference,
//        /*[in]*/ VARIANT_BOOL fBlindUpdate ) ;
//
//    typedef HRESULT (__stdcall* Old_GetInvitationText) (
//        IConference* pConference,
//        /*[in]*/ enum ConfInvitationType InvitationType,
//        /*[in]*/ enum TextFormat format,
//        /*[out,retval]*/ BSTR * pbstrInvitationText ) ;
//    static HRESULT __stdcall New_GetInvitationText (
//        IConference* pConference,
//        /*[in]*/ enum ConfInvitationType InvitationType,
//        /*[in]*/ enum TextFormat format,
//        /*[out,retval]*/ BSTR * pbstrInvitationText ) ;
//
//
//    typedef HRESULT (__stdcall* Old_Join) ( IConference* pConference) ;
//    static HRESULT __stdcall New_Join ( IConference* pConference) ;
//
//    typedef HRESULT (__stdcall* Old_Cancel) (IConference* pConference );
//    static HRESULT __stdcall New_Cancel (IConference* pConference );
//
//    typedef HRESULT (__stdcall* Old_GetPrependedInvitationText) (
//        IConference* pConference,
//        /*[in]*/ enum ConfInvitationType InvitationType,
//        /*[in]*/ enum TextFormat format,
//        /*[in]*/ BSTR pbstrPrependText,
//        VARIANT_BOOL fAddSeperator,
//        /*[out,retval]*/ BSTR * pbstrInvitationText );
//    static HRESULT __stdcall New_GetPrependedInvitationText (
//        IConference* pConference,
//        /*[in]*/ enum ConfInvitationType InvitationType,
//        /*[in]*/ enum TextFormat format,
//        /*[in]*/ BSTR pbstrPrependText,
//        VARIANT_BOOL fAddSeperator,
//        /*[out,retval]*/ BSTR * pbstrInvitationText );
//
//
//    typedef HRESULT (__stdcall* Old_SetWordInvitationText) (
//        IConference* pConference,
//        /*[in]*/ enum ConfInvitationType InvitationType,
//        /*[in]*/ IDispatch * pdispWord );
//    static HRESULT __stdcall New_SetWordInvitationText (
//        IConference* pConference,
//        /*[in]*/ enum ConfInvitationType InvitationType,
//        /*[in]*/ IDispatch * pdispWord );
//};
//////////////////////////////////////////////////////////////////////////