#include "StdAfx.h"
#include "CommContainer.h"
#include "NamedPipeAdapter.h"


CCommContainer::CCommContainer(void)
{
}

CCommContainer::~CCommContainer(void)
{
}

int CCommContainer::CacheData(nextlabs::CMD_TYPE cmd, const nextlabs::cache_key *_key, const unsigned char *_value, unsigned int len)
{
	if (!_key || !_value)
	{
		return ERR_EMPTYPOINTER;
	}

	std::string ret = nextlabs::CPackage::Package(cmd, _key, _value, len);
	
	CNamedPipeAdapter comm;
	return  comm.SendData((const unsigned char*)ret.c_str(), static_cast<int> (ret.length()));

}

int CCommContainer::GetData(_In_ const nextlabs::cache_key* _key, _In_ bool forcedelete, _Out_ nextlabs::cache_value* _value)
{
	if(!_key)
		return ERR_EMPTYPOINTER;

	CNamedPipeAdapter comm;
	return comm.GetData(_key, forcedelete, _value);
}
