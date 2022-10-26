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

#ifndef _SimpleLog_h_
#define _SimpleLog_h_

#include "ilog.h"

class SimpleLog : public ILog
{
public:
    SimpleLog(void);
    virtual ~SimpleLog(void);

    virtual bool IsFatalEnabled () {return m_bFatal;}
    virtual bool IsErrorEnabled (){return m_bError;}
    virtual bool IsWarnEnabled (){return m_bWarn;}
    virtual bool IsInfoEnabled (){return m_bInfo;}
    virtual bool IsDebugEnabled (){return m_bDebug;}
    virtual bool IsTraceEnabled (){return m_bTrace;}

    void WriteStringToFile (TCHAR* str)
    {
        //FILE *fp_log = fopen ("C:\\agentlog.txt", "a");
        //if (fp_log != NULL)
        //{
        //    _ftprintf (fp_log, _T("%s\n"), str);
        //    fclose (fp_log);
        //}
    }

    virtual void Fatal (TCHAR* str) {if (m_bFatal) _tprintf (str); WriteStringToFile (str);}
    virtual void Error (TCHAR* str) {if (m_bError) _tprintf (str); WriteStringToFile (str);}
    virtual void Warn (TCHAR* str) {if (m_bWarn) _tprintf (str); WriteStringToFile (str);}
    virtual void Info (TCHAR* str) {if (m_bInfo) _tprintf (str); WriteStringToFile (str);}
    virtual void Debug (TCHAR* str) {if (m_bDebug) _tprintf (str); WriteStringToFile (str);}
    virtual void Trace (TCHAR* str) {if (m_bTrace) _tprintf (str); WriteStringToFile (str);}

private:
    bool m_bFatal;
    bool m_bError;
    bool m_bWarn;
    bool m_bInfo;
    bool m_bDebug;
    bool m_bTrace;
};

#endif