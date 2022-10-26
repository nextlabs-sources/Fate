#pragma once

typedef pair<string, string> PATH_CONTENT;
typedef pair<PATH_CONTENT,DWORD>PATH_TIME ; 
typedef list<PATH_TIME> PATH_CONTENT_LIST;

typedef pair<HANDLE, string> HANDLE_NAME;
typedef list<HANDLE_NAME> HANDLE_NAME_LIST;

typedef pair<SOCKET, string> SOCKET_BUF;
typedef pair<SOCKET_BUF, FTP_EVAL_INFO> SOCKET_BUF_EVAL;
typedef list<SOCKET_BUF_EVAL> SOCKET_BUF_EVAL_LIST;

typedef pair<DWORD, string> THREAD_CONTENT;
typedef list<THREAD_CONTENT> THREAD_CONTENT_LIST;

typedef pair<DWORD, SOCKET> THREAD_SOCKET;
typedef list<THREAD_SOCKET> THREAD_SOCKET_LIST;

typedef pair<DWORD, FTP_EVAL_INFO> CONHANDLE_EVAL;
typedef list<CONHANDLE_EVAL> CONHANDLE_EVAL_LIST;

class CMapperMgr
{
public:
	static const unsigned long MAX_CONTENT_SIZE = 4096;
	static const unsigned long MAX_TIME_OUT	= 120000 ;

	static CMapperMgr& Instance();
	void PushFileInfoToList(const string& path, const string& content);
	string PopFileInfoFromList(const string& content);

	FTP_EVAL_INFO PopSocketBufEval4Explorer(const string& sBuf) ;
	FTP_EVAL_INFO PopSocketBufEval4RecvEval(const SOCKET& s) ;
	BOOL RemoveSocketBufEval4RecvEval(const SOCKET& s) ;

	void PushHandleName(HANDLE hFile, const string& sName);
	string PopHandleName(HANDLE hFile);

	void PushSocketBufEval(SOCKET s, const string& sBuf, const FTP_EVAL_INFO & evalInfo);
	FTP_EVAL_INFO PopSocketBufEval(const string& sBuf);

	void PushThreadContent(DWORD dwThread, const string& sContent);
	string PopThreadContent(DWORD dwThread);

	void PushThreadSocket(DWORD dwThread, SOCKET s);
	SOCKET PopThreadSocket(DWORD dwThread);

	void AddConhandleEval(DWORD dwHandle, const FTP_EVAL_INFO & evalInfo);
	FTP_EVAL_INFO GetEvalByConhandle(DWORD dwHandle);
	void RemoveConhandleEval(DWORD dwHandle);

	void PushDelHandleName(HANDLE hFile, const string& sName);
	string PopDelHandleName(HANDLE hFile);


private:
	CMapperMgr(void);
	CMapperMgr(const CMapperMgr&);
	void operator = (const CMapperMgr&);
	~CMapperMgr(void);

private:
	PATH_CONTENT_LIST    m_listPathContent;
	HANDLE_NAME_LIST     m_listHandleName;
	SOCKET_BUF_EVAL_LIST m_listSocketBufEval;

	THREAD_CONTENT_LIST m_listThreadContent;
	THREAD_SOCKET_LIST  m_listThreadSocket;

	CONHANDLE_EVAL_LIST m_listConhandleEval;

	HANDLE_NAME_LIST    m_listDelHandleName;
};
