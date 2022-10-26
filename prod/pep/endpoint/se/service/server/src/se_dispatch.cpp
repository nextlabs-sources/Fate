
#include <windows.h>
#include <cstdio>
#include <cstdlib>

#include "jni.h"

extern "C" void server_dispatch( const char* in_string , char** out_string );
extern void Initialize(void);

void log( const char* string )
{
  FILE* fp;  // = fopen_s("c:\\agentLog\\foo.txt","a");
  // remove warning C4996
  errno_t err = fopen_s(&fp, "c:\\agentLog\\foo.txt","a");
  if( err != 0 )
	  return;
  if( fp )
  {
    fprintf(fp,"%s",string);
    fclose(fp);
  }
}

void logw( const wchar_t* string )
{
  FILE* fp;  // = fopen("c:\\agentLog\\foo.txt","a");
  errno_t err = fopen_s(&fp, "c:\\agentLog\\foo.txt","a");
  if( err != 0 ) 
	  return;
  if( fp )
  {
    fwprintf(fp,L"%s",string);
	fclose(fp);
  }
}

extern "C"
JNIEXPORT void JNICALL Java_com_bluejungle_EDP_SystemEncryptionService_Initialize
  (JNIEnv * env, jobject obj)
{
  Initialize();
}/* Java_com_bluejungle_EDP_SystemEncryptionService_Initialize */

extern "C"
JNIEXPORT jstring JNICALL Java_com_bluejungle_EDP_SystemEncryptionService_Dispatch
  (JNIEnv * env, jobject obj, jstring in_input)
{
  char* input = NULL;
  char* output = NULL;
  wchar_t* outputw = NULL;
  jstring result_string = NULL;

  const wchar_t *str = (const TCHAR*)env->GetStringChars(in_input,0);
  if( str == NULL )
  {
    goto Dispatch_complete;
  }

  size_t input_count = wcslen(str) + 1;
  input = (char*)malloc( input_count * sizeof(char) );
  if( input == NULL )
  {
    goto Dispatch_complete;
  }

  _snprintf_s(input,input_count, _TRUNCATE,"%ws",str);

  server_dispatch(input,&output);
  if( output == NULL )
  {
    goto Dispatch_complete;
  }

  size_t output_count = strlen(output) + 1;
  outputw = (wchar_t*)malloc( output_count * sizeof(wchar_t) );
  if( outputw == NULL )
  {
    goto Dispatch_complete;
  }

  _snwprintf_s(outputw,output_count, _TRUNCATE,L"%hs",output);

  result_string = env->NewString((const jchar*)outputw, static_cast<jsize> (wcslen(outputw)));

 Dispatch_complete:

  if( input != NULL )
    free(input);

  if( output != NULL )
    free(output);

  if( outputw != NULL )
    free(outputw);

  return result_string;

}/*  Java_com_bluejungle_QANullService_QANullService_Dispatch */
