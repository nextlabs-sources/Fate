#pragma once
#include "commonlib_helper.h"

class CCommunicationBase
{
public:
	virtual int SendData(const unsigned char* data, int len) = 0;
	virtual int GetData(const nextlabs::cache_key* _key, bool forcedelete, _Out_ nextlabs::cache_value* _value) = 0;
};
