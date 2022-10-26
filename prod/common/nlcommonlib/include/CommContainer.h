#pragma once
#include "commonlib_helper.h"
#include <string>

class CCommContainer
{
public:
	CCommContainer(void);
	~CCommContainer(void);

	int CacheData(_In_ nextlabs::CMD_TYPE cmd, _In_ const nextlabs::cache_key* _key, _In_ const unsigned char* _value, _In_ unsigned int len);
	int GetData(_In_ const nextlabs::cache_key* _key, _In_ bool forcedelete, _Out_ nextlabs::cache_value* _value);
};
