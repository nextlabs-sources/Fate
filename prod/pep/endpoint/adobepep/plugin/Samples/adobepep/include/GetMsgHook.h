#pragma once

#include <windows.h>

namespace AdobeXI
{

	class CGetMsgHook
	{
	public:
		CGetMsgHook(void);
		~CGetMsgHook(void);

	public:
		bool InstallGetMsgHook();
		HHOOK GetHookHandle();
		void  UnHookHandle();

	protected:
		static LRESULT CALLBACK GetMsgProcMy(_In_  int code,_In_  WPARAM wParam, _In_  LPARAM lParam );


	protected:
		HHOOK m_hGetMsgHook;
	};

	extern CGetMsgHook theGetMsgHook;
}

