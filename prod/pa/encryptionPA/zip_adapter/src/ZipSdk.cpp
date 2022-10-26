/*
Written by Derek Zheng
June 2008
*/

#pragma once

#include "stdafx.h"

#include "log.h"

#include "Common/MyInitGuid.h"
#include "Windows/PropVariant.h"
#include "Windows/FileFind.h"
#include "Windows/FileDir.h"
#include "Common/MyString.h"
#include "Common/StringToInt.h"
#include "Common/MyCom.h"
#include "Common/Wildcard.h"
#include "7zip/Archive/IArchive.h"
#include "7zip/Common/FileStreams.h"
#include "7zip/Common/ProgressUtils.h"
#include "7zip/IPassword.h"
#include "7zip/ICoder.h"

#include "EnumDirItems.h"
#include "ZipSdk.h"
#include "7ZipCallback.h"

using namespace NWindows;
using namespace NCOM;
using namespace NFile;

typedef HRESULT (*funcCreateObject)(const GUID *clsid, const GUID *iid, void **outObject);
typedef HRESULT (*funcGetNumberOfFormats)(UINT32 *numFormats);

DEFINE_GUID(CLSID_CArchiveHandler, 
			0x23170F69, 0x40C1, 0x278A, 0x10, 0x00, 0x00, 0x01, 0x10, 0x00, 0x00, 0x00);

#define CLS_ARC_ID_ITEM(cls) ((cls).Data4[5])

// Static-SFX (for Linux) can be big.
const UInt64 kMaxCheckStartPosition = 1 << 22;

HINSTANCE g_h7ZipDll = NULL;
funcCreateObject g_fCreateObject = NULL;
funcGetNumberOfFormats g_fGetNumberOfFormats = NULL;

static CZipSdk *s_pZipSdk = NULL;

static LPCWSTR s_wstrZipCipherAlgo[ZIP_CIPHER_ALGO_MAX] = {
	L"ZIPCRYPTO",
	L"AES128",
	L"AES192",
	L"AES256"
};

static ZipCompressFormat ConvertFileExt2ZipCompFormat(LPWSTR lpwzExt);

GsmErrorT CZipSdk::Init( void )
{
	return GSM_ERR_NO_ERROR;
}

void CZipSdk::Release( void )
{
}

GsmErrorT CZipSdk::CompressFile(ZipCompressParam &param)
{
	CMyComPtr<IOutArchive> outArchive;
	CMyComPtr<ISetProperties> setProperties;
	GUID cls = CLSID_CArchiveHandler;
	HRESULT hr = S_OK;
	UINT32 uiNumFormats = 0;
	NWildcard::CCensor wildcardCensor;
	StringVector::iterator iterFile;
	IEnumDirItemCallback enumCallback;
	UStringVector errorPaths;
	CRecordVector<DWORD> errorCodes;
	CObjectVector<CDirItem> dirItems;

	CLS_ARC_ID_ITEM(cls) = (unsigned char)param.eFormat;

	if (!g_fGetNumberOfFormats || !g_fCreateObject)
	{
		DPW((L"7z_archive.dll is not loaded!\n"));
		return GSM_ERR_NO_DLL;
	}

	hr = g_fGetNumberOfFormats(&uiNumFormats);

	hr = g_fCreateObject(&cls, &IID_IOutArchive, (void **)&outArchive);
	if (FAILED(hr))
	{
		DPW((L"Format %d isn't supported!\n", param.eFormat));
		return GSM_ERR_ZIP_UNSUPPORTEDFORMAT;
	}

	hr = outArchive->QueryInterface(IID_ISetProperties, (void **)&setProperties);
	if (FAILED(hr))
	{
		DPW((L"Format %d doesn't support ISetProperties interface!\n", param.eFormat));
		return GSM_ERR_ZIP_UNSUPPORTEDFORMAT;
	}

	const UString archivePath(param.wstrArchive.c_str());

	if (NFind::DoesFileExist(archivePath))
	{
		DPW((L"Archive file %s already exists!\n", param.wstrArchive.c_str()));
		return GSM_ERR_FILE_EXISTS;
	}

	CMyComPtr<ISequentialOutStream> seqOutStream;
	COutFileStream *outStreamSpec = NULL;

	UString resultPath;
	int pos;
	NFile::NDirectory::MyGetFullPathName(archivePath, resultPath, pos);
	NFile::NDirectory::CreateComplexDirectory(resultPath.Left(pos));

	outStreamSpec = new COutFileStream;
	seqOutStream = outStreamSpec;

	if (!outStreamSpec->Create(resultPath, false))
	{
		DPW((L"Archive file %s already exists!\n", param.wstrArchive.c_str()));
		return GSM_ERR_FILE_EXISTS;
	}

	for (iterFile = param.vecFiles.begin(); iterFile != param.vecFiles.end(); iterFile++)
	{
		wildcardCensor.AddItem(true, UString((*iterFile).c_str()), false);
	}

	hr = EnumerateItems(wildcardCensor, dirItems, &enumCallback, errorPaths, errorCodes);
	if (FAILED(hr))
	{
		if (outStreamSpec)
			outStreamSpec->Close();

		DPW((L"Enumerate source files failed!\n"));
		return GSM_ERR_ZIP_GENERAL;
	}

	if (param.eFormat == ZIP_COMPRESS_FORMAT_ZIP)
	{
		CRecordVector<const wchar_t *> names;
		CPropVariant props[2];
		names.Add(L"X");
		names.Add(L"EMethod");
		props[0] = (UInt32)param.eLevel;
		props[1] = (LPCOLESTR)s_wstrZipCipherAlgo[param.eAlgo];

		setProperties->SetProperties(&names.Front(), props, 2);
	}

	CArchiveUpdateCallback *updateCallbackSpec = new CArchiveUpdateCallback;
	CMyComPtr<IArchiveUpdateCallback> updateCallback(updateCallbackSpec);

	updateCallbackSpec->m_bDoEncrypt = (bool)param.bEncrypt;
	updateCallbackSpec->m_wzPassword = param.wzPassword;
	updateCallbackSpec->m_fZipPasswordCallback = param.fZipPasswordCb;
	updateCallbackSpec->m_lpPasswordContext = param.lpPasswdContxt;
	updateCallbackSpec->m_pDirItems = &dirItems;
	updateCallbackSpec->m_fZipProcessCallback = param.fZipProcessCallback;
	updateCallbackSpec->m_lpProcessContext = param.lpProcessContxt;

	hr = outArchive->UpdateItems(seqOutStream, dirItems.Size(), updateCallback);
	if (FAILED(hr))
	{
		if (outStreamSpec)
			outStreamSpec->Close();

		DPW((L"Create output archive failed!(format=%d, result=0x%x)", param.eFormat, hr));
		return GSM_ERR_ZIP_GENERAL;
	}

	/* Add comments to the zip file */
	if (!param.wstrComments.empty() && param.eFormat == ZIP_COMPRESS_FORMAT_ZIP)
	{
		CMyComPtr<IOutStream> iOutStream;
		hr = seqOutStream->QueryInterface(IID_IOutStream, (void **)&iOutStream);
		if (SUCCEEDED(hr) && iOutStream)
		{
			UInt16 uiCommentsLen = (UInt16)param.wstrComments.size();
			UInt32 proceededSize = 0;
			unsigned char szComments[1024];

			WideCharToMultiByte(CP_ACP, 0, param.wstrComments.c_str(), -1, (LPSTR)szComments, 1024, NULL, NULL);
			iOutStream->Seek(0, STREAM_SEEK_END, NULL);
			iOutStream->Seek(-2, STREAM_SEEK_CUR, NULL);
			iOutStream->Write((void *)&uiCommentsLen, sizeof(UInt16), &proceededSize);
			iOutStream->Write((void *)szComments, uiCommentsLen, &proceededSize);
		}
	}

	return GSM_ERR_NO_ERROR;
}

GsmErrorT CZipSdk::UnCompressFile(ZipUnCompressParam &param)
{
	CMyComPtr<IInArchive> inArchive;
	GUID cls = CLSID_CArchiveHandler;
	HRESULT hr = S_OK;
	UINT32 uiNumFormats = 0;

	if (!g_fGetNumberOfFormats || !g_fCreateObject)
	{
		DPW((L"7z_archive.dll is not loaded!\n"));
		return GSM_ERR_NO_DLL;
	}

	if (param.eFormat)
	{
		CLS_ARC_ID_ITEM(cls) = (unsigned char)param.eFormat;
	}
	else
	{
		LPWSTR lpwzExt = (LPWSTR)(param.wstrArchive.c_str() + param.wstrArchive.size() - 4);
		ZipCompressFormat format;
		format = ConvertFileExt2ZipCompFormat(lpwzExt);
		if (format == ZIP_COMPRESS_FORMAT_UNKNOWN)
		{
			DPW((L"File extend %%s isn't supported!\n", lpwzExt));
			return GSM_ERR_UNZIP_UNSUPPORTEDFORMAT;
		}

		CLS_ARC_ID_ITEM(cls) = (unsigned char)format;
	}

	hr = g_fGetNumberOfFormats(&uiNumFormats);

	hr = g_fCreateObject(&cls, &IID_IInArchive, (void **)&inArchive);
	if (FAILED(hr))
	{
		DPW((L"Format %d isn't supported!\n", param.eFormat));
		return GSM_ERR_UNZIP_UNSUPPORTEDFORMAT;
	}


	CInFileStream *inStreamSpec = new CInFileStream;
	CMyComPtr<IInStream> inStream(inStreamSpec);
	if (!inStreamSpec->Open(param.wstrArchive.c_str()))
	{
		DPW((L"Open archive %s failed(err=0x%x)!\n", param.wstrArchive.c_str(), GetLastError()));
		return GSM_ERR_FILE_NOT_FOUND;
	}

	COpenCallbackImp *openCallbackSpec = new COpenCallbackImp;
	CMyComPtr<IArchiveOpenCallback> openCallback = openCallbackSpec;
	hr = inArchive->Open(inStream, &kMaxCheckStartPosition, openCallback);
	if (FAILED(hr))
	{
		DPW((L"Open archive %s failed!\n", param.wstrArchive.c_str()));
		return GSM_ERR_UNZIP_UNSUPPORTEDFORMAT;
	}

	CPropVariant prop;
	hr = inArchive->GetArchiveProperty(kpidComment, &prop);
	if (SUCCEEDED(hr))
	{
		if (prop.vt == VT_BSTR)
		{
			param.wstrComments = prop.bstrVal;
		}
	}

	UInt32 numItems;
	CRecordVector<UInt32> realIndices;
	hr = inArchive->GetNumberOfItems(&numItems);
	if (FAILED(hr))
	{
		DPW((L"There is no item in archive %s!\n", param.wstrArchive.c_str()));
		return GSM_ERR_UNZIP_UNSUPPORTEDFORMAT;
	}

	for(UInt32 i = 0; i < numItems; i++)
	{
		UString filePath;
		GetArchiveItemPath(inArchive, i, UString(param.wstrArchive.c_str()), filePath);
		bool isFolder;
		IsArchiveItemFolder(inArchive, i, isFolder);
		realIndices.Add(i);
	}
	if (realIndices.Size() == 0)
	{
		return GSM_ERR_NO_ERROR;
	}

	UString archiveFileName = ExtractFileNameFromPath(param.wstrArchive.c_str());

	int dotPos = archiveFileName.ReverseFind(L'.');
	UString defaultName = archiveFileName;
	if (dotPos > 0)
	{
		defaultName = archiveFileName.Left(dotPos);
	}

	UString outDir = param.wstrDstFolder.c_str();
	outDir.Replace(L"*", defaultName);
	if(!outDir.IsEmpty())
		if(!NFile::NDirectory::CreateComplexDirectory(outDir))
		{
			DPW((L"Can not create output directory %s!\n", outDir));
			return GSM_ERR_UNZIP_GENERAL;
		}

	CArchiveExtractCallback *extractCallbackSpec = new CArchiveExtractCallback;
	CMyComPtr<IArchiveExtractCallback> ec(extractCallbackSpec);

	extractCallbackSpec->Init(inArchive, outDir, defaultName, param.eOverwriteMode, param.wstrPassword.c_str(),
		param.fZipPasswordCb, param.lpPasswdContxt, param.fZipOverWirtePromptCallback,
		param.lpOverWirtePromptContext, param.fZipProcessCallback, param.lpProcessContxt);

	hr = inArchive->Extract(&realIndices.Front(), 
		realIndices.Size(), 0, extractCallbackSpec);
	switch (extractCallbackSpec->m_iOperationResult)
	{
	case NArchive::NExtract::NOperationResult::kOK:
		break;
	case NArchive::NExtract::NOperationResult::kUnSupportedMethod:
		return GSM_ERR_UNZIP_UNSUPPORTEDFORMAT;
	case NArchive::NExtract::NOperationResult::kCRCError:
		return GSM_ERR_UNZIP_CRC;
	case NArchive::NExtract::NOperationResult::kDataError:
		return GSM_ERR_UNZIP_DATA;
	default:
		return GSM_ERR_UNZIP_GENERAL;
	}

	if (FAILED(hr))
	{
		return GSM_ERR_UNZIP_GENERAL;
	}

	return GSM_ERR_NO_ERROR;
}

GsmErrorT CZipSdk::ReadComments(std::wstring &wstrArchive, std::wstring &wstrComments)
{
	CMyComPtr<IInArchive> inArchive;
	GUID cls = CLSID_CArchiveHandler;
	HRESULT hr = S_OK;

	wstrComments = L"";

	if (!g_fGetNumberOfFormats || !g_fCreateObject)
	{
		DPW((L"7z_archive.dll is not loaded!\n"));
		return GSM_ERR_NO_DLL;
	}

	LPWSTR lpwzExt = (LPWSTR)(wstrArchive.c_str() + wcslen(wstrArchive.c_str()) - 4);
	ZipCompressFormat format;
	format = ConvertFileExt2ZipCompFormat(lpwzExt);
	if (format != ZIP_COMPRESS_FORMAT_ZIP)
	{
		DPW((L"Only zip file contains comments section!\n"));
		return GSM_ERR_ZIP_UNSUPPORTEDFORMAT;
	}
	
	CLS_ARC_ID_ITEM(cls) = (unsigned char)format;

	hr = g_fCreateObject(&cls, &IID_IInArchive, (void **)&inArchive);
	if (FAILED(hr))
	{
		DPW((L"Format %d isn't supported!\n", format));
		return GSM_ERR_ZIP_UNSUPPORTEDFORMAT;
	}

	CInFileStream *inStreamSpec = new CInFileStream;
	CMyComPtr<IInStream> inStream(inStreamSpec);
	if (!inStreamSpec->Open(wstrArchive.c_str()))
	{
		DPW((L"Open archive %s failed(err=0x%x)!\n", wstrArchive.c_str(), GetLastError()));
		return GSM_ERR_FILE_NOT_FOUND;
	}

	COpenCallbackImp *openCallbackSpec = new COpenCallbackImp;
	CMyComPtr<IArchiveOpenCallback> openCallback = openCallbackSpec;
	hr = inArchive->Open(inStream, &kMaxCheckStartPosition, openCallback);
	if (FAILED(hr))
	{
		DPW((L"Open archive %s failed!\n", wstrArchive.c_str()));
		return GSM_ERR_ZIP_UNSUPPORTEDFORMAT;
	}

	CPropVariant prop;
	hr = inArchive->GetArchiveProperty(kpidComment, &prop);
	if (SUCCEEDED(hr))
	{
		if (prop.vt == VT_BSTR)
		{
			wstrComments = prop.bstrVal;
		}
	}

	return GSM_ERR_NO_ERROR;
}

GsmErrorT CZipSdk::WriteComments(std::wstring &wstrArchive, std::wstring &wstrComments)
{
	GsmErrorT err = GSM_ERR_NO_ERROR;
	std::wstring wstrOriginalComments = L"";

	err = ReadComments(wstrArchive, wstrOriginalComments);
	if (err != GSM_ERR_NO_ERROR)
	{
		return err;
	}

	LONG lSeekPos = -2 - (LONG)wstrOriginalComments.size();
	DWORD len = (DWORD)wstrComments.size();
	DWORD dwWritten = 0;
	HANDLE hFile = ::CreateFileW(wstrArchive.c_str(), GENERIC_WRITE, FILE_SHARE_READ, NULL, 
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	::SetFilePointer(hFile, 0, NULL, FILE_END);
	::SetFilePointer(hFile, lSeekPos, NULL, FILE_CURRENT);
	::WriteFile(hFile, &len, sizeof(short), &dwWritten, NULL);
	if (len)
	{
		unsigned char szComments[1024];

		WideCharToMultiByte(CP_ACP, 0, wstrComments.c_str(), -1, (LPSTR)szComments, 1024, NULL, NULL);
		::WriteFile(hFile, szComments, len, &dwWritten, NULL);
	}
	CloseHandle(hFile);

	return GSM_ERR_NO_ERROR;
}

#if 0
GSMSDK_API CZipSdk * CreateZipSdkInstance( void )
{
	CZipSdk *pZipSdk = new CZipSdk;

	return pZipSdk;
}

GSMSDK_API void DestroyZipSdkInstance( CZipSdk *pZipSdk )
{
	if (pZipSdk)
	{
		delete pZipSdk;
		pZipSdk = NULL;
	}
}
#endif

static ZipCompressFormat ConvertFileExt2ZipCompFormat(LPWSTR lpwzExt)
{
	if (!_wcsicmp(lpwzExt, L".zip"))
	{
		return ZIP_COMPRESS_FORMAT_ZIP;
	}
	else if (!_wcsicmp(lpwzExt, L".bz2") || !_wcsicmp(lpwzExt, L"bzip2")
		|| !_wcsicmp(lpwzExt, L"tbz2") || !_wcsicmp(lpwzExt, L"tbz"))
	{
		return ZIP_COMPRESS_FORMAT_BZIP2;
	}
	else if (!_wcsicmp(lpwzExt, L".rar"))
	{
		return ZIP_COMPRESS_FORMAT_RAR;
	}
	else if (!_wcsicmp(lpwzExt, L".arj"))
	{
		return ZIP_COMPRESS_FORMAT_ARJ;
	}
	else if (!_wcsicmp(lpwzExt+2, L".z") || !_wcsicmp(lpwzExt, L".taz"))
	{
		return ZIP_COMPRESS_FORMAT_Z;
	}
	else if (!_wcsicmp(lpwzExt+1, L".7z"))
	{
		return ZIP_COMPRESS_FORMAT_7Z;
	}
	else if (!_wcsicmp(lpwzExt, L".cab"))
	{
		return ZIP_COMPRESS_FORMAT_CAB;
	}
	else if (!_wcsicmp(lpwzExt, L".chm"))
	{
		return ZIP_COMPRESS_FORMAT_CHM;
	}
	else if (!_wcsicmp(lpwzExt, L".msi") || !_wcsicmp(lpwzExt, L".doc")
		|| !_wcsicmp(lpwzExt, L".xls") || !_wcsicmp(lpwzExt, L".ppt"))
	{
		return ZIP_COMPRESS_FORMAT_COM;
	}
	else if (!_wcsicmp(lpwzExt, L"cpio"))
	{
		return ZIP_COMPRESS_FORMAT_CPIO;
	}
	else if (!_wcsicmp(lpwzExt, L".deb"))
	{
		return ZIP_COMPRESS_FORMAT_DEB;
	}
	else if (!_wcsicmp(lpwzExt+1, L".gz") || !_wcsicmp(lpwzExt, L"gzip")
		|| !_wcsicmp(lpwzExt, L".tgz") || !_wcsicmp(lpwzExt, L".tpz"))
	{
		return ZIP_COMPRESS_FORMAT_GZIP;
	}
	else if (!_wcsicmp(lpwzExt, L".iso"))
	{
		return ZIP_COMPRESS_FORMAT_ISO;
	}
	else if (!_wcsicmp(lpwzExt, L".lzh") || !_wcsicmp(lpwzExt, L".lha"))
	{
		return ZIP_COMPRESS_FORMAT_LZH;
	}
	else if (!_wcsicmp(lpwzExt, L"lzma"))
	{
		return ZIP_COMPRESS_FORMAT_LZMA;
	}
	else if (!_wcsicmp(lpwzExt, L".rar") || !_wcsicmp(lpwzExt, L".r00"))
	{
		return ZIP_COMPRESS_FORMAT_RAR;
	}
	else if (!_wcsicmp(lpwzExt, L".rpm"))
	{
		return ZIP_COMPRESS_FORMAT_RPM;
	}
	else if (!_wcsicmp(lpwzExt, L".001"))
	{
		return ZIP_COMPRESS_FORMAT_SPLIT;
	}
	else if (!_wcsicmp(lpwzExt, L".tar"))
	{
		return ZIP_COMPRESS_FORMAT_TAR;
	}
	else if (!_wcsicmp(lpwzExt, L".wim") || !_wcsicmp(lpwzExt, L".swm"))
	{
		return ZIP_COMPRESS_FORMAT_WIM;
	}
	else
	{
		return ZIP_COMPRESS_FORMAT_UNKNOWN;
	}
}

