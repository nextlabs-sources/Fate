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

#ifndef _SecurityAttributesFactory_h_
#define _SecurityAttributesFactory_h_

class SecurityAttributesFactory
{
private:
    static SECURITY_ATTRIBUTES* m_pSecurityAttributes;

public:
    SecurityAttributesFactory(void);
    ~SecurityAttributesFactory(void);

    static SECURITY_ATTRIBUTES* GetSecurityAttributes ();
};

#endif