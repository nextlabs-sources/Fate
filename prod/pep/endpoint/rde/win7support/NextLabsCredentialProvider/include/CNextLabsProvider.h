//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//


#include "helpers.h"
#include <windows.h>
#include <strsafe.h>

#include "CNextLabsCredential.h"


#pragma warning(push)
#pragma warning(disable: 6387)
#include <credentialprovider.h>
#pragma warning(pop)

#define MAX_CREDENTIALS 3
#define MAX_DWORD   0xffffffff        // maximum DWORD

class CNextLabsProvider : public ICredentialProvider
{
  public:
    // IUnknown
    STDMETHOD_(ULONG, AddRef)()
    {
        return _cRef++;
    }
    
    STDMETHOD_(ULONG, Release)()
    {
        LONG cRef = _cRef--;
        if (!cRef)
        {
            delete this;
        }
        return cRef;
    }

    STDMETHOD (QueryInterface)(REFIID riid, void** ppv)
    {
        HRESULT hr;
        if (IID_IUnknown == riid || 
            IID_ICredentialProvider == riid)
        {
            *ppv = this;
            reinterpret_cast<IUnknown*>(*ppv)->AddRef();
            hr = S_OK;
        }
        else
        {
            *ppv = NULL;
            hr = E_NOINTERFACE;
        }
        return hr;
    }

  public:
    STDMETHODIMP SetUsageScenario(CREDENTIAL_PROVIDER_USAGE_SCENARIO cpus, DWORD dwFlags);
    STDMETHODIMP SetSerialization(const CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION* pcpcs);

    STDMETHODIMP Advise(_In_ ICredentialProviderEvents* pcpe, UINT_PTR upAdviseContext);
    STDMETHODIMP UnAdvise();

    STDMETHODIMP GetFieldDescriptorCount(_Out_ DWORD* pdwCount);
    STDMETHODIMP GetFieldDescriptorAt(DWORD dwIndex,  _Deref_out_ CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR** ppcpfd);

    STDMETHODIMP GetCredentialCount(_Out_ DWORD* pdwCount,
                                      _Out_ DWORD* pdwDefault,
                                      _Out_ BOOL* pbAutoLogonWithDefault);
    STDMETHODIMP GetCredentialAt(DWORD dwIndex, 
                                   _Out_ ICredentialProviderCredential** ppcpc);

    friend HRESULT CNextLabsProvider_CreateInstance(REFIID riid, _Deref_out_ void** ppv);

  protected:
    CNextLabsProvider();
    ~CNextLabsProvider();
    
  private:
    
    HRESULT _EnumerateOneCredential(_In_ DWORD dwCredientialIndex,
                                    _In_ PCWSTR pwzUsername
                                    );
    HRESULT _EnumerateSetSerialization();

    // Create/free enumerated credentials.
    HRESULT _EnumerateCredentials();
    void _ReleaseEnumeratedCredentials();
    void _CleanupSetSerialization();


private:
    LONG              _cRef;
    CNextLabsCredential *_rgpCredentials[MAX_CREDENTIALS];  // Pointers to the credentials which will be enumerated by 
                                                          // this Provider.
    DWORD                                   _dwNumCreds;
    KERB_INTERACTIVE_UNLOCK_LOGON*          _pkiulSetSerialization;
    DWORD                                   _dwSetSerializationCred; //index into rgpCredentials for the SetSerializationCred
    bool                                    _bAutoSubmitSetSerializationCred;
};