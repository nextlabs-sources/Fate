// InspsEventDisp.cpp : Implementation of CInspsEventDisp

#include "stdafx.h"
#include "InspsEventDisp.h"
#include "InspEventDisp.h"
#include "outlookUtilities.h"


// CInspsEventDisp
STDMETHODIMP CInspsEventDisp::Invoke(DISPID dispidMember,
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

      if	(pdispparams && DISPATCH_METHOD==wFlags)
      {
        switch	(dispidMember)
        {
        case 0xf001:    // NewInspector
        {
            CComQIPtr<IDispatch> spDispatch(pdispparams->rgvarg[0].pdispVal);
            CComQIPtr<Outlook::_Inspector> spInspector(spDispatch);
            OnNewInspector(spInspector);
            break;
        }
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

void CInspsEventDisp::OnNewInspector(CComQIPtr<Outlook::_Inspector> Inspector)
{
    HRESULT hr = S_OK;
    CComObject<CInspEventDisp>*	pInspDisp = NULL;

	{
		VARIANT_BOOL				bIsWord=0;
		hr=Inspector->IsWordMail(&bIsWord);
		if(SUCCEEDED(hr)&&bIsWord==0)
			return ;
	}

    if(Inspector)
    {
        CComPtr<IDispatch>  lpDisp = 0;
        hr = Inspector->get_CurrentItem(&lpDisp);
        if(SUCCEEDED(hr) && lpDisp)
        {
			ITEM_TYPE mailItemType = DEFAULT ;
			if(OLUtilities::CheckGetMailItemType(lpDisp,mailItemType) == TRUE)
            {
              /*  spCurMailItam->Release();*/
                hr = CComObject<CInspEventDisp>::CreateInstance(&pInspDisp);
                if(SUCCEEDED(hr) && pInspDisp)
                {
                    DP((L"\n***\npInspector = 0X%X\npInspDisp = 0X%X\n", Inspector, pInspDisp));
                    CComPtr<IUnknown> spInspSinkObj = NULL;
                    hr = pInspDisp->QueryInterface(IID_IUnknown, (void**)&spInspSinkObj);
                    if(SUCCEEDED(hr) && spInspSinkObj)
                    {
                        DWORD dwInspSinkCookie = 0;
                        hr = AtlAdvise(Inspector, spInspSinkObj, __uuidof(Outlook::InspectorEvents), &dwInspSinkCookie);
                        if(SUCCEEDED(hr) && dwInspSinkCookie)
                        {
                            pInspDisp->InitInspSink(Inspector, spInspSinkObj, dwInspSinkCookie);
                        }
                    }                    
                    DP((L"***\n"));
                }
            }
        }
    }

}
