

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 7.00.0555 */
/* at Mon Dec 21 14:07:08 2015
 */
/* Compiler settings for src/CE_Explorer.idl:
    Oicf, W1, Zp8, env=Win32 (32b run), target_arch=X86 7.00.0555 
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

#ifndef __CE_Explorer_h__
#define __CE_Explorer_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __ICOEShell_FWD_DEFINED__
#define __ICOEShell_FWD_DEFINED__
typedef interface ICOEShell ICOEShell;
#endif 	/* __ICOEShell_FWD_DEFINED__ */


#ifndef __COEShell_FWD_DEFINED__
#define __COEShell_FWD_DEFINED__

#ifdef __cplusplus
typedef class COEShell COEShell;
#else
typedef struct COEShell COEShell;
#endif /* __cplusplus */

#endif 	/* __COEShell_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 


#ifndef __ICOEShell_INTERFACE_DEFINED__
#define __ICOEShell_INTERFACE_DEFINED__

/* interface ICOEShell */
/* [unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_ICOEShell;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("09A48212-1DDA-4BC8-8A7E-6C3DA6C3541B")
    ICOEShell : public IUnknown
    {
    public:
    };
    
#else 	/* C style interface */

    typedef struct ICOEShellVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICOEShell * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICOEShell * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICOEShell * This);
        
        END_INTERFACE
    } ICOEShellVtbl;

    interface ICOEShell
    {
        CONST_VTBL struct ICOEShellVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICOEShell_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define ICOEShell_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define ICOEShell_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __ICOEShell_INTERFACE_DEFINED__ */



#ifndef __CE_ExplorerLib_LIBRARY_DEFINED__
#define __CE_ExplorerLib_LIBRARY_DEFINED__

/* library CE_ExplorerLib */
/* [helpstring][version][uuid] */ 


EXTERN_C const IID LIBID_CE_ExplorerLib;

EXTERN_C const CLSID CLSID_COEShell;

#ifdef __cplusplus

class DECLSPEC_UUID("38B14C4F-31AE-468B-8BD2-DCB57645074A")
COEShell;
#endif
#endif /* __CE_ExplorerLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


