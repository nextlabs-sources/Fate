

#include <windows.h>
#include <stdio.h>

#include "svccon.hpp"


CSvcCon::CSvcCon()
{
}

CSvcCon::~CSvcCon()
{
}

BOOL CSvcCon::Open(__in LPCWSTR wzName)
{
    return TRUE;
}

BOOL CSvcCon::Create(__in LPCWSTR wzName, __in BOOL bFailIfExist)
{
    return TRUE;
}

BOOL CSvcCon::Delete(__in LPCWSTR wzName)
{
    return TRUE;
}

BOOL CSvcCon::Start()
{
    return TRUE;
}

BOOL CSvcCon::Stop()
{
    return TRUE;
}

BOOL CSvcCon::Status()
{
    return TRUE;
}

BOOL CSvcCon::BootStart()
{
    return TRUE;
}

BOOL CSvcCon::SystemStart()
{
    return TRUE;
}

BOOL CSvcCon::AutoStart()
{
    return TRUE;
}

BOOL CSvcCon::ManualStart()
{
    return TRUE;
}

BOOL CSvcCon::Disable()
{
    return TRUE;
}
