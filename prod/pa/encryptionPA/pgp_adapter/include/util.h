/*
Written by Derek Zheng
May 2008
*/

#ifndef _WIN32_UTIL_H_
#define _WIN32_UTIL_H_

typedef void * LockHandle;

LockHandle GsmLockAlloc(void);

void GsmLockFree(LockHandle hLock);

int GsmLock(LockHandle hLock);

void GsmUnlock(LockHandle hLock);

#endif