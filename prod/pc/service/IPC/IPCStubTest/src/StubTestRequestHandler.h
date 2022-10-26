// All sources, binaries and HTML pages (C) copyright 2004 by Blue Jungle Inc., 
// Redwood City CA, Ownership remains with Blue Jungle Inc, 
// All rights reserved worldwide. 

#ifndef _StubTestRequestHandler_h_
#define _StubTestRequestHandler_h_


#include "..\..\ipcstub\src\iipcrequesthandler.h"
/**
 * 
 * Test Request Handler class
 * 
 * @author fuad
 * @version $Id:
 *          //depot/main/Destiny/main/src/etc/eclipse/destiny-code-templates.xml#2 $:
 *  
 */
class StubTestRequestHandler :
    public IIPCRequestHandler
{
public:
    StubTestRequestHandler(void);
    ~StubTestRequestHandler(void);

    /**
    * prints the method name and thread id of the thread that handler
    * the request. Prints an error of the parameters are not as expected.
    *
    * @param request
    *            request object
    * @param response
    *            pointer response object
    * @return true if invocation is successful
    * 
    */
    virtual bool Invoke(IPCREQUEST& request, IPCREQUEST* pResponse);

    /**
    * @return name of request handler to use for generating 
    * unique names for OS objects
    */
    virtual TCHAR* GetName ();

};

#endif
