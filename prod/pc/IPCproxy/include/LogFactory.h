// All sources, binaries and HTML pages (C) copyright 2004 by Blue Jungle Inc., 
// Redwood City CA, Ownership remains with Blue Jungle Inc, 
// All rights reserved worldwide. 

/**
 * 
 * IPCProxy test program
 * 
 * @see #_tmain() for usage instructions
 * 
 * @author fuad
 * @version $Id:
 *          //depot/main/Destiny/main/src/etc/eclipse/destiny-code-templates.xml#2 $:
 *  
 */
#ifndef _LogFactory_h_
#define _LogFactory_h_

class LogFactory
{

private:
    static ILog* m_pLog;

public:
    LogFactory(void);
    ~LogFactory(void);

    static ILog* GetLogger ();
};

#endif