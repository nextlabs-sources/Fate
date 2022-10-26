/* Copyright 2005-2006, Voltage Security, all rights reserved.
 */

#include "vibe.h"

#ifndef _VIBE_SAMPLE_UTIL_H
#define _VIBE_SAMPLE_UTIL_H

#ifdef __cplusplus
extern "C" {
#endif

/* Support routine, wait for the user to type "Enter" or "Return"
 * before continuing.
 * <p>This may be useful in asynchronous operations or simply right
 * before exiting (wait until you've read the output).
 * <p>The promptFlag arg is a flag indicating whether the function
 * should print out a prompt to the user. For example,
 * <pre>
 * <code>
 *   Type Enter/Return to continue.
 * </code>
 * </pre>
 * <p>The prompt arg is a string to print out for prompting.
 * <p>If the promptFlag is 0, the function will print out no prompt and
 * the prompt arg is ignored.
 * <p>If the promptFlag is 1, the function will print out a default
 * prompt and the prompt arg is ignored.
 * <p>If the promptFlag is 2, the function will print out the given
 * prompt. If the promptArg is NULL, the function will print out no
 * prompt.
 * <p>If the promptFlag is some other value, the function will treat it
 * as if it were 1 (default prompt message, prompt arg ignored).
 * <p>Regardless of the promptFlag and prompt args, the function will
 * wait for a user response, that response being "Enter" or "Return".
 */
void SampleUtilWaitForResponse (
   unsigned int promptFlag,
   char *prompt
   );

/* This routine will print out a buffer. It will print out each byte of
 * the buffer in hex. The formate will be as follows.
 * <pre>
 * <code>
 *   a4 b1 c0 86 d8 4a cb 3c 57 03 97 76 bf 21 16 74
 *   d8 d0 58 c9 83 c6 10 0f b2 a9 58 32 79 c5
 * </code>
 * <pre>
 * <p>There are no marks (such as 0x or commas) and there are 16 bytes
 * output per line.
 * <p>If the label arg is not NULL, the function will print out
 * whatever is passed in. The function expects the label to be all
 * ASCII characters and NULL-terminated.
 * <p>If the buffer is NULL or the len is 0, the function will print
 * out the label only.
 */
void SampleUtilPrintBuffer (
   char *label,
   unsigned char *buffer,
   unsigned int len
   );

/* This routine will print out a string. It assumes theString is a
 * NULL-terminated ASCII string.
 * <p>If the label arg is not NULL, the function will print out
 * whatever is passed in. The function expects the label to be all
 * ASCII characters and NULL-terminated.
 * <p>If theString is NULL, the function will print out the label only.
 */
void SampleUtilPrintString (
   char *label,
   char *theString
   );

/* This routine will read a line of user input. It reads from stdin
 * (the command-line screen). It reads ASCII characters.
 * <p>This caller will supply a buffer and size. If the user types in
 * more than (bufferSize - 1) bytes, the function will return an error.
 * The user will type in the "Enter" or "Return", which will accoount
 * for the "- 1". So if you want a max of, say, 8 characters, make sure
 * the buffer is at least 9 bytes long.
 * <p>The routine will set inputLen to the number of bytes input.
 * <p>If the flag is 0, set the reader so that the characters typed in
 * are not actually displayed on the screen (echo off). This is for
 * password collection.
 */
int SampleUtilReadLine (
   unsigned char *buffer,
   unsigned int bufferSize,
   unsigned int *inputLen,
   unsigned int flag
   );

/* This routine will print out a result.
 * <p>Print out the value of status as an int, a hex, and a string.
 * <p>If this function cannot perform the task (libCtx is NULL or some
 * faulty value, for instance), it will do nothing.
 */
void SampleUtilPrintResult (
   VtLibCtx libCtx,
   int status
   );

/* This is a utility for sample programs that want to read from or
 * write to a file.
 * <p>This opens a file only. It does not read, write, or move a file
 * descriptor.
 * <p>Set the flag to either SAMPLE_UTIL_OPEN_FILE_READ or
 * SAMPLE_UTIL_OPEN_FILE_WRITE. If you open for WRITE, the file will be
 * created (an old file of the same name will be deleted). Future write
 * functions will append, there is no way to move a file descriptor.
 * <p>The routine will build a single file name from the three string
 * inputs (directory, fileName, and extension). If any of the string
 * args are NULL, the routine will ignore it. The purpose of the
 * extension is generally to add a new extension to an existing
 * fileName. For example, if the fileName is "sample.txt", and the
 * entension is NULL, the function will open "sample.txt". However, if
 * the fileName is "sample.txt" and the extension is ".vsf", the
 * function will open "sample.txt.vsf".
 * <p>The sample utility file functions are not meant to be robust file
 * handling routines. They are simple functions that support simple
 * samples.
 * <p>Return a 0 for success or a nonzero error code. The error code
 * might be a VT_ERROR_ value.
 */
int SampleUtilOpenFile (
   VtLibCtx libCtx,
   char *directory,
   char *fileName,
   char *extension,
   unsigned int flag,
   Pointer *fileHandle
   );

#define SAMPLE_UTIL_OPEN_FILE_READ   1
#define SAMPLE_UTIL_OPEN_FILE_WRITE  2

/* This is a utility for sample programs that have opened a file. Close
 * it when you are done with it.
 * <p>The sample utility file functions are not meant to be robust file
 * handling routines. They are simple functions that support simple
 * samples.
 */
void SampleUtilCloseFile (
   VtLibCtx libCtx,
   Pointer *fileHandle
   );

/* This is a utility for sample programs that want to delete a file.
 * <p>The sample utility file functions are not meant to be robust file
 * handling routines. They are simple functions that support simple
 * samples.
 */
int SampleUtilRemoveFile (
   VtLibCtx libCtx,
   char *fileName
   );

/* Call this utility function to determine a file size of a file opened
 * for reading. If you call this on a file opened for writing, it will
 * return an error.
 * <p>This will tell you the file size, not the number of bytes left
 * unread. That is, if you call this immediately after opening a file
 * for reading and call it again after one or more calls to ReadFile,
 * it will return the same size.
 * <p>The sample utility file functions are not meant to be robust file
 * handling routines. They are simple functions that support simple
 * samples.
 */
int SampleUtilFileSize (
   VtLibCtx libCtx,
   Pointer fileHandle,
   unsigned int *fileSize
   );

/* This is a utility for sample programs that want to write to a file.
 * <p>If the file was opened to read, this function will return an
 * error. The write will append. You can call Write as many times as
 * you'd like, so long as the file is opened (the fileHandle is valid).
 * <p>The sample utility file functions are not meant to be robust file
 * handling routines. They are simple functions that support simple
 * samples.
 * <p>Return a 0 for success or a nonzero error code. The error code
 * might be a VT_ERROR_ value.
 */
int SampleUtilWriteFile (
   VtLibCtx libCtx,
   Pointer fileHandle,
   unsigned char *dataToWrite,
   unsigned int dataToWriteLen
   );

/* This is a utility for sample programs that want to read from a file.
 * <p>If the file was opened to write, this function will return an
 * error. The read will read the specified number of bytes (count)
 * starting wherever the previous read left off. It is not possible to
 * move a file descriptor to different places in the file.
 * <p>The function will place the collected bytes into the given output
 * buffer. It is the responsibility of the caller to pass in a valid
 * buffer of an appropriate size. The function will also set bytesRead
 * to the number of bytes actually read. It will also set endOfFileFlag
 * to either 0 (have not reached the end of the file yet) or 1 (have
 * reached the end of the file).
 * <p>You can call Read as many times as you'd like, so long as the
 * file is opened (the fileHandle is valid). If the file has been
 * entirely read, the function will read nothing, place nothing into
 * the output buffer and set the endOfFileFlag to 1.
 * <p>The sample utility file functions are not meant to be robust file
 * handling routines. They are simple functions that support simple
 * samples.
 * <p>Return a 0 for success or a nonzero error code. The error code
 * might be a VT_ERROR_ value.
 */
int SampleUtilReadFile (
   VtLibCtx libCtx,
   Pointer fileHandle,
   unsigned char *buffer,
   unsigned int count,
   unsigned int *bytesRead,
   unsigned int *endOfFileFlag
   );

/* Compare two buffers. If the two buffers are the same (same length,
 * same contents), the function sets cmpResult to 0. If the lengths are
 * different, the function sets cmpResult to -1. If the lengths are the
 * same, but the contents are different, the function sets cmpResult to
 * +1. The return value is 0 for success or nonzero error code. If the
 * two buffers do not compare to the same thing, the return value can
 * still be 0. The return indicates whether the function did its task,
 * its task being to compare. To compare and find a difference is a
 * successful completion of the task.
 */
int SampleUtilCompareBuffers (
   VtLibCtx libCtx,
   unsigned char *buffer1,
   unsigned int len1,
   unsigned char *buffer2,
   unsigned int len2,
   int *cmpResult
   );

/* Compare the contents of two files.
 * <p>This support utilitiy routine will open the two files for
 * reading, then compare the contents. If they are not the same length,
 * the function will set cmpResult to a positive number, the difference
 * between the two lengths. If they are the same length, but the
 * contents are different, the function will set cmpResult to a
 * negative number. If the lengths are the same and the contents are
 * the same, the function will set cmpResult to 0.
 */
int SampleUtilCompareFiles (
   VtLibCtx libCtx,
   char *directory1,
   char *fileName1,
   char *extension1,
   char *directory2,
   char *fileName2,
   char *extension2,
   int *cmpResult
   );

/* Find the length of theString. This function expects the string to be
 * ASCII and NULL-terminated. The length does not include the
 * NULL-terminating character.
 */
unsigned int SampleUtilStrlen (
   char *theString
   );

/* Compare two strings. If they are the same length and the contents
 * are identical, the function returns 0. If string1 is lexigraphically
 * less than string2, return a number < 0. Otherwise, return a number >
 * 0.
 */
int SampleUtilStrcmp (
   char *string1,
   char *string2
   );

#ifdef __cplusplus
}
#endif

#endif /* _VIBE_SAMPLE_UTIL_H */
