#pragma once
/*
*	Office PEP support:
*		1.	Classify: use/pep change file tags( custom tag ), NextLabs ribbon button  
*		2.	Open: open file, initialize
*		3.	Save: edit
*		4.	Save as: save as to the same type
*		5.	Convert: save as another type, DO NOT CARE ABOUT the destination path
*		6.	Send: send & publish, DO NOT CARE ABOUT  the destination path
*		7.	Pates: Copy\Paste\Cut content & \Select Slides --> deny or not and no obligation
*		8.	Insert: insert data, insert object, insert text, DO NOT CARE ABOUT the destination path
*		9.	Change attributes: change file permission: read, write ...
*		10. Print: judge if it allow print ( do evaluation )
*/

#include "stdafx.h"
#include "NLOfficePEP_Comm.h"

class CNLProcess
{
public:
	CNLProcess( );
	~CNLProcess(void);

// trigger the action by action module
public:
	ProcessResult HandleFacet( _In_ const OfficeAction&	emAction, _In_ const EventType& emEventTriggerPoint, _Inout_ STUNLOFFICE_EVENTSTATUS& stuEventStatus );
	
	ProcessResult NLProcessComNotifyWord(  _In_ IDispatch* pDoc, _In_ const WordEvent& emWordComEvent,  _In_ const VARIANT_BOOL&  bVariantSaveAsUI );
	ProcessResult NLProcessComNotifyExcel( _In_ IDispatch* pDoc, _In_ const ExcelEvent& emExcelComEvent,  _In_ const VARIANT_BOOL&  bVariantSaveAsUI );
	ProcessResult NLProcessComNotifyPpt(   _In_ IDispatch* pDoc, _In_ const PPTEvent& emPptComEvent, _In_ const VARIANT_BOOL&  bVariantSaveAsUI );
	
	ProcessResult NLProcessHookEvent( _Inout_ HookEvent&	stuOfficeHookEvent );
	
private:
	ProcessResult NLProcessOfficeActionFace( _In_ IDispatch* pDoc, _In_ const OfficeAction&	emAction, _In_ const EventType& emEventTriggerPoint, _Inout_ STUNLOFFICE_EVENTSTATUS& stuEventStatus );

private: // process action
	ProcessResult OnOpen(    _In_ IDispatch* pDoc, _In_ const wstring& wstrFilePath );
	ProcessResult OnEdit(    _In_ IDispatch* pDoc, _In_ const wstring& wstrFilePath );
	ProcessResult OnCopy(    _In_ IDispatch* pDoc, _In_ const wstring& wstrFilePath, _In_ const OfficeFile& emSaveAsType );
	ProcessResult OnPaste(   _In_ IDispatch* pDoc, _In_ const wstring& wstrFilePath );
	ProcessResult OnPrint(   _In_ IDispatch* pDoc, _In_ const wstring& wstrFilePath );
	ProcessResult OnSend(    _In_ IDispatch* pDoc, _In_ const wstring& wstrFilePath );
	ProcessResult OnInsert(  _In_ const wstring& wstrFilePath, _In_ const InsertType& emInsertType );
	ProcessResult OnConvert( _In_ IDispatch* pDoc, _In_ const wstring& wstrFilePath );
	ProcessResult OnClose(   _In_ IDispatch* pDoc, _In_ const wstring& wstrFilePath );
	// this function only used in the active event.
	// we only can do some simple works in this function, if we do match the performance will be bad.
	void OnWndActive(_In_ IDispatch* pDoc);
	// passive action
	void NLProcessClassifyAction( _In_ const wstring& wstrFilePath );

// deny action
private:
	// used to start up the passive action. we can invoked this function after the file success opened. 
	void NLStartUpPassiveAction( _In_ const wstring& wstrFilePath );

	// this function only used after we deny the open action
	bool NLCloseDocInOpenAction( _In_ IDispatch* pDispDoc, _In_ const wstring& wstrFilePath );

	// we can use this function to close
	bool NLCloseDoc( _In_ IDispatch* pDispDoc, _In_ const wstring& wstrFilePath );

	// check if there is save requirement and return if the file need close
	bool NLCheckBeforeWordClose( _In_ IDispatch* pDoc, _In_ const wstring& wstrFilePath, _Out_ VARIANT_BOOL& bVariantCancel );

	// used to do word insert action
	ProcessResult OnWordInsert( _In_ const InsertType& emInsertType );
	
	// process excel insert action: insert object, text, picture and data from access, data from text
	ProcessResult OnExcelInsert( _In_ const wstring& wstrFilePath, _In_ const InsertType& emInsertType );
	
	// process PPT insert action: insert picture, text, object
	ProcessResult OnPPTInsert( _In_ const wstring& wstrFilePath, _In_ const InsertType& emInsertType );

	// used to do excel insert picture
	ProcessResult NLDoExcelInsertPic( );

	// used to do PPT insert picture
	ProcessResult NLDoPPTInsertPic( );

	// used for emWordNewDoc event, success closed return true
	bool NLDoCloseByCloseFlag();

	// used for emExcelHyperLink event
	void NLDoExcelHyperLink();
	
	/*
	*\ Brief: use this function we can change if we need to process this function. this function just used in NLProcessOfficeActionFace.
	*    Note that the no need handle flag will be revert in the function. 
	*\ Parameter:
	*		[in] pDoc: the document IDispatch interface, if this is NULL, it will get the current active document IDispatch. we use this pointer to get the file path.
	*		[in] emAction: the office action what we want to check.
	*\ Return value: 
	*		return true means don't need to process this action, otherwise return false. 
	*/	
	bool NLIsNoNeedHandle( _In_ IDispatch* pDoc, _In_ const OfficeAction& emAction );
	


private:	
	wstring NLUpdateCurActiveFile( _In_ IDispatch* pDoc );

	// When triggered Save routine, Process class must distinguish the right file-type user want to set
	inline void SetFileSaveAsType(OfficeFile emFileType)
	{
		InterlockedExchange(reinterpret_cast<long*>(&m_emFileSaveAsType), static_cast<long>(emFileType));
	}
	inline OfficeFile GetFileSaveAsType() { return m_emFileSaveAsType; }
	
	void NLSetPassiveFlag( _In_ const wstring& wstrFilePath, _In_ const bool& bPassiveFlag );
	bool NLGetPassiveFlag( _In_ const wstring& wstrFilePath );

	ProcessResult NLProcessProtectedViewAction(_In_ IDispatch* pDoc, _In_ const wstring& wstrPath, _In_ IDispatch* pDispProtectedView);
	
	// process protect view open
	void ProtectedViewWindowBeforeClose( _In_ IDispatch* pDisp, _In_ const wstring& wstrFilePath );
private:
	OfficeFile m_emFileSaveAsType;
	
	CRITICAL_SECTION m_csMapPassiveFlag;
	map<wstring,bool> m_mapPassiveFlag;

	std::wstring m_wstrPalisadePath;
};
