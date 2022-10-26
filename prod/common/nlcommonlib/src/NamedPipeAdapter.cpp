#include "StdAfx.h"
#include "NamedPipeAdapter.h"
#include "commonlib_helper.h"

using namespace nextlabs;

CNamedPipeAdapter::CNamedPipeAdapter(void)
{
}

CNamedPipeAdapter::~CNamedPipeAdapter(void)
{
}

int CNamedPipeAdapter::Connect(HANDLE& h)
{
	DWORD dwError = ERR_SUCCESS;
	HANDLE hPipe = INVALID_HANDLE_VALUE; 
	int nIndex = 0;
	for(;;) 
	{ 
		if(nIndex > 500)
		{
			dwError = (DWORD)ERR_CONNECT_NLSTORAGE_FAILED;
			break;
		}

		hPipe = CreateFile( 
			NLSTORAGE_NAMEDPIPE,   // pipe name 
			GENERIC_READ |  // read and write access 
			GENERIC_WRITE, 
			0,              // no sharing 
			NULL,           // default security attributes
			OPEN_EXISTING,  // opens existing pipe 
			0,              // default attributes 
			NULL);          // no template file 

		// Break if the pipe handle is valid. 
		if (hPipe != INVALID_HANDLE_VALUE) 
		{
			h = hPipe;
			dwError = ERR_SUCCESS;
			break; 
		}

		dwError = GetLastError();

		if (dwError == ERROR_FILE_NOT_FOUND)
		{
			nIndex++;
			Sleep(10);
			continue;
		}

		// Exit if an error other than ERROR_PIPE_BUSY occurs. 
		if (ERROR_PIPE_BUSY != dwError) 
		{
			break;
		}

		// All pipe instances are busy, so wait for 20 seconds. 
		if ( ! WaitNamedPipe(NLSTORAGE_NAMEDPIPE, 20000)) 
		{  
			dwError = GetLastError();

			if (dwError == ERROR_FILE_NOT_FOUND)
			{
				nIndex++;
				Sleep(10);
				continue;
			}
			
			break;
		} 
	} 

	return dwError;
}

int CNamedPipeAdapter::Send(HANDLE hPipe, const unsigned char *data, int len)
{
	DWORD dwMode = PIPE_READMODE_MESSAGE; 
	BOOL fSuccess = SetNamedPipeHandleState( 
		hPipe,    // pipe handle 
		&dwMode,  // new pipe mode 
		NULL,     // don't set maximum bytes 
		NULL);    // don't set maximum time 
	if ( ! fSuccess) 
	{
		printf("SetNamedPipeHandleState failed\n");
		return GetLastError();
	}

	int totalWrite = 0;


	while ( len > 0 )
	{
		DWORD cbWritten = 0;
		fSuccess = WriteFile( 
			hPipe,                  // pipe handle 
			data + totalWrite,             // message 
			len <= BUFSIZE? len: BUFSIZE,              // message length 
			&cbWritten,             // bytes written 
			NULL);  

		if ( ! fSuccess) 
		{
			DWORD dwErr = GetLastError();
			printf("WriteFile failed, error: %d\n", dwErr);
			return dwErr;
		}
		len -= cbWritten;
		totalWrite += cbWritten;
	}

	return ERR_SUCCESS;
}

int CNamedPipeAdapter::SendData(const unsigned char *data, int len)
{
	HANDLE hPipe = INVALID_HANDLE_VALUE;
	
	int err = Connect(hPipe);

	if (hPipe == INVALID_HANDLE_VALUE || err != ERR_SUCCESS)
	{
		return err;
	}

	Send(hPipe, data, len);

	CloseHandle(hPipe);

	return ERR_SUCCESS;
}

int CNamedPipeAdapter::GetData(const nextlabs::cache_key *_key, bool forcedelete, _Out_ nextlabs::cache_value* _value)
{
	HANDLE hPipe = INVALID_HANDLE_VALUE;
	
	int err = Connect(hPipe);

	if (hPipe == INVALID_HANDLE_VALUE || err != ERR_SUCCESS)
	{
		printf("connect failed, error: %d\n", err);
		return err;
	}
	
	char cDelete;
	if (forcedelete)
		cDelete = 1;
	else
		cDelete = 0;
	


	std::string sData = CPackage::Package(CMD_GET_DATA, _key, (const unsigned char*)&cDelete, 1);
	
	std::string::size_type len = sData.length();
	const char* data = sData.c_str();

	if(Send(hPipe, (const unsigned char*)data, static_cast<int>(len)) != ERR_SUCCESS)
		return ERR_CONNECT_NLSTORAGE_FAILED;


	//read the response
	HANDLE hHeap      = GetProcessHeap();
	char* pchRequest = (char*)HeapAlloc(hHeap, 0, BUFSIZE);

	if (pchRequest == NULL)
		return ERR_EMPTYPOINTER;

	std::string result;
	unsigned int nTotal = 0;
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

		if ( !fSuccess && GetLastError() != ERROR_MORE_DATA )
			break;

		result.append(pchRequest, cbBytesRead);

		if (nTotal == 0 && result.length() >= 9)
		{
			unsigned int nKeyLen = 0;
			unsigned int nValueLen = 0;
			memcpy(&nKeyLen, result.c_str() + 1, 4);
			memcpy(&nValueLen, result.c_str() + 5, 4);
			//get the length of byte stream
			nTotal = nKeyLen + nValueLen + 9;
		}
		if (nTotal > 0 && result.length() == nTotal)
		{
			break;
		}
	}

	CloseHandle(hPipe);
	HeapFree(hHeap, 0, pchRequest);

	if (nTotal > 0 && result.length() == nTotal)
	{
		CPackage pack;
		pack.Parse((const unsigned char*)result.c_str(), static_cast<unsigned int>(result.length()));

		*_value = pack.GetValue();

		return ERR_SUCCESS;
	}

	return ERR_RECEIVE_FAILED;
}

