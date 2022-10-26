#ifndef _CESDK_HELPER_
#define _CESDK_HELPER_

/*************************************************************************************************************
comm_helper supports 2 ways to communicate with PC: cesdk and nlcc.
usage sample:
1. using cesdk

	nextlabs::cesdk_context context;
	nextlabs::comm_helper::Init_Cesdk(&context);//you only need to initialize one time.

	//you can create the comm_base object anywhere, it's thread safe. make sure you called nextlabs::comm_helper::Init_Cesdk before.
	boost::shared_ptr<nextlabs::comm_base> ptr = nextlabs::comm_helper::CreateComm(nextlabs::type_cesdk, &context);

	nextlabs::eval_parms parm;

	parm.SetAction(L"COPY");
	parm.SetSrc(L"c:\\test\\a.txt");
	parm.SetTarget(L"d:\\a.txt");

	if(ptr->Query(&parm))
	{
		bool b = ptr->IsDenied();

		wprintf_s(L"eval result, denied: %d\n", b);

		nextlabs::Obligations& obs = ptr->GetObligations();

		std::list<nextlabs::Obligation> listOb = obs.GetObligations();
		for (std::list<nextlabs::Obligation>::const_iterator itr = listOb.begin(); itr != listOb.end(); itr++)
		{

			wprintf_s(L"obligation name: %s, policy name: %s\n", (*itr).name.c_str(), (*itr).policy.c_str());

			for(nextlabs::ObligationOptions::const_iterator itOp = (*itr).options.begin(); itOp != (*itr).options.end(); itOp++)
			{
			wprintf_s(L"obligation content name: %s, value: %s\n", (*itOp).first.c_str(), (*itOp).second.c_str());
			}
		}
	}
	else
	{
		wprintf_s(L"query failed: %d\n", ptr->GetLastError());
	}

	nextlabs::comm_helper::Release_Cesdk(&context);

	
	eval for email

	nextlabs::eval_parms parm2;
	parm2.SetAction(L"EMAIL");
	parm2.SetSrc(L"c:\\kaka\\a.txt", L"fso");

	std::vector<std::wstring> vRecipients;
	vRecipients.push_back(std::wstring(L"john.tyler@qapf1.qalab01.nextlabs.com"));
	parm2.SetRecipients(&vRecipients);

	ptr->Query(&parm2);
	...

2. using nlcc

	boost::shared_ptr<nextlabs::comm_base> ptr = nextlabs::comm_helper::CreateComm(nextlabs::type_nlcc, NULL);

	nextlabs::eval_parms parm;
	parm.SetAction( L"MOVE");
	parm.SetSrc(L"c:\\kaka\\a.txt", L"fso");
	parm.SetTarget(L"d:\\a.txt", L"fso");

	if(ptr->Query(&parm, NULL))
	{
		wprintf_s(L"nlcc query is denied: %d\n", ptr->IsDenied());
	}
	else
		wprintf_s(L"nlcc query failed");

*************************************************************************************************************************************/


#pragma warning(push)
#pragma warning(disable:6334 6011 4996)
#include "boost\unordered_map.hpp"
#include "boost\shared_ptr.hpp"
#include "boost\weak_ptr.hpp"
#pragma warning(pop)

#include "nlconfig.hpp"
#include <assert.h>
#include <string>
#include <vector>
#include <Sddl.h>
#include "comm_base.hpp"
#include "cesdk_wrapper.hpp"
#include "nlcc_wrapper.hpp"


namespace nextlabs
{
	class comm_helper
	{
	public:
		static bool Init_Cesdk(_Out_ cesdk_context* context)
		{
			return cesdk_wrapper::Init(context);
		}

		static void Release_Cesdk(_In_ cesdk_context* context)
		{
			cesdk_wrapper::Release(context);
		}

		_Ret_ static boost::shared_ptr<comm_base> CreateComm(_In_ comm_type type, /* condition _In_*/ cesdk_context* context)
		{
			boost::shared_ptr<comm_base> ptr;
			switch(type)
			{
			case type_cesdk:
				ptr = boost::shared_ptr<comm_base>(new cesdk_wrapper(context));
				break;
			case type_nlcc:
				ptr = boost::shared_ptr<comm_base>(new nlcc_wrapper());
				break;
			}

			return ptr;
		}
		
	
	};
}


#endif