#ifndef __NLCA_CLIENT_HPP__
#define __NLCA_CLIENT_HPP__

#include "nl_content_analysis.hpp"

/** NLCAService_SearchFile
 *
 *  \param file (in)       File to search.  This must be the full path.
 *  \param pid (in)        PID of process for which the search is being done.
 *  \param exps (in/out)   Expressions to search for.
 *  \param timeout_ms (in) Timeout in milliseconds for search.  If the search exceeds
 *                         this duration this method will fail (return false).
 *
 *  \return true if the search completes, otherwise false.
 */
bool NLCAService_SearchFile(const wchar_t* file ,
			    int pid ,
			    std::list<NLCA::Expression *> &exps, 
			    int timeout_ms = 0 );
bool NLCAService_Stop(void);

#endif //__NLCA_CLIENT_H__
