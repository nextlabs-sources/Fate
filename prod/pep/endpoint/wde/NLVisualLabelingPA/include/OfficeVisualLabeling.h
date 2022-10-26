#pragma once
// Comply with DoxyGen Format

class COfficeVL
{
public:
	static bool DoVisualLabeling(const wstring& strFilePath,const PABase::ACTION& theAction,const NM_VLObligation::VisualLabelingInfo& theInfo,HWND hWnd = NULL);
private:
	/*
	 * brief:\get word docuemnt's object
	 *  step1: try to get object by check activeobject
	 *	step2: try to open and get the document's object
	 * note: the caller should responsible invoke release after use the Dispatch object
	 */
	static IDispatch* GetWordObject(const wstring& strFilePath);
	static IDispatch* GetExcelObject(const wstring& strFilePath);
	static IDispatch* GetPPTObject(const wstring& strFilePath);
};
