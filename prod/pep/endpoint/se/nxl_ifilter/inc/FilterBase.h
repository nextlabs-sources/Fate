#pragma once
#include <shlwapi.h>
#include <propkey.h>
#include <propsys.h>
#include <filter.h>
#include <filterr.h>
#include <io.h>
#include <atlcomcli.h>

GUID const CLSID_NXLFILTER =
{
    0x38522F37,
    0xC617,
    0x4438,
    { 0xBA, 0x5E, 0x77, 0xBA, 0x8B, 0x65, 0x52, 0x37 }
};
//SZ_NXLFILTER_CLSID L"{38522F37-C617-4438-BA5E-77BA8B655237}"


class CFilterBase : public IFilter, public IPersistFile, public IPersistStream
{

public:
    CFilterBase() :m_pFilt(NULL)
    {
    }

    virtual ~CFilterBase()
    {
    }

    // IFilter
    IFACEMETHODIMP Init(ULONG grfFlags, ULONG cAttributes, const FULLPROPSPEC *aAttributes, ULONG *pFlags);
    IFACEMETHODIMP GetChunk(STAT_CHUNK *pStat);
    IFACEMETHODIMP GetText(ULONG *pcwcBuffer, WCHAR *awcBuffer);
    IFACEMETHODIMP GetValue(PROPVARIANT **ppPropValue);
    IFACEMETHODIMP BindRegion(FILTERREGION, REFIID, void **)
    {
        return E_NOTIMPL;
    }

	//IPersist
	IFACEMETHODIMP GetClassID( CLSID * pClassID );

	//IPersistFile
    IFACEMETHODIMP  IsDirty();
    IFACEMETHODIMP  Load( LPCWSTR pszFileName, DWORD dwMode);
    IFACEMETHODIMP  Save( LPCWSTR pszFileName, BOOL fRemember );
    IFACEMETHODIMP  SaveCompleted( LPCWSTR pszFileName );
    IFACEMETHODIMP  GetCurFile( LPWSTR  * ppszFileName );

	//IPersistStream
    IFACEMETHODIMP Load(IStream *pStm);
	IFACEMETHODIMP Save(IStream *pStm, BOOL fClearDirty);
	IFACEMETHODIMP GetSizeMax(ULARGE_INTEGER *pcbSize);


private:

protected:
	CComPtr<IFilter>					m_pFilt;
	WCHAR          m_tempFile[MAX_PATH];
};
