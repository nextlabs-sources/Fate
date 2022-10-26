/*******************************************************************************
 *
 * NextLabs Content Analysis Serialization Interface
 *
 * This interface defines the serializable RPC data structure 
 * using boost.org serialization library.
 *
 *
 ******************************************************************************/

#ifndef __NLCA_SERIALIZATION_HPP__
#define __NLCA_SERIALIZATION_HPP__

#include <iostream>
#include <string>
#include <list>
#include <boost/regex.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/list.hpp>

#include "nl_content_analysis.hpp"
#include "brain.h"
//#include "celog.h"

/** NLCA - NextLabs Content Analysis Interface
 */
namespace NLCA
{
  /** SerializableExpression
   *
   *  Serializable regular expression object.  
   *  It is same as class expression but allow boost serialization library
   *  to access its members.
   */
  class SerializableExpression
  {
    public:
      SerializableExpression(Expression &e) :
	rx_string(),                /* expression string */
	match_count(0),             /* running match count */
	required_match_count(1)     /* one required matche */
      {
	//expTypeName=typeid(e).name();
	type=e.GetType();
	rx_string=e.RegexString();
	match_count=e.MatchCount();
	required_match_count=e.RequiredMatchCount();
      }/* SerializableExpression */
    
      SerializableExpression& operator=(const Expression &e) {
	//expTypeName=typeid(e).name();
	rx_string=e.RegexString();
	match_count=e.MatchCount();
	required_match_count=e.RequiredMatchCount();
	type=e.GetType();
	return (*this);
      }

      //Access member functions
      //const char* GetExpTypeName() {return expTypeName.c_str();}
      Expression::ExpressionType GetType() const throw() {
	return type;
      }
      const std::wstring& RegexString(void) const throw() {
	return rx_string;
      }/* RegexString */
      size_t RequiredMatchCount(void) const throw() {
	return required_match_count;
      }/* RequiredMatchCount */
      size_t MatchCount(void) const throw() {
	return match_count;
      }/* MatchCount */
      void SetMatch(size_t s) {match_count=s;}


#if 0
      /** Display
       *
       *  \brief Display the expression to stdout
       */
      void display(CELog &calog){
	if(!rx_string.empty()) {
	  calog.Log(CELOG_DEBUG,"expression type: %hs\n",expTypeName.c_str());
	  calog.Log(CELOG_DEBUG,"           string: %ws\n", rx_string.c_str());
          calog.Log(CELOG_DEBUG,"           m_count: %d\n", match_count);
	  calog.Log(CELOG_DEBUG,"           required_match: %d\n", required_match_count);
	}
      }
#endif

      /** CreateExpression
       *
       *  \brief Generate non-serializable Expression object
       */
      void CreateExpressionObject(Expression *e) {
	e->SetRequiredMatchCount(required_match_count);

	// The expression has already been constructed via SetExpression.  Avoid reconstruction and
	// simply assign the expression string.
	e->SetExpressionRaw(rx_string);
	e->SetMatch(match_count);
      }

    private:
    //std::string expTypeName;             /* expression type name, e.g. SSN */
      Expression::ExpressionType type;     /*expression type */
      std::wstring rx_string;              /* Regular expression - string */
      size_t match_count;                  /* Running match count */
      size_t required_match_count;         /* Required match count */

      //boost data structure serialization
      friend class boost::serialization::access;
      // When the class Archive corresponds to an output archive, the
      // & operator is defined similar to <<.  Likewise, when the class Archive
      // is a type of input archive the & operator is defined similar to >>.
      template<class Archive>
      void serialize(Archive & ar, const unsigned int version) {
	//ar & expTypeName;
	ar & type;
	ar & rx_string;
	ar & match_count;
	ar & required_match_count;
      }

      SerializableExpression(void) :
	rx_string(),                /* expression string */
	match_count(0),             /* running match count */
	required_match_count(1)     /* one required matche */
      {
	//no default constructor
	//expression type name must be specified
	//in order to generate non-serializable Expression object
      }/* SerializableExpression */
  };/* class NLCA::SerializableExpression */

  /** Content
   *
   *  Object of content to be analized.
   */
   class Content
   {
     public:
       typedef enum {FILE, DATA} ContentType;
     private:
       friend class boost::serialization::access;
       // When the class Archive corresponds to an output archive, the
       // & operator is defined similar to <<. Likewise, when the class Archive
       // is a type of input archive the & operator is defined similar to >>.
       template<class Archive>
       void serialize(Archive & ar, const unsigned int version) {
	 ar & details;
	 ar & type;
       }

       std::wstring details; //content details (e.g. location or data)
       ContentType type; //content type                
     public:
       Content():type(DATA) {};// content by default is DATA type
       Content(std::wstring &d, ContentType t):details(d), type(t) {}
       Content(const wchar_t* d, ContentType t):type(t) {
	 if(d) details=d;
	 else details=L"";
       }

#if 0
       void display(CELog &calog){
	 calog.Log(CELOG_DEBUG, _T("Content type: %s\n"), 
	       type==FILE?_T("FILE"):_T("DATA"));
	 if(!details.empty())
	   calog.Log(CELOG_DEBUG, _T("        details: %s\n"),details.c_str());
       }
#endif
     ContentType GetType() {return type;}
     void GetDetails(std::wstring &d) {d=details;}
   }; //class NLCA::Content

  /** ContentAnalysis
    *
    *  A wrapper of content to be analized and the expressions to
    *  be serarched against.
    */
  class ContentAnalysis
  {
    private:
      friend class boost::serialization::access;
      // When the class Archive corresponds to an output archive, the
      // & operator is defined similar to <<.  Likewise, when the class Archive
      // is a type of input archive the & operator is defined similar to >>.
      template<class Archive>
      void serialize(Archive & ar, const unsigned int version) {
	ar & _content;
	ar & _expressions;
	ar & bHasMatch;
      }

      NLCA::Content _content; //content to be analized
      std::list<NLCA::SerializableExpression> _expressions; //expressions
      bool bHasMatch; //true, content matches one or more of expressions
    public:
      ContentAnalysis():bHasMatch(false){};
      ContentAnalysis(Content &c, 
		      std::list<NLCA::SerializableExpression> &expression_list)
	:_content(c), _expressions(expression_list), bHasMatch(false){}

      ContentAnalysis &operator=(ContentAnalysis &rhs) {
	if(this != &rhs) {
	  _content=rhs._content;
	  _expressions=rhs._expressions;
	}
	return (*this);
      }

#if 0
      void display(CELog &calog){
	_content.display(calog);
	std::list<NLCA::SerializableExpression>::iterator it;
	std::list<NLCA::SerializableExpression>::const_iterator eit;
	it = _expressions.begin();
	eit= _expressions.end();
	for(; it != eit; ++it )
	  (*it).display(calog);
	calog.Log(CELOG_DEBUG, _T("Content vs. expressions: %s\n"), 
	      bHasMatch?_T("match"):_T("no match"));
      }
#endif
    //Access Functions
    Content &GetContent() {return _content;}
    std::list<NLCA::SerializableExpression> &GetExpressions() {
      return _expressions;}
    bool HasMatch() {return bHasMatch;}

    void SetMatchResult(bool m) {bHasMatch=m;}
  }; //ContentAnalysis  
}; /* NLCA */

#endif /* __NLCA_SERIALIZATION_HPP__ */
