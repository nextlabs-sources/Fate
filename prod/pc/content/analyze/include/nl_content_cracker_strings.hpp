#include <io.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cassert>
#include "nl_content_cracker.hpp"
#include "nl_content_misc.hpp"

class NLContentCracker_Strings : public NLCA::ContentCracker
{
  public:
    /** NLContentCracker_Strings
     *
     *  \brief Create an IFilter instance for the given file.  Throws HRESULT
     *         if an IFilter object cannot be constructed.
     *
     *  \param file (in) File to construct iFilter for.
     *  \param zero_bytes (in) does reading zero bytes means we should assume eos?
     */
    NLContentCracker_Strings( const wchar_t* file, bool zero_bytes_flag=false ) :
        force_eos(false),
        zero_bytes_means_eos(zero_bytes_flag),
        file_name_is_temp(false)
    {
      assert( file != NULL );
      if( file == NULL )
      {
  	throw (HRESULT)HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
      }

      file_name = file;

      if (nextlabs::windows_fse::is_wfse_installed() && NLCA::CopyFileToTemp(file_name)) 
      {
          file_name_is_temp = true;
      }

      errno_t result = _wsopen_s(&fp,file_name.c_str(),_O_BINARY|_O_RDONLY|_O_SEQUENTIAL,_SH_DENYNO,_S_IREAD);
      if( result != 0 )
      {
  	throw (HRESULT)HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
      }

      /* char_map maps [' ','~') to character and all others to space. */
      memset(char_map,' ',sizeof(char_map)); // all get default of space
      for( unsigned char ch = ' ' ; ch < '~' ; ch++ ) // [' ','~')
      {
	char_map[ch] = ch;   // ch -> ch
      }
      char_map['\t'] = '\t'; // tab -> tab
      char_map['\n'] = '\n'; // newline -> newline
    }/* NLContentCracker_Strings */

    ~NLContentCracker_Strings(void) {
      if(fp != -1)
      {
	_close(fp);	
      }

      if (file_name_is_temp)
      {
          DeleteFile(file_name.c_str());
      }
    }

    bool GetText( wchar_t* buf , unsigned long& buf_size , bool& eos )
    {
      assert( buf != NULL && buf_size > 0 );
      if( buf == NULL )
      {
	return false;
      }
      char* abuf = NULL;

      abuf = new (std::nothrow) char[buf_size];
      if( abuf == NULL )
      {
	eos = true;
	return false;
      }

      int bytes_read = _read(fp,abuf,buf_size-1);  /* read into callers bufer */

      if( bytes_read < 0 || (bytes_read == 0 && zero_bytes_means_eos))                     /* fail or eof */
      {
	delete [] abuf;
	return false;
      }
      eos =_eof(fp) || force_eos;              /* eof reached? */

      /* Remove non-printable chars from the read. */
      for( int i = 0 ; i < bytes_read ; ++i )
      {
	buf[i] = (wchar_t)char_map[abuf[i]]; /* map char to printable char */
      }
      buf_size = bytes_read;          /* bytes read is wchar_t elements */
      buf[buf_size] = (wchar_t)NULL;  /* always terminate string */
      delete [] abuf;
      return true;
    }/* GetText */

    bool Cancel(void)
    {
      force_eos = true;
      return true;
    }/* Cancel */

  private:

    bool zero_bytes_means_eos;   /* should we assume reading zero bytes means eos? */
    bool force_eos;              /* force end-of-stream for cancel? */
    int fp;                      /* file descriptor */
    unsigned char char_map[256]; /* printable character map */
    std::wstring file_name;      /* file name */
    bool file_name_is_temp;      /* file is a temp file that should be deleted */

};/* NLContentCracker_Strings */
