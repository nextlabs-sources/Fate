/*
Written by Derek Zheng
March 2008
*/

#pragma once

#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "log.h"
#include "GsmSdk.h"
#include "PgpSdkAdapter.h"

#if 0
GSMSDK_API CGsmSdk * GsmCreateSdkInstance(GsmSdkTypeEnum sdkType, LPCTSTR lpszKeyStorePath)
{
	switch (sdkType)
	{
	case GSM_SDK_TYPE_PGP:
		{
			CPgpSdkAdapter *pPgpSdk = new CPgpSdkAdapter();
			if (pPgpSdk)
			{
				GsmErrorT err = pPgpSdk->Init(lpszKeyStorePath);
				if (err)
				{
					DP((_T("Initialise PGP SDK failed(err=%d)\n"), err));
					delete pPgpSdk;
					pPgpSdk = NULL;
				}
			}

			return (CGsmSdk *)pPgpSdk;
		}
	default:
		break;
	}

	return NULL;
}

GSMSDK_API void GsmDestroySdkInstance(CGsmSdk *pSdk)
{
	if (pSdk)
	{
		pSdk->Release();
		delete pSdk;
	}
}
#endif