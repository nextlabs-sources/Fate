/****************************************************************************************
 *
 * NextLabs System Encryption Tool
 *
 ***************************************************************************************/

#include <windows.h>
#include <string>
#include <iostream>
#include <list>

#include "NextLabsEncryption_Types.h"
#include "nl_sysenc_lib.h"
#include "nl_sysenc_lib_fw.h"
#include "eframework/resattr/resattr_loader.hpp"

#include "nlsysenc_efilter.hpp"

#include "nextlabs_feature_manager.hpp"
#include "nextlabs_features.h"

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

nextlabs::resattr_loader g_resattr_loader;

bool op_encrypt = false;   // encrypt files?
bool op_incdirs = false;   // include directories?
bool op_recurse = false;   // recursive?
bool op_fastwrite = false; // Fast-Write?
bool op_fullpaths = false; // show full paths?
bool op_all = false;       // show all files?
bool op_debug = false;     // debug output?
bool op_filter  = false;   // user filter during traversal?
bool op_exitonerror = false; // exit on first error?
bool op_logfailures = false; // print a list of files we failed to encrypt?
int max_encryption_attempts = 3;  // make at least these many (unsuccessful) attempts before quitting 

std::wstring op_filter_regex;

std::list<std::wstring> encryptionfailures;

static void print_help(void)
{
  fprintf(stdout, "Enterprise System Encryption %d.%d (Built %s %s)\n",
	  VERSION_MAJOR, VERSION_MINOR,
	  __DATE__, __TIME__);
  fprintf(stdout, "usage: nlSysEncryption [/path] [files] [option] ...\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "Options:\n");
  fprintf(stdout, "   --encrypt, -e, /e   Encrypt files.  When directory inclusion is specified\n");
  fprintf(stdout, "                       the DRM option will be set on directories.\n");
  fprintf(stdout, "   --incdirs, -i, /i   Include directories.  DRM will be set on all directories\n");
  fprintf(stdout, "                       when encryption (i.e. -e) is set.\n");
  fprintf(stdout, "   --recurse, -r, /r   Recursive file traverasl.  Includes subdirectories.\n");
  fprintf(stdout, "   --heavywrite, -hw, /hw  The passed file(s)/directory should be encrypted as\n");
  fprintf(stdout, "                       Heavy-Write file(s)/directory (requires -e)\n");
  fprintf(stdout, "   --full, -f, /f      Show full file paths.\n");
  fprintf(stdout, "   --all, -a, /a       Show all files whether encrypted or not.\n");
  fprintf(stdout, "   --help, -h, /h, /?  This screen.\n");
#if 0
  // We might add these to the help info later, but not right now
  fprintf(stdout, "   --exitonerror       Quit after the first error\n");
  fprintf(stdout, "   --logfailures       Print a list of files we were unable to encrypt\n");
  fprintf(stdout, "   --attempts=n        Make 'n' attempts to encrypt a file before giving up\n");
#endif
  fprintf(stdout, "\n");
  fprintf(stdout, "Examples:\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "  nlSysEncryption C:\\Docs\\Confidential\\*.*\n");
  fprintf(stdout, "  nlSysEncryption /path C:\\Docs\\Confidential\\*.*\n");
  fprintf(stdout, "  nlSysEncryption C:\\Docs\\Confidential\\*.doc -e -i\n");
}/* print_help */

static void display( _In_ const wchar_t* in_file,bool is_directory )
{
  bool is_encrypted = false;
  bool is_encrypted_fw = false;
  std::wstring fname(in_file);
  std::wstring::size_type off = fname.find_last_of(L"\\/");
  if( off != std::wstring::npos )
  {
    fname.erase(0,off+1);  /* trim leading directory to get the file name only */
  }

  NextLabsFile_Header_t nl;
  NextLabsEncryptionFile_Header_t se;
  NextLabsFile_Header_1_0_t nl10;
  NextLabsEncryptionFile_Header_1_0_t se10;

  memset(&nl,0x00,sizeof(nl));
  memset(&se,0x00,sizeof(se));
  memset(&nl10,0x00,sizeof(nl10));
  memset(&se10,0x00,sizeof(se10));

  char key_ring_name[NLE_KEY_RING_NAME_MAX_LEN+1] = {0};
  char hash[512] = {0};

  is_encrypted = SE_IsEncrypted(in_file);
  is_encrypted_fw = SE_IsEncryptedFW(in_file);

  if( is_encrypted_fw == true )
  {
    BOOL rv_nl, rv_se;
    rv_nl = SE_GetFileInfoFW(in_file,SE_FileInfo_NextLabs,&nl10);
    rv_se = SE_GetFileInfoFW(in_file,SE_FileInfo_NextLabsEncryption,&se10);

    if( rv_nl == TRUE && rv_se == TRUE )
    {
      memcpy(key_ring_name,se10.pcKeyRingName,NLE_KEY_RING_NAME_MAX_LEN);

      /* Write out the Key ID to a string for printing */
      for( size_t i = 0 ; i < _countof(se10.pcKeyID.hash) ; i++ )
      {
        char temp[32] = {0};
        _snprintf_s(temp,_countof(temp), _TRUNCATE,"%02x",se10.pcKeyID.hash[i]);
        strncat_s(hash,_countof(hash),temp, _TRUNCATE);
      }
    }
  }
  else if( is_encrypted == true )
  {
    BOOL rv_nl, rv_se;
    rv_nl = SE_GetFileInfo(in_file,SE_FileInfo_NextLabs,&nl);
    rv_se = SE_GetFileInfo(in_file,SE_FileInfo_NextLabsEncryption,&se);

    if( rv_nl == TRUE && rv_se == TRUE )
    {
      memcpy(key_ring_name,se.pcKeyRingName,NLE_KEY_RING_NAME_MAX_LEN);

      /* Write out the Key ID to a string for printing */
      for( size_t i = 0 ; i < _countof(se.pcKeyID.hash) ; i++ )
      {
        char temp[32] = {0};
        _snprintf_s(temp,_countof(temp), _TRUNCATE,"%02x",se.pcKeyID.hash[i]);
        strncat_s(hash,_countof(hash),temp, _TRUNCATE);
      }
    }
  }

  if( is_encrypted == false )
  {
    if( op_recurse == false || op_all == true )
    {
      fwprintf(stdout, L" (not encrypted)                %s\n",
	       op_fullpaths == true ? in_file : fname.c_str());
    }
    return;
  }

  if( op_debug == true  )
  {
    if( is_encrypted_fw == true )
    {
      if( is_directory == true )
      {
        fwprintf(stdout, L" (directory)                                                                                   (heavy-write) %s\n", fname.c_str());
      }
      else
      {
        fwprintf(stdout, L" %d.%d %d.%d %-16hs %-16hs 0x%x (heavy-write) %s\n",
                 nl10.version_major, nl10.version_minor,
                 se10.version_major, se10.version_minor,
                 key_ring_name,
                 hash,
                 se10.pcKeyID.timestamp,
                 op_fullpaths == true ? in_file : fname.c_str());
      }
    }
    else
    {
      if( is_directory == true )
      {
        fwprintf(stdout, L" (directory)                                                                                                 %s\n", fname.c_str());
      }
      else
      {
        fwprintf(stdout, L" %d.%d %d.%d %-16hs %-16hs 0x%x               %s\n",
                 nl.version_major, nl.version_minor,
                 se.version_major, se.version_minor,
                 key_ring_name,
                 hash,
                 se.pcKeyID.timestamp,
                 op_fullpaths == true ? in_file : fname.c_str());
      }
    }
  }
  else
  {
    if( is_directory == true )
    {
      strncpy_s(key_ring_name,_countof(key_ring_name),"(directory)",_TRUNCATE);
    }

    fwprintf(stdout, L" %-16hs %s %s\n",
	     key_ring_name,
         is_encrypted_fw == true ? L"(heavy-write)" : L"             ",
	     op_fullpaths == true ? in_file : fname.c_str());
  }
}/* display */

/** is_process_file
 *
 *  \brief Return true if the file should be processed.
 */
static bool is_process_file( _In_ const wchar_t* in_file )
{
  assert( in_file != NULL );

  if( op_filter == false )
  {
    return true;
  }

  /* get just the file name which is to be filtered */
  std::wstring file(in_file);
  std::wstring::size_type i = file.find_last_of(L"\\/");
  if( i != std::wstring::npos )
  {
    file.erase(0,i+1);
  }

  bool matched = false;
  boost::regex::flag_type flags = boost::regex::icase | boost::regex::no_except;
  boost::wregex filter_regex(op_filter_regex.c_str(), flags);
  boost::wsmatch what;

  try
  {
    matched = boost::regex_match( file, what, filter_regex );
  }
  catch(...)
  {
  }

  return matched;
}

/** process_file
 *
 *  \brief Process a file for encryption or information
 */
static bool process_file( _In_ const wchar_t* in_file,bool is_top_dir=false )
{
  if(op_filter==true&&is_top_dir==true&& is_process_file(in_file) == false )
  {
    return true;  /* omit the file */
  }
 
  DWORD attrs = GetFileAttributesW(in_file);
  if (attrs == INVALID_FILE_ATTRIBUTES)
  {
      std::wcout << L"File " << in_file << L" has INVALID_FILE_ATTRIBUTES" << std::endl;
    return false;
  }

  bool is_directory = (attrs & FILE_ATTRIBUTE_DIRECTORY);

  if( op_encrypt == false )
  {
    display(in_file,is_directory);
    return true;
  }


  /***********************************************************************
   * The following handles encrypting files and/or directories
   **********************************************************************/

  if( op_encrypt == true && SE_IsEncrypted(in_file) == TRUE )
  {
    fwprintf(stdout, L"nlSysEncryption: %s is already encrypted\n", in_file);
    return true;
  }

  /* A directory should be encrypted if one of the following conditions is true:
   *
   *  (1) Option to include directories was explicity set.
   *  (2) Option to recurse was not set.
   */
  bool encrypt_dir =
    (is_directory && op_incdirs) ||
    (is_directory && op_recurse == false);
  BOOL ret;

  int encryption_attempts = 1;

  if( encrypt_dir == true )
  {
    while(true)
    {
        if (encryption_attempts > 1)
        {
            Sleep(2000);
            fwprintf(stdout, L"Encrypting %s (attempt %d)\n", in_file, encryption_attempts);
        }
        else
        {
            fwprintf(stdout, L"Encrypting %s\n", in_file);
        }

        DWORD lastErr;

        if( op_fastwrite == true )
        {
            ret = SE_EncryptDirectoryFW(in_file);
        }
        else
        {
            ret = SE_EncryptDirectory(in_file);
        }
        
        if( ret == TRUE)
        {
            return true;
        }

        lastErr = GetLastError();

        if( ret == FALSE && ( lastErr == ERROR_NOT_SUPPORTED || ++encryption_attempts > max_encryption_attempts ) )
        {
            fwprintf(stderr, L"nlSysEncryption: Cannot encrypt directory %s (le %d)\n", in_file, lastErr);
            encryptionfailures.push_back(in_file);
            return false;
        }
    }
  }

  bool encrypt_file = (!is_directory);  /* if not a directory it is a file */
  if( encrypt_file == true )
  {
      while(true)
      {
          if (encryption_attempts > 1)
          {
              Sleep(2000);
              fwprintf(stdout, L"Encrypting %s (attempt %d)\n", in_file, encryption_attempts);
          }
          else
          {
              fwprintf(stdout, L"Encrypting %s\n", in_file);
          }

          DWORD lastErr = ERROR_SUCCESS;

          if( op_fastwrite == true )
          {
              ret = SE_EncryptFileFW(in_file);
              lastErr = GetLastError();
          }
          else
          {
              // Read existing attributes in the original file format before
              // encrypting the file.  Write the attributes to the encrypted
              // file so that they go into NextLabs file header.
              //
              // NOTE:
              // - CreateAttributeManager() and AllocAttributes() return zero
              //   on success.
              // - ReadResourceAttributes() and WriteResourceAttributes()
              //   return non-zero on success.
              // Don't ask me why.
              ResourceAttributeManager *mgr;
              ResourceAttributes *resAttrs;

              ret = FALSE;
			  
			  SYSTEMTIME	StStart;
			  SYSTEMTIME	StEnd;
			  SYSTEMTIME	StEncStart;
			  SYSTEMTIME	StEncEnd;
			  
			  memset(&StStart, 0, sizeof(SYSTEMTIME));
			  memset(&StEnd, 0, sizeof(SYSTEMTIME));
			  memset(&StEncStart, 0, sizeof(SYSTEMTIME));
			  memset(&StEncEnd, 0, sizeof(SYSTEMTIME));

			  GetLocalTime(&StStart);
			  
              if (!g_resattr_loader.ensure_loaded())
              {
                return false;
              }

              if (g_resattr_loader.m_fns.CreateAttributeManager(&mgr) == 0)
              {
                if (g_resattr_loader.m_fns.AllocAttributes(&resAttrs) == 0)
                {
                  if (g_resattr_loader.m_fns.ReadResourceAttributesW(mgr, in_file, resAttrs) != 0)
                  {
				    BOOL IsEncrypted;
					
					GetLocalTime(&StEncStart);
				    IsEncrypted = SE_EncryptFile(in_file);
					GetLocalTime(&StEncEnd);
                    if (IsEncrypted)
                    {
                      ret = (g_resattr_loader.m_fns.WriteResourceAttributesW(mgr, in_file, resAttrs)
                             != 0);
                    }

                    lastErr = GetLastError();
                  }

                  g_resattr_loader.m_fns.FreeAttributes(resAttrs);
                }

                g_resattr_loader.m_fns.CloseAttributeManager(mgr);
              }		  
			  
			  GetLocalTime(&StEnd);
			  
			  printf("\n");
			  printf("Time consumed:\n");
			  printf("Start:    %04d-%02d-%02d %02d:%02d:%02d.%03d\n", StStart.wYear, StStart.wMonth, StStart.wDay, StStart.wHour, StStart.wMinute, StStart.wSecond, StStart.wMilliseconds);
			  printf("End:      %04d-%02d-%02d %02d:%02d:%02d.%03d\n", StEnd.wYear, StEnd.wMonth, StEnd.wDay, StEnd.wHour, StEnd.wMinute, StEnd.wSecond, StEnd.wMilliseconds);
			  printf("EncStart: %04d-%02d-%02d %02d:%02d:%02d.%03d\n", StEncStart.wYear, StEncStart.wMonth, StEncStart.wDay, StEncStart.wHour, StEncStart.wMinute, StEncStart.wSecond, StEncStart.wMilliseconds);
			  printf("EncEnd:   %04d-%02d-%02d %02d:%02d:%02d.%03d\n", StEncEnd.wYear, StEncEnd.wMonth, StEncEnd.wDay, StEncEnd.wHour, StEncEnd.wMinute, StEncEnd.wSecond, StEncEnd.wMilliseconds);
			  printf("\n");
          }
  
          if( ret == FALSE && ( lastErr == ERROR_NOT_SUPPORTED || ++encryption_attempts > max_encryption_attempts ) )
          {
              fwprintf(stderr, L"nlSysEncryption: Cannot encrypt file %s (le %d)\n", in_file, lastErr);
              encryptionfailures.push_back(in_file);
              return false;
          }

          if( ret == TRUE)
          {
              break;
          }
      }
  }

  return true;
}/* process_file */

/* Determine if the given path is EFS-encrypted
 */
static bool is_efs_encrypted( _In_ const wchar_t* in_path )
{
  DWORD attrs = GetFileAttributesW(in_path);

  return ( attrs != INVALID_FILE_ATTRIBUTES &&
           (attrs & FILE_ATTRIBUTE_ENCRYPTED) != 0 );
}/* is_efs_encrypted */


/** scan
 *
 *  \brief Scan the given path.
 *
 *  \params include_root (in) Determines if the given path should be processed itself as
 *                            an object.
 *           
 */
static bool scan( _In_ const wchar_t* path ,
		  bool include_root, bool is_top_dir=false )
{
  fs::wpath full_path(path);
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
    std::wcout << path << " does not exist" << std::endl;

    return false; /* path does not exist */
  }

  if( fs::is_directory(full_path) )
  {
	/* If a trailing slash exists strip it to make a directory name */
	std::wstring directory(path);
	if( boost::algorithm::iends_with(directory,L"\\") == true ||
	boost::algorithm::iends_with(directory,L"/") == true )
	{
	  directory.erase(directory.length()-1);
	}
	BOOL is_encrypted = SE_IsEncrypted(directory.c_str());
	BOOL is_encrypted_fw = SE_IsEncryptedFW(directory.c_str());
	if( op_encrypt == false )
	{
	  fwprintf(stdout,L"Directory of %s %s%s\n",
		   directory.c_str(),
		   is_encrypted_fw ? L"(heavy-write) " : L"",
		   is_encrypted ? L"(encrypted)" : L"(not encrypted)");
	}
  }

  /* process this object */
  if( include_root == true )
  {
      if (!process_file(path) && op_exitonerror) {
          std::wcout << L"Failed processing " << path << std::endl;
          return false;
      }
  }

  if( !fs::is_directory(full_path))
  {
    return true; /* not a directory - nothing more to do */
  }

  if( is_efs_encrypted(full_path.file_string().c_str()) && op_encrypt )
  {
    return true; /* EFS directory and not view-only - nothing more to do */
  }

  fs::wdirectory_iterator end_iter;

  /******************************************************************************************
   * Processing files and directories.
   *
   * Order: (1) files
   *        (2) directories
   *****************************************************************************************/

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
      /* Show this object if:
       *
       *   (1) It is a file
       *   (2) It is a directory and traversal is not recursive.
       */
      if( fs::is_regular_file(it->status()) || (fs::is_directory(it->status()) && !op_recurse) )
      {
          if (!process_file(it->path().file_string().c_str(),is_top_dir) && op_exitonerror) {
              std::cout << "Failed processing" << it->path().file_string().c_str() << std::endl;
              return false;
          }
      }
    }
    catch( const std::exception& ex )
    {
      std::cout << "nlSysEncryption: Error accessing " << it->path().file_string().c_str() << " " << ex.what() << std::endl;
      if (op_exitonerror) {
          return false;
      }
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
	if( op_recurse == true )
	{
		if(op_filter==true&&is_top_dir==true)
		{
                    if(is_process_file(it->path().file_string().c_str()) == true ) {
                        if (!scan(it->path().file_string().c_str(),true) && op_exitonerror) {
                            std::cout << "Failed sanning" << it->path().file_string().c_str() << std::endl;
                            return false;
                        }
                    }
		}
		else {
                    if (!scan(it->path().file_string().c_str(),true) && op_exitonerror) {
                        std::cout << "Failed sanning" << it->path().file_string().c_str() << std::endl;
                        return false;
                    }
              }
	}
      }
    }
    catch( const std::exception& ex )
    {
      std::cout << "nlSysEncryption: Error accessing " << it->path().file_string().c_str() << " " << ex.what() << std::endl;
      if (op_exitonerror) {
          return false;
      }
    }
  }/* for */
  return true;
}/* scan */

int wmain( int argc , wchar_t** argv )
{
  /* Install exception filter for minidump generation */
  SetUnhandledExceptionFilter(efilter);

  nextlabs::feature_manager feat;

  feat.open();
  if( feat.is_enabled(NEXTLABS_FEATURE_ENCRYPTION_SYSTEM) == false )
  {
    fprintf(stdout, "NextLabs System Encryption cannot be started. This machine does not have an active license.\n");
    return 1;
  }

  int start_arg = 2;
  /* Handle 'nlsysencryption /path [path] ...' parameter format */
  if( argc > 1 && _wcsicmp(argv[1],L"/path") == 0 )
  {
    start_arg++;
  }

  /* Process options */
  for( int i = start_arg ; i < argc ; ++i )
  {
    if( _wcsicmp(argv[i],L"--encrypt") == 0 ||             /* encrypt */
	_wcsicmp(argv[i],L"-e") == 0 ||
	_wcsicmp(argv[i],L"/e") == 0 )
    {
      op_encrypt = true;
    }
    else if( _wcsicmp(argv[i],L"--incdirs") == 0 ||        /* include dirs */
	     _wcsicmp(argv[i],L"-i") == 0 ||
	     _wcsicmp(argv[i],L"/i") == 0 )
    {
      op_incdirs = true;
    }
    else if( _wcsicmp(argv[i],L"--recurse") == 0 ||        /* recursive? */
	     _wcsicmp(argv[i],L"-r") == 0 ||
	     _wcsicmp(argv[i],L"/r") == 0 )
    {
      op_recurse = true;
    }
    else if( _wcsicmp(argv[i],L"--heavywrite") == 0 ||     /* Fast-Write? */
	     _wcsicmp(argv[i],L"-hw") == 0 ||
	     _wcsicmp(argv[i],L"/hw") == 0 )
    {
      op_fastwrite = true;
    }
    else if( _wcsicmp(argv[i],L"--debug") == 0 ||          /* debug */
	     _wcsicmp(argv[i],L"-d") == 0 ||
	     _wcsicmp(argv[i],L"/d") == 0 )
    {
      op_debug = true;
    }
    else if( _wcsicmp(argv[i],L"--full") == 0 ||           /* full paths? */
	     _wcsicmp(argv[i],L"-f") == 0 ||
	     _wcsicmp(argv[i],L"/f") == 0 )
    {
      op_fullpaths = true;
    }
    else if( _wcsicmp(argv[i],L"--all") == 0 ||           /* all files? */
	     _wcsicmp(argv[i],L"-a") == 0 ||
	     _wcsicmp(argv[i],L"/a") == 0 )
    {
      op_all = true;
    }
    else if (_wcsicmp(argv[i],L"--exitonerror") == 0)
    {
      op_exitonerror = true;
    }
    else if (_wcsicmp(argv[i],L"--logfailures") == 0)
    {
      op_logfailures = true;
    }
    else if (_wcsnicmp(argv[i],L"--attempts=", 11) == 0)
    {
      max_encryption_attempts = _wtoi(argv[i]+11);
    }
    else
    {
      fprintf(stderr, "nlSysEncryption: Unknown parameter '%ws'.\n", argv[i]);
      return 1;
    }
  }/* for */

  /* There must be at least one option */
  if( (argc < 2 || (_wcsicmp(argv[1],L"--help") == 0 ||
		    _wcsicmp(argv[1],L"-h") == 0 ||
		    _wcsicmp(argv[1],L"/h") == 0 ||
		    _wcsicmp(argv[1],L"/?") == 0)) )
  {
    print_help();
    return 1;
  }

  /* --heavywrite option requires --encrypt option */
  if( op_fastwrite == true && op_encrypt == false )
  {
    fprintf(stderr, "nlSysEncryption: --heavywrite option requires --encrypt option\n");
    return 1;
  }

  std::wstring op_file(argv[1]);

  /* Handle 'nlsysencryption /path [path] ...' parameter format */
  if( argc > 1 && _wcsicmp(argv[1],L"/path") == 0 )
  {
    op_file.assign(argv[2]);
  }

  /* Handle implicit current working directory*/
  wchar_t full_path[MAX_PATH];
  DWORD ret;

  ret = GetFullPathName(op_file.c_str(), _countof(full_path), full_path, NULL);
  if ( ret > _countof(full_path) || ret == 0 )
  {
    fwprintf(stderr, L"nlSysEncryption: Cannot access directory %s (le %d)\n",
	     op_file.c_str(), GetLastError());
    return 1;
  }

  op_file.assign(full_path);

  /* If the path contains "\*\", it is a directory wildcard path.  Don't map
   * the wildcard to POSIX regex.
   *
   * Otherwise, the path is *probably* either a file wildcard path, or not a
   * wildcard path.  So if there is a wildcard map it to POSIX regex. */
  bool include_root = true;
  if( boost::algorithm::icontains(op_file,L"\\*\\") == false &&
      ( boost::algorithm::icontains(op_file,L"*") == true ||
        boost::algorithm::icontains(op_file,L"?") == true ) )
  {
    std::wstring::size_type i = op_file.find_last_of(L"\\/");
    if( i != std::wstring::npos )
    {
      op_filter_regex.assign(op_file,i+1,op_file.length()-i);
      op_file.erase(i+1);
      include_root = false;

	  boost::algorithm::replace_all(op_filter_regex,L".", L"[.]");
      boost::algorithm::replace_all(op_filter_regex,L"*", L".*");

	  /* Always replace ? with . */
      boost::algorithm::replace_all(op_filter_regex,L"?", L".");
    }
    op_filter = true;
  }

  // Dummy Code to let nlSysEncryption.exe to be trusted.
  HANDLE hFile = CreateFileW(op_file.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
  if (hFile != INVALID_HANDLE_VALUE)
  {
	  CloseHandle(hFile);
  }

  bool path_exists = false;
  try
  {
    path_exists = fs::exists(fs::wpath(op_file));
  }
  catch( ... )
  {
    path_exists = false;
  }

  ///* If path does not exist, assume the path is a directory. */
  if( path_exists == false )
  {
    fprintf(stderr, "nlSysEncryption: Path does not exist.  Assuming path is directory.\n");

	BOOL is_encrypted = SE_IsEncrypted(op_file.c_str());
	BOOL is_encrypted_fw = SE_IsEncryptedFW(op_file.c_str());

    if( op_encrypt == false )
    {
	  fwprintf(stdout,L"Directory of %s %s%s\n",
		   op_file.c_str(),
		   is_encrypted_fw ? L"(heavy-write) " : L"",
		   is_encrypted ? L"(encrypted)" : L"(not encrypted)");
      return 0;
    }
    else
    {
      if( is_encrypted == TRUE )
      {
        fwprintf(stdout, L"nlSysEncryption: %s is already encrypted\n", op_file.c_str());
        return 1;
      }

      BOOL ret2;

      if( op_fastwrite == true )
      {
        ret2 = SE_EncryptDirectoryFW(op_file.c_str());
      }
      else
      {
        ret2 = SE_EncryptDirectory(op_file.c_str());
      }

      if( ret2 == FALSE)
      {
        fwprintf(stderr, L"nlSysEncryption: Cannot encrypt directory %s (le %d)\n", op_file.c_str(), GetLastError());
        return 1;
      }

      return 0;
	}
  }

  /* begin traversing */
  scan(op_file.c_str(),include_root,true);

  if (encryptionfailures.size() > 0 && op_logfailures) {
      fwprintf(stdout, L"\nEncryption failures:\n");
      for (std::list<std::wstring>::iterator i=encryptionfailures.begin();
           i != encryptionfailures.end();
           ++i) {
          fwprintf(stdout, L"%s\n", (*i).c_str());
      }
  }

  return 0;
}/* main */
