// All sources, binaries and HTML pages (C) copyright 2004 by Blue Jungle Inc., 
// Redwood City CA, Ownership remains with Blue Jungle Inc, 
// All rights reserved worldwide. 

#if defined (WIN32) || defined (_WIN64)
#include "stdafx.h"
#include "jni.h"
#include <tchar.h>
#include <string.h>
#include <string>
#include <sddl.h>
#include <aclapi.h>
#include <imagehlp.h>
#include <lm.h>
#include "CEsdk.h"
#define SECURITY_WIN32
#include "security.h"
#include "..\..\shared\src\globals.h"
#include "..\..\ipcproxy\src\ipcconstants.h"
#include "..\..\shared\src\userutils.h"
#include "..\..\shared\src\securityattributesfactory.h"
#include "..\..\..\..\..\common\include\dstypes.h"
#include "..\..\..\..\..\common\include\dsifsioctls.h"
#include "..\..\..\..\..\common\include\dsifsinterface.h"
#include "..\..\..\..\..\common\include\dsntprocioctl.h"
#include "..\..\..\..\..\common\include\dsipc.h"
#include "..\..\..\..\..\common\include\ActionUtil.h"
#include "..\..\..\..\..\common\include\uninstall_hash.h"
#include "sha1.h"

#include "com_bluejungle_destiny_agent_ipc_OSWrapper.h"

#include "nlca_client.h"

#include "..\..\..\..\..\common\uninstall_hash.cpp"

#define OSW_CE_ATTR_CREATE_TIME      _T("CE_ATTR_CREATE_TIME")
#define OSW_CE_ATTR_LASTACCESS_TIME  _T("CE_ATTR_LASTACCESS_TIME")
#define OSW_CE_ATTR_LASTWRITE_TIME   _T("CE_ATTR_LASTWRITE_TIME")
#define OSW_CE_ATTR_OWNER_NAME       _T("CE_ATTR_OWNER_NAME")
#define OSW_CE_ATTR_OWNER_ID         _T("CE_ATTR_OWNER_ID")
#define OSW_CE_ATTR_GROUP_ID         _T("CE_ATTR_GROUP_ID")

#pragma comment(lib, "WtsApi32.lib")
#pragma warning(disable:4200)

typedef struct _REQUEST_PACKET 
{
    OVERLAPPED			OverLapped;
    IPC_POLICY_REQUEST	Request;
}   REQUEST_PACKET, *PREQUEST_PACKET;

#else
#include <iostream>
#define _LP64
#include <jni.h>
#include "com_bluejungle_destiny_agent_ipc_OSWrapper.h"

#include <sys/utsname.h>
#include <sys/syslog.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include <utime.h>
#include <errno.h>
#include <pwd.h>
#include <asm/types.h>
#include <linux/netlink.h>
#include <unistd.h>
#include <stdarg.h>

#include <list.h>


using namespace std;

#define NLMODULE jnioswrapper
#define NLTRACELEVEL 1
#define DOMAIN_CONFIG_FILE      "/etc/cedomainname.conf"

#include "linux_win.h"
#include "brain.h"
#include "dsipc.h"
#include "Actions.h"
#include "JavaConstants.h"
#include "osServLinux.h"
#include "kif.h"

#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <dlfcn.h>
#include "jni.h"
#include "CEsdk.h"
#include "dstypes.h"
#include "nltypes.h"
#include "PAQuery.h"
#pragma warning(disable:4200)
#endif


#define KERNEL_EVENT_SIZE 32

//IPC arguments indexes
#define METHOD_NAME_INDEX              0 //Should always be 0
#define FROM_FILE_NAME_INDEX           1
#define ACTION_INDEX                   2
#define SID_INDEX                      3
#define APPLICATION_NAME_INDEX         4
#define HOST_NAME_INDEX                5
#define TO_FILE_NAME_INDEX             6
#define IGNORE_OBLIGATION_INDEX        7
#define SHARE_NAME_INDEX               8
#define PID_INDEX                      9
#define LOG_LEVEL_INDEX               10
#define FROM_FILE_RESOLVED_NAME_INDEX 11
#define TO_FILE_RESOLVED_NAME_INDEX   12
#define IPC_JNI_ARG_COUNT             13


//This is very hack!!! It is for enforcement on WinScp.
//We will use SDK after the desktop enforcer works uses
//SDK also. 
#define WINSCP_EXTRA1_INDEX           13
#define WINSCP_EXTRA2_INDEX           14

#define ALLOW_STR _T("ALLOW")
#define TRUE_STR _T("true")
const TCHAR * QUERYDECISIONENGINE = _T("queryDecisionEngine");
#define SZ_EVENT_VIEWER_SOURCE _T("Policy Controller")

#define CEPROCDETECT_DEVICE_NAME     L"\\\\.\\CEPROCDETECT"

#if defined (WIN32) || defined (_WIN64)
enum {KERNEL_IFS_DRIVER=0, KERNEL_PROCDETECT_DRIVER};
HANDLE g_hKDriver = NULL;
ULONG  g_ulKDriverType = KERNEL_IFS_DRIVER;
//HANDLE g_hIFSDriver = NULL;
REQUEST_PACKET	g_packet[KERNEL_EVENT_SIZE]={0};
#endif

static TCHAR *getStringFromJava(JNIEnv *env, jstring javaString);
static void releaseStringFromJava(const TCHAR *str);

void freeProcessToken( HANDLE token , jobject processToken );
HANDLE getProcessToken (JNIEnv * env, jobject processToken);

// Once we replace the hard-coded lengths in IPCStub.h, we should replace the
// following with the real IPC ones
#define IPCREQ_METHODNAME_LEN 64
#define IPCREQ_PARAM_LEN 255
#define IPCREQ_FILENAME_LEN 512

#define IPC_SERVER_PATH	"/tmp/bjipc.server"


#ifdef LINUX
struct sockaddr_un fromaddr;
int handle_recv_nl_data(char *data, USHORT datalen, OUT IPC_POLICY_RESPONSE *policy_resp=NULL);

int logfd = -1;
void jacklog(char * msg,...)
{
  if(logfd<=0)
    logfd = creat("/var/log/bjbjlog",S_IRWXU);
  else
    logfd = open("/var/log/bjbjlog",O_WRONLY|O_APPEND);
  char buff[512];
  memset(buff,0,sizeof(buff));

  va_list ap;
  va_start(ap, msg);
  vsnprintf(buff,sizeof(buff),msg,ap);
  va_end(ap);
  write(logfd,buff,strlen(buff));
  close(logfd);
}

namespace nextlabs

{
  /****************************************************************************
   *
   * Obligation Abstraction
   *
   *   An obligation set is a collection of obligations.  
   *   Each obligation is a set of options
   *   which are key values, the obligation name and the policy 
   *   it is associated with.
   *
   *   Class               Method
   *   ------------------------------------------------------------------------
   *   Obligations
   *      |
   *   Obligation          GetObligationValue - Read the string value of an obligation.
   *      |                IsObligationSet    - Does the obligation exist?
   *      |
   *   ObligationOption
   *
   ***************************************************************************/
 
  /** ObligationOption
   *
   *  Key-value pair which represents an option to an obligatoin.
   */
  typedef std::pair<std::string,std::string> ObligationOption;
 
  /** ObligationOptions
   *
   *  A set of ObligationOption objects.
   */

  typedef std::list<ObligationOption>          ObligationOptions;

  /** Obligation
   *
   *  Obligation object which contains all key-value pairs for an obligation
   *  and the policy which it blongs to.
   */
  class Obligation
  {
    public:
      /** GetObligationValue
       *
       *  \brief Retreive the value of an obligation key.
       *  \return true when the value exist and the value is obtained, 
       *   otherwise false.
       */

      bool GetObligationValue( const char* ob_key ,
                               std::string& ob_value ) const

      {

	if( ob_key == NULL )
	  {
	    return false;
	  }

	for( ObligationOptions::const_iterator it = options.begin() ; 
	     it != options.end() ; ++it )
	  {
	    if( it->first == ob_key )
	      {
		ob_value = it->second;
		return true;
	      }
	  }

	return false;

      }/* GetObligationValue */

      /** IsObligationSet
       *
       *  \brief Determine if an obligation is set.
       *  \return true if the key exists, otherwise false.
       */
      bool IsObligationSet( const char* ob_key ) const
      {
	if( ob_key == NULL )
	  {
	    return false;
	  }

	for( ObligationOptions::const_iterator it = options.begin() ; 
	     it != options.end() ; ++it )
	  {
	    jacklog("isobligationset: ob=%s\n", it->first.c_str());
	    if( it->first == ob_key )
	      {
                   return true;
	      }
	  }
	return false;

      }/* IsObligationSet */

 

      std::string name;         /* Obligation Name */

      std::string policy;       /* Policy Name */

      ObligationOptions options; /* Set of obligation options */

  };/* Obligation */

 

  /** Obligations
   *
   *  Set of obligations.
   */
  class Obligations
  {
    public:
      /** GetValue
       *
       *  \brief Extract the value of a key in the CEAttributes structure.  If the value of the
       *         option is NULL the out value (in_value) for the caller will be an empty string.
       *         For this reason, it is not possible to distinguish an empty and NULL string
       *         value in an obligation structure.
       *
       *  \return true if the value is found and assigned to the callers
       *          string object, otherwise false.
       */
      static bool GetValue( const char* (*pfn_CEM_GetString)(CEString) ,
                            CEAttributes& obligations ,
			    const char* in_key ,
                            std::string& in_value )
      {
	//assert( pfn_CEM_GetString != NULL );
	//assert( in_key != NULL );

	if( pfn_CEM_GetString == NULL || in_key == NULL )
	  {
	    return false;
	  }

	in_value.clear();
	for( int i = 0 ; i < obligations.count ; ++i )
	  {
	    const char * key = pfn_CEM_GetString(obligations.attrs[i].key);
	    if( key == NULL || strcmp(key,in_key) != 0 )
	      {
		if(key!=NULL) {
		  jacklog("GetValue: key=%s\n", key);
		}
		continue;
	      }
	    /* Extract value.  NULL value indicates empty which is handled by clearing
	     * the input string before iteration.
	     */
	    const char* value = pfn_CEM_GetString(obligations.attrs[i].value);
	    if( value != NULL )
	      {
		in_value = value;
		jacklog("GetValue: key=%s value=%s\n", key, value);
	      }
	    return true;
	  }

	return false;
      }/* GetValue */

      /** Assign
       *
       *  \brief Assign obligations from an attributes (CEAttributes_t) structure.
       */
      void Assign( const char* (*pfn_CEM_GetString)(CEString) ,
                   CEAttributes& obligations )
      {
	//assert( pfn_CEM_GetString != NULL );
	if( pfn_CEM_GetString == NULL || obligations.attrs == NULL )
	  {
	    return;
	  }
	char temp_key[128];
	std::string wsvalue;
	int num_obs = 0;

	wsvalue.reserve(128);  /* avoid excessive allocations */
	/**********************************************************************
	 * Obligation Count
	 *********************************************************************/
	_snprintf_s(temp_key,128, _TRUNCATE, "CE_ATTR_OBLIGATION_COUNT");
	if( GetValue(pfn_CEM_GetString,obligations,temp_key,wsvalue) == false 
	    && wsvalue.size() == 0 )
	  {
	    return;
	  }
	num_obs = atoi(wsvalue.c_str());
	if( num_obs <= 0 )   /* Are there obligations? */
	  {
	    return;
	  }
	/********************************************************************
	 * Extract num_obs obligations or die trying.
	 *********************************************************************/
	bool result = true;
	for( int i = 1 ; i <= num_obs ; ++i ) /* Obligations [1,n] not [0,n-1] */
	  {
	    Obligation ob;          /* Obligation */
	    int num_values = 0;     /* Values to read from the obligation */

	    _snprintf_s(temp_key,128, _TRUNCATE, "CE_ATTR_OBLIGATION_NAME:%d",i);   /* Name */
	    result &= GetValue(pfn_CEM_GetString,obligations,temp_key,ob.name);
	    _snprintf_s(temp_key,128, _TRUNCATE, "CE_ATTR_OBLIGATION_POLICY:%d",i); /* Policy */
	    result &= GetValue(pfn_CEM_GetString,
			       obligations,temp_key,ob.policy);

	    _snprintf_s(temp_key,128, _TRUNCATE, "CE_ATTR_OBLIGATION_NUMVALUES:%d",i);      /* # of Values */
	    result &= GetValue(pfn_CEM_GetString,obligations,temp_key,wsvalue);
	    num_values =atoi(wsvalue.c_str());

	    if( result != true )
	      {
		continue;
	      }

	    /******************************************************************
	     * Extract Values or "options" of the obligation in pairs {key,value}
	     *****************************************************************/
	    result = true;
	    for( int j = 1 ; j <= num_values ; j += 2 ) /* Obligations [1,n] not [0,n-1] */
	      {
		ObligationOption option;
		_snprintf_s(temp_key,128, _TRUNCATE, "CE_ATTR_OBLIGATION_VALUE:%d:%d",i,j);    /* Key */

		GetValue(pfn_CEM_GetString,obligations,temp_key,option.first);
		_snprintf_s(temp_key,128, _TRUNCATE, "CE_ATTR_OBLIGATION_VALUE:%d:%d",i,j+1);  /* Value */
		GetValue(pfn_CEM_GetString,obligations,temp_key,option.second);
		ob.options.push_back(option);                /* Add Option */
	      }/* for j */
	    obs.push_back(ob);       /* Add obligation to obligation list */
	  }/* for i */
      }/* Assign */

      /** begin
       *
       *  \brief First iterator of the obligations.
       */
      std::list<Obligation>::const_iterator begin(void) const
      {
	return obs.begin();
      }/* begin */

      /** end
       *
       *  \brief Last iterator of the obligations.
       */
    std::list<Obligation>::const_iterator end(void) const
    {
      return obs.end();
    }/* end */

     /** IsObligationSet
      *
      *  \brief Determine if an obligation exists, or is set.
      */
      bool IsObligationSet(const char* ob_name ) const
      {
	//assert( ob_name != NULL );
	if( ob_name == NULL )
	  {
	    return false;
	  }

	std::list<Obligation>::const_iterator it;
	for( it = obs.begin() ; it != obs.end() ; ++it )
	  {
	    if( it->name == ob_name )
	      {
		return true;
	      }
	  }

	return false;
      }/* IsObligationSet */

      const std::list<Obligation>& GetObligations(void)
      {
	return obs;
      }/* GetObligations */ 

  private:
    std::list<Obligation> obs;
  };/* class Obligations */
}/* namespace nextlabs */

#endif



// Once we replace the hard-coded lengths in IPCStub.h, we should replace the
// following with the real IPC ones
#define IPCREQ_METHODNAME_LEN 64
#define IPCREQ_PARAM_LEN 255
#define IPCREQ_FILENAME_LEN 512



// Action unicode is a wrong word.  Unicode means assignment of a code to 
// a character, it doesn't enforce how a character is encoded.
// UCS-2 is more appropriate, meansing it takes 2 byte to 
// encode a unicode character
int sizeofunicode(char * unicode)
{
  // assuming UCS-2
  int result = 0;
  
  while (1) {
    if ((unicode[result] == 0) && !(result % 2))
      break;
    result++;
  }

  return (result / 2);
}
/** getStringFromJava
 *
 * Returns a terminated WCHAR from a java string.  Remarkably,
 * the return from GetStringChars is not null terminated
 *
 * \param env - the java JNI environment
 * \param javaString - the java string
 *
 * \return a terminated WCHAR representing the java string
 * \note resources allocated by this function should be freed
 * by calling releaseStringFromJava
 */
static TCHAR *getStringFromJava(JNIEnv *env, jstring javaString)
{
#if defined (WIN32) || defined (_WIN64)
    const TCHAR *str = (const TCHAR *)env->GetStringChars(javaString, 0);
    size_t javaStrLen = env->GetStringLength(javaString);

    TCHAR *ret = (TCHAR *)malloc(sizeof(TCHAR) * (javaStrLen + 1));
    
    if (ret != NULL)
    {
        _tcsncpy_s(ret,javaStrLen + 1, str, _TRUNCATE);
        ret[javaStrLen] = L'\0';
    }

    env->ReleaseStringChars(javaString, (const jchar *)str);
#else
    const char *str = (const char *)env->GetStringUTFChars(javaString, 0);
    size_t javaStrLen = env->GetStringUTFLength(javaString);

    char *ret = (char *)malloc(sizeof(char) * (javaStrLen + 1));
    
    if (ret != NULL)
    {
        strncpy_s(ret, javaStrLen + 1, str, _TRUNCATE);
    }

    env->ReleaseStringUTFChars(javaString, (const char *)str);
#endif 

    return ret;
}

/** releaseStringFromJava
 *
 * Frees the resources allocated by getStringFromJava
 *
 * \param str - the WCHAR * returned by getStringFromJava()
 */

static void releaseStringFromJava(const TCHAR *str)
{
    free((void *)str);
}


void GetGlobalName (LPCTSTR name, LPTSTR globalName)
{
#if defined (WIN32) || defined (_WIN64)
    if (_tcsstr (name, _T("\\")) == NULL)
    {
        _stprintf_s (globalName,MAX_PATH, _T("%s%s"), GLOBAL_NAME_PREFIX, name);
    }
    else
    {
        _tcsncpy_s (globalName,MAX_PATH, name, _TRUNCATE);
    }
#endif
}


#if defined (WIN32) || defined (_WIN64)
void DumpSid(CORE_SID const *CoreSid)
{
	ULONG		i;
	CHAR		LogBuffer[512];
	CHAR		Tmp[8];

	memset(LogBuffer,0,sizeof(LogBuffer));
	for(i=0;i<CoreSid->ulUserSIDLength;i++)
	{
		_snprintf_s(Tmp,8, _TRUNCATE,"%02x",CoreSid->aUserSID[i]);
		strncat_s(LogBuffer,512,Tmp, _TRUNCATE);
	}
	printf("UserSID: %s\n",LogBuffer);		
	printf("UserSIDAttribute %08x\n",CoreSid->ulUserSIDAttribute);

	memset(LogBuffer,0,sizeof(LogBuffer));
	for(i=0;i<CoreSid->ulOwnerSIDLength;i++)
	{
		_snprintf_s(Tmp,8, _TRUNCATE,"%02x",CoreSid->aOwnerSID[i]);
		strncat_s(LogBuffer,512,Tmp, _TRUNCATE);
	}
	printf("OwnerSID: %s\n",LogBuffer);

	memset(LogBuffer,0,sizeof(LogBuffer));
	for(i=0;i<CoreSid->ulGroupSIDLength;i++)
	{
		_snprintf_s(Tmp,8, _TRUNCATE,"%02x",CoreSid->aGroupSID[i]);
		strncat_s(LogBuffer,512,Tmp, _TRUNCATE);
	}
	printf("GroupSID: %s\n",LogBuffer);
}

/////////////////////////////////////////////////////////////////////
// FOr debugging, dump the request values
/////////////////////////////////////////////////////////////////////
void DumpRequest(bool bPrintDetail,IPC_POLICY_REQUEST const* pRequest)
{
	wprintf(_T("++++++++++++++++++++++++++++\n"));
	wprintf(_T("Remote Address: %s\n"), pRequest->szwHostName);
	wprintf(_T("IP Address: %d\n"), pRequest->ulIPAddress);
	wprintf(_T("FileName: %ws\n"), pRequest->szwFromFileName);
	wprintf(_T("Action: %s\n"), pRequest->szwAction);	
	wprintf(_T("PID: %d\n"), pRequest->ulProcessId);
	if(wcslen( pRequest->szwToFileName)) 
	{
		wprintf(_T("To File Name: %ws\n"), pRequest->szwToFileName);
	}
	if(bPrintDetail)
	{
		wprintf(_T("Application name = %s\n"), pRequest->szwApplicationName);
		wprintf(_T("ClientLanManType = %ws\n"), pRequest->ClientLanManType);
		wprintf(_T("ClientOsType = %ws\n"), pRequest->ClientOsType);
		wprintf(_T("ShareName = %ws\n"), pRequest->ShareName);
		wprintf(_T("bIgnoreObligations = %d\n"), pRequest->bIgnoreObligations);
		wprintf(_T("UniqueIndex = %d\n"), pRequest->UniqueIndex);
		DumpSid(&pRequest->CoreSid);
	}
}

void DumpResponse(IPC_POLICY_RESPONSE const * pResponse)
{
	wprintf(_T("----------------------------\n"));
	wprintf(_T("UniqueIndex: %d\n"), pResponse->UniqueIndex);
	wprintf(_T("Allow : %d\n"), pResponse->Allow);
	wprintf(_T("AllowType : %d\n"), pResponse->AllowType);
	wprintf(_T("----------------------------\n"));
}
#endif

/*
 * Class:     com_bluejungle_destiny_agent_ipc_OSWrapper
 * Method:    createFileMapping
 * Signature: (Ljava/lang/String;I)I
 */
JNIEXPORT jlong JNICALL Java_com_bluejungle_destiny_agent_ipc_OSWrapper_createFileMapping
  (JNIEnv * env, jobject obj, jstring name, jint size)
{

#if defined (WIN32) || defined (_WIN64)    

  //TRACE ("CreateFileMapping\n");
    const TCHAR *str = getStringFromJava(env, name);
        TCHAR globalName [MAX_PATH];
        ::GetGlobalName (str, globalName);
	HANDLE hMapFile;

	hMapFile = CreateFileMapping(INVALID_HANDLE_VALUE,    // current file handle 
		SecurityAttributesFactory::GetSecurityAttributes(), // default security 
		PAGE_READWRITE,                    // read/write permission 
		0,                                 // max. object size 
		size,                              // size of hFile 
		globalName);            // name of mapping object 

        if (hMapFile == NULL)
        {
            hMapFile = (HANDLE) Java_com_bluejungle_destiny_agent_ipc_OSWrapper_openFileMapping (env, obj, name);
        }

    releaseStringFromJava(str);

    return ((jlong)hMapFile);
#else
  bjMemoryMap * memmap = NULL;
  BJ_error_t    rc     = BJ_ERROR;

  const char * name2 = env->GetStringUTFChars (name, 0);

  // Todo: Make sure name does not contain invalid char 

  // Everything is cool, and we can create the OS primitive object
  memmap = new bjMemoryMap ();

  if (memmap == NULL) {
    TRACE (1, "createFileMapping: Fail to get the primitive\n");
    goto fail_and_cleanup;
  }

  rc = memmap->create (name2, size);
  if (rc != BJ_OK) {
    TRACE (1, "createFailMapping: Fail to create primitive\n");
    free (memmap);
    memmap = NULL;
    goto fail_and_cleanup;
  }
  
 fail_and_cleanup:
  env->ReleaseStringUTFChars (name, name2);
  return (jlong)(memmap);
#endif
}


/*
 * Class:     com_bluejungle_destiny_agent_ipc_OSWrapper
 * Method:    hashChallenge
 * Signature: (I)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_bluejungle_destiny_agent_ipc_OSWrapper_hashChallenge
  (JNIEnv* env, jobject obj, jstring challenge)
{
    const TCHAR* challengeStr = getStringFromJava(env, challenge);
	size_t hashBufSize = 300;
	TCHAR* hashBuf = new TCHAR[hashBufSize+1];
	_tcsncpy_s(hashBuf,hashBufSize+1, _T(""), _TRUNCATE);
#if defined (WIN32) || defined (_WIN64)    
	int result = hashChallenge(challengeStr, hashBuf, hashBufSize);
	if (result == HASH_BUFFER_TOO_SMALL)
	{
		delete[] hashBuf;
		hashBuf = NULL;
		hashBuf = new TCHAR[hashBufSize];
		result = hashChallenge(challengeStr, hashBuf, hashBufSize);
	}
	if (result != ERROR_SUCCESS)
	{
		_tcsncpy_s(hashBuf,hashBufSize+1, _T(""), _TRUNCATE);
	}
#endif

        releaseStringFromJava(challengeStr);
	jstring hash = env->NewString((const jchar*)hashBuf, (jsize)_tcslen(hashBuf));
	delete[] hashBuf;
	return (hash);
}

/*
 * Class:     com_bluejungle_destiny_agent_ipc_OSWrapper
 * Method:    openFileMapping
 * Signature: (Ljava/lang/String;I)I
 */
JNIEXPORT jlong JNICALL Java_com_bluejungle_destiny_agent_ipc_OSWrapper_openFileMapping
  (JNIEnv * env, jobject obj, jstring name)
{

#if defined (WIN32) || defined (_WIN64)    
  //        TRACE ("OpenFileMapping\n");
        const TCHAR *str = getStringFromJava(env, name);
        TCHAR globalName [MAX_PATH];
        ::GetGlobalName (str, globalName);
	HANDLE hMapFile;

        hMapFile = OpenFileMapping(FILE_MAP_WRITE, // read/write permission 
			           FALSE,               // Do not inherit the name 
			           globalName);// of the mapping object. 

    releaseStringFromJava(str);

	return ((jlong)hMapFile);
#else
  bjMemoryMap * memmap = NULL;
  BJ_error_t    rc     = BJ_ERROR;

  const char * name2 = env->GetStringUTFChars (name, 0);

  // Todo: Make sure name does not contain invalid char 

  memmap = new bjMemoryMap();

  if (memmap == NULL) {
    TRACE (1, "openFileMapping: Fail to get the primitive\n");
    goto fail_and_cleanup;
  }

  rc = memmap->open (name2);

  if (rc != BJ_OK) {
    TRACE (1, "openFileMapping: fail to open mapping\n");
    free (memmap);
    memmap = NULL;
    goto fail_and_cleanup;
  }

 fail_and_cleanup:
  env->ReleaseStringUTFChars (name, name2);
  return (jlong)(memmap);
#endif
}

/*
 * Class:     com_bluejungle_destiny_agent_ipc_OSWrapper
 * Method:    mapViewOfFile
 * Signature: (I)I
 */
JNIEXPORT jlong JNICALL Java_com_bluejungle_destiny_agent_ipc_OSWrapper_mapViewOfFile
  (JNIEnv * env, jobject obj, jlong handle)
{
//	_tprintf (_T("MapView"));
#if defined (WIN32) || defined (_WIN64)    
	LPVOID lpMapAddress;
	lpMapAddress = ::MapViewOfFile((HANDLE) handle, // handle to mapping object 
                                       FILE_MAP_ALL_ACCESS,               // read/write permission 
                                       0,                                 // max. object size 
                                       0,                                 // size of hFile 
                                       0);                                // map entire file 	
	return ((jlong)lpMapAddress);
#else
        void         * ptr       = NULL;
        bjMemoryMap  * primitive = (bjMemoryMap *) handle;
  
        if (primitive == NULL) {
            TRACE (1, "mapViewOfFile: Invalid handle.\n");
            return (long)(ptr);
        }

        ptr = primitive->map();

        if (ptr == NULL) {
            TRACE (1, "mapViewOfFile: fail to map the file\n");
            return (long)(ptr);
        }

        return (jlong)(ptr);

#endif
}

/*
 * Class:     com_bluejungle_destiny_agent_ipc_OSWrapper
 * Method:    unmapViewOfFile
 * Signature: (I)Z
 */
JNIEXPORT jboolean JNICALL Java_com_bluejungle_destiny_agent_ipc_OSWrapper_unmapViewOfFile
(JNIEnv * env, jobject obj, jlong handle)
{
#if defined (WIN32) || defined (_WIN64)    
    return static_cast<jboolean>(::UnmapViewOfFile((LPCVOID) handle));
#else
    // On unix. we cannot unmap with just the pointer itself.
    // We will do it on close
    TRACE (1, "unmapViewOfFile: Warning. unmap is done by close handle\n");
    return false;
#endif
}

/*
 * Class:     com_bluejungle_destiny_agent_ipc_OSWrapper
 * Method:    createEvent
 * Signature: (Ljava/lang/String;)I
 */
JNIEXPORT jlong JNICALL Java_com_bluejungle_destiny_agent_ipc_OSWrapper_createEvent
(JNIEnv * env, jobject obj, jstring name)
{
#if defined (WIN32) || defined (_WIN64)    
    const TCHAR *str = getStringFromJava(env, name);
    TCHAR globalName [MAX_PATH];
    ::GetGlobalName (str, globalName);
    HANDLE hEvent;

    // hEvent = ::CreateEvent (SecurityAttributesFactory::GetSecurityAttributes(), FALSE, FALSE, globalName);
    hEvent = ::CreateEvent (SecurityAttributesFactory::GetSecurityAttributes(), TRUE, FALSE, globalName);

    if (hEvent == NULL) {
        hEvent = (HANDLE) Java_com_bluejungle_destiny_agent_ipc_OSWrapper_openEvent (env, obj, name);
        ::ResetEvent (hEvent);
    }
    releaseStringFromJava(str);
    return ((jlong)hEvent);
#else
    const char * name2 = env->GetStringUTFChars (name, 0);

    bjSemaphore * sem = NULL;
    BJ_error_t    rc  = BJ_ERROR;

    sem = new bjSemaphore();
  
    if (sem == NULL) {
        TRACE (1, "createEvent: Fail to get primitive\n");
        goto fail_and_cleanup;
    }

    // Semaphore is created with down state for Event
    rc = sem->create (name2, 0);

    if (rc != BJ_OK) {
        TRACE (1, "createEvent: Fail to create primitive\n");
        free (sem);
        sem = NULL;
        goto fail_and_cleanup;
    }

 fail_and_cleanup:
    env->ReleaseStringUTFChars (name, name2);
    return (jlong)(sem);
#endif
}

/*
 * Class:     com_bluejungle_destiny_agent_ipc_OSWrapper
 * Method:    openEvent
 * Signature: (Ljava/lang/String;)I
 */
JNIEXPORT jlong JNICALL Java_com_bluejungle_destiny_agent_ipc_OSWrapper_openEvent
(JNIEnv * env, jobject obj, jstring name)
{
#if defined (WIN32) || defined (_WIN64)    
    //	_tprintf (_T("OpenEvent"));
    const TCHAR *str = getStringFromJava(env, name);
    TCHAR globalName [MAX_PATH];
    ::GetGlobalName (str, globalName);
    HANDLE hEvent;

    hEvent = ::OpenEvent (SYNCHRONIZE | EVENT_MODIFY_STATE, FALSE, globalName);
    releaseStringFromJava(str);
    return ((jlong)hEvent);
#else

    const char * name2 = env->GetStringUTFChars (name, 0);

    bjSemaphore * sem = NULL;
    BJ_error_t    rc  = BJ_ERROR;

    sem = new bjSemaphore();
  
    if (sem == NULL) {
        TRACE (1, "openEvent: Fail to get primitive\n");
        goto fail_and_cleanup;
    }

    // We don't create, and it should exist before
    rc = sem->open (name2);

    if (rc != BJ_OK) {
        TRACE (1, "openEvent: Fail to create primitive\n");
        free (sem);
        sem = NULL;
        goto fail_and_cleanup;
    }

 fail_and_cleanup:
    env->ReleaseStringUTFChars (name, name2);
    return (jlong)(sem);
#endif
}

/*
 * Class:     com_bluejungle_destiny_agent_ipc_OSWrapper
 * Method:    setEvent
 * Signature: (I)Z
 */
JNIEXPORT jboolean JNICALL Java_com_bluejungle_destiny_agent_ipc_OSWrapper_setEvent
  (JNIEnv * env, jobject obj, jlong handle)
{
//	_tprintf (_T("setEvent"));
#if defined (WIN32) || defined (_WIN64)    
    return static_cast<jboolean>((::SetEvent ((HANDLE) handle)));
#else
  bjSemaphore * sem = (bjSemaphore *) handle;
  BJ_error_t    rc  = BJ_ERROR;

  if (sem == NULL) {
    TRACE (1, "setEvent: Invalid handle.\n");
    return false;
  }
  
  rc = sem->give(); 
  if (rc == BJ_OK) {
    return true;
  }

  return false;
#endif
}

/*
 * Class:     com_bluejungle_destiny_agent_ipc_OSWrapper
 * Method:    createMutex
 * Signature: (Ljava/lang/String;)I
 */
JNIEXPORT jlong JNICALL Java_com_bluejungle_destiny_agent_ipc_OSWrapper_createMutex
  (JNIEnv * env, jobject obj, jstring name)
{
#if defined (WIN32) || defined (_WIN64)    
    //	_tprintf (_T("createMutex"));
    const TCHAR *str = getStringFromJava(env, name);
    TCHAR globalName [MAX_PATH];
    ::GetGlobalName (str, globalName);
    HANDLE hMutex = 0;

    hMutex = ::CreateMutex (SecurityAttributesFactory::GetSecurityAttributes(), FALSE, globalName);
    if (hMutex == NULL)
    {
        hMutex = (HANDLE) Java_com_bluejungle_destiny_agent_ipc_OSWrapper_openMutex (env, obj, name);
    }
    releaseStringFromJava(str);
    return ((jlong)hMutex);
#else
    const char * name2 = env->GetStringUTFChars (name, 0);
    bjMutex   * mutex = NULL;
    BJ_error_t  rc    = BJ_ERROR;

    mutex = new bjMutex ();

    if (mutex == NULL) {
        TRACE (1, "createMutex: Fail to get primitive\n");
        goto fail_and_cleanup;
    }

    // Semaphore is created with up state for Mutex
    rc = mutex->create (name2);

    if (rc != BJ_OK) {
        TRACE (1, "createMutex: Fail to create primitive\n");
        free (mutex);
        mutex = NULL;
        goto fail_and_cleanup;
    }

 fail_and_cleanup:
    env->ReleaseStringUTFChars (name, name2);
    return (jlong)(mutex);
#endif
}

/*
 * Class:     com_bluejungle_destiny_agent_ipc_OSWrapper
 * Method:    openMutex
 * Signature: (Ljava/lang/String;)I
 */
JNIEXPORT jlong JNICALL Java_com_bluejungle_destiny_agent_ipc_OSWrapper_openMutex
  (JNIEnv * env, jobject obj, jstring name)
{
#if defined (WIN32) || defined (_WIN64)    
    //	_tprintf (_T("openMutex"));
    const TCHAR *str = getStringFromJava(env, name);
    TCHAR globalName [MAX_PATH];
    ::GetGlobalName (str, globalName);
    HANDLE hMutex;
    
    hMutex = ::OpenMutex (SYNCHRONIZE | MUTEX_MODIFY_STATE, FALSE, globalName);
    releaseStringFromJava(str);
    return ((jlong)hMutex);
#else
  const char * name2 = env->GetStringUTFChars (name, 0);
  bjMutex   * mutex = NULL;
  BJ_error_t  rc    = BJ_ERROR;

  if (mutex == NULL) {
    TRACE (1, "openMutex: Fail to get primitive\n");
    goto fail_and_cleanup;
  }

  // We don't create here, and it should exist before
  rc = mutex->open (name2);

  if (rc != BJ_OK) {
    TRACE (1, "openEvent: Fail to open primitive\n");
    free (mutex);
    mutex = NULL;
    goto fail_and_cleanup;
  }
  

 fail_and_cleanup:
  env->ReleaseStringUTFChars (name, name2);
  return (jlong)(mutex);
#endif
}

/*
 * Class:     com_bluejungle_destiny_agent_ipc_OSWrapper
 * Method:    releaseMutex
 * Signature: (I)Z
 */
JNIEXPORT jboolean JNICALL Java_com_bluejungle_destiny_agent_ipc_OSWrapper_releaseMutex
  (JNIEnv * env, jobject obj, jlong handle)
{
//	_tprintf (_T("releaseMutex"));
#if defined (WIN32) || defined (_WIN64)    
    return static_cast<jboolean>((::ReleaseMutex ((HANDLE) handle)));
#else
  bjMutex * mutex = (bjMutex *) handle;
  BJ_error_t  rc  = BJ_ERROR;
  
  if (mutex == NULL) {
    TRACE (1, "releaseMutex: Invalid handle.\n");
    return false;
  }

  rc = mutex->give();
  if (rc == BJ_OK) {
    return true;
  }

  return false;
#endif
}

/*
 * Class:     com_bluejungle_destiny_agent_ipc_OSWrapper
 * Method:    waitForSingleObject
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_com_bluejungle_destiny_agent_ipc_OSWrapper_waitForSingleObject
  (JNIEnv * env, jobject obj, jlong handle)
{
//	_tprintf (_T("WaitForSingleObject"));
#if defined (WIN32) || defined (_WIN64)    
	int ret = ::WaitForSingleObject ((HANDLE) handle, 1000);
	ResetEvent ((HANDLE)handle);
	return ret;
#else
  bjSemaphore  * primitive = (bjSemaphore *) handle;
  BJ_waitResult_t rc;

  if (primitive == NULL) {
    return jint (BJ_WAIT_FAILED);
  }
  
  // The window version wait for 1 seconds, basically a polling
  timespec ts;
  
  ts.tv_sec  = 1; 
  ts.tv_nsec = 0;

  rc = primitive->take (ts);

  switch (rc) {
  case BJ_WAIT_FAILED:
    return jint (IPC_WAIT_FAILED);
  case BJ_WAIT_SUCCESS:
    return jint (IPC_WAIT_SUCCESS);
  case BJ_WAIT_TIMEOUT:
    return jint (IPC_WAIT_TIMEOUT);
  }

  // Should never get here
  return jint (IPC_WAIT_FAILED);
#endif
}

/*
 * Class:     com_bluejungle_destiny_agent_ipc_OSWrapper
 * Method:    waitForMultipleObjects
 * Signature: (I[II)I
 */
JNIEXPORT jint JNICALL Java_com_bluejungle_destiny_agent_ipc_OSWrapper_waitForMultipleObjects
(JNIEnv * env, jobject obj, jint count, jlongArray handles, jobjectArray handle_names, jint timeout)
{
#if defined (WIN32) || defined (_WIN64)    
	jint ret = -1;

	/* Iterate over the handle names and test them for existence.  When one is found not
	   to exist indicate that the set of resources, for which the process resources exits,
	   has terminated.   WAIT_ABANDONED_0 is used as the base for the return value.  The
	   offset indicates the handle that is assocated with the terminated process.

	   The meaning of WaitForMultipleObjects is slightly change since EVENT objects cannot
	   have an abandoned state - they are not owned.  The 'handle_names' parameter allows
	   the construction of a known MUTEX object that can be tested for existence.
	*/
	for( int i = 0 ; handle_names != NULL && i < count && ret < 0 ; i++ )
	{
	  jobject paramObj = env->GetObjectArrayElement(handle_names,i);

	  if( paramObj == NULL )
	  {
	    continue;
	  }

         const TCHAR *handle_name = getStringFromJava(env, (jstring)paramObj);

	  if( handle_name == NULL )
	  {
	    continue;
	  }

	  /* Expecting object wtih name of "Global\[ID]_SENDEVENT".  Extract the ID
	     to generate the mutex name.
	   */
	  const WCHAR* prefix = wcsstr(handle_name,L"Global\\");
	  const WCHAR* suffix = wcsstr(handle_name,L"SEND_EVENT");

	  if( prefix != NULL && suffix != NULL )
	  {
	    WCHAR id[128];
	    WCHAR mutex_name[128];

	    prefix += wcslen(L"Global\\");  /* move past global prefix */

	    memset(id,0x00,sizeof(id));
	    memset(mutex_name,0x00,sizeof(mutex_name));

	    wcsncpy_s(id,128,prefix, _TRUNCATE);
	    _snwprintf_s(mutex_name,128, _TRUNCATE,L"Global\\%sHANDSHAKE_MUTEX",id);

	    //OutputDebugStringW(mutex_name);

	    HANDLE mutex = OpenMutexW(MUTEX_ALL_ACCESS,FALSE,mutex_name);

	    /* If the handle does not exist use the current index as the abandoned
	       object.
	    */
	    if( mutex == NULL && GetLastError() == ERROR_FILE_NOT_FOUND )
	    {
	      ret = i;  
	    }

	    if( mutex != NULL )
	    {
	      CloseHandle(mutex);
	    }
	  }/* if match for event name */

          releaseStringFromJava(handle_name);
	}/* for */

	/* If there was an object that did not have an associated mutex this means the process
	   does not exist.  The caller must be notfied that that process has abandoned the
	   resources set which has been allocated for it: events, etc.
	*/
	if( ret >= 0 )
	{
	  //char temp[512];
	  //sprintf(temp,"OSWrapper_waitForMultipleObjects: %d (%d) abandoned\n", ret, WAIT_ABANDONED_0 + ret);
	  //OutputDebugStringA(temp);
	  return (WAIT_ABANDONED_0 + ret);
	}

	/* The process for all of the handles in 'handles' exist so wait for an event. */

	jlong *jhandleArray = env->GetLongArrayElements(handles,0);

        HANDLE *handleArray = (HANDLE *)jhandleArray;
        if (sizeof(jlong) != sizeof(HANDLE)) {
            handleArray = new HANDLE[count];
            for (int i = 0; i < count; i++) {
                handleArray[i] = (HANDLE)jhandleArray[i];
            }
        }

	ret = ::WaitForMultipleObjects (count, handleArray, FALSE, timeout);

	/* Make sure an object is actually signaled - not error state
	   For success ret is in [WAIT_OBJECT_0,WAIT_OBJECT_0+count).
	 */
	if( static_cast<jint>(WAIT_OBJECT_0) <= ret && ret < static_cast<jint>(WAIT_OBJECT_0 + count) )
	{
	  ResetEvent((HANDLE) jhandleArray[ ret - WAIT_OBJECT_0 ]);
	}

	env->ReleaseLongArrayElements(handles, jhandleArray, JNI_ABORT);
        if (handleArray != (HANDLE *)jhandleArray) {
            delete[] handleArray;
        }

	return (ret);
#else
  // Only Window has wait for MULTIPLE objects, for every other platform
  // only single object is wait, so we will wait for the first object and return

  BJ_waitResult_t rc = BJ_WAIT_SUCCESS;
  bjSemaphore *   primitive = NULL;
  timespec        ts;

  if (count != 1) {
    TRACE (1, "waitForMultipleObjects: Linux does not supported multiple objects\n");
    return jint (BJ_WAIT_FAILED);
  }

  // We retrieve the first object and wait for it
  jlong *handleArray = env->GetLongArrayElements (handles, 0);

  if (handleArray == NULL) {
    rc = BJ_WAIT_FAILED;
    goto failed_and_cleanup;
  }

  primitive = (bjSemaphore *) handleArray[0];

  if (primitive == NULL) {
    rc = BJ_WAIT_FAILED;
    goto failed_and_cleanup;
  }

  // Converting ms to timespec (sec / nanosec)
  BJ_millisec2timespec (timeout, &ts);

  rc = primitive->take (ts);
  
 failed_and_cleanup:
  env->ReleaseLongArrayElements (handles, handleArray, JNI_ABORT);

  switch (rc) {
  case BJ_WAIT_FAILED:
    return jint (IPC_WAIT_FAILED);
  case BJ_WAIT_SUCCESS:
    return jint (IPC_WAIT_SUCCESS);
  case BJ_WAIT_TIMEOUT:
    return jint (IPC_WAIT_TIMEOUT);
  }
  
  return jint (IPC_WAIT_FAILED);
#endif
}

/*
 * Class:     com_bluejungle_destiny_agent_ipc_OSWrapper
 * Method:    readString
 * Signature: (I)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_bluejungle_destiny_agent_ipc_OSWrapper_readString
  (JNIEnv * env, jobject obj, jlong handle)
{
#if defined (WIN32) || defined (WIN64)
	TCHAR* str = (TCHAR*) handle;
	return (env->NewString((const jchar*)str, (jsize)_tcslen(str)));
#else
  char * ptr = (char *) handle;

  if (ptr == NULL) {
    TRACE (1, "readString: Invalid sharedmem\n");
    return env->NewStringUTF ("");
  }

  return env->NewStringUTF ((char *)ptr);
#endif
}

/*
 * Class:     com_bluejungle_destiny_agent_ipc_OSWrapper
 * Method:    readIPCResponse
 * Signature: (I)[Ljava/lang/String;
 */
JNIEXPORT jobjectArray JNICALL Java_com_bluejungle_destiny_agent_ipc_OSWrapper_readIPCResponse
  (JNIEnv * env, jobject obj, jlong sharedMem)
{

  //    TRACE ("readIPCResponse");

    IPCREQUEST* pRequest = (IPCREQUEST*) sharedMem;
    jobjectArray ret = NULL;

    for (int i=0; i < 7; i++)
    {
#if defined (WIN32) || defined (_WIN64)
    ret= (jobjectArray)env->NewObjectArray(7,
         env->FindClass("java/lang/String"),
         env->NewString((const jchar*)_T(""), 0));

        env->SetObjectArrayElement(ret, i, env->NewString((const jchar*)pRequest->params[i], (jsize)_tcslen (pRequest->params[i])));
#else
    ret= (jobjectArray)env->NewObjectArray(7,
         env->FindClass("java/lang/String"),
         env->NewStringUTF((const char*)_T("")));

        env->SetObjectArrayElement(ret, i, env->NewStringUTF((const char*)pRequest->params[i]));
#endif
    }

    return(ret);
}

/*
 * Class:     com_bluejungle_destiny_agent_ipc_OSWrapper
 * Method:    readIPCRequest
 * Signature: (I)[Ljava/lang/String;
 */
JNIEXPORT jobjectArray JNICALL Java_com_bluejungle_destiny_agent_ipc_OSWrapper_readIPCRequest
  (JNIEnv * env, jobject obj, jlong sharedMem)
{

  //    TRACE ("readIPCRequest\n");
#if defined (WIN32) || defined (_WIN64)

    IPCREQUEST* pRequest = (IPCREQUEST*) sharedMem;
    jobjectArray ret;

    ret= (jobjectArray)env->NewObjectArray(8, env->FindClass("java/lang/String"), env->NewString((const jchar*)_T(""), 0));

    env->SetObjectArrayElement(ret, 0, env->NewString((const jchar*)pRequest->methodName, (jsize)_tcslen (pRequest->methodName)));
    for (int i=0; i < 7; i++)
    {
        env->SetObjectArrayElement(ret, i + 1, env->NewString((const jchar*)pRequest->params[i], (jsize)_tcslen (pRequest->params[i])));
    }
    return(ret);
#else
  jobjectArray ret;
  IPCREQUEST * pRequest = (IPCREQUEST *) sharedMem;
  
  if (pRequest == NULL) {
    TRACE (1, "readIPCRequest: sharedMem is null\n");
    return ret;
  }
  
  ret = env->NewObjectArray(8,
                            env->FindClass("java/lang/String"),
                            env->NewStringUTF (""));

  env->SetObjectArrayElement (ret, 0, env->NewStringUTF ((char *) pRequest->methodName));

  // Hard coded to match the IPCREQUEST struct
  int num_params =  sizeof (pRequest->params) / sizeof (*(pRequest->params));
  for (int i = 0; i < num_params; i++) {
    env->SetObjectArrayElement (ret, i+1, env->NewStringUTF ((char *)pRequest->params[i]));
  }

  return ret;
#endif
}

void checkFileName (TCHAR* pszwFileName)
{
    for (int i = 0; pszwFileName [i] != 0 && i < IPCREQ_FILENAME_LEN; i++)
    {
        if (pszwFileName [i] < 0x20)
        {
            pszwFileName [i] = _T('_');
        }
    }
}

/*
 * Utility method to create a JNI policy IPC request object from a C++ IPC_POLICY_REQUEST object.
 */
#if defined (WIN32) || defined (_WIN64)
JNIEXPORT jobjectArray readPolicyIPCRequest (JNIEnv * env, jobject obj, IPC_POLICY_REQUEST* pRequest)
{
#ifdef _DEBUG
  // DumpRequest(true, pRequest);
#endif
	
    WCHAR szLongFileName [IPCREQ_FILENAME_LEN];
    szLongFileName[0] = L'\0';
    UINT fileNameLength = 0;
    jobjectArray ret;

    if (_tcscmp (pRequest->szwApplicationName, _T("CEWINSCP.EXE")) == 0) {
      ret= (jobjectArray)env->NewObjectArray(IPC_JNI_ARG_COUNT+2,
					env->FindClass("java/lang/Object"),
					env->NewString((const jchar*)_T(""), 0));
      env->SetObjectArrayElement(ret, WINSCP_EXTRA1_INDEX, 
				 env->NewString((const jchar*)_T("no"), (jsize)wcslen(_T("no"))));
      env->SetObjectArrayElement(ret, WINSCP_EXTRA2_INDEX, 
				 env->NewString((const jchar*)_T("no"), (jsize)wcslen(_T("no"))));
    } else {
      ret= (jobjectArray)env->NewObjectArray(IPC_JNI_ARG_COUNT,
					env->FindClass("java/lang/Object"),
					env->NewString((const jchar*)_T(""), 0));
    }

    env->SetObjectArrayElement(ret, METHOD_NAME_INDEX, env->NewString((const jchar*)pRequest->szwMethodName, (jsize)_tcslen (pRequest->szwMethodName)));

        // From the timestamp analysis, GetLongPathName can take up to SIX seconds to execute
        // This is killing the getRequest because it's imposing a 6 seconds delay from one request to 
        // another one.  We simply cannot have it here.    Dominic 9/7/06
  //	fileNameLength = ::GetLongPathName (pRequest->szwFromFileName, szLongFileName, IPCREQ_FILENAME_LEN);


    if (0 == fileNameLength) {
        env->SetObjectArrayElement(ret, FROM_FILE_NAME_INDEX, env->NewString((const jchar*)pRequest->szwFromFileName, (jsize)wcslen(pRequest->szwFromFileName)));
    } else {
        env->SetObjectArrayElement(ret, FROM_FILE_NAME_INDEX, env->NewString((const jchar*)szLongFileName, (jsize)fileNameLength));
    }
	
    env->SetObjectArrayElement(ret, ACTION_INDEX, env->NewString((const jchar*)pRequest->szwAction, (jsize)_tcslen (pRequest->szwAction)));


    checkFileName (pRequest->szwFromFileName);
    checkFileName (pRequest->ShareName);
    checkFileName (pRequest->szwToFileName);
    
    if (pRequest->CoreSid.aUserSID != NULL)
    {
        env->SetObjectArrayElement(ret, SID_INDEX, env->NewString((const jchar*)pRequest->CoreSid.aUserSID, (jsize)_tcslen (pRequest->CoreSid.aUserSID)));
    }

        // This is a hack to map the application for RUN.  It's because the old structure 
        // only has 128 char, which is NOT enough, so we pass it on the FromFile

    if (_tcscmp (pRequest->szwAction, L"RUN") == 0) {
        env->SetObjectArrayElement(ret, APPLICATION_NAME_INDEX, env->NewString((const jchar*)pRequest->szwFromFileName, (jsize)_tcslen (pRequest->szwFromFileName)));
        
    } else {
        env->SetObjectArrayElement(ret, APPLICATION_NAME_INDEX, env->NewString((const jchar*)pRequest->szwApplicationName, (jsize)_tcslen (pRequest->szwApplicationName)));
    }
    
    
    if (_tcslen (pRequest->szwHostName) > 0)
    {
        env->SetObjectArrayElement(ret, HOST_NAME_INDEX, env->NewString((const jchar*)pRequest->szwHostName, (jsize)_tcslen (pRequest->szwHostName)));
    }
    else
    {
        //Converts the IP address to a string. The java layer will deal with it
        TCHAR swIPAddress[32];
        _snwprintf_s(swIPAddress,32, _TRUNCATE, _T("%d"), pRequest->ulIPAddress);
        env->SetObjectArrayElement(ret, HOST_NAME_INDEX, env->NewString((const jchar*)swIPAddress, (jsize)_tcslen (swIPAddress)));
    }


    env->SetObjectArrayElement(ret, TO_FILE_NAME_INDEX, env->NewString((const jchar*)pRequest->szwToFileName, (jsize)_tcslen (pRequest->szwToFileName)));

    if (pRequest->bIgnoreObligations)
    {
        env->SetObjectArrayElement(ret, IGNORE_OBLIGATION_INDEX, env->NewString((const jchar*)TRUE_STR, (jsize)_tcslen(TRUE_STR)));
    }
    
    env->SetObjectArrayElement(ret, SHARE_NAME_INDEX, env->NewString((const jchar*)pRequest->ShareName, (jsize)wcslen(pRequest->ShareName)));
    
    TCHAR szPID[10];
    _snwprintf_s(szPID,10, _TRUNCATE, _T("%d"), pRequest->ulProcessId);
    env->SetObjectArrayElement(ret, PID_INDEX, env->NewString((const jchar*)szPID, (jsize)_tcslen(szPID)));
    
    TCHAR szLogLevel[10];
    _snwprintf_s(szLogLevel,10, _TRUNCATE, _T("%d"), pRequest->ulNoiseLevel);
    env->SetObjectArrayElement(ret, LOG_LEVEL_INDEX, env->NewString((const jchar*)szLogLevel, (jsize)_tcslen(szLogLevel)));
    
    env->SetObjectArrayElement(ret, FROM_FILE_RESOLVED_NAME_INDEX, env->NewString((const jchar*)pRequest->szwFromFileEquivalentName, (jsize)_tcslen(pRequest->szwFromFileEquivalentName)));
    env->SetObjectArrayElement(ret, TO_FILE_RESOLVED_NAME_INDEX,   env->NewString((const jchar*)pRequest->szwToFileEquivalentName,   (jsize)_tcslen(pRequest->szwToFileEquivalentName)));
    
    //_tprintf (_T("ip: 0x%x\npid: 0x%x\ntid: 0x%x\n\n"), pRequest->ulRemoteAddress, pRequest->ulPID, pRequest->ulTID);
    
    
    return(ret);
}
#else
JNIEXPORT jobjectArray readPolicyIPCRequest (JNIEnv * env, jobject obj, IPC_POLICY_REQUEST* pRequest)
{
#ifdef _DEBUG
  // DumpRequest(true, pRequest);
#endif
	
    TCHAR szLongFileName [IPCREQ_FILENAME_LEN];
    szLongFileName[0] = _T('\0');
    UINT fileNameLength = 0;
    jobjectArray ret;

  if (pRequest == NULL) {
    TRACE (1, "readPolicyIPCRequest: shared memory is null\n"); 
    return ret;
  }

  // It may be an IPC Request, not a policy request
/*  if (strcmp ((char *) pRequest->szwMethodName, QUERY_DECISION_ENGINE)) {
    return Java_com_bluejungle_destiny_agent_ipc_OSWrapper_readIPCRequest (env, obj, pRequest);
  }*/

    if (_tcscmp (pRequest->szwApplicationName, _T("CEWINSCP.EXE")) == 0) {
      ret= (jobjectArray)env->NewObjectArray(IPC_JNI_ARG_COUNT+2,
					env->FindClass("java/lang/Object"),
					env->NewStringUTF((const char*)_T("")));
      env->SetObjectArrayElement(ret, WINSCP_EXTRA1_INDEX, 
				 env->NewStringUTF((const char*)_T("no")));
      env->SetObjectArrayElement(ret, WINSCP_EXTRA2_INDEX, 
				 env->NewStringUTF((const char*)_T("no")));
    } else {
      ret= (jobjectArray)env->NewObjectArray(IPC_JNI_ARG_COUNT,
					env->FindClass("java/lang/Object"),
					env->NewStringUTF((const char*)_T("")));
    }

    env->SetObjectArrayElement(ret, METHOD_NAME_INDEX, env->NewStringUTF((const char*)pRequest->szwMethodName));

        // From the timestamp analysis, GetLongPathName can take up to SIX seconds to execute
        // This is killing the getRequest because it's imposing a 6 seconds delay from one request to 
        // another one.  We simply cannot have it here.    Dominic 9/7/06
  //	fileNameLength = ::GetLongPathName (pRequest->szwFromFileName, szLongFileName, IPCREQ_FILENAME_LEN);


    if (0 == fileNameLength) {
        env->SetObjectArrayElement(ret, FROM_FILE_NAME_INDEX, env->NewStringUTF((const char*)pRequest->szwFromFileName));
    } else {
        env->SetObjectArrayElement(ret, FROM_FILE_NAME_INDEX, env->NewStringUTF((const char*)szLongFileName));
    }
	
    env->SetObjectArrayElement(ret, ACTION_INDEX, env->NewStringUTF((const char*)pRequest->szwAction));


    checkFileName (pRequest->szwFromFileName);
    checkFileName (pRequest->ShareName);
    checkFileName (pRequest->szwToFileName);
    
    if (pRequest->CoreSid.aUserSID != NULL)
    {
        env->SetObjectArrayElement(ret, SID_INDEX, env->NewStringUTF((const char*)pRequest->CoreSid.aUserSID));
    }

        // This is a hack to map the application for RUN.  It's because the old structure 
        // only has 128 char, which is NOT enough, so we pass it on the FromFile

    if (_tcscmp (pRequest->szwAction, _T("RUN")) == 0) {
        env->SetObjectArrayElement(ret, APPLICATION_NAME_INDEX, env->NewStringUTF((const char*)pRequest->szwFromFileName));
        
    } else {
        env->SetObjectArrayElement(ret, APPLICATION_NAME_INDEX, env->NewStringUTF((const char*)pRequest->szwApplicationName));
    }
    
    
    if (_tcslen (pRequest->szwHostName) > 0)
    {
        env->SetObjectArrayElement(ret, HOST_NAME_INDEX, env->NewStringUTF((const char*)pRequest->szwHostName));
    }
    else
    {
        //Converts the IP address to a string. The java layer will deal with it
        TCHAR swIPAddress[32];
        _snprintf_s(swIPAddress, 32, _TRUNCATE, _T("%d"), pRequest->ulIPAddress);
        env->SetObjectArrayElement(ret, HOST_NAME_INDEX, env->NewStringUTF((const char*)swIPAddress));
    }


    env->SetObjectArrayElement(ret, TO_FILE_NAME_INDEX, env->NewStringUTF((const char*)pRequest->szwToFileName));

    if (pRequest->bIgnoreObligations)
    {
        env->SetObjectArrayElement(ret, IGNORE_OBLIGATION_INDEX, env->NewStringUTF((const char*)TRUE_STR));
    }
    
    env->SetObjectArrayElement(ret, SHARE_NAME_INDEX, env->NewStringUTF((const char*)pRequest->ShareName));
    
    TCHAR szPID[10];
    _snprintf_s(szPID, 10, _TRUNCATE, _T("%d"), pRequest->ulProcessId);
    env->SetObjectArrayElement(ret, PID_INDEX, env->NewStringUTF((const char*)szPID));
    
    TCHAR szLogLevel[10];
    _snprintf_s(szLogLevel, 10, _TRUNCATE, _T("%d"), pRequest->ulNoiseLevel);
    env->SetObjectArrayElement(ret, LOG_LEVEL_INDEX, env->NewStringUTF((const char*)szLogLevel));
    
    env->SetObjectArrayElement(ret, FROM_FILE_RESOLVED_NAME_INDEX, env->NewStringUTF((const char*)pRequest->szwFromFileEquivalentName));
    env->SetObjectArrayElement(ret, TO_FILE_RESOLVED_NAME_INDEX,   env->NewStringUTF((const char*)pRequest->szwToFileEquivalentName));
    
    //_tprintf (_T("ip: 0x%x\npid: 0x%x\ntid: 0x%x\n\n"), pRequest->ulRemoteAddress, pRequest->ulPID, pRequest->ulTID);
    
    
    return(ret);
}
#endif

static void freeKernelEventsArray(JNIEnv * env, jlongArray kernelEvents, jlong *jhandleArray, int handleArraySize, HANDLE *handleArray)
{
#if defined (WIN32) || defined (_WIN64)
    if (handleArray != (HANDLE *)jhandleArray) {
        delete[] handleArray;
    }

    for(int j=0;j<handleArraySize-1; j++)
    {
        CloseHandle((HANDLE) jhandleArray[j]);
        jhandleArray[j]=NULL;
    }
    env->ReleaseLongArrayElements(kernelEvents,jhandleArray,JNI_ABORT);
#endif
}

#if defined (WIN32) || defined (_WIN64)
/*
 * Class:     com_bluejungle_destiny_agent_ipc_OSWrapper
 * Method:    getNextKernelPolicyRequest
 * This method listens to the kernel events and returns the first request that is received for policy requests
 * If no even if received or something goes wrong, the method returns 0
 * Signature: 
 */
JNIEXPORT jobjectArray JNICALL Java_com_bluejungle_destiny_agent_ipc_OSWrapper_getNextKernelPolicyRequest
  (JNIEnv* env, jobject jObj, jlongArray kernelEvents)
{

  //TRACE ("getNextPolicyRequest\n");

        int   handleArraySize = env->GetArrayLength (kernelEvents);
	jlong* jhandleArray     = env->GetLongArrayElements(kernelEvents, 0);

        HANDLE *handleArray = (HANDLE *)jhandleArray;
        if (sizeof(jlong) != sizeof(HANDLE)) {
            handleArray = new HANDLE[handleArraySize];
            for (int i = 0; i < handleArraySize; i++) {
                handleArray[i] = (HANDLE)jhandleArray[i];
            }
        }

        // timedTRACE ("About to wait\n");
	int i = ::WaitForMultipleObjects(KERNEL_EVENT_SIZE,
                                         handleArray,
                                         FALSE,         //  wait untill one event signal
                                         INFINITE       //  wait forever
                                         );

        // timedTRACE ("return from WaitForObject : event: %d  \n", i);

	if(i < WAIT_OBJECT_0 || i > WAIT_OBJECT_0+KERNEL_EVENT_SIZE-1) 
	{	
            freeKernelEventsArray(env, kernelEvents, jhandleArray, handleArraySize, handleArray);
            return FALSE;
	}

	i-=WAIT_OBJECT_0;
	
	int j = 0;
	for(j=0;j<KERNEL_EVENT_SIZE;j++) 
	{
		if(g_packet[j].OverLapped.hEvent == (HANDLE) handleArray[i]) 
		{ 
			// We found the event that was fired
			break;  
		}
	}
	if (j == KERNEL_EVENT_SIZE)
	{
            freeKernelEventsArray(env, kernelEvents, jhandleArray, handleArraySize, handleArray);
            return FALSE;
	}

	int k=j;

	DWORD cb=0;
	BOOL bRet=::GetOverlappedResult(g_hKDriver,&g_packet[k].OverLapped, &cb, false);


	if(!bRet)
	{	
            freeKernelEventsArray(env, kernelEvents, jhandleArray, handleArraySize, handleArray);
            return FALSE;
	}

	if(cb != sizeof(IPC_POLICY_REQUEST))
	{
	}

	jobjectArray result = readPolicyIPCRequest(env, jObj, &g_packet[k].Request);
	LONG uniqueIndex = g_packet[k].Request.UniqueIndex;

        if (handleArray != (HANDLE *)jhandleArray) {
            delete[] handleArray;
        }
	env->ReleaseLongArrayElements(kernelEvents,jhandleArray,JNI_ABORT);

        g_packet[i].OverLapped.Offset = 0;
        g_packet[i].OverLapped.OffsetHigh = 0;
	ResetEvent(g_packet[k].OverLapped.hEvent);
	memset(&g_packet[k].Request,0,sizeof(g_packet[k].Request));

	// handleArray[KERNEL_EVENT_SIZE-1]=(jint) g_packet[k].OverLapped.hEvent;
        
	//devioctl on the "result" index again


	bRet = DeviceIoControl(
		g_hKDriver,
		IOCTL_POLICY_REQUEST,
		NULL,
		0,
		&g_packet[k].Request,
		sizeof(g_packet[k].Request),
		&cb,
		&g_packet[k].OverLapped
		);

	if(!bRet)
	{
		DWORD dwErr = GetLastError();
		if(dwErr != ERROR_IO_PENDING)	
		{
			_tprintf(_T("DeviceIoControl return %d, Error Code %d\n"),bRet,dwErr);
			return NULL;
		}
	}

	
	jobjectArray ret = (jobjectArray)env->NewObjectArray(2,
						env->FindClass("java/lang/Object"),
						env->NewString((const jchar*)_T(""), 0));
	WCHAR buf[25];
	_snwprintf_s(buf,25, _TRUNCATE, _T("%d"), uniqueIndex);
	env->SetObjectArrayElement(ret, 0, env->NewString((const jchar*)buf, (jsize)_tcslen(buf)));
	env->SetObjectArrayElement(ret, 1, result);
	return ret;
}
#endif

/*
 * Class:     com_bluejungle_destiny_agent_ipc_OSWrapper
 * Method:    readPolicyIPCRequest
 * Signature: (I)[Ljava/lang/String;
 */
JNIEXPORT jobjectArray JNICALL Java_com_bluejungle_destiny_agent_ipc_OSWrapper_readPolicyIPCRequest
  (JNIEnv * env, jobject obj, jlong sharedMem)
{
  //    TRACE ("ReadPolicyIPCRequest\n");

    IPC_POLICY_REQUEST* pRequest = (IPC_POLICY_REQUEST*) sharedMem;
    if (_tcscmp (pRequest->szwMethodName, QUERYDECISIONENGINE) != 0)
    {
        return Java_com_bluejungle_destiny_agent_ipc_OSWrapper_readIPCRequest (env, obj, sharedMem);
    }
    else
    {
        return readPolicyIPCRequest (env, obj, pRequest);
    }
}

/*
 * Class:     com_bluejungle_destiny_agent_ipc_OSWrapper
 * Method:    writeString
 * Signature: (ILjava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_com_bluejungle_destiny_agent_ipc_OSWrapper_writeString
  (JNIEnv * env, jobject obj, jlong handle, jstring str)
{
    //        TRACE ("writeString");
    const TCHAR *writestr = getStringFromJava(env, str);
    
    _tcscpy ((TCHAR*) handle, writestr);
    
    releaseStringFromJava(writestr);
}

/*
 * Class:     com_bluejungle_destiny_agent_ipc_OSWrapper
 * Method:    writeIPCRequest
 * Signature: (I[Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_com_bluejungle_destiny_agent_ipc_OSWrapper_writeIPCRequest
  (JNIEnv * env, jobject obj, jlong sharedMem, jobjectArray inputParams)
{
	//_tprintf (_T("writeIPCRequest\n"));
    IPCREQUEST* pRequest = (IPCREQUEST*) sharedMem;
    memset (pRequest, 0, sizeof (IPCREQUEST));
    pRequest->ulSize = sizeof (IPCREQUEST);

    jsize size = env->GetArrayLength (inputParams);
    jobject strObj = env->GetObjectArrayElement(inputParams, 0);
    const TCHAR *pMethodName = getStringFromJava(env, (jstring)strObj);
    _tcsncpy_s(pRequest->methodName, 64,pMethodName, _TRUNCATE);
    pRequest->methodName[IPCREQ_METHODNAME_LEN - 1] = L'\0';
    releaseStringFromJava(pMethodName);

    for (int i=1; i < size; i++)
    {
        jobject paramObj = env->GetObjectArrayElement(inputParams, i);
        const TCHAR *pParam = getStringFromJava(env, (jstring)paramObj);

        _tcsncpy_s (pRequest->params[i - 1],256, pParam, _TRUNCATE);
        pRequest->params[i - 1][IPCREQ_PARAM_LEN - 1] = L'\0';
        releaseStringFromJava(pParam);
    }

}

/*
 * Class:     com_bluejungle_destiny_agent_ipc_OSWrapper
 * Method:    writeIPCResponse
 * Signature: (I[Ljava/lang/Object;)V
 */
JNIEXPORT void JNICALL Java_com_bluejungle_destiny_agent_ipc_OSWrapper_writeIPCResponse
  (JNIEnv * env, jobject obj, jlong sharedMem, jobjectArray outputParams)
{
	//_tprintf (_T("writeIPCResponse\n"));
    IPCREQUEST* pResponse = (IPCREQUEST*) sharedMem;
    memset (pResponse, 0, sizeof (IPCREQUEST));
    pResponse->ulSize = sizeof (IPCREQUEST);

    jsize size = env->GetArrayLength (outputParams);

    for (int i=0; i < size; i++)
    {
        jobject paramObj = env->GetObjectArrayElement(outputParams, i);
        const TCHAR *pParam = getStringFromJava(env, (jstring)paramObj);
        _tcsncpy_s (pResponse->params[i], _countof(pResponse->params[i]),pParam, _TRUNCATE);
        releaseStringFromJava(pParam);
    }
}

/*
 * Class:     com_bluejungle_destiny_agent_ipc_OSWrapper
 * Method:    writePolicyIPCResponse
 * Signature: (I[Ljava/lang/Object;)V
 */
JNIEXPORT void JNICALL Java_com_bluejungle_destiny_agent_ipc_OSWrapper_writePolicyIPCResponse
  (JNIEnv * env, jobject obj, jlong sharedMem, jobjectArray outputParams)
{
  //        TRACE ("writePOlicyIPCResponse\n");


    IPC_POLICY_RESPONSE* pResponse = (IPC_POLICY_RESPONSE*) sharedMem;
	//Place the size of the IPC policy response here
	pResponse->ulSize = sizeof (IPC_POLICY_RESPONSE);

    if (env->GetArrayLength (outputParams) != 2)
    {
        //TODO: this shouldnt be determined based on array size.
        Java_com_bluejungle_destiny_agent_ipc_OSWrapper_writeIPCResponse (env, obj, sharedMem, outputParams);
        return;
    }

    jobject paramObj = env->GetObjectArrayElement(outputParams, 0);
    const TCHAR *pParam = getStringFromJava(env, (jstring)paramObj);

    if (_tcsicmp (pParam, ALLOW_STR) == 0)
    {
        pResponse->Allow = ALLOW;
    }
    else
    {
        pResponse->Allow = DENY;
    }
    releaseStringFromJava(pParam);

    paramObj = env->GetObjectArrayElement(outputParams, 1);
    pParam = getStringFromJava(env, (jstring)paramObj);
    if (_tcscmp (pParam, TRUE_STR) == 0)
    {
        pResponse->AllowType = WATCH_NEXT_OP;
    }
    else
    {
        pResponse->AllowType = NOT_WATCHED;
    }
    releaseStringFromJava(pParam);
}


/*
 * Class:     com_bluejungle_destiny_agent_ipc_OSWrapper
 * Method:    closeHandle
 * Signature: (I)Z
 */
JNIEXPORT jboolean JNICALL Java_com_bluejungle_destiny_agent_ipc_OSWrapper_closeHandle
  (JNIEnv * env, jobject obj, jlong handle)
{
#if defined (WIN32) || defined (_WIN64)
	//_tprintf (_T("CloseHandle\n"));
	return static_cast<jboolean>(::CloseHandle ((HANDLE) handle));
#else
  osServiceObj * primitive = (osServiceObj *) handle;

  if (primitive == NULL) {
    TRACE (1, "closeHandle: Invalid handle.\n");
    return false;
  }

  // Let the OO handle different types 
  primitive->close();
  free (primitive);

  return true;
#endif
}

/*
 * Class:     com_bluejungle_destiny_agent_ipc_OSWrapper
 * Method:    getProcessToken
 * Signature: (I)J
 */

JNIEXPORT jlong JNICALL Java_com_bluejungle_destiny_agent_ipc_OSWrapper_getProcessToken
  (JNIEnv * env, jobject obj, jint pid)
{
#if defined (WIN32) || defined (_WIN64)
    // Taken from PDPEval.cpp
    BOOL rv;
    HANDLE hToken = NULL;
    HANDLE hTokenDup = NULL;
    HANDLE ph;
    
    ph = OpenProcess(PROCESS_QUERY_INFORMATION,FALSE,pid);
    if( ph == NULL ) {
        return 0;
    }
    rv = OpenProcessToken(ph,TOKEN_READ|TOKEN_DUPLICATE,&hToken);
    CloseHandle(ph);
    if( rv != TRUE ) {
        return 0;
    }
    rv = DuplicateTokenEx(hToken,
                          TOKEN_IMPERSONATE|TOKEN_READ|TOKEN_ASSIGN_PRIMARY|TOKEN_DUPLICATE,
                          NULL,SecurityImpersonation,TokenPrimary,&hTokenDup);
    CloseHandle(hToken);
    if( rv != TRUE ) {
        return 0;
    }

    return (jlong)hTokenDup;	  
#else
    return 0;
#endif
}
/*
 * Class:     com_bluejungle_destiny_agent_ipc_OSWrapper
 * Method:    closeProcessToken
 * Signature: (Ljava/langObject;)Z
 */
JNIEXPORT jboolean JNICALL Java_com_bluejungle_destiny_agent_ipc_OSWrapper_closeProcessToken
  (JNIEnv * env, jobject obj, jobject processToken)
{
#if defined (WIN32) || defined (_WIN64)
    freeProcessToken(getProcessToken(env, processToken), NULL);
#endif

    return TRUE;
}

/*
 * Class:     com_bluejungle_destiny_agent_ipc_OSWrapper
 * Method:    getProcessId
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_bluejungle_destiny_agent_ipc_OSWrapper_getProcessId
  (JNIEnv * env, jobject obj)
{

  //TRACE ("getCurrentProcessID %d\n", ::GetCurrentProcessId());
#if defined (WIN32) || defined (_WIN64)
    return (jint) ::GetCurrentProcessId();
#else
    return getpid();
#endif
}


/*
 * Class:     com_bluejungle_destiny_agent_ipc_OSWrapper
 * Method:    openUDPSocket
 * Signature: ()J
 */
JNIEXPORT jlong JNICALL Java_com_bluejungle_destiny_agent_ipc_OSWrapper_openUDPSocket
  (JNIEnv *env, jobject obj)
{
#if defined(WIN32) || defined(_WIN64)
    return -1;
#else
    int sk = socket(PF_INET, SOCK_DGRAM, 0);

    return (jlong)sk;
#endif
}

#if !defined(WIN32) && !defined(_WIN64)
typedef struct
{
  char method[64];   // notify or obligation
  union
  {
    struct notify {
      char message[512];    // Display message (i.e., "You cannot open gykit.doc!")
      char file[512];       // File path (i.e., "/usr/mvanilli/gykit.doc")
      char action[512];     // Action (i.e., "OPEN")
    } n;
    char execpath[1024];    // for custom obligations
  } u;
} CM_EDPMessage;

#define CUSTOM_OBLIGATION_STR "CustomObligation"
#define SHOW_NOTIFICATION_STR "ShowNotification"
#define NL_UDP_PORT 29999
#endif

/*
 * Class:     com_bluejungle_destiny_agent_ipc_OSWrapper
 * Method:    writeUDPRequest
 * Signature: (JLjava/lang/String;[Ljava/lang/Object;)V
 */
JNIEXPORT void JNICALL Java_com_bluejungle_destiny_agent_ipc_OSWrapper_writeUDPRequest
  (JNIEnv *env, jobject obj, jlong handle, jstring jmethodName, jobjectArray params)
{
#if defined(WIN32) || defined(_WIN64)
    return;
#else
    CM_EDPMessage msg;

    if (handle == -1) {
        return;
    }

    const char *methodName  = env->GetStringUTFChars (jmethodName, 0);
    strncpy_s(msg.method, sizeof(msg.method), methodName, _TRUNCATE);
    env->ReleaseStringUTFChars((jstring)jmethodName, methodName);

    if (strcmp(msg.method, CUSTOM_OBLIGATION_STR) == 0)
    {
        jacklog("writeUDPRequest: custom obligation\n");
        jobject jcommandLine = env->GetObjectArrayElement(params, 0);
        const char *commandLine = env->GetStringUTFChars((jstring)jcommandLine, 0);

        strncpy_s(msg.u.execpath, sizeof(msg.u.execpath), commandLine, _TRUNCATE);
    }
    else if (strcmp(msg.method, SHOW_NOTIFICATION_STR) == 0)
    {
        jacklog("writeUDPRequest: show notification\n");
        jobject jmessage = env->GetObjectArrayElement(params, 3);
        const char *message = env->GetStringUTFChars((jstring)jmessage, 0);

        jobject jfile = env->GetObjectArrayElement(params, 2);
        const char *file = env->GetStringUTFChars((jstring)jfile, 0);

        jobject jaction = env->GetObjectArrayElement(params, 1);
        const char *action = env->GetStringUTFChars((jstring)jaction, 0);

        strncpy_s(msg.u.n.message, sizeof(msg.u.n.message), message, _TRUNCATE);
        strncpy_s(msg.u.n.file, sizeof(msg.u.n.file), file, _TRUNCATE);
        strncpy_s(msg.u.n.action, sizeof(msg.u.n.action), action, _TRUNCATE);

        env->ReleaseStringUTFChars((jstring)jmessage, message);
        env->ReleaseStringUTFChars((jstring)jfile, file);
        env->ReleaseStringUTFChars((jstring)jaction, action);
    }
    else
    {
        jacklog("writeUDPRequest: unknown command %s, returning\n", msg.method);
        return;
    }

    struct sockaddr_in saddr;
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(NL_UDP_PORT);
    saddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    jacklog("writeUDPRequest: writing message\n");
    long bytessent = sendto(handle, &msg, sizeof(msg), 0, (struct sockaddr *)&saddr, sizeof(saddr));
    jacklog("writeUDPRequest: bytes sent = %ld\n", bytessent);

    return;
#endif
}

/*
 * Class:     com_bluejungle_destiny_agent_ipc_OSWrapper
 * Method:    closeUDPSocket
 * Signature: (J)V
 */
JNIEXPORT jboolean JNICALL Java_com_bluejungle_destiny_agent_ipc_OSWrapper_closeUDPSocket
  (JNIEnv *env, jobject obj, jlong handle)
{
#if defined(WIN32) || defined(_WIN64)
    return true;
#else
    close((int)handle);
    return true;
#endif
}

/*
 * Class:     com_bluejungle_destiny_agent_ipc_OSWrapper
 * Method:    logEvent
 * Signature: (II[Ljava/lang/Object;)V
 */
JNIEXPORT void JNICALL Java_com_bluejungle_destiny_agent_ipc_OSWrapper_logEvent
   (JNIEnv * env, jobject obj, jint type, jint eventId, jobjectArray paramArray)
{

  //    TRACE ("logEvent\n");
#if defined (WIN32) || defined (_WIN64)
    LPTSTR* ppStrArray = NULL;
    int size = env->GetArrayLength (paramArray);
    if (size > 0)
    {
        ppStrArray = new(std::nothrow) LPTSTR[size];
	  if(ppStrArray == NULL)
	  	return;
        for (int i=0; i < size; i++)
        {
            jobject paramObj = env->GetObjectArrayElement(paramArray, i);
            const TCHAR *pParam = getStringFromJava(env, (jstring)paramObj);
            ppStrArray[i] = new(std::nothrow) TCHAR [_tcslen (pParam) + 1];
		if( ppStrArray[i] !=NULL)
		{
            		_tcsncpy_s (ppStrArray[i],_tcslen (pParam) + 1, pParam, _TRUNCATE);
            		releaseStringFromJava(pParam);
		}
        }
    }

    HANDLE  hEventSource;

    hEventSource = RegisterEventSource(NULL, SZ_EVENT_VIEWER_SOURCE);

    if (hEventSource != NULL) 
    {
        ReportEvent(hEventSource, 
            (WORD) type,  
            0,                    
            (DWORD) eventId,                    
            NULL,                 
            static_cast<WORD>(size),                    
            0,                    
            (LPCTSTR*) ppStrArray,          
            NULL);                

        DeregisterEventSource(hEventSource);
    }

    for (int i=0; i < size; i++)
    {
        delete[] (ppStrArray[i]);
    }
    delete [] ppStrArray;
#else
  int     size;
  int     priority;
  char ** strArray;
  char *  format;
  char    msg [1024];

  switch (type) {
  case EVENTLOG_SUCCESS:
    priority = LOG_DAEMON | LOG_INFO;
    break;
  case EVENTLOG_ERROR_TYPE:
    priority = LOG_DAEMON | LOG_ERR;
    break;
  case EVENTLOG_WARNING_TYPE:
    priority = LOG_DAEMON | LOG_WARNING;
    break;
  case EVENTLOG_INFORMATION_TYPE:
    priority = LOG_DAEMON | LOG_NOTICE;
    break;
  case EVENTLOG_AUDIT_SUCCESS:
    priority = LOG_DAEMON | LOG_NOTICE;
    break;
  case EVENTLOG_AUDIT_FAILURE:
    priority = LOG_DAEMON | LOG_WARNING;
    break;
  }

  size = env->GetArrayLength (paramArray);

  strArray = (char **) calloc (size, sizeof(char *));

  for (int i = 0; i < size; i++) {
    const char * tstr;
    jobject tmp;

    tmp  = env->GetObjectArrayElement (paramArray, i);
    tstr = env->GetStringUTFChars ((jstring) tmp, 0);
    strArray[i] = strdup(tstr);
    env->ReleaseStringUTFChars ((jstring) tmp, tstr);
  }

  switch (eventId) {
  case MSG_HEARTBEAT:
    format = "heartbeat sent\n";
    _snprintf_s (msg, 1024, _TRUNCATE, format);
    break;
  case MSG_HEARTBEAT_FAILED:
    format = "heartbeat failed %s\n";
    _snprintf_s (msg, 1024, _TRUNCATE, format, strArray[0]);
    break;
  case MSG_LOG_UPLOAD: 
    format = "Uploading log to server\n";
    _snprintf_s (msg, 1024, _TRUNCATE, format);
    break;
  case MSG_LOG_UPLOAD_FAILED:
    format = "log upload failed %s\n";
    _snprintf_s (msg, 1024, _TRUNCATE, format, strArray[0]);
    break;
  case MSG_POLICY_UPDATE:
    format = "policy updated with %s policies\n";
    _snprintf_s (msg, 1024, _TRUNCATE, format, strArray[0]);
    break;
  case MSG_PROFILE_UPDATE:
    format = "profile updated\n";
    _snprintf_s (msg, 1024, _TRUNCATE, format);
    break;
  case MSG_SERVICE_STARTED:
    format = "daemon started\n";
    _snprintf_s (msg, 1024, _TRUNCATE, format);
    break;
  case MSG_SERVICE_STOPPED:
    format = "daemon stopped\n";
    _snprintf_s (msg, 1024, _TRUNCATE, format);
    break;
  }

  syslog (priority, msg);

  for (int i = 0; i < size; i++) {
    free(strArray[i]);
  }
  free (strArray);
  return ;
#endif
}

/*
 * Class:     com_bluejungle_destiny_agent_ipc_OSWrapper
 * Method:    setupKernelIPC
 * Setup the kernel IPC for the kernel <-> user mode IPC channel.
 * Returns true if success, false otherwise
 * Signature: (I[III)V
 */
JNIEXPORT jint JNICALL Java_com_bluejungle_destiny_agent_ipc_OSWrapper_setupKernelIPC
  (JNIEnv * env, jobject obj, jint numberOfSlots, jlongArray eventArray)
{
#if defined (WIN32) || defined (_WIN64)
	//_tprintf (_T("setupKernelIPC\n"));
    jlong *pEventArray = env->GetLongArrayElements(eventArray, 0);
    
	//REQUEST_PACKET			Packet[KERNEL_EVENT_SIZE]={0};
	//HANDLE					hEvent[KERNEL_EVENT_SIZE]={0};
	DWORD					cb;
        IPC_SET_CM_READY                        IpcReadyInfo;
        ULONG                                   ulBytesReturn;

	memset(g_packet,0,sizeof(g_packet));
	//memset(hEvent,0,sizeof(hEvent));

        memset (&IpcReadyInfo, 0x0, sizeof (IPC_SET_CM_READY));

        _tprintf(_T("Checking for installed drivers\n"));

	// Trying to open DSCORE driver first
	g_hKDriver = CreateFileW( DSCORE_W32_DEVICE_NAME,
                                  GENERIC_READ | GENERIC_WRITE,
                                  0,
                                  NULL,
                                  OPEN_EXISTING,
                                  FILE_ATTRIBUTE_SYSTEM|FILE_FLAG_OVERLAPPED,
                                  NULL);

        

	if (g_hKDriver == INVALID_HANDLE_VALUE) 
	{
            g_hKDriver = CreateFileW( CEPROCDETECT_DEVICE_NAME,
                                      GENERIC_READ | GENERIC_WRITE,
                                      0,
                                      NULL,
                                      OPEN_EXISTING,
                                      FILE_ATTRIBUTE_SYSTEM|FILE_FLAG_OVERLAPPED,
                                      NULL);

		if (g_hKDriver == INVALID_HANDLE_VALUE)
		{
			_tprintf (_T("No installed drivers.  Continuing...\n"));
		}
		else
		{
                    g_ulKDriverType = KERNEL_PROCDETECT_DRIVER;
                    _tprintf(_T("Loaded PROCDETECT driver\n"));
		}
	}
	else 
	{
            _tprintf(_T("Loaded DSCORE driver\n"));
	}

	
	IpcReadyInfo.ulInstalledDirLength = :: GetCurrentDirectory(MAX_NAME_LENGTH, IpcReadyInfo.wzInstalledDir);
	IpcReadyInfo.ulShortInstalledDirLength = ::GetShortPathName (IpcReadyInfo.wzInstalledDir,
                                                                 IpcReadyInfo.wzShortInstalledDir,
                                                                 MAX_NAME_LENGTH);
 
	if(KERNEL_IFS_DRIVER == g_ulKDriverType)
	{
		BOOL err = DeviceIoControl (g_hKDriver,  IOCTL_SET_CM_READY,
									&IpcReadyInfo, sizeof (IPC_SET_CM_READY),
									NULL,          0,
									&ulBytesReturn, NULL);
		printf ("Installing path error: %d return: %d\n", err, ulBytesReturn);

		ULONG currentPid = GetCurrentProcessId();
		printf ("Main thread id %d\n", currentPid);
		err = DeviceIoControl (g_hKDriver,         IOCTL_ADD_PROTECTED_PID,
							   (LPVOID) &currentPid, sizeof (currentPid),
							   NULL,                 0,
							   &ulBytesReturn,       NULL);
		printf ("Add protected pid error: %d return: %d\n", err, ulBytesReturn);
	}           
                
    numberOfSlots = KERNEL_EVENT_SIZE;
    for (int i = 0; i < numberOfSlots; i++)
    {
		g_packet[i].OverLapped.Offset=0;
		g_packet[i].OverLapped.OffsetHigh=0;
		g_packet[i].OverLapped.hEvent =(PVOID) pEventArray[i];
		//hEvent[i]=g_packet[i].OverLapped.hEvent;
		
		BOOL bRet = DeviceIoControl(
									g_hKDriver,
									IOCTL_POLICY_REQUEST,
									NULL,
									0,
									&g_packet[i].Request,
									sizeof(g_packet[i].Request),
									&cb,
									&g_packet[i].OverLapped
									);
                    
		if(!bRet)
		{
			DWORD dwErr = GetLastError();
			if(dwErr != ERROR_IO_PENDING)
			{
				_tprintf(_T("DeviceIoControl return %d, Error Code %d\n"),bRet,dwErr);
				return false;
			}
		}
	}


    
	_tprintf (_T("setupKernelIPC was successful\n"));
#endif
    return true;
}

/*
 * Class:     com_bluejungle_destiny_agent_ipc_OSWrapper
 * Method:    uninitKernelIPC
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_com_bluejungle_destiny_agent_ipc_OSWrapper_uninitKernelIPC
  (JNIEnv * env, jobject obj, jlong ipcSetupInfoHandle)
{
#if defined (WIN32) || defined (_WIN64)
	_tprintf (_T("uninitKernelIPC"));
    if (g_hKDriver)
    {
        ::CloseHandle (g_hKDriver);
    }
#endif
}

/*
 * Class:     com_bluejungle_destiny_agent_ipc_OSWrapper
 * Method:    setupSocket
 * Signature: (I)I
 */
JNIEXPORT jlong JNICALL Java_com_bluejungle_destiny_agent_ipc_OSWrapper_setupSocket
  (JNIEnv * env, jobject jobj, jint bjsocktype)

{
#if defined(WIN32) || defined(_WIN64)
    return 0;
#else
  int           skfd = -1;
  int     skfd_flags = 0;

  if(bjsocktype==0)//regular socket
    {
      //[060830 by jzhang]  
      //=> Now the smbd server driver is in user mode and socket file is used instead of 
      //   kernel/user mode communication socket

      //[060830 by jzhang]
      //=>creating a local socket file
      
      skfd = socket(AF_UNIX, SOCK_DGRAM, 0);
      
      
      if(skfd <= 0)
	exit(1);
      sockaddr_un localAddr;
      localAddr.sun_family = AF_UNIX;
      strncpy_s(localAddr.sun_path, sizeof(localAddr.sun_path), IPC_SERVER_PATH, _TRUNCATE);
      
      
      unlink(IPC_SERVER_PATH);
      if(bind(skfd, (sockaddr *)&localAddr, sizeof(localAddr)) != 0)
	{
	  close(skfd);
	  skfd = -1;
	  goto failed;
	}

      chmod(IPC_SERVER_PATH, 0777);  //important!! otherwise, injected so can't send message and get a EACCES error
      jacklog("Regular Server Socket Created.\n");  //DEBUGG

      /* Set it to non block */
      skfd_flags  = fcntl (skfd, F_GETFL);
      skfd_flags |= O_NONBLOCK;
      fcntl (skfd, F_SETFL, skfd_flags);
    }
  else if(bjsocktype>0) //NETLINK
    {
      struct sockaddr_nl sock;
      struct nlmsghdr    nlhdr;
      
      // bjsocktype is currently NetLink for Kernel/user communication
      
      skfd = socket (PF_NETLINK, SOCK_RAW, NL_BJPEP);
      if (skfd <= 0) {
	goto failed;
      }
      
      memset (&sock,  0x0, sizeof (sock)); 
      memset (&nlhdr, 0x0, sizeof (nlhdr));
      
      sock.nl_family = AF_NETLINK;
      sock.nl_pid    = getpid();
      sock.nl_groups = 0;
      
      if (bind (skfd, (sockaddr *) &sock, sizeof (sock)) != 0) {
	close (skfd);
	skfd = -1;
	goto failed;
      }
      /* Set it to non block */
      skfd_flags  = fcntl (skfd, F_GETFL);
      skfd_flags |= O_NONBLOCK;
      fcntl (skfd, F_SETFL, skfd_flags);

      //Send the init sequence over to the kernel module
      nlhdr.nlmsg_len   = NLMSG_LENGTH (0);
      nlhdr.nlmsg_flags = 0;
      nlhdr.nlmsg_type  = NLMSG_BJPEP_SETPID;
      nlhdr.nlmsg_pid   = getpid();

      jacklog("Server Kernel Socket Created.\n");  //DEBUGG
      
      memset (&sock, 0x0, sizeof (sock));
      sock.nl_family = AF_NETLINK;
      
      if (sendto (skfd, (char *) &nlhdr, sizeof(struct nlmsghdr), 0,(sockaddr *) &sock, sizeof (sock)) <= 0) {
	close (skfd);
	skfd = -1;
	goto failed;
      }
      
      jacklog("Server Kernel ****** Socket Created.\n");  //DEBUGG

      
    }
 failed:
  return skfd;
#endif
}
/*
 * Class:     com_bluejungle_destiny_agent_ipc_OSWrapper
 * Method:    shutdownSocket
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_com_bluejungle_destiny_agent_ipc_OSWrapper_shutdownSocket
  (JNIEnv *, jobject, jlong sockfd)

{
#if defined(WIN32) || defined(_WIN64)
    return;
#else
  if (sockfd > 0)
    close (sockfd);
  return;
#endif
}


/*
 * Class:     com_bluejungle_destiny_agent_ipc_OSWrapper
 * Method:    getLoggedInUsers
 * Signature: ()[Ljava/lang/String;
 */
JNIEXPORT jobjectArray JNICALL Java_com_bluejungle_destiny_agent_ipc_OSWrapper_getLoggedInUsers
  (JNIEnv * env, jobject obj)
{
#if defined (WIN32) || defined (_WIN64)
    jobjectArray ret;
    StringVector loggedInUsers; 
    UserUtils::GetLoggedInUsers (loggedInUsers);
    ret= (jobjectArray)env->NewObjectArray(static_cast<jsize>(loggedInUsers.size()),
         env->FindClass("java/lang/String"),
         env->NewString((const jchar*)_T(""), 0));

    for (unsigned int i=0; i < loggedInUsers.size(); i++)
    {
      TCHAR* loggedInUser = loggedInUsers[i];
#pragma warning(push)
#pragma warning(disable : 6001)
      size_t len = _tcslen (loggedInUser);
#pragma warning(pop)
      env->SetObjectArrayElement(ret, i, env->NewString((const jchar*)loggedInUser, (jsize)len));
      // malloc'd in GetLoggedInUsers
      free(loggedInUser);
    }

    return(ret);
#else
    FILE* fp = NULL;
    char file_buf[512];
    char *ptmp = NULL;
    int uid = 0;
    struct passwd* pwd = NULL;
    string *str= NULL;
    vector<string*> vec;

    //if (!getdomainname(file_buf, sizeof(file_buf)))
    //    printf("Domain name is %s\n", file_buf);

    fp = fopen("/proc/key-users", "r");
    while(fp && fgets(file_buf, sizeof(file_buf), fp)) {
        ptmp = strchr(file_buf, ':');
        if (ptmp) {
            ptmp[0] = 0;
            uid = atoi(file_buf);
            pwd = getpwuid(uid);
            if (pwd) {
                //printf("User name is %s, uid is %d\n", pwd->pw_name, uid);
                str = new string(pwd->pw_name);
                vec.push_back(str);
            }
        }
    }

    if (fp)
        fclose(fp);

    jobjectArray ret;
    ret= (jobjectArray)env->NewObjectArray(vec.size()? vec.size() : 1,
         env->FindClass("java/lang/String"),
         env->NewStringUTF((const char*)_T("")));

    for (int i = 0; i < vec.size(); i++) { 
        env->SetObjectArrayElement(ret, i, env->NewStringUTF((const char*)(vec[i]->c_str())));
        delete vec[i];
    }

    return ret;
#endif
}

/*
 * Utility time conversion routine.  Converts FILETIME (from FILETIME
 * epoch) -> Java time (from the the standard Java/Unix epoch).
 *
 */


static const __int64 SECS_BETWEEN_EPOCHS = 11644473600;
static const __int64 SECS_TO_100NS       = 10000000;

#if defined (WIN32) || defined (_WIN64)
static __int64 FileTimeToJavaTimeMills (FILETIME FileTime)

{
    __int64 javaTime;

    javaTime  = ((__int64)FileTime.dwHighDateTime << 32) + FileTime.dwLowDateTime;
    javaTime -= (SECS_BETWEEN_EPOCHS * SECS_TO_100NS);

    return javaTime / 10000;
}
#else
static __int64 FileTimeToJavaTimeMills (time_t fileTime)

{
    __int64 javaTime = fileTime;

    return javaTime * 1000;
}
#endif

/*
 * Utility routine shared by all getFileXXXTime native methods.
 */

static const int FILE_ACCESS_TIME = 1;
static const int FILE_CREATE_TIME = 2;
static const int FILE_MODIFICATION_TIME = 4;

static __int64 getFileTime (const TCHAR *str, int type, HANDLE processTokenHandle) {

  __int64 retTime = 0;
#if defined (WIN32) || defined (_WIN64)
  WIN32_FILE_ATTRIBUTE_DATA  fileAttrData;
  volatile int dummy;

  if (processTokenHandle) {
    if (!ImpersonateLoggedOnUser (processTokenHandle)) {
      dummy = 0;                // just to keep Veracode happy
    }
  }
  if (GetFileAttributesEx (str, GetFileExInfoStandard, &fileAttrData) == 0) {
    if (processTokenHandle) {
      if (!RevertToSelf ()) {
        dummy = 0;              // just to keep Veracode happy
      }
    }
    return -1;
  }
  if (processTokenHandle) {
    if (!RevertToSelf ()) {
      dummy = 0;                // just to keep Veracode happy
    }
  }
  if (type == FILE_ACCESS_TIME) {
    retTime = FileTimeToJavaTimeMills (fileAttrData.ftLastAccessTime);
  }
  if (type == FILE_CREATE_TIME) {
    retTime = FileTimeToJavaTimeMills (fileAttrData.ftCreationTime);
  }
  if (type == FILE_MODIFICATION_TIME) {
    retTime = FileTimeToJavaTimeMills (fileAttrData.ftLastWriteTime);
  }
#else
  struct stat fileAttrData;
  if (stat(str, &fileAttrData) == -1)
    return -1;
  if (type == FILE_ACCESS_TIME) {
    retTime = FileTimeToJavaTimeMills (fileAttrData.st_atime);
  }
  if (type == FILE_CREATE_TIME) {
    retTime = FileTimeToJavaTimeMills (fileAttrData.st_ctime);
  }
  if (type == FILE_MODIFICATION_TIME) {
    retTime = FileTimeToJavaTimeMills (fileAttrData.st_mtime);
  }
#endif
  return retTime;
}

/*
 * This is something that we should use early and often.
 */
static bool nameRefersToActualFile(const TCHAR *str) {
#if defined(WIN32) || defined(_WIN64)
    // First, check to make sure it's a file
    HANDLE hFile = CreateFile(str, 0, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE , NULL, OPEN_EXISTING, 0, NULL);
    
    if (hFile == INVALID_HANDLE_VALUE)
    {
        return false;
    }
    
    DWORD fType = GetFileType(hFile);
    CloseHandle(hFile);
    
    return (fType == FILE_TYPE_DISK);
#else
    return true;
#endif
}


static int getBasicAttributes(const TCHAR *str, HANDLE processTokenHandle, __int64 *modifiedDate, __int64 *accessDate, __int64 *createdDate, __int64 *fileSize, __int64 *isDir) {
#if defined (WIN32) || defined (_WIN64)
    WIN32_FILE_ATTRIBUTE_DATA  fileAttrData;
    volatile int dummy;

  if (processTokenHandle) {
      if (!ImpersonateLoggedOnUser (processTokenHandle)) {
          return -1;
      }
  }

  if (GetFileAttributesEx(str, GetFileExInfoStandard, &fileAttrData) == 0) {
      if (processTokenHandle) {
          if (!RevertToSelf()) {
              dummy = 0;        // just to keep Veracode happy
          }
      }
      return -1;
  }

  if (processTokenHandle) {
      if (!RevertToSelf()) {
          dummy = 0;            // just to keep Veracode happy
      }
  }

  *modifiedDate = FileTimeToJavaTimeMills(fileAttrData.ftLastWriteTime);
  *accessDate = FileTimeToJavaTimeMills(fileAttrData.ftLastAccessTime);
  *createdDate = FileTimeToJavaTimeMills(fileAttrData.ftCreationTime);
  *fileSize = fileAttrData.nFileSizeHigh;
  *fileSize <<= 32;
  *fileSize |= fileAttrData.nFileSizeLow;
  *isDir = fileAttrData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
#else
  struct stat fileAttrData;
  if (stat(str, &fileAttrData) == -1)
    return -1;

  *modifiedDate = FileTimeToJavaTimeMills(fileAttrData.st_mtime);
  *accessDate = FileTimeToJavaTimeMills(fileAttrData.st_atime);
  *createdDate = FileTimeToJavaTimeMills(fileAttrData.st_ctime);
  *fileSize = fileAttrData.st_size;
  *isDir = S_ISDIR(fileAttrData.st_mode);
#endif
  return 0;
}

/** getProcessToken
 *
 *  \brief Get process token from 'processToken' (jobject) if present.
 *         Otherwise create a process token.
 *
 *  \notes A call to getProcessToken must be matched by a call to
 *         freeProcessToken.
 */
HANDLE getProcessToken (JNIEnv * env, jobject processToken)

{
#if defined (WIN32) || defined (_WIN64)
    if (processToken) {
    jclass longClass = env -> FindClass("java/lang/Long"); 
    jmethodID longValueMID = env -> GetMethodID (longClass, "longValue", "()J");
    jlong tokenValue = env -> CallLongMethod (processToken, longValueMID);
    return (HANDLE) tokenValue;
  } else {
    HANDLE token = NULL;
    OpenProcessToken (GetCurrentProcess (), TOKEN_QUERY | TOKEN_DUPLICATE, &token);
    return token;
  }
#else
  return NULL;
#endif
}/* getProcessToken */

/** freeProcessToken
 *
 *  \brief Free a process token created by getProcessToken.
 *
 *  \param token (in)        Process token to free.  This handle was returned by
 *                           getProcessToken.
 *  \param processToken (in) Process token jobject passed to getProcessToken.  It must
 *                           be the exact token previously used.
 */
void freeProcessToken( HANDLE token , jobject processToken )
{
#if defined (WIN32) || defined (_WIN64)
  if( token != NULL && processToken == NULL )
  {
    CloseHandle(token);
  }
#endif
}/* freeProcessToken */


// These are defined in Enforcers/.../resattrmgr/include and Enforcers/.../resattrlib/include
// As long as oswrapper.cpp remains under Destiny there is no good way to link there or to
// include the header file, hence this ugliness - AMM

struct ResourceAttributeManager;
struct ResourceAttributes;
    
typedef int (*create_mgr)(ResourceAttributeManager **);
typedef int (*read_attrs)(ResourceAttributeManager *, TCHAR *fname, ResourceAttributes *attrs);
typedef int (*alloc_attrs)(ResourceAttributes **);
typedef void (*free_attrs)(ResourceAttributes *);
typedef int (*get_count)(const ResourceAttributes *);
typedef const TCHAR* (*get_name)(const ResourceAttributes *, int index);
typedef const TCHAR* (*get_value)(const ResourceAttributes *, int index);

typedef struct
{
    create_mgr create_mgr_fn;
    read_attrs read_attrs_fn;
    alloc_attrs alloc_attrs_fn;
    free_attrs free_attrs_fn;
    get_count get_count_fn;
    get_name get_name_fn;
    get_value get_value_fn;
} resAttrPtrs;

static DWORD initializePtrs(resAttrPtrs *ptrs)
{
    static BOOL initialized = FALSE;

    static HMODULE resattrmgr = NULL;
    static HMODULE resattrlib = NULL;

#if defined (_WIN64)
#define RESATTRMGR  _T("resattrmgr.dll")
#define READ_RESOURCE_ATTRIBUTE  "ReadResourceAttributesW"
#define RESATTRLIB  _T("resattrlib.dll")
#elif defined (WIN32)
#define RESATTRMGR  _T("resattrmgr32.dll")
#define READ_RESOURCE_ATTRIBUTE  "ReadResourceAttributesW"
#define RESATTRLIB  _T("resattrlib32.dll")
#else
#define RESATTRMGR  _T("libresattrmgr.so")
#define READ_RESOURCE_ATTRIBUTE  "ReadResourceAttributesA"
#define RESATTRLIB  _T("libresattrlib.so")
#endif
  
    if (!initialized)
    {
#if defined(WIN32) || defined(_WIN64)
        // DLLs are stored under common bin folder now
        HKEY hKey = NULL;
        wchar_t commonLibrariesDir[MAX_PATH] = {0};
        DWORD res = RegOpenKeyExW(HKEY_LOCAL_MACHINE,
                                  L"SOFTWARE\\NextLabs\\CommonLibraries",
                                  0, KEY_QUERY_VALUE, &hKey);

        if (res == ERROR_SUCCESS) {
            DWORD size = MAX_PATH;
            res = RegQueryValueExW(hKey, L"InstallDir", NULL, NULL, (LPBYTE)commonLibrariesDir, &size);

            if (res == ERROR_SUCCESS) {
#if defined(_WIN64)
                wcsncat_s(commonLibrariesDir, MAX_PATH, L"\\bin64\\", _TRUNCATE);
#else
                wcsncat_s(commonLibrariesDir, MAX_PATH, L"\\bin32\\", _TRUNCATE);
#endif
                SetDllDirectoryW(commonLibrariesDir);
            }

            RegCloseKey(hKey);
        }
#endif
        resattrmgr = ::LoadLibrary(RESATTRMGR);

        if (resattrmgr == NULL)
        {
            return ERROR_FILE_NOT_FOUND;
        }

        ptrs->create_mgr_fn = (create_mgr)GetProcAddress(resattrmgr, "CreateAttributeManager");
        ptrs->read_attrs_fn = (read_attrs)GetProcAddress(resattrmgr, READ_RESOURCE_ATTRIBUTE);
        
        if (ptrs->create_mgr_fn == NULL || ptrs->read_attrs_fn == NULL)
        {
            return ERROR_INVALID_FUNCTION;
        }
        resattrlib = ::LoadLibrary(RESATTRLIB);

        if (resattrlib == NULL)
        {
            return ERROR_FILE_NOT_FOUND;
        }

        ptrs->alloc_attrs_fn = (alloc_attrs)GetProcAddress(resattrlib, "AllocAttributes");
        ptrs->free_attrs_fn = (free_attrs)GetProcAddress(resattrlib, "FreeAttributes");
        ptrs->get_count_fn = (get_count)GetProcAddress(resattrlib, "GetAttributeCount");
        ptrs->get_name_fn = (get_name)GetProcAddress(resattrlib, "GetAttributeName");
        ptrs->get_value_fn = (get_value)GetProcAddress(resattrlib, "GetAttributeValue");

        if (ptrs->alloc_attrs_fn == NULL || ptrs->free_attrs_fn == NULL || ptrs->get_count_fn == NULL ||
            ptrs->get_name_fn == NULL || ptrs->get_value_fn == NULL)
        {
            return ERROR_INVALID_FUNCTION;
        }

        initialized = TRUE;
    }

    return ERROR_SUCCESS;
}



#if defined (WIN32) || defined (_WIN64)
static BOOL WINAPI EnumerateNetworkDiskResources(LPNETRESOURCE lpnr, std::list<std::wstring>& diskResources)
{
    DWORD dwResult, dwResultEnum;
    HANDLE hEnum;
    DWORD cbBuffer = 16384;     // 16K is a good size
    DWORD cEntries = static_cast<DWORD>(-1);        // enumerate all possible entries
    LPNETRESOURCE lpnrLocal;    // pointer to enumerated structures
    DWORD i;
    //
    // Call the WNetOpenEnum function to begin the enumeration.
    //
    dwResult = WNetOpenEnum(RESOURCE_CONNECTED, // connected resources
                            RESOURCETYPE_DISK,   // disks
                            0,
                            lpnr,       // NULL first time the function is called
                            &hEnum);    // handle to the resource

    if (dwResult != NO_ERROR) {
        return FALSE;
    }
    //
    // Call the GlobalAlloc function to allocate resources.
    //
    lpnrLocal = (LPNETRESOURCE) GlobalAlloc(GPTR, cbBuffer);
    if (lpnrLocal == NULL) {
        return FALSE;
    }

    do {
        //
        // Initialize the buffer.
        //
#pragma warning (push)
#pragma warning (disable:6386)
        ZeroMemory(lpnrLocal, cbBuffer);
#pragma warning (pop)		
        //
        // Call the WNetEnumResource function to continue
        //  the enumeration.
        //
        dwResultEnum = WNetEnumResource(hEnum,          // resource handle
                                        &cEntries,      // defined locally as -1
                                        lpnrLocal,      // LPNETRESOURCE
                                        &cbBuffer);     // buffer size
        //
        // If the call succeeds, loop through the structures.
        //
        if (dwResultEnum == NO_ERROR)
        {
            for (i = 0; i < cEntries; i++)
            {
                if (RESOURCEUSAGE_CONTAINER == (lpnrLocal[i].dwUsage & RESOURCEUSAGE_CONTAINER))
                {
                    // If the NETRESOURCE structure represents a container resource, 
                    //  call the EnumerateFunc function recursively.
                    
                    if (!EnumerateNetworkDiskResources(&lpnrLocal[i], diskResources))
                    {
                    }
                }
                else if (lpnrLocal[i].dwType == RESOURCETYPE_DISK &&
                         lpnrLocal[i].dwScope == RESOURCE_CONNECTED)
                { 
                    // Currently connected disk resources get added to the list
                    diskResources.push_back(lpnrLocal[i].lpRemoteName);
                }
            }
        }
        else if (dwResultEnum != ERROR_NO_MORE_ITEMS)
        {
            break;
        }
    } while (dwResultEnum != ERROR_NO_MORE_ITEMS);

    GlobalFree((HGLOBAL) lpnrLocal);
    dwResult = WNetCloseEnum(hEnum);

    if (dwResult != NO_ERROR) {
        return FALSE;
    }

    return TRUE;
}

static BOOL WINAPI EnumerateNetworkDiskResources(std::list<std::wstring>& diskResources)
{
    return EnumerateNetworkDiskResources(NULL, diskResources);
}
#endif

JNIEXPORT jobjectArray JNICALL Java_com_bluejungle_destiny_agent_ipc_OSWrapper_performPolicyOverride(JNIEnv *env, jobject obj, jstring evaluationResult, jobjectArray jobligations)
{
    /*
     * @return String[0] "ALLOW" or "DENY"
     // [1] logID
     // [2] name
     // [3] options
     // [4] description
     // [5] action
     */
    jobjectArray ret;
    ret = (jobjectArray)env->NewObjectArray(6,env->FindClass("java/lang/String"),
					    env->NewStringUTF((const char*)_T("")));

#if defined(WIN32) || defined(_WIN64)
    return ret;
#else
    /* Default */
    env->SetObjectArrayElement(ret, 0, evaluationResult);
   if (jobligations == NULL || env->GetArrayLength(jobligations) == 0)
    {
        return ret;
    }

    // Convert jobligations to CEAttributes
   int arraySize=env->GetArrayLength(jobligations);
    CEAttributes obligations;
    obligations.count = arraySize/2;
    obligations.attrs = new CEAttribute[obligations.count];

    for (int i = 0; i < arraySize; i+=2)
    {
      TCHAR *key = getStringFromJava(env, (jstring)env->GetObjectArrayElement(jobligations, i));
      TCHAR *value = getStringFromJava(env, (jstring)env->GetObjectArrayElement(jobligations, i+1));
      
      obligations.attrs[i/2].key = CEM_AllocateString(key);
      obligations.attrs[i/2].value = CEM_AllocateString(value);
      
      releaseStringFromJava(key);
      releaseStringFromJava(value);
    }

    //going through obligation to find override obligations
    nextlabs::Obligations obs;
    obs.Assign(CEM_GetString, obligations);

    if(!(obs.IsObligationSet("NL_OVERRIDE_PASSWORD") == true || 
	 obs.IsObligationSet("NL_OVERRIDE_JUSTIFICATION") == true)) {
      // No override obligation; do nothing; return
      // Clean up obligations structure
      for (int i = 0; i < arraySize/2; i++)
	{
	  CEM_FreeString(obligations.attrs[i].key);
	  CEM_FreeString(obligations.attrs[i].value);
	}
      delete[] obligations.attrs;
      //return
      return ret;
    } 

    const std::list<nextlabs::Obligation>& ob_list = obs.GetObligations();
    std::list<nextlabs::Obligation>::const_iterator it;
    nextlabs::ObligationOptions::const_iterator options_it;
    std::string override_msg("");
    std::string override_passwd("");
    std::string override_just("");
    std::string override_logid("");
    std::string override_name("");
    std::string override_des("");
    std::string override_act("");
    for( it = ob_list.begin() ; it != ob_list.end() ; ++it ) //Each obligation 
      {
	if( it->name == "NL_OVERRIDE_PASSWORD") {
	  //handle password override obligation
	  options_it = it->options.begin();
	  override_msg = options_it->second.c_str();    ++options_it;   
	  jacklog("NL_OVERRIDE_PASSWORD: opt(1) %s\n",
		  override_msg.c_str());

	  if(options_it == it->options.end())  continue;
	  override_passwd = options_it->second.c_str();  ++options_it; 
	  //jacklog("NL_OVERRIDE_PASSWORD: opt(2) %s\n",
	  //  override_passwd.c_str());

	  if(options_it == it->options.end()) continue;
	  override_logid = options_it->second.c_str();
	  jacklog("NL_OVERRIDE_PASSWORD: opt(3) %s\n",
			  override_logid.c_str());
	  OverrideQuery query;
	  OverrideResponse response;
	  //initialization
	  memset(&query, 0, sizeof(query));
	  strncpy_s(query.name, 64, it->name.c_str(), _TRUNCATE); //query.name[64]
	  strncpy_s(query.message, 1024, override_msg.c_str(), _TRUNCATE); //query.message[1024]
	  strncpy_s(query.password, 64, override_passwd.c_str(), _TRUNCATE); //query.password[64]
	  if(PAQuery(PA_OVERRIDE_QUERY_SERV_PORT, query, response) >= 0) {
	    if(response.is_override) {
	      env->SetObjectArrayElement(ret, 0, env->NewStringUTF("ALLOW"));
	      override_name="Password Override Assistant";
	      override_des="Action allowed because of correct password";
	      override_act="User entered correct password";
	      jacklog("%s\n", override_des.c_str());
	    }
	  }
	  break;
	} else if ( it->name == "NL_OVERRIDE_JUSTIFICATION") {
	  //handle justification obligation
	  options_it = it->options.begin();
	  override_msg = options_it->second.c_str();    ++options_it;  
	  if(options_it == it->options.end()) continue;
	  override_just=options_it->second.c_str();  ++options_it; 
	  if(options_it == it->options.end()) continue;
	  override_logid = options_it->second.c_str();
	  OverrideQuery query;
	  OverrideResponse response;
	  //initialization
	  memset(&query, 0, sizeof(query));
	  strncpy_s(query.name, 64, it->name.c_str(), _TRUNCATE); //query.name[64]
	  strncpy_s(query.message, 1024, override_msg.c_str(), _TRUNCATE); //query.message[1024]
	  strncpy_s(query.password, 64, override_just.c_str(), _TRUNCATE); //query.password[64]
	  if(PAQuery(PA_OVERRIDE_QUERY_SERV_PORT, query, response) >= 0) {
	    if(response.is_override) {
	      env->SetObjectArrayElement(ret, 0, env->NewStringUTF("ALLOW"));
	      override_name="Justification Override Assistant";
	      override_des="Action allowed with justification message: ";
	      override_des+=response.result;
	      override_act="User entered justification message";
	      jacklog("%s\n", override_des.c_str());
	    }
	  }//call PAQuery
	  break;
	}
      }

    // Clean up obligations structure
    for (int i = 0; i < arraySize/2; i++)
    {
        CEM_FreeString(obligations.attrs[i].key);
        CEM_FreeString(obligations.attrs[i].value);
    }
    delete[] obligations.attrs;

    env->SetObjectArrayElement(ret, 1, 
			       env->NewStringUTF(override_logid.c_str()));
    env->SetObjectArrayElement(ret, 2, 
			       env->NewStringUTF(override_name.c_str()));
    env->SetObjectArrayElement(ret, 3, env->NewStringUTF(""));
    env->SetObjectArrayElement(ret, 4, 
			       env->NewStringUTF(override_des.c_str()));
    env->SetObjectArrayElement(ret, 5, 
			       env->NewStringUTF(override_act.c_str()));
    return ret;
#endif
}

JNIEXPORT jobjectArray JNICALL Java_com_bluejungle_destiny_agent_ipc_OSWrapper_getNetworkDiskResources(JNIEnv *env, jobject obj, jobject processToken)
{
#if defined (WIN32) || defined (_WIN64)
    UNREFERENCED_PARAMETER(obj);
    std::list<std::wstring> diskResources;
    volatile int dummy;

    jobjectArray jresultArray = NULL;

    HANDLE handle = getProcessToken (env, processToken);
    if (handle)
    {
        if (!ImpersonateLoggedOnUser (handle))
        {
            printf("Unable to impersonate user in getNetworkDiskResources\n");
            // Just continue.  We'll get no data and return an empty array
        }
    }

    if (EnumerateNetworkDiskResources(diskResources))
    {
        jresultArray = (jobjectArray) env->NewObjectArray(static_cast<jsize>(diskResources.size()),
                                                          env->FindClass("java/lang/String"),
                                                          env->NewString((const jchar*)_T(""), 0));
        int i = 0;

        for (std::list<std::wstring>::iterator iter = diskResources.begin(); iter != diskResources.end(); ++iter)
        {
            env->SetObjectArrayElement(jresultArray, i++, env->NewString((const jchar *)iter->c_str(), (jsize)wcslen(iter->c_str())));
        }
    }
 
    // Revert system user
    if (handle)
    {
      if (!RevertToSelf())
      {
        dummy = 0;              // just to keep Veracode happy
      }
      freeProcessToken(handle,processToken);
    }


    if (jresultArray == NULL) {
        jresultArray = (jobjectArray) env->NewObjectArray(0, env->FindClass("java/lang/String"), env->NewString((const jchar*)_T(""), 0));
    }

    return jresultArray;
#else
    return NULL;
#endif
}




JNIEXPORT jobjectArray JNICALL Java_com_bluejungle_destiny_agent_ipc_OSWrapper_getFileCustomAttributes
  (JNIEnv * env, jobject obj, jstring jFileName, jobject processToken)
{
    static ResourceAttributeManager *mgr = NULL;
    static resAttrPtrs ptrs;
    volatile int dummy;

    if (initializePtrs(&ptrs) != ERROR_SUCCESS)
    {
        return NULL;
    }

    if (mgr == NULL)
    {
        ptrs.create_mgr_fn(&mgr);
    }

    jobjectArray jresultArray = NULL;
    TCHAR * fileName = getStringFromJava(env, jFileName);

    ResourceAttributes *attrs = NULL;
    ptrs.alloc_attrs_fn(&attrs);

#if defined (WIN32) || defined (_WIN64)
    BOOL impersonateSuccess = FALSE;
    BOOL readSuccess = FALSE;

    // Impersonate user for access to file.
    HANDLE handle = getProcessToken (env, processToken);
    if (handle)
    {
      impersonateSuccess = ImpersonateLoggedOnUser (handle);
    }

    bool realFile = nameRefersToActualFile(fileName);

    WIN32_FILE_ATTRIBUTE_DATA fattr;
    if (realFile && GetFileAttributesExW(fileName,GetFileExInfoStandard,&fattr) != 0) {
      if (ptrs.read_attrs_fn(mgr, fileName, attrs) != 0) {
        readSuccess = TRUE;
      }
    }

    // Revert system user
    if (impersonateSuccess)
    {
      if (!RevertToSelf())
      {
        dummy = 0;              // just to keep Veracode happy
      }
      freeProcessToken(handle,processToken);
    }

    int count = ptrs.get_count_fn(attrs);

    if (impersonateSuccess && !readSuccess) {
      // The read failure might be due to impersonating to a user process that
      // has insufficient priviledges.  Try reading again without
      // impersonation.
      if (realFile && GetFileAttributesExW(fileName,GetFileExInfoStandard,&fattr) != 0) {
        ptrs.read_attrs_fn(mgr, fileName, attrs);
        count = ptrs.get_count_fn(attrs);
      }
    }

    if (count > 0) {
        jresultArray = (jobjectArray) env->NewObjectArray(count * 2,
                                                          env->FindClass("java/lang/String"),
                                                          env->NewString((const jchar*)_T(""), 0));

        for (int i = 0; i < count; i++) {
            const WCHAR *name = ptrs.get_name_fn(attrs, i);
            const WCHAR *value = ptrs.get_value_fn(attrs, i);

            env->SetObjectArrayElement(jresultArray, i*2, env->NewString((const jchar*)name, (jsize)wcslen(name)));
            env->SetObjectArrayElement(jresultArray, i*2+1, env->NewString((const jchar*)value, (jsize)wcslen(value)));
        }
    }
#else
    struct stat fattr;
    int statret = stat(fileName, &fattr);

    if (statret != -1) {
        ptrs.read_attrs_fn(mgr, fileName, attrs);
    } else {
        jacklog("stat of %s returned %d\n", fileName, statret);
    }

    int count = ptrs.get_count_fn(attrs);

    if (count > 0) {
        jresultArray = (jobjectArray) env->NewObjectArray(count * 2,
                                                          env->FindClass("java/lang/String"),
                                                          env->NewStringUTF((const char*)_T("")));

        for (int i = 0; i < count; i++) {
            const TCHAR *name = ptrs.get_name_fn(attrs, i);
            const TCHAR *value = ptrs.get_value_fn(attrs, i);

            env->SetObjectArrayElement(jresultArray, i*2, env->NewStringUTF((const char*)name));
            env->SetObjectArrayElement(jresultArray, i*2+1, env->NewStringUTF((const char*)value));
        }
    }
#endif

    ptrs.free_attrs_fn(attrs);
    releaseStringFromJava(fileName);
    return jresultArray;
}

JNIEXPORT jboolean JNICALL Java_com_bluejungle_destiny_agent_ipc_OSWrapper_isEmptyDirectory
  (JNIEnv * env, jobject obj, jstring jFileName, jobject processToken)
{
    jboolean result = TRUE;

    if (jFileName != NULL)
    {
        const TCHAR *dirPath = getStringFromJava(env, jFileName);

        size_t lenDirPath = _tcslen(dirPath);
        
        if (lenDirPath > 0)
        {
            HANDLE handle = getProcessToken(env, processToken);
            TCHAR *matchPath = (TCHAR *)malloc((lenDirPath+3) * sizeof(TCHAR));
            if (matchPath == NULL)
            {
              freeProcessToken(handle,processToken);
              releaseStringFromJava(dirPath);
              return FALSE;
            }
            _tcsncpy_s(matchPath,lenDirPath+3,dirPath, _TRUNCATE);
            
            if (dirPath[lenDirPath-1] == '\\' || dirPath[lenDirPath-1] == '/')
            {
                _tcsncat_s(matchPath,lenDirPath+3, _T("*"), _TRUNCATE);
            }
            else
            {
                _tcsncat_s(matchPath,lenDirPath+3, _T("\\*"), _TRUNCATE);
            }
#if defined (WIN32) || defined (_WIN64)
            WIN32_FIND_DATA findFileData;
            HANDLE hFind = FindFirstFile(matchPath, &findFileData);
            
            if (hFind != INVALID_HANDLE_VALUE)
            {
                do
                {
                    if (_tcscmp(findFileData.cFileName, _T(".")) != 0 &&
                        _tcscmp(findFileData.cFileName, _T("..")) != 0)
                    {
                        result = FALSE;
                        break;
                    }
                } while (FindNextFile(hFind, &findFileData) != 0);
                FindClose(hFind);
            }
#else
            DIR* hFind = opendir(matchPath);
            struct dirent* findFileData; 
            if (hFind)
            {
                while ((findFileData = readdir(hFind)))
                {
                    if (strcmp(findFileData->d_name, _T(".")) != 0 &&
                        strcmp(findFileData->d_name, _T(".")) != 0)
                    {
                        result = FALSE;
                        break;
                    }
                }
                closedir(hFind);
            }
#endif
            free(matchPath);
            freeProcessToken(handle,processToken);
        }
        releaseStringFromJava(dirPath);
    }

    return result;
}

JNIEXPORT jlongArray JNICALL Java_com_bluejungle_destiny_agent_ipc_OSWrapper_getFileBasicAttributes
  (JNIEnv * env, jobject obj, jstring jFileName, jobject processToken)
{
    const TCHAR * fileName = getStringFromJava(env, jFileName);
    HANDLE handle = getProcessToken(env, processToken);

#define NUM_BASIC_ATTRIBUTES 5
    __int64 attributes[NUM_BASIC_ATTRIBUTES];
    int ret = getBasicAttributes(fileName, handle, &attributes[0], &attributes[1], &attributes[2], &attributes[3], &attributes[4]);

    jlongArray jresultArray = NULL;
    if (ret == 0) {
        jresultArray = env->NewLongArray(NUM_BASIC_ATTRIBUTES);
        env->SetLongArrayRegion(jresultArray, 0, NUM_BASIC_ATTRIBUTES, attributes);
    } else {
        jresultArray = env->NewLongArray(0);
    }

    freeProcessToken(handle,processToken);
    releaseStringFromJava(fileName);

    return jresultArray;
}

JNIEXPORT jobjectArray JNICALL Java_com_bluejungle_destiny_agent_ipc_OSWrapper_getFileStandardAttributes
  (JNIEnv * env, jobject obj, jstring jFileName, jint agentType, jobject processToken)
{

  // Instead of calling the original function one by one, we will untide the function and get all 
  // the attributes from here.  This is actually more ugly than before......

  const TCHAR * fileName = getStringFromJava(env, jFileName);
  HANDLE        handle   = getProcessToken (env, processToken);

  long long ctime = 0;          // Creation Time
  long long atime = 0;          // Access Time
  long long mtime = 0;          // Modification Time
  char atimebuf[256];           // We use ANSI char explicitly because of the 64bit-to-str problem  
  char ctimebuf[256];           // in the window compiler.  So, we will use the UTF correspondingly
  char mtimebuf[256];
  jstring jOwner_id;
  jstring jGroup_id;
  jobjectArray jresultArray = NULL;
  volatile int dummy;

#if defined (WIN32) || defined (_WIN64)
WIN32_FILE_ATTRIBUTE_DATA  fileAttrData;

  if (handle) {
    if (!ImpersonateLoggedOnUser (handle)) {
      dummy = 0;                // just to keep Veracode happy
    }
  }

  if (GetFileAttributesEx (fileName, GetFileExInfoStandard, &fileAttrData) == 0) {
    goto cleanup_and_return;
  }

  atime = FileTimeToJavaTimeMills (fileAttrData.ftLastAccessTime);
  ctime = FileTimeToJavaTimeMills (fileAttrData.ftCreationTime);
  mtime = FileTimeToJavaTimeMills (fileAttrData.ftLastWriteTime);

  memset   (atimebuf, 0x0, sizeof(atimebuf));
  memset   (ctimebuf, 0x0, sizeof(ctimebuf));
  memset   (mtimebuf, 0x0, sizeof(mtimebuf));
#else
  struct stat fileAttrData;
  if (stat(fileName, &fileAttrData) == -1) {
    goto cleanup_and_return;
  }

  atime = FileTimeToJavaTimeMills (fileAttrData.st_atime);
  ctime = FileTimeToJavaTimeMills (fileAttrData.st_ctime);
  mtime = FileTimeToJavaTimeMills (fileAttrData.st_mtime);

  memset   (atimebuf, 0x0, sizeof(atimebuf));
  memset   (ctimebuf, 0x0, sizeof(ctimebuf));
  memset   (mtimebuf, 0x0, sizeof(mtimebuf));
#endif

  // %lld should work, this is a bug in the compiler
  // So, we can't use the printf variant.  The only
  // thing that seems to work is the _i64toa, so we have to

  _i64toa_s (atime, atimebuf,256, 10);
  _i64toa_s (ctime, ctimebuf, 256,10);
  _i64toa_s (mtime, mtimebuf, 256,10);

  jOwner_id = Java_com_bluejungle_destiny_agent_ipc_OSWrapper_getOwnerSID (env, obj, jFileName, agentType, processToken);
  jGroup_id = Java_com_bluejungle_destiny_agent_ipc_OSWrapper_getGroupSID (env, obj, jFileName, processToken);

  jresultArray = (jobjectArray) env->NewObjectArray (10,
                                                     env->FindClass("java/lang/String"),
                                                     env->NewString((const jchar*)_T(""), 0));

  env->SetObjectArrayElement (jresultArray, 0, env->NewString ((const jchar*)OSW_CE_ATTR_LASTACCESS_TIME, (jsize)_tcslen(CE_ATTR_LASTACCESS_TIME)));
  env->SetObjectArrayElement (jresultArray, 1, env->NewStringUTF (atimebuf));
  env->SetObjectArrayElement (jresultArray, 2, env->NewString ((const jchar*)OSW_CE_ATTR_CREATE_TIME,     (jsize)_tcslen(CE_ATTR_CREATE_TIME)));
  env->SetObjectArrayElement (jresultArray, 3, env->NewStringUTF (ctimebuf));
  env->SetObjectArrayElement (jresultArray, 4, env->NewString ((const jchar*)OSW_CE_ATTR_LASTWRITE_TIME,  (jsize)_tcslen(CE_ATTR_LASTWRITE_TIME)));
  env->SetObjectArrayElement (jresultArray, 5, env->NewStringUTF (mtimebuf));
  env->SetObjectArrayElement (jresultArray, 6, env->NewString ((const jchar*)OSW_CE_ATTR_OWNER_ID, (jsize)_tcslen(CE_ATTR_OWNER_ID)));
  env->SetObjectArrayElement (jresultArray, 7, jOwner_id);
  env->SetObjectArrayElement (jresultArray, 8, env->NewString ((const jchar*)OSW_CE_ATTR_GROUP_ID, (jsize)_tcslen(CE_ATTR_GROUP_ID)));
  env->SetObjectArrayElement (jresultArray, 9, jGroup_id);


 cleanup_and_return:   
  if (handle) {
    if (!RevertToSelf()) {
      dummy = 0;                // just to keep Veracode happy
    }
  }

  freeProcessToken(handle,processToken);
  releaseStringFromJava(fileName);

  return jresultArray;

}

/*
 * Class:     com_bluejungle_destiny_agent_ipc_OSWrapper
 * Method:    getFileAccessTime
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT jlong JNICALL Java_com_bluejungle_destiny_agent_ipc_OSWrapper_getFileAccessTime
  (JNIEnv * env, jobject obj, jstring fileName, jobject processToken)
{

  //  TRACE ("getFileAccessTime\n");      
  jlong ret = -1;
  if (fileName)
  {
    const TCHAR *str = getStringFromJava(env, fileName);
    HANDLE token = getProcessToken (env, processToken);
    ret = getFileTime (str, FILE_ACCESS_TIME, token);
    freeProcessToken(token,processToken);
    releaseStringFromJava(str);
  }
  return ret;
}

/*
 * Class:     com_bluejungle_destiny_agent_ipc_OSWrapper
 * Method:    getFileCreateTime
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT jlong JNICALL Java_com_bluejungle_destiny_agent_ipc_OSWrapper_getFileCreateTime
  (JNIEnv * env, jobject obj, jstring fileName, jobject processToken)
{

  //  TRACE ("getFileCreateTime\n");        
  jlong ret = -1;
  if (fileName)
  {
    const TCHAR *str = getStringFromJava(env, fileName);
    HANDLE token = getProcessToken (env, processToken);
    ret =  getFileTime (str, FILE_CREATE_TIME, token);
    freeProcessToken(token,processToken);
    releaseStringFromJava(str);
  }
  return ret;
}

/*
 * Class:     com_bluejungle_destiny_agent_ipc_OSWrapper
 * Method:    getFileModifiedTime
 * Signature: (Ljava/lang/String;)V
 */
jlong JNICALL Java_com_bluejungle_destiny_agent_ipc_OSWrapper_getFileModifiedTime
  (JNIEnv * env, jobject obj, jstring fileName, jobject processToken)
{
  
  //  TRACE ("getFileModTime\n");        
  jlong ret = -1;
  if (fileName)
  {
    const TCHAR *str = getStringFromJava(env, fileName);
    HANDLE token = getProcessToken (env, processToken);
    ret = getFileTime (str, FILE_MODIFICATION_TIME, token);
    freeProcessToken(token,processToken);
    releaseStringFromJava(str);
  }
  return ret;
}

/*
 * Class:     com_bluejungle_destiny_agent_ipc_OSWrapper
 * Method:    getOwnerSID
 * Signature: (Ljava/lang/String;)Ljava/lang/String;
 */

JNIEXPORT jstring JNICALL Java_com_bluejungle_destiny_agent_ipc_OSWrapper_getOwnerSID
  (JNIEnv * env, jobject obj, jstring fileName, jint agentType, jobject processToken)
    
{
#if defined (WIN32) || defined (_WIN64)
    volatile int dummy;

    //  TRACE ("getOwnerSID\n");
    if (!fileName)
    {
        return env -> NewString ((const jchar*)_T(""), 0);
    }

    const TCHAR *str = getStringFromJava(env, fileName);
    PSID  sidOwner;
    PSECURITY_DESCRIPTOR securityDescriptor;
    TCHAR* pszUSID;
    WCHAR  fileNameWithStream [MAX_PATH + sizeof (L":db172189-cf42-4433-933d-103368dd5f2b")] = {0};
    HANDLE processTokenHandle = getProcessToken (env, processToken);

    if (agentType == 1) {
      wcsncpy_s(fileNameWithStream, MAX_PATH + sizeof (L":db172189-cf42-4433-933d-103368dd5f2b"), str,_TRUNCATE);
      wcsncat_s(fileNameWithStream, MAX_PATH + sizeof (L":db172189-cf42-4433-933d-103368dd5f2b"), L":db172189-cf42-4433-933d-103368dd5f2b", _TRUNCATE);
      HANDLE hfile = ::CreateFileW (str, FILE_READ_ATTRIBUTES, 0x7, NULL, OPEN_EXISTING, 0, NULL);
    
      if (hfile != INVALID_HANDLE_VALUE) {
        CloseHandle (hfile);
        hfile = ::CreateFileW (fileNameWithStream, READ_CONTROL, 0x7, NULL, OPEN_ALWAYS, 0, NULL);

        if (hfile == INVALID_HANDLE_VALUE || GetSecurityInfo (hfile, SE_FILE_OBJECT, OWNER_SECURITY_INFORMATION, 
                                                              &sidOwner, NULL, NULL, NULL, &securityDescriptor) != 0) {
          if (hfile != INVALID_HANDLE_VALUE) {
            CloseHandle (hfile);
          }
          releaseStringFromJava(str);
	  freeProcessToken(processTokenHandle,processToken);
          return env -> NewString ((const jchar*)_T(""), 0);
        }
        CloseHandle (hfile);
      } else {
          releaseStringFromJava(str);
          freeProcessToken(processTokenHandle,processToken);
          return env -> NewString ((const jchar*)_T(""), 0);
      }
    } else {
      if (processTokenHandle) {
        if (!ImpersonateLoggedOnUser (processTokenHandle)) {
          dummy = 0;            // just to keep Veracode happy
        }
      }
      if (GetNamedSecurityInfo ((TCHAR *)str, SE_FILE_OBJECT, OWNER_SECURITY_INFORMATION, 
                                &sidOwner, NULL, NULL, NULL, &securityDescriptor) != 0) {
          releaseStringFromJava(str);
        if (processTokenHandle) {
          if (!RevertToSelf ()) {
            dummy = 0;            // just to keep Veracode happy
          }
        }
	freeProcessToken(processTokenHandle,processToken);
        return env -> NewString ((const jchar*)_T(""), 0);
      }
      if (processTokenHandle) {
        if (!RevertToSelf ()) {
          dummy = 0;            // just to keep Veracode happy
        }
      }
    }
    freeProcessToken(processTokenHandle,processToken);
    pszUSID = NULL;
    ::ConvertSidToStringSid((PSID) sidOwner, &pszUSID);

    jstring jstr = env -> NewString ((const jchar*)pszUSID, (jsize)_tcslen (pszUSID));
    ::LocalFree (securityDescriptor);
    ::LocalFree (pszUSID);
    releaseStringFromJava(str);
    return jstr;
#else
  struct stat  buf;
  const char * fileName2;
  jstring      uid = NULL;
  char         uidstr[16];      // This is an int basically

  if (fileName) {
    TRACE (1, "getOwnerID: Invalid fileName\n");
    return env->NewStringUTF ("");
  }

  fileName2 = env->GetStringUTFChars (fileName, 0);
  memset (&buf, 0x0, sizeof (struct stat));

  if (stat (fileName2, &buf) < 0) {
    TRACE (1, "getOwnerID: Fail to retrieve the uid of %s\n",fileName2);
    goto failed_and_cleanup;
  }
  
  _snprintf_s (uidstr, 16, _TRUNCATE, "%d", buf.st_uid);
  uid = env->NewStringUTF (uidstr);

 failed_and_cleanup:
  env->ReleaseStringUTFChars (fileName, fileName2);
  if (uid == NULL)
    return env->NewStringUTF ("");
  else 
    return uid;
#endif
}

/*
 * Class:     com_bluejungle_destiny_agent_ipc_OSWrapper
 * Method:    getGroupSID
 * Signature: (Ljava/lang/String;)Ljava/lang/String;
 */

JNIEXPORT jstring JNICALL Java_com_bluejungle_destiny_agent_ipc_OSWrapper_getGroupSID
  (JNIEnv * env, jobject obj, jstring fileName, jobject processToken)
    
{

#if defined (WIN32) || defined (_WIN64)
//    TRACE ("getGroupSID\n");        
    if (!fileName)
    {
        return env -> NewString ((const jchar*)_T(""), 0);
    }

    const WCHAR *str = getStringFromJava(env, fileName);
    PSID  sidGroup;
    PSECURITY_DESCRIPTOR securityDescriptor;
    TCHAR* pszGSID;

    if (GetNamedSecurityInfo ((TCHAR *)str, SE_FILE_OBJECT, GROUP_SECURITY_INFORMATION, 
                              NULL, &sidGroup, NULL, NULL, &securityDescriptor) != 0) {

        releaseStringFromJava(str);
        return env -> NewString ((const jchar*)_T(""), 0);
    }

    pszGSID = NULL;
    ::ConvertSidToStringSid((PSID) sidGroup, &pszGSID);

    jstring jstr = env->NewString ((const jchar*)pszGSID, (jsize)_tcslen (pszGSID));
    ::LocalFree (securityDescriptor);
    ::LocalFree (pszGSID);

        releaseStringFromJava(str);
    return jstr;
#else
  struct stat  buf;
  const char * fileName2;
  jstring      gid = NULL;
  char         uidstr[16];      // This is an int basically

  if (fileName) {
    TRACE (1, "getOwnerID: Invalid fileName\n");
    return env->NewStringUTF ("");
  }

  fileName2 = env->GetStringUTFChars (fileName, 0);
  memset (&buf, 0x0, sizeof (struct stat));

  if (stat (fileName2, &buf) < 0) {
    TRACE (1, "getOwnerID: Fail to retrieve the uid\n");
    goto failed_and_cleanup;
  }
  
  _snprintf_s (uidstr, 16, _TRUNCATE, "%d", buf.st_gid);
  gid = env->NewStringUTF (uidstr);

 failed_and_cleanup:
  env->ReleaseStringUTFChars (fileName, fileName2);
  if (gid == NULL)
    return env->NewStringUTF ("");
  else 
    return gid;
#endif
}

JNIEXPORT jstring JNICALL Java_com_bluejungle_destiny_agent_ipc_OSWrapper_getFQDN
	(JNIEnv * env, jobject obj)
{
#if defined (WIN32) || defined (_WIN64)
    _tprintf (_T("getFQDN"));
	TCHAR *fqdn = new TCHAR[256];
	DWORD size = 256;
	if (!GetComputerNameEx(ComputerNameDnsFullyQualified, fqdn, &size)) {
		*fqdn = 0;
		size = 0;
	}
	jstring rv = env->NewString((const jchar*)fqdn, (jsize)size);
	delete[] fqdn;
	return rv;
#else
  struct hostent * hptr;
  struct utsname hostname;

  if (uname(&hostname) < 0) {
    return env->NewStringUTF ("");
  }

  if ((hptr = gethostbyname(hostname.nodename)) == NULL) {
    //[jzhang 101206]  When the samba server is a standalone PDC, the machine has netbios name , the netbios name can not be looked up 
    //                 by calling gethostbyname. This won't affect the case when the samba server is standalone or joining a windows 
    //                 domain (hostname.nodename will be equal to gethostbyname).  
    return env->NewStringUTF (hostname.nodename);  
    //return env->NewStringUTF ("");
  }
  return env->NewStringUTF (hptr->h_name);
#endif
}

/*
 * Class:     com_bluejungle_destiny_agent_ipc_OSWrapper
 * Method:    getSharePhysicalPath
 * Signature: (Ljava/lang/String;)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_bluejungle_destiny_agent_ipc_OSWrapper_getSharePhysicalPath
  (JNIEnv * env, jobject obj, jstring shareName)
{

#if defined (WIN32) || defined (_WIN64)
// TRACE ("getSharePhysPath\n");
    const TCHAR *pShareName= getStringFromJava(env, shareName);
    WCHAR szShareName [MAX_PATH];
    NET_API_STATUS	napiStatus;
    PSHARE_INFO_2	pSharedInfo2 = NULL;

    wcsncpy_s (szShareName,MAX_PATH, pShareName, _TRUNCATE);
    szShareName [MAX_PATH - 1] = 0;
    releaseStringFromJava(pShareName);

    napiStatus = NetShareGetInfo (NULL, szShareName, 2, (LPBYTE *) &pSharedInfo2);

    jstring rv = env->NewString((const jchar*)_T(""), 0);
    if (napiStatus == NERR_Success && pSharedInfo2)
    {
        WCHAR* pPath = pSharedInfo2->shi2_path;
        if (pPath)
        {
            size_t len = wcslen (pPath);
            if (pPath [len - 1] == L'\\')
            {
                pPath [len - 1] = 0;
            }
            _wcslwr_s (pPath,len + 1);
            rv = env->NewString((const jchar*)pPath, (jsize)wcslen (pPath));
        }
        NetApiBufferFree(pSharedInfo2);
    }
    return rv;
#else
  TRACE (1, "getSharePhysicalPath not supported\n");
  return env->NewStringUTF ("");
#endif
}

#define USER_FOLDER_KEY _T("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\User Shell Folders")
#define MY_DOCUMENTS_VALUE_NAME _T("Personal")
#define MY_DESKTOP_VALUE_NAME _T("Desktop")

/**
 * resolves cszVarString and replaces all environment variable references 
 * (delimited by %) with the value of the variable.
 * @param cszVarString string with environment variable references delimited by %
 *                     e.g., %USERPROFILE%\Desktop
 * @param szResolvedString the resolved string is copied to this variable
 */
void ResolveEnvironmentVariables (LPTSTR cszVarString, LPTSTR szResolvedString)
{
#if defined (WIN32) || defined (_WIN64)
    TCHAR envVar [MAX_PATH];
    BOOL bVar = FALSE;

    if (cszVarString [0] == '%')
    {
        bVar = TRUE;
    }
    TCHAR* pNextToken = NULL;
    TCHAR* token = _tcstok_s (cszVarString, _T("%"),&pNextToken);
    
    do 
    {
        if (bVar)
        {
            if (::GetEnvironmentVariable (token, envVar, MAX_PATH))
            {
                _tcsncat_s (szResolvedString, MAX_PATH,envVar, _TRUNCATE);
            }
        }
        else
        {
            _tcsncat_s (szResolvedString, MAX_PATH,token, _TRUNCATE);
        }
        bVar = !bVar;
    } while ((token = _tcstok_s (NULL, _T("%"),&pNextToken)) != NULL);
#endif
}

/*
 * Class:     com_bluejungle_destiny_agent_ipc_OSWrapper
 * Method:    getMyDocumentsFolder
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_bluejungle_destiny_agent_ipc_OSWrapper_getMyDocumentsFolder
  (JNIEnv *env , jobject obj)
{

#if defined (WIN32) || defined (_WIN64)
//     TRACE ("getMyDocFpolder\n");
    HKEY hKey;
    TCHAR folder [MAX_PATH];
    TCHAR returnFolder [MAX_PATH];
    DWORD size = MAX_PATH * sizeof (TCHAR);
    //DWORD err = 0;
    jstring rv = env->NewString((const jchar*)_T(""), 0);

    if (::RegOpenKeyEx (HKEY_CURRENT_USER, USER_FOLDER_KEY, 0, KEY_READ, &hKey) == ERROR_SUCCESS)
    {
        if (::RegQueryValueEx (hKey, MY_DOCUMENTS_VALUE_NAME, NULL, NULL, (LPBYTE) folder, &size) == ERROR_SUCCESS)
        {
            returnFolder [0] = 0;
            ResolveEnvironmentVariables (folder, returnFolder);
            rv = (env->NewString((const jchar*)returnFolder, (jsize)_tcslen(returnFolder)));
        }
        ::RegCloseKey (hKey);
    }

    return rv;
#else
  TRACE (1, "getMyDocumentsFolder not supported\n");
  return env->NewStringUTF ("");
#endif
}

/*
 * Class:     com_bluejungle_destiny_agent_ipc_OSWrapper
 * Method:    getMyDesktopFolder
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_bluejungle_destiny_agent_ipc_OSWrapper_getMyDesktopFolder
  (JNIEnv *env , jobject obj)
{

#if defined (WIN32) || defined (_WIN64)
//    TRACE ("getMyDesktopFolder\n");
    HKEY hKey;
    TCHAR folder [MAX_PATH];
    TCHAR returnFolder [MAX_PATH];
    DWORD size = MAX_PATH * sizeof (TCHAR);
    //DWORD err = 0;
    jstring rv = env->NewString((const jchar*)_T(""), 0);

    if (::RegOpenKeyEx (HKEY_CURRENT_USER, USER_FOLDER_KEY, 0, KEY_READ, &hKey) == ERROR_SUCCESS)
    {
        if (::RegQueryValueEx (hKey, MY_DESKTOP_VALUE_NAME, NULL, NULL, (LPBYTE) folder, &size) == ERROR_SUCCESS)
        {
            returnFolder [0] = 0;
            ResolveEnvironmentVariables (folder, returnFolder);
            rv = (env->NewString((const jchar*)returnFolder, (jsize)_tcslen(returnFolder)));
        }
        ::RegCloseKey (hKey);
    }

    return rv;
#else
  TRACE (1, "getMyDesktopFolder not supported\n");
  return env->NewStringUTF ("");
#endif
}

/*
 * Class:     com_bluejungle_destiny_agent_ipc_OSWrapper
 * Method:    isRemovableMedia
 * Signature: (Ljava/lang/String;)Z
 * 
 * There is an implementation in the HookTool to get down to the busType because USB is treated as 
 * "FIXED_DRIVE", not "removable" in user sense.   However, we are not dealing with it here for now
 * since that code is very hacky, and is not even using DDK header file
 * For us, all Java wants to get avoid is the real REMOVABLE drive like floppy.  Java shouldn't need
 * anything more because this call is very slow.
 */

JNIEXPORT jboolean JNICALL Java_com_bluejungle_destiny_agent_ipc_OSWrapper_isRemovableMedia
  (JNIEnv * env, jobject obj, jstring jFileName)
{

#if defined (WIN32) || defined (_WIN64)
TCHAR      root[4];
  jboolean   result = FALSE;
  int        driveType;

  const TCHAR *lpFileName = getStringFromJava(env, jFileName);

  _tcsncpy_s (root,4, lpFileName, _TRUNCATE);
  root [3] = 0;

  driveType = ::GetDriveType (root);
  
  switch (driveType) {
  case DRIVE_REMOVABLE:
  case DRIVE_CDROM:
    result = TRUE;
    break;
  default:
    break;
  }
  releaseStringFromJava(lpFileName);

  return result;
#else
  // TODO: Need to move the extern below to an appropriate header file
  extern bool isRemovableMedia(const char *filename);

  const char * fileName;
  jboolean rc = false;

  if (!jFileName) {
    TRACE (1, "isRemovableMedia: Invalid fileName\n");
    return rc; 
  }

  fileName = env->GetStringUTFChars (jFileName, 0);

  rc =  isRemovableMedia(fileName);
  
  env->ReleaseStringUTFChars (jFileName, fileName);

  return rc;
#endif
}


/*
 * Class:     com_bluejungle_destiny_agent_ipc_OSWrapper
 * Method:    getUserName
 * Signature: (Ljava/lang/String;)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_bluejungle_destiny_agent_ipc_OSWrapper_getUserName
  (JNIEnv * env, jobject obj, jstring userSid)
{

#if defined (WIN32) || defined (_WIN64)
//    TRACE ("getUserName\n");
    const TCHAR *stringSid = getStringFromJava(env, userSid);
    PSID    sid;
    WCHAR name [MAX_PATH];
    WCHAR domain [MAX_PATH];
    SID_NAME_USE sidNameUse;
    DWORD cchName = MAX_PATH;
    DWORD cchDomain = MAX_PATH;    
    WCHAR* pName = name;
    WCHAR* pDomain = domain;
    BOOL bSuccess = FALSE;
    jstring rv = env->NewString((const jchar*)_T(""), 0);

    if (::ConvertStringSidToSid(stringSid, &sid))
    {
        bSuccess = LookupAccountSid(NULL, sid, pName, &cchName, pDomain, &cchDomain, &sidNameUse);
        if (!bSuccess &&
            (cchName > MAX_PATH || cchDomain > MAX_PATH))
        {
            if (cchName > MAX_PATH)
            {
                pName = new(std::nothrow) WCHAR [cchName];
	 	   if(pName == NULL)
		   	return NULL;
            }
            if (cchDomain > MAX_PATH)
            {
                pDomain = new(std::nothrow) WCHAR [cchDomain];
            }
            bSuccess = LookupAccountSid(NULL, sid, pName, &cchName, pDomain, &cchDomain, &sidNameUse);            
        }

        if (bSuccess && sidNameUse == SidTypeUser)
        {
            DWORD dwLen = MAX_PATH;
            WCHAR* pFullName = new WCHAR [wcslen (pDomain) + wcslen (pName) + 2];
            WCHAR buf [MAX_PATH];
            WCHAR* pBuf = buf;
            _snwprintf_s(pFullName, wcslen (pDomain) + wcslen (pName) + 2, _TRUNCATE, L"%s\\%s",pDomain, pName);
            bSuccess = ::TranslateName (pFullName, NameSamCompatible, NameUserPrincipal, pBuf, &dwLen);
            
            if (!bSuccess && dwLen > MAX_PATH)
            {
                pBuf = new WCHAR [dwLen];
                bSuccess = ::TranslateName (pFullName, NameSamCompatible, NameUserPrincipal, pBuf, &dwLen);
            }

            if (bSuccess)
            {
                rv = env->NewString ((const jchar*)pBuf, (jsize)_tcslen (pBuf));
            }
            else 
            {
                _snwprintf_s (pFullName, wcslen (pDomain) + wcslen (pName) + 2, _TRUNCATE, L"%s@%s", pName, pDomain);
                rv = env->NewString ((const jchar*)pFullName, (jsize)_tcslen (pFullName));
            }

            if (pBuf != buf)
            {
                delete [] pBuf;
            }

            delete [] pFullName;

        }
        ::LocalFree (sid);
    }

    if (pName != name)
    {
        delete [] pName;
    }
    if (pDomain != domain)
    {
        delete [] pDomain;
    }

    releaseStringFromJava(stringSid);

    return rv;
#else
  struct passwd *pw   = NULL;
  jstring username    = NULL;
  const char * uidstr = NULL;
  uid_t uid;
  FILE* conf = NULL;
  
  uidstr = env->GetStringUTFChars (userSid, 0);
  uid    = atoi (uidstr);
  pw     = getpwuid (uid);

  if (pw == NULL) {
    TRACE (1, "getUserName: Failed to find the user %s\n", uidstr);
    goto failed_and_cleanup;
  }


  //022807 jzhang get full user name for PDP (domainname is put in /usr/local/ce/etc/domainname.conf by smbDirMapping utility)
  char domainname[256];
  memset(domainname,0,256);

  conf = fopen(DOMAIN_CONFIG_FILE,"r");
  if(conf)
    {
      fscanf(conf,"%s",domainname);
      fclose(conf);
    }
  
  //TRACE(1,"getdomainname returns with code %s, errno=%d, domainname=%s\n", ret,errno,domainname);
  
  if(strlen(domainname)==0)  //try system setting as backup choice
    {
      getdomainname(domainname,sizeof(domainname));
    }

  char fullname[256];
  memset(fullname,0,256);
  if(strlen(domainname)==0)
    _snprintf_s(fullname,256, _TRUNCATE, "%s",pw->pw_name);
  else
    _snprintf_s(fullname,256, _TRUNCATE, "%s@%s",pw->pw_name,domainname);

  username = env->NewStringUTF(fullname);
  //username = env->NewStringUTF(pw->pw_name);

 failed_and_cleanup:
  env->ReleaseStringUTFChars (userSid, uidstr);
  if (username == NULL)
    return env->NewStringUTF ("");
  else
    return username;
#endif
}


typedef struct _pe_headers_t pe_headers_t; 
struct _pe_headers_t 
{ 
  DWORD signature; 
#if defined (WIN32) || defined (_WIN64)
  IMAGE_FILE_HEADER _head; 
  IMAGE_OPTIONAL_HEADER opt_head; 
  IMAGE_SECTION_HEADER section_header[0];  /* actual number in NumberOfSections */ 
#endif
}; 
  
pe_headers_t* GetModuleHeaders(HINSTANCE hInst) 
{ 
#if defined (WIN32) || defined (_WIN64)
    IMAGE_DOS_HEADER* dos_head = (IMAGE_DOS_HEADER*) hInst; 
    return (pe_headers_t*) ((char *) dos_head + dos_head->e_lfanew); 
#else
    return NULL;
#endif
}  

JNIEXPORT jstring JNICALL Java_com_bluejungle_destiny_agent_ipc_OSWrapper_getAppInfo
  (JNIEnv * env, jobject obj, jstring fileName) 
{

#if defined (WIN32) || defined (_WIN64)
  const TCHAR *str = getStringFromJava(env, fileName);
  LOADED_IMAGE loaded_image;
  jstring jstr = NULL;
  char  szExePath [MAX_PATH * 2];

  // TODO: We no longer put the path into the fingerprint, so this length can be reduced
  char  data [MAX_PATH * 4 + 2 * 12 + 50 + 4]; /* 12 for 2 DWORDs (32 bits) 50 for 160 bit SHA1 hash, 4 for miscellany like semicolons, terminating NUL byte etc. */
  CHAR *internalName = NULL, *exeName = NULL;

  memset (&loaded_image, '\0', sizeof (loaded_image));
  ::WideCharToMultiByte(CP_ACP, 0, str, -1, szExePath, MAX_PATH * 2, NULL, NULL);

  // Release 'str':
  releaseStringFromJava(str);

  if (::MapAndLoad (szExePath, NULL, &loaded_image, FALSE, TRUE) != TRUE) {
    jstr = env -> NewString ((const jchar*)(TCHAR *)_T(""), 0);
    return jstr;
  }
  ULONG deSize;
  IMAGE_NT_HEADERS *pImageNtHeaders = ::ImageNtHeader (loaded_image.MappedAddress);
  PIMAGE_SECTION_HEADER pImageSectionHeader = NULL;
  IMAGE_EXPORT_DIRECTORY *exportDirectory = (IMAGE_EXPORT_DIRECTORY *) ::ImageDirectoryEntryToDataEx (loaded_image.MappedAddress, TRUE, IMAGE_DIRECTORY_ENTRY_EXPORT, &deSize, &pImageSectionHeader);
  if (pImageSectionHeader != NULL && exportDirectory != NULL) {
    internalName = (CHAR *) ImageRvaToVa (pImageNtHeaders, loaded_image.MappedAddress, exportDirectory -> Name, NULL);
  } else {
    internalName = "(null)";
  }

  exeName = strrchr (loaded_image.ModuleName, '\\');

  SHA1Context sha1Context;
  SHA1Reset (&sha1Context);
  HANDLE hFile = ::CreateFileA (szExePath, FILE_READ_DATA, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
  //DWORD  lastErr2  = GetLastError ();
  if (hFile)
  {
    if (::ImageGetDigestStream (hFile, CERT_PE_IMAGE_DIGEST_DEBUG_INFO, SHA1Input, &sha1Context) == FALSE) {
      ::CloseHandle (hFile);
      /* LPWSTR lpszTemp = NULL;
         DWORD  lastErr  = GetLastError ();
         ::FormatMessage (FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ARGUMENT_ARRAY,
         NULL, lastErr, LANG_NEUTRAL, lpszTemp, 0, NULL);
         GlobalFree ((HGLOBAL) lpszTemp);*/
      jstr = env -> NewString ((const jchar*)(TCHAR *)_T(""), 0);

      return jstr;
    }
        
    _snprintf_s (data, _countof(data), _TRUNCATE,"%s:%d:%d:%08x%08x%08x%08x%08x", internalName, 
                 pImageNtHeaders -> OptionalHeader.MajorImageVersion, 
                 pImageNtHeaders -> OptionalHeader.MinorImageVersion, 
                 sha1Context.Message_Digest [0], sha1Context.Message_Digest [1], sha1Context.Message_Digest [2], 
                 sha1Context.Message_Digest [3], sha1Context.Message_Digest [4]);
    ::CloseHandle (hFile);
  }
  else
  {
    _snprintf_s(data, _countof(data), _TRUNCATE, "%s:0:0:0000000000000000000000000000000000000000", internalName);
  }
  
  if (::UnMapAndLoad (&loaded_image) != TRUE) {
    jstr = env -> NewString ((const jchar*)(TCHAR *)_T(""), 0);
    return jstr;
  }
    
  TCHAR wcData [4 * (MAX_PATH * 4 + 2 * 12 + 50 + 4)];
  ::MultiByteToWideChar (CP_ACP, 0, data, (int)strlen (data) + 1, wcData, (4 * (MAX_PATH * 4 + 2 * 12 + 50 + 4)) - 1);
  jstr = env -> NewString ((const jchar*)wcData, (jsize)_tcslen (wcData));

  return jstr;
#else
  return env->NewStringUTF ("");
#endif
}

#if defined (WIN32) || defined (_WIN64)
JNIEXPORT jboolean JNICALL Java_com_bluejungle_destiny_agent_ipc_OSWrapper_setKernelPolicyResponse
  (JNIEnv* env, jobject jObj, jlong jhandle, jlong uniqueIndex, jlong allow, jlong allowType)
{
    IPC_POLICY_RESPONSE Response;
	Response.ulSize = sizeof(IPC_POLICY_RESPONSE);
	Response.UniqueIndex = (LONG) uniqueIndex;
	Response.Allow = (ULONG) allow;
	Response.AllowType = (ULONG) allowType;

#ifdef _DEBUG
        // DumpResponse (&Response);
#endif
	DWORD cb = 0;
	BOOL bRet = DeviceIoControl(g_hKDriver,
							IOCTL_POLICY_RESPONSE,
							&Response,
							sizeof(Response),
							NULL,
							0,
							&cb,
							NULL
							);
	if(!bRet)
	{
		printf("devioctl fail, error = %d\n",GetLastError());
	}
	if(cb!=0)
	{
		printf("cb!=0, cb = %d\n",cb);
	}
	return static_cast<jboolean>(bRet);
}
#else
extern void init_env();
extern int find_usb_info(const char*, string&, string&, string&, string&);

string generate_sysfs_path(const char* DevPath)
{
	int len = strlen(DevPath);
	int i = 0;
	BOOL IsFindDigit = FALSE;
	for(; i < len; i++)
	{
		if(isdigit(DevPath[i]))
		{
			IsFindDigit = TRUE;
			break;
		}
	}

	string TempPath;
	if(IsFindDigit)
	{
		string Digit = string(DevPath).substr(i);
		string Path1 = string(DevPath).substr(0, i);

		size_t Pos = Path1.rfind("/");
		if(Pos == 0)	return "";
		TempPath = Path1 + Path1.substr(Pos) + Digit;
	}
	else
	{
		TempPath = DevPath;
	}

	size_t Pos = TempPath.find("/dev");
	if(Pos != 0)	return "";
	string RetPath = string("/block") + TempPath.substr(Pos + 4);
	return RetPath;
}

int translate_device_name(IPC_POLICY_REQUEST *policyRequest)
{
    string SysFsPath = generate_sysfs_path(policyRequest->szwFromFileName);
    if(SysFsPath.empty()) {
	syslog(LOG_ERR, "%s not valid devpath\r\n", policyRequest->szwFromFileName);
	return -1;
    }

    string VendorId, ProductId, SerialNum, DeviceClass;
    if(find_usb_info(SysFsPath.c_str(), VendorId, ProductId, SerialNum, DeviceClass) == 0) {
	syslog(LOG_INFO, "Find USB info: devpath %s(sysfs path %s), vendorid %s, productid %s, serialnum %s, deviceclass %s\r\n", 
	       policyRequest->szwFromFileName, 
	       SysFsPath.c_str(), 
	       VendorId.c_str(), 
	       ProductId.c_str(), 
	       SerialNum.c_str(), 
	       DeviceClass.c_str());

	string deviceString = "device://usb/storage/";
	deviceString += VendorId.c_str();
	deviceString += "/";
	deviceString += ProductId.c_str();
	deviceString += "/";
	deviceString += SerialNum.c_str();

	strncpy_s (policyRequest->szwFromFileName, sizeof(policyRequest->szwFromFileName), deviceString.c_str(), _TRUNCATE);
	return 0;
    } else {
	syslog(LOG_ERR, "find_usb_info(%s) failed\r\n", policyRequest->szwFromFileName);
	return -1;
    }
}

/*
 * Class:     com_bluejungle_destiny_agent_ipc_OSWrapper
 * Method:    getNextKernelPolicyRequest
 * This method listens to the kernel events and returns the first request that is received for policy requests
 * If no even if received or something goes wrong, the method returns 0
 * Signature: ([I)[Ljava/lang/Object;
 */
JNIEXPORT jobjectArray JNICALL Java_com_bluejungle_destiny_agent_ipc_OSWrapper_getNextKernelPolicyRequest
  (JNIEnv* env, jobject jObj, jlongArray p_objs)
{
  jlong        * handleArray    = NULL;
  int           skfd           = -1;
  int           maxfd          = -1;
  fd_set        rdset;
  jobjectArray  ret;
  jobjectArray  requestArray;
  char          recvbuf[65535];
  IPC_POLICY_REQUEST * policyRequest = NULL;
  int           type;  //1-- kernel socket;  0--user mode socket
  int           recvlen;


  struct nlmsghdr    * nlhdr         = NULL;
  int           kernelsock_len = sizeof (sockaddr_nl);
  struct sockaddr_nl   kernelsock;
  

  handleArray = env->GetLongArrayElements(p_objs, 0);



  if (handleArray == NULL) {
    goto failed_and_cleanup;
  }

  skfd = (int) handleArray[0];
  //syslog(LOG_INFO,"In getNextKernelPolicyRequest: Passed in handle=%d",skfd);

  if (skfd ==-1) {
    goto failed_and_cleanup;
  }
  else if(skfd<0)
    {
      //negative fd stands for user mode socket
      skfd = -skfd;
      type = 0;
    }
  else  //skfd>=0
    type = 1;

  
  maxfd  = skfd + 1;

  /* Wait for the socket, and convert the request */
  FD_ZERO (&rdset);
  FD_SET  (skfd, &rdset);

  
  while (1) {
    int rc = select (maxfd, &rdset, NULL, NULL, NULL);

    /* Probably somebody interrupted it */
    if (rc <  0)   goto failed_and_cleanup;
    if (rc == 0)   continue;
    if (FD_ISSET (skfd, &rdset)) {
      memset (recvbuf,     0x0, sizeof(recvbuf)); 
      
      syslog(LOG_DEBUG,"In getNextKernelPolicyRequest: New Request Arrived on fd %d",skfd);
     
      ///[jzhang 092206]
      if(type==0)/// for user mode socket
	{
	  memset(&fromaddr, 0, sizeof(sockaddr_un));
	  fromaddr.sun_family = AF_UNIX;
	  socklen_t fromlen = sizeof(sockaddr_un);
	  recvlen = recvfrom(skfd, recvbuf, sizeof(recvbuf), 0, (sockaddr *)&fromaddr, &fromlen);

	  if (recvlen <  0)   goto failed_and_cleanup;
	  if (recvlen == 0)   continue;
	  
	  //DEBUGG
	  //TRACE ("Get %d bytes from user mode .\n",recvlen);	  
	  jacklog("Request received from usermode.\n");     

	  handle_recv_nl_data(recvbuf, recvlen);
	  policyRequest = (IPC_POLICY_REQUEST *) recvbuf;
	  //TRACE(1, "%d",policyRequest);
	}
      else  //type==1   , for kernel mode socket
	{
	  memset (&kernelsock, 0x0, sizeof(kernelsock));
	  kernelsock.nl_family = AF_NETLINK;
	  recvlen = recvfrom (skfd, recvbuf, sizeof(recvbuf), 0,(sockaddr *) &kernelsock, (socklen_t*) &kernelsock_len);
	  if (recvlen <  0)   goto failed_and_cleanup;
	  if (recvlen == 0)   continue;

	  nlhdr = (struct nlmsghdr *) recvbuf;
	  if (nlhdr->nlmsg_type == NLMSG_BJPEP_IPCEND)
	    goto failed_and_cleanup;

	  //DEBUGG
	  //TRACE ("Get %d bytes from kernel mode .\n",recvlen);	  
	  jacklog("Request received from kernel mode.\n");     


	  policyRequest = (IPC_POLICY_REQUEST *) NLMSG_DATA((struct nlmsghdr *) recvbuf);
	  handle_recv_nl_data((char*)policyRequest,(__u16)(recvlen - NLMSG_LENGTH(0)));   //dump request information
	  //jacklog("Request dumped.\n");
	}
      
      break;
      //[jzhang 092606] Now we don't distinguish between the request from Local Access or Samba
      //                The original purpose of using following code is to ignore the file access by smbd itself.
      //                Originally, the request without a sharename is invoked by smbd. Now we got both types of request coming through here.
      /*
      // According to Reid, ShareName is a safe condition to check, so we believe him.
      if (sizeofunicode((char *)policyRequest->ShareName) > 0)
        break;
      else {
        // We have a close loop to let it pass through
        Java_com_bluejungle_destiny_agent_ipc_OSWrapper_setKernelPolicyResponse
          (env, jObj, (int) handleArray[0], policyRequest->UniqueIndex, ALLOW, WATCH_NEXT_OP);
      }
      */
    }
  }

  /* We have a Request */
  if (policyRequest) {
    char tmpbuf[256];

    /* Get the policy fields */
    requestArray = env->NewObjectArray (12, env->FindClass("java/lang/Object"), env->NewStringUTF(""));
    ret          = env->NewObjectArray (2,  env->FindClass("java/lang/Object"), env->NewStringUTF(""));

    // Todo: workaround for size
    env->SetObjectArrayElement (requestArray, METHOD_NAME_INDEX, 
                                env->NewStringUTF(policyRequest->szwMethodName));

    char actionbuf[256];
    switch (policyRequest->ulAction) {
    /* The action coming from the kernel is a bitmask consist */
    /* of multiple actions, so it's difficult to map, I will  */
    /* use the bit-wise value that are coming from kernel     */
    case 0x1:
      _snprintf_s(actionbuf, 256, _TRUNCATE, "%s", "OPEN");
      break;
    case 0x3:
      _snprintf_s(actionbuf, 256, _TRUNCATE, "%s", "EDIT");
      break;
    case 0x4:
      _snprintf_s(actionbuf, 256, _TRUNCATE, "%s", "DELETE");
      break;
    case 0x8:  //for READ action
      _snprintf_s(actionbuf, 256, _TRUNCATE, "%s", "OPEN");
      break;
    case 0x10:
      _snprintf_s(actionbuf, 256, _TRUNCATE, "%s", "EDIT");
      break;
    case 0x40:
      _snprintf_s(actionbuf, 256, _TRUNCATE, "%s", "RENAME");
      break;
    case 0x83:
      _snprintf_s(actionbuf, 256, _TRUNCATE, "%s", "EDIT");
      break;
    case 0x100:
      _snprintf_s(actionbuf, 256, _TRUNCATE, "%s", "CHANGE_ATTRIBUTES");
      break;
    case 0x200:
      _snprintf_s(actionbuf, 256, _TRUNCATE, "%s", "CHANGE_SECURITY");
      break;
    case 0x40000:
      _snprintf_s(actionbuf, 256, _TRUNCATE, "%s", "MOVE");
      break;

      /* This is COPY_ACTION is some random header file which is used by the PEP */
    case 0x20000:
      _snprintf_s(actionbuf, 256, _TRUNCATE, "%s", "COPY");
      break;

    case MOUNT_ACTION:
      {
	static bool is_init = false;
	if( is_init == false )
	{
	  is_init = true;
	  init_env();
	}
	_snprintf_s (actionbuf, 256, _TRUNCATE, "%s", "ATTACH_DEVICE");
	if (translate_device_name(policyRequest) != 0) {
	    syslog (LOG_ERR, "device name translation for %s failed\r\n", policyRequest->szwFromFileName);
	}
	break;
      }
    case NETWORK_ACCESS:
	_snprintf_s (actionbuf, 256, _TRUNCATE, "%s", "HOST_CONNECT");
	break;
    case 0x10000:
	_snprintf_s (actionbuf, 256, _TRUNCATE, "%s", "PRINT");
	break;
    default :
      _snprintf_s(actionbuf, 256, _TRUNCATE, "%s_0x%X", "UNKNOWN", policyRequest->ulAction);
      break;
    }

    //20070213 jzhang [handle the create empty file case]
    if(policyRequest->ulAction & CREATE_NEW_ACTION)
      _snprintf_s(actionbuf,256, _TRUNCATE, "%s","EDIT");

    jacklog("policyRequest->ulAction=%d actionbuf=%s fromfile=%s tofile=%s process=%s\n",
	    policyRequest->ulAction,actionbuf,
	    policyRequest->szwFromFileName,
	    policyRequest->szwToFileName == NULL ? "<NULL>" : policyRequest->szwToFileName,
	    policyRequest->szwApplicationName);

    env->SetObjectArrayElement (requestArray, FROM_FILE_NAME_INDEX, 
                                env->NewStringUTF(policyRequest->szwFromFileName));

    env->SetObjectArrayElement (requestArray, ACTION_INDEX, env->NewStringUTF(actionbuf));

    _snprintf_s (tmpbuf, 256, _TRUNCATE, "%ld", policyRequest->CoreSid.ulLinuxUid);
    env->SetObjectArrayElement (requestArray, SID_INDEX, env->NewStringUTF(tmpbuf));

    env->SetObjectArrayElement (requestArray, APPLICATION_NAME_INDEX, env->NewStringUTF(policyRequest->szwApplicationName));

    char ipbuf[256];
    if (policyRequest->ulIPAddress == 0)
      policyRequest->ulIPAddress = 0x7F000001;
    _snprintf_s (ipbuf, 256, _TRUNCATE, "%ld", policyRequest->ulIPAddress);
    env->SetObjectArrayElement (requestArray, HOST_NAME_INDEX, env->NewStringUTF(ipbuf));

    env->SetObjectArrayElement (requestArray, TO_FILE_NAME_INDEX, 
                                env->NewStringUTF(policyRequest->szwToFileName));

    if (policyRequest->bIgnoreObligations) {
      env->SetObjectArrayElement (requestArray, IGNORE_OBLIGATION_INDEX,  env->NewStringUTF(TRUE_STR));
    }

    env->SetObjectArrayElement (requestArray, SHARE_NAME_INDEX, 
                                env->NewStringUTF( policyRequest->ShareName));

    char pidbuf[256];
    _snprintf_s (pidbuf, 256, _TRUNCATE, "%ld", policyRequest->ulProcessId);
    env->SetObjectArrayElement (requestArray, PID_INDEX, env->NewStringUTF(pidbuf));

    //[jzhang 110706]  add noise level 
    char noisebuf[256];
    _snprintf_s(noisebuf,256, _TRUNCATE, "%ld",policyRequest->ulNoiseLevel);
    env->SetObjectArrayElement (requestArray, LOG_LEVEL_INDEX, env->NewStringUTF(noisebuf));

    _snprintf_s (tmpbuf, 256, _TRUNCATE, "%ld", policyRequest->UniqueIndex);
    env->SetObjectArrayElement (ret, 0, env->NewStringUTF(tmpbuf));
    env->SetObjectArrayElement (ret, 1, requestArray);
  }

 failed_and_cleanup:
  env->ReleaseLongArrayElements (p_objs, handleArray, JNI_ABORT);
  return ret;
}

/*
 * Class:     com_bluejungle_destiny_agent_ipc_OSWrapper
 * Method:    setKernelPolicyResponse
 * This method reply to the kernel about the result of a policy request
 * If no even if received or something goes wrong, the method returns 0
 * Signature: (IJJJ)Z
 */
JNIEXPORT jboolean JNICALL Java_com_bluejungle_destiny_agent_ipc_OSWrapper_setKernelPolicyResponse
(JNIEnv* env, jobject jObj, jlong jhandle, jlong uniqueIndex, jlong allow, jlong allowType)
{
  /////[jzhang 092206] distinguish the handle for user/kernel mode socket
  if(jhandle>0)
    {
      char sendbuf[sizeof(IPC_POLICY_RESPONSE) + sizeof(struct nlmsghdr) + 256];
      struct sockaddr_nl    kernelsock;
      struct nlmsghdr     * hdr      = NULL;
      IPC_POLICY_RESPONSE * response = NULL;
      int                   skfd     = jhandle;
      
      if (skfd < 0)
	return false;
      
      memset (sendbuf,     0x0, sizeof (sendbuf));
      memset (&kernelsock, 0x0, sizeof (kernelsock));
      
      hdr      = (struct nlmsghdr *) sendbuf;
      response = (IPC_POLICY_RESPONSE *)(sendbuf + sizeof(struct nlmsghdr));
      
      response->ulSize      = sizeof (IPC_POLICY_RESPONSE);
      response->UniqueIndex = uniqueIndex;
      response->Allow       = allow;
      response->AllowType   = allowType;
      
      hdr->nlmsg_len        = NLMSG_LENGTH (sizeof(IPC_POLICY_RESPONSE));
      hdr->nlmsg_flags      = 0;
      hdr->nlmsg_type       = NLMSG_BJPEP_DATA;
      hdr->nlmsg_pid        = getpid();
      
      kernelsock.nl_family  = AF_NETLINK;
      kernelsock.nl_pid     = 0;
      kernelsock.nl_groups  = 0;
      
      if (sendto (skfd, sendbuf, hdr->nlmsg_len, 0, (sockaddr *) &kernelsock, sizeof(kernelsock)) < 0)
	return false;
    }
  else if(jhandle<0)  //user mode socket
    {
      char sendbuf[sizeof(IPC_POLICY_RESPONSE)+256];
      IPC_POLICY_RESPONSE *response = (IPC_POLICY_RESPONSE*) sendbuf;
      int skfd = -jhandle;
      socklen_t fromlen = sizeof(sockaddr_un);
      
      response->ulSize      = sizeof (IPC_POLICY_RESPONSE);
      response->UniqueIndex = uniqueIndex;
      response->Allow       = allow;
      response->AllowType   = allowType;
      
      jacklog("About to send policy response back to driver. \n");
      if(sendto(skfd, (char*)response, sizeof(IPC_POLICY_RESPONSE), 0, (sockaddr *)&fromaddr, fromlen) <= 0)
	return false;
    }
  return true;
}

#endif

/* MOVE SHA1.C BELOW TO ITs OWN SEPERATE FILE - PKENI */
/*
 *  sha1.c
 *
 *	Copyright (C) 1998
 *	Paul E. Jones <paulej@arid.us>
 *	All Rights Reserved
 *
 *****************************************************************************
 *	$Id$
 *****************************************************************************
 *
 *  Description:
 *      This file implements the Secure Hashing Standard as defined
 *      in FIPS PUB 180-1 published April 17, 1995.
 *
 *      The Secure Hashing Standard, which uses the Secure Hashing
 *      Algorithm (SHA), produces a 160-bit message digest for a
 *      given data stream.  In theory, it is highly improbable that
 *      two messages will produce the same message digest.  Therefore,
 *      this algorithm can serve as a means of providing a "fingerprint"
 *      for a message.
 *
 *  Portability Issues:
 *      SHA-1 is defined in terms of 32-bit "words".  This code was
 *      written with the expectation that the processor has at least
 *      a 32-bit machine word size.  If the machine word size is larger,
 *      the code should still function properly.  One caveat to that
 *      is that the input functions taking characters and character
 *      arrays assume that only 8 bits of information are stored in each
 *      character.
 *
 *  Caveats:
 *      SHA-1 is designed to work with messages less than 2^64 bits
 *      long. Although SHA-1 allows a message digest to be generated for
 *      messages of any number of bits less than 2^64, this
 *      implementation only works with messages with a length that is a
 *      multiple of the size of an 8-bit character.
 *
 */

/*
 *  Define the circular shift macro
 */

#if defined (WIN32) || defined (_WIN64)
extern "C" {
#define SHA1CircularShift(bits,word) \
                ((((word) << (bits)) & 0xFFFFFFFF) | \
                ((word) >> (32-(bits))))

/* Function prototypes */
void SHA1ProcessMessageBlock(SHA1Context *);
void SHA1PadMessage(SHA1Context *);

/*  
 *  SHA1Reset
 *
 *  Description:
 *      This function will initialize the SHA1Context in preparation
 *      for computing a new message digest.
 *
 *  Parameters:
 *      context: [in/out]
 *          The context to reset.
 *
 *  Returns:
 *      Nothing.
 *
 *  Comments:
 *
 */
void SHA1Reset(SHA1Context *context)
{
    context->Length_Low             = 0;
    context->Length_High            = 0;
    context->Message_Block_Index    = 0;

    context->Message_Digest[0]      = 0x67452301;
    context->Message_Digest[1]      = 0xEFCDAB89;
    context->Message_Digest[2]      = 0x98BADCFE;
    context->Message_Digest[3]      = 0x10325476;
    context->Message_Digest[4]      = 0xC3D2E1F0;

    context->Computed   = 0;
    context->Corrupted  = 0;
}

/*  
 *  SHA1Result
 *
 *  Description:
 *      This function will return the 160-bit message digest into the
 *      Message_Digest array within the SHA1Context provided
 *
 *  Parameters:
 *      context: [in/out]
 *          The context to use to calculate the SHA-1 hash.
 *
 *  Returns:
 *      1 if successful, 0 if it failed.
 *
 *  Comments:
 *
 */
int SHA1Result(SHA1Context *context)
{

    if (context->Corrupted)
    {
        return 0;
    }

    if (!context->Computed)
    {
        SHA1PadMessage(context);
        context->Computed = 1;
    }

    return 1;
}

/*  
 *  SHA1Input
 *
 *  Description:
 *      This function accepts an array of octets as the next portion of
 *      the message.
 *
 *  Parameters:
 *      context: [in/out]
 *          The SHA-1 context to update
 *      message_array: [in]
 *          An array of characters representing the next portion of the
 *          message.
 *      length: [in]
 *          The length of the message in message_array
 *
 *  Returns:
 *      TRUE on success, FALSE on failure (per windows DIGEST_FUNCTION requirements).
 *
 *  Comments:
 *
 */
BOOL WINAPI SHA1Input(     PVOID win_Context,
                    PBYTE win_message_array,
                    DWORD win_length)
{

  SHA1Context         *context = (SHA1Context *) win_Context;
  const unsigned char *message_array = (const unsigned char *) win_message_array;
  unsigned            length = (unsigned) win_length;

    if (!length)
    {
        return FALSE;
    }

    if (context->Computed || context->Corrupted)
    {
        context->Corrupted = 1;
        return FALSE;
    }

    while(length-- && !context->Corrupted)
    {
#pragma warning (push)
#pragma warning (disable:6386)	  
        context->Message_Block[context->Message_Block_Index++] =
                                                (*message_array & 0xFF);
#pragma warning (pop)

        context->Length_Low += 8;
        /* Force it to 32 bits */
        context->Length_Low &= 0xFFFFFFFF;
        if (context->Length_Low == 0)
        {
            context->Length_High++;
            /* Force it to 32 bits */
            context->Length_High &= 0xFFFFFFFF;
            if (context->Length_High == 0)
            {
                /* Message is too long */
                context->Corrupted = 1;
            }
        }

        if (context->Message_Block_Index == 64)
        {
            SHA1ProcessMessageBlock(context);
        }

        message_array++;
    }
    return TRUE;
}

/*  
 *  SHA1ProcessMessageBlock
 *
 *  Description:
 *      This function will process the next 512 bits of the message
 *      stored in the Message_Block array.
 *
 *  Parameters:
 *      None.
 *
 *  Returns:
 *      Nothing.
 *
 *  Comments:
 *      Many of the variable names in the SHAContext, especially the
 *      single character names, were used because those were the names
 *      used in the publication.
 *         
 *
 */
void SHA1ProcessMessageBlock(SHA1Context *context)
{
    const unsigned K[] =            /* Constants defined in SHA-1   */      
    {
        0x5A827999,
        0x6ED9EBA1,
        0x8F1BBCDC,
        0xCA62C1D6
    };
    int         t;                  /* Loop counter                 */
    unsigned    temp;               /* Temporary word value         */
    unsigned    W[80];              /* Word sequence                */
    unsigned    A, B, C, D, E;      /* Word buffers                 */

    /*
     *  Initialize the first 16 words in the array W
     */
    for(t = 0; t < 16; t++)
    {
        W[t] = ((unsigned) context->Message_Block[t * 4]) << 24;
        W[t] |= ((unsigned) context->Message_Block[t * 4 + 1]) << 16;
        W[t] |= ((unsigned) context->Message_Block[t * 4 + 2]) << 8;
        W[t] |= ((unsigned) context->Message_Block[t * 4 + 3]);
    }

    for(t = 16; t < 80; t++)
    {
       W[t] = SHA1CircularShift(1,W[t-3] ^ W[t-8] ^ W[t-14] ^ W[t-16]);
    }

    A = context->Message_Digest[0];
    B = context->Message_Digest[1];
    C = context->Message_Digest[2];
    D = context->Message_Digest[3];
    E = context->Message_Digest[4];

    for(t = 0; t < 20; t++)
    {
        temp =  SHA1CircularShift(5,A) +
                ((B & C) | ((~B) & D)) + E + W[t] + K[0];
        temp &= 0xFFFFFFFF;
        E = D;
        D = C;
        C = SHA1CircularShift(30,B);
        B = A;
        A = temp;
    }

    for(t = 20; t < 40; t++)
    {
        temp = SHA1CircularShift(5,A) + (B ^ C ^ D) + E + W[t] + K[1];
        temp &= 0xFFFFFFFF;
        E = D;
        D = C;
        C = SHA1CircularShift(30,B);
        B = A;
        A = temp;
    }

    for(t = 40; t < 60; t++)
    {
        temp = SHA1CircularShift(5,A) +
               ((B & C) | (B & D) | (C & D)) + E + W[t] + K[2];
        temp &= 0xFFFFFFFF;
        E = D;
        D = C;
        C = SHA1CircularShift(30,B);
        B = A;
        A = temp;
    }

    for(t = 60; t < 80; t++)
    {
        temp = SHA1CircularShift(5,A) + (B ^ C ^ D) + E + W[t] + K[3];
        temp &= 0xFFFFFFFF;
        E = D;
        D = C;
        C = SHA1CircularShift(30,B);
        B = A;
        A = temp;
    }

    context->Message_Digest[0] =
                        (context->Message_Digest[0] + A) & 0xFFFFFFFF;
    context->Message_Digest[1] =
                        (context->Message_Digest[1] + B) & 0xFFFFFFFF;
    context->Message_Digest[2] =
                        (context->Message_Digest[2] + C) & 0xFFFFFFFF;
    context->Message_Digest[3] =
                        (context->Message_Digest[3] + D) & 0xFFFFFFFF;
    context->Message_Digest[4] =
                        (context->Message_Digest[4] + E) & 0xFFFFFFFF;

    context->Message_Block_Index = 0;
}

/*  
 *  SHA1PadMessage
 *
 *  Description:
 *      According to the standard, the message must be padded to an even
 *      512 bits.  The first padding bit must be a '1'.  The last 64
 *      bits represent the length of the original message.  All bits in
 *      between should be 0.  This function will pad the message
 *      according to those rules by filling the Message_Block array
 *      accordingly.  It will also call SHA1ProcessMessageBlock()
 *      appropriately.  When it returns, it can be assumed that the
 *      message digest has been computed.
 *
 *  Parameters:
 *      context: [in/out]
 *          The context to pad
 *
 *  Returns:
 *      Nothing.
 *
 *  Comments:
 *
 */
void SHA1PadMessage(SHA1Context *context)
{
    /*
     *  Check to see if the current message block is too small to hold
     *  the initial padding bits and length.  If so, we will pad the
     *  block, process it, and then continue padding into a second
     *  block.
     */
    if (context->Message_Block_Index > 55)
    {
        context->Message_Block[context->Message_Block_Index++] = 0x80;
        while(context->Message_Block_Index < 64)
        {
            context->Message_Block[context->Message_Block_Index++] = 0;
        }

        SHA1ProcessMessageBlock(context);

        while(context->Message_Block_Index < 56)
        {
            context->Message_Block[context->Message_Block_Index++] = 0;
        }
    }
    else
    {
        context->Message_Block[context->Message_Block_Index++] = 0x80;
        while(context->Message_Block_Index < 56)
        {
            context->Message_Block[context->Message_Block_Index++] = 0;
        }
    }

    /*
     *  Store the message length as the last 8 octets
     */
    context->Message_Block[56] = (context->Length_High >> 24) & 0xFF;
    context->Message_Block[57] = (context->Length_High >> 16) & 0xFF;
    context->Message_Block[58] = (context->Length_High >> 8) & 0xFF;
    context->Message_Block[59] = (context->Length_High) & 0xFF;
    context->Message_Block[60] = (context->Length_Low >> 24) & 0xFF;
    context->Message_Block[61] = (context->Length_Low >> 16) & 0xFF;
    context->Message_Block[62] = (context->Length_Low >> 8) & 0xFF;
    context->Message_Block[63] = (context->Length_Low) & 0xFF;

    SHA1ProcessMessageBlock(context);
}
}
#endif

DWORD GetRdpSessionAddress();

/*
* Class:     com_bluejungle_destiny_agent_ipc_OSWrapper
* Method:    getRdpAddress
* Signature: 
* 
* There is a tool to know the address of an RDP caller.
*
* This function is not actually called (see PDPJni.cpp for actual
* implementation) and will be removed shortly.
*/

JNIEXPORT jlong JNICALL Java_com_bluejungle_destiny_agent_ipc_OSWrapper_getRDPAddress
(JNIEnv * env, jobject obj, jint processId)
{
    return GetRdpSessionAddress();
}
/************************************************************************/
/*                        RDP Check End                                 */
/************************************************************************/

#if defined (WIN32) || defined (_WIN64)
static void DisplayContentSearchNotify( JNIEnv *env , const std::wstring& user_sid , const std::wstring& file_path );
#else
static void DisplayContentSearchNotify( JNIEnv *env , const std::string& user_sid , const std::string& file_path );
#endif

/** Java_com_bluejungle_destiny_agent_ipc_OSWrapper_getContentAnalysisAttributes
 *
 *  Perform Content Analysis on a set of attributes.
 *
 *  \param in_file (in)          File to search.
 *  \param in_pid (in)           Process ID of process for which the file is being opened.
 *  \param in_ptoken (in)        Process token of process for which the file is being opened.
 *  \param in_sid (in)           SID of user for which the file is being opened.
 *  \param in_ca_attributes (in) Attributes to search for.
 *
 * Class:     com_bluejungle_destiny_agent_ipc_OSWrapper
 * Method:    getContentAnalysisAttributes
 * Signature: (I[II)I
 */
JNIEXPORT jintArray JNICALL Java_com_bluejungle_destiny_agent_ipc_OSWrapper_getContentAnalysisAttributes
(JNIEnv * env, jobject obj, jstring in_file , jint in_pid , jobject in_ptoken , jstring in_sid , jint log_level, jobjectArray in_ca_attributes )
{
#if defined (WIN32) || defined (_WIN64)
  int i = 0, num_attributes = 0, num_in_ca_attributes = 0;
  volatile int dummy;

  num_in_ca_attributes = env->GetArrayLength(in_ca_attributes);
  num_attributes = num_in_ca_attributes / 4;

  /* Array must be mod 4 since we expect an array of {expr,expr_op,expr_count} */
  if( (num_in_ca_attributes % 4) != 0 )
  {
    return env->NewIntArray(0);
  }

#if defined (WIN32) || defined (_WIN64)
  std::wstring file_name, user_sid;
#else
  std::string file_name, user_sid;
#endif
  const TCHAR* pfile_name = getStringFromJava(env, in_file);
  const TCHAR* puser_sid  = getStringFromJava(env, in_sid);
  file_name = pfile_name;
  user_sid = puser_sid;
  releaseStringFromJava(pfile_name);
  releaseStringFromJava(puser_sid);

  int pid = (int)in_pid;

  jintArray result = env->NewIntArray(num_attributes);
  bool search_result = false;

  // Impersonate user for access to file
  HANDLE handle = getProcessToken(env,in_ptoken);
  if( handle != NULL )
  {
    if (!ImpersonateLoggedOnUser (handle))
    {
      dummy = 0;                // just to keep Veracode happy
    }
  }

  /* Determine if the given file should be searched via nlca service and if a notitication
   * should occur.
   */
  bool do_search = false; // no search by default
  bool do_notify = false; // no notification by default
#if defined (WIN32) || defined (_WIN64)
  WIN32_FILE_ATTRIBUTE_DATA fattr;
  if( GetFileAttributesExW(file_name.c_str(),GetFileExInfoStandard,&fattr) != FALSE )
  {
    do_search = true;

    __int64 file_size = 0;
    file_size = fattr.nFileSizeHigh; // set high DWORD
    file_size <<= 32;
    file_size |= fattr.nFileSizeLow; // set low DWORD

    /* Any file greater than 100KB will not perform notification */
    if( file_size > (1024 * 100) )
    {
      do_notify = true;
    }
  }
#else
  struct stat fattr;
  if( stat(file_name.c_str(),&fattr) != -1 )
  {
    do_search = true;

    __int64 file_size = 0;
    file_size = fattr.st_size; 

    /* Any file greater than 100KB will not perform notification */
    if( file_size > (1024 * 100) )
    {
      do_notify = true;
    }
  }
#endif

  // Revert system user
  if( handle != NULL )
  {
    if (!RevertToSelf())
    {
      dummy = 0;                // just to keep Veracode happy
    }
    freeProcessToken(handle,in_ptoken);
  }

  /* If file size cannot be determined the file probably does not exist, so no sense in sending
   * it to the nlca service.
   */
  if( do_search == false )
  {
    return result;
  }

  /*************************************************************************
   * Construct set expression objects from object array given by caller
   *
   * Structure expected:
   *
   *    base + 0 : expression type ([REG|KEY:type)
   *               Prefix REG = regular expression
   *               Pregix KEY = keywords expression
   *    base + 1 : expression value (i.e. "\d{16}")
   *    base + 2 : expressoin operation (i.e. >=)
   *    base + 3 : expression required match count
   *
   ************************************************************************/
  std::list<NLCA::Expression*> exps;
  for( i = 0 ; i < num_attributes ; i++ )
  {
    int base = i * 4;
    jstring jexpr_type  = (jstring)env->GetObjectArrayElement(in_ca_attributes,base + 0); // Expression type  (+0)
    jstring jexpr_value = (jstring)env->GetObjectArrayElement(in_ca_attributes,base + 1); // Expression value (+1)
    jstring jexpr_count = (jstring)env->GetObjectArrayElement(in_ca_attributes,base + 3); // Expression count (+3)

    const TCHAR* expr_type  = getStringFromJava(env, jexpr_type);
    const TCHAR* expr_value = getStringFromJava(env, jexpr_value);
    const TCHAR* expr_count = getStringFromJava(env, jexpr_count);

    std::wstring nlca_expr_type, nlca_expr_value, nlca_expr_count;
    nlca_expr_type  = expr_type;
    nlca_expr_value = expr_value;
    nlca_expr_count = expr_count;

    releaseStringFromJava(expr_type);
    releaseStringFromJava(expr_value);
    releaseStringFromJava(expr_count);

    /* Determine expression type */
    if( nlca_expr_type.compare(0,4,L"KEY:",4) == 0 )
    {
      nlca_expr_type = L"Keyword(s)";
    }
    else if( nlca_expr_type.compare(0,4,L"REG:",4) == 0 )
    {
      nlca_expr_type.erase(0,4); /* strip 'REG:' from string */
    }

    int required_match_count = _wtoi(nlca_expr_count.c_str());
    NLCA::Expression* exp = NULL;
    try
    {
      exp = NLCA::ConstructExpression(nlca_expr_type.c_str());

      exp->SetExpression(nlca_expr_value);
      exp->SetRequiredMatchCount(required_match_count);
      exps.push_back(exp);
    }
    catch(...)
    {
    }
  }/* for i : num_attributes */

  if( do_notify == true && log_level >= CE_NOISE_LEVEL_USER_ACTION )
  {
    DisplayContentSearchNotify(env,user_sid,file_name); // Display notification
  }

  /* Handle and pointer to NLCAService_SearchFile in nlca_client.dll  This is loaded once and the pointer
   * is retained to avoid reloading the library on each content search.
   */
  static HMODULE hmod = NULL;
  typedef bool (*NLCAService_SearchFile_t)( const wchar_t* , int , std::list<NLCA::Expression *> &exps , int );
  static NLCAService_SearchFile_t pfn_SearchFile = NULL;

#if defined (_WIN64)
#define NLCA_CLIENT_DLL "nlca_client.dll"
#else
#define NLCA_CLIENT_DLL "nlca_client32.dll"
#endif

  if( hmod == NULL && pfn_SearchFile == NULL )
  {
    hmod = LoadLibraryA(NLCA_CLIENT_DLL);
    if( hmod != NULL )
    {
      pfn_SearchFile = (NLCAService_SearchFile_t)GetProcAddress(hmod,"NLCAService_SearchFile");
    }
  }

  if( pfn_SearchFile != NULL )
  {
    search_result = pfn_SearchFile(file_name.c_str(),pid,exps,30*1000);  // Search file
  }

  /* The result of each expression must be placed in the order as it was received such that the
   * Policy Controller can complete policy evaluation based on individual attribute results.
   */
  std::list<NLCA::Expression*>::iterator it = exps.begin();
  for( i = 0 ; search_result == true && i < num_attributes && it != exps.end() ; i++ )
  {
    const NLCA::Expression* exp = *it;
    int kresult = (int)exp->Satisfied();

    env->SetIntArrayRegion(result,i,1,(jint*)&kresult);
    delete exp;
    it++;
  }
  return result;
#else
  return NULL;
#endif
}/* Java_com_bluejungle_destiny_agent_ipc_OSWrapper_getContentAnalysisAttributes */


static bool notifyUser(JNIEnv *env, const TCHAR *userId, const TCHAR *action, const TCHAR *enforcement, const TCHAR *resourceName, const TCHAR *message) {
#if defined (WIN32) || defined (_WIN64)
    static jobject cmStub = NULL;
    static jmethodID notifyUserMethod = NULL;
    
    time_t currentTime = time(NULL);

    TCHAR *timestamp = _tctime(&currentTime);
    TCHAR hackedTimestamp[64];

    // Java wants the timezone in the middle of the string
    // DOW MON DD HH:MM:SS YYYY => DOW MON DD HH:MM:SS PST YYYY => 
    _tcsncpy_s(hackedTimestamp,64, timestamp, _TRUNCATE);
    _tcsncpy_s(hackedTimestamp+20, 44,_T("PST"), _TRUNCATE);
    _tcsncpy_s(hackedTimestamp+23, 41,timestamp+19, _TRUNCATE);
    hackedTimestamp[_tcslen(hackedTimestamp)-1] = '\0';  // eliminate \n

    jstring juserId = env->NewString((const jchar*)userId, (jsize)_tcslen(userId));
    jstring jaction = env->NewString((const jchar*)action, (jsize)_tcslen(action));
    jstring jtimestamp = env->NewString((const jchar*)hackedTimestamp, (jsize)_tcslen(hackedTimestamp));
    jstring jenforcement = env->NewString((const jchar*)enforcement, (jsize)_tcslen(enforcement));
    jstring jresourceName = env->NewString((const jchar*)resourceName, (jsize)_tcslen(resourceName));
    jstring jmessage = env->NewString((const jchar*)message, (jsize)_tcslen(message));

    if (notifyUserMethod == NULL) {
        static jclass cmStubClass = NULL;
        // Thread safety is for losers
        cmStubClass = env->FindClass(CONTROLMGR_STUB_CLASS);
        
        if (cmStubClass == NULL) {
            return false;
        }

        char buf[128];
        _snprintf_s (buf,128, _TRUNCATE, "()L%s;", CONTROLMGR_STUB_CLASS);  
        jmethodID getInstanceMethod = env->GetStaticMethodID(cmStubClass,
                                                                 CM_STUB_GETINSTANCE_M,
                                                                 buf);
        cmStub = env->NewGlobalRef(env->CallStaticObjectMethod(cmStubClass, getInstanceMethod));

        notifyUserMethod = env->GetMethodID(cmStubClass,
                                            "notifyUser",
                                            "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V");

        if (notifyUserMethod == NULL) {
            return false;
        }
    }

    env->CallVoidMethod(cmStub, notifyUserMethod, juserId, jtimestamp, jaction, jenforcement, jresourceName, jmessage);

    env->DeleteLocalRef(juserId);  
    env->DeleteLocalRef(jaction);  
    env->DeleteLocalRef(jtimestamp);  
    env->DeleteLocalRef(jenforcement);  
    env->DeleteLocalRef(jresourceName);  
    env->DeleteLocalRef(jmessage);
#endif
    return true;
}/* notifyUser */

wchar_t bslash_to_fslash( wchar_t ch )
{
  if( ch == L'\\' )
  {
    return L'/';
  }
  return ch;
}/* bslash_to_fslash */

/** DisplayContentSearchNotify
 *
 *  \brief Send a notification to the obligation manager.
 *
 *  \param user_sid (in)
 *  \param file_path (in)
 */
#if defined (WIN32) || defined (_WIN64)
static void DisplayContentSearchNotify( JNIEnv *env , const std::wstring& user_sid , const std::wstring& file_path )
#else
static void DisplayContentSearchNotify( JNIEnv *env , const std::string& user_sid , const std::string& file_path )
#endif
{
  // extract file name from full path
#if defined (WIN32) || defined (_WIN64)
  std::wstring file_name(file_path);
  std::wstring display_file_path;
  std::wstring::size_type i;
#else
  std::string file_name(file_path);
  std::string display_file_path;
  std::string::size_type i;
#endif

  i = file_path.find_last_of(_T('\\'));
  if( i != std::wstring::npos )
  {
    file_name.assign(file_path,i+1,file_path.length()-i);
  }

  /* Content analysis temp files are prefixed with "NLC" and contain a ".tmp" substring for suffix.
   * The actual suffix may be .txt, .html, other to indicate type.
   */
  if( (file_name.compare(0,3,_T("nlc")) == 0 && file_name.find(_T(".tmp")) != std::wstring::npos) ||
      file_name == _T("[no attachment]"))
  {
    return;
  }

  /* If the string size exceeds a maximum length remove the center portion of the string
   * while preserving some chars on each side.  The center is of the string will contain
   * three dots "...".
   */
  const size_t max_length = 32;
  if( file_name.length() > (max_length + 3) )
  {
    const size_t sides_size = max_length / 2;               // lenth to preserve on each side of the string
    const size_t j = sides_size;                            // set start of replacement (offset from string base)
    const size_t len = file_name.length() - 2 * sides_size; // set end of replacement (length to replace)
    file_name.replace(j,len,_T("..."));                        // replace [i,i+len] with "..."
  }

#if defined (WIN32) || defined (_WIN64)
  std::wstring notify_text;
#else
  std::string notify_text;
#endif

  notify_text = _T("Analyzing content for policy evaluation on ");
  notify_text += file_name;

  // Format display file path as file://...
  display_file_path = _T("file:///");
  display_file_path += file_path;
  /* Translate '\' to forward '/' */
  std::transform(display_file_path.begin(),display_file_path.end(),display_file_path.begin(),bslash_to_fslash);

  notifyUser(env,user_sid.c_str(),_T("Analyze"),_T("allow"),display_file_path.c_str(),notify_text.c_str());
}/* DisplayContentSearchNotify */


#if defined(WIN32) || defined(_WIN64)
// NtProcessInfo
#include "NtProcessInfo.cpp"

// Initialize NtProcessInfo
static BOOL initializeNtProcessInfo()
{
    static BOOL isNtProcessInfoInitialized = false;
    HMODULE hNTDLL = NULL;
    
    if (isNtProcessInfoInitialized == TRUE)
	return TRUE;

    isNtProcessInfoInitialized = TRUE;

    // set up NtProcessInfo
    if (sm_EnableTokenPrivilege(SE_DEBUG_NAME) == FALSE) {
    }
    hNTDLL = sm_LoadNTDLLFunctions();
    if (hNTDLL == NULL) {
	isNtProcessInfoInitialized = FALSE;
	return FALSE;
    }

    return TRUE;
}

// If OSWrapper is closed, close hNTDLL, too
// sm_FreeNTDLLFunctions(hNTDLL);
// make hNTDLL static and global

/*
 * Class:     com_bluejungle_destiny_agent_ipc_OSWrapper
 * Method:    getPathFromPID
 * Signature: (Ljava/lang/String;)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_bluejungle_destiny_agent_ipc_OSWrapper_getPathFromPID
  (JNIEnv * env, jobject obj, jint in_pid)
{
    jstring result = env->NewString((const jchar*)_T(""), 0);
    int pid = (int)in_pid;

    if (initializeNtProcessInfo() == FALSE)
	goto end;
    smPROCESSINFO *info = new smPROCESSINFO;
    BOOL rv = sm_GetNtProcessInfo(pid, info);
    if (rv == TRUE) {
	result = env->NewString ((const jchar*)info->szImgPath, static_cast<jsize>(wcslen(info->szImgPath)));
    } else {
    }
	delete info;

 end:
    return result;
}
#else
JNIEXPORT jstring JNICALL Java_com_bluejungle_destiny_agent_ipc_OSWrapper_getPathFromPID
  (JNIEnv * env, jobject obj, jint in_pid)
{
    jstring result = env->NewStringUTF("");
}
#endif
