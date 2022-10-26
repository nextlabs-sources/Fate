/*********************************************************************************************
 *
 * Verdict Cache
 *
 * General object for a policy verdict (decision) cache.
 *
 ********************************************************************************************/

#ifndef __VERDICT_CACHE_HPP__
#define __VERDICT_CACHE_HPP__

#include <string>
#include <algorithm>
#include <map>
#include <boost/utility.hpp>

#pragma warning( push )
#pragma warning( disable : 6011 4996)
#  include <boost/algorithm/string.hpp>
#include <boost/unordered/unordered_map.hpp>
#pragma warning( pop )

namespace nextlabs
{

  /** cache_entry
   *
   *  \brief Resource entry in the cache.
   */
  class cache_entry
  {
    public:
      cache_entry() :
	mod_time(0),
	touch_time(0),
	ttl(60),         /* 60 second default */
	info()
      {
	touch_time = GetTickCount();           /* touch time is current time for new elements */
      }/* cache_entry */

      time_t mod_time;                         /* modification time */
      time_t touch_time;                       /* touch time of element (access,created,etc.) */
      std::size_t ttl;                         /* time to live in seconds */
      std::map<std::wstring,DWORD> info;       /* resource information <name,time_added> */
  };/* cache_entry */

  /** verdict_cache
   *
   *  \brief Verdict cache interface.
   *
   *  \notes This interface is not MT-safe.  Default size is 100 elements.  Default window
   *         is 60 seconds.
   */
  class verdict_cache : boost::noncopyable
  {
    public:

      verdict_cache(void) :
	param_element_bound(100),     /* 100 elements */
	elem_map(),                   /* cache element map */
	stats_hitcount(0),
	stats_misscount(0)
      {
      }/* verdict_cache */

      /** clear
       *
       *  \brief Clear the cache.
       */
      void clear(void) throw()
      {
	elem_map.clear();
      }/* clear */

      /** resize
       *
       *  \brief Reset the size (bound) of the cache.
       *
       *  \notes The cache is cleared as a side-effect of this method.
       */
      void resize( std::size_t in_bound ) throw()
      {
	clear();
	param_element_bound = in_bound;
      }/* resize */

      /** query
       *
       *  \brief Determine if the given resource and action require a policy decision.
       *
       *  \param in_action (in) Action.
       *  \param in_source (in) Resource.
       *  \param mod_time (in)  Modification time of Resource parameter.
       *
       *  \return true if a previous deicision can be used, otherwise false.
       */
      bool query( _In_ const wchar_t* in_action ,
		  _In_ const wchar_t* in_source ,
		  _In_ time_t in_mod_time ) throw()
      {
	assert( in_action != NULL );
	assert( in_source != NULL );
	if( in_action == NULL || in_source == NULL )
	{
	  return false;
	}

	std::wstring source_lower(in_source);
	boost::algorithm::to_lower(source_lower);
	boost::unordered_map<std::wstring,nextlabs::cache_entry>::iterator it = elem_map.find(source_lower);
	if( it != elem_map.end() )
	{
	  const nextlabs::cache_entry& ce = it->second; /* reference to element */

	  if( ce.mod_time != in_mod_time ) /* Modify time mismatch is a miss */
	  {
	    stats_misscount++;
	    return false;
	  }

	  /* Find an entry based on action.  There may be multiple actions for a single resource. */
	  std::map<std::wstring,DWORD>::const_iterator info = ce.info.find(in_action);
	  if( info != ce.info.end() )
	  {
	    DWORD entry_time = info->second;                 /* entry time in ms */
	    DWORD curr_time  = GetTickCount();               /* current time in ms */
	    if( (entry_time + (ce.ttl * 1000)) > curr_time ) /* Cache entry has expired? (ms -> sec) */
	    {
	      it->second.touch_time = GetTickCount();
	      stats_hitcount++;                              /* We have a hit */
	      return true;
	    }
	    elem_map.erase(it);                              /* Entry is expired.  Remove it. */
	  }
	}
	stats_misscount++;                                   /* miss */
	return false;
      }/* query */

      /** query_resource_exist
       *
       *  \brief Determine if the given resource exists in the cache.
       *
       *  \param in_source (in) Resource.
       *
       *  \return true if there is an entry for the given resource, otherwise false.
       */
      bool query_resource_exist( _In_ const wchar_t* in_source ) const throw()
      {
	assert( in_source != NULL );
	if( in_source == NULL )
	{
	  return false;
	}

	std::wstring source_lower(in_source);
	boost::algorithm::to_lower(source_lower);
	boost::unordered_map<std::wstring,nextlabs::cache_entry>::const_iterator it = elem_map.find(source_lower);
	if( it != elem_map.end() )
	{
	  return true;
	}
	return false;
      }/* query_resource_exist */

      /** commit
       *
       *  \brief Commit a decision to the verdict cache.
       *
       *  \param in_action (in)   Action.
       *  \param in_source (in)   Resource.
       *  \param in_ttl (in)      Time to Live (TTL) in seconds.
       *  \param in_mod_time (in) Modification time of Resource parameter.
       */
      void commit( _In_ const wchar_t* in_action ,
		   _In_ const wchar_t* in_source ,
		   _In_ std::size_t in_ttl,
		   _In_ time_t in_mod_time ) throw()
      {
	assert( in_action != NULL );
	assert( in_source != NULL );
	if( in_action == NULL || in_source == NULL )
        {
	  return;
	}

	if( elem_map.size() >= param_element_bound )
	{
		boost::unordered_map<std::wstring,nextlabs::cache_entry>::iterator it;
		boost::unordered_map<std::wstring,nextlabs::cache_entry>::iterator itEarliest = elem_map.end();
		time_t earliest = 0;

		for(it = elem_map.begin(); it != elem_map.end(); it++)
		{
			if(earliest == 0)
			{
				earliest = it->second.touch_time;
				itEarliest = it;
			}
			else
			{
				if(it->second.touch_time < earliest)
				{
					earliest = it->second.touch_time;
					itEarliest = it;
				}
			}
		}
	  
	  assert( itEarliest != elem_map.end() );
	  elem_map.erase(itEarliest);
	  
	}

	std::wstring source_lower(in_source);                 /* commit in lower case */
	boost::algorithm::to_lower(source_lower);
	nextlabs::cache_entry& ce = elem_map[source_lower];   /* reference element and populate it */
	ce.info[in_action] = GetTickCount();
	ce.ttl             = in_ttl;
	ce.mod_time        = in_mod_time;
      }/* commit */

      /** misses
       *
       *  \brief Return the number of cache misses.
       */
      std::size_t misses(void) const throw()
      {
	return stats_misscount;
      }/* misses */

      /** hits
       *
       *  \brief Return the number of cache hits.
       */
      std::size_t hits(void) const throw()
      {
	return stats_hitcount;
      }/* hits */

    private:

      /** min_touch_time_pred
       *
       *  \brief Binary predicate for std::min_element.  Implements operator() for element
       *         comparison.
       */
      class min_touch_time_pred
      {
        public:

	  /* Return true if first param is strictly less than the second param. */
	  bool operator ()( const std::pair<std::wstring,nextlabs::cache_entry>& elem_a ,
			    const std::pair<std::wstring,nextlabs::cache_entry>& elem_b )
	  {
	    if( elem_a.second.touch_time < elem_b.second.touch_time )
	    {
	      return true;
	    }
	    return false;
	  }// operator()
      };// min_touch_time_pred

      std::size_t param_element_bound;                       /* maximum cache size */
	  boost::unordered_map<std::wstring,nextlabs::cache_entry> elem_map; /* cache elements */
      std::size_t stats_hitcount;                            /* hit count */
      std::size_t stats_misscount;                           /* miss count */

  };/* verdict_cache */

}/* nextlabs */

#endif /* __VERDICT_CACHE_HPP__ */
