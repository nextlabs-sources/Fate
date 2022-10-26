/*==========================eval.cpp========================================*
 *                                                                          *
 * All sources, binaries and HTML pages (C) copyright 2006 by Blue Nextlabs,*
 * Redwood City CA, Ownership remains with Blue Jungle Inc,                 * 
 * All rights reserved worldwide.                                           *
 *                                                                          * 
 * Author : Heidi Zhou                                                      *
 * Date   : 1/22/2007                                                       *
 * Note   : Implementations of SDK CEEVALUATE_XXX APIs.                     *
 *==========================================================================*/
#include <algorithm>
#include <string>
#include <cstdlib>
#include <time.h>
#include "brain.h"
#include "cetype.h"
#include "marshal.h"
#include "PEPMan.h"
#include "nlthread.h"
#include "JavaConstants.h"
#include "CESDK_private.h"
#include "nlconfig.hpp"

#if defined(WIN32)
#include "eframework/timer/timer_high_resolution.hpp"
#endif

#include "celog.h"
#include "celog_policy_windbg.hpp"

#if defined (Linux)
#include "linux_win.h"
#ifdef nlsprintf
#undef nlsprintf
#define nlsprintf snprintf
#endif
#endif

namespace {
  enum {PEPEVAL_MAX_BUF_LENGTH=1024};

  /* Make sure this table is in sync with the CEsdk.h action enumeration */
  /* Also, the strings must be defined in the JavaConstants.h            */
  /* The actions and corresponding strings need to be sync in the following
     files: JavaConstans.h, PDPEval.cpp, eval.cpp, CEsdk.h. */
  nlchar* actionTable[]={ NULL,
                          CE_ACTION_STRING_READ,                   
                          CE_ACTION_STRING_DELETE,                 
                          CE_ACTION_STRING_MOVE,                   
                          CE_ACTION_STRING_COPY,                   
                          CE_ACTION_STRING_WRITE,                  
                          CE_ACTION_STRING_RENAME,                 
                          CE_ACTION_STRING_CHANGE_ATTR_FILE,
                          CE_ACTION_STRING_CHANGE_SEC_FILE,        
                          CE_ACTION_STRING_PRINT_FILE,             
                          CE_ACTION_STRING_PASTE_FILE,             
                          CE_ACTION_STRING_EMAIL_FILE,             
                          CE_ACTION_STRING_IM_FILE,                
                          CE_ACTION_STRING_EXPORT,                 
                          CE_ACTION_STRING_IMPORT,                 
                          CE_ACTION_STRING_CHECKIN,                
                          CE_ACTION_STRING_CHECKOUT,
                          CE_ACTION_STRING_ATTACH,
                          CE_ACTION_STRING_RUN,
                          CE_ACTION_STRING_REPLY,
                          CE_ACTION_STRING_FORWARD,
                          CE_ACTION_STRING_NEW_EMAIL,
                          CE_ACTION_STRING_AVD,
                          CE_ACTION_STRING_MEETING,
                          CE_ACTION_STRING_PROCESS_TERMINATE,
                          CE_ACTION_STRING_WM_SHARE,
                          CE_ACTION_STRING_WM_RECORD,
                          CE_ACTION_STRING_WM_QUESTION,
                          CE_ACTION_STRING_WM_VOICE, 
                          CE_ACTION_STRING_WM_VIDEO,
                          CE_ACTION_STRING_WM_JOIN };

  //Shared data among threads with a process for evaluation
  typedef struct _SharedDataTag {
    //Store the current logging user mydesktop
    nlstring myDesktop;
    nlthread_mutex_t mutexMyDesktop;
  
    //Store the current logging user mydesktop
    nlstring myDocuments;
    nlthread_mutex_t mutexMyDocuments;

    _SharedDataTag():myDesktop(_T("")), myDocuments(_T("")) {
      nlthread_mutex_init(&mutexMyDocuments);
      nlthread_mutex_init(&mutexMyDesktop);
    }

    ~_SharedDataTag() {
      nlthread_mutex_destroy(&mutexMyDocuments);
      nlthread_mutex_destroy(&mutexMyDesktop);
    }
  } SharedData;
  SharedData sharedData;

  //Initaizlied shared data, i.e. [MyDesktop] folder, [MyDocuments] folder
  inline void InitializeSharedData()
  {
    //Get mydesktop folder if it hasn't been set yet.
    nlthread_mutex_lock(&sharedData.mutexMyDesktop);
    if(sharedData.myDesktop.empty()) {
      nlchar fBuf[PEPEVAL_MAX_BUF_LENGTH];
      int size=PEPEVAL_MAX_BUF_LENGTH;
      if(NL_GetMyDesktopFolder(fBuf, &size)) {
        sharedData.myDesktop=fBuf;

        // string to lower
        std::transform(sharedData.myDesktop.begin(),sharedData.myDesktop.end(),
                       sharedData.myDesktop.begin(),towlower);
      }
    }
    nlthread_mutex_unlock(&sharedData.mutexMyDesktop);

    //Get my document folder if it hasn't been set yet.
    nlthread_mutex_lock(&sharedData.mutexMyDocuments);
    if(sharedData.myDocuments.empty()) {
      nlchar fBuf[PEPEVAL_MAX_BUF_LENGTH];
      int size=PEPEVAL_MAX_BUF_LENGTH;
      if(NL_GetMyDocumentsFolder(fBuf, &size)) {
        sharedData.myDocuments=fBuf;

        // string to lower
        std::transform(sharedData.myDocuments.begin(),sharedData.myDocuments.end(),
                       sharedData.myDocuments.begin(),towlower);
      }
    }
    nlthread_mutex_unlock(&sharedData.mutexMyDocuments);
  }

  void GetResolvedName(nlstring &resolvedName)
  {
    // input parameter to lower for various comparisons
    std::transform(resolvedName.begin(),resolvedName.end(),resolvedName.begin(),towlower);

    //Check if it is on removable media
    if(NL_IsRemovableMedia(resolvedName.c_str())) {
      //It is on removable media
      nlchar *pstr=nlstrstr(resolvedName.c_str(), _T(":\\"));

      if( pstr != NULL )
      {
        nlchar tmpStr[1024];
        size_t l=nlstrlen(_T(":\\"));
        _snwprintf_s(tmpStr, _countof(tmpStr), _TRUNCATE, _T("[REMOVABLEMEDIA]\\%s"), pstr+l);
        resolvedName=tmpStr;
      }
      return;
    }

    nlthread_mutex_lock(&sharedData.mutexMyDocuments);
    nlthread_mutex_lock(&sharedData.mutexMyDesktop);
    if(sharedData.myDocuments.empty() && sharedData.myDesktop.empty()) {
      nlthread_mutex_unlock(&sharedData.mutexMyDocuments);
      nlthread_mutex_unlock(&sharedData.mutexMyDesktop);
      nlchar tmpStr[1024];

      if(NL_GetFilePhysicalPath(resolvedName.c_str(), tmpStr, 1024)) {
        //The file's physical path is different from logical path
        //return the physical path as resoved name
        resolvedName=tmpStr;      
      }    
      return;
    }

    if( sharedData.myDocuments.size() > 0 &&
        nlstrstr(resolvedName.c_str(), sharedData.myDocuments.c_str())) {
      //It is in the mydocuments folder
      nlchar tmpStr[1024];
      nlchar *pstr=nlstrstr(resolvedName.c_str(),
                            sharedData.myDocuments.c_str());

      // pstr will always be non-null due to conditional to get here
      size_t l=sharedData.myDocuments.length();
      _snwprintf_s(tmpStr, _countof(tmpStr), _TRUNCATE, _T("[mydocuments]%s"), pstr+l);
      resolvedName=tmpStr;

      nlthread_mutex_unlock(&sharedData.mutexMyDocuments);
      nlthread_mutex_unlock(&sharedData.mutexMyDesktop);
      return;
    }

    if( sharedData.myDesktop.size() > 0 &&
        nlstrstr(resolvedName.c_str(), sharedData.myDesktop.c_str())) {
      //It is in the mydesktop folder

      if( sharedData.myDesktop.compare(0,sharedData.myDesktop.length(),resolvedName,0,sharedData.myDesktop.length()) == 0 )
      {
        /* Replace the 'my desktop' prefix with the policy definition [mydesktop] */
        resolvedName.replace(0,sharedData.myDesktop.length(),_T("[mydesktop]"),_countof(_T("[mydesktop]")) - 1);
      }

      nlthread_mutex_unlock(&sharedData.mutexMyDocuments);
      nlthread_mutex_unlock(&sharedData.mutexMyDesktop);
      return;
    }

    //Unlock the mutex
    nlthread_mutex_unlock(&sharedData.mutexMyDocuments);
    nlthread_mutex_unlock(&sharedData.mutexMyDesktop);

    nlchar tmpStr[1024];
    if(NL_GetFilePhysicalPath(resolvedName.c_str(), tmpStr, 1024)) {
      //The file's physical path is different from logical path
      //return the physical path as resoved name
      resolvedName=tmpStr;      
    }    
    return;
  }

  bool hasAttributes(const CEAttributes *attrs) {
    return (attrs != NULL && attrs->count > 0 && attrs->attrs != NULL);
  }

  void SetAttribute(CEAttribute *attr, const nlchar *key, const nlchar *value) {
    attr->key = CEM_AllocateString(key);
    attr->value = CEM_AllocateString(value);
  }

  void SetAttribute(CEAttribute *attr, const CEString key, const CEString value) {
    attr->key = key;
    attr->value = value;
  }

  void FreeLocalAllocatedAttrs(CEAttributes a)
  {
    if(a.attrs==NULL || a.count==0)
      return;

    for(int i=0; i<a.count; i++) {
      CEM_FreeString(a.attrs[i].key);
      CEM_FreeString(a.attrs[i].value);
    }
    delete [] a.attrs;
  }

  inline CEResult_t IsValidActionEnum(CEAction_t a)
  {
    if((int)a <= 0)
      return CE_RESULT_INVALID_ACTION_ENUM;
    if((int)a > CE_ACTION_WM_JOIN)
      return CE_RESULT_INVALID_ACTION_ENUM;
    return CE_RESULT_SUCCESS;
  }

  inline void ComposeAttrDimension(CEAttributes *d, int count, const nlchar **array) 
  {
    nlchar tmpStr[PEPEVAL_MAX_BUF_LENGTH];
  
    d->count=count;
    d->attrs=new CEAttribute[d->count];

    for(int i=0; i<count; i++) {
      nlsprintf(tmpStr, PEPEVAL_MAX_BUF_LENGTH, _T("%d"), i);

      SetAttribute(&d->attrs[i], tmpStr, array[i]);
    }
  }

  inline void AddUserDefaultAttrs(CEAttributes *attrs, CEHandle handle,
                                  CEString userName, CEString userID,
                                  bool bConvertToLowerCase=false,
                                  CEAttributes *ua=NULL)
  {
    nlchar tmpBuf[1024];
    int endIndex=0;

    if (EMPTY_CESTRING(userName) && EMPTY_CESTRING(userID)) {
      //Get user information from CEHandle
      attrs->count=0;
      if(handle->userName) attrs->count++;
      if(handle->userID) attrs->count++;
      if(hasAttributes(ua)) {
        endIndex=attrs->count;
        (attrs->count)+=(ua->count);
      }
      attrs->attrs=new CEAttribute[attrs->count];
      int index=0;
      if(handle->userName) {
        if(bConvertToLowerCase && nlstrchr(handle->userName, '@')) {
          //We assume this is email address. It is very heuristic though.
          //Email address has to be converted to lower case
          nlstrntolow(tmpBuf, 1024, handle->userName, 
                      nlstrlen(handle->userName));
          SetAttribute(&attrs->attrs[index], _T("name"), tmpBuf);
        } else {
          SetAttribute(&attrs->attrs[index], _T("name"), handle->userName);
        }
        index++;
      }
      if(handle->userID) {
        if(bConvertToLowerCase && nlstrchr(handle->userID, '@')) {
          //We assume this is email address. It is very heuristic though.
          //Email address has to be converted to lower case
          nlstrntolow(tmpBuf, 1024, handle->userID, 
                      nlstrlen(handle->userID));
          SetAttribute(&attrs->attrs[index], _T("id"), tmpBuf);
        } else 
          SetAttribute(&attrs->attrs[index], _T("id"), handle->userName);
      }
    } else if(EMPTY_CESTRING(userName) && 
              NON_EMPTY_CESTRING(userID)) {
      //Get user Name from CEHandle
      attrs->count=1;
      if(handle->userName) attrs->count++;
      if(hasAttributes(ua)) {
        endIndex=attrs->count;
        (attrs->count)+=(ua->count);
      }
      attrs->attrs=new CEAttribute[attrs->count];
      int index=0;
      if(bConvertToLowerCase && nlstrchr(userID->buf, '@')) {
        //We assume this is email address. It is very heuristic though.
        //Email address has to be converted to lower case
        nlstrntolow(tmpBuf, 1024, userID->buf, userID->length);
        SetAttribute(&attrs->attrs[index], _T("id"), tmpBuf);
      } else 
        SetAttribute(&attrs->attrs[index], _T("id"), userID->buf);
      index++;
      if(handle->userName) {
        if(bConvertToLowerCase && nlstrchr(handle->userName, '@')) {
          //We assume this is email address. It is very heuristic though.
          //Email address has to be converted to lower case
          nlstrntolow(tmpBuf, 1024, handle->userName, 
                      nlstrlen(handle->userName));
          SetAttribute(&attrs->attrs[index], _T("name"), tmpBuf);
        } else {
          SetAttribute(&attrs->attrs[index], _T("name"), handle->userName);
        }
      }
    } else if(EMPTY_CESTRING(userID) &&
              NON_EMPTY_CESTRING(userName)) {
      //Get user ID from CEHandle
      attrs->count=1;
      if(handle->userID) attrs->count++;
      if(hasAttributes(ua)) {
        endIndex=attrs->count;
        (attrs->count)+=(ua->count);
      }
      attrs->attrs=new CEAttribute[attrs->count];
      int index=0;
      if(bConvertToLowerCase && nlstrchr(userName->buf, '@')) {
        //We assume this is email address. It is very heuristic though.
        //Email address has to be converted to lower case
        nlstrntolow(tmpBuf, 1024, userName->buf, userName->length);
        SetAttribute(&attrs->attrs[index], _T("name"), tmpBuf);
      } else {
        SetAttribute(&attrs->attrs[index], _T("name"), userName->buf);
      }
      index++;
      if(handle->userID) {
        if(bConvertToLowerCase && nlstrchr(handle->userID, '@')) {
          //We assume this is email address. It is very heuristic though.
          //Email address has to be converted to lower case
          nlstrntolow(tmpBuf, 1024, handle->userID, nlstrlen(handle->userID));
          SetAttribute(&attrs->attrs[index], _T("id"), tmpBuf);
        } else {
          SetAttribute(&attrs->attrs[index], _T("id"), handle->userID);
        }
      }
    } else {
      attrs->count=2;
      if(hasAttributes(ua)) {
        endIndex=attrs->count;
        (attrs->count)+=(ua->count);
      }
      attrs->attrs=new CEAttribute[attrs->count];
      if(bConvertToLowerCase && nlstrchr(userID->buf, '@')) {
        //We assume this is email address. It is very heuristic though.
        //Email address has to be converted to lower case
        nlstrntolow(tmpBuf, 1024, userID->buf, userID->length);
        SetAttribute(&attrs->attrs[0], _T("id"), tmpBuf);
      } else {
        SetAttribute(&attrs->attrs[0], _T("id"), userID->buf);
      }
      if(bConvertToLowerCase && nlstrchr(userName->buf, '@')) {
        //We assume this is email address. It is very heuristic though.
        //Email address has to be converted to lower case
        nlstrntolow(tmpBuf, 1024, userName->buf, userName->length);
        SetAttribute(&attrs->attrs[1], _T("name"), tmpBuf);
      } else {
        SetAttribute(&attrs->attrs[1], _T("name"), userName->buf);
      }
    }

    if(hasAttributes(ua)) {
      for(int j=0; j<ua->count; j++) {
        SetAttribute(&attrs->attrs[j+endIndex], ua->attrs[j].key->buf, ua->attrs[j].value->buf);
      }
    }

  }

  inline void ComposeOSAttrs(CEAttributes *attrs, 
                             bool bUseUserID, 
                             CEString userID)
  {
    nlchar idBuf[1024];

    attrs->count=0;
    attrs->attrs=NULL;

    attrs->count=1;
    attrs->attrs=new CEAttribute[attrs->count];

    if(bUseUserID && userID != NULL) {
      //Compose attributes
      SetAttribute(&attrs->attrs[0], _T("id"), userID->buf);
    } else if(NL_getUserId (idBuf, 1024)==BJ_OK) {
      //Compose attributes
      SetAttribute(&attrs->attrs[0], _T("id"), idBuf);
    } else {
      SetAttribute(&attrs->attrs[0], _T("id"), _T(NL_UNKNOWN_USER_ID));
    }
  }

  void ComposeAttributes(CEAttributes *dest, const CEAttributes *source)
  {
    dest->count = source->count;

    dest->attrs = new CEAttribute[dest->count];

    for (int i = 0; i < source->count; i++) {
      SetAttribute(&dest->attrs[i], source->attrs[i].key->buf, source->attrs[i].value->buf);
    }

  }

  void ComposePoliciesAttributes(CEAttributes *attrs, CEString additionalPQL, CEBoolean ignoreBuiltin)
  {
    if (EMPTY_CESTRING(additionalPQL))
    {
      attrs->count = 0;
      attrs->attrs = NULL;
      return;
    }

    attrs->count = 2;
    attrs->attrs = new CEAttribute[attrs->count];
    SetAttribute(&attrs->attrs[0], _T("pql"), additionalPQL->buf);
    SetAttribute(&attrs->attrs[1], _T("ignoredefault"), ignoreBuiltin == CETrue ? _T("yes") : _T("no"));
  }

  void ComposeAppAttrs(CEAttributes *attrs, CEApplication *app, 
                       CEHandle pepApp, CEAttributes *appAttrs=NULL)
  {
    nlstring fullAppName;
    nlstring fullPEPAppName;
    nlstring appURL;
    bool bHasURL=false;
    bool bHasPath=false;
  
    //Get application name including path
    if(app && NON_EMPTY_CESTRING(app->appPath)) {
      fullAppName=app->appPath->buf;
      bHasPath=true;
      if(pepApp->binaryPath) 
        fullPEPAppName=pepApp->binaryPath;
    } else {
      if(pepApp->binaryPath) {
        fullAppName=fullPEPAppName=pepApp->binaryPath;
        bHasPath=true;
      }
    }

    //Get web application's visiting URL if it exists
    if(app && NON_EMPTY_CESTRING(app->appURL)) {
      appURL=app->appURL->buf;
      bHasURL=true;
    }

    //Compose attributes
    attrs->count=1; 
    if(bHasPath)
      attrs->count+=1;
    if(bHasURL)
      attrs->count+=1;
    if(hasAttributes(appAttrs)) {
      attrs->count+=appAttrs->count;
    }
    if(attrs->count > 0) {
      int index=0;
      attrs->attrs=new CEAttribute[attrs->count];
    
      //Application process ID
      nlchar pidStr[1024];
#if defined (WIN32) || defined (_WIN64)
      DWORD appPid=GetCurrentProcessId();
      nlsprintf(pidStr, 1024, _T("%d"), appPid);
#else
      pid_t appPid=getpid();
      nlsprintf(pidStr, 1024, _T("%d"), appPid);
#endif
      SetAttribute(&attrs->attrs[index++], _T("pid"), pidStr);
    
      if(bHasPath) {
        SetAttribute(&attrs->attrs[index++], _T("name"), fullAppName.c_str());
      }
      if(bHasURL) {
        SetAttribute(&attrs->attrs[index++], _T("url"), appURL.c_str());
      }
      if(hasAttributes(appAttrs)) {
        for(int j=0; j<appAttrs->count; j++) {
          SetAttribute(&attrs->attrs[j+index], appAttrs->attrs[j].key->buf, appAttrs->attrs[j].value->buf);
        }
      }
    } else {
      attrs->attrs=NULL;
    }
  }

  inline void ComposeActionAttrs(CEAttributes *attrs, CEAction_t operation)
  {
    attrs->count=1;
    attrs->attrs=new CEAttribute[attrs->count];
    SetAttribute(attrs->attrs, _T("name"), actionTable[operation]);
  }

  inline void ComposeActionAttrs(CEAttributes *attrs, CEString operation)
  {
    attrs->count=1;
    attrs->attrs=new CEAttribute[attrs->count];
    SetAttribute(attrs->attrs, _T("name"), operation->buf);
  }

  inline void AddHostAttributes(CEAttributes *attrs, CEint32 ipNumber)
  {
    attrs->count=1;
    attrs->attrs=new CEAttribute[attrs->count];
    // We pass the IP address in network-byte order form
    nlchar ipStr[PEPEVAL_MAX_BUF_LENGTH];
    nlsprintf(ipStr, PEPEVAL_MAX_BUF_LENGTH, _T("%d"), ipNumber);
    SetAttribute(attrs->attrs, _T("inet_address"), ipStr);
  }

  void ComposeSendToAttrs(CEAttributes *d, CEint32 numRecipients,
                          CEString *recipients)
  {
    //No recipients
    if(numRecipients<=0 || numRecipients>CE_NUM_RECIPIENTS_MAX ||
       recipients==NULL ) {
      d->attrs=NULL;
      d->count=0;
      return;
    }

    nlchar tmpBuf[1024];
    d->count=numRecipients;
    d->attrs=new CEAttribute[(size_t) numRecipients];
    for(int i=0; i<numRecipients; i++) {
      nlstrntolow(tmpBuf, 1024,recipients[i]->buf, recipients[i]->length);

      SetAttribute(&d->attrs[i], _T("email"), tmpBuf);
    }
  }

  void ComposeResourceAttrs(CEAttributes *d, CEAttributes *s,
                            const nlchar *destinytype, 
                            const nlchar *resourcetype, 
                            CEString resource)
  {
    if(EMPTY_CESTRING(resource)) {
      d->count=0;
      d->attrs=NULL;
      return; //resource is empty
    }

    //Initaizlied shared data, i.e. [MyDesktop] folder, [MyDocuments] folder
    InitializeSharedData();

    //Get resource's resolved name
    nlstring resolvedName=resource->buf;
    GetResolvedName(resolvedName);

    d->count=3; //The defaults: CE::id, CE::destinytype, CE::resolved_name/url
    if(hasAttributes(s)) {
      d->count+=s->count;
    }
    d->attrs=new CEAttribute[d->count];

    SetAttribute(&d->attrs[0], _T("CE::destinytype"), destinytype);
    SetAttribute(&d->attrs[1], _T("CE::id"), resource->buf);
    SetAttribute(&d->attrs[2], resourcetype, resolvedName.c_str());

    if(hasAttributes(s)) {
      for(int i=0; i<s->count; i++) {
        SetAttribute(&d->attrs[3+i], s->attrs[i].key->buf, s->attrs[i].value->buf);
      }
    }
  }

  //Compose resource attributes with structure CEResource
  void ComposeResourceAttrs(CEAttributes *d, 
                            const CEAttributes *s,
                            const CEResource *resource)
  {
    const nlchar *resourceTitle = _T("");
    bool bAddResourceTitle=false;
    bool bHasResourceType=false;
    nlstring resolvedName;

    //initialize output
    d->count=0;
    d->attrs=NULL;
  
    //Initaizlied shared data, i.e. [MyDesktop] folder, [MyDocuments] folder
    InitializeSharedData();
 
    //input checking
    if (resource == NULL || EMPTY_CESTRING(resource->resourceName)) {
      return;
    }

    //get attributs count
    d->count=1; //The defaults: CE::id 
    //count destinytype in if CE::destinytype exists
    if(NON_EMPTY_CESTRING(resource->resourceType)) {
      d->count+=1;
      bHasResourceType=true;
      //count resource title ("url" or "resovedname") in if it 
      //the resource type is "fso" or "portal"
      if(nlstricmp(resource->resourceType->buf, _T("fso"))==0) {
        bAddResourceTitle=true;
        resourceTitle=_T("CE::resolved_name");
        //Get resource's resolved name
        resolvedName =resource->resourceName->buf;
        GetResolvedName(resolvedName);
        d->count+=1;
      } else if (nlstricmp(resource->resourceType->buf, _T("portal")) == 0) {
        bAddResourceTitle=true;
        resourceTitle=_T("url");
        resolvedName =resource->resourceName->buf;
        d->count+=1;
      }
    }

    //count resource attributs in
    if(hasAttributes(s)) {
      d->count+=s->count;
    }
  
    //allocate attributes
    d->attrs=new CEAttribute[d->count];

    //assign attributes
    int base=1;
    SetAttribute(&d->attrs[0], _T("CE::id"), resource->resourceName->buf);
    if(bHasResourceType) {
      SetAttribute(&d->attrs[1], _T("CE::destinytype"), resource->resourceType->buf);
      base=2;
      if(bAddResourceTitle) {
        base=3;
        SetAttribute(&d->attrs[2], resourceTitle, resolvedName.c_str());
      }
    }
    if(hasAttributes(s)) {
      for(int i=0; i<s->count; i++) {
        SetAttribute(&d->attrs[base+i], s->attrs[i].key->buf, s->attrs[i].value->buf);
      }
    }
  }

  CEResult_t CheckPortalInputs(CEHandle handle, 
                               CEAction_t operation, 
                               CEString sourceURL, 
                               CEAttributes * sourceAttributes,
                               CEString targetURL,
                               CEAttributes * targetAttributes,
                               CEUser  *user,
                               CEint32 timeout_in_millisec)
  {
    if(handle == NULL) {
      return CE_RESULT_NULL_CEHANDLE;
    }

    if(IsValidActionEnum(operation) != CE_RESULT_SUCCESS)
      return CE_RESULT_INVALID_ACTION_ENUM;

    if(!(timeout_in_millisec == CE_INFINITE || timeout_in_millisec >=0)) { 
      return CE_RESULT_INVALID_PARAMS;
    }

    if(!(operation == CE_ACTION_READ || operation == CE_ACTION_DELETE ||
         operation == CE_ACTION_MOVE || operation == CE_ACTION_COPY ||
         operation == CE_ACTION_WRITE || operation == CE_ACTION_EXPORT ||
         operation == CE_ACTION_ATTACH)) {
      return CE_RESULT_INVALID_EVAL_ACTION;
    }

    if(EMPTY_CESTRING(sourceURL)) { 
      return CE_RESULT_EMPTY_SOURCE;
    }
  
    //Check sourceAttributes
    if(sourceAttributes==NULL) {
      return CE_RESULT_EMPTY_SOURCE_ATTR;
    }
    for(int i=0; i < sourceAttributes->count; i++) {
      if(EMPTY_CESTRING(sourceAttributes->attrs[i].key)) {
        return CE_RESULT_EMPTY_ATTR_KEY;
      } 

      if(EMPTY_CESTRING(sourceAttributes->attrs[i].value)) {
        return CE_RESULT_EMPTY_ATTR_VALUE;
      }
    }

    //Check target for the following actions
    if(operation == CE_ACTION_MOVE || operation == CE_ACTION_COPY ) {
      if(EMPTY_CESTRING(targetURL)) {
        //In CheckFile, it allows COPY action without target for the case when
        //target is not set. For example, when open word application and insert 
        //a file, the document has not been named yet. In CheckPort, it is not
        //sure that this kind of case is applicable. Thus, target is still 
        //necessary for COPY action.
        return CE_RESULT_MISSING_TARGET;
      }     
    }
 
    //Check targetAttributes
    if(targetAttributes && targetAttributes->count!=0) {
      for(int i=0; i < targetAttributes->count; i++) {
        if(EMPTY_CESTRING(targetAttributes->attrs[i].key)) {
          return CE_RESULT_EMPTY_ATTR_KEY;
        } 
      
        if(EMPTY_CESTRING(targetAttributes->attrs[i].value)) {
          return CE_RESULT_EMPTY_ATTR_VALUE;
        }
      }
    } 

    if(user == NULL) {
      return CE_RESULT_EMPTY_PORTAL_USER;
    } else if(EMPTY_CESTRING(user->userID)) {
      return CE_RESULT_EMPTY_PORTAL_USERID;
    }
  
    return CE_RESULT_SUCCESS;
  }

  CEResult_t CheckFileInputs(CEHandle handle, 
                             CEAction_t operation, 
                             CEString sourceFullFileName, 
                             CEAttributes * sourceAttributes,
                             CEString targetFullFileName,
                             CEAttributes * targetAttributes,
                             CEUser  *user,
                             CEint32 timeout_in_millisec)
  {
    if(handle == NULL) {
      return CE_RESULT_NULL_CEHANDLE;
    }

    if(IsValidActionEnum(operation) != CE_RESULT_SUCCESS)
      return CE_RESULT_INVALID_ACTION_ENUM;

    if(!(timeout_in_millisec == CE_INFINITE || timeout_in_millisec >=0)) { 
      return CE_RESULT_INVALID_PARAMS;
    }

    if(!(operation == CE_ACTION_READ || operation == CE_ACTION_DELETE ||
         operation == CE_ACTION_MOVE || operation == CE_ACTION_COPY ||
         operation == CE_ACTION_WRITE || operation == CE_ACTION_RENAME ||
         operation == CE_ACTION_CHANGE_ATTR_FILE ||
         operation == CE_ACTION_CHANGE_SEC_FILE ||
         operation == CE_ACTION_PRINT_FILE || 
         operation == CE_ACTION_PASTE_FILE ||
         operation == CE_ACTION_EMAIL_FILE ||
         operation == CE_ACTION_IM_FILE ||
         operation == CE_ACTION_RUN ||
         operation == CE_ACTION_PROCESS_TERMINATE)) {
      return CE_RESULT_INVALID_EVAL_ACTION;
    }

    if(EMPTY_CESTRING(sourceFullFileName)) {
      return CE_RESULT_EMPTY_SOURCE;
    }
  
    if(!(operation == CE_ACTION_PROCESS_TERMINATE)) {
      bool  bHasModifiedTime=false;

      //Check sourceAttributes
      if(sourceAttributes==NULL) {
        return CE_RESULT_EMPTY_SOURCE_ATTR;
      }
      for(int i=0; i < sourceAttributes->count; i++) {
        if(EMPTY_CESTRING(sourceAttributes->attrs[i].key)) {
          return CE_RESULT_EMPTY_ATTR_KEY;
        } else if(nlstrstr(sourceAttributes->attrs[i].key->buf, 
                           CE_ATTR_LASTWRITE_TIME))
          bHasModifiedTime=true;
      
        if(EMPTY_CESTRING(sourceAttributes->attrs[i].value)) {
          return CE_RESULT_EMPTY_ATTR_VALUE;
        }
      }
 
      if(!bHasModifiedTime) {
        return CE_RESULT_MISSING_MODIFIED_DATE;
      } 
    }

    //Check target for the following operations
    if(operation == CE_ACTION_MOVE ||
       operation == CE_ACTION_RENAME) {
      if(EMPTY_CESTRING(targetFullFileName)) {
        return CE_RESULT_MISSING_TARGET;
      }
    }
     
    //Check targetAttributes
    if(targetAttributes && targetAttributes->count!=0) {
      for(int i=0; i < targetAttributes->count; i++) {
        if(EMPTY_CESTRING(targetAttributes->attrs[i].key)) {
          return CE_RESULT_EMPTY_ATTR_KEY;
        } 

        if(EMPTY_CESTRING(targetAttributes->attrs[i].value)) {
          return CE_RESULT_EMPTY_ATTR_VALUE;
        }
      }
    } 

    return CE_RESULT_SUCCESS;
  }

  CEResult_t CheckResourceInputs(CEHandle handle, 
                                 CEString operation, 
                                 const CEResource *source, 
                                 const CEAttributes * sourceAttributes,
                                 const CEResource *target,
                                 const CEAttributes * targetAttributes,
                                 CEint32 timeout_in_millisec)
  {
    if(handle == NULL) {
      return CE_RESULT_NULL_CEHANDLE;
    }

    if(EMPTY_CESTRING(operation)) {
      return CE_RESULT_INVALID_PARAMS;
    }

    if(!(timeout_in_millisec == CE_INFINITE || timeout_in_millisec >=0)) { 
      return CE_RESULT_INVALID_PARAMS;
    }
  
    if (source == NULL || EMPTY_CESTRING(source->resourceName)) {
      return CE_RESULT_EMPTY_SOURCE;
    }

    //Check sourceAttributes
    if(sourceAttributes) {
      for(int i=0; i < sourceAttributes->count; i++) {
        if(EMPTY_CESTRING(sourceAttributes->attrs[i].key)) {
          return CE_RESULT_EMPTY_ATTR_KEY;
        } 

        if(EMPTY_CESTRING(sourceAttributes->attrs[i].value)) {
          return CE_RESULT_EMPTY_ATTR_VALUE;
        }
      }
    }
     
    //Check targetAttributes
    if(targetAttributes && targetAttributes->count!=0) {
      for(int i=0; i < targetAttributes->count; i++) {
        if(EMPTY_CESTRING(targetAttributes->attrs[i].key)) {
          return CE_RESULT_EMPTY_ATTR_KEY;
        } 

        if(EMPTY_CESTRING(targetAttributes->attrs[i].value)) {
          return CE_RESULT_EMPTY_ATTR_VALUE;
        }
      }
    } 

    return CE_RESULT_SUCCESS;
  }

  CEResult_t CheckMsgAttachmentInput(CEHandle handle, 
                                     CEAction_t operation, 
                                     CEString sourceFullFileName, 
                                     CEAttributes * sourceAttributes,
                                     int numRecipients,
                                     CEString *recipients,
                                     CEUser  *user,
                                     CEint32 timeout_in_millisec)
  {
    if(handle == NULL) {
      return CE_RESULT_NULL_CEHANDLE;
    }

    if(IsValidActionEnum(operation) != CE_RESULT_SUCCESS)
      return CE_RESULT_INVALID_ACTION_ENUM;

    if(!(timeout_in_millisec == CE_INFINITE || timeout_in_millisec >=0)) {  
      return CE_RESULT_INVALID_PARAMS;
    }

    if(!(operation == CE_ACTION_PASTE_FILE || 
         operation == CE_ACTION_EMAIL_FILE || 
         operation == CE_ACTION_IM_FILE || 
         operation == CE_ACTION_AVD || 
         operation == CE_ACTION_MEETING ||
         operation == CE_ACTION_WM_SHARE ||
         operation == CE_ACTION_WM_RECORD ||
         operation == CE_ACTION_WM_QUESTION ||
         operation == CE_ACTION_WM_VOICE ||
         operation == CE_ACTION_WM_VIDEO ||
         operation == CE_ACTION_WM_JOIN )) {
      return CE_RESULT_INVALID_EVAL_ACTION;
    }

    if(EMPTY_CESTRING(sourceFullFileName)) {
      return CE_RESULT_EMPTY_SOURCE;
    }
  
    bool  bHasModifiedTime=false;

    //Check sourceAttributes
    if(sourceAttributes==NULL) {
      return CE_RESULT_EMPTY_SOURCE_ATTR;
    }
    for(int i=0; i < sourceAttributes->count; i++) {
      if(EMPTY_CESTRING(sourceAttributes->attrs[i].key)) {
        return CE_RESULT_EMPTY_ATTR_KEY;
      } else if(nlstrstr(sourceAttributes->attrs[i].key->buf, 
                         CE_ATTR_LASTWRITE_TIME))
        bHasModifiedTime=true;
    
      if(EMPTY_CESTRING(sourceAttributes->attrs[i].value)) {
        return CE_RESULT_EMPTY_ATTR_VALUE;
      }
    }
 
    if(!bHasModifiedTime) {
      return CE_RESULT_MISSING_MODIFIED_DATE;
    } 

    return CE_RESULT_SUCCESS;
  }

  CEResult_t CSCINVOKE_CheckInputs(CEHandle handle, 
                                   CEString sourceName, 
                                   CEString targetName,
                                   nlchar **sourceAttributesStrs,
                                   CEint32 numSourceAttributes,
                                   nlchar **targetAttributesStrs,
                                   CEint32 numTargetAttributes)
  {
    if(handle == NULL) {
      return CE_RESULT_NULL_CEHANDLE;
    }

    if(EMPTY_CESTRING(sourceName)) {
      return CE_RESULT_EMPTY_SOURCE;
    }
  
    //Check sourceAttributes
    for(int i=0; i < numSourceAttributes; i++) {
      if(sourceAttributesStrs[i]==NULL) {
        return CE_RESULT_EMPTY_ATTR_KEY;
      }
    }
 
    //Check targetAttributes
    for(int i=0; i < numTargetAttributes; i++) {
      if(targetAttributesStrs[i]==NULL) {
        return CE_RESULT_EMPTY_ATTR_KEY;
      }
    }

    return CE_RESULT_SUCCESS;
  }

  static void copyEnforcementData(const CEEnforcement_t* source, CEEnforcement_t *destination)
  {
      if(destination) {
        destination->result = source->result;
        if(source->obligation == NULL)
          destination->obligation = NULL;
        else {
          destination->obligation = new CEAttributes;
          destination->obligation->count = source->obligation->count;
          destination->obligation->attrs=new CEAttribute [source->obligation->count];
          for(int j=0; j < source->obligation->count; j++) {
            if(source->obligation->attrs[j].key == NULL ) 
              destination->obligation->attrs[j].key=NULL;
            else {
              CEString key = new struct _CEString();
              key->length = source->obligation->attrs[j].key->length;
              key->buf = new nlchar[source->obligation->attrs[j].key->length+1];
              nlstrncpy_s(key->buf, source->obligation->attrs[j].key->length+1,
                          source->obligation->attrs[j].key->buf,_TRUNCATE);
              destination->obligation->attrs[j].key = key;
            }
            
            if(source->obligation->attrs[j].value == NULL)
              destination->obligation->attrs[j].value=NULL;
            else {
              CEString value = new struct _CEString();
              value->length = source->obligation->attrs[j].value->length;
              value->buf = new nlchar[source->obligation->attrs[j].value->length+1];
              nlstrncpy_s(value->buf, source->obligation->attrs[j].value->length+1,
                          source->obligation->attrs[j].value->buf, _TRUNCATE);
              destination->obligation->attrs[j].value = value;
            }
          }
        }
      }
  }

  /* ------------------------------------------------------------------------
   * CEEVALUATE_CheckMetadata()
   *
   * Internal interface between PDP/PEP and Java policy engine. This interface
   * passing policy request using dynamic attribute list
   *
   * Ask the Policy Decision Point Server to evaluate the operation
   * 
   * Arguments : 
   * handle (INPUT): Handle from the CONN_Initialize()
   *  dimensions (INPUT): pointer to evaluation query dimension CEAttributes, e.g. dimensions->count=7; dimensions->attrs: {"0", "from"}, {"1", "host"}, {"2", "application"}, {"3", "user"}, {"4", "action"}, {"5", "to"}, {"6", "operating-system-user"}.
   *  attributeMatrix (INPUT): the pointer to evaluation query matrix corresponding to the above dimension. Actually, it is an array of CEAttributes and the length of array is the attributes dimensions. The followings is the pseudo code to fill the matrix,
   *  //Compose dynamic attributes list
   *  attrsMatrix=new CEAttributes[7];
   * 
   *  //from attributes
   *  //attrsMatrix[0].count=3;
   *  //attrsMatrix[0].attrs=new CEAttribute[3];
   *  //attrsMatrix[0].attrs[0].key=CEM_AllocateString(_T("CE::destinytype"));
   *  //attrsMatrix[0].attrs[0].value=CEM_AllocateString(destinytype);
   *  //attrsMatrix[0].attrs[1].key=CEM_AllocateString(_T("CE::id"));
   *  //attrsMatrix[0].attrs[1].value=CEM_AllocateString(resource->buf);
   *  //attrsMatrix[0].attrs[2].key=CEM_AllocateString(resourcetype);
   *  //attrsMatrix[0].attrs[2].value=CEM_AllocateString(resolvedName.c_str());
   *        
   *  //Similar to above, compose host, application, user, action, to, operating-system-user attributes.
   *
   * noiseLevel(INPUT): Desirable noise level to be used for this evaluation*
   * performObligation (INPUT): Perform the obligation defined by the policy 
   *                            (e.g. logging / email)*
   * timeout_in_millisec (INPUT): Desirable timeout in milliseconds for 
   * this evaluation
   * Enforcement (OUTPUT): Result of the policy for enforcement 
   * --------------------------------------------------------------------------------
   */ 
  CEResult_t CEEVALUATE_CheckMetadata (CEHandle handle, 
                                       CEAttributes * dimensions,
                                       CEAttributes * attributeMatrix,
                                       CENoiseLevel_t noiseLevel,
                                       CEBoolean performObligation,
                                       CEEnforcement_t * enforcement,
                                       CEint32 timeout_in_millisec)
  {
    try {    
      CEAttributes_Array aattrs;
      std::vector<void *> args;
      nlchar reqIDStr[100];
      args.reserve(RPC_MAX_NUM_ARGUMENTS);
      nlsprintf(reqIDStr, _countof(reqIDStr), _T("%lu+%lu"), 
                nlthread_selfID(), 
                PEPMAN_GetUniqueRequestID());
      CEString reqIDArg = CEM_AllocateString(reqIDStr);
      nlstring reqID(reqIDStr);
      size_t reqLen;

      // Should initialize the output first, otherwise, we are returning
      // a bogus output if there is error. e.g. timeout
      if (enforcement) {
        memset (enforcement, 0x0, sizeof (CEEnforcement_t));
        enforcement->result = CEAllow;
      }

      //Convert CEAttributes * to CEAttributes_Array
      aattrs.count=dimensions->count;
      aattrs.attrs_array=attributeMatrix;

      //Construct input arguments vector
      args.push_back(reqIDArg);
      args.push_back(&(handle->sessionID));
      args.push_back(dimensions);
      args.push_back(&aattrs);
      args.push_back(&noiseLevel);
      args.push_back(&performObligation);
    
      //Marshal request 
      char *packed=Marshal_PackReqFunc(_T("CEEVALUATE_CheckMetadata"), 
                                       args, reqLen);
      if(!packed) {
        return CE_RESULT_INVALID_PARAMS;
      } 

      //Do RPC call
      nlstring reqFunc(100u,' ');
      CEResult_t reqResult;
      vector<void *> reqOut;
      reqOut.reserve(RPC_MAX_NUM_ARGUMENTS);
      CEResult_t result=PEPMAN_RPCCall(reqID, packed, reqLen, handle, 
                                       reqFunc, reqResult, reqOut, 
                                       timeout_in_millisec);
      if(result != CE_RESULT_SUCCESS) {
        Marshal_PackFree(packed); 
        reqOut.clear(); 
        CEM_FreeString(reqIDArg);
        return result;
      }

      copyEnforcementData((CEEnforcement_t *)reqOut[1], enforcement);

      //Cleaning up
      Marshal_PackFree(packed); 
      Marshal_UnPackFree(reqFunc.c_str(), reqOut, false); 
      CEM_FreeString(reqIDArg);
    
      //Return result
      return reqResult;    
    } catch (std::exception &e) {
      return CE_RESULT_GENERAL_FAILED;
    } catch (...) {
      return CE_RESULT_GENERAL_FAILED;
    }
  }
}

/* ------------------------------------------------------------------------
 * CEEVALUATE_CheckResourceEx()
 * asks Policy Controller to evaluation the operation on multi resources
 *
 * Parameters
 * handle (INPUT): handle from the CONN_Initialize().
 * requests (INPUT): an array of CERequests
 * numRequests (INPUT): the number of items in requests
 * additionalPQL (INPUT): A CEString of PQL that will be evaluated along with
 *  or instead of the polices in the bundle
 * ignoreBuiltinPolicies (INPUT): if true, the policies in the bundle will be
 *  ignored in favor of 'additionalPQL'. If false, they will be evaluated together
 * ipNumber (INPUT): the IP address of client machine.
 * performObligation (INPUT): perform the obligation defined by the policy 
 *   (e.g. logging / email)
 * noiseLevel (INPUT): desirable noise level to be used for this evaluation.
 * enforcements (OUTPUT): an array of enforcement results (the length is the same
 *   as the number of items in requests, so numRequests)
 * timeout_in_millisec (INPUT): desirable timeout in milliseconds for this 
 *   evaluation
 *
 * Return
 * If the evaluation succeeds, the return value is CE_RESULT_SUCCESS; 
 * otherwise, it returns error value, e.g. CE_RESULT_NULL_CEHANDLE.
 * ------------------------------------------------------------------------
 */ 
CEResult_t CEEVALUATE_CheckResourcesEx(CEHandle handle,
                                       CERequest *requests,
                                       CEint32 numRequests,
                                       CEString additionalPQL,
                                       CEBoolean ignoreBuiltinPolicies,
                                       CEint32 ipNumber,
                                       CEEnforcement_t *enforcements,
                                       CEint32 timeout_in_millisec)
{
  
#ifdef WIN32
  nextlabs::high_resolution_timer ht;
#endif

  try 
  {
    if (timeout_in_millisec == 0) {
      return CE_RESULT_TIMEDOUT;
    }

    if (requests == NULL || numRequests <= 0) {
      return CE_RESULT_INVALID_PARAMS;
    }

    if (numRequests > CE_MAX_NUM_REQUESTS) {
      return CE_RESULT_TOO_MANY_REQUESTS;
    }

    std::vector<void *> args;

    nlstring fmtString = _T("si");  // req id, # of queries

    nlchar reqIDStr[64];
    nlsprintf(reqIDStr, _countof(reqIDStr), _T("%lu+%lu"), nlthread_selfID(), PEPMAN_GetUniqueRequestID());

    CEString reqIDArg = CEM_AllocateString(reqIDStr);
    args.push_back(reqIDArg);
    args.push_back(new CEint32(numRequests));

    for (int i = 0; i < numRequests; i++)
    {
      CEResult_t result = CheckResourceInputs(handle, requests[i].operation,
                                              requests[i].source, requests[i].sourceAttributes,
                                              requests[i].target, requests[i].targetAttributes,
                                              timeout_in_millisec);

      // dimension names, dimensions, noise level, perform obligations
      fmtString += _T("a[aib");

      if (result != CE_RESULT_SUCCESS)
      {
        return result;
      }

      // We don't have a fixed list of dimensions, thanks to additionalAttributes. We can be clever
      // and work out just what attributes we have, but it's easier to be stupid and assume it's
      // the maximum eight (from, to, user, app, host, action, sendto, operating-system-user) plus
      // plus one for the additionalPolicies, plus whatever is in additionalAttributes
      //
      // TODO - Helper methods, so I don't have to do this manually

      int numDimensions = 8 + 1 + requests[i].numAdditionalAttributes;

      std::vector<const nlchar *> dimensionAttrs;
      dimensionAttrs.push_back(_T("from"));
      dimensionAttrs.push_back(_T("host"));
      dimensionAttrs.push_back(_T("application"));
      dimensionAttrs.push_back(_T("user"));
      dimensionAttrs.push_back(_T("action"));
      dimensionAttrs.push_back(_T("to"));
      dimensionAttrs.push_back(_T("sendto"));
      dimensionAttrs.push_back(_T("operating-system-user"));

      // Additional PQL
      dimensionAttrs.push_back(_T("policies"));
    
      for (int j = 0; j < requests[i].numAdditionalAttributes; j++) {
        dimensionAttrs.push_back(requests[i].additionalAttributes[j].name->buf);
      }

      CEAttributes *dimensions = new CEAttributes;
      ComposeAttrDimension(dimensions, numDimensions, &dimensionAttrs[0]);

      args.push_back(dimensions);

      // Build the actual attributes
      CEAttributes *attrsArray = new CEAttributes[numDimensions];

      int matrixIndex = 0;

      ComposeResourceAttrs(&attrsArray[matrixIndex++], requests[i].sourceAttributes, requests[i].source);

      AddHostAttributes(&attrsArray[matrixIndex++], ipNumber);

      ComposeAppAttrs(&attrsArray[matrixIndex++], requests[i].app, handle, requests[i].appAttributes);

      AddUserDefaultAttrs(&attrsArray[matrixIndex++], handle, requests[i].user->userName, requests[i].user->userID, false, requests[i].userAttributes);

      ComposeActionAttrs(&attrsArray[matrixIndex++], requests[i].operation);

      ComposeResourceAttrs(&attrsArray[matrixIndex++], requests[i].targetAttributes, requests[i].target);

      ComposeSendToAttrs(&attrsArray[matrixIndex++], requests[i].numRecipients, requests[i].recipients);

      ComposeOSAttrs(&attrsArray[matrixIndex++], true, requests[i].user->userID);

      ComposePoliciesAttributes(&attrsArray[matrixIndex++], additionalPQL, ignoreBuiltinPolicies);

      for (int j = 0; j < requests[i].numAdditionalAttributes; j++) {
        ComposeAttributes(&attrsArray[matrixIndex++], &requests[i].additionalAttributes[j].attrs);
      }

      CEAttributes_Array *aattrs = new CEAttributes_Array();
      aattrs->count = numDimensions;
      aattrs->attrs_array = attrsArray;

      args.push_back(aattrs);
      args.push_back(new CEint32(requests[i].noiseLevel));
      args.push_back(new CEBoolean(requests[i].performObligation));
    }

    static const nlchar *functionName = _T("CEEVALUATE_CheckResourcesEx");

    size_t requestLen;
    char *marshaledRequest = Marshal_PackReqFuncAndFormat(functionName,
                                                          fmtString.c_str(),
                                                          args,
                                                          requestLen);

    nlstring requestFunc(100u, ' ');
    CEResult_t requestResult;
    vector<void *> response;
    response.reserve(1024);
    nlstring reqID(reqIDStr);

    CEResult_t res = PEPMAN_RPCCall(reqID, marshaledRequest, requestLen, handle, requestFunc, requestResult, response, timeout_in_millisec);

    if(res != CE_RESULT_SUCCESS) {
      Marshal_PackFree(marshaledRequest); 
      response.clear(); 
      CEM_FreeString(reqIDArg);
      return res;
    }


    // Free the request
    Marshal_PackFree(marshaledRequest);

    // reqID
    CEM_FreeString(reqIDArg);
    // num requests
    delete (CEint32 *)args[1];

    for (int i = 0; i < numRequests; ++i)
    {
      CEAttributes *dimensions = (CEAttributes *)args[i*4 +2];
      FreeLocalAllocatedAttrs(*dimensions);
      delete dimensions;

      // Get each of the members of the attributes array and free them
      CEAttributes_Array *aattrs = (CEAttributes_Array *)args[i*4 + 3];
      for (int j = 0; j < aattrs->count; ++j)
      {
        FreeLocalAllocatedAttrs(aattrs->attrs_array[j]);
      }

      // And then the attributes array itself
      delete [] aattrs->attrs_array;
      delete aattrs;

      // noise level and perform obligation
      delete (CEint32 *)args[i*4 + 4];
      delete (CEBoolean *)args[i*4 + 5];
    }


    // Unpack the response
    // We don't actually need to check the fmt string, we know what's there

    // The first three arguments are
    // fmt string
    // req id
    // number responses
    for (int i = 0; i < numRequests; ++i)
    {
      const CEEnforcement_t enf = { (CEResponse_t)*(CEint32 *)response[i*2+3],
                                    (CEAttributes *)response[i*2+4]};

      copyEnforcementData(&enf, &enforcements[i]);
    }

    Marshal_UnPackFree(functionName, response, false); 

    return requestResult;        
  } catch (std::exception &e) {
    return CE_RESULT_GENERAL_FAILED;
  } catch (...) {
    return CE_RESULT_GENERAL_FAILED;
  }
}

/* ------------------------------------------------------------------------
 * CEEVALUATE_CheckResource()
 * asks Policy Controller to evaluation the operation on resources. 
 *
 * Parameters
 * handle (INPUT): handle from the CONN_Initialize().
 * operation (INPUT): operation in CEString type.
 * source (INPUT): the source resource in CEResource type. 
 * sourceAttributes (INPUT): associate attributes of the source. This can be 
 *   NULL if no associate attributes.
 * target (INPUT): the target resource in CEResource type. This can be NULL 
 *   if the operation has no target associated.  
 * targetAttributes (INPUT): Associate attributes of the target. This can be
 *   NULL if no associate attributes. 
 * user (INPUT): the user who access the resources.
 * userAttributes(INPUT): Associate attributes of the user. This can be NULL 
 *   if no associate user attribute.   
 * app (INPUT): the application which access this resource
 * appAttributes(INPUT): Associate attributes of the application. This can 
 *   be NULL if no associate application attribute.   
 * recipients (INPUT): The string array of recipients for the messaging case,
 *    e.g. the source is attached with email. This can be NULL if no 
 *    recipients. 
 * numRecipients (INPUT): The number of recipients for the messaging case, 
 *   e.g. the source is attached with email.   
 * ipNumber (INPUT): the IP address of client machine.
 * performObligation (INPUT): perform the obligation defined by the policy 
 *   (e.g. logging / email)
 * noiseLevel (INPUT): desirable noise level to be used for this evaluation.
 * enforcement (OUTPUT): result of the policy for enforcement.
 * timeout_in_millisec (INPUT): desirable timeout in milliseconds for this 
 *   evaluation
 * 
 * Return
 * 
 * If the evaluation succeeds, the return value is CE_RESULT_SUCCESS; 
 * otherwise, it returns error value, e.g. CE_RESULT_NULL_CEHANDLE.
 * ------------------------------------------------------------------------
 */ 
CEResult_t CEEVALUATE_CheckResources(CEHandle handle, 
                                     const CEString operation,
                                     const CEResource* source,           
                                     const CEAttributes * sourceAttributes,
                                     const CEResource* target,           
                                     const CEAttributes * targetAttributes,
                                     const CEUser  *user,
                                     CEAttributes * userAttributes,
                                     CEApplication *app,
                                     CEAttributes * appAttributes,
                                     CEString *recipients,
                                     CEint32 numRecipients,
                                     const CEint32 ipNumber,
                                     const CEBoolean performObligation,
                                     const CENoiseLevel_t noiseLevel,
                                     CEEnforcement_t * enforcement,
                                     const CEint32 timeout_in_millisec)
{

#ifdef WIN32
  nextlabs::high_resolution_timer ht;
#endif

  try {    
    // Initialize the output parameters first for defensive programming 
    if (enforcement) {
      memset (enforcement, 0x0, sizeof (CEEnforcement_t));
      enforcement->result = CEAllow;
    }

    if(timeout_in_millisec == 0) {
      return CE_RESULT_TIMEDOUT;
    }

    CEResult_t result=CheckResourceInputs(handle, operation, 
                                          source, sourceAttributes,
                                          target, targetAttributes, 
                                          timeout_in_millisec);
    if(result != CE_RESULT_SUCCESS)
      return result;
   
    int numDimension=8;
    const nlchar *dimensionAttrs[8]={_T("from"), _T("host"),
                                     _T("application"), _T("user"), 
                                     _T("action"), _T("to"), _T("sendto"),
                                     _T("operating-system-user")};
    CEAttributes dimensions;
    CEAttributes *attrsMatrix;
    CEResult_t reqResult=CE_RESULT_SUCCESS;

    //Compose attribute dimension
    ComposeAttrDimension(&dimensions, numDimension, dimensionAttrs);

    //Compose dynamic attributes list
    attrsMatrix=new CEAttributes[numDimension];

    //from attributes
    ComposeResourceAttrs(&(attrsMatrix[0]), sourceAttributes,
                         source);
    
    //host attributes: add the default attribute "inet_address" 
    AddHostAttributes(&(attrsMatrix[1]), ipNumber);

    //application attributes
    ComposeAppAttrs(&(attrsMatrix[2]), app, handle, appAttributes);

    //user attributes
    AddUserDefaultAttrs(&(attrsMatrix[3]), handle, user?user->userName:NULL,
                        user?user->userID:NULL,false, userAttributes);

    //action attributes
    ComposeActionAttrs(&(attrsMatrix[4]), operation);

    //to attributes
    ComposeResourceAttrs(&(attrsMatrix[5]), targetAttributes,
                         target);
    
    //sendto attributes
    ComposeSendToAttrs(&(attrsMatrix[6]), numRecipients, recipients);

    // operating-system-user attributes; 
    // CheckMsgAttach use current process user's id
    ComposeOSAttrs(&(attrsMatrix[7]), true, user?user->userID:NULL);

    //Call CheckMetaData
    reqResult= CEEVALUATE_CheckMetadata (handle, 
                                         &dimensions,
                                         attrsMatrix,
                                         noiseLevel,
                                         performObligation,
                                         enforcement,
                                         timeout_in_millisec);

    //Free local allocated memory
    FreeLocalAllocatedAttrs(dimensions);
    for(int i=0; i<numDimension; i++) 
      FreeLocalAllocatedAttrs(attrsMatrix[i]);
    delete [] attrsMatrix;
    
#ifdef WIN32
    ht.stop();
#endif

    //Return result
    return reqResult;        
  } catch (std::exception &e) {
    return CE_RESULT_GENERAL_FAILED;
  } catch (...) {

    return CE_RESULT_GENERAL_FAILED;
  }
  //  return CE_RESULT_SUCCESS;   
}/*--CSCINVOKE_CEEVALUATE_CheckResources--*/

/* ------------------------------------------------------------------------
 * CEEVALUATE_CheckPortal()
 *
 * Ask the Policy Decision Point Server to evaluate the operation on 
 * portal resource
 * 
 * Arguments : 
 * handle (INPUT): Handle from the CONN_Initialize()
 * Operation (INPUT): Operation on the file
 * sourceURL (INPUT): the url of the source resource
 * targetURL (INPUT): the url of the target resource
 * performObligation (INPUT): Perform the obligation defined by the policy 
 *                            (e.g. logging / email)
 * sourceAttributes (INPUT): Associate attributes of the source
 * targetAttributes (INPUT): Associate attributes of the target
 * noiseLevel (INPUT): Desirable noise level to be used for this evaluation
 * ipNumber (INPUT): the ip address of client machine
 * user (INPUT): the user who access this URL.
 * timeout_in_millisec (INPUT): Desirable timeout in milliseconds for 
 * this evaluation
 * Enforcement (OUTPUT): Result of the policy for enforcement
 *
 * ------------------------------------------------------------------------
 */ 
CEResult_t CEEVALUATE_CheckPortal(CEHandle handle, 
                                  CEAction_t operation, 
                                  CEString sourceURL, 
                                  CEAttributes * sourceAttributes,
                                  CEString targetURL,
                                  CEAttributes * targetAttributes,
                                  CEint32 ipNumber,
                                  CEUser  *user,
                                  CEBoolean performObligation,
                                  CENoiseLevel_t noiseLevel,
                                  CEEnforcement_t * enforcement,
                                  CEint32 timeout_in_millisec)
{
#ifdef WIN32
  nextlabs::high_resolution_timer ht;
#endif
  
  try {
    // Initialize the output parameters first for defensive programming 
    if (enforcement) {
      memset (enforcement, 0x0, sizeof (CEEnforcement_t));
      enforcement->result = CEAllow;
    }

    if(timeout_in_millisec == 0) {
      return CE_RESULT_TIMEDOUT;
    }
    
    CEResult_t result=CheckPortalInputs(handle, operation, 
                                        sourceURL, sourceAttributes,
                                        targetURL, targetAttributes, user,
                                        timeout_in_millisec);
    if(result != CE_RESULT_SUCCESS)
      return result;

    int numDimension=7;
    const nlchar *dimensionAttrs[7]={_T("from"), _T("host"), 
                                     _T("application"),
                                     _T("user"), _T("action"), _T("to"),
                                     _T("operating-system-user")};
    CEAttributes dimensions;
    CEAttributes *attrsMatrix;
    CEResult_t reqResult=CE_RESULT_SUCCESS;

    //Compose attribute dimension
    ComposeAttrDimension(&dimensions, numDimension, dimensionAttrs);

    //Compose dynamic attributes list
    attrsMatrix=new CEAttributes[numDimension];

    //from attributes
    ComposeResourceAttrs(&(attrsMatrix[0]), sourceAttributes,
                         _T("portal"), _T("url"), sourceURL);
    
    //host attributes: add the default attribute "inet_address" 
    AddHostAttributes(&(attrsMatrix[1]), ipNumber);

    //application attributes
    //For portal, application attriubte is empty
    attrsMatrix[2].count=0;
    attrsMatrix[2].attrs=NULL;

    //user attributes
    AddUserDefaultAttrs(&(attrsMatrix[3]), handle, 
                        user?user->userName:NULL,
                        user?user->userID:NULL);

    //action attributes
    ComposeActionAttrs(&(attrsMatrix[4]), operation);
    
    //to attributes
    ComposeResourceAttrs(&(attrsMatrix[5]), targetAttributes,
                         _T("portal"), _T("url"), targetURL);

    // operating-system-user attributes
    // CheckMsgAttach use current process user's id
    ComposeOSAttrs(&(attrsMatrix[6]), true, user?user->userID:NULL);

    //Call CheckMetaData
    reqResult= CEEVALUATE_CheckMetadata (handle, 
                                         &dimensions,
                                         attrsMatrix,
                                         noiseLevel,
                                         performObligation,
                                         enforcement,
                                         timeout_in_millisec);

    //Free local allocated memory
    FreeLocalAllocatedAttrs(dimensions);
    for(int i=0; i<numDimension; i++) 
      FreeLocalAllocatedAttrs(attrsMatrix[i]);
    delete [] attrsMatrix;
    
#ifdef WIN32
    ht.stop();
#endif
    //Return result
    return reqResult;    
  } catch (std::exception &e) {
    return CE_RESULT_GENERAL_FAILED;
  } catch (...) {
    return CE_RESULT_GENERAL_FAILED;
  }
}

/* ------------------------------------------------------------------------
 * CEEVALUATE_CheckFile()
 *
 * Ask the Policy Decision Point Server to evaluate the operation on 
 * file resource
 * 
 * Arguments : 
 * handle (INPUT): Handle from the CONN_Initialize()
 * Operation (INPUT): Operation on the file
 * sourceFullFileName (INPUT): the full name of the source resource
 * targetFullFileName (INPUT): the full name of the target resource
 * performObligation (INPUT): Perform the obligation defined by the policy 
 *                            (e.g. logging / email)
 * sourceAttributes (INPUT): Associate attributes of the source
 * targetAttributes (INPUT): Associate attributes of the target
 * noiseLevel (INPUT): Desirable noise level to be used for this evaluation
 * ipNumber (INPUT): the ip address of client machine
 * user (INPUT): the user who access this file
 * app (INPUT): the application which access this file
 * timeout_in_millisec (INPUT): Desirable timeout in milliseconds for 
 * this evaluation
 * Enforcement (OUTPUT): Result of the policy for enforcement
 *
 * ------------------------------------------------------------------------
 */ 
CEResult_t CEEVALUATE_CheckFile(CEHandle handle, 
                                CEAction_t operation, 
                                CEString sourceFullFileName, 
                                CEAttributes * sourceAttributes,
                                CEString targetFullFileName,
                                CEAttributes * targetAttributes,
                                CEint32 ipNumber,
                                CEUser  *user,
                                CEApplication *app,
                                CEBoolean performObligation,
                                CENoiseLevel_t noiseLevel,
                                CEEnforcement_t * enforcement,
                                CEint32 timeout_in_millisec)
{
#ifdef WIN32
  nextlabs::high_resolution_timer ht;
#endif

  try {
    // Should initialize the output first, otherwise, we are returning
    // a bogus output if there is error. e.g. timeout
    if (enforcement) {
      memset (enforcement, 0x0, sizeof (CEEnforcement_t));
      enforcement->result = CEAllow;
    }

    if(timeout_in_millisec == 0) {
      return CE_RESULT_TIMEDOUT;
    }
    
    CEResult_t result=CheckFileInputs(handle, operation, 
                                      sourceFullFileName, sourceAttributes,
                                      targetFullFileName, targetAttributes, 
                                      user, timeout_in_millisec);
    if(result != CE_RESULT_SUCCESS) 
      return result;

    int numDimension=7;
    const nlchar *dimensionAttrs[7]={_T("from"), _T("host"),
                                     _T("application"), _T("user"), 
                                     _T("action"), _T("to"),
                                     _T("operating-system-user")};
    CEAttributes dimensions;
    CEAttributes *attrsMatrix;
    CEResult_t reqResult=CE_RESULT_SUCCESS;

    //Compose attribute dimension
    ComposeAttrDimension(&dimensions, numDimension, dimensionAttrs);

    //Compose dynamic attributes list
    attrsMatrix=new CEAttributes[numDimension];

    //from attributes
    ComposeResourceAttrs(&(attrsMatrix[0]), sourceAttributes,
                         _T("fso"), _T("CE::resolved_name"), 
                         sourceFullFileName);

    //host attributes: add the default attribute "inet_address" 
    AddHostAttributes(&(attrsMatrix[1]), ipNumber);

    //application attributes
    ComposeAppAttrs(&(attrsMatrix[2]), app, handle);

    //user attributes
    AddUserDefaultAttrs(&(attrsMatrix[3]), handle, user?user->userName:NULL,
                        user?user->userID:NULL);
    
    //action attributes
    ComposeActionAttrs(&(attrsMatrix[4]), operation);

    //to attributes
    ComposeResourceAttrs(&(attrsMatrix[5]), targetAttributes,
                         _T("fso"), _T("CE::resolved_name"), 
                         targetFullFileName);
    
    // operating-system-user attributes; 
    // CheckMsgAttach use current process user's id
    ComposeOSAttrs(&(attrsMatrix[6]), true, user?user->userID:NULL);

    //Call CheckMetaData
    reqResult= CEEVALUATE_CheckMetadata (handle, 
                                         &dimensions,
                                         attrsMatrix,
                                         noiseLevel,
                                         performObligation,
                                         enforcement,
                                         timeout_in_millisec);

    //Free local allocated memory
    FreeLocalAllocatedAttrs(dimensions);
    for(int i=0; i<numDimension; i++) 
      FreeLocalAllocatedAttrs(attrsMatrix[i]);
    delete [] attrsMatrix;
    
#ifdef WIN32
    ht.stop();
#endif
    //Return result
    return reqResult;    
  } catch (std::exception &e) {
    return CE_RESULT_GENERAL_FAILED;
  } catch (...) {
    return CE_RESULT_GENERAL_FAILED;
  }
}

/* ------------------------------------------------------------------------
 * CEEVALUATE_CheckMessageAttachment()
 *
 * Ask the Policy Decision Point Server to evaluate the operation on 
 * sending message with attchment
 * 
 * Arguments : 
 * handle (INPUT): Handle from the CONN_Initialize()
 * Operation (INPUT): Operation on the message
 * sourceFullFileName (INPUT): the full name of the source resource
 * numRecipients (INPUT): the number of message recipients
 * recipients (INPUT): the CEString array of message recipients
 * performObligation (INPUT): Perform the obligation defined by the policy 
 *                            (e.g. logging / email)
 * sourceAttributes (INPUT): Associate attributes of the source
 * userAttributes (INPUT): Associate attributes of the user
 * appAttributes (INPUT): Associate attributes of the application
 * noiseLevel (INPUT): Desirable noise level to be used for this evaluation
 * ipNumber (INPUT): the ip address of client machine
 * user (INPUT): the user who access this file
 * timeout_in_millisec (INPUT): Desirable timeout in milliseconds for 
 * this evaluation
 * Enforcement (OUTPUT): Result of the policy for enforcement
 *
 * ------------------------------------------------------------------------
 */ 
CEResult_t 
CEEVALUATE_CheckMessageAttachment(CEHandle handle, 
                                  CEAction_t operation, 
                                  CEString sourceFullFileName, 
                                  CEAttributes * sourceAttributes,
                                  CEint32 numRecipients,
                                  CEString *recipients,
                                  CEint32 ipNumber,
                                  CEUser  *user,
                                  CEAttributes * userAttributes,
                                  CEApplication *app,
                                  CEAttributes * appAttributes,
                                  CEBoolean performObligation,
                                  CENoiseLevel_t noiseLevel,
                                  CEEnforcement_t * enforcement,
                                  CEint32 timeout_in_millisec)
{
#ifdef WIN32
  nextlabs::high_resolution_timer ht;
#endif
  
  try {
    // Should initialize the output first, otherwise, we are returning
    // a bogus output if there is error. e.g. timeout
    if (enforcement) {
      memset (enforcement, 0x0, sizeof (CEEnforcement_t));
      enforcement->result = CEAllow;
    }

    if(timeout_in_millisec == 0) {
      return CE_RESULT_TIMEDOUT;
    }
    
    CEResult_t result=CheckMsgAttachmentInput(handle, operation, 
                                              sourceFullFileName, 
                                              sourceAttributes,
                                              numRecipients, recipients,user,
                                              timeout_in_millisec);

    if(result != CE_RESULT_SUCCESS) 
      return result;

    int numDimension=7;
    const nlchar *dimensionAttrs[7]={_T("from"), _T("host"), 
                                     _T("application"),
                                     _T("user"), _T("action"), 
                                     _T("sendto"), 
                                     _T("operating-system-user")};
    CEAttributes dimensions;
    CEAttributes *attrsMatrix;
    CEResult_t reqResult=CE_RESULT_SUCCESS;

    //Compose attribute dimension
    ComposeAttrDimension(&dimensions, numDimension, dimensionAttrs);

    //Compose dynamic attributes list
    attrsMatrix=new CEAttributes[numDimension];

    //from attributes 
    ComposeResourceAttrs(&(attrsMatrix[0]), sourceAttributes,
                         _T("fso"), _T("CE::resolved_name"), 
                         sourceFullFileName);

    //host attributes: add the default attribute "inet_address" 
    AddHostAttributes(&(attrsMatrix[1]), ipNumber);

    //application attributes
    ComposeAppAttrs(&(attrsMatrix[2]), app, handle, appAttributes);

    //user attributes
    AddUserDefaultAttrs(&(attrsMatrix[3]), handle, user?user->userName:NULL,
                        user?user->userID:NULL, true, userAttributes);
    
    //action attributes
    ComposeActionAttrs(&(attrsMatrix[4]), operation);

    //sendto attributes
    ComposeSendToAttrs(&(attrsMatrix[5]), numRecipients,recipients);
    
    // operating-system-user attributes
    // CheckMsgAttach use current process user's id
    ComposeOSAttrs(&(attrsMatrix[6]), false, NULL );

    //Call CheckMetaData
    reqResult= CEEVALUATE_CheckMetadata (handle, 
                                         &dimensions,
                                         attrsMatrix,
                                         noiseLevel,
                                         performObligation,
                                         enforcement,
                                         timeout_in_millisec);

    //Free local allocated memory
    FreeLocalAllocatedAttrs(dimensions);
    for(int i=0; i<numDimension; i++) {
      FreeLocalAllocatedAttrs(attrsMatrix[i]);
    }
    delete [] attrsMatrix;
    
#ifdef WIN32
    ht.stop();
#endif
    //Return result
    return reqResult;    
    
  } catch (std::exception &e) {
    return CE_RESULT_GENERAL_FAILED;
  } catch (...) {
    return CE_RESULT_GENERAL_FAILED;
  }
}

/* ------------------------------------------------------------------------
 * CEEVALUATE_FreeEnforcement
 *
 * Basic service to free a CE-type CEEnforcement_t from the system
 *
 * Arguments : enforcement      : CE-type CEEnforcement_t to be freed
 *
 * ------------------------------------------------------------------------
 */ 
CEResult_t CEEVALUATE_FreeEnforcement(CEEnforcement_t enf)
{
  if(enf.obligation == NULL) 
    return CE_RESULT_SUCCESS;

  for(int i=0; i<enf.obligation->count; i++) {
    if(enf.obligation->attrs[i].key)
      delete enf.obligation->attrs[i].key;
    if(enf.obligation->attrs[i].value)
      delete enf.obligation->attrs[i].value;
  }
  delete [] enf.obligation->attrs;
  delete enf.obligation;

  return CE_RESULT_SUCCESS;
}
/* ------------------------------------------------------------------------
 * CSCINVOKE_CEEVALUATE_CheckPortal()
 *
 * Ask the Policy Decision Point Server to evaluate the operation.
 *
 * Since .net marshaler doesn't support struct array, calling the function
 * 'CEEVALUATE_CheckPortal' from C# application needs lots of hacking
 * and manual marshaling code. This function is provided to simplify
 * the marshaling in application code.
 * 
 * This function is for the application in C#. It is called through the 
 * the platform invoke method of C#. 
 *
 * Arguments : 
 * handle (INPUT): Handle from the CONN_Initialize()
 * Operation (INPUT): Operation on the file
 * sourceURL (INPUT): url of source resource
 * targetURL (INPUT): url of target resource
 * sourceAttributes (INPUT): Associate attributes of the source. This is
 *   a string array in the order of "key-1""value-1""key-2""value-2"...
 * targetAttributes (INPUT): Associate attributes of the target. This is
 *   a string array in the order of "key-1""value-1""key-2""value-2"...
 * userName (INPUT): name of the user who access this URL.
 * userID (INPUT): id of the user who access this URL.
 * performObligation (INPUT): Perform the obligation defined by the policy 
 *                            (e.g. logging / email)
 * noiseLevel (INPUT): Desirable noise level to be used for this evaluation
 * ipAddress (INPUT): For Sharepointe, the ip address of client machine
 * timeout_in_millisec (INPUT): Desirable timeout in milliseconds for 
 * this evaluation
 * enforcement_ob (OUTPUT): the resulted enforcement obligation  
 * from the policy decision point server. This is a string array in the 
 * order of "key-1""value-1""key-2""value-2"...
 * enforcement_result (OUTPUT): the resulted enforcement integer decision.
 * enforcement_ob (OUTPUT): Result of the policy for enforcement obligations.
 *   This is a string array in the order of 
 *   "key-1""value-1""key-2""value-2"...
 * enforcement_result (OUTPUT): Result of the policy for enforcement result
 * numEnforcements (OUTPUT): number of resulted enforcement obligations
 * ------------------------------------------------------------------------
 */ 
CEResult_t CSCINVOKE_CEEVALUATE_CheckPortal(CEHandle handle, 
                                            CEAction_t operation, 
                                            CEString sourceURL, 
                                            nlchar **sourceAttributesStrs,
                                            CEint32 numSourceAttributes,
                                            CEString targetURL,
                                            nlchar **targetAttributesStrs,
                                            CEint32 numTargetAttributes,
                                            CEint32 ipAddress,
                                            CEString userName,
                                            CEString userID,
                                            CEBoolean performObligation,
                                            CENoiseLevel_t noiseLevel,
                                            nlchar ***enforcement_ob,
                                            CEResponse_t *enforcement_result,
                                            CEint32 *numEnforcements,
                                            CEint32 timeout_in_millisec)
{
  try {
    CEEnforcement_t enf;
    CEAttributes sourceAttributes;
    CEAttributes targetAttributes;
    CEResult_t result;

    result=CSCINVOKE_CheckInputs(handle, sourceURL, targetURL,
                                 sourceAttributesStrs, numSourceAttributes,
                                 targetAttributesStrs, numTargetAttributes);
    if(result != CE_RESULT_SUCCESS)
      return result;

    //Construct source attributes
    sourceAttributes.attrs = new CEAttribute[numSourceAttributes/2];
    sourceAttributes.count = numSourceAttributes/2;
    for(int i=0, j=0; i<numSourceAttributes; i++, j++) {
      CEString key = new struct _CEString();
      key->length = nlstrlen(sourceAttributesStrs[i]);
      key->buf = new nlchar[key->length+1];
      nlstrncpy_s(key->buf, key->length+1, sourceAttributesStrs[i],
                  _TRUNCATE);

      i++;

      CEString value = new struct _CEString();
      value->length = nlstrlen(sourceAttributesStrs[i]);
      value->buf = new nlchar[value->length+1];
      nlstrncpy_s(value->buf, value->length+1, sourceAttributesStrs[i],
                  _TRUNCATE);

      SetAttribute(&sourceAttributes.attrs[j], key, value);
    }

    //Construct target attributes
    if(numTargetAttributes != 0) {
      targetAttributes.attrs = new CEAttribute[numTargetAttributes/2];
      targetAttributes.count = numTargetAttributes/2;
      for(int i=0, j=0; i<numTargetAttributes; i++, j++) {
        CEString key = new struct _CEString();
        key->length = nlstrlen(targetAttributesStrs[i]);
        key->buf = new nlchar[key->length+1];
        nlstrncpy_s(key->buf, key->length+1, targetAttributesStrs[i],
                    _TRUNCATE);

        i++;

        CEString value = new struct _CEString();
        value->length = nlstrlen(targetAttributesStrs[i]);
        value->buf = new nlchar[value->length+1];
        nlstrncpy_s(value->buf, value->length+1, targetAttributesStrs[i],
                    _TRUNCATE);

        SetAttribute(&targetAttributes.attrs[j], key, value);
      }
    } else {
      targetAttributes.attrs = NULL;
      targetAttributes.count = 0;
    }

    CEUser user;
    user.userName=userName;
    user.userID=userID;
    CEResult_t res=CEEVALUATE_CheckPortal(handle, operation,
                                          sourceURL, 
                                          &sourceAttributes,
                                          targetURL,
                                          &targetAttributes,
                                          ipAddress,
                                          &user,
                                          performObligation,
                                          noiseLevel,
                                          &enf,
                                          timeout_in_millisec);
 
    //Destruct source attributes
    for(int i=0; i<sourceAttributes.count; i++) {
      delete sourceAttributes.attrs[i].key;
      delete sourceAttributes.attrs[i].value;
    }
    delete [] sourceAttributes.attrs;
    
    //Destruct target attributes
    if(numTargetAttributes > 0) {
      for(int i=0; i<targetAttributes.count; i++) {
        delete targetAttributes.attrs[i].key;
        delete targetAttributes.attrs[i].value;
      }
      delete [] targetAttributes.attrs;
    }

    //Assign enforcement result
    *enforcement_result = enf.result;
    if(enf.obligation == NULL)
      *numEnforcements=0;
    else {
      *numEnforcements = 2*enf.obligation->count;
      *enforcement_ob = new nlchar*[2*enf.obligation->count];
      size_t bufLen;
      for(int i=0, j=0; i<enf.obligation->count; i++, j++) {
        //enforcement attribute key
        if(enf.obligation->attrs[i].key){
          bufLen=enf.obligation->attrs[i].key->length;
          (*enforcement_ob)[j] = new nlchar[bufLen+1];
          if(enf.obligation->attrs[i].key->buf)
            nlstrncpy_s((*enforcement_ob)[j], bufLen+1,
                        enf.obligation->attrs[i].key->buf,
                        _TRUNCATE);
          else
            (*enforcement_ob)[j][0]='\0';
        } else
          (*enforcement_ob)[j] = NULL;

        j++;

        //enforcement attribute value
        if(enf.obligation->attrs[i].value) {
          bufLen=enf.obligation->attrs[i].value->length;
          (*enforcement_ob)[j] = new nlchar[bufLen+1];
          if(enf.obligation->attrs[i].value->buf)
            nlstrncpy_s((*enforcement_ob)[j], bufLen+1,
                        enf.obligation->attrs[i].value->buf,
                        _TRUNCATE);
          else
            (*enforcement_ob)[j][0]='\0';
        } else
          (*enforcement_ob)[j]=NULL;
   
      }
    }

    //Destruct enf
    if(enf.obligation)
      CEEVALUATE_FreeEnforcement(enf);

    //Return result
    return res;    
  } catch (std::exception &e) {
    return CE_RESULT_GENERAL_FAILED;
  } catch (...) {
    return CE_RESULT_GENERAL_FAILED;
  }
}
/* ------------------------------------------------------------------------
 * CSCINVOKE_CEEVALUATE_CheckFile()
 *
 * Ask the Policy Decision Point Server to evaluate the operation on files.
 *
 * Since .net marshaler doesn't support struct array, calling the function
 * 'CEEVALUATE_CheckFile' from C# application needs lots of hacking
 * and manual marshaling code. This function is provided to simplify
 * the marshaling in application code.
 * 
 * This function is for the application in C#. It is called through the 
 * the platform invoke method of C#. 
 *
 * Arguments : 
 * handle (INPUT): Handle from the CONN_Initialize()
 * Operation (INPUT): Operation on the file
 * sourceFile (INPUT): url of source resource
 * targetFile (INPUT): url of target resource
 * sourceAttributes (INPUT): Associate attributes of the source. This is
 *   a string array in the order of "key-1""value-1""key-2""value-2"...
 * targetAttributes (INPUT): Associate attributes of the target. This is
 *   a string array in the order of "key-1""value-1""key-2""value-2"...
 * userName (INPUT): name of the user who access this URL.
 * userID (INPUT): id of the user who access this URL.
 * appName (INPUT): name of the application that accesses the file
 * appPath (INPUT): the full path of the application. (Optional)
 * appURL (INPUT): the url that a web application is visiting. 
 * performObligation (INPUT): Perform the obligation defined by the policy 
 *                            (e.g. logging / email)
 * noiseLevel (INPUT): Desirable noise level to be used for this evaluation
 * ipAddress (INPUT): For Sharepointe, the ip address of client machine
 * timeout_in_millisec (INPUT): Desirable timeout in milliseconds for 
 * this evaluation
 * enforcement_ob (OUTPUT): the resulted enforcement obligation  
 * from the policy decision point server. This is a string array in the 
 * order of "key-1""value-1""key-2""value-2"...
 * enforcement_result (OUTPUT): the resulted enforcement integer decision.
 * enforcement_ob (OUTPUT): Result of the policy for enforcement obligations.
 *   This is a string array in the order of 
 *   "key-1""value-1""key-2""value-2"...
 * enforcement_result (OUTPUT): Result of the policy for enforcement result
 * numEnforcements (OUTPUT): number of resulted enforcement obligations
 * ------------------------------------------------------------------------
 */ 
CEResult_t CSCINVOKE_CEEVALUATE_CheckFile(CEHandle handle, 
                                          CEAction_t operation, 
                                          CEString sourceURL, 
                                          nlchar **sourceAttributesStrs,
                                          CEint32 numSourceAttributes,
                                          CEString targetURL,
                                          nlchar **targetAttributesStrs,
                                          CEint32 numTargetAttributes,
                                          CEint32 ipAddress,
                                          CEString userName,
                                          CEString userID,
                                          CEString appName,
                                          CEString appPath,
                                          CEString appURL,
                                          CEBoolean performObligation,
                                          CENoiseLevel_t noiseLevel,
                                          nlchar ***enforcement_ob,
                                          CEResponse_t *enforcement_result,
                                          CEint32 *numEnforcements,
                                          CEint32 timeout_in_millisec)
{
  try {
    CEEnforcement_t enf;
    CEAttributes sourceAttributes;
    CEAttributes targetAttributes;
    
    CEResult_t result=CSCINVOKE_CheckInputs(handle, sourceURL, targetURL,
                                            sourceAttributesStrs, 
                                            numSourceAttributes,
                                            targetAttributesStrs, 
                                            numTargetAttributes);
    if(result != CE_RESULT_SUCCESS) 
      return result;

    //Construct source attributes
    sourceAttributes.attrs = new CEAttribute[numSourceAttributes/2];
    sourceAttributes.count = numSourceAttributes/2;
   
    for(int i=0, j=0; i<numSourceAttributes; i++, j++) {
      CEString key = new struct _CEString();
      key->length = nlstrlen(sourceAttributesStrs[i]);
      key->buf = new nlchar[key->length+1];
      nlstrncpy_s(key->buf, key->length+1, sourceAttributesStrs[i],
                  _TRUNCATE);

      i++;

      CEString value = new struct _CEString();
      value->length = nlstrlen(sourceAttributesStrs[i]);
      value->buf = new nlchar[value->length+1];
      nlstrncpy_s(value->buf, value->length+1, sourceAttributesStrs[i],
                  _TRUNCATE);

      SetAttribute(&sourceAttributes.attrs[j], key, value);

    }

    //Construct target attributes
    if(numTargetAttributes != 0) {
      targetAttributes.attrs = new CEAttribute[numTargetAttributes/2];
      targetAttributes.count = numTargetAttributes/2;
      for(int i=0, j=0; i<numTargetAttributes; i++, j++) {
        CEString key = new struct _CEString();
        key->length = nlstrlen(targetAttributesStrs[i]);
        key->buf = new nlchar[key->length+1];
        nlstrncpy_s(key->buf, key->length+1, targetAttributesStrs[i],
                    _TRUNCATE);


        i++;

        CEString value = new struct _CEString();
        value->length = nlstrlen(targetAttributesStrs[i]);
        value->buf = new nlchar[value->length+1];
        nlstrncpy_s(value->buf, value->length+1, targetAttributesStrs[i],
                    _TRUNCATE);

        SetAttribute(&targetAttributes.attrs[j], key, value);
      }
    } else {
      targetAttributes.attrs = NULL;
      targetAttributes.count = 0;
    }

    CEUser user;
    user.userName=userName;
    user.userID=userID;
    CEApplication app;
    app.appName=appName;
    app.appPath=appPath;
    app.appURL=appURL;
    CEResult_t res=CEEVALUATE_CheckFile(handle, 
                                        operation,
                                        sourceURL, 
                                        &sourceAttributes,
                                        targetURL,
                                        &targetAttributes,
                                        ipAddress,
                                        &user,
                                        &app,
                                        performObligation,
                                        noiseLevel,
                                        &enf,
                                        timeout_in_millisec);
 
    //Destruct source attributes
    for(int i=0; i<sourceAttributes.count; i++) {
      delete sourceAttributes.attrs[i].key;
      delete sourceAttributes.attrs[i].value;
    }
    delete [] sourceAttributes.attrs;
    
    //Destruct target attributes
    if(numTargetAttributes > 0) {
      for(int i=0; i<targetAttributes.count; i++) {
        delete targetAttributes.attrs[i].key;
        delete targetAttributes.attrs[i].value;
      }
      delete [] targetAttributes.attrs;
    }

    //Assign enforcement result
    *enforcement_result = enf.result;
    if(enf.obligation == NULL)
      *numEnforcements=0;
    else {
      *numEnforcements = 2*enf.obligation->count;
      *enforcement_ob = new nlchar*[2*enf.obligation->count];
      size_t bufLen;
      for(int i=0, j=0; i<enf.obligation->count; i++, j++) {
        //enforcement attribute key
        if(enf.obligation->attrs[i].key) {
          bufLen=enf.obligation->attrs[i].key->length;
          (*enforcement_ob)[j] = new nlchar[bufLen+1];
          if(enf.obligation->attrs[i].key->buf)
            nlstrncpy_s((*enforcement_ob)[j], bufLen+1,
                        enf.obligation->attrs[i].key->buf,
                        _TRUNCATE);
          else
            (*enforcement_ob)[j][0]='\0';
        } else
          (*enforcement_ob)[j]= NULL;
   

        j++;

        //enforcement attribute value
        if(enf.obligation->attrs[i].value){
          bufLen=enf.obligation->attrs[i].value->length;
          (*enforcement_ob)[j] = new nlchar[bufLen+1];
          if(enf.obligation->attrs[i].value->buf)
            nlstrncpy_s((*enforcement_ob)[j], bufLen+1,
                        enf.obligation->attrs[i].value->buf,
                        _TRUNCATE);
          else
            (*enforcement_ob)[j][0]='\0';
        }else
          (*enforcement_ob)[j]=NULL;
      }
    }

    //Destruct enf
    if(enf.obligation)
      CEEVALUATE_FreeEnforcement(enf);

    //Return result
    return res;    
  } catch (std::exception &e) {

    return CE_RESULT_GENERAL_FAILED;
  } catch (...) {
 
    return CE_RESULT_GENERAL_FAILED;
  }
}

/* ------------------------------------------------------------------------
 * CSCINVOKE_CEEVALUATE_CheckMessageAttachment()
 *
 * Ask the Policy Decision Point Server to evaluate the operation.
 *
 * Since .net marshaler doesn't support struct array, calling the function
 * 'CEEVALUATE_CheckMessageAttachment' from C# application needs lots of 
 * hacking
 * and manual marshaling code. This function is provided to simplify
 * the marshaling in application code.
 * 
 * This function is for the application in C#. It is called through the 
 * the platform invoke method of C#. 
 *
 * Arguments : 
 * handle (INPUT): Handle from the CONN_Initialize()
 * Operation (INPUT): Operation on the file
 * sourceURL (INPUT): url of source resource
 * targetURL (INPUT): url of target resource
 * sourceAttributes (INPUT): Associate attributes of the source. This is
 *   a string array in the order of "key-1""value-1""key-2""value-2"...
 * targetAttributes (INPUT): Associate attributes of the target. This is
 *   a string array in the order of "key-1""value-1""key-2""value-2"...
 * userName (INPUT): name of the user who access this URL.
 * userID (INPUT): id of the user who access this URL.
 * performObligation (INPUT): Perform the obligation defined by the policy 
 *                            (e.g. logging / email)
 * noiseLevel (INPUT): Desirable noise level to be used for this evaluation
 * ipAddress (INPUT): For Sharepointe, the ip address of client machine
 * timeout_in_millisec (INPUT): Desirable timeout in milliseconds for 
 * this evaluation
 * enforcement_ob (OUTPUT): the resulted enforcement obligation  
 * from the policy decision point server. This is a string array in the 
 * order of "key-1""value-1""key-2""value-2"...
 * enforcement_result (OUTPUT): the resulted enforcement integer decision.
 * enforcement_ob (OUTPUT): Result of the policy for enforcement obligations.
 *   This is a string array in the order of 
 *   "key-1""value-1""key-2""value-2"...
 * enforcement_result (OUTPUT): Result of the policy for enforcement result
 * numEnforcements (OUTPUT): number of resulted enforcement obligations
 * ------------------------------------------------------------------------
 */ 
CEResult_t 
CSCINVOKE_CEEVALUATE_CheckMsgAttachment(CEHandle handle, 
                                        CEAction_t operation, 
                                        CEString sourceFullFileName, 
                                        nlchar **sourceAttributesStrs,
                                        CEint32 numSourceAttributes,
                                        CEint32 numRecipients,
                                        nlchar **recipientsStrs,
                                        CEint32 ipAddress,
                                        CEString userName,
                                        CEString userID,
                                        nlchar **userAttributesStrs,
                                        CEint32 numUserAttributes,
                                        CEString appName,
                                        CEString appPath,
                                        CEString appURL,
                                        nlchar **appAttributesStrs,
                                        CEint32 numAppAttributes,
                                        CEBoolean performObligation,
                                        CENoiseLevel_t noiseLevel,
                                        nlchar ***enforcement_ob,
                                        CEResponse_t *enforcement_result,
                                        CEint32 *numEnforcements,
                                        CEint32 timeout_in_millisec)
{
  try {
    CEEnforcement_t enf;
    CEAttributes sourceAttributes;
    CEAttributes userAttributes;
    CEAttributes appAttributes;
    CEString *recipients=NULL;
    CEResult_t result;

    result=CSCINVOKE_CheckInputs(handle, sourceFullFileName,NULL,
                                 sourceAttributesStrs, numSourceAttributes,
                                 NULL, 0);
    if(result != CE_RESULT_SUCCESS)
      return result;

    //Construct source attributes
    sourceAttributes.attrs = new CEAttribute[numSourceAttributes/2];
    sourceAttributes.count = numSourceAttributes/2;

    for(int i=0, j=0; i<numSourceAttributes; i++, j++) {
      CEString key = new struct _CEString();
      key->length = nlstrlen(sourceAttributesStrs[i]);
      key->buf = new nlchar[key->length+1];
      nlstrncpy_s(key->buf, key->length+1, sourceAttributesStrs[i],
                  _TRUNCATE);

      i++;

      CEString value = new struct _CEString();
      value->length = nlstrlen(sourceAttributesStrs[i]);
      value->buf = new nlchar[value->length+1];
      nlstrncpy_s(value->buf, value->length+1, sourceAttributesStrs[i],
                  _TRUNCATE);

      SetAttribute(&sourceAttributes.attrs[j], key, value);

    }

    //Construct recipients
    if(numRecipients > 0 && numRecipients <= CE_NUM_RECIPIENTS_MAX &&
       recipientsStrs != NULL) {
      recipients= new CEString[(size_t) numRecipients];
      for(int i=0; i<numRecipients; i++) 
        recipients[i]=CEM_AllocateString(recipientsStrs[i]);
    } else {
      numRecipients=0;
    }


    //Construct user
    CEUser user;
    user.userName=userName;
    user.userID=userID;
    userAttributes.count = numUserAttributes/2;
    if(userAttributes.count>0)
      userAttributes.attrs = new CEAttribute[numUserAttributes/2];
    else
      userAttributes.attrs = NULL;

    for(int i=0, j=0; i<numUserAttributes; i++, j++) {
      CEString key = new struct _CEString();
      key->length = nlstrlen(userAttributesStrs[i]);
      key->buf = new nlchar[key->length+1];
      nlstrncpy_s(key->buf, key->length+1, userAttributesStrs[i], _TRUNCATE);

      i++;

      CEString value = new struct _CEString();
      value->length = nlstrlen(userAttributesStrs[i]);
      value->buf = new nlchar[value->length+1];
      nlstrncpy_s(value->buf, value->length+1, userAttributesStrs[i],
                  _TRUNCATE);
      SetAttribute(&userAttributes.attrs[j], key, value);
    }

    //Construct application
    CEApplication app;
    app.appName=appName;
    app.appPath=appPath;
    app.appURL=appURL;
    appAttributes.count = numAppAttributes/2;
    if(appAttributes.count>0)
    {
      appAttributes.attrs = new CEAttribute[numAppAttributes/2];
        
      for(int i=0, j=0; i<numAppAttributes; i++, j++) {
        CEString key = new struct _CEString();
        key->length = nlstrlen(appAttributesStrs[i]);
        key->buf = new nlchar[key->length+1];
        nlstrncpy_s(key->buf, key->length+1, appAttributesStrs[i],
                    _TRUNCATE);
            
        i++;
            
        CEString value = new struct _CEString();
        value->length = nlstrlen(appAttributesStrs[i]);
        value->buf = new nlchar[value->length+1];
        nlstrncpy_s(value->buf, value->length+1, appAttributesStrs[i],
                    _TRUNCATE);

        SetAttribute(&appAttributes.attrs[j], key, value);
      }
    }
    else
    {
      appAttributes.attrs = NULL;
    }

    CEResult_t res=CEEVALUATE_CheckMessageAttachment(handle, 
                                                     operation,
                                                     sourceFullFileName, 
                                                     &sourceAttributes,
                                                     numRecipients,
                                                     recipients,
                                                     ipAddress,
                                                     &user,
                                                     &userAttributes,
                                                     &app,
                                                     &appAttributes,
                                                     performObligation,
                                                     noiseLevel,
                                                     &enf,
                                                     timeout_in_millisec);
    //Destruct source attributes
    for(int i=0; i<sourceAttributes.count; i++) {
      delete sourceAttributes.attrs[i].key;
      delete sourceAttributes.attrs[i].value;
    }
    delete [] sourceAttributes.attrs;
    
    //Destruct user attributes
    if(userAttributes.count >0 && userAttributes.attrs) {
      for(int i=0; i<userAttributes.count; i++) {
        delete userAttributes.attrs[i].key;
        delete userAttributes.attrs[i].value;
      }
      delete [] userAttributes.attrs;
    }


    //Destruct application attributes
    if(appAttributes.count >0 && appAttributes.attrs) {
      for(int i=0; i<appAttributes.count; i++) {
        delete appAttributes.attrs[i].key;
        delete appAttributes.attrs[i].value;
      }
      delete [] appAttributes.attrs;
    }


    //Destruct recipients
    if(numRecipients > 0) {
      for(int i=0; i<numRecipients; i++) 
        CEM_FreeString(recipients[i]);
      delete [] recipients;
    }

    //Assign enforcement result
    *enforcement_result = enf.result;
    if(enf.obligation == NULL)
      *numEnforcements=0;
    else {
      *numEnforcements = 2*enf.obligation->count;
      *enforcement_ob = new nlchar*[2*enf.obligation->count];
      size_t bufLen;
      for(int i=0, j=0; i<enf.obligation->count; i++, j++) {
        //enforcement attribute key
        if(enf.obligation->attrs[i].key){
          bufLen=enf.obligation->attrs[i].key->length;
          (*enforcement_ob)[j] = new nlchar[bufLen+1];
          if(enf.obligation->attrs[i].key->buf)
            nlstrncpy_s((*enforcement_ob)[j], bufLen+1,
                        enf.obligation->attrs[i].key->buf,
                        _TRUNCATE);
          else
            (*enforcement_ob)[j][0]='\0';
        }else {
          (*enforcement_ob)[j] = NULL;
        }

        j++;

        //enforcement attribute value
        if(enf.obligation->attrs[i].value) {
          bufLen=enf.obligation->attrs[i].value->length;
          (*enforcement_ob)[j] = new nlchar[bufLen+1];
          if(enf.obligation->attrs[i].value->buf) {
            nlstrncpy_s((*enforcement_ob)[j], bufLen+1,
                        enf.obligation->attrs[i].value->buf,
                        _TRUNCATE);
          }else
            (*enforcement_ob)[j][0]='\0';
        } else {
          (*enforcement_ob)[j] = NULL;
        }
   
      }
    }

    //Destruct enf
    if(enf.obligation)
      CEEVALUATE_FreeEnforcement(enf);

    //Return result
    return res;    
  } catch (std::exception &e) {

    return CE_RESULT_GENERAL_FAILED;
  } catch (...) {
    return CE_RESULT_GENERAL_FAILED;
  }
}
/* ------------------------------------------------------------------------
 * CSCINVOKE_CEEVALUATE_CheckResource()
 *
 * Ask the Policy Decision Point Server to evaluate the operation.
 *
 * Since .net marshaler doesn't support struct array, calling the function
 * 'CEEVALUATE_CheckMessageAttachment' from C# application needs lots of 
 * hacking
 * and manual marshaling code. This function is provided to simplify
 * the marshaling in application code.
 * 
 * This function is for the application in C#. It is called through the 
 * the platform invoke method of C#. 
 *
 * Arguments : 
 * handle (INPUT): Handle from the CONN_Initialize()
 * Operation (INPUT): Operation on the file
 * sourceURL (INPUT): url of source resource
 * targetURL (INPUT): url of target resource
 * sourceAttributes (INPUT): Associate attributes of the source. This is
 *   a string array in the order of "key-1""value-1""key-2""value-2"...
 * targetAttributes (INPUT): Associate attributes of the target. This is
 *   a string array in the order of "key-1""value-1""key-2""value-2"...
 * userName (INPUT): name of the user who access this URL.
 * userID (INPUT): id of the user who access this URL.
 * performObligation (INPUT): Perform the obligation defined by the policy 
 *                            (e.g. logging / email)
 * noiseLevel (INPUT): Desirable noise level to be used for this evaluation
 * ipAddress (INPUT): For Sharepointe, the ip address of client machine
 * timeout_in_millisec (INPUT): Desirable timeout in milliseconds for 
 * this evaluation
 * enforcement_ob (OUTPUT): the resulted enforcement obligation  
 * from the policy decision point server. This is a string array in the 
 * order of "key-1""value-1""key-2""value-2"...
 * enforcement_result (OUTPUT): the resulted enforcement integer decision.
 * enforcement_ob (OUTPUT): Result of the policy for enforcement obligations.
 *   This is a string array in the order of 
 *   "key-1""value-1""key-2""value-2"...
 * enforcement_result (OUTPUT): Result of the policy for enforcement result
 * numEnforcements (OUTPUT): number of resulted enforcement obligations
 * ------------------------------------------------------------------------
 */ 
CEResult_t 
CSCINVOKE_CEEVALUATE_CheckResources(CEHandle handle, 
                                    CEString operation,
                                    CEString sourceName,
                                    CEString sourceType,
                                    nlchar **sourceAttributesStrs,
                                    CEint32 numSourceAttributes,
                                    CEString targetName,
                                    CEString targetType,
                                    nlchar **targetAttributesStrs,
                                    CEint32 numTargetAttributes,
                                    CEString userName,
                                    CEString userID,
                                    nlchar **userAttributesStrs,
                                    CEint32 numUserAttributes,
                                    CEString appName,
                                    CEString appPath,
                                    CEString appURL,
                                    nlchar **appAttributesStrs,
                                    CEint32 numAppAttributes,
                                    CEint32 numRecipients,
                                    nlchar **recipientsStrs,
                                    CEint32 ipAddress,
                                    CEBoolean performObligation,
                                    CENoiseLevel_t noiseLevel,
                                    nlchar ***enforcement_ob,
                                    CEResponse_t *enforcement_result,
                                    CEint32 *numEnforcements,
                                    CEint32 timeout_in_millisec)
{
  try {
    CEEnforcement_t enf;
    CEAttributes sourceAttributes;
    CEAttributes targetAttributes;
    CEAttributes userAttributes;
    CEAttributes appAttributes;
    CEString *recipients=NULL;
    CEResult_t result;

    result=CSCINVOKE_CheckInputs(handle, sourceName, NULL,
                                 sourceAttributesStrs, numSourceAttributes,
                                 targetAttributesStrs, numTargetAttributes);
    if(result != CE_RESULT_SUCCESS)
      return result;

    //construct source
    CEResource *source=CEM_CreateResource(sourceName->buf, sourceType->buf);
    if(source==NULL) {
      //invalid parameters cause creating resource failed
      return CE_RESULT_INVALID_PARAMS;
    }

    //construct target
    CEResource *target=NULL;
    if(NON_EMPTY_CESTRING(targetName)) {
      target=CEM_CreateResource(targetName->buf, targetType->buf);
      if(target == NULL) {
        //invalid parameters cause creating resource failed
        CEM_FreeResource(source);
        return CE_RESULT_INVALID_PARAMS;
      }
    }

    //Construct source attributes
    sourceAttributes.attrs = new CEAttribute[numSourceAttributes/2];
    sourceAttributes.count = numSourceAttributes/2;
    for(int i=0, j=0; i<numSourceAttributes; i++, j++) {
      CEString key = new struct _CEString();
      key->length = nlstrlen(sourceAttributesStrs[i]);
      key->buf = new nlchar[key->length+1];
      nlstrncpy_s(key->buf, key->length+1, sourceAttributesStrs[i],
                  _TRUNCATE);

      i++;

      CEString value = new struct _CEString();
      value->length = nlstrlen(sourceAttributesStrs[i]);
      value->buf = new nlchar[value->length+1];
      nlstrncpy_s(value->buf, value->length+1, sourceAttributesStrs[i],
                  _TRUNCATE);
      SetAttribute(&sourceAttributes.attrs[j], key, value);
    }

    //Construct target attributes
    if(numTargetAttributes != 0) {
      targetAttributes.attrs = new CEAttribute[numTargetAttributes/2];
      targetAttributes.count = numTargetAttributes/2;
      for(int i=0, j=0; i<numTargetAttributes; i++, j++) {
        CEString key = new struct _CEString();
        key->length = nlstrlen(targetAttributesStrs[i]);
        key->buf = new nlchar[key->length+1];
        nlstrncpy_s(key->buf, key->length+1, targetAttributesStrs[i],
                    _TRUNCATE);

        i++;

        CEString value = new struct _CEString();
        value->length = nlstrlen(targetAttributesStrs[i]);
        value->buf = new nlchar[value->length+1];
        nlstrncpy_s(value->buf, value->length+1, targetAttributesStrs[i],
                    _TRUNCATE);

        SetAttribute(&targetAttributes.attrs[j], key, value);
      }
    } else {
      targetAttributes.attrs = NULL;
      targetAttributes.count = 0;
    }

    //Construct recipients
    if(numRecipients > 0 && numRecipients <= CE_NUM_RECIPIENTS_MAX
       && recipientsStrs != NULL) {
      recipients= new CEString[numRecipients];
      for(int i=0; i<numRecipients; i++) 
        recipients[i]=CEM_AllocateString(recipientsStrs[i]);
    } else {
      numRecipients=0;
    }

    //Construct user
    CEUser user;
    user.userName=userName;
    user.userID=userID;
    userAttributes.count = numUserAttributes/2;
    if(userAttributes.count>0)
      userAttributes.attrs = new CEAttribute[numUserAttributes/2];
    else
      userAttributes.attrs = NULL;
    for(int i=0, j=0; i<numUserAttributes; i++, j++) {
      CEString key = new struct _CEString();
      key->length = nlstrlen(userAttributesStrs[i]);
      key->buf = new nlchar[key->length+1];
      nlstrncpy_s(key->buf, key->length+1, userAttributesStrs[i], _TRUNCATE);

      i++;

      CEString value = new struct _CEString();
      value->length = nlstrlen(userAttributesStrs[i]);
      value->buf = new nlchar[value->length+1];
      nlstrncpy_s(value->buf, value->length+1, userAttributesStrs[i],
                  _TRUNCATE);

      SetAttribute(&userAttributes.attrs[j], key, value);
    }

    //Construct application
    CEApplication app;
    app.appName=appName;
    app.appPath=appPath;
    app.appURL=appURL;
    appAttributes.count = numAppAttributes/2;
    if(appAttributes.count>0)
    {
      appAttributes.attrs = new CEAttribute[numAppAttributes/2];
      for(int i=0, j=0; i<numAppAttributes; i++, j++) {
        CEString key = new struct _CEString();
        key->length = nlstrlen(appAttributesStrs[i]);
        key->buf = new nlchar[key->length+1];
        nlstrncpy_s(key->buf, key->length+1, appAttributesStrs[i],
                    _TRUNCATE);
            
        i++;
            
        CEString value = new struct _CEString();
        value->length = nlstrlen(appAttributesStrs[i]);
        value->buf = new nlchar[value->length+1];
        nlstrncpy_s(value->buf, value->length+1, appAttributesStrs[i],
                    _TRUNCATE);

        SetAttribute(&appAttributes.attrs[j], key, value);
      }
    }
    else
    {
      appAttributes.attrs = NULL;
    }

    CEResult_t res=CEEVALUATE_CheckResources(handle, 
                                             operation,
                                             source,
                                             &sourceAttributes,
                                             target,
                                             &targetAttributes,
                                             &user,
                                             &userAttributes,
                                             &app,
                                             &appAttributes,
                                             recipients,
                                             numRecipients,
                                             ipAddress,
                                             performObligation,
                                             noiseLevel,
                                             &enf,
                                             timeout_in_millisec);
    //Destruct source resource
    CEM_FreeResource(source);
    //Destruct source attributes
    for(int i=0; i<sourceAttributes.count; i++) {
      delete sourceAttributes.attrs[i].key;
      delete sourceAttributes.attrs[i].value;
    }
    delete [] sourceAttributes.attrs;

    if(target) {
      //destruct target resource
      CEM_FreeResource(target);
    }
    //Destruct target attributes
    if(numTargetAttributes > 0) {
      for(int i=0; i<targetAttributes.count; i++) {
        delete targetAttributes.attrs[i].key;
        delete targetAttributes.attrs[i].value;
      }
      delete [] targetAttributes.attrs;
    }

    //Destruct user attributes
    if(userAttributes.count >0 && userAttributes.attrs) {
      for(int i=0; i<userAttributes.count; i++) {
        delete userAttributes.attrs[i].key;
        delete userAttributes.attrs[i].value;
      }
      delete [] userAttributes.attrs;
    }

    //Destruct application attributes
    if(appAttributes.count >0 && appAttributes.attrs) {
      for(int i=0; i<appAttributes.count; i++) {
        delete appAttributes.attrs[i].key;
        delete appAttributes.attrs[i].value;
      }
      delete [] appAttributes.attrs;
    }

    //Destruct recipients
    if(numRecipients > 0) {
      for(int i=0; i<numRecipients; i++) 
        CEM_FreeString(recipients[i]);
      delete [] recipients;
    }

    //Assign enforcement result
    *enforcement_result = enf.result;
    if(enf.obligation == NULL)
      *numEnforcements=0;
    else {
      *numEnforcements = 2*enf.obligation->count;
      *enforcement_ob = new nlchar*[2*enf.obligation->count];
      size_t bufLen;
      for(int i=0, j=0; i<enf.obligation->count; i++, j++) {
        //enforcement attribute key
        if(enf.obligation->attrs[i].key){
          bufLen=enf.obligation->attrs[i].key->length;
          (*enforcement_ob)[j] = new nlchar[bufLen+1];
          if(enf.obligation->attrs[i].key->buf)
            nlstrncpy_s((*enforcement_ob)[j], bufLen+1,
                        enf.obligation->attrs[i].key->buf,
                        _TRUNCATE);
          else
            (*enforcement_ob)[j][0]='\0';
        }else {
          (*enforcement_ob)[j] = NULL;
        }

        j++;

        //enforcement attribute value
        if(enf.obligation->attrs[i].value) {
          bufLen=enf.obligation->attrs[i].value->length;
          (*enforcement_ob)[j] = new nlchar[bufLen+1];
          if(enf.obligation->attrs[i].value->buf) {
            nlstrncpy_s((*enforcement_ob)[j], bufLen+1,
                        enf.obligation->attrs[i].value->buf,
                        _TRUNCATE);
          }else
            (*enforcement_ob)[j][0]='\0';
        } else {
          (*enforcement_ob)[j] = NULL;
        }  
      }
    }

    //Destruct enf
    if(enf.obligation)
      CEEVALUATE_FreeEnforcement(enf);

    //Return result
    return res;    
  } catch (std::exception &e) {
    return CE_RESULT_GENERAL_FAILED;
  } catch (...) {
    return CE_RESULT_GENERAL_FAILED;
  }
}/*CSCINVOKE_CEEVALUATE_CheckResources*/

/*
 * CSCINVOKE_CEEVALUATE_CheckResourcesEx
 *
 * Ask the PDP server to evaluate a number of operations simultaneously
 */
CEResult_t
CSCINVOKE_CEEVALUATE_CheckResourcesEx(CEHandle handle,
                                      CERequest* requests,
                                      CEint32 numRequests,
                                      CEString additionalPQL,
                                      CEBoolean ignoreBuiltinPolicies,
                                      CEint32 ipAddress,
                                      int *results,
                                      int *obligationCounts,
                                      nlchar ***obligations,
                                      CEint32 timeout)
{
  try {
    std::vector<CEEnforcement_t> enforcementResults;

    enforcementResults.resize(numRequests);

    CEResult_t res = CEEVALUATE_CheckResourcesEx(handle,
                                                 requests,
                                                 numRequests,
                                                 additionalPQL,
                                                 ignoreBuiltinPolicies,
                                                 ipAddress,
                                                 &enforcementResults[0],
                                                 timeout);

    if (res == CE_RESULT_SUCCESS)
    {
      for (int i = 0; i < numRequests; i++)
      {
        const CEAttributes* currObligation = enforcementResults[i].obligation;

        results[i] = enforcementResults[i].result;

        if (currObligation == NULL)
        {
          obligationCounts[i] = 0;
          obligations[i] = NULL;
        }
        else
        {
          obligationCounts[i] = currObligation->count * 2;

          obligations[i] = new nlchar*[currObligation->count*2];  // space for both keys and values

          for (int j = 0; j < currObligation->count; j++)
          {
            int keysize = currObligation->attrs[j].key->length+1;
            obligations[i][j*2] = new nlchar[keysize];
            nlstrcpy_s(obligations[i][j*2], keysize, currObligation->attrs[j].key->buf);

            int valsize = currObligation->attrs[j].value->length+1;
            obligations[i][j*2+1] = new nlchar[valsize];
            nlstrcpy_s(obligations[i][j*2+1], valsize, currObligation->attrs[j].value->buf);
          }

          CEEVALUATE_FreeEnforcement(enforcementResults[i]);
        }
      }
    }

    return res;
  } catch (std::exception &e) {
    return CE_RESULT_GENERAL_FAILED;
  } catch (...) {
    return CE_RESULT_GENERAL_FAILED;
  }

}


/* ------------------------------------------------------------------------
 * CSCINVOKE_CEEVALUATE_GetString
 *
 * This function is to simplify the marshaling for C# application
 * This function will return the pointer "strPtr" to the No."index" string in
 * the string array pointed by "ptr". Meanwhile, it returns the length of
 * string. 
 * 
 * Arguments : ptr(INPUT): pointer to the array of strings of enforcement
 *                  obligation
 *             index(INPUT): the index of the string that is acquired.
 *             nlchar(OUTPUT): the poter to the string that is acquired.
 * Return: the length of the string that is acquired.
 * ------------------------------------------------------------------------
 */ 
int CSCINVOKE_CEEVALUATE_GetString(nlchar **ptr,
                                   int index,
                                   nlchar **strPtr)
{
  try{
    if(ptr[index]) {
      *strPtr=ptr[index];
      return nlstrlen(ptr[index]);
    }
    *strPtr=NULL;
  } catch (std::exception &e) {
    return -1;
  } catch (...) {    
    return -1;
  }

  return 0;
}

/* ------------------------------------------------------------------------
 * CSCINVOKE_CEEVALUATE_FreeStringArray
 *
 * Basic service to free enforcement obligation array of string from the system
 * This function is to simplify the marshaling for C# application
 * 
 * Arguments : ptr(INPUT): pointer to the array of strings to be freed.
 *             numStrings: the number of strings in the array
 * ------------------------------------------------------------------------
 */ 
CEResult_t CSCINVOKE_CEEVALUATE_FreeStringArray(nlchar **ptr,
                                                int numStrings)
{
  for(int i=0; i<numStrings; i++) {
    delete [] ptr[i];
  }
  delete [] ptr;
  return CE_RESULT_SUCCESS;
}

