#include "ScreenCaptureAuxiliary.h"

namespace
{
	const WCHAR* DefaultImageName = L"\\SCEBackGround.bmp";
	const wchar_t* DefaultBackgroundStr = L"NextLabs";

	const int DefaultImageWidth = 0x100;
	const int DefaultImageHeight = DefaultImageWidth;
	const int DefaultImagePlane = 0x01;
	const int DefaultImageBitsPerPel = 0x20;

	const int TextScale = 10;
	const int TextDenominator = 60;
}

namespace nextlabs
{

ScreenCaptureAuxiliary  ScreenCaptureAuxiliary::sm_Instance;

ScreenCaptureAuxiliary::ScreenCaptureAuxiliary() : m_InsertImage(NULL), m_OrgBmpWidth(0), m_OrgBmpHeight(0)
{
	std::wstring ImagePath;
	
	GetImagePath(ImagePath);

	m_InsertImage = static_cast<HBITMAP>(LoadImageW(NULL, ImagePath.c_str(), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE));

	if (m_InsertImage != NULL)
	{
		BITMAP bmp = {0};

		GetObject(m_InsertImage, sizeof(BITMAP), &bmp);

		m_OrgBmpWidth = bmp.bmWidth;

		m_OrgBmpHeight = bmp.bmHeight;
	}
}

HDC ScreenCaptureAuxiliary::GenerateHDC(const std::wstring& DisplayText, LONG Width, LONG Height, HDC hdc) const
{
	HDC hdcDest = NULL;
	if (m_InsertImage == NULL)
	{
		hdcDest = GenerateTextHDC(DisplayText, Width, Height, hdc);
	}
	else
	{
		hdcDest = GenerateHDC(Width, Height, hdc);
	}

	return hdcDest;
}

HDC ScreenCaptureAuxiliary::GenerateTextHDC(const std::wstring& DisplayText, LONG Width, LONG Height, HDC hdc) const
{
	HBITMAP hBitmap = CreateBitmap(Width, Height, DefaultImagePlane, DefaultImageBitsPerPel, NULL);

	if (hBitmap == NULL)
	{
		return NULL;
	}

	HDC hdcDest = CreateCompatibleDC(hdc);

	SelectObject(hdcDest, hBitmap);

	SetDisplayText(hdcDest, DisplayText, Width, Height);

	DeleteObject(hBitmap);

	return hdcDest;
}

HDC ScreenCaptureAuxiliary::GenerateHDC(LONG Width, LONG Height, HDC hdc) const
{
	LONG InsertImageWidth = 0;
	LONG InsertImageHeight = 0;

	SetSuitableSize(Width, Height, InsertImageWidth, InsertImageHeight);

	HBITMAP hBitmapSrc = static_cast<HBITMAP>(CopyImage(m_InsertImage, IMAGE_BITMAP, InsertImageWidth, InsertImageHeight, 0));

	if (NULL == hBitmapSrc)
	{
		return GenerateTextHDC( DefaultBackgroundStr, Width, Height, hdc);
	}

	HDC hdcSrc = CreateCompatibleDC(hdc); 
	SelectObject(hdcSrc, hBitmapSrc);

	//dest background bitmap
	HBITMAP hBitmapDst = CreateCompatibleBitmap(hdcSrc, Width, Height);

	if (hBitmapDst == NULL)
	{
		DeleteObject(hBitmapSrc);
		DeleteDC(hdcSrc);
		return GenerateTextHDC(DefaultBackgroundStr, Width, Height, hdc);
	}

	HDC hdcDst  = CreateCompatibleDC(hdc);
	SelectObject(hdcDst, hBitmapDst);
	int orgx = (Width - InsertImageWidth)/2;
	int orgy = (Height - InsertImageHeight)/2;

	BitBlt(hdcDst, orgx, orgy, InsertImageWidth, InsertImageHeight, hdcSrc, 0, 0, SRCCOPY);

	DeleteObject(hBitmapSrc);
	DeleteObject(hBitmapDst);
	DeleteDC(hdcSrc);

	return hdcDst;
}

void ScreenCaptureAuxiliary::SetSuitableSize(LONG Width, LONG Height, LONG& InsertImageWidth, LONG& InsertImageHeight)const
{
	if (Width >= m_OrgBmpWidth)
	{
		if (Height >= m_OrgBmpHeight)
		{
			InsertImageHeight = m_OrgBmpHeight;
			InsertImageWidth = m_OrgBmpWidth;
		}
		else
		{
			InsertImageHeight = Height;
			InsertImageWidth = m_OrgBmpWidth*Height/m_OrgBmpHeight;
		}

	}
	else 
	{
		if (Height >= m_OrgBmpHeight*Width/m_OrgBmpWidth)
		{
			InsertImageHeight = m_OrgBmpHeight*Width/m_OrgBmpWidth;
			InsertImageWidth = Width;
		}
		else
		{
			InsertImageHeight = Height;
			InsertImageWidth = m_OrgBmpWidth*Height/m_OrgBmpHeight;
		}
	}
}

void ScreenCaptureAuxiliary::ReplaceHDC(const std::wstring& DisplayText, HDC hdc) const
{
	HBITMAP hBitmapSrc = static_cast<HBITMAP>(GetCurrentObject(hdc, OBJ_BITMAP));

	if (NULL == hBitmapSrc)
	{
		return;
	}

	BITMAP bmp = {0};

	GetObject(hBitmapSrc, sizeof(BITMAP), &bmp);

	HDC hdcSrc = GenerateHDC(DisplayText, bmp.bmWidth, bmp.bmHeight, hdc);
	
	if (NULL != hdcSrc)
	{
		BitBlt(hdc, 0, 0, bmp.bmWidth, bmp.bmHeight, hdcSrc, 0, 0, SRCCOPY);

		DeleteDC(hdcSrc);
	}
}

void ScreenCaptureAuxiliary::SetDisplayText(HDC& hdcDst, const std::wstring& DisplayText, _In_ LONG Width, _In_ LONG Height) const
{
	long lfHeight = -MulDiv(Height/TextScale, GetDeviceCaps(hdcDst, LOGPIXELSY), TextDenominator);

	HFONT hf = CreateFontW(lfHeight, 0, 0, 0, 0, TRUE, 0, 0, 0, 0, 0, 0, 0, NULL);

	if (NULL == hf)
	{
		return;
	}

	SelectObject(hdcDst, hf);

	SIZE TextSize = {0};

	GetTextExtentPoint32W(hdcDst, DisplayText.c_str(), static_cast<int>(DisplayText.size()), &TextSize);

	if (TextSize.cx < Width)
	{
		SetTextAlign(hdcDst, TA_CENTER);
		TextOutW(hdcDst, Width/2, (Height-TextSize.cy)/2, DisplayText.c_str(), static_cast<int>(DisplayText.size()));
	}
	else
	{
		SetTextAlign(hdcDst, TA_LEFT);
		TextOutW(hdcDst, 0, (Height-TextSize.cy)/2, DisplayText.c_str(), static_cast<int>(DisplayText.size()));
	}
 
	DeleteObject(hf);
}

void ScreenCaptureAuxiliary::GetImagePath(std::wstring& ImagePath) const
{
	HMODULE hModule = NULL;

	if (GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, reinterpret_cast<LPCWSTR>(GetInstance), &hModule))
	{
		WCHAR ModulePath[MAX_PATH] = { 0 };

		GetModuleFileNameW(hModule, ModulePath, MAX_PATH);

		ImagePath = ModulePath;

		std::wstring::size_type found = ImagePath.rfind(L'\\');

		if (found != std::wstring::npos)
		{
			ImagePath.replace(found, std::wstring::npos, DefaultImageName);
		}
	}
}

std::string ScreenCaptureAuxiliary::wstringTosting(const std::wstring& wstr) const
{
	int Size = WideCharToMultiByte( CP_ACP, 0, wstr.c_str(), -1, NULL, 0, NULL, NULL );

	if ( 0 == Size )
	{
		return std::string();
	}

	char* str = new char[Size]();

	WideCharToMultiByte( CP_ACP, 0, wstr.c_str(), -1, str, Size, NULL, NULL );

	std::string retstr = str;
	
	delete []str;

	return retstr;
}

std::wstring ScreenCaptureAuxiliary::stringTowsting(const std::string& str) const
{
	int Size = MultiByteToWideChar( CP_ACP, 0, str.c_str(), -1, NULL, 0 );

	if ( 0 == Size )
	{
		return std::wstring();
	}

	WCHAR* wstr = new WCHAR[Size]();

	MultiByteToWideChar( CP_ACP, 0, str.c_str ( ), -1, wstr, Size );

	std::wstring retwstr = wstr;

	delete []wstr;

	return retwstr;
}

}