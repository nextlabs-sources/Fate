#pragma once
#include "stdafx.h"
#include <vector>
#include <string>
#include <map>
#include "celog.h"
using namespace std;

class CEvalCache
{
private:
	CEvalCache(void);
	~CEvalCache(void);

public:
	static CEvalCache* GetInstance();

	//	we only cache "allow" enforcement
	void commit(const wstring& eval_key);
	
	//	if return false, means there is no cache for eval_key, otherwise return true
	bool query(const wstring& eval_key);

private:
	//	wstring: "action"+"sender"+"all recipient"+"attachment file name"
	//	DWORD: time clock to add this item, it will expire in 5 seconds
	map<wstring, DWORD> m_eval_cache;	

	//	threads safe
	CRITICAL_SECTION m_cs;
};
