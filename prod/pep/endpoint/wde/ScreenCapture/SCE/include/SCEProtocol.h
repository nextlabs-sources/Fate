#pragma once
#include <string>

#include "SCEServer.h"

namespace SCE
{

class SCEProtocol : private boost::noncopyable
{
public:
	_Check_return_ static boost::shared_ptr<SCEProtocol> GetInstance(_In_ const char* charProtocol);

public:
	virtual ~SCEProtocol()
	{

	}

protected:
	SCEProtocol(_In_ const std::string& strProtocol) : m_strProtocol(strProtocol), m_End(true)
	{

	}

public:
	_Check_return_ virtual bool Parse();

	_Check_return_ virtual bool Execute(_In_ SCEServer* pSCEServer);

	_Check_return_ virtual bool IsEnd() const;

	_Check_return_ virtual bool GetNextProtocol(_Out_ bool& bWrite, _In_ boost::shared_ptr<std::string> NextProtocol) const;

protected:
	const std::string m_strProtocol;
	std::string m_strNextProtocol;

	bool m_End;
};

class RegisterProtocol : public SCEProtocol
{
	friend boost::shared_ptr<SCEProtocol> SCEProtocol::GetInstance(const char* charProtocol);

public:
	virtual ~RegisterProtocol()
	{

	}

private:
	RegisterProtocol(_In_ const std::string& strProtocol) : SCEProtocol(strProtocol)
	{

	}

public:
	_Check_return_ virtual bool Parse();

	_Check_return_ virtual bool Execute(_In_ SCEServer* pSCEServer);

	_Check_return_ virtual bool GetNextProtocol(_Out_ bool& bWrite, _In_ boost::shared_ptr<std::string> NextProtocol) const;

private:
	std::string m_ConnectType;
	std::string m_ConnectArgument;
};

class UnregisterProtocol : public SCEProtocol
{
	friend boost::shared_ptr<SCEProtocol> SCEProtocol::GetInstance(const char* charProtocol);

public:
	virtual ~UnregisterProtocol()
	{

	}

private:
	UnregisterProtocol(_In_ const std::string& strProtocol) : SCEProtocol(strProtocol), m_PID(0)
	{

	}

public:
	_Check_return_ virtual bool Parse();

	_Check_return_ virtual bool Execute(_In_ SCEServer* pSCEServer);

	_Check_return_ virtual bool GetNextProtocol(_Out_ bool& bWrite, _In_ boost::shared_ptr<std::string> NextProtocol) const;

private:
	DWORD m_PID;
};

class QueryProtocol : public SCEProtocol
{
	friend boost::shared_ptr<SCEProtocol> SCEProtocol::GetInstance(const char* charProtocol);

public:
	virtual ~QueryProtocol()
	{

	}

private:
	QueryProtocol(_In_ const std::string& strProtocol) : SCEProtocol(strProtocol), m_PID(0)
	{

	}

public:
	_Check_return_ virtual bool Parse();

	_Check_return_ virtual bool Execute(_In_ SCEServer* pSCEServer);

	_Check_return_ virtual bool GetNextProtocol(_Out_ bool& bWrite, _In_ boost::shared_ptr<std::string> NextProtocol) const;

private:
 	DWORD m_PID;
};

}