
#include <windows.h>
#include <string>
#include <imagehlp.h>
#include <cstdio>
#include <cstdlib>
#include <wchar.h>
#include <psapi.h>

#pragma warning( push )
#pragma warning( disable : 6011 )
#  include <boost/algorithm/string.hpp>
#pragma warning( pop )

static std::string get_error_string( DWORD in_le )
{
  char buf[256] = {0};
  FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM,NULL,in_le,0,buf,_countof(buf),NULL);
  std::string out_buf(buf);

  boost::algorithm::replace_all(out_buf,"\n","");
  boost::algorithm::replace_all(out_buf,"\r","");

  return out_buf;
}/* get_error_string */

/** process_snapshot
 *
 *  \brief Take a snapshot of a process by its process ID (pid).
 *
 *  \param pid (in)  Process ID of process to snapshot.
 *  \param file (in) File to store snapshot.
 */
static bool process_snapshot( int pid ,
			      __in const char* file )
{
  bool result = true;  /* dump result */
  HANDLE ph;           /* handle to process for minidump write */
  HANDLE fh;           /* handle to file for minidump write */

  ph = OpenProcess(PROCESS_ALL_ACCESS,FALSE,pid);
  if( ph == NULL )
  {
    fprintf(stderr, "nlpsnap: cannot open process PID %d (le %d,%s)\n",
	    pid, GetLastError(), get_error_string(GetLastError()).c_str());
    return false;
  }

  fh = CreateFileA(file,GENERIC_READ|GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
  if( fh == INVALID_HANDLE_VALUE )
  {
    fprintf(stderr, "nlpsnap: cannot open %s (le %d,%s)\n",
	    file, GetLastError(), get_error_string(GetLastError()).c_str());
    CloseHandle(ph);
    return false;
  }

  MINIDUMP_TYPE type = (MINIDUMP_TYPE)(MiniDumpWithFullMemory |
				       MiniDumpWithDataSegs |
				       MiniDumpWithHandleData |
				       MiniDumpWithProcessThreadData);

  fprintf(stdout, "nlpsnap: writing snapshot\n");
  BOOL rv = MiniDumpWriteDump(ph,pid,fh,type,NULL,NULL,NULL);
  if( rv != TRUE )
  {
    fprintf(stderr, "nlpsnap: MiniDumpWriteDump failed (le %d,%s)\n",
	    GetLastError(), get_error_string(GetLastError()).c_str());
    result = false;
  }

  CloseHandle(ph);
  CloseHandle(fh);

  return result;
}/* process_snapshot */

int main( int argc , char** argv )
{
  if( argc != 2 && argc != 3 )
  {
    fprintf(stdout, "Usage: nlpsnap [pid] [optional file]\n");
    fprintf(stdout, "Create a snapshot (dump file) of an active process.\n");
    fprintf(stdout, "\n");
    fprintf(stdout, "  [pid]  Process ID of process to snapshot\n");
    fprintf(stdout, "  [file] File to store snapshot\n");
    return 0;
  }

  std::string file;
  if( argc == 3 )
  {
    file.assign(argv[2]);  /* accept input param */
  }
  else
  {
    /* derive name from image name */
    char mname[MAX_PATH] = {0};
    GetProcessImageFileNameA(GetCurrentProcess(),mname,_countof(mname));  
    file.assign(mname);
    file.append(".dmp");
    std::string::size_type i = file.find_last_of("\\/");
    if( i != std::string::npos )
    {
      file.erase(0,i+1);
    }
  }

  int pid = atoi(argv[1]);
  if( pid == 0 )
  {
    fprintf(stderr, "nlpsnap: '%s' is not a process id.\n", argv[1]);
    return 1;
  }
  fprintf(stdout, "nlpsnap: creating a dump of process %d to file %s\n",
	  pid, file.c_str());

  bool result = process_snapshot(pid,file.c_str());
  if( result == false )
  {
    fprintf(stderr, "nlpsnap: cannot create dump (le %d,%s)\n",
	    GetLastError(),get_error_string(GetLastError()).c_str());
    return 1;
  }
  fprintf(stdout, "nlpsnap: complete\n");
  return 0;
}/* main */
