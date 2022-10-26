#include "StdAfx.h"
#include "CriticalSections.h"

CRITICAL_SECTION  CCriticalSections::m_csMap;

BOOL CCriticalSections::Init()
{
	BOOL b = InitializeCriticalSectionAndSpinCount(&m_csMap, 0x00000400);
	return b;
}

void CCriticalSections::Delete()
{
	DeleteCriticalSection(&m_csMap);
}