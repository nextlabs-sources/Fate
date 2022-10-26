#include "stdafx.h"
#include "GZipHelper.h"

CZLibLoader g_ZlibLoader;

CZLibLoader::CZLibLoader()
{
	m_hlib = NULL;
	m_p_crc32 = NULL;
	m_p_inflateInit2_ = NULL;
	m_p_inflate = NULL;
	m_p_inflateReset = NULL;
	m_p_inflateEnd = NULL;
}

CZLibLoader::~CZLibLoader()
{
}

BOOL CZLibLoader::LoadZlib()
{
	if (m_hlib)
	{
		//	only load once.
		return TRUE;
	}

	//	try to load
	wstring commonPath = GetCommonComponentsDir();
#ifdef _WIN64
	commonPath += GZIP_HELPER_ZILB_NAME_X64;
#else
	commonPath += GZIP_HELPER_ZILB_NAME;
#endif
	m_hlib = LoadLibraryW(commonPath.c_str());
	if( m_hlib  == NULL )
	{
		g_log.Log(CELOG_DEBUG, L"HTTPE:: load unzip lib fail......%s, %d\n", commonPath.c_str(), GetLastError());
		return FALSE;
	}
	g_log.Log(CELOG_DEBUG, "HTTPE:: load unzip lib succeed......\n");

	//	try to get address of loaded library.
	m_p_crc32 = (crc32_type)GetProcAddress(m_hlib,"crc32");
	m_p_inflateInit2_ = (inflateInit2__type)GetProcAddress(m_hlib,"inflateInit2_");
	m_p_inflate = (inflate_type)GetProcAddress(m_hlib,"inflate");
	m_p_inflateReset = (inflateReset_type)GetProcAddress(m_hlib,"inflateReset");
	m_p_inflateEnd = (inflateEnd_type)GetProcAddress(m_hlib,"inflateEnd");

	if (!m_p_crc32 || !m_p_inflateInit2_ || !m_p_inflate || !m_p_inflateReset || !m_p_inflateEnd )
	{
		FreeZlib();
		return FALSE;
	}

	return TRUE;
}

VOID CZLibLoader::FreeZlib()
{
	if (m_hlib)
	{
		FreeLibrary(m_hlib);
		m_hlib = NULL;
	}
}

BOOL CZLibLoader::IsInited()
{
	return m_hlib ? TRUE : FALSE;
}
