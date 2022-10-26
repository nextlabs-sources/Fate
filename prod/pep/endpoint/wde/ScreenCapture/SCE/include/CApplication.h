#pragma once
#include <Windows.h>
#include <vector>

namespace SCEClient
{

class CApplication
{
public:
	CApplication() : m_SessionID(0)
	{
		ProcessIdToSessionId(GetCurrentProcessId(), &m_SessionID);
	}

	~CApplication()
	{

	}

public:
	_Check_return_ bool QueryApp(_In_ DWORD ProcessID, _Out_ std::string& DisplayText) const;

private:
	_Check_return_ bool QueryAllAppOfCurrentSession(_Out_ std::string& DisplayText) const;

	_Check_return_ std::vector<std::wstring> GetProcessesOfCurrentSession() const;

	_Check_return_ BOOL DeviceNameToDriveName(_Inout_ LPWSTR pszFilename) const;

private:
	DWORD m_SessionID;
};

}