#include "StdAfx.h"
#include "PrtSrnKey.h"
#include "Utility.h"
#include "SCEServer.h"

#pragma warning(push)
#pragma warning(disable: 4995) 
#include "celog.h"
#pragma warning(pop)

extern CELog  g_log;

namespace SCE
{

namespace
{
	const char* DefaultBackgroundStr = "NextLabs";

	const int SleepTimeOnOpenCLipboardFail = 20;
}

void CPrtSrnKey::Handle()
{
	std::string strBackground;

	try
	{
		if (SCEServer::GetInstance()->Query(0, strBackground))
		{
			g_log.Log(CELOG_DEBUG, L"No need to deny PrtSrn key\n");
			
			if (!CaptureScreen())
			{
				g_log.Log(CELOG_DEBUG, L"Failed to capture screen\n");
			}

			return;
		}
	}
	catch (...)
	{
		g_log.Log(CELOG_DEBUG, L"exception in CPrtSrnKey::Handle\n");			
	}
	
	if (strBackground.empty())
	{
		strBackground = DefaultBackgroundStr;
	}

	std::wstring wstrBackground = Utility::GetInstance().stringTowstring(strBackground);
	
	if (!ReplaceScreen(wstrBackground))
	{
		g_log.Log(CELOG_DEBUG, L"Failed to replace screen\n");
	}

	return;
}	

bool CPrtSrnKey::CaptureScreen() const
{
	HDC hDesktopDC = GetWindowDC(NULL); 
	if (NULL == hDesktopDC)
	{
		return false;
	}

	RECT rect = {0};
	GetScreenRegion(rect);

	HBITMAP hCaptureBitmap = CreateCompatibleBitmap(hDesktopDC, rect.right - rect.left, rect.bottom - rect.top);
	if (NULL == hCaptureBitmap)
	{
		ReleaseDC(NULL, hDesktopDC);

		return false;
	}

	HDC hCaptureDC = CreateCompatibleDC(hDesktopDC);
	if (NULL == hCaptureDC)
	{
		DeleteObject(hCaptureBitmap);
		ReleaseDC(NULL, hDesktopDC);

		return false;
	}

	SelectObject(hCaptureDC, hCaptureBitmap);
	
	if (BitBlt(hCaptureDC, 0, 0, rect.right - rect.left, rect.bottom - rect.top, hDesktopDC, rect.left, rect.top, SRCCOPY | CAPTUREBLT))
	{
		SetClipboardBitmap(hCaptureBitmap);
	}

	DeleteDC(hCaptureDC);
	DeleteObject(hCaptureBitmap);
	ReleaseDC(NULL, hDesktopDC);

	return true;
}

void CPrtSrnKey::GetScreenRegion(RECT& rect) const
{
	LONG cxScreen = GetSystemMetrics(SM_CXSCREEN);
	LONG cyScreen = GetSystemMetrics(SM_CYSCREEN);

	if (!m_bFullScreen)
	{
		HWND hDesk = GetForegroundWindow();

		if (NULL != hDesk)
		{
			GetWindowRect(hDesk, &rect);
			
			if (rect.left < 0)
			{
				rect.left = 0;
			}
			
			if (rect.top < 0)
			{
				rect.top = 0;
			}

			if (rect.right > cxScreen || 0 == rect.right)
			{
				rect.right = cxScreen;
			}

			if (rect.bottom > cyScreen || 0 == rect.bottom)
			{
				rect.bottom = cyScreen;
			}

			return;
		}
	}
	
	rect.right = cxScreen;
	rect.bottom = cyScreen;
}

void CPrtSrnKey::SetClipboardBitmap(HBITMAP hBitmap) const
{
	do 
	{
		if(OpenClipboard(NULL))
		{	
			EmptyClipboard();

			if (NULL == SetClipboardData(CF_BITMAP, hBitmap))
			{
				g_log.Log(CELOG_DEBUG, L"Set clipboard Data failed, error : %d\n", GetLastError());
			}

			CloseClipboard();

			return;
		}
		
		Sleep(SleepTimeOnOpenCLipboardFail);
	} while (TRUE);
}

bool CPrtSrnKey::ReplaceScreen(const std::wstring& strBackground) const
{
	RECT rect = {0};
	GetScreenRegion(rect);

	HBITMAP hBitmap = Utility::GetInstance().GenerateImage(strBackground, rect.right - rect.left, rect.bottom - rect.top);
	if (hBitmap == NULL)
	{
		return false;
	}

	SetClipboardBitmap(hBitmap);

	DeleteObject(hBitmap);

	return true;
}

}