/************************************************************************************
 *
 * auto_disable
 *
 * Management of current hooking state.  Support for disablement of hooking for the
 * current thread or process.
 *
 ***********************************************************************************/

#ifndef __AUTO_DISABLE_H__
#define __AUTO_DISABLE_H__

#include <windows.h>
#include <cassert>
#include <map>
#include <boost/utility.hpp>

namespace nextlabs
{

  /** recursion_control
   *
   *  \brief Class to allow for recursion control.  The current thread or process may be
   *         set in an enabled or disable mode.  This mode can be checked to determine
   *         if the current context is performing recursion.
   */
  class recursion_control : boost::noncopyable
  {
    public:
    /** initialize
     *
     *  \brief Initialize auto_Support API.
     */
    recursion_control(void) :
      disabled_process(false),
      disabled_thread()
    {
      if( InitializeCriticalSectionAndSpinCount(&cs,0x80004000) == FALSE )
      {
	InitializeCriticalSection(&cs);
      }
    }/* recursion_control */
    
	  ~recursion_control()
	  {
		  DeleteCriticalSection(&cs);
	  }

    /** thread_enable
     *
     *  \brief Set hooking state for the current thread.  It is a programming error
     *         to call thread_enable for a thread that has not called
     *         thread_disable.
     *  \sa thread_disable
     */
    void thread_enable(void)
    {
      EnterCriticalSection(&cs);
      assert( disabled_thread[GetCurrentThreadId()] > 0 );
      disabled_thread[GetCurrentThreadId()]--;
      LeaveCriticalSection(&cs);
    }/* thread_enable */

    /** thread_disable
     *
     *  \brief Set hooking state for the current thread.  After this is called the
     *         current thread is set to disable recursion.
     *  \sa thread_enable
     */
    void thread_disable(void)
    {
      DWORD tid = GetCurrentThreadId();
      EnterCriticalSection(&cs);
      /* Initialize thread state to zero when this is the first state change. */
      if( disabled_thread.find(tid) == disabled_thread.end() )
      {
	disabled_thread[tid] = 0;
      }
      disabled_thread[tid]++;
      LeaveCriticalSection(&cs);
    }/* thread_disable */

    /** process_enable
     *
     *  \brief Enable hooking/recursion for the current process.
     *  \sa process_disable
     */
    void process_enable(void)
    {
      EnterCriticalSection(&cs);
      disabled_process = false;
      LeaveCriticalSection(&cs);
    }/* process_enable */
    
    /** process_disable
     *
     *  \brief Disable hooking for the current process.
     *  \sa process_enable
     */
    void process_disable(void)
    {
      EnterCriticalSection(&cs);
      disabled_process = true;
      LeaveCriticalSection(&cs);
    }/* process_disable */

    /** is_process_disabled
     *
     *  \brief Determine the hook state for the current process.
     *  \return true if the current process' hooks are disabled, otherwise
     *        false.
     *  \sa process_disable, process_enable
     */
    bool is_process_disabled(void)
    {
      bool result;
      EnterCriticalSection(&cs);
      result = disabled_process;
      LeaveCriticalSection(&cs);
      return result;
    }/* is_process_disabled */

    /** is_thread_disabled
     *
     *  \brief Determine the hook state for the current process.
     *  \return true if the current thread's hooking is disabled, otherwise
     *          false.
     *  \sa thread_disable, thread_enable
     */
    bool is_thread_disabled(void)
    {
      bool result = false;
      DWORD tid = GetCurrentThreadId();
      EnterCriticalSection(&cs);
      /* If the current thread ID is not in the map it's not been disabled */
      if( disabled_thread.find(tid) != disabled_thread.end() )
      {
	if( disabled_thread[tid] > 0 )
	{
	  result = true;
	}
      }
      LeaveCriticalSection(&cs);
      return result;
    }/* is_thread_disabled */

    /** is_disabled
     *
     *  \brief Determine the hook/recursion state for the current process or thread.
     *  \return true if hooks are disabled for the current code path,
     *          otherwise false.
     *  \sa process_disable, process_enable, thread_disable, thread_enable
     */
    bool is_disabled(void)
    {
      bool result;
      EnterCriticalSection(&cs);
      result = is_thread_disabled() || is_process_disabled();
      LeaveCriticalSection(&cs);
      return result;
    }/* is_disabled */

  private:

    std::map<DWORD,int>  disabled_thread;   /* Thread state is disabled? */
    bool                 disabled_process;  /* Process state is disabled? */
    CRITICAL_SECTION     cs;                /* Protect state variables */

  };/* auto_control */

  /** recursion_control_auto
   *
   *  \brief Handle thread (current context) disablement automatically.  When an
   *         instance of this object is created it will disable recusion for the
   *         current thread until is is destroyed.  Typically this occurs when the
   *         stack is being unwound since an instance of this object would typically
   *         be local to the context.
   */
    class recursion_control_auto : boost::noncopyable
    {
      public:

        /** recursion_control_auto
	 *
	 *  \brief Construct an auto recursion control object.
	 *
	 *  \param in_ac (in) Recursion control instance.
	 */
        recursion_control_auto( nextlabs::recursion_control& in_ac ) : ac(in_ac)
        {
	  ac.thread_disable();
	}/* recursion_control_auto */

        ~recursion_control_auto(void)
        {
	  ac.thread_enable();
	}/* ~recursion_control_auto */

      private:
        nextlabs::recursion_control& ac;

    };/* recursion_control_auto */

}/* namespace nextlabs */

#endif /* __AUTO_DISABLE_H__ */
