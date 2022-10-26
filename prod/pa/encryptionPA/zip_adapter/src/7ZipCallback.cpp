/*
Written by Derek Zheng
June 2008
*/

#pragma once

#include "stdafx.h"

#include "log.h"

#include "Common/MyInitGuid.h"
#include "Common/MyCom.h"
#include "Common/MyString.h"
#include "Common/StringConvert.h"
#include "Common/IntToString.h"
#include "Common/Defs.h"
#include "Common/ComTry.h"
#include "Common/Wildcard.h"

#include "Windows/PropVariant.h"
#include "Windows/PropVariantConversions.h"
#include "Windows/FileFind.h"
#include "Windows/FileDir.h"

#include "7zip/Common/FileStreams.h"
#include "7zip/Common/ProgressUtils.h"
#include "7zip/Common/FilePathAutoRename.h"
#include "7zip/Archive/IArchive.h"
#include "7zip/IPassword.h"
#include "7zip/ICoder.h"

#include "DirItem.h"
#include "ExtractingFilePath.h"

#include "ZipSdk.h"
#include "7ZipCallback.h"

using namespace NWindows;

CArchiveUpdateCallback::CArchiveUpdateCallback():
	m_bDoEncrypt(false),
	m_wzPassword(),
	m_fZipPasswordCallback(NULL),
	m_lpPasswordContext(NULL),
	m_pDirItems(0),
	m_uiTotalSize(0),
	m_fZipProcessCallback(NULL),
	m_lpProcessContext(NULL)
{}

STDMETHODIMP CArchiveUpdateCallback::SetTotal(UInt64 size)
{
	m_uiTotalSize = size;

	return S_OK;
}

STDMETHODIMP CArchiveUpdateCallback::SetCompleted(const UInt64 *completeValue)
{
	if (m_fZipProcessCallback)
	{
		m_fZipProcessCallback(m_lpProcessContext, *completeValue, m_uiTotalSize);
	}

	return S_OK;
}

STDMETHODIMP CArchiveUpdateCallback::SetRatioInfo(const UInt64 *inSize, const UInt64 *outSize)
{
	UNUSED(inSize);
	UNUSED(outSize);
	return S_OK;
}

STDMETHODIMP CArchiveUpdateCallback::EnumProperties(IEnumSTATPROPSTG **)
{
	return E_NOTIMPL;
}

STDMETHODIMP CArchiveUpdateCallback::GetUpdateItemInfo(UInt32 index, 
													   Int32 *newData, Int32 *newProperties, UInt32 *indexInArchive)
{
	UNUSED(index);
	*newData = BoolToInt(true);
	*newProperties = BoolToInt(true);

	*indexInArchive = UInt32(-1);

	return S_OK;
}

STDMETHODIMP CArchiveUpdateCallback::GetProperty(UInt32 index, PROPID propID, PROPVARIANT *value)
{
	NWindows::NCOM::CPropVariant propVariant;

	if (propID == kpidIsAnti)
	{
		propVariant = false;
		propVariant.Detach(value);
		return S_OK;
	}

	const CDirItem &dirItem = (*m_pDirItems)[index];
	switch(propID)
	{
	case kpidPath:
		propVariant = dirItem.Name;
		break;
	case kpidIsFolder:
		propVariant = dirItem.IsDirectory();
		break;
	case kpidSize:
		propVariant = dirItem.Size;
		break;
	case kpidAttributes:
		propVariant = dirItem.Attributes;
		break;
	case kpidLastAccessTime:
		propVariant = dirItem.LastAccessTime;
		break;
	case kpidCreationTime:
		propVariant = dirItem.CreationTime;
		break;
	case kpidLastWriteTime:
		propVariant = dirItem.LastWriteTime;
		break;
	}

	propVariant.Detach(value);
	return S_OK;
}

STDMETHODIMP CArchiveUpdateCallback::GetStream(UInt32 index, ISequentialInStream **inStream)
{
	COM_TRY_BEGIN

	const CDirItem &dirItem = (*m_pDirItems)[index];
	if(dirItem.IsDirectory())
		return S_OK;

	CInFileStream *inStreamSpec = new CInFileStream;
	CMyComPtr<ISequentialInStream> inStreamLoc(inStreamSpec);
	UString path = dirItem.FullPath;
	if(!inStreamSpec->OpenShared(path, true))
	{
		DPW((L"Open file %s failed!\n", path));
		return E_FAIL;
	}

	*inStream = inStreamLoc.Detach();

	return S_OK;
	COM_TRY_END
}

STDMETHODIMP CArchiveUpdateCallback::SetOperationResult(Int32 operationResult)
{
	UNUSED(operationResult);
	return S_OK;
}

STDMETHODIMP CArchiveUpdateCallback::CryptoGetTextPassword2(Int32 *passwordIsDefined, BSTR *password)
{
	COM_TRY_BEGIN
	if (!m_bDoEncrypt)
	{
		*passwordIsDefined = BoolToInt(false);
		*password = NULL;
		return S_OK;
	}

	*passwordIsDefined = BoolToInt(true);
	if (m_wzPassword.IsEmpty())
	{
		if (!m_fZipPasswordCallback)
		{
			DPW((L"Password should be set if encrypting the zip archive!\n"));
			return E_FAIL;
		}
		HRESULT hr = m_fZipPasswordCallback(m_lpPasswordContext, password);
		return hr;
	}
	else
	{
		CMyComBSTR tempName(m_wzPassword);
		*password = tempName.Detach();
	}

	return S_OK;
	COM_TRY_END
}

// COpenCallbackImp
STDMETHODIMP COpenCallbackImp::SetTotal(const UInt64 *files, const UInt64 *bytes)
{
	COM_TRY_BEGIN
	UNUSED(files);
	UNUSED(bytes);
	return S_OK;
	COM_TRY_END
}

STDMETHODIMP COpenCallbackImp::SetCompleted(const UInt64 *files, const UInt64 *bytes)
{
	COM_TRY_BEGIN
	UNUSED(files);
	UNUSED(bytes);
	return S_OK;
	COM_TRY_END
}

STDMETHODIMP COpenCallbackImp::GetProperty(PROPID propID, PROPVARIANT *value)
{
	COM_TRY_BEGIN
	NCOM::CPropVariant propVariant;

	switch(propID)
	{
	case kpidName:
		propVariant = _fileInfo.Name;
		break;
	case kpidIsFolder:
		propVariant = _fileInfo.IsDirectory();
		break;
	case kpidSize:
		propVariant = _fileInfo.Size;
		break;
	case kpidAttributes:
		propVariant = (UInt32)_fileInfo.Attributes;
		break;
	case kpidLastAccessTime:
		propVariant = _fileInfo.LastAccessTime;
		break;
	case kpidCreationTime:
		propVariant = _fileInfo.CreationTime;
		break;
	case kpidLastWriteTime:
		propVariant = _fileInfo.LastWriteTime;
		break;
	}
	propVariant.Detach(value);
	return S_OK;
	COM_TRY_END
}

int COpenCallbackImp::FindName(const UString &name)
{
	for (int i = 0; i < FileNames.Size(); i++)
		if (name.CompareNoCase(FileNames[i]) == 0)
			return i;
	return -1;
}

struct CInFileStreamVol: public CInFileStream
{
	UString Name;
	COpenCallbackImp *OpenCallbackImp;
	CMyComPtr<IArchiveOpenCallback> OpenCallbackRef;
	~CInFileStreamVol()
	{
		int index = OpenCallbackImp->FindName(Name);
		if (index >= 0)
			OpenCallbackImp->FileNames.Delete(index);
	}
};

STDMETHODIMP COpenCallbackImp::GetStream(const wchar_t *name, IInStream **inStream)
{
	COM_TRY_BEGIN
	*inStream = NULL;
	UString fullPath = _folderPrefix + name;
	if (!NFile::NFind::FindFile(fullPath, _fileInfo))
		return S_FALSE;
	if (_fileInfo.IsDirectory())
		return S_FALSE;
	CInFileStreamVol *inFile = new CInFileStreamVol;
	CMyComPtr<IInStream> inStreamTemp = inFile;
	if (!inFile->Open(fullPath))
		return ::GetLastError();
	*inStream = inStreamTemp.Detach();
	inFile->Name = name;
	inFile->OpenCallbackImp = this;
	inFile->OpenCallbackRef = this;
	FileNames.Add(name);
	TotalSize += _fileInfo.Size;
	return S_OK;
	COM_TRY_END
}

#ifndef _NO_CRYPTO
STDMETHODIMP COpenCallbackImp::CryptoGetTextPassword(BSTR *password)
{
	COM_TRY_BEGIN
	UNUSED(password);
	return S_OK;
	COM_TRY_END
}
#endif

// CArchiveExtractCallback
void CArchiveExtractCallback::Init( IInArchive *archiveHandler, 
								   const UString &directoryPath,
								   const UString &itemDefaultName,
								   ZipOverwriteMode overwriteMode,
								   const UString &passwd,
								   funcZipPasswordCallback fPasswdCb,
								   LPVOID lpPasswdContxt,
								   funcZipOverWirtePromptCallback fOverwritePromptCb,
								   LPVOID lpOverwritePromptContxt,
								   funcZipProcessCallback fProcessCb,
								   LPVOID lpProcessContxt)
{
	_numErrors = 0;
	_curSize = 0;
	_unpTotal = 1;
	m_iOperationResult = 0;

	_itemDefaultName = itemDefaultName;
	_archiveHandler = archiveHandler;
	_directoryPath = directoryPath;
	NFile::NName::NormalizeDirPathPrefix(_directoryPath);

	_overwriteMode = overwriteMode;
	m_wzPassword = passwd;
	m_fZipPasswordCallback = fPasswdCb;
	m_lpPasswordContext = lpPasswdContxt;
	m_fZipOverWirtePromptCallback = fOverwritePromptCb;
	m_lpOverWirtePromptContext = lpOverwritePromptContxt;
	m_fZipProcessCallback = fProcessCb;
	m_lpProcessContxt = lpProcessContxt;
}

STDMETHODIMP CArchiveExtractCallback::SetTotal(UInt64 size)
{
	COM_TRY_BEGIN
	_unpTotal = size;
	return S_OK;
	COM_TRY_END
}

STDMETHODIMP CArchiveExtractCallback::SetCompleted(const UInt64 *completeValue)
{
	COM_TRY_BEGIN

	if (m_fZipProcessCallback)
	{
		return m_fZipProcessCallback(m_lpProcessContxt, *completeValue, _unpTotal);
	}

	return S_OK;
	COM_TRY_END
}

STDMETHODIMP CArchiveExtractCallback::SetRatioInfo(const UInt64 *inSize, const UInt64 *outSize)
{
	COM_TRY_BEGIN
		UNUSED(inSize);
		UNUSED(outSize);
		return S_OK;
	COM_TRY_END
}

void CArchiveExtractCallback::CreateComplexDirectory(const UStringVector &dirPathParts, UString &fullPath)
{
	fullPath = _directoryPath;
	for(int i = 0; i < dirPathParts.Size(); i++)
	{
		if (i > 0)
			fullPath += wchar_t(NFile::NName::kDirDelimiter);
		fullPath += dirPathParts[i];
		NFile::NDirectory::MyCreateDirectory(fullPath);
	}
}

static UString MakePathNameFromParts(const UStringVector &parts)
{
	UString result;
	for(int i = 0; i < parts.Size(); i++)
	{
		if(i != 0)
			result += wchar_t(NFile::NName::kDirDelimiter);
		result += parts[i];
	}
	return result;
}


HRESULT CArchiveExtractCallback::GetTime(int index, PROPID propID, FILETIME &filetime, bool &filetimeIsDefined)
{
	filetimeIsDefined = false;
	NCOM::CPropVariant prop;
	RINOK(_archiveHandler->GetProperty(index, propID, &prop));
	if (prop.vt == VT_FILETIME)
	{
		filetime = prop.filetime;
		filetimeIsDefined = (filetime.dwHighDateTime != 0 || filetime.dwLowDateTime != 0);
	}
	else if (prop.vt != VT_EMPTY)
		return E_FAIL;
	return S_OK;
}

STDMETHODIMP CArchiveExtractCallback::GetStream(UInt32 index, ISequentialOutStream **outStream, Int32 askExtractMode)
{
	COM_TRY_BEGIN
		*outStream = 0;
	_outFileStream.Release();

	_encrypted = false;
	_isSplit = false;
	_curSize = 0;

	UString fullPath;

	RINOK(GetArchiveItemPath(_archiveHandler, index, _itemDefaultName, fullPath));
	RINOK(IsArchiveItemFolder(_archiveHandler, index, _processedFileInfo.IsDirectory));

	_filePath = fullPath;

	{
		NCOM::CPropVariant prop;
		RINOK(_archiveHandler->GetProperty(index, kpidPosition, &prop));
		if (prop.vt != VT_EMPTY)
		{
			if (prop.vt != VT_UI8)
				return E_FAIL;
			_position = prop.uhVal.QuadPart;
			_isSplit = true;
		}
	}

	RINOK(IsArchiveItemProp(_archiveHandler, index, kpidEncrypted, _encrypted));

	bool newFileSizeDefined;
	UInt64 newFileSize;
	{
		NCOM::CPropVariant prop;
		RINOK(_archiveHandler->GetProperty(index, kpidSize, &prop));
		newFileSizeDefined = (prop.vt != VT_EMPTY);
		if (newFileSizeDefined)
		{
			newFileSize = ConvertPropVariantToUInt64(prop);
			_curSize = newFileSize;
		}
	}

	if(askExtractMode == NArchive::NExtract::NAskMode::kExtract)
	{
		{
			NCOM::CPropVariant prop;
			RINOK(_archiveHandler->GetProperty(index, kpidAttributes, &prop));
			if (prop.vt == VT_EMPTY)
			{
				_processedFileInfo.Attributes = 0;
				_processedFileInfo.AttributesAreDefined = false;
			}
			else
			{
				if (prop.vt != VT_UI4)
					return E_FAIL;
				_processedFileInfo.Attributes = prop.ulVal;
				_processedFileInfo.AttributesAreDefined = true;
			}
		}

		RINOK(GetTime(index, kpidCreationTime, _processedFileInfo.CreationTime,
			_processedFileInfo.IsCreationTimeDefined));
		RINOK(GetTime(index, kpidLastWriteTime, _processedFileInfo.LastWriteTime, 
			_processedFileInfo.IsLastWriteTimeDefined));
		RINOK(GetTime(index, kpidLastAccessTime, _processedFileInfo.LastAccessTime,
			_processedFileInfo.IsLastAccessTimeDefined));

		bool isAnti = false;
		RINOK(IsArchiveItemProp(_archiveHandler, index, kpidIsAnti, isAnti));

		UStringVector pathParts; 
		SplitPathToParts(fullPath, pathParts);

		if(pathParts.IsEmpty())
			return E_FAIL;

		MakeCorrectPath(pathParts);
		UString processedPath = MakePathNameFromParts(pathParts);
		if (!isAnti)
		{
			if (!_processedFileInfo.IsDirectory)
			{
				if (!pathParts.IsEmpty())
					pathParts.DeleteBack();
			}

			if (!pathParts.IsEmpty())
			{
				UString fullPathNew;
				CreateComplexDirectory(pathParts, fullPathNew);
				if (_processedFileInfo.IsDirectory)
					NFile::NDirectory::SetDirTime(fullPathNew, 
					(WriteCreated && _processedFileInfo.IsCreationTimeDefined) ? &_processedFileInfo.CreationTime : NULL, 
					(WriteAccessed && _processedFileInfo.IsLastAccessTimeDefined) ? &_processedFileInfo.LastAccessTime : NULL, 
					(WriteModified && _processedFileInfo.IsLastWriteTimeDefined) ? &_processedFileInfo.LastWriteTime : NULL);
			}
		}

		UString fullProcessedPath = _directoryPath + processedPath;

		if(_processedFileInfo.IsDirectory)
		{
			_diskFilePath = fullProcessedPath;
			if (isAnti)
				NFile::NDirectory::MyRemoveDirectory(_diskFilePath);
			return S_OK;
		}

		if (!_isSplit)
		{
			NFile::NFind::CFileInfoW fileInfo;
			if(NFile::NFind::FindFile(fullProcessedPath, fileInfo))
			{
				switch(_overwriteMode)
				{
				case ZIP_OVERWRITE_MODE_SkipExisting:
					return S_OK;
				case ZIP_OVERWRITE_MODE_AskBefore:
					{
						ZipOverwriteAnswer overwiteResult = ZIP_OVERWRITE_ANSWER_Yes;
						if (!m_fZipOverWirtePromptCallback)
						{
							m_fZipOverWirtePromptCallback(m_lpOverWirtePromptContext, &overwiteResult);
						}

						switch(overwiteResult)
						{
							case ZIP_OVERWRITE_ANSWER_Cancel:
								return E_ABORT;
							case ZIP_OVERWRITE_ANSWER_No:
								return S_OK;
							case ZIP_OVERWRITE_ANSWER_NoToAll:
								_overwriteMode = ZIP_OVERWRITE_MODE_SkipExisting;
								return S_OK;
							case ZIP_OVERWRITE_ANSWER_YesToAll:
								_overwriteMode = ZIP_OVERWRITE_MODE_WithoutPrompt;
								break;
							case ZIP_OVERWRITE_ANSWER_Yes:
								break;
							case ZIP_OVERWRITE_ANSWER_AutoRename:
								_overwriteMode = ZIP_OVERWRITE_MODE_AutoRename;
								break;
							default:
								return E_FAIL;
						}
					}
				}
				if (_overwriteMode == ZIP_OVERWRITE_MODE_AutoRename)
				{
					if (!AutoRenamePath(fullProcessedPath))
					{
						DPW((L"Can't auto rename file %s!\n", fullProcessedPath));
						return E_FAIL;
					}
				}
				else if (_overwriteMode == ZIP_OVERWRITE_MODE_AutoRenameExisting)
				{
					UString existPath = fullProcessedPath;
					if (!AutoRenamePath(existPath))
					{
						DPW((L"Can't auto rename file %s!\n", fullProcessedPath));
						return E_FAIL;
					}
					if(!NFile::NDirectory::MyMoveFile(fullProcessedPath, existPath))
					{
						DPW((L"Can't auto rename file %s!\n", fullProcessedPath));
						return E_FAIL;
					}
				}
				else if (!NFile::NDirectory::DeleteFileAlways(fullProcessedPath))
				{
					DPW((L"Can't delete file %s!\n", fullProcessedPath));
					return S_OK;
				}
			}
		}
		if (!isAnti)
		{
			_outFileStreamSpec = new COutFileStream;
			CMyComPtr<ISequentialOutStream> outStreamLoc(_outFileStreamSpec);
			if (!_outFileStreamSpec->Open(fullProcessedPath, _isSplit ? OPEN_ALWAYS: CREATE_ALWAYS))
			{
				{
					DPW((L"Can not open output file %s!\n", fullProcessedPath));
					
					return S_OK;
				}
			}
			if (_isSplit)
			{
				RINOK(_outFileStreamSpec->Seek(_position, STREAM_SEEK_SET, NULL));
			}
			_outFileStream = outStreamLoc;
			*outStream = outStreamLoc.Detach();
		}
		_diskFilePath = fullProcessedPath;
	}
	else
	{
		*outStream = NULL;
	}
	return S_OK;
	COM_TRY_END
}

STDMETHODIMP CArchiveExtractCallback::PrepareOperation(Int32 askExtractMode)
{
	COM_TRY_BEGIN
	_extractMode = false;
	switch (askExtractMode)
	{
	case NArchive::NExtract::NAskMode::kExtract:
		_extractMode = true;
	};

	return S_OK;
	COM_TRY_END
}

STDMETHODIMP CArchiveExtractCallback::SetOperationResult(Int32 operationResult)
{
	COM_TRY_BEGIN

	m_iOperationResult = operationResult;

	switch(operationResult)
	{
		case NArchive::NExtract::NOperationResult::kOK:
			break;
		case NArchive::NExtract::NOperationResult::kUnSupportedMethod:
		case NArchive::NExtract::NOperationResult::kCRCError:
		case NArchive::NExtract::NOperationResult::kDataError:
		default:
			_outFileStream.Release();
			return E_FAIL;
	}
	if (_outFileStream != NULL)
	{
		_outFileStreamSpec->SetTime(
			(WriteCreated && _processedFileInfo.IsCreationTimeDefined) ? &_processedFileInfo.CreationTime : NULL, 
			(WriteAccessed && _processedFileInfo.IsLastAccessTimeDefined) ? &_processedFileInfo.LastAccessTime : NULL, 
			(WriteModified && _processedFileInfo.IsLastWriteTimeDefined) ? &_processedFileInfo.LastWriteTime : NULL);
		_curSize = _outFileStreamSpec->ProcessedSize;
		RINOK(_outFileStreamSpec->Close());
		_outFileStream.Release();
	}
	UnpackSize += _curSize;
	if (_processedFileInfo.IsDirectory)
		NumFolders++;
	else
		NumFiles++;

	if (_extractMode && _processedFileInfo.AttributesAreDefined)
		NFile::NDirectory::MySetFileAttributes(_diskFilePath, _processedFileInfo.Attributes);
	return S_OK;
	COM_TRY_END
}


STDMETHODIMP CArchiveExtractCallback::CryptoGetTextPassword(BSTR *password)
{
	COM_TRY_BEGIN
	if (m_wzPassword)
	{
		CMyComBSTR tempName(m_wzPassword);
		*password = tempName.Detach();
		return S_OK;
	}

	if (m_fZipPasswordCallback)
	{
		return m_fZipPasswordCallback(m_lpPasswordContext, password);
	}
	return S_OK;
	COM_TRY_END
}

// functions

HRESULT GetArchiveItemPath(IInArchive *archive, UInt32 index, UString &result)
{
	NCOM::CPropVariant prop;
	RINOK(archive->GetProperty(index, kpidPath, &prop));
	if(prop.vt == VT_BSTR)
		result = prop.bstrVal;
	else if (prop.vt == VT_EMPTY)
		result.Empty();
	else
		return E_FAIL;
	return S_OK;
}

HRESULT GetArchiveItemPath(IInArchive *archive, UInt32 index, const UString &defaultName, UString &result)
{
	RINOK(GetArchiveItemPath(archive, index, result));
	if (result.IsEmpty())
	{
		result = defaultName;
		NCOM::CPropVariant prop;
		RINOK(archive->GetProperty(index, kpidExtension, &prop));
		if (prop.vt == VT_BSTR)
		{
			result += L'.';
			result += prop.bstrVal;
		}
		else if (prop.vt != VT_EMPTY)
			return E_FAIL;
	}
	return S_OK;
}

HRESULT GetArchiveItemFileTime(IInArchive *archive, UInt32 index, 
							   const FILETIME &defaultFileTime, FILETIME &fileTime)
{
	NCOM::CPropVariant prop;
	RINOK(archive->GetProperty(index, kpidLastWriteTime, &prop));
	if (prop.vt == VT_FILETIME)
		fileTime = prop.filetime;
	else if (prop.vt == VT_EMPTY)
		fileTime = defaultFileTime;
	else
		return E_FAIL;
	return S_OK;
}

HRESULT IsArchiveItemProp(IInArchive *archive, UInt32 index, PROPID propID, bool &result)
{
	NCOM::CPropVariant prop;
	RINOK(archive->GetProperty(index, propID, &prop));
	if(prop.vt == VT_BOOL)
		result = VARIANT_BOOLToBool(prop.boolVal);
	else if (prop.vt == VT_EMPTY)
		result = false;
	else
		return E_FAIL;
	return S_OK;
}

HRESULT IsArchiveItemFolder(IInArchive *archive, UInt32 index, bool &result)
{
	return IsArchiveItemProp(archive, index, kpidIsFolder, result);
}