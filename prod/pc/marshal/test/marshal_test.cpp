#include "brain.h"
#include "nlstrings.h"
#include "cetype.h"
#include "marshal.h"
#include "CESDK_private.h"
#include <iostream>

void test_pack_unpack_request()
{
  std::cout << "\n\n\nCalling test_pack_unpack_request\n";

  vector<void *> argBuf;
  nlstring funcName(100u, ' ');
  CEint32 n=2;
  CEBoolean b=CETrue;
  CEAction_t a=CE_ACTION_READ;

  argBuf.reserve(RPC_MAX_NUM_ARGUMENTS);
  CEEnforcement_t e1;
  e1.result=CEAllow;
  e1.obligation=NULL;

  CEEnforcement_t e2;
  e2.result=CEDeny;
  CEString okey=CEM_AllocateString(_T("obligation2 key"));
  CEString oval=CEM_AllocateString(_T("obligation2 value"));
  CEAttribute oattr;
  CEAttributes oattrs;
  oattr.key = okey;
  oattr.value = oval;
  oattrs.count=1;
  oattrs.attrs=&oattr;
  e2.obligation=&oattrs;

  CEPEP_t p=CE_PEP_CLASS_KERNEL;
  CEKeyRoot_t r=CE_KEYROOT_USERS;
  CEString str=CEM_AllocateString(_T("hello! Marshal"));
  int reqLen;

  argBuf.push_back(&n);
  argBuf.push_back(&b);
  argBuf.push_back(&a);
  argBuf.push_back(&e1);
  argBuf.push_back(&e2);
  argBuf.push_back(&p);
  argBuf.push_back(&r);
  argBuf.push_back(str);
  char *packed=Marshal_PackReqFunc(_T("test_marshal"), argBuf, reqLen);

  if(packed) {
    vector<void *> argBuf;
    argBuf.reserve(RPC_MAX_NUM_ARGUMENTS);
    CEResult_t ret=Marshal_UnPackReqFunc(packed, funcName, argBuf);   
    if(ret == CE_RESULT_SUCCESS) {                                         
      TRACE(0, _T("\nPack/Unpack request successfully:\nbuffer length %d\nfuncName: %s\nn=%d expect %d\nb=%d expect %d\na=%d expect %d\ne1.result=%d expect %d\ne1.obligation's size=%d expect %d\ne2.result=%d expect %d\ne2.obligation's size=%d expect %d\ne2.obligation.key='%s' expect '%s'\ne2.obligation.val='%s' expect '%s'\np=%d expect %d\nr=%d expect %d\nstr '%s' expect '%s'\n\n"), reqLen, funcName.c_str(),
	    *((CEint32 *)argBuf[0]),n,
	    *((CEBoolean *)argBuf[1]),b,
	    *((CEAction_t *)argBuf[2]),a,
	    ((CEEnforcement_t *)argBuf[3])->result,e1.result,
	    (((CEEnforcement_t *)argBuf[3])->obligation)?((CEEnforcement_t *)argBuf[3])->obligation->count:0,
  	    (e1.obligation)?e1.obligation->count:0,
	    ((CEEnforcement_t *)argBuf[4])->result,e2.result,
	    (((CEEnforcement_t *)argBuf[4])->obligation)?((CEEnforcement_t *)argBuf[4])->obligation->count:0,
  	    (e2.obligation)?e2.obligation->count:0,
	    ((CEEnforcement_t *)argBuf[4])->obligation->attrs->key->buf,
  	    e2.obligation->attrs->key->buf,
	    ((CEEnforcement_t *)argBuf[4])->obligation->attrs->value->buf,
  	    e2.obligation->attrs->value->buf,
	    *((CEPEP_t *)argBuf[5]),p,
	    *((CEKeyRoot_t *)argBuf[6]),r,
	    ((CEString)argBuf[7])->buf ,str->buf);
      Marshal_UnPackFree(funcName.c_str(), argBuf, true); 
    } else {
      std::cout<<"Unpack request failed"<<std::endl;  
    }
    Marshal_PackFree(packed); packed = NULL;
  } else
    std::cout<<"Pack request failed"<<std::endl;    
  CEM_FreeString(str);  
  CEM_FreeString(okey);
  CEM_FreeString(oval);
}

void test_pack_unpack_reply()
{
  std::cout << "\n\n\nCalling test_pack_unpack_reply\n";

  vector<void *> argBuf;
  nlstring funcName(100u, ' ');
  int replyLen;
  argBuf.reserve(10);

  CEAttributes_Array aattrs;
  aattrs.count=3;
  aattrs.attrs_array=new CEAttributes[3];
  CEString key=CEM_AllocateString(_T("CEAttribute key"));
  CEString val=CEM_AllocateString(_T("CEAttribute value"));
  for(int i=0; i<3; i++) {
    aattrs.attrs_array[i].count=2;
    aattrs.attrs_array[i].attrs=new CEAttribute[2];
    aattrs.attrs_array[i].attrs[0].key=key;
    aattrs.attrs_array[i].attrs[0].value=val;
    aattrs.attrs_array[i].attrs[1].key=key;
    aattrs.attrs_array[i].attrs[1].value=val;
  }
  argBuf.push_back(&aattrs);
  
  char *packed=Marshal_PackFuncReply(_T("test_marshal"), 
				     CE_RESULT_SUCCESS, argBuf, replyLen);
  if(packed) {
    vector<void *> argBuf;
    argBuf.reserve(10);
    CEResult_t reply;
    CEResult_t rt=Marshal_UnPackFuncReply(packed,funcName,reply, argBuf);  
    if(rt == CE_RESULT_SUCCESS) {    
      TRACE(0, 
	    _T("\nPack/Unpack reply successfully: \nreply length=%d\nfuncName %s\nreply=%d exepct %d\nAttrs Array count=%d expect 3\n"),
	    replyLen,funcName.c_str(),
	    reply,CE_RESULT_SUCCESS, 
	    ((CEAttributes_Array *)argBuf[0])->count);
      CEAttributes *aattrs=((CEAttributes_Array *)argBuf[0])->attrs_array;
      for(int i=0; i<((CEAttributes_Array *)argBuf[0])->count; i++) {
	TRACE(0, _T("Array-%d attributes count=%d execpt 2\n"),
	      i, aattrs[i].count);
	CEAttribute *attrs=aattrs[i].attrs;
	for(int j=0; j<aattrs[i].count; j++) {
	  TRACE(0, _T("\tAttr[%d][%d]: key='%s' value='%s'\n"),
		i, j, attrs[j].key->buf, attrs[j].value->buf);
	}
      }
      Marshal_UnPackFree(funcName.c_str(), argBuf, false); 
      Marshal_PackFree(packed); packed = NULL;
    } else {
      std::cout<<"\nUnpack reply failed"<<std::endl;  
      Marshal_PackFree(packed);	  
    }
  } else 
    std::cout<<"\nPack reply failed"<<std::endl;
  for(int i=0; i<3; i++) {
    delete [] aattrs.attrs_array[i].attrs;
  }
  delete [] aattrs.attrs_array;
  CEM_FreeString(key);
  CEM_FreeString(val);
}


void test_generic_request()
{
    std::cout << "\n\n\nCalling test_generic_request\n";

    vector<void *> argBuf;

    CEString s1 = CEM_AllocateString(_T("String Number One"));
    CEString s2 = CEM_AllocateString(_T("String Number Two"));

    CEint32 i1 = 42;  // The answer
    CEint32 i2 = 8675309; // Jenny
    
    CEBoolean b1 = CETrue;
    CEBoolean b2 = CEFalse;

    CEString k1 = CEM_AllocateString(_T("key1"));
    CEString v1 = CEM_AllocateString(_T("value1"));
    CEString k2 = CEM_AllocateString(_T("key2"));
    CEString v2 = CEM_AllocateString(_T("value2"));

    CEAttributes attrs;
    attrs.count = 2;
    attrs.attrs = (CEAttribute *)malloc(sizeof(CEAttribute) * attrs.count);
    attrs.attrs[0].key = k1;
    attrs.attrs[0].value = v1;
    attrs.attrs[1].key = k2;
    attrs.attrs[1].value = v2;

    argBuf.push_back(s1);
    argBuf.push_back(&i1);
    argBuf.push_back(&b1);
    argBuf.push_back(s2);
    argBuf.push_back(&i2);
    argBuf.push_back(&b2);
    argBuf.push_back(&attrs);

    nlchar *fmt = _T("sibsiba");
    int reqLen;

    nlstring funcName(100u, ' ');
    std::cout << "Calling Marshal_PackReqGeneric\n";
    char *packed = Marshal_PackReqGeneric(fmt, argBuf, reqLen);
    std::cout << "Called Marshal_PackReqGeneric\n";

    if (packed) {
        std::cout << "Pack generic was successful\n";
        vector<void *> argBuf;
        argBuf.reserve(10);
        CEResult_t ret = Marshal_UnPackReqFunc(packed, funcName, argBuf);
        if (ret == CE_RESULT_SUCCESS) {
            std::wcout << L"Unpack succeeded, " << argBuf.size() << L" entries in argbuf\n";

            std::wcout << L"First argument is format string: " << ((CEString)argBuf[0])->buf << std::endl;

            std::wcout << L"Got " << ((CEString)argBuf[1])->buf << L" expecting " << s1->buf << std::endl;
            std::wcout << L"Got " << *((CEint32 *)argBuf[2]) << L" expecting " << i1 << std::endl;
            std::wcout << L"Got " << *((CEBoolean *)argBuf[3]) << L" expecting " << b1 << std::endl;
            std::wcout << L"Got " << ((CEString)argBuf[4])->buf << L" expecting " << s2->buf << std::endl;
            std::wcout << L"Got " << *((CEint32 *)argBuf[5]) << L" expecting " << i2 << std::endl;
            std::wcout << L"Got " << *((CEBoolean *)argBuf[6]) << L" expecting " << b2 << std::endl;

            CEAttributes *unpackattrs = (CEAttributes *)argBuf[7];

            std::wcout << L"Got " << unpackattrs->count << L" attributes, expecting 2" << std::endl;
            std::wcout << L"  " << unpackattrs->attrs[0].key->buf << L"=" << unpackattrs->attrs[0].value->buf << std::endl;
            std::wcout << L"  " << unpackattrs->attrs[1].key->buf << L"=" << unpackattrs->attrs[1].value->buf << std::endl;

            std::wcout << L"Calling free generic" << std::endl;

            Marshal_FreeGeneric(argBuf);
                                   
        } else {
            std::cout<<"Unpack request failed"<<std::endl;  
        }
    } else {
        std::cout<<"\nPack generic request failed"<<std::endl;
    }
}

void test_generic_reply()
{
    std::cout << "\n\n\nCalling test_generic_reply\n";

    vector<void *> argBuf;

    CEString s1 = CEM_AllocateString(_T("String Number Uno"));
    CEString s2 = CEM_AllocateString(_T("String Number Dos"));

    CEint32 i1 = 42;  // The answer
    CEint32 i2 = 8675309; // Jenny
    
    CEBoolean b1 = CETrue;
    CEBoolean b2 = CEFalse;

    CEString k1 = CEM_AllocateString(_T("key1"));
    CEString v1 = CEM_AllocateString(_T("value1"));
    CEString k2 = CEM_AllocateString(_T("key2"));
    CEString v2 = CEM_AllocateString(_T("value2"));

    CEAttributes attrs;
    attrs.count = 2;
    attrs.attrs = (CEAttribute *)malloc(sizeof(CEAttribute) * attrs.count);
    attrs.attrs[0].key = k1;
    attrs.attrs[0].value = v1;
    attrs.attrs[1].key = k2;
    attrs.attrs[1].value = v2;

    argBuf.push_back(&attrs);
    argBuf.push_back(s1);
    argBuf.push_back(s2);
    argBuf.push_back(&i1);
    argBuf.push_back(&b1);
    argBuf.push_back(&i2);
    argBuf.push_back(&b2);

    nlchar *fmt = _T("assibib");
    int reqLen;
    
    CEResult_t response = CE_RESULT_INVALID_ACTION_ENUM;

    nlstring funcName(100u, ' ');
    std::cout << "Calling Marshal_PackReplyGeneric\n";

    char *packed = Marshal_PackReplyGeneric(fmt, response, argBuf, reqLen);
    std::cout << "Called Marshal_PackReplyGeneric\n";

    if (packed) {
        CEResult_t res;
        std::cout << "Pack generic was successful\n";
        vector<void *> argBuf;
        argBuf.reserve(10);
        CEResult_t ret = Marshal_UnPackFuncReply(packed, funcName, res, argBuf);
        if (ret == CE_RESULT_SUCCESS) {
            std::wcout << L"Unpack succeeded, " << argBuf.size() << L" entries in argbuf\n";

            std::wcout << L"Response was " << res << L" expecting " << response << std::endl;
            std::wcout << L"First argument is format string: " << ((CEString)argBuf[0])->buf << std::endl;

            CEAttributes *unpackattrs = (CEAttributes *)argBuf[1];

            std::wcout << L"Got " << unpackattrs->count << L" attributes, expecting 2" << std::endl;
            std::wcout << L"  " << unpackattrs->attrs[0].key->buf << L"=" << unpackattrs->attrs[0].value->buf << std::endl;
            std::wcout << L"  " << unpackattrs->attrs[1].key->buf << L"=" << unpackattrs->attrs[1].value->buf << std::endl;

            std::wcout << L"Got " << ((CEString)argBuf[2])->buf << L" expecting " << s1->buf << std::endl;
            std::wcout << L"Got " << ((CEString)argBuf[3])->buf << L" expecting " << s2->buf << std::endl;
            std::wcout << L"Got " << *((CEint32 *)argBuf[4]) << L" expecting " << i1 << std::endl;
            std::wcout << L"Got " << *((CEBoolean *)argBuf[5]) << L" expecting " << b1 << std::endl;
            std::wcout << L"Got " << *((CEint32 *)argBuf[6]) << L" expecting " << i2 << std::endl;
            std::wcout << L"Got " << *((CEBoolean *)argBuf[7]) << L" expecting " << b2 << std::endl;

            Marshal_FreeGeneric(argBuf);
        } else {
            std::cout<<"Unpack reply failed"<<std::endl;  
        }
    } else {
        std::cout<<"\nPack generic reply failed"<<std::endl;
    }
}

int main()
{
  test_pack_unpack_request();

  test_pack_unpack_reply();

  test_generic_request();

  test_generic_reply();

  return 0;
}
