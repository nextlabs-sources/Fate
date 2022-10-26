// TestBoost.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <Windows.h>
#include <string>
#include <process.h>
#include "../include/commonlib_helper.h"

#pragma warning(push)
#pragma warning(disable:4512 4244 6011 4996 4100 6334 6326)
#include "boost/unordered_map.hpp"
#pragma warning(pop)


using namespace nextlabs;



void TestNlStorage(void*)
{
	HMODULE h = LoadLibraryW(L"nlcommonlib.dll");
	nl_CacheData fnCacheData = (nl_CacheData)GetProcAddress(h, "nl_CacheData");
	nl_GetData fnGetData = (nl_GetData)GetProcAddress(h, "nl_GetData");
	nl_FreeMem fnFreeMem = (nl_FreeMem)GetProcAddress(h, "nl_FreeMem");

	int total = 0;
	for (int i = 0; i < 200; i++)
	{
		cache_key _key;
		_key.set_value(L"kevin", (const unsigned char*)&i, 4);

		int err = fnCacheData(&_key, (const unsigned char*)&i, 4);
		if(ERR_SUCCESS != err)
			printf("cache failed, index: %d, err: %d\n", i, err);

		total += i;
	}

	int totalrecv = 0;
	for (int j = 0; j < 200; j++)
	{
		cache_key _key;
		_key.set_value(L"kevin", (const unsigned char*)&j, 4);

		unsigned char* buf = NULL;
		unsigned int len = 0;
		if(ERR_SUCCESS != fnGetData(&_key, false, &buf, &len))
			printf("get data failed\n");


		int m =0;
		memcpy(&m, buf, len);
		if(m != j)
			printf("error, index: %d, get value: %d len %d\n", j, m, len);


		if(buf != NULL)
			fnFreeMem(buf);

		totalrecv += m;
	}

	printf("result: total saved %d, total recv: %d\n", total, totalrecv);
}

void TestNlStorage2(void*)
{
	HMODULE h = LoadLibraryW(L"nlcommonlib.dll");
	nl_CacheData fnCacheData = (nl_CacheData)GetProcAddress(h, "nl_CacheData");
	nl_GetData fnGetData = (nl_GetData)GetProcAddress(h, "nl_GetData");
	nl_FreeMem fnFreeMem = (nl_FreeMem)GetProcAddress(h, "nl_FreeMem");


	cache_key _key;
	_key.set_value(L"kevin", (const unsigned char*)"111", 3);
	for (int i = 0; i < 1000; i++)
	{


		fnCacheData(&_key, (const unsigned char*)"12345", 5);
		unsigned char* buf = NULL;
		unsigned int len = 0;
		if(ERR_SUCCESS != fnGetData(&_key, true, &buf, &len))
			printf("get data failed\n");

		if(memcmp(buf, "12345", 5) != 0)
			printf("failed\n");
		else
			printf("success\n");
		if(buf != NULL)
			fnFreeMem(buf);

		Sleep(10);
	}
	printf("test end\n");

}

void TestNlStorage3(void*)
{
	HMODULE h = LoadLibraryW(L"nlcommonlib.dll");
	nl_CacheData fnCacheData = (nl_CacheData)GetProcAddress(h, "nl_CacheData");
	nl_GetData fnGetData = (nl_GetData)GetProcAddress(h, "nl_GetData");
	nl_FreeMem fnFreeMem = (nl_FreeMem)GetProcAddress(h, "nl_FreeMem");


	cache_key _key;
	_key.set_value(L"kevin", (const unsigned char*)"111", 3);
	for (int i = 0; i < 1000; i++)
	{
		fnCacheData(&_key, (const unsigned char*)"12345", 5);
		unsigned char* buf = NULL;
		unsigned int len = 0;
		if(ERR_SUCCESS != fnCacheData(&_key, (const unsigned char*)"1111", 4))
			printf("send data failed\n");

		printf("cache\n");

		Sleep(10);
	}
	printf("test end\n");

}

int _tmain(int argc, _TCHAR* argv[])
{
	std::wstring a = L"a";
	std::wstring b = L"a";
	bool r = a == b;
	//	TestNlStorage2(NULL);

	for (int i = 0; i < 10; i++)
	{
		_beginthread(TestNlStorage, 0, NULL);
	}



	Sleep(100000);

	return 0;
}

