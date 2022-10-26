#ifndef __H_ODHD_WORD2K7__
#define  __H_ODHD_WORD2K7__

#ifndef WSO2K3
#include "odhd.h"
//following constant should be arranged according the order of ODHDCATEGORY definition
static StaticCatTitle gs_CatTitle_Word2K7[]={
	{	
		L"Comments, Revisions, Versions, and Annotations",
		L"The following items were found:",
		L"No items were found.",
		L"All items were successfully removed.",
		L"Failed to remove comments or revisions or versions or annotations.",
		L"nothing",
		1
	},
	{
		L"Document Properties and Personal Information",
		L"The following document information was found:",
		L"No document properties or personal information was found.",
		L"Document properties and personal information were successfully removed.",
		L"Failed to remove document properties and personal information.",
		L"nothing",
		1
	},
#ifdef WSO2013
	{
		L"",
		L"",
		L"",
		L"",
		L"",
		L"",
		0
	},
#endif
		{
		L"Custom XML Data",
		L"Custom XML data was found.",
		L"No custom XML data was found.",
		L"Custom XML data was successfully removed.",
		L"Failed to remove custom XML data.",
		L"nothing",
		1
	},
	{
		L"Headers, Footers and Watermarks",
		L"The following items were found:",
		L"No headers, footers, or watermarks were found.",
		L"Headers,footers,and watermarks were removed from the document.",
		L"Failed to remove headers and footers.",
		L"nothing",
		1
	},
	{
		L"Hidden Text",
		L"Hidden text was found.",
		L"No hidden text was found.",
		L"Hidden text was successfully removed.",
		L"Failed to remove hidden text.",
		L"nothing",
		1
	}
};


static StaticItemResult gs_ItemTitle_Word2K7[]={
	{L"Comments",	L"Comments.",	L"No comment found.",	L"Can't remove comment(s).",0},
	{L"Revision",	L"Revision marks.",	L"No revision found.",	L"Can't remove revision(s).",0},
	{L"Version",	L"Document versions.",	L"No version found.",	L"Can't remove version(s).",0},
	{L"",L"",L"",L"",0},//Ink annotations

	{L"Document Properties",L"Document Properties.",L"No Document Property found.",L"Can't remove Document Property.",0},
	{L"E-mail headers",L"E-mail header(s).",L"No E-mail header found.",L"Can't remove E-mail header.",0},
	{L"Routing slips",L"Routing slips.",L"No Routing slips found.",L"Can't remove Routing slips.",0},
	{L"Send-for-review Information",L"Send-for-review Information.",L"No Send-for-review Information found.",L"Can't remove Send-for-review Information.",0},
	{L"Document server properties",L"Document server propertie(s).",L"No Document server propertie found.",L"Can't remove document server propertie(s).",0},
	{L"Document Management Policy information",L"Document Management Policy information.",L"No Document Management Policy information found.",L"Can't remove Document Management Policy information.",0},
	{L"Content type information",L"Content type information.",L"No Content type information found.",L"Can't remove Content type information.",0},
	{L"Databinding link information",L"Databinding link information.",L"No Databinding link information found.",L"Can't remove Databinding link information.",0},
	{L"User Name",L"User Name.",L"No User Name found.",L"Can't remove User Name.",0},
	{L"Template name",L"Template name.",L"No Template name found.",L"Can't remove Template name.",0},
	
	{L"Document Headers",L"Header(s).",L"No Document Header found.",L"Can't remove Document Header(s).",0},
	{L"Document Footers",L"Footer(s).",L"No Document Footer found.",L"Can't remove Document Footer(s).",0},
	{L"Watermarks",L"Watermark(s).",L"No watermark found.",L"Can't remove watermark(s).",0},
	
	{L"Hidden Text",L"Hidden Text.",L"No Hidden Text found.",L"Can't remove Hidden Text.",0},
	
	{L"Custom XML Data",L"Custom XML Data.",L"No Custom XML Data found.",L"Can't remove Custom XML Data.",0}
	//{L"",L"",L"",L""},
};

class CODHDWord2K7Inspector:public CODHDInspector
{
public:
	CODHDWord2K7Inspector():CODHDInspector((long)ODHDCAT_WORD_MAX,gs_CatTitle_Word2K7)
	{
		m_pApp=NULL;/*GetWordApp();*/
		m_HasRevision=FALSE;
		m_HasVersion=FALSE;
		m_HasComment=FALSE;
		m_HasContentType=FALSE;

		m_InspectorFail1=FALSE;
		m_InspectorFail2=FALSE;
		m_InspectorFail3=FALSE;
		m_InspectorIndex1=1;
		m_InspectorIndex2=2;
		m_InspectorIndex3=3;
		
	};
	virtual ~CODHDWord2K7Inspector(){};


	virtual BOOL			Inspect(std::wstring strSrcFileName,std::wstring strTempFileName);
	virtual BOOL			Remove(ODHDCATEGORY odhdCat,ODHDITEM odhdItem=ODHDITEM_DEFAULT);
	//virtual long			GetContent(ODHDCATEGORY odhdCat,ODHDITEM odhdItem,std::wstring &strContent);
	//virtual long			SetContent(ODHDCATEGORY odhdCat,ODHDITEM odhdItem,std::wstring &strContent);
	//virtual ODHDSTATUS	GetCatStatus(ODHDCATEGORY odhdCat);
	//virtual void			GetTitle(ODHDCATEGORY odhdCat,std::wstring &strTitle);//strTitle return combinated string of all title(name)s
	//virtual void			GetResult(ODHDCATEGORY odhdCat,std::wstring & strResult);//strResult return combinated string of all inspect's result
	virtual BOOL			IsValidPropertyName(LPCWSTR strPropName);

	virtual void			GetResult(ODHDCATEGORY odhdCat,std::wstring & strResult);

	virtual BOOL			FilterByCategory(ODHDCATEGORY odhdCat, ODHDSTATUS odhdStatus);
	virtual BOOL			GetFilter(ODHDCATEGORY odhdCat);
private:
	Word::_wordApplication* GetWordApp()
	{
		if(m_pApp==NULL)
		{
			m_pApp=MSOAppInstance::Instance()->GetWordAppInstance();
			assert(m_pApp);
		}
		return m_pApp;
	};
	BOOL	SaveDocAfterRemove(Word::_Document*pDoc);
	ODHDSTATUS	InspectComments(Word::_Document *pDoc);
	ODHDSTATUS	InspectRevision(Word::_Document *pDoc);
	ODHDSTATUS	InspectVersion(Word::_Document *pDoc);

	ODHDSTATUS	InspectDocProperty(Word::_Document *pDoc);
	 ODHDSTATUS	InspectBuiltinProperty(Word::_Document *pDoc);
	 ODHDSTATUS	InspectCustomProperty(Word::_Document *pDoc);
	ODHDSTATUS	InspectEmailHeader(Word::_Document *pDoc);
	ODHDSTATUS	InspectRoutingSlip(Word::_Document *pDoc);
	ODHDSTATUS	InspectSend4Review(Word::_Document*pDoc);
	ODHDSTATUS	InspectDocSrvProp(Word::_Document *pDoc);
	ODHDSTATUS	InspectSrvPolicy(Word::_Document *pDoc);
	ODHDSTATUS	InspectContentType(Word::_Document* pDoc);
	ODHDSTATUS	InspectDataLink(Word::_Document*pDoc);
	ODHDSTATUS	InspectUserName(Word::_Document*pDoc);
	ODHDSTATUS	InspectTemplate(Word::_Document*pDoc);

	ODHDSTATUS	InspectHeaderFooter(Word::_Document *pDoc);
	ODHDSTATUS	InspectHiddenText(Word::_Document *pDoc);
	ODHDSTATUS	InspectCustomXML(Word::_Document*pDoc);

	BOOL	RemoveCategoryComments(Word::_Document*pDoc);
	BOOL	RemoveCategoryDocProp(Word::_Document*pDoc);
	BOOL	RemoveCategoryHeaderFooter(Word::_Document*pDoc);
	BOOL	RemoveCategoryHiddenText(Word::_Document*pDoc);
	BOOL	RemoveCategoryCustomXML(Word::_Document*pDoc);
#ifdef WSO2013
	BOOL ProcessOutlookTempFolderinReg(const int op, std::wstring& wstrRegValBackup);
#endif
	Word::_wordApplication* m_pApp;
	BOOL	m_HasRevision;
	BOOL	m_HasVersion;
	BOOL	m_HasComment;
	BOOL	m_HasContentType;

	DWORD   m_InspectorFail1;
	DWORD   m_InspectorFail2;
	DWORD   m_InspectorFail3;

	DWORD	m_InspectorIndex1;//Custom XML
	DWORD	m_InspectorIndex2;//Header and Footers
	DWORD	m_InspectorIndex3;//Hidden Text

	
};

#endif //#ifdef WSO2K7

#endif
