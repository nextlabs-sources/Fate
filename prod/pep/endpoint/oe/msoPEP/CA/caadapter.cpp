#include "stdafx.h"
#include "caadapter.h"
#include <fstream>
#include <map>
using namespace std;




BOOL CAAdapter::RedactBuffer( std::wstring& text_to_redact , CARegExpressions& regExps , bool& text_changed )
{
	BOOL result = TRUE;
	NLCA::Expressions rx;
	for( size_t i = 0 ; i < regExps.size() ; i++ )
	{
		NLCA::Expression* exp=NULL;
		DP((L"RedactBuffer:%s",regExps[i]->m_Regex.c_str()));
		if(regExps[i]!=NULL&&regExps[i]->m_RegexType.length())
		    exp = NLCA::ConstructExpression(regExps[i]->m_RegexType.c_str()); // construct new expression object
		else
			exp = NLCA::ConstructExpression((wchar_t*)NULL);
		if(exp)
		{
			exp->SetExpression(regExps[i]->m_Regex.c_str());                               // set expression from option
		    exp->SetRequiredMatchCount(regExps[i]->m_MatchCount);                 // match count required
			rx.Add(exp);
		}
  }/* for */

  /*******************************************************************************
   * Search text and redact any satisfied matches.
   ******************************************************************************/
  if( NLCA::SearchAndRedactText(text_to_redact,rx,text_changed) == false )
  {
    fprintf(stderr, "do_redaction: NLCA::SearchAndText failed\n");
    return FALSE;
  }
  return result;
}/* do_redaction */



BOOL CAAdapter::RedactBufferBody( std::wstring& text_to_redact , CARegExpressions& regExps , bool& text_changed ,map<wstring,wstring>& mapReplaceBuf)
{
	BOOL result = TRUE;
	NLCA::Expressions rx;
	for( size_t i = 0 ; i < regExps.size() ; i++ )
	{
		NLCA::Expression* exp=NULL;
		DP((L"RedactBuffer:%s",regExps[i]->m_Regex.c_str()));
		if(regExps[i]!=NULL&&regExps[i]->m_RegexType.length())
		    exp = NLCA::ConstructExpression(regExps[i]->m_RegexType.c_str()); // construct new expression object
		else
			exp = NLCA::ConstructExpression((wchar_t*)NULL);
		if(exp)
		{
			exp->SetExpression(regExps[i]->m_Regex.c_str());                               // set expression from option
		    exp->SetRequiredMatchCount(regExps[i]->m_MatchCount);                 // match count required
			rx.Add(exp);
		}
  }/* for */

  /*******************************************************************************
   * Search text and redact any satisfied matches.
   ******************************************************************************/
  if( NLCA::SearchAndRedactTextEx(text_to_redact,rx,text_changed,mapReplaceBuf) == false )
  {
    fprintf(stderr, "do_redaction: NLCA::SearchAndText failed\n");
    return FALSE;
  }
  return result;
}/* do_redaction */
