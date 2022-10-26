// ItemEventDisp.cpp : Implementation of CItemEventDisp

#include "stdafx.h"
#include "resattrmgr.h"
#include "ItemEventDisp.h"
#include "outlookUtilities.h"
#include "../common/FileCache.h"
#include "../common/policy.h"
#include "../common/CommonTools.h"
#include "outlookobj.h"
#include "ParameterForMulQuery.h"
#include "AttachmentObligationData.h"
#include "ExceptionHandler.h"

#pragma warning(push)

#pragma warning(disable: 4267)
#include "nlexcept.h"
#pragma warning(pop)
#include "../service/svragent.h"
#include "mailAttach.h"
#include "interDlg.h"
#include <hash_set>
#include "assistantdlg.h"
#include "../ca/caadapter.h"
#include "..\PAEx\OE_PAMngr.h"
#include "adaptermanager.h"
#include "..\Yeled\YeledDlg.h"
#include "MailItemUtility.h"
#pragma warning(push)
#pragma warning(disable: 6334)
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#pragma warning(pop)
#include "eframework/platform/cesdk_loader.hpp"
#include "nextlabs_feature_manager.hpp"
#include "nextlabs_features.h"
#include "msopath.hpp"
#include "DlgAlertMessage.h"
#include "../outlook/QueryPCInfo.h"
#include "../common/AttachmentFileMgr.h"
#include "../common/RuntimeTracer.h"
#include "outlookUtilities.h"
#include "MailPropertyTool.h"

#pragma comment(lib,"libcmt.lib")

#define OB_NAME_DM                      L"MISMATCH"
#define OB_NAME_MC                      L"MULTIPLE"
#define OB_NAME_MT                      L"MISSING"
#define OB_NAME_IUO                     L"INTERNAL"
#define OB_NAME_ER                      L"EXTERNAL"
#define OB_NAME_HDR                     L"HDR"
#define OB_NAME_LD                      L"LOGDECISION"
#define OB_NAME_MAP						L"MAIL_ATTR_PARSING"
#define OB_NAME_IFT						L"INTERACTIVE_FILE_TAGGING"
#define OB_NAME_IFT_EXIST				L"INTERACTIVE_FILE_TAGGING_EXIST"

#define OB_NAME_OHC						L"OE_HIERARCHICAL_CLASSIFICATION"
#define OB_NAME_OHC_EXIST				L"OE_HIERARCHICAL_CLASSIFICATION_EXIST"

#define OB_NAME_AFT						L"AUTOMATIC_FILE_TAGGING"
#define OB_NAME_AFT_EXIST				L"AUTOMATIC_FILE_TAGGING_EXIST"

#define OB_NAME_CRL						L"REJECT_UNLESS_SILENT_OVERRIDE"
#define OB_RL_ARG_NAME_LICENSE			L"Licenses"
#define OB_RL_ARG_NAME_ALLOW_OVERRIDE	L"Allow Override"
#define OB_RL_ARG_NAME_MESSAGE			L"Message"

#define OB_DM_ARG_NAME_URL              L"Help URL"
#define OB_DM_ARG_NAME_RECIPIENTS       L"Offending Recipients"
#define OB_DM_ARG_NAME_CLIENT_NAME      L"Resource Client Name"
#define OB_MC_ARG_NAME_URL              L"Help URL"
#define OB_MC_ARG_NAME_RECIPIENTS       L"Recipients"
#define OB_MT_ARG_NAME_URL              L"Help URL"
#define OB_MT_ARG_NAME_PROPERTY_NAME    L"Tag Name"
#define OB_MT_ARG_NAME_CLIENT_NAMES     L"Clients"
#define OB_MT_ARG_NAME_CLIENT_IDS       L"Client Ids"
#define OB_MT_ARG_NAME_NOT_CLIENT_RELATED_ID    L"Unknown Id"
#define OB_IUO_ARG_NAME_URL             L"Help URL"
#define OB_ER_ARG_NAME_URL              L"Help URL"
#define OB_ER_ARG_NAME_RECIPIENTS       L"Offending Recipients"
#define OB_HDR_ARG_NAME_URL             L"Help URL"
#define OB_LD_ARG_NAME_COOKIE           L"Cookie"
#define OB_MAP_ARG_NAME_TYPE			L"Result"
#define OB_MAP_ARG_NAME_MSG				L"Message"
#define OB_MAP_ARG_NAME_MIN				L"MinRecipients"
#define OB_MAP_ARG_NAME_MAX				L"MaxRecipients"

#define OB_IFT_ARG_NAME_TAGNAME			L"Tag Name"
#define OB_IFT_ARG_NAME_TAGVALUE		L"Tag Value"
#define OB_IFT_ARG_NAME_DOTAGFAIL		L"Do Tag Fail"

#define OB_RICH_USER_MESSAGE			L"RICH_USER_MESSAGE"

#define OB_OE_WARNING_MSG_PROCEED_CANCEL L"OE_WARNING_MSG_PROCEED_CANCEL"

#define OBLIGATION_STR_DELIM            L";"
#define OBLIGATION_URL_DEFAULT          L"http://www.nextlabs.com/"

#define OB_NAME_CE_NOTIFY				L"CE::NOTIFY"
#define OB_NAME_RICHALERT_MESSAGE       L"RICH_ALERT_MESSAGE"

HWND CItemEventDisp::s_hCurActiveWindow = NULL;

//following four lines are for ODHD
extern HINSTANCE	g_hODHD;
typedef std::vector<std::wstring> RecipientVector;
typedef std::vector<std::pair<std::wstring,std::wstring>> AttachVector;
typedef std::vector<std::wstring> HelpUrlVector;
typedef long (*TFunHdrObligation)(HWND ,RecipientVector &,AttachVector &,HelpUrlVector &,VARIANT_BOOL) ;

extern nextlabs::cesdk_loader cesdk;

 nextlabs::feature_manager feat;

//#ifdef _WIN64
//#pragma comment(lib, "resattrlib")
//#pragma comment(lib, "resattrmgr")
//#else
//#pragma comment(lib, "resattrlib32")
//#pragma comment(lib, "resattrmgr32")
//#endif
extern HINSTANCE    g_hInstance;
extern std::wstring g_strOETempFolder;
extern CFileCache   g_RealFileCache;
extern CFileCache   g_TempFileCache;

#ifdef WSO2K7
extern Word::_wordApplication*		g_spWordApp;
extern Excel::_excelApplication*	g_spExcelApp;
extern PPT::_pptApplication*		g_spPPTApp;
#endif

extern HINSTANCE	g_hTAG ;
extern HINSTANCE	g_hENC ;
extern HINSTANCE	g_hPE ;	
typedef struct _ATTACHTEMPFILEINFO
{
	WCHAR wzTempFilePath[MAX_PATH+1];
	WCHAR wzDispName[MAX_PATH+1];
}ATTACHTEMPFILEINFO, *LPATTACHTEMPFILEINFO;
static HRESULT SetTempFilePath(Attachment* Attachment, std::wstring& strRealFilePath, std::wstring& strTempFilePath, BOOL* pbInWordTempFolder);

extern CMeetingItemCache g_MeetingItemCache;
extern COutlookObj* g_pOutlookObj;

DWORD InitFeat()
{
    return feat.open();
}


PolicyCommunicator* g_spPolicyCommunicator = NULL;

//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////

CDmAssistDlgEx::CDmAssistDlgEx(LPCONTEXT pDmContext):m_pDmContext(pDmContext)
{
}
void CDmAssistDlgEx::ProcessOK()
{
	STRINGLIST* pListRecipients = m_pDmContext->pListRecipients;
	const std::vector<int>* pListRecipientIndices =
		m_pDmContext->pListRecipientIndices;
	std::vector<int> checkedList;
	int i=0, j=0;

	// Get the list of checked recipients.
	for (std::vector<CDmData>::iterator dmit=m_dmData.begin(); dmit!=m_dmData.end(); ++dmit)
	{
		if( (*dmit).bChecked ) checkedList.push_back(i);
		i++;
	}


	// Walk through all the recipients in the dialog.
	for (i = (int)pListRecipientIndices->size() - 1,
		j = (int)checkedList.size() - 1;
		i >= 0;
	i--)
	{
		if (j < 0 || i > checkedList[j])
		{
			// This recipient in the dialog is not selected.  Remove it.
			m_pDmContext->emailData->GetRecipientsData().AddRmRecipients((*pListRecipients)[(*pListRecipientIndices)[i]]);
			m_pDmContext->emailData->GetRecipientsData().SetRecipientsChanged(true);
		}
		else
		{
			// This recipient in the dialog is selected.  Skip it.
			j--;
		}
	}
}
void CDmAssistDlgEx::ProcessCancel()
{
}
void CDmAssistDlgEx::ProcessItemRemove(int nItem)
{
	
	const STRINGLIST* pListRecipients = m_pDmContext->pListRecipients;
	const std::vector<int>* pListRecipientIndices =
		m_pDmContext->pListRecipientIndices;
	const std::vector<BOOL>* pListRecipientDmFlags =
		m_pDmContext->pListRecipientDmFlags;
	const std::vector<BOOL>* pListRecipientMcFlags =
		m_pDmContext->pListRecipientMcFlags;
	

	vector<CAttachmentData>* pVecAttachmentsData = m_pDmContext->pvecAttachmentData;

	int* pnAttachments = m_pDmContext->pnAttachments;
	BOOL* pbNeedHdr = m_pDmContext->pbNeedHdr;


	BOOL bHdrAttachmentExists = FALSE;
	int i, j;

	i = (*pListRecipientIndices)[nItem];
	j = 0;

	//
	// Remove all attachments listed under this recipient in the dialog.
	//
	while (j < *pnAttachments)
	{
		if ((*pVecAttachmentsData)[j].IsAttachmentRemoved())
		{
			j++;
			continue;
		}
		
		
		if ((*pVecAttachmentsData)[j].dm &&
			OLUtilities::IsStringInListIgnoreCase
			((*pVecAttachmentsData)[j].dmRecipients, OBLIGATION_STR_DELIM,
			(*pListRecipients)[i].c_str()))
		{
			// This attachment is listed under this recipient.  Remove it.
			(*pVecAttachmentsData)[j].SetAttachmentRemoved(true);
			j++;
		}
		else
		{
			// This attachment is not listed under this recipient.  Skip it.
			if ((*pVecAttachmentsData)[j].hdr)
			{
				bHdrAttachmentExists = TRUE;
			}

			j++;
		}
	}

	if (*pbNeedHdr && !bHdrAttachmentExists)
	{
		// There were attachments under HDR, but there are now none.  Change
		// "Next" button to "Send Email" since we will skip HDR dialog
		// afterwards.
		*pbNeedHdr = FALSE;
		SetLastFlag();
	}

	// Remove all data
	m_viewDlg.CleanView();
	m_dmData.clear();

	//
	// Re-generate attachment lists for all recipients that are displayed in
	// the dialog.
	//
	UINT k;

	for (k = 0; k < pListRecipientIndices->size(); k++)
	{
		STRINGPAIRVECTOR info;

		i = (*pListRecipientIndices)[k];

		for (j = 0; j < *pnAttachments; j++)
		{
			if ((*pVecAttachmentsData)[j].IsAttachmentRemoved())
			{
				continue;
			}

			if ((*pVecAttachmentsData)[j].dm &&
				OLUtilities::IsStringInListIgnoreCase
				((*pVecAttachmentsData)[j].dmRecipients, OBLIGATION_STR_DELIM,
				(*pListRecipients)[i].c_str()))
			{
				info.push_back(std::pair<std::wstring, std::wstring>
					((*pVecAttachmentsData)[j].GetDispName(),
					(*pVecAttachmentsData)[j].dmClientName));
			}
		}

		AddDmItemData((*pListRecipientMcFlags)[k],
			(*pListRecipientDmFlags)[k],
			m_pDmContext->bSelectable,
			(*pListRecipients)[i].c_str(), info);
	}

	AddViewData();
}
void CDmAssistDlgEx::ProcessItemCheck(int nItem, BOOL bChecked)
{
	m_dmData[nItem].bChecked  = bChecked;
	{
		if(m_viewDlg.HasItemSelected())
			::EnableWindow(::GetDlgItem(m_hWnd, IDOK), TRUE);
		else
		{
			if(m_pDmContext->bCheckRecipientLicense)
			{
				if((static_cast<int>(m_dmData.size())==(*(m_pDmContext->pnRecipients))) && !m_pDmContext->bExistDefaultRecipients)
					::EnableWindow(::GetDlgItem(m_hWnd, IDOK), FALSE);
				else
					::EnableWindow(::GetDlgItem(m_hWnd, IDOK), TRUE);	
			}
			else
				::EnableWindow(::GetDlgItem(m_hWnd, IDOK), FALSE);
		}
	}
}
void CDmAssistDlgEx::ProcessItemCbnSelect(int nItem, int nSel)
{
	UNREFERENCED_PARAMETER(nSel);
	UNREFERENCED_PARAMETER(nItem);
	//WCHAR wzMsg[256];
	//swprintf(wzMsg, 255, L"Item %d select client #%d (Zero base)\n", nItem, nSel);
	//MessageBoxW(wzMsg);
}
void CDmAssistDlgEx::AddDmItemData(BOOL bMultiUser, BOOL bMultiClient, BOOL bSelectable, LPCWSTR pwzRecipient, STRINGPAIRVECTOR& pairString)
{
	int i = (int)m_dmData.size();
	m_dmData.push_back(CDmData());
	m_dmData[i].bMultiUser  = bMultiUser;
	m_dmData[i].bMultiClient= bMultiClient;
	m_dmData[i].bSelectable = bSelectable;
	m_dmData[i].strRecipient= pwzRecipient;
	m_dmData[i].attachInfo = pairString;
}

void CDmAssistDlgEx::AddViewData()
{
	for (std::vector<CDmData>::iterator it=m_dmData.begin(); it!=m_dmData.end(); ++it)
	{
		m_viewDlg.AddItem(*it);
	}
	m_viewDlg.ResetView();
}
void CDmAssistDlgEx::ReallocWindows()
{
	RECT rcWinClient, rcUserClient;
	int  nCY = 0;

	if(m_pDmContext->bCheckRecipientLicense)
		::SetWindowTextW(m_hWnd, L"Review Recipients");
	// Set Icon
	HICON hIcon = (HICON)::LoadImage(g_hInstance, MAKEINTRESOURCE(IDI_MAIN), 
		IMAGE_ICON, ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON), LR_DEFAULTCOLOR);
	SetIcon(hIcon, TRUE);
	HICON hIconSmall = (HICON)::LoadImage(g_hInstance, MAKEINTRESOURCE(IDI_MAIN), 
		IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);
	SetIcon(hIconSmall, FALSE);

	// Set whole windows position
	SetWindowPos(HWND_TOP, 0, 0, WINCX, WINCY, SWP_NOMOVE);
	GetClientRect(&rcWinClient);
	rcUserClient.top   = rcUserClient.left = MYMARGIN;
	rcUserClient.right = rcWinClient.right - MYMARGIN;
	rcUserClient.bottom= rcWinClient.bottom - MYMARGIN;
	nCY = rcUserClient.top;

	HFONT  hFont = NULL;
	if (m_pDmContext->bCheckRecipientLicense)
	{
		hFont = CreateFont(16,0,0,0,FW_NORMAL,FALSE,FALSE,0,ANSI_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH | FF_SWISS,_T("Segoe UI"));
	}

	// Set main icon and text
	int iNewLineCount=2;
	int iMessageHigh=BIGICON;
	bool bIsShowRecipientMsg = false;
	if(m_pDmContext->bCheckRecipientLicense)
	{
		m_HyperLink.GetDisplayMsg(m_pDmContext->strMessage,m_HyperLink.m_vecMsg,m_HyperLink.m_strHttpAddr,m_HyperLink.m_nLineNumber);

		if (!m_HyperLink.m_vecMsg.empty())
		{
			bIsShowRecipientMsg = true;
			if (m_HyperLink.m_nLineNumber > 5)
			{
				m_HyperLink.m_nLineNumber = 5;
			}

			iMessageHigh=static_cast<int>((m_HyperLink.m_nLineNumber*BIGICON))/2;
		}
		else
		{
			iMessageHigh=(iNewLineCount*BIGICON)/2;
		}
		if(iMessageHigh < BIGICON)
			iMessageHigh = BIGICON;
	}
	::SetWindowPos(::GetDlgItem(m_hWnd, IDC_EXTMAIL), HWND_TOP, rcUserClient.left, nCY, BIGICON, BIGICON, SWP_SHOWWINDOW);
	if (bIsShowRecipientMsg)
	{
		::SetWindowPos(::GetDlgItem(m_hWnd, IDC_NOTIFY), HWND_TOP, rcUserClient.left+BIGICON+MYMARGIN/2, nCY, WIDTH(rcUserClient)-(BIGICON+MYMARGIN/2), iMessageHigh/*BIGICON*/, SWP_HIDEWINDOW);
	}
	else
	{
		::SetWindowPos(::GetDlgItem(m_hWnd, IDC_NOTIFY), HWND_TOP, rcUserClient.left+BIGICON+MYMARGIN/2, nCY, WIDTH(rcUserClient)-(BIGICON+MYMARGIN/2), iMessageHigh/*BIGICON*/, SWP_SHOWWINDOW);
	}

	if(m_pDmContext->bCheckRecipientLicense)
	{
		if (bIsShowRecipientMsg)
		{
			m_HyperLink.m_hParentWnd = m_hWnd;
			m_HyperLink.m_hCtrlWnd = ::GetDlgItem(m_hWnd, IDC_NOTIFY);
			m_HyperLink.CovertTextToHyperLink();
		}
		else
		{
			::SetWindowTextW(::GetDlgItem(m_hWnd, IDC_NOTIFY), DM_DLG_INFO_CRL);
		}

	}
	else
		::SetWindowTextW(::GetDlgItem(m_hWnd, IDC_NOTIFY), DM_DLG_INFO);
	//nCY += BIGICON;
	nCY +=iMessageHigh;

	// Set Horz Etched Line
	nCY += LINESPACE;
	::SetWindowPos(::GetDlgItem(m_hWnd, IDC_HORZETCHED), HWND_TOP, rcUserClient.left, nCY, WIDTH(rcUserClient), LINESPACE, SWP_SHOWWINDOW);
	nCY += LINESPACE;

	// Set Multiple Client
	if(m_pDmContext->bCheckRecipientLicense==FALSE)
	{
		nCY += LINESPACE;
		::SetWindowPos(::GetDlgItem(m_hWnd, IDC_MULTICLIENT), HWND_TOP, rcUserClient.left+MYMARGIN, nCY, SMALLICON, SMALLICON, SWP_SHOWWINDOW);
		::SetWindowPos(::GetDlgItem(m_hWnd, IDC_MULTICLIENTDESC), HWND_TOP, rcUserClient.left+MYMARGIN+SMALLICON*2, nCY, 300, SMALLICON, SWP_SHOWWINDOW);
	}
	else
	{
		::SetWindowPos(::GetDlgItem(m_hWnd, IDC_MULTICLIENT), HWND_BOTTOM, rcUserClient.left+MYMARGIN, nCY, SMALLICON, SMALLICON, SWP_HIDEWINDOW);
		::SetWindowPos(::GetDlgItem(m_hWnd, IDC_MULTICLIENTDESC), HWND_BOTTOM, rcUserClient.left+MYMARGIN+SMALLICON*2, nCY, 300, SMALLICON, SWP_HIDEWINDOW);
	}
	::SendMessage(::GetDlgItem(m_hWnd, IDC_MULTICLIENTDESC), WM_SETFONT, (WPARAM)g_fntNormal, (LPARAM)TRUE);
	if(m_pDmContext->bCheckRecipientLicense==FALSE)
		nCY += SMALLICON;

	// Set Multiple User
	if(m_pDmContext->bCheckRecipientLicense==FALSE)
	{
		nCY += LINESPACE;
		::SetWindowPos(::GetDlgItem(m_hWnd, IDC_MULTIUSER), HWND_TOP, rcUserClient.left+MYMARGIN, nCY, SMALLICON, SMALLICON, SWP_SHOWWINDOW);
		::SetWindowPos(::GetDlgItem(m_hWnd, IDC_MULTIUSERDESC), HWND_TOP, rcUserClient.left+MYMARGIN+SMALLICON*2, nCY, 300, SMALLICON, SWP_SHOWWINDOW);
	}
	else
	{
		::SetWindowPos(::GetDlgItem(m_hWnd, IDC_MULTIUSER), HWND_BOTTOM, rcUserClient.left+MYMARGIN, nCY, SMALLICON, SMALLICON, SWP_HIDEWINDOW);
		::SetWindowPos(::GetDlgItem(m_hWnd, IDC_MULTIUSERDESC), HWND_BOTTOM, rcUserClient.left+MYMARGIN+SMALLICON*2, nCY, 300, SMALLICON, SWP_HIDEWINDOW);
	}
	::SendMessage(::GetDlgItem(m_hWnd, IDC_MULTIUSERDESC), WM_SETFONT, (WPARAM)g_fntNormal, (LPARAM)TRUE);
	if(m_pDmContext->bCheckRecipientLicense==FALSE)
		nCY += SMALLICON;

	// Set View
	if(m_pDmContext->bCheckRecipientLicense)
		nCY += LINESPACE;
	else
		nCY += LINESPACE*2;
	CViewItemDlg::m_nX     = MYMARGIN;
	CViewItemDlg::m_nWidth = WIDTH(rcUserClient) - MYMARGIN -2 - ::GetSystemMetrics(SM_CXVSCROLL);
	m_viewDlg.Create(m_hWnd, 0);
	m_viewDlg.SetWindowPos(HWND_TOP, rcUserClient.left, nCY, WIDTH(rcUserClient), HEIGHT(rcUserClient)-nCY-BUTTONCY-5, SWP_SHOWWINDOW);

	// Set Button
	nCY = rcUserClient.bottom-BUTTONCY;
	::SetWindowPos(::GetDlgItem(m_hWnd, IDOK), HWND_TOP, rcUserClient.right-BUTTONCX*2-10, nCY, BUTTONCX, BUTTONCY, SWP_SHOWWINDOW);
	if(!m_bLast) ::SetWindowTextW(::GetDlgItem(m_hWnd, IDOK), L"Next");
	if(m_pDmContext->bCheckRecipientLicense)
	{
		::SendMessage( ::GetDlgItem(m_hWnd,IDOK), WM_SETFONT, (WPARAM)hFont, MAKELONG(TRUE,0) );
		::SetWindowTextW(::GetDlgItem(m_hWnd, IDOK), L"Send Email");
		if((static_cast<int>(m_dmData.size())==(*(m_pDmContext->pnRecipients))) && !m_pDmContext->bExistDefaultRecipients)
			::EnableWindow(::GetDlgItem(m_hWnd, IDOK), FALSE);
		else
			::EnableWindow(::GetDlgItem(m_hWnd, IDOK), TRUE);	
	}
	else
		::EnableWindow(::GetDlgItem(m_hWnd, IDOK), FALSE);
	if (m_pDmContext->bCheckRecipientLicense)
	{
		::SendMessage( ::GetDlgItem(m_hWnd,IDCANCEL), WM_SETFONT, (WPARAM)hFont, MAKELONG(TRUE,0) );
		::SetWindowTextW(::GetDlgItem(m_hWnd, IDCANCEL), L"Cancel");
	}

	::SetWindowPos(::GetDlgItem(m_hWnd, IDCANCEL), HWND_TOP, rcUserClient.right-BUTTONCX, nCY, BUTTONCX, BUTTONCY, SWP_SHOWWINDOW);

}



CTagAssistDlgEx::CTagAssistDlgEx(LPCONTEXT pTagContext):m_pTagContext(pTagContext)
{
}
void CTagAssistDlgEx::ProcessOK()
{

	vector<CAttachmentData>* pVecAttachmentsData = m_pTagContext->pvecAttachmentData;
	std::vector<int> vTagIndices;
	ResourceAttributeManager *pMgr;

	// Get the list of tag indices.
	for (std::vector<CTagData>::iterator ittd=m_tagData.begin(); ittd!=m_tagData.end(); ++ittd)
	{
		vTagIndices.push_back((*ittd).nSelect);
	}        

	if(0 >= vTagIndices.size())
	{
		DP((L"CTagAssistDlg::ProcessOK: error: No tags is found\n"));
		return;
	}

	if (0 != glfCreateAttributeManager(&pMgr) || NULL == pMgr)
	{
		DP((L"CTagAssistDlg::ProcessOK: error: Can't create attribute mgr\n"));
		pMgr = NULL;
	}

	std::vector<int>::iterator it = vTagIndices.begin();

	for (UINT i = 0; i < (*pVecAttachmentsData).size(); i++)
	{

		if ((*pVecAttachmentsData)[i].mt)
		{
			STRINGLIST vClientIds;

			vClientIds.push_back((*pVecAttachmentsData)[i].mtNotClientRelatedId);
			OLUtilities::ParseStringList((*pVecAttachmentsData)[i].mtClientIds,
				OBLIGATION_STR_DELIM, vClientIds);
			wcsncpy_s((*pVecAttachmentsData)[i].mtClientIdChosen,
				_countof((*pVecAttachmentsData)[i].mtClientIdChosen),
				vClientIds[*it].c_str(), _TRUNCATE);

			if (NULL == pMgr) {
				continue;
			}

			ResourceAttributes *pAttr = NULL;

			if (0 != glfAllocAttributes(&pAttr) || NULL == pAttr)
			{
				DP((L"CTagAssistDlg::ProcessOK: error: Can't alloc attribute\n"));
			}
			else
			{
				glfAddAttributeW(pAttr, (*pVecAttachmentsData)[i].mtPropertyName,
					vClientIds[*it].c_str());

				// Write the client ID back to the file.  Ignore any error.
				if (0 != glfWriteResourceAttributesW(pMgr, (*pVecAttachmentsData)[i].GetSourcePath().c_str(),
					pAttr))
				{
					DP((L"CTagAssistDlg::ProcessOK: error: Can't write attribute\n"));
				}

				if (0 != glfWriteResourceAttributesW(pMgr, (*pVecAttachmentsData)[i].GetTempPath().c_str(),
					pAttr))
				{
					DP((L"CTagAssistDlg::ProcessOK: error: Can't write attribute to temp attachment file\n"));
				}
				glfFreeAttributes(pAttr);
			}

			it++;
		}
	}

	if (NULL != pMgr)
	{
		glfCloseAttributeManager(pMgr);
	}
}
void CTagAssistDlgEx::ProcessCancel()
{
}
void CTagAssistDlgEx::ProcessItemCbnSelect(int nItem, int nSel)
{
	m_tagData[nItem].nSelect = nSel;
}
void CTagAssistDlgEx::AddTagItemData(LPCWSTR pwzAttach, STRINGVECTOR& vecClients)
{
	int i = (int)m_tagData.size();
	m_tagData.push_back(CTagData());
	m_tagData[i].strFile    = pwzAttach;
	m_tagData[i].vecClients = vecClients;
}
void CTagAssistDlgEx::AddTagItemData(LPCWSTR pwzAttach, LPCWSTR pwzClients)
{
	int i = (int)m_tagData.size();
	m_tagData.push_back(CTagData());
	m_tagData[i].strFile    = pwzAttach;
	ParseClients(pwzClients, m_tagData[i].vecClients);
}
void CTagAssistDlgEx::ParseClients(LPCWSTR pwzClients, STRINGVECTOR& vClients)
{
	const WCHAR* pwzStart = pwzClients;
	const WCHAR* pwzEnd   = wcsstr(pwzStart, L";");
	do 
	{
		if(NULL==pwzEnd)
		{
			vClients.push_back(pwzStart);
			break;
		}
		if(pwzEnd != pwzStart)
		{
			std::wstring strTemp(pwzStart, (pwzEnd-pwzStart));
			vClients.push_back(strTemp);
		}
		pwzStart = ++pwzEnd;
		pwzEnd = wcsstr(pwzStart, L";");
	} while(1);
}


void CTagAssistDlgEx::AddViewData()
{
	for (std::vector<CTagData>::iterator it=m_tagData.begin(); it!=m_tagData.end(); ++it)
	{
		m_viewDlg.AddItem(*it);
	}
}
void CTagAssistDlgEx::ReallocWindows()
{
	RECT rcWinClient, rcUserClient;
	int  nCY = 0;

	// Set Icon
	HICON hIcon = (HICON)::LoadImage(g_hInstance, MAKEINTRESOURCE(IDI_MAIN), 
		IMAGE_ICON, ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON), LR_DEFAULTCOLOR);
	SetIcon(hIcon, TRUE);
	HICON hIconSmall = (HICON)::LoadImage(g_hInstance, MAKEINTRESOURCE(IDI_MAIN), 
		IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);
	SetIcon(hIconSmall, FALSE);

	// Set whole windows position
	SetWindowPos(HWND_TOP, 0, 0, WINCX, WINCY, SWP_NOMOVE);
	GetClientRect(&rcWinClient);
	rcUserClient.top   = rcUserClient.left = MYMARGIN;
	rcUserClient.right = rcWinClient.right - MYMARGIN;
	rcUserClient.bottom= rcWinClient.bottom - MYMARGIN;
	nCY = rcUserClient.top;

	// Set main icon and text
	::SetWindowPos(::GetDlgItem(m_hWnd, IDC_EXTMAIL), HWND_TOP, rcUserClient.left, nCY, BIGICON, BIGICON, SWP_SHOWWINDOW);
	::SetWindowPos(::GetDlgItem(m_hWnd, IDC_NOTIFY), HWND_TOP, rcUserClient.left+BIGICON+MYMARGIN/2, nCY, WIDTH(rcUserClient)-(BIGICON+MYMARGIN/2), BIGICON, SWP_SHOWWINDOW);
	::SetWindowTextW(::GetDlgItem(m_hWnd, IDC_NOTIFY), TAG_DLG_INFO);
	nCY += BIGICON;

	// Set Horz Etched Line, Multiple Client, Multiple User
	::SetWindowPos(::GetDlgItem(m_hWnd, IDC_HORZETCHED), HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_HIDEWINDOW);
	::SetWindowPos(::GetDlgItem(m_hWnd, IDC_MULTICLIENT), HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_HIDEWINDOW);
	::SetWindowPos(::GetDlgItem(m_hWnd, IDC_MULTICLIENTDESC), HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_HIDEWINDOW);
	::SetWindowPos(::GetDlgItem(m_hWnd, IDC_MULTIUSER), HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_HIDEWINDOW);
	::SetWindowPos(::GetDlgItem(m_hWnd, IDC_MULTIUSERDESC), HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_HIDEWINDOW);

	// Set View
	nCY += LINESPACE*2;
	CViewItemDlg::m_nX     = MYMARGIN;
	CViewItemDlg::m_nWidth = WIDTH(rcUserClient) - MYMARGIN -2 - ::GetSystemMetrics(SM_CXVSCROLL);
	m_viewDlg.Create(m_hWnd, 0);
	m_viewDlg.SetWindowPos(HWND_TOP, rcUserClient.left, nCY, WIDTH(rcUserClient), HEIGHT(rcUserClient)-nCY-BUTTONCY-5, SWP_SHOWWINDOW);

	// Set Button
	nCY = rcUserClient.bottom-BUTTONCY;
	::SetWindowPos(::GetDlgItem(m_hWnd, IDOK), HWND_TOP, rcUserClient.right-BUTTONCX*2-10, nCY, BUTTONCX, BUTTONCY, SWP_SHOWWINDOW);
	if(!m_bLast) ::SetWindowTextW(::GetDlgItem(m_hWnd, IDOK), L"Next");
	//::EnableWindow(::GetDlgItem(m_hWnd, IDOK), FALSE);
	::SetWindowPos(::GetDlgItem(m_hWnd, IDCANCEL), HWND_TOP, rcUserClient.right-BUTTONCX, nCY, BUTTONCX, BUTTONCY, SWP_SHOWWINDOW);
}






/*
Added for the Mail Attribute Parsing
*/

CMAttrParAssistDlg::CMAttrParAssistDlg() : CAssistantDialog()
{

}
CMAttrParAssistDlg::~CMAttrParAssistDlg()
{	
}
void CMAttrParAssistDlg::SetMAttrParType( const BOOL& bIsBlock ,const STRINGLIST& listRecipients)
{
	m_IsBlock = bIsBlock ;
	STRINGLIST templistRecipients = listRecipients ;
	for( STRINGLIST::iterator itor = templistRecipients.begin() ; itor != templistRecipients.end(); ++itor )
	{
		AddDmItemData( (*itor).c_str() ) ;
	}
}
void CMAttrParAssistDlg::SetMAttrParMessage( const std::wstring strMsgType, const std::wstring strMsg ) 
{
	m_MessageInfo = strMsg ;
	m_strmsgType = strMsgType  ;
}
void CMAttrParAssistDlg::AddDmItemData( LPCWSTR pwzRecipient/*, STRINGPAIRVECTOR& pairString*/)
{
	int i = (int)m_mapData.size();
	m_mapData.push_back(CMapData());
	m_mapData[i].strRecipient= pwzRecipient;
	//m_mapData[i].attachInfo = pairString;
}

void CMAttrParAssistDlg::AddViewData()
{
	//CMAttrParData MAttrParData;
	for (std::vector<CMapData>::iterator it=m_mapData.begin(); it!=m_mapData.end(); ++it)
	{
		m_viewDlg.AddItem(*it);
	}
	m_viewDlg.ResetView();
}
void CMAttrParAssistDlg::ReallocWindows()
{
	RECT rcWinClient, rcUserClient;
	int  nCY = 0;

	// Set Icon
	HICON hIcon = (HICON)::LoadImage(g_hInstance, MAKEINTRESOURCE(IDI_MAIN), 
		IMAGE_ICON, ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON), LR_DEFAULTCOLOR);
	SetIcon(hIcon, TRUE);
	HICON hIconSmall = (HICON)::LoadImage(g_hInstance, MAKEINTRESOURCE(IDI_MAIN), 
		IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);
	SetIcon(hIconSmall, FALSE);

	// Set whole windows position
	SetWindowPos(HWND_TOP, 0, 0, WINCX, WINCY, SWP_NOMOVE);
	GetClientRect(&rcWinClient);
	rcUserClient.top   = rcUserClient.left = MYMARGIN;
	rcUserClient.right = rcWinClient.right - MYMARGIN;
	rcUserClient.bottom= rcWinClient.bottom - MYMARGIN;
	nCY = rcUserClient.top;

	// Set main icon and text
	::SetWindowPos(::GetDlgItem(m_hWnd, IDC_EXTMAIL), HWND_TOP, rcUserClient.left, nCY, BIGICON, BIGICON, SWP_SHOWWINDOW);
	std::wstring strTempMsg = m_MessageInfo ;
	if( !m_strmsgType.empty() )
	{
		strTempMsg = m_strmsgType + L": " +m_MessageInfo ;
	}
	HWND hInfoWnd = ::GetDlgItem(m_hWnd, IDC_NOTIFY) ;
	::SendMessage(hInfoWnd, WM_SETFONT, (WPARAM)g_fntNormal, (LPARAM)TRUE);
	HDC hMsgDc = ::GetDC( hInfoWnd) ;
	SIZE sizeStr ;
	INT iHeight = BIGICON ;
	if( GetTextExtentPoint32( hMsgDc,strTempMsg.c_str(),(INT)strTempMsg.length(),&sizeStr) )
	{
		UINT icount = sizeStr.cx/WIDTH(rcUserClient) ;
		if( icount>=2)
		{
			UINT iLen = (UINT)strTempMsg.length()/(icount-1) -2;

			//	BOOL bExtend = FALSE ; 

			if( m_MessageInfo.find( L" ")==std::string::npos )
			{
				UINT i=0  ;
				for(  ; i< icount ;  )
				{
					if( iLen*(i+1)<strTempMsg.length() )
					{
						strTempMsg.insert( iLen*(i+1) , L" " ) ;
					}
					i++ ;
				}
			}
			iHeight = sizeStr.cy*icount ;
		}
		DP((L"The string size compute result:width[%d],Height[%d],count[%d]",sizeStr.cx,iHeight,icount));

	}
	::SetWindowPos(hInfoWnd, HWND_TOP, rcUserClient.left+BIGICON+MYMARGIN, nCY, WIDTH(rcUserClient)-(BIGICON+MYMARGIN/2), iHeight, SWP_SHOWWINDOW);
	::SetWindowTextW(hInfoWnd, strTempMsg.c_str());

	nCY += iHeight;


	// Set Horz Etched Line
	//nCY += LINESPACE;
	::SetWindowPos(::GetDlgItem(m_hWnd, IDC_HORZETCHED), HWND_TOP, rcUserClient.left, nCY, WIDTH(rcUserClient), LINESPACE, SWP_SHOWWINDOW);
	//nCY += LINESPACE;
	// Set Multiple Client
	nCY += LINESPACE;
	::SetWindowPos(::GetDlgItem(m_hWnd, IDC_MULTICLIENT), HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_HIDEWINDOW);
	::SetWindowPos(::GetDlgItem(m_hWnd, IDC_MULTICLIENTDESC), HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_HIDEWINDOW);
	// Set Multiple User
	::SetWindowPos(::GetDlgItem(m_hWnd, IDC_MULTIUSER), HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_HIDEWINDOW);
	::SetWindowPos(::GetDlgItem(m_hWnd, IDC_MULTIUSERDESC), HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_HIDEWINDOW);


	// Set View
	nCY += LINESPACE*2;
	CViewItemDlg::m_nX     = MYMARGIN;
	CViewItemDlg::m_nWidth = WIDTH(rcUserClient) - MYMARGIN -2 - ::GetSystemMetrics(SM_CXVSCROLL);
	m_viewDlg.Create(m_hWnd, 0);
	m_viewDlg.SetWindowPos(HWND_TOP, rcUserClient.left, nCY, WIDTH(rcUserClient), HEIGHT(rcUserClient)-nCY-BUTTONCY-5, SWP_SHOWWINDOW);


	// Set Button
	nCY = rcUserClient.bottom-BUTTONCY;        
	::SetWindowPos(::GetDlgItem(m_hWnd, IDOK), HWND_TOP, rcUserClient.left+(WIDTH(rcUserClient)-BUTTONCX)/2, nCY, BUTTONCX, BUTTONCY, SWP_SHOWWINDOW);
	if((!m_IsBlock)&&(!m_bLast)) ::SetWindowTextW(::GetDlgItem(m_hWnd, IDOK), L"Send");
	else ::SetWindowTextW(::GetDlgItem(m_hWnd, IDOK), L"OK");
	if( !m_IsBlock)
	{
		::SetWindowPos(::GetDlgItem(m_hWnd, IDCANCEL), HWND_TOP, rcUserClient.right-BUTTONCX, nCY, BUTTONCX, BUTTONCY, SWP_SHOWWINDOW);
	}
}


CInterAssistDlg::CInterAssistDlg() : CAssistantDialog()
{
	m_Files.clear();
}
CInterAssistDlg::~CInterAssistDlg()
{
	m_Files.clear();
}
void CInterAssistDlg::AddInterItemData(LPCWSTR pwzAttach)
{
	m_Files.push_back(std::wstring(pwzAttach));
}


void CInterAssistDlg::AddViewData()
{
	CInterData interData;
	for (STRINGVECTOR::iterator it=m_Files.begin(); it!=m_Files.end(); ++it)
	{
		interData.strFile = *it;
		m_viewDlg.AddItem(interData);
	}
}
void CInterAssistDlg::ReallocWindows()
{
	RECT rcWinClient, rcUserClient;
	int  nCY = 0;

	// Set Icon
	HICON hIcon = (HICON)::LoadImage(g_hInstance, MAKEINTRESOURCE(IDI_MAIN), 
		IMAGE_ICON, ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON), LR_DEFAULTCOLOR);
	SetIcon(hIcon, TRUE);
	HICON hIconSmall = (HICON)::LoadImage(g_hInstance, MAKEINTRESOURCE(IDI_MAIN), 
		IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);
	SetIcon(hIconSmall, FALSE);

	// Set whole windows position
	SetWindowPos(HWND_TOP, 0, 0, WINCX, WINCY, SWP_NOMOVE);
	GetClientRect(&rcWinClient);
	rcUserClient.top   = rcUserClient.left = MYMARGIN;
	rcUserClient.right = rcWinClient.right - MYMARGIN;
	rcUserClient.bottom= rcWinClient.bottom - MYMARGIN;
	nCY = rcUserClient.top;

	// Set main icon and text
	::SetWindowPos(::GetDlgItem(m_hWnd, IDC_EXTMAIL), HWND_TOP, rcUserClient.left, nCY, BIGICON, BIGICON, SWP_SHOWWINDOW);
	::SetWindowPos(::GetDlgItem(m_hWnd, IDC_NOTIFY), HWND_TOP, rcUserClient.left+BIGICON+MYMARGIN/2, nCY, WIDTH(rcUserClient)-(BIGICON+MYMARGIN/2), BIGICON, SWP_SHOWWINDOW);
	::SetWindowTextW(::GetDlgItem(m_hWnd, IDC_NOTIFY), INTER_DLG_INFO);
	nCY += BIGICON;

	// Set Horz Etched Line, Multiple Client, Multiple User
	::SetWindowPos(::GetDlgItem(m_hWnd, IDC_HORZETCHED), HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_HIDEWINDOW);
	::SetWindowPos(::GetDlgItem(m_hWnd, IDC_MULTICLIENT), HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_HIDEWINDOW);
	::SetWindowPos(::GetDlgItem(m_hWnd, IDC_MULTICLIENTDESC), HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_HIDEWINDOW);
	::SetWindowPos(::GetDlgItem(m_hWnd, IDC_MULTIUSER), HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_HIDEWINDOW);
	::SetWindowPos(::GetDlgItem(m_hWnd, IDC_MULTIUSERDESC), HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_HIDEWINDOW);

	// Set View
	nCY += LINESPACE*2;
	CViewItemDlg::m_nX     = MYMARGIN;
	CViewItemDlg::m_nWidth = WIDTH(rcUserClient) - MYMARGIN -2 - ::GetSystemMetrics(SM_CXVSCROLL);
	m_viewDlg.Create(m_hWnd, 0);
	m_viewDlg.SetWindowPos(HWND_TOP, rcUserClient.left, nCY, WIDTH(rcUserClient), HEIGHT(rcUserClient)-nCY-BUTTONCY-5, SWP_SHOWWINDOW);

	// Set Button
	nCY = rcUserClient.bottom-BUTTONCY;        
	::SetWindowPos(::GetDlgItem(m_hWnd, IDOK), HWND_TOP, rcUserClient.left+(WIDTH(rcUserClient)-BUTTONCX)/2, nCY, BUTTONCX, BUTTONCY, SWP_SHOWWINDOW);
	if(!m_bLast) ::SetWindowTextW(::GetDlgItem(m_hWnd, IDOK), L"Next");
	else ::SetWindowTextW(::GetDlgItem(m_hWnd, IDOK), L"OK");
}

//////////////////////////////////////////////////////////////////////////

BOOL IsLocalDrive(LPCWSTR wzPath)
{
	if(wcslen(wzPath) < 2)
		return FALSE;
	if(L':'!=wzPath[1] || wzPath[0] < L'A' ||  (wzPath[0] > L'Z' && wzPath[0] < L'a') || wzPath[0] > L'z')
		return FALSE;

	return TRUE;
}

static BOOL GetRemoteCanonicalName(LPCWSTR wzHost, LPWSTR wzCanonName, int nSize)
{
    UNREFERENCED_PARAMETER(wzHost);
    UNREFERENCED_PARAMETER(wzCanonName);
    UNREFERENCED_PARAMETER(nSize);
	return TRUE;
}

#define NETMAP_HEAD	L"\\Device\\LanmanRedirector\\;"
BOOL IsMapped(LPCWSTR wzPath, LPWSTR wzRealPath, int cch)
{
	BOOL bIsMapped = FALSE;
	if(IsLocalDrive(wzPath))
	{
		WCHAR wzDriver[3]; wzDriver[0] = wzPath[0]; wzDriver[1]=L':'; wzDriver[2] = 0;
		WCHAR wzRealDriver[MAX_PATH+1];	memset(wzRealDriver, 0, sizeof(wzRealDriver));
		if(0 < QueryDosDeviceW(wzDriver, wzRealDriver, MAX_PATH))
		{
			if(0 == wcsncmp(wzRealDriver, L"\\??\\", 4))
			{
				bIsMapped = TRUE;
				wcsncpy_s(wzRealPath, cch, wzRealDriver+4, _TRUNCATE);
				wcsncat_s(wzRealPath, cch, wzPath+2, _TRUNCATE);
			}
			else if(0 == _wcsnicmp(wzRealDriver, NETMAP_HEAD, (int)wcslen(NETMAP_HEAD)))
			{
				bIsMapped = TRUE;
				WCHAR* pStart = wcsstr(wzRealDriver+(int)wcslen(NETMAP_HEAD), L"\\");
				if(pStart)//((int)wcslen(wzRealDriver) >  (int)wcslen(NETMAP_HEAD)+18)
				{
					wcsncpy_s(wzRealPath, cch, L"\\", _TRUNCATE);
					wcsncpy_s(wzRealPath+1, cch-1, pStart, _TRUNCATE);//wzRealDriver+(int)wcslen(NETMAP_HEAD)+18, cch);

					// Now we get the remote path here
					wcsncat_s(wzRealPath, cch, wzPath+2, _TRUNCATE);
				}
				else
				{
					wcsncpy_s(wzRealPath, cch, wzPath, _TRUNCATE);
				}

			}
		}
	}

	return bIsMapped;
}

void GetRealPath(LPCWSTR wzPath, LPWSTR wzRealPath, int cch)
{
    WCHAR wzTempPath[MAX_PATH+1] = {0};

	wcsncpy_s(wzTempPath, MAX_PATH, wzPath, _TRUNCATE);
	while(IsMapped(wzTempPath, wzRealPath, cch))
	{
		wcsncpy_s(wzTempPath, MAX_PATH, wzRealPath, _TRUNCATE);
	}
	wcsncpy_s(wzRealPath, cch, wzTempPath, _TRUNCATE);
}

static BOOL GetFQDNPath(LPCWSTR src, LPWSTR fqdnpath, int nSize)
{
    WCHAR   wzHostName[MAX_PATH+1] = {0};
    WCHAR   wzFqdnName[MAX_PATH+1] = {0};

	if(2 >= wcslen(src))
		return FALSE;
	if(L'\\'!=src[0] || L'\\'!=src[1])
		return FALSE;

	wcsncpy_s(wzHostName, MAX_PATH, (src+2), _TRUNCATE);
	WCHAR* pHostEnd = wcsstr(wzHostName, L"\\");
	const WCHAR* pHostEnd2= wcsstr((src+2), L"\\");
	if(pHostEnd) *pHostEnd = 0;

	OLUtilities::GetFQDN(wzHostName, wzFqdnName, MAX_PATH);
	wcsncpy_s(fqdnpath, nSize, L"\\\\", _TRUNCATE); 
	wcsncat_s(fqdnpath, nSize, wzFqdnName, _TRUNCATE); 
	if(pHostEnd2)
	{
		wcsncat_s(fqdnpath, nSize, pHostEnd2, _TRUNCATE);
	}

	return TRUE;
}
/************************************************************************/
/*                                                                      */
/************************************************************************/
// CItemEventDisp
STDMETHODIMP CItemEventDisp::Invoke(DISPID dispidMember,
									REFIID riid,
									LCID lcid,
									WORD wFlags,
									DISPPARAMS* pdispparams,
									VARIANT* pvarResult,
									EXCEPINFO* pexcepinfo,
									UINT* puArgErr)
{
    UNREFERENCED_PARAMETER(puArgErr);
    UNREFERENCED_PARAMETER(pexcepinfo);
    UNREFERENCED_PARAMETER(pvarResult);
    UNREFERENCED_PARAMETER(lcid);
    UNREFERENCED_PARAMETER(riid);
	HRESULT hr = S_OK;

    __try
    {
		if	(pdispparams && DISPATCH_METHOD==wFlags)
		{
			switch	(dispidMember)
			{
			case 0xfbaf:
				if ((this->m_nEventNeedSink & PRVIEW_EVENT) && pdispparams->rgvarg[0].pboolVal != NULL &&  pdispparams->rgvarg[1].pdispVal != NULL)
				{
					OnPreviewAttachment(pdispparams->rgvarg[1].pdispVal,pdispparams->rgvarg[0].pboolVal);
				}
				else
				{
					logd(L"return the uncare Preview event!");
				}
				break;
			case 0xf002:    // Write
				
				if((this->m_nEventNeedSink & WRITE_EVENT) && (pdispparams->cArgs>=1)&& (pdispparams->rgvarg[0].vt==(VT_BOOL+VT_BYREF))
					&& (pdispparams->rgvarg[0].pboolVal!=NULL) )
				{
					OnWrite(pdispparams->rgvarg[0].pboolVal);
				}
				else 
				{
					logd(L"return the uncare Write event!");
				}
				break;
			case 0xf005:    // Send
				
				if((this->m_nEventNeedSink & SEND_EVENT) && (pdispparams->cArgs>=1)&& (pdispparams->rgvarg[0].vt==(VT_BOOL+VT_BYREF))
					&& (pdispparams->rgvarg[0].pboolVal!=NULL) )
				{
                    logd(L"======>onsend trggiered!");
					OnSendEx(pdispparams->rgvarg[0].pboolVal);
				}
				else
				{
					logd(L"return the uncare send event!");
				}
				break;

			case 0x0000f004: //close
				
				if((this->m_nEventNeedSink & CLOSE_EVENT) && (pdispparams->cArgs>=1)&& (pdispparams->rgvarg[0].vt==(VT_BOOL+VT_BYREF))
					&& (pdispparams->rgvarg[0].pboolVal!=NULL) )
				{
					OnClose(pdispparams->rgvarg[0].pboolVal);
				}
				else
				{
					logd(L"return the uncare close event!");
				}
				break;

			case 0x0000f466: //Reply
			case 0x0000f467: //ReplyAll
			case 0x0000f468: //Forward
				if((this->m_nEventNeedSink & REPLAY_EVENT) && (pdispparams->cArgs>=2) //!!!! The parameter order is reversed index
					&& (pdispparams->rgvarg[1].vt==VT_DISPATCH) && (pdispparams->rgvarg[1].pdispVal!=NULL)
					&& (pdispparams->rgvarg[0].vt==(VT_BOOL+VT_BYREF)) && (pdispparams->rgvarg[0].pboolVal!=NULL) )
				{
					logd(L"Reply, ReplyAll or Forward\n");
					logd(L"CopyMessageHeaderTo(%p)\n", pdispparams->rgvarg[1].pdispVal);
				
					CopyMessageHeaderTo(pdispparams->rgvarg[1].pdispVal);
				}
				else
				{
					logd(L"return the uncare Reply/Forward event!");
				}
				break;

			default:
				//if (!this->m_bNeedToSink) return hr;
				hr = DISP_E_MEMBERNOTFOUND;
				break;
			}
		}
		else
		{
			hr = DISP_E_PARAMNOTFOUND;
		}
	}
	__except(1 )
	{
		/* empty */
        ;
	}

	return hr;
}

static BOOL MakeAttachTempPath(int nIndex, LPCWSTR wzAttachName, std::wstring& strTempPath)
{
	SYSTEMTIME	sysTime;	memset(&sysTime, 0, sizeof(SYSTEMTIME));
	GetLocalTime(&sysTime);
	int nLen = 0;

	WCHAR wzTempAttachmentPath[MAX_PATH];	memset(wzTempAttachmentPath, 0, sizeof(wzTempAttachmentPath));
	WCHAR wzTempAttachmentPath2[MAX_PATH];	memset(wzTempAttachmentPath2, 0, sizeof(wzTempAttachmentPath2));

	// Get folder path
	_snwprintf_s(wzTempAttachmentPath, MAX_PATH, _TRUNCATE, L"%s%04d%02d%02d_%02d%02d%02d_%d\\",
		g_strOETempFolder.c_str(),
		sysTime.wYear, sysTime.wMonth, sysTime.wDay,
		sysTime.wHour, sysTime.wMinute, sysTime.wSecond, sysTime.wMilliseconds);
	if( !CreateDirectoryW(wzTempAttachmentPath, NULL) && ERROR_ALREADY_EXISTS!=GetLastError() )
		return FALSE;

	nLen = (int)wcslen(wzTempAttachmentPath);
	if(nLen>0 && L'\\'!=wzTempAttachmentPath[nLen-1])
		wcsncat_s(wzTempAttachmentPath, MAX_PATH, L"\\", _TRUNCATE);
	_snwprintf_s(wzTempAttachmentPath2, MAX_PATH, _TRUNCATE, L"%s%d", wzTempAttachmentPath, nIndex);
	if( !CreateDirectoryW(wzTempAttachmentPath2, NULL) && ERROR_ALREADY_EXISTS!=GetLastError() )
		return FALSE;

	strTempPath = wzTempAttachmentPath2;
	strTempPath.append(L"\\");
	strTempPath.append(wzAttachName);
	return TRUE;
}

void CItemEventDisp::ReAddAttachmentForForwardMail()
{
# if 0
	CComPtr<Outlook::Attachments>       spAttachments = NULL;
	std::vector<std::wstring>   listTempFile;

	// process all attachments
	HRESULT hr = m_spMailItem->get_Attachments(&spAttachments);
	if(SUCCEEDED(hr) && spAttachments)
	{
		long                    lCurIndex    = 0;
		long					lAttachCount = 0;
		hr = spAttachments->get_Count(&lAttachCount);
		if(SUCCEEDED(hr) && lAttachCount)
		{
			for (lCurIndex=lAttachCount; lCurIndex>0; lCurIndex--)
			{
				CComPtr<Outlook::Attachment>	spAttachment = NULL;
				CComBSTR                bstrFileName;
				std::wstring            strFileName;
				CComBSTR				bstrDispName;
				std::wstring            strDispName;
				std::wstring            strTempFile;
				CComBSTR                varTempFile;

				// check each attachment
				hr = spAttachments->Item(CComVariant(lCurIndex), &spAttachment);
				if(FAILED(spAttachment) || NULL==spAttachment)
					continue;
				hr=spAttachment->get_DisplayName(&bstrDispName);
				if(SUCCEEDED(hr))
				{
					strDispName = (NULL==bstrDispName.m_str)?L"":bstrDispName.m_str;
					const wchar_t* pTempMagic=wcsstr(strDispName.c_str(), TEMP_MAGIC_NAME);
					if(pTempMagic&&(IsLocalDrive(pTempMagic)||(L'\\'!=pTempMagic[1])))
					{
						spAttachment->Release();
						continue;
					}
				}
				else
				{
					DP((L"CItemEventDisp::ReAddAttachmentForForwardMail! Fail to get attachment display name (index=%d)\n", lCurIndex));
					goto ERROR_CONTINUE_LOOP;
				}
				hr = spAttachment->get_FileName(&bstrFileName);
				if(SUCCEEDED(hr))
				{
					strFileName = (NULL==bstrFileName.m_str)?L"":bstrFileName.m_str;
				}
				else
				{
					DP((L"CItemEventDisp::ReAddAttachmentForForwardMail! Fail to get attachment file name (index=%d)\n", lCurIndex));
					goto ERROR_CONTINUE_LOOP;
				}
				/*
				removed by chellee on 04.09.2008;9:29, for the encryption...
				*/
				//--------------------------------------------------------------------------------
				/*	if(!OLUtilities::IsWordFile(bstrFileName)
				&& !OLUtilities::IsExcelFile(bstrFileName)
				&& !OLUtilities::IsPwptFile(bstrFileName)
				&& !OLUtilities::IsRTFFile(bstrFileName)
				)
				{
				DP((L"This is not a office file: %s\n", bstrFileName));
				goto ERROR_CONTINUE_LOOP;
				}*/
				//--------------------------------------------------------------------------------
				// make the temp file path and create the folder to store the temp file
				if(!MakeAttachTempPath((int)lCurIndex, strFileName.c_str(), strTempFile))
					goto ERROR_CONTINUE_LOOP;

				// Save the file
				varTempFile = strTempFile.c_str();
				hr = spAttachment->SaveAsFile(varTempFile);
				if (FAILED(hr))
				{
					DP((L"Fail save to the temp file: %s\n", strTempFile.c_str()));
					goto ERROR_CONTINUE_LOOP;
				}
				DP((L"Save to the temp file: %s\n", strTempFile.c_str()));

				// Delete this attachment from Attachments
				hr = spAttachment->Delete();
				if (FAILED(hr))
				{
					DP((L"Fail delete the attachment(%d): %s\n", lCurIndex, strFileName.c_str()));
					goto ERROR_CONTINUE_LOOP;
				}
				hr=m_spMailItem->Save();
				if (FAILED(hr))
				{
					DP((L"Fail save mail item the attachment(%d): %s\n", lCurIndex, strFileName.c_str()));
					goto ERROR_CONTINUE_LOOP;
				}
				DP((L"Delete the attachment(%d): %s\n", lCurIndex, strFileName.c_str()));
				spAttachment->Release();
				spAttachment = NULL;

				// the file is deleted
				listTempFile.push_back(strTempFile);

ERROR_CONTINUE_LOOP:
				if(spAttachment) spAttachment->Release();
			}

			// If there is any file to be re-added, do it
			for (std::vector<std::wstring>::iterator it=listTempFile.begin(); it!=listTempFile.end(); ++it)
			{
				CComVariant varSource((*it).c_str());
				CComVariant varOptional((long)DISP_E_PARAMNOTFOUND, VT_ERROR);
				CComPtr<Outlook::Attachment>	spNewAttach = 0;
				// Re-attach the attachment
				hr = spAttachments->Add(varSource, varOptional, varOptional, varOptional, &spNewAttach);
				if(SUCCEEDED(hr)&&spNewAttach)
				{
					hr=m_spMailItem->Save();
					spNewAttach->Release();
				}
				DP((L"Add the attachment: %s\n", (*it).c_str()));
			}

			listTempFile.clear();
		}
		spAttachments->Release();
		spAttachments = NULL;
	}
#else if
	OLUtilities::CheckReattach(m_spMailItem, FALSE, !m_bLaunchedBy3rdPart);
#endif
}

static void RemoveAllAttachment(CComPtr<Outlook::Attachments> spAttachments)
{
	long                    lCurIndex    = 0;
    long					lAttachCount = 0;
	HRESULT hr = spAttachments->get_Count(&lAttachCount);
	if(SUCCEEDED(hr) && lAttachCount)
	{
		for (lCurIndex=lAttachCount; lCurIndex>0; lCurIndex--)
		{
			CComPtr<Outlook::Attachment>	spAttachment = NULL;
			hr = spAttachments->Item(CComVariant(lCurIndex), &spAttachment);
			if(FAILED(hr) || NULL==spAttachment)
				continue;
			spAttachment->Delete();
		}
	}
}
void GetFilePathFromROT(const wstring& strFileName,wstring& strFilePath)
{
    IRunningObjectTable* lpROT = NULL;
    IEnumMoniker* pEnum = NULL;
    IMalloc* pMalloc = NULL;
    BOOL bFind = FALSE;

    std::vector<std::wstring> vecRotFile;

    HRESULT hr = GetRunningObjectTable(0, &lpROT);
    BOOL bInitCom = FALSE;
    if (hr == CO_E_NOTINITIALIZED)
    {
        CoInitialize(NULL);
        bInitCom = TRUE;
        hr = GetRunningObjectTable(0, &lpROT);
    }

    if (FAILED(hr))
    {
        goto _EXIT_GETFILEPATH_FROM_ROT_;
    }

    hr = lpROT->EnumRunning(&pEnum);
    if (FAILED(hr))
    {
        goto _EXIT_GETFILEPATH_FROM_ROT_;
    }


    hr = CoGetMalloc(1, &pMalloc);
    if (FAILED(hr))
    {
        goto _EXIT_GETFILEPATH_FROM_ROT_;
    }

    IMoniker* pMoniker = NULL;
    ULONG lFetch = 0;

    IBindCtx* pCtx = NULL;
    hr = CreateBindCtx(0, &pCtx);
    if (FAILED(hr))
    {
        goto _EXIT_GETFILEPATH_FROM_ROT_;
    }

    LPOLESTR pDisplayName = NULL;
    while ((hr = pEnum->Next(1, &pMoniker, &lFetch)) == S_OK && !bFind)
    {
        hr = pMoniker->GetDisplayName(pCtx, NULL, &pDisplayName);
        if (SUCCEEDED(hr) && pDisplayName != NULL)
        {
            wstring strDisplayName(pDisplayName);
            pMalloc->Free(pDisplayName);
            if (boost::algorithm::iends_with(strDisplayName, strFileName))
            {
                strFilePath = strDisplayName;
                break;
            }
        }
        pMoniker->Release();
    }
    pCtx->Release();
_EXIT_GETFILEPATH_FROM_ROT_:
    if (pMalloc)
    {
        pMalloc->Release();
    }
    if (pEnum)
    {
        pEnum->Release();
    }
    if (lpROT)
    {
        lpROT->Release();
    }

}
void CItemEventDisp::GetAttachmentSourcePathFor3rdPartSend(LPCWSTR pwzSubject,BOOL bNeedSave)
{
    UNREFERENCED_PARAMETER(bNeedSave);
    bool bFind = false;
	CComPtr<Outlook::Attachments>       spAttachments = NULL;
	KeyWords                    theNode;	
	theNode.strSubject=pwzSubject;
	std::vector<FilePair>		listTempFile;
    wstring strAttachmentFileName = L"";

	DP((L"GetAttachmentSourcePathFor3rdPartSend:: Subject=%s\n", pwzSubject));
	// process all attachments
	HRESULT hr =MailItemUtility::get_Attachments(m_spMailItem,&spAttachments,TRUE);
	if(SUCCEEDED(hr) && spAttachments)
	{
		long                    lCurIndex    = 0;
		long					lAttachCount = 0;
		hr = spAttachments->get_Count(&lAttachCount);
		if(SUCCEEDED(hr) && lAttachCount)
		{
			theNode.uAttachmentCount=lAttachCount;
			for (lCurIndex=lAttachCount; lCurIndex>0; lCurIndex--)
			{
				CComPtr<Outlook::Attachment>	spAttachment = NULL;
				std::wstring            strTempFile;
				CComBSTR                varTempFile;

				// check each attachment
				hr = spAttachments->Item(CComVariant(lCurIndex), &spAttachment);
				if(FAILED(hr) || NULL==spAttachment)
					continue;

                std::wstring strFileName = OLUtilities::GetAttachFileName(spAttachment);
				if(strFileName.empty())
					continue;
                strAttachmentFileName = strFileName;

				theNode.vecFiles.push_back(FilePair(strFileName.c_str(),L""));
				DP((L"GetAttachmentSourcePathFor3rdPartSend:: Find attachment %s\n", strFileName.c_str()));
			}

			// Find all file path in service
			bool bTrue = CTransferInfo::get_FileInfo(theNode.strSubject.c_str(),theNode.uAttachmentCount,theNode.vecFiles);
			if(bTrue)
			{
				for (lCurIndex=lAttachCount; lCurIndex>0; lCurIndex--)
				{
					CComPtr<Outlook::Attachment>	spAttachment;
					CComBSTR                bstrFileName;
					CComBSTR				bstrDispName;
					std::wstring            strTempFile;
 					CComBSTR                varTempFile;

					// check each attachment
					hr = spAttachments->Item(CComVariant(lCurIndex), &spAttachment);
					if(FAILED(hr) || NULL==spAttachment.p)
						continue;

					hr = spAttachment->get_FileName(&bstrFileName);
					if(FAILED(hr) || NULL==bstrFileName.m_str)
                        continue;

					//get the source path and set the source path info to display name
					for (std::vector<FilePair>::iterator itInternal=theNode.vecFiles.begin(); itInternal!=theNode.vecFiles.end(); ++itInternal)
					{
						if (_wcsicmp((*itInternal).first.c_str(), bstrFileName)==0)
						{
							m_map3thAppAttachmentSourcePath[lCurIndex] = (*itInternal).second;
                            bFind = true;
							DP((L"Added source path for 3th app. index=%d, source=%s.", lCurIndex, (*itInternal).second.c_str())); 
						}
					}
				}
			}
#ifdef WSO2K16 
            if (!bFind)
            {
                wstring strPath = L"";
                GetFilePathFromROT(strAttachmentFileName, strPath);
                if (!strPath.empty())
                {
                    m_map3thAppAttachmentSourcePath[1] = strPath;
                }
            }
#endif
		}
	}
}
				

void CItemEventDisp::OnOpen()//VARIANT_BOOL* Cancel)
{
	WCHAR wzSenderAddr[MAX_MAILADDR_LENGTH+1];
	WCHAR wzSubject[1024+1];
	int   nRecipients = 0;

	DP((L"CItemEventDisp::OnOpen\n"));
	memset(wzSenderAddr, 0, sizeof(wzSenderAddr));
	memset(wzSubject, 0, sizeof(wzSubject));

	nRecipients = OLUtilities::GetMailRecipients(m_spMailItem);
	OLUtilities::GetMailSubject(m_spMailItem, wzSubject, 1024);
	OLUtilities::GetSenderAddr(m_spMailItem, wzSenderAddr, MAX_MAILADDR_LENGTH);

	m_mtMailType = EXISTINGMAIL;

#if defined(WSO2K13) || defined(WSO2K16)
	std::wstring wstrCmd = GetCommandLineW();
	bool bLaunchByOfficeApp = false;

	CComBSTR bstrType;
	HRESULT hrType = MailItemUtility::get_SenderEmailType(m_spMailItem, &bstrType);

	std::wstring wstrType;
	if ((SUCCEEDED(hrType) && bstrType != NULL))
		wstrType = bstrType;
	if (boost::algorithm::iends_with(wstrCmd, L"-Embedding") //Excel
		||boost::algorithm::icontains(wstrCmd, L"simplemapi") || //ppt word, outlook not launch
		 0 ==  _wcsicmp(wstrType.c_str(), L"EX")) //ppt word, outlook has been launched
		bLaunchByOfficeApp = true;

	if(0 == wzSenderAddr[0] || bLaunchByOfficeApp)
	{
#else 
	if(0 == wzSenderAddr[0])
	{
#endif 
		if(0 == nRecipients && 0!=wzSubject[0])
		{
			if(0==_wcsnicmp(L"FW:", wzSubject, 3)||0==_wcsnicmp(L"转发", wzSubject, 2))
			{
				// Forward
				m_mtMailType = FWMAIL;
				DP((L"It is a forward mail!\n"));
			}
			else
			{
				// Other send
				m_mtMailType = OTHERSEND;
				DP((L"It is a Other send mail!\n"));
			}
		}
		else if(0 != nRecipients)
		{
			// reply
			m_mtMailType = REPLYMAIL;
			DP((L"It is a reply mail!\n"));
		}
		else
		{
			// modified by derek
			CComPtr<Attachments> spAttachments = NULL;
			long lAttachCount = 0;
			HRESULT hr = MailItemUtility::get_Attachments(m_spMailItem,&spAttachments,TRUE);
			if(SUCCEEDED(hr) && spAttachments)
			{
				hr = spAttachments->get_Count(&lAttachCount);
			}

			if (lAttachCount)
			{
				// Other send
				m_mtMailType = OTHERSEND;
				DP((L"It is a Other send mail!\n"));
			}
			else
			{
				// New Mail
				m_mtMailType = NEWMAIL;
				DP((L"It is a new mail!\n"));
			}
		}
	}
	else
	{
		m_mtMailType = EXISTINGMAIL;
		DP((L"It is a existing mail! Sender = %s\n", wzSenderAddr));
	}

	/*
	If the mail is a forward mail, we should check the attachments already exist.
	-- There is no OnAttachment event, so we must process the attachment here

	if(FWMAIL == m_mtMailType)
	ReAddAttachmentForForwardMail();*/

	if(OTHERSEND == m_mtMailType)
	{	
#if defined(WSO2K13) || defined(WSO2K16)
        // Delete SaveItem if 3rdPartSend and outlook Embedding.(like: explorer, excel...)
		if (boost::algorithm::iends_with((std::wstring)(GetCommandLineW()), L"-Embedding"))
        {
			SetLaunchedBy3rdPartFlag(TRUE);
        }
 
#endif
		GetAttachmentSourcePathFor3rdPartSend(wzSubject, !m_bLaunchedBy3rdPart);

		//re-attach, for third-part application send(include desktop->send to->Mail Recipient).
		//because this action may cause email attachment crash when we forward the email,sometimes.
		ReAttachAllAttachmentForOtherSend();
	}

}

void CItemEventDisp::ReAttachAllAttachmentForOtherSend()
{
	CComPtr<Outlook::Attachments> spAttachments = NULL;
	HRESULT hr =MailItemUtility::get_Attachments(m_spMailItem,&spAttachments,TRUE);
	if(SUCCEEDED(hr) && spAttachments)
	{
		long					lAttachCount = 0;
		hr = spAttachments->get_Count(&lAttachCount);
		for (long lCurIndex=lAttachCount; lCurIndex>0; lCurIndex--)
		{
			CComPtr<Outlook::Attachment>	spAttachment = NULL;
			CComVariant varIndex = lCurIndex;
			spAttachments->Item(varIndex, &spAttachment);

			if (spAttachment)
			{
				std::map<long,std::wstring>::const_iterator itAttachPath = m_map3thAppAttachmentSourcePath.find(lCurIndex);
				std::wstring strSouce= (itAttachPath!=m_map3thAppAttachmentSourcePath.end()) ? itAttachPath->second.c_str() : L"";

			    if(strSouce.empty()|| !PathFileExistsW(strSouce.c_str()) )
				{//save as a temp file
					CAttachmentFileMgr& theAttachmentFileMgr = CAttachmentFileMgr::GetInstance();
					std::wstring wstrRealSourceFileFullPath  = L"";
					std::wstring wstrOETempFileFullPath = L"";
					bool bRet = theAttachmentFileMgr.GetAttachmentFilePath(spAttachment, wstrRealSourceFileFullPath, wstrOETempFileFullPath, emAttachmentFromUnknown, MAIL_ITEM);
					if (bRet && (!wstrOETempFileFullPath.empty()))
					{
						strSouce = wstrOETempFileFullPath;
					}
				}

				//re-attach
				if ((!strSouce.empty()) && PathFileExistsW(strSouce.c_str()))
				{
					// Attach success ==> delete
					CComVariant varSource(strSouce.c_str());
					CComVariant varOptional((long)DISP_E_PARAMNOTFOUND, VT_ERROR);
					CComPtr<Outlook::Attachment>	spNewAttach = 0;
					hr = spAttachments->Add(varSource, varOptional, varOptional, varOptional, &spNewAttach);
					if (SUCCEEDED(hr) && spNewAttach)
					{
						spAttachment->Delete();
					}
				}
				DP((L"testx ReAttachAllAttachmentForOtherSend, index=%d, path=%s, hResult=0x%x\n", lCurIndex, strSouce.c_str(), hr));
			}			
		}
	}
}


void CItemEventDisp::OnCustomAction(CComPtr<IDispatch> Action, CComPtr<IDispatch> Response, VARIANT_BOOL* Cancel)
{
    UNREFERENCED_PARAMETER(Action);
    UNREFERENCED_PARAMETER(Response);
    UNREFERENCED_PARAMETER(Cancel);
}

void CItemEventDisp::OnCustomPropertyChange(BSTR Name)
{
    UNREFERENCED_PARAMETER(Name);
}

void CItemEventDisp::OnForward(CComPtr<IDispatch> Forward, VARIANT_BOOL* Cancel)
{
    UNREFERENCED_PARAMETER(Forward);
    UNREFERENCED_PARAMETER(Cancel);
}

void CItemEventDisp::OnClose(VARIANT_BOOL* Cancel)
{
	UNREFERENCED_PARAMETER(Cancel);
	//delete OE temp path cache
	CAttachmentFileMgr& refAttachFileMgr = CAttachmentFileMgr::GetInstance();
	std::list<std::wstring>::iterator itOETempPath = m_lstOETempFile.begin();
	while (itOETempPath != m_lstOETempFile.end())
	{
		refAttachFileMgr.DeleteFileFromCache(*itOETempPath);
		itOETempPath++;
	}

	//clear 3th app source path cache
	m_map3thAppAttachmentSourcePath.clear();
}

void CItemEventDisp::CollectAllOETempFileInfo(CSendEmailData& emailData)
{
    std::vector<CAttachmentData>& refVecAttachment = emailData.GetAttachmentData();
	std::vector<CAttachmentData>::iterator itAttach = refVecAttachment.begin();
	while (itAttach != refVecAttachment.end())
	{
		m_lstOETempFile.push_back(itAttach->GetOrgTempPath());
		itAttach++;
	}
}

void CItemEventDisp::OnPropertyChange(BSTR Name)
{
    UNREFERENCED_PARAMETER(Name);
}

void CItemEventDisp::OnRead()
{
}

void CItemEventDisp::OnReply(CComPtr<IDispatch> Response, VARIANT_BOOL* Cancel)
{
    UNREFERENCED_PARAMETER(Response);
    UNREFERENCED_PARAMETER(Cancel);
}

void CItemEventDisp::OnReplyAll(CComPtr<IDispatch> Response, VARIANT_BOOL* Cancel)
{
    UNREFERENCED_PARAMETER(Response);
    UNREFERENCED_PARAMETER(Cancel);
}

BOOL EndWith(std::wstring& strValue, const WCHAR wcValue)
{
	if(0 >= (int)strValue.length()) return FALSE;

	const WCHAR* lpwzValue = strValue.c_str();
	if(NULL == lpwzValue) return FALSE;

	if(wcValue == lpwzValue[strValue.length()-1]) return TRUE;

	return FALSE;
}

void AddFakeAttachmentIfNeeded(ATTACHMENTLIST& listAttachments,
									  int& nAttachments,
									  BOOL& bEmptyAttachment)
{
	// If there is no "non-ignored" attachment, add a fake one
	BOOL bNeedFake = FALSE;
	if(0 == nAttachments)
	{
		bNeedFake = TRUE;
	}
	else
	{
		unsigned i;
		for(i = 0; i < listAttachments.size(); i++)
		{
			LpAttachmentData lpAttachmentData = (LpAttachmentData)listAttachments[i];
			if(lpAttachmentData && !lpAttachmentData->bIgnored)
			{
				break;
			}
		}
		if(i >= listAttachments.size())
		{
			bNeedFake = TRUE;
		}
	}

	if(bNeedFake)
	{
		DP((L"Need to add a fake attachment.\n"));
		LpAttachmentData lpAttachmentData;

		bEmptyAttachment = TRUE;
		lpAttachmentData = new AttachmentData;
		if(NULL != lpAttachmentData)
		{
			memset(lpAttachmentData, 0, sizeof(AttachmentData));

			lpAttachmentData->type = attachOthers;

			wcsncpy_s(lpAttachmentData->dispname,MAX_PATH, L"No_attachment.ice", _TRUNCATE);

			wcsncpy_s(lpAttachmentData->src,MAX_SRC_PATH_LENGTH, L"C:\\No_attachment.ice", _TRUNCATE);

			wcsncpy_s(lpAttachmentData->resolved,MAX_SRC_PATH_LENGTH, L"C:\\No_attachment.ice", _TRUNCATE);

			wcsncpy_s(lpAttachmentData->temp,MAX_SRC_PATH_LENGTH, L"C:\\No_attachment.ice", _TRUNCATE);

			listAttachments.push_back(lpAttachmentData);
			nAttachments = (int)listAttachments.size();
		}
	}
}
void ResetDisplayNameForAttachments(BOOL bEmptyAttachment,int nAttachments,ATTACHMENTLIST&listAttachments,CComPtr<IDispatch> pMailItem )
{
	int i=0;
	for (i=0; i<nAttachments; i++)
	{
		if(bEmptyAttachment)
			break;
		CComPtr<Outlook::Attachment> pAttachment = NULL;
		BOOL                 bValidAttachment = FALSE;
		LpAttachmentData lpAttachmentData = (LpAttachmentData)listAttachments[i];
		bValidAttachment = OLUtilities::GetAttachmentByIndex(pMailItem, i, &pAttachment);

		if(!bValidAttachment || NULL==pAttachment)  // fail to get the attachment object, just skip it
			continue;
		if(0 != lpAttachmentData->dispname[0])
		{
			/*
				Modified by chellee on 21/10/08; 11:11 AM. For the .Msg File
			*/
			DP((L"Reset Display Name:Source File[%s],Temp:[%s],Display Name[%s]! \n",lpAttachmentData->src,lpAttachmentData->temp, lpAttachmentData->dispname));
			if(IsMsgFile(lpAttachmentData->src))
			{
				LPCWSTR pszFileName = wcsrchr(lpAttachmentData->src, L'\\');
				if(NULL != pszFileName)
				{	
					DP((L"Display name: %s\n", pszFileName+1));
					CComBSTR bstrDispName(pszFileName+1);
					pAttachment->put_DisplayName(bstrDispName);
				}
			}
			else
			{
				CComBSTR bstrDispName(lpAttachmentData->dispname);
				pAttachment->put_DisplayName(bstrDispName);
			}
		}
	}
}

BOOL NeedReattachForSendMenuFromOtherApplication(CComPtr<IDispatch> pMailItem)
{
  //If we have one single attachment and its name doesn't have full path,
  //the attachment will be saveed to disk and query based on saved file 
  //for checking tagging purpose
 BOOL bRet=FALSE;
 long lAttachCount=0;
 CComPtr<Outlook::Attachments> pAttachments=NULL;
 HRESULT hr=MailItemUtility::get_Attachments(pMailItem,&pAttachments,TRUE);
 if(SUCCEEDED(hr)&&pAttachments)
 {
   hr=pAttachments->get_Count(&lAttachCount);
  if(SUCCEEDED(hr))
  {
   if(lAttachCount==1)
   {
    CComPtr<Outlook::Attachment> pAttachment=NULL;
    hr=pAttachments->Item(CComVariant(1),&pAttachment);
    if(SUCCEEDED(hr)&&pAttachment)
    {
     CComBSTR bstrDispName;
     hr=pAttachment->get_DisplayName((&bstrDispName));
     if(SUCCEEDED(hr))
     {
		DP((L"CItemEventDisp::need 5 name=%s", bstrDispName)); 
		if(wcsstr(bstrDispName, L"\\")==NULL && wcsstr(bstrDispName, L"/")==NULL) {		
			DP((L"CItemEventDisp::doesn't include path separator; do save to disk for tagging checking\n")); 
			bRet=TRUE; //doesn't include path separator; do save to disk for tagging check
		}
		//if(wcscmp(bstrDispName,TEMP_MAGIC_NAME)==0)
		//bRet=TRUE;
     }
    }
   }
  }
 }
 return bRet;
}

//#define AYUEN_TEST_CODE
#ifdef AYUEN_TEST_CODE
#pragma message("Remove test code!!! &#*&$#*($&*#!($*#!(&$")
static BOOL bTestDm=FALSE, bTestMc=FALSE, bTestMt=FALSE, bTestIuo=FALSE, bTestEr=FALSE, bTestHdr=TRUE;
#endif

void CItemEventDisp::DoEmailApprovalForFlex(ATTACHMENTLIST&   listAttachments,CEAttributes *obligation)
{
	AdapterCommon::Attachments adapterAttachments;
	// Check each policy
	size_t nAttachments = listAttachments.size();
	DP((L"============ in DoEmailApprovalForFlex, attachments is [%d] ==============.\n",nAttachments));
	for (size_t i=0; i<nAttachments; i++)
	{
		LpAttachmentData lpAttachmentData = (LpAttachmentData)listAttachments[i];
		// Find mapped path here
		if(0 != lpAttachmentData->src[0])
		{
			GetRealPath(lpAttachmentData->src, lpAttachmentData->resolved, MAX_SRC_PATH_LENGTH);
		}
		AdapterCommon::Attachment oneAttachment;
		oneAttachment.SetSrcPath(lpAttachmentData->src);
		oneAttachment.SetTempPath(lpAttachmentData->temp);
		if(i==0)
		{
			CObligations aOb;
			int nObligations=aOb.GetObligations(obligation);;
			int iIndOb=0;
			for(iIndOb=0;iIndOb<nObligations;iIndOb++)
			{
				AdapterCommon::Obligation ob;
				LPOBLIGATION lpOb=aOb.m_Obligations[iIndOb];
				ob.SetName(lpOb->name.c_str());
				if(lpOb->hasArgs)
				{
					stdext::hash_map<std::wstring, std::wstring>::iterator itHash;
					for(itHash=lpOb->argNameValues.begin();itHash!=lpOb->argNameValues.end();itHash++)
						ob.AddAttribute(AdapterCommon::Attribute(itHash->first.c_str(),itHash->second.c_str()));
				}
				else
				{
					int iIndValues=0,cValues=(int)lpOb->values.size();
					for(iIndValues=0;iIndValues<cValues;iIndValues++)
					{
						if(iIndValues+1<cValues)
							ob.AddAttribute(AdapterCommon::Attribute(lpOb->values[iIndValues++].c_str(),lpOb->values[iIndValues].c_str()));
						else
							break;
					}
				}
				oneAttachment.AddObligation(ob);
			}
			aOb.FreeObligations();
		}
		adapterAttachments.AddAttachment(oneAttachment);
	}
	if( feat.is_enabled( NEXTLABS_FEATURE_PA_EMAIL_STRIP_ATTACHMENT) == true )
	{
		DP((L"\n[Start] Call RepositoryAdaptersManager in DoEmailApprovalForFlex\n"));
		RepositoryAdaptersManager(L"Outlook Enforcer",m_spMailItem,&adapterAttachments);
		DP((L"\n[End] Call RepositoryAdaptersManager in DoEmailApprovalForFlex\n"));
	}
}

class CSendPerformance
{
public:
	CSendPerformance()
	{
		m_dwTime = ::GetTickCount();
	}
	~CSendPerformance()
	{
		DWORD dwEndTime = ::GetTickCount();
		wchar_t strlog[1024] = {0};
		StringCchPrintfW(strlog,1024,L"OnSend function spend time is [%d]",dwEndTime-m_dwTime);
		NLPRINT_DEBUGVIEWLOG(L"%s", strlog);
		CEventLog::WriteEventLog(strlog,EVENTLOG_INFORMATION_TYPE,EVENTLOG_INFO_ID);
	}
	DWORD m_dwTime;
};

BOOL HasSpecialObligation(CEAttributes *pObligation, const wchar_t* kpwchObName)
{NLONLY_DEBUG
    NLPRINT_DEBUGVIEWLOG(L"Obligation:[%p]", pObligation);
    if ((NULL != pObligation) && (NULL != kpwchObName))
    {
        NLPRINT_DEBUGVIEWLOG(L"Obligation count:[%d]", pObligation->count);
        for (int iAttribute=0; iAttribute<pObligation->count; iAttribute++)
        {
            const wchar_t* wszName = cesdk.fns.CEM_GetString(pObligation->attrs[iAttribute].key);
            if (wszName && _wcsnicmp(wszName, CE_ATTR_OBLIGATION_NAME, wcslen(CE_ATTR_OBLIGATION_NAME) )==0 )
            {
                const wchar_t* szValue = cesdk.fns.CEM_GetString(pObligation->attrs[iAttribute].value);
                if (szValue && _wcsicmp(szValue, kpwchObName)==0)
                {
                    return TRUE;
                }
            }
        }
    }
    return FALSE;
}

// Prepare mail x-header HC tagging if OB_XHEADER_HIERARCHICAL_CLASSIFICATION exists
void PerpareXHeaderHCTagging(CSendEmailData & EmailData, CEAttributes *pHCObligation)
{
	CMessageHeader &msgHeader = EmailData.GetMessageHeader();
	COE_PAMngr *pPAMngr = msgHeader.GetPAMngr();

	if (NULL == pPAMngr)
	{
		pPAMngr = new COE_PAMngr();

		//see CPAMngr::bool SetObligations(const wchar_t *strSrcFileName, const wchar_t *TargetFileName, const wchar_t* strDestFileName, const CEAttributes *obligation) 
		// Only process the obligation XHEADER_HIERARCHICAL_CLASSIFICATION
		pPAMngr->SetOnlyAcceptXHeader(TRUE);

		msgHeader.SetPAMngr(pPAMngr);
	}

	// e.g %LocalAppData%\Temp\CEOutlookAddin\NLHEADER{12345678-1234-1234-12345678}.doc
	std::wstring& sHCTempFileRef = EmailData.GetMessageHeader().GetHCTempFile();
	if(sHCTempFileRef.empty())
	{
		sHCTempFileRef.assign(g_strOETempFolder);
		if (L'\\' != sHCTempFileRef[sHCTempFileRef.length() - 1])
		{
			sHCTempFileRef.append(L"\\");
		}
		sHCTempFileRef.append(L"NLHEADER");
		sHCTempFileRef.append(NLNewGUID());
		sHCTempFileRef.append(L".doc");

		const wchar_t* pwzFileName = sHCTempFileRef.c_str();

		logi(L"[PerpareXHeaderHCTagging] Create HCTempFile = %s", pwzFileName);

		//create the file
		HANDLE hTmpFile = CreateFile(pwzFileName,    // name of the file
			GENERIC_WRITE, // open for writing
			0,             // sharing mode, none in this case
			0,             // use default security descriptor
			CREATE_ALWAYS, // overwrite if exists
			FILE_ATTRIBUTE_NORMAL,
			0);
		if (INVALID_HANDLE_VALUE != hTmpFile)
		{
			CloseHandle(hTmpFile);
		}
		else
		{
			loge(L"CreateFile(%s) failed: R%pE%u", pwzFileName, hTmpFile, GetLastError());
		}
		
		int nObligations = pPAMngr->SetOE_Obligations(L"Email Header", pwzFileName, pHCObligation, OEACTIONFORHSC_FROCEDOING);
		logi(L"[PerpareXHeaderHCTagging] SetObligations returned %d", nObligations);

		msgHeader.SetNeedHCTagging();
	}
}

// returns true if the attachment need be ignored, otherwise, returns false.
BOOL IsAttachmentIgnored(std::vector<CAttachmentData> &attachmentsData, CComPtr<Outlook::Attachments> attachments, int iAtt)
{
	// Bug 41984 - [oe8.5]it will show many gif file in alert message when email calendar

	CAttachmentData& attachmentData = attachmentsData[iAtt];
	if (attachments != NULL)
	{
		CComPtr<Outlook::Attachment> pAttachment;
		CComVariant varAttachIndex = attachmentData.GetOriginalAttachIndex();
		attachments->Item(varAttachIndex, &pAttachment);
		if (pAttachment)
		{
			CComPtr<Outlook::_PropertyAccessor> spPropAccess;
			HRESULT hr = pAttachment->get_PropertyAccessor(&spPropAccess);
			if (SUCCEEDED(hr) && spPropAccess)
			{
				//https://stackoverflow.com/questions/9124913/how-to-know-if-attachment-is-a-signature-in-an-outlook-email
				//https://stackoverflow.com/questions/12310925/distinguish-visible-and-invisible-attachments-with-outlook-vba
				CComBSTR sbsPRAttachContentId(L"http://schemas.microsoft.com/mapi/proptag/0x3712001F"); //PR_ATTACH_CONTENT_ID
				//CComBSTR sbsPRAttachmentHidden(L"http://schemas.microsoft.com/mapi/proptag/0x7FFE000B"); //PR_ATTACHMENT_HIDDEN

				CComVariant varPropValue;
				HRESULT hr = spPropAccess->GetProperty(sbsPRAttachContentId, &varPropValue);

				if (SUCCEEDED(hr) && varPropValue.bstrVal)
				{
					bool isIgnored = boost::ends_with(attachmentData.FileName(), L".gif") && ::SysStringLen(varPropValue.bstrVal);

					logd(L"[IsAttachmentIgnored]Attachment(pos=%d, idx=%d/%d), FileName=%s, cid=%s, isIgnored=%d", attachmentData.GetOriginalAttachIndex(), 
						iAtt, attachmentsData.size(), attachmentData.FileName().c_str(), (LPCWSTR)varPropValue.bstrVal, isIgnored);

					if ( isIgnored )
					{
						return TRUE;
					}
				}
			}
		}
	}
	return FALSE;
}

LPCWSTR DecideActionForHSC(COE_PAMngr &paMngr, CComPtr<Outlook::Attachments> attachments, CAttachmentData& attachmentData)
{
	bool isFileModified = false;
	if (attachments != NULL)
	{
		CComPtr<Outlook::Attachment> pAttachment;
		CComVariant varAttachIndex = attachmentData.GetOriginalAttachIndex();
		attachments->Item(varAttachIndex, &pAttachment);
		if (pAttachment)
		{
			isFileModified = attachmentData.GetOutlookTempModifyFlagAndUpdate(pAttachment);
			logd(L"[DecideActionForHSC]GetOutlootTempModifyFlagAndUpdate return %d, src=%s.\n", isFileModified, attachmentData.GetSourcePath().c_str());
		}
	}

	wchar_t* wszActionForHSC = OEACTIONFORHSC_NOTHING;
	if (isFileModified)
	{
		wszActionForHSC = OEACTIONFORHSC_FROCEDOING;
	}
	else if (attachmentData.GetHCTagAlreadTagged().size()>0)
	{
		wszActionForHSC = OEACTIONFORHSC_FROCENOTDO;
		VECTOR_TAGPAIR::const_iterator it;
		for(it = attachmentData.GetHCTagAlreadTagged().begin(); it != attachmentData.GetHCTagAlreadTagged().end(); it++){
			logd(L"[DecideActionForHSC]HC tag key = %s", it->first.c_str());
			logd(L"[DecideActionForHSC]HC tag value = %s", it->second.c_str());
		}
	}
	else
	{
		logd(L"[DecideActionForHSC]attachmentData.GetHCTagAlreadTagged().size()=%d",attachmentData.GetHCTagAlreadTagged().size());
	}
	
	return wszActionForHSC;
}

void ClassifyObligation(CSendEmailData & EmailData,CQueryPCInfo &QueryPCInfo,COE_PAMngr &paMngr, CComPtr<IDispatch> spMailItem)
{NLONLY_DEBUG
	
	bool bSujectAllow = true;
	bool bBodyAllow = true;
	bool bHeaderAllow = true;
	bool bAttachAllow = true;

	CEEnforcement_t *enforcer;

	int nBodyRecordPos = EmailData.GetBodyData().GetRecordPos();
	enforcer = QueryPCInfo.GetEnforcement() + nBodyRecordPos;
	logi(L"[ClassifyObligation]Body_Record_Position = %d", nBodyRecordPos);
	CObligations::ClassifyObligationType(enforcer, BODY_COMMAND, EmailData, 0, 0, bBodyAllow);

	int nSubjectRecordPos = EmailData.GetSubjectData().GetRecordPos();
	enforcer = QueryPCInfo.GetEnforcement() + nSubjectRecordPos;
	logi(L"[ClassifyObligation]Subject_Record_Position = %d", nSubjectRecordPos);
	CObligations::ClassifyObligationType(enforcer, SUBJECT_COMMAND, EmailData, 0, 0, bSujectAllow);

	if (EmailData.GetInheritHeader())//m_bInheritHeader,default value false
	{
		int nHeaderRecordPos = EmailData.GetMessageHeader().GetRecordPos();
		enforcer = QueryPCInfo.GetEnforcement() + nHeaderRecordPos;
		logi(L"[ClassifyObligation]MessageHeader_Record_Position = %d.", nHeaderRecordPos);
		CObligations::ClassifyObligationType(enforcer, MESSAGE_HEADER_COMMAND, EmailData, 0, 0, bHeaderAllow);
	}

	bool bTempAllow = true;
	size_t nAttachmentNum = EmailData.GetAttachmentData().size();
	int nClassfiedObligationType = 0;
	paMngr.SetRecipients(EmailData.GetRecipientsData().GetRealRecipients());
	paMngr.SetSender(EmailData.GetSender());
	bool bDoneRejectSlient = false;

	CComPtr<Outlook::Attachments> attachments = NULL;
	MailItemUtility::get_Attachments(spMailItem, &attachments, TRUE);
	for (DWORD i = 0; i < nAttachmentNum; i++)
	{
		if(EmailData.GetAttachmentData()[i].IsAttachmentRemoved())
		{

			continue;
		}

		DWORD dwOblTypes = 0;
		BOOL isAttachmentIgnored = IsAttachmentIgnored(EmailData.GetAttachmentData(), attachments, i);
		EmailData.GetAttachmentData()[i].SetIgnored(isAttachmentIgnored);

		int nAttachmentRecordPos = EmailData.GetAttachmentData()[i].GetRecordPos();
		CObligations::ClassifyObligationType(QueryPCInfo.GetEnforcement() + nAttachmentRecordPos,ATTACHMENT_COMMAND,EmailData,i,0,bTempAllow, &dwOblTypes);

		logd(L"Attachments[%d] Allowed=%d: %s\n", i, bTempAllow, EmailData.GetAttachmentData()[i].GetTempPath().c_str());

		CAttachmentData& attachmentData = EmailData.GetAttachmentData()[i];

		if(isAttachmentIgnored) // to skip SetOE_Obligations
		{
			continue;
		}

		LPCWSTR wszActionForHSC = NULL;
		CEEnforcement_t* enforcer = (CEEnforcement_t*)(QueryPCInfo.GetEnforcement() + nAttachmentRecordPos);
		if (enforcer != NULL)
		{
			CAttachmentData& attachmentData = EmailData.GetAttachmentData()[i];

			//check file modified
			bool bModified = false;
			if (HasSpecialObligation(enforcer->obligation, OB_NAME_OHC))
			{
               logd(L"HasHierarchicalClassificationObligation return true.\n");
			   if (attachments != NULL)
			   {
				   CComPtr<Outlook::Attachment> pAttachment;
				   CComVariant varAttachIndex = attachmentData.GetOriginalAttachIndex();
				   attachments->Item(varAttachIndex, &pAttachment);
				   if (pAttachment)
				   {
					   bModified = attachmentData.GetOutlookTempModifyFlagAndUpdate(pAttachment);
					   logd(L"GetOutlootTempModifyFlagAndUpdate return %d, src=%s.\n", bModified, attachmentData.GetSourcePath().c_str());
				   }
			   }
			}

			wszActionForHSC = OEACTIONFORHSC_NOTHING;
			if (bModified)
			{
				wszActionForHSC = OEACTIONFORHSC_FROCEDOING;
			}
			else if (attachmentData.GetHCTagAlreadTagged().size()>0)
			{
				wszActionForHSC = OEACTIONFORHSC_FROCENOTDO;
				VECTOR_TAGPAIR::const_iterator it;
				for(it = attachmentData.GetHCTagAlreadTagged().begin(); it != attachmentData.GetHCTagAlreadTagged().end(); it++){
					logd(L"[ClassifyObligation]HC tag key = %s", it->first.c_str());
					logd(L"[ClassifyObligation]HC tag value = %s", it->second.c_str());
				}
			}
			else
			{
				logd(L"[ClassifyObligation]attachmentData.GetHCTagAlreadTagged().size()=%d",attachmentData.GetHCTagAlreadTagged().size());
			}

			paMngr.SetOE_Obligations(EmailData.GetAttachmentData()[i].GetSourcePath().c_str(),EmailData.GetAttachmentData()[i].GetTempPath().c_str(),enforcer->obligation, wszActionForHSC);
		}


		bAttachAllow = bAttachAllow&&bTempAllow;

		vector<int> vecRecipientRecordPos= EmailData.GetAttachmentData()[i].GetRecordPosForReceiveAction();
		vector<int> vecRecipientIndex = EmailData.GetAttachmentData()[i].GetReceiverIndexForReceiveAction();
		
		if (!bDoneRejectSlient)
		{
			int nExistType = EmailData.GetAttachmentData()[i].GetExistObligationType();
			if (nExistType & LOOP_OBLIGATION_REJECT_UNLESS_SILENT_OVERRIDE)
			{
				bDoneRejectSlient = true;
				logd(L"ExistType has been set LOOP_OBLIGATION_REJECT_UNLESS_SILENT_OVERRIDE, set bDoneRejectSlient true.\n");
			}
		}

		LPCWSTR wszActionForHSCOfAttRecAsUserCmd = NULL;
		for(size_t j = 0; j < vecRecipientRecordPos.size(); j++)
		{
			DWORD dwOblTypesOfAttRecAsUserCmd = 0;
			int nRecipientPos = vecRecipientIndex[j];
			int nRecordPos = vecRecipientRecordPos[j];
			nClassfiedObligationType = nClassfiedObligationType | CObligations::ClassifyObligationType(QueryPCInfo.GetEnforcement() + nRecordPos
				, ATTACHMENT_RECIPIENT_AS_USER_COMMAND, EmailData,i,nRecipientPos ,bTempAllow, &dwOblTypesOfAttRecAsUserCmd);

			// Only workaround when ATTACHMENT_COMMAND request not matches HC (HDR) and ATTACHMENT_RECIPIENT_AS_USER_COMMAND request matches HC (HDR)
			// because we only allow to pop up UI dialog once for the same file. 
			// See detail about doing obligations when recipient attributes match at http://bugs.cn.nextlabs.com/show_bug.cgi?id=45872#c8 or http://bugs.cn.nextlabs.com/show_bug.cgi?id=46026
			// EmailData.GetAttachmentData()[nAttachmentPos].GetExistObligationType()
			if (!((LOOP_OBLIGATION_FILE_OHC | LOOP_OBLIGATION_HDR) & dwOblTypes) && ((LOOP_OBLIGATION_FILE_OHC | LOOP_OBLIGATION_FILE_TAGGING_AUTO | LOOP_OBLIGATION_HDR )& dwOblTypesOfAttRecAsUserCmd))
			{
				CEEnforcement_t* enforcer = (CEEnforcement_t*)(QueryPCInfo.GetEnforcement() + nRecordPos );
				if (NULL != enforcer)
				{
					// first one of ATTACHMENT_RECIPIENT_AS_USER_COMMAND requests
					if (NULL == wszActionForHSCOfAttRecAsUserCmd)
					{
						CAttachmentData& attachmentData = EmailData.GetAttachmentData()[i];
						wszActionForHSCOfAttRecAsUserCmd = DecideActionForHSC(paMngr, attachments, attachmentData);
						paMngr.SetOE_Obligations(attachmentData.GetSourcePath().c_str(),attachmentData.GetTempPath().c_str(),enforcer->obligation, wszActionForHSC);
					}
				}
			}
			
			if (bDoneRejectSlient && (nClassfiedObligationType & LOOP_OBLIGATION_REJECT_UNLESS_SILENT_OVERRIDE))
			{
				bTempAllow = true;
				logd(L"After done LOOP_OBLIGATION_REJECT_UNLESS_SILENT_OVERRIDE,set allow == true.\n");
			}
			bAttachAllow = bAttachAllow&&bTempAllow;
		}
	}


	if (!bDoneRejectSlient)
	{
		if (nClassfiedObligationType & LOOP_OBLIGATION_REJECT_UNLESS_SILENT_OVERRIDE )
		{
			logd(L"Set SetExistObligationTypeForAllAttachment for LOOP_OBLIGATION_REJECT_UNLESS_SILENT_OVERRIDE.\n");
			EmailData.SetExistObligationTypeForAllAttachment(LOOP_OBLIGATION_REJECT_UNLESS_SILENT_OVERRIDE);
		}
	}

	BOOL bRecipientAllow = TRUE;
	// Classify for each Recipient
	std::vector<int> &vecRecordPosForRecipients = EmailData.GetRecordPosForRecipients();
	//bool bDumpAllowForRecipient;
	for (int idxRecipient = 0, nCount = vecRecordPosForRecipients.size(); idxRecipient < nCount; ++idxRecipient)
	{
		bool bTempAllow = TRUE;
		logd(L"[ClassifyObligation]Recipient_%d_Record_Position = %d.", idxRecipient, vecRecordPosForRecipients[idxRecipient]);
		CObligations::ClassifyObligationType(QueryPCInfo.GetEnforcement() + vecRecordPosForRecipients[idxRecipient], RECIPIENT_COMMAND, EmailData, 0, idxRecipient, bTempAllow);
		bRecipientAllow = bRecipientAllow && bTempAllow;
	}

	logd(L"Allow value: Header=%d, Subject=%d, Body=%d, Attach=%d, Recipient=%d", bHeaderAllow, bSujectAllow, bBodyAllow, bAttachAllow, bRecipientAllow);
	EmailData.SetAllow(bSujectAllow && bBodyAllow && bHeaderAllow && bAttachAllow && bRecipientAllow);
	EmailData.SetAllowBegin(EmailData.GetAllow());
}



EXCUTEOBLACTION CAttachmentObligationData::ExcuteMailAttributeParsingEx(CSendEmailData & EmailData)
{
	if( EmailData.m_OnlyAttachmentData.m_bNeedMailap ) 
	{//Mail Attribute Parsing
	
		EmailData.m_OnlyAttachmentData.m_mapInfo.bObligationDone = true;
		

		DP((L"bNeedMailap Need maillap")) ;
		CMAttrParAssistDlg mailapDlg ;
		mailapDlg.SetMAttrParType( EmailData.m_OnlyAttachmentData.m_mapInfo.bIsBlock,  EmailData.GetRecipientsData().GetRealRecipients() ) ;
		mailapDlg.SetMAttrParMessage( EmailData.m_OnlyAttachmentData.m_mapInfo.msgType,  EmailData.m_OnlyAttachmentData.m_mapInfo.strMessage ) ;
		mailapDlg.SetHelpUrl( EmailData.m_OnlyAttachmentData.m_mapInfo.url.c_str() ) ;

		// Show dialog
		LRESULT lResult = mailapDlg.DoModal(EmailData.GetWnd());
		if (IDOK == lResult)
		{
			if( EmailData.m_OnlyAttachmentData.m_mapInfo.bIsBlock )
			{
				//block
				return EXCUTE_CANCEL_MAYBE_LOG_DESIGN;
			}
		}
		else
		{
			//Warning, user click cancel
			return EXCUTE_CANCEL_MAYBE_LOG_DESIGN;
		}		
		EmailData.SetEvalAgain(true);
	}
	return EXCUTESUCCESS;

}

EXCUTEOBLACTION ExcuteOblgation(CSendEmailData & EmailData,PolicyCommunicator* pPolicyCommunicator,CComPtr<IDispatch> dspMailItem,COE_PAMngr& paMngr)
{
    NLTRACER_CREATEINSTANCE(true, true, L"Begin excute obligations");
    logd(L"[ExecuteObligation]Begin excute obligations");

	CAttachmentObligationData AttachmentOBL;
	EXCUTEOBLACTION excuteRet;

	AttachmentOBL.ExcuteRichUserMsg(EmailData);

	EmailData.ExecuteInheritHeaderObliation();

	excuteRet = AttachmentOBL.ExcuteContentReactionBody(EmailData);
	if (excuteRet != EXCUTESUCCESS)
	{
		EmailData.SetAllow(false);
		return excuteRet;
	}
	excuteRet = AttachmentOBL.ExcuteContentReactionSubject(EmailData);
	if (excuteRet != EXCUTESUCCESS)
	{
		EmailData.SetAllow(false);
		return excuteRet;
	}

    /**< Below obligations contains user UI, no need record the times. MailAttributeParsing, Notification, EmailStrIPAttachmen, Internet, HelpURLFirstMatchAttachment, EmailVerify, EmailHDR, FileTagging, Encryption, PortableEncryption, NXLEncrpty */
    NLTRACER_SETRUNTIMEINFO(NULL);
    excuteRet = AttachmentOBL.ExcuteMailAttributeParsingEx(EmailData);  // UI
	if (excuteRet != EXCUTESUCCESS)
	{
		EmailData.SetAllow(false);
		return excuteRet;
	}
	excuteRet = AttachmentOBL.ExcuteMailNotificationEx(EmailData);
	if (excuteRet != EXCUTESUCCESS)
	{
		EmailData.SetAllow(false);
		return excuteRet;
	}

	excuteRet = AttachmentOBL.ExcuteEmailStrIPAttachmentEx(EmailData,dspMailItem,paMngr);
	if (excuteRet != EXCUTESUCCESS)
	{
		EmailData.SetAllow(false);
		return excuteRet;
	}

	


	excuteRet = AttachmentOBL.ExcuteInternetUseOnlyEx(EmailData);
	if (excuteRet != EXCUTESUCCESS)
	{
		EmailData.SetAllow(false);
		return excuteRet;
	}

	excuteRet = AttachmentOBL.ExcuteHelpURLFirstMatchAttachmentEx(EmailData);
	if (excuteRet != EXCUTESUCCESS)
	{
		EmailData.SetAllow(false);
		return excuteRet;
	}
	excuteRet = AttachmentOBL.ExcuteEmailVerifyEx(EmailData);
	if (excuteRet != EXCUTESUCCESS)
	{
		EmailData.SetAllow(false);
		return excuteRet;
	}

	excuteRet = AttachmentOBL.ExcuteEmailHDREx(EmailData);
	if (excuteRet != EXCUTESUCCESS)
	{
		EmailData.SetAllow(false);
		return excuteRet;
	}

	excuteRet = AttachmentOBL.ExcuteFileTaggingEx(EmailData, pPolicyCommunicator, paMngr, dspMailItem);
	if (excuteRet != EXCUTESUCCESS)
	{
		EmailData.SetAllow(false);
		return excuteRet;
	}

	excuteRet = AttachmentOBL.ExcuteHeaderTaggingEx(EmailData,pPolicyCommunicator, paMngr, dspMailItem);
	if (excuteRet != EXCUTESUCCESS)
	{
		EmailData.SetAllow(false);
		return excuteRet;
	}

	excuteRet = AttachmentOBL.ExcuteEncryptionEx(EmailData,pPolicyCommunicator, paMngr);
	if (excuteRet != EXCUTESUCCESS)
	{
		EmailData.SetAllow(false);
		return excuteRet;
	}


#if 0
	excuteRet = AttachmentOBL.ExcutePortableEncryptionEx(EmailData, pPolicyCommunicator, paMngr);
	if (excuteRet != EXCUTESUCCESS)
	{
		EmailData.SetAllow(false);
		return excuteRet;
	}

#endif

	excuteRet = AttachmentOBL.ExcuteNXLEncrptyEx(EmailData);
	if (excuteRet != EXCUTESUCCESS)
	{
		EmailData.SetAllow(false);
		return excuteRet;
	}

    NLTRACER_SETRUNTIMEINFO(NULL);
	excuteRet = AttachmentOBL.ExcutePrependBody(EmailData);
	if (excuteRet != EXCUTESUCCESS)
	{
		EmailData.SetAllow(false);
		return excuteRet;
	}

	excuteRet = AttachmentOBL.ExcuteAppendBody(EmailData);
	if (excuteRet != EXCUTESUCCESS)
	{
		EmailData.SetAllow(false);
		return excuteRet;
	}

	excuteRet = AttachmentOBL.ExcutePrependSubject(EmailData);
	if (excuteRet != EXCUTESUCCESS)
	{
		EmailData.SetAllow(false);
		return excuteRet;
	}

	//we can't update real recipients when doing obligation. so we update real recipients after all obligation finished.
    EmailData.GetRecipientsData().UpdateRealRecipients();

	return excuteRet;
}


void CItemEventDisp::Init(CComPtr<IDispatch> dspMailItem,CSendEmailData &emailData,CAttachmentObligationData & AttachmentOblData, bool bInline)
{
	emailData.m_AttachmentOblData = &AttachmentOblData;

	emailData.m_OnlyAttachmentData.m_bNeedDm = FALSE;
	emailData.m_OnlyAttachmentData.m_bNeedMc = FALSE;
	emailData.m_OnlyAttachmentData.m_bNeedIuo = FALSE;
	emailData.m_OnlyAttachmentData.m_bNeedEr = FALSE;
	emailData.m_OnlyAttachmentData.m_bNeedHdr = FALSE;
	emailData.m_OnlyAttachmentData.m_bNeedStripAttachment=FALSE;
	emailData.m_OnlyAttachmentData.m_bNeedLd = FALSE;
	emailData.m_OnlyAttachmentData.m_bNeedMt = FALSE;
	emailData.m_OnlyAttachmentData.m_bNeedMailap = FALSE;
	emailData.m_OnlyAttachmentData.m_bNeedMailNoti = FALSE;
	emailData.m_OnlyAttachmentData.m_bNeedCrl = FALSE;


	emailData.m_OnlyAttachmentData.m_bNeedInterFileTagging = FALSE;
	emailData.m_OnlyAttachmentData.m_bNeedAutoFileTagging = FALSE;
	emailData.m_OnlyAttachmentData.m_bNeedHCFileTagging = FALSE;
	emailData.m_OnlyAttachmentData.m_bNeedZIP = FALSE;
	emailData.m_OnlyAttachmentData.m_bNeedPGP = FALSE;
	emailData.m_OnlyAttachmentData.m_bNeedURMsg = FALSE;
	emailData.m_OnlyAttachmentData.m_RecordTempRecipientsForCrl = L"";
	emailData.m_OnlyAttachmentData.m_bNeedRMC = FALSE;
	
	if (!bInline)
	{
		OLUtilities::ActiveMailWindow(dspMailItem,emailData.m_OnlyAttachmentData.m_bIsWordMail);
	}
	emailData.SetWnd(GetForegroundWindow());
  
	emailData.SetSender(PolicyCommunicator::m_wzSenderName);
	logd(L"[CItemEventDisp::Init]PolicyCommunicator::m_wzSenderName = %s", PolicyCommunicator::m_wzSenderName);
	if (m_mtMailType == FWMAIL)
	{
		emailData.SetForward(true);
	}
}


void EvaAgainInit(CSendEmailData &emailData,COE_PAMngr& paMngr)
{
	logd(L"[EvaAgainInit]");

	emailData.m_OnlyAttachmentData.m_bNeedDm = FALSE;
	emailData.m_OnlyAttachmentData.m_bNeedMc = FALSE;
	emailData.m_OnlyAttachmentData.m_bNeedIuo = FALSE;
	emailData.m_OnlyAttachmentData.m_bNeedEr = FALSE;
	emailData.m_OnlyAttachmentData.m_bNeedHdr = FALSE;
	emailData.m_OnlyAttachmentData.m_bNeedStripAttachment=FALSE;
	emailData.m_OnlyAttachmentData.m_bNeedLd = FALSE;
	emailData.m_OnlyAttachmentData.m_bNeedMt = FALSE;
	emailData.m_OnlyAttachmentData.m_bNeedMailap = FALSE;
	emailData.m_OnlyAttachmentData.m_bNeedMailNoti = FALSE;
	emailData.m_OnlyAttachmentData.m_bNeedCrl = FALSE;


	emailData.m_OnlyAttachmentData.m_bNeedInterFileTagging = FALSE;
	emailData.m_OnlyAttachmentData.m_bNeedAutoFileTagging = FALSE;
	emailData.m_OnlyAttachmentData.m_bNeedHCFileTagging = FALSE;
	emailData.m_OnlyAttachmentData.m_bNeedZIP = FALSE;
	emailData.m_OnlyAttachmentData.m_bNeedPGP = FALSE;
	emailData.m_OnlyAttachmentData.m_bNeedURMsg = FALSE;
	emailData.m_OnlyAttachmentData.m_bNeedRMC = FALSE;
	emailData.m_OnlyAttachmentData.m_RecordTempRecipientsForCrl = L"";
	emailData.ResetInformation();
	paMngr.ClearObjectList();
	emailData.m_AttachmentOblData->m_adapterAttachments.FreeAttachments();
	//emailData.SaveMsgAlertMessageData();

	//// Bug 43866 - [oe8.5]can't show rich alert message if deny do content redaction
	// emailData.ClearAllAlertMessages();
	emailData.IncreaseEvaAgainTimes();

	//emailData.SetMsgAlertMessage();
	
}

bool ChangedContent(CSendEmailData & EmailData,COE_PAMngr& paMngr)
{
	bool bRet = EmailData.IsEvalAgain();
	if (bRet)
	{
		EvaAgainInit(EmailData,paMngr);
	}
	return bRet;
}

SAFEARRAY* CreateSafeArrayFromFile(const wchar_t* wstrFileName)
{
	//open file
	HANDLE hFile = CreateFileW(wstrFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
	if (hFile==INVALID_HANDLE_VALUE)
	{
		return NULL;
	}
	else
	{
		//get file size
		DWORD dwFileSizeH=0, dwFileSizeL=0;
		dwFileSizeL = GetFileSize(hFile, &dwFileSizeH);

		//create safearray
		SAFEARRAYBOUND  Bound;
		Bound.lLbound   = 0;
		Bound.cElements = dwFileSizeL; //now we just care this value,because the attachment can't be too large.
		SAFEARRAY* pSafeArray = SafeArrayCreate(VT_UI1, 1, &Bound);

		unsigned char* pData=NULL;
		HRESULT hr = SafeArrayAccessData(pSafeArray, (void**)&pData);
		if (SUCCEEDED(hr) && pData)
		{
			DWORD dwReadCount = 0;
			BOOL bReadFile = ReadFile(hFile, pData, dwFileSizeL, &dwReadCount, NULL);
			if (!bReadFile)
			{
				DP((L"CreateSafearrayFromFile read file failed.\n"));
			}
			SafeArrayUnaccessData(pSafeArray);
		}

		//free
		CloseHandle(hFile);
		hFile = INVALID_HANDLE_VALUE;

		return pSafeArray;
	}
}

BOOL UpdateAttachmentContent(CComPtr<Outlook::Attachment> pAttachment, const wchar_t* wszNewDataFile)
{
	HRESULT hr;
	CComPtr<Outlook::_PropertyAccessor> pPropAccess;
	hr = pAttachment->get_PropertyAccessor(&pPropAccess);
	if (pPropAccess)
	{
		const static CComBSTR bstrScheNameContent = L"http://schemas.microsoft.com/mapi/proptag/0x37010102";
		CComVariant varNewData;
		varNewData.vt = VT_ARRAY|VT_UI1;
		varNewData.parray = CreateSafeArrayFromFile(wszNewDataFile);

			//get attach method
		hr = pPropAccess->SetProperty(bstrScheNameContent, varNewData);

		
		if (SUCCEEDED(hr))
		{

			return TRUE;
		}
		else
		{
			loge(L"UpdateAttachmentContent failed. hr=0x%x\n",hr);
			return FALSE;
		}
	}else
	{
		loge(L"[UpdateAttachmentContent]get_PropertyAccessor failed. hr=0x%x\n", hr);
	}
	return FALSE;
}


BOOL UpdateAttachment(CSendEmailData & EmailData,CComPtr<IDispatch> dspMailItem)
{
	CComPtr<Outlook::Attachments> attachments = NULL;
	MailItemUtility::get_Attachments(dspMailItem, &attachments,TRUE);
	if (attachments != NULL)
	{
		long nAttachCount=0;
		attachments->get_Count(&nAttachCount);

		std::vector< std::pair< CComPtr<Outlook::Attachment>, int> > vecAttachmentsNeedReattaching;

		for(int nAttachIndex=nAttachCount; nAttachIndex>0; nAttachIndex--)
		{
			CComPtr<Outlook::Attachment> pAttachment;
			CComVariant varAttachIndex = nAttachIndex;
			attachments->Item(varAttachIndex, &pAttachment);

			CAttachmentData* pAttachmentData = EmailData.GetAttachmentDataByIndex(nAttachIndex);

			if (pAttachmentData)
			{

				logd(L"[UpdateAttachment]#%d, Updated=%d,FileNameChangedAfterObligation=%d, NxlSuccessful=%d, OETempPath=%s, SourcePath=%s, OrgTempPath=%s", 
					nAttachIndex, pAttachmentData->IsAttachmentUpdated(), pAttachmentData->IsFileNameChangedAfterObligation(), pAttachmentData->IsNxlSuccessful(),
					pAttachmentData->GetTempPath().c_str(), pAttachmentData->GetSourcePath().c_str(), pAttachmentData->GetOrgTempPath().c_str());

				if (pAttachmentData->IsAttachmentUpdated())
				{
					if (pAttachmentData->IsAttachmentRemoved())
					{
						attachments->Remove(nAttachIndex);

					}
					else if (!pAttachmentData->IsFileNameChangedAfterObligation() && !pAttachmentData->IsNxlSuccessful())
					{

						logd(L"execute[UpdateAttachmentContent].");
						//file name not changed, we only update the content of this attachment
						UpdateAttachmentContent(pAttachment, pAttachmentData->GetTempPath().c_str());

					}
					else
					{
						vecAttachmentsNeedReattaching.push_back(std::make_pair(pAttachment, nAttachIndex));
					} 
				}
				else
				{
					//attachment not changed
				}
			}	
		}

		// file name changed, we need re-attach it
		HRESULT hr;
		int nStep;
		std::vector< std::pair< CComPtr<Outlook::Attachment>, int> >::const_iterator itAtt;
		for( itAtt = vecAttachmentsNeedReattaching.begin(); itAtt != vecAttachmentsNeedReattaching.end(); ++itAtt)
		{
			nStep = 0;
			// https://msdn.microsoft.com/en-us/vba/outlook-vba/articles/attachments-add-method-outlook
			// To ensure consistent results, always save an item before adding or removing objects in the Attachments collection of the item.
			const CComPtr<Outlook::Attachment> &spAttachment = itAtt->first;
			const long nAttachIndex = itAtt->second;

			long nAttIdx = -1;
			CComBSTR sbsFileName;
			spAttachment->get_Index(&nAttIdx);
			spAttachment->get_FileName(&sbsFileName);

			hr = MailItemUtility::Save(dspMailItem);
			++nStep;
			if (SUCCEEDED(hr))
			{
				hr = spAttachment->Delete();
				++nStep;
				if (SUCCEEDED(hr))
				{
					hr = MailItemUtility::Save(dspMailItem);
					++nStep;
					if (SUCCEEDED(hr))
					{
						long nNewAttIdx = -1;
						CAttachmentData* pAttachmentData = EmailData.GetAttachmentDataByIndex(nAttachIndex);
						CAttachmentFileMgr& attachFileMgr = CAttachmentFileMgr::GetInstance();
						std::wstring wstrOETempPath = pAttachmentData->GetTempPath();
						CComPtr<Outlook::Attachment> spNewAttach=attachFileMgr.AddFileInAttachment(attachments, wstrOETempPath,
							pAttachmentData->GetSourcePath(), pAttachmentData->GetOrgTempPath(), emFileCacheUpdate);
						pAttachmentData->SetTempPath(wstrOETempPath.c_str());

						if (NULL != spNewAttach)
						{
							hr = spNewAttach->get_Index(&nNewAttIdx);
						}
						logd(L"[UpdateAttachment]_%d wstrOETempPath=%s, spNewAttach@%p, NewAttIdx=%d; Old: FileName=%s, Attachment_%d", 
							nAttachIndex, wstrOETempPath.c_str(), spNewAttach.p, nNewAttIdx, (LPCWSTR)sbsFileName.m_str, nAttIdx);

						if (NULL == spNewAttach)
						{
							return FALSE;	
						}else
						{
							continue;
						}
					}
				}
			}
		
			loge(L"[UpdateAttachment]Reattach:Step=%d, FileName=%s, Attachment_%d vs CAttachmentData_%d, hr=%#x", nStep, (LPCWSTR)sbsFileName.m_str, nAttIdx, nAttachIndex, hr);
		}
	}
}

bool UpdateRecipents(CSendEmailData & EmailData,CComPtr<IDispatch> dspMailItem)
{
	wstring strSenderInRecipient = EmailData.GetRecipientsData().GetSenderInRecipient();
	if (!strSenderInRecipient.empty())
	{
		 STRINGLIST RealRecipients = EmailData.GetRecipientsData().GetRealRecipients();
		 bool bExistOtherRecipients = false;
		 STRINGLIST::iterator Senderitor;
		 for (Senderitor = RealRecipients.begin(); Senderitor != RealRecipients.end(); Senderitor++)
		 {
			if (_wcsicmp(strSenderInRecipient.c_str(),(*Senderitor).c_str()) != 0)
			{
				bExistOtherRecipients = true;
				break;
			}
		 }
		 if (!bExistOtherRecipients)
		 {
			 MessageBox(NULL, L"No recipients were selected, please check again.", L"Nextlabs Enforcer For Microsoft Outlook", MB_OK);
			 return false;
		 }
	}


	STRINGLIST RmRecipients = EmailData.GetRecipientsData().GetRmRecipients();
	STRINGLIST::iterator itor;
	for (itor = RmRecipients.begin(); itor != RmRecipients.end(); itor++)
	{
		OLUtilities::RemoveMailRecipient(dspMailItem,*itor);
	}
	return true;
}
void UpdateSubject(CSendEmailData & EmailData,CComPtr<IDispatch> dspMailItem)
{

	wstring strSubject = EmailData.GetSubjectData().GetTempSubject();
	if (OLUtilities::SetMailSubject(dspMailItem, strSubject.c_str()))
	{
		logd(L"===>set mail subject succeed.\n");
	}
	else
	{
		loge(L"===>set mail subject failed.\n");
	}
}

BOOL GetWordDocument(BOOL bInline, CComPtr<IDispatch> dspMailItem, Word::_Document** ppWordDoc)
{
   if (bInline)
   {
#if defined(WSO2K16) || defined(WSO2K13)
	   CComPtr<IDispatch> spWordEditor;
	   HRESULT hr = g_pOutlookObj->get_Explorer()->get_ActiveInlineResponseWordEditor(&spWordEditor);
	   if (SUCCEEDED(hr) && spWordEditor)
	   {
		   hr = spWordEditor->QueryInterface(Word::IID__Document, (void**)ppWordDoc);
		   return (SUCCEEDED(hr) && *ppWordDoc);
	   }
#else
	   return FALSE;
#endif
   }
   else
   {
	   CComPtr<Outlook::_Inspector> spInspector;
	   HRESULT hr = MailItemUtility::get_GetInspector(dspMailItem, &spInspector);
	   if (SUCCEEDED(hr) && spInspector)
	   {
		   CComPtr<IDispatch> spWordEditor;
		   hr = spInspector->get_WordEditor(&spWordEditor);
		   if (SUCCEEDED(hr) && spWordEditor)
		   {
			   hr = spWordEditor->QueryInterface(Word::IID__Document, (void**)ppWordDoc);
			   return (SUCCEEDED(hr) && *ppWordDoc);
		   }
	   }
   }

   return FALSE;
}
void WordEditorPutText(CSendEmailData & EmailData, wstring& strBeforeBody, wstring& strAfterBody)
{
	CComPtr<Word::_Document> pWordDoc = EmailData.GetWordDoc();
	if (pWordDoc == NULL)
	{
		return;
	}

	CComPtr<Word::Paragraphs> spParagraphs;
	HRESULT hr = pWordDoc->get_Paragraphs(&spParagraphs);
	if (SUCCEEDED(hr) && spParagraphs)
	{
		CComPtr<Word::Paragraph> spFirstParagraph;
		hr = spParagraphs->get_First(&spFirstParagraph);
		if (SUCCEEDED(hr) && spFirstParagraph )
		{
			CComPtr<Word::Range> spRange;
			hr = spFirstParagraph->get_Range(&spRange);
			if (SUCCEEDED(hr)&&spRange)
			{
				CComBSTR bstrBefore(strBeforeBody.c_str());
				spRange->InsertBefore(bstrBefore);
			}
		}


		CComPtr<Word::Paragraph> spAfterParagraph;
		hr = spParagraphs->get_Last(&spAfterParagraph);
		if (SUCCEEDED(hr) && spAfterParagraph )
		{
			CComPtr<Word::Range> spRange;
			hr = spAfterParagraph->get_Range(&spRange);
			if (SUCCEEDED(hr)&&spRange)
			{
				CComBSTR bstrAfter(strAfterBody.c_str());
				spRange->InsertAfter(bstrAfter);
			}
		}
	}
}

wstring ConvertStripAttachmentMessage(_In_ wstring kwstrStripAttachmentMessage)
{
    if (!kwstrStripAttachmentMessage.empty())
    {
        boost::algorithm::replace_all(kwstrStripAttachmentMessage, L"<br>", L"\n");
        return OLUtilities::DeleteHtmlFormat(kwstrStripAttachmentMessage);
    }
    return L"";
}

void UpdateBody(CSendEmailData & EmailData,CComPtr<IDispatch> dspMailItem, BOOL bInline)
{
	CComPtr<Word::_Document> wordDoc;
	if (GetWordDocument(bInline, dspMailItem, &wordDoc))
	{
		logd(L"Update Body with Word Document interface.\n");
		EmailData.SetWordDoc(wordDoc);

        CBodyData& obBodyData = EmailData.GetBodyData();
		obBodyData.ReplaceParagraphsText(wordDoc);
		map<wstring,bool> mapAppend = obBodyData.GetAppendBody();
		map<wstring,bool>::iterator itor;
		bool bPutText = false;
		wstring strAppendBody = L"\n";
		for (itor = mapAppend.begin(); itor != mapAppend.end(); itor++)
		{
			strAppendBody += itor->first;
            strAppendBody += L"\n";
			bPutText = true;
		}
		wstring strPrependBody;
		map<wstring,bool> mapPrepend = obBodyData.GetPrependBody();
		for (itor = mapPrepend.begin(); itor != mapPrepend.end(); itor++)
		{
			strPrependBody += itor->first;
			strPrependBody += L"\n";
			bPutText = true;
		}

        // Strip attachment
        wstring strTopMsg = ConvertStripAttachmentMessage(obBodyData.GetStripAttachmentTopMessage());
        if (!strTopMsg.empty())
        {
            strPrependBody += strTopMsg + L"\n";
            bPutText = true;
        }
        wstring strBottomMsg = ConvertStripAttachmentMessage(obBodyData.GetStripAttachmentBottomMessage());
        if (!strBottomMsg.empty())
        {
            strAppendBody += strBottomMsg + L"\n";
            bPutText = true;
        }

		if (bPutText )
		{
			WordEditorPutText(EmailData,strPrependBody,strAppendBody);
		}
	}
	else
	{
		logd(L"Update Body with pub_body method.\n");
		wstring strBody =EmailData.GetBodyData().GetStripAttachmentTopMessage()+ EmailData.GetBodyData().GetTempBody()+EmailData.GetBodyData().GetStripAttachmentBottomMessage();
		OLUtilities::SetMailBody(dspMailItem, strBody.c_str());
	}

	MailItemUtility::Save(dspMailItem);
}

bool CItemEventDisp::ReplaceAssignTaskContent(CComPtr<IDispatch> dspTaskItem)//assign task request or task
{
	HRESULT hr = S_FALSE;
	CComPtr<Outlook::_TaskRequestItem> spTaskRequestItem = 0;
	CComPtr<Outlook::_TaskItem> spAssociatedTaskItem = 0;
	ITEM_TYPE taskType = DEFAULT;
	OLUtilities::CheckGetMailItemType(dspTaskItem, taskType);
	logi(L"[ReplaceAssignTaskContent]====>tasktype=%d", taskType);

	hr = m_spSaveOrigMailItem->QueryInterface(Outlook::IID__TaskRequestItem, (void**)&spTaskRequestItem);
	hr = dspTaskItem->QueryInterface(Outlook::IID__TaskItem, (void**)&spAssociatedTaskItem);
	if (NULL == spTaskRequestItem || NULL == spAssociatedTaskItem)
	{
		return false;
	}

	CComPtr<Outlook::Attachments> attachments = NULL;
	hr = MailItemUtility::get_Attachments(m_spSaveOrigMailItem, &attachments, FALSE);
	if (NULL == attachments)
	{
		loge(L"[ReplaceAssignTaskContent]Attachments of the request task is null, hr=%#x\n", hr);
		return false;
	}

	std::wstring sTaskFile(L"task.msg");
	{
		CComPtr<Outlook::Attachment> spOldAttachment = 0;
		hr = attachments->Item(CComVariant(1), &spOldAttachment);
		if (SUCCEEDED(hr))
		{
			CComBSTR sbsOldAttachmentFileName;
			hr = spOldAttachment->get_FileName(&sbsOldAttachmentFileName);
			logd(L"[ReplaceAssignTaskContent]Remove old attachment FileName=%s return %#x", (LPCWSTR)sbsOldAttachmentFileName, hr);
			sTaskFile = sbsOldAttachmentFileName;
		}
	}
	// e.g %LocalAppData%\Temp\CEOutlookAddin\TASK{12345678-1234-1234-12345678}xx.msg
	boost::filesystem::wpath pathAssociatedTaskTmpFile(g_strOETempFolder);
	pathAssociatedTaskTmpFile /= L"TASK" + NLNewGUID() + sTaskFile;

	sTaskFile = pathAssociatedTaskTmpFile.string();
	logd(L"[ReplaceAssignTaskContent]Saved associated task file path: %s", sTaskFile.c_str());

	hr = spAssociatedTaskItem->Save();
	if (FAILED(hr))
	{
		loge(L"[ReplaceAssignTaskContent]Associated task: Save failed! hr=%#x\n", hr);
		return false;
	}
	
	// Save the associated task to the filesystem
	hr = spAssociatedTaskItem->SaveAs(CComBSTR(sTaskFile.c_str()), CComVariant(vtMissing));
	if (FAILED(hr))
	{
		loge(L"[ReplaceAssignTaskContent]Associated task: SaveAs failed! hr=%#x\n", hr);
		return false;
	}

	//Substitute the specific attachment in the request task for the previously saved associated task file

	hr = attachments->Remove(1);
	if(SUCCEEDED(hr))
	{
		CComVariant varSource(sTaskFile.c_str());
		CComVariant varOptional((long)DISP_E_PARAMNOTFOUND, VT_ERROR);
		CComPtr<Outlook::Attachment> spNewAttach = 0;
		hr = attachments->Add(varSource, varOptional, varOptional, varOptional, &spNewAttach);
		if(SUCCEEDED(hr))
		{
			CComBSTR sbsNewAttachmentFileName;
			hr = spNewAttach->get_FileName(&sbsNewAttachmentFileName);
			logd(L"[ReplaceAssignTaskContent]successfully substitute the task attachment in the request task, FileName=%s, hr=%#x", (LPCWSTR)sbsNewAttachmentFileName, hr);
			return true;
		}
		else
		{
			loge(L"[ReplaceAssignTaskContent]failed to add the task attachment, hr=%#x",hr);
			return false;
		}
	}
	else
	{
		loge(L"[ReplaceAssignTaskContent]failed to remove the task attachment, hr=%#x",hr);
		return false;
	}
}




bool CItemEventDisp::UpdateEmailContent(CSendEmailData & EmailData,CComPtr<IDispatch> dspMailItem, BOOL bInline)
{
	if (EmailData.GetRecipientsData().IsRecipientsChanged())
	{
		if (!UpdateRecipents(EmailData, dspMailItem))
		{
			return false;
		}
	}
	if (EmailData.GetSubjectData().IsSubjectChanged())
	{
		logd(L"[UpdateEmailContent]begin change subject.\n");
		UpdateSubject(EmailData,dspMailItem);
	}


	//now we must first update body,then update attachment. if not outlook may crash after returned from onsendex
	if (EmailData.GetBodyData().IsBodyChanged())
	{
		UpdateBody(EmailData,dspMailItem, bInline);
	}


	if (EmailData.HasAttachmentRemoved() || EmailData.HasAttachmentUpdated())
	{
		logd(L"[UpdateEmailContent]Begin to update attachment.");
		if (!UpdateAttachment(EmailData, dspMailItem))
		{
			logd(L"[UpdateEmailContent]Update attachment failed.");
			return false;
		}	
		logd(L"[UpdateEmailContent]Update attachment success.");
	}

	ITEM_TYPE origEmailType = DEFAULT;
	OLUtilities::CheckGetMailItemType(m_spSaveOrigMailItem, origEmailType);
	if (TASK_REQUEST_ITEM == origEmailType)
	{
		return ReplaceAssignTaskContent(dspMailItem);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
static __declspec(noinline) void SetExpInfo(char* filename = __FILE__, int line = __LINE__, bool bNormalReturn = false)
{
    UNREFERENCED_PARAMETER(bNormalReturn);
    CExceptionHandler::SetExpInfo(filename, line);
}

__declspec(noinline) void CItemEventDisp::OnSendEx(VARIANT_BOOL* Cancel, bool bInline)
{
	g_pOutlookObj->WaitDelayInitFinish();
	
    CExceptionHandler* pHandler = CExceptionHandler::GetInstance();
    pHandler->SetProcessExceptionHandlers(0);
    pHandler->SetThreadExceptionHandlers(0);
    __try
    {
        _OnSendEx(Cancel, bInline);
    }
    __except (CExceptionHandler::ExceptionFilter(GetExceptionCode(), GetExceptionInformation()))
    {
        *Cancel = VARIANT_TRUE;
    }
}



void CItemEventDisp::SaveAttachmentDumb(CComPtr<IDispatch> dspMailItem)
{
	CComPtr<Outlook::Attachments> spAttachments = NULL;
	HRESULT hr = MailItemUtility::get_Attachments(dspMailItem, &spAttachments,TRUE);
	if (SUCCEEDED(hr) && (NULL != spAttachments))
	{
		long lAttachCount = 0; 
		hr = spAttachments->get_Count(&lAttachCount);
		if (SUCCEEDED(hr) && lAttachCount > 0)
		{
			for (long lCurIndex = lAttachCount; lCurIndex > 0; lCurIndex--)
			{
				CComPtr<Outlook::Attachment> spAttachment = NULL;
				hr = spAttachments->Item(CComVariant(lCurIndex), &spAttachment);

				if (SUCCEEDED(hr) && (NULL!=spAttachment))
				{
					CComBSTR bstrFile;
					spAttachment->SaveAsFile(bstrFile);
				}
			}
		}
	}
}

void CItemEventDisp::DeleteCustomerProperties(CComPtr<IDispatch> dspMailItem)
{
	CComPtr<Outlook::Attachments> attachments = NULL;
	MailItemUtility::get_Attachments(m_spMailItem, &attachments,TRUE);

	if (attachments)
	{
		long lAttachCount = 0;
		attachments->get_Count(&lAttachCount);
		for (long lAttachIndex = 0; lAttachIndex < lAttachCount; lAttachIndex++)
		{
			CComPtr<Outlook::Attachment> pAttachment;
			CComVariant varAttachIndex = lAttachIndex; 
			attachments->Item(varAttachIndex, &pAttachment);
			if (pAttachment)
			{
				DeleteCustomerPropertiesForAttachment(pAttachment);
			}
		}
	}
}

void CItemEventDisp::DeleteCustomerPropertiesForAttachment(CComPtr<Outlook::Attachment> pAttachment)
{
	CMailPropertyTool::DelAttachmentProperty(pAttachment, PROP_NAME_HCTAG);
	CMailPropertyTool::DelAttachmentProperty(pAttachment, PROP_NAME_PREOUTLOOKTEMPLASTMODIFYTIME);
	CMailPropertyTool::DelAttachmentProperty(pAttachment, PROP_NAME_ATTACHMENT_SENDTIMES);
	CMailPropertyTool::DelAttachmentProperty(pAttachment, PROP_NAME_ATTACHMENT_ISCHANGED);
}

void CItemEventDisp::SyncInformationBeforeUpdateContent(COE_PAMngr* paMngr, CSendEmailData* pEmailData)
{
	CComPtr<Outlook::Attachments> attachments = NULL;
	MailItemUtility::get_Attachments(m_spMailItem, &attachments,TRUE);

	const int nAttachmentNum = (int) pEmailData->GetAttachmentData().size();
	for (int i = 0; i < nAttachmentNum; i++)
	{
		CAttachmentData& attachData = pEmailData->GetAttachmentData()[i];

		//added tags to temp file
		if (attachData.GetHCTagAlreadTagged().size()>0)
		{
			paMngr->TaggingOnFile(g_hTAG, attachData.GetTempPath().c_str(), &attachData.GetHCTagAlreadTagged() );
			logd(L"[SyncInformationBeforeUpdateContent] tag on temp file when send out. %s\n", attachData.GetTempPath().c_str());
		}


		//remove last modify time tag on temp file
		if(!attachData.IsFileNameChangedAfterObligation())
		{
			paMngr->DeleteLastModifyTimeTag(g_hTAG, attachData.GetTempPath().c_str() );
			logd(L"[SyncInformationBeforeUpdateContent] delete last modify time tag when send out. %s\n", attachData.GetTempPath().c_str());
		}

		//delete MAPI property for attachment before send out
		if (attachments)
		{
			CComPtr<Outlook::Attachment> pAttachment;
			CComVariant varAttachIndex = attachData.GetOriginalAttachIndex();
			attachments->Item(varAttachIndex, &pAttachment);
			if (pEmailData->GetAttachmentData()[i].GetAttachmentUpdateProperty(pAttachment) && FALSE == pEmailData->GetAttachmentData()[i].IsAttachmentUpdated())
			{
				logd(L"[SyncInformationBeforeUpdateContent]GetAttachmentUpdateProperty[%d]=%d", i, TRUE);
				pEmailData->SetAttachmentUpdated(i, TRUE);//send two times(cancel fist time), changed by first time;
			}
			else
			{
				logd(L"[SyncInformationBeforeUpdateContent]GetAttachmentUpdateProperty[%d]=%d", i, FALSE);
			}
			if (pAttachment)
			{
				DeleteCustomerPropertiesForAttachment( pAttachment);
			}
		}
	}

	//delete MAPI property for mailitem before sendout
	CMailPropertyTool::SetMailProperty(m_spMailItem, PROP_NAME_MAIL_SENDTIMES, L"");
}

int CItemEventDisp::GetMailSendTimes()
{
	std::wstring wstrOnSendTimes = CMailPropertyTool::GetMailProperty(m_spMailItem, PROP_NAME_MAIL_SENDTIMES);
	if (wstrOnSendTimes.size()>0)
	{
		return wcstol(wstrOnSendTimes.c_str(), NULL, 10);
	}
	else
	{
		DP((L"testx GetMailSendTimes failed. default=0\n"));
		return 0;
	}
}

void CItemEventDisp::SetMailSendTimes(int nSendTimes)
{
	wchar_t wszOnSendTimes[100] = { 0 };
	wsprintfW(wszOnSendTimes, L"%d", nSendTimes);
	CMailPropertyTool::SetMailProperty(m_spMailItem, PROP_NAME_MAIL_SENDTIMES, wszOnSendTimes);
}

BOOL CItemEventDisp::CopyMessageHeaderTo(IDispatch *pItemDisp)
{
	logi(L"CopyMessageHeaderTo(%p)\n", pItemDisp);
	ATL::CComVariant varPropertyAccessor;

	// OriginItem.PropertyAccessor
	HRESULT hr = m_spMailItem.GetPropertyByName(W2COLE(L"PropertyAccessor"), &varPropertyAccessor);
	if (FAILED(hr) || VT_DISPATCH != varPropertyAccessor.vt)
	{
		loge(L"[CopyMessageHeaderTo]GetPropertyByName(PropertyAccessor) return %#x\n", hr);
		return FALSE;
	}

	// MessageHeader = OriginItem.PropertyAccessor.GetProperty(PR_);
	ATL::CComPtr<Outlook::_PropertyAccessor> spPropertyAccessor((Outlook::_PropertyAccessor*)varPropertyAccessor.pdispVal);
	ATL::CComBSTR sbsMessageHeader(L"http://schemas.microsoft.com/mapi/proptag/0x007D001F");
	ATL::CComVariant varMessageHeader;
	hr = spPropertyAccessor->GetProperty(sbsMessageHeader, &varMessageHeader);
	if (FAILED(hr) || VT_BSTR != varMessageHeader.vt || NULL == varMessageHeader.bstrVal)
	{
		 // maybe not exist if the destination item is newly created by New Email rather than Reply/ReplyAll/Forward
		logd(L"[CopyMessageHeaderTo]_PropertyAccessor.GetProperty return %#x, BSTR@%#x\n", hr, varMessageHeader.bstrVal);
		return FALSE;
	}

	//NewItem.UserProperties
	ATL::CComPtr<IDispatch> spDestItem(pItemDisp);
	ATL::CComVariant varUserProperties;
	hr = spDestItem.GetPropertyByName(W2COLE(L"UserProperties"), &varUserProperties);
	if (FAILED(hr) || VT_DISPATCH != varUserProperties.vt)
	{
		logd(L"[CopyMessageHeaderTo]GetPropertyByName(UserProperties) return %#x\n", hr);
		return FALSE;
	}

	//UserProperty = NewItem.UserProperties.Add(L"OriginMessageHeader")
	ATL::CComPtr<Outlook::UserProperties> spUserProperties((Outlook::UserProperties*)varUserProperties.pdispVal);
	ATL::CComBSTR sbsPropertyName(P_NAME_ORIGIN_MESSAGE_HEADER_W);
	ATL::CComPtr<Outlook::UserProperty> spUserProperty;
	ATL::CComVariant addToFolderField(VARIANT_TRUE), varOptional((long)DISP_E_PARAMNOTFOUND, VT_ERROR);
	hr = spUserProperties->Add(sbsPropertyName, Outlook::OlUserPropertyType::olText, addToFolderField, varOptional, &spUserProperty);
	if (FAILED(hr) || NULL == spUserProperty)
	{
		logd(L"[CopyMessageHeaderTo]UserProperties.Add return %#x\n", hr);
		return FALSE;
	}

	//UserProperty.Value = MessageHeader
	hr = spUserProperty->put_Value(varMessageHeader);

	logd(L"[CopyMessageHeaderTo] put user property about header return HRESULT=%#x\n", hr);

	CMessageHeader::ResetNLIncrementalHeaderProperty(pItemDisp);

	return SUCCEEDED(hr);
}

void CItemEventDisp::OnPreviewAttachment(IDispatch * lpDispItem,VARIANT_BOOL* Cancel)
{
	if (lpDispItem != NULL)
	{
		CComPtr<Outlook::Attachment> spAttachment = NULL;
		HRESULT hr = lpDispItem->QueryInterface(IID_Attachment,(void**)&spAttachment);
		if (SUCCEEDED(hr)&&spAttachment)
		{
			CAttachmentFileMgr& theAttachmentFileMgr = CAttachmentFileMgr::GetInstance();

			std::wstring wstrRealSourceFileFullPath  = L"";
			std::wstring wstrOETempFileFullPath = L"";
			bool bRet = theAttachmentFileMgr.GetAttachmentFilePath(spAttachment, wstrRealSourceFileFullPath, wstrOETempFileFullPath, emAttachmentFromUnknown, MAIL_ITEM);
			BOOL bAllow = TRUE;
			if (bRet && (!wstrOETempFileFullPath.empty()))
			{
				if (g_spPolicyCommunicator==NULL)
				{
					g_spPolicyCommunicator = PolicyCommunicator::CreateInstance();
				}
				bAllow = g_spPolicyCommunicator->QueryOutlookPreviewPolicy(wstrOETempFileFullPath.c_str());
				if (!bAllow)
				{
					* Cancel = TRUE;
				}

			}

			logd(L"[OnPreviewAttachment]RealSourceFileFullPath=%s, OETempFileFullPath=%s, Cancel=%d, bAllow=%d, GetAttachmentFilePath returned %d", 
				wstrRealSourceFileFullPath.c_str(), wstrOETempFileFullPath.c_str(), *Cancel, bAllow, bRet);
			return;
		}
	}
	logd(L"[OnPreviewAttachment]Failed, Cancel=%d",*Cancel);
}

//////////////////////////////////////////////////////////////////////////

// @param bInline A switch to pop up a window activating an inspector window by bringing it to the foreground and setting keyboard focus.
BOOL CItemEventDisp::DoEnforcement(CSendEmailData *pOuterEmailData, CComPtr<IDispatch> spItem, VARIANT_BOOL* Cancel, bool bInline, ITEM_TYPE origEmailType)
{
	CSendEmailData emailData;
	CAttachmentObligationData  AttachmentOblData;
	std::map<long,std::wstring> map3thAppAttachmentPath;

	Init(spItem, emailData, AttachmentOblData, bInline);

	bool bGetEmailData = emailData.GetDataFromMailItem(spItem, map3thAppAttachmentPath, origEmailType,TRUE);

	logd(L"[DoEnforcement]GetDataFromMailItem return %d\n", bGetEmailData);

	if (!bGetEmailData)
	{
		return FALSE;
	}

	CollectAllOETempFileInfo(emailData);


	std::vector<CAttachmentData>& refVecAttachmentData = emailData.GetAttachmentData(); 
	logd(L"Attachment Count = %d\n", refVecAttachmentData.size());

	std::vector<CAttachmentData>::iterator itAttachment = emailData.GetAttachmentData().begin();
	for ( ; itAttachment != refVecAttachmentData.end(); ++itAttachment)
	{
		if (!itAttachment->IsAttachmentRemoved())
		{
			LPCWSTR pszTmpPath = itAttachment->GetTempPath().c_str();
			logd(L"Attachment: GetTempPath = %s, Exists=%d\n", pszTmpPath, PathFileExistsW(pszTmpPath));
		}
	}
	std::map<long,std::wstring>::iterator attachmentIterator = map3thAppAttachmentPath.begin();
	for ( ; attachmentIterator != map3thAppAttachmentPath.end(); ++attachmentIterator)
	{
		logd(L"The attached file: %s\n", attachmentIterator->second.c_str());
	}
	//ignore the msg recipients and sender, using the original recipients and sender
	//if (!emailData.GetRecipientsData().GetRealRecipentsNum())
	//{
	if (pOuterEmailData)
	{
		emailData.GetRecipientsData() = pOuterEmailData->GetRecipientsData();
		emailData.GetSender() = pOuterEmailData->GetSender();
	}
		//logd(L"RealRecipentsNum = %d\n", emailData.GetRecipientsData().GetRealRecipentsNum());
	//}

	//Package Policy Request
	CQueryPCInfo queryPCInfo;
	INT intQueryTime = 0;
	while((emailData.GetNeedToInheritHeader() && 1 == intQueryTime) || (!emailData.GetNeedToInheritHeader() && 0 == intQueryTime))
	{
		emailData.SetInheritHeader(TRUE);
		bool bConstructRequest = queryPCInfo.ConstructRequestFromEmailData(emailData, spItem);
		if (!bConstructRequest)
		{
			loge(L"Failed to construct PC request\n");
			return FALSE;
		}

		//Query policy
		if (g_spPolicyCommunicator)
		{
			BOOL bConnect = FALSE;
			BOOL bQueryPC = g_spPolicyCommunicator->QueryPolicy(queryPCInfo, bConnect);
			ActionOnQueryPCFinish(emailData, Cancel, bInline, bQueryPC==TRUE ? true : false);
			if (!bQueryPC)
			{
				loge(L"Failed to query PC\n");
				return FALSE;
			}
		}

		COE_PAMngr paMngr;
		ClassifyObligation(emailData, queryPCInfo, paMngr, spItem);
		intQueryTime++;
	}

	// Bug 43974 - [oe8.5]only show one attachment name in alert message if deny two MSG file 
	pOuterEmailData->PutAlertMessages(emailData);

	return emailData.GetAllow();
}


//assign task or task request,it is not the same as task/task request or othe item forward
bool CItemEventDisp::ParseAssignTaskItem(const wchar_t *pTaskItemPath, CSendEmailData *pTaskData, CComPtr<IDispatch> &taskItem, bool bInline, ITEM_TYPE origEmailType)
{
	HRESULT hr;
	CAttachmentObligationData  attachmentOblData;
	//std::map<long,std::wstring> map3thAppAttachmentPath;
	//CComBSTR bstrMAPI(L"MAPI");
	//CComPtr<Outlook::_NameSpace> spNameSpace;
	//hr = g_pOutlookObj->get_App()->GetNamespace(bstrMAPI, &spNameSpace);
	//LPCWSTR pszSourcePath = pTaskItemPath;
	CComBSTR sbsAttachmentFilePath(pTaskItemPath);
	//CComPtr<IDispatch> spItem;
	//hr = spNameSpace->OpenSharedItem(sbsAttachmentFilePath, &taskItem);
	CComVariant comVarOptional((long)DISP_E_PARAMNOTFOUND, VT_ERROR); 

	hr = g_pOutlookObj->get_App()->CreateItemFromTemplate(sbsAttachmentFilePath, comVarOptional, &taskItem);//pay attention to 2th, 3th parameter
	if(FAILED(hr) || NULL == taskItem)
	{
		loge(L"%#x = O,parse assign task CreateItemFromTemplate on %s failed\n", hr, pTaskItemPath);
		return false;
	}
	else 
	{
		//logi(L"===>parse task attachmentOblData =0x%x\n", &attachmentOblData);
		Init(taskItem, *pTaskData, attachmentOblData, bInline);
		//logi(L"===>parse task pTaskData->m_AttachmentOblData =0x%x\n", pTaskData->m_AttachmentOblData);
		pTaskData->GetDataFromMailItem(taskItem, m_map3thAppAttachmentSourcePath, origEmailType, TRUE);//m_map3thAppAttachmentSourcePath

		logd(L"%#x = O,parse assign task CreateItemFromTemplate on %s successed\n", hr, pTaskItemPath);
		return true;
	}
	return false;
}
void CItemEventDisp::_OnSendEx(VARIANT_BOOL* Cancel, bool bInline)
{
	NLTRACER_CREATEINSTANCE(true, true, L"Begin send email");
	logd(L"Enter CItemEventDisp::OnSendEx.\n");

	PrintRunningInfo(g_pOutlookObj->get_App());

	CEventLog::SetInitTime(); 
	if(NULL == m_spMailItem) return;
	ITEM_TYPE origEmailType = DEFAULT;
	OLUtilities::CheckGetMailItemType(m_spMailItem,origEmailType);
	logd(L"m_spMailItem=%x", m_spMailItem);
	if (!CMailPropertyTool::OESupportMailType(origEmailType))
	{
		loge(L"The email type is not supported.\n");
		return;	
	}
	else
	{
		logd(L"The original mail type = %d\n", origEmailType);
	}

	//update mail send times
	int nMailSendTimes = GetMailSendTimes();
	nMailSendTimes++;
	if (1==nMailSendTimes)
	{
       DeleteCustomerProperties(m_spMailItem);
	}
    SetMailSendTimes(nMailSendTimes);

	//save email first, because we need to access MAPI data. if not saved, the data is in outlook buffer not flush to MAPI data.
	MailItemUtility::Save(m_spMailItem);

#if defined(WSO2K10)
	//in outlook2010, when you forward an email contains an embed image and a file attachment, then added a file attachment, 
	//the index of the attachment may change after outlook::attachment::SaveAsFile called.
	//so we called SaveAsFile here to keep the index of attachment don't be changed during the following process.
	SaveAttachmentDumb(m_spMailItem);
#endif

	//Init
	if (g_spPolicyCommunicator==NULL)
	{
		g_spPolicyCommunicator = PolicyCommunicator::CreateInstance();
	}
    SetExpInfo(__FILE__,__LINE__);
	
	//get email data
	CSendEmailData emailData;
	//CSendEmailData assignTaskData;//get the assigned task data
	CComPtr<Outlook::_TaskItem> spAssociatedTaskItem = 0;
	CAttachmentObligationData  AttachmentOblData;

	if (TASK_REQUEST_ITEM == origEmailType)
	{
		CComPtr<Outlook::_TaskRequestItem> spCurTaskReqItem = 0;
		HRESULT hr = m_spMailItem->QueryInterface(Outlook::IID__TaskRequestItem, (void**)&spCurTaskReqItem);
		if(SUCCEEDED(hr) && spCurTaskReqItem)
		{
			logd(L"[_OnSendEx]get task request item successfully!");
			hr = spCurTaskReqItem->GetAssociatedTask( TRUE, &spAssociatedTaskItem ) ;
			if(SUCCEEDED(hr) && spAssociatedTaskItem)
			{
				m_spSaveOrigMailItem.operator = ( m_spMailItem );
				m_spMailItem.operator =( spAssociatedTaskItem );
				logd(L"[_OnSendEx]get the associated task successfully! spCurTaskItem = %p, %p\n", spAssociatedTaskItem, spAssociatedTaskItem.p);
			}
		}
	}

    SetExpInfo(__FILE__, __LINE__);

	Init(m_spMailItem, emailData, AttachmentOblData, bInline);
    SaveCurrentActiveWindow(); // FixBug 35118

    SetExpInfo(__FILE__, __LINE__);
	
	bool bGetEmailData = emailData.GetDataFromMailItem(m_spMailItem, m_map3thAppAttachmentSourcePath, origEmailType, FALSE);

	/* 
	 * for appointment/meeting, oe 8.5 delete the recipients which are equal sender 
	 * and don't set recipients changed variable
	*/
	//can't estimate the influence, so we can't delete the recipients which are equl sender directly
	//emailData.DeleteOrganigerForMeeting(emailData);
	m_origEmailData = &emailData;//used for saving assign task 
	std::map<long,std::wstring>::iterator attachmentIterator = m_map3thAppAttachmentSourcePath.begin();
	for ( ; attachmentIterator != m_map3thAppAttachmentSourcePath.end(); ++attachmentIterator)
	{
		logd(L"The 3thApp attached file: %s\n", attachmentIterator->second.c_str());
	}

	std::vector<CAttachmentData> refVecAttachmentData  = emailData.GetAttachmentData();
	logd(L"[_OnSendEx]Attachment Count = %d\n", refVecAttachmentData.size());	
	
	BOOL bAggregatedAllow = TRUE;
	
	if(refVecAttachmentData.size())//do enforcement for email or assign task's attachments with .msg, .ics, or .vcf format
	{
		HRESULT hr;
		std::vector<CAttachmentData>::iterator itAttachment = refVecAttachmentData.begin();
		for (;itAttachment != refVecAttachmentData.end();itAttachment++)
		{
			if (!itAttachment->IsAttachmentRemoved())
			{
				CComPtr<IDispatch> spItem;
				std::wstring path = itAttachment->GetTempPath();
				std::wstring  extensionNmae = path.substr(path.length()-3, 3);
				if ( 0 != extensionNmae.compare(L"msg") && 0 != extensionNmae.compare(L"vcf") && 0 != extensionNmae.compare(L"ics") ){
					loge(L"The attachment is not .msg, .vcf or .ics, let's continue\n", hr, itAttachment->GetTempPath().c_str());
					continue;
				}
				CComBSTR sbsAttachmentFilePath(itAttachment->GetTempPath().c_str());
				CComVariant comVarOptional((long)DISP_E_PARAMNOTFOUND, VT_ERROR); 
				hr = g_pOutlookObj->get_App()->CreateItemFromTemplate(sbsAttachmentFilePath, comVarOptional, &spItem);
				if(FAILED(hr) || NULL == spItem)
				{
					loge(L"%#x = O,CreateItemFromTemplate on %s failed\n", hr, itAttachment->GetTempPath().c_str());
					continue;
				}
				logd(L"%#x = O,CreateItemFromTemplate on %s success\n, spItem = %x", hr, itAttachment->GetTempPath().c_str(), spItem);
				ITEM_TYPE attachmentItemType = DEFAULT;
				OLUtilities::CheckGetMailItemType(spItem, attachmentItemType);

				//if ( (attachmentItemType != MAIL_ITEM) && (attachmentItemType != MEETING_ITEM) && (attachmentItemType != APPOINTMENT_ITEM))
				if (!CMailPropertyTool::OESupportMailType(attachmentItemType))
				{

					continue;
				}
	
				MailItemUtility::Save(spItem);
				bool bTempAllow = DoEnforcement(&emailData, spItem, Cancel, TRUE, origEmailType); //may be this "emaildata" should be changed to assignTaskData
				if (IsMsgFile(path.c_str()))
				{
					MailItemUtility::DeleteMailItem(spItem);
				}

				bAggregatedAllow = bAggregatedAllow && bTempAllow;
			}
		}
	}

	if (!bAggregatedAllow)
	{
		logd(L"[_OnSendEx]bAggregatedAllow=%d", bAggregatedAllow);
		emailData.SetAllow(false);
		ExecuteRichAlertMessage(&emailData, Cancel);
		*Cancel = VARIANT_TRUE;
		return;
	}

	/*
		finished to do enforcement for .msg,  .vcf, .ics attachment
	*/
	
	//distinguish between normal email and assign task
	CSendEmailData *pEmailDataToHandle = &emailData;
	CComPtr<IDispatch> emailItemToHandle = m_spMailItem;

	/*
		From here OE 8.5 changes all "emailData" "m_spMailItem" to pEmailDataToHandle,emailItemToHandle
	*/
	SetExpInfo(__FILE__, __LINE__);
	if (!bGetEmailData)
	{
        SetExpInfo(__FILE__, __LINE__,true);
		return;
	}
    SetExpInfo(__FILE__, __LINE__);
    //CollectAllOETempFileInfo(emailData);
	CollectAllOETempFileInfo(*pEmailDataToHandle);

	COE_PAMngr paMngr;

	bool bChangedContent = true;
	EXCUTEOBLACTION ExRet = EXCUTESUCCESS;
	while(bChangedContent)
	{
        SetExpInfo(__FILE__, __LINE__);
		//emailData.SetEvalAgain(false);
		pEmailDataToHandle->SetEvalAgain(false);

		//Package Policy Request
		CQueryPCInfo queryPCInfo;
        SetExpInfo(__FILE__, __LINE__);
		//bool bConstructRequest = queryPCInfo.ConstructRequestFromEmailData(emailData,m_spMailItem);
		bool bConstructRequest = queryPCInfo.ConstructRequestFromEmailData(*pEmailDataToHandle, emailItemToHandle);
		if (!bConstructRequest)
		{
            SetExpInfo(__FILE__, __LINE__,true);
			return;
		}

		//Query policy
		if (g_spPolicyCommunicator)
		{
			BOOL bConnect= FALSE;
            SetExpInfo(__FILE__, __LINE__);
			BOOL bQueryPC=g_spPolicyCommunicator->QueryPolicy(queryPCInfo, bConnect);
			//ActionOnQueryPCFinish(emailData, Cancel, bInline, bQueryPC==TRUE?true:false);
			ActionOnQueryPCFinish(*pEmailDataToHandle, Cancel, bInline, bQueryPC==TRUE?true:false);
			if (!bQueryPC)
			{
				logd(L"After ActionOnQueryPCFinish, bQueryPC = %d, Cancel=%d", bQueryPC, *Cancel);
				return;
			}
		}


		//Classify Obligation
        SetExpInfo(__FILE__, __LINE__);
		//ClassifyObligation(emailData,queryPCInfo,paMngr, m_spMailItem);
		ClassifyObligation(*pEmailDataToHandle, queryPCInfo, paMngr, emailItemToHandle);
        SetExpInfo(__FILE__, __LINE__);
        NLTRACER_SETRUNTIMEINFO(NULL);
        //ExRet = ExcuteOblgation(emailData,g_spPolicyCommunicator,m_spMailItem,paMngr);
		ExRet = ExcuteOblgation(*pEmailDataToHandle, g_spPolicyCommunicator, emailItemToHandle, paMngr);
        SetExpInfo(__FILE__, __LINE__);
        NLTRACER_SETRUNTIMEINFO(NULL);
		if (ExRet == EXCUTESUCCESS)
		{
            SetExpInfo(__FILE__, __LINE__);
			//bChangedContent = ChangedContent(emailData,paMngr);
			bChangedContent = ChangedContent(*pEmailDataToHandle, paMngr);
            SetExpInfo(__FILE__, __LINE__);
		}
		else
		{
			break;
		}
	}

	// If Inherit_XHeader and REJECT_UNLESS_SILENT_OVERRIDE coexist, Click OK of REJECT_UNLESS_SILENT_OVERRIDE to set allow to true but Inherit_XHeader
	// will lead to eval again, so the second classification won't pop up the REJECT_UNLESS_SILENT_OVERRIDE dialog but allow will be false
	if (pEmailDataToHandle->GetEvaAgainTimes() && pEmailDataToHandle->AllowOfRejectUnlessSilent())
	{
		logd(L"Since the user clicked OK of the REJECT_UNLESS_SILENT_OVERRIDE dialog for the first Eva, need to force to reset allow to true finally");
		pEmailDataToHandle->SetAllow(true);
	}

	// Apply the preceding processing to auto-tagging or HC-tagging on header into the email
	if(!pEmailDataToHandle->GetMessageHeader().UpdateEmailHeaders(m_spMailItem))
	{
		logd(L"Failed to execute UpdateEmailHeaders ");
		pEmailDataToHandle->SetAllow(false);
	}

    SetExpInfo(__FILE__, __LINE__);

	BOOL isCanceledOrFailed = EXCUTE_CANCEL_MAYBE_LOG_DESIGN == ExRet; // canceled by the user or executing failed.

	//logi(L"===>pEmailDataToHandle->GetAllow() = %d\n", pEmailDataToHandle->GetAllow());
	if(/*emailData.GetAllow()*/pEmailDataToHandle->GetAllow())
	{
        NLTRACER_SETRUNTIMEINFO(NULL);
        SetExpInfo(__FILE__, __LINE__);
		bool bProcess = AttachmentOblData.ExcuteWarnMsgEx(/*emailData*/*pEmailDataToHandle);
        SetExpInfo(__FILE__, __LINE__);
        NLTRACER_SETRUNTIMEINFO(NULL);
		if (!bProcess) //cancel After Warning
		{
			logd(L"ExcuteWarnMsgEx: user clicked the cancel button, old isCanceledOrFailed=%d", isCanceledOrFailed);
			*Cancel = VARIANT_TRUE;
			isCanceledOrFailed = TRUE;
		}
#if 0
		else // proceed after warning or no warning obligation
		{
			SyncInformationBeforeUpdateContent(&paMngr, /*&emailData*/pEmailDataToHandle);
			
			AttachmentOblData.ExcuteHCReportLogEx(/*emailData*/*pEmailDataToHandle);
			if (!UpdateEmailContent(/*emailData*/*pEmailDataToHandle,/*m_spMailItem*/emailItemToHandle, bInline))
			{
				MessageBoxW(emailData.GetWnd()/*pEmailDataToHandle->GetWnd()*/, L"Failed to update email content. please try again.", MESSAGE_TITLE, MB_OK);
				*Cancel = VARIANT_TRUE;
			}
			SetExpInfo(__FILE__, __LINE__);
		}
#endif

	}
	else
	{
		*Cancel = VARIANT_TRUE;
	}

	logd(L"Email before determining alert: allowed=%d, Cancel=%d, ExRet?%d", pEmailDataToHandle->GetAllow(), *Cancel, ExRet);
	
	if (!isCanceledOrFailed)
	{
		ExecuteRichAlertMessage(pEmailDataToHandle, Cancel);
	}
	if (VARIANT_TRUE != *Cancel){
		logi(L"Prepare to update content, get allow=%d", pEmailDataToHandle->GetAllow());
		SyncInformationBeforeUpdateContent(&paMngr, /*&emailData*/pEmailDataToHandle);
		AttachmentOblData.ExcuteHCReportLogEx(/*emailData*/*pEmailDataToHandle);
		if (!UpdateEmailContent(/*emailData*/*pEmailDataToHandle,/*m_spMailItem*/emailItemToHandle, bInline))
		{
			MessageBoxW(emailData.GetWnd()/*pEmailDataToHandle->GetWnd()*/, L"Failed to update email content. please try again.", MESSAGE_TITLE, MB_OK);
			*Cancel = VARIANT_TRUE;
		}
		SetExpInfo(__FILE__, __LINE__);
	}

}

/*
Note: Some obligation(s), for example, Reject Unless Silent Override, will affect the Rich Alert Message 

+-------------+------------------------------------------+---------------------------------+-----------------+------------------------------+
| Enforcement | After obligation executed                | Rich Alert Message              | Expectation     | Conclusion                   |
+             +------------------------------------------+---------------------------------+                 +                              +
|             | If Reject Unless Silent Override exists? | Based on | Dialog Button(s)     |                 |                              |
+-------------+------------------------------------------+----------+----------------------+-----------------+------------------------------+
| deny        | Send email (reset allow)                 | allow    | Proceed              | Allowed to send | compromised, allowed to send |
|             |                                          |          | Cancel               | Denied to send  | conflict, denied to send     |
+-------------+------------------------------------------+----------+----------------------+-----------------+------------------------------+
| allow       | Send email (reset allow)                 | allow    | Proceed              | Allowed to send | consistent, allowed to send  |
|             |                                          |          | Cancel               | Denied to send  | conflict, denied to send     |
+             +------------------------------------------+----------+----------------------+-----------------+------------------------------+
|             | Cancel (set deny)                        | deny     | OK (no deny message) | Denied to send  | compromised, denied to send  |
+-------------+------------------------------------------+----------+----------------------+-----------------+------------------------------+
*/
BOOL CItemEventDisp::ExecuteRichAlertMessage(CSendEmailData *pEmailDataToHandle, VARIANT_BOOL* Cancel)
{
	logd(L"[ExecuteRichAlertMessage]BGN!");

	std::wstring displayAllowedMessage = theCfg[L"RichAlertMessage"][L"DisplayAllowedMessagesEvenIfDenied"];
	BOOL displayAllowIfDenied = !displayAllowedMessage.empty();


#define GET_SIZE(t) pEmailDataToHandle->GetAllowed##t##Messages().GetMessages().size(), pEmailDataToHandle->GetDenied##t##Messages().GetMessages().size()
	logd(L"[ExecuteRichAlertMessage]nAllowRecipient=%d, nDenyRecipient=%d, nAllowAttRecipient=%d, nDenyAttRecipient=%d, nAllowAtt=%d, nDenyAtt=%d" 
		L", nAllowOther=%d, nDenyOther=%d", GET_SIZE(Recipient), GET_SIZE(AttachmentRecipient), GET_SIZE(Attachment), GET_SIZE(Other));
#undef GET_SIZE

	pEmailDataToHandle->RemoveTrivialAlertMessages();

	// the two message collection about recipient should be merged
	AlertMsgs deniedRecipientAlertMsgs(pEmailDataToHandle->GetDeniedRecipientMessages(), false, AlertMsg::RECIPIENT);
	deniedRecipientAlertMsgs.addRecipientMessages(pEmailDataToHandle->GetDeniedAttachmentRecipientMessages(), false);

	AlertMsgs deniedAttachmentAlertMsgs(pEmailDataToHandle->GetDeniedAttachmentMessages(), false, AlertMsg::ATTACHMENT);
	//AlertMsgs deniedOtherAlertMsgs(pEmailDataToHandle->GetDeniedOtherMessages(), false, AlertMsg::OTHER);

	AlertMsgs allAlertMsgs;
	
	// First, integrate denied alert messages and evaluate the enforcement result

	allAlertMsgs.add(deniedRecipientAlertMsgs);
	allAlertMsgs.add(deniedAttachmentAlertMsgs);
	allAlertMsgs.addUniqueMessagesWithoutItems(pEmailDataToHandle->GetDeniedOtherMessages(), false); // not allow duplicated message text

	const int deniedCount = allAlertMsgs.size();
	const bool currentAllowValue = pEmailDataToHandle->GetAllow();

	// Then, integrate allowed alert messages: 1, allowed; 2 denied and display allowed messages

	AlertMsgs allowedAlertMsgs;

	if (currentAllowValue)
	{
		// when allowed, show message with text and its detail items

		AlertMsgs allowedRecipientAlertMsgs(pEmailDataToHandle->GetAllowedRecipientMessages(), true, AlertMsg::RECIPIENT);
		allowedRecipientAlertMsgs.addRecipientMessages(pEmailDataToHandle->GetAllowedAttachmentRecipientMessages(), true);

		AlertMsgs allowedAttachmentAlertMsgs(pEmailDataToHandle->GetAllowedAttachmentMessages(), true, AlertMsg::ATTACHMENT);

		allowedAlertMsgs.add(allowedRecipientAlertMsgs);
		allowedAlertMsgs.add(allowedAttachmentAlertMsgs);
	}else if(displayAllowIfDenied)
	{
		// When denied, display allowed messages except for those denied messages, but only including message text.

		allowedAlertMsgs.addUniqueMessagesWithoutItems(pEmailDataToHandle->GetAllowedRecipientMessages());
		allowedAlertMsgs.addUniqueMessagesWithoutItems(pEmailDataToHandle->GetAllowedAttachmentRecipientMessages());
		allowedAlertMsgs.addUniqueMessagesWithoutItems(pEmailDataToHandle->GetAllowedAttachmentMessages());
	}
	allowedAlertMsgs.addUniqueMessagesWithoutItems(pEmailDataToHandle->GetAllowedOtherMessages());
	allAlertMsgs.add(allowedAlertMsgs);

	const int allowedCount = allAlertMsgs.size() - deniedCount;
	logd(L"[ExecuteRichAlertMessage]EmailData(aggregatedAllow=%d, allow=%d), RichAlertMessage(deniedCount=%d, allowedCount=%d), displayAllowIfDenied=%d", 
		pEmailDataToHandle->GetAllowBegin(), pEmailDataToHandle->GetAllow(), deniedCount, allowedCount, displayAllowIfDenied);

	BOOL hasAlertMessage = (0 < allAlertMsgs.size());
	BOOL allowedToSend = (*Cancel == VARIANT_FALSE);

	if(hasAlertMessage)
	{
		// Allowed view:  IDOK = _T("Proceed"), IDCANCEL = _T("Cancel")
		// Denied view:   IDOK = _T("OK")
		CDlgAlertMessage dlgAlert(IDD_ALERT_MESSAGE_BOX, currentAllowValue, allAlertMsgs);

		int nDlgRet = dlgAlert.DoModal(pEmailDataToHandle->GetWnd());

		if (IDCANCEL == nDlgRet)
		{
			logd(L"[ExecuteRichAlertMessage]The user choosed CANCEL when poping up the warning alert dialog");
			pEmailDataToHandle->SetAllow(false);
			*Cancel = VARIANT_TRUE;
		}
	}

	logd(L"[ExecuteRichAlertMessage]END! allowedToSend = %d", allowedToSend);

	return hasAlertMessage;
}

void CItemEventDisp::SaveCurrentActiveWindow()
{
    // Check current mail item type
    ITEM_TYPE emItemType = DEFAULT;
    OLUtilities::CheckGetMailItemType(m_spMailItem, emItemType);
    NLPRINT_DEBUGVIEWLOG(L"KimTest: Dispatch:[0x%p], Type:[%d]\n", m_spMailItem.p, emItemType);

    // Check current active window
    CItemEventDisp::s_hCurActiveWindow = GetActiveWindow();
    NLPRINT_DEBUGVIEWLOG(L"KimTest: hCurActiveWindow:[0x%p]\n", CItemEventDisp::s_hCurActiveWindow);
}

void CItemEventDisp::OnSend(VARIANT_BOOL* Cancel, bool bInline)
{
    UNREFERENCED_PARAMETER(Cancel);
    UNREFERENCED_PARAMETER(bInline);
}

void CItemEventDisp::OnWrite(VARIANT_BOOL* Cancel)
{
    //UNREFERENCED_PARAMETER(Cancel);
	ITEM_TYPE itemType  = DEFAULT ;
	if( OLUtilities::CheckGetMailItemType( m_spMailItem,  itemType )  == TRUE && itemType == TASK_ITEM )
	{
		*Cancel = m_bCancel ;
		m_bCancel = FALSE ;
		if( (*Cancel) == TRUE)
		{
			CComPtr<Outlook::_TaskItem> spCurTaskItem = 0;
			CComPtr<Outlook::_TaskItem> spCopy = 0 ;
			HRESULT hr = m_spMailItem->QueryInterface(Outlook::IID__TaskItem, (void**)&spCurTaskItem);
			if(SUCCEEDED(hr) && spCurTaskItem)
			{
				hr=	spCurTaskItem->Copy( (IDispatch**)&spCopy) ;
 			
				if(SUCCEEDED(hr) && spCopy)
				{
					DPW((L"Deny CreateItemFromTemplate  item OnSend")) ;
					BSTR strDelegateor ;
					hr = spCurTaskItem->get_Delegator(&strDelegateor) ;
					if(SUCCEEDED(hr) && strDelegateor==NULL)
					{
						hr = spCurTaskItem->Delete();
					}
					if(m_PCCancel == FALSE|| MessageBoxW( NULL,L"Task assignment prevented by policy. Would you like to re-open the task for editing?",L"Outlook Enforcer", MB_YESNO |MB_ICONINFORMATION) ==IDYES)
					{
						VARIANT bShow;
						bShow.vt = VT_BOOL ;
						bShow.boolVal = FALSE ;
						hr = spCopy->Display(bShow);
						if(SUCCEEDED(hr) )
						{
							DPW((L"Deny spCurTaskItem Display Success")) ;
						}
					}
				}
			}
		}
	}
}

void CItemEventDisp::OnBeforeCheckNames(VARIANT_BOOL* Cancel)
{
    UNREFERENCED_PARAMETER(Cancel);
}

static HRESULT SetTempFilePath(Attachment* Attachment, std::wstring& strRealFilePath, std::wstring& strTempFilePath, BOOL* pbInWordTempFolder)
{
    std::wstring    strFileName;
	HRESULT         hr = S_OK;

	*pbInWordTempFolder	= FALSE;
	strRealFilePath = L"";
	strTempFilePath = L"";

    strFileName = OLUtilities::GetAttachFileName(Attachment);
	if(!strFileName.empty())
	{
        std::wstring strCombPath;

        //
        // Get file path
        //
		long nFileSize = 0;
		Attachment->get_Size(&nFileSize);
		g_RealFileCache.FindByName(strFileName.c_str(), strRealFilePath, nFileSize);
		
		g_TempFileCache.FindByName(strFileName.c_str(), strTempFilePath);

 		if (strRealFilePath.empty())
 			strRealFilePath.assign(strTempFilePath.c_str());  
        //
        // Compose path string
        //
        strCombPath = strTempFilePath;
        strCombPath += TEMP_MAGIC_NAME;
        strCombPath += strRealFilePath;

		DP((L"File Name: '%s'\n", strFileName.c_str()));
		DP((L"Real path: '%s'\n", strRealFilePath.c_str()));
		DP((L"Temp path: '%s'\n", strTempFilePath.c_str()));
		DP((L"Comb path: '%s'\n", strCombPath.c_str()));

		const WCHAR* pwzExt = NULL;
		WCHAR wzLocalSettingsPath[MAX_PATH+1];  memset(wzLocalSettingsPath, 0, sizeof(wzLocalSettingsPath));
		if(SHGetSpecialFolderPath(NULL, wzLocalSettingsPath, CSIDL_INTERNET_CACHE, FALSE))
		{
#if WSO2K3
			WCHAR* pwzTmp = wcsrchr(wzLocalSettingsPath, L'\\');
			if(pwzTmp)
				*(pwzTmp+1) = 0;
			wcsncat_s(wzLocalSettingsPath, MAX_PATH+1, L"Temp\\~WRD", _TRUNCATE);
#else
			wcsncat_s(wzLocalSettingsPath, MAX_PATH+1, L"\\Content.Word\\", _TRUNCATE);
#endif
			DP((L"Temp word path: %s\n", wzLocalSettingsPath));
		}
		if(0==strRealFilePath.length() && 0==strTempFilePath.length())
		{
			*pbInWordTempFolder = TRUE;
		}
#if WSO2K3
		else if(!strRealFilePath.empty() && strTempFilePath.empty() && 0==_wcsnicmp(strRealFilePath.c_str(), wzLocalSettingsPath, wcslen(wzLocalSettingsPath)))
		{
			pwzExt = wcsrchr(strRealFilePath.c_str(), L'.');
			if(pwzExt && 0==_wcsicmp(pwzExt, L".MSG"))
				*pbInWordTempFolder = TRUE;
		}
#else
		else if(!strRealFilePath.empty() && strTempFilePath.empty() && 0==_wcsnicmp(strRealFilePath.c_str(), wzLocalSettingsPath, wcslen(wzLocalSettingsPath)))
		{
			pwzExt = wcsrchr(strRealFilePath.c_str(), L'.');
			if(pwzExt && 0==_wcsicmp(pwzExt, L".MSG"))
				*pbInWordTempFolder = TRUE;
		}
#endif
		else
		{
			// do nothing
		}

		if(*pbInWordTempFolder)
		{
			// Do nothing
		}
		else
		{
			OLUtilities::SetAttachmentDispName(Attachment, strCombPath.c_str());
		}
	}

	return hr;
}


void CItemEventDisp::OnAttachmentRead(Attachment* Attachment)
{
    UNREFERENCED_PARAMETER(Attachment);
	DP((L"CItemEventDisp::OnAttachmentRead\n"));
}

void CItemEventDisp::OnBeforeAttachmentSave(Attachment* Attachment, VARIANT_BOOL* Cancel)
{
    UNREFERENCED_PARAMETER(Attachment);
    UNREFERENCED_PARAMETER(Cancel);
	DP((L"CItemEventDisp::OnBeforeAttachmentSave\n"));
}
/*
void CItemEventDisp::SaveNoteFileContent(CComPtr<IDispatch> dspNoteItem)
{
	CComBSTR  cmstrSubject;
	CComBSTR  cmstrBody;
	HRESULT  hr   = S_OK;

	wchar_t wzFileName[MAX_PATH] = {0};

	if (!GetTempFileName(g_strOETempFolder.c_str(), L"NLCA", 0, wzFileName))
	{
		logd(L"Get Notes content temp file name failed.\n");
		return;
	}

	CComPtr<Outlook::_NoteItem> spCurNoteItem = 0;
	hr = dspNoteItem->QueryInterface(Outlook::IID__NoteItem, (void**)&spCurNoteItem );
	
	if(SUCCEEDED(hr) && spCurNoteItem)
	{
		hr = spCurNoteItem->get_Subject(&cmstrSubject) ;
		hr = spCurNoteItem->get_Body(&cmstrBody) ;

		if (cmstrSubject!=NULL && cmstrBody!=NULL)
		{
			std::wofstream ofs(wzFileName, std::wofstream::binary);
			
			if (ofs.is_open())
			{
				ofs<<cmstrSubject.m_str<<L"\n";
				ofs<<cmstrBody.m_str;
				ofs.close();
			}
			else
			{
				loge(L"Create note content file: %s failed. \n", wzFileName);
			}
		}
	}
}
*/
/***********************************************************************************/
/* CLASS CObligations                                                              */
/***********************************************************************************/
CObligations::CObligations()
{
	m_Obligations.clear();
}

CObligations::~CObligations()
{
}

int  CObligations::GetObligations(CEAttributes *obligation)
{
	int ccObligation  = 0;
	int i=0;

	if(NULL== obligation || NULL==obligation->attrs||0==obligation->count) return ccObligation;

    stdext::hash_set<std::wstring> obTypesWithArgs;
    obTypesWithArgs.insert(OB_NAME_DM);
    obTypesWithArgs.insert(OB_NAME_MC);
    obTypesWithArgs.insert(OB_NAME_MT);
    obTypesWithArgs.insert(OB_NAME_IUO);
    obTypesWithArgs.insert(OB_NAME_ER);
    obTypesWithArgs.insert(OB_NAME_HDR);
    obTypesWithArgs.insert(OB_NAME_LD);
	obTypesWithArgs.insert(OB_NAME_MAP);
	obTypesWithArgs.insert(OB_NAME_CRL);
	// The first key must be the count of obligation
	if(NULL==cesdk.fns.CEM_GetString(obligation->attrs[0].key)
        || NULL==cesdk.fns.CEM_GetString(obligation->attrs[0].value))
        return ccObligation;
	// Get all of the obligations
	// All obligations must start with OBLIGATION_NAME_ID
	LPOBLIGATION    lpObligation = NULL;
	for(i=1; i<obligation->count; i++)
	{
		if(NULL==cesdk.fns.CEM_GetString(obligation->attrs[i].key)
            || NULL==cesdk.fns.CEM_GetString(obligation->attrs[i].value))
			continue;

		std::wstring name(cesdk.fns.CEM_GetString(obligation->attrs[i].key));
		std::wstring value(cesdk.fns.CEM_GetString(obligation->attrs[i].value));
		DP((L"Obligations: [%s], <%s>\n", name.c_str(), value.c_str()));
		// create new Obligations
		if(0==wcsncmp(OBLIGATION_NAME_ID, name.c_str(), wcslen(OBLIGATION_NAME_ID)))
		{
			// new Obligation
			lpObligation = new OBLIGATION;
			if(NULL == lpObligation) continue;
			m_Obligations.push_back(lpObligation);

			lpObligation->name = value;
            lpObligation->hasArgs =
                (obTypesWithArgs.find(value) != obTypesWithArgs.end());
		}
		else if(0==wcsncmp(OBLIGATION_NUMVALUES_ID, name.c_str(), wcslen(OBLIGATION_NUMVALUES_ID)))
		{
			if(NULL == lpObligation) continue;
		}
		else if(0==wcsncmp(OBLIGATION_VALUE_ID, name.c_str(), wcslen(OBLIGATION_VALUE_ID)))
		{
			if(NULL == lpObligation) continue;
            if(lpObligation->hasArgs)
            {
                if(i+1<obligation->count)
                {
                    if(NULL==cesdk.fns.CEM_GetString(obligation->attrs[i+1].key)
                        || NULL==cesdk.fns.CEM_GetString(obligation->attrs[i+1].value))
                    {
                        i++;
                        continue;
                    }

                    std::wstring name2(cesdk.fns.CEM_GetString(obligation->attrs[i+1].key));
                    std::wstring value2(cesdk.fns.CEM_GetString(obligation->attrs[i+1].value));
                    DP((L"Obligations: [%s], <%s>\n", name2.c_str(), value2.c_str()));
                    lpObligation->argNameValues[value] = value2;
                    i++;
                }
            }
            else
            {
                lpObligation->values.push_back(value);
            }
		}
		else
		{
			continue;
		}
	}

	ccObligation = (int)m_Obligations.size();
	return ccObligation;
}

void CObligations::FreeObligations()
{
	int ccObligations = (int)m_Obligations.size();
	int i = 0;

	for(i=0; i<ccObligations; i++)
	{
		LPOBLIGATION lpObligation = m_Obligations[i];
		if(lpObligation)
		{
			lpObligation->values.clear();
			delete lpObligation;
		}
		lpObligation = NULL;
	}
	m_Obligations.clear();
}



BOOL CObligations::CheckHiddenDataRemoval(LPHIDDENDATAREMOVALINFO hdrInfo)
{
    int i;

	for(i=0; i<(int)m_Obligations.size(); i++)
	{
		if(0 == _wcsicmp(OB_NAME_HDR, m_Obligations[i]->name.c_str()))
		{
			// this is a Hidden Data Removal obligation
			hdrInfo->valid = TRUE;

            stdext::hash_map<std::wstring, std::wstring>::iterator it;

            it = m_Obligations[i]->argNameValues.find(OB_HDR_ARG_NAME_URL);
            hdrInfo->url = (it == m_Obligations[i]->argNameValues.end() ?
                            /*OBLIGATION_URL_DEFAULT*/L""
                            :
                            it->second);

			break;
		}
	}

	return hdrInfo->valid;
}


/*
1. 用来判断某些obligation 我们是否做过
2. 如果是要做多次的obligation, 我们需要判断是否以前做过同样的。
*/


emObligationExistType CheckValidAttachmentObl(CSendEmailData & SendEmailData, int nAttachmentPos,int nOblType,VECTOR_TAGPAIR& vecTagInfo)
{

	int nExistOblType = SendEmailData.GetAttachmentData()[nAttachmentPos].GetExistObligationType();
	logd(L"CheckValidAttachmentObl attachment[%d] existobltype %d\n",nAttachmentPos,nExistOblType);
	if (nOblType & nExistOblType)
	{
		//attachment had done this obligation
		if (nOblType & LOOP_OBLIGATION_HDR || nOblType & LOOP_OBLIGATION_VERIFY_RECIPIENTS
			|| nOblType & LOOP_OBLIGATION_MUTIPLE_CLIENT_CONFIGURATION || nOblType & LOOP_OBLIGATION_MISSING_TAG
			|| nOblType & LOOP_OBLIGATION_REJECT_UNLESS_SILENT_OVERRIDE || nOblType & LOOP_OBLIGATION_OIRM_AUTOMATIC
			|| nOblType & LOOP_OBLIGATION_OIRM_INTERACTIVE || nOblType & LOOP_OBLIGATION_ZIP_ENCRYTION
			|| nOblType & LOOP_OBLIGATION_PGP_ENCRYTION || nOblType & LOOP_OBLIGATION_STRIP_ATTACHMENT
			|| nOblType & LOOP_OBLIGATION_EMAIL_APPROVAL || nOblType & LOOP_OBLIGATION_DOMAIN_MISMATCH_CONFIRMATION
			|| nOblType & LOOP_OBLIGATION_MAIL_ATTR_PARSING || nOblType & LOOP_OBLIGATION_MAIL_NOTIFICATION
			|| nOblType & LOOP_OBLIGATION_FILE_OHC || nOblType & LOOP_OBLIGATION_LOGDECISION
			|| nOblType & LOOP_OBLIGATION_INTERNAL_USE_ONLY || nOblType & LOOP_OBLIGATION_FTPADAPTER
			|| nOblType & LOOP_OBLIGATION_FSADAPTER || nOblType & LOOP_OBLIGATION_SPUPLOADADAPTER)
		{
			return emOBL_EXIST;
		}

		if (nOblType & LOOP_OBLIGATION_FILE_TAGGING )
		{
			// we need check the file tag is the same
		
			if (SendEmailData.GetAttachmentData()[nAttachmentPos].GetAttachmentTagData().IsTempAddedInteractiveTagCustomTagExist(vecTagInfo))
			{
				return emOBL_EXIST_SAME;
			}
			else
			{
				return emOBL_NOT_EXIST;
			}
		}

		if (nOblType & LOOP_OBLIGATION_FILE_TAGGING_AUTO)
		{
			if (SendEmailData.GetAttachmentData()[nAttachmentPos].GetAttachmentTagData().IsTempAddedAutoTagCustomTagExist(vecTagInfo))
			{
				return emOBL_EXIST_SAME;
			}
			else
			{
				return emOBL_NOT_EXIST;
			}
		}
		
	}
	else
	{
		return emOBL_NOT_EXIST;
	}
	return emOBL_NOT_EXIST;
}

/*
	用来记录这个obligation 是不是需要做的。
	emOBL_EXIST 表示 这个obligation，我们曾经做过，需求只要求我们只做一次，所以这种不需要再做
	emOBL_NOT_EXIST 表示  这个obligation, 我们没有做过， 这种需要做
	emOBL_EXIST_SAME 表示 这个obligation, 曾经做过，但是需求要求我们做多次，表示这个obligation我们曾经做过完全相同的，所以这个不用做。
	//vectaginfo used for file tag,caInfo used for body obligation, strbuf used for body or subject obligations
*/


emObligationExistType SetObligationIntoCache(/*CEEnforcement_t *enforcer,*/EmQueryResCommand emCommand,
							CSendEmailData & SendEmailData, int nAttachmentPos,int nOblType,VECTOR_TAGPAIR& vecTagInfo,CAOblInfo& caInfo,wstring &strBuf)
{NLONLY_DEBUG
    NLPRINT_DEBUGVIEWLOG(L"emCommand:[%d], nOblType:[%d]\n", emCommand, nOblType); // CONTENTREDACTION_DOB_BODY 1 << 15 === LOOP_OBLIGATION_APEND_BODY
    emObligationExistType ExistType = emOBL_NOT_EXIST;
	if ((nOblType & LOOP_OBLIGATION_PREPEND_BODY) || (nOblType & LOOP_OBLIGATION_APEND_BODY) || (nOblType & LOOP_OBLIGATION_PREPEND_SUBJECT))
	{
		if (nOblType & LOOP_OBLIGATION_PREPEND_BODY)
		{
			if (SendEmailData.GetBodyData().IsExistSamePrependBody(strBuf))
			{
				ExistType = emOBL_EXIST_SAME;
			}
			else
			{
				SendEmailData.GetBodyData().SetPrependBody(strBuf,false);
			}
		}

		if ( nOblType & LOOP_OBLIGATION_APEND_BODY)
		{
			if (SendEmailData.GetBodyData().IsExistSameAppendBody(strBuf))
			{
				ExistType = emOBL_EXIST_SAME;
			}
			else
			{
				SendEmailData.GetBodyData().SetAppendBody(strBuf,false);
			}
		}

		if ( nOblType & LOOP_OBLIGATION_PREPEND_SUBJECT)
		{
			if (SendEmailData.GetSubjectData().IsExistSamePrependSubject(strBuf))
			{
				ExistType = emOBL_EXIST_SAME;
			}
			else
			{
				SendEmailData.GetSubjectData().SetPrependSubject(strBuf,false);
			}
		}

		return ExistType;
	}

	if (emCommand == ATTACHMENT_COMMAND || ATTACHMENT_RECIPIENT_AS_USER_COMMAND == emCommand)
	{
		ExistType = CheckValidAttachmentObl(SendEmailData, nAttachmentPos,nOblType,vecTagInfo);
		logd(L"===>SetObligationIntoCache %d ExistType =0x%08X, nOblType=%#x\n", emCommand, ExistType, nOblType);

		if (ATTACHMENT_RECIPIENT_AS_USER_COMMAND == emCommand)
		{
			// for ATTACHMENT_RECIPIENT_AS_USER_COMMAND, only allow OHC (0x100000), FILE_TAGGING (0x2000), TAGGING_AUTO (0x200000), see #ClassifyObligationType
			//if (LOOP_OBLIGATION_FILE_OHC != nOblType && LOOP_OBLIGATION_FILE_TAGGING != nOblType && LOOP_OBLIGATION_FILE_TAGGING_AUTO != nOblType)
			if (!(LOOP_OBLIGATION_FILE_OHC | LOOP_OBLIGATION_FILE_TAGGING | LOOP_OBLIGATION_FILE_TAGGING_AUTO) & nOblType)
			{
				logw(L"===>only support OHC (0x100000), FILE_TAGGING (0x2000), TAGGING_AUTO (0x200000), but currrent is %#x", nOblType);
				return ExistType;
			}
		}

		if (emOBL_EXIST == ExistType)
		{
			//this obligation just need to do one time. the information is no need cache 
		}
		if (emOBL_NOT_EXIST == ExistType)
		{
			//set exist type
			SendEmailData.GetAttachmentData()[nAttachmentPos].SetExistObligationType(nOblType);
			logi(L"===>SetObligationIntoCache ATTACHMENT_COMMAND SetExistObligationType =%#x\n", nOblType);
			if (nOblType & LOOP_OBLIGATION_FILE_TAGGING )
			{
				SendEmailData.GetAttachmentData()[nAttachmentPos].GetAttachmentTagData().SetTempAddedInteractiveTagCustomTag(vecTagInfo);
			}

			if (nOblType & LOOP_OBLIGATION_FILE_TAGGING_AUTO)
			{
				SendEmailData.GetAttachmentData()[nAttachmentPos].GetAttachmentTagData().SetTempAddedAutoTagCustomTag(vecTagInfo);
			}
		}
	}
	else if (emCommand == BODY_COMMAND)
	{
		SendEmailData.GetBodyData().SetExistObligationType(nOblType);
		SendEmailData.GetBodyData().SetCABodyOblData(caInfo,nOblType);
	}
	else if (emCommand == SUBJECT_COMMAND)
	{
		SendEmailData.GetSubjectData().SetExistObligationType(nOblType);
		SendEmailData.GetSubjectData().SetCASubjectOblData(caInfo,nOblType);
	}

	return ExistType;
}

/*
nRecipientNum: recipient index
*/
void SenderMatchRejectUtillSilent(CEEnforcement_t *enforcer)
{
	if (enforcer != NULL && enforcer->obligation != NULL)
	{
		std::wstring value(cesdk.fns.CEM_GetString(enforcer->obligation->attrs[0].value));
		std::wstring name(cesdk.fns.CEM_GetString(enforcer->obligation->attrs[0].key));
		logd(L"[SenderMatchRejectUtillSilent]Attrs[0], \"%s\"=\"%s\"", name.c_str(), value.c_str());
		if (0 == _wcsnicmp(L"1", value.c_str(), 1))//"CE_ATTR_OBLIGATION_COUNT"="1"
		{
			std::wstring value(cesdk.fns.CEM_GetString(enforcer->obligation->attrs[1].value));
			/*
				CE_ATTR_OBLIGATION_NAME:1
				REJECT_UNLESS_SILENT_OVERRIDE
			*/
			if (0 == _wcsnicmp(L"REJECT_UNLESS_SILENT_OVERRIDE", value.c_str(), wcslen(L"REJECT_UNLESS_SILENT_OVERRIDE")))
			{
				enforcer->result = CEAllow;
			}
		}
	}
}


int CObligations::ClassifyObligationType(CEEnforcement_t *enforcer,EmQueryResCommand emCommand, CSendEmailData & SendEmailData,int nAttachmentPos,int nRecipientNum,bool &bAllow, DWORD* pdwOblTypes /* = NULL */)
{NLONLY_DEBUG
	if (enforcer->result == CEDeny)
	{
		bAllow = false;
	}
	else
	{
		bAllow = true;
	}
	int nType = 0;
	if ((emCommand == ATTACHMENT_COMMAND || ATTACHMENT_RECIPIENT_AS_USER_COMMAND == emCommand) && (nAttachmentPos < 0  || SendEmailData.GetAttachmentData().size() < (DWORD)nAttachmentPos) )
	{
		return nType;
	}
	if (emCommand == ATTACHMENT_COMMAND){
		//if (enforcer->obligation->count == 1 && )
	}
	static std::vector<std::wstring> vecCA ;
	vecCA.push_back(L"NLCA_REDACTION_CCN");
	vecCA.push_back(L"NLCA_REDACTION_CV");
	vecCA.push_back(L"NLCA_REDACTION_PH");
	vecCA.push_back(L"NLCA_REDACTION_SSN");
	vecCA.push_back(L"NLCA_REDACTION_IP");
	vecCA.push_back(L"NLCA_REDACTION_EM");
	vecCA.push_back(L"NLCA_REDACTION_DOB");
	vecCA.push_back(L"NLCA_REDACTION_MA");
	vecCA.push_back(L"NLCA_REDACTION_KEYWORD");
	vecCA.push_back(L"NLCA_REDACTION");
	
	if (enforcer != NULL && enforcer->obligation != NULL)
	{
		AdapterCommon::Attachment AdapterAttachment;
		emObligationExistType  OblExistType;
		DWORD dwOblTypes = 0;
		for (int i = 0; i < enforcer->obligation->count; i++)
		{
			if(NULL==cesdk.fns.CEM_GetString(enforcer->obligation->attrs[i].key)
				|| NULL==cesdk.fns.CEM_GetString(enforcer->obligation->attrs[i].value))
				continue;
			std::wstring value(cesdk.fns.CEM_GetString(enforcer->obligation->attrs[i].value));
			std::wstring name(cesdk.fns.CEM_GetString(enforcer->obligation->attrs[i].key));

			VECTOR_TAGPAIR vec;
			CAOblInfo ca;
			wstring strBuf;

			NLPRINT_DEBUGVIEWLOG(L"i = [%d],name = [%s],value = [%s]  !!emCommand=%d\n",i, name.c_str(),value.c_str(), emCommand);
			logd(L"[ClassifyObligationType]Attrs[%d], cmd_%d, \"%s\"=\"%s\"",i, emCommand, name.c_str(),value.c_str());
			if (0==_wcsnicmp(OB_NAME_RICHALERT_MESSAGE, value.c_str(), wcslen(OB_NAME_RICHALERT_MESSAGE)))
			{
				// e.g. CE_ATTR_OBLIGATION_POLICY:1   	OE8.5cases/Sam/AllowIfSubjectHasAutoTagHeader	enforcer->obligation->attrs[1]

				// Bug 43866 - [oe8.5]can't show rich alert message if deny do content redaction
				if (0 != SendEmailData.GetRichAlerMessageEvalAgainTime() && SendEmailData.GetRichAlerMessageEvalAgainTime() != SendEmailData.GetEvaAgainTimes())
				{
					continue;
				}else
				{
					SendEmailData.SetRichAlerMessageEvalAgainTime(SendEmailData.GetEvaAgainTimes());
				}

				if (enforcer->obligation->count >  i + 7)
				{
					LPCWSTR pwzPolicyName  = cesdk.fns.CEM_GetString(enforcer->obligation->attrs[i + 1].value);
					LPCWSTR pwzAlertMsg  = cesdk.fns.CEM_GetString(enforcer->obligation->attrs[i + 7].value);
					logd(L"[ClassifyObligationType]Allow=%d, obligations[%d/%d], emCommand=%d, OB_NAME_RICHALERT_MESSAGE %s for %s, RichAlerMessageEvalAgainTimeAt_%u"
						, bAllow, i, enforcer->obligation->count, emCommand, pwzAlertMsg, pwzPolicyName, SendEmailData.GetRichAlerMessageEvalAgainTime());

					if (NULL != pwzAlertMsg)
					{
						if(ATTACHMENT_COMMAND == emCommand)
						{
							//CAttachmentData& attachmentData = SendEmailData.GetAttachmentData()[nAttachmentPos];
							//std::wstring wsItem = attachmentData.FileName();

							////logd(L"[ClassifyObligationType]%d/%d, Temp=%s, Source=%s, OrgTemp=%s", i, enforcer->obligation->count, attachmentData.GetTempPath().c_str()
							////	, attachmentData.GetSourcePath().c_str(), attachmentData.GetOrgTempPath().c_str());
							//logd(L"[ClassifyObligationType]%d/%d, Allow=%d, ATTACHMENT[%d]=%s", i, enforcer->obligation->count, bAllow, nAttachmentPos, wsItem.c_str());

							//if (bAllow)
							//	SendEmailData.GetAllowedAttachmentMessages().put(pwzAlertMsg, pwzPolicyName, wsItem, emCommand);
							//else
							//	SendEmailData.GetDeniedAttachmentMessages().put(pwzAlertMsg, pwzPolicyName, wsItem, emCommand);
						}else if (ATTACHMENT_RECIPIENT_AS_USER_COMMAND == emCommand)
						{
							//// only show files' name in the PA, see http://bugs.cn.nextlabs.com/show_bug.cgi?id=41874

							CAttachmentData& attachmentData = SendEmailData.GetAttachmentData()[nAttachmentPos];
							std::wstring wsItem = attachmentData.FileName();

							vector<int> vecRecipientIndex = attachmentData.GetReceiverIndexForReceiveAction();
							STRINGLIST recipients = SendEmailData.GetRecipientsData().GetSendPCRecipients();
							std::wstring& wsRecipient = recipients[vecRecipientIndex[nRecipientNum]];

							logd(L"[ClassifyObligationType]ATTACHMENT_RECIPIENT_AS_USER_COMMAND, for %d/%d, Allow=%d, Recipients_%d/%d vs SendPCRecipients_%d/%d: %s, ATTACHMENT%d=%s", i,
								 enforcer->obligation->count, bAllow, nRecipientNum, recipients.size(), vecRecipientIndex[nRecipientNum], vecRecipientIndex.size(), 
								 wsRecipient.c_str(), nAttachmentPos, wsItem.c_str());

							if (bAllow)
							{
								if (!SendEmailData.IsEqualSender(wsRecipient))//we need the recipient which is not equal sender
								{
									SendEmailData.GetAllowedAttachmentRecipientMessages().put(pwzAlertMsg, pwzPolicyName, wsRecipient, emCommand);
								}
								SendEmailData.GetAllowedAttachmentMessages().put(pwzAlertMsg, pwzPolicyName, wsItem, emCommand, nAttachmentPos);
							}else
							{
								if (!SendEmailData.IsEqualSender(wsRecipient))
								{
									SendEmailData.GetDeniedAttachmentRecipientMessages().put(pwzAlertMsg, pwzPolicyName, wsRecipient, emCommand);
								}
								SendEmailData.GetDeniedAttachmentMessages().put(pwzAlertMsg, pwzPolicyName, wsItem, emCommand, nAttachmentPos);
							}
						}else if (RECIPIENT_COMMAND == emCommand)
						{
							STRINGLIST recipients = SendEmailData.GetRecipientsData().GetRealRecipients();
							std::wstring& wsRecipient = recipients[nRecipientNum];

							logd(L"[ClassifyObligationType]RECIPIENT_COMMAND Obliation_%d/%d, Allow=%d, Recipients_%d/%d: %s", i, enforcer->obligation->count, bAllow
								, nRecipientNum, recipients.size(), wsRecipient.c_str());
							
							// Bug 44492 - [oe8.5]still show sender in rich alert message PA if send a meeting 
							if (!SendEmailData.IsEqualSender(wsRecipient)) // Hit the sender is one of the recipients.
							{
								if (bAllow)
									SendEmailData.GetAllowedRecipientMessages().put(pwzAlertMsg, pwzPolicyName, wsRecipient, emCommand);
								else
									SendEmailData.GetDeniedRecipientMessages().put(pwzAlertMsg, pwzPolicyName, wsRecipient, emCommand);
							}
						}else{ 
							// Also process other respond obligations, such as x-header @see http://bugs.cn.nextlabs.com/show_bug.cgi?id=41938

							logd(L"[ClassifyObligationType]Obliation_%d/%d, Allow=%d", i, enforcer->obligation->count, bAllow);
							if (bAllow)
							{
								SendEmailData.GetAllowedOtherMessages().put(pwzAlertMsg, pwzPolicyName, std::wstring(), emCommand);
							}
							else
							{
								SendEmailData.GetDeniedOtherMessages().put(pwzAlertMsg, pwzPolicyName, std::wstring(), emCommand);
							}
						}
					}
				}
				SendEmailData.GetAlertMessageData().hasAlertObligation = TRUE;
				//SendEmailData.m_AttachmentOblData->GetRichAlertMsgFromObl(enforcer->obligation,i,SendEmailData,bAllow);
			}
		
			if(0==_wcsnicmp(OB_RICH_USER_MESSAGE, value.c_str(), wcslen(OB_RICH_USER_MESSAGE)))
			{
				SendEmailData.m_AttachmentOblData->GetRichUserMsg(enforcer->obligation,i,SendEmailData);
			}
			
			//RejectUnlessSilent
			if(0== _wcsnicmp(OB_NAME_CRL, value.c_str(), wcslen(OB_NAME_CRL)) && emCommand == ATTACHMENT_RECIPIENT_AS_USER_COMMAND && !bAllow)
			{		
				nType = nType | LOOP_OBLIGATION_REJECT_UNLESS_SILENT_OVERRIDE;
				OblExistType = SetObligationIntoCache(emCommand,SendEmailData,nAttachmentPos,LOOP_OBLIGATION_REJECT_UNLESS_SILENT_OVERRIDE,vec,ca,strBuf);
				//logd(L"=====>ClassifyObligationType RejectUnlessSilent OblExistType=%d\n", OblExistType);
				if (!SendEmailData.AllowOfRejectUnlessSilent())
				{
					//set do obligation parameter in csendemaildata
					if (SendEmailData.m_AttachmentOblData->GetRejectUnlessSilentInfoFromObl(enforcer->obligation,i,nAttachmentPos,SendEmailData,nRecipientNum))
					{
						i += 5;
						continue;
					}
				}
			}

			
			if (0 == _wcsnicmp(L"Inherit_XHeader", value.c_str(), wcslen(L"Inherit_XHeader")))
			{
				bool needToInheritHeader = false;
				if (enforcer->obligation->count >  i + 3)
				{
					const TCHAR* pTemp3 = cesdk.fns.CEM_GetString(enforcer->obligation->attrs[i + 3].value);
					if (NULL != pTemp3)
					{
						if (0 == _wcsicmp(L"true", pTemp3))
						{
							needToInheritHeader = true;
							const TCHAR* pTemp5 = cesdk.fns.CEM_GetString(enforcer->obligation->attrs[i + 5].value);
							if (NULL != pTemp5)
							{
								SendEmailData.ExtraHeadersNeedInheriting(pTemp5);
							}
						}
						i += 3; // This obligation Inherit_XHeader spans 4 CEAttributes, help accelerate iteration
					}
				}
				SendEmailData.SetNeedToInheritHeader(needToInheritHeader);
				logd(L"Inherit XHeader=%s, needToInheritHeader=%d", value.c_str(), needToInheritHeader);
				continue;
			}
			else if (0 == _wcsnicmp(OB_AUTOMATIC_XHEADER_TAGGING, value.c_str(), wcslen(OB_AUTOMATIC_XHEADER_TAGGING)))
			{
				logd(L"[ClassifyObligationType]index@%d=%s, obligation count=%d", i, value.c_str(), enforcer->obligation->count);
				
				// 13 CEAttributes, e.g. 
				//CE_ATTR_OBLIGATION_NAME:1      AUTOMATIC_XHEADER_TAGGING
				//CE_ATTR_OBLIGATION_POLICY:1    OE8.5cases/Sam/AllowToDoAutoHeaderTagIfSubjectHasAutoHeaderTagInvalidName
				//CE_ATTR_OBLIGATION_VALUE:1:1   Tag Name
				//CE_ATTR_OBLIGATION_VALUE:1:2   /
				//CE_ATTR_OBLIGATION_VALUE:1:3   Tag Value
				//CE_ATTR_OBLIGATION_VALUE:1:4   invalid name?
				//CE_ATTR_OBLIGATION_VALUE:1:5   Description
				//CE_ATTR_OBLIGATION_VALUE:1:6   ff
				//CE_ATTR_OBLIGATION_VALUE:1:7   Decision on Tag Error
				//CE_ATTR_OBLIGATION_VALUE:1:8   continue
				//CE_ATTR_OBLIGATION_VALUE:1:9   Message for block reason
				//CE_ATTR_OBLIGATION_VALUE:1:10  The application is unable to tag the email, due to unknown issues.
				//CE_ATTR_OBLIGATION_NUMVALUES:1 10

				if (enforcer->obligation->count >  i + 11)
				{
					LPCWSTR pwzTagName = cesdk.fns.CEM_GetString(enforcer->obligation->attrs[i + 3].value);
					LPCWSTR pwzTagValue = cesdk.fns.CEM_GetString(enforcer->obligation->attrs[i + 5].value);
					LPCWSTR pwzDescription = cesdk.fns.CEM_GetString(enforcer->obligation->attrs[i + 7].value);
					LPCWSTR pwzErrAction = cesdk.fns.CEM_GetString(enforcer->obligation->attrs[i + 9].value);
					LPCWSTR pwzMsgIfBlock = cesdk.fns.CEM_GetString(enforcer->obligation->attrs[i + 11].value);

					// if you leave the editor empty in Policy Studio, #pwTagName is NULL
					logd(L"Auto-Header: TagName=%s, TagValue=%s, ErrAction=%s, MsgIfBlock=%s", pwzTagName, pwzTagValue, pwzErrAction, pwzMsgIfBlock);
					
					if (NULL != pwzTagName && NULL != pwzTagValue )
					{
						CMessageHeader& header = SendEmailData.GetMessageHeader();

						vec.push_back(pair<wstring,wstring>(pwzTagName,pwzTagValue));

						header.SetTagPairs(vec); //merge
						header.SetAutoTag(true);

						if (!boost::iequals(header.ErrorAction(), DECISION_ON_TAG_ERROR_BLOCK))
						{
							header.ErrorAction(NULL != pwzErrAction ? pwzErrAction : L"");
							header.MessageIfBlcok(NULL != pwzMsgIfBlock ? pwzMsgIfBlock : L"");
						}
						i += 12; // This obligation AUTOMATIC_XHEADER_TAGGING spans 13 CEAttributes, help accelerate iteration
					}
				}
				continue;
			}
			// X-Header Hierarchical Classification (just like File Hierarchical Classification)
			else if(0 == _wcsnicmp(OB_XHEADER_HIERARCHICAL_CLASSIFICATION, value.c_str(), wcslen(OB_XHEADER_HIERARCHICAL_CLASSIFICATION)))
			{
				// Only the first matched and returned obligation will be executed.
				if (!SendEmailData.GetMessageHeader().HasHCTagging())
				{
					logd(L"[ClassifyObligationType]index@%d=%s, obligation count=%d", i, value.c_str(), enforcer->obligation->count);

					////change the enforcer value
					//cesdk.fns.CEM_FreeString(enforcer->obligation->attrs[i].value);
					//enforcer->obligation->attrs[i].value = cesdk.fns.CEM_AllocateString(OB_NAME_OHC); 
					
					if (enforcer->obligation->count >  i + 7)
					{
						CMessageHeader& header = SendEmailData.GetMessageHeader();

						LPCWSTR pwzErrAction = cesdk.fns.CEM_GetString(enforcer->obligation->attrs[i + 5].value);
						LPCWSTR pwzMsgIfBlock = cesdk.fns.CEM_GetString(enforcer->obligation->attrs[i + 7].value);
						
						// If one of responded abolitions already has "block", nothing to do.
						if (!boost::iequals(header.ErrorAction(), DECISION_ON_TAG_ERROR_BLOCK)) //StrCmpIW(pwzErrAction, L"block")
						{
							header.ErrorAction(NULL != pwzErrAction ? pwzErrAction : L"");
							header.MessageIfBlcok(NULL != pwzMsgIfBlock ? pwzMsgIfBlock : L"");
						}

						logd(L"HC-Header: ErrAction=%s, MsgIfBlock=%s", pwzErrAction, pwzMsgIfBlock);
					}

					PerpareXHeaderHCTagging(SendEmailData, enforcer->obligation);
				}else
				{
					logd(L"!!Ignored, because it's not the first matched OB_XHEADER_HIERARCHICAL_CLASSIFICATION=%s", value.c_str());
				}
				// i += 14;// This obligation XHEADER_HIERARCHICAL_CLASSIFICATION spans 15 CEAttributes
				continue;
			}

			//prepend email body
			else if (0 == _wcsnicmp(OBLIGATION_PREPEND_BODY, value.c_str(), wcslen(OBLIGATION_PREPEND_BODY)))
			{
				if (SendEmailData.m_AttachmentOblData->GetPrependBodyFromObl(enforcer->obligation,i,strBuf))
				{
					SetObligationIntoCache(emCommand,SendEmailData,nAttachmentPos,LOOP_OBLIGATION_PREPEND_BODY,vec,ca,strBuf);
				}
				continue;
			}
			//APEND_BODY
			else if (0 == _wcsnicmp(OBLIGATION_APEND_BODY, value.c_str(), wcslen(OBLIGATION_APEND_BODY)))
			{
				if (SendEmailData.m_AttachmentOblData->GetAppendBodyFromObl(enforcer->obligation,i,strBuf))
				{
					SetObligationIntoCache(emCommand,SendEmailData,nAttachmentPos,LOOP_OBLIGATION_APEND_BODY,vec,ca,strBuf);
				}
				continue;
			}
			//PREPEND_SUBJECT
			else if (0 == _wcsnicmp(OBLIGATION_PREPEND_SUBJECT, value.c_str(), wcslen(OBLIGATION_PREPEND_SUBJECT)))
			{
				if (SendEmailData.m_AttachmentOblData->GetPrependSubjectFromObl(enforcer->obligation,i,strBuf))
				{
					SetObligationIntoCache(emCommand,SendEmailData,nAttachmentPos,LOOP_OBLIGATION_PREPEND_SUBJECT,vec,ca,strBuf);
				}
				continue;
			}
			//Outlook: Hierarchical Classification
			else if (0 == _wcsnicmp(OB_NAME_OHC, value.c_str(), wcslen(OB_NAME_OHC)) && (emCommand == ATTACHMENT_COMMAND || ATTACHMENT_RECIPIENT_AS_USER_COMMAND == emCommand))
			{
				dwOblTypes |= LOOP_OBLIGATION_FILE_OHC; 
				OblExistType = SetObligationIntoCache(emCommand,SendEmailData,nAttachmentPos,LOOP_OBLIGATION_FILE_OHC,vec,ca,strBuf);
				logd(L"[ClassifyObligationType]obligations[%d/%d], OE_HIERARCHICAL_CLASSIFICATION, OblExistType=%d vs emOBL_EXIST=%d", i, enforcer->obligation->count, OblExistType, emOBL_EXIST);
				if (OblExistType == emOBL_EXIST) 
				{
					//change the enforcer value
					cesdk.fns.CEM_FreeString(enforcer->obligation->attrs[i].value);
					enforcer->obligation->attrs[i].value = cesdk.fns.CEM_AllocateString(OB_NAME_OHC_EXIST);
				}
				else
				{
					SendEmailData.m_OnlyAttachmentData.m_bNeedHCFileTagging = TRUE;
				}
				i += 13; // This obligation OE_HIERARCHICAL_CLASSIFICATION spans 19 CEAttributes i += 18;
				continue;
			}
			//Automatic File tagging
			else if (0 == _wcsnicmp(OB_NAME_AFT, value.c_str(), wcslen(OB_NAME_AFT)) && (emCommand == ATTACHMENT_COMMAND || ATTACHMENT_RECIPIENT_AS_USER_COMMAND == emCommand))
			{
				dwOblTypes |= LOOP_OBLIGATION_FILE_TAGGING_AUTO; 
				if (SendEmailData.m_AttachmentOblData->GetFileCustomAutoTagFromObl(enforcer->obligation,i,vec))
				{
					OblExistType = SetObligationIntoCache(emCommand,SendEmailData,nAttachmentPos,LOOP_OBLIGATION_FILE_TAGGING_AUTO,vec,ca,strBuf);
					if (OblExistType == emOBL_EXIST_SAME) 
					{
						//change the enforcer value
						cesdk.fns.CEM_FreeString(enforcer->obligation->attrs[i].value);
						enforcer->obligation->attrs[i].value = cesdk.fns.CEM_AllocateString(OB_NAME_AFT_EXIST);

						i += 15;
					}
					else
					{
						SendEmailData.m_OnlyAttachmentData.m_bNeedAutoFileTagging = TRUE;
					}
				}
				continue;
			}
			//Hidden Data Remove 
			else if (0 == _wcsnicmp(OB_NAME_HDR, value.c_str(), wcslen(OB_NAME_HDR)) && (emCommand == ATTACHMENT_COMMAND || ATTACHMENT_RECIPIENT_AS_USER_COMMAND == emCommand))
			{
				dwOblTypes |= LOOP_OBLIGATION_HDR; 
				OblExistType = SetObligationIntoCache(emCommand,SendEmailData,nAttachmentPos,LOOP_OBLIGATION_HDR,vec,ca,strBuf);
				if (OblExistType == emOBL_NOT_EXIST) 
				{
					if (SendEmailData.m_AttachmentOblData->GetHDRFromObl(enforcer->obligation,i,nAttachmentPos,SendEmailData))
					{
						i += 3;	
					}
					continue;
				}

			}
			//MailAttrParse
			if (0 == _wcsnicmp(OB_NAME_MAP, value.c_str(), wcslen(OB_NAME_MAP)) && emCommand == ATTACHMENT_COMMAND)
			{
				if (SendEmailData.m_OnlyAttachmentData.m_mapInfo.bIsBlock || SendEmailData.m_OnlyAttachmentData.m_mapInfo.bObligationDone)
				{
					continue;
				}
				
				OblExistType = SetObligationIntoCache(emCommand,SendEmailData,nAttachmentPos,LOOP_OBLIGATION_MAIL_ATTR_PARSING,vec,ca,strBuf);
				if (OblExistType == emOBL_NOT_EXIST || !SendEmailData.m_OnlyAttachmentData.m_mapInfo.bIsBlock)
				{
					if (SendEmailData.m_AttachmentOblData->GetMailAttributeParsingFromObl(enforcer->obligation,i,SendEmailData))
					{
						i += 11;
					}
					continue;
				}
			}
			
			//MAIL_NOTIFICATION
			if (0 == _wcsnicmp(NOTI_OBLIGATION, value.c_str(), wcslen(NOTI_OBLIGATION)) && emCommand == ATTACHMENT_COMMAND)
			{
				OblExistType = SetObligationIntoCache(emCommand,SendEmailData,nAttachmentPos,LOOP_OBLIGATION_MAIL_NOTIFICATION,vec,ca,strBuf);
				if (OblExistType == emOBL_NOT_EXIST) 
				{
					if (SendEmailData.m_AttachmentOblData->GetMailNotiFromObl(enforcer->obligation,i,SendEmailData))
					{
						i += 15;	
					}
					continue;
				}
			}
			//Internal Use only
			else if (0 == _wcsnicmp(OB_NAME_IUO, value.c_str(), wcslen(OB_NAME_IUO)) && emCommand == ATTACHMENT_COMMAND)
			{
				OblExistType = SetObligationIntoCache(emCommand,SendEmailData,nAttachmentPos,LOOP_OBLIGATION_INTERNAL_USE_ONLY,vec,ca,strBuf);
				if (OblExistType == emOBL_NOT_EXIST) 
				{
					if (SendEmailData.m_AttachmentOblData->GetInteUseOnlyFromObl(enforcer->obligation,i,nAttachmentPos,SendEmailData))
					{
						i += 3;	
					}
					continue;
				}
			}
			//Missing tag
			else if (0 == _wcsnicmp(OB_NAME_MT, value.c_str(), wcslen(OB_NAME_MT)) && emCommand == ATTACHMENT_COMMAND)
			{
				OblExistType = SetObligationIntoCache(emCommand,SendEmailData,nAttachmentPos,LOOP_OBLIGATION_MISSING_TAG,vec,ca,strBuf);
				if (OblExistType == emOBL_NOT_EXIST) 
				{
					if (SendEmailData.m_AttachmentOblData->GetMissTagFromObl(enforcer->obligation,i,nAttachmentPos,SendEmailData))
					{
						i += 9;	
					}
					continue;
				}
			}
			//Verify Recipients
			else if (0 == _wcsnicmp(OB_NAME_ER, value.c_str(), wcslen(OB_NAME_ER)) && emCommand == ATTACHMENT_COMMAND)
			{
				OblExistType = SetObligationIntoCache(emCommand,SendEmailData,nAttachmentPos,LOOP_OBLIGATION_VERIFY_RECIPIENTS,vec,ca,strBuf);
				if (OblExistType == emOBL_NOT_EXIST) 
				{
					if (SendEmailData.m_AttachmentOblData->GetVerifyRecipientFromObl(enforcer->obligation,i,nAttachmentPos,SendEmailData))
					{
						i += 5;	
					}
					continue;
				}
			}
			//Multiple client confirmation
			else if (0 == _wcsnicmp(OB_NAME_MC, value.c_str(), wcslen(OB_NAME_MC)) && emCommand == ATTACHMENT_COMMAND)
			{
				OblExistType = SetObligationIntoCache(emCommand,SendEmailData,nAttachmentPos,LOOP_OBLIGATION_MUTIPLE_CLIENT_CONFIGURATION,vec,ca,strBuf);
				if (OblExistType == emOBL_NOT_EXIST) 
				{
					if (SendEmailData.m_AttachmentOblData->GetMulClientFromObl(enforcer->obligation,i,nAttachmentPos,SendEmailData))
					{
						i += 5;	
					}
					continue;
				}
			}
			//Domain Mismatch confirmation
			else if (0 == _wcsnicmp(OB_NAME_DM, value.c_str(), wcslen(OB_NAME_DM)) && emCommand == ATTACHMENT_COMMAND)
			{
				OblExistType = SetObligationIntoCache(emCommand,SendEmailData,nAttachmentPos,LOOP_OBLIGATION_DOMAIN_MISMATCH_CONFIRMATION,vec,ca,strBuf);
				if (OblExistType == emOBL_NOT_EXIST) 
				{
					if (SendEmailData.m_AttachmentOblData->GetDomainMismatchFromObl(enforcer->obligation,i,nAttachmentPos,SendEmailData))
					{
						i += 7;	
					}
					continue;
				}
			}
			
			//Interactive File tagging
			else if (0 == _wcsnicmp(OB_NAME_IFT, value.c_str(), wcslen(OB_NAME_IFT)) && emCommand == ATTACHMENT_COMMAND)
			{
				if (SendEmailData.m_AttachmentOblData->GetFileCustomInterTagFromObl(enforcer->obligation,i,vec))
				{
					OblExistType = SetObligationIntoCache(emCommand,SendEmailData,nAttachmentPos,LOOP_OBLIGATION_FILE_TAGGING,vec,ca,strBuf);
					if (OblExistType == emOBL_EXIST_SAME) 
					{
						//change the enforcer value
						cesdk.fns.CEM_FreeString(enforcer->obligation->attrs[i].value);
						enforcer->obligation->attrs[i].value = cesdk.fns.CEM_AllocateString(OB_NAME_IFT_EXIST);
						i += 15;
					}
					else
					{
						SendEmailData.m_OnlyAttachmentData.m_bNeedInterFileTagging = TRUE;
					}
				}
				continue;
			}
			
			//Outlook Integrated Rights Management - automatic
			else if (0 == _wcsnicmp(OBLIGATION_INTEGRATED_RIGHTS_MANAGEMENT_AUTOMATIC, value.c_str(), wcslen(OBLIGATION_INTEGRATED_RIGHTS_MANAGEMENT_AUTOMATIC)) && emCommand == ATTACHMENT_COMMAND)
			{
				OblExistType = SetObligationIntoCache(emCommand,SendEmailData,nAttachmentPos,LOOP_OBLIGATION_OIRM_AUTOMATIC,vec,ca,strBuf);
				if (OblExistType == emOBL_NOT_EXIST) 
				{
					if (SendEmailData.m_AttachmentOblData->GetNxlAutoTagFromObl(enforcer->obligation,i,SendEmailData,nAttachmentPos))
					{
						SendEmailData.m_OnlyAttachmentData.m_bNeedRMC = TRUE;
						i += 3;	
					}
					continue;
				}
				
			}
			//Outlook Integrated Rights Management - interactive
			else if (0 == _wcsnicmp(OBLIGATION_INTEGRATED_RIGHTS_MANAGEMENT_INTERACTIVE, value.c_str(), wcslen(OBLIGATION_INTEGRATED_RIGHTS_MANAGEMENT_INTERACTIVE)) && emCommand == ATTACHMENT_COMMAND)
			{
				OblExistType = SetObligationIntoCache(emCommand,SendEmailData,nAttachmentPos,LOOP_OBLIGATION_OIRM_INTERACTIVE,vec,ca,strBuf);
				if (OblExistType == emOBL_NOT_EXIST) 
				{
					SendEmailData.GetAttachmentData()[nAttachmentPos].SetInterActiveNxl(TRUE);
					SendEmailData.m_OnlyAttachmentData.m_bNeedRMC = TRUE;
				}
				continue;
			}
			//ZIP
			else if (0 == _wcsnicmp(OBLIGATION_PASSWORD_BASED_ENCRYPTION, value.c_str(), wcslen(OBLIGATION_PASSWORD_BASED_ENCRYPTION)) && emCommand == ATTACHMENT_COMMAND)
			{
				OblExistType = SetObligationIntoCache(emCommand,SendEmailData,nAttachmentPos,LOOP_OBLIGATION_ZIP_ENCRYTION,vec,ca,strBuf);
				if (OblExistType == emOBL_EXIST) 
				{
					//change the enforcer value
					cesdk.fns.CEM_FreeString(enforcer->obligation->attrs[i].value);
					enforcer->obligation->attrs[i].value = cesdk.fns.CEM_AllocateString(OBLIGATION_PASSWORD_BASED_ENCRYPTION_EXIST);			
				}
				else
				{
					SendEmailData.m_OnlyAttachmentData.m_bNeedZIP = TRUE;
				}
			}
			//PGP
			else if (0 == _wcsnicmp(OBLIGATION_IDENTITY_BASED_ENCRYPTION, value.c_str(), wcslen(OBLIGATION_IDENTITY_BASED_ENCRYPTION)) && emCommand == ATTACHMENT_COMMAND)
			{
				OblExistType = SetObligationIntoCache(emCommand,SendEmailData,nAttachmentPos,LOOP_OBLIGATION_PGP_ENCRYTION,vec,ca,strBuf);
				if (OblExistType == emOBL_EXIST) 
				{
					//change the enforcer value
					cesdk.fns.CEM_FreeString(enforcer->obligation->attrs[i].value);
					enforcer->obligation->attrs[i].value = cesdk.fns.CEM_AllocateString(OBLIGATION_IDENTITY_BASED_ENCRYPTION_EXIST);			
				}
				else
				{
					SendEmailData.m_OnlyAttachmentData.m_bNeedPGP = TRUE;
				}
			}
			
			//Strip ftp
			else if (0 == _wcsnicmp(OBLIGATION_FTPADAPTER, value.c_str(), wcslen(OBLIGATION_FTPADAPTER)) && emCommand == ATTACHMENT_COMMAND)
			{
				if (!SendEmailData.GetAttachmentData()[nAttachmentPos].IsIgnored())
				{
					AdapterAttachment.SetStripFlag(true);
					OblExistType = SetObligationIntoCache(emCommand,SendEmailData,nAttachmentPos,LOOP_OBLIGATION_FTPADAPTER,vec,ca,strBuf);
					if (OblExistType == emOBL_NOT_EXIST) 
					{
						if (SendEmailData.m_AttachmentOblData->GetStripFTPFromObl(enforcer->obligation,i,SendEmailData,AdapterAttachment))
						{
							i += 27;
							continue;
						}
					}
				}
			}
			//Strip File Server
			else if (0 == _wcsnicmp(OBLIGATION_FSADAPTER, value.c_str(), wcslen(OBLIGATION_FSADAPTER)) && emCommand == ATTACHMENT_COMMAND)
			{
				if (!SendEmailData.GetAttachmentData()[nAttachmentPos].IsIgnored())
				{
					AdapterAttachment.SetStripFlag(true);
					OblExistType = SetObligationIntoCache(emCommand,SendEmailData,nAttachmentPos,LOOP_OBLIGATION_FSADAPTER,vec,ca,strBuf);
					if (OblExistType == emOBL_NOT_EXIST) 
					{
						if (SendEmailData.m_AttachmentOblData->GetStripFSFromObl(enforcer->obligation,i,SendEmailData,AdapterAttachment))
						{
							i += 9;
							continue;
						}
					}
				}
			}
			//Outlook: Warning message with proceed/cancel
			else if (0 == _wcsnicmp(OB_OE_WARNING_MSG_PROCEED_CANCEL, value.c_str(), wcslen(OB_OE_WARNING_MSG_PROCEED_CANCEL)) && emCommand == ATTACHMENT_COMMAND)
			{
				logd(L"[CObligations::ClassifyObligationType]nAttachmentPos=%d, %s", nAttachmentPos, value.c_str());
				SendEmailData.m_AttachmentOblData->GetWarningMsgFromObl(enforcer->obligation,i,nAttachmentPos,SendEmailData);
			}
			else if (vecCA.end() != find(vecCA.begin(), vecCA.end(), value.c_str()) && emCommand == BODY_COMMAND)
			//else if (0 == wcsncmp(CONTENTREDACTION_STR, value.c_str(), wcslen(CONTENTREDACTION_STR)) && emCommand == BODY_COMMAND)
			{
				ca.SetCAType(emBODY);
				int nContentRedataType = SendEmailData.m_AttachmentOblData->GetContentReactionFromObl(enforcer->obligation,i,ca);
                NLPRINT_DEBUGVIEWLOG(L"Ob type:[%d]\n", nContentRedataType);    // CONTENTREDACTION_DOB_BODY
				SetObligationIntoCache(emCommand,SendEmailData,nAttachmentPos,nContentRedataType,vec,ca,strBuf);
			}
			//else if (0 == _wcsnicmp(CONTENTREDACTION_STR, value.c_str(), wcslen(CONTENTREDACTION_STR)) && emCommand == SUBJECT_COMMAND)
			else if (vecCA.end() != find(vecCA.begin(), vecCA.end(), value.c_str()) && emCommand == SUBJECT_COMMAND)
			{
				ca.SetCAType(emSubject);
				int nContentRedataType = SendEmailData.m_AttachmentOblData->GetContentReactionFromObl(enforcer->obligation,i,ca);
				SetObligationIntoCache(emCommand,SendEmailData,nAttachmentPos,nContentRedataType,vec,ca,strBuf);
			}
		}

		if (NULL != pdwOblTypes)
		{
			*pdwOblTypes = dwOblTypes;
		}

		if (emCommand == ATTACHMENT_COMMAND)
		{
			if(AdapterAttachment.GetStripFlag())
			{
				AdapterAttachment.SetSrcPath(const_cast<WCHAR *>(SendEmailData.GetAttachmentData()[nAttachmentPos].GetSourcePath().c_str()));
				AdapterAttachment.SetTempPath(const_cast<WCHAR*>(SendEmailData.GetAttachmentData()[nAttachmentPos].GetTempPath().c_str()));
				SendEmailData.m_AttachmentOblData->m_adapterAttachments.AddAttachment(AdapterAttachment);
			}
		}
		
	}
	return nType;
}

void CItemEventDisp::ActionOnQueryPCFinish(CSendEmailData& emailData, VARIANT_BOOL* Cancel, bool bInline, bool bQueryResult)
{
    UNREFERENCED_PARAMETER(bInline);
	if (!bQueryResult)
	{
		std::wstring wstrActionOnQueryPCFailed = theCfg[L"ConditionAction"][L"ActionOnQueryPCFailed"];
		if (wstrActionOnQueryPCFailed.empty())
		{
			wstrActionOnQueryPCFailed = L"allow";//set the default value
		}
		BOOL bAllowSend = _wcsicmp(wstrActionOnQueryPCFailed.c_str(), L"deny")!=0;
		*Cancel = bAllowSend ? VARIANT_FALSE : VARIANT_TRUE;

		//show message
		std::wstring wstrMsg = theCfg[L"ConditionAction"][L"QueryPCFailedMessage"];
		if (wstrMsg.empty())
		{
			wstrMsg = L"Outlook is not able to perform export compliance checking. Please ensure this transmission satisfies all export and compliance marking polices.";//set the default value
		}
		std::wstring wstrMsgChoice = theCfg[L"ConditionAction"][L"QueryPCFailedChoice"];
		if (wstrMsgChoice.empty())
		{
			wstrMsgChoice = L"Click OK to send this email; to review it, click CANCEL.";
		}
		
		if (!wstrMsg.empty())
		{
            if (bAllowSend)
            {
				//wstrMsg += L"\nClick Ok send out email, otherwise click Cancel.";
				wstrMsg += L"\n";
				wstrMsg += wstrMsgChoice;
                if (MessageBoxW(emailData.GetWnd(), wstrMsg.c_str(), MESSAGE_TITLE, MB_OKCANCEL | MB_ICONWARNING) == IDCANCEL)
                {
                    *Cancel = VARIANT_TRUE;
                }
            }
            else
            {
                MessageBoxW(emailData.GetWnd(), wstrMsg.c_str(), MESSAGE_TITLE, MB_OK | MB_ICONWARNING);
            }
		}


		//record log
		if(bAllowSend)
		{
			std::wstring wstrAllowLog= L"Query PC failed but the config.ini allow to this email:";
			wstrAllowLog += L"Sender=";
			wstrAllowLog += emailData.GetSender();
			wstrAllowLog += L" ";

			wstrAllowLog += L"Receiver=";
			STRINGLIST lstReceivers = emailData.GetOriginRecipients();
			STRINGLIST::iterator itRec = lstReceivers.begin();
			while (itRec != lstReceivers.end())
			{
				wstrAllowLog += *itRec;
				wstrAllowLog += L";";
				itRec++;
			}
			wstrAllowLog += L" ";

			wstrAllowLog += L"Subject=";
			wstrAllowLog += emailData.GetSubjectData().GetOriginSubject();
			wstrAllowLog += L" ";

			wstrAllowLog += L"Attachments=";
			std::vector<CAttachmentData>& vecAttachment = emailData.GetAttachmentData();
			std::vector<CAttachmentData>::iterator itAttach = vecAttachment.begin();
			while (itAttach != vecAttachment.end())
			{
				wstrAllowLog += GetFileName(itAttach->GetSourcePath());
				wstrAllowLog += L";";
				itAttach++;
			}
			wstrAllowLog += L" ";

			CEventLog::WriteEventLog(wstrAllowLog.c_str(),EVENTLOG_SUCCESS,EVENTLOG_INFO_ID);
		}
	}
}

