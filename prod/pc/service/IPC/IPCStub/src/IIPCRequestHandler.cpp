// All sources, binaries and HTML pages (C) copyright 2004 by Blue Jungle Inc., 
// Redwood City CA, Ownership remains with Blue Jungle Inc, 
// All rights reserved worldwide. 

/**
 * 
 * @author fuad
 * @version $Id:
 *          //depot/main/Destiny/main/src/etc/eclipse/destiny-code-templates.xml#2 $:
 */

#include "StdAfx.h"
#include "..\..\shared\src\globals.h"
#include ".\iipcrequesthandler.h"

IIPCRequestHandler::IIPCRequestHandler(void)
{
}

IIPCRequestHandler::~IIPCRequestHandler(void)
{
}


/**
* @return name of request handler to use for generating 
* unique names for OS objects
*/
TCHAR* IIPCRequestHandler::GetName ()
{
    return (NULL);
}
