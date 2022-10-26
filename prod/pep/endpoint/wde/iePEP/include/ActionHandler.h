#pragma once

#include <windows.h>
#include <stdlib.h>
#include <winsock2.h>
#include <Commdlg.h>
#include <string>
#include "eframework/policy/comm_helper.hpp"

#pragma warning( push )
#pragma warning( disable : 6011 )
#  include <boost/algorithm/string.hpp>
#pragma warning(pop)

void ConvertURLCharacterW(std::wstring& strUrl);

extern nextlabs::cesdk_context cesdk_context_instance;

class CActionHandler
{
public:
	//allow return true
	virtual BOOL OpenAction ( const WCHAR* ) = 0;
	virtual BOOL PrintAction ( const DOCINFOW*, const WCHAR* ) = 0;
	virtual BOOL PrintAction(const WCHAR*) = 0;
	virtual BOOL UploadAction ( const LPOPENFILENAME, const std::wstring& ) = 0;
    virtual BOOL UploadAction ( const std::wstring& src, const std::wstring& dest) = 0;
    virtual BOOL PasteAction(const std::wstring& src, const std::wstring& dest) = 0;
public:
	static CActionHandler* GetInstance ( );
	
public:
	static BOOL GetCanonicalName ( std::wstring& CanonicalName, const std::wstring& FileName );
	static std::wstring FormatPath(const std::wstring& path);
public:
	CActionHandler ( );
	CActionHandler ( const CActionHandler& );
	CActionHandler& operator= ( const CActionHandler& );
	virtual ~CActionHandler ( );

private:
	enum IEVersionType { IEVersionNULL = 0, IEVersion6 = 6, IEVersion7, IEVersion8, IEVersion9, IEVersion10, IEVersion11 };

private:
	static IEVersionType GetIEVersion ( );
};

class CIEDefaultActionHandler : public CActionHandler
{
public:
	//allow return true
	BOOL OpenAction ( const WCHAR* );
	BOOL PrintAction ( const DOCINFOW*, const WCHAR* );
	BOOL PrintAction(const WCHAR*);
	BOOL UploadAction ( const LPOPENFILENAME, const std::wstring& );
    BOOL UploadAction ( const std::wstring& src, const std::wstring& dest);
    BOOL PasteAction(const std::wstring& src, const std::wstring& dest);

public:
	CIEDefaultActionHandler ( );
	CIEDefaultActionHandler ( const CIEDefaultActionHandler& );
	CIEDefaultActionHandler& operator= ( const CIEDefaultActionHandler& );
	virtual ~CIEDefaultActionHandler ( );
};