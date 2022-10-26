

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 7.00.0555 */
/* at Wed Dec 18 11:02:00 2013
 */
/* Compiler settings for src/OutlookAddin.idl:
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

#ifndef __OutlookAddin_h__
#define __OutlookAddin_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __IoutlookImpl_FWD_DEFINED__
#define __IoutlookImpl_FWD_DEFINED__
typedef interface IoutlookImpl IoutlookImpl;
#endif 	/* __IoutlookImpl_FWD_DEFINED__ */


#ifndef __outlookImpl_FWD_DEFINED__
#define __outlookImpl_FWD_DEFINED__

#ifdef __cplusplus
typedef class outlookImpl outlookImpl;
#else
typedef struct outlookImpl outlookImpl;
#endif /* __cplusplus */

#endif 	/* __outlookImpl_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 


#ifndef __IoutlookImpl_INTERFACE_DEFINED__
#define __IoutlookImpl_INTERFACE_DEFINED__

/* interface IoutlookImpl */
/* [unique][helpstring][nonextensible][dual][uuid][object] */ 


EXTERN_C const IID IID_IoutlookImpl;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("1EAB084D-CCDA-456F-A467-BB3261DCE3D0")
    IoutlookImpl : public IDispatch
    {
    public:
    };
    
#else 	/* C style interface */

    typedef struct IoutlookImplVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IoutlookImpl * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IoutlookImpl * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IoutlookImpl * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IoutlookImpl * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IoutlookImpl * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IoutlookImpl * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IoutlookImpl * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        END_INTERFACE
    } IoutlookImplVtbl;

    interface IoutlookImpl
    {
        CONST_VTBL struct IoutlookImplVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IoutlookImpl_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IoutlookImpl_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IoutlookImpl_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IoutlookImpl_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IoutlookImpl_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IoutlookImpl_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IoutlookImpl_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IoutlookImpl_INTERFACE_DEFINED__ */



#ifndef __OutlookAddinLib_LIBRARY_DEFINED__
#define __OutlookAddinLib_LIBRARY_DEFINED__

/* library OutlookAddinLib */
/* [helpstring][version][uuid] */ 


EXTERN_C const IID LIBID_OutlookAddinLib;

EXTERN_C const CLSID CLSID_outlookImpl;

#ifdef __cplusplus

class DECLSPEC_UUID("09C3BB91-4D93-471F-AB17-8CF422D48156")
outlookImpl;
#endif
#endif /* __OutlookAddinLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


