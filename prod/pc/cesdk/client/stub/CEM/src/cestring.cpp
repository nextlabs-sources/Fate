#include <string>
#include "cetype.h"
#include "CEsdk.h"

CEResult_t CEM_FreeString(CEString cestr)
{
  if(cestr==NULL)
    return CE_RESULT_SUCCESS;

  delete cestr;

  return CE_RESULT_SUCCESS;
}

#ifdef WIN32
CEString CEM_AllocateString(const TCHAR * str)
#else
CEString CEM_AllocateString(const char * str)
#endif
{
  if(str == NULL)
    return NULL;

  try {
  CEString ces=new struct _CEString();

  ces->length=nlstrlen(str);
  ces->buf = new nlchar[ces->length+1];
  nlstrncpy_s(ces->buf, ces->length+1, str, _TRUNCATE);

  return ces;
  } catch(...) {
	return (NULL);
  }
}

#ifdef WIN32
const TCHAR *CEM_GetString(CEString cestr)
#else
const char *CEM_GetString(CEString cestr)
#endif
{
  if(cestr==NULL)
    return NULL;

  if(cestr->buf == NULL)
    return NULL;

#ifdef WIN32
  return (const TCHAR *)cestr->buf;
#else
  return (const char *)cestr->buf;
#endif
}

#ifdef WIN32
CEResult_t CEM_ReallocateString(CEString cestr, const TCHAR * newstr)
#else
CEResult_t CEM_ReallocateString(CEString cestr, const char* newstr)
#endif
{
  if(cestr==NULL)
    return CE_RESULT_INVALID_PARAMS;

  if(cestr->buf != NULL)
    delete [] cestr->buf;

  cestr->length=nlstrlen(newstr);
  cestr->buf = new nlchar[cestr->length+1];
  nlstrncpy_s(cestr->buf, cestr->length+1, newstr, _TRUNCATE);

  return CE_RESULT_SUCCESS;
}

void CEM_FreeResource(CEResource *resource)
{
  if(resource == NULL)
    return;

  CEM_FreeString(resource->resourceName);
  CEM_FreeString(resource->resourceType);

  delete resource;
  
  return;
}

#if defined (Linux) || defined (Darwin)
CEResource *
CEM_CreateResource (const char * resourceName, const char* resourceType)
{
  if(resourceName == NULL)
    return NULL; //resource name is not optional
  
  if(strlen(resourceName) <= 0)
    return NULL; //resource name is not optional

  if(resourceType == NULL)
    return NULL; //resource type is not optional
  
  if(strlen(resourceType) <= 0)
    return NULL; //resource type is not optional

  try {
    CEResource *newResource=new CEResource;
    CEString resName=CEM_AllocateString(resourceName);
    CEString resType=NULL;
    
    resType=CEM_AllocateString(resourceType);

    newResource->resourceName=resName;
    newResource->resourceType=resType;
  
    return newResource;
  } catch (...) {
    return NULL;
  }
}
#endif

#if defined (WIN32) || defined (_WIN64)
CEResource *
CEM_CreateResourceW (const wchar_t * resourceName, const wchar_t * resourceType)
{
  if(resourceName == NULL)
    return NULL; //resource name is not optional

  if(wcslen(resourceName) <= 0)
    return NULL; //resource name is not optional

  if(resourceType == NULL)
    return NULL; //resource type is not optional

  if(wcslen(resourceType) <= 0)
    return NULL; //resource type is not optional

  try {
    CEResource *newResource=new CEResource;
#ifdef UNICODE
    CEString resName;
    resName=CEM_AllocateString(resourceName); //nlchar is wchar_t under UNICODE
    CEString resType=NULL;
    resType=CEM_AllocateString(resourceType); //nlchar is wchar_t under UNICODE

    newResource->resourceName=resName;
    newResource->resourceType=resType;
#else
    char tmpStr[1024];
    int numC;

    //compose resource source
    numC=WideCharToMultiByte(CP_ACP, 0, resourceName, -1,
				 tmpStr, _countof(tmpStr));
    CEString resName;
    resName=CEM_AllocateString(tmpStr); //nlchar is char under NON-UNICODE
    newResource->resourceName=resName;

    //compose resource type
    numC=WideCharToMultiByte(CA_ACP, 0, resourceType, -1,
			     tmpStr, _countof(tmpStr));
    CEString resType;
    resType=CEM_AllocateString(tmpStr); //nlchar is char under NON-UNICODE
    newResource->resourceType=resType;
#endif
 
    return newResource;
  } catch (...) {
    return NULL;
  }  
}

CEResource *
CEM_CreateResourceA (const char * resourceName, const char* resourceType)
{
  if(resourceName == NULL)
    return NULL; //resource name is not optional

  if(strlen(resourceName) <= 0) 
    return NULL; //resource name is not optional

  if(resourceType == NULL)
    return NULL; //resource type is not optional

  if(strlen(resourceType) <= 0) 
    return NULL; //resource type is not optional

  try {
    CEResource *newResource=new CEResource;

#ifdef UNICODE
    wchar_t tmpStr[1024];
    int numWC;

    //compose resource name
    numWC=MultiByteToWideChar(CP_ACP, 0, resourceName, (int)strlen(resourceName),
			      tmpStr, _countof(tmpStr)); // cast of strlen(resourceType) to int is necessary because on x64, size_t is 64 bit and int is 32 bit.  I assume this value does not exceed 32 bit max integer value
    tmpStr[numWC]=L'\0';
    CEString resName;
    resName=CEM_AllocateString(tmpStr); //nlchar is wchar_t under UNICODE
    newResource->resourceName=resName;

    //compose resource type
    numWC=MultiByteToWideChar(CP_ACP, 0, resourceType, (int)strlen(resourceType),
			      tmpStr, _countof(tmpStr)); // cast of strlen(resourceType) to int is necessary because on x64, size_t is 64 bit and int is 32 bit.  I assume this value does not exceed 32 bit max integer value
    tmpStr[numWC]=L'\0';
    CEString resType;
    resType=CEM_AllocateString(tmpStr); //nlchar is wchar_t under UNICODE
    newResource->resourceType=resType;
#else
    //compose resource name
    CEString resName;
    resName=CEM_AllocateString(resourceName);//nlchar is char under NON-UNICODE
    //compose resource type
    CEString resType=NULL;
    resType=CEM_AllocateString(resourceType);//nlchar is char under NON-UNICODE

    newResource->resourceName=resName;
    newResource->resourceType=resType;
#endif  
    return newResource;
  } catch (...) {
    return NULL;
  }  
}
#endif 
