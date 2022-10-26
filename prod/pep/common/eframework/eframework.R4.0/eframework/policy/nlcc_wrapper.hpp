#ifndef _NLCC_WRAPPER_
#define _NLCC_WRAPPER_
#include "comm_base.hpp"

#pragma warning(push)
#pragma warning(disable:6031 4800)
#include "eframework\policy\policy.hpp"
#pragma warning(pop)


namespace nextlabs
{
	class nlcc_wrapper: public comm_base
	{
	public:
		nlcc_wrapper()
		{
			m_nErrorCode = 0;
		}

        _Check_return_
		bool Connect(_Deref_out_ void** pCon, _In_ int nTimeOut = 5000)
		{
			nTimeOut;
			policy_connection* pNlccCon = new policy_connection();
			bool b = pNlccCon->connect();
			
			if (b)
			{
				*pCon = pNlccCon;
			}
			else
			{
				delete pNlccCon;
				pNlccCon = NULL;
			}
			return b;
		}

		bool Query(_In_ parms* param, _In_opt_ void* con, _In_opt_ void* reserved = NULL)
		{
			reserved;

			if (!param)
			{
				return false;
			}

			if( !ConfiguedInLicense(param) || nextlabs::policy_controller::is_up() == false )
			{
				m_bIsDenied = false;
				return true;
			}


			m_bIsDenied = false;

			//get the current process path
			static wchar_t szAppPath[1025] = {0};
			if (wcslen(szAppPath) == 0)
			{
				GetModuleFileNameW(NULL, szAppPath, 1024);
			}
			

			policy_connection* pCon = NULL;
			if (con == NULL)
			{
				//we need to create a connection
				if (!Connect((void**)&pCon))
				{
					return false;
				}
			}
			
			nextlabs::policy_query pquery;


#pragma warning(push)
#pragma warning(disable:6031)
			pquery.set_action(param->GetAction());
			pquery.set_source(param->GetSrcPath(), param->GetSrcType());
			pquery.set_target(param->GetTargetPath(), param->GetTargetType());
			if (param->GetAppPath() != NULL)
				pquery.set_application(param->GetAppPath());
			else
				pquery.set_application(szAppPath);
			
#pragma warning(pop)

			bool result = true;
			if(pCon != NULL)
			{
				result = pCon->query(pquery);

				if(!result)
					m_nErrorCode = GetLastError();

				pCon->disconnect();
				delete pCon;
				pCon = NULL;

			}
			else
			{
				if(con != NULL)
				{
					result = ((policy_connection*)con)->query(pquery);
					if(!result)
						m_nErrorCode = GetLastError();
				}
			}
			
			m_bIsDenied = !pquery.is_allow();
		
			return result;
			
		}

		_Ret_ bool IsDenied()
		{
			return m_bIsDenied;
		}

		_Ret_ nextlabs::Obligations& GetObligations()
		{
			return m_obs;
		}


	private:
		Obligations m_obs;
		bool m_bIsDenied;
	};
}

#endif
