/* Copyright 2003-2006, Voltage Security, all rights reserved.
 */

/* This include file contains the definitions necessary to build client
 * applications that use the Voltage Toolkit API.
 *
 * The include file vibe.h contains many opaque types (objects and
 * contexts, for example). The actual definitions are made internally.
 * This is for porting purposes. Different platforms may require
 * different definitions of specific fields of data structures (e.g.
 * unsigned int versus unsigned long). This include file that defines
 * the API is to be platform independent. Any platform differences
 * should not be reflected in this file.
 *
 * Note that the #define of VT_CALLING_CONV is not a platform issue.
 * Two build of the library on the same platform (chip, OS, compiler)
 * may use different definitions for these #defines based on how the
 * application wants to call the toolkit's functions.
 */

#include "vibecrypto.h"

#ifndef _VIBE_H
#define _VIBE_H

#ifdef __cplusplus
extern "C" {
#endif

/*=========================================================*/
/*                                                         */
/* Library Context                                         */
/*                                                         */
/*=========================================================*/

/* The Library Context along with create, destroy and other functions
 * are declared in vibecrypto.h. Those functions are part of
 * vibecrypto.lib (or equivalent library). The following are
 * declarations of library context operations found in vibe.lib.
 */

/* These are the VtLibCtxParams supported by the toolkit. Each
 * VtLibCtxParam is used in conjunction with special info for the
 * function. If there is no special info, the accompaniment is a NULL
 * pointer.
 */

/** SetParam and GetParam.
 * <p>Set or get the error context associated with the library context.
 * <p>When setting, build the VtErrorCtx and pass it as the
 * associatedInfo.
 * <p>When getting, pass the address of a VtErrorCtx variable as
 * the associated info. IMPORTANT NOTE!!! This LibCtxParam will return
 * a reference to the previously specified error context object, it will
 * not clone it, so the library context retains ownership of the error
 * context. 
 */
extern VtLibCtxParam VtLibCtxParamErrorCtx;

/** SetParam and GetParam.
 * <p>Set or get the VerifyFailureList associated with the library
 * context.
 * <p>When setting, build the VtVerifyFailureList and pass it as the
 * associatedInfo.
 * <p>When getting, pass the address of a VtVerifyFailureList variable
 * as the associated info. IMPORTANT NOTE!!! This LibCtxParam will
 * return a reference to the previously specified verify failure list,
 * it will not clone it, so the library context retains ownership of
 * the verify failure list.
 */
extern VtLibCtxParam VtLibCtxParamVerifyFailureList;

/** SetParam only.
 * <p>Set the library context to indicate whether functions (such as
 * transport context operations) are allowed to attempt to make network
 * access calls. This will generally be called to turn off network
 * access.
 * <p>The default is "allowed". That is, if VtSetLibCtxParam is never
 * called with VtLibCtxParamNetAccess, the toolkit will allow network
 * access.
 * <p>The data associated with VtLibCtxParamNetAccess is a pointer to
 * an unsigned int set to either VT_ALLOWED or VT_NOT_ALLOWED.
 */
extern VtLibCtxParam VtLibCtxParamNetAccess;

/** SetParam only.
 * <p>Set the library context to indicate whether functions (such as
 * transport context operations) are allowed to attempt to make user
 * interface (UI) calls. This will generally be called to turn off UI
 * access.
 * <p>The default is "allowed". That is, if VtSetLibCtxParam is never
 * called with VtLibCtxParamUIAccess, the toolkit will allow UI access.
 * <p>The data associated with VtLibCtxParamUIAccess is a pointer to an
 * unsigned int set to either VT_ALLOWED or VT_NOT_ALLOWED.
 */
extern VtLibCtxParam VtLibCtxParamUIAccess;

/** For use with VtLibCtxParams, set a flag to this value to indicate
 * something is allowed.
 */
#define VT_ALLOWED      1
/** For use with VtLibCtxParams, set a flag to this value to indicate
 * something is not allowed.
 */
#define VT_NOT_ALLOWED  0

/** SetParam and GetParam
 * <p>This VtLibCtxParam is used to set a library context with a file
 * context.
 * <p>Or it is used to get the file ctx out of the libeCtx.
 * <p>Some toolkit functions or contexts will need a file ctx to operate.
 * These functions will generally have an argument that is the file ctx
 * to use. If an application will be using the same file ctx for all
 * function calls, it can store the file ctx in the libCtx. When
 * toolkit functions need a file ctx, they can use the one supplied to
 * them as an argument, or if there is none, find the file ctx inside
 * the libCtx.
 * <p>This LibCtxParam can be used to set only once with each libCtx.
 * That is, it is possible to set a libCtx with one file ctx only.
 * <p>IMPORTANT NOTE!!! This LibCtxParam will copy a reference to the
 * supplied file ctx, it will not clone it.
 * <p>The associated info is a VtFileCtx.
 * <p>When setting, build the VtFileCtx and pass it as the
 * associatedInfo.
 * <p>When getting, pass in the address of a VtFileCtx variable as the
 * getInfo, the Get function will deposit a file ctx at the address.
 * The returned ctx is the same one passed to the Set function.
 */
extern VtLibCtxParam VtLibCtxParamFileCtx;

/** SetParam and GetParam
 * <p>This VtLibCtxParam is used to set a library context with a policy
 * context.
 * <p>Or it is used to get the policy ctx out of the libeCtx.
 * <p>Many toolkit functions need a policy ctx to operate. These
 * functions will generally have an argument that is the policy ctx to
 * use. If an application will be using the same policy ctx for all
 * function calls, it can store the ctx in the libCtx. When toolkit
 * functions need a policy ctx, they can use the one supplied to them
 * as an argument, or if there is none, find the policy ctx inside the
 * libCtx.
 * <p>This LibCtxParam can be used to set only once with each libCtx.
 * That is, it is possible to set a libCtx with one policy ctx only.
 * <p>IMPORTANT NOTE!!! This LibCtxParam will copy a reference to the
 * supplied policy ctx, it will not clone it.
 * <p>The associated info is a VtPolicyCtx.
 * <p>When setting, build the VtPolicyCtx and pass it as the
 * associatedInfo.
 * <p>When getting, pass in the address of a VtPolicyCtx variable as
 * the getInfo, the Get function will deposit a policy ctx at the
 * address. The returned ctx is the same one passed to the Set function.
 */
extern VtLibCtxParam VtLibCtxParamPolicyCtx;

/** SetParam and GetParam
 * <p>This VtLibCtxParam is used to set a library context with a
 * transport context.
 * <p>Or it is used to get the transport ctx out of the libeCtx.
 * <p>Many toolkit functions need a transport ctx to operate. These
 * functions will generally have an argument that is the transport ctx
 * to use. If an application will be using the same transport ctx for
 * all function calls, it can store the ctx in the libCtx. When toolkit
 * functions need a transport ctx, they can use the one supplied to
 * them as an argument, or if there is none, find the transport ctx
 * inside the libCtx.
 * <p>This LibCtxParam can be used to set only once with each libCtx.
 * That is, it is possible to set a libCtx with one transport ctx only.
 * <p>IMPORTANT NOTE!!! This LibCtxParam will copy a reference to the
 * supplied transport ctx, it will not clone it.
 * <p>The associated info is a VtTransportCtx.
 * <p>When setting, build the VtTransportCtx and pass it as the
 * associatedInfo.
 * <p>When getting, pass in the address of a VtTransportCtx variable as
 * the getInfo, the Get function will deposit a transport ctx at the
 * address. The returned ctx is the same one passed to the Set function.
 */
extern VtLibCtxParam VtLibCtxParamTransportCtx;

/** SetParam and GetParam
 * <p>This VtLibCtxParam is used to set a library context with a
 * storage context.
 * <p>Or it is used to get the storage ctx out of the libeCtx.
 * <p>Many toolkit functions need a storage ctx to operate. These
 * functions will generally have an argument that is the storage ctx
 * to use. If an application will be using the same storage ctx for
 * all function calls, it can store the ctx in the libCtx. When toolkit
 * functions need a storage ctx, they can use the one supplied to
 * them as an argument, or if there is none, find the storage ctx
 * inside the libCtx.
 * <p>This LibCtxParam can be used to set only once with each libCtx.
 * That is, it is possible to set a libCtx with one storage ctx only.
 * <p>IMPORTANT NOTE!!! This LibCtxParam will copy a reference to the
 * supplied storage ctx, it will not clone it.
 * <p>The associated info is a VtStorageCtx.
 * <p>When setting, build the VtStorageCtx and pass it as the
 * associatedInfo.
 * <p>When getting, pass in the address of a VtStorageCtx variable as
 * the getInfo, the Get function will deposit a storage ctx at the
 * address. The returned ctx is the same one passed to the Set function.
 */
extern VtLibCtxParam VtLibCtxParamStorageCtx;

/** SetParam and GetParam
 * <p>This VtLibCtxParam is used to set a library context with a
 * cert verify context.
 * <p>Or it is used to get the cert verify ctx out of the libeCtx.
 * <p>Many toolkit functions need a cert verify ctx to operate. These
 * functions will generally have an argument that is the cert verify
 * ctx to use. If an application will be using the same cert verify ctx
 * for all function calls, it can store the ctx in the libCtx. When
 * toolkit functions need a cert verify ctx, they can use the one
 * supplied to them as an argument, or if there is none, find the
 * cert verify ctx inside the libCtx.
 * <p>This LibCtxParam can be used to set only once with each libCtx.
 * That is, it is possible to set a libCtx with one cert verify ctx only.
 * <p>IMPORTANT NOTE!!! This LibCtxParam will copy a reference to the
 * supplied cert verify ctx, it will not clone it.
 * <p>The associated info is a VtCertVerifyCtx.
 * <p>When setting, build the VtCertVerifyCtx and pass it as the
 * associatedInfo.
 * <p>When getting, pass in the address of a VtCertVerifyCtx variable as
 * the getInfo, the Get function will deposit a cert verify ctx at the
 * address. The returned ctx is the same one passed to the Set function.
 */
extern VtLibCtxParam VtLibCtxParamCertVerifyCtx;

/** SetParam and GetParam
 * <p>This VtLibCtxParam is used to set a library context with an MpInt
 * context.
 * <p>Or it is used to get the MpInt ctx out of the libeCtx.
 * <p>Many toolkit functions need an MpInt ctx to operate. These
 * functions will generally have an argument that is the MpInt ctx
 * to use. If an application will be using the same MpInt ctx for all
 * function calls, it can store the ctx in the libCtx. When toolkit
 * functions need an MpInt ctx, they can use the one supplied to them
 * as an argument, or if there is none, find the MpInt ctx inside the
 * libCtx.
 * <p>This LibCtxParam can be used to set only once with each libCtx.
 * That is, it is possible to set a libCtx with one MpInt ctx only.
 * <p>IMPORTANT NOTE!!! This LibCtxParam will copy a reference to the
 * supplied MpInt ctx, it will not clone it.
 * <p>The associated info is a VtMpIntCtx.
 * <p>When setting, build the VtMpIntCtx and pass it as the
 * associatedInfo.
 * <p>When getting, pass in the address of a VtMpIntCtx variable as
 * the getInfo, the Get function will deposit an MpInt ctx at the
 * address. The returned ctx is the same one passed to the Set function.
 */
extern VtLibCtxParam VtLibCtxParamMpCtx;

/** SetParam and GetParam
 * <p>This VtLibCtxParam is used to set a library context with a random
 * object.
 * <p>Or it is used to get the random object out of the libeCtx.
 * <p>Many toolkit functions need a random object to operate. These
 * functions will generally have an argument that is the random object
 * to use. If an application will be using the same random object for
 * all function calls, it can store the object in the libCtx. When
 * toolkit functions need a random object, they can use the one
 * supplied to them as an argument, or if there is none, find the
 * random object inside the libCtx.
 * <p>This LibCtxParam can be used to set only once with each libCtx.
 * That is, it is possible to set a libCtx with one random object only.
 * <p>IMPORTANT NOTE!!! This LibCtxParam will copy a reference to the
 * supplied random object, it will not clone it.
 * <p>The associated info is a VtRandomObject.
 * <p>When setting, build the VtRandomObject and pass it as the
 * associatedInfo.
 * <p>When getting, pass in the address of a VtRandomObject variable as
 * the getInfo, the Get function will deposit a random object at the
 * address. The returned object is the same one passed to the Set
 * function.
 */
extern VtLibCtxParam VtLibCtxParamRandomObj;

/** SetParam only.
 * <p>This VtLibCtxParam is used to set a library context with an array
 * of DerCoders.
 * <p>Many toolkit functions need an an array of DER coders to operate.
 * These functions will generally have an argument that is the coder
 * array to use. If an application will be using the same DER coder
 * array in all function calls, it can store the array in the libCtx.
 * When toolkit functions need a coder array, they can use the one
 * supplied to them as an argument, or if there is none, find the array
 * inside the libCtx.
 * <p>This LibCtxParam can be used only once with each libCtx. That is,
 * it is possible to set a libCtx with one DER coder array only.
 * <p>The associated info is a VtDERCoderArray struct.
 * <p>When setting, build the VtDERCoderArray and pass it's address as
 * the associatedInfo.
 */
extern VtLibCtxParam VtLibCtxParamDERCoderArray;

/** SetParam only.
 * <p>This VtLibCtxParam is used to set a library context with an array
 * of SchemaDecodes.
 * <p>Many toolkit functions need an an array of identity schema
 * decoders to operate. These functions will generally have an argument
 * that is the decoder array to use. If an application will be using
 * the same schema decoder array in all function calls, it can store
 * the array in the libCtx. When toolkit functions need a decoder
 * array, they can use the one supplied to them as an argument, or if
 * there is none, find the array inside the libCtx.
 * <p>This LibCtxParam can be used only once with each libCtx. That is,
 * it is possible to set a libCtx with one decoder array only.
 * <p>The associated info is a VtSchemaDecodeArray struct.
 * <p>When setting, build the VtSchemaDecodeArray and pass its address
 * as the associatedInfo.
 */
extern VtLibCtxParam VtLibCtxParamSchemaDecodeArray;

/** SetParam and GetParam.
 * <p>Set or get the connection cache context associated with the
 * library context.
 * <p>When setting, build the VtConnectionCacheCtx and pass it as the
 * associatedInfo.
 * <p>When getting, pass the address of a VtConnectionCacheCtx variable
 * as the associated info. IMPORTANT NOTE!!! This LibCtxParam will
 * return a reference to the previously specified connection cache
 * context object, it will not clone it, so the library context retains
 * ownership of the connection cache context. 
 */
extern VtLibCtxParam VtLibCtxParamConnectionCacheCtx;

/** This function is available for convenience, it saves you the work
 * of building all the supporting objects and contexts (Random,
 * Transport, Storage, Policy, etc.). It will create a library context,
 * using default Windows memory and threading Impls. It will then
 * build other objects and contexts and load them into the created
 * libCtx.
 * <p>Call this routine if you are building an app for Windows and are
 * willing to use the memory, threading, Transport, Storage, Policy,
 * etc. Impls used by this default. If you build your own libCtx and
 * load in the objects and contexts you specify, your app's code size
 * can be smaller. However, this function is more convenient.
 * <p>Call VtDestroyLibCtx when you are done with it.
 * <p>This function will use
 * <code>
 * <pre>
 *   VtMemoryImplWin32
 *   VtThreadImplWin32Multi
 * </pre>
 * </code>
 * <p>It will build the following objects and contexts and load them
 * (using VtSetLibCtxParam).
 * <code>
 * <pre>
 *   ErrorCtx             VtErrorCtxImplBasic
 *   MpIntCtx             VtMpIntImplOpenSSL
 *   IBECacheCtx          VtIBECacheCtxImplBasic
 *   FileCtx              VtFileImplWin32
 *   PolicyCtx            VtPolicyImplXmlURLWinINet
 *   StorageCtx           one provider: VtStorageProviderFileWin32
 *   TransportCtx         VtTransportImplHttpsWinINet
 *   CertVerifyCtx        VtCertVerifyImplBasic
 *   VerifyFailureList    VtVerifyFailureListImplBasic
 *   RandomObject         VtRandomImplFips186Prng (seeded using CAPI)
 *   DERCoderArray        all supported DerCoders
 *   SchemaDecodeArray    all supported SchemaDecodes
 * </pre>
 * </code>
 * <p>If you would like to seed the random object further, you can get
 * a reference to that object using VtGetLibCtxParam.
 * <p>If you would like to load the IBECacheCtx into another libCtx,
 * you can get a reference to that context by using VtGetLibCtxParam.
 * <p>This function will use the policy provider that downloads the
 * policy info from a URL. You must supply the URL (the policyURL
 * argument). Alternatively, you can pass a NULL for the URL and the
 * function will not build a policy context. In that case, you should
 * build your own policy context, which you can load into the libCtx
 * using VtSetLibCtxParam.
 * <p>The policyURL argument is an unsigned char array. It is a UTF-8
 * string and NULL-terminated.
 *
 * @param libCtx A pointer to where the routine will deposit the
 * created library context.
 * @param policyURL The UTF-8, NULL-terminated string that is the URL
 * where the policy will be found, or NULL, indicating the function
 * should not build a policy ctx.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtCreateLibCtxWindowsDefault (
   VtLibCtx *libCtx,
   unsigned char *policyURL
);

/*=========================================================*/
/*                                                         */
/* Toolkit Version                                         */
/*                                                         */
/*=========================================================*/

/* Some version functions and declarations are in vibecrypto.h.
 */

/* These are the VtLibraryVersions supported by vibe.lib.
 */

/** Use this VtLibraryVersion to determine the version of "vibe".
 */
VtLibraryVersion VtLibraryVersionVibe;

/** Use this VtLibraryVersion to determine the version of "vibeproviders".
 */
VtLibraryVersion VtLibraryVersionProviders;

/*=========================================================*/
/*                                                         */
/* Time                                                    */
/*                                                         */
/*=========================================================*/

/**
 * \defgroup TimeGroup Time
 */

/*@{*/

/** Various Functions rely on time. The following struct is how time is
 * passed in and out of the toolkit.
 * <code>
 * <pre>
 *   year: 4 decimal digits, e.g. 2003, 2004.
 *  month: 1 or 2 decimal digits, e.g. 1 (January), 11 (November).
 *    day: 1 or 2 decimal digits.
 *   hour: 24-hour clock, e.g. 0 (12:00 Midnight), 7 (7:00 AM), 14 (2:00 PM).
 * minute: 1 or 2 decimal digits, 0 to 59.
 * second: 1 or 2 decimal digits, 0 to 59.
 * </pre>
 * </code>
 * <p>Declare a variable to be of type VtTime, set the fields to
 * the desired time.
 */
typedef struct
{
  /** year: 4 decimal digits, e.g. 2003, 2004.
   */
  int year;
  /** month: 1 or 2 decimal digits, e.g. 1 (January), 11 (November).
   */
  int month;
  /** day: 1 or 2 decimal digits.
   */
  int day;
  /** hour: 24-hour clock, e.g. 0 (12:00 Midnight), 7 (7:00 AM), 14 (2:00 PM).
   */
  int hour;
  /** minute: 1 or 2 decimal digits, 0 to 59.
   */
  int minute;
  /** second: 1 or 2 decimal digits, 0 to 59.
   */
  int second;
} VtTime;

/** Utility function, get the time in a VtTime type. This function
 * will return the GMTime.
 * <p>Declare a variable to be of type VtTime, pass the address of
 * that variable to this function. The routine will fill in the fields
 * with the time.
 *
 * @param libCtx The library context.
 * @param theTime The address of the struct the function will fill with
 * the time.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtGetTime (
   VtLibCtx libCtx,
   VtTime *theTime
);

/*@}*/

/*=========================================================*/
/*                                                         */
/* File Context                                            */
/*                                                         */
/*=========================================================*/

/**
 * \defgroup FileCtxGroup File Context
 */

/*@{*/

/** The file context.
 * <p>Note that the ctx is a pointer type.
 */
typedef struct VtFileCtxDef *VtFileCtx;

/** The function VtCreateFileCtx builds a file context using a
 * VtFileImpl. This typedef defines what a VtFileImpl is. Although it
 * is a function pointer, an application should never call a VtFileImpl
 * directly, only pass it as an argument to VtCreateFileCtx.
 */
typedef int VT_CALLING_CONV (VtFileImpl) (
   VtFileCtx *, Pointer, unsigned int);

/** The functions VtSetFileParam and VtGetFileParam add or get
 * information to or from a file ctx. The information to add or get is
 * defined by a VtFileParam. This typedef defines what a VtFileParam
 * is. Although a VtFileParam is a function pointer, an application
 * should never call one directly, only pass it as an argument to
 * VtSetFileParam or VtGetFileParam.
 */
typedef int VT_CALLING_CONV (VtFileParam) (
   VtFileCtx, Pointer, unsigned int);

/** Create a new file context. This allocates space for an "empty"
 * context, then loads the given FileImpl to make it an "active"
 * context.
 * <p>The VtFileImpl defines the file implementation. The include
 * file vibe.h defines the supported FileImpls. Look through the
 * include file to see which FileImpl to use for your application.
 * All supported FileImpls will be defined as in the following
 * example.
 * <code>
 * <pre>
 *   extern VtFileImpl VtFileImplWin32;
 * </pre>
 * </code>
 * <p>Associated with each FileImpl is specific info. The
 * documentation for each FileImpl will describe the associated info
 * it needs. That data could be another object, it could be data in a
 * particular struct, it might be a NULL pointer. Check the
 * documentation for each FileImpl for a description of the data and
 * its required format.
 * <p>To use this function decide which FileImpl you want to use,
 * then determine what information that FileImpl needs and in which
 * format it is presented. Collect the data in the appropriate format
 * then call this function passing in the desired FileImpl and the
 * required info. The associated info must be cast to Pointer.
 * <p>For example, if the required info is a VtItem, declare a variable
 * to be of type VtItem, fill in the fields, then pass the address of
 * that VtItem cast to Pointer.
 * <p>The input fileCtx is a pointer to a context. It should point to
 * a NULL VtFileCtx. This function will go to the address given and
 * deposit a created context.
 * <p>If you call Create, you must call Destroy after you are done with
 * the context but before the program exits.
 * <p>Example:
 * <code>
 * <pre>
 *    int status;
 *    VtFileCtx fileCtx = (VtFileCtx)0; 
 *
 *    do {
 *          . . . 
 *      status = VtCreateFileCtx (
 *        libCtx, VtFileImplWin32, (Pointer)0, &fileCtx);
 *      if (status != 0)
 *        break;
 *          . . .
 *    } while (0);
 * 
 *    VtDestroyFileCtx (&fileCtx);
 * </pre>
 * </code>
 * @param libCtx The library context.
 * @param fileImpl The implementation the context will use.
 * @param associatedInfo The info needed by the FileImpl.
 * @param fileCtx A pointer to where the routine will deposit the
 * created context.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtCreateFileCtx (
   VtLibCtx libCtx,
   VtFileImpl fileImpl,
   Pointer associatedInfo,
   VtFileCtx *fileCtx
);

/* These are the VtFileImpls supported by the toolkit. Each
 * FileImpl is used in conjunction with special info for the function.
 * If there is no special info, the accompaniment is a NULL pointer.
 */

/** This VtFileImpl is used to build a file context using Windows file
 * operations (CreateFile, GetFileSize, etc.). It does not suuport any 
 * asynchronous operations.
 * <p>The data associated with VtFileImplWin32 is a NULL pointer. 
 */
extern VtFileImpl VtFileImplWin32;

/** This VtFileImpl is used to build a file context on embedded devics
 * using Windows CE as operating system. It works exactly the same as
 * VtFileImplWin32 except for temporary files. If you don't need to use
 * temporary files you can use VtFileImpl32 instead. But if you need to
 * work with temporary files, this impl must be used.
 * <p>The data associated with VtFileImplWinCE is a NULL pointer. 
 */
extern VtFileImpl VtFileImplWinCE;

/** This VtFileImpl is used to build a file context with the POSIX file
 * functions on most platforms. It uses standard functions (fopen,
 * fwrite, etc.) for performing file related tasks.
 * <p>The data associated with VtFileImplPOSIX is a NULL pointer. 
 */
extern VtFileImpl VtFileImplPOSIX;

/** This VtFileImpl is used to build a file context that does file
 * locking around accesses to a file. It wraps another VtFileImpl (e.g.
 * VtFileImplWin32 or VtFileImplPOSIX) that does the actual file
 * access. You must pass in a valid pointer to a VtFileImplInfoLocking
 * struct (described below) as the info when creating the file context.
 * This specifies (among other things) the base file implementation to
 * use.
 */
extern VtFileImpl VtFileImplLocking;

/* These are the valid values for the flags field of the
 * VtFileImplInfoLocking struct that is used to initialize a
 * file context using the VtFileImplLocking impl.
 */

/** Use this value in the flags field of the VtFileImplInfoLocking
 * if cross-process file locking is desired from the locking file context.
 * This would be appropriate if there will be multiple programs that
 * use the toolkit running at the same time. If this flag is not set, then
 * the file locks will only synchronize access to the files from different
 * threads within the current process. Shared file locking is the most
 * conservative (i.e. it will always work correctly), but the overhead
 * is higher than for in-process locking.
 */
#define VT_FILE_IMPL_LOCKING_SHARED  1

/** VT_TIMEOUT_INFINITE can be used as the value for the timeout field
 * of VtFileImplInfoLocking. If this value is used for the timeout,
 * then the operation will not timeout.
 */
#define VT_TIMEOUT_INFINITE    0xFFFFFFFF

/** A VtFileImplInfoLocking is passed to initialize a file context with
 * the VtFileImplLocking implementation.
 * <p>The baseImpl field should be another file context impl (e.g.
 * VtFileImplDefault, VtFileImplWin32) that will be used for the actual
 * file access.
 * <p>The flags field supports the VT_FILE_IMPL_LOCKING_SHARED flags which
 * is used to indicate cross-process file locking is required.
 * <p>The timeout field specifies how long the file context should
 * wait to acquire the file lock before timing out. The time is specified
 * in milliseconds.
 */
typedef struct
{
  VtFileImpl     *baseImpl;
  Pointer         baseImplData;
  unsigned int    flags;
  unsigned long   timeout;
} VtFileImplInfoLocking;


/** Destroy the file context.  This frees up any memory allocated
 * during the context's creation and use.
 * <p>If you call Create, you must call Destroy after you are done with
 * the context but before the program exits.
 * <p>Example:
 * <code>
 * <pre>
 *    int status;
 *    VtFileCtx fileCtx = (VtFileCtx)0; 
 *
 *    do {
 *          . . . 
 *      status = VtCreateFileCtx (
 *        libCtx, VtFileImplWin32, (Pointer)0,
 *        &fileCtx);
 *      if (status != 0)
 *        break;
 *          . . .
 *    } while (0);
 * 
 *    VtDestroyFileCtx (&fileCtx);
 * </pre>
 * </code>
 * @param fileCtx A pointer to where the routine will find the context to
 * destroy.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtDestroyFileCtx (
   VtFileCtx *fileCtx
);

/** Set the file context with the information given.
 * <p>The VtFileParam defines what information the ctx will be set
 * with.
 * <p>The include file vibe.h defines the supported FileParams.
 * Look through the include file to see which FileParam to use for
 * your application. All supported FileParams will be defined as in
 * the following example.
 * <code>
 * <pre>
 *   extern VtFileParam VtFileInfo;
 * </pre>
 * </code>
 * <p>Associated with each FileParam is specific info. The
 * documentation for each FileParam will describe the associated info
 * it needs. That data could be another object, it could be data in a
 * particular struct, it might be a NULL pointer. Check the
 * documentation for each FileParam for a description of the data and
 * its required format.
 * <p>To use this function decide which FileParam you want to use,
 * then determine what information that FileParam needs and in which
 * format it is presented. Collect the data in the appropriate format
 * then call this function passing in the desired FileParam and the
 * required info. The associated info must be cast to Pointer.
 * <p>For example, if the required info is a char array (a
 * NULL-terminated string), declare a variable to be of type char *,
 * set it to point to the NULL-terminated string, then pass that char *
 * cast to Pointer.
 *
 * @param fileCtx The context to set.
 * @param fileParam What the ctx is being set to.
 * @param associatedInfo The info needed by the fileParam.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtSetFileParam (
   VtFileCtx fileCtx,
   VtFileParam fileParam,
   Pointer associatedInfo
);

/* There are currently no VtFileParams supported by the toolkit.
 */

/** This struct is used by functions or objects to specify a file
 * context along with a base path. It is the application's
 * responsibility to make sure the format used to specify a path (if
 * specified) is the format the chosen file context uses.
 */
typedef struct
{
  /** The File context to be used.
   */
  VtFileCtx fileCtx;

  /** Full path of the directory  where the files will be found or are
   * to be placed, or the full path of a specific file. Relative paths may
   * or may not work. Its recommended to use full paths.
   * The Impl, Param, or object that uses this struct will indicate in its
   * documentation whether path is a directory path of a file path.
   * <p>This is often NULL, meaning use a default path or file.
   */
  unsigned char *path;

} VtFileCtxUseInfo;

/*@}*/

/*=========================================================*/
/*                                                         */
/* Policy Context                                          */
/*                                                         */
/*=========================================================*/

/**
 * \defgroup PolicyCtxGroup Policy Context
 */

/*@{*/

/** The policy context.
 * <p>Note that the ctx is a pointer type.
 */
typedef struct VtPolicyCtxDef *VtPolicyCtx;

/** The function VtCreatePolicyCtx builds a policy context using a
 * VtPolicyImpl. This typedef defines what a VtPolicyImpl is. Although
 * it is a function pointer, an application should never call a
 * VtPolicyImpl directly, only pass it as an argument to
 * VtCreatePolicyCtx.
 */
typedef int VT_CALLING_CONV (VtPolicyImpl) (
   VtPolicyCtx *, Pointer, unsigned int);

/** The functions VtSetPolicyParam and VtGetPolicyParam add or get
 * information to or from a policy ctx. The information to add or get
 * is defined by a VtPolicyParam. This typedef defines what a
 * VtPolicyParam is. Although a VtPolicyParam is a function pointer, an
 * application should never call one directly, only pass it as an
 * argument to VtSetPolicyParam or VtGetPolicyParam.
 */
typedef int VT_CALLING_CONV (VtPolicyParam) (
   VtPolicyCtx, Pointer, unsigned int);

/** Create a new policy context. This allocates space for an "empty"
 * context, then loads the given PolicyImpl to make it an "active"
 * context.
 * <p>The VtPolicyImpl defines the policy implementation. The include
 * file vibe.h defines the supported PolicyImpls. Look through the
 * include file to see which PolicyImpl to use for your application.
 * All supported PolicyImpls will be defined as in the following
 * example.
 * <code>
 * <pre>
 *   extern VtPolicyImpl VtPolicyImplXmlURLWinINet;
 * </pre>
 * </code>
 * <p>Associated with each PolicyImpl is specific info. The
 * documentation for each PolicyImpl will describe the associated info
 * it needs. That data could be another object, it could be data in a
 * particular struct, it might be a NULL pointer. Check the
 * documentation for each PolicyImpl for a description of the data and
 * its required format.
 * <p>To use this function decide which PolicyImpl you want to use,
 * then determine what information that PolicyImpl needs and in which
 * format it is presented. Collect the data in the appropriate format
 * then call this function passing in the desired PolicyImpl and the
 * required info. The associated info must be cast to Pointer.
 * <p>For example, if the required info is a VtItem, declare a variable
 * to be of type VtItem, fill in the fields, then pass the address of
 * that VtItem cast to Pointer.
 * <p>The input policyCtx is a pointer to a context. It should point to
 * a NULL VtPolicyCtx. This function will go to the address given and
 * deposit a created context.
 * <p>If you call Create, you must call Destroy after you are done with
 * the context but before the program exits.
 * <p>Example:
 * <code>
 * <pre>
 *    int status;
 *    VtWinINetHttpTimeInfo policyInfo;
 *
 *    policyInfo.url =  (Pointer)
 *        "https://voltage-ps-0000.developer.voltage.com/clientPolicy.xml";
 *    policyInfo.timeout = 10000;
 *
 *    do {
 *          . . .
 *
 *      status = VtCreatePolicyCtx (
 *        libCtx, VtPolicyImplXmlURLWinINet, (Pointer)&policyInfo,
 *        &policyCtx);
 *      if (status != 0)
 *        break;
 *          . . .
 *    } while (0);
 *
 *    VtDestroyPolicyCtx (&policyCtx);
 * </pre>
 * </code>
 * @param libCtx The library context.
 * @param policyImpl The implementation the context will use.
 * @param associatedInfo The info needed by the PolicyImpl.
 * @param policyCtx A pointer to where the routine will deposit the
 * created context.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtCreatePolicyCtx (
   VtLibCtx libCtx,
   VtPolicyImpl policyImpl,
   Pointer associatedInfo,
   VtPolicyCtx *policyCtx
);

/* These are the VtPolicyImpls supported by the toolkit. Each
 * PolicyImpl is used in conjunction with special info for the function.
 * If there is no special info, the accompaniment is a NULL pointer.
 */

/** This VtPolicyImpl is used to build a policy context with the
 * implementation that reads policy info from an XML string in the
 * buffer supplied.
 * <p>This policy provider relies on a local definition of policy
 * information. This definition is encapsulated in an XML string.
 * <p>The data associated with VtPolicyImplXmlBuffer is a buffer
 * containing the XML string, which is a NULL-terminated UTF8 string.
 */
extern VtPolicyImpl VtPolicyImplXmlBuffer;

/** This VtPolicyImpl is used to build a policy context with the
 * implementation that reads policy info from an XML string stored in a
 * file.
 * <p>This policy provider relies on a local definition of policy
 * information. This definition is encapsulated in an XML string.
 * <p>The data associated with VtPolicyImplXmlFile is a pointer to a
 * VtFileCtxUseInfo struct giving the file context and the file name
 * (with full path) of the file containing the XML string, which is a
 * NULL-terminated UTF8 string.
 */
extern VtPolicyImpl VtPolicyImplXmlFile;

/** This VtPolicyImpl is used to build a policy context with the
 * implementation that reads policy info from an XML string stored at a
 * URL. This implementation will Use WinINet to make an HTTPS
 * connection to the policy location and download the data at the URL.
 * It expects the data to downlown will be an XML string.
 * <p>This Impl differs from VtPolicyImplXmlURLWinINet in that this
 * also includes a timeout value in the associated info.
 * <p>This policy provider relies on a definition of policy
 * information. This definition is encapsulated in an XML string.
 * <p>The data associated with VtPolicyImplXmlURLWinINet is a pointer
 * to VtWinINetHttpTimeInfo that specifies a URL and a time out value.
 * The URL is a UTF-8 string and NULL-terminated.
 * <p>The URL is likely to be something that looks like this.
 * <code>
 * <pre>
 *   "https://voltage-ps-0000.developer.voltage.com/policy.xml"
 * </pre>
 * </code>
 */
extern VtPolicyImpl VtPolicyImplXmlURLWinINetTime;

/** This VtPolicyImpl is used to build a policy context with the
 * implementation that reads policy info from an XML string stored at a
 * URL. This implementation will Use WinINet to make an HTTPS
 * connection to the policy location and download the data at the URL.
 * It expects the data to downlown will be an XML string.
 * <p>This policy provider relies on a definition of policy
 * information. This definition is encapsulated in an XML string.
 * <p>This Impl differs from VtPolicyImplXmlURLWinINetTime in that this
 * Impl uses a default timeout value (currently the default value is
 * 10000 milliseconds = 10 seconds).
 * <p>The data associated with VtPolicyImplXmlURLWinINet is a pointer
 * to an unsigned char array that is the URL. It is a UTF-8 string and
 * NULL-terminated.
 * <p>The URL is likely to be something that looks like this.
 * <code>
 * <pre>
 *   "https://voltage-ps-0000.developer.voltage.com/policy.xml"
 * </pre>
 * </code>
 */
extern VtPolicyImpl VtPolicyImplXmlURLWinINet;

/** This VtPolicyImpl is used to build a policy context with the
 * implementation that reads policy info from an XML string stored at a
 * URL. This implementation will use Curl to make an HTTPS
 * connection to the policy location and download the data at the URL.
 * It expects the data to downlown will be an XML string.
 * <p>This policy provider relies on a definition of policy
 * information. This definition is encapsulated in an XML string.
 * <p>This Impl differs from VtPolicyImplXmlURLCurl in that this also
 * includes a timeout value in the associated info.
 * <p>The data associated with VtPolicyImplXmlURLCurl is a pointer
 * to a VtCurlHttpTimeInfo struct that gives the URL, a trustStore (see
 * Curl documentation for info on trust stores) and a timeout value.
 * The URL is a UTF-8 string and NULL-terminated.
 * <p>The URL is likely to be something that looks like this.
 * <code>
 * <pre>
 *   "voltage-ps-0000.developer.voltage.com"
 * </pre>
 * </code>
 */
extern VtPolicyImpl VtPolicyImplXmlURLCurlTime;

/** This VtPolicyImpl is used to build a policy context with the
 * implementation that reads policy info from an XML string stored at a
 * URL. This implementation will use Curl to make an HTTPS
 * connection to the policy location and download the data at the URL.
 * It expects the data to downlown will be an XML string.
 * <p>This policy provider relies on a definition of policy
 * information. This definition is encapsulated in an XML string.
 * <p>This Impl differs from VtPolicyImplXmlURLCurlTime in that this
 * Impl uses a default timeout value (currently the default value is
 * 10000 milliseconds = 10 seconds).
 * <p>The data associated with VtPolicyImplXmlURLCurl is a pointer
 * to a VtCurlHttpInfo struct that gives the URL and a trustStore (see
 * Curl documentation for info on trust stores). The URL is a UTF-8
 * string and NULL-terminated.
 * <p>The URL is likely to be something that looks like this.
 * <code>
 * <pre>
 *   "voltage-ps-0000.developer.voltage.com"
 * </pre>
 * </code>
 */
extern VtPolicyImpl VtPolicyImplXmlURLCurl;

/** This data struct is the associated info for
 * VtPolicyImplXmlURLWinINetTime. It defines the info needed to build a
 * policy based on info downloaded from a URL.
 * <p>The timeout value is the connection time out, meaning that it
 * specifies the time to wait for a connection to be established with a
 * given host. Once the connection is established this timeout value has
 * no effect. It is given in milliseconds.
 */
typedef struct
{
  unsigned char *url;
  unsigned long timeout;
} VtWinINetHttpTimeInfo;

/** This data struct is the associated info for
 * VtPolicyImplXmlURLCurlTime. It defines the info needed to build a
 * policy based on info downloaded from a URL.
 * <p>The timeout value is the connection time out, meaning that it
 * specifies the time to wait for a connection to be established with a
 * given host. Once the connection is established this timeout value has
 * no effect. It is given in milliseconds.
 */
typedef struct
{
  unsigned char *url;
  unsigned char *trustStore;
  unsigned long timeout;
} VtCurlHttpTimeInfo;

/** This data struct is the associated info for VtPolicyImplXmlURLCurl.
 * It defines the info needed to build a policy based on info downloaded
 * from a URL.
 */
typedef struct
{
  unsigned char *url;
  unsigned char *trustStore;
} VtCurlHttpInfo;

/** Destroy the policy context.  This frees up any memory allocated
 * during the context's creation and use.
 * <p>If you call Create, you must call Destroy after you are done with
 * the context but before the program exits.
 * <p>Example:
 * <code>
 * <pre>
 *    int status;
 *    VtWinINetHttpTimeInfo policyInfo;   
 *
 *    policyInfo.url =  (Pointer)
 *        "https://voltage-ps-0000.developer.voltage.com/clientPolicy.xml";
 *    policyInfo.timeout = 10000;
 *
 *    do {
 *          . . .
 *
 *      status = VtCreatePolicyCtx (
 *        libCtx, VtPolicyImplXmlURLWinINet, (Pointer)&policyInfo,
 *        &policyCtx);
 *      if (status != 0)
 *        break;
 *          . . .
 *    } while (0);
 *
 *    VtDestroyPolicyCtx (&policyCtx);
 * </pre>
 * </code>
 * @param policyCtx A pointer to where the routine will find the context to
 * destroy.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtDestroyPolicyCtx (
   VtPolicyCtx *policyCtx
);

/** Set the policy context with the information given.
 * <p>The VtPolicyParam defines what information the ctx will be set
 * with.
 * <p>The include file vibe.h defines the supported PolicyParams.
 * Look through the include file to see which PolicyParam to use for
 * your application. All supported PolicyParams will be defined as in
 * the following example.
 * <code>
 * <pre>
 *   extern VtPolicyParam VtPolicyInfo;
 * </pre>
 * </code>
 * <p>Associated with each PolicyParam is specific info. The
 * documentation for each PolicyParam will describe the associated info
 * it needs. That data could be another object, it could be data in a
 * particular struct, it might be a NULL pointer. Check the
 * documentation for each PolicyParam for a description of the data and
 * its required format.
 * <p>To use this function decide which PolicyParam you want to use,
 * then determine what information that PolicyParam needs and in which
 * format it is presented. Collect the data in the appropriate format
 * then call this function passing in the desired PolicyParam and the
 * required info. The associated info must be cast to Pointer.
 * <p>For example, if the required info is a char array (a
 * NULL-terminated string), declare a variable to be of type char *,
 * set it to point to the NULL-terminated string, then pass that char *
 * cast to Pointer.
 *
 * @param policyCtx The context to set.
 * @param policyParam What the ctx is being set to.
 * @param associatedInfo The info needed by the PolicyParam.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtSetPolicyParam (
   VtPolicyCtx policyCtx,
   VtPolicyParam policyParam,
   Pointer associatedInfo
);

/** Get the specified type of information out of a policy context.
 * <p>This function will go to the address given by getInfo and
 * deposit a pointer. That pointer is the address of the data. The
 * memory to hold the information belongs to the object, do not free it.
 * <p>The VtPolicyParam will specify what kind of information will be
 * returned, the getInfo is where the function will deposit the info.
 * <p>The include file vibe.h defines the supported PolicyParams.
 * Look through the include file to see which PolicyParam to use for
 * your application.
 * <p>See also VtSetPolicyParam.
 * <p>Most (if not all) PolicyParams will be usable only after loading
 * the policy into the context. These Params simply return each
 * individual policy element that was part of the entire policy.
 * <p>To use this function decide which PolicyParam you want to use,
 * then determine what information that PolicyParam will return and
 * in which format it is presented. Declare a variable to be a pointer
 * to the appropriate type, then call this function passing in the
 * desired PolicyParam and the address of the variable. Cast the
 * address of the pointer to the appropriate type: (Pointer *).
 * <p>If the associated info is already a pointer type (such as an
 * object), declare a variable to be of the appropriate type and pass
 * the address of that variable cast to (Pointer *).
 * <p>For example, if the returned info is a VtString, declare a
 * variable to be of type (VtString *), pass the address of that
 * variable (&varName) cast to (Pointer *).
 * <p>Example:
 * <pre>
 * <code>
 *    unsigned int *rate;
 *
 *    do {
 *          . . .
 *      status = VtGetPolicyParam (
 *        polciyCtx, VtPolicyParamRefreshRatePositive, (Pointer *)&rate);
 *      if (status != 0)
 *        break;
 *          . . .
 *    } while (0);
 * </code>
 * </pre>
 *
 * @param policyCtx The context to query.
 * @param policyParam Defines what info to get.
 * @param getInfo The address of a pointer. The function will go to
 * that address and deposit a pointer to the info requested.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtGetPolicyParam (
   VtPolicyCtx policyCtx,
   VtPolicyParam policyParam,
   Pointer *getInfo
);

/* These are the VtPolicyParams supported by the toolkit. Each
 * VtPolicyParam is used in conjunction with special info for the
 * function. If there is no special info, the accompaniment is a NULL
 * pointer.
 */

/** GetParam only.
 * <p>This VtPolicyParam returns the refresh rate positive in the
 * policy context.
 * <p>The refresh rate positive is the length of time a value can be
 * stored before refreshing it. For instance, if you download
 * parameters from a district and store them, the next time you need
 * those parameters, you don't need to download them, just get them
 * from storage. However, it is possible an app wants to periodically
 * check in with the district to see if something has changed. The
 * policy says that an app should refresh the contents of storage at
 * the given rate. This param is "positive" in that this is the rate at
 * which values that have been found should be refreshed. See also
 * RefreshRateNegative.
 * <p>The associated info is a pointer to an unsigned int.
 * <p>When getting, pass in the address of an unsigned int pointer, the
 * Get function will deposit a pointer to an unsigned int there. The
 * unsigned int belongs to the policy context out of which the getInfo
 * came.
 * <p>The value returned is number of seconds. For example, a refresh
 * rate positive of one week will be 0x00093A80 = 604,800.
 */
extern VtPolicyParam VtPolicyParamRefreshRatePositive;

/** GetParam only.
 * <p>This VtPolicyParam returns the refresh rate negative in the
 * policy context.
 * <p>The refresh rate negative is the length of time a negative value
 * can be stored before refreshing it. For instance, if you try to
 * download parameters from a purported district and find there is no
 * district, you can store that information (that there is no district
 * of a particular name). The next time you want to know if there is a
 * district of a particular name, you can recover that info from
 * storage and avoid making an unsuccessful network connection.
 * However, it is possible an app wants to periodically check in to see
 * if a district has been created for that name. Or maybe an app wants
 * to check again in case the first call was during a time period when
 * the key server was down. The policy says that an app should refresh
 * the contents of storage at the given rate. This param is "negative"
 * in that this is the rate at which information stored indicating that
 * values do not exist should be refreshed. See also
 * RefreshRatePositive.
 * <p>The associated info is a pointer to an unsigned int.
 * <p>When getting, pass in the address of an unsigned int pointer, the
 * Get function will deposit a pointer to an unsigned int there. The
 * unsigned int belongs to the policy context out of which the getInfo
 * came.
 * <p>The value returned is number of seconds. For example, a refresh
 * rate positive of one week will be 0x00093A80 = 604,800.
 */
extern VtPolicyParam VtPolicyParamRefreshRateNegative;

/*@}*/

/*=========================================================*/
/*                                                         */
/* Transport Context                                       */
/*                                                         */
/*=========================================================*/

/**
 * \defgroup TransportCtxGroup Transport Context
 */

/*@{*/

/** The transport context.
 * <p>Note that the ctx is a pointer type.
 */
typedef struct VtTransportCtxDef *VtTransportCtx;

/** The function VtCreateTransportCtx builds a transport ctx using a
 * VtTransportImpl. This typedef defines what a VtTransportImpl is.
 * Although it is a function pointer, an application should never call
 * a VtTransportImpl directly, only pass it as an argument to
 * VtCreateTransportCtx.
 */
typedef int VT_CALLING_CONV (VtTransportImpl) (
   VtTransportCtx *, Pointer, unsigned int);

/** The functions VtSetTransportParam and VtGetTransportParam add or
 * get information to or from a transport ctx. The information to add
 * or get is defined by a VtTransportParam. This typedef defines what a
 * VtTransportParam is. Although a VtTransportParam is a function
 * pointer, an application should never call one directly, only pass it
 * as an argument to VtSetTransportParam or VtGetTransportParam.
 */
typedef int VT_CALLING_CONV (VtTransportParam) (
   VtTransportCtx, Pointer, unsigned int);

/* Previous download request:
 * <p>It can happen that a process or thread makes a request for a key
 * and cert by calling VtObtainPrivateKeysAndCert, then another process
 * or thread makes a request for a key for the same identity. Although
 * rare, it is possible and a transport provider must be prepared to
 * handle this situation.
 * <p>In the current release of the toolkit, the transport providers
 * will check to see if they can find information indicating that another
 * request for the key has been made. If so, it will return
 * VT_ERROR_DOWNLOAD_PREVIOUS. The provider will not attempt to
 * download the key, it will not make any network connections. Upon
 * receipt of this error, the app must decide whether to cancel the
 * current request and let the previous request take care of the
 * download, or continue with the current request.
 * <p>To "cancel" the current request, simply don't call
 * ObtainKeyAndCert again. To override, call it again.
 * <p>Why override? It's possible that the earlier request was started,
 * but never completed, possibly because the operation was interrupted
 * or cancelled. The first request will never complete, so it makes
 * sense to go ahead with the second request.
 * <p>Then why not override? If the first request was not cancelled or
 * interrupted, it will complete. Possibly it is in the middle of an
 * email answerback authentication which can take a long time. If the
 * second request overrides the PREVIOUS error, then it can obtain a
 * cert for a signing key that does not match the DSA signing key from
 * the first request. It is possible that two processes or threads will
 * store keys and certs obtained, and if the signing keys are
 * different, it is possible mismatched private key and cert will be
 * stored.
 * <p>Once again, for most apps, this is extremely unlikely to happen,
 * but apps should be aware of the possibility.
 * <p>It is possible to set a flag when building the transport ctx to
 * indicate that if encountering a PREVIOUS, the provider should not
 * return PREVIOUS to the app, but should just continue.
 * <p>For more on PREVIOUS and ASYNC, see the User's Manual.
 */

/* Asynchronous download:
 * <p>Transport functions often require network communications. While
 * many network operations can be automatic, some are asynchronous.
 * That is, the transport function might launch a new process to
 * perform authentication and return to the app. Although the toolkit
 * returns to the app, the download has not been completed. The app
 * must wait until the launched process completes before continuing.
 * <p>There are three possible reactions to asynchronous situations.
 * One, the transport provider can simply do the operations necessary
 * (call this R1 for reaction 1). Two, it can return and expect the
 * application to perform the operations (call this R2). Or three, it
 * can return to let the app know there is an async issue, then require
 * the app to make the call again so that the provider can finish the
 * task (call this R3).
 * <p>A transport provider might not necessarily offer all three
 * options.
 * <p>If the app and provider "agree" to R1, the provider will
 * immediately perform the async operation. However, the provider may
 * launch this operation only and not wait for it to finish. In this
 * case, the function will return to the application with the error
 * VT_ERROR_DOWNLOAD_PENDING. This means the async operations are
 * running but have not finished yet. The current version of the
 * toolkit has no mechanism for notifying the application when the
 * async operation has completed.
 * <p>If the app and provider "agree" to R2 or R3, the transport
 * provider needs a way to indicate that an async situation is at hand.
 * It will do this by returning the error code VT_ERROR_ASYNC_DOWNLOAD.
 * This error will eventually be returned by the original toolkit call
 * (such as VtObtainPrivateKeysAndCert). This error code indicates that
 * the function did not complete, however, it can still perform its
 * task with more work. The function has suspended operations and this
 * return code is the way to tell the application of the situation.
 * <p>If R2, the transport provider must define a way to indicate what
 * the app needs to do and give it the necessary information. Any extra
 * functions to execute R2 will be defined by the transport provider
 * and are outside the scope of the toolkit.
 * <p>For R3, to complete the operation, the app must call the original
 * toolkit function again. Some apps might call the original function
 * immediately, others may want to spawn a new thread, one thread
 * calling the original toolkit function, the other thread performing
 * some other operation. As with R1, though, the provider may launch
 * this operation only and not wait for it to finish. In this case, the
 * function will return to the application with the error
 * VT_ERROR_DOWNLOAD_PENDING.
 * <p>There are, then, 5 possible returns from a download function:
 * <pre>
 * <code>
 *
 *    0 - The operation completed successfully
 *    PREVIOUS - The transport provider found a previous request for
 *    the same identity, to continue anyway, call again
 *    ASYNC - The transport provider needs to perform an async
 *    operation, to launch the async operation, call again
 *    PENDING - The transport provider launched the async operation but
 *    it has not completed yet, resolution procedures to be defined
 *    VT_ERROR - Some other error happened (network connection failure,
 *    authorization failure, etc.), the data cannot be downloaded
 *
 * </code>
 * </pre>
 * <p>Current resolution technique: If the return from ObtainKeyAndCert
 * is ASYNC, call again. If the return is PENDING, wait until the async
 * operation completes and call again. Currently there is no
 * programmatic way to know when it completes (we hope to have that
 * feature in the future), but an app can generally have some way to
 * determine with user input, such as an OK button. The return from the
 * second call will be 0, the async operation completed successfully and
 * the material has been loaded into the appropriate objects.
 * Alternatively, if PENDING, "poll" by calling the function again and
 * keep calling the function while the return is PENDING, until
 * obtaining another return. If for some future call, the return is 0,
 * the download is complete, if some other error, the download failed.
 * <p>For more on PREVIOUS and ASYNC, see the User's Manual.
 */

/** Create a new transport context. This allocates space for an "empty"
 * context, then loads the given TransportImpl to make it an "active"
 * context.
 * <p>The VtTransportImpl defines the transport implementation. The
 * include file vibe.h defines the supported TransportImpls. Look
 * through the include file to see which TransportImpl to use for your
 * application. All supported TransportImpls will be defined as in the
 * following example.
 * <code>
 * <pre>
 *   extern VtTransportImpl VtTransportImplHttpsWinINet;
 * </pre>
 * </code>
 * <p>Associated with each TransportImpl is specific info. The
 * documentation for each TransportImpl will describe the associated
 * info it needs. That data could be another object, it could be data
 * in a particular struct, it might be a NULL pointer. Check the
 * documentation for each TransportImpl for a description of the data
 * and its required format.
 * <p>To use this function decide which TransportImpl you want to use,
 * then determine what information that TransportImpl needs and in
 * which format it is presented. Collect the data in the appropriate
 * format then call this function passing in the desired TransportImpl
 * and the required info. The associated info must be cast to Pointer.
 * <p>For example, if the required info is a VtItem, declare a variable
 * to be of type VtItem, fill in the fields, then pass the address of
 * that VtItem cast to Pointer.
 * <p>The input transportCtx is a pointer to a context. It should point
 * to a NULL VtTransportCtx. This function will go to the address given
 * and deposit a created context.
 * <p>If you call Create, you must call Destroy after you are done with
 * the context but before the program exits.
 * <p>Example:
 * <code>
 * <pre>
 *    int status;
 *    VtTransportCtx transportCtx = (VtTransportCtx)0;
 *    VtWinINetTransportInfo transInfo;
 *
 *    do {
 *          . . .
 *      transInfo.uiHandle = (Pointer)0;
 *      transInfo.asyncFlag = VT_ASYNC_RESPONSE_ALERT;
 *      transInfo.fileCtx = (VtFileCtx)0;
 *      status = VtCreateTransportCtx (
 *        libCtx, VtTransportImplHttpsWinINet, (Pointer)&transInfo,
 *        &transportCtx);
 *      if (status != 0)
 *        break;
 *          . . .
 *    } while (0);
 *
 *    VtDestroyTransportCtx (&transportCtx);
 * </pre>
 * </code>
 * @param libCtx The library context.
 * @param transportImpl The implementation the context will use.
 * @param associatedInfo The info needed by the TransportImpl.
 * @param transportCtx A pointer to where the routine will deposit the
 * created context.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtCreateTransportCtx (
   VtLibCtx libCtx,
   VtTransportImpl transportImpl,
   Pointer associatedInfo,
   VtTransportCtx *transportCtx
);

/* These are the TransportImpls supported by the toolkit. Each
 * TransportImpl is used in conjunction with special info for the
 * function. If there is no special info, the accompaniment is a NULL
 * pointer.
 */

/** This VtTransportImpl is used to set a transport context with an
 * implementation that will contact districts using HTTPS with WinINet
 * performing the HTTPS functions.
 * <p>The data associated with VtTransportImplHttpsWinINet is a pointer
 * to a VtWinINetTransportInfo struct.
 * <p>When downloading IBE Params, this transport provider will never
 * return ASYNC or PENDING.
 * <p>When downoading an IBE private key, this transport provider can
 * return ASYNC or PENDING.
 * <p>Note that you can reset the timeout value by calling
 * VtSetTransportParam using VtTransportParamTimeout.
 */
extern VtTransportImpl VtTransportImplHttpsWinINet;

/** This VtTransportImpl is used to set a transport context with an
 * implementation that will contact districts using HTTPS with WinHttp
 * performing the HTTPS functions.
 * <p>The data associated with VtTransportImplHttpsWinHttp is a pointer
 * to a VtWinHttpTransportInfo struct.
 * <p>When downloading IBE Params, this transport provider will never
 * return ASYNC or PENDING.
 * <p>When downoading an IBE private key, this transport provider can
 * return ASYNC or PENDING.
 * <p>Note that you can reset the timeout value by calling
 * VtSetTransportParam using VtTransportParamTimeout.
 */
extern VtTransportImpl VtTransportImplHttpsWinHttp;

/** This VtTransportImpl is used to set a transport context with an
 * implementation that will contact districts using HTTPS with Curl
 * performing the HTTPS functions.
 * <p>The data associated with VtTransportImplHttpsCurl is a pointer
 * to a VtCurlTransportInfo struct.
 * <p>When downloading IBE Params, this transport provider will never
 * return ASYNC or PENDING.
 * <p>When downoading an IBE private key, this transport provider
 * should not return ASYNC or PENDING.
 * <p>Note that you can reset the timeout value by calling
 * VtSetTransportParam using VtTransportParamTimeout.
 */
extern VtTransportImpl VtTransportImplHttpsCurl;

/** This VtTransportImpl is used to set a transport context with the
 * token based transport technique. This technique uses HTTPS for
 * communication using WinINet and uses tokens based on a shared secret.
 * The secret is shared between the server and the client makes the
 * request (e.g. request to get the private key). The client will create
 * a token from the given secret and send it along with the request.
 * When the server sees the token it can determine whether the sent token
 * is correct, the server knows that the request is coming from a trusted
 * client and will issue the key. This kind of mechanism is useful for
 * processes that need to request keys without performing any interactive
 * authentication work (ex. a cron job running overnight).
 * <p>The data associated with TransportImplDelegatedWinINet is a NULL
 * pointer.
 * <p>Note that you can reset the timeout value by calling
 * VtSetTransportParam using VtTransportParamTimeout.
 * <p>When downloading IBE Params or keys, this transport provider
 * should not return ASYNC or PENDING.
 */
extern VtTransportImpl VtTransportImplDelegatedWinINet;

/** This VtTransportImpl is used to set a transport context with the
 * token based transport technique. This technique uses HTTPS for
 * communication using WinHttp and uses tokens based on a shared secret.
 * The secret is shared between the server when the client makes a
 * request for a cert or IBE private key). The client will create
 * a token from the given secret and send it along with the request.
 * When the server sees the token it can determine whether the sent token
 * is correct, and if the server knows that the request is coming from
 * a trusted client, it will issue the key. This kind of mechanism is
 * useful for processes that need to request keys without performing
 * any interactive authentication work (ex. a cron job running overnight).
 * <p>The data associated with TransportImplDelegatedWinHttp is a NULL
 * pointer.
 * <p>Note that you can reset the timeout value by calling
 * VtSetTransportParam using VtTransportParamTimeout.
 * <p>When downloading IBE Params or keys, this transport provider
 * should not return ASYNC or PENDING.
 */
extern VtTransportImpl VtTransportImplDelegatedWinHttp;

/** This VtTransportImpl is used to set a transport context with the
 * token based transport technique. This technique uses HTTPS for
 * communication using Curl and uses tokens based on a shared secret.
 * The secret is shared between the server and the client makes the
 * request (e.g. request to get the private key). The client will create
 * a token from the given secret and send it along with the request.
 * When the server sees the token it can determine whether the sent token
 * is correct, the server knows that the request is coming from a trusted
 * client and will issue the key. This kind of mechanism is useful for
 * processes that need to request keys without performing any interactive
 * authentication work (ex. a cron job running overnight).
 * <p>The data associated with VtTransportImplDelegatedCurl is a
 * pointer to a buffer containing the trustStore name. The trustStore
 * is used by Curl, it is the path of the directory where the
 * certificates of trusted certificate authorities (CAs) are stored.
 * This directory should contain certificates of the trusted CAs in
 * PEM format and should be hashed using OpenSSL's c_rehash utility.
 * It is a UTF-8 NULL-terminated string.
 * <p>If you pass NULL associated info, Curl should use a default trust
 * store (see Curl documentation), however, there is no guarantee this
 * will work.
 * <p>When downloading IBE Params or keys, this transport provider
 * should not return ASYNC or PENDING.
 * <p>Note that you can reset the timeout value by calling
 * VtSetTransportParam using VtTransportParamTimeout.
 */
extern VtTransportImpl VtTransportImplDelegatedCurl;

/** This is the data struct to use when setting a transport provider to
 * the VtTransportImplHttpsWinINet.
 * <p>The uiHandle is HWND cast to a Pointer (it's a Pointer so the
 * vibe.h include file does not need to know about the HWND type).
 * <p>If supplied, the uiHandle might be used for downloading private
 * keys, but not paramaters.
 * <p>The asyncFlag tells the transport function what to do if
 * encountering a PREVIOUS or ASYNC situation. Continue without
 * informing the app (R1 from above) or return the ASYNC_DOWNLOAD or
 * PREVIOUS error and require the app to call the toolkit function
 * again (R3 from above). Note that this provider does not offer the
 * option of having the app do the extra work (R2 from above).
 * <p>The asyncFlag is a bit field. Set it to the OR of the following
 * flags.
 * <code>
 * <pre>
 *   VT_PREVIOUS_RESPONSE_CONTINUE
 *     or
 *   VT_PREVIOUS_RESPONSE_ALERT
 *
 *   VT_ASYNC_RESPONSE_CONTINUE
 *     or
 *   VT_ASYNC_RESPONSE_ALERT
 * </pre>
 * </code>
 * <p>If the asyncFlag is 0, the transport will use default values
 * (probably ALERT).
 * <p>For an ALERT flag, the transport provider will not perform an
 * async operation until the function is called again. If CONTINUE, the
 * provider will simply continue when encountering the situation
 * corresponding to the set bit.
 * <p> The fileCtx field is no longer used in version 2.1, it can be
 * NULL.
 */
typedef struct
{
  Pointer            uiHandle;
  unsigned int       asyncFlag;
  VtFileCtx          fileCtx;
} VtWinINetTransportInfo;

/** This is the data struct to use when setting a transport provider to
 * the VtTransportImplHttpsWinHttp.
 * <p>The uiHandle is HWND cast to a Pointer (it's a Pointer so the
 * vibe.h include file does not need to know about the HWND type).
 * <p>If supplied, the uiHandle might be used for downloading private
 * keys, but not paramaters.
 * <p>The asyncFlag tells the transport function what to do if
 * encountering a PREVIOUS or ASYNC situation. Continue without
 * informing the app (R1 from above) or return the ASYNC_DOWNLOAD or
 * PREVIOUS error and require the app to call the toolkit function
 * again (R3 from above). Note that this provider does not offer the
 * option of having the app do the extra work (R2 from above).
 * <p>The asyncFlag is a bit field. Set it to the OR of the following
 * flags.
 * <code>
 * <pre>
 *   VT_PREVIOUS_RESPONSE_CONTINUE
 *     or
 *   VT_PREVIOUS_RESPONSE_ALERT
 *
 *   VT_ASYNC_RESPONSE_CONTINUE
 *     or
 *   VT_ASYNC_RESPONSE_ALERT
 * </pre>
 * </code>
 * <p>If the asyncFlag is 0, the transport will use default values
 * (probably ALERT).
 * <p>For an ALERT flag, the transport provider will not perform an
 * async operation until the function is called again. If CONTINUE, the
 * provider will simply continue when encountering the situation
 * corresponding to the set bit.
 * <p> The fileCtx field is no longer used in version 2.1, it can be
 * NULL.
 * <p>The timeout value is in milliseconds. If the value is 0xffffffff
 * (VT_TIMEOUT_INFINITE), the transport provider will consider the
 * timeout to be infinite. If the value is 0, the transport provider
 * will use a default timeout (probably 10 seconds).
 */
typedef struct
{
  Pointer            uiHandle;
  unsigned int       asyncFlag;
  VtFileCtx          fileCtx;
  unsigned int       timeout;
} VtWinHttpTransportInfo;

/** This is the data struct to use when setting a transport provider to
 * the VtTransportImplHttpsCurl.
 * <p>The uiHandle is HWND cast to a Pointer (it's a Pointer so the
 * vibe.h include file does not need to know about the HWND type).
 * <p>If supplied, the uiHandle might be used for downloading private
 * keys, but not paramaters.
 * <p>The trustStore is used by Curl, it is the path of the directory
 * where the certificates of trusted certificate authorities (CAs) are
 * stored. This directory should contain certificates of the trusted
 * CAs in PEM format and should be hashed using OpenSSL's c_rehash
 * utility. It is a UTF-8 NULL-terminated string.
 * <p>The asyncFlag tells the transport function what to do if
 * encountering a PREVIOUS or ASYNC situation. Continue without
 * informing the app (R1 from above) or return the ASYNC_DOWNLOAD or
 * PREVIOUS error and require the app to call the toolkit function
 * again (R3 from above). Note that this provider does not offer the
 * option of having the app do the extra work (R2 from above).
 * <p>The asyncFlag is a bit field. Set it to the OR of the following
 * flags.
 * <code>
 * <pre>
 *   VT_PREVIOUS_RESPONSE_CONTINUE
 *     or
 *   VT_PREVIOUS_RESPONSE_ALERT
 *
 *   VT_ASYNC_RESPONSE_CONTINUE
 *     or
 *   VT_ASYNC_RESPONSE_ALERT
 * </pre>
 * </code>
 * <p>If the asyncFlag is 0, the transport will use default values
 * (probably ALERT).
 * <p>For an ALERT flag, the transport provider will not perform an
 * async operation until the function is called again. If CONTINUE, the
 * provider will simply continue when encountering the situation
 * corresponding to the set bit.
 * <p> The fileCtx field is no longer used in version 2.1, it can be
 * NULL.
 */
typedef struct
{
  Pointer         uiHandle;
  unsigned int    asyncFlag;
  VtFileCtx       fileCtx;
  unsigned char  *trustStore;
} VtCurlTransportInfo;

/** Use this value for the asyncFlag field in the
 * transport info structs. A transport operation encountering an
 * asynchronous situation should continue the work without informing
 * the app.
 */
#define VT_ASYNC_RESPONSE_CONTINUE     1
/** Use this value for the asyncFlag field in the
 * transport info structs. A transport operation encountering an
 * asynchronous situation should stop the operation and inform the app.
 */
#define VT_ASYNC_RESPONSE_ALERT        2
/** Use this value for the asyncFlag field in the
 * transport info structs. A transport operation encountering a
 * previous download request should continue the work without informing
 * the app.
 */
#define VT_PREVIOUS_RESPONSE_CONTINUE  4
/** Use this value for the asyncFlag field in the
 * transport info structs. A transport operation encountering a
 * previous download request should stop the operation and inform the
 * app.
 */
#define VT_PREVIOUS_RESPONSE_ALERT     8

/** Destroy the transport context.  This frees up any memory allocated
 * during the context's creation and use.
 * <p>If you call Create, you must call Destroy after you are done with
 * the context but before the program exits.
 * <p>Example:
 * <code>
 * <pre>
 *    int status;
 *    VtTransportCtx transportCtx = (VtTransportCtx)0;
 *    VtWinINetTransportInfo transInfo;
 *
 *    do {
 *          . . .
 *      transInfo.uiHandle = (Pointer)0;
 *      transInfo.asyncFlag = VT_ASYNC_RESPONSE_ALERT;
 *      transInfo.fileCtx = (VtFileCtx)0;
 *      status = VtCreateTransportCtx (
 *        libCtx, VtTransportImplHttpsWinINet, (Pointer)&transInfo,
 *        &transportCtx);
 *      if (status != 0)
 *        break;
 *          . . .
 *    } while (0);
 *
 *    VtDestroyTransportCtx (&transportCtx);
 * </pre>
 * </code>
 * @param transportCtx A pointer to where the routine will find the
 * context to destroy.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtDestroyTransportCtx (
   VtTransportCtx *transportCtx
);

/** Set the transport context with the information given.
 * <p>The VtTransportParam defines what information the ctx will be set
 * with.
 * <p>The include file vibe.h defines the supported TransportParams.
 * Look through the include file to see which TransportParam to use for
 * your application. All supported TransportParams will be defined as
 * in the following example.
 * <code>
 * <pre>
 *   extern VtTransportParam VtTransportInfo;
 * </pre>
 * </code>
 * <p>Associated with each TransportParam is specific info. The
 * documentation for each TransportParam will describe the associated
 * info it needs. That data could be another object, it could be data
 * in a particular struct, it might be a NULL pointer. Check the
 * documentation for each TransportParam for a description of the data
 * and its required format.
 * <p>To use this function decide which TransportParam you want to use,
 * then determine what information that TransportParam needs and in
 * which format it is presented. Collect the data in the appropriate
 * format then call this function passing in the desired TransportParam
 * and the required info. The associated info must be cast to Pointer.
 * <p>For example, if the required info is a VtItem struct, declare a
 * variable to be of type VtItem, set the fields with the appropriate
 * info, then pass the address of that VtItem cast to Pointer.
 *
 * @param transportCtx The context to set.
 * @param transportParam What the ctx is being set to.
 * @param associatedInfo The info needed by the InfoType.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtSetTransportParam (
   VtTransportCtx transportCtx,
   VtTransportParam transportParam,
   Pointer associatedInfo
);

/** Get the specified type of information out of a transport context.
 * <p>This function will go to the address given by getInfo and
 * deposit a pointer. That pointer is the address of the data. The
 * memory to hold the information belongs to the object, do not free it.
 * <p>The VtTransportParam will specify what kind of information will be
 * returned, the getInfo is where the function will deposit the info.
 * <p>The include file vibe.h defines the supported TransportParams.
 * Look through the include file to see which TransportParam to use for
 * your application.
 * <p>See also VtSetTransportParam.
 * <p>To use this function decide which TransportParam you want to use,
 * then determine what information that TransportParam will return and
 * in which format it is presented. Declare a variable to be a pointer
 * to the appropriate type, then call this function passing in the
 * desired TransportParam and the address of the variable. Cast the
 * address of the pointer to the appropriate type: (Pointer *).
 * <p>If the associated info is already a pointer type (such as an
 * object), declare a variable to be of the appropriate type and pass
 * the address of that variable cast to (Pointer *).
 * <p>For example, if the returned info is an unsigned int, declare a
 * variable to be of type (unsigned int *), pass the address of that
 * variable (&varName) cast to (Pointer *).
 * <p>Example:
 * <pre>
 * <code>
 *    unsigned int *getTimeout;
 *
 *    do {
 *          . . .
 *      status = VtGetTransportParam (
 *        polciyCtx, VtTransportParamTimeout, (Pointer *)&timeout);
 *      if (status != 0)
 *        break;
 *          . . .
 *    } while (0);
 * </code>
 * </pre>
 *
 * @param transportCtx The context to query.
 * @param transportParam Defines what info to get.
 * @param getInfo The address of a pointer. The function will go to
 * that address and deposit a pointer to the info requested.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtGetTransportParam (
   VtTransportCtx transportCtx,
   VtTransportParam transportParam,
   Pointer *getInfo
);

/* These are the VtTransportParams supported by the toolkit. Each
 * VtTransportParam is used in conjunction with special info for the
 * function. If there is no special info, the accompaniment is a NULL
 * pointer.
 */

/** SetParam and GetParam.
 * <p>Use this to set the timeout value used by the transport provider.
 * <p>The timeout is given in number of milliseconds. It is the amount
 * of time the transport provider will wait for a network download to
 * complete before giving up. If you do not use this Param the
 * transport provider will use a default timeout.
 * <p>The data associated with VtTransportParamTimeout is a pointer to
 * an unsigned int giving the timeout value in milliseconds. If the
 * value is 0xffffffff (VT_TIMEOUT_INFINITE), the transport provider
 * will consider the timeout to be infinite.
 */
extern VtTransportParam VtTransportParamTimeout;

/** SetParam only
 * <p>This SetParam loads a username/password collector into a
 * transport ctx. Whenever a transport provider needs to perform
 * password based authentication, it will call this username/password
 * collector and pass it the identity object for which the
 * authentication information (username and password) is required.
 * <p>If you load a username/password collector into a transport ctx,
 * it is still possible the password collector will not be called. The
 * collector will only be called when the transport ctx needs to
 * authenticate an identity.
 * <p> The data associated while using the SetParam is a pointer to the 
 * VtUserPassCollectorCallback struct. 
 * <p>If a transport provider doesn't need a password collector an error will
 * be returned.
 */
extern VtTransportParam VtTransportParamUserPassCollector;

/** This definition is to retain backwards compatibility. Beginning
 * with version 2.2 of the toolkit, the PasswordCollector was renamed
 * to UserPassCollector to help keep clear the the PasswordManager and
 * UserPassCollector.
 */
#define VtTransportParamPasswordCollector VtTransportParamUserPassCollector

/** SetParam only.
 * <p>This VtTransportParam is used to set a transport context with a
 * shared secret associated with a district. It is used with token-based
 * transport providers (e.g. VtTransportImplDelegatedWinINet).
 * <p>The associated info is a VtDelegatedSecretInfo struct.
 * <p>When setting, build the VtDelegatedSecretInfo struct and pass a
 * pointer to the struct as the associatedInfo.
 * <p>NOTE!!! In previous versions of the toolkit, the associated info
 * was VtTransportDelegatedInfo. The new associated info is actually
 * the same structure, just with the field names changed. In the old
 * version, the associated info demanded a fully qualified name, this
 * new associated info allows an unqualified or qualified district name
 * (e.g. developer.voltage.com or developer.voltage.com#1084400664).
 * The new struct simply renames the fields to reflect this fact. The
 * old struct will still work, and you can use an unqualified name for
 * the "qualDistName" field. It's probably clearer to use the new
 * struct with the more accurate field name.
 */
extern VtTransportParam VtTransportParamDelegatedSecret;

/** This is the data struct to accompany
 * VtTransportParamDelegatedSecret.
 * <p>The districtName field can be the unqualified or qualified
 * district name (e.g. developer.voltage.com or
 * developer.voltage.com#1084400664). It must be a NULL-terminated UTF8
 * string.
 * <p>Each district can have multiple shared secrets for different 
 * identities. Often email addresses of identities that match a
 * particular regular expression are assigned one secret. For example,
 * all users whose email addresses match *\@voltage.com can be assigned
 * one secret while users whose email addresses match *\@hotmail.com
 * can be assigned another secret. (Note: if you're reading these
 * comments in doxygen, they are correct. If you're reading them in the
 * source file, ignore the \ in the *\@voltage.com. That's there for
 * doxygen formatting. It's really *<at-sign>voltage.com. The at-sign
 * is a doxygen symbol and must be preceded with the backslash to
 * indicate that doxygen should not treat it as a special symbol, much
 * like the C language string usage, char *directory = "c:\\temp".)
 * <p>regularExpression is the regular expression to match users
 * emails. (for instance, *\@hotmail.com). Currently it's not being
 * used but is there for future releases. It must be a NULL-terminated
 * UTF8 string.
 */
typedef struct
{
  unsigned char *districtName;
  unsigned char *regularExpression;
  VtItem sharedSecret;
} VtDelegatedSecretInfo;

/** This is the old structure to accompany
 * VtTransportParamDelegatedSecret. The fields are the same, just
 * different names to reflect that the district name supplied does not
 * have to be qualified.
 */
typedef struct
{
  unsigned char *qualDistName;
  unsigned char *regExp;
  VtItem sharedSecret;
} VtTransportDelegatedInfo;

/*@}*/

/*=========================================================*/
/*                                                         */
/* Connection Cache Context                                */
/*                                                         */
/*=========================================================*/

/**
 * \defgroup ConnectionCacheCtxGroup Connection Cache Context
 */

/*@{*/

/** The connection cache context.
 * <p>Note that the ctx is a pointer type.
 * <p>The connection cache context is used by the transport
 * contexts to cache connections across HTTP requests, which
 * improves performance.
 * <p>The caller will first create a connection cache context,
 * then set it for the library context.
 */
typedef struct VtConnectionCacheCtxDef *VtConnectionCacheCtx;

/** The function VtCreateConnectionCacheCtx builds a connection
 * cache context using a VtConnectionCacheCtxImpl. This typedef
 * defines what a VtConnectionCacheCtxImpl is. Although a
 * VtConnectionCacheCtxImpl is a function pointer, an application
 * should never call one directly, only pass it as an argument to
 * VtCreateConnectionCacheCtx.
 */
typedef int VT_CALLING_CONV (VtConnectionCacheCtxImpl) (
   VtConnectionCacheCtx *, Pointer, unsigned int);

/** Create a new Connection Cache Ctx. This allocates space for an "empty"
 * ctx, then loads the given ConnectionCacheCtxImpl to make it "active".
 * <p>Currently there is only one thing you can do with a Connection Cache
 * Ctx, load it into the libCtx (see VtSetLibCtxParam). The Connection Cache
 * Ctx loaded into the libCtx will allow many transport context operations
 * to execute faster, although the code size and memory usage of the application
 * will increase.
 * <p>The VtConnectionCacheCtxImpl defines the caching implementation. The
 * supported ConnectionCacheCtxImpls are defined below. All supported
 * VtConnectionCacheCtxImpls will be defined as in the following example.
 * <code>
 * <pre>
 *   extern VtConnectionCacheCtxImpl VtConnectionCacheCtxImplBasic;
 * </pre>
 * </code>
 * <p>Associated with each VtConnectionCacheCtxImpl is specific info. The
 * documentation for each VtConnectionCacheCtxImpl will describe the
 * associated info it needs. That data could be another object, it could
 * be data in a particular struct, it might be a NULL pointer. Check the
 * documentation for each VtConnectionCacheCtxImpl for a description of
 * the data and its required format.
 * <p>The input connectionCacheCtx is a pointer to a cache object. It should
 * point to a NULL VtConnectionCacheCtx. This function will go to the address
 * given and deposit a created context.
 * <p>If you call Create, you must call Destroy after you are done with
 * the cache but before the program exits.
 * <p>Example:
 * <code>
 * <pre>
 *    int status;
 *    VtConnectionCacheCtx connectionCacheCtx = (VtConnectionCacheCtx)0; 
 *
 *    do {
 *          . . . 
 *      status = VtCreateConnectionCacheCtx (
 *        libCtx, VtConnectionCacheCtxImplBasic, (Pointer)0, &connectionCacheCtx);
 *      if (status != 0)
 *        break;
 *          . . .
 *    } while (0);
 * 
 *    VtDestroyConnectionCacheCtx (&connectionCacheCtx);
 * </pre>
 * </code>
 * @param libCtx The library context.
 * @param connectionCacheCtxImpl The implementation the cache ctx will use.
 * @param associatedInfo The info needed by the VtConnectionCacheCtxImpl.
 * @param connectionCacheCtx A pointer to where the routine will deposit the
 * created context.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtCreateConnectionCacheCtx (
   VtLibCtx libCtx,
   VtConnectionCacheCtxImpl connectionCacheCtxImpl,
   Pointer associatedInfo,
   VtConnectionCacheCtx *connectionCacheCtx
);

/* These are the VtConnectionCacheCtxImpls supported by the toolkit. Each
 * ConnectionCacheCtxImpl is used in conjunction with special info for the
 * function. If there is no special info, the accompaniment is a NULL
 * pointer.
 */

/** This VtConnectionCacheCtxImplBasic is used to build a connection
 * cache that performs basic caching operations on all platforms.
 * <p>The data associated with VtConnectionCacheCtxImplBasic is a NULL
 * pointer.
 */
extern VtConnectionCacheCtxImpl VtConnectionCacheCtxImplBasic;

/** Destroy the Connection Cache Ctx.  This frees up any memory allocated
 * during the context's creation and use.
 * <p>If you call Create, you must call Destroy after you are done with
 * the cache but before the program exits.
 * <p>Example:
 * <code>
 * <pre>
 *    int status;
 *    VtConnectionCacheCtx connectionCacheCtx = (VtConnectionCacheCtx)0; 
 *
 *    do {
 *          . . . 
 *      status = VtCreateConnectionCacheCtx (
 *        libCtx, VtConnectionCacheCtxImplBasic, (Pointer)0, &connectionCacheCtx);
 *      if (status != 0)
 *        break;
 *          . . .
 *    } while (0);
 * 
 *    VtDestroyConnectionCacheCtx (&connectionCacheCtx);
 * </pre>
 * </code>
 * @param connectionCacheCtx A pointer to where the routine will find the ctx
 * to destroy.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtDestroyConnectionCacheCtx (
   VtConnectionCacheCtx *connectionCacheCtx
);

/*@}*/

/*=========================================================*/
/*                                                         */
/* Storage Context                                         */
/*                                                         */
/*=========================================================*/

/**
 * \defgroup StorageCtxGroup Storage Context
 */

/*@{*/

/** The storage context.
 * <p>Note that the ctx is a pointer type.
 * <p>The storage context is somewhat different. It is a single
 * "object" that can contain many storage providers. An application may
 * have different values stored in different places.
 * <p>For instance, an application may store certs and district
 * parameters on disk, but private keys on a hardware token. When
 * searching for elements, the toolkit functions will search all
 * providers given. So when the toolkit needs district parameters, it
 * will find it in the storage provider that stored elements on disk.
 * When it needs a private key, it will find it in the storage provider
 * that stored elements on the hardware token.
 * <p>An application will build a storage context, then add providers
 * using the VtAddStorageProvider call.
 */
typedef struct VtStorageCtxDef *VtStorageCtx;

/** The function VtCreateStorageCtx builds a storage context using a
 * VtStorageImpl. This typedef defines what a VtStorageImpl is.
 * Although a VtStorageImpl is a function pointer, an application should
 * never call one directly, only pass it as an argument to
 * VtCreateStorageCtx.
 */
typedef int VT_CALLING_CONV (VtStorageImpl) (
   VtStorageCtx *, Pointer, unsigned int);

/** The function VtAddStorageProvider adds a provider to a storage ctx.
 * The provider to add is defined by a VtStorageProvider. This typedef
 * defines what a VtStorageProvider is. Although a StorageProvider is a
 * function pointer, an application should never call one directly,
 * only pass it as an argument to VtAddStorageProvider.
 */
typedef int VT_CALLING_CONV (VtStorageProvider) (
   VtStorageCtx, Pointer, unsigned int, int);

/** The functions VtSetStorageParam and VtGetStorageParam add or get
 * information to or from a storage ctx. The information to add or get
 * is defined by a VtStorageParam. This typedef defines what a
 * VtStorageParam is. Although a StorageParam is a function pointer, an
 * application should never call one directly, only pass it as an
 * argument to VtSetStorageParam or VtGetStorageParam.
 */
typedef int VT_CALLING_CONV (VtStorageParam) (
   VtStorageCtx, Pointer, unsigned int);

/** Create a new storage context. This allocates space for an "empty"
 * context, then loads the given StorageImpl to make it an "active"
 * context.
 * <p>The VtStorageImpl defines the storage implementation. The include
 * file vibe.h defines the supported StorageImpls. Look through the
 * include file to see which StorageImpl to use for your application.
 * All supported StorageImpls will be defined as in the following
 * example.
 * <code>
 * <pre>
 *   extern VtStorageImpl VtStorageImplBasic;
 * </pre>
 * </code>
 * <p>Associated with each StorageImpl is specific info. The
 * documentation for each StorageImpl will describe the associated info
 * it needs. That data could be another object, it could be data in a
 * particular struct, it might be a NULL pointer. Check the
 * documentation for each StorageImpl for a description of the data and
 * its required format.
 * <p>To use this function decide which StorageImpl you want to use,
 * then determine what information that StorageImpl needs and in which
 * format it is presented. Collect the data in the appropriate format
 * then call this function passing in the desired StorageImpl and the
 * required info. The associated info must be cast to Pointer.
 * <p>For example, if the required info is a VtItem, declare a variable
 * to be of type VtItem, fill in the fields, then pass the address of
 * that VtItem cast to Pointer.
 * <p>The input storageCtx is a pointer to a context. It should point to
 * a NULL VtStorageCtx. This function will go to the address given and
 * deposit a created context.
 * <p>If you call Create, you must call Destroy after you are done with
 * the context but before the program exits.
 * <p>Example:
 * <code>
 * <pre>
 *    int status;
 *    VtStorageCtx storageCtx = (VtStorageCtx)0;
 *
 *    do {
 *          . . .
 *      status = VtCreateStorageCtx (
 *        libCtx, VtStorageImplBasic, (Pointer)0, &storageCtx);
 *      if (status != 0)
 *        break;
 *          . . .
 *    } while (0);
 *
 *    VtDestroyStorageCtx (&storageCtx);
 * </pre>
 * </code>
 * @param libCtx The library context.
 * @param storageImpl The implementation the context will use.
 * @param associatedInfo The info needed by the StorageImpl.
 * @param storageCtx A pointer to where the routine will deposit the
 * created context.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtCreateStorageCtx (
   VtLibCtx libCtx,
   VtStorageImpl storageImpl,
   Pointer associatedInfo,
   VtStorageCtx *storageCtx
);

/* These are the VtStorageImpls supported by the toolkit. Each
 * StorageImpl is used in conjunction with special info for the function.
 * If there is no special info, the accompaniment is a NULL pointer.
 */

/** This VtStorageImpl is used to build a storage context with the
 * basic implementation.
 * <p>The data associated with VtStorageImplBasic is a NULL pointer:
 * (Pointer)0.
 */
extern VtStorageImpl VtStorageImplBasic;

/** Destroy the storage context.  This frees up any memory allocated
 * during the context's creation and use.
 * <p>If you call Create, you must call Destroy after you are done with
 * the context but before the program exits.
 * <p>Example:
 * <code>
 * <pre>
 *    int status;
 *    VtStorageCtx storageCtx = (VtStorageCtx)0;
 *
 *    do {
 *          . . .
 *      status = VtCreateStorageCtx (
 *        libCtx, VtStorageImplBasic, (Pointer)0, &storageCtx);
 *      if (status != 0)
 *        break;
 *          . . .
 *    } while (0);
 *
 *    VtDestroyStorageCtx (&storageCtx);
 * </pre>
 * </code>
 * @param storageCtx A pointer to where the routine will find the context to
 * destroy.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtDestroyStorageCtx (
   VtStorageCtx *storageCtx
);

/** Add a provider to the storage context. It is possible to add more
 * than one storage provider to a storage context.
 * <p>The VtStorageProvider defines the provider.
 * <p>The include file vibe.h defines the supported StorageProviders.
 * Look through the include file to see which StorageProvider to use for
 * your application. All supported StorageProviders will be defined as
 * in the following example.
 * <code>
 * <pre>
 *   extern VtStorageProvider VtStorageFileWin32;
 * </pre>
 * </code>
 * <p>Associated with each StorageProvider is specific info. The
 * documentation for each StorageProvider will describe the associated
 * info it needs. That data could be another object, it could be data
 * in a particular struct, it might be a NULL pointer. Check the
 * documentation for each StorageProvider for a description of the data
 * and its required format.
 * <p>To use this function decide which StorageProvider you want to
 * use, then determine what information that StorageProvider needs and
 * in which format it is presented. Collect the data in the appropriate
 * format then call this function passing in the desired
 * StorageProvider adn the required info. The associated info must be
 * cast to Pointer.
 * <p>For example, if the required info is a char array (a
 * NULL-terminated string), declare a variable to be of type char *,
 * set it to point to the NULL-terminated string, then pass that char *
 * cast to Pointer.
 *
 * @param storageCtx The context to which the provider is added.
 * @param storageProvider The storage provider to add.
 * @param associatedInfo The info needed by the StorageProvider.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtAddStorageProvider (
   VtStorageCtx storageCtx,
   VtStorageProvider storageProvider,
   Pointer associatedInfo
);

/* These are the StorageProviders supported by the toolkit. Each
 * StorageProvider is used in conjunction with special info for the
 * function. If there is no special info, the accompaniment is a
 * NULL pointer.
 */

/** This VtStorageProvider is used to add a file-based storage system
 * on Win32 platforms that support Microsoft's Windows protected storage.
 * This provider stores all the data in files and does not use the
 * Windows registry, as some of the older Voltage client software did.
 * <p>This storage provider protects both public and sensitive
 * information. The purpose of protecting non-sensitive information is
 * to detect any tampering whereas sensitive information is protected
 * so that malicious users can't gain access to it. The users are given
 * the option of adding additionanl protection by using their own
 * secret information (passwords). For more information on how to
 * protect the storage with your own password see the documentation of
 * VtStorageParamWinExtraPassword.
 * <p>Signing certs, current districts, district parameters and request
 * info files are protected with login credentials of the user to
 * detect any kind of tampering with the data.
 * <p>Private keys, signing keys and Authentication tokens are by default 
 * protected using the login credentials of the user but can be further 
 * protected using a user supplied password. 
 * <p>The data associated with VtStorageFileWin32 is a pointer to a
 * VtFileCtxUseInfo struct or a NULL pointer.
 * <p>If not NULL, the path field of that struct is where the context
 * will store the data. If that field is null, the data is stored in a
 * default location.
 * <p>If the fileCtx field is NULL, the toolkit will look in the libCtx
 * for a fileCtx. If the field is not NULL, the storage context will
 * use the fileCtx given, even if there is one inside the libCtx.
 * <p>If the associated info is NULL, the toolkit will use the fileCtx
 * loaded in the libCtx and store the info in the default location.
 * <p>Example:
 * <code>
 * <pre>
 *    int status;
 *    VtFileCtxUseInfo storageInfo;
 *
 *    do {
 *      status = VtCreateStorageCtx (
 *        libCtx, VtStorageImplBasic, (Pointer)0, &storageCtx);
 *      if (status != 0)
 *        break;
 *
 *      // Assume the file ctx has been created.
 *      storageInfo.fileCtx = fileCtx;
 *      storageInfo.path = "C:\\Voltage";
 *      status = VtAddStorageProvider (
 *        storageCtx, VtStorageFileWin32, (Pointer)&storageInfo);
 *      if (status != 0)
 *        break;
 *
 *    } while (0);
 *
 *    VtDestroyStorageCtx (&storageCtx);
 * </pre>
 * </code>
 */
extern VtStorageProvider VtStorageFileWin32;

/** This VtStorageProvider is used to add a file and registry based 
 * storage provider on Win32 platforms that support Microsoft's Windows
 * protected storage. This provider is provided for backward compatibility 
 * with older Voltage client software.
 * This storage by default, protects any sensitive information using login 
 * credentials of the user. The users are also given option to protect any 
 * sensitive information using their own secret(passwords). For more information
 * on how to protect the storage with your own password see the documentation of
 * the function VtStorageParamWinExtraPassword. 
 * <p>Private keys, signing keys and Authentication tokens are by default 
 * protected using the login credentials of the user but can be 
 * further protected using a user supplied password
 * <p>Older client software (such as SecureMail and SecureFile) stored
 * IBE and DSA private keys in a non-standard way. The toolkit stores
 * them following PKCS #8 and in different directories. This storage stores
 * current districts and public parameters in windows registry. The format to
 * store current districts and district parameters is also different and they 
 * don't have district validity periods in it. The format of IBE and DSA 
 * private keys is also diferent as compared to the toolkit storage.
 * <p>This provider will, when searching for keys, look for items
 * stored the older client way. It will store keys using the technique
 * used by the older client software. 
 * <p>See also the documentation for VtStorageFileWin32.
 * <p>The data associated with VtStorageFileWin32Client is a file ctx
 * or a NULL pointer. If a NULL pointer, the toolkit will try to find a file
 * context from the libCtx. 
 */
extern VtStorageProvider VtStorageFileWin32Client;

/** This VtStorageProvider is used to add a file-based storage system
 * on Unix platforms. Keys, certs, and other material are stored in
 * files, sensitive material (such as private keys) are protected by
 * the underlying OS file permissions.
 * <p>The data associated with VtStorageFileUnix is a pointer to a
 * VtFileCtxUseInfo struct.
 * <p>The path field of that object tells where to store the data. If
 * it's null, the data is stored in a default location.
 * <p>Example:
 * <code>
 * <pre>
 *    int status;
 *    VtFileCtxUseInfo fileInfo;
 *
 *    do {
 *      status = VtCreateStorageCtx (
 *        libCtx, VtStorageImplBasic, (Pointer)0, &storageCtx);
 *      if (status != 0)
 *        break;
 *
 *      // Assume the file ctx has been created.
 *      fileInfo.fileCtx = fileCtx;
 *      fileInfo.path = "/home/Voltage";
 *      status = VtAddStorageProvider (
 *        storageCtx, VtStorageFileUnix, (Pointer)&fileInfo);
 *      if (status != 0)
 *        break;
 *
 *    } while (0);
 *
 *    VtDestroyStorageCtx (&storageCtx);
 * </pre>
 * </code>
 */
extern VtStorageProvider VtStorageFileUnix;

/** Set the storage context with the information given.
 * <p>The VtStorageParam defines what the ctx will be set with.
 * <p>The include file vibe.h defines the supported StorageParams.
 * Look through the include file to see which StorageParam to use for
 * your application. All supported StorageParams will be defined as in
 * the following example.
 * <code>
 * <pre>
 *   extern VtStorageParam VtStorageKey;
 * </pre>
 * </code>
 * <p>Associated with each StorageParam is specific info. The
 * documentation for each StorageParam will describe the associated info
 * it needs. That data could be another object, it could be data in a
 * particular struct, it might be a NULL pointer. Check the
 * documentation for each StorageParam for a description of the data and
 * its required format.
 * <p>To use this function decide which StorageParam you want to use,
 * then determine what information that StorageParam needs and in which
 * format it is presented. Collect the data in the appropriate format
 * then call this function passing in the desired StorageParam and the
 * required info. The associated info must be cast to Pointer.
 * <p>For example, if the required info is a key object, build the key
 * object, then pass that object cast to Pointer.
 *
 * @param storageCtx The context to set.
 * @param storageParam What the ctx is being set to.
 * @param associatedInfo The info needed by the StorageParam.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtSetStorageParam (
   VtStorageCtx storageCtx,
   VtStorageParam storageParam,
   Pointer associatedInfo
);

/* These are the VtStorageParams supported by the toolkit. Each
 * VtStorageParam is used in conjunction with special info for the
 * function. If there is no special info, the accompaniment is a NULL
 * pointer.
 */

/** SetParam only
 * <p>Use this StorageParam to set a PasswordManager on a storage
 * provider. The storage provider will call this PasswordManager
 * whenever it needs to perform password related operations. This is
 * mainly to support secure storage providers that protects the stored
 * data with a password.
 * <p>The associated info for this param is a pointer to 
 * VtPasswordManagerCallback struct.
 * <p>If this Param is used on a storage ctx that has more than one
 * provider loaded, it will operate only on the first provider.
 * <p>If the provider does not need a password manager, setting a context
 * with this Param will cause an error.
 * <p>This Param is not for use with non-password-based storage providers.
 */
extern VtStorageParam VtStorageParamPasswordManager;

/** SetParam only.
 * <p>Use this StorageParam to set or change an extra password in a
 * Windows storage provider.
 * <p>Both the Win32 and Win32Client storage providers use Windows
 * login credentials and the Windows protected storage to protect
 * stored entries. It is possible to add another password to use when
 * storing (and retrieving) private keys.
 * <p>The associated info is a random object. The storage provider must
 * have already been loaded with a password manager (By calling 
 * VtStorageParamPasswordManager as mentioned above).
 * <p>If this Param is used on a storage ctx that has more than one
 * provider loaded, it will operate only on the first provider.
 * <p>If the provider does not use an extra password, setting a context
 * with this Param will cause an error.
 * <p>This Param is not for use with password-based storage providers.
 * This is only for the Windows file-based providers that use an extra
 * password.
 * <p>Note that when a provider is built, it is tied to a particular
 * directory. This param will set or change the password for only those
 * keys in the directory specified when the provider was created (or
 * the default directory if no path is specified).
 */
extern VtStorageParam VtStorageParamWinExtraPassword;

/*@}*/

/*=========================================================*/
/*                                                         */
/* Cert Verify Context                                     */
/*                                                         */
/*=========================================================*/

/**
 * \defgroup CertVfyCtxGroup CertVerify Context
 */

/*@{*/

/** The certVerify context.
 * <p>Note that the ctx is a pointer type.
 */
typedef struct VtCertVerifyCtxDef *VtCertVerifyCtx;

/** The function VtCreateCertVerifyCtx builds a certVerify context
 * using a VtCertVerifyImpl. This typedef defines what a
 * VtCertVerifyImpl is. Although it is a function pointer, an
 * application should never call a VtCertVerifyImpl directly, only pass
 * it as an argument to VtCreateCertVerifyCtx.
 */
typedef int VT_CALLING_CONV (VtCertVerifyImpl) (
   VtCertVerifyCtx *, Pointer, unsigned int);

/** The functions VtSetCertVerifyParam and VtGetCertVerifyParam add or
 * get information to or from a certVerify ctx. The information to add
 * or get is defined by a VtCertVerifyParam. This typedef defines what
 * a VtCertVerifyParam is. Although a VtCertVerifyParam is a function
 * pointer, an application should never call one directly, only pass it
 * as an argument to VtSetCertVerifyParam or VtGetCertVerifyParam.
 */
typedef int VT_CALLING_CONV (VtCertVerifyParam) (
   VtCertVerifyCtx, Pointer, unsigned int);

/** Create a new certVerify context. This allocates space for an "empty"
 * context, then loads the given CertVerifyImpl to make it an "active"
 * context.
 * <p>The VtCertVerifyImpl defines the certVerify implementation. The
 * include file vibe.h defines the supported CertVerifyImpls. Look
 * through the include file to see which CertVerifyImpl to use for your
 * application. All supported CertVerifyImpls will be defined as in the
 * following example.
 * <code>
 * <pre>
 *   extern VtCertVerifyImpl VtCertVerifyImplBasic;
 * </pre>
 * </code>
 * <p>Associated with each CertVerifyImpl is specific info. The
 * documentation for each CertVerifyImpl will describe the associated
 * info it needs. That data could be another object, it could be data
 * in a particular struct, it might be a NULL pointer. Check the
 * documentation for each CertVerifyImpl for a description of the data
 * and its required format.
 * <p>To use this function decide which CertVerifyImpl you want to use,
 * then determine what information that CertVerifyImpl needs and in which
 * format it is presented. Collect the data in the appropriate format
 * then call this function passing in the desired CertVerifyImpl and the
 * required info. The associated info must be cast to Pointer.
 * <p>For example, if the required info is a VtItem, declare a variable
 * to be of type VtItem, fill in the fields, then pass the address of
 * that VtItem cast to Pointer.
 * <p>The input certVerifyCtx is a pointer to a context. It should point
 * to a NULL VtCertVerifyCtx. This function will go to the address given
 * and deposit a created context.
 * <p>If you call Create, you must call Destroy after you are done with
 * the context but before the program exits.
 * <p>Example:
 * <code>
 * <pre>
 *    int status;
 *    VtCertVerifyCtx certVerifyCtx = (VtCertVerifyCtx)0;
 *
 *    do {
 *          . . .
 *      status = VtCreateCertVerifyCtx (
 *        libCtx, VtCertVerifyImplBasic, (Pointer)0,
 *        &certVerifyCtx);
 *      if (status != 0)
 *        break;
 *          . . .
 *    } while (0);
 *
 *    VtDestroyCertVerifyCtx (&certVerifyCtx);
 * </pre>
 * </code>
 * @param libCtx The library context.
 * @param certVerifyImpl The implementation the context will use.
 * @param associatedInfo The info needed by the CertVerifyImpl.
 * @param certVerifyCtx A pointer to where the routine will deposit the
 * created context.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtCreateCertVerifyCtx (
   VtLibCtx libCtx,
   VtCertVerifyImpl certVerifyImpl,
   Pointer associatedInfo,
   VtCertVerifyCtx *certVerifyCtx
);

/* These are the VtCertVerifyImpls supported by the toolkit. Each
 * CertVerifyImpl is used in conjunction with special info for the
 * function. If there is no special info, the accompaniment is a NULL
 * pointer.
 * The documentation for each Impl also describes what verifyInfo is
 * needed and what format it is presented when verifying a certificate.
 */

/** This VtCertVerifyImpl is used to build a CertVerifyCtx that will
 * perform basic verification. It will check the date and key usage
 * of the cert, along with the signature. It will not check any other
 * extension.
 * <p>When creating a cert verify ctx, the associated info is a NULL
 * pointer: (Pointer)0.
 * <p>When verifying a cert, the verifyInfo is a pointer to a
 * VtBasicCertVerifyInfo struct.
 * <p>The data associated with VtCertVerifyImplBasic is a NULL pointer:
 * (Pointer)0.
 */
extern VtCertVerifyImpl VtCertVerifyImplBasic;

/** This is the data struct to use as the verifyInfo when verifying a
 * cert using the Basic certVerifyCtx.
 * <p>The usageTime field is to indicate at what time the cert was
 * used. For example, if the cert is for a signing key, at what time
 * was the signature to check created.
 * <p>The keyUsage flag is to indicate what the key was used for. For
 * example, is the app checking a signature on a PKCS #7 message? Is it
 * checking the signature on a cert? Indicate what the key is being
 * used for and the certVerifyCtx will determine if the cert is allowed
 * to be used for that purpose.
 * <p>Set the keyUsage field to be the logical OR of the following that
 * apply (based on the KeyUsage X.509 extension).
 * <code>
 * <pre>
 *    VT_KEY_USAGE_DIGITAL_SIGNATURE
 *    VT_KEY_USAGE_NON_REPUDIATION
 *    VT_KEY_USAGE_KEY_ENCIPHERMENT
 *    VT_KEY_USAGE_DATA_ENCIPHERMENT
 *    VT_KEY_USAGE_KEY_AGREEMENT
 *    VT_KEY_USAGE_KEY_CERT_SIGN
 *    VT_KEY_USAGE_CRL_SIGN
 *    VT_KEY_USAGE_ENCIPHER_ONLY
 *    VT_KEY_USAGE_DECIPHER_ONLY
 * </pre>
 * </code>
 */
typedef struct
{
  VtTime usageTime;
  unsigned int keyUsage;
} VtBasicCertVerifyInfo;

/** For use in verifyInfo structs that need to know keyUsage. This
 * indicates the cert's partner key was used to create a digital
 * signature on a document that is not another cert. (See also X.509,
 * KeyUsage extension.)
 */
#define VT_KEY_USAGE_DIGITAL_SIGNATURE  0x8000

/** For use in verifyInfo structs that need to know keyUsage. This
 * indicates the cert's partner key was used to sign a document and the
 * signature provides a non-repudiation.
 */
#define VT_KEY_USAGE_NON_REPUDIATION    0x4000

/** For use in verifyInfo structs that need to know keyUsage. This
 * indicates the cert's partner key was used to encrypt another key.
 */
#define VT_KEY_USAGE_KEY_ENCIPHERMENT   0x2000

/** For use in verifyInfo structs that need to know keyUsage. This
 * indicates the cert's partner key was used to encrypt data.
 */
#define VT_KEY_USAGE_DATA_ENCIPHERMENT  0x1000

/** For use in verifyInfo structs that need to know keyUsage. This
 * indicates the cert's partner key was used to perform key agreement.
 */
#define VT_KEY_USAGE_KEY_AGREEMENT      0x0800

/** For use in verifyInfo structs that need to know keyUsage. This
 * indicates the cert's partner key was used to sign another cert.
 */
#define VT_KEY_USAGE_KEY_CERT_SIGN      0x0400

/** For use in verifyInfo structs that need to know keyUsage. This
 * indicates the cert's partner key was used to sign a CRL
 */
#define VT_KEY_USAGE_CRL_SIGN           0x0200

/** For use in verifyInfo structs that need to know keyUsage. This flag
 * is used in conjunction with KEY_AGREEMENT. If the cert's partner key
 * was used to perform key agreement and the key used to encrypt data,
 * set this bit.
 */
#define VT_KEY_USAGE_ENCIPHER           0x0100

/** For use in verifyInfo structs that need to know keyUsage. This flag
 * is used in conjunction with KEY_AGREEMENT. If the cert's partner key
 * was used to perform key agreement and the key used to decrypt data,
 * set this bit.
 */
#define VT_KEY_USAGE_DECIPHER           0x0080

/** Destroy the certVerify context.  This frees up any memory allocated
 * during the context's creation and use.
 * <p>If you call Create, you must call Destroy after you are done with
 * the context but before the program exits.
 * <p>Example:
 * <code>
 * <pre>
 *    int status;
 *    VtCertVerifyCtx certVerifyCtx = (VtCertVerifyCtx)0;
 *
 *    do {
 *          . . .
 *      status = VtCreateCertVerifyCtx (
 *        libCtx, VtCertVerifyImplBasic, (Pointer)0,
 *        &certVerifyCtx);
 *      if (status != 0)
 *        break;
 *          . . .
 *    } while (0);
 *
 *    VtDestroyCertVerifyCtx (&certVerifyCtx);
 * </pre>
 * </code>
 * @param certVerifyCtx A pointer to where the routine will find the context to
 * destroy.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtDestroyCertVerifyCtx (
   VtCertVerifyCtx *certVerifyCtx
);

/** Set the certVerify context with the information given.
 * <p>The VtCertVerifyParam defines what information the ctx will be
 * set with.
 * <p>The include file vibe.h defines the supported CertVerifyParams.
 * Look through the include file to see which CertVerifyParam to use for
 * your application. All supported CertVerifyParams will be defined as
 * in the following example.
 * <code>
 * <pre>
 *   extern VtCertVerifyParam VtCertVerifyParamInfo;
 * </pre>
 * </code>
 * <p>Associated with each CertVerifyParam is specific info. The
 * documentation for each CertVerifyParam will describe the associated
 * info it needs. That data could be another object, it could be data
 * in a particular struct, it might be a NULL pointer. Check the
 * documentation for each CertVerifyParam for a description of the data
 * and its required format.
 * <p>To use this function decide which CertVerifyParam you want to use,
 * then determine what information that CertVerifyParam needs and in
 * which format it is presented. Collect the data in the appropriate
 * format then call this function passing in the desired CertVerifyParam
 * and the required info. The associated info must be cast to Pointer.
 * <p>For example, if the required info is a char array (a
 * NULL-terminated string), declare a variable to be of type char *,
 * set it to point to the NULL-terminated string, then pass that char *
 * cast to Pointer.
 *
 * @param certVerifyCtx The context to set.
 * @param certVerifyParam What the ctx is being set to.
 * @param associatedInfo The info needed by the CertVerifyParam.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtSetCertVerifyParam (
   VtCertVerifyCtx certVerifyCtx,
   VtCertVerifyParam certVerifyParam,
   Pointer associatedInfo
);

/* These are the VtCertVerifyParams supported by the toolkit. Each
 * CertVerifyParam is used in conjunction with special info for the
 * function. If there is no special info, the accompaniment is a NULL
 * pointer.
 */

/** SetParam only
 * <p>Add an array of cert objects to the context. The context will use
 * those certs when verifying other certs.
 * <p>Any cert in the list supplied with this Param will be considered
 * trusted. That is, the context will not verify the signature of any
 * of thes certs in the trusted list. If the elements verify (validity
 * dates, extensions, etc.), the context will use a trusted cert's
 * public key to verify signatures or other certs without further
 * checks. In other words, it will not chain.
 * <p>The associated info is a VtCertObjectList struct
 * <p>When setting, build the VtCertObjectList and pass its address as
 * the associatedInfo.
 */
VtCertVerifyParam VtCertVerifyParamTrustedCerts;

/** SetParam only
 * <p>Add an array of cert objects to the context. The context will use
 * those certs when verifying other certs.
 * <p>Any cert in the list supplied with this Param will NOT be
 * considered trusted. That is, if the context uses a cert from this
 * list to verify a signature or another cert, it will verify the
 * signature of the cert. In other words, it will chain.
 * <p>The associated info is a VtCertObjectList struct
 * <p>When setting, build the VtCertObjectList and pass its address as
 * the associatedInfo.
 */
VtCertVerifyParam VtCertVerifyParamUntrustedCerts;

/** SetParam only
 * <p>Indicate how the cert verification ctx should handle certs from
 * districts when the district in the cert is not the current district.
 * <p>The info associated with VtCertVerifyParamCurrentDistrictCheck is
 * a pointer to a VtCurrentDistrictCheckInfo struct.
 * <p>See the User's manual for more information.
 */
VtCertVerifyParam VtCertVerifyParamCurrentDistrictCheck;

/** This is the data to accompany VtCertVerifyParamCurrentDistrictCheck.
 * <p>The flag must be one of the following values.
 * <pre>
 * <code>
 *    VT_CURRENT_DISTRICT_CHECK_IGNORE
 *    VT_CURRENT_DISTRICT_CHECK_IGNORE_PLANNED
 *    VT_CURRENT_DISTRICT_CHECK_STRICT
 * </code>
 * </pre>
 * <p>The policy, storage, and transport contexts can be NULL if they
 * are loaded in the libCtx, or if the flag is "IGNORE".
 */
typedef struct
{
  unsigned int flag;
  VtPolicyCtx policyCtx;
  VtStorageCtx storageCtx;
  VtTransportCtx transportCtx;
} VtCurrentDistrictCheckInfo;

/** For use with VtCertVerifyParamCurrentDistrictCheck, idicate that a
 * cert verify ctx should ignore the current district when verifying
 * certs.
 */
#define VT_CURRENT_DISTRICT_CHECK_IGNORE           0x001

/** For use with VtCertVerifyParamCurrentDistrictCheck, idicate that a
 * cert verify ctx should ignore planned rollovers when the district in
 * a CA cert is not the current district.
 */
#define VT_CURRENT_DISTRICT_CHECK_IGNORE_PLANNED   0x002

/** For use with VtCertVerifyParamCurrentDistrictCheck, idicate that a
 * cert verify ctx should return "does not verify" and set the
 * VerifyFailureList when the district in a CA cert is not the current
 * district.
 */
#define VT_CURRENT_DISTRICT_CHECK_STRICT           0x100

/*@}*/

/*=========================================================*/
/*                                                         */
/* District Object                                         */
/*                                                         */
/*=========================================================*/

/**
 * \defgroup DistrictGroup District Object
 */

/*@{*/

/** The district object.
 * <p>Note that the object is a pointer type.
 */
typedef struct VtDistrictObjectDef *VtDistrictObject;

/** The function VtCreateDistrictObject builds a district object using
 * a VtDistrictImpl. This typedef defines what a VtDistrictImpl is.
 * Although a VtDistrictImpl is a function pointer, an application
 * should never call one directly, only pass it as an argument to
 * VtCreateDistrictObject.
 */
typedef int VT_CALLING_CONV (VtDistrictImpl) (
   VtDistrictObject *, Pointer, unsigned int);

/** The functions VtSetDistrictParam and VtGetDistrictParam add or
 * get information to or from a district object. The information to add
 * or get is defined by a VtDistrictParam. This typedef defines what a
 * VtDistrictParam is. Although a VtDistrictParam is a function
 * pointer, an application should never call one directly, only pass it
 * as an argument to VtSetDistrictParam or VtGetDistrictParam.
 */
typedef int VT_CALLING_CONV (VtDistrictParam) (
   VtDistrictObject, Pointer, unsigned int);

/** Create a new district object. This allocates space for an "empty"
 * object, then loads the given DistrictImpl to make it an "active"
 * object.
 * <p>The VtDistrictImpl defines some of the district object operations.
 * The include file vibe.h defines the supported DistrictImpls. Look
 * through the include file to see which DistrictImpl to use for your
 * application. All supported DistrictImpls will be defined as in the
 * following example.
 * <pre>
 * <code>
 *   extern VtDistrictImpl VtDistrictImplBasic;
 * </code>
 * </pre>
 * <p>Associated with each DistrictImpl is specific info. The
 * documentation for each DistrictImpl will describe the associated
 * info it needs. That data could be another object, it could be data
 * in a particular struct, it might be a NULL pointer. Check the
 * documentation for each DistrictImpl for a description of the data
 * and its required format.
 * <p>To use this function decide which DistrictImpl you want to use,
 * then determine what information that DistrictImpl needs and in which
 * format it is presented. Collect the data in the appropriate format
 * then call this function passing in the desired DistrictImpl and the
 * required info. The associated info must be cast to Pointer.
 * <p>For example, if the required info is a VtItem struct, declare a
 * variable to be of type VtItem, fill in the fields, then pass the address
 * of that struct cast to Pointer.
 * <p>The input distObj is a pointer to an object. It should point to
 * a NULL VtDistrictObject. This function will go to the address given
 * and deposit a created object.
 * <p>If you call Create, you must call Destroy after you are done with
 * the object but before the program exits.
 * <p>Example:
 * <pre>
 * <code>
 *    int status;
 *    VtDistrictObject distObj = (VtDistrictObject)0;
 *
 *    do {
 *          . . .
 *      status = VtCreateDistrictObject (
 *        libCtx, VtDistrictImplBasic, (Pointer)0, &distObj);
 *      if (status != 0)
 *        break;
 *          . . .
 *    } while (0);
 *
 *    VtDestroyDistrictObject (&distObj);
 * </code>
 * </pre>
 *
 * @param libCtx The library context.
 * @param districtImpl The implementation the object will use.
 * @param associatedInfo The info needed by the DistrictImpl.
 * @param distObj A pointer to where the routine will deposit the
 * created object.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtCreateDistrictObject (
   VtLibCtx libCtx,
   VtDistrictImpl districtImpl,
   Pointer associatedInfo,
   VtDistrictObject *distObj
);

/* These are the VtDistrictImpls supported by the toolkit. Each
 * DistrictImpl is used in conjunction with special info for the function.
 * If there is no special info, the accompaniment is a NULL pointer.
 */

/** This VtDistrictImpl is used to load up the MpIntCtx the object will
 * use when dealing with district parameters, which contain IBE
 * parameters. For the toolkit to operate on IBE parameters, it must
 * have an MpIntCtx.
 * <p>If the associatedInfo passed in with this Impl is NULL, then the
 * Impl will try to find an mpCtx in the libCtx.
 * <p>If the object is set with specific parameters, this mpCtx will be
 * ignored, the one inside the parameter object passed in will be used.
 * <p>The data associated with VtDistrictImplMpCtx is a VtMpIntCtx.
 */
extern VtDistrictImpl VtDistrictImplMpCtx;

/** When building a district object, use this VtDistrictImpl if the
 * mpCtx desired is already loaded into the libCtx.
 * <p>A district object cannot operate without an mpCtx, so do not use
 * this Impl unless the libCtx contains an mpCtx.
 * <p>The data associated with VtDistrictImplBasic is a NULL pointer:
 * (Pointer)0.
 */
extern VtDistrictImpl VtDistrictImplBasic;

/** Destroy a District Object. This frees up any memory allocated
 * during the object's creation and use.
 * <p>If you call Create, you must call Destroy after you are done with
 * the object but before the program exits.
 * <p>Example:
 * <pre>
 * <code>
 *    int status;
 *    VtDistrictObject distObj = (VtDistrictObject)0;
 *
 *    do {
 *          . . .
 *      status = VtCreateDistrictObject (
 *        libCtx, VtDistrictImplBasic, (Pointer)0, &distObj);
 *      if (status != 0)
 *        break;
 *          . . .
 *    } while (0);
 *
 *    VtDestroyDistrictObject (&distObj);
 * </code>
 * </pre>
 * @param distObj A pointer to where the routine will find the object
 * to destroy.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtDestroyDistrictObject (
   VtDistrictObject *distObj
);

/** Set the district object with the information given.
 * <p>The VtDistrictParam defines what information the object will be
 * set with.
 * <p>The include file vibe.h defines the supported DistrictParams.
 * Look through the include file to see which DistrictParam to use for
 * your application. All supported DistrictParams will be defined as
 * in the following example.
 * <pre>
 * <code>
 *   extern VtDistrictParam VtDistrictParamDomainName;
 * </code>
 * </pre>
 * <p>Associated with each DistrictParam is specific info. The
 * documentation for each DistrictParam will describe the associated
 * info it needs. That data could be another object, it could be data
 * in a particular struct, it might be a NULL pointer. Check the
 * documentation for each DistrictParam for a description of the data
 * and its required format.
 * <p>To use this function decide which DistrictParam you want to use,
 * then determine what information that DistrictParam needs and in
 * which format it is presented. Collect the data in the appropriate
 * format then call this function passing in the desired DistrictParam
 * and the required info. The associated info must be cast to Pointer.
 * <p>For example, if the required info is a VtString struct,
 * declare a variable to be of type VtString, fill in the fields,
 * then pass the address of that struct cast to Pointer.
 * <p>Example:
 * <pre>
 * <code>
 *    unsigned char *distQualName =
 *      (unsigned char *)"sample.com#123456789";
 *
 *    do {
 *          . . .
 *      status = VtSetDistrictParam (
 *        distObj, VtDistrictParamQualifiedName, (Pointer)distQualName);
 *      if (status != 0)
 *        break;
 *          . . .
 *    } while (0);
 * </code>
 * </pre>
 *
 * @param distObj The object to set.
 * @param districtParam What the object is being set to.
 * @param associatedInfo The info needed by the DistrictParam.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtSetDistrictParam (
   VtDistrictObject distObj,
   VtDistrictParam districtParam,
   Pointer associatedInfo
);

/** Get the specified type of information out of a district object.
 * <p>This function will go to the address given by getInfo and
 * deposit a pointer. That pointer is the address of the data. The
 * memory to hold the information belongs to the object, do not free it.
 * <p>The VtDistrictParam will specify what kind of information will be
 * returned, the getInfo is where the function will deposit the info.
 * <p>The include file vibe.h defines the supported DistrictParams.
 * Look through the include file to see which DistrictParam to use for
 * your application.
 * <p>See also VtSetDistrictParam.
 * <p>To use this function decide which DistrictParam you want to use,
 * then determine what information that DistrictParam will return and
 * in which format it is presented. Declare a variable to be a pointer
 * to the appropriate type, then call this function passing in the
 * desired DistrictParam and the address of the variable. Cast the
 * address of the pointer to the appropriate type: (Pointer *).
 * <p>If the associated info is already a pointer type (such as an
 * object), declare a variable to be of the appropriate type and pass
 * the address of that variable cast to (Pointer *).
 * <p>For example, if the returned info is a VtString, declare a
 * variable to be of type (VtString *), pass the address of that
 * variable (&varName) cast to (Pointer *).
 * <p>Example:
 * <pre>
 * <code>
 *    unsigned char *domainName;
 *
 *    do {
 *          . . .
 *      status = VtGetDistrictParam (
 *        distObj, VtDistrictParamDomainName, (Pointer *)&domainName);
 *      if (status != 0)
 *        break;
 *          . . .
 *    } while (0);
 * </code>
 * </pre>
 *
 * @param distObj The object to query.
 * @param districtParam Defines what info to get.
 * @param getInfo The address of a pointer. The function will go to
 * that address and deposit a pointer to the info requested.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtGetDistrictParam (
   VtDistrictObject distObj,
   VtDistrictParam districtParam,
   Pointer *getInfo
);

/* These are the VtDistrictParams supported by the toolkit. Each
 * DistrictParam is used in conjunction with special info for the
 * function. If there is no special info, the accompaniment is a NULL
 * pointer.
 */

/** SetParam and GetParam.
 * <p>This VtDistrictParam is used to set a district object with the
 * domain name of the district.
 * <p>Or it is used to get the district name out of a district object.
 * <p>The associated info is a UTF-8 string
 * <p>The name must be a UTF-8 string (an ASCII string is already a
 * UTF-8 string, so that is automatically valid input). The string must
 * be NULL-terminated.
 * <p>UTF-8 is a way to encode all possible Unicode characters into a
 * byte array. Any character can be represented as a one-, two-,
 * three-, four-, five-, or six-byte sequence. They are encoded in such
 * a way that the decoder will know whether a character is encoded as
 * one, two, three, four, five, or six bytes. All characters are
 * non-zero. That means a UTF-8 string can be NULL-terminated. The
 * NULL-terminating character is a single 00 byte.
 * <p>When setting, build the UTF-8 string and pass it as the
 * associatedInfo.
 * <p>For example,
 * <pre>
 * <code>
 *   unsigned char *domainName = (unsigned char *)"sample.com";
 *
 *   status = VtSetDistrictParam (
 *     distObj, VtDistrictParamDomainName, (Pointer)domainName);
 * </code>
 * </pre>
 * <p>When getting, pass in the address of an unsigned char array
 * (unsigned char *) variable, the Get function will deposit a pointer
 * at the address. The pointer points to a UTF-8 string.
 */
extern VtDistrictParam VtDistrictParamDomainName;

/** SetParam and GetParam.
 * <p>This VtDistrictParam is used to set a district object with the
 * fully-qualified name of the district.
 * <p>Or it is used to get the qualified name out of a district object.
 * <p>The associated info is a UTF-8 string
 * <p>The name must be a UTF-8 string (an ASCII string is already a
 * UTF-8 string, so that is automatically valid input). The string must
 * be NULL-terminated.
 * <p>UTF-8 is a way to encode all possible Unicode characters into a
 * byte array. Any character can be represented as a one-, two-,
 * three-, four-, five-, or six-byte sequence. They are encoded in such
 * a way that the decoder will know whether a character is encoded as
 * one, two, three, four, five, or six bytes. All characters are
 * non-zero. That means a UTF-8 string can be NULL-terminated. The
 * NULL-terminating character is a single 00 byte.
 * <p>When setting, build the UTF-8 string and pass it as the
 * associatedInfo.
 * <p>For example,
 * <pre>
 * <code>
 *   unsigned char *qualifiedName =
 *     (unsigned char *)"sample.com#123456789";
 *
 *   status = VtSetDistrictParam (
 *     distObj, VtDistrictParamQualifiedName, (Pointer)qualifiedName);
 * </code>
 * </pre>
 * <p>When getting, pass in the address of an unsigned char array
 * (unsigned char *) variable, the Get function will deposit a pointer
 * at the address. The pointer points to a UTF-8 string.
 */
extern VtDistrictParam VtDistrictParamQualifiedName;

/** SetParam only.
 * <p>This VtDistrictParam is used to set a district object with the
 * base64 encoded parameters for the district.
 * <p>The district parameter must be a base64 encoded string. Because 
 * Base64 encoding always producses ASCII characters, the length of
 * an ASCII string can be detrmined easily if they are NULL terminated.
 * <p>The associated info is an NULL-terminated ASCII string.
 * <p>When setting, collect the district params into a buffer and pass
 * the buffer as the associatedInfo.
 */
extern VtDistrictParam VtDistrictParamBase64Params;

/** GetParam only.
 * <p>This VtDistrictParam returns the math parameters (prime, subprime,
 * etc.) in the district object.
 * <p>The associated info is a VtParameterObject object.
 * <p>When getting, pass in the address of a VtParameterObject
 * variable, the Get function will deposit an object there. The object
 * belongs to the district object out of which the getInfo came.
 */
extern VtDistrictParam VtDistrictParamIBEParameters;

/** GetParam only.
 * <p>This VtDistrictParam returns the extensions in a set of district
 * parameters.
 * <p>The associated info is a VtX509ExtensionList struct.
 * <p>When getting, pass in the address of a pointer as the getInfo,
 * the Get function will deposit a pointer to a VtX509ExtensionList
 * struct at that address. All memory belongs to the object out of
 * which the getInfo came.
 */
extern VtDistrictParam VtDistrictParamExtensions;

/** GetParam only.
 * <p>This VtDistrictParam returns the location of the policy server
 * for a given district. The location is returned as an URL which is 
 * a NULL terminated UTF-8 encoded string.
 * <p>The associated info is an unsigned char array.
 * <p>When getting, pass the address of a pointer as the getInfo, the
 * Get function will deposit a pointer to a buffer at that address.
 * <p>For example,
 * <pre>
 * <code>
 *  unsigned char *policyServer = (unsigned char *)0;
 *
 *  status = VtGetDistrictParam (
 *     distObj, VtDistrictParamPolicyServer, (Pointer *)&policyServer);
 * </code>
 * </pre>
 */
extern VtDistrictParam VtDistrictParamPolicyServer;

/** GetParam only.
 * <p>This VtDistrictParam returns the list of key schemas in a set of
 * district parameters, this will be the key schemas a district will
 * support.
 * <p>The associated info is a VtOidList struct.
 * <p>When getting, pass the address of a pointer as the getInfo, the
 * Get function will deposit a pointer to a VtOidList struct at that
 * address.
 */
extern VtDistrictParam VtDistrictParamSupportedKeySchemas;

/** GetParam only.
 * <p>This VtDistrictParam returns the DSA parameters (prime, subprime,
 * base) in the district object.
 * <p>A district will possess a set of district parameters. This
 * includes the IBE math parameters, supported key schemas, validity
 * dates, and so on. Some districts might also include a set of DSA
 * parameters. These are parameters to use when generating a new DSA
 * key pair to use when signing. By using specified parameters, you do
 * not need to generate a new parameter set. Generating new parameters
 * can be time-consuming, so using a district's parameter set can save
 * time.
 * <p>When you obtain a private IBE key from a district, you can also
 * obtain a signing cert. In order to get a signing cert, you must
 * generate a new key pair and build a cert request. To build a new DSA
 * key pair, you will need DSA parameters. Your app can either generate
 * new ones each time (a slow operation) or, after obtaining the
 * district parameters, you can try to get DSA params out of the
 * district object to use. If the district object has params, you can
 * use them. If not, generate your own.
 * <p>The associated info is a VtDSAParamInfo struct.
 * <p>When getting, pass the address of a pointer as the getInfo, the
 * Get function will deposit a pointer to a VtDSAParamInfo struct at
 * that address.
 * <p>If the district does not contain a set of DSA parameters, the Get
 * function will return the error VT_ERROR_GET_INFO_UNAVAILABLE.
 */
extern VtDistrictParam VtDistrictParamDSAParams;

/** GetParam only.
 * <p>This VtDistrictParam returns an array of trusted certs. When
 * downloading parameters or keys, the transport provider can load any
 * trusted certs (district certs, CA certs) into the district object.
 * These are certs that have been verified during the download process.
 * An application can store these in the storage provider or use them
 * for other cert verification operations on leaf certs.
 * <p>The associated info is a VtCertObjectList struct.
 * <p>When getting, pass the address of a pointer as the getInfo, the
 * Get function will deposit a pointer to a VtCertObjectList struct at
 * that address.
 */
extern VtDistrictParam VtDistrictParamTrustedCerts;

/** This data struct is used by VtX509ExtensionList.
 * <p>The oid will contain the OID value, not the tag and length.
 * <p>The critical field will be 0 for not critical (false) and 1 for
 * critical (true).
 * <p>The value will be the contents of the OCTET STRING (that is, the
 * OCTET STRING tag and length are not included), so that it is the
 * encoding of the value associated with the OID.
 */
typedef struct
{
  VtItem oid;
  unsigned int critical;
  VtItem value;
} VtX509Extension;

/** When called with VtDistrictParamExtensions, the function
 * VtGetDistrictParam will return a pointer to the following struct.
 */
typedef struct
{
  unsigned int count;
  VtX509Extension *extensions;
} VtX509ExtensionList;

/** When called with VtDistrictParamSupportedKeySchemas, the function
 * VtGetDistrictParam will return a pointer to the following struct.
 */
typedef struct
{
  unsigned int count;
  VtItem *oids;
} VtOidList;

/*@}*/

/*=========================================================*/
/*                                                         */
/* Identity Object                                         */
/*                                                         */
/*=========================================================*/

/**
 * \defgroup IdentityGroup Identity Object
 */

/*@{*/

/** The identity object.
 * <p>Note that the object is a pointer type.
 */
typedef struct VtIdentityObjectDef *VtIdentityObject;

/** The function VtCreateIdentityObject builds an identity object using
 * a VtIdentityImpl. This typedef defines what a VtIdentityImpl is.
 * Although a VtIdentityImpl is a function pointer, an application should
 * never call one directly, only pass it as an argument to
 * VtCreateIdentityObject.
 */
typedef int VT_CALLING_CONV (VtIdentityImpl) (
   VtIdentityObject *, Pointer, unsigned int);

/** The functions VtSetIdentityParam and VtGetIdentityParam add or
 * get information to or from an identity object. The information to
 * add or get is defined by a VtIdentityParam. This typedef defines
 * what a VtIdentityParam is. Although a VtIdentityParam is a function
 * pointer, an application should never call one directly, only pass it
 * as an argument to VtSetIdentityParam or VtGetIdentityParam.
 */
typedef int VT_CALLING_CONV (VtIdentityParam) (
   VtIdentityObject, Pointer, unsigned int);

/** Forward referencing, used by VtIdentitySchemaDecode to pass an
 * encoded identity object to a decoder. An encoded identity is the DER
 * encoding of an ASN.1 definition. The Asn1Identity contains the
 * encoded identity as an ASN.1 object rather than a byte array.
 * Applications can ignore this data type.
 */
typedef struct Asn1IdentityDef Asn1Identity;

/** When calling VtDecodeIdentity, an application will pass in an array
 * of decoders. Each decoder knows how to decode a particular identity
 * schema.
 * <p>This typedef defines what a VtIdentitySchemaDecode is. Although
 * it is a function pointer, an application should never call a Decoder
 * directly, only pass it as an argument to VtDecodeIdentity.
 */
typedef int VT_CALLING_CONV (VtIdentitySchemaDecode) (
   VtIdentityObject, Asn1Identity *, unsigned int);

/** Create a new identity object. This allocates space for an "empty"
 * object, then loads the given IdentityImpl to make it an "active"
 * object.
 * <p>The VtIdentityImpl defines some of the identity object operations.
 * The include file vibe.h defines the supported IdentityImpls. Look
 * through the include file to see which IdentityImpl to use for your
 * application. All supported IdentityImpls will be defined as in the
 * following example.
 * <pre>
 * <code>
 *   extern VtIdentityImpl VtIdentityImplMpCtx;
 * </code>
 * </pre>
 * <p>Associated with each IdentityImpl is specific info. The
 * documentation for each IdentityImpl will describe the associated
 * info it needs. That data could be another object, it could be data
 * in a particular struct, it might be a NULL pointer. Check the
 * documentation for each IdentityImpl for a description of the data
 * and its required format.
 * <p>To use this function decide which IdentityImpl you want to use,
 * then determine what information that IdentityImpl needs and in which
 * format it is presented. Collect the data in the appropriate format
 * then call this function passing in the desired IdentityImpl and the
 * required info. The associated info must be cast to Pointer.
 * <p>For example, if the required info is a VtItem struct, declare a
 * variable to be of type VtItem, fill in the fields, then pass the address
 * of that struct cast to Pointer.
 * <p>The input idObj is a pointer to an object. It should point to a
 * NULL VtIdentityObject. This function will go to the address given
 * and deposit a created object.
 * <p>If you call Create, you must call Destroy after you are done with
 * the object but before the program exits.
 * <p>Example:
 * <pre>
 * <code>
 *    int status;
 *    VtIdentityObject idObj = (VtIdentityObject)0;
 *
 *    do {
 *          . . .
 *      status = VtCreateIdentityObject (
 *        libCtx, VtIdentityImplBasic, (Pointer)0, &idObj);
 *      if (status != 0)
 *        break;
 *          . . .
 *    } while (0);
 *
 *    VtDestroyIdentityObject (&idObj);
 * </code>
 * </pre>
 *
 * @param libCtx The library context.
 * @param identityImpl The implementation the object will use.
 * @param associatedInfo The info needed by the IdentityImpl.
 * @param idObj A pointer to where the routine will deposit the
 * created object.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtCreateIdentityObject (
   VtLibCtx libCtx,
   VtIdentityImpl identityImpl,
   Pointer associatedInfo,
   VtIdentityObject *idObj
);

/* These are the VtIdentityImpls supported by the toolkit. Each
 * IdentityImpl is used in conjunction with special info for the function.
 * If there is no special info, the accompaniment is a NULL pointer.
 */

/** This VtIdentityImpl is used to load up the MpIntCtx the object will
 * use when dealing with district parameters, which contain IBE
 * parameters. For the toolkit to operate on IBE parameters, it must
 * have an MpIntCtx.
 * <p>If the associatedInfo passed in with this Impl is NULL, then the
 * Impl will try to find an mpCtx in the libCtx.
 * <p>If the object is set with a specific district object, this mpCtx
 * will be ignored, the one inside the district object passed in will be
 * used.
 * <p>The data associated with VtIdentityImplMpCtx is a VtMpIntCtx.
 */
extern VtIdentityImpl VtIdentityImplMpCtx;

/** When building an identity object, use this VtIdentityImpl if the
 * mpCtx desired is loaded into the libCtx.
 * <p>An identity object cannot operate without an mpCtx, so do not use
 * this Impl unless the libCtx contains an mpCtx.
 * <p>The data associated with VtIdentityImplBasic is a NULL pointer:
 * (Pointer)0.
 */
extern VtIdentityImpl VtIdentityImplBasic;

/** Destroy an Identity Object. This frees up any memory allocated
 * during the object's creation and use.
 * <p>If you call Create, you must call Destroy after you are done with
 * the object but before the program exits.
 * <p>Example:
 * <code>
 * <pre>
 *    int status;
 *    VtIdentityObject idObj = (VtIdentityObject)0;
 *
 *    do {
 *          . . .
 *      status = VtCreateIdentityObject (
 *        libCtx, VtIdentityImplBasic, (Pointer)0, &idObj);
 *      if (status != 0)
 *        break;
 *          . . .
 *    } while (0);
 *
 *    VtDestroyIdentityObject (&idObj);
 * </pre>
 * </code>
 * @param idObj A pointer to where the routine will find the object
 * to destroy.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtDestroyIdentityObject (
   VtIdentityObject *idObj
);

/** Set the identity object with the information given.
 * <p>The VtIdentityParam defines what information the object will be
 * set with.
 * <p>The include file vibe.h defines the supported IdentityParams.
 * Look through the include file to see which IdentityParam to use for
 * your application. All supported IdentityParams will be defined as
 * in the following example.
 * <code>
 * <pre>
 *   extern VtIdentityParam VtIdentityParam822Email;
 * </pre>
 * </code>
 * <p>Associated with each IdentityParam is specific info. The
 * documentation for each IdentityParam will describe the associated
 * info it needs. That data could be another object, it could be data
 * in a particular struct, it might be a NULL pointer. Check the
 * documentation for each IdentityParam for a description of the data
 * and its required format.
 * <p>To use this function decide which IdentityParam you want to use,
 * then determine what information that IdentityParam needs and in
 * which format it is presented. Collect the data in the appropriate
 * format then call this function passing in the desired IdentityParam
 * and the required info. The associated info must be cast to Pointer.
 * <p>For example, if the required info is a VtString struct,
 * declare a variable to be of type VtString, fill in the fields,
 * then pass the address of that struct cast to Pointer.
 *
 * @param idObj The object to set.
 * @param identityParam What the object is being set to.
 * @param associatedInfo The info needed by the IdentityParam.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtSetIdentityParam (
   VtIdentityObject idObj,
   VtIdentityParam identityParam,
   Pointer associatedInfo
);

/** Get the specified type of information out of an identity object.
 * <p>This function will go to the address given by getInfo and
 * deposit a pointer. That pointer is the address of the data. The
 * memory to hold the information belongs to the object, do not free it.
 * <p>The VtIdentityParam will specify what kind of information will be
 * returned, the getInfo is where the function will deposit the info.
 * <p>The include file vibe.h defines the supported IdentityParams.
 * Look through the include file to see which IdentityParam to use for
 * your application.
 * <p>See also the documentation for VtSetIdentityParam.
 * <p>To use this function decide which IdentityParam you want to use,
 * then determine what information that IdentityParam will return and
 * in which format it is presented. Declare a variable to be a pointer
 * to the appropriate type, then call this function passing in the
 * desired IdentityParam and the address of the variable. Cast the
 * address of the pointer to the appropriate type: (Pointer *).
 * <p>If the associated info is already a pointer type (such as an
 * object), declare a variable to be of the appropriate type and pass
 * the address of that variable cast to (Pointer *).
 * <p>For example, if the returned info is a VtDistrictObject, declare
 * a variable to be of type VtDistrictObject, pass the address of that
 * variable (&varName) cast to (Pointer *).
 * <p>Example:
 * <code>
 * <pre>
 *    VtDistrictObject distObj;
 *
 *    do {
 *          . . .
 *      status = VtGetIdentityParam (
 *        idObj, VtIdentityParamDistrict, (Pointer *)&distObj);
 *      if (status != 0)
 *        break;
 *          . . .
 *    } while (0);
 * </pre>
 * </code>
 * @param idObj The object to query.
 * @param identityParam Defines what info to get.
 * @param getInfo The address of a pointer. The function will go to
 * that address and deposit a pointer to the info requested.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtGetIdentityParam (
   VtIdentityObject idObj,
   VtIdentityParam identityParam,
   Pointer *getInfo
);

/* These are the IdentityParams supported by the toolkit. Each
 * IdentityParam is used in conjunction with special info for the
 * function. If there is no special info, the accompaniment is a NULL
 * pointer.
 */

/** SetParam and GetParam.
 * <p>This VtIdentityParam is used to set an identity object with an
 * email address as the identity. For this schema, the identity is made
 * up of more than just the email address, it also includes a time.
 * <p>Or it is used to get the email info out of the identity object.
 * <p>The associated info is a VtEmailInfo struct.
 * <p>When setting, build the VtEmailInfo struct with the email address
 * and a time. Then pass the address of that struct as the
 * associatedInfo.
 * <p>When getting, pass in the address of a pointer as the getInfo,
 * the Get function will deposit a VtEmailInfo pointer at the address.
 * <p>The email address must be UTF-8 string (an ASCII string is
 * already a UTF-8 string).
 * <p>The time in an encoded identity (the actual public key of IBE) is
 * actually a "notBefore" time. For example, an identity based on the
 * email address name@sample.com can be name@sample.com for the week of
 * Oct. 20, 2003. Then name@sample.com for the week of April 11, 2005
 * is a different identity. There will be different IBE private keys
 * associated with each of those two identities, even though they use
 * the same email address.
 * <p>Generally, an app will use a validity period, so that identities
 * don't change every second. For instance, name@sample.com at 3:00 GMT
 * Oct. 22, 2003, and name@sample.com at 17:30 GMT Oct. 25, 2003, can
 * use the same "notBefore" time (e.g. 0:00 GMT, Oct. 20, 2003). In
 * other words, the time associated with an identity is actually a
 * week, rather than a particular second. That allows a user to
 * download one private key every week, rather than one private key
 * every email message.
 * <p>Hence, the time in the encoded identity can be different from the
 * time given in the Param's associated info.
 * <p>When you Get this Param out of an object before the identity is
 * encoded, the time will be the same time originally input. When you
 * Get this param out of an object after the identity is encoded, the
 * time will be the notBefore time. That is, the input time is
 * converted by the encoder into a notBefore time. If there is a
 * notBefore time available, that is the time that will be returned.
 * <p>With this IdentityParam, the validity period is set to one week,
 * with the base time being 0:00 GMT Jan. 6, 2002. To use a different
 * base time and/or validity period, is VtIdentityParam822EmailValidity.
 */
extern VtIdentityParam VtIdentityParam822Email;

/** SetParam and GetParam.
 * <p>This VtIdentityParam is used to set an identity object with an
 * email address as the identity. For this schema, the identity is made
 * up of more than just the email address, it also includes a time. The
 * time is computed as a "notBefore" time based on the app-supplied
 * base time and vailidity period. The email address must be UTF-8
 * string (an ASCII string is already a UTF-8 string).
 * <p>Or it is used to get the email and validity info out of the
 * identity object.
 * <p>The associated info is a VtEmailValidityInfo struct.
 * <p>When setting, build the VtEmailValidityInfo struct with the email
 * address, an identity time, a base time, and a validity period (in
 * seconds). Then pass the address of that struct as the associatedInfo.
 * <p>When getting, pass in the address of a pointer as the getInfo,
 * the Get function will deposit a VtEmailValidityInfo pointer at the
 * address.
 * <p>The time in an encoded identity (the actual public key of IBE) is
 * actually a "notBefore" time. For example, an identity based on the
 * email address name@sample.com can be name@sample.com for the week of
 * Oct. 20, 2003. Then name@sample.com for the week of April 11, 2005
 * is a different identity. There will be different IBE private keys
 * associated with each of those two identities, even though they use
 * the same email address.
 * <p>Generally, an app will use a validity period, so that identities
 * don't change every second. For instance, name@sample.com at 3:00 GMT
 * Oct. 22, 2003, and name@sample.com at 17:30 GMT Oct. 25, 2003, can
 * use the same "notBefore" time (e.g. 0:00 GMT, Oct. 20, 2003). In
 * other words, the time associated with an identity is actually a
 * week, rather than a particular second. That allows a user to
 * download one private key every week, rather than one private key
 * every email message.
 * <p>Hence, the time in the encoded identity can be different from the
 * time given in the Param's associated info.
 * <p>With this IdentityParam, the validity period is
 * application-defined. The associated info contains (in addition to
 * the expected fields of email address and time) a field for a base
 * time and another field for validity period. The toolkit will
 * determine a notBefore time based on those values.
 * <p>The validity period is given in number of seconds. For example, a
 * validity period of one hour is 3600 seconds.
 * <p>When you Get this Param out of an object before the identity is
 * encoded, the email time will be the same time originally input. When
 * you Get this param out of an object after the identity is encoded,
 * the email time will be the notBefore time. That is, the input time
 * is converted by the encoder into a notBefore time. If there is a
 * notBefore time available, that is the time that will be returned.
 */
extern VtIdentityParam VtIdentityParam822EmailValidity;

/** GetParam only
 * <p>An identity is generally a collection of attributes, one of which
 * is time. Use this Param to get the time associated with an identity.
 * <p>This is available only from encoded identities. An identity
 * schema may "convert" an input time into a notBefore time. That is,
 * an identity can have a time of Monday, Jan. 3, 2005, 12:00 M GMT,
 * and that time is valid for one week. On Tuesday, Jan. 4, 2005, the
 * time associated with that identity is still the Monday time. In this
 * way, an identity changes once a week, not every second.
 * <p>When you Get this Param out of an object, you get the notBefore
 * time. Of course, if a notBefore time is not available (the identity
 * is not yet encoded), this Param will not be able to return anything.
 * <p>The associated info is a VtTime struct.
 * <p>When getting, pass in the address of a pointer as the getInfo,
 * the Get function will deposit a pointer to a VtTime struct at the
 * address.
 */
extern VtIdentityParam VtIdentityParamTime;

/** SetParam and GetParam.
 * <p>This VtIdentityParam is used to set an identity object with a
 * district. In many operations, the toolkit will determine a district
 * based on an identity. But if an identity object is already set with
 * a district object, toolkit operations will not try to determine a
 * district, it will simply use the one loaded by this IdentityParam.
 * <p>Or it is used to get district information (represented as a
 * district object) out of an identity object.
 * <p>The associated info is a VtDistrictObject.
 * <p>When setting, build the VtDistrictObject and pass it as the
 * associatedInfo.
 * <p>When getting, pass in the address of a VtDistrictObject variable
 * as the getInfo, the Get function will deposit a district object at
 * the address.
 */
extern VtIdentityParam VtIdentityParamDistrict;

/** GetParam only
 * <p>Associated with an identity is a private key. Also associated
 * with it is a cert. The common name of the cert (part of the Name
 * element of the X.509 cert) is based on the identity.
 * <p>Use this VtIdentityParam to get the common name from an identity.
 * An identity object is set with a schema. Each schema will know what
 * part of the identity information is the common name. For example,
 * with the schema 822Email, there are two parts to the identity info,
 * the email address and the time. The schema knows that the common
 * name is the email address itself.
 * <p>The common name will be a "byte array". That is, when getting the
 * common name out of the ID object, this IdentityParam will simply
 * return the series of bytes that is the DirectoryString for the
 * commonName attribute.
 * <p>The associated info is a VtItem.
 * <p>When getting, pass in the address of a pointer as the getInfo,
 * the Get function will deposit a pointer to a VtItem at the address.
 */
extern VtIdentityParam VtIdentityParamCommonName;

/** GetParam only
 * <p>An identity can be used as a public key only when it has the
 * system parameters (prime, subprime, base point, public point). An
 * app may find it helpful to have these params (or possibly just the
 * prime length).
 * <p>The associated info is a VtBFType1IBEParamInfo struct.
 * <p>When getting, pass in the address of a pointer as the getInfo,
 * the Get function will deposit a pointer to a VtBFType1IBEParamInfo
 * struct at the address.
 */
extern VtIdentityParam VtIdentityParamIBEParams;

/** This is the data struct to use when setting an identity object to
 * 822Email (VtIdentityParam822Email).
 * <p>The email address must be a UTF-8 string (an ASCII string is
 * already a UTF-8 string, so that is automatically valid input). The
 * string must be NULL-terminated.
 * <p>UTF-8 is a way to encode all possible Unicode characters into a
 * byte array. Any character can be represented as a one-, two-,
 * three-, four-, five-, or six-byte sequence. They are encoded in such
 * a way that the decoder will know whether a character is encoded as
 * one, two, three, four, five, or six bytes. All characters are
 * non-zero. That means a UTF-8 string can be NULL-terminated. The
 * NULL-terminating character is a single 00 byte.
 * <p>If all the emailTime fields are set to 0, the toolkit will use
 * the current time.
 */
typedef struct
{
  unsigned char *emailAddress;
  VtTime emailTime;
} VtEmailInfo;

/** This is the data struct to use when setting an identity object to
 * 822Email with a specified base time and validity period
 * (VtIdentityParam822EmailValidity).
 * <p>The email address must be a UTF-8 string (an ASCII string is
 * already a UTF-8 string, so that is automatically valid input). The
 * string must be NULL-terminated.
 * <p>UTF-8 is a way to encode all possible Unicode characters into a
 * byte array. Any character can be represented as a one-, two-,
 * three-, four-, five-, or six-byte sequence. They are encoded in such
 * a way that the decoder will know whether a character is encoded as
 * one, two, three, four, five, or six bytes. All characters are
 * non-zero. That means a UTF-8 string can be NULL-terminated. The
 * NULL-terminating character is a single 00 byte.
 * <p>If all the emailTime fields are set to 0, the toolkit will use
 * the current time.
 * <p>The baseTime must be set and it must be before the email time.
 * <p>The validityPeriod is given in seconds (e.g. one hour is 3600
 * seconds, 30 days is 2,592,000 seconds).
 * <p>For convenience, the following #defines are given for common
 * validity periods.
 * <pre>
 * <code>
 *    VT_SECONDS_IN_ONE_HOUR
 *    VT_SECONDS_IN_ONE_DAY
 *    VT_SECONDS_IN_7_DAYS
 *    VT_SECONDS_IN_30_DAYS
 *    VT_SECONDS_IN_365_DAYS
 * </code>
 * </pre>
 * <p>Note that 30 days is not exactly a "Month", twelve 30-day periods
 * will equal 360 days. Also 365 days is not exactly a "year" because
 * some years are leap years, containing 366 days.
 * <p>The segmentCount is the number of segments into which the
 * validityPeriod is to be broken. This is so that not every email
 * address "rolls over" to a new identity time at the same time. For
 * example, a week will probably be broken into 7 segments, a day
 * probably 12 or 24. The maximum segmentCount is 52. If a week is
 * broken into 7 segments, about one seventh of email addresses will
 * change every Monday, another seventh every Tuesday and so on.
 */
typedef struct
{
  unsigned char *emailAddress;
  VtTime emailTime;
  VtTime baseTime;
  unsigned int validityPeriod;
  unsigned int segmentCount;
} VtEmailValidityInfo;

/** For use with VtEmailValidityInfo, this is the number of seconds in
 * one hour.
 */
#define VT_SECONDS_IN_ONE_HOUR   0x00000E10
/** For use with VtEmailValidityInfo, this is the number of seconds in
 * one day.
 */
#define VT_SECONDS_IN_ONE_DAY    0x00015180
/** For use with VtEmailValidityInfo, this is the number of seconds in
 * seven days (one week).
 */
#define VT_SECONDS_IN_7_DAYS     0x00093A80
/** For use with VtEmailValidityInfo, this is the number of seconds in
 * 30 days.
 */
#define VT_SECONDS_IN_30_DAYS    0x00278D00
/** For use with VtEmailValidityInfo, this is the number of seconds in
 * 365 days.
 */
#define VT_SECONDS_IN_365_DAYS   0x01E13380

/** Create the encoding of an identity. This "converts" an identity into
 * a byte array.
 * <p>If the identity has already been encoded, this function will not
 * re-encode, it will simply return the existing encoding.
 * <p>The version argument indicates which version of the IBCS #2
 * standard to use when encoding. Currently available are
 * <code>
 * <pre>
 *    VT_ENCODE_IBCS_2_V_1
 *    VT_ENCODE_IBCS_2_V_2
 *    VT_ENCODE_IBCS_2_V_DISTRICT
 * </pre>
 * </code>
 * <p>The version numbers (V_1 and V_2) specify using the encodings
 * outlined in IBCS #2. The district version (V_DISTRICT) specifies
 * using the encoding standard version given by the district. That is,
 * a district may specify a version number. If so, when using that
 * district to generate a public key, the sender (encryptor) should use
 * that standard. If the caller passes in V_DISTRICT and the district
 * does not specify a version, the function will use version 1 (V_1).
 * <p>You can logically OR the following value with the version.
 * <code>
 * <pre>
 *    VT_ENCODE_FOR_SIGNING
 * </pre>
 * </code>
 * <p>For example, you can set version to
 * <code>
 * <pre>
 *    VT_ENCODE_IBCS_2_V_DISTRICT | VT_ENCODE_FOR_SIGNING
 * </pre>
 * </code>
 * <p>This will indicate to the function that the caller wants to
 * create the encoding of an identity that will be used to send and
 * sign. Under the covers, the toolkit will obtain the cert and IBE
 * priovate key from a specific district. This may very well be the
 * same district if encoding regularly, but this guarantees that the
 * district will be local.
 * <p>When building a SecureMail (or ZDM, P7, etc.) message, the
 * toolkit will encode the sender's identity using the
 * ENCODE_FOR_SIGNING bit, but will encode the recipients using the
 * regular method.
 * <p>It is permitted to pass in no storage ctx (NULL).
 * <p> This routine will go to the address given by encodingLen and
 * deposit the length of the output (the number of bytes placed into
 * the encoding buffer). If the buffer is not big enough, this function
 * will return the "BUFFER_TOO_SMALL" error and set the unsigned int at
 * encodingLen to the needed size.
 *
 * @param idObj The identity to encode.
 * @param version The version of IBCS #2 to follow.
 * @param policyCtx The policy context to use, if needed.
 * @param storageCtx The storage ctx containing the storage providers
 * which the function will search and into which downloaded entries
 * will be stored.
 * @param transportCtx The transport context to use when contacting
 * districts, if necessary.
 * @param encoding The buffer into which the routine will place the
 * result.
 * @param bufferSize The size, in bytes, of the encoding buffer.
 * @param encodingLen The address where the routine will deposit the
 * resulting length, in bytes, of the encoding.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtEncodeIdentity (
   VtIdentityObject idObj,
   unsigned int version,
   VtPolicyCtx policyCtx,
   VtStorageCtx storageCtx,
   VtTransportCtx transportCtx,
   unsigned char *encoding,
   unsigned int bufferSize,
   unsigned int *encodingLen
);

/** For use with VtEncodeIdentity, this instructs the function to
 * follow the version of IBCS #2 the district uses. If the district
 * does no specify a version, use version 1.
 */
#define VT_ENCODE_IBCS_2_V_DISTRICT  255
/** For use with VtEncodeIdentity, this instructs the function to
 * follow version 1 of IBCS #2.
 */
#define VT_ENCODE_IBCS_2_V_1         1
/** For use with VtEncodeIdentity, this instructs the function to
 * follow version 2 of IBCS #2.
 */
#define VT_ENCODE_IBCS_2_V_2         2
/** For use with VtEncodeIdentity, this instructs the function that the
 * encoded identity will be used to sign. The function will use the
 * local district defined in the policy, if so defined. If not defined,
 * it will use the regular destrict determination process.
 */
#define VT_ENCODE_FOR_SIGNING        0x8000

/* Decoding Identities.
 */

/** Decode the district name from an encoded identity.
 * <p>An encoded identity (the public key) consists of a district (a
 * UTF8String) and a schema. This function returns the district name in
 * the given buffer. The routine will go to the address given by
 * districtNameLen and deposit the length of the output (the number of
 * bytes placed into the districtName buffer). If the buffer is not big
 * enough, this function will return the "BUFFER_TOO_SMALL" error and set
 * the unsigned int at districtNameLen to the needed size.
 * <p>The district name specified will be in UTF-8 form.
 *
 * @param libCtx The library context to use.
 * @param encoding The buffer containing the encoded identity.
 * @param encodingLen The length, in bytes, of the encoding.
 * @param districtName The buffer into which this function will place
 * the district name.
 * @param bufferSize The size, in bytes, of the output buffer.
 * @param districtNameLen The address where this routine will go to
 * deposit the length, in bytes, of the district name.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtDecodeIdentityDistrict (
   VtLibCtx libCtx,
   unsigned char *encoding,
   unsigned int encodingLen,
   unsigned char *districtName,
   unsigned int bufferSize,
   unsigned int *districtNameLen
);

/** Decode a byte array that is an encoded identity, a public key. The
 * caller passes in a created but empty (not-yet-set) identity object.
 * This function will set the object with the schema of the encoding.
 * <p>The function will also set the unsigned int at the address given
 * by the arrayIndex arg with the index into the decoder array of the
 * schema the identity actually is. The caller can pass a NULL pointer
 * for the arrayIndex argument and the routine will simply not return
 * that information.
 * <p>If the encoded ID is for a schema not represented in the array,
 * the function will return an error and the value at arrayIndex is
 * undefined.
 * <p>Once the encoding has been decoded, the caller can get information
 * out of the identity object using VtGetIdentityParam.
 *
 * @param encoding The byte array that is an encoded identity.
 * @param encodingLen The length, in bytes, of the encoding.
 * @param decoders An array of VtIdentitySchemaDecodes.
 * @param decoderCount The number of decoders in the array.
 * @param arrayIndex The address where the function will deposit the
 * index into the array of the schema the identity is.
 * @param idObj The identity object that will be set with the decoded
 * information.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtDecodeIdentity (
   unsigned char *encoding,
   unsigned int encodingLen,
   VtIdentitySchemaDecode **decoders,
   unsigned int decoderCount,
   unsigned int *arrayIndex,
   VtIdentityObject idObj
);

/* These are the VtIdentitySchemaDecodes supported by the toolkit.
 * If you add a SchemaDecode, add it to the list of
 * VT_ALL_SCHEMA_DECODES and increase the VT_ALL_SCHEMA_DECODE_COUNT.
 */

/** For use with VtDecodeIdentity, this decoder knows how to read an
 * 822 email schema.
 */
extern VtIdentitySchemaDecode VtIdentitySchemaDecode822Email;

/** Use this #define to help build a SchemaDecode array containing every
 * SchemaDecode supported in the toolkit.
 * <code>
 * <pre>
 *   VtIdentitySchemaDecode *allSchemaDecodes[VT_ALL_SCHEMA_DECODE_COUNT] =
 *     { VT_ALL_SCHEMA_DECODES };
 * </pre>
 * </code>
 */
#define VT_ALL_SCHEMA_DECODE_COUNT       1
/** Use this #define to help build a SchemaDecode array containing every
 * SchemaDecode supported in the toolkit.
 * <code>
 * <pre>
 *   VtIdentitySchemaDecode *allSchemaDecodes[VT_ALL_SCHEMA_DECODE_COUNT] =
 *     { VT_ALL_SCHEMA_DECODES };
 * </pre>
 * </code>
 */
#define VT_ALL_SCHEMA_DECODES         \
    VtIdentitySchemaDecode822Email

/** This is the data struct to accompany VtLibCtxParamSchemaDecodeArray.
 */
typedef struct
{
  unsigned int decoderCount;
  VtIdentitySchemaDecode **decoders;
} VtSchemaDecodeArray;

/*@}*/

/*=========================================================*/
/*                                                         */
/* Identity List                                           */
/*                                                         */
/*=========================================================*/

/**
 * \defgroup IdentityListGroup Identity List Object
 */

/*@{*/

/** The identity list object.
 * <p>Note that the object is a pointer type.
 */
typedef struct VtIdentityListDef *VtIdentityList;

/** The function VtCreateIdentityList builds an identity list using
 * a VtIdentityListImpl. This typedef defines what a VtIdentityListImpl
 * is. Although a VtIdentityListImpl is a function pointer, an
 * application should never call one directly, only pass it as an
 * argument to VtCreateIdentityList.
 */
typedef int VT_CALLING_CONV (VtIdentityListImpl) (
   VtIdentityList *, Pointer, unsigned int);

/** Create a new IdentityList. This allocates space for an "empty"
 * object, then loads the given IdentityListImpl to make it an "active"
 * object.
 * <p>The VtIdentityListImpl defines some of the identity list
 * operations. The include file vibe.h defines the supported
 * IdentityListImpls. Look through the include file to see which
 * IdentityListImpl to use for your application. All supported
 * IdentityListImpls will be defined as in the following example.
 * <code>
 * <pre>
 *   extern VtIdentityListImpl VtIdentityListImplBasic;
 * </pre>
 * </code>
 * <p>Associated with each IdentityListImpl is specific info. The
 * documentation for each IdentityListImpl will describe the associated
 * info it needs. That data could be another object, it could be data
 * in a particular struct, it might be a NULL pointer. Check the
 * documentation for each IdentityListImpl for a description of the data
 * and its required format.
 * <p>To use this function decide which IdentityListImpl you want to use,
 * then determine what information that IdentityListImpl needs and in which
 * format it is presented. Collect the data in the appropriate format
 * then call this function passing in the desired IdentityListImpl and the
 * required info. The associated info must be cast to Pointer.
 * <p>For example, if the required info is a VtItem struct, declare a
 * variable to be of type VtItem, fill in the fields, then pass the
 * address of that struct cast to Pointer.
 * <p>The input idList is a pointer to an object. It should point to
 * a NULL VtIdentityList. This function will go to the address given
 * and deposit a created object.
 * <p>If you call Create, you must call Destroy after you are done with
 * the list but before the program exits.
 * <p>Example:
 * <code>
 * <pre>
 *    int status;
 *    VtIdentityList idList = (VtIdentityList)0;
 *
 *    do {
 *          . . .
 *      status = VtCreateIdentityList (
 *        libCtx, VtIdentityListImplBasic, (Pointer)0, &idList);
 *      if (status != 0)
 *        break;
 *          . . .
 *    } while (0);
 *
 *    VtDestroyIdentityList (&idList);
 * </pre>
 * </code>
 * @param libCtx The library context.
 * @param identityImpl The implementation the object will use.
 * @param associatedInfo The info needed by the IdentityListImpl.
 * @param idList A pointer to where the routine will deposit the
 * created object.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtCreateIdentityList (
   VtLibCtx libCtx,
   VtIdentityListImpl identityImpl,
   Pointer associatedInfo,
   VtIdentityList *idList
);

/* These are the VtIdentityListImpls supported by the toolkit. Each
 * IdentityListImpl is used in conjunction with special info for the
 * function. If there is no special info, the accompaniment is a NULL
 * pointer.
 */

/** This VtIdentityListImpl is used to load up the MpIntCtx the object
 * will use when dealing with district parameters, which contain IBE
 * parameters. For the toolkit to operate on IBE parameters, it must
 * have an MpIntCtx.
 * <p>If the associatedInfo passed in with this Impl is NULL, then the
 * Impl will try to find an mpCtx in the libCtx.
 * <p>If an identity object added to the list already possesses an
 * mpCtx, this mpCtx will be ignored, the one inside the identity object
 * passed in will be used.
 * <p>The data associated with VtIdentityListImplMpCtx is a VtMpIntCtx.
 */
extern VtIdentityListImpl VtIdentityListImplMpCtx;

/** When building an identity list object, use this VtIdentityListImpl
 * if the mpCtx desired is loaded into the libCtx.
 * <p>An identity list cannot operate without an mpCtx, so do not use
 * this Impl unless the libCtx contains an mpCtx.
 * <p>The data associated with VtIdentityListImplBasic is a NULL
 * pointer: (Pointer)0.
 */
extern VtIdentityListImpl VtIdentityListImplBasic;

/** Destroy an Identity List. This frees up any memory allocated
 * during the object's creation and use.
 * <p>If you call Create, you must call Destroy after you are done with
 * the list but before the program exits.
 * <p>Example:
 * <code>
 * <pre>
 *    int status;
 *    VtIdentityList idList = (VtIdentityList)0;
 *
 *    do {
 *          . . .
 *      status = VtCreateIdentityList (
 *        libCtx, VtIdentityListImplBasic, (Pointer)0, &idList);
 *      if (status != 0)
 *        break;
 *          . . .
 *    } while (0);
 *
 *    VtDestroyIdentityList (&idList);
 * </pre>
 * </code>
 * @param idList A pointer to where the routine will find the object
 * to destroy.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtDestroyIdentityList (
   VtIdentityList *idList
);

/** Add the given identity object to the identity list.
 * <p>If the listIndex arg is not NULL, the function will set the
 * unsigned int at that address to the index inside the identity list
 * object of the newly added identity. So long as the identity list
 * exists, and the identity is not removed from the list, that
 * identity's index will never change, even if other identities are
 * removed from the list.
 * <p>This function will clone the identity object. That is, the
 * identity objects inside the identity list are "independent" of the
 * object passed in by the caller. Changes made to the caller-owned
 * object will not be reflected in the identity object inside the
 * identity list.
 *
 * @param idList The list to which the identity is added.
 * @param idObj The identity object containing the identity to add.
 * @param listIndex If NULL, the argument is ignored. If not NULL, it
 * is the address where the routine will deposit the index inside the
 * identity list object of the identity.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtAddIdObjectToIdentityList (
   VtIdentityList idList,
   VtIdentityObject idObj,
   unsigned int *listIndex
);

/** Add the given identity info to the identity list. This function
 * takes in a VtIdentityParam and info, rather than an identity
 * object. It is a convenience function, there is no need to create and
 * set an identity object, this function will create an internal id
 * object.
 * <p>If the listIndex arg is not NULL, the function will set the
 * unsigned int at that address to the index inside the identity list
 * object of the newly added identity. So long as the identity list
 * exists, and the identity is not removed from the list, that
 * identity's index will never change, even if other identities are
 * removed from the list.
 * <p>The VtIdentityParam defines what the object will be set to. It
 * can be a name, parameters, or some other data the object can use to
 * perform its task.
 * <p>See the documentation for VtSetIdentityParam for more
 * information on IdentityParams.
 *
 * @param idList The List to which the identity is added.
 * @param identityParam What identity is being added.
 * @param associatedInfo The info needed by the IdentityParam.
 * @param listIndex If NULL, the argument is ignored. If not NULL, it
 * is the address where the routine will deposit the index inside the
 * identity list object of the identity.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtAddNewIdToIdentityList (
   VtIdentityList idList,
   VtIdentityParam identityParam,
   Pointer associatedInfo,
   unsigned int *listIndex
);

/** How many active identities are in the list?
 * <p>The function will set the unsigned int at the address count with
 * the number of active identities (see SetEntryStatus).
 * <p>The maxIndex is the index of the "last" active identity in the
 * list. If the count is 0, the maxIndex is meaningless.
 * <p>There are some number of entries in the list, some of them
 * active, some inactive. Each entry (active or inactive) has an index.
 * If there is an entry in the list with an index greater than the
 * returned maxIndex, it will be inactive.
 * <p>The identities are indexed 0 (zero) through maxIndex. Use these
 * indices for VtGetIdentityListIdentity.
 *
 * @param idList The list to query.
 * @param count The address where this function will deposit the count.
 * @param maxIndex The address where this function will deposit the
 * largest number that is an index for an active entry. If the count is
 * 0, the returned maxIndex is meaningless.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtGetIdentityListCount (
   VtIdentityList idList,
   unsigned int *count,
   unsigned int *maxIndex
);

/** Get the identity at the given listIndex out of the identity list.
 * The listIndex passed in must be the same index returned from the
 * call to AddToIdentityList.
 * <p>If there is no identity at that index, or the entry is inactive,
 * the function will return VT_ERROR_NO_ID_AT_INDEX. In order to get
 * all the active identities out of the identity list, therefore, call
 * VtGetIdentityListCount to get the maxIndex, then loop from 0 to
 * maxIndex calling VtGetIdentityListIdentity. If the return is 0, you
 * have an identity. If the return is VT_ERROR_NO_ID_AT_INDEX, you
 * don't have an identity, move on. Any other error should be handled.
 * <p>The function will go to the address given by identity and
 * deposit an object. The object belongs to the IdentityList, do not
 * destroy the object returned. That is, the object returned is a
 * reference to the object in the list, not a clone. Any changes made
 * to the returned object will be reflected inside the list.
 *
 * @param idList The list to query.
 * @param listIndex The index of the identity in the list to get.
 * @param idObj The address where this function will deposit a
 * reference to the identity at the specified index.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtGetIdentityListIdentity (
   VtIdentityList idList,
   unsigned int listIndex,
   VtIdentityObject *idObj
);

/** Set the status of the identity in the identity list to one of the
 * following.
 * <code>
 * <pre>
 *    VT_RECIPIENT_LIST_ENTRY_ACTIVE
 *    VT_RECIPIENT_LIST_ENTRY_INACTIVE
 * </pre>
 * </code>
 * <p>If an entry is set to INACTIVE, toolkit functions will treat that
 * identity entry as if it does not exist. The internal copy of the
 * identity is not changed or destroyed, the toolkit simply ignores it.
 * If the entry is already inactive, this function will do nothing and
 * return 0 (success). If the index is beyond the end of the list (no
 * entry for that index), this function returns an error. It would be
 * possible to logically say that setting a non-existent entry to
 * inactive is no error, but if setting it to inactive were possible,
 * then so would setting it to active.
 * <p>If an entry is set to ACTIVE, the toolkit will now recognize the
 * entry. If the entry is already active, this function will do nothing
 * and return 0 (success). If the index is beyond the end of the list
 * (no entry for that index, either active or inactive), the function
 * will return an error.
 *
 * @param idList The list with the entry to set.
 * @param listIndex The index of the identity to set.
 * @param newStatus The flag indicating to what status the entry should
 * be set.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtSetEntryStatusInIdentityList (
   VtIdentityList idList,
   unsigned int listIndex,
   unsigned int newStatus
);

/** For use with VtSetEntryStatusInIdentityList, indicate that an
 * entry should be made active.
 */
#define VT_RECIPIENT_LIST_ENTRY_ACTIVE    1
/** For use with VtSetEntryStatusInIdentityList, indicate that an
 * entry should be made inactive.
 */
#define VT_RECIPIENT_LIST_ENTRY_INACTIVE  0

/*@}*/

/*=========================================================*/
/*                                                         */
/* Password Manager and Username-Password Collector        */
/*                                                         */
/*=========================================================*/

/**
 * \defgroup PasswordMgrGroup Password Manager/Collector
 */

/*@{*/

/** The following is the definition of a password manager callback.
 * <p>A manager does more than collect, it can set or change a password
 * as well. It deals only with passwords, not usernames. It is
 * generally used in storage contexts that protect data with
 * password-based encryption, or use an "extra" password.
 * <p>When the toolkit needs a password, it will call whatever password
 * manager function is loaded (wherever it is to be loaded: in a data
 * struct, as an arg, etc.). An application will supply a function that
 * satisfies the definition of the password manager typedef, along with
 * local info for the toolkit to pass back to the manager function when
 * it is called.
 * <p>The toolkit will have possession of the appData (it will be
 * loaded along with the manager function). The toolkit will either
 * copy the data (if an AppDataCopy function is supplied) or copy a
 * reference (if no AppDataCopy is supplied). The appData might
 * contain a window handle to open a password collection window, it
 * might simply contain a buffer with the password. The appData is
 * whatever the password manager function needs to perform the tasks.
 * <p>The toolkit can call the manager to set, change, or collect a
 * password. It will pass a flag indicating what it wants.
 * <p>When a toolkit function needs a password, it will call whatever
 * function it was given, using the appData it has as the appData
 * arg to the manager function. It will also pass a flag indicating it
 * is collecting a password. The toolkit will also pass to the manager
 * function two addresses. The first is the address where the manager
 * function should deposit a pointer to the password. The second
 * address is where the manager will deposit the length of the
 * password. (The manager function also contains args for an old
 * password and length, but when collecting a password, those args are
 * ignored.)
 * <p>Note that the toolkit does not supply a buffer, only an address
 * where the manager deposits a pointer to memory owned by the manager.
 * The toolkit will not alter or free the memory holding the password.
 * <p>When the toolkit is done with the password, it will call the
 * manager again, this time with a purpose indicating it is "releasing"
 * the password. At that point, the manager can free that memory (if
 * it is indeed separate allocated memory) or it may keep the buffer
 * around in case the toolkit asks for the password again.
 * <p>The purpose argument is the toolkit's way of indicating what it
 * wants the password manager to do. This flag will be one of the
 * following.
 * <pre>
 * <code>
 *    VT_PASSWORD_MGR_PURPOSE_SET
 *    VT_PASSWORD_MGR_PURPOSE_CHANGE
 *    VT_PASSWORD_MGR_PURPOSE_CHANGE_RETRY
 *    VT_PASSWORD_MGR_PURPOSE_COLLECT
 *    VT_PASSWORD_MGR_PURPOSE_COLLECT_RETRY
 *    VT_PASSWORD_MGR_PURPOSE_RELEASE
 * </code>
 * </pre>
 * <p>When the purpose is SET, the toolkit will be setting up something
 * to hold password info (it may be storing the HMAC of a password in a
 * file, for instance). The password manager's job is to obtain and
 * return the password that the toolkit will set, and which will be used
 * in the future. (If SET, the oldPassword and oldPasswordLen args should
 * be ignored.)
 * <p>When the purpose is CHANGE, the manager's job is to obtain and
 * return the old password and a new one. The toolkit will confirm the
 * old password is correct and set the new one. When the purpose is
 * CHANGE_RETRY, the toolkit had called CHANGE, the manager returned an
 * old and new password, but the old password did not work, so now the
 * toolkit is asking the manager for another old password to try.
 * <p>If the purpose is SET or CHANGE, the new password can be NULL
 * (with 0 length), indicating that the toolkit should no longer use a
 * password. For example with Win32 storage, it is possible to use an
 * "extra" password (beyond the normal protection offerred by
 * CryptProtectData). If an extra password is being used, it is
 * possible to change to no extra password.
 * <p>When the purpose is COLLECT, the toolkit is asking the manager to
 * return the password. When the purpose is COLLECT_RETRY, the toolkit
 * had called COLLECT, the manager returned a password, but that
 * password did not work, so now the toolkit is asking the manager for
 * a new password to try. (If COLLECT or COLLECT_RETRY, the oldPassword
 * and oldPasswordLen args should be ignored.)
 * <p>When the purpose is RELEASE, the toolkit is letting the manager
 * know the toolkit has used the password it collected (any password,
 * old and new for setting or regular collection) and the manager can
 * free any memory or close any windows or do whatever it needs to do
 * to clean up. The password, passwordLen, oldPassword, and
 * oldPasswordLen args will contain the pointers and lengths the
 * manager returned to the toolkit. If RELEASE, the implementation must
 * return 0. The toolkit will call with RELEASE even if there had been
 * no successful SET, COLLECT, etc. call. That is, the toolkit will
 * call the manager to COLLECT, something goes wrong, there is an
 * error, but as part of the cleanup, the toolkit calls RELEASE. It is
 * the password manager's responsibility to know if the RELEASE is
 * necessary and perform operations if it is, and do nothing if not.
 * <p>The COLLECT_RETRY call will come after a COLLECT or another
 * COLLECT_RETRY call.
 * <p>The toolkit can call the password manager to COLLECT more than
 * once, if it needs a password again. However, it will always call
 * with RELEASE before asking to collect again. A manager might collect
 * the password from a user once, then keep it around in case the
 * toolkit requests it again.
 * <p>The return value from this call should be 0 for success, or
 * either VT_ERROR_MEMORY, VT_ERROR_PASSWORD_CALLBACK, or
 * VT_ERROR_INVALID_CALLBACK_INFO when the function could not complete
 * the task. If the error is not memory and the app wants to know more
 * than ERROR_PASSWORD_CALLBACK or INVALID_CALLBACK_INFO (e.g., there
 * was a maximum retry count and the user kept typing in the wrong
 * password and the max was reached), it should set something up in the
 * appData.
 * <p>After calling the password manager, the toolkit will check the
 * return value, if 0, it will continue, if not 0, it will discontinue
 * operations and return the error to the app. So any error number is
 * actually valid. However, most apps will probably find it easier to
 * manage and maintain if the callback returns only the MEMORY,
 * PASSWORD_CALLBACK, or INVALID_CALLBACK_INFO errors, and determine
 * error specifics outside the toolkit.
 *
 * @param libCtx The library context if the password manager can use it
 * (for example, to allocate and free memory).
 * @param appData The application-supplied local information passed
 * to the toolkit when the manager was originally loaded, now being
 * passed back to the manager function.
 * @param purpose A flag indicating what the toolkit is asking the
 * manager to do.
 * @param oldPassword The address where the manager is to deposit a
 * pointer to an old password if the purpose is CHANGE.
 * @param oldPasswordLen The address where the manager is to deposit
 * the length, in bytes, of the old password, if the purpose is CHANGE.
 * @param password The address where the manager is to deposit a
 * pointer to the password (new password if the purpose is CHANGE).
 * @param passwordLen The address where the manager is to deposit the
 * length, in bytes, of the password.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
typedef int (*VtPasswordManager) (
   VtLibCtx libCtx,
   Pointer appData,
   unsigned int purpose,
   unsigned char **oldPassword,
   unsigned int *oldPasswordLen,
   unsigned char **password,
   unsigned int *passwordLen
);

/** Possible values for the purpose argument of a password manager.
 */
#define VT_PASSWORD_MGR_PURPOSE_SET            0x0001
#define VT_PASSWORD_MGR_PURPOSE_CHANGE         0x0002
#define VT_PASSWORD_MGR_PURPOSE_CHANGE_RETRY   0x4002
#define VT_PASSWORD_MGR_PURPOSE_COLLECT        0x0004
#define VT_PASSWORD_MGR_PURPOSE_COLLECT_RETRY  0x4004
#define VT_PASSWORD_MGR_PURPOSE_RELEASE        0x8000

/** The following is the definition of a username-password collector
 * callback. The username and password are generally associated with an
 * identity. The toolkit will ask the collector to return a username
 * and password associated with a given identity.
 * <p>The collector cannot manage usernames and passwords, it can only
 * collect them and pass them on to the operation that needs them. It
 * will be used in cases where a username and an associated password
 * are required to perform some kind of operation, generally in
 * username/password based authentications.
 * <p>See the password manager can do more with passwords.
 * <p>When the toolkit needs a username and password, it will call 
 * whatever username-password collector function is loaded. An
 * application will supply a function that satisfies the definition of
 * the username-password collector typedef, along with the local info
 * for the toolkit to pass back to the collector function when it is
 * called.
 * <p>The toolkit will have possession of the appData (it will be
 * loaded along with the manager function). The toolkit will either
 * copy the data (if an AppDataCopy function is supplied) or copy a
 * reference (if no AppDataCopy is supplied). The appData might
 * contain a window handle to open a username-password collection
 * window, it might simply contain buffers with the username and
 * password. The appData is whatever the username-password collector
 * function needs to perform the tasks.
 * <p>The toolkit can call the username-password collector to collect
 * or release a username-password combination. It will pass a flag
 * indicating what it wants.
 * <p>When a toolkit function needs a username and password, it will
 * call whatever function it was given, using the appData it has as
 * the appData arg to the collector function. It will also pass a flag
 * indicating it is collecting a password. The toolkit will also pass
 * to the collector function four addresses. The first address is where
 * the collector function should deposit a pointer to the username. The
 * second address is where the collector function should deposit the
 * length of the username. The third pointer is the address where the
 * collector function will deposit the pointer to the password. The
 * fourth address is where the collector function will deposit length
 * of the password.
 * <p>Note that the toolkit does not supply buffers, only addresses
 * where the collector function deposits a pointer to memory owned by the 
 * collector function itself. The toolkit will not alter or free the
 * memory holding the password.
 * <p>When the toolkit is done with the username and password, it will
 * call the collector function again, this time with a purpose
 * indicating it is "releasing" the username and password. At that
 * point, the collector can free that memory (if it is indeed separate
 * allocated memory) or it may keep the buffers around in case the
 * toolkit asks for the username and password again.
 * <p>The purpose argument is the toolkit's way of indicating what it
 * wants the password collector to do. This flag will be one of the
 * following.
 * <pre>
 * <code>    
 *    VT_USER_PASS_COLLECTOR_PURPOSE_COLLECT
 *    VT_USER_PASS_COLLECTOR_PURPOSE_COLLECT_RETRY
 *    VT_USER_PASS_COLLECTOR_PURPOSE_RELEASE
 * </code>
 * </pre> 
 * <p>When the purpose is COLLECT, the toolkit is asking the collector to
 * return the username and the associated password for a given identity.
 * <p>When the purpose is COLLECT_RETRY, the toolkit had called
 * COLLECT, the collector returned a username and password, but that
 * combination did not work, so now the toolkit is asking the collector
 * to try again. This can be helpful to keep track of how many times
 * the username-password has been requested and not allow requests
 * after certain number of tries.
 * <p>When the purpose is RELEASE, the toolkit is letting the collector 
 * function know that toolkit has used the username and password it 
 * collected and the collector can free any memory or close any windows
 * or do whatever it needs to do to clean up. The username,
 * usernameLen, password, and passwordLen args will contain the pointers
 * and lengths the collector returned to the toolkit. If RELEASE, the
 * implementation must return 0. The toolkit will call with RELEASE even
 * if there had been no successful COLLECT. That is, the toolkit might
 * call the collector to COLLECT, but something goes wrong, there is an
 * error, and as part of the cleanup, the toolkit calls RELEASE. It is
 * the password collector's responsibility to know if the RELEASE is
 * necessary and perform operations if it is, or do nothing if not.
 * <p>The COLLECT_RETRY call will come after a COLLECT or another
 * COLLECT_RETRY call.
 * <p>The toolkit can call the username-password collector to COLLECT
 * more than once, if it needs a username and password again. However,
 * it will always call with RELEASE before asking to collect again. A
 * username-password collector might collect the username and password
 * from a user once, then keep it around in case the toolkit requests
 * it again.
 * <p>The return value from this call should be 0 for success, or either 
 * VT_ERROR_MEMORY, VT_ERROR_PASSWORD_CALLBACK, or 
 * VT_ERROR_INVALID_CALLBACK_INFO when the function could not complete
 * the task. If the error is not memory and the app wants to know more
 * than ERROR_PASSWORD_CALLBACK or INVALID_CALLBACK_INFO (e.g., there
 * was a maximum retry count and the user kept typing in the wrong
 * password and the max was reached), it should set something up in the
 * appData.
 * <p>After calling the username-password collector, the toolkit will
 * check the return value, if 0, it will continue, if not 0, it will
 * discontinue operations and return the error to the app. So any error
 * number is actually valid. However, most apps will probably find it
 * easier to manage and maintain if the callback returns only the
 * MEMORY, PASSWORD_CALLBACK, or INVALID_CALLBACK_INFO errors, and
 * determine error specifics outside the toolkit.
 *
 * @param libCtx The library context if the password collector can use it
 * (for example, to allocate and free memory).
 * @param appData The application-supplied local information passed
 * to the toolkit when the collector was originally loaded, now being
 * passed back to the collector function.
 * @param purpose A flag indicating what the toolkit is asking the
 * collector function to do.
 * @param idObj The identity object for which the username-password
 * collector function has been called.
 * @param username The address where the collector function is to deposit
 * a pointer to the username.
 * @param usernameLen The address where the collector is to deposit
 * the length, in bytes, of the username.
 * @param password The address where the password collector is to deposit a
 * pointer to the password.
 * @param passwordLen The address where the password collector is to deposit the
 * length, in bytes, of the password.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
typedef int (*VtUserPassCollector) (
  VtLibCtx libCtx,
  Pointer appData,
  VtIdentityObject idObj,
  unsigned int purpose,
  unsigned char **username,
  unsigned int *usernameLen,
  unsigned char **password,
  unsigned int *passwordLen
);

/** Possible values for the purpose argument of a password collector.
 */
#define VT_USER_PASS_COLLECTOR_PURPOSE_COLLECT        0x0004
#define VT_USER_PASS_COLLECTOR_PURPOSE_COLLECT_RETRY  0x4004
#define VT_USER_PASS_COLLECTOR_PURPOSE_RELEASE        0x8000

/** Part of password functions is appData. Because it is specific to
 * the app and the password function, the toolkit will not know how
 * to copy that data. Hence, part of the password ctx is a function
 * pointer that can copy the data. You can pass a NULL for the
 * AppDataCopy function and the toolkit will copy a reference to the
 * data. However, in such cases make sure that the data referenced will
 * be valid whenever it is used. In other words the data should not
 * live in a temporary memory that will be recycled.
 * <p>This typedef defines what an AppDataCopy function is.
 * <p>Upon loading the password ctx (VtSetParam), the toolkit will
 * either copy the appData pointer (if no AppDataCopy function is
 * supplied) or will call the AppDataCopy function and store the data
 * produced in the object's password ctx field.
 *
 * @param libCtx The library ctx to use (possibly for memory
 * allocation, for example).
 * @param appData The application-specific data to copy.
 * @param AppDataCopy The address where the function will deposit the
 * copy of the appData.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
typedef int VT_CALLING_CONV (*VtPasswordAppDataCopy) (
   VtLibCtx libCtx, Pointer appData, Pointer *copyOfAppData);

/** If an app supplies an AppDataCopy function, it must supply an
 * AppDataFree function as well. The toolkit will store the function
 * pointer given and call it when destroying the object.
 * <p>Note that there is no error return. An AppDataFree function
 * should be written in such a way that there is no possible error.
 *
 * @param VtLibCtx The library ctx to use (possibly for memory free,
 * for example).
 * @param appData The address where the function will find the pointer
 * to the appData to free.
 * @return None.
 */
typedef void VT_CALLING_CONV (*VtPasswordAppDataFree) (
   VtLibCtx libCtx, Pointer *appData);

/** A PasswordManager callback is simply a password function and
 * application-specific data.
 * <p>When building a password context, build the application-specific
 * data (It may simply be a UI handle, or it might be a data struct
 * containing multiple elements). If it is not already a pointer type,
 * cast it to Pointer and set the appData field to that pointer. If it
 * is not a pointer type (a struct or int, for example), set the
 * appData field to the address of the info cast to Pointer.
 * <p>You can also pass an AppDataCopy function so that when the toolkit
 * loads the password ctx into an object/ctx, it will copy the appData
 * itself, rather than a reference. However, you can also pass a NULL
 * for the AppDataCopy function, in which case the toolkit will only
 * copy a reference to the appData.
 * <p>If you include an AppDataCopy function, you must also include an
 * AppDataFree function. When the toolkit destroys the object, it will
 * call the AppDataFree function to free up any memory allocated by the
 * AppDataCopy function. If you supply an AppDataCopy function and do
 * not supply an AppDataFree function, the toolkit will not load the
 * password ctx and will return an error.
 * <p>When the toolkit calls the PasswordFunction, it will pass the
 * appData. Inside the password function, the code can then
 * dereference the appData Pointer to the appropriate type. 
 */
typedef struct
{
  VtPasswordManager       PasswordFunction;
  Pointer                 appData;
  VtPasswordAppDataCopy   AppDataCopy;
  VtPasswordAppDataFree   AppDataFree;
} VtPasswordManagerCallback;

/** A Username-Password Collector callback is simply a
 * username-password function and application-specific data.
 * <p>When building a username-password context, build the
 * application-specific data (It may simply be a UI handle, or it might
 * be a data struct containing multiple elements). If it is not already
 * a pointer type, cast it to Pointer and set the appData field to that
 * pointer. If it is not a pointer type (a struct or int, for example),
 * set the appData field to the address of the info cast to Pointer.
 * <p>You can also pass an AppDataCopy function so that when the toolkit
 * loads the username-password ctx into an object/ctx, it will copy the
 * appData itself, rather than a reference. However, you can also pass
 * a NULL for the AppDataCopy function, in which case the toolkit will
 * only copy a reference to the appData.
 * <p>If you include an AppDataCopy function, you must also include an
 * AppDataFree function. When the toolkit destroys the object, it will
 * call the AppDataFree function to free up any memory allocated by the
 * AppDataCopy function. If you supply an AppDataCopy function and do
 * not supply an AppDataFree function, the toolkit will not load the
 * username-password ctx and will return an error.
 * <p>When the toolkit calls the UserPassFunction, it will pass the
 * appData. Inside the password function, the code can then
 * dereference the appData Pointer to the appropriate type. 
 */
typedef struct
{
  VtUserPassCollector     UserPassFunction;
  Pointer                 appData;
  VtPasswordAppDataCopy   AppDataCopy;
  VtPasswordAppDataFree   AppDataFree;
} VtUserPassCollectorCallback;

/*@}*/

/*=========================================================*/
/*                                                         */
/* Multi-Precision Integer Arithmetic Context              */
/*                                                         */
/*=========================================================*/

/* The MpInt Context along with create, destroy and other functions
 * are declared in vibecrypto.h. Those functions are part of
 * vibecrypto.lib. The following are declarations of MpInt context
 * operations found in vibe.lib.
 */

/**
 * \defgroup MPIntGroup Multi-Precision Integer Arithmetic Context
 */

/*@{*/

/** This VtMpIntImpl is used to set an MpIntCtx with the Gnu
 * multi-precision (GMP) arithmetic implementation.
 * <p>The data associated with VtMpIntImplGMP is NULL pointer:
 * (Pointer)0.
 */
extern VtMpIntImpl VtMpIntImplGMP;

/** This VtMpIntImpl is used to set an MpIntCtx with the
 * multi-precision implementation in OpenSSL (bn).
 * <p>The data associated with VtMpIntImplOpenSSL is NULL pointer:
 * (Pointer)0.
 */
extern VtMpIntImpl VtMpIntImplOpenSSL;

/*@}*/

/*=========================================================*/
/*                                                         */
/* DER/BER Encode and Decode                               */
/*                                                         */
/*=========================================================*/

/**
 * \defgroup DERGroup DER/BER Encode and Decode
 */

/*@{*/

/** Forward referencing, this type is used by VtDerCoder to pass
 * information around. Applications can ignore this data type.
 */
typedef struct VtDerCoderInfoDef VtDerCoderInfo;

/** Several operations need to encode or decode algorithm IDs, keys,
 * or other DER constructs. These functions often know how to encode
 * or decode general ASN.1 definitions, but not the details. For
 * example, and algorithm ID consists of an OID and params. The params
 * are defined by the OID, for one alg they are NULL (05 00), for
 * another they are an OCTET STRING, and for stil another they are a
 * SEQUENCE of things. The encoding/decoding functions cannot know how
 * everything is encoded or decoded (every algorithm ID with every
 * parameter set, every key type, etc.). They do the general work and
 * let the VtDerCoders fill in the details.
 * <p>Although a VtDerCoder is a function pointer, an application
 * should never call one directly, only pass it (or an array of them)
 * as an argument to another function.
 */
typedef int VT_CALLING_CONV (VtDerCoder) (
   VtDerCoderInfo *, Pointer, unsigned int);

/** Create the DER encoding of an algorithm ID.
 * <p>The VtDerCoder will specify what Alg ID is to be returned.
 * <p>The include file vibe.h defines the supported DerCoders. Look
 * through the include file to see which DerCoder to use for your
 * application. All supported DerCoders will be defined as in the
 * following example.
 * <code>
 * <pre>
 *   extern VtDerCoder VtDerCoderAES128CBC;
 * </pre>
 * </code>
 * <p>Associated with each DerCoder is specific info. The documentation
 * for each DerCoder will describe the associated info it needs. That
 * data could be another object, it could be data in a particular
 * struct, it might be a NULL pointer. Check the documentation for each
 * DerCoder for a description of the data and its required format.
 * <p>To use this function decide which DerCoder you want to use, then
 * determine what information that DerCoder needs and in which format it
 * is presented. Collect the data in the appropriate format then call
 * this function passing in the desired DerCoder and the required info.
 * The associated info must be cast to Pointer.
 * <p>The encoded Alg ID will be deposited into the buffer given by the
 * Alg ID arg.
 * <p>This routine will go to the address given by algIdLen and deposit
 * the length of the output (the number of bytes placed into the algId
 * buffer). If the buffer is not big enough, this function will return
 * the "BUFFER_TOO_SMALL" error and set the unsigned int at algIdLen to
 * the needed size.
 *
 * @param libCtx The library context to use.
 * @param derCoder The VtDerCoder that will do the encoding.
 * @param associatedInfo The info the DerCoder needs.
 * @param algId The buffer into which the DER encoding will be placed.
 * @param bufferSize The size, in bytes, of the output buffer.
 * @param algIdLen The address where the routine will deposit the
 * encoding length (in bytes).
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtDerEncodeAlgId (
   VtLibCtx libCtx,
   VtDerCoder derCoder,
   Pointer associatedInfo,
   unsigned char *algId,
   unsigned int bufferSize,
   unsigned int *algIdLen
);

/** Create the DER encoding of a key.
 * <p>The VtDerCoder will specify what encoded key is to be returned.
 * <p>The include file vibe.h defines the supported DerCoders. Look
 * through the include file to see which DerCoder to use for your
 * application. All supported DerCoders will be defined as in the
 * following example.
 * <code>
 * <pre>
 *   extern VtDerCoder VtDerCoderBFType1IBEPrivateKey;
 * </pre>
 * </code>
 * <p>Associated with each DerCoder is specific info. The documentation
 * for each DerCoder will describe the associated info it needs. That
 * data could be another object, it could be data in a particular
 * struct, it might be a NULL pointer. Check the documentation for each
 * DerCoder for a description of the data and its required format.
 * <p>To use this function decide which DerCoder you want to use, then
 * determine what information that DerCoder needs and in which format it
 * is presented. Collect the data in the appropriate format then call
 * this function passing in the desired DerCoder and the required info.
 * The associated info must be cast to Pointer.
 * <p>The encoded key will be deposited into the buffer given by the
 * keyDer arg.
 * <p>This routine will go to the address given by keyDerLen and deposit
 * the length of the output (the number of bytes placed into the algId
 * buffer). If the buffer is not big enough, this function will return
 * the "BUFFER_TOO_SMALL" error and set the unsigned int at keyDerLen to
 * the needed size.
 *
 * @param libCtx The library context to use.
 * @param derCoder The VtDerCoder that will do the DER encoding.
 * @param associatedInfo The info the DerCoder needs.
 * @param keyDer The buffer into which the DER encoding will be placed.
 * @param bufferSize The size, in bytes, of the output buffer.
 * @param keyDerLen The address where the routine will deposit the
 * encoding length (in bytes).
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtDerEncodeKey (
   VtLibCtx libCtx,
   VtDerCoder derCoder,
   Pointer associatedInfo,
   unsigned char *keyDer,
   unsigned int bufferSize,
   unsigned int *keyDerLen
);

/** Create the DER encoding of parameters.
 * <p>The VtDerCoder will specify what encoded parameter set is to be
 * returned.
 * <p>The include file vibe.h defines the supported DerCoders. Look
 * through the include file to see which DerCoder to use for your
 * application. All supported DerCoders will be defined as in the
 * following example.
 * <code>
 * <pre>
 *   extern VtDerCoder VtDerCoderBFType1IBEParams;
 * </pre>
 * </code>
 * <p>Associated with each DerCoder is specific info. The documentation
 * for each DerCoder will describe the associated info it needs. That
 * data could be another object, it could be data in a particular
 * struct, it might be a NULL pointer. Check the documentation for each
 * DerCoder for a description of the data and its required format.
 * <p>To use this function decide which DerCoder you want to use, then
 * determine what information that DerCoder needs and in which format it
 * is presented. Collect the data in the appropriate format then call
 * this function passing in the desired DerCoder and the required info.
 * The associated info must be cast to Pointer.
 * <p>The encoded params will be deposited into the buffer given by the
 * paramDer arg.
 * <p>This routine will go to the address given by paramDerLen and
 * deposit the length of the output (the number of bytes placed into
 * the paramDer buffer). If the buffer is not big enough, this function
 * will return the "BUFFER_TOO_SMALL" error and set the unsigned int at
 * paramDerLen to the needed size.
 *
 * @param libCtx The library context to use.
 * @param derCoder The VtDerCoder that will do the DER encoding.
 * @param associatedInfo The info the DerCoder needs.
 * @param paramDer The buffer into which the DER encoding will be placed.
 * @param bufferSize The size, in bytes, of the output buffer.
 * @param paramDerLen The address where the routine will deposit the
 * encoding length (in bytes).
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtDerEncodeParams (
   VtLibCtx libCtx,
   VtDerCoder derCoder,
   Pointer associatedInfo,
   unsigned char *paramDer,
   unsigned int bufferSize,
   unsigned int *paramDerLen
);

/** This function will determine from the BER encoding what the
 * algorithm is. The encoding must be an algorithm ID or the DER
 * encoding of a key. The routine will go to the address given by
 * algorithm and deposit a flag indicating what algorithm is
 * represented by the encoding. The value of algorithm will be one of
 * the flags listed in vibe.h. They are the #defines that begin with
 * either VT_ALG_ID_ or VT_KEY_BER_ (for example,
 * VT_ALG_ID_BF_TYPE1_IBE_ENCRYPT or VT_KEY_BER_BF_TYPE1_IBE_PRI).
 * <p>The caller creates a list of VtDerCoder’s, the function will use
 * the list to determine the algorithm. If the BER encoding is for an
 * entity not covered in the list, the function will set algorithm to
 * VT_UNKNOWN_BER and return an error.
 * <p>The buffer encoding contains the entity to decode. Because the
 * entity to decode may be part of a larger encoding, the length of the
 * actual Alg ID or key may not be known. However, the length of the
 * larger encoding will likely be known. Hence, the maximum length of
 * the entity in question can be the length of the larger encoding
 * minus the number of bytes to the beginning of the entity. This
 * function will not return an error if the exact length of the Alg ID
 * or key is not known.
 *
 * @param libCtx The library context to use.
 * @param berEncoding The BER encoding to decode.
 * @param maxEncodingLen The maximum length of the BER encoding.
 * @param derCoders The caller-created list of DerCoders, presumably
 * the list of the possible things the caller expects to find. 
 * @param derCoderCount The length of the list, the number of
 * DerCoders in the list.
 * @param algorithm The address where the function will deposit a flag
 * indicating what algorithm or key is represented by the BER encoding.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtGetAlgorithmFromBer (
   VtLibCtx libCtx,
   unsigned char *berEncoding,
   unsigned int maxEncodingLen,
   VtDerCoder **derCoders,
   unsigned int derCoderCount,
   unsigned int *algorithm
);

/* The following is a list of flags returned by VtGetAlgorithmFromBer.
 * They are the algorithms for which there is an OID and the toolkit
 * knows what the OID is.
 */

#define VT_ALG_ID_MIN_IBE_CIPHER        0x0001
/** A flag returned by VtGetAlgorithmFromBer if the BER construct is
 * the Alg ID for IBE encryption.
 */
#define VT_ALG_ID_BF_TYPE1_IBE_ENCRYPT  0x0001

#define VT_ALG_ID_MAX_IBE_CIPHER        0x0001

/** A flag returned by VtGetAlgorithmFromBer if the BER construct is
 * the encoding of IBE parameters.
 */
#define VT_OID_BF_TYPE1_IBE_PARAMS      0x0021

/** A flag returned by VtGetAlgorithmFromBer if the BER construct is
 * the Alg ID for DSA with SHA-1 signing.
 */
#define VT_ALG_ID_DSA_SHA1_SIGN         0x0031

/** A flag returned by VtGetAlgorithmFromBer if the BER construct is
 * the Alg ID for SHA-1.
 */
#define VT_ALG_ID_SHA1                  0x0041
/** A flag returned by VtGetAlgorithmFromBer if the BER construct is
 * the Alg ID for SHA-256.
 */
#define VT_ALG_ID_SHA256                0x0042
/** A flag returned by VtGetAlgorithmFromBer if the BER construct is
 * the Alg ID for SHA-384.
 */
#define VT_ALG_ID_SHA384                0x0043
/** A flag returned by VtGetAlgorithmFromBer if the BER construct is
 * the Alg ID for SHA-512.
 */
#define VT_ALG_ID_SHA512                0x0044
/** A flag returned by VtGetAlgorithmFromBer if the BER construct is
 * the Alg ID for SHA-224.
 */
#define VT_ALG_ID_SHA224                0x0046
/** A flag returned by VtGetAlgorithmFromBer if the BER construct is
 * the Alg ID for MD5.
 */
#define VT_ALG_ID_MD5                   0x0045

/** A flag returned by VtGetAlgorithmFromBer if the BER construct is
 * the Alg ID for HMAC-SHA-1.
 */
#define VT_ALG_ID_HMAC_SHA1             0x0050
/** A flag returned by VtGetAlgorithmFromBer if the BER construct is
 * the Alg ID for HMAC-MD5.
 */
#define VT_ALG_ID_HMAC_MD5              0x0051

#define VT_ALG_ID_MIN_SYM_CIPHER        0x0066
/** A flag returned by VtGetAlgorithmFromBer if the BER construct is
 * the Alg ID for DES-CBC-Pad.
 */
#define VT_ALG_ID_DES_CBC_PAD         0x0066
/** A flag returned by VtGetAlgorithmFromBer if the BER construct is
 * the Alg ID for TripleDES-EDE-CBC-Pad.
 */
#define VT_ALG_ID_3DES_EDE_CBC_PAD    0x0076

/** A flag returned by VtGetAlgorithmFromBer if the BER construct is
 * the Alg ID for AES-128-ECB.
 */
#define VT_ALG_ID_AES_128_ECB         0x0081
/** A flag returned by VtGetAlgorithmFromBer if the BER construct is
 * the Alg ID for AES-128-CBC.
 */
#define VT_ALG_ID_AES_128_CBC         0x0082
/** A flag returned by VtGetAlgorithmFromBer if the BER construct is
 * the Alg ID for AES-128-OFB.
 */
#define VT_ALG_ID_AES_128_OFB         0x0083
/** A flag returned by VtGetAlgorithmFromBer if the BER construct is
 * the Alg ID for AES-128-CFB.
 */
#define VT_ALG_ID_AES_128_CFB         0x0084
/** A flag returned by VtGetAlgorithmFromBer if the BER construct is
 * the Alg ID for AES-192-ECB.
 */
#define VT_ALG_ID_AES_192_ECB         0x0085
/** A flag returned by VtGetAlgorithmFromBer if the BER construct is
 * the Alg ID for AES-192-CBC.
 */
#define VT_ALG_ID_AES_192_CBC         0x0086
/** A flag returned by VtGetAlgorithmFromBer if the BER construct is
 * the Alg ID for AES-192-OFB.
 */
#define VT_ALG_ID_AES_192_OFB         0x0087
/** A flag returned by VtGetAlgorithmFromBer if the BER construct is
 * the Alg ID for AES-192-CFB.
 */
#define VT_ALG_ID_AES_192_CFB         0x0088
/** A flag returned by VtGetAlgorithmFromBer if the BER construct is
 * the Alg ID for AES-256-ECB.
 */
#define VT_ALG_ID_AES_256_ECB         0x0089
/** A flag returned by VtGetAlgorithmFromBer if the BER construct is
 * the Alg ID for AES-256-CBC.
 */
#define VT_ALG_ID_AES_256_CBC         0x008a
/** A flag returned by VtGetAlgorithmFromBer if the BER construct is
 * the Alg ID for AES-256-OFB.
 */
#define VT_ALG_ID_AES_256_OFB         0x008b
/** A flag returned by VtGetAlgorithmFromBer if the BER construct is
 * the Alg ID for AES-256-CFB.
 */
#define VT_ALG_ID_AES_256_CFB         0x008c
#define VT_ALG_ID_MAX_SYM_CIPHER      0x00ff

/** A flag returned by VtGetAlgorithmFromBer if the BER construct is
 * an IBE private key.
 */
#define VT_KEY_BER_BF_TYPE1_IBE_PRI   0x8001
/** A flag returned by VtGetAlgorithmFromBer if the BER construct is
 * a DSA private key.
 */
#define VT_KEY_BER_DSA_PRIVATE        0x8031
/** A flag returned by VtGetAlgorithmFromBer if the BER construct is
 * a DSA public key.
 */
#define VT_KEY_BER_DSA_PUBLIC         0x8032

/** A flag returned by VtGetAlgorithmFromBer if the BER construct is
 * unknown by any of the DerCoders passed in.
 */
#define VT_UNKNOWN_BER                0xffff

/* These are the VtDerCoders supported by the toolkit. When a Coder is
 * being used to encode, it is used in conjunction with special info
 * for the function. If there is no special info, the accompaniment is
 * a NULL pointer.
 * When used to decode (get the algorithm or set an object), there is
 * no special info.
 * If you add a DerCoder, add it to the list of VT_ALL_DER_CODERS and
 * increase the VT_ALL_DER_CODER_COUNT.
 */

/** This VtDerCoder refers to encrypting and decrypting. It knows about
 * the IBE encryption Alg ID.
 * <p>For encoding, the data associated with VtDerCoderBFType1IBE is NULL
 * pointer: (Pointer)0.
 */
extern VtDerCoder VtDerCoderBFType1IBE;

/** This VtDerCoder refers to signing and verifying. It knows about the
 * "DSA with SHA-1" signature Alg ID.
 * <p>For encoding, the data associated with VtDerCoderDSAwSHA1 is NULL
 * pointer: (Pointer)0.
 * <p>An object built using this DerCoder will generate signatures and
 * expect signatures to verify in the DER format:
 * <code>
 * <pre>
 *    Dss-Sig-Value  ::=  SEQUENCE  {
 *       r       INTEGER,
 *       s       INTEGER  }
 * </pre>
 * </code>
 */
extern VtDerCoder VtDerCoderDSAwSHA1;

/** This VtDerCoder knows about the HMAC with SHA-1 alg ID.
 * <p>For encoding, the data associated with VtDerCoderHMACwSHA1 is NULL
 * pointer: (Pointer)0.
 */
extern VtDerCoder VtDerCoderHMACwSHA1;

/** This VtDerCoder knows about the SHA-1 alg ID.
 * <p>For encoding, the data associated with VtDerCoderSHA1 is NULL
 * pointer: (Pointer)0.
 */
extern VtDerCoder VtDerCoderSHA1;
/** This VtDerCoder knows about the SHA-256 alg ID.
 * <p>For encoding, the data associated with VtDerCoderSHA256 is NULL
 * pointer: (Pointer)0.
 */
extern VtDerCoder VtDerCoderSHA256;
/** This VtDerCoder knows about the SHA-512 alg ID.
 * <p>For encoding, the data associated with VtDerCoderSHA512 is NULL
 * pointer: (Pointer)0.
 */
extern VtDerCoder VtDerCoderSHA512;

/** This VtDerCoder knows about the HMAC with MD5 alg ID.
 * <p>For encoding, the data associated with VtDerCoderHMACwMD5 is NULL
 * pointer: (Pointer)0.
 */
extern VtDerCoder VtDerCoderHMACwMD5;

/** This VtDerCoder knows about the MD5 alg ID.
 * <p>For encoding, the data associated with VtDerCoderMD5 is NULL
 * pointer: (Pointer)0.
 */
extern VtDerCoder VtDerCoderMD5;
/** This VtDerCoder knows about the "AES-128-ECB" alg ID.
 * <p>For encoding, the data associated with VtDerCoderAES128ECB is
 * an algorithm object set and initialized to perform AES-128 in ECB
 * mode with no padding. That is, build an algorithm object using
 * VtAlgorithmImplAES with the BlockCipherInfo set with VtFeedbackECB
 * and VtPaddingNoPad. Then Init with a 128-bit key.
 */
extern VtDerCoder VtDerCoderAES128ECB;
/** This VtDerCoder knows about the "AES-192-ECB" alg ID.
 * <p>For encoding, the data associated with VtDerCoderAES192ECB is
 * an algorithm object set and initialized to perform AES-192 in ECB
 * mode with no padding. That is, build an algorithm object using
 * VtAlgorithmImplAES with the BlockCipherInfo set with VtFeedbackECB
 * and VtPaddingNoPad. Then Init with a 192-bit key.
 */
extern VtDerCoder VtDerCoderAES192ECB;
/** This VtDerCoder knows about the "AES-256-ECB" alg ID.
 * <p>For encoding, the data associated with VtDerCoderAES256ECB is
 * an algorithm object set and initialized to perform AES-256 in ECB
 * mode with no padding. That is, build an algorithm object using
 * VtAlgorithmImplAES with the BlockCipherInfo set with VtFeedbackECB
 * and VtPaddingNoPad. Then Init with a 256-bit key.
 */
extern VtDerCoder VtDerCoderAES256ECB;
/** This VtDerCoder knows about the "AES-128-CBC" alg ID.
 * <p>For encoding, the data associated with VtDerCoderAES128CBC is
 * an algorithm object set and initialized to perform AES-128 in CBC
 * mode with no padding. That is, build an algorithm object using
 * VtAlgorithmImplAES with the BlockCipherInfo set with VtFeedbackCBC
 * and VtPaddingNoPad. Then Init with a 128-bit key.
 */
extern VtDerCoder VtDerCoderAES128CBC;
/** This VtDerCoder knows about the "AES-128-CBC" alg ID.
 * <p>This differs from VtDerCoderAES128CBC in that this DerCoder will
 * build objects that pad following the technique specified in PKCS #7
 * (which is the same technique described in PKCS #5).
 * <p>The OID is defined by NIST as 2.16.840.1.101.3.4.1.2. The OID
 * does not specify padding. In fact, in an email message, a NIST
 * representative explicitly stated that the OID does not specify
 * padding. However, the representative also suggested following the
 * instructions specified in Appendix A of NIST Special Publication
 * 800-38A. This appendix recommends applications pad data when using
 * AES in CBC mode, then gives one possible padding scheme which is not
 * the technique outlined in PKCS #5 (the most commonly used block
 * cipher padding scheme in use today). It further states, "For the
 * above padding method, the padding bits can be removed unambiguously,
 * provided the receiver can determine that the message is indeed
 * padded. One way to ensure that the receiver does not mistakenly
 * remove bits from an unpadded message is to require the sender to pad
 * every message, including messages in which the final block (segment)
 * is already complete. For such messages, an entire block (segment) of
 * padding is appended."
 * <p>A "standard" is supposed to specify how to do something in a
 * standard way, so that two entities can know what something means
 * without having to communicate anything in advance. But here is a
 * standard (along with the people who wrote and disseminate it) that
 * explicitly states that the sender and receiver must communicate in
 * advance to resolve an intentional ambiguity in the standard.
 * <p>Fortunately, PKCS #7 specifies that all block ciphers in modes
 * that require complete blocks (such as CBC) shall pad (even if the
 * input length is a multiple of the block size) and shall pad
 * following the technique specified in PKCS #5. So this DerCoder maps
 * the NIST OID to an object that encrypts using AES in CBC mode and
 * pads following the technique outlined in PKCS #7. It is a DerCoder
 * tied to a standard (PKCS #7) that resolves the NIST ambiguity.
 * <p>For encoding, the data associated with VtDerCoderPkcs7AES128CBC
 * is an algorithm object set and initialized to perform AES-128 in CBC
 * mode with PKCS #5 padding. That is, build an algorithm object using
 * VtAlgorithmImplAES with the BlockCipherInfo set with VtFeedbackCBC
 * and VtPaddingPkcs5. Then Init with a 128-bit key.
 */
extern VtDerCoder VtDerCoderPkcs7AES128CBC;
/** This VtDerCoder knows about the "AES-192-CBC" alg ID.
 * <p>For encoding, the data associated with VtDerCoderAES192CBC is
 * an algorithm object set and initialized to perform AES-192 in CBC
 * mode with no padding. That is, build an algorithm object using
 * VtAlgorithmImplAES with the BlockCipherInfo set with VtFeedbackCBC
 * and VtPaddingNoPad. Then Init with a 192-bit key.
 */
extern VtDerCoder VtDerCoderAES192CBC;
/** This VtDerCoder knows about the "AES-192-CBC" alg ID.
 * <p>This differs from VtDerCoderAES192CBC in that this DerCoder will
 * build objects that pad following the technique specified in PKCS #7
 * (which is the same technique described in PKCS #5).
 * <p>The OID is defined by NIST as 2.16.840.1.101.3.4.1.22. The OID
 * does not specify padding. In fact, in an email message, a NIST
 * representative explicitly stated that the OID does not specify
 * padding. However, the representative also suggested following the
 * instructions specified in Appendix A of NIST Special Publication
 * 800-38A. This appendix recommends applications pad data when using
 * AES in CBC mode, then gives one possible padding scheme which is not
 * the technique outlined in PKCS #5 (the most commonly used block
 * cipher padding scheme in use today). It further states, "For the
 * above padding method, the padding bits can be removed unambiguously,
 * provided the receiver can determine that the message is indeed
 * padded. One way to ensure that the receiver does not mistakenly
 * remove bits from an unpadded message is to require the sender to pad
 * every message, including messages in which the final block (segment)
 * is already complete. For such messages, an entire block (segment) of
 * padding is appended."
 * <p>A "standard" is supposed to specify how to do something in a
 * standard way, so that two entities can know what something means
 * without having to communicate anything in advance. But here is a
 * standard (along with the people who wrote and disseminate it) that
 * explicitly states that the sender and receiver must communicate in
 * advance to resolve an intentional ambiguity in the standard.
 * <p>Fortunately, PKCS #7 specifies that all block ciphers in modes
 * that require complete blocks (such as CBC) shall pad (even if the
 * input length is a multiple of the block size) and shall pad
 * following the technique specified in PKCS #5. So this DerCoder maps
 * the NIST OID to an object that encrypts using AES in CBC mode and
 * pads following the technique outlined in PKCS #7. It is a DerCoder
 * tied to a standard (PKCS #7) that resolves the NIST ambiguity.
 * <p>For encoding, the data associated with VtDerCoderPkcs7AES192CBC
 * is an algorithm object set and initialized to perform AES-192 in CBC
 * mode with PKCS #5 padding. That is, build an algorithm object using
 * VtAlgorithmImplAES with the BlockCipherInfo set with VtFeedbackCBC
 * and VtPaddingPkcs5. Then Init with a 192-bit key.
 */
extern VtDerCoder VtDerCoderPkcs7AES192CBC;
/** This VtDerCoder knows about the "AES-256-CBC" alg ID.
 * <p>For encoding, the data associated with VtDerCoderAES256CBC is
 * an algorithm object set and initialized to perform AES-256 in CBC
 * mode with no padding. That is, build an algorithm object using
 * VtAlgorithmImplAES with the BlockCipherInfo set with VtFeedbackCBC
 * and VtPaddingNoPad. Then Init with a 256-bit key.
 */
extern VtDerCoder VtDerCoderAES256CBC;
/** This VtDerCoder knows about the "AES-256-CBC" alg ID.
 * <p>This differs from VtDerCoderAES256CBC in that this DerCoder will
 * build objects that pad following the technique specified in PKCS #7
 * (which is the same technique described in PKCS #5).
 * <p>The OID is defined by NIST as 2.16.840.1.101.3.4.1.42. The OID
 * does not specify padding. In fact, in an email message, a NIST
 * representative explicitly stated that the OID does not specify
 * padding. However, the representative also suggested following the
 * instructions specified in Appendix A of NIST Special Publication
 * 800-38A. This appendix recommends applications pad data when using
 * AES in CBC mode, then gives one possible padding scheme which is not
 * the technique outlined in PKCS #5 (the most commonly used block
 * cipher padding scheme in use today). It further states, "For the
 * above padding method, the padding bits can be removed unambiguously,
 * provided the receiver can determine that the message is indeed
 * padded. One way to ensure that the receiver does not mistakenly
 * remove bits from an unpadded message is to require the sender to pad
 * every message, including messages in which the final block (segment)
 * is already complete. For such messages, an entire block (segment) of
 * padding is appended."
 * <p>A "standard" is supposed to specify how to do something in a
 * standard way, so that two entities can know what something means
 * without having to communicate anything in advance. But here is a
 * standard (along with the people who wrote and disseminate it) that
 * explicitly states that the sender and receiver must communicate in
 * advance to resolve an intentional ambiguity in the standard.
 * <p>Fortunately, PKCS #7 specifies that all block ciphers in modes
 * that require complete blocks (such as CBC) shall pad (even if the
 * input length is a multiple of the block size) and shall pad
 * following the technique specified in PKCS #5. So this DerCoder maps
 * the NIST OID to an object that encrypts using AES in CBC mode and
 * pads following the technique outlined in PKCS #7. It is a DerCoder
 * tied to a standard (PKCS #7) that resolves the NIST ambiguity.
 * <p>For encoding, the data associated with VtDerCoderPkcs7AES256CBC
 * is an algorithm object set and initilized to perform AES-256 in CBC
 * mode with PKCS #5 padding. That is, build an algorithm object using
 * VtAlgorithmImplAES with the BlockCipherInfo set with VtFeedbackCBC
 * and VtPaddingPkcs5. Then Init with a 256-bit key.
 */
extern VtDerCoder VtDerCoderPkcs7AES256CBC;
/** This VtDerCoder knows about the "AES-128-OFB" alg ID.
 * <p>For encoding, the data associated with VtDerCoderAES128OFB is
 * an algorithm object set and initialized to perform AES-128 in OFB
 * mode with no padding. That is, build an algorithm object using
 * VtAlgorithmImplAES with the BlockCipherInfo set with VtFeedbackOFB
 * and VtPaddingNoPad. Then Init with a 128-bit key.
 */
extern VtDerCoder VtDerCoderAES128OFB;
/** This VtDerCoder knows about the "AES-192-OFB" alg ID.
 * <p>For encoding, the data associated with VtDerCoderAES192OFB is
 * an algorithm object set and initialized to perform AES-192 in OFB
 * mode with no padding. That is, build an algorithm object using
 * VtAlgorithmImplAES with the BlockCipherInfo set with VtFeedbackOFB
 * and VtPaddingNoPad. Then Init with a 192-bit key.
 */
extern VtDerCoder VtDerCoderAES192OFB;
/** This VtDerCoder knows about the "AES-256-OFB" alg ID.
 * <p>For encoding, the data associated with VtDerCoderAES256OFB is
 * an algorithm object set and initialized to perform AES-256 in OFB
 * mode with no padding. That is, build an algorithm object using
 * VtAlgorithmImplAES with the BlockCipherInfo set with VtFeedbackOFB
 * and VtPaddingNoPad. Then Init with a 256-bit key.
 */
extern VtDerCoder VtDerCoderAES256OFB;
/** This VtDerCoder knows about the "AES-128-CFB" alg ID.
 * <p>For encoding, the data associated with VtDerCoderAES128CFB is
 * an algorithm object set and initialized to perform AES-128 in CFB
 * mode with no padding. That is, build an algorithm object using
 * VtAlgorithmImplAES with the BlockCipherInfo set with VtFeedbackCFB
 * and VtPaddingNoPad. Then Init with a 128-bit key.
 */
extern VtDerCoder VtDerCoderAES128CFB;
/** This VtDerCoder knows about the "AES-192-CFB" alg ID.
 * <p>For encoding, the data associated with VtDerCoderAES192CFB is
 * an algorithm object set and initialized to perform AES-192 in CFB
 * mode with no padding. That is, build an algorithm object using
 * VtAlgorithmImplAES with the BlockCipherInfo set with VtFeedbackCFB
 * and VtPaddingNoPad. Then Init with a 192-bit key.
 */
extern VtDerCoder VtDerCoderAES192CFB;
/** This VtDerCoder knows about the "AES-256-CFB" alg ID.
 * <p>For encoding, the data associated with VtDerCoderAES256CFB is
 * an algorithm object set and initialized to perform AES-256 in CFB
 * mode with no padding. That is, build an algorithm object using
 * VtAlgorithmImplAES with the BlockCipherInfo set with VtFeedbackCFB
 * and VtPaddingNoPad. Then Init with a 256-bit key.
 */
extern VtDerCoder VtDerCoderAES256CFB;

/** This VtDerCoder knows about the "DES-CBC" alg ID.
 * <p>For encoding, the data associated with VtDerCoderDESwCBCPad is an
 * algorithm object set to perform DES in CBC mode with PKCS #5 padding.
 * That is, build an algorithm object using VtAlgorithmImplDES with the
 * BlockCipherInfo set with VtFeedbackCBC and VtPaddingPkcs5.
 */
extern VtDerCoder VtDerCoderDESwCBCPad;
/** This VtDerCoder knows about the "TripleDES-EDE-CBC" alg ID.
 * <p>For encoding, the data associated with VtDerCoder3DESEDEwCBC
 * is an algorithm object set to perform Triple DES in CBC mode with
 * PKCS #5 padding. That is, build an algorithm object using
 * VtAlgorithmImplDES with the BlockCipherInfo set with VtFeedbackCBC
 * and VtPaddingPkcs5.
 */
extern VtDerCoder VtDerCoder3DESEDEwCBCPad;

/** This VtDerCoder knows about the encoding of an IBE private key.
 * <p>For encoding, the data associated with
 * VtDerCoderBFType1IBEPrivateKey is a key object set with IBE private
 * key data.
 */
extern VtDerCoder VtDerCoderBFType1IBEPrivateKey;

/** This VtDerCoder knows about the encoding of a DSA private key.
 * <p>For encoding, the data associated with VtDerCoderDSAPrivateKey is
 * a key object set with DSA private key data.
 */
extern VtDerCoder VtDerCoderDSAPrivateKey;
/** This VtDerCoder knows about the encoding of a DSA public key.
 * <p>For encoding, the data associated with VtDerCoderDSAPublicKey is
 * a key object set with DSA public key data.
 */
extern VtDerCoder VtDerCoderDSAPublicKey;

/** This VtDerCoder knows about the encoding of a IBE params.
 * <p>For encoding, the data associated with VtDerCoderBFType1IBEParams
 * is a param object set with IBE private param data.
 */
extern VtDerCoder VtDerCoderBFType1IBEParams;

/** This is the data struct to accompany VtLibCtxParamDERCoderArray.
 */
typedef struct
{
  unsigned int derCoderCount;
  VtDerCoder **derCoders;
} VtDERCoderArray;

/** Use this #define to help build a DerCoder array containing every
 * DerCoder supported in the toolkit.
 * <code>
 * <pre>
 *   VtDerCoder *allDerCoders[VT_ALL_DER_CODER_COUNT] =
 *     { VT_ALL_DER_CODERS };
 * </pre>
 * </code>
 */
#define VT_ALL_DER_CODER_COUNT       26
/** Use this #define to help build a DerCoder array containing every
 * DerCoder supported in the toolkit.
 * <code>
 * <pre>
 *   VtDerCoder *allDerCoders[VT_ALL_DER_CODER_COUNT] =
 *     { VT_ALL_DER_CODERS };
 * </pre>
 * </code>
 * <p>NOTE!!! There are two DerCoders for each key length of AES-CBC,
 * one does no padding and the Pkcs7 version does padding. This list
 * includes the Pkcs7 AES-CBC DerCoders (the ones that pad), not the
 * "base" version.
 */
#define VT_ALL_DER_CODERS            \
    VtDerCoderBFType1IBE,            \
    VtDerCoderDSAwSHA1,              \
    VtDerCoderSHA1,                  \
    VtDerCoderHMACwSHA1,             \
    VtDerCoderSHA256,                \
    VtDerCoderSHA512,                \
    VtDerCoderMD5,                   \
    VtDerCoderHMACwMD5,              \
    VtDerCoderAES128ECB,             \
    VtDerCoderAES192ECB,             \
    VtDerCoderAES256ECB,             \
    VtDerCoderPkcs7AES128CBC,        \
    VtDerCoderPkcs7AES192CBC,        \
    VtDerCoderPkcs7AES256CBC,        \
    VtDerCoderAES128OFB,             \
    VtDerCoderAES192OFB,             \
    VtDerCoderAES256OFB,             \
    VtDerCoderAES128CFB,             \
    VtDerCoderAES192CFB,             \
    VtDerCoderAES256CFB,             \
    VtDerCoderDESwCBCPad,            \
    VtDerCoder3DESEDEwCBCPad,        \
    VtDerCoderBFType1IBEPrivateKey,  \
    VtDerCoderDSAPrivateKey,         \
    VtDerCoderDSAPublicKey,          \
    VtDerCoderBFType1IBEParams

/*@}*/

/*=========================================================*/
/*                                                         */
/* Algorithm Object                                        */
/*                                                         */
/*=========================================================*/

/* The Algorithm Object along with create, destroy and other functions
 * are declared in vibecrypto.h. Those functions are part of
 * vibecrypto.lib. The following are declarations of algorithm object
 * operations found in vibe.lib.
 */

/**
 * \defgroup AlgorithmGroup Algorithm Object
 */

/*@{*/

/** This VtAlgorithmImpl is used to set an algorithm object to
 * perform the operation defined in an algorithm identifier.
 * <p>An Alg ID consists of an OBJECT IDENTITFIER and parameters.
 * <code>
 * <pre>
 *    SEQUENCE {
 *      algorithm   OBJECT IDENTIFIER,
 *      parameters  (defined by OID) OPTIONAL }
 * </pre>
 * </code>
 * <p>The data associated with VtAlgorithmImplAlgId is a pointer to a
 * VtSetAlgIdInfo struct.
 */
extern VtAlgorithmImpl VtAlgorithmImplAlgId;

/** This is the data type to accompany  VtAlgorithmImplAlgId and
 * VtRecipientInfoListParamSymmetricAlgorithm. The spp supplies a list
 * of DerCoder’s it's willing to support along with a given algId. The
 * toolkit will find the appropriate DerCoder in the list to match the
 * algId and use it. If the BER encoding is for an entity not covered
 * in the list, the toolkit will return an error.
 */
typedef struct
{
  VtDerCoder **derCoders;
  unsigned int derCoderCount;
  unsigned char *berEncoding;
  unsigned int maxEncodingLen;
} VtSetAlgIdInfo;

/*@}*/

/*=========================================================*/
/*                                                         */
/* Random Object                                           */
/*                                                         */
/*=========================================================*/

/* The Random Object along with create, destroy and other functions are
 * declared in vibecrypto.h. Those functions are part of
 * vibecrypto.lib. The following are declarations of random object
 * operations found in vibeproviders.lib.
 */

/**
 * \defgroup RandomGroup Random Object
 */

/*@{*/

/** This VtRandomImpl is used to set a random object to perform
 * pseudo-random number generation using seed material obtained from
 * CAPI's random source.
 * <p>The data associated with VtRandomImplCAPIAutoSeed is a NULL
 * Pointer: (Pointer)0.
 * <p>The actual PRNG implementation will be the algorithm specified
 * in FIPS PUB 186-2, however, the app does not need to build the
 * VtFips186PrngInfo struct (XKEY, primeQ, etc). The toolkit will take
 * care of all the details.
 * <p>Furthermore, upon creation, the toolkit will seed the object
 * using seed material obtained from CAPI (autoseeding).
 * <p>Even though this Impl autoseeds, you can still add more seed
 * material to an object built using this Impl (VtSeedRandomObject).
 * <p>NOTE! There is no guarantee that the CAPI autoseeding technique
 * will add a particular number of bits of entropy.
 */
extern VtRandomImpl VtRandomImplCAPIAutoSeed;

/** This VtRandomImpl is used to set a random object to perform
 * pseudo-random number generation using seed material obtained from
 * OpenSSL's random seed operation.
 * <> The data associated with VtRandomImplOpenSSLAutoSeed is a NULL
 * pointer: (Pointer)0.
 * <p>The actual PRNG implementation will be the algorithm specified
 * in FIPS PUB 186-2, however, the app does not need to build the
 * VtFips186PrngInfo struct (XKEY, primeQ, etc). The toolkit will take
 * care of all the details.
 * <p>Furthermore, upon creation, the toolkit will seed the object
 * using seed material obtained from OpenSSL (autoseeding).
 * <p>Even though this Impl autoseeds, you can still add more seed
 * material to an object built using this Impl (VtSeedRandomObject).
 * <p>NOTE! There is no guarantee that the OpenSSL autoseeding
 * technique will add a particular number of bits of entropy.
 */
extern VtRandomImpl VtRandomImplOpenSSLAutoSeed;

/** This VtRandomImpl is used to set a random object to perform
 * pseudo-random number generation using seed material obtained from
 * a source based on the platform. On Windows, the actual source of
 * seed material will likely be CAPI, on other platforms, it will
 * likely be OpenSSL. That is, this Impl will likely be the same as
 * VtRandomImplCAPIAutoSeed on Windows platforms and
 * VtRandomImplOpenSSLAutoSeed on other platforms, this Impl just
 * allows you to write code that will work on any platform (no need to
 * build a particular random object for a Windows port and a different
 * random object for a Linux or Solaris port).
 * <p>The data associated with VtRandomImplAutoSeed is a NULL pointer:
 * (Pointer)0.
 * <p>The actual PRNG implementation will be the algorithm specified
 * in FIPS PUB 186-2, however, the app does not need to build the
 * VtFips186PrngInfo struct (XKEY, primeQ, etc). The toolkit will take
 * care of all the details.
 * <p>Furthermore, upon creation, the toolkit will seed the object
 * using seed material obtained from some source, which will probably
 * be CAPI or OpenSSL (autoseeding).
 * <p>Even though this Impl autoseeds, you can still add more seed
 * material to an object built using this Impl (VtSeedRandomObject).
 * <p>NOTE! There is no guarantee that the autoseeding technique used
 * will add a particular number of bits of entropy.
 */
extern VtRandomImpl VtRandomImplAutoSeed;

/*@}*/

/*=========================================================*/
/*                                                         */
/* Parameter Object                                        */
/*                                                         */
/*=========================================================*/

/* The Parameter Object along with create, destroy and other functions
 * are declared in vibecrypto.h. Those functions are part of
 * vibecrypto.lib. The following are declarations of parameter object
 * operations found in vibe.lib.
 */

/**
 * \defgroup ParameterGroup Parameter Object
 */

/*@{*/

/** SetParam only.
 * <p>This VtParameterParam is used to set a parameter object with a
 * specific set of parameters that are in BER encoded form. Currently
 * only DSA parameters in BER form are supported.
 * <p>The data associated with VtParameterParamBer is a pointer to a
 * VtSetParamBerInfo struct.
 */
extern VtParameterParam VtParameterParamBer;

/** This is the data struct to accompany VtParameterParamBer.
 * <p>Set the derCoders to an array of DerCoders. The toolkit will
 * find the DerCoder that can decode the particular set of parameters
 * passed in. An application will generally build a DerCoder list
 * containing all the parameter BER encodings it's willing to support.
 */
typedef struct
{
  VtDerCoder **derCoders;
  unsigned int derCoderCount;
  unsigned char *berEncoding;
  unsigned int maxEncodingLen;
} VtSetParamBerInfo;

/*@}*/

/*=========================================================*/
/*                                                         */
/* Key Object                                              */
/*                                                         */
/*=========================================================*/

/* The Key Object along with create, destroy and other functions
 * are declared in vibecrypto.h. Those functions are part of
 * vibecrypto.lib. The following are declarations of key object
 * operations found in vibe.lib.
 */

/**
 * \defgroup KeyGroup Key Object
 */

/*@{*/

/** SetParam only.
 * <p>This VtKeyParam is used to set a key object with a specific key
 * that is in BER encoded form. Currently only DSA keys in BER form are
 * supported.
 * <p>The data associated with VtKeyParamBer is a pointer to a
 * VtSetKeyBerInfo struct.
 */
extern VtKeyParam VtKeyParamBer;

/** SetParam only.
 * <p>This VtKeyParam is used to set a key object with a specific key
 * from an X.509 cert, represented by the DER encoding.
 * <p>The data associated with VtKeyParamX509Cert is a pointer to a
 * VtCertInfo struct. For this KeyParam, the certificate's
 * SubjectPublicKeyInfo is the public key being loaded.
 */
extern VtKeyParam VtKeyParamX509Cert;

/** SetParam only.
 * <p>This VtKeyParam is used to set a key object with a specific key
 * from an X.509 cert, which is represented by an object (rather than
 * the DER of the cert).
 * <p>The data associated with VtKeyParamX509Cert is a VtCertObject.
 */
extern VtKeyParam VtKeyParamX509CertObject;

/** This is the data type to accompany VtKeyParamX509Cert.
 * <p>The DerCoders contains the der types that can decode the
 * subjectPublicKey, such as VtDerCoderDSAPublicKey.
 * <p>The cert points to the DER encoding of the cert.
 */
typedef struct
{
  VtDerCoder **derCoders;
  unsigned int derCoderCount;
  unsigned char *cert;
  unsigned int certLen;
} VtCertInfo;

/** This is the data type to accompany VtKeyParamBer.
 * <p>The caller creates a list of DerCoder’s, the toolkit will find
 * the appropriate DerCoder in the list that can decode the BER
 * encoding and use it. If the BER encoding is for an entity not covered
 * in the list, the toolkit will return an error.
 * <p>IBE keys may contain the IBE parameters (prime, subprimes, etc.)
 * or may not (in most applications they will not). If so, the function
 * that decodes the BER encoding will need to find the parameters from
 * storage or download them. If so, the caller must pass in the storage
 * provider and/or the transport context along with the policy context.
 * <p>If the BER encoding is for a DSA key or if the BER encoding is for
 * an IBE key and contains the parameters, then leave the policyCtx,
 * storageCtx and transportCtx NULL.
 */
typedef struct
{
  VtDerCoder **derCoders;
  unsigned int derCoderCount;
  unsigned char *berEncoding;
  unsigned int maxEncodingLen;
  VtPolicyCtx policyCtx;
  VtStorageCtx storageCtx;
  VtTransportCtx transportCtx;
} VtSetKeyBerInfo;

/*@}*/

/*=========================================================*/
/*                                                         */
/* Token Generation                                        */
/*                                                         */
/*=========================================================*/

/**
 * \defgroup TokenGenGroup Token Generation
 */

/*@{*/

/** The function VtConstructAuthToken builds an auth token using a
 * VtAuthTokenImpl. This typedef defines what a VtAuthTokenImpl is.
 * Although it is a function pointer, an application should never call
 * a VtAuthTokenImpl directly, only pass it as an argument to
 * VtConstructAuthToken.
 */
typedef int VT_CALLING_CONV (VtAuthTokenImpl) (
   VtIdentityObject idObj, 
   Pointer associatedInfo, 
   unsigned char *token,
   unsigned int tokenSize,  
   unsigned int *outLen, 
   unsigned int flag
);

/** Build an authentication token using the given identity,
 * AuthTokenImpl, and the Impl's associated info.
 * <p>An auth token is based on a shared secret and a time. It is used
 * to authenticate a given identity.
 * <p>Currently all key servers require an SSL connection to download a
 * key, so the auth token will not be sent in the clear. Furthermore,
 * the key servers will likely have a timeout for tokens. That is, a
 * token is based on a time and the time is sent. If too much time has
 * elapsed since the token was constructed and received, the key server
 * can reject the token and require the sender to construct a new one
 * based on an updated time.
 * <p>For example:
 * <code>
 * <pre>
 *    unsigned int tokenLen;
 *    unsigned char token[256];
 *    VtVoltageAuthTokenInfo tokenInfo;
 *
 *    VtGetTime (libCtx, &(tokenInfo.authTime));
 *    tokenInfo.sharedSecret.data = "12345678";
 *    tokenIndo.sharedSecret.len = 8;
 *    tokenInfo.policyCtx = policyCtx;
 *    tokenInfo.storageCtx = storageCtx;
 *    tokenInfo.transportCtx = transportCtx;
 *    status = VtConstructAuthToken (
 *      identity, VtAuthTokenImplVoltage, (Pointer)&tokenInfo,
 *      token, sizeof (token), &tokenLen);
 * </pre>
 * </code>
 * <p>This function will place the constructed token into the
 * app-supplied buffer. If the buffer is NULL or simply not big enough
 * to hold the token, the function will return the BUFFER_TOO_SMALL
 * error and set tokenLen to the required size.
 *
 * @param identity The identity for which the token is to be built. 
 * If the identity is not encoded the function will encode it first.
 * @param authTokenImpl The implementation to use in constructing the
 * token.
 * @param associatedInfo The info needed by the AuthTokenImpl.
 * @param token The buffer into which the function will place the token.
 * @param bufferSize The size, in bytes, of the output buffer.
 * @param tokenLen The address where the routine will deposit the
 * resulting length, in bytes, of the token.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VtConstructAuthToken (
   VtIdentityObject identity,
   VtAuthTokenImpl authTokenImpl,
   Pointer associatedInfo,
   unsigned char *token,
   unsigned int bufferSize,
   unsigned int *tokenLen
);

/* These are the AuthTokenImpls supported by the toolkit. Each
 * AuthTokenImpl is used in conjunction with special info for the
 * function. If there is no special info, the accompaniment is a NULL
 * pointer.
 */

/** This AuthTokenImpl is used to build an auth token following the
 * Voltage proprietary definition. It will build a NULL-terminated
 * ASCII string (actually a base 64 encoded binary string) from the
 * identity, the authentication time (which might be different from the
 * identity's time or the notBefore time), and a secret.
 * <p>The data associated with VtAuthTokenImplVoltage is a pointer to a
 * VtVoltageAuthTokenInfo struct.
 */
extern VtAuthTokenImpl VtAuthTokenImplVoltage;

/** This is the data to accompany VtAuthTokenImplVoltage.
 * <p>For the Impl to work, it will need the encoded identity. If, when
 * constructing the token, the Impl finds that the supplied identity
 * has not been encoded yet it will get the encoding by calling
 * VtEncodeIdentity. To make that call it will need a policy, transport
 * and an optional storage context. If your app will have encoded the
 * identity before constructing the token, or if the contexts are
 * loaded into the libCtx (used to create the identity object), then
 * these context fields can be NULL.
 * <p> The authTime field is currently ignored by the 
 * VtAuthTokenImplVoltage Impl but that may change in future.
 * <p>The sharedSecret is the value both the requestor and the key
 * server know, it is used to construct the token.
 */
typedef struct
{
  VtTime          authTime;
  VtItem          sharedSecret;
  VtPolicyCtx     policyCtx;
  VtStorageCtx    storageCtx;
  VtTransportCtx  transportCtx;
} VtVoltageAuthTokenInfo;

/*@}*/

/*=========================================================*/
/*                                                         */
/* Certificate Requests                                    */
/*                                                         */
/*=========================================================*/

/**
 * \defgroup CertReqGroup Certificate Request Object
 */

/*@{*/

/** The certificate request object.
 * <p>Note that the object is a pointer type.
 */
typedef struct VtCertRequestObjectDef *VtCertRequestObject;

/** The function VtCreateCertRequestObject builds a cert request object
 * using a VtCertRequestImpl. This typedef defines what a
 * VtCertRequestImpl is. Although a VtCertRequestImpl is a function
 * pointer, an application should never call one directly, only pass it
 * as an argument to VtCreateCertRequestObject.
 */
typedef int VT_CALLING_CONV (VtCertRequestImpl) (
   VtCertRequestObject *, Pointer, unsigned int);

/** The functions VtSetCertRequestParam and VtGetCertRequestParam add
 * or get information to a cert request object. The information to add
 * or get is defined by a VtCertRequestParam. This typedef defines what
 * a VtCertRequestParam is. Although a VtCertRequestParam is a function
 * pointer, an application should never call one directly, only pass it
 * as an argument to VtSetCertRequestParam or VtGetCertRequestParam.
 */
typedef int VT_CALLING_CONV (VtCertRequestParam) (
   VtCertRequestObject, Pointer, unsigned int);

/** The function VtGenerateCertRequest will build a certificate request
 * for a public signing key for the entity given by the identity. The
 * VtCertRequestType defines what sort of request (PKCS #10? DSA? RSA?
 * etc.) is to be generated. This typedef defines what a
 * VtCertRequestType is.
 * <p> Although a VtCertRequestType is a function pointer, an
 * application should never call one directly, only pass it as an
 * argument to VtGenerateCertRequest.
 */
typedef int VT_CALLING_CONV (VtCertRequestType) (
   VtIdentityObject, VtKeyObject, VtKeyObject, VtCertRequestObject,
   Pointer, unsigned int);

/** Create a new cert request object. This allocates space for an
 * "empty" object, then loads the given CertRequestImpl to make it an
 * "active" object.
 * <p>The VtCertRequestImpl defines some of the key object operations.
 * The include file vibe.h defines the supported CertRequestImpls.
 * Look through the include file to see which CertRequestImpl to use for
 * your application. All supported CertRequestImpls will be defined as
 * in the following example.
 * <code>
 * <pre>
 *   extern VtCertRequestImpl VtCertRequestImplMpCtx;
 * </pre>
 * </code>
 * <p>Associated with each CertRequestImpl is specific info. The
 * documentation for each CertRequestImpl will describe the associated
 * info it needs. That data could be another object, it could be data in
 * a particular struct, it might be a NULL pointer. Check the
 * documentation for each CertRequestImpl for a description of the data
 * and its required format.
 * <p>To use this function decide which CertRequestImpl you want to use,
 * then determine what information that CertRequestImpl needs and in
 * which format it is presented. Collect the data in the appropriate
 * format then call this function passing in the desired CertRequestImpl
 * and the required info. The associated info must be cast to Pointer.
 * <p>For example, if the required info is a VtItem struct, declare a
 * variable to be of type VtItem, fill in the fields, then pass the address
 * of that struct cast to Pointer.
 * <p>The input certReqObj is a pointer to an object. It should point
 * to a NULL VtCertRequestObject. This function will go to the address
 * given and deposit a created object.
 * <p>If you call Create, you must call Destroy after you are done with
 * the object but before the program exits.
 * <p>Example:
 * <code>
 * <pre>
 *    int status;
 *    VtCertRequestObject certReqObj = (VtCertRequestObject)0;
 *
 *    do {
 *          . . .
 *      status = VtCreateCertRequestObject (
 *        libCtx, VtCertRequestImplBasic, (Pointer)0, &certReqObj);
 *      if (status != 0)
 *        break;
 *          . . .
 *    } while (0);
 *
 *    VtDestroyCertRequestObject (&certReqObj);
 * </pre>
 * </code>
 * @param libCtx The library context.
 * @param certRequestImpl The implementation the object will use.
 * @param associatedInfo The info needed by the CertRequestImpl.
 * @param certReqObj A pointer to where the routine will deposit the
 * created object.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtCreateCertRequestObject (
   VtLibCtx libCtx,
   VtCertRequestImpl certRequestImpl,
   Pointer associatedInfo,
   VtCertRequestObject *certReqObj
);

/* These are the VtCertRequestImpls supported by the toolkit. Each
 * CertRequestImpl is used in conjunction with special info for the
 * function. If there is no special info, the accompaniment is a NULL
 * pointer.
 */

/** This VtCertRequestImpl is used to load up the MpIntCtx the object will
 * use when dealing with public keys. For the toolkit to operate on
 * public keys, it must have an MpIntCtx.
 * <p>If the associatedInfo passed in with this Impl is NULL, then the
 * Impl will try to find an mpCtx in the libCtx.
 * <p>Note that the loaded mpCtx will only be used for public keys,
 * never private keys.
 * <p>The data associated with VtCertRequestImplMpCtx is a VtMpIntCtx.
 */
extern VtCertRequestImpl VtCertRequestImplMpCtx;

/** When building a cert request object, use this VtCertRequestImpl if
 * the mpCtx desired is loaded into the libCtx.
 * <p>A cert request object cannot operate without an mpCtx, so do not
 * use this Impl unless the libCtx contains an mpCtx.
 * <p>The data associated with VtCertRequestImplBasic is a NULL
 * pointer: (Pointer)0.
 */
extern VtCertRequestImpl VtCertRequestImplBasic;

/** Destroy a Cert Request Object. This frees up any memory allocated
 * during the object's creation and use.
 * <p>If you call Create, you must call Destroy after you are done with
 * the object but before the program exits.
 * <p>Example:
 * <code>
 * <pre>
 *    int status;
 *    VtCertRequestObject certReqObj = (VtCertRequestObject)0;
 *
 *    do {
 *          . . .
 *      status = VtCreateCertRequestObject (
 *        libCtx, VtCertRequestImplBasic, (Pointer)0, &certReqObj);
 *      if (status != 0)
 *        break;
 *          . . .
 *    } while (0);
 *
 *    VtDestroyCertRequestObject (&certReqObj);
 * </pre>
 * </code>
 * @param certReqObj A pointer to where the routine will find the object
 * to destroy.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtDestroyCertRequestObject (
   VtCertRequestObject *certReqObj
);

/** Set the cert request object with the information given.
 * <p>The VtCertRequestParam defines what information the object will be
 * set with.
 * <p>The include file vibe.h defines the supported CertRequestParams.
 * Look through the include file to see which CertRequestParam to use for
 * your application. All supported CertRequestParams will be defined as
 * in the following example.
 * <code>
 * <pre>
 *   extern VtCertRequestParam VtCertRequestParamP10Der;
 * </pre>
 * </code>
 * <p>Associated with each CertRequestParam is specific info. The
 * documentation for each CertRequestParam will describe the associated
 * info it needs. That data could be another object, it could be data
 * in a particular struct, it might be a NULL pointer. Check the
 * documentation for each CertRequestParam for a description of the
 * data and its required format.
 * <p>To use this function decide which CertRequestParam you want to
 * use, then determine what information that CertRequestParam needs and
 * in which format it is presented. Collect the data in the appropriate
 * format then call this function passing in the desired
 * CertRequestParam and the required info. The associated info must be
 * cast to Pointer.
 * <p>For example, if the required info is a particular struct, declare
 * a variable to be of that type struct, fill in the fields, then pass
 * the address of that struct cast to Pointer.
 *
 * @param certReqObj The object to set.
 * @param certRequestParam The type of info to set.
 * @param associatedInfo The info needed by the CertRequestParam.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtSetCertRequestParam (
   VtCertRequestObject certReqObj,
   VtCertRequestParam certRequestParam,
   Pointer associatedInfo
);

/** Get the specified type of information out of a cert request object.
 * <p>This function will go to the address given by getInfo and deposit
 * a pointer. That pointer is the address of the data. The memory to
 * hold the information belongs to the object, do not free it.
 * <p>The VtCertRequestParam will specify what kind of information will
 * be returned, the getInfo is where the function will deposit the info.
 * <p>The include file vibe.h defines the supported
 * CertRequestParams. Look through the include file to see which
 * CertRequestParam to use for your application.
 * <p>See also VtSetCertRequestParam.
 * <p>To use this function decide which CertRequestParam you want to
 * use, then determine what information that CertRequestParam will
 * return and in which format it is presented. Declare a variable to be
 * a pointer to the appropriate type, then call this function passing
 * in the desired CertRequestParam and the address of the variable.
 * Cast the address of the pointer to the appropriate type: (Pointer *).
 * <p>If the associated info is already a pointer type (such as an
 * object), declare a variable to be of the appropriate type and pass
 * the address of that variable cast to (Pointer *).
 * <p>For example, if the returned info is a VtItem struct, declare a
 * variable to be of type (VtItem *), pass the address of that variable
 * (&varName) cast to (Pointer *).
 *
 * @param certReqObj The object containing the data to get.
 * @param certRequestParam The type of info to get.
 * @param getInfo The address of a pointer. The function will go to
 * that address and deposit a pointer to the info requested.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtGetCertRequestParam (
   VtCertRequestObject certReqObj,
   VtCertRequestParam certRequestParam,
   Pointer *getInfo
);

/* These are the CertRequestParams supported by the toolkit. Each
 * CertRequestParam is used in conjunction with special info for the
 * function.
 */

/** SetParam only.
 * <p>Use this VtCertRequestParam to set an object with the DER
 * encoding of a PKCS #10 request.
 * <p>The data associated with VtCertRequestParamP10Der is a pointer to a
 * VtCertRequestInfo struct that gives the data and length of the DER
 * of the P10 request along with an mpCtx and a DerCoder list that
 * should contain the DerCoder for the key in the request.
 */
extern VtCertRequestParam VtCertRequestParamP10Der;

/** GetParam only.
 * <p>Use this VtCertRequestParam to get the cert request data out of
 * the object. The object must already contain a PKCS #10 request
 * (built by calling GenerateCertRequest, most likely).
 * <p>The associated info is a VtItem.
 * <p>When getting, pass in the address of a pointer as the getInfo,
 * the Get function will desposit a pointer to a VtItem struct at the
 * address. The VtItem will give the data and length of the DER of the
 * P10 request.
 */
extern VtCertRequestParam VtCertRequestParamP10DerData;

/** Generate a cert request for the given public key, using the private
 * key to sign it. The owner of the cert is represented by the identity
 * object. The identity object must contain the encoded identity (this
 * function will not be able to compute the encoding).
 * <p>The caller supplies a created but empty VtCertRequestObject, this
 * function will load the cert request into it.
 * <p>The CertRequestType is the argument to indicate what kind of
 * request the function is to build. See the list of valid
 * CertRequestImpls in vibe.h.
 *
 * @param identity The identity for which a cert is requested.
 * @param pubKey The key object containing the public key for which a
 * request is being made.
 * @param priKey The partner to the public key, the cert request will
 * almost certainly need to be signed by the private key.
 * @param certRequestType Indicates what form of cert request is to be
 * built.
 * @param associatedInfo The info the RequestImpl needs.
 * @param certReq The object into which the result will be placed.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtGenerateCertRequest (
   VtIdentityObject identity,
   VtKeyObject pubKey,
   VtKeyObject priKey,
   VtCertRequestType certRequestType,
   Pointer associatedInfo,
   VtCertRequestObject certReq
);

/* These are the VtCertRequestTypes supported by the toolkit. Each
 * Type is used in conjunction with special info for the function.
 */

/** Use this VtCertRequestType to build a cert request for a DSA public
 * key following PKCS #10.
 * <p>The data associated with VtCertRequestTypeP10DSA is a random
 * object. When calling VtGenerateCertRequest, pass a seeded random
 * object (cast to Pointer) as the certReqTypeInfo argument. The object
 * should be seeded with strong seed material.
 */
extern VtCertRequestType VtCertRequestTypeP10DSA;

/** This is the data type to accompany VtCertRequestParamP10Der.
 * <p>The DerCoders contains the der types that can decode the
 * subjectPublicKey, such as VtDerCoderDSAPublicKey.
 */
typedef struct
{
  VtDerCoder **derCoders;
  unsigned int derCoderCount;
  unsigned char *certRequest;
  unsigned int certRequestLen;
} VtCertRequestInfo;

/*@}*/

/*=========================================================*/
/*                                                         */
/* Certificates                                            */
/*                                                         */
/*=========================================================*/

/**
 * \defgroup CertGroup Certificate Object
 */

/*@{*/

/** The certificate object.
 * <p>Note that the object is a pointer type.
 */
typedef struct VtCertObjectDef *VtCertObject;

/** The function VtCreateCertObject builds a cert object using a
 * VtCertImpl. This typedef defines what a VtCertImpl is. Although a
 * VtCertImpl is a function pointer, an application should never call
 * one directly, only pass it as an argument to VtCreateCertObject.
 */
typedef int VT_CALLING_CONV (VtCertImpl) (
   VtCertObject *, Pointer, unsigned int);

/** The functions VtSetCertParam and VtGetCertParam add or get
 * information to or from a cert object. The information to add or get
 * is defined by a VtCertParam. This typedef defines what a VtCertParam
 * is. Although a VtCertParam is a function pointer, an application
 * should never call one directly, only pass it as an argument to
 * VtSetCertParam or VtGetCertParam.
 */
typedef int VT_CALLING_CONV (VtCertParam) (
   VtCertObject, Pointer, unsigned int);

/** Create a new cert object. This allocates space for an "empty"
 * object, then loads the given CertImpl to make it an "active" object.
 * <p>The VtCertImpl defines some of the cert object operations.
 * The include file vibe.h defines the supported CertImpls. Look
 * through the include file to see which CertImpl to use for your
 * application. All supported CertImpls will be defined as in the
 * following example.
 * <code>
 * <pre>
 *   extern VtCertImpl VtCertImplMpCtx;
 * </pre>
 * </code>
 * <p>Associated with each CertImpl is specific info. The documentation
 * for each CertImpl will describe the associated info it needs. That
 * data could be another object, it could be data in a particular
 * struct, it might be a NULL pointer. Check the documentation for each
 * CertImpl for a description of the data and its required format.
 * <p>To use this function decide which CertImpl you want to use, then
 * determine what information that CertImpl needs and in which format it
 * is presented. Collect the data in the appropriate format then call
 * this function passing in the desired CertImpl and the required info.
 * The associated info must be cast to Pointer.
 * <p>For example, if the required info is a VtItem struct, declare a
 * variable to be of type VtItem, fill in the fields, then pass the address
 * of that struct cast to Pointer.
 * <p>The input certObj is a pointer to an object. It should point to a
 * NULL VtCertObject. This function will go to the address given and
 * deposit a created object.
 * <p>If you call Create, you must call Destroy after you are done with
 * the object but before the program exits.
 * <p>Example:
 * <code>
 * <pre>
 *    int status;
 *    VtCertObject certObj = (VtCertObject)0;
 *
 *    do {
 *          . . .
 *      status = VtCreateCertObject (
 *        libCtx, VtCertImplBasic, (Pointer)0, &certObj);
 *      if (status != 0)
 *        break;
 *          . . .
 *    } while (0);
 *
 *    VtDestroyCertObject (&certObj);
 * </pre>
 * </code>
 * @param libCtx The library context.
 * @param certImpl The implementation the object will use.
 * @param associatedInfo The info needed by the CertImpl.
 * @param certObj A pointer to where the routine will deposit the
 * created object.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtCreateCertObject (
   VtLibCtx libCtx,
   VtCertImpl certImpl,
   Pointer associatedInfo,
   VtCertObject *certObj
);

/* These are the VtCertImpls supported by the toolkit. Each CertImpl is
 * used in conjunction with special info for the function. If there is
 * no special info, the accompaniment is a NULL pointer.
 */

/** This VtCertImpl is used to load up the MpIntCtx the object will
 * use when dealing with public keys. For the toolkit to operate on
 * public keys, it must have an MpIntCtx.
 * <p>If the associatedInfo passed in with this Impl is NULL, then the
 * Impl will try to find an mpCtx in the libCtx.
 * <p>The data associated with VtCertImplMpCtx is a VtMpIntCtx.
 */
extern VtCertImpl VtCertImplMpCtx;

/** When building a cert object, use this VtCertImpl if the mpCtx
 * desired is loaded into the libCtx.
 * <p>A cert object cannot operate without an mpCtx, so do not use
 * this Impl unless the libCtx contains an mpCtx.
 * <p>The data associated with VtCertImplBasic is a NULL pointer:
 * (Pointer)0.
 */
extern VtCertImpl VtCertImplBasic;

/** Destroy a Cert Object. This frees up any memory allocated during
 * the object's creation and use.
 * <p>If you call Create, you must call Destroy after you are done with
 * the object but before the program exits.
 * <p>Example:
 * <code>
 * <pre>
 *    int status;
 *    VtCertObject certObj = (VtCertObject)0;
 *
 *    do {
 *          . . .
 *      status = VtCreateCertObject (
 *        libCtx, VtCertImplBasic, (Pointer)0, &certObj);
 *      if (status != 0)
 *        break;
 *          . . .
 *    } while (0);
 *
 *    VtDestroyCertObject (&certObj);
 * </pre>
 * </code>
 * @param certObj A pointer to where the routine will find the object
 * to destroy.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtDestroyCertObject (
   VtCertObject *certObj
);

/** Set the cert object with the information given.
 * <p>The VtCertParam defines what information the object will be set
 * with.
 * <p>The include file vibe.h defines the supported CertParams. Look
 * through the include file to see which CertParam to use for your
 * application. All supported CertParams will be defined as in the
 * following example.
 * <code>
 * <pre>
 *   extern VtCertParam VtCertParamX509Der;
 * </pre>
 * </code>
 * <p>Associated with each CertParam is specific info. The documentation
 * for each CertParam will describe the associated info it needs. That
 * data could be another object, it could be data in a particular
 * struct, it might be a NULL pointer. Check the documentation for each
 * CertParam for a description of the data and its required format.
 * <p>To use this function decide which CertParam you want to use, then
 * determine what information that CertParam needs and in which format
 * it is presented. Collect the data in the appropriate format then call
 * this function passing in the desired CertParam and the required info.
 * The associated info must be cast to Pointer.
 * <p>For example, if the required info is a particular struct, declare
 * a variable to be of that type struct, fill in the fields, then pass
 * the address of that struct cast to Pointer.
 *
 * @param certObj The object to set.
 * @param certParam The type of info to set.
 * @param associatedInfo The info needed by the CertParam.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtSetCertParam (
   VtCertObject certObj,
   VtCertParam certParam,
   Pointer associatedInfo
);

/** Get the specified type of information out of a cert object.
 * <p>This function will go to the address given by getInfo and deposit
 * a pointer. That pointer is the address of the data. The memory to
 * hold the information belongs to the object, do not free it.
 * <p>The VtCertParam will specify what kind of information will be
 * returned, the getInfo is where the function will deposit the info.
 * <p>The include file vibe.h defines the supported
 * CertParams. Look through the include file to see which CertParam to
 * use for your application.
 * <p>See also VtSetCertParam.
 * <p>To use this function decide which CertParam you want to use, then
 * determine what information that CertParam will return and in which
 * format it is presented. Declare a variable to be a pointer to the
 * appropriate type, then call this function passing in the desired
 * CertParam and the address of the variable. Cast the address of the
 * pointer to the appropriate type: (Pointer *).
 * <p>If the associated info is already a pointer type (such as an
 * object), declare a variable to be of the appropriate type and pass
 * the address of that variable cast to (Pointer *).
 * <p>For example, if the returned info is a VtItem struct, declare a
 * variable to be of type (VtItem *), pass the address of that variable
 * (&varName) cast to (Pointer *).
 *
 * @param certObj The object containing the data to get.
 * @param certParam The type of info to get.
 * @param getInfo The address of a pointer. The function will go to
 * that address and deposit a pointer to the info requested.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtGetCertParam (
   VtCertObject certObj,
   VtCertParam certParam,
   Pointer *getInfo
);

/* These are the CertParams supported by the toolkit. Each CertParam
 * is used in conjunction with special info for the function.
 */

/** SetParam only.
 * <p>Use this VtCertParam to set an object with the DER encoding of an
 * X.509 cert. When setting with this VtCertParam, the toolkit will not
 * verify anything about the cert passed in, it will simply parse it
 * enough to confirm that the data is indeed an X.509 cert and keep an
 * internal copy of the info.
 * <p>The data associated with VtCertParamX509Der is a pointer to a
 * VtCertInfo struct that gives the data and length of the DER of the
 * X.509 cert, along with a DER coder array, which should contain the
 * DerCoder that understands the key inside the cert.
 */
extern VtCertParam VtCertParamX509Der;

/** GetParam only.
 * <p>Use this VtCertParam to get the cert data out of the object. The
 * object must already contain an X.509 cert (built by calling
 * VtObtainPrivateKeysAndCert, most likely).
 * <p>The associated info is a VtItem.
 * <p>When getting, pass in the address of a pointer as the getInfo,
 * the Get function will desposit a pointer to a VtItem struct at the
 * address. The VtItem will give the data and length of the DER of the
 * X.509 cert.
 */
extern VtCertParam VtCertParamX509DerData;

/** This data struct is used to pass arrays of certs around. For
 * example, see VtDistrictParamTrustedCerts.
 */
typedef struct
{
  unsigned int count;
  VtCertObject *certObjects;
} VtCertObjectList;

/*@}*/

/*=========================================================*/
/*                                                         */
/* Verify Failure List                                     */
/*                                                         */
/*=========================================================*/

/**
 * \defgroup VfyFailListGroup Verify Failure List
 */

/*@{*/

/** In previous versions of the toolkit, we had a placeholder type
 * called VtVerifyStack. There was no implementation, so for any
 * function or data struct that had the VtVerifyStack as an argument or
 * field, applications would use a NULL pointer.
 * <p>With version 2.5 of the toolkit, we are implementing this
 * element, but have decided it would be better to call it
 * VtVerifyFailureList. This entity contains the list of reasons a
 * particular verification failed. After making a verify call that has
 * the ability to use a VtVerifyFailureList (e.g. see
 * VtSecureMailVerify), you can check the List to see which function or
 * functions found problems with the verifications and what the
 * specific problems were.
 * <p>This #define is here so that previous applications that set the
 * old VerifyStack argument to NULL, will still be valid.
 */
#define VtVerifyStack VtVerifyFailureList

/** The VerifyFailureList.
 * <p>Note that the List is a pointer type.
 */
typedef struct VtVerifyFailureListDef *VtVerifyFailureList;

/** The function VtCreateVerifyFailureList builds a verify failure list
 * using a VtVerifyFailureListImpl. This typedef defines what a
 * VtVerifyFailureListImpl is. Although a VtVerifyFailureListImpl is a
 * function pointer, an application should never call one directly,
 * only pass it as an argument to VtCreateVerifyFailureList.
 */
typedef int VT_CALLING_CONV (VtVerifyFailureListImpl) (
   VtVerifyFailureList *, Pointer, unsigned int);

/** When you want to get the reason a verification failed, call
 * VtGetVerifyFailureListEntry. It will return a pointer to the
 * following struct.
 * <p>Note that there can be more than one failure (hence, the "list"
 * of "verify list"). Each failure will have its own
 * VtVerifyFailureReason.
 * <p>If a cert failed, that cert will be included in the failedCert
 * field. This will be a VtCertObject. Use VtGetCertParam to find out
 * what info you want to know about the cert.
 * <p>The failureReasonString is likely to be an ASCII string, but some
 * implementations of a VerifyFailureList might use a different
 * character set. The string is NULL-terminated, but the length (in
 * bytes, not characters) is given as well, it is the length not
 * counting the NULL terminator.
 */
typedef struct
{
  unsigned int            failureReason;
  unsigned char          *failureReasonString;
  unsigned int            stringLen;
  VtCertObject            failedCert;
} VtVerifyFailureReason;

/* These are the possible failure reasons, they are the values that the
 * failureReason field in the VtVerifyFailureReason struct might be. A
 * reason of 0 means no failure.
 * <p>Some failures are listed as "fatal". These are errors that an app
 * should probably not allow a user to ignore. For example, an expired
 * cert might be something a user can ignore, an app might want to let
 * the user know the cert is expired and let it pass anyway. However,
 * if the PKCS #7 authenticated attribute of the digest fails, an app
 * should not give the user a choice, because this failure is
 * equivalent to a signature failing.
 */

/** This is the value of the failureReason field in the
 * VtVerifyFailureReason struct when the reason a verification failed
 * is that the toolkit could not read something. Maybe a DER encoding
 * was wrong, maybe a number was out of range, or something else. It's
 * a general failure when the toolkit simply could not understand
 * something.
 * <p>This should be a fatal failure.
 */
#define VT_VFY_FAIL_REASON_UNKNOWN_VALUE          0xffff
/** This is the value of the failureReason field in the
 * VtVerifyFailureReason struct when the reason a verification failed
 * is that the the verify code could find no cert from which to extract
 * a public key to use to verify a signature.
 * <p>This should be a fatal failure.
 */
#define VT_VFY_FAIL_REASON_NO_CERT                0x0001
/** This is the value of the failureReason field in the
 * VtVerifyFailureReason struct when the reason a verification failed
 * is that the the signature on a message did not verify.
 * <p>This should be a fatal failure, the signature could not even be
 * checked.
 */
#define VT_VFY_FAIL_REASON_MSG_SIGNATURE          0x0002
/** This is the value of the failureReason field in the
 * VtVerifyFailureReason struct when the reason a verification failed
 * is that the signature time is outside the validity period of the
 * cert.
 */
#define VT_VFY_FAIL_REASON_CERT_VALIDITY          0x0010
/** This is the value of the failureReason field in the
 * VtVerifyFailureReason struct when the reason a verification failed
 * is that the KeyUsage extension in the cert indicated that the key is
 * not allowed to be used for what it is being used.
 * <p>For example, a cert is being used as a CA cert, but the KeyUsage
 * extension says it's not allowed to be a CA cert.
 * <p>This should be a fatal failure.
 */
#define VT_VFY_FAIL_REASON_CERT_KEY_USAGE         0x0011
/** This is the value of the failureReason field in the
 * VtVerifyFailureReason struct when the reason a verification failed
 * is that the toolkit could not chain up to a root. It could not find
 * the necessary certs, or could not find a trusted root.
 * <p>This should be a fatal failure.
 */
#define VT_VFY_FAIL_REASON_CERT_CHAIN             0x0012
/** This is the value of the failureReason field in the
 * VtVerifyFailureReason struct when the reason a verification failed
 * is that the PKCS #7 authenticated attribute "ContentType" did not
 * match.
 */
#define VT_VFY_FAIL_REASON_P7_ATTR_CONTENT_TYPE   0x0101
/** This is the value of the failureReason field in the
 * VtVerifyFailureReason struct when the reason a verification failed
 * is that the PKCS #7 authenticated attribute digest did not match the
 * computed digest.
 * <p>This should be a fatal failure. This is equivalent to a signature
 * failing. The Digest attribute is the digest of the message data
 * itself, the actual signature itself is applied to the authenticated
 * attributes.
 */
#define VT_VFY_FAIL_REASON_P7_ATTR_DIGEST         0x0102
/** This is the value of the failureReason field in the
 * VtVerifyFailureReason struct when the reason a verification failed
 * is that the a district cert is for a district that is not the
 * current district.
 * <p>This might not be a fatal failure, you might want to accept a
 * cert that was signed using an older district cert, maybe the message
 * is old or a rollover was recent.
 */
#define VT_VFY_FAIL_REASON_DIST_NOT_CURRENT       0x0201

/** Create a new verify failure list. This allocates space for an
 * "empty" list, then loads the given VerifyFailureListImpl to make it
 * "active".
 * <p>The VtVerifyFailureListImpl defines some of the list operations.
 * The include file vibe.h defines the supported
 * VerifyFailureListImpls. Look through the include file to see which
 * VerifyFailureListImpl to use for your application. All supported
 * VerifyFailureListImpls will be defined as in the following example.
 * <code>
 * <pre>
 *   extern VtVerifyFailureListImpl VtVerifyFailureListImplBasic;
 * </pre>
 * </code>
 * <p>Associated with each VerifyFailureListImpl is specific info. The
 * documentation for each VerifyFailureListImpl will describe the
 * associated info it needs. That data could be another object, it
 * could be data in a particular struct, it might be a NULL pointer.
 * Check the documentation for each VerifyFailureListImpl for a
 * description of the data and its required format.
 * <p>To use this function decide which VerifyFailureListImpl you want
 * to use, then determine what information that VerifyFailureListImpl
 * needs and in which format it is presented. Collect the data in the
 * appropriate format then call this function passing in the desired
 * VerifyFailureListImpl and the required info. The associated info
 * must be cast to Pointer.
 * <p>For example, if the required info is a VtItem struct, declare a
 * variable to be of type VtItem, fill in the fields, then pass the
 * address of that struct cast to Pointer.
 * <p>The input argument verifyFailureList is a pointer to a List. It
 * should point to a NULL VtVerifyFailureList. This function will go to
 * the address given and deposit a created List.
 * <p>If you call Create, you must call Destroy after you are done with
 * the List but before the program exits.
 * <p>Example:
 * <code>
 * <pre>
 *    int status;
 *    VtVerifyFailureList vfyFailList = (VtVerifyVerifyFailureList)0;
 *
 *    do {
 *          . . .
 *      status = VtCreateVerifyFailureList (
 *        libCtx, VtVerifyFailureListImplBasic, (Pointer)0, &vfyFailList);
 *      if (status != 0)
 *        break;
 *          . . .
 *    } while (0);
 *
 *    VtDestroyVerifyFailureList (&vfyFailList);
 * </pre>
 * </code>
 * @param libCtx The library context.
 * @param verifyFailureListImpl The implementation the List will use.
 * @param associatedInfo The info needed by the VerifyFailureListImpl.
 * @param verifyFailureList A pointer to where the routine will deposit
 * the created List.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtCreateVerifyFailureList (
   VtLibCtx libCtx,
   VtVerifyFailureListImpl verifyFailureListImpl,
   Pointer associatedInfo,
   VtVerifyFailureList *verifyFailureList
);

/* These are the VtVerifyFailureListImpls supported by the toolkit.
 * Each VerifyFailureListImpl is used in conjunction with special info
 * for the function. If there is no special info, the accompaniment is
 * a NULL pointer.
 */

/** This Impl will load the basic verify failure list implementation.
 * <p>The data associated with VtVerifyFailureListImplBasic is a NULL
 * pointer: (Pointer)0.
 */
extern VtVerifyFailureListImpl VtVerifyFailureListImplBasic;

/** Destroy a Verify Failure List. This frees up any memory allocated
 * during the stack's creation and use.
 * <p>If you call Create, you must call Destroy after you are done with
 * the object but before the program exits.
 * <p>Example:
 * <code>
 * <pre>
 *    int status;
 *    VtVerifyFailureList vfyFailList = (VtVerifyVerifyFailureList)0;
 *
 *    do {
 *          . . .
 *      status = VtCreateVerifyFailureList (
 *        libCtx, VtVerifyFailureListImplBasic, (Pointer)0, &vfyFailList);
 *      if (status != 0)
 *        break;
 *          . . .
 *    } while (0);
 *
 *    VtDestroyVerifyFailureList (&vfyFailList);
 * </pre>
 * </code>
 * @param verifyFailureList A pointer to where the routine will find
 * the List to destroy.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtDestroyVerifyFailureList (
   VtVerifyFailureList *verifyFailureList
);

/** This function will clear the List of any entries, freeing memory
 * allocated to hold those entries.
 * <p>After performing a verification, you can clear a
 * VerifyFailureList and use it again.
 * <p>Actually, any function that uses a VerifyFailureList will clear
 * it before using it. That is, if you don't clear the List, the
 * toolkit will clear it before starting a new verify operation.
 * <p>Upon return, the list will have no entries, its verifyResult will
 * be "True" (non-zero) and the primary failureReson will be 0.
 *
 * @param verifyFailureList The List to clear.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtClearVerifyFailureList (
   VtVerifyFailureList verifyFailureList
);

/** After a call to a function that uses a verify failure list returns
 * with a verification failure, call this function to find out how many
 * failure reasons are in the List.
 * <p>A verification might fail for more than one reason. For example,
 * the toolkit might check a leaf cert and find it is expired. However,
 * the toolkit will continue checking because the app might want to
 * accept a verification even though a cert is out of date. After
 * checking more, the toolkit might find that a signing cert does not
 * have the KeyUsage extension. In this case, there are two
 * verification failures, the leaf cert out of date, the CA cert
 * missing an extension. The list will contain both those reasons. The
 * app can then cycle through the reasons and decide (maybe with input
 * from the user) whether to accept the verification anyway. If a
 * reason is signature failure, then the app will not accept the
 * verification, no option.
 * <p>This function will go to the address given by count and deposit
 * an unsigned int there, the number of failures. 
 * <p>If you call this function and there are no failures in the list,
 * the function will set count to 0.
 *
 * @param verifyFailureList The List to query.
 * @param count The address where the function will depoisit the count.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtGetVerifyFailureCount (
   VtVerifyFailureList verifyFailureList,
   unsigned int *count
);

/** Get the VtVerifyFailureReason entry at the given index. A verify
 * failure list contains a series of entries, each describing a reason
 * the verification failed. 
 * <p>A verification might fail for more than one reason. For example,
 * the toolkit might check a leaf cert and find it is expired. However,
 * the toolkit will continue checking because the app might want to
 * accept a verification even though a cert is out of date. After
 * checking more, the toolkit might find that a signing cert does not
 * have the KeyUsage extension. In this case, there are two
 * verification failures, the leaf cert out of date, the CA cert
 * missing an extension. The list will contain both those reasons. The
 * app can then cycle through the reasons and decide (maybe with input
 * from the user) whether to accept the verification anyway. If a
 * reason is signature failure, then the app will not accept the
 * verification, no option.
 * <p>The caller passes the address of a pointer. The function will go
 * to that address and deposit a pointer to a VtVerifyFailureReason.
 * The app can then follow that pointer to find the info. All memory at
 * that pointer belongs to the toolkit, do not alter or free it.
 * <p>The indices of the entries are from 0 (zero) to count - 1, where
 * count is the total number of entries (see VtGetVerifyFailureCount to
 * get the count).
 * <p>The entry at index 0 is the primary failure reason, or the first
 * failure encountered.
 * <p>If the index is beyond the range of entries in the list, this
 * function will return an error.
 *
 * @param verifyFailureList The List to query.
 * @param index Which entry to get.
 * @param entry The address where the function will deposit a pointer
 * to a VtVerifyFailureReason struct conaining the info.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtGetVerifyFailureEntry (
   VtVerifyFailureList verifyFailureList,
   unsigned int index,
   VtVerifyFailureReason **entry
);

/*@}*/

/*=========================================================*/
/*                                                         */
/* PKCS #7 Object (P7 = PKCS #7)                           */
/*                                                         */
/*=========================================================*/

/**
 * \defgroup PKCS7Group PKCS#7 Object
 */

/*@{*/

/** The PKCS #7 object (in the crypto industry, P7 is a common
 * abbreviation of PKCS #7).
 * <p>Note that the object is a pointer type.
 */
typedef struct VtPkcs7ObjectDef *VtPkcs7Object;

/** The function VtCreatePkcs7Object builds an object that builds or
 * reads PKCS #7 messages using a VtPkcs7Impl. This typedef defines
 * what a VtPkcs7Impl is. Although a VtPkcs7Impl is a function pointer,
 * an application should never call one directly, only pass it as an
 * argument to VtCreatePkcs7Object.
 */
typedef int VT_CALLING_CONV (VtPkcs7Impl) (
   VtPkcs7Object *, Pointer, unsigned int);

/** The functions VtSetPkcs7Param and VtGetPkcs7Param add or get
 * information to or from a P7 object. The information to add or get is
 * defined by a VtPkcs7Param. This typedef defines what a VtPkcs7Param
 * is. Although a VtPkcs7Param is a function pointer, an application
 * should never call one directly, only pass it as an argument to
 * VtSetPkcs7Param or VtGetPkcs7Param.
 */
typedef int VT_CALLING_CONV (VtPkcs7Param) (
   VtPkcs7Object, Pointer, unsigned int);

/** Use this flag to indicate that a message should be constructed
 * using PKCS #7 version 1.5.
 * <p>Currently PKCS #7 is only "released" in version 1.5, but a
 * version 1.6 is in the works.
 */
#define VT_PKCS7_VERSION_1_5   15

/* These flags indicate what contentType is in a PKCS #7 message. They
 * are also the values of the last byte in PKCS #7 contentType OIDs,
 * so don't change these #defines.
 */

/** A flag indicating the contents of a PKCS #7 message is "Data".
 */
#define VT_PKCS7_DATA            1
/** A flag indicating the contents of a PKCS #7 message is "SignedData".
 */
#define VT_PKCS7_SIGNED_DATA     2
/** A flag indicating the contents of a PKCS #7 message is
 * "EnvelopedData".
 */
#define VT_PKCS7_ENVELOPED_DATA  3
/** A flag indicating the contents of a PKCS #7 message is
 * "SignedAndEnvelopedData".
 */
#define VT_PKCS7_SIGN_ENV_DATA   4
/** A flag indicating the contents of a PKCS #7 message is
 * "DigestedData".
 */
#define VT_PKCS7_DIGESTED_DATA   5
/** A flag indicating the contents of a PKCS #7 message is
 * "EncryptedData".
 */
#define VT_PKCS7_ENCRYPTED_DATA  6

/** Create a new Pkcs7 object. This allocates space for an "empty" object,
 * then loads the given Pkcs7Impl to make it an "active" object.
 * <p>The VtPkcs7Impl defines what sort of message the object will be
 * able to process (generally reading and writing will be different
 * Impls). The include file vibe.h defines the supported Pkcs7Impls.
 * Look through the include file to see which Pkcs7Impl to use for your
 * application. All supported Pkcs7Impls will be defined as in the
 * following example.
 * <code>
 * <pre>
 *   extern VtPkcs7Impl VtPkcs7ImplWriteSignedDSA;
 * </pre>
 * </code>
 * <p>Associated with each Pkcs7Impl is specific info. The
 * documentation for each Pkcs7Impl will describe the associated info
 * it needs. That data could be another object, it could be data in a
 * particular struct, it might be a NULL pointer. Check the
 * documentation for each Pkcs7Impl for a description of the data and
 * its required format.
 * <p>To use this function decide which Pkcs7Impl you want to use, then
 * determine what information that Pkcs7Impl needs and in which format
 * it is presented. Collect the data in the appropriate format then call
 * this function passing in the desired Pkcs7Impl and the required info.
 * The associated info must be cast to Pointer.
 * <p>For example, if the required info is a VtItem struct, declare a
 * variable to be of type VtItem, fill in the fields, then pass the address
 * of that struct cast to Pointer.
 * <p>The input pkcs7Obj is a pointer to an object. It should point to
 * a NULL VtPkcs7Object. This function will go to the address given and
 * deposit a created object.
 * <p>If you call Create, you must call Destroy after you are done with
 * the object but before the program exits.
 * <p>Example:
 * <code>
 * <pre>
 *    int status;
 *    VtPkcs7Object pkcs7Obj = (VtPkcs7Object)0;
 *
 *    do {
 *          . . .
 *      status = VtCreatePkcs7Object (
 *        libCtx, VtPkcs7ImplWriteSignedDSA, (Pointer)0, &pkcs7Obj);
 *      if (status != 0)
 *        break;
 *          . . .
 *    } while (0);
 *
 *    VtDestroyPkcs7Object (&pkcs7Obj);
 * </pre>
 * </code>
 * @param libCtx The library context.
 * @param pkcs7Impl The implementation the object will use.
 * @param associatedInfo The info needed by the Pkcs7Impl.
 * @param pkcs7Obj A pointer to where the routine will deposit the
 * created object.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtCreatePkcs7Object (
   VtLibCtx libCtx,
   VtPkcs7Impl pkcs7Impl,
   Pointer associatedInfo,
   VtPkcs7Object *pkcs7Obj
);

/* These are the VtPkcs7Impls supported by the toolkit. Each
 * VtPkcs7Impl is used in conjunction with special info for the
 * function. If there is no special info, the accompaniment is a NULL
 * pointer.
 */

/** Use this Impl to build a P7 object that will be able to create
 * SignedData messages. For this Impl, the data type of the underlying
 * contentInfo will be "Data" (as opposed to EnvelopedData or
 * DigestedData, etc.). This Impl will use version 1.5 of PKCS #7.
 * <p>The data associated with VtPkcs7ImplWriteSignedDSA is a NULL
 * pointer: (Pointer)0.
 */
extern VtPkcs7Impl VtPkcs7ImplWriteSignedDSA;

/** Use this Impl to build a P7 object that will be able to create
 * EnvelopedData messages. For this Impl, the data type of the
 * underlying contentInfo will be "Data" (as opposed to SignedData or
 * DigestedData, etc.). This Impl will use version 1.5 of PKCS #7.
 * <p>This Impl will use IBE as the enveloping algorithm (encrypting
 * the symmetric key). The app must call VtSetPkcs7Param with a Param
 * that specifies the symmetric algorithm (such as
 * VtPkcs7ParamEnv3DESCBC or VtPkcs7ParamEnvAESCBC).
 * <p>The data associated with VtPkcs7ImplWriteEnvelopeIBE is a NULL
 * pointer: (Pointer)0.
 */
extern VtPkcs7Impl VtPkcs7ImplWriteEnvelopeIBE;

/** Use this Impl to build a P7 object that will be able to read
 * SignedData messages. This Impl will be able to read any contentType
 * of the contentInfo, but will return it undecoded. That is, the data
 * that was actually signed will be simply returned "as is". If it is,
 * for example, "EnvelopedData", the P7 object set with this Impl will
 * not decode the EnvelopedData, it will simply return the contents to
 * be decoded by another object.
 * <p>The data associated with VtPkcs7ImplReadSignedData is a pointer
 * to a VtReadPkcs7Info struct containing a DerCoder array, a
 * SchemaDecode array, and an MpIntCtx. The DerCoder array contains all
 * the algorithms the app is willing to support (keys in certs,
 * signature algorithms). The SchemaDecode array contains all the
 * identity types the app is willing to support. The MpIntCtx will be
 * used for the public key operations when verifying signatures. If no
 * DerCoders, SchemaDecodes or MpIntCtx is supplied, the Impl will try
 * to find these elements in the libCtx.
 * <p>If the accompanying info is a NULL pointer, this Impl will try to
 * find all the info it needs from the libCtx.
 */
extern VtPkcs7Impl VtPkcs7ImplReadSignedData;

/** Use this Impl to build a P7 object that will be able to read
 * EnvelopedData messages. This Impl will be able to read any
 * contentType of the encryptedContentInfo, but will return it
 * undecoded. That is, the data that was actually encrypted will be
 * simply returned "as is". If it is, for example, "SignedData", the P7
 * object set with this Impl will not decode the SignedData, it will
 * simply return the contents to be decoded by another object.
 * <p>The data associated with VtPkcs7ImplReadEnvelopedData is a
 * pointer to a VtReadPkcs7Info struct containing all the algorithms
 * (asymmetric and symmetric algorithms represented as DERCoders) and
 * identity schemas (represented as SchemaDecodes) the app is willing
 * to support. The mpCtx is for performing IBE private key operations
 * with the Identity eventually chosen to decrypt the message.
 * <p>If no DerCoders, SchemaDecodes, or mpCtx are supplied, the Impl
 * will try to find these elements in the libCtx.
 * <p>If the accompanying info is a NULL pointer, this Impl will try to
 * find all the info it needs from the libCtx.
 */
extern VtPkcs7Impl VtPkcs7ImplReadEnvelopedData;

/** This is the data struct to accompany VtPkcs7ImplReadSignedData and
 * VtPkcs7ImplReadEnvelopedData.
 */
typedef struct
{
  VtDerCoder **derCoders;
  unsigned int derCoderCount;
  VtIdentitySchemaDecode **decoders;
  unsigned int decoderCount;
  VtMpIntCtx mpCtx;
} VtReadPkcs7Info;

/** Destroy a Pkcs7 Object. This frees up any memory allocated during
 * the object's creation and use.
 * <p>If you call Create, you must call Destroy after you are done with
 * the object but before the program exits.
 * <p>Example:
 * <code>
 * <pre>
 *    int status;
 *    VtPkcs7Object pkcs7Obj = (VtPkcs7Object)0;
 *
 *    do {
 *          . . .
 *      status = VtCreatePkcs7Object (
 *        libCtx, VtPkcs7ImplWriteSignedDSA, (Pointer)0, &pkcs7Obj);
 *      if (status != 0)
 *        break;
 *          . . .
 *    } while (0);
 *
 *    VtDestroyPkcs7Object (&pkcs7Obj);
 * </pre>
 * </code>
 * @param pkcs7Obj A pointer to where the routine will find the object
 * to destroy.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtDestroyPkcs7Object (
   VtPkcs7Object *pkcs7Obj
);

/** Set the P7 object with the information given.
 * <p>The VtPkcs7Param defines what information the object will be set
 * with.
 * <p>The include file vibe.h defines the supported Pkcs7Params.
 * Look through the include file to see which Pkcs7Param to use for
 * your application. All supported Pkcs7Params will be defined as in
 * the following example.
 * <code>
 * <pre>
 *   extern VtPkcs7Param VtPkcs7ParamDataLen;
 * </pre>
 * </code>
 * <p>Associated with each Pkcs7Param is specific info. The
 * documentation for each Pkcs7Param will describe the associated info
 * it needs. That data could be another object, it could be data
 * in a particular struct, it might be a NULL pointer. Check the
 * documentation for each Pkcs7Param for a description of the data and
 * its required format.
 * <p>To use this function decide which Pkcs7Param you want to use,
 * then determine what information that ParameterParam needs and in
 * which format it is presented. Collect the data in the appropriate
 * format then call this function passing in the desired Pkcs7Param and
 * and the required info. The associated info must be cast to Pointer.
 * <p>For example, if the required info is a particular struct, declare
 * a variable to be of that type struct, fill in the fields, then pass
 * the address of that struct cast to Pointer.
 *
 * @param pkcs7Obj The object to set.
 * @param pkcs7Param What the object is being set to.
 * @param associatedInfo The info needed by the Pkcs7Param.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtSetPkcs7Param (
   VtPkcs7Object pkcs7Obj,
   VtPkcs7Param pkcs7Param,
   Pointer associatedInfo
);

/** Get the specified type of information out of a P7 object.
 * <p>This function will go to the address given by getInfo and
 * deposit a pointer. That pointer is the address of the data. The
 * memory to hold the information belongs to the object, do not free it.
 * <p>The VtPkcs7Param will specify what kind of information will be
 * returned, the getInfo is where the function will deposit the info.
 * <p>The include file vibe.h defines the supported Pkcs7Params. Look
 * through the include file to see which Okcs7Param to use for your
 * application.
 * <p>See also VtSetPkcs7Param.
 * <p>To use this function decide which Pkcs7Param you want to use,
 * then determine what information that Pkcs7Param will return and in
 * which format it is presented. Declare a variable to be a pointer to
 * the appropriate type, then call this function passing in the desired
 * Pkcs7Param and the address of the variable. Cast the address of the
 * pointer to the appropriate type: (Pointer *).
 * <p>If the associated info is already a pointer type (such as an
 * object), declare a variable to be of the appropriate type and pass
 * the address of that variable cast to (Pointer *).
 * <p>For example, if the returned info is a VtIdentityList, declare a
 * variable to be of type VtIdentityList, pass the address of that
 * variable (&varName) cast to (Pointer *).
 * <p>Example:
 * <code>
 * <pre>
 *    VtIdentityList getRecipList;
 *
 *    do {
 *          . . .
 *      status = VtGetPkcs7Param (
 *        p7Obj, VtPkcs7ParamRecipientList, (Pointer *)&getRecipList);
 *      if (status != 0)
 *        break;
 *          . . .
 *    } while (0);
 * </pre>
 * </code>
 * @param pkcs7Obj The object to query.
 * @param pkcs7Param Defines what info to get.
 * @param getInfo The address of a pointer. The function will go to
 * that address and deposit a pointer to the info requested.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtGetPkcs7Param (
   VtPkcs7Object pkcs7Obj,
   VtPkcs7Param pkcs7Param,
   Pointer *getInfo
);

/* These are the VtPkcs7Params supported by the toolkit. Each
 * VtPkcs7Param is used in conjunction with special info for the
 * function.
 */

/** SetParam only.
 * <p>Use this Param to set a P7 object with a signer represented as an
 * identity. With an object built with this Param, the toolkit will
 * obtain the signing key and cert during the call to VtWriteInit,
 * either from storage or from the key server.
 * <p>If you set a Pkcs7Object using this Param, do not set it with the
 * signing key and cert for the same identity using
 * VtPkcs7ParamSignerInfo. That is, for each signer, use either
 * VtPkcs7ParamSignerId or VtPkcs7ParamSignerInfo.
 * <p>The info associated with this Param is an identity object.
 */
extern VtPkcs7Param VtPkcs7ParamSignerId;

/** SetParam only.
 * <p>Use this Param to set a P7 object with info it will need in order
 * to sign a message. This will include a private key with which to
 * sign and optionally the cert associated with that private key. This
 * is information necessary to build the SignerInfo for that signer.
 * <p>The accompanying data includes an identity object. That info is
 * optional. If you have an identity only, the toolkit will obtain the
 * key and cert (VtPkcs7ParamSignerId). If you have the key and cert,
 * pass them in to save time. If you have the key, cert, and identity,
 * you can pass all three in. If the key and cert are not associated
 * with an identity (not an IBE-based sender), pass the key and cert
 * only.
 * <p>The private key object passed in at this time will not be cloned.
 * That is, the Pkcs7Object set with this Param will copy a reference
 * to that key. When the Pkcs7Object needs to sign (WriteFinal), it
 * will use that key object. If you use this Param, make sure the key
 * object stays unchanged until after WriteFinal.
 * <p>The info associated with this Param is a pointer to a
 * VtPkcs7SignerInfo struct.
 */
extern VtPkcs7Param VtPkcs7ParamSignerInfo;

/** SetParam only.
 * <p>Use this Param to add a cert to be included in a signed message.
 * A PKCS #7 signed message (SignedData, SignedAndEnvelopedData) will
 * contain an OPTIONAL list of certs. These are typically the cert
 * needed to verify the signature of the message along with the CA
 * certs needed to chain the leaf cert up to the root.
 * <p>Use this Param to add certs to be included in the message. A
 * toolkit PKCS #7 Impl will generally include the leaf cert but no
 * other. When using an Impl that does not automatically include
 * various certs, emply this Param to make sure the certs you want
 * included are included.
 * <p>The info associated with this Param is a cert object.
 */
extern VtPkcs7Param VtPkcs7ParamCert;

/** SetParam only.
 * <p>Use this Pkcs7Param with a P7 Object built to write
 * EnvelopedData, it indicates the object should use Triple DES in CBC
 * mode to encrypt the bulk data.
 * <p>The P7 object will generate the IV and Triple DES key. It will
 * generate the key data using the random object supplied by the
 * application during the call to VtPkcs7WriteInit.
 * <p>The data associated with VtPkcs7ParamEnv3DESCBC is random object
 * or a NULL pointer. The random object will be used to generate the
 * initialization vector. If no random object is passed in, the Param
 * will look in the libCtx for one. If there's no random in the libCtx,
 * the Param will use time of day as the seed for generating the IV
 * (see the User's Manual for information on seeds).
 */
extern VtPkcs7Param VtPkcs7ParamEnv3DESCBC;

/** SetParam only.
 * <p>Use this Pkcs7Param with a P7 Object built to write
 * EnvelopedData, it indicates the object should use AES-128 in CBC
 * mode to encrypt the bulk data.
 * <p>In creating the P7 message, the object will indicate, using an
 * algorithm identitifer, that the symmetric algorithm is AES-128 in
 * CBC mode. The OID is defined by NIST as 2.16.840.1.101.3.4.1.2. The
 * OID does not specify padding. In fact, in an email message, a NIST
 * representative explicitly stated that the OID does not specify
 * padding. However, the representative also suggested following the
 * instructions specified in Appendix A of NIST Special Publication
 * 800-38A. This appendix recommends applications pad data when using
 * AES in CBC mode, then gives one possible padding scheme which is not
 * the technique outlined in PKCS #5 (the most commonly used block
 * cipher padding scheme in use today). It further states, "For the
 * above padding method, the padding bits can be removed unambiguously,
 * provided the receiver can determine that the message is indeed
 * padded. One way to ensure that the receiver does not mistakenly
 * remove bits from an unpadded message is to require the sender to pad
 * every message, including messages in which the final block (segment)
 * is already complete. For such messages, an entire block (segment) of
 * padding is appended."
 * <p>A "standard" is supposed to specify how to do something in a
 * standard way, so that two entities can know what something means
 * without having to communicate anything in advance. But here is a
 * standard (along with the people wo wrote and disseminate it) that
 * explicitly states that the sender and receiver must communicate in
 * advance to resolve an intentional ambiguity in the standard.
 * <p>Fortunately, PKCS #7 specifies that all block ciphers in modes
 * that require complete blocks (such as CBC) shall pad (even if the
 * input length is a multiple of the block size) and shall pad
 * following the technique specified in PKCS #5.
 * <p>The P7 object will generate the IV and AES key. It will generate
 * the key data using the random object supplied by the application
 * during the call to VtPkcs7WriteInit.
 * <p>The data associated with VtPkcs7ParamEnvAES128CBC is random object
 * or a NULL pointer. The random object will be used to generate the
 * initialization vector. If no random object is passed in, the Param
 * will look in the libCtx for one. If there's no random in the libCtx,
 * the Param will use time of day as the seed for generating the IV
 * (see the User's Manual for information on seeds).
 */
extern VtPkcs7Param VtPkcs7ParamEnvAES128CBC;

/** SetParam and GetParam.
 * <p>Use this param to add a list of recipients to an object built to
 * write EnvelopedData.
 * <p>Or use this param to get the list of recipients of a P7
 * EnvelopedData message being read.
 * <p>When an object built to write EnvelopedData creates the digital
 * envelope, it will use each of the identities in the IdentityList to
 * build a RecipientInfo (which includes the session key encrypted
 * using the recipient's public key, which in this case is the
 * identity).
 * <p>When an object built to read EnvelopedData reads all the
 * RecipientInfos, it will build a VtIdentityList containing the
 * identities of all the recipients. The app can then get that list by
 * using this Pkcs7Param, and then examine each of the identities to
 * choose which one the object should use to open the envelope
 * (indicate which identity to use by calling VtSetPkcs7Param with
 * VtPkcs7ParamRecipientIndex).
 * <p>The info associated with this Param is a VtIdentityList object.
 * <p>When getting, pass in the address of a VtIdentityList variable as
 * the getInfo, the Get function will desposit a VtIdentityList at the
 * address.
 */
extern VtPkcs7Param VtPkcs7ParamRecipientList;

/** SetParam and GetParam.
 * <p>Use this param to add recipients to an object built to write
 * EnvelopedData, the recipients represented as an already existing
 * RecipientInfoList (as opposed to an IdentityList).
 * <p>Or use this param to get the recipients of a P7 EnvelopedData
 * message being read, represented as a RecipientInfoList.
 * <p>When an object built to write EnvelopedData creates the digital
 * envelope, it will use the encoded RecipientInfos from the associated
 * info, instead of building it. It will also use the symmetric key and
 * symmetric algorithm specified in the associated info.
 * <p>When Writing an EnvelopedData message, using this Param is
 * equivalent to using VtPkcs7ParamRecipientList along with either
 * VtPkcs7ParamEnv3DESCBC or VtPkcs7ParamEnvAES128CBC.
 * <p>When an object built to read EnvelopedData is done reading the
 * entire message, an app can use this Param to get the
 * RecipientInfoList representation of the recipients. The app can then
 * get information out of the RecipientInfoList, such as the symmetric
 * key and algorithm.
 * <p>Note! When reading, the CHOOSE_RECIPIENT error means you get an
 * IdentityList out of the P7 object and you pick an identity to use to
 * decrypt. You cannot get the RecipientInfoList at that time, you must
 * wait until reading the entire message.
 * <p>After Getting the RecipientInfoList out of a P7 object, you can
 * call VtEncodeRecipientInfoList. If you want the symmetric key
 * protected in the RecipientInfoBlock, Set it with a secretValue
 * first. NOTE!!! This is a departure from normal toolkit rules.
 * Normally we say do not alter objects returned from a Get call,
 * however, in this case, the object returned by the Get will have no
 * secretValue installed, so if you want to use one, you must add one.
 * <p>The info associated with this Param is a VtRecipientInfoList
 * object.
 * <p>NOTE!!! If setting a P7 object with a RecipientInfoList, each
 * RecipientInfo must be encoded. That is, you must have called
 * VtEncodeRecipientInfo on each RecipientInfoObject inside the
 * RecipientInfoList, or you must have called
 * VtEncodeRecipientInfoList on the RecipientInfoList.
 * <p>NOTE!!! If setting a P7 object with a RecipientInfoList, the
 * RecipientInfoList MUST be set so that it can produce the
 * RecipientInfoBlock. (See the documentation on
 * VtEncodeRecipientInfoList for info on obtaining RecipientInfoBlock
 * versus RecipientInfos.)
 * <p>When getting, pass in the address of a VtRecipientInfoList
 * variable as the getInfo, the Get function will desposit a
 * VtRecipientInfoList at the address.
 */
extern VtPkcs7Param VtPkcs7ParamRecipientInfoList;

/** GetParam only.
 * <p>Use this param to get the list of signers out of a P7 SignedData
 * message being read.
 * <p>When an object built to read SignedData reads all the
 * SignerInfos, it will build a VtIdentityList containing the
 * identities of all the signers. The app can then get that list by
 * using this Pkcs7Param.
 * <p>The info associated with this Param is a VtIdentityList object.
 * <p>When getting, pass in the address of a VtIdentityList variable as
 * the getInfo, the Get function will desposit a VtIdentityList at the
 * address.
 */
extern VtPkcs7Param VtPkcs7ParamSignerList;

/** SetParam only.
 * <p>Use this Pkcs7Param to let a P7 Object built to read
 * EnvelopedData know which identity to use to decrypt the message.
 * <p>When the return from VtPkcs7ReadUpdate is
 * VT_ERROR_CHOOSE_RECIPIENT, the app should get the recipientlist out
 * of of the P7 object (using VtPkcs7ParamRecipientList), examine the
 * identities of the recipients, and choose which one should be used to
 * decrypt the message. The identity chosen will have an index
 * associated with it, it is the index into the IdentityList of all the
 * recipients. This Param tells the P7 object which identity in the
 * IdentityList the object should use.
 * <p>Once the P7 object knows which identity to use, it will obtain
 * the IBE private key. Therefore, the app also passes in policy,
 * storage, and transport contexts for the P7 object to use to obtain
 * the private key. If no contexts are passed in, the P7 object will
 * look in the libCtx for those entities.
 * <p>Note that this will only obtain an IBE private key with which to
 * decrypt a session key. If the session key was encrypted with any
 * other algorithm, this Param will not be able to load the appropriate
 * private key needed.
 * <p>The data associated with VtPkcs7ParamRecipientIndex is a pointer
 * to a VtPkcs7RecipientIndexInfo struct.
 */
extern VtPkcs7Param VtPkcs7ParamRecipientIndex;

/** SetParam only.
 * <p>Use this Pkcs7Param with a P7 Object built to read EnvelopedData,
 * it indicates which identity to use to decrypt the message.
 * <p>When the return from VtPkcs7ReadUpdate is
 * VT_ERROR_CHOOSE_RECIPIENT, the app needs to tell the object which
 * recipient to use. Using this Param does that. This Param sets the P7
 * object with the IdentityObject and KeyObject provided, so that when
 * the P7 object needs to decrypt the session key, it will use the
 * given key object (it uses the identity object to find the
 * appropriate RecipientInfo).
 * <p>This Param is similar to VtPkcs7ParamRecipientIndex in that it
 * specifies the recipient to use, however using this Param means the
 * toolkit will not try to find a private key.
 * <p>If the identity given does not match any identity of the
 * message's actual recipients, the Read function will not be able to
 * decrypt the session key and therefore will not be able to decrypt
 * the message.
 * <p>The data associated with VtPkcs7ParamRecipient is a pointer to a
 * VtPkcs7RecipientInfo struct.
 * <p>NOTE !!! This param will copy a reference to the key object, not
 * clone it. That is, after setting with this Param, the P7 object will
 * contain a reference to the key object passed in, do not destroy that
 * object until after decrypting the message. The P7 object not do
 * anything to that key object, it will only use it to decrypt the
 * session key.
 */
extern VtPkcs7Param VtPkcs7ParamRecipient;

/** SetParam only
 * <p>Use this Param to indicate how long the data in a message will be.
 * Some Impls (SignedData and EnvelopedData for example) must know,
 * before actually writing data, how long the message will be. If the
 * message is not supplied to the object all at once (call WriteFinal
 * without any previous calls to WriteUpdate), use this Param.
 * <p>Note that this supplies the length of the actual data (for
 * instance, the data to sign or encrypt), not the message data.
 * <p>The info associated with this Param is a pointer to an unsigned
 * int giving the length of the data.
 */
extern VtPkcs7Param VtPkcs7ParamDataLen;

/** GetParam only
 * <p>A PKCS #7 SignedData message optionally contains the time it was
 * signed (the signingTimeAttribute). This Param gets that time out of
 * the message.
 * <p>Get this info only after completely reading the message.
 * <p>The info associated with this Param is a pointer to a VtTime
 * struct.
 * <p>When getting, pass in the address of a pointer as the getInfo,
 * the Get function will desposit a pointer to a VtTime struct at the
 * address.
 */
extern VtPkcs7Param VtPkcs7ParamSigningTime;

/** This is the data struct to accompany VtPkcs7ParamSignerInfo.
 * The senderId is optional. If you have it available, pass it in, if
 * not, leave the field NULL.
 * <p>If you have an identity only, the toolkit will obtain the key and
 * cert (VtSecureMailParamSenderId). If you have the key and cert, pass
 * them in to save time. If you have the key, cert, and identity, you
 * can pass all three in. If the key and cert are not associated with
 * an identity (not an IBE-based sender), pass the key and cert only.
 */
typedef struct
{
  VtKeyObject privateKey;
  VtCertObject signerCert;
  VtIdentityObject signerId;
} VtPkcs7SignerInfo;

/** This is the data struct to accompany VtPkcs7ParamRecipientIndex.
 */
typedef struct
{
  unsigned int index;
  VtPolicyCtx policyCtx;
  VtStorageCtx storageCtx;
  VtTransportCtx transportCtx;
} VtPkcs7RecipientIndexInfo;

/** This is the data struct to accompany VtPkcs7ParamRecipientIndex.
 */
typedef struct
{
  VtIdentityObject identity;
  VtKeyObject priKey;
} VtPkcs7RecipientInfo;

/** Initialize the P7 object for writing. The object should have been
 * created with an Impl that creates a message as opposed to reading a
 * message. This function will make sure it has all the information
 * necessary to write the message (except for the actual data itself)
 * and collect any "missing" information using the policy, storage, and
 * transport contexts.
 * <p>The app had the opportunity to add info to the object during the
 * Create and Set functions, but some of that info might be
 * "incomplete" and the Init function will find any needed elements.
 * For example, in order to write the message, the object may need a
 * particular IBE key. If the app added only an Identity object, the
 * Init function will use the contexts to retrieve or download the
 * appropriate information to build the key.
 * <p>If no policy, storage, and/or transport ctx are given, the
 * function will use the contexts found in the libCtx (if any).
 *
 * @param pkcs7Obj The object created and set to write a PKCS #7
 * message.
 * @param policyCtx The policy ctx to use if IBE operations are
 * performed.
 * @param storageCtx The storage ctx to use if the function needs to
 * retrieve or store elements needed to build the message.
 * @param transportCtx The transport ctx to use if IBE operations are
 * performed.
 * @param random A random object the function will use if it needs
 * random bytes (for a symmetric key or IV, for example).
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtPkcs7WriteInit (
   VtPkcs7Object pkcs7Obj,
   VtPolicyCtx policyCtx,
   VtStorageCtx storageCtx,
   VtTransportCtx transportCtx,
   VtRandomObject random
);

/** Write out message. This function will write out what it can. It
 * does not assume that it has all the data of the message yet. If this
 * is the first call to Update, it may write out some header information
 * (the initial 30 len along with the contentType, for example) along
 * with the processed data itself. That is, the output may be bigger
 * than the input.
 * <p>The function will deposit the output into the message buffer,
 * which is bufferSize bytes big. It will set the unsigned int at
 * messageLen to the number of bytes placed into the buffer. If the
 * buffer is not big enough, the function will return
 * VT_ERROR_BUFFER_TOO_SMALL and set messageLen to the required size.
 * <p>If the object was not set with a P7Impl that can write, the
 * function will return an error.
 *
 * @param pkcs7Obj The P7 object built to write.
 * @param random A random object that can generate random numbers if
 * necessary.
 * @param inputData The actual message data.
 * @param inputDataLen The length, in bytes, of the inputData.
 * @param message The output buffer, where the resulting P7 message
 * will be deposited.
 * @param bufferSize The size, in bytes, of the output buffer.
 * @param messageLen The address where the implementation will deposit
 * the length, the number of bytes placed into the output buffer.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtPkcs7WriteUpdate (
   VtPkcs7Object pkcs7Obj,
   VtRandomObject random,
   unsigned char *inputData,
   unsigned int inputDataLen,
   unsigned char *message,
   unsigned int bufferSize,
   unsigned int *messageLen
);

/** Finish writing out the message. This function will write out any
 * actual data left, then write out any trailing information (such as
 * certificates, SignerInfos, etc. for SignedData). This function will
 * match the total input to the specified input from the call to
 * SetPkcs7Param with the VtPkcs7ParamDataLen Param (if it was used).
 * If there had been no call to Update, this function may write out
 * some header information (the initial 30 len along with the
 * contentType, for example) along with the processed data itself. That
 * is, the output may be bigger than the input.
 * <p>The function will deposit the output into the message buffer,
 * which is bufferSize bytes big. It will set the unsigned int at
 * messageLen to the number of bytes placed into the buffer. If the
 * buffer is not big enough, the function will return
 * VT_ERROR_BUFFER_TOO_SMALL and set messageLen to the required size.
 * <p>If the object was not set with a P7Impl that can write, the
 * function will return an error.
 *
 * @param pkcs7Obj The P7 object built to write.
 * @param random A random object that can generate random numbers if
 * necessary.
 * @param inputData The actual message data.
 * @param inputDataLen The length, in bytes, of the inputData.
 * @param message The output buffer, where the resulting P7 message
 * will be deposited.
 * @param bufferSize The size, in bytes, of the output buffer.
 * @param messageLen The address where the implementation will deposit
 * the length, the number of bytes placed into the output buffer.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtPkcs7WriteFinal (
   VtPkcs7Object pkcs7Obj,
   VtRandomObject random,
   unsigned char *inputData,
   unsigned int inputDataLen,
   unsigned char *message,
   unsigned int bufferSize,
   unsigned int *messageLen
);

/** Initialize the P7 object for reading. The object should have been
 * created with an Impl that reads a message as opposed to writing a
 * message. This function will make sure it has all the information
 * necessary to read the message (except for the actual message itself).
 *
 * @param pkcs7Obj The object created and set to read a PKCS #7 message.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtPkcs7ReadInit (
   VtPkcs7Object pkcs7Obj
);

/** Begin or continue reading a P7 message. This function does not
 * demand that the entire P7 message be supplied, it will read whatever
 * is given.
 * <p>The function will go to the address given by bytesRead and
 * deposit there an unsigned int indicating how many bytes of the
 * message the function read. It is possible that the function does not
 * read all the bytes given. For example, if given the first part of a
 * SignedData message, the function may read the leading info
 * (contentInfo's OID, digest algId, etc.) but not the actual data to
 * verify itself. That can happen if the output buffer given is not big
 * enough to hold the data the function needs to return. Hence, the
 * function read up to the data, but stopped. The function will return
 * the bytes read and the BUFFER_TOO_SMALL error.
 * <p>An application should call Update, check the bytesRead, and if
 * the bytesRead is not the same as the input length, call Update
 * again, this time on the input data bytesRead further along. If the
 * bytesRead returned is the same as the messageLen, then the
 * application should call Update with the next block of input.
 * <p>If there is any data to return, (the content of the contentInfo),
 * the function will return it in the given outputData buffer
 * (bufferSize bytes big) and will place into the unsigned int at
 * outputDataLen the number of bytes placed into the buffer.  If the
 * buffer is not big enough, the routine will return
 * VT_ERROR_BUFFER_TOO_SMALL and set outputDataLen to the required size.
 * <p>It is possible that there is no outputData. For instance, if the
 * Update is called with only the first part of a P7 SignedData message,
 * and the data given contains only version and digest algorithm IDs,
 * then the function will read that data but have no content to return.
 * Therefore, if you call this function with a NULL output buffer, it
 * is still possible the return is 0 (no error) rather than the
 * BUFFER_TOO_SMALL error.
 *
 * @param pkcs7Obj The P7 object built to read.
 * @param message The current part of the P7 message.
 * @param messageLen The length, in bytes, of the message data.
 * @param bytesRead The address where the routine will deposit the
 * number of bytes of the message actually read. If 0, call the Update
 * function again with the same data. If messageLen, the function read
 * all the bytes, the next call should pass in the next bytes in the
 * message.
 * @param outputData The output buffer, where the resulting content
 * data will be deposited.
 * @param bufferSize The size, in bytes, of the output buffer.
 * @param outputDataLen The address where the function will deposit
 * the length, the number of bytes placed into the output buffer.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtPkcs7ReadUpdate (
   VtPkcs7Object pkcs7Obj,
   unsigned char *message,
   unsigned int messageLen,
   unsigned int *bytesRead,
   unsigned char *outputData,
   unsigned int bufferSize,
   unsigned int *outputDataLen
);

/** Finish reading a P7 message. This function will check to make sure
 * the entire P7 message was supplied, either during all the calls to
 * Update and this call to Final, or all at once if this is the only
 * call to Read.
 * <p>The function will go to the address given by bytesRead and
 * deposit there an unsigned int indicating how many bytes of the
 * buffer the function read. It is possible that the function does not
 * read all the bytes given. For example, if given the first part of a
 * SignedData message, the function may read the leading info
 * (contentInfo's OID, digest algId, etc.) but not the actual data to
 * verify itself. That can happen if the output buffer given is not big
 * enough to hold the data the function needs to return. Hence, the
 * function read up to the data, but stopped. The function will return
 * the bytes read and the BUFFER_TOO_SMALL error.
 * <p>An application should call Final, check the bytesRead, and if
 * the bytesRead is not the same as the input length, call Final again,
 * this time on the input data bytesRead further along. If the
 * bytesRead returned is the same as the messageLen, then the entire
 * message has been read.
 * <p>If there is any data to return, (the content of the contentInfo),
 * the function will return it in the given outputData buffer
 * (bufferSize bytes big) and will place into the unsigned int at
 * outputDataLen the number of bytes placed into the buffer.  If the
 * buffer is not big enough, the routine will return
 * VT_ERROR_BUFFER_TOO_SMALL and set outputDataLen to the required size.
 * <p>It is possible that there is no outputData. For instance, if
 * previous calls to Update collected all the actual data, this call to
 * Final will be reading only "trailing" information in the message.
 * <p>If the input does not complete a message, this function will
 * return an error.
 * <p>Note that for SignedData or SignedAndEnvelopedData, this function
 * does not verify signatures, it is only able to read the message. To
 * verify signatures, after a successful call to ReadFinal, call a
 * Pkcs7Verify function.
 *
 * @param pkcs7Obj The P7 object built to read.
 * @param message The current part of the P7 message.
 * @param messageLen The length, in bytes, of the message data.
 * @param bytesRead The address where the routine will deposit the
 * number of bytes of the message actually read. If 0, call the Update
 * function again with the same data. If messageLen, the function read
 * all the bytes, the next call should pass in the next bytes in the
 * message.
 * @param outputData The output buffer, where the resulting content
 * data will be deposited.
 * @param bufferSize The size, in bytes, of the output buffer.
 * @param outputDataLen The address where the function will deposit
 * the length, the number of bytes placed into the output buffer.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtPkcs7ReadFinal (
   VtPkcs7Object pkcs7Obj,
   unsigned char *message,
   unsigned int messageLen,
   unsigned int *bytesRead,
   unsigned char *outputData,
   unsigned int bufferSize,
   unsigned int *outputDataLen
);

/** Verify all signatures in a PKCS #7 message. If there is more than
 * one signer, this function will try to verify all signatures and all
 * signer certs. If one element does not verify, this function sets the
 * verifyResult to did not verify.
 * <p>Before calling this function the application must finish reading
 * the message. That is, do not call this function until after a
 * successful call to ReadFinal.
 * <p>The caller passes in policy, storage and transport contexts the
 * function will use to help it find certificates it needs to chain a
 * leaf cert. The function may download district parameters to obtain
 * certs.
 * <p>The caller also passes in a certVerifyCtx, which this function
 * will use to verify any untrusted certs it encounters. The caller
 * must also pass in the appropriate associated info (verifyCtxInfo)
 * for the particular certVerifyCtx. That is the info the specific ctx
 * needs to verify a cert. This associated info will be applied to each
 * leaf cert the function verifies.
 *
 * @param pkcs7Obj The object built to read and used to read an entire
 * P7 message.
 * @param policyCtx A policyCtx to use if necessary.
 * @param storageCtx A storageCtx to use to help find any certs needed
 * to verify signatures or other certs.
 * @param transportCtx A transportCtx to use if necessary.
 * @param certVerifyCtx The certVerifyCtx the function will use when
 * verifying untrusted certs in a chain.
 * @param verifyCtxInfo The info the certVerifyCtx needs to help it
 * verify. This info will be applied to leaf certs.
 * @param vfyFailList Currently unused, reserved for future
 * implementations.
 * @param verifyResult The address where this function will deposit the
 * result of the verify, one for verify, zero for not verified.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtPkcs7VerifyAll (
   VtPkcs7Object pkcs7Obj,
   VtPolicyCtx policyCtx,
   VtStorageCtx storageCtx,
   VtTransportCtx transportCtx,
   VtCertVerifyCtx certVerifyCtx,
   Pointer verifyCtxInfo,
   VtVerifyFailureList vfyFailList,
   unsigned int *verifyResult
);

/** This function determines what the content type of a PKCS #7 message
 * is (Data, EnvelopedData, SignedData, etc.). It goes to the address
 * given by contentType and deposits one of the following flags.
 * <code>
 * <pre>
 *    VT_PKCS7_DATA
 *    VT_PKCS7_SIGNED_DATA
 *    VT_PKCS7_ENVELOPED_DATA
 *    VT_PKCS7_SIGN_ENV_DATA
 *    VT_PKCS7_DIGESTED_DATA
 *    VT_PKCS7_ENCRYPTED_DATA
 * </pre>
 * </code>
 * <p>Typically, an application will call this function to determine
 * the contentType of the message, then call VtCreatePkcs7Object with
 * the VtPkcs7Impl that knows how to read the particular content type.
 * <p>If there is not enough of the message to read the content type,
 * this function returns an error.
 *
 * @param libCtx The library context to use.
 * @param message The buffer containing the PKCS #7 message.
 * @param messageLen The length, in bytes, of the message.
 * @param contentType The address where the function will deposit a
 * flag indicating the content type.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtReadPkcs7ContentType (
   VtLibCtx libCtx,
   unsigned char *message,
   unsigned int messageLen,
   unsigned int *contentType
);

/** An X509CertList consists of an array of VtItems and the count, the
 * number of VtItems. Each VtItem is the data and length of an X.509
 * cert, the DER encoding.
 * <p>This data type is used by VtReadPkcs7DataAndVerify.
 */
typedef struct
{
  int count;
  VtItem *certs;
} VtX509CertList;

/*@}*/

/*=========================================================*/
/*                                                         */
/* RecipientInfo Object                                    */
/*                                                         */
/*=========================================================*/

/**
 * \defgroup RecipInfoGroup RecipientInfo Object
 */

/*@{*/

/** The RecipientInfo object.
 * <p>Note that the object is a pointer type.
 */
typedef struct VtRecipientInfoObjectDef *VtRecipientInfoObject;

/** The function VtCreateRecipientInfoObject builds a RecipientInfo
 * object using a VtRecipientInfoImpl. This typedef defines what a
 * VtRecipientInfoImpl is. Although a VtRecipientInfoImpl is a function
 * pointer, an application should never call one directly, only pass it
 * as an argument to VtCreateRecipientInfoObject.
 */
typedef int VT_CALLING_CONV (VtRecipientInfoImpl) (
   VtRecipientInfoObject *, Pointer, unsigned int);

/** The functions VtSetRecipientInfoParam and VtGetRecipientInfoParam
 * add or get information to or from a RecipientInfo object. The
 * information to add or get is defined by a VtRecipientInfoParam. This
 * typedef defines what a VtRecipientInfoParam is. Although a
 * VtRecipientInfoParam is a function pointer, an application should
 * never call one directly, only pass it as an argument to
 * VtSetRecipientInfoParam or VtGetRecipientInfoParam.
 */
typedef int VT_CALLING_CONV (VtRecipientInfoParam) (
   VtRecipientInfoObject, Pointer, unsigned int);

/** Create a new RecipientInfo object. This allocates space for an
 * "empty" object, then loads the given RecipientInfoImpl to make it an
 * "active" object.
 * <p>The VtRecipientInfoImpl defines some of the identity object
 * operations. The include file vibe.h defines the supported
 * RecipientInfoImpls. Look through the include file to see which
 * RecipientInfoImpl to use for your application. All supported
 * RecipientInfoImpls will be defined as in the following example.
 * <pre>
 * <code>
 *   extern VtRecipientInfoImpl VtRecipientInfoImplMpCtx;
 * </code>
 * </pre>
 * <p>Associated with each RecipientInfoImpl is specific info. The
 * documentation for each RecipientInfoImpl will describe the
 * associated info it needs. That data could be another object, it
 * could be data in a particular struct, it might be a NULL pointer.
 * Check the documentation for each RecipientInfoImpl for a description
 * of the data and its required format.
 * <p>To use this function decide which RecipientInfoImpl you want to
 * use, then determine what information that RecipientInfoImpl needs
 * and in which format it is presented. Collect the data in the
 * appropriate format then call this function passing in the desired
 * RecipientInfoImpl and the required info. The associated info must be
 * cast to Pointer.
 * <p>For example, if the required info is a VtItem struct, declare a
 * variable to be of type VtItem, fill in the fields, then pass the address
 * of that struct cast to Pointer.
 * <p>The input recipInfoObj is a pointer to an object. It should point
 * to a NULL VtRecipientInfoObject. This function will go to the
 * address given and deposit a created object.
 * <p>If you call Create, you must call Destroy after you are done with
 * the object but before the program exits.
 * <p>Example:
 * <pre>
 * <code>
 *    int status;
 *    VtRecipientInfoObject recipInfoObj = (VtRecipientInfoObject)0;
 *
 *    do {
 *          . . .
 *      status = VtCreateRecipientInfoObject (
 *        libCtx, VtRecipientInfoImplMpCtx, (Pointer)0, &recipInfoObj);
 *      if (status != 0)
 *        break;
 *          . . .
 *    } while (0);
 *
 *    VtDestroyRecipientInfoObject (&recipInfoObj);
 * </code>
 * </pre>
 *
 * @param libCtx The library context.
 * @param recipientInfoImpl The implementation the object will use.
 * @param associatedInfo The info needed by the RecipientInfoImpl.
 * @param recipInfoObj A pointer to where the routine will deposit the
 * created object.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtCreateRecipientInfoObject (
   VtLibCtx libCtx,
   VtRecipientInfoImpl recipientInfoImpl,
   Pointer associatedInfo,
   VtRecipientInfoObject *recipInfoObj
);

/* These are the VtRecipientInfoImpls supported by the toolkit. Each
 * RecipientInfoImpl is used in conjunction with special info for the
 * function. If there is no special info, the accompaniment is a NULL
 * pointer.
 */

/** This VtRecipientInfoImpl is used to load up the MpIntCtx the object
 * will use when performing IBE operations (encrypting or decrypting
 * symmetric keys). For the toolkit to perform IBE operations, it must
 * have an MpIntCtx.
 * <p>If the associatedInfo passed in with this Impl is NULL, then the
 * Impl will try to find an mpCtx in the libCtx.
 * <p>The data associated with VtRecipientInfoImplMpCtx is a VtMpIntCtx.
 */
extern VtRecipientInfoImpl VtRecipientInfoImplMpCtx;

/** Destroy a RecipientInfo Object. This frees up any memory allocated
 * during the object's creation and use.
 * <p>If you call Create, you must call Destroy after you are done with
 * the object but before the program exits.
 * <p>Example:
 * <code>
 * <pre>
 *    int status;
 *    VtRecipientInfoObject recipInfoObj = (VtRecipientInfoObject)0;
 *
 *    do {
 *          . . .
 *      status = VtCreateRecipientInfoObject (
 *        libCtx, VtRecipientInfoImplMpCtx, (Pointer)0, &recipInfoObj);
 *      if (status != 0)
 *        break;
 *          . . .
 *    } while (0);
 *
 *    VtDestroyRecipientInfoObject (&recipInfoObj);
 * </pre>
 * </code>
 *
 * @param recipInfoObj A pointer to where the routine will find the
 * object to destroy.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtDestroyRecipientInfoObject (
   VtRecipientInfoObject *recipInfoObj
);

/** Set the RecipientInfo object with the information given.
 * <p>The VtRecipientInfoParam defines what information the object will
 * be set with.
 * <p>The include file vibe.h defines the supported RecipientInfoParams.
 * Look through the include file to see which RecipientInfoParam to use
 * for your application. All supported RecipientInfoParams will be
 * defined as in the following example.
 * <code>
 * <pre>
 *   extern VtRecipientInfoParam VtRecipientInfoParam822EmailValidity;
 * </pre>
 * </code>
 * <p>Associated with each RecipientInfoParam is specific info. The
 * documentation for each RecipientInfoParam will describe the
 * associated info it needs. That data could be another object, it
 * could be data in a particular struct, it might be a NULL pointer.
 * Check the documentation for each RecipientInfoParam for a
 * description of the data and its required format.
 * <p>To use this function decide which RecipientInfoParam you want to
 * use, then determine what information that RecipientInfoParam needs
 * and in which format it is presented. Collect the data in the
 * appropriate format then call this function passing in the desired
 * RecipientInfoParam and the required info. The associated info must
 * be cast to Pointer.
 * <p>For example, if the required info is a VtEmailInfo struct,
 * declare a variable to be of type VtEmailInfo, fill in the fields,
 * then pass the address of that struct cast to Pointer.
 *
 * @param recipInfoObj The object to set.
 * @param recipientInfoParam What the object is being set to.
 * @param associatedInfo The info needed by the RecipientInfoParam.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtSetRecipientInfoParam (
   VtRecipientInfoObject recipInfoObj,
   VtRecipientInfoParam recipientInfoParam,
   Pointer associatedInfo
);

/** Get the specified type of information out of a RecipientInfo object.
 * <p>This function will go to the address given by getInfo and
 * deposit a pointer. That pointer is the address of the data. The
 * memory to hold the information belongs to the object, do not free it.
 * <p>The VtRecipientInfoParam will specify what kind of information
 * will be returned, the getInfo is where the function will deposit the
 * info.
 * <p>The include file vibe.h defines the supported RecipientInfoParams.
 * Look through the include file to see which RecipientInfoParam to use
 * for your application.
 * <p>See also the documentation for VtSetRecipientInfoParam.
 * <p>To use this function decide which RecipientInfoParam you want to
 * use, then determine what information that RecipientInfoParam will
 * return and in which format it is presented. Declare a variable to be
 * a pointer to the appropriate type, then call this function passing
 * in the desired RecipientInfoParam and the address of the variable.
 * Cast the address of the pointer to the appropriate type: (Pointer *).
 * <p>If the associated info is already a pointer type (such as an
 * object), declare a variable to be of the appropriate type and pass
 * the address of that variable cast to (Pointer *).
 * <p>For example, if the returned info is a VtIdentityObject, declare
 * a variable to be of type VtIdentityObject, pass the address of that
 * variable (&varName) cast to (Pointer *).
 * <p>Example:
 * <code>
 * <pre>
 *    VtIdentityObject getIdObj;
 *
 *    do {
 *          . . .
 *      status = VtGetRecipientInfoParam (
 *        recipInfoObj, VtRecipientInfoParamIdentity, (Pointer *)&getIdObj);
 *      if (status != 0)
 *        break;
 *          . . .
 *    } while (0);
 * </pre>
 * </code>
 *
 * @param recipInfoObj The object to query.
 * @param recipientInfoParam Defines what info to get.
 * @param getInfo The address of a pointer. The function will go to
 * that address and deposit a pointer to the info requested.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtGetRecipientInfoParam (
   VtRecipientInfoObject recipInfoObj,
   VtRecipientInfoParam recipientInfoParam,
   Pointer *getInfo
);

/* These are the RecipientInfoParams supported by the toolkit. Each
 * RecipientInfoParam is used in conjunction with special info for the
 * function. If there is no special info, the accompaniment is a NULL
 * pointer.
 */

/** SetParam and GetParam.
 * <p>This VtRecipientInfoParam is used to set a RecipientInfo object
 * with an email address as the identity. For this schema, the identity
 * is made up of more than just the email address, it also includes a
 * time. The time is computed as a "notBefore" time based on the
 * app-supplied base time and vailidity period. The email address must
 * be UTF-8 string (an ASCII string is already a UTF-8 string).
 * <p>Or it is used to get the email and validity info out of the
 * identity object.
 * <p>The associated info is a VtEmailValidityInfo struct.
 * <p>When setting, build the VtEmailValidityInfo struct with the email
 * address, an identity time, a base time, and a validity period (in
 * seconds). Then pass the address of that struct as the associatedInfo.
 * <p>When getting, pass in the address of a pointer as the getInfo,
 * the Get function will deposit a VtEmailValidityInfo pointer at the
 * address.
 * <p>The time in an encoded identity (the actual public key of IBE) is
 * actually a "notBefore" time. For example, an identity based on the
 * email address name@sample.com can be name@sample.com for the week of
 * Oct. 20, 2003. Then name@sample.com for the week of April 11, 2005
 * is a different identity. There will be different IBE private keys
 * associated with each of those two identities, even though they use
 * the same email address.
 * <p>Generally, an app will use a validity period, so that identities
 * don't change every second. For instance, name@sample.com at 3:00 GMT
 * Oct. 22, 2003, and name@sample.com at 17:30 GMT Oct. 25, 2003, can
 * use the same "notBefore" time (e.g. 0:00 GMT, Oct. 20, 2003). In
 * other words, the time associated with an identity is actually a
 * week, rather than a particular second. That allows a user to
 * download one private key every week, rather than one private key
 * every email message.
 * <p>Hence, the time in the encoded identity can be different from the
 * time given in the Param's associated info.
 * <p>With this RecipientInfoParam, the validity period is
 * application-defined. The associated info contains (in addition to
 * the expected fields of email address and time) a field for a base
 * time and another field for validity period. The toolkit will
 * determine a notBefore time based on those values.
 * <p>The validity period is given in number of seconds. For example, a
 * validity period of one hour is 3600 seconds.
 * <p>When you Get this Param out of an object before the identity is
 * encoded, the email time will be the same time originally input. When
 * you Get this param out of an object after the identity is encoded,
 * the email time will be the notBefore time. That is, the input time
 * is converted by the encoder into a notBefore time. If there is a
 * notBefore time available, that is the time that will be returned.
 */
extern VtRecipientInfoParam VtRecipientInfoParam822EmailValidity;

/** SetParam and GetParam.
 * <p>When setting, this Param is used to build RecipientInfo, it is an
 * alternate to VtRecipientInfoParam822EmailValidity as a way to
 * indicate the recipient. If you already have an identity object built
 * for a recipient, use this param. The toolkit will clone the identity
 * object passed in.
 * <p>After calling VtDecodeRecipientInfo, call VtGetRecipientInfoParam
 * to see the decoded identity. The call VtDecodeRecipientInfoIdentity
 * exists, it builds an identity object from an encoded RecipientInfo,
 * but it does not decode the entire RecipientInfo. Your app might not
 * need to use this Param if you do call VtDecodeRecipientInfoIdentity,
 * but if your app doesn't need to call VtDecodeRecipientInfoIdentity,
 * you can save some effort by using this Param (the toolkit won't have
 * to decode the RecipientInfo twice).
 * <p>If getting, the toolkit will return a reference to an identity
 * object stored insde the RecipientInfo object, this object is owned
 * by the toolkit, you should not alter or destroy the object returned.
 * <p>The info associated with VtRecipientInfoParamIdObject is a
 * VtIdentityObject.
 */
extern VtRecipientInfoParam VtRecipientInfoParamIdObject;

/** SetParam and GetParam.
 * <p>This param is used to load the symmetric key into the object. Or
 * it is used to get the symmetric key out when reading RecipientInfo.
 * <p>RecipientInfo consists of the IssuerSerial (this idescribes the
 * public key used to encrypt the symmetric key), the key encryption
 * algorithm identifier, and the encrypted symmetric key. This Param
 * loads that symmetric key that will be encrypted (using the identity
 * loaded using VtRecipientInfoParam822EmailValidity).
 * <p>The data associated with VtRecipientInfoParamSymmetricKey is a
 * pointer to a VtItem giving the data and length of the key.
 */
extern VtRecipientInfoParam VtRecipientInfoParamSymmetricKey;

/** SetParam only. (Note: if you have a RecipientInfoObject and you
 * want to get the IBE algorithm out, use VtRecipientInfoParamIBEAlgId).
 * <p>This param is used to indicate which IBE algorithm the object
 * should use when encrypting the symmetric key. Currently there are
 * two possible algorithms BF-IBE and BB-IBE.
 * <p>The info associated with VtRecipientInfoParamIBEAlgorithm is the
 * VtDerCoder for the chosen algorithm. An algorithm without a DerCoder
 * is not allowed.
 * <p>For example, if you want to encrypt the symmetric key using BF
 * IBE, then call SetRecipientInfoParam with this param and the
 * associated info of VtDerCoderBFType1IBE.
 */
extern VtRecipientInfoParam VtRecipientInfoParamIBEAlgorithm;

/** GetParam only. (Note: if you have a RecipientInfoObject and you
 * want to set the IBE algorithm out, use
 * VtRecipientInfoParamIBEAlgorithm).
 * <p>This param is used to report which IBE algorithm the object used
 * (or will use) when encrypting the symmetric key. Currently there are
 * two possible algorithms BF-IBE and BB-IBE.
 * <p>The Param returns the info as an algorithm identifier.
 * <p>The info associated with VtRecipientInfoParamIBEAlgorithm is a
 * pointer to a VtItem given the data and length of the algId.
 */
extern VtRecipientInfoParam VtRecipientInfoParamIBEAlgId;

/** Build the RecipientInfo encoding. This "converts" the info in the
 * RecipientInfo object into a byte array.
 * <p>Note that there is a RecipientInfoBlock and the encoded
 * RecipientInfo. These are two separate things. This function builds
 * the DER encoding of the ASN.1 RecipientInfo definition in PKCS #7.
 * <p>To encode the RecipientInfo, the object must be set with an
 * identity (SetParams with VtRecipientInfoParam822EmailValidity or
 * VtRecipientInfoParamIdentity, for example), a symmetric key
 * (SetParams with VtRecipientInfoParamSymmetricKey, and
 * VtRecipientInfoParamSymmetricAlgFlag), and an IBE algorithm
 * (SetParams with VtRecipientInfoParamIBEAlgorithm).
 * <p>The function will use IBE to encrypt the session key.
 * <p>Part of the RecipientInfo is the encoded identity, so this
 * function will need to know which version of the IBCS #2 standard to
 * use when encoding. Currently available are
 * <code>
 * <pre>
 *    VT_ENCODE_IBCS_2_V_1
 *    VT_ENCODE_IBCS_2_V_2
 *    VT_ENCODE_IBCS_2_V_DISTRICT
 * </pre>
 * </code>
 * <p>The version numbers (V_1 and V_2) specify using the encodings
 * outlined in IBCS #2. The district version (V_DISTRICT) specifies
 * using the encoding standard version given by the district. That is,
 * a district may specify a version number. If so, when using that
 * district to generate a public key, the sender (encryptor) should use
 * that standard. If the caller passes in V_DISTRICT and the district
 * does not specify a version, the function will use version 1 (V_1).
 * <p>It is permitted to pass in no storage ctx (NULL).
 * <p>This routine will go to the address given by encodingLen and
 * deposit the length of the output (the number of bytes placed into
 * the encoding buffer). If the buffer is not big enough, this function
 * will return the "BUFFER_TOO_SMALL" error and set the unsigned int at
 * encodingLen to the needed size.
 * <p>The function will generate the encoding and store it internally,
 * then if there is enough space in the output buffer, it will copy the
 * data there. If the buffer is not big enough, it will return
 * BUFFER_TOO_SMALL. So if you want to encode the RecipientInfo, but
 * don't actually need the data itself (for example, maybe what you
 * really want is the RecipientInfoBlock, which requires the encoded
 * RecipientInfo first), Call this Encode function with a NULL output
 * buffer and the object will contain the encoding.
 *
 * @param recipInfoObj The object containing the info to encode.
 * @param version The version of IBCS #2 to follow.
 * @param policyCtx The policy context to use, if needed.
 * @param storageCtx The storage ctx containing the storage providers
 * which the function will search and into which downloaded entries
 * will be stored.
 * @param transportCtx The transport context to use when contacting
 * districts, if necessary.
 * @param random A source of random bytes if necessary (encrytping
 * using IBE generally requires random bytes).
 * @param encoding The buffer into which the routine will place the
 * result.
 * @param bufferSize The size, in bytes, of the encoding buffer.
 * @param encodingLen The address where the routine will deposit the
 * resulting length, in bytes, of the encoding.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtEncodeRecipientInfo (
   VtRecipientInfoObject recipInfoObj,
   unsigned int version,
   VtPolicyCtx policyCtx,
   VtStorageCtx storageCtx,
   VtTransportCtx transportCtx,
   VtRandomObject random,
   unsigned char *encoding,
   unsigned int bufferSize,
   unsigned int *encodingLen
);

/** Decode a byte array that is an encoded RecipientInfo. The caller
 * passes in a created but empty (not-yet-set) RecipientInfo object.
 * This function will set the object with the info in the encoding.
 * <p>The RecipientInfo contains the encoded identity (inside the
 * IssuerSerialNumber, it is the public key used to encrypt the
 * symmetric key), the algId of the key encrypting algorithm (the IBE
 * algId, either BF or BB), and the encrypted key.
 * <p>This function will decode separate the components, storing them
 * inside the object. It will not decrypt the encrypted key (for that
 * it would need the private key, but an app does not know if it will
 * have access to a private key until it has decoded the RecipientInfo
 * and examined the identity).
 * <p>In order to decode the RecipientInfo, the function will have to
 * decode the encoded identity. Hence, the caller must supply an array
 * of decoders (VtIdentitySchemaDecode) the schemas the app is willing
 * to support. The function will find the decoder that will decode the
 * identity, and return the index of the chosen decoder at the address
 * given by arrayIndex. The caller can pass a NULL pointer for the
 * arrayIndex argument and the routine will simply not return that
 * information.
 * <p>In order to decode, the function will read the
 * keyEncryptionAlgorithm, an algId specifying the algorithm used to
 * encrypt the symmetric key. Hence, the caller must supply a
 * VtDerCoder array, listing the algorithms the app is willing to
 * support. The algorithm will be an IBE algorithm, but it can be BF or
 * BB, therefore, the need for an array. The routine will go to the
 * address given by algorithm and deposit a flag indicating what
 * algorithm is used. The value of algorithm will be one of the flags
 * listed in vibe.h. They are the #defines that begin with VT_ALG_ID_
 * (for example, VT_ALG_ID_BF_TYPE1_IBE_ENCRYPT or
 * VT_ALG_ID_BB_TYPE1_IBE_ENCRYPT).
 * <p>After decoding, call VtGetRecipientInfoParam to get the specific
 * info out of the object (such as VtRecipientInfoParamIdObject).
 *
 * @param encoding The buffer containing the encoded RecipientInfo.
 * @param encodingLen The length, in bytes, of the encoding.
 * @param decoders An array of VtIdentitySchemaDecodes.
 * @param decoderCount The number of decoders in the array.
 * @param arrayIndex The address where the function will deposit the
 * index into the array of the schema the identity is.
 * @param derCoders Represents the list of IBE algorithms the app is
 * willing to support.
 * @param derCoderCount The number of derCoders in the array.
 * @param algorithm The address where the function will deposit a flag
 * indicating what algorithm or key is represented by the BER encoding.
 * @param recipInfoObj The created but empty RecipientInfoObject which
 * the function will load with the ID and key info from the
 * RecipientInfo.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtDecodeRecipientInfo (
   unsigned char *encoding,
   unsigned int encodingLen,
   VtIdentitySchemaDecode **decoders,
   unsigned int decoderCount,
   unsigned int *arrayIndex,
   VtDerCoder **derCoders,
   unsigned int derCoderCount,
   unsigned int *algorithm,
   VtRecipientInfoObject recipInfoObj
);

/** After decoding a RecipientInfo, use this function to decrypt the
 * symmetric key inside. This will decrypt the key data and store it
 * internally. To see the key data, call VtGetRecipientInfoParam with
 * VtRecipientInfoParamSymmetricKey.
 * <p>An app will likely decode each RecipientInfo into a RecipientInfo
 * object, get the identity object out of the RecipientInfo object, and
 * examine it to determine if this is an identity for which it will be
 * able to obtain a private key. Once the app finds the RecipientInfo
 * it wants to use to decrypt the symmetric key, it will call this
 * function.
 * <p>NOTE!!! This can be a departure from normal toolkit rules.
 * Normally we say do not alter objects returned from a Get call,
 * however, in this case, you can Get a RecipientInfo object from a
 * RecipientInfoList and call the Decrypt function on the object
 * returned.
 * <p>In order to decrypt the symmetric key, the function will need the
 * IBE private key, so the caller must supply a policy, transport, and
 * optionally storage context (or they must be loaded in the libCtx).
 *
 * @param recipInfoObj The object containing the symmetric key to
 * decrypt, along with other info needed to obtain the private key.
 * @param policyCtx The policy context to use (if not loaded in the
 * libCtx).
 * @param storageCtx The storage ctx containing the storage providers
 * which the function will search and into which downloaded entries
 * will be stored (if storage is desired and if no storageCtx is loaded
 * in the libCtx).
 * @param transportCtx The transport context to use when contacting
 * districts (if not loaded in the libCtx).
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtDecryptRecipientInfo (
   VtRecipientInfoObject recipInfoObj,
   VtPolicyCtx policyCtx,
   VtStorageCtx storageCtx,
   VtTransportCtx transportCtx
);

/*@}*/

/*=========================================================*/
/*                                                         */
/* RecipientInfoList                                       */
/*                                                         */
/*=========================================================*/

/**
 * \defgroup RecipInfoListGroup RecipientInfoList Object
 */

/*@{*/

/** The RecipientInfoList object.
 * <p>Note that the object is a pointer type.
 */
typedef struct VtRecipientInfoListDef *VtRecipientInfoList;

/** The function VtCreateRecipientInfoList builds a RecipientInfo list
 * using a VtRecipientInfoListImpl. This typedef defines what a
 * VtRecipientInfoListImpl is. Although a VtRecipientInfoListImpl is a
 * function pointer, an application should never call one directly,
 * only pass it as an argument to VtCreateRecipientInfoList.
 */
typedef int VT_CALLING_CONV (VtRecipientInfoListImpl) (
   VtRecipientInfoList *, Pointer, unsigned int);

/** The functions VtSetRecipientInfoListParam and
 * VtGetRecipientInfoListParam add or get information to or from a
 * RecipientInfoList. The information to add or get is defined by a
 * VtRecipientInfoListParam. This typedef defines what a
 * VtRecipientInfoListParam is. Although a VtRecipientInfoListParam is
 * a function pointer, an application should never call one directly,
 * only pass it as an argument to VtSetRecipientInfoListParam or
 * VtGetRecipientInfoListParam.
 */
typedef int VT_CALLING_CONV (VtRecipientInfoListParam) (
   VtRecipientInfoList, Pointer, unsigned int);

/** Create a new RecipientInfoList. This allocates space for an "empty"
 * object, then loads the given RecipientInfoListImpl to make it an
 * "active" object.
 * <p>The VtRecipientInfoListImpl defines some of the RecipientInfo list
 * operations. The include file vibe.h defines the supported
 * RecipientInfoListImpls. Look through the include file to see which
 * RecipientInfoListImpl to use for your application. All supported
 * RecipientInfoListImpls will be defined as in the following example.
 * <code>
 * <pre>
 *   extern VtRecipientInfoListImpl VtRecipientInfoListImplMpCtx;
 * </pre>
 * </code>
 * <p>Associated with each RecipientInfoListImpl is specific info. The
 * documentation for each RecipientInfoListImpl will describe the
 * associated info it needs. That data could be another object, it
 * could be data in a particular struct, it might be a NULL pointer.
 * Check the documentation for each RecipientInfoListImpl for a
 * description of the data and its required format.
 * <p>To use this function decide which RecipientInfoListImpl you want
 * to use, then determine what information that RecipientInfoListImpl
 * needs and in which format it is presented. Collect the data in the
 * appropriate format then call this function passing in the desired
 * RecipientInfoListImpl and the required info. The associated info
 * must be cast to Pointer.
 * <p>For example, if the required info is a VtItem struct, declare a
 * variable to be of type VtItem, fill in the fields, then pass the
 * address of that struct cast to Pointer.
 * <p>The input recipInfoList is a pointer to an object. It should
 * point to a NULL VtRecipientInfoList. This function will go to the
 * address given and deposit a created object.
 * <p>If you call Create, you must call Destroy after you are done with
 * the list but before the program exits.
 * <p>Example:
 * <code>
 * <pre>
 *    int status;
 *    VtRecipientInfoList recipInfoList = (VtRecipientInfoList)0;
 *
 *    do {
 *          . . .
 *      status = VtCreateRecipientInfoList (
 *        libCtx, VtRecipientInfoListImplMpCtx, (Pointer)0, &recipInfoList);
 *      if (status != 0)
 *        break;
 *          . . .
 *    } while (0);
 *
 *    VtDestroyRecipientInfoList (&recipInfoList);
 * </pre>
 * </code>
 *
 * @param libCtx The library context.
 * @param recipInfoListImpl The implementation the object will use.
 * @param associatedInfo The info needed by the RecipientInfoListImpl.
 * @param recipInfoList A pointer to where the routine will deposit the
 * created object.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtCreateRecipientInfoList (
   VtLibCtx libCtx,
   VtRecipientInfoListImpl recipInfoListImpl,
   Pointer associatedInfo,
   VtRecipientInfoList *recipInfoList
);

/* These are the VtRecipientInfoListImpls supported by the toolkit.
 * Each RecipientInfoListImpl is used in conjunction with special info
 * for the function. If there is no special info, the accompaniment is
 * a NULL pointer.
 */

/** This VtRecipientInfoListImpl is used to load up the MpIntCtx the
 * object will use when performing IBE operations (encrypting or
 * decrypting symmetric keys). For the toolkit to perform IBE
 * operations, it must have an MpIntCtx.
 * <p>If the associatedInfo passed in with this Impl is NULL, then the
 * Impl will try to find an mpCtx in the libCtx.
 * <p>The data associated with VtRecipientInfoListImplMpCtx is a
 * VtMpIntCtx.
 */
extern VtRecipientInfoListImpl VtRecipientInfoListImplMpCtx;

/** Destroy a RecipientInfo List. This frees up any memory allocated
 * during the object's creation and use.
 * <p>If you call Create, you must call Destroy after you are done with
 * the list but before the program exits.
 * <p>Example:
 * <code>
 * <pre>
 *    int status;
 *    VtRecipientInfoList recipInfoList = (VtRecipientInfoList)0;
 *
 *    do {
 *          . . .
 *      status = VtCreateRecipientInfoList (
 *        libCtx, VtRecipientInfoListImplMpCtx, (Pointer)0, &recipInfoList);
 *      if (status != 0)
 *        break;
 *          . . .
 *    } while (0);
 *
 *    VtDestroyRecipientInfoList (&recipInfoList);
 * </pre>
 * </code>
 *
 * @param recipInfoList A pointer to where the routine will find the
 * object to destroy.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtDestroyRecipientInfoList (
   VtRecipientInfoList *recipInfoList
);

/** Set the RecipientInfoList with the information given.
 * <p>The VtRecipientInfoListParam defines what information the object
 * will be set with.
 * <p>The include file vibe.h defines the supported
 * RecipientInfoListParams. Look through the include file to see which
 * RecipientInfoListParam to use for your application. All supported
 * RecipientInfoListParams will be defined as in the following example.
 * <code>
 * <pre>
 *   extern VtRecipientInfoListParam  VtRecipientInfoListParamSecretValue;
 * </pre>
 * </code>
 * <p>Associated with each RecipientInfoListParam is specific info. The
 * documentation for each RecipientInfoListParam will describe the
 * associated info it needs. That data could be another object, it
 * could be data in a particular struct, it might be a NULL pointer.
 * Check the documentation for each RecipientInfoListParam for a
 * description of the data and its required format.
 * <p>To use this function decide which RecipientInfoListParam you want
 * to use, then determine what information that RecipientInfoListParam
 * needs and in which format it is presented. Collect the data in the
 * appropriate format then call this function passing in the desired
 * RecipientInfoListParam and the required info. The associated info
 * must be cast to Pointer.
 * <p>For example, if the required info is a VtItem struct, declare a
 * variable to be of type VtItem, fill in the fields, then pass the
 * address of that struct cast to Pointer.
 *
 * @param recipInfoList The list to set.
 * @param recipientInfoListParam What the object is being set to.
 * @param associatedInfo The info needed by the RecipientInfoListParam.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtSetRecipientInfoListParam (
   VtRecipientInfoList recipInfoList,
   VtRecipientInfoListParam recipientInfoListParam,
   Pointer associatedInfo
);

/** Get the specified type of information out of a RecipientInfoList.
 * <p>This function will go to the address given by getInfo and
 * deposit a pointer. That pointer is the address of the data. The
 * memory to hold the information belongs to the object, do not free it.
 * <p>The VtRecipientInfoListParam will specify what kind of
 * information will be returned, the getInfo is where the function will
 * deposit the info.
 * <p>The include file vibe.h defines the supported
 * RecipientInfoListParams. Look through the include file to see which
 * RecipientInfoListParam to use for your application.
 * <p>See also the documentation for VtSetRecipientInfoListParam.
 * <p>To use this function decide which RecipientInfoListParam you want
 * to use, then determine what information that RecipientInfoListParam
 * will return and in which format it is presented. Declare a variable
 * to be a pointer to the appropriate type, then call this function
 * passing in the desired RecipientInfoListParam and the address of the
 * variable. Cast the address of the pointer to the appropriate type:
 * (Pointer *).
 * <p>If the associated info is already a pointer type (such as an
 * object), declare a variable to be of the appropriate type and pass
 * the address of that variable cast to (Pointer *).
 *
 * @param recipInfoList The list to query.
 * @param recipientInfoListParam Defines what info to get.
 * @param getInfo The address of a pointer. The function will go to
 * that address and deposit a pointer to the info requested.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtGetRecipientInfoListParam (
   VtRecipientInfoList recipInfoList,
   VtRecipientInfoListParam recipientInfoListParam,
   Pointer *getInfo
);

/* These are the RecipientInfoListParams supported by the toolkit. Each
 * RecipientInfoListParam is used in conjunction with special info for
 * the function. If there is no special info, the accompaniment is a
 * NULL pointer.
 */

/** SetParam and GetParam.
 * <p>This param is used to load a secret value into the object. This
 * secret value will be used to derive a Key Encrypting Key (KEK) that
 * will be used to encrypt the symmetric key.
 * <p>After building the RecipientInfoList, an app has the option of
 * getting the RecipientInfoBlock out of the list (see
 * VtRecipientInfoListParamRecipInfoBlock). This is a single block of
 * data containing the RecipientInfos, the symmetric key, and the
 * contentEncryptionAlgorithm. Some apps will want the symmetric key
 * protected so it is not returned in the clear.
 * <p>If you do not set a RecipientInfoList with this Param, and get
 * the RecipientInfoBlock out, the symmetric key will be in the clear.
 * <p>If an app has a RecipientInfoBlock, and wants to get the
 * symmetric key out, and the Block was originally created using this
 * param, then the key inside the Block is protected, so it will be
 * necessary to set the object with this param.
 * <p>The info associated with VtRecipientInfoListParamSecretValue is a
 * pointer to a VtItem giving the data and length of the secret value.
 */
extern VtRecipientInfoListParam VtRecipientInfoListParamSecretValue;

/** SetParam only.
 * <p>This param is used to define the symmetric algorithm in the
 * object. Part of the RecipientInfoBlock is the algorithm identifier
 * of the symmetric algorithm. If an app uses the RecipientInfoBlock,
 * it will have a symmetric key, but for which algorithm? Is it
 * Triple-DES-CBC or AES-192-CFB? AES-128-ECB or AES-128-CBC?
 * <p>You pass in the algId of the algorithm you want to use, along
 * with a list of DerCoders. The Param will find the DerCoder to match
 * the algId and use it, along with the algId, to build an algorithm
 * object if needed.
 * <p>You can "reset" an object with this Param. For example, if you
 * get the RecipientInfoList out of a Voltage message object after
 * reading (and decrypting), and want to use it to re-encrypt, but want
 * to change the IV, set with this Param.
 * <p>Because this param can reset objects, it is, in a sense,
 * dangerous when reading and re-encrypting. When you read a message,
 * it will have the original algId. If you Set with this param, you
 * will lose old information, or you can create incompatibilities
 * (original algorithm Triple-DES, new algorithm AES-128, the key
 * doesn't match the new algorithm). Therefore, the best way to use
 * this if re-encrypting is to Get the orignial algId, copy it into a
 * buffer, change the IV and call Set.
 * <p>Of course, if you never call Set with this Param, you will not
 * lose info or create incompatibilities, but you will use the same IV.
 * <p>The info associated with VtRecipientInfoListParamSymmetricAlgorithm
 * is a pointer to a VtSetAlgIdInfo struct.
 */
extern VtRecipientInfoListParam VtRecipientInfoListParamSymmetricAlgorithm;

/** GetParam only.
 * <p>After reading RecipientInfoBlock, or getting a RecipientInfoList
 * out of a Voltage message object, if you want to see what the
 * symmetric algorithm is, call with this Param.
 * <p>If you want to re-encrypt, you can get the algId out of the List,
 * copy it into your own buffer, change the IV, then call SetParam with
 * VtRecipientInfoListParamSymmetricAlgorithm.
 * <p>The info returned by VtRecipientInfoListParamSymmetricAlgId is a
 * pointer to a VtItem giving the data and length of the algorithm
 * identifier of the symmetric algorithm.
 */
extern VtRecipientInfoListParam VtRecipientInfoListParamSymmetricAlgId;

/** GetParam only.
 * <p>This will generally be used when obtaining a RecipientInfoList
 * from a P7 or other Voltage message object. After reading the
 * message, you can get the RecipientInfoList (see, for example, the
 * documentation for VtPkcs7ParamRecipientInfoList). You can then get
 * the RecipientInfoBlock, or each RecipientInfo. With this Param, you
 * can also get the symmetric key.
 * <p>If none of the RecipientInfo objects inside the List have the
 * symmetric key decrypted, the Get function will return
 * GET_INFO_UNAVAILABLE.
 * <p>Note, this returns the symmetric key in the clear. It does not
 * return the encrypted symmetric key (encrypted using someone's
 * identity) or the protected symmetric key (protected using the
 * RecipientInfoList's secretValue).
 * <p>The data associated with VtRecipientInfoListParamSymmetricKey is
 * a pointer to a VtItem.
 */
extern VtRecipientInfoListParam VtRecipientInfoListParamSymmetricKey;

/** Add the given RecipientInfo object to the RecipientInfo list.
 * <p>Each RecipientInfo object must be set with the same symmetric key
 * and algorithm.
 * <p>If the listIndex arg is not NULL, the function will set the
 * unsigned int at that address to the index inside the RecipientInfo
 * list of the newly added RecipientInfo object. So long as the
 * RecipientInfo list exists, and the RecipientInfo object is not
 * removed from the list, that RecipientInfo object's index will never
 * change.
 * <p>This function will clone the RecipientInfo object. That is, the
 * RecipientInfo objects inside the RecipientInfo list are
 * "independent" of the object passed in by the caller. Changes made to
 * the caller-owned object will not be reflected in the RecipientInfo
 * object inside the RecipientInfo list.
 *
 * @param recipInfoList The list to which the RecipientInfo object is
 * added.
 * @param recipInfoObj The RecipientInfo object containing the
 * RecipientInfo to add.
 * @param listIndex If NULL, the argument is ignored. If not NULL, it
 * is the address where the routine will deposit the index inside the
 * RecipientInfo list object of the RecipientInfo object.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtAddRecipientInfoToList (
   VtRecipientInfoList recipInfoList,
   VtRecipientInfoObject recipInfoObj,
   unsigned int *listIndex
);

/** How many active RecipientInfo objects are in the list?
 * <p>The function will set the unsigned int at the address count with
 * the number of RecipientInfo entries.
 * <p>The maxIndex is the index of the "last" active RecipientInfo in
 * the list. If the count is 0, the maxIndex is meaningless.
 * <p>There are some number of entries in the list, some of them
 * active, some inactive. Each entry (active or inactive) has an index.
 * If there is an entry in the list with an index greater than the
 * returned maxIndex, it will be inactive.
 * <p>The entries are indexed 0 (zero) through maxIndex. Use these
 * indices for VtGetRecipientInfoListEntry.
 *
 * @param recipInfoList The list to query.
 * @param count The address where this function will deposit the count.
 * @param maxIndex The address where this function will deposit the
 * largest number that is an index for an active entry. If the count is
 * 0, the returned maxIndex is meaningless.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtGetRecipientInfoListCount (
   VtRecipientInfoList recipInfoList,
   unsigned int *count,
   unsigned int *maxIndex
);

/** Get the RecipientInfo object at the given listIndex out of the
 * RecipientInfo list.
 * <p>If there is no RecipientInfo object at that index, or the entry
 * is inactive, the function will return VT_ERROR_NO_ID_AT_INDEX. In
 * order to get all the active RecipientInfo objects out of the
 * RecipientInfoList, therefore, call VtGetRecipientInfoListCount to
 * get the maxIndex, then loop from 0 to maxIndex calling
 * VtGetRecipientInfoListEntry. If the return is 0, you have a
 * RecipientInfo object. If the return is VT_ERROR_NO_ID_AT_INDEX, you
 * don't have a RecipientInfoObject, move on. Any other error should be
 * handled.
 * <p>The function will go to the address given by recipInfoObj and
 * deposit an object. The object belongs to the RecipientInfoList, do
 * not destroy the object returned. That is, the object returned is a
 * reference to the object in the list, not a clone. Any changes made
 * to the returned object will be reflected inside the list.
 *
 * @param recipInfoList The list to query.
 * @param listIndex The index of the identity in the list to get.
 * @param recipInfoObj The address where this function will deposit a
 * reference to the RecipientInfo object at the specified index.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtGetRecipientInfoListEntry (
   VtRecipientInfoList recipInfoList,
   unsigned int listIndex,
   VtRecipientInfoObject *recipInfoObj
);

/** Set the status of the RecipientInfoObject entry in the
 * RecipientInfoList to one of the following.
 * <code>
 * <pre>
 *    VT_RECIPIENT_INFO_LIST_ENTRY_ACTIVE
 *    VT_RECIPIENT_INFO_LIST_ENTRY_INACTIVE
 * </pre>
 * </code>
 * <p>If an entry is set to INACTIVE, toolkit functions will treat that
 * RecipientInfo entry as if it does not exist. The internal copy of
 * the RecipientInfo is not changed or destroyed, the toolkit simply
 * ignores it. If the entry is already inactive, this function will do
 * nothing and return 0 (success). If the index is beyond the end of
 * the list (no entry for that index), this function returns an error.
 * It would be possible to logically say that setting a non-existent
 * entry to inactive is no error, but if setting it to inactive were
 * possible, then so would setting it to active.
 * <p>If an entry is set to ACTIVE, the toolkit will now recognize the
 * entry. If the entry is already active, this function will do nothing
 * and return 0 (success). If the index is beyond the end of the list
 * (no entry for that index, either active or inactive), the function
 * will return an error.
 *
 * @param recipInfoList The list with the entry to set.
 * @param listIndex The index of the entry to set.
 * @param newStatus The flag indicating to what status the entry should
 * be set.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtSetEntryStatusInRecipientInfoList (
   VtRecipientInfoList recipInfoList,
   unsigned int listIndex,
   unsigned int newStatus
);

/** For use with VtSetEntryStatusInIdentityList, indicate that an
 * entry should be made active.
 */
#define VT_RECIPIENT_INFO_LIST_ENTRY_ACTIVE    1
/** For use with VtSetEntryStatusInIdentityList, indicate that an
 * entry should be made inactive.
 */
#define VT_RECIPIENT_INFO_LIST_ENTRY_INACTIVE  0

/** Create an encoding out of the RecipientInfoList. Build either the
 * RecipientInfos or the RecipientInfoBlock. The argument encodingFlag
 * indicates which construct is requested. It must be one of the
 * following values.
 * <pre>
 * <code>
 *    VT_ENCODING_RECIPIENT_INFOS
 *    VT_ENCODING_RECIPIENT_INFO_BLOCK
 * </code>
 * </pre>
 * <p>The RecipientInfos is defined in PKCS #7 (EnvelopedData and
 * SignedAndEnvelopedData) and is simply
 * <pre>
 * <code>
 *    RecipientInfos ::= SEQUENCE OF RecipientInfo
 * </code>
 * </pre>
 * <p>If you build RecipientInfo, there's no need to set the
 * RecipientInfoList object with the symmetric algorithm or secret
 * value. An app would likely add RecipientInfoObjects to the List,
 * then encode for RecipientInfos.
 * <p>The RecipientInfoBlock is the DER encoding of the following ASN.1
 * definition.
 * <pre>
 * <code>
 *    RecipientInfoBlock ::= SEQUENCE {
 *      ibcsVersion                   INTEGER,
 *      recipientInfos                RecipientInfos,
 *      contentEncryptionAlgorithm    AlgorithmIdentifier,
 *      keyProtectionAlgorithm        AlgorithmIdentifier OPTIONAL,
 *      symmetricKey                  OCTET STRING }
 * </code>
 * </pre>
 * <p>The contentEncryptionAlgorithm is also from PKCS #7, it is the
 * algorithm identitfier of the symmetric algorithm that encrypts the
 * bulk data, the content of EnvelopedData. See also the documentation
 * for VtRecipientInfoListParamSymmetricAlgorithm.
 * <p>The keyProtectionAlgorithm is built by the toolkit. If you set
 * the RecipientInfoList with a secret value (see
 * VtRecipientInfoListParamSecretValue), the function will encrypt
 * (protect) the symmetric key using a key derived from the secret
 * value. The algId will define the algorithm used and its parameters.
 * If you do not set the List with a secretValue, the toolkit will
 * leave this element blank and will return the symmetric key in
 * plaintext.
 * <p>The symmetric key will be either in plaintext or encrypted based
 * on whether a secretValue was loaded, but it will always be an OCTET
 * STRING.
 * <p>In order to build RecipientInfos, the routine will have to encode
 * each RecipientInfo. If they are not encoded already, the function
 * will need the version and random object, along with the policy,
 * storage, and transport contexts. See the documentation for
 * VtEncodeRecipientInfo for more info.
 *
 * @param recipInfoList The object containing the info to encode.
 * @param encodingType A flag indicating whether the function should
 * produce RecipientInfos or RecipientInfoBlock.
 * @param version The version of IBCS #2 to follow.
 * @param policyCtx The policy context to use, if needed.
 * @param storageCtx The storage ctx containing the storage providers
 * which the function will search and into which downloaded entries
 * will be stored.
 * @param transportCtx The transport context to use when contacting
 * districts, if necessary.
 * @param random A source of random bytes if necessary (encrytping
 * using IBE generally requires random bytes).
 * @param encoding The buffer into which the routine will place the
 * result.
 * @param bufferSize The size, in bytes, of the encoding buffer.
 * @param encodingLen The address where the routine will deposit the
 * resulting length, in bytes, of the encoding.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtEncodeRecipientInfoList (
   VtRecipientInfoList recipInfoList,
   unsigned int encodingType,
   unsigned int version,
   VtPolicyCtx policyCtx,
   VtStorageCtx storageCtx,
   VtTransportCtx transportCtx,
   VtRandomObject random,
   unsigned char *encoding,
   unsigned int bufferSize,
   unsigned int *encodingLen
);

/** Use this flag when calling VtEncodeRecipientInfoList to indicate
 * that the function should produce the encoding of RecipientInfos.
 */
#define VT_ENCODING_RECIPIENT_INFOS        1
/** Use this flag when calling VtEncodeRecipientInfoList to indicate
 * that the function should produce the encoding of RecipientInfoBlock.
 */
#define VT_ENCODING_RECIPIENT_INFO_BLOCK   2

/** Decode a byte array that is an encoded RecipientInfos or
 * RecipientInfoBlock. The caller passes in a created but empty
 * (not-yet-set) RecipientInfoList. This function will set the object
 * with the info in the encoding.
 * <p>If the encoding is RecipientInfos, this function is equivalent to
 * calling VtAddRecipientInfoToList for each of the recipients.
 * <p>In order to decode, the function will need to decode each
 * individual RecipientInfo. To do that, the function will need the
 * decoders and derCoders (see the documentation for
 * VtDecodeRecipientInfo).
 * <p>Note that there are no args for arrayIndex with the decoders, or
 * algorithm with the derCoders. Because there can be more than one
 * RecipientInfo, the arrayIndex and algorithm could apply to one
 * entry, but not necessarily all, so such a return might not give an
 * accurate description.
 * <p>NOTE!!! If the symmetric key in the RecipientInfoBlock is
 * protected (encrypted, see documentation for
 * VtEncodeRecipientInfoList), you must Set the RecipientInfoList with
 * the secretValue before calling this function. See the documentation
 * for VtRecipientInfoListParamSecretValue. If you set a secretValue
 * and the symmetric key was not encrypted, the function will ignore
 * the secret value.
 * <p>Once you have decoded the encoding, you can get data out of the
 * RecipientInfoList, including each individual RecipientInfo.
 *
 * @param encoding The buffer containing the encoded RecipientInfos or
 * RecipientInfoBlock.
 * @param encodingLen The length, in bytes, of the encoding.
 * @param decoders An array of VtIdentitySchemaDecodes.
 * @param decoderCount The number of decoders in the array.
 * @param derCoders Represents the list of IBE algorithms the app is
 * willing to support.
 * @param derCoderCount The number of derCoders in the array.
 * @param recipInfoList The created but empty RecipientInfoList which
 * the function will load with the info from the RecipientInfoBlock.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtDecodeRecipientInfoList (
   unsigned char *encoding,
   unsigned int encodingLen,
   VtIdentitySchemaDecode **decoders,
   unsigned int decoderCount,
   VtDerCoder **derCoders,
   unsigned int derCoderCount,
   VtRecipientInfoList recipInfoList
);

/*@}*/

/*=========================================================*/
/*                                                         */
/* Secure Mail Object                                      */
/*                                                         */
/*=========================================================*/

/**
 * \defgroup SecureMailGroup SecureMail Object
 */

/*@{*/

/** The SecureMail object.
 * <p>Note that the object is a pointer type.
 */
typedef struct VtSecureMailObjectDef *VtSecureMailObject;

/** The function VtCreateSecureMailObject builds an object that builds
 * or reads SecureMail messages using a VtSecureMailImpl. This typedef
 * defines what a VtSecureMailImpl is. Although a VtSecureMailImpl is a
 * function pointer, an application should never call one directly,
 * only pass it as an argument to VtCreateSecureMailObject.
 */
typedef int VT_CALLING_CONV (VtSecureMailImpl) (
   VtSecureMailObject *, Pointer, unsigned int);

/** The functions VtSetSecureMailParam and VtGetSecureMailParam add or
 * get information to or from a SecureMail object. The information to
 * add or get is defined by a VtSecureMailParam. This typedef defines
 * what a VtSecureMailParam is. Although a VtSecureMailParam is a
 * function pointer, an application should never call one directly,
 * only pass it as an argument to VtSetSecureMailParam or
 * VtGetSecureMailParam.
 */
typedef int VT_CALLING_CONV (VtSecureMailParam) (
   VtSecureMailObject, Pointer, unsigned int);

/** Create a new SecureMail object. This allocates space for an "empty"
 * object, then loads the given SecureMailImpl to make it an "active"
 * object.
 * <p>The VtSecureMailImpl defines what sort of message the object will
 * be able to process (generally reading and writing will be different
 * Impls). The include file vibe.h defines the supported
 * SecureMailImpls. Look through the include file to see which
 * SecureMailImpl to use for your application. All supported
 * SecureMailImpls will be defined as in the following example.
 * <code>
 * <pre>
 *   extern VtSecureMailImpl VtSecureMailImplWrite;
 * </pre>
 * </code>
 * <p>Associated with each SecureMailImpl is specific info. The
 * documentation for each SecureMailImpl will describe the associated
 * info it needs. That data could be another object, it could be data
 * in a particular struct, it might be a NULL pointer. Check the
 * documentation for each SecureMailImpl for a description of the data
 * and its required format.
 * <p>To use this function decide which SecureMailImpl you want to use,
 * then determine what information that SecureMailImpl needs and in which
 * format it is presented. Collect the data in the appropriate format
 * then call this function passing in the desired SecureMailImpl and the
 * required info. The associated info must be cast to Pointer.
 * <p>For example, if the required info is a VtItem struct, declare a
 * variable to be of type VtItem, fill in the fields, then pass the
 * address of that struct cast to Pointer.
 * <p>The input secureMailObj is a pointer to an object. It should
 * point to a NULL VtSecureMailObject. This function will go to the
 * address given and deposit a created object.
 * <p>If you call Create, you must call Destroy after you are done with
 * the object but before the program exits.
 * <p>Example:
 * <code>
 * <pre>
 *    int status;
 *    VtSecureMailObject secureMailObj = (VtSecureMailObject)0;
 *
 *    do {
 *          . . .
 *      status = VtCreateSecureMailObject (
 *        libCtx, VtSecureMailImplWrite, (Pointer)0, &secureMailObj);
 *      if (status != 0)
 *        break;
 *          . . .
 *    } while (0);
 *
 *    VtDestroySecureMailObject (&secureMailObj);
 * </pre>
 * </code>
 * @param libCtx The library context.
 * @param secureMailImpl The implementation the object will use.
 * @param associatedInfo The info needed by the SecureMailImpl.
 * @param secureMailObj A pointer to where the routine will deposit the
 * created object.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtCreateSecureMailObject (
   VtLibCtx libCtx,
   VtSecureMailImpl secureMailImpl,
   Pointer associatedInfo,
   VtSecureMailObject *secureMailObj
);

/* These are the VtSecureMailImpls supported by the toolkit. Each
 * VtSecureMailImpl is used in conjunction with special info for the
 * function. If there is no special info, the accompaniment is a NULL
 * pointer.
 */

/** Use this Impl to build a SecureMail object that will be able to
 * create SecureMail messages.
 * <p>The data associated with VtSecureMailImplWrite is a NULL pointer:
 * (Pointer)0.
 */
extern VtSecureMailImpl VtSecureMailImplWrite;

/** Use this Impl to build a SecureMail object that will be able to
 * create SecureMail V2 messages.
 * <p>The data associated with VtSecureMailImplWrite is a NULL pointer:
 * (Pointer)0.
 */
extern VtSecureMailImpl VtSecureMail2ImplWrite;

/** Use this Impl to write a SecureMail2 object starting from a
 * ZDM object. The ZDM object must be a ZDMv2 object and all of the
 * input data must have already been set (i.e. the current entry
 * must have been set to done) before using it to write the
 * secure mail output.
 * <p>The data associated with VtSecureMailImplWrite is a pointer to
 * a ZDM object.
 */
extern VtSecureMailImpl VtSecureMail2ImplWriteFromZDM;

/** Use this Impl to build a SecureMail object that will be able to
 * read SecureMail messages.
 * <p>The data associated with VtSecureMailImplRead is a pointer to a
 * VtReadSecureMailInfo struct containing all the algorithms
 * (asymmetric and symmetric algorithms represented as DERCoders) and
 * identity schemas (represented as SchemaDecodes) the app is willing
 * to support. The mpCtx is for performing IBE private key operations
 * with the Identity eventually chosen to decrypt the message.
 * <p>If no DerCoders, SchemaDecodes, or mpCtx are supplied, the Impl
 * will try to find these elements in the libCtx.
 * <p>If the accompanying info is a NULL pointer, this Impl will try to
 * find all the info it needs from the libCtx.
 */
extern VtSecureMailImpl VtSecureMailImplRead;

/** Use this Impl to build a SecureMail object that will be able to
 * read SecureMail V2 messages.
 * <p>The data associated with VtSecureMailImplRead is a pointer to a
 * VtReadSecureMail2Info struct containing all the asymmetric and
 * symmetric algorithms (derCoders/derCoderCount fields) and
 * identity schemas (decoders/decoderCount fields) the app is willing
 * to support. The mpCtx is for performing IBE private key operations
 * with the identity eventually chosen to decrypt the message.
 * <p>If no DerCoders, SchemaDecodes, or mpCtx are supplied, the impl
 * will try to find these elements in the libCtx.
 * <p>If the accompanying info is a NULL pointer, this Impl will try to
 * find all the info it needs from the libCtx.
 */
extern VtSecureMailImpl VtSecureMail2ImplRead;

/** Use the Impl to build a SecureMail object that will be able to read
 * any version of the SecureMail format.
 * <p>The data associated with VtSecureMailImplRead is a pointer to a
 * VtReadSecureMailInfo struct containing all the asymmetric and
 * symmetric algorithms (derCoders/derCoderCount fields) and
 * identity schemas (decoders/decoderCount fields) the app is willing
 * to support. The mpCtx is for performing IBE private key operations
 * with the identity eventually chosen to decrypt the message.
 * <p>If no DerCoders, SchemaDecodes, or mpCtx are supplied, the impl
 * will try to find these elements in the libCtx.
 * <p>If the accompanying info is a NULL pointer, this Impl will try to
 * find all the info it needs from the libCtx.
 */
extern VtSecureMailImpl VtSecureMailImplReadGeneric;
 
/** This is the data struct to accompany VtSecureMailImplRead.
 */
typedef struct
{
  VtDerCoder **derCoders;
  unsigned int derCoderCount;
  VtIdentitySchemaDecode **decoders;
  unsigned int decoderCount;
  VtMpIntCtx mpCtx;
} VtReadSecureMailInfo;

/** This is the deprecated name for the associated info for the
 * VtSecureMail2ImplRead impl. You should use the
 * VtReadSecureMailInfo struct as the associated info for all
 * SecureMail read impls.
 */
typedef VtReadSecureMailInfo VtReadSecureMail2Info;

/** Destroy a SecureMail Object. This frees up any memory allocated
 * during the object's creation and use.
 * <p>If you call Create, you must call Destroy after you are done with
 * the object but before the program exits.
 * <p>Example:
 * <code>
 * <pre>
 *    int status;
 *    VtSecureMailObject secureMailObj = (VtSecureMailObject)0;
 *
 *    do {
 *          . . .
 *      status = VtCreateSecureMailObject (
 *        libCtx, VtSecureMailImplWrite, (Pointer)0, &secureMailObj);
 *      if (status != 0)
 *        break;
 *          . . .
 *    } while (0);
 *
 *    VtDestroySecureMailObject (&secureMailObj);
 * </pre>
 * </code>
 * @param secureMailObj A pointer to where the routine will find the
 * object to destroy.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtDestroySecureMailObject (
   VtSecureMailObject *secureMailObj
);

/** Set the secureMail object with the information given.
 * <p>The VtSecureMailParam defines what information the object will be
 * set with.
 * <p>The include file vibe.h defines the supported SecureMailParams.
 * Look through the include file to see which SecureMailParam to use for
 * your application. All supported SecureMailParams will be defined as
 * in the following example.
 * <code>
 * <pre>
 *   extern VtSecureMailParam VtSecureMailParamDataLen;
 * </pre>
 * </code>
 * <p>Associated with each SecureMailParam is specific info. The
 * documentation for each SecureMailParam will describe the associated
 * info it needs. That data could be another object, it could be data
 * in a particular struct, it might be a NULL pointer. Check the
 * documentation for each SecureMailParam for a description of the data
 * and its required format.
 * <p>To use this function decide which SecureMailParam you want to
 * use, then determine what information that SecureMailParam needs and
 * in which format it is presented. Collect the data in the appropriate
 * format then call this function passing in the desired
 * SecureMailParam and and the required info. The associated info must
 * be cast to Pointer.
 * <p>For example, if the required info is a particular struct, declare
 * a variable to be of that type struct, fill in the fields, then pass
 * the address of that struct cast to Pointer.
 *
 * @param secureMailObj The object to set.
 * @param secureMailParam What the object is being set to.
 * @param associatedInfo The info needed by the SecureMailParam.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtSetSecureMailParam (
   VtSecureMailObject secureMailObj,
   VtSecureMailParam secureMailParam,
   Pointer associatedInfo
);

/** Get the specified type of information out of a SecureMail object.
 * <p>This function will go to the address given by getInfo and
 * deposit a pointer. That pointer is the address of the data. The
 * memory to hold the information belongs to the object, do not free it.
 * <p>The VtSecureMailParam will specify what kind of information will
 * be returned, the getInfo is where the function will deposit the info.
 * <p>The include file vibe.h defines the supported SecureMailParams.
 * Look through the include file to see which SecureMailParam to use for
 * your application.
 * <p>See also VtSetSecureMailParam.
 * <p>To use this function decide which SecureMailParam you want to use,
 * then determine what information that SecureMailParam will return and
 * in which format it is presented. Declare a variable to be a pointer
 * to the appropriate type, then call this function passing in the
 * desired SecureMailParam and the address of the variable. Cast the
 * address of the pointer to the appropriate type: (Pointer *).
 * <p>If the associated info is already a pointer type (such as an
 * object), declare a variable to be of the appropriate type and pass
 * the address of that variable cast to (Pointer *).
 * <p>For example, if the returned info is a VtIdentityList, declare a
 * variable to be of type VtIdentityList, pass the address of that
 * variable (&varName) cast to (Pointer *).
 * <p>Example:
 * <code>
 * <pre>
 *    VtIdentityList getRecipList;
 *
 *    do {
 *          . . .
 *      status = VtGetSecureMailParam (
 *        readObj, VtSecureMailParamRecipientList,
 *        (Pointer *)&getRecipList);
 *      if (status != 0)
 *        break;
 *          . . .
 *    } while (0);
 * </pre>
 * </code>
 * @param secureMailObj The object to query.
 * @param secureMailParam Defines what info to get.
 * @param getInfo The address of a pointer. The function will go to
 * that address and deposit a pointer to the info requested.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtGetSecureMailParam (
   VtSecureMailObject secureMailObj,
   VtSecureMailParam secureMailParam,
   Pointer *getInfo
);

/* These are the VtSecureMailParams supported by the toolkit. Each
 * VtSecureMailParam is used in conjunction with special info for the
 * function.
 */

/** SetParam only.
 * <p>Use this Param to set a SecureMail object with a sender
 * represented as an identity. With an object built with this Param,
 * the toolkit will obtain the signing key and cert during the call to
 * VtWriteInit, either from storage or from the key server.
 * <p>If you set a SecureMailObject using this Param, do not set it
 * with another identity, either using this SecureMailParam or
 * VtSecureMailParamSenderInfo. That is, a SecureMail message has one
 * and only one sender.
 * <p>The info associated with this Param is an identity object.
 */
extern VtSecureMailParam VtSecureMailParamSenderId;

/** SetParam only.
 * <p>Use this Param to set a SecureMail object with sender info it will
 * need in order to send a message. This will include a private key with
 * which to sign and optionally the cert associated with that private
 * key.
 * <p>The accompanying data includes an identity object. That info is
 * optional. If you have an identity only, the toolkit will obtain the
 * key and cert (VtSecureMailParamSenderId). If you have the key and
 * cert, pass them in to save time. If you have the key, cert, and
 * identity, you can pass all three in. If the key and cert are not
 * associated with an identity (not an IBE-based sender), pass the key
 * and cert only.
 * <p>If you set a SecureMailObject using this Param, do not set it
 * with another sender identity, either using this SecureMailParam or
 * VtSecureMailParamSenderId. That is, a SecureMail message has one
 * and only one sender.
 * <p>The private key object passed in at this time will not be cloned.
 * That is, the SecureMailObject set with this Param will copy a
 * reference to that key. When the SecureMailObject needs to sign
 * (WriteFinal), it will use that key object. If you use this Param,
 * make sure the key object stays unchanged until after WriteFinal.
 * <p>The info associated with this Param is a pointer to a
 * VtSecureMailSenderInfo struct.
 */
extern VtSecureMailParam VtSecureMailParamSenderInfo;

/** SetParam and GetParam.
 * <p>Use this param to add a list of recipients to an object built to
 * write a SecureMail message.
 * <p>Or use this param to get the list of recipients of a SecureMail
 * message being read.
 * <p>When an object built to write a SecureMail message creates the
 * digital envelope, it will use each of the identities in the
 * IdentityList to build a RecipientInfo (which includes the session
 * key encrypted using the recipient's public key, which in this case
 * is the identity).
 * <p>When an object built to read a SecureMail message reads all the
 * RecipientInfos, it will build a VtIdentityList containing the
 * identities of all the recipients. The app can then get that list by
 * using this SecureMailParam, and then examine each of the identities
 * to choose which one the object should use to open the envelope
 * (indicate which identity to use by calling VtSetSecureMailParam with
 * VtSecureMailParamRecipientIndex).
 * <p>The info associated with this Param is a VtIdentityList object.
 */
extern VtSecureMailParam VtSecureMailParamRecipientList;

/** SetParam and GetParam.
 * <p>Use this param to add recipients to an object built to write a
 * SecureMail message, the recipients represented as an already
 * existing RecipientInfoList (as opposed to an IdentityList).
 * <p>Or use this param to get the recipients of a SecureMail message
 * being read, represented as a RecipientInfoList.
 * <p>When an object built to write SecureMail creates the digital
 * envelope, it will use the encoded RecipientInfos from the associated
 * info, instead of building it. It will also use the symmetric key and
 * symmetric algorithm specified in the associated info.
 * <p>When writing a SecureMail message, using this Param is equivalent
 * to using VtSecureMailParamRecipientList along with either
 * VtSecureMailParamEnv3DESCBC or VtSecureMailParamEnvAES128CBC.
 * <p>When an object built to read SecureMail is done reading the
 * entire message, an app can use this Param to get the
 * RecipientInfoList representation of the recipients. The app can then
 * get information out of the RecipientInfoList, such as the symmetric
 * key and algorithm.
 * <p>Note! When reading, the CHOOSE_RECIPIENT error means you get an
 * IdentityList out of the SecureMail object and you pick an identity
 * to use to decrypt. You cannot get the RecipientInfoList at that
 * time, you must wait until reading the entire message.
 * <p>After Getting the RecipientInfoList out of a SecureMail object,
 * you can call VtEncodeRecipientInfoList. If you want the symmetric key
 * protected in the RecipientInfoBlock, Set it with a secretValue
 * first. NOTE!!! This is a departure from normal toolkit rules.
 * Normally we say do not alter objects returned from a Get call,
 * however, in this case, the object returned by the Get will have no
 * secretValue installed, so if you want to use one, you must add one.
 * <p>The info associated with this Param is a VtRecipientInfoList
 * object.
 * <p>NOTE!!! If setting a SecureMail object with a RecipientInfoList,
 * each RecipientInfo must be encoded. That is, you must have called
 * VtEncodeRecipientInfo on each RecipientInfoObject inside the
 * RecipientInfoList, or you must have called
 * VtEncodeRecipientInfoList on the RecipientInfoList.
 * <p>NOTE!!! If setting a SecureMail object with a RecipientInfoList,
 * the RecipientInfoList MUST be set so that it can produce the
 * RecipientInfoBlock. (See the documentation on
 * VtEncodeRecipientInfoList for info on obtaining RecipientInfoBlock
 * versus RecipientInfos.)
 * <p>When getting, pass in the address of a VtRecipientInfoList
 * variable as the getInfo, the Get function will desposit a
 * VtRecipientInfoList at the address.
 */
extern VtSecureMailParam VtSecureMailParamRecipientInfoList;

/** GetParam only.
 * <p>Use this param to get the list of signers out of a SecureMail
 * message being read.
 * <p>When an object built to read SecureMail reads all the
 * SignerInfos, it will build a VtIdentityList containing the
 * identities of all the signers. The app can then get that list by
 * using this Param.
 * <p>The info associated with this Param is a VtIdentityList object.
 * <p>When getting, pass in the address of a VtIdentityList variable as
 * the getInfo, the Get function will desposit a VtIdentityList at the
 * address.
 */
extern VtSecureMailParam VtSecureMailParamSignerList;

/** SetParam only.
 * <p>Use this SecureMailParam to let a SecureMail Object built to read
 * know which identity to use to decrypt the message.
 * <p>When the return from VtSecureMailReadUpdate is
 * VT_ERROR_CHOOSE_RECIPIENT, the app should get the recipient list out
 * of of the SecureMail object (using VtSecureMailParamRecipientList),
 * examine the identities of the recipients, and choose which one should
 * be used to decrypt the message. The identity chosen will have an
 * index associated with it, it is the index into the IdentityList of
 * all the recipients. This Param tells the SecureMail object which
 * identity in the IdentityList the object should use.
 * <p>Once the SecureMail object knows which identity to use, it will
 * obtain the IBE private key. Therefore, the app also passes in policy,
 * storage, and transport contexts for the SecureMail object to use to
 * obtain the private key. If no contexts are passed in, the SecureMail
 * object will look in the libCtx for those entities.
 * <p>Note that this will only obtain an IBE private key with which to
 * decrypt a session key. If the session key was encrypted with any
 * other algorithm, this Param will not be able to load the appropriate
 * private key needed.
 * <p>The data associated with VtSecureMailParamRecipientIndex is a
 * pointer to a VtSecureMailRecipientIndexInfo struct.
 */
extern VtSecureMailParam VtSecureMailParamRecipientIndex;

/** SetParam only.
 * <p>Use this SecureMailParam with a SecureMail Object built to write.
 * It indicates the object should use Triple DES in CBC mode to encrypt
 * the bulk data.
 * <p>The SecureMail object will generate the IV and Triple DES key. It
 * will generate the key data using the random object supplied by the
 * application during the call to VtSecureMailWriteInit.
 * <p>The data associated with VtSecureMailParam3DESCBC is a NULL
 * pointer: (Pointer)0.
 */
extern VtSecureMailParam VtSecureMailParam3DESCBC;

/** SetParam only.
 * <p>Use this SecureMailParam with a SecureMail Object built to write.
 * It indicates the object should use AES-128 in CBC mode to encrypt
 * the bulk data.
 * <p>In creating the SecureMail message, the object will indicate,
 * using an algorithm identitifer, that the symmetric algorithm is
 * AES-128 in CBC mode. The OID is defined by NIST as
 * 2.16.840.1.101.3.4.1.2. The OID does not specify padding. In fact,
 * in an email message, a NIST representative explicitly stated that
 * the OID does not specify padding. However, the representative also
 * suggested following the instructions specified in Appendix A of NIST
 * Special Publication 800-38A. This appendix recommends applications
 * pad data when using AES in CBC mode, then gives one possible padding
 * scheme which is not the technique outlined in PKCS #5 (the most
 * commonly used block cipher padding scheme in use today). It further
 * states, "For the above padding method, the padding bits can be
 * removed unambiguously, provided the receiver can determine that the
 * message is indeed padded. One way to ensure that the receiver does
 * not mistakenly remove bits from an unpadded message is to require
 * the sender to pad every message, including messages in which the
 * final block (segment) is already complete. For such messages, an
 * entire block (segment) of padding is appended."
 * <p>A "standard" is supposed to specify how to do something in a
 * standard way, so that two entities can know what something means
 * without having to communicate anything in advance. But here is a
 * standard (along with the people wo wrote and disseminate it) that
 * explicitly states that the sender and receiver must communicate in
 * advance to resolve an intentional ambiguity in the standard.
 * <p>Fortunately, PKCS #7 specifies that all block ciphers in modes
 * that require complete blocks (such as CBC) shall pad (even if the
 * input length is a multiple of the block size) and shall pad
 * following the technique specified in PKCS #5. A SecureMail message
 * uses PKCS #7 to construct the underlying EnvelopedData.
 * <p>The SecureMail object will generate the IV and AES key. It will
 * generate the key data using the random object supplied by the
 * application during the call to VtSecureMailWriteInit.
 * <p>The data associated with VtSecureMailParamAES128CBC is a NULL
 * pointer: (Pointer)0.
 */
extern VtSecureMailParam VtSecureMailParamAES128CBC;

/** SetParam only
 * <p>Use this Param to indicate how long the data in a message will be.
 * A SecureMail object that will write, must know, before actually
 * writing data, how long the message will be. If the message is not
 * supplied to the object all at once (call WriteFinal without any
 * previous calls to WriteUpdate), use this Param.
 * <p>Note that this supplies the length of the actual data (for
 * instance, the data that will be built into a SecureMail message),
 * not the message itself.
 * <p>The info associated with this Param is a pointer to an unsigned
 * int giving the length of the data.
 */
extern VtSecureMailParam VtSecureMailParamDataLen;

/** SetParam and GetParam
 * <p>Set an object to write with a content type. Or Get the content
 * type from an object that has read a SecureMail message.
 * <p>A SecureMail message is actually the data you pass with some
 * other info prepended. This prepended info looks something like this.
 * <code>
 * <pre>
 *   content-type: text/plain
 *   content-length: 1486
 * </pre>
 * </code>
 * The content-type describes what the following data is. It might be
 * Multipart/alternative, text/html, text/plain, or other values. The
 * content-length is the length of the data you are securing.
 * <p>If you do not use this Param, the toolkit will set the
 * content-type to be text/plain.
 * <p>The toolkit will not check the value you pass in, it will simply
 * write out to the message the same thing you pass in.
 * <p>When reading a SecureMail message, you can get this param after
 * the object has read this first part of the data. Remember, when
 * getting the content-type out, the memory belongs to the object, do
 * not alter or free it.
 * <p>The data associated with VtSecureMailParamContentType is an
 * unsigned char array, a UTF-8 NULL-terminated string.
 * <p>When getting, pass in the address of a pointer as the getInfo,
 * the Get function will deposit a pointer to a buffer at that address.
 */
extern VtSecureMailParam VtSecureMailParamContentType;

/** SetParam only
 * <p>When the toolkit builds SecureMail messages, it will need to
 * include new lines. If the application does not specify a new line
 * character, the toolkit will use "CR LF" (carriage return line feed),
 * also known as 0x0D 0x0A.
 * <p>See also the documentation concerning VtBase64Info.
 * <p>The data associated with VtSecureMailParamNewLineCharacter is a
 * pointer to an unsigned int. The value of that int must be either
 * <code>
 * <pre>
 *    VT_SECURE_MAIL_NEW_LINE_LF
 *    VT_SECURE_MAIL_NEW_LINE_CR_LF
 * </pre>
 * </code>
 */
extern VtSecureMailParam VtSecureMailParamNewLineCharacter;

/** Use this flag as the value of the unsigned int that is the info to
 * accompany VtSecureMailParamNewLineCharacter to indicate that the
 * toolkit should use the "LineFeed" new line character when writing
 * out a new line.
 */
#define VT_SECURE_MAIL_NEW_LINE_LF     VT_BASE64_NEW_LINE_LF
/** Use this flag as the value of the unsigned int that is the info to
 * accompany VtSecureMailParamNewLineCharacter to indicate that the
 * toolkit should use the "CarriageReturn LineFeed" new line characters
 * when writing out a new line.
 */
#define VT_SECURE_MAIL_NEW_LINE_CR_LF  VT_BASE64_NEW_LINE_CR_LF

/** GetParam only - VtSecureMailImplRead only (i.e. V1 SecureMail only)
 * <p>Before signing and encrypting the message data, SecureMail adds
 * some content info. This will be elements such as
 * <code>
 * <pre>
 *    content-type: text/plain
 *    content-length: 4096
 * </pre>
 * </code>
 * <p>After reading a SecureMail message, if you want to know what
 * content info the message contained, call VtGetSecureMailParam with
 * this Param.
 * <p>There may be more than one element (for example, content-type is
 * one, content-length is another). Each element will be
 * NULL-terminated ASCII or NULL-terminated UTF-8.
 * <p>The info associated with this Param is a pointer to a
 * VtUtf8StringList struct.
 * <p>When getting, pass in the address of a pointer as the getInfo,
 * the Get function will desposit a pointer to a VtUtf8StringList
 * struct at the address.
 * <p>The V2 SecureMail format stores its headers in a different format,
 * so this param can only be used with the V1 impl.
 */
extern VtSecureMailParam VtSecureMailParamContentDescriptors;

/** GetParam only
 * <p>Currently, this is valid only with a SecureMail object built to
 * read.
 * <p>A SecureMail message contains the time it was signed (the
 * signingTimeAttribute in the P7 SignedData). This Param gets that
 * time out of the message.
 * <p>Get this info only after completely reading the message.
 * <p>The info associated with this Param is a pointer to a VtTime
 * struct.
 * <p>When getting, pass in the address of a pointer as the getInfo,
 * the Get function will desposit a pointer to a VtTime struct at the
 * address.
 */
extern VtSecureMailParam VtSecureMailParamMessageTime;

/** GetParam & SetParam; used only for SecureMail2 objects
 * <p>A SecureMail2 message contains a header indicating
 * the character set of the encrypted message data. This param
 * is used to set this value when writing a SecureMail2 message
 * or get it when reading a SecureMail2 message. To guarantee
 * compatibility with all existing Voltage products the character
 * set of the message data should be utf-8, which is the default
 * value for this param, so typically a toolkit app won't need
 * to call this param.
 * <p>Get this info only after completely reading the message.
 * <p>When setting this param, the associated info is a pointer to
 * a null-terminated UTF-8 string indicating the MIME character set.
 * If this param is not set by the caller, then the SecureMail object
 * will use a default value of "utf-8".
 * <p>When getting this param, the associated info is the address
 * of a unsigned char*. The GetParam call will return a pointer to
 * the character set string in that location. The SecureMail object
 * owns the string, so the caller should not free it.
 */
extern VtSecureMailParam VtSecureMailParamCharacterSet;

/** GetParam & SetParam; used only for SecureMail2 objects
 * <p>A SecureMail2 message contains a header indicating
 * the original character set of the encrypted message data.
 * To guarantee compatibility with all Voltage products, when writing
 * a SecureMail message a toolkit app should convert a message from
 * its original character set to the utf-8 character set before
 * inputting the message data to the SecureMail object. In this case,
 * the original character set param can be set so that an app reading
 * the SecureMail message can convert it back to the original character
 * set. This is often necessary in desktop clients for the message to
 * render properly. This param is used to set the value of the original
 * character set when writing a SecureMail2 message or get it when reading
 * a SecureMail2 message.
 * <p>A reader can get this info only after completely reading the message.
 * <p>When setting this param, the associated info is a pointer to
 * a null-terminated UTF-8 string indicating the MIME character set.
 * If this param is not set by the caller, then the SecureMail object
 * will use a default value of "utf-8".
 * <p>When getting this param, the associated info is the address
 * of a unsigned char*. The GetParam call will return a pointer to
 * the character set string in that location. The SecureMail object
 * owns the string, so the caller should not free it.
 */
extern VtSecureMailParam VtSecureMailParamOriginalCharacterSet;

/** SetParam only; used only for SecureMail2 objects
 * This param is used to set the charset attribute of the
 * content-type meta tag of an HTML-wrapped SecureMail2 message.
 * It has no effect if the message is plain-text-wrapped.
 * If this param is not set by the caller, then the default
 * value will be "utf-8". The output from the SecureMail object
 * will always be utf-8. You'd use this param if the toolkit
 * app was going to convert the message output from the SecureMail
 * object to a different character set before sending it.
 * <p>The info associated with this Param is a pointer to a
 * null-terminated UTF-8 string indicating the MIME character set.
 */
extern VtSecureMailParam VtSecureMailParamTemplateCharset;

/** SetParam only; used only for SecureMail2 objects
 * This param is used to specify which message format should be
 * used for the SecureMail2 message. For SecureMail objects the
 * message format is really just used to specify whether the
 * ciphertext should be wrapped in plain text or HTML, so the
 * format should either be VT_MESSAGE_FORMAT_SECURE_MAIL_PLAIN
 * or VT_MESSAGE_FORMAT_SECURE_MAIL_HTML. If this param is not
 * set explicitly then the default is to use HTML wrapping.
 */
extern VtSecureMailParam VtSecureMailParamMessageFormat;

/** GetParam only
 * <p>Once a SecureMail object has seen enough input to determine
 * that the input data is a SecureMail message (i.e. it has seen
 * the ----- begin message tag), then this param can be used to
 * determine which version was specified in the begin tag.
 * <p>If it's called before the begin message tag has been seen,
 * then it will return a VT_ERROR_UNDETERMINED_VERSION error.
 * If it's called after it's been determined that the message
 * is not SecureMail message, then it will return a
 * VT_ERROR_INVALID_SECURE_MAIL_MSG error. If it can determine
 * the version, then it will set the associated info to the
 * version and return 0.
 * <p>The info associated with this Param is a pointer to an
 * unsigned int that will be set to the version if it can
 * be determined.
 */
extern VtSecureMailParam VtSecureMailParamVersion;

/** This is the data struct to accompany VtSecureMailParamSenderInfo.
 * <p>The private key will be used to sign the message. It must be the
 * private key partner to the signerCert.
 * <p>The senderId is optional. If you have it available, pass it in,
 * if not, leave the field NULL.
 * <p>If you have an identity only, the toolkit will obtain the key and
 * cert (VtSecureMailParamSenderId). If you have the key and cert, pass
 * them in to save time. If you have the key, cert, and identity, you
 * can pass all three in. If the key and cert are not associated with
 * an identity (not an IBE-based sender), pass the key and cert only.
 */

typedef struct
{
  VtKeyObject privateKey;
  VtCertObject signerCert;
  VtIdentityObject senderId;
} VtSecureMailSenderInfo;

/** This is the data struct to accompany VtSecureMailParamRecipientIndex.
 */
typedef struct
{
  unsigned int index;
  VtPolicyCtx policyCtx;
  VtStorageCtx storageCtx;
  VtTransportCtx transportCtx;
} VtSecureMailRecipientIndexInfo;

/** This is the data struct to accompany
 * VtSecureMailParamContentDescriptors. Upon return from GetParam, the
 * utf8Strings field will point to an array of NULL-terminated strings.
 * <p>For example,
 * <code>
 * <pre>
 *   // Assume we have a function that displays lines of UTF-8
 *   // characters. That is, it takes in a NULL-terminated UTF-8 string
 *   // and prints it out.
 *   void DisplayLine (unsigned char *line);
 *
 *   unsigned int index;
 *   VtUtf8StringList *getContentDescriptors;
 *
 *     status = VtGetSecureMailParam (
 *       obj, VtSecureMailParamContentInfo,
 *       (Pointer *)&getContentDescriptors);
 *
 *     for (index = 0; index < getContentInfo->count; ++index)
 *       DisplayLine (getContentInfo->utf8Strings[index]);
 * </pre>
 * </code>
 */
typedef struct
{
  unsigned int count;
  unsigned char **utf8Strings;
} VtUtf8StringList;

/** Initialize the SecureMail object for writing. The object should
 * have been created with an Impl that creates a message as opposed to
 * reading a message. This function will make sure it has all the
 * information necessary to write the message (except for the actual
 * data itself) and collect any "missing" information using the policy,
 * storage, and transport contexts.
 * <p>The app had the opportunity to add info to the object during the
 * Create and Set functions, but some of that info might be
 * "incomplete" and the Init function will find any needed elements.
 * For example, in order to write the message, the object may need a
 * particular IBE key. If the app added only an Identity object, the
 * Init function will use the contexts to retrieve or download the
 * appropriate information to build the key.
 * <p>If no policy, storage, and/or transport ctx are given, the
 * function will use the contexts found in the libCtx (if any).
 *
 * @param secureMailObj The object created and set to write a
 * SecureMail message.
 * @param policyCtx The policy ctx to use if IBE operations are
 * performed.
 * @param storageCtx The storage ctx to use if the function needs to
 * retrieve or store elements needed to build the message.
 * @param transportCtx The transport ctx to use if IBE operations are
 * performed.
 * @param random A random object the function will use if it needs
 * random bytes (for a symmetric key or IV, for example).
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtSecureMailWriteInit (
   VtSecureMailObject secureMailObj,
   VtPolicyCtx policyCtx,
   VtStorageCtx storageCtx,
   VtTransportCtx transportCtx,
   VtRandomObject random
);

/** Write out message. This function will write out what it can. It
 * does not assume that it has all the data of the message yet. If this
 * is the first call to Update, it will write out some header information
 * along with the processed data itself. That is, the output will be
 * bigger than the input.
 * <p>If this is not the first call to Update, the input data will be
 * Base64 encoded, so the output will be bigger than the input.
 * <p>The function will deposit the output into the message buffer,
 * which is bufferSize bytes big. It will set the unsigned int at
 * messageLen to the number of bytes placed into the buffer. If the
 * buffer is not big enough, the function will return
 * VT_ERROR_BUFFER_TOO_SMALL and set messageLen to the required size.
 * <p>If the object was not set with a SecureMailImpl that can write,
 * the function will return an error.
 *
 * @param secureMailObj The SecureMail object built to write.
 * @param random A random object that can generate random numbers if
 * necessary.
 * @param inputData The actual data to build into a SecureMail message.
 * @param inputDataLen The length, in bytes, of the inputData.
 * @param message The output buffer, where the resulting SecureMail
 * message will be deposited.
 * @param bufferSize The size, in bytes, of the output buffer.
 * @param messageLen The address where the implementation will deposit
 * the length, the number of bytes placed into the output buffer.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtSecureMailWriteUpdate (
   VtSecureMailObject secureMailObj,
   VtRandomObject random,
   unsigned char *inputData,
   unsigned int inputDataLen,
   unsigned char *message,
   unsigned int bufferSize,
   unsigned int *messageLen
);

/** Finish writing out the message. This function will write out any
 * actual data left, then write out any trailing information (such as
 * footers). This function will match the total input to the specified
 * input from the call to SetSecureMailParam with the
 * VtSecureMailParamDataLen Param (if it was used). If there had been no
 * call to Update, this function may write out some header information
 * along with the processed data itself. That is, the output may be
 * bigger than the input.
 * <p>The function will deposit the output into the message buffer,
 * which is bufferSize bytes big. It will set the unsigned int at
 * messageLen to the number of bytes placed into the buffer. If the
 * buffer is not big enough, the function will return
 * VT_ERROR_BUFFER_TOO_SMALL and set messageLen to the required size.
 * <p>If the object was not set with a SecureMailImpl that can write,
 * the function will return an error.
 *
 * @param secureMailObj The SecureMail object built to write.
 * @param random A random object that can generate random numbers if
 * necessary.
 * @param inputData The actual message data.
 * @param inputDataLen The length, in bytes, of the inputData.
 * @param message The output buffer, where the resulting P7 message
 * will be deposited.
 * @param bufferSize The size, in bytes, of the output buffer.
 * @param messageLen The address where the implementation will deposit
 * the length, the number of bytes placed into the output buffer.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtSecureMailWriteFinal (
   VtSecureMailObject secureMailObj,
   VtRandomObject random,
   unsigned char *inputData,
   unsigned int inputDataLen,
   unsigned char *message,
   unsigned int bufferSize,
   unsigned int *messageLen
);

/** Initialize the SecureMail object for reading. The object should
 * have been created with an Impl that reads a message as opposed to
 * writing a message. This function will make sure it has all the
 * information necessary to read the message (except for the actual
 * message itself).
 *
 * @param secureMailObj The object created and set to read a SecureMail
 * message.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtSecureMailReadInit (
   VtSecureMailObject secureMailObj
);

/** Begin or continue reading a SecureMail message. This function does
 * not demand that the entire SecureMail message be supplied, it will
 * read whatever is given.
 * <p>The function will go to the address given by bytesRead and
 * deposit there an unsigned int indicating how many bytes of the
 * message the function read. It is possible that the function does not
 * read all the bytes given. For example, if given the first part of a
 * message, the function may read the leading info (headers, etc.) but
 * not the actual data itself. That can happen if the output buffer
 * given is not big enough to hold the data the function needs to
 * return. Hence, the function read up to the data, but stopped. The
 * function will return the bytes read and the BUFFER_TOO_SMALL error.
 * <p>An application should call Update, check the bytesRead, and if
 * the bytesRead is not the same as the input length, call Update
 * again, this time on the input data bytesRead further along. If the
 * bytesRead returned is the same as the messageLen, then the
 * application should call Update with the next block of input.
 * <p>If there is any data to return, the function will return it in
 * the given outputData buffer (bufferSize bytes big) and will place
 * into the unsigned int at outputDataLen the number of bytes placed
 * into the buffer.  If the buffer is not big enough, the routine will
 * return VT_ERROR_BUFFER_TOO_SMALL and set outputDataLen to the
 * required size.
 * <p>It is possible that there is no outputData. For instance, if the
 * Update is called with only the first part of a SecureMail message,
 * and the data given contains only header info, then the function will
 * read that data but have no content to return. Therefore, if you call
 * this function with a NULL output buffer, it is still possible the
 * return is 0 (no error) rather than the BUFFER_TOO_SMALL error.
 *
 * @param secureMailObj The SecureMail object built to read.
 * @param message The current part of the SecureMail message.
 * @param messageLen The length, in bytes, of the message data.
 * @param bytesRead The address where the routine will deposit the
 * number of bytes of the message actually read. If 0, call the Update
 * function again with the same data. If messageLen, the function read
 * all the bytes, the next call should pass in the next bytes in the
 * message.
 * @param outputData The output buffer, where the resulting content
 * data will be deposited.
 * @param bufferSize The size, in bytes, of the output buffer.
 * @param outputDataLen The address where the function will deposit
 * the length, the number of bytes placed into the output buffer.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtSecureMailReadUpdate (
   VtSecureMailObject secureMailObj,
   unsigned char *message,
   unsigned int messageLen,
   unsigned int *bytesRead,
   unsigned char *outputData,
   unsigned int bufferSize,
   unsigned int *outputDataLen
);

/** Finish reading a SecureMail message. This function will check to
 * make sure the entire SecureMail message was supplied, either during
 * all the calls to Update and this call to Final, or all at once if
 * this is the only call to Read.
 * <p>The function will go to the address given by bytesRead and
 * deposit there an unsigned int indicating how many bytes of the
 * buffer the function read. It is possible that the function does not
 * read all the bytes given. For example, it may read up to the part
 * where the function needs the app to choose a recipient before it can
 * read further. Or the output buffer is not big enough.
 * <p>An application should call Final, check the bytesRead, and if
 * the bytesRead is not the same as the input length, call Final again,
 * this time on the input data bytesRead further along. If the
 * bytesRead returned is the same as the messageLen, then the entire
 * message has been read.
 * <p>If there is any data to return, the function will return it in
 * the given outputData buffer (bufferSize bytes big) and will place
 * into the unsigned int at outputDataLen the number of bytes placed
 * into the buffer.  If the buffer is not big enough, the routine will
 * return VT_ERROR_BUFFER_TOO_SMALL and set outputDataLen to the
 * required size.
 * <p>It is possible that there is no outputData. For instance, if
 * previous calls to Update collected all the actual data, this call to
 * Final will be reading only "trailing" information in the message.
 * <p>If the input does not complete a message, this function will
 * return an error.
 * <p>Note that this function does not verify signatures, it is only
 * able to read the message. To verify signatures, after a successful
 * call to ReadFinal, call a SecureMailVerify function.
 *
 * @param secureMailObj The SecureMail object built to read.
 * @param message The current part of the SecureMail message.
 * @param messageLen The length, in bytes, of the message data.
 * @param bytesRead The address where the routine will deposit the
 * number of bytes of the message actually read. If 0, call the Final
 * function again with the same data. If messageLen, the function read
 * all the bytes, the next call should pass in the next bytes in the
 * message.
 * @param outputData The output buffer, where the resulting content
 * data will be deposited.
 * @param bufferSize The size, in bytes, of the output buffer.
 * @param outputDataLen The address where the function will deposit
 * the length, the number of bytes placed into the output buffer.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtSecureMailReadFinal (
   VtSecureMailObject secureMailObj,
   unsigned char *message,
   unsigned int messageLen,
   unsigned int *bytesRead,
   unsigned char *outputData,
   unsigned int bufferSize,
   unsigned int *outputDataLen
);

/** Verify all signatures in a SecureMail message. If there is more than
 * one signer, this function will try to verify all signatures and all
 * signer certs. If one element does not verify, this function sets the
 * verifyResult to did not verify.
 * <p>Before calling this function the application must finish reading
 * the message. That is, do not call this function until after a
 * successful call to ReadFinal.
 * <p>The caller passes in policy, storage and transport contexts the
 * function will use to help it find certificates it needs to chain a
 * leaf cert. The function may download district parameters to obtain
 * certs.
 * <p>The caller also passes in a certVerifyCtx, which this function
 * will use to verify any untrusted certs it encounters. The caller
 * must also pass in the appropriate associated info (verifyCtxInfo)
 * for the particular certVerifyCtx. That is the info the specific ctx
 * needs to verify a cert. This associated info will be applied to each
 * leaf cert the function verifies.
 *
 * @param secureMailObj The object built to read and used to read an
 * entire SecureMail message.
 * @param policyCtx A policyCtx to use if necessary.
 * @param storageCtx A storageCtx to use to help find any certs needed
 * to verify signatures or other certs.
 * @param transportCtx A transportCtx to use if necessary.
 * @param certVerifyCtx The certVerifyCtx the function will use when
 * verifying untrusted certs in a chain.
 * @param verifyCtxInfo The info the certVerifyCtx needs to help it
 * verify. This info will be applied to leaf certs.
 * @param vfyFailList Currently unused, reserved for future
 * implementations.
 * @param verifyResult The address where this function will deposit the
 * result of the verify, one for verify, zero for not verified.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtSecureMailVerify (
   VtSecureMailObject secureMailObj,
   VtPolicyCtx policyCtx,
   VtStorageCtx storageCtx,
   VtTransportCtx transportCtx,
   VtCertVerifyCtx certVerifyCtx,
   Pointer verifyCtxInfo,
   VtVerifyFailureList vfyFailList,
   unsigned int *verifyResult
);

/*@}*/

/*=========================================================*/
/*                                                         */
/* Secure File Object                                      */
/*                                                         */
/*=========================================================*/

/**
 * \defgroup SecureFileGroup SecureFile Object
 */

/*@{*/

/** The SecureFile object.
 * <p>Note that the object is a pointer type.
 * <p>Note also that the SecureFile object does NOT read and write
 * files, it only reads and writes data in the SecureFile format. That
 * is, it reads and writes data from and to memory, just like the
 * SecureMail object. It is the responsibility of the application to
 * perform the file reads and writes.
 */
typedef struct VtSecureFileObjectDef *VtSecureFileObject;

/** The function VtCreateSecureFileObject builds an object that builds
 * or reads SecureFile content using a VtSecureFileImpl. This typedef
 * defines what a VtSecureFileImpl is. Although a VtSecureFileImpl is a
 * function pointer, an application should never call one directly,
 * only pass it as an argument to VtCreateSecureFileObject.
 */
typedef int VT_CALLING_CONV (VtSecureFileImpl) (
   VtSecureFileObject *, Pointer, unsigned int);

/** The functions VtSetSecureFileParam and VtGetSecureFileParam add or
 * get information to or from a SecureFile object. The information to
 * add or get is defined by a VtSecureFileParam. This typedef defines
 * what a VtSecureFileParam is. Although a VtSecureFileParam is a
 * function pointer, an application should never call one directly,
 * only pass it as an argument to VtSetSecureFileParam or
 * VtGetSecureFileParam. 
 */
typedef int VT_CALLING_CONV (VtSecureFileParam) (
   VtSecureFileObject, Pointer, unsigned int);

/** Create a new SecureFile object. This allocates space for an "empty"
 * object, then loads the given SecureFileImpl to make it an "active"
 * object.
 * <p>Note that the SecureFile object does NOT read and write files, it
 * only reads and writes data in the SecureFile format. That is, it
 * reads and writes data from and to memory, just like the SecureMail
 * object. It is the responsibility of the application to perform the
 * file reads and writes.
 * <p>The VtSecureFileImpl defines what the object will be able to
 * process (generally reading and writing will be different Impls).
 * The include file vibe.h defines the supported SecureFileImpls. Look
 * through the include file to see which SecureFileImpl to use for your
 * application. All supported SecureFileImpls will be defined as in
 * the following example.
 * <code>
 * <pre>
 *   extern VtSecureFileImpl VtSecureFileImplWrite;
 * </pre>
 * </code>
 * <p>Associated with each SecureFileImpl is specific info. The
 * documentation for each SecureFileImpl will describe the associated
 * info it needs. That data could be another object, it could be data
 * in a particular struct, it might be a NULL pointer. Check the
 * documentation for each SecureFileImpl for a description of the data
 * and its required format.
 * <p>To use this function decide which SecureFileImpl you want to use,
 * then determine what information that SecureFileImpl needs and in which
 * format it is presented. Collect the data in the appropriate format
 * then call this function passing in the desired SecureFileImpl and the
 * required info. The associated info must be cast to Pointer.
 * <p>For example, if the required info is a VtItem struct, declare a
 * variable to be of type VtItem, fill in the fields, then pass the
 * address of that struct cast to Pointer.
 * <p>The input secureFileObj is a pointer to an object. It should
 * point to a NULL VtSecureFileObject. This function will go to the
 * address given and deposit a created object.
 * <p>If you call Create, you must call Destroy after you are done with
 * the object but before the program exits.
 * <p>Example:
 * <code>
 * <pre>
 *    int status;
 *    VtSecureFileObject secureFileObj = (VtSecureFileObject)0;
 *
 *    do {
 *          . . .
 *      status = VtCreateSecureFileObject (
 *        libCtx, VtSecureFileImplWrite, (Pointer)0, &secureFileObj);
 *      if (status != 0)
 *        break;
 *          . . .
 *    } while (0);
 *
 *    VtDestroySecureFileObject (&secureFileObj);
 * </pre>
 * </code>
 * @param libCtx The library context.
 * @param secureFileImpl The implementation the object will use.
 * @param associatedInfo The info needed by the SecureFileImpl.
 * @param secureFileObj A pointer to where the routine will deposit the
 * created object.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtCreateSecureFileObject (
   VtLibCtx libCtx,
   VtSecureFileImpl secureFileImpl,
   Pointer associatedInfo,
   VtSecureFileObject *secureFileObj
);

/* These are the VtSecureFileImpls supported by the toolkit. Each
 * VtSecureFileImpl is used in conjunction with special info for the
 * function. If there is no special info, the accompaniment is a NULL
 * pointer.
 */

/** Use this Impl to build a SecureFile object that will be able to
 * create SecureFile contents.
 * <p>Note that the SecureFile object does NOT read and write files, it
 * only reads and writes data in the SecureFile format. That is, it
 * reads and writes data from and to memory, just like the SecureMail
 * object. It is the responsibility of the application to perform the
 * file reads and writes.
 * <p>The data associated with VtSecureFileImplWrite is a NULL pointer:
 * (Pointer)0.
 */
extern VtSecureFileImpl VtSecureFileImplWrite;

/** Use this Impl to build a SecureFile object that will be able to
 * read SecureFile content.
 * <p>Note that the SecureFile object does NOT read and write files, it
 * only reads and writes data in the SecureFile format. That is, it
 * reads and writes data from and to memory, just like the SecureMail
 * object. It is the responsibility of the application to perform the
 * file reads and writes.
 * <p>The data associated with VtSecureFileImplRead is a pointer to a
 * VtReadSecureFileInfo struct containing all the algorithms
 * (asymmetric and symmetric algorithms represented as DERCoders) and
 * identity schemas (represented as SchemaDecodes) the app is willing
 * to support. The mpCtx is for performing IBE private key operations
 * with the Identity eventually chosen to decrypt the content.
 * <p>If no DerCoders, SchemaDecodes, or mpCtx are supplied, the Impl
 * will try to find these elements in the libCtx.
 * <p>If the accompanying info is a NULL pointer, this Impl will try to
 * find all the info it needs from the libCtx.
 */
extern VtSecureFileImpl VtSecureFileImplRead;

/** This is the data struct to accompany VtSecureFileImplRead.
 */
typedef struct
{
  VtDerCoder **derCoders;
  unsigned int derCoderCount;
  VtIdentitySchemaDecode **decoders;
  unsigned int decoderCount;
  VtMpIntCtx mpCtx;
} VtReadSecureFileInfo;

/** Destroy a SecureFile Object. This frees up any memory allocated
 * during the object's creation and use.
 * <p>If you call Create, you must call Destroy after you are done with
 * the object but before the program exits.
 * <p>Example:
 * <code>
 * <pre>
 *    int status;
 *    VtSecureFileObject secureFileObj = (VtSecureFileObject)0;
 *
 *    do {
 *          . . .
 *      status = VtCreateSecureFileObject (
 *        libCtx, VtSecureFileImplWrite, (Pointer)0, &secureFileObj);
 *      if (status != 0)
 *        break;
 *          . . .
 *    } while (0);
 *
 *    VtDestroySecureFileObject (&secureFileObj);
 * </pre>
 * </code>
 * @param secureFileObj A pointer to where the routine will find the
 * object to destroy.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtDestroySecureFileObject (
   VtSecureFileObject *secureFileObj
);

/** Set the secureFile object with the information given.
 * <p>The VtSecureFileParam defines what information the object will be
 * set with.
 * <p>The include file vibe.h defines the supported SecureFileParams.
 * Look through the include file to see which SecureFileParam to use for
 * your application. All supported SecureFileParams will be defined as
 * in the following example.
 * <code>
 * <pre>
 *   extern VtSecureFileParam VtSecureFileParamDataLen;
 * </pre>
 * </code>
 * <p>Associated with each SecureFileParam is specific info. The
 * documentation for each SecureFileParam will describe the associated
 * info it needs. That data could be another object, it could be data
 * in a particular struct, it might be a NULL pointer. Check the
 * documentation for each SecureFileParam for a description of the data
 * and its required format.
 * <p>To use this function decide which SecureFileParam you want to
 * use, then determine what information that ParameterParam needs and
 * in which format it is presented. Collect the data in the appropriate
 * format then call this function passing in the desired
 * SecureFileParam and and the required info. The associated info must
 * be cast to Pointer.
 * <p>For example, if the required info is a particular struct, declare
 * a variable to be of that type struct, fill in the fields, then pass
 * the address of that struct cast to Pointer.
 *
 * @param secureFileObj The object to set.
 * @param secureFileParam What the object is being set to.
 * @param associatedInfo The info needed by the SecureFileParam.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtSetSecureFileParam (
   VtSecureFileObject secureFileObj,
   VtSecureFileParam secureFileParam,
   Pointer associatedInfo
);

/** Get the specified type of information out of a SecureFile object.
 * <p>This function will go to the address given by getInfo and
 * deposit a pointer. That pointer is the address of the data. The
 * memory to hold the information belongs to the object, do not free it.
 * <p>The VtSecureFileParam will specify what kind of information will
 * be returned, the getInfo is where the function will deposit the info.
 * <p>The include file vibe.h defines the supported SecureFileParams.
 * Look through the include file to see which SecureFileParam to use for
 * your application.
 * <p>See also VtSetSecureFileParam.
 * <p>To use this function decide which SecureFileParam you want to use,
 * then determine what information that SecureFileParam will return and
 * in which format it is presented. Declare a variable to be a pointer
 * to the appropriate type, then call this function passing in the
 * desired SecureFileParam and the address of the variable. Cast the
 * address of the pointer to the appropriate type: (Pointer *).
 * <p>If the associated info is already a pointer type (such as an
 * object), declare a variable to be of the appropriate type and pass
 * the address of that variable cast to (Pointer *).
 * <p>For example, if the returned info is a VtIdentityList, declare a
 * variable to be of type VtIdentityList, pass the address of that
 * variable (&varName) cast to (Pointer *).
 * <p>Example:
 * <code>
 * <pre>
 *    VtIdentityList getRecipList;
 *
 *    do {
 *          . . .
 *      status = VtGetSecureFileParam (
 *        readObj, VtSecureFileParamRecipientList,
 *        (Pointer *)&getRecipList);
 *      if (status != 0)
 *        break;
 *          . . .
 *    } while (0);
 * </pre>
 * </code>
 * @param secureFileObj The object to query.
 * @param secureFileParam Defines what info to get.
 * @param getInfo The address of a pointer. The function will go to
 * that address and deposit a pointer to the info requested.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtGetSecureFileParam (
   VtSecureFileObject secureFileObj,
   VtSecureFileParam secureFileParam,
   Pointer *getInfo
);

/* These are the VtSecureFileParams supported by the toolkit. Each
 * VtSecureFileParam is used in conjunction with special info for the
 * function.
 */

/** SetParam only.
 * <p>Use this Param to set a SecureFile object with a sender
 * represented as an identity. With an object built with this Param,
 * the toolkit will obtain the signing key and cert during the call to
 * VtWriteInit, either from storage or from the key server.
 * <p>If you set a SecureFileObject using this Param, do not set it
 * with another identity, either using this SecureFileParam or
 * VtSecureFileParamSenderInfo. That is, SecureFile content has one and
 * only one sender.
 * <p>The info associated with this Param is an identity object.
 */
extern VtSecureFileParam VtSecureFileParamSenderId;

/** SetParam only.
 * <p>Use this Param to set a SecureFile object with sender info it will
 * need in order to encrypt content. This will include a private key
 * with which to sign and optionally the cert associated with that
 * private key.
 * <p>The accompanying data includes an identity object. That info is
 * optional. If you have an identity only, the toolkit will obtain the
 * key and cert (VtSecureFileParamSenderId). If you have the key and
 * cert, pass them in to save time. If you have the key, cert, and
 * identity, you can pass all three in. If the key and cert are not
 * associated with an identity (not an IBE-based sender), pass the key
 * and cert only.
 * <p>If you set a SecureFileObject using this Param, do not set it
 * with another sender identity, either using this SecureFileParam or
 * VtSecureFileParamSenderId. That is, SecureFile content has one and
 * only one sender.
 * <p>The private key object passed in at this time will not be cloned.
 * That is, the SecureFileObject set with this Param will copy a
 * reference to that key. When the SecureFileObject needs to sign
 * (WriteFinal), it will use that key object. If you use this Param,
 * make sure the key object stays unchanged until after WriteFinal.
 * <p>The info associated with this Param is a pointer to a
 * VtSecureFileSenderInfo struct.
 */
extern VtSecureFileParam VtSecureFileParamSenderInfo;

/** SetParam and GetParam.
 * <p>Use this param to add a list of recipients to an object built to
 * write SecureFile content.
 * <p>Or use this param to get the list of recipients of SecureFile
 * content being read.
 * <p>When an object built to write SecureFile content creates the
 * digital envelope, it will use each of the identities in the
 * IdentityList to build a RecipientInfo (which includes the session
 * key encrypted using the recipient's public key, which in this case
 * is the identity).
 * <p>When an object built to read SecureFile content reads all the
 * RecipientInfos, it will build a VtIdentityList containing the
 * identities of all the recipients. The app can then get that list by
 * using this SecureFileParam, and then examine each of the identities
 * to choose which one the object should use to open the envelope
 * (indicate which identity to use by calling VtSetSecureFileParam with
 * VtSecureFileParamRecipientIndex).
 * <p>The info associated with this Param is a VtIdentityList object.
 */
extern VtSecureFileParam VtSecureFileParamRecipientList;

/** SetParam and GetParam.
 * <p>Use this param to add recipients to an object built to write
 * SecureFile content, the recipients represented as an already
 * existing RecipientInfoList (as opposed to an IdentityList).
 * <p>Or use this param to get the recipients of SecureFile content
 * being read, represented as a RecipientInfoList.
 * <p>When an object built to write SecureFile creates the digital
 * envelope, it will use the encoded RecipientInfos from the associated
 * info, instead of building it. It will also use the symmetric key and
 * symmetric algorithm specified in the associated info.
 * <p>When writing SecureFile content, using this Param is equivalent
 * to using VtSecureFileParamRecipientList along with either
 * VtSecureFileParamEnv3DESCBC or VtSecureFileParamEnvAES128CBC.
 * <p>When an object built to read SecureFile is done reading the
 * entire contents, an app can use this Param to get the
 * RecipientInfoList representation of the recipients. The app can then
 * get information out of the RecipientInfoList, such as the symmetric
 * key and algorithm.
 * <p>Note! When reading, the CHOOSE_RECIPIENT error means you get an
 * IdentityList out of the SecureMail object and you pick an identity
 * to use to decrypt. You cannot get the RecipientInfoList at that
 * time, you must wait until reading the entire contents.
 * <p>After Getting the RecipientInfoList out of a SecureFile object,
 * you can call VtEncodeRecipientInfoList. If you want the symmetric key
 * protected in the RecipientInfoBlock, Set it with a secretValue
 * first. NOTE!!! This is a departure from normal toolkit rules.
 * Normally we say do not alter objects returned from a Get call,
 * however, in this case, the object returned by the Get will have no
 * secretValue installed, so if you want to use one, you must add one.
 * <p>The info associated with this Param is a VtRecipientInfoList
 * object.
 * <p>NOTE!!! If setting a SecureFile object with a RecipientInfoList,
 * each RecipientInfo must be encoded. That is, you must have called
 * VtEncodeRecipientInfo on each RecipientInfoObject inside the
 * RecipientInfoList, or you must have called
 * VtEncodeRecipientInfoList on the RecipientInfoList.
 * <p>NOTE!!! If setting a SecureFile object with a RecipientInfoList,
 * the RecipientInfoList MUST be set so that it can produce the
 * RecipientInfoBlock. (See the documentation on
 * VtEncodeRecipientInfoList for info on obtaining RecipientInfoBlock
 * versus RecipientInfos.)
 * <p>When getting, pass in the address of a VtRecipientInfoList
 * variable as the getInfo, the Get function will desposit a
 * VtRecipientInfoList at the address.
 */
extern VtSecureFileParam VtSecureFileParamRecipientInfoList;

/** SetParam only.
 * <p>Use this SecureFileParam to let a SecureFile Object built to read
 * know which identity to use to decrypt the content.
 * <p>When the return from VtSecureFileReadUpdate is
 * VT_ERROR_CHOOSE_RECIPIENT, the app should get the recipient list out
 * of of the SecureFile object (using VtSecureFileParamRecipientList),
 * examine the identities of the recipients, and choose which one should
 * be used to decrypt the content. The identity chosen will have an
 * index associated with it, it is the index into the IdentityList of
 * all the recipients. This Param tells the SecureFile object which
 * identity in the IdentityList the object should use.
 * <p>Once the SecureFile object knows which identity to use, it will
 * obtain the IBE private key. Therefore, the app also passes in policy,
 * storage, and transport contexts for the SecureFile object to use to
 * obtain the private key. If no contexts are passed in, the SecureFile
 * object will look in the libCtx for those entities.
 * <p>Note that this will only obtain an IBE private key with which to
 * decrypt a session key. If the session key was encrypted with any
 * other algorithm, this Param will not be able to load the appropriate
 * private key needed.
 * <p>The data associated with VtSecureFileParamRecipientIndex is a
 * pointer to a VtSecureFileRecipientIndexInfo struct.
 */
extern VtSecureFileParam VtSecureFileParamRecipientIndex;

/** SetParam only.
 * <p>Use this SecureFileParam with a SecureFile Object built to write.
 * It indicates the object should use Triple DES in CBC mode to encrypt
 * the bulk data.
 * <p>The SecureFile object will generate the IV and Triple DES key. It
 * will generate the key data using the random object supplied by the
 * application during the call to VtSecureFileWriteInit.
 * <p>The data associated with VtSecureFileParam3DESCBC is a NULL
 * pointer: (Pointer)0.
 */
extern VtSecureFileParam VtSecureFileParam3DESCBC;

/** SetParam only.
 * <p>Use this SecureFileParam with a SecureFile Object built to write.
 * It indicates the object should use AES-128 in CBC mode to encrypt
 * the bulk data.
 * <p>In creating the SecureFile content, the object will indicate,
 * using an algorithm identitifer, that the symmetric algorithm is
 * AES-128 in CBC mode. The OID is defined by NIST as
 * 2.16.840.1.101.3.4.1.2. The OID does not specify padding. In fact,
 * in an email message, a NIST representative explicitly stated that
 * the OID does not specify padding. However, the representative also
 * suggested following the instructions specified in Appendix A of NIST
 * Special Publication 800-38A. This appendix recommends applications
 * pad data when using AES in CBC mode, then gives one possible padding
 * scheme which is not the technique outlined in PKCS #5 (the most
 * commonly used block cipher padding scheme in use today). It further
 * states, "For the above padding method, the padding bits can be
 * removed unambiguously, provided the receiver can determine that the
 * message is indeed padded. One way to ensure that the receiver does
 * not mistakenly remove bits from an unpadded message is to require
 * the sender to pad every message, including messages in which the
 * final block (segment) is already complete. For such messages, an
 * entire block (segment) of padding is appended."
 * <p>A "standard" is supposed to specify how to do something in a
 * standard way, so that two entities can know what something means
 * without having to communicate anything in advance. But here is a
 * standard (along with the people wo wrote and disseminate it) that
 * explicitly states that the sender and receiver must communicate in
 * advance to resolve an intentional ambiguity in the standard.
 * <p>Fortunately, PKCS #7 specifies that all block ciphers in modes
 * that require complete blocks (such as CBC) shall pad (even if the
 * input length is a multiple of the block size) and shall pad
 * following the technique specified in PKCS #5. SecureFile content
 * uses PKCS #7 to construct the underlying EnvelopedData.
 * <p>The SecureFile object will generate the IV and AES key. It will
 * generate the key data using the random object supplied by the
 * application during the call to VtSecureFileWriteInit.
 * <p>The data associated with VtSecureFileParamAES128CBC is a NULL
 * pointer: (Pointer)0.
 */
extern VtSecureFileParam VtSecureFileParamAES128CBC;

/** SetParam only.
 * <p>Use this Param to indicate to a SecureFile object built to write
 * that the encrypted data created will be either the a file or an
 * attachment to a SecureMail v.1 message. A SecureFile has a string
 * "version: 1" at the beginning, an attachment does not. That is, an
 * attachment is the same as a SecureFile except it does not have the
 * "version: 1" string.
 * <p>The data associated with VtSecureFileParamMessageType is a
 * pointer to an unsigned int set to either VT_SECURE_FILE_TYPE_FILE or
 * VT_SECURE_FILE_TYPE_ATTACHMENT.
 * <p>The default is FILE. That is, if you do not set a SecureFile
 * object with this Param, the object will build file contents.
 */
extern VtSecureFileParam VtSecureFileParamMessageType;

/** For use with VtSecureFileParamMessageType, set a flag to this value
 * to indicate the data to build into SecureFile format is a file.
 */
#define VT_SECURE_FILE_TYPE_FILE        1
/** For use with VtSecureFileParamMessageType, set a flag to this value
 * to indicate the data to build into SecureFile format is an
 * attachment to a SecureMail v.1 message.
 */
#define VT_SECURE_FILE_TYPE_ATTACHMENT  2

/** SetParam and GetParam
 * <p>Set an object to write with a content type. Or Get the content
 * type from an object that has read SecureFile contents.
 * <p>SecureFile contents actually consist of the data you pass with
 * some other info prepended. This prepended info looks something like
 * this.
 * <code>
 * <pre>
 *   content-type: text/plain
 *   content-length: 1486
 * </pre>
 * </code>
 * The content-type describes what the following data is. It might be
 * Multipart/alternative, text/html, text/plain, or other values. The
 * content-length is the length of the data you are securing.
 * <p>When writing SecureFile contents, the object can specify the
 * content type for the body in its encrypted headers to be compatible
 * with existing Voltage products, so if this param is not called
 * explicitly for the message body the SecureFile object will default
 * to use text/plain. If you're using some other content type, then you
 * should call this param with that content type.
 * <p>The toolkit will not check the value you pass in, it will simply
 * write out to the message the same thing you pass in.
 * <p>When reading SecureFile contents, you can get this param after the
 * object has read this first part of the data. Remember, when getting
 * the content-type out, the memory belongs to the object, do not alter
 * or free it.
 * <p>The data associated with VtSecureFileParamContentType is an
 * unsigned char array, a UTF-8 NULL-terminated string.
 * <p>When getting, pass in the address of a pointer as the getInfo,
 * the Get function will desposit a pointer to a buffer at the address.
 */
extern VtSecureFileParam VtSecureFileParamContentType;

/** SetParam and GetParam
 * <p>When writing SecureFile info, use this Param to indicate what the
 * file name is of the file contents being encrypted.
 * <p>When reading, after contents have been read, this will allow an
 * app to extract the file name out of the object.
 * <p>The info associated with this Param is a UTF-8 string, a
 * NULL-terminated unsigned char array (unsigned char *).
 * <p>When getting, pass in the address of a pointer as the getInfo,
 * the Get function will desposit a pointer to a buffer at the address.
 */
extern VtSecureFileParam VtSecureFileParamFileName;

/** GetParam only.
 * <p>Use this param to get the list of signers out of SecureFile
 * content being read.
 * <p>When an object built to read SecureFile reads all the
 * SignerInfos, it will build a VtIdentityList containing the
 * identities of all the signers. The app can then get that list by
 * using this Param.
 * <p>The info associated with this Param is a VtIdentityList object.
 * <p>When getting, pass in the address of a VtIdentityList variable as
 * the getInfo, the Get function will desposit a VtIdentityList at the
 * address.
 */
extern VtSecureFileParam VtSecureFileParamSignerList;

/** GetParam only
 * <p>Before signing and encrypting the file contents, SecureFile adds
 * some content info. This will be elements such as
 * <code>
 * <pre>
 *    content-type: application/octet-string; file-name=
 * </pre>
 * </code>
 * <p>After reading SecureFile content, if you want to know what
 * content info the message contained, call VtGetSecureFileParam with
 * this Param.
 * <p>There may be more than one element. Each element will be
 * NULL-terminated ASCII or NULL-terminated UTF-8.
 * <p>The info associated with this Param is a pointer to a
 * VtUtf8StringList struct.
 * <p>When getting, pass in the address of a pointer as the getInfo,
 * the Get function will desposit a pointer to a VtUtf8StringList at
 * the address.
 */
extern VtSecureFileParam VtSecureFileParamContentDescriptors;

/** SetParam only
 * <p>Use this Param to indicate how long the data in content will be.
 * A SecureFile object that will write, must know, before actually
 * writing data, how long the content will be. If the content is not
 * supplied to the object all at once (call WriteFinal without any
 * previous calls to WriteUpdate), use this Param.
 * <p>Note that this supplies the length of the actual data (for
 * instance, the data that will be built into SecureFile content), not
 * the content itself.
 * <p>The info associated with this Param is a pointer to an unsigned
 * int giving the length of the data.
 */
extern VtSecureFileParam VtSecureFileParamDataLen;

/** GetParam only
 * <p>SecureFile content contains the time it was signed (the
 * signingTimeAttribute in the P7 SignedData). This Param gets that
 * time out of the message.
 * <p>Get this info only after completely reading the content.
 * <p>The info associated with this Param is a pointer to a VtTime
 * struct.
 * <p>When getting, pass in the address of a pointer as the getInfo,
 * the Get function will desposit a pointer to a VtTime struct at the
 * address.
 */
extern VtSecureFileParam VtSecureFileParamMessageTime;

/** SetParam only
 * <p>When the toolkit builds SecureFile content, it will need to
 * include new lines. If the application does not specify a new line
 * character, the toolkit will use "CR LF" (carriage return line feed),
 * also known as 0x0D 0x0A.
 * <p>See also the documentation concerning VtBase64Info.
 * <p>The data associated with VtSecureFileParamNewLineCharacter is a
 * pointer to an unsigned int. The value of that int must be either
 * <code>
 * <pre>
 *    VT_SECURE_FILE_NEW_LINE_LF
 *    VT_SECURE_FILE_NEW_LINE_CR_LF
 * </pre>
 * </code>
 */
extern VtSecureFileParam VtSecureFileParamNewLineCharacter;

/** Use this flag as the value of the unsigned int that is the info to
 * accompany VtSecureFileParamNewLineCharacter to indicate that the
 * toolkit should use the "LineFeed" new line character when writing
 * out a new line.
 */
#define VT_SECURE_FILE_NEW_LINE_LF     VT_BASE64_NEW_LINE_LF
/** Use this flag as the value of the unsigned int that is the info to
 * accompany VtSecureFileParamNewLineCharacter to indicate that the
 * toolkit should use the "CarriageReturn LineFeed" new line characters
 * when writing out a new line.
 */
#define VT_SECURE_FILE_NEW_LINE_CR_LF  VT_BASE64_NEW_LINE_CR_LF

/** This is the data struct to accompany VtSecureFileParamSenderInfo.
 * <p>The private key will be used to sign the file. It must be the
 * private key partner to the signerCert.
 * <p>The senderId is optional. If you have it available, pass it in,
 * if not, leave the field NULL.
 * <p>If you have an identity only, the toolkit will obtain the key and
 * cert (VtSecureFileParamSenderId). If you have the key and cert, pass
 * them in to save time. If you have the key, cert, and identity, you
 * can pass all three in. If the key and cert are not associated with
 * an identity (not an IBE-based sender), pass the key and cert only.
 */
typedef struct
{
  VtKeyObject privateKey;
  VtCertObject signerCert;
  VtIdentityObject senderId;
} VtSecureFileSenderInfo;

/** This is the data struct to accompany VtSecureFileParamRecipientIndex.
 */
typedef struct
{
  unsigned int index;
  VtPolicyCtx policyCtx;
  VtStorageCtx storageCtx;
  VtTransportCtx transportCtx;
} VtSecureFileRecipientIndexInfo;

/** Initialize the SecureFile object for writing. The object should
 * have been created with an Impl that creates content as opposed to
 * reading content. This function will make sure it has all the
 * information necessary to write the content (except for the actual
 * data itself) and collect any "missing" information using the policy,
 * storage, and transport contexts.
 * <p>The app had the opportunity to add info to the object during the
 * Create and Set functions, but some of that info might be
 * "incomplete" and the Init function will find any needed elements.
 * For example, in order to write the content, the object may need a
 * particular IBE key. If the app added only an Identity object, the
 * Init function will use the contexts to retrieve or download the
 * appropriate information to build the key.
 * <p>If no policy, storage, and/or transport ctx are given, the
 * function will use the contexts found in the libCtx (if any).
 *
 * @param secureFileObj The object created and set to write SecureFile
 * content.
 * @param policyCtx The policy ctx to use if IBE operations are
 * performed.
 * @param storageCtx The storage ctx to use if the function needs to
 * retrieve or store elements needed to build the content.
 * @param transportCtx The transport ctx to use if IBE operations are
 * performed.
 * @param random A random object the function will use if it needs
 * random bytes (for a symmetric key or IV, for example).
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtSecureFileWriteInit (
   VtSecureFileObject secureFileObj,
   VtPolicyCtx policyCtx,
   VtStorageCtx storageCtx,
   VtTransportCtx transportCtx,
   VtRandomObject random
);

/** Write out content. This function will write out what it can. It
 * does not assume that it has all the data of the content yet. If this
 * is the first call to Update, it will write out some header information
 * along with the processed data itself. That is, the output will be
 * bigger than the input.
 * <p>Note that the SecureFile object does NOT read and write files, it
 * only reads and writes data in the SecureFile format. That is, it
 * reads and writes data from and to memory, just like the SecureMail
 * object. It is the responsibility of the application to perform the
 * file reads and writes.
 * <p>If this is not the first call to Update, the input data will be
 * Base64 encoded, so the output will be bigger than the input.
 * <p>The function will deposit the output into the content buffer,
 * which is bufferSize bytes big. It will set the unsigned int at
 * contentLen to the number of bytes placed into the buffer. If the
 * buffer is not big enough, the function will return
 * VT_ERROR_BUFFER_TOO_SMALL and set contentLen to the required size.
 * <p>If the object was not set with a SecureFileImpl that can write,
 * the function will return an error.
 *
 * @param secureFileObj The SecureFile object built to write.
 * @param random A random object that can generate random numbers if
 * necessary.
 * @param inputData The actual data to build into a SecureFile content.
 * @param inputDataLen The length, in bytes, of the inputData.
 * @param content The output buffer, where the resulting SecureFile
 * content will be deposited.
 * @param bufferSize The size, in bytes, of the output buffer.
 * @param contentLen The address where the implementation will deposit
 * the length, the number of bytes placed into the output buffer.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtSecureFileWriteUpdate (
   VtSecureFileObject secureFileObj,
   VtRandomObject random,
   unsigned char *inputData,
   unsigned int inputDataLen,
   unsigned char *content,
   unsigned int bufferSize,
   unsigned int *contentLen
);

/** Finish writing out the content. This function will write out any
 * actual data left, then write out any trailing information (such as
 * footers). This function will match the total input to the specified
 * input from the call to SetSecureFileParam with the
 * VtSecureFileParamDataLen Param (if it was used). If there had been no
 * call to Update, this function may write out some header information
 * along with the processed data itself. That is, the output may be
 * bigger than the input.
 * <p>Note that the SecureFile object does NOT read and write files, it
 * only reads and writes data in the SecureFile format. That is, it
 * reads and writes data from and to memory, just like the SecureMail
 * object. It is the responsibility of the application to perform the
 * file reads and writes.
 * <p>The function will deposit the output into the content buffer,
 * which is bufferSize bytes big. It will set the unsigned int at
 * contentLen to the number of bytes placed into the buffer. If the
 * buffer is not big enough, the function will return
 * VT_ERROR_BUFFER_TOO_SMALL and set contentLen to the required size.
 * <p>If the object was not set with a SecureFileImpl that can write,
 * the function will return an error.
 *
 * @param secureFileObj The SecureFile object built to write.
 * @param random A random object that can generate random numbers if
 * necessary.
 * @param inputData The actual content data.
 * @param inputDataLen The length, in bytes, of the inputData.
 * @param content The output buffer, where the resulting SecureFile
 * content will be deposited.
 * @param bufferSize The size, in bytes, of the output buffer.
 * @param contentLen The address where the implementation will deposit
 * the length, the number of bytes placed into the output buffer.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtSecureFileWriteFinal (
   VtSecureFileObject secureFileObj,
   VtRandomObject random,
   unsigned char *inputData,
   unsigned int inputDataLen,
   unsigned char *content,
   unsigned int bufferSize,
   unsigned int *contentLen
);

/** Initialize the SecureFile object for reading. The object should
 * have been created with an Impl that reads content as opposed to
 * writing content. This function will make sure it has all the
 * information necessary to read the content (except for the actual
 * content itself).
 *
 * @param secureFileObj The object created and set to read SecureFile
 * content.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtSecureFileReadInit (
   VtSecureFileObject secureFileObj
);

/** Begin or continue reading SecureFile content. This function does
 * not demand that the entire SecureFile content be supplied, it will
 * read whatever is given.
 * <p>Note that the SecureFile object does NOT read and write files, it
 * only reads and writes data in the SecureFile format. That is, it
 * reads and writes data from and to memory, just like the SecureMail
 * object. It is the responsibility of the application to perform the
 * file reads and writes.
 * <p>The function will go to the address given by bytesRead and
 * deposit there an unsigned int indicating how many bytes of the
 * content the function read. It is possible that the function does not
 * read all the bytes given. For example, if given the first part of a
 * content, the function may read the leading info (headers, etc.) but
 * not the actual data itself. That can happen if the output buffer
 * given is not big enough to hold the data the function needs to
 * return. Hence, the function read up to the data, but stopped. The
 * function will return the bytes read and the BUFFER_TOO_SMALL error.
 * <p>An application should call Update, check the bytesRead, and if
 * the bytesRead is not the same as the input length, call Update
 * again, this time on the input data bytesRead further along. If the
 * bytesRead returned is the same as the contentLen, then the
 * application should call Update with the next block of input.
 * <p>If there is any data to return, the function will return it in
 * the given outputData buffer (bufferSize bytes big) and will place
 * into the unsigned int at outputDataLen the number of bytes placed
 * into the buffer.  If the buffer is not big enough, the routine will
 * return VT_ERROR_BUFFER_TOO_SMALL and set outputDataLen to the
 * required size.
 * <p>It is possible that there is no outputData. For instance, if the
 * Update is called with only the first part of a SecureFile content,
 * and the data given contains only header info, then the function will
 * read that data but have no content to return. Therefore, if you call
 * this function with a NULL output buffer, it is still possible the
 * return is 0 (no error) rather than the BUFFER_TOO_SMALL error.
 *
 * @param secureFileObj The SecureFile object built to read.
 * @param content The current part of the SecureFile content.
 * @param contentLen The length, in bytes, of the content data.
 * @param bytesRead The address where the routine will deposit the
 * number of bytes of the content actually read. If 0, call the Update
 * function again with the same data. If contentLen, the function read
 * all the bytes, the next call should pass in the next bytes in the
 * content.
 * @param outputData The output buffer, where the resulting content
 * data will be deposited.
 * @param bufferSize The size, in bytes, of the output buffer.
 * @param outputDataLen The address where the function will deposit
 * the length, the number of bytes placed into the output buffer.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtSecureFileReadUpdate (
   VtSecureFileObject secureFileObj,
   unsigned char *content,
   unsigned int contentLen,
   unsigned int *bytesRead,
   unsigned char *outputData,
   unsigned int bufferSize,
   unsigned int *outputDataLen
);

/** Finish reading SecureFile content. This function will check to
 * make sure the entire SecureFile content was supplied, either during
 * all the calls to Update and this call to Final, or all at once if
 * this is the only call to Read.
 * <p>Note that the SecureFile object does NOT read and write files, it
 * only reads and writes data in the SecureFile format. That is, it
 * reads and writes data from and to memory, just like the SecureMail
 * object. It is the responsibility of the application to perform the
 * file reads and writes.
 * <p>The function will go to the address given by bytesRead and
 * deposit there an unsigned int indicating how many bytes of the
 * buffer the function read. It is possible that the function does not
 * read all the bytes given. For example, it may read up to the part
 * where the function needs the app to choose a recipient before it can
 * read further. Or the output buffer is not big enough.
 * <p>An application should call Final, check the bytesRead, and if
 * the bytesRead is not the same as the input length, call Final again,
 * this time on the input data bytesRead further along. If the
 * bytesRead returned is the same as the contentLen, then the entire
 * content has been read.
 * <p>If there is any data to return, the function will return it in
 * the given outputData buffer (bufferSize bytes big) and will place
 * into the unsigned int at outputDataLen the number of bytes placed
 * into the buffer.  If the buffer is not big enough, the routine will
 * return VT_ERROR_BUFFER_TOO_SMALL and set outputDataLen to the
 * required size.
 * <p>It is possible that there is no outputData. For instance, if
 * previous calls to Update collected all the actual data, this call to
 * Final will be reading only "trailing" information in the content.
 * <p>If the input does not complete a content, this function will
 * return an error.
 * <p>Note that this function does not verify signatures, it is only
 * able to read the content. To verify signatures, after a successful
 * call to ReadFinal, call a SecureFileVerify function.
 *
 * @param secureFileObj The SecureFile object built to read.
 * @param content The current part of the SecureFile content.
 * @param contentLen The length, in bytes, of the content data.
 * @param bytesRead The address where the routine will deposit the
 * number of bytes of the content actually read. If 0, call the Final
 * function again with the same data. If contentLen, the function read
 * all the bytes, the next call should pass in the next bytes in the
 * content.
 * @param outputData The output buffer, where the resulting content
 * data will be deposited.
 * @param bufferSize The size, in bytes, of the output buffer.
 * @param outputDataLen The address where the function will deposit
 * the length, the number of bytes placed into the output buffer.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtSecureFileReadFinal (
   VtSecureFileObject secureFileObj,
   unsigned char *content,
   unsigned int contentLen,
   unsigned int *bytesRead,
   unsigned char *outputData,
   unsigned int bufferSize,
   unsigned int *outputDataLen
);

/** Verify all signatures in SecureFile content. If there is more than
 * one signer, this function will try to verify all signatures and all
 * signer certs. If one element does not verify, this function sets the
 * verifyResult to did not verify.
 * <p>Before calling this function the application must finish reading
 * the content. That is, do not call this function until after a
 * successful call to ReadFinal.
 * <p>The caller passes in policy, storage and transport contexts the
 * function will use to help it find certificates it needs to chain a
 * leaf cert. The function may download district parameters to obtain
 * certs.
 * <p>The caller also passes in a certVerifyCtx, which this function
 * will use to verify any untrusted certs it encounters. The caller
 * must also pass in the appropriate associated info (verifyCtxInfo)
 * for the particular certVerifyCtx. That is the info the specific ctx
 * needs to verify a cert. This associated info will be applied to each
 * leaf cert the function verifies.
 *
 * @param secureFileObj The object built to read and used to read an
 * entire SecureFile content.
 * @param policyCtx A policyCtx to use if necessary.
 * @param storageCtx A storageCtx to use to help find any certs needed
 * to verify signatures or other certs.
 * @param transportCtx A transportCtx to use if necessary.
 * @param certVerifyCtx The certVerifyCtx the function will use when
 * verifying untrusted certs in a chain.
 * @param verifyCtxInfo The info the certVerifyCtx needs to help it
 * verify. This info will be applied to leaf certs.
 * @param vfyFailList Currently unused, reserved for future
 * implementations.
 * @param verifyResult The address where this function will deposit the
 * result of the verify, one for verify, zero for not verified.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtSecureFileVerify (
   VtSecureFileObject secureFileObj,
   VtPolicyCtx policyCtx,
   VtStorageCtx storageCtx,
   VtTransportCtx transportCtx,
   VtCertVerifyCtx certVerifyCtx,
   Pointer verifyCtxInfo,
   VtVerifyFailureList vfyFailList,
   unsigned int *verifyResult
);

/*@}*/

/*=========================================================*/
/*                                                         */
/* ZDM Object (ZDM = Zero Download Messenger)              */
/*                                                         */
/*=========================================================*/

/**
 * \defgroup ZDMGroup ZDM Object
 */

/*@{*/

/** The ZDM object.
 * <p>Note that the object is a pointer type.
 */
typedef struct VtZDMObjectDef *VtZDMObject;

/** The function VtCreateZDMObject builds an object that builds or
 * reads ZDM content using a VtZDMImpl. This typedef defines what a
 * VtZDMImpl is. Although a VtZDMImpl is a function pointer, an
 * application should never call one directly, only pass it as an
 * argument to VtCreateZDMObject.
 */
typedef int VT_CALLING_CONV (VtZDMImpl) (
   VtZDMObject *, Pointer, unsigned int);

/** The functions VtSetZDMParam and VtGetZDMParam add or get
 * information to or from a ZDM object. The information to add or get
 * is defined by a VtZDMParam. This typedef defines what a VtZDMParam
 * is. Although a VtZDMParam is a function pointer, an application
 * should never call one directly, only pass it as an argument to
 * VtSetZDMParam or VtGetZDMParam.
 */
typedef int VT_CALLING_CONV (VtZDMParam) (
   VtZDMObject, Pointer, unsigned int);

/** Create a new ZDM object. This allocates space for an "empty"
 * object, then loads the given ZDMImpl to make it an "active" object.
 * <p>The VtZDMImpl defines what the object will be able to process
 * (generally reading and writing will be different Impls). The
 * include file vibe.h defines the supported ZDMImpls. Look through
 * the include file to see which ZDMImpl to use for your application.
 * All supported ZDMImpls will be defined as in the following example.
 * <code>
 * <pre>
 *   extern VtZDMImpl VtZDMImplWrite;
 * </pre>
 * </code>
 * <p>Associated with each ZDMImpl is specific info. The documentation
 * for each ZDMImpl will describe the associated info it needs. That
 * data could be another object, it could be data in a particular
 * struct, it might be a NULL pointer. Check the documentation for each
 * ZDMImpl for a description of the data and its required format.
 * <p>To use this function decide which ZDMImpl you want to use, then
 * determine what information that ZDMImpl needs and in which format it
 * is presented. Collect the data in the appropriate format then call
 * this function passing in the desired ZDMImpl and the required info.
 * The associated info must be cast to Pointer.
 * <p>For example, if the required info is a VtItem struct, declare a
 * variable to be of type VtItem, fill in the fields, then pass the
 * address of that struct cast to Pointer.
 * <p>The input zdmObj is a pointer to an object. It should point to a
 * NULL VtZDMObject. This function will go to the address given and
 * deposit a created object.
 * <p>If you call Create, you must call Destroy after you are done with
 * the object but before the program exits.
 * <p>Example:
 * <code>
 * <pre>
 *    int status;
 *    VtZDMObject zdmObj = (VtZDMObject)0;
 *
 *    do {
 *          . . .
 *      status = VtCreateZDMObject (
 *        libCtx, VtZDMImplWrite, (Pointer)0, &zdmObj);
 *      if (status != 0)
 *        break;
 *          . . .
 *    } while (0);
 *
 *    VtDestroyZDMObject (&zdmObj);
 * </pre>
 * </code>
 * @param libCtx The library context.
 * @param zdmImpl The implementation the object will use.
 * @param associatedInfo The info needed by the ZDMImpl.
 * @param zdmObj A pointer to where the routine will deposit the
 * created object.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtCreateZDMObject (
   VtLibCtx libCtx,
   VtZDMImpl zdmImpl,
   Pointer associatedInfo,
   VtZDMObject *zdmObj
);

/* These are the VtZDMImpls supported by the toolkit. Each VtZDMImple
 * is used in conjunction with special info for the function. If there
 * is no special info, the accompaniment is a NULL pointer.
 */

/** Use this Impl to build a ZDM object that will be able to create ZDM
 * content.
 * <p>The data associated with VtZDMImplWrite is a NULL pointer:
 * (Pointer)0.
 */
extern VtZDMImpl VtZDMImplWrite;

/** Use this Impl to build a ZDM object that will be able to create ZDM
 * content given an existing SecureMail message.
 * <p>An app might want to build both a SecureMail and ZDM message.
 * However, to save computation time, it is possible to build a ZDM
 * message if the SecureMail message is already generated (or partially
 * generated using WriteUpdate).
 * <p>The data associated with VtZDMImplWriteFromSecureMail is a
 * VtSecureMailObject that has been through at least
 * VtSecureMailWriteInit.
 * <p>Note that in order to create a ZDM object that writes ZDM from
 * SecureMail, you must first Create, Set, and Init a SecureMail object.
 */
extern VtZDMImpl VtZDMImplWriteFromSecureMail;

/** Use this Impl to build a ZDM object that will be able to create ZDM
 * content given existing SecureFile content.
 * <p>An app might want to build both a SecureFile and ZDM message.
 * However, to save computation time, it is possible to build a ZDM
 * message if the SecureFile content is already generated (or partially
 * generated using WriteUpdate).
 * <p>The data associated with VtZDMImplWriteFromSecureFile is a
 * VtSecureFileObject that has been through at least
 * VtSecureFileWriteInit.
 * <p>Note that in order to create a ZDM object that writes ZDM from
 * SecureFile, you must first Create, Set, and Init a SecureFile object.
 */
extern VtZDMImpl VtZDMImplWriteFromSecureFile;

/** Use this Impl to build a ZDM object that will be able to read ZDM
 * content.
 * <p>The data associated with VtZDMImplRead is a pointer to a
 * VtReadZDMInfo struct containing all the algorithms (asymmetric and
 * symmetric algorithms represented as DERCoders) and identity schemas
 * (represented as SchemaDecodes) the app is willing to support. The
 * mpCtx is for performing IBE private key operations with the Identity
 * eventually chosen to decrypt the content.
 * <p>If no DerCoders, SchemaDecodes, or mpCtx are supplied, the Impl
 * will try to find these elements in the libCtx.
 * <p>If the accompanying info is a NULL pointer, this Impl will try to
 * find all the info it needs from the libCtx.
 */
extern VtZDMImpl VtZDMImplRead;

/** Use this Impl to build a ZDM object that will be able to create ZDMv2
 * content.
 * <p>The data associated with VtZDMImplWrite is a NULL pointer:
 * (Pointer)0.
 */
extern VtZDMImpl VtZDM2ImplWrite;

/** Use this Impl to build a ZDM object that will be able to read ZDMv2
 * content.
 * <p>The data associated with VtZDMImplRead is a pointer to a
 * VtReadZDMInfo struct containing all the algorithms (asymmetric and
 * symmetric algorithms represented as DERCoders) and identity schemas
 * (represented as SchemaDecodes) the app is willing to support. The
 * mpCtx is for performing IBE private key operations with the Identity
 * eventually chosen to decrypt the content.
 * <p>If no DerCoders, SchemaDecodes, or mpCtx are supplied, the Impl
 * will try to find these elements in the libCtx.
 * <p>If the accompanying info is a NULL pointer, this Impl will try to
 * find all the info it needs from the libCtx.
 */
extern VtZDMImpl VtZDM2ImplRead;

/** This is the data struct to accompany VtZDMImplRead.
 */
typedef struct
{
  VtDerCoder **derCoders;
  unsigned int derCoderCount;
  VtIdentitySchemaDecode **decoders;
  unsigned int decoderCount;
  VtMpIntCtx mpCtx;
} VtReadZDMInfo;

/** Destroy a ZDM Object. This frees up any memory allocated during the
 * object's creation and use.
 * <p>If you call Create, you must call Destroy after you are done with
 * the object but before the program exits.
 * <p>Example:
 * <code>
 * <pre>
 *    int status;
 *    VtZDMObject zdmObj = (VtZDMObject)0;
 *
 *    do {
 *          . . .
 *      status = VtCreateZDMObject (
 *        libCtx, VtZDMImplWrite, (Pointer)0, &zdmObj);
 *      if (status != 0)
 *        break;
 *          . . .
 *    } while (0);
 *
 *    VtDestroyZDMObject (&zdmObj);
 * </pre>
 * </code>
 * @param zdmObj A pointer to where the routine will find the object to
 * destroy.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtDestroyZDMObject (
   VtZDMObject *zdmObj
);

/** Set the ZDM object with the information given.
 * <p>The VtZDMParam defines what information the object will be set
 * with.
 * <p>The include file vibe.h defines the supported ZDMParams. Look
 * through the include file to see which ZDMParam to use for your
 * application. All supported ZDMParams will be defined as in the
 * following example.
 * <code>
 * <pre>
 *   extern VtZDMParam VtZDMParamDataLen;
 * </pre>
 * </code>
 * <p>Associated with each ZDMParam is specific info. The documentation
 * for each ZDMParam will describe the associated info it needs. That
 * data could be another object, it could be data in a particular
 * struct, it might be a NULL pointer. Check the documentation for each
 * ZDMParam for a description of the data and its required format.
 * <p>To use this function decide which ZDMParam you want to use, then
 * determine what information that ZDMParam needs and in which format
 * it is presented. Collect the data in the appropriate format then
 * call this function passing in the desired ZDMParam and and the
 * required info. The associated info must be cast to Pointer.
 * <p>For example, if the required info is a particular struct, declare
 * a variable to be of that type struct, fill in the fields, then pass
 * the address of that struct cast to Pointer.
 *
 * @param zdmObj The object to set.
 * @param zdmParam What the object is being set to.
 * @param associatedInfo The info needed by the ZDMParam.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtSetZDMParam (
   VtZDMObject zdmObj,
   VtZDMParam zdmParam,
   Pointer associatedInfo
);

/** Get the specified type of information out of a ZDM object.
 * <p>This function will go to the address given by getInfo and
 * deposit a pointer. That pointer is the address of the data. The
 * memory to hold the information belongs to the object, do not free it.
 * <p>The VtZDMParam will specify what kind of information will be
 * returned, the getInfo is where the function will deposit the info.
 * <p>The include file vibe.h defines the supported ZDMParams. Look
 * through the include file to see which ZDMParam to use for your
 * application.
 * <p>See also VtSetZDMParam.
 * <p>To use this function decide which ZDMParam you want to use, then
 * determine what information that ZDMParam will return and in which
 * format it is presented. Declare a variable to be a pointer to the
 * appropriate type, then call this function passing in the desired
 * ZDMParam and the address of the variable. Cast the address of the
 * pointer to the appropriate type: (Pointer *).
 * <p>If the associated info is already a pointer type (such as an
 * object), declare a variable to be of the appropriate type and pass
 * the address of that variable cast to (Pointer *).
 * <p>For example, if the returned info is a VtIdentityList, declare a
 * variable to be of type VtIdentityList, pass the address of that
 * variable (&varName) cast to (Pointer *).
 * <p>Example:
 * <code>
 * <pre>
 *    VtIdentityList getRecipList;
 *
 *    do {
 *          . . .
 *      status = VtGetZDMParam (
 *        readObj, VtZDMParamRecipientList, (Pointer *)&getRecipList);
 *      if (status != 0)
 *        break;
 *          . . .
 *    } while (0);
 * </pre>
 * </code>
 * @param zdmObj The object to query.
 * @param zdmParam Defines what info to get.
 * @param getInfo The address of a pointer. The function will go to
 * that address and deposit a pointer to the info requested.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtGetZDMParam (
   VtZDMObject zdmObj,
   VtZDMParam zdmParam,
   Pointer *getInfo
);

/* These are the VtZDMParams supported by the toolkit. Each VtZDMParam
 * is used in conjunction with special info for the function.
 */

/** SetParam only.
 * <p>Use this Param to indicate to a ZDM object built to write that
 * the message created will be, either the mail message itself or an
 * attachment.
 * <p>The data associated with VtZDMParamMessageType is a pointer to an
 * unsigned int set to either VT_ZDM_MAIL or VT_ZDM_ATTACHMENT.
 */
extern VtZDMParam VtZDMParamMessageType;

/** For use with VtZDMParamMessageType, set a flag to this value to
 * indicate the data to build into ZDM format is a mail message.
 */
#define VT_ZDM_MAIL        1
/** For use with VtZDMParamMessageType, set a flag to this value to
 * indicate the data to build into ZDM format is an attachment to a
 * mail message.
 */
#define VT_ZDM_ATTACHMENT  2

/** SetParam only.
 * <p>Use this Param on a ZDM object built to write to specify what the
 * Subject of the mail is. A ZDM message includes a Subject line, so an
 * application has to somwhow let the object know what the Subject is.
 * <p>If you do not set a ZDM object with this Param, the object will
 * leave the Subject line blank.
 * <p>The data associated with VtZDMParamSubjectLine is an unsigned char
 * array, a UTF-8 NULL-terminated string.
 */
extern VtZDMParam VtZDMParamSubjectLine;

/** SetParam only.
 * <p>Use this Param to set a ZDM object with a sender represented as
 * an identity. With an object built with this Param, the toolkit will
 * obtain the signing key and cert during the call to VtWriteInit,
 * either from storage or from the key server.
 * <p>If you set a ZDMObject using this Param, do not set it with
 * another identity, either using this ZDMParam or VtZDMParamSenderInfo.
 * That is, a ZDM message has one and only one sender.
 * <p>The info associated with this Param is an identity object.
 */
extern VtZDMParam VtZDMParamSenderId;

/** SetParam only.
 * <p>Use this Param to set a ZDM object with sender info it will need
 * in order to send a message. This will include a private key with
 * which to sign and optionally the cert associated with that private
 * key.
 * <p>The accompanying data includes an identity object. That info is
 * NOT optional. If you have an identity only, the toolkit will obtain
 * the key and cert (VtZDMParamSenderId). If you have the key and cert,
 * pass them in to save time, but you must also have an identity object
 * as well.
 * <p>NOTE!!! If you do not pass in the senderId, the object will not
 * be able to build the ZDM message.
 * <p>If you set a ZDMObject using this Param, do not set it with
 * another sender identity, either using this ZDMParam or
 * VtZDMParamSenderId. That is, a ZDM message has one and only one
 * sender.
 * <p>The private key object passed in at this time will not be cloned.
 * That is, the ZDMObject set with this Param will copy a reference to
 * that key. When the ZDMObject needs to sign (WriteFinal), it will use
 * that key object. If you use this Param, make sure the key object
 * stays unchanged until after WriteFinal.
 * <p>The info associated with this Param is a pointer to a
 * VtZDMSenderInfo struct.
 */
extern VtZDMParam VtZDMParamSenderInfo;

/** SetParam and GetParam.
 * <p>Use this param to add a list of recipients to an object built to
 * write a ZDM message.
 * <p>Or use this param to get the list of recipients of a ZDM message
 * being read.
 * <p>When an object built to write a ZDM message creates the digital
 * envelope, it will use each of the identities in the IdentityList to
 * build a RecipientInfo (which includes the session key encrypted
 * using the recipient's public key, which in this case is the
 * identity).
 * <p>When an object built to read a ZDM message reads all the
 * RecipientInfos, it will build a VtIdentityList containing the
 * identities of all the recipients. The app can then get that list by
 * using this ZDMParam, and then examine each of the identities to
 * choose which one the object should use to open the envelope
 * (indicate which identity to use by calling VtSetZDMParam with
 * VtZDMParamRecipientIndex).
 * <p>The info associated with this Param is a VtIdentityList object.
 * <p>When getting, pass in the address of a VtIdentityList variable as
 * the getInfo, the Get function will desposit a VtIdentityList at the
 * address.
 */
extern VtZDMParam VtZDMParamRecipientList;

/** SetParam and GetParam.
 * <p>Use this param to add recipients to an object built to write a
 * ZDM message, the recipients represented as an already
 * existing RecipientInfoList (as opposed to an IdentityList).
 * <p>Or use this param to get the recipients of a ZDM message being
 * read, represented as a RecipientInfoList.
 * <p>When an object built to write ZDM creates the digital envelope,
 * it will use the encoded RecipientInfos from the associated info,
 * instead of building it. It will also use the symmetric key and
 * symmetric algorithm specified in the associated info.
 * <p>When writing a ZDM message, using this Param is equivalent to
 * using VtZDMParamRecipientList along with either VtZDMParamEnv3DESCBC
 * or VtZDMParamEnvAES128CBC.
 * <p>When an object built to read ZDM is done reading the entire
 * message, an app can use this Param to get the RecipientInfoList
 * representation of the recipients. The app can then get information
 * out of the RecipientInfoList, such as the symmetric key and
 * algorithm.
 * <p>Note! When reading, the CHOOSE_RECIPIENT error means you get an
 * IdentityList out of the ZDM object and you pick an identity to use
 * to decrypt. You cannot get the RecipientInfoList at that time, you
 * must wait until reading the entire message.
 * <p>After Getting the RecipientInfoList out of a ZDM object, you can
 * call VtEncodeRecipientInfoList. If you want the symmetric key
 * protected in the RecipientInfoBlock, Set it with a secretValue
 * first. NOTE!!! This is a departure from normal toolkit rules.
 * Normally we say do not alter objects returned from a Get call,
 * however, in this case, the object returned by the Get will have no
 * secretValue installed, so if you want to use one, you must add one.
 * <p>The info associated with this Param is a VtRecipientInfoList
 * object.
 * <p>NOTE!!! If setting a ZDM object with a RecipientInfoList, each
 * RecipientInfo must be encoded. That is, you must have called
 * VtEncodeRecipientInfo on each RecipientInfoObject inside the
 * RecipientInfoList, or you must have called
 * VtEncodeRecipientInfoList on the RecipientInfoList.
 * <p>NOTE!!! If setting a ZDM object with a RecipientInfoList, the
 * RecipientInfoList MUST be set so that it can produce the
 * RecipientInfoBlock. (See the documentation on
 * VtEncodeRecipientInfoList for info on obtaining RecipientInfoBlock
 * versus RecipientInfos.)
 * <p>When getting, pass in the address of a VtRecipientInfoList
 * variable as the getInfo, the Get function will desposit a
 * VtRecipientInfoList at the address.
 */
extern VtZDMParam VtZDMParamRecipientInfoList;

/** SetParam only.
 * <p>Use this ZDMParam with a ZDM Object built to write. It indicates
 * the object should use Triple DES in CBC mode to encrypt the bulk
 * data.
 * <p>The ZDM object will generate the IV and Triple DES key. It will
 * generate the key data using the random object supplied by the
 * application during the call to VtZDMWriteInit.
 * <p>The data associated with VtZDMParam3DESCBC is a NULL pointer:
 * (Pointer)0.
 */
extern VtZDMParam VtZDMParam3DESCBC;

/** SetParam only.
 * <p>Use this ZDMParam with a ZDM Object built to write. It indicates
 * the object should use AES-128 in CBC mode to encrypt the bulk data.
 * <p>In creating the ZDM message, the object will indicate, using an
 * algorithm identitifer, that the symmetric algorithm is AES-128 in
 * CBC mode. The OID is defined by NIST as 2.16.840.1.101.3.4.1.2. The
 * OID does not specify padding. In fact, in an email message, a NIST
 * representative explicitly stated that the OID does not specify
 * padding. However, the representative also suggested following the
 * instructions specified in Appendix A of NIST Special Publication
 * 800-38A. This appendix recommends applications pad data when using
 * AES in CBC mode, then gives one possible padding scheme which is not
 * the technique outlined in PKCS #5 (the most commonly used block
 * cipher padding scheme in use today). It further states, "For the
 * above padding method, the padding bits can be removed unambiguously,
 * provided the receiver can determine that the message is indeed
 * padded. One way to ensure that the receiver does not mistakenly
 * remove bits from an unpadded message is to require the sender to pad
 * every message, including messages in which the final block (segment)
 * is already complete. For such messages, an entire block (segment) of
 * padding is appended."
 * <p>A "standard" is supposed to specify how to do something in a
 * standard way, so that two entities can know what something means
 * without having to communicate anything in advance. But here is a
 * standard (along with the people who wrote and disseminate it) that
 * explicitly states that the sender and receiver must communicate in
 * advance to resolve an intentional ambiguity in the standard.
 * <p>Fortunately, PKCS #7 specifies that all block ciphers in modes
 * that require complete blocks (such as CBC) shall pad (even if the
 * input length is a multiple of the block size) and shall pad
 * following the technique specified in PKCS #5. A ZDM message uses
 * PKCS #7 to construct the underlying EnvelopedData.
 * <p>The ZDM object will generate the IV and AES key. It will generate
 * the key data using the random object supplied by the application
 * during the call to VtZDMWriteInit.
 * <p>The data associated with VtZDMParamAES128CBC is a NULL pointer:
 * (Pointer)0.
 */
extern VtZDMParam VtZDMParamAES128CBC;

/** GetParam only
 * <p>A ZDM message contains the time it was signed (the
 * signingTimeAttribute in the P7 SignedData). This Param gets that
 * time out of the message.
 * <p>Get this info only after completely reading the message.
 * <p>The info associated with this Param is a pointer to a VtTime
 * struct.
 * <p>When getting, pass in the address of a pointer as the getInfo,
 * the Get function will desposit a pointer to a VtTime struct at the
 * address.
 */
extern VtZDMParam VtZDMParamMessageTime;

/** SetParam only
 * <p>Use this Param to indicate how long the data in a message will be.
 * A ZDM object that will write, must know, before actually writing
 * data, how long the message will be. If the message is not supplied
 * to the object all at once (call WriteFinal without any previous calls
 * to WriteUpdate), use this Param.
 * <p>Note that this supplies the length of the actual data (for
 * instance, the data that will be built into a ZDM message), not the
 * message itself.
 * <p>The info associated with this Param is a pointer to an unsigned
 * int giving the length of the data.
 */
extern VtZDMParam VtZDMParamDataLen;

/** SetParam only
 * <p>When the toolkit builds ZDM messages, it will need to include new
 * lines. If the application does not specify a new line character, the
 * toolkit will use "CR LF" (carriage return line feed), also known as
 * 0x0D 0x0A.
 * <p>See also the documentation concerning VtBase64Info.
 * <p>The data associated with VtZDMParamNewLineCharacter is a pointer
 * to an unsigned int. The value of that int must be either
 * <code>
 * <pre>
 *    VT_ZDM_NEW_LINE_LF
 *    VT_ZDM_NEW_LINE_CR_LF
 * </pre>
 * </code>
 */
extern VtZDMParam VtZDMParamNewLineCharacter;

/** Use this flag as the value of the unsigned int that is the info to
 * accompany VtZDMParamNewLineCharacter to indicate that the toolkit
 * should use the "LineFeed" new line character when writing out a new
 * line.
 */
#define VT_ZDM_NEW_LINE_LF     VT_BASE64_NEW_LINE_LF
/** Use this flag as the value of the unsigned int that is the info to
 * accompany VtZDMParamNewLineCharacter to indicate that the toolkit
 * should use the "CarriageReturn LineFeed" new line characters when
 * writing out a new line.
 */
#define VT_ZDM_NEW_LINE_CR_LF  VT_BASE64_NEW_LINE_CR_LF

/** SetParam and GetParam
 * <p>Set an object to write with a content type. Or Get the content
 * type from an object that has read a ZDM message.
 * <p>A ZDM mail message is actually the data you pass with some other
 * info prepended. This prepended info looks something like this.
 * <code>
 * <pre>
 *   content-type: text/plain
 *   content-length: 1486
 * </pre>
 * </code>
 * The content-type describes what the following data is. It might be
 * Multipart/alternative, text/html, text/plain, or other values. The
 * content-length is the length of the data you are securing.
 * When writing a ZDM message, the ZDM object must specify the content
 * type for the message body in its encrypted headers to be compatible
 * with existing Voltage products, so if this param is not called
 * explicitly for the message body the ZDM object will default to use
 * text/plain. If you're using some other content type, then you should
 * call this param with that content type. Setting the content type for
 * the attachments (i.e. for ZDMv2 objects) is optional.
 * <p>The toolkit will not check the value you pass in, it will simply
 * write out to the message the same thing you pass in.
 * <p>When reading a ZDM message, you can get this param after the
 * object has read this first part of the data. Remember, when getting
 * the content-type out, the memory belongs to the object, do not alter
 * or free it.
 * <p>The data associated with VtZDMParamContentType is an unsigned char
 * array, a UTF-8 NULL-terminated string.
 * <p>When getting, pass in the address of a pointer as the getInfo,
 * the Get function will desposit a pointer to a buffer at the address.
 */
extern VtZDMParam VtZDMParamContentType;

/** SetParam and GetParam
 * <p>When writing a ZDM attachment, use this Param to indicate what the
 * file name is of the file contents being encrypted. This Param can be
 * used only when building ZDM attachments, see VtZDMParamMessageType.
 * <p>When reading, after contents have been read, this will allow an
 * app to extract the file name out of the object.
 * <p>The info associated with this Param is a UTF-8 string, a
 * NULL-terminated unsigned char array (unsigned char *).
 * <p>When getting, pass in the address of a pointer as the getInfo,
 * the Get function will desposit a pointer to a buffer at the address.
 */
extern VtZDMParam VtZDMParamFileName;

/** SetParam only.
 * <p>Use this ZDMParam to let a ZDM Object built to read know which
 * identity to use to decrypt the message.
 * <p>When the return from VtZDMReadUpdate is VT_ERROR_CHOOSE_RECIPIENT,
 * the app should get the recipient list out of of the ZDM object
 * (using VtZDMParamRecipientList), examine the identities of the
 * recipients, and choose which one should be used to decrypt the
 * message. The identity chosen will have an index associated with it,
 * it is the index into the IdentityList of all the recipients. This
 * Param tells the ZDM object which identity in the IdentityList the
 * object should use.
 * <p>Once the ZDM object knows which identity to use, it will obtain
 * the IBE private key. Therefore, the app also passes in policy,
 * storage, and transport contexts for the SecureMail object to use to
 * obtain the private key. If no contexts are passed in, the ZDM object
 * will look in the libCtx for those entities.
 * <p>Note that this will only obtain an IBE private key with which to
 * decrypt a session key. If the session key was encrypted with any
 * other algorithm, this Param will not be able to load the appropriate
 * private key needed.
 * <p>The data associated with VtZDMParamRecipientIndex is a pointer to
 * a VtZDMRecipientIndexInfo struct.
 */
extern VtZDMParam VtZDMParamRecipientIndex;

/** GetParam only.
 * <p>Use this param to get the list of signers out of a ZDM message
 * being read.
 * <p>When an object built to read ZDM reads all the SignerInfos, it
 * will build a VtIdentityList containing the identities of all the
 * signers. The app can then get that list by using this Param.
 * <p>The info associated with this Param is a VtIdentityList object.
 * <p>When getting, pass in the address of a VtIdentityList variable as
 * the getInfo, the Get function will desposit a VtIdentityList at the
 * address.
 */
extern VtZDMParam VtZDMParamSignerList;

/** GetParam only - VtZDMImplRead only (i.e. V1 ZDM only)
 * <p>Before signing and encrypting the message data, ZDM adds some
 * content info. This will be elements such as
 * <code>
 * <pre>
 *    content-type: text/plain
 *    content-length: 4096
 * </pre>
 * </code>
 * <p>After reading a ZDM message, if you want to know what content
 * info the message contained, call VtGetZDMParam with this Param.
 * <p>There may be more than one element (for example, content-type is
 * one, content-length is another). Each element will be
 * NULL-terminated ASCII or NULL-terminated UTF-8.
 * <p>The info associated with this Param is a pointer to a
 * VtUtf8StringList struct.
 * <p>When getting, pass in the address of a pointer as the getInfo,
 * the Get function will desposit a pointer to a VtUtf8StringList at
 * the address.
 * <p>The V2 ZDM format stores its headers in a different format,
 * so this param can only be used with the V1 impl.
 */
extern VtZDMParam VtZDMParamContentDescriptors;

/** The following params are for the ZDMv2 implementation. They should
 * not be used with a ZDM object created with a v1 impl.
 */

/** SetParam and GetParam.
 * <p>Use this param to get/set the list of primary email recipients for
 * a ZDM2 message. This is the list of recipients that appear in the
 * "To:" list of the email. This list will be encoded in the ZDM headers
 * included in the main message body of the ZDM.
 * <p>The info associated with this Param is a VtZDMEmailRecipientList
 * object.
 */
extern VtZDMParam VtZDMParamPrimaryEmailRecipientList;

/** SetParam and GetParam.
 * <p>Use this param to get/set the list of CC email recipients for
 * a ZDM2 message. This is the list of recipients that appear in the
 * "CC:" list of the email. This list will be encoded in ZDM headers
 * included in the main message body of the ZDM.
 * <p>The info associated with this Param is a VtZDMEmailRecipientList
 * struct (defined below).
 */
extern VtZDMParam VtZDMParamCCEmailRecipientList;

/** SetParam only - VtZDM2ImplRead and VtZDM2ImplWrite
 * <p> Use VtZDMParamBufferType to specify what type of buffer the
 * ZDM object should use for any internal buffering it requires.
 * The bufferType field of the VtBufferTypeInfo struct should be
 * set to either VT_BUFFER_TYPE_MEMORY or VT_BUFFER_TYPE_FILE.
 * The VT_BUFFER_TYPE_STREAM type is not allowed, because the
 * ZDM object may need to allocate more than one temporary buffers,
 * so it can't use a single stream object.
 * <p>The associated info is a pointer to a VtBufferTypeInfo
 * struct. If the associated info is NULL, then memory-based
 * buffering will be used. This is also the default method if this
 * parameter is never set.
 */
extern VtZDMParam VtZDMParamBufferType;

/** SetParam only - read impl
 * <p>Use VtZDMParamInputStream to specify a stream
 * to use for the input data for the ZDM. This can be used
 * instead of explicitly streaming in all the data using
 * VtZDMReadUpdate. This can be useful if the archive
 * is coming from a file, so that the caller can just set up a
 * stream directly from the file rather than having the archive
 * read the contents of the file into another stream.
 * <p>The associated info is the VtStreamObject to be used for
 * the input stream.
 */
extern VtZDMParam VtZDMParamInputStream;

/** GetParam only - VtZDM2ImplRead
 * <p>This param returns the number of attachment files contained in the
 * ZDM. The caller can then iterate over the attachments to get the
 * attributes and data for each one.
 * <p>The associated data for this param is an unsigned int (i.e. the
 * argument is a unsigned int* cast to a Pointer*)
 */
extern VtZDMParam VtZDMParamAttachmentCount;

/** SetParam only - both VtZDM2ReadImpl and VtZDM2WriteImpl.
 * <p>Use this param to specify the current entry in the ZDM object.
 * Calls to other parameters to get/set the content type, characters set
 * or other attributes as well as ReadUpdate and WriteUpdate calls
 * refer to the current entry specified with this param. The current
 * entry is either the main message body or one of the attachments.
 * By default the current entry is the main message body.
 * <p>The associated info is a pointer to a VtZDMCurrentEntryInfo
 * struct (defined below).
 */
extern VtZDMParam VtZDMParamCurrentEntry;

/** SetParam and GetParam.
 * <p>Set this param on a ZDM writer object to specify the
 * "Reply To" header of the mail. This can be used to specify to the
 * reader of the ZDM that any reply should go to the ReplyTo address
 * instead of back to the sender's address.
 * <p>Get this param from a ZDM reader to retrieve the ReplyTo address.
 * <p>If you do not set a ZDM object with this Param, the ReplyTo header
 * will not be specified and any reply will go back to the sender.
 * <p>The data associated with VtZDMParamReplyTo is an unsigned char
 * array, a UTF-8 NULL-terminated string.
 */
extern VtZDMParam VtZDMParamReplyTo;

/** SetParam and GetParam
 * <p>Specify the character set of the current entry (i.e either the
 * main message body or attachment specified with VtZDMParamCurrentEntry).
 * This character set value should be the encoding used for the data as it's
 * input to the ZDM object (perhaps different than original encoding).
 * The character set attribute should be one of the standard charset
 * strings used with MIME content types. The character set
 * values for the main message body and attachments will be included in the
 * ZDM headers that are prepended to the main message body.
 * <p>The toolkit will not check the value you pass in, it will simply
 * write out to the ZDM headers the same thing you pass in.
 * <p>When writing a ZDM message, the ZDM object must specify the character
 * set for the message body in its encrypted headers to be compatible with
 * existing Voltage products, so if this param is not called explicitly for
 * the message body the ZDM object will default to use utf-8. If you're
 * using some other encoding, then you should call this param with that
 * encoding. Setting the character set for the attachments is optional.
 * <p>When reading a ZDM message, you can get this param only after
 * completely inputting the contents of the entire ZDM attachment.
 * The string returned is owned by the ZDM object, so the caller should
 * not free it. If the current entry is an attachment and the character
 * set was not specified for the attachment then this param will return
 * the VT_ERROR_ATTRIBUTE_NOT_SET error code.
 * <p>The data associated with VtZDMParamCharacterSet is a pointer to an
 * unsigned char array, a UTF-8 NULL-terminated string.
 */
extern VtZDMParam VtZDMParamCharacterSet;

/** SetParam and GetParam
 * <p>Specify the original character set of the main message body or an
 * attachment. This character set value should be the original encoding used
 * for the data before it was (possibly) converted to a different encoding
 * for input to the ZDM object. The ZDM object doesn't use this attribute
 * itself, but it may be used by a Toolkit application when reading the ZDM
 * to determine if it needs to convert the data. The original character set
 * attribute should be one of the standard charset strings used with MIME
 * content types. The toolkit will not check the value you pass in -- it will
 * simply write out to the ZDM headers whatever you pass in.
 * <p>When writing a ZDM message, the ZDM object must specify the original
 * character set for the message body in its encrypted headers to be
 * compatible with existing Voltage products, so if this param is not called
 * explicitly for the message body the ZDM object will default to use utf-8.
 * If you're using some other encoding, then you should call this param with
 * that encoding. Setting the original character set for the attachments is
 * optional.
 * <p>When reading a ZDM message, you can get this param only after
 * completely inputting the contents of the entire ZDM attachment.
 * The string returned is owned by the ZDM object, so the caller should
 * not free it. If the current entry is an attachment and the character
 * set was not specified for the attachment then this param will return
 * the VT_ERROR_ATTRIBUTE_NOT_SET error code.
 * <p>The data associated with VtZDMParamOriginalCharacterSet is a pointer
 * to an unsigned char array, a UTF-8 NULL-terminated string.
 */
extern VtZDMParam VtZDMParamOriginalCharacterSet;

/** SetParam and GetParam - VtZDM2ImplWrite & VtZDM2ImplRead
 * <p>When writing, use this param to specify the message ID string for
 * the ZDM. The message ID should be a globally unique identifier
 * string that can be used to identify the message. If this attribute
 * isn't specified explicitly by the caller, the ZDM object will
 * generate a message ID automatically to be included in the ZDM headers.
 * <p>When writing a ZDM message, if this param is not called explicitly,
 * then the ZDM will default to use utf-8 for the original character set
 * specified in the ZDM headers. If you're using some other encoding,
 * then you should call this param with that encoding.
 * <p>When reading, use this param to get the message ID of the ZDM
 * object. The message ID returned by this param is extracted from the
 * encrypted ZDM headers, so it is secure. The string returned is owned
 * by the ZDM object, so the caller should not free it.
 * <p>The data associated with VtZDMParamMessageID is a pointer to an
 * unsigned char array, a UTF-8 NULL-terminated string.
 */
extern VtZDMParam VtZDMParamMessageID;

/** GetParam only - VtZDM2ImplRead
 * <p>When reading, use this param to get the insecure message ID of the
 * ZDM object. The ZDM object stores the message ID both securely
 * (in the encrypted ZDM headers) and insecurely (in an unencrypted
 * entry in the archive), so this param returns the insecure one. This
 * would be used if the reader wanted to do some logging of receipt of
 * the ZDM without having to decrypt the message. This message ID shouldn't
 * be used for any other purpose (e.g. message recall, read receipt), though,
 * where security is an issue. To get the secure message ID, use the
 * VtZDMParamMessageID param. The string returned is owned
 * by the ZDM object, so the caller should not free it.
 * <p>The data associated with VtZDMParamInsecureMessageID is a pointer
 * to an unsigned char array, a UTF-8 NULL-terminated string.
 */
extern VtZDMParam VtZDMParamInsecureMessageID;

/** GetParam only - VtZDM2ImplRead
 * <p>When reading a ZDM object, use this Param to get the file size
 * of an attachment. It should be called after the attachment entry
 * has been set with VtZDMParamCurrentEntry. When writing a ZDM
 * object the file size is determine automatically by the size of the
 * input data, so this attribute doesn't need to be used for writing.
 * <p>The info associated with this Param is an unsigned long.
 */
extern VtZDMParam VtZDMParamFileSize;

/** SetParam only - VtZDM2ImplWrite
 * <p>When writing a ZDM object, use this Param to specify the
 * charset included in the content type meta-tag of the HTML
 * wrapping. This value will be substituted in for the Charset
 * template variable used in the ZDM of HTML message body
 * template. It should correspond to the encoding that will
 * actually be used for the HTML as it's sent (i.e. if the
 * caller is going to convert the UTF-8 output from the ZDM
 * object to a different encoding).
 * <p>If this param is not set by the caller, then the default
 * value will be "utf-8".
 * <p>The info associated with this Param is a UTF-8 string, a
 * NULL-terminated unsigned char array (unsigned char *).
 */
extern VtZDMParam VtZDMParamTemplateCharset;

/** SetParam only - VtZDM2ImplWrite
 * <p>When writing a ZDM object, use this param to specify the
 * designated recipient. The designated recipient is the
 * email address of the intended recipient of the ZDM. It is
 * used during writing to automatically pick the identity used
 * to decrypt the ZDM rather than requiring the identity to be
 * explicitly selected. This will typically be used in gateway
 * applications for things like statement delivery.
 * <p>The info associated with this Param is a UTF-8 string, a
 * NULL-terminated unsigned char array (unsigned char *).
 */
extern VtZDMParam VtZDMParamDesignatedRecipient;

/** GetParam only - VtZDM2ImplWrite
 * <p>This param returns which version of ZDM should be used to send
 * the message. It checks the public parameters of the districts of
 * all of the recipients to see whether ZDM is enabled and which
 * version (i.e. version 1 or 2) is supported. It will return the
 * highest version supported by everyone. So if any of the districts
 * has the ZDM version set to 1, then the supported version will be 1.
 * Also, if all of the districts have ZDM disabled then the supported
 * version will be 0 indicating that ZDM should not be used for the
 * message.
 * <p>If the supported version is 1, then the caller should stop
 * using the ZDM object, destroy it, and switch to using a ZDM1 object
 * to send the v1 ZDM. If the supported version is 2, then the
 * caller can continue with the v2 code path to send the v2 ZDM.
 * <p>The caller must have already set up the recipient list and
 * called WriteInit before checking for the supported version.
 * <p>The info associated with this param is an unsigned int (i.e.
 * the argument to the GetParam call is the address of an
 * unsigned int cast to a Pointer*).
 */
extern VtZDMParam VtZDMParamSupportedVersion;

/** SetParam only - VtZDM2ImplWrite
 * <p>Use VtZDMParamCompressionEnabled to enable or disable whether
 * compression should be used for the ZDM attachments. The new setting
 * will affect any attachments that are subsequently added to the ZDM.
 * If this param is called after the data input for an attachment has
 * begun, then the new setting will not apply to that attachment.
 * <p>The info associated with this param is a pointer to an int. If
 * its value is 0, then compression is disabled. Otherwise compression
 * is enabled.
 */
extern VtZDMParam VtZDMParamCompressionEnabled;

/** GetParam only - VtZDM2ImplWrite & VtZDM2ImplRead
 * <p>When reading or writing a V2 ZDM object, use this param to
 * get the number of bytes remaining to be output. You'd
 * typically use this to determine how big a buffer you need to
 * allocate before calling WriteUpdate or ReadUpdate.
 * <p>When writing you'd get this param after setting the current
 * entry to VT_ZDM_CURRENT_ENTRY_DONE. When reading you'd get it
 * after setting the current entry to the message body or one of
 * the attachments.
 * <p>The info associated with this param is a pointer to an
 * unsigned int which will be set to the remaining output size.
 */
extern VtZDMParam VtZDMParamRemainingOutputSize;

/** SetParam only - VtZDM2ImplWrite
 * <p>When writing a ZDM object, use this param to specify the
 * format of the message data that should be output. Typically
 * this will be VT_MESSAGE_FORMAT_ZDM to output the
 * HTML-wrapped ZDM. Setting the message format resets the output
 * state of the ZDM object, so if you want to write the ZDM multiple
 * times with different wrappings (e.g. setting a different
 * designated recipient each time) you should call this param
 * each time before rewriting the ZDM.
 * <p>The info associated with this param is a pointer to an
 * unsigned int specifying the format - typically VT_MESSAGE_FORMAT_ZDM.
 */
extern VtZDMParam VtZDMParamMessageFormat;

/** SetParam only
 * <p>When writing a ZDM, use this param to specify the parameters
 * to use for padding the ciphertext in the ZDM to a specified length.
 * This is used to support the ZDM proxy support that was added to
 * work around restrictions in Outlook Web Access where it was
 * converting form posts to gets. For OWA the ZDM post data needs to
 * be padded to at least 4K to enabled the ZDM proxy workaround.
 * <p>The info associated with this param is a VtZDMDataPaddingInfo
 * struct.
 */
extern VtZDMParam VtZDMParamDataPadding;

/** SetParam only
 * <p>When writing a version 3 ZDM, use this param to specify the
 * maximum input field size in the generated ZDM output. If this
 * Param is not specified, then a default value of 2048 is used.
 * The max input field size can't be set to less than 200.
 * <p>The info associated with this param is an unsigned int.
 */
extern VtZDMParam VtZDMParamMaxInputFieldSize;

/** This is the data struct to accompany VtZDMParamSenderInfo.
 * <p>The senderID must be specified. The private key, if specified,
 * will be used to sign the message. It must be the private key
 * partner to the signerCert.
 * <p>If you have an identity only, the toolkit will obtain the key and
 * cert (VtZDMParamSenderId). If you have the key and cert, pass them
 * in to save time. If you have the key, cert, and identity, you can
 * pass all three in.
 */
typedef struct
{
  VtKeyObject privateKey;
  VtCertObject signerCert;
  VtIdentityObject senderId;
} VtZDMSenderInfo;

/** This is the data struct to accompany VtZDMParamRecipientIndex.
 */
typedef struct
{
  unsigned int index;
  VtPolicyCtx policyCtx;
  VtStorageCtx storageCtx;
  VtTransportCtx transportCtx;
} VtZDMRecipientIndexInfo;

/** This struct specifies an email recipient list of the ZDM (either
 * primary or CC recipients). The count field is the number of
 * recipient email addresses in the list. The emailList field is an
 * array of email addresses containing "count" elements.
 * <p>This struct is the associated info when getting or setting the
 * primary or CC email recipient list using the
 * VtZDMParamPrimaryEmailRecipientList or VtZDMParamCCEmailRecipientList
 * param. When getting an email recipient list from a ZDM object the
 * memory for the list that is returned (including the emailList array)
 * is owned by the ZDM object and should not be freed by the caller.
 */
typedef struct
{
  unsigned int count;
  const unsigned char** emailList;
} VtZDMEmailRecipientList;

/** Use VT_ZDM_CURRENT_ENTRY_MESSAGE_BODY as the type field in the
 * VtCurrentEntryInfo struct (defined below) to set the current
 * entry in the ZDM2 object to be the main message body. In this
 * case the attachmentIndex field is not used in the
 * VtCurrentEntryInfo.
 */
#define VT_ZDM_CURRENT_ENTRY_MESSAGE_BODY   1

/** Use VT_ZDM_CURRENT_ENTRY_ATTACHMENT as the type field in the
 * VtCurrentEntryInfo struct (defined below) to set the current
 * entry in the ZDM2 object to be one of the attachments. The index
 * of the attachment is specified in the zero-based attachmentIndex
 * field.
 */
#define VT_ZDM_CURRENT_ENTRY_ATTACHMENT     2

/** Use VT_ZDM_CURRENT_ENTRY_DONE as the type field in the
 * VtZDMCurrentEntryInfo struct when writing a ZDM to indicate that
 * no further entry data will be input to the ZDM. Subsequent calls
 * to VtZDMWriteUpdate wil output any buffered ZDM data.
 */
#define VT_ZDM_CURRENT_ENTRY_DONE           3

/** This struct is the associated info for the VtZDMParamCurrentEntry
 * param. The type field should be set to either
 * VT_ZDM_CURRENT_ENTRY_MESSAGE_BODY (to indicate the current entry
 * is the main message body) or VT_ZDM_CURRENT_ENTRY_ATTACHMENT to set
 * it to one of the attachments. When reading an attachment from a ZDM2
 * reader object, the index field specifies the zero-based index of the
 * attachment. The index field is not used when writing adding an
 * an attachment to a ZDM2 writer object - the new attachment is added
 * to the end of the attachment list.
 */
typedef struct
{
  int type;
  int index;
} VtZDMCurrentEntryInfo;

/** This struct is the associated info for the VtZDMParamDataPadding
 * param. The paddedSize is the size to which the ZDM post data will
 * be padded - it will be at least this size, possibly more. The
 * padChar field specifies the character that will be used to padd
 * out the data. If the VtZDMParamDataPadding param is not called,
 * then default values of 4096 and the space character will be used.
 */
typedef struct
{
  unsigned int paddedSize;
  unsigned char padChar;
} VtZDMDataPaddingInfo;

/** The following constants are the flags to be used as the info
 * for the MessageFormat param.
 */
#define VT_MESSAGE_FORMAT_SECURE_MAIL         0x0001
#define VT_MESSAGE_FORMAT_ZDM_DATA            0x0002
#define VT_MESSAGE_FORMAT_PLAIN_WRAPPED       0x0004
#define VT_MESSAGE_FORMAT_HTML_WRAPPED        0x0008

/* Common message formats */

/** The HTML-wrapped ZDM attachment included with the email.
 * Use just VT_MESSAGE_FORMAT_ZDM_DATA if it's just the base64-encoded
 * ciphertext blob (i.e. the data posted to the server).
 */
#define VT_MESSAGE_FORMAT_ZDM \
  (VT_MESSAGE_FORMAT_ZDM_DATA | VT_MESSAGE_FORMAT_HTML_WRAPPED)

/** The plain-text-wrapped SecureMail2 message */
#define VT_MESSAGE_FORMAT_SECURE_MAIL_PLAIN \
  (VT_MESSAGE_FORMAT_SECURE_MAIL | VT_MESSAGE_FORMAT_PLAIN_WRAPPED)

/** The HTML-wrapped SecureMail2 message */
#define VT_MESSAGE_FORMAT_SECURE_MAIL_HTML \
  (VT_MESSAGE_FORMAT_SECURE_MAIL | VT_MESSAGE_FORMAT_HTML_WRAPPED)

/* These are deprecated constants. Use the ones above instead */
#define VT_MESSAGE_FORMAT_ZDM_DATA_V2 VT_MESSAGE_FORMAT_ZDM_DATA
#define VT_MESSAGE_FORMAT_ZDM_V2 VT_MESSAGE_FORMAT_ZDM
#define VT_MESSAGE_FORMAT_SECURE_MAIL_PLAIN_V2 VT_MESSAGE_FORMAT_SECURE_MAIL_PLAIN
#define VT_MESSAGE_FORMAT_SECURE_MAIL_HTML_V2 VT_MESSAGE_FORMAT_SECURE_MAIL_HTML

/** Initialize the ZDM object for writing. The object should have been
 * created with an Impl that creates a message as opposed to reading a
 * message. This function will make sure it has all the information
 * necessary to write the message (except for the actual data itself)
 * and collect any "missing" information using the policy, storage, and
 * transport contexts.
 * <p>The app had the opportunity to add info to the object during the
 * Create and Set functions, but some of that info might be
 * "incomplete" and the Init function will find any needed elements.
 * For example, in order to write the message, the object may need a
 * particular IBE key. If the app added only an Identity object, the
 * Init function will use the contexts to retrieve or download the
 * appropriate information to build the key.
 * <p>If no policy, storage, and/or transport ctx are given, the
 * function will use the contexts found in the libCtx (if any).
 *
 * @param zdmObj The object created and set to write a ZDM message.
 * @param policyCtx The policy ctx to use if IBE operations are
 * performed.
 * @param storageCtx The storage ctx to use if the function needs to
 * retrieve or store elements needed to build the message.
 * @param transportCtx The transport ctx to use if IBE operations are
 * performed.
 * @param random A random object the function will use if it needs
 * random bytes (for a symmetric key or IV, for example).
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtZDMWriteInit (
   VtZDMObject zdmObj,
   VtPolicyCtx policyCtx,
   VtStorageCtx storageCtx,
   VtTransportCtx transportCtx,
   VtRandomObject random
);

/** Write out message. This function will write out what it can. It
 * does not assume that it has all the data of the message yet. If this
 * is the first call to Update, it will write out some header information
 * along with the processed data itself. That is, the output will be
 * bigger than the input.
 * <p>If this is not the first call to Update, the input data will be
 * Base64 encoded, so the output will be bigger than the input.
 * <p>The function will deposit the output into the message buffer,
 * which is bufferSize bytes big. It will set the unsigned int at
 * messageLen to the number of bytes placed into the buffer. If the
 * buffer is not big enough, the function will return
 * VT_ERROR_BUFFER_TOO_SMALL and set messageLen to the required size.
 * <p>If the object was not set with a SecureMailImpl that can write,
 * the function will return an error.
 *
 * @param zdmObj The ZDM object built to write.
 * @param random A random object that can generate random numbers if
 * necessary.
 * @param inputData The actual data to build into a ZDM message.
 * @param inputDataLen The length, in bytes, of the inputData.
 * @param message The output buffer, where the resulting ZDM message
 * will be deposited.
 * @param bufferSize The size, in bytes, of the output buffer.
 * @param messageLen The address where the implementation will deposit
 * the length, the number of bytes placed into the output buffer.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtZDMWriteUpdate (
   VtZDMObject zdmObj,
   VtRandomObject random,
   unsigned char *inputData,
   unsigned int inputDataLen,
   unsigned char *message,
   unsigned int bufferSize,
   unsigned int *messageLen
);

/** Finish writing out the message. This function will write out any
 * actual data left, then write out any trailing information (such as
 * footers). This function will match the total input to the specified
 * input from the call to SetZDMParam with the VtZDMParamDataLen Param
 * (if it was used). If there had been no call to Update, this function
 * may write out some header information along with the processed data
 * itself. That is, the output may be bigger than the input.
 * <p>The function will deposit the output into the message buffer,
 * which is bufferSize bytes big. It will set the unsigned int at
 * messageLen to the number of bytes placed into the buffer. If the
 * buffer is not big enough, the function will return
 * VT_ERROR_BUFFER_TOO_SMALL and set messageLen to the required size.
 * <p>If the object was not set with a ZDMImpl that can write, the
 * function will return an error.
 *
 * @param zdmObj The ZDM object built to write.
 * @param random A random object that can generate random numbers if
 * necessary.
 * @param inputData The actual message data.
 * @param inputDataLen The length, in bytes, of the inputData.
 * @param message The output buffer, where the resulting P7 message
 * will be deposited.
 * @param bufferSize The size, in bytes, of the output buffer.
 * @param messageLen The address where the implementation will deposit
 * the length, the number of bytes placed into the output buffer.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtZDMWriteFinal (
   VtZDMObject zdmObj,
   VtRandomObject random,
   unsigned char *inputData,
   unsigned int inputDataLen,
   unsigned char *message,
   unsigned int bufferSize,
   unsigned int *messageLen
);

/** Initialize the ZDM object for reading. The object should have been
 * created with an Impl that reads a message as opposed to writing a
 * message. This function will make sure it has all the information
 * necessary to read the message (except for the actual message itself).
 *
 * @param zdmObj The object created and set to read a ZDM message.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtZDMReadInit (
   VtZDMObject zdmObj
);

/** Begin or continue reading a ZDM message. This function does not
 * demand that the entire ZDM message be supplied, it will read whatever
 * is given.
 * <p>The function will go to the address given by bytesRead and
 * deposit there an unsigned int indicating how many bytes of the
 * message the function read. It is possible that the function does not
 * read all the bytes given. For example, if given the first part of a
 * message, the function may read the leading info (headers, etc.) but
 * not the actual data itself. That can happen if the output buffer
 * given is not big enough to hold the data the function needs to
 * return. Hence, the function read up to the data, but stopped. The
 * function will return the bytes read and the BUFFER_TOO_SMALL error.
 * <p>An application should call Update, check the bytesRead, and if
 * the bytesRead is not the same as the input length, call Update
 * again, this time on the input data bytesRead further along. If the
 * bytesRead returned is the same as the messageLen, then the
 * application should call Update with the next block of input.
 * <p>If there is any data to return, the function will return it in
 * the given outputData buffer (bufferSize bytes big) and will place
 * into the unsigned int at outputDataLen the number of bytes placed
 * into the buffer.  If the buffer is not big enough, the routine will
 * return VT_ERROR_BUFFER_TOO_SMALL and set outputDataLen to the
 * required size.
 * <p>It is possible that there is no outputData. For instance, if the
 * Update is called with only the first part of a ZDM message, and the
 * data given contains only header info, then the function will read
 * that data but have no content to return. Therefore, if you call this
 * function with a NULL output buffer, it is still possible the return
 * is 0 (no error) rather than the BUFFER_TOO_SMALL error.
 *
 * @param zdmObj The ZDM object built to read.
 * @param message The current part of the ZDM message.
 * @param messageLen The length, in bytes, of the message data.
 * @param bytesRead The address where the routine will deposit the
 * number of bytes of the message actually read. If 0, call the Update
 * function again with the same data. If messageLen, the function read
 * all the bytes, the next call should pass in the next bytes in the
 * message.
 * @param outputData The output buffer, where the resulting content
 * data will be deposited.
 * @param bufferSize The size, in bytes, of the output buffer.
 * @param outputDataLen The address where the function will deposit
 * the length, the number of bytes placed into the output buffer.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtZDMReadUpdate (
   VtZDMObject zdmObj,
   unsigned char *message,
   unsigned int messageLen,
   unsigned int *bytesRead,
   unsigned char *outputData,
   unsigned int bufferSize,
   unsigned int *outputDataLen
);

/** Finish reading a ZDM message. This function will check to make sure
 * the entire ZDM message was supplied, either during all the calls to
 * Update and this call to Final, or all at once if this is the only
 * call to Read.
 * <p>The function will go to the address given by bytesRead and
 * deposit there an unsigned int indicating how many bytes of the
 * buffer the function read. It is possible that the function does not
 * read all the bytes given. For example, it may read up to the part
 * where the function needs the app to choose a recipient before it can
 * read further. Or the output buffer is not big enough.
 * <p>An application should call Final, check the bytesRead, and if
 * the bytesRead is not the same as the input length, call Final again,
 * this time on the input data bytesRead further along. If the
 * bytesRead returned is the same as the messageLen, then the entire
 * message has been read.
 * <p>If there is any data to return, the function will return it in
 * the given outputData buffer (bufferSize bytes big) and will place
 * into the unsigned int at outputDataLen the number of bytes placed
 * into the buffer.  If the buffer is not big enough, the routine will
 * return VT_ERROR_BUFFER_TOO_SMALL and set outputDataLen to the
 * required size.
 * <p>It is possible that there is no outputData. For instance, if
 * previous calls to Update collected all the actual data, this call to
 * Final will be reading only "trailing" information in the message.
 * <p>If the input does not complete a message, this function will
 * return an error.
 * <p>Note that this function does not verify signatures, it is only
 * able to read the message. To verify signatures, after a successful
 * call to ReadFinal, call a ZDMVerify function.
 *
 * @param zdmObj The ZDM object built to read.
 * @param message The current part of the ZDM message.
 * @param messageLen The length, in bytes, of the message data.
 * @param bytesRead The address where the routine will deposit the
 * number of bytes of the message actually read. If 0, call the Final
 * function again with the same data. If messageLen, the function read
 * all the bytes, the next call should pass in the next bytes in the
 * message.
 * @param outputData The output buffer, where the resulting content
 * data will be deposited.
 * @param bufferSize The size, in bytes, of the output buffer.
 * @param outputDataLen The address where the function will deposit
 * the length, the number of bytes placed into the output buffer.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtZDMReadFinal (
   VtZDMObject zdmObj,
   unsigned char *message,
   unsigned int messageLen,
   unsigned int *bytesRead,
   unsigned char *outputData,
   unsigned int bufferSize,
   unsigned int *outputDataLen
);

/** Verify all signatures in a ZDM message. If there is more than one
 * signer, this function will try to verify all signatures and all
 * signer certs. If one element does not verify, this function sets the
 * verifyResult to did not verify.
 * <p>Before calling this function the application must finish reading
 * the message. That is, do not call this function until after a
 * successful call to ReadFinal.
 * <p>The caller passes in policy, storage and transport contexts the
 * function will use to help it find certificates it needs to chain a
 * leaf cert. The function may download district parameters to obtain
 * certs.
 * <p>The caller also passes in a certVerifyCtx, which this function
 * will use to verify any untrusted certs it encounters. The caller
 * must also pass in the appropriate associated info (verifyCtxInfo)
 * for the particular certVerifyCtx. That is the info the specific ctx
 * needs to verify a cert. This associated info will be applied to each
 * leaf cert the function verifies.
 *
 * @param zdmObj The object built to read and used to read an
 * entire ZDM message.
 * @param policyCtx A policyCtx to use if necessary.
 * @param storageCtx A storageCtx to use to help find any certs needed
 * to verify signatures or other certs.
 * @param transportCtx A transportCtx to use if necessary.
 * @param certVerifyCtx The certVerifyCtx the function will use when
 * verifying untrusted certs in a chain.
 * @param verifyCtxInfo The info the certVerifyCtx needs to help it
 * verify. This info will be applied to leaf certs.
 * @param vfyFailList Currently unused, reserved for future
 * implementations.
 * @param verifyResult The address where this function will deposit the
 * result of the verify, one for verify, zero for not verified.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtZDMVerify (
   VtZDMObject zdmObj,
   VtPolicyCtx policyCtx,
   VtStorageCtx storageCtx,
   VtTransportCtx transportCtx,
   VtCertVerifyCtx certVerifyCtx,
   Pointer verifyCtxInfo,
   VtVerifyFailureList vfyFailList,
   unsigned int *verifyResult
);

/** Determine whether or not the message is a ZDM message. The
 * message data should include at least the first 512 bytes of the
 * message. If it is a ZDM message, then the version and type are
 * returned in the output parameters and 0 is returned. For V1
 * ZDM messages, the type is either VT_ZDM_MAIL or VT_ZDM_ATTACHMENT.
 * For V2 and V3 messages, the type is always VT_ZDM_MAIL. If the
 * message is not a ZDM, then a VT_ERROR_INVALID_ZDM_MSG error code
 * is returned.
 * @param libCtx The lib ctx to use.
 * @param message The beginning (at least 512 bytes) of the message.
 * @param messageLength The length of the message data.
 * @param version Set to the ZDM version if the message is a ZDM.
 * @param type Set to the ZDM type if the message is a ZDM.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtZDMDetermineVersionAndType (
  VtLibCtx libCtx,
  unsigned char* message,
  unsigned int messageLength,
  unsigned int* version,
  unsigned int* type
);

/*@}*/

/*=========================================================*/
/*                                                         */
/* Secure Archive Object                                   */
/*                                                         */
/*=========================================================*/

/**
 * \defgroup SecureArchiveGroup Secure Archive Object
 */

/*@{*/

/** The Secure Archive object.
 * <p>Note that the object is a pointer type.
 */
typedef struct VtSecureArchiveObjectDef *VtSecureArchiveObject;

/** The function VtCreateSecureArchiveObject builds an object that
 * reads or writes secure archive content using a VtSecureArchiveImpl.
 * This typedef defines what a VtSecureArchiveImpl is. Although a
 * VtSecureArchiveImpl is a function pointer, an application should
 * never call one directly, only pass it as an argument to
 * VtCreateSecureArchiveObject.
 */
typedef int VT_CALLING_CONV (VtSecureArchiveImpl) (
   VtSecureArchiveObject *, Pointer, unsigned int);

/** The functions VtSetSecureArchiveParam and VtGetSecureArchiveParam
 * add or get information to or from a secure archive object. The
 * information to add or get is defined by a VtSecureArchiveParam. This
 * typedef defines what a* VtSecureArchiveParam is. Although a
 * VtSecureArchiveParam is a function pointer, an application should
 * never call one directly, only pass it as an argument to
 * VtSetSecureArchiveParam or VtGetSecureArchiveParam.
 */
typedef int VT_CALLING_CONV (VtSecureArchiveParam) (
   VtSecureArchiveObject, Pointer, unsigned int);

/** Create a new secure archive object. This allocates space for an "empty"
 * object, then loads the given impl to make it an "active" object.
 * <p>The VtSecureArchiveImpl defines what the object will be able to process
 * (generally reading and writing will be different Impls). The
 * include file vibe.h defines the supported impls. Look through
 * the include file to see which impl to use for your application.
 * All supported impls will be defined as in the following example.
 * <code>
 * <pre>
 *   extern VtSecureArchiveImpl VtSecureArchiveImplWrite;
 * </pre>
 * </code>
 * <p>Associated with each impl is specific info. The documentation
 * for each impl will describe the associated info it needs. That
 * data could be another object, it could be data in a particular
 * struct, it might be a NULL pointer. Check the documentation for each
 * impl for a description of the data and its required format.
 * <p>To use this function decide which impl you want to use, then
 * determine what information that impl needs and in which format it
 * is presented. Collect the data in the appropriate format then call
 * this function passing in the desired impl and the required info.
 * The associated info must be cast to Pointer.
 *
 * @param libCtx The library context.
 * @param secureArchiveImpl The implementation the object will use.
 * @param associatedInfo The info needed by the SecureArchiveImpl.
 * @param secureArchiveObj A pointer to where the routine will deposit the
 * created object.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtCreateSecureArchiveObject (
   VtLibCtx libCtx,
   VtSecureArchiveImpl secureArchiveImpl,
   Pointer associatedInfo,
   VtSecureArchiveObject *secureArchiveObj
);

/* These are the VtSecureArchiveImpls supported by the toolkit.
 * Each VtSecureArchiveImpl is used in conjunction with special info for
 * the function. If there is no special info, the accompaniment
 * is a NULL pointer.
 */

/** Use this Impl to build a secure archive object that will be
 * able to create secure archive content.
 * <p>The data associated with VtSecureArchiveImplWrite is a
 * NULL pointer: (Pointer)0.
 */
extern VtSecureArchiveImpl VtSecureArchiveImplWrite;

/** Use this Impl to build a secure archive object that will be able
 * to read secure archive content.
 * <p>The data associated with VtSecureArchiveImplRead is a pointer to
 * a VtReadSecureArchiveInfo struct containing all the algorithms
 * (asymmetric and symmetric algorithms represented as DERCoders)
 * and identity schemas (represented as SchemaDecodes) the app is
 * willing to support. The mpCtx is for performing IBE private key
 * operations with the identity eventually chosen to decrypt the content.
 * <p>If no DerCoders, SchemaDecodes, or mpCtx are supplied, the Impl
 * will try to find these elements in the libCtx.
 * <p>If the accompanying info is a NULL pointer, this Impl will try to
 * find all the info it needs from the libCtx.
 */
extern VtSecureArchiveImpl VtSecureArchiveImplRead;

/** This is the data struct to accompany VtSecureArchiveImplRead.
 */
typedef struct
{
  VtDerCoder **derCoders;
  unsigned int derCoderCount;
  VtIdentitySchemaDecode **decoders;
  unsigned int decoderCount;
  VtMpIntCtx mpCtx;
} VtReadSecureArchiveInfo;

/** Destroy a secure archive object. This frees up any memory
 * allocated during the object's creation and use.
 * <p>If you call Create, you should call Destroy after you are done with
 * the object but before the program exits.
 * <p>Example:
 * <code>
 * <pre>
 *    int status;
 *    VtSecureArchiveObject secureArchiveObj = (VtSecureArchiveObject)0;
 *
 *    do {
 *          . . .
 *      status = VtCreateSecureArchiveObject (
 *        libCtx, VtSecureArchiveImplWrite, (Pointer)0, &secureArchiveObj);
 *      if (status != 0)
 *        break;
 *          . . .
 *    } while (0);
 *
 *    VtDestroySecureArchiveObject (&secureArchiveObj);
 * </pre>
 * </code>
 *
 * @param secureArchiveObj A pointer to where the routine will find the object to
 * destroy.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtDestroySecureArchiveObject (
   VtSecureArchiveObject *secureArchiveObj
);

/** Set the SecureArchive object with the information given.
 * <p>The VtSecureArchiveParam defines what information the object
 * will be set with.
 * <p>The supported secure archive params are defined below. Look
 * through them to see which params to use for your application.
 * All supported params will be defined as in the following example.
 * <code>
 * <pre>
 *   extern VtSecureArchiveParam VtSecureArchiveParamSignerId;
 * </pre>
 * </code>
 * <p>Associated with each secure archive param is specific info. The
 * documentation for each secure archive param will describe the
 * associated info it needs. That data could be another object, it
 * could be data in a particular struct, it might be a NULL pointer.
 * Check the documentation for each secure archive param for a
 * description of the data and its required format.
 * <p>To use this function decide which SecureArchiveParam you want
 * to use, then determine what information that SecureArchiveParam
 * needs and in which format it is presented. Collect the data in the
 * appropriate format then call this function passing in the desired
 * param and and the required info. The associated info must be cast
 * to Pointer.
 * <p>For example, if the required info is a particular struct, declare
 * a variable to be of that type struct, fill in the fields, then pass
 * the address of that struct cast to Pointer.
 *
 * @param secureArchiveObj The object to set.
 * @param secureArchiveParam What the object is being set to.
 * @param associatedInfo The info needed by the VtSecureArchiveParam.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtSetSecureArchiveParam (
   VtSecureArchiveObject secureArchiveObj,
   VtSecureArchiveParam secureArchiveParam,
   Pointer associatedInfo
);

/** Get the specified type of information out of a secure archive object.
 * <p>This function will go to the address given by getInfo and
 * deposit a pointer. That pointer is the address of the data. The
 * memory to hold the information belongs to the object, do not free it.
 * <p>The VtSecureArchiveParam will specify what kind of information will be
 * returned, the getInfo is where the function will deposit the info.
 * <p>The supported params are described below. Look through them to see
 * which params to use for your application.
 * <p>See also VtSetSecureArchiveParam.
 * <p>To use this function decide which param you want to use, then
 * determine what information that param will return and in which
 * format it is presented. Declare a variable to be a pointer to the
 * appropriate type, then call this function passing in the desired
 * ZDMParam and the address of the variable. Cast the address of the
 * pointer to the appropriate type: (Pointer *).
 * <p>If the associated info is already a pointer type (such as an
 * object), declare a variable to be of the appropriate type and pass
 * the address of that variable cast to (Pointer *).
 * <p>For example, if the returned info is a VtIdentityList, declare a
 * variable to be of type VtIdentityList, pass the address of that
 * variable (&varName) cast to (Pointer *).
 * <p>Example:
 * <pre>
 * <code>
 *    VtIdentityList recipientList;
 *
 *    do {
 *          . . .
 *      status = VtGetSecureArchiveParam (
 *        readObj, VtSecureArchiveParamRecipientList, (Pointer *)&recipientList);
 *      if (status != 0)
 *        break;
 *          . . .
 *    } while (0);
 * </code>
 * </pre>
 *
 * @param secureArchiveObj The object to query.
 * @param secureArchiveParam Defines what info to get.
 * @param getInfo The address of a pointer. The function will go to
 * that address and deposit a pointer to the info requested.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtGetSecureArchiveParam (
   VtSecureArchiveObject secureArchiveObj,
   VtSecureArchiveParam secureArchiveParam,
   Pointer *getInfo
);

/* These are the VtSecureArchiveParams supported by the toolkit.
 * Each VtSecureArchiveParam is used in conjunction with special
 * info for the function.
 */

/** SetParam only - VtSecureArchiveImplWrite
 * <p>Use this param to set a secure archive object with a signer
 * represented as an identity. With an object built with this param,
 * the toolkit will obtain the signing key and cert during the call
 * to VtWriteInit, either from storage or from the key server.
 * <p>If you set a secure archive object using this param, do not set
 * it with another identity, either using this param or
 * VtSecureArchiveParamSignerInfo. That is, a secure archive has one
 * and only one signer.
 * <p>The info associated with this Param is an identity object.
 */
extern VtSecureArchiveParam VtSecureArchiveParamSignerId;

/** SetParam only - VtSecureArchiveImplWrite
 * <p>Use this param to set a secure archive object with the signer
 * info it will need in order to create the archive. This will include
 * a private key with which to sign and optionally the cert associated
 * with that private key.
 * <p>The accompanying data includes an identity object. That info is
 * optional. If you have an identity only, the toolkit will obtain
 * the key and cert (VtSecureArchiveParamSignerId). If you have the key
 * and cert, pass them in to save time.
 * <p>If you set a secure archive object using this param, do not set
 * it with another identity, either using this param or
 * VtSecureArchiveParamSignerId. That is, a secure archive has one and
 * only one signer.
 * <p>The private key object passed in at this time will not be cloned.
 * That is, the secure archive object set with this param will copy a
 * reference to that key. When the secure archive object needs to sign,
 * it will use that key object. If you use this param, make sure the
 * key object stays unchanged until after WriteFinal.
 * <p>The info associated with this Param is a pointer to a
 * VtSecureArchiveSignerInfo struct.
 */
extern VtSecureArchiveParam VtSecureArchiveParamSignerInfo;

/** SetParam & GetParam with VtSecureArchiveImplWrite or GetParam
 * only with VtSecureArchiveImplRead.
 * <p>Use this param to get/set the list of recipients for a
 * secure archive. This is the list of identities that will be able
 * to decrypt the archive contents.
 * <p>When an object built to read a secure archive reads all the
 * RecipientInfos, it will build a VtIdentityList containing the
 * identities of all the recipients. The app can then get that list by
 * using this param, and then examine each of the identities to
 * choose which one the object should use to open the envelope
 * (indicate which identity to use by calling VtSetSecureArchiveParam
 * with the VtSecureArchiveParamRecipientIndex param).
 * <p>The info associated with this param is a VtIdentityList object.
 * When used with VtGetSecureArchiveParam you'd pass the address of
 * a VtIdentityList variable.
 */
extern VtSecureArchiveParam VtSecureArchiveParamRecipientList;

/** SetParam and GetParam.
 * <p>Use this param to add recipients to an object built to write
 * SecureArchivecontent, the recipients represented as an already
 * existing RecipientInfoList (as opposed to an IdentityList).
 * <p>Or use this param to get the recipients of a SecureArchive being
 * read, represented as a RecipientInfoList.
 * <p>When an object built to write SecureArchive creates the digital
 * envelope, it will use the encoded RecipientInfos from the associated
 * info, instead of building it. It will also use the symmetric key and
 * symmetric algorithm specified in the associated info.
 * <p>When writing SecureArchive content, using this Param is equivalent
 * to using VtSecureArchiveParamRecipientList along with either
 * VtSecureArchiveParamEnv3DESCBC or VtSecureArchiveParamEnvAES128CBC.
 * <p>When an object built to read a SecureArchive is done reading the
 * entire message, an app can use this Param to get the
 * RecipientInfoList representation of the recipients. The app can then
 * get information out of the RecipientInfoList, such as the symmetric
 * key and algorithm.
 * <p>Note! When reading, the CHOOSE_RECIPIENT error means you get an
 * IdentityList out of the SecureArchive object and you pick an identity
 * to use to decrypt. You cannot get the RecipientInfoList at that
 * time, you must wait until reading the entire message.
 * <p>After Getting the RecipientInfoList out of a SecureArchive object,
 * you can call VtEncodeRecipientInfoList. If you want the symmetric key
 * protected in the RecipientInfoBlock, Set it with a secretValue
 * first. NOTE!!! This is a departure from normal toolkit rules.
 * Normally we say do not alter objects returned from a Get call,
 * however, in this case, the object returned by the Get will have no
 * secretValue installed, so if you want to use one, you must add one.
 * <p>The info associated with this Param is a VtRecipientInfoList
 * object.
 * <p>NOTE!!! If setting a SecureArchive object with a RecipientInfoList,
 * each RecipientInfo must be encoded. That is, you must have called
 * VtEncodeRecipientInfo on each RecipientInfoObject inside the
 * RecipientInfoList, or you must have called
 * VtEncodeRecipientInfoList on the RecipientInfoList.
 * <p>NOTE!!! If setting a SecureArchive object with a RecipientInfoList,
 * the RecipientInfoList MUST be set so that it can produce the
 * RecipientInfoBlock. (See the documentation on
 * VtEncodeRecipientInfoList for info on obtaining RecipientInfoBlock
 * versus RecipientInfos.)
 * <p>When getting, pass in the address of a VtRecipientInfoList
 * variable as the getInfo, the Get function will desposit a
 * VtRecipientInfoList at the address.
 */
extern VtSecureArchiveParam VtSecureArchiveParamRecipientInfoList;

/** SetParam only - VtSecureArchiveImplWrite
 * <p>Use this SecureArchiveParam with a secure archive built to write.
 * It indicates the object should use Triple DES in CBC mode to encrypt
 * the data.
 * <p>The data associated with VtSecureArchiveParam3DESCBC is a
 * NULL pointer: (Pointer)0.
 */
extern VtSecureArchiveParam VtSecureArchiveParam3DESCBC;

/** SetParam only - VtSecureArchiveImplWrite
 * <p>Use this param with a secure archive object built to write. It
 * indicates the object should use AES-128 in CBC mode to encrypt the
 * data.
 * <p>The data associated with VtSecureArchiveParamAES128CBC is a
 * NULL pointer: (Pointer)0.
 */
extern VtSecureArchiveParam VtSecureArchiveParamAES128CBC;

/** SetParam only - VtSecureArchiveImplWrite
 * <p>Use VtSecureArchiveParamCompressionEnabled to enable or disable
 * whether compression should be used for the secure entries. The new
 * setting will affect any secure entries that are subsequently added
 * to the secure archive. If this param is called after the data input
 * for a secure entry has begun, then the new setting will not apply to 
 * that entry.
 * <p>The info associated with this param is a pointer to an int. If
 * its value is 0, then compression is disabled. Otherwise compression
 * is enabled.
 */
extern VtSecureArchiveParam VtSecureArchiveParamCompressionEnabled;

/** GetParam only - VtSecureArchiveImplRead
 * <p>The entries in a secure archive contains the time they were signed
 * (the signingTimeAttribute in the P7 SignedData). This Param gets that
 * time out of the current entry of the secure archive.
 * <p>Get this info only after completely reading the message.
 * <p>The info associated with this Param is a pointer to a VtTime
 * struct.
 */
extern VtSecureArchiveParam VtSecureArchiveParamSigningTime;

/** SetParam only. VtSecureArchiveImplRead
 * <p>Use this param to let a secure archive object built to read
 * know which identity to use to decrypt the archive entries.
 * <p>Typically this param will be called when the return code
 * from VtSecureArchiveReadUpdate is VT_ERROR_CHOOSE_RECIPIENT.
 * At that point the app can get the recipient list out of of the
 * secure archive object (using VtSecureArchiveParamRecipientList),
 * examine the identities of the recipients, and choose which one
 * should be used to decrypt the message. The identity chosen will
 * have an index associated with it; it is the index into the
 * IdentityList of all the recipients. This param specified the index
 * of the identity in the IdentityList the object should use. If you
 * know which recipient to use beforehand (e.g. you know that there's
 * always only going to be just one recipient), then you don't need to
 * iterate over the recipient list and can instead just call this
 * param with the known index at any point after the secure archive
 * object is created.
 * <p>When the secure archive actually begin to decrypt an entry it
 * will also need policy, storage, and transport contexts to obtain
 * the IBE private key. These contexts are also specified when the
 * recipient index is set. The secure archive object will keep references
 * to these contexts and will use them to decrypt each entry, so
 * these context must remain valid while the secure archive object is
 * being used. If the contexts aren't specified in the recipient index
 * info, then the secure archive will look for them in the library
 * context.
 * <p>The data associated with VtSecureArchiveParamRecipientIndex is a
 * pointer to a VtSecureArchiveRecipientIndexInfo struct.
 */
extern VtSecureArchiveParam VtSecureArchiveParamRecipientIndex;

/** SetParam only - both VtSecureArchiveReadImpl and VtSecureArchiveWriteImpl.
 * <p> Use VtSecureArchiveParamBufferType to specify what type of
 * buffer the secure archive object should use for any internal
 * buffering it requires. For example, in the current implementation
 * this buffer type will be used for the intermediate buffers needed
 * when entries are compressed. The buffer type must be either
 * VT_BUFFER_TYPE_MEMORY or VT_BUFFER_TYPE_FILE (i.e. it can't
 * be VT_BUFFER_TYPE_STREAM).
 * <p>The associated info is a pointer to a VtBufferTypeInfo
 * struct. If the associated info is NULL, then memory-based
 * buffering will be used. This is also the default method if this
 * parameter is never set.
 */
extern VtSecureArchiveParam VtSecureArchiveParamBufferType;

/** SetParam only - VtSecureArchiveImplRead
 * <p>Use VtSecureArchiveParamInputStream to specify a stream
 * to use for the input data for the archive. This can be used
 * instead of explicitly streaming in all the data using
 * VtSecureArchiveReadUpdate. This can be easier and more
 * efficient if the archive is coming from a file, so that the
 * caller can just set up a stream directly from the file.
 * <p>This param should be called before the ReadInit call is made.
 * If you set the input stream, then you can skip the phase where
 * you input the contents of the secure archive and go straight to
 * setting the current entry and outputting the message body and/or
 * attachment data.
 * <p> The caller retains ownership of the stream and is responsible
 * for closing and destroying the stream after the secure archive
 * is no longer being used.
 * <p>The associated info is the VtStreamObject to be used for
 * the input stream.
 */
extern VtSecureArchiveParam VtSecureArchiveParamInputStream;

/** SetParam only - VtSecureArchiveImplWrite
 * <p>Use VtSecureArchiveParamOutputStream to specify a stream
 * to use for the output data for the archive. This can be used
 * instead of explicitly streaming out all the data using
 * VtSecureArchiveWriteUpdate. This can be easier and more efficient
 * if the archive will be written to a file, so that the caller can
 * just set up a stream directly to the file rather than having the
 * archive maintain a separate internal stream that will just be
 * copied out to the file at the end.
 * <p>This param should be called before the WriteInit call is made.
 * After the current entry is set to done, then the output stream
 * will contain the data for the secure archive (i.e. the data that
 * would be streamed out if you continued to make WriteUpdate and/or
 * WriteFinal calls).
 * The caller retains ownership of the stream and is responsible for
 * closing and destroying the stream after the secure archive is no
 * longer being used.
 * <p>The associated info is the VtStreamObject to be used for
 * the output stream.
 */
extern VtSecureArchiveParam VtSecureArchiveParamOutputStream;

/** GetParam only - VtSecureArchiveReadImpl.
 * <p>Use this param to obtain the number of secure entries in the
 * secure archive.
 * <p>The associated info is a pointer to an unsigned int where the
 * count will be returned.
 */
extern VtSecureArchiveParam VtSecureArchiveParamEntryCount;

/** SetParam only - both VtSecureArchiveReadImpl and VtSecureArchiveWriteImpl.
 * <p>Use this param to specify the current entry in the secure archive.
 * Calls to other parameters to get/set the content type, characters set
 * or other attributes as well as ReadUpdate and WriteUpdate calls
 * refer to the current entry specified with this param. The current
 * entry can be a secure entry, an insecure entry, or the index data.
 * By default the current entry is the index data.
 * <p>When writing a secure archive the typical sequence to add a new entry
 * is to use the VtSecureArchiveParamCurrentEntry param with an entry type of
 * VT_SECURE_ARCHIVE_CURRENT_ENTRY_SECURE, then use the
 * VtSecureArchiveParamFileName param to set the name of the entry, and then
 * make a sequence of the VtSecureArchiveWriteUpdate calls to stream in the
 * data for the entry. This can be repeated to add more entries to the
 * archive. After all the entries have been added, you'd make a final
 * call with the VtSecureArchiveParamCurrentEntry param to set the current
 * entry to VT_SECURE_ARCHIVE_CURRENT_ENTRY_DONE. Then you'd make more
 * WriteUpdate calls with a valid output buffer to stream out the contents
 * of the secure archive. The steps for adding an insecure entry or index
 * data are the same except that you'd use a different entry type in the
 * VtSecureArchiveCurrentEntryInfo struct when setting the current entry.
 * <p>When reading a secure archive, you'd first stream in the contents of
 * the secure archive with a sequence of ReadUpdate calls, then you'd set
 * the current entry to VT_SECURE_ARCHIVE_CURRENT_ENTRY_SECURE with a valid
 * entry index (between 0 and the total count; you can get the count using
 * the VtSecureArchiveParamEntryCount param). Then you can get the name of
 * the entry using the VtSecureArchiveFileName param, and stream out the
 * data with a sequence of ReadUpdate calls with a valid output buffer.
 * The steps for reading an insecure entry or the index data are the same
 * except for the entry type value when setting the current entry.
 * <p>The associated info is a pointer to a VtSecureArchiveCurrentEntryInfo
 * struct.
 */
extern VtSecureArchiveParam VtSecureArchiveParamCurrentEntry;

/** SetParam with VtSecureArchiveWriteImpl and GetParam with
 * VtSecureArchiveReadImpl.
 * <p>Use this param to get or set an attribute associated with the
 * current entry. If there is no current entry, then the attribute
 * is associated with the overall archive. When writing a secure
 * archive, set this param to specify an attribute. When reading
 * a secure archive, get this param to return the value of an attribute.
 * <p>When setting an attribute both the name and value field of
 * the attribute info should be specified.
 * <p>When getting an attribute the associated info should point to
 * a VtSecureArchiveAttributeInfo struct whose name field has been
 * set to specify which attribute to get. The attribute value will
 * be returned in the value field of the attribute info struct.
 * <p>The associated info is a pointer to a VtSecureArchiveAttributeInfo
 * struct.
 */
extern VtSecureArchiveParam VtSecureArchiveParamEntryAttribute;

/** SetParam with VtSecureArchiveWriteImpl and GetParam with
 * VtSecureArchiveReadImpl.
 * <p>Use this param to get or set an attribute associated with the
 * overall archive. When writing a secure archive, set this param
 * to specify an attribute. When reading a secure archive, get this
 * param to return the value of an attribute.
 * <p>When setting an attribute both the name and value field of
 * the attribute info should be specified.
 * <p>When getting an attribute the associated info should point to
 * a VtSecureArchiveAttributeInfo struct whose name field has been
 * set to specify which attribute to get. The attribute value will
 * be returned in the value field of the attribute info struct.
 * <p>The associated info is a pointer to a VtSecureArchiveAttributeInfo
 * struct.
 */
extern VtSecureArchiveParam VtSecureArchiveParamArchiveAttribute;

/** SetParam with VtSecureArchiveWriteImpl and GetParam with
 * VtSecureArchiveReadImpl.
 * <p>When writing a secure archive entry, set this param to specify
 * the file name of the current entry being encrypted.
 * <p>When reading, get this param to return the file name of the
 * current entry.
 * <p>The info associated with this Param is a UTF-8 string, a
 * NULL-terminated unsigned char array (unsigned char *). When
 * used with VtGetSecureArchiveParam you'd pass the address of
 * a unsigned char* variable.
 */
extern VtSecureArchiveParam VtSecureArchiveParamFileName;

/** GetParam only - VtSecureArchiveReadImpl only
 * <p>When reading a secure archive entry, use this param to obtain
 * the size of the current entry.
 * <p>The info associated with this Param is a pointer to an
 * unsigned int.
 */
extern VtSecureArchiveParam VtSecureArchiveParamFileSize;

/** SetParam with VtSecureArchiveWriteImpl and GetParam with
 * VtSecureArchiveReadImpl.
 * <p>When writing a secure archive entry, use this param to specify
 * the mod date of the current entry being encrypted. This can be used
 * later when the entry is extracted from the archive to restore the
 * mod date for the extracted file.
 * <p>When reading, this param will return the mod date of the
 * current entry.
 * <p>The info associated with this param is a pointer to a VtTime struct.
 */
extern VtSecureArchiveParam VtSecureArchiveParamModDate;

/** GetParam only - VtSecureArchiveReadImpl
 * <p>Use this param to get the list of signers out of a secure archive
 * being read.
 * <p>When an object built to read reads all the SignerInfos, it
 * will build a VtIdentityList containing the identities of all the
 * signers. The app can then get that list by using this Param.
 * <p>The info associated with this Param is a VtIdentityList object.
 */
extern VtSecureArchiveParam VtSecureArchiveParamSignerList;

/** GetParam only
 * <p>When reading or writing a secure archive object, use this
 * param to get the number of bytes remaining to be output. You'd
 * typically use this to determine how big a buffer you need to
 * allocate before calling WriteUpdate or ReadUpdate.
 * <p>When writing you'd get this param after setting the current
 * entry to VT_SECURE_ARCHIVE_CURRENT_ENTRY_DONE. When reading you'd
 * get it after setting the current entry to the message body or one
 * of the attachments.
 * <p>The info associated with this param is a pointer to an
 * unsigned int which will be set to the remaining output size.
 */
extern VtSecureArchiveParam VtSecureArchiveParamRemainingOutputSize;

/** This is the data struct to accompany VtSecureArchiveParamSignerInfo.
 * The signerId is optional. If you have it available, pass it in, if
 * not, leave the field NULL.
 * <p>If you have an identity only, the toolkit will obtain the key and
 * cert (VtSecureArchiveParamSignerId). If you have the key and cert,
 * pass them in to save time. If you have the key, cert, and identity,
 * you can pass all three in. If the key and cert are not associated
 * with an identity (not an IBE-based sender), pass the key and cert
 * only.
 */
typedef struct
{
  VtKeyObject privateKey;
  VtCertObject signerCert;
  VtIdentityObject signerId;
} VtSecureArchiveSignerInfo;

/** This is the data struct to accompany VtSecureArchiveParamRecipientIndex.
 * If any of the context fields are NULL, then the secure archive will try
 * to retrieve them from the library context.
 */
typedef struct
{
  unsigned int index;
  VtPolicyCtx policyCtx;
  VtStorageCtx storageCtx;
  VtTransportCtx transportCtx;
} VtSecureArchiveRecipientIndexInfo;

/** Use VT_SECURE_ARCHIVE_CURRENT_ENTRY_SECURE as the type field in
 * the VtSecureArchiveCurrentEntryInfo struct (defined below) to set
 * the current entry in the secure archive object to be a secure
 * entry. When used with a secure archive writer object, a new
 * secure entry is created at the end of the entry list (i.e. the
 * "index" and "name" fields of the VtSecureArchiveCurrentEntryInfo
 * struct are ignored). When used with a secure archive reader object,
 * the index of the entry is specified in the "index" field of the
 * VtSecureArchiveCurrentEntryInfo struct.
 */
#define VT_SECURE_ARCHIVE_CURRENT_ENTRY_SECURE    1

/** Use VT_SECURE_ARCHIVE_CURRENT_ENTRY_INSECURE as the type field in
 * the VtSecureArchiveCurrentEntryInfo struct (defined below) to set
 * the current entry in the secure archive object to be one of the
 * insecure entries. The data for insecure entries will not be
 * encrypted. When used with a secure archive writer object, a new
 * unnamed insecure entry is created (i.e the "index" and "name" fields
 * of VtSecureArchiveCurrentEntryInfo are ignored). The name should be
 * specified later by setting the VtZDMParamFileName param. When used
 * with a secure archive reader object, the name of the insecure entry
 * is specified in the "name" field of the 
 * VtSecureArchiveCurrentEntryInfo struct.
 */
#define VT_SECURE_ARCHIVE_CURRENT_ENTRY_INSECURE  2

/** Use VT_SECURE_ARCHIVE_CURRENT_ENTRY_INDEX as the type field in the
 * VtSecureArchiveCurrentEntryInfo struct to set the current entry
 * in the secure archive object to be the additional data that is
 * appended to the archive index. This is used, for example, with
 * ZDM2 objects to include the main message body in the archive
 * index, which eliminates the extra decryption that
 * would be required if the message body was stored separately.
 * The index and name fields are not used.
 */
#define VT_SECURE_ARCHIVE_CURRENT_ENTRY_INDEX     3

/** Use VT_SECURE_ARCHIVE_CURRENT_ENTRY_DONE as the type field in the
 * VtCurrentEntryInfo struct (defined below) when writing a secure
 * archive to indicate that no further entry data will be input to
 * the archive. Subsequent calls to VtSecureArchiveWriteUpdate will
 * output any buffered archive data. The index and name fields are
 * not used.
 */
#define VT_SECURE_ARCHIVE_CURRENT_ENTRY_DONE      4

/** This struct is the associated info for the
 * VtSecureArchiveParamCurrentEntry param. The type field should be
 * set to either VT_SECURE_ARCHIVE_CURRENT_ENTRY_SECURE,
 * VT_SECURE_ARCHIVE_CURRENT_ENTRY_INSECURE,
 * VT_SECURE_ARCHIVE_CURRENT_ENTRY_INDEX or
 * VT_SECURE_ARCHIVE_CURRENT_ENTRY_DONE.
 * <p>When writing a secure archive, only the type field of the info
 * struct is used. It indicates whether you're adding a new secure or
 * insecure entry, or that you're done adding entries. If you're adding
 * a new entry you specify the name of the entry with the
 * VtSecureArchiveParamFileName param. Setting the current entry to an
 * existing entry during writing is not supported currently. 
 * <p>When using this param to read a secure archive, if the entry
 * type is VT_SECURE_ARCHIVE_CURRENT_ENTRY_SECURE, then the index field
 * should be set to specify the index of the secure entry. The index should
 * be >= 0 and less than the entry count returned from the
 * VtSecureArchiveParamEntryCount param. If the type is
 * VT_SECURE_ARCHIVE_CURRENT_ENTRY_INSECURE, then the name field should be
 * set to specify the name of the insecure entry to read. There is currently
 * no way to iterate over all of the insecure entries, so you must already
 * know the name.
 */
typedef struct
{
  int type;
  int index;
  const unsigned char* name;
} VtSecureArchiveCurrentEntryInfo;

/** This struct is the associated info for the
 * VtSecureArchiveParamEntryAttribute and
 * VtSecureArchiveParamArchiveAttribute params.
 * <p>When setting an attribute both the name and value should be
 * specified (NULL-terminated UTF-8 strings). When getting an attribute
 * the associated info will point to a VtSecureArchiveAttributeInfo with
 * the name specified and the corresponding value will be filled in by
 * the GetParam call. The secure archive object retains ownership of the
 * value string returned by the GetParam call, so the caller should not
 * free it.
 */
typedef struct
{
  const unsigned char* name;
  const unsigned char* value;
} VtSecureArchiveAttributeInfo;

/** Initialize the secure archive object for writing. The object
 * should have been created with an impl that writes a secure
 * archive as opposed to reading. This function supplies the object
 * with information necessary to write the secure archive
 * (except for the actual data itself) and collect any "missing"
 * information using the policy, storage, and transport contexts.
 * <p>The app had the opportunity to add info to the object during the
 * Create and Set functions, but some of that info might be
 * "incomplete" and the Init function will find any needed elements.
 * For example, in order to write the archive, the object may need a
 * particular IBE key. If the app added only an Identity object, the
 * Init function will use the contexts to retrieve or download the
 * appropriate information to build the key.
 * <p>If the caller passes in any of the contexts or the random object,
 * then these contexts/objects must not be destroyed before the secure
 * archive is finished writing (i.e. until after WriteFinal is called).
 * <p>If no policy, storage, and/or transport ctx are given, the
 * function will use the contexts found in the libCtx (if any).
 *
 * @param secureArchiveObj The object created and set to write a
 * secure archive.
 * @param policyCtx The policy ctx to use if IBE operations are
 * performed.
 * @param storageCtx The storage ctx to use if the function needs to
 * retrieve or store elements needed to build the message.
 * @param transportCtx The transport ctx to use if IBE operations are
 * performed.
 * @param random A random object the function will use if it needs
 * random bytes (for a symmetric key or IV, for example).
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtSecureArchiveWriteInit (
   VtSecureArchiveObject secureArchiveObj,
   VtPolicyCtx policyCtx,
   VtStorageCtx storageCtx,
   VtTransportCtx transportCtx,
   VtRandomObject random
);

/** Update the secure archive. Currently, due to the archiving and
 * compressing implementations used, the secure archive impls must
 * buffer the entire archive (either in memory or a file), so it's
 * not able to support streaming of output as it gets input. This
 * means that WriteUpdate has separate input and output phases.
 * <p>In the input phase the caller inputs the index and secure
 * entry data - no output is generated. When input is complete, and
 * the current entry has been set to VT_SECURE_ARCHIVE_CURRENT_ENTRY_DONE,
 * then the output phase begins and calls to WriteUpdate stream out the
 * final secure archive.
 * <p>If the object was not set with a SecureArchiveImpl that can write,
 * the function will return an error.
 *
 * @param secureArchiveObj The secure archive object built to write.
 * @param input The actual data to build into a secure archive.
 * @param inputLen The length, in bytes, of the inputData.
 * @param output The output buffer, where the resulting secure
 * archive will be deposited.
 * @param bufferSize The size, in bytes, of the output buffer.
 * @param outputLen The address where the implementation will deposit
 * the length, the number of bytes placed into the output buffer.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtSecureArchiveWriteUpdate (
   VtSecureArchiveObject secureArchiveObj,
   unsigned char *input,
   unsigned int inputLen,
   unsigned char *output,
   unsigned int bufferSize,
   unsigned int *outputLen
);

/** Finish writing out the secure archive. This function will write
 * out any remaining data that hasn't already been streamed out with
 * VtSecureArchiveWriteUpdate calls.
 * <p>The function will deposit the output into the output buffer,
 * which is bufferSize bytes big. It will set the unsigned int at
 * outputLen to the number of bytes placed into the buffer. If the
 * buffer is not big enough, the function will return
 * VT_ERROR_BUFFER_TOO_SMALL and set outputLen to the required size.
 * <p>If the object was not set with a impl that can write, the
 * function will return an error.
 *
 * @param secureArchiveObj The secure archive object built to write.
 * @param output The output buffer, where the resulting secure
 * archive output will be put.
 * @param bufferSize The size, in bytes, of the output buffer.
 * @param outputLen The address where the implementation will deposit
 * the length, the number of bytes placed into the output buffer.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtSecureArchiveWriteFinal (
   VtSecureArchiveObject secureArchiveObj,
   unsigned char *output,
   unsigned int bufferSize,
   unsigned int *outputLen
);

/** Initialize the secure archive object for reading. The object should
 * have been created with an impl that supports reading.
 *
 * @param secureArchiveObj The object created and set to read a ZDM message.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtSecureArchiveReadInit (
   VtSecureArchiveObject secureArchiveObj
);

/** Continue reading the secure archive. Currently, due to the archiving
 * and compressing implementations used, the secure archive impls must
 * buffer the entire archive (either in memory or a file), so it's
 * not able to support streaming of output as it gets input. This
 * means that WriteUpdate has separate input and output phases.
 * <p>When reading this means that the caller must first stream in the
 * entire archive data (or else specify the input stream using the
 * VtSecureArchiveParamInputStream param), but no output will be
 * generated. Then the caller can specify a current entry and begin
 * streaming out the entry or index data using ReadUpdate calls.
 * This can then be repeated to write out other entries in the archive.
 * <p>During the output phase it is possible that there is no outputData
 * if the entry or index data was empty. In this case it is possible that
 * ReadUpdate will return 0 even if it's passed a NULL output buffer, so
 * you shouldn't assume that you'll always get a VT_ERROR_BUFFER_TOO_SMALL
 * error in that case.
 *
 * @param secureArchiveObj The secure archive object built to read.
 * @param input The current part of the secure archive data.
 * @param inputLen The length, in bytes, of the secure archive data.
 * @param bytesRead The address where the routine will deposit the
 * number of bytes of the message actually read. If 0, call the Update
 * function again with the same data. If messageLen, the function read
 * all the bytes, the next call should pass in the next bytes in the
 * message.
 * @param output The output buffer, where the resulting content
 * data will be deposited.
 * @param bufferSize The size, in bytes, of the output buffer.
 * @param outputLen The address where the function will deposit
 * the length, the number of bytes placed into the output buffer.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtSecureArchiveReadUpdate (
   VtSecureArchiveObject secureArchiveObj,
   unsigned char *input,
   unsigned int inputLen,
   unsigned int *bytesRead,
   unsigned char *output,
   unsigned int bufferSize,
   unsigned int *outputLen
);

/** Finish reading from a secure archive object.
 * Only call this after you've finished reading all entry data
 * from the object.
 *
 * @param secureArchiveObj The secure archive object built to read.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtSecureArchiveReadFinal (
   VtSecureArchiveObject secureArchiveObj
);

/** Verify all signatures in the current secure archive entry. If there
 * is more than one signer, this function will try to verify all signatures
 * and all signer certs. If any element does not verify, this function
 * sets the verifyResult to did not verify.
 * <p>Before calling this function the application must finish reading
 * the current entry. That is, do not call this function until after a
 * series of ReadUpdate calls has returned the entire contents of the
 * current entry.
 * <p>The caller passes in policy, storage and transport contexts the
 * function will use to help it find certificates it needs to chain a
 * leaf cert. The function may download district parameters to obtain
 * certs.
 * <p>The caller also passes in a certVerifyCtx, which this function
 * will use to verify any untrusted certs it encounters. The caller
 * must also pass in the appropriate associated info (verifyCtxInfo)
 * for the particular certVerifyCtx. That is the info the specific ctx
 * needs to verify a cert. This associated info will be applied to each
 * leaf cert the function verifies.
 *
 * @param secureArchiveObj The secure archive object built to read
 * @param policyCtx A policyCtx to use if necessary.
 * @param storageCtx A storageCtx to use to help find any certs needed
 * to verify signatures or other certs.
 * @param transportCtx A transportCtx to use if necessary.
 * @param certVerifyCtx The certVerifyCtx the function will use when
 * verifying untrusted certs in a chain.
 * @param verifyCtxInfo The info the certVerifyCtx needs to help it
 * verify. This info will be applied to leaf certs.
 * @param vfyFailList Currently unused, reserved for future
 * implementations.
 * @param verifyResult The address where this function will deposit the
 * result of the verify, one for verify, zero for not verified.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtSecureArchiveVerify (
   VtSecureArchiveObject secureArchiveObj,
   VtPolicyCtx policyCtx,
   VtStorageCtx storageCtx,
   VtTransportCtx transportCtx,
   VtCertVerifyCtx certVerifyCtx,
   Pointer verifyCtxInfo,
   VtVerifyFailureList vfyFailList,
   unsigned int *verifyResult
);

/*@}*/

/*=========================================================*/
/*                                                         */
/* Stream Functions                                        */
/*                                                         */
/*=========================================================*/

/**
 * \defgroup Stream Functions
 */

/*@{*/

/** The Stream object.
 * <p>Note that the object is a pointer type.
 */
typedef struct VtStreamObjectDef* VtStreamObject;

/** A type representing a size or position of a stream.
 * TODO: This should probably be defined in environment.h so it can
 * conditionally be defined to use _int64 where available, but
 * currently the public header files don't include environment.h
 */
typedef long VtStreamSize;

/** This constant is the maximum allowed size of a stream. This can
 * be passed as the end position argument to VtStreamCopySection to
 * copy up until the end of the input stream.
 */
#define VT_STREAM_SIZE_MAX 0x7fffffff

/** This value is the open mode for a stream that has not been opened yet.
 * It is currently only used internally in the toolkit.
 */
#define VT_STREAM_CLOSED            0x00

/** Use this value as the open mode field in the associated info for
 * a stream. This will be used in conjunction with the
 * VT_STREAM_OPEN_ON_ACCESS flag. If the first access is a write
 * operation then the stream will be opened in write mode. If the
 * first access is a read operation, then it will be opened in read
 * mode.
 */
#define VT_STREAM_OPEN_DEFAULT      0x00

/** Use this value either as the mode argument to VtStreamOpen or as the
 * openMode field in the associated info for a stream. The stream will be
 * opened for reading. If you try to write to a stream opened in read mode,
 * then you'll get a VT_ERROR_STREAM_NOT_WRITABLE error.
 */
#define VT_STREAM_OPEN_READ         0x01

/** Use this value either as the mode argument to VtStreamOpen or as the
 * openMode field in the associated info for a stream. The stream will be
 * opened for writing. If you try to read from a stream opened in write mode,
 * then you'll get a VT_ERROR_STREAM_NOT_READABLE error. A stream opened in
 * write mode will overwrite the previous contents of the stream.
 */
#define VT_STREAM_OPEN_WRITE        0x02

/** Use this value either as the mode argument to VtStreamOpen or as the
 * openMode field in the associated info for a stream. The stream will be
 * opened for reading and writing. The previous contents are not overwritten.
 */
#define VT_STREAM_OPEN_READ_WRITE   (VT_STREAM_OPEN_READ | VT_STREAM_OPEN_WRITE)

/** Use this value either as the mode argument to VtStreamOpen or as the
 * openMode field in the associated info for a file-based stream. This mode is
 * only used with the VtStreamImplFile implementation. Instead of opening an
 * existing file it will create a temporary file in an appropriate directory
 * (e.g. /tmp on Unix) and open the file such that the temp file will
 * automatically be deleted when the stream is closed or the program
 * terminates. The stream will be opened for reading and writing.
 */
#define VT_STREAM_OPEN_TEMPORARY    (VT_STREAM_OPEN_READ_WRITE | 0x04)

/** Use this value for the mode argument to VtStreamSetPosition to set the
 * position relative to the start of stream.
 */
#define VT_STREAM_FROM_START    1

/** Use this value for the mode argument to VtStreamSetPosition to set the
 * position relative to the end of stream. The position argument should be
 * negative to set the position backwards from the end of the stream.
 */
#define VT_STREAM_FROM_END      2

/** Use this value for the mode argument to VtStreamSetPosition to set the
 * position relative to the current position of stream.
 */
#define VT_STREAM_FROM_POSITION 3

/** The function VtCreateStreamObject builds a new stream object.
 * The VtStreamImpl argument defines what type of stream to create.
 * This typedef defines what a VtStreamImpl is. Although a VtStreamImpl
 * is a function pointer, an application should never call one directly,
 * only pass it as an argument to VtCreateStreamObject.
 */
typedef int VT_CALLING_CONV (VtStreamImpl) (VtStreamObject*, Pointer, unsigned int);

/** Create a new stream object. The VtStreamImpl determines how the
 * stream is implemented (e.g. memory-based, file-based).
 * @param libCtx The library context.
 * @param streamImpl The implementation the object will use.
 * @param info The info needed by streamImpl.
 * @param stream Set to the newly create stream object
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtCreateStreamObject(
  VtLibCtx libCtx,
  VtStreamImpl streamImpl,
  Pointer info,
  VtStreamObject* stream
);

/** Destroy a stream object. This frees any resources associated with
 * the stream. If the stream wasn't closed, VtStreamClose will be
 * called automatically. At the end *stream will be set to 0.
 * @param stream The stream object
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtDestroyStreamObject(
  VtStreamObject* stream
);

/** Open the stream. The mode parameter determines if the stream is
 * opened for reading, writing, or both. The initial position of the
 * stream is 0. VtStreamOpen must be called before making any calls
 * to read or write from the stream or to get/set the position.
 * @param stream The stream object
 * @param mode Which mode to open the stream (read, write, read/write)
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtStreamOpen(
  VtStreamObject stream,
  unsigned int mode
);

/** Close the stream. Call this after you're done making any read or
 * write calls on the stream.
 * @param stream The stream object
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtStreamClose(
  VtStreamObject stream
);

/** Returns whether the stream is open or not
 * @param stream The stream object
 * @param isOpen Set to 1 if the stream is open; 0 if not
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtStreamIsOpen(
  VtStreamObject stream,
  int* isOpen
);

/** Read data from the stream into the output buffer. The position
 * in the stream will be advanced by the number of bytes actually read.
 * The stream needs to be open before you can call VtStreamRead or have
 * been created with the VT_STREAM_OPEN_ON_ACCESS flag.
 * @param stream The stream object
 * @param data Pointer to an output buffer to which the data is read
 * @param length Size in bytes of the output buffer
 * @param readLength Number of bytes actually read to the output buffer
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtStreamRead(
  VtStreamObject stream,
  unsigned char* data,
  VtStreamSize length,
  VtStreamSize* readLength
);

/** Write data to the stream from the input buffer. The position in
 * the stream will be advanced by the number of bytes written. The
 * stream needs to be open before you can call VtStreamWrite or have
 * been created with the VT_STREAM_OPEN_ON_ACCESS flag.
 * @param stream The stream object
 * @param data Pointer to the input buffer from which the data is written
 * @param length Size in bytes of the input buffer
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtStreamWrite(
  VtStreamObject stream,
  const unsigned char* data,
  VtStreamSize length
);

/** Returns the current position of the stream. The initial position
 * of a stream is set to 0 when it is first opened via VtStreamOpen.
 * There is a single position used for both read and write operations
 * (i.e. when a stream is opened in read/write mode).
 * @param stream The stream object
 * @param position Set to the current position of the stream
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtStreamGetPosition(
  VtStreamObject stream,
  VtStreamSize* position
);

/** Sets the current position of the stream. The position can be
 * set relative to the start or end of the stream or relative to
 * the current position of the stream depending on the mode
 * parameter. To set the position to an absolute value, set the
 * offset parameter to the new position and use VT_STREAM_FROM_START
 * as the mode parameter.
 * @param stream The stream object
 * @param offset Amount to change the position depending on the mode
 * @param mode Change the position relative to start, end, or position
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtStreamSetPosition(
  VtStreamObject stream,
  VtStreamSize offset,
  int mode
);

/** Get the size of the stream. The stream needs to be open before
 * calling this.
 * @param stream The stream object
 * @param size Set to the size of the stream
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtStreamGetSize(
  VtStreamObject stream,
  VtStreamSize* size
);

/** If supported by the impl, this call returns the memory buffer
 * for the stream. This calls is supported by the VtStreamImplMemory
 * impl, but not by the VtStreamImplFile impl. It is mainly for cases
 * where the caller knows it is using the memory impl and then needs
 * to call some other function with the memory buffer. This call
 * avoids having to allocate a separate buffer into which the stream
 * contents would be read. The stream object retains ownership of the
 * buffer, so the caller should not delete it. Also, the buffer pointer
 * is only guaranteed to remain valid until the next write to the stream.
 * @param stream The stream object to the buffer from
 * @param buffer Set to the pointer to the in-memory buffer
 * @param size Set to the size of the in-memory buffer
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code. In particular it will return
 * VT_ERROR_UNSUPPORTED if the impl does not support an in-memory buffer.
 */
int VT_CALLING_CONV VtStreamGetBuffer(
  VtStreamObject stream,
  unsigned char** buffer,
  VtStreamSize* size
);

/** Copy the contents of one stream to another. The streams don't
 * need to be opened before calling this function; it will open them
 * (and close them) if necessary. If the streams aren't already open
 * the input stream will be opened in read-only mode, and the output
 * stream will be opened in write-only mode. If the streams are already
 * open then the copy operation will work from the current position
 * for each stream. This call (if successful) will change the current
 * position of both the input and output stream if they were
 * already open.
 * @param inputStream The stream object to copy from
 * @param outputStream The stream object to copy to
 * @param copied The number of bytes actually copied (can be NULL)
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtStreamCopy(
  VtStreamObject inputStream,
  VtStreamObject outputStream,
  VtStreamSize* copied
);

/** Copy a portion of one stream to another. This works the same way
 * as VtStreamCopy except that the "start" and "end" arguments are
 * absolute positions from the start of the stream, so the current
 * position of the inputStream (i.e. if it was already open) will be
 * ignored. This call (if successful) will change the current
 * position of both the input and output stream if they were
 * already open. Use a start position of 0 and an end position of
 * VT_STREAM_SIZE_MAX to copy the entire input stream to the output
 * stream.
 * @param inputStream The stream object to copy from
 * @param start The start of the section to copy to the output stream
 * @param end The end of the section to copy to the output stream
 * @param outputStream The stream object to copy to
 * @param copied The number of bytes actually copied (can be NULL)
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtStreamCopySection(
  VtStreamObject inputStream,
  VtStreamSize start,
  VtStreamSize end,
  VtStreamObject outputStream,
  VtStreamSize* copied
);

/** The VtStreamImplMemory will use a memory buffer for the stream data.
 * The memory can either be allocated dynamically by the impl (i.e. when
 * writing to the stream) or the caller can specify a buffer to use as
 * the contents of the stream (i.e. when reading from the stream).
 * <p>The associated info is a pointer to a VtStreamImplMemoryInfo struct.
 * You can also pass a NULL pointer for the associated info. In this case
 * it will default to using a NULL data buffer with the openMode set to
 * VT_STREAM_OPEN_DEFAULT and the VT_STREAM_OPEN_ON_ACCESS flag enabled.
 */
extern VtStreamImpl VtStreamImplMemory;

/** This value can be specified in the flags field of the
 * VtStreamImplMemoryInfo struct passed to the memory-based stream
 * as the associated info. It indicates that ownership of the buffer
 * passed in the info as the "data" field should be assumed by the impl.
 * If this flags is set, then the data buffer must have been allocated
 * using the same library context as the stream.
 */
#define VT_STREAM_IMPL_OWNS_DATA      0x0001

/** This value can be specified in the flags field of the associated
 * info for a stream. It specifies that the stream should be opened
 * automatically by the create call. If this is set, then the stream
 * will be opened with the mode specified in the "openMode" field of
 * the info.
 */
#define VT_STREAM_OPEN_ON_CREATE      0x0002

/** This value can be specified in the flags field of the associated
 * info for a stream. It specifies that the stream should be opened
 * when it is first accessed during a read or write operation. If the
 * "openMode" field is VT_STREAM_OPEN_DEFAULT, then the stream will
 * be opened with the appropriate mode for the operation
 * (i.e. VT_STREAM_OPEN_READ for a read; VT_STREAM_OPEN_WRITE for a write).
 */ 
#define VT_STREAM_OPEN_ON_ACCESS      0x0004

/** This struct is the associated info for a memory-based stream created
 * with the VtStreamImplMemory impl.
 * <p>The "data" field specifies a data buffer that contains the contents
 * of the stream. The "length" field is the size of the data buffer.
 * These fields would typically be used with a read-only stream.
 * If the VT_STREAM_IMPL_OWNS_DATA flag is set in the flags field then
 * the stream object assumes ownership of the buffer and will free it
 * using the stream library context when the stream is destroyed. If
 * the data field is set to NULL, then the stream will allocate the
 * buffer itself and expand it as necessary as data is written to the
 * stream.
 * <p>The openMode field specifies the mode in which to open the stream
 * when used in conjunction with the VT_STREAM_OPEN_ON_CREATE and
 * VT_STREAM_OPEN_ON_ACCESS flags in the "flags" field.
 * <p>The flags field can include any of the VT_STREAM_IMPL_OWNS_DATA,
 * VT_STREAM_OPEN_ON_CREATE, and VT_STREAM_OPEN_ON_ACCESS or'd
 * together.
 */
typedef struct
{
  unsigned char*  data;
  VtStreamSize    length;
  unsigned int    openMode;
  unsigned int    flags;
} VtStreamImplMemoryInfo;

/** The VtStreamImplFile impl will use a file for the stream data. The
 * name/path for the file is specified in the associated info.
 * <p>Ths associated info is a pointer to a VtStreamImplFileInfo struct.
 */
extern VtStreamImpl VtStreamImplFile;

/** This is the associated info for file-based stream created with the
 * VtStreamImplFile impl.
 * <p>The fileCtx field is the file context to use for file operations.
 * If it's NULL, then the stream object will look for a file context
 * stored in the library context.
 * <p>The fileName field is the name or path of the file to be used for
 * the stream. If the file name is not a full path, then the name will be
 * resolved relative to the current directory. If the stream is to be
 * opened with the VT_STREAM_OPEN_TEMPORARY mode, then the file name is
 * instead a prefix that will be used to create a unique name in a
 * platform-dependent temp file directory. Only the first 3 letters of
 * the prefix are guaranteed to be used in the temp file name.
 * <p>The openMode field specifies the mode in which to open the stream
 * when used in conjunction with the VT_STREAM_OPEN_ON_CREATE and
 * VT_STREAM_OPEN_ON_ACCESS flags in the "flags" field.
 * <p>The flags field can include either the VT_STREAM_OPEN_ON_CREATE or
 * VT_STREAM_OPEN_ON_ACCESS flags (or 0).
 */
typedef struct
{
  VtFileCtx       fileCtx;
  unsigned char*  fileName;
  unsigned int    openMode;
  unsigned int    flags;
} VtStreamImplFileInfo;

/** For use with buffer type params, set the info to this
 * value to specify to use memory, if any internal buffering is needed.
 */
#define VT_BUFFER_TYPE_MEMORY   1

/** For use with buffer type params, set the info to this
 * value to specify to use temp files, if any internal buffering is needed.
 */
#define VT_BUFFER_TYPE_FILE     2

/** For use with buffer type params, set the info to this
 * value to specify to use the specified stream for internal buffering.
 */
#define VT_BUFFER_TYPE_STREAM   3

/** The associated info for the various buffer type params
 * (e.g. VtZDMParamBufferType).
 * The bufferType field should be set to one of VT_BUFFER_TYPE_MEMORY,
 * VT_BUFFER_TYPE_FILE or VT_BUFFER_TYPE_STREAM.
 * If the buffer type is VT_BUFFER_TYPE_FILE, then fileCtx can be
 * set to the file context to use to create/open the temp file
 * used for the buffer. If fileCtx is 0, then the file context
 * will be obtained from the library context.
 * If the buffer type is VT_BUFFER_TYPE_STREAM then streamObj field
 * must be set to the stream to read to or write from. For the other
 * buffer types the streamObj field is not used.
 */
typedef struct
{
  int bufferType;
  VtStreamObject streamObj;
  VtFileCtx fileCtx;
} VtBufferTypeInfo;

/*@}*/

/*=========================================================*/
/*                                                         */
/* Storage Functions                                       */
/*                                                         */
/*=========================================================*/

/**
 * \defgroup StorageFuncsGroup Storage Functions
 */

/*@{*/

/** When specifying what kind of entry you're interested in (storing or
 * retrieving), use a VtEntryType.
 */
typedef unsigned int VtEntryType;

/** Used in storage functions, this VtEntryType indicates that the
 * entry of interest is the current district.
 * <pre>
 * <code>
 *    Store
 *      reference: NULL-term UTF-8     entry: district object
 *                 string, DomainName
 *    EntryCount
 *      reference: NULL-term UTF-8
 *                 string, DomainName
 *    Retrieve
 *      reference: NULL-term UTF-8     entry: district object
 *                 string, DomainName
 *    Delete
 *      reference: NULL-term UTF-8
 *                 string, DomainName
 * </code>
 * </pre>
 * <p>For VtStoreEntry, the reference is not allowed to be NULL and the
 * entry can be NULL or a district object. If the entry is NULL or an
 * object that does not contain a QualifiedName, the function will store
 * the info that there is no current district associated with the
 * domain. If there is a QualifiedName in the entry, the function will
 * store that name as the current district associated with the domain.
 * It will also store the validity dates of the district (if the
 * validity dates are not in the district object, the function will
 * return an error).
 * <p>For VtGetEntryCount, the reference is not allowed to be NULL. The
 * count will always be 0 or 1.
 * <p>For VtRetrieveEntry, the reference is not allowed to be NULL and
 * the entry must be a district object not set with a QualifiedName. If
 * it is set with a DomainName, that value must be the same as the
 * reference. The Retrieve function will set the DomainName if
 * necessary and the QualifiedName in the entry. Also, you must supply
 * a pointer to a VtTime struct as the timeOfStore arg, which the
 * function will set.
 * <p>For VtDeleteEntry, the reference is not allowed to be NULL.
 */
#define VT_ENTRY_TYPE_CURRENT_DISTRICT   1

/** Used in storage functions, this VtEntryType indicates that the
 * entry of interest is the district parameter set.
 * <pre>
 * <code>
 *    Store
 *      reference: NULL                entry: district object
 *    EntryCount
 *      reference: district object
 *    Retrieve
 *      reference: NULL                entry: district object
 *    Delete
 *      reference: district object
 * </code>
 * </pre>
 * <p>For VtStoreEntry, the reference must be NULL, the entry is both
 * reference and entry. The entry is not allowed to be NULL and must be
 * a district object set with district parameters.
 * <p>For VtGetEntryCount, the reference is allowed to be NULL. If the
 * reference is not NULL, some providers might require that either no
 * name in the object is set (equivalent to a NULL entry) or that the
 * QualifiedName be set. DomainName only might not be an option.
 * <p>For VtRetrieveEntry, the reference must be NULL, the entry is
 * both reference and entry. The entry is not allowed to be NULL and
 * must not contain district parameters yet. Some providers might
 * require that either no name in the object is set or that the
 * QualifiedName be set. DomainName only might not be an option. Also,
 * you must supply a pointer to a VtTime struct as the timeOfStore arg,
 * which the function will set.
 * <p>For VtDeleteEntry, the reference is allowed to be NULL.
 */
#define VT_ENTRY_TYPE_DISTRICT_PARAMS    2

/** Used in storage functions, this VtEntryType indicates that the
 * entry of interest is an authentication token.
 * <p>An authentication token is a NULL-terminated ASCII string.
 * <pre>
 * <code>
 *    Store
 *      reference: district object     entry: NULL-term ASCII string
 *    EntryCount
 *      reference: district object
 *    Retrieve
 *      reference: district object     entry: NULL-term ASCII string
 *    Delete
 *      reference: district object
 * </code>
 * </pre>
 * <p>For VtStoreEntry, the reference cannot be NULL. Some providers
 * might require that the QualifiedName in the reference be set.
 * <p>For VtGetEntryCount, the reference cannot be NULL. Some providers
 * might require that the QualifiedName in the reference be set.
 * <p>For VtRetrieveEntry, the reference cannot be NULL. Some providers
 * might require that the QualifiedName in the reference be set. The
 * entry argument must be a pointer to a char * argument. The Retrieve
 * function will deposit a pointer to the token. The memory holding the
 * token belongs to the toolkit, do not free or alter it. For example,
 * <pre>
 * <code>
 *    char *token = (char *)0;
 *
 *    status = VtRetrieveEntry (
 *      libCtx, storageCtx, VT_ENTRY_TYPE_AUTH_TOKEN, index,
 *      (Pointer)districtObject, (Pointer)&token, (VtTime *)0);
 * </code>
 * </pre>
 * <p>Also for RetrieveEntry, you can pass a NULL for the timeOfStore
 * arg. If you pass a pointer to a VtTime struct, the function will set
 * all the fields to 0.
 * <p>For VtDeleteEntry, the reference cannot be NULL. Some providers
 * might require that the QualifiedName in the reference be set.
 */
#define VT_ENTRY_TYPE_AUTH_TOKEN         3

/** Used in storage functions, this VtEntryType indicates that the
 * entry of interest is an IBE private key.
 * <pre>
 * <code>
 *    Store
 *      reference: identity object     entry: key object
 *    EntryCount
 *      reference: identity object
 *    Retrieve
 *      reference: identity object     entry: key object
 *    Delete
 *      reference: identity object
 * </code>
 * </pre>
 * <p>For VtStoreEntry, the reference cannot be NULL. Some providers
 * might require the id object to be set with the encoded ID.
 * <p>For VtGetEntryCount, the reference is allowed to be NULL. If it
 * is not NULL, some providers might require the id object to be set
 * with the encoded ID.
 * <p>For VtRetrieveEntry, the reference is allowed to be NULL. If the
 * reference is not NULL, some providers might require the id object to
 * be set with the encoded ID. Also, you can pass a NULL for the
 * timeOfStore arg. If you pass a pointer to a VtTime struct, the
 * function will set all the fields to 0.
 * <p>For VtDeleteEntry, the reference is allowed to be NULL.
 */
#define VT_ENTRY_TYPE_IBE_PRI_KEY        4

/** Used in storage functions, indicate the entry of interest is a
 * signing private key.
 * <pre>
 * <code>
 *    Store
 *      reference: identity object     entry: key object
 *    EntryCount
 *      reference: identity object
 *    Retrieve
 *      reference: identity object     entry: key object
 *    Delete
 *      reference: identity object
 * </code>
 * </pre>
 * <p>For VtStoreEntry, the reference cannot be NULL. Some providers
 * might require the id object to be set with the encoded ID.
 * <p>For VtGetEntryCount, the reference is allowed to be NULL. If it
 * is not NULL, some providers might require the id object to be set
 * with the encoded ID.
 * <p>For VtRetrieveEntry, the reference is allowed to be NULL. If the
 * reference is not NULL, some providers might require the id object to
 * be set with the encoded ID. Also, you can pass a NULL for the
 * timeOfStore arg. If you pass a pointer to a VtTime struct, the
 * function will set all the fields to 0.
 * <p>For VtDeleteEntry, the reference is allowed to be NULL.
 */
#define VT_ENTRY_TYPE_SIGNING_PRI_KEY    5
/** Used in storage functions, indicate the entry of interest is a
 * certificate.
 * <pre>
 * <code>
 *    Store
 *      reference: identity object     entry: cert object
 *    EntryCount
 *      reference: identity object
 *    Retrieve
 *      reference: identity object     entry: cert object
 *    Delete
 *      reference: identity object
 * </code>
 * </pre>
 * <p>For VtStoreEntry, the reference cannot be NULL. Some providers
 * might require the id object to be set with the encoded ID.
 * <p>For VtGetEntryCount, the reference is allowed to be NULL. If it
 * is not NULL, some providers might require the id object to be set
 * with the encoded ID.
 * <p>For VtRetrieveEntry, the reference is allowed to be NULL. If the
 * reference is not NULL, some providers might require the id object to
 * be set with the encoded ID. Also, you can pass a NULL for the
 * timeOfStore arg. If you pass a pointer to a VtTime struct, the
 * function will set all the fields to 0.
 * <p>For VtDeleteEntry, the reference is allowed to be NULL.
 */
#define VT_ENTRY_TYPE_CERTIFICATE        6

/** This function will store the given entry in the specified storage
 * provider. The function will store the entry based on the reference.
 * <p>The entryType is a flag, indicating what is to be stored.
 * <pre>
 * <code>
 *     VT_ENTRY_TYPE_CURRENT_DISTRICT
 *     VT_ENTRY_TYPE_DISTRICT_PARAMS
 *     VT_ENTRY_TYPE_IBE_PRI_KEY
 *     VT_ENTRY_TYPE_SIGNING_PRI_KEY
 *     VT_ENTRY_TYPE_CERTIFICATE
 *     VT_ENTRY_TYPE_AUTH_TOKEN
 * </code>
 * </pre>
 * <p>See the documentation for each VtEntryType to determine what the
 * actual types of the references and entries are. Cast the appropriate
 * element to Pointer before calling the function.
 * <p>If the storage ctx contains more than one provider, this function
 * will store the entry in the first provider that can store the entry.
 *
 * @param libCtx If the storageCtx provided is NULL, the function will
 * try to find one in the libCtx.
 * @param entryType What kind of entry is this?
 * @param reference The reference against which the entry will be
 * stored.
 * @param entry The value to store.
 * @param storageCtx The ctx which contains the provider into which the
 * entry is to be stored.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtStoreEntry (
   VtLibCtx libCtx,
   VtEntryType entryType,
   Pointer reference,
   Pointer entry,
   VtStorageCtx storageCtx
);

/** How many entries of the entryType are associated with the given
 * reference? This function will check all storage providers in the
 * storageCtx.
 * <p>The entryType is a flag, indicating what is to be retrieved.
 * <pre>
 * <code>
 *     VT_ENTRY_TYPE_CURRENT_DISTRICT
 *     VT_ENTRY_TYPE_DISTRICT_PARAMS
 *     VT_ENTRY_TYPE_IBE_PRI_KEY
 *     VT_ENTRY_TYPE_SIGNING_PRI_KEY
 *     VT_ENTRY_TYPE_CERTIFICATE
 *     VT_ENTRY_TYPE_AUTH_TOKEN
 * </code>
 * </pre>
 * <p>See the documentation for each VtEntryType to determine what the
 * actual types of the references are. Cast the appropriate element to
 * Pointer before calling the function.
 * <p>For some VtEntryTypes, the reference can be NULL. If so, this
 * routine will find the count of all such entry types. For example,
 * when finding the count of private keys, if the reference is NULL,
 * the function will return the number of private keys in storage,
 * regardless of the identity to which the key is attached and
 * regardless of the validity period of the key.
 * <p>This function will go to the address given by entryCount and deposit
 * there the number of entries in the list.
 * <p>The entries are indexed 0 (zero) through entryCount ?1.
 * <p>If there are no entries, the count will be 0 and the return value
 * will be 0 (success). It is not an error to find there are no entries
 * in storage corresponding to the reference.
 *
 * @param libCtx If the storageCtx provided is NULL, the function will
 * try to find one in the libCtx.
 * @param storageCtx The ctx containing the providers with the entries
 * to count.
 * @param entryType What kind of entry to search for.
 * @param reference The referance against which the search is made.
 * @param entryCount The address where the function will deposit the
 * resulting count.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtGetEntryCount (
   VtLibCtx libCtx,
   VtStorageCtx storageCtx,
   VtEntryType entryType,
   Pointer reference,
   unsigned int *entryCount
);

/** Retrieve a stored element based on the reference and index.
 * <p>For some VtEntryTypes, there will be at most 1 entry associated
 * with a reference. For example, each identity (the email address and
 * time, for instance) has only one IBE private key (an email address
 * and another time is another identity and has another key).
 * <p>The index is determined based on VtGetEntryCount. For index to be
 * meaningful, the reference should be the same as used (or would be
 * used) in the VtGetEntryCount call.
 * <p>For some VtEntryTypes, the reference can be NULL.
 * <p>Indicate with the entryType what this function is to retrieve. The
 * function will get the entry of the given index, with respect to the
 * reference, in the specified storage provider.
 * <p>The entryType is a flag, indicating what is to be stored.
 * <pre>
 * <code>
 *     VT_ENTRY_TYPE_CURRENT_DISTRICT
 *     VT_ENTRY_TYPE_DISTRICT_PARAMS
 *     VT_ENTRY_TYPE_IBE_PRI_KEY
 *     VT_ENTRY_TYPE_SIGNING_PRI_KEY
 *     VT_ENTRY_TYPE_CERTIFICATE
 *     VT_ENTRY_TYPE_AUTH_TOKEN
 * </code>
 * </pre>
 * <p>See the documentation for each VtEntryType to determine what the
 * actual types of the references and entries are. Cast the appropriate
 * element to Pointer before calling the function.
 * <p>For CURRENT_DISTRICT and DISTRICT_PARAMS, entries will be stored
 * along with the time they were stored. When retrieving such entries,
 * the caller also supplies a pointer to an existing VtTime struct. The
 * function will fill it with the time the entry retrieved was stored.
 * For other entryTypes, the time the entry is stored is not stored, so
 * there is no way to get the timeOfStore, so pass a NULL for that
 * argument. If you pass a pointer to a VtTime, rather than NULL, the
 * routine will set all the fields of that struct to 0.
 *
 * @param libCtx If the storageCtx provided is NULL, the function will
 * try to find one in the libCtx.
 * @param storageCtx The ctx containing the providers with the entries
 * to find.
 * @param entryType What kind of entry to search for.
 * @param index The index into the storage context of the entry to
 * return.
 * @param reference The referance against which the search is made.
 * @param entry Where the function will place the result. It can be a
 * created but empty object or an address where the routine will
 * deposit a string.
 * @param timeOfStore If the entryType is
 * VT_ENTRY_TYPE_CURRENT_DISTRICT or VT_ENTRY_TYPE_DISTRICT_PARAMS,
 * this is the address where the function will deposit the time the
 * entry retrieved was stored. For other entryTypes, this argument can
 * be NULL, but if not, the function will set all fields to 0.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtRetrieveEntry (
   VtLibCtx libCtx,
   VtStorageCtx storageCtx,
   VtEntryType entryType,
   unsigned int index,
   Pointer reference,
   Pointer entry,
   VtTime *timeOfStore
);

/** Take the given entry out of storage. This function will go to the
 * storage context specified, find the entry of the given entryType
 * belonging to the given reference and index, and delete it from
 * storage.
 * <p>If this function cannot find the entry in storage, there's
 * nothing to delete. Although the function had not actually deleted
 * the entry, after the function, the entry is not there. The function
 * will return 0 (success).
 * <p>For some VtEntryTypes, there will be at most 1 entry associated
 * with a reference. For example, each identity (the email address and
 * time, for instance) has only one IBE private key (an email address
 * and another time is another identity and has another key).
 * <p>The index is determined based on VtGetEntryCount. For index to be
 * meaningful, the reference should be the same as used (or would be
 * used) in the VtGetEntryCount call.
 * <p>For some VtEntryTypes, the reference can be NULL.
 * <p>The entryType is a flag, indicating what is to be deleted.
 * <pre>
 * <code>
 *     VT_ENTRY_TYPE_CURRENT_DISTRICT
 *     VT_ENTRY_TYPE_DISTRICT_PARAMS
 *     VT_ENTRY_TYPE_IBE_PRI_KEY
 *     VT_ENTRY_TYPE_SIGNING_PRI_KEY
 *     VT_ENTRY_TYPE_CERTIFICATE
 *     VT_ENTRY_TYPE_AUTH_TOKEN
 * </code>
 * </pre>
 * <p>See the documentation for each VtEntryType to determine what the
 * actual types of the references are. Cast the appropriate element to
 * Pointer before calling the function.
 *
 * @param libCtx If the storageCtx provided is NULL, the function will
 * try to find one in the libCtx.
 * @param storageCtx The provider containing the entry to delete.
 * @param entryType What kind of entry to search for.
 * @param reference The referance against which the search is made.
 * @param index The index of the entry to delete.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtDeleteEntry (
   VtLibCtx libCtx,
   VtStorageCtx storageCtx,
   VtEntryType entryType,
   Pointer reference,
   unsigned int index
);

/*@}*/

/*=========================================================*/
/*                                                         */
/* Cert Verification Functions                             */
/*                                                         */
/*=========================================================*/

/**
 * \defgroup CertVerifyGroup Cert Verification Functions
 */

/*@{*/

/** Verify a certificate. This function will use the verifyCtx to check
 * various elements of the cert, such as time and key usage, along with
 * the signature. Which elements are checked is dependent on the
 * verifyCtx chosen. See the documentation for the verifyCtx chosen to
 * determine what verifyInfo is needed and in what format it is
 * supplied.
 * <p>The caller passes a DerCoder array. This will contain public key
 * and signature algorithm DerCoders. In order to verify a cert, the
 * function will probably need to verify a signature. That means the
 * routine will need an algorithm object built to verify and a key
 * object containing the public key. The public key needed to verify a
 * cert is almost certainly found in another cert. Hence the function
 * will need to extract that public key and build a key object. The key
 * in the cert will be DER encoded, so the DerCoders will be used to
 * "convert" the DER encoded key into a key object. The algorithm used
 * to sign will be represented in the cert as an algorithm ID, so the
 * function will need to be able to "convert" the algId into an
 * algorithm object. A DerCoder for the algorithm will do that.
 * <p>The application will build a DerCoder array containing DerCoders
 * for all the algorithms it is willing to support and pass it in to
 * this routine. Alternatively, the application can set the libCtx with
 * the DerCoders and pass NULL to this function. If passed a NULL
 * DerCoder array, this function will use the list it finds in the
 * libCtx.
 * <p>The cert verify context will almost certainly need other certs
 * (CA and/or root certs) in order to verify the cert. The app can pass
 * in lists of trusted and untrusted certs to the CertVerifyCtx (using
 * VtSetCertVerifyParam with VtCertVerifyParamTrustedCerts and
 * VtCertVerifyParamUntrustedCerts) or it can optionally pass in lists
 * to this call (or both). If the app calls a cert trusted, this
 * function will not check its signature. When using a trustedCert, the
 * function will simply check that the validity dates work and the cert
 * is allowed to perform the function requested.
 * <p>The function will set the unsigned int at the address given by
 * verifyResult to either 1 (verifies) or 0 (does not verify).
 *
 * @param certToVerify Pass the cert to verify as a Cert object.
 * @param verifyCtx The CertVerifyCtx this function will use to verify
 * the cert.
 * @param verifyInfo The info the particular verifyCtx needs to verify
 * the cert. This will likely be a struct containing several elements
 * of information.
 * @param derCoders A list of DerCoders the function can use to
 * decode the public keys inside a cert, if it needs to extract a
 * public key from a cert (such as a CA cert higher up in the chain
 * from the certToVerify) in order to perform signature verification.
 * @param derCoderCount The number of DerCoders in the list.
 * @param trustedCerts A list of certs the app deems trusted. The
 * function might look through this list to find a CA cert needed to
 * verify another cert.
 * @param untrustedCerts A list of certs the app does not trust yet.
 * The function might look through this list to find a CA cert needed
 * to verify another cert.
 * @param storageCtx Currently unused, may be used in future versions
 * of the toolkit.
 * @param vfyFailList Currently unused, reserved for future
 * implementations.
 * @param verifyResult The address where this function will deposit the
 * result of the verify, one for verify, zero for not verified.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtVerifyCert (
   VtCertObject certToVerify,
   VtCertVerifyCtx verifyCtx,
   Pointer verifyInfo,
   VtDerCoder **derCoders,
   unsigned int derCoderCount,
   VtCertObjectList *trustedCerts,
   VtCertObjectList *untrustedCerts,
   VtStorageCtx storageCtx,
   VtVerifyFailureList vfyFailList,
   unsigned int *verifyResult
);

/*@}*/

/*=========================================================*/
/*                                                         */
/* IBE Protocol Functions                                  */
/*                                                         */
/*=========================================================*/

/**
 * \defgroup IBEProtocolGroup IBE Protocol Functions */

/*@{*/

/** Determine the district for the given identity. The function will use
 * the policy context provided (if one is provided) and normal toolkit
 * determination algorithms to determine to which district the identity
 * belongs. It will return the district name in the given buffer. This
 * routine will go to the address given by districtNameLen and deposit
 * the length of the output (the number of bytes placed into the
 * districtName buffer). If the buffer is not big enough, this function
 * will return the "BUFFER_TOO_SMALL" error and set the unsigned int at
 * districtNameLen to the needed size.
 * <p>The return will be the UTF-8 encoding of the name.
 * <p>If no district could be found, this function will return an error.
 * <p>The district name might be in the storage provider, if so, the
 * function will not try to contact actual districts.
 * <p>If the district name is not in storage, the function will
 * generate candidate district names and try to contact them, just to
 * see if they exist. In contacting them, the districts send the
 * parameters anyway, so this function will store them in the storage
 * provider. If more than one provider is supplied in the providerCtx,
 * this function will store the params in the first provider in the
 * provider object that can store them.
 * <p>VT_ERROR_ASYNC_DOWNLOAD: This function may return the
 * "asynchronous download" error. This error code indicates that the
 * information could not be downloaded because further caller
 * operations are required. These operations are defined by the
 * transport context. In other words, the function could not download
 * the data but will be able to download it after the caller performs
 * some other operations defined by the transport provider. If this
 * function returns ASYNC_DOWNLOAD, execute the special transport
 * operations, then call this routine again.
 *
 * @param idObj The identity for which the district is requested.
 * @param policyCtx The context containing the policy variations.
 * @param storageCtx The storage provider to search and into which
 * parameters will be stored if necessary.
 * @param transportCtx The transport context to use when contacting
 * districts, if necessary.
 * @param districtName The buffer into which this function will place
 * the UTF-8 encoded district name.
 * @param bufferSize The size, in bytes, of the output buffer.
 * @param districtNameLen The address where the routine will deposit the
 * name length (in bytes).
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtDetermineDistrict (
   VtIdentityObject idObj,
   VtPolicyCtx policyCtx,
   VtStorageCtx storageCtx,
   VtTransportCtx transportCtx,
   unsigned char *districtName,
   unsigned int bufferSize,
   unsigned int *districtNameLen
);

/** Obtain the IBE parameters for the given district. The function will
 * look in all storage providers given. If not in storage, the function
 * will contact districts using the transport provider. If the function
 * downloads parameters, it will store them using the storage context.
 * <p>The parameters will be stored inside the district object. To get
 * them out, call GetDistrictInfo using
 * VtDistrictGetTypeParameters.
 * <p>VT_ERROR_ASYNC_DOWNLOAD: This function may return the
 * "asynchronous download" error. This error code indicates that the
 * information could not be downloaded because further caller
 * operations are required. These operations are defined by the
 * transport context. In other words, the function could not download
 * the data but will be able to download it after the caller performs
 * some other operations defined by the transport provider. If this
 * function returns ASYNC_DOWNLOAD, execute the special transport
 * operations, then call this routine again.
 *
 * @param distObj The district from which parameters are requested.
 * @param policyCtx The context containing the policy variations.
 * @param storageCtx The storage provider to search and into which
 * parameters will be stored if necessary.
 * @param transportCtx The transport context to use when contacting
 * districts, if necessary.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtObtainIBEParams (
   VtDistrictObject distObj,
   VtPolicyCtx policyCtx,
   VtStorageCtx storageCtx,
   VtTransportCtx transportCtx
);

/** Build an IBE public key object based on an identity. The caller
 * passes in the identity and a created but empty KeyObject, the routine
 * will set the object with the IBE public key data.
 * <p>The key might be in storage (represented by the storage provider).
 * If so, the function will return that key. Otherwise it will download
 * the IBE parameters from the district/key server using the given
 * transport provider. The function will store the parameters in the
 * provider. If the storage provider object contains more than one
 * provider, the function will store the info in the first provider
 * that can store it.
 * <p>The version indicates which version of the IBCS #2 standard to use
 * when encoding. Currently available are
 * <code>
 * <pre>
 *    VT_ENCODE_IBCS_2_V_1
 *    VT_ENCODE_IBCS_2_V_2
 *    VT_ENCODE_IBCS_2_V_DISTRICT
 * </pre>
 * </code>
 * <p>VT_ERROR_ASYNC_DOWNLOAD: This function may return the
 * "asynchronous download" error. This error code indicates that the
 * information could not be downloaded because further caller
 * operations are required. These operations are defined by the
 * transport context. In other words, the function could not download
 * the data but will be able to download it after the caller performs
 * some other operations defined by the transport provider. If this
 * function returns ASYNC_DOWNLOAD, execute the special transport
 * operations, then call this routine again.
 *
 * @param idObj The identity for which the public key is to be built.
 * @param encodeVersion The IBCS #2 encoding version to use.
 * @param policyCtx The policy context to use, if needed.
 * @param storageCtx The storage provider to search and into which
 * the key will be stored if necessary.
 * @param transportCtx The transport context to use when contacting
 * districts, if necessary.
 * @param ibePubKey The created but empty object into which this
 * routine will set the public key.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtBuildIBEPublicKey (
   VtIdentityObject idObj,
   unsigned int encodeVersion,
   VtPolicyCtx policyCtx,
   VtStorageCtx storageCtx,
   VtTransportCtx transportCtx,
   VtKeyObject ibePubKey
);

/** When you have an identity, but not the IBE private key, signing
 * private key or the cert for that identity, call this routine. The
 * caller passes in the identity and created but empty Key and Cert
 * objects, the routine will set the objects with the IBE private key,
 * the signing private key and the cert.
 * <p>The keys and cert might be in storage (represented by the storage
 * provider). If so, the function will return those keys and cert.
 * <p>If the function cannot find the keys and cert in storage, it will
 * generate a new DSA key pair, build a cert request, and download the
 * IBE private key and the cert from the district/key server using the
 * given transport provider. It is the responsibility of the storage
 * provider or the transport provider along with the district/key server
 * to authenticate the identity. When generating a new key pair, the
 * function will use the parameter set in the district parameter
 * extensions, if the district parameters contain the "dsa-parameters"
 * extension. If the district does not have a parameter set, the
 * function will generate new system parameters.
 * <p>If the function downloads the key from the server, and if a
 * storage context is available, it will call on that storage ctx to
 * store the newly created private key, the returned cert, and the
 * returned IBE private key. If the storage context object contains
 * more than one provider, the function will store each element in the
 * first provider that can store it.
 * <p>If a NULL transport context is given, and if there is no
 * transport context in the libCtx, this function will look only in
 * storage. If the elements are not in storage, the function will not
 * generate new values and download, but will return
 * VT_ERROR_NO_TRANSPORT_CTX error.
 * <p>If a NULL storage context is given, and if there is no storage
 * context in the libCtx, this function will generate a new key pair
 * and download the values. Nothing, of course, will be stored.
 * <p>It is also possible to instruct this function to look only in
 * storage, to never try to download, by setting the input arg flag to
 * the appropriate value.
 * <p>If the flag arg is 0 (VT_OBTAIN_KEYS_FLAG_STORAGE_AND_DOWNLOAD),
 * the function will execute the default behavior (look in storage and
 * if not there, download. Otherwise, the flag arg should be the OR of
 * all the VT_OBTAIN_KEYS_FLAG_ values which define non-default
 * behavior. Currently, there are only three possible non-default
 * values:
 * <pre>
 * <code>
 *     VT_OBTAIN_KEYS_FLAG_STORAGE_ONLY
 *     VT_OBTAIN_KEYS_FLAG_DOWNLOAD_ONLY
 *     VT_OBTAIN_KEYS_FLAG_DO_NOT_STORE
 * </code>
 * </pre>
 * <p>If you set the flag to contain the bit for STORAGE_ONLY, the
 * function will try to find the values in storage, but if they are not
 * there, it will not generate a new key pair and download.
 * <p>If you set the flag to contain the bit for DOWNLOAD_ONLY, the
 * function will not search the storage providers, but will generate a
 * new signing key pair and cert request, then download the values and
 * store them (unless the DO_NOT_STORE bit is set).
 * <p>If you set the flag to contain the bit for DO_NOT_STORE, the
 * function will not store any element it downloads.
 * <p>If you want to use your own system parameters (you don't want
 * this function to generate new params, for example), you should call
 * this function with the VT_OBTAIN_KEYS_FLAG_STORAGE_ONLY set in the
 * flag argument. If the function succeeds, you're done. If not use
 * your own system params to generate a DSA key pair, then build a cert
 * request and call VtDownloadIBEPrivateKeyAndCert.
 * <p>If the caller passes a NULL object for the ibe private key,
 * signing key, or cert, the function will not obtain (nor store) those
 * values. For example, if you pass a NULL ibePriKey, and a non-NULL
 * signingKey and cert, the function will obtain only the signing key
 * and cert. If the function had to generate a new DSA key pair and
 * download the cert, it will store only the private key and cert, not
 * an IBE private key that might have been downloaded while the cert
 * was downloaded.
 * <p>NOTE!!! If this function finds in storage a signing private key but
 * no cert, or a cert but no signing private key, it will generate a
 * new pair and download a new cert. It will store the new key and
 * cert, which will likely overwrite the old one (whichever of the two
 * was in storage). If one of the two elements is missing and if no
 * transport ctx is provided (or found in the libCtx), the function will
 * not return the element that is in storage.
 * <p>VT_ERROR_ASYNC_DOWNLOAD: This function may return the
 * "asynchronous download" error. This error code indicates that the
 * information could not be downloaded because further caller
 * operations are required. These operations are defined by the
 * transport context. In other words, the function could not download
 * the data but will be able to download it after the caller performs
 * some other operations defined by the transport provider. If this
 * function returns ASYNC_DOWNLOAD, execute the special transport
 * operations, then call this routine again.
 *
 * @param identity The identity for which the IBE private key, signing
 * private key, and cert are requested.
 * @param random A random object to use if generating a new key pair.
 * @param flag If 0, the function will perform default behavior.
 * Otherwise, set it to the OR of the VT_OBTAIN_KEYS_FLAG_ values which
 * define non-default behavior.
 * @param policyCtx The policy context to use, if needed.
 * @param storageCtx Contains the storage providers in which the
 * function will search and, if elements are downloaded, into which the
 * keys and cert will be stored.
 * @param transportCtx The transport context to use when contacting
 * districts, if necessary.
 * @param signingKey The created but empty object which this routine
 * will set with the private signing key, either generated or retrieved
 * from storage.
 * @param signingCert The created but empty object which this routine
 * will set with the cert, either downloaded or retrieved from storage.
 * @param ibePriKey The created but empty object which this routine
 * will set with the IBE private key, either downloaded or retrieved
 * from storage.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtObtainPrivateKeysAndCert (
   VtIdentityObject identity,
   VtRandomObject random,
   unsigned int flag,
   VtPolicyCtx policyCtx,
   VtStorageCtx storageCtx,
   VtTransportCtx transportCtx,
   VtKeyObject signingKey,
   VtCertObject signingCert,
   VtKeyObject ibePriKey
);

/** For use with VtObtainPrivateKeysAndCert. Set the flag argument to
 * this value to let the function know that it should look in storage
 * for the keys and cert, and if it doesn't find them there, try to
 * download them. This is the default behavior.
 */
#define VT_OBTAIN_KEYS_FLAG_STORAGE_AND_DOWNLOAD  0x0000
/** For use with VtObtainPrivateKeysAndCert. Set the flag argument with
 * this bit to let the function know that it should look in storage
 * only for the keys and cert.
 */
#define VT_OBTAIN_KEYS_FLAG_STORAGE_ONLY          0x0001
/** For use with VtObtainPrivateKeysAndCert. Set the flag argument with
 * this bit to let the function know that it should not look in storage
 * to find values, but should download any elements. The function will
 * store anything it downloads.
 */
#define VT_OBTAIN_KEYS_FLAG_DOWNLOAD_ONLY         0x0002
/** For use with VtObtainPrivateKeysAndCert. This flag must be used along
 * with one of the flags for download using a bitwise OR operation. It can't
 * be used with VT_OBTAIN_KEYS_FLAG_STORAGE_ONLY. This flag tells the 
 * VtObtainPrivateKeysAndCert function not to store any data that it 
 * has downloads. This flag has no effect if the data was not downloaded
 * from the network. Note that "data" means keys and certs. It doesn't affect
 * anything else downloaded over network, for ex. public parameters.
 * this bit should be ORed with one of the flags above.
 */
#define VT_OBTAIN_KEYS_FLAG_DO_NOT_STORE          0x0004

/** This is a mask to help determine if some inappropriate bits are set
 * in an ObtainPrivateKeysAndCert flag.
 */
#define VT_OBTAIN_KEYS_FLAG_MASK \
    ~(VT_OBTAIN_KEYS_FLAG_STORAGE_ONLY | \
      VT_OBTAIN_KEYS_FLAG_DOWNLOAD_ONLY | \
      VT_OBTAIN_KEYS_FLAG_DO_NOT_STORE)

/** This function will use the policy and transport contexts to
 * download the IBE private key and the signing cert. It will NOT try
 * to find the elements in storage, it will always download. The
 * storage provider passed in might be used for asynchronous download.
 * <p>The caller passes in the identity and cert request object, along
 * with created but empty Key and Cert objects, the routine will set
 * the empty objects with the IBE private key and cert.
 * <p>This function does not store the returned IBE private key nor
 * signing cert (although the asynchronous download might). After
 * downloading, if you want to make sure the key and cert are stored,
 * you should call VtStoreEntry explicitly for each of the elements.
 * <p>If the caller passes a NULL object for the IBE private key or
 * cert, the function will not obtain those values. For example, if you
 * pass a NULL ibePriKey, and a non-NULL cert (and cert request), the
 * function will obtain only the cert. (If both are NULL, or if the
 * cert is not NULL and the cert request is NULL, the function returns
 * an error.)
 * <p>VT_ERROR_ASYNC_DOWNLOAD: This function may return the
 * "asynchronous download" error. This error code indicates that the
 * information could not be downloaded because further caller
 * operations are required. These operations are defined by the
 * transport context. In other words, the function could not download
 * the data but will be able to download it after the caller performs
 * some other operations defined by the transport provider. If this
 * function returns ASYNC_DOWNLOAD, execute the special transport
 * operations, then call this routine again.
 *
 * @param idObj The identity for which the IBE private key and cert are
 * requested.
 * @param policyCtx The policy context to use, if needed.
 * @param storageCtx Available for use in asynchronous download, if
 * necessary.
 * @param transportCtx The transport context to use when contacting
 * districts, if necessary.
 * @param certReq The object built with the cert request to be used to
 * obtain the cert.
 * @param signingCert The created but empty object which this routine
 * will set with the downloaded cert.
 * @param ibePriKey The created but empty object which this routine
 * will set with the downloaded IBE private key.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtDownloadIBEPrivateKeyAndCert (
   VtIdentityObject idObj,
   VtPolicyCtx policyCtx,
   VtStorageCtx storageCtx,
   VtTransportCtx transportCtx,
   VtCertRequestObject certReq,
   VtCertObject signingCert,
   VtKeyObject ibePriKey
);

/*@}*/

/*=========================================================*/
/*                                                         */
/* Surrender Callback                                      */
/*                                                         */
/*=========================================================*/

/**
 * \defgroup SurrenderCallback Surrender Callback
 */

/*@{*/

/** Set the cert request object with a surrender ctx.
 * <p>The surrender function will be called when performing any sign
 * operation. That is, if you set a cert request object with a
 * surrender ctx, the toolkit will not surrender when creating the cert
 * request per se, but will call the Surrender function when creating a
 * signature or verifying a signature. In creating a cert request, the
 * toolkit will likely test the key pair along with signing the cert
 * request. That means the Surrender function will be called during two
 * calls to signing and one call to verifying.
 * <p>The data associated with VtCertRequestParamSurrenderCallback is a
 * pointer to a VtSurrenderCallback struct.
 */
extern VtCertRequestParam VtCertRequestParamSurrenderCallback;

/** Set the P7 object with a surrender ctx.
 * <p>The data associated with VtPkcs7ParamSurrenderCallback is a pointer to
 * a VtSurrenderCallback struct.
 */
extern VtPkcs7Param VtPkcs7ParamSurrenderCallback;

/** Set the RecipientInfo object with a surrender ctx.
 * <p>The data associated with VtRecipientInfoParamSurrenderCallback is
 * a pointer to a VtSurrenderCallback struct.
 */
extern VtRecipientInfoParam VtRecipientInfoParamSurrenderCallback;

/** Set the SecureMail object with a surrender ctx.
 * <p>The data associated with VtSecureMailParamSurrenderCallback is a 
 * pointer to a VtSurrenderCallback struct.
 */
extern VtSecureMailParam VtSecureMailParamSurrenderCallback;

/** Set the SecureFile object with a surrender ctx.
 * <p>The data associated with VtSecureFileParamSurrenderCallback is a 
 * pointer to a VtSurrenderCallback struct.
 */
extern VtSecureFileParam VtSecureFileParamSurrenderCallback;

/** Set the SecureZDM object with a surrender ctx.
 * <p>The data associated with VtSecureZDMParamSurrenderCallback is a 
 * pointer to a VtSurrenderCallback struct.
 */
extern VtZDMParam VtZDMParamSurrenderCallback;

/** Set the SecureFile object with a surrender ctx.
 * <p>The data associated with VtSecureFileParamSurrenderCallback is a 
 * pointer to a VtSurrenderCallback struct.
 */
extern VtSecureArchiveParam VtSecureArchiveParamSurrenderCallback;

/** Set the transport context with a surrender ctx.
 * <p>Although all transport contexts will accept a surrender ctx,
 * there is no guarantee that a particular implementation will use it.
 * <p>The data associated with VtTransportParamSurrenderCallback is a
 * pointer to a VtSurrenderCallback struct.
 */
extern VtTransportParam VtTransportParamSurrenderCallback;

/** Set the cert verify context with a surrender ctx.
 * <p>Although all cert verify contexts will accept a surrender ctx,
 * there is no guarantee that a particular implementation will use it.
 * <p>The data associated with VtCertVerifyParamSurrenderCallback is a
 * pointer to a VtSurrenderCallback struct.
 */
extern VtCertVerifyParam VtCertVerifyParamSurrenderCallback;

/*@}*/

/*=========================================================*/
/*                                                         */
/* 64-bit lengths                                          */
/*                                                         */
/*=========================================================*/

/* For most applications that build and read PKCS #7, SecureMail,
 * SecureFile, and ZDM messages, the lengths of the data will never be
 * more than 2^32 bytes. Therefore, length arguments that are 32-bit
 * integers (unsigned int on the vast majority of platforms) will be
 * able to hold any length needed.
 * <p>However, some rare apps might have data to process that is more
 * than 2^32 bytes (that's over 4,294,967,296 bytes, or 4 Gigabytes).
 * For those apps, certain length arguments will have to be 64-bit
 * integers.
 * <p>The toolkit can be rebuilt to handle 64-bit lengths of data.
 * <p>The toolkit is more efficient with unsigned ints as lengths, so
 * the default build does not allow such large input. If your app will
 * never need to build, or will never encounter, data of 4 or more
 * Gigabytes, then you should use the default build. But if you know
 * you will need, or think you might need, 64-bit lengths, then rebuild
 * the toolkit, setting the environment variable VT_64_BIT_LENGTH to 64.
 * <p>You will also need to specify what a 64-bit unsigned int is. On
 * many Unix platforms, an unsigned long long is a 64-bit integer. On
 * Windows, unsigned long long and unsigned _int64 are both 64-bit
 * types.
 * <p>When you typedef the 64-bit integer, it must be unsigned.
 * <p>For more information on using the toolkit with 64-bit lengths,
 * see the User's Manual.
 */

/* #define VT_64_BIT_LENGTH 64
 */

#ifndef VT_64_BIT_LENGTH
#define VT_64_BIT_LENGTH 0
#endif  /* ifndef VT_64_BIT_LENGTH */

#if VT_64_BIT_LENGTH == 64

/** VtUInt64 instead of UInt64 to avoid potential name collisions.
 */
typedef unsigned _int64 VtUInt64;
/* typedef unsigned long long VtUInt64; */

/** SetParam only.
 * <p>Each of these params is functionally equivalent to the regular
 * DataLen Param, except these take a VtUInt64, instead of an unsigned
 * int.
 * <p>For example, before building a P7 message, you can set the P7
 * object with the data length (how many bytes the total length will
 * be, the total number of bytes to be passed into all calls to
 * WriteUpdate, and WriteFinal). If the total length will be (or might
 * be) a number bigger than 4 Gigabytes, call VtSetPkcs7Param with
 * VtPkcs7ParamDataLen64.
 * <p>The associated info is a Pointer to VtUInt64.
 */
extern VtPkcs7Param VtPkcs7ParamDataLen64;
extern VtSecureMailParam VtSecureMailParamDataLen64;
extern VtSecureFileParam VtSecureFileParamDataLen64;
/** You can set the data length of a ZDM v1 object to a 64-bit length,
 * but not v2.
 * <p>Version 2 ZDM uses third party code for compression and file zip,
 * these packages do not allow 64-bit lengths.
 */
extern VtZDMParam VtZDMParamDataLen64;

/** The function VtGetTotalOutputLen64 finds the total output length of
 * a P7, SecureMail, SecureFile, or ZDM message, using a
 * VtGetOutputLen64Impl. This typedef defines what a
 * VtGetOutputLen64Impl is. Although it is a function pointer, an
 * application should never call a VtGetOutputLen64Impl directly, only
 * pass it as an argument to VtGetTotalOutputLen64.
 */
typedef int VT_CALLING_CONV (VtGetOutputLen64Impl) (
   VtLibCtx, Pointer, VtUInt64 *, unsigned int);

/** When building a PKCS #7, SecureMail, SecureFile, or ZDM message, and
 * you want to know how big the total output will be, call this
 * function.
 * <p>Normally, to determine the output length, you would call
 * WriteFinal with a NULL output buffer. This is similar.
 * <p>This is a valid call only after setting the Write object with the
 * DataLen (either with Vt*ParamDataLen or Vt*ParamDataLen64), and
 * calling Vt*WriteInit.
 * <p>To call this function, use the GetOutputLen64Impl for the type of
 * message you're building (if building a P7 message, you will have a
 * VtPkcs7Object created to Write, so use VtGetOutputLen64ImplPkcs7).
 * The associated info for each is the object created to Write.
 * <p>The routine will deposit a VtUInt64 at the totalOutputLen address
 * given.
 */
int VT_CALLING_CONV VtGetTotalOutputLen64 (
   VtLibCtx libCtx,
   VtGetOutputLen64Impl getLen64Impl,
   Pointer associatedInfo,
   VtUInt64 *totalOutputLen
);

/* Use these Impls with VtGetTotalOutputLen64 to determine how long the
 * total message will be.
 * <p>The associated info for each object set to write (P7, SecureMail,
 * etc.).
 */
extern VtGetOutputLen64Impl VtGetOutputLen64ImplPkcs7;
extern VtGetOutputLen64Impl VtGetOutputLen64ImplSecureMail;
extern VtGetOutputLen64Impl VtGetOutputLen64ImplSecureFile;
extern VtGetOutputLen64Impl VtGetOutputLen64ImplZDM;

#endif  /* if VT_64_BIT_LENGTH == 64 */

/*=========================================================*/
/*                                                         */
/* Error codes and functions                               */
/*                                                         */
/*=========================================================*/

/**
 * \defgroup ErrorGroup Error Codes
 */

/*@{*/

/** Given an int that is an error code (the return from a Toolkit
 * call), get a string representation of that code.
 * <p>For example, if the error code is 2, the function returns the
 * string "VT_ERROR_UNIMPLEMENTED".
 * <p>An application could have code that looks like the following.
 * <code>
 * <pre>
 *   do {
 *     status = Vt...
 *     if (status != 0)
 *       break;
 *
 *     status = Vt...
 *     if (status != 0)
 *       break;
 *
 *     . . .
 *   } while (0);
 *
 *   if (status != 0) {
 *     errorString = VtGetErrorAsASCIIString (libCtx, status);
 *     printf ("error = %s\n", errorString);
 *   }
 * </pre>
 * </code>
 * This function will return a NULL-terminated ASCII string. That
 * string is part of global static space. That space will be read only,
 * so should not be a problem in multi-threaded apps. However, because
 * of this (and the fact that calling this routine can add plenty of
 * code size), applications may want to use this function only in debug
 * code.
 * <p>If the errorCode is 0, the function will return the string "No
 * Error".
 * <p>If the errorCode is not one defined by the toolkit, it will
 * return "Unknown Error".
 * <p>For some ports of the toolkit (such as the standard Intel/Windows
 * release), the libCtx is ignored. However, for other builds (such as
 * the FIPS version), the libCtx is needed and if an invalid libCtx is
 * passed in, the return can be something like "Invalid lib ctx".
 *
 * @param libCtx
 * @param errorCode The int that is the error code.
 * @return A NULL-terminated ASCII string that is the string version of
 * the error code.
 */
char VT_CALLING_CONV *VtGetErrorAsASCIIString (
   VtLibCtx libCtx,
   int errorCode
);

#define VT_ERROR_FILE_OPEN                      VT_ERROR_VIBE_BASE+2
#define VT_ERROR_FILE_CLOSE                     VT_ERROR_VIBE_BASE+3
#define VT_ERROR_FILE_READ                      VT_ERROR_VIBE_BASE+4
#define VT_ERROR_FILE_WRITE                     VT_ERROR_VIBE_BASE+5
#define VT_ERROR_FILE_DELETE                    VT_ERROR_VIBE_BASE+6
#define VT_ERROR_FILE_HANDLE_NOT_NULL           VT_ERROR_VIBE_BASE+7
#define VT_ERROR_FILE_MODE_UNKNOWN              VT_ERROR_VIBE_BASE+8
#define VT_ERROR_FILE_RENAME                    VT_ERROR_VIBE_BASE+9
#define VT_ERROR_FILE_COPY                      VT_ERROR_VIBE_BASE+10
#define VT_ERROR_FILE_CREATE_DIR                VT_ERROR_VIBE_BASE+11
#define VT_ERROR_FILE_ATTRIBUTES                VT_ERROR_VIBE_BASE+12
#define VT_ERROR_FILE_INVALID_HANDLE            VT_ERROR_VIBE_BASE+13
#define VT_ERROR_FILE_INVALID_SIZE              VT_ERROR_VIBE_BASE+14
#define VT_ERROR_FILE_POINTER                   VT_ERROR_VIBE_BASE+15
#define VT_ERROR_INVALID_TIME                   VT_ERROR_VIBE_BASE+16
#define VT_ERROR_INVALID_ID_OBJ                 VT_ERROR_VIBE_BASE+17
#define VT_ERROR_INVALID_DIST_OBJ               VT_ERROR_VIBE_BASE+18
#define VT_ERROR_INVALID_CERT_OBJ               VT_ERROR_VIBE_BASE+19
#define VT_ERROR_INVALID_VERIFY_FAIL_LIST       VT_ERROR_VIBE_BASE+20
#define VT_ERROR_INVALID_P7_OBJ                 VT_ERROR_VIBE_BASE+21
#define VT_ERROR_INVALID_RECIP_INFO_OBJ         VT_ERROR_VIBE_BASE+22
#define VT_ERROR_INVALID_RECIP_INFO_LIST        VT_ERROR_VIBE_BASE+23
#define VT_ERROR_NO_ID_IN_P7_MESSAGE            VT_ERROR_VIBE_BASE+24
#define VT_ERROR_CHOOSE_RECIPIENT               VT_ERROR_VIBE_BASE+25
#define VT_ERROR_INVALID_SECURE_MAIL_OBJ        VT_ERROR_VIBE_BASE+26
#define VT_ERROR_INVALID_SECURE_MAIL_MSG        VT_ERROR_VIBE_BASE+27
#define VT_ERROR_INVALID_SECURE_FILE_OBJ        VT_ERROR_VIBE_BASE+28
#define VT_ERROR_INVALID_SECURE_FILE_MSG        VT_ERROR_VIBE_BASE+29
#define VT_ERROR_INVALID_ZDM_OBJ                VT_ERROR_VIBE_BASE+30
#define VT_ERROR_INVALID_ZDM_MSG                VT_ERROR_VIBE_BASE+31
#define VT_ERROR_INVALID_ZDM_INPUT              VT_ERROR_VIBE_BASE+32
#define VT_ERROR_INVALID_ZDM_TEMPLATE           VT_ERROR_VIBE_BASE+33
#define VT_ERROR_NO_ID_AT_INDEX                 VT_ERROR_VIBE_BASE+34
#define VT_ERROR_ENCODING_VERSION               VT_ERROR_VIBE_BASE+35
#define VT_ERROR_UNKNOWN_SCHEMA                 VT_ERROR_VIBE_BASE+36
#define VT_ERROR_UNKNOWN_BER                    VT_ERROR_VIBE_BASE+37
#define VT_ERROR_INVALID_DIST_PARAMS            VT_ERROR_VIBE_BASE+38
#define VT_ERROR_DISTRICT_VALIDITY              VT_ERROR_VIBE_BASE+39
#define VT_ERROR_DISTRICT_NOT_VERIFIED          VT_ERROR_VIBE_BASE+40
#define VT_ERROR_DISTRICT_INFO_UNKNOWN          VT_ERROR_VIBE_BASE+41
#define VT_ERROR_NO_MATCHING_SCHEMA             VT_ERROR_VIBE_BASE+42
#define VT_ERROR_POLICY_NOT_SUPPORTED           VT_ERROR_VIBE_BASE+43
#define VT_ERROR_NO_STORAGE_PROVIDER_LOADED     VT_ERROR_VIBE_BASE+44
#define VT_ERROR_ENTRY_NOT_FOUND                VT_ERROR_VIBE_BASE+45
#define VT_ERROR_ENTRY_NOT_STORED               VT_ERROR_VIBE_BASE+46
#define VT_ERROR_INVALID_STORAGE_HANDLE         VT_ERROR_VIBE_BASE+47
#define VT_ERROR_INVALID_STORAGE_REF            VT_ERROR_VIBE_BASE+48
#define VT_ERROR_INVALID_STORAGE_ENTRY          VT_ERROR_VIBE_BASE+49
#define VT_ERROR_AUTHORIZATION_DENIED           VT_ERROR_VIBE_BASE+50
#define VT_ERROR_UNKNOWN_AUTH_SCHEME            VT_ERROR_VIBE_BASE+51
#define VT_ERROR_UNKNOWN_DISTRICT               VT_ERROR_VIBE_BASE+52
#define VT_ERROR_WRONG_DISTRICT                 VT_ERROR_VIBE_BASE+53
#define VT_ERROR_CERT_REQUEST                   VT_ERROR_VIBE_BASE+54
#define VT_ERROR_CANNOT_VERIFY_CERT             VT_ERROR_VIBE_BASE+55
#define VT_ERROR_NETWORK_CONNECT                VT_ERROR_VIBE_BASE+56
#define VT_ERROR_URL                            VT_ERROR_VIBE_BASE+57
#define VT_ERROR_DOWNLOAD_FAILURE               VT_ERROR_VIBE_BASE+58
#define VT_ERROR_INVALID_STORAGE_ASYNC          VT_ERROR_VIBE_BASE+59
#define VT_ERROR_ASYNC_DOWNLOAD                 VT_ERROR_VIBE_BASE+60
#define VT_ERROR_DOWNLOAD_PENDING               VT_ERROR_VIBE_BASE+61
#define VT_ERROR_DOWNLOAD_PREVIOUS              VT_ERROR_VIBE_BASE+62
#define VT_ERROR_TOKEN_CONFIGURATION            VT_ERROR_VIBE_BASE+63
#define VT_ERROR_INVALID_PROVIDER_USE           VT_ERROR_VIBE_BASE+64
#define VT_ERROR_UNICODE_TRANSLATION            VT_ERROR_VIBE_BASE+65
#define VT_ERROR_UNSUPPORTED                    VT_ERROR_VIBE_BASE+66
#define VT_ERROR_SHARED_MEMORY                  VT_ERROR_VIBE_BASE+67
#define VT_ERROR_SHARED_MEMORY_KEY              VT_ERROR_VIBE_BASE+68
#define VT_ERROR_READ_WRITE_LOCK                VT_ERROR_VIBE_BASE+69
#define VT_ERROR_FILE_LOCK                      VT_ERROR_VIBE_BASE+70
#define VT_ERROR_FILE_LOCK_TABLE_FULL           VT_ERROR_VIBE_BASE+71
#define VT_ERROR_INVALID_FILE_LOCK              VT_ERROR_VIBE_BASE+72
#define VT_ERROR_TIMEOUT                        VT_ERROR_VIBE_BASE+73
#define VT_ERROR_TOOLKIT_LOCK                   VT_ERROR_VIBE_BASE+74
#define VT_ERROR_NO_DISTRICT_SECRET             VT_ERROR_VIBE_BASE+75
#define VT_ERROR_FILE_DOES_NOT_EXIST            VT_ERROR_VIBE_BASE+76
#define VT_ERROR_FILE_MOVE_METHOD               VT_ERROR_VIBE_BASE+77
#define VT_ERROR_FILE_END_OF_FILE               VT_ERROR_VIBE_BASE+78
#define VT_ERROR_NEED_PASSWORD                  VT_ERROR_VIBE_BASE+79
#define VT_ERROR_NEED_USERNAME_PASSWORD         VT_ERROR_VIBE_BASE+80
#define VT_ERROR_SET_PASSWORD                   VT_ERROR_VIBE_BASE+81
#define VT_ERROR_GET_PASSWORD                   VT_ERROR_VIBE_BASE+82
#define VT_ERROR_PASSWORD_CHECK                 VT_ERROR_VIBE_BASE+83
#define VT_ERROR_INVALID_CALLBACK_INFO          VT_ERROR_VIBE_BASE+84
#define VT_ERROR_PASSWORD_CALLBACK              VT_ERROR_VIBE_BASE+85
#define VT_ERROR_OPEN_REGISTRY_KEY              VT_ERROR_VIBE_BASE+86
#define VT_ERROR_READ_REGISTRY_VALUE            VT_ERROR_VIBE_BASE+87
#define VT_ERROR_WRITE_REGISTRY_VALUE           VT_ERROR_VIBE_BASE+88
#define VT_ERROR_DELETE_REGISTRY_VALUE          VT_ERROR_VIBE_BASE+89
#define VT_ERROR_NO_FILE_CTX                    VT_ERROR_VIBE_BASE+90
#define VT_ERROR_NO_STORAGE_CTX                 VT_ERROR_VIBE_BASE+91
#define VT_ERROR_NO_TRANSPORT_CTX               VT_ERROR_VIBE_BASE+92
#define VT_ERROR_NO_POLICY_CTX                  VT_ERROR_VIBE_BASE+93
#define VT_ERROR_NO_CERT_VERIFY_CTX             VT_ERROR_VIBE_BASE+94
#define VT_ERROR_NO_DER_CODERS                  VT_ERROR_VIBE_BASE+95
#define VT_ERROR_NO_SCHEMA_DECODERS             VT_ERROR_VIBE_BASE+96
#define VT_ERROR_INVALID_FILE_CTX               VT_ERROR_VIBE_BASE+97
#define VT_ERROR_INVALID_STORAGE_CTX            VT_ERROR_VIBE_BASE+98
#define VT_ERROR_INVALID_TRANSPORT_CTX          VT_ERROR_VIBE_BASE+99
#define VT_ERROR_INVALID_POLICY_CTX             VT_ERROR_VIBE_BASE+100
#define VT_ERROR_INVALID_CERT_VFY_CTX           VT_ERROR_VIBE_BASE+101
#define VT_ERROR_INVALID_CERT_REQ_OBJ           VT_ERROR_VIBE_BASE+102
#define VT_ERROR_INVALID_ID_LIST_OBJ            VT_ERROR_VIBE_BASE+103
#define VT_ERROR_INVALID_ARB_STORE_OBJ          VT_ERROR_VIBE_BASE+104
#define VT_ERROR_COMPRESS                       VT_ERROR_VIBE_BASE+105
#define VT_ERROR_DECOMPRESS                     VT_ERROR_VIBE_BASE+106
#define VT_ERROR_TEMP_FILE                      VT_ERROR_VIBE_BASE+107
#define VT_ERROR_ARCHIVE_WRITE                  VT_ERROR_VIBE_BASE+108
#define VT_ERROR_ARCHIVE_READ                   VT_ERROR_VIBE_BASE+109
#define VT_ERROR_ARCHIVE_BUFFER_TYPE            VT_ERROR_VIBE_BASE+110
#define VT_ERROR_ARCHIVE_ENTRY_NOT_FOUND        VT_ERROR_VIBE_BASE+111
#define VT_ERROR_INVALID_FILE_NAME              VT_ERROR_VIBE_BASE+112
#define VT_ERROR_INVALID_STREAM                 VT_ERROR_VIBE_BASE+113
#define VT_ERROR_INVALID_STREAM_OPEN_MODE       VT_ERROR_VIBE_BASE+114
#define VT_ERROR_INVALID_STREAM_SEEK_MODE       VT_ERROR_VIBE_BASE+115
#define VT_ERROR_END_OF_STREAM                  VT_ERROR_VIBE_BASE+116
#define VT_ERROR_STREAM_NOT_READABLE            VT_ERROR_VIBE_BASE+117
#define VT_ERROR_STREAM_NOT_WRITABLE            VT_ERROR_VIBE_BASE+118
#define VT_ERROR_STREAM_WRITE                   VT_ERROR_VIBE_BASE+119
#define VT_ERROR_STREAM_READ                    VT_ERROR_VIBE_BASE+120
#define VT_ERROR_STREAM_NOT_OPEN                VT_ERROR_VIBE_BASE+121
#define VT_ERROR_STREAM_ALREADY_OPEN            VT_ERROR_VIBE_BASE+122
#define VT_ERROR_INVALID_SECURE_ARCHIVE_OBJ     VT_ERROR_VIBE_BASE+123
#define VT_ERROR_INVALID_SECURE_ARCHIVE_ENTRY   VT_ERROR_VIBE_BASE+124
#define VT_ERROR_SECURE_ARCHIVE_ATTR_NAME       VT_ERROR_VIBE_BASE+125
#define VT_ERROR_SECURE_ARCHIVE_SET_ATTR        VT_ERROR_VIBE_BASE+126
#define VT_ERROR_NO_MESSAGE_DATA_VAR            VT_ERROR_VIBE_BASE+127
#define VT_ERROR_INVALID_MESSAGE_FORMAT         VT_ERROR_VIBE_BASE+128
#define VT_ERROR_INVALID_ZDM_VERSION            VT_ERROR_VIBE_BASE+129
#define VT_ERROR_INVALID_ZDM_LOCATION           VT_ERROR_VIBE_BASE+130
#define VT_ERROR_NO_ZDM_LOCATION                VT_ERROR_VIBE_BASE+131
#define VT_ERROR_NUMERIC_CONVERSION             VT_ERROR_VIBE_BASE+132
#define VT_ERROR_INVALID_COMPRESSION_ALG        VT_ERROR_VIBE_BASE+133
#define VT_ERROR_INVALID_ZDM_ENTRY              VT_ERROR_VIBE_BASE+134
#define VT_ERROR_INVALID_SECURE_MAIL_TEMPLATE   VT_ERROR_VIBE_BASE+135
#define VT_ERROR_DATA_NODE_NOT_FOUND            VT_ERROR_VIBE_BASE+136
#define VT_ERROR_DATA_NODE_WRONG_TYPE           VT_ERROR_VIBE_BASE+137
#define VT_ERROR_DATA_NODE_INVALID_EXPRESSION   VT_ERROR_VIBE_BASE+138
#define VT_ERROR_DATA_NODE_INDEX_OUT_OF_BOUNDS  VT_ERROR_VIBE_BASE+139
#define VT_ERROR_DATA_NODE_INVALID_FORMAT_FLAGS VT_ERROR_VIBE_BASE+140
#define VT_ERROR_DATA_NODE_XML_PARSE            VT_ERROR_VIBE_BASE+141
#define VT_ERROR_INVALID_END_OF_LINES           VT_ERROR_VIBE_BASE+142
#define VT_ERROR_ATTRIBUTE_NOT_SET              VT_ERROR_VIBE_BASE+143
#define VT_ERROR_MISMATCHED_KEYS                VT_ERROR_VIBE_BASE+144
#define VT_ERROR_UNDETERMINED_VERSION           VT_ERROR_VIBE_BASE+145
#define VT_ERROR_INVALID_CONNECTION_CACHE_CTX   VT_ERROR_VIBE_BASE+146
#define VT_ERROR_INVALID_CONNECTION             VT_ERROR_VIBE_BASE+147
#define VT_ERROR_ALL_CONNECTIONS_IN_USE         VT_ERROR_VIBE_BASE+148

/*@}*/

#ifdef __cplusplus
}
#endif

#endif /* _VIBE_H */
