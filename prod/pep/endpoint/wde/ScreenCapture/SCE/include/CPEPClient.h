#pragma once
#include <Windows.h>
#include <string>
#include <map>

#include "Utility.h"

#include <boost/noncopyable.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>

#pragma warning(push)
#pragma warning(disable: 4512 4244)  
#include <boost/thread.hpp>
#pragma warning(pop)

namespace SCEClient
{

class IPEPClient;

class CPEPClient
{
public:
	CPEPClient()
	{
		
	}

	~CPEPClient()
	{

	}
 
public:
	_Check_return_ bool Add(_In_ const std::string& Type, _In_ const std::string& Argument) const;

	_Check_return_ bool Remove(_In_ DWORD ProcessID) const;

	_Check_return_ bool Query(_In_ DWORD ProcessID, _Out_ std::string& DisplayText) const;

private:
	_Check_return_ bool AddSocketClient(_In_ const std::string& Argument) const;

	_Check_return_ bool AddNamedPipeClient(_In_ const std::string& Argument) const;

private:
	static boost::mutex	sm_Mutex;

	static std::map<DWORD, boost::shared_ptr<IPEPClient>> sm_PEPClient;
};

class IPEPClient : private boost::noncopyable
{
public:
	virtual ~IPEPClient()
	{

	}

protected:
	IPEPClient()
	{

	}

public:
	_Check_return_ virtual bool Query(_Out_ std::string& DisplayText, _Out_ bool& bInvalid) const = 0;

protected:
	_Check_return_ bool Parse(_Out_ std::string& DisplayText, _In_ const std::string& str) const;

};

class PEPSocketClient : public IPEPClient
{
	friend class CPEPClient;

public:
	virtual ~PEPSocketClient()
	{

	}
	
private:
	PEPSocketClient(_In_ USHORT Port) : m_Port(Port)
	{

	}

public:
	_Check_return_ virtual bool Query(_Out_ std::string& DisplayText, _Out_ bool& bInvalid) const;

private:
	const USHORT m_Port;
};

const WCHAR* const NamedPipePrefix = L"\\\\.\\pipe\\";

class PEPNamedPipeClient : public IPEPClient
{
	friend class CPEPClient;

public:
	virtual ~PEPNamedPipeClient()
	{

	}

private:
	PEPNamedPipeClient(_In_ const std::string& name) : m_name(NamedPipePrefix + Utility::GetInstance().stringTowstring(name))
	{

	}

public:
	_Check_return_ virtual bool Query(_Out_ std::string& DisplayText, _Out_ bool& bInvalid) const;

private:
	const std::wstring m_name;
};

}