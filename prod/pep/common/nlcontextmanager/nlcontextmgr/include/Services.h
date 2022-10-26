#pragma once

#pragma warning(push)
#pragma warning(disable:4512 4244 6011)
#include "boost/threadpool.hpp"
#pragma warning(pop)

class CServices
{
public: 
	static void StartServices();

private:
	static void NamedPipeService();//If necessary, we can add other service types, like SOCKET, FILE MAPPING...
	
	static void NamedPipeWorkThread(void* parm);

	static boost::threadpool::pool m_tpworking;
	static boost::threadpool::pool m_tpservice;
};
