// All sources, binaries and HTML pages (C) copyright 2004 by Blue Jungle Inc., 
// Redwood City CA, Ownership remains with Blue Jungle Inc, 
// All rights reserved worldwide. 

/**
 * 
 * Test Request Handler class
 * 
 * @author fuad
 * @version $Id:
 *          //depot/main/Destiny/main/src/etc/eclipse/destiny-code-templates.xml#2 $:
 *  
 */

#include "StdAfx.h"
#include "..\..\shared\src\globals.h"
#include "..\..\ipcproxy\src\ipcconstants.h"
#include ".\stubtestrequesthandler.h"
#include "..\..\..\..\..\common\include\dsipc.h"

StubTestRequestHandler::StubTestRequestHandler(void)
{
}

StubTestRequestHandler::~StubTestRequestHandler(void)
{
}

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
bool StubTestRequestHandler::Invoke(IPCREQUEST& request, IPCREQUEST* pResponse)
{
    pResponse->ulSize = sizeof (IPCREQUEST);
    if (_tcscmp (request.methodName, _T("method1")) != 0 ||
        _tcscmp (request.params[0], _T ("param1")) != 0 ||
        _tcscmp (request.params[1], _T ("param2")) != 0)
    {
        _tprintf (_T("Incorrect Arguments\n"));
        return (false);
    }
    else
    {
        _tprintf (_T("Thread %d: Method invoked: %s\n"), ::GetCurrentThreadId (), request.methodName);
    }

    _tcsncpy_s (pResponse->params[0], _countof(pResponse->params[0]), _T("retval1"), _TRUNCATE);
    _tcsncpy_s (pResponse->params[1], _countof(pResponse->params[1]), _T("retval2"), _TRUNCATE);

    return (true);
}

/**
* @return name of request handler to use for generating 
* unique names for OS objects
*/
TCHAR* StubTestRequestHandler::GetName ()
{
    return (_T("StubTestRequestHandler"));
}
