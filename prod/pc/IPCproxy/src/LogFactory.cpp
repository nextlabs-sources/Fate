#include "globals.h"
#include "simplelog.h"
#include ".\logfactory.h"

LogFactory::LogFactory(void)
{
}

LogFactory::~LogFactory(void)
{
}

ILog* LogFactory::m_pLog = NULL;

/**
 * @return an instance of SimpleLog
 */
ILog* LogFactory::GetLogger ()
{
    if (!LogFactory::m_pLog)
    {
        LogFactory::m_pLog = new SimpleLog();
    }
    return (LogFactory::m_pLog);
}
