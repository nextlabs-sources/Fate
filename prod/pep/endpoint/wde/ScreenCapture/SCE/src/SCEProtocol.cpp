#include "SCEProtocol.h"

#pragma warning(push)
#pragma warning(disable: 4995) 
#include "celog.h"
#pragma warning(pop)

extern CELog  g_log;

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

namespace SCE
{

namespace
{
	const char RegisterHead[] = "register:";
	const char QueryHead[] = "query:";
	const char UnregisterHead[] = "unregister:";
}

boost::shared_ptr<SCEProtocol> SCEProtocol::GetInstance(const char* charProtocol)
{
	std::string strProtocol = charProtocol;

	if (boost::algorithm::istarts_with(strProtocol, RegisterHead))
	{
		return boost::shared_ptr<SCEProtocol>(new RegisterProtocol(strProtocol.substr(_countof(RegisterHead) - 1)));			
	}
	else if (boost::algorithm::istarts_with(strProtocol, QueryHead))
	{
		return boost::shared_ptr<SCEProtocol>(new QueryProtocol(strProtocol.substr(_countof(QueryHead) - 1)));			
	}
	else if (boost::algorithm::istarts_with(strProtocol, UnregisterHead))
	{ 
		return boost::shared_ptr<SCEProtocol>(new UnregisterProtocol(strProtocol.substr(_countof(UnregisterHead) - 1)));			
	}

	return boost::shared_ptr<SCEProtocol>(new SCEProtocol(strProtocol));
}

bool SCEProtocol::Parse()
{
	return false;
}

bool SCEProtocol::Execute(SCEServer* pSCEServer)
{
	return false;
}

bool SCEProtocol::IsEnd() const
{
	return m_End;
}

bool SCEProtocol::GetNextProtocol(bool& bWrite, boost::shared_ptr<std::string> NextProtocol) const
{
	return false;
}

//////////////////////////////////////////////////////////////////////////

namespace
{
	const char* RegisterFormat = "^type=(\\w{0,});(.{0,})";
	const char* RegisterSuccessful = "register=successful";
	const char* RegisterFailed = "register=failed";
	const int RegisterResultLength = 32;
}

bool RegisterProtocol::Parse()
{
	cregex reg = cregex::compile(RegisterFormat);

	cmatch what;
		
	try
	{
		regex_match(m_strProtocol.c_str(), what, reg);
	}
	catch (...)
	{
		g_log.Log(CELOG_DEBUG, L"exception in RegisterProtocol::Parse\n");	

		return false;
	}

	if (what.size() < 3)
	{
		return false;
	}

	m_ConnectType = what[1];
	m_ConnectArgument = what[2];
	
	return true;
}

bool RegisterProtocol::Execute(SCEServer* pSCEServer)
{
	if (NULL == pSCEServer)
	{
		return false;
	}

	if (pSCEServer->AddPEPClient(m_ConnectType, m_ConnectArgument))
	{
		m_strNextProtocol = RegisterSuccessful;
	}
	else
	{
		m_strNextProtocol = RegisterFailed;
	}

	m_strNextProtocol.resize(RegisterResultLength);

	m_End = false;

	return true;
}

bool RegisterProtocol::GetNextProtocol(bool& bWrite, boost::shared_ptr<std::string> NextProtocol) const
{
	if (IsEnd())
	{
		return false;
	}
	
	bWrite = true;

	*NextProtocol = m_strNextProtocol;

	return true;
}

//////////////////////////////////////////////////////////////////////////

namespace
{
	const char* UnregisterFormat = "^pid=(\\d{0,})";;
	const char* UnregisterSuccessful = "unregister=successful";
	const char* UnregisterFailed = "unregister=failed";
	const int UnregisterResultLength = 32;
}

bool UnregisterProtocol::Parse()
{
	cregex reg = cregex::compile(UnregisterFormat);

	cmatch what;

	try
	{
		regex_match(m_strProtocol.c_str(), what, reg);

		if (what.size() < 2)
		{
			return false;
		}

		m_PID = boost::lexical_cast<DWORD>(what[1]);
	}
	catch (...)
	{
		g_log.Log(CELOG_DEBUG, L"exception in UnregisterProtocol::Parse\n");	

		return false;
	}

	return true;
}

bool UnregisterProtocol::Execute(SCEServer* pSCEServer)
{
	if (NULL == pSCEServer)
	{
		return false;
	}

	if (pSCEServer->RemovePEPClient(m_PID))
	{
		m_strNextProtocol = UnregisterSuccessful;
	}
	else
	{
		m_strNextProtocol = UnregisterFailed;
	}

	m_strNextProtocol.resize(UnregisterResultLength);

	m_End = false;

	return true;
}

bool UnregisterProtocol::GetNextProtocol(bool& bWrite, boost::shared_ptr<std::string> NextProtocol) const
{
	if (IsEnd())
	{
		return false;
	}

	bWrite = true;

	*NextProtocol = m_strNextProtocol;

	return true;
}

//////////////////////////////////////////////////////////////////////////

namespace
{
	const char* QueryFormat = "^pid=(\\d{0,})";
	const char* QueryAllow = "query=allow";
	const char* QueryDeny = "query=deny;displaytext=";
	const int QueryResultLength = 512;
}

bool QueryProtocol::Parse()
{
	cregex reg = cregex::compile(QueryFormat);

	cmatch what;

	try
	{
		regex_match(m_strProtocol.c_str(), what, reg);

		if (what.size() < 2)
		{
			return false;
		}

		m_PID = boost::lexical_cast<DWORD>(what[1]);
	}
	catch (...)
	{
		g_log.Log(CELOG_DEBUG, L"exception in QueryProtocol::Parse\n");	

		return false;
	}

	return true;
}

bool QueryProtocol::Execute(SCEServer* pSCEServer)
{
	if (NULL == pSCEServer)
	{
		return false;
	}

	std::string DisplayText;

	if (pSCEServer->Query(m_PID, DisplayText))
	{
		m_strNextProtocol = QueryAllow;
	}
	else
	{
		m_strNextProtocol = QueryDeny + DisplayText;
	}

	m_strNextProtocol.resize(QueryResultLength);

	m_End = false;

	return true;
}

bool QueryProtocol::GetNextProtocol(bool& bWrite, boost::shared_ptr<std::string> NextProtocol) const
{
	if (IsEnd())
	{
		return false;
	}

	bWrite = true;

	*NextProtocol = m_strNextProtocol;

	return true;
}

}