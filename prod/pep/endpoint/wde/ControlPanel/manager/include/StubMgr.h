#pragma once

class IPCStub;
class IIPCRequestHandler;

class CStubMgr
{
private:
	CStubMgr(void);
	CStubMgr(const CStubMgr&);
	~CStubMgr(void);
	void operator = (const CStubMgr&);
	
private:
	IPCStub* m_pStub;
	IIPCRequestHandler* m_pRequestHandler;
	HANDLE m_hStubThread;

	static DWORD WINAPI my_ThreadProc(LPVOID lpStub);

public:
	static CStubMgr& GetInstance();

	/*
	
	start stub, after started, stub will monitor share memory and call requestHandler::Invoke etc when ipcproxy trigger request to requestHandler.
	
	*/
	BOOL StartStub();

	/*
	
	stop stub.
	
	*/
	BOOL StopStub();
};
