 // InspEventDisp.cpp : Implementation of CInspEventDisp

#include "stdafx.h"
#include "InspEventDisp.h"
#include "ItemEventDisp.h"
#include "MailItemUtility.h"
#include "../common/FileCache.h"
 CItemEventList g_ItemCache ;
extern CMeetingItemCache g_MeetingItemCache;

// CInspEventDisp
STDMETHODIMP CInspEventDisp::Invoke(DISPID dispidMember,
                                        REFIID riid,
                                        LCID lcid,
                                        WORD wFlags,
                                        DISPPARAMS* pdispparams,
                                        VARIANT* pvarResult,
                                        EXCEPINFO* pexcepinfo,
                                        UINT* puArgErr)
{
    UNREFERENCED_PARAMETER(riid);
    UNREFERENCED_PARAMETER(lcid);
    UNREFERENCED_PARAMETER(pvarResult);
    UNREFERENCED_PARAMETER(pexcepinfo);
    UNREFERENCED_PARAMETER(puArgErr);
    HRESULT hr = S_OK;


      if	(pdispparams && DISPATCH_METHOD==wFlags)
      {
        switch	(dispidMember)
        {
        case 0xf001:    // Activate
            OnActivate();
            break;
        case 0xf006:    // Deactivate
            OnDeactivate();
            break;
        case 0xf008:    // Close
            OnClose();
            break;
        default:
            hr = DISP_E_MEMBERNOTFOUND;
            break;
        }
      }
      else
      {
        hr = DISP_E_PARAMNOTFOUND;
      }

    return hr;
}
void CInspEventDisp::OnActivate()
{
}

void CInspEventDisp::OnDeactivate()
{
}

void CInspEventDisp::OnClose()
{
	CComPtr<Outlook::_MeetingItem>     spMeetingItem;
	HRESULT hr = m_spMailItem->QueryInterface(Outlook::IID__MeetingItem, (void**)&spMeetingItem);
	if(SUCCEEDED(hr) && spMeetingItem != NULL)
	{
		BSTR bstrEntryID;
		hr = spMeetingItem->get_EntryID(&bstrEntryID);
		if (SUCCEEDED(hr) && bstrEntryID != NULL)
		{
			BSTR bstrCIndex;
			hr = spMeetingItem->get_ConversationIndex(&bstrCIndex);
			if (SUCCEEDED(hr) && bstrCIndex != NULL)
			{
				g_MeetingItemCache.AddMIMember(bstrEntryID, bstrCIndex);
				SysFreeString(bstrCIndex);
			}
			SysFreeString(bstrEntryID);
		}
	}

    if(m_bClosed) return;

#if defined(WSO2K13) || defined(WSO2K16)
	if (m_spMailItem != NULL)
	{
		CComObject<CItemEventDisp> spMailItemDisp(m_spMailItem);
		spMailItemDisp.SetLaunchedBy3rdPartFlag(FALSE);
	}

#endif

    m_bClosed = TRUE;
    DP((L"\n***\n"));
    DP((L"OnClose: this = 0X%X\nm_spMailItem = 0X%X\n", this, m_spMailItem));
    DP((L"***\n"));
    if(m_spMailItem)
    {
        DP((L"Before unadvise m_spMailItem\n"));
		g_ItemCache.DeleteItem( m_spMailItem );
        if(m_spMailItemSinkCookie) AtlUnadvise(m_spMailItem, __uuidof(Outlook::ItemEvents), m_spMailItemSinkCookie);
        DP((L"After unadvise m_spMailItem\n"));
        DP((L"After release m_spMailItemSinkObj\n"));
        m_spMailItemSinkCookie = 0;
        DP((L"After release m_spMailItem\n"));
    }


}

BOOL CInspEventDisp::InitInspSink(CComPtr<Outlook::_Inspector> spInsp, CComPtr<IUnknown> spInpSinkObj, DWORD dwInspSinkCookie)
{
    HRESULT  hr   = S_OK;
    BOOL     bRet = FALSE;
	
	m_spMailItem= 0;
    m_bClosed   = FALSE;
    m_spInspSinkCookie     = dwInspSinkCookie;
    m_spMailItemSinkCookie = 0;
    m_spInspSinkObj        = spInpSinkObj;
    m_spMailItemSinkObj    = 0;
    try
    {
        hr = spInsp->get_CurrentItem((IDispatch**)&m_spMailItem);
        if (FAILED(hr) || 0==m_spMailItem) goto last_exit;

        DP((L"CInspEventDisp::InitInspSink  m_spMailItem = 0X%X\n", m_spMailItem));

        CComObject<CItemEventDisp>* spMailItemSinkObj = 0;
        hr = CComObject<CItemEventDisp>::CreateInstance(&spMailItemSinkObj);
        if(SUCCEEDED(hr) && spMailItemSinkObj)
        {
            DP((L"CInspEventDisp::InitInspSink  spMailItemSinkObj = 0X%X\n", spMailItemSinkObj));
            spMailItemSinkObj->SetMailItemPtr(m_spMailItem);
            spMailItemSinkObj->QueryInterface(IID_IUnknown, (void**)&m_spMailItemSinkObj);
            if(SUCCEEDED(hr) && m_spMailItemSinkObj)
            {
                DP((L"CInspEventDisp::InitInspSink  spMailItemSinkUnk = 0X%X\n", m_spMailItemSinkObj));
                hr = AtlAdvise(m_spMailItem, m_spMailItemSinkObj, __uuidof(Outlook::ItemEvents), &m_spMailItemSinkCookie);
				g_ItemCache.AddItem( m_spMailItem, spMailItemSinkObj );
                if(FAILED(hr))
                {
                    DP((L"CInspEventDisp::InitInspSink  Fail to AtlAdvise mail event\n"));
                }
				spMailItemSinkObj->OnOpen();
            }
            else
            {
                DP((L"CInspEventDisp::InitInspSink  Fail to QueryInterface IID_IUnknown\n"));
            }
        }
        else
        {
            DP((L"CInspEventDisp::InitInspSink  Fail to create CItemEventDisp\n"));
        }
    }
    catch (_com_error e)
    {
    }

last_exit:
    return bRet;
}
