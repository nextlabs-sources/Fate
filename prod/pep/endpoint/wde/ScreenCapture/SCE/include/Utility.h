#pragma once
#include <Windows.h>
#include <string>
#include <boost/noncopyable.hpp>

class Utility : private boost::noncopyable
{
public:
	_Check_return_ static Utility& GetInstance()
	{
		return sm_UtilityInstance;
	}

private:
	Utility();

	~Utility()
	{
		if (NULL != m_InsertImage)
		{
			DeleteObject(m_InsertImage);
		}
	}

public:
	_Check_return_ HBITMAP GenerateImage(_In_ const std::wstring& DisplayText, _In_ LONG Width, _In_ LONG Height) const;

	_Check_return_ std::string wstringTostring(_In_ const std::wstring& wstr) const;

	_Check_return_ std::wstring stringTowstring(_In_ const std::string& str) const;
	
	_Check_return_ bool IsExplorer(_In_ DWORD dwProcessID) const;

private:
	_Check_return_ HBITMAP GenerateTextImage(_In_ const std::wstring& DisplayText, _In_ LONG Width, _In_ LONG Height) const;

	_Check_return_ HBITMAP GenerateImage(_In_ LONG Width, _In_ LONG Height) const;

	void SetSuitableSize(_In_ LONG Width, _In_ LONG Height, _Out_ LONG& InsertImageWidth, _Out_ LONG& InsertImageHeight)const ;

	void SetDisplayText(_Inout_ HDC& hdcDst, _In_ const std::wstring& DisplayText, _In_ LONG Width, _In_ LONG Height) const;

	void GetImagePath(_Out_ std::wstring& ImagePath) const;

	_Check_return_ bool GetProcessName(_In_ DWORD dwProcessID, _Out_ std::wstring& ProcessName) const;

private:
	static Utility sm_UtilityInstance;

private:
	HBITMAP m_InsertImage;
	int m_OrgBmpWidth;
	int m_OrgBmpHeight;
};