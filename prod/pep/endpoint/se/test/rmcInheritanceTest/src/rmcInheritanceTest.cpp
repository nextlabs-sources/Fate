#include <windows.h>
#include <string>

#include "nl_sysenc_lib.h"

#define MAXDRMFILEONESHOTCOUNT_DEFAULT_VALUE    200



static bool op_child = false;
static bool op_verbose = false;
static bool op_help = false;
static std::wstring child;
static std::wstring filePath;
static std::wstring wildcardPath;



static void printHelp(void)
{
  printf("\n");
  printf("RMC Inheritance Test Tool %d.%d (Built %s %s)\n",
         VERSION_MAJOR, VERSION_MINOR, __DATE__, __TIME__);
  printf("usage: RMCInheritanceTest option... filePath wildcardPath\n");
  printf("\n");
  printf("filePath and wildcardPath must be full paths with drive letters.\n");
  printf("wildcardPath must match filePath.\n");
  printf("\n");
  printf("Options:\n");
  printf("   --verbose                Verbose output.\n");
  printf("   --help, -h, /?           This screen.\n");
  printf("\n");
  printf("Test the RMC Inheritance feature, using the passed file as the test file\n");
  printf("\n");
  printf("Example:\n");
  printf("\n");
  printf("  RMCInheritanceTest C:\\dir1\\dir2\\dir3\\testfile.txt C:\\*1\\dir2\\*\\*file.txt\n");
} /* printHelp */

static bool processOptions(int argc, const wchar_t * const argv[])
{
  for (int i = 1; i < argc; i++)
  {
    const wchar_t *option = wcsstr(argv[i], L"=");
    if (option != NULL)
    {
      option++;
    }

    // "--child" is an internal option.  The user should not use it.
    if (wcsncmp(argv[i], L"--child=", wcslen(L"--child=")) == 0)
    {
      op_child = true;
      child = option;
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
    else if (wildcardPath != L"")
    {
      fprintf(stderr, "Too many parameters: %ws\n", argv[i]);
      printHelp();
      return false;
    }
    else if (filePath != L"")
    {
      wildcardPath = argv[i];
    }
    else
    {
      filePath = argv[i];
    }
  }

  if (op_help)
  {
    printHelp();
    return false;
  }

  if (wildcardPath == L"")
  {
    fprintf(stderr, "Missing wildcardPath.\n");
    printHelp();
    return false;
  }

  if (filePath == L"")
  {
    fprintf(stderr, "Missing filePath.\n");
    printHelp();
    return false;
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

static int ensureExistingFileCorrectEncState(const std::wstring path,
                                             bool shouldBeEncrypted)
{
  if (shouldBeEncrypted)
  {
    if (!SE_IsEncrypted(path.c_str()))
    {
      printf("Error: %ws should be encrypted, but is not.\n", path.c_str());
      return -1;
    }
  }
  else
  {
    if (SE_IsEncrypted(path.c_str()))
    {
      printf("Error: %ws should not be encrypted, but is.\n", path.c_str());
      return -1;
    }
    else if (GetLastError() != ERROR_SUCCESS)
    {
      printf("Error: Call to SE_IsEncrypted(%ws) failed.\n", path.c_str());
      return -1;
    }
  }

  return 0;
}

static int ensureNewFileCorrectEncState(const std::wstring path,
                                        bool shouldBeEncrypted)
{
  HANDLE h;

  h = CreateFile(path.c_str(), GENERIC_WRITE, 0, NULL, CREATE_NEW,
                 FILE_ATTRIBUTE_NORMAL, NULL);
  if (h == INVALID_HANDLE_VALUE)
  {
    printf("Error: Cannot create %ws.\n", path.c_str());
    return -1;
  }

  CloseHandle(h);

  if (ensureExistingFileCorrectEncState(path, shouldBeEncrypted) != 0)
  {
    return -1;
  }

  if (!DeleteFile(path.c_str()))
  {
    printf("Error: Cannot delete %ws.\n", path.c_str());
    return -1;
  }

  return 0;
}

static int callChildProcess(int argc, const wchar_t * const argv[], int testNum)
{
  // Spawn the child process.
  wchar_t testNumStr[100];
  std::wstring childCmdLine;
  int i;

  _snwprintf_s(testNumStr, 100, _TRUNCATE, L"%d", testNum);
  childCmdLine = argv[0];
  childCmdLine += L" --child=";
  childCmdLine += testNumStr;

  for (i = 1; i < argc; i++)
  {
    childCmdLine += L" ";
    childCmdLine += argv[i];
  }

  return _wsystem(childCmdLine.c_str());
}

static int test1(void)
{
  //
  // Test 1:
  // 1. Create the file without marking it first.  It should not be
  //    encrypted since it has never been marked by this process.
  //
  printf("\nTest 1\n");

  if (op_verbose) printf("\tStep 1\n");
  return ensureNewFileCorrectEncState(filePath, false);
}

static int test2(void)
{
  //
  // Test 2:
  // 1. Mark the file.
  // 2. Create a different file.  It should not be encrypted.
  // 3. Create the file.  It should be encrypted.
  //
  printf("\nTest 2\n");

  const std::wstring otherFilePath = filePath + L"_other";

  // Delete the other file if it exists.
  if (!DeleteFile(otherFilePath.c_str()) &&
      GetLastError() != ERROR_FILE_NOT_FOUND)
  {
    printf("Error: Cannot delete %ws.\n", otherFilePath.c_str());
    return -1;
  }

  if (op_verbose) printf("\tStep 1\n");
  if (!SE_MarkFileAsDRMOneShot(filePath.c_str()))
  {
    printf("Error: Call to SE_MarkFileAsDRMOneShot(%ws) failed.\n",
           filePath.c_str());
    return -1;
  }

  if (op_verbose) printf("\tStep 2\n");
  if (ensureNewFileCorrectEncState(otherFilePath, false) != 0)
  {
    return -1;
  }

  if (op_verbose) printf("\tStep 3\n");
  if (ensureNewFileCorrectEncState(filePath, true) != 0)
  {
    return -1;
  }

  return 0;
}

static int test3(void)
{
  //
  // Test 3:
  // 1. Mark the file.
  // 2. Create the file.  It should be encrypted.
  // 3. Create the file again.  It should not be encrypted.
  //
  printf("\nTest 3\n");

  if (op_verbose) printf("\tStep 1\n");
  if (!SE_MarkFileAsDRMOneShot(filePath.c_str()))
  {
    printf("Error: Call to SE_MarkFileAsDRMOneShot(%ws) failed.\n",
           filePath.c_str());
    return -1;
  }

  if (op_verbose) printf("\tStep 2\n");
  if (ensureNewFileCorrectEncState(filePath, true) != 0)
  {
    return -1;
  }

  if (op_verbose) printf("\tStep 3\n");
  if (ensureNewFileCorrectEncState(filePath, false) != 0)
  {
    return -1;
  }

  return 0;
}

static int test4(int argc, const wchar_t * const argv[])
{
  //
  // Test 4:
  // 1. Mark the file.
  // 2. Let the child process create the file.  It should not be encrypted.
  // 3. Create the file again in this process.  It should be encrypted.
  //
  printf("\nTest 4\n");

  if (op_verbose) printf("\tStep 1\n");
  if (!SE_MarkFileAsDRMOneShot(filePath.c_str()))
  {
    printf("Error: Call to SE_MarkFileAsDRMOneShot(%ws) failed.\n",
           filePath.c_str());
    return -1;
  }

  if (callChildProcess(argc, argv, 4) != 0)
  {
    return -1;
  }

  if (op_verbose) printf("\tStep 3\n");
  if (ensureNewFileCorrectEncState(filePath, true) != 0)
  {
    return -1;
  }

  return 0;
}

static int test4Child(void)
{
  if (op_verbose) printf("Test 4 Child\n");

  if (op_verbose) printf("\tStep 2\n");
  return ensureNewFileCorrectEncState(filePath, false);
}

static int test5(void)
{
  //
  // Test 5:
  // 1. Create the file.
  // 2. Mark the existing file.
  // 3. Open the file for writing.  It should not be encrypted.
  // 4. Delete the file.
  // 5. Create the file again.  It should be encrypted.
  //
  printf("\nTest 5\n");

  HANDLE h;

  if (op_verbose) printf("\tStep 1\n");
  h = CreateFile(filePath.c_str(), GENERIC_WRITE, 0, NULL, CREATE_NEW,
                 FILE_ATTRIBUTE_NORMAL, NULL);
  if (h == INVALID_HANDLE_VALUE)
  {
    printf("Error: Cannot create %ws.\n", filePath.c_str());
    return -1;
  }

  CloseHandle(h);

  if (op_verbose) printf("\tStep 2\n");
  if (!SE_MarkFileAsDRMOneShot(filePath.c_str()))
  {
    printf("Error: Call to SE_MarkFileAsDRMOneShot(%ws) failed.\n",
           filePath.c_str());
    return -1;
  }

  if (op_verbose) printf("\tStep 3\n");
  h = CreateFile(filePath.c_str(), GENERIC_WRITE, 0, NULL, OPEN_ALWAYS,
                 FILE_ATTRIBUTE_NORMAL, NULL);
  if (h == INVALID_HANDLE_VALUE)
  {
    printf("Error: Cannot create %ws.\n", filePath.c_str());
    return -1;
  }

  CloseHandle(h);

  if (ensureExistingFileCorrectEncState(filePath, false) != 0)
  {
    return -1;
  }

  if (op_verbose) printf("\tStep 4\n");
  if (!DeleteFile(filePath.c_str()))
  {
    printf("Error: Cannot delete %ws.\n", filePath.c_str());
    return -1;
  }

  if (op_verbose) printf("\tStep 5\n");
  if (ensureNewFileCorrectEncState(filePath, true) != 0)
  {
    return -1;
  }

  return 0;
}

static int test6(void)
{
  //
  // Test 6:
  // 1. Mark the wildcard path.
  // 2. Create a different file.  It should not be encrypted.
  // 3. Create the file.  It should be encrypted.
  // 4. Create the file.  It should not be encrypted.
  //
  printf("\nTest 6\n");

  const std::wstring otherFilePath = filePath + L"_other";
  
  if (op_verbose) printf("\tStep 1\n");
  if (!SE_MarkFileAsDRMOneShot(wildcardPath.c_str()))
  {
    printf("Error: Call to SE_MarkFileAsDRMOneShot(%ws) failed.\n",
           filePath.c_str());
    return -1;
  }

  if (op_verbose) printf("\tStep 2\n");
  if (ensureNewFileCorrectEncState(otherFilePath, false) != 0)
  {
    return -1;
  }

  if (op_verbose) printf("\tStep 3\n");
  if (ensureNewFileCorrectEncState(filePath, true) != 0)
  {
    return -1;
  }

  if (op_verbose) printf("\tStep 4\n");
  if (ensureNewFileCorrectEncState(filePath, false) != 0)
  {
    return -1;
  }

  return 0;
}

static int test7(void)
{
  //
  // Test 7:
  // 1. Unmark a different file.  Unmarking should fail.
  // 2. Mark the file.
  // 3. Unmark a different file.  Unmarking should fail.
  // 4. Mark a different file.
  // 5. Unmark the file.
  // 6. Create the file.  It should not be encrypted.
  // 7. Mark the file.
  // 8. Unmark the file.
  // 9. Create the file.  It should not be encrypted.
  // 10. Unmark the different file.
  //
  printf("\nTest 7\n");

  const std::wstring otherFilePath = filePath + L"_other";

  if (op_verbose) printf("\tStep 1\n");
  if (SE_UnmarkFileAsDRMOneShot(otherFilePath.c_str()))
  {
    printf("Error: Call to SE_UnmarkFileAsDRMOneShot(%ws) should have failed, but succeeded.\n",
           otherFilePath.c_str());
    return -1;
  }

  if (op_verbose) printf("\tStep 2\n");
  if (!SE_MarkFileAsDRMOneShot(filePath.c_str()))
  {
    printf("Error: Call to SE_MarkFileAsDRMOneShot(%ws) failed.\n",
           filePath.c_str());
    return -1;
  }

  if (op_verbose) printf("\tStep 3\n");
  if (SE_UnmarkFileAsDRMOneShot(otherFilePath.c_str()))
  {
    printf("Error: Call to SE_UnmarkFileAsDRMOneShot(%ws) should have failed, but succeeded.\n",
           otherFilePath.c_str());
    return -1;
  }

  if (op_verbose) printf("\tStep 4\n");
  if (!SE_MarkFileAsDRMOneShot(otherFilePath.c_str()))
  {
    printf("Error: Call to SE_MarkFileAsDRMOneShot(%ws) failed.\n",
           otherFilePath.c_str());
    return -1;
  }

  if (op_verbose) printf("\tStep 5\n");
  if (!SE_UnmarkFileAsDRMOneShot(filePath.c_str()))
  {
    printf("Error: Call to SE_UnmarkFileAsDRMOneShot(%ws) failed.\n",
           filePath.c_str());
    return -1;
  }

  if (op_verbose) printf("\tStep 6\n");
  if (ensureNewFileCorrectEncState(filePath, false) != 0)
  {
    return -1;
  }

  if (op_verbose) printf("\tStep 7\n");
  if (!SE_MarkFileAsDRMOneShot(filePath.c_str()))
  {
    printf("Error: Call to SE_MarkFileAsDRMOneShot(%ws) failed.\n",
           filePath.c_str());
    return -1;
  }

  if (op_verbose) printf("\tStep 8\n");
  if (!SE_UnmarkFileAsDRMOneShot(filePath.c_str()))
  {
    printf("Error: Call to SE_UnmarkFileAsDRMOneShot(%ws) failed.\n",
           filePath.c_str());
    return -1;
  }

  if (op_verbose) printf("\tStep 9\n");
  if (ensureNewFileCorrectEncState(filePath, false) != 0)
  {
    return -1;
  }

  if (op_verbose) printf("\tStep 10\n");
  if (!SE_UnmarkFileAsDRMOneShot(otherFilePath.c_str()))
  {
    printf("Error: Call to SE_UnmarkFileAsDRMOneShot(%ws) failed.\n",
           otherFilePath.c_str());
    return -1;
  }

  return 0;
}

static int test8(void)
{
  //
  // Test 8:
  // 1. Mark a different file 199 times.
  // 2. Mark the file.
  // 3. Create the file.  It should be encrypted.
  // 4. Mark a different file.  Marking should succeed.
  // 5. Mark the file.  Marking should fail.
  // 6. Create the file.  It should not be encrypted.
  // 7. Unmark the different file 200 times.
  // 8. Unmark the different file.  Unmarking should fail.
  //
  printf("\nTest 8\n");

  const std::wstring otherFilePath = filePath + L"_other";
  int i;

  if (op_verbose) printf("\tStep 1\n");
  for (i = 0; i < MAXDRMFILEONESHOTCOUNT_DEFAULT_VALUE - 1; i++)
  {
    if (!SE_MarkFileAsDRMOneShot(otherFilePath.c_str()))
    {
      printf("Error: Call to SE_MarkFileAsDRMOneShot(%ws) failed.\n",
             otherFilePath.c_str());
      return -1;
    }
  }

  if (op_verbose) printf("\tStep 2\n");
  if (!SE_MarkFileAsDRMOneShot(filePath.c_str()))
  {
    printf("Error: Call to SE_MarkFileAsDRMOneShot(%ws) failed.\n",
           filePath.c_str());
    return -1;
  }

  if (op_verbose) printf("\tStep 3\n");
  if (ensureNewFileCorrectEncState(filePath, true) != 0)
  {
    return -1;
  }

  if (op_verbose) printf("\tStep 4\n");
  if (!SE_MarkFileAsDRMOneShot(otherFilePath.c_str()))
  {
    printf("Error: Call to SE_MarkFileAsDRMOneShot(%ws) failed.\n",
           otherFilePath.c_str());
    return -1;
  }

  if (op_verbose) printf("\tStep 5\n");
  if (SE_MarkFileAsDRMOneShot(filePath.c_str()))
  {
    printf("Error: Call to SE_MarkFileAsDRMOneShot(%ws) should have failed, but succeeded.\n",
           filePath.c_str());
    return -1;
  }

  if (op_verbose) printf("\tStep 6\n");
  if (ensureNewFileCorrectEncState(filePath, false) != 0)
  {
    return -1;
  }

  if (op_verbose) printf("\tStep 7\n");
  for (i = 0; i < MAXDRMFILEONESHOTCOUNT_DEFAULT_VALUE; i++)
  {
    if (!SE_UnmarkFileAsDRMOneShot(otherFilePath.c_str()))
    {
      printf("Error: Call to SE_UnmarkFileAsDRMOneShot(%ws) failed.\n",
             otherFilePath.c_str());
      return -1;
    }
  }

  if (op_verbose) printf("\tStep 8\n");
  if (SE_UnmarkFileAsDRMOneShot(otherFilePath.c_str()))
  {
    printf("Error: Call to SE_UnmarkFileAsDRMOneShot(%ws) should have failed, but succeeded.\n",
           otherFilePath.c_str());
    return -1;
  }

  return 0;
}

static int test9(void)
{
  //
  // Test 9:
  // 1. Mark the wildcard path.
  // 2. Unmark a different wildcard path.  Unmarking should fail.
  // 3. Unmark the file.  Unmarking should fail.
  // 4. Create the file.  It should be encrypted.
  // 5. Mark the wildcard path.
  // 6. Unmark the wildcard path.
  // 7. Create the file.  It should not be encrypted.
  //
  printf("\nTest 9\n");

  const std::wstring otherWildcardPath = wildcardPath + L"_other";

  if (op_verbose) printf("\tStep 1\n");
  if (!SE_MarkFileAsDRMOneShot(wildcardPath.c_str()))
  {
    printf("Error: Call to SE_MarkFileAsDRMOneShot(%ws) failed.\n",
           wildcardPath.c_str());
    return -1;
  }

  if (op_verbose) printf("\tStep 2\n");
  if (SE_UnmarkFileAsDRMOneShot(otherWildcardPath.c_str()))
  {
    printf("Error: Call to SE_UnmarkFileAsDRMOneShot(%ws) should have failed, but succeeded.\n",
           otherWildcardPath.c_str());
    return -1;
  }

  if (op_verbose) printf("\tStep 3\n");
  if (SE_UnmarkFileAsDRMOneShot(filePath.c_str()))
  {
    printf("Error: Call to SE_UnmarkFileAsDRMOneShot(%ws) should have failed, but succeeded.\n",
           filePath.c_str());
    return -1;
  }

  if (op_verbose) printf("\tStep 4\n");
  if (ensureNewFileCorrectEncState(filePath, true) != 0)
  {
    return -1;
  }

  if (op_verbose) printf("\tStep 5\n");
  if (!SE_MarkFileAsDRMOneShot(wildcardPath.c_str()))
  {
    printf("Error: Call to SE_MarkFileAsDRMOneShot(%ws) failed.\n",
           wildcardPath.c_str());
    return -1;
  }

  if (op_verbose) printf("\tStep 6\n");
  if (!SE_UnmarkFileAsDRMOneShot(wildcardPath.c_str()))
  {
    printf("Error: Call to SE_UnmarkFileAsDRMOneShot(%ws) failed.\n",
           wildcardPath.c_str());
    return -1;
  }

  if (op_verbose) printf("\tStep 7\n");
  if (ensureNewFileCorrectEncState(filePath, false) != 0)
  {
    return -1;
  }

  return 0;
}

static int test10(void)
{
  //
  // Test 10:
  // 1. Mark the file 200 times.
  // 2. Unmark all files.
  // 3. Create the file.  It should not be encrypted.
  //
  printf("\nTest 10\n");

  int i;

  if (op_verbose) printf("\tStep 1\n");
  for (i = 0; i < MAXDRMFILEONESHOTCOUNT_DEFAULT_VALUE; i++)
  {
    if (!SE_MarkFileAsDRMOneShot(filePath.c_str()))
    {
      printf("Error: Call to SE_MarkFileAsDRMOneShot(%ws) failed.\n",
             filePath.c_str());
      return -1;
    }
  }

  if (op_verbose) printf("\tStep 2\n");
  if (!SE_UnmarkAllFilesAsDRMOneShot())
  {
    printf("Error: Call to SE_UnmarkAllFilesAsDRMOneShot() failed.\n");
    return -1;
  }

  if (op_verbose) printf("\tStep 3\n");
  if (ensureNewFileCorrectEncState(filePath, false) != 0)
  {
    return -1;
  }

  return 0;
}

static int test11(int argc, const wchar_t * const argv[])
{
  //
  // Test 11:
  // 1. Mark the file.
  // 2. Let the child process unmark the file.  Unmarking should fail.
  // 3. Let the child process unmark all files.  Unmarking should fail.
  // 4. Create the file.  It should be encrypted.
  // 
  printf("\nTest 11\n");

  if (op_verbose) printf("\tStep 1\n");
  if (!SE_MarkFileAsDRMOneShot(filePath.c_str()))
  {
    printf("Error: Call to SE_MarkFileAsDRMOneShot(%ws) failed.\n",
           filePath.c_str());
    return -1;
  }

  if (callChildProcess(argc, argv, 11) != 0)
  {
    return -1;
  }

  if (op_verbose) printf("\tStep 4\n");
  if (ensureNewFileCorrectEncState(filePath, true) != 0)
  {
    return -1;
  }

  return 0;
}

static int test11Child(void)
{
  if (op_verbose) printf("Test 11 Child\n");

  if (op_verbose) printf("\tStep 2\n");
  if (SE_UnmarkFileAsDRMOneShot(filePath.c_str()))
  {
    printf("Error: Call to SE_UnmarkFileAsDRMOneShot(%ws) should have failed, but succeeded.\n",
           filePath.c_str());
    return -1;
  }

  if (op_verbose) printf("\tStep 3\n");
  if (SE_UnmarkAllFilesAsDRMOneShot())
  {
    printf("Error: Call to SE_UnmarkAllFilesAsDRMOneShot() should have failed, but succeeded.\n");
    return -1;
  }

  return 0;
}

static int parentProc(int argc, const wchar_t * const argv[])
{
  // Delete the file if it exists.
  if (!DeleteFile(filePath.c_str()) && GetLastError() != ERROR_FILE_NOT_FOUND)
  {
    printf("Error: Cannot delete %ws.\n", filePath.c_str());
    goto failed;
  }

  if (test1() != 0)
  {
    goto failed;
  }

  if (test2() != 0)
  {
    goto failed;
  }

  if (test3() != 0)
  {
    goto failed;
  }

  if (test4(argc, argv) != 0)
  {
    goto failed;
  }

  if (test5() != 0)
  {
    goto failed;
  }

  if (test6() != 0)
  {
    goto failed;
  }

  if (test7() != 0)
  {
    goto failed;
  }

  if (test8() != 0)
  {
    goto failed;
  }

  if (test9() != 0)
  {
    goto failed;
  }

  if (test10() != 0)
  {
    goto failed;
  }

  if (test11(argc, argv) != 0)
  {
    goto failed;
  }

  printf("\nAll tests passed.\n");
  return 0;

failed:
  printf("\nTEST FAILED!\n");
  return -1;
}

static int childProc(void)
{
  switch (_wtoi(child.c_str()))
  {
  case 4:
    return test4Child();
  case 11:
    return test11Child();
  default:
    return -1;
  }
}

int wmain(int argc, const wchar_t * const argv[])
{
  if (!processOptions(argc, argv))
  {
    return 1;
  }

  if (op_child)
  {
    return childProc();
  }
  else
  {
    return parentProc(argc, argv);
  }
} /* wmain */
