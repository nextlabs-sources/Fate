#include "stdafx.h"
#include "NLOfficePEP_Comm.h"
#include "NLObMgr.h"
#include "NLHookAPI.h"
#include "OfficeListener.h"
#include "dllmain.h"

// Boost
// better NOT to move asio.hpp into stdafx.h
#pragma warning( push )
#pragma warning( disable: 4244 4267 4512 4996 6011 6031 6258 6386 6385 6328 6309 6387 6334  )
#include <boost/asio.hpp>
#pragma warning( pop )

using namespace boost;
using namespace boost::asio;

#include "TalkWithSCE.h"


namespace
{
	const char* SCEServerIP = "127.0.0.1";
	const USHORT SCEServerBasedPort = 20000;

	const WCHAR* NamedPipePrefix = L"\\\\.\\pipe\\";
	const WCHAR* NamedPipeFlag = L"OfficeScreenCapture";
	const char* charNamedPipeFlag = "OfficeScreenCapture";
	const int NamedPipeBufferSize = 512;

	const char* RegisterHead = "register:";
	const char* RegisterType = "type=namedpipe";
	const char* RegisterName = "name=";
	const char* RegisterPID = "pid=";
	const char* RegisterSeparator = ";";
	const int RegisterLength = 64;

	const char* RegisterSuccessful = "register=successful";
	const int RegisterResultLength = 32;

	const char* UnregisterHead = "unregister:";
	const int UnregisterLength = 64;

	const char* UnregisterSuccessful = "unregister=successful";
	const char* UnregisterFailed = "unregister=failed";
	const int UnregisterResultLength = 32;

	const char* QueryAllow = "query=allow";
	const char* QueryDeny = "query=deny;displaytext=";
	const int QueryResultLength = 512;
}

TalkWithSCE TalkWithSCE::sm_ThisInstance;

void TalkWithSCE::StartServerThread()
{
	std::wstring PipeName = NamedPipePrefix;
	PipeName += NamedPipeFlag + boost::lexical_cast<std::wstring>(GetCurrentProcessId());;

	HANDLE hPipe = CreateNamedPipeW ( PipeName.c_str ( ), PIPE_ACCESS_OUTBOUND, PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT, PIPE_UNLIMITED_INSTANCES, NamedPipeBufferSize, NamedPipeBufferSize, 0, NULL );

	if (INVALID_HANDLE_VALUE == hPipe)
	{
		NLPRINT_DEBUGLOG( L"CreateNamedPipe fail, error:%d\n", GetLastError() );	
		return;
	}

	RegisterPEPCLient();

	while(true)
	{
		if (!ConnectNamedPipe(hPipe, NULL) && GetLastError() != ERROR_PIPE_CONNECTED)
		{
			NLPRINT_DEBUGLOG( L"ConnectNamedPipe fail, error:%d\n", GetLastError() );	
			continue;
		}

 		string DisplayText;
 
 		string QueryResult;
 
 		if (QueryScreenCapture(DisplayText))
 		{
 			QueryResult = QueryAllow;
 		}
 		else
 		{
 			QueryResult = QueryDeny + DisplayText;
 		}
 
 		QueryResult.resize(QueryResultLength);
 		QueryResult[QueryResultLength - 1] = '\0';
		
		DWORD WriteNumber = 0;
		
		if (!WriteFile(hPipe, QueryResult.c_str(), QueryResultLength, &WriteNumber, NULL) || WriteNumber != QueryResultLength)
		{
			NLPRINT_DEBUGLOG( L"In TalkWithSCE::StartServerThread, WriteFile error:%d\n", GetLastError());	
		}
		
		if (!FlushFileBuffers(hPipe))
		{
			NLPRINT_DEBUGLOG( L"FlushFileBuffers fail, error:%d\n", GetLastError());		
		}

		if (!DisconnectNamedPipe(hPipe))
		{
			NLPRINT_DEBUGLOG( L"DisconnectNamedPipe fail, error:%d\n", GetLastError());	
		}
	}
}

void TalkWithSCE::RegisterPEPCLient() const
{
	DWORD SessionID = 0;
	DWORD ProcessID = GetCurrentProcessId();

	ProcessIdToSessionId(ProcessID, &SessionID);

	USHORT ServerPort = SCEServerBasedPort + static_cast<USHORT>(SessionID);

	//format:register:type=namedpipe;name=OfficeScreenCapture2438;pid=2438
	string RegisterStr = RegisterHead; 
	RegisterStr += RegisterType; 
	RegisterStr += RegisterSeparator; 
	RegisterStr += RegisterName; 
	RegisterStr += charNamedPipeFlag + boost::lexical_cast<string>(ProcessID);;
	RegisterStr += RegisterSeparator; 
	RegisterStr += RegisterPID;
	RegisterStr += boost::lexical_cast<string>(ProcessID); 
	RegisterStr.resize(RegisterLength);

	while(true)
	{
		try
		{
			io_service ios;

			ip::tcp::socket sock(ios);
			ip::tcp::endpoint ep(ip::address::from_string(SCEServerIP), ServerPort);

			sock.connect(ep);

			write(sock, buffer(RegisterStr));

			vector<char> str(RegisterResultLength);

			read(sock, buffer(str));

			if (boost::algorithm::iequals(&str[0], RegisterSuccessful))
			{
				break;
			}
		}
		catch (...)
		{
			NLPRINT_DEBUGLOG( L"exception in RegisterPEPCLient\n");	
		}
	}
}

void TalkWithSCE::UnregisterPEPCLient() const
{
	DWORD SessionID = 0;
	DWORD ProcessID = GetCurrentProcessId();

	ProcessIdToSessionId(ProcessID, &SessionID);

	USHORT ServerPort = SCEServerBasedPort + static_cast<USHORT>(SessionID);

	string UnregisterStr = UnregisterHead; 
	UnregisterStr += RegisterPID; 
	UnregisterStr += boost::lexical_cast<string>(ProcessID); 
	UnregisterStr.resize(UnregisterLength);

	try
	{
		io_service ios;

		ip::tcp::socket sock(ios);
		ip::tcp::endpoint ep(ip::address::from_string(SCEServerIP), ServerPort);

		sock.connect(ep);

		write(sock, buffer(UnregisterStr));

		vector<char> str(UnregisterResultLength);

		read(sock, buffer(str));
	}
	catch (...)
	{
		NLPRINT_DEBUGLOG( L"exception in UnregisterPEPCLient\n");	
	}
}

bool TalkWithSCE::QueryScreenCapture(string& DisplayText)
{
	//Get all opened files, then query PC
	bool bdeny = false;
	
	nextlabs::Obligations obs;
	
	boost::shared_lock<boost::shared_mutex> readerLock(m_mutex);

	for (map<wstring, OpenedFilesStruct>::iterator it = m_AllOpenedFiles.begin(); it != m_AllOpenedFiles.end(); ++it)
	{
		if (it->second.Count > 0)
		{			
			if( kRtPCDeny == NLOBMGRINSTANCE.NLGetEvaluationResult( (it->first).c_str(), kOA_SCREENCAPTURE, NULL ) )
			{
				bdeny = true;

				break;
			}
		}
	}

	return !bdeny;
}

void TalkWithSCE::CacheOpenedFile(const wstring& fileName, EM_FLAG flag, const TAG& vecTag, PVOID pObj)
{
	if (EF_SUB == flag && NULL != pObj)
	{
		boost::unique_lock<boost::shared_mutex> writeLock(m_mutex);
		
		for (std::map<std::wstring, OpenedFilesStruct>::iterator it = m_AllOpenedFiles.begin(); it != m_AllOpenedFiles.end(); ++it)
		{
			if (it->second.pObj == pObj)
			{
				if (1 == it->second.Count)
				{
					m_AllOpenedFiles.erase(it);
				}
				else
				{
					it->second.Count--;
				}

				break;
			}
		}

		return;
	}

	if (fileName.empty() || isNewOfficeFile(fileName))
	{
		return;
	}

	// fix bug24546
	wstring wstrFileName(fileName);
	NLConvertNetFilePathBySepcifiedSeparator( wstrFileName );	
	NLPRINT_DEBUGLOGEX( true, L" @@@@@@@@@@@@@@@@@ file path:[%s], after convert:[%s] \n", fileName.c_str(), wstrFileName.c_str() );

	boost::unique_lock<boost::shared_mutex> writeLock(m_mutex);
	std::map<std::wstring, OpenedFilesStruct>::iterator it = m_AllOpenedFiles.find(wstrFileName);

	if (EF_ADD == flag)
	{
		if (m_AllOpenedFiles.end() == it)
		{
			OpenedFilesStruct OpenedFileStr(1, pObj, vecTag);

			m_AllOpenedFiles[wstrFileName] = OpenedFileStr;
		}
		else
		{
			if (0 == it->second.Count)
			{
				m_AllOpenedFiles.erase(it);
			}
			else
			{
				it->second.Count++;
			}
		}	
	}
	else
	{
		if (m_AllOpenedFiles.end() != it)
		{
			if (1 == it->second.Count)
			{
				m_AllOpenedFiles.erase(it);
			}
			else
			{
				it->second.Count--;
			}
		}
		else
		{
			OpenedFilesStruct OpenedFileStr(0, pObj, vecTag);

			m_AllOpenedFiles[wstrFileName] = OpenedFileStr;
		}
	}
}


