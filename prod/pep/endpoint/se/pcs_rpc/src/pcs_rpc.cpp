
#include <cstdio>
#include <cstdlib>
#include <string>
#include <list>

#include "pcs_rpc_packer.hpp"

extern int yyparse(void);

typedef struct
{
  int x;
  int y;
} Test_t;

void test_packer(void)
{
  packer in_packer;
  packer out_packer;
  Test_t t;
  int x = 101;
  int y = 102;

  t.x = 1000;
  t.y = 100;

  in_packer.pack(&x);
  in_packer.pack(&y);
  in_packer.pack("If there's a new way, I'll be the first in line. But, it better work this time.");
  in_packer.pack(__FILE__);
  in_packer.pack(&t);
  in_packer.pack(&x);
  in_packer.pack(&y);
  in_packer.set_return(CE_RESULT_VERSION_MISMATCH);

  fprintf(stdout, "%d bytes : %s\n", in_packer.size(), in_packer.get_coded_string().c_str());

  void* arg = NULL;
  size_t size = 0;

  out_packer.assign_code_string(in_packer.get_coded_string().c_str());

  arg = out_packer.at(0,&size);
  fprintf(stdout, "arg 0: %d\n", *( (int*)arg ) );

  arg = out_packer.at(1,&size);
  fprintf(stdout, "arg 1: %d\n", *( (int*)arg ) );

  arg = out_packer.at(2,&size);
  fprintf(stdout, "arg 2: %s (%d)\n", ( (char*)arg ) , size);

  arg = out_packer.at(3,&size);
  fprintf(stdout, "arg 3: %s (%d)\n", ( (char*)arg ) , size);

  arg = out_packer.at(4,&size);
  memset(&t,0x00,sizeof(t));
  memcpy(&t,arg,sizeof(t));
  Test_t* pt = (Test_t*)&t;
  fprintf(stdout, "arg 4: %d , %d (%d)\n", pt->x , pt->y , size);

  arg = out_packer.at(5,&size);
  fprintf(stdout, "arg 5: %d (%d)\n", ( *(char*)arg ) , size);

  arg = out_packer.at(6,&size);
  fprintf(stdout, "arg 6: %d (%d)\n", ( *(int*)arg ) , size);

  fprintf(stdout, "return %d\n", out_packer.get_return());

  fprintf(stdout, "IN : %d bytes : %s\n", in_packer.size(), in_packer.get_coded_string().c_str());

  fprintf(stdout, "OUT: %d bytes : %s\n", out_packer.size(), out_packer.get_coded_string().c_str());

  fprintf(stdout, "done\n");

}/* test_packer */

std::string op_file;      /* file prefix */
std::string op_service;   /* server name */
bool op_verbose = false;  /* verbose? */
bool op_verify  = false;  /* generate verification (fake) servier code? */

void print_help(void)
{
  fprintf(stdout, "pcs_idl (Built %s %s)\n",__DATE__,__TIME__);
  fprintf(stdout, "\n");
  fprintf(stdout, "  --prefix=[string]                    File prefix\n");
  fprintf(stdout, "  --service=[service]                  Service\n");
  fprintf(stdout, "  --verbose, --v, -v, /v               Verbose\n");
  fprintf(stdout, "  --z, -z, /z                          Verification\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "Examples:\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "\n");
}/* print_help */

int main( int argc , char** argv )
{
  /* Process options */
  for( int i = 1 ; i < argc ; ++i )
  {
    char* option = strstr(argv[i],"=");
    if( option != NULL )
    {
      option++;
    }

    if( strncmp(argv[i],"--prefix=",strlen("--prefix=")) == 0 )
    {
      op_file = option;
      continue;
    }

    if( strncmp(argv[i],"--service=",strlen("--service=")) == 0 )
    {
      op_service = option;
      continue;
    }

    if( strcmp(argv[i],"--verbose") == 0 ||
	strcmp(argv[i],"--v") == 0 ||
	strcmp(argv[i],"-v") == 0 ||
	strcmp(argv[i],"/v") == 0 )
    {
      op_verbose = true;
      continue;
    }

    if( strcmp(argv[i],"--test") == 0 ||
	strcmp(argv[i],"--t") == 0 ||
	strcmp(argv[i],"-t") == 0 ||
	strcmp(argv[i],"/t") == 0 )
    {
      test_packer();
      return 0;
    }

    if( strcmp(argv[i],"--z") == 0 ||
	strcmp(argv[i],"-z") == 0 ||
	strcmp(argv[i],"/z") == 0 )
    {
      op_verify = true;
      continue;
    }

    if( strcmp(argv[i],"--help") == 0 ||
	strcmp(argv[i],"-h") == 0 ||
	strcmp(argv[i],"/?") == 0 )
    {
      print_help();
      return 0;
    }

  }/* for */

  if( op_service.empty() == true )
  {
    fprintf(stderr, "Error: Service is undefined but required.  See option '--service'.\n");
    return 1;
  }

  /* start parsing stdin */
  int parse_result = yyparse();
  if( parse_result )
  {
    fprintf(stderr, "error: parse error\n");
    return 1;
  }
  fprintf(stdout, "complete\n");
  return 0;
}/* main */

int yyerror( char* s )
{
  fprintf(stderr, "ERROR: %s\n", s);
  return 0;
}

extern "C" int yywrap(void)
{
  return 1;
}
