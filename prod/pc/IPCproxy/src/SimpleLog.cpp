#include "globals.h"
#include ".\simplelog.h"

SimpleLog::SimpleLog(void)
{
    m_bFatal = false;
    m_bError = false;
    m_bWarn = false;
    m_bInfo = false;
    m_bDebug = false;
    m_bTrace = false;
}

SimpleLog::~SimpleLog(void)
{
}
