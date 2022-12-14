

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 7.00.0555 */
/* at Wed Dec 18 09:49:44 2013
 */
/* Compiler settings for src/cbPep.idl:
    Oicf, W1, Zp8, env=Win64 (32b run), target_arch=AMD64 7.00.0555 
    protocol : dce , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
/* @@MIDL_FILE_HEADING(  ) */

#pragma warning( disable: 4049 )  /* more than 64k source lines */


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 475
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif // __RPCNDR_H_VERSION__

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __cbPep_h__
#define __cbPep_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __INxtShellExtension_FWD_DEFINED__
#define __INxtShellExtension_FWD_DEFINED__
typedef interface INxtShellExtension INxtShellExtension;
#endif 	/* __INxtShellExtension_FWD_DEFINED__ */


#ifndef __NxtShellExtension_FWD_DEFINED__
#define __NxtShellExtension_FWD_DEFINED__

#ifdef __cplusplus
typedef class NxtShellExtension NxtShellExtension;
#else
typedef struct NxtShellExtension NxtShellExtension;
#endif /* __cplusplus */

#endif 	/* __NxtShellExtension_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 


#ifndef __INxtShellExtension_INTERFACE_DEFINED__
#define __INxtShellExtension_INTERFACE_DEFINED__

/* interface INxtShellExtension */
/* [unique][helpstring][nonextensible][dual][uuid][object] */ 


EXTERN_C const IID IID_INxtShellExtension;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("069C9AA9-4DBB-4CA5-9FE2-4585C0F6360D")
    INxtShellExtension : public IDispatch
    {
    public:
    };
    
#else 	/* C style interface */

    typedef struct INxtShellExtensionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            INxtShellExtension * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            INxtShellExtension * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            INxtShellExtension * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            INxtShellExtension * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            INxtShellExtension * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            INxtShellExtension * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            INxtShellExtension * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        END_INTERFACE
    } INxtShellExtensionVtbl;

    interface INxtShellExtension
    {
        CONST_VTBL struct INxtShellExtensionVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define INxtShellExtension_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define INxtShellExtension_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define INxtShellExtension_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define INxtShellExtension_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define INxtShellExtension_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define INxtShellExtension_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define INxtShellExtension_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __INxtShellExtension_INTERFACE_DEFINED__ */



#ifndef __cbPepLib_LIBRARY_DEFINED__
#define __cbPepLib_LIBRARY_DEFINED__

/* library cbPepLib */
/* [helpstring][version][uuid] */ 


EXTERN_C const IID LIBID_cbPepLib;

EXTERN_C const CLSID CLSID_NxtShellExtension;

#ifdef __cplusplus

class DECLSPEC_UUID("7ABFB944-EB91-4E60-826E-BC9AB54DD6AB")
NxtShellExtension;
#endif
#endif /* __cbPepLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


