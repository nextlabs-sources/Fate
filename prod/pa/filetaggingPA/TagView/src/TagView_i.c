

/* this ALWAYS GENERATED file contains the IIDs and CLSIDs */

/* link this file in with the server and any clients */


 /* File created by MIDL compiler version 7.00.0500 */
/* at Wed Jan 27 08:01:02 2010
 */
/* Compiler settings for .\TagView.idl:
    Oicf, W1, Zp8, env=Win32 (32b run)
    protocol : dce , ms_ext, c_ext
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
//@@MIDL_FILE_HEADING(  )

#pragma warning( disable: 4049 )  /* more than 64k source lines */


#ifdef __cplusplus
extern "C"{
#endif 


#include <rpc.h>
#include <rpcndr.h>

#ifdef _MIDL_USE_GUIDDEF_

#ifndef INITGUID
#define INITGUID
#include <guiddef.h>
#undef INITGUID
#else
#include <guiddef.h>
#endif

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
        DEFINE_GUID(name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8)

#else // !_MIDL_USE_GUIDDEF_

#ifndef __IID_DEFINED__
#define __IID_DEFINED__

typedef struct _IID
{
    unsigned long x;
    unsigned short s1;
    unsigned short s2;
    unsigned char  c[8];
} IID;

#endif // __IID_DEFINED__

#ifndef CLSID_DEFINED
#define CLSID_DEFINED
typedef IID CLSID;
#endif // CLSID_DEFINED

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
        const type name = {l,w1,w2,{b1,b2,b3,b4,b5,b6,b7,b8}}

#endif !_MIDL_USE_GUIDDEF_

MIDL_DEFINE_GUID(IID, IID_IComponentRegistrar,0xa817e7a2,0x43fa,0x11d0,0x9e,0x44,0x00,0xaa,0x00,0xb6,0x77,0x0a);


MIDL_DEFINE_GUID(IID, IID_IShellExt,0x2772336A,0x9A23,0x4978,0x80,0xF8,0x0E,0x6C,0xC6,0x1E,0x42,0xAD);


MIDL_DEFINE_GUID(IID, LIBID_TagViewLib,0x841E6D40,0xDBFD,0x47F2,0x87,0x56,0x99,0x9F,0x95,0x2E,0x6C,0x56);


MIDL_DEFINE_GUID(CLSID, CLSID_CompReg,0x93FBAC56,0x1C03,0x4907,0x8B,0xA8,0xE6,0x54,0xD9,0x30,0xA4,0xE3);


MIDL_DEFINE_GUID(CLSID, CLSID_ShellExt,0xE7F164F5,0x8E7C,0x4AAB,0xBC,0x8D,0x96,0xA3,0x96,0x66,0x1B,0xFA);

#undef MIDL_DEFINE_GUID

#ifdef __cplusplus
}
#endif



