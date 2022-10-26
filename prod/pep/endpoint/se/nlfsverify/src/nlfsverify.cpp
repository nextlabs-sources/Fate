/****************************************************************************************
 *
 * NextLabs FS Verifier
 *
 * Try and destroy any data modification driver.
 *
 *
 * TODO:
 *
 *   - Support block size other than 1 byte
 *   - Support for random I/O sequences
 *   - Support for random truncation
 *   - Improve data generation
 *
 ***************************************************************************************/

#include <windows.h>
#include <process.h>
#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <share.h>
#include <errno.h>
#include <cstdio>
#include <cstdlib>
#include <time.h>
#include <string>
#include <list>

#include <boost/scoped_ptr.hpp>

#define WRITE_OP 0
#define READ_OP 1

void print_help(void)
{
  fprintf(stdout, "NextLabs FileSystem Verifier (Built %s %s)\n", __DATE__, __TIME__);
  fprintf(stdout, "usage: nlfsverify [option] ...\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "Options:\n");
  fprintf(stdout, "   --file=[file name]         File name. Default is 'foo.bin' in the current\n");
  fprintf(stdout, "                              working directory.  File will be truncated.\n");
  fprintf(stdout, "   --file-size=[count]        File size (bytes). Default is 1MB (2^20).\n");
  fprintf(stdout, "   --block-size=[count]       Block size (bytes). Default is 1.\n");
  fprintf(stdout, "   --verbose                  Verbose output.\n");
  fprintf(stdout, "   --debug                    Debug output. Implies '--verbose'.\n");
  fprintf(stdout, "   --help                     This screen.\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "File Generation Parameters:\n");
  fprintf(stdout, "   --filegen                  Generate a File.\n");
  fprintf(stdout, "   --seed=[value]             Seed Value for Randomness. Default is 1.\n");
  fprintf(stdout, "   --operations=[count]       Number of Operations. Default is 10.\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "When a block compare fails the offset of the compare failure is shown and the\n");
  fprintf(stdout, "program exit value is non-zero.\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "Examples:\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "  nlfsverify --file=c:\\foo.bin --file-size=1000 --block-size=1024\n");
  fprintf(stdout, "  nlfsverify --file=c:\\efs\\foo.bin --file-size=8192 --debug\n");
}/* print_help */

static std::size_t param_file_size  = 1 << 20;      // file size in bytes (2^20/1MB)
static std::size_t param_block_size = 1;            // block size in bytes
static std::size_t param_seed = 1;                  // seed value for randomness
static unsigned long param_operations = 1;            // Number of operations
static bool        op_filegen       = false;        // file generation?
static bool        op_verbose       = false;        // verbose ouptut?
static bool        op_debug         = false;        // debug ouptut?

/** transform_block
 *
 *  \brief Transform a block given its offset.
 *
 *  \param buf (in)        Buffer size.
 *  \param buf_size (in)   Buffer size in bytes.
 *  \param block_size (in) Block size in bytes.
 *  \param offset (in)     Offset where 'buf' begins into the content.
 */
static void transform_block( _In_ unsigned char* buf ,
			     std::size_t buf_size ,
			     std::size_t block_size ,
			     std::size_t offset )
{
  for( std::size_t i = 0 ; i < buf_size ; i++ )
  {
    //buf[i] = 0x41/*'A'*/; //(unsigned char)(offset % 16);
    buf[i] = (unsigned char)(offset % 253);
  }
}/* transform_block */

int wmain( int argc , wchar_t** argv )
{
  if( argc <= 1 )
  {
    print_help();
    return 0;
  }

  std::wstring file_name(L"foo.bin");
  /* Process options */
  for( int i = 1 ; i < argc ; ++i )
  {
    wchar_t* option = wcsstr(argv[i],L"=");
    if( option != NULL )
    {
      option++;
    }

    if( wcsncmp(argv[i],L"--help",wcslen(L"--help")) == 0 )
    {
      print_help();
      return 0;
    }
    if( wcsncmp(argv[i],L"--verbose",wcslen(L"--verbose")) == 0 )
    {
      op_verbose = true;
      continue;
    }
	if( wcsncmp(argv[i],L"--filegen",wcslen(L"--filegen")) == 0 )
    {
      op_filegen = true;
      continue;
    }
    if( wcsncmp(argv[i],L"--debug",wcslen(L"--debug")) == 0 )
    {
      op_verbose = true;
      op_debug = true;
      continue;
    }
    if( wcsncmp(argv[i],L"--file=",wcslen(L"--file=")) == 0 )
    {
      file_name.assign(option);
      continue;
    }
    if( wcsncmp(argv[i],L"--file-size=",wcslen(L"--file-size=")) == 0 )
    {
      param_file_size = _wtoi(option);
      continue;
    }
    if( wcsncmp(argv[i],L"--block-size=",wcslen(L"--block-size=")) == 0 )
    {
      //fprintf(stdout, "nlfsverify: unsupported option, using default.\n");
      param_block_size = _wtoi(option);
      continue;
    }
	if( wcsncmp(argv[i],L"--seed=",wcslen(L"--seed=")) == 0 )
    {
      param_seed = _wtoi(option);
      continue;
    }
	if( wcsncmp(argv[i],L"--operations=",wcslen(L"--operations=")) == 0 )
    {
      param_operations = _wtoi(option);
      continue;
    }
  }/* for */

  int fd = -1;      // file descriptor for i/o
  errno_t err = 0;  // i/o error handling

  /*********************************************************************************************
   * Create file and write blocks until the given size is reached.  Each block is transformed
   * to correct data.
   ********************************************************************************************/
  err = _wsopen_s(&fd,file_name.c_str(),_O_RDWR|_O_CREAT|_O_TRUNC|_O_BINARY,_SH_DENYRW,_S_IREAD|_S_IWRITE);
  if( err != 0 )
  {
    fprintf(stderr, "nlfsverify: _wsopen_s failed (errno %d,le %d) when create file\n",errno,GetLastError());
    return 1;
  }

  if( op_verbose == true )
  {
    fprintf(stdout, "nlfsverify: creating file: file_size = %d, block_size = %d\n" , param_file_size, param_block_size);
  }

  boost::scoped_ptr<unsigned char> buf( new unsigned char[param_block_size] );     // write buffer
  boost::scoped_ptr<unsigned char> rb_buf( new unsigned char[param_block_size] );  // read back buffer

  LARGE_INTEGER perf_time_start;
  LARGE_INTEGER perf_time_end;
  LARGE_INTEGER perf_freq;
  if( QueryPerformanceFrequency(&perf_freq) == FALSE )
  {
    fprintf(stderr, "nlfsverify: QueryPerformanceFrequency failed (le %d)\n", GetLastError());
    return 1;
  }

  int rv = -1;
  std::size_t offset = 0;

  QueryPerformanceCounter(&perf_time_start);
  for( offset = 0 ; offset < param_file_size ; offset += param_block_size )
  {
    transform_block(buf.get(),param_block_size,param_block_size,offset);

    rv = _write(fd,buf.get(),static_cast<unsigned int>(param_block_size));
    if( op_debug == true )
    {
      fprintf(stdout, "nlfsverify: wrote %d bytes @ %d (%d)\n", param_block_size, offset, rv);
    }
    if( rv == -1 )
    {
      fprintf(stderr, "nlfsverify: error: _write failed (errno %d,le %d)\n", errno, GetLastError());
      exit(1);
    }
    if( rv != (int)param_block_size )
    {
        fprintf(stderr, "nlfsverify: error:  (file size=%d) _write size not correct at offste %d (to read: %d, read: %d) (errono %d,le %d)\n", param_file_size, offset, param_block_size, rv, errno, GetLastError());
        return 1;
    }
  }

  if(op_filegen)
  {
	  srand(static_cast<unsigned int>(param_seed));
	  long pos;
	  int random;
	  int no_of_bytes;
	  int operation;
	  for( unsigned long i = 0; i < param_operations; i++ )
	  {
		  /* Seek the beginning of the file: */
		  pos = _lseek( fd, 0L, SEEK_SET );
          if( pos == -1L )
			perror( "_lseek to beginning failed" );

		  random = rand();
		  no_of_bytes = random;
 
		  offset = random % param_file_size;
		  
  		  /* Seek into the file: */
		  pos = _lseek( fd, static_cast<long>(offset), SEEK_CUR );
		  if( pos == -1L )
		    perror( "_lseek to beginning failed" );

		  operation = random % 2;
		  if( op_debug == true )
		  {
			  fprintf(stdout, "nlfsverify: File_Size= %d, Random=No_of_bytes= %d, Offset=%d, Operation= %d.\n", param_file_size, random, offset, operation);
		  }
		  switch(operation)
		  {
			case WRITE_OP:
				{
					int temp = static_cast<int>(param_file_size-(offset+no_of_bytes));
					if(temp < 0) no_of_bytes = static_cast<int>(param_file_size-offset);
					boost::scoped_ptr<unsigned char> writebuf( new unsigned char[no_of_bytes] );     // write buffer

					transform_block(writebuf.get(),no_of_bytes,no_of_bytes,offset);
					
					rv = _write(fd,writebuf.get(),no_of_bytes);
					if( rv == -1 )
					{
					  fprintf(stderr, "nlfsverify: error: _write failed (errono %d,le %d)\n", errno, GetLastError());
					  return 1;
					}
					if( op_debug == true )
					{
					  fprintf(stdout, "nlfsverify: wrote %d bytes @ %d (%d)\n", no_of_bytes, offset, rv);
					}
					break;
				}
			case READ_OP:
				{
				boost::scoped_ptr<unsigned char> readbuf( new unsigned char[no_of_bytes] );  // read back buffer

				rv = _read(fd,readbuf.get(),no_of_bytes);
				if( rv == -1 )
				{
				  fprintf(stderr, "nlfsverify: error: _read failed (errono %d,le %d)\n", errno, GetLastError());
				  return 1;
				}
				if( op_debug == true )
				{
				  fprintf(stdout, "nlfsverify: read %d bytes @ %d (%d)\n", rv, offset, rv);
				}
				break;
				}
		  }
	  }
	  if( _close(fd) == -1 )
	  {
		fprintf(stderr, "nlfsverify: error: _close failed (errno %d,le %d)\n", errno, GetLastError());
		return 1;
	  }
	  return 1;
  }

  if( _close(fd) == -1 )
  {
    fprintf(stderr, "nlfsverify: error: _close failed (errno %d,le %d)\n", errno, GetLastError());
    return 1;
  }
  fd = -1;

  /*********************************************************************************************
   * Read back file.  Open the file and read back blocks and compare to generated blocks based
   * on the offset.
   ********************************************************************************************/
  err = _wsopen_s(&fd,file_name.c_str(),_O_RDONLY|_O_BINARY,_SH_DENYRW,_S_IREAD);
  if( err != 0 )
  {
    fprintf(stderr, "nlfsverify: _wsopen_s failed (errno %d,le %d) when read back\n",errno,GetLastError());
    return 1;
  }
  for( offset = 0 ; offset < param_file_size ; offset += param_block_size )
  {
    transform_block(buf.get(),param_block_size,param_block_size,offset);

    rv = _read(fd,rb_buf.get(),static_cast<unsigned int>(param_block_size));
    if( op_debug == true )
    {
      fprintf(stdout, "nlfsverify: read %d bytes @ %d (%d)\n", param_block_size, offset, rv);
    }
    if( rv == -1 )
    {
        fprintf(stderr, "nlfsverify: error: _read failed (errono %d,le %d)\n", errno, GetLastError());
        return 1;
    }
    if( rv != (int)param_block_size )
    {
        fprintf(stderr, "nlfsverify: error:  (file size=%d) _read size not correct at offste %d (to read: %d, read: %d) (errono %d,le %d)\n", param_file_size, offset, param_block_size, rv, errno, GetLastError());
        return 1;
    }

    /* compare generated block with read block */
    if( memcmp(buf.get(),rb_buf.get(),param_block_size) != 0 )
    {
      fprintf(stderr,"nlfsverify: error: (file size=%d) compare failed at offset %d read=%03d, write=%03d\n", param_file_size, offset, *rb_buf.get(), *buf.get());
      return 1;
    }
  }
  if( _close(fd) == -1 )
  {
    fprintf(stderr, "nlfsverify: error: _close failed (errno %d, le %d)\n", errno, GetLastError());
    return 1;
  }
  QueryPerformanceCounter(&perf_time_end);

  LARGE_INTEGER perf_time_diff; // elapsed time between start and end
  perf_time_diff.QuadPart = perf_time_end.QuadPart - perf_time_start.QuadPart;

  /* The difference must be normalized to milliseconds.  perf_freq is counts per
   * second.  This is normalized to counts per millisecond.
   */
  std::size_t curr_time = (std::size_t)(perf_time_diff.QuadPart / (perf_freq.QuadPart / 1000));

  if( op_verbose == true )
  {
    fprintf(stdout, "nlfsverify: complete (%dms)\n", curr_time);
  }

  return 0;
}/* main */
