#ifndef _CESDK_WRAPPER_
#define _CESDK_WRAPPER_

#pragma warning(push)
#pragma warning(disable:6334 6011)
#include "boost\unordered_map.hpp"
#include "boost\shared_ptr.hpp"
#include "boost\weak_ptr.hpp"
#pragma warning(pop)

#if defined(_WIN32) || defined (_WIN64)
  #include <winsock2.h> // timeval
#pragma warning(push)
#pragma warning(disable:6386)
  #include <Ws2tcpip.h>	
#pragma warning(pop)
  #include <windows.h>
#else
  #include <pthread.h>
  #include "linux_win.h"	
#endif


#include "nlconfig.hpp"
#include <assert.h>
#include <string>
#include <vector>
#include <Sddl.h>
#include "comm_base.hpp"


namespace nextlabs
{
	namespace
	{

		void GetUserInfo(_Out_z_cap_(nSize) LPWSTR wzSid, _In_ int nSize, _Inout_z_cap_(UserNameLen) LPWSTR UserName, _In_ int UserNameLen)
		{
			HANDLE hTokenHandle = NULL;

			if(!OpenThreadToken(GetCurrentThread(), TOKEN_QUERY, TRUE, &hTokenHandle))
			{
				if(GetLastError() == ERROR_NO_TOKEN)
				{
					if(!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hTokenHandle ))
					{
						goto _exit;
					}
				}
				else
				{
					goto _exit;
				}
			}

			// Get SID
			UCHAR   InfoBuffer[512];
			DWORD   cbInfoBuffer = 512;
			LPTSTR  StringSid = NULL;
			WCHAR   uname[64] = {0}; DWORD unamelen = 63;
			WCHAR   dname[64] = {0}; DWORD dnamelen = 63;
			WCHAR   fqdnname[MAX_PATH+1]; memset(fqdnname, 0, sizeof(fqdnname));
			SID_NAME_USE snu;

			if(!GetTokenInformation(hTokenHandle, TokenUser, InfoBuffer, cbInfoBuffer, &cbInfoBuffer))
				goto _exit;
			if(ConvertSidToStringSid(((PTOKEN_USER)InfoBuffer)->User.Sid, &StringSid))
			{
				wcsncpy_s(wzSid, nSize, StringSid, _TRUNCATE);
				if(StringSid) LocalFree(StringSid);
			}
			if(LookupAccountSid(NULL, ((PTOKEN_USER)InfoBuffer)->User.Sid, uname, &unamelen, dname, &dnamelen, &snu))   
			{
				WCHAR wzHostname[MAX_PATH] = { 0 };
				DWORD dwSize = MAX_PATH;

				GetComputerNameExW(ComputerNameDnsFullyQualified, wzHostname, &dwSize);

				wcsncpy_s(fqdnname, MAX_PATH, wzHostname, NI_MAXHOST);
				wcsncat_s(UserName, UserNameLen, fqdnname, _TRUNCATE);
				wcsncat_s(UserName, UserNameLen, L"\\", _TRUNCATE); 
				wcsncat_s(UserName, UserNameLen, uname, _TRUNCATE);
			}

	_exit:
			if(NULL!=hTokenHandle) CloseHandle(hTokenHandle); hTokenHandle=NULL;
		}
	}



	class cesdk_wrapper: public comm_base
	{
		friend class comm_helper;
	private:
	    _Check_return_
		static bool Init(_Out_ cesdk_context* context)
		{
			if(!context)
				return false;

			if (context->m_sdk.is_loaded())
			{
				return true;
			}

			context->m_pApp = new CEApplication;
			context->m_pApp->appName = NULL;
			context->m_pApp->appPath = NULL;
			context->m_pApp->appURL = NULL;

			wchar_t szDir[MAX_PATH] = {0};
			if(NLConfig::ReadKey(L"SOFTWARE\\NextLabs\\CommonLibraries\\InstallDir", szDir, MAX_PATH))
			{
#ifdef _WIN64
				wcsncat_s(szDir, MAX_PATH, L"\\bin64\\", _TRUNCATE);
#else
				wcsncat_s(szDir, MAX_PATH, L"\\bin32\\", _TRUNCATE);
#endif
				if(context->m_sdk.load(szDir))
				{
					//Get current logon user info
					wchar_t szID[128] = {0};
					wchar_t szName[128] = {0};
					GetUserInfo(szID, 128, szName, 128);
					context->m_strUserName.assign(szName);
					context->m_strUserID.assign(szID);

					//Get current process info
					wchar_t szAppPath[1024] = {0};
					wchar_t szAppName[1024] = {0};
					GetModuleFileNameW(NULL, szAppPath, 1024);
					if (wcslen(szAppPath) > 0)
					{
						wchar_t* p = wcsrchr(szAppPath, '\\');
						if(p != NULL)
							wcsncpy_s(szAppName, 1024, p + 1, _TRUNCATE);
					}
					context->m_pApp->appName = context->m_sdk.fns.CEM_AllocateString(szAppName);
					context->m_pApp->appPath = context->m_sdk.fns.CEM_AllocateString(szAppPath);

					wchar_t szConfigFile[MAX_PATH] = {0};
					if(NLConfig::ReadKey(L"SOFTWARE\\NextLabs\\Compliant Enterprise\\Policy Controller\\PolicyControllerDir", szConfigFile, MAX_PATH))
					{
						wcsncat_s(szConfigFile, MAX_PATH, L"\\service\\injection.ini", _TRUNCATE);
						context->bEnableReverseDNS = GetPrivateProfileIntW(L"info", L"EnableReverseDNS", 0, szConfigFile);
					}
						
					return true;
				}

			}

			
			return false;
		}

		static void Release(_In_ cesdk_context* context)
		{
			if (!context)
			{
				return;
			}

			if (context->m_pApp != NULL)
			{
				if (context->m_pApp->appName != NULL)
				{
					context->m_sdk.fns.CEM_FreeString(context->m_pApp->appName);
				}

				if (context->m_pApp->appPath != NULL)
				{
					context->m_sdk.fns.CEM_FreeString(context->m_pApp->appPath);
				}

				if (context->m_pApp->appURL != NULL)
				{
					context->m_sdk.fns.CEM_FreeString(context->m_pApp->appURL);
				}

				delete context->m_pApp;
				context->m_pApp = NULL;
			}

			context->m_sdk.unload();
			
		}

	public:
		cesdk_wrapper(_In_ cesdk_context* context)
		{
			m_conext = context;	
			m_nErrorCode = 0;
		}

		~cesdk_wrapper()
		{
			
		}

        _Check_return_
		bool Connect(_Deref_out_ void** pCon, _In_ int nTimeOut = 5000)
		{
			if(!pCon)
			{
				return false;
			}
				

			cesdk_connection* pCesdk_Con = new cesdk_connection();
			pCesdk_Con->set_sdk((cesdk_loader*)&m_conext->m_sdk);
			pCesdk_Con->set_timeout(nTimeOut);
			bool ret = pCesdk_Con->connect();
			m_nErrorCode = pCesdk_Con->get_last_error();

			if (ret)
			{
				*pCon = pCesdk_Con;
			}
			else
			{
				delete pCesdk_Con;
				pCesdk_Con = NULL;
			}
			
			return ret;
		}

		bool Query(_In_ parms* param, _In_opt_ void* con = NULL, _In_opt_ void* reserved = NULL)
		{
			reserved;

			//reset the members.
			m_bIsDenied = false;
			m_obs = Obligations();

			if (param == NULL)
			{
				return false;
			}
			
			

			if( !ConfiguedInLicense(param) || nextlabs::policy_controller::is_up() == false )
			{
				m_bIsDenied = false;
				return true;
			}

			cesdk_connection* pCon = NULL;
			if (con == NULL)
			{
				/*
				We need to create a new connection
				*/
				bool b = Connect((void**)&pCon); 
				if(!b)
				{
					return false;
				}
			}

			cesdk_query query(m_conext->m_sdk);

#pragma warning(push)
#pragma warning(disable:6031)
			query.set_action(param->GetAction());
			query.set_noise_level(param->GetNoiseLevel());
			query.set_timeout(param->GetTimeout());
			query.set_obligations(param->GetOBPerformFlag());
			query.set_ip(param->GetIp());

			if(param->GetUserName() != NULL)
				query.set_user_name(param->GetUserName());
			else
				query.set_user_name(m_conext->m_strUserName.c_str());
			if (param->GetUserID() != NULL)
				query.set_user_id(param->GetUserID());
			else
				query.set_user_id(m_conext->m_strUserID.c_str());

			if (param->GetApp() != NULL)
				query.set_app(param->GetApp());
			else
				query.set_app(m_conext->m_pApp);



			//set source attributes
			CEAttributes src_attrs;
			src_attrs.count = 0;
			src_attrs.attrs = NULL;
			if(param->GetSrcAttrs() != NULL)
			{
				src_attrs.count = static_cast<CEint32>(param->GetSrcAttrs()->size());

				if (!m_conext->bEnableReverseDNS)
				{
					src_attrs.count += 1;
				}

				if(src_attrs.count > 0)
				{	
					src_attrs.attrs = new CEAttribute[src_attrs.count];
					int index = 0;
					for (ATTRS::const_iterator itr = param->GetSrcAttrs()->begin(); itr != param->GetSrcAttrs()->end(); itr++)
					{
						src_attrs.attrs[index].key = m_conext->m_sdk.fns.CEM_AllocateString(((*itr).first).c_str());
						src_attrs.attrs[index].value = m_conext->m_sdk.fns.CEM_AllocateString(((*itr).second).c_str());
						index++;
					}

					if (!m_conext->bEnableReverseDNS)
					{
						src_attrs.attrs[index].key = m_conext->m_sdk.fns.CEM_AllocateString(L"ce::get_equivalent_host_names");
						src_attrs.attrs[index].value = m_conext->m_sdk.fns.CEM_AllocateString(L"no");
					}
				}
			}
			else
			{
				if (!m_conext->bEnableReverseDNS)
				{
					src_attrs.count = 1;
					src_attrs.attrs = new CEAttribute[1];
					src_attrs.attrs[0].key = m_conext->m_sdk.fns.CEM_AllocateString(L"ce::get_equivalent_host_names");
					src_attrs.attrs[0].value = m_conext->m_sdk.fns.CEM_AllocateString(L"no");
				}
			}
			query.set_source(param->GetSrcPath(), param->GetSrcType(), &src_attrs);


			//set target attributs
			CEAttributes target_attrs;
			target_attrs.count = 0;
			target_attrs.attrs = NULL;
			if (param->GetTargetAttrs() != NULL)
			{
				target_attrs.count = static_cast<CEint32>(param->GetTargetAttrs()->size());
				if (target_attrs.count > 0)
				{
					target_attrs.attrs = new CEAttribute[target_attrs.count];

					int index = 0;
					for (ATTRS::const_iterator itr = param->GetTargetAttrs()->begin(); itr != param->GetTargetAttrs()->end(); itr++)
					{
						target_attrs.attrs[index].key = m_conext->m_sdk.fns.CEM_AllocateString(((*itr).first).c_str());
						target_attrs.attrs[index].value = m_conext->m_sdk.fns.CEM_AllocateString(((*itr).second).c_str());
						index++;
					}
				}

			}
			query.set_target(param->GetTargetPath(), param->GetTargetType(), &target_attrs);

			//application attributes
			CEAttributes app_attrs;
			app_attrs.count = 0;
			app_attrs.attrs = NULL;
			if (param->GetAppAttrs() != NULL)
			{
				const ATTRS* vAttrs = param->GetAppAttrs();
				app_attrs.count = static_cast<CEint32>(vAttrs->size());
				if (app_attrs.count > 0)
				{
					app_attrs.attrs = new CEAttribute[app_attrs.count];

					int index = 0;
					for (ATTRS::const_iterator itr = vAttrs->begin(); itr != vAttrs->end(); itr++)
					{
						app_attrs.attrs[index].key = m_conext->m_sdk.fns.CEM_AllocateString(((*itr).first).c_str());
						app_attrs.attrs[index].value = m_conext->m_sdk.fns.CEM_AllocateString(((*itr).second).c_str());
						index++;
					}
				}
			}
			query.set_appattrs(&app_attrs);
#pragma warning(pop)

			std::vector<std::wstring>* pRecipients = param->GetRecipients();
			CEString* recipients = NULL;
			CEint32 num = 0;
			if (pRecipients != NULL)
			{
				num = static_cast<CEint32>(pRecipients->size());
				if (num > 0)
				{
    				recipients = new CEString[num];
    				if (recipients)
    				{
    					for (CEint32 i = 0; i < num; i++)
    					{
    						recipients[i] = m_conext->m_sdk.fns.CEM_AllocateString(pRecipients->at(i).c_str());
    					}
    				}
				}
			}
			query.set_recipients(recipients, num);


			//do query
			bool ret = true;
			if (pCon)
				ret = query.query(pCon->get_connection_handle());
			else
				ret = query.query(((cesdk_connection*)con)->get_connection_handle());

			if (recipients != NULL)
			{
				for (int i = 0; i < num; i++)
				{
					m_conext->m_sdk.fns.CEM_FreeString(recipients[i]);
				}
				delete []recipients;
				recipients = NULL;
			}

			m_bIsDenied = query.is_deny();
			m_obs = query.get_obligations();
			m_nErrorCode = query.get_last_error();

			//release memory
			if (src_attrs.count > 0 && src_attrs.attrs != NULL)
			{
				for (int i = 0; i < src_attrs.count; i++)
				{
					m_conext->m_sdk.fns.CEM_FreeString(src_attrs.attrs[i].key);
					m_conext->m_sdk.fns.CEM_FreeString(src_attrs.attrs[i].value);
				}
				delete [] src_attrs.attrs;
			}

			if (target_attrs.count > 0 && target_attrs.attrs != NULL)
			{
				for (int i = 0; i < target_attrs.count; i++)
				{
					m_conext->m_sdk.fns.CEM_FreeString(target_attrs.attrs[i].key);
					m_conext->m_sdk.fns.CEM_FreeString(target_attrs.attrs[i].value);
				}
				delete [] target_attrs.attrs;
			}

			if (app_attrs.count > 0 && app_attrs.attrs != NULL)
			{
				for (int i = 0; i < app_attrs.count; i++)
				{
					m_conext->m_sdk.fns.CEM_FreeString(app_attrs.attrs[i].key);
					m_conext->m_sdk.fns.CEM_FreeString(app_attrs.attrs[i].value);
				}
				delete [] app_attrs.attrs;
			}

			if (pCon)
			{
				if(pCon->is_connected())
					pCon->disconnect();

				delete pCon;
				pCon = NULL;
			}
			return ret;
		}
        
		_Ret_ nextlabs::Obligations& GetObligations()
		{
			return m_obs;
		}

		_Ret_ bool IsDenied()
		{
			return m_bIsDenied;
		}


		
	private:
		const cesdk_context* m_conext;

		Obligations m_obs;
		bool m_bIsDenied;
		
	};

	

}


#endif