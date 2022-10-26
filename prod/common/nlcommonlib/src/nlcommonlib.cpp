// nlcommonlib.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "commonlib_helper.h"
#include "CommContainer.h"

#pragma warning(push)
#pragma warning(disable:6334 6011)
#include "boost\unordered_map.hpp"
#include "boost\shared_ptr.hpp"
#include "boost\weak_ptr.hpp"
#pragma warning(pop)

using namespace nextlabs;

int nl_CacheData(_In_ const cache_key* _key, _In_ const unsigned char* _value, _In_ const unsigned int len)
{
	CCommContainer comm;
	return comm.CacheData(CMD_CACHE_DATA, _key, _value, len);
}

int nl_GetData(_In_ const cache_key* _key, _In_ bool forcedelete, _Out_ unsigned char** data, _Out_ unsigned int* len)
{
	CCommContainer comm;
	
	nextlabs::cache_value _value;
	int ret = comm.GetData(_key, forcedelete, &_value);
	if(ret == ERR_SUCCESS)
	{
		*len = (unsigned int)_value.GetLength();
		*data = new unsigned char[*len];
		memcpy(*data, _value.GetValue().get(), *len);
	}
	else
	{
		*len = 0;
		*data = NULL;
	}

	return ret;
}

void nl_FreeMem(_In_ unsigned char* pMem)
{
	if (pMem != NULL)
	{
		delete []pMem;
	}
}
