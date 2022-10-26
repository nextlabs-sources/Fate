// All sources, binaries and HTML pages (C) copyright 2004 by Blue Jungle Inc., 
// Redwood City CA, Ownership remains with Blue Jungle Inc, 
// All rights reserved worldwide. 

/**
* This code originally came from codeguru.com. It is free to use.
* 
* @see http://www.codeguru.com/Cpp/W-P/win32/tutorials/article.php/c4545
* @author fuad
* @version $Id:
*          //depot/main/Destiny/main/src/etc/eclipse/destiny-code-templates.xml#2 $:
*  
*/

// NotQuiteNullDacl.cpp: implementation of the NotQuiteNullDacl class.
//
// See header file for usage information.
//
//////////////////////////////////////////////////////////////////////

#if defined (WIN32) 
#pragma once

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target other versions of Windows.
#endif						

#include <windows.h>
#include <stdio.h>
#include <tchar.h>
//#include "globals.h"
#include "NotQuiteNullDacl.h"

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
NotQuiteNullDacl::NotQuiteNullDacl()
{
   m_pEveryoneSid = NULL;
   m_pDACL = NULL;

   m_dwErr = 0;
}

NotQuiteNullDacl::~NotQuiteNullDacl()
{
   if ( m_pDACL != NULL )
   {
      ::HeapFree( GetProcessHeap(), 0, m_pDACL );
      m_pDACL = NULL;
   }
   if ( m_pEveryoneSid != NULL )
   {
      ::FreeSid( m_pEveryoneSid );
      m_pEveryoneSid = NULL;
   }
}


//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
// Public methods
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

//********************************************************************
// Create -- All the real work of creating the DACL gets done here 
// rather than in the constructor, so we can return a 
// success/failure status
//
bool NotQuiteNullDacl::Create()
{
   bool bCreateOk = false;
   m_dwErr = 0;

   // need a SID for the Everyone group
   bool bEveryoneSID = CreateEveryoneSid();

   if ( bEveryoneSID )
   {
      // calculate a size for the DACL we're going to use
      ULONG lACLSize = CalculateAclSize();

      if ( lACLSize > 0 )
      {
         // Allocate memory for the ACL
         m_pDACL = (PACL)::HeapAlloc( GetProcessHeap(), 0, lACLSize );

         if ( m_pDACL != NULL )
         {
            // initialize the actual ACL
            BOOL bInitACL = ::InitializeAcl( m_pDACL, lACLSize, ACL_REVISION );
            if ( bInitACL )
            {
               // add the ACEs that deny/grant the accesses we're after
               bCreateOk = AddNotQuiteNullAces();
            }
            else
            {
               m_dwErr = ::GetLastError();
            }
         }
         else
         {
            m_dwErr = ::GetLastError();
         }
      }
   }

   return bCreateOk;
}

//********************************************************************
// GetAndDetachPDACL -- Retrieve the stored DACL, and detach it from
// the object.
//
PACL NotQuiteNullDacl::GetAndDetachPDACL()
{
   PACL pDACL = m_pDACL;
   m_pDACL = NULL;
   return pDACL;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
// Private methods
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

//********************************************************************
// CreateEveryoneSid -- Create a SID for the built-in "Everyone" group
//
bool NotQuiteNullDacl::CreateEveryoneSid()
{
   bool bSidOk = false;
   m_dwErr = 0;

   SID_IDENTIFIER_AUTHORITY SidAuth = SECURITY_WORLD_SID_AUTHORITY;
   BOOL bAllocAndInitOk = ::AllocateAndInitializeSid( &SidAuth,            // authority
                                                      1,                   // count of subauthorities used
                                                      SECURITY_WORLD_RID,  // subauthority we're using
                                                      0, 0, 0, 0, 0, 0, 0, // seven unused subauthorities
                                                      &m_pEveryoneSid );   // pointer to receive the new SID
   if ( bAllocAndInitOk )
   {
      bSidOk = true;
   }
   else
   {
      m_dwErr = ::GetLastError();
   }
   
   return bSidOk;
}


//********************************************************************
// CalculateAclSize -- This method calculates the size of the ACL we
// are about to create.  
// NB:  This is just a special-purpose version based on 
// "Ask Dr. Gui #49".  A more general method is
// outlined in Richter/Clark, Chapter 10, page 455.  This is
// sufficient for the purpose of creating our NotQuiteNullDacl but 
// not for creating more general ACLs.
//
ULONG NotQuiteNullDacl::CalculateAclSize()
{
   ULONG lACLSize = 0;
   m_dwErr = 0;

   // validate that SID we created, just to be sure
   BOOL bSIDOk = ::IsValidSid( m_pEveryoneSid );
   if ( bSIDOk )
   {
      // we're using that SID twice
      lACLSize += ( ::GetLengthSid( m_pEveryoneSid ) * 2 );
      // we're using one access allowed ace and one access denied
      // we substract the length of the sidstart member which 
      // is already counted in the sid lengths above 
      // (See the Initialize ACL documentation for more info.)
      lACLSize += ( sizeof(ACCESS_ALLOWED_ACE) - sizeof(((ACCESS_ALLOWED_ACE*)0)->SidStart) );
      lACLSize += ( sizeof(ACCESS_DENIED_ACE) - sizeof(((ACCESS_DENIED_ACE*)0)->SidStart) );
      // and we need to add in the size of the acl itself
      lACLSize += sizeof( ACL );
   }
   // else -- there's no extended error info to get

   return lACLSize;
}


//********************************************************************
// AddNotQuiteNullAces -- Add the two ACEs to the DACL
// NB:  Note that the order of execution is critical here.
//
bool NotQuiteNullDacl::AddNotQuiteNullAces()
{
   bool bAddedOk = false;
   m_dwErr = 0;

   BOOL bAddAce;
      
   // Add an ace that denies WRITE_OWNER and WRITE_DAC to 'everyone'
   // which denies everyone but the owner of the associated object
   // access to the object's security.
   bAddAce = ::AddAccessDeniedAce( m_pDACL,                   // the acl to add the ace to
                                   ACL_REVISION,              // revision, must be ACL_REVISION
                                   WRITE_OWNER | WRITE_DAC,   // accesses to deny
                                   m_pEveryoneSid );          // SID to be denied (everyone)
   if ( bAddAce )
   {
      // Add an ace that gives 'everyone' all accesses to the object.
      // By itself, the bit of code below would be the moral equivalent
      // of a NULL DACL -- it gives all rights to everyone.  But, because
      // accesses are evaluating in order of their placement in the DACL
      // the DeniedACE above acts as a filter, before this can be evaluated.
      bAddAce = ::AddAccessAllowedAce( m_pDACL,                   // the acl to add the ace to
                                       ACL_REVISION,              // revision, must be ACL_REVISION
                                       GENERIC_ALL,               // accesses to allow
                                       m_pEveryoneSid );          // SID to be allowed
      if ( bAddAce )
      {
         bAddedOk = true;
      }
   }

   if ( !bAddedOk )
   {
      m_dwErr = ::GetLastError();
   }

   return bAddedOk;
}
#endif //WIN32

