#pragma once

#include <string>

using namespace std;


/*

maintain password of enterprise dlp, used by decrypt bundle

*/

class CPWDMgr
{
private:
	CPWDMgr(void)
	{

	}

	~CPWDMgr(void)
	{

	}

	//	don't define these two functions
	CPWDMgr(const CPWDMgr &);
	void operator = (const CPWDMgr &);

public:

	static CPWDMgr& GetInstance()
	{
		static CPWDMgr mgr;

		return mgr;
	}

	BOOL setpwd(const wstring& pwd)
	{
		m_pwd = pwd;
		return TRUE;
	}


	BOOL getpwd(wstring & pwd)
	{
		pwd = m_pwd;
		return TRUE;
	}


private:

	wstring m_pwd;
};
