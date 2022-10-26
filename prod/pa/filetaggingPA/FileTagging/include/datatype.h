#ifndef _DATA_TYPE_H_
#define _DATA_TYPE_H_ 

class CFileTagMgr;
class CPromptDlg;
class CFileTagViewDlg;

typedef struct struFileTag
{
	std::wstring			strTagName;
	std::wstring			strTagType;
	std::list<std::wstring>		listValues; 
}FILETAG, *LPFILETAG;


typedef struct struFileTag_Pair
{
	std::wstring			strTagName;
	std::wstring			strTagValue; 
}FILETAG_PAIR, *LPFILETAG_PAIR;


typedef struct struFileTag_Item
{
	std::wstring							strHint;
	std::wstring     						strFile;
	std::wstring							strDisplayFile;
	std::list< smart_ptr<FILETAG> >     	listTags;
	BYTE									btTarget;//kevin 2008-10-23
	std::wstring							strTempFilePath;//kevin 2008-10-23

	std::wstring                            strTagOnError;
	std::wstring                            strTagOnUnSupportFileType;
	std::wstring                            strMessageForBlockAction;

}FILETAG_ITEM, *LPFILETAG_ITEM;

typedef struct struAutomaticThreadParam
{
	LPCWSTR								pszFileName;
	CPromptDlg*							pDlg;
	CFileTagMgr*						pMgr;
	std::list<smart_ptr<FILETAG>>*		pList;
	BOOL								bSucceed;
	DWORD								dwErrorID;
}AUTOTHREADPARAM, *LPAUTOTHREADPARAM;

typedef struct struManualThreadParam
{
	std::list<smart_ptr<FILETAG_PAIR>>* pList;
	LPCWSTR								pszFileName;
	CPromptDlg*							pDlg;
	CFileTagMgr*						pMgr;
	BOOL								bSucceed;
	DWORD								dwErrorID;
}MANUALTHREADPARAM, *LPMANUALTHREADPARAM;

typedef struct struRemoveTagThreadParam
{
	std::list<std::wstring>*	pList;
	BOOL						bSucceed;
	LPCWSTR						pszFileName;
	CPromptDlg*					pDlg;
	CFileTagMgr*				pMgr;
	DWORD						dwErrorID;
}REMOVETAGTHREADPARAM, * LPREMOVETAGTHREADPARAM;

typedef struct struGetTagValueByNameThreadParam
{
	CPromptDlg*					pDlg;
	CFileTagMgr*				pMgr;
	FILETAG_ITEM*				pItem;
	BOOL						bSuccess;
	DWORD						dwErrorID;
}GET_TAGVALUE_BYNAME_THREAD_PARAM, *LPGET_TAGVALUE_BYNAME_THREAD_PARAM;

typedef struct struGetAllTagValuesThreadParam
{
	LPCWSTR									pszFileName;
	CPromptDlg*								pDlg;
	std::list<smart_ptr<FILETAG_PAIR>>*		pList;
	CFileTagMgr*							pMgr;
	BOOL									bSuccess;
	DWORD									dwErrorID;
}GETALLTAGVALUESTHREADPARAM, *LPGETALLTAGVALUESTHREADPARAM;


typedef struct struHierarchicalClassification_ITEM
{
	std::wstring							strHint;
	std::wstring     						strFile;						//dest file, it is a temp file.
	std::wstring							strDisplayFile;	  //source file 
	std::wstring     	                    strClassificationXml;
	std::wstring                            strTagOnError;
	std::wstring                            strTagOnUnSupportFileType;
	std::wstring                            strMessageForBlockAction;
	std::wstring                            strTagTarget; // source, dest , both
    FILETIME                                stuFileTime;    // UTC time
    DWORD                                   dwLastModifyTimeInterval;
	std::wstring							strLogID;
	std::wstring                            strOEAction; //force to do / force not to do
}HSC_ITEM, *LHSC_ITEM;


#endif
