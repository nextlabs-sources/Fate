/*********************************************************************
 *
 * CEAdapter.h
 *
 * This file provides the interface for adapter state of hooked
 * functions.
 *
 * Notes: This interface is not completely MT-safe.
 *
 ********************************************************************/

#ifndef __CEADAPTER_H__
#define __CEADAPTER_H__

//#include <string>
#include <vector>
#include <stack>

#include <windows.h>

#ifndef WINAPI
  #define WINAPI __stdcall
#endif

/* Export support */
#define CEADAPTER_EXPORT __declspec(dllexport) 

/** ce_noncopyable
 *
 *  Class to provide non-copy semantics to a class which derives from it.
 */
class ce_noncopyable
{
 public:
  ce_noncopyable() { /* empty */ }
 private:
  ce_noncopyable( const ce_noncopyable& );
  ce_noncopyable& operator=( const ce_noncopyable& );
};/* ce_noncopyable */

/** Adapter install level.  This is specified as part of the hook library
 *  configuration to inform the injection framework where the adapter
 *  should be installed with respect to other adapters injected into
 *  the process.
 */
enum
{
  CEADAPTER_MIN_LEVEL     = 0,   /** Minimum */
  CEADAPTER_DEFAULT_LEVEL = 5,   /** Default - "don't care" */
  CEADAPTER_MAX_LEVEL     = 9    /** Maximum */
};

/** CEADAPTER_SetDeny
 *
 *  SetDeny actual call to hooked function.
 */
extern "C" CEADAPTER_EXPORT void CEADAPTER_SetDeny(void);

/** CEADAPTER_IsDenied
 *
 *  Determine if a request to cancel the actual hooked function has
 *  been made.
 */
extern "C" CEADAPTER_EXPORT int WINAPI CEADAPTER_IsDenied(void);

/** CEADAPTER_IsEndPoint
 *
 *  Determine if the current context is the end-point.
 */
extern "C" CEADAPTER_EXPORT int WINAPI CEADAPTER_IsEndPoint(void);

/** CEADAPTER_AddHook
 *
 *  \brief Add a hooked function for a given library.
 *
 *  \param lib (in)     Library name.
 *  \param fn_name (in) Function name in the given library.
 *  \param fn (in)      Function pointer from the given library.
 */
extern "C" CEADAPTER_EXPORT int WINAPI CEADAPTER_AddHook( const char* lib , 
							  const char* fn_name ,
							  void* fn );

/** adapter
 *
 *  \brief Adapter state for current hooked function call.
 */
class adapter : public ce_noncopyable
{
 private:

  /** TypeWrapper
   *
   *  \brief Type wrapper base class.  This is an abstract base class used
   *         as a handle to a concrete type instance.
   */
  class TypeWrapper
  {
    public:
      virtual ~TypeWrapper() {}
  };/* TypeWrapper */

  /** TypeWrapperConcrete
   *
   *  \brief Type wrapper concrete class. Encapsulate a type and its value
   *         such that it can be stored in thread-local state for use in
   *         Next<>().  Only the copy constructor is available since the
   *         class instance must be initialized using an instance of T.
   */
  template <typename T>
  class TypeWrapperConcrete : public TypeWrapper
  {
    public:
      TypeWrapperConcrete( const T& t ) : type_instance(t)
      {
	/* empty */
      }
      T type_instance;   /* type instance */
    private:
      TypeWrapperConcrete(void);
      TypeWrapperConcrete& operator=( const TypeWrapperConcrete& );
  };/* TypeWrapperConcrete */

  /** CallStackCounter
   *
   *  \brief Helper class indented to perform accounting of current location
   *         of hook function stack.  Its constructor increments the stack
   *         location and its destructor decrements it.
   *         This class access the thread-local instance of 'adapter::state'.
   */
  class CallStackCounter : public ce_noncopyable
  {
    public:
      CEADAPTER_EXPORT CallStackCounter(void);   /** Increment stack counter */
      CEADAPTER_EXPORT ~CallStackCounter(void);  /** Decrement stack counter */
  };/* CallStackCounter */

  /** NextFunction
   *
   *  \brief Return the next adapter to call in the adapter stack.
   *         SetAdapterState() must be called first.
   *
   *  \return Return NULL at the end of the stack.  In practice this
   *          this should never occur since the last adapter on the
   *          stack actual hooked function which does *not* call
   *          into the next adapter - there is none.
   *
   *  \note MT-safe.  This should not be called directly.  It is called
   *        only from the context of Next<>(...) template generated function.
   *        Does not throw.
   *
   *  \sa SetAdapterState, TargetFunction.
   */
  CEADAPTER_EXPORT static void* WINAPI NextFunction(void);

 public:

  /** state
   *
   *  \brief State for thread-local state of current hooked function traversal.
   */
  class state : public ce_noncopyable
  {
    public:
      state(void);
      ~state(void);

      /* State of current hook. */
      class current_state
      {
        public:
	  current_state(void);
	  size_t index;                    /** Current location in the stack.  This is modified
					       only by CallStackCounter class and by Reset.*/
	  bool is_denied;                  /** End-point should call hooked function? */
	  const std::vector<void*>* stack; /** Function stack */
	  TypeWrapper* return_value;       /** Return value (abstract base) */
      };/* current_state */

      /* The current thread may have more than a single hook state.  This can occur when
         the target of a hooked function calls a hooked function.  There are then two hooked
         functions in the call stack with separate states.
      */
      std::stack<current_state> state_stack;

      /* Pointer to target function (e.g. CreateFileW). */
      volatile void (WINAPI* target)(void);
  };/* state */

  /** State
   *
   *  \brief Return a pointer to the current thread state.
   *
   *  \return Pointer to the current thread state.  If a thread state does not exist one
   *          will be created.
   */
  CEADAPTER_EXPORT static adapter::state* WINAPI State(void);

  /** CurrentState
   *
   *  \brief Return a pointer to the thread state's current adapter state.
   *
   *  \return The state instance for the current thread.  When there is no hook state
   *          NULL is returned.
   *
   *  \note Does not throw.
   */
  CEADAPTER_EXPORT static adapter::state::current_state* WINAPI CurrentState(void);

  /** TargetFunction
   *
   *  \brief Returns the target function address.
   *
   *  \param lib (in)    Name of library the symbol resides in.
   *  \param symbol (in) Name of the symbol.
   *
   *  \return Return the target function address or NULL on error.
   *
   *  \note MT-safe.  This should not be called directly.
   *
   *  \sa SetAdapterState, NextFunction, AddHook, RemoveHook.
   */
  CEADAPTER_EXPORT static void* WINAPI TargetFunction( const char* lib ,
						       const char* symbol );

  /** AddHook
   *
   *  \brief Add a hook for a given <lib,function>.  After a successful call
   *         to AddHook it is possible that the hooked function is called.
   *
   *  \param lib (in)      Name of library which contains function to hook.
   *  \param symbol (in)   Name of function to hook in given library.
   *  \param fn (in)       Address of hook function instead of calling the
   *                       target library directly.
   *
   *  \return On success true, else false.
   *
   *  \note Not MT-safe.  AddHook should be called only from within the adapter
   *        entry point, AdapterEntry.  Does not throw.
   *  \sa RemoveHook, TargetFunction.
   */
  CEADAPTER_EXPORT static bool WINAPI AddHook( const char* lib , 
					       const char* symbol ,
					       void* fn );

  /** RemoveHook
   *
   *  \brief Remove a hook added by AddHook().
   *
   *  \param lib (in)      Name of library which contains function to hook.
   *  \param symbol (in)   Name of function to hook in given library.
   *  \param fn (in)       Address of hook function instead of calling the
   *                       target library directly.
   *
   *  \return On success true, else false.
   *
   *  \note Not MT-safe.  RemoveHook should be called only from within the adapter
   *        entry point, AdapterEntry.  Does not throw.
   *  \sa AddHook, TargetFunction.
   */
  CEADAPTER_EXPORT static bool WINAPI RemoveHook( const char* lib , 
						  const char* symbol ,
						  void* fn );

  /** SetAdapterState
   *
   *  \brief Set thread-local storage state for the current hooked function
   *         <lib,function> so that the NextXXX() calls are able to operate.
   *
   *  \param lib    Target library.
   *  \param symbol Target function from target library.
   *
   *  \return true when the adapter state is set for the given library and
   *          function.  Otherwise false.
   *
   *  \note MT-safe.  Does not throw.
   *
   *  \sa NextFunction, TargetFunction.
  */
  CEADAPTER_EXPORT static bool WINAPI SetAdapterState( const char* lib ,
						       const char* symbol );

  /** SetAdapterState
   *
   *  \brief Set thread-local storage state for the current hooked function
   *         <lib,function> so that the NextXXX() calls are able to operate.
   *
   *  \param fn Function of hook.
   *
   *  \return true when the adapter state is set for the given library and
   *          function.  Otherwise false.
   *
   *  \note MT-safe.  Does not throw.
   *
   *  \sa NextFunction, TargetFunction.
  */
  CEADAPTER_EXPORT static bool WINAPI SetAdapterState( const void* fn ); 

  /** SetDeny
   *
   *  \brief SetDeny the current hooked call.  The actuall functiong being
   *         hooked will not be called.  SetAdapterState() must be called first.
   *
   *  \sa IsDenied, SetDenyReturnValue.
   *
   *  \return None.
   *
   *  \note MT-safe.  Does not throw.
   */
  CEADAPTER_EXPORT static void WINAPI SetDeny(void);

  /** IsDenied
   *
   *  \brief Determine if the current hooked funciton has been cancelled.
   *         SetAdapterState() must be called first.
   *
   *  \return True if the current hooked function has been cancelled by
   *          a call to SetDeny().
   *
   *  \sa SetDeny.
   *
   *  \note MT-safe.  Does not throw.
   */
  CEADAPTER_EXPORT static bool WINAPI IsDenied(void);

  /** IsEndPoint
   *
   *  \brief Determine if the current hooked funciton has been cancelled.
   *         SetAdapterState() must be called first.
   *
   *  \return True if the current context is the end-point.
   *
   *  \note MT-safe.  Does not throw.
   */
  CEADAPTER_EXPORT static bool WINAPI IsEndPoint(void);

  /** SetDenyReturnValue
   *
   *  \brief Set return value for the current calling sequence.  As a
   *         side effect SetDeny() will be called.
   *
   *  \param return_value (in) Return value.
   *
   *  \sa SetDeny, IsDenied.
   *
   *  \note MT-safe.  Throws according to type T provided by caller.
   */
  template <typename T>
  CEADAPTER_EXPORT static void WINAPI SetDenyReturnValue( const T& return_value )
  {
    adapter::state::current_state* cs = adapter::CurrentState();
    // Free existing value if any
    if( cs->return_value != NULL ) 
    {
      delete cs->return_value;
    }
    // Create instance of return type with the callers value.
    cs->return_value = new TypeWrapperConcrete<T>(return_value);
    // Set deny state per documented side-effect
    adapter::SetDeny();
  }/* SetDenyReturnValue */

  /* Default return values.  Provided to simplify hook functions where
     specializations for known types can be used (e.g. BOOL, HANDLE).
     For use of non-specialized instance the type must be constructed
     such that it is in an initialized state.
   */
  template <typename R>
  static R WINAPI DefaultReturnValue(void)
  {
    return R(); // requires default constructor
  }

  /** DefaultReturnValue
   *
   *  \brief BOOL specialization.
   */
  template <BOOL>
  static BOOL WINAPI DefaultReturnValue(void)
  {
    return FALSE;
  }

  /** DefaultReturnValue
   *
   *  \brief HANDLE specialization.
   */
  template <HANDLE>
  static HANDLE WINAPI DefaultReturnValue(void)
  {
    return INVALID_HANDLE_VALUE;
  }

  /** ReturnValue
   *
   *  \brief Provide the return value set in SetDenyReturnValue.
   *
   *  \return Instance of return value set by SetDEnyReturnValue.
   *
   *  \note MT-safe.  SetReturnValue for type T must have been called.
   */
  template <typename T>
  CEADAPTER_EXPORT static T WINAPI ReturnValue(void)
  {
    adapter::state::current_state* cs = adapter::CurrentState();
    // When there is no return value the default, if any, should be used.
    if( cs->return_value == NULL )
    {
      return adapter::DefaultReturnValue<T>();
    }
    return static_cast< TypeWrapperConcrete<T>* >(cs->return_value)->type_instance;
  }/* ReturnValue */

  /************************************************************************
   * Templates for halding call into next adapter.  These are type-safe
   * wrappers for traversing the adapter stack for a given hooked funcion.
   *
   * These are strongly typed, so it is important that their use with
   * actual hooked function signature is used.
   *
   * Next  - Next with non-void return type.
   * NextV - Next with void return type.
   *
   * The adapter calling Next() must first check the end-point and
   * cancel state to determine if Next() should be called.  When
   * the calling adapter is an end-point and the cancel state is set it
   * should not call Next() which would result in the target function
   * being called.  Instead it should set any state required to simulate
   * the target function and, when applicable, return an appropriate
   * return value.
   *
   *   See documentation for:
   *     adapter::IsEndPoint()
   *     adapter::IsDenied()
   *
   ***********************************************************************/
  template <typename R>
  static R WINAPI Next(void)
  {
    CallStackCounter counter;
    if( adapter::IsDenied() == true && adapter::IsEndPoint() == true )
    {
      return adapter::ReturnValue<R>();
    }
    typedef R (WINAPI *fn_t)();
    fn_t fn = (fn_t)adapter::NextFunction();
    return (*fn)();
  }

  template <typename R,typename T0>
  static R WINAPI Next( T0 t0 )
  {
    CallStackCounter counter;
    if( adapter::IsDenied() == true && adapter::IsEndPoint() == true )
    {
      return adapter::ReturnValue<R>();
    }
    typedef R (WINAPI *fn_t)(T0 t0);
    fn_t fn = (fn_t)adapter::NextFunction();
    return (*fn)(t0);
  }

  template <typename R,typename T0,typename T1>
  static R WINAPI Next( T0 t0 , T1 t1)
  {
    CallStackCounter counter;
    if( adapter::IsDenied() == true && adapter::IsEndPoint() == true )
    {
      return adapter::ReturnValue<R>();
    }
    typedef R (WINAPI *fn_t)(T0 t0,T1 t1);
    fn_t fn = (fn_t)adapter::NextFunction();
    return (*fn)(t0,t1);
  }

  template <typename R, typename T0, typename T1, typename T2>
  static R WINAPI Next( T0 t0 , T1 t1 , T2 t2)
  {
    CallStackCounter counter;
    if( adapter::IsDenied() == true && adapter::IsEndPoint() == true )
    {
      return adapter::ReturnValue<R>();
    }
    typedef R (WINAPI *fn_t)(T0 t0,T1 t1,T2 t2);
    fn_t fn = (fn_t)adapter::NextFunction();
    return (*fn)(t0,t1,t2);
  }

  template <typename R,typename T0,typename T1,typename T2,typename T3>
  static R WINAPI Next( T0 t0 , T1 t1 , T2 t2 , T3 t3 )
  {
    CallStackCounter counter;
    if( adapter::IsDenied() == true && adapter::IsEndPoint() == true )
    {
      return adapter::ReturnValue<R>();
    }
    typedef R (WINAPI *fn_t)(T0 t0,T1 t1,T2 t2,T3 t3);
    fn_t fn = (fn_t)adapter::NextFunction();
    return (*fn)(t0,t1,t2,t3);
  }

  template <typename R,typename T0,typename T1,typename T2,typename T3,typename T4>
  static R WINAPI Next( T0 t0 , T1 t1 , T2 t2 , T3 t3, T4 t4 )
  {
    CallStackCounter counter;
    if( adapter::IsDenied() == true && adapter::IsEndPoint() == true )
    {
      return adapter::ReturnValue<R>();
    }
    typedef R (WINAPI *fn_t)(T0 t0,T1 t1,T2 t2,T3 t3,T4 t4);
    fn_t fn = (fn_t)adapter::NextFunction();
    return (*fn)(t0,t1,t2,t3,t4);
  }

  template <typename R,typename T0,typename T1,typename T2,typename T3,typename T4,typename T5>
  static R WINAPI Next( T0 t0 , T1 t1 , T2 t2 , T3 t3, T4 t4 , T5 t5 )
  {
    CallStackCounter counter;
    if( adapter::IsDenied() == true && adapter::IsEndPoint() == true )
    {
      return adapter::ReturnValue<R>();
    }
    typedef R (WINAPI *fn_t)(T0 t0,T1 t1,T2 t2,T3 t3,T4 t4,T5 t5);
    fn_t fn = (fn_t)adapter::NextFunction();
    return (*fn)(t0,t1,t2,t3,t4,t5);
  }

  template <typename R,typename T0,typename T1,typename T2,typename T3,typename T4,typename T5,typename T6>
  static R WINAPI Next( T0 t0 , T1 t1 , T2 t2 , T3 t3, T4 t4 , T5 t5 , T6 t6 )
  {
    CallStackCounter counter;
    if( adapter::IsDenied() == true && adapter::IsEndPoint() == true )
    {
      return adapter::ReturnValue<R>();
    }
    typedef R (WINAPI WINAPI *fn_t)(T0 t0,T1 t1,T2 t2,T3 t3,T4 t4,T5 t5,T6 t6);
    fn_t fn = (fn_t)adapter::NextFunction();
    return (*fn)(t0,t1,t2,t3,t4,t5,t6);
  }

  template <typename R,typename T0,typename T1,typename T2,typename T3,typename T4,typename T5,typename T6,typename T7>
  static R WINAPI Next( T0 t0 , T1 t1 , T2 t2 , T3 t3, T4 t4 , T5 t5 , T6 t6 , T7 t7 )
  {
    CallStackCounter counter;
    if( adapter::IsDenied() == true && adapter::IsEndPoint() == true )
    {
      return adapter::ReturnValue<R>();
    }
    typedef R (WINAPI *fn_t)(T0 t0,T1 t1,T2 t2,T3 t3,T4 t4,T5 t5,T6 t6,T7 t7);
    fn_t fn = (fn_t)adapter::NextFunction();
    return (*fn)(t0,t1,t2,t3,t4,t5,t6,t7);
  }

  /**************************************************************
   * Specialization of 'void' functions since return value code
   * is not required.
   *************************************************************/
  template <typename T0>
  static void WINAPI NextV( T0 t0 )
  {
    CallStackCounter counter;
    if( adapter::IsDenied() == true && adapter::IsEndPoint() == true )
    {
      return;
    }
    typedef void (WINAPI *fn_t)(T0 t0);
    fn_t fn = (fn_t)adapter::NextFunction();
    (*fn)(t0);
  }

  template <typename T0,typename T1>
  static void WINAPI NextV( T0 t0 , T1 t1)
  {
    CallStackCounter counter;
    if( adapter::IsDenied() == true && adapter::IsEndPoint() == true )
    {
      return;
    }
    typedef void (WINAPI *fn_t)(T0 t0,T1 t1);
    fn_t fn = (fn_t)adapter::NextFunction();
    (*fn)(t0,t1);
  }

  template <typename T0,typename T1,typename T2>
  static void WINAPI NextV( T0 t0 , T1 t1 , T2 t2)
  {
    CallStackCounter counter;
    if( adapter::IsDenied() == true && adapter::IsEndPoint() == true )
    {
      return;
    }
    typedef void (WINAPI *fn_t)(T0 t0,T1 t1,T2 t2);
    fn_t fn = (fn_t)adapter::NextFunction();
    (*fn)(t0,t1,t2);
  }

  template <typename T0, typename T1,typename T2,typename T3>
  static void WINAPI NextV( T0 t0 , T1 t1 , T2 t2 , T3 t3)
  {
    CallStackCounter counter;
    if( adapter::IsDenied() == true && adapter::IsEndPoint() == true )
    {
      return;
    }
    typedef void (WINAPI *fn_t)(T0 t0,T1 t1,T2 t2,T3 t3);
    fn_t fn = (fn_t)adapter::NextFunction();
    (*fn)(t0,t1,t2,t3);
  }
  
  template <typename T0, typename T1,typename T2,typename T3,typename T4>
  static void WINAPI NextV( T0 t0 , T1 t1 , T2 t2 , T3 t3, T4 t4 )
  {
    CallStackCounter counter;
    if( adapter::IsDenied() == true && adapter::IsEndPoint() == true )
    {
      return;
    }
    typedef void (WINAPI *fn_t)(T0 t0,T1 t1,T2 t2,T3 t3,T4 t4);
    fn_t fn = (fn_t)adapter::NextFunction();
    (*fn)(t0,t1,t2,t3,t4);
  }

  template <typename T0,typename T1,typename T2,typename T3,typename T4,typename T5>
  static void WINAPI NextV( T0 t0 , T1 t1 , T2 t2 , T3 t3 , T4 t4 , T5 t5 )
  {
    CallStackCounter counter;
    if( adapter::IsDenied() == true && adapter::IsEndPoint() == true )
    {
      return;
    }
    typedef void (WINAPI *fn_t)(T0 t0,T1 t1,T2 t2,T3 t3,T4 t4,T5 t5);
    fn_t fn = (fn_t)adapter::NextFunction();
    (*fn)(t0,t1,t2,t3,t4,t5);
  }

  /** Target
   *
   *  \brief Call the target function for the current adapter state.  For example,
   *         if <CreateFileW,kernel32.dll> is hooked, calling Target<>() will call
   *         CreateFileW in the library kernel32.dll.  This method provides a
   *         mechanism for adapters to call actual functions within a hooked call.
   */
  template <typename R>
  static R WINAPI Target(void)
  {
    typedef R (WINAPI *fn_t)();
    fn_t fn = (fn_t)adapter::Target();
    return (*fn)();
  }

  template <typename R,typename T0>
  static R WINAPI Target( T0 t0 )
  {
    typedef R (WINAPI *fn_t)(T0 t0);
    fn_t fn = (fn_t)adapter::Target();
    return (*fn)(t0);
  }

  template <typename R,typename T0,typename T1>
  static R WINAPI Target(T0 t0,T1 t1)
  {
    typedef R (WINAPI *fn_t)(T0 t0,T1 t1);
    fn_t fn = (fn_t)adapter::Target();
    return (*fn)(t0,t1);
  }

}; /* Adapter */

#endif /* __CEADAPTER_H__ */
