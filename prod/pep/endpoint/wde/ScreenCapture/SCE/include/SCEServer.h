#pragma once
#include <Windows.h>
#include <string>
#include <vector>

#include "CApplication.h"
#include "CPEPClient.h"

#include <boost/noncopyable.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>

#pragma warning(push)
#pragma warning(disable: 6386 6031 6328 6258 6309 6387 6334 4267)  
#include <boost/asio.hpp>
#pragma warning(pop)

#pragma warning(push)
#pragma warning(disable: 4512 4244)  
#include <boost/thread.hpp>
#pragma warning(pop)

namespace SCE
{

const USHORT SCEServerBasedPort = 20000;

class SCEServer : private boost::noncopyable
{
private:
	SCEServer(_In_ USHORT Port): accepter(ios, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), Port)), m_Port(Port)
	{
		start();
	}

public:
	_Check_return_ static boost::shared_ptr<SCEServer> Create( _In_ USHORT Port);

	_Check_return_ static boost::shared_ptr<SCEServer> GetInstance();

	_Check_return_ static bool SCEServer::Release();

public:
	void accept_handler(_In_ const boost::system::error_code& ec, _In_ boost::shared_ptr<boost::asio::ip::tcp::socket> sock);

	void read_handler(_In_ const boost::system::error_code& ec, _In_ boost::shared_ptr<std::vector<char>> str, _In_ boost::shared_ptr<boost::asio::ip::tcp::socket> sock);

	void write_handler(_In_ const boost::system::error_code& ec, _In_ boost::shared_ptr<std::string> str, _In_ boost::shared_ptr<boost::asio::ip::tcp::socket> sock);

	void Run()
	{
		ios.run();
	}

	void start();

public:
	_Check_return_ bool Query(_In_ DWORD ProcessID, _Out_ std::string& DisplayText) const;

	_Check_return_ bool AddPEPClient(_In_ const std::string& Type, _In_ const std::string& Argument);

	_Check_return_ bool RemovePEPClient(_In_ DWORD ProcessID);

private:
	static boost::mutex	sm_Mutex;

	static boost::shared_ptr<SCEServer> sm_SCEServer;
	
private:
	boost::asio::io_service ios;

	boost::asio::ip::tcp::acceptor accepter;

	const USHORT m_Port;

private:
	SCEClient::CApplication m_ApplicationClient;
	SCEClient::CPEPClient m_PEPClient;
};

}