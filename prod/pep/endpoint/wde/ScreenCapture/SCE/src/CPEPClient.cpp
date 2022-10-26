#include "CPEPClient.h"
#include <vector>

#pragma warning( push )
#pragma warning( disable : 6011 )
#  include <boost/algorithm/string.hpp>
#pragma warning(pop)

#pragma warning( push )
#pragma warning( disable : 4996 6326 6246 6385 4328 )
#include <boost/xpressive/xpressive_dynamic.hpp>
#pragma warning(pop)

using namespace boost::xpressive;

#include <boost/lexical_cast.hpp>

#pragma warning(push)
#pragma warning(disable: 6386 6031 6328 6258 6309 6387 6334 4267)  
#include <boost/asio.hpp>
#pragma warning(pop)

using namespace boost::asio;

#pragma warning(push)
#pragma warning(disable: 4995) 
#include "celog.h"
#pragma warning(pop)

extern CELog  g_log;

namespace SCEClient
{

boost::mutex CPEPClient::sm_Mutex;

std::map<DWORD, boost::shared_ptr<IPEPClient>> CPEPClient::sm_PEPClient;

namespace
{
	const char* SocketType = "socket";
	const char* SocketFormat = "^port=(\\d{0,});pid=(.{0,})";

	const char* NamedPipeType = "namedpipe";
	const char* NamedPipeFormat = "^name=(\\w{0,});pid=(.{0,})";
}

bool CPEPClient::Add(const std::string& Type, const std::string& Argument) const
{
	if (boost::algorithm::iequals(Type, SocketType))
	{
		return AddSocketClient(Argument);
	}
	else if (boost::algorithm::iequals(Type, NamedPipeType))
	{
		return AddNamedPipeClient(Argument);
	}

	return false;
}

bool CPEPClient::AddSocketClient(const std::string& Argument) const
{
	cregex reg = cregex::compile(SocketFormat);

	cmatch what;

	USHORT Port = 0;
	DWORD ProcessID = 0;

	try
	{
		regex_match(Argument.c_str(), what, reg);

		if (what.size() < 3)
		{
			return false;
		}

		Port = boost::lexical_cast<USHORT>(what[1]);
		ProcessID = boost::lexical_cast<DWORD>(what[2]);
	}
	catch (...)
	{
		g_log.Log(CELOG_DEBUG, L"exception in CPEPClient::AddSocketClient\n");	

		return false;
	}

	boost::mutex::scoped_lock lock(sm_Mutex);

	sm_PEPClient.insert(std::make_pair(ProcessID, new PEPSocketClient(Port)));

	return true;
}

bool CPEPClient::AddNamedPipeClient(const std::string& Argument) const
{
	cregex reg = cregex::compile(NamedPipeFormat);

	cmatch what;

	std::string name;
	DWORD ProcessID = 0;

	try
	{
		regex_match(Argument.c_str(), what, reg);

		if (what.size() < 3)
		{
			return false;
		}

		name = what[1];
		ProcessID = boost::lexical_cast<DWORD>(what[2]);
	}
	catch (...)
	{
		g_log.Log(CELOG_DEBUG, L"exception in CPEPClient::AddNamedPipeClient\n");	

		return false;
	}

	boost::mutex::scoped_lock lock(sm_Mutex);

	sm_PEPClient.insert(std::make_pair(ProcessID, new PEPNamedPipeClient(name)));

	return true;
}

bool CPEPClient::Remove(DWORD ProcessID) const
{
	boost::mutex::scoped_lock lock(sm_Mutex);
	
	sm_PEPClient.erase(ProcessID);

	return true;
}

bool CPEPClient::Query(DWORD ProcessID, std::string& DisplayText) const
 {
	DisplayText.clear(); 

	bool bAllow = true;
	bool bInvalid = false;

	if(0 == ProcessID)
	{
		boost::mutex::scoped_lock lock(sm_Mutex);

		for (std::map<DWORD, boost::shared_ptr<IPEPClient>>::const_iterator cit = sm_PEPClient.begin(); cit != sm_PEPClient.end(); )
		{
			bAllow = cit->second->Query(DisplayText, bInvalid);

			if (bInvalid)
			{
				cit = sm_PEPClient.erase(cit);	
				continue;
			}

			if (!bAllow)
			{
				return false;
			}

			cit++;
		}	
	}
	else
	{
		boost::mutex::scoped_lock lock(sm_Mutex);

		std::map<DWORD, boost::shared_ptr<IPEPClient>>::const_iterator cit = sm_PEPClient.find(ProcessID);

		if (cit == sm_PEPClient.end())
		{
			return true;
		}

		bAllow = cit->second->Query(DisplayText, bInvalid);

		if (bInvalid)
		{
			sm_PEPClient.erase(ProcessID);	
		}
	}

	return bAllow;
}

//////////////////////////////////////////////////////////////////////////

namespace
{
	const char* PEPClientIP = "127.0.0.1";
	const char* QueryFormat = "^query=(\\w{0,})(;displaytext=(\\w{0,}))?";
	const char* DenyFlag = "deny";
	const char* AllowFlag = "allow";
	const int QueryResultLength = 512;
}

bool IPEPClient::Parse(std::string& DisplayText, const std::string& str) const
{
	cregex reg = cregex::compile(QueryFormat);

	cmatch what;

	try
	{
		regex_match(str.c_str(), what, reg);

		if (what.size() < 4)
		{
			return false;
		}
	}
	catch (...)
	{
		g_log.Log(CELOG_DEBUG, L"exception in IPEPClient::Parse\n");	

		return false;
	}

	const std::string QueryFlag = what[1];

	if (boost::algorithm::iequals(QueryFlag, AllowFlag))
	{
		return true;
	}
	else if (boost::algorithm::iequals(QueryFlag, DenyFlag))
	{
		DisplayText = what[3];
	}

	return false;
}

bool PEPSocketClient::Query(std::string& DisplayText, bool& bInvalid) const
{
	bInvalid = false;

	try
	{
		io_service ios;

		ip::tcp::socket sock(ios);
		ip::tcp::endpoint ep(ip::address::from_string(PEPClientIP), m_Port);

		sock.connect(ep);

		std::vector<char> str(QueryResultLength);

		read(sock, buffer(str));
		
		return Parse(DisplayText, &str[0]);
	}
	catch (...)
	{
		g_log.Log(CELOG_DEBUG, L"exception in PEPSocketClient::Query\n");	

		bInvalid = true;			
	}

	return true;
}

bool PEPNamedPipeClient::Query(std::string& DisplayText, bool& bInvalid) const
{
	bInvalid = false;

	HANDLE hPipe = CreateFileW ( m_name.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL );

	if (INVALID_HANDLE_VALUE == hPipe)
	{
		if (ERROR_PIPE_BUSY == GetLastError())
		{
			return false;
		}
		else
		{
			bInvalid = true;
			return true;
		}
	}

	char Buffer[QueryResultLength] = { 0 };
	DWORD ReadNumber = 0;

	if (ReadFile(hPipe, Buffer, QueryResultLength, &ReadNumber, NULL) && ReadNumber == QueryResultLength)
	{
		CloseHandle(hPipe);
		return Parse(DisplayText, Buffer);
	}

	CloseHandle(hPipe);
	return false;
}

}