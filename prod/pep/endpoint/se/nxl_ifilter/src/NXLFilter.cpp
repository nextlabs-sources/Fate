#include <new>
#include "FilterBase.h"

#ifdef _X86_
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.VC90.CRT' version='9.0.21022.8' processorArchitecture='x86' publicKeyToken='1fc8b3b9a1e18e3b' language='*'\"") 
#else ifdef _AMD64_
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.VC90.CRT' version='9.0.21022.8' processorArchitecture='amd64' publicKeyToken='1fc8b3b9a1e18e3b' language='*'\"") 
#endif

void DllAddRef();
void DllRelease();

class CNXLFilter : public CFilterBase
{
public:
    CNXLFilter() : m_cRef(1)
    {
        DllAddRef();
    }

    ~CNXLFilter()
    {
        DllRelease();
    }

    // IUnknown
    IFACEMETHODIMP QueryInterface(REFIID riid, void ** ppv)
    {
        static const QITAB qit[] =
        {
            QITABENT(CNXLFilter, IFilter),
			QITABENT(CNXLFilter, IPersistFile),
			QITABENT(CNXLFilter, IPersistStream),
            {0, 0},
        };
        return QISearch(this, qit, riid, ppv);
    }

    IFACEMETHODIMP_(ULONG) AddRef()
    {
        return InterlockedIncrement(&m_cRef);
    }

    IFACEMETHODIMP_(ULONG) Release()
    {
        long cRef = InterlockedDecrement(&m_cRef);
        if (cRef == 0)
        {
            delete this;
        }
        return cRef;
    }

private:

    long m_cRef;
};

HRESULT CNXLFilter_CreateInstance(REFIID riid, void **ppv)
{
    HRESULT hr = E_OUTOFMEMORY;
    CNXLFilter *pFilter = new (std::nothrow) CNXLFilter();
    if (pFilter)
    {
        hr = pFilter->QueryInterface(riid, ppv);
        pFilter->Release();
    }
    return hr;
}
