
%{

#include <cstdio>
#include <cstdlib>
#include <string>
#include <list>

#include "pcs_idl_objects.hpp"
#include "pcs_rpc_packer.hpp"

#pragma warning( disable : 4996 )
#pragma warning( disable : 4065 )

#pragma warning( push )
#pragma warning( disable : 6011 )
#  include <boost/algorithm/string.hpp>
#pragma warning( pop )

extern int yylex();
extern int yyerror( char* s );
extern "C" int yywrap();

extern std::string op_file;
extern std::string op_service;
extern bool op_verify;

std::list< idl_param > params;
std::list< idl_object* > idl_objects;

class idl_header : public idl_object
{
  public:

    idl_object_type_t type(void)
    {
      return PCS_IDL_OBJECT_HEADER;
    }

    bool code_gen_client( std::string& code )
    {
      std::string text(header);
      text.erase(0,1);
      text.erase(text.length()-1,1);
      code = text;
      return true;
    }

    bool code_gen_server( std::string& code )
    {
      return code_gen_client(code);
    }

    void set_header( std::string in_header )
    {
      header = in_header;
    }

 private:
    std::string header;
};/* idl_header */

class idl_method : public idl_object
{
  public:

    idl_object_type_t type(void)
    {
      return PCS_IDL_OBJECT_METHOD;
    }

    bool code_gen_client_header( std::string& code )
    {
      code.clear();

      code.append("\n");
      code.append("extern \"C\"\n");
      code.append("CEResult_t ");
      code.append(name);
      code.append("( CEHandle h , ");

      std::list<idl_param>::iterator it;
      for( it = params.begin() ; it != params.end() ; )
      {
        if( !it->type_qualifier.empty() )
        {
          code.append(" ");
          code.append( it->type_qualifier.c_str() );
        }

	code.append(" ");
	code.append( it->type.c_str() );

	if( it->options & IDL_PARAM_OPTION_OUT )
	{
	  code.append("*");
	}

	code.append(" ");
	code.append( it->name.c_str() );
	code.append(" ");

	it++;
	if( it != params.end() )
	{
	  code.append(" , ");
	}
      }
      code.append(", int timeout_ms );\n");

      code.append("\n");

      return true;
    }

    bool code_gen_server_header( std::string& code )
    {
      char temp[512] = {0};
      code.clear();

      code.append("\n");
      code.append("extern \"C\"\n");
      code.append("CEResult_t ");

      sprintf(temp,"server_%s",name.c_str());
      code.append(temp);
      code.append("(");

      std::list<idl_param>::iterator it;
      for( it = params.begin() ; it != params.end() ; )
      {
        if( !it->type_qualifier.empty() )
        {
          code.append(" ");
          code.append( it->type_qualifier.c_str() );
        }

	code.append(" ");
	code.append( it->type.c_str() );

	if( it->options & IDL_PARAM_OPTION_OUT )
	{
	  code.append("*");
	}

	code.append(" ");
	code.append( it->name.c_str() );
	code.append(" ");

	it++;
	if( it != params.end() )
	{
	  code.append(" , ");
	}
      }
      code.append(");\n");

      code.append("\n");

      return true;
    }/* code_gen_server_header */

    bool code_gen_client( std::string& code )
    {
      char temp[512] = {0};
      code.clear();

      if( op_verify == true )
      {
	code.append("/* Verification \"fake\" CE SDK service API */\n");
	code.append("extern \"C\" CEResult_t FakeServiceInvoke(CEHandle,CEString,CEString,void**,void***,CEint32);\n");
	code.append("extern \"C\" CEResult_t FakeServiceResponseFree(void**);\n");
      }

      code.append("\n");
      code.append("extern \"C\"\n");
      code.append("CEResult_t ");
      code.append(name);
      code.append("( CEHandle h , ");

      std::list<idl_param>::iterator it;
      for( it = params.begin() ; it != params.end() ; )
      {
        if( !it->type_qualifier.empty() )
        {
          code.append(" ");
          code.append( it->type_qualifier.c_str() );
        }

	code.append(" ");
	code.append( it->type.c_str() );

	if( it->options & IDL_PARAM_OPTION_OUT )
	{
	  code.append("*");
	}

	code.append(" ");
	code.append( it->name.c_str() );
	code.append(" ");

	it++;
	if( it != params.end() )
	{
	  code.append(" , ");
	}
      }

      code.append(" , int in_timeout ");
      code.append(")\n");

      std::string invoke_code;

      /*******************************************************************************
       * Generate packing code
       ******************************************************************************/
      invoke_code.append("\n");
      invoke_code.append("  CEString packed_string = NULL;\n");
      invoke_code.append("  CEString service_name = NULL;\n");
      invoke_code.append("  CEString fmt = NULL;\n");
      invoke_code.append("\n");
      invoke_code.append("  /* Packing input parameters */\n");
      invoke_code.append("  packer in_pack;\n");

      invoke_code.append("\n");
      invoke_code.append("  /* Set RPC parameters */\n");
      sprintf(temp,"  in_pack.set_service(\"%s\");\n",op_service.c_str());
      invoke_code.append(temp);

      sprintf(temp,"  in_pack.set_method(\"%s\");\n",name.c_str());
      invoke_code.append(temp);
      invoke_code.append("\n");

      /* Generate code for packing input parameters */
      for( it = params.begin() ; it != params.end() ; ++it )
      {
	if( it->options & IDL_PARAM_OPTION_IN )
	{
          if( it->options & IDL_PARAM_OPTION_PTR )
          {
            sprintf(temp,"  in_pack.pack(%s);\n",it->name.c_str());
          }
          else
          {
            sprintf(temp,"  in_pack.pack(&%s);\n",it->name.c_str());
          }

	  invoke_code.append(temp);
	}
      }

      code.append("{\n  CEResult_t rv = CE_RESULT_GENERAL_FAILED;\n");
      invoke_code.append("  \n"
			 "  packer out_packer;\n");

      sprintf(temp,
	      "  /* Allocate packed string */\n"
	      "  size_t temp_size = in_pack.size_coded() * sizeof(wchar_t);\n"
	      "  wchar_t* temp = (wchar_t*)malloc( temp_size );\n"
	      "  if( temp == NULL )\n"
	      "  {\n"
	      "    rv = CE_RESULT_GENERAL_FAILED;\n"
	      "    goto complete;\n"
	      "  }\n"
	      "\n");
      invoke_code.append(temp);

      invoke_code.append("  /* Write out coded string for invokation */\n");
      invoke_code.append("  swprintf_s(temp,temp_size/sizeof(wchar_t),L\"%hs\",in_pack.get_coded_string().c_str());\n"
                         "  packed_string = cesdk->fns.CEM_AllocateString(temp);\n"
			 "  if( packed_string == NULL )\n"
			 "    goto complete;\n"
			 "  \n"
			 "  void** response = NULL;\n"
			 "\n");

      sprintf(temp,"  service_name = cesdk->fns.CEM_AllocateString(L\"%s\");\n",op_service.c_str());
      invoke_code.append(temp);
      invoke_code.append("  if( service_name == NULL )\n"
			 "    goto complete;\n"
			 "\n"
			 );

      invoke_code.append("  fmt = cesdk->fns.CEM_AllocateString(L\"s\");\n"
			 "  if( fmt == NULL )\n"
			 "    goto complete;\n"
			 "\n"
			 "  /* Allocate request object */\n"
			 "  void* request[2] = {0};\n"
			 "  *(request + 0) = (void*)packed_string;\n"
			 "  *(request + 1) = NULL;\n"
			 "\n");

      invoke_code.append("  /* Call into service as a transport */\n");
      if( op_verify == true )
      {
	invoke_code.append("  rv = FakeServiceInvoke(h,service_name,fmt,request,&response,in_timeout);\n");
      }
      else
      {
	invoke_code.append("  rv = cesdk->fns.ServiceInvoke(h,service_name,fmt,request,&response,in_timeout);\n");
      }

      invoke_code.append("  if( rv != CE_RESULT_SUCCESS )\n"
			 "  {\n"
			 "    goto complete;\n"
			 "  }\n"
			 "  \n"
			 "  cesdk->fns.CEM_FreeString(packed_string);\n"
			 "  \n"
			 "  /* response[0] holds the format */\n"
			 "  CEString response_string = (CEString)response[1];\n"
			 "  assert( response_string != NULL );\n"
			 "  \n"
			 "  /* Unpack and decode response */\n"
			 "  size_t out_param_size = 0;\n"
			 "  const void* out_param_ptr = NULL;\n"

			 "  UNREFERENCED_PARAMETER(out_param_size);\n"
			 "  UNREFERENCED_PARAMETER(out_param_ptr);\n"
			 "\n"
			 "  out_packer.assign_code_string(cesdk->fns.CEM_GetString(response_string));\n\n"
			 );

      if( op_verify == true )
      {
	invoke_code.append("  FakeServiceResponseFree(response);\n");
      }
      else
      {
	invoke_code.append("  cesdk->fns.ServiceResponseFree(response);\n");
      }
      invoke_code.append("\n");

      /*******************************************************************************
       * Process the unpacking of the parameters.
       ******************************************************************************/
      size_t out_param_count = 0;
      for( it = params.begin() ; it != params.end() ; ++it )
      {
	if( it->options & IDL_PARAM_OPTION_OUT )
	{
	  sprintf(temp,"  /* Unpack '%s' parameter %d */\n",it->name.c_str(), out_param_count);
	  invoke_code.append(temp);

	  sprintf(temp,"  out_param_ptr = out_packer.at(%d,&out_param_size);\n",out_param_count);
	  invoke_code.append(temp);
	  invoke_code.append("  assert( out_param_ptr != NULL );\n");
	  invoke_code.append("\n");

	  /* allocation requires space for response */
	  if( it->options & IDL_PARAM_OPTION_ALLOC )
	  {
	    sprintf(temp,"  *%s = (%s%s)malloc(out_param_size);\n",it->name.c_str(),
                    it->type_qualifier.empty() ? "" : (it->type_qualifier + " ").c_str(),
                    it->type.c_str());
	    invoke_code.append(temp);
	    
	    sprintf(temp,
		    "  if( *%s == NULL )\n"
		    "  {\n"
		    "    rv = CE_RESULT_GENERAL_FAILED;\n"
		    "    goto complete;\n"
		    "  }\n",
		    it->name.c_str());
	    invoke_code.append(temp);
	    
	    sprintf(temp,"  memcpy(*%s,out_param_ptr,out_param_size);\n",it->name.c_str());
	    invoke_code.append(temp);
	  }
	  else
	  {
	    sprintf(temp,"  memcpy(%s,out_param_ptr,out_param_size);\n",it->name.c_str());
	    invoke_code.append(temp);
	  }

	  invoke_code.append("\n");
	  out_param_count++;
	}
      }/* for */

      code.append(invoke_code);

      code.append("  rv = out_packer.get_return();\n");

      /*******************************************************************************
       * Function is complete
       ******************************************************************************/
      code.append("\n"
		  "  complete:\n"
		  "\n"
		  "  if( service_name != NULL )\n"
		  "    cesdk->fns.CEM_FreeString(service_name);\n"
		  "\n"
		  "  if( fmt != NULL )\n"
		  "    cesdk->fns.CEM_FreeString(fmt);\n"
		  "\n"
		  "  if( temp != NULL )\n"
		  "    free(temp);\n"
		  "\n"
		  );

      code.append("  return rv;\n");
      sprintf(temp,"} /* %s */\n",name.c_str());
      code.append(temp);
      code.append("\n");

      return true;
    }/* code_gen_client */

    bool code_gen_server( std::string& code )
    {
      std::list<idl_param>::iterator it;
      char temp[512] = {0};
      code.clear();

      code.append("\n");

      sprintf(temp,"extern \"C\" void dispatch_%s( const char* in_string , char** out_string )\n", name.c_str());
      code.append(temp);
      code.append("{\n");

      for( it = params.begin() ; it != params.end() ; it++ )
      {
	sprintf(temp,"  %s param_%s;\n",it->type.c_str(),it->name.c_str());
	code.append(temp);
      }

      code.append("\n");

      code.append("  CEResult_t rv;\n");
      code.append("  packer in_packer;\n");
      code.append("  packer out_packer;\n");
      code.append("  void* in_param_ptr = NULL;\n");
      code.append("  void* out_param_ptr = NULL;\n");
      code.append("  size_t in_param_size = 0;\n");
      code.append("  size_t out_param_size = 0;\n");

      code.append("\n");
      code.append("  UNREFERENCED_PARAMETER(in_param_ptr);\n");
      code.append("  UNREFERENCED_PARAMETER(out_param_ptr);\n");
      code.append("  UNREFERENCED_PARAMETER(in_param_size);\n");
      code.append("  UNREFERENCED_PARAMETER(out_param_size);\n");

      code.append("  \n");

      code.append("  in_packer.assign_code_string(in_string);\n");

      code.append("\n");
      code.append("  /* Unpack the input paramters and setup for call */\n");
      code.append("\n");
      size_t i = 0;
      for( it = params.begin() ; it != params.end() ; it++ )
      {
	if( it->options & IDL_PARAM_OPTION_IN )
	{
	  sprintf(temp,"  /* Input parameter %d is param_%s */\n",i,it->name.c_str());
	  code.append(temp);

	  sprintf(temp,"  in_param_ptr = in_packer.at(%d,&in_param_size);\n",i);
	  code.append(temp);
	  code.append("  assert( in_param_ptr != NULL );\n");

          if( it->options & IDL_PARAM_OPTION_PTR )
          {
            sprintf(temp,"  param_%s = (%s)malloc( in_param_size );\n",
                    it->name.c_str(), it->type.c_str());
            code.append(temp);
            sprintf(temp,"  memcpy(param_%s,in_param_ptr,in_param_size);\n", it->name.c_str());
            code.append(temp);
          }
          else
          {
            sprintf(temp,"  memcpy(&param_%s,in_param_ptr,in_param_size);\n", it->name.c_str());
            code.append(temp);
          }

	  code.append("\n");
	  i++;
	}
      }
      code.append("\n");

      code.append("  /* Call server-side implementation */\n");
      sprintf(temp,"  rv = server_%s(",name.c_str());
      code.append(temp);

      i = 0;
      for( it = params.begin() ; it != params.end() ; )
      {
	if( it->options & IDL_PARAM_OPTION_OUT )
	{
	  sprintf(temp,"&param_%s",it->name.c_str());
	}
	else if(  it->options & IDL_PARAM_OPTION_IN )
        {
	  sprintf(temp,"param_%s",it->name.c_str());
	}
	else
	{
	  fprintf(stderr, "error: [in|out] not defined for parameter %s\n", it->name.c_str());
	  exit(1);
	}
	code.append(temp);
	it++;
	if( it != params.end() )
	{
	  code.append(",");
	}
      }
      code.append(");\n");
      code.append("\n");

      /*******************************************************************************
       * Free allocated input parameters
       ******************************************************************************/
      code.append("  /* Free allocated input parameters */\n");
      for( it = params.begin() ; it != params.end() ; it++ )
      {
	if( ( it->options & IDL_PARAM_OPTION_IN ) && ( it->options & IDL_PARAM_OPTION_PTR ) )
	{
	  sprintf(temp,"  /* Free input parameter param_%s */\n",it->name.c_str());
	  code.append(temp);
	  sprintf(temp,"  free(param_%s);\n",it->name.c_str());
	  code.append(temp);
	}
      }
      code.append("\n");

      /* Pack the return value for the client caller */
      code.append("  out_packer.set_return(rv);\n");

      /* If the implementation call failed, then bail out since output parameters do
       * not require packing.  The method must complete since the return value will
       * be packed and the output string must be constructed and returned to the
       * RPC client.
       */
      code.append("  if( rv != CE_RESULT_SUCCESS )\n"
		  "  {\n"
		  "    goto complete;\n"
		  "  }\n"
		  );

      /*******************************************************************************
       * Pack reply
       ******************************************************************************/
      for( it = params.begin() ; it != params.end() ; ++it )
      {
	if( it->options & IDL_PARAM_OPTION_OUT )
	{
	  /* If the out parameter is allocated, then pack the raw parameter based on
	   * the size of the output size parameter set by the callee.
	   */
	  if( it->options & IDL_PARAM_OPTION_ALLOC )
	  {
	    sprintf(temp,"  out_packer.pack_raw(param_%s,param_%s);\n",it->name.c_str(),it->size_of_object.c_str());
	    code.append(temp);

	    sprintf(temp,"  free(param_%s);\n",it->name.c_str());
	    code.append(temp);
	  }
	  else
	  {
	    sprintf(temp,"  out_packer.pack(&param_%s);\n",it->name.c_str());
	    code.append(temp);
	  }
	}
	code.append("\n");
      }

      code.append("  complete:\n");

      /* Allocate space for the codified reply */
      code.append("  size_t rpc_out_string_size = out_packer.get_coded_string().length() + 1;\n");
      code.append("  *out_string = (char*)malloc( rpc_out_string_size );\n");
      code.append("  if( *out_string == NULL )\n");
      code.append("  {\n");
      code.append("    return;\n");
      code.append("  }\n");
      code.append("  strncpy_s(*out_string,rpc_out_string_size,out_packer.get_coded_string().c_str(),_TRUNCATE);\n");
      sprintf(temp, "} /* dispatch_%s */\n", name.c_str());
      code.append(temp);
      code.append("\n");

      return true;
    }/* code_gen_server */

    void set_name( std::string in_name )
    {
      name = in_name;
    }

    void set_params( std::list< idl_param >& in_params )
    {
      params = in_params;
    }

    //private:
    std::list< idl_param > params;
    std::string body;
    std::string name;
};/* class idl_method */

%}

%union
{
  char string_val[1024];
}

%start Start

%token T_LEFT_PAREN
%token T_RIGHT_PAREN
%token T_LEFT_BRACKET
%token T_RIGHT_BRACKET
%token T_COMMA
%token T_IDL
%token T_TYPE
%token T_PARAM
%token T_METHOD
%token T_HEADER
%token <string_val> T_BODY
%token <string_val> T_STRING_LITERAL

%type <string_val> MethodPrototype
%type <string_val> ParamOptions
%type <string_val> ParamOptionList

%%

Start: IDLStatementList
{
  char temp[512] = {0};
  FILE* fp = NULL;
  std::list<idl_object*>::iterator it;

  sprintf(temp,"%s_client.hpp",op_file.c_str());
  fp = fopen(temp,"w+");
  fprintf(fp, "#ifndef __%s_CLIENT_HPP__\n", op_file.c_str());
  fprintf(fp, "#define __%s_CLIENT_HPP__\n", op_file.c_str());
  fprintf(fp, "\n");
  fprintf(fp,
	  "\n"
	  "#include <windows.h>\n"
	  "#include <tchar.h>\n"
	  "#include \"CEsdk.h\"\n"
	  "#include \"eframework/platform/cesdk_loader.hpp\"\n"
	  "\n"
	  "/* Set the CE SDK for RPC client */\n"
	  "extern \"C\" void pcs_rpc_set_sdk( nextlabs::cesdk_loader* in_cesdk );\n"
	  "\n"
	  );

  for( it = idl_objects.begin() ; it != idl_objects.end() ; ++it )
  {
    std::string code;
    (*it)->code_gen_client_header(code);
    fprintf(fp, "%s", code.c_str());
  }
  fprintf(fp, "#endif __%s_CLIENT_HPP__\n", op_file.c_str());
  fclose(fp);

  sprintf(temp,"%s_client.cpp",op_file.c_str());
  fp = fopen(temp,"w+");
  fprintf(fp,
	  "#include \"pcs_rpc_packer.hpp\"\n"
	  "#include <tchar.h>\n"
	  "#include \"CEsdk.h\"\n"
	  "#include \"eframework/platform/cesdk_loader.hpp\"\n"
	  );

  fprintf(fp,
	  "\n"
	  "\n"
	  "static nextlabs::cesdk_loader* cesdk = NULL;\n"
	  "\n"
	  "\n"
	  "extern \"C\" void pcs_rpc_set_sdk( nextlabs::cesdk_loader* in_cesdk )\n"
	  "{\n"
	  "  assert( in_cesdk != NULL );\n"
	  "  cesdk = in_cesdk;\n"
	  "}\n"
	  "\n"
	  );
  for( it = idl_objects.begin() ; it != idl_objects.end() ; ++it )
  {
    std::string code;
    (*it)->code_gen_client(code);
    fprintf(fp, "%s", code.c_str());
  }
  fclose(fp);

  /**********************************************************************
   * Server code generation
   *********************************************************************/

  sprintf(temp,"%s_server.cpp",op_file.c_str());
  fp = fopen(temp,"w+");

  fprintf(fp, "\n");
  fprintf(fp, "#include <windows.h>\n");
  fprintf(fp, "#include \"CEsdk.h\"\n");
  fprintf(fp, "#include \"pcs_rpc.hpp\"\n");
  fprintf(fp, "#include \"pcs_rpc_packer.hpp\"\n");

  /* Prototype server calls */
  for( it = idl_objects.begin() ; it != idl_objects.end() ; ++it )
  {
    std::string code;
    (*it)->code_gen_server_header(code);
    fprintf(fp, "%s", code.c_str());
  }

  fprintf(fp, "\n");
  fprintf(fp, "\n");
  for( it = idl_objects.begin() ; it != idl_objects.end() ; ++it )
  {
    std::string code;
    (*it)->code_gen_server(code);
    fprintf(fp, "%s", code.c_str());
  }

  fprintf(fp,
	  "\n"
	  "\n /* Dispatch to the proper method based on the request */\n"
	  "extern \"C\" void server_dispatch( const char* in_string , char** out_string )\n"
	  "{\n"
	  "\n"
	  "  char method[64] = {0};\n"
	  "\n"
	  "  if( pcs_rpc_request::get_method(in_string,method,_countof(method)) == false )\n"
	  "    return;\n"
	  "\n"
	  );

  /* Generate code for dispatch into method based on request */
  for( it = idl_objects.begin() ; it != idl_objects.end() ; ++it )
  {
    if( (*it)->type() != PCS_IDL_OBJECT_METHOD )
      continue;

    idl_method* method = dynamic_cast<idl_method*>(*it);
    fprintf(fp,
	    "  if( strcmp(method,\"%s\") == 0 )\n"
	    "    dispatch_%s(in_string,out_string);\n"
	    "\n",
	    method->name.c_str(), method->name.c_str());
    
  }
  fprintf(fp,"  /* Invalid method */\n");
  fprintf(fp,"}\n");

  fclose(fp);

  for( it = idl_objects.begin() ; it != idl_objects.end() ; ++it )
  {
    delete (*it);
  }

  YYACCEPT;
}
;

IDLStatementList: IDLStatementList IDLStatement
|                 IDLStatement
;

IDLStatement: T_IDL T_HEADER T_BODY
{
  idl_header* header = new idl_header();
  header->set_header($3);
  idl_objects.push_back(header);
}
|             T_IDL T_METHOD MethodPrototype
{
  idl_method* method = new idl_method();
  method->set_name($3);
  method->set_params(params);
  idl_objects.push_back(method);
  params.clear();
}
|             T_IDL error
{
  fprintf(stderr, "error: bad @idl statement\n");
  YYABORT;
}
;

Param: T_IDL T_PARAM ParamOptions T_LEFT_PAREN T_RIGHT_PAREN
{
  //fprintf(stdout, "type: no type\n");
}
|      T_IDL T_PARAM ParamOptions T_LEFT_PAREN T_STRING_LITERAL T_RIGHT_PAREN T_STRING_LITERAL
{
  idl_param param;
  param.name = $7;
  param.type = $5;
  param.options = 0x0;

  if( boost::algorithm::icontains($3,"out") == true )
  {
    param.options |= IDL_PARAM_OPTION_OUT;
  }
  if( boost::algorithm::icontains($3,"in") == true &&
      boost::algorithm::icontains($3,"*") == false )
  {
    param.options |= IDL_PARAM_OPTION_IN;
  }
  if( boost::algorithm::icontains($3,"alloc") == true )
  {
    param.options |= IDL_PARAM_OPTION_ALLOC;
  }

  if( boost::algorithm::icontains($3,"ptr") == true )
  {
    param.options |= IDL_PARAM_OPTION_PTR;
  }

  if( boost::algorithm::icontains($3,"size*") == true )
  {
    std::string::size_type i;
    std::string foo($3);
    i = foo.find_last_of('*');
    if( i != std::wstring::npos )
    {
      foo.erase(0,i+1);
    }
    param.size_of_object = foo;
  }

  params.push_back(param);
}
|      T_IDL T_PARAM ParamOptions T_LEFT_PAREN T_STRING_LITERAL T_STRING_LITERAL T_RIGHT_PAREN T_STRING_LITERAL
{
  idl_param param;
  param.name = $8;
  param.type = $6;
  param.type_qualifier = $5;
  param.options = 0x0;

  if( boost::algorithm::icontains($3,"out") == true )
  {
    param.options |= IDL_PARAM_OPTION_OUT;
  }
  if( boost::algorithm::icontains($3,"in") == true &&
      boost::algorithm::icontains($3,"*") == false )
  {
    param.options |= IDL_PARAM_OPTION_IN;
  }
  if( boost::algorithm::icontains($3,"alloc") == true )
  {
    param.options |= IDL_PARAM_OPTION_ALLOC;
  }

  if( boost::algorithm::icontains($3,"ptr") == true )
  {
    param.options |= IDL_PARAM_OPTION_PTR;
  }

  if( boost::algorithm::icontains($3,"size*") == true )
  {
    std::string::size_type i;
    std::string foo($3);
    i = foo.find_last_of('*');
    if( i != std::wstring::npos )
    {
      foo.erase(0,i+1);
    }
    param.size_of_object = foo;
  }

  params.push_back(param);
}
;

ParamList: ParamList T_COMMA Param
|          Param
;

ParamOptionList: ParamOptionList T_COMMA T_STRING_LITERAL
{
  strcat($$,",");
  strcat($$,$3);
}
|                T_STRING_LITERAL
{
  //fprintf(stdout, "ParamOption(%s)\n", $1);
  strcpy($$,$1);
}
;

ParamOptions: T_LEFT_BRACKET T_RIGHT_BRACKET
{
  strcpy($$,"");
}
|            T_LEFT_BRACKET ParamOptionList T_RIGHT_BRACKET
{
  //fprintf(stdout, "ParamOpionts: %s\n", $2);
  strcpy($$,$2);
}
;

MethodPrototype: T_STRING_LITERAL T_LEFT_PAREN ParamList T_RIGHT_PAREN
{
  strcpy($$,$1);
  //fprintf(stdout, "method: %s\n", $1);
}
|     T_STRING_LITERAL T_LEFT_PAREN T_RIGHT_PAREN
{
  strcpy($$,$1);
  //fprintf(stdout, "method: %s\n", $1);
}
;
