#include "stdafx.h"
#include "NLVisualLabelingPA.h"
#include "WordDocLabeling.h"

//Set Word watermark
bool CWordLabeling::SetWatermarkText(IDispatch* pDoc,const MARKUP& theMarkupInfo)
{
	CComQIPtr<Word::_Document> ptrDoc(pDoc);
	Word::SectionsPtr ptrSections ;
	HRESULT hr = ptrDoc->get_Sections( &ptrSections ) ;
	if( FAILED( hr ) ||( ptrSections==NULL ) )
	{
		return false;
	}
	LONG iCount ;
	hr = ptrSections->get_Count( &iCount) ;
	if( FAILED( hr ))
	{
		return false;
	}
	for( INT i=0 ; i < iCount ; i++ )
	{
		Word::SectionPtr  ptrSection ;
		hr = ptrSections->Item( (LONG)(i+1), &ptrSection) ;
		if( SUCCEEDED( hr ) && ptrSection )
		{
			Word::wordHeadersFootersPtr ptrFooters ;
			hr = ptrSection->get_Footers(	&ptrFooters) ;
			if( SUCCEEDED( hr ) )
			{

				Word::wordHeaderFooterPtr ptrFooter ;
				hr = ptrFooters->Item( Word::wdHeaderFooterFirstPage, &ptrFooter ) ;
				if( SUCCEEDED( hr ) )
				{	
					Word::wordShapesPtr shapes ;
					hr = ptrFooter->get_wordShapes( &shapes ) ;
					if( SUCCEEDED( hr ) )
					{
						Word::wordShapePtr ptrWordArt ;
						BSTR Text = ::SysAllocString(theMarkupInfo.strInputText.c_str()) ;
						BSTR format = ::SysAllocString(	theMarkupInfo.strFont.c_str()) ;
						hr = shapes->AddTextEffect(Office::msoTextEffect2, Text, format, (float)theMarkupInfo.fSize,
							Office::msoFalse,Office::msoFalse,243.75, 223.5 ,&vtMissing, &ptrWordArt) ;
						::SysFreeString(Text ) ;
						::SysFreeString(format ) ;
						if(ptrWordArt != NULL)
						{
							ptrWordArt->Select();
							ptrWordArt->put_Name(_bstr_t(theMarkupInfo.strTagName.c_str()));

							Word::wordTextEffectFormatPtr txtEffect ;
							ptrWordArt->get_TextEffect( &txtEffect) ;
							txtEffect->put_NormalizedHeight(Office::msoFalse);

							Word::wordLineFormatPtr pLine ;
							hr = ptrWordArt->get_Line( &pLine ) ;
							pLine->put_Visible(Office::msoFalse);
							Word::wordFillFormatPtr pFill ;
							ptrWordArt->get_Fill( &pFill) ;
							pFill->put_Visible(Office::msoTrue);
							pFill->Solid();
							Word::wordColorFormatPtr foreColor ;
							pFill->get_ForeColor(&foreColor) ;
							foreColor->put_wordRGB( theMarkupInfo.lColor );

							pFill->put_Transparency(theMarkupInfo.fSemi);
							ptrWordArt->put_Rotation(theMarkupInfo.fLayout);
							ptrWordArt->put_LockAspectRatio(Office::msoTrue);
							Word::WrapFormatPtr pWrap;
							ptrWordArt->get_WrapFormat( &pWrap );
							pWrap->put_AllowOverlap( Office::msoTrue );
							pWrap->put_Type( Word::wdWrapNone );
							ptrWordArt->put_RelativeHorizontalPosition( Word::wdRelativeHorizontalPositionMargin );
							ptrWordArt->put_RelativeVerticalPosition( Word::wdRelativeVerticalPositionMargin );
							ptrWordArt->put_Left(Word::wdShapeCenter);
							ptrWordArt->put_Top(Word::wdShapeCenter );
						}
					}
				}
			}
		}
	}
	return true;
}

 bool CWordLabeling::SetHeaderText(IDispatch* ptrDoc,const MARKUP& theInfo)
 {
	return MarkupHeaderFooter(ptrDoc, theInfo.strInputText, theInfo.strJustify, theInfo.strFont, theInfo.fSize, theInfo.lColor, TRUE)?true:false;
 }

 bool CWordLabeling::SetFooterText(IDispatch* ptrDoc,const MARKUP& theInfo)
 {
	 return MarkupHeaderFooter(ptrDoc, theInfo.strInputText, theInfo.strJustify, theInfo.strFont, theInfo.fSize, theInfo.lColor, FALSE)?true:false;
 }

//Remove Word watermark
bool CWordLabeling::RemoveWatermarkText(IDispatch* pDoc,const wchar_t* strTagName)
{
	CComQIPtr<Word::_Document> ptrDoc(pDoc);
	Word::SectionsPtr ptrSections ;
	HRESULT hr = ptrDoc->get_Sections( &ptrSections ) ;
	if( FAILED( hr ) ||( ptrSections==NULL ) )
	{
		ptrDoc->Close();
	}
	LONG iCount ;
	hr = ptrSections->get_Count( &iCount) ;
	if( FAILED( hr ))
	{
		ptrDoc->Close();
	}
	for( INT x=0 ; x < iCount ; x++ )
	{
		Word::SectionPtr  ptrSection ;
		hr = ptrSections->Item( (LONG)(x+1), &ptrSection) ;
		if( SUCCEEDED( hr ) && ptrSection )
		{
			Word::wordHeadersFootersPtr ptrFooters ;
			hr = ptrSection->get_Footers(	&ptrFooters) ;
			if( SUCCEEDED( hr ) )
			{
				Word::wordHeaderFooterPtr ptrFooter ;
				hr = ptrFooters->Item( Word::wdHeaderFooterFirstPage, &ptrFooter ) ;
				if( SUCCEEDED( hr ) )
				{	
					Word::wordShapesPtr shapes ;
					hr = ptrFooter->get_wordShapes( &shapes ) ;
					if( SUCCEEDED( hr ) )
					{
						long iShapeCount = 0;
						shapes->get_Count(&iShapeCount);
						for(int n=0;n<iShapeCount;n++)
						{
							Word::wordShapePtr ptrWordShape;
							CComVariant theIndex(n+1);
							hr = shapes->Item(&theIndex,&ptrWordShape);
							if(SUCCEEDED(hr))
							{
								BSTR bstrName;
								ptrWordShape->get_Name(&bstrName);
								std::wstring strName(bstrName);
								::SysFreeString(bstrName);

								if(strName.compare(strTagName) == 0)
								{
									hr = ptrWordShape->Select();
									hr = ptrWordShape->Delete();
								}
							}
						}
					}
				}
			}
		}
	}
	if(FAILED(hr))	return false;
	ptrDoc->Save();
	return true;
}

std::wstring CWordLabeling::GetIndent(const std::wstring& Justification)
{
	if(Justification == L"Right")
		return L"\t\t";
	else if(Justification == L"Center")
		return L"\t";
	else
		return L""; // For Left and Unknown
}

int CWordLabeling::GetLines(const std::wstring& wstrText, std::vector<std::wstring>& vecLines)
{
	std::wstring::size_type start = 0;
	std::wstring::size_type end = 0;

	vecLines.clear();
	do 
	{
		end = wstrText.find(L'\n', start);
		vecLines.push_back( wstrText.substr( start, (std::wstring::npos == end)?(std::wstring::npos):(end-start) ) );
		start = end+1;
	} while (std::wstring::npos != end);

	return (int)(vecLines.size());
}

std::wstring CWordLabeling::ResetIndent(const std::wstring& text, const std::wstring& markuptext, const std::wstring& indent)
{
	int nLines = 0;
	std::vector<std::wstring> vecLines;
	std::wstring newtext;

	nLines = GetLines(text, vecLines);
	for (int i=0; i<nLines; i++)
	{
		if(boost::algorithm::iends_with(vecLines[i], markuptext))
			vecLines[i] = indent + markuptext;
		newtext += vecLines[i];
		newtext += (i!=(nLines-1))?L"\n":L"";
	}

	return newtext;
}

void CWordLabeling::GetHeaderText(Word::wordHeaderFooter* pHeader, std::wstring& wstrHeaderText)
{
	HRESULT         hr = S_OK;
	VARIANT_BOOL    bHeaderExist = VARIANT_FALSE;

	wstrHeaderText = L"";
	hr = pHeader->get_Exists(&bHeaderExist);
	if(SUCCEEDED(hr) && bHeaderExist==VARIANT_TRUE)
	{
		CComPtr<Word::Range> spRange;
		hr = pHeader->get_Range(&spRange);
		if(SUCCEEDED(hr) && NULL!=spRange.p)
		{
			CComBSTR bstrHeaderText;
			hr = spRange->get_Text(&bstrHeaderText);
			if(SUCCEEDED(hr) && NULL!=bstrHeaderText.m_str)
				wstrHeaderText = bstrHeaderText.m_str;
		}
	}
}

void CWordLabeling::SetText(Word::wordSelection* pSelection,
						  const std::wstring& wstrText,
						  int offset,
						  int overwrite,
						  const std::wstring& wstrFont,
						  float fSize,
						  long lColor
						  )
{
	HRESULT hr      = S_OK;
	CComVariant vChar(wdCharacter);
	CComVariant vLen(offset);
	CComVariant vMove(wdMove);
	CComVariant vExtend(wdExtend);
	long        lProp = 0;

	vLen.lVal = offset;
	hr = pSelection->MoveRight(&vChar, &vLen, &vMove, &lProp);
	if(SUCCEEDED(hr))
	{
		vLen.lVal = overwrite;
		hr = overwrite?pSelection->MoveRight(&vChar,&vLen,&vExtend,&lProp):S_OK;
		if(SUCCEEDED(hr))
		{
			// Put font
			CComPtr<Word::_Font> spFont;
			hr = pSelection->get_wordFont(&spFont);
			if(SUCCEEDED(hr) && NULL!=spFont.p)
			{
				spFont->put_Size(float(fSize));
				spFont->put_Color((Word::WdColor)lColor);
				spFont->put_Name(_bstr_t(wstrFont.c_str()));
				spFont->put_Bold(FALSE);
				spFont->put_Italic(FALSE);
				pSelection->put_wordFont(spFont);
				pSelection->TypeText(_bstr_t(wstrText.c_str()));
			}
		}
	}
}

BOOL CWordLabeling::MarkupHeaderFooter(const LPDISPATCH pDoc,
									 const std::wstring& strMarkupText,
									 const std::wstring& strJustify,
									 const std::wstring& strFont,
									 float fSize,
									 long lColor,
									 BOOL bHeader)
{
	BOOL    bRet    = FALSE;
	HRESULT hr      = S_OK;
	CComQIPtr<Word::_Document>      spDoc(pDoc);
	CComPtr<Word::_wordApplication> spWordApp;
	CComPtr<Word::Window>           spAvtiveWindow;
	CComPtr<Word::wordPane>         spAvtivePane;
	CComPtr<Word::wordView>         spView;
	CComPtr<Word::wordSelection>    spSelection;
	CComPtr<Word::wordHeaderFooter> spHeader;
	std::wstring                    wstrOrigHeaderText;
	std::wstring                    wstrNewHeaderText;
	std::wstring					wstrNewText;
	std::wstring                    wstrIndent;
	BOOL                            bInHeaderFooterView = FALSE;

	static WCHAR wzEmptyFooter[2] = {0x0d, 0x0};

	CComVariant vChar(wdCharacter);
	CComVariant vLen(0);
	CComVariant vMove(wdMove);
	CComVariant vExtend(wdExtend);

	if(spDoc==NULL)
		goto _exit;

	// Get all valid objects
	hr = spDoc->get_ActiveWindow(&spAvtiveWindow);
	if(FAILED(hr) || NULL==spAvtiveWindow.p)
		goto _exit;
	hr = spAvtiveWindow->get_ActivePane(&spAvtivePane);
	if(FAILED(hr) || NULL==spAvtivePane.p)
		goto _exit;
	hr = spAvtivePane->get_wordView(&spView);
	if(FAILED(hr) || NULL==spView.p)
		goto _exit;
	hr = spView->put_SeekView(bHeader?wdSeekCurrentPageHeader:wdSeekCurrentPageFooter);
	if(FAILED(hr))
		goto _exit;
	bInHeaderFooterView = TRUE;
	hr = spAvtivePane->get_wordSelection(&spSelection);
	if(FAILED(hr) || NULL==spSelection.p)
		goto _exit;
	hr = spSelection->get_wordHeaderFooter(&spHeader);
	if(FAILED(hr) || NULL==spHeader.p)
		goto _exit;

	// Get indent
	wstrIndent = GetIndent(strJustify);
	wstrNewText = strMarkupText;
	wstrNewText.insert(0, L"[");
	wstrNewText.insert(wstrNewText.size(), L"]");

	// Get original header text
	GetHeaderText(spHeader, wstrOrigHeaderText);

	size_t nPos1 = wstrOrigHeaderText.find(L"[");
	size_t nPos2 = wstrOrigHeaderText.rfind(L"]");
	if( (nPos1 != wstring::npos) && (nPos2 != wstring::npos) && (nPos1 < nPos2) )
	{
		wstrOrigHeaderText.replace(nPos1, nPos2-nPos1+1, wstrNewText);
		SetText(spSelection, wstrNewText, static_cast<int> (nPos1), static_cast<int> (nPos2-nPos1+1), strFont, fSize, lColor);
	}
	else
	{
		if(0 == wstrOrigHeaderText.length())
		{
			wstrNewHeaderText = wstrIndent + wstrNewText;
			SetText(spSelection, wstrNewHeaderText, 0, 0, strFont, fSize, lColor);
		}
		else
		{
			if( 1==wstrOrigHeaderText.length() && wstrOrigHeaderText==wzEmptyFooter)
			{
				wstrNewHeaderText = wstrIndent + wstrNewText;
				SetText(spSelection, wstrNewHeaderText, 0, 1, strFont, fSize, lColor);
			}
			else
			{
				if(bHeader)
				{
					wstrNewHeaderText = wstrIndent + wstrNewText + L"\n";
					SetText(spSelection, wstrNewHeaderText, 0, 0, strFont, fSize, lColor);
				}
				else
				{
					wstrNewHeaderText = L"\n" + wstrIndent + wstrNewText;
					SetText(spSelection, wstrNewHeaderText, static_cast<int> (wstrOrigHeaderText.length()), 0, strFont, fSize, lColor);
				}
			}
		}

	}

	bRet = TRUE;
_exit:
	if(bInHeaderFooterView)
		spView->put_SeekView(wdSeekMainDocument);
	return bRet;
}