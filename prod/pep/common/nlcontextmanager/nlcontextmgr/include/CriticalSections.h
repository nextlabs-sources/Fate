#pragma once

#include <windows.h>

class CCriticalSections
{
public:
	static BOOL Init();
	static void Delete();

	static CRITICAL_SECTION  m_csMap;
};
