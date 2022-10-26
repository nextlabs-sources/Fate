/*========================marshal.cpp=======================================*
 *                                                                          *
 * All sources, binaries and HTML pages (C) copyright 2006 by Blue Jungle., *
 * Redwood City CA, Ownership remains with Blue Jungle Inc,                 * 
 * All rights reserved worldwide.                                           *
 *                                                                          * 
 * Author : Heidi Zhou                                                      *
 * Date   : 12/26/2006                                                      *
 * Note   : The RPC layer marshalling APIs.                                 *
 *==========================================================================*/
#include <iostream>
#include <cassert>
#include "cetype.h"
#include "marshal.h"
#include "marshal_internal.h"
#include <map>
#include "nlstrings.h"
#include "brain.h"
#include "CESDK_private.h"

#if defined(Linux)
#include "linux_win.h" 
#endif

using namespace MARSHALL;
using namespace std;

/*==========================================================================*
 * Global variables.                                                        *
 *==========================================================================*/

/*==========================================================================*
 * Internal Global variables and functions scoped in this file.             *
 *==========================================================================*/
namespace {
  //Define some constants
  enum {FUNC_NOT_FOUND=-1, UNIT_LEN=4};

  //define 32bit unsigned interger type
  typedef struct _uint32_t {
    unsigned num:32;
  } uint32_t;

  //RPC function signature table
  std::vector<CEMarshalFunc> marshalFuncTbl;

  //RPC function lookup table
  std::map<nlstring,int> funcLookupTbl;

  //Dummy variable to initialize marshalFuncTbl
  MarshalHelper DoInit;  

  //Get the function index in the table "marshalFuncTbl" 
  //Return -1, if the function doesn't exist.
  inline int GetFuncIndex(const nlchar *funcName)
  {
    /* Get the function index in the table "marshalFuncTbl" */
    map<nlstring, int>::iterator it;
    it=funcLookupTbl.find(funcName);
    
    /* The function is not a valid CE RPC function. Return NULL. */ 
    if(it == funcLookupTbl.end()) 
      return FUNC_NOT_FOUND;

    return it->second;
  }


  /*
   * The number of arguments described by this format string
   */
  static int numArguments(const nlchar *fmt)
  {
    int numargs = 0;
    while (*fmt)
    {
      if (*fmt == '[') {
        fmt++;
        if (*(fmt) == 'a') {
          numargs++;
        } else {
          // Error
          return 0;
        }
      }
      else {
        numargs++;
      }
      fmt++;
    }

    return numargs;
  }

  /*
   * Split apart a format string character by character
   */
  static const nlchar *getEncoding(const nlchar **fmt)
  {
    nlchar * ret = NULL;

    if (fmt != NULL)
    {
      switch(**fmt)
      {
        case 'i':
          ret = _T(NAME_CEint32);
          break;
        case 'b':
          ret = _T(NAME_CEBoolean);
          break;
        case 's':
          ret = _T(NAME_CEString);
          break;
        case 'a':
          ret = _T(NAME_CEAttributes);
          break;
        case '[':
          (*fmt)++;
          if (**fmt == 'a') {
            ret = _T(NAME_CEAttributes_Array);
          }
          break;
        default:
          break;
      }

      (*fmt)++;
    }


    return ret;
  }


  //Get the length of one argument length
  size_t GetFuncArgLen(const nlchar *type, void *argPtr)
  {
    size_t length=UNIT_LEN; //The header of data type

    if(nlstrcmp(type, _T(NAME_CEString)) == 0) {
      //add the length of count field
      length+=UNIT_LEN;

      //add the length of byte size field
      length+=UNIT_LEN;

      if(!argPtr) //empty string
        return length;

      //Calculate the lenghth of string and padding bytes
      nlchar *str=((CEString)argPtr)->buf;
      if(str == NULL)
        return length;

      size_t strLen= nlstrblen(str);
      if(strLen%UNIT_LEN != 0)
        strLen+=(UNIT_LEN-(strLen%UNIT_LEN));

      return length+strLen;
    } else if (nlstrcmp(type, _T(NAME_CEAttribute)) == 0) {
      //add the length of count field
      length+=UNIT_LEN;

      //add the length of 2 members of CEAttribute
      CEAttribute *attr=(CEAttribute *)argPtr;
      length+=GetFuncArgLen(_T(NAME_CEString), (void *)(attr->key));
      length+=GetFuncArgLen(_T(NAME_CEString), (void *)(attr->value));
      return length;
    } else if (nlstrcmp(type, _T(NAME_CEAttributes)) == 0) {
      //add the length of count field
      length+=UNIT_LEN;

      if(!argPtr)
        return length;

      //Pack the elements of CEAttributes
      CEAttributes *attrs=(CEAttributes *)argPtr;
      for(int i=0; i < attrs->count; i++)
        length+=GetFuncArgLen(_T(NAME_CEAttribute), 
                              (void *)(&(attrs->attrs[i])));
      return length;
    } else if (nlstrcmp(type, _T(NAME_CEEnforcement_t)) ==0) {
      CEEnforcement_t *enf=(CEEnforcement_t *)argPtr;

      //Get the length of CEEnforcement_t member 'result'
      length+=GetFuncArgLen(_T(NAME_CEint32), &(enf->result));

      //Get the length of CEEnforcement_t member 'obligation'
      if(enf->obligation == NULL) {
        CEAttributes dummy_attrs;
        dummy_attrs.count=0;
        dummy_attrs.attrs=NULL;
        length+=GetFuncArgLen(_T(NAME_CEAttributes), &dummy_attrs);     
      } else 
        length+=GetFuncArgLen(_T(NAME_CEAttributes), enf->obligation);
      return length;
    } else if (nlstrcmp(type, _T(NAME_CEHandle)) == 0) {
      return length+2*UNIT_LEN;
    } else if (nlstrcmp(type, _T(NAME_CEAttributes_Array)) == 0) {
      //Add the length of count field
      length+=UNIT_LEN;
    
      if(!argPtr) return length;

      //Pack the attrs_array of CEAttributes_Array
      CEAttributes_Array *aattrs=(CEAttributes_Array *)argPtr;
      for(int i=0; i<aattrs->count; i++)
        length+=GetFuncArgLen(_T(NAME_CEAttributes), &(aattrs->attrs_array[i]));
      return length;
    } 

    //simple data type whose length is 4 bytes
    //They are CEint32, CEBoolean, CEKeyRoot_t, CEAction_t, CEPEP_t
    return length+UNIT_LEN;
  }

  //Get the length of one array type argument
  //If the type includes "[]", the type is an array type.
  //The data type has to be simple data type which is 4-bytes long;
  //otherwise the funtion fails and returns 0.  
  size_t GetFuncArrayArgLen(const nlchar *atype, int len)
  {
    size_t length=UNIT_LEN*2; //The header of array data type

    if(nlstrstr(atype, _T(NAME_CEint32)) || 
       nlstrstr(atype, _T(NAME_CEBoolean)) || 
       nlstrstr(atype, _T(NAME_CEKeyRoot_t)) ||
       nlstrstr(atype, _T(NAME_CEAction_t)) ||
       nlstrstr(atype, _T(NAME_CEPEP_t)))
      length+=len*UNIT_LEN;
    else
      return 0;

    return length;
  }

  //Get the length of data packet for a request RPC function
  size_t GetReqFuncLength(int funcID, vector<void *> &argv)
  {
    size_t length=UNIT_LEN*2; //The length of header is 8

    /* Sanity check */
    assert( funcID >= 0 && funcID < static_cast<int>(marshalFuncTbl.size()) );
    assert( marshalFuncTbl[funcID].inputArgs.size() == argv.size() );
    if( funcID < 0 || funcID >= static_cast<int>(marshalFuncTbl.size()) ||
        marshalFuncTbl[funcID].inputArgs.size() != argv.size() )
    {
      return 0;
    }

    /*Calculate the length of input arguments*/
    for(unsigned int i=0; i < (marshalFuncTbl[funcID].inputArgs).size(); i++) {
      if(nlstrstr((marshalFuncTbl[funcID].inputArgs[i].c_str()),_T("[]"))) {
        //Array type. The following i+1th element specifies the length
        //of the array. 
        const nlchar *atype=marshalFuncTbl[funcID].inputArgs[i].c_str();
        length+=GetFuncArrayArgLen(atype, *((int *)argv[i]));
      } else 
        length+=GetFuncArgLen(marshalFuncTbl[funcID].inputArgs[i].c_str(),
                              argv[i]);
    }

    return length;  
  }

  //Get the length of data packet for a RPC function reply
  size_t GetFuncReplyLength(int funcID, vector<void *> &argv)
  {
    size_t length=3*UNIT_LEN; //The length of header is 12

    /* Sanity check */
    assert( funcID >= 0 && funcID < static_cast<int>(marshalFuncTbl.size()) );
    assert( marshalFuncTbl[funcID].outputArgs.size() == argv.size() );
    if( funcID < 0 || funcID >= static_cast<int>(marshalFuncTbl.size()) ||
        marshalFuncTbl[funcID].outputArgs.size() != argv.size() )
    {
      return 0;
    }

    /*Calculate the length of output arguments*/
    for(unsigned int i=0; i<marshalFuncTbl[funcID].outputArgs.size(); i++) {
      if(nlstrstr(marshalFuncTbl[funcID].outputArgs[i].c_str(),_T("[]"))) {
        //Array type. The following i+1th element specifies the length
        //of the array. 
        const nlchar *atype=marshalFuncTbl[funcID].outputArgs[i].c_str();
        length+=GetFuncArrayArgLen(atype, *((int *)argv[i]));
      } else 
        length+=GetFuncArgLen(marshalFuncTbl[funcID].outputArgs[i].c_str(),
                              argv[i]);
    }

    return length;  
  }


  static CEString convertToCEString(const nlchar *str)
  {
    CEString cestr = new struct _CEString();
    try {
      cestr->buf = new nlchar[nlstrlen(str)+1];
      cestr->buf[0] = '\0';
      nlstrcpy_s(cestr->buf, nlstrlen(str)+1, str);
      cestr->length=nlstrlen(str);      
    } catch (...) {
      // Avoid memory leak
      delete cestr;
      return NULL;
    }
    return cestr;
  }

  static size_t GetFuncGenericLength(CEString fmt, vector<void *> &argv, bool isReply = false);

  static size_t GetFuncGenericLength(CEString fmt, vector<void *> &argv, bool isReply)
  {
    size_t length = 2 * UNIT_LEN;

    if (isReply) {
      // Extra space for CEResult_t
      length += UNIT_LEN;
    }

    // Space for the format string itself
    length += GetFuncArgLen(_T(NAME_CEString), fmt);

    const nlchar *nlfmt = fmt->buf;

    for (unsigned int i = 0; i < argv.size(); i++) {
      const nlchar *type = getEncoding(&nlfmt);

      if (type == NULL) {
        // Argument length mismatch
        return 0;
      }

      // Oddly, CEAttributes_Array is not an array type, so we don't
      // need to check for it or call GetFuncArrayArgLen
      length+=GetFuncArgLen(type, argv[i]);
    }

    return length;
  }

  //UnPack the request function header. 
  //Return the function ID.  
  CEResult_t UnPackReqFuncHeader(const char *request, int &funcID)
  {
    uint32_t unit;
    int i=0;
    unsigned int numArg=0;

    //Get the software version  
    memcpy(&unit, request, UNIT_LEN);
    if((0x0000FFFF & unit.num) ^ CESDK_VERSION_MAJOR)
      return CE_RESULT_VERSION_MISMATCH;

    //Get the function ID
    for(i=0; i< static_cast<int>(marshalFuncTbl.size()); i++) {
      if(!((0xFFFF0000 & unit.num) ^ marshalFuncTbl[i].requestID)) {
        funcID=i;
        break;
      }
    }  
    if(i== static_cast<int>(marshalFuncTbl.size()))
      return CE_RESULT_FUNCTION_NOT_AVAILBLE;

    //Get the unmber of function input arguments
    //Check if the number of argument match with function signature
    memcpy(&numArg, request+UNIT_LEN, UNIT_LEN);
    if(numArg != marshalFuncTbl[funcID].inputArgs.size())
      return CE_RESULT_INVALID_PARAMS; 
  
    return CE_RESULT_SUCCESS;
  }

  //Pack the request function header into the buffer "dst"
  //Return the start position where the following data will be 
  //packed into. 
  char *PackReqFuncHeader(int funcID, char *dst)
  {
    char *cur=dst;
    uint32_t unit;
    int numArg;

    //Pack software version and function id into data packet
    unit.num = 0x0000FFFF & CESDK_VERSION_MAJOR;
    unit.num = unit.num | marshalFuncTbl[funcID].requestID;
    memcpy(cur, &unit, UNIT_LEN);
    cur=cur+UNIT_LEN;

    //Pack the number of input arguments into data packet
    numArg =  (int)(marshalFuncTbl[funcID].inputArgs.size()); // cast to int is necessary because on x64, size_t is 64 bit and int is 32 bit.  I will use int here because UNIT_LEN is 4 (4 byte = 32 bit).  I assume this value does not exceed 32 bit max integer value.
    memcpy(cur, &numArg,UNIT_LEN);
    cur=cur+UNIT_LEN;

    return cur;
  }

  //Pack reply function header into the buffer "dst". 
  //Return the start position where the following data will be 
  //packed into. 
  char *PackFuncReplyHeader(int funcID, CEResult_t rst, char *dst)
  {
    char *cur=dst;
    uint32_t unit;
    int argNum;

    //Pack software version and function id into data packet
    unit.num = 0x0000FFFF & CESDK_VERSION_MAJOR;
    unit.num = unit.num | marshalFuncTbl[funcID].replyID;
    memcpy(cur, &unit, UNIT_LEN);
    cur=cur+UNIT_LEN;

    //Pack the number of output arguments into data packet
    argNum= (int)(marshalFuncTbl[funcID].outputArgs.size()); // cast to int is necessary because on x64, size_t is 64 bit and int is 32 bit.  I will use int here because UNIT_LEN is 4 (4 byte = 32 bit).  I assume this value does not exceed 32 bit max integer value.
    memcpy(cur, &argNum, UNIT_LEN);
    cur=cur+UNIT_LEN;
  
    //Pack the return code into data packet
    memset(cur, 0, UNIT_LEN);
    memcpy(cur, &rst, sizeof(CEResult_t));
    cur=cur+UNIT_LEN;
  
    return cur;
  }

  //UnPack the function reply header. 
  //Return the function ID, number of return argument, and returned code  
  CEResult_t UnPackFuncReplyHeader(const char *reply, int &funcID, 
                                   CEResult_t &result)
  {
    uint32_t unit;
    int i=0;
    unsigned int numArg;

    //Get the software version  
    memcpy(&unit, reply, UNIT_LEN);
    if((0x0000FFFF & unit.num) ^ CESDK_VERSION_MAJOR)
      return CE_RESULT_VERSION_MISMATCH;

    //Get the function ID
    for(i=0; i<static_cast<int>(marshalFuncTbl.size()); i++) {
      if(!((0xFFFF0000 & unit.num) ^ marshalFuncTbl[i].replyID)) {
        funcID=i;
        break;
      }
    }  
    if(i==static_cast<int>(marshalFuncTbl.size()))
      return CE_RESULT_FUNCTION_NOT_AVAILBLE;

    //Get the unmber of function input arguments
    memcpy(&numArg, reply+UNIT_LEN, UNIT_LEN);
    if(numArg != marshalFuncTbl[i].outputArgs.size())
      return CE_RESULT_INVALID_PARAMS;
  
    //Get the returned code
    CEResult_t tmp_result;
    memcpy_s(&tmp_result, sizeof tmp_result, reply+2*UNIT_LEN,
             sizeof(CEResult_t));

    result = tmp_result;
    return CE_RESULT_SUCCESS;
  }

  //Pack one function argument into the buffer "dst".  
  char *PackFuncArgument(const nlchar *type, void *argPtr, char *dst)
  {
    char *cur=dst;
    uint32_t tmpInt={ID_CEBoolean};
    int numArg;

    if(nlstrcmp(type, _T(NAME_CEString)) == 0) {
      //Pack data type
      tmpInt.num = ID_CEString;
      memcpy(cur, &tmpInt, UNIT_LEN);
      cur=cur+UNIT_LEN;
    
      if(argPtr && ((CEString)argPtr)->length != 0) {
        nlchar *str=((CEString)argPtr)->buf;
        size_t len_in_byte = nlstrblen(str);

        //Pack length of string
        memcpy(cur, &(((CEString)argPtr)->length), UNIT_LEN);
        cur=cur+UNIT_LEN;

        //Pack byte length of string
        memcpy(cur, &len_in_byte, UNIT_LEN);
        cur=cur+UNIT_LEN;

        //Pack string
        memcpy(cur, str, len_in_byte);
        cur+=len_in_byte;

        //Pack padding bytes
        if(len_in_byte%4 != 0) {
          len_in_byte=4-(len_in_byte%4);
          memset(cur, 0, len_in_byte);
          cur+=len_in_byte;
        }  
      } else {
        //Empty string
        //Pack length of string
        memset(cur, 0, UNIT_LEN);
        cur=cur+UNIT_LEN;

        //Pack byte length of string
        memset(cur, 0, UNIT_LEN);
        cur=cur+UNIT_LEN;
      }
    } else if (nlstrcmp(type, _T(NAME_CEAttribute)) == 0) {
      //Pack data type
      tmpInt.num = ID_CEAttribute;
      memcpy(cur, &tmpInt, UNIT_LEN);
      cur=cur+UNIT_LEN;

      //Pack number (2) of members of CEAttribute
      numArg=2;
      memcpy(cur, &numArg, UNIT_LEN);
      cur+=UNIT_LEN;

      //Pack 2 members of CEAttribute
      CEAttribute *attr=(CEAttribute *)argPtr;
      cur=PackFuncArgument(_T(NAME_CEString), (void *)(attr->key), cur);
      cur=PackFuncArgument(_T(NAME_CEString), (void *)(attr->value), cur);
    } else if (nlstrcmp(type, _T(NAME_CEAttributes_Array)) == 0) {
      //pack data type
      tmpInt.num = ID_CEAttributes_Array;
      memcpy(cur, &tmpInt, UNIT_LEN);
      cur+=UNIT_LEN;

      if(argPtr) {
        CEAttributes_Array *aattrs=(CEAttributes_Array *)argPtr;

        //Pack the number of elements of CEAttributes_Array
        numArg=aattrs->count;
        memcpy(cur, &numArg, UNIT_LEN);
        cur=cur+UNIT_LEN;

        //pack the elements of CEAttributes_Array
        for(int i=0; i<aattrs->count; i++) 
          cur=PackFuncArgument(_T(NAME_CEAttributes), 
                               (void *)(&(aattrs->attrs_array[i])), cur);
      }
    } else if (nlstrcmp(type, _T(NAME_CEAttributes)) == 0) {
      //Pack data type
      tmpInt.num = ID_CEAttributes;
      memcpy(cur, &tmpInt, UNIT_LEN);
      cur=cur+UNIT_LEN;

      if(argPtr) {
        CEAttributes *attrs=(CEAttributes *)argPtr;

        //Pack the number of elements of CEAttributes
        numArg=attrs->count;
        memcpy(cur, &numArg, UNIT_LEN);
        cur=cur+UNIT_LEN;

        //Pack the elements of CEAttributes
        for(int i=0; i < attrs->count; i++)
          cur=PackFuncArgument(_T(NAME_CEAttribute), 
                               (void *)(&(attrs->attrs[i])), cur);
      } else {
        //Empty Attributes
        //Pack the number of elements of CEAttributes
        numArg=0;
        memcpy(cur, &numArg, UNIT_LEN);
        cur=cur+UNIT_LEN;      
      }
    } else if(nlstrcmp(type, _T(NAME_CEEnforcement_t))==0) {
      //Pack data type
      tmpInt.num = ID_CEEnforcement_t;
      memcpy(cur, &tmpInt, UNIT_LEN);
      cur=cur+UNIT_LEN;

      CEEnforcement_t *enf=(CEEnforcement_t *)argPtr;

      //Pack the CEEnforcement member 'result'
      cur=PackFuncArgument(_T(NAME_CEint32), &(enf->result), cur);

      //Pack the CEEnforcement member 'obligation'
      if(enf->obligation == NULL) {
        CEAttributes dummy_attrs;
        dummy_attrs.count = 0;
        dummy_attrs.attrs = NULL;
        cur=PackFuncArgument(_T(NAME_CEAttributes), &dummy_attrs, cur);      
      } else 
        cur=PackFuncArgument(_T(NAME_CEAttributes), enf->obligation, cur);
    }  else if(nlstrcmp(type, _T(NAME_CEHandle))==0) {
      //Pack data Type
      tmpInt.num = ID_CEHandle;
      memcpy(cur, &tmpInt, UNIT_LEN);
      cur=cur+UNIT_LEN;
      memcpy(cur, (unsigned long long *)argPtr, 2*UNIT_LEN); // always copy 64 bits
      cur=cur+2*UNIT_LEN;
    } else { //simple data type whose length is 4 bytes
      //Pack data Type
      if(nlstrcmp(type, _T(NAME_CEint32))==0) 
        tmpInt.num = ID_CEINT32;
      else if(nlstrcmp(type, _T(NAME_CEBoolean))==0)
        tmpInt.num = ID_CEBoolean;
      else if(nlstrcmp(type, _T(NAME_CEKeyRoot_t))==0)
        tmpInt.num = ID_CEKeyRoot_t;
      else if(nlstrcmp(type, _T(NAME_CEAction_t))==0)
        tmpInt.num = ID_CEAction_t;
      else if(nlstrcmp(type, _T(NAME_CEPEP_t))==0)
        tmpInt.num = ID_CEPEP_t;
      memcpy(cur, &tmpInt, UNIT_LEN);
      cur=cur+UNIT_LEN;

      //Pack data value
      memset(cur, 0, UNIT_LEN);
      if(nlstrcmp(type, _T(NAME_CEint32))==0) 
        memcpy(cur, argPtr, sizeof(CEint32));
      else if(nlstrcmp(type, _T(NAME_CEBoolean))==0)
        memcpy(cur, argPtr, sizeof(CEBoolean));
      else if(nlstrcmp(type, _T(NAME_CEKeyRoot_t))==0)
        memcpy(cur, argPtr, sizeof(CEKeyRoot_t));
      else if(nlstrcmp(type, _T(NAME_CEAction_t))==0)
        memcpy(cur, argPtr, sizeof(CEAction_t));
      else if(nlstrcmp(type, _T(NAME_CEPEP_t))==0)
        memcpy(cur, argPtr, sizeof(CEPEP_t));
      cur=cur+UNIT_LEN;
    }

    return cur;
  }

  //UnPack one function argument from stream "str" into "arg".  
  char *UnPackFuncArgument(const nlchar *type, char *str, void **arg, 
                           bool &bUnpackSuccess, bool bNewAllocation=true )
  {
    uint32_t tmpInt;
    memcpy(&tmpInt, str, UNIT_LEN);
    *arg=NULL;

    bUnpackSuccess=true;

    if(nlstrcmp(type, _T(NAME_CEString)) == 0 && tmpInt.num==ID_CEString) {
      //UnPack string length
      int len;
      memcpy(&len, str+UNIT_LEN, UNIT_LEN);

      //UnPack string byte length
      int len_in_byte;
      memcpy(&len_in_byte, str+2*UNIT_LEN, UNIT_LEN);

      //Assign
      if(len < 0 || len_in_byte < 0) {
        bUnpackSuccess=false; 
        return str+3*UNIT_LEN;
      } else if(len == 0 || len_in_byte==0) {
        *arg = NULL;
        return str+3*UNIT_LEN;
      } else {
        //That is (len > 0 && len_in_byte > 0) 
        CEString ces=new struct _CEString();
        ces->buf = new nlchar[len_in_byte+1];
        ces->buf[0]='\0';
        nlstrplatformcpy(ces->buf, (nlchar *)(str+3*UNIT_LEN), len,
                         len_in_byte, len_in_byte+1);
        ces->length=len;      
        *arg = (void *)ces;

        //Skip padding bytes
        //int len_in_byte=nlstrblen(ces->buf);
        if(len_in_byte%UNIT_LEN != 0) {
          len_in_byte+=(UNIT_LEN-(len_in_byte%UNIT_LEN));
        } 
      }
      return str+3*UNIT_LEN+len_in_byte;
    } else if (nlstrcmp(type, _T(NAME_CEAttribute)) == 0 && 
               tmpInt.num==ID_CEAttribute) {
      CEString key;
      CEString value;

      str=UnPackFuncArgument(_T(NAME_CEString), 
                             str+UNIT_LEN*2, 
                             (void **)(&key), bUnpackSuccess);
      if(!bUnpackSuccess)
        return str;
      str=UnPackFuncArgument(_T(NAME_CEString), str, 
                             (void **)(&value), bUnpackSuccess);
      if(!bUnpackSuccess) {
        delete key;
        return str;
      }

      CEAttribute *aptr=new CEAttribute;
      aptr->key = key;
      aptr->value = value;
      *arg = aptr;    

      return str;
    } else if (nlstrcmp(type, _T(NAME_CEAttributes_Array)) == 0 && 
               tmpInt.num==ID_CEAttributes_Array ) {
      //UnPack the number of elements of CEAttributes_Array
      int len;
      memcpy(&len, str+UNIT_LEN, UNIT_LEN);
      str+=UNIT_LEN*2;
    
      if(len == 0) {
        *arg = NULL;
        return str;
      }

      //Assign
      CEAttributes_Array *aattrs= new CEAttributes_Array;
      (*arg)=(void *)aattrs;
      aattrs->count=len;
      if(len > 0)
        aattrs->attrs_array=new CEAttributes [len];
      else
        aattrs->attrs_array=NULL;

      //UnPack the CEAttributes elements of CEAttributes_Array
      for(int i=0; i < len; i++) {      
        str=UnPackFuncArgument(_T(NAME_CEAttributes), str,
                               (void **)(&(aattrs->attrs_array[i])), 
                               bUnpackSuccess, false);
      }
      return str;    
    } else if (nlstrcmp(type, _T(NAME_CEAttributes)) == 0 && 
               tmpInt.num==ID_CEAttributes ) {
      //UnPack the number of elements of CEAttributes
      int len;
      memcpy(&len, str+UNIT_LEN, UNIT_LEN);
      str+=UNIT_LEN*2;
    
      if(len == 0) {
        if(bNewAllocation) 
          *arg=NULL;
        else {
          CEAttributes *tmp_attr=( CEAttributes *)(arg);
          tmp_attr->count=0;
          tmp_attr->attrs=NULL;
        } 
        return str;
      }

      //Assign
      CEAttributes *attr=NULL;
      if(bNewAllocation) {
        attr= new CEAttributes;
        (*arg)=(void *)attr;
      } else 
        attr=(CEAttributes *)arg;
      attr->count=len;
      if(len > 0)
        attr->attrs=new CEAttribute [len];
      else
        attr->attrs=NULL;

      //UnPack the CEAttribute elements of CEAttributes
      for(int i=0; i < len; i++) {
        CEString key;
        CEString value;
      
        //Skip CEAttribute header
        str+=UNIT_LEN*2;

        str=UnPackFuncArgument(_T(NAME_CEString), str, 
                               (void **)(&key), bUnpackSuccess);
        if(!bUnpackSuccess) {
          for(int j=0; j<i; j++) {
            delete (attr->attrs[j]).key;
            delete (attr->attrs[j]).value;
          }
          delete [] attr->attrs;
          delete attr;
          (*arg)=NULL;
          return str;
        }
        str=UnPackFuncArgument(_T(NAME_CEString), str, 
                               (void **)(&value), bUnpackSuccess);
        if(!bUnpackSuccess) {
          for(int j=0; j<i; j++) {
            delete (attr->attrs[j]).key;
            delete (attr->attrs[j]).value;
          }
          delete [] attr->attrs;
          delete attr;
          (*arg)=NULL;
          return str;
        }
        (attr->attrs[i]).key=key;
        (attr->attrs[i]).value=value;
      }
      return str;
    } else if(nlstrcmp(type, _T(NAME_CEEnforcement_t))==0 && 
              tmpInt.num == ID_CEEnforcement_t) {    
      CEEnforcement_t *p = new CEEnforcement_t;

      //skip the header
      str+=UNIT_LEN;

      //Unpack CEEnforcement_t member 'result'
      str+=UNIT_LEN; //Skip CEint32 header
      memcpy(&p->result, str, sizeof(CEint32));
      str+=UNIT_LEN;

      //Unpack CEEnforcement_t member 'obligation'
      str=UnPackFuncArgument(_T(NAME_CEAttributes), str, 
                             (void **)(&(p->obligation)), bUnpackSuccess);
      if(p->obligation && p->obligation->count == 0) {
        //obligation is empty. Thus, we should free the member 'obligation'
        //and assign NULL to it
        delete p->obligation;
        p->obligation=NULL;
      }

      //Return 
      *arg = p;
      return str;
    } else if(nlstrcmp(type, _T(NAME_CEHandle))==0 && tmpInt.num == ID_CEHandle) {
      unsigned long long *p = new unsigned long long;
      memcpy(p, str+UNIT_LEN, 2*UNIT_LEN); //64bit
      *arg = p;
      return  str+3*UNIT_LEN;
    } else { //simple data type whose length is 4 bytes
      //Allocate memory
      if(nlstrcmp(type, _T(NAME_CEint32))==0 && tmpInt.num==ID_CEINT32) {
        CEint32 *p = new CEint32; 
        memcpy(p, str+UNIT_LEN, sizeof(CEint32));
        *arg = p;
      }else if(nlstrcmp(type, _T(NAME_CEBoolean))==0 && 
               tmpInt.num == ID_CEBoolean) {
        CEBoolean *p = new CEBoolean;
        memcpy(p, str+UNIT_LEN, sizeof(CEBoolean));
        *arg = p;
      }else if(nlstrcmp(type,_T(NAME_CEKeyRoot_t))==0 && 
               tmpInt.num == ID_CEKeyRoot_t) {
        CEKeyRoot_t *p= new CEKeyRoot_t;
        memcpy(p, str+UNIT_LEN, sizeof(CEKeyRoot_t));
        *arg = p;
      }else if(nlstrcmp(type, _T(NAME_CEAction_t))==0 && 
               tmpInt.num == ID_CEAction_t) {
        CEAction_t *p = new CEAction_t;
        memcpy(p, str+UNIT_LEN, sizeof(CEAction_t));
        *arg = p;
      } else if(nlstrcmp(type, _T(NAME_CEPEP_t))==0 && 
                tmpInt.num == ID_CEPEP_t) {
        CEPEP_t *p = new CEPEP_t;
        memcpy(p, str+UNIT_LEN, sizeof(CEPEP_t));
        *arg = p;
      } else {
        //unknown type; unpacking failed
        bUnpackSuccess=false; 
        return str;
      }
      return str+2*UNIT_LEN;
    }
  }

  //Pack one array type function argument into the buffer "dst". 
  //If the type includes "[]", the type is an array type.
  //The data type has to be simple data type which is 4-bytes long;
  //otherwise the funtion fails and returns NULL.  
  char *PackFuncArrayArgument(const nlchar *atype, int len, 
                              void *argPtr, char *dst)
  {
    char *cur=dst;
    uint32_t tmpInt;

    if(nlstrstr(atype, _T(NAME_CEint32))) {
      //Pack data type
      tmpInt.num = ID_CEString;
      memcpy(cur, &tmpInt, UNIT_LEN);
      cur=cur+UNIT_LEN;

      //Pack array length
      memcpy(cur, &len, UNIT_LEN);
      cur+=UNIT_LEN;

      //Pack elements
      CEint32 *v=(CEint32 *)argPtr;
      for(int i=0; i<len; i++) {
        memset(cur, 0, UNIT_LEN);
        memcpy(cur, &v[i], sizeof(CEint32));
        cur=cur+UNIT_LEN;
      }
    } else if(nlstrstr(atype, _T(NAME_CEBoolean))) {
      //Pack data type
      tmpInt.num = ID_CEBoolean;
      memcpy(cur, &tmpInt, UNIT_LEN);
      cur=cur+UNIT_LEN;

      //Pack array length
      memcpy(cur, &len, UNIT_LEN);
      cur+=UNIT_LEN;

      //Pack elements
      CEBoolean *v=(CEBoolean *)argPtr;
      for(int i=0; i<len; i++) {
        memset(cur, 0, UNIT_LEN);
        memcpy(cur, &v[i], sizeof(CEBoolean));
        cur=cur+UNIT_LEN;
      }
    } else if(nlstrstr(atype, _T(NAME_CEKeyRoot_t))) {
      //Pack data type
      tmpInt.num = ID_CEKeyRoot_t;
      memcpy(cur, &tmpInt, UNIT_LEN);
      cur=cur+UNIT_LEN;

      //Pack array length
      memcpy(cur, &len, UNIT_LEN);
      cur+=UNIT_LEN;

      //Pack elements
      CEKeyRoot_t *v=(CEKeyRoot_t *)argPtr;
      for(int i=0; i<len; i++) {
        memset(cur, 0, UNIT_LEN);
        memcpy(cur, &v[i], sizeof(CEKeyRoot_t));
        cur=cur+UNIT_LEN;
      }
    } else if(nlstrstr(atype, _T(NAME_CEAction_t))) {
      //Pack data type
      tmpInt.num = ID_CEAction_t;
      memcpy(cur, &tmpInt, UNIT_LEN);
      cur=cur+UNIT_LEN;

      //Pack array length
      memcpy(cur, &len, UNIT_LEN);
      cur+=UNIT_LEN;

      //Pack elements
      CEAction_t *v=(CEAction_t *)argPtr;
      for(int i=0; i<len; i++) {
        memset(cur, 0, UNIT_LEN);
        memcpy(cur, &v[i], sizeof(CEAction_t));
        cur=cur+UNIT_LEN;
      }
    } else if(nlstrstr(atype, _T(NAME_CEPEP_t))) {
      //Pack data type
      tmpInt.num = ID_CEPEP_t;
      memcpy(cur, &tmpInt, UNIT_LEN);
      cur=cur+UNIT_LEN;

      //Pack array length
      memcpy(cur, &len, UNIT_LEN);
      cur+=UNIT_LEN;

      //Pack elements
      CEPEP_t *v=(CEPEP_t *)argPtr;
      for(int i=0; i<len; i++) {
        memset(cur, 0, UNIT_LEN);
        memcpy(cur, &v[i], sizeof(CEPEP_t));
        cur=cur+UNIT_LEN;
      }
    } else
      return NULL;
    return cur;
  }

  //UnPack one array type function argument from stream "str" 
  //into "arg"
  char *UnPackFuncArrayArgument(const nlchar *atype, char *str, 
                                int &len, void **arg)
  {
    uint32_t tmpInt;
    memcpy(&tmpInt, str, UNIT_LEN);
    *arg=NULL;

    //UnPack array length
    memcpy(&len, str+UNIT_LEN, UNIT_LEN);
    str+=2*UNIT_LEN; //The length of header

    if(nlstrstr(atype, _T(NAME_CEint32)) && tmpInt.num==ID_CEINT32)  {
      //UnPack elements
      *arg = (void *)(new CEint32[len]);
      CEint32 *v=(CEint32 *)(*arg);

      for(int i=0; i<len; i++) {
        memcpy((void *)(&v[i]), (void *)str, sizeof(CEint32));
        str+=UNIT_LEN;
      }
    } else if(nlstrstr(atype, _T(NAME_CEBoolean)) && tmpInt.num==ID_CEBoolean) {
      //UnPack elements
      *arg = (void *)(new CEBoolean[len]);
      CEBoolean *v=(CEBoolean *)(*arg);

      for(int i=0; i<len; i++) {
        memcpy((void *)(&v[i]), (void *)str, sizeof(CEBoolean));
        str+=UNIT_LEN;
      }
    } else if(nlstrstr(atype, _T(NAME_CEKeyRoot_t)) && 
              tmpInt.num==ID_CEKeyRoot_t) {
      //UnPack elements
      *arg = (void *)(new CEKeyRoot_t[len]);
      CEKeyRoot_t *v=(CEKeyRoot_t *)(*arg);
 
      for(int i=0; i<len; i++) {
        memcpy((void *)(&v[i]), (void *)str, sizeof(CEKeyRoot_t));
        str+=UNIT_LEN;
      }
    } else if(nlstrstr(atype, _T(NAME_CEAction_t)) && 
              tmpInt.num==ID_CEAction_t) {
      //UnPack elements
      *arg = (void *)(new CEAction_t[len]);
      CEAction_t *v=(CEAction_t *)(*arg);

      for(int i=0; i<len; i++) {
        memcpy((void *)(&v[i]), (void *)str, sizeof(CEAction_t));
        str+=UNIT_LEN;
      }
    } else if(nlstrstr(atype, _T(NAME_CEPEP_t)) && tmpInt.num==ID_CEPEP_t) {
      //UnPack elements
      *arg = (void *)(new CEPEP_t[len]);
      CEPEP_t *v=(CEPEP_t *)(*arg);

      for(int i=0; i<len; i++) {
        memcpy((void *)(&v[i]), (void *)str, sizeof(CEPEP_t));
        str+=UNIT_LEN;
      }
    } 
    return str;
  }

  CEResult_t FreeGenericExplicitFormat(const nlchar *fmt, vector<void *> &argv, int startIndex)
  {
    const nlchar * enc;

    try
    {
      while ((enc = getEncoding(&fmt)) != NULL)
      {
        if (nlstrcmp(enc, _T(NAME_CEint32)) == 0) {
          delete (CEint32 *)argv[startIndex];
        } else if (nlstrcmp(enc, _T(NAME_CEBoolean)) == 0) {
          delete (CEBoolean *)argv[startIndex];
        } else if (nlstrcmp(enc, _T(NAME_CEString)) == 0) {
          delete (CEString)argv[startIndex];
        } else if (nlstrcmp(enc, _T(NAME_CEAttributes)) == 0) {
          CEAttributes *attrs = (CEAttributes *)argv[startIndex];
          if (attrs != NULL) {
            for (int i = 0; i < attrs->count; i++) {
              delete attrs->attrs[i].key;
              delete attrs->attrs[i].value;
            }
            delete [] attrs->attrs;
            delete attrs;
          }
        } else if (nlstrcmp(enc, _T(NAME_CEAttributes_Array)) == 0) {
          CEAttributes_Array *arr = (CEAttributes_Array *)argv[startIndex];
          if (arr != NULL) {
            for (int i = 0; i < arr->count; i++) {
              CEAttributes *attrs = &arr->attrs_array[i];
              if (attrs != NULL) {
                for (int j = 0; j < attrs->count; j++) {
                  delete attrs->attrs[j].key;
                  delete attrs->attrs[j].value;
                }
                delete [] attrs->attrs;
              }
            }
            delete [] arr->attrs_array;
            delete arr;
          }
        }
        argv[startIndex++] = NULL;
      }

      return CE_RESULT_SUCCESS;
    } catch (MarshalException &) {
      return CE_RESULT_GENERAL_FAILED;
    } catch (exception &) {
      return CE_RESULT_GENERAL_FAILED;
    } catch (...) {
      return CE_RESULT_GENERAL_FAILED;
    }
  }
}

//Constructor of CEMarshalFunc structure
CEMarshalFunc::CEMarshalFunc(const nlchar *n, const unsigned q, 
                             int in, const nlchar **inArgs,
                             const unsigned p, 
                             int on, const nlchar **outArgs): 
  funcName(n), requestID(q), replyID(p) 
{
  for(int i=0; i<in; i++)
    inputArgs.push_back(inArgs[i]);

  for(int i=0; i<on; i++)
    outputArgs.push_back(outArgs[i]);
}

//Constructor of class MarshalHelper 
MarshalHelper::MarshalHelper()
{

  //Initialize marshFuncTbl
  //Layout of the input argument type vector:
  //1. Every input arguments array includes the string in the format of
  //   "<thread-id>+<request-time>" as the first input argument, called
  //   "reqID". This argument is the identifier of  a request from a client
  //   at a certain time. 
  //2. After "reqID", it is the list of public arguments that match  
  //   SDK API's signature
  //3. After the public arguments' list, it is the list of private arguments
  //   that need to be sent over the socket and used by client/server stub
  //   code
  //Layout of the output argument type vector:
  //1. Every output arguments array include the string in the format of
  //   "<thread-id>+<request-time>" as the first output argument, called
  //   "reqID". This argument is the identifier of a request from a client
  //   at a certain time. 
  //2. After "reqID", it is the list of public arguments that match SDK
  //   API's signature.
  //3. After the public arguments' list, it is the list of private arguments
  //   that need to be sent over the socket and used by pep and pdp
  //   code

  //SDK API: CECONN_Initialize
  const nlchar *i0[7]={_T(NAME_CEString),  //reqID
                       _T(NAME_CEString),  //public: type
                       _T(NAME_CEString),  //public: appName
                       _T(NAME_CEString),  //public: binaryPath
                       _T(NAME_CEString),  //public: userName
                       _T(NAME_CEString),  //public: userID
                       _T(NAME_CEint32)};  //private: pep local host ip address
  const nlchar *o0[2]={_T(NAME_CEString),  //reqID
                       _T(NAME_CEHandle)};  //public: sessionID on server side
  CEMarshalFunc f0(_T("CECONN_Initialize"),
                   FUNCID_CONN_INITIALIZE_Q,7, i0,
                   FUNCID_CONN_INITIALIZE_P,2,o0);
  marshalFuncTbl.push_back(f0);  
  
  //SDK API: CECONN_Close
  const nlchar *i1[2]={_T(NAME_CEString), //reqID
                       _T(NAME_CEHandle)}; //public: sessionID on server side
  const nlchar *o1[1]={_T(NAME_CEString)};//reqID
  CEMarshalFunc f1(_T("CECONN_Close"),
                   FUNCID_CONN_CLOSE_Q,2, i1,
                   FUNCID_CONN_CLOSE_P,1,o1);
  marshalFuncTbl.push_back(f1);  

  //SDK API: CEPROTECT_LockKey
  const nlchar *i2[4]={_T(NAME_CEString),  //reqID
                       _T(NAME_CEHandle),  //session id on pdp side
                       _T(NAME_CEKeyRoot_t),   //public: CEKeyRoot_t 
                       _T(NAME_CEString)}; //public: key
  const nlchar *o2[1]={_T(NAME_CEString)}; //reqID
  CEMarshalFunc f2(_T("CEPROTECT_LockKey"), 
                   FUNCID_PROTECT_LOCKKEY_Q, 4, i2,
                   FUNCID_PROTECT_LOCKKEY_P, 1, o2);
  marshalFuncTbl.push_back(f2);

  //SDK API: CEPROTECT_UnlockKey
  const nlchar *i3[4]={_T(NAME_CEString),  //reqID
                       _T(NAME_CEHandle),  //session id on pdp side
                       _T(NAME_CEKeyRoot_t),   //public: CEKeyRoot_t
                       _T(NAME_CEString)}; //public: key
  const nlchar *o3[1]={_T(NAME_CEString)}; //reqID
  CEMarshalFunc f3(_T("CEPROTECT_UnlockKey"), 
                   FUNCID_PROTECT_UNLOCKKEY_Q, 4, i3,
                   FUNCID_PROTECT_UNLOCKKEY_P, 1, o3);
  marshalFuncTbl.push_back(f3);  

  const nlchar *i4[4]={_T("CEString"), _T("CEString"),
                       _T("CEString"), _T("CEint32")};
  const nlchar *o4[1]={_T("CEString")};
  CEMarshalFunc f4(_T("CEPROTECT_ProtectFile"),0x000A0000,4, i4,
                   0x000B0000, 1, o4);
  marshalFuncTbl.push_back(f4);  

  const nlchar *i5[3]={_T("CEString"), _T("CEString"),_T("CEString")};
  const nlchar *o5[1]={_T("CEString")};
  CEMarshalFunc f5(_T("CEPROTECT_UnprotectFile"),0x000C0000,3,
                   i5,0x000D0000,1,o5);
  marshalFuncTbl.push_back(f5);  

  const nlchar *i6[4]={_T("CEString"), _T("CEString"), _T("CEint32"), 
                       _T("CEint32")};
  const nlchar *o6[1]={_T("CEString")};
  CEMarshalFunc f6(_T("CEPROTECT_ProtectProcess"),0x000E0000,4,
                   i6,0x000F0000,1,o6);
  marshalFuncTbl.push_back(f6);  

  const nlchar *i7[3]={_T("CEString"), _T("CEString"), _T("CEint32")};
  const nlchar *o7[1]={_T("CEString")};
  CEMarshalFunc f7(_T("CEPROTECT_UnprotectProcess"),0x00100000,3,i7,
                   0x00110000,1,o7);
  marshalFuncTbl.push_back(f7);  

  //SDK API: CEEVALUATE_CheckMetadata
  const nlchar *i8[10]={_T(NAME_CEString), //reqID
                        _T(NAME_CEHandle), //public: session id
                        _T(NAME_CEAttributes), //public: dimensions
                        _T(NAME_CEAttributes_Array), //public: attributeMatrix
                        _T(NAME_CEint32),      //public: noiseLevel
                        _T(NAME_CEBoolean)};  //public: performObligation
  const nlchar *o8[2]={_T(NAME_CEString), //reqID
                       _T(NAME_CEEnforcement_t)}; //public: enforcement
  CEMarshalFunc f8(_T("CEEVALUATE_CheckMetadata"),
                   FUNCID_EVALUATE_CHECKMETADATA_Q,6, i8,
                   FUNCID_EVALUATE_CHECKMETADATA_P,2,o8);
  marshalFuncTbl.push_back(f8);  

  const nlchar *i9[2]={_T("CEString"), _T("CEString")};
  const nlchar *o9[2]={_T("CEString"), _T("CEPEP_t")};
  CEMarshalFunc f9(_T("CECONTEXT_GetSystemPEP"),0x00140000,
                   2,i9,0x00150000,1,o9);
  marshalFuncTbl.push_back(f9);  

  const nlchar *i10[4]={_T("CEString"), _T("CEString"), _T("CEPEP_t"), 
                        _T("CEin32t")};
  const nlchar *o10[1]={_T("CEString")};
  CEMarshalFunc f10(_T("CECONTEXT_IgnorePEP"),0x00160000,4,
                    i10,0x00170000,1,o10);
  marshalFuncTbl.push_back(f10);  

  const nlchar *i11[3]={_T("CEString"), _T("CEString"), _T("CEString")};
  const nlchar *o11[2]={_T("CEString"), _T("CEAttributes")};
  CEMarshalFunc f11(_T("CECONTEXT_GetFileAttributes"),0x00180000,3,
                    i11,0x00190000,2,o11);
  marshalFuncTbl.push_back(f11);  

  //SDK API: CELOGGING_SetNoiseLevel
  const nlchar *i12[3]={_T(NAME_CEString), //reqID
                        _T(NAME_CEHandle), //public: session id
                        _T(NAME_CEint32)}; //public: noise level
  const nlchar *o12[1]={_T(NAME_CEString)}; //reqID
  CEMarshalFunc f12(_T("CELOGGING_SetNoiseLevel"),
                    FUNCID_LOGGING_SETNOISELEVEL_Q,3, i12,
                    FUNCID_LOGGING_SETNOISELEVEL_P,1,o12);
  marshalFuncTbl.push_back(f12);  

  const nlchar *i13[2]={_T("CEString"), _T("CEString")};
  const nlchar *o13[2]={_T("CEString"), _T("CEint32")};
  CEMarshalFunc f13(_T("CELOGGING_GetNoiseLevel"),0x001C0000,2,
                    i13,0x001D0000,2,o13);
  marshalFuncTbl.push_back(f13);  

  //SDK API: CEP_StopPDP
  const nlchar *i14[3]={_T(NAME_CEString), //reqID
                        _T(NAME_CEHandle), //public: session id
                        _T(NAME_CEString)};  //public: password
  const nlchar *o14[1]={_T(NAME_CEString)}; //reqID
  CEMarshalFunc f14(_T("CEP_StopPDP"),
                    FUNCID_CEP_STOPPDP_Q,3,i14,
                    FUNCID_CEP_STOPPDP_P,1,o14);
  marshalFuncTbl.push_back(f14);  

  //SDK API: CEP_GetChallenge
  const nlchar *i15[2]={_T(NAME_CEString), //reqID
                        _T(NAME_CEHandle)}; //public: session id
  const nlchar *o15[2]={_T(NAME_CEString), //reqID
                        _T(NAME_CEString)}; //public: challenge
  CEMarshalFunc f15(_T("CEP_GetChallenge"),
                    FUNCID_CEP_GETCHALLENGE_Q, 2, i15,
                    FUNCID_CEP_GETCHALLENGE_P,2,o15);
  marshalFuncTbl.push_back(f15);  

  //SDK API: CELOG_LogDecision  
  const nlchar *i16[5]={_T(NAME_CEString), //reqID
                        _T(NAME_CEHandle), //public: session id
                        _T(NAME_CEString), //public: cookie
                        _T(NAME_CEint32),  //public: userResponse (Deny/Allow)
                        _T(NAME_CEAttributes)}; //public: optional attrs
  const nlchar *o16[2]={_T(NAME_CEString)}; //reqID
  CEMarshalFunc f16(_T("CELOG_LogDecision"),
                    FUNCID_CELOG_LOGDECISION_Q,5, i16,
                    FUNCID_CELOG_LOGDECISION_P,1,o16);
  marshalFuncTbl.push_back(f16);  

  //SDK API: CELOGGING_LogAssistantData 
  const nlchar *i17[8]={_T(NAME_CEString), //reqID
                        _T(NAME_CEHandle), //public: session id
                        _T(NAME_CEString), //public: logIdentifier
                        _T(NAME_CEString), //public: assistantName 
                        _T(NAME_CEString), //public: assistantOptions
                        _T(NAME_CEString), //public: assistantDescription
                        _T(NAME_CEString), //public: assistantUserActions
                        _T(NAME_CEAttributes)}; //public: optional attrs
  const nlchar *o17[1]={_T(NAME_CEString)}; //reqID
  CEMarshalFunc f17(_T("CELOG_LogAssistantData"),
                    FUNCID_CELOG_LOGASSISTANTDATA_Q,8,i17,
                    FUNCID_CELOG_LOGASSISTANTDATA_P,1,o17);
  marshalFuncTbl.push_back(f17);  

  //SDK API: Pseudo function "generic"
  CEMarshalFunc f18(_T("GENERIC_QUERY"),
                    FUNCID_GENERIC_FUNCCALL_Q, 0, NULL,
                    FUNCID_GENERIC_FUNCCALL_P, 0, NULL);
  marshalFuncTbl.push_back(f18);  

  //SDK API: Make process "trusted"
  const nlchar *i19[] = { _T(NAME_CEString),  // reqID
                          _T(NAME_CEint32),   // public: process id
                          _T(NAME_CEString)}; // public: password
  const nlchar *o19[1]={_T(NAME_CEString)};   //reqID
  CEMarshalFunc f19(_T("CESEC_MakeProcessTrusted"),
                    FUNCID_CESEC_MAKETRUSTED_Q,_countof(i19),i19,
                    FUNCID_CESEC_MAKETRUSTED_P,_countof(o19),o19);
  marshalFuncTbl.push_back(f19);

  CEMarshalFunc f20(_T("CEEVALUATE_CheckResourcesEx"),
                    FUNCID_EVALUATE_CHECKMULTIRESOURCES_Q, 0, NULL,
                    FUNCID_EVALUATE_CHECKMULTIRESOURCES_P, 0, NULL);
  marshalFuncTbl.push_back(f20);


  const nlchar *i_nn[8]={_T(NAME_CEint32), 
                         _T(NAME_CEBoolean), 
                         _T(NAME_CEAction_t), 
                         _T(NAME_CEEnforcement_t), 
                         _T(NAME_CEEnforcement_t),
                         _T(NAME_CEPEP_t), 
                         _T(NAME_CEKeyRoot_t), 
                         _T(NAME_CEString)};
  const nlchar *o_nn[1]={_T(NAME_CEAttributes_Array)};
  CEMarshalFunc f_nn(_T("test_marshal"), 0xFFFE0000,8, i_nn,  
                     0xFFFF0000, 1, o_nn);
  marshalFuncTbl.push_back(f_nn);  

  //Initialize funcLookupTbl
  if(funcLookupTbl.size() == 0) {
    /*Initialize the lookup table*/
    for(int i=0; i<static_cast<int>(marshalFuncTbl.size()); i++) 
    {
      funcLookupTbl[marshalFuncTbl[i].funcName]=i;
    }
  }
}


/* =======================Marshal_PackReqFunc=============================*
 * Generate a buffer for a request function that will be sent over the    *
 * socket.                                                                *
 * Parameters:                                                            *
 * funcName (input): the name of the request function. The function has to*
 *                   be the valid CE RPC function.                        *
 * argv (input): the vector of the pointers to function input arguments.  *
 * reqLen (output): the length of retruned function layout.               *
 *                                                                        *
 * Return:                                                                *
 *   It will return NULL if the function fails; otherwise, it will        *
 *   return the pointer to the buffer of function layout.                 *
 * Example:                                                               *
 *   CEHandle h;                                                          *
 *   int reqLen;                                                          *
 *   std::vector<void *> argbuf;                                          *
 *   ...                                                                  *
 *   argBuf.push_back((void *)(&h));                                      *  
 *   char *buf=Marshal_PackReqFunc("CONN_Close", argBuf, reqLen);         *
 *   Marshal_PackFree(buf);                                               *
 *   buf=NULL;                                                            *
 * Note:                                                                  *
 *   It is caller's responsibility call the function "Marshal_PackFree" to*
 *   free the memory of buffer returned by the function                   *
 *   "Marshal_PackReqFunc".                                               *
 *                                                                        *
 *   The order of arguments vector has to be same as the order of         *
 *   request function's input arguments.                                  * 
 *                                                                        *
 *   If the ith argument is an array type, the i+1th argument must be the *
 *   lenghth of array. For example,                                       *
 *   std::vector<void *> argbuf;                                          *
 *   CEint32 a[10];                                                       *
 *   int len=10;                                                          *
 *   int reqLen;                                                          *
 *   ...                                                                  *
 *   argBuf.push_back((void *)a);                                         *  
 *   argBuf.push_back((void *)(&len));                                    * 
 *   char *buf=Marshal_PackReqFunc("xxx", argBuf, reqLen);                *
 * =======================================================================*/
char *Marshal_PackReqFunc(const nlchar *funcName, 
                          vector<void *> &argv,
                          size_t &reqLen)
{
  try {
    /* Invalid input */
    if(funcName == NULL )
    {
      return NULL;
    }

    /* Get the function index */
    int funcID=GetFuncIndex(funcName);

    /* Invalid function */
    if(funcID == FUNC_NOT_FOUND)
    {
      return NULL;
    }

    /* The number of arguments mismatch. */ 
    if(marshalFuncTbl[funcID].inputArgs.size() != argv.size())
    {
      return NULL;
    }
    
    /*Get the byte size of the buffer of request function*/
    size_t len=GetReqFuncLength(funcID, argv);
    if(len<=0)
    {
      return NULL;
    }
    reqLen=len;

    /*Allocate memory for the result buffer*/
    char *buf_start=new char[len];
    if(buf_start==NULL) return NULL;

    /*Pack the function's header*/
    char *buf_cur=PackReqFuncHeader(funcID, buf_start);

    /*Pack function input arguments*/
    for(unsigned int i=0; i<marshalFuncTbl[funcID].inputArgs.size(); i++) {
      if(nlstrstr(marshalFuncTbl[funcID].inputArgs[i].c_str(),_T("[]"))) {
        //Array type. The following i+1th element specifies the length
        //of the array. 
        const nlchar *atype=marshalFuncTbl[funcID].inputArgs[i].c_str();
        void *aptr=argv[i++];
        buf_cur=PackFuncArrayArgument(atype, *((int *)argv[i]),aptr, buf_cur);
      } else 
        buf_cur=PackFuncArgument(marshalFuncTbl[funcID].inputArgs[i].c_str(),
                                 argv[i], buf_cur);
    }

    /*Return the pointer to the result buffer*/
    return buf_start;    
  } catch (MarshalException &) {
    return NULL;
  } catch (exception &) {
    return NULL;
  } catch (...) {
    return NULL;
  }
}

/* =======================Marshal_PackReqGeneric==========================*
 * Generate a buffer for a format string that will be sent over the       *
 * socket.                                                                *
 * Parameters:                                                            *
 * fmt (input): the format string describing the data                     *
 * argv (input): the vector of the pointers to function input arguments.  *
 * reqLen (output): the length of retruned function layout.               *
 *                                                                        *
 * Return:                                                                *
 *   It will return NULL if the function fails; otherwise, it will        *
 *   return the pointer to the buffer of function layout.                 *
 * Example:                                                               *
 *   CEInt32 arg1;                                                        *
 *   CEInt32 arg2;                                                        *
 *   std::vector<void *> argbuf;                                          *
 *   int reqLen;                                                          *
 *   ...                                                                  *
 *   argBuf.push_back((void *)(&arg1));                                   *  
 *   argBuf.push_back((void *)(&arg2));                                   *  
 *   char *buf=Marshal_PackReqGeneric("ii", argBuf, reqLen);              *
 *   Marshal_PackFree(buf);                                               *
 *   buf=NULL;                                                            *
 * Note:                                                                  *
 *   It is caller's responsibility call the function "Marshal_PackFree"   *
 *   to free the memory of buffer returned by the function                *
 *   "Marshal_PackReqGeneric".                                            *
 * =======================================================================*/
char *Marshal_PackReqGeneric(const nlchar *fmt, vector<void *> &argv, size_t &reqLen)
{
  return Marshal_PackReqFuncAndFormat(_T("GENERIC_QUERY"), fmt, argv, reqLen);
}

/* =======================Marshal_PackReqFuncAndFormat====================*
 * Generate a buffer for a format string that will be sent over the       *
 * socket. This provides named functions with variable arguments (a sort  *
 * of combination of PackReqFunc and PackRegGeneric                       *
 * Parameters:                                                            *
 * funcName (input): the name of the request function. The function has to*
 *                   be the valid CE function.                            *
 * fmt (input): the format string describing the data                     *
 * argv (input): the vector of the pointers to function input arguments.  *
 * reqLen (output): the length of retruned function layout.               *
 *                                                                        *
 * Return:                                                                *
 *   It will return NULL if the function fails; otherwise, it will        *
 *   return the pointer to the buffer of function layout.                 *
 * Example:                                                               *
 *   CEInt32 arg1;                                                        *
 *   CEInt32 arg2;                                                        *
 *   std::vector<void *> argbuf;                                          *
 *   int reqLen;                                                          *
 *   ...                                                                  *
 *   argBuf.push_back((void *)(&arg1));                                   *  
 *   argBuf.push_back((void *)(&arg2));                                   *  
 *   char *buf=Marshal_PackReqFuncAndFormat(_T("myfunc"), "ii",           *
 *                                          argBuf, reqLen);              *
 *   Marshal_PackFree(buf);                                               *
 *   buf=NULL;                                                            *
 * Note:                                                                  *
 *   It is caller's responsibility call the function "Marshal_PackFree"   *
 *   to free the memory of buffer returned by the function                *
 *   "Marshal_PackReqFuncAndFormat".                                      *
 * =======================================================================*/
char *Marshal_PackReqFuncAndFormat(const nlchar *funcName, const nlchar *fmt, vector<void *> &argv, size_t &reqLen)
{
  try {
    if (fmt == NULL) {
      return NULL;
    }

    /* Argument mismatch */
    if (static_cast<int>(argv.size()) != numArguments(fmt)) {
      return NULL;
    }

    /* Get the function index */
    int funcID = GetFuncIndex(funcName);

    /* That's not good */
    if (funcID == FUNC_NOT_FOUND) {
      return NULL;
    }

    CEString cefmt = convertToCEString(fmt);

    reqLen = GetFuncGenericLength(cefmt, argv);
    if (reqLen <= 0) {
      return NULL;
    }
    char *buf_start = new char[reqLen];
    char *buf_cur = PackReqFuncHeader(funcID, buf_start);

    buf_cur=PackFuncArgument(_T(NAME_CEString), cefmt, buf_cur);
    delete cefmt;

    const nlchar *type;

    int i = 0;
    while((type = getEncoding(&fmt)) != NULL)
    {
      buf_cur=PackFuncArgument(type, argv[i++], buf_cur);
    }

    return buf_start;
  } catch (MarshalException &) {
    return NULL;
  } catch (exception &) {
    return NULL;
  } catch (...) {
    return NULL;
  }
}

/* =======================Marshal_PackFuncReply===========================*
 * Generate a buffer for a CE request function's reply that will be sent  *
 * over the socket.                                                       *
 * Parameters:                                                            *
 * funcName (input): the name of the replied function. The function has to*
 *                   be the valid CE function.                            *
 * result: the return value in CEResult_t type.                           *
 * argv (input): the pointer to the array of returned arguments.          *
 * replyLen (output): the length of retruned function reply layout.       *
 *                                                                        *
 * Return:                                                                *
 *   It will return NULL if the function fails; otherwise, it will        *
 *   return the pointer to the buffer of function layout.                 *
 * Example:                                                               *
 *   CEResult_t r;                                                        *
 *   CEHandle h;                                                          *
 *   int replyLen;                                                        *
 *   std::vector<void *> returnArgv;                                      *
 *   ...                                                                  *
 *   returnArgv.push_back((void *)(&h));                                  *
 *   char *buf=Marshal_PackFuncReply("CONN_Initialize", r, returnArgv,    *
 *                                   replyLen);                           *
 *   Marshal_PackFree(buf);                                               *
 *   buf=NULL;                                                            *
 * Note:                                                                  *
 *   It is caller's responsibility to call the function "Marshal_PackFree"*
 *   to free the memory of buffer returned by the function                *
 *   "Marshal_PackFuncReply".                                             * 
 *                                                                        *
 *   The order of elements in vector is same as the order of request      *
 *   function's returned arguments.                                       *  
 *                                                                        *
 *   If the ith argument is an array type, the i+1th argument must be the *
 *   lenghth of array. For example,                                       *
 *   std::vector<void *> argbuf;                                          *
 *   CEint32 a[10];                                                       *
 *   int replyLen;                                                        *
 *   int len=10;                                                          *
 *   ...                                                                  *
 *   argBuf.push_back((void *)a);                                         *  
 *   argBuf.push_back((void *)(&len));                                    * 
 *   char *buf=Marshal_PackFuncReply("xxx", argBuf, replyLen);            *
 * =======================================================================*/
char *Marshal_PackFuncReply(const nlchar *funcName, 
                            CEResult_t result, 
                            vector<void *> &argv, 
                            size_t &replyLenInByte)
{
  try {
    /* Invalid input */
    if(funcName == NULL )
      return NULL;

    /* Get the function index */
    int funcID=GetFuncIndex(funcName);

    /* Invalid function */
    if(funcID == FUNC_NOT_FOUND)
      return NULL;

    /*Get the size of the buffer of request function*/
    size_t len=GetFuncReplyLength(funcID, argv);
    if(len<=0) return NULL;
    replyLenInByte=len;

    /*Allocate memory for the result buffer*/
    char *buf_start=new char[len];
    if(buf_start==NULL) return NULL;

    /*Pack the function's header*/
    char *buf_cur=PackFuncReplyHeader(funcID, result, buf_start);

    /*Pack function input arguments*/
    for(unsigned int i=0; i<marshalFuncTbl[funcID].outputArgs.size(); i++) {
      if(nlstrstr(marshalFuncTbl[funcID].outputArgs[i].c_str(),_T("[]"))) {
        //Array type. The following i+1th element specifies the length
        //of the array. 
        const nlchar *atype=marshalFuncTbl[funcID].outputArgs[i].c_str();
        void *aptr=argv[i++];
        buf_cur=PackFuncArrayArgument(atype, *((int *)argv[i]), aptr, buf_cur);
      } else 
        buf_cur=PackFuncArgument(marshalFuncTbl[funcID].outputArgs[i].c_str(),
                                 argv[i], buf_cur);
    }

    /*Return the pointer to the result buffer*/
    return buf_start;    
  } catch (MarshalException &) {
    return NULL;
  } catch (exception &) {
    return NULL;
  } catch (...) {
    return NULL;
  }
}

/* =======================Marshal_PackReplyGeneric========================*
 * Generate a buffer for a CE request function's reply that will be sent  *
 * over the socket.                                                       *
 * Parameters:                                                            *
 * fmt: the format string                                                 *
 * result: the return value in CEResult type.                             *
 * argv (input): the pointer to the array of returned arguments           *
 * replyLen (output): the length of retruned function reply layout.       *
 *                                                                        *
 * Return:                                                                *
 *   It will return NULL if the function fails; otherwise, it will        *
 *   return the pointer to the buffer of function layout.                 *
 * Example:                                                               *
 *   CEResult r;                                                          *
 *   CEInt32 arg;                                                         *
 *   int replyLen;                                                        *
 *   std::vector<void *> returnArgv;                                      *
 *   ...                                                                  *
 *   returnArgv.push_back((void *)(&arg);                                 *
 *   char *buf=Marshal_PackFuncReply("i", r, returnArgv, replyLen);       *
 *   Marshal_PackFree(buf);                                               *
 *   buf=NULL;                                                            *
 * Note:                                                                  *
 *   It is caller's responsibility to call the function "Marshal_PackFree"*
 *   to free the memory of buffer returned by the function                *
 *   "Marshal_PackFuncReply".                                             * 
 * =======================================================================*/
char *Marshal_PackReplyGeneric(const nlchar *fmt,
                               CEResult_t result, 
                               vector<void *> &argv,
                               size_t &replyLen)
{
  return Marshal_PackFuncAndFormatReply(_T("GENERIC_QUERY"), fmt, result, argv, replyLen);
}


/* =======================Marshal_PackFuncAndFormatReply==================*
 * Generate a buffer for a CE request function's reply that will be sent  *
 * over the socket.                                                       *
 * Parameters:                                                            *
 * funcName (input): the function name                                    *
 * fmt (input): a format string describing the data                       *
 * result: the return value in CEResult type.                             *
 * argv (input): the pointer to the array of returned arguments.          *
 * replyLen (output): the length of retruned function reply layout.       *
 *                                                                        *
 * Return:                                                                *
 *   It will return NULL if the function fails; otherwise, it will        *
 *   return the pointer to the buffer of function layout.                 *
 * Example:                                                               *
 *   CEResult r;                                                          *
 *   CEInt32 arg;                                                         *
 *   int replyLen;                                                        *
 *   std::vector<void *> returnArgv;                                      *
 *   ...                                                                  *
 *   returnArgv.push_back((void *)(&arg);                                 *
 *   char *buf=Marshal_PackFuncReply("i", r, returnArgv, replyLen);       *
 *   Marshal_PackFree(buf);                                               *
 *   buf=NULL;                                                            *
 * Note:                                                                  *
 *   It is caller's responsibility to call the function "Marshal_PackFree"*
 *   to free the memory of buffer returned by the function                *
 *   "Marshal_PackFuncReply".                                             * 
 * =======================================================================*/
char *Marshal_PackFuncAndFormatReply(const nlchar *funcName, const nlchar *fmt, CEResult_t result, vector<void *> &argv, size_t &replyLen)
{
  try {
    if (fmt == NULL) {
      return NULL;
    }

    int funcID=GetFuncIndex(funcName);

    if (funcID == FUNC_NOT_FOUND) {
      // Shouldn't be possible
      return NULL;
    }

    CEString cefmt = convertToCEString(fmt);

    replyLen = GetFuncGenericLength(cefmt, argv, true);

    if (replyLen <= 0) {
      return NULL;
    }

    char *buf_start = new char[replyLen];
        
    char *buf_cur=PackFuncReplyHeader(funcID, result, buf_start);

    buf_cur = PackFuncArgument(_T(NAME_CEString), cefmt, buf_cur);

    delete cefmt;

    const nlchar *type;

    int i = 0;
    while((type = getEncoding(&fmt)) != NULL)
    {
      buf_cur=PackFuncArgument(type, argv[i++], buf_cur);
    }

    return buf_start;    
  } catch (MarshalException &) {
    return NULL;
  } catch (exception &) {
    return NULL;
  } catch (...) {
    return NULL;
  }
}

/* =======================Marshal_PackFree================================*
 * Free the memory allocated for packing request function or reply.       *
 *                                                                        *
 * Parameters:                                                            *
 * bufPtr (input): pointer to the memory to be freed.                     *
 *                                                                        *
 * Return:                                                                *
 *   It will return CE_RESULT_GENERAL_FAILED if the function fails;       *
 *   otherwise, it will CE_RESULT_SUCCESS.                                *
 *                                                                        *
 * Example:                                                               *
 *   CEResult_t r;                                                        *
 *   CEHandle h;                                                          *
 *   std::vector<void *> returnArgv;                                      *
 *   ...                                                                  *
 *   returnArgv.push_back((void *)(&h));                                  *
 *   char *buf=Marshal_PackFuncReply("CONN_Initialize", r, returnArgv);   *
 *   Marshal_PackFree(buf);                                               *
 *   buf=NULL;                                                            *
 * Note:                                                                  *
 *   It is caller's responsibility to call function "Marshal_PackFree" to *
 *   free the memory of buffer returned by the function                   *
 *   "Marshal_PackFuncReply".                                             * 
 * =======================================================================*/
CEResult_t Marshal_PackFree(char *bufPtr) 
{
  if(bufPtr)
    delete [] bufPtr;
  return CE_RESULT_SUCCESS; 
}


/* =======================Marshal_UnPackFree==============================*
 * Free the memory allocated for unpacking request function or reply.     *
 *                                                                        *
 * Parameters:                                                            *
 * argv (input/output): the the vector of pointers to allocated memory.   *
 * funcName (input): the name of the function                             *
 * bRequest (input): true if free the memory for requestion function;     *
 *                   false if free the memory for function reply.         *
 *                                                                        *
 * Return:                                                                *
 *   It will return CE_RESULT_GENERAL_FAILED if the function fails;       *
 *   otherwise, it will CE_RESULT_SUCCESS.                                *
 * Example:                                                               *
 *   std::string funcName;                                                *    
 *   std::vector<void *> argv;                                            *
 *   ...                                                                  *
 *   CEResult_t r=Marshal_UnPackReqFunc(request, funcName, argv);           *
 *   if(r == CE_RESULT_SUCCESS) {                                         *
 *     if(funcID=FUNC_CONN_Close)                                         *
 *       CONN_Close((CEHandle)argv[0]);                                   *
 *     Marshal_UnPackFree(funcName.c_str(), argv, true);                  *
 *   }                                                                    *
 * Note:                                                                  *
 *   It is caller's responsibility to call function "Marshal_UnPackFree"  *
 *   to free the memory allocated for unpacking request function and reply*
 *========================================================================*/
CEResult_t Marshal_UnPackFree(const nlchar *funcName, 
                              vector<void *> &argv,
                              bool bRequest)
{
  try {
    int funcID=GetFuncIndex(funcName);
    
    /* Invalid function */
    if(funcID == FUNC_NOT_FOUND)
      return CE_RESULT_FUNCTION_NOT_AVAILBLE;

    /*Free each argument's memory*/
    vector<nlstring> &args=(bRequest)?marshalFuncTbl[funcID].inputArgs:
                           marshalFuncTbl[funcID].outputArgs;
    
    if ((bRequest && marshalFuncTbl[funcID].inputHasFixedFormat()) ||
        (!bRequest && marshalFuncTbl[funcID].outputHasFixedFormat())) {
      for(unsigned int i=0; i<argv.size(); i++) {
        if(nlstrstr(args[i].c_str(),_T("[]"))){
          //Array type. The following i+1th element specifies the length
          //of the array.
          if (nlstrstr(args[i].c_str(), _T(NAME_CEint32))) {
            delete [] (CEint32 *)argv[i++];
          } else if (nlstrstr(args[i].c_str(),_T(NAME_CEBoolean))) {
            delete [] (CEBoolean *)argv[i++];
          } else if (nlstrstr(args[i].c_str(),_T(NAME_CEKeyRoot_t))) {
            delete [] (CEKeyRoot_t *)argv[i++];
          } else if (nlstrstr(args[i].c_str(),_T(NAME_CEAction_t))) {
            delete [] (CEAction_t *)argv[i++];
          } else if (nlstrstr(args[i].c_str(),_T(NAME_CEPEP_t))) {
            delete [] (CEPEP_t *)argv[i++];
          }
          delete (int *)argv[i];
        } else if(args[i]==_T(NAME_CEAttributes_Array)) {
          if(argv[i]) {
            CEAttributes *aattrs=((CEAttributes_Array *)argv[i])->attrs_array;
            for(int j=0; j<((CEAttributes_Array *)argv[i])->count; j++) {
              CEAttribute *attrs=aattrs[j].attrs;
              for(int l=0; l<aattrs[j].count; l++) {
                delete attrs[l].key;
                delete attrs[l].value;
              }
              delete [] attrs;
            }   
            delete [] aattrs;
            delete (CEAttributes_Array *)argv[i];
          }
        } else if(args[i]==_T(NAME_CEAttributes)) {
          if(argv[i]) {
            for(int j=0; j<((CEAttributes *)argv[i])->count; j++) {
              delete (((CEAttributes *)argv[i])->attrs[j]).key;
              delete (((CEAttributes *)argv[i])->attrs[j]).value;
            }   
            delete [] ((CEAttributes *)argv[i])->attrs;
            delete (CEAttributes *)argv[i];
          }
        } else if(args[i]==_T(NAME_CEAttribute)) {
          delete ((CEAttribute *)argv[i])->key;
          delete ((CEAttribute *)argv[i])->value;
          delete (CEAttribute *)argv[i];
        } else if (args[i]==_T(NAME_CEString)) {
          delete (CEString)argv[i];
        } else if (args[i]==_T(NAME_CEint32)) {
          delete (CEint32 *)argv[i];
        } else if (args[i] == _T(NAME_CEBoolean)) {
          delete (CEBoolean *)argv[i];
        } else if (args[i] == _T(NAME_CEKeyRoot_t)) {
          delete (CEKeyRoot_t *)argv[i];
        } else if (args[i] == _T(NAME_CEAction_t)) {
          delete (CEAction_t *)argv[i];
        } else if (args[i] == _T(NAME_CEEnforcement_t)) {
          if(((CEEnforcement_t *)argv[i])->obligation) {
            for(int j=0; j<((CEEnforcement_t *)argv[i])->obligation->count;j++) {
              delete ((CEEnforcement_t *)argv[i])->obligation->attrs[j].key;
              delete ((CEEnforcement_t *)argv[i])->obligation->attrs[j].value;
            }   
            delete [] ((CEEnforcement_t *)argv[i])->obligation->attrs;
            delete ((CEEnforcement_t *)argv[i])->obligation;
          }
          delete (CEEnforcement_t *)argv[i];
        } else if (args[i] == _T(NAME_CEPEP_t)) {
          delete (CEPEP_t *)argv[i];
        } else if (args[i] == _T(NAME_CEHandle)) {
          delete (struct _CEHandle *)argv[i];
        }
        argv[i]=NULL;
      }
      return CE_RESULT_SUCCESS; 
    } else {
      return Marshal_FreeGeneric(argv);
    }
  } catch (MarshalException &) {
    return CE_RESULT_GENERAL_FAILED;
  } catch (exception &) {
    return CE_RESULT_GENERAL_FAILED;
  } catch (...) {
    return CE_RESULT_GENERAL_FAILED;
  }
}



    
/* =======================Marshal_FreeGeneric=============================*
 * Free the memory allocated for unpacking request function or reply.     *
 *                                                                        *
 * Parameters:                                                            *
 * argv (input/output): the the vector of pointers to allocated memory.   *
 *                                                                        *
 * Return:                                                                *
 *   It will return CE_RESULT_GENERAL_FAILED if the function fails;       *
 *   otherwise, it will CE_RESULT_SUCCESS.                                *
 * Example:                                                               *
 *   std::vector<void *> argv;                                            *
 *   ...                                                                  *
 *   CEResult r=Marshal_UnPackReqGeneric(request, fmt, argv);             *
 *   if(r == CE_RESULT_SUCCESS) {                                         *
 *     Marshal_FreeGeneric(argv);                                         *
 *   }                                                                    *
 * Note:                                                                  *
 *   It is caller's responsibility to call function "Marshal_FreeGeneric" *
 *   to free the memory allocated for unpacking request function and reply*
 *                                                                        *
 *   This obviously shares a lot of logic with Marshal_UnPackFree         *
 *========================================================================*/
CEResult_t Marshal_FreeGeneric(vector<void *> &argv)
{
  if (argv.size() < 1) {
    // No data.  This implies something wrong with the input, but
    // since there is nothing to free we don't need to return an error
    return CE_RESULT_SUCCESS;
  }

  const nlchar *fmt = ((CEString)argv[0])->buf;
  
  CEResult_t res = FreeGenericExplicitFormat(fmt, argv, 1);

  delete (CEString)argv[0]; 
  argv[0] = NULL;

  return res;
}

/* ================Marshal_FreeGenericExplicitFormat======================*
 * Free the memory allocated for unpacking request function or reply.     *
 * This is used in cases where the fmt string is *not* part of the argv   *
 * vector (e.g. when constructing, as opposed to receiving the request or *
 *  response)                                                             *
 *                                                                        *
 * Parameters:                                                            *
 * fmt (input): the format string describing the data                     *
 * argv (input/output): the the vector of pointers to allocated memory.   *
 *                                                                        *
 * Return:                                                                *
 *   It will return CE_RESULT_GENERAL_FAILED if the function fails;       *
 *   otherwise, it will CE_RESULT_SUCCESS.                                *
 * Example:                                                               *
 *   std::string fmt;                                                     *    
 *   std::vector<void *> argv;                                            *
 *   ...                                                                  *
 *   CEResult r=Marshal_UnPackReqGeneric(request, fmt, argv);             *
 *   if(r == CE_RESULT_SUCCESS) {                                         *
 *     Marshal_FreeGeneric(argv);                                         *
 *   }                                                                    *
 * Note:                                                                  *
 *   It is caller's responsibility to call function "Marshal_FreeGeneric" *
 *   to free the memory allocated for unpacking request function and reply*
 *========================================================================*/
CEResult_t Marshal_FreeGenericExplicitFormat(const nlchar *fmt, vector<void *> &argv)
{
  return FreeGenericExplicitFormat(fmt, argv, 0);

}

/* =======================Marshal_UnPackReqFunc===========================*
 * Unpack a buffer of a request function.  Return the function name and   *
 * the arry of its input arguments.                                       *
 *                                                                        *
 * Parameters:                                                            *
 * request (input): the buffer storing the request function. The function *
 *                  has to be the valid CE function.                      *
 * funcName (output): the name of request function.                       *
 * argv (output): the vector of function input arguments.                 *
 *                                                                        *
 * Return:                                                                *
 *   It will return CE_RESULT_GENERAL_FAILED if the function fails;       *
 *   otherwise, it will CE_RESULT_SUCCESS.                                *
 * Example:                                                               *
 *   std::string funcName;                                                *    
 *   std::vector<void *> argv;                                            *
 *   ...                                                                  *
 *   CEResult_t r=Marshal_UnPackReqFunc(request, funcName, argv);           *
 *   if(r == CE_RESULT_SUCCESS) {                                         *
 *     if(funcID=FUNC_CONN_Close)                                         *
 *       CONN_Close((CEHandle)argv[0]);                                   *
 *     Marshal_UnPackFree(funcName.c_str(), argv, true);                  *
 *   }                                                                    *
 * Note:                                                                  *
 *   It is caller's responsibility to call function "Marshal_UnPackFree"  *
 *   to free the memory allocated for request function arguments.         *
 *                                                                        *
 *   The order of elements in vector is same as the order of request      *
 *   function's arguments.                                                *  
 *========================================================================*/
CEResult_t Marshal_UnPackReqFunc(char *request, nlstring &funcName, 
                                 vector<void *> &argv)
{
  try {
    if(request == NULL) 
      return CE_RESULT_GENERAL_FAILED;

    int funcID;
    CEResult_t ret=UnPackReqFuncHeader(request, funcID);

    if(ret != CE_RESULT_SUCCESS)
      return ret;

    funcName=marshalFuncTbl[funcID].funcName;

    request+=8; //The length of request function header

    void *arg; 
    bool bUnpackArgSuccess=true;

    if (marshalFuncTbl[funcID].inputHasFixedFormat()) {
      int aLen;
      for(unsigned int i=0; i<marshalFuncTbl[funcID].inputArgs.size(); i++) {
        aLen=0;
        if(nlstrstr(marshalFuncTbl[funcID].inputArgs[i].c_str(),_T("[]"))) {
          //Array type. The following i+1th element specifies the length
          //of the array. 
          const nlchar *atype=marshalFuncTbl[funcID].inputArgs[i].c_str();
          request=UnPackFuncArrayArgument(atype, request, aLen, &arg);
        } else 
          request=UnPackFuncArgument(marshalFuncTbl[funcID].inputArgs[i].c_str(),
                                     request, &arg, bUnpackArgSuccess);
        if(!bUnpackArgSuccess) {
          Marshal_UnPackFree(funcName.c_str(), argv, true);
          return CE_RESULT_GENERAL_FAILED;
        }
        argv.push_back(arg);
        if( aLen > 0) 
          argv.push_back(new int(aLen));
      }
    } else {
      // The first argument is the fmt
      request = UnPackFuncArgument(_T("CEString"), request, &arg, bUnpackArgSuccess);

      if (!bUnpackArgSuccess) {
        return CE_RESULT_GENERAL_FAILED;
      }

      const nlchar *fmt = ((CEString)arg)->buf;

      if (fmt == NULL) {
        return CE_RESULT_GENERAL_FAILED;
      }

      argv.push_back(arg);

      const nlchar *enc;
      while ((enc = getEncoding(&fmt)) != NULL) {
        // The generic interface doesn't support arrays
        request = UnPackFuncArgument(enc, request, &arg, bUnpackArgSuccess);

        if (!bUnpackArgSuccess) {
          Marshal_FreeGeneric(argv);
          return CE_RESULT_GENERAL_FAILED;
        }
        argv.push_back(arg);
      }
        
    }
    
    return CE_RESULT_SUCCESS; 
  } catch (MarshalException &) {
    return CE_RESULT_GENERAL_FAILED;
  } catch (exception &) {
    return CE_RESULT_GENERAL_FAILED;
  } catch (...) {
    return CE_RESULT_GENERAL_FAILED;
  }
}


/* =======================Marshal_UnPackFuncReply=========================*
 * Unpack a buffer of a request function reply.  Return the function      *
 * name, the returned code, and the vector of ite returned arguments.     *
 *                                                                        *
 * Parameters:                                                            *
 * reply  (input): the buffer storing the reply of the request function.  *
 *                 The function has to be the valid CE function.          *
 * funcName (output): the name of request function.                       *
 * result (output): the return code of request function.                  *
 * argv (output): the the vector of function returned arguments.          *
 *                                                                        *
 * Return:                                                                *
 *   It will return CE_RESULT_GENERAL_FAILED if the function fails;       *
 *   otherwise, it will CE_RESULT_SUCCESS.                                *
 * Example:                                                               *
 *   std::string funcName;                                                *    
 *   std::vector<void *> argv;                                            *
 *   CEResult_t funcRet;                                                    *
 *   ...                                                                  *
 *   CEResult_t r=Marshal_UnPackFuncReply(reply, funcName, result, argv);   *
 *   if(r == CE_RESULT_SUCCESS) {                                         *
 *     if(funcID=FUNC_CONN_Close)                                         *
 *       std::cout<<"CONN_Close "                                         *
 *                <<(result==CE_RESULT_SUCCESS)?"succeed":"failed"        *
 *                <<std::endl;                                            *
 *     Marshal_UnPackFree(funcName, argv, false);                         *
 *   }                                                                    *
 * Note:                                                                  *
 *   It is caller's responsibility to call function "Marshal_UnPackFree"  *
 *   to free the memory allocated for function reply arguments.           *
 *                                                                        *
 *   The order of elements in vector is same as the order of request      *
 *   function's return arguments.                                         *  
 *========================================================================*/
CEResult_t Marshal_UnPackFuncReply(char *reply, 
                                   nlstring &funcName, 
                                   CEResult_t &result, 
                                   vector<void *> &argv)
{
  try {
    int funcID;
    CEResult_t ret=UnPackFuncReplyHeader(reply, funcID, result);

    if(ret != CE_RESULT_SUCCESS)
      return ret;

    funcName=marshalFuncTbl[funcID].funcName;
    
    reply+=12; //The length of function reply header

    void *arg; 
    bool bUnpackArgSuccess=true;

    if (marshalFuncTbl[funcID].outputHasFixedFormat()) {
      int aLen;
      for(unsigned int i=0; i<marshalFuncTbl[funcID].outputArgs.size(); i++) {
        aLen=0;
        if(nlstrstr(marshalFuncTbl[funcID].outputArgs[i].c_str(),_T("[]"))) {
          //Array type. The following i+1th element specifies the length
          //of the array. 
          const nlchar *atype=marshalFuncTbl[funcID].outputArgs[i].c_str();
          reply=UnPackFuncArrayArgument(atype, reply, aLen, &arg);
        } else 
          reply=UnPackFuncArgument(marshalFuncTbl[funcID].outputArgs[i].c_str(),
                                   reply, &arg, bUnpackArgSuccess);
        if(!bUnpackArgSuccess) {
          Marshal_UnPackFree(funcName.c_str(), argv, false);
          return CE_RESULT_GENERAL_FAILED;
        }
        argv.push_back(arg);
        if( aLen > 0) 
          argv.push_back(new int(aLen));
      }
    } else {
      // The first argument is the fmt
      reply = UnPackFuncArgument(_T("CEString"), reply, &arg, bUnpackArgSuccess);

      if (!bUnpackArgSuccess) {
        return CE_RESULT_GENERAL_FAILED;
      }

      const nlchar *fmt = ((CEString)arg)->buf;

      if (fmt == NULL) {
        return CE_RESULT_GENERAL_FAILED;
      }

      argv.push_back(arg);

      const nlchar *enc;
      while ((enc = getEncoding(&fmt)) != NULL) {
        // The generic interface doesn't support arrays
        reply = UnPackFuncArgument(enc, reply, &arg, bUnpackArgSuccess);

        if (!bUnpackArgSuccess) {
          Marshal_FreeGeneric(argv);
          return CE_RESULT_GENERAL_FAILED;
        }
        argv.push_back(arg);
      }
    }

    return CE_RESULT_SUCCESS; 
  } catch (MarshalException &) {
    return CE_RESULT_GENERAL_FAILED;
  } catch (exception &) {
    return CE_RESULT_GENERAL_FAILED;
  } catch (...) {
    return CE_RESULT_GENERAL_FAILED;
  }
}

/* =======================Marsh_GetFuncSignature==========================*
 * Get a CE RPC function's signature.                                     *
 *                                                                        *
 * Parameters:                                                            *
 * funcName (input): the name of the function. The function has to        *
 *                   be the valid CE RPC function.                        *
 *                                                                        *
 * Return:                                                                *
 *   It will return NULL if the function fails; otherwise, it will        *
 *   return the CEMarshalFunc type pointer to the function signature.     *
 * Example:                                                               *
 *   CEMarshalFunc *f=Marshal_GetFuncSignature("PROTECT_LockKey");        *
 * =======================================================================*/
CEMarshalFunc *Marsh_GetFuncSignature(const nlchar *funcName)
{
  try {
    /* Invalid Input */
    if(funcName==NULL)
      return NULL;
    
    /* Get the function index */
    int index = GetFuncIndex(funcName);
    
    /* The function is not a valid CE RPC function. Return NULL. */ 
    if(index == FUNC_NOT_FOUND) 
      return NULL;
    
    /* Return the pointer to RPC function signature. */
    return &marshalFuncTbl[index]; 
  } catch (MarshalException &) {
    return NULL;
  } catch (exception &) {
    return NULL;
  } catch (...) {
    return NULL;
  }
}

// creates CEString based on given character array
// code is copy & pasted from CEM_AllocateString()
// this is necessary to allocate memory inside cemarshal.dll
#ifdef WIN32
CEString Marshal_AllocateCEString(const TCHAR * str)
#else
  CEString Marshal_AllocateCEString(const char * str)
#endif
{
  if(str == NULL)
    return NULL;
    
  try {
    CEString ces=new struct _CEString();
 
    ces->length=nlstrlen(str);
    ces->buf = new nlchar[ces->length+1];
    nlstrncpy_s(ces->buf, ces->length+1, str, _TRUNCATE);
 
    return ces;
  } catch(...) {
    return (NULL);
  }
}
    
// Marshal_FreeString
// free string allocated by Marshal_AllocateString
// code is copy & pasted from CEM_FreeString()
CEResult_t Marshal_FreeCEString(CEString cestr)
{
  if(cestr==NULL)
    return CE_RESULT_SUCCESS;
    
  delete cestr;
    
  return CE_RESULT_SUCCESS;
}

