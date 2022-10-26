/***************************************************************************************
 *
 * nl_content_analysis_expr.hpp
 *
 * This file implements the NLCA::Expression set of classes.  A base class for the
 * expression interface is defined and several custom implementation are provided.
 *
 **************************************************************************************/

#ifndef __NL_CONTENT_ANALYSIS_EXPR_HPP__
#define __NL_CONTENT_ANALYSIS_EXPR_HPP__

#include <windows.h>
#include <cstdio>
#include <string>
#include <boost/regex.hpp>

namespace NLCA
{
  /** Expression
   *
   *  Regular expression object.  Contains regular expression and context 
   *  information
   *  for that expression with respect to a search.
   *
   *  Expression searches match regardless of case via boost::regex::icase.
   */
  class Expression
  {
    public:
      typedef enum {GENERAL, CCN, SSN, KEYWORDS} ExpressionType;
      Expression(void) :
	rx_string(),                /* expression string */
	match_count(0),             /* running match count */
	required_match_count(0),    /* no required match - search all */
	type(GENERAL)
      { }/* Expression */

      virtual ~Expression(void)
      {
      }/* ~Expression */

      ExpressionType GetType() const throw() {return type;}

      /** SetExpression
       *
       *  \brief Set the regular expression.  A derived class may 
       */
      virtual void SetExpression( const std::wstring& in_string ) throw()
      {
	SetExpressionRaw(in_string);
      }/* SetExpression */

      /** SetExpressionRaw
       *
       *  \brief Set the regular expression directly.  There is never a change to
       *         the expression string.
       */
      void SetExpressionRaw( const std::wstring& in_string ) throw()
      {
	rx_string = in_string;  // expression in literal format
	boost::regex::flag_type flags = boost::regex::icase | boost::regex::no_except;
	rx_boost.assign(in_string,flags);  // construct regular expression
      }/* SetExpression */

      /** Redact
       *
       *  \brief Redact text of a string.  This is the default redaction policy.
       *         Derive from NLCA::Expression to override this.
       *
       *  \param text (in-out) Full text.
       *  \param pos (in)      Position of text to redact.
       *  \param len (in)      Length of text to redact.
       */
      virtual void Redact( std::wstring& text , size_t pos , size_t len ) throw()
      {
	text.replace(pos,len,len,L'X');  /* replace [pos,pos+len] with 'X' */
      }/* Redact */

      virtual void Redact( std::string& text , size_t pos , size_t len ) throw()
      {
	text.replace(pos,len,len,'X');   /* replace [pos,pos+len] with 'X' */
      }/* Redact */

      /** IsValid
       *
       *  \brief Is the matching text valid for the search criteria.  For example,
       *         credit card numbers require validation of data structure.
       *  \param text (in) Matched text.
       *  \return true if the match is valid, otherwise false.
       */
      virtual bool IsValid( const std::wstring& text ) const throw()
      {
          UNREFERENCED_PARAMETER(text);
	return true;
      }/* IsValid */

      /** IsValid
       *
       *  \brief By default wrap char version to wchar_t to derived classes.
       */
      virtual bool IsValid( const std::string& text ) const throw()
      {
	std::wstring ws_text(text.length(),L' ');
	std::copy(text.begin(),text.end(),ws_text.begin());
	return IsValid(ws_text); /* Expression::IsValid(std::wstring) */
      }/* IsValid */

      /** Clear
       *
       *  \brief Clear the internal state of the expression.
       */
      void Clear(void)
      {
	rx_string.clear();
	match_count = 0;
	required_match_count = 0;
      }/* Clear */

      /** SetRequiredMatchCount
       *
       *  \brief Set the required match count to satisfy the search.  The default
       *         value is zero (0) which indicates there is no required match count.
       */
      void SetRequiredMatchCount( size_t in_required_match_count ) throw()
      {
	required_match_count = in_required_match_count;
      }/* SetRequiredMatchCount */

      /** Satisfied
       *
       *  \brief Has the match criteria been satisfied for this expression?
       *
       *  \return true if satisfied, otherwise false.
       */
      bool Satisfied(void) const throw()
      {
	if( required_match_count > 0 && match_count >= required_match_count )
	{
	  return true;
	}
	return false;
      }/* Satisfied */

      /** RequiredMatchCount
       *
       *  \brief return required_match_count
       */
      size_t RequiredMatchCount(void) const throw()
      {
	return required_match_count;
      }/* RequiredMatchCount */

      /** AddMatch
       *
       *  \brief Add a match from the regular expression.
       */
      void AddMatch(void) throw()
      {
	match_count++;
      }/* AddMatch */

      /** SetMatch
       *
       *  \brief Set match count
       */
      void SetMatch(size_t m) throw()
      {
	match_count=m;
      }/* SetMatch */

      /** MatchCount
       *
       *  \brief Return the number of expression matches.
       */
      size_t MatchCount(void) const throw()
      {
	return match_count;
      }/* MatchCount */

      /** RegexString
       *
       *  \brief Expression string.
       */
      const std::wstring& RegexString(void) const throw()
      {
	return rx_string;
      }/* RegexString */

      /** Regex
       *
       *  \brief Return a reference to a constructed BOOST regular expression.
       *  \return Reference to constructed BOOST regular expression.
       */
      boost::basic_regex<wchar_t>& Regex(void)
      {
	return rx_boost;
      }/* Regex */

      /** Display
       *
       *  \brief Display the expression to stdout
       */
      void display(){
	if(!rx_string.empty()) {
	  printf("expression type: %d %s\n", type, typeid(*this).name());
	  wprintf(L"           string: %s\n", rx_string.c_str());
          wprintf(L"           m_count: %d\n", match_count);
	  wprintf(L"           required_match: %d\n", required_match_count);
	}
      }

  protected:
      ExpressionType type;                  /* Type of expression */

  private:
      std::wstring rx_string;               /* Regular expression - string */
      boost::basic_regex<wchar_t> rx_boost; /* Regular expression - BOOST */
      size_t match_count;                   /* Running match count */
      size_t required_match_count;          /* Required match count */

  };/* class NLCA::Expression */

  /** ExpressionCCN
   *
   *  \brief Expression to represent credit card number (CCN).  This class implements
   *         validation of the CCN number and redaction to omit delimiters.
   */
  class ExpressionCCN : public NLCA::Expression
  {
  public:
    ExpressionCCN(){ type=NLCA::Expression::CCN;}
    
    /** Redact
     *
     *  \brief Only redact non '-' characters to 'X'
     *  \sa NLCA::Express::Redact()
     */
    void Redact( std::wstring& text , size_t pos , size_t len ) throw()
    {
      for( size_t i = pos ; i < pos + len ; i++ )
      {
	try
	{
	  wchar_t ch = text.at(i);
	  if( ch != L'-' && ch != ' ' )
	  {
	    text.replace(i,1,1,L'X');  /* text[i] = L'X' */
	  }
	}
	catch(...)
	{
	  /* empty */
	}
      }
    }/* Redact */

    /** IsValid
     *
     *  \brief Is the matching text valid for the search criteria.  For example,
     *         credit card numbers require validation of data structure.
     *  \param text (in) Matched text.
     *  \return true if the match is valid, otherwise false.
     */
    bool IsValid( const std::wstring& text ) const throw()
    {
      if( text.empty() == true )
      {
	return false;
      }
      int x[16] = { 0 };  // Credit cards account numbers are [13,16] digits
      int i = 0;
      for( std::wstring::const_iterator it = text.begin() ; it != text.end() ; ++it )
      {
	// Interested in numbers and avoid overrun of x[]
	if( iswdigit(*it) && i < sizeof(x)/sizeof(int) )
	{
	  x[i] = (int)(*it - L'0');  // determine numeric value
	  i++;
	}
      }
      assert( i > 0 );
      return IsValidCCN(x,i); /* Lunh mod 10 */
    }/* IsValid */

  private:

    /** IsValidCCN
     *
     *  \brief Determine if a given string of digits passes Lunh mod 10 check.
     *  \return true of digits form a valid CCN, otherwise false.
     */
    bool IsValidCCN( const int* digits , int num ) const
    {
        assert( digits != NULL );
        if( digits == NULL )
        {
            return false;
        }
        int sum = 0;
        bool alt = false;
        for( int i = num - 1 ; i >= 0 ; --i )
        {
            int current_digit = digits[i];
            if( alt == true )
            {
                current_digit *= 2;
                if( current_digit > 9 )
                {
                    current_digit -= 9; 
                }
            }
            sum += current_digit;
            alt = !alt;
        }
        return ( (sum % 10) == 0 );
    }/* IsValidCCN */
  };/* ExpressionCCN */

  /** ExpressionSSN
   *
   *  \brief Expression to represent social security number (SSN).  This class implements
   *         validation of the SSN number and redaction to omit delimiters.
   */
  class ExpressionSSN : public NLCA::Expression
  {
  public:
    ExpressionSSN() {type=NLCA::Expression::SSN;}
    /** Redact
     *
     *  \brief Only redact non '-' characters to 'X'
     *  \sa NLCA::Express::Redact()
     */
    void Redact( std::wstring& text , size_t pos , size_t len ) throw()
    {
      for( size_t i = pos ; i < pos + len ; i++ )
      {
	try
	{
	  wchar_t ch = text.at(i);
	  if( ch != L'-' && ch != ' ' )
	  {
	    text.replace(i,1,1,L'X');  /* text[i] = L'X' */
	  }
	}
	catch(...)
	{
	  /* empty */
	}
      }
    }/* Redact */

    /** IsValid
     *
     *  \brief Is the matching text valid for the search criteria.  For example,
     *         credit card numbers require validation of data structure.
     *  \param text (in) Matched text.
     *  \return true if the match is valid, otherwise false.
     */
    bool IsValid( const std::wstring& text ) const throw()
    {
      if( text.empty() == true )
      {
	return false;
      }
      std::wstring digits;

      for( std::wstring::const_iterator it = text.begin() ; it != text.end() ; ++it )
      {
	if( isdigit(*it) )
	{
	  digits += *it;
	}
      }
      return IsValidSSN(digits);
    }/* IsValid */

  private:

    /** IsValidSSN
     *
     *  \brief Determine if a given string of digits passes Lunh mod 10 check.
     *  \return true of digits form a valid CCN, otherwise false.
     */
    bool IsValidSSN( const std::wstring text ) const
    {
      if( text.length() < 9 )
      {
	return false;
      }

      std::wstring area, group, serial;

      // SSN Format AAA-BB-CCCC
      area   = text.substr(0,3); // AAA  (area number)
      group  = text.substr(3,2); // BB   (group number)
      serial = text.substr(5,7); // CCCC (serial number)

      // String to integer
      int area_number   = _wtoi(area.c_str());
      int group_number  = _wtoi(group.c_str());
      int serial_number = _wtoi(serial.c_str());

      /* Invalid SSN's per ssa.gov

         Area Number's:
	    (1) 800 or 900 series
	    (2) 700 series above 772
	    (3) 666
       */
      if( area_number > 772 || area_number == 666 || area_number == 000 )
      {
	return false;
      }
      if( group_number == 00 )
      {
	return false;
      }
      if( serial_number == 0000 )
      {
	return false;
      }
      return true;
    }/* IsValidCCN */
  };/* ExpressionSSN */

  /** ExpressionKeywords
   *
   *  \brief This class implements keywords expression which is constructed with SetKeywords
   *         where the parameter is a string which contains a set of keywords.
   *
   *         Example: NextLabs "Secret Document"
   *                  This means two keywords (1) "NextLabs" and (2) "Secret Document".
   */
  class ExpressionKeywords : public NLCA::Expression
  {
    public:
    ExpressionKeywords(){type=NLCA::Expression::KEYWORDS;}

      /** SetExpression
       *
       *  \param in_string (in) A string which contains a set of keywords in the form of single
       *                        words or quoted literal phrases.
       */
      void SetExpression( const std::wstring& in_string ) throw()
      {
	std::list<std::wstring> keywords;
	ExtractKeywords(in_string,keywords);  // extract keywords
	if( keywords.size() <= 0 )
	{
	  return;
	}

	// construct OR expression for each keyword - 'keyword1|keyword2|keyword3|...|keywordN'
	std::wstring keywords_regex;
	for( std::list<std::wstring>::const_iterator it = keywords.begin() ; it != keywords.end() ; )
	{
	  /* Construct each keyword as a literal using the char set expression syntax of [].  This
	   * must be done to avoid conflicts with POSIX literal support \Q[literal\E since one of
	   * these markers may be present in the literal string provided by the user of this
	   * class.
	   */
	  std::wstring word;
	  for( std::wstring::const_iterator i = it->begin() ; i != it->end() ; ++i )
	  {
	    word += L"[";
	    word += *i;
	    word += L"]";
	  }

	  /* Wrap expression with parens and word boundary marker.  Keywords must not match within a
	   * word.  Matches *must* occur on a word boundary.  For example, keyword of "icy" should
	   * not match "icyhot".
	   */
	  word.insert(0,L"(\\b");  // prefix with "(\b" for word boundary
	  word.append(L"\\b)");    // suffix with "\b)" for word boundary

	  keywords_regex += word;     /* Add word to regex */
	  ++it;                       /* Move to next word */
	  if( it != keywords.end() )  /* Append '|' (or) to all but last keyword */
	  {
	    keywords_regex += L"|";
	  }
	}
	Expression::SetExpression(keywords_regex);
      }/* SetExpression */

    private:

      /** ExtractKeywords
       *
       *  \brief Extract keywords from a text string.
       *
       *  \param text (in)      Text to extract keywords from.
       *  \param keywords (out) List of keywords returned to caller.
       *
       *  \return true if keywords were extracted, otherwise false.
       */
      bool ExtractKeywords( const std::wstring& text , std::list<std::wstring>& keywords ) throw()
      {
	std::wstring::size_type i = 0;
	for( ; i != std::wstring::npos && i < text.length() ; )
	{
	  std::wstring::size_type j = 0;
	  i = text.find_first_not_of(L" \t",i); // Non-whitespace
	  if( i == std::wstring::npos )
	  {
	    break;  // done
	  }

	  bool token_quoted = false;
	  if( text.at(i) == L'"' )
	  {
	    token_quoted = true;
	    j = text.find_first_of(L'"',i+1);  // Last quote offset not including current quote
	  }
	  else
	  {
	    j = text.find_first_of(L' ',i);    // Whitespace separator.  Current is non-whitespace
	  }

	  /* Current token goes to end of string.  Two cases: 
	   *  (1) Quote string may be missing ending quote
	   *  (2) Non-quoted string is end of text
	   */
	  if( j == std::wstring::npos )
	  {
	    j = text.length();
	  }

	  std::wstring token(text,i,j-i); // Assign [i,j-1] from input string to token
	  if( token_quoted == true )
	  {
	    token.erase(0,1); // erase leading quote
	  }

	  //fprintf(stderr, "[%d,%d] %ws\n",i,j, token.c_str());
	  keywords.push_back(token);
	  i = j + 1;
	}//for
	return true;
      }/* ExtractKeywords */

  };/* ExpressionKeywords */

}/* namespace NLCA */

#endif /* __NL_CONTENT_ANALYSIS_EXPR_HPP__ */
