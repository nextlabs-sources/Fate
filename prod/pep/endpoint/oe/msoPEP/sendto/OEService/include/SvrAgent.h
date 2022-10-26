#pragma  once
#include "stdafx.h"

#include <atltime.h>
#include <vector>
#include <list>

#include "RegEdit.h"

#define SETFILESMSG	0x101
#define GETFILESMSG	0x102

#define PARAMBIN	0x01
#define PARAMSTR	0x02

// Define this space time for check the file-map list
#define VERIFY_TIME	1800000

typedef std::pair<std::wstring, std::wstring>   FilePair;
typedef struct _keywords
{
	std::wstring strSubject;
	unsigned int uAttachmentCount;
	std::vector<FilePair>	vecFiles;
}KeyWords;
typedef std::pair<unsigned long,KeyWords>	NodePair;


// add by tonny for common user can access to registry at 6/11
class COEService;

// add by Tonny for merge log.cpp into this file
//////////////////////////////////////////////////////////////////////////

class Mutex 
{
	friend class MUTEX;
public:
	Mutex()
	{
		InitializeCriticalSection(&cs);
	}
	~Mutex()
	{
		DeleteCriticalSection(&cs);
	}
private:
	void lock()
	{
		EnterCriticalSection(&cs);
	}
	void unlock()
	{
		LeaveCriticalSection(&cs);
	}
private:
	Mutex(const Mutex& mutex);
	void operator=(const Mutex& mutex);
private:	
	CRITICAL_SECTION cs;
};


class MUTEX
{
public:
	MUTEX(Mutex *mutex):_mutex(mutex)
	{
		_mutex->lock();
	}
	~MUTEX()
	{
		_mutex->unlock();
	}
private:
	Mutex* _mutex;
};

//////////////////////////////////////////////////////////////////////////
// parse the transfer protocol between CE and service  
class CPacket
{
	//friend class COEService;
	// at 6/30 modified by Tonny
	// changed from private to public in order to static function can invoke it
public:
	CPacket(void)
		:m_hStopEvent(INVALID_HANDLE_VALUE),
		m_hThread(INVALID_HANDLE_VALUE)
	{}
	~CPacket(void)
	{
		ClearList();
	}
public:
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

		if(pNode.uAttachmentCount==0)	return 0;

		// file pair vector
		nIndex += 5;
		std::wstring strName,strPath;
		while(true)
		{
			strName = (wchar_t*)(szBuf+nIndex);
			nIndex += (UINT)strName.length()*sizeof(wchar_t);
			nIndex +=2;
			strPath = (wchar_t*)(szBuf+nIndex);
			nIndex += (UINT)strPath.length()*sizeof(wchar_t);
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
		int nParamLen = (INT)(pNode.strSubject.length()+1)*2;
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
			nstrlen = (UINT)files.first.length()*2;
			memcpy(pszBuf + nIndex,files.first.c_str(),nstrlen);
			nIndex += nstrlen;
			memcpy(pszBuf+nIndex,&end,2);
			nIndex += 2;
			nstrlen =(DWORD) files.second.length()*2;
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

	bool ClearList()
	{
		MUTEX _mutex(&m_hMutex);
		m_listNode.clear();
		return true;
	}

	void InsertANode(const KeyWords& pInNode)	// add curtime
	{
        // We should only output debug information in debug mode
#ifdef DEBUG	// output log
		wchar_t* strInfo = new wchar_t[BUF_LEN];
        if(NULL != strInfo)
        {
            memset(strInfo, 0, sizeof(strInfo));
            _snwprintf_s(strInfo,BUF_LEN, _TRUNCATE,L"Subject is:%ws\tAttachment count is:%d\r\nAttachments is:\r\n",pInNode.strSubject.c_str(),
                pInNode.uAttachmentCount);
            for(size_t i=0;i < pInNode.vecFiles.size();i++)
            {
                wcsncat_s(strInfo,BUF_LEN,L"\tFile name is:\t", _TRUNCATE);
                wcsncat_s(strInfo,BUF_LEN,pInNode.vecFiles[i].first.c_str(), _TRUNCATE);
                wcsncat_s(strInfo,BUF_LEN,L"\r\n", _TRUNCATE);
            }
           // OutputDebugStringW(L"Insert Node!\r\n\t");
           // OutputDebugStringW(strInfo);
		    //CLog::WriteLog(L"Insert Node",strInfo);
            delete []strInfo; strInfo=NULL;
        }
#endif
		MUTEX theMutex(&m_hMutex);
		size_t nsize = m_listNode.size();
		if(nsize >= 1000)
			m_listNode.pop_front();
		m_listNode.push_back(NodePair(GetTickCount(),pInNode));
	}
	// find a node and remove the node from list
	bool GetANode(KeyWords& pInOutNode)
	{
		MUTEX theMutex(&m_hMutex);
		std::list<NodePair>::iterator iter = m_listNode.begin();
		for(;iter != m_listNode.end();iter++)
		{
			bool bFind = true;
			const KeyWords& theNode = (*iter).second;

			std::vector<FilePair>::const_iterator iterVec = theNode.vecFiles.begin(),
				oldIter = pInOutNode.vecFiles.begin();

			if(pInOutNode.uAttachmentCount == theNode.uAttachmentCount &&
				_wcsicmp(pInOutNode.strSubject.c_str(),theNode.strSubject.c_str()) == 0 &&
				pInOutNode.vecFiles.size() == theNode.vecFiles.size())
			{
				int nsize = (INT)pInOutNode.vecFiles.size();
				for(int i=0;i<nsize;i++)
				{
					if(_wcsicmp(pInOutNode.vecFiles[i].first.c_str(),theNode.vecFiles[nsize-i-1].first.c_str()) !=0)
					{
						bFind=false;	
						break;
					}
				}
			}
			else
				bFind = false;
			if(bFind)
			{
				int nsize =(INT) pInOutNode.vecFiles.size();
				pInOutNode.vecFiles.clear();
				for(int j=nsize-1;j>=0;j--)
					pInOutNode.vecFiles.push_back(theNode.vecFiles[j]) ;

 				m_listNode.erase(iter);
				return true;
			}
		}

		return false;
	}
public:
	// list header
	//KeyWords*	m_pHead;
	std::list<NodePair>	m_listNode;
	// signal stop event
	HANDLE	m_hStopEvent;
	// mutex for access list 
	Mutex	m_hMutex;
	// process list time out thread handler
	HANDLE	m_hThread;
};


//////////////////////////////////////////////////////////////////////////
class CSvrSocket
{
	//friend class COEService;
	// changed for static function can call
public:
	CSvrSocket(void):m_hSocket( INVALID_SOCKET),m_nPort(0){}
	~CSvrSocket(void){	CloseSocket();	}
private:
	SOCKET	m_hSocket;
	SOCKADDR_IN m_scClient;
	unsigned int m_nPort;
public:
	SOCKET	GetSocket()	const
	{
		return m_hSocket;
	}
	unsigned int RecvData(char* szBuf,unsigned long lBufLen)
	{
		int nSockLen=sizeof(m_scClient);
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
			//server.sin_port = INADDR_ANY;//htons(nPort);
			server.sin_port = htons(LOWORD(nPort));

			if(bind(m_hSocket,(struct sockaddr*) &server,nLen)   ==   SOCKET_ERROR)	return false;
			nLen = sizeof(server);
			if(getsockname(m_hSocket,(struct sockaddr *)&server, &nLen) != SOCKET_ERROR) 
			{
				m_nPort = ntohs(server.sin_port);
				CRegEdit::WriteServicePort(m_nPort);
			}
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
/*
class CTransferInfo
{
public:
	static bool put_FileInfo(const wchar_t* strSubject,const unsigned int uCount,const std::vector<FilePair>& vecFilePair)
	{
		CSvrSocket theSocket;
		theSocket.StartSocket(false);
		KeyWords theNode;
		theNode.pNext=NULL;
		theNode.strSubject=strSubject;
		theNode.uAttachmentCount = uCount;
		for(int i=0;i<vecFilePair.size();i++)
		{
			theNode.vecFiles.push_back(vecFilePair[i]);
		}
		CPacket thePacket;
		char szBuf[BUF_LEN]={0};
		unsigned long uLen=BUF_LEN;
		bool bTrue = thePacket.GetBinFromKeyWords(SETFILESMSG,theNode,szBuf,uLen);
		if(bTrue)
		{
			SOCKADDR_IN sockRemote;
			ZeroMemory(&sockRemote,sizeof(sockRemote));
			sockRemote.sin_addr.s_addr = inet_addr(LOCAL_IP);
			sockRemote.sin_port = htons(LOCAL_PORT);
			sockRemote.sin_family = AF_INET;
			int nRecv = theSocket.SendData(szBuf,uLen,(SOCKADDR*)&sockRemote);
			if(nRecv == uLen)	return true;
		}
		return false;
	}
	static bool get_FileInfo(const wchar_t* strSubject,const unsigned int uCount,std::vector<FilePair>& vecFilePair)
	{
		CSvrSocket theSocket;
		theSocket.StartSocket(false);
		KeyWords theNode;
		theNode.strSubject=strSubject;
		theNode.uAttachmentCount = uCount;
		for(int i=0;i<vecFilePair.size();i++)
		{
			theNode.vecFiles.push_back(vecFilePair[i]);
		}
		CPacket thePacket;
		char szBuf[BUF_LEN];
		unsigned long uLen=BUF_LEN;
		bool bTrue = thePacket.GetBinFromKeyWords(GETFILESMSG,theNode,szBuf,uLen);
		if(bTrue)
		{
			SOCKADDR_IN sockRemote;
			ZeroMemory(&sockRemote,sizeof(sockRemote));
			sockRemote.sin_addr.s_addr = inet_addr(LOCAL_IP);
			sockRemote.sin_port = htons(LOCAL_PORT);
			sockRemote.sin_family = AF_INET;
			int nSend = theSocket.SendData(szBuf,uLen,(SOCKADDR*)&sockRemote);
			if(nSend != uLen)	return false;
			int nRecv = theSocket.RecvData(szBuf,BUF_LEN);
			theNode.vecFiles.clear();
			bTrue = thePacket.GetKeyWordFromBin(szBuf,nRecv,theNode);
			if(bTrue)
			{
				vecFilePair.clear();
				for(int i=0;i<theNode.vecFiles.size();i++)
				{
					vecFilePair.push_back(theNode.vecFiles[i]);
				}
				return true;
			}
		}
		return false;
	}
};
*/