#pragma  once

/*! works for Word document */
class CWordLabeling
{
public:
	CWordLabeling(){};
	~CWordLabeling(){};

	virtual bool SetWatermarkText(IDispatch* pDoc,const MARKUP& theMARKUPINFO);

	virtual bool SetWatermarkImage(IDispatch* pDoc,const MARKUP& theMARKUPINFO){return false;};

	virtual bool SetHeaderText(IDispatch* ptrDoc,const MARKUP& theInfo);

	virtual bool SetFooterText(IDispatch* ptrDoc,const MARKUP& theInfo);

	virtual bool SetWatermarkText(const wchar_t* strDocPath,const MARKUP& theMARKUPINFO){return false;};

	virtual bool SetWatermarkImage(const wchar_t* strDocPath,const MARKUP& theMARKUPINFO){return false;};

	virtual bool SetHeaderText(const wchar_t* strDocPath,const MARKUP& theMARKUPINFO)
	{
		//OutputDebugStringW(strDocPath);
		return false;
	}

	virtual bool SetFooterText(const wchar_t* strDocPath,const MARKUP& theMARKUPINFO){return false;};

	virtual bool RemoveWatermarkText(IDispatch* pDoc,const wchar_t* strTagName);

	virtual bool RemoveWatermarkImage(IDispatch* pDoc,const wchar_t* strTagName){return false;};

	virtual bool RemoveHeaderText(IDispatch* pDoc,const wchar_t* strTextValue){return false;};

	virtual bool RemoveFooterText(IDispatch* pDoc,const wchar_t* strTextValue){return false;};


	virtual bool RemoveWatermarkText(const wchar_t* strDocPath,const wchar_t* strTagName){return false;};

	virtual bool RemoveWatermarkImage(const wchar_t* strDocPath,const wchar_t* strTagName){return false;};

	virtual bool RemoveHeaderText(const wchar_t* strDocPath,const wchar_t* strTextValue){return false;};

	virtual bool RemoveFooterText(const wchar_t* strDocPath,const wchar_t* strTextValue){return false;};

public:
	BOOL MarkupHeaderFooter(const LPDISPATCH pDoc,
		const std::wstring& strMarkupText,
		const std::wstring& strJustify,
		const std::wstring& strFont,
		float fSize,
		long lColor,
		BOOL bHeader);

protected:
	std::wstring GetIndent(const std::wstring& Justification);
	int GetLines(const std::wstring& wstrText, std::vector<std::wstring>& vecLines);
	std::wstring ResetIndent(const std::wstring& text, const std::wstring& markuptext, const std::wstring& indent);
	void GetHeaderText(Word::wordHeaderFooter* pHeader, std::wstring& wstrHeaderText);
	void SetText(Word::wordSelection* pSelection,
		const std::wstring& wstrText,
		int offset,
		int overwrite,
		const std::wstring& wstrFont,
		float fSize,
		long lColor);

};