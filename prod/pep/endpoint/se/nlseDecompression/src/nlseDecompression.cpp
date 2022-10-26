#include <Windows.h>
#include <string>
#include <WinIoCtl.h>



static bool op_recurse = false;
static bool op_verbose = false;
static bool op_help = false;
static std::wstring input;



static void printHelp(void)
{
  printf("\n");
  printf("NextLabs System Encryption Decompression %d.%d (Built %s %s)\n",
         VERSION_MAJOR, VERSION_MINOR, __DATE__, __TIME__);
  printf("Usage: NLSEDecompression option... file/dir \n");
  printf("\n");
  printf("Options:\n");
  printf("   --r                      Recursive file traversal.  Include subdirectories.\n");
  printf("   --verbose                Verbose output.\n");
  printf("   --help, -h, /?           This screen.\n");
  printf("\n");
  printf("Decompress NTFS-compressed data to prepare for System Encryption.\n");
  printf("\n");
  printf("Example:\n");
  printf("\n");
  printf("  NLSEDecompression --r C:\\dir1\n");
} /* printHelp */

static bool processOptions(int argc, wchar_t* argv[])
{
  for (int i = 1; i < argc; i++)
  {
    if (wcscmp(argv[i], L"--r") == 0)
    {
      op_recurse = true;
    }
    else if (_wcsicmp(argv[i], L"--verbose") == 0)
    {
      op_verbose = true;
    }
    else if (_wcsicmp(argv[i], L"--help") == 0 ||
             _wcsicmp(argv[i], L"-h") == 0 ||
             _wcsicmp(argv[i], L"/?") == 0)
    {
      op_help = true;
    }
    else if (argv[i][0] == L'-')
    {
      fprintf(stderr, "Unknown option: %ws\n", argv[i]);
      printHelp();
      return false;
    }
    else if (input != L"")
    {
      fprintf(stderr, "Too many parameters: %ws\n", argv[i]);
      printHelp();
      return false;
    }
    else
    {
      input = argv[i];
    }
  }

  if (op_help)
  {
    printHelp();
    return false;
  }

  if (input == L"")
  {
    fprintf(stderr, "Missing file/directory.\n");
    printHelp();
    return false;
  }

  return true;
}

static void printLastError(void)
{
  DWORD lastErr = GetLastError();
  WCHAR errStr[1024] = {0};

  FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_MAX_WIDTH_MASK,
                NULL, lastErr, 0, errStr, 1024, NULL);
  fwprintf(stderr, L"%s\n", errStr);
} /* printLastError */

static bool decompressFileOrDir(LPCWSTR path, bool isDir)
{
  DWORD attrs = GetFileAttributes(path);
  if (attrs == INVALID_FILE_ATTRIBUTES)
  {
    fwprintf(stderr, L"Cannot access %s: ", path);
    printLastError();
    return false;
  }

  if ((attrs & FILE_ATTRIBUTE_COMPRESSED) == 0)
  {
    wprintf(L"Skipping uncompressed %hs %s\n", (isDir ? "directory" : "file"),
	    path);
    return true;
  }

  /* If the file is read-only, un set that attribute to permit compression.  The
   * attribute will be reset after the uncompression is complete.
   */
  bool ro_enabled = false;
  if( (attrs & FILE_ATTRIBUTE_READONLY) )
  {
    DWORD new_attrs = attrs & (~FILE_ATTRIBUTE_READONLY);
    if( SetFileAttributes(path,new_attrs) == FALSE )
    {
      wprintf(L"Cannot uncompressed %hs %s\n", (isDir ? "directory" : "file"),
	      path);
      return true;
    }
    ro_enabled = true;
  }

  HANDLE h;

  h = CreateFile(path, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL,
                 OPEN_EXISTING, (isDir ? FILE_FLAG_BACKUP_SEMANTICS : 0),
                 NULL);
  if (h == INVALID_HANDLE_VALUE)
  {
    fwprintf(stderr, L"Cannot open %s: ", path);
    printLastError();
    return false;
  }

  USHORT inBuffer = COMPRESSION_FORMAT_NONE;
  DWORD bytesReturned;
  BOOL ret;

  ret = DeviceIoControl(h, FSCTL_SET_COMPRESSION, &inBuffer, sizeof inBuffer,
                        NULL, 0, &bytesReturned, NULL);
  if (!ret)
  {
    fwprintf(stderr, L"Cannot decompress %s: " , path);
    printLastError();
  }

  CloseHandle(h);

  /* If read-only has been unset, then add read-only attribute again. */
  if( ro_enabled == true )
  {
    /* Original attribute set contained FILE_ATTRIBUTE_READONLY */
    if( SetFileAttributes(path,attrs) == FALSE )
    {
      wprintf(L"Cannot uncompressed %hs %s\n", (isDir ? "directory" : "file"),
	      path);
      return true;
    }
  }

  return (bool) ret;
}

static bool doFile(LPCWSTR inFileName)
{
  if (op_verbose)
  {
    wprintf(L"Decompressing file %s\n", inFileName);
  }

  return decompressFileOrDir(inFileName, false);
} /* doFile */

static bool doDir(LPCWSTR inDirName)
{
  std::wstring wildcardPath;
  WIN32_FIND_DATA findFileData;
  HANDLE searchHandle;
  BOOL bRet;

  if (op_verbose)
  {
    wprintf(L"Decompressing directory %ws\n", inDirName);
  }

  decompressFileOrDir(inDirName, true);
  // Ignore error, and continue.

  wildcardPath = inDirName;
  wildcardPath += L"\\*";

  searchHandle = FindFirstFile(wildcardPath.c_str(), &findFileData);

  if (searchHandle == INVALID_HANDLE_VALUE)
  {
    if (GetLastError() == ERROR_FILE_NOT_FOUND)
    {
      // Directory is empty and has no "." or ".." (e.g. Root dir of a FAT
      // volume).  Done.
      return true;
    }
    else
    {
      fwprintf(stderr, L"Error scanning directory %s: ", inDirName);
      printLastError();
      return false;
    }
  }

  do
  {
    std::wstring inNameStr;

    inNameStr = inDirName;
    inNameStr += L'\\';
    inNameStr += findFileData.cFileName;

    if ((findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
    {
      doFile(inNameStr.c_str());
      // Ignore error from doFile(), and continue.
    }
    else
    {
      if (wcscmp(findFileData.cFileName, L".") != 0 &&
          wcscmp(findFileData.cFileName, L"..") != 0)
      {
        if (op_recurse)
        {
          doDir(inNameStr.c_str());
        }
        else
        {
          if (op_verbose)
          {
            wprintf(L"Skipping directory %s\n", findFileData.cFileName);
          }
        }
      }
    }

    bRet = FindNextFile(searchHandle, &findFileData);
  } while (bRet);

  if (GetLastError() == ERROR_NO_MORE_FILES)
  {
    FindClose(searchHandle);
    return true;
  }
  else
  {
    fwprintf(stderr, L"Error scanning directory %s: ", inDirName);
    printLastError();
    FindClose(searchHandle);
    return false;
  }
} /* doDir */

int wmain(int argc, wchar_t* argv[])
{
  bool ret;

  ret = processOptions(argc, argv);
  if (!ret)
  {
    return 1;
  }

  DWORD attrs = GetFileAttributes(input.c_str());
  if (attrs == INVALID_FILE_ATTRIBUTES)
  {
    fwprintf(stderr, L"Error accessing %s: ", input.c_str());
    printLastError();
    return 1;
  }

  if (attrs & FILE_ATTRIBUTE_DIRECTORY)
  {
    ret = doDir(input.c_str());
  }
  else
  {
    ret = doFile(input.c_str());
  }

  return ret ? 0 : 1;
} /* wmain */
