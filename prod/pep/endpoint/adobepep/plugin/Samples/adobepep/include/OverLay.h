#ifndef COVERLAY_H
#define COVERLAY_H

#pragma warning(push)
#pragma warning(disable:4819 4996 4995)
#include <boost/algorithm/string.hpp>
#pragma warning(pop)
#include <vector>
#include <string>
#include <objbase.h>
#include <gdiplus.h>
#include "VLObligation.h"
#include <process.h>

using namespace Gdiplus;
using namespace std;

#include "obMgr.h"


#define ADOBE_READER_SUFFIX	L" - Adobe Reader"
#define ADOBE_BAT_SUFFIX L" - Adobe Acrobat"


#ifdef _WIN64

#ifdef MSO2K3
const wchar_t PA_MODULE_NAME_NLVISUALABELING[]	= L"NLVisualLabelingPA2003.dll";
#else 
const wchar_t PA_MODULE_NAME_NLVISUALABELING[]	= L"NLVisualLabelingPA2007.dll";
#endif
#else
#ifdef MSO2K3
const wchar_t PA_MODULE_NAME_NLVISUALABELING[]	= L"NLVisualLabelingPA200332.dll";
#else
const wchar_t PA_MODULE_NAME_NLVISUALABELING[]	= L"NLVisualLabelingPA200732.dll";
#endif
#endif

struct _CEString {
	wchar_t *buf;
	size_t length;

	_CEString () { buf=NULL;}
	~_CEString () { if(buf) delete [] buf;}
};

typedef struct _CEString * CEString;

typedef struct _ADOBE_WND_INFO
{
	HWND	hAdobeView;
	HWND	hAdobeFrame;
	wchar_t* strFilePath;
}ADOBE_WND_INFO,*PADOBE_WND_INFO;


#define PRINT_VL_FUNC	"PrintVL"
typedef bool (*FuncPrintVL)(const HDC& hDC,const NM_VLObligation::IVisualLabelingInfo& theInfo);

#define DESTORYOVERLAY_FUN "DestoryOverlay"
typedef int (*DestroyOverlayFun)(HWND hView);

class COverLay
{
public:
	COverLay(void);
	~COverLay(void);
public:
	void DoViewOL(nextlabs::Obligations& obs,const wstring &strFilePath);
	void DoPrintOL(HDC hdc);
	

private:
	HWND GetPdfViewHandle(const wstring & strFilePath);
	wstring ReadPDFTitle(const wstring& strFilePath);
	
public:
	void SetHDC(const HDC& theHDC);

	void releaseHDC(const HDC& theHDC);

	bool SetOverlayData(_In_ nextlabs::Obligations& obs,_In_ const wstring & strFilePath);

	void SetOverLayDate(NM_VLObligation::VisualLabelingInfo VLInfo);

	void ReleaseOverlayData();

	bool IsSameHDC(const HDC& theHDC);

	void DoPrintOverlay();

	void StartGDIPlus();

	void ShutDownGDIPlus();

	void GetInfoValue(const wstring &strInfo,const wstring strName,wstring &strValue);

	//void SetVLInfo(wstring strInfo,wstring strPolicyName, wstring strFilePath, wstring strTextValue,NM_VLObligation::VisualLabelingInfo &VLInfo);

	//bool ReadReg(const HKEY hKey,const wstring strKeyName,wstring &strbuf);

	//bool ReadOLInfoFromReg(bool bDeny,bool bIsExistOL,wstring &strInfo, wstring &strPolicyName, wstring &strFilePath, wstring &strTextValue);
public:
	//bool WriteOLInfoToReg(bool bDeny,bool bIsExistOL);
	wstring SetOLInfo();

	wstring ConstructOLData(bool bDeny,bool bExistOL);

	void GetVLInfo(wstring strInfo,bool &bDeny,bool &bIsExistOL,NM_VLObligation::VisualLabelingInfo &VLInfo);

	bool IsExistViewOL(_In_ nextlabs::Obligations& obs);

	wstring GetEnforceBinDir();

	HWND GetViewHwnd();

	void SetViewHwnd(const HWND hView);

private:
	FuncPrintVL GetPrintVLFunAddr();
	void FreePrintVLAddr();
	bool ReadKey( const wchar_t* in_key , void* in_value , size_t in_value_size );
	
private:
	HDC				m_hDC;
	ULONG_PTR		m_gdiplusToken;
	NM_VLObligation::VisualLabelingInfo  m_OverLayInfo;
	FuncPrintVL m_pPrintVLFunc;
	HMODULE     m_hPrintVLDLL;
	static HWND  m_hViewWnd;
};


class COverLayData
{
private:
	COverLayData(void);
	~COverLayData(void);	
public:
	static COverLayData & GetInstance();
	void AddFrameAndPath(const HWND &hFrame,const wstring &strFilePath);	
	wstring GetFilePathFromFrameWnd(const HWND &hFrame);
	void DelAllInfo();	
	
private:
	map<HWND,wstring>     m_mapInfor;
	CRITICAL_SECTION	  m_ViewSec;
};

#endif