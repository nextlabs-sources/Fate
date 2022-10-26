#pragma once

/********************Cache data sample**********************************

	HMODULE h = LoadLibraryW(L"nlcommonlib.dll");
	nl_CacheData fnCacheData = (nl_CacheData)GetProcAddress(h, "nl_CacheData");
	nl_GetData fnGetData = (nl_GetData)GetProcAddress(h, "nl_GetData");
	nl_FreeMem fnFreeMem = (nl_FreeMem)GetProcAddress(h, "nl_FreeMem");


	cache_key _key;
	_key.set_value(L"basepep", (const unsigned char*)"path", 4);

	int err = fnCacheData(&_key, (const unsigned char*)"c:\\test\\a.txt", 13);
	if(ERR_SUCCESS != err)
		printf("cache failed, err: %d\n", err);


	unsigned char* buf = NULL;
	unsigned int len = 0;
	if(ERR_SUCCESS != fnGetData(&_key, false, &buf, &len))
		printf("get data failed\n");
	else
	{//get the cached data here
		char* temp = new char[len + 1];
		memcpy(temp,buf, len);
		temp[len] = '\0';
		printf("result: %s\n", temp);
		delete []temp;
	}

	if(buf != NULL)
		fnFreeMem(buf);

********************************************************************/

#include <stdio.h>
#include <windows.h>

#include <string>
#include <sstream>

#pragma warning(push)
#pragma warning(disable:4512 4244 6011 4996 4100 6334 6326)
#include <boost/functional/hash.hpp>
#include <boost/shared_ptr.hpp>
#pragma warning(pop)

//errors
#define ERR_SUCCESS 0
#define ERR_UNKNOWN -1
#define ERR_CONNECT_NLSTORAGE_FAILED -2
#define ERR_TIMEOUT -3
#define ERR_EMPTYPOINTER -4
#define ERR_RECEIVE_FAILED -5


//length
#define SIZE_STREAMLEN		4		//use 4 bytes to store the stream length of the content from pipe.
#define SIZE_TYPE			1
#define SIZE_KEYLEN			4
#define SIZE_VALUELEN		4


//pipe buffer and name
#define BUFSIZE 4
#define NLSTORAGE_NAMEDPIPE L"\\\\.\\pipe\\nlstoragepipe"



namespace nextlabs
{
	class cache_key
	{
	private:
		std::wstring m_strHost;//store the host name
		boost::shared_ptr<unsigned char> m_pKey;//store the key byte stream
		unsigned int m_nKeyLen;//the length of key stream.

	public:
		cache_key()
		{
			m_nKeyLen = 0;
		}

		cache_key(const cache_key& other)
		{
			if(this == &other)
				return;

			m_strHost = other.m_strHost;
			m_pKey = other.m_pKey;
			m_nKeyLen = other.m_nKeyLen;
		}

		void set_value(_In_ const wchar_t* pszHost, _In_ const unsigned char* pszKey, _In_ unsigned int nKeyLen)
		{
			if (pszHost == NULL || pszKey == NULL)
			{
				return;
			}

			m_strHost = std::wstring(pszHost);
			m_pKey.reset(new unsigned char[nKeyLen]);
			memcpy_s(m_pKey.get(), nKeyLen, pszKey, nKeyLen);

			m_nKeyLen = nKeyLen;
		}

		void get_bytestream(_Out_ boost::shared_ptr<unsigned char>& ret, _Out_ unsigned int& len) const
		{
			unsigned int host = (unsigned)m_strHost.length() * sizeof(wchar_t);
			unsigned int nLen = sizeof(unsigned int) + host + m_nKeyLen;
			boost::shared_ptr<unsigned char> ptr(new unsigned char[nLen]);

			memcpy_s(ptr.get(), nLen, &host, sizeof(unsigned int));

			memcpy_s(ptr.get() + sizeof(unsigned int), nLen, m_strHost.c_str(), host);
			if(m_nKeyLen > 0)
				memcpy(ptr.get() + sizeof(unsigned int) + host , m_pKey.get(), m_nKeyLen);
			
			ret = ptr;
			len = nLen;
		}

		cache_key& operator= (const cache_key& other) 
		{
			if (this == &other)
			{
				return *this;
			}

			m_strHost = other.m_strHost;
			m_pKey = other.m_pKey;
			m_nKeyLen = other.m_nKeyLen;

			return *this;
		}

		bool operator ==(cache_key const& other) const
		{
			bool ret = (m_strHost == other.m_strHost && m_nKeyLen == other.m_nKeyLen && memcmp(m_pKey.get(), other.m_pKey.get(), m_nKeyLen) == 0);
			return ret;
		}

		

		friend std::size_t hash_value(cache_key const& p)
		{
			std::size_t seed = 0;
			boost::hash_combine(seed, p.m_strHost);
			boost::hash_combine(seed, p.m_pKey);
			boost::hash_combine(seed, p.m_nKeyLen);

			return seed;
		}

		static boost::shared_ptr<cache_key> parse_key(_In_bytecount_(len) const unsigned char* data, _In_ unsigned int len)
		{
			unsigned int nHostLen = 0;
			memcpy_s(&nHostLen, sizeof(unsigned int), data, len<sizeof(unsigned int )?len:sizeof(unsigned int));
			boost::shared_ptr<cache_key> _key(new cache_key);
			wchar_t* host = new wchar_t[nHostLen / sizeof(wchar_t) + 1];
			memcpy_s(host, nHostLen, data + sizeof(unsigned int), nHostLen);
			host[nHostLen / sizeof(wchar_t)] = '\0';
 
			_key->set_value(host, data + sizeof(unsigned int) +  nHostLen, len - sizeof(unsigned int) - nHostLen);

			delete []host;

			return _key;
		}
	};

	class cache_value
	{
	private:
		boost::shared_ptr<unsigned char> m_pValue;
		unsigned int m_nValueLen;
	public:
		cache_value()
		{
			m_nValueLen = 0;
		}

		cache_value(const cache_value& other)
		{
			if(this == &other)
				return;

			m_pValue = other.m_pValue;
			m_nValueLen = other.m_nValueLen;
		}

		void SetValue(_In_ unsigned char* _value, _In_ unsigned int len)
		{
			m_pValue.reset(_value);
			m_nValueLen = len;
		}

		boost::shared_ptr<unsigned char> GetValue() const
		{
			return m_pValue;
		}

		unsigned int GetLength() const
		{
			return m_nValueLen;
		}

		cache_value& operator = (const cache_value& _value)
		{
			if(this == &_value)
				return *this;

			m_pValue = _value.m_pValue;
			m_nValueLen = _value.m_nValueLen;
			return *this;
		}

	};


	enum CMD_TYPE{CMD_CACHE_DATA = 0x01, CMD_GET_DATA = 0x02, CMD_RESPONSE = 0x03};

	class CPackage
	{
	private:
		int m_command;
		cache_key m_key;
		cache_value m_value;
	public:
		CPackage()
		{

		};

		inline void SetCommand(int cmd){ m_command = cmd; }
		inline int GetCommand(){ return m_command; }

		inline void SetKey(cache_key& _key){ m_key = _key; }
		inline cache_key& GetKey(){ return m_key; }

		inline void SetValue(cache_value& _value){ m_value = _value; }
		inline cache_value& GetValue(){ return m_value; }

		
		//package the data
		static std::string Package(_In_ CMD_TYPE cmd, _In_ const cache_key* _key, _In_ const unsigned char* _value, _In_ unsigned int len)
		{
			std::string sKey = "";
			if (_key != NULL)
			{
				boost::shared_ptr<unsigned char> ptr;
				unsigned int rlen = 0;
				_key->get_bytestream(ptr, rlen);
				if (rlen > 0)
				{
					sKey.assign((const char*)ptr.get(), rlen);
				}
			}

			std::string ret = "";//the byte stream
			
			char c = (char)cmd;//command
			ret.append(1, c);

			//append key length
			char buf[SIZE_KEYLEN] = {0};
			unsigned int nKeyLen = (unsigned int)sKey.length();
			memcpy_s(buf, sizeof(unsigned int), &nKeyLen, sizeof(unsigned int));
			ret.append(buf, sizeof(unsigned int));

			//append value length
			memcpy_s(buf, sizeof(unsigned int), &len, sizeof(unsigned int));
			ret.append(buf, sizeof(unsigned int));

			//append key byte stream
			ret.append(sKey.c_str(), sKey.length());

			//append value byte stream
			if(_value != NULL)
				ret.append((char*)_value, len);

			return ret;
			
		};

		//parse the data
		bool Parse(const unsigned char* data, unsigned int len)
		{
			if (!data || len < SIZE_KEYLEN + SIZE_VALUELEN + SIZE_TYPE)
			{
				return false;
			}
			
			std::string sData((char*)data, len);
			const char* ptr = sData.c_str();

			//get command
			m_command = (int)sData[0];

			
			//get key length
			unsigned int nKeyLen = 0;
			memcpy_s(&nKeyLen, sizeof(unsigned int), ptr + SIZE_TYPE,  sizeof(unsigned int));

			//get value length
			unsigned int nValueLen = 0;
			memcpy_s(&nValueLen, sizeof(unsigned int), ptr + SIZE_TYPE + SIZE_KEYLEN, sizeof(unsigned int));

			//get key
			if(nKeyLen > 0)
			{
				boost::shared_ptr<cache_key> pKey = cache_key::parse_key((const unsigned char*)ptr + SIZE_TYPE + SIZE_KEYLEN + SIZE_VALUELEN, nKeyLen);
				if(pKey.use_count() > 0)
				{
					m_key = *pKey.get();
				}
			}
			
			//get value
			if(nValueLen > 0)
			{
				unsigned char* pValue = new unsigned char[nValueLen];
				memcpy_s(pValue, nValueLen, ptr + SIZE_TYPE + SIZE_KEYLEN + SIZE_VALUELEN + nKeyLen, nValueLen);
				m_value.SetValue(pValue, nValueLen);
			}

			return true;

		};

	
	
	};

	//exported functions
	typedef int (*nl_CacheData)(_In_ const cache_key* _key, _In_ const unsigned char* _value, _In_ const unsigned int len);
	typedef int (*nl_GetData)(_In_ const cache_key* _key, _In_ bool forcedelete, _Out_ unsigned char** data, _Out_ unsigned int* len);
	typedef void (*nl_FreeMem)(_In_ unsigned char* pMem);
}
