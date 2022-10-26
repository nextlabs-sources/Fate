// All sources, binaries and HTML pages (C) copyright 2004 by Blue Jungle Inc., 
// Redwood City CA, Ownership remains with Blue Jungle Inc, 
// All rights reserved worldwide. 

/**
* This code originally came from codeguru.com. It is free to use.
* 
* Usage:
*
    // create the not-quite-null-dacl
    NotQuiteNullDacl Dacl;
    bool bDaclOk = Dacl.Create();

    if ( bDaclOk)
    {
    bool bSetupOk = false;

    // declare and initialize a security attributes structure
    SECURITY_ATTRIBUTES MutexAttributes;
    ZeroMemory( &MutexAttributes, sizeof(MutexAttributes) );
    MutexAttributes.nLength = sizeof( MutexAttributes );
    MutexAttributes.bInheritHandle = FALSE; // object uninheritable

    // declare and initialize a security descriptor
    SECURITY_DESCRIPTOR SD;
    BOOL bInitOk = InitializeSecurityDescriptor( &SD,
                            SECURITY_DESCRIPTOR_REVISION );
    if ( bInitOk )
    {
        // assign the dacl to it
        BOOL bSetOk = SetSecurityDescriptorDacl( &SD,
                                                TRUE,
                                                Dacl.GetPDACL(),
                                                FALSE );
        if ( bSetOk )
        {
            // Make the security attributes point to 
            //the security descriptor
            MutexAttributes.lpSecurityDescriptor = &SD;
            bSetupOk = true;
        }
    }

    if ( bSetupOk )
    {
        // use the security attributes to call CreateMutex
    }
    else
    {
        // the setup failed somehow, use GetLastError() 
        // to find out why
    }
    }
    else
    {
    // nqnDacl create failed, 
    // use Dacl.GetErr() to find out how
    }
* 
* @see http://www.codeguru.com/Cpp/W-P/win32/tutorials/article.php/c4545
* @author fuad
* @version $Id:
*          //depot/main/Destiny/main/src/etc/eclipse/destiny-code-templates.xml#2 $:
*  
*/


// NotQuiteNullDacl.h: interface for the NotQuiteNullDacl class.
// 
// This class allows the user to simply create a Not-Quite-Null 
// Discretionary Access Control List (Dacl).  This Dacl has 
// many of the attributes of the Null Dacl that is often used
// to open up access to file and kernel objects but denies the
// user access to the objects security attributes.  Therefore
// it is less subject to abuse than the simple Null Dacl.
//
// This class is based heavily on the concepts and sample code 
// found in Chapter 10 of Programming Server-Side Applications
// for Microsoft Windows 2000 by Richter and Clark.  It also 
// used ideas and code from the MSDN article "Ask Dr. Gui #49"
//
//////////////////////////////////////////////////////////////////////

#ifndef _NotQuiteNullDacl_h_
#define _NotQuiteNullDacl_h_

#include <aclapi.h>

class NotQuiteNullDacl  
{
public:
    // constructor/destructor
    NotQuiteNullDacl();
    virtual ~NotQuiteNullDacl();

    // accessors
    PACL  GetPDACL() {return m_pDACL;};
    DWORD GetErr() {return m_dwErr;}

    // the real work gets done here
    bool Create();

    PACL GetAndDetachPDACL();

private:
    bool CreateEveryoneSid();
    ULONG CalculateAclSize();
    bool AddNotQuiteNullAces();


private:
    PACL                    m_pDACL;
    PSID                    m_pEveryoneSid;
    DWORD                   m_dwErr;
};

#endif // _NotQuiteNullDacl_h_
