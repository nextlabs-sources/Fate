#pragma  once
/*! works for PowerPoint document */
class CPPTLabeling 
{
public:
	CPPTLabeling(){};
	~CPPTLabeling(){};
	virtual bool SetWatermarkText(IDispatch* pDoc,const MARKUP& theMARKUPINFO);

	virtual bool SetWatermarkImage(IDispatch* pDoc,const MARKUP& theMARKUPINFO){return false;};

	virtual bool SetHeaderText(IDispatch* pDoc,const MARKUP& theMARKUPINFO);

	virtual bool SetFooterText(IDispatch* pDoc,const MARKUP& theMARKUPINFO);

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

private:
	bool AddHeaderFooter(BOOL IsHeader, PPT::pptHeaderFooterPtr ptrHFer,const MARKUP& theInfo);
};