
/*
	Create the mapper to do the mapping: File path with content.
	It also need the timer to control,which list should be removed if it has been time out
*/
#ifndef __NEXTLABS_MAPPER_MANAGER_H__
#define __NEXTLABS_MAPPER_MANAGER_H__


enum EWriteFile_EvalResult
{
	WF_EvR_Unset = 0,
	WF_EvR_Deny,
	WF_EvR_Allow
};

typedef struct tag_STORECREATEINFO
{
	HANDLE hFile ;
	std::wstring strPath ;
	DWORD dThread ;
	EWriteFile_EvalResult writeEvalRes;
	
	tag_STORECREATEINFO()
	{
		writeEvalRes = WF_EvR_Unset;
	}
}*PSTORECREATEINFO,STORECREATEINFO;
typedef std::list<STORECREATEINFO> CREATEFILE_LIST ;
typedef struct tag_STOREDFILEINFO
{
	HANDLE		hFile ;
	std::wstring strPath ;
	std::string strPlainData ;
	DWORD		dTimeStart ;
	DWORD		dThread ;

}*PSTOREDFILEINFO,STOREDFILEINFO;
typedef std::list<	 STOREDFILEINFO	> FILE_INFO_LIST ;

typedef struct tag_STOREDSOCKETINFO
{
	SOCKET s ;
	std::string strSocketData ;
	DWORD dThread ;
	std::wstring strRemotPath ;
	std::string strPlainData ;
	DWORD dwTimeEntry;
}*PSTOREDSOCKETINFO,STOREDSOCKETINFO ;
typedef std::list< STOREDSOCKETINFO > SOCKET_INFO_LIST ;

typedef struct tag_ENCRYPTANDDECRYPTDATA
{
	std::string strPlain;
	std::string strCipher;
	DWORD dwTimeEntry;
}*PENCRYPTANDDECRYPTDATA , ENCRYPTANDDECRYPTDATA;
typedef std::list<ENCRYPTANDDECRYPTDATA>  ENCRYPT_AND_DECRYPT_LIST;


typedef struct tag_HTTPSDOWNLOADINFO
{
	SOCKET s;
	string strData;
	DWORD dwTimeEntry;
}HTTPS_DOWNLOAD_INFO;



class CMapperMngr
{
public:
	static const unsigned long MAX_CONTENT_SIZE = 4096;
	static const unsigned long MAX_TIME_OUT	= 120000 ;


	static CMapperMngr& Instance();
	void SaveLocalHandleAndContent(const HANDLE& hFile,const std::string& content,const std::wstring& strPath ) ;
	//BOOL SaveLocalDataByHandle( const HANDLE hFile, const std::string& content ) ;
	void saveRemotePathAndData( const std::wstring& remotePath, const std::string& strdata /*,const BOOL &IsHTTPS = FALSE*/) ;

	//	add a parameter socket
	void saveRemotePathAndData2( const std::wstring& remotePath, const std::string& strdata, const SOCKET s) ;

	std::wstring GetLocalPathByData( const std::string& strData ) ;
	std::wstring GetLocalPathByDataName(const std::string &strData, const std::wstring& strFileName);
	std::wstring GetRemotePathByData( const std::string& strData/*, const BOOL& IsHTTPS = FALSE */) ;
	BOOL RemoveItem4FileInfo() ;

	//	add by Benjamin, Jan 28 2010
	//	compared with GetRemotePathByData(), this version return the related socket on which the data was downloaed.
	std::wstring GetRemotePathByData_withSocket(const std::string &strData, SOCKET& s);

	/*
	Store the create file handle and the thread.
	*/
	VOID SaveLocalFileHandle( const HANDLE& hFile, const std::wstring & strPath,const DWORD& dID ) ;
	std::wstring GetLocalPathByHandle( const HANDLE &hFile ) ;
	BOOL RemoveItemByHandle( const HANDLE& hFile ) ;
	
	//	comment in Dec,30,2009
	//	a file handle may have a related writefile evaluation result if there was already writefile be called before
	//	we cache the result for both performance and user friendship--bug #837--which is an user friendship bug.
	EWriteFile_EvalResult GetWriteEvalResultByHandle( const HANDLE& hFile ) ;
	VOID SetWriteEvalResultByHandle( const HANDLE& hFile, const EWriteFile_EvalResult evalRes );
	
	void MapEncryptAndDecryptData(const std::string& strPlain , const std::string& strCipher );
	std::string GetDecryptByEncryptData(const std::string& strCipher);


	/************************************************************
	For download with HTTPS,
	We need to call MapSocketEncryptedData in "WSPRecv",
	and call GetSocketByEncryptedData in "decrypt" function.
	We can parse the data packet in "decrypt" function after we get
	the plain text.
	*************************************************************/
	void MapSocketEncryptedData(SOCKET s, const string & strData);
	bool GetSocketByEncryptedData(const string & strData, SOCKET& s);

	//	comment in Jan,08,2010
	//	a file may have a related writefile evaluation result if there was already writefile be called with the same file name.
	//	if the writefile is denied, we deny the writefile with the same file name in 10 seconds. 
	VOID GetWriteEvalResultByFileName(const wstring & strFilename, EWriteFile_EvalResult& evalRes);
	VOID SetWriteEvalResultByFileName(const wstring & strFilename, const EWriteFile_EvalResult evalRes);

private: 
	BOOL RemoveTimeoutItem4FileInfo(const DWORD dCurrentTime, const HANDLE& hFile,const std::string& content,const std::wstring & strPath );
private:
	CMapperMngr(void);
	CMapperMngr(const CMapperMngr&);
	void operator = (const CMapperMngr&);
	~CMapperMngr(void);
private:
	 FILE_INFO_LIST		m_listFileInfo ;
	 SOCKET_INFO_LIST	m_listSocketInfo ;
	 CREATEFILE_LIST	m_mapFileInfo ;
	 ENCRYPT_AND_DECRYPT_LIST m_listEncryptDecrypt;
	 std::list<HTTPS_DOWNLOAD_INFO> m_list_Download_HTTPS;

	//	comment in Jan,08,2010
	//	a file may have a related writefile evaluation result if there was already writefile be called with the same file name.
	//	if the writefile is denied, we deny the writefile with the same file name in 10 seconds. 
	typedef struct __FILENAME_EVAL_STRUCT
	{
		EWriteFile_EvalResult eEvalRes;	//	deny or allow
		DWORD dwStartTick;	//	the time when this evaluation result is set with the filename.

		__FILENAME_EVAL_STRUCT()
		{
			eEvalRes = WF_EvR_Unset;
			dwStartTick = 0;
		}
	}FILENAME_EVAL_STRUCT;
	std::map<std::wstring, FILENAME_EVAL_STRUCT> m_mapFilenameEval;
	static const DWORD m_dwFilenameEvalTimeout = 100000;	//	100 seconds.
};
#endif