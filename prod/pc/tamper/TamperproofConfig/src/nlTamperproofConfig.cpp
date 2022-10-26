/*=======================nlTamperproofConfig.cpp============================*
 *                                                                          *
 * All sources, binaries and HTML pages (C) copyright 2006 by NextLabs,     *
 * San Mateo CA, Ownership remains with NextLabs Inc,                       * 
 * All rights reserved worldwide.                                           *
 *                                                                          * 
 * Author : Heidi Zhou                                                      *
 * Date   : 8/28/2008                                                       *
 * Note   : API to read tamperproof configuration file                      *
 *          On Unix, these APIs do nothing currently.                      *
 *==========================================================================*/
#include <Windows.h>
#include <list>
#include "celog.h"
#include "nlTamperproofConfig.h"
#include <vector>
#include <strsafe.h>

namespace {
const char * nextlabsMacro="[NextLabs]";
WCHAR nlDir[MAX_PATH];     // NextLabs' directory
const char * windowsSystemMacro="[System]";
WCHAR systemDir[MAX_PATH]; // windows\system32' directory

bool ProcessMacro(char *line, const char *macroName, const WCHAR *macroValue, NLTamperproofEntry &e, NLTamperproofMap &m)
{
  WCHAR twStr[MAX_PATH];
  char *macrop=strstr(line, macroName);

  if(macrop != NULL) {
    if(macrop == line) { //format: file=<access>,[Macro]???
        //advance to after "[Macro]"
        line+=strlen(macroName);
        size_t endIndex=strlen(line);
        if(endIndex <= 0 || endIndex == 1) {
            //format: "file=<access>,[Macro]" or "file=<access>,[Macro]\"
            wcsncpy_s(twStr, MAX_PATH, macroValue, _TRUNCATE);
            endIndex=wcslen(twStr);
            twStr[endIndex-1]=L'\0';

            // add to map
            m[twStr]=e;
        } else {
            //format: file=<access>,[Macro]\xxx
            line+=1; // skip '\'
            MultiByteToWideChar( CP_ACP, 0, line, (int)strlen(line)+1, twStr, sizeof(twStr)/sizeof(twStr[0]));
            nlstring f(macroValue);  
            f+=twStr;

            // add to map
            m[f]=e;
        }    
    }

    return true;
  }

  return false;
}

inline void ProcessProtectedFile(char *in, NLTamperproofMap &m)
{
  char *p=(char *)strstr(in, "=");

  if(p==NULL)
    return; //invalid format: no "="

  if(strlen(p) <=1)
    return; //invalid format: empty after '='

  //move to what is after "=";
  p++;

  NLTamperproofEntry e;
  e.type=NL_TAMPERPROOF_TYPE_FILE;

  char *ap=strstr(p,"ro,"); //access position
  if(ap) {
    e.access=NL_TAMPERPROOF_ACCESS_RO;
  } else {
    ap=strstr(p,"na,");
    if(ap) {
      e.access=NL_TAMPERPROOF_ACCESS_NONE;
    } else {
      return; //malformed
    }
  }

  //advance to file name
  p=ap+3;

  if(strlen(p)<=0)
    return;  //invalid format: no file name

#if defined (WIN32) || defined (_WIN64)
  //try to replace maro [NextLabs]
  WCHAR twStr[MAX_PATH];

  if (ProcessMacro(p, nextlabsMacro, nlDir, e, m)) {
      return;
  }

  if (ProcessMacro(p, windowsSystemMacro, systemDir, e, m)) {
      return;
  }

  // Ordinary line
  MultiByteToWideChar( CP_ACP, 0, p, (int)strlen(p)+1, twStr, (int)sizeof(twStr)/sizeof(twStr[0]));
  m[twStr]=e;
#endif //#if defined (WIN32) || defined (_WIN64)
}

inline void ProcessProtectedProc(char *in, NLTamperproofMap &m)
{
  char *p=(char *)strstr(in, "=");

  if(p==NULL)
    return; //invalid format: no "="

  if(strlen(p) <=1)
    return; //invalid format: empty after '='

  //move to what is after "=";
  p++;

  //add into map
#if defined (WIN32) || defined (_WIN64)
  WCHAR twStr[MAX_PATH];
  MultiByteToWideChar( CP_ACP, 0, p, (int)strlen(p)+1, twStr,
		       sizeof(twStr)/sizeof(twStr[0]));
  NLTamperproofEntry e;
  e.type=NL_TAMPERPROOF_TYPE_PROCESS;
  e.access=NL_TAMPERPROOF_ACCESS_NOKILL;
  m[twStr]=e;
#endif //#if defined (WIN32) || defined (_WIN64)  
}

//Get the root registry key and return sub-key
  inline char *GetRegRootKey(char *in, WCHAR *rootKey, int rootKeyBufLen)
{
  char *subKey=NULL;
#if defined (WIN32) || defined (_WIN64)
  char *rp;
  char *rKeys[10]={"HKEY_CLASSES_ROOT\\",
		 "HKCR\\",
		 "HKEY_CURRENT_USER\\",
		 "HKCU\\",
		 "HKEY_LOCAL_MACHINE\\",
		 "HKLM\\",
		 "HKEY_USERS\\",
		 "HKEY_CURRENT_CONFIG\\",
		 "KEY_PERFORMANCE_DATA\\",
		 "HKEY_DYN_DATA\\"};
  WCHAR  *fullRootKeys[10]={L"HKEY_CLASSES_ROOT",
		 L"HKEY_CLASSES_ROOT",
		 L"HKEY_CURRENT_USER",
		 L"HKEY_CURRENT_USER",
		 L"HKEY_LOCAL_MACHINE",
		 L"HKEY_LOCAL_MACHINE",
		 L"HKEY_USERS",
		 L"HKEY_CURRENT_CONFIG",
		 L"KEY_PERFORMANCE_DATA",
		 L"HKEY_DYN_DATA"};

  //Check if the following string is started with one of 
  // HKEY_CLASSES_ROOT (HKCR)
  // HKEY_CURRENT_USER (HKCU)
  // HKEY_LOCAL_MACHINE (HKLM)
  // HKEY_USERS (HKU)
  // HKEY_CURRENT_CONFIG
  // KEY_PERFORMANCE_DATA
  // HKEY_DYN_DATA
  for(int i=0; i<10; i++) {
    rp=strstr(in, rKeys[i]);
    if(rp) {
      if(rp!=in) {
	//If not started with root key, it is malformed
	return NULL;
      }
      subKey=rp+strlen(rKeys[i]);
      _snwprintf_s(rootKey,
		 rootKeyBufLen, _TRUNCATE,		
		 L"%s",
		 fullRootKeys[i]);
      return subKey;
    }
  }
#endif //#if defined (WIN32) || defined (_WIN64)
  return subKey;
}

inline void ProcessProtectedRegKey(char *in, NLTamperproofMap &m)
{
#if defined (WIN32) || defined (_WIN64)
  char *p=(char *)strstr(in, "=");

  if(p==NULL)
    return; //invalid format: no "="

  if(strlen(p) <=1)
    return; //invalid format: empty after '='

  //move to what is after "=";
  p++;

  WCHAR twStr[MAX_PATH];
  WCHAR twStr1[MAX_PATH];

  //Get the root key and subkey
  p=GetRegRootKey(p, twStr, MAX_PATH);  
  if(p==NULL)
    return; //Can't find root key, malformed

  //add into map
  NLTamperproofEntry e;
  e.type=NL_TAMPERPROOF_TYPE_REGKEY;
  e.access=NL_TAMPERPROOF_ACCESS_RO;
  e.root=twStr;
  MultiByteToWideChar( CP_ACP, 0, p, (int)strlen(p)+1, twStr,
		       sizeof(twStr)/sizeof(twStr[0]));
  _snwprintf_s(twStr1,
	       sizeof(twStr1)/sizeof(twStr[0]), _TRUNCATE,
	       L"%s\\%s",
	       e.root.c_str(),twStr);
  
  m[twStr1]=e;
#endif //#if defined (WIN32) || defined (_WIN64)
}

inline void ProcessExemptProc(char *in, std::set<nlstring> &proc)
{
  char *p=(char *)strstr(in, "=");

  if(p==NULL)
    return; //invalid format: no "="

  if(strlen(p) <=1)
    return; //invalid format: empty after '='

  //move to what is after "=";
  p++;

  //add into exempt process set
#if defined (WIN32) || defined (_WIN64)
  WCHAR twStr[MAX_PATH];
  MultiByteToWideChar( CP_ACP, 0, p, (int)strlen(p)+1, twStr,
		       sizeof(twStr)/sizeof(twStr[0]));
  proc.insert(twStr);
#endif //#if defined (WIN32) || defined (_WIN64)
}

void GetAllFilesUnderConfigDir(nlchar *dirName, 
			       nlchar *searchableDir,
 			std::vector<nlstring> &location)
{
  if(dirName==NULL)
    return;

#if defined (WIN32) || defined (_WIN64)
  WIN32_FIND_DATA ffd;
  HANDLE hFind = INVALID_HANDLE_VALUE;
  DWORD dwError=0;

  // Find the first file in the directory.

  hFind = FindFirstFile(searchableDir, &ffd);
  if (INVALID_HANDLE_VALUE == hFind) {

    //TRACE(CELOG_DEBUG, _T("FindFirstFile failed: %d\n"), GetLastError());
    return;
  } 
   
  nlstring fullPath;
   // retrieve the directory 
   do {
     if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
       //only care about file not directory
       fullPath=dirName;
       fullPath+=ffd.cFileName;
       location.push_back(fullPath);
     }
   } while (FindNextFile(hFind, &ffd) != 0);
 
   dwError = GetLastError();
   if (dwError != ERROR_NO_MORE_FILES) 
   {
     //TRACE(CELOG_DEBUG, _T("FindFirstFile failed: %d\n"), dwError);
   }

   FindClose(hFind);
   return;

#endif //#if defined (WIN32) || defined (_WIN64)
  return;
}

void GetConfigLocation(std::vector<nlstring> &location)
{
#if defined (WIN32) || defined (_WIN64)

  LONG status;
  HKEY hKey = NULL; 

  //TRACE(CELOG_DEBUG, _T("GetTPConfigLocation\n"));

  status = RegOpenKeyExA(HKEY_LOCAL_MACHINE,
			  "SOFTWARE\\NextLabs\\Compliant Enterprise\\Policy Controller",
			  0,
			  KEY_QUERY_VALUE,
			  &hKey);

  if( status != ERROR_SUCCESS ) {
    //TRACE(CELOG_ERR, _T("GetTPConfigLocation: reg key open failed %d.\n"), 
    //status);
    return;
  }

  WCHAR pcRoot[MAX_PATH];                 /* InstallDir */
  DWORD pc_root_size = sizeof(pcRoot);

  status = RegQueryValueExW(hKey,               /* handle to reg */      
			    L"InstallDir",      /* key to read */
			     NULL,               /* reserved */
			     NULL,               /* type */
			     (LPBYTE)pcRoot,    /* InstallDir */
			     &pc_root_size       /* size (in/out) */
			    );

  RegCloseKey(hKey);

  if( status != ERROR_SUCCESS ) {
    //TRACE(CELOG_DEBUG, 
    //_T("GetTPConfigLocation: No configuration path.\n"));
    return;
  }

  //get NextLabs directory
  _snwprintf_s(nlDir,
	       sizeof(nlDir)/sizeof(nlDir[0]), _TRUNCATE,
	       L"%s",
	       pcRoot);
  
  // get windows system directory
  if (GetSystemDirectoryW(systemDir, _countof(systemDir)) == 0) {
      // Uh oh.  Pick a likely default
      //TRACE(CELOG_ERR, _T("GetSystemDirectory returned error %d\n"), ::GetLastError());
      wcsncpy_s(systemDir, MAX_PATH, L"c:\\windows\\system32\\", _TRUNCATE);
  } else {
      wcsncat_s(systemDir, MAX_PATH, L"\\", _TRUNCATE);
  }

  WCHAR cDir[MAX_PATH];
  WCHAR sDir[MAX_PATH];
  _snwprintf_s(cDir,
	       sizeof(cDir)/sizeof(cDir[0]), _TRUNCATE,
	       L"%sPolicy Controller\\config\\tamper_resistance\\",
	       pcRoot);
  _snwprintf_s(sDir,
	       sizeof(sDir)/sizeof(sDir[0]), _TRUNCATE,
	       L"%sPolicy Controller\\config\\tamper_resistance\\*",
	       pcRoot);
  //TRACE(CELOG_DEBUG, _T("GetTPConfigLocation: %s\n"),cDir);

  /*Get all the files' names under tamper_resistance directory*/
  GetAllFilesUnderConfigDir(cDir, sDir, location);

  return;
#endif //#if defined (WIN32) || defined (_WIN64)

  return;
} //GetConfigurationLocation

void ReadATamperproofConfig(NLTamperproofType type,
			    const WCHAR *cFile, 
			    NLTamperproofMap &out)
{
  FILE* fp;
  errno_t eno = _wfopen_s(&fp, cFile, L"r");

  if( eno != 0 ) {
    /* There are no hooks to install */
    //TRACE(CELOG_DEBUG, 
    //_T("ReadATamperproofConfig: No configuration file %s.\n"),
    //  cFile);
    return;
  }

  //Line length is a possible full path (MAX_PATH) with library symbols (512)
  char temp[MAX_PATH + 512];
  std::set<nlstring> exemptProcs;
  NLTamperproofMap tmpMap;

  /* Read file line by line and parse configuration.

     The format is, 
     < object type>=[action,]<object>

     The object type has to be "FileExemptProc", "file", "process" or 
     "registry". Other than that will be ingored.          
  */
  while( fgets(temp,sizeof(temp),fp) != NULL )
  {
    /* ignore line - empty or comment */
    if( strlen(temp) <= 1) {
      //TRACE(CELOG_DEBUG, 
      //    _T("ReadATamperproofConfig: ignore empty line\n"));
      continue; 
    }

    char* delim = "\n"; /* delimiter - '\n' is used to avoid manual trim */
    char* next_token = temp;
    char *p;
    char *p1=NULL;
    p1=strtok_s(next_token, delim, &next_token);
    if(p1==NULL)
      p1=temp;
    p=(char *)strstr(p1, "FileExemptProc");
    if(p && p==p1) {
      //Ignore if not "file" or "all" information needed
      if(type == NL_TAMPERPROOF_TYPE_FILE ||
	 type == NL_TAMPERPROOF_TYPE_ALL)
	ProcessExemptProc(p, exemptProcs);
      continue;
    } 
    p=(char *)strstr(p1, "file");
    if(p && p==p1) {
      //Ignore if not "file" or "all" information needed
      if(type == NL_TAMPERPROOF_TYPE_FILE ||
	 type == NL_TAMPERPROOF_TYPE_ALL)
	ProcessProtectedFile(p, tmpMap);
      continue;
    } 
    p=(char *)strstr(p1, "process");
    if(p && p==p1) {
      //Ignore if not "process" or "all" information needed
      if(type == NL_TAMPERPROOF_TYPE_PROCESS ||
	 type == NL_TAMPERPROOF_TYPE_ALL)
	ProcessProtectedProc(p, tmpMap);
      continue;
    } 
    p=(char *)strstr(p1, "registry");
    if(p && p==p1) {
      //Ignore if not "regkey" or "all" information needed
      if(type == NL_TAMPERPROOF_TYPE_REGKEY ||
	 type == NL_TAMPERPROOF_TYPE_ALL)
	ProcessProtectedRegKey(p, tmpMap);
      continue;
    } 
    
    //TRACE(CELOG_DEBUG, _T("ReadATamperproofConfig: ignore one line\n"));
  }/* while fgets */
  fclose(fp);

  //TRACE(CELOG_DEBUG, 
  //_T("ReadATamperproofConfig: in file %s, %d entries read\n"), 
  //cFile, tmpMap.size());

  NLTamperproofMap::iterator mit;
  NLTamperproofMap::iterator mit_e=tmpMap.end();

  //Add file exempt process if "file" or "all" information needed
  if(type == NL_TAMPERPROOF_TYPE_FILE ||
     type == NL_TAMPERPROOF_TYPE_ALL){
    std::set<nlstring>::iterator pit=exemptProcs.begin();
    std::set<nlstring>::iterator pit_e=exemptProcs.end();
    
    for(; pit != pit_e; pit++) {
      for(mit=tmpMap.begin(); mit != mit_e; mit++) {
	if(mit->second.type == NL_TAMPERPROOF_TYPE_FILE)
	  mit->second.exemptProc.insert(*pit);
      }
    }
  }

  //Merge tmpMap into the final output map
  for(mit=tmpMap.begin(); mit != mit_e; mit++) {
    out[mit->first]=mit->second;
  }
  return;  
}
}

/* ======================NLTamperproofConfig_Load=========================*
 * Load tamperproof configuration information                             *
 *                                                                        *
 * Parameters:                                                            *
 * type (input): the type that are relevant.                              *
 * outBuffer (output): the pointer to a NLTamperproofMap buffer that will *
 *                     store the tamperproof configuration information.   *
 *                                                                        *
 * Return:                                                                *
 *   It will return "true" if the function succeeds; otherwise, it will   *
 *   return "false"                                                       *
 * =======================================================================*/
bool NLTamperproofConfiguration_Load(NLTamperproofType type, 
				     NLTamperproofMap **outBuffer)
{ 
  std::vector<nlstring> location;

  //Check input
  if(outBuffer == NULL) {
    //TRACE(CELOG_ERR, _T("Invalid parameter: outBuffer is NULL\n"));
    return false;
  } else {
    *outBuffer=NULL;
  }
  
  //Get paths to tamperproof configuration files
  GetConfigLocation(location);

  size_t llen=location.size();
  if(llen <= 0) {
    //TRACE(CELOG_DEBUG, _T("No any tamperproof configuration files\n"));
    return true;
  }

  *outBuffer=new NLTamperproofMap;
  if(*outBuffer==NULL) {
    //TRACE(CELOG_ERR, _T("Failed to allocate NLTamperproofMap\n")); 
    return false;
  }

  NLTamperproofMap &tpMap=**outBuffer; 
  for(size_t i=0; i<llen; i++) {
    ReadATamperproofConfig(type, location[i].c_str(), tpMap);
  }

  if(tpMap.size()==0) {
    //TRACE(CELOG_DEBUG, 
    //_T("No any tamperproof configuration information (type=%d)\n"),
    //	  type);
    return true;    
  }
  return true;
}
  
/* ======================NLTamperproofConfig_Free=========================*
 * Free the bufffer returned from NLTamperproofConfig_Load                *
 *                                                                        *
 * Parameters:                                                            *
 * type (input): the type that are relevant.                              *
 * buffer (input): the pointer to a NLTamperproofMap buffer that be freed *
 *                                                                        *
 * Return:                                                                *
 *   It will return "true" if the function succeeds; otherwise, it will   *
 *   return "false"                                                       *
 * =======================================================================*/
bool NLTamperproofConfiguration_Free(NLTamperproofMap *buffer)
{
  if(buffer == NULL)
    return true;

  delete buffer;

  return true;
}

