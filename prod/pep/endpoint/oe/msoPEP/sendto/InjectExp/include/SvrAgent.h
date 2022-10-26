#pragma once
#include <vector>

#include "RegEdit.h"

#define SETFILESMSG	0x101
#define GETFILESMSG	0x102

#define PARAMBIN	0x01
#define PARAMSTR	0x02

#define BUF_LEN		8192

typedef std::pair<std::wstring, std::wstring>   FilePair;
typedef struct _keywords
{
	std::wstring strSubject;
	unsigned int uAttachmentCount;
	std::vector<FilePair>	vecFiles;
}KeyWords;

class COEService;
class CPacket
{
	friend class CTransferInfo;
private:
	CPacket(void){}
	~CPacket(void){}
private:
	unsigned short GetKeyWordFromBin(const char* szBuf,const unsigned int uLen,KeyWords& pNode)
	{
		DWORD dwLen =0;
		unsigned int nIndex=0;
		memcpy(&dwLen,szBuf,4);
		if(dwLen != uLen)	return 0;
		nIndex += 4;

		// get msg type
		unsigned short uMstType=0;
		memcpy(&uMstType,szBuf+nIndex,2);
		nIndex +=2;

		// the reserve byte
		nIndex ++;

		// get parameters number
		unsigned int nParameters = szBuf[nIndex++];
		if(nParameters != 3)	return 0;

		//get first parameter
		int nType = PARAMBIN;
		if(szBuf[nIndex++] == PARAMSTR)	nType = PARAMSTR;

		UINT	uParaLen=0;
		memcpy(&uParaLen,szBuf+nIndex,4);
		nIndex += 4;
		pNode.strSubject = (wchar_t*)(szBuf+nIndex);
		nIndex +=uParaLen;

		nIndex += 5;
		memcpy(&pNode.uAttachmentCount,szBuf+nIndex,4);
		nIndex += 4;

		// file pair vector
		nIndex += 5;
		std::wstring strName,strPath;
		while(true)
		{
			strName = (wchar_t*)(szBuf+nIndex);
			nIndex += (int)strName.length()*sizeof(wchar_t);
			nIndex +=2;
			strPath = (wchar_t*)(szBuf+nIndex);
			nIndex += (int)strPath.length()*sizeof(wchar_t);
			nIndex +=2;
			pNode.vecFiles.push_back(FilePair(strName,strPath));
			if(nIndex == uLen)	break;
		}
		return uMstType;
	}

	// create binary be send according to the KeyWords pNode
	bool GetBinFromKeyWords(unsigned short nMsgType,const KeyWords& pNode,char* pszBuf,unsigned long& uPackLen)
	{
		unsigned int nLen = uPackLen;
		memset(pszBuf,0,nLen);
		wchar_t end = L'\0';

		unsigned int nIndex = 4;

		memcpy(pszBuf+nIndex,&nMsgType,sizeof(nMsgType));
		nIndex += 2;

		pszBuf[nIndex++]=0;
		pszBuf[nIndex++]=3;	// there parameters

		// add parameter one
		pszBuf[nIndex++]=PARAMSTR;
		int nParamLen = ((int)pNode.strSubject.length()+1)*2;
		memcpy(pszBuf+nIndex,&nParamLen,4);
		nIndex += 4;

		memcpy(pszBuf+nIndex,pNode.strSubject.c_str(),nParamLen-2);
		nIndex += nParamLen-2;
		memcpy(pszBuf+nIndex,&end,2);
		nIndex += 2;

		// parameter second
		pszBuf[nIndex++]=PARAMBIN;
		nParamLen = 4;
		memcpy(pszBuf+nIndex,&nParamLen,4);
		nIndex += 4;
		memcpy(pszBuf+nIndex,&pNode.uAttachmentCount,4);
		nIndex += 4;

		// third parameter
		pszBuf[nIndex++]=PARAMBIN;
		// record the start pos of the third parameter
		int nLenStart = nIndex; 
		nIndex += 4;

		std::vector<FilePair>::const_iterator iter = pNode.vecFiles.begin();

		unsigned int nstrlen=0;
		for(;iter != pNode.vecFiles.end();iter ++)
		{
			FilePair files = (*iter);
			nstrlen = (unsigned int)files.first.length()*2;
			memcpy(pszBuf + nIndex,files.first.c_str(),nstrlen);
			nIndex += nstrlen;
			memcpy(pszBuf+nIndex,&end,2);
			nIndex += 2;
			nstrlen = (unsigned int)files.second.length()*2;
			memcpy(pszBuf+nIndex,files.second.c_str(),nstrlen);
			nIndex += nstrlen;
			memcpy(pszBuf+nIndex,&end,2);
			nIndex += 2;
		}
		uPackLen = nIndex;
		memcpy(pszBuf,&nIndex,4);
		nIndex = nIndex-nLenStart-4;
		memcpy(pszBuf+nLenStart,&nIndex,4);
		return true;
	}
};


//////////////////////////////////////////////////////////////////////////
class CSvrSocket
{
	friend class CTransferInfo;
private:
	CSvrSocket(void):m_hSocket( INVALID_SOCKET){}
	~CSvrSocket(void){	CloseSocket();	}
private:
	SOCKET	m_hSocket;
	SOCKADDR_IN m_scClient;
private:
	SOCKET	GetSocket()	const
	{
		return m_hSocket;
	}
	unsigned int RecvData(char* szBuf,unsigned long lBufLen)
	{
		int nSockLen=sizeof(m_scClient);
		int nTimeOut=2000;//设置接收超时6秒
		if(::setsockopt(m_hSocket,SOL_SOCKET,SO_RCVTIMEO,(char *)&nTimeOut,sizeof(nTimeOut))==SOCKET_ERROR)
            return (unsigned int)SOCKET_ERROR;
		unsigned int nRecv = recvfrom(m_hSocket,szBuf,lBufLen,NULL,(SOCKADDR*)&m_scClient,&nSockLen);
		return nRecv;
	}
	unsigned int SendData(const char* szBuf,const unsigned int nLen,SOCKADDR* pAddr=NULL)
	{
		int nSockLen = sizeof(SOCKADDR_IN);
		unsigned int nSend = 0;
		if(pAddr == NULL)
			nSend = sendto(m_hSocket,szBuf,nLen,NULL,(SOCKADDR*)&m_scClient,nSockLen);
		else
			nSend = sendto(m_hSocket,szBuf,nLen,NULL,pAddr,nSockLen);
		return nSend;
	}
	bool StartSocket(bool bSvr = true,const char* strIP=LOCAL_IP,const unsigned int nPort=LOCAL_PORT)
	{
		SOCKADDR_IN server;
		int nLen = sizeof(server);

		WSADATA   wsda; 
		if(0 != WSAStartup(MAKEWORD(2,1),   &wsda))
		{
			return false;
		}
		m_hSocket = socket(AF_INET,SOCK_DGRAM,NULL);

		if(bSvr)
		{
			ZeroMemory(&server,nLen);
			server.sin_family = AF_INET;
			server.sin_addr.s_addr = inet_addr(strIP);
			server.sin_port = htons(LOWORD(nPort));

			if(bind(m_hSocket,(struct sockaddr*) &server,sizeof(server))   ==   SOCKET_ERROR)	return false;
		}
		return true;
	}
	bool CloseSocket()
	{
		if(m_hSocket != INVALID_SOCKET)
		{
			shutdown(m_hSocket,SD_BOTH);
			closesocket(m_hSocket);
			m_hSocket = INVALID_SOCKET;
		}
		return true;
	}
};
//////////////////////////////////////////////////////////////////////////


class CTransferInfo
{
private:
	static DWORD m_uPort;
	static void   GetPort()
	{
		if(m_uPort == 0)
			CRegEdit::ReadServicePort(m_uPort);
	}
public:
	static bool put_FileInfo(const wchar_t* strSubject,const unsigned int uCount,const std::vector<FilePair>& vecFilePair)
	{
		GetPort();
		CSvrSocket theSocket;
		theSocket.StartSocket(false);
		KeyWords theNode;
		theNode.strSubject=strSubject;
		theNode.uAttachmentCount = uCount;
		for(unsigned int i=0;i<vecFilePair.size();i++)
			theNode.vecFiles.push_back(vecFilePair[i]);

		CPacket thePacket;
		char szBuf[BUF_LEN]={0};
		unsigned long uLen=BUF_LEN;
		bool bTrue = thePacket.GetBinFromKeyWords(SETFILESMSG,theNode,szBuf,uLen);
		if(bTrue)
		{
			SOCKADDR_IN sockRemote;
			ZeroMemory(&sockRemote,sizeof(sockRemote));
			sockRemote.sin_addr.s_addr = inet_addr(LOCAL_IP);
			sockRemote.sin_port = htons((u_short)m_uPort);
			sockRemote.sin_family = AF_INET;
			int nRecv = theSocket.SendData(szBuf,uLen,(SOCKADDR*)&sockRemote);
			if(nRecv == (int)uLen)	return true;
		}
		return false;
	}
	static bool get_FileInfo(const wchar_t* strSubject,const unsigned int uCount,std::vector<FilePair>& vecFilePair)
	{
		GetPort();
		CSvrSocket theSocket;
		theSocket.StartSocket(false);
		KeyWords theNode;
		theNode.strSubject=strSubject;
		theNode.uAttachmentCount = uCount;
		for(unsigned int i=0;i<vecFilePair.size();i++)
			theNode.vecFiles.push_back(vecFilePair[i]);

		CPacket thePacket;
		char szBuf[BUF_LEN];
		unsigned long uLen=BUF_LEN;
		bool bTrue = thePacket.GetBinFromKeyWords(GETFILESMSG,theNode,szBuf,uLen);
		if(bTrue)
		{
			SOCKADDR_IN sockRemote;
			ZeroMemory(&sockRemote,sizeof(sockRemote));
			sockRemote.sin_addr.s_addr = inet_addr(LOCAL_IP);

			sockRemote.sin_port = htons((u_short)m_uPort);
			sockRemote.sin_family = AF_INET;
			int nSend = theSocket.SendData(szBuf,uLen,(SOCKADDR*)&sockRemote);
			if(nSend != (int)uLen)	return false;
			int nRecv = theSocket.RecvData(szBuf,BUF_LEN);
			if(nRecv == SOCKET_ERROR)	return false;
			theNode.vecFiles.clear();
			unsigned short uMsg = thePacket.GetKeyWordFromBin(szBuf,nRecv,theNode);
			if(uMsg > 0)
			{
				vecFilePair.clear();
				for(unsigned int i=0;i<theNode.vecFiles.size();i++)
					vecFilePair.push_back(theNode.vecFiles[i]);
				return true;
			}
		}
		return false;
	}
};
DWORD CTransferInfo::m_uPort = 0;