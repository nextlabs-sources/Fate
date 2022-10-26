#pragma once
#include "stdafx.h"
#include <string>
#include <map>
#include <set>
#include <vector>
#include "HookBase.h"


class CUnknown:public IUnknown
{
public:

    virtual HRESULT STDMETHODCALLTYPE QueryInterface( 
        /* [in] */ REFIID riid,
        /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject)
    {	  
		riid ;
		ppvObject ;
        return S_OK; 
    }

    virtual ULONG STDMETHODCALLTYPE AddRef( void)
    { 
        return S_OK; 
    }

    virtual ULONG STDMETHODCALLTYPE Release( void)
    { 
        return S_OK; 
    }

};

class CUccContext: public IUccContext
{
public:

    virtual HRESULT STDMETHODCALLTYPE QueryInterface( 
        /* [in] */ REFIID riid,
        /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject)
    { 
		riid ;
		ppvObject ;
        return S_OK; 
    }

    virtual ULONG STDMETHODCALLTYPE AddRef( void)
    { 
        return S_OK; 
    }

    virtual ULONG STDMETHODCALLTYPE Release( void)
    { 
        return S_OK; 
    }


    virtual HRESULT __stdcall get_Property (
        /*[in]*/ long lPropertyId,
    /*[out,retval]*/ struct IUccProperty * * ppProperty )
    { 
		lPropertyId ;
		ppProperty ;
        return S_OK; 
    }
    virtual HRESULT __stdcall IsPropertySet (
        /*[in]*/ long lPropertyId,
        /*[out,retval]*/ VARIANT_BOOL * pbSet )
    {
		lPropertyId ;
		pbSet ;
        return S_OK; 
    }
    virtual HRESULT __stdcall get_NamedProperty (
        /*[in]*/ BSTR bstrPropertyId,
    /*[out,retval]*/ struct IUccProperty * * ppProperty )
    { 
		bstrPropertyId ;
		ppProperty ;
        return S_OK; 
    }
    virtual HRESULT __stdcall IsNamedPropertySet (
        /*[in]*/ BSTR bstrPropertyName,
        /*[out,retval]*/ VARIANT_BOOL * pbSet )
    { 
		bstrPropertyName ;
		pbSet ;
        return S_OK; 
    }


    virtual HRESULT __stdcall AddProperty (
        /*[in]*/ long lPropertyId,
        /*[in]*/ VARIANT vValue,
    /*[out,retval]*/ struct IUccProperty * * ppProperty )
    { 
		lPropertyId ;
		ppProperty ;
		vValue ;
        return S_OK; 
    }
    virtual HRESULT __stdcall RemoveProperty (
        /*[in]*/ long lPropertyId )
    { 
		lPropertyId ;
        return S_OK; 
    }
    virtual HRESULT __stdcall AddNamedProperty (
        /*[in]*/ BSTR bstrPropertyName,
        /*[in]*/ VARIANT vValue,
    /*[out,retval]*/ struct IUccProperty * * ppProperty ) 
    { 
		bstrPropertyName ;
		vValue ;
		ppProperty ;
        return S_OK; 
    }
    virtual HRESULT __stdcall RemoveNamedProperty (
        /*[in]*/ BSTR bstrPropertyId ) 
    { 
		bstrPropertyId;
        return S_OK;
    }
};

class COperationContext: public IUccOperationContext
{
public:  

    virtual HRESULT STDMETHODCALLTYPE QueryInterface( 
        /* [in] */ REFIID riid,
        /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject)
    { 
        if( riid == IID_IUnknown )
        {
            (*ppvObject) = (IUnknown*)this;
        }
        else if( riid == IID_IUccOperationContext )
        {
            (*ppvObject) = (IUccOperationContext*)this;
        }
        else
        {
            return E_FAIL;
        }
        return S_OK; 
    }

    virtual ULONG STDMETHODCALLTYPE AddRef( void)
    { 
        return S_OK; 
    }

    virtual ULONG STDMETHODCALLTYPE Release( void)
    { 
        return S_OK; 
    }

		virtual HRESULT __stdcall Initialize (
		/*[in]*/ UCCPLONG_PTR lOperationId,
		/*[in]*/ struct IUccContext * pOperation )
    { 
			lOperationId ;
			pOperation ;
			return S_OK; 
    }
    virtual HRESULT __stdcall get_OperationId (
        /*[out,retval]*/ UCCPLONG_PTR * plOperationId )
    { 
        *plOperationId = 6; 
        return S_OK; 
    }
    virtual HRESULT __stdcall get_Context (
    /*[out,retval]*/ struct IUccContext * * ppContext )
    { 
		ppContext ;
        return S_OK; 
    }

};


class CParticipant
{
private:

    IUccSessionParticipant* m_pPart;
    std::wstring m_strName;
    std::wstring m_strRole;
    std::wstring m_strSIP;

public:

    CParticipant(){}
    CParticipant( IUccSessionParticipant* pPart, const std::wstring& strName, const std::wstring& strRole , const std::wstring& strSIP ):
    m_pPart(pPart),
        m_strName(strName),
        m_strRole(strRole),
        m_strSIP(strSIP)
    {}

public:

    void SetName( const std::wstring& strName ){ m_strName = strName; }
    const std::wstring& GetName()const { return m_strName; }
    
    void SetRole( const std::wstring& strRole ){ m_strRole = strRole; }
    const std::wstring& GetRole()const { return m_strRole; }

    void SetSIP( const std::wstring& strSIP ){ m_strSIP = strSIP; }
    const std::wstring& GetSIP()const { return m_strSIP; }

    void SetPart( IUccSessionParticipant* pPart ){ m_pPart = pPart; }
    IUccSessionParticipant* GetPart()const { return m_pPart; }
};

class CPartDB
{
    INSTANCE_DECLARE( CPartDB );

private:

    CPartDB(){ InitializeCriticalSection(&m_Lock); }
    ~CPartDB();

public:

    void SetSession( IUccSession* pSession ){ m_pUccSession = pSession; }
    IUccSession* GetSession(){ return m_pUccSession; }

    void SetLocalPart( const CParticipant& aLocalPart ){ CLock aLock( &m_Lock ); m_LocalPart = aLocalPart; }
    const CParticipant& GetLocalPart()const { return m_LocalPart; }

    void AddNonLocalPart( const CParticipant aNonLocalPart );

    void RemoveNonLocalPart( const std::wstring& aNonLocalPartName );

    void RemoveNonLocalPart( IUccSessionParticipant* pSessPart );
    
    IUccSessionParticipant* GetNonLocalPart( const std::wstring& aNonLocalPartName )
    {
        CLock aLock( &m_Lock );
        std::map< std::wstring, CParticipant >::iterator it = m_mpNonLocalPart.find( aNonLocalPartName );
        if( it != m_mpNonLocalPart.end() )
        {
            return (*it).second.GetPart();
        }
        return 0;
    }

    const std::wstring& GetPresenterAttendeeInfo();

	const std::wstring GetPresenterInfoByName(std::wstring name);

    void GetSIPList( std::vector< std::wstring >& vctSip );
    
private:

    CRITICAL_SECTION        m_Lock;
    IUccSession*            m_pUccSession;

    CParticipant            m_LocalPart;

    COperationContext       m_OperationContext;

    std::map< std::wstring, CParticipant > m_mpNonLocalPart;

    std::wstring            m_strPresenterAttendeeInfo;
};



class CProcessDB
{
    INSTANCE_DECLARE( CProcessDB );
private:

    CProcessDB(){ InitializeCriticalSection(&m_Lock); }
    ~CProcessDB();

    CRITICAL_SECTION        m_Lock;
    std::wstring            m_strProcessName;
    std::set< std::wstring > m_stProcessName;

public:

    void AddName( const TCHAR* pstrName )
    {
        CLock aLock( &m_Lock );
        if( m_stProcessName.find( pstrName ) == m_stProcessName.end() )
        {
            //if( wcscmp( pstrName, TEXT("PWConsole.exe") ) != 0 )
            {
				m_stProcessName.clear() ;
                m_stProcessName.insert( pstrName );
                m_strProcessName += pstrName;
                //m_strProcessName += TEXT('\n');
            }
        }
    }

    const TCHAR* GetName()
    { 
        CLock aLock( &m_Lock );
        return m_strProcessName.c_str(); 
    }

    DWORD GetNameLen()
    { 
        CLock aLock( &m_Lock );
        if( m_strProcessName.empty() )
        {
            return 0;
        }
        else
        {
            return (DWORD)m_strProcessName.size(); 
        }
    }

    void Reset()
    {
        CLock aLock( &m_Lock );
        m_stProcessName.clear();
        m_strProcessName.clear();
    }
};

