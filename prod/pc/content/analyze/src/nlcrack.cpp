/*************************************************************************************
 *
 * NextLabs Content Analysis Tool
 *
 ************************************************************************************/
#include <windows.h>
#ifdef NLCA_PARALLEL_SEARCH
#  include <omp.h>
#endif
#include <sddl.h>
#include <aclapi.h>
#include <list>
#include <string>
#include <iostream>
#include <exception>

#include "nl_content_analysis.hpp"          // SearchText,RedactText,etc.
#include "nl_content_cracker_ifilter.hpp"   // IFilter extraction
#include "nl_content_cracker_strings.hpp"   // Raw (UNIX strings) extraction
#include "nlca_client.h"                    // NLCA Client

typedef struct
{
  const wchar_t* expr_key;   // Expression key (i.e. @ccn)
  const wchar_t* expr_name;  // Expression name
  const wchar_t* expr_desc;  // Expression description
  const wchar_t* expr_value; // Expression value (regex)
} NLCrackerExpression;

/* nlcrack_exps
 *
 * Default expressions/match criteria.  This *must* be synchronized with ../configuration/attributes.xml
 */
const NLCrackerExpression nlcrack_exps[] =
  {
    { L"@ccn"      , L"CCN"            , L"Credit Card Number"     ,
      L"\\b\\d{4}(\\s|[-]){0,1}\\d{4}(\\s|[-]){0,1}\\d{2}(\\s|[-]){0,1}\\d{2}(\\s|[-]){0,1}\\d{1,4}\\b" },

    { L"@currency" , L"CurrencyValue"  , L"Currency Value"         ,
      L"([\\x{0024}\\x{00a2}-\\x{00a5}\\x{20a1}-\\x{20cf}])(\\s)*((([-(]){0,1}\\d{1,3}([,.]\\d{3})*([,.]\\d{1,2}){0,1}[)]{0,1})|([,.]\\d{1,2}))" },

    { L"@phone"    , L"PhoneNumber"    , L"Phone Number"           ,
      L"(([(]{0,1}\\d{3}([).-]|\\s)\\s{0,10}\\d{3}([-.]|\\s)\\d{4})|(\\b\\d{3}([.-]|\\s)\\d{4}))\\b" },

    { L"@ssn"      , L"SSN"            , L"Social Security Number" ,
      L"\\b\\d{3}([- ]){0,1}\\d{2}([- ]){0,1}\\d{4}\\b" },

    { L"@ip4"      , L"IPv4Address"    , L"IPv4 Address"           ,
      L"\\b((2[0-4]\\d)|(25[0-5])|(1{0,1}\\d{1,2}))([.]((2[0-4]\\d)|(25[0-5])|(1{0,1}\\d{1,2}))){3}(/\\d{1,2}){0,1}\\b" },

    { L"@email"    , L"EmailAddress"   , L"Email Address"          ,
      L"\\b\\w(\\w|[.+#$!-])*@(\\w+\\.){1,3}\\w{2,6}\\b" },

    { L"@date"     , L"Date"           , L"Date (i.e. DOB)"        ,
      L"\\b\\d{1,2}\\s*/\\s*\\d{1,2}\\s*/\\s*(\\d{4}|\\d{2})|((Jan|Feb|Mar|Apr|May|Jun|Jul|Aug|Sep|Oct|Nov|Dec)\\w{0,6}(\\s)+\\d{1,2}(st|nd|rd|th){0,1}(\\s)*([,]){0,1}\\s*\\d{4})|(\\d{1,2}(st|nd|rd|th){0,1}(\\s)*(Jan|Feb|Mar|Apr|May|Jun|Jul|Aug|Sep|Oct|Nov|Dec)\\w{0,6}\\s*[,]{0,1}\\s*\\d{4})\\b" },

    { L"@address"  , L"MailingAddress" , L"Mailing Address"        ,
      L"\\b(AL|AK|AS|AZ|AR|CA|CO|CT|DE|DC|FM|FL|GA|GU|HI|ID|IL|IN|IA|KS|KY|LA|ME|MH|MD|MA|MI|MN|MS|MO|MT|NE|NV|NH|NJ|NM|NY|NC|ND|MP|OH|OK|OR|PW|PA|PR|RI|SC|SD|TN|TX|UT|VT|VI|VA|WA|WV|WI|WY)(\\s)*\\d{5}(\\s|[-]\\d{4}){0,1}\\b" }
  };/* nlcrack_exps */

/** GetExpressionTypeFromValue
 *
 *  \brief Match the expression value (regex) to the type of expression.  This occurs only when
 *         the expression value is known and from the above table.
 *
 *  \return NULL when not found, otherwise the expression name (type).
 */
static const wchar_t* GetExpressionTypeFromValue( const wchar_t* expr_value )
{
  for( int i = 0 ; i < _countof(nlcrack_exps) ; i++ )
  {
    if( wcscmp(expr_value,nlcrack_exps[i].expr_value) == 0 )
    {
      return nlcrack_exps[i].expr_name;
    }
  }
  return NULL;
}/* GetExpressionName */

/** DisplayContent
 *
 *  \brief Display the content of a file through its NLCA::ContentCracker interface.
 *
 *  \param cracker (in) Cracker instance for file.
 *  \param rx (in-out)  Expressions to search for.
 *  \param redact (in)  Redact expression matches.
 */
void DisplayContent( NLCA::ContentCracker& cracker , NLCA::Expressions& rx , bool redact )
{
  bool eos = false;
  std::wstring ws_buf;
  ws_buf.reserve(NLCA::CHUNK_SIZE);
  wchar_t* buf = new wchar_t[NLCA::CHUNK_SIZE];
  for( ; eos == false ; )
  {
    unsigned long buf_size = NLCA::CHUNK_SIZE;;
    if( cracker.GetText(buf,buf_size,eos) == false )
    {
      break;
    }
    if( buf_size == 0 )
    {
      continue;
    }

    ws_buf = buf;
    if( rx.GetExpressions().size() > 0 )
    {
      /*************************************************************
       * Analyze text for redaction support
       ************************************************************/
      if( NLCA::SearchText<wchar_t>(ws_buf,rx) == true && redact == true )
      {
	NLCA::RedactText<wchar_t>(ws_buf,rx);
      }
    }

    char* oem_buf = new char[NLCA::CHUNK_SIZE];
    CharToOemW(ws_buf.c_str(),oem_buf); // conver to console character set
    std::wcout << oem_buf;
    delete [] oem_buf;
  }/* for */

  delete [] buf;
}/* DisplayContent */

/** nlcrack_test
 *
 *  \brief Perform self-test of Analyze interface.
 *
 *  \return true on successful pass of all test, otherwise false.
 */
bool nlcrack_test(void)
{
  std::wcout << L"nlcrack_test: performing self test\n";

  std::wcout << L"nlcrack_test: checking bad expression\n";
  NLCA::Expression* bad_exp = new NLCA::Expression();
  bad_exp->SetExpression(L"\\d{1,2}}");
  bad_exp->SetRequiredMatchCount((size_t)-1);
  delete bad_exp;
  std::wcout << L"nlcrack_test: checking bad expression: passed\n";

  /* There are a total of 4 matches for "Shrub\w+" */
  std::string   ainput("ROGER:  Yes.  Shrubberies are my trade.  I am a shrubber.  My name is 'Roger the Shrubber'.  I arrange, design, and sell shrubberies.");
  std::wstring winput(L"ROGER:  Yes.  Shrubberies are my trade.  I am a shrubber.  My name is 'Roger the Shrubber'.  I arrange, design, and sell shrubberies.");
  std::string   aoutput("ROGER:  Yes.  XXXXXXXXXXX are my trade.  I am a XXXXXXXX.  My name is 'Roger the XXXXXXXX'.  I arrange, design, and sell XXXXXXXXXXX.");
  std::wstring woutput(L"ROGER:  Yes.  XXXXXXXXXXX are my trade.  I am a XXXXXXXX.  My name is 'Roger the XXXXXXXX'.  I arrange, design, and sell XXXXXXXXXXX.");

  /****************************************************************************
   * AnalyzeText (w/o redaction)
   ***************************************************************************/
  std::wcout << L"nlcrack_test: NLCA::SearchText without redaction: testing\n";
  NLCA::Expression* exp;
  size_t amatch_count = 0;
  size_t wmatch_count = 0;
  exp = new NLCA::Expression();
  exp->SetExpression(L"Shrub\\w+");
  NLCA::SearchText<char>(ainput,*exp);
  amatch_count = exp->MatchCount();

  exp->Clear();
  exp->SetExpression(L"Shrub\\w+");
  NLCA::SearchText<wchar_t>(winput,*exp);
  wmatch_count = exp->MatchCount();

  if( amatch_count != 4 || wmatch_count != 4 || amatch_count != wmatch_count )
  {
    fprintf(stdout, "nlcrack_test: NLCA::SearchText without redaction failed: match count inequality (%d %d %d)\n",
	    amatch_count, wmatch_count, 4);
    delete exp;
    return false;
  }
  std::wcout << L"nlcrack_test: NLCA::SearchText without redaction: passed\n";

  for( int i = 0 ; i < 10 ; i++ )
  {
    exp->Clear();
    exp->SetExpression(L"Shrub\\w+");
    exp->SetRequiredMatchCount(i);
    NLCA::SearchText<wchar_t>(winput,*exp);

    /* Results:
     *    (0,4] match count must be satisfied since there are 4 matches for Shrub\w+.
     *    (4,9] match count must not be satisified.
     *    [0]   match count must not be satisfied.
     */
    switch(i)
    {
      case 1:
      case 2:
      case 3:
      case 4:
	if( exp->Satisfied() == false )
	{
	  std::wcout << L"nlcrack_test: NLCA::SearchText without redaction: failed match count (less than)\n";
	}
	break;
      case 5:
      case 6:
      case 7:
      case 8:
      case 9:
      case 0:
	if( exp->Satisfied() == true )
	{
	  std::wcout << L"nlcrack_test: NLCA::SearchText without redaction: failed match count\n";
	}
	break;
    }
  }
  delete exp;

  /****************************************************************************
   * AnalyzeText (w/redaction)
   ***************************************************************************/
  std::wcout << L"nlcrack_test: AnalyzeText with redaction: testing\n";
  std::string redact_ainput(ainput);
  std::wstring redact_winput(winput);
  exp = new NLCA::Expression();
  exp->SetExpression(L"Shrub\\w+");
  exp->SetRequiredMatchCount(10);
  NLCA::AnalyzeText<char>(redact_ainput,*exp,true);
  delete exp;

  exp = new NLCA::Expression();
  exp->SetExpression(L"Shrub\\w+");
  exp->SetRequiredMatchCount(10);
  NLCA::AnalyzeText<wchar_t>(redact_winput,*exp,true);
  delete exp;

  if( redact_ainput != aoutput || redact_winput != woutput )
  {
    fprintf(stdout, "nlcrack_test: NLCA::RedacText with redaction failed: textual inequality\n");
    return false;
  }
  std::wcout << L"nlcrack_test: NLCA::RedactText with redaction: passed\n";
  std::cout << "nlcrack_test: all test passed\n";

  return true;
}/* nlcrack_test */

void display_help(void)
{
  fprintf(stdout, "NextLabs Content Analysis Cracking Tool (Built %s %s)\n",__DATE__,__TIME__);
  fprintf(stdout, "usage: nlcrack [file] [option] ...\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "options\n");
  fprintf(stdout, "  --help                    This screen\n");
  fprintf(stdout, "  --regex=[regex],[count]   Match regex with required match count.\n");
  fprintf(stdout, "                            A match count of 0 never satisfied.  The\n");
  fprintf(stdout, "                            entire file is searched.\n");
  fprintf(stdout, "       [regex] may be one of the followin default values:\n");
  for( int i = 0 ; i < _countof(nlcrack_exps) ; i++ )
  {
    fprintf(stdout, "                            %-10ws %ws\n", nlcrack_exps[i].expr_key, nlcrack_exps[i].expr_desc);
  }
  fprintf(stdout, "  --redact                  Redact matches with 'X'\n");
  fprintf(stdout, "  --display                 Display content\n");
  fprintf(stdout, "  --strings, --raw          Use 'strings' or 'raw' interpretation\n");
  fprintf(stdout, "  --time                    Elapsed runtime in milliseconds\n");
  fprintf(stdout, "  --test                    Perform self test for integrity\n");
  fprintf(stdout, "  --service                 Use NLCA Service\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "examples:\n");
  fprintf(stdout, "  nlcrack foo.txt                       Crack file contents to stdout\n");
  fprintf(stdout, "  nlcrack foo.txt --regex=\"hello\",0     (match \"hello\")\n");
  fprintf(stdout, "  nlcrack foo.txt --regex=\"\\d{4}\",10    (match XXXX digits x10)\n");
  fprintf(stdout, "  nlcrack foo.txt --regex=@ccn,10       (CCN x10)\n");
  fprintf(stdout, "  nlcrack foo.txt --regex=@currency,20  (CurrencyValues x20)\n");
}/* display_help */

int wmain( int argc, wchar_t** argv )
{
#ifdef NLCA_PARALLEL_SEARCH
  omp_set_num_threads(2);
#endif

  if( argc == 2 && wcscmp(argv[1],L"--test") == 0 ) /* self test */
  {
    nlcrack_test();
    return 0;
  }

  if( argc < 2 || (argc == 2 && wcscmp(argv[1],L"--help") == 0) )
  {
    display_help();
    return 0;
  }

  /********************************************************************************
   * Options
   ********************************************************************************/
  bool op_strings = false;  /* strings interpretation */
  bool op_display = false;  /* display file contents */
  bool op_redact  = false;  /* redact expression matches */
  bool op_time    = false;  /* elapsed time in ms */
  bool op_service = false;  /* use nlca service? */

  if( argc == 2 ) /* display by default when only file is specified */
  {
    op_display = true;
  }

  std::list< std::pair<std::wstring,int> > regex_list;
  for( int i = 1 ; i < argc ; ++i )
  {
    if( wcscmp(argv[i],L"--strings") == 0 ||    /* 'strings' interpretation */
	wcscmp(argv[i],L"--raw") == 0 )
    {
      op_strings = true;
      continue;
    }
    if( wcscmp(argv[i],L"--display") == 0 )     /* display file */
    {
      op_display = true;
      continue;
    }
    if( wcscmp(argv[i],L"--redact") == 0 )      /* redaction */
    {
      op_redact = true;
      op_display = true; // redaction implies display
      continue;
    }
    if( wcscmp(argv[i],L"--time") == 0 )        /* elapsed time */
    {
      op_time = true;
      continue;
    }
    if( wcscmp(argv[i],L"--service") == 0 )     /* use service? */
    {
      op_service = true;
      continue;
    }
    if( wcsncmp(argv[i],L"--regex",_countof(L"--regex")-1) == 0 )      /* expression */
    {
      std::wstring p(argv[i]);

      std::wstring::size_type ri = p.find_first_of(L'=');
      std::wstring::size_type rj = p.find_last_of(L',');
      std::wstring exp;
      std::wstring exp_count(L"0");  // default of zero

      if( ri == std::wstring::npos )
      {
	fprintf(stderr, "nlcrack: invalid use of '--regex'.  Try 'nlcrack --help' for more information.\n");
	return 1;
      }

      exp = p.substr(ri,rj-ri);         // expression
      exp.erase(0,1);

      if( rj != std::wstring::npos )  // optional expression count
      {
	exp_count = p.substr(rj);
	exp_count.erase(0,1);
      }

      /* Search for default expression tag (i.e. @ccn) */
      for( int exp_i = 0 ; exp_i < _countof(nlcrack_exps) ; exp_i++ )
      {
	if( wcscmp(nlcrack_exps[exp_i].expr_key,exp.c_str()) == 0 )
	{
	  exp = nlcrack_exps[exp_i].expr_value;
	}
      }

      /* construct <exp,exp_count> pair */
      std::pair<std::wstring,int> rx;
      rx.first = exp;
      rx.second = _wtoi(exp_count.c_str());
      regex_list.push_back(rx);
      continue;
    }
  }/* for */

  /***********************************************************************
   * Construct regular expressions from parameter list.
   **********************************************************************/
  NLCA::Expressions rx;
  for( std::list< std::pair<std::wstring,int> >::iterator it = regex_list.begin() ;
       it != regex_list.end() ; ++it )
  {
    const wchar_t* expr_type = GetExpressionTypeFromValue(it->first.c_str());
    NLCA::Expression* exp = NLCA::ConstructExpression(expr_type);
    exp->SetExpression(it->first);
    exp->SetRequiredMatchCount(it->second);
    rx.Add(exp);
  }/* for */

  if( op_service == true )
  {
    std::list<NLCA::Expression*> elist;

    for( NLCA::ExpressionList::const_iterator it = rx.GetExpressions().begin() ; it != rx.GetExpressions().end() ; ++it )
    {
      elist.push_front(*it);
    }/* for rx */

    HMODULE hmod = LoadLibraryA("nlca_client.dll");
    if( hmod != NULL )
    {
      typedef bool (*NLCAService_SearchFile_t)( const wchar_t* , int , std::list<NLCA::Expression *> &exps , int );
      NLCAService_SearchFile_t pfn_SearchFile = (NLCAService_SearchFile_t)GetProcAddress(hmod,"NLCAService_SearchFile");
      if( pfn_SearchFile == NULL || pfn_SearchFile(argv[1],(int)GetCurrentProcessId(),elist,1000*1000) == false )
      {
	fprintf(stderr, "nlcrack: NLCAService::AnalyzeFile failed\n");
      }
      else
      {
	for( NLCA::ExpressionList::const_iterator it = rx.GetExpressions().begin() ; it != rx.GetExpressions().end() ; ++it )
	{
	  const NLCA::Expression& exp = **it;
	  fprintf(stdout, "[%ws] matched %d of %d : satisfied = %s\n",
		  exp.RegexString().c_str(),
		  exp.MatchCount(), exp.RequiredMatchCount(),
		  exp.Satisfied() == true ? "true" : "false");
	}/* for rx */
      }
      FreeLibrary(hmod);
    }
    return 0;
  }/* if op_service */

  CoInitializeEx(0,COINIT_MULTITHREADED);

  /********************************************************************************
   * Crack file contents
   ********************************************************************************/
  NLCA::ContentCracker* cracker = NULL;
  if( op_strings == true )  /* force 'strings' interpretation */
  {
    try
    {
      cracker = new NLContentCracker_Strings(argv[1]);
    }
    catch( ... )
    {
      fprintf(stderr, "nlcrack: cannot extract text from %ws (raw)\n", argv[1]);
      return 0;
    }
  }
  else
  {
    try
    {
      cracker = new NLContentCracker_IFilter(argv[1]);  /* use default of IFilter */
    }
    catch( HRESULT& hr )
    {
      fprintf(stderr, "nlcrack: cannot extract text from %ws (IFilter) (0x%x)\n", argv[1], hr);
      return 0;
    }
  }

  /***********************************************************************
   * Crack file?
   **********************************************************************/
  if( op_display == true ) /* Display content? */
  {
    DisplayContent(*cracker,rx,op_redact);
    return 0;
  }

  // are all match requirements satisfied?
  bool all_exp_satisfied = true;

  DWORD start_time = GetTickCount(); /* start time */
  /************************************************************************
   * Search the file with respect to the regex list
   ***********************************************************************/
  if( NLCA::SearchFile(*cracker,rx) == true )
  {
    DWORD runtime = GetTickCount() - start_time;

    if( op_time == true )
    {
      fprintf(stdout, "Runtime %d ms\n", runtime);
    }

    for( NLCA::ExpressionList::const_iterator it = rx.GetExpressions().begin() ;
	 it != rx.GetExpressions().end() ; ++it )
    {
      const NLCA::Expression& exp = **it;

      fprintf(stdout, "[%ws] matched %d of %d : satisfied = %s\n",
	      exp.RegexString().c_str(),
	      exp.MatchCount(), exp.RequiredMatchCount(),
	      exp.Satisfied() == true ? "true" : "false");

      // current expression satisfied - failure will cause all failure
      all_exp_satisfied = all_exp_satisfied && exp.Satisfied();
    }/* for rx */
  }/* if */

  delete cracker;

  CoUninitialize();

  // return value indicates match requirements satisfied or not
  return (all_exp_satisfied == true ? 0 : 1);
}/* main */
