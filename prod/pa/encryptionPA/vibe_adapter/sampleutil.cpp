/* Copyright 2005-2006, Voltage Security, all rights reserved.
 */
#include "stdafx.h"

#include "sampleutil.h"

#include <stdio.h>
#include <string.h>

#ifdef WIN32
#include <conio.h>
#else
#include <termios.h>
/* If flag is 0, turn echo off, otherwise, turn it on.
 */
static int SetEcho (unsigned int flag);
#endif

void SampleUtilWaitForResponse (
   unsigned int promptFlag,
   char *prompt
   )
{
  int status;
  char *defaultMsg = "Type Enter/Return to continue.";
  char *msgToUse = (char *)0;

  switch (promptFlag)
  {
    case 0:
      break;

    default:
    case 1:
      msgToUse = defaultMsg;
      break;

    case 2:
      msgToUse = prompt;
  }

  if (msgToUse != (char *)0)
    printf ("\n%s\n", msgToUse);

  fflush (stdin);
  status = getchar ();
}

void SampleUtilPrintBuffer (
   char *label,
   unsigned char *buffer,
   unsigned int len
   )
{
  unsigned int index, limit;

  if (label != (char *)0)
    printf ("\n%s", label);

  printf ("\n");

  if ( (buffer != (unsigned char *)0) && (len != 0) )
  {
    index = 0;
    limit = len - 1;
    do
    {
      printf (" %02x", buffer[index]);

      if ((index & 15) == 15)
        printf ("\n");

      index++;

    } while (index <= limit);

    printf ("\n");
  }
}

void SampleUtilPrintString (
   char *label,
   char *theString
   )
{
  if (label != (char *)0)
    printf ("\n%s", label);

  if (theString != (char *)0)
    printf ("\n%s", theString);
}

#ifdef WIN32
typedef int (*TheGetCharacter) (void);

int SampleUtilReadLine (
   unsigned char *buffer,
   unsigned int bufferSize,
   unsigned int *inputLen,
   unsigned int flag
   )
{
  int nextChar;
  unsigned int index, maxIndex;
  TheGetCharacter TheGet;

  *inputLen = 0;

  /* _getche echos the typed in value to the keyboard, _getch
   * suppresses the echo.
   */
  TheGet = _getche;
  if (flag == 0)
    TheGet = _getch;

  index = 0;
  maxIndex = bufferSize - 2;

  int iCondition = 1;
  do
  {
    nextChar = TheGet ();
    if ( (nextChar == 0x0d) || (nextChar == 0x0a) )
      break;

    buffer[index] = (unsigned char)nextChar;
    index++;

    if (index > maxIndex)
      return (VT_ERROR_INVALID_INPUT);

  } while (iCondition);

  buffer[index] = 0;

  return (0);
}
#else
int SampleUtilReadLine (
   unsigned char *buffer,
   unsigned int bufferSize,
   unsigned int *inputLen,
   unsigned int flag
   )
{
  int status;
  unsigned int index;
  char *retVal;

  *inputLen = 0;

  if (flag == 0)
  {
    status = SetEcho (0);
    if (status != 0)
      return (status);
  }

  retVal = fgets ((char *)buffer, (int)bufferSize, stdin);

  if (flag == 0)
    SetEcho (1);

  if (retVal == (char *)0)
    return (VT_ERROR_INVALID_INPUT);

  /* fgets appended a NULL-term char, but any new line chars are still
   * there. Replace them with zeroes.
   */
  index = strlen (retVal);
  index--;

  for (; index > 0; --index)
  {
    if ( (retVal[index] != 0x0d) && (retVal[index] != 0x0a) )
      break;

    retVal[index] = 0;
  }

  if (index == 0)
    return (VT_ERROR_INVALID_INPUT);

  return (0);
}
#endif

void SampleUtilPrintResult (
   VtLibCtx libCtx,
   int status
   )
{
  int newStatus;
  unsigned int index;
  char *errorAsString, *theString;
  VtErrorCtx getErrorCtx;
  VtErrorStack *currentStack;

  errorAsString = VtGetErrorAsASCIIString (libCtx, status);
  if (errorAsString != (char *)0)
    printf ("\nstatus = %i = 0x%04x\n%s\n", status, status, errorAsString);

  /* If there is no error, no need to do anything further.
   */
  if (status == 0)
    return;

  /* If there is an error, look at the error ctx, if there is one and
   * everything works.
   */
  newStatus = VtGetLibCtxParam (
    libCtx, VtLibCtxParamErrorCtx, (Pointer *)&getErrorCtx);
  if (newStatus != 0)
    return;

  if (getErrorCtx == (VtErrorCtx)0)
    return;

  /* Get the first stack (current implementations allow only one stack
   * anyway.
   */
  newStatus = VtGetErrorStack (getErrorCtx, 0, &currentStack);
  if (newStatus != 0)
    return;

  if (currentStack == (VtErrorStack *)0)
    return;

  printf ("\n");
  for (index = 0; index < currentStack->count; ++index)
  {
    theString = "Unknown function";
    if (currentStack->elementList[index].functionName != (char *)0)
      theString = currentStack->elementList[index].functionName;
    printf ("function: %s\n", theString);

    printf (
      "function line = %d\n", currentStack->elementList[index].functionLine);

    theString = "Unknown error string";
    if (currentStack->elementList[index].errorAsString != (char *)0)
      theString = currentStack->elementList[index].errorAsString;
    printf ("status = %s\n", theString);
  }
}

typedef struct
{
  FILE *handle;
  unsigned int readOrWrite;
  unsigned int fileSize;
  unsigned int bytesRead;
} SampleUtilFileHandle;

int SampleUtilOpenFile (
   VtLibCtx libCtx,
   char *directory,
   char *fileName,
   char *extension,
   unsigned int flag,
   Pointer *fileHandle
   )
{
  int status;
  unsigned int count, offset, dirLen, nameLen, extLen, totalLen;
  SampleUtilFileHandle *newHandle = (SampleUtilFileHandle *)0;
  char *fullName = (char *)0;
#ifdef WIN32
  char separator[1] = { '\\' };
#else
  char separator[1] = { '/' };
#endif
  FILE *handle = (FILE *)0;

  *fileHandle = (Pointer)0;

  dirLen = 0;
  nameLen = 0;
  extLen = 0;
  count = 0;

  int iCondtion = 0;
  do
  {
    /* Build the full file name.
     */
    if (directory != (char *)0)
    {
      dirLen = (unsigned int)strlen (directory);
      count++;
    }
    if (fileName != (char *)0)
      nameLen = (unsigned int)strlen (fileName);
    if (extension != (char *)0)
      extLen = (unsigned int)strlen (extension);

    totalLen = dirLen + nameLen + extLen + count * sizeof (separator);
    status = VtMalloc (libCtx, totalLen + 1, 0, (Pointer *)&fullName);
    if (status != 0)
      break;

    VtMemcpy (libCtx, (Pointer)fullName, (Pointer)directory, dirLen);
    offset = dirLen;
    if (count != 0)
    {
      VtMemcpy (libCtx, (Pointer)fullName + offset, (Pointer)separator, sizeof (separator));
      offset += sizeof (separator);
    }
    VtMemcpy (libCtx, (Pointer)fullName + offset, (Pointer)fileName, nameLen);
    offset += nameLen;
    VtMemcpy (libCtx, (Pointer)fullName + offset, (Pointer)extension, extLen);
    fullName[totalLen] = 0;

    status = VtMalloc (
      libCtx, sizeof (SampleUtilFileHandle), 0, (Pointer *)&newHandle);
    if (status != 0)
      break;

    status = VtMemset (
      libCtx, (Pointer)newHandle, 0, sizeof (SampleUtilFileHandle));

    status = VT_ERROR_FILE_OPEN;
    if (flag == SAMPLE_UTIL_OPEN_FILE_READ)
    {
      fopen_s(&handle, fullName, "rb");
      if (handle == (FILE *)0)
        break;

      if (fseek (handle, 0, SEEK_END) != 0)
        break;
      newHandle->fileSize = (unsigned int)ftell (handle);
      if (fseek (handle, 0, SEEK_SET) != 0)
        break;

      newHandle->readOrWrite = SAMPLE_UTIL_OPEN_FILE_READ;
    }
    else
    {
      fopen_s(&handle, fullName, "wb");
      if (handle == (FILE *)0)
        break;

      newHandle->readOrWrite = SAMPLE_UTIL_OPEN_FILE_WRITE;
    }

    status = 0;

    newHandle->handle = handle;
    *fileHandle = (Pointer)newHandle;

  } while (iCondtion);

  VtFree (libCtx, (Pointer *)&fullName);

  if (status == 0)
    return (0);

  if (handle != (FILE *)0)
    fclose (handle);

  VtFree (libCtx, (Pointer *)&newHandle);

  return (status);
}

void SampleUtilCloseFile (
   VtLibCtx libCtx,
   Pointer *fileHandle
   )
{
  SampleUtilFileHandle *theHandle;

  if (fileHandle == (Pointer *)0)
    return;
  if (*fileHandle == (Pointer)0)
    return;

  theHandle = (SampleUtilFileHandle *)(*fileHandle);
  if (theHandle->handle != (FILE *)0)
    fclose (theHandle->handle);

  VtFree (libCtx, fileHandle);
}

int SampleUtilRemoveFile (
   VtLibCtx libCtx,
   char *fileName
   )
{
  int status;
  UNUSED(libCtx);

  status = remove (fileName);
  if (status == 0)
    return (0);

  return (VT_ERROR_FILE_DELETE);
}

int SampleUtilFileSize (
   VtLibCtx libCtx,
   Pointer fileHandle,
   unsigned int *fileSize
   )
{
  int status;
  SampleUtilFileHandle *theHandle;

  UNUSED(libCtx);

  *fileSize = 0;

  int iCondtion = 0;
  do
  {
    status = VT_ERROR_NULL_ARG;
    if (fileHandle == (Pointer)0)
      break;

    status = VT_ERROR_FILE_INVALID_HANDLE;
    theHandle = (SampleUtilFileHandle *)fileHandle;
    if (theHandle->readOrWrite != SAMPLE_UTIL_OPEN_FILE_READ)
      break;

    *fileSize = theHandle->fileSize;
    status = 0;

  } while (iCondtion);

  return (status);
}

int SampleUtilWriteFile (
   VtLibCtx libCtx,
   Pointer fileHandle,
   unsigned char *dataToWrite,
   unsigned int dataToWriteLen
   )
{
  int status;
  unsigned int bytesWritten;
  SampleUtilFileHandle *theHandle;

  UNUSED(libCtx);

  int iCondtion = 0;
  do
  {
    status = VT_ERROR_NULL_ARG;
    if (fileHandle == (Pointer)0)
      break;

    if (dataToWrite == (unsigned char *)0)
      break;

    status = VT_ERROR_FILE_INVALID_HANDLE;
    theHandle = (SampleUtilFileHandle *)fileHandle;
    if (theHandle->readOrWrite != SAMPLE_UTIL_OPEN_FILE_WRITE)
      break;

    status = VT_ERROR_FILE_WRITE;
    bytesWritten = (unsigned int)fwrite (
      (void *)dataToWrite, 1, (size_t)dataToWriteLen, theHandle->handle);
    if (bytesWritten != dataToWriteLen)
      break;

    status = 0;

  } while (iCondtion);

  return (status);
}

int SampleUtilReadFile (
   VtLibCtx libCtx,
   Pointer fileHandle,
   unsigned char *buffer,
   unsigned int count,
   unsigned int *bytesRead,
   unsigned int *endOfFileFlag
   )
{
  int status;
  unsigned int countRead, toRead;
  SampleUtilFileHandle *theHandle;

  UNUSED(libCtx);

  *bytesRead = 0;
  *endOfFileFlag = 0;

  int iCondtion = 0;
  do
  {
    status = VT_ERROR_NULL_ARG;
    if ( (fileHandle == (Pointer)0) || (buffer == (unsigned char *)0) )
      break;

    status = VT_ERROR_FILE_INVALID_HANDLE;
    theHandle = (SampleUtilFileHandle *)fileHandle;
    if (theHandle->readOrWrite != SAMPLE_UTIL_OPEN_FILE_READ)
      break;

    /* How many bytes are still available to be read? If the caller
     * asked for more bytes than are available, read only what is
     * available.
     */
    toRead = theHandle->fileSize - theHandle->bytesRead;
    if (count > toRead)
      count = toRead;

    if (count != 0)
    {
      status = VT_ERROR_FILE_READ;
      countRead = (unsigned int)fread (
        (void *)buffer, 1, (size_t)count, theHandle->handle);
      if (countRead != count)
        break;
    }

    *bytesRead = count;

    theHandle->bytesRead += count;

    if (theHandle->bytesRead == theHandle->fileSize)
      *endOfFileFlag = 1;

    status = 0;

  } while (iCondtion);

  return (status);
}

int SampleUtilCompareBuffers (
   VtLibCtx libCtx,
   unsigned char *buffer1,
   unsigned int len1,
   unsigned char *buffer2,
   unsigned int len2,
   int *cmpResult
   )
{
  int status, vtResult;

  *cmpResult = -1;
  if (len1 != len2)
    return (0);

  *cmpResult = 0;
  status = VtMemcmp (libCtx, buffer1, buffer2, len1, &vtResult);
  if (status != 0)
    return (status);

  if (vtResult == 0)
    return (0);

  *cmpResult = 1;
  return (0);
}

#define SAMPLE_UTIL_CMP_BUFFER_SIZE  128

int SampleUtilCompareFiles (
   VtLibCtx libCtx,
   char *directory1,
   char *fileName1,
   char *extension1,
   char *directory2,
   char *fileName2,
   char *extension2,
   int *cmpResult
   )
{
  int status, fileSize1, fileSize2, memcmpResult = 1;
  unsigned int bytesRead1, bytesRead2, endOfFile1, endOfFile2;
  Pointer fileHandle1 = (Pointer)0;
  Pointer fileHandle2 = (Pointer)0;
  unsigned char buffer1[SAMPLE_UTIL_CMP_BUFFER_SIZE];
  unsigned char buffer2[SAMPLE_UTIL_CMP_BUFFER_SIZE];

  int iCondtion = 0;
  do
  {
    /* Open the two files and get their sizes.
     */
    status = SampleUtilOpenFile (
      libCtx, directory1, fileName1, extension1, SAMPLE_UTIL_OPEN_FILE_READ,
      &fileHandle1);
    if (status != 0)
      break;

    status = SampleUtilFileSize (
      libCtx, fileHandle1, (unsigned int *)&fileSize1);
    if (status != 0)
      break;

    status = SampleUtilOpenFile (
      libCtx, directory2, fileName2, extension2, SAMPLE_UTIL_OPEN_FILE_READ,
      &fileHandle2);
    if (status != 0)
      break;

    status = SampleUtilFileSize (
      libCtx, fileHandle2, (unsigned int *)&fileSize2);
    if (status != 0)
      break;

    /* Are the two files the same size?
     */
    if (fileSize1 >= fileSize2)
      fileSize1 -= fileSize2;
    else
      fileSize1 = fileSize2 - fileSize1;

    *cmpResult = fileSize1;
    if (fileSize1 != 0)
      break;

    do
    {
      /* Get the next block of data.
       */
      status = SampleUtilReadFile (
        libCtx, fileHandle1, buffer1, SAMPLE_UTIL_CMP_BUFFER_SIZE,
        &bytesRead1, &endOfFile1);
      if (status != 0)
        break;

      status = SampleUtilReadFile (
        libCtx, fileHandle2, buffer2, SAMPLE_UTIL_CMP_BUFFER_SIZE,
        &bytesRead2, &endOfFile2);
      if (status != 0)
        break;

      status = VT_ERROR_FILE_READ;
      if (endOfFile1 != endOfFile2)
        break;
      if (bytesRead1 != bytesRead2)
        break;

      status = VtMemcmp (
        libCtx, buffer1, buffer2, bytesRead1, &memcmpResult);
      if (status != 0)
        break;

      if (memcmpResult != 0)
        break;

    } while (endOfFile1 == 0);

    /* If memcmpResult is not 0, the data differed. Set cmpResult to a
     * negative number. If memcmpResult is 0, then we compared each
     * block and they all compared equal. cmpResult was set to 0
     * earlier, so it's what it is supposed to be.
     */
    if (memcmpResult != 0)
      *cmpResult = -1;

  } while (iCondtion);

  VtMemset (libCtx, buffer1, 0, SAMPLE_UTIL_CMP_BUFFER_SIZE);
  VtMemset (libCtx, buffer2, 0, SAMPLE_UTIL_CMP_BUFFER_SIZE);

  SampleUtilCloseFile (libCtx, &fileHandle1);
  SampleUtilCloseFile (libCtx, &fileHandle2);

  return (status);
}

unsigned int SampleUtilStrlen (
   char *theString
   )
{
  return ((unsigned int)strlen (theString));
}

int SampleUtilStrcmp (
   char *string1,
   char *string2
   )
{
  return (strcmp (string1, string2));
}

#ifndef WIN32
static int SetEcho (
   unsigned int flag
   )
{
  struct termios info;

  if (tcgetattr (0, &info) != 0)
    return (VT_ERROR_INVALID_INPUT);

  /* Set the ECHO flag. If this is what we want, we're done. If we want
   * to clear it, then we can XOR and guarantee it's cleared.
   */
  info.c_lflag |= ECHO;

  if (flag == 0)
    info.c_lflag ^= ECHO;

  if (tcsetattr (0, TCSANOW, &info) != 0)
    return (VT_ERROR_INVALID_INPUT);
}
#endif
