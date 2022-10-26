#pragma once

//class CHookedFlash:public CHookBase
//{
//    INSTANCE_DECLARE( CHookedFlash );
//public:
//
//    void Hook( void* pFlash );
//
//public:
//
//    typedef HRESULT (__stdcall* Func_get_ReadyState )( IShockwaveFlash* pFlash,
//        /*[out,retval]*/ long * pVal );
//    typedef HRESULT (__stdcall* Func_get_TotalFrames )( IShockwaveFlash* pFlash,
//        /*[out,retval]*/ long * pVal );
//    typedef HRESULT (__stdcall* Func_get_Playing )( IShockwaveFlash* pFlash,
//        /*[out,retval]*/ VARIANT_BOOL * pVal );
//    typedef HRESULT (__stdcall* Func_put_Playing )( IShockwaveFlash* pFlash,
//        /*[in]*/ VARIANT_BOOL pVal );
//    typedef HRESULT (__stdcall* Func_get_Quality )( IShockwaveFlash* pFlash,
//        /*[out,retval]*/ int * pVal );
//    typedef HRESULT (__stdcall* Func_put_Quality )( IShockwaveFlash* pFlash,
//        /*[in]*/ int pVal );
//    typedef HRESULT (__stdcall* Func_get_ScaleMode )( IShockwaveFlash* pFlash,
//        /*[out,retval]*/ int * pVal );
//    typedef HRESULT (__stdcall* Func_put_ScaleMode )( IShockwaveFlash* pFlash,
//        /*[in]*/ int pVal );
//    typedef HRESULT (__stdcall* Func_get_AlignMode )( IShockwaveFlash* pFlash,
//        /*[out,retval]*/ int * pVal );
//    typedef HRESULT (__stdcall* Func_put_AlignMode )( IShockwaveFlash* pFlash,
//        /*[in]*/ int pVal );
//    typedef HRESULT (__stdcall* Func_get_BackgroundColor )( IShockwaveFlash* pFlash,
//        /*[out,retval]*/ long * pVal );
//    typedef HRESULT (__stdcall* Func_put_BackgroundColor )( IShockwaveFlash* pFlash,
//        /*[in]*/ long pVal );
//    typedef HRESULT (__stdcall* Func_get_Loop )( IShockwaveFlash* pFlash,
//        /*[out,retval]*/ VARIANT_BOOL * pVal );
//    typedef HRESULT (__stdcall* Func_put_Loop )( IShockwaveFlash* pFlash,
//        /*[in]*/ VARIANT_BOOL pVal );
//    typedef HRESULT (__stdcall* Func_get_Movie )( IShockwaveFlash* pFlash,
//        /*[out,retval]*/ BSTR * pVal );
//    typedef HRESULT (__stdcall* Func_put_Movie )( IShockwaveFlash* pFlash,
//        /*[in]*/ BSTR pVal );
//    typedef HRESULT (__stdcall* Func_get_FrameNum )( IShockwaveFlash* pFlash,
//        /*[out,retval]*/ long * pVal );
//    typedef HRESULT (__stdcall* Func_put_FrameNum )( IShockwaveFlash* pFlash,
//        /*[in]*/ long pVal );
//    typedef HRESULT (__stdcall* Func_SetZoomRect )( IShockwaveFlash* pFlash,
//        /*[in]*/ long left,
//        /*[in]*/ long top,
//        /*[in]*/ long right,
//        /*[in]*/ long bottom );
//    typedef HRESULT (__stdcall* Func_Zoom )( IShockwaveFlash* pFlash,
//        /*[in]*/ int factor );
//    typedef HRESULT (__stdcall* Func_Pan )( IShockwaveFlash* pFlash,
//        /*[in]*/ long x,
//        /*[in]*/ long y,
//        /*[in]*/ int mode );
//    typedef HRESULT (__stdcall* Func_Play )( IShockwaveFlash* pFlash );
//    typedef HRESULT (__stdcall* Func_Stop )( IShockwaveFlash* pFlash );
//    typedef HRESULT (__stdcall* Func_Back )( IShockwaveFlash* pFlash );
//    typedef HRESULT (__stdcall* Func_Forward )( IShockwaveFlash* pFlash );
//    typedef HRESULT (__stdcall* Func_Rewind )( IShockwaveFlash* pFlash );
//    typedef HRESULT (__stdcall* Func_StopPlay )( IShockwaveFlash* pFlash );
//    typedef HRESULT (__stdcall* Func_GotoFrame )( IShockwaveFlash* pFlash,
//        /*[in]*/ long FrameNum );
//    typedef HRESULT (__stdcall* Func_CurrentFrame )( IShockwaveFlash* pFlash,
//        /*[out,retval]*/ long * FrameNum );
//    typedef HRESULT (__stdcall* Func_IsPlaying )( IShockwaveFlash* pFlash,
//        /*[out,retval]*/ VARIANT_BOOL * Playing );
//    typedef HRESULT (__stdcall* Func_PercentLoaded )( IShockwaveFlash* pFlash,
//        /*[out,retval]*/ long * percent );
//    typedef HRESULT (__stdcall* Func_FrameLoaded )( IShockwaveFlash* pFlash,
//        /*[in]*/ long FrameNum,
//        /*[out,retval]*/ VARIANT_BOOL * loaded );
//    typedef HRESULT (__stdcall* Func_FlashVersion )( IShockwaveFlash* pFlash,
//        /*[out,retval]*/ long * version );
//    typedef HRESULT (__stdcall* Func_get_WMode )( IShockwaveFlash* pFlash,
//        /*[out,retval]*/ BSTR * pVal );
//    typedef HRESULT (__stdcall* Func_put_WMode )( IShockwaveFlash* pFlash,
//        /*[in]*/ BSTR pVal );
//    typedef HRESULT (__stdcall* Func_get_SAlign )( IShockwaveFlash* pFlash,
//        /*[out,retval]*/ BSTR * pVal );
//    typedef HRESULT (__stdcall* Func_put_SAlign )( IShockwaveFlash* pFlash,
//        /*[in]*/ BSTR pVal );
//    typedef HRESULT (__stdcall* Func_get_Menu )( IShockwaveFlash* pFlash,
//        /*[out,retval]*/ VARIANT_BOOL * pVal );
//    typedef HRESULT (__stdcall* Func_put_Menu )( IShockwaveFlash* pFlash,
//        /*[in]*/ VARIANT_BOOL pVal );
//    typedef HRESULT (__stdcall* Func_get_Base )( IShockwaveFlash* pFlash,
//        /*[out,retval]*/ BSTR * pVal );
//    typedef HRESULT (__stdcall* Func_put_Base )( IShockwaveFlash* pFlash,
//        /*[in]*/ BSTR pVal );
//    typedef HRESULT (__stdcall* Func_get_Scale )( IShockwaveFlash* pFlash,
//        /*[out,retval]*/ BSTR * pVal );
//    typedef HRESULT (__stdcall* Func_put_Scale )( IShockwaveFlash* pFlash,
//        /*[in]*/ BSTR pVal );
//    typedef HRESULT (__stdcall* Func_get_DeviceFont )( IShockwaveFlash* pFlash,
//        /*[out,retval]*/ VARIANT_BOOL * pVal );
//    typedef HRESULT (__stdcall* Func_put_DeviceFont )( IShockwaveFlash* pFlash,
//        /*[in]*/ VARIANT_BOOL pVal );
//    typedef HRESULT (__stdcall* Func_get_EmbedMovie )( IShockwaveFlash* pFlash,
//        /*[out,retval]*/ VARIANT_BOOL * pVal );
//    typedef HRESULT (__stdcall* Func_put_EmbedMovie )( IShockwaveFlash* pFlash,
//        /*[in]*/ VARIANT_BOOL pVal );
//    typedef HRESULT (__stdcall* Func_get_BGColor )( IShockwaveFlash* pFlash,
//        /*[out,retval]*/ BSTR * pVal );
//    typedef HRESULT (__stdcall* Func_put_BGColor )( IShockwaveFlash* pFlash,
//        /*[in]*/ BSTR pVal );
//    typedef HRESULT (__stdcall* Func_get_Quality2 )( IShockwaveFlash* pFlash,
//        /*[out,retval]*/ BSTR * pVal );
//    typedef HRESULT (__stdcall* Func_put_Quality2 )( IShockwaveFlash* pFlash,
//        /*[in]*/ BSTR pVal );
//    typedef HRESULT (__stdcall* Func_LoadMovie )( IShockwaveFlash* pFlash,
//        /*[in]*/ int layer,
//        /*[in]*/ BSTR url );
//    typedef HRESULT (__stdcall* Func_TGotoFrame )( IShockwaveFlash* pFlash,
//        /*[in]*/ BSTR target,
//        /*[in]*/ long FrameNum );
//    typedef HRESULT (__stdcall* Func_TGotoLabel )( IShockwaveFlash* pFlash,
//        /*[in]*/ BSTR target,
//        /*[in]*/ BSTR label );
//    typedef HRESULT (__stdcall* Func_TCurrentFrame )( IShockwaveFlash* pFlash,
//        /*[in]*/ BSTR target,
//        /*[out,retval]*/ long * FrameNum );
//    typedef HRESULT (__stdcall* Func_TCurrentLabel )( IShockwaveFlash* pFlash,
//        /*[in]*/ BSTR target,
//        /*[out,retval]*/ BSTR * pVal );
//    typedef HRESULT (__stdcall* Func_TPlay )( IShockwaveFlash* pFlash,
//        /*[in]*/ BSTR target );
//    typedef HRESULT (__stdcall* Func_TStopPlay )( IShockwaveFlash* pFlash,
//        /*[in]*/ BSTR target );
//    typedef HRESULT (__stdcall* Func_SetVariable )( IShockwaveFlash* pFlash,
//        /*[in]*/ BSTR name,
//        /*[in]*/ BSTR value );
//    typedef HRESULT (__stdcall* Func_GetVariable )( IShockwaveFlash* pFlash,
//        /*[in]*/ BSTR name,
//        /*[out,retval]*/ BSTR * pVal );
//    typedef HRESULT (__stdcall* Func_TSetProperty )( IShockwaveFlash* pFlash,
//        /*[in]*/ BSTR target,
//        /*[in]*/ int property,
//        /*[in]*/ BSTR value );
//    typedef HRESULT (__stdcall* Func_TGetProperty )( IShockwaveFlash* pFlash,
//        /*[in]*/ BSTR target,
//        /*[in]*/ int property,
//        /*[out,retval]*/ BSTR * pVal );
//    typedef HRESULT (__stdcall* Func_TCallFrame )( IShockwaveFlash* pFlash,
//        /*[in]*/ BSTR target,
//        /*[in]*/ int FrameNum );
//    typedef HRESULT (__stdcall* Func_TCallLabel )( IShockwaveFlash* pFlash,
//        /*[in]*/ BSTR target,
//        /*[in]*/ BSTR label );
//    typedef HRESULT (__stdcall* Func_TSetPropertyNum )( IShockwaveFlash* pFlash,
//        /*[in]*/ BSTR target,
//        /*[in]*/ int property,
//        /*[in]*/ double value );
//    typedef HRESULT (__stdcall* Func_TGetPropertyNum )( IShockwaveFlash* pFlash,
//        /*[in]*/ BSTR target,
//        /*[in]*/ int property,
//        /*[out,retval]*/ double * pVal );
//    typedef HRESULT (__stdcall* Func_TGetPropertyAsNumber )( IShockwaveFlash* pFlash,
//        /*[in]*/ BSTR target,
//        /*[in]*/ int property,
//        /*[out,retval]*/ double * pVal );
//    typedef HRESULT (__stdcall* Func_get_SWRemote )( IShockwaveFlash* pFlash,
//        /*[out,retval]*/ BSTR * pVal );
//    typedef HRESULT (__stdcall* Func_put_SWRemote )( IShockwaveFlash* pFlash,
//        /*[in]*/ BSTR pVal );
//    typedef HRESULT (__stdcall* Func_get_FlashVars )( IShockwaveFlash* pFlash,
//        /*[out,retval]*/ BSTR * pVal );
//    typedef HRESULT (__stdcall* Func_put_FlashVars )( IShockwaveFlash* pFlash,
//        /*[in]*/ BSTR pVal );
//    typedef HRESULT (__stdcall* Func_get_AllowScriptAccess )( IShockwaveFlash* pFlash,
//        /*[out,retval]*/ BSTR * pVal );
//    typedef HRESULT (__stdcall* Func_put_AllowScriptAccess )( IShockwaveFlash* pFlash,
//        /*[in]*/ BSTR pVal );
//
//
//
//    static HRESULT  __stdcall Hooked_get_ReadyState ( IShockwaveFlash* pFlash,
//        /*[out,retval]*/ long * pVal );
//    static HRESULT  __stdcall Hooked_get_TotalFrames ( IShockwaveFlash* pFlash,
//        /*[out,retval]*/ long * pVal );
//    static HRESULT  __stdcall Hooked_get_Playing ( IShockwaveFlash* pFlash,
//        /*[out,retval]*/ VARIANT_BOOL * pVal );
//    static HRESULT  __stdcall Hooked_put_Playing ( IShockwaveFlash* pFlash,
//        /*[in]*/ VARIANT_BOOL pVal );
//    static HRESULT  __stdcall Hooked_get_Quality ( IShockwaveFlash* pFlash,
//        /*[out,retval]*/ int * pVal );
//    static HRESULT  __stdcall Hooked_put_Quality ( IShockwaveFlash* pFlash,
//        /*[in]*/ int pVal );
//    static HRESULT  __stdcall Hooked_get_ScaleMode ( IShockwaveFlash* pFlash,
//        /*[out,retval]*/ int * pVal );
//    static HRESULT  __stdcall Hooked_put_ScaleMode ( IShockwaveFlash* pFlash,
//        /*[in]*/ int pVal );
//    static HRESULT  __stdcall Hooked_get_AlignMode ( IShockwaveFlash* pFlash,
//        /*[out,retval]*/ int * pVal );
//    static HRESULT  __stdcall Hooked_put_AlignMode ( IShockwaveFlash* pFlash,
//        /*[in]*/ int pVal );
//    static HRESULT  __stdcall Hooked_get_BackgroundColor ( IShockwaveFlash* pFlash,
//        /*[out,retval]*/ long * pVal );
//    static HRESULT  __stdcall Hooked_put_BackgroundColor ( IShockwaveFlash* pFlash,
//        /*[in]*/ long pVal );
//    static HRESULT  __stdcall Hooked_get_Loop ( IShockwaveFlash* pFlash,
//        /*[out,retval]*/ VARIANT_BOOL * pVal );
//    static HRESULT  __stdcall Hooked_put_Loop ( IShockwaveFlash* pFlash,
//        /*[in]*/ VARIANT_BOOL pVal );
//    static HRESULT  __stdcall Hooked_get_Movie ( IShockwaveFlash* pFlash,
//        /*[out,retval]*/ BSTR * pVal );
//    static HRESULT  __stdcall Hooked_put_Movie ( IShockwaveFlash* pFlash,
//        /*[in]*/ BSTR pVal );
//    static HRESULT  __stdcall Hooked_get_FrameNum ( IShockwaveFlash* pFlash,
//        /*[out,retval]*/ long * pVal );
//    static HRESULT  __stdcall Hooked_put_FrameNum ( IShockwaveFlash* pFlash,
//        /*[in]*/ long pVal );
//    static HRESULT  __stdcall Hooked_SetZoomRect ( IShockwaveFlash* pFlash,
//        /*[in]*/ long left,
//        /*[in]*/ long top,
//        /*[in]*/ long right,
//        /*[in]*/ long bottom );
//    static HRESULT  __stdcall Hooked_Zoom ( IShockwaveFlash* pFlash,
//        /*[in]*/ int factor );
//    static HRESULT  __stdcall Hooked_Pan ( IShockwaveFlash* pFlash,
//        /*[in]*/ long x,
//        /*[in]*/ long y,
//        /*[in]*/ int mode );
//    static HRESULT  __stdcall Hooked_Play ( IShockwaveFlash* pFlash );
//    static HRESULT  __stdcall Hooked_Stop ( IShockwaveFlash* pFlash );
//    static HRESULT  __stdcall Hooked_Back ( IShockwaveFlash* pFlash );
//    static HRESULT  __stdcall Hooked_Forward ( IShockwaveFlash* pFlash );
//    static HRESULT  __stdcall Hooked_Rewind ( IShockwaveFlash* pFlash );
//    static HRESULT  __stdcall Hooked_StopPlay ( IShockwaveFlash* pFlash );
//    static HRESULT  __stdcall Hooked_GotoFrame ( IShockwaveFlash* pFlash,
//        /*[in]*/ long FrameNum );
//    static HRESULT  __stdcall Hooked_CurrentFrame ( IShockwaveFlash* pFlash,
//        /*[out,retval]*/ long * FrameNum );
//    static HRESULT  __stdcall Hooked_IsPlaying ( IShockwaveFlash* pFlash,
//        /*[out,retval]*/ VARIANT_BOOL * Playing );
//    static HRESULT  __stdcall Hooked_PercentLoaded ( IShockwaveFlash* pFlash,
//        /*[out,retval]*/ long * percent );
//    static HRESULT  __stdcall Hooked_FrameLoaded ( IShockwaveFlash* pFlash,
//        /*[in]*/ long FrameNum,
//        /*[out,retval]*/ VARIANT_BOOL * loaded );
//    static HRESULT  __stdcall Hooked_FlashVersion ( IShockwaveFlash* pFlash,
//        /*[out,retval]*/ long * version );
//    static HRESULT  __stdcall Hooked_get_WMode ( IShockwaveFlash* pFlash,
//        /*[out,retval]*/ BSTR * pVal );
//    static HRESULT  __stdcall Hooked_put_WMode ( IShockwaveFlash* pFlash,
//        /*[in]*/ BSTR pVal );
//    static HRESULT  __stdcall Hooked_get_SAlign ( IShockwaveFlash* pFlash,
//        /*[out,retval]*/ BSTR * pVal );
//    static HRESULT  __stdcall Hooked_put_SAlign ( IShockwaveFlash* pFlash,
//        /*[in]*/ BSTR pVal );
//    static HRESULT  __stdcall Hooked_get_Menu ( IShockwaveFlash* pFlash,
//        /*[out,retval]*/ VARIANT_BOOL * pVal );
//    static HRESULT  __stdcall Hooked_put_Menu ( IShockwaveFlash* pFlash,
//        /*[in]*/ VARIANT_BOOL pVal );
//    static HRESULT  __stdcall Hooked_get_Base ( IShockwaveFlash* pFlash,
//        /*[out,retval]*/ BSTR * pVal );
//    static HRESULT  __stdcall Hooked_put_Base ( IShockwaveFlash* pFlash,
//        /*[in]*/ BSTR pVal );
//    static HRESULT  __stdcall Hooked_get_Scale ( IShockwaveFlash* pFlash,
//        /*[out,retval]*/ BSTR * pVal );
//    static HRESULT  __stdcall Hooked_put_Scale ( IShockwaveFlash* pFlash,
//        /*[in]*/ BSTR pVal );
//    static HRESULT  __stdcall Hooked_get_DeviceFont ( IShockwaveFlash* pFlash,
//        /*[out,retval]*/ VARIANT_BOOL * pVal );
//    static HRESULT  __stdcall Hooked_put_DeviceFont ( IShockwaveFlash* pFlash,
//        /*[in]*/ VARIANT_BOOL pVal );
//    static HRESULT  __stdcall Hooked_get_EmbedMovie ( IShockwaveFlash* pFlash,
//        /*[out,retval]*/ VARIANT_BOOL * pVal );
//    static HRESULT  __stdcall Hooked_put_EmbedMovie ( IShockwaveFlash* pFlash,
//        /*[in]*/ VARIANT_BOOL pVal );
//    static HRESULT  __stdcall Hooked_get_BGColor ( IShockwaveFlash* pFlash,
//        /*[out,retval]*/ BSTR * pVal );
//    static HRESULT  __stdcall Hooked_put_BGColor ( IShockwaveFlash* pFlash,
//        /*[in]*/ BSTR pVal );
//    static HRESULT  __stdcall Hooked_get_Quality2 ( IShockwaveFlash* pFlash,
//        /*[out,retval]*/ BSTR * pVal );
//    static HRESULT  __stdcall Hooked_put_Quality2 ( IShockwaveFlash* pFlash,
//        /*[in]*/ BSTR pVal );
//    static HRESULT  __stdcall Hooked_LoadMovie ( IShockwaveFlash* pFlash,
//        /*[in]*/ int layer,
//        /*[in]*/ BSTR url );
//    static HRESULT  __stdcall Hooked_TGotoFrame ( IShockwaveFlash* pFlash,
//        /*[in]*/ BSTR target,
//        /*[in]*/ long FrameNum );
//    static HRESULT  __stdcall Hooked_TGotoLabel ( IShockwaveFlash* pFlash,
//        /*[in]*/ BSTR target,
//        /*[in]*/ BSTR label );
//    static HRESULT  __stdcall Hooked_TCurrentFrame ( IShockwaveFlash* pFlash,
//        /*[in]*/ BSTR target,
//        /*[out,retval]*/ long * FrameNum );
//    static HRESULT  __stdcall Hooked_TCurrentLabel ( IShockwaveFlash* pFlash,
//        /*[in]*/ BSTR target,
//        /*[out,retval]*/ BSTR * pVal );
//    static HRESULT  __stdcall Hooked_TPlay ( IShockwaveFlash* pFlash,
//        /*[in]*/ BSTR target );
//    static HRESULT  __stdcall Hooked_TStopPlay ( IShockwaveFlash* pFlash,
//        /*[in]*/ BSTR target );
//    static HRESULT  __stdcall Hooked_SetVariable ( IShockwaveFlash* pFlash,
//        /*[in]*/ BSTR name,
//        /*[in]*/ BSTR value );
//    static HRESULT  __stdcall Hooked_GetVariable ( IShockwaveFlash* pFlash,
//        /*[in]*/ BSTR name,
//        /*[out,retval]*/ BSTR * pVal );
//    static HRESULT  __stdcall Hooked_TSetProperty ( IShockwaveFlash* pFlash,
//        /*[in]*/ BSTR target,
//        /*[in]*/ int property,
//        /*[in]*/ BSTR value );
//    static HRESULT  __stdcall Hooked_TGetProperty ( IShockwaveFlash* pFlash,
//        /*[in]*/ BSTR target,
//        /*[in]*/ int property,
//        /*[out,retval]*/ BSTR * pVal );
//    static HRESULT  __stdcall Hooked_TCallFrame ( IShockwaveFlash* pFlash,
//        /*[in]*/ BSTR target,
//        /*[in]*/ int FrameNum );
//    static HRESULT  __stdcall Hooked_TCallLabel ( IShockwaveFlash* pFlash,
//        /*[in]*/ BSTR target,
//        /*[in]*/ BSTR label );
//    static HRESULT  __stdcall Hooked_TSetPropertyNum ( IShockwaveFlash* pFlash,
//        /*[in]*/ BSTR target,
//        /*[in]*/ int property,
//        /*[in]*/ double value );
//    static HRESULT  __stdcall Hooked_TGetPropertyNum ( IShockwaveFlash* pFlash,
//        /*[in]*/ BSTR target,
//        /*[in]*/ int property,
//        /*[out,retval]*/ double * pVal );
//    static HRESULT  __stdcall Hooked_TGetPropertyAsNumber ( IShockwaveFlash* pFlash,
//        /*[in]*/ BSTR target,
//        /*[in]*/ int property,
//        /*[out,retval]*/ double * pVal );
//    static HRESULT  __stdcall Hooked_get_SWRemote ( IShockwaveFlash* pFlash,
//        /*[out,retval]*/ BSTR * pVal );
//    static HRESULT  __stdcall Hooked_put_SWRemote ( IShockwaveFlash* pFlash,
//        /*[in]*/ BSTR pVal );
//    static HRESULT  __stdcall Hooked_get_FlashVars ( IShockwaveFlash* pFlash,
//        /*[out,retval]*/ BSTR * pVal );
//    static HRESULT  __stdcall Hooked_put_FlashVars ( IShockwaveFlash* pFlash,
//        /*[in]*/ BSTR pVal );
//    static HRESULT  __stdcall Hooked_get_AllowScriptAccess ( IShockwaveFlash* pFlash,
//        /*[out,retval]*/ BSTR * pVal );
//    static HRESULT  __stdcall Hooked_put_AllowScriptAccess ( IShockwaveFlash* pFlash,
//        /*[in]*/ BSTR pVal );
//
//
//};