#include "stdafx.h"
#include "HookedFlash.h"
#include "MsgHook.h"

//INSTANCE_DEFINE( CHookedFlash );
//
//void CHookedFlash::Hook( void* pFlash )
//{
//    SubstituteOrgFuncWithNew( pFlash, 7, (void*)Hooked_get_ReadyState );
//    SubstituteOrgFuncWithNew( pFlash, 8, (void*)Hooked_get_TotalFrames );
//    SubstituteOrgFuncWithNew( pFlash, 9, (void*)Hooked_get_Playing );
//    SubstituteOrgFuncWithNew( pFlash, 10, (void*)Hooked_put_Playing );
//    SubstituteOrgFuncWithNew( pFlash, 11, (void*)Hooked_get_Quality );
//    SubstituteOrgFuncWithNew( pFlash, 12, (void*)Hooked_put_Quality );
//    SubstituteOrgFuncWithNew( pFlash, 13, (void*)Hooked_get_ScaleMode );
//    SubstituteOrgFuncWithNew( pFlash, 14, (void*)Hooked_put_ScaleMode );
//    SubstituteOrgFuncWithNew( pFlash, 15, (void*)Hooked_get_AlignMode );
//    SubstituteOrgFuncWithNew( pFlash, 16, (void*)Hooked_put_AlignMode );
//    SubstituteOrgFuncWithNew( pFlash, 17, (void*)Hooked_get_BackgroundColor );
//    SubstituteOrgFuncWithNew( pFlash, 18, (void*)Hooked_put_BackgroundColor );
//    SubstituteOrgFuncWithNew( pFlash, 19, (void*)Hooked_get_Loop );
//    SubstituteOrgFuncWithNew( pFlash, 20, (void*)Hooked_put_Loop );
//    SubstituteOrgFuncWithNew( pFlash, 21, (void*)Hooked_get_Movie );
//    SubstituteOrgFuncWithNew( pFlash, 22, (void*)Hooked_put_Movie );
//    SubstituteOrgFuncWithNew( pFlash, 23, (void*)Hooked_get_FrameNum );
//    SubstituteOrgFuncWithNew( pFlash, 24, (void*)Hooked_put_FrameNum  );
//    SubstituteOrgFuncWithNew( pFlash, 25, (void*)Hooked_SetZoomRect );
//    SubstituteOrgFuncWithNew( pFlash, 26, (void*)Hooked_Zoom  );
//    SubstituteOrgFuncWithNew( pFlash, 27, (void*)Hooked_Pan  );
//    SubstituteOrgFuncWithNew( pFlash, 28, (void*)Hooked_Play );
//    SubstituteOrgFuncWithNew( pFlash, 29, (void*)Hooked_Stop );
//    SubstituteOrgFuncWithNew( pFlash, 30, (void*)Hooked_Back );
//    SubstituteOrgFuncWithNew( pFlash, 31, (void*)Hooked_Forward ) ;
//    SubstituteOrgFuncWithNew( pFlash, 32, (void*)Hooked_Rewind );
//    SubstituteOrgFuncWithNew( pFlash, 33, (void*)Hooked_StopPlay );
//    SubstituteOrgFuncWithNew( pFlash, 34, (void*)Hooked_GotoFrame );
//    SubstituteOrgFuncWithNew( pFlash, 35, (void*)Hooked_CurrentFrame );
//    SubstituteOrgFuncWithNew( pFlash, 36, (void*)Hooked_IsPlaying );
//    SubstituteOrgFuncWithNew( pFlash, 37, (void*)Hooked_PercentLoaded );
//    SubstituteOrgFuncWithNew( pFlash, 38, (void*)Hooked_FrameLoaded  );
//    SubstituteOrgFuncWithNew( pFlash, 39, (void*)Hooked_FlashVersion );
//    SubstituteOrgFuncWithNew( pFlash, 40, (void*)Hooked_get_WMode );
//    SubstituteOrgFuncWithNew( pFlash, 41, (void*)Hooked_put_WMode );
//    SubstituteOrgFuncWithNew( pFlash, 42, (void*)Hooked_get_SAlign );
//    SubstituteOrgFuncWithNew( pFlash, 43, (void*)Hooked_put_SAlign );
//    SubstituteOrgFuncWithNew( pFlash, 44, (void*)Hooked_get_Menu );
//    SubstituteOrgFuncWithNew( pFlash, 45, (void*)Hooked_put_Menu );
//    SubstituteOrgFuncWithNew( pFlash, 46, (void*)Hooked_get_Base );
//    SubstituteOrgFuncWithNew( pFlash, 47, (void*)Hooked_put_Base );
//    SubstituteOrgFuncWithNew( pFlash, 48, (void*)Hooked_get_Scale );
//    SubstituteOrgFuncWithNew( pFlash, 49, (void*)Hooked_put_Scale );
//    SubstituteOrgFuncWithNew( pFlash, 50, (void*)Hooked_get_DeviceFont );
//    SubstituteOrgFuncWithNew( pFlash, 51, (void*)Hooked_put_DeviceFont );
//    SubstituteOrgFuncWithNew( pFlash, 52, (void*)Hooked_get_EmbedMovie );
//    SubstituteOrgFuncWithNew( pFlash, 53, (void*)Hooked_put_EmbedMovie );
//    SubstituteOrgFuncWithNew( pFlash, 54, (void*)Hooked_get_BGColor );
//    SubstituteOrgFuncWithNew( pFlash, 55, (void*)Hooked_put_BGColor );
//    SubstituteOrgFuncWithNew( pFlash, 56, (void*)Hooked_get_Quality2 );
//    SubstituteOrgFuncWithNew( pFlash, 57, (void*)Hooked_put_Quality2 );
//    SubstituteOrgFuncWithNew( pFlash, 58, (void*)Hooked_LoadMovie );
//    SubstituteOrgFuncWithNew( pFlash, 59, (void*)Hooked_TGotoFrame );
//    SubstituteOrgFuncWithNew( pFlash, 60, (void*)Hooked_TGotoLabel );
//    SubstituteOrgFuncWithNew( pFlash, 61, (void*)Hooked_TCurrentFrame );
//    SubstituteOrgFuncWithNew( pFlash, 62, (void*)Hooked_TCurrentLabel );
//    SubstituteOrgFuncWithNew( pFlash, 63, (void*)Hooked_TPlay );
//    SubstituteOrgFuncWithNew( pFlash, 64, (void*)Hooked_TStopPlay );
//    SubstituteOrgFuncWithNew( pFlash, 65, (void*)Hooked_SetVariable );
//    SubstituteOrgFuncWithNew( pFlash, 66, (void*)Hooked_GetVariable );
//    SubstituteOrgFuncWithNew( pFlash, 67, (void*)Hooked_TSetProperty );
//    SubstituteOrgFuncWithNew( pFlash, 68, (void*)Hooked_TGetProperty );
//    SubstituteOrgFuncWithNew( pFlash, 69, (void*)Hooked_TCallFrame );
//    SubstituteOrgFuncWithNew( pFlash, 70, (void*)Hooked_TCallLabel );
//    SubstituteOrgFuncWithNew( pFlash, 71, (void*)Hooked_TSetPropertyNum );
//    SubstituteOrgFuncWithNew( pFlash, 72, (void*)Hooked_TGetPropertyNum );
//    SubstituteOrgFuncWithNew( pFlash, 73, (void*)Hooked_TGetPropertyAsNumber  );
//    SubstituteOrgFuncWithNew( pFlash, 74, (void*)Hooked_get_SWRemote );
//    SubstituteOrgFuncWithNew( pFlash, 75, (void*)Hooked_put_SWRemote );
//    SubstituteOrgFuncWithNew( pFlash, 76, (void*)Hooked_get_FlashVars );
//    SubstituteOrgFuncWithNew( pFlash, 77, (void*)Hooked_put_FlashVars );
//    SubstituteOrgFuncWithNew( pFlash, 78, (void*)Hooked_get_AllowScriptAccess );
//    SubstituteOrgFuncWithNew( pFlash, 79, (void*)Hooked_put_AllowScriptAccess );
//    DoHook( pFlash );
//}
//
//HRESULT  __stdcall CHookedFlash::Hooked_get_ReadyState ( IShockwaveFlash* pFlash,
//    /*[out,retval]*/ long * pVal )
//{ 
//    Func_get_ReadyState pOrgFunc = (Func_get_ReadyState)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_get_ReadyState ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( pFlash );
//        pOrgFunc = (Func_get_ReadyState)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_get_ReadyState ));
//    }
//
//    HRESULT hr = E_NOTIMPL;
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( pFlash, pVal );
//    }
//    return hr;
//}
//
//HRESULT  __stdcall CHookedFlash::Hooked_get_TotalFrames ( IShockwaveFlash* pFlash,
//    /*[out,retval]*/ long * pVal )
//{ 
//    HRESULT hr = E_NOTIMPL; 
//    Func_get_TotalFrames pOrgFunc = (Func_get_TotalFrames)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_get_TotalFrames ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( pFlash );
//        pOrgFunc = (Func_get_TotalFrames)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_get_TotalFrames ));
//    }
//
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( pFlash, pVal );
//    }
//    return hr; 
//}
//
//HRESULT  __stdcall CHookedFlash::Hooked_get_Playing ( IShockwaveFlash* pFlash,
//    /*[out,retval]*/ VARIANT_BOOL * pVal )
//{ 
//    HRESULT hr = E_NOTIMPL; 
//    Func_get_Playing pOrgFunc = (Func_get_Playing)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_get_Playing ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( pFlash );
//        pOrgFunc = (Func_get_Playing)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_get_Playing ));
//    }
//
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( pFlash, pVal );
//    }
//    return hr; 
//}
//
//HRESULT  __stdcall CHookedFlash::Hooked_put_Playing ( IShockwaveFlash* pFlash,
//    /*[in]*/ VARIANT_BOOL pVal )
//{ 
//    HRESULT hr = E_NOTIMPL; 
//    Func_put_Playing pOrgFunc = (Func_put_Playing)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_put_Playing ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( pFlash );
//        pOrgFunc = (Func_put_Playing)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_put_Playing ));
//    }
//
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( pFlash, pVal );
//    }
//    return hr; 
//}
//
//HRESULT  __stdcall CHookedFlash::Hooked_get_Quality ( IShockwaveFlash* pFlash,
//    /*[out,retval]*/ int * pVal )
//{ 
//    HRESULT hr = E_NOTIMPL; 
//    Func_get_Quality pOrgFunc = (Func_get_Quality)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_get_Quality ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( pFlash );
//        pOrgFunc = (Func_get_Quality)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_get_Quality ));
//    }
//
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( pFlash, pVal );
//    }
//    return hr; 
//}
//
//HRESULT  __stdcall CHookedFlash::Hooked_put_Quality ( IShockwaveFlash* pFlash,
//    /*[in]*/ int pVal )
//{ 
//    HRESULT hr = E_NOTIMPL; 
//    Func_put_Quality pOrgFunc = (Func_put_Quality)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_put_Quality ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( pFlash );
//        pOrgFunc = (Func_put_Quality)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_put_Quality ));
//    }
//
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( pFlash, pVal );
//    }
//    return hr; 
//}
//
//HRESULT  __stdcall CHookedFlash::Hooked_get_ScaleMode ( IShockwaveFlash* pFlash,
//    /*[out,retval]*/ int * pVal )
//{ 
//    HRESULT hr = E_NOTIMPL; 
//    Func_get_ScaleMode pOrgFunc = (Func_get_ScaleMode)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_get_ScaleMode ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( pFlash );
//        pOrgFunc = (Func_get_ScaleMode)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_get_ScaleMode ));
//    }
//
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( pFlash, pVal );
//    }
//    return hr; 
//}
//
//HRESULT  __stdcall CHookedFlash::Hooked_put_ScaleMode ( IShockwaveFlash* pFlash,
//    /*[in]*/ int pVal )
//{ 
//    HRESULT hr = E_NOTIMPL; 
//    Func_put_ScaleMode pOrgFunc = (Func_put_ScaleMode)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_put_ScaleMode ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( pFlash );
//        pOrgFunc = (Func_put_ScaleMode)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_put_ScaleMode ));
//    }
//
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( pFlash, pVal );
//    }
//    return hr; 
//}
//
//HRESULT  __stdcall CHookedFlash::Hooked_get_AlignMode ( IShockwaveFlash* pFlash,
//    /*[out,retval]*/ int * pVal )
//{ 
//    HRESULT hr = E_NOTIMPL; 
//    Func_get_AlignMode pOrgFunc = (Func_get_AlignMode)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_get_AlignMode ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( pFlash );
//        pOrgFunc = (Func_get_AlignMode)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_get_AlignMode ));
//    }
//
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( pFlash, pVal );
//    }
//    return hr;
//}
//
//HRESULT  __stdcall CHookedFlash::Hooked_put_AlignMode ( IShockwaveFlash* pFlash,
//    /*[in]*/ int pVal )
//{ 
//    HRESULT hr = E_NOTIMPL; 
//    Func_put_AlignMode pOrgFunc = (Func_put_AlignMode)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_put_AlignMode ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( pFlash );
//        pOrgFunc = (Func_put_AlignMode)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_put_AlignMode ));
//    }
//
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( pFlash, pVal );
//    }
//    return hr;
//}
//
//HRESULT  __stdcall CHookedFlash::Hooked_get_BackgroundColor ( IShockwaveFlash* pFlash,
//    /*[out,retval]*/ long * pVal )
//{ 
//    HRESULT hr = E_NOTIMPL; 
//    Func_get_BackgroundColor pOrgFunc = (Func_get_BackgroundColor)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_get_BackgroundColor ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( pFlash );
//        pOrgFunc = (Func_get_BackgroundColor)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_get_BackgroundColor ));
//    }
//
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( pFlash, pVal );
//    }
//    return hr;
//}
//
//HRESULT  __stdcall CHookedFlash::Hooked_put_BackgroundColor ( IShockwaveFlash* pFlash,
//    /*[in]*/ long pVal )
//{ 
//    HRESULT hr = E_NOTIMPL; 
//    Func_put_BackgroundColor pOrgFunc = (Func_put_BackgroundColor)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_put_BackgroundColor ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( pFlash );
//        pOrgFunc = (Func_put_BackgroundColor)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_put_BackgroundColor ));
//    }
//
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( pFlash, pVal );
//    }
//    return hr;
//}
//
//HRESULT  __stdcall CHookedFlash::Hooked_get_Loop ( IShockwaveFlash* pFlash,
//    /*[out,retval]*/ VARIANT_BOOL * pVal )
//{ 
//    HRESULT hr = E_NOTIMPL; 
//    Func_get_Loop pOrgFunc = (Func_get_Loop)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_get_Loop ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( pFlash );
//        pOrgFunc = (Func_get_Loop)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_get_Loop ));
//    }
//
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( pFlash, pVal );
//    }
//    return hr;
//}
//
//HRESULT  __stdcall CHookedFlash::Hooked_put_Loop ( IShockwaveFlash* pFlash,
//    /*[in]*/ VARIANT_BOOL pVal )
//{ 
//    HRESULT hr = E_NOTIMPL; 
//    Func_put_Loop pOrgFunc = (Func_put_Loop)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_put_Loop ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( pFlash );
//        pOrgFunc = (Func_put_Loop)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_put_Loop ));
//    }
//
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( pFlash, pVal );
//    }
//    return hr;
//}
//
//HRESULT  __stdcall CHookedFlash::Hooked_get_Movie ( IShockwaveFlash* pFlash,
//    /*[out,retval]*/ BSTR * pVal )
//{ 
//    HRESULT hr = E_NOTIMPL; 
//    Func_get_Movie pOrgFunc = (Func_get_Movie)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_get_Movie ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( pFlash );
//        pOrgFunc = (Func_get_Movie)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_get_Movie ));
//    }
//
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( pFlash, pVal );
//    }
//    return hr;
//}
//
//HRESULT  __stdcall CHookedFlash::Hooked_put_Movie ( IShockwaveFlash* pFlash,
//    /*[in]*/ BSTR pVal )
//{ 
//    HRESULT hr = E_NOTIMPL; 
//    Func_put_Movie pOrgFunc = (Func_put_Movie)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_put_Movie ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( pFlash );
//        pOrgFunc = (Func_put_Movie)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_put_Movie ));
//    }
//
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( pFlash, pVal );
//    }
//    return hr;
//}
//
//HRESULT  __stdcall CHookedFlash::Hooked_get_FrameNum ( IShockwaveFlash* pFlash,
//    /*[out,retval]*/ long * pVal )
//{ 
//    HRESULT hr = E_NOTIMPL; 
//    Func_get_FrameNum pOrgFunc = (Func_get_FrameNum)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_get_FrameNum ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( pFlash );
//        pOrgFunc = (Func_get_FrameNum)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_get_FrameNum ));
//    }
//
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( pFlash, pVal );
//    }
//    return hr;
//}
//
//HRESULT  __stdcall CHookedFlash::Hooked_put_FrameNum ( IShockwaveFlash* pFlash,
//    /*[in]*/ long pVal )
//{ 
//    HRESULT hr = E_NOTIMPL; 
//    Func_put_FrameNum pOrgFunc = (Func_put_FrameNum)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_put_FrameNum ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( pFlash );
//        pOrgFunc = (Func_put_FrameNum)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_put_FrameNum ));
//    }
//
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( pFlash, pVal );
//    }
//    return hr;
//}
//
//HRESULT  __stdcall CHookedFlash::Hooked_SetZoomRect ( IShockwaveFlash* pFlash,
//    /*[in]*/ long left,
//    /*[in]*/ long top,
//    /*[in]*/ long right,
//    /*[in]*/ long bottom )
//{ 
//    HRESULT hr = E_NOTIMPL; 
//    Func_SetZoomRect pOrgFunc = (Func_SetZoomRect)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_SetZoomRect ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( pFlash );
//        pOrgFunc = (Func_SetZoomRect)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_SetZoomRect ));
//    }
//
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( pFlash, left, top, right, bottom );
//    }
//    return hr;
//}
//
//HRESULT  __stdcall CHookedFlash::Hooked_Zoom ( IShockwaveFlash* pFlash,
//    /*[in]*/ int factor )
//{ 
//    HRESULT hr = E_NOTIMPL; 
//    Func_Zoom pOrgFunc = (Func_Zoom)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_Zoom ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( pFlash );
//        pOrgFunc = (Func_Zoom)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_Zoom ));
//    }
//
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( pFlash, factor );
//    }
//    return hr;
//}
//
//HRESULT  __stdcall CHookedFlash::Hooked_Pan ( IShockwaveFlash* pFlash,
//                                   /*[in]*/ long x,
//                                   /*[in]*/ long y,
//                                   /*[in]*/ int mode )
//{ 
//    HRESULT hr = E_NOTIMPL; 
//    Func_Pan pOrgFunc = (Func_Pan)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_Pan ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( pFlash );
//        pOrgFunc = (Func_Pan)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_Pan ));
//    }
//
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( pFlash, x, y, mode );
//    }
//    return hr;
//}
//
//HRESULT  __stdcall CHookedFlash::Hooked_Play ( IShockwaveFlash* pFlash )
//{ 
//    HRESULT hr = E_NOTIMPL; 
//    Func_Play pOrgFunc = (Func_Play)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_Play ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( pFlash );
//        pOrgFunc = (Func_Play)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_Play ));
//    }
//
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( pFlash );
//    }
//    return hr;
//}
//
//HRESULT  __stdcall CHookedFlash::Hooked_Stop ( IShockwaveFlash* pFlash )
//{ 
//    HRESULT hr = E_NOTIMPL; 
//    Func_Stop pOrgFunc = (Func_Stop)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_Stop ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( pFlash );
//        pOrgFunc = (Func_Stop)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_Stop ));
//    }
//
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( pFlash );
//    }
//    return hr;
//}
//
//HRESULT  __stdcall CHookedFlash::Hooked_Back ( IShockwaveFlash* pFlash )
//{ 
//    HRESULT hr = E_NOTIMPL; 
//    Func_Back pOrgFunc = (Func_Back)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_Back ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( pFlash );
//        pOrgFunc = (Func_Back)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_Back ));
//    }
//
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( pFlash );
//    }
//    return hr;
//}
//
//HRESULT  __stdcall CHookedFlash::Hooked_Forward ( IShockwaveFlash* pFlash )
//{ 
//    HRESULT hr = E_NOTIMPL; 
//    Func_Forward pOrgFunc = (Func_Forward)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_Forward ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( pFlash );
//        pOrgFunc = (Func_Forward)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_Forward ));
//    }
//
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( pFlash );
//    }
//    return hr;
//}
//
//HRESULT  __stdcall CHookedFlash::Hooked_Rewind ( IShockwaveFlash* pFlash )
//{ 
//    HRESULT hr = E_NOTIMPL; 
//    Func_Rewind pOrgFunc = (Func_Rewind)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_Rewind ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( pFlash );
//        pOrgFunc = (Func_Rewind)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_Rewind ));
//    }
//
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( pFlash );
//    }
//    return hr;
//}
//
//HRESULT  __stdcall CHookedFlash::Hooked_StopPlay ( IShockwaveFlash* pFlash )
//{ 
//    HRESULT hr = E_NOTIMPL; 
//    Func_StopPlay pOrgFunc = (Func_StopPlay)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_StopPlay ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( pFlash );
//        pOrgFunc = (Func_StopPlay)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_StopPlay ));
//    }
//
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( pFlash );
//    }
//    return hr;
//}
//
//HRESULT  __stdcall CHookedFlash::Hooked_GotoFrame ( IShockwaveFlash* pFlash,
//    /*[in]*/ long FrameNum )
//{ 
//    HRESULT hr = E_NOTIMPL; 
//    Func_GotoFrame pOrgFunc = (Func_GotoFrame)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_GotoFrame ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( pFlash );
//        pOrgFunc = (Func_GotoFrame)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_GotoFrame ));
//    }
//
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( pFlash, FrameNum );
//    }
//    return hr;
//}
//
//HRESULT  __stdcall CHookedFlash::Hooked_CurrentFrame ( IShockwaveFlash* pFlash,
//    /*[out,retval]*/ long * FrameNum )
//{ 
//    HRESULT hr = E_NOTIMPL; 
//    Func_CurrentFrame pOrgFunc = (Func_CurrentFrame)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_CurrentFrame ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( pFlash );
//        pOrgFunc = (Func_CurrentFrame)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_CurrentFrame ));
//    }
//
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( pFlash, FrameNum );
//    }
//    return hr;
//}
//
//HRESULT  __stdcall CHookedFlash::Hooked_IsPlaying ( IShockwaveFlash* pFlash,
//    /*[out,retval]*/ VARIANT_BOOL * Playing )
//{ 
//    HRESULT hr = E_NOTIMPL; 
//    Func_IsPlaying pOrgFunc = (Func_IsPlaying)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_IsPlaying ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( pFlash );
//        pOrgFunc = (Func_IsPlaying)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_IsPlaying ));
//    }
//
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( pFlash, Playing );
//    }
//    return hr;
//}
//
//HRESULT  __stdcall CHookedFlash::Hooked_PercentLoaded ( IShockwaveFlash* pFlash,
//    /*[out,retval]*/ long * percent )
//{ 
//    HRESULT hr = E_NOTIMPL; 
//    Func_PercentLoaded pOrgFunc = (Func_PercentLoaded)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_PercentLoaded ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( pFlash );
//        pOrgFunc = (Func_PercentLoaded)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_PercentLoaded ));
//    }
//
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( pFlash, percent );
//    }
//    return hr;
//}
//
//HRESULT  __stdcall CHookedFlash::Hooked_FrameLoaded ( IShockwaveFlash* pFlash,
//    /*[in]*/ long FrameNum,
//    /*[out,retval]*/ VARIANT_BOOL * loaded )
//{ 
//    HRESULT hr = E_NOTIMPL; 
//    Func_FrameLoaded pOrgFunc = (Func_FrameLoaded)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_FrameLoaded ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( pFlash );
//        pOrgFunc = (Func_FrameLoaded)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_FrameLoaded ));
//    }
//
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( pFlash, FrameNum, loaded );
//    }
//    return hr;
//}
//
//HRESULT  __stdcall CHookedFlash::Hooked_FlashVersion ( IShockwaveFlash* pFlash,
//    /*[out,retval]*/ long * version )
//{ 
//    CMsgHook::GetInstance()->SetMsgHook( GetWindowThreadProcessId(GetWindowHwd(), NULL) );
//    Func_FlashVersion pOrgFunc = (Func_FlashVersion)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_FlashVersion ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( pFlash );
//        pOrgFunc = (Func_FlashVersion)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_FlashVersion ));
//    }
//
//    HRESULT hr = E_NOTIMPL;
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( pFlash, version );
//    }
//    return hr; 
//}
//
//HRESULT  __stdcall CHookedFlash::Hooked_get_WMode ( IShockwaveFlash* pFlash,
//    /*[out,retval]*/ BSTR * pVal )
//{ 
//    HRESULT hr = E_NOTIMPL; 
//    Func_get_WMode pOrgFunc = (Func_get_WMode)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_get_WMode ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( pFlash );
//        pOrgFunc = (Func_get_WMode)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_get_WMode ));
//    }
//
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( pFlash, pVal );
//    }
//    return hr;
//}
//
//HRESULT  __stdcall CHookedFlash::Hooked_put_WMode ( IShockwaveFlash* pFlash,
//    /*[in]*/ BSTR pVal )
//{ 
//    HRESULT hr = E_NOTIMPL; 
//    Func_put_WMode pOrgFunc = (Func_put_WMode)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_put_WMode ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( pFlash );
//        pOrgFunc = (Func_put_WMode)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_put_WMode ));
//    }
//
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( pFlash, pVal );
//    }
//    return hr;
//}
//
//HRESULT  __stdcall CHookedFlash::Hooked_get_SAlign ( IShockwaveFlash* pFlash,
//    /*[out,retval]*/ BSTR * pVal )
//{ 
//    HRESULT hr = E_NOTIMPL; 
//    Func_get_SAlign pOrgFunc = (Func_get_SAlign)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_get_SAlign ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( pFlash );
//        pOrgFunc = (Func_get_SAlign)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_get_SAlign ));
//    }
//
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( pFlash, pVal );
//    }
//    return hr;
//}
//
//HRESULT  __stdcall CHookedFlash::Hooked_put_SAlign ( IShockwaveFlash* pFlash,
//    /*[in]*/ BSTR pVal )
//{ 
//    HRESULT hr = E_NOTIMPL; 
//    Func_put_SAlign pOrgFunc = (Func_put_SAlign)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_put_SAlign ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( pFlash );
//        pOrgFunc = (Func_put_SAlign)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_put_SAlign ));
//    }
//
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( pFlash, pVal );
//    }
//    return hr;
//}
//
//HRESULT  __stdcall CHookedFlash::Hooked_get_Menu ( IShockwaveFlash* pFlash,
//    /*[out,retval]*/ VARIANT_BOOL * pVal )
//{ 
//    HRESULT hr = E_NOTIMPL; 
//    Func_get_Menu pOrgFunc = (Func_get_Menu)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_get_Menu ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( pFlash );
//        pOrgFunc = (Func_get_Menu)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_get_Menu ));
//    }
//
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( pFlash, pVal );
//    }
//    return hr;
//}
//
//HRESULT  __stdcall CHookedFlash::Hooked_put_Menu ( IShockwaveFlash* pFlash,
//    /*[in]*/ VARIANT_BOOL pVal )
//{ 
//    HRESULT hr = E_NOTIMPL; 
//    Func_put_Menu pOrgFunc = (Func_put_Menu)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_put_Menu ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( pFlash );
//        pOrgFunc = (Func_put_Menu)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_put_Menu ));
//    }
//
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( pFlash, pVal );
//    }
//    return hr;
//}
//
//HRESULT  __stdcall CHookedFlash::Hooked_get_Base ( IShockwaveFlash* pFlash,
//    /*[out,retval]*/ BSTR * pVal )
//{ 
//    HRESULT hr = E_NOTIMPL; 
//    Func_get_Base pOrgFunc = (Func_get_Base)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_get_Base ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( pFlash );
//        pOrgFunc = (Func_get_Base)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_get_Base ));
//    }
//
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( pFlash, pVal );
//    }
//    return hr;
//}
//
//HRESULT  __stdcall CHookedFlash::Hooked_put_Base ( IShockwaveFlash* pFlash,
//    /*[in]*/ BSTR pVal )
//{ 
//    HRESULT hr = E_NOTIMPL; 
//    Func_put_Base pOrgFunc = (Func_put_Base)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_put_Base ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( pFlash );
//        pOrgFunc = (Func_put_Base)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_put_Base ));
//    }
//
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( pFlash, pVal );
//    }
//    return hr;
//}
//
//HRESULT  __stdcall CHookedFlash::Hooked_get_Scale ( IShockwaveFlash* pFlash,
//    /*[out,retval]*/ BSTR * pVal )
//{ 
//    HRESULT hr = E_NOTIMPL; 
//    Func_get_Scale pOrgFunc = (Func_get_Scale)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_get_Scale ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( pFlash );
//        pOrgFunc = (Func_get_Scale)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_get_Scale ));
//    }
//
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( pFlash, pVal );
//    }
//    return hr;
//}
//
//HRESULT  __stdcall CHookedFlash::Hooked_put_Scale ( IShockwaveFlash* pFlash,
//    /*[in]*/ BSTR pVal )
//{ 
//    HRESULT hr = E_NOTIMPL; 
//    Func_put_Scale pOrgFunc = (Func_put_Scale)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_put_Scale ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( pFlash );
//        pOrgFunc = (Func_put_Scale)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_put_Scale ));
//    }
//
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( pFlash, pVal );
//    }
//    return hr;
//}
//
//HRESULT  __stdcall CHookedFlash::Hooked_get_DeviceFont ( IShockwaveFlash* pFlash,
//    /*[out,retval]*/ VARIANT_BOOL * pVal )
//{ 
//    HRESULT hr = E_NOTIMPL; 
//    Func_get_DeviceFont pOrgFunc = (Func_get_DeviceFont)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_get_DeviceFont ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( pFlash );
//        pOrgFunc = (Func_get_DeviceFont)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_get_DeviceFont ));
//    }
//
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( pFlash, pVal );
//    }
//    return hr;
//}
//
//HRESULT  __stdcall CHookedFlash::Hooked_put_DeviceFont ( IShockwaveFlash* pFlash,
//    /*[in]*/ VARIANT_BOOL pVal )
//{ 
//    HRESULT hr = E_NOTIMPL; 
//    Func_put_DeviceFont pOrgFunc = (Func_put_DeviceFont)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_put_DeviceFont ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( pFlash );
//        pOrgFunc = (Func_put_DeviceFont)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_put_DeviceFont ));
//    }
//
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( pFlash, pVal );
//    }
//    return hr; 
//}
//
//HRESULT  __stdcall CHookedFlash::Hooked_get_EmbedMovie ( IShockwaveFlash* pFlash,
//    /*[out,retval]*/ VARIANT_BOOL * pVal )
//{ 
//    HRESULT hr = E_NOTIMPL; 
//    Func_get_EmbedMovie pOrgFunc = (Func_get_EmbedMovie)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_get_EmbedMovie ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( pFlash );
//        pOrgFunc = (Func_get_EmbedMovie)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_get_EmbedMovie ));
//    }
//
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( pFlash, pVal );
//    }
//    return hr; 
//}
//
//HRESULT  __stdcall CHookedFlash::Hooked_put_EmbedMovie ( IShockwaveFlash* pFlash,
//    /*[in]*/ VARIANT_BOOL pVal )
//{ 
//    HRESULT hr = E_NOTIMPL; 
//    Func_put_EmbedMovie pOrgFunc = (Func_put_EmbedMovie)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_put_EmbedMovie ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( pFlash );
//        pOrgFunc = (Func_put_EmbedMovie)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_put_EmbedMovie ));
//    }
//
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( pFlash, pVal );
//    }
//    return hr;
//}
//
//HRESULT  __stdcall CHookedFlash::Hooked_get_BGColor ( IShockwaveFlash* pFlash,
//    /*[out,retval]*/ BSTR * pVal )
//{ 
//    HRESULT hr = E_NOTIMPL; 
//    Func_get_BGColor pOrgFunc = (Func_get_BGColor)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_get_BGColor ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( pFlash );
//        pOrgFunc = (Func_get_BGColor)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_get_BGColor ));
//    }
//
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( pFlash, pVal );
//    }
//    return hr;
//}
//
//HRESULT  __stdcall CHookedFlash::Hooked_put_BGColor ( IShockwaveFlash* pFlash,
//    /*[in]*/ BSTR pVal )
//{ 
//    HRESULT hr = E_NOTIMPL; 
//    Func_put_BGColor pOrgFunc = (Func_put_BGColor)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_put_BGColor ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( pFlash );
//        pOrgFunc = (Func_put_BGColor)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_put_BGColor ));
//    }
//
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( pFlash, pVal );
//    }
//    return hr;
//}
//
//HRESULT  __stdcall CHookedFlash::Hooked_get_Quality2 ( IShockwaveFlash* pFlash,
//    /*[out,retval]*/ BSTR * pVal )
//{ 
//    HRESULT hr = E_NOTIMPL; 
//    Func_get_Quality2 pOrgFunc = (Func_get_Quality2)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_get_Quality2 ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( pFlash );
//        pOrgFunc = (Func_get_Quality2)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_get_Quality2 ));
//    }
//
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( pFlash, pVal );
//    }
//    return hr;
//}
//
//HRESULT  __stdcall CHookedFlash::Hooked_put_Quality2 ( IShockwaveFlash* pFlash,
//    /*[in]*/ BSTR pVal )
//{ 
//    HRESULT hr = E_NOTIMPL; 
//    Func_put_Quality2 pOrgFunc = (Func_put_Quality2)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_put_Quality2 ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( pFlash );
//        pOrgFunc = (Func_put_Quality2)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_put_Quality2 ));
//    }
//
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( pFlash, pVal );
//    }
//    return hr;
//}
//
//HRESULT  __stdcall CHookedFlash::Hooked_LoadMovie ( IShockwaveFlash* pFlash,
//    /*[in]*/ int layer,
//    /*[in]*/ BSTR url )
//{ 
//    HRESULT hr = E_NOTIMPL; 
//    Func_LoadMovie pOrgFunc = (Func_LoadMovie)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_LoadMovie ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( pFlash );
//        pOrgFunc = (Func_LoadMovie)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_LoadMovie ));
//    }
//
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( pFlash, layer, url );
//    }
//    return hr;
//}
//
//HRESULT  __stdcall CHookedFlash::Hooked_TGotoFrame ( IShockwaveFlash* pFlash,
//    /*[in]*/ BSTR target,
//    /*[in]*/ long FrameNum )
//{ 
//    HRESULT hr = E_NOTIMPL; 
//    Func_TGotoFrame pOrgFunc = (Func_TGotoFrame)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_TGotoFrame ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( pFlash );
//        pOrgFunc = (Func_TGotoFrame)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_TGotoFrame ));
//    }
//
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( pFlash, target, FrameNum );
//    }
//    return hr;
//}
//
//HRESULT  __stdcall CHookedFlash::Hooked_TGotoLabel ( IShockwaveFlash* pFlash,
//    /*[in]*/ BSTR target,
//    /*[in]*/ BSTR label )
//{ 
//    HRESULT hr = E_NOTIMPL; 
//    Func_TGotoLabel pOrgFunc = (Func_TGotoLabel)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_TGotoLabel ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( pFlash );
//        pOrgFunc = (Func_TGotoLabel)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_TGotoLabel ));
//    }
//
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( pFlash, target, label );
//    }
//    return hr;
//}
//
//HRESULT  __stdcall CHookedFlash::Hooked_TCurrentFrame ( IShockwaveFlash* pFlash,
//    /*[in]*/ BSTR target,
//    /*[out,retval]*/ long * FrameNum )
//{ 
//    HRESULT hr = E_NOTIMPL; 
//    Func_TCurrentFrame pOrgFunc = (Func_TCurrentFrame)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_TCurrentFrame ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( pFlash );
//        pOrgFunc = (Func_TCurrentFrame)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_TCurrentFrame ));
//    }
//
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( pFlash, target, FrameNum );
//    }
//    return hr;
//}
//
//HRESULT  __stdcall CHookedFlash::Hooked_TCurrentLabel ( IShockwaveFlash* pFlash,
//    /*[in]*/ BSTR target,
//    /*[out,retval]*/ BSTR * pVal )
//{ 
//    HRESULT hr = E_NOTIMPL; 
//    Func_TCurrentLabel pOrgFunc = (Func_TCurrentLabel)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_TCurrentLabel ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( pFlash );
//        pOrgFunc = (Func_TCurrentLabel)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_TCurrentLabel ));
//    }
//
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( pFlash, target, pVal );
//    }
//    return hr;
//}
//
//HRESULT  __stdcall CHookedFlash::Hooked_TPlay ( IShockwaveFlash* pFlash,
//    /*[in]*/ BSTR target )
//{ 
//    HRESULT hr = E_NOTIMPL; 
//    Func_TPlay pOrgFunc = (Func_TPlay)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_TPlay ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( pFlash );
//        pOrgFunc = (Func_TPlay)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_TPlay ));
//    }
//
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( pFlash, target );
//    }
//    return hr;
//}
//
//HRESULT  __stdcall CHookedFlash::Hooked_TStopPlay ( IShockwaveFlash* pFlash,
//    /*[in]*/ BSTR target )
//{ 
//    HRESULT hr = E_NOTIMPL; 
//    Func_TStopPlay pOrgFunc = (Func_TStopPlay)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_TStopPlay ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( pFlash );
//        pOrgFunc = (Func_TStopPlay)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_TStopPlay ));
//    }
//
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( pFlash, target );
//    }
//    return hr;
//}
//
//HRESULT  __stdcall CHookedFlash::Hooked_SetVariable ( IShockwaveFlash* pFlash,
//    /*[in]*/ BSTR name,
//    /*[in]*/ BSTR value )
//{ 
//    HRESULT hr = E_NOTIMPL; 
//    Func_SetVariable pOrgFunc = (Func_SetVariable)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_SetVariable ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( pFlash );
//        pOrgFunc = (Func_SetVariable)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_SetVariable ));
//    }
//
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( pFlash, name, value );
//    }
//    return hr;
//}
//
//HRESULT  __stdcall CHookedFlash::Hooked_GetVariable ( IShockwaveFlash* pFlash,
//    /*[in]*/ BSTR name,
//    /*[out,retval]*/ BSTR * pVal )
//{ 
//    HRESULT hr = E_NOTIMPL; 
//    Func_GetVariable pOrgFunc = (Func_GetVariable)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_GetVariable ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( pFlash );
//        pOrgFunc = (Func_GetVariable)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_GetVariable ));
//    }
//
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( pFlash, name, pVal );
//    }
//    return hr;
//}
//
//HRESULT  __stdcall CHookedFlash::Hooked_TSetProperty ( IShockwaveFlash* pFlash,
//    /*[in]*/ BSTR target,
//    /*[in]*/ int property,
//    /*[in]*/ BSTR value )
//{ 
//    HRESULT hr = E_NOTIMPL; 
//    Func_TSetProperty pOrgFunc = (Func_TSetProperty)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_TSetProperty ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( pFlash );
//        pOrgFunc = (Func_TSetProperty)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_TSetProperty ));
//    }
//
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( pFlash, target, property, value );
//    }
//    return hr;
//}
//
//HRESULT  __stdcall CHookedFlash::Hooked_TGetProperty ( IShockwaveFlash* pFlash,
//    /*[in]*/ BSTR target,
//    /*[in]*/ int property,
//    /*[out,retval]*/ BSTR * pVal )
//{ 
//    HRESULT hr = E_NOTIMPL; 
//    Func_TGetProperty pOrgFunc = (Func_TGetProperty)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_TGetProperty ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( pFlash );
//        pOrgFunc = (Func_TGetProperty)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_TGetProperty ));
//    }
//
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( pFlash, target, property, pVal );
//    }
//    return hr;
//}
//
//HRESULT  __stdcall CHookedFlash::Hooked_TCallFrame ( IShockwaveFlash* pFlash,
//    /*[in]*/ BSTR target,
//    /*[in]*/ int FrameNum )
//{ 
//    HRESULT hr = E_NOTIMPL; 
//    Func_TCallFrame pOrgFunc = (Func_TCallFrame)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_TCallFrame ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( pFlash );
//        pOrgFunc = (Func_TCallFrame)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_TCallFrame ));
//    }
//
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( pFlash, target, FrameNum );
//    }
//    return hr;
//}
//
//HRESULT  __stdcall CHookedFlash::Hooked_TCallLabel ( IShockwaveFlash* pFlash,
//    /*[in]*/ BSTR target,
//    /*[in]*/ BSTR label )
//{ 
//    HRESULT hr = E_NOTIMPL; 
//    Func_TCallLabel pOrgFunc = (Func_TCallLabel)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_TCallLabel ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( pFlash );
//        pOrgFunc = (Func_TCallLabel)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_TCallLabel ));
//    }
//
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( pFlash, target, label );
//    }
//    return hr;
//}
//
//HRESULT  __stdcall CHookedFlash::Hooked_TSetPropertyNum ( IShockwaveFlash* pFlash,
//    /*[in]*/ BSTR target,
//    /*[in]*/ int property,
//    /*[in]*/ double value )
//{ 
//    HRESULT hr = E_NOTIMPL; 
//    Func_TSetPropertyNum pOrgFunc = (Func_TSetPropertyNum)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_TSetPropertyNum ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( pFlash );
//        pOrgFunc = (Func_TSetPropertyNum)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_TSetPropertyNum ));
//    }
//
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( pFlash, target, property, value );
//    }
//    return hr;
//}
//
//HRESULT  __stdcall CHookedFlash::Hooked_TGetPropertyNum ( IShockwaveFlash* pFlash,
//    /*[in]*/ BSTR target,
//    /*[in]*/ int property,
//    /*[out,retval]*/ double * pVal )
//{ 
//    HRESULT hr = E_NOTIMPL; 
//    Func_TGetPropertyNum pOrgFunc = (Func_TGetPropertyNum)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_TGetPropertyNum ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( pFlash );
//        pOrgFunc = (Func_TGetPropertyNum)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_TGetPropertyNum ));
//    }
//
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( pFlash, target, property, pVal );
//    }
//    return hr;
//}
//
//HRESULT  __stdcall CHookedFlash::Hooked_TGetPropertyAsNumber ( IShockwaveFlash* pFlash,
//    /*[in]*/ BSTR target,
//    /*[in]*/ int property,
//    /*[out,retval]*/ double * pVal )
//{ 
//    HRESULT hr = E_NOTIMPL; 
//    Func_TGetPropertyAsNumber pOrgFunc = (Func_TGetPropertyAsNumber)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_TGetPropertyAsNumber ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( pFlash );
//        pOrgFunc = (Func_TGetPropertyAsNumber)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_TGetPropertyAsNumber ));
//    }
//
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( pFlash, target, property, pVal );
//    }
//    return hr;
//}
//
//HRESULT  __stdcall CHookedFlash::Hooked_get_SWRemote ( IShockwaveFlash* pFlash,
//    /*[out,retval]*/ BSTR * pVal )
//{ 
//    HRESULT hr = E_NOTIMPL; 
//    Func_get_SWRemote pOrgFunc = (Func_get_SWRemote)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_get_SWRemote ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( pFlash );
//        pOrgFunc = (Func_get_SWRemote)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_get_SWRemote ));
//    }
//
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( pFlash, pVal );
//    }
//    return hr;
//}
//
//HRESULT  __stdcall CHookedFlash::Hooked_put_SWRemote ( IShockwaveFlash* pFlash,
//    /*[in]*/ BSTR pVal )
//{ 
//    HRESULT hr = E_NOTIMPL; 
//    Func_put_SWRemote pOrgFunc = (Func_put_SWRemote)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_put_SWRemote ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( pFlash );
//        pOrgFunc = (Func_put_SWRemote)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_put_SWRemote ));
//    }
//
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( pFlash, pVal );
//    }
//    return hr;
//}
//
//HRESULT  __stdcall CHookedFlash::Hooked_get_FlashVars ( IShockwaveFlash* pFlash,
//    /*[out,retval]*/ BSTR * pVal )
//{ 
//    HRESULT hr = E_NOTIMPL; 
//    Func_get_FlashVars pOrgFunc = (Func_get_FlashVars)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_get_FlashVars ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( pFlash );
//        pOrgFunc = (Func_get_FlashVars)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_get_FlashVars ));
//    }
//
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( pFlash, pVal );
//    }
//    return hr;
//}
//
//HRESULT  __stdcall CHookedFlash::Hooked_put_FlashVars ( IShockwaveFlash* pFlash,
//    /*[in]*/ BSTR pVal )
//{ 
//    HRESULT hr = E_NOTIMPL; 
//    Func_put_FlashVars pOrgFunc = (Func_put_FlashVars)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_put_FlashVars ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( pFlash );
//        pOrgFunc = (Func_put_FlashVars)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_put_FlashVars ));
//    }
//
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( pFlash, pVal );
//    }
//    return hr;
//}
//
//HRESULT  __stdcall CHookedFlash::Hooked_get_AllowScriptAccess ( IShockwaveFlash* pFlash,
//    /*[out,retval]*/ BSTR * pVal )
//{ 
//    HRESULT hr = E_NOTIMPL; 
//    Func_get_AllowScriptAccess pOrgFunc = (Func_get_AllowScriptAccess)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_get_AllowScriptAccess ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( pFlash );
//        pOrgFunc = (Func_get_AllowScriptAccess)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_get_AllowScriptAccess ));
//    }
//
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( pFlash, pVal );
//    }
//    return hr;
//}
//
//HRESULT  __stdcall CHookedFlash::Hooked_put_AllowScriptAccess ( IShockwaveFlash* pFlash,
//    /*[in]*/ BSTR pVal )
//{ 
//    HRESULT hr = E_NOTIMPL; 
//    Func_put_AllowScriptAccess pOrgFunc = (Func_put_AllowScriptAccess)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_put_AllowScriptAccess ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( pFlash );
//        pOrgFunc = (Func_put_AllowScriptAccess)(GetInstance()->GetOrgFunc( (void*)pFlash, Hooked_put_AllowScriptAccess ));
//    }
//
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( pFlash, pVal );
//    }
//    return hr;
//}

