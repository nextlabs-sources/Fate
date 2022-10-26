#include "StdAfx.h"
#include "VLObligation.h"
#include "OfficeVisualLabeling.h"
#include "Utility.h"

using namespace NM_VLObligation;
 
extern INT InvokeViewVL(const wstring& strFilePath, const VisualLabelingInfo& theInfo,HWND hParent);

IDispatch* COfficeVL::GetWordObject(const wstring& strFilePath)
{
	IDispatch* pDoc = GetOfficeDocObject(strFilePath.c_str());
	if(pDoc == NULL)
	{
		// try to open the document and get the object
		CLSID clsid;
		HRESULT hr = CLSIDFromProgID(L"Word.Application", &clsid);
		IDispatch *pWordApp;
		hr = CoCreateInstance(clsid, NULL, CLSCTX_LOCAL_SERVER, IID_IDispatch, (void **)&pWordApp);

		CComQIPtr<Word::_wordApplication> ptrWord(pWordApp);
		ptrWord->put_Visible(VARIANT_FALSE);
		Word::DocumentsPtr pWordDocs=NULL;
		ptrWord->get_Documents(&pWordDocs);
		long Count;
		pWordDocs->get_Count(&Count);
		for (int i=0; i<Count; i++)
		{
			Word::_DocumentPtr pWordDoc;
			CComVariant theIndex(i+1);
			pWordDocs->Item(&theIndex, &pWordDoc);
			BSTR FullName;
			pWordDoc->get_FullName(&FullName);
			if (_wcsicmp(FullName,strFilePath.c_str()) == 0)
			{
				hr = pWordDoc->QueryInterface(IID_IDispatch,(void**)&pDoc);
				break;
			}
		}
	}
	return pDoc;
}
IDispatch* COfficeVL::GetExcelObject(const wstring& strFilePath)
{
	IDispatch* pWkBook = GetOfficeDocObject(strFilePath.c_str());
	if(pWkBook == NULL)
	{
		// try to open the workbook and get the object
		CLSID clsid;
		HRESULT hr = CLSIDFromProgID(L"Excel.Application", &clsid);
		IDispatch *pExcelApp;
		hr = CoCreateInstance(clsid, NULL, CLSCTX_LOCAL_SERVER, IID_IDispatch, (void **)&pExcelApp);
		CComQIPtr<Excel::_excelApplication> ptrExcel(pExcelApp);
		ptrExcel->put_Visible(1,VARIANT_FALSE);
		Excel::WorkbooksPtr pExcelWbooks=NULL;
		ptrExcel->get_Workbooks(&pExcelWbooks);
		long Count;
		pExcelWbooks->get_Count(&Count);
		for (int i=0; i<Count; i++)
		{
			Excel::_WorkbookPtr pExcelWbook;
			pExcelWbooks->get_Item(_variant_t(i+1), &pExcelWbook);
			BSTR FullName;
			pExcelWbook->get_FullName(1,&FullName);
			if (_wcsicmp(FullName,strFilePath.c_str()) == 0)
			{
				hr = pExcelWbook->QueryInterface(IID_IDispatch,(void**)&pWkBook);
				break;
			}
		}
	}
	return pWkBook;
}
IDispatch* COfficeVL::GetPPTObject(const wstring& strFilePath)
{
	IDispatch* pPresent = GetOfficeDocObject(strFilePath.c_str());
	if(pPresent == NULL)
	{
		// try to open the Presentation and get the object
		CLSID clsid;
		HRESULT hr = CLSIDFromProgID(L"PPT.Application", &clsid);
		IDispatch *pPPTApp;
		hr = CoCreateInstance(clsid, NULL, CLSCTX_LOCAL_SERVER, IID_IDispatch, (void **)&pPPTApp);
		CComQIPtr<PPT::_pptApplication> ptrPPT(pPPTApp);
		ptrPPT->put_Visible(msoFalse);
		PPT::PresentationsPtr pPPTPresents=NULL;
		ptrPPT->get_Presentations(&pPPTPresents);
		long Count;
		pPPTPresents->get_Count(&Count);
		for (int i=0; i<Count; i++)
		{
			PPT::_PresentationPtr pPPTPresent;
			CComVariant theIndex(i+1);
			pPPTPresents->Item(theIndex, &pPPTPresent);
			BSTR FullName;
			pPPTPresent->get_FullName(&FullName);
			if (_wcsicmp(FullName,strFilePath.c_str()) == 0)
			{
				hr = pPPTPresent->QueryInterface(IID_IDispatch,(void**)&pPresent);
				break;
			}
		}
	}
	return pPresent;
}
bool COfficeVL::DoVisualLabeling(const wstring& strFilePath, const PABase::ACTION& theAction,const VisualLabelingInfo& newInfo,HWND hWnd)
{
	switch(theAction)
	{
	case AT_READ:
		{
			// invoke floating window module 
			InvokeViewVL(strFilePath,newInfo,hWnd);
		}
		break;
	case AT_PERSISTED:
		{
#if 0
			if(IsWordType(strFilePath))
			{
				IDispatch* pDoc = GetWordObject(strFilePath);
				if(pDoc != NULL)
				{
					// apply the Labeling by this com object
					CWordLabeling WdLabeling;
					if (_wcsicmp(newInfo.strLabelingType.c_str(), Watermark) == 0)
					{
						WdLabeling.SetWatermarkText(pDoc, newInfo);
					}
					else if(_wcsicmp(newInfo.strLabelingType.c_str(), Header) == 0)
					{
						WdLabeling.SetHeaderText(pDoc, newInfo);
					}
					else if(_wcsicmp(newInfo.strLabelingType.c_str(), Footer) == 0)
					{
						WdLabeling.SetFooterText(pDoc, newInfo);
					}
					pDoc->Release();
				}
			}
			else if(IsExcelType(strFilePath))
			{
				IDispatch* pWkBook = GetExcelObject(strFilePath);
				if(pWkBook != NULL)	
				{
					// apply the Labeling by this com object
					CExcelLabeling ExLabeling;
					if (_wcsicmp(newInfo.strLabelingType.c_str(), Watermark) == 0)
					{
						ExLabeling.SetWatermarkText(pWkBook, newInfo);
					}
					else if(_wcsicmp(newInfo.strLabelingType.c_str(), Header) == 0)
					{
						ExLabeling.SetHeaderText(pWkBook, newInfo);
					}
					else if(_wcsicmp(newInfo.strLabelingType.c_str(), Footer) == 0)
					{
						ExLabeling.SetFooterText(pWkBook, newInfo);
					}
					pWkBook->Release();
				}
			}
			else if(IsPPTType(strFilePath))
			{
				IDispatch* pPresent = GetPPTObject(strFilePath);
				// PPT type
				if(pPresent != NULL)
				{
					// apply the Labeling by this com object
					CPPTLabeling PptLabeling;
					if (_wcsicmp(newInfo.strLabelingType.c_str(),Watermark) == 0)
					{
						PptLabeling.SetWatermarkText(pPresent, newInfo);
					}
					else if(_wcsicmp(newInfo.strLabelingType.c_str(), Header) == 0)
					{
						PptLabeling.SetHeaderText(pPresent, newInfo);
					}
					else if(_wcsicmp(newInfo.strLabelingType.c_str(), Footer) == 0)
					{
						PptLabeling.SetFooterText(pPresent, newInfo);
					}
					pPresent->Release();
				}
			}
#endif
		}
		break;
	default:
		break;
	}
	return true;
}