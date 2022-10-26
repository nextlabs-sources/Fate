

#ifndef __NX_PPLX_TASKS_HPP__
#define __NX_PPLX_TASKS_HPP__


#if (defined(_MSC_VER) && (_MSC_VER >= 1800))
#include <ppltasks.h>
namespace pplx = Concurrency;
#if (_MSC_VER >= 1900)
#include <concrt.h>
namespace Concurrency {
namespace extensibility {
typedef ::std::condition_variable                           condition_variable_t;
typedef ::std::mutex                                        critical_section_t;
typedef ::std::unique_lock< ::std::mutex>                   scoped_critical_section_t;

typedef ::Concurrency::event event_t;
typedef ::Concurrency::reader_writer_lock                   reader_writer_lock_t;
typedef ::Concurrency::reader_writer_lock::scoped_lock      scoped_rw_lock_t;
typedef ::Concurrency::reader_writer_lock::scoped_lock_read scoped_read_lock_t;

typedef ::Concurrency::details::_ReentrantBlockingLock      recursive_lock_t;
typedef recursive_lock_t::_Scoped_lock                      scoped_recursive_lock_t;
}
}
#endif // defined(_MSC_VER) && (_MSC_VER >= 1800)
#endif // _MSC_VER >= 1900



#endif