#ifndef __H_ODHD_PPT__
#define __H_ODHD_PPT__
#include "odhd.h"

//following constant should be arranged according the order of ODHDCATEGORY definition
static StaticCatTitle gs_CatTitle_Ppt[]={
	{
		L"Comments and Annotations",
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
	/*{
		L"Custom XML Data",
		L"Custom XML data was found.",
		L"No custom XML data was found.",
		L"Custom XML data was successfully removed.",
		L"Failed to remove custom XML data.",
		L"nothing",
		0
	},
	{
		L"Invisible On-Slide Content",
		L"Invisible objects were found.",
		L"No invisible objects found.",
		L"Invisible objects were successfully removed.",
		L"Failed to remove invisible objects.",
		L"nothing",
		0
	},
	{
		L"Off-Slide Content",
		L"Off-Slide content was found.",
		L"No off-slide content was found.",
		L"Off-Slide content was successfully removed.",
		L"Failed to remove off-slide content.",
		L"nothing",
		0
	},
	{
		L"Presentation Notes",
		L"Presentation notes were found.",
		L"No presentation notes were found.",
		L"All presentation notes were removed.",
		L"Failed to remove presentation notes.",
		L"nothing",
		0
	}*/
};
static StaticItemResult gs_ItemTitle_Ppt[]={
	{L"Comments",	L"Comment(s).",	L"No comment found.",	L"Can't remove comment(s).",0},
	{L"",L"",L"",L"",0},//Ink annotations

	{L"Document Properties",L"Document Properties.",L"No Document Property found.",L"Can't remove Document Property.",0},
	{L"E-mail headers",L"E-mail header(s).",L"No E-mail header found.",L"Can't remove E-mail header.",0},
	{L"Routing slips",L"Routing slips.",L"No Routing slips found.",L"Can't remove Routing slips.",0},
	{L"Send-for-review Information",L"Send-for-review Information.",L"No Send-for-review Information found.",L"Can't remove Send-for-review Information.",0},
	{L"Document server properties",L"Document server propertie(s).",L"No Document server propertie found.",L"Can't remove document server propertie(s).",0},
	{L"Document Management Policy information",L"Document Management Policy information.",L"No Document Management Policy information found.",L"Can't remove Document Management Policy information.",0},
	{L"Content type information",L"Content type information.",L"No Content type information found.",L"Can't remove Content type information.",0},
	{L"File path for publishing web pages",L"File path for publishing web pages.",L"No file path found.",L"Can't remove File path for publishing web pages.",0},
	
	{L"Invisible On-Slide content",L"Invisible On-Slide content",L"No Invisible On-Slide content.",L"Can't remove invisible on-slide content.",0},
	
	{L"Off-Slide Content",L"Off-Slide Content.",L"No Off-Slide Content found.",L"Can't remove Off-Slide Content.",0},
	
	{L"Presentation Notes",L"Presentation Notes.",L"No Presentation Note(s) found.",L"Can't remove Presentation Notes.",0},
	
	{L"Custom XML Data",L"Custom XML Data.",L"No Custom XML Data found.",L"Can't remove Custom XML Data.",0},

};
class CODHDPptInspector:public CODHDInspector
{
public:
	CODHDPptInspector():CODHDInspector((long)ODHDCAT_PPT_MAX,gs_CatTitle_Ppt){m_pApp=NULL;GetPPTApp();};
	virtual ~CODHDPptInspector(){};
	virtual BOOL			Inspect(std::wstring strSrcFileName,std::wstring strTempFileName);
	virtual BOOL			Remove(ODHDCATEGORY odhdCat,ODHDITEM odhdItem=ODHDITEM_DEFAULT);
	virtual BOOL			IsValidPropertyName(LPCWSTR strPropName);

	virtual BOOL			FilterByCategory(ODHDCATEGORY odhdCat, ODHDSTATUS odhdStatus);
	virtual BOOL			GetFilter(ODHDCATEGORY odhdCat);

private:
	PPT::_pptApplication* GetPPTApp()
	{
		if(m_pApp==NULL)
		{
			m_pApp=MSOAppInstance::Instance()->GetPPTAppInstance();
			assert(m_pApp);
		}
		return m_pApp;
	};

	ODHDSTATUS		InspectComments(PPT::_Presentation *pDoc);

	ODHDSTATUS		InspectDocProperty(PPT::_Presentation *pDoc);
	  ODHDSTATUS	InspectBuiltinProperty(PPT::_Presentation *pDoc);
	  ODHDSTATUS	InspectCustomProperty(PPT::_Presentation *pDoc);
	ODHDSTATUS		InspectEmailHeader(PPT::_Presentation *pDoc);
	ODHDSTATUS		InspectRoutingSlip(PPT::_Presentation *pDoc);
	ODHDSTATUS		InspectSend4Review(PPT::_Presentation *pDoc);
	ODHDSTATUS		InspectDocSrvProp(PPT::_Presentation *pDoc);
	ODHDSTATUS		InspectSrvPolicy(PPT::_Presentation *pDoc);
	ODHDSTATUS		InspectContentType(PPT::_Presentation *pDoc);
	ODHDSTATUS		InspectWebPublish(PPT::_Presentation *pDoc);
	ODHDSTATUS		InspectOnSlideInvisible(PPT::_Presentation *pDoc);
	ODHDSTATUS		InspectOffSlide(PPT::_Presentation *pDoc);
	ODHDSTATUS		InspectPresentationNote(PPT::_Presentation *pDoc);
	
	ODHDSTATUS		InspectCustomXML(PPT::_Presentation *pDoc);

	BOOL			SaveDocAfterRemove(PPT::_Presentation*pDoc);

	BOOL			RemoveCategoryComments(PPT::_Presentation*pDoc);
	BOOL			RemoveCategoryDocProp(PPT::_Presentation*pDoc);
	
	BOOL			RemoveWebPublish(PPT::_Presentation*pDoc);
	
	PPT::_pptApplication* m_pApp;
};






#endif //__H_ODHD_PPT__