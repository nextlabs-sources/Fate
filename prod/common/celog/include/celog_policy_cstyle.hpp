/**********************************************************************
 *
 * celog_policy_cstyle.hpp
 *
 *********************************************************************/

#ifndef __CELOGPOLICY_CSTYLE_HPP__
#define __CELOGPOLICY_CSTYLE_HPP__

/** CELogPolicy_CStyle
 *
 *  \brief C-style adapter for CELogPolicy.
 */
class CELogPolicy_CStyle : public CELogPolicy
{
  public:
    CELogPolicy_CStyle( int (*_policy)(const wchar_t*) ) :
      policy(_policy)
    {
      /* empty */
    }

    /* Log into c-style function */
    int Log( const wchar_t* msg )
    {
      return policy(msg);
    }

    /* Log into c-style function */
    int Log( const char* msg )
    {
      return 0;
    }

  private:
    /* Prevent link with default constructors. */
    CELogPolicy_CStyle(void);
    CELogPolicy_CStyle( const CELogPolicy_CStyle& _policy );
    CELogPolicy_CStyle& operator=( const CELogPolicy_CStyle& _policy );

    int (*policy)(const wchar_t* msg);
};/* CELogPolicy_CStyle */

#endif /* __CELOGPOLICY_CSTYLE_HPP__ */
