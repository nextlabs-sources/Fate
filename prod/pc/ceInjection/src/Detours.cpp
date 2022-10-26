/*=========================Detours.cpp======================================*
 *                                                                          *
 * All sources, binaries and HTML pages (C) copyright 2007 by NextLabs,     *
 * San Mateo City CA, Ownership remains with NextLabs Inc,                  *
 * All rights reserved worldwide.                                           *
 *                                                                          *
 * Author : Heidi Zhou                                                      *
 * Date   : 10/18/2007                                                      *
 * Note   : Define data structures for managing detour functions.           *
 *==========================================================================*/
#include "Detours.h"

APIDetours::~APIDetours()
{
  std::multimap<DetourPriority, DetourItem *>::iterator it;
  for(it=preDetourFuncs.begin(); it != preDetourFuncs.end(); it++)
    delete it->second;

  for(it=postDetourFuncs.begin(); it != postDetourFuncs.end(); it++)
    delete it->second;
}

bool APIDetours::AddOneDetour(const char *preDetourName, 
			      PVOID preDetourPtr, 
			      const char *postDetourName,
			      PVOID postDetourPtr)
{
  if(preDetourName && preDetourPtr!=NULL) {
    DetourItem *pd=new DetourItem(preDetourName, preDetourPtr);
    preDetourFuncs.insert(std::pair<DetourPriority, DetourItem*>(HIGH, pd));
  }

  if(postDetourName && postDetourPtr!=NULL) {
    DetourItem *pd=new DetourItem(postDetourName, postDetourPtr);
    postDetourFuncs.insert(std::pair<DetourPriority, DetourItem*>(HIGH, pd));
  }
  return true;
}
