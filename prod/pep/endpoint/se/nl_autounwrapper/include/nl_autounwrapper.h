// nl_autowrapper.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CNL_AutoUnwrapperApp:
// See nl_autounwrapper.cpp for the implementation of this class
//

class CNL_AutoUnwrapperApp : public CWinApp
{
public:
	CNL_AutoUnwrapperApp();

// Overrides
	public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CNL_AutoUnwrapperApp theApp;