#ifndef __H_ODHD_FACT__
#define __H_ODHD_FACT__
#include "odhd_word.h"
#include "odhd_word2k7.h"
#include "odhd_excel.h"
#include "odhd_excel2k7.h"
#include "odhd_ppt.h"
#include "odhd_ppt2k7.h"

class CODHDInspectorFactory
{
public:
	static CODHDInspector* CreateODHDInspector(std::wstring srcFileName,CHdrProgDlg *pProgDlg=NULL)
	{
		EnumODType attachType=GetAttachType(srcFileName);
		CODHDInspector *pODHDInspector=NULL;
		switch(attachType)
		{
		case ODTYPE_WORD:
			if(pProgDlg!=NULL)
				pProgDlg->SetInspectItem(L"Creating Word instance ...");
#ifndef WSO2K3
			pODHDInspector=new CODHDWord2K7Inspector();
#else
			pODHDInspector=new CODHDWordInspector();
#endif
			break;
		case ODTYPE_EXCEL:
			if(pProgDlg!=NULL)
				pProgDlg->SetInspectItem(L"Creating Excel instance ...");
#ifndef WSO2K3
			pODHDInspector=new CODHDExcel2K7Inspector();
#else
			pODHDInspector=new CODHDExcelInspector();
#endif
			break;
		case ODTYPE_PPT:
			if(pProgDlg!=NULL)
				pProgDlg->SetInspectItem(L"Creating PowerPoint instance ...");
#ifndef WSO2K3
			pODHDInspector=new CODHDPpt2K7Inspector();
#else
			pODHDInspector=new CODHDPptInspector();
#endif
			
			break;
		default:
			break;

		}
		return pODHDInspector;
	}

	static EnumODType GetAttachType(std::wstring strFileName)
	{
		if(CODHDUtilities::IsWordFile(strFileName.c_str()))
			return ODTYPE_WORD;
		else if(CODHDUtilities::IsExcelFile(strFileName.c_str()))
			return ODTYPE_EXCEL;
		else if(CODHDUtilities::IsPptFile(strFileName.c_str()))
			return ODTYPE_PPT;
		else
			return ODTYPE_OTHERS;

	}
};



#endif //__H_ODHD_FACT__