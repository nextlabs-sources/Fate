#pragma once

#include "CommunicationBase.h"

class CNamedPipeAdapter: public CCommunicationBase
{
public:
	CNamedPipeAdapter(void);
	~CNamedPipeAdapter(void);

	virtual int SendData(const unsigned char* data, int len);
	virtual int GetData(const nextlabs::cache_key* _key, bool forcedelete, _Out_ nextlabs::cache_value* _value);

private:
	int Connect(HANDLE& h);
	int Send(HANDLE hPipe, const unsigned char *data, int len);
};
