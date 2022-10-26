

#ifndef __NXRM_COMMONUI_DLGUNIT_HPP__
#define __NXRM_COMMONUI_DLGUNIT_HPP__

#include <Windows.h>
#include <Prsht.h>


class CDlgUnit
{
public:
    CDlgUnit();
    virtual ~CDlgUnit();

    void SetDluUnits(_In_ int cx, _In_ int cy);
    void SetPixelUnits(_In_ int cx, _In_ int cy);

    inline int GetDluCx() const throw() {return _dlucx;}
    inline int GetDluCy() const throw() {return _dlucy;}
    inline int GetPixelCx() const throw() {return _pixelcx;}
    inline int GetPixelCy() const throw() {return _pixelcy;}

protected:
    inline int GetBaseCx() const throw() {return _basecx;}
    inline int GetBaseCy() const throw() {return _basecy;}

private:
    int  _basecx;
    int  _basecy;
    int  _dlucx;
    int  _dlucy;
    int  _pixelcx;
    int  _pixelcy;
};

class CSmPropSheetSize : public RECT
{
public:
};


#endif