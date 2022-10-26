
#ifndef __NLCA_CLIENT_HPP__
#define __NLCA_CLIENT_HPP__

#include <list>
#include <winsock2.h>
#include <windows.h>

#include <sstream>

#pragma warning(push)
#pragma warning(disable: 4512)
#pragma warning(disable: 6326)
#  include <boost/archive/text_oarchive.hpp>
#  include <boost/archive/text_iarchive.hpp>
#pragma warning(pop)

#include "nl_content_analysis.hpp"
#include "nlca_serialization.hpp"
#include "nlca_rpc.hpp"

/* NLCAClient
 *
 * \brief Interface for clients to use the NLCA service.
 *
 */
class NLCAClient
{
  public:
    /** SearchFile
     *
     *  \brief Request the NLCA service to perform analysis of the given file.
     *
     *  \param file (in)                File to search.
     *  \param pid (in)                 Process ID
     *  \param expression_list (in-out) Set of expressions to search for.
     *  \param timeout_ms (in)          Timeout if no response from service within given number
     *                                  of milliseconds.
     *
     *  \return true if the search completed, otherwise false.
     */
    __declspec(dllexport) bool SearchFile( const wchar_t* file ,
					   int pid ,
					   std::list<NLCA::Expression *> &exps, 
					   int timeout_ms = 0 );
  
    /** Stop
     *
     *  \brief Issue a 'stop' command to the NLCA service.
     */
    __declspec(dllexport) bool Stop(void);

  private:

    /** Connect
     *
     *  \brief Connect to the NLCA service.
     */
    bool Connect(void);

    void Disconnect(void);

    bool ReadingAnalysisResult(std::list<NLCA::Expression *> &outExs,
			       timeval* tv );

    bool SendRequest(NLCA::ContentAnalysisRPC &rpc);
    
    void ConstructCARPCPacket(const wchar_t* file ,
			      int pid ,
			      std::list<NLCA::Expression *> &expression_list,
			      NLCA::ContentAnalysisRPC &caRPC);

    SOCKET s;  // socket for comm. with nlca service

};/* NLCAClient */

#endif /* __NLCA_CLIENT_HPP__ */
