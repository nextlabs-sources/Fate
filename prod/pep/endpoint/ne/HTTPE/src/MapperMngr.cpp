#include "stdafx.h"
#include "MapperMngr.h"
#include "criticalMngr.h"
#include "Utilities.h"

#define  MAX_TIMEOUT_REMOTE     10 * 60 * 1000

#define  MAX_TIMEOUT_ENCRYPT    60 * 1000
#define  MAX_TIMEOUT_DECRYPT    2 * 60 * 1000
#define  MAX_ELEMENT_ENCRYPT	20

CMapperMngr::CMapperMngr(void)
{

}
CMapperMngr::~CMapperMngr(void)
{
}

CMapperMngr& CMapperMngr::Instance()
{
	static CMapperMngr ins;
	
	return ins;
}
void CMapperMngr::SaveLocalHandleAndContent(const HANDLE& hFile,const std::string& content,const std::wstring& strPath)
{	
	DWORD dID = ::GetCurrentThreadId() ;

	if( !RemoveTimeoutItem4FileInfo(GetTickCount(),hFile,content,strPath )	)
	{
		STOREDFILEINFO fileInfo ; 
		fileInfo.dTimeStart = GetTickCount() ;
		fileInfo.hFile = hFile ; 
		fileInfo.strPlainData =	content ;
		fileInfo.dThread = dID ;
		fileInfo.strPath = strPath ;

		::EnterCriticalSection(&CcriticalMngr::s_csFileInfo);
		m_listFileInfo.push_back( fileInfo ) ;
		::LeaveCriticalSection(&CcriticalMngr::s_csFileInfo);
	}
}

std::wstring CMapperMngr::GetLocalPathByData(const std::string &strData)
{
	wstring sPath;

	::EnterCriticalSection(&CcriticalMngr::s_csFileInfo);
	
	FILE_INFO_LIST::iterator itor = m_listFileInfo.begin() ;
	for(; itor != m_listFileInfo.end(); ++itor)
	{
		if(strData.find((*itor).strPlainData) == 0 || (*itor).strPlainData.find(strData) == 0)
		{
			// When we find it, we remove it
			sPath =(*itor).strPath ;
			m_listFileInfo.erase(itor);
			break;
		}	
	}
	
	::LeaveCriticalSection(&CcriticalMngr::s_csFileInfo);
	return	sPath ;
}

std::wstring CMapperMngr::GetLocalPathByDataName(const std::string &strData, const std::wstring& strFileName)
{
	wstring sPath;

	::EnterCriticalSection(&CcriticalMngr::s_csFileInfo);
	
	FILE_INFO_LIST::iterator itor = m_listFileInfo.begin() ;
	
	for(; itor != m_listFileInfo.end(); ++itor)
	{
		const wchar_t* filename = wcsrchr((*itor).strPath.c_str(), '\\');
		if(filename)
		{
			filename = filename + 1;
		}

		if ( ( filename && _wcsicmp( filename, strFileName.c_str() ) == 0 )&&( strData.find((*itor).strPlainData) != string::npos || (*itor).strPlainData.find(strData)  != string::npos ) )
		{
			// When we find it, we remove it
			sPath =(*itor).strPath ;
			m_listFileInfo.erase(itor);
			break;
		}
	}
	::LeaveCriticalSection(&CcriticalMngr::s_csFileInfo);
	return	sPath ;

}

void CMapperMngr::saveRemotePathAndData( const std::wstring &remotePath, const std::string &strdata/*,const BOOL &IsHTTPS*/ )
{
	
	if( remotePath.empty() )
	{
		return;
	}

	::EnterCriticalSection(&CcriticalMngr::s_csSocketInfo);	 
	SOCKET_INFO_LIST::reverse_iterator itor = m_listSocketInfo.rbegin();
	for(; itor != m_listSocketInfo.rend(); ++itor)
	{
		if((*itor).strRemotPath.compare(remotePath) == 0 )
		{
			(*itor).strPlainData = strdata;
			(*itor).dwTimeEntry = GetTickCount();
			::LeaveCriticalSection(&CcriticalMngr::s_csSocketInfo);
			return;
		}
	}
	if(itor == m_listSocketInfo.rend())
	{
   		STOREDSOCKETINFO socketInfo ; 
		socketInfo.strPlainData = strdata ;

		socketInfo.strRemotPath =   remotePath ;
		socketInfo.dwTimeEntry = GetTickCount();
		m_listSocketInfo.push_back( socketInfo ) ;
	}

	::LeaveCriticalSection(&CcriticalMngr::s_csSocketInfo);

}

void CMapperMngr::saveRemotePathAndData2( const std::wstring& remotePath, const std::string& strdata, const SOCKET s)
{
	if( remotePath.empty() )
	{
		return;
	}

	::EnterCriticalSection(&CcriticalMngr::s_csSocketInfo);	 
	SOCKET_INFO_LIST::reverse_iterator itor = m_listSocketInfo.rbegin();
	for(; itor != m_listSocketInfo.rend(); ++itor)
	{
		if((*itor).strRemotPath.compare(remotePath) == 0 )
		{
			if (s == (*itor).s)
			{
				//	the socket is the same, we can replace the data.
				/*Modified by chellee for the sharepoint sendto->download as a copy,It just update the information but not the position
				(*itor).strPlainData = strdata;
				(*itor).dwTimeEntry = GetTickCount();*/
				m_listSocketInfo.erase( (++itor).base() ) ;
				STOREDSOCKETINFO socketInfo ; 
				socketInfo.strPlainData = strdata ;
				socketInfo.strRemotPath =   remotePath ;
				socketInfo.s = s;
				socketInfo.dwTimeEntry = GetTickCount();
				m_listSocketInfo.push_back( socketInfo ) ;

				::LeaveCriticalSection(&CcriticalMngr::s_csSocketInfo);
				return;
			}
			else
			{
				//	the socket is different, there maybe many socket to download data from 
				//	server, we can not simply replace the data, because these data is not
				//	downloaded in order.

				//	NOW, add a new one, These data is the head block of the data downloaded
				//	by current socket, we need to cache these data, this is useful if the application write data downloaded by each socket
				//	into an individual file.	this happened in #882
				STOREDSOCKETINFO socketInfo ; 
				socketInfo.strPlainData = strdata ;
				socketInfo.strRemotPath =   remotePath ;
				socketInfo.s = s;
				socketInfo.dwTimeEntry = GetTickCount();
				m_listSocketInfo.push_back( socketInfo ) ;

				::LeaveCriticalSection(&CcriticalMngr::s_csSocketInfo);
				return;
			}
		}
	}
	if(itor == m_listSocketInfo.rend())
	{
		STOREDSOCKETINFO socketInfo ; 
		socketInfo.strPlainData = strdata ;
		socketInfo.strRemotPath =   remotePath ;
		socketInfo.s = s;
		socketInfo.dwTimeEntry = GetTickCount();
		m_listSocketInfo.push_back( socketInfo ) ;
	}

	::LeaveCriticalSection(&CcriticalMngr::s_csSocketInfo);
}


std::wstring CMapperMngr::GetRemotePathByData(const std::string &strData/*,const BOOL& IsHTTPS*/)
{
	wstring strPath ;

	::EnterCriticalSection(&CcriticalMngr::s_csSocketInfo);	 
	
	SOCKET_INFO_LIST::reverse_iterator itor =	m_listSocketInfo.rbegin() ;
	wstring strTemp;
	for(; itor != m_listSocketInfo.rend(); )
	{
		if(GetTickCount() - (*itor).dwTimeEntry > MAX_TIMEOUT_REMOTE)
		{//added by kevin. It's better to avoid call "Log" in a "critical section" loop. this is dangerous.
			m_listSocketInfo.erase(--itor.base());
			itor = m_listSocketInfo.rbegin();
			continue;
		}

		if( (*itor).strPlainData.find( strData ) == 0 || strData.find( (*itor).strPlainData) == 0)
		{
			strPath =   (*itor).strRemotPath ;
			break ;
		}

		if( strTemp.empty() && ((*itor).strPlainData.find( strData ) != string::npos ||strData.find( (*itor).strPlainData)!= string::npos) )
		{
			strTemp =   (*itor).strRemotPath ;
		}
		++itor;
	}
	::LeaveCriticalSection(&CcriticalMngr::s_csSocketInfo);

	if(strPath.empty())
	{
		strPath = strTemp;
	}
	return strPath ;
}

std::wstring CMapperMngr::GetRemotePathByData_withSocket(const std::string &strData, SOCKET& s)
{
	wstring strPath ;

	::EnterCriticalSection(&CcriticalMngr::s_csSocketInfo);	 

	SOCKET_INFO_LIST::reverse_iterator itor =	m_listSocketInfo.rbegin() ;
	wstring strTemp;
	SOCKET socketTemp = 0;
	for(; itor != m_listSocketInfo.rend(); )
	{
		if(GetTickCount() - (*itor).dwTimeEntry > MAX_TIMEOUT_REMOTE)
		{
			m_listSocketInfo.erase(--itor.base());
			itor = m_listSocketInfo.rbegin();
			continue;
		}

		if( (*itor).strPlainData.find( strData ) == 0 || strData.find( (*itor).strPlainData) == 0)
		{
			strPath =   (*itor).strRemotPath ;
			//	this is new added compared with GetRemotePathByData()
			//	remember the socket which related with the downloaded data.
			//	this is not the temp, so directly assign the value to output parameter s.
			s = (*itor).s;
			break ;
		}

		if( strTemp.empty() && ((*itor).strPlainData.find( strData ) != string::npos ||strData.find( (*itor).strPlainData)!= string::npos) )
		{
			strTemp =   (*itor).strRemotPath ;
			//	this is new added compared with GetRemotePathByData()
			//	remember the temp socket which related with the downloaded data.
			socketTemp = (*itor).s;
		}
		++itor;
	}
	::LeaveCriticalSection(&CcriticalMngr::s_csSocketInfo);

	if(strPath.empty())
	{
		strPath = strTemp;
		//	this is new added compared with GetRemotePathByData()
		//	assign the temp socket to output parameter s
		s = socketTemp;
	}
	return strPath ;
}



/*
 RemoveTimeoutItem4FileInfo need to check if the path is not empty,
 Check if the file has been in the list,
 True, return TRUE   ;
 Remove it and push back
 FALSE, return FALSE ;
*/
BOOL CMapperMngr::RemoveTimeoutItem4FileInfo(const DWORD dCurrentTime, const HANDLE& hFile,const std::string& content,const std::wstring & strPath )
{
	DWORD dCurTime = dCurrentTime ;
	if( dCurTime == 0 )
	{
		dCurTime = GetTickCount() ; 
	}

	::EnterCriticalSection(&CcriticalMngr::s_csFileInfo);
	FILE_INFO_LIST::iterator itor = m_listFileInfo.begin() ; 

	//int nCount = 0;
	for(; itor != m_listFileInfo.end(); )
	{
		if( (hFile)&&(hFile== (*itor).hFile))
		{
			if(((*itor).strPlainData.length() ==content.length() )&&(memcmp((*itor).strPlainData.c_str(),content.c_str(),content.length())==0)	 )
			{
				(*itor).dTimeStart = dCurrentTime ;
				::LeaveCriticalSection(&CcriticalMngr::s_csFileInfo);
				return TRUE ;
			}
			else
			{
				if(	 (*itor).strPath == strPath )
				{
					if(	  (*itor).strPlainData.length() <MAX_CONTENT_SIZE)
					{
						 (*itor).strPlainData +=  content ;
					}
					(*itor).dTimeStart = dCurTime ; 
					::LeaveCriticalSection(&CcriticalMngr::s_csFileInfo);
					return TRUE ;
				}
				/**********************************
				We don't need these codes.
				It doesn't matter if there are 2 same
				handles, since we won't use handle as 
				key.
				fix bug886
										Kevin 2010-1-12
				************************************/
			}
		}
		if( (dCurTime - (*itor).dTimeStart) >MAX_TIME_OUT	 )
		{
			itor = m_listFileInfo.erase(itor);
			continue ;
		}  
		itor++;
	}
	::LeaveCriticalSection(&CcriticalMngr::s_csFileInfo);
	return FALSE ;
}
VOID CMapperMngr::SaveLocalFileHandle( const HANDLE& hFile, const std::wstring & strPath,const DWORD& dID ) 
{
	::EnterCriticalSection(&CcriticalMngr::s_csFileInfo);
	CREATEFILE_LIST::iterator itor = m_mapFileInfo.begin() ;
	for( ;itor!= m_mapFileInfo.end() ; ++itor)
	{
		if((*itor).hFile == hFile )
		{
			m_mapFileInfo.erase( itor) ;
			break ;
		}
	}
	STORECREATEINFO tmpInfo ;
	tmpInfo.dThread = dID ;
	tmpInfo.hFile = hFile ;
	tmpInfo.strPath = strPath ; 
	m_mapFileInfo.push_back( tmpInfo ) ;
	::LeaveCriticalSection(&CcriticalMngr::s_csFileInfo);
}

std::wstring CMapperMngr::GetLocalPathByHandle( const HANDLE &hFile ) 
{
	std::wstring strPath ;

	::EnterCriticalSection(&CcriticalMngr::s_csFileInfo);
	if( !m_mapFileInfo.empty() )
	{
		CREATEFILE_LIST::iterator itor = m_mapFileInfo.begin() ;
		for( ;itor!= m_mapFileInfo.end() ; ++itor)
		{
			if( (*itor).hFile==hFile )
			{
				strPath = (*itor).strPath ;

				//	comment in Dec,25,2009,
				//	the item should be erased in RemoveItemByHandle, which is trigged by CloseHandle.
				//	before the handle is closed, this item is useful for getting file name by handle value, 
				//	so, we do not erase it here.
				break ;
			}
		}
	}
	::LeaveCriticalSection(&CcriticalMngr::s_csFileInfo);
	return strPath ;
}
BOOL CMapperMngr::RemoveItemByHandle( const HANDLE& hFile )
{
	::EnterCriticalSection(&CcriticalMngr::s_csFileInfo);
	BOOL bRet = FALSE ;
	if( !m_mapFileInfo.empty() )
	{
		CREATEFILE_LIST::iterator itor = m_mapFileInfo.begin() ;
		for( ;itor!= m_mapFileInfo.end() ; ++itor)
		{
			if((*itor).hFile==hFile )
			{
				m_mapFileInfo.erase( itor) ;
				bRet = TRUE ;
				break ;
			}
		}
	}
	::LeaveCriticalSection(&CcriticalMngr::s_csFileInfo);
	return bRet ;
}

void CMapperMngr::MapEncryptAndDecryptData(const std::string &strPlain, const std::string &strCipher)
{
	ENCRYPTANDDECRYPTDATA encrytAndDecryt ;
	encrytAndDecryt.strPlain    = strPlain;
	encrytAndDecryt.strCipher = strCipher;
	encrytAndDecryt.dwTimeEntry = GetTickCount();

	::EnterCriticalSection(&CcriticalMngr::s_csEncryptDecryptMap);
	
	if(m_listEncryptDecrypt.size() > MAX_ELEMENT_ENCRYPT)
	{
		m_listEncryptDecrypt.pop_front();

	}
	
	m_listEncryptDecrypt.push_back(encrytAndDecryt);
	::LeaveCriticalSection(&CcriticalMngr::s_csEncryptDecryptMap);
}

std::string CMapperMngr::GetDecryptByEncryptData(const std::string &strCipher)
{
	std::string strPlain = "NOTHTTPS";

	::EnterCriticalSection(&CcriticalMngr::s_csEncryptDecryptMap);
	
	ENCRYPT_AND_DECRYPT_LIST::iterator itor = m_listEncryptDecrypt.begin();
	for (; itor != m_listEncryptDecrypt.end() ; )
	{
		if(GetTickCount() - (*itor).dwTimeEntry > MAX_TIMEOUT_ENCRYPT)
		{
			itor = m_listEncryptDecrypt.erase(itor);
			continue;
		}

		if ( strCipher.find( (*itor).strCipher) != string::npos ||
			(*itor).strCipher.find( strCipher)  !=  string::npos )
		{
			strPlain = (*itor).strPlain;
			m_listEncryptDecrypt.erase(itor);
			break;
		}
		itor++;
	}

	::LeaveCriticalSection(&CcriticalMngr::s_csEncryptDecryptMap);
	return strPlain;
}

void CMapperMngr::MapSocketEncryptedData(SOCKET s, const string & strData)
{
	HTTPS_DOWNLOAD_INFO info;
	info.s = s;
	info.strData = strData;
	info.dwTimeEntry = GetTickCount();

	::EnterCriticalSection(&CcriticalMngr::s_csDownload_HTTPS);
	if(m_list_Download_HTTPS.size() > MAX_ELEMENT_ENCRYPT)//fix bug10988
	{
		m_list_Download_HTTPS.pop_front();
	}

	m_list_Download_HTTPS.push_back(info);
	::LeaveCriticalSection(&CcriticalMngr::s_csDownload_HTTPS);
}

bool CMapperMngr::GetSocketByEncryptedData(const std::string & strData, SOCKET& s)
{
	HTTPS_DOWNLOAD_INFO info;

	bool bRet = false;

	::EnterCriticalSection(&CcriticalMngr::s_csDownload_HTTPS);

	for(std::list<HTTPS_DOWNLOAD_INFO>::iterator itr = m_list_Download_HTTPS.begin(); itr != m_list_Download_HTTPS.end(); )
	{
		if(GetTickCount() - (*itr).dwTimeEntry > MAX_TIMEOUT_DECRYPT)
		{
			itr = m_list_Download_HTTPS.erase(itr);
			continue;
		}

		if((*itr).strData.find(strData) != string::npos || strData.find((*itr).strData) != string::npos)
		{
			bRet = true;
			s = (*itr).s;
			m_list_Download_HTTPS.erase(itr);
			break;
		}
		itr++;
	}

	::LeaveCriticalSection(&CcriticalMngr::s_csDownload_HTTPS);
	return bRet;
}

EWriteFile_EvalResult CMapperMngr::GetWriteEvalResultByHandle( const HANDLE& hFile )
{
	::EnterCriticalSection(&CcriticalMngr::s_csFileInfo);
	if( !m_mapFileInfo.empty() )
	{
		CREATEFILE_LIST::iterator itor = m_mapFileInfo.begin() ;
		for( ;itor!= m_mapFileInfo.end() ; ++itor)
		{
			if( (*itor).hFile==hFile )
			{
				EWriteFile_EvalResult evalRes = itor->writeEvalRes;
				::LeaveCriticalSection(&CcriticalMngr::s_csFileInfo);
				return evalRes;
			}
		}
	}
	::LeaveCriticalSection(&CcriticalMngr::s_csFileInfo);
	return WF_EvR_Unset;
}

VOID CMapperMngr::SetWriteEvalResultByHandle( const HANDLE& hFile, const EWriteFile_EvalResult evalRes )
{
	::EnterCriticalSection(&CcriticalMngr::s_csFileInfo);
	if( !m_mapFileInfo.empty() )
	{
		CREATEFILE_LIST::iterator itor = m_mapFileInfo.begin() ;
		for( ;itor!= m_mapFileInfo.end() ; ++itor)
		{
			if( (*itor).hFile==hFile )
			{
				itor->writeEvalRes = evalRes;

				::LeaveCriticalSection(&CcriticalMngr::s_csFileInfo);
				return;
			}
		}
	}
	::LeaveCriticalSection(&CcriticalMngr::s_csFileInfo);
}

VOID CMapperMngr::GetWriteEvalResultByFileName(const wstring & strFilename, EWriteFile_EvalResult& evalRes)
{
	//	set default output first.
	evalRes = WF_EvR_Unset;

	::EnterCriticalSection(&CcriticalMngr::s_csEvalFilename);

	//	try to see if there is a evaluation result.
	map<wstring, FILENAME_EVAL_STRUCT>::iterator it = m_mapFilenameEval.find(strFilename);
	if (it != m_mapFilenameEval.end())
	{
		//	has a evaluation result with the filename.
			//	try to check if the result is timeout
			DWORD dwCurTick = ::GetTickCount();
			if ( (dwCurTick - it->second.dwStartTick) < m_dwFilenameEvalTimeout )
			{
				//	is not timeout
			evalRes = (*it).second.eEvalRes;
				::LeaveCriticalSection(&CcriticalMngr::s_csEvalFilename);
				return;
			}
			else
			{
				//	it is timeout,	remove this one.
				m_mapFilenameEval.erase(it);
			}
		
	}

	::LeaveCriticalSection(&CcriticalMngr::s_csEvalFilename);
	return;
}

VOID CMapperMngr::SetWriteEvalResultByFileName(const wstring & strFilename, const EWriteFile_EvalResult evalRes)
{
	//	initial the instance first.
	FILENAME_EVAL_STRUCT sEvaRes;
	sEvaRes.eEvalRes = evalRes;
	sEvaRes.dwStartTick = ::GetTickCount();

	::EnterCriticalSection(&CcriticalMngr::s_csEvalFilename);

	//	in the map, \c strFilename should have one can only have one FILENAME_EVAL_STRUCT instance.
	//	so directly use operator [] or map.
	m_mapFilenameEval[strFilename] = sEvaRes;

	::LeaveCriticalSection(&CcriticalMngr::s_csEvalFilename);

	return;
}