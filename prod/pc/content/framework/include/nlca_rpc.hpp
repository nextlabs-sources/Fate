
#ifndef __NLCA_RPC_HPP__
#define __NLCA_RPC_HPP__

#include <winsock2.h>
#include <windows.h>
#include <sstream>

#pragma warning(push)
#pragma warning(disable: 4512)
#pragma warning(disable: 6326)
#  include <boost/archive/text_oarchive.hpp>
#  include <boost/archive/text_iarchive.hpp>
#  include <boost/archive/archive_exception.hpp>
#pragma warning(pop)

#include "nl_content_analysis.hpp"
#include "nlca_serialization.hpp"
#include "brain.h"

namespace NLCA {
class ContentAnalysisRPCTask;

/* ContentAnalysisRPC
 *
 * \brief Content analysis RPC data structure
 *
 */
class ContentAnalysisRPC 
{
  public:
    typedef enum{NotSpecified, DoContentAnalysis, StopService} Type;
  private:
    friend class ContentAnalysisRPCTask;
    friend class boost::serialization::access;
    // When the class Archive corresponds to an output archive, the
    // & operator is defined similar to <<.  Likewise, when the class Archive
    // is a type of input archive the & operator is defined similar to >>.
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version) {
      ar & _ca;
      ar & pid;
      ar & type;
    }

    NLCA::ContentAnalysis _ca;
    int pid; // Process ID of client
    Type type;

  public:
  ContentAnalysisRPC():type(NotSpecified), pid(0){}; 
  ContentAnalysisRPC(Type t):type(t), pid(0) {
    }
    ContentAnalysisRPC(NLCA::ContentAnalysis &ca, 
		       int in_pid, Type t):_ca(ca), pid(in_pid), type(t){
    }

    ~ContentAnalysisRPC(){};

    //Access member function
    Type GetRPCType() {return type;}
    NLCA::ContentAnalysis & GetContentAnalysis() {return _ca;}
    void SetContent(NLCA::ContentAnalysis &ca) {_ca=ca;}
    void SetType(Type t) {type=t;}
    void SetPID( int in_pid ) {pid=in_pid;}
#if 0
    void display(CELog &calog){
      if(type != DoContentAnalysis)
      {
	calog.Log(CELOG_DEBUG, _T("Content Analysis RPC %d\n"), type);		
      } else {
	calog.Log(CELOG_DEBUG, _T("Content Analysis Request (pid=%d)\n"), 
		  pid);	
	//_ca.display(calog);
      }
    }
#endif
}; //ContentAnalysisRPC
}
/* NLCAService
 *
 * \brief Interface for clients to use the NLCA service.
 *
 */
class NLCAService
{
  public:

    static const int SERVICE_PORT = 27019;

    /** Serialization
     *
     *  \brief Serialize ContentAnalysis RPC packet
     */
    template <typename T>
    static void Serialization(const T& e, std::string &out, unsigned long &oSize)
    {
      try {
        std::ostringstream archive_stream;
        boost::archive::text_oarchive archive(archive_stream);
	archive << e;
	out=archive_stream.str();
	oSize=(unsigned long)out.size();
      } catch (boost::archive::archive_exception &) {
	oSize=0;
	return;
      }
      catch( std::bad_alloc& )
      {
	return;
      }
    }

    /** Deserialization
     *
     *  \brief deserialize ContentAnalysis RPC packet
     *  \return true if deserialization was successful, otherwise false.
     */
    template <typename T>
    static bool Deserialization(const char *in, long inSize, T& out) {
      try {
	std::string tmpBuf(in, inSize);
	std::istringstream archive_stream(tmpBuf);
	boost::archive::text_iarchive archive(archive_stream);
	archive >> out;
	return true;
      } catch (boost::archive::archive_exception &) {
	return false;
      }
      catch( std::bad_alloc& )
      {
	return false;
      }
    }
};//NLCAService
#endif /* __NLCA_RPC_HPP__ */
