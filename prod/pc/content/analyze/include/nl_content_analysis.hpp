/*******************************************************************************
 *
 * NextLabs Content Analysis Interface
 *
 * This interface provides methods to search text using POSIX regular 
 * expressions.  The interfaces allow search of plain text via std::wstring and
 * by file.
 *
 * NLCA::SearchText - Perform analysis of text.
 * NLCA::RedactText - Perform redaction of text.
 * NLCA::SearchAndRedactText
 * NLCA::SearchFile - Perform analysis of a file.
 *
 ******************************************************************************/

#ifndef __NL_CONTENT_ANALYASIS_HPP__
#define __NL_CONTENT_ANALYASIS_HPP__

#include <windows.h>
#ifdef NLCA_PARALLEL_SEARCH
#  include <omp.h>
#endif
#include <string>
#include <list>
#include <map>
using namespace std;

#pragma warning(push)
#pragma warning(disable: 6011 6334 6385 4267)
#include <boost/regex.hpp>
#pragma warning(pop)
#include "nl_content_cracker.hpp"
#include "nl_content_analysis_expr.hpp"

/** NLCA - NextLabs Content Analysis Interface
 */
namespace NLCA
{
  /** ExpressionList
   *
   *  Storage structure for NLCA::Expression
   */
  typedef std::vector<NLCA::Expression*> ExpressionList;

  class Expressions
  {
    public:
      Expressions(void) :
	exps()
      {
      }/* Expressions */

      ~Expressions(void)
      {
	/* Free all expression objects */
	for( NLCA::ExpressionList::const_iterator it = exps.begin() ; it != exps.end() ; ++it )
	{
	  delete *it;
	}
      }/* ~Expressions */

      void Add( NLCA::Expression* exp ) throw()
      {
	exps.push_back(exp);
      }/* Add */
  
      const ExpressionList& GetExpressions(void) const throw()
      {
	return exps;
      }/* GetExpressions */
      
    private:
      NLCA::ExpressionList exps;
  };/* Expressions */

  /** ConstructExpression
   *
   *  \brief Abstract factor for NLCA::Expression objects.
   *
   *  \return An NLCA::Expression expression object created from a possible
   *          subclass of NLCA::Expression.
   */
  static NLCA::Expression* ConstructExpression(const NLCA::Expression::ExpressionType type)
  {
    switch(type) {
    case NLCA::Expression::GENERAL:
      return new NLCA::Expression();
    case NLCA::Expression::CCN:
      return new NLCA::ExpressionCCN();
    case NLCA::Expression::SSN:
      return new NLCA::ExpressionSSN();
    case NLCA::Expression::KEYWORDS:
      return new NLCA::ExpressionKeywords();
    }
    return new NLCA::Expression();
  }/* ConstructExpression */

  static NLCA::Expression* ConstructExpression( const char* in_name = NULL )
  {
    if( in_name == NULL )
    {
      return new NLCA::Expression();
    }
    if( strcmp(in_name,"CCN") == 0 ||                                       // Credit Card Numbers (CCN)
	strcmp(in_name,"RegexType_CCN") == 0 ||
	strcmp(typeid(ExpressionCCN).name(),in_name) == 0 )
    {
      return new NLCA::ExpressionCCN();
    }
    else if( strcmp(in_name,"SSN") == 0 ||                                  // Social Security Numbers (SSN)
	     strcmp(in_name,"RegexType_SSN") == 0 ||
	     strcmp(typeid(ExpressionSSN).name(),in_name) == 0 )
    {
      return new NLCA::ExpressionSSN();
    }
    else if( strcmp(in_name,"Keyword(s)") == 0 ||                           // Keyword(s)
	     strcmp(typeid(ExpressionKeywords).name(),in_name) == 0 )
    {
      return new NLCA::ExpressionKeywords();
    }
    return new NLCA::Expression();
  }/* ConstructExpression */

  /* Wrapper for 'wchar_t*' type */
  static NLCA::Expression* ConstructExpression( const wchar_t* in_name = NULL )
  {
    if( in_name == NULL )
    {
      return new NLCA::Expression();
    }
    char name[128]; 
    _snprintf_s(name,sizeof(name)/sizeof(char),_TRUNCATE,"%ws",in_name);
    return ConstructExpression(name);
  }/* ConstructExpression */

  /** ExpressionSetSatisfied
   *
   *  \brief Determine if the expression set has been satisified.  All expressions must be
   *         satisified for a result of true.
   */
  static bool ExpressionSetSatisfied( const NLCA::Expressions& exps ) throw()
  {
    for( NLCA::ExpressionList::const_iterator it = exps.GetExpressions().begin() ;
	 it != exps.GetExpressions().end() ; ++it )
    {
      if( (*it)->Satisfied() == false ) /* at least one not satisfied? */
      {
	return false;
      }
    }/* for rx */
    return true;
  }/* ExpressionSetSatisfied */

  /** CHUNK_SIZE
   *
   *  \brief Chunk size of text segment to be read by a NLCA interfaces.
   */
  static const size_t CHUNK_SIZE = 32 * 1024;

  /** AnalyzeText
   *
   *  \brief Analyze text with respect an expression.  When specified redact any matched
   *         text such that it is scrubbed from the source.
   *
   *  \param text (in-out) Text to search.  When redact is enabled the string will be
   *                       modified.  Each sub-string match will be struck from the string.
   *  \param rx (in-out)   Expression object.  The state of an expression object may
   *                       change as a result of the search.
   *  \param redact (in)   Redact content with respect to the 'rx' expression set.
   *
   *  \return true on success full completion of search, otherwise false.
   *
   *  \notes Template use for type 'charT' specifies character type.  char and wchar_t may
   *         be used.  May throw std::runtime_error on error.
   */
  template <typename charT>
  static bool AnalyzeText( std::basic_string<charT>& text ,
			   NLCA::Expression& exp ,
			   bool redact = false ) throw()
  {
    std::basic_string<charT>::const_iterator start = text.begin();        /* start of string to search */
    std::basic_string<charT>::const_iterator end   = text.end();          /* end of string to search */
    boost::match_results<std::basic_string<charT>::const_iterator> what;  /* what is found */

    /*********************************************************************************
     * Search for a regex match.  If found, place the matching text in the expression
     * object for the caller.
     ********************************************************************************/
    int pos = 0;              /* current position used for striking */
    boost::basic_regex<wchar_t>& regex = exp.Regex();
    for( ; ; )                /* iterate over text while no faults */
    {
      bool matched = false;
      /*******************************************************************************
       * There are several ways the regex library can throw:
       *   (1) Bad expression.
       *   (2) Reach exponentional runtime during search.
       * Exception is interpreted as failure.
       ******************************************************************************/
      try
      {
	matched = boost::regex_search(start,end,what,regex,boost::match_default);
      }
      catch(...)
      {
	return false;
      }
      if( matched == false ) /* match found? */
      {
	break;  /* no match of regex in [start,end] */
      }
      start = what[0].second;                /* advance past match */
      if( exp.IsValid(what.str()) == false ) /* valid match? */
      {
	continue;
      }
      if( redact == true )                 /* redact content? */
      {
	int len = (int)what.length();      /* length of text to redact */
	pos += (int)what.position();       /* set position to start of match */
	exp.Redact(text,pos,len);          /* perform redaction */
	pos += len;                        /* position for next match */
      }/* if redact */
      exp.AddMatch();                      /* increment match count for expression */
    }/* for */
    return true;
  }/* AnalyzeText */


  static void GetReplaceBufInfo(const std::wstring & text, int pos, int len,map<std::wstring,std::wstring> &mapReplaceBuf )
  {
	  if (text.length() >= pos + len)
	  {
		  std::wstring strNeedReplaceBuf = text.substr(pos,len);
		  std::wstring strReplaceBuf(len,'X');
		  mapReplaceBuf[strNeedReplaceBuf] = strReplaceBuf;
	  }
  }

  /** AnalyzeText
   *
   *  \brief Analyze text with respect an expression.  When specified redact any matched
   *         text such that it is scrubbed from the source.
   *
   *  \param text (in-out) Text to search.  When redact is enabled the string will be
   *                       modified.  Each sub-string match will be struck from the string.
   *  \param rx (in-out)   Expression object.  The state of an expression object may
   *                       change as a result of the search.
   *  \param redact (in)   Redact content with respect to the 'rx' expression set.
   *
   *  \return true on success full completion of search, otherwise false.
   *
   *  \notes Template use for type 'charT' specifies character type.  char and wchar_t may
   *         be used.  May throw std::runtime_error on error.
   */
  template <typename charT>
  static bool AnalyzeTextEx( std::basic_string<charT>& text ,
			   NLCA::Expression& exp ,
			   bool redact ,map<wstring,wstring> &mapReplaceBuf ) throw()
  {
    std::basic_string<charT>::const_iterator start = text.begin();        /* start of string to search */
    std::basic_string<charT>::const_iterator end   = text.end();          /* end of string to search */
    boost::match_results<std::basic_string<charT>::const_iterator> what;  /* what is found */

    /*********************************************************************************
     * Search for a regex match.  If found, place the matching text in the expression
     * object for the caller.
     ********************************************************************************/
    int pos = 0;              /* current position used for striking */
    boost::basic_regex<wchar_t>& regex = exp.Regex();
    for( ; ; )                /* iterate over text while no faults */
    {
      bool matched = false;
      /*******************************************************************************
       * There are several ways the regex library can throw:
       *   (1) Bad expression.
       *   (2) Reach exponentional runtime during search.
       * Exception is interpreted as failure.
       ******************************************************************************/
      try
      {
	matched = boost::regex_search(start,end,what,regex,boost::match_default);
      }
      catch(...)
      {
	return false;
      }
      if( matched == false ) /* match found? */
      {
	break;  /* no match of regex in [start,end] */
      }
      start = what[0].second;                /* advance past match */
      if( exp.IsValid(what.str()) == false ) /* valid match? */
      {
	continue;
      }
      if( redact == true )                 /* redact content? */
      {
	int len = (int)what.length();      /* length of text to redact */
	pos += (int)what.position();       /* set position to start of match */
	GetReplaceBufInfo(text,pos,len,mapReplaceBuf);
	exp.Redact(text,pos,len);          /* perform redaction */
	
	pos += len;                        /* position for next match */
      }/* if redact */
      exp.AddMatch();                      /* increment match count for expression */
    }/* for */
    return true;
  }/* AnalyzeText */

  /* Wrap AnalyzeText for search of a single expression */
  template <typename charT>
  static bool SearchText( std::basic_string<charT>& text , NLCA::Expression& rx ) throw()
  {
    return AnalyzeText<charT>(text,rx,false);  /* search text for matches (w/o redaction) */
  }/* SearchText */

  /* Wrap AnalyzeText for redaction of a set of expressions */
  template <typename charT>
  static bool SearchText( std::basic_string<charT>& text , NLCA::Expressions& rxs ) throw()
  {
    int i = 0;
    int size = (int)rxs.GetExpressions().size();
    NLCA::Expression* exp = NULL;
#ifdef NLCA_PARALLEL_SEARCH
    #pragma omp parallel for private(exp)
#endif
    for( i = 0 ; i < size ; i++ )
    {
      exp = rxs.GetExpressions().at(i);
      AnalyzeText<charT>(text,*exp,false);
    }/* for rx */
    return true;
  }/* SearchText */

  /* Wrap AnalyzeText for redaction of a single expression */
  template <typename charT>
  static bool RedactText( std::basic_string<charT>& text , NLCA::Expression& rx ) throw()
  {
    return AnalyzeText<charT>(text,rx,true); /* redact all content matches */
  }/* RedactText */


  /* Wrap AnalyzeText for redaction of a single expression */
  template <typename charT>
  static bool RedactTextEx( std::basic_string<charT>& text , NLCA::Expression& rx ,map<wstring,wstring>& mapReplaceBuf) throw()
  {
	  return AnalyzeTextEx<charT>(text,rx,true,mapReplaceBuf); /* redact all content matches */
  }/* RedactText */


  /* Wrap AnalyzeText for redaction of a set of expressions */
  template <typename charT>
  static bool RedactText( std::basic_string<charT>& text , NLCA::Expressions& rxs ) throw()
  {
    for( NLCA::ExpressionList::const_iterator it = rxs.GetExpressions().begin() ;
	 it != rxs.GetExpressions().end() ; ++it )
    {
      if( RedactText<charT>(text,**it) == false ) /* redact all content matches */
      {
	break; // RedactText failed
      }
    }/* for rx */
    return true;
  }/* RedactText */

  /** SearchAndRedactText
   *
   *  \brief Search text and redact matches.
   *
   *  \param text (in-out)      Text to search and redact.
   *  \param rxs (in-out)       Expressions to search for.
   *  \param text_changed (out) Text was changed (content was redacted).
   *
   *  \retun true on successful completion of search and redact, otherwise false.
   */
  template <typename charT>
  static bool SearchAndRedactText( std::basic_string<charT>& text ,
				   NLCA::Expressions& rxs ,
				   bool& text_changed ) throw()
  {
    text_changed = false; // no change by default
    if( SearchText<charT>(text,rxs) == false ) // search text for matches w/respect to rxs
    {
      return false;
    }
    for( NLCA::ExpressionList::const_iterator it = rxs.GetExpressions().begin() ;
	 it != rxs.GetExpressions().end() ; ++it )
    {
      if( (*it)->Satisfied() == true ) // match criteria was satisfied?
      {
	text_changed = true;
	if( RedactText<charT>(text,**it) == false )  // redact match
	{
	  return false;
	}
      }
    }/* for rx */
    return true;
  }/* SearchAndRedactText */


  template <typename charT>
  static bool SearchAndRedactTextEx( std::basic_string<charT>& text ,
	  NLCA::Expressions& rxs ,
	  bool& text_changed ,map<wstring,wstring>& mapReplaceBuf) throw()
  {
	  text_changed = false; // no change by default
	  if( SearchText<charT>(text,rxs) == false ) // search text for matches w/respect to rxs
	  {
		  return false;
	  }
	  for( NLCA::ExpressionList::const_iterator it = rxs.GetExpressions().begin() ;
		  it != rxs.GetExpressions().end() ; ++it )
	  {
		  if( (*it)->Satisfied() == true ) // match criteria was satisfied?
		  {
			  text_changed = true;
			  if( RedactTextEx<charT>(text,**it,mapReplaceBuf) == false )  // redact match
			  {
				  return false;
			  }
		  }
	  }/* for rx */
	  return true;
  }/* SearchAndRedactText */

  /** SearchFile
   *
   *  \brief Analyze a file with respect to the expression list.
   *
   *  \param filename (in) Full path of file name to search.
   *  \param rx (out)      Expression object list.
   *
   *  \return true on success full completion of search, otherwise false.  Once
   *          any match criteria has been satisfied the search will stop.
   *
   *  \notes May throw std::runtime_error on error.
   */
  static bool SearchFile( NLCA::ContentCracker& cracker ,
			  NLCA::Expressions& rx )
  {
    bool eos = false;
    std::wstring ws_buf;
    ws_buf.reserve(NLCA::CHUNK_SIZE);
    wchar_t* buf = new wchar_t[NLCA::CHUNK_SIZE];

    //DWORD total_time = 0;
    for( ; eos == false ; ) /* read from cracker until end of stream (eos) */
    {
      unsigned long buf_size = NLCA::CHUNK_SIZE;
      if( cracker.GetText(buf,buf_size,eos,NLCA::CHUNK_SIZE) == false )
      {
	    delete[] buf;
		return false;
      }

      if( buf_size == 0 )  /* zero is not an error - may be between segments */
      {
		continue;
      }

      ws_buf = buf;
      //DWORD start_time = GetTickCount();
      if( NLCA::SearchText<wchar_t>(ws_buf,rx) == false )
      {
		break;
      }  
      if( ExpressionSetSatisfied(rx) == true )   /* search satisfied? */
      {
		break;
      }
      //total_time += (GetTickCount() - start_time);
    }/* for */
    //fprintf(stdout, "TOTAL TIME %d\n", total_time);
    delete [] buf;
    return true;
  }/* SearchFile */

}; /* NLCA */

#endif /* __NL_CONTENT_ANALYASIS_HPP__ */
