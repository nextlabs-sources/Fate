#include "stdafx.h"
#include "NLVisualLabelingPA.h"
#include "PPTDocLabeling.h"

//Set PPT watermark
bool CPPTLabeling::SetWatermarkText(IDispatch* pDoc,const MARKUP& theMarkupInfo)
{
	CComQIPtr<_Presentation> ptrPresentation(pDoc);
	_MasterPtr ptrMaster;
	if(FAILED(ptrPresentation->get_SlideMaster(&ptrMaster)))
		return false;

	pptShapesPtr ptrShapes;
	if(FAILED(ptrMaster->get_pptShapes(&ptrShapes)))
		return false;

	pptShapePtr ptrShape;
	_bstr_t fontName (theMarkupInfo.strFont.c_str()) ;
	if(FAILED(ptrShapes->AddTextEffect( Office::msoTextEffect1,_bstr_t(theMarkupInfo.strInputText.c_str()), fontName,
		(float)theMarkupInfo.fSize,Office::msoFalse,Office::msoFalse,243.75, 223.5,&ptrShape) ) )
	{
		return false;
	}
	if(ptrShape != NULL)
	{
		ptrShape->put_Name(_bstr_t(theMarkupInfo.strTagName.c_str()));
		ptrShape->put_Rotation(theMarkupInfo.fLayout);
	}

	PPT::pptFillFormatPtr pFill;
	ptrShape->get_Fill(&pFill);
	PPT::pptColorFormatPtr foreColor ;
	pFill->get_ForeColor(&foreColor);
	foreColor->put_pptRGB(theMarkupInfo.lColor);
	pFill->put_Transparency(theMarkupInfo.fSemi);

	PPT::_pptApplicationPtr ptrPPTApp=NULL;
	ptrPresentation->get_pptApplication(&ptrPPTApp);

	if(ptrPPTApp)
	{
		PPT::DocumentWindowPtr pDocWindow=NULL;
		HRESULT hr = ptrPPTApp->get_ActiveWindow(&pDocWindow);
		if(SUCCEEDED(hr))
		{
			hr = pDocWindow->put_ViewType(ppViewNormal);
		}
	}
	return true;
}

bool CPPTLabeling::SetHeaderText(IDispatch* pDoc,const MARKUP& theInfo)
{
	bool bRet = false;
	CComQIPtr<_Presentation> ptrDoc(pDoc);
	CComPtr<PPT::Slides> pSlides = NULL;
	HRESULT hr = ptrDoc->get_Slides(&pSlides);
	if(FAILED(hr) || pSlides == NULL)	return false;

	long icount = 0;
	hr = pSlides->get_Count(&icount);
	if(FAILED(hr) || icount < 1)	return false;

	for(int i=0;i<icount;i++)
	{
		CComPtr<PPT::_Slide> pSlide=NULL;
		hr = pSlides->Item(CComVariant(i+1),&pSlide);
		if(FAILED(hr) && pSlide == NULL)	continue;

		CComPtr<PPT::pptHeadersFooters> pHFers=NULL;
		hr = pSlide->get_pptHeadersFooters(&pHFers);
		if(FAILED(hr) && pHFers == NULL)	continue;

		PPT::pptHeaderFooterPtr pHFooter = NULL;
		hr = pHFers->get_Header(&pHFooter);
		if(SUCCEEDED(hr) && pHFooter != NULL)
		{
			AddHeaderFooter(TRUE, pHFooter,theInfo);
		}
	}
	return bRet;
}
bool CPPTLabeling::SetFooterText(IDispatch* pDoc,const MARKUP& theInfo)
{
	/*CComQIPtr<_Presentation> ptrPresentation(pDoc);
	_MasterPtr ptrMaster;
	if(FAILED(ptrPresentation->get_SlideMaster(&ptrMaster)))
		return false;

	PPT::pptHeadersFootersPtr ptrHeadersFooters;
	if(FAILED(ptrMaster->get_pptHeadersFooters(&ptrHeadersFooters)))
		return false;
	PPT::pptHeaderFooter ptrFooter;
	ptrHeadersFooters->get_Footer(&ptrFooter);
	ptrFooter->put_Visible(msoTrue);*/

	bool bRet = false;
	CComQIPtr<_Presentation> ptrDoc(pDoc);
	CComPtr<PPT::Slides> pSlides = NULL;
	HRESULT hr = ptrDoc->get_Slides(&pSlides);
	if(FAILED(hr) || pSlides == NULL)
		return false;

	long icount = 0;
	hr = pSlides->get_Count(&icount);
	if(FAILED(hr) || icount < 1)
		return false;

	for(int i=0;i<icount;i++)
	{
		CComPtr<PPT::_Slide> pSlide=NULL;
		hr = pSlides->Item(CComVariant(i+1),&pSlide);
		if(FAILED(hr) && pSlide == NULL)
			continue;

		CComPtr<PPT::pptHeadersFooters> pHFers=NULL;
		hr = pSlide->get_pptHeadersFooters(&pHFers);
		if(FAILED(hr) && pHFers == NULL)
			continue;

		PPT::pptHeaderFooterPtr pHFooter = NULL;
		hr = pHFers->get_Footer(&pHFooter);
		if(SUCCEEDED(hr) && pHFooter != NULL)
			AddHeaderFooter(FALSE, pHFooter, theInfo);
	}
	return bRet;
}

//Remove PPT watermark
bool CPPTLabeling::RemoveWatermarkText(IDispatch* pDoc,const wchar_t* strTagName)
{
	CComQIPtr<_Presentation> ptrPresentation(pDoc);
	_MasterPtr ptrMaster;
	if(FAILED(ptrPresentation->get_SlideMaster(&ptrMaster)))
		return false;

	pptShapesPtr ptrShapes;
	if(FAILED(ptrMaster->get_pptShapes(&ptrShapes)))
		return false;

	int lItemCount = 0;
	HRESULT hr = ptrShapes->get_Count(&lItemCount);
	if(FAILED(hr))	return false;

	for(long j=0;j<lItemCount;j++)
	{
		pptShapePtr ptrShape;
		CComVariant theIndex(j+1);
		hr = ptrShapes->Item(theIndex,&ptrShape);
		if(SUCCEEDED(hr) && ptrShape != NULL)
		{
			BSTR theName = NULL;
			ptrShape->get_Name(&theName);
			std::wstring strName(theName);
			::SysFreeString(theName);
			if(strName.compare(strTagName) == 0)
			{
				ptrShape->Select(Office::msoTrue);
				ptrShape->Delete();
			}
		}
	}
	return true;
}

bool CPPTLabeling::AddHeaderFooter(BOOL IsHeader, PPT::pptHeaderFooterPtr ptrHFer,const MARKUP& theInfo)
{
	bool			bRet=true;
	HRESULT			hr   = S_OK;
	std::wstring	wstrNewText(theInfo.strInputText);

	hr = ptrHFer->put_Visible(msoTrue);

	wstrNewText.insert(0, L"[");
	wstrNewText.insert(wstrNewText.size(), L"]");

	BSTR bstrText;
	hr = ptrHFer->get_Text(&bstrText);
	if(SUCCEEDED(hr) && bstrText != NULL)
	{
		std::wstring strText(bstrText);
		::SysFreeString(bstrText);
		
		size_t nPos1 = strText.find(L"[");
		size_t nPos2 = strText.rfind(L"]");
		if( (nPos1 != wstring::npos) && (nPos2 != wstring::npos) && (nPos1 < nPos2) )
			strText.replace(nPos1, nPos2-nPos1+1, wstrNewText);
		else
			strText = wstrNewText;

		hr = ptrHFer->put_Text(_bstr_t(strText.c_str()));
		if(FAILED(hr))
			bRet = false;
	}
	else
	{
		hr = ptrHFer->put_Text(_bstr_t(wstrNewText.c_str()));
		if(FAILED(hr))
			bRet = false;
	}

	return bRet;
}