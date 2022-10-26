/**********************************************************************
 *
 * celog_policy_file.hpp
 *
 * Default file logging policy.
 *
 *********************************************************************/

#ifndef __CELOG_POLICY_FILE_HPP__
#define __CELOG_POLICY_FILE_HPP__

#include <cassert>
#if defined (WIN32) || defined(_WIN64)
#include <io.h>
#include <share.h>
#include <boost/utility.hpp>
#else
#include <unistd.h>
#include <sys/stat.h>
#include "linux_win.h"
#include "boost/utility.hpp"
#endif
#include "celog.h"

#define CELOGPOLICY_FILE_MAX_SIZE (1024 * 1024)  /* 1MB */

/** CELogPolicy_File
 *
 *  \brief Policy for logging to a file.  The file will be flushed with
 *         each write which occurs when the Log method is called.
 */
class CELogPolicy_File : boost::noncopyable, public CELogPolicy
{
  public:
    /** CELogPolicy_File
     *
     *  \brief Constructo for CELogPolicy_File instance.  Pass a file
     *         to be used.
     *
     *  \param file_path (in) File to be used for logging.
     */
    CELogPolicy_File( const char* file_path ) :
      fp(NULL),
      max_size(CELOGPOLICY_FILE_MAX_SIZE)
    {
      assert( file_path != NULL );
      if(file_path != NULL)
      {
	fp = _fsopen(file_path,"a+",_SH_DENYWR);
      }
    }

    virtual ~CELogPolicy_File(void)
    {
      if( fp != NULL )
      {
	fclose(fp);
      }
    }

    /** Log
     *
     *  \brief Log message to a file and flush that file.
     */
    virtual int Log( const wchar_t* string )
    {
      if( fp == NULL )
      {
	return 0;
      }
      
      /* If the file exceeds max_size it must be truncated. */
      int current_size;
#if defined (WIN32) || defined(_WIN64)
      current_size = _filelength(_fileno(fp));
#else
      struct stat stat_buf;
      fstat(fileno(fp), &stat_buf);
      current_size = stat_buf.st_size;
#endif
      if( current_size >= max_size )
      {
	int chsize_result = _chsize(_fileno(fp),0L); /* size length to zero bytes */
	if( chsize_result == -1 )
	{
	  return -1;
	}
      }
      int nchars;
      nchars = fputws(string,fp);

      fflush(fp);

      if( nchars >= 0 )
      {
	current_size += (nchars + 1);
      }
      return nchars;
    }/* Log */

    /** Log
     *
     *  \brief Log message to a file and flush that file.
     */
    virtual int Log( const char* string )
    {
      if( fp == NULL )
      {
	return 0;
      }
      
      /* If the file exceeds max_size it must be truncated. */
      int current_size;
#if defined (WIN32) || defined(_WIN64)
      current_size = _filelength(_fileno(fp));
#else
      struct stat stat_buf;
      fstat(fileno(fp), &stat_buf);
      current_size = stat_buf.st_size;
#endif
      if( current_size >= max_size )
      {
	int chsize_result = _chsize(_fileno(fp),0L); /* size length to zero bytes */
	if( chsize_result == -1 )
	{
	  return -1;
	}
      }
      int nchars;
      nchars = fputs(string,fp);

      fflush(fp);

      if( nchars >= 0 )
      {
	current_size += (nchars + 1);
      }
      return nchars;
    }/* Log */

    /** SetMaxLogSize
     *
     *  \brief Set the maximum size the log can grow to.  When the sizes is reached the
     *         file will be truncated to zero bytes.
     *  \param new_max_size (in) The maximum size for the long in bytes.
     */
    void SetMaxLogSize( long new_max_size )
    {
      max_size = new_max_size;
    }/* SetMaxLogSize */

  private:

    FILE* fp;                  /** File stream for logging. */
    long max_size;             /** Maximum file size (bytes) */
};/* CELogPolicy_File */

#endif /* __CELOG_POLICY_CBUF_HPP__ */
