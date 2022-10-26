// Obligation.h : main header file for the PROJECT_NAME application
//
#ifndef _OBLIGATION_H_
#define _OBLIGATION_H_

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif


#include "resource.h"		// main symbols
#include <string>

#include <vector>

typedef struct _base_argument_flex
{
	//parameters from command line
	std::wstring wstrSource;
	std::wstring wstrRequesterAddress;
	std::wstring wstrUser;
	std::wstring wstrApprovers;
	std::wstring wstrFtpDir;
	std::wstring wstrFtpUser;
	std::wstring wstrFtpPasswd;
	std::wstring wstrRecipients;
	std::wstring wstrRequesterDisplayName;
	std::wstring wstrMessageFile; //which include the Subject and the email body

	//inputs by user
	std::wstring wstrSelectedApprover;
	
	std::wstring wstrPurpose;//this is Content
	std::wstring wstrSubject;//this is Subject
	std::vector<std::wstring> vFiles;

	//
	std::wstring wstrCurUserName;
	std::wstring wstrCurSID;
}BaseArgumentFlex;

// CObligationApp:
// See Obligation.cpp for the implementation of this class
//

class CObligationApp : public CWinApp
{
public:
	static const WCHAR PARA_SOURCE[];
	static const WCHAR PARA_USER[];
	static const WCHAR PARA_RECIPIENTS[];
	static const WCHAR PARA_APPROVERS[];
	static const WCHAR PARA_FTPDIR[];
	static const WCHAR PARA_FTPUSER[];
	static const WCHAR PARA_FTPPASSWD[];
	static const WCHAR PARA_MESSAGEFILE[];
public:
	CObligationApp();

// Overrides
	public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
public:
	// Parse CommandLine and initialize the Base Argument in according to CommandLine
	bool ParseCommandLine(const wchar_t* pCommandLine);
	bool ParseCommandLineFlex(int argc, LPWSTR* argv,BaseArgumentFlex & baseArguFlex);
public:
	// check the common available for all of the type
	bool CheckAvailable(void);
public:
	bool DoWork(void);
	bool DoWork(BaseArgumentFlex &baseArgumentFlex);
};


extern CObligationApp theApp;


//extern BaseArgumentFlex g_BaseArgumentFlex;


#endif //_OBLIGATION_H_