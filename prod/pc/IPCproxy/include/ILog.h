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

#ifndef _ILog_h_
#define _ILog_h_

class ILog
{
public:
    ILog(void);
    virtual ~ILog(void);

    virtual bool IsFatalEnabled () = 0;
    virtual bool IsErrorEnabled () = 0;
    virtual bool IsWarnEnabled () = 0;
    virtual bool IsInfoEnabled () = 0;
    virtual bool IsDebugEnabled () = 0;
    virtual bool IsTraceEnabled () = 0;

    virtual void Fatal (TCHAR* str) = 0;
    virtual void Error (TCHAR* str) = 0;
    virtual void Warn (TCHAR* str) = 0;
    virtual void Info (TCHAR* str) = 0;
    virtual void Debug (TCHAR* str) = 0;
    virtual void Trace (TCHAR* str) = 0;
};

#endif