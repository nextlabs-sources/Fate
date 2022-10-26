// NLPortableEncryptionCtx.h : main header file for the NLPortableEncryptionCtx DLL
//

#if !defined(AFX_NLPortableEncryptionCtx_H__982905C7_3928_11D3_FFEF_00500402F30B__INCLUDED_)
#define AFX_NLPortableEncryptionCtx_H__982905C7_3928_11D3_FFEF_00500402F30B__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CNLPortableEncryptionCtxApp
// See NLPortableEncryptionCtx.cpp for the implementation of this class
//

class CNLPortableEncryptionCtxApp : public CWinApp
{
public:
	CNLPortableEncryptionCtxApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNLPortableEncryptionCtxApp)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CNLPortableEncryptionCtxApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

extern UINT      g_cRefThisDll;

/////////////////////////////////////////////////////////////
// use GUIDGEN to generate a GUID for your shell extension
// ...
//use GUIDGEN to generate a GUID for your shell extension. call
//it "CLSID_MyFileNLPortableEncryptionCtxID"
//ex. (don't use this GUID!!)
//DEFINE_GUID(CLSID_MyFileNLPortableEncryptionCtxID, 
//0xc14f7671, 0x33d8, 0x13d3, 0xa0, 0xab, 0x0, 0x50, 0x4, 0x2, 0xf3, 0xb);
DEFINE_GUID(CLSID_MyFileNLPortableEncryptionCtxID, 
0xc14f7671, 0x33d8, 0x13d3, 0xa0, 0xab, 0x0, 0x50, 0x4, 0x2, 0xf3, 0xb);


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NLPortableEncryptionCtx_H__982905C7_3928_11D3_FFEF_00500402F30B__INCLUDED_)
