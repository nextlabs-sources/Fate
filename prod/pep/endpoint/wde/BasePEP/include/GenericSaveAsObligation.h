#pragma once
#ifdef _WIN32_WINNT
#undef _WIN32_WINNT
#endif
#define _WIN32_WINNT 0x0600
#include <windows.h>
#include <Commdlg.h>
#include <string>
#include <map>
#include "cesdk.h"
#include "eframework/platform/cesdk_obligations.hpp"
#include "PAMngr.h"
#include "nlconfig.hpp"
#include "resattrlib.h"
#include "resattrmgr.h"
#include "commonutils.hpp"

namespace nextlabs
{

class GenericSaveAsObligation
{
public:
	virtual BOOL Prepare ( const std::wstring& SrcFileName, const std::wstring& DstFileName, const nextlabs::Obligations& obs, const LPOPENFILENAMEW lpofn ) = 0;
	virtual BOOL Prepare ( const std::wstring& SrcFileName, const std::wstring& DstFileName, const nextlabs::Obligations& obs, void* pIFileSaveDialog ) = 0;
	virtual BOOL DoCreateFileA ( HANDLE& hFile, LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, const LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile ) = 0;
	virtual BOOL DoCreateFileW ( HANDLE& hFile, LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, const LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile ) = 0;
	virtual BOOL DoCopyFileW ( BOOL bRet, LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName ) = 0;
	virtual BOOL DoMoveFileW ( BOOL bRet, LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName ) = 0;

public:
	BOOL SetValid ( BOOL bStatus );

public:
	static GenericSaveAsObligation* GetInstance();

	virtual ~GenericSaveAsObligation ( );	

private:
	typedef int (*CreateAttributeManagerType)(ResourceAttributeManager **mgr);
	typedef int (*AllocAttributesType)(ResourceAttributes **attrs);
	typedef int (*ReadResourceAttributesWType)(ResourceAttributeManager *mgr, const WCHAR *filename, ResourceAttributes *attrs);
	typedef int (*GetAttributeCountType)(const ResourceAttributes *attrs);
	typedef void (*FreeAttributesType)(ResourceAttributes *attrs);
	typedef void (*CloseAttributeManagerType)(ResourceAttributeManager *mgr);
	typedef void (*AddAttributeWType)(ResourceAttributes *attrs, const WCHAR *name, const WCHAR *value);
	typedef const WCHAR *(*GetAttributeNameType)(const ResourceAttributes *attrs, int index);
	typedef const WCHAR * (*GetAttributeValueType)(const ResourceAttributes *attrs, int index);
	typedef int (*WriteResourceAttributesWType)(ResourceAttributeManager *mgr, const WCHAR *filename, ResourceAttributes *attrs);

	static CreateAttributeManagerType lfCreateAttributeManager;
	static AllocAttributesType lfAllocAttributes;
	static ReadResourceAttributesWType lfReadResourceAttributesW;
	static GetAttributeCountType lfGetAttributeCount;
	static FreeAttributesType lfFreeAttributes;
	static CloseAttributeManagerType lfCloseAttributeManager;
	static AddAttributeWType lfAddAttributeW;
	static GetAttributeNameType lfGetAttributeName;
	static GetAttributeValueType lfGetAttributeValue;
	static WriteResourceAttributesWType lfWriteResourceAttributesW;

protected:
	static bool InitResattr();
	static BOOL GetFileTags ( LPCWSTR lpszFilePath, std::map<std::wstring, std::wstring>& mapTags );
	static BOOL SetFileTags ( LPCWSTR lpszFilePath, std::map<std::wstring, std::wstring>& mapTags );

protected:
	GenericSaveAsObligation ( );

	CRITICAL_SECTION CriticalSection;
 	std::wstring SourceFile;
 	std::wstring DestinationFile;
 	nextlabs::Obligations obs;
 	BOOL bValid;
	DWORD dwThreadID;
	std::map<std::wstring, std::wstring> SourceFileTags;

private:
	GenericSaveAsObligation ( const GenericSaveAsObligation& );
	GenericSaveAsObligation& operator= ( const GenericSaveAsObligation& );

};

class NullSaveAsObligation : public GenericSaveAsObligation
{
public:
	BOOL Prepare ( const std::wstring& SrcFileName, const std::wstring& DstFileName, const nextlabs::Obligations& obs, const LPOPENFILENAMEW lpofn );
	BOOL Prepare ( const std::wstring& SrcFileName, const std::wstring& DstFileName, const nextlabs::Obligations& obs, void* pIFileSaveDialog );
	BOOL DoCreateFileA ( HANDLE& hFile, LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, const LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile );
	BOOL DoCreateFileW ( HANDLE& hFile, LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, const LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile );
	BOOL DoCopyFileW ( BOOL bRet, LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName );
	BOOL DoMoveFileW ( BOOL bRet, LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName );

public:
	NullSaveAsObligation ( );

	virtual ~NullSaveAsObligation ( );

private:
	NullSaveAsObligation ( const NullSaveAsObligation& );
	NullSaveAsObligation& operator= ( const NullSaveAsObligation& );

};

class CommonSaveAsObligation : public GenericSaveAsObligation
{
public:
	BOOL Prepare ( const std::wstring& SrcFileName, const std::wstring& DstFileName, const nextlabs::Obligations& obs, const LPOPENFILENAMEW lpofn );
	BOOL Prepare ( const std::wstring& SrcFileName, const std::wstring& DstFileName, const nextlabs::Obligations& obs, void* pIFileSaveDialog );
	BOOL DoCreateFileA ( HANDLE& hFile, LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, const LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile );
	BOOL DoCreateFileW ( HANDLE& hFile, LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, const LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile );
	BOOL DoCopyFileW ( BOOL bRet, LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName );
	BOOL DoMoveFileW ( BOOL bRet, LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName );

public:
	CommonSaveAsObligation ( );

	virtual ~CommonSaveAsObligation ( );

private:
	CommonSaveAsObligation ( const CommonSaveAsObligation& );
	CommonSaveAsObligation& operator= ( const CommonSaveAsObligation& );

};

class MSPaintSaveAsObligation : public CommonSaveAsObligation
{
public:
	BOOL Prepare ( const std::wstring& SrcFileName, const std::wstring& DstFileName, const nextlabs::Obligations& obs, const LPOPENFILENAMEW lpofn );
	BOOL Prepare ( const std::wstring& SrcFileName, const std::wstring& DstFileName, const nextlabs::Obligations& obs, void* pIFileSaveDialog );
	BOOL DoCreateFileW ( HANDLE& hFile, LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, const LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile );
	BOOL DoCopyFileW ( BOOL bRet, LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName );

public:
	MSPaintSaveAsObligation ( );

	virtual ~MSPaintSaveAsObligation ( );

private:
	int OSVersion;
	BOOL bCreateFile;
	BOOL bCopyFile;

private:
	MSPaintSaveAsObligation ( const MSPaintSaveAsObligation& );
	MSPaintSaveAsObligation& operator= ( const MSPaintSaveAsObligation& );

};

class ChromeSaveAsObligation : public CommonSaveAsObligation
{
public:
	BOOL DoCreateFileA ( HANDLE& hFile, LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, const LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile );
	BOOL DoCreateFileW ( HANDLE& hFile, LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, const LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile );
	BOOL DoMoveFileW ( BOOL bRet, LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName );

public:
	ChromeSaveAsObligation ( );

	virtual ~ChromeSaveAsObligation ( );

private:
	ChromeSaveAsObligation ( const ChromeSaveAsObligation& );
	ChromeSaveAsObligation& operator= ( const ChromeSaveAsObligation& );

};

}