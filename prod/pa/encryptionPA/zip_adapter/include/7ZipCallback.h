#ifndef __7ZIP_CALLBACK_H
#define __7ZIP_CALLBACK_H

class CArchiveUpdateCallback: 
	public IArchiveUpdateCallback,
	public ICryptoGetTextPassword2,
	public ICompressProgressInfo,
	public CMyUnknownImp
{
public:
	MY_UNKNOWN_IMP3(
		IArchiveUpdateCallback, 
		ICryptoGetTextPassword2,
		ICompressProgressInfo)

	// IProgress
	STDMETHOD(SetTotal)(UInt64 size);
	STDMETHOD(SetCompleted)(const UInt64 *completeValue);
	STDMETHOD(SetRatioInfo)(const UInt64 *inSize, const UInt64 *outSize);

	// IUpdateCallback
	STDMETHOD(EnumProperties)(IEnumSTATPROPSTG **enumerator);  
	STDMETHOD(GetUpdateItemInfo)(UInt32 index, 
		Int32 *newData, Int32 *newProperties, UInt32 *indexInArchive);
	STDMETHOD(GetProperty)(UInt32 index, PROPID propID, PROPVARIANT *value);
	STDMETHOD(GetStream)(UInt32 index, ISequentialInStream **inStream);
	STDMETHOD(SetOperationResult)(Int32 operationResult);

	STDMETHOD(CryptoGetTextPassword2)(Int32 *passwordIsDefined, BSTR *password);

public:
#if 0
	CRecordVector<UInt64> VolumesSizes;
	UString VolName;
	UString VolExt;

	UString DirPrefix;
	bool ShareForWrite;
	bool StdInMode;
#endif
	bool m_bDoEncrypt;
	UString m_wzPassword;
	funcZipPasswordCallback m_fZipPasswordCallback;
	LPVOID m_lpPasswordContext;
	const CObjectVector<CDirItem> *m_pDirItems;

	UInt64 m_uiTotalSize;
	funcZipProcessCallback m_fZipProcessCallback;
	LPVOID m_lpProcessContext;
#if 0
	const CObjectVector<CArchiveItem> *ArchiveItems;
	const CObjectVector<CUpdatePair2> *UpdatePairs;
	CMyComPtr<IInArchive> Archive;
#endif

	CArchiveUpdateCallback();
};

class COpenCallbackImp: 
	public IArchiveOpenCallback,
	public IArchiveOpenVolumeCallback,
#ifndef _NO_CRYPTO
	public ICryptoGetTextPassword,
#endif  
	public CMyUnknownImp
{
public:
#ifndef _NO_CRYPTO
	MY_UNKNOWN_IMP2(
		IArchiveOpenVolumeCallback,
		ICryptoGetTextPassword
		)
#else
	MY_UNKNOWN_IMP1(
		IArchiveOpenVolumeCallback
		)
#endif

	STDMETHOD(SetTotal)(const UInt64 *files, const UInt64 *bytes);
	STDMETHOD(SetCompleted)(const UInt64 *files, const UInt64 *bytes);

	// IArchiveOpenVolumeCallback
	STDMETHOD(GetProperty)(PROPID propID, PROPVARIANT *value);
	STDMETHOD(GetStream)(const wchar_t *name, IInStream **inStream);

#ifndef _NO_CRYPTO
	// ICryptoGetTextPassword
	STDMETHOD(CryptoGetTextPassword)(BSTR *password);
#endif

private:
	UString _folderPrefix;
	NWindows::NFile::NFind::CFileInfoW _fileInfo;
public:
	UStringVector FileNames;
	UInt64 TotalSize;

	COpenCallbackImp() {}
	void Init(const UString &folderPrefix,  const UString &fileName)
	{
		_folderPrefix = folderPrefix;
		if (!NWindows::NFile::NFind::FindFile(_folderPrefix + fileName, _fileInfo))
			throw 1;
		FileNames.Clear();
		TotalSize = 0;
	}
	int FindName(const UString &name);
};

class CArchiveExtractCallback: 
	public IArchiveExtractCallback,
	// public IArchiveVolumeExtractCallback,
	public ICryptoGetTextPassword,
	public ICompressProgressInfo,
	public CMyUnknownImp
{
public:
	MY_UNKNOWN_IMP2(ICryptoGetTextPassword, ICompressProgressInfo)
		// COM_INTERFACE_ENTRY(IArchiveVolumeExtractCallback)

	// IProgress
	STDMETHOD(SetTotal)(UInt64 size);
	STDMETHOD(SetCompleted)(const UInt64 *completeValue);
	STDMETHOD(SetRatioInfo)(const UInt64 *inSize, const UInt64 *outSize);

	// IExtractCallBack
	STDMETHOD(GetStream)(UInt32 index, ISequentialOutStream **outStream, Int32 askExtractMode);
	STDMETHOD(PrepareOperation)(Int32 askExtractMode);
	STDMETHOD(SetOperationResult)(Int32 resultEOperationResult);

	// IArchiveVolumeExtractCallback
	// STDMETHOD(GetInStream)(const wchar_t *name, ISequentialInStream **inStream);

	// ICryptoGetTextPassword
	STDMETHOD(CryptoGetTextPassword)(BSTR *aPassword);

private:
	CMyComPtr<IInArchive> _archiveHandler;
	UString _directoryPath;

	UString _filePath;
	UInt64 _position;
	bool _isSplit;

	UString _diskFilePath;

	bool _extractMode;

	bool WriteModified;
	bool WriteCreated;
	bool WriteAccessed;

	bool _encrypted;

	struct CProcessedFileInfo
	{
		FILETIME CreationTime;
		FILETIME LastWriteTime;
		FILETIME LastAccessTime;
		UInt32 Attributes;

		bool IsCreationTimeDefined;
		bool IsLastWriteTimeDefined;
		bool IsLastAccessTimeDefined;

		bool IsDirectory;
		bool AttributesAreDefined;
	} _processedFileInfo;

	UInt64 _curSize;
	COutFileStream *_outFileStreamSpec;
	CMyComPtr<ISequentialOutStream> _outFileStream;

	UString _itemDefaultName;
	ZipOverwriteMode _overwriteMode;
	UString m_wzPassword;
	funcZipPasswordCallback m_fZipPasswordCallback;
	LPVOID m_lpPasswordContext;
	funcZipOverWirtePromptCallback m_fZipOverWirtePromptCallback;
	LPVOID m_lpOverWirtePromptContext;
	funcZipProcessCallback m_fZipProcessCallback;
	LPVOID m_lpProcessContxt;

	void CreateComplexDirectory(const UStringVector &dirPathParts, UString &fullPath);
	HRESULT GetTime(int index, PROPID propID, FILETIME &filetime, bool &filetimeIsDefined);
public:
	CArchiveExtractCallback():
	  WriteModified(true),
		  WriteCreated(true),
		  WriteAccessed(false),
		  m_wzPassword()
	  {

	  }

	  UInt64 _unpTotal;

	  UInt64 NumFolders;
	  UInt64 NumFiles;
	  UInt64 UnpackSize;

	  void Init(
		  IInArchive *archiveHandler, 
		  const UString &directoryPath,
		  const UString &itemDefaultName,
		  ZipOverwriteMode overwriteMode,
		  const UString &passwd,
		  funcZipPasswordCallback fPasswdCb,
		  LPVOID lpPasswdContxt,
		  funcZipOverWirtePromptCallback fOverwritePromptCb,
		  LPVOID lpOverwritePromptContxt,
		  funcZipProcessCallback fProcessCb,
		  LPVOID lpProcessContxt);

	  UInt64 _numErrors;

	  Int32 m_iOperationResult;
};

HRESULT GetArchiveItemPath(IInArchive *archive, UInt32 index, UString &result);
HRESULT GetArchiveItemPath(IInArchive *archive, UInt32 index, const UString &defaultName, UString &result);
HRESULT GetArchiveItemFileTime(IInArchive *archive, UInt32 index, 
							   const FILETIME &defaultFileTime, FILETIME &fileTime);
HRESULT IsArchiveItemProp(IInArchive *archive, UInt32 index, PROPID propID, bool &result);
HRESULT IsArchiveItemFolder(IInArchive *archive, UInt32 index, bool &result);

#endif