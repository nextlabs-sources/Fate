
%{
#include <cstdio>
#include <cstdlib>
#include "pcs_idl_grammar_tab.hpp"

#pragma warning( disable : 4018 )
#pragma warning( disable : 4996 )

%}

digit         [0-9]
letter        [a-zA-Z]

%%

"("      { return T_LEFT_PAREN; }
")"      { return T_RIGHT_PAREN; }
"["      { return T_LEFT_BRACKET; }
"]"      { return T_RIGHT_BRACKET; }
","      { return T_COMMA; }
"header" { return T_HEADER; }
"method" { return T_METHOD; }
"type"   { return T_TYPE; }
"param"  { return T_PARAM; }
"@idl"   { return T_IDL; }

([a-zA-Z0-9_]|"*"|"&")+ {

  strncpy_s(yylval.string_val,_countof(yylval.string_val),yytext,_TRUNCATE);
  return T_STRING_LITERAL;
}

"//".+ { }
"/*".+"*/" { }

"{"(.|\n)+"}" {
  strncpy_s(yylval.string_val,_countof(yylval.string_val),yytext,_TRUNCATE);

  if( strstr(yytext,"@idl") )
  {
    REJECT;
  }

  return T_BODY;
}

%%
