#ifndef _NLVLVIEWPRINT_H_
#define _NLVLVIEWPRINT_H_

#ifdef NLVLVIEWPRINT_DLL
#define NLVLVIEWPRINT_DLL extern "C" _declspec(dllimport)
#else
#define NLVLVIEWPRINT_DLL extern "C" _declspec(dllexport)
#endif

#include "VLObligation.h"
#include <windows.h>
#include <string>
using namespace std;

enum OverlayError
{
	NLOverlaySuccess,  
	NLOverlayGDIFail,
	NLOverlayCreatewindowFail,
	NLOverlayParametersError,
	NLNoViewHandle,
	NLOverlayUnknowError
};
NLVLVIEWPRINT_DLL int ViewVisualLabeling(const wstring& strFilePath, const NM_VLObligation::VisualLabelingInfo& theInfo,HWND hParent);
NLVLVIEWPRINT_DLL bool PrintVL(const HDC& hDC,const NM_VLObligation::IVisualLabelingInfo& theInfo);

#endif