#include "Utility.h"
#include <Psapi.h>

#pragma warning(push)
#pragma warning(disable: 4995) 
#include "celog.h"
#pragma warning(pop)

#pragma warning( push )
#pragma warning( disable : 6011 )
#  include <boost/algorithm/string.hpp>
#pragma warning(pop)

extern CELog  g_log;

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

	const WCHAR* ExplorerName = L"\\explorer.exe";
}

Utility  Utility::sm_UtilityInstance;

Utility::Utility() : m_InsertImage(NULL), m_OrgBmpWidth(0), m_OrgBmpHeight(0)
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

HBITMAP Utility::GenerateImage(const std::wstring& DisplayText, LONG Width, LONG Height) const
{
	HBITMAP BackgroundImage = NULL;

	if (NULL == m_InsertImage)
	{
		BackgroundImage = GenerateTextImage(DisplayText, Width, Height);
	}
	else
	{
		BackgroundImage = GenerateImage(Width, Height);
	}

	return BackgroundImage;
}

HBITMAP Utility::GenerateImage(LONG Width, LONG Height) const
{
	LONG InsertImageWidth = 0;
	LONG InsertImageHeight = 0;

	SetSuitableSize(Width, Height, InsertImageWidth, InsertImageHeight);
	
	HBITMAP hBitmapSrc = static_cast<HBITMAP>(CopyImage(m_InsertImage, IMAGE_BITMAP, InsertImageWidth, InsertImageHeight, 0));

	if (NULL == hBitmapSrc)
	{
		return GenerateTextImage( DefaultBackgroundStr, Width, Height);
	}

	HDC hdcSrc = CreateCompatibleDC(NULL); 
	SelectObject(hdcSrc, hBitmapSrc);

	//dest background bitmap
	HBITMAP hBitmapDst = CreateCompatibleBitmap(hdcSrc, Width, Height);

	if (hBitmapDst == NULL)
	{
		DeleteObject(hBitmapSrc);
		DeleteDC(hdcSrc);
		return GenerateTextImage( DefaultBackgroundStr, Width, Height);
	}

	HDC hdcDst  = CreateCompatibleDC(NULL);
	SelectObject(hdcDst, hBitmapDst);
	int orgx = (Width - InsertImageWidth)/2;
	int orgy = (Height - InsertImageHeight)/2;

	BitBlt(hdcDst, orgx, orgy, InsertImageWidth, InsertImageHeight, hdcSrc, 0, 0, SRCCOPY);

	DeleteObject(hBitmapSrc);
	DeleteDC(hdcDst);
	DeleteDC(hdcSrc);
	
	return hBitmapDst;
}

void Utility::SetSuitableSize(LONG Width, LONG Height, LONG& InsertImageWidth, LONG& InsertImageHeight)const
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

HBITMAP Utility::GenerateTextImage(const std::wstring& DisplayText, LONG Width, LONG Height) const
{
	HBITMAP hBitmapSrc = CreateBitmap(Width, Height, DefaultImagePlane, DefaultImageBitsPerPel, NULL);

	if (hBitmapSrc == NULL)
	{
		return NULL;
	}

	HDC hdcSrc = CreateCompatibleDC(NULL);

	SelectObject(hdcSrc, hBitmapSrc);

	SetDisplayText(hdcSrc, DisplayText, Width, Height);

	DeleteDC(hdcSrc);

	return hBitmapSrc;
}

void Utility::SetDisplayText(HDC& hdcDst, const std::wstring& DisplayText, _In_ LONG Width, _In_ LONG Height) const
{
	long lfHeight = -MulDiv(Height/TextScale, GetDeviceCaps(hdcDst, LOGPIXELSY), TextDenominator);

	HFONT hf = CreateFontW(lfHeight, 0, 0, 0, 0, TRUE, 0, 0, 0, 0, 0, 0, 0, NULL);

	if (NULL == hf)
	{
		g_log.Log(CELOG_DEBUG, L"Create font failed , last error : %d\n", GetLastError());

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

void Utility::GetImagePath(std::wstring& ImagePath) const
{
	WCHAR ModulePath[MAX_PATH] = { 0 };

	GetModuleFileNameW(NULL, ModulePath, MAX_PATH);

	ImagePath = ModulePath;

	std::wstring::size_type found = ImagePath.rfind(L'\\');

	if (found != std::wstring::npos)
	{
		ImagePath.replace(found, std::wstring::npos, DefaultImageName);
	}
}

std::string Utility::wstringTostring(const std::wstring& wstr) const
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

std::wstring Utility::stringTowstring(const std::string& str) const
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

bool Utility::IsExplorer(DWORD dwProcessID) const
{
	std::wstring ProcessName;

	if (GetProcessName(dwProcessID, ProcessName))
	{
		if (boost::algorithm::iends_with(ProcessName, ExplorerName))
		{
			return true;
		}
	}

	return false;
}

bool Utility::GetProcessName(DWORD dwProcessID, std::wstring& ProcessName) const
{
	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, false, dwProcessID);

	if (NULL == hProcess)
	{
		return false;
	}
	
	WCHAR wchProcessName[MAX_PATH] = { 0 };
	
	bool bReturn = false;

	if (0 != GetProcessImageFileName(hProcess, wchProcessName, MAX_PATH))
	{
		bReturn = true;

		ProcessName = wchProcessName;
	}

	CloseHandle(hProcess);

	return bReturn;
}