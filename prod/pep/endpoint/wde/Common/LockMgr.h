/*
 * LockMgr.h 
 * Author: Helen Friedland
 * All sources, binaries and HTML pages (C) copyright 2004 by Blue Jungle Inc., 
 * Redwood City CA, Ownership remains with Blue Jungle Inc, 
 * All rights reserved worldwide. 
 */


#ifndef _LOCKMGR_H_
#define _LOCKMGR_H_

#include <boost/utility.hpp>

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <windows.h>

//---------------------------------------------------------------------------
//
// class CCSWrapper 
//
// Win32 CRTICIAL_SECTION user object wrapper
//
//---------------------------------------------------------------------------
class CCSWrapper
{
public:
	CCSWrapper()
	{
	  ::InitializeCriticalSection( &m_cs );
	}
	virtual ~CCSWrapper()
	{
	  ::DeleteCriticalSection( &m_cs );
	}
	// 
	// This function waits for ownership of the specified critical section object 
	// 
	void Enter()
	{
	  ::EnterCriticalSection( &m_cs );
	}
	//
	// Releases ownership of the specified critical section object. 
	// 
	void Leave()
	{
	  ::LeaveCriticalSection( &m_cs );
	}
private:
	CRITICAL_SECTION m_cs;
};



//---------------------------------------------------------------------------
//
// class CLockMgr  
//
// Provides the access-control mechanism used in controlling access to a resource 
// in a multithreaded environment. This class is used in combination with 
// CCSWrapper and rather than direct calls for locking and unlocking shared 
// resources, it performs it in the constructor and the destructor of the class. 
// Having this approach we can just simply instantiate an object of type CCSWrapper 
// on the stack in the beginning of the target method. The object will be 
// automatically released when it goes out of the scope. This solves the issues 
// with exception handling of the protected by CCSWrapper code.
//
//---------------------------------------------------------------------------
template <class T>
class CLockMgr : boost::noncopyable
{
public:
	//
	// Constructor
	//
	CLockMgr(T& lockObject, BOOL bEnabled):
		m_rLockObject( lockObject ),
		m_bEnabled( bEnabled )
	{
		if ( m_bEnabled )
			m_rLockObject.Enter();
	}
	//
	// Destructor
	//
	virtual ~CLockMgr()
	{
		if ( m_bEnabled )
			m_rLockObject.Leave();
	}
private:
	T&   m_rLockObject;
	BOOL m_bEnabled;
};

#endif //_LOCKMGR_H_

//--------------------- End of the file -------------------------------------
