#include "SCEServer.h"
#include "SCEProtocol.h"
#include <exception>

#include <boost/bind.hpp>

using namespace boost::asio;

#pragma warning(push)
#pragma warning(disable: 4995) 
#include "celog.h"
#pragma warning(pop)

extern CELog  g_log;

namespace SCE
{

boost::mutex	 SCEServer::sm_Mutex;
boost::shared_ptr<SCEServer>  SCEServer::sm_SCEServer;

const int ReceivedLength = 64;

boost::shared_ptr<SCEServer> SCEServer::GetInstance()
{
	boost::mutex::scoped_lock lock(sm_Mutex);

	if ( NULL == sm_SCEServer )
	{
		throw std::exception("SCE Null instance");  
	}

	return sm_SCEServer;
}

boost::shared_ptr<SCEServer> SCEServer::Create(USHORT Port)
{
	boost::mutex::scoped_lock lock(sm_Mutex);

	sm_SCEServer.reset(new SCEServer(Port));

	return sm_SCEServer;
}

bool SCEServer::Release()
{
	boost::mutex::scoped_lock lock(sm_Mutex);

	sm_SCEServer.reset();

	return true;
}

void SCEServer::start()
{
	boost::shared_ptr<ip::tcp::socket> sock (new ip::tcp::socket(ios));

	accepter.async_accept(*sock, boost::bind(&SCEServer::accept_handler, this, placeholders::error, sock));
}

void SCEServer::accept_handler(const boost::system::error_code& ec, boost::shared_ptr<boost::asio::ip::tcp::socket> sock)
{
	if(ec)
	{
		g_log.Log(CELOG_DEBUG, L"SCE server accept_handle fail\n");

		return;
	}

	boost::shared_ptr<std::vector<char>> str(new std::vector<char>(ReceivedLength));
 
	async_read(*sock, buffer(*str), boost::bind(&SCEServer::read_handler, this, placeholders::error, str, sock));

	start();
}

void SCEServer::read_handler(const boost::system::error_code& ec, boost::shared_ptr<std::vector<char>> str, boost::shared_ptr<boost::asio::ip::tcp::socket> sock)
{
	if(ec)
	{
		g_log.Log(CELOG_DEBUG, L"SCE server read_handler fail\n");

		return;
	}

	boost::shared_ptr<SCEProtocol> ThisProtocol = SCEProtocol::GetInstance(&(*str)[0]);
	
	if (!ThisProtocol->Parse())
	{
		g_log.Log(CELOG_DEBUG, L"SCE server received protocol is incorrect\n");

		return;
	}

	if (!ThisProtocol->Execute(this))
	{
		g_log.Log(CELOG_DEBUG, L"SCE server fails to execute current protocol\n");

		return;
	}

	bool bWrite = false;

	boost::shared_ptr<std::string> NextProtocol(new std::string);
	
	if (ThisProtocol->GetNextProtocol(bWrite, NextProtocol))
	{
		if (!bWrite)
		{
			boost::shared_ptr<std::vector<char>> strWrite(new std::vector<char>(ReceivedLength));

			async_read(*sock, buffer(*strWrite), boost::bind(&SCEServer::read_handler, this, placeholders::error, strWrite, sock));
		}
		else
		{
			async_write(*sock, buffer(*NextProtocol), boost::bind(&SCEServer::write_handler, this, placeholders::error, NextProtocol, sock));	
		}				
	}
	else
	{
		g_log.Log(CELOG_DEBUG, L"SCE server end this socket\n");
	}

	return ;
}

void SCEServer::write_handler(const boost::system::error_code& ec, boost::shared_ptr<std::string> str, boost::shared_ptr<boost::asio::ip::tcp::socket> sock)
{
	if(ec)
	{
		g_log.Log(CELOG_DEBUG, L"SCE server write_handler fail\n");

		return;
	}

	boost::shared_ptr<std::vector<char>> strRead(new std::vector<char>(ReceivedLength));

	async_read(*sock, buffer(*strRead), boost::bind(&SCEServer::read_handler, this, placeholders::error, strRead, sock));

	return;
}

bool SCEServer::AddPEPClient(const std::string& Type, const std::string& Argument)
{ 
	return m_PEPClient.Add(Type, Argument);
}

bool SCEServer::RemovePEPClient(DWORD ProcessID)
{
	return m_PEPClient.Remove(ProcessID);
}

bool SCEServer::Query(DWORD ProcessID, std::string& DisplayText) const  
{
	if (!m_ApplicationClient.QueryApp(ProcessID, DisplayText))
	{
		return false;
	}
	
	if (!m_PEPClient.Query(ProcessID, DisplayText))
	{
		return false;
	}

	return true;
}

}