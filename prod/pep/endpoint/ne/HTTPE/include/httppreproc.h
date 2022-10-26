#pragma once

#include <string>
#include "httpmsg.h"

class CHttpPreProc
{
public:
	CHttpPreProc(void);
	~CHttpPreProc();
	/*
	return value:

	TRUE, means it doesn't need the further handle.
	*/
	static BOOL PreProcessMsg(smartHttpMsg& httpMsg, BOOL& allow);
	static BOOL PreCheck(SOCKET s);
private:
	CHttpPreProc(const CHttpPreProc&);
	void operator = (const CHttpPreProc&);
};


	