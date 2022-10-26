#pragma once
#include <Windows.h>
#include <string>
#include <boost/noncopyable.hpp>

namespace nextlabs
{

class ScreenCaptureAuxiliary : private boost::noncopyable
{
public:
	_Check_return_ static ScreenCaptureAuxiliary& GetInstance()
	{
		return sm_Instance;
	}

private:
	ScreenCaptureAuxiliary();

	~ScreenCaptureAuxiliary()
	{
		if (NULL != m_InsertImage)
		{
			DeleteObject(m_InsertImage);	
		}
	}

public:
	_Check_return_ HDC GenerateHDC(_In_ const std::wstring& DisplayText, _In_ LONG Width, _In_ LONG Height, _In_opt_ HDC hdc = NULL) const;

	void ReplaceHDC(_In_ const std::wstring& DisplayText, _In_ HDC hdc) const;

	_Check_return_ std::string wstringTosting(_In_ const std::wstring& wstr) const;

	_Check_return_ std::wstring stringTowsting(_In_ const std::string& str) const;

private:
	_Check_return_ HDC GenerateTextHDC(_In_ const std::wstring& DisplayText, _In_ LONG Width, _In_ LONG Height, _In_opt_ HDC hdc = NULL) const;

	_Check_return_ HDC GenerateHDC(_In_ LONG Width, _In_ LONG Height, _In_opt_ HDC hdc = NULL) const;

	void SetSuitableSize(_In_ LONG Width, _In_ LONG Height, _Out_ LONG& InsertImageWidth, _Out_ LONG& InsertImageHeight)const ;

	void SetDisplayText(_Inout_ HDC& hdcDst, _In_ const std::wstring& DisplayText, _In_ LONG Width, _In_ LONG Height) const;

	void GetImagePath(_Out_ std::wstring& ImagePath) const;

private:
	static ScreenCaptureAuxiliary sm_Instance;

private:
	HBITMAP m_InsertImage;
	int m_OrgBmpWidth;
	int m_OrgBmpHeight;
};

}