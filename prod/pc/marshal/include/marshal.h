/*========================marshal.h=========================================*
 *                                                                          *
 * All sources, binaries and HTML pages (C) copyright 2006 by Blue Jungle., *
 * Redwood City CA, Ownership remains with Blue Jungle Inc,                 * 
 * All rights reserved worldwide.                                           *
 *                                                                          * 
 * Author : Heidi Zhou                                                      *
 * Date   : 12/26/2006                                                      *
 * Note   : This file includes the declarations of the RPC layer            *
 *          marshalling APIs and global variables used by the other modules *
 *          inside CE enforcer.                                             *
 *==========================================================================*/

#ifndef __CE_MARSHALL_H
#define __CE_MARSHALL_H

#include "nlstrings.h"
#include "CEsdk.h"
#include <string>
#include <vector>

using namespace std;

//Some constants
//On MS Windows, it needs to reserve space for std::vector.
//Thus, RPC_MAX_NUM_ARGUMENTS makes sure that returned RPC
//arguments vector is big enough.
enum {RPC_MAX_NUM_ARGUMENTS=20};

/*CEMarshalFunc*/
struct CEMarshalFunc
{
  nlstring funcName;

  //the request ID of function 
  unsigned requestID:32; 

  //the vector of input argument type
  //
  // most method calls are fixed format. The type and number of arguments
  // is defined by the contents of inputArgs:
  // 1. Every input arguments array includes the string in the format of
  //    "<thread-id>+<request-time>" as the first input argument, called
  //    "reqID". This argument is the identifier of  a request from a client
  //    at a certain time. 
  // 2. After "reqID", it is the list of public arguments that match the
  //    the SDK API's signature
  // 3. After the public arguments' list, it is the list of private arguments
  //    that need to be sent over the socket and used by client/server stub
  //    code.
  //
  // If this vector is empty then we are doing a free-form method call
  // where the type and number of arguments is defined by a format string
  vector<nlstring> inputArgs; 

  //the reply ID of function 
  unsigned replyID:32; 

  // the vector of output argument type
  //
  // as with the input arguments, most responses are fixed format. The
  // type and number of arguments is defined by the contents of outputArgs
  // Layout:
  // 1. Every output arguments array include the string in the format of
  //    "<thread-id>+<request-time>" as the first output argument, called
  //    "reqID". This argument is the identifier of a request from a client
  //    at a certain time. 
  // 2. After "reqID", it is the list of public arguments that match SDK
  //    API's signature.
  // 3. After the public arguments' list, it is the list of private arguments
  //    that need to be sent over the socket and used by client/server stub
  //    code.
  //
  // If this vector is empty then we are doing a free-form method call
  // where the type and number of arguments is defined by a format string
  vector<nlstring> outputArgs; 

  CEMarshalFunc(const nlchar *n, const unsigned q, 
		int in, const nlchar **inArgs,
		const unsigned p, int on, const nlchar **outArgs);

  bool inputHasFixedFormat() {
    return inputArgs.size() != 0;
  }

  bool outputHasFixedFormat() {
    return outputArgs.size() != 0;
  }
  
};
typedef struct CEMarshalFunc CEMarshalFunc;
/*CEMarshalFunc end*/

/* =======================Marshal_PackReqFunc=============================*
 * Generate a buffer for a request function that will be sent over the    *
 * socket.                                                                *
 * Parameters:                                                            *
 * funcName (input): the name of the request function. The function has to*
 *                   be the valid CE function.                            *
 * argv (input): the vector of the pointers to function input arguments.  *
 * reqLen (output): the length of retruned function layout.               *
 *                                                                        *
 * Return:                                                                *
 *   It will return NULL if the function fails; otherwise, it will        *
 *   return the pointer to the buffer of function layout.                 *
 * Example:                                                               *
 *   CEHandle h;                                                          *
 *   std::vector<void *> argbuf;                                          *
 *   int reqLen;                                                          *
 *   ...                                                                  *
 *   argBuf.push_back((void *)(&h));                                      *  
 *   char *buf=Marshal_PackReqFunc("FUNC_CONN_Close", argBuf, reqLen);    *
 *   Marshal_PackFree(buf);                                               *
 *   buf=NULL;                                                            *
 * Note:                                                                  *
 *   It is caller's responsibility call the function "Marshal_PackFree" to*
 *   free the memory of buffer returned by the function                   *
 *   "Marshal_PackReqFunc".                                               *
 *                                                                        *
 *   The order of arguments vector has to be same as the order of         *
 *   request function's input arguments.                                  *  
 * =======================================================================*/
char *Marshal_PackReqFunc(const nlchar *funcName, vector<void *> &argv,
			  size_t &reqLen);

/* =======================Marshal_PackReqGeneric==========================*
 * Generate a buffer for a format string that will be sent over the       *
 * socket.                                                                *
 * The function name is GENERIC_FUNCTION, which has a specific handler on *
 * the PDP side. If you want a different function name then use           *
 * Marshal_PackReqFuncAndFormat                                           *
 * Parameters:                                                            *
 * fmt (input): the format string describing the data                     *
 * argv (input): the vector of the pointers to function input arguments.  *
 * reqLen (output): the length of retruned function layout.               *
 *                                                                        *
 * Return:                                                                *
 *   It will return NULL if the function fails; otherwise, it will        *
 *   return the pointer to the buffer of function layout.                 *
 * Example:                                                               *
 *   CEInt32 arg1;                                                        *
 *   CEInt32 arg2;                                                        *
 *   std::vector<void *> argbuf;                                          *
 *   int reqLen;                                                          *
 *   ...                                                                  *
 *   argBuf.push_back((void *)(&arg1));                                   *  
 *   argBuf.push_back((void *)(&arg2));                                   *  
 *   char *buf=Marshal_PackReqGeneric("ii", argBuf, reqLen);              *
 *   Marshal_PackFree(buf);                                               *
 *   buf=NULL;                                                            *
 * Note:                                                                  *
 *   It is caller's responsibility call the function "Marshal_PackFree"   *
 *   to free the memory of buffer returned by the function                *
 *   "Marshal_PackReqFunc".                                               *
 * =======================================================================*/
char *Marshal_PackReqGeneric(const nlchar *fmt, vector<void *> &argv,
			  size_t &reqLen);

/* =======================Marshal_PackReqFuncAndFormat====================*
 * Generate a buffer for a format string that will be sent over the       *
 * socket. This provides named functions with variable arguments (a sort  *
 * of combination of PackReqFunc and PackRegGeneric                       *
 * Parameters:                                                            *
 * funcName (input): the name of the request function. The function has to*
 *                   be the valid CE function.                            *
 * fmt (input): the format string describing the data                     *
 * argv (input): the vector of the pointers to function input arguments.  *
 * reqLen (output): the length of retruned function layout.               *
 *                                                                        *
 * Return:                                                                *
 *   It will return NULL if the function fails; otherwise, it will        *
 *   return the pointer to the buffer of function layout.                 *
 * Example:                                                               *
 *   CEInt32 arg1;                                                        *
 *   CEInt32 arg2;                                                        *
 *   std::vector<void *> argbuf;                                          *
 *   int reqLen;                                                          *
 *   ...                                                                  *
 *   argBuf.push_back((void *)(&arg1));                                   *  
 *   argBuf.push_back((void *)(&arg2));                                   *  
 *   char *buf=Marshal_PackReqFuncAndFormat(_T("myfunc"), "ii",           *
 *                                          argBuf, reqLen);              *
 *   Marshal_PackFree(buf);                                               *
 *   buf=NULL;                                                            *
 * Note:                                                                  *
 *   It is caller's responsibility call the function "Marshal_PackFree"   *
 *   to free the memory of buffer returned by the function                *
 *   "Marshal_PackReqFuncAndFormat".                                      *
 * =======================================================================*/
char *Marshal_PackReqFuncAndFormat(const nlchar *funcName, const nlchar *fmt,
                                   vector<void *> &argv, size_t &reqLen);

/* =======================Marshal_PackFuncReply===========================*
 * Generate a buffer for a CE request function's reply that will be sent  *
 * over the socket.                                                       *
 * Parameters:                                                            *
 * funcName (input): the name of the replied function. The function has to*
 *                   be the valid CE function.                            *
 * result: the return value in CEResult type.                             *
 * argv (input): the pointer to the array of returned arguments.          *
 * replyLen (output): the length of retruned function reply layout.       *
 *                                                                        *
 * Return:                                                                *
 *   It will return NULL if the function fails; otherwise, it will        *
 *   return the pointer to the buffer of function layout.                 *
 * Example:                                                               *
 *   CEResult r;                                                          *
 *   CEHandle h;                                                          *
 *   int replyLen;                                                        *
 *   std::vector<void *> returnArgv;                                      *
 *   ...                                                                  *
 *   returnArgv.push_back((void *)(&h));                                  *
 *   char *buf=Marshal_PackFuncReply("CONN_Initialize", r, returnArgv,    *
 *                                   replyLen);                           *
 *   Marshal_PackFree(buf);                                               *
 *   buf=NULL;                                                            *
 * Note:                                                                  *
 *   It is caller's responsibility to call the function "Marshal_PackFree"*
 *   to free the memory of buffer returned by the function                *
 *   "Marshal_PackFuncReply".                                             * 
 *                                                                        *
 *   The order of elements in vector is same as the order of request      *
 *   function's returned arguments.                                       *  
 * =======================================================================*/
char *Marshal_PackFuncReply(const nlchar *funcName, 
			    CEResult_t result, 
			    vector<void *> &argv,
			    size_t &replyLenInByte);

/* =======================Marshal_PackReplyGeneric========================*
 * Generate a buffer for a CE request function's reply that will be sent  *
 * over the socket. The "funcName" is assumed to be "GENERIC_FUNCTION".   *
 * if you want to supply your own, use PackFuncAndFormatReply             *
 * Parameters:                                                            *
 * fmt (input): a format string describing the data                       *
 * result: the return value in CEResult type.                             *
 * argv (input): the pointer to the array of returned arguments.          *
 * replyLen (output): the length of retruned function reply layout.       *
 *                                                                        *
 * Return:                                                                *
 *   It will return NULL if the function fails; otherwise, it will        *
 *   return the pointer to the buffer of function layout.                 *
 * Example:                                                               *
 *   CEResult r;                                                          *
 *   CEInt32 arg;                                                         *
 *   int replyLen;                                                        *
 *   std::vector<void *> returnArgv;                                      *
 *   ...                                                                  *
 *   returnArgv.push_back((void *)(&arg);                                 *
 *   char *buf=Marshal_PackFuncReply("i", r, returnArgv, replyLen);       *
 *   Marshal_PackFree(buf);                                               *
 *   buf=NULL;                                                            *
 * Note:                                                                  *
 *   It is caller's responsibility to call the function "Marshal_PackFree"*
 *   to free the memory of buffer returned by the function                *
 *   "Marshal_PackFuncReply".                                             * 
 * =======================================================================*/
char *Marshal_PackReplyGeneric(const nlchar *fmt, 
                               CEResult_t result, 
                               vector<void *> &argv,
                               size_t &replyLenInByte);

/* =======================Marshal_PackFuncAndFormatReply==================*
 * Generate a buffer for a CE request function's reply that will be sent  *
 * over the socket.                                                       *
 * Parameters:                                                            *
 * funcName (input): the function name                                    *
 * fmt (input): a format string describing the data                       *
 * result: the return value in CEResult type.                             *
 * argv (input): the pointer to the array of returned arguments.          *
 * replyLen (output): the length of retruned function reply layout.       *
 *                                                                        *
 * Return:                                                                *
 *   It will return NULL if the function fails; otherwise, it will        *
 *   return the pointer to the buffer of function layout.                 *
 * Example:                                                               *
 *   CEResult r;                                                          *
 *   CEInt32 arg;                                                         *
 *   int replyLen;                                                        *
 *   std::vector<void *> returnArgv;                                      *
 *   ...                                                                  *
 *   returnArgv.push_back((void *)(&arg);                                 *
 *   char *buf=Marshal_PackFuncReply("i", r, returnArgv, replyLen);       *
 *   Marshal_PackFree(buf);                                               *
 *   buf=NULL;                                                            *
 * Note:                                                                  *
 *   It is caller's responsibility to call the function "Marshal_PackFree"*
 *   to free the memory of buffer returned by the function                *
 *   "Marshal_PackFuncReply".                                             * 
 * =======================================================================*/
char *Marshal_PackFuncAndFormatReply(const nlchar *funcName,
                                     const nlchar *fmt, 
                                     CEResult_t result, 
                                     vector<void *> &argv,
                                     size_t &replyLenInByte);

/* =======================Marshal_PackFree================================*
 * Free the memory allocated for packing request function or reply.       *
 *                                                                        *
 * Parameters:                                                            *
 * bufPtr (input): pointer to the memory to be freed.                     *
 *                                                                        *
 * Return:                                                                *
 *   It will return CE_RESULT_GENERAL_FAILED if the function fails;       *
 *   otherwise, it will CE_RESULT_SUCCESS.                                *
 *                                                                        *
 * Example:                                                               *
 *   CEResult r;                                                          *
 *   CEHandle h;                                                          *
 *   std::vector<void *> returnArgv;                                      *
 *   ...                                                                  *
 *   returnArgv.push_back((void *)(&h));                                  *
 *   char *buf=Marshal_PackFuncReply(FUNC_CONN_Initialize, r, returnArgv);*
 *   Marshal_PackFree(buf);                                               *
 *   buf=NULL;                                                            *
 * Note:                                                                  *
 *   It is caller's responsibility to call function "Marshal_PackFree" to *
 *   free the memory of buffer returned by the function                   *
 *   "Marshal_PackFuncReply".                                             * 
 * =======================================================================*/
CEResult_t Marshal_PackFree(char *bufPtr);

/* =======================Marshal_UnPackReqFunc===========================*
 * Unpack a buffer of a request function.  Return the function name and   *
 * the arry of its input arguments.                                       *
 *                                                                        *
 * Parameters:                                                            *
 * request (input): the buffer storing the request function. The function *
 *                  has to be the valid CE function.                      *
 * funcName (output): the name of request function.                       *
 * argv (output): the vector of function input arguments.                 *
 *                                                                        *
 * Return:                                                                *
 *   It will return CE_RESULT_GENERAL_FAILED if the function fails;       *
 *   otherwise, it will CE_RESULT_SUCCESS.                                *
 * Example:                                                               *
 *   std::string funcName;                                                *    
 *   std::vector<void *> argv;                                            *
 *   ...                                                                  *
 *   CEResult r=Marshal_UnPackReqFunc(request, funcName, argv);           *
 *   if(r == CE_RESULT_SUCCESS) {                                         *
 *     if(funcID=FUNC_CONN_Close)                                         *
 *       CONN_Close((CEHandle)argv[0]);                                   *
 *     Marshal_UnPackFree(funcName.c_str(), argv, true);                  *
 *   }                                                                    *
 * Note:                                                                  *
 *   It is caller's responsibility to call function "Marshal_UnPackFree"  *
 *   to free the memory allocated for request function arguments.         *
 *                                                                        *
 *   The order of elements in vector is same as the order of request      *
 *   function's arguments.                                                *  
 *========================================================================*/
CEResult_t Marshal_UnPackReqFunc(char *request, nlstring &funcName, 
				 vector<void *> &argv);

/* =======================Marshal_UnPackFuncReply=========================*
 * Unpack a buffer of a request function reply.  Return the function      *
 * name, the returned code, and the vector of ite returned arguments.     *
 *                                                                        *
 * Parameters:                                                            *
 * reply  (input): the buffer storing the reply of the request function.  *
 *                 The function has to be the valid CE function.          *
 * funcName (output): the name of request function.                       *
 * result (output): the return code of request function.                  *
 * argv (output): the the vector of function returned arguments.          *
 *                                                                        *
 * Return:                                                                *
 *   It will return CE_RESULT_GENERAL_FAILED if the function fails;       *
 *   otherwise, it will CE_RESULT_SUCCESS.                                *
 * Example:                                                               *
 *   std::string funcName;                                                *    
 *   std::vector<void *> argv;                                            *
 *   CEResult funcRet;                                                    *
 *   ...                                                                  *
 *   CEResult r=Marshal_UnPackFuncReply(reply, funcName, result, argv);   *
 *   if(r == CE_RESULT_SUCCESS) {                                         *
 *     if(funcID=FUNC_CONN_Close)                                         *
 *       std::cout<<"CONN_Close "                                         *
 *                <<(result==CE_RESULT_SUCCESS)?"succeed":"failed"        *
 *                <<std::endl;                                            *
 *     Marshal_UnPackFree(funcName, argv, false);                         *
 *   }                                                                    *
 * Note:                                                                  *
 *   It is caller's responsibility to call function "Marshal_UnPackFree"  *
 *   to free the memory allocated for function reply arguments.           *
 *                                                                        *
 *   The order of elements in vector is same as the order of request      *
 *   function's return arguments.                                         *  
 *========================================================================*/
CEResult_t Marshal_UnPackFuncReply(char *reply, 
				   nlstring &funcName, 
				   CEResult_t &result, 
				   vector<void *> &argv);

/* =======================Marshal_UnPackFree==============================*
 * Free the memory allocated for unpacking request function or reply.     *
 *                                                                        *
 * Parameters:                                                            *
 * argv (input/output): the the vector of pointers to allocated memory.   *
 * funcName (input): the name of the function                             *
 * bRequest (input): true if free the memory for requestion function;     *
 *                   false if free the memory for function reply.         *
 *                                                                        *
 * Return:                                                                *
 *   It will return CE_RESULT_GENERAL_FAILED if the function fails;       *
 *   otherwise, it will CE_RESULT_SUCCESS.                                *
 * Example:                                                               *
 *   std::string funcName;                                                *    
 *   std::vector<void *> argv;                                            *
 *   ...                                                                  *
 *   CEResult r=Marshal_UnPackReqFunc(request, funcName, argv);           *
 *   if(r == CE_RESULT_SUCCESS) {                                         *
 *     if(funcID=FUNC_CONN_Close)                                         *
 *       CONN_Close((CEHandle)argv[0]);                                   *
 *     Marshal_UnPackFree(funcName.c_str(), argv, true);                  *
 *   }                                                                    *
 * Note:                                                                  *
 *   It is caller's responsibility to call function "Marshal_UnPackFree"  *
 *   to free the memory allocated for unpacking request function and reply*
 *========================================================================*/
CEResult_t Marshal_UnPackFree(const nlchar *funcName, 
			      vector<void *> &argv,
			      bool bRequest);

/* =======================Marshal_FreeGeneric=============================*
 * Free the memory allocated for unpacking request function or reply.     *
 * The format of the data (e.g. the first item is a string, the second an *
 * int, etc, is expressed by a format CEString at argv[0]                 *
 *                                                                        *
 * Every item in the vector will be deleted by a call to 'delete', so     *
 * should be allocated with 'new'. This includes CEint32 and CEBoolean,   *
 * as well as more complex types.                                         *
 *                                                                        *
 * Parameters:                                                            *
 * argv (input/output): the the vector of pointers to allocated memory.   *
 *   it is assumed that the first entry of this vector is a "fmt" string. *
 *   See FreeGenericExplicitFormat if this is not the case                *
 *                                                                        *
 * Return:                                                                *
 *   It will return CE_RESULT_GENERAL_FAILED if the function fails;       *
 *   otherwise, it will CE_RESULT_SUCCESS.                                *
 * Example:                                                               *
 *   std::string fmt;                                                     *
 *   std::vector<void *> argv;                                            *
 *   ...                                                                  *
 *   CEResult r=Marshal_UnPackReqGeneric(request, fmt, argv);             *
 *   if(r == CE_RESULT_SUCCESS) {                                         *
 *     Marshal_FreeGeneric(argv);                                         *
 *   }                                                                    *
 * Note:                                                                  *
 *   It is caller's responsibility to call function "Marshal_FreeGeneric" *
 *   to free the memory allocated for unpacking request function and reply*
 *========================================================================*/
CEResult_t Marshal_FreeGeneric(vector<void *> &argv);

/* =======================Marshal_FreeGenericExplicitFormat===============*
 * Free the memory allocated for unpacking request function or reply.     *
 * This is used in cases where the fmt string is *not* part of the argv   *
 * vector (e.g. when constructing, as opposed to receiving the request or *
 * response)                                                             *
 * 
 * See notes about allocation *
 * Parameters:                                                            *
 * fmt (input): the format string describing the data                     *
 * argv (input/output): the the vector of pointers to allocated memory.   *
 *                                                                        *
 * Return:                                                                *
 *   It will return CE_RESULT_GENERAL_FAILED if the function fails;       *
 *   otherwise, it will CE_RESULT_SUCCESS.                                *
 * Example:                                                               *
 *   std::string fmt;                                                     *    
 *   std::vector<void *> argv;                                            *
 *   ...                                                                  *
 *   CEResult r=Marshal_UnPackReqGeneric(request, fmt, argv);             *
 *   if(r == CE_RESULT_SUCCESS) {                                         *
 *     Marshal_FreeGeneric(argv);                                         *
 *   }                                                                    *
 * Note:                                                                  *
 *   It is caller's responsibility to call function "Marshal_FreeGeneric" *
 *   to free the memory allocated for unpacking request function and reply*
 *========================================================================*/
CEResult_t Marshal_FreeGenericExplicitFormat(const nlchar *fmt, vector<void *> &argv);

/* =======================Marsh_GetFuncSignature==========================*
 * Get a SDK API function's signature.                                    *
 *                                                                        *
 * Parameters:                                                            *
 * funcName (input): the name of the function. The function has to        *
 *                   be the valid CE function.                            *
 *                                                                        *
 * Return:                                                                *
 *   It will return NULL if the function fails; otherwise, it will        *
 *   return the CEMarshalFunc type pointer to the function signature.     *
 * Example:                                                               *
 *   CEMarshalFunc *f=Marshal_GetFuncSignature("CONN_Close");             *
 * =======================================================================*/
CEMarshalFunc *Marsh_GetFuncSignature(const nlchar *funcName);

// Marshal_AllocateString() and Marhshal_FreeString() are added to fix bug 10957.
// The bug was caused by ceservice.dll free'ing a CEString allocated by cemarshal.dll.
// Caller of cemarshal needs a way to allocate and free buffers in cemarshal.dll.

// Marshal_AllocateString
// creates CEString based on given character array
#if defined (Linux) || defined (Darwin)
CEString
Marshal_AllocateCEString (const char * str);
#endif 

#if defined (WIN32) || defined (_WIN64)
CEString
Marshal_AllocateCEString (const TCHAR * str);
#endif 

// Marshal_FreeString
// free string allocated by Marshal_AllocateString
CEResult_t Marshal_FreeCEString(CEString str);

#endif /* marshall.h */

