#include "StdAfx.h"
#include "SharedMemory.h"


CSharedMemory::CSharedMemory(void)
{
	m_hSharedMemory = NULL;
	m_nSize = 0;
	m_strSharedMemoryName.clear();
}

CSharedMemory::CSharedMemory(const std::wstring & strName, int nSize)
{
	m_hSharedMemory = NULL;
	m_nSize = nSize;
	m_strSharedMemoryName = strName;
}

CSharedMemory::~CSharedMemory(void)
{
	CloseSharedMemory();
}

BOOL CSharedMemory::CreateSharedMemory(const std::wstring & strName, int nSize)
{
	m_nSize = nSize;
	m_strSharedMemoryName = strName;

	m_hSharedMemory = CreateFileMapping(
										INVALID_HANDLE_VALUE,    // use paging file
										NULL,                    // default security 
										PAGE_READWRITE,          // read/write access
										0,                       // max. object size 
										m_nSize,                // buffer size  
										m_strSharedMemoryName.c_str());                 // name of mapping object

	if (m_hSharedMemory == NULL) 
	{ 
		return FALSE;
	}
	return TRUE;
}

void CSharedMemory::CloseSharedMemory()
{
	if(m_hSharedMemory != NULL)
	{
		CloseHandle(m_hSharedMemory);
	}
}

BOOL CSharedMemory::WriteSharedMemory(PVOID pInfo, int nLen)
{
	if(!pInfo || nLen > m_nSize)
	{
		return FALSE;
	}

	HANDLE hMapFile = NULL;
	LPVOID pBuf = NULL;

	hMapFile = OpenFileMapping(
		FILE_MAP_ALL_ACCESS,   // read/write access
		FALSE,                 // do not inherit the name
		m_strSharedMemoryName.c_str());               // name of mapping object 

	if (hMapFile == NULL) 
	{ 
		printf("Could not open file mapping object (%d).\n", 
			GetLastError());

		return FALSE;
	} 

	pBuf =  MapViewOfFile(hMapFile,   // handle to map object
		FILE_MAP_ALL_ACCESS, // read/write permission
		0,                   
		0,                   
		m_nSize);           

	if (pBuf == NULL) 
	{ 
		CloseHandle(hMapFile);
		return FALSE;
	}
	for( INT i=0; i<nLen ; ++i )
	{
		if( IsBadWritePtr((char*)pBuf+i,sizeof(char)) )
		{
			break ;
		}
		memcpy_s((PVOID)((char*)pBuf +i), sizeof(char),(PVOID)((char*)pInfo+i), sizeof(char));
	}

	UnmapViewOfFile(pBuf);

	CloseHandle(hMapFile);

	return TRUE;
}

BOOL CSharedMemory::ReadSharedMemory(PVOID pInfo, int nLen)
{
	if(!pInfo || nLen > m_nSize)
	{
		return FALSE;
	}

	HANDLE hMapFile = NULL ;
	LPVOID pBuf = NULL;

	hMapFile = OpenFileMapping(
		FILE_MAP_ALL_ACCESS,   // read/write access
		FALSE,                 // do not inherit the name
		m_strSharedMemoryName.c_str());               // name of mapping object 

	if (hMapFile == NULL) 
	{ 
		printf("Could not open file mapping object (%d).\n", 
			GetLastError());
		return FALSE;
	} 

	pBuf =  MapViewOfFile(hMapFile, // handle to map object
		FILE_MAP_ALL_ACCESS,  // read/write permission
		0,                    
		0,                    
		m_nSize);                   

	if (pBuf == NULL) 
	{ 
		printf("Could not map view of file (%d).\n", 
			GetLastError()); 
		CloseHandle(hMapFile);
		return FALSE;
	}

	memcpy_s(pInfo, nLen, pBuf, nLen );//read the data from Shared Memory

	UnmapViewOfFile(pBuf);

	CloseHandle(hMapFile);


	return TRUE;
}
