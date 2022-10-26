// All sources, binaries and HTML pages (C) copyright 2004 by Blue Jungle Inc., 
// Redwood City CA, Ownership remains with Blue Jungle Inc, 
// All rights reserved worldwide. 

#ifndef _IIPCRequestHandler_h_
#define _IIPCRequestHandler_h_

#include "stdafx.h"
#include "..\..\..\..\..\common\include\dsipc.h"

/**
 * The IIPRequestHandler interface must be implemented by all concrete request 
 * handlers. IPCStub will dispatch all requests by calling Invoke on instances 
 * of IIPCRequestHandler. 
 * 
 * For an example, see IPCStubTest/StubTestRequestHandler.h, cpp
 *
 * @author fuad
 * @version $Id:
 *          //depot/main/Destiny/main/src/etc/eclipse/destiny-code-templates.xml#2 $:
 */
class DllExport IIPCRequestHandler
{
public:
    IIPCRequestHandler(void);
    virtual ~IIPCRequestHandler(void);

    /**
    *
    * invokes a method. The method name is specified in the request object
    * The implementation of this method must zero out the response object 
    * and set the ulSize parameter of the response object
    * 
    * @param request
    *            request object
    * @param response
    *            pointer response object
    * @return true if invocation is successful
    * 
    */
    virtual bool Invoke(IPCREQUEST& request, IPCREQUEST* pResponse) = 0;

    /**
    * @return name of request handler to use for generating 
    * unique names for OS objects
    */
    virtual TCHAR* GetName () = 0;
};

#endif
