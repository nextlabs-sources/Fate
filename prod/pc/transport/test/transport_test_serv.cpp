/***************************************************************************
Module:  transport_win32_test.cpp
***************************************************************************/

// Include the standard Windows and C-Runtime header files here.include
//#include <windows.h>


// Include the exported data structures, symbols, functions, and variables.
//#include "winsock.h"
#include "transport.h"

int getIndex(int len,char* buf)
{
  char tmpbuf[100];
  for(int i=0;i<len;i++)
    {
      if(buf[i]!=';')
	tmpbuf[i] = buf[i];
      else
	{
	  tmpbuf[i] = 0;
	  return atoi(tmpbuf);
	}
    }
  return -1;
}


////////////////////////////////////////////////////////////////////////////

//int WINAPI WinMain(HINSTANCE hinstExe, HINSTANCE, LPTSTR pszCmdLine, int) {
int main(void){

  //MessageBox(NULL, "world", TEXT("hello"), MB_OK);
  if(TRANSPORT_Serv_Initialize()!=CE_RESULT_SUCCESS)
    return 1;
  TRANSPORT_QUEUE_ITEM qi;
  while(1)
    {
      TRANSPORT_Serv_GetNextRequest(&qi);

      //TRACE(1,_T("time after TRANSPORT_Serv_GetNextRequest: %f\n"),NL_GetCurrentTimeInMillisec() );
 
      //printf("!!got data:%s\n",qi.buf);
      //printf("!!!!got request #%d.\n",getIndex(qi.buflen,qi.buf));
      TRANSPORT_Sendn(qi.sock,qi.buflen,qi.buf);
      //!!remember to free the memeory
      TRANSPORT_MemoryFree(qi);
    }
}

////////////////////////////// End of File /////////////////////////////////
