#include <winsock2.h>
#include <winsock.h>
#include <cassert>
#include <algorithm>
#include <string>
#include <vector>

#include "celog.h"
#include "CEAdapter.h"

#include "madCHook_helper.h"

#undef TRACE
#define TRACE

/** hook
 *
 *  \brief Hook entry parameterized by <lib,function>.
 */
class hook
{
  public:
    hook( const char* clib , const char* cfunction ) :
      lib(clib),
      symbol(cfunction),
      fn_stack(),
      target(NULL)
    {
      /* empty */
    }

    std::string lib;              // Library name (e.g. kernel32.dll)
    std::string symbol;           // Symbol name (e.g. CreateFile)
    std::vector<void*> fn_stack;  // Adapter stack

    /** Target function (e.g. <CreateFileW,kernel32.dll>).  This space
	is used by HookAPI (madCHook).  It seems that the madCHook library
	retains a pointer to this member for its implementation.
    */
    volatile void (__stdcall* target)(void);

  private:
    hook(void);           // Avoid use of default constructor.
    hook( const hook& );
};/* hook */

/** hook_find_pred
 *
 *  Unary predicate for us with std::find_if.  It is constructed with:
 *    (a) <lib,symbol>
 *    (b) <fn>
 *
 *  Either of the above parameters can be used to uniquely identify the
 *  adapter state.
 */
class hook_find_pred
{
  public:
    /** hook_find_pred
     *
     *  \param in_lib (in)    Library.
     *  \param in_symbol (in) Symbol in library.
     */
    hook_find_pred( const char* in_lib , const char* in_symbol ) :
      lib(in_lib),
      symbol(in_symbol)
    {
      /* empty */
    }

    hook_find_pred( const void* in_fn ) :
      lib(NULL),
      symbol(NULL),
      fn(in_fn)
    {
    }

    /* Used by std::find_if for comparison to 'hook*' type */
    bool operator () ( const hook* h )
    {
      assert( h != NULL );
      if( h == NULL )
      {
	return false;
      }
      /* <lib,symbol> find */
      if( lib != NULL && symbol != NULL )
      {
	return (lib == h->lib && symbol == h->symbol);
      }

      /* <fn> find - Search adapter list */
      if( std::find(h->fn_stack.begin(),h->fn_stack.end(),fn) != h->fn_stack.end() )
      {
	return true;
      }
      return false;
    }
  private:
    const char* lib;      // Library 
    const char* symbol;   // Symbol
    const void* fn;       // Hook function
};/* hook_find_pred */

/* Aggregate hook list */
static std::vector<hook*> hlist;

/* Thread local index for TlsGetValue() */
extern DWORD cedi_tls_index;

/*********************************************************************
 * C interface implementation
 ********************************************************************/
extern "C" void CEADAPTER_SetDeny(void)
{
  adapter::SetDeny();
}/* CEADAPTER_SetDeny */

extern "C" int WINAPI CEADAPTER_IsDenied(void)
{
  return (int)adapter::IsDenied();
}/* CEADAPTER_IsDenied */

extern "C" int WINAPI CEADAPTER_IsEndPoint(void)
{
  return (int)adapter::IsEndPoint();
}/* CEADAPTER_IsEndPoint */

extern "C" int WINAPI CEADAPTER_AddHook( const char* lib , 
					 const char* symbol ,
					 void* fn )
{
  return (int)adapter::AddHook(lib,symbol,fn);
}/* CEADAPTER_AddHook */

bool WINAPI adapter::AddHook( const char* lib , 
			      const char* symbol ,
			      void* fn )
{
  assert( lib != NULL && symbol != NULL && fn != NULL );

  TRACE(CELOG_DEBUG,L"adapter::AddHook: <%hs,%hs,0x%x> : fn 0x%x\n", lib, symbol, fn);

  /* If the hook entry already exists, use it to add another adapter's
     function.
  */
  std::vector<hook*>::iterator it;
  it = std::find_if(hlist.begin(),hlist.end(),hook_find_pred(lib,symbol));

  bool result = true;
  if( it == hlist.end() )
  {
    /* next_fn is the next hook function.  Since ceInjection hooks only a
       single function with madCHook's HookAPI() the next_fn is the target
       function (e.g. CreateFileW in kernel32.dll).
     */
    TRACE(CELOG_DEBUG,L"adapter::AddHook: HookAPI\n");

    hook* h = new (std::nothrow) hook(lib,symbol);

    if( h != NULL )
    {
      /* madCHook will call a number of methods that may be hooked.  For example,
	 when hooking CreateFile or CloseHandle these hooks are actually called
	 within the call to HookAPI.  This results in the caller's hook being called
	 without the hook being completely added.

	 To avoid this problem the hook is pushed first and we rely on HookAPI making
	 the assignment to 'target' as a side-effect before any hooks are called.
	 Then NextFunction can determine this case and forward the caller to the target
	 function thus avoiding the state problem.
      */
      h->fn_stack.push_back(fn);         /* hook function */
      hlist.push_back(h);

      BOOL rv = HookAPI(lib,symbol,fn,(PVOID*)&h->target,0);

      TRACE(CELOG_DEBUG, L"adapter::AddHook: HookAPI result %d (%d/%d)\n", result, TRUE, FALSE);

      /* If HookAPI failed remove the previously pushed hook entry. */
      if( rv != FALSE )
      {
	TRACE(CELOG_DEBUG, L"adapter::AddHook: push 0x%x (fn)\n", fn);
	TRACE(CELOG_DEBUG, L"adapter::AddHook: push 0x%x (target)\n", h->target);
	h->fn_stack.push_back(h->target);  /* target function */
      }
      else
      {
	delete h;
	hlist.pop_back();
	result = false;
      }
    }/* if( h != NULL ) */
  }
  else
  {
    TRACE(CELOG_DEBUG, L"adapter::AddHook: push 0x%x (hook)\n", fn);

    /* Insert after first element since the first element is hooked function
       via HookAPI.
     */
    (*it)->fn_stack.insert((*it)->fn_stack.begin() + 1,fn);
  }

  return result;

}/* adapter::AddHook */

bool WINAPI adapter::RemoveHook( const char* lib ,
				 const char* symbol ,
				 void* fn )
{
  assert( lib != NULL && symbol != NULL && fn != NULL );

  TRACE(CELOG_DEBUG,L"adapter::RemoveHook: UnhookAPI <%hs,%hs,0x%x>\n", lib, symbol, fn);

  if( fn == NULL )
  {
    return false;
  }

  /* There are a few cases that are handled:
   *   (1) The <lib,symbol> does not exist.  Return false.
   *   (2) The <lib,symbol> exists.  Unhook it.  There are no other
   *       hooks on the adapter function stack.  There was only one
   *       adapter hooking <lib,symbol>.
   *   (3) The <lib,symbol> exists.  Unhook it.  There is another adapter
   *       hooking <lib,symbol>.  Install it as the next hook.
   */

  std::vector<hook*>::iterator it;

  /* Find the target function in the hook list by <lib,symbol> */
  it = std::find_if(hlist.begin(),hlist.end(),hook_find_pred(lib,symbol));

  if( it == hlist.end() )
  {
    return false;
  }

  /* Unhook from the target function.  Since only a single function is hooked 
   */
  TRACE(CELOG_DEBUG,L"adapter::RemoveHook: UnhookAPI 0x%x\n", (*it)->target);

  BOOL result;
  result = UnhookAPI((PVOID*)&(*it)->target);

  TRACE(CELOG_DEBUG,L"adapter::RemoveHook: UnhookAPI 0x%x (result %d)\n", (*it)->target, result);

  /* Remove hook from hlist. */
  std::vector<void*>::iterator i;

  for( i = (*it)->fn_stack.begin() ; i != (*it)->fn_stack.end() ; ++i )
  {
    if( *i == fn )
    {
      TRACE(CELOG_DEBUG,L"adapter::RemoveHook: remove 0x%x\n", *i );
      (*it)->fn_stack.erase(i);
      break;
    }
  }

  TRACE(CELOG_DEBUG,L"adapter::RemoveHook: size %d\n", (*it)->fn_stack.size() );

  /* Case (2): If there is only a single function on the stack it is the target
     function which is no longer hooks.  This means there are no other adapters
     hooking that function, so the hook entry should be removed.
  */
  if( (*it)->fn_stack.size() <= 1 )
  {
    delete (*it);
    hlist.erase(it);
    return true;
  }

  /* Case 3: Install next adapter as hooked function via HookAPI. */
  fn = *(*it)->fn_stack.begin();
  
  TRACE(CELOG_DEBUG,L"adapter::RemoveHook: next adapter @ 0x%x\n", fn );

  result = HookAPI(lib,                    /* base lib */
		   symbol,                 /* base fn */
		   fn,                     /* hook fn */
		   (PVOID*)&(*it)->target, /* next */
		   0);

  return result;
}/* adapter::RemoveHook */

/**********************************************************************
 * C++ interface implementation
 *********************************************************************/
void* WINAPI adapter::NextFunction(void)
{
  state::current_state* cs = adapter::CurrentState();

  TRACE(CELOG_DEBUG,L"adapter::NextFunction: %d/%d\n", cs->index, cs->stack->size());

  /* Sanity check to verify that the current location is within
     the stack bounds.  When this occurs provide the target function
     if set.
  */
  if( cs->index >= cs->stack->size() )
  {
    adapter::state* st = adapter::State();
    return st->target;
  }

  /* Retreive the next adapter's function to call */
  void* fn = NULL;

  try
  {
    fn = cs->stack->at(cs->index);
  }
  catch(...)
  {
    /* empty to prevent throw */
    assert(0);
    fn = NULL;
  }

  TRACE(CELOG_DEBUG, L"adapter::NextFunction: 0x%x\n", fn);

  return fn;
}/* adapter::Next */

void* WINAPI adapter::TargetFunction( const char* lib ,
				      const char* symbol )
{
  assert( lib != NULL && symbol != NULL );
  std::vector<hook*>::iterator it;

  /* Find the target function in the hook list by <lib,symbol> */
  it = std::find_if(hlist.begin(),hlist.end(),hook_find_pred(lib,symbol));

  if( it != hlist.end() )
  {
    /* The target function is a member of the hook entry. */
    return (*it)->target;
  }

  return NULL;
}/* adapter::Target */

bool WINAPI adapter::SetAdapterState( const void* fn )
{
  std::vector<hook*>::iterator it;
  it = std::find_if(hlist.begin(),hlist.end(),hook_find_pred(fn));

  assert( it != hlist.end() );

  if( it == hlist.end() )
  {
    return false;   /* no hook entry found */
  }
  return adapter::SetAdapterState((*it)->lib.c_str(),(*it)->symbol.c_str());
}/* adapter::SetAdapterState */

bool WINAPI adapter::SetAdapterState( const char* lib,
				      const char* symbol )
{
  assert( lib != NULL && symbol != NULL );
  TRACE(0, L"adapter::SetAdapterState: <%hs,%hs>\n",lib,symbol);

  /* Find <lib,symbol> */
  std::vector<hook*>::iterator it;

  it = std::find_if(hlist.begin(),hlist.end(),hook_find_pred(lib,symbol));

  assert( it != hlist.end() );

  if( it == hlist.end() )
  {
    return false;   /* no hook entry found */
  }

  adapter::state* st = adapter::State();
  adapter::state::current_state* cs = adapter::CurrentState();

  if( cs == NULL || cs->stack != &(*it)->fn_stack )
  {
    /* The state being set by the caller does not match the current state on
       the state stack.  Push new state on the state stack which becomes the
       new current state.
    */
    adapter::state::current_state as;
    as.stack = &(*it)->fn_stack;
    st->target = (*it)->target;
    st->state_stack.push(as);
  }
  return true;
}/* adapter::SetAdapterState */

CEADAPTER_EXPORT bool WINAPI adapter::IsDenied(void)
{
  return adapter::CurrentState()->is_denied;
}/* adapter::IsDenied */

CEADAPTER_EXPORT void WINAPI adapter::SetDeny(void)
{
  adapter::CurrentState()->is_denied = true;
}/* adapter::SetDeny */

CEADAPTER_EXPORT bool WINAPI adapter::IsEndPoint(void)
{
  /* Index is current location in the stack, but the end-point is one
     before.  The last entry is the target function (e.g. CreateFileW).
  */
  adapter::state::current_state* cs = adapter::CurrentState();
  if( (cs->index + 1) >= cs->stack->size() )
  {
    return true;
  }
  return false;
}/* adapter::IsEndPoint */

adapter::state::current_state* WINAPI adapter::CurrentState(void)
{
  state* state = adapter::State();

  if( state->state_stack.empty() == true )
  {
    return NULL;
  }

  return &state->state_stack.top();
}/* adapter::CurrentState */

adapter::state* WINAPI adapter::State(void)
{
  state* state = (adapter::state*)TlsGetValue(cedi_tls_index);

  /* It is possble that the current call occurs when a library has been
     injected, and hook installed, but the current thread already existed.
     This means that DLL_THREAD_ATTACH would not have been called, so
     CreateTLS() would not have created TLS data for this thread.  In that
     case, crease the TLS state and continue.

     It is expected that this would occur during SetAdapterState which
     occurs in a hook of an existing thread per above case.  Creating TLS
     should allow the hook to continue.
  */
  if( state == NULL )
  {
    state = new (std::nothrow) adapter::state();
    TRACE(CELOG_DEBUG,L"adapter::CurrentState: No current state.  Creating one.\n");
    TlsSetValue(cedi_tls_index,(PVOID)state);
  }
  return state;
}/* adapter::State */

CEADAPTER_EXPORT adapter::CallStackCounter::CallStackCounter(void)
{
  /* Increment the call stack counter */
  adapter::CurrentState()->index++;
}/* adapter::CallStackCounter::CallStackCounter */

CEADAPTER_EXPORT adapter::CallStackCounter::~CallStackCounter(void)
{
  /* Decrement the call stack counter */
  state::current_state* cs = adapter::CurrentState();

  cs->index--;

  /* When the stack is unrolled the index will be at the base (0), so
     other thread state must be reset.  The call is ending.
  */
  if( cs->index == 0 )
  {
    /* Reset the thread-local state for next time a hook is called:
         (1) Release allocated return value.
	 (2) Pop the current hook state from the thread state stack.
     */
    if( cs->return_value != NULL )
    {
      delete cs->return_value;
    }
    adapter::State()->state_stack.pop();
  }
}/* adapter::CallStackCounter::~CallStackCounter */

/**********************************************************************
 * adapter::state
 *********************************************************************/
adapter::state::state(void) :
  state_stack(),
  target(NULL)
{
}/* adapter::state::state */

adapter::state::~state(void)
{
}/* adapter::state::~state */

adapter::state::current_state::current_state(void) :
  index(0),
  is_denied(false),
  stack(NULL),
  return_value(NULL)
{
  /* empty */
};/* adapter::state::current_state */
