#include <windows.h>
#include <string>

#include "eframework/platform/policy_controller.hpp"
#include "nl_sysenc_lib.h"
#include "nl_sysenc_lib_fw.h"
#include "eframework/resattr/resattr_loader.hpp"
#include "NextLabsTaggingLib.h"



static nextlabs::resattr_loader g_resattr_loader;

static bool op_key = false;
static bool op_data = false;
static bool op_recurse = false;
static bool op_passwd = false;
static bool op_output = false;
static bool op_inplace = false;
static bool op_verbose = false;
static bool op_help = false;
static std::wstring passwd;
static std::wstring input;
static std::wstring output;



static void printHelp(void)
{
  printf("\n");
  printf("NextLabs System Encryption Recovery %d.%d (Built %s %s)\n",
         VERSION_MAJOR, VERSION_MINOR, __DATE__, __TIME__);
  printf("usage: NLSERecovery option... file/dir \n");
  printf("\n");
  printf("Options:\n");
  printf("   --data                   Recover the data of the specified file or the files\n");
  printf("                            under the specified directory.\n");
  printf("                            (Must be specified in this version)\n");
  printf("   --password=<password>    Password to allow decryption.\n");
  printf("   --r                      Recursive file traversal.  Include subdirectories.\n");
  printf("   --output=<file/dir>      Output location of recovered files.\n");
  printf("   --inplace                Overwrite encrypted files with recovered versions.\n");
  printf("                            (One of --output and --inplace must be specified.)\n");
  printf("   --verbose                Verbose output.\n");
  printf("   --help, -h, /?           This screen.\n");
  printf("\n");
  printf("Recover data from encrypted versions of files to the original.\n");
  printf("\n");
  printf("Example:\n");
  printf("\n");
  printf("  nlseRecovery --data --password=mYpAsSwD --output=C:\\dir2 C:\\dir1\n");
} /* printHelp */

static bool processOptions(int argc, wchar_t* argv[])
{
  for (int i = 1; i < argc; i++)
  {
    wchar_t* option = wcsstr(argv[i], L"=");
    if (option != NULL)
    {
      option++;
    }

    if (wcscmp(argv[i], L"--key") == 0)
    {
      op_key = true;
    }
    else if (wcscmp(argv[i], L"--data") == 0)
    {
      op_data = true;
    }
    else if (wcsncmp(argv[i], L"--password=", wcslen(L"--password=")) == 0)
    {
      op_passwd = true;
      passwd = option;
    }
    else if (wcscmp(argv[i], L"--r") == 0)
    {
      op_recurse = true;
    }
    else if (wcsncmp(argv[i], L"--output=", wcslen(L"--output=")) == 0)
    {
      op_output = true;
      output = option;
    }
    else if (_wcsicmp(argv[i], L"--inplace") == 0)
    {
      op_inplace = true;
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

  if (op_key)
  {
    fprintf(stderr, "\"--key\" option is not support in this version of NLSERecovery.\n");
    return false;
  }

  if (!op_data)
  {
    fprintf(stderr, "Must specify \"--data\" option.\n");
    printHelp();
    return false;
  }

  if (op_output == op_inplace)
  {
    fprintf(stderr, "Must specify either \"--output\" or \"--inplace\", but not both.\n");
    printHelp();
    return false;
  }

  if (op_output && output == L"")
  {
    fprintf(stderr, "Missing output file/directory.\n");
    printHelp();
    return false;
  }

  if (input == L"")
  {
    fprintf(stderr, "Missing file/directory.\n");
    printHelp();
    return false;
  }

  if (op_passwd)
  {
    if (passwd == L"")
    {
      fprintf(stderr, "Missing password.\n");
      printHelp();
      return false;
    }
  }
  else
  {
    // Need to make password not echo to screen.
    WCHAR buf[256];
    printf("Password: ");
    fgetws(buf, _countof(buf), stdin);
    buf[wcslen(buf) - 1] = L'\0';   // get rid of '\n'
    passwd = buf;
  }

  return true;
}

static void printLastError(void)
{
  DWORD lastErr = GetLastError();
  WCHAR errStr[1024];

  FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_MAX_WIDTH_MASK,
                NULL, lastErr, 0, errStr, 1024, NULL);
  fprintf(stderr, "%ws\n", errStr);
} /* printLastError */

static bool doFile(LPCWSTR inFileName, LPCWSTR outFileName OPTIONAL)
{
  BOOL ret;

  if (SE_IsEncryptedFW(inFileName))
  {
    ret = SE_DecryptFileFW(inFileName, outFileName, passwd.c_str());
  }
  else
  {
    // Read existing attributes in the NextLabs file header before
    // decrypting the file.  Write the attributes to the decrypted
    // file so that they go into the original file format.
    //
    // The NXL_xxx reserved tags are skipped.
    //
    // NOTE:
    // - CreateAttributeManager() and AllocAttributes() return zero
    //   on success.
    // - ReadResourceAttributes() and WriteResourceAttributes()
    //   return non-zero on success.
    // Don't ask me why.
    ResourceAttributeManager *mgr;
    ResourceAttributes *inAttrs, *outAttrs;

    ret = FALSE;

    if (!g_resattr_loader.ensure_loaded())
    {
      return false;
    }

    if (g_resattr_loader.m_fns.CreateAttributeManager(&mgr) == 0)
    {
      if (g_resattr_loader.m_fns.AllocAttributes(&inAttrs) == 0)
      {
        if (g_resattr_loader.m_fns.AllocAttributes(&outAttrs) == 0)
        {
          if (g_resattr_loader.m_fns.ReadResourceAttributesW(mgr, inFileName,
                                                             inAttrs) != 0)
          {
            if (SE_DecryptFile(inFileName, outFileName, passwd.c_str()))
            {
              int i;

              for (i = 0;
                   i < g_resattr_loader.m_fns.GetAttributeCount(inAttrs);
                   i++)
              {
                const WCHAR* name;

                name = g_resattr_loader.m_fns.GetAttributeName(inAttrs, i);

                if (wcscmp(name, NL_ATTR_KEY_WRAPPED) != 0 &&
                    wcscmp(name, NLE_ATTR_KEY_ENCRYPTED) != 0 &&
                    wcscmp(name, NLE_ATTR_KEY_FAST_WRITE_ENCRYPTED) != 0 &&
                    wcscmp(name, NLE_ATTR_KEY_KEY_RING_NAME) != 0 &&
                    wcscmp(name, NLE_ATTR_KEY_REQUIRES_LOCAL_ENCRYPTION) != 0)
                {
                  g_resattr_loader.m_fns.AddAttributeW
                    (outAttrs, name,
                     g_resattr_loader.m_fns.GetAttributeValue(inAttrs, i));
                }
              }

              ret = (g_resattr_loader.m_fns.WriteResourceAttributesW(
                         mgr,
                         outFileName == NULL ? inFileName : outFileName,
                         outAttrs)
                     != 0);
            }
          }

          g_resattr_loader.m_fns.FreeAttributes(outAttrs);
        }

        g_resattr_loader.m_fns.FreeAttributes(inAttrs);
      }

      g_resattr_loader.m_fns.CloseAttributeManager(mgr);
    }
  }

  if (!ret)
  {
    fprintf(stderr, "NLSERecovery: Cannot decrypt file %ws (le %d)\n",
            inFileName, GetLastError());
    return false;
  }

  return true;
} /* doFile */

static bool doDir(LPCWSTR inDirName, LPCWSTR outDirName OPTIONAL)
{
  std::wstring wildcardPath;
  WIN32_FIND_DATA findFileData;
  HANDLE searchHandle;
  BOOL bRet;
  BOOL ret;

  // Remove the DRM on the directory only if we are decryping in-place.
  if (outDirName == NULL || outDirName[0] == L'\0')
  {
    if (SE_IsEncryptedFW(inDirName))
    {
      ret = SE_DecryptDirectoryFW(inDirName, passwd.c_str());
    }
    else
    {
      ret = SE_DecryptDirectory(inDirName, passwd.c_str());
    }

    if (!ret && GetLastError() != ERROR_FILE_NOT_FOUND)
    {
      fprintf(stderr, "NLSERecovery: Cannot decrypt directory %ws (le %d)\n",
              inDirName, GetLastError());
      return false;
    }
  }

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
      fprintf(stderr, "Error scanning directory %ws: ", inDirName);
      printLastError();
      return false;
    }
  }

  do
  {
    std::wstring inNameStr, outNameStr;

    inNameStr = inDirName;
    inNameStr += L'\\';
    inNameStr += findFileData.cFileName;

    if (outDirName != NULL && outDirName[0] != L'\0')
    {
      outNameStr = outDirName;
      outNameStr += L'\\';
      outNameStr += findFileData.cFileName;
    }

    if ((findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
    {
      if (outDirName == NULL || outDirName[0] == L'\0')
      {
        if(doFile(inNameStr.c_str(), NULL))
            SetFileAttributesW(inNameStr.c_str(), findFileData.dwFileAttributes);
      }
      else
      {
        if(doFile(inNameStr.c_str(), outNameStr.c_str()))
            SetFileAttributesW(outNameStr.c_str(), findFileData.dwFileAttributes);
      }
      // Ignore error from doFile(), and continue.
    }
    else
    {
      if (wcscmp(findFileData.cFileName, L".") != 0 &&
          wcscmp(findFileData.cFileName, L"..") != 0)
      {
        if (op_recurse)
        {
          if (outDirName == NULL || outDirName[0] == L'\0')
          {
            doDir(inNameStr.c_str(), NULL);
          }
          else
          {
            bRet = CreateDirectory(outNameStr.c_str(), NULL);
            if (!bRet && GetLastError() != ERROR_ALREADY_EXISTS)
            {
              fprintf(stderr, "Error creating directory %ws: ",
                      outNameStr.c_str());
              printLastError();
              FindClose(searchHandle);
              return false;
            }

            doDir(inNameStr.c_str(), outNameStr.c_str());
          }
        }
        else
        {
          if (op_verbose)
          {
            printf("skipping directory %ws\n", findFileData.cFileName);
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
    fprintf(stderr, "Error scanning directory %ws: ", inDirName);
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

  // Check if PC is running.
  if (!nextlabs::policy_controller::is_up())
  {
    fprintf(stderr, "Error: Control Center Enforcer Service is not running\n");
    return 1;
  }

  /* Handle implicit current working directory */
  wchar_t full_path[MAX_PATH];
  DWORD ret2;

  ret2 = GetFullPathName(input.c_str(), _countof(full_path), full_path, NULL);
  if (ret2 > _countof(full_path) || ret2 == 0)
  {
    fprintf(stderr, "Error accessing %ws: ", input.c_str());
    printLastError();
    return 1;
  }

  input.assign(full_path);

  DWORD attrs = GetFileAttributes(input.c_str());
  if (attrs == INVALID_FILE_ATTRIBUTES)
  {
    DWORD lastErr = GetLastError();

    fprintf(stderr, "Error accessing %ws: ", input.c_str());
    printLastError();

    /* If path does not exist, assume the path is a directory. */
    if ((lastErr != ERROR_FILE_NOT_FOUND && lastErr != ERROR_PATH_NOT_FOUND) &&
        wcsstr(input.c_str(), L"\\*\\") == NULL)
    {
      return 1;
    }

    fprintf(stderr, "Assuming path is directory.\n");

    if (!SE_IsEncryptedEx(input.c_str(), FALSE))
    {
      fprintf(stderr, "%ws is not encrypted\n", input.c_str());
      return 1;
    }

    BOOL ret3;

    if (SE_IsEncryptedFW(input.c_str()))
    {
      ret3 = SE_DecryptDirectoryFW(input.c_str(), passwd.c_str());
    }
    else
    {
      ret3 = SE_DecryptDirectory(input.c_str(), passwd.c_str());
    }

    if (!ret3)
    {
      fprintf(stderr, "Cannot recover directory %ws: ", input.c_str());
      printLastError();
      return 1;
    }

    return 0;
  }

  if (attrs & FILE_ATTRIBUTE_DIRECTORY)
  {
    ret = doDir(input.c_str(), output.c_str());
  }
  else
  {
      if(output.empty())
      {
          ret = doFile(input.c_str(), NULL);
          if(ret) SetFileAttributesW(input.c_str(), attrs);
      }
      else
      {
          ret = doFile(input.c_str(), output.c_str());
          if(ret) SetFileAttributesW(output.c_str(), attrs);
      }
  }

  return ret ? 0 : 1;
} /* wmain */
