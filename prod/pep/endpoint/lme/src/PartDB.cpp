#include "stdafx.h"
#include "PartDB.h"
#include "Platform.h"
#include "time.h"

INSTANCE_DEFINE( CPartDB );

void CPartDB::GetSIPList( std::vector< std::wstring >& vctSip )
{
    CLock aLock( &m_Lock );
    vctSip.push_back( m_LocalPart.GetSIP() );

    std::map< std::wstring, CParticipant >::iterator it = m_mpNonLocalPart.begin(); 

    for( ; it != m_mpNonLocalPart.end(); ++it )
    {
        CParticipant& aParticipant = ( *it ).second;
        vctSip.push_back( aParticipant.GetSIP() );
    }
}

const std::wstring CPartDB::GetPresenterInfoByName(std::wstring name)
{
 std::map< std::wstring, CParticipant >::iterator it = m_mpNonLocalPart.begin(); 
 for( ; it != m_mpNonLocalPart.end(); ++it )
 {
	 CParticipant& aParticipant = ( *it ).second;
	 if( _wcsnicmp( aParticipant.GetName().c_str(), name.c_str(), aParticipant.GetName().length()) == 0 )
	 {
		 return aParticipant.GetSIP().c_str() ;
	 }
 }
 return L"";
}
const std::wstring& CPartDB::GetPresenterAttendeeInfo()
{
    CLock aLock( &m_Lock );
    m_strPresenterAttendeeInfo.clear();

    m_strPresenterAttendeeInfo += TEXT("Current Time: ");
    time_t aTime;
    time( &aTime );
    m_strPresenterAttendeeInfo += _wctime( &aTime );

    m_strPresenterAttendeeInfo += TEXT("\n");

    m_strPresenterAttendeeInfo += TEXT("==== Presenter ==== \n ");
    m_strPresenterAttendeeInfo += TEXT( "Name: " ) ;
    m_strPresenterAttendeeInfo += m_LocalPart.GetName();
    m_strPresenterAttendeeInfo += TEXT( "\n Role: " );
    m_strPresenterAttendeeInfo += m_LocalPart.GetRole();
    m_strPresenterAttendeeInfo += TEXT( "\n  SIP: " );
    m_strPresenterAttendeeInfo += m_LocalPart.GetSIP();
    m_strPresenterAttendeeInfo += TEXT("\n\n==== Attendee ====");

    std::map< std::wstring, CParticipant >::iterator it = m_mpNonLocalPart.begin(); 

    for( ; it != m_mpNonLocalPart.end(); ++it )
    {
        CParticipant& aParticipant = ( *it ).second;

        m_strPresenterAttendeeInfo += TEXT( "\n Name: " ) ;
        m_strPresenterAttendeeInfo += aParticipant.GetName();
        m_strPresenterAttendeeInfo += TEXT( "\n Role: " );
        m_strPresenterAttendeeInfo += aParticipant.GetRole();
        m_strPresenterAttendeeInfo += TEXT( "\n  SIP: " );
        m_strPresenterAttendeeInfo += aParticipant.GetSIP();
    }

    return m_strPresenterAttendeeInfo;
}


void CPartDB::RemoveNonLocalPart( const std::wstring& aNonLocalPartName )
{
    CLock aLock( &m_Lock );
    std::map< std::wstring, CParticipant >::iterator it = m_mpNonLocalPart.find( aNonLocalPartName );
    if( it != m_mpNonLocalPart.end() )
    {     
        //NewSession_RemoveParticipant( GetSession(), (*it).second.GetPart(), &m_OperationContext );
        GetSession()->RemoveParticipant( (*it).second.GetPart(), &m_OperationContext );
        GetSession()->Terminate( UCCTR_NORMAL, 0 );
        m_mpNonLocalPart.erase( it ); 
    }
}


void CPartDB::AddNonLocalPart( const CParticipant aNonLocalPart )
{
    CLock aLock( &m_Lock );
    bool bShowInfo = false;

    std::wstring strInfo( TEXT( "== Just Joined in ==: \n" ) );

    const std::wstring& aName = aNonLocalPart.GetName();
    if( m_mpNonLocalPart.find( aName ) == m_mpNonLocalPart.end() )
    {
        m_mpNonLocalPart[aName] = aNonLocalPart; 
        /*if( !MsgBoxAllowOrDeny( aNonLocalPart.GetName().c_str(), TEXT("Allow This Participant to Join in?") ) )
        {
            RemoveNonLocalPart( aName );
        }
        else*/
        {
            bShowInfo = true;

            strInfo += TEXT( "Name: " );
            strInfo += aNonLocalPart.GetName();
            strInfo += TEXT( "\nRole: " );
            strInfo += aNonLocalPart.GetRole();
            strInfo += TEXT( "\n SIP: " );
            strInfo += aNonLocalPart.GetSIP();        
        }
    }   

    if( !bShowInfo )
    {
        return;
    }

    strInfo += TEXT( "\n\n== Already Joined in ==: \n" ) ;

    const CParticipant& aParticipant = GetLocalPart();
    if( !aParticipant.GetName().empty() )
    {
        strInfo += TEXT( "Name: " ) ;
        strInfo += aParticipant.GetName();
        strInfo += TEXT( "\nRole: " );
        strInfo += aParticipant.GetRole();
        strInfo += TEXT( "\n SIP: " );
        strInfo += aParticipant.GetSIP();
    }

    std::map< std::wstring, CParticipant >::iterator it = m_mpNonLocalPart.begin(); 

    for( ; it != m_mpNonLocalPart.end(); ++it )
    {
        CParticipant& aParticipant_l = ( *it ).second;

        if( aParticipant_l.GetName() != aNonLocalPart.GetName() )
        {
            strInfo += TEXT( "\n\nName: " ) ;
            strInfo += aParticipant_l.GetName();
            strInfo += TEXT( "\nRole: " );
            strInfo += aParticipant_l.GetRole();
            strInfo += TEXT( "\n SIP: " );
            strInfo += aParticipant_l.GetSIP();
        }
    }

   // DemoShowInfo( strInfo.c_str(), TEXT("Participant Joined in") );
}


void CPartDB::RemoveNonLocalPart( IUccSessionParticipant* pSessPart )
{
    CLock aLock( &m_Lock );
    std::map< std::wstring, CParticipant >::iterator it = m_mpNonLocalPart.begin();

    bool bShowInfo = false;

    for( ; it != m_mpNonLocalPart.end(); ++it )
    {
        if( (*it).second.GetPart() == pSessPart )
        {
            m_mpNonLocalPart.erase( it );
            bShowInfo = true;
            break;
        }
    }

    if( !bShowInfo )
    {
        return;
    }

    /*const CParticipant& aParticipant = GetLocalPart();
    std::wstring strInfo( TEXT( "Name: " ) );
    strInfo += aParticipant.GetName();
    strInfo += TEXT( "\nRole: " );
    strInfo += aParticipant.GetRole();
    strInfo += TEXT( "\n SIP: " );
    strInfo += aParticipant.GetSIP();

    it = m_mpNonLocalPart.begin();

    for( ; it != m_mpNonLocalPart.end(); ++it )
    {
        CParticipant& aParticipant = ( *it ).second;

        strInfo += TEXT( "\n\nName: " ) ;
        strInfo += aParticipant.GetName();
        strInfo += TEXT( "\nRole: " );
        strInfo += aParticipant.GetRole();
        strInfo += TEXT( "\n SIP: " );
        strInfo += aParticipant.GetSIP();
    }*/

    

 //   DemoShowInfo( GetPresenterAttendeeInfo().c_str(), TEXT("A Participant has Left, remain participant: ") );
}


INSTANCE_DEFINE( CProcessDB );
