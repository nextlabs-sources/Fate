// IEObj.cpp : Implementation of CIEObj

#include "stdafx.h"
#include "IEObj.h"
#include "event_handler.h"
#include "celog.h"
#include "utils.h"

#define CELOG_CUR_MODULE L"iePEP"
#define CELOG_CUR_FILE CELOG_FILEPATH_PROD_PEP_ENDPOINT_WDE_IEPEP_SRC_IEOHJ_CPP

nextlabs::cesdk_context cesdk_context_instance;
unsigned int g_ip = 0;

// CIEObj
HRESULT CIEObj::SetSite(IUnknown *pUnkSite)
{
	CELOG_LOG(CELOG_DUMP, L" The Parameters are: pUnkSite=%p \n", pUnkSite);

    if (pUnkSite !=  NULL)
    {
        WebBrowserEventHandler* spHandler = new WebBrowserEventHandler((IWebBrowser2*)pUnkSite);
        if(spHandler)
        {
            spHandler->RegisterEventDispatch(pUnkSite);
			
			nextlabs::comm_helper::Init_Cesdk(&cesdk_context_instance);

			g_ip = GetIp();
        }
    }

    return E_FAIL;
}