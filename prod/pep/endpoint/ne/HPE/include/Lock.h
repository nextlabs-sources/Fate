#ifndef __LOCK_H__
#define __LOCK_H__
namespace FTPE
{
	class CLock
	{
	public:
		CLock()
		{
			m_hLock = NULL ;
			if ( !CreateLock() )
			{
				m_hLock = NULL ;
			}
		}

		virtual ~CLock()
		{
			DestroyLock() ;
		}
	protected:
		virtual BOOL CreateLock()
		{
			m_hLock = ::CreateEvent( NULL , FALSE , TRUE , NULL ) ;
			if ( m_hLock == NULL )
			{
				return FALSE ;
			}
			return TRUE ;
		}
		virtual BOOL CreateLockByName( wchar_t *pszObjectName)
		{
			m_hLock = CreateEvent( 
				NULL,               // default security attributes
				TRUE,               // manual-reset event
				TRUE,               // initial state is signaled
				pszObjectName  // object name
				); 
			return FALSE ;//reserved
		}
		virtual BOOL DestroyLock()
		{
			// free event
			if ( m_hLock!=NULL )
			{
				CloseHandle( m_hLock ) ;
			} else {
				return FALSE ;
			}
			m_hLock = NULL ;
			return TRUE ;
		}
		virtual VOID Lock()
		{
			if ( m_hLock == NULL )
			{
				return ;
			}
			::WaitForSingleObject( m_hLock , INFINITE ) ;
		}
		virtual BOOL UnLock()
		{
			return ::SetEvent( m_hLock ) ;
		}
	private:
		HANDLE m_hLock ;
	} ;
}
#endif 
