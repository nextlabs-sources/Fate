
#include <windows.h>
#include <cstdio>
#include <cstdlib>

typedef int (WINAPI *MYPROC)(); 
 
#pragma comment(linker,"/subsystem:\"windows\" /entry:\"mainCRTStartup\"")

int main( int argc , char** argv )
{
  int rv = -1;
  if( argc != 3 ||
      ( strcmp(argv[1],"install") && strcmp(argv[1],"remove") ) )
  {
    fprintf(stdout, "InstallSPICaller [install|remove] [DLL]\n");
    return 1;
  }

 
  HINSTANCE hDLL; 
  MYPROC ProcAdd; 
  BOOL   fRunTimeLinkSuccess = FALSE; 

     
  size_t origsize = strlen(argv[2]) + 1;
  size_t convertedChars = 0;
  wchar_t *wcstring;
  wcstring=(wchar_t *) malloc(sizeof(wchar_t)*origsize);
  mbstowcs_s(&convertedChars, wcstring, origsize, argv[2], _TRUNCATE);

   
  if (NULL != wcstring)
  {
      hDLL = LoadLibrary(wcstring);
  	  free(wcstring);
  }
  else 
  	return 1;

	// If the handle is valid, try to get the function address.
     if (hDLL != NULL) 
        { 
        if( strcmp(argv[1],"install") == 0 )
        {
          fprintf(stdout, "Installing\n");	
          ProcAdd = (MYPROC) GetProcAddress(hDLL, "InstallSPI");       
        } else {
          if( strcmp(argv[1],"remove") == 0 )
          {
            fprintf(stdout, "Removing\n");
            ProcAdd = (MYPROC) GetProcAddress(hDLL, "UninstallSPI"); 
          } else {
            fprintf(stdout, "Invalid second argument:%s\n",argv[1]);
            return -1;
          }
        }

 
        // If the function address is valid, call the function.
        if (NULL != ProcAdd) 
        {
            fRunTimeLinkSuccess = TRUE;
            fprintf(stdout, "Calling function\n");
            rv=ProcAdd();
        } else {
           fprintf(stdout, "Failed to find function in library.\n");
           return -2;
        }

     } 

     return rv;
}/* main */

