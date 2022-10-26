#include "stdafx.h"
#include "gzipper.h"
#include "criticalMngr.h"

#pragma warning(push)
#pragma warning(disable: 4127)
#include "GZipHelper.h"
#pragma warning(pop)

CGZipper::CGZipper()
{
}

CGZipper::~CGZipper()
{
}

CGZipper& CGZipper::Instance()
{
	::EnterCriticalSection(&CcriticalMngr::s_csCGzipper);
	static CGZipper ins;
	::LeaveCriticalSection(&CcriticalMngr::s_csCGzipper);
	return ins;
}

BOOL CGZipper::UNZipData(const string& input, string& output)
{
	if (!input.length())
	{
		return TRUE;
	}
	CGZIP2A plain( (LPGZIP)input.c_str(), (int)input.length() );  // do decompressing here
	string tmp(plain.psz, plain.Length);
	output = tmp;
	return TRUE;
}
