/*
 * NLTag Tool: Used for viewing, adding, deleting 
 * custom attributes (tags) on a file.
 * Author: Abdur Rehman Pathan
 */
#include <winsock2.h>
#include <windows.h>
#include <process.h>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <list>

#include <sys/types.h>
#include <sys/stat.h>

#include <sys/types.h>
#include <sys/timeb.h>
#include <algorithm>

#include "resattrlib.h"
#include "resattrmgr.h"

#pragma  warning(push)
#pragma  warning(disable:4819)
#include <boost\algorithm\string.hpp>
#pragma  warning(pop)

// logging
#include "celog.h"
#include "celog_policy_file.hpp"
#include "celog_policy_windbg.hpp"

#include "Exttag.h"

resAttrPtrs ptrs;
ResourceAttributeManager *mgr = NULL;
static BOOL initialized = false;

/* Variables only required for recording time with --time */
static LARGE_INTEGER perf_freq;
static LARGE_INTEGER perf_time_start;
static LARGE_INTEGER perf_time_end;

/* Filename */
static WCHAR op_filename[1025]={0};
static wchar_t out_file[1025]={0};

/* Tag operation */
static bool op_viewtags = false;
static bool op_addtag   = false;
static bool op_deltag   = false;
static bool op_clear    = false;
static bool op_assert   = false;
static bool op_parsefile	=false;

static const wchar_t* g_cszFileAction=L"-batch";
/* Tag features */
static bool op_tnflag   = false;
static bool op_tvflag   = false;
static bool op_time     = false;
static bool op_help     = false;
static bool op_verbose  = false;

/** initializePtrs
 *
 *  \brief Load libraries and initialize pointers to functions.
 *
 *  \param pptrs (in)    Address of resAttrPtrs structure.
 *  \return  ERROR_SUCCESS on success, and ERROR code otherwise
 */
static DWORD initializePtrs(resAttrPtrs *pptrs)
{
  static HMODULE resattrmgr = NULL;
  static HMODULE resattrlib = NULL;
  
  if( !initialized )
  {
    /* Load resattrmgr library */
#ifdef _WIN64
      resattrmgr = ::LoadLibrary( L"c:\\Program Files\\NextLabs\\Policy Controller\\bin\\resattrmgr.dll" );
#else
      resattrmgr = ::LoadLibrary( L"c:\\Program Files\\NextLabs\\Policy Controller\\bin\\resattrmgr32.dll" );
#endif

    if ( NULL == resattrmgr ) {
	  printf ("Unable to find resattrmgr.dll under Policy Controller/bin.  Will try local folder.\n");
#ifdef _WIN64
	resattrmgr = ::LoadLibrary( L"resattrmgr.dll" );
#else
	resattrmgr = ::LoadLibrary( L"resattrmgr32.dll" );
#endif
    }
	
    if ( NULL == resattrmgr )
    {
      fprintf(stderr, "Unable to load resattrmgr.dll (Error: %d)\n", GetLastError());
      return ERROR_FILE_NOT_FOUND;
    }

    pptrs->create_mgr_fn   = (create_mgr)  GetProcAddress(resattrmgr, "CreateAttributeManager");
    pptrs->read_attrs_fn   = (read_attrs)  GetProcAddress(resattrmgr, "ReadResourceAttributesW");
    pptrs->write_attrs_fn  = (write_attrs) GetProcAddress(resattrmgr, "WriteResourceAttributesW");
    pptrs->remove_attrs_fn = (remove_attrs)GetProcAddress(resattrmgr, "RemoveResourceAttributesW");
    
    if( ( NULL == pptrs->create_mgr_fn )  ||
        ( NULL == pptrs->read_attrs_fn )  ||
        ( NULL == pptrs->write_attrs_fn ) ||
        ( NULL == pptrs->remove_attrs_fn ) )
    {
      fprintf(stdout, "Unable to find CreateAttributeManager, ReadResourceAttributesW, \
               WriteResourceAttributesW or RemoveResourceAttributesW  in resattrmgr.dll\n");
      return ERROR_INVALID_FUNCTION;
    }

    /* Load resattrlib library */
    // Is this actually necessary?  resattrmgr should load it automatically
#ifdef _WIN64
    resattrlib = ::LoadLibrary( L"c:\\Program Files\\NextLabs\\Policy Controller\\bin\\resattrlib.dll" );
#else
    resattrlib = ::LoadLibrary( L"c:\\Program Files\\NextLabs\\Policy Controller\\bin\\resattrlib32.dll" );
#endif
    if( NULL == resattrlib )
    {
	  printf ("Unable to find resattrlib.dll under Policy Controller/bin.  Will try local folder.\n");
#ifdef _WIN64
	  resattrlib = ::LoadLibrary( L"resattrlib.dll" );
#else
	  resattrlib = ::LoadLibrary( L"resattrlib32.dll" );
#endif
    }
    if( NULL == resattrlib )
    {
      fprintf(stderr, "Unable to load resattrlib.dll (Error: %d)\n", GetLastError());
      return ERROR_FILE_NOT_FOUND;
    }

    pptrs->alloc_attrs_fn = (alloc_attrs) GetProcAddress(resattrlib, "AllocAttributes");
    pptrs->free_attrs_fn  = (free_attrs)  GetProcAddress(resattrlib, "FreeAttributes");
    pptrs->get_count_fn   = (get_count)   GetProcAddress(resattrlib, "GetAttributeCount");
    pptrs->get_name_fn    = (get_name)    GetProcAddress(resattrlib, "GetAttributeName");
    pptrs->get_value_fn   = (get_value)   GetProcAddress(resattrlib, "GetAttributeValue");
    pptrs->add_attrs_fn   = (add_attrs)   GetProcAddress(resattrlib, "AddAttributeW");

    if( ( NULL == pptrs->alloc_attrs_fn ) ||
        ( NULL == pptrs->free_attrs_fn )  ||
        ( NULL == pptrs->get_count_fn )   ||
        ( NULL == pptrs->get_name_fn )    ||
        ( NULL == pptrs->get_value_fn )   ||
        ( NULL == pptrs->add_attrs_fn ) )
    {
      fprintf(stdout, "Unable to find functions in resattrlib.dll\n");
      return ERROR_INVALID_FUNCTION;
    }
    initialized = true;
  }
  return ERROR_SUCCESS;
} /* End of initializePtrs() */

/** viewTags
 *
 *  \brief Display tags on a file.
 *
 *  \param attrs (in)    Pointer to customer attributes structure.
 *  \return void
 */
void viewTags(ResourceAttributes *attrs)
{
  /* If --time option is set, record start time */
  if( true == op_time )
  {
    if( false == QueryPerformanceFrequency(&perf_freq) )
    {
      fprintf(stderr, "nltag: QueryPerformanceFrequency failed (le %d)\n", GetLastError());
      return;
    }
    QueryPerformanceCounter(&perf_time_start);
  }

  /* Read the custom attributes on the file */
  if( false == ( ptrs.read_attrs_fn( mgr, op_filename, attrs ) ) )
  {
    fprintf(stderr, "ReadAttributes() failed\n" ) ;
    return;
  }

  /* If --time option is set, record end time and display time for read_attrs() */
  if( true == op_time )
  {
    QueryPerformanceCounter(&perf_time_end);

    LARGE_INTEGER perf_time_diff; // elapsed time between start and end
    perf_time_diff.QuadPart = perf_time_end.QuadPart - perf_time_start.QuadPart;

    /* The difference must be normalized to milliseconds.  perf_freq is counts per
     * second.  This is normalized to counts per millisecond.
     */
    std::size_t curr_time    = (std::size_t)(perf_time_diff.QuadPart / (perf_freq.QuadPart / 1000));
    std::size_t curr_time_us = (std::size_t)(perf_time_diff.QuadPart % (perf_freq.QuadPart / 1000));

    fprintf(stdout, "\nTime taken to Read Attrs = %d.%06dms\n\n", curr_time, curr_time_us);
  }

  /* Retrive the number of attributes on the file to be displayed */
  int count = ptrs.get_count_fn( attrs );

  if (count > 0) 
  {
    /* Display the attributes applied on the file */
    for (int i = 0; i < count; i++) 
    {
      const WCHAR *name = ptrs.get_name_fn(attrs, i);
      const WCHAR *value = ptrs.get_value_fn(attrs, i);

      if( ( NULL != name ) && ( NULL != value ) )
      {
        fprintf(stdout, "  %d) %ws = %ws\n", (i+1), name, value);
      }
      else
      {
        if(NULL != name)
          fprintf(stdout, "  %d) %ws = (null)\n", (i+1), name);
        else if (NULL != value)
          fprintf(stdout, "  %d) (null) = %ws\n", (i+1), value);
        else
          fprintf(stdout, "  %d) (null) = (null)\n", (i+1) );
      }
    }
  }
  else
  {
    /* count is 0, hence no tags are applied on the file */
    fprintf(stdout, "\n ** No tags on file %ws **\n", op_filename);
  }
} /* End of viewTags() */

/** addTag
 *
 *  \brief Add given tag on the file.
 *
 *  \param attrs    (in)    Pointer to customer attributes structure.
 *  \param tagname  (in)    Name of the Tag to be added
 *  \param tagvalue (in)    Value of the tag to be added
 *  \return void
 */
void addTag(ResourceAttributes *attrs, const WCHAR *tagname, const WCHAR * tagvalue)
{
  if( false == ( ptrs.read_attrs_fn( mgr, op_filename, attrs ) ) )
  {
	  fprintf(stderr, "AddTag: 1) ReadAttributes() failed\n" ) ;
    return;
  }

  /* Add the new attributes to the Vector (not yet to the file) */
  ptrs.add_attrs_fn ( attrs, tagname, tagvalue );

  /* If --time option is set, record start time */
  if( true == op_time )
  {
    if( false == QueryPerformanceFrequency(&perf_freq) )
    {
      fprintf(stderr, "nltag: QueryPerformanceFrequency failed (le %d)\n", GetLastError());
      return;
    }
    QueryPerformanceCounter(&perf_time_start);
  }

  /* Write the newly added custom attributes to the file */
  if( false == ( ptrs.write_attrs_fn( mgr, op_filename, attrs ) ) )
  {
    fprintf(stderr, "WriteAttributes() failed\n" ) ;
    return;
  }

  /* If --time option is set, record end time and display time for write_attrs() */
  if( true == op_time )
  {
    QueryPerformanceCounter(&perf_time_end);

    LARGE_INTEGER perf_time_diff; // elapsed time between start and end
    perf_time_diff.QuadPart = perf_time_end.QuadPart - perf_time_start.QuadPart;

    /* The difference must be normalized to milliseconds.  perf_freq is counts per
     * second.  This is normalized to counts per millisecond.
     */
    std::size_t curr_time    = (std::size_t)(perf_time_diff.QuadPart / (perf_freq.QuadPart / 1000));
    std::size_t curr_time_us = (std::size_t)(perf_time_diff.QuadPart % (perf_freq.QuadPart / 1000));

    fprintf(stdout, "\nTime taken to Write Attrs = %d.%06dms\n\n", curr_time, curr_time_us);
  }
} /* End of addTag() */

/** delTag
 *
 *  \brief Delete given tag of a file.
 *
 *  \param attrs    (in)    Pointer to customer attributes structure.
 *  \param tagname  (in)    Name of the Tag to be deleted
 *  \param tagvalue (in)    Value of the tag to be deleted
 *  \return void
 */
void delTag(ResourceAttributes *attrs, const WCHAR *tagname, const WCHAR * tagvalue)
{
  /* First Read the custom attributes on the file */
  if ( false == (ptrs.read_attrs_fn( mgr, op_filename, attrs ) ) )
  {
    fprintf(stderr, "ReadAttributes() failed\n" ) ;
    return;
  }

  /* Create a temporary copy of the attributes on the file */
  ResourceAttributes *tempattrs = NULL;
  
  if( ERROR_SUCCESS != ( ptrs.alloc_attrs_fn( &tempattrs ) ) )
  {
    fprintf(stderr, "AllocAttributes() failed\n" ) ;
    return;
  }

  if ( false == ( ptrs.read_attrs_fn( mgr, op_filename, tempattrs ) ) )
  {
    fprintf(stderr, "ReadAttributes() failed\n" ) ;
    return;
  }

  /* Remove all and rewrite the ones we want */
  if ( false == ( ptrs.remove_attrs_fn( mgr, op_filename, attrs ) ) )
  {
    fprintf(stderr, "RemoveAttributes() failed\n" ) ;
    return;
  }

  /* Free the old copy of the attributes */
  ptrs.free_attrs_fn ( attrs );


  /* Realloc a copy of the attrbiutes to be reqritten */
  if ( ERROR_SUCCESS != ( ptrs.alloc_attrs_fn( &attrs ) ) )
  {
    fprintf(stderr, "AllocAttributes() failed\n" ) ;
    return;
  }
 
  int count = ptrs.get_count_fn( tempattrs );

  /* Retrive the number of attributes on the file to be displayed */
  if (count > 0) 
  {
    /* Flag used to check whether given tag name/value pair was found */
    bool found_flag = false;
 
    for (int i = 0; i < count; i++) 
    {
      const WCHAR *name = ptrs.get_name_fn(tempattrs, i);
      const WCHAR *value = ptrs.get_value_fn(tempattrs, i);
 
      if( ( NULL != name ) && ( NULL != value ) )
      {
        /* Since, there is no direct method exported to remove a tag 
         * on the file, clear all tags and then add only the ones which 
         * are not to be deleted 
         */
        if( ( !_wcsicmp(tagname, name) && !_wcsicmp(tagvalue, value) ) ||
            ( !_wcsicmp(tagname, name) && !wcscmp(tagvalue, L"0") )    ||
            ( !_wcsicmp(tagname, L"0") && !_wcsicmp(tagvalue, value) ) ) 
        {

          /* Deleting here means that it is not re-added to the file attributes */
          fprintf(stdout, "\nDeleting Tag  %ws = %ws on file %ws\n", name, value, op_filename);
          found_flag = true;
        }
        else
        {
          /* Re-Add this tag name value since it did not match the given tag name/value */
          addTag( attrs, name, value );


		  /* Free the old copy of the attributes */
		  ptrs.free_attrs_fn ( attrs );

		  /* Realloc a copy of the attrbiutes to be reqritten */
		  if ( ERROR_SUCCESS != ( ptrs.alloc_attrs_fn( &attrs ) ) )
		  {
			  fprintf(stderr, "AllocAttributes() failed\n" ) ;
			  return;
		  }

        }
      }
    }

    if( !found_flag )
    {
      /* The given tag name/value were not found */
      fprintf(stdout, "\nEither Tagname %ws or TagValue %ws did not match file tags\n", tagname, tagvalue);
      fprintf(stdout, "Please use nltag.exe %ws --view to view existing file tags\n", op_filename);
    }
  }
  else
  {
    /* count is 0, hence no tags are applied on the file */
    fprintf(stdout, "\n ** No tags on file %ws **\n", op_filename);
  }
} /* End of delTag() */

/** clearTags
 *
 *  \brief Delete/Clear all tags on a file.
 *
 *  \param attrs    (in)    Pointer to customer attributes structure.
 *  \return void
 */
void clearTags( ResourceAttributes *attrs )
{
  /* Read the custom attributes on the file */
  if ( false == ( ptrs.read_attrs_fn  ( mgr, op_filename, attrs ) ) )
  {
    fprintf(stderr, "ReadAttributes() failed\n" ) ;
    return;
  }

  /* Remove all the custom attributes on the file */
  if( false == ( ptrs.remove_attrs_fn( mgr, op_filename, attrs ) ) )
  {
    fprintf(stderr, "RemoveAttributes() failed\n" ) ;
    return;
  }
  fprintf(stdout, "\nAll tags on file %ws have been deleted.\n", op_filename);
} /* End of clearTags() */

/** assertTag
 *
 *  \brief Adds a tag pair to a file. 
 *		   Asserts that particular tag on a file. Also verifies count of tags
 *
 *  \param attrs    (in)    Pointer to customer attributes structure.
 *  \param tagname  (in)    Name of the Tag to be checked
 *  \param tagvalue (in)    Value of the tag to be checked
 *  \return void
 */
bool assertTag( ResourceAttributes *attrs, const WCHAR *tagname, const WCHAR * tagvalue )
{
  if ( false == ( ptrs.read_attrs_fn( mgr, op_filename, attrs ) ) )
  {
	fprintf(stderr, "AssertTag: 1) ReadAttributes() failed\n" ) ;
    return false;
  }
  int oldcount = ptrs.get_count_fn( attrs );

  ResourceAttributes *addattrs = NULL, *resultAttrs = NULL;

  // allocate space in addattrs
  if( ERROR_SUCCESS != ( ptrs.alloc_attrs_fn( &addattrs )) ||
      ERROR_SUCCESS != ( ptrs.alloc_attrs_fn( &resultAttrs )) )
  {
    fprintf(stderr, "AllocAttributes() failed\n" ) ;
    return false;
  }

  addTag(addattrs, tagname, tagvalue);
  
  if ( false == ( ptrs.read_attrs_fn( mgr, op_filename, resultAttrs ) ) )
  {
    fprintf(stderr, "AssertTag: 2) ReadAttributes() failed\n" ) ;
    return false;
  }
  int newcount = ptrs.get_count_fn( resultAttrs );
  /* Flag used to check whether given tag name/value pair was found */
  bool found_flag = false;

  /* Retrive the number of attributes on the file to verify */
  if (newcount > 0) 
  {
    for (int i = 0; i < newcount; i++) 
    {
      const WCHAR *name = ptrs.get_name_fn(resultAttrs, i);
      const WCHAR *value = ptrs.get_value_fn(resultAttrs, i);
 
      if( ( NULL != name ) && ( NULL != value ) )
      {
        /* Since, there is no direct method exported to remove a tag 
         * on the file, clear all tags and then add only the ones which 
         * are not to be deleted 
         */
        if( ( !_wcsicmp(tagname, name) && !_wcsicmp(tagvalue, value) ) ) 
        {

          /* Deleting here means that it is not re-added to the file attributes */
          found_flag = true;
        }
      } 
    }
  }

  if( ( newcount == (oldcount + 1) ) && (true == found_flag) )
	  return true;
  else {
      fprintf (stderr, "found_flag: %d, newcount: %d, oldcount: %d\n", found_flag, newcount, oldcount);
	  return false;
  }


} /* End of assertTag() */

/** printHelp
 *
 *  \brief Prints the Help Screen for this program.
 */
void printHelp( void )
{
  fprintf(stdout, "\nUsage: nltag [filename] [options] ...\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "Options:\n");
  fprintf(stdout, "\n  --help               This Help Screen\n");
  fprintf(stdout, "  --view               Display Tags on file [filename]\n");
  fprintf(stdout, "  --add --name=[key] --value=[value]    Add [key]=[value] on [filename]\n");
  fprintf(stdout, "  --del --name=[key] --value=[value]    Del [key]=[value] on [filename]\n"); 
  //fprintf(stdout, "                       (Use 0 for key/value if too long)\n");
  fprintf(stdout, "  --clear              Delete ALL Tags on file [filename]\n");
  fprintf(stdout, "  --assert --name=[key] --value=[value] Add & Check [key]=[value] on [filename]\n");
  fprintf(stdout, "  --time               Total time taken (--view, --add)\n");
  //fprintf(stdout, "  --verbose                Verbose output\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "Examples:\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "  nltag c:\\foo.txt (same as nltag c:\\foo.txt --view)\n");
  fprintf(stdout, "  nltag c:\\foo.txt --add --name=foo --value=bar\n");
  fprintf(stdout, "  nltag c:\\foo.txt --clear (Deletes all tags on c:\\foo.txt)\n");
  fprintf(stdout, "  nltag -batch -input=c:\\in.csv -output=c:\\out.csv\n");
  fprintf(stdout, "  nltag c:\\foo.txt --del --name=foo --value=bar\n");
  fprintf(stdout, "  nltag c:\\foo.txt --add --name=foo --value=bar --time\n");
  //fprintf(stdout, "  nltag c:\\foo.txt --del --name=0 --value=bar (deletes all tags with value \"bar\")\n");
  //fprintf(stdout, "  nltag c:\\foo.txt --del --name=foo --value=0 (deletes all tags with name \"foo\")\n");
  fprintf(stdout, "\n");
}/* end of printHelp */
//////////////////////////////////////////////////////////////////////////
// parse file
extern bool parseTags(__in const wchar_t* szCVSFile,__in const wchar_t* szOutFile);
//////////////////////////////////////////////////////////////////////////

/* Main */
int wmain( int argc,wchar_t **argv )
{
  fprintf(stdout, "NextLabs Tag Tool (Built %s %s)\n", __DATE__, __TIME__);
   //set up logging
    CELogS::Instance()->SetLevel(CELOG_DEBUG);
    CELogS::Instance()->SetPolicy( new CELogPolicy_File("nltag.log") );
    CELogS::Instance()->SetPolicy( new CELogPolicy_WinDbg() );
    TRACE(CELOG_INFO,"Logging started\n");
    
  std::wstring op_tagname;
  std::wstring op_tagvalue;
  if(argc >= 2)
  {
	  /* parse file */
	  if(0 == _wcsicmp(argv[1],g_cszFileAction))
	  {
		  op_parsefile = true;
	  }
  }

  /* Process options */
  for( int i = 2 ; i < argc ; ++i )
  {
    wchar_t * option = wcsstr(argv[i],L"=");
    if( NULL != option )
    {
      option++;
    }

    /* Tag Name */
    if( 0 == wcsncmp( argv[i], L"--name=", wcslen(L"--name=") ) )
    {
      op_tnflag  = true;
      op_tagname = option;
      continue;
    }

    /* Tag Value */
    if( 0 == wcsncmp( argv[i], L"--value=", wcslen(L"--value=") ) )
    {
      op_tvflag   = true;
      op_tagvalue = option;
      continue;
    }

	/* input file path */
	if( 0 == wcsncmp( argv[i], L"-input=", wcslen(L"-input=") ) )
	{
		wcscpy_s(op_filename,1024,option);
		continue;
	}
	/* output file path */
	if( 0 == wcsncmp( argv[i], L"-output=", wcslen(L"-output=") ) )
	{
		wcscpy_s(out_file,1024,option);
		continue;
	}
    /* Time */
    if( 0 == _wcsicmp(argv[i], L"--time") )
    {
      op_time = true;
      continue;
    }

    /* View Tags */
    if( 0 == _wcsicmp(argv[i], L"--view") )
    {
      op_viewtags = true;
      continue;
    }

    /* Add Tag */
    if( 0 == _wcsicmp(argv[i], L"--add") )
    {
      op_addtag = true;
      continue;
    }

    /* Delete Tag */
    if( 0 == _wcsicmp(argv[i], L"--del") )
    {
      op_deltag = true;
      continue;
    }

    /* Clear all Tags */
    if( 0 == _wcsicmp(argv[i], L"--clear") )
    {
      op_clear = true;
      continue;
    }

    /* Assert Tag: TODO */
    if( 0 == _wcsicmp(argv[i], L"--assert") )
    {
      op_assert = true;
      continue;
    }

    /* Help */
    if( 0 == _wcsicmp(argv[i], L"--help") )
    {
      op_help = true;
      continue;
    }

    /* Verbose Output */
    else if( 0 == _wcsicmp(argv[i], L"--verbose") )
    {
      op_verbose = true;
      continue;
    }

    /* Invalid Option */
    else
    {
      fprintf(stdout, "\nERROR: Invalid option: %ws\n", argv[i]);
      op_help = true;
    }
  }/* for */

  /* First Argument should always be a filename.
   * Check if first argument is an option, and display help screen
   */
  if( ( argc >= 2 ) && ( NULL != argv[1] ) )
  {
    if( !op_parsefile && (!wcsncmp( argv[1], L"--", 2 ) || !wcsncmp (argv[1], L"-", 1 ) ))
    {
      op_help = true;
    }
  }
  
  bool tag_check = true; 

  /* If either --add, --del or --assert option is used,
   * the --name and --value option must be provided too
   */
  if ( op_addtag || op_deltag || op_assert )
  {
    tag_check = (op_tnflag && op_tvflag);
  }

  /* Show the Help screen if:
   *   1) No arguments are provided
   *   2) --help option is provided
   *   3) If the 1st argument is NULL (i.e. Filename not provided)
   *   4) If --add, --del, --assert option is used, but --name and --value are not provided
   */
  if( ( argc <= 1 ) || ( true == op_help ) || (NULL == argv[1] ) ||  ( false == tag_check ) )
  {
    printHelp();
    return -1;
  }

  /* Use the first argument as Filename */
  if(!op_parsefile)  wcsncpy_s(op_filename, sizeof(op_filename)/sizeof(WCHAR), argv[1], _TRUNCATE);

  /* Check if file exists */
  struct _stat buf;
  if( !_wstat(op_filename, &buf) ) 
  {
    fprintf(stdout, "\nFileName: %ws\t", op_filename);
    if(op_tnflag && op_tvflag)
    {
      fprintf(stdout, "TagName: %ws\t TagValue: %ws", op_tagname.c_str(), op_tagvalue.c_str());
    }
    fprintf(stdout, "\n");
  }
  else
  {
    fprintf(stderr, "\nERROR: File %ws does not exist (Error: %d)\n", op_filename, GetLastError() );
    return -1;
  }

  if(op_parsefile && !boost::algorithm::iends_with(op_filename,L".csv"))
  {
	  fprintf(stderr, "\nERROR: File %ws is not a csv file\n", op_filename );
	  return -1;
  }


  /* Initialize pointers to the functions */
  if( ERROR_SUCCESS != (initializePtrs(&ptrs)) )
  {
    fprintf(stderr, "Fatal error initializing ptrs in getFileCustomAttributes\n");
    return -1;
  }

  /* Create the Resoure Attribute Manager */
  if( 0 != ptrs.create_mgr_fn(&mgr) )
  {
    fprintf(stderr, "Fatal error creating resource manager\n");
    return -1;
  }
  
  /* Allocate the ResourceAttributes object */ 
  ResourceAttributes *attrs = NULL;
  if( ERROR_SUCCESS != ptrs.alloc_attrs_fn(&attrs) )
  {
      fprintf(stderr, "AllocAttributes() failed\n" ) ;
      return -1;
  }
  
  /* Only one operation can be performed in one instance of the program as
   * 1) Add Tag --add
   * 2) Delete Tag --del
   * 3) Clear Tags --clear
   * 4) Assert Tag --assert
   * 5) View Tags --view
   * The other options such as --time, --help and --verbose are not an operation
   */
  
  /* Add Tag */
  if( op_addtag )
  {
    addTag(attrs, op_tagname.c_str(), op_tagvalue.c_str());
    fprintf(stdout, "\nTag %ws = %ws has been successfully added to %ws\n", op_tagname.c_str(), op_tagvalue.c_str(), op_filename);
  }
  /* Delete Tag */
  else if( op_deltag )
  {
    delTag( attrs, op_tagname.c_str(), op_tagvalue.c_str() );
  }
  /* Clear All Tags */
  else if( op_clear )
  {
    clearTags(attrs);
  }
  else if(op_parsefile)
  {
	  parseTags(op_filename,out_file);
  }
  /* Assert Tag */
  else if( op_assert )
  {
    bool ret = assertTag( attrs, op_tagname.c_str(), op_tagvalue.c_str() );
	if( true == ret )
	{
		fprintf(stdout, "\nPASS for Tag %ws = %ws on Filename: %ws\n", op_tagname.c_str(), op_tagvalue.c_str(), op_filename);
	}
	else
	{
		fprintf(stdout, "\nFAIL for Tag %ws = %ws on Filename: %ws\n", op_tagname.c_str(), op_tagvalue.c_str(), op_filename);
	}
  }
  else
  {
    /* View Tags */
    viewTags( attrs );
  }
  if(attrs != NULL)
  {
	  ptrs.free_attrs_fn(attrs);
	  attrs = NULL;
  }
  return 0;
} /* End of wmain */

/* End of file nltag.cpp */
