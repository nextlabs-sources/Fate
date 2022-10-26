
#ifndef __NEXTLABS_FEATURE_MANAGER_HPP__
#define __NEXTLABS_FEATURE_MANAGER_HPP__

#include <windows.h>
#include <fstream>
#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <string>
#include <map>

#pragma warning( push )
#pragma warning( disable : 6011 4996)
#  include <boost/utility.hpp>
#  include <boost/algorithm/string.hpp>
#pragma warning( pop )

#include "nlconfig.hpp"

namespace nextlabs
{

  /** feature_manager
   *
   *  \brief A class to manage the checking of feature enablement.
   */
  class feature_manager : boost::noncopyable
  {
    public:

      feature_manager(void) :
	feat()
      {
      }

      DWORD open(void)
      {
	wchar_t pc_path[MAX_PATH] = {0};
	bool rv;
	rv = NLConfig::GetComponentInstallPath(L"Compliant Enterprise\\Policy Controller",
					       pc_path,_countof(pc_path));

	if( rv == false )
	{
	  return ERROR_FILE_NOT_FOUND;
	}
	std::wstring lfile_path(pc_path);
	lfile_path.append(L"\\license\\license.cfg");
	return open(lfile_path.c_str());
      }/* open */

      /** open
       *
       *  \brief Open a license file.
       */
      DWORD open( _In_ const wchar_t* in_file )
      {
	assert( in_file != NULL );

	std::wfstream file;
	wchar_t line[512] = {0};

	file.open(in_file,std::ios_base::in);
	if( file.is_open() == false )
	{
	  return ERROR_FILE_NOT_FOUND;
	}

	/* Read the license file into the feature map (key-value) table of enablement. */
	for( ; ; )
	{
	  file.getline(line,_countof(line));
	  if( file.eof() == true )
	  {
	    break;
	  }
	  /* parse line */
	  std::wstring cpp_line(line);

	  /* remove whitespace */
	  boost::algorithm::erase_all(cpp_line,L" ");
	  boost::algorithm::erase_all(cpp_line,L"\t");
	  boost::algorithm::erase_all(cpp_line,L"\n");

	  /* Comment line? */
	  if( boost::algorithm::istarts_with(cpp_line,L"#") == true ||
	      boost::algorithm::istarts_with(cpp_line,L";") == true )
	  {
	    continue;
	  }

	  /* Does not contain a '=' separator */
	  if( boost::algorithm::contains(cpp_line,L"=") == false )
	  {
	    continue;
	  }

	  /* commit this line after separation of '=' */
	  std::wstring key;
	  std::wstring value;
	  BOOL value_bool = FALSE;

	  std::wstring::size_type i;
	  i = cpp_line.find_first_of(L'=');

	  /* Strings w/o '=' should have been filtered by now. */
	  assert( i != std::wstring::npos );

	  key.assign(cpp_line,0,i);
	  value.assign(cpp_line,i+1,cpp_line.length()-i);

	  //fprintf(stdout, "key/value = <'%ws','%ws'>\n", key.c_str(), value.c_str());

	  value_bool = FALSE;
	  if( value == L"TRUE" )
	  {
	    value_bool = TRUE;
	  }

	  feat[key] = value_bool;
	}/* for */

	file.close();
	return ERROR_SUCCESS;
      }/* open */

      /** query
       *
       *  \brief Determine if a key is enabled/set.
       *
       *  \param in_key (in)        Key to check.
       *  \param in_result (in-out) Result of key to check.
       *
       *  \return ERROR_SUCCESS on successfully finding the key, otherwise ERROR_NOT_FOUND.
       */
      DWORD query( _In_ const wchar_t* in_key ,
		   _Out_ BOOL* in_result ) const
      {
	std::map<std::wstring,BOOL>::const_iterator it;
	it = feat.find(in_key);
	if( it == feat.end() )
	{
	  return ERROR_NOT_FOUND;
	}

	*in_result = it->second;
	return ERROR_SUCCESS;
      }/* query */

      /** is_enabled
       *
       *  \brief Determine if a specific feature is enabled.  This differs from query in that
       *         it does NOT provide any information about error conditions such as a missing
       *         key.  A missing key is treated as feature disabled.  It is not possible to
       *         determine if the feature is missing or explicitly disabled.
       */
      bool is_enabled( _In_ const wchar_t* in_key ) const
      {
	BOOL feature_result;
	bool enabled = false;
	if( query(in_key,&feature_result) == ERROR_SUCCESS
	    && feature_result == TRUE )
	{
	  enabled = true;
	}
	return enabled;
      }/* is_enabled */

    private:

      std::map<std::wstring,BOOL> feat;

  };/* feature_manager */

};/* namespace nextlabs */

#endif /*  __NEXTLABS_FEATURE_MANAGER_HPP__ */
