#include "..\include\WinAPIFile.h"


CWinAPIFile::CWinAPIFile(void)
{
	InitializeCriticalSection(&m_cs_created);
}

CWinAPIFile::~CWinAPIFile(void)
{
	DeleteCriticalSection(&m_cs_created);
}
