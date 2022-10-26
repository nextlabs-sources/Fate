#pragma once

class CSharedMemory//shared memory 
{
public:
	CSharedMemory(void);
	CSharedMemory(const std::wstring & strName, int nSize);
	~CSharedMemory(void);
public:
	BOOL CreateSharedMemory(const std::wstring & strName, int nSize);
	void CloseSharedMemory();

	BOOL WriteSharedMemory(PVOID pInfo, int nLen);
	BOOL ReadSharedMemory(PVOID pInfo, int nLen);
protected:
	HANDLE			m_hSharedMemory;
	int				m_nSize;
	std::wstring	m_strSharedMemoryName;
};
