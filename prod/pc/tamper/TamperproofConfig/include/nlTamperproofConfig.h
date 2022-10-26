/*=======================nlTamperproofConfig.h==============================*
 *                                                                          *
 * All sources, binaries and HTML pages (C) copyright 2006 by NextLabs,     *
 * San Mateo CA, Ownership remains with NextLabs Inc,                       * 
 * All rights reserved worldwide.                                           *
 *                                                                          * 
 * Author : Heidi Zhou                                                      *
 * Date   : 8/28/2008                                                       *
 * Note   : declarations of API to read tamperproof configuration file      *
 *==========================================================================*/
#ifndef __NL_TAMPERPROOF_CONFIG_H
#define __NL_TAMPERPROOF_CONFIG_H

#include "nlstrings.h"
#include <string>
#include <set>
#include <map>

using namespace std;

/**
 * tamperproof type
 */
typedef enum _NLTamperproofType_t {
  NL_TAMPERPROOF_TYPE_PROCESS = 0, 
  NL_TAMPERPROOF_TYPE_FILE    = 1,
  NL_TAMPERPROOF_TYPE_REGKEY  = 2,
  NL_TAMPERPROOF_TYPE_ALL     = 3
} NLTamperproofType;

/**
 * tamperproof access
 */
typedef enum _NLTamperproofAccess_t {
  NL_TAMPERPROOF_ACCESS_ALL = 0,    //all access allowed 
  NL_TAMPERPROOF_ACCESS_RO    = 1,  //read only
  NL_TAMPERPROOF_ACCESS_NOKILL = 2, //kill not allowed for process
  NL_TAMPERPROOF_ACCESS_NONE  = 3   //no access allowed
} NLTamperproofAccess;

/**
 * tamperproof access
 */
typedef struct NLTamperproof {
  std::set<nlstring> exemptProc;
  nlstring root;
  NLTamperproofAccess access;
  NLTamperproofType type;
}NLTamperproofEntry;

typedef std::map<nlstring, NLTamperproofEntry> NLTamperproofMap;

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
				     NLTamperproofMap **outBuffer);
  
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
bool NLTamperproofConfiguration_Free(NLTamperproofMap *buffer);

#endif /*nlTamperproofConfig.h*/
