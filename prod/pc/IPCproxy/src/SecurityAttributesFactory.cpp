// All sources, binaries and HTML pages (C) copyright 2004 by Blue Jungle Inc., 
// Redwood City CA, Ownership remains with Blue Jungle Inc, 
// All rights reserved worldwide. 

/**
 * 
 * @author fuad
 * @version $Id:
 *          //depot/main/Destiny/main/src/etc/eclipse/destiny-code-templates.xml#2 $:
 *  
 */

#include "globals.h"
#include "NotQuiteNullDacl.h"
#include "securityattributesfactory.h"

SecurityAttributesFactory::SecurityAttributesFactory(void)
{
}

SecurityAttributesFactory::~SecurityAttributesFactory(void)
{
}

SECURITY_ATTRIBUTES* SecurityAttributesFactory::m_pSecurityAttributes = NULL;

SECURITY_ATTRIBUTES* SecurityAttributesFactory::GetSecurityAttributes ()
{
    if (m_pSecurityAttributes == NULL)
    {
        NotQuiteNullDacl* pDacl = NULL;
        SECURITY_ATTRIBUTES* pSecurityAttributes = NULL;
        SECURITY_DESCRIPTOR* pSD = NULL;
        PACL pDACL = NULL;

        // create the not-quite-null-dacl and security descriptor to 
        // be used for creating all events/filemappings/mutexes
        pDacl = new (std::nothrow) NotQuiteNullDacl ();
        if (NULL == pDacl)
            goto cleanUp;

        if (!pDacl->Create())
            goto cleanUp;

        // declare and initialize a security attributes structure
        pSecurityAttributes = new (std::nothrow) SECURITY_ATTRIBUTES;
        if (NULL == pSecurityAttributes)
            goto cleanUp;

        ZeroMemory( pSecurityAttributes, sizeof( *pSecurityAttributes ) );
        pSecurityAttributes->nLength = sizeof( *pSecurityAttributes );
        pSecurityAttributes->bInheritHandle = FALSE; // object uninheritable

        // declare and initialize a security descriptor
        pSD = new (std::nothrow) SECURITY_DESCRIPTOR;
        if (NULL == pSD)
            goto cleanUp;

        if (!InitializeSecurityDescriptor( pSD, SECURITY_DESCRIPTOR_REVISION ))
            goto cleanUp;

        // assign the dacl to it
        pDACL = pDacl->GetAndDetachPDACL();

        if (!SetSecurityDescriptorDacl( pSD, TRUE, pDACL, FALSE ))
            goto cleanUp;

        pDACL = NULL;

        // Make the security attributes point to the security descriptor
        pSecurityAttributes->lpSecurityDescriptor = pSD;
        pSD = NULL;
        m_pSecurityAttributes = pSecurityAttributes;
        pSecurityAttributes = NULL;

cleanUp:
        if (NULL != pDACL)
            ::HeapFree( GetProcessHeap(), 0, pDACL );
        if (NULL != pSD)
            delete pSD;
        if (NULL != pSecurityAttributes)
            delete pSecurityAttributes;
        if (NULL != pDacl)
            delete pDacl;
    }
    return (m_pSecurityAttributes);

}
