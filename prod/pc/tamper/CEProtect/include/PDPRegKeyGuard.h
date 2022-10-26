/*========================PDPRegKeyGuard.h==================================*
 *                                                                          *
 * All sources, binaries and HTML pages (C) copyright 2006 by Blue Jungle., *
 * Redwood City CA, Ownership remains with Blue Jungle Inc,                 * 
 * All rights reserved worldwide.                                           *
 *                                                                          * 
 * Note   : This file is private for registration key proection.            *
 *==========================================================================*/

#ifndef __CE_PDPREGKEYGUARD_H
#define __CE_PDPREGKEYGUARD_H


#include <string>
#include <stdexcept>
#include <map>
#include "nlstrings.h"
#include "brain.h"
#include "nltypes.h"
#include "nlthread.h"
#include "CEsdk.h"

namespace PDPREGKEYGUARD {
using namespace std;
struct KeyToFileMapping 
{
  HKEY hRootKey;
  TCHAR keyName [MAX_PATH];
  HKEY hKey;
  TCHAR fileName [MAX_PATH];
};
typedef struct KeyToFileMapping KeyToFileMapping;

enum OPERATION {ADD, REMOVE};

struct Request {
  enum STATUS {WAITING, RETURNED};

  HKEY rootKey;
  LPCTSTR  keyName;
  OPERATION op;
  STATUS status;
  CEResult_t result;

  nlthread_cond_t cond; //the conidtion variable when the answer arrives
  nlthread_mutex_t mutex; //The mutext to protect answer buffer;

  //Constructor
  Request(HKEY r, LPCTSTR k, OPERATION op);

  //Destructor
  ~Request() { 
    nlthread_cond_destroy(&cond); 
    nlthread_mutex_destroy(&mutex);}
};
typedef struct Request Request;

/**
* This class protects keys in the registry from being changed. It restores
* protected keys to their original values if they are changed.
*
*/
class RegKeyGuard
{
public:
  enum {MAX_KEY_SIZE=100};

  RegKeyGuard(void);
  virtual ~RegKeyGuard(void);
	
  //Sets the name of the hardware profile key to protect.
  //Handling of hardware profile keys is special
  BOOL SetHardwareProfileKey (LPCTSTR pKeyName);

  void Run ();

  void Stop ();

  void removeKeys();

  void Init(bool bDesktop);

  //Adds a key to be protected
  CEResult_t AddKey (HKEY hRootKey, LPCTSTR pKeyName);

  //Add a key operation(protecting/unprotecting) request
  CEResult_t AddARequest(OPERATION op, HKEY hRootKey, LPCTSTR pkeyName);
private:
  //Handle adding/removing registration keys request
  void HandleRequest();

  //Restore the hardware profile settings
  BOOL RestoreHardwareProfileKey (HKEY hKey, LPCTSTR pHardwareProfileKey);

	//	add parent keys that need protected, this is different from the keys that need be protected from configuration files.
	void AddPrarentMonitorKeys();

	//	determine if it is a parent key by root key and sub key name,
	//	like, IsParentKey(HKEY_LOCAL_MACHINE, L"Software\\NextLabs")
	BOOL IsParentKey(HKEY hRootKey, LPCTSTR pKeyName);

	//	determine if it is a parent key by full key name,
	BOOL IsParentKey(const wstring& KeyName);

	//	determine if it is a key to be protected that has any parent key
	BOOL HasParentKey(const wstring& KeyName);

	//	determine if it is a subkey to be protected of the specified parent key
	BOOL IsSubKey(const wstring& KeyName, const wstring& parentKey);

	//	restore key content
	void RestoreKey(std::map<wstring, KeyToFileMapping *>::iterator mit);

  //The queue of adding/removing protected key
  std::vector<Request *> reqQ;
  //The mutext to protect the request queue
  nlthread_mutex_t reqQMutex; 

  //The list of protect registration key. 
  //The key of the map is in the format "RootKey+KeyName"
  std::map<nlstring, KeyToFileMapping *> m_keys;

  //The mapping between an event (index) and a registration key
  std::map<int, nlstring> m_eventKey; 

  //The hardware profile key
  WCHAR m_hardwareProfileKey[MAX_KEY_SIZE];
  //The hardware profile key in the map m_keys 
  nlstring hardwareProfileKeyStr; 

  //pdp stop event
  HANDLE m_stopEvent;
  //request of adding/remove a key from protection list
  HANDLE m_reqEvent;

  //The array of events. 
  //The first one is for pdp stop event, a.k.a. m_stopEvent; 
  //the second is for the request of adding/removing a key from 
  //protection list, a.k.a. m_reqEvent; 
  HANDLE *m_eventArray;
  //The current end index of event array
  unsigned int m_eventArrayTail;
  //The length of the event array
  unsigned int m_eventArrayLen;
};
}
#endif /* PDPRegKeyGuard.h */
