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

#ifndef _VIBE_CRYPTO_H
#define _VIBE_CRYPTO_H

#ifdef __cplusplus
extern "C" {
#endif

/** VT_CALLING_CONV
 * <p>Toolkit functions in vibe.h are declared as follows.
 * <code>
 * <pre>
 *    int VT_CALLING_CONV FunctionName (
 *      someType arg1,
 *      anotherType arg2
 *    );
 * </pre>
 * </code>
 * The VT_CALLING_CONV is for potential builds with a special
 * _pascal_ or similar calling convention. The default is "nothing",
 * that is, no special calling convention (statndard C).
 * <p>If you need a toolkit with a different calling convention, change
 * this #define.
 * <p>Internal functions use VOLT_CALLING_CONV, defined in base.h.
 */
#define VT_CALLING_CONV

/** This is a "generic" pointer.
 */
typedef unsigned char *Pointer;

/** A VtItem is just a way to present data and its length in one
 * structure.
 */
typedef struct
{
  /** The byte array.
   */
  unsigned char *data;
  /** The length in bytes.
   */
  unsigned int len;
} VtItem;

/*=========================================================*/
/*                                                         */
/* Library Context                                         */
/*                                                         */
/*=========================================================*/

/* Further Library Context operations are declared in vibe.h.
 */

/** The Library Context.
 * <p>Note that the context is a pointer type.
 */
typedef struct VtLibCtxDef *VtLibCtx;

/** Forward referencing, the toolkit will use this type when building
 * a memory context for a libCtx. Applications can ignore this data
 * type.
 */
typedef struct VtMemoryInfoDef VtMemoryInfo;

/** Forward referencing, the toolkit will use this type when building
 * a thread context for a libCtx. Applications can ignore this data
 * type.
 */
typedef struct VtThreadInfoDef VtThreadInfo;

/** A library context possesses a memory context. All toolkit memory
 * operations (alloc, free, memset, etc.) are performed by a memory
 * context inside the library context. However, the libCtx does not
 * come with a "built-in" memory context, it constructs one when the
 * libCtx is being created (VtCreateLibCtx). It builds one based
 * on the MemoryImpl it is given.
 * <p>The VtMemoryImpl contains the implementations of the memory
 * functions. These implementations are loaded into the libCtx. This
 * typedef defines what a VtMemoryImpl is. Although a MemoryImpl
 * is a function pointer, an application should never call one
 * directly, only pass it as an argument to VtCreateLibCtx.
 */
typedef int VT_CALLING_CONV (VtMemoryImpl) (VtLibCtx, VtMemoryInfo *);

/** A library context possesses a thread context. All toolkit thread
 * operations (locking, etc.) are performed by a thread context inside
 * the library context. However, the libCtx does not come with a
 * "built-in" thread context, it constructs one when the libCtx is
 * being created (VtCreateLibCtx). It builds one based on the
 * ThreadImpl it is given.
 * <p>The toolkit will not create or spawn threads. However, some
 * toolkit operations will acquire a lock so that if the application is
 * using multiple threads, the toolkit operations will not cause
 * problems.
 * <p>The VtThreadImpl contains the implementations of the thread
 * functions. These implementations are loaded into the libCtx. This
 * typedef defines what a VtThreadImpl is. Although a ThreadImpl
 * is a function pointer, an application should never call one
 * directly, only pass it as an argument to VtCreateLibCtx.
 */
typedef int VT_CALLING_CONV (VtThreadImpl) (VtLibCtx, VtThreadInfo *);

/** Create a new Library Context. This builds an "empty" context using
 * the MemoryImpl passed in.
 * <p>The library context created will define the way all toolkit
 * functions handle system operations. The two most important classes
 * of system operations, memory and threading, are set during creation.
 * Other categories (such as network or UI) can be set using the
 * VtSetLibCtxParam function.
 * <p>When creating the library context, you must specify the memory
 * and threading implementations using a VtMemoryImpl and
 * VtThreadImpl. The toolkit documentation will contain a list of
 * all supported MemoryImpls and ThreadImpls.
 * <p>Each MemoryImpl and ThreadImpl will have information to accompany
 * it. That data could be a VtItem, it could be NULL. Check the
 * documentation for each MemoryImpl and ThreadImpl for a description of
 * the data and its required format.
 * <p>To use this function decide which MemoryImpl and ThreadImpl you
 * want to use, then determine what information those Impls need and in
 * which format it is presented. Collect the required data in the
 * appropriate format then call this function passing in the desired
 * MemoryImpl and ThreadImpl along with the associated info. The
 * associated info must be cast to Pointer.
 * <p>For example, if the required info is a VtItem struct, declare a
 * variable to be of type VtItem, fill in the fields, then pass the address
 * of that struct cast to Pointer.
 * <p>The input libCtx is a pointer to a context. It should point to
 * a NULL VtLibCtx. This function will go to the address given and
 * deposit a created context.
 * <p>If you call Create, you must call Destroy after you are done with
 * the context but before the program exits.
 * <p>The Destroy should be the last thing you call. After calling
 * Destroy, you must not call any more toolkit functions.
 * <p>Example:
 * <code>
 * <pre>
 *    int status;
 *    VtLibCtx libCtx = (VtLibCtx)0;
 *
 *    do {
 *      status = VtCreateLibCtx (
 *        VtMemoryImplWin32, (Pointer)0,
 *        VtThreadImplWin32Multi, (Pointer)0,
 *        &libCtx);
 *      if (status != 0)
 *        break;
 *          . . .
 *    } while (0);
 *
 *    VtDestroyLibCtx (&libCtx);
 * </pre>
 * </code>
 * @param MemoryImpl The memory implementation.
 * @param memoryInfo The info the memoryImpl needs.
 * @param ThreadImpl The threading system.
 * @param threadInfo The info the threadImpl needs.
 * @param libCtx A pointer to where the routine will deposit the
 * created library context.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtCreateLibCtx (
   VtMemoryImpl MemoryImpl,
   Pointer memoryInfo,
   VtThreadImpl ThreadImpl,
   Pointer threadInfo,
   VtLibCtx *libCtx
);

/* The following are the MemoryImpls currently supported. Each Impl is
 * used in conjunction with special info for the function. If there is
 * no special info, the accompaniment is a NULL pointer.
 */

/** Use this MemoryImpl when building a library context, it will load a
 * memory implementation that uses the basic malloc, free, memset,
 * etc., routines offered by the operating system.
 * <p>The data associated with VtMemoryImplDefault is a NULL pointer
 * or a pointer to an int set to one of the following values.
 * <code>
 * <pre>
 *    VT_MEM_OVERWRITE_ALL
 *    VT_MEM_OVERWRITE_SENSITIVE
 * </pre>
 * </code>
 * <p>If the flag is set to VT_MEM_OVERWRITE_ALL, before freeing any
 * allocated memory, the memory implementation will overwrite all memory
 * with zeroes -- memset (buffer, 0, bufferSize). If the flag is set to
 * VT_MEM_OVERWRITE_SENSITIVE, the memory implementation will
 * overwrite only "sensitive" data (symmetric and private key data,
 * authTokens, etc.) before freeing. Other memory (for public keys,
 * district names, etc.) will be freed unchanged.
 * <p>If the data accompanying VtMemoryImplDefault is a NULL pointer,
 * the memory implementation will overwrite all memory. That is, a NULL
 * pointer is the same as the VT_MEM_OVERWRITE_ALL flag.
 */
VtMemoryImpl VtMemoryImplDefault;

/** Use this MemoryImpl when building a library context, it will load a
 * memory implementation that uses the GlobalAlloc, GlobalFree, etc.,
 * routines offered by the Win32 Operating System.
 * <p>The data associated with VtMemoryImplWin32 is a NULL pointer or a
 * pointer to an int set to one of the following values.
 * <code>
 * <pre>
 *    VOLT_MEM_OVERWRITE_ALL
 *    VOLT_MEM_OVERWRITE_SENSITIVE
 * </pre>
 * </code>
 * <p>If the flag is set to VT_MEM_OVERWRITE_ALL, before freeing any
 * allocated memory, the memory implementation will overwrite all memory
 * with zeroes -- memset (buffer, 0, bufferSize). If the flag is set to
 * VT_MEM_OVERWRITE_SENSITIVE, the memory implementation will
 * overwrite only "sensitive" data (symmetric and private key data,
 * authTokens, etc.) before freeing. Other memory (for public keys,
 * district names, etc.) will be freed unchanged.
 * <p>If the data accompanying VtMemoryImplWin32 is a NULL pointer, the
 * memory implementation will overwrite all memory. That is, a NULL
 * pointer is the same as the VT_MEM_OVERWRITE_ALL flag.
 */
VtMemoryImpl VtMemoryImplWin32;

/** For use with various MemoryImpls, instruct the memory context to
 * overwrite only sensitive data before freeing it.
 */
#define VT_MEM_OVERWRITE_SENSITIVE    0
/* defined for backwards compatibility.
 */
#define VOLT_MEM_OVERWRITE_SENSITIVE  VT_MEM_OVERWRITE_SENSITIVE
/** For use with various MemoryImpls, instruct the memory context to
 * overwrite all data before freeing it.
 */
#define VT_MEM_OVERWRITE_ALL          1
/* defined for backwards compatibility.
 */
#define VOLT_MEM_OVERWRITE_ALL        VT_MEM_OVERWRITE_ALL

/** Use this MemoryImpl when building a library context, it will load a
 * memory implementation that uses basic malloc, free, etc. routines,
 * and also performs some rudimentary memory "bookkeeping" to help find
 * memory leaks. This memory implementation is slower and uses more
 * memory. It is not intended for production code. It will print out a
 * memory report when the libCtx is destroyed (DestroyLibCtx).
 * <p>The data associated with VtMemoryImplDebug is a pointer to a
 * VtDebugMemoryInfo struct.
 */
VtMemoryImpl VtMemoryImplDebug;

/** This struct is the data to accompany VtMemoryImplDebug.
 */
typedef struct
{
  /** The maxCtxSpace field indicates the maximum space (measured in
   * bytes) you would like the memory context to use for the context
   * itself. This does not limit the number of bytes that the toolkit
   * can allocate, only the number of bytes the context itself can use.
   * How much space you should set is dependent on how much work you
   * plan to do. 4096 bytes will likely be sufficient for most test
   * programs. If it is not enough, a Malloc call will return
   * VT_ERROR_MEMORY and the memory report printed out will indicate
   * that you "Ran out of entries, need more space." This value must be
   * at least 2000 (or thereabouts). If the value is too low, you won't
   * be able to build a libCtx.
   */
  unsigned int maxCtxSpace;
  /** The flags field is a bit field, the OR of further defining flags.
   * See the list of VT_DEBUG_MEM_FLAG_ #defines for valid values.
   * Also OR in VT_MEM_OVERWRITE_ALL to instruct the impl to
   * overwrite all memory (not just sensitive data) before freeing.
   */
  unsigned int flags;
} VtDebugMemoryInfo;

/** Use this flag to tell the debug memory implementation to simply use
 * printf to print out any reports or error messages. This will likely
 * be simple messages to stdout, displayed on the console. If this bit
 * is set in the flags field of the VtDebugMemoryInfo struct, the
 * VT_DEBUG_MEM_FLAG_WIN_MSG bit must not be set.
 */
#define VT_DEBUG_MEM_FLAG_PRINTF     0x0100
/* defined for backwards compatibility.
 */
#define VOLT_DEBUG_MEM_FLAG_PRINTF   VT_DEBUG_MEM_FLAG_PRINTF
/** Use this flag to tell the debug memory implementation to print out
 * any reports or error messages to a Windows MessageBox. If this bit
 * is set in the flags field of the VtDebugMemoryInfo struct, the
 * VT_DEBUG_MEM_FLAG_PRINTF bit must not be set.
 */
#define VT_DEBUG_MEM_FLAG_WIN_MSG    0x0200
/* defined for backwards compatibility.
 */
#define VOLT_DEBUG_MEM_FLAG_WIN_MSG  VT_DEBUG_MEM_FLAG_WIN_MSG

/* The following are the ThreadImpls currently supported. Each Impl is
 * used in conjunction with special info for the function. If there is
 * no special info, the accompaniment is a NULL pointer.
 */

/** Use this ThreadImpl when building a library context, it will load a
 * thread implementation that does no locking. If an application will
 * be run in a single thread, there is no need for the toolkit
 * operations to guard aginst multiple thread access.
 * <p>The data associated with VtThreadImplDefaultSingle is a NULL
 * pointer: (Pointer)0.
 */
VtThreadImpl VtThreadImplDefaultSingle;

/** Use this ThreadImpl when building a library context, it will load a
 * thread implementation that performs Win32 locking.
 * <p>The data associated with VtThreadImplWin32Multi is a NULL
 * pointer: (Pointer)0.
 */
VtThreadImpl VtThreadImplWin32Multi;

/** Use this ThreadImpl when building a library context, it will load a
 * thread implementation that performs pthread locking (Linux, for
 * example).
 * <p>The data associated with VtThreadImplPThread is a NULL pointer:
 * (Pointer)0.
 */
VtThreadImpl VtThreadImplPThread;

/** Create a library context for FIPS.
 * <p>This function will build a libCtx, much as VtCreateLibCtx does.
 * However, this will load the shared library, perform the integrity
 * check, and run the self-tests required by FIPS.
 * <p>If the library linked in is not the FIPS version of the toolkit,
 * this function will not work.
 * <p>This function does not offer an option for memory or thread
 * Impls.
 * <p>The input libCtx is a pointer to a context. It should point to
 * a NULL VtLibCtx. This function will go to the address given and
 * deposit a created context.
 * <p>If you call Create, you must call Destroy after you are done with
 * the context but before the program exits. The Destroy to call is the
 * special FIPS destructor: VtDestroyLibCtxFips.
 * <p>The Destroy should be the last thing you call. After calling
 * Destroy, you must not call any more toolkit functions.
 * <p>Most, if not all, LibCtxParams will work with the libCtx built
 * by this function.
 *
 * @param libCtx A pointer to where the routine will deposit the
 * created library context.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtCreateLibCtxFips (
   VtLibCtx *libCtx
);

/** Destroy the Library Context. This frees up any memory and closes
 * any connections or libraries opened in creating the context.
 * <p>The input is the address of the context. The function will go to
 * that address to find the context to destroy, then it will deposit at
 * that address a NULL pointer.
 * <p>Example:
 * <code>
 * <pre>
 *    int status;
 *    VtLibCtx libCtx = (VtLibCtx)0;
 *
 *    do {
 *      status = VtCreateLibCtx (
 *        VtMemoryImplWin32, (Pointer)0,
 *        VtThreadImplWin32Multi, (Pointer)0,
 *        &libCtx);
 *      if (status != 0)
 *        break;
 *          . . .
 *    } while (0);
 *
 *    VtDestroyLibCtx (&libCtx);
 * </pre>
 * </code>
 * @param libCtx A pointer to where the routine will deposit the
 * created library context.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtDestroyLibCtx (
   VtLibCtx *libCtx
);

/** Destroy a FIPS Library Context. This frees up any memory and closes
 * any connections or libraries opened in creating the context.
 * <p>An application that creates a FIPS libCtx must call this function
 * to destroy it.
 * <p>The input is the address of the context. The function will go to
 * that address to find the context to destroy, then it will deposit at
 * that address a NULL pointer.
 *
 * @param libCtx A pointer to where the routine will deposit the
 * created library context.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtDestroyLibCtxFips (
   VtLibCtx *libCtx
);

/** The function VtSetLibCtxParam adds information to a library
 * context. The information to add is defined by a VtLibCtxParam.
 * This typedef defines what a VtLibCtxParam is. Although a
 * LibCtxParam is a function pointer, an application should never call
 * one directly, only pass it as an argument to VtSetLibCtxParam.
 * <p>To set a libCtx is to add information or describe universal
 * behavior.
 */
typedef int VT_CALLING_CONV (VtLibCtxParam) (VtLibCtx, Pointer, unsigned int);

/** Set the library context with the info given.
 * <p>The VtLibCtxParam defines what info will be added to the
 * libCtx.
 * <p>The include file vibe.h defines the supported VtLibCtxParams.
 * Look through the include file to see which VtLibCtxParam you want
 * for your application. All supported VtLibCtxParams will be defined as
 * in the following example.
 * <code>
 * <pre>
 *   extern VtLibCtxParam VtLibCtxParamIBECacheCtx;
 * </pre>
 * </code>
 * <p>Associated with each VtLibCtxParam is specific info. The
 * documentation for each VtLibCtxParam will describe the
 * associated info it needs. That data could be another object, it
 * could be data in a particular struct, it might be a NULL pointer.
 * Check the documentation for each VtLibCtxParam for a
 * description of the data and its required format.
 * <p>To use this function decide which VtLibCtxParam you want to
 * use, then determine what information that VtLibCtxParam needs
 * and in which format it is presented. Collect the data in the
 * appropriate format then call this function, passing in the desired
 * VtLibCtxParam and the required info. The associated info must
 * be cast to Pointer.
 * <p>For example, if the required info is a VtItem, declare a
 * variable to be a VtItem, set the fields with the appropriate
 * information, then pass the address of the VtItem cast to Pointer.
 *
 * @param libCtx The context to set.
 * @param libCtxParam What the object is being set to.
 * @param associatedInfo The info needed by the LibCtxParam.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtSetLibCtxParam (
   VtLibCtx libCtx,
   VtLibCtxParam libCtxParam,
   Pointer associatedInfo
);

/** Get the specified type of information out of a libCtx.
 * <p>This function will go to the address given by getInfo and deposit
 * a pointer. That pointer is the address of the data. The memory to
 * hold the information belongs to the libCtx, do not free it.
 * <p>The VtLibCtxParam will specify what kind of information will be
 * returned, the getInfo is where the function will deposit the info.
 * <p>The include file vibe.h defines the supported LibCtxParams. Look
 * through the include file to see which LibCtxParam to use for your
 * application.
 * <p>See also the documentation for VtSetLibCtxParam.
 * <p>To use this function decide which LibCtxParam you want to use,
 * then determine what information that LibCtxParam will return and in
 * which format it is presented. Declare a variable to be a pointer to
 * the appropriate type, then call this function passing in the desired
 * LibCtxParam and the address of the variable. Cast the address of the
 * pointer to the appropriate type: (Pointer *).
 * <p>If the associated info is already a pointer type (such as an
 * object), declare a variable to be of the appropriate type and pass
 * the address of that variable cast to (Pointer *).
 * <p>For example, if the returned info is a VtRandomObject, declare a
 * variable to be of type VtRandomObject, pass the address of that
 * variable (&varName) cast to (Pointer *).
 *
 * @param libCtx The libCtx to query.
 * @param libCtxParam Defines what info to get.
 * @param getInfo The address of a pointer. The function will go to
 * that address and deposit a pointer to the info requested.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtGetLibCtxParam (
   VtLibCtx libCtx,
   VtLibCtxParam libCtxParam,
   Pointer *getInfo
);

/* These are the VtLibCtxParams supported by the toolkit. Each
 * VtLibCtxParam is used in conjunction with special info for the
 * function. If there is no special info, the accompaniment is a NULL
 * pointer.
 */

/** SetParam or GetParam.
 * <p>Set the library context with IBE caching functions, or get the
 * cache ctx from a libCtx (to get or set entries).
 * <p>The caching operations in this VtLibCtxParam will store useful
 * information in the library context so that IBE operations don't have
 * to constantly rebuild them. This will improve the performance of IBE
 * operations, but increase the code size.
 * <p>The associated info is a VtIBECacheCtx.
 * <p>When setting, build the VtIBECacheCtx and pass it as the
 * associatedInfo.
 */
extern VtLibCtxParam VtLibCtxParamIBECacheCtx;

/** Show the FIPS error status. There is a separate error for FIPS
 * operations. If the toolkit encounters a FIPS error, it will not
 * perform any more FIPS operations. That is, the error is not
 * "cleared" after a function completes, it stays set as long as the
 * libCtx is "alive". Clear a FIPS error by destroying the libCtx and
 * creating a new one.
 * <p>NOTE: Remember, all objects built using the libCtx must be
 * destroyed before destroying the libCtx itself. If a libCtx possesses
 * a FIPS error, it will not be able to perform any function, but it
 * will be able to destroy objects and be destroyed.
 * <p>This function is how the caller can see what the current FIPS
 * error is. It will be 0 for no error or a code indicating what FIPS
 * related operation failed.
 * <p>If the library linked in is not FIPS, this function will return 0.
 * <p>If the libCtx passed in is not valid (e.g. NULL or not a FIPS
 * libCtx when the library linked in is a FIPS library), the return
 * will be VT_ERROR_INVALID_LIB_CTX.
 *
 * @param libCtx The libCtx built by calling VtCreateLibCtxFips.
 * @return The FIPS error, 0 for no current FIPS error or a non-zero
 * code, or VT_ERROR_INVALID_LIB_CTX.
 */
int VT_CALLING_CONV VtGetFipsError (
   VtLibCtx libCtx
);

/* The following are function calls added to the toolkit as a
 * convenience. If an application needs to allocate, reallocate, free,
 * set, copy, move, or compare memory, it will have its own technique.
 * However, when the toolkit operates on memory buffers, it uses the
 * memory implementation passed in during LibCtx creation. If the
 * application wants to use the same memory context, call the following
 * routines.
 */

/** Allocate memory using the memory implementation in the libCtx.
 * <p>This is added as a convenience, allowing an application to use
 * the same memory context the toolkit functions will use.
 * <p>As with all malloc routines, any memory allocated using this call
 * must be freed using its partner function, VtFree.
 * <p>Often, memory must be overwritten before freeing it (to "wipe
 * away" any sensitive information). If the memory is sensitive, pass
 * in the flag VT_MEMORY_SENSITIVE. If not, pass in 0. Note that there
 * are some memory contexts that will overwrite all memory before
 * freeing, whether it is flagged SENSITIVE or not.
 * <p>Also, a memory context might treat any non-zero flag as
 * equivalent to SENSITIVE. That is, if you pass 0, you're saying the
 * memory is not sensitive. If you pass any non-zero value (including,
 * but not limited to SENSITIVE), the context interprets it to mean
 * SENSITIVE.
 * <p>NOTE!!! The return value is an int (0 for success or nonzero
 * error code), NOT the allocated buffer. Most memory allocation
 * routines return the buffer (or NULL if the allocation failed). This
 * is different.
 * <p>The allocated memory is returned at the address given by the
 * buffer argument. That is, pass in an address where the function will
 * deposit the pointer to the allocated memory.
 * <p>Example:
 * <pre>
 * <code>
 *   unsigned char *newBuffer = (unsigned char *)0;
 *
 *   do {
 *     status = VtMalloc (
 *       libCtx, bufferSize, VT_MEMORY_SENSITIVE, (Pointer *)&newBuffer);
 *     if (status != 0)
 *       break;
 *
 *     status = VtMemset (libCtx, newBuffer, 0, (Pointer)bufferSize);
 *     if (status != 0)
 *       break;
 *
 *   } while (0);
 *
 *   VtFree (libCtx, (Pointer *)&newBuffer);
 * </code>
 * </pre>
 *
 * @param libCtx The libCtx containing the memory context to use when
 * allocating memory.
 * @param size The number of bytes to allocate.
 * @param flag Indicates whether the memory is sensitive or not.
 * @param buffer The address where the function will deposit the
 * pointer to the allocated memory.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtMalloc (
   VtLibCtx libCtx,
   unsigned int size,
   unsigned int flag,
   Pointer *buffer
);

/** Reallocate memory using the memory implementation in the libCtx.
 * <p>Allocate a new buffer, size bytes big, copy the contents of the
 * original buffer into the new buffer, free the original buffer, and
 * return the new buffer.
 * <p>This is added as a convenience, allowing an application to use
 * the same memory context the toolkit functions will use.
 * <p>As with all allocation routines, any memory allocated using this
 * call must be freed using its partner function, VtFree.
 * <p>If the original buffer is NULL, this function is equivalent to
 * VtMalloc. In this case, the flag argument is used. Often, memory
 * must be overwritten before freeing it (to "wipe away" any sensitive
 * information). If the memory is sensitive, pass in the flag
 * VT_MEMORY_SENSITIVE. If not, pass in 0. Note that there are some
 * memory contexts that will overwrite all memory before freeing,
 * whether it is flagged SENSITIVE or not.
 * <p>Also, a memory context might treat any non-zero flag as
 * equivalent to SENSITIVE. That is, if you pass 0, you're saying the
 * memory is not sensitive. If you pass any non-zero value (including,
 * but not limited to SENSITIVE), the context interprets it to mean
 * SENSITIVE.
 * <p>If the original buffer is not NULL, the flag argument is ignored
 * and the function uses the sensitivity of the original. That is, if
 * the input buffer was originally created to be SENSITIVE, the
 * reallocated buffer is SENSITIVE, regardless of the flag to this
 * function.
 * <p>If the original buffer is not NULL, and it is already big enough
 * for the requested size, the function does not allocate anything new,
 * it simply returns the original buffer.
 * <p>If the original buffer is not NULL, and it is not big enough, the
 * function will allocate a new buffer, copy the contents of the old
 * into the new, and free the old. It will return the new buffer.
 * <p>NOTE!!! The return value is an int (0 for success or nonzero
 * error code), NOT the allocated buffer. Most memory reallocation
 * routines return the buffer (or NULL if the reallocation failed).
 * This is different. Furthermore, if there is an error, this function
 * does NOT free the original buffer.
 * <p>The caller passes in the address of the pointer to the original
 * buffer. The function will go to that address to pick up the pointer.
 * <p>The function will deposit the new buffer at the same location.
 * <p>Example:
 * <pre>
 * <code>
 *   unsigned char *buf = (unsigned char *)0;
 *
 *   do {
 *
 *       // Equivalent to VtMalloc, the function will go to the address
 *       // given by buf and find a NULL pointer. So it will allocate a
 *       // new buffer and return the new buffer at the address.
 *       status = VtRealloc (
 *         libCtx, 20, VT_MEMORY_SENSITIVE, (Pointer *)&buf);
 *       if (status != 0)
 *         break;
 *
 *       // The value of the buf variable is now an address, let's say
 *       // the value is 0x0088ABC0.
 *
 *       // The original buffer is big enough, so the function will
 *       // essentially do nothing. Also, the memory is still SENSITIVE
 *       // because the original buffer was SENSITIVE. VtRealloc
 *       // ignores the flag when the original buffer is not NULL.
 *       status = VtRealloc (libCtx, 16, 0, (Pointer *)&buf);
 *       if (status != 0)
 *         break;
 *
 *       // The value of the buf variable is has not changed, it is
 *       // still 0x0088ABC0.
 *
 *       // The original buffer is not big enough, the function will
 *       // allocate a new buffer. Also, the memory is still SENSITIVE
 *       // because the original buffer was SENSITIVE.
 *       status = VtRealloc (libCtx, 64, 0, (Pointer *)&buf);
 *       if (status != 0)
 *         break;
 *
 *       // The value of the buf variable is has changed, it is no
 *       // longer 0x0088ABC0. Let's say it is 0x00881230. If you went
 *       // to the address 0x0088ABC0, you would find the 20 bytes
 *       // there overwritten and the memory freed. If you go to the
 *       // address 0x00881230, the first 20 bytes are the contents of
 *       // the original buffer, plus there are 44 more bytes available
 *       // for use.
 *
 *   } while (0);
 *
 *   VtFree (libCtx, (Pointer *)&buffer);
 * </code>
 * </pre>
 * <p>If you call VtRealloc, you should always call VtFree, even if
 * there was an error. If there is an error, the original buffer might
 * not have been freed.
 *
 * @param libCtx The libCtx containing the memory context to use when
 * allocating memory.
 * @param size The number of bytes to allocate.
 * @param flag Indicates whether the memory is sensitive or not (only
 * used if the original buffer is NULL).
 * @param buffer The address where the function will find the pointer
 * to the original buffer. Also the address where the function will
 * deposit the resulting pointer to the allocated memory.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtRealloc (
   VtLibCtx libCtx,
   unsigned int size,
   unsigned int flag,
   Pointer *buffer
);

/** For use with VtMalloc and VtRealloc, instructs the memory context
 * to overwrite the memory before freeing it.
 */
#define VT_MEMORY_SENSITIVE                 0x220

/** Free memory that had been allocated using VtMalloc or VtRealloc.
 * <p>If the memory was allocated using the VT_MEMORY_SENSITIVE flag,
 * this function will overwrite the memory before freeing it. Note
 * that some memory contexts will overwrite all memory before freeing,
 * whether it is flagged SENSITIVE or not.
 * <p>This function should be called only on memory that had been
 * allocated using its partner functions, VtMalloc and VtRealloc.
 * <p>If the buffer to free is NULL, this function will do nothing (it
 * will not try to free memory at address 0).
 * <p>The caller passes the address of the pointer to the buffer. That
 * is, the function will go to the address given by buffer and find the
 * pointer to the allocated memory. Upon return, the function will have
 * deposited a NULL pointer at the address.
 *
 * @param libCtx The libCtx containing the memory context to use when
 * freeing memory.
 * @param buffer The address where the function will find the pointer
 * to the allocated memory to free.
 * @return none.
 */
void VT_CALLING_CONV VtFree (
   VtLibCtx libCtx,
   Pointer *buffer
);

/** Set the count bytes of buffer to the given value.
 * <p>The value is an unsigned int (likely to be 4 bytes of
 * information), however, the function will only examine the low order
 * byte. Many compilers do not allow chars to be passed as arguments,
 * so we pass an unsigned int.
 * <p>The underlying memory ctx will be called on to perform this
 * operation, and it might not check the size of the buffer. Hence, the
 * caller should not set unallocated memory.
 *
 * @param libCtx The libCtx containing the memory context to use when
 * memsetting memory.
 * @param buffer The buffer to set.
 * @param value The value each byte in the buffer will be.
 * @param count How many bytes to set.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtMemset (
   VtLibCtx libCtx,
   Pointer buffer,
   unsigned int value,
   unsigned int count
);

/** Copy count bytes from source to dest.
 * <p>The two buffers should not overlap. If the buffers might overlap,
 * then call VtMemmove.
 * <p>The underlying memory ctx will be called on to perform this
 * operation, and it might not check the size of the buffer. Hence, the
 * caller should not copy from or copy into unallocated memory.
 *
 * @param libCtx The libCtx containing the memory context to use when
 * memcpying memory.
 * @param dest The destination buffer into which the bytes will be
 * copied.
 * @param source The originating buffer from which the bytes will be
 * copied.
 * @param count How many bytes to copy.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtMemcpy (
   VtLibCtx libCtx,
   Pointer dest,
   Pointer source,
   unsigned int count
);

/** Copy count bytes from source to dest. The addresses may overlap.
 * <p>The underlying memory ctx will be called on to perform this
 * operation, and it might not check the size of the buffer. Hence, the
 * caller should not copy from or copy into unallocated memory.
 *
 * @param libCtx The libCtx containing the memory context to use when
 * memmoving memory.
 * @param dest The destination buffer into which the bytes will be
 * copied.
 * @param source The originating buffer from which the bytes will be
 * copied.
 * @param count How many bytes to move.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtMemmove (
   VtLibCtx libCtx,
   Pointer dest,
   Pointer source,
   unsigned int count
);

/** Compare count bytes in buffer1 and buffer2. This function sets the
 * int at the address given by cmpResult to the result of the
 * comparison.
 * <p>Set the result to 0 if the two buffers contain the same data. Set
 * it to 1 or -1 if they differ.
 * <p>If the cmpResult value is not 0, it is based on the first byte
 * that differs between the two buffers. If buffer1 > buffer2, set it
 * to 1, if buffer1 < buffer2, set it to -1.
 * <p>If the return from this function is not 0 (an error), then the
 * value of cmpResult is meaningless.
 * <p>The underlying memory ctx will be called on to perform this
 * operation, and it might not check the size of the buffer. Hence, the
 * caller should not ask the function to compare unallocated memory.
 *
 * @param libCtx The libCtx containing the memory context to use when
 * comparing memory.
 * @param buffer1 The first buffer to compare.
 * @param buffer2 The second buffer to compare.
 * @param count How many bytes to compare.
 * @param cmpResult The address where the function will deposit the
 * result of the comparison, -1, 0, or +1.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtMemcmp (
   VtLibCtx libCtx,
   Pointer buffer1,
   Pointer buffer2,
   unsigned int count,
   int *cmpResult
);

/*=========================================================*/
/*                                                         */
/* Toolkit Version                                         */
/*                                                         */
/*=========================================================*/

/** When requesting a version number, you will need to specify for which
 * library (vibecrypto, cibe, vibeproviders, etc.) the version number
 * is requested. Do that by passing a VtLibraryVersion.  Although it is
 * a function pointer, an application should never call a
 * VtLibraryVersion directly, only pass it as an argument to a
 * GetVersion function.
 */
typedef int VT_CALLING_CONV (VtLibraryVersion) (VtLibCtx, int *, char **);

/** This function returns the version number of a library in the
 * toolkit.
 * <p>There are several libraries (vibecrypto, vibe, vibeproviders,
 * etc.), this will determine the version of the specified library.
 * Specify which library with the whichLib argument. It must be one of
 * the VtLibraryVersions defined in vibecrypto.h or vibe.h.
 * <p>The function will set the int at the address given by
 * versionNumber to a version number as an integer. See the set of
 * #define VT_VERSION_NUMBER_* for an explanation of what each number
 * means.
 * <p>The function will also, if requested, return a string
 * representation of the version. The caller passes in an address, the
 * routine will deposit at that address a pointer to a NULL-terminated
 * ASCII string. The memory holding the string belongs to the toolkit,
 * do not free or alter it. If the caller passes a NULL, the function
 * will return only the integer version number.
 * <p>Note that this function returns an ASCII string, not a UTF-8
 * string.
 *
 * @param libCtx The library context to use.
 * @param whichLib A VtLibraryVersion, it indicates for which library
 * the version is requested.
 * @param versionNumber The address where the function will deposit the
 * version as an integer.
 * @param versionString The address where the function will deposit a
 * pointer to a NULL-terminated string representation of the version,
 * or NULL, indicating the function should return only the integer
 * representation of the version.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtGetVersionNumber (
   VtLibCtx libCtx,
   VtLibraryVersion whichLib,
   int *versionNumber,
   char **versionString
);

/* These are the VtLibraryVersions supported by vibecrypto.lib.
 */

/** Use this VtLibraryVersion to determine the version of "vibecrypto".
 */
VtLibraryVersion VtLibraryVersionCrypto;

/** Use this VtLibraryVersion to determine the version of
 * "vibecryptofips".
 */
VtLibraryVersion VtLibraryVersionCryptoFips;

/** The function VtGetVersionNumber returns this value if the toolkit
 * is version 2.0 Beta 1.
 */
#define VT_VERSION_NUMBER_2_0_BETA_1  200001

/** The function VtGetVersionNumber returns this value if the toolkit
 * is version 2.0 Beta 2.
 */
#define VT_VERSION_NUMBER_2_0_BETA_2  200002

/** The function VtGetVersionNumber returns this value if the toolkit
 * is version 2.0 Beta 3.
 */
#define VT_VERSION_NUMBER_2_0_BETA_3  200003

/** The function VtGetVersionNumber returns this value if the toolkit
 * is version 2.0.
 */
#define VT_VERSION_NUMBER_2_0         200020

/** The function VtGetVersionNumber returns this value if the toolkit
 * is version 2.1 Beta 1.
 */
#define VT_VERSION_NUMBER_2_1_BETA_1  201001

/** The function VtGetVersionNumber returns this value if the toolkit
 * is version 2.1 Beta 2.
 */
#define VT_VERSION_NUMBER_2_1_BETA_2  201002

/** The function VtGetVersionNumber returns this value if the toolkit
 * is version 2.1.
 */
#define VT_VERSION_NUMBER_2_1         201021

/** The function VtGetVersionNumber returns this value if the toolkit
 * is version 2.2.
 */
#define VT_VERSION_NUMBER_2_2         202022

/** The function VtGetVersionNumber returns this value if the toolkit
 * is version 2.3 Beta 1.
 */
#define VT_VERSION_NUMBER_2_3_BETA_1  203001

/** The function VtGetVersionNumber returns this value if the toolkit
 * is version 2.3.
 */
#define VT_VERSION_NUMBER_2_3         203023

/** The function VtGetVersionNumber returns this value if the toolkit
 * is version 2.5 Beta 1.
 */
#define VT_VERSION_NUMBER_2_5_BETA_1  205001

/*=========================================================*/
/*                                                         */
/* Multi-Precision Integer Arithmetic Context              */
/*                                                         */
/*=========================================================*/

/**
 * \defgroup MPIntGroup Multi-Precision Integer Arithmetic Context
 */

/*@{*/

/** The multi-precision integer context.
 * <p>Note that the context is a pointer type.
 */
typedef struct VtMpIntCtxDef *VtMpIntCtx;

/** The function VtCreateMpIntCtx builds an MpInt context using a
 * VtMpIntImpl. This typedef defines what a VtMpIntImpl is. Although
 * it is a function pointer, an application should never call a
 * VtMpIntImpl directly, only pass it as an argument to
 * VtCreateMpIntCtx.
 */
typedef int VT_CALLING_CONV (VtMpIntImpl) (
   VtMpIntCtx *, Pointer, unsigned int);

/** The function VtSetMpIntParam adds information to an MpInt ctx. The
 * information to add is defined by a VtMpIntParam. This typedef
 * defines what a VtMpIntParam is. Although a VtMpIntParam is a
 * function pointer, an application should never call one directly,
 * only pass it as an argument to VtSetMpIntParam.
 */
typedef int VT_CALLING_CONV (VtMpIntParam) (
   VtMpIntCtx, Pointer, unsigned int);

/** Create a new MpInt context. This allocates space for an "empty"
 * context, then loads the given MpIntImpl to make it an "active"
 * context.
 * <p>The VtMpIntImpl defines the MpInt implementation. The include
 * file vibe.h defines the supported MpIntImpls. Look through the
 * include file to see which MpIntImpl to use for your application.
 * All supported MpIntImpls will be defined as in the following
 * example.
 * <code>
 * <pre>
 *   extern VtMpIntImpl VtMpIntImplOpenSSL;
 * </pre>
 * </code>
 * <p>Associated with each MpIntImpl is specific info. The
 * documentation for each MpIntImpl will describe the associated info
 * it needs. That data could be another object, it could be data in a
 * particular struct, it might be a NULL pointer. Check the
 * documentation for each MpIntImpl for a description of the data and
 * its required format.
 * <p>To use this function decide which MpIntImpl you want to use,
 * then determine what information that MpIntImpl needs and in which
 * format it is presented. Collect the data in the appropriate format
 * then call this function passing in the desired MpIntImpl and the
 * required info. The associated info must be cast to Pointer.
 * <p>For example, if the required info is a VtItem, declare a variable
 * to be of type VtItem, fill in the fields, then pass the address of
 * that VtItem cast to Pointer.
 * <p>The input mpCtx is a pointer to a context. It should point to
 * a NULL VtMpIntCtx. This function will go to the address given and
 * deposit a created context.
 * <p>If you call Create, you must call Destroy after you are done with
 * the context but before the program exits.
 * <p>Example:
 * <code>
 * <pre>
 *    int status;
 *    VtMpIntCtx mpCtx = (VtMpIntCtx)0;
 *
 *    do {
 *          . . .
 *      status = VtCreateMpIntCtx (
 *        libCtx, VtMpIntImplOpenSSL, (Pointer)0, &mpCtx);
 *      if (status != 0)
 *        break;
 *          . . .
 *    } while (0);
 *
 *    VtDestroyMpIntCtx (&mpCtx);
 * </pre>
 * </code>
 * @param libCtx The library context.
 * @param mpIntImpl The implementation the context will use.
 * @param associatedInfo The info needed by the MpIntImpl.
 * @param mpCtx A pointer to where the routine will deposit the
 * created context.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtCreateMpIntCtx (
   VtLibCtx libCtx,
   VtMpIntImpl mpIntImpl,
   Pointer associatedInfo,
   VtMpIntCtx *mpCtx
);

/* These are the VtMpIntImpls supported by the toolkit. Each
 * VtMpIntImpl is used in conjunction with special info for the
 * function. If there is no special info, the accompaniment is a NULL
 * pointer.
 */

/** This VtMpIntImpl is used to set an MpIntCtx with a
 * multi-precision implementation when performing FIPS work.
 * <p>This MpIntImpl is available only in the FIPS version of the
 * toolkit and not available in any other version of the toolkit.
 * <p>The data associated with VtMpIntImplFips is NULL pointer:
 * (Pointer)0.
 */
extern VtMpIntImpl VtMpIntImplFips;

/** Destroy the multi-precision arithmetic context.  This frees up any
 * memory allocated during the context's creation and use.
 * <p>If you call Create, you must call Destroy after you are done with
 * the context but before the program exits.
 * <p>Example:
 * <code>
 * <pre>
 *    int status;
 *    VtMpIntCtx mpCtx = (VtMpIntCtx)0;
 *
 *    do {
 *          . . .
 *      status = VtCreateMpIntCtx (
 *        libCtx, VtMpIntImplOpenSSL, (Pointer)0, &mpCtx);
 *      if (status != 0)
 *        break;
 *          . . .
 *    } while (0);
 *
 *    VtDestroyMpIntCtx (&mpCtx);
 * </pre>
 * </code>
 * @param mpCtx A pointer to where the routine will find the context to
 * destroy.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtDestroyMpIntCtx (
   VtMpIntCtx *mpCtx
);

/** Set the MpInt context with the information given.
 * <p>The VtMpIntParam defines what information the ctx will be set
 * with.
 * <p>The include file vibe.h defines the supported MpIntParams.
 * Look through the include file to see which MpIntParam to use for
 * your application. All supported MpIntParams will be defined as in
 * the following example.
 * <code>
 * <pre>
 *   extern VtMpIntParam VtMpIntInfo;
 * </pre>
 * </code>
 * <p>Associated with each MpIntParam is specific info. The
 * documentation for each MpIntParam will describe the associated info
 * it needs. That data could be another object, it could be data in a
 * particular struct, it might be a NULL pointer. Check the
 * documentation for each MpIntParam for a description of the data and
 * its required format.
 * <p>To use this function decide which MpIntParam you want to use,
 * then determine what information that MpIntParam needs and in which
 * format it is presented. Collect the data in the appropriate format
 * then call this function passing in the desired MpIntParam and the
 * required info. The associated info must be cast to Pointer.
 * <p>For example, if the required info is a char array (a
 * NULL-terminated string), declare a variable to be of type char *,
 * set it to point to the NULL-terminated string, then pass that char *
 * cast to Pointer.
 *
 * @param mpCtx The context to set.
 * @param mpIntParam What the ctx is being set to.
 * @param associatedInfo The info needed by the MpIntParam.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtSetMpIntParam (
   VtMpIntCtx mpCtx,
   VtMpIntParam mpIntParam,
   Pointer associatedInfo
);

/* There are currently no VtMpIntParams supported by the toolkit.
 */

/*@}*/

/*=========================================================*/
/*                                                         */
/* IBE Cache Context                                       */
/*                                                         */
/*=========================================================*/

/**
 * \defgroup IBECacheCtxGroup IBE Cache Context
 */

/*@{*/

/** The IBE cache.
 * <p>Note that the cache is a pointer type.
 */
typedef struct VtIBECacheCtxDef *VtIBECacheCtx;

/** The function VtCreateIBECacheCtx builds an IBE cache using a
 * VtIBECacheCtxImpl. This typedef defines what a VtIBECacheCtxImpl is.
 * Although it is a function pointer, an application should never call
 * a VtIBECacheCtxImpl directly, only pass it as an argument to
 * VtCreateIBECacheCtx.
 */
typedef int VT_CALLING_CONV (VtIBECacheCtxImpl) (
   VtIBECacheCtx *, Pointer, unsigned int);

/** Create a new IBE Cache Ctx. This allocates space for an "empty"
 * ctx, then loads the given IBECacheCtxImpl to make it "active".
 * <p>Currently there is only one thing you can do with an IBE Cache
 * Ctx, load it into the libCtx (see VtSetLibCtxParam). The IBE Cache
 * Ctx loaded into the libCtx will allow many IBE operations to execute
 * faster, although the code size and memory usage of the application
 * will increase.
 * <p>The VtIBECacheCtxImpl defines the caching implementation. The
 * include file vibe.h defines the supported IBECacheCtxImpls. Look
 * through the include file to see which IBECacheCtxImpl to use for your
 * application. All supported IBECacheCtxImpls will be defined as in
 * the following example.
 * <code>
 * <pre>
 *   extern VtIBECacheCtxImpl VtIBECacheCtxImplBasic;
 * </pre>
 * </code>
 * <p>Associated with each IBECacheCtxImpl is specific info. The
 * documentation for each IBECacheCtxImpl will describe the associated
 * info it needs. That data could be another object, it could be data
 * in a particular struct, it might be a NULL pointer. Check the
 * documentation for each IBECacheCtxImpl for a description of the data
 * and its required format.
 * <p>The input ibeCacheCtx is a pointer to a ccach object. It should
 * point to a NULL VtIBECacheCtx. This function will go to the address
 * given and deposit a created context.
 * <p>If you call Create, you must call Destroy after you are done with
 * the cache but before the program exits.
 * <p>Example:
 * <code>
 * <pre>
 *    int status;
 *    VtIBECacheCtx ibeCacheCtx = (VtIBECacheCtx)0; 
 *
 *    do {
 *          . . . 
 *      status = VtCreateIBECacheCtx (
 *        libCtx, VtIBECacheCtxImplBasic, (Pointer)0, &ibeCacheCtx);
 *      if (status != 0)
 *        break;
 *          . . .
 *    } while (0);
 * 
 *    VtDestroyIBECacheCtx (&ibeCacheCtx);
 * </pre>
 * </code>
 * @param libCtx The library context.
 * @param ibeCacheCtxImpl The implementation the cache ctx will use.
 * @param associatedInfo The info needed by the IBECacheCtxImpl.
 * @param ibeCacheCtx A pointer to where the routine will deposit the
 * created context.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtCreateIBECacheCtx (
   VtLibCtx libCtx,
   VtIBECacheCtxImpl ibeCacheCtxImpl,
   Pointer associatedInfo,
   VtIBECacheCtx *ibeCacheCtx
);

/* These are the VtIBECacheCtxImpls supported by the toolkit. Each
 * IBECacheCtxImpl is used in conjunction with special info for the
 * function. If there is no special info, the accompaniment is a NULL
 * pointer.
 */

/** This VtIBECacheCtxImpl is used to build an IBE Cache that performs
 * basic caching operations on all platforms.
 * <p>The data associated with VtIBECacheCtxImplBasic is a NULL
 * pointer.
 */
extern VtIBECacheCtxImpl VtIBECacheCtxImplBasic;

/** Destroy the IBE Cache Ctx.  This frees up any memory allocated
 * during the context's creation and use.
 * <p>If you call Create, you must call Destroy after you are done with
 * the cache but before the program exits.
 * <p>Example:
 * <code>
 * <pre>
 *    int status;
 *    VtIBECacheCtx ibeCacheCtx = (VtIBECacheCtx)0; 
 *
 *    do {
 *          . . . 
 *      status = VtCreateIBECacheCtx (
 *        libCtx, VtIBECacheCtxImplBasic, (Pointer)0, &ibeCacheCtx);
 *      if (status != 0)
 *        break;
 *          . . .
 *    } while (0);
 * 
 *    VtDestroyIBECacheCtx (&ibeCacheCtx);
 * </pre>
 * </code>
 * @param ibeCacheCtx A pointer to where the routine will find the ctx
 * to destroy.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtDestroyIBECacheCtx (
   VtIBECacheCtx *ibeCacheCtx
);

/** How many elements are there currently in the IBE Cache that is
 * loaded in the libCtx?
 * <p>Note that this only gets the count from a cache ctx in the
 * libCtx, there is no way to get a count from a ctx not loaded in a
 * libCtx.
 * <p>The function will set the unsigned int at the address count with
 * the number of elements in the cache.
 * <p>The elements inside are numbered from 0 to count - 1.
 *
 * @param libCtx The libCtx containing the IBE cache to query.
 * @param count The address where this function will deposit the count.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtGetIBECacheElementCount (
   VtLibCtx libCtx,
   unsigned int *count
);

/** Get the cache element at the given index.
 * <p>NOTE!!! A cache element can be very large. With the
 * Boneh-Franklin version of IBE (BF), it can be more than 64K bytes.
 * The larger the prime the larger the cache element. It might be more
 * than 128K bytes. With the Boneh-Boyen version of IBE (BB), it can be
 * more than 190K bytes for smaller primes up to more than 360K bytes
 * for larger primes.
 * <p>Note also that this only gets an element from a cache ctx in the
 * libCtx, there is no way to get an element from a ctx not loaded in a
 * libCtx.
 * <p>If there is no element at that index, the function will return
 * VT_ERROR_NO_ELEMENT_AT_INDEX. To determine what indices are valid,
 * call VtGetIBECacheCount.
 * <p>The function will return the data in the caller-supplied buffer.
 * If the buffer is not big enough, the routine will return the
 * "BUFFER_TOO_SMALL" error and set the unsigned int at elementLen to
 * the needed size.
 * <p>The entry is a single byte array. Inside the byte array is an
 * identifier indicating which kind of IBE info is represented (BF Type
 * 1 or BB Type 1), a reference against which it is possible to
 * determine which parameter set the element represents, the actual
 * parameter set (prime, subprime, base point, etc.), and an
 * acceleration table. Call VtGetIBECacheElementReference to get a flag
 * indicating the kind of IBE and the reference.
 *
 * @param libCtx The libCtx containing the IBE cache to query.
 * @param index The index of the element to retrieve.
 * @param element The buffer into which the function will place the
 * result (if NULL, the function will return BUFFER_TOO_SMALL and set
 * the elementLen to the size needed).
 * @param bufferSize The size, in bytes, of the element buffer.
 * @param elementLen The address where the function will deposit the
 * length (in bytes). This is either the size needed (if the buffer is
 * not big enough) or the number of bytes placed into the output buffer.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtGetIBECacheElement (
   VtLibCtx libCtx,
   unsigned int index,
   unsigned char *element,
   unsigned int bufferSize,
   unsigned int *elementLen
);

/** Set the cache with the element given.
 * <p>Note that this only sets an element in a cache ctx in the libCtx,
 * there is no way to set an element in a ctx not loaded in a libCtx.
 * <p>The function will create a new element in the IBE cache and copy
 * the information from the provided element.
 * <p>If the element is already in the cache, the function will not
 * create a new copy of the element, it will simply return the index.
 * <p>The function will return to the caller the index inside the cache
 * of the element. Because the function might have found the element
 * already in the cache, the index returned does not necessarily
 * indicate the total count of elements in the cache.
 *
 * @param libCtx The libCtx containing the IBE cache to set.
 * @param mpCtx The mpCtx to be used for IBE math operations.
 * @param element The element to add.
 * @param elementLen The length, in bytes, of the element.
 * @param index The address where the function will deposit the index
 * inside the cache of the new element.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtSetIBECacheElement (
   VtLibCtx libCtx,
   VtMpIntCtx mpCtx,
   unsigned char *element,
   unsigned int elementLen,
   unsigned int *index
);

/** Given an IBE Cache element, "extract" the flag indicating what type
 * of IBE the element represents (BF Type 1 or BB Type 1), and the
 * reference.
 * <p>This function is for when you have an IBE Cache element, but want
 * to isolate individual elements. For example, the type and reference
 * will be useful for storing and retrieving cache elements, and for
 * finding an appropriate element for a particular parameter set.
 * <p>The caller supplies the element as a byte array and its length.
 * The caller also supplies the address of an unsigned int where the
 * function will deposit the flag indicating the type, the address of
 * an unsigned char pointer where the function will deposit the pointer
 * to the reference, and the address of an unsigned int where the
 * function will deposit the length of the reference.
 * <p>The flag will be one of the following two values.
 * <pre>
 * <code>
 *   VT_IBE_CACHE_ENTRY_BF_TYPE1
 *   VT_IBE_CACHE_ENTRY_BB_TYPE1
 * </code>
 * </pre>
 * <p>If the flag is VT_IBE_CACHE_ENTRY_BF_TYPE1, the reference will be
 * the base point and public point (4 coordinates as a byte array).
 * <p>If the flag is VT_IBE_CACHE_ENTRY_BB_TYPE1, the reference will be
 * the base point and public point Alpha (4 coordinates as a byte array).
 * <p>The returned reference is simply the address inside the element
 * where the reference begins.
 *
 * @param libCtx The libCtx containing the IBE cache.
 * @param element The byte array with the element to query.
 * @param elementLen The length, in bytes, of the element.
 * @param flag The address where the routine will deposit a flag
 * indicating what type of IBE the element represents.
 * @param reference The address where the routine will deposit the
 * pointer to the reference (the pointer will point to some location
 * inside the element).
 * @param referenceLen The address where the routine will deposit the
 * length of the reference.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtGetIBECacheElementReference (
   VtLibCtx libCtx,
   unsigned char *element,
   unsigned int elementLen,
   unsigned int *flag,
   unsigned char **reference,
   unsigned int *referenceLen
);

/** This flag indicates that an IBE Cache entry represents a BF Type 1
 * IBE parameter set.
 */
#define VT_IBE_CACHE_ENTRY_BF_TYPE1  0xBF1
/** This flag indicates that an IBE Cache entry represents a BB Type 1
 * IBE parameter set.
 */
#define VT_IBE_CACHE_ENTRY_BB_TYPE1  0xBB1

/** Delete the element at the given index from the cache.
 * <p>Note that this only deletes an element from a cache ctx in the
 * libCtx, there is no way to delete an element in a ctx not loaded in
 * a libCtx.
 * <p>If there is no element at that index, the function will return
 * VT_ERROR_NO_ELEMENT_AT_INDEX. To determine what indices are valid,
 * call VtGetIBECacheCount.
 * <p>If some object is still using the entry, the function will not
 * delete it, instead it will return the error VT_ERROR_INVALID_DELETE.
 * For example, if a parameter object is built using the params in a
 * particular element, or an algorithm object is initialized
 * (EncryptInit, DecryptInit) using a key that uses the params in the
 * element, then the element is still in use. After those objects are
 * destroyed (or reinitialized), they will not be using the element. 
 * <p>Note that when the cache ctx is destroyed, all elements are
 * deleted. This function is simply a way to reduce the size of the
 * cache if that becomes desirable.
 * <p>If an element is deleted, other elements inside the cache might
 * be re-indexed. For example, if a cache contains 4 elements (numbered
 * 0, 1, 2, and 3) and you delete the element at index 2, the element
 * that was indexed 3 will now be indexed 2.
 *
 * @param libCtx The libCtx containing the IBE cache from which the
 * element is to be deleted.
 * @param index The index of the element to delete.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtDeleteIBECacheElement (
   VtLibCtx libCtx,
   unsigned int index
);

/*@}*/

/*=========================================================*/
/*                                                         */
/* Random Object                                           */
/*                                                         */
/*=========================================================*/

/**
 * \defgroup RandomGroup Random Object
 */

/*@{*/

/** The random object.
 * <p>Note that the object is a pointer type.
 */
typedef struct VtRandomObjectDef *VtRandomObject;

/** The function VtCreateRandomObject builds a random object using
 * a VtRandomImpl. This typedef defines what a VtRandomImpl is.
 * Although a VtRandomImpl is a function pointer, an application should
 * never call one directly, only pass it as an argument to
 * VtCreateRandomObject.
 */
typedef int VT_CALLING_CONV (VtRandomImpl) (
   VtRandomObject *, Pointer, unsigned int);

/** The function VtSetRandomParam adds information to a random object.
 * The information to add is defined by a VtRandomParam. This typedef
 * defines what a VtRandomParam is. Although a VtRandomParam is a
 * function pointer, an application should never call one directly,
 * only pass it as an argument to VtSetRandomParam.
 */
typedef int VT_CALLING_CONV (VtRandomParam) (
   VtRandomObject, Pointer, unsigned int);

/** Create a new random object. This allocates space for an "empty"
 * object, then loads the given RandomImpl to make it an "active"
 * object.
 * <p>The VtRandomImpl defines the random implementation. The include
 * file vibe.h defines the supported RandomImpls. Look through the
 * include file to see which RandomImpl to use for your application.
 * All supported RandomImpls will be defined as in the following
 * example.
 * <code>
 * <pre>
 *   extern VtRandomImpl VtRandomImplFips186Prng;
 * </pre>
 * </code>
 * <p>Associated with each RandomImpl is specific info. The
 * documentation for each RandomImpl will describe the associated
 * info it needs. That data could be another object, it could be data
 * in a particular struct, it might be a NULL pointer. Check the
 * documentation for each RandomImpl for a description of the data
 * and its required format.
 * <p>To use this function decide which RandomImpl you want to use,
 * then determine what information that RandomImpl needs and in which
 * format it is presented. Collect the data in the appropriate format
 * then call this function passing in the desired RandomImpl and the
 * required info. The associated info must be cast to Pointer.
 * <p>For example, if the required info is a VtItem struct, declare a
 * variable to be of type VtItem, fill in the fields, then pass the address
 * of that struct cast to Pointer.
 * <p>The input randObj is a pointer to an object. It should point to
 * a NULL VtRandomObject. This function will go to the address given
 * and deposit a created object.
 * <p>If you call Create, you must call Destroy after you are done with
 * the object but before the program exits.
 * <p>Example:
 * <code>
 * <pre>
 *    int status;
 *    VtRandomObject randObj = (VtRandomObject)0;
 *    VtFips186PrngInfo fipsPrngInfo;
 *
 *    do {
 *          . . .
 *      // Init all fields to NULL or 0.
 *      memset (&fipsPrngInfo, 0, sizeof (fipsPrngInfo));
 *      fipsPrngInfo.variation = FIPS_186_PRNG_3_1;
 *      fipsPrngInfo.XKEY.data = someSeedMaterial;
 *      fipsPrngInfo.XKEY.len = 32;
 *      // Assume we have an MpIntCtx built.
 *      fipsPrngInfo.mpCtx = mpCtx;
 *      status = VtCreateRandomObject (
 *        libCtx, VtRandomImplFips186Prng, (Pointer)&fipsPrngInfo, &randObj);
 *      if (status != 0)
 *        break;
 *          . . .
 *    } while (0);
 *
 *    VtDestroyRandomObject (&randObj);
 * </pre>
 * </code>
 * @param libCtx The library context.
 * @param randomImpl The implementation the object will use.
 * @param associatedInfo The info needed by the RandomImpl.
 * @param randObj A pointer to where the routine will deposit the
 * created object.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtCreateRandomObject (
   VtLibCtx libCtx,
   VtRandomImpl randomImpl,
   Pointer associatedInfo,
   VtRandomObject *randObj
);

/* These are the VtRandomImpls supported by the toolkit. Each
 * VtRandomImpl is used in conjunction with special info for the
 * function. If there is no special info, the accompaniment is a NULL
 * pointer.
 */

/** This VtRandomImpl is used to set a random object to perform
 * pseudo-random number generation according to the specifications in
 * FIPS PUB 186-2. The data associated with VtRandomImplFips186Prng is
 * a pointer to a VtFips186PrngInfo struct.
 * <p>FIPS 186-2 describes how to generate private key x values and
 * signature k values for DSA. It also specifies a way to generate
 * "general purpose" random bytes. This Impl generates "general
 * purpose" random bytes. When generating x's and k's, the toolkit will
 * use the PRNG given only to generate "XSEED" and "XKEY" or "KSEED"
 * and "KKEY" material. It will then use those values to generate the
 * x's and k's as specified. In other words, your application does not
 * need to build separate objects for separate purposes. One object
 * built using this Impl in one particular way can be used to generate
 * DSA key pairs and create DSA signatures.
 * <p>Another way to put it is that if you want your application to be
 * FIPS certified, you must follow certain rules concerning x and k
 * generation. However, the toolkit follows those rules internally. The
 * random object you pass in is not the determining factor in proper
 * FIPS value generation. Hence, so long as you pass in a valid random
 * object seeded sufficiently (any variation) the toolkit will generate
 * x's and k's appropriately.
 * <p>As a general-purpose PRNG, this Impl is FIPS-certified as well.
 * <p>This algorithm reqires at least 32 bytes of seed material. More
 * seed material, is, of course, allowed.
 */
extern VtRandomImpl VtRandomImplFips186Prng;

/** This is the data struct used to accompany VtRandomImplFips186Prng.
 * The fields of this struct are there for internal use mainly. This
 * PRNG is designed so we can obtain FIPS certification. If you supply
 * a NULL primeQ, then your application will be generating random bytes
 * in a FIPS-certified manner.
 * <p>XKEY is the initial state of the object. It is seed material. It
 * must be supplied. For this implementation it must be 32 bytes, no
 * more, no less.
 * <p>The primeQ is optional. It is there for internal FIPS
 * certification purposes. If no primeQ is supplied, the algorithm will
 * still work. Voltage recommends passing in no primeQ (a NULL with
 * length 0).
 * <p>However, if you do supply a primeQ, it must be 160 bits long. If
 * you do pass in a primeQ, the toolkit will not check the value to
 * determine if it is indeed a prime or not, it will simply make sure
 * it is the appropriate size.
 * <p>If no primeQ is supplied, it is not necessary to supply an
 * MpIntCtx.
 * <p>The variation will generally not be important in applications and
 * is used primarily for FIPS certification. The value of variation
 * must be one of the following
 * <code>
 * <pre>
 *    FIPS_186_PRNG_3_1_ALG
 *    FIPS_186_PRNG_3_1_CERTIFY
 *    FIPS_186_PRNG_3_2_ALG
 *    FIPS_186_PRNG_3_2_CERTIFY
 * </pre>
 * </code>
 * <p>Voltage recommends using FIPS_186_PRNG_3_1_CERTIFY.
 * <p>The following description explains the differences among the
 * variations, but most applications will not need to use any variation
 * other than the recommended.
 * <p>There are two algorithms given in FIPS 186-2, they are in
 * Appendix 3, section 3.1 and 3.2. The 3.1 variation is for generating
 * private key values (also known as "x") of DSA key pairs and the 3.2
 * variation is for generating k values when creating DSA signatures.
 * <p>Each of these is further broken into "ALG" and "CERTIFY". One
 * part of the FIPS certification process demands that the PRNG
 * implementation "throw away" the first block of generated values. This
 * is how FIPS knows implementations thwart a particular attack. This
 * is the "CERTIFY" version.
 * <p>However, another part of the FIPS certification process demands
 * that the PRNG implementation does NOT "throw away" the first block
 * of generated values. This is the "ALG" version (it performs the
 * algorithm only, it does not add extra protection).
 * <p>Hence, to obtain FIPS certification, our PRNG must be able to
 * generate random values either way, throwing away the first block and
 * not throwing away the first block. The ALG and CERTIFY versions
 * determine whether the first block is thrown away or not.
 */
typedef struct
{
  /** Must be 32 bytes, no more, no less.
   */
  VtItem XKEY;
  /** For most applications, pass a NULL primeQ (length 0). If you pass
   * in primeQ, it must be exactly 160 bits (20 bytes with the most
   * significant bit set). Voltage recommends passing a NULL.
   */
  VtItem primeQ;
  /** If you pass in a primeQ, there must be an mpCtx, if not here in
   * the info struct, then in the libCtx. If there is no primeQ, then
   * there is no need for an mpCtx (this is a change from previous
   * versions of the toolkit).
   */
  VtMpIntCtx mpCtx;
  /** Voltage recommends FIPS_186_PRNG_3_1_CERTIFY. Other variations
   * are used internally for FIPS certification. It is not necessary to
   * use specific variations for specific purposes, the toolkit takes
   * care of that.
   */
  unsigned int variation;
} VtFips186PrngInfo;

/** Set this bit to indicate "CERTIFY". Make sure this bit position is
 * used only for this purpose in FIPS_186_PRNG flags. This is used so
 * the toolkit can know whether to throw away the first block when
 * performing FIPS certification. Apps will almost certainly never need
 * to explicitly use this flag.
 */
#define FIPS_186_CERTIFY           0x800
/** For use in VtFips186PrngInfo, indicate that the object should
 * follow the algorithm in FIPS 186, section 3.1. Do not throw away the
 * first generated block.
 */
#define FIPS_186_PRNG_3_1_ALG      0x031
/** For use in VtFips186PrngInfo, indicate that the object should
 * follow the algorithm in FIPS 186, section 3.1. Throw away the first
 * generated block.
 */
#define FIPS_186_PRNG_3_1_CERTIFY  FIPS_186_PRNG_3_1_ALG+FIPS_186_CERTIFY
/** For use in VtFips186PrngInfo, indicate that the object should
 * follow the algorithm in FIPS 186, section 3.2. Do not throw away the
 * first generated block.
 */
#define FIPS_186_PRNG_3_2_ALG      0x032
/** For use in VtFips186PrngInfo, indicate that the object should
 * follow the algorithm in FIPS 186, section 3.2. Throw away the first
 * generated block.
 */
#define FIPS_186_PRNG_3_2_CERTIFY  FIPS_186_PRNG_3_2_ALG+FIPS_186_CERTIFY

/** Destroy a random Object. This frees up any memory allocated during
 * the object's creation and use.
 * <p>If you call Create, you must call Destroy after you are done with
 * the object but before the program exits.
 * <p>Example:
 * <code>
 * <pre>
 *    int status;
 *    VtRandomObject randObj = (VtRandomObject)0;
 *    VtFips186PrngInfo fipsPrngInfo;
 *
 *    do {
 *          . . .
 *      // Init all fields to NULL or 0: primeP and mpCtx will remain
 *      // NULL.
 *      memset (&fipsPrngInfo, 0, sizeof (fipsPrngInfo));
 *      fipsPrngInfo.variation = FIPS_186_PRNG_3_1;
 *      fipsPrngInfo.XKEY.data = someSeedMaterial;
 *      fipsPrngInfo.XKEY.len = 32;
 *      status = VtCreateRandomObject (
 *        libCtx, VtRandomImplFips186Prng, (Pointer)&fipsPrngInfo, &randObj);
 *      if (status != 0)
 *        break;
 *          . . .
 *    } while (0);
 *
 *    VtDestroyRandomObject (&randObj);
 * </pre>
 * </code>
 * @param randObj A pointer to where the routine will find the object
 * to destroy.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtDestroyRandomObject (
   VtRandomObject *randObj
);

/** Set the random object with the info given.
 * <p>The VtRandomParam defines what information the object will be set
 * with.
 * <p>The include file vibe.h defines the supported RandomParams.
 * Look through the include file to see which RandomParam to use for
 * your application. All supported RandomParams will be defined as in
 * the following example.
 * <code>
 * <pre>
 *   extern VtRandomParam VtRandomInfo;
 * </pre>
 * </code>
 * <p>Associated with each RandomParam is specific info. The
 * documentation for each RandomParam will describe the associated info
 * it needs. That data could be another object, it could be data in a
 * particular struct, it might be a NULL pointer. Check the
 * documentation for each RandomParam for a description of the data and
 * its required format.
 * <p>To use this function decide which RandomParam you want to use,
 * then determine what information that RandomParam needs and in which
 * format it is presented. Collect the data in the appropriate format
 * then call this function passing in the desired RandomParam and the
 * required info. The associated info must be cast to Pointer.
 * <p>For example, if the required info is a particular struct, declare
 * a variable to be the appropriate struct, set the fields with the
 * appropriate information, then pass the address of that struct cast
 * to Pointer.
 *
 * @param randObj The object to set.
 * @param randomParam What the object is being set to.
 * @param associatedInfo The info needed by the RandomParam.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtSetRandomParam (
   VtRandomObject randObj,
   VtRandomParam randomParam,
   Pointer associatedInfo
);

/* There are currently no VtRandomParams supported by the toolkit.
 */

/** Add seed bytes to the random object. This updates the object, no
 * matter what. Even if there has been a call to Generate since the
 * last call to Seed, each call to Seed updates the object.
 *
 * @param randObj The object to seed.
 * @param seedData The buffer containing the seed bytes.
 * @param seedLen The length, in bytes, of the seedData.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtSeedRandom (
   VtRandomObject randObj,
   unsigned char *seedData,
   unsigned int seedLen
);

/** Generate the requested number of random bytes.
 * <p>If randomLen is 0, the function will do nothing.
 * <p>If randomLen is not 0 and randomBytes is NULL, the function will
 * return an error.
 *
 * @param randObj The random object.
 * @param randomBytes The buffer into which the function will deposit
 * the bytes.
 * @param randomLen The number of bytes to generate.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtGenerateRandomBytes (
   VtRandomObject randObj,
   unsigned char *randomBytes,
   unsigned int randomLen
);

/*@}*/

/*=========================================================*/
/*                                                         */
/* Algorithm Object                                        */
/*                                                         */
/*=========================================================*/

/**
 * \defgroup AlgorithmGroup Algorithm Object
 */

/*@{*/

/** The algorithm object.
 * <p>Note that the object is a pointer type.
 */
typedef struct VtAlgorithmObjectDef *VtAlgorithmObject;

/** The function VtCreateAlgorithmObject builds an algorithm object
 * using a VtAlgorithmImpl. This typedef defines what a VtAlgorithmImpl
 * is. Although a VtAlgorithmImpl is a function pointer, an application
 * should never call one directly, only pass it as an argument to
 * VtCreateAlgorithmObject.
 */
typedef int VT_CALLING_CONV (VtAlgorithmImpl) (
   VtAlgorithmObject *, Pointer, unsigned int);

/** The function VtSetAlgorithmParam adds information to an algorithm
 * object. The information to add is defined by a VtAlgorithmParam. This
 * typedef defines what a VtAlgorithmParam is. Although a
 * VtAlgorithmParam is a function pointer, an application should never
 * call one directly, only pass it as an argument to
 * VtSetAlgorithmParam.
 */
typedef int VT_CALLING_CONV (VtAlgorithmParam) (
   VtAlgorithmObject, Pointer, unsigned int);

/** Forward referencing for the VtFeedbackMode. Applications can
 * ignore this data type.
 */
typedef struct VtFeedbackInfoDef VtFeedbackInfo;

/** Forward referencing for the VtPaddingScheme. Applications can
 * ignore this data type.
 */
typedef struct VtPaddingInfoDef VtPaddingInfo;

/** When setting an algorithm object to a block cipher (DES, Triple
 * DES, AES), a feedback mode is needed. This is how a feedback mode is
 * defined. Although a VtFeedbackMode is a function pointer, an
 * application should never call one directly.
 */
typedef int VT_CALLING_CONV (VtFeedbackMode) (
   VtAlgorithmObject, VtFeedbackInfo *, unsigned int);

/** When setting an algorithm object to a cipher, a padding scheme is
 * often needed. This is how a padding scheme is defined. Although a
 * VtPaddingScheme is a function pointer, an application should never
 * call one directly.
 */
typedef int VT_CALLING_CONV (VtPaddingScheme) (
   VtAlgorithmObject, VtPaddingInfo *, unsigned int);

/** Create a new algorithm object. This allocates space for an "empty"
 * object, then loads the given AlgorithmImpl to make it an "active"
 * object.
 * <p>The VtAlgorithmImpl defines the algorithm implementation. The
 * include file vibe.h defines the supported AlgorithmImpls. Look
 * through the include file to see which AlgorithmImpl to use for your
 * application. All supported AlgorithmImpls will be defined as in the
 * following example.
 * <code>
 * <pre>
 *   extern VtAlgorithmImpl VtAlgorithmImplSHA1;
 * </pre>
 * </code>
 * <p>Associated with each AlgorithmImpl is specific info. The
 * documentation for each AlgorithmImpl will describe the associated
 * info it needs. That data could be another object, it could be data
 * in a particular struct, it might be a NULL pointer. Check the
 * documentation for each AlgorithmImpl for a description of the data
 * and its required format.
 * <p>To use this function decide which AlgorithmImpl you want to use,
 * then determine what information that AlgorithmImpl needs and in
 * which format it is presented. Collect the data in the appropriate
 * format then call this function passing in the desired AlgorithmImpl
 * and the required info. The associated info must be cast to Pointer.
 * <p>For example, if the required info is a VtItem struct, declare a
 * variable to be of type VtItem, fill in the fields, then pass the address
 * of that struct cast to Pointer.
 * <p>The input algObj is a pointer to an object. It should point to
 * a NULL VtAlgorithmObject. This function will go to the address given
 * and deposit a created object.
 * <p>If you call Create, you must call Destroy after you are done with
 * the object but before the program exits.
 * <p>Example:
 * <code>
 * <pre>
 *    int status;
 *    VtAlgorithmObject algObj = (VtAlgorithmObject)0;
 *
 *    do {
 *          . . .
 *      status = VtCreateAlgorithmObject (
 *        libCtx, VtAlgorithmImplSHA1, (Pointer)0, &algObj);
 *      if (status != 0)
 *        break;
 *          . . .
 *    } while (0);
 *
 *    VtDestroyAlgorithmObject (&algObj);
 * </pre>
 * </code>
 * @param libCtx The library context.
 * @param algorithmImpl The implementation the object will use.
 * @param associatedInfo The info needed by the AlgorithmImpl.
 * @param algObj A pointer to where the routine will deposit the
 * created object.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtCreateAlgorithmObject (
   VtLibCtx libCtx,
   VtAlgorithmImpl algorithmImpl,
   Pointer associatedInfo,
   VtAlgorithmObject *algObj
);

/* These are the VtAlgorithmImpls supported by the toolkit. Each
 * AlgorithmImpl is used in conjunction with special info for the
 * function. If there is no special info, the accompaniment is a NULL
 * pointer.
 */

/** This VtAlgorithmImpl is used to set an algorithm object to
 * perform IBE encryption or decryption.
 * <p>When using this Impl, you are must call only EncryptInit and
 * EncryptFinal, or DecryptInit and DecryptFinal (Update is not
 * allowed). You must have all the data to process in one buffer. This
 * will generally not be a burdon, because IBE is generally used to
 * encrypt symmetric keys, which are usually 16 to 32 bytes long.
 * <p>The data associated with VtAlgorithmImplBFType1IBE is a NULL
 * pointer: (Pointer)0.
 */
extern VtAlgorithmImpl VtAlgorithmImplBFType1IBE;

/** This VtAlgorithmImpl is used to set an algorithm object to
 * perform BB IBE encryption or decryption.
 * <p>Note that there are currently two versions of IBE implemented in
 * the toolkit, BF for Boneh-Franklin, and BB for Boneh-Boyen.
 * <p>When using this Impl, you are must call only EncryptInit and
 * EncryptFinal, or DecryptInit and DecryptFinal (Update is not
 * allowed). You must have all the data to process in one buffer. This
 * will generally not be a burdon, because IBE is generally used to
 * encrypt symmetric keys, which are usually 16 to 32 bytes long.
 * <p>The data associated with VtAlgorithmImplBBType1IBE is a NULL
 * pointer: (Pointer)0.
 */
extern VtAlgorithmImpl VtAlgorithmImplBBType1IBE;

/** This VtAlgorithmImpl is used to set an algorithm object to
 * sign using DSA. Currently, the only digest algorithm allowed with
 * DSA is SHA-1.
 * <p>The data associated with VtAlgorithmImplDSASign is a pointer to an
 * unsigned int, a flag indicating the format of the signature,
 * <code>
 * <pre>
 *   VT_DSA_SIGNATURE_R_S
 *   VT_DSA_SIGNATURE_DER_ENCODED
 * </pre>
 * </code>
 * <p>If _R_S, the signature will be the r value concatenated with the
 * s value, both of which are the same size as the subprime (currently
 * only a 160-bit subprime is supported). If _DER_ENCODED, the
 * signature will be the DER encoding of the ASN.1 definition
 * <code>
 * <pre>
 *    Dss-Sig-Value  ::=  SEQUENCE  {
 *       r       INTEGER,
 *       s       INTEGER  }
 * </pre>
 * </code>
 * <p>The ASN.1 definition is the way most (if not all) standards specify
 * the presentation of a DSA signature.
 * <p>Note that an algorithm built with this Impl does not perform
 * digesting. The Voltage toolkit sign and verify API take as input the
 * digest of the data to sign, not the data itself.
 * <p>The DSA standards specifies using SHA-1 as the digest algorithm
 * to use with DSA. The toolkit currently supports only SHA-1 as the
 * digest algorithm. The digest input to VtSign must be 20 bytes. The
 * digestAlg must be VT_DIGEST_ALG_SHA1.
 */
extern VtAlgorithmImpl VtAlgorithmImplDSASign;

/** For use with VtAlgorithmImplDSASign, indicate that the
 * implementation should produce the signature as the concatenation of
 * r and s.
 */
#define VT_DSA_SIGNATURE_R_S           1
/** For use with VtAlgorithmImplDSASign, indicate that the implementation
 * should produce the signature as the DER encoding of the following
 * ASN.1 definition.
 * <pre>
 * <code>
 *  SEQUENCE { r, s }
 * </code>
 * </pre>
 */
#define VT_DSA_SIGNATURE_DER_ENCODED   2

/** This VtAlgorithmImpl is used to set an algorithm object to
 * verify DSA signatures. Currently, the only digest algorithm allowed
 * with DSA is SHA-1.
 * <p>The data associated with VtAlgorithmImplDSAVerify is a pointer
 * to an unsigned int, a flag indicating the format of the signature,
 * <code>
 * <pre>
 *   VT_DSA_SIGNATURE_R_S
 *   VT_DSA_SIGNATURE_DER_ENCODED
 * </pre>
 * </code>
 * <p>If _R_S, the signature will be the r value concatenated with the
 * s value, both of which are the same size as the subprime (currently
 * only a 160-bit subprime is supported). If _DER_ENCODED, the
 * signature will be the DER encoding of the ASN.1 definition
 * <code>
 * <pre>
 *    Dss-Sig-Value  ::=  SEQUENCE  {
 *       r       INTEGER,
 *       s       INTEGER  }
 * </pre>
 * </code>
 * <p>The ASN.1 definition is the way most (if not all) standards specify
 * the presentation of a DSA signature.
 * <p>Note that this Setter does not perform digesting. The Voltage
 * toolkit sign and verify API take as input the digest of the data to
 * sign, not the data itself.
 * <p>The DSA standards specifies using SHA-1 as the digest algorithm
 * to use with DSA. The toolkit currently supports only SHA-1 as the
 * digest algorithm. The digest input to VtVerifySignature must be 20
 * bytes. The digestAlg must be VT_DIGEST_ALG_SHA1.
 */
extern VtAlgorithmImpl VtAlgorithmImplDSAVerify;

/** This VtAlgorithmImpl is used to set an algorithm object to sign
 * using DSA or verify DSA signatures. Applications that use this
 * AlgorithmImpl will be bigger (load more code) than those that set an
 * object to only sign or only verify. Currently, the only digest
 * algorithm allowed with DSA is SHA-1.
 * <p>The data associated with VtAlgorithmImplDSASignVerify is a pointer to
 * an unsigned int, a flag indicating the format of the signature,
 * <code>
 * <pre>
 *   VT_DSA_SIGNATURE_R_S
 *   VT_DSA_SIGNATURE_DER_ENCODED
 * </pre>
 * </code>
 * <p>If _R_S, the signature will be the r value concatenated with the
 * s value, both of which are the same size as the subprime (currently
 * only a 160-bit subprime is supported). If _DER_ENCODED, the
 * signature will be the DER encoding of the ASN.1 definition
 * <code>
 * <pre>
 *    Dss-Sig-Value  ::=  SEQUENCE  {
 *       r       INTEGER,
 *       s       INTEGER  }
 * </pre>
 * </code>
 * <p>The ASN.1 definition is the way most (if not all) standards specify
 * the presentation of a DSA signature.
 * <p>Note that this Setter does not perform digesting. The Voltage
 * toolkit sign and verify API take as input the digest of the data to
 * sign, not the data itself.
 * <p>The DSA standards specifies using SHA-1 as the digest algorithm
 * to use with DSA. The toolkit currently supports only SHA-1 as the
 * digest algorithm. The digest input to VtSign or VtVerifySignature
 * must be 20 bytes. The digestAlg must be VT_DIGEST_ALG_SHA1.
 */
extern VtAlgorithmImpl VtAlgorithmImplDSASignVerify;

/** This VtAlgorithmImpl is used to set an algorithm object to
 * encrypt or decrypt using RSA.
 * <p>The data associated with VtAlgorithmImplRSAEncrypt is a pointer
 * to a VtRSAInfo struct. This struct contains a padding scheme and the
 * padding scheme's parameters.
 * <p>The padding scheme is allowed to be NoPad, but if so, the input
 * data must be a multiple of the modulus length in bytes. If NULL
 * associated info is passed in, this Impl will assume NoPad.
 * <p>If the padding scheme is not NULL, then the input is only allowed
 * to be one padded block. That is, the actual data to encrypt must be
 * less than one block in length, the padding scheme will pad out to
 * one block. There can be other limitations on the length of input,
 * see the documentation for the particular padding scheme for more
 * info.
 * <p>Furthermore, if the padding scheme is not NULL, you are must call
 * only EncryptInit and EncryptFinal, or DecryptInit and DecryptFinal
 * (Update is not allowed). You must have all the data to process in
 * one buffer. This will generally not be a burdon, because padded RSA
 * allows only one block anyway.
 */
extern VtAlgorithmImpl VtAlgorithmImplRSAEncrypt;

/** This VtAlgorithmImpl is used to set an algorithm object to
 * sign or verify using RSA.
 * <p>The data associated with VtAlgorithmImplRSASignVerify is a
 * pointer to a VtRSAInfo struct. This struct contains a padding scheme
 * and the padding scheme's parameters.
 * <p>Note that an algorithm built with this Impl does not perform
 * digesting. The Voltage toolkit sign and verify API take as input the
 * digest of the data to sign, not the data itself.
 */
extern VtAlgorithmImpl VtAlgorithmImplRSASignVerify;

/** This VtAlgorithmImpl is used to set an algorithm object to
 * sign using RSA following X9.31.
 * <p>This Impl will not sign data digested using MD5.
 * <p>The X9.31 standard can be used only with digest algorithms
 * specified in ISO/IEC 10118. This does not include MD5. The toolkit
 * supports SHA-1, SHA-224, SHA-256, SHA-384, and SHA-512, which are
 * specified in ISO/IEC 10118.
 * <p>The data associated with VtAlgorithmImplRSASignVerifyX931 is a
 * NULL pointer: (Pointer)0.
 * <p>Note that an algorithm built with this Impl does not perform
 * digesting. The Voltage toolkit sign and verify API take as input the
 * digest of the data to sign, not the data itself.
 */
extern VtAlgorithmImpl VtAlgorithmImplRSASignVerifyX931;

/** This VtAlgorithmImpl is used to set an algorithm object to perform
 * Diffie-Hellman key agreement.
 * <p>An algorithm object built with this Impl can be used in the
 * VtGenerateSharedSecret function.
 * <p>The data associated with VtAlgorithmImplDHKeyAgree is a NULL
 * pointer: (Pointer)0.
 */
extern VtAlgorithmImpl VtAlgorithmImplDHKeyAgree;

/** This VtAlgorithmImpl is used to set an algorithm object to
 * digest using SHA-1.
 * <p>The data associated with VtAlgorithmImplSHA1 is NULL pointer:
 * (Pointer)0.
 */
extern VtAlgorithmImpl VtAlgorithmImplSHA1;

/** This VtAlgorithmImpl is used to set an algorithm object to
 * digest using SHA-224.
 * <p>The data associated with VtAlgorithmImplSHA224 is NULL pointer:
 * (Pointer)0.
 */
extern VtAlgorithmImpl VtAlgorithmImplSHA224;

/** This VtAlgorithmImpl is used to set an algorithm object to
 * digest using SHA-256.
 * <p>The data associated with VtAlgorithmImplSHA256 is NULL pointer:
 * (Pointer)0.
 */
extern VtAlgorithmImpl VtAlgorithmImplSHA256;

/** This VtAlgorithmImpl is used to set an algorithm object to
 * digest using SHA-384.
 * <p>The data associated with VtAlgorithmImplSHA384 is NULL pointer:
 * (Pointer)0.
 */
extern VtAlgorithmImpl VtAlgorithmImplSHA384;

/** This VtAlgorithmImpl is used to set an algorithm object to
 * digest using SHA-512.
 * <p>The data associated with VtAlgorithmImplSHA512 is NULL pointer:
 * (Pointer)0.
 */
extern VtAlgorithmImpl VtAlgorithmImplSHA512;

/** This VtAlgorithmImpl is used to set an algorithm object to
 * digest using the techniques defined in SHA-1, but making variations
 * in the initial state.
 * <p>The data associated with VtAlgorithmImplGeneralSHA1 is a pointer
 * to a VtGeneralSHA1Info struct.
 * <p>SHA-1 has an internal state of 5 32-bit words (160 bits). It is
 * set to an initial state before performing any digest operations. For
 * example, the regular SHA-1 initial state is the following.
 * <code>
 * <pre>
 *    H0 = 0x67452301
 *    H1 = 0xEFCDAB89
 *    H2 = 0x98BADCFE
 *    H3 = 0x10325476
 *    H4 = 0xC3D2E1F0
 * </pre>
 * </code>
 * <p>It is possible to use different initial states. This might be
 * useful in building a PRNG, for instance. In fact, FIPS 186-2
 * specifies a sort of PRN generation that relies on SHA-1 with an
 * initial state of the following
 * <code>
 * <pre>
 *    H0 = 0xEFCDAB89
 *    H1 = 0x98BADCFE
 *    H2 = 0x10325476
 *    H3 = 0xC3D2E1F0
 *    H4 = 0x67452301
 * </pre>
 * </code>
 * <p>That is a "shift" of the regular SHA-1 initial state.
 * <p>To use VtAlgorithmImplGeneralSHA1, create a 20 byte buffer and
 * fill it with the initial state. Order the bytes as they appear in
 * the initial state (don't order based on endianness). For example, to
 * pass in the alternative initial state from FIPS 186-2, do the
 * following.
 * <code>
 * <pre>
 *   // For the following intial state
 *   //   0xefcdab89
 *   //   0x98badcfe
 *   //   0x10325476
 *   //   0xC3D2E1F0
 *   //   0x67452301
 *   // build the following table.
 *   unsigned char initState[20] = {
 *     0xEF, 0xCD, 0xAB, 0x89,
 *     0x98, 0xBA, 0xDC, 0xFE,
 *     0x10, 0x32, 0x54, 0x76,
 *     0xC3, 0xD2, 0xE1, 0xF0,
 *     0x67, 0x45, 0x23, 0x01,
 *   };
 *   VtGeneralSHA1Info genSHA1Info;
 *
 *   genSHA1Info.padding = VT_SHA1_FIPS_186_PAD;
 *   genSHA1Info.initState.data = initState;
 *   genSHA1Info.initState.len = 20;
 * </pre>
 * </code>
 */
extern VtAlgorithmImpl VtAlgorithmImplGeneralSHA1;

/** This VtAlgorithmImpl is used to set an algorithm object to
 * digest using MD5.
 * <p>The data associated with VtAlgorithmImplMD5 is NULL pointer:
 * (Pointer)0.
 */
extern VtAlgorithmImpl VtAlgorithmImplMD5;

/** This VtAlgorithmImpl is used to set an algorithm object to
 * perform MAC operation using HMAC.
 * <p>HMAC stands for "Hash-Based Message Authentication Checksum".
 * That means it uses a hash algorithm (message digest) to do much of
 * its work.
 * <p>The data associated with VtAlgorithmImplHMAC is a pointer to a
 * VtHMACInfo struct that specifies the digest algorithm.
 */
extern VtAlgorithmImpl VtAlgorithmImplHMAC;

/** This VtAlgorithmImpl is used to set an algorithm object to
 * encode or decode binary data into Base 64.
 * <p>The data associated with VtAlgorithmImplBase64 is a pointer to a
 * VtBase64Info struct, or NULL if the caller wants to use the default
 * values (48-byte binary to 64-byte base64 blocks, 0x0a "end of line"
 * character and error on invalid Base64 characters).
 */
extern VtAlgorithmImpl VtAlgorithmImplBase64;

/** This VtAlgorithmImpl is used to set an algorithm object to
 * encrypt or decrypt data using DES.
 * <p>DES is a block cipher, so to work, it will need a feedback mode
 * and a padding scheme. The feedback can be ECB, which is equivalent
 * to no feedback, and the padding can be NoPad. But you must
 * explicitly specify the feedback and padding. You do this in the
 * associated info.
 * <p>The data associated with VtAlgorithmImplDES is a pointer to a
 * VtBlockCipherInfo struct.
 * <p>In the BlockCipherInfo, you will have a choice of feedbacks and
 * padding schemes. If you choose CFB or OFB as the feedback, you must
 * select NoPad as the padding scheme.
 */
extern VtAlgorithmImpl VtAlgorithmImplDES;

/** This VtAlgorithmImpl is used to set an algorithm object to
 * encrypt or decrypt data using Triple DES EDE
 * (Encrypt-Decrypt-Encrypt).
 * <p>TripleDES is a block cipher, so to work, it will need a feedback
 * mode and a padding scheme. The feedback can be ECB, which is
 * equivalent to no feedback, and the padding can be NoPad. But you
 * must explicitly specify the feedback and padding. You do this in the
 * associated info.
 * <p>The data associated with VtAlgorithmImpl3DESEDE is a pointer
 * to a VtBlockCipherInfo struct.
 * <p>In the BlockCipherInfo, you will have a choice of feedbacks and
 * padding schemes. If you choose CFB or OFB as the feedback, you must
 * select NoPad as the padding scheme.
 */
extern VtAlgorithmImpl VtAlgorithmImpl3DESEDE;

/** This VtAlgorithmImpl is used to set an algorithm object to
 * encrypt or decrypt data using AES.
 * <p>AES is a block cipher, so to work, it will need a feedback mode
 * and a padding scheme. The feedback can be ECB, which is equivalent
 * to no feedback, and the padding can be NoPad. But you must
 * explicitly specify the feedback and padding. You do this in the
 * associated info.
 * <p>The data associated with VtAlgorithmImplAES is a pointer to a
 * VtBlockCipherInfo struct.
 * <p>In the BlockCipherInfo, you will have a choice of feedbacks and
 * padding schemes. If you choose CFB or OFB as the feedback, you must
 * select NoPad as the padding scheme.
 */
extern VtAlgorithmImpl VtAlgorithmImplAES;

/* The following is a list of VtFeedbackModes and VtPaddingSchemes
 * supported by the toolkit. Each FeedbackMode and PaddingScheme is used
 * in conjunction with special info for the function. If there is no
 * special info, the accompaniment is a NULL pointer.
 */

/** Use VtFeedbackECB if the cipher's feedback should be Electronic Code
 * Book (ECB). ECB is equivalent to "no feedback".
 * <p>The data associated with VtFeedbackECB is NULL pointer:
 * (Pointer)0.
 */
#define VtFeedbackECB (VtFeedbackMode *)0

/** The data associated with VtAlgorithmNoFeedback is
 * NULL pointer: (Pointer)0.
 */
#define VtFeedbackNoFeedback (VtFeedbackMode *)0

/** Use VtFeedbackCBC if the cipher's feedback should be Cipher Block
 * Chaining (CBC).
 * <p>The data associated with VtFeedbackCBC is a pointer to an
 * VtItem containing the initialization vector. The length of the IV must
 * be the same length of the cipher's block size (e.g. DES is an 8-byte
 * block cipher and AES is a 16-byte block cipher).
 */
extern VtFeedbackMode VtFeedbackCBC;

/** Use VtFeedbackCFB if the cipher's feedback should be Cipher Feedback
 * (CFB).
 * <p>If using CFB, the toolkit does not allow you to use a padding
 * scheme. With CFB, padding is not necessary, and the toolkit goes one
 * step further and does not allow padding. If you choose CFB for your
 * block cipher, you must use VtPaddingNoPad. If you try to use a
 * padding scheme, the toolkit will return an error at Create time.
 * <p>The data associated with VtFeedbackCFB is a pointer to a
 * VtCFBInfo struct.
 */
extern VtFeedbackMode VtFeedbackCFB;

/** Use VtFeedbackOFB if the cipher's feedback should be Output Feedback
 * (OFB).
 * <p>If using OFB, the toolkit does not allow you to use a padding
 * scheme. With OFB, padding is not necessary, and the toolkit goes one
 * step further and does not allow padding. If you choose OFB for your
 * block cipher, you must use VtPaddingNoPad. If you tey to use a
 * padding scheme, the toolkit will return an error at Create time.
 * <p>The data associated with VtFeedbackOFB is a pointer to a VtItem
 * containing the initialization vector. The length of the IV must
 * be the same length of the cipher's block size (e.g. DES is an 8-byte
 * block cipher and AES is a 16-byte block cipher).
 */
extern VtFeedbackMode VtFeedbackOFB;

/** Use VtPaddingPkcs5 to pad block ciphers following the method
 * described in PKCS #5.
 * <p>The data associated with VtPaddingPkcs5 is NULL pointer:
 * (Pointer)0.
 */
extern VtPaddingScheme VtPaddingPkcs5;

/** Use VtPaddingPkcs1Type1 to pad or unpad RSA signing following
 * the rules described in PKCS #1 version 1.
 * <p>Note that this is not PSS. It is the original PKCS #1 padding
 * scheme.
 * <p>The data associated with VtPaddingPkcs1Type1 is a NULL pointer:
 * (Pointer)0.
 */
extern VtPaddingScheme VtPaddingPkcs1Type1;

/** Use VtPaddingPkcs1Type2 to pad or unpad RSA encryption following
 * the rules described in PKCS #1 version 1.
 * <p>Note that this is not OAEP. It is the original PKCS #1 padding
 * scheme.
 * <p>If using this padding scheme, the total length of input is
 * allowed to be no more than k - 11 bytes long, where k is the modulus
 * length in bytes. For example, a 1024-bit RSA key has a 1024-bit
 * modulus, which is 128 bytes. The maximum length of total input for
 * such a key would be 117 bytes. This is a requirement of the standard.
 * <p>The data associated with VtPaddingPkcs1Type2 is a NULL pointer:
 * (Pointer)0.
 */
extern VtPaddingScheme VtPaddingPkcs1Type2;

/** Use VtPaddingPkcs1OAEP to pad or unpad RSA encryption following
 * the rules described in PKCS #1 version 2.1.
 * <p>If using this padding scheme, the total length of input is
 * allowed to be no more than k - (2 * digestLen) - 2 bytes long, where
 * k is the modulus length in bytes, and digestLen is the length of the
 * chosen digest algorithm's output. For example, a 1024-bit RSA key
 * has a 1024-bit modulus, which is 128 bytes, and SHA-1 produces a
 * 20-byte digest. Hence, the maximum length of (total) input for such
 * a key would be 86 bytes. This is a requirement of the standard.
 * <p>The data associated with VtPaddingPkcs1OAEP is a pointer to a
 * VtOAEPInfo struct.
 * <p>If the algorithm object is to be used performing SSL/TLS
 * operations, the digest algorithm must be SHA-1 and the label must be
 * empty.
 */
extern VtPaddingScheme VtPaddingPkcs1OAEP;

/** Use VtPaddingNoPad when a cipher's plaintext is not to be padded.
 * This is to be used for a block cipher or an asymmeteric cipher that
 * uses a padding scheme.
 * <p>The data associated with VtPaddingNoPad is NULL pointer:
 * (Pointer)0.
 */
#define VtPaddingNoPad (VtPaddingScheme *)0

/** This is the data struct to accompany VtAlgorithmImplRSAEncrypt and
 * VtAlgorithmImplRSASignVerify.
 */
typedef struct
{
  VtPaddingScheme   *padding;
  Pointer            paddingInfo;
} VtRSAInfo;

/** OAEP uses a digest algorithm, a mask generating function and a
 * label.
 * <p>For the Voltage implementation of OAEP, the digestImpl provided
 * will define the OAEP's digest algorithm.
 * <p>For the Voltage implementation of OAEP, the mask generating
 * function will be MGF1, which requires a digest algorithm. The digest
 * algorithm supplied as the OAEP digest algorithm will be the same
 * digest algorithm used in MGF1.
 * <p>The label is arbitrary data and can be empty (NULL data and 0 len
 * in the VtItem).
 * <p>If the algorithm object is to be used performing SSL/TLS
 * operations, the digestImpl must be VtAlgorithmImplSHA1 (which takes
 * NULL digestInfo) and the label must be empty.
 */
typedef struct
{
  VtAlgorithmImpl   *digestImpl;
  Pointer            digestInfo;
  VtItem             label;
} VtOAEPInfo;

/** This is the data struct to accompany VtAlgorithmImplGeneralSHA1.
 * <p>The padding field is a flag, it must be one of the following two
 * values.
 * <code>
 * <pre>
 *    VT_SHA1_STD_PAD
 *    VT_SHA1_FIPS_186_PAD
 * </pre>
 * </code>
 * <p>VT_SHA1_STD_PAD indicates to pad as the SHA-1 standard specifies.
 * <p>VT_SHA1_FIPS_186_PAD indicates to pad as described in FIPS 186.
 * <p>The initState must be 20 bytes long.
 */
typedef struct
{
  unsigned int padding;
  VtItem initState;
} VtGeneralSHA1Info;

/** For use in VtGeneralSHA1Info, indicate that the object should pad
 * as specified in the SHA-1 standard.
 */
#define VT_SHA1_STD_PAD       180
/** For use in VtGeneralSHA1Info, indicate that the object should pad
 * as specified in the FIPS 186.
 */
#define VT_SHA1_FIPS_186_PAD  186

/** This is the data struct to accompany VtAlgorithmImplHMAC.
 * <p>The digestImpl field specifies which digest algorithm to use for
 * the underlying operations. It will be one of the AlgorithmImpls that
 * perform digests, such as VtAlgorithmImplMD5 or VtAlgorithmImplSHA1.
 * <p>The digestImplInfo field holds the associated info the digest
 * Impl needs.
 * <p>In other words, the Impl and info are what you would use if you
 * were creating an algorithm object to perform a digest alone.
 */
typedef struct
{
  VtAlgorithmImpl  *digestImpl;
  Pointer           digestImplInfo;
} VtHMACInfo;

/** This is the data struct used to accompany the several block cipher
 * VtAlgorithmImpls.
 * <p>The first field is for special info needed by the cipher. Most
 * ciphers do not need any further defining data, but there are some
 * algorithms that do. For example, RC2 has effectiveKeyBits and RC5
 * has round count.
 * <p>A block cipher requires a feedback mode and padding scheme.
 * <p>Set the Feedback field to one of the supported
 * VtFeedbackModes. The feedbackInfo is for the special info
 * associated with the chosen feedback mode. It might be a VtItem
 * containing an IV, it could be some other struct with more
 * information, or it could be NULL.
 * <p>Set the Padding field to one of the supported VtPaddingSchemes.
 * The paddingInfo is for the special info associated with the chosen
 * padding scheme.
 */
typedef struct
{
  Pointer cipherInfo;
  VtFeedbackMode *feedback;
  Pointer feedbackInfo;
  VtPaddingScheme *padding;
  Pointer paddingInfo;
} VtBlockCipherInfo;

/** This is the data to accompany VtFeedbackCFB. The transfer size is
 * the number of bits XORed for each round of encryption.
 * <p>The length of the IV must be the same length of the cipher's block
 * size (e.g. DES is an 8-byte block cipher and AES is a 16-byte block
 * cipher).
 * <p>The transfer size can be 1, 8 or the block size. For example, with
 * DES in CFB mode, the allowed values for transferSizeBits are 1, 8
 * and 64. For AES, they are 1, 8 or 128.
 */
typedef struct
{
  VtItem initVector;
  unsigned int transferSizeBits;
} VtCFBInfo;

/** This is the data to accompany VtAlgorithmImplBase64.
 * <p>The base64BlockSize indicates the size of a Base64 "line". The
 * allowed values are 56, 60, 64, ..., 76. For example, if
 * base64BlockSize is 64, then for each 48 bytes of binary input, the
 * toolkit will produce 64 bytes of output plus one or two bytes,
 * depending on the line feed choice.
 * <p>The newLineCharacter can be one of the following:
 * <code>
 * <pre>
 *    VT_BASE64_NO_NEW_LINE
 *    VT_BASE64_NEW_LINE_LF
 *    VT_BASE64_NEW_LINE_CR_LF
 * </pre>
 * </code>
 * <p>If NO_NEW_LINE, when encoding, the toolkit will not add new line
 * characters, it will simply create a continuous stream of Base 64
 * characters. The "LF" is "line feed", the "CR" is "carriage return".
 * After each full block of base64 output, the encoder will place a
 * "new line" character or characters. "LF" indicates using "line feed"
 * or 0x0a. "CR_LF" means using "carriage return line feed" or
 * 0x0d 0x0a.
 * <p>The errorCheck field indicates whether the toolkit, when decoding,
 * should check base64 input for valid characters. This field should be
 * set to one of the following.
 * <code>
 * <pre>
 *    VT_BASE64_ERROR_CHECK
 *    VT_BASE64_NO_ERROR_CHECK
 * </pre>
 * </code>
 * <p>If "ERROR_CHECK", the toolkit will return an error if an invalid
 * character is encountered. If "NO_ERROR_CHECK", the toolkit will
 * skip invalid characters. 
 * <p>When Base64 encoding, the toolkit will ignore the errorCheck field.
 * <p>When Base64 decoding, the toolkit will ignore the base64BlockSize
 * and newLineCharacter fields and simply read whatever the sender
 * used.
 * <p>Whether encoding or decoding, when setting an object using this
 * data, the toolkit will check the validity of the input.
 */
typedef struct
{
  unsigned int base64BlockSize;
  unsigned int newLineCharacter;
  unsigned int errorCheck;
} VtBase64Info;

/** Use this flag in the newLineCharacter field of Base64Info to
 * indicate that when Base 64 encoding, don't create new lines, just
 * create a continuous stream of data.
 */
#define VT_BASE64_NO_NEW_LINE     0
/** Use this flag in the newLineCharacter field of Base64Info to
 * indicate that when Base 64 encoding, use the "LineFeed" new line
 * character after producing a block of output.
 */
#define VT_BASE64_NEW_LINE_LF     0x0a
/** Use this flag in the newLineCharacter field of Base64Info to
 * indicate that when Base 64 encoding, use the "CarriageReturn,
 * LineFeed" new line characters after producing a block of output.
 */
#define VT_BASE64_NEW_LINE_CR_LF  0x0d0a
/** Use this flag in the errorCheck field of Base64Info to indicate
 * that when Base 64 decoding, return an error if encountering an
 * inappropriate character.
 */
#define VT_BASE64_ERROR_CHECK     1
/** Use this flag in the errorCheck field of Base64Info to indicate
 * that when Base 64 decoding, ignore inappropriate characters.
 */
#define VT_BASE64_NO_ERROR_CHECK  0

/** Destroy an Algorithm Object. This frees up any memory allocated
 * during the object's creation and use.
 * <p>If you call Create, you must call Destroy after you are done with
 * the object but before the program exits.
 * <p>Example:
 * <code>
 * <pre>
 *    int status;
 *    VtAlgorithmObject algObj = (VtAlgorithmObject)0;
 *
 *    do {
 *          . . .
 *      status = VtCreateAlgorithmObject (
 *        libCtx, VtAlgorithmImplSHA1, (Pointer)0, &algObj);
 *      if (status != 0)
 *        break;
 *          . . .
 *    } while (0);
 *
 *    VtDestroyAlgorithmObject (&algObj);
 * </pre>
 * </code>
 * @param algObj A pointer to where the routine will find the object
 * to destroy.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtDestroyAlgorithmObject (
   VtAlgorithmObject *algObj
);

/** Set the algorithm object with the info given.
 * <p>The VtAlgorithmParam defines what information the object will be
 * set with.
 * <p>The include file vibe.h defines the supported AlgorithmParams.
 * Look through the include file to see which AlgorithmParam to use for
 * your application. All supported AlgorithmParams will be defined as
 * in the following example.
 * <code>
 * <pre>
 *   extern VtAlgorithmParam VtAlgorithmNewIv;
 * </pre>
 * </code>
 * <p>Associated with each AlgorithmParam is specific info. The
 * documentation for each AlgorithmParam will describe the associated
 * info it needs. That data could be another object, it could be data
 * in a particular struct, it might be a NULL pointer. Check the
 * documentation for each AlgorithmParam for a description of the data
 * and its required format.
 * <p>To use this function decide which AlgorithmParam you want to use,
 * then determine what information that Setter needs and in which
 * format it is presented. Collect the data in the appropriate format
 * then call this function passing in the desired AlgorithmParam and
 * the required info. The associated info must be cast to Pointer.
 * <p>For example, if the required info is a VtItem, declare a variable
 * to be of type VtItem, set the fields, then pass the address of that
 * VtItem cast to Pointer.
 *
 * @param algObj The object to set.
 * @param algorithmParam What the object is being set to.
 * @param associatedInfo The info needed by the AlgorithmParam.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtSetAlgorithmParam (
   VtAlgorithmObject algObj,
   VtAlgorithmParam algorithmParam,
   Pointer associatedInfo
);

/** Get the specified type of information out of an algorithm object.
 * <p>This function will go to the address given by getInfo and deposit
 * a pointer. That pointer is the address of the data. The memory to
 * hold the information belongs to the object, do not free it.
 * <p>The VtAlgorithmParam will specify what kind of information will be
 * returned, the getInfo is where the function will deposit the info.
 * <p>The include file vibecrypto.h defines the supported
 * AlgorithmParams. Look through the include file to see which
 * AlgorithmParam to use for your application.
 * <p>See also VtSetAlgorithmParam.
 * <p>To use this function decide which AlgorithmParam you want to use,
 * then determine what information that AlgorithmParam will return and
 * in which format it is presented. Declare a variable to be a pointer
 * to the appropriate type, then call this function passing in the
 * desired AlgorithmParam and the address of the variable. Cast the
 * address of the pointer to the appropriate type: (Pointer *).
 * <p>If the associated info is already a pointer type (such as an
 * object), declare a variable to be of the apporpriate type and pass
 * the address of that variable cast to (Pointer *).
 * <p>For example, if the returned info is a VtItem struct, declare a
 * variable to be of type (VtItem *), pass the address of that variable
 * (&varName) cast to (Pointer *).
 *
 * @param algObj The object containing the data to get.
 * @param algorithmParam The type of info to get.
 * @param getInfo The address of a pointer. The function will go to
 * that address and deposit a pointer to the info requested.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtGetAlgorithmParam (
   VtAlgorithmObject algObj,
   VtAlgorithmParam algorithmParam,
   Pointer *getInfo
);

/* These are the AlgorithmParams supported by the toolkit. Each
 * AlgorithmParam is used in conjunction with special info for the
 * function.
 */

/** GetInfo only.
 * <p>Use this VtAlgorithmParam to get a digest object out of signing
 * or verification object. After getting info using this
 * AlgorithmParam, you will have an object that digests data. The
 * object returned belongs to the signing or verification object out of
 * which it came. You can use the digest object returned, but it will
 * be destroyed by the signing or verification object, when the signing
 * or verification object is destroyed.
 * <p>An application might create a signing or verification object
 * based on an algorithm identifier. The app doesn't know which
 * algorithm is being used, it simply passed an algID along with a
 * DerCoder list to the toolkit. In order to digest the data to sign or
 * verify, the app can call on the sign/verify algorithm object to give
 * it a digest object.
 * <p>The associated info is a VtAlgorithmObject built to digest.
 * <p>The info returned using this AlgorithmParam is another algorithm
 * object. That object belongs to the sign/verify object. The caller
 * can use it but must not destroy it.
 * <p>Example:
 * <pre>
 * <code>
 *    VtAlgorithmObject getDigester;
 *
 *      status = VtGetAlgorithmParam (
 *        signer, VtAlgorithmParamSigDigestAlgObj,
 *        (Pointer *)&getDigester);
 * </code>
 * </pre>
 */
extern VtAlgorithmParam VtAlgorithmParamSigDigestAlgObj;

/** SetParam only.
 * <p>Use this VtAlgorithmParam on algorithm objects that are built to
 * perform a message digest. This will reset the initial state.
 * <p>This is generally to be used with SHA-256, and SHA-512. With
 * SHA-256, for example, it is possible to change the initial state and
 * convert the algorithm into SHA-224.
 * <p>Not all digest algorithms will allow the initial state to be set.
 * If you call VtSetAlgorithmParam using this Param on an object for
 * which the initial state cannot be reset, the function will return an
 * error.
 * <p>Build a regular digest object. Then set the object with this
 * Param. Thereafter, for every call to DigestInit, the initial state
 * will be the value given.
 * <p>The associated info is a pointer to a VtItem. Order the bytes as
 * they appear in the initial state (don't order based on endianness).
 * For example, to "convert" SHA-256 to SHA-224, the new initial state
 * is defined as follows.
 * <pre>
 * <code>
 *   H0 = c1059ed8
 *   H1 = 367cd507
 *   H2 = 3070dd17
 *   H3 = f70e5939
 *   H4 = ffc00b31
 *   H5 = 68581511
 *   H6 = 64f98fa7
 *   H7 = befa4fa4
 * </code>
 * </pre>
 * <p>To pass this new state in, do the following.
 * <code>
 * <pre>
 *   VtItem stateItem;
 *   unsigned char initState[32] = {
 *   0xc1, 0x05, 0x9e, 0xd8,
 *   0x36, 0x7c, 0xd5, 0x07,
 *   0x30, 0x70, 0xdd, 0x17,
 *   0xf7, 0x0e, 0x59, 0x39,
 *   0xff, 0xc0, 0x0b, 0x31,
 *   0x68, 0x58, 0x15, 0x11,
 *   0x64, 0xf9, 0x8f, 0xa7,
 *   0xbe, 0xfa, 0x4f, 0xa4
 *   };
 *
 *   stateItem.data = initState;
 *   stateItem.len = 32;
 * </pre>
 * </code>
 */
extern VtAlgorithmParam VtAlgorithmParamDigestInitialState;

/*@}*/

/*=========================================================*/
/*                                                         */
/* Parameter Object                                        */
/*                                                         */
/*=========================================================*/

/**
 * \defgroup ParameterGroup Parameter Object
 */

/*@{*/

/** The parameter object.
 * <p>Note that the object is a pointer type.
 * <p>A parameter object generates and holds "system parameters" or
 * "math parameters".
 * <p>For example, DSA is a digital signature algorithm (in fact, the
 * name "DSA" is the acronym for the wonderfully creative and
 * marvelously unique words "Digital Signature Algorithm"). The
 * algorithm relies on a prime, subprime and base. These are the system
 * parameters. A private key consists of a the system params and a
 * private value "x", the public key consists of the system params and
 * a public value "y" (the names of the variables were chosen by US
 * government bureaucrats, that's why they are so poetic). It generally
 * does not affect security if more than one person uses the same
 * system parameters, just as long as thy use different private and
 * public values (the public value is derived from the private value).
 * <p>Similarly, IBE uses system parameters, prime, subprime, base
 * point and public point (names picked by professors and engineers, so
 * only marginally more descriptive than "x" and "y"). These parameters
 * are shared by many users. The identities are the public values and
 * the private values are derived from the identities.
 * <p>In IBE, there are also district parameters, which consist of the
 * system parameters (or "math parameters") and other information about
 * the district, such as policy and web addresses.
 */
typedef struct VtParameterObjectDef *VtParameterObject;

/** The function VtCreateParameterObject builds a parameter object
 * using a VtParameterImpl. This typedef defines what a VtParameterImpl
 * is. Although a VtParameterImpl is a function pointer, an application
 * should never call one directly, only pass it as an argument to
 * VtCreateParameterObject.
 */
typedef int VT_CALLING_CONV (VtParameterImpl) (
   VtParameterObject *, Pointer, unsigned int);

/** The function VtSetParameterParam adds information to a parameter
 * object. The information to add is defined by a VtParameterParam. This
 * typedef defines what a VtParameterParam is. Although a
 * VtParameterParam is a function pointer, an application should never
 * call one directly, only pass it as an argument to
 * VtSetParameterParam.
 */
typedef int VT_CALLING_CONV (VtParameterParam) (
   VtParameterObject, Pointer, unsigned int);

/** The function VtGenerateParameters generates new parameters using a
 * VtParamGenImpl. This typedef defines what a VtParamGenImpl is. Although
 * a VtParamGenImpl is a function pointer, an application should never
 * call one directly, only pass it as an argument to
 * VtGenerateParameters.
 */
typedef int VT_CALLING_CONV (VtParamGenImpl) (
   VtParameterObject, Pointer, unsigned int, VtRandomObject);

/** Create a new parameter object. This allocates space for an "empty"
 * object, then loads the given ParameterImpl to make it an "active"
 * object.
 * <p>The VtParameterImpl defines the algorithm implementation. The
 * include file vibe.h defines the supported ParameterImpls. Look
 * through the include file to see which ParameterImpl to use for your
 * application. All supported ParameterImpls will be defined as in the
 * following example.
 * <code>
 * <pre>
 *   extern VtParameterImpl VtParameterImplMpCtx;
 * </pre>
 * </code>
 * <p>Associated with each ParameterImpl is specific info. The
 * documentation for each ParameterImpl will describe the associated
 * info it needs. That data could be another object, it could be data
 * in a particular struct, it might be a NULL pointer. Check the
 * documentation for each ParameterImpl for a description of the data
 * and its required format.
 * <p>To use this function decide which ParameterImpl you want to use,
 * then determine what information that ParameterImpl needs and in
 * which format it is presented. Collect the data in the appropriate
 * format then call this function passing in the desired ParameterImpl
 * and the required info. The associated info must be cast to Pointer.
 * <p>For example, if the required info is a VtItem struct, declare a
 * variable to be of type VtItem, fill in the fields, then pass the address
 * of that struct cast to Pointer.
 * <p>The input paramObj is a pointer to an object. It should point to
 * a NULL VtParameterObject. This function will go to the address given
 * and deposit a created object.
 * <p>If you call Create, you must call Destroy after you are done with
 * the object but before the program exits.
 * <p>Example:
 * <pre>
 * <code>
 *    int status;
 *    VtParameterObject paramObj = (VtParameterObject)0;
 *
 *    do {
 *          . . .
 *      status = VtCreateParameterObject (
 *        libCtx, VtParameterImplDefault, (Pointer)0, &paramObj);
 *      if (status != 0)
 *        break;
 *          . . .
 *    } while (0);
 *
 *    VtDestroyParameterObject (&paramObj);
 * </code>
 * </pre>
 * @param libCtx The library context.
 * @param parameterImpl The implementation the object will use.
 * @param associatedInfo The info needed by the ParameterImpl.
 * @param paramObj A pointer to where the routine will deposit the
 * created object.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtCreateParameterObject (
   VtLibCtx libCtx,
   VtParameterImpl parameterImpl,
   Pointer associatedInfo,
   VtParameterObject *paramObj
);

/* These are the VtParameterImpls supported by the toolkit. Each
 * ParameterImpl is used in conjunction with special info for the
 * function. If there is no special info, the accompaniment is a NULL
 * pointer.
 */

/** This VtParameterImpl is used when building a parameter object. It
 * loads the VtMpIntCtx this and other objects should use when
 * performing operations with this parameter set.
 * <p>If the associatedInfo passed in with this Impl is NULL, then the
 * Impl will try to find an mpCtx in the libCtx.
 * <p>The data associated with VtParameterImplMpCtx is a VtMpIntCtx.
 */
extern VtParameterImpl VtParameterImplMpCtx;

/** When building a parameter object, use this VtParameterImpl if the
 * mpCtx desired is loaded into the libCtx.
 * <p>The data associated with VtParameterImplDefault is a NULL pointer:
 * (Pointer)0.
 */
extern VtParameterImpl VtParameterImplDefault;

/** Destroy a Parameter Object. This frees up any memory allocated
 * during the object's creation and use.
 * <p>If you call Create, you must call Destroy after you are done with
 * the object but before the program exits.
 * <p>Example:
 * <code>
 * <pre>
 *    int status;
 *    VtParameterObject paramObj = (VtParameterObject)0;
 *
 *    do {
 *          . . .
 *      status = VtCreateParameterObject (
 *        libCtx, VtParameterImplDefault, (Pointer)0, &paramObj);
 *      if (status != 0)
 *        break;
 *          . . .
 *    } while (0);
 *
 *    VtDestroyParameterObject (&paramObj);
 * </pre>
 * </code>
 * @param paramObj A pointer to where the routine will find the object
 * to destroy.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtDestroyParameterObject (
   VtParameterObject *paramObj
);

/** Set the parameter object with the information given.
 * <p>The VtParameterParam defines what information the object will be
 * set with.
 * <p>The include file vibe.h defines the supported ParameterParams.
 * Look through the include file to see which ParameterParam to use for
 * your application. All supported ParameterParams will be defined as
 * in the following example.
 * <code>
 * <pre>
 *   extern VtParameterParam VtParameterParamBFType1IBECurve;
 * </pre>
 * </code>
 * <p>Associated with each ParameterParam is specific info. The
 * documentation for each ParameterParam will describe the associated
 * info it needs. That data could be another object, it could be data
 * in a particular struct, it might be a NULL pointer. Check the
 * documentation for each ParameterParam for a description of the data
 * and its required format.
 * <p>To use this function decide which ParameterParam you want to use,
 * then determine what information that ParameterParam needs and in
 * which format it is presented. Collect the data in the appropriate
 * format then call this function passing in the desired ParameterParam
 * and the required info. The associated info must be cast to Pointer.
 * <p>For example, if the required info is a particular struct, declare
 * a variable to be of that type struct, fill in the fields, then pass
 * the address of that struct cast to Pointer.
 *
 * @param paramObj The object to set.
 * @param parameterParam What the object is being set to.
 * @param associatedInfo The info needed by the ParameterParam.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtSetParameterParam (
   VtParameterObject paramObj,
   VtParameterParam parameterParam,
   Pointer associatedInfo
);

/** Get the specified type of information out of a parameter object.
 * <p>This function will go to the address given by getInfo and deposit
 * a pointer. That pointer is the address of the data. The memory to
 * hold the information belongs to the object, do not free it.
 * <p>The VtParameterParam will specify what kind of information will be
 * returned, the getInfo is where the function will deposit the info.
 * <p>The include file vibe.h defines the supported ParameterParams.
 * Look through the include file to see which ParameterParam to use for
 * your application.
 * <p>See also VtSetParameterParam.
 * <p>To use this function decide which ParameterParam you want to use,
 * then determine what information that ParameterParam will return and
 * in which format it is presented. Declare a variable to be a pointer
 * to the appropriate type, then call this function passing in the
 * desired ParameterParam and the address of the variable. Cast the
 * address of the pointer to the appropriate type: (Pointer *).
 * <p>If the associated info is already a pointer type (such as an
 * object), declare a variable to be of the apporpriate type and pass
 * the address of that variable cast to (Pointer *).
 * <p>For example, if the returned info is a VtItem struct, declare a
 * variable to be of type (VtItem *), pass the address of that variable
 * (&varName) cast to (Pointer *).
 *
 * @param paramObj The object containing the data to get.
 * @param parameterParam The type of info to get.
 * @param getInfo The address of a pointer. The function will go to
 * that address and deposit a pointer to the info requested.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtGetParameterParam (
   VtParameterObject paramObj,
   VtParameterParam parameterParam,
   Pointer *getInfo
);

/* These are the ParameterParams supported by the toolkit. Each
 * ParameterParam is used in conjunction with special info for the
 * function. If there is no special info, the accompaniment is a NULL
 * pointer.
 */

/** SetObject and GetInfo.
 * <p>This VtParameterParam is used to set a parameter object with a
 * specific IBE type 1 curve (prime, subprime, base point).
 * <p>Or it is used to get the curve info out of an object.
 * <p>The associated info is a VtBFType1IBECurveInfo struct.
 * <p>When setting, build the VtBFType1IBECurveInfo struct and pass a
 * pointer to the struct as the associatedInfo.
 * <p>When getting, pass in the address of a pointer as the getInfo,
 * the Get function will deposit a pointer to a VtBFType1IBECurveInfo
 * struct at the address. All memory belongs to the object out of which
 * the getInfo came.
 */
extern VtParameterParam VtParameterParamBFType1IBECurve;

/** SetObject and GetInfo.
 * <p>This VtParameterParam is used to set a parameter object with a
 * specific set of IBE type 1 parameters (prime, subprime, base point,
 * public point).
 * <p>Or it is used to get the parameter info out of an object.
 * <p>The associated info is a VtBFType1IBEParamInfo struct.
 * <p>When setting, build the VtBFType1IBEParamInfo struct and pass a
 * pointer to the struct as the associatedInfo.
 * <p>When getting, pass in the address of a pointer as the getInfo,
 * the Get function will deposit a pointer to a VtBFType1IBEParamInfo
 * struct at the address. All memory belongs to the object out of which
 * the getInfo came.
 */
extern VtParameterParam VtParameterParamBFType1IBEParams;

/** SetObject and GetInfo.
 * <p>This VtParameterParam is used to set a parameter object with a
 * specific set of BB IBE type 1 parameters (prime, subprime, base
 * point, public point alpha, public point beta, public point gamma).
 * <p>Note that there are currently two versions of IBE implemented in
 * the toolkit, BF for Boneh-Franklin, and BB for Boneh-Boyen.
 * <p>Or it is used to get the parameter info out of an object.
 * <p>The associated info is a VtBBType1IBEParamInfo struct.
 * <p>When setting, build the VtBBType1IBEParamInfo struct and pass a
 * pointer to the struct as the associatedInfo.
 * <p>When getting, pass in the address of a pointer as the getInfo,
 * the Get function will deposit a pointer to a VtBBType1IBEParamInfo
 * struct at the address. All memory belongs to the object out of which
 * the getInfo came.
 */
extern VtParameterParam VtParameterParamBBType1IBEParams;

/** SetObject and GetInfo.
 * <p>This VtParameterParam is used to set a parameter object with a
 * specific set of DSA parameters (prime, subprime, base).
 * <p>Or it is used to get the parameter info out of an object.
 * <p>The associated info is a VtDSAParamInfo struct.
 * <p>When setting, build the VtDSAParamInfo struct and pass a pointer
 * to the struct as the associatedInfo.
 * <p>When getting, pass in the address of a pointer as the getInfo,
 * the Get function will deposit a pointer to a VtDSAParamInfo struct
 * at the address. All memory belongs to the object out of which the
 * getInfo came.
 * <p>Currently there is only one valid prime length, 1024.
 */
extern VtParameterParam VtParameterParamDSAParams;

/** GetInfo only.
 * <p>This VtParameterParam is used to get a parameter set out of a
 * paramter object after generating them. The difference between this
 * Param and VtParameterParamDSAParams is that this one will return
 * more than the prime, subprime and base. It will also return the
 * "SEED", "counter" and "h" values specified by FIPS certification
 * tests.
 * <p>If your app does not need to know what the FIPS "SEED",
 * "counter", and "h" values are, use VtParameterParamDSAParams to
 * extract the parameters from an object.
 * <p>The associated info is a VtDSAParamFipsInfo struct.
 * <p>When getting, pass in the address of a pointer as the getInfo,
 * the Get function will deposit a pointer to a VtDSAParamFipsInfo
 * struct at the address. All memory belongs to the object out of which
 * the getInfo came.
 */
extern VtParameterParam VtParameterParamDSAParamsFips;

/** SetObject and GetInfo.
 * <p>This VtParameterParam is used to set a parameter object with a
 * specific set of DH parameters (prime and base).
 * <p>Or it is used to get the parameter info out of an object.
 * <p>The associated info is a VtDHParamInfo struct.
 * <p>Note that traditional DH parameters consist of a prime and base,
 * but the VtDHParamInfo struct contains a subprime as well. This is
 * for FIPS purposes. If you are using the FIPS certified version of
 * the toolkit, you must include the subprime. If you are using the
 * non FIPS certified version, the subprime is optional.
 * <p>When setting, build the VtDHParamInfo struct and pass a pointer
 * to the struct as the associatedInfo.
 * <p>When getting, pass in the address of a pointer as the getInfo,
 * the Get function will deposit a pointer to a VtDHParamInfo struct
 * at the address. All memory belongs to the object out of which the
 * getInfo came.
 * <p>Currently there is only one valid prime length, 1024.
 */
extern VtParameterParam VtParameterParamDHParams;

/** GetInfo only.
 * <p>This VtParameterParam is used to get a parameter set out of a
 * paramter object after generating them. The difference between this
 * Param and VtParameterParamDHParams is that this one will return
 * more than the prime and base. It will also return the subprime,
 * "SEED", and "counter" values specified by FIPS certification tests.
 * <p>If your app does not need to know what the FIPS subprime, "SEED",
 * and "counter" values are, use VtParameterParamDHParams to extract
 * the parameters from an object.
 * <p>The associated info is a VtDHParamFipsInfo struct.
 * <p>When getting, pass in the address of a pointer as the getInfo,
 * the Get function will deposit a pointer to a VtDHParamFipsInfo
 * struct at the address. All memory belongs to the object out of which
 * the getInfo came.
 */
extern VtParameterParam VtParameterParamDHParamsFips;

/** Generate system parameters. This function will fill the given param
 * object with params. Think of this as an alternate way to Set the
 * parameter object.
 * <p>The paramObj should be created but empty. After the function
 * generates the parameters, they will be in the object and can be
 * retrieved using the appropriate VtParameterParam.
 * <p>To use this function, decide what kind of parameters you want to
 * generate and find the VtParamGenImpl that will generate those
 * params. Associated with each ParamGenImpl is information it needs to
 * accomplish the task. Determine from the documentation what kind of
 * info the particular ParamGenImpl needs and collect that data. Pass
 * the ParamGenImpl and its associated info to this function.
 * <p>The function will use the random object to generate values.
 * Although system parameters are generally not secret, it is still a
 * good idea to seed the random object with strong seed material.
 *
 * @param paramGenImpl Describes what kind of parameters to generate.
 * @param associatedInfo The info needed by the ParamGenImpl
 * @param random An object set to generate random bytes, the source of
 * any random material needed by the paramter generator.
 * @param paramObj The object that will receive the generated
 * parameters.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtGenerateParameters (
   VtParamGenImpl paramGenImpl,
   Pointer associatedInfo,
   VtRandomObject random,
   VtParameterObject paramObj
);

/* These are the VtParamGenImpls supported by the toolkit. Each
 * VtParamGenImpl is used in conjunction with special info for the
 * function. If there is no special info, the accompaniment is a NULL
 * pointer.
 */

/** This VtParamGenImpl is used to generate DSA params (prime, subprime,
 * base).
 * <p>The data associated with VtParamGenImplDSAParams is a pointer to
 * an unsigned int giving the prime size in bits.
 * <p>Currently there is only one valid prime length, 1024.
 */
extern VtParamGenImpl VtParamGenImplDSAParams;

/** This VtParamGenImpl is used to generate DH params (prime and base
 * are the recognized parameters, this Impl will generate a subprime as
 * well).
 * <p>The data associated with VtParamGenImplDHParams is a pointer to
 * an unsigned int giving the prime size in bits.
 * <p>Currently there is only one supported prime length, 1024.
 */
extern VtParamGenImpl VtParamGenImplDHParams;

/** This struct holds a BF Type 1 point. (Note that there are currently
 * two versions of IBE implemented in the toolkit, BF for
 * Boneh-Franklin, and BB for Boneh-Boyen.)
 * <p>If the field isInfinity is 0, the point is not infinity and the
 * coordinate contents are "live". If isInfinity is non zero, then the
 * point is infinity and the contents of the coordinates are ignored.
 * <p>If you ever use this struct, it will likely be for a Get routine,
 * getting the info out of an object. Someone else generated the point,
 * you're just seeing what it is. If you do use this struct to set an
 * object with a point, you will likely be filling it with coordinates
 * you received earlier, rather than something you generated yourself.
 */
typedef struct
{
  unsigned int  isInfinity;
  VtItem        xCoord;
  VtItem        yCoord;
} VtBFType1IBEPoint;

/** This struct holds a BB Type 1 point. (Note that there are currently
 * two versions of IBE implemented in the toolkit, BF for
 * Boneh-Franklin, and BB for Boneh-Boyen.)
 * <p>If the field isInfinity is 0, the point is not infinity and the
 * coordinate contents are "live". If isInfinity is non zero, then the
 * point is infinity and the contents of the coordinates are ignored.
 * <p>If you ever use this struct, it will likely be for a Get routine,
 * getting the info out of an object. Someone else generated the point,
 * you're just seeing what it is. If you do use this struct to set an
 * object with a point, you will likely be filling it with coordinates
 * you received earlier, rather than something you generated yourself.
 */
typedef struct
{
  unsigned int  isInfinity;
  VtItem        xCoord;
  VtItem        yCoord;
} VtBBType1IBEPoint;

/** This is the struct that holds BF IBE Type 1 curve info. (Note that
 * there are currently two versions of IBE implemented in the toolkit,
 * BF for Boneh-Franklin, and BB for Boneh-Boyen.)
 * <p>If you ever use this struct, it will likely be for a Get routine,
 * getting the info out of an object. Someone else generated the curve,
 * you're just seeing what it is. If you do use this struct to set an
 * object with a curve, you will likely be filling it with a prime,
 * subprime, and base point you received earlier, rather than something
 * you generated yourself.
 * <p>The values of the coordinates of the base point are less than the
 * value of the prime.
 */
typedef struct
{
  VtItem             primeP;
  VtItem             subprimeQ;
  VtBFType1IBEPoint  basePointG;
} VtBFType1IBECurveInfo;

/** This is the struct that holds BF IBE Type 1 parameter info. (Note
 * that there are currently two versions of IBE implemented in the
 * toolkit, BF for Boneh-Franklin, and BB for Boneh-Boyen.)
 * <p>If you ever use this struct, it will likely be for a Get routine,
 * getting the info out of an object. Someone else generated the
 * params, you're just seeing what it is. If you do use this struct to
 * set an object with a parameter set, you will likely be filling it
 * with a prime, subprime, base point, and public point you received
 * earlier, rather than something you generated yourself.
 * <p>The values of the coordinates of the base point and public point
 * are less than the value of the prime.
 */
typedef struct
{
  VtBFType1IBECurveInfo  curve;
  VtBFType1IBEPoint      pubPointP;
} VtBFType1IBEParamInfo;

/** This is the struct that holds BB IBE Type 1 parameter info. (Note
 * that there are currently two versions of IBE implemented in the
 * toolkit, BF for Boneh-Franklin, and BB for Boneh-Boyen.)
 * <p>If you ever use this struct, it will likely be for a Get routine,
 * getting the info out of an object. Someone else generated the
 * params, you're just seeing what it is. If you do use this struct to
 * set an object with a parameter set, you will likely be filling it
 * with a prime, subprime, base point, and public point you received
 * earlier, rather than something you generated yourself.
 * <p>The values of the coordinates of the base point and public points
 * are less than the value of the prime.
 */
typedef struct
{
  VtItem             primeP;
  VtItem             subprimeQ;
  VtBBType1IBEPoint  basePointG;
  VtBBType1IBEPoint  pubPointAlpha;
  VtBBType1IBEPoint  pubPointBeta;
  VtBBType1IBEPoint  pubPointGamma;
} VtBBType1IBEParamInfo;

/** This is the struct that holds DSA parameters.
 */
typedef struct
{
  VtItem primeP;
  VtItem subprimeQ;
  VtItem baseG;
} VtDSAParamInfo;

/** This struct is used to return DSA parameters and FIPS test values
 * after generating params. If your app does not need to know what the
 * FIPS "SEED", "counter", and "h" values are, use
 * VtParameterParamDSAParams to extract the parameters from an object.
 */
typedef struct
{
  VtItem primeP;
  VtItem subprimeQ;
  VtItem baseG;
  VtItem SEED;
  VtItem hVal;
  unsigned int counter;
} VtDSAParamFipsInfo;

/** This is the struct that holds DH parameters.
 * <p>Note that traditional DH parameters consist of a prime and base,
 * but the VtDHParamInfo struct contains a subprime as well. This is
 * for FIPS purposes. If you are using the FIPS certified version of
 * the toolkit, you must include the subprime. If you are using the
 * non FIPS certified version, the subprime is optional.
 */
typedef struct
{
  VtItem   primeP;
  VtItem   subprimeQ;
  VtItem   baseG;
} VtDHParamInfo;

/** This struct is used to return DH parameters and FIPS test values
 * after generating params. If your app does not need to know what the
 * FIPS subprimeQ, "SEED" and "counter", values are, use
 * VtParameterParamDHParams to extract the parameters from an object.
 */
typedef struct
{
  VtItem primeP;
  VtItem subprimeQ;
  VtItem baseG;
  VtItem SEED;
  unsigned int counter;
} VtDHParamFipsInfo;

/** If there are params needed for a special purpose, they will consist
 * of an OID (describing the special usage) and the params. If there
 * is more than one set of special usage params, collect them in a list.
 */
typedef struct
{
  VtItem oid;
  VtParameterObject params;
} VtUsageParams;

/** This data struct contains a list of VtUsageParams
 */
typedef struct
{
  unsigned int count;
  VtUsageParams *paramsList;
} VtUsageParamsList;

/*@}*/

/*=========================================================*/
/*                                                         */
/* Key Object                                              */
/*                                                         */
/*=========================================================*/

/**
 * \defgroup KeyGroup Key Object
 */

/*@{*/

/** The key object.
 * <p>Note that the object is a pointer type.
 */
typedef struct VtKeyObjectDef *VtKeyObject;

/** The function VtCreateKeyObject builds a key object using a
 * VtKeyImpl. This typedef defines what a VtKeyImpl is. Although a
 * VtKeyImpl is a function pointer, an application should never call
 * one directly, only pass it as an argument to VtCreateKeyObject.
 */
typedef int VT_CALLING_CONV (VtKeyImpl) (
   VtKeyObject *, Pointer, unsigned int);

/** The function VtSetKeyParam adds information to a key object. The
 * information to add is defined by a VtKeyParam. This typedef defines
 * what a VtKeyParam is. Although a VtKeyParam is a function pointer,
 * an application should never call one directly, only pass it as an
 * argument to VtSetKeyParam.
 */
typedef int VT_CALLING_CONV (VtKeyParam) (
   VtKeyObject, Pointer, unsigned int);

/** The function VtGenerateKeyPair generates a new public/private key
 * pair using a VtKeyPairGenImpl. This typedef defines what a
 * VtKeyPairGenImpl is. Although a VtKeyPairGenImpl is a function
 * pointer, an application should never call one directly, only pass it
 * as an argument to VtGenerateKeyPair.
 */
typedef int VT_CALLING_CONV (VtKeyPairGenImpl) (
   VtKeyObject, Pointer, unsigned int, VtRandomObject);

/** Create a new key object. This allocates space for an "empty"
 * object, then loads the given KeyImpl to make it an "active" object.
 * <p>The VtKeyImpl defines some of the key object operations. The
 * include file vibe.h defines the supported KeyImpls. Look through
 * the include file to see which KeyImpl to use for your application.
 * All supported KeyImpls will be defined as in the following example.
 * <code>
 * <pre>
 *   extern VtKeyImpl VtKeyImplDefault;
 * </pre>
 * </code>
 * <p>Associated with each KeyImpl is specific info. The documentation
 * for each KeyImpl will describe the associated info it needs. That
 * data could be another object, it could be data in a particular
 * struct, it might be a NULL pointer. Check the documentation for each
 * KeyImpl for a description of the data and its required format.
 * <p>To use this function decide which KeyImpl you want to use, then
 * determine what information that KeyImpl needs and in which format it
 * is presented. Collect the data in the appropriate format then call
 * this function passing in the desired KeyImpl and the required info.
 * The associated info must be cast to Pointer.
 * <p>For example, if the required info is a VtItem struct, declare a
 * variable to be of type VtItem, fill in the fields, then pass the address
 * of that struct cast to Pointer.
 * <p>The input keyObj is a pointer to an object. It should point to a
 * NULL VtKeyObject. This function will go to the address given and
 * deposit a created object.
 * <p>If you call Create, you must call Destroy after you are done with
 * the object but before the program exits.
 * <p>Example:
 * <code>
 * <pre>
 *    int status;
 *    VtKeyObject keyObj = (VtKeyObject)0;
 *
 *    do {
 *          . . .
 *      status = VtCreateKeyObject (
 *        libCtx, VtKeyImplDefault, (Pointer)0, &keyObj);
 *      if (status != 0)
 *        break;
 *          . . .
 *    } while (0);
 *
 *    VtDestroyKeyObject (&keyObj);
 * </pre>
 * </code>
 * @param libCtx The library context.
 * @param keyImpl The implementation the object will use.
 * @param associatedInfo The info needed by the KeyImpl.
 * @param keyObj A pointer to where the routine will deposit the
 * created object.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtCreateKeyObject (
   VtLibCtx libCtx,
   VtKeyImpl keyImpl,
   Pointer associatedInfo,
   VtKeyObject *keyObj
);

/* These are the VtKeyImpls supported by the toolkit. Each KeyImpl is
 * used in conjunction with special info for the function. If there is
 * no special info, the accompaniment is a NULL pointer.
 */

/** This VtKeyImpl is used when building an asymmetric key object
 * (public key or private key). It loads the VtMpIntCtx this and other
 * objects should use when performing operations with this key.
 * <p>The data associated with VtKeyImplMpCtx is a VtMpIntCtx or NULL.
 * <p>If the associated data is NULL, the Impl will try to find an
 * mpCtx in the libCtx. That is, if you load the mpCtx into the libCtx,
 * and you build an asymmetric key object, you must still use this
 * Impl, but you can pass a NULL associated info.
 * <p>If you do not pass a NULL info, then the Impl will use the mpCtx
 * passed with it and not look in the libCtx.
 */
extern VtKeyImpl VtKeyImplMpCtx;

/** When building key objects that do not require an mpCtx, use this
 * VtKeyImpl. For example, symmetric keys generally do not need an
 * mpCtx.
 * <p>Note that if you use this Impl, the toolkit will NOT look in the
 * libCtx for an mpCtx. This Impl simply won't load an mpCtx into the
 * object.
 * <p>The data associated with VtKeyImplDefault is a NULL pointer:
 * (Pointer)0.
 */
extern VtKeyImpl VtKeyImplDefault;

/** Destroy a Key Object. This frees up any memory allocated during the
 * object's creation and use.
 * <p>If you call Create, you must call Destroy after you are done with
 * the object but before the program exits.
 * <p>Example:
 * <code>
 * <pre>
 *    int status;
 *    VtKeyObject keyObj = (VtKeyObject)0;
 *
 *    do {
 *          . . .
 *      status = VtCreateKeyObject (
 *        libCtx, VtKeyImplDefault, (Pointer)0, &keyObj);
 *      if (status != 0)
 *        break;
 *          . . .
 *    } while (0);
 *
 *    VtDestroyKeyObject (&keyObj);
 * </pre>
 * </code>
 * @param keyObj A pointer to where the routine will find the object
 * to destroy.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtDestroyKeyObject (
   VtKeyObject *keyObj
);

/** Set the key object with the information given.
 * <p>The VtKeyParam defines what information the object will be set
 * with.
 * <p>The include file vibe.h defines the supported KeyParams. Look
 * through the include file to see which KeyParam to use for your
 * application. All supported KeyParams will be defined as in the
 * following example.
 * <code>
 * <pre>
 *   extern VtKeyParam VtKeyParamAES;
 * </pre>
 * </code>
 * <p>Associated with each KeyParam is specific info. The documentation
 * for each KeyParam will describe the associated info it needs. That
 * data could be another object, it could be data in a particular
 * struct, it might be a NULL pointer. Check the documentation for each
 * KeyParam for a description of the data and its required format.
 * <p>To use this function decide which KeyParam you want to use, then
 * determine what information that KeyParam needs and in which format
 * it is presented. Collect the data in the appropriate format then
 * call this function passing in the desired KeyParam and the required
 * info. The associated info must be cast to Pointer.
 * <p>For example, if the required info is a particular struct, declare
 * a variable to be of that type struct, fill in the fields, then pass
 * the address of that struct cast to Pointer.
 *
 * @param keyObj The object to set.
 * @param keyParam The type of info to set.
 * @param associatedInfo The info needed by the KeyParam.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtSetKeyParam (
   VtKeyObject keyObj,
   VtKeyParam keyParam,
   Pointer associatedInfo
);

/** Get the specified type of information out of a key object.
 * <p>This function will go to the address given by getInfo and deposit
 * a pointer. That pointer is the address of the data. The memory to
 * hold the information belongs to the object, do not free it.
 * <p>The VtKeyParam will specify what kind of information will be
 * returned, the getInfo is where the function will deposit the info.
 * <p>The include file vibe.h defines the supported KeyParams. Look
 * through the include file to see which KeyParam to use for your
 * application.
 * <p>See also VtSetKeyParam.
 * <p>To use this function decide which KeyParam you want to use, then
 * determine what information that KeyParam will return and in which
 * format it is presented. Declare a variable to be a pointer to the
 * appropriate type, then call this function passing in the desired
 * KeyParam and the address of the variable. Cast the address of the
 * pointer to the appropriate type: (Pointer *).
 * <p>If the associated info is already a pointer type (such as an
 * object), declare a variable to be of the apporpriate type and pass
 * the address of that variable cast to (Pointer *).
 * <p>For example, if the returned info is a VtItem struct, declare a
 * variable to be of type (VtItem *), pass the address of that variable
 * (&varName) cast to (Pointer *).
 *
 * @param keyObj The object containing the data to get.
 * @param keyParam The type of info to get.
 * @param getInfo The address of a pointer. The function will go to
 * that address and deposit a pointer to the info requested.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtGetKeyParam (
   VtKeyObject keyObj,
   VtKeyParam keyParam,
   Pointer *getInfo
);

/* These are the KeyParams supported by the toolkit. Each KeyParam is
 * used in conjunction with special info for the function.
 */

/** SetObject and GetInfo.
 * <p>This VtKeyParam is used to set a key object with specific IBE
 * public key info. The info consists of the system, or math,
 * parameters (prime, subprime, base point, public point) along with
 * the encoded identity.
 * <p>Or it is used to get the key info out of an object.
 * <p>The associated info is a VtBFType1IBEPubKeyInfo struct.
 * <p>When setting, build the VtBFType1IBEPubKeyInfo struct and pass a
 * pointer to the struct as the associatedInfo.
 * <p>When getting, pass in the address of a pointer as the getInfo,
 * the Get function will deposit a pointer to a VtBFType1IBEPubKeyInfo
 * struct at the address. All memory belongs to the object out of which
 * the getInfo came.
 */
extern VtKeyParam VtKeyParamBFType1IBEPublic;

/** SetObject and GetInfo.
 * <p>This VtKeyParam is used to set a key object with specific IBE
 * private key info. The info consists of the system, or math,
 * parameters (prime, subprime, base point, public point) along with
 * the encoded identity and private point.
 * <p>Or it is used to get the key info out of an object.
 * <p>The associated info is a VtBFType1IBEPriKeyInfo struct.
 * <p>When setting, build the VtBFType1IBEPriKeyInfo struct and pass a
 * pointer to the struct as the associatedInfo.
 * <p>When getting, pass in the address of a pointer as the getInfo,
 * the Get function will deposit a pointer to a VtBFType1IBEPriKeyInfo
 * struct at the address. All memory belongs to the object out of which
 * the getInfo came.
 */
extern VtKeyParam VtKeyParamBFType1IBEPrivate;

/** SetObject and GetInfo.
 * <p>This VtKeyParam is used to set a key object with specific BB IBE
 * public key info. The info consists of the system, or math,
 * parameters (prime, subprime, base point, public points) along with
 * the encoded identity.
 * <p>Or it is used to get the key info out of an object.
 * <p>Note that there are currently two versions of IBE implemented in
 * the toolkit, BF for Boneh-Franklin, and BB for Boneh-Boyen.
 * <p>The associated info is a VtBBType1IBEPubKeyInfo struct.
 * <p>When setting, build the VtBBType1IBEPubKeyInfo struct and pass a
 * pointer to the struct as the associatedInfo.
 * <p>When getting, pass in the address of a pointer as the getInfo,
 * the Get function will deposit a pointer to a VtBBType1IBEPubKeyInfo
 * struct at the address. All memory belongs to the object out of which
 * the getInfo came.
 */
extern VtKeyParam VtKeyParamBBType1IBEPublic;

/** SetObject and GetInfo.
 * <p>This VtKeyParam is used to set a key object with specific BB IBE
 * private key info. The info consists of the system, or math,
 * parameters (prime, subprime, base point, etc.) along with the
 * encoded identity and private point.
 * <p>Or it is used to get the key info out of an object.
 * <p>Note that there are currently two versions of IBE implemented in
 * the toolkit, BF for Boneh-Franklin, and BB for Boneh-Boyen.
 * <p>The associated info is a VtBBType1IBEPriKeyInfo struct.
 * <p>When setting, build the VtBBType1IBEPriKeyInfo struct and pass a
 * pointer to the struct as the associatedInfo.
 * <p>When getting, pass in the address of a pointer as the getInfo,
 * the Get function will deposit a pointer to a VtBBType1IBEPriKeyInfo
 * struct at the address. All memory belongs to the object out of which
 * the getInfo came.
 */
extern VtKeyParam VtKeyParamBBType1IBEPrivate;

/** SetObject only.
 * <p>Use this to set a key object with specific key data, with the key
 * object matching the given algorithm object.
 * <p>An application might create an algorithm object that encrypts or
 * decrypts using a symmetric cipher based on an algorithm identifier.
 * The app doesn't know which algorithm is being used, it simply passed
 * an algID along with a DerCoder list to the toolkit. In order to
 * build a key object that matches the algorithm object, set the key
 * object using this param.
 * <p>The data associated with VtKeyParamSymmetric is a pointer to a
 * VtSymKeyInfo struct, which contains the algorithm object the key
 * must match, along with the data and length of the key material.
 * <p>Example:
 * <pre>
 * <code>
 *    VtSymKeyInfo symKeyInfo;
 *
 *    algIdInfo.derCoders = derCoders;
 *    algIdInfo.derCoderCount = 3;
 *    algIdInfo.berEncoding = algorithmId;
 *    algIdInfo.maxEncodingLen = algorithmIdLen;
 *    status = VtCreateAlgorithmObject (
 *      libCtx, VtAlgorithmImplAlgId, (Pointer)&algIdInfo, &decryptor);
 *
 *    status = VtCreateKeyObject (
 *      libCtx, VtKeyImplDefault, (Pointer)0, &keyObj);
 *
 *    symKeyInfo.algObject = decryptor;
 *    symKeyInfo.data = keyData;
 *    symKeyInfo.len = keyDataLen;
 *    status = VtSetKeyParam (
 *      keyObj, VtKeyParamSymmetric, (Pointer)&symKeyInfo);
 * </code>
 * </pre>
 */
extern VtKeyParam VtKeyParamSymmetric;

/** SetObject and GetInfo.
 * <p>This VtKeyParam is used to set a key object with specific AES key
 * data.
 * <p>Or it is used to get the key info out of an object.
 * <p>The associated info is a VtItem struct.
 * <p>When setting, build the VtItem struct and pass a pointer to the
 * struct as the associatedInfo.
 * <p>When getting, pass in the address of a pointer as the getInfo,
 * the Get function will deposit a pointer to a VtItem struct at the
 * address. All memory belongs to the object out of which the getInfo
 * came.
 * <p>The length (in bytes) of the key data must be 16, 24 or 32.
 */
extern VtKeyParam VtKeyParamAES;

/** SetObject and GetInfo.
 * <p>This VtKeyParam is used to set a key object with specific DES key
 * data.
 * <p>Or it is used to get the key info out of an object.
 * <p>When getting key data out, the output can be different from the
 * input. DES keys possess parity bits. One bit in each byte is set or
 * not set based on the other seven bits. When the toolkit sets a key
 * object with a DES key, it will store the key data internally with
 * the parity bit appropriately set. So if you set a key object with
 * data for which one or more parity bits are not set correctly, and if
 * you get the key data out, the output will be different from the
 * input.
 * <p>The associated info is a VtItem struct.
 * <p>When setting, build the VtItem struct and pass a pointer to the
 * struct as the associatedInfo.
 * <p>When getting, pass in the address of a pointer as the getInfo,
 * the Get function will deposit a pointer to a VtItem struct at the
 * address. All memory belongs to the object out of which the getInfo
 * came.
 * <p>The length (in bytes) of the key data must be 8.
 */
extern VtKeyParam VtKeyParamDES;

/** SetObject and GetInfo.
 * <p>This VtKeyParam is used to set a key object with specific Triple
 * DES key data.
 * <p>Or it is used to get the key info out of an object.
 * <p>When getting key data out, the output can be different from the
 * input. DES keys (and hence Triple DES keys) possess parity bits. One
 * bit in each byte is set or not set based on the other seven bits.
 * When the toolkit sets a key object with a DES key, it will store the
 * key data internally with the parity bit appropriately set. So if you
 * set a key object with data for which one or more parity bits are not
 * set correctly, and if you get the key data out, the output will be
 * different from the input.
 * <p>The associated info is a VtItem struct.
 * <p>When setting, build the VtItem struct and pass a pointer to the
 * struct as the associatedInfo.
 * <p>When getting, pass in the address of a pointer as the getInfo,
 * the Get function will deposit a pointer to a VtItem struct at the
 * address. All memory belongs to the object out of which the getInfo
 * came.
 * <p>The length (in bytes) of the key data must be 24.
 */
extern VtKeyParam VtKeyParamTripleDES;

/** SetObject and GetInfo.
 * <p>This VtKeyParam is used to set a key object with specific DSA
 * public key data.
 * <p>Or it is used to get the key info out of an object.
 * <p>The associated info is a VtDSAPubKeyInfo struct.
 * <p>When setting, build the VtDSAPubKeyInfo struct and pass a pointer
 * to the struct as the associatedInfo.
 * <p>When getting, pass in the address of a pointer as the getInfo,
 * the Get function will deposit a pointer to a VtDSAPubKeyInfo struct
 * at the address. All memory belongs to the object out of which the
 * getInfo came.
 */
extern VtKeyParam VtKeyParamDSAPublic;

/** SetObject and GetInfo.
 * <p>This VtKeyParam is used to set a key object with specific DSA
 * private key data.
 * <p>Or it is used to get the key info out of an object.
 * <p>The associated info is a VtDSAPriKeyInfo struct.
 * <p>When setting, build the VtDSAPriKeyInfo struct and pass a pointer
 * to the struct as the associatedInfo.
 * <p>When getting, pass in the address of a pointer as the getInfo,
 * the Get function will deposit a pointer to a VtDSAPriKeyInfo struct
 * at the address. All memory belongs to the object out of which the
 * getInfo came.
 */
extern VtKeyParam VtKeyParamDSAPrivate;

/** SetObject and GetInfo.
 * <p>This VtKeyParam is used to set a key object with specific RSA
 * public key data. The object will be available to verify signatures
 * but not encrypt data.
 * <p>Or it is used to get the key info out of an object.
 * <p>FIPS rules require RSA public keys to be capable of verifying or
 * encrypting, but not both. So when setting a public key object, you
 * must specify which type of key the object will be.
 * <p>A VtKeyObject contains an internal flag indicating what it is
 * allowed to do, and this Param will make sure this internal flag is
 * set to Verify.
 * <p>The associated info is a VtRSAPubKeyInfo struct.
 * <p>When setting, build the VtRSAPubKeyInfo struct and pass a pointer
 * to the struct as the associatedInfo.
 * <p>When getting, pass in the address of a pointer as the getInfo,
 * the Get function will deposit a pointer to a VtRSAPubKeyInfo struct
 * at the address. All memory belongs to the object out of which the
 * getInfo came.
 */
extern VtKeyParam VtKeyParamRSAPublicVerify;

/** SetObject and GetInfo.
 * <p>This VtKeyParam is used to set a key object with specific RSA
 * public key data. The object will be available to encrypt data but
 * not verify signatures.
 * <p>Or it is used to get the key info out of an object.
 * <p>FIPS rules require RSA public keys to be capable of verifying or
 * encrypting, but not both. So when setting a public key object, you
 * must specify which type of key the object will be.
 * <p>A VtKeyObject contains an internal flag indicating what it is
 * allowed to do, and this Param will make sure this internal flag is
 * set to Encrypt.
 * <p>The associated info is a VtRSAPubKeyInfo struct.
 * <p>When setting, build the VtRSAPubKeyInfo struct and pass a pointer
 * to the struct as the associatedInfo.
 * <p>When getting, pass in the address of a pointer as the getInfo,
 * the Get function will deposit a pointer to a VtRSAPubKeyInfo struct
 * at the address. All memory belongs to the object out of which the
 * getInfo came.
 */
extern VtKeyParam VtKeyParamRSAPublicEncrypt;

/** SetObject and GetInfo.
 * <p>This VtKeyParam is used to set a key object with specific RSA
 * private key data. The object will be available to create signatures
 * but not decrypt data.
 * <p>Or it is used to get the key info out of an object.
 * <p>FIPS rules require RSA private keys to be capable of signing or
 * decrypting, but not both. So when setting a private key object, you
 * must specify which type of key the object will be.
 * <p>A VtKeyObject contains an internal flag indicating what it is
 * allowed to do, and this Param will make sure this internal flag is
 * set to Sign.
 * <p>The associated info is a VtRSAPriKeyInfo struct.
 * <p>When setting, build the VtRSAPriKeyInfo struct and pass a pointer
 * to the struct as the associatedInfo.
 * <p>When getting, pass in the address of a pointer as the getInfo,
 * the Get function will deposit a pointer to a VtRSAPriKeyInfo struct
 * at the address. All memory belongs to the object out of which the
 * getInfo came.
 * <p>This Param expects the key data to include the CRT (Chinese
 * Remainder Theorem) information. It also expects the prime to be the
 * product of two primes.
 */
extern VtKeyParam VtKeyParamRSAPrivateSign;

/** SetObject and GetInfo.
 * <p>This VtKeyParam is used to set a key object with specific RSA
 * private key data. The object will be available to decrypt data but
 * not create signatures.
 * <p>Or it is used to get the key info out of an object.
 * <p>FIPS rules require RSA private keys to be capable of signing or
 * decrypting, but not both. So when setting a private key object, you
 * must specify which type of key the object will be.
 * <p>A VtKeyObject contains an internal flag indicating what it is
 * allowed to do, and this Param will make sure this internal flag is
 * set to Decrypt.
 * <p>The associated info is a VtRSAPriKeyInfo struct.
 * <p>When setting, build the VtRSAPriKeyInfo struct and pass a pointer
 * to the struct as the associatedInfo.
 * <p>When getting, pass in the address of a pointer as the getInfo,
 * the Get function will deposit a pointer to a VtRSAPriKeyInfo struct
 * at the address. All memory belongs to the object out of which the
 * getInfo came.
 * <p>This Param expects the key data to include the CRT (Chinese
 * Remainder Theorem) information. It also expects the prime to be the
 * product of two primes.
 */
extern VtKeyParam VtKeyParamRSAPrivateDecrypt;

/** SetObject and GetInfo.
 * <p>This VtKeyParam is used to set a key object with specific DH
 * public key data.
 * <p>Or it is used to get the key info out of an object.
 * <p>The associated info is a VtDHPubKeyInfo struct.
 * <p>When setting, build the VtDHPubKeyInfo struct and pass a pointer
 * to the struct as the associatedInfo.
 * <p>When getting, pass in the address of a pointer as the getInfo,
 * the Get function will deposit a pointer to a VtDHPubKeyInfo struct
 * at the address. All memory belongs to the object out of which the
 * getInfo came.
 */
extern VtKeyParam VtKeyParamDHPublic;

/** SetObject and GetInfo.
 * <p>This VtKeyParam is used to set a key object with specific DH
 * private key data.
 * <p>Or it is used to get the key info out of an object.
 * <p>The associated info is a VtDHPriKeyInfo struct.
 * <p>When setting, build the VtDHPriKeyInfo struct and pass a pointer
 * to the struct as the associatedInfo.
 * <p>When getting, pass in the address of a pointer as the getInfo,
 * the Get function will deposit a pointer to a VtDHPriKeyInfo struct
 * at the address. All memory belongs to the object out of which the
 * getInfo came.
 */
extern VtKeyParam VtKeyParamDHPrivate;

/** GetInfo only.
 * <p>This VtKeyParam is used to get the shared secret result of a
 * Diffie-Hellman Key Agreement operation ouot of a key object.
 * <p>The associated info is a VtItem struct.
 * <p>When getting, pass in the address of a pointer as the getInfo,
 * the Get function will deposit a pointer to a VtItem struct at the
 * address. All memory belongs to the object out of which the getInfo
 * came.
 */
extern VtKeyParam VtKeyParamDHSharedSecret;

/** SetObject and GetInfo.
 * <p>This VtKeyParam is used to set a key object with specific secret
 * data used by HMAC functions.
 * <p>Or it is used to get the key info out of an object.
 * <p>The associated info is a VtItem struct.
 * <p>When setting, build the VtItem struct and pass a pointer to the
 * struct as the associatedInfo.
 * <p>When getting, pass in the address of a pointer as the getInfo,
 * the Get function will deposit a pointer to a VtItem struct at the
 * address. All memory belongs to the object out of which the getInfo
 * came.
 * <p>RFC 2104 (the standard specifying HMAC) recommends key sizes of
 * at least the underlying digest length and no more than the
 * underlying digest block size.
 * <pre>
 * <code>
 *         Recommended key sizes, in bytes
 *    HMAC with MD5        16  64
 *    HMAC with SHA-1      20  64
 *    HMAC with SHA-224    28  64
 *    HMAC with SHA-256    32  64
 *    HMAC with SHA-384    48  128
 *    HMAC with SHA-512    64  128
 * </code>
 * </pre>
 * <p>The HMAC algorithm will use an internal key of block size bytes.
 * That is, no matter what size key you supply for HMAC with SHA-1, for
 * example, the algorithm will use a 64-byte key. If the key you supply
 * is less than 64 bytes long, the algorithm will extend it with 00
 * bytes to make it 64. If you supply a key longer than 64 bytes, the
 * algorithm will digest the key and use the resulting 20-byte value as
 * the key (so a key longer than 64-bytes is effectively 20 bytes).
 * <p>The toolkit's implementation of HMAC will accept any key size,
 * but Voltage recommends using only the key sizes listed above.
 */
extern VtKeyParam VtKeyParamHMAC;

/** Generate a key pair. This function will fill the given two key
 * objects with the new keys. Think of this as an alternate way to Set
 * the key objects.
 * <p>The pubKey and priKey should be created but empty. After the
 * function generates the key pair, they will be in the objects and can
 * be retrieved using the appropriate VtKeyParams.
 * <p>To use this function, decide what kind of key pair you want to
 * generate and find the VtKeyPairGenImpl that will generate those
 * keys. Associated with each KeyPairGenImpl is information it needs to
 * accomplish the task. Determine from the documentation what kind of
 * info the particular KeyPairGenImpl needs and collect that data. Pass
 * the KeyPairGenImpl and its associated info to this function.
 * <p>The function will use the random object to generate values.
 * Because the function will generate a private key, the random must be
 * seeded using strong seed material.
 *
 * @param keyPairGenImpl Describes what kind of key pair to generate.
 * @param associatedInfo The info needed by the KeyPairGenImpl
 * @param random An object set to generate random bytes, the source of
 * any random material needed by the key pair generator.
 * @param pubKey The object that will receive the generated public key.
 * @param priKey The object that will receive the generated private key.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtGenerateKeyPair (
   VtKeyPairGenImpl keyPairGenImpl,
   Pointer associatedInfo,
   VtRandomObject random,
   VtKeyObject pubKey,
   VtKeyObject priKey
);

/* These are the VtKeyPairGenImpls supported by the toolkit. Each
 * KeyPairGenImpl is used in conjunction with special info for the
 * function. If there is no special info, the accompaniment is a NULL
 * pointer.
 */

/** This VtKeyPairGenImpl is used to generate a DSA key pair.
 * <p>The data associated with VtKeyPairGenDSA is a VtParameterObject
 * containing the DSA parameters (prime, subprime, base).
 * <p>Currently there is only one valid prime length, 1024.
 */
extern VtKeyPairGenImpl VtKeyPairGenDSA;

/** This VtKeyPairGenImpl is used to generate a DSA key pair. The
 * difference between this Impl and VtKeyPairGenDSA is that this Impl
 * will also generate new parameters (prime, subprime, base). 
 * <p>The data associated with VtKeyPairGenDSA is a pointer to an
 * unsigned int giving the prime size in bits.
 * <p>Currently there is only one valid prime length, 1024.
 */
extern VtKeyPairGenImpl VtKeyPairGenDSAAndParams;

/** This VtKeyPairGenImpl is used to generate an RSA key pair.
 * <p>The data associated with VtKeyPairGenDSA is a pointer to a
 * VtRSAKeyPairGenInfo struct. That struct contains two elements, a
 * modulus length in bits (for RSA, the modulus length is also known as
 * the key length) and a flag indicating what the key pair will be used
 * for (signing or encrypting, FIPS rules requre keys to be capable of
 * signing/verifying or encrypting/decrypting, but not both).
 * <p>The Voltage toolkit currently supports two RSA modulus sizes,
 * 1024 and 2048.
 * <p>This Impl will generate a key pair with a public exponent of
 * 65,537 (this number is also known as "Fermat 4" or F4, in hex it is
 * 0x01 00 01).
 */
extern VtKeyPairGenImpl VtKeyPairGenRSA;

/** This VtKeyPairGenImpl is used to generate a DH key pair.
 * <p>The data associated with VtKeyPairGenDH is a VtParameterObject
 * containing the DH parameters (prime, subprime --if not FIPS the
 * subprime is optional -- and base). If no parameter object is
 * supplied, the toolkit will NOT generate a parameter set, it will
 * simply return an error.
 * <p>Currently there is only one supported prime length, 1024.
 * <p>With this Impl, the KeyPairGen function will generate a random
 * private value, then perform Hiffie-Hellman phase 1 to generate the
 * public value. The private and public values are returned in key
 * objects. To get the data out, call VtGetKeyParam using
 * VtKeyParamDHPublic and VtKeyParamDHPrivate.
 */
extern VtKeyPairGenImpl VtKeyPairGenDH;

/** This is the data struct to accompany VtKeyParamBFType1IBEPublic.
 */
typedef struct
{
  VtParameterObject ibeParams;
  VtItem encodedId;
} VtBFType1IBEPubKeyInfo;

/** This is the data struct to accompany VtKeyParamBBType1IBEPublic.
 * <p>It's the same structure as VtBFType1IBEPubKeyInfo.
 */
typedef struct
{
  VtParameterObject ibeParams;
  VtItem encodedId;
} VtBBType1IBEPubKeyInfo;

/** This is the data struct to accompany VtKeyParamBFType1IBEPrivate.
 */
typedef struct
{
  VtParameterObject ibeParams;
  VtItem encodedId;
  VtBFType1IBEPoint privatePoint;
} VtBFType1IBEPriKeyInfo;

/** This is the data struct to accompany VtKeyParamBBType1IBEPrivate.
 */
typedef struct
{
  VtParameterObject ibeParams;
  VtItem encodedId;
  VtBBType1IBEPoint privatePointD0;
  VtBBType1IBEPoint privatePointD1;
} VtBBType1IBEPriKeyInfo;

/** This is the data type to accompany VtKeyParamDSAPrivate.
 */
typedef struct
{
  VtItem primeP;
  VtItem subprimeQ;
  VtItem baseG;
  VtItem pubValY;
  VtItem priValX;
} VtDSAPriKeyInfo;

/** This is the data type to accompany VtKeyParamDSAPublic.
 */
typedef struct
{
  VtItem primeP;
  VtItem subprimeQ;
  VtItem baseG;
  VtItem pubValY;
} VtDSAPubKeyInfo;

/** This is the data struct to accompany VtKeyPairGenRSA.
 * <p>The modulusBits field is the length of the modulus in bits.
 * Currently only 1024 and 2048 bits are supported.
 * <p>The usageFlag is either
 * <pre>
 * <code>
 *    VT_RSA_KEY_USAGE_SIGN_VERIFY
 *    VT_RSA_KEY_USAGE_ENCRYPT_DECRYPT
 * </code>
 * </pre>
 * <p>FIPS rules require that RSA keys are allowed to encrypt/decrypt
 * or sign/verify, but not both. When you generate an RSA key pair, you
 * must specify what the key pair will be used for.
 * <p>A VtKeyObject contains an internal flag indicating what it is
 * allowed to do, and the usageFlag will be used to set this internal
 * flag.
 */
typedef struct
{
  unsigned int modulusBits;
  unsigned int usageFlag;
} VtRSAKeyPairGenInfo;

/** This is the value to use for the usageFlag field in the
 * VtRSAKeyPairGenInfo struct, when the key pair to generate will be
 * capable of signing and verifying.
 */
#define VT_RSA_KEY_USAGE_SIGN_VERIFY      1
/** This is the value to use for the usageFlag field in the
 * VtRSAKeyPairGenInfo struct, when the key pair to generate will be
 * capable of encrypting and decrypting.
 */
#define VT_RSA_KEY_USAGE_ENCRYPT_DECRYPT  2

/** This is the data type to accompany VtKeyParamRSAPublic.
 */
typedef struct
{
  VtItem modulus;
  VtItem pubExpo;
} VtRSAPubKeyInfo;

/** This is the data type to accompany VtKeyParamRSAPrivate.
 * <p>Some implementations of RSA can still operate if they do not have
 * access to the public and private exponents, or if they have the
 * priExpo but not the CRT info. However, a particular implementation
 * might still require all elements to be present. Therefore, it is
 * advisable to always have all elements available.
 */
typedef struct
{
  VtItem modulus;
  VtItem pubExpo;
  VtItem priExpo;
  VtItem prime1;
  VtItem prime2;
  VtItem exponent1;
  VtItem exponent2;
  VtItem coefficient;
} VtRSAPriKeyInfo;

/** This is the data type to accompany VtKeyParamDHPublic.
 * <p>Note that traditional DH parameters consist of a prime and base,
 * but this struct contains a subprime as well. This is for FIPS
 * purposes. If you are using the FIPS certified version of the
 * toolkit, you must include the subprime. If you are using the
 * non FIPS certified version, the subprime is optional.
 */
typedef struct
{
  VtItem primeP;
  VtItem subprimeQ;
  VtItem baseG;
  VtItem pubValY;
} VtDHPubKeyInfo;

/** This is the data type to accompany VtKeyParamDHPrivate.
 * <p>Note that traditional DH parameters consist of a prime and base,
 * but this struct contains a subprime as well. This is for FIPS
 * purposes. If you are using the FIPS certified version of the
 * toolkit, you must include the subprime. If you are using the
 * non FIPS certified version, the subprime is optional.
 */
typedef struct
{
  VtItem primeP;
  VtItem subprimeQ;
  VtItem baseG;
  VtItem pubValY;
  VtItem priValX;
} VtDHPriKeyInfo;

/** This is the data type to accompany VtKeyParamSymmetric. The
 * algObject will be the object used to encrypt or decrypt. The
 * KeyParam will set the key object to match the algObject. The key
 * object will be set with the key data in the data buffer, of length
 * len bytes.
 */
typedef struct
{
  VtAlgorithmObject   algObject;
  unsigned char      *data;
  unsigned int        len;
} VtSymKeyInfo;

/*@}*/

/*=========================================================*/
/*                                                         */
/* State and Clone Functions                               */
/*                                                         */
/*=========================================================*/

/**
 * \defgroup StateGroup State And Clone Functions
 */

/*@{*/

/** Get the internal state.
 * <p>Not all objects will work with this function. That is, for some
 * objects or objects set with particular information, there will be no
 * returnable internal state.
 * <p>The internal state may be dependent on particular
 * implementations. When setting an object with a state, it will
 * probably be necessary that the destination object be set with the
 * same SetType as the source object.
 * <p>The caller passes in a buffer, the state argument, and its size,
 * this function will fill that buffer with the state and drop off at
 * the address given by stateLen the resulting length.
 * <p>If the buffer is too small (or NULL), the function will return a
 * "BUFFER_TOO_SMALL" error and set stateLen to the required size.
 *
 * @param theObject The object for which the state is requested.
 * @param state The buffer into which this function will place the
 * state.
 * @param bufferSize The size, in bytes, of the buffer.
 * @param stateLen The address where the function will deposit the
 * resulting length of the state, the number of bytes placed into the
 * buffer.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtGetObjectState (
   Pointer theObject,
   unsigned char *state,
   unsigned int bufferSize,
   unsigned int *stateLen
);

/** Set the internal state.
 * <p>The input state should be the value returned by a call to
 * GetState.
 * <p>The internal state may be dependent on particular
 * implementations. When setting an object with a state, it will
 * probably be necessary that the destination object be set with the
 * same SetType as the source object.
 *
 * @param theObject The object for which the state is requested.
 * @param state The buffer containing the internal state. It should be
 * the result of a previous call to GetState.
 * @param stateLen The length of the state.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtSetObjectState (
   Pointer theObject,
   unsigned char *state,
   unsigned int stateLen
 );

/** Get the library context associated with an object.
 *
 * @param theObject The object for which we're getting the associated
 * library context
 * @param libCtx The associated library context will be returned here.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtGetLibCtxFromObject (
   Pointer theObject,
   VtLibCtx *libCtx
);

/** Clone the source object.
 * <p>This function will create a new object of the same type as the
 * source object. It is the responsibility of the caller to call the
 * appropriate Destroy function with the resulting destination object.
 * <p>This function makes a "deep" clone, meaning a new copy of all
 * internal elements and their states are copied.
 * <p>Not all objects are clonable.
 *
 * @param sourceObject The object to clone.
 * @param destObject The address where this function will deposit the
 * newly-created object.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtCloneObject (
   Pointer sourceObject,
   Pointer *destObject
);

/*@}*/

/*=========================================================*/
/*                                                         */
/* Crypto Functions                                        */
/*                                                         */
/*=========================================================*/

/**
 * \defgroup CryptoFuncsGroup Crypto Functions
 */

/*@{*/

/** Initialize the object for digesting. Digest algorithms generally
 * have an initial state, this function puts the object into that state.
 *
 * @param algObj The digest object to init.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtDigestInit (
   VtAlgorithmObject algObj
);

/** Continue digesting, processing the dataToDigestLen bytes at
 * dataToDigest.
 *
 * @param algObj The digest object.
 * @param dataToDigest The input data.
 * @param dataToDigestLen The length, in bytes, of the input data.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtDigestUpdate (
   VtAlgorithmObject algObj,
   unsigned char *dataToDigest,
   unsigned int dataToDigestLen
);

/** Finish the digest process, generating the final digest output. Place
 * the result into the caller-supplied digest buffer, which is
 * bufferSize bytes big. This routine will go to the address given by
 * digestLen and deposit the length of the digest (the number of bytes
 * placed into the digest buffer). If the buffer is not big enough,
 * this function will return the "BUFFER_TOO_SMALL" error and set the
 * unsigned int at digestLen to the needed size.
 * <p>The dataToDigest is the last of the data to digest. It can be
 * NULL or the length can be 0. If the output buffer is not big enough,
 * this function will not operate on the input data.
 * <p>If the entire data to digest is available in a single buffer, it
 * is possible to call DigestFinal without calling DigestUpdate.
 *
 * @param algObj The digest object.
 * @param dataToDigest The last of the data to digest, this can be NULL.
 * @param dataToDigestLen The length, in bytes, of the dataToDigest.
 * @param digest The output buffer.
 * @param bufferSize The size, in bytes, of the output buffer.
 * @param digestLen The address where the routine will deposit the
 * digest length (in bytes).
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtDigestFinal (
   VtAlgorithmObject algObj,
   unsigned char *dataToDigest,
   unsigned int dataToDigestLen,
   unsigned char *digest,
   unsigned int bufferSize,
   unsigned int *digestLen
);

/** Initialize the object for MAC operation. MAC algorithms generally
 * have an initial state. The MAC objects are generally initialized with 
 * some secret information. This function puts the object into that state.
 *
 * @param algObj The MAC object to init.
 * @param keyObj The VtKeyObject containing the secret to be used by
 * the MAC object.
 * If the secret or keyObj is NULL the previously set secret will be used. 
 * If the MAC object is being initialized for the first time and with a
 * NULL key object an error will be returned. 
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtMACInit (
   VtAlgorithmObject algObj,
   VtKeyObject keyObj
);

/** Continue performing the MAC operation, processing the dataToMACLen
 * bytes at dataToMAC.
 *
 * @param algObj The MAC object.
 * @param dataToMAC The input data.
 * @param dataToMACLen The length, in bytes, of the input data.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtMACUpdate (
   VtAlgorithmObject algObj,
   unsigned char *dataToMAC,
   unsigned int dataToMACLen
);

/** Finish the MAC process, generating the final MAC output. Place
 * the result into the caller-supplied buffer, which is
 * bufferSize bytes big. This routine will go to the address given by
 * macLen and deposit the length of the MAC (the number of bytes
 * placed into the MAC buffer). If the buffer is not big enough,
 * this function will return the "BUFFER_TOO_SMALL" error and set the
 * unsigned int at macLen to the needed size.
 * <p>The dataToMAC is the last of the data to MAC. It can be
 * NULL or the length can be 0. If the output buffer is not big enough,
 * this function will not operate on the input data.
 * <p>If the entire data to MAC is available in a single buffer, it
 * is possible to call MACFinal without calling MACUpdate.
 *
 * @param algObj The MAC object.
 * @param dataToMAC The last of the data to MAC, this can be NULL.
 * @param dataToMACLen The length, in bytes, of the dataToMAC.
 * @param mac The output buffer.
 * @param bufferSize The size, in bytes, of the output buffer.
 * @param macLen The address where the routine will deposit the
 * MAC length (in bytes).
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtMACFinal (
   VtAlgorithmObject algObj,
   unsigned char *dataToMAC,
   unsigned int dataToMACLen,
   unsigned char *mac,
   unsigned int bufferSize,
   unsigned int *macLen
);

/** Initialize the object for encrypting. Encryption algorithms generally
 * have an internal key table built from the key data (the encryption
 * key table can be different from the decryption key table). This
 * function will build the key table if there is one or perform other
 * setup operations.
 * <p>The key object must be set with data compatible with the
 * algorithm object. For example, if the algorithm object is set to
 * perform AES in software, the key object must be set with an AES key
 * and the actual data (not just a handle) must be available.
 *
 * @param algObj The encryption object to init.
 * @param keyObj The key object containing the key info the algObj
 * needs.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtEncryptInit (
   VtAlgorithmObject algObj,
   VtKeyObject keyObj
);

/** Continue encrypting, processing the dataToEncryptLen bytes at
 * dataToEncrypt, placing any result into encryptedData, a buffer
 * bufferSize bytes big.
 * <p>This routine will go to the address given by encryptedDataLen
 * and deposit the length of the output (the number of bytes placed
 * into the encryptedData buffer). If the buffer is not big enough,
 * this function will return the "BUFFER_TOO_SMALL" error and set the
 * unsigned int at encryptedDataLen to the needed size.
 * <p>Note that because of padding issues, the output length might
 * not be the same as the input length.
 * <p>Some encryption algorithms or their padding schemes require
 * random bytes. If so, the caller should pass an algorithm object set
 * and seeded to perform random number generation. If not, pass a NULL
 * algorithm object.
 * <p>It is recommended that the input and output buffers not overlap.
 *
 * @param algObj The encryption object.
 * @param random An algorithm object set and seeded to generate random
 * bytes if needed.
 * @param dataToEncrypt The buffer containing the data to encrypt.
 * @param dataToEncryptLen The length, in bytes, of the dataToEncrypt.
 * @param encryptedData The output buffer.
 * @param bufferSize The size, in bytes, of the output buffer.
 * @param encryptedDataLen The address where the routine will deposit the
 * encrypted data length (in bytes).
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtEncryptUpdate (
   VtAlgorithmObject algObj,
   VtRandomObject random,
   unsigned char *dataToEncrypt,
   unsigned int dataToEncryptLen,
   unsigned char *encryptedData,
   unsigned int bufferSize,
   unsigned int *encryptedDataLen
);

/** Finish encrypting, processing any pad bytes, placing any result into
 * encryptedData, a buffer bufferSize bytes big. This routine will go
 * to the address given by encryptedDataLen and deposit the length of
 * the output (the number of bytes placed into the encryptedData
 * buffer). If the buffer is not big enough, this function will return
 * the "BUFFER_TOO_SMALL" error and set the unsigned int at
 * encryptedDataLen to the needed size.
 * <p>The dataToEncrypt is the last of the data to encrypt. It can be
 * NULL or the length can be 0. If the output buffer is not big enough,
 * this function will not operate on the input data.
 * <p>If the entire data to encrypt is available in a single buffer, it
 * is possible to call EncryptFinal without calling EncryptUpdate.
 * <p>Some encryption algorithms or their padding schemes require
 * random bytes. If so, the caller should pass an algorithm object set
 * and seeded to perform random number generation. If not, pass a NULL
 * algorithm object.
 * <p>Note that because of padding issues, there may be output even if
 * there is no more input.
 *
 * @param algObj The encryption object.
 * @param random An algorithm object set and seeded to generate random
 * bytes if needed.
 * @param dataToEncrypt The last of the data to encrypt, this can be NULL.
 * @param dataToEncryptLen The length, in bytes, of the dataToEncrypt.
 * @param encryptedData The output buffer.
 * @param bufferSize The size, in bytes, of the output buffer.
 * @param encryptedDataLen The address where the routine will deposit the
 * encrypted data length (in bytes).
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtEncryptFinal (
   VtAlgorithmObject algObj,
   VtRandomObject random,
   unsigned char *dataToEncrypt,
   unsigned int dataToEncryptLen,
   unsigned char *encryptedData,
   unsigned int bufferSize,
   unsigned int *encryptedDataLen
);

/** Initialize the object for decrypting. Encryption algorithms generally
 * have an internal key table built from the key data (the encryption
 * key table can be different from the decryption key table). This
 * function will build the key table if there is one or perform other
 * setup operations.
 * <p>The key object must be set with data compatible with the
 * algorithm object. For example, if the algorithm object is set to
 * perform AES in software, the key object must be set with an AES key
 * and the actual data (not just a handle) must be available.
 *
 * @param algObj The decryption object to init.
 * @param keyObj The key object containing the key info the algObj
 * needs.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtDecryptInit (
   VtAlgorithmObject algObj,
   VtKeyObject keyObj
);

/** Continue decrypting, processing the dataToDecryptLen bytes at
 * dataToDecrypt, placing any result into decryptedData, a buffer
 * bufferSize bytes big. This routine will go to the address given by
 * decryptedDataLen and deposit the length of the output (the number
 * of bytes placed into the decryptedData buffer). If the buffer is
 * not big enough, this function will return the "BUFFER_TOO_SMALL"
 * error and set the unsigned int at decryptedDataLen to the needed
 * size.
 * <p>Note that because of padding issues, the output length might
 * not be the same as the input length.
 * <p>If the algorithm needs a source of random bytes, the caller
 * should pass a random object, otherwise, pass NULL. Currently there
 * is no algorithm for which decryption needs a source of random bytes,
 * but the argument is there just in case.
 * <p>It is recommended that the input and output buffers not overlap.
 *
 * @param algObj The decryption object.
 * @param random An algorithm object set and seeded to generate random
 * bytes if needed.
 * @param dataToDecrypt The buffer containing the data to decrypt.
 * @param dataToDecryptLen The length, in bytes, of the dataToDecrypt.
 * @param decryptedData The output buffer.
 * @param bufferSize The size, in bytes, of the output buffer.
 * @param decryptedDataLen The address where the routine will deposit the
 * decrypted data length (in bytes).
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtDecryptUpdate (
   VtAlgorithmObject algObj,
   VtRandomObject random,
   unsigned char *dataToDecrypt,
   unsigned int dataToDecryptLen,
   unsigned char *decryptedData,
   unsigned int bufferSize,
   unsigned int *decryptedDataLen
);

/** Finish decrypting, processing any pad bytes, placing any result into
 * decryptedData, a buffer bufferSize bytes big. This routine will go
 * to the address given by decryptedDataLen and deposit the length of
 * the output (the number of bytes placed into the decryptedData
 * buffer). If the buffer is not big enough, this function will return
 * the "BUFFER_TOO_SMALL" error and set the unsigned int at
 * decryptedDataLen to the needed size.
 * <p>The dataToDecrypt is the last of the data to decrypt. It can be
 * NULL or the length can be 0. If the output buffer is not big enough,
 * this function will not operate on the input data.
 * <p>If the entire data to decrypt is available in a single buffer, it
 * is possible to call DecryptFinal without calling DecryptUpdate.
 * <p>If the algorithm needs a source of random bytes, the caller
 * should pass a random object, otherwise, pass NULL. Currently there
 * is no algorithm for which decryption needs a source of random bytes,
 * but the argument is there just in case.
 * <p>Note that because of padding issues, there may be output even  if
 * there is no more input.
 *
 * @param algObj The decryption object.
 * @param random An algorithm object set and seeded to generate random
 * bytes if needed.
 * @param dataToDecrypt The last of the data to decrypt, this can be NULL.
 * @param dataToDecryptLen The length, in bytes, of the dataToDecrypt.
 * @param decryptedData The output buffer.
 * @param bufferSize The size, in bytes, of the output buffer.
 * @param decryptedDataLen The address where the routine will deposit the
 * decrypted data length (in bytes).
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtDecryptFinal (
   VtAlgorithmObject algObj,
   VtRandomObject random,
   unsigned char *dataToDecrypt,
   unsigned int dataToDecryptLen,
   unsigned char *decryptedData,
   unsigned int bufferSize,
   unsigned int *decryptedDataLen
);

/** Encrypt or decrypt 1 bit using CFB (with CFB, to encrypt or
 * decrypt, the actual operation is the same).
 * <p>This function is here primarily for FIPS certification. Most
 * applications will likely never need to call this function.
 * <p>If you build an algorithm object to perform a block cipher in CFB
 * mode, and you set the transferSizeBits to be 1, you can call this
 * routine. If you use an algorithm object built to perform any other
 * form of encryption (e.g. CBC or OFB, or even CFB with a
 * transferSizeBits of 8), the function will return an error.
 * <p>This function will encrypt or decrypt only 1 bit, if you have,
 * for example, 5 bits to encrypt, you must call this function 5 times.
 * <p>The bit to encrypt is the low order bit of the bitToProcess. That
 * is, if the bit to encrypt is 0, set bitToProcess to be the unsigned
 * int 0 (0x00000000), and if the bit is 1, set bitToProcess to be 1
 * (0x00000001).
 * <p>The bitToProcess is an unsigned int instead of an unsiged char,
 * because some compilers do not allow an unsigned char as an argument
 * in a function call.
 * <p>The funtion will set the unsigned int processedBit to be the
 * encrypted or decrypted bit. That is, the function will set the
 * unsigned int to be either 0 or 1.
 * <p>Note that if you call EncryptFinal or DecryptFinal, the operation
 * will work on bytes. You can always call ProcessCFBOneBit for each
 * individual bit of data, then call EncryptFinal or DecryptFinal with
 * NULL input of length 0.
 *
 * @param algObj The encryption or decryption object.
 * @param random An algorithm object set and seeded to generate random
 * bytes if needed.
 * @param bitToProcess The bit to process.
 * @param processedBit The address where the routine will deposit the
 * resulting bit. The result is the low order bit, so the processedBit
 * will be either 0 or 1.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtProcessCFBOneBit (
   VtAlgorithmObject algObj,
   VtRandomObject random,
   unsigned int bitToProcess,
   unsigned int *processedBit
);

/** Create a signature.
 * <p>The data to sign is the digest. That is, this function expects
 * the caller to have first digested the data. If the caller wants to
 * sign actual data, rather than the digest of the data, pass in the
 * actual data as the digest argument.
 * <p>The routine will deposit the result into the caller-supplied
 * signature buffer (of size bufferSize bytes). It will then go to the
 * address given by signatureLen and deposit the length of the result.
 * If the buffer is not big enough, the routine will return the
 * "BUFFER_TOO_SMALL" error and set the signatureLen value to the
 * required space.
 * <p>The caller must indicate which digest algorithm was used to sign
 * the data. This information is passed in the digestAlg value. Use one
 * of the following flags.
 * <pre>
 * <code>
 *    VT_DIGEST_ALG_NONE
 *    VT_DIGEST_ALG_ID
 *    VT_DIGEST_ALG_MD5
 *    VT_DIGEST_ALG_SHA1
 *    VT_DIGEST_ALG_SHA224
 *    VT_DIGEST_ALG_SHA256
 *    VT_DIGEST_ALG_SHA384
 *    VT_DIGEST_ALG_SHA512
 * </code>
 * </pre>
 * <p>Note that signature algorithms do not necessarily support all
 * digest algorithms. For example, DSA supports only SHA-1 for now.
 *
 * @param algObj The object set to create signatures.
 * @param keyObj The object containing the key to use in creating the
 * signature.
 * @param random An object set and seeded to generate random bytes, in
 * case the algorithm object needs them.
 * @param digestAlg A flag indicating which digest algorithm was used
 * to digest the data to sign.
 * @param digest The digest of the data to sign.
 * @param digestLen The length, in bytes, of the digest.
 * @param signature The buffer into which the function will deposit the
 * resulting signature.
 * @param bufferSize The size, in bytes, of the signature buffer.
 * @param signatureLen The address where the function will deposit the
 * length of the resulting signature, the number of bytes placed into
 * the output buffer.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtSign (
   VtAlgorithmObject algObj,
   VtKeyObject keyObj,
   VtRandomObject random,
   unsigned int digestAlg,
   unsigned char *digest,
   unsigned int digestLen,
   unsigned char *signature,
   unsigned int bufferSize,
   unsigned int *signatureLen
);

/** Verify a signature.
 * <p>The data to sign is the digest. That is, this function expects
 * the caller to have first digested the data. If the data signed was
 * the actual data, rather than the digest of the data, pass in the
 * actual data as the digest argument.
 * <p>The caller must indicate which digest algorithm was used to sign
 * the data. This information is passed in the digestAlg value. Use one
 * of the following flags.
 * <pre>
 * <code>
 *    VT_DIGEST_ALG_NONE
 *    VT_DIGEST_ALG_ID
 *    VT_DIGEST_ALG_MD5
 *    VT_DIGEST_ALG_SHA1
 *    VT_DIGEST_ALG_SHA224
 *    VT_DIGEST_ALG_SHA256
 *    VT_DIGEST_ALG_SHA384
 *    VT_DIGEST_ALG_SHA512
 * </code>
 * </pre>
 * <p>The result of the verification (whether the signature verifies or
 * not) is an unsigned int depositied into the address given by
 * verifyResult. If the signature verifies, the value will be set to 1
 * (yes, true), if not, it will be set to 0 (no, false).
 * <p>If the signature does not verify, the return value will still be
 * 0 (success, no error). There was no error in the function, it did
 * what it was supposed to do, determine if a signature verified or not.
 * <p>If the return value is not 0, then the verifyResult value is
 * meaningless.
 * <p>Note that signature algorithms do not necessarily support all
 * digest algorithms. For example, DSA supports only SHA-1 for now.
 *
 * @param algObj The object set to verify signatures.
 * @param keyObj The object containing the key to use in verifying the
 * signature.
 * @param random An object set and seeded to generate random bytes, in
 * case the algorithm object needs them.
 * @param digestAlg A flag indicating which digest algorithm was used
 * to digest the data to verify.
 * @param digest The digest of the data to verify.
 * @param digestLen The length, in bytes, of the digest.
 * @param signature The signature to verify.
 * @param signatureLen The length, in bytes, of the signature to verify.
 * @param verifyResult The address where this function will deposit the
 * result of the verify, 1 for verify, 0 for not.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtVerifySignature (
   VtAlgorithmObject algObj,
   VtKeyObject keyObj,
   VtRandomObject random,
   unsigned int digestAlg,
   unsigned char *digest,
   unsigned int digestLen,
   unsigned char *signature,
   unsigned int signatureLen,
   unsigned int *verifyResult
);

/** For use with VtSign or VtVerifySignature. This flag indicates the
 * data to sign or verify was not digested, but is used directly.
 */
#define VT_DIGEST_ALG_NONE         0
/** For use with VtSign or VtVerifySignature. This flag indicates the
 * data to sign or verify was digested, but the algorithm used was
 * derived from an algorithm ID, not explicitly specified by the app.
 * <p>If you have a situation where the signature algorithm used is
 * presented as an algID, you build an algorithm object using
 * VtAlgorithmImplAlgId (see vibe.h). You can get a digest algorithm
 * object out of the signature object and use it to digest the data to
 * sign or verify. It is possible the app does not know which digest
 * algorithm was used. When you call VtSign or VtVerify, pass this flag
 * to indicate that the digest is based on an algID.
 */
#define VT_DIGEST_ALG_ID           65535
/** For use with VtSign or VtVerifySignature. This flag indicates the
 * data to sign or verify was digested using MD5.
 */
#define VT_DIGEST_ALG_MD5          5
/** For use with VtSign or VtVerifySignature. This flag indicates the
 * data to sign or verify was digested using SHA-1.
 */
#define VT_DIGEST_ALG_SHA1         1
/** For use with VtSign or VtVerifySignature. This flag indicates the
 * data to sign or verify was digested using SHA-224.
 */
#define VT_DIGEST_ALG_SHA224     224
/** For use with VtSign or VtVerifySignature. This flag indicates the
 * data to sign or verify was digested using SHA-256.
 */
#define VT_DIGEST_ALG_SHA256     256
/** For use with VtSign or VtVerifySignature. This flag indicates the
 * data to sign or verify was digested using SHA-384.
 */
#define VT_DIGEST_ALG_SHA384     384
/** For use with VtSign or VtVerifySignature. This flag indicates the
 * data to sign or verify was digested using SHA-512.
 */
#define VT_DIGEST_ALG_SHA512     512

/** Initialize the object for encoding (for example, Base64).
 *
 * @param algObj The object to init.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtEncodeInit (
   VtAlgorithmObject algObj
);

/** Continue encoding, processing the dataToEncodeLen bytes at
 * dataToEncode, placing any result into encodedData, a buffer
 * bufferSize bytes big. This routine will go to the address given by
 * encodedDataLen and deposit the length of the output (the number of
 * bytes placed into the encodedData buffer). If the buffer is not big
 * enough, this function will return the "BUFFER_TOO_SMALL" error and
 * set the unsigned int at encodedDataLen to the needed size.
 * <p>Some encoding algorithm or its padding scheme may require random
 * bytes. If so, the caller should pass an algorithm object set and
 * seeded to perform random number generation. If not, pass a NULL
 * algorithm object.
 * <p>Note that the output length might not be the same as the input
 * length.
 * <p>It is recommended that the input and output buffers not overlap.
 *
 * @param algObj The encoding object.
 * @param random An algorithm object set and seeded to generate random
 * bytes if needed.
 * @param dataToEncode The buffer containing the data to encode.
 * @param dataToEncodeLen The length, in bytes, of the dataToEncode.
 * @param encodedData The output buffer.
 * @param bufferSize The size, in bytes, of the output buffer.
 * @param encodedDataLen The address where the routine will deposit the
 * encoded data length (in bytes).
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtEncodeUpdate (
   VtAlgorithmObject algObj,
   VtRandomObject random,
   unsigned char *dataToEncode,
   unsigned int dataToEncodeLen,
   unsigned char *encodedData,
   unsigned int bufferSize,
   unsigned int *encodedDataLen
);

/** Finish encoding, processing any pad bytes, placing any result into
 * encodedData, a buffer bufferSize bytes big. This routine will go to
 * the address given by encodedDataLen and deposit the length of the
 * output (the number of bytes placed into the encodedData buffer). If
 * the buffer is not big enough, this function will return the
 * "BUFFER_TOO_SMALL" error and set the unsigned int at encodedDataLen
 * to the needed size.
 * <p>The dataToEncode is the last of the data to encode. It can be
 * NULL or the length can be 0. If the output buffer is not big enough,
 * this function will not operate on the input data.
 * <p>If the entire data to encode is available in a single buffer, it
 * is possible to call EncodeFinal without calling EncodeUpdate.
 * <p>Some encoding algorithm or its padding scheme may require random
 * bytes. If so, the caller should pass an algorithm object set and
 * seeded to perform random number generation. If not, pass a NULL
 * algorithm object.
 * <p>Note that because of padding issues, there may be output even if
 * there is no more input.
 *
 * @param algObj The encoding object.
 * @param random An algorithm object set and seeded to generate random
 * bytes if needed.
 * @param dataToEncode The last of the data to encode, this can be NULL.
 * @param dataToEncodeLen The length, in bytes, of the dataToEncode.
 * @param encodedData The output buffer.
 * @param bufferSize The size, in bytes, of the output buffer.
 * @param encodedDataLen The address where the routine will deposit the
 * encoded data length (in bytes).
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtEncodeFinal (
   VtAlgorithmObject algObj,
   VtRandomObject random,
   unsigned char *dataToEncode,
   unsigned int dataToEncodeLen,
   unsigned char *encodedData,
   unsigned int bufferSize,
   unsigned int *encodedDataLen
);

/** Initialize the object for decoding (for example, Base64).
 *
 * @param algObj The object to init.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtDecodeInit (
   VtAlgorithmObject algObj
);

/** Continue decoding, processing the dataToDecodeLen bytes at
 * dataToDecode, placing any result into decodedData, a buffer
 * bufferSize bytes big. This routine will go to the address given by
 * decodedDataLen and deposit the length of the output (the number of
 * bytes placed into the decodedData buffer). If the buffer is not big
 * enough, this function will return the "BUFFER_TOO_SMALL" error and
 * set the unsigned int at decodedDataLen to the needed size.
 * <p>Some decoding algorithm or its padding scheme may require random
 * bytes. If so, the caller should pass an algorithm object set and
 * seeded to perform random number generation. If not, pass a NULL
 * algorithm object.
 * <p>Note that the output length might not be the same as the input
 * length.
 * <p>It is recommended that the input and output buffers not overlap.
 *
 * @param algObj The decoding object.
 * @param random An algorithm object set and seeded to generate random
 * bytes if needed.
 * @param dataToDecode The buffer containing the data to decode.
 * @param dataToDecodeLen The length, in bytes, of the dataToDecode.
 * @param decodedData The output buffer.
 * @param bufferSize The size, in bytes, of the output buffer.
 * @param decodedDataLen The address where the routine will deposit the
 * decoded data length (in bytes).
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtDecodeUpdate (
   VtAlgorithmObject algObj,
   VtRandomObject random,
   unsigned char *dataToDecode,
   unsigned int dataToDecodeLen,
   unsigned char *decodedData,
   unsigned int bufferSize,
   unsigned int *decodedDataLen
);

/** Finish decoding, processing any pad bytes, placing any result into
 * decodedData, a buffer bufferSize bytes big. This routine will go to
 * the address given by decodedDataLen and deposit the length of the
 * output (the number of bytes placed into the decodedData buffer). If
 * the buffer is not big enough, this function will return the
 * "BUFFER_TOO_SMALL" error and set the unsigned int at decodedDataLen
 * to the needed size.
 * <p>The dataToDecode is the last of the data to decode. It can be
 * NULL or the length can be 0. If the output buffer is not big enough,
 * this function will not operate on the input data.
 * <p>If the entire data to decode is available in a single buffer, it
 * is possible to call DecodeFinal without calling DecodeUpdate.
 * <p>Some decoding algorithm or its padding scheme may require random
 * bytes. If so, the caller should pass an algorithm object set and
 * seeded to perform random number generation. If not, pass a NULL
 * algorithm object.
 * <p>Note that because of padding issues, there may be output even if
 * there is no more input.
 *
 * @param algObj The decoding object.
 * @param random An algorithm object set and seeded to generate random
 * bytes if needed.
 * @param dataToDecode The last of the data to decode, this can be NULL.
 * @param dataToDecodeLen The length, in bytes, of the dataToDecode.
 * @param decodedData The output buffer.
 * @param bufferSize The size, in bytes, of the output buffer.
 * @param decodedDataLen The address where the routine will deposit the
 * decoded data length (in bytes).
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtDecodeFinal (
   VtAlgorithmObject algObj,
   VtRandomObject random,
   unsigned char *dataToDecode,
   unsigned int dataToDecodeLen,
   unsigned char *decodedData,
   unsigned int bufferSize,
   unsigned int *decodedDataLen
);

/** Generate a shared secret. This function is generally used to
 * perform Phase 2 of Diffie-Hellman. It combines another party's
 * public key with the caller's private key to produce a shared secret.
 * <p>The shared secret is stored in a key object. The caller creates
 * an empty key object and passes it to this function, which will fill
 * that object with the shared secret data.
 *
 * @param algObj The object built to perform key agreement.
 * @param random An object set to generate random bytes, the source of
 * any random material needed by the function.
 * @param otherPartyPubKey The object that contains the other party's
 * public key.
 * @param myPriKey The object that contain's the caller's private key.
 * @param sharedSecret
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtGenerateSharedSecret (
   VtAlgorithmObject algObj,
   VtRandomObject random,
   VtKeyObject otherPartyPubKey,
   VtKeyObject myPriKey,
   VtKeyObject sharedSecret
);

/*@}*/

/*=========================================================*/
/*                                                         */
/* Surrender callback                                      */
/*                                                         */
/*=========================================================*/

/**
 * \defgroup SurrenderCallback Surrender Callback
 */

/*@{*/

/** This is the info the toolkit will pass to the Surrender function.
 * <p>The callingFlag indicates which function is calling the Surrender
 * function. It will be one of the VT_SURRENDER_FNCT_ flags defined
 * below.
 * <p>The callCount is the count of how many times the toolkit will
 * call the Surrender function for the current operation. The first
 * call will always be right before the toolkit starts the operation.
 * The last call will always be when the operation is done.
 * <p>The callNumber indicates which call this is.
 * <p>For example, a particular operation may call the Surrender
 * function a total of 10 times. The callCount will be 10. The first
 * time that operation calls the Surrender function, the callNumber
 * will be 1, the second time it will be 2, and so on. Note that
 * counting begins at 1, not the typical C programming practice of
 * counting beginning at 0.
 * <p>For some operations, the callCount will be 2, meaning the
 * operation will call once before the operation starts and once after
 * it completes. The function may not be able to make any calls during
 * the operation. For instance, when downloading district parameters,
 * the toolkit calls on some non-toolkit function to make the network
 * connection. During this network procedure, the toolkit cannot call
 * the Surrender function because the toolkit is not working, the
 * network function is working.
 * <p>For some operations, the callCount will be 0, meaning the
 * function does not know the total count. There will be more than 2
 * calls, that's all that's known. For example, the number of
 * operations needed to generate DSA parameters varies greatly from
 * instance to instance. To generate params, the toolkit must find two
 * primes, p and q, such that p - 1 is a multiple of q. That may take
 * some time, or the code might get lucky and find a pair quickly.
 * Hence, there may be a lot of calls to Surrender, or just a few, but
 * there's no way of knowing in advance.
 * <p>If the callCount is 0, the last call will be when the operation
 * is done and the callNumber will be 0 (this is why counting begins at
 * 1 for callNumber).
 */
typedef struct
{
  unsigned int callingFlag;
  unsigned int callCount;
  unsigned int callNumber;
} VtSurrenderInfo;

/* The following are the values the toolkit will use when setting the
 * callingFlag field when calling a Surrender function.
 */

/** The value of callingFlag if the function calling the Surrender
 * function is performing BF Type 1 IBE Encryption.
 * <p>For this call, there is a specified callCount.
 */
#define VT_SURRENDER_FNCT_BF_TYPE1_IBE_ENCRYPT    0x0001
/** The value of callingFlag if the function calling the Surrender
 * function is performing BF Type 1 IBE Decryption.
 * <p>For this call, there is a specified callCount.
 */
#define VT_SURRENDER_FNCT_BF_TYPE1_IBE_DECRYPT    0x0002
/** The value of callingFlag if the function calling the Surrender
 * function is performing BF Type 1 IBE Parameter Generation (server
 * toolkit).
 * <p>For this call, the callCount will be 0, the function will not
 * know how long the operation will take.
 */
#define VT_SURRENDER_FNCT_BF_TYPE1_IBE_PARAM_GEN  0x0011
/** The value of callingFlag if the function calling the Surrender
 * function is performing BB Type 1 IBE Parameter Generation (server
 * toolkit).
 * <p>For this call, the callCount will be 0, the function will not
 * know how long the operation will take.
 */
#define VT_SURRENDER_FNCT_BB_TYPE1_IBE_PARAM_GEN  0x0013
/** The value of callingFlag if the function calling the Surrender
 * function is performing BB Type 1 IBE Encryption.
 * <p>For this call, there is a specified callCount.
 */
#define VT_SURRENDER_FNCT_BB_TYPE1_IBE_ENCRYPT    0x0008
/** The value of callingFlag if the function calling the Surrender
 * function is performing BB Type 1 IBE Decryption.
 * <p>For this call, there is a specified callCount.
 */
#define VT_SURRENDER_FNCT_BB_TYPE1_IBE_DECRYPT    0x0009
/** The value of callingFlag if the function calling the Surrender
 * function is performing BF Type 1 IBE private key derivateion (server
 * toolkit).
 * <p>For this call, the callCount will be 0, the function will not
 * know how long the operation will take.
 */
#define VT_SURRENDER_FNCT_BF_TYPE1_IBE_KEY_DERIVE 0x0018
/** The value of callingFlag if the function calling the Surrender
 * function is performing BB Type 1 IBE private key derivateion (server
 * toolkit).
 * <p>For this call, the callCount will be 0, the function will not
 * know how long the operation will take.
 */
#define VT_SURRENDER_FNCT_BB_TYPE1_IBE_KEY_DERIVE 0x0019
/** The value of callingFlag if the function calling the Surrender
 * function is downloading district parameters.
 * <p>For this call, the callCount will be 2.
 */
#define VT_SURRENDER_FNCT_DIST_PARAM_DOWNLOAD     0x0020
/** The value of callingFlag if the function calling the Surrender
 * function is downloading an IBE private key and a signing cert.
 * <p>For this call, the callCount will be 2.
 */
#define VT_SURRENDER_FNCT_IBE_KEY_DOWNLOAD        0x0021
/** The value of callingFlag if the function calling the Surrender
 * function is generating DSA parameters.
 * <p>For this call, the callCount will be 0, the function will not
 * know how long the operation will take.
 */
#define VT_SURRENDER_FNCT_DSA_PARAM_GEN           0x0100
/** The value of callingFlag if the function calling the Surrender
 * function is generating a DSA key pair.
 * <p>For this call, the callCount will be 0, the function will not
 * know how long the operation will take.
 */
#define VT_SURRENDER_FNCT_DSA_KEY_GEN             0x0101
/** The value of callingFlag if the function calling the Surrender
 * function is creating a DSA signature.
 * <p>For this call, there is a specified callCount.
 */
#define VT_SURRENDER_FNCT_DSA_SIGN                0x0102
/** The value of callingFlag if the function calling the Surrender
 * function is verifying a DSA signature.
 * <p>For this call, there is a specified callCount.
 */
#define VT_SURRENDER_FNCT_DSA_VERIFY              0x0103
/** The value of callingFlag if the function calling the Surrender
 * function is generating an RSA key pair.
 * <p>For this call, the callCount will be 0, the function will not
 * know how long the operation will take.
 */
#define VT_SURRENDER_FNCT_RSA_KEY_GEN             0x0201
/** The value of callingFlag if the function calling the Surrender
 * function is creating an RSA signature.
 * <p>For this call, there is a specified callCount.
 */
#define VT_SURRENDER_FNCT_RSA_SIGN                0x0202
/** The value of callingFlag if the function calling the Surrender
 * function is verifying an RSA signature.
 * <p>For this call, there is a specified callCount.
 */
#define VT_SURRENDER_FNCT_RSA_VERIFY              0x0203
/** The value of callingFlag if the function calling the Surrender
 * function is encrypting using RSA.
 * <p>For this call, there is a specified callCount.
 */
#define VT_SURRENDER_FNCT_RSA_ENCRYPT             0x0209
/** The value of callingFlag if the function calling the Surrender
 * function is decrypting using RSA.
 * <p>For this call, there is a specified callCount.
 */
#define VT_SURRENDER_FNCT_RSA_DECRYPT             0x020a
/** The value of callingFlag if the function calling the Surrender
 * function is generating DH parameters.
 * <p>For this call, the callCount will be 0, the function will not
 * know how long the operation will take.
 */
#define VT_SURRENDER_FNCT_DH_PARAM_GEN            0x0300
/** The value of callingFlag if the function calling the Surrender
 * function is generating a DH key pair.
 * <p>For this call, the callCount will be 0, the function will not
 * know how long the operation will take.
 */
#define VT_SURRENDER_FNCT_DH_KEY_GEN              0x0301

/** To build a SurrenderCallback, you must have a VtSurrenderFunction. The
 * following is the definition of a SurrenderFunction.
 * <p>At various intervals, toolkit functions will call the
 * SurrenderFunction you supply. When the toolkit calls the
 * SurrenderFunction, it will pass to it the appData you specify when
 * building the SurrenderCallback.
 * <p>Once it has been called, your SurrenderFunction will then have in
 * its possession the appData and can dereference the pointer
 * appropriately.
 * <p>The SurrenderFunction performs its operations (open a Window,
 * disply a message or progess bar, whatever) then returns. When it
 * returns, the program will then return to the toolkit. The toolkit
 * will check the return value. If it is zero, it will continue its
 * operation. If it is not 0, it will return the error returned by the
 * SurrenderFunction.
 * <p>The toolkit will also pass to the Surrender function the
 * surrenderInfo. This info specifies what operation is calling the
 * Surrender and info on a count (see documentation for
 * VtSurrenderInfo).
 * <p>The libCtx the toolkit will pass to the Surrender function is the
 * libCtx used by the object that is set with the surrender ctx.
 *
 * @param libCtx The libCtx used by the object for which the surrender
 * ctx is set.
 * @param appData The application-specific data supplied in the
 * SurrenderCallback.
 * @param surrenderInfo Info the toolkit passes to the Surrender
 * function about the calling operation.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
typedef int VT_CALLING_CONV (*VtSurrenderFunction) (
   VtLibCtx libCtx, Pointer appData, VtSurrenderInfo *surrenderInfo);

/** Part of a SurrenderCallback is appData. Because it is specific to the app
 * and the surrender ctx, the toolkit will not know how to copy that
 * data. Hence, part of the SurrenderCallback is a function pointer that can
 * copy the data. You can pass a NULL for the AppDataCopy function and
 * the toolkit will copy a reference to the data.
 * <p>This typedef defines what an AppDataCopy function is.
 * <p>Upon loading the surrender ctx (VtSetParam), the toolkit will
 * either copy the appData pointer (if no AppDataCopy function is
 * supplied) or will call the AppDataCopy function and store the data
 * produced in the object's surrender ctx field.
 *
 * @param libCtx The library ctx to use (possibly for memory
 * allocation, for example).
 * @param appData The application-specific data to copy.
 * @param AppDataCopy The address where the function will deposit the
 * copy of the appData.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
typedef int VT_CALLING_CONV (*VtSurrenderAppDataCopy) (
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
typedef void VT_CALLING_CONV (*VtSurrenderAppDataFree) (
   VtLibCtx libCtx, Pointer *appData);

/** A SurrenderCallback is simply a Surrender function and
 * application-specific data. When building a surrender context, build
 * the application-specific data (maybe it's simply a UI handle, maybe
 * it's a data struct containing multiple elements). If it is already a
 * pointer type, cast it to Pointer and set the appData field to that
 * pointer. If it is not a pointer type (a struct or int, for example),
 * set the appData field to the address of the info cast to Pointer.
 * <p>You can also pass a AppDataCopy function so that when the toolkit
 * loads the surrender ctx into an object, it will copy the appData
 * itself, rather than a reference. However, you can also pass a NULL
 * for the AppDataCopy function, in which case the toolkit will only
 * copy a reference to the appData.
 * <p>If you include an AppDataCopy function, you must also include an
 * AppDataFree function. When the toolkit destroys the object, it will
 * call the AppDataFree function to free up any memory allocated by the
 * AppDataCopy function. If you supply an AppDataCopy function and do
 * not supply an AppDataFree function, the toolkit will not load the
 * surrender ctx and will return an error.
 * <p>When the toolkit calls the Surrender function, it will pass the
 * appData. Inside the Surrender function, the code can then
 * dereference the appData Pointer to the appropriate type.
 * <p>The algorithmFlags field indicates which algorithms and functions
 * should call the Surrender function. It is an array of one or more
 * unsigned ints that are set with the VT_SURRENDER_ALG_ values. The
 * flagCount field is the number of algorithmFlags in the array.
 */
typedef struct
{
  VtSurrenderFunction     Surrender;
  Pointer                 appData;
  VtSurrenderAppDataCopy  AppDataCopy;
  VtSurrenderAppDataFree  AppDataFree;
} VtSurrenderCallback;

/** SetObject only.
 * <p>Set the algorithm object with a surrender ctx.
 * <p>Although all algorithm objects will accept a surrender ctx, not
 * all algorithms will use it. For example, a digest algorithm object
 * will not call the surrender function, even if one is supplied.
 * <p>Some algorithms use "subordinate" objects to get the job done,
 * and will pass on the surrender ctx to those objects. For example,
 * when generating a DSA key pair, the key generator may need to
 * generate parameters, and will create and use a subordinate param gen
 * object. It will also need to test the key pair generated, so will
 * create and use signing and verification objects. The key generator
 * will call the surrender function, but so will the param gen, signing
 * and verifying objects.
 * <p>See the list of VT_SURRENDER_FNCT_ flags to determine which
 * algorithms will use a surrender ctx.
 * <p>The associated info is a VtSurrenderCallback struct.
 * <p>When setting, build the VtSurrenderCallback struct and pass a pointer
 * to the struct as the associatedInfo.
 */
extern VtAlgorithmParam VtAlgorithmParamSurrenderCallback;

/** SetObject only.
 * <p>Set the parameter object with a surrender ctx. It will be used
 * during parameter generation.
 * <p>Although all parameter objects will accept a surrender ctx, a
 * particular algorithm might not use it.
 * <p>See the list of VT_SURRENDER_FNCT_ flags to determine which
 * algorithms will use a surrender ctx.
 * <p>The associated info is a VtSurrenderCallback struct.
 * <p>When setting, build the VtSurrenderCallback struct and pass a pointer
 * to the struct as the associatedInfo.
 */
extern VtParameterParam VtParameterParamSurrenderCallback;

/** SetObject only.
 * <p>Set the key object with a surrender ctx. It will be used when
 * generating keys or key pairs.
 * <p>Although all key objects will accept a surrender ctx, a
 * particular algorithm might not use it. Because there is no key
 * generation function for AES keys, for example, no operation on an
 * AES key will use the surrender ctx.
 * <p>See the list of VT_SURRENDER_FNCT_ flags to determine which
 * algorithms will use a surrender ctx.
 * <p>The associated info is a VtSurrenderCallback struct.
 * <p>When setting, build the VtSurrenderCallback struct and pass a pointer
 * to the struct as the associatedInfo.
 */
extern VtKeyParam VtKeyParamSurrenderCallback;

/** To use a surrender ctx, build a VtSurrenderCallback struct. Fill in the
 * fields with your Surrender function, any appData that function
 * will need (such as a uiHandle), an algorithmFlag indicating which
 * algorithms will surrender, and a callingFlag indicating which
 * functions should surrender.
 * <p>For example,
 * <pre>
 * <code>
 *   VtSurrenderCallback surrCtx;
 *   unsigned int counter;
 *
 *     counter = 0;
 *     surrCtx.Surrender = MySurrenderFnct;
 *     surrCtx.appData = (Pointer)&counter;
 *     surrCtx.AppDataCopy = (VtSurrenderAppDataCopy)0;
 *     surrCtx.AppDataFree = (VtSurrenderAppDataFree)0;
 *     status = VtSetAlgorithmParam (
 *       libCtx, VtAlgorithmParamSurrenderCallback, (Pointer)&surrCtx);
 * </code>
 * </pre>
 */

/*@}*/

/*=========================================================*/
/*                                                         */
/* Error Ctx                                               */
/*                                                         */
/*=========================================================*/

/**
 * \defgroup ErrorCtx Error Context
 */

/*@{*/

/** The error context.
 * <p>Note that the ctx is a pointer type.
 */
typedef struct VtErrorCtxDef *VtErrorCtx;

/** The function VtCreateErrorCtx builds an error context using a
 * VtErrorCtxImpl. This typedef defines what a VtErrorCtxImpl is.
 * Although it is a function pointer, an application should never call
 * a VtErrorCtxImpl directly, only pass it as an argument to
 * VtCreateErrorCtx.
 */
typedef int VT_CALLING_CONV (VtErrorCtxImpl) (
   VtErrorCtx *, Pointer, unsigned int);

/** The function VtSetErrorCtxParam adds information to an error ctx.
 * The information to add is defined by a VtErrorCtxParam. This typedef
 * defines what a VtErrorCtxParam is. Although a VtErrorCtxParam is a
 * function pointer, an application should never call one directly,
 * only pass it as an argument to VtSetErrorCtxParam.
 */
typedef int VT_CALLING_CONV (VtErrorCtxParam) (
   VtErrorCtx, Pointer, unsigned int);

/** An error ctx contains a stack for each error (for some
 * implementations, only one error is allowed in the error ctx at any
 * one time). A stack consists of a series of elements. Each element
 * contains the info on which function is calling the log function.
 * <p>For example, suppose an app calls ApiFunction1. That calls
 * Subroutine1, which in turn calls Subroutine2. There is an error in
 * Subroutine2, so it calls the LogError function, specifying a
 * PRIMARY error. The LogError function creates a new stack (possibly
 * clearing any old stacks), then adds one element to that stack, an
 * element indicating Subroutine2 had a PRIMARY error. LogError
 * returns, then Subroutine2 returns to Subroutine1. Because there was
 * an error in Subroutine2, Subroutine1 is going to stop its operations
 * and pass on the error to its caller. Before it returns, it calls
 * LogError. This time the error is not PRIMARY, so LogError adds
 * another element to the current stack. This new element indicates
 * Subroutine1 had an error. LogError returns, Subroutine1 returns to
 * ApiFunction1. This function calls the LogError function as well, a
 * new element is added. One error, one stack, three elements in the
 * stack.
 * <p>Once an app has control, it can get the stack out of the error
 * ctx. It can see a trace of the functions, from the API call down to
 * the actual routine that had the error.
 * <p>This is the definition of an error stack element.
 * <p>The errorCode is the actual value of the tollkit error (one of the
 * VT_ERROR_ values defined in vibecrypto.h and vibe.h. 
 * <p>errorCodeSystem is the system's error code (in case of system errors).
 * Its value will differ from platform to platform. This value should 
 * only be used if a system error has ocurred and must not be used to find out
 * if a system error has ocurred or not (based on if it is 0 or non-zero). 
 * To find out if the element represents a system error, use the errorType field.
 * When more information about system error is required, use the description 
 * field along with the errorCodeSystem field. On Unix platform this value is
 * the same as the value of the "errno" variable and windows it is the value
 * returned by "GetLastError" function.
 * <p>The errorType field is the OR of the VT_ERROR_TYPE_ bits.
 * <p>The functionLine field contains the line in the source code where
 * the error occurred. If 0, that info was unavailable.
 * <p>The functionName and description fields are NULL-terminated ASCII
 * strings, they might be NULL.
 */
typedef struct
{
  int                  errorCode;
  int                  errorCodeSystem;
  unsigned int         errorType;
  unsigned int         functionLine;
  char                *errorAsString;
  char                *functionName;
  char                *description;
} VtErrorStackElement;

/** This is the definition of an error stack. It is an array of stack
 * elements.
 * <p>The memoryError field is either 0 or VT_ERROR_MEMORY. If the
 * memoryError is 0, there was no problem with memory, the stack is
 * complete. If it is not 0, then there was a memory error. That error
 * could be the original error or possibly the memory error happened
 * while building the error stack. If so, the error stack may be
 * incomplete.
 * <p>The count field indicates how many elements there are in the
 * stack.
 * <p>The elementList is an array of VtErrorStackElements. Note that
 * the list is a list of Elements, not a list of pointers. So to access
 * the fields of each stack, use the ".", not the "->".
 * <pre>
 * <code>
 *    VtErrorStack *errorStack;
 *
 *    errorStack->elementList[2].errorCode
 * </code>
 * </pre>
 * <p>The stack will begin with the PRIMARY error. That is,
 * elementList[0] will be the Element describing where the error
 * begins. The last element (elementList[errorStack->count - 1]) will
 * be for the API function the app called.
 */
typedef struct
{
  int                   memoryError;
  unsigned int          count;
  VtErrorStackElement  *elementList;
} VtErrorStack;

/* These are the bits that can be set for the errorType field in the
 * VtErrorStackElement struct. If the "PRIMARY" bit is set, the error
 * occurred in the given function. If the PRIMARY bit is not set, the
 * function that is represented by the element had called another
 * function in which an error occurred and is just passing on the error.
 * <p>"SYSTEM" means the error was a system error (such as internet
 * connection failure). If the SYSTEM bit is not set, the error is a
 * toolkit error (such as VT_ERROR_INVALID_INPUT_LENGTH).
 */

/** For the errorType field in the VtErrorStackElement struct, if this
 * bit is set, then the error occurred in the routine reperesented by
 * the element.
 */
#define VT_ERROR_TYPE_PRIMARY     0x0001
/** For the errorType field in the VtErrorStackElement struct, if this
 * bit is set, then the error represented by the element is an error in
 * an outside function (e.g. a third party library function such as
 * OpenSSL or GMP) as opposed to a toolkit error.
 */
#define VT_ERROR_TYPE_OUTSIDE     0x0010
/** For the errorType field in the VtErrorStackElement struct, if this
 * bit is set, then the error represented by the element is a system
 * error (such as an internet connection failure) as opposed to a
 * toolkit error.
 */
#define VT_ERROR_TYPE_SYSTEM      0x0100

/** Create a new error context. This allocates space for an "empty"
 * context, then loads the given ErrorCtxImpl to make it an "active"
 * context.
 * <p>The VtErrorCtxImpl defines the policy implementation. The include
 * file vibe.h defines the supported ErrorCtxImpls. Look through the
 * include file to see which ErrorCtxImpl to use for your application.
 * All supported ErrorCtxImpls will be defined as in the following
 * example.
 * <pre>
 * <code>
 *   extern VtErrorCtxImpl VtErrorCtxImplBasic;
 * </code>
 * </pre>
 * <p>Associated with each ErrorCtxImpl is specific info. The
 * documentation for each ErrorCtxImpl will describe the associated info
 * it needs. That data could be another object, it could be data in a
 * particular struct, it might be a NULL pointer. Check the
 * documentation for each ErrorCtxImpl for a description of the data and
 * its required format.
 * <p>To use this function decide which ErrorCtxImpl you want to use,
 * then determine what information that ErrorCtxImpl needs and in which
 * format it is presented. Collect the data in the appropriate format
 * then call this function passing in the desired ErrorCtxImpl and the
 * required info. The associated info must be cast to Pointer.
 * <p>For example, if the required info is a VtItem, declare a variable
 * to be of type VtItem, fill in the fields, then pass the address of
 * that VtItem cast to Pointer.
 * <p>The input errorCtx is a pointer to a context. It should point to
 * a NULL VtErrorCtx. This function will go to the address given and
 * deposit a created context.
 * <p>If you call Create, you must call Destroy after you are done with
 * the context but before the program exits.
 * <p>Example:
 * <pre>
 * <code>
 *    int status;
 *    VtErrorCtx errorCtx = (VtErrorCtx)0;
 *
 *    do {
 *          . . .
 *
 *      status = VtCreateErrorCtx (
 *        libCtx, VtErrorCtxImplBasic, (Pointer)0, &errorCtx);
 *      if (status != 0)
 *        break;
 *          . . .
 *    } while (0);
 *
 *    VtDestroyErrorCtx (&errorCtx);
 * </code>
 * </pre>
 *
 * @param libCtx The library context.
 * @param errorImpl The implementation the context will use.
 * @param associatedInfo The info needed by the ErrorCtxImpl.
 * @param errorCtx A pointer to where the routine will deposit the
 * created context.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtCreateErrorCtx (
   VtLibCtx libCtx,
   VtErrorCtxImpl errorImpl,
   Pointer associatedInfo,
   VtErrorCtx *errorCtx
);

/* These are the VtErrorCtxImpls supported by the toolkit. Each
 * ErrorCtxImpl is used in conjunction with special info for the
 * function. If there is no special info, the accompaniment is a NULL
 * pointer.
 */

/** This VtErrorCtxImpl is used to build an error context with a basic
 * error ctx supplied by Voltage. It will allow only one error stack at
 * any one time. That is, each time a new PRIMARY error is logged, the
 * Basic ctx will clear any old errors and start the new stack.
 * <p>The data associated with VtErrorCtxImplBasic is a null pointer:
 * (Pointer)0.
 */
extern VtErrorCtxImpl VtErrorCtxImplBasic;

/** Destroy the error context.  This frees up any memory allocated
 * during the context's creation and use.
 * <p>If you call Create, you must call Destroy after you are done with
 * the context but before the program exits.
 * <pre>
 * <code>
 *    int status;
 *    VtErrorCtx errorCtx = (VtErrorCtx)0;
 *
 *    do {
 *          . . .
 *
 *      status = VtCreateErrorCtx (
 *        libCtx, VtErrorCtxImplBasic, (Pointer)0, &errorCtx);
 *      if (status != 0)
 *        break;
 *          . . .
 *    } while (0);
 *
 *    VtDestroyErrorCtx (&errorCtx);
 * </code>
 * </pre>
 * @param errorCtx A pointer to where the routine will find the context to
 * destroy.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtDestroyErrorCtx (
   VtErrorCtx *errorCtx
);

/** Set the error context with the information given.
 * <p>The VtErrorCtxParam defines what information the ctx will be set
 * with.
 * <p>The include file vibecrypto.h defines the supported
 * ErrorCtxParams. Look through the include file to see which
 * ErrorCtxParam to use for your application. All supported
 * ErrorCtxParams will be defined as in the following example.
 * <code>
 * <pre>
 *   extern VtErrorCtxParam VtErrorCtxParamExcludedErrors;
 * </pre>
 * </code>
 * <p>Associated with each ErrorCtxParam is specific info. The
 * documentation for each ErrorCtxParam will describe the associated info
 * it needs. That data could be another object, it could be data in a
 * particular struct, it might be a NULL pointer. Check the
 * documentation for each ErrorCtxParam for a description of the data and
 * its required format.
 * <p>To use this function decide which ErrorCtxParam you want to use,
 * then determine what information that ErrorCtxParam needs and in which
 * format it is presented. Collect the data in the appropriate format
 * then call this function passing in the desired ErrorCtxParam and the
 * required info. The associated info must be cast to Pointer.
 * <p>For example, if the required info is a char array (a
 * NULL-terminated string), declare a variable to be of type char *,
 * set it to point to the NULL-terminated string, then pass that char *
 * cast to Pointer.
 *
 * @param errorCtx The context to set.
 * @param errorCtxParam What the ctx is being set to.
 * @param associatedInfo The info needed by the ErrorCtxParam.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtSetErrorCtxParam (
   VtErrorCtx errorCtx,
   VtErrorCtxParam errorCtxParam,
   Pointer associatedInfo
);

/** Get the specified type of information out of an error ctx.
 * <p>This function will go to the address given by getInfo and deposit
 * a pointer. That pointer is the address of the data. The memory to
 * hold the information belongs to the ctx, do not free it.
 * <p>The VtErrorCtxParam will specify what kind of information will be
 * returned, the getInfo is where the function will deposit the info.
 * <p>The include file vibecrypto.h defines the supported
 * ErrorCtxParams. Look through the include file to see which
 * ErrorCtxParam to use for your application.
 * <p>See also VtSetErrorCtxParam.
 * <p>To use this function decide which ErrorCtxParam you want to use,
 * then determine what information that ErrorCtxParam will return and
 * in which format it is presented. Declare a variable to be a pointer
 * to the appropriate type, then call this function passing in the
 * desired ErrorCtxParam and the address of the variable. Cast the
 * address of the pointer to the appropriate type: (Pointer *).
 * <p>If the associated info is already a pointer type (such as an
 * object), declare a variable to be of the apporpriate type and pass
 * the address of that variable cast to (Pointer *).
 * <p>For example, if the returned info is an unsigned int, declare a
 * variable to be of type (unsigned int *), pass the address of that
 * variable (&varName) cast to (Pointer *).
 *
 * @param errorCtx The ctx containing the data to get.
 * @param errorCtxParam The type of info to get.
 * @param getInfo The address of a pointer. The function will go to
 * that address and deposit a pointer to the info requested.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtGetErrorCtxParam (
   VtErrorCtx errorCtx,
   VtErrorCtxParam errorCtxParam,
   Pointer *getInfo
);

/* The following are the ErrorCtxParams currently supported. Each
 * Param is used in conjunction with special info for the function. If
 * there is no special info, the accompaniment is a NULL pointer.
 */

/** SetParam and GetParam.
 * <p>Use this ErrorCtxParam to set the error ctx with a list of errors
 * the error ctx should not log. For example, an app may want the error
 * ctx to ignore VT_ERROR_BUFFER_TOO_SMALL.
 * <p>Or it is used to get the excluded error list out of a context.
 * <p>If an error ctx already has some excluded errors, this Param will
 * add the new excluded errors, previously excluded errors are still
 * excluded.
 * <p>The associated info is a VtExcludedErrors struct.
 * <p>When setting, build the VtExcludedErrors struct and pass a
 * pointer to the struct as the associatedInfo.
 * <p>When getting, pass in the address of a pointer as the getInfo,
 * the Get function will deposit a pointer to a VtExcludedErrors struct
 * at the address. All memory belongs to the context out of which the
 * getInfo came.
 */
extern VtErrorCtxParam VtErrorCtxParamExcludedErrors;

/** This is the data struct to accompany VtErrorCtxParamExcludedErrors.
 * Build an array of ints, set each int in the array to an error to be
 * excluded, set the excludedErrors field to that array and set count
 * to the number of excluded errors.
 */
typedef struct
{
  int *excludedErrors;
  unsigned int count;
} VtExcludedErrors;

/** Get the number of errors currently residing in the error ctx. This
 * will be the number of PRIMARY errors, or error stacks.
 * <p>Note that some error ctx implementations may have at most only
 * one error at any one time. That is, for some implementations, the
 * count will always be either 0 or 1.
 * <p>The function will go to the address given by count and deposit an
 * unsigned int, the number of errors in the ctx.
 *
 * @param errorCtx The context to query.
 * @param count The address where the function will deposit the number
 * of errors.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtGetErrorCtxCount (
   VtErrorCtx errorCtx,
   unsigned int *count
);

/** Get the error stack at the given index. The index is based on the
 * count of stacks in the ctx (counting beings at 0, see
 * VtGetErrorCtxCount).
 * <p>Note that some error ctx implementations may have at most only
 * one error at any one time. That is, for some implementations, the
 * iindex must always be 0.
 * <p>If there is no error stack at the given index, the function
 * returns the VT_ERROR_GET_INFO_UNAVAILABLE error (but does not log
 * the error, no errors caused by error ctx functions log errors).
 * <p>The function will go to the address given by errorStack and
 * deposit a pointer to the stack. The memory for the stack belongs to
 * the errorCtx, do not alter or free that memory.
 *
 * @param errorCtx The context to query.
 * @param index The index into the list of stacks inside the ctx of the
 * particular stack to return.
 * @param errorStack The address where the function will deposit a
 * pointer to the stack.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtGetErrorStack (
   VtErrorCtx errorCtx,
   unsigned int index,
   VtErrorStack **errorStack
);

/** Clear all errors in the error ctx. This will set the error ctx to a
 * state that indicates there are no errors.
 *
 * @param errorCtx The context to clear.
 * @return an int, 0 if the function completed successfully or a
 * non-zero error code.
 */
int VT_CALLING_CONV VtClearErrorCtx (
   VtErrorCtx errorCtx
);

/*@}*/

/* Error codes.
 */

/**
 * \defgroup ErrorGroup Error Codes
 */

/*@{*/
#define VT_ERROR_BASE                     0
#define VT_ERROR_CRYPTO_BASE              VT_ERROR_BASE+0
#define VT_ERROR_VIBE_BASE                VT_ERROR_CRYPTO_BASE+512
#define VT_ERROR_FIPS_BASE                VT_ERROR_VIBE_BASE+512

/* These are error codes vibecrypto.lib can return. Other libs can also
 * return these errors.
 */
#define VT_ERROR_GENERAL                  VT_ERROR_CRYPTO_BASE+1
#define VT_ERROR_UNIMPLEMENTED            VT_ERROR_CRYPTO_BASE+2
#define VT_ERROR_MEMORY                   VT_ERROR_CRYPTO_BASE+3
#define VT_ERROR_FIPS                     VT_ERROR_CRYPTO_BASE+4
#define VT_ERROR_LOAD_LIBRARY             VT_ERROR_CRYPTO_BASE+5
#define VT_ERROR_PROC_ADDRESS             VT_ERROR_CRYPTO_BASE+6
#define VT_ERROR_FULL_NAME                VT_ERROR_CRYPTO_BASE+7
#define VT_ERROR_NULL_ARG                 VT_ERROR_CRYPTO_BASE+8
#define VT_ERROR_NON_NULL_ARG             VT_ERROR_CRYPTO_BASE+9
#define VT_ERROR_INVALID_CALL_ORDER       VT_ERROR_CRYPTO_BASE+10
#define VT_ERROR_INVALID_IMPL             VT_ERROR_CRYPTO_BASE+11
#define VT_ERROR_INVALID_SET              VT_ERROR_CRYPTO_BASE+12
#define VT_ERROR_INVALID_ASSOCIATED_INFO  VT_ERROR_CRYPTO_BASE+13
#define VT_ERROR_INVALID_GET              VT_ERROR_CRYPTO_BASE+14
#define VT_ERROR_GET_INFO_UNAVAILABLE     VT_ERROR_CRYPTO_BASE+15
#define VT_ERROR_INVALID_KEY_LENGTH       VT_ERROR_CRYPTO_BASE+16
#define VT_ERROR_INVALID_INPUT            VT_ERROR_CRYPTO_BASE+17
#define VT_ERROR_INVALID_INPUT_LENGTH     VT_ERROR_CRYPTO_BASE+18
#define VT_ERROR_INVALID_ENCODING         VT_ERROR_CRYPTO_BASE+19
#define VT_ERROR_INVALID_PAD              VT_ERROR_CRYPTO_BASE+20
#define VT_ERROR_BUFFER_TOO_SMALL         VT_ERROR_CRYPTO_BASE+21
#define VT_ERROR_INVALID_LIB_CTX          VT_ERROR_CRYPTO_BASE+22
#define VT_ERROR_INVALID_LIB_CTX_PARAM    VT_ERROR_CRYPTO_BASE+23
#define VT_ERROR_INVALID_OBJ              VT_ERROR_CRYPTO_BASE+24
#define VT_ERROR_INVALID_ALG_OBJ          VT_ERROR_CRYPTO_BASE+25
#define VT_ERROR_INVALID_KEY_OBJ          VT_ERROR_CRYPTO_BASE+26
#define VT_ERROR_INVALID_PARAM_OBJ        VT_ERROR_CRYPTO_BASE+27
#define VT_ERROR_INVALID_PARAM_LENGTH     VT_ERROR_CRYPTO_BASE+28
#define VT_ERROR_INVALID_PARAMS           VT_ERROR_CRYPTO_BASE+29
#define VT_ERROR_CURVE_TYPE_NOT_SUPPORTED VT_ERROR_CRYPTO_BASE+30
#define VT_ERROR_INVALID_MP_INT_CTX       VT_ERROR_CRYPTO_BASE+31
#define VT_ERROR_INVALID_TYPE             VT_ERROR_CRYPTO_BASE+32
#define VT_ERROR_NO_RANDOM_OBJECT         VT_ERROR_CRYPTO_BASE+33
#define VT_ERROR_RANDOM_OBJECT            VT_ERROR_CRYPTO_BASE+34
#define VT_ERROR_INSUFFICIENT_SEED        VT_ERROR_CRYPTO_BASE+35
#define VT_ERROR_NOT_CLONABLE             VT_ERROR_CRYPTO_BASE+36
#define VT_ERROR_STATE_NOT_AVAILABLE      VT_ERROR_CRYPTO_BASE+37
#define VT_ERROR_INVALID_STATE            VT_ERROR_CRYPTO_BASE+38
#define VT_ERROR_NO_PRIME_FOUND           VT_ERROR_CRYPTO_BASE+39
#define VT_ERROR_NO_IBE_CURVE_FOUND       VT_ERROR_CRYPTO_BASE+40
#define VT_ERROR_NO_IBE_PARAMS_FOUND      VT_ERROR_CRYPTO_BASE+41
#define VT_ERROR_UNSUPPORTED_MEMORY_TYPE  VT_ERROR_CRYPTO_BASE+42
#define VT_ERROR_UNSUPPORTED_THREAD_TYPE  VT_ERROR_CRYPTO_BASE+43
#define VT_ERROR_BAD_KEY_GEN              VT_ERROR_CRYPTO_BASE+44
#define VT_ERROR_UNMATCHED_KEY_PAIR       VT_ERROR_CRYPTO_BASE+45
#define VT_ERROR_SIGNATURE_COMPUTATION    VT_ERROR_CRYPTO_BASE+46
#define VT_ERROR_NO_MATH_LIBRARY          VT_ERROR_CRYPTO_BASE+47
#define VT_ERROR_MP_PROVIDER              VT_ERROR_CRYPTO_BASE+48
#define VT_ERROR_MP_INT_RANGE             VT_ERROR_CRYPTO_BASE+49
#define VT_ERROR_DIVIDE_BY_ZERO           VT_ERROR_CRYPTO_BASE+50
#define VT_ERROR_NO_INVERSE               VT_ERROR_CRYPTO_BASE+51
#define VT_ERROR_INVALID_SURRENDER_SET    VT_ERROR_CRYPTO_BASE+52
#define VT_ERROR_INVALID_SURRENDER_INFO   VT_ERROR_CRYPTO_BASE+53
#define VT_ERROR_SURRENDER_CANCEL         VT_ERROR_CRYPTO_BASE+54
#define VT_ERROR_INVALID_RANDOM_OBJ       VT_ERROR_CRYPTO_BASE+55
#define VT_ERROR_INVALID_IBE_CACHE_CTX    VT_ERROR_CRYPTO_BASE+56
#define VT_ERROR_INVALID_ERROR_CTX        VT_ERROR_CRYPTO_BASE+57
#define VT_ERROR_INVALID_PRI_KEY_OBJ      VT_ERROR_CRYPTO_BASE+58
#define VT_ERROR_INVALID_PUB_KEY_OBJ      VT_ERROR_CRYPTO_BASE+59
#define VT_ERROR_INVALID_ENCODE_OBJ       VT_ERROR_CRYPTO_BASE+60
#define VT_ERROR_INVALID_CIPHER_OBJ       VT_ERROR_CRYPTO_BASE+61
#define VT_ERROR_INVALID_DIGEST_OBJ       VT_ERROR_CRYPTO_BASE+62
#define VT_ERROR_INVALID_MAC_OBJ          VT_ERROR_CRYPTO_BASE+63
#define VT_ERROR_INVALID_SIGN_OBJ         VT_ERROR_CRYPTO_BASE+64
#define VT_ERROR_INVALID_VERIFY_OBJ       VT_ERROR_CRYPTO_BASE+65
#define VT_ERROR_INVALID_KEY_AGREE_OBJ    VT_ERROR_CRYPTO_BASE+66
#define VT_ERROR_NO_IMPL                  VT_ERROR_CRYPTO_BASE+67
#define VT_ERROR_NO_ELEMENT_AT_INDEX      VT_ERROR_CRYPTO_BASE+68
#define VT_ERROR_INVALID_DELETE           VT_ERROR_CRYPTO_BASE+69
#define VT_ERROR_THREAD_LOCAL_DATA        VT_ERROR_CRYPTO_BASE+70

/* The following are FIPS error codes. These are the values which the
 * functions VtCreateLibCtxFips and VtGetFipsError will return. These
 * codes will not be returned by any other toolkit function.
 * The function VtCreateLibCtxFips can return a FIPS error or a regular
 * error.
 *
 * Possible error combinations for functions other than
 * VtCreateLibCtxFips and VtGetFipsError
 *      regular              FIPS
 *  -----------------------------------------------------------------
 *       error                 0      Some regular error (such as
 *                                    MEMORY, INVALID_INPUT_LENGTH,
 *                                    etc.) prevents fnct from
 *                                    executing, but there is no
 *                                    FIPS error to be cleared. If
 *                                    the fnct that failed is a
 *                                    FIPS operation, it was not a
 *                                    FIPS error, so the module is
 *                                    still active.
 *         0                 error    Fnct does no FIPS work, it
 *                                    successfully executed. There
 *                                    is a FIPS error to be cleared
 *                                    if the app wants to do FIPS
 *                                    work. FIPS module is not
 *                                    active.
 *   VT_ERROR_FIPS           error    Fnct will not execute, there
 *                                    is no regular error (MEMORY,
 *                                    INVALID_SET_INFO, etc.) but
 *                                    op is FIPS related and there
 *                                    is some FIPS error yet to be
 *                                    cleared. FIPS module is not
 *                                    active.
 */
#define VT_ERROR_FIPS_SELF_TESTS             VT_ERROR_FIPS_BASE+1
#define VT_ERROR_FIPS_SW_INTEGRITY           VT_ERROR_FIPS_BASE+10
#define VT_ERROR_FIPS_DSA_CONSISTENCY        VT_ERROR_FIPS_BASE+20
#define VT_ERROR_FIPS_RSA_SIGN_VERIFY        VT_ERROR_FIPS_BASE+21
#define VT_ERROR_FIPS_RSA_ENC_DEC            VT_ERROR_FIPS_BASE+22
#define VT_ERROR_FIPS_DH_CONSISTENCY         VT_ERROR_FIPS_BASE+23
#define VT_ERROR_FIPS_IBE_CONSISTENCY        VT_ERROR_FIPS_BASE+24
#define VT_ERROR_FIPS_KNOWN_ANSWER_SHA1      VT_ERROR_FIPS_BASE+30
#define VT_ERROR_FIPS_KNOWN_ANSWER_SHA2      VT_ERROR_FIPS_BASE+31
#define VT_ERROR_FIPS_KNOWN_ANSWER_MD5       VT_ERROR_FIPS_BASE+32
#define VT_ERROR_FIPS_KNOWN_ANSWER_HMAC      VT_ERROR_FIPS_BASE+33
#define VT_ERROR_FIPS_KNOWN_ANSWER_AES       VT_ERROR_FIPS_BASE+34
#define VT_ERROR_FIPS_KNOWN_ANSWER_DES       VT_ERROR_FIPS_BASE+35
#define VT_ERROR_FIPS_KNOWN_ANSWER_3DES      VT_ERROR_FIPS_BASE+36
#define VT_ERROR_FIPS_KNOWN_ANSWER_186_DRNG  VT_ERROR_FIPS_BASE+37
#define VT_ERROR_FIPS_DRNG_SEED              VT_ERROR_FIPS_BASE+50
#define VT_ERROR_FIPS_DRNG_OUTPUT            VT_ERROR_FIPS_BASE+51
#define VT_ERROR_FIPS_DSA_PAIR_GEN           VT_ERROR_FIPS_BASE+60
#define VT_ERROR_FIPS_RSA_PAIR_GEN           VT_ERROR_FIPS_BASE+61
#define VT_ERROR_FIPS_DH_PAIR_GEN            VT_ERROR_FIPS_BASE+62
#define VT_ERROR_FIPS_IBE_PRI_DERIVE         VT_ERROR_FIPS_BASE+63

/*@}*/

#ifdef __cplusplus
}
#endif

#endif /* _VIBE_CRYPTO_H */
