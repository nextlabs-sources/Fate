#pragma once

#include <map>
#include <set>
#include "FileTagPanel.h"
#include "datatype.h"
#include "FileTag.h"
#include "PABase.h"
#include "UIInterface.h"

#include <vector>
#include <string>

typedef struct tagHCADDTAGINFO
{
	vector<pair<wstring,wstring>> vecTagInfo;
	wstring strSrcPath;
	wstring strDstPath;
	int   AddTagStatus;
	wstring strLogID;
}HCADDTAGINFO,*PHCADDTAGINFO;

typedef DWORD  (WINAPI *CREATEMAINFRAME)(		const HWND _hChildWnd  ,
										 const BOOL _bIsModel , 
										 const wchar_t* _pszHelpURL ,
										 const wchar_t* _strBtName,
										 const pafUI::BTSTATUS _BTStatu ,
										 const HWND _hParent ,
										 const wchar_t* _pszTitle , 
										 const wchar_t* _pszDescription  ) ;
typedef UI_STATUS (WINAPI *GETPARENTWINDOW)(HWND &_hWnd, HWND hParent) ;
typedef UI_STATUS  (WINAPI *SETNEXT_OKCCALLBACK)( /*CALLBACK* pFunc*/pafUI::ONCLICKNEXTBT pFunc ,PVOID _pData, HWND hParent);
typedef UI_STATUS(WINAPI *PAUI_FUN_ShowHierarchicalClassifyDlg)(HWND hParentWnd, const wchar_t* pszFileName, bool bLastFile, const wchar_t* pszDescript,
	const wchar_t* pszXmlDefine, const wchar_t* pOldTags, wchar_t** szOutAddTagBuf, wchar_t** szOutDelTagBuf);
typedef void (WINAPI *PAUI_FUN_ReleaseBuffer)(wchar_t* pBuf);

class CFileTagMgr
{
public:
	CFileTagMgr(void);
	~CFileTagMgr(void);

public:
	DWORD ShowFileTagPanel();

	PABase::PA_STATUS DoFileTaggingAssistant(PABase::PA_PARAM& _iParam, const HWND _hParentWnd, bool bEncryptBeforeTag = false,vector<std::tr1::shared_ptr<HCADDTAGINFO>> *pVecHCInfo = NULL);
	PABase::PA_STATUS DoFileTaggingAssistant2(PABase::PA_PARAM& _iParam, const HWND _hParentWnd,vector<pair<wstring,wstring>>* pSrcTagsPair,vector<pair<wstring,wstring>>* pDstTagsPair);
	BOOL NextFileTaggingItem(smart_ptr<FILETAG_ITEM>& spItem, BOOL& bNextItem, CFileTagPanel*& pPanel);
	BOOL GetCurrentItem(smart_ptr<FILETAG_ITEM>& spItem);
	std::wstring GetOKButtonText();
	//get the current instance
	HINSTANCE GetCurrentInstance();

	CFileTag* GetFileTag();
	HWND GetFileTagPanelWnd();
	void ShowViewResetTagsDlg(LPCWSTR pszFileName);

	void AddLog( 
				std::wstring wstrlogIdentifier,
				std::wstring wstrAssistantName, 
				std::wstring wstrAssistantOptions, 
				std::wstring wstrAssistantDescription, 
				std::wstring wstrAssistantUserActions, 
				PABase::ATTRIBUTELIST &optAttributes);
	std::wstring GetResString(DWORD dwID);

	HWND GetMailWindow(){return m_hParentWnd;}

	static BOOL FileTypeSupport(LPCWSTR wszFileName);
	BOOL DoCurrentManualTagging();

	PABase::PA_STATUS DeleteFileLastModifyTimeTag(const wchar_t* wszFile);
    PABase::PA_STATUS DoTaggingOnFile(const wchar_t* wszFileName,const vector<pair<wstring,wstring>>* pVecTags);
protected:
	int ParseParameters(PABase::PA_PARAM &_iParam, bool bEncryptBeforeTag = false);
	
	
private:
	//for automatic-tag obligation
	BOOL HasAutomaticTag_Obligation(IN PABase::OBJECTINFO& obj, OUT std::list<smart_ptr<FILETAG>>& listFileTagItem, OUT std::map<std::wstring, std::wstring>& mapProps);
	void MergeAutomaticTag_Obligation(LPCWSTR pszFileName, LPCWSTR pszDisplayName, std::list<smart_ptr<FILETAG>>& listFileTagItem, LPCWSTR pszTempFile, std::map<std::wstring, std::wstring>& mapProps);
	
	//for manual-tag obligation
	BOOL HasManualTag_Obligation(IN PABase::OBJECTINFO& obj, OUT std::list<smart_ptr<FILETAG>>& listFileTagItem, OUT std::map<std::wstring, std::wstring>& mapProps);
	void MergeManualTag_Obligation(LPCWSTR pszFileName, LPCWSTR pszDisplayName, std::list<smart_ptr<FILETAG>>& listFileTagItem, LPCWSTR pszTempFile, std::map<std::wstring, std::wstring>& mapProps);

	//for Hierarchical Structure of Classification obligation
	int GetHSCTag_Obligation(IN PABase::OBJECTINFO& obj, LPCWSTR pszFileName, LPCWSTR pszDisplayName);

	//common APIs for automatic/manual tag obligations
	std::wstring GetTagNameInObligation(IN PABase::OBLIGATION& ob);
	void GetTagValuesInObligation(IN PABase::OBLIGATION& ob, OUT std::list<std::wstring>& listValues);
	std::wstring GetDescriptionInObligation(IN PABase::OBLIGATION& ob);
	std::wstring GetTargetInObligation(IN PABase::OBLIGATION& ob);
	BOOL GetFileTagItemByFileName(IN LPCWSTR pszFileName, IN std::list<smart_ptr<FILETAG_ITEM>>& listItems, OUT smart_ptr<FILETAG_ITEM>& listFileItems);
	BOOL Is_IndicatedObligation(IN PABase::OBLIGATION& ob, IN LPCWSTR pszOB);


	//Display all FILETAG_ITEM
	void DisplayAllFileTaggingItems();

	BOOL DoAutomaticTagging();
	int DoHierarchicalStructureClassification(HWND hParentWnd,vector<std::tr1::shared_ptr<HCADDTAGINFO>> *pVecHCInfo);
    int GetTagsAndLastHSCPos(std::vector<std::list<smart_ptr<FILETAG_PAIR>>>& tagpair);

	BOOL GetTheFirstOBForTagging(PABase::OBLIGATIONLIST& listOB, OUT PABase::OBLIGATION& obTagging);
	
	BOOL GetFileterTagsName(PABase::OBLIGATIONLIST& listOB, OUT std::set<std::wstring>& FileterTagsName);

	void GetHSCInfoFromObligation(PABase::OBLIGATION* obligation, HSC_ITEM& hscItem);

	size_t FromatHierarchicalClassificationTag(LPCWSTR wszTag, std::list<smart_ptr<FILETAG_PAIR>>& lstTag);

	std::wstring GetValueFromObligation(PABase::OBLIGATION& ob, LPCWSTR pszKeyName);

	bool RemoveUnsupportFileTypeItem();
	void GetDelTag(LPCWSTR wszTag, std::list<std::wstring>& lstDelTag);
	BOOL DeleteOldHierarchicalStructureTag(HSC_ITEM* pHscItem, std::list<std::wstring>& lstDelTag);

	BOOL NeedDoHierarchicalStructureTag(smart_ptr<HSC_ITEM> pHscItem, std::list<smart_ptr<FILETAG_PAIR>>& lstTag);

protected:
	

	std::list<smart_ptr<FILETAG_ITEM>>		m_listManualItems;//stores all ¡°manual tag¡± items.
	std::list<smart_ptr<FILETAG_ITEM>>		m_listAutoItems;
	std::list<smart_ptr<HSC_ITEM>>		    m_listHierarchicalClassification;
	int 									m_nIndexItem;//the index of the current item

	std::wstring							m_strAutoTag_Obligation;
	std::wstring							m_strManualTag_Obligation;
	std::wstring                            m_strHierarchicalClassify_Obligation;

	HINSTANCE								m_hInstance;

	CFileTag								m_FileTag;

	HWND									m_hParentWnd;
	HWND									m_hWnd;

	std::wstring							m_strDefaultHint;
	BOOL									m_bLastPA;
	std::wstring							m_strLastButtonText;
	PABase::LogFunc							m_pLogFunc;
	PVOID									m_lpLogCtx;

	SETNEXT_OKCCALLBACK						m_fpSetNext_OKCallBack;
	CREATEMAINFRAME							m_fpCreateMainFrame;
	GETPARENTWINDOW							m_fpGetParentWindow;
	PAUI_FUN_ShowHierarchicalClassifyDlg    m_fpShowHSCDlg;
	PAUI_FUN_ReleaseBuffer                  m_fpReleaseBuffer;

	PABase::ACTION							m_action;


public:
	/*
	*\brief: used to collect all tag pairs base on tag obligation.
			 if it is not NULL, it means ,we only need to get interactive tag pair, 
			 invoker responsible for add tag.
	*/
	vector<pair<wstring,wstring>>*			m_pSrcTagPairs;	
	vector<pair<wstring,wstring>>*			m_pDstTagPairs;
public:
	CFileTagPanel*	m_pFileTagPanel;
};
