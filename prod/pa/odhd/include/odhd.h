#ifndef __H_ODHD__

#define __H_ODHD__

#include <string>

#include <vector>

#include <algorithm> 

#include <windows.h>

#include <crtdbg.h>

#include "log.h"

#include "odhd_inc.h"

#include "HdrProgDlg.h"

#include "hdrDlg.h"





#ifdef NDEBUG

#ifndef assert

#define assert(parm)

#endif

#endif





#define MAX_HDRDLG_PROGRESS	10

extern HINSTANCE            g_hInstance;

static const  int ODHDCAT_MAX=10;



typedef std::vector<std::wstring> RecipientVector;

typedef std::vector<std::pair<std::wstring,std::wstring>> AttachVector;

typedef std::vector<std::wstring> HelpUrlVector;



extern "C" __declspec(dllexport) long HDRObligation(HWND hDialogParentWnd,

    RecipientVector &vecRecipients, AttachVector &vecAttachments,

    HelpUrlVector &vecHelpUrls,VARIANT_BOOL isWordMail);

HRESULT DispatchCallWraper(CComPtr<IDispatch> pDisp,int dispatchType, VARIANT *pvResult,  LPOLESTR propName, int cArgs...);



enum EnumODType{ODTYPE_WORD=0, ODTYPE_EXCEL, ODTYPE_PPT, ODTYPE_OTHERS=20};



/*

	INSPECT_FAILED: Remove Hidden data failed

	INSPECT_NONE:	No Hidden data found

	INSPECT_HAVE:	Found hidden data

	INSPECT_REMOVED:Hidden data be removed succeeded

	INSPECT_INSPECTFAIL:Detect hidden data failed

*/

enum ODHDSTATUS{INSPECT_FAILED=-1,INSPECT_NONE=0,INSPECT_HAVE=1,INSPECT_REMOVED=2,INSPECT_DETECTFAIL=3};



enum ODHDCATEGORY

{

//Word

	ODHDCAT_WORD_MIN=0,	

	ODHDCAT_WORD_COMMENTS=0, 

	ODHDCAT_WORD_PROP,

#ifndef WSO2K3
#ifdef WSO2013
 	ODHDCAT_COLLAPSE,
#endif
	ODHDCAT_WORD_XML,

#endif

	ODHDCAT_WORD_HEADFOOT, 

//#ifdef WSO2K7

	ODHDCAT_WORD_TEXT,

//#endif

	ODHDCAT_WORD_MAX,



//Excel

	ODHDCAT_EXCEL_MIN=0,

	ODHDCAT_EXCEL_COMMENTS=0,	

	ODHDCAT_EXCEL_PROP,	

#ifndef WSO2K3

	ODHDCAT_EXCEL_XML,

#endif

	ODHDCAT_EXCEL_HEADFOOT,	

	ODHDCAT_EXCEL_ROWCOL,

	ODHDCAT_EXCEL_WORKSHEET,

	ODHDCAT_EXCEL_INVISCONTENT,

	ODHDCAT_EXCEL_AUTOFILTER,

	ODHDCAT_EXCEL_MAX,



//PPT

	ODHDCAT_PPT_MIN=0,

	ODHDCAT_PPT_COMMENTS=0,	

	ODHDCAT_PPT_PROP,

#ifndef WSO2K3

	ODHDCAT_PPT_XML,	

	ODHDCAT_PPT_ONSLIDE,

	ODHDCAT_PPT_OFFSLIDE,	

	ODHDCAT_PPT_NOTE,

#endif

	ODHDCAT_PPT_MAX

};

enum ODHDITEM

{

	ODHDITEM_DEFAULT=0,

	//Word's items

	ODHDITEM_WORD_MIN=0,

	ODHDITEM_WORD_COMMENTS=0,	ODHDITEM_WORD_REVISION,

	ODHDITEM_WORD_VERSION,	ODHDITEM_WORD_INK,

	

	ODHDITEM_WORD_DOCPROP,	ODHDITEM_WORD_EMAIL,

	ODHDITEM_WORD_ROUTESLIP,ODHDITEM_WORD_SEND4REVIEW,

	ODHDITEM_WORD_SRVPROP,	ODHDITEM_WORD_MANAGEPOLICY,

	ODHDITEM_WORD_CONTENTTYPE,ODHDITEM_WORD_DATABINDLINK,

	ODHDITEM_WORD_USERNAME,	ODHDITEM_WORD_TEMPLATE,



	ODHDITEM_WORD_DOCHEADER,ODHDITEM_WORD_FOOTER,

	ODHDITEM_WORD_WATERMARK,



	ODHDITEM_WORD_HIDDENTEXT,

	ODHDITEM_WORD_CUSTOMXML,

	ODHDITEM_WORD_MAX,



	//Excel's items

	ODHDITEM_EXCEL_MIN=0,

	ODHDITEM_EXCEL_COMMENTS=0,	ODHDITEM_EXCEL_INK,

	

	ODHDITEM_EXCEL_DOCPROP,		ODHDITEM_EXCEL_EMAIL,

	ODHDITEM_EXCEL_ROUTESLIP,	ODHDITEM_EXCEL_SEND4REVIEW,

	ODHDITEM_EXCEL_SRVPROP,		ODHDITEM_EXCEL_MANAGEPOLICY,

	ODHDITEM_EXCEL_CONTENTTYPE,	ODHDITEM_EXCEL_USERNAME,

	ODHDITEM_EXCEL_PRINTER,		ODHDITEM_EXCEL_SCENARIO,

	ODHDITEM_EXCEL_WEBPUBLISH,	ODHDITEM_EXCEL_CMNT4NAMES,

	ODHDITEM_EXCEL_EXTDATACONN,	

	

	ODHDITEM_EXCEL_WSHEADER,	ODHDITEM_EXCEL_WSFOOTER,



	ODHDITEM_EXCEL_HIDDENROWS,	ODHDITEM_EXCEL_HIDDENCOLS,



	ODHDITEM_EXCEL_HIDDENWS,

	ODHDITEM_EXCEL_CUSTOMXML,

	ODHDITEM_EXCEL_INVIOBJECT,

	ODHDITEM_EXCEL_AUTOFILTER,

	ODHDITEM_EXCEL_MAX,



	//PowerPoint's items

	ODHDITEM_PPT_MIN=0,

	ODHDITEM_PPT_COMMENTS=0,	ODHDITEM_PPT_INK,



	ODHDITEM_PPT_DOCPROP,	ODHDITEM_PPT_EMAIL,

	ODHDITEM_PPT_ROUTESLIP,	ODHDITEM_PPT_SEND4REVIEW,

	ODHDITEM_PPT_SRVPROP,	ODHDITEM_PPT_MANAGEPOLICY,

	ODHDITEM_PPT_CONTENTTYPE,ODHDITEM_PPT_WEBPUBLISH,

	

	ODHDITEM_PPT_INVIOBJECT,

	ODHDITEM_PPT_OFFSLIDE,

	ODHDITEM_PPT_PRESENTNOTE,

	ODHDITEM_PPT_CUSTOMXML,

	ODHDITEM_PPT_MAX

};



struct StaticCatTitle

{

	std::wstring m_Title;

	std::wstring m_ItemFoundInfo;

	std::wstring m_NoFoundInfo;

	std::wstring m_RemoveSucceed;

	std::wstring m_RemoveFail;

	std::wstring m_Hint;

	long	m_HaveItemList;//1:have;0:no

};



struct StaticItemResult

{

	std::wstring m_itemTitle;

	std::wstring m_itemFound;

	std::wstring m_itemNotFound;

	std::wstring m_itemRemoveError;

	long	m_NeedItemNum;//whether to list out the number of found items.

};



class CODHDUtilities

{

public:

	static BOOL IsWordFile(LPCWSTR pwzFile);

	static BOOL IsWordTemplateFile(LPCWSTR pwzFile);

	static BOOL IsExcelFile(LPCWSTR pwzFile);

	static BOOL IsExcelTemplateFile(LPCWSTR pwzFile);

	static BOOL IsPptFile(LPCWSTR pwzFile);

	static BOOL IsOffice2007File(LPCWSTR pwzFile);

	static DWORD GetWordMicroTrustLevel2003(DWORD dwDefaultLevel = 3);      // "Low security level"

	static DWORD GetExcelMicroTrustLevel2003(DWORD dwDefaultLevel = 3);

	static DWORD GetPptMicroTrustLevel2003(DWORD dwDefaultLevel = 3);



	static void ReplaceFileName(std::wstring &OriginalMessage, std::wstring &FileName, std::wstring &NewMessage);



	static void SetWordMicroTrustLevel2003(DWORD dwLevel=1);

	static void SetExcelMicroTrustLevel2003(DWORD dwLevel=1);

	static void SetPptMicroTrustLevel2003(DWORD dwLevel=1);



	static BOOL GetDocumentPropValue(CComPtr<IDispatch> pProps, LPCWSTR pwzName, LPWSTR pwzValue, int nSize);

	static BOOL EmptyDocumentPropValue(CComPtr<IDispatch> pProps, LPCWSTR pwzName, LPCWSTR pwzValue);

	static BOOL RemoveDocumentProperties(CComPtr<IDispatch> lpDocumentProps,LPCWSTR pPropNames[],int iPropCount);

	static BOOL RemoveCustomProperties(CComPtr<IDispatch> lpCustomProps);

	static void ShowPasswordWindows(void *dummy);
	static void ShowPromtWindows(void *dummy);
#ifndef WSO2K3

	static BOOL UpPrivilege(HANDLE hprocess,LPCTSTR lpname) ;

	static BOOL GetDestProcessId(LPCWSTR wzProcName, DWORD* pdwProcId);

	static void InjectDllToRemoteProcess(wchar_t* wszExecName);

#endif

};



class CODHDItem

{

public:

	CODHDItem(ODHDITEM indItem){m_indItem=indItem;m_itemFoundCount=0;m_itemShowCount=0;};

	~CODHDItem(){};

	void SetItemTitle(std::wstring itemTitle){m_itemTitle=itemTitle;};

	void SetItemResult(std::wstring itemResult){m_itemResult=itemResult;};

	void SetItemCount(long lFoundCount){m_itemFoundCount=lFoundCount;};



	void SetItemNumShow(long itemShowCount){m_itemShowCount=itemShowCount;};

	std::wstring GetItemResult()

	{

		wchar_t num[8];

		_snwprintf_s(num, 8, _TRUNCATE,L"%d",m_itemFoundCount);

		if(m_itemShowCount)

			return m_itemResult+L": "+num;

		else

			return m_itemResult;

	};

	friend bool operator == (CODHDItem * srcHDItem,const  ODHDITEM indItem);

private:

	ODHDITEM	 m_indItem;

	long		 m_itemFoundCount;

	long		 m_itemShowCount;

	std::wstring m_itemTitle;

	std::wstring m_itemResult;



};



class CODHDCategory

{

typedef std::vector<CODHDItem*> HDItemVec;

public:

	CODHDCategory(ODHDCATEGORY hdCat){m_Category=hdCat;m_pCatStrings=NULL;m_catStatus=INSPECT_NONE;};

	~CODHDCategory(){};



	CODHDItem* GetHDItem(ODHDITEM indItem)

	{

		CODHDItem*	pItem=NULL;

		HDItemVec::iterator iter=std::find(m_hdItems.begin(),m_hdItems.end(),indItem);

		if(iter!=m_hdItems.end())

			pItem=*iter;

		else

		{

			pItem=new CODHDItem(indItem);

			m_hdItems.push_back(pItem);

		}

		return pItem;

	}



	void Clear()

	{

		HDItemVec::iterator iter=m_hdItems.begin();

		for(;iter!=m_hdItems.end();iter++)

		{

			delete (*iter);

		}

		m_hdItems.clear();

	};

	void			SetCatTitle(std::wstring catTitle){m_catTitle=catTitle;};

	void			SetCatStrings(StaticCatTitle* pCatStrings){m_pCatStrings=pCatStrings;};

	std::wstring	GetCatTitle(){return m_pCatStrings->m_Title;};



	void			SetCatStatus(ODHDSTATUS catStatus){m_catStatus=catStatus;};

	ODHDSTATUS		GetCatStatus(){return m_catStatus;};





	void			GetItemsResult(std::wstring &strResult,long starFlag=1)

	{

		switch(m_catStatus)

		{

		case INSPECT_NONE:

			strResult= m_pCatStrings->m_NoFoundInfo;

			break;

		case INSPECT_HAVE:

			if(starFlag)

				strResult= m_pCatStrings->m_ItemFoundInfo;

			break;

		case INSPECT_FAILED:

			strResult= m_pCatStrings->m_RemoveFail;

			break;

		case INSPECT_REMOVED:

			strResult= m_pCatStrings->m_RemoveSucceed;

			break;

		default:

			break;



		}



		if(m_catStatus==INSPECT_NONE||INSPECT_FAILED==m_catStatus||INSPECT_REMOVED==m_catStatus)

			return;

		if(m_pCatStrings->m_HaveItemList==0)

			return;

		HDItemVec::iterator iter=m_hdItems.begin();



		for(;iter!=m_hdItems.end();iter++)

		{

			if(starFlag)

				strResult +=L"\n  * "+(*iter)->GetItemResult();

			else

				strResult +=L"\n"+(*iter)->GetItemResult();

		}

	};



#ifndef WSO2K3

	void			GetItemsResult2K7(std::wstring &strResult,long byInspector=0)

	{

		switch(m_catStatus)

		{

		case INSPECT_NONE:

			strResult= m_pCatStrings->m_NoFoundInfo;

			break;

		case INSPECT_HAVE:

			if(byInspector==0)

				strResult= m_pCatStrings->m_ItemFoundInfo;

			break;

		case INSPECT_FAILED:

			strResult= m_pCatStrings->m_RemoveFail;

			break;

		case INSPECT_REMOVED:

			strResult= m_pCatStrings->m_RemoveSucceed;

			break;

		default:

			break;



		}



		if(m_catStatus==INSPECT_NONE||INSPECT_FAILED==m_catStatus||INSPECT_REMOVED==m_catStatus)

			return;

		if(m_pCatStrings->m_HaveItemList==0)

			return;

		HDItemVec::iterator iter=m_hdItems.begin();



		for(;iter!=m_hdItems.end();iter++)

		{

			if(byInspector==1)

				strResult +=(*iter)->GetItemResult();

			else

				strResult +=L"\n  * "+(*iter)->GetItemResult();

		}



	};

#endif

	friend bool operator == (CODHDCategory * srcHDCat,const  ODHDCATEGORY hdCat);

private:

	ODHDCATEGORY	m_Category;

	HDItemVec		m_hdItems;

	std::wstring	m_catTitle;

	StaticCatTitle* m_pCatStrings;

	ODHDSTATUS		m_catStatus;



};



class CODHDCatManager

{

typedef std::vector<CODHDCategory*> HDCatVec;

public:

	CODHDCatManager(long catCount/*=ODHDCAT_MAX*/,StaticCatTitle *pCatTitle)

	{

		long iIndex=0;

		CODHDCategory *pODHDCat=NULL;

		m_CatCount=catCount;

		for(;iIndex<m_CatCount;iIndex++)

		{

			pODHDCat=new CODHDCategory((ODHDCATEGORY)iIndex);

			pODHDCat->SetCatStrings(&pCatTitle[iIndex]);

			pODHDCat->SetCatTitle(pCatTitle[iIndex].m_Title);

			m_hdCats.push_back(pODHDCat);

		}

	};

	~CODHDCatManager()

	{

		CODHDCategory *pODHDCat=NULL;

		HDCatVec::iterator	iter;

		for(iter=m_hdCats.begin();iter!=m_hdCats.end();iter++)

		{

			(*iter)->Clear();

			delete (*iter);

		}

		m_hdCats.clear();

	};

	CODHDCategory* GetHDCategory(ODHDCATEGORY hdCat)

	{

		CODHDCategory*	pODHDCat=NULL;

		HDCatVec::iterator iter=std::find(m_hdCats.begin(),m_hdCats.end(),hdCat);

		if(iter!=m_hdCats.end())

			pODHDCat=*iter;

		else

		{

			pODHDCat=new CODHDCategory(hdCat);

			m_hdCats.push_back(pODHDCat);

		}

		return pODHDCat;

	}

	



private:

	long		m_CatCount;

	HDCatVec	m_hdCats;

};



class MSOAppInstance

{

public:

	static MSOAppInstance * Instance()

	{

		if( 0== _instance.get())

			_instance.reset(new MSOAppInstance);

		return _instance.get();

	};

	Word::_wordApplication*		GetWordAppInstance()

	{

		HRESULT         hr= S_OK;

		if(m_pWordApp)

			return m_pWordApp;

		hr = CoCreateInstance(Word::CLSID_wordApplication, NULL, CLSCTX_LOCAL_SERVER, Word::IID__wordApplication, (void**)&m_pWordApp);

		DP((L"Create word instance : %s :: 0x%X\n", (SUCCEEDED(hr))?L"OK":L"Fail", hr));

		if(FAILED(hr) || 0==m_pWordApp)

			return NULL;

#ifndef WSO2K3

		CODHDUtilities::InjectDllToRemoteProcess(L"WINWORD.EXE");

#endif

		return m_pWordApp;

	};

	Excel::_excelApplication*	GetExcelAppInstance()

	{

		HRESULT         hr= S_OK;

		if(m_pExcelApp)

			return m_pExcelApp;

		hr = CoCreateInstance(Excel::CLSID_excelApplication, NULL, CLSCTX_LOCAL_SERVER, Excel::IID__excelApplication, (void**)&m_pExcelApp);

		DP((L"Create excel instance : %s :: 0x%X\n", (SUCCEEDED(hr))?L"OK":L"Fail", hr));

		if(FAILED(hr) || 0==m_pExcelApp)

			return NULL;

#ifndef WSO2K3

		CODHDUtilities::InjectDllToRemoteProcess(L"EXCEL.EXE");

#endif

		return m_pExcelApp;

	};

	PPT::_pptApplication*		GetPPTAppInstance()

	{

		HRESULT         hr= S_OK;

		if(m_pPPTApp)

			return m_pPPTApp;

		hr = CoCreateInstance(PPT::CLSID_pptApplication, NULL, CLSCTX_LOCAL_SERVER, PPT::IID__pptApplication, (void**)&m_pPPTApp);

		DP((L"Create power point instance : %s :: 0x%X\n", (SUCCEEDED(hr))?L"OK":L"Fail", hr));

		if(FAILED(hr) || 0==m_pPPTApp)

			return (PPT::_pptApplication*)NULL;

#ifndef WSO2K3

		CODHDUtilities::InjectDllToRemoteProcess(L"POWERPNT.EXE");

#endif

		

		return m_pPPTApp;

	};

	void ReleaseWordApp()

	{

		HRESULT hr=S_OK;

		if(m_pWordApp)

		{

			CComPtr<Word::Documents>	pDocs;

			hr=m_pWordApp->get_Documents(&pDocs);

			if(SUCCEEDED(hr)&&pDocs)

			{

				long lDocCount=0;

				hr=pDocs->get_Count(&lDocCount);

				if(SUCCEEDED(hr)&&lDocCount>0)

					;

				else

				{

					CComVariant vSaveChanges   =wdDoNotSaveChanges;

					hr=m_pWordApp->Quit(&vSaveChanges);

					m_pWordApp = NULL;

				}

			}

			else

			{

				if(FAILED(hr)||pDocs==NULL)

				{

					CComVariant vSaveChanges   =wdDoNotSaveChanges;

					hr=m_pWordApp->Quit(&vSaveChanges);

					m_pWordApp = NULL;

				}

			}

		}

	};

	void ReleaseAppInstance()

	{

		HRESULT	hr = S_OK;

		/*if(m_pWordApp)

		{

			hr=m_pWordApp->Quit();

			hr=m_pWordApp->Release();

			m_pWordApp = NULL;

		}*/

		ReleaseWordApp();

		if(m_pExcelApp)

		{

			m_pExcelApp->Quit();

			m_pExcelApp = NULL;

		}

		if(m_pPPTApp)

		{

			CComPtr<PPT::Presentations>	spPrents = NULL;

			long                lCount   = 0;

			BOOL                bQuit    = TRUE;

			hr = m_pPPTApp->get_Presentations(&spPrents);

			if(SUCCEEDED(hr) && spPrents)

			{

				hr = spPrents->get_Count(&lCount);

				if(SUCCEEDED(hr) && lCount)

					bQuit = FALSE;

			}

			if(bQuit)

				m_pPPTApp->Quit();

			m_pPPTApp = NULL;

		}

	}

protected:

	MSOAppInstance(void)

	{

		m_pWordApp=NULL;

		m_pExcelApp=NULL;

		m_pPPTApp=NULL;

		m_dwWordTrustLevel	= CODHDUtilities::GetWordMicroTrustLevel2003();

		m_dwExcelTrustLevel = CODHDUtilities::GetExcelMicroTrustLevel2003();

		m_dwPptTrustLevel	= CODHDUtilities::GetPptMicroTrustLevel2003();

		CODHDUtilities::SetWordMicroTrustLevel2003();

		CODHDUtilities::SetExcelMicroTrustLevel2003();

		CODHDUtilities::SetPptMicroTrustLevel2003();

	};

	~MSOAppInstance(void)

	{

		ReleaseAppInstance();

		CODHDUtilities::SetWordMicroTrustLevel2003(m_dwWordTrustLevel);

		CODHDUtilities::SetExcelMicroTrustLevel2003(m_dwExcelTrustLevel);

		CODHDUtilities::SetPptMicroTrustLevel2003(m_dwPptTrustLevel);

	};

	friend class std::auto_ptr<MSOAppInstance>;

	static std::auto_ptr<MSOAppInstance> _instance;



private:

	DWORD	m_dwWordTrustLevel;

	DWORD	m_dwExcelTrustLevel;

	DWORD	m_dwPptTrustLevel;

	CComPtr<Word::_wordApplication> 		m_pWordApp;

	CComPtr<Excel::_excelApplication>	m_pExcelApp;

	CComPtr<PPT::_pptApplication>		m_pPPTApp;

};




enum EnOfficeVersion {EnOfficeVersion_2003 = 0, EnOfficeVersion_2007, EnOfficeVersion_2010, EnOfficeVersion_2013,EnOfficeVersion_2016, EnOfficeVersion_Other};


class CODHDInspector

{

public:

	//CODHDInspector(){ m_pCatManager=new CODHDCatManager;};

	CODHDInspector(long catCount,StaticCatTitle *pCatTitle)
	{
		m_Count=catCount;
		m_pCatTitle=pCatTitle; 
		m_pCatManager=new CODHDCatManager(catCount,pCatTitle);
		m_pHDRFile = NULL;
		m_IsOffice2010=FALSE;
		m_IsOffice2013=FALSE;
		m_IsOffice2016=FALSE;

		int nVer = GetInstalledOfficeVersion();
		if(nVer==EnOfficeVersion_2010)
			m_IsOffice2010=TRUE;
		else if (nVer == EnOfficeVersion_2013)
			m_IsOffice2013=TRUE;
		else if (nVer == EnOfficeVersion_2016)
			m_IsOffice2016=TRUE;
	};

	int GetInstalledOfficeVersion(void);

	BOOL	m_IsOffice2010;
	BOOL	m_IsOffice2013;
	BOOL	m_IsOffice2016;

	virtual ~CODHDInspector(){delete m_pCatManager;};

	virtual	BOOL			Inspect(std::wstring strSrcFileName,std::wstring strTempFileName){return TRUE;};

	virtual BOOL			Remove(ODHDCATEGORY odhdCat,ODHDITEM odhdItem=ODHDITEM_DEFAULT){return TRUE;};

	virtual BOOL			Remove(int odhdCat,ODHDITEM odhdItem=ODHDITEM_DEFAULT){return Remove((ODHDCATEGORY)odhdCat,odhdItem);};



	virtual BOOL			FilterByCategory(ODHDCATEGORY odhdCat, ODHDSTATUS odhdStatus) { return FALSE; }

	virtual BOOL			GetFilter(ODHDCATEGORY odhdCat) { return FALSE; }

	//virtual long			GetRemoveButtonVisible(){return m_lRemoveButtonVisible;}
	//virtual void			SetRemoveButtonVisible(long lVal){m_lRemoveButtonVisible=lVal;}

	virtual long			GetContent(ODHDCATEGORY odhdCat,ODHDITEM odhdItem,std::wstring &strContent);

	virtual long			SetContent(ODHDCATEGORY odhdCat,ODHDITEM odhdItem,std::wstring &strContent);



	virtual ODHDSTATUS		GetCatStatus(ODHDCATEGORY odhdCat);

	virtual ODHDSTATUS		GetCatStatus(int odhdCat);

	virtual void			SetCatStatus(ODHDCATEGORY odhdCat,ODHDSTATUS odhdStatus)

	{

		CODHDCategory *	pCat=NULL;

		ODHDSTATUS		newCatStatus=odhdStatus;

		assert(m_pCatManager);

		pCat=m_pCatManager->GetHDCategory(odhdCat);

		pCat->SetCatStatus(odhdStatus);

	};

	virtual void			SetCatStatus(int i,ODHDSTATUS odhdStatus){SetCatStatus((ODHDCATEGORY)i,odhdStatus);};



	virtual void			GetTitle(ODHDCATEGORY odhdCat,std::wstring &strTitle);//strTitle return combinated string of all title(name)s

	virtual std::wstring	GetTitle(ODHDCATEGORY odhdCat);

	virtual std::wstring	GetTitle(int i);



	virtual void			GetResult(ODHDCATEGORY odhdCat,std::wstring & strResult);//strResult return combinated string of all inspect's result

	virtual std::wstring	GetResult(ODHDCATEGORY odhdCat);

	virtual std::wstring	GetResult(int i);



	virtual BOOL			GetNote(std::wstring &strNote){return FALSE;};

	virtual	void			SetProgDlg(CHdrProgDlg *pHdrProgDlg){m_progDlg=pHdrProgDlg;};

	virtual void			SetProgTitle(LPCWSTR wzItem){if(m_progDlg)m_progDlg->SetInspectItem(wzItem);};

	virtual void			SetProgTitle(ODHDCATEGORY odhdCat){SetProgTitle(m_pCatTitle[(int)odhdCat].m_Title.c_str());};

	virtual void			MakeStep(int step){if(m_progDlg)m_progDlg->MakeStep(step);};

	virtual void			MakeProgress(LPCWSTR wzTitle,int step){SetProgTitle(wzTitle);MakeStep(step);};



	virtual BOOL			IsValidPropertyName(LPCWSTR strPropName)=0;

	virtual void			PrintInspectResult();

	virtual BOOL			GetPropertyValueByName(CComPtr<IDispatch> pProps, LPCWSTR pwzName, LPWSTR pwzValue, int nSize)

	{

		HRESULT hr   = S_OK;

		BOOL    bGet = FALSE;

		VARIANT result; VariantInit(&result);

		VARIANT x; x.vt = VT_BSTR; 

		

		x.bstrVal = ::SysAllocString(pwzName);

		hr=DispatchCallWraper(pProps,DISPATCH_PROPERTYGET, &result, L"Item", 1, x);



		CComPtr<IDispatch> pProp = result.pdispVal;

		SysFreeString(x.bstrVal);



		if(pProp)

		{

			hr=DispatchCallWraper(pProp,DISPATCH_PROPERTYGET, &result, L"Value", 0);

			if(result.bstrVal)

			{

				bGet = TRUE;

				wcsncpy_s(pwzValue, nSize, result.bstrVal, _TRUNCATE);

			}

		}

		::SysFreeString(x.bstrVal);

		return bGet;

	};

#ifndef WSO2K3

	virtual BOOL			InspectorExist(Office::DocumentInspectors* spInspectors,int iInspectorIndex,TCHAR *szInspectorName);

	virtual ODHDSTATUS		InspectByInspector(Office::DocumentInspectors* spInspectors,ODHDCATEGORY odhdCat,ODHDITEM odhdItem,DWORD* dwInspectFail=NULL);

	virtual ODHDSTATUS		InspectByInspector(Office::DocumentInspectors* spInspectors,int iInspectorIndex,ODHDCATEGORY odhdCat,ODHDITEM odhdItem,DWORD *dwInspectFail=NULL);

	virtual BOOL			RemoveByInspector(Office::DocumentInspectors* spInspectors,ODHDCATEGORY odhdCat,ODHDITEM odhdItem);

	virtual BOOL			RemoveByInspector(Office::DocumentInspectors* spInspectors,int iInspectorIndex,ODHDCATEGORY odhdCat,ODHDITEM odhdItem);

#endif

	virtual void			RecordInspectResult(ODHDCATEGORY odhdCat,

												ODHDITEM odhdItem,

												ODHDSTATUS odhdStatus,

												CComBSTR bstrItemTitle,

												CComBSTR bstrItemResult,

												long itemFoundCount=0,

												long itemShowCount=0)

	{

		USES_CONVERSION;

		LPCTSTR   szStr =   W2T(bstrItemTitle);

		std::wstring strTitle(szStr);

		szStr=W2T(bstrItemResult);

		std::wstring strResult(szStr);

		RecordInspectResult(odhdCat,odhdItem,odhdStatus,strTitle,strResult,itemFoundCount,itemShowCount);

		

	}

	virtual void			RecordInspectResult(ODHDCATEGORY odhdCat,

												ODHDITEM odhdItem,

												ODHDSTATUS odhdStatus,

												std::wstring strItemTitle,

												std::wstring strItemResult,

												long itemFoundCount=0,

												long itemShowCount=0)

	{

		CODHDCategory *pCat=NULL;

		CODHDItem *		pItem=NULL;

		ODHDSTATUS		newCatStatus=odhdStatus;



		FilterByCategory(odhdCat, odhdStatus);

		

		assert(m_pCatManager);

		pCat=m_pCatManager->GetHDCategory(odhdCat);

		pCat->SetCatStatus(odhdStatus);



		assert(pCat);

		pItem=pCat->GetHDItem(odhdItem);

		assert(pItem);

		

		pItem->SetItemTitle(strItemTitle);

		pItem->SetItemResult(strItemResult);

		pItem->SetItemCount(itemFoundCount);

		pItem->SetItemNumShow(itemShowCount);

		/*

		switch(newCatStatus)

		{

		case INSPECT_FAILED:

			break;

		case INSPECT_NONE:

			break;

		case INSPECT_HAVE:

			break;

		case INSPECT_REMOVED:

			break;

		default:

			break;

		}*/

	};

	std::wstring	GetFilePath(){return m_strSrcFileName;};

	long			GetSize(){return m_Count;};



	void SetHDRFile(HDRFile *pHdrFile) { m_pHDRFile = pHdrFile; }

	HDRFile* GetHDRFile(){return m_pHDRFile;}

private:

	StaticCatTitle	*m_pCatTitle;

	long			m_Count;

	long			m_lRemoveButtonVisible;

protected:

	BOOL	HasDocumentProperties(CComPtr<IDispatch> pDisp);

	CHdrProgDlg*		m_progDlg;

	std::wstring		m_strTempFileName;

	std::wstring		m_strSrcFileName;

	CODHDCatManager*	m_pCatManager;



	HDRFile*			m_pHDRFile;



};



class HdrAction : public HdrActionBase

{

public:

    typedef struct _CONTEXT

    {

        std::vector<CODHDInspector *>* pInspectors;

        HelpUrlVector* pvecHelpUrls;

    }CONTEXT, *LPCONTEXT;



	void OnRemove(int nAttachmentIndex, int nItemIndex, std::wstring& strBody,std::wstring& strNote,

                  int* pnStatus, LPVOID pContext);

    void OnNext(int nAttachmentIndex, std::wstring& strAttach,

                std::wstring& strHelpUrl, LPVOID pContext);

};



#endif //__H_ODHD__

