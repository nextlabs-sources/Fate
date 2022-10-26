
#pragma once

#include <string>
#include <vector>
using namespace std;


#pragma warning(push)
#pragma warning(disable:6334 6011 4996 4189 4100 4819)
#include "boost\unordered_map.hpp"
#include "boost\shared_ptr.hpp"
#include "boost\weak_ptr.hpp"
#include "boost\format.hpp"
#pragma warning(pop)

#include "comm_helper.hpp"
#include "celog.h"

class CPolicy
{
private:
	CPolicy(void)
	{
		m_bInit=false;
	}
	virtual ~CPolicy(void)
	{
		nextlabs::comm_helper::Release_Cesdk(&m_context);
	}
	void init()
	{
		if (!m_bInit)
		{
			m_bInit=true;
			nextlabs::comm_helper::Init_Cesdk(&m_context);//you only need to initialize one time.
			m_ptr = nextlabs::comm_helper::CreateComm(nextlabs::type_cesdk, &m_context);
		}
	}

public:
	static CPolicy* GetInstance()
	{
		static CPolicy ins;
		ins.init();
		return &ins;
	}

	void query(const string& action, 
		const string& source, 
		bool& bdeny)
	{
		bdeny = false;

		//query PC
		nextlabs::eval_parms parm;

		wstring waction(action.begin(),action.end());
		wstring wsource(source.begin(),source.end());

		parm.SetAction(waction.c_str());
		parm.SetSrc(wsource.c_str(),L"fso");

		if(m_ptr->Query(&parm))
		{
			bdeny = m_ptr->IsDenied();
		}
	}
private:
	bool m_bInit;
	nextlabs::cesdk_context m_context;
	boost::shared_ptr<nextlabs::comm_base> m_ptr;
};
