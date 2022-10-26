/*========================marshal_internal.h================================*
 *                                                                          *
 * All sources, binaries and HTML pages (C) copyright 2006 by Blue Jungle., *
 * Redwood City CA, Ownership remains with Blue Jungle Inc,                 * 
 * All rights reserved worldwide.                                           *
 *                                                                          * 
 * Author : Heidi Zhou                                                      *
 * Date   : 12/26/2006                                                      *
 * Note   : Some internal class and data structure of module "marshal"      *
 *==========================================================================*/
#ifndef __CE_MARSHALL_INTERNAL_H
#define __CE_MARSHALL_INTERNAL_H

#include <string>
#include <stdexcept>

namespace MARSHALL {
/*==========================================================================*
 * Class of exception                                                       *
 *==========================================================================*/
class MarshalException : public std::exception
{
  std::string msg;
 public:
  virtual const char *what() const throw() { return msg.c_str(); }

  MarshalException(const char *eMsg):msg(eMsg) {}
  virtual ~MarshalException() throw() {}
};
/*==========================================================================*
 * Class of MarshalHelper                                                   *
 *==========================================================================*/
class MarshalHelper
{
 public:
  MarshalHelper();
  ~MarshalHelper(){}
};

/*==========================================================================*
 * CE Data Type ID sent over the socket                                     *
 *==========================================================================*/
enum {
  ID_CEINT32            =0x00000001,
  ID_CEBoolean          =0x00000002,
  ID_CEKeyRoot_t        =0x00000003,
  ID_CEAction_t         =0x00000004,
  ID_CEEnforcement_t    =0x00000005,
  ID_CEPEP_t            =0x00000006,
  ID_CEHandle           =0x00000007,
  ID_CEString           =0x00000040,
  ID_CEAttribute        =0x00000400,
  ID_CEAttributes       =0x00000401,
  ID_CEAttributes_Array =0x00000402
};
/*==========================================================================*
 * CE Data Type Name                                                        *
 *==========================================================================*/
#define NAME_CEint32                "CEint32"
#define NAME_CEBoolean              "CEBoolean"
#define NAME_CEKeyRoot_t            "CEKeyRoot_t"
#define NAME_CEAction_t             "CEAction_t"
#define NAME_CEEnforcement_t        "CEEnforcement_t"
#define NAME_CEPEP_t                "CEPEP_t"
#define NAME_CEHandle               "CEHandle"
#define NAME_CEString               "CEString"
#define NAME_CEAttribute            "CEAttribute"
#define NAME_CEAttributes           "CEAttributes"
#define NAME_CEAttributes_Array     "CEAttributes_Array"
}
#endif /* marshal_internal.h */
