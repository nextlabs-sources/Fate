#include "StdAfx.h"
#include "Services.h"
#include <process.h>
#include "commonlib_helper.h"
#include "CacheMgr.h"

using namespace nextlabs;

boost::threadpool::pool CServices::m_tpworking(100);
boost::threadpool::pool CServices::m_tpservice(5);


void CServices::StartServices()
{
	//post the service thread
	m_tpservice.schedule(NamedPipeService);
	m_tpservice.schedule(NamedPipeService);
	m_tpservice.schedule(NamedPipeService);
	m_tpservice.schedule(NamedPipeService);
	m_tpservice.schedule(NamedPipeService);


	m_tpservice.wait();
}


void CServices::NamedPipeService()
{
	BOOL   fConnected = FALSE;
	HANDLE hPipe = INVALID_HANDLE_VALUE;

	for (;;)
	{
		PSECURITY_DESCRIPTOR pSD = (PSECURITY_DESCRIPTOR)LocalAlloc(LPTR,SECURITY_DESCRIPTOR_MIN_LENGTH);
		if (pSD == NULL)
		{
			return;
		}

		if (!InitializeSecurityDescriptor(pSD, SECURITY_DESCRIPTOR_REVISION))
		{
			LocalFree((HLOCAL)pSD);
			return;
		}

		//add a NULL disc. ACL to the security descriptor.
		if (!SetSecurityDescriptorDacl(pSD, TRUE, (PACL)NULL, FALSE))
		{
			LocalFree((HLOCAL)pSD);
			return;
		}

		SECURITY_ATTRIBUTES sa = { 0 };
		sa.nLength = sizeof(sa);
		sa.lpSecurityDescriptor = pSD;
		sa.bInheritHandle = TRUE;

		hPipe = CreateNamedPipe( 
			NLSTORAGE_NAMEDPIPE,             // pipe name 
			PIPE_ACCESS_DUPLEX,       // read/write access 
			PIPE_TYPE_MESSAGE |       // message type pipe 
			PIPE_READMODE_MESSAGE |   // message-read mode 
			PIPE_WAIT,                // blocking mode 
			PIPE_UNLIMITED_INSTANCES, // max. instances  
			BUFSIZE,                  // output buffer size 
			BUFSIZE,                  // input buffer size 
			0,                        // client time-out 
			&sa);                    // default security attribute 

		LocalFree((HLOCAL)pSD);

		if (hPipe == INVALID_HANDLE_VALUE) 
		{
			return;
		}

		// Wait for the client to connect; if it succeeds, 
		// the function returns a nonzero value. If the function
		// returns zero, GetLastError returns ERROR_PIPE_CONNECTED. 
		fConnected = ConnectNamedPipe(hPipe, NULL) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED); 

		if (fConnected) 
		{
			m_tpworking.schedule(boost::bind(&NamedPipeWorkThread, (void*)hPipe));
		}
		else
		{
			// The client could not connect, so close the pipe. 
			CloseHandle(hPipe); 
		}
	}
}

void CServices::NamedPipeWorkThread(void* parm)
{
	if (parm == NULL)
		return;

	HANDLE hPipe = (HANDLE)parm;

	HANDLE hHeap      = GetProcessHeap();
	char* pchRequest = (char*)HeapAlloc(hHeap, 0, BUFSIZE);

	if (pchRequest == NULL)
		return;

	//Try to get the data from client
	std::string ret;
	unsigned int nTotalLen = 0;
	for(;;)
	{
		DWORD cbBytesRead = 0;
		BOOL fSuccess = FALSE;

		fSuccess = ReadFile( 
			hPipe,        // handle to pipe 
			pchRequest,    // buffer to receive data 
			BUFSIZE, // size of buffer 
			&cbBytesRead, // number of bytes read 
			NULL);        // not overlapped I/O 

		
		if (!fSuccess || cbBytesRead == 0)
		{   
			if (GetLastError() == ERROR_BROKEN_PIPE)
			{
				_tprintf(TEXT("InstanceThread: client disconnected.\n")); 		
			}
			else
			{
				_tprintf(TEXT("InstanceThread ReadFile failed, GLE=%d.\n"), GetLastError()); 
			}
			break;
		}

		ret.append(pchRequest, cbBytesRead);

		if (nTotalLen == 0 && ret.length() >= 9)
		{
			unsigned int nKeyLen = 0;
			unsigned int nValueLen = 0;
			memcpy(&nKeyLen, ret.c_str() + 1, 4);
			memcpy(&nValueLen, ret.c_str() + 5, 4);
			//get the length of byte stream
			nTotalLen = nKeyLen + nValueLen + 9;
		}
		if(nTotalLen > 0 && ret.length() == nTotalLen)
			break;
	}


	if(nTotalLen > 0 && ret.length() == nTotalLen)
	{//get the data correctly
		boost::shared_ptr<CPackage> pack(new CPackage);
		pack->Parse((const unsigned char*)ret.c_str(), static_cast<unsigned int>(ret.length()));


		CCacheMgr* pMgr = CCacheMgr::CreateInst();

		switch (pack->GetCommand())
		{
		case CMD_CACHE_DATA:
			pMgr->CacheData(pack.get());
			break;
		case CMD_GET_DATA:
			int flag = (int)pack->GetValue().GetValue().get()[0];
			bool forcedelete = flag == 1? true: false;
			cache_value result = pMgr->GetData(pack.get(), forcedelete);

#pragma warning(push)
#pragma warning(disable:6309 6387)
			std::string sData = CPackage::Package(CMD_RESPONSE, NULL, (const unsigned char*)result.GetValue().get(), result.GetLength());
#pragma warning(pop)

			//write result to client
			int totalWrite = 0;
			unsigned int len = (unsigned int)sData.length();
			const char* data = sData.c_str();

			BOOL fSuccess = false;
			while ( len > 0 )
			{
				DWORD cbWritten = 0;
				fSuccess = WriteFile( 
					hPipe,                  // pipe handle 
					data + totalWrite,             // message 
					len <= BUFSIZE? len: BUFSIZE,              // message length 
					&cbWritten,             // bytes written 
					NULL);  

				if ( !fSuccess) 
				{
					break;
				}
				len -= cbWritten;
				totalWrite += cbWritten;
			}
			FlushFileBuffers(hPipe);
			break;
		}
		
	}

	DisconnectNamedPipe(hPipe); 
	CloseHandle(hPipe); 

	HeapFree(hHeap, 0, pchRequest);
	
}