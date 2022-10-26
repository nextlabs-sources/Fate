
#ifndef _NUDF_IMG_BITMAP_HPP__
#define _NUDF_IMG_BITMAP_HPP__


#include <Windows.h>

#include <string>

namespace nudf {
namespace image {


class CBitmap
{
public:
    CBitmap();
    CBitmap(HBITMAP hBitmap, COLORREF clrBK);
    virtual ~CBitmap();

    inline HBITMAP GetHandle() const throw() {return _hbmp;}
    inline COLORREF GetBkColor() const throw() {return _clrBK;}
    inline void SetBkColor(COLORREF clrBK) {_clrBK = clrBK;}

    inline void Attach(HBITMAP hBitmap, COLORREF clrBK) {_hbmp = hBitmap; _clrBK = clrBK;}
    inline HBITMAP Detach() const throw() {HBITMAP h = _hbmp; _hbmp = NULL; return h;}

    virtual bool Create(_In_opt_ HDC hDC, _In_ int cx, _In_ int cy, _In_ COLORREF clrBK);
    virtual void Clear();
    bool Rotate(_In_opt_ HDC hDC, _In_ int nAngle);
    bool SetTransparency(_In_ ULONG ratio);
	bool SetAlphaChannel(_In_ ULONG percentage, COLORREF _clrForground);
    CBitmap& operator = (const CBitmap& bmp);
    bool ToFile(_In_ const wchar_t* file) const throw();
	bool ToPNGFile(_In_ const wchar_t* file) const throw();

    
    static const float PI;


private:
	int GetEncoderClsid(const WCHAR* format, CLSID* pClsid) const throw();
    mutable HBITMAP _hbmp;
    COLORREF    _clrBK;
	ULONG_PTR   _gdiplusToken;
};

class CTextBitmap : public CBitmap
{
public:
    CTextBitmap();
    virtual ~CTextBitmap();

    enum Alignment {
        Center = 0,
        Left,
        Right
    };
    
    bool Create(_In_ LPCWSTR wzText, _In_ HFONT hFont, _In_ COLORREF clrBK, _In_ COLORREF clrFont, _In_ Alignment alignment=Center);
    bool Create(_In_ LPCWSTR wzText, _In_ LPCWSTR wzFont, _In_ int nFontSize, _In_ COLORREF clrBK, _In_ COLORREF clrFont, _In_ Alignment alignment=Center);

private:
    virtual bool Create(_In_opt_ HDC hDC, _In_ int cx, _In_ int cy, _In_ COLORREF clrBK);
};

    
}   // namespace nudf::image
}   // namespace nudf

#endif  // _NUDF_IMG_BITMAP_HPP__