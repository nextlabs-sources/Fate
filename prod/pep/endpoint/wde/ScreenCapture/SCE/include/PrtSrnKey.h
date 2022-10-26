#pragma once
#include <Windows.h>
#include <string>
#include <boost/noncopyable.hpp>

namespace SCE
{

class CPrtSrnKey :public boost::noncopyable
{
public:
	CPrtSrnKey(_In_ bool bFullScreen = TRUE) : m_Width(0), m_Height(0), m_bFullScreen(bFullScreen)
	{
	}

public:
	void Handle();

private:
	_Check_return_ bool ReplaceScreen(_In_ const std::wstring& strBackground) const;

	_Check_return_ bool CaptureScreen() const;
	
	void GetScreenRegion(_Out_ RECT& rect) const;

	void SetClipboardBitmap(_In_ HBITMAP hBitmap) const;

private:
	LONG m_Width;
	LONG m_Height;

	const bool m_bFullScreen;
};

}