

#include <Windows.h>

#include "dlgunit.hpp"


CDlgUnit::CDlgUnit() : _basecx(0), _basecy(0), _dlucx(0), _dlucy(0), _pixelcx(0), _pixelcy(0)
{
    LONG units = GetDialogBaseUnits();
    _basecx = (int) (units & 0x0000FFFF);
    _basecy = (int) ((units & 0xFFFF0000) >> 16);
}

CDlgUnit::~CDlgUnit()
{
}

void CDlgUnit::SetDluUnits(_In_ int cx, _In_ int cy)
{
    _dlucx = cx;
    _dlucy = cy;
    _pixelcx = MulDiv(cx, _basecx, 4);
    _pixelcy = MulDiv(cy, _basecy, 8);
}

void CDlgUnit::SetPixelUnits(_In_ int cx, _In_ int cy)
{    
    _pixelcx = cx;
    _pixelcy = cy;
    _dlucx = MulDiv(cx, 4, _basecx);
    _dlucy = MulDiv(cy, 8, _basecy);
}