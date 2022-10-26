#pragma once

#include "commonlib_helper.h"

#pragma warning(push)
#pragma warning(disable:4512 4244 6011 4996 4100 6334 6326)
#include "boost/unordered_map.hpp"
#pragma warning(pop)

using namespace nextlabs;
class CCacheMgr
{
public:
	static CCacheMgr* CreateInst();
	void CacheData(CPackage* pack);
	cache_value GetData(CPackage* pack, bool forcedelete);
private:
	CCacheMgr(void);
	~CCacheMgr(void);
private:
	boost::unordered_map<cache_key, cache_value> m_mapCache;
};
