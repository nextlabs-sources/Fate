/****************************************************************************************
 *
 * NextLabs Uninstall Cleanup Utility Tool
 *
 * FileCleanup Class
 * January 2012 
 ***************************************************************************************/

#include <windows.h>
#include <string>
#include <iostream>
#include <list>

#include "nl_fileCleanup.h"

#pragma warning( push )
#pragma warning( disable : 6011 )
#pragma warning(disable: 6334)
#  include <boost/algorithm/string.hpp>
#  include <boost/functional/hash.hpp>
#  include <boost/filesystem/operations.hpp>
#  include <boost/filesystem/path.hpp>
#  include <boost/regex.hpp>
#pragma warning( pop )

namespace fs = boost::filesystem;

// create file and directory - used by testing
int FileCleanup::createItems(itemList& items)
{
	fprintf(m_File, "\nCreate files and directories\n");
	itemListIter str = items.begin();
	for( ; str != items.end(); str++)
	{
		createItem(*str);
	}	
	return 0;
};
// delete files and directories
int FileCleanup::deleteItems(itemList& items)
{
	fprintf(m_File, "\n\nDelete files and directories ...\n");
	printf("\n\nDelete files and directories ...\n");

	// collect exception paths
	getExceptionItems(items);

	itemListIter str9 = items.begin();
	// process one by one -- input ordering is not important
	for( ; str9 != items.end(); str9++)
	{
		printf("\n %ws\n", (*str9).c_str());
		deleteItem( *str9 );
	}	
	return m_numErrors;	
};

// create one file or one directory
int FileCleanup::createItem(wstring str)
{
	traverse((const wchar_t*)str.c_str()/*include_root*/ );
	return 0;	
};
// delete one file or one directory
int FileCleanup::deleteItem(const wstring str)
{	
	traverse((const wchar_t*)str.c_str()/*include_root*/ );
	return 0;	
};

/**
 * traverse under directory using recursive method
 *
 */
bool FileCleanup::traverse( __in const wchar_t* path )
{
  int returnC;
  int localCnt;
  bool deletedFlag;
  fs::wpath full_path(path);
  assert(!full_path.empty());
  bool path_exists = false;

  try
  {
		path_exists = fs::exists(full_path);
  }
  catch( ... )
  {
		path_exists = false;
  }
  if( path_exists == false )
  {
	  //printf("\n\tpath %ls does not exist\n", path );
	  fprintf(m_File, "\n\tpath %ls does not exist\n", path );
      return true; /* path does not exist it is Ok*/
  }
#if 1
	if( IsSubpathOfExceptionPaths(full_path.file_string() ) == 0)
	{
		// remove file
		if( m_Delete == true )
		{
			// internally use fs::remove(full_path);
			localCnt = remove_recursive(full_path.file_string().c_str());
			if( localCnt == 0 )
			{
				m_numErrors++;
				fprintf(m_File, "\tERROR: Failed to remove a file or directory %ls\n", full_path.file_string().c_str());
				printf("\tERROR: Failed to remove a file or directory %ls\n", full_path.file_string().c_str() );
			}
			else
			{
				fprintf(m_File, "\tRemoved a file or directory %ls (number of files and dirs = %d) \n", full_path.file_string().c_str(), localCnt);
				printf("\tRemoved a file or directory %ls (number of files and dirs = %d)\n", full_path.file_string().c_str(), localCnt );
			}
		} else
		{
			fprintf(m_File, "\tRemoved a file or directory %ls (%ls)\n", full_path.file_string().c_str(), msg );
			printf("\ttRemoved a file or directory %ls (%ls)\n", full_path.file_string().c_str(),msg );
		}
		return true;
	}
#endif

  fs::wdirectory_iterator end_iter;

  /* Process all files first */

  fs::wdirectory_iterator it;
  try
  {
    it = fs::wdirectory_iterator(full_path);
  }
  catch( std::exception fse )
  {
    it = end_iter;
  }
  for( ; it != end_iter ; ++it )
  {

	try
    {
		if( fs::is_regular_file(it->status()) )
		{
			// check if path is subpath of exception paths
			// consider the case that exception path is a regular file  !!!!
		  
			returnC = IsSubpathOfExceptionPaths(it->path().file_string() );
		    if( returnC == 0 )
			{
				// remove file
				if( m_Delete == true )
				{
					deletedFlag = fs::remove(it->path());
					if( deletedFlag == false )
					{
						m_numErrors++;
						fprintf(m_File, "\tERROR: Failed to remove file %ls  \n", it->path().file_string().c_str());
						printf("\tERROR: Failed to remove file %ls  \n", it->path().file_string().c_str() );
					}
				} 
				fprintf(m_File, "\tFile %ls  %ls Removed\n", it->path().file_string().c_str(), msg );
				printf("\tFile %ls  %ls Removed\n", it->path().file_string().c_str(),msg );
				
			 }
		}
    }
    catch( const std::exception& ex )
    {
      std::cout << "\n\tERROR in removing " << it->path().file_string().c_str() << "\n\t\t " << ex.what() << std::endl;
    }
  }/* for */

  /* Process all directories */
  try
  {
    it = fs::wdirectory_iterator(full_path);
  }
  catch( std::exception fse )
  {
    it = end_iter;
  }
  for(  ; it != end_iter ; ++it )
  {

    try
    {
      if( fs::is_directory(it->status()) )
      {
		  // check if path is subpath of exception paths
		  returnC = IsSubpathOfExceptionPaths(it->path().file_string() );
		  if( returnC > 0 )
		  {
				// it is a subpath
			    if( returnC == 1 )
				{
 					// it is innode of exception path and so continue to traverse
					traverse((const wchar_t*)it->path().file_string().c_str()/*include_root*/);
				}

				// dont remove and backtract if returnC == 2

		  } else
		  {
				// remove directory
			    unsigned long cnt = 0;
				if( m_Delete == true)
				{
					// remove files and dirs physically
					//internnally use cnt = fs::remove_all(it->path());
					cnt = remove_recursive(it->path().file_string().c_str());
					if( cnt == 0 )
					{
						m_numErrors++;
						fprintf(m_File, "\tERROR: Directory %ls is not removed \n",
							it->path().file_string().c_str() );
						printf("\tERROR: Directory %ls  is not removed \n",
							it->path().file_string().c_str() );
					} else
					{
						fprintf(m_File, "\tRemoved directory %ls - %ls (number of files and dirs = %d)\n",
							it->path().file_string().c_str(), msg, cnt );
						printf("\tRemoved directory %ls - %ls (number of files and dirs = %d)\n",
							it->path().file_string().c_str(), msg, cnt );
					}
				} else
				{
					fprintf(m_File, "\tRemoved directory %ls - %ls\n",
						it->path().file_string().c_str(), msg);
					printf("\tRemoved directory %ls %ls\n",
						it->path().file_string().c_str(), msg );
				}
		  }
      }
    }
    catch( const std::exception& ex )
    {
		std::wcout << L"\n\tERROR: Failed to remove " << it->path().file_string().c_str() << "\n\t\t " << ex.what() << std::endl;
    }
  }/* for */
  return true;
}/* traverse */

// compare target delete path with one or more exception paths
// return value:
//     1   :  deletePath is a subpath of one path of exceptionPaths and they are not the same
//     2   :  deletePath and  at least one of exceptionPaths are same
//     0   :  otherwise
int FileCleanup::IsSubpathOfExceptionPaths(const wstring deletestr)
{
	int globalRtn =0;
	int localRtn;
	m_deletepath.clear();
	itemListIter str = m_exceptions.begin();
	for( ; str != m_exceptions.end(); str++)
	{
		localRtn = IsSubpathOfExceptionPath(deletestr, *str);
		if( globalRtn < localRtn)
			globalRtn = localRtn;
		if( globalRtn == 2 )
			return 2; // this in exception path -- dont delete
	}	
	return globalRtn; // not in exception path so delete
}

// classify to two groups: deleted path and exceptions
// At the end, items has only paths for deletion
//              m_exceptions vector has a list of exception paths
int FileCleanup::getExceptionItems(itemList& items)
{
	wchar_t buffer[128];
	m_deletepath.clear();
	itemListIter str = items.begin();
	for( ; str != items.end(); str++)
	{
		if( wcsncmp((*str).c_str(), L"-", 1) == 0 )
		{
			// remove '-' in string
			wcscpy_s(buffer,128, (*str).c_str() );
			m_exceptions.push_back( wstring(buffer+1) );
		}
		else
		{
			m_deletepath.push_back( wstring(*str) );
		}
	}	
	items.clear();
	str = m_deletepath.begin();
	for( ; str != m_deletepath.end(); str++)
	{
		items.push_back( wstring(*str) );
	}	
	return 0;
}


// compare target delete path with one exception path
// return value:
//     1   :  deletePath is a subpath of exceptionPath and they are not the same
//     2   :  deletePath and  exceptionPath are same
//     0   :  otherwise
int FileCleanup::IsSubpathOfExceptionPath(const wstring deletePath, const wstring exceptionPath)
{
	size_t st;
	fs::wpath deletefull_path(deletePath.c_str());
	fs::wpath exceptfull_path(exceptionPath.c_str());
	std::wstring delpath = deletefull_path.native_directory_string();
	std::wstring exceptpath = exceptfull_path.native_directory_string();

	if( delpath.size() > exceptpath.size() )
		return 0;
	for( st =0; st < delpath.size(); st++ )
	{
		if( delpath[st] != exceptpath[st] )
			return 0;
	}
    if( (exceptpath[st] == '\\') )
	{
		return 1;
	}
	if(  (exceptpath[st] == '\0') )
	{
		return 2;
	}
	return 0;
}


/** 
 * remove files and directories using recursive method 
 */
int FileCleanup::remove_recursive( __in const wchar_t* path  )
{
  bool deleteChildrenDone = true;
  int totalCount =0;
  int localCnt;
  fs::wpath full_path(path);
  assert(!full_path.empty());
  bool path_exists = false;

  try
  {
		path_exists = fs::exists(full_path);
  }
  catch( ... )
  {
		path_exists = false;
  }
  if( path_exists == false )
  {
	  //printf("\n\tpath %ls does not exist\n", path );
	  fprintf(m_File, "\n\tremove_recursive:path %ls does not exist\n", path );
      return 0; /* path does not exist it is Ok*/
  }

  fs::wdirectory_iterator end_iter;

  /* Process all files first */
  fs::wdirectory_iterator it;
  try
  {
    it = fs::wdirectory_iterator(full_path);
  }
  catch( std::exception fse )
  {
    it = end_iter;
  }
  for( ; it != end_iter ; ++it )
  {
	try
    {
		if( fs::is_regular_file(it->status()) )
		{
			// regular file
			bool removed = fs::remove( it->path() );
			if( removed == false)
			{
				m_numErrors++;
				deleteChildrenDone = false;
				fprintf(m_File, "\tERROR: Failed to remove file %ls\n", it->path().file_string().c_str() );
				printf("\tERROR: Failed to remove file %ls\n", it->path().file_string().c_str() );
			} else
				totalCount++;
			 
		}
    }
    catch( const std::exception& ex )
    {
      		m_numErrors++;
			std::wcout << L"\n\tERROR: Failed to remove file " << it->path().file_string().c_str() << "\n\t\t " << ex.what() << std::endl;
    }
  }/* for */

  /* Process all directories */
  try
  {
    it = fs::wdirectory_iterator(full_path);
  }
  catch( std::exception fse )
  {
    it = end_iter;
  }
  for(  ; it != end_iter ; ++it )
  {

    try
    {
      if( fs::is_directory(it->status()) )
      {
			// remove files and dirs physically
			localCnt = remove_recursive(it->path().file_string().c_str());
			if( localCnt == 0 )
			{
				m_numErrors++;
				fprintf(m_File, "\tERROR: Failed to remove directory %ls\n",
					it->path().file_string().c_str() );
				printf("\tERROR: Failed to remove directory %ls\n",
					it->path().file_string().c_str() );
				deleteChildrenDone = false;
			}
			totalCount += localCnt;
		  
      }
    }
    catch( const std::exception& ex )
    {
		m_numErrors++;
		std::wcout << L"\n\tERROR: Failed to remove directory " << it->path().file_string().c_str() << "\n\t\t " << ex.what() << std::endl;
    }
  }/* for */


  try
  {
		// delete this directory if childern dir and files are deleted
		if( deleteChildrenDone == true )
		{
			// delete top-parent dir because dir is empty
			bool removed = fs::remove(full_path);
			if( removed == false)
			{
				m_numErrors++;
				fprintf(m_File, "\tERROR: Failed to remove file %ls\n", full_path.file_string().c_str() );
				printf("\tERROR: Failed to remove file %ls\n", full_path.file_string().c_str() );
			}else
				totalCount++;
		} 
  }
  catch( const std::exception& ex )
  {
      m_numErrors++;
	  std::wcout << L"\n\tERROR: Failed to remove file or directory " << full_path.file_string().c_str() << "\n\t\t " << ex.what() << std::endl;
  }

  return totalCount;
}/* remove_recursive */