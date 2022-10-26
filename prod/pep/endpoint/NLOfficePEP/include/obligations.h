#pragma once
#ifndef __NEXTLABS_OFFICEPEP_OBLIGATIONS__
#define __NEXTLABS_OFFICEPEP_OBLIGATIONS__

#include <objbase.h>
#include <gdiplus.h>
#include "VLObligation.h"
#include "NLOfficePEP_Comm.h"

#pragma warning( push )
#pragma warning(disable:4819 4996 4995)
#include "eframework\platform\cesdk_obligations.hpp"
#include "PAMngr.h"
#pragma warning( pop )

#pragma warning(push)
#pragma warning(disable:4819 4996)
#include <boost/algorithm/string.hpp>
#pragma warning(pop)

#include "resattrmgr.h"
#include "resattrlib.h"

using std::map;
using std::wstring;
using std::vector;

//
// Encrypt
//
typedef BOOL(*fnSE_IsEncrypted)(_In_ const wchar_t* path);
typedef BOOL(*fnSE_EncryptFile)(_In_ const wchar_t* path);
typedef BOOL(*fnSE_MarkFileAsDRMOneShot)(_In_ const wchar_t* path);

class CNLEncrypt
{
public:
	CNLEncrypt(void);
	~CNLEncrypt(void);

public:
	bool EncryptTempFile(_In_ const AppType& emOfficeAppType,
		_In_ const wstring& wstrFile, 
		_In_ const bool bForSave = false);
	bool EncryptFile(_In_ const wstring& wstrPath);
	bool IsEncryptedFile(_In_ const wstring& wstrPath);

private:
	bool PreEncryptFile(_In_ const wstring& wstrPath);
	bool IsInited();
	void SetInitFlag(_In_ const bool& bFlag);

private:
	bool										m_bInit;
	HMODULE										m_hEncryptLib;
	fnSE_IsEncrypted							m_pFuncIsEncrypted;
	fnSE_EncryptFile							m_pFuncEncryptFile;
	fnSE_MarkFileAsDRMOneShot					m_pFuncMarkEncryptedFile;
};



typedef void(*PFUNC_RELEASETAGBUF)(const wchar_t* szBuf);

#define NLFILETAGGINGINSTANCE ( CNLFileTagging::NLGetInstance() )

class CNLFileTagging
{
private:
	CNLFileTagging(void);
	~CNLFileTagging(void);
public:
	static CNLFileTagging& NLGetInstance();

	bool NLDoFileTagging(_In_ const wchar_t* pwchSrcFilePath, _In_ const wchar_t* pwchDesFilePath,
		_In_ nextlabs::Obligations& obs, _In_ const OfficeAction& emAction, _Out_ vector<pair<wstring, wstring>>& vecTagPair);
private:
	// convert tag string to tag pair
	bool NLGetVecStringPairFromBuf(_In_ const wchar_t* pwszBuf, _In_ const DWORD& dwLen, _Out_ vector<pair<wstring, wstring>>& vecTagPair);

	// initialize
	bool NLInitialize();
	void NLUnInitialize();

	__inline void NLSetInitFlag(_In_ const bool& bFlag);
	__inline bool NLGetInitFlag();

private:
	HMODULE m_hModule;
	PFUNC_RELEASETAGBUF m_pfuncReaseTagBuf;
	bool		m_bInitFlag;	// 0: false, 1: true
};


//
// View Overlay
//  

#define NLVIEWOVERLAYINSTANCE ( CNLOvelayView::GetInstance() )
extern HINSTANCE g_hInstance;

struct _CEString {
	wchar_t *buf;
	size_t length;

	_CEString() { buf = NULL; }
	~_CEString() { if (buf) delete buf; }
};

class CNLOvelayView
{
public:
	static CNLOvelayView& GetInstance();
	void AddCach(HWND hView, bool bUsed = false, const wstring& filepath = L"");
	void AddView(HWND hView);
	void ClearCreatewindowCache(void);
	bool IsEmptyCreatewindowCache(void);
	void GetDoOverlayHwnd(AppType AppType, vector<HWND> &vecHwnd);
	void SetPathTryDoOverlay(const wstring &strFilePath);
	bool PathIsTryDoOverlay(const wstring &strFilePath);
	void DoViewOverlay(nextlabs::Obligations& obs, const wchar_t* szFile);
	void SendPrintPreview(bool switchstatus);
	void TriggerDoViewOverlay(void);
private:
	CNLOvelayView();
	~CNLOvelayView();
	void DelOverlayPath();
	static const wchar_t* COPYOF_CEM_GetString(CEString cestr);
	bool CallPaf(__in PA_Mngr::CPAMngr& pam, __in const PABase::ACTION pam_action, __in const std::wstring& new_source);
	bool GetNormalOfficeView(const wstring& strPath, HWND& hViewWnd, AppType	AppType);
private:
	map<HWND, pair<bool, wstring>>		m_mapViewUsed;	//cache view handle when createWindow	
	CRITICAL_SECTION	m_mapViewSec;
	bool				m_bSharePointExcelOnce;
	vector<HWND>        m_vechView;
	CRITICAL_SECTION	m_vecViewSec;
	vector<wstring>     m_vecFilePath;
	CRITICAL_SECTION	m_vecFilePathSec;
};


using namespace Gdiplus;

#define PRINT_VL_FUNC	"PrintVL"
typedef bool(*FuncPrintVL)(const HDC& hDC, const NM_VLObligation::IVisualLabelingInfo& theInfo);
#define NLPRINTOVERLAYINSTANCE ( CNLPrintOverlay::GetInstance() )

class CNLPrintOverlay
{
private:
	CNLPrintOverlay(void);
	~CNLPrintOverlay(void);
public:
	static CNLPrintOverlay & GetInstance();

	void SetHDC(const HDC& theHDC);
	void releaseHDC(const HDC& theHDC);
	bool SetOverlayData(_In_ nextlabs::Obligations& obs, _In_ const wstring & strFilePath);
	void ReleaseOverlayData();
	bool IsSameHDC(const HDC& theHDC);
	void DoPrintOverlay();
	void StartGDIPlus();
	void ShutDownGDIPlus();
	bool IsDoOverlay();
	void SetDoOverlayFlag(bool b);

private:
	FuncPrintVL GetPrintVLFunAddr();
	void FreePrintVLAddr();
	HDC				m_hDC;
	ULONG_PTR		m_gdiplusToken;
	FuncPrintVL m_pPrintVLFunc;
	HMODULE     m_hPrintVLDLL;
	NM_VLObligation::IVisualLabelingInfo m_theInfo;
	bool		m_bIsDoOverLay;
};


/*
*\ Brief: this class is used to synchronize the custom tags to golden tags
*		1. for 6.2.6 SE, we only can write the golden tags after the file closed. we need the file write permission when we want to synchronize the golden tags
*   2. close action or after save as the source file will closed. ( Note: PPT save as like .png format the source file didn't closed )
*		3. we use a thread to do synchronize the golden tags. this thread create when office connect and will be destroyed when the office pep disconnect or DllMain detach.
*		4. this class does not support multiple files synchronize the golden tags at one trigger event.
*/
class CNLSESynchroGoldenTags
{
public:
	CNLSESynchroGoldenTags(void);
	~CNLSESynchroGoldenTags(void);

	// public for initialize and uninitialize. we will create a thread to synchronize the golden tags. this two use to manager the thread create and destroy.
public:
	// init or uninit only used for asynchronous I/O
	void NLInitialize();
	void NLUnInitialize();

private:
	void NLSetInitializeFlag(_In_ BOOL bInitialize);
	bool NLGetInitializeFlag();

	// public for start to process synchronize the golden tags
public:
	/*
	*\ Brief: this function used to start up synchronize the golden tags
	*\ Parameter:
	*		[IN] wstrFilePath: the file full path which will be synchronize the golden tags. we can save the golden tags by NLSetGoldenTags before we start synchronize golden tags.
	*   [IN] vecGoldenTags: the golden tags need synchronize
	*		[in] bIOSynchronize:
	*					true:  I/O synchronous
	*					false: I/O Asynchronous
	*\ Return value:
	*		return false: means start synchronize golden tags failed.
	*		return true:  only means the function invoke success. \
	*	 Note: function return true no means the golden tags synchronize success, because we only can known write tags success but we don't known the tags is the golden tags or not.
	*		Success synchronize golden tags need: SE file, right start up time and success start synchronize
	*/
	bool NLStartSynchronizeGoldenTags(_In_ const wstring& wstrFilePath, _In_ const vector<pair<wstring, wstring>>& vecGoldenTags, _In_ const bool bIOSynchronize = false);

	bool NLStartSynchronizeGoldenTags(_In_ const map<wstring, STUNLOFFICE_GOLDENTAG>& mapGoldenTags, _In_ const bool bIOSynchronize = false);

	//////////////////////////for thread////////////////////////////////////////////////
private:
	static unsigned __stdcall NLSynchronizeGoldenTagsThread(void* pArguments);

	// private tools
private:
	void NLCopyIntoProcessContainer(_In_ const wstring& wstrFilePath, _In_ const vector<pair<wstring, wstring>>& vecGoldenTags);

	void NLCopyIntoProcessContainer(_In_ const map<wstring, STUNLOFFICE_GOLDENTAG>& mapGoldenTags);

	void NLGetProcessGoldenTags(_Out_ map<wstring, STUNLOFFICE_GOLDENTAG>& mapGoldenTags);

	void NLSetThreadEndFlag(_In_ BOOL bNeedEndThread);

	bool NLGetThreadEndFlag();

	bool NLSynchronizeGoldenTags(_In_ const wstring& wstrFilePath, _In_ const vector<pair<wstring, wstring>>& vecGoldenTags);

private:
	CRITICAL_SECTION m_csMapGoldenTag;
	map<wstring, STUNLOFFICE_GOLDENTAG> m_mapGoldenTag;

	HANDLE m_hSemaphore;
	HANDLE m_hThread;
	BOOL m_bNeedEndThread;
	BOOL m_bInitialize;
};


//
// Tag obligation
//

typedef int(*CreateAttributeManagerType)(ResourceAttributeManager **mgr);
typedef int(*AllocAttributesType)(ResourceAttributes **attrs);
typedef int(*ReadResourceAttributesWType)(ResourceAttributeManager *mgr, const WCHAR *filename, ResourceAttributes *attrs);
typedef int(*GetAttributeCountType)(const ResourceAttributes *attrs);
typedef void(*FreeAttributesType)(ResourceAttributes *attrs);
typedef void(*CloseAttributeManagerType)(ResourceAttributeManager *mgr);
typedef void(*AddAttributeWType)(ResourceAttributes *attrs, const WCHAR *name, const WCHAR *value);
typedef const WCHAR *(*GetAttributeNameType)(const ResourceAttributes *attrs, int index);
typedef const WCHAR * (*GetAttributeValueType)(const ResourceAttributes *attrs, int index);
typedef int(*WriteResourceAttributesWType)(ResourceAttributeManager *mgr, const WCHAR *filename, ResourceAttributes *attrs);
typedef int(*RemoveResourceAttributesWType)(ResourceAttributeManager *mgr, const WCHAR *filename, ResourceAttributes *attrs);
typedef int(*Convert4GetAttr)(ResourceAttributes *attrs, ResourceAttributes* existing_attrs);
typedef int(*Convert4SetAttr)(ResourceAttributes* attrs_to_set, ResourceAttributes* merged_attrs);

class CNLTag
{
public:
	CNLTag(void);
	~CNLTag(void);

public:
	// read tag base on file use tag library, tag pair: <itar,yes>, <itar,no>
	bool ReadTag(_In_ const wstring& wstrPath, _Out_ vector<pair<wstring, wstring>>& vecTagPair);
	// read tag base on office com object. read office custom attributes, tag pair: <itar,yes|no>
	bool ReadTag(_In_ IDispatch* pDocObj, _Out_ vector<pair<wstring, wstring>>& vecTagPair);

	// add tag base on file use tag library.
	bool AddTag(_In_ const wstring& wstrPath, _In_ const vector<pair<wstring, wstring>>& vecTagPair);
	// add tag on document base on office COM object, add tag to custom attributes
	// bIsSave = true, save tag.
	bool AddTag(_In_ IDispatch* pDocObj, _In_ const vector<pair<wstring, wstring>>& vecTagPair, _In_ const bool bIsSave = true);

	bool RemoveTag(_In_ IDispatch* pDocObj, _In_ bool bIsSave = true);
	bool RemoveTag(_In_ const wstring& wstrPath, _In_ const vector<pair<wstring, wstring>>& vecTagPair);

	bool RemoveAllTag(_In_ const wstring& wstrPath);
	bool RemoveAllTag(_In_ IDispatch* pDocObj, _In_ const bool& bIsSave);

private:
	// tools, before you use the following function you need the if we get the tag library success
	void NLAddAttributeFromVector(_Inout_ ResourceAttributes*& pAttr, _In_ const vector<pair<wstring, wstring>>& vecTagPair, _In_ const bool bIgnoreSEInherent = false);
	void NLGetAttributeToVetor(_In_   ResourceAttributes*& pAttr, _Out_ vector<pair<wstring, wstring>>& vecTagPair);

	bool NLAlloceResource(_Out_ ResourceAttributeManager*& pMgr, _Out_ vector<ResourceAttributes*>& vecpAttr, _In_ unsigned int nCount);
	bool NLAlloceResource(_Out_ ResourceAttributeManager*& pMgr, _Out_ ResourceAttributes*& pAttr);

	void NLFreeResource(_Inout_ ResourceAttributeManager*& pMgr, _Inout_ vector<ResourceAttributes*>&  vecpAttr);
	void NLFreeResource(_Inout_ ResourceAttributeManager*& pMgr, _Inout_ ResourceAttributes*& pAttr);

	bool NLSaveCustomTags(_In_ IDispatch* pDoc);
	bool NLDeleteTags(_In_ IDispatch* pCustomProperties);

	// Common function
	CComPtr<IDispatch> GetCustomProperties(_In_ IDispatch* pCurDoc);
	DWORD GetIEVersion();
	bool NLIsEncryptFileInherentTag(_In_ const wchar_t* pwchTagName);

	// for file path
	bool NLGetEffectFilePath(_Inout_ wstring& wstrEffectFilePath);
	bool NLHttpPathToUncPath(_Inout_ wstring& wstrFilePath);
    bool NLFilePathToUncPath(_Inout_ wstring& wstrFilePath);

	// for load tag library
	bool GetTagDllHandle();
	bool GetTagFunAddr();
	bool IsGetFunSuc();
	void SetGetFunSucFlag(_In_ bool bSuccess);

private:
	bool			m_bIsGetFunSuc;
	DWORD			m_dwIEVer;
	HMODULE	m_hLib;
	HMODULE	m_hMgr;

	// we can not make sure the return value means 
	CreateAttributeManagerType			m_lfCreateAttributeManager;
	AllocAttributesType						m_lfAllocAttributes;
	ReadResourceAttributesWType		m_lfReadResourceAttributesW;	// return 1, means right
	GetAttributeCountType					m_lfGetAttributeCount;

	FreeAttributesType							m_lfFreeAttributes;						// no return value
	CloseAttributeManagerType			m_lfCloseAttributeManager;		// no return value
	AddAttributeWType							m_lfAddAttributeW;						// no return value

	GetAttributeNameType				  	m_lfGetAttributeName;
	GetAttributeValueType					m_lfGetAttributeValue;
	WriteResourceAttributesWType		m_lfWriteResourceAttributesW;	// return 1, means right
	RemoveResourceAttributesWType	m_lfRemoveResourceAttributesW;

	Convert4GetAttr								m_lConvert4GetAttr;		// this function alway return 1, no means			
	Convert4SetAttr								m_lConvert4SetAttr;		// this function alway return 1, no means
};


#endif	//__NEXTLABS_OFFICEPEP_OBLIGATIONS__