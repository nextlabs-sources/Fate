#include "..\include\Classify.h"
#include "celog.h"

#define CELOG_CUR_MODULE L"ADOBEPEP"
#undef CELOG_CUR_FILE
#define CELOG_CUR_FILE CELOG_FILEPATH_PROD_PEP_ENDPOINT_ADOBEPEP_PLUGIN_SAMPLESADOBEPEP_SRC_CLASSIFY_CPP

CClassify::CClassify(void)
{
	m_bShowed=false;
}

CClassify::~CClassify(void)
{
}

void CClassify::ShowDocPropDlg(bool bShow)
{
	if (true==bShow)
	{
		CELOG_LOGA(CELOG_DEBUG, "document properties dialog is showed\n");
	}
	else
	{
		CELOG_LOGA(CELOG_DEBUG, "document properties dialog is hidden\n");
	}
	
	m_bShowed=bShow;
}
bool CClassify::IsDocPropDlgShowed()
{
	return m_bShowed;
}