/******************************************************************************
 *
 * NextLabs Dump Header Tool
 *
 * Dump the header and stream info of NextLabs files.
 *
 *****************************************************************************/

#include <Windows.h>
#include <stdlib.h>
#include <stdio.h>
#include "NextLabsEncryption_Types.h"
#include "NextLabsTagging_Types.h"
#include "nl_sysenc_lib.h"
#include "nl_sysenc_lib_fw.h"



static bool op_verbose = false;
static bool op_help = false;
static const wchar_t *fName = NULL;
static wchar_t fullPathName[MAX_PATH];



static void printHelp(const wchar_t *cmd)
{
  wprintf(L"\n");
  wprintf(L"Usage: %s option... file\n", cmd);
  wprintf(L"\n");
  wprintf(L"Options:\n");
  wprintf(L"   --verbose                Verbose output.\n");
  wprintf(L"   --help, -h, /?           This screen.\n");
  wprintf(L"\n");
  wprintf(L"Dump the header of a NextLabs file.\n");
  wprintf(L"\n");
  wprintf(L"Example:\n");
  wprintf(L"\n");
  wprintf(L"  %s foo.txt\n", cmd);
} /* printHelp */

static bool processOptions(int argc, const wchar_t* argv[])
{
  int i;

  for (i = 1; i < argc; i++)
  {
    const wchar_t* option = wcsstr(argv[i], L"=");
    if (option != NULL)
    {
      option++;
    }

    if (_wcsicmp(argv[i], L"--verbose") == 0)
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
      printHelp(argv[0]);
      return false;
    }
    else if (fName == NULL)
    {
      fName = argv[i];
    }
    else
    {
      fprintf(stderr, "Too many parameters: %ws\n", argv[i]);
      printHelp(argv[0]);
      return false;
    }
  }

  if (op_help)
  {
    printHelp(argv[0]);
    return false;
  }

  if (fName == NULL)
  {
    fprintf(stderr, "Missing file.\n");
    printHelp(argv[0]);
    return false;
  }

  return true;
} /* processOptions */

void printByteArray(const unsigned char array[], int len)
{
  const int bytesPerLine = 8;

  for (int i = 0; i < len; i++)
  {
    if (i % bytesPerLine == 0)
      printf("\t");

    printf("0x%02hX", (unsigned short) array[i]);

    if (i < len - 1)
      printf(", ");

    if ((i + 1) % bytesPerLine == 0 || i == len - 1)
      printf("\n");
  }
} /* printByteArray */

int printNLHeader10(void)
{
  NextLabsFile_Header_1_0_t nl10;

  if (!SE_GetFileInfoFW(fullPathName, SE_FileInfo_NextLabs, &nl10))
  {
    fwprintf(stderr, L"Can't get NextLabs header 1.0 info\n");
    return -1;
  }

  printf("NextLabsFile_Header_1_0_t.version_major = %hu\n",
         (unsigned short) nl10.version_major);
  printf("NextLabsFile_Header_1_0_t.version_minor = %hu\n",
         (unsigned short) nl10.version_minor);
  printf("NextLabsFile_Header_1_0_t.header_size = %u\n", nl10.header_size);
  printf("NextLabsFile_Header_1_0_t.stream_count = %u\n", nl10.stream_count);
  printf("NextLabsFile_Header_1_0_t.cookie = {\n");
  printByteArray(nl10.cookie, _countof(nl10.cookie));
  printf("}\n");

  printf("\n");
  return 0;
}

int printNLEStream10(void)
{
  NextLabsEncryptionFile_Header_1_0_t nle10;

  if (!SE_GetFileInfoFW(fullPathName, SE_FileInfo_NextLabsEncryption, &nle10))
  {
    fwprintf(stderr, L"Can't get NextLabsEncryption stream 1.0 info\n");
    return -1;
  }

  printf("NextLabsEncryptionFile_Header_1_0_t.sh.stream_size = %u\n",
         nle10.sh.stream_size);
  printf("NextLabsEncryptionFile_Header_1_0_t.sh.stream_name = \"%.8s\"\n",
         nle10.sh.stream_name);
  printf("NextLabsEncryptionFile_Header_1_0_t.version_major = %hu\n",
         (unsigned short) nle10.version_major);
  printf("NextLabsEncryptionFile_Header_1_0_t.version_minor = %hu\n",
         (unsigned short) nle10.version_minor);
  printf("NextLabsEncryptionFile_Header_1_0_t.pcKeyRingName = \"%.16s\"\n",
         nle10.pcKeyRingName);

  printf("NextLabsEncryptionFile_Header_1_0_t.pcKeyID.hash = {\n");
  printByteArray(nle10.pcKeyID.hash, _countof(nle10.pcKeyID.hash));
  printf("}\n");

  printf("NextLabsEncryptionFile_Header_1_0_t.pcKeyID.timestamp = 0x%08lX\n",
         nle10.pcKeyID.timestamp);
  printf("NextLabsEncryptionFile_Header_1_0_t.fileRealLength = %I64u\n",
         nle10.fileRealLength);

  printf("NextLabsEncryptionFile_Header_1_0_t.key = {\n");
  printByteArray(nle10.key, _countof(nle10.key));
  printf("}\n");

  printf("NextLabsEncryptionFile_Header_1_0_t.paddingLen = %lu\n",
         nle10.paddingLen);

  printf("NextLabsEncryptionFile_Header_1_0_t.paddingData = {\n");
  printByteArray(nle10.paddingData, _countof(nle10.paddingData));
  printf("}\n");

  printf("\n");
  return 0;
}

int printNLHeader(void)
{
  NextLabsFile_Header_t nl;

  if (!SE_GetFileInfo(fullPathName, SE_FileInfo_NextLabs, &nl))
  {
    fwprintf(stderr, L"Can't get NextLabs header info\n");
    return -1;
  }

  printf("NextLabsFile_Header_t.version_major = %hu\n",
         (unsigned short) nl.version_major);
  printf("NextLabsFile_Header_t.version_minor = %hu\n",
         (unsigned short) nl.version_minor);
  printf("NextLabsFile_Header_t.header_size = %u\n", nl.header_size);
  printf("NextLabsFile_Header_t.stream_count = %u\n", nl.stream_count);

  printf("NextLabsFile_Header_t.cookie = {\n");
  printByteArray(nl.cookie, _countof(nl.cookie));
  printf("}\n");

  printf("NextLabsFile_Header_t.attrs = 0x%08lX\n", nl.attrs);
  printf("NextLabsFile_Header_t.flags = 0x%016I64X\n", nl.flags);
  printf("\tNLF_WRAPPED is %s\n", nl.flags & NLF_WRAPPED ? "set" : "clear");
  printf("NextLabsFile_Header_t.orig_file_name = \"%.256ws\"\n",
         nl.orig_file_name);

  printf("\n");
  return 0;
} /* printNLHeader */

int printNLEStream(void)
{
  NextLabsEncryptionFile_Header_t nle;

  if (!SE_GetFileInfo(fullPathName, SE_FileInfo_NextLabsEncryption, &nle))
  {
    fwprintf(stderr, L"Can't get NextLabsEncryption stream info\n");
    return -1;
  }

  printf("NextLabsEncryptionFile_Header_t.sh.stream_size = %u\n",
         nle.sh.stream_size);
  printf("NextLabsEncryptionFile_Header_t.sh.stream_name = \"%.8s\"\n",
         nle.sh.stream_name);
  printf("NextLabsEncryptionFile_Header_t.version_major = %hu\n",
         (unsigned short) nle.version_major);
  printf("NextLabsEncryptionFile_Header_t.version_minor = %hu\n",
         (unsigned short) nle.version_minor);
  printf("NextLabsEncryptionFile_Header_t.pcKeyRingName = \"%.16s\"\n",
         nle.pcKeyRingName);

  printf("NextLabsEncryptionFile_Header_t.pcKeyID.hash = {\n");
  printByteArray(nle.pcKeyID.hash, _countof(nle.pcKeyID.hash));
  printf("}\n");

  printf("NextLabsEncryptionFile_Header_t.pcKeyID.timestamp = 0x%08lX\n",
         nle.pcKeyID.timestamp);
  printf("NextLabsEncryptionFile_Header_t.fileRealLength = %I64u\n",
         nle.fileRealLength);

  printf("NextLabsEncryptionFile_Header_t.key = {\n");
  printByteArray(nle.key, _countof(nle.key));
  printf("}\n");

  printf("NextLabsEncryptionFile_Header_t.flags = 0x%016I64X\n", nle.flags);
  printf("\tNLEF_REQUIRES_LOCAL_ENCRYPTION is %s\n",
         nle.flags & NLEF_REQUIRES_LOCAL_ENCRYPTION ? "set" : "clear");
  printf("NextLabsEncryptionFile_Header_t.paddingLen = %lu\n", nle.paddingLen);

  printf("NextLabsEncryptionFile_Header_t.paddingData = {\n");
  printByteArray(nle.paddingData, _countof(nle.paddingData));
  printf("}\n");

  printf("\n");
  return 0;
} /* printNLEStream */

int printNLTStream(void)
{
  NextLabsTaggingFile_Header_t nlt;

  if (!SE_GetFileInfo(fullPathName, SE_FileInfo_NextLabsTagging, &nlt))
  {
    fwprintf(stderr, L"Can't get NextLabsTagging stream info\n");
    return -1;
  }

  printf("NextLabsTaggingFile_Header_t.sh.stream_size = %u\n",
         nlt.sh.stream_size);
  printf("NextLabsTaggingFile_Header_t.sh.stream_name = \"%.8s\"\n",
         nlt.sh.stream_name);
  printf("NextLabsTaggingFile_Header_t.version_major = %hu\n",
         (unsigned short) nlt.version_major);
  printf("NextLabsTaggingFile_Header_t.version_minor = %hu\n",
         (unsigned short) nlt.version_minor);
  printf("NextLabsTaggingFile_Header_t.tagsSize = %lu\n", nlt.tagsSize);

  printf("NextLabsTaggingFile_Header.tag_data = {\n");
  for (size_t i = 0;
       i < nlt.tagsSize / sizeof(wchar_t);
       i += wcslen(&nlt.tag_data[i]) + 1)
  {
    printf("\t%ws\n", &nlt.tag_data[i]);
  }
  printf("}\n");

  printf("\n");
  return 0;
} /* printNLTStream */

int wmain( int argc , const wchar_t* argv[] )
{
  if (!processOptions(argc, argv))
  {
    goto exit;
  }

  DWORD ret2;

  ret2 = GetFullPathName(fName, _countof(fullPathName), fullPathName, NULL);
  if (ret2 >= _countof(fullPathName) || ret2 == 0)
  {
    fprintf(stderr, "Error accessing %ws: \n", fName);
    return -1;
  }

  if (SE_IsEncryptedFW(fullPathName))
  {
    if (printNLHeader10() != 0)
      return -1;

    if (printNLEStream10() != 0)
      return -1;
  }
  else
  {
    if (printNLHeader() != 0)
      return -1;

    if (printNLEStream() != 0)
      return -1;

    if (printNLTStream() != 0)
      return -1;
  }

exit:
  return 0;
} /* wmain */
