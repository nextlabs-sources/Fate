#pragma once

#include "obligations.h"
#include "NLOfficePEP_Comm.h"

#define NLOBMGRINSTANCE (CNLObMgr::NLGetInstance())


typedef enum tyEMNLOFFICE_ENCPERMISSION
{
	emNLEncPermissionUnKnwon = 0,
	emNLEncPermissionNeedDeny,
	emNLEncPermissionNeedEncryption,	
	emNLEncPermissionDoNothing
}EMNLOFFICE_ENCPERMISSION;

typedef struct tagOverlayInfo
{
	HDC hDC;
}OverLayInfo;


class CNLLogicFlag; // forward declaration
class CNLData;		// forward declaration

class CNLObMgr
{
private:
	CNLObMgr(void);
	~CNLObMgr(void);
public:
	static CNLObMgr& NLGetInstance();

public: // public for initialize
	/*
	*\ Brief: if we want to use this class, we must initialize it. it will initialize tag cache and the encryption flag
	*\ Parameter
	*		[IN] wstrFilePath: the file full path
	*   [IN] pDoc: if this is not NULL, use it to update the tags, for not-SE file it will more fast
	*		[IN] bGoldenTagFlag: true: save the old golden tags into the file tag cache; false, nothing to do
	*\ Return value: 
	*		true: initialize success, false: initialize failed.
	*/	
	bool NLInitializeObMgr( _In_ const wstring& wstrFilePath, _In_opt_ IDispatch* pDoc = NULL, _In_ const bool bGoldenTagFlag = true );
	
	// Tag cache initialize flag
	void NLSetObMgrInitializeFlag( _In_ const wstring& wstrFilePath, _In_ const bool& bInitialize );
	bool NLGetObMgrInitializeFlag( _In_ const wstring& wstrFilePath );

	// Tag pre open flag
	ActionResult NLGetPreOpenEvaActionResult( _In_ const wstring& wstrFilePath, _In_ const bool kbAutoCheckFromPC, _In_ const ActionResult& kemDefaultActionResultIfFailed) throw();
	void NLSetPreOpenEvaActionResult( _In_ const wstring& wstrFilePath, _In_ const ActionResult& emActionResult );
	bool IsContainsPreOpenSpecifyEvaActionResult(_In_ const ActionResult& kemActionResultIn) const throw();

private:
	// revert all tag cache flag as false
	void NLRevertObMgrInitializeFlag();
	void SetPreopenEvaResultValidTimeMs(_In_ const DWORD kdwLocalFilePreopenEvaResultValidTimeMs, _In_ const DWORD kdwNetFilePreopenEvaResultValidTimeMs) throw() 
        { m_dwLocalFilePreopenEvaResultValidTimeMs = kdwLocalFilePreopenEvaResultValidTimeMs; m_dwNetFilePreopenEvaResultValidTimeMs = kdwNetFilePreopenEvaResultValidTimeMs; }
	ActionResult NLInnerGetPreOpenEvaActionResult( _In_ const wstring& wstrFilePath ) const throw();
	bool IsValidPreopEvaActionResult(_In_ const std::wstring& wstrFilePath, _In_ const std::pair<ActionResult, DWORD>& pairActionResult, _In_ const DWORD kdwCurTick) const throw();

public: // public for obligation, tag & encryption
	/*
	*\ Brief: this used to get the the source file permission from policy.
	*\ Parameter:
	*		[in] pwchSource: the source file path, this can be NULL. if it is NULL, it will use the current active file path that we saved in the cache to query policy
	*   [in] emAction: the current action
	*   [in] pwchDest: the destination file path, only copy action need it
	*   [in] noise_level: noise level used to control the bubble pop up:
	*            default value is CE_NOISE_LEVEL_USER_ACTION, if no need to pop up bubble, can set it as: CE_NOISE_LEVEL_APPLICATION
	*   [in] wstrapp_attr_value: used to specific pep or user, this value can be ATTR_NAME_OFFICE or ATTR_NAME_USER( classify user change custom tags use ATTR_NAME_USER )
	*\ Return value: 
	*	  
	*/	
	ActionResult NLGetEvaluationResult( _In_ const WCHAR* pwchSource, _In_ const OfficeAction& emAction,
		_In_opt_ const WCHAR* pwchDest,	
		_In_ CENoiseLevel_t noise_level = CE_NOISE_LEVEL_USER_ACTION, 
		_In_ const wstring wstrapp_attr_value = L"OfficePEP");

	/*
	*Brief: this function is used for evaluating multiple file at one time
	*[in]pwchDest: 
	*/
	
	ActionResult NLGetEvaluationResult( _In_ vector<wstring>& vecwstrSource, _In_ const OfficeAction& emAction,
		_In_opt_ const WCHAR* pwchDest,	
		_In_ CENoiseLevel_t noise_level = CE_NOISE_LEVEL_USER_ACTION, 
		_In_ const wstring wstrapp_attr_value = L"OfficePEP");

	/*
	*\ Brief: this function used to process action as common work follow.
	*			if you don't initialize the ObMgr flag, this function will initialize it and set the initialize flag false before it return.
	*			I suggest you initialize the ObMgr by yourself before you invoke this function.
	*\ Parameter
	*		[in] pDoc: the IDispatch of the file and it can be NULL, for no-SE file, use pDoc write tags more fast
	*   [in] pwchSrcFilePath: source file path, must not be NULL
	*   [in] pwchDesFilePath: destination file path, only copy action use this, others this must be NULL
	*   [in] emAction: the current action
	*\ Return value: 
	*	  to see STUNLOFFICE_RESULT
	*/	
	ProcessResult NLProcessActionCommon( 
		_In_opt_ IDispatch* pDoc, 
		_In_ const wchar_t* pwchSrcFilePath,
		_In_opt_ const wchar_t* pwchDesFilePath,
		_In_ const OfficeAction& emAction 
		);
		
	/*
	*\ Brief: this is used to process protect view open action. 
	*\ Parameter
	*		[in] wstrPath: file full path which need processed.
	*		[in] pDoc: document IDispath used for file tags.
	*\ Return value: 
	*		to see STUNLOFFICE_RESULT
	*/	
	ProcessResult NLProcessActionProtectedView( _In_ const wstring& wstrPath, _In_ IDispatch* pDoc );

private:
	// this function will parse obligation, encrypt file and synchronize tags
	// this function no need check the parameter
	bool NLDoObligationCommon( 
		_In_opt_	IDispatch*	pDoc,
		_In_  nextlabs::Obligations&	obs,
		_In_  const wchar_t* const		pwchSourceFilePath,
		_In_opt_  const wchar_t* const		pwchDestinationFilePath,
		_In_  const OfficeAction&							emAction,
		_In_  const vector<pair<wstring,wstring>>&	vecSrcTag,
		_In_opt_  const OverLayInfo*						pStuNLOverlayInfo
		);

	/*
	*\ Brief: this function is used to do obligation, it will do all the obligations that the current action support and return the tags and the encryption flag. 
	*			 Note that the obligations must query policy by PEP not USER 
	*\ Parameter
	*		[IN] obs: NextLabs obligation structure, this obligation must query policy  by PEP.
	*		[IN] pwchSourceFilePath: source file full path
	*		[IN] pwchDestinationFilePath: source file full path, now only copy action we care the destination path, others this parameter must be NULL.
	*		[IN] emAction: the current action 
	*		[IN] vecSrcTag:	 the current source file tags. We need pass this parameter for parse some obligations and if process module use a tag cache and we can't get the accurate tags by file path 
	*		[IN] pStuNLOverlayInfo:	used for overlay obligation, if not used you can set it NULL
	*		[OUT] bIsNeedEncrypt: out parameter and tell the invoker if there is encryption obligation. Exist it is true,otherwise it is false
	*		[OUT] vecObligationTag: the tags get from all the support obligations
	*\ Return value: 
	*		if we parse all the obligations right it will return true, otherwise return false.
	*/	
	bool NLParseObligation( 
		_In_  nextlabs::Obligations&	obs,
		_In_  const wchar_t* const		pwchSourceFilePath,
		_In_opt_  const wchar_t* const		pwchDestinationFilePath,
		_In_  const OfficeAction&							emAction,
		_In_  const vector<pair<wstring,wstring>>&	vecSrcTag,
		_In_opt_  const OverLayInfo*						pStuNLOverlayInfo,
		_Out_ bool&	                                bIsNeedEncrypt,	
		_Out_ vector<pair<wstring,wstring>>&	vecObligationTag,
		_Inout_opt_ vector<pair<wstring,wstring>>*	pvecAllFileTags = NULL
		);
	
	/*
	*  Note: this function do not check the parameters
	*\ Brief: this function only used to check the SE file permissions
	*         for save as SE file only allow save to local and SE file do not allow send action
	*\ Parameter:
	*		[in] pwchSourceFilePath: source file path, this must be not NULL
	*   [in] pwchDestinationFilePath: destination file path, only copy action need this path
	*   [in] emAction: the current action
	*\ Return value: 
	*		to see EMNLOFFICE_ENCPERMISSION
	*/	
	EMNLOFFICE_ENCPERMISSION NLGetSEPermissopnForCurAction( _In_ const wchar_t* const	pwchSourceFilePath,
																				_In_opt_  const wchar_t* const pwchDestinationFilePath,
																				_In_  const OfficeAction& emAction );
	
	/*
	*\ Brief: function tools for save office file by ourself, but now no used. 
	*\ Parameter:
	*		[IN] pDoc: the document IDispatch
	*\ Return value: 
	*		return true save success otherwise failed
	*/	
	bool NLSaveFileByPEP( _In_ IDispatch* pDoc );

	// for Synchronize the golden tags
public:
	// init or uninit only used for asynchronous I/O
	void NLInitializeSynchronizeGoldenTags();
	void NLUnInitializeSynchronizeGoldenTags();	
	/*
	*\ Brief: this function used to start up synchronize the golden tags
	*\ Parameter:
	*		[IN] wstrFilePath: the file full path which will be synchronize the golden tags. we can save the golden tags by NLSetGoldenTags before we start synchronize golden tags.
	*		[IN] bIOSynchronize:
	*					true:  I/O synchronous
	*					false: I/O Asynchronous
	*		[IN] pwchTempFilePath: temp file path.
	*					eg: edit office file, it will write text into a temp file and then replace the temp file to the source file
	*							if you want to synchronize the golden tags at this replace time, you should get golden tags from the source file golden tag cache and write the golden tags into the temp file.
	*\ Return value: 
	*		return false: means start synchronize golden tags failed.
	*		return true:  only means the function invoke success. \
	*	 Note: function return true no means the golden tags synchronize success, because we only can known write tags success but we don't known the tags is the golden tags or not.
	*		Success synchronize golden tags need: SE file, right start up time and success start synchronize	
	*/	
	bool NLStartSynchronizeGoldenTags( _In_ const wstring& wstrFilePath,  _In_ const bool bIOSynchronize = false, _In_ const wchar_t* pwchTempFilePath = NULL );
	
	bool NLStartSynchronizeGoldenTags( _In_ const bool bIOSynchronize = false );
	
private:
	/*
	*\ Brief: note we always read tags from tag cache
	*\ Parameter: see about NLInnerReadTag
	*\ Return value: see about NLInnerReadTag
	*/	
	bool NLReadTag( _In_ const wstring& wstrFilePath, _Out_ vector<pair<wstring,wstring>>& vecFileTagPair );
	bool NLReadTag( _In_ IDispatch* pDoc,         _Out_ vector<pair<wstring,wstring>>& vecFileTagPair );
	
	/*
	*\ Brief: note when we write tags we will alway update the tag cache 
	*\ Parameter:
	*		[IN] wstrFilePath: the file path
	*		[IN] vecFileTagPair: file tags
	*		[IN] bIsAllFileTag: true, vecFileTagPair is all the file tags; false: vecFileTagPair is just the obligation tags
	*		[IN] bIfNeedErrHanding: need use error handling or not
	*\ Return value: success write tags or not.
	*/
	bool NLWriteTag( _In_ const wstring& wstrFilePath, _In_ const vector<pair<wstring,wstring>>& vecFileTagPair, _In_ const bool bIsAllFileTag, _In_ const bool bIfNeedErrHanding = false, _In_ const bool bFourceGoldenTag = false );
	bool NLWriteTag( _In_ IDispatch* pDoc,         _In_ const vector<pair<wstring,wstring>>& vecFileTagPair, _In_ const bool bIsAllFileTag, _In_ const bool bIfNeedErrHanding = false, _In_ const bool bIfNeedSave = true );
	void NLSynchroSETagCache( _In_ const wstring& wstrFilePath, _In_ const vector<pair<wstring,wstring>>& vecFileTagPair );

	// Golden tags back
	void NLSetGoldenTagCache( _In_ const wstring& wstrFilePath, _In_ const vector<pair<wstring,wstring>>& vecGoldeTags );
	bool NLGetGoldenTagCache( _In_ const wstring& wstrFilePath, _Out_ vector<pair<wstring,wstring>>& vecGoldeTags );
	void NLGetGoldenTagCache( _Out_ map<wstring, STUNLOFFICE_GOLDENTAG>& mapGoldenTags );

	// File tags cache	
	void NLSetFileTagCache( _In_ const wstring& wstrFilePath, _In_ const vector<pair<wstring,wstring>>& vecTagPair );
	bool NLGetFileTagCache( _In_ const wstring& wstrFilePath, _Out_ vector<pair<wstring,wstring>>& vecTagPair );
	
	/*
	*\ Brief: update the file golden tags to golden tag cache
	*\ Parameter
	*		[IN] wstrFilePath: source file path
	*/	
	void NLUpdateGoldenTagCache( _In_ const wstring& wstrFilePath );

public:
	/*
	*\ Brief: this function is only used for save/save as and this function is used to encrypt the temp file during the save/save as action.
	*\ Parameter:
	*		[IN]	emOfficeAppType: office type: word, excel, PPT
	*		[IN]	wstrFilePath: the file full path
	*		[IN]	bForSave: this is only used for Save/Save as action. true: save action, false: save as action
	*		[IN]	bUpdateGoldenTagCache: true, set the current SE file tag into the golden tag cache
	*\ Return value: 
	*		Encrypt succeed return true, otherwise return false.
	*/
	bool NLEncryptTempFile( _In_ const AppType& emOfficeAppType, _In_ const wstring& wstrFilePath, _In_ const bool bForSave, _In_ const bool bUpdateGoldenTagCache = false );
	bool NLEncryptTempFile( _In_ const AppType& emOfficeAppType, _In_ const wstring& wstrFilePath, _In_ const vector<pair<wstring,wstring>>& vecFileTags, _In_ const bool bForSave );

	/*
	*\ Brief: Encrypt file, SE encryption, we only can encrypt the file when it closed
	*\ Parameter:
	*		[IN]	wstrFilePath: the file full path.
	*\ Return value: 
	*		Encrypt succeed return true, otherwise return false.
	*/	
	bool NLEncryptFile( _In_ const wstring& wstrFilePath );
	bool NLEncryptFile( _In_ const wstring& wstrFilePath, _In_ const vector<pair<wstring,wstring>>& vecFileTags );

public:
	/*
	*\ Brief: this is used to judge if it is the encrypt file
	*\ Parameter:
	*		[IN]	wstrFilePath: the file full path.
	*\ Return value: 
	*		if the file is encrypt file return true, otherwise return false.
	*/	
	bool NLIsEncryptedFile( _In_ const wstring& wstrFilePath );
	
	// synchronize the golden tags.
	// all synchronize golden tags methods use this function write the golden tags into the file.
	bool NLSynchroniseFileTags( _In_ const wstring& wstrFilePath, _In_ const vector<pair<wstring,wstring>>& vecGoldenTags );

	// public for office runtime flags
public:
	// Current runtime status
	void NLSetCurRibbonEvent( _In_ const wstring& wstrFilePath, _In_ const STUNLOFFICE_RIBBONEVENT& stuCurRibbonEvent );
	bool NLGetCurRibbonEvent( _In_ const wstring& wstrFilePath, _Out_ STUNLOFFICE_RIBBONEVENT& stuCurRibbonEvent );

	void NLSetCurComNotify( _In_ const wstring& wstrFilePath, _In_ const NotifyEvent& stuCurComNotify );
	bool NLGetCurComNotify( _In_ const wstring& wstrFilePath, _Out_ NotifyEvent& stuCurComNotify );

	// Close flag
	void NLSetCloseFlag( _In_ const wstring& wstrFilePath, _In_ const bool bIfNeedClose );
	bool NLGetCloseFlag( _In_ const wstring& wstrFilePath );
	
	// Encrypt flag
	void NLSetEncryptRequirementFlag( _In_ const wstring& wstrFilePath, _In_ const bool bNeedEncrypt );
	bool NLGetEncryptRequirementFlag( _In_ const wstring& wstrFilePath );

	// pDoc cache
	void NLSetRealsePDocFlag( _In_ const wstring& wstrFilePath, _In_ const bool bNeedRealse );
	void NLSetFilePDocCache( _In_ const wstring& wstrFilePath,  IDispatch* pDoc );
	CComPtr<IDispatch> NLGetFilePDocCache( _In_ const wstring& wstrFilePath );
	
	// Current file path
	void NLSetCurActiveFilePath( _In_ const wstring& wstrFilePath );
	wstring NLGetCurActiveFilePath(  );
	
	// for bug24122, used for PPT right click to do print
	void NLSetPPTPrintActiveFilePath( _In_ const wstring& wstrPPTPrintActiveFilePath );
	wstring NLGetPPTPrintActiveFilePath(  );

	// No need handle flag
	void NLSetNoNeedHandleFlag( _In_ const wstring& wstrFilePath, _In_ const OfficeAction& emAction, _In_ const bool& bNoNeedHandle  );
	bool NLGetNoNeedHandleFlag( _In_ const wstring& wstrFilePath, _In_ const OfficeAction& emAction );
	
	// Action start up
	void NLSetActionStartUpFlag( _In_ const wstring& wstrFilePath, _In_ const OfficeAction& emAction,
		                           _In_ const bool& bActionStartUp,  _In_opt_ const ActionFlag* pstuOfficeActionFlag = NULL );
	bool NLGetActionStartUpFlag( _In_ const wstring& wstrFilePath, _In_ const OfficeAction& emAction,
		                           _Out_opt_ ActionFlag* pstuOfficeActionFlag = NULL );

	// Hyperlink file path cache
	void NLSetLinkFilePath( _In_ const wstring& wstrHyperLinkFilePath );
	wstring NLGetLinkFilePath( );
	
	// Clear cache
	void NLClearFileCache( _In_ const wstring& wstrFilePath );
	void NLClearCache( void );
	
	// Clear golden tag cache, the golden tag cache only can clear when it success synchronized into the file
	void NLClearGoldenTagFileCache( _In_ const wstring& wstrFilePath );
	void NLClearGoldenTagCache( void );
	
	// Set save as flag
	void NLSetSaveAsFlag( _In_ const wstring& wstrDesFilePath, _In_ const bool bSaveAsFlag, _In_ const wstring& wstrSrcFilePath );
	bool NLGetSaveAsFlag( _In_ const wstring& wstrDesFilePath, _Out_ wstring& wstrSrcFilePath );
	
	// Open in IE, file path cache
	void NLSetIEFilePathCache( _In_ const IDispatch* pDoc, _In_ const wstring& wstrIEFilePath );
	wstring NLGetIEFilePathCache( _In_ const IDispatch* pDoc );
	
	// office user classify custom tags flag
	void NLSetClassifyCustomTagsFlag( _In_ const wstring& wstrFilePath, _In_ const bool& bUserClassifyCustomTags );
	bool NLGetClassifyCustomTagsFlag( _In_ const wstring& wstrFilePath );

	// Word open in IE flag
	void NLSetIEOpenFlag( _In_ const wstring& wstrFilePath, _In_ const bool& bIEOpenFlag );
	bool NLGetIEOpenFlag( _In_ const wstring& wstrFilePath );
	
 	bool GetPasteEvaResult(const wstring& strPath,bool& bAllow);
 	void SetPasteEvaResult(const wstring& strPath,bool bAllow);

	// edit action triggered by user save operation
	void NLSetUserSaveFlag( _In_ const wstring& wstrFilePath, _In_ const bool& bIsUserSave );
	bool NLGetUserSaveFlag( _In_ const wstring& wstrFilePath );
	
	// close flag for PPT
	void NLSetPPTUserCancelCloseFlag( _In_ const wstring& wstrFilePath, _In_ const bool bIsPPTUserCancelClsoe );
	bool NLGetPPTUserCancelCloseFlag( _In_ const wstring& wstrFilePath );
private:
	void NLSetEncryptFileFlag( _In_ const wstring& wstrFilePath, _In_ const bool bIsEncryptFile );
	bool NLGetEncryptFileFlag( _In_ const wstring& wstrFilePath );
	
	/*
	*\ Brief: this function is used to synchronize the cache tags and the file tags
	*\ Parameter
	*		[IN] wstrFilePath: the file full path
	*		[IN] pDoc: com interface and we will use this to read the file custom tags 
	*		[IN] bRefreshTagCache: 
	*					true: clear the tag cache and then read tags from file to update the tag cache
	*					false: clear the file tags and get the tags from tag cache and then write it into the file.
	*		[OUT] vecFileTagPair: return the update tags
	*\ Return value: 
	*		if we update tags success the function return true, otherwise return false.
	*/	
	bool NLInnerUpdateTags( _In_ const wstring& wstrFilePath, _In_ const bool& bRefreshTagCache, _Out_ vector<pair<wstring,wstring>>& vecFileTagPair );
	bool NLInnerUpdateTags( _In_ IDispatch* pDoc,         _In_ const bool& bRefreshTagCache, _Out_ vector<pair<wstring,wstring>>& vecFileTagPair );
	
	/*
	*\ Brief: this two functions are used to read tags, pDoc or file path 
	*\ Parameter
	*		[IN] wstrFilePath: file full path
	*		[IN] pDoc: com interface and we will use this to read the file custom tags 
	*		[OUT] vecFileTagPair: return the file tag pair
	*		[IN] bReadFromTagCache: the flag for tag cache. If it is true we read tags from tag cache, otherwise we read tags from file
	*			  Note: I think we only need to read tag from file once at open action and then save the tag into cache.
	*\ Return value: 
	*		if we read tags success the function return true, otherwise return false.
	*/	
	bool NLInnerReadTag( _In_ const wstring& wstrFilePath, _Out_ vector<pair<wstring,wstring>>& vecFileTagPair, _In_ bool bReadFromTagCache = true );
	bool NLInnerReadTag( _In_ IDispatch* pDoc,         _Out_ vector<pair<wstring,wstring>>& vecFileTagPair, _In_ bool bReadFromTagCache = true );
	
	/*
	*\ Brief: this two function are used to write tags, pDoc or file path
	*\ Parameter
	*		[IN] wstrFilePath: the file full path
	*		[IN] pDoc: com interface and we will use this to read the file custom tags 
	*		[IN] vecFileTagPair: the tags that we want to write into the file, all file tags
	*		[IN] bUpdateTagCache: if this flag is true we will add the file tags into the cache after we success to write the tags into file.
	*		[IN] bIfNeedErrHanding: this is used for error handling. If this is true means we need pop up error handling dialog when we add tags failed, otherwise no used error handling
	*\ Return value: 
	*		if we write tags success the function return true, otherwise return false.
	*/	
	bool NLInnerWriteTag( _In_ const wstring& wstrFilePath, _In_ const vector<pair<wstring,wstring>>& vecFileTagPair, _In_ bool bUpdateTagCache = true, _In_ bool bIfNeedErrHanding = false );
	bool NLInnerWriteTag( _In_ IDispatch* pDoc,         _In_ const vector<pair<wstring,wstring>>& vecFileTagPair, _In_ bool bUpdateTagCache = true, _In_ bool bIfNeedErrHanding = false, _In_ const bool bIfNeedSave = true );
	
	/*
	*\ Brief: every action can do some obligations and some obligation only support by the specified actions.
	*			 here I use a map to keep the relations and this function will initialize the map.
	*/	
	void NLInitMapBetweenActionAndObligation();

	/*
	*\ Brief: this function used to set the obligation flag
	*\ Parameter:
	*		[IN] unFlag: unsigned int, every bit will be an obligation flag(bool), to see STUNLOFFICE_OBLIGATIONMAP
	*\ Return value: 
	*		return a structure STUNLOFFICE_OBLIGATIONMAP
	*/	
	OblMap NLSetObligationFlag( _In_ const UINT32 unFlag = 0 );
	bool NLIsSupportTheObligation( _In_ const OfficeAction emAction, _In_ const ObligationType emObligation );
	
// private for do obligation
private:
	bool NLCheckIfObligationExist( _In_ nextlabs::Obligations& obs, _In_ const ObligationType& emObligation );

private:
	// tools
	/*
	*\ Brief: this function is used to  remove the same tags form vecTagPair and the new tag pair without same tags return by vecOutTagPair 
	*\ Parameter:
	*		[OUT] vecOutTagPair: out tag pair, no same tags
	*		[IN] vecTagPair: source tag pair, this tag pair may be has some same tags
	*		[IN] bIfNeedWholeMatch: 
	*				true:  same tag means, tag name and the tag value are both the same.
	*				false: same tag means, just the tag name is the same the tag is the same tags. In this case the same tag that at the last will be save into vecOutTagPair. 
	*/	
	void NLRemoveSameTags( _Out_ vector<pair<wstring,wstring>>& vecOutTagPair, _In_ const vector<pair<wstring,wstring>>& vecTagPair, _In_ const bool& bIfNeedWholeMatch = true );
	
	/*
	*\ Brief: this function is used to merge the tag with the same tag name. the tag value will be separate by the separator: wstrSeparator
	*\ Parameter
	*		[OUT] vecOutTagPair: out the merge result
	*		[IN] vecTagPair: there are some tags with the same tag name, the function will connect the tag values and return it.
	*		[IN] wstrSeparator: the separator used to separate the tag values with the same tag name
	*/	
	void NLMergeTagsByTagName( _Out_ vector<pair<wstring,wstring>>& vecOutTagPair, _In_ const vector<pair<wstring,wstring>>& vecTagPair, _In_ const wstring wstrSeparator = L"|");
	
	/*
	*\ Brief: the two tag pairs may be has some same tags and this function will use vecTagPair to replace the vecOutTagPair.
	*			 eg: vecSrcTagPair: <itar,yes>, <project, office>; vecTagPair: <itar, no>, <nextlabs,yes> --> the result of vecOutTagPair: <itar, no>, <project, office>, <nextlabs,yes>
	*\ Parameter
	*		[OUT] vecOutTagPair: out tag pair, the replace tag result
	*		[IN] vecSrcTagPair: source tag pair
	*		[IN] vecDesTagPair: use this tag pair to replace vecSrcTagPair
	*/	
	void NLReplaceTagsByTagName( _Out_ vector<pair<wstring,wstring>>& vecOutTagPair, _In_ const vector<pair<wstring,wstring>>& vecSrcTagPair, _In_ const vector<pair<wstring,wstring>>& vecDesTagPair );
	
	void NLReplaceTagsByTagNameEx( _Inout_ vector<pair<wstring,wstring>>& vecOutTagPair, _In_ const vector<pair<wstring,wstring>>& vecTagPair, _In_ const bool bNeedSplitOut );
	/*
	*\ Brief: this function used to split the tag pair. it will use the separator "g_wstrTagValueSeparator" to split the tag values in "vecTagPair".
	*			eg: <itar, yes|no> --> <itar,yes>, <itar,no>
 	*\ Parameter:
	*		[INOUT] vecTagPair: file tags
	*		[IN]    g_wstrTagValueSeparator: separator used to split tag values
	*/
	void NLSplitTagValuesBySeprator( _Inout_ vector<pair<wstring,wstring>>& vecTagPair, _In_ const wstring wstrSeparator = L"|");
private:
	map<OfficeAction, OblMap> m_mapActionWithObligation;	// used to save the relations between action and obligation

	CNLTag				m_theNxtTag;
	CNLEncrypt			m_theNLEncrypt;
	CNLData*			m_pNLData; 
	CNLLogicFlag*		m_pLogicFlag;
	CNLSESynchroGoldenTags m_theSESynchronizeGoldenTags;

	CRITICAL_SECTION m_csObMgrInitialize;
	map<wstring, bool> m_mapObMgrInitialize;

	mutable CRITICAL_SECTION m_csPreOpenEvaActionResult;
    map<wstring, std::pair<ActionResult, DWORD> > m_mapPreOpenEvaActionResult;
	DWORD m_dwLocalFilePreopenEvaResultValidTimeMs;
    DWORD m_dwNetFilePreopenEvaResultValidTimeMs;

	map<wstring,bool>	m_mapPasteResult;
	CRITICAL_SECTION 	m_csmap;
	CRITICAL_SECTION m_csGoldenTag;
};
