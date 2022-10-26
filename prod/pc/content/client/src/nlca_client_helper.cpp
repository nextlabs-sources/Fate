/*============================nlca_client.cpp===============================*
 *                                                                          *
 * All sources, binaries and HTML pages (C) copyright 2008 by NextLabs,     *
 * San Mateo CA, Ownership remains with NextLabs Inc,                       * 
 * All rights reserved worldwide.                                           *
 *                                                                          * 
 * Author : Heidi Zhou Nolan                                                *
 * Date   : 10/28/2008                                                      *
 * Note   : NLCA Client implementation.                                     *
 *==========================================================================*/
#include "nlca_client_helper.hpp"
#include "nlca_client.h"
#include "nlca_framework_network.hpp"

using namespace NLCA;

void NLCAClient::ConstructCARPCPacket(const wchar_t* file ,
				       int pid ,
				       std::list<NLCA::Expression *> &exps,
				       ContentAnalysisRPC &caRPC) 
{
  NLCA::Content content(file, NLCA::Content::FILE);
  std::list<NLCA::SerializableExpression> sExps;

  //Construct serializable expression list
  std::list<NLCA::Expression *>::iterator it;
  std::list<NLCA::Expression *>::const_iterator eit;
  it = exps.begin();
  eit= exps.end();
  for(; it != eit; ++it ) {
    NLCA::SerializableExpression e(*(*it));
    sExps.push_back(e);
  }

  NLCA::ContentAnalysis ca(content, sExps);
  caRPC.SetContent(ca);
  caRPC.SetType(ContentAnalysisRPC::DoContentAnalysis);
  caRPC.SetPID(pid);
}

bool NLCAClient::SendRequest(ContentAnalysisRPC &rpc)
{
  /* serialize data */
  std::string serialized; //result of serialization
  unsigned long bytes, nbytes;
  NLCAService::Serialization(rpc, serialized, bytes);

  if(bytes == 0)
    return false; //no request

  //Sending RPC packet
  int bytes_sent;
  nbytes = htonl(bytes);             // byte order for network
  //printf("payload (%d %d): %s--\n", bytes, nbytes, serialized.c_str());
  bytes_sent = send(s,(char*)&nbytes,sizeof(nbytes),0); //Send payload size
  if( bytes_sent != sizeof(nbytes) ){
    return false;
  }
  bytes_sent = send(s,(char*)serialized.c_str(),bytes,0); // Send payload
  if( static_cast<unsigned long>(bytes_sent) != bytes ) {
    return false;
  }

  return true;
}

bool NLCAClient::ReadingAnalysisResult(std::list<Expression *> &outExps,
					timeval* tv ) 
{
  /***********************************************************************
   * Honor timeout using select.  The socket should become readable
   * within the given timeout, otherwise the read back is considered
   * to be failed.
   **********************************************************************/
   fd_set rfds;
   FD_ZERO(&rfds);
   FD_SET(s,&rfds);
   int nfds = select(0,&rfds,NULL,NULL,tv);
   if( nfds < 1 ) {
     printf("timed out\n");
     return false;
   }

   /***********************************************************************
    * Read the payload size
    **********************************************************************/
   int payload_size;
   int bytes_read = recv(s, (char*)&payload_size, sizeof(payload_size),0);
   /* Handle: (1) error, (2) graceful disconnect,(3) invalid payload size */
   if( bytes_read <= 0 || bytes_read < sizeof(payload_size) ) {
     //TRACE(CELOG_ERR,_T("NLCAService Client: Disconnected [%d]\n"), s);
     return false;
   }
   payload_size = ntohl(payload_size);
   //fwprintf(stdout, L"NLCAService Client: payload_size = %d\n", 
   //payload_size);
   assert( payload_size > 0 && payload_size < (32 * 1024) ); /* (0,32KB) */
   /***********************************************************************
    * Read payload
    **********************************************************************/
   char* buf = new (std::nothrow) char [payload_size+1];
   if( buf != NULL ) {
     bytes_read = nl_recv_sync(s,(char*)buf,payload_size,0,1000); // 1000ms timeout

     //fprintf(stdout, "NLCAService Client: %s\n", buf);
     /*********************************************************************
      * Deserialize payload
      ********************************************************************/
     ContentAnalysisRPC caRPC;
     bool bDeserial=NLCAService::Deserialization(buf, payload_size, caRPC); 
     delete [] buf;	
     
     if(bDeserial) {
       //Get Expressions Match Result
       NLCA::ContentAnalysis &ca=caRPC.GetContentAnalysis();

       std::list<NLCA::Expression *>::iterator oit=outExps.begin();
       std::list<NLCA::Expression *>::iterator eoit=outExps.end();
       std::list<NLCA::SerializableExpression>::iterator it;
       std::list<NLCA::SerializableExpression>::const_iterator eit;
       it = ca.GetExpressions().begin();
       eit= ca.GetExpressions().end();
       for(; oit != eoit; ++oit) {
	 for(; it != eit; ++it) {
	   if((*oit)->GetType() == (*it).GetType()) {
	     const std::wstring &exp1=(*oit)->RegexString();
	     const std::wstring &exp2=(*it).RegexString();
	     if(exp1 == exp2) {
	       if((*oit)->RequiredMatchCount() == (*it).RequiredMatchCount()) {
		 //Find the expression; assign the result
		 (*oit)->SetMatch((*it).MatchCount());
		 break;
	       }//RequiredMatchCount ==
	     }//RegexString ==	      
	   }//ExpType ==
	 }//for(; it != eit; ++it)
	 it = ca.GetExpressions().begin();
       }//for(; oit != eoit; ++oit) 
       return true;
     }//if(bDeserial)
   }//if( buf != NULL ) 

   return false;
}


void NLCAClient::Disconnect(void)
{
  closesocket(s);
  WSACleanup();
  s = INVALID_SOCKET;
}

bool NLCAClient::Connect(void)
{
  WSADATA wsaData;
  if( WSAStartup(MAKEWORD(2,2),&wsaData) != NO_ERROR ) {
    return false;
  }
  s = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
  if( s == INVALID_SOCKET ) {
    WSACleanup();
    return false;
  }

  /********************************************************************
   * Connect to nlca service on loopback
   *******************************************************************/
  sockaddr_in cserv; 
  cserv.sin_family = AF_INET;
  cserv.sin_addr.s_addr = inet_addr("127.0.0.1");
  cserv.sin_port = htons(NLCAService::SERVICE_PORT);
  if( connect(s,(SOCKADDR*)&cserv,sizeof(cserv)) == SOCKET_ERROR ) {
    closesocket(s);
    WSACleanup();
    s = INVALID_SOCKET;
    return false;
  }
  return true;
}

bool NLCAClient::SearchFile(const wchar_t* file ,
			     int pid ,
			     std::list<NLCA::Expression *> &expression_list , 
			     int timeout_ms)
{
  assert( file != NULL );
  if( Connect() == false ) {
    return false;
  }

  /*Construct RPC packet*/
  ContentAnalysisRPC caRPC;
  ConstructCARPCPacket(file, pid, expression_list, caRPC);
      
  timeval tv = {0};
  bool result = false;
  if(SendRequest(caRPC)==false){
    goto AnalyzeFile_complete;
  }
      
  /* read response */
  //TRACE(CELOG_DEBUG,_T("waiting for result\n"));
  timeval* ptv = &tv;

  if( timeout_ms > 0) {
    tv.tv_sec  = timeout_ms / 1000;           // seconds
    tv.tv_usec = (timeout_ms % 1000) * 1000;  // microseconds
  }
  result=ReadingAnalysisResult(expression_list,ptv);

 AnalyzeFile_complete:
  Disconnect();
  return result;
}/* SearchFile */

bool NLCAClient::Stop(void)
{
  if( Connect() == false ) {
    return false;
  }

  /* send stop command */
  ContentAnalysisRPC caRPC(ContentAnalysisRPC::StopService);
      
  if(SendRequest(caRPC)==false) {
    goto stop_complete;
  }

  /* read response?? */

stop_complete:
  Disconnect();
  return true;
}/* Stop */


bool NLCAService_SearchFile(const wchar_t* file ,
			    int pid ,
			    std::list<NLCA::Expression *> &exps, 
			    int timeout_ms)
{
  NLCAClient nlca;
  return nlca.SearchFile(file, pid, exps, timeout_ms);
}

bool NLCAService_Stop(void)
{
  NLCAClient nlca;
  return nlca.Stop();
}

