
#ifndef __PCS_IDL_OBJECTS_HPP__
#define __PCS_IDL_OBJECTS_HPP__

#include <cstring>
#include <string>
#include <cassert>

typedef enum
{
  PCS_IDL_OBJECT_HEADER,
  PCS_IDL_OBJECT_METHOD
} idl_object_type_t;

/** idl_object
 *
 *  \brief IDL Object
 */
class idl_object
{
  public:

    virtual bool code_gen_client_header( std::string& code )
    {
      return false;
    }

    virtual bool code_gen_server_header( std::string& code )
    {
      return false;
    }

    virtual bool code_gen_client( std::string& code ) = 0;
    virtual bool code_gen_server( std::string& code ) = 0;

    virtual idl_object_type_t type(void) = 0;

    virtual ~idl_object(void)
    {
    }

};/* class idl_object */

#define IDL_PARAM_OPTION_IN       (0x1 << 0)   /* Input parameter */
#define IDL_PARAM_OPTION_OUT      (0x1 << 1)   /* Output parameter */
#define IDL_PARAM_OPTION_REQUIRED (0x1 << 2)   /* Required */
#define IDL_PARAM_OPTION_ALLOC    (0x1 << 3)   /* Callee allocated */
#define IDL_PARAM_OPTION_PTR      (0x1 << 4)   /* Pointer */

/** idl_param
 *
 *  \brief Parameter of an IDL method
 */
class idl_param
{
 public:
  idl_param() :
    options(0x0)
  {
  }

  std::string name;            /* Name of object (i.e., foo) */
  std::string type;            /* Type of object (i.e., int) */
  std::string type_qualifier;  /* Type qualifier of object (i.e., const) */
  std::string size_of_object;  /* This size of this param is controlled by the
				* size, in bytes, of this parameter.
				*/
  int options;                 /* IDL_PARAM_OPTION_XXX */
};/* idl_param */

#endif /* __PCS_IDL_OBJECTS_HPP__ */
