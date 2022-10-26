#pragma  once
#include <string>
#include "NL_base64.h"

using std::wstring;

//////////////////////////////////////////////////////////////////////////
std::wstring	g_strPDFDocumentPath=L"";	
static DWORD g_dwIsAdobeAddin=0;	// 0 is no initialize,8,9,10

inline void ReadAdobeReaderVersion(DWORD& dwVersion)
{
	// default is 9.0
	dwVersion = 9;
	HKEY hKey;

	if (::RegOpenKeyExW (HKEY_LOCAL_MACHINE, L"SOFTWARE\\Adobe\\Acrobat Reader", 
		0, KEY_READ, &hKey) == ERROR_SUCCESS)
	{
		wchar_t szKeyName[512]={0};
		int i=0;
		while(true)
		{
			if(RegEnumKey(hKey,i++,szKeyName,512) != ERROR_SUCCESS)	break;
			if(_wcsicmp(szKeyName,L"8.0") == 0)
			{
				dwVersion = 8;	break;
			}
			else if(_wcsicmp(szKeyName,L"9.0") == 0)
			{
				dwVersion = 9; break;
			}
			else if(_wcsicmp(szKeyName,L"10.0") == 0)
			{
				dwVersion = 10; break;
			}
		}

		::RegCloseKey (hKey);
	}
}

std::string MyWideCharToMultipleByte(const std::wstring & strValue)
{
	int nLen = WideCharToMultiByte(CP_ACP, 0, strValue.c_str(), (int)strValue.length(), NULL, 0, NULL, NULL); 

	char* pBuf = new char[nLen + 1];
	if(!pBuf)
		return "";
	memset(pBuf, 0, nLen +1);
	nLen = WideCharToMultiByte(CP_ACP, 0, strValue.c_str(), (int)strValue.length(), pBuf, nLen, NULL, NULL); 

	std::string strResult(pBuf);

	if(pBuf)
	{
		delete []pBuf;
		pBuf = NULL;
	}
	return strResult;
}

std::wstring MyMultipleByteToWideChar(const std::string & strValue)
{
	int nLen = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, strValue.c_str(), (int)strValue.length(), NULL, 0 );
	DWORD dError = 0 ;
	if( nLen == 0 )
	{
		dError = ::GetLastError() ;
		if( dError == ERROR_INVALID_FLAGS )
		{
			nLen = MultiByteToWideChar(CP_ACP, 0, strValue.c_str(), (int)strValue.length(), NULL, 0 );
		}
	}
	wchar_t* pBuf = new wchar_t[nLen + 1];
	if(!pBuf)
		return L"";

	memset(pBuf, 0, (nLen + 1) * sizeof(wchar_t));
	MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, strValue.c_str(), (int)strValue.length(), pBuf, nLen);
	dError = ::GetLastError() ;
	if( dError == ERROR_INVALID_FLAGS )
	{
		MultiByteToWideChar(CP_ACP, 0, strValue.c_str(), (int)strValue.length(), pBuf, nLen);
	}
	std::wstring strResult(pBuf);

	if(pBuf)
	{
		delete []pBuf;
		pBuf = NULL;
	}
	return strResult;
}

#define PDF_ENCODEINDEX "?rf="

bool DecodeHttpPath(const wstring& strTempPath,wstring& strTruePath)
{
	if(!boost::algorithm::istarts_with(strTempPath,L"http://") &&
		!boost::algorithm::istarts_with(strTempPath,L"https://"))	return false;
	if(!boost::algorithm::icontains(strTempPath,PDF_KEYWORD)) 	return false;

	std::string csrul = MyWideCharToMultipleByte(strTempPath);
	std::string strpath(csrul);
	std::transform(csrul.begin(), csrul.end(), csrul.begin(),
		tolower);

	std::string strlog = "The temp file path is:\t" + csrul + "\t the true path is:\t";

	size_t nStart = csrul.find(PDF_ENCODEINDEX);
	if(nStart == std::string::npos)	return false;
	strpath.erase(0,nStart + strlen(PDF_ENCODEINDEX));

	csrul = Base64::base64_decode(strpath);

	strlog += csrul + ".......\n";

	if(!csrul.empty() && boost::algorithm::starts_with(csrul,"http"))
	{
		strTruePath = MyMultipleByteToWideChar(csrul);
		return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
const wchar_t*	 PDFFILE_OPERATION_MUTEX	= L"e84er32956s-9130-4d86-q8f2-0f1368ed0501";
const wchar_t*   PDFFILE_OPERATION_UUID     = L"e84er32956s-9130-4d86-q8f2-0f1368ed0502";

struct PDFCACHE
{
	wchar_t szPath[2048];
};
class CPDF_Manager
{
private:
	HANDLE m_pdf_file_mutex;
	HANDLE m_pdf_file_mappting;
public:
	CPDF_Manager()
	{
		m_pdf_file_mutex = CreateMutexW(NULL,FALSE,PDFFILE_OPERATION_MUTEX);
		m_pdf_file_mappting = ::CreateFileMapping(INVALID_HANDLE_VALUE,NULL,PAGE_READWRITE,0,sizeof(PDFCACHE),PDFFILE_OPERATION_UUID);
	}
	~CPDF_Manager()
	{
		if(m_pdf_file_mappting != NULL)	CloseHandle(m_pdf_file_mappting);
		if(m_pdf_file_mutex != NULL)	CloseHandle(m_pdf_file_mutex);
	}
public:
	/*
	*\ brief: cache current pdf document file for print/copy/paste pdf file in browser with
	*	adobe reader x, because the action was happens on parent process,but we need to 
	*	get the current path from here. and we can user file mapping in parent process
	*/
	void CacheCurrentPDFPath(wstring& strPath)
	{
		if(m_pdf_file_mutex == NULL ||
			m_pdf_file_mappting == NULL)
		{
			return ;
		}
		if(boost::algorithm::istarts_with(strPath,L"file:///"))		boost::algorithm::ierase_first(strPath,L"file:///");

		bool bHttps = boost::algorithm::istarts_with(strPath,L"https://");

		if(boost::algorithm::icontains(strPath,PDF_KEYWORD))
		{
			wstring strTruePath;
			if(DecodeHttpPath(strPath,strTruePath) && !strTruePath.empty())
			{
				strPath = strTruePath;
			}
		}
		if(!boost::algorithm::iends_with(strPath,L".pdf")) return ;

		if (bHttps)	boost::algorithm::replace_first(strPath,L"http://",L"https://");

		ConvertURLCharacterW ( strPath );

		// open mutex
		DWORD dRet  = WaitForSingleObject(m_pdf_file_mutex, INFINITE);
		if(WAIT_OBJECT_0 == dRet)
		{
			// open filemapping
			PDFCACHE* pMaping = (PDFCACHE*)MapViewOfFile(m_pdf_file_mappting,FILE_MAP_WRITE,0,0,0);
			// set path into file mapping
			if (pMaping != NULL)
			{
				wcsncpy_s(pMaping->szPath,2048,strPath.c_str(), _TRUNCATE);
				UnmapViewOfFile(pMaping); 
			}
			ReleaseMutex(m_pdf_file_mutex);
		}
	}
};
