#ifndef __ODHD_INC_H
#define __ODHD_INC_H

#ifdef ODHD_EXPORTS
#define ODHD_API	__declspec(dllexport)
#else
#define ODHD_API	__declspec(dllimport)
#endif

#include <vector>
#include "basetype.h"

class HDRFile
{
public:
	HDRFile(std::wstring &strFileName, std::wstring &strFilePath, DWORD dwCatType,DWORD dwRemoveButton=0xFF) 
		: m_strFileName(strFileName), m_strFilePath(strFilePath)
	{
		m_dwNeedDetectCatType = dwCatType;
#ifdef WSO2K3
		DWORD dwHalfPart=dwRemoveButton&0x03;
		DWORD dwShiftHalfPart=dwRemoveButton&0xF8;
		m_dwRemoveButton=dwHalfPart|(dwShiftHalfPart>>1);
#else
		m_dwRemoveButton = dwRemoveButton;
#endif
		m_dwFoundCatType = 0;
		m_dwRemovedCatType = 0;
	}
	~HDRFile() {}

public:
	std::wstring m_strFileName;
	std::wstring m_strFilePath;
	DWORD m_dwNeedDetectCatType;
	DWORD m_dwRemoveButton;

	DWORD m_dwFoundCatType;
	DWORD m_dwRemovedCatType;
};

typedef std::vector<HDRFile *> HDRFileList;

extern "C" ODHD_API HRESULT RemoveHiddenData(HWND hParentWnd, HDRFileList &hdrFileList, std::wstring &strMsg, std::vector<std::wstring> &vecHelpUrls, LONG lBtnVisible, BOOL *pbCanceled);

#endif //__ODHD_INC_H