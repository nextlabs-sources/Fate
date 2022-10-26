// PA_Labeling.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

#include "VLObligation.h"
#include "Utility.h"
#include "OfficeVisualLabeling.h"
#include "PDFVisualLabeling.h"
#include "celog.h"
#include "COverlayTool.h"
#include "strsafe.h"
using namespace NM_VLObligation;

extern CELog NLVisualLabelingPALog;

typedef struct _LabelingItem
{
	wstring strFilePath;								// the labeling document
	IDispatch* pDoc;									// the document's com object
	vector<VisualLabelingInfo> vecInfo;				// this document's markup information
	HWND hParentWnd;									// parent window.
	ACTION	theAction;								// here ,we focus on copy/print/open
	_LabelingItem():pDoc(NULL),hParentWnd(NULL),theAction(AT_DEFAULT)
	{
	};
}OB_LABELINGITEM;
typedef vector<OB_LABELINGITEM> OB_LABELINGITEMS;
//////////////////////////////////////////////////////////////////////////
// Interface for PAF
static PA_STATUS PA_DoVisualLabeling(const OB_LABELINGITEMS& theItems)
{
	PA_STATUS theRet = PA_SUCCESS;
	CoInitialize(NULL);
	OB_LABELINGITEMS::const_iterator iter = theItems.begin();
	for(;iter != theItems.end();++iter)	// most of the time ,only one item for one obligation
	{
		const OB_LABELINGITEM& theItem = (*iter);
		bool bPDF = false;
		if(boost::algorithm::iends_with(theItem.strFilePath,L".PDF"))	bPDF = true;
		for(size_t j=0; j<theItem.vecInfo.size(); j++)
		{
			// change the Markupinfo structure at here.
			const VisualLabelingInfo& theMarkUpInfo = theItem.vecInfo[j];
			if(!bPDF)			COfficeVL::DoVisualLabeling(theItem.strFilePath,theItem.theAction,theMarkUpInfo,theItem.hParentWnd);
			else 		CPDFVL::DoVisualLabeling(theItem.strFilePath,theItem.theAction,theMarkUpInfo,theItem.hParentWnd);
		}
	}
	CoUninitialize();
	return theRet;
}



PA_STATUS WINAPI DoPolicyAssistant( PA_PARAM &_iParam, const HWND _hParentWnd , bool autoWrapforSe)
{
	NLVisualLabelingPALog.Log(CELOG_DEBUG,L"Enter to NLVisualLabelingPA module...................\n");
	//OutputDebugStringW(L"Enter to NLVisualLabelingPA module...................\n");
	OB_LABELINGITEMS theItems;
	// parse obligation 
	OBJECTINFOLIST::iterator objInfoiter = _iParam.objList.begin();
	for (;objInfoiter != _iParam.objList.end();++objInfoiter)
	{
		OB_LABELINGITEM theItem;
		OBJECTINFO& theObInfo = (*objInfoiter);
		theItem.hParentWnd = _hParentWnd;
		theItem.strFilePath = theObInfo.strSrcName;
		theItem.theAction = _iParam._action;
		OBLIGATIONLIST::iterator obIter = theObInfo.obList.begin();
		for(;obIter != theObInfo.obList.end();++obIter)
		{
			OBLIGATION& theOb = (*obIter);
			if(0 == _wcsicmp(OBLIGATION_VIEW_OVERLAY, theOb.strOBName.c_str()))
			{
				NLVisualLabelingPALog.Log(CELOG_DEBUG,L"Get Obligation_View_Overlay................\n");
				//OutputDebugStringW(L"Get Obligation_View_Overlay................\n");
				VisualLabelingInfo theInfo;
				if(theObInfo.strSrcName.length() < 2048)
				{
					wcsncpy_s(theInfo.strFilePath,2048,theObInfo.strSrcName.c_str(), _TRUNCATE);
				}
				else
					NLVisualLabelingPALog.Log(CELOG_DEBUG, L"The file path is longer than 2048......emgerency issue.\n");
				//OutputDebugStringW(L"The file path is longer than 2048......emgerency issue.\n");
				ATTRIBUTELIST::iterator iter = theOb.attrList.begin();
				for(;iter != theOb.attrList.end();++iter)
				{
					if(_wcsicmp((*iter).strValue.c_str(),str_text_name) == 0)
					{
						iter++;
						if((*iter).strValue.length() < 512)
						{
							wcsncpy_s(theInfo.strText,512,(*iter).strValue.c_str(), _TRUNCATE);
						}
						else
							NLVisualLabelingPALog.Log(CELOG_DEBUG, L"The strText is longer than 512......emgerency issue.\n");
						//OutputDebugStringW(L"The strText is longer than 512......emgerency issue.\n");
						continue;
					}
					else if(_wcsicmp((*iter).strValue.c_str(),str_classification_map_name) == 0)
					{
						iter++;
						if((*iter).strValue.length() < 512)
						{
							wcsncpy_s(theInfo.strClassificationMap,512,(*iter).strValue.c_str(), _TRUNCATE);
						}
						else
							NLVisualLabelingPALog.Log(CELOG_DEBUG, L"The strClassificationMap is longer than 512......emgerency issue.\n");
						//OutputDebugStringW(L"The strClassificationMap is longer than 512......emgerency issue.\n");
						continue;
					}
					else if(_wcsicmp((*iter).strValue.c_str(),str_transparency_name) == 0)
					{
						iter++;
						try
						{
							theInfo.dwTransparency = boost::lexical_cast<unsigned long>((*iter).strValue.c_str());
						}
						catch (...)
						{	
							theInfo.dwTransparency = 35;
						}
						continue;
					}
					else if(_wcsicmp((*iter).strValue.c_str(),str_font_name) == 0)
					{
						iter++;
						if((*iter).strValue.length() < 64)
						{
							wcsncpy_s(theInfo.strFontName,64,(*iter).strValue.c_str(), _TRUNCATE);
						}
						else
							NLVisualLabelingPALog.Log(CELOG_DEBUG, L"The strFontName is longer than 64......emgerency issue.\n");
						//OutputDebugStringW(L"The strFontName is longer than 64......emgerency issue.\n");
						continue;
					}
					else if(_wcsicmp((*iter).strValue.c_str(),str_font_size1_name) == 0)
					{
						iter++;
						try
						{
							theInfo.dwFontSize1 = boost::lexical_cast<unsigned long>((*iter).strValue.c_str());
						}
						catch (...)
						{	
							theInfo.dwFontSize1 = 36;
						}
						continue;
					}
					else if(_wcsicmp((*iter).strValue.c_str(),str_font_size2_name) == 0)
					{
						iter++;
						try
						{
							theInfo.dwFontSize2 = boost::lexical_cast<unsigned long>((*iter).strValue.c_str());
						}
						catch (...)
						{	
							theInfo.dwFontSize2 = 28;
						}
						continue;
					}
					else if(_wcsicmp((*iter).strValue.c_str(),str_font_bold_name) == 0)
					{
						iter++;
						if(_wcsicmp((*iter).strValue.c_str(),L"false") == 0)		theInfo.bFontBold = false;
						else theInfo.bFontBold = true;
						continue;
					}
					else if(_wcsicmp((*iter).strValue.c_str(),str_font_color_name) == 0)
					{
						iter++;
						wstring strColor = (*iter).strValue;
						boost::algorithm::replace_all(strColor,L"#",L"");
				
						try
						{
							theInfo.dwFontColor = COverlayTool::GetFontColor(strColor);
						}
						catch (...)
						{	
							theInfo.dwFontColor = 888888;
						}
						continue;
					}
					else if(_wcsicmp((*iter).strValue.c_str(),str_placement_view_print_name) == 0)
					{
						iter++;
						if((*iter).strValue.length() < 64)
						{
							wcsncpy_s(theInfo.strPlacement,64,(*iter).strValue.c_str(), _TRUNCATE);
						}
						else
							NLVisualLabelingPALog.Log(CELOG_DEBUG, L"The strPlacement is longer than 64......emgerency issue.\n");
						//OutputDebugStringW(L"The strPlacement is longer than 64......emgerency issue.\n");
						continue;
					}
					else if(_wcsicmp((*iter).strValue.c_str(),str_left_margin_view_name) == 0)
					{
						iter++;
						try
						{
							theInfo.dwLeftMargin = boost::lexical_cast<DWORD>((*iter).strValue.c_str());
						}
						catch (...)
						{
							theInfo.dwLeftMargin = 100;
						}						
						continue;
					}
					else if(_wcsicmp((*iter).strValue.c_str(),str_top_margin_view_name) == 0)
					{
						iter++;
						try
						{
							theInfo.dwTopMargin = boost::lexical_cast<DWORD>((*iter).strValue.c_str());
						}
						catch (...)
						{
							theInfo.dwTopMargin = 100;
						}
						continue;
					}
					else if(_wcsicmp((*iter).strValue.c_str(),str_hor_space_view_name) == 0)
					{
						iter++;
						try
						{
							theInfo.dwHorSpace = boost::lexical_cast<DWORD>((*iter).strValue.c_str());
						}
						catch (...)
						{
							theInfo.dwHorSpace = 200;
						}
						continue;
					}
					else if(_wcsicmp((*iter).strValue.c_str(),str_ver_space_view_name) == 0)
					{
						iter++;
						try
						{
							theInfo.dwVerSpace = boost::lexical_cast<DWORD>((*iter).strValue.c_str());
						}
						catch (...)
						{
							theInfo.dwVerSpace = 300;
						}
						continue;
					}
					else if(_wcsicmp((*iter).strValue.c_str(),str_policy_name) == 0)
					{
						iter++;
						if((*iter).strValue.length() < 512)
						{
							wcsncpy_s(theInfo.strPolicyName,512,(*iter).strValue.c_str(), _TRUNCATE);
						}
						else
							NLVisualLabelingPALog.Log(CELOG_DEBUG, L"The strPolicyName is longer than 512......emgerency issue.\n");
						//OutputDebugStringW(L"The strPolicyName is longer than 512......emgerency issue.\n");
						continue;
					}
					else if(_wcsicmp((*iter).strValue.c_str(),str_date_format_name) == 0)
					{
						iter++;
						if((*iter).strValue.length() < 128)
						{
							wcsncpy_s(theInfo.strDateFormat,128,(*iter).strValue.c_str(), _TRUNCATE);
						}
						else
							NLVisualLabelingPALog.Log(CELOG_DEBUG, L"The strDateFormat is longer than 128......emgerency issue.\n");
						//OutputDebugStringW(L"The strDateFormat is longer than 128......emgerency issue.\n");
						continue;
					}
					else if(_wcsicmp((*iter).strValue.c_str(),str_time_format_name) == 0)
					{
						iter++;
						if((*iter).strValue.length() < 128)
						{
							wcsncpy_s(theInfo.strTimeFormat,128,(*iter).strValue.c_str(), _TRUNCATE);
						}
						else
							NLVisualLabelingPALog.Log(CELOG_DEBUG, L"The strTimeFormat is longer than 128......emgerency issue.\n");
						//OutputDebugStringW(L"The strTimeFormat is longer than 128......emgerency issue.\n");
						continue;
					}
					//... keep going until you get all what you need
				}
				wstring strTempTxTValue = L"";
				COverlayTool::ConvertTxT(theInfo.strText,theInfo.strFilePath,theInfo.strPolicyName,theInfo.strDateFormat,theInfo.strTimeFormat,strTempTxTValue);
				StringCchPrintf(theInfo.strTextValue,1024,L"%s",strTempTxTValue.c_str());
				theItem.vecInfo.push_back(theInfo);
			}
		}
		theItems.push_back(theItem);
	}
	PA_DoVisualLabeling(theItems);
	return 0;
}

//////////////////////////////////////////////////////////////////////////
// Interface for common invoking
VL_PERSISTED_RESULT AddVisualLabeling(const wstring& strFile,const ACTION& theAction,const VisualLabelingInfo& theInfo,IDispatch* pDoc=NULL)
{
	VL_PERSISTED_RESULT theRet = emVL_NOERROR;
	bool bPDF = false;
	bool bOffice = IsWordType(strFile) || IsExcelType(strFile) || IsPPTType(strFile);
	if(!bOffice)
	{
		if(boost::algorithm::iends_with(strFile,L".PDF"))	bPDF = true;
	}

	if(bOffice)			COfficeVL::DoVisualLabeling(strFile,theAction,theInfo);
	else if(bPDF)		CPDFVL::DoVisualLabeling(strFile,theAction,theInfo);

	return theRet;
}

VL_PERSISTED_RESULT MoveVisualLabeling(const wstring& strFile,const wstring& UID,IDispatch* pDoc=NULL)
{
	VL_PERSISTED_RESULT theRet = emVL_NOERROR;
	return theRet;
}
