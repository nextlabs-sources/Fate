#ifndef __OBEMAIL_H__
#define __OBEMAIL_H__


#include "obligation.h"
#pragma once

#include <string>



class COBEmail
{
public:
	COBEmail(void);
public:
	~COBEmail(void);
public:
	bool SendEmail();
	bool SendEmail(BaseArgumentFlex & baseArguFlex);
private:
	std::wstring m_strReceiver;
	std::wstring m_strCCAddress;
	std::wstring m_strSubject;
	std::wstring m_strBody;
};

#endif

