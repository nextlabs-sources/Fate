/********************************************************************************************
 *
 * Policy Query
 *
 *******************************************************************************************/

#ifndef __EFRMAEWORK_POLICY_QUERY_HPP__
#define __EFRMAEWORK_POLICY_QUERY_HPP__

#include <cassert>
#include <string>
#include <list>

#include <eframework/timer/timer_high_resolution.hpp>
#include <eframework/platform/policy_controller.hpp>
#include <eframework/policy/policy_obligations.hpp>

namespace nextlabs
{

  /** policy_query
   *
   *  \brief Evaluation object for policy query and decision.
   */
  class policy_query : boost::noncopyable
  {
    public:

    friend class policy_connection;

    /** policy_query
     *
     *  \brief Constructor for policy_query.
     *
     *  \param in_cesdk (in) Reference to a CE SDK instance.
     */
    policy_query() :
      deny(false),
      timeout(5000),
      event_level(3),
      perform_obligations(true)
    {
      /* empty */
    }

    /** set_obligations
     *
     *  \brief Determine if obligations should be performed.
     */
    void set_obligations(_In_ bool in_perf_obs ) throw()
    {
      perform_obligations = in_perf_obs;    /* perform obligations */
    }/* set_obligations */

    /** set_event_level
     *
     *  \brief Set the noise level of the evaluation.  The default noise level is
     *         CE_NOISE_LEVEL_USER_ACTION.
     */
    void set_event_level(_In_ int in_event_level ) throw()
    {
      event_level = in_event_level;
    }/* set_event_level */

    /** set_source
     *
     *  \brief Set the source resource of the evaluation.  If the resource type is not
     *         specified it will be document (fso) type.
     *
     *  \param in_resource (in)      Resource name.
     *  \param in_resource_type (in) Resource type (i.e. fso).
     *
     *  \return true on success, otherwise false.
     */
    bool set_source( _In_z_ const wchar_t* in_resource ,
		     _In_z_ const wchar_t* in_resource_type ) throw()
    {
      assert( in_resource != NULL );
      assert( in_resource_type != NULL );

      source_name.assign(in_resource);
      source_type.assign(in_resource_type);

      if( target_name.empty() == true ||
	  target_type.empty() == true )
      {
	return false;
      }

      return true;
    }/* set_source */

    /** add_source_attribute
     *
     *  \brief Add an attribute to the source resource.
     */
    _Check_return_
    bool add_source_attribute( _In_z_ const wchar_t* in_key ,
			       _In_z_ const wchar_t* in_value )
    {
      assert( in_key != NULL );
      assert( in_value != NULL );
      source_attrs.push_back( std::pair<std::wstring,std::wstring>(in_key,in_value) );
      return true;
    }/* add_source_attribute */

    /** set_target
     *
     *  \brief Set the target resource.
     */
    bool set_target( _In_z_ const wchar_t* in_resource ,
		     _In_z_ const wchar_t* in_resource_type ) throw()
    {
      assert( in_resource != NULL );
      assert( in_resource_type != NULL );

      target_name.assign(in_resource);
      target_type.assign(in_resource_type);

      if( target_name.empty() == true ||
	  target_type.empty() == true )
      {
	return false;
      }

      return true;
    }/* set_target */

    /** set_target_attribute
     *
     *  \brief Add an attribute to the target resource.
     */
    _Check_return_
    bool add_target_attribute( _In_z_ const wchar_t* in_key ,
			       _In_z_ const wchar_t* in_value )
    {
      assert( in_key != NULL );
      assert( in_value != NULL );
      target_attrs.push_back( std::pair<std::wstring,std::wstring>(in_key,in_value) );
      return true;
    }/* add_source_attribute */

    /** set_action
     *
     *  \brief Set the action of the evaluation.
     *  \param in_action (in) Action name (i.e. OPEN).
     *  \return true on success, otherwise false.
     *  \sa query
     */
    _Check_return_
    bool set_action( _In_z_ const wchar_t* in_action ) throw()
    {
      assert( in_action != NULL );
      action.assign(in_action);
      return true;
    }/* set_action */

    /** is_deny
     *
     *  \brief Did the evaluation result in a deny decision?
     *  \return true if the result of query was a deny, otherwise false.
     *  \sa is_allow
     */
    _Ret_ bool is_deny(void) const throw()
    {
      return deny;
    }/* is_deny */

    /** is_deny
     *
     *  \brief Did the evaluation result in a allow decision?
     *  \sa is_deny
     */
    _Ret_ bool is_allow(void) const throw()
    {
      return !is_deny();
    }/* is_allow */

    /** set_timeout
     *
     *  \brief Set the policy query timeout in milliseconds.  The default timeout
     *         is 5000 milliseconds.
     *  \param in_timeout_ms (in) Timeout in milliseconds.
     *  \sa query
     */
    void set_timeout(_In_ size_t in_timeout_ms ) throw()
    {
      timeout = (int)in_timeout_ms;
    }/* set_timeout */

    /** query_time
     *
     *  \brief Return the duration, in milliseconds, of a call to query.
     *  \sa query
     */
    _Ret_ double query_time(void) throw()
    {
      return timer.diff();
    }/* timer */

    void set_user_id(_In_z_ const wchar_t* in_user_id )
    {
      assert( in_user_id != NULL );
      user_id.assign(in_user_id);
    }/* set_user_id */

    void set_application(_In_z_ const wchar_t* in_app_path )
    {
      assert( in_app_path != NULL );
      app_path.assign(in_app_path);
    }/* set_application */

    _Ret_ const nextlabs::policy_obligations& get_obligations(void)
    {
      return obligations;
    }/* get_obligations */

    private:

      nextlabs::high_resolution_timer timer;  /* high res timer */

      std::wstring action;        /* action */

      std::wstring source_name;   /* source resource */
      std::wstring source_type;

      std::wstring target_name;   /* target resource */
      std::wstring target_type;

      std::list< std::pair<std::wstring,std::wstring> > source_attrs;
      std::list< std::pair<std::wstring,std::wstring> > target_attrs;

      std::wstring user_id;                           /* User SID (i.e., SID) */
      std::wstring app_path;                          /* Application path */

      bool deny;                                      /* evaluation result deny? */
      int timeout;                                    /* timeout (ms) */
      int event_level;                                /* event level for query */
      bool perform_obligations;                       /* perform obligations? */
      nextlabs::policy_obligations obligations;       /* obligations */

  };/* policy_query */

}/* namespace nextlabs */

#endif /* __EFRMAEWORK_POLICY_QUERY_HPP__ */
