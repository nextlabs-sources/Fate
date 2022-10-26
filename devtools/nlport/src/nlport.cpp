#include <cstdio>
#include <cstdlib>
#include <string>
#include <winsock2.h>
#include <windows.h>

int wmain( int argc , wchar_t** argv )
{
  bool op_verbose = false;
  std::size_t op_count = 1;  
  std::size_t op_delay = 0; 
  std::size_t op_port = 9100;
  std::string op_host("127.0.0.1");

  /* Process options */
  for( int i = 1 ; i < argc ; ++i )
  {
    wchar_t* option = wcsstr(argv[i],L"=");
    if (option == NULL)
    {
        continue;
    }
    
    option++;

    if( wcsncmp(argv[i],L"--count=",wcslen(L"--count=")) == 0 )
    {
      op_count = _wtoi(option);
      continue;
    }
    if( wcsncmp(argv[i],L"--port=",wcslen(L"--port=")) == 0 )
    {
      op_port = _wtoi(option);
      continue;
    }
    if( wcsncmp(argv[i],L"--delay=",wcslen(L"--delay=")) == 0 )
    {
      op_delay = _wtoi(option);
      continue;
    }
    if( wcsncmp(argv[i],L"--host=",wcslen(L"--host=")) == 0 )
    {
      std::wstring hostw(option);
      std::string hosta(hostw.begin(),hostw.end());
      op_host.assign(hosta);
      continue;
    }
    else if( _wcsicmp(argv[i],L"--verbose") == 0 )
    {
      op_verbose = true;
      continue;
    }
  }/* for */

  /* There must be at least one option */
  if( argc == 2 && (_wcsicmp(argv[1],L"--help") == 0 ||
		    _wcsicmp(argv[1],L"-h") == 0 ||
		    _wcsicmp(argv[1],L"/?") == 0) )
  {
    fprintf(stdout, "NextLabs Port (Built %s %s)\n", __DATE__, __TIME__);
    fprintf(stdout, "usage: nlport [option] ...\n");
    fprintf(stdout, "\n");
    fprintf(stdout, "Options:\n");
    fprintf(stdout, "   --host=[ip4 address]    IPv4 address of host to connect to.  Default is\n");
    fprintf(stdout, "                           127.0.0.1 (localhost).\n");
    fprintf(stdout, "   --port=[port]           Port to use in connection to host.  Default is 9100.\n");
    fprintf(stdout, "   --count=[iterations]    Number of iterations to perform.\n");
    fprintf(stdout, "                           Default is 5.  0 means infinite.\n");
    fprintf(stdout, "   --delay=[ms]            Delay between operations in milliseconds.\n");
    fprintf(stdout, "   --verbose               Verbose output.\n");
    fprintf(stdout, "   --help, -h, /?          This screen.\n");
    fprintf(stdout, "\n");
    fprintf(stdout, "Examples:\n");
    fprintf(stdout, "\n");
    fprintf(stdout, "  nlport --count=100 --host=10.10.10.10 --port=13000\n");
    fprintf(stdout, "  nlport --count=0 --verbose --delay=100\n");
    return 0;
  }

  WSADATA wsaData;
  int iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
  if( iResult != NO_ERROR )
  {
    fprintf(stderr, "nlport: WSAStartup failed (WSA LastError %d)\n",WSAGetLastError());
    return 1;
  }

  std::size_t i = 0;
  for( i = 0 ; op_count == 0 || i < op_count ; i++ )
  {
    SOCKET s;
    s = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    if( s == INVALID_SOCKET )
    {
      fprintf(stderr, "nlport: socket failed (WSA LastError %d)\n", WSAGetLastError());
      return 1;
    }

    sockaddr_in clientService; 
    clientService.sin_family = AF_INET;
    clientService.sin_addr.s_addr = inet_addr(op_host.c_str());
    clientService.sin_port = htons((u_short)op_port);

    if( op_verbose == true )
    {
      fprintf(stdout, "nlport: connect to %s:%d\n", op_host.c_str(), op_port);
    }
    if( connect(s,(SOCKADDR*)&clientService,sizeof(clientService)) == SOCKET_ERROR )
    {
      fprintf(stderr, "nlport: connect failed (WSA LastError %d)\n", WSAGetLastError());
      return 1;
    }
    closesocket(s);

    Sleep(static_cast<DWORD>(op_delay));
  }
  WSACleanup();
  return 0;
}/* main */
