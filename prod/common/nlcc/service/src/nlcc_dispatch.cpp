
#include <windows.h>
#include <winioctl.h>
#include <cstdio>
#include <cstdlib>

#include "jni.h"
#include "nlconfig.hpp"
#include "nlcc.h"
#include "nlcc_ioctl.h"
#include "nlcc_ulib.h"

#include "celog.h"
#include "celog_policy_file.hpp"

#include "com_bluejungle_NLCC_NLCCServiceDispatcher.h"

//#define NLCC_PASSTHROUGH

#ifdef NLCC_PASSTHROUGH
#  pragma warning( disable : 4702 )
#endif

static CELog nlcclog;
static HANDLE h;

extern "C"
JNIEXPORT jboolean JNICALL Java_com_bluejungle_NLCC_NLCCServiceDispatcher_Open
  (JNIEnv *, jobject)
{
  jboolean rv = JNI_TRUE;
  wchar_t logfile[MAX_PATH] = {0};
  bool result;

  result = NLConfig::ReadKey(L"SOFTWARE\\NextLabs\\Compliant Enterprise\\Policy Controller\\PolicyControllerDir",
			     logfile,_countof(logfile));
  if( result == true )
  {
    CELogPolicy_File* fp = NULL;
    wcsncat_s(logfile,_countof(logfile),L"\\agentLog\\nlcc.log", _TRUNCATE);
    char logfilea[MAX_PATH] = {0};
    _snprintf_s(logfilea,MAX_PATH, _TRUNCATE,"%ws",logfile);

    try
    {
      fp = new CELogPolicy_File(logfilea);
      nlcclog.SetPolicy(fp);
    }
    catch( ... )
    {
    }
  }

  nlcclog.SetLevel(CELOG_WARNING);
  if( NLConfig::IsDebugMode() == true )
  {
    nlcclog.SetLevel(CELOG_DEBUG);
  }

  nlcclog.Enable();

  nlcclog.Log(CELOG_INFO, "Open '%ws'\n", NLCC_PDP_DEVICE);
  h = CreateFileW(NLCC_PDP_DEVICE,GENERIC_READ|GENERIC_WRITE,0,NULL,OPEN_EXISTING,
		  FILE_ATTRIBUTE_NORMAL|FILE_FLAG_OVERLAPPED,
		  NULL);
  if( h == INVALID_HANDLE_VALUE )
  {
    nlcclog.Log(CELOG_CRIT, "Open: CreateFile failed (%d)\n", GetLastError());
    rv = JNI_FALSE;
  }
  nlcclog.Log(CELOG_INFO, "Open: handle 0x%x\n", h);
  return rv;
}/* NLCCServiceDispatcher_Open */

extern "C"
JNIEXPORT jobjectArray JNICALL Java_com_bluejungle_NLCC_NLCCServiceDispatcher_ReadRequest
  (JNIEnv* env , jobject )
{
  jobjectArray objects = NULL;
  PNLCC_QUERY request = NULL;
  BOOL rv;
  DWORD bytes_out = 0;
  OVERLAPPED ol;
  errno_t err = 0;

  nlcclog.Log(CELOG_DEBUG, "ReadRequest: begin: POLICY_REQUEST I/O (h 0x%x)\n", h);

  /* Read request */
  request = (PNLCC_QUERY)malloc( sizeof(NLCC_QUERY) );
  if( request == NULL )
  {
    nlcclog.Log(CELOG_CRIT, "ReadRequest: out of memory\n");
    goto NLCCServiceDispatcher_ReadRequest_complete;
  }
  NLCC_UInitializeQuery(request);

  memset(&ol,0x00,sizeof(ol));
  ol.hEvent = CreateEventW(NULL,TRUE,FALSE,NULL);
  if( ol.hEvent == NULL )
  {
    goto NLCCServiceDispatcher_ReadRequest_complete;
  }

  /* Read a policy request (request for decision) form the NLCC driver */
  rv = DeviceIoControl(h,IOCTL_POLICY_REQUEST,NULL,0,request,sizeof(*request),&bytes_out,&ol);
  if( rv == FALSE )
  {
    /* Pending I/O will return false and use ERROR_IO_PENDING.  Retrieve the result. */
    if( GetLastError() == ERROR_IO_PENDING )
    {
      rv = GetOverlappedResult(h,&ol,&bytes_out,TRUE);
    }
  }
  CloseHandle(ol.hEvent);

  /* If the I/O has failed fail to caller. */
  if( rv == FALSE )
  {
    nlcclog.Log(CELOG_ERR, "ReadRequest: DeviceIoControl failed (le %d)\n", GetLastError());
    goto NLCCServiceDispatcher_ReadRequest_complete;
  }

  /* Read size must match the expected size */
  if( bytes_out != sizeof(*request) )
  {
    nlcclog.Log(CELOG_ERR, "IOCTL_POLICY_REQUEST failed read size (%d but should be %d)\n", bytes_out, sizeof(*request));
    goto NLCCServiceDispatcher_ReadRequest_complete;
  }

  /* Add the PID as an attribute */
  wchar_t temp[32] = {0};
  _itow_s(request->info.request.pid,temp,_countof(temp),10);
  if (NLCC_UAddAttribute(request,L"pid",temp) != 0)
  {
    goto NLCCServiceDispatcher_ReadRequest_complete;
  }

	/* Add the IP as an attribute */
	_itow_s(request->info.request.ip,temp,_countof(temp),10);
  if (NLCC_UAddAttribute(request,L"ip",temp) != 0)
  {
    goto NLCCServiceDispatcher_ReadRequest_complete;
  }

  /* calculate number of attributes */
  jsize num_attrs = 0;
  for( size_t i = 0 ; ; i++ , num_attrs++ )
  {
    const wchar_t* key = NULL;
    const wchar_t* value = NULL;
    if( NLCC_UGetAttributeByIndex(request,i,&key,&value) != 0 )
    {
      break;
    }
  }

#ifdef NLCC_PASSTHROUGH
  /* Passthrough will respond immediatly but permit the query to go through the PDP */
  PNLCC_QUERY response = (PNLCC_QUERY)malloc( sizeof(NLCC_QUERY) );
  if( response == NULL )
  {
    nlcclog.Log(CELOG_CRIT, "ReadRequest: out of memory\n");
    goto NLCCServiceDispatcher_ReadRequest_complete;
  }
  NLCC_UInitializeQuery(response);
  response->tx_id = request->tx_id;
  response->info.response.allow = 1; /* TRUE (Allow) */
  nlcclog.Log(CELOG_DEBUG, L"Sending passthrough response (TX ID %d) : allow %d\n",
	      response->tx_id, response->info.response.allow);
  rv = DeviceIoControl(h,IOCTL_POLICY_RESPONSE,response,sizeof(*response),NULL,0,&bytes_out,NULL);
  free(response);
#endif

  num_attrs += 1; /* space for first two reserved spaces */

  nlcclog.Log(CELOG_DEBUG, "Read request: %d (bytes out %d) : %d attrs : TX ID %d\n",
	      rv,bytes_out,num_attrs,request->tx_id);

  objects = (jobjectArray)env->NewObjectArray(num_attrs * 2,env->FindClass("java/lang/String"),
					      env->NewStringUTF(""));

  wchar_t tx_id_string[256] = {0};
  err = _i64tow_s(request->tx_id,tx_id_string,_countof(tx_id_string),10);
  if( err != 0 )
  {
    nlcclog.Log(CELOG_CRIT, "Read request: TX ID conversion failed (errno %d)\n",err);
  }
  env->SetObjectArrayElement(objects,0,env->NewString((const jchar*)tx_id_string,(jsize)wcslen(tx_id_string)));

  nlcclog.Log(CELOG_DEBUG,L"ReadRequest: packing attributes\n");

  /* Pack attributes into the JNI array */
  int index = 2;
  for( size_t i = 0 ; ; i++ )
  {
    const wchar_t* key = NULL;
    const wchar_t* value = NULL;
    if( NLCC_UGetAttributeByIndex(request,i,&key,&value) != 0 )
    {
      break;
    }

    if( key == NULL || value == NULL )
    {
      nlcclog.Log(CELOG_ERR, L"attr[%d]: key 0x%p , value 0x%p\n", i, key, value);
      break;
    }

    nlcclog.Log(CELOG_DEBUG, L"attr[%d] (%d,%d): %ws = %ws\n", i, index + 0, index + 1, key, value);

    /* Set key/value */
    env->SetObjectArrayElement(objects,index + 0,env->NewString((const jchar*)key,(jsize)wcslen(key)));
    env->SetObjectArrayElement(objects,index + 1,env->NewString((const jchar*)value,(jsize)wcslen(value)));

    index += 2;
  }

 NLCCServiceDispatcher_ReadRequest_complete:

  if( request != NULL )
  {
    free(request);
  }

  nlcclog.Log(CELOG_DEBUG, L"ReadRequest done\n");

  return objects;
}/* NLCCServiceDispatcher_ReadRequest */

extern "C"
JNIEXPORT jboolean JNICALL Java_com_bluejungle_NLCC_NLCCServiceDispatcher_SendResult
(JNIEnv* env, jobject , jlong in_tx_id , jboolean in_allow , jobjectArray in_objects )
{
#ifdef NLCC_PASSTHROUGH
  return JNI_TRUE;
#endif

  jboolean result = JNI_TRUE;
  BOOL rv;
  DWORD bytes_out = 0;
  OVERLAPPED ol;
  PNLCC_QUERY response;

  response = (PNLCC_QUERY)malloc( sizeof(NLCC_QUERY) );
  if( response == NULL )
  {
    nlcclog.Log(CELOG_CRIT, "ReadRequest: out of memory\n");
    goto SendResult_complete;
  }

  NLCC_UInitializeQuery(response);

  response->tx_id = in_tx_id;
  response->info.response.allow = NLCC_POLICY_RESULT_ALLOW; /* TRUE (Allow) */
  if( !in_allow )
  {
    response->info.response.allow = NLCC_POLICY_RESULT_DENY; /* FALSE (Deny) */
  }

  jobject ob = NULL;
  jsize len = 0;

  len = env->GetArrayLength(in_objects);
  nlcclog.Log(CELOG_DEBUG, L"SendResult: processing obligations (%d)\n", len);
  for( int i = 0 ; i < len ; i++ )
  {
    ob = env->GetObjectArrayElement(in_objects,i);

    const wchar_t* cline = (const wchar_t*)env->GetStringChars((jstring)ob,0);
    if( cline == NULL )
    {
      nlcclog.Log(CELOG_CRIT, L"SendResult: obligation at index %d failed\n");
      continue;
    }
    assert( cline != NULL );
    if (NLCC_UAddAttribute(response,L"attr",cline) != 0)
    {
        continue;
    }
    env->ReleaseStringChars((jstring)ob,(const jchar*)cline);
  }/* for objects */

  nlcclog.Log(CELOG_DEBUG, L"Sending response (TX ID %d) : allow %d\n",
  	      response->tx_id, response->info.response.allow);

  memset(&ol,0x00,sizeof(ol));
  ol.hEvent = CreateEventW(NULL,TRUE,FALSE,NULL);
  if( ol.hEvent == NULL )
  {
    result = JNI_FALSE;
    goto SendResult_complete;
  }
  rv = DeviceIoControl(h,IOCTL_POLICY_RESPONSE,response,sizeof(*response),NULL,0,&bytes_out,&ol);
  if( rv == FALSE )
  {
    /* Pending I/O will return false and use ERROR_IO_PENDING.  Retrieve the result.
     */
    if( GetLastError() == ERROR_IO_PENDING )
    {
      rv = GetOverlappedResult(h,&ol,&bytes_out,TRUE);
    }
  }
  if( rv == FALSE )
  {
    nlcclog.Log(CELOG_WARNING, L"Sending response - failed (le %d)\n",GetLastError());
    result = JNI_FALSE;
  }

  CloseHandle(ol.hEvent);
  free(response);

 SendResult_complete:

  nlcclog.Log(CELOG_DEBUG, L"Sending response - complete (TX ID %d)\n", in_tx_id);

  return result;
}
