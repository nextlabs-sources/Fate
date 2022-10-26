/****************************************************************************************************
 *
 * NLCA Content Redaction Demo
 *
 * This source file demonstrates how content redaction can be implemented using the NLCA interface.
 * Specifically, the use of NLCA::Expression and NLCA::Expressions is used with search and replace
 * methods of NLCA::SearchAndReplace.
 *
 * See nl_content_analysis.hpp for detailed information about these mthods.
 *
 ***************************************************************************************************/

#include <iostream>
#include <fstream>

#include "nl_content_analysis.hpp"

/** expression
 *
 *  \brief This structure represents the information required to construct an expression
 *         object for use with NLCA interface such as NLCA::SearchText.
 *
 *  Struct Member       Obligation Name (key-value)
 *  -----------------------------------------------
 *  name                RegexType
 *  regex               Regex
 *  match_count         With Match Count >=
 */
typedef struct
{
  const wchar_t* name;    // Opaque name used to construct NLCA::Expression instance
  const wchar_t* regex;   // Regular expression literal string (i.e. "\d{16}")
  size_t match_count;     // Match count required for redaction
} expression;

/** do_redaction
 *
 *  \brief Perform redaction of the given text with respect to the given expressions.
 *
 *  \param text_to_redact (in-out) Text to search and redact with respect to expressions.
 *  \param exps (in)               Expressions to search for and redact matches.
 *  \param num_exps (in)           Number of expressions.
 *  \param text_changed (out)      Was the text (text_to_redact) changed?
 *
 *  \return true if text was changed as a result of redaction request, other wise false.
 */
static bool do_redaction( std::wstring& text_to_redact , const expression* exps , int num_exps , bool& text_changed )
{
  bool result = false;
  /*******************************************************************************
   * Construct set of expressions.  NLCA::Expressions own expressions objects
   * given by NLCA::Expressions::Add(), so there is no need to 'delete' them.
   ******************************************************************************/
  NLCA::Expressions rx;
  for( int i = 0 ; i < num_exps ; i++ )
  {
    NLCA::Expression* exp = NLCA::ConstructExpression(exps[i].name); // construct new expression object
    exp->SetExpression(exps[i].regex);                               // set expression from option
    exp->SetRequiredMatchCount(exps[i].match_count);                 // match count required
    rx.Add(exp);
  }/* for */

  /*******************************************************************************
   * Search text and redact any satisfied matches.
   ******************************************************************************/
  if( NLCA::SearchAndRedactText(text_to_redact,rx,text_changed) == false )
  {
    fprintf(stderr, "do_redaction: NLCA::SearchAndText failed\n");
    return false;
  }
  return result;
}/* do_redaction */

/* Demonstrate how to redact text based on a set of match criteria.
 *
 * (1) Construct expression set in do_redaction()
 * (2) Use SearchAndRedactText in do_redaction() to perform match and redaction.
 *
 *  input.txt is the text input.
 *  output.txt is the output to expect.
 */
int main(void)
{
  /* This information would come from an obligation.  There are not 100 matches of "name1" expression
   * so matches for that expression are not redacted.  Recall the match count must be satisfied
   * before any content can be changed (redacted).
   */
  const expression exps[] =
    {
      { L"name1" , L"th\\w+"  , 100 } , // words that begin with 'th'
      { L"name2" , L"\\w+\\:" , 2 } ,   // word with ':' suffix
      { L"name3" , L"king"    , 1}      // 'king' plain text
    };

  std::wfstream file(L"input.txt",std::ios_base::in); // open read-only
  if( file.is_open() == false )
  {
    std::wcerr << "cannot open input.txt\n";
    return 1;
  }

  /* read file into text */
  wchar_t text[1024];
  file.read(text,_countof(text));
  file.close();

  /* use string for redaction */
  std::wstring text_to_redact(text);

  fprintf(stdout, "Before\n");
  fprintf(stdout, "-----------------------------------------------------------------\n");
  fprintf(stdout, "%ws", text_to_redact.c_str());
  fprintf(stdout, "\n");

  bool changed = false;
  bool result = false;
  result = do_redaction(text_to_redact,&exps[0],_countof(exps),changed);
  if( changed == true )
  {
    fprintf(stdout, "text was redacted\n");
  }

  fprintf(stdout, "After\n");
  fprintf(stdout, "-----------------------------------------------------------------\n");
  fprintf(stdout, "%ws", text_to_redact.c_str());

  return 0;
}/* main */
