#include "..\include\Send.h"
#include "celog.h"

#define CELOG_CUR_MODULE L"ADOBEPEP"
#undef CELOG_CUR_FILE
#define CELOG_CUR_FILE CELOG_FILEPATH_PROD_PEP_ENDPOINT_ADOBEPEP_PLUGIN_SAMPLESADOBEPEP_SRC_SEND_CPP

CSend::CSend(void)
{
	m_bShowed=false;
}

CSend::~CSend(void)
{
}

void CSend::ShowSendAndCollaborateLive(bool bshow)
{
	if (true==bshow)
	{
		CELOG_LOGA(CELOG_DEBUG, "send and collaborate live dialog is showed\n");
	}
	else
	{
		CELOG_LOGA(CELOG_DEBUG, "send and collaborate live dialog is hidden\n");
	}

	m_bShowed=bshow;
}

bool CSend::IsSendCollaborateLiveShowed()
{
	return m_bShowed;
}
