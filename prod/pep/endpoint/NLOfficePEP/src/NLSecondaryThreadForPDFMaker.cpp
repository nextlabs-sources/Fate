#include "stdafx.h"
#include <utils.h>
#include "NLOfficePEP_Comm.h"
#include "NLSecondaryThreadForPDFMaker.h"

#define NLBUFFER_LENGTH 512

#define NLPDFMAKER_CONTROLID_NO 0x07

////////////////////////////CELog2//////////////////////////////////////////////
// for CELog2 we should define: CELOG_CUR_FILE
#define CELOG_CUR_FILE static_cast<celog_filepathint_t>(CELOG_FILEPATH_OFFICEPEP_MIN + EMNLFILEINTEGER_NLSECONDARYTHREADFORPDFMAKER)
//////////////////////////////////////////////////////////////////////////

CNLSecondaryThreadForPDFMaker::CNLSecondaryThreadForPDFMaker(void) 
						: m_bNeedExitThread( FALSE ), 
						m_hThread( NULL ), 
						m_hPDFMakerConvertEvent( NULL ), 
						m_bInitializeFlag( FALSE )
{
}

CNLSecondaryThreadForPDFMaker::~CNLSecondaryThreadForPDFMaker(void)
{

}

CNLSecondaryThreadForPDFMaker& CNLSecondaryThreadForPDFMaker::NLGetInstance()
{
	static CNLSecondaryThreadForPDFMaker theIns;
	return theIns;
}

bool CNLSecondaryThreadForPDFMaker::NLStartPDFMakerSecondaryThread( )
{
	static const wchar_t* g_pwchPDFMakerOfficeAddInKey = L"SOFTWARE\\Microsoft\\Office\\Word\\Addins\\PDFMaker.OfficeAddin";
	bool bRet = true;
	NLSetThreadInitializeFlag( FALSE );

	//HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Office\Word\Addins\PDFMaker.OfficeAddin
	HKEY hKey = NULL;
	long lResult = RegOpenKeyExW( HKEY_LOCAL_MACHINE, g_pwchPDFMakerOfficeAddInKey, 0, KEY_READ, &hKey);
	if( ERROR_SUCCESS == lResult )     
	{	
		RegCloseKey(hKey);
		
		bRet = false;
		NLSetThreadExitFlag( FALSE );

		// use an event object to control the secondary thread ( Note: second parameter is TRUE, manual reset )
		m_hPDFMakerConvertEvent = CreateEventW( NULL, TRUE, FALSE, NULL );
		if ( NULL != m_hPDFMakerConvertEvent )
		{
			// create a thread to deny convert action what cause by adobe PDFMaker
			m_hThread = (HANDLE)_beginthreadex( NULL, 0, &NLFindPDFMakerConvertWindow, this, 0, NULL );
			if ( NULL == m_hThread )
			{
				NLCloseHandle( m_hPDFMakerConvertEvent );
			}
			else
			{
				bRet = true;
			}
		}
		NLSetThreadInitializeFlag( bRet ? TRUE : FALSE );
	}
	return bRet;
}

void CNLSecondaryThreadForPDFMaker::NLStartDenyPDFMakerConvert()
{
	NLActivateThreadEvent( true );
}

void CNLSecondaryThreadForPDFMaker::NLExitPDFMakerSecondaryThread()
{
	if ( NLGetThreadInitializeFlag() )
	{
		NLSetThreadExitFlag( TRUE );
		NLActivateThreadEvent( true );

		if ( NULL != m_hThread )
		{
			WaitForSingleObject( m_hThread, INFINITE );
		}

		NLCloseHandle( m_hThread );
		NLCloseHandle( m_hPDFMakerConvertEvent );
		
		NLSetThreadInitializeFlag( FALSE );
	}
}

void CNLSecondaryThreadForPDFMaker::NLSetThreadExitFlag( _In_ BOOL bNeedExitThread )
{
	InterlockedExchange( reinterpret_cast<LONG*>( &m_bNeedExitThread ), static_cast<LONG>( bNeedExitThread ) );
}

bool CNLSecondaryThreadForPDFMaker::NLGetThreadExitFlag()
{
	return m_bNeedExitThread;
}

void CNLSecondaryThreadForPDFMaker::NLSetThreadInitializeFlag( _In_ BOOL bInitializeFlag )
{
	InterlockedExchange( reinterpret_cast<LONG*>( &m_bInitializeFlag ), static_cast<LONG>( bInitializeFlag ) );
}

bool CNLSecondaryThreadForPDFMaker::NLGetThreadInitializeFlag()
{
	return m_bInitializeFlag;
}

void CNLSecondaryThreadForPDFMaker::NLActivateThreadEvent( _In_ const bool bActivateEvent )
{
	if ( NULL != m_hPDFMakerConvertEvent )
	{
		if ( bActivateEvent )
		{
			SetEvent( m_hPDFMakerConvertEvent );
		} 
		else
		{
			ResetEvent( m_hPDFMakerConvertEvent );
		}
	}
}

// Secondary thread to deny convert action what cause by adobe PDFMaker 
unsigned __stdcall CNLSecondaryThreadForPDFMaker::NLFindPDFMakerConvertWindow( void* pArguments )
{NLCELOG_ENTER
	if ( NULL == pArguments )
	{
		NLPRINT_DEBUGLOG( L"!!!the parameter pass into the secondary thread for PDFMaker is wrong, please check it \n" );
		NLCELOG_RETURN_VAL( 1 )
	}
	
	CNLSecondaryThreadForPDFMaker* pThis = static_cast<CNLSecondaryThreadForPDFMaker*>(pArguments);
	
	while ( true )
	{
		WaitForSingleObject( pThis->m_hPDFMakerConvertEvent, INFINITE );		
		pThis->NLActivateThreadEvent( FALSE ); // wait for the next operation

		if ( pThis->NLGetThreadExitFlag() )
		{
			// office file close, PDFMaker secondary thread exit
			pThis->NLSetThreadExitFlag( FALSE );
			break;
		}
		
		/* 
			do deny.
			NOTE: return FALSE means success to find the window and do deny 		
		*/
	
		for ( int nCount = 0; nCount<50; nCount++ )
		{
			if ( EnumWindows( EnumTopProc, NULL ) )
			{
				// do deny failed, sometime the secondary thread run too fast so that it finish to find the target window before it create
				NLPRINT_DEBUGLOG( L"!!!Failed to deny the PDFMaker convert action, please check \n" );
				Sleep( 100 );
				continue;
			}
			break;
		}

		Sleep(100);
	}

	NLCELOG_RETURN_VAL( 0 )
}

BOOL CALLBACK CNLSecondaryThreadForPDFMaker::EnumTopProc( HWND hwnd, LPARAM lParam )
{
	if( hwnd != NULL )
	{
		DWORD dwProcessId = GetCurrentProcessId();
		DWORD dwHwndProcId = 0;
		GetWindowThreadProcessId( hwnd, &dwHwndProcId );

		if ( dwHwndProcId == dwProcessId )
		{
			wchar_t wszCaption[NLBUFFER_LENGTH]   = {0};
			wchar_t wszClassName[NLBUFFER_LENGTH] = {0};
			::GetClassNameW( hwnd, wszClassName, NLBUFFER_LENGTH-1 );
			
			NLPRINT_DEBUGLOG( L"The window class name is:[%s] --- \n", wszClassName );
			if ( 0 == _wcsicmp( L"#32770", wszClassName ) || 0 == _wcsicmp( L"#32770 (Dialog)", wszClassName ) )
			{
				NLPRINT_DEBUGLOG( L"The window class name is:[%s], caption is:[%s] \n", wszClassName, wszCaption );
				::GetWindowTextW( hwnd, wszCaption,NLBUFFER_LENGTH-1 );
				if ( 0 == _wcsicmp( wszCaption, L"Acrobat PDFMaker" ) ||
					0 == _wcsicmp( wszCaption, L"Acrobat PDFMaker - Mail Merge" ) ||
					0 == _wcsicmp( wszCaption, L"Save Adobe PDF File As" ) ||
					boost::algorithm::istarts_with( wszCaption, L"Import Comments from Adobe" ) )
				{
					NLPRINT_DEBUGLOG( L"deny PDFMaker convert. The window class name is:[%s], caption is:[%s], HWnd:[0x%x] \n", wszClassName, wszCaption, hwnd );

					// find it and to do deny, end PDFMaker convert					
					// check if it is the window that prompt the user to save the file before do convert 
					HWND hNo = GetDlgItem( hwnd, NLPDFMAKER_CONTROLID_NO );
					
					if ( NULL != hNo )
					{
						wchar_t wszCaptionNo[NLBUFFER_LENGTH]   = {0};
						::GetWindowTextW( hNo, wszCaptionNo, NLBUFFER_LENGTH-1 );
						
						if ( 0 == _wcsicmp( wszCaption, L"Acrobat PDFMaker" ) && 0 == _wcsicmp( wszCaptionNo, L"&No" ) )
						{
							NLPRINT_DEBUGLOG( L"close prompt dialog \n" );
							::PostMessage( hwnd, WM_COMMAND, NLPDFMAKER_CONTROLID_NO, 0 ); 
							
							return FALSE;
						}
					}

					// normal
					::PostMessage( hwnd, WM_CLOSE, 0, 0 );
					return FALSE;
				}
			}
		}
	}
	return TRUE;
}
