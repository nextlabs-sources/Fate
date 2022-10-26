#ifndef _AUXILIARY_H_
#define _AUXILIARY_H_

#include "stdafx.h"
#include <string>
using namespace std;
#include <boost/algorithm/string.hpp>
#define REPEAT_SHOW_TEXT L"repeat"

struct TextInfo
{
	wstring strUser;
	wstring strHostHome;
};

enum emProcessType
{
	 WORD_TYPE,
	 PPT_TYPE,
	 EXCEL_TYPE,
	 ADOBE_TYPE,
	 ADOBE_ADDIN_TYPE,
	 UNKNOWN_TYPE
};

enum emFullScreenType
{
	FULLSCREEN_NO_FILENAME,
	FULLSCREEN_FILENAME_FALSE,
	FULLSCREEN_FILENAME_TRUE,
	FULLSCREEN_IE
};

enum em0penType
{
	IE_OPNE_TYPE,
	APP_OPEN_TYPE,
	UNKNOW_OPEN_TYPE
};


class CAuxiliary
{
public:
	CAuxiliary();
	~CAuxiliary();
	Color ChooseFontColor(__in DWORD dwColor,__in long lTransParent);
	void GetUserHostHome(__in const wstring & strText,__out TextInfo& theTextInfo);
	bool IsRepeat(__in const wstring &strPlacement);
	emProcessType GetProgressType();
	int GetAppVer();
	static void DisableAeroPeek(bool bDisable=true);
	/*
	**	Applicable scenario:	protect view , Frame window and Full page window, get file name from caption.
	*/
	static void GetFileNameFromCaption(__inout wstring& str);
	static DWORD GetParentProcessID(DWORD dwProcessID);
	em0penType GetOpenType(HWND hForeWnd);
private:
	wstring GetFileName(const wstring& str);
public:
	static  emProcessType m_emProcType;
	static  em0penType    m_emOpenType;
	static  int			  m_version;
};


#endif
