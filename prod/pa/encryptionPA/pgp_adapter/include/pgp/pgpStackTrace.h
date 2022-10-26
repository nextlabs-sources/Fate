/*____________________________________________________________________________
	Copyright (C) 2007 PGP Corporation
	All rights reserved.

	$Id$
____________________________________________________________________________*/

#ifndef Included_pgpStackTrace_h	/* [ */
#define Included_pgpStackTrace_h

#include "pgpBase.h"

PGP_BEGIN_C_DECLARATIONS

char *pgpAllocCurrentStackAsString(void);

/* These Win32-only functions are ref-counted */
void pgpSDKWin32DebugInit();
void pgpSDKWin32DebugCleanup();

void pgpDumpLeak( size_t allocSize, void **array );
int pgpGetCurrentStackAsArray( void **p, unsigned max_size );

PGP_END_C_DECLARATIONS

#if defined(__cplusplus) && (PGP_WIN32 || PGP_UNIX_LINUX || PGP_UNIX_DARWIN)

#include <string>
#include <vector>
#ifdef __GNUC__
#include <ext/malloc_allocator.h>
typedef std::basic_string<char, std::char_traits<char>, __gnu_cxx::malloc_allocator<char> > string_with_malloc;
typedef std::vector<string_with_malloc, __gnu_cxx::malloc_allocator<string_with_malloc> > string_vector;
typedef std::vector<void *, __gnu_cxx::malloc_allocator<void*> > ptr_vector;
#else

/* The definition of standard-mandated malloc_allocator is missing from VS.NET STL. 
 * Took it verbatium from April 15, 2003 Dr. Dobb's article 
 * "The Standard Librarian: What Are Allocators Good For?"
 */

namespace pgppfl  {
template <class T> class malloc_allocator
{
public:
  typedef T                 value_type;
  typedef value_type*       pointer;
  typedef const value_type* const_pointer;
  typedef value_type&       reference;
  typedef const value_type& const_reference;
  typedef size_t       size_type;
  typedef ptrdiff_t    difference_type;
  
  template <class U> 
  struct rebind { typedef malloc_allocator<U> other; };

  malloc_allocator() {}
  malloc_allocator(const malloc_allocator&) {}
  template <class U> 
  malloc_allocator(const malloc_allocator<U>&) {}
  ~malloc_allocator() {}

  pointer address(reference x) const { return &x; }
  const_pointer address(const_reference x) const { 
    return x;
  }

  pointer allocate(size_type n, void *dummy = 0) {
    void* p = malloc(n * sizeof(T));
    if (!p)
      throw std::bad_alloc();
    return static_cast<pointer>(p);
  }

  void deallocate(pointer p, size_type) { free(p); }

  size_type max_size() const { 
    return static_cast<size_type>(-1) / sizeof(T);
  }

  void construct(pointer p, const value_type& x) { 
    new(p) value_type(x); 
  }
  void destroy(pointer p) { p->~value_type(); }

private:
  void operator=(const malloc_allocator&);
};

template<> class malloc_allocator<void>
{
  typedef void        value_type;
  typedef void*       pointer;
  typedef const void* const_pointer;

  template <class U> 
  struct rebind { typedef malloc_allocator<U> other; };
};


template <class T>
inline bool operator==(const malloc_allocator<T>&, 
                       const malloc_allocator<T>&) {
  return true;
}

template <class T>
inline bool operator!=(const malloc_allocator<T>&, 
                       const malloc_allocator<T>&) {
  return false;
}
} // namespace pgppfl

typedef std::basic_string<char, std::char_traits<char>, pgppfl::malloc_allocator<char> > string_with_malloc;
typedef std::vector<string_with_malloc, pgppfl::malloc_allocator<string_with_malloc> > string_vector;
typedef std::vector<void *, pgppfl::malloc_allocator<void*> > ptr_vector;

#endif	

int pgpGetCurrentStack(string_vector &stack);

#endif	/* platforms on which we track the stack */


#endif /* ] Included_pgpStackTrace_h */
