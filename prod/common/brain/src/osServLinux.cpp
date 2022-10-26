//
// All sources, binaries and HTML pages (C) copyright 2006 by Blue Jungle Inc, 
// Redwood City CA, Ownership remains with Blue Jungle Inc, 
// All rights reserved worldwide. 
// 
// Author : Dominic Lam
// Date   : 5/23/2006
// Note   : First attempt to map the OS services for Linux 
//          Base class for all the OS service objects
//          We use that because the Java level does not distinguish 
//          one service from another (eg. shared memory .vs. mutex) 
//
// $Id$
// 
// Best viewed by 80-column terminal
// <-------------------------------------------------------------------------->


#include <iostream>
#include <sys/mman.h>

#include <fcntl.h>
#include <pthread.h>

#include <errno.h>

#include "brain.h"
#include "osServLinux.h"

// Todo: Probably need to change the permission, or protect by policy

#define OFLAGS   ( O_RDWR  | O_CREAT | O_EXCL )
#define OMODE    ( S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH )
#define MPROT    ( PROT_READ | PROT_WRITE )

// ---------------------------------------------------------------------------
// Base class for all the OS primitive
// ---------------------------------------------------------------------------
osServiceObj::osServiceObj()
{
  owner     = false;
  ref_count = 0;
}

osServiceObj::~osServiceObj()
{
}

void osServiceObj::setName (const char * n)
{  
    strncpy_s (name, PATH_MAX, n, _TRUNCATE);
}

// ---------------------------------------------------------------------------
// Class for shared memory map primitives
// The implementation is based on the POSIX shared memory
// ---------------------------------------------------------------------------

bjMemoryMap::bjMemoryMap()
{
  memset(name, 0x0, sizeof (name));
  fd   = -1;
  size = 0;
  ptr  = NULL;
}

bjMemoryMap::~bjMemoryMap()
{
  if (fd > 0)
    ::close (fd);
  fd = -1;

  close();
}

BJ_error_t bjMemoryMap::create (const char * n, int s)
{
  setName (n);

  shm_unlink (name);

  char tmp[1024];
  _snprintf_s(tmp,1024, _TRUNCATE, "create memmap: %s\n",name);

  fd = shm_open (name, OFLAGS, OMODE);

  if (fd < 0) {
    perror (strerror (errno));
    return BJ_ERROR;
  }
  
  if (ftruncate (fd, s) < 0) {
    return BJ_ERROR;
  }

  // Only the owner can delete it
  size      = s;
  owner     = true;
  ref_count = 1;

  return BJ_OK;
}


BJ_error_t bjMemoryMap::open (const char * n)
{
  struct stat stat;
  
  setName (n);

  char tmp[1024];
  _snprintf_s(tmp,1024, _TRUNCATE, "open memmap: %s\n",name);

  fd = shm_open (name, O_RDWR, OMODE);

  if (fd < 0) {
    perror (strerror (errno));
    return BJ_ERROR;
  }

  if (fstat (fd, &stat) < 0) {
    return BJ_ERROR;
  }

  size = stat.st_size;
  ref_count++;

  return BJ_OK;
}

void * bjMemoryMap::map()
{
  ptr = mmap (0, size, MPROT, MAP_SHARED, fd, 0);
  
  if (ptr == MAP_FAILED) {
    ptr = NULL;
  }

  // Reset the fd
  ::close (fd);
  fd = -1;
  return ptr;
}

void bjMemoryMap::unmap()
{
  if ((ptr != NULL) && (size > 0)) {
    if (munmap (ptr, size) < 0) {
    }
  }
}

void * bjMemoryMap::getmap()
{
  return ptr;
}

// The API is NOT symmetric Open/CreateFileMapping is one type
// But close is a generic type. So I'm forced to do it here.
void bjMemoryMap::close() 
{
  unmap();
  ref_count--;

  if (owner && (ref_count <= 0)) {
    if (strlen(name)) {
      shm_unlink (name);
    }
  }
  memset (name, 0x0, sizeof(name));

  return;
}

// ---------------------------------------------------------------------------
// Class for mutex primitives
// The implementation is based on the POSIX semaphore
// ---------------------------------------------------------------------------

bjSemaphore::bjSemaphore()
{
  memset (name, 0x0, sizeof(name));
  semid = SEM_FAILED;
  return;
}

bjSemaphore::~bjSemaphore()
{
  close();
}

BJ_error_t bjSemaphore::create(const char * n, int initialValue)
{
  setName (n);

  // Start from a clean slate
  sem_unlink (name);

  char tmp[1024];
  _snprintf_s(tmp,1024, _TRUNCATE, "create bjSemaphore: %s\n",name);

  // Semaphore is created as down to begin with
  semid = sem_open (name, OFLAGS, OMODE, initialValue);

  if (semid == SEM_FAILED) {
    perror (strerror (errno));
    return BJ_ERROR;
  }
  owner     = true;
  ref_count = 1;

  return BJ_OK;
}

BJ_error_t bjSemaphore::open (const char * n)
{
  setName (n);

  // Semaphore is created
  semid = sem_open (name, O_RDWR, OMODE);

  if (semid == SEM_FAILED) {
    perror (strerror (errno));
    return BJ_ERROR;
  }
  ref_count++;
  return BJ_OK;
}


// ---------------------------------------------------------------------------
// Class for semaphore primitives
// semTake takes a timeout in timespec for specifying how long it should wait 
// The return values are defined in IPCconstants.h to make it backward
// compatible with window implementation 
// ---------------------------------------------------------------------------

BJ_waitResult_t bjSemaphore::take (timespec t)
{
  timespec        ts;
  BJ_waitResult_t rc = BJ_WAIT_SUCCESS;

  clock_gettime (CLOCK_REALTIME, &ts);

  // Take care of the wrap around of nanosecond
  ts.tv_sec  +=  t.tv_sec + ((ts.tv_nsec + t.tv_nsec) / 1000000000);
  ts.tv_nsec  = (ts.tv_nsec + t.tv_nsec) % 1000000000;

  if (sem_timedwait (semid, &ts) == -1) {
    switch (errno) {
    case ETIMEDOUT:
      rc = BJ_WAIT_TIMEOUT;
      break;
    default:
      rc = BJ_WAIT_FAILED;
      break;
    }
  } 
  
  return rc;
}

BJ_waitResult_t bjSemaphore::take ()
{
  if (sem_wait(semid) == -1)
    return BJ_WAIT_FAILED;

  return BJ_WAIT_SUCCESS;
}

// SemGive
BJ_error_t bjSemaphore::give ()
{
  if (semid == SEM_FAILED)
    return BJ_ERROR;

  sem_post (semid);
  return BJ_OK;
}

void bjSemaphore::close()
{
  ref_count--;

  if (semid != SEM_FAILED) {
    sem_close (semid);
  }

  if (owner && (ref_count <= 0)) {
    if (strlen(name)) {
      sem_unlink (name);
    }
  }
  memset (name, 0x0, sizeof(name));
  return;
}

// ---------------------------------------------------------------------------
// Class for mutex primitives
// The implementation is based on the POSIX semaphore
// ---------------------------------------------------------------------------


bjMutex::bjMutex()
{
  pid = -1;
  tid = pthread_t(-1);
}

bjMutex::~bjMutex()
{
  close();
}

BJ_error_t bjMutex::create(const char * n)
{
  return bjSemaphore::create(n, 1);
}

// ---------------------------------------------------------------------------
// Class for semaphore primitives
// semTake takes a timeout in timespec for specifying how long it should wait 
// The return values are defined in IPCconstants.h to make it backward
// compatible with window implementation 
// ---------------------------------------------------------------------------

BJ_waitResult_t bjMutex::take (timespec t)
{
  // Latch the process and thread who tries to lock the mutex
  pid = getpid();
  tid = pthread_self();
  return bjSemaphore::take (t);
}

BJ_waitResult_t bjMutex::take()
{
  // Latch the process and thread who tries to lock the mutex
  pid = getpid();
  tid = pthread_self();
  return bjSemaphore::take();
}

BJ_error_t bjMutex::give ()
{
  if (semid == SEM_FAILED)
    return BJ_ERROR;

  // For mutex, the one who locks it should be the one who release it
  // Also, we make sure we only release it when it's locked
  if ((pid == getpid()) && (tid == pthread_self())) {
    int value;
    sem_getvalue (semid, &value);
    if (value == 0) {
      sem_post (semid);
      return BJ_OK;
    }
  }
  return BJ_ERROR;
}
