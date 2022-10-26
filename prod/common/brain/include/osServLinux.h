//
// All sources, binaries and HTML pages (C) copyright 2006 by Blue Jungle., 
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
// Best viewed by 80-column terminal
// <-------------------------------------------------------------------------->

#ifndef OSSERVLINUX_H
#define OSSERVLINUX_H

#include <semaphore.h>

// ---------------------------------------------------------------------------
// Base class for us to abstract out the Java object
// for all the OS Service Primitives
// ---------------------------------------------------------------------------

class osServiceObj {
protected:
  osServiceObj();
  virtual ~osServiceObj();
  void    setName (const char * n);
  char    name[PATH_MAX];
  bool    owner;
  int     ref_count;

public:
  virtual void  close() = 0;
};


// ---------------------------------------------------------------------------
// Class for shared memory map primitives
// The implementation is based on the POSIX shared memory
// ---------------------------------------------------------------------------

// Class for Share Memory Map
class bjMemoryMap:public osServiceObj {
 private:
  int    fd;                    // file descriptor
  int    size;                  // size of the mapping
  void * ptr;                   // mmap ptr

 public:  
  bjMemoryMap ();
  ~bjMemoryMap();
  BJ_error_t  create (const char * n, int s); // name and size
  BJ_error_t  open   (const char * n);        // name
  void * map    (); 
  void   unmap  (); 
  void * getmap ();
  void   close  ();
};

// ---------------------------------------------------------------------------
// Class for semaphore primitives
// The implementation is based on the POSIX semaphore
// ---------------------------------------------------------------------------

class bjSemaphore:public osServiceObj {
 protected:
  sem_t * semid;

 public:
  bjSemaphore ();
  ~bjSemaphore();
  BJ_error_t      create (const char * n, int v);
  BJ_error_t      open   (const char * n);
  BJ_waitResult_t take   (timespec t);
  BJ_waitResult_t take   ();
  BJ_error_t      give   ();
  void            close  ();

};

// ---------------------------------------------------------------------------
// Class for mutex primitives
// The implementation is based on the POSIX semaphore
// ---------------------------------------------------------------------------

class bjMutex:public bjSemaphore {
 private:
  pid_t     pid;
  pthread_t tid;

 public:
  bjMutex  ();
  ~bjMutex ();
  BJ_error_t      create (const char * n);
  BJ_waitResult_t take   (timespec t);
  BJ_waitResult_t take   ();
  BJ_error_t      give   ();
};


#endif  /* OSSERVLINUX_H */
