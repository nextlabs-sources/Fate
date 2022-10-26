#ifndef _COMM_BASE_
#define _COMM_BASE_

#pragma warning(push)
#pragma warning(disable:4100 4800)
#include "eframework\platform\cesdk.hpp"
#pragma warning(pop)

#include "nextlabs_feature_manager.hpp"
#include "nextlabs_features.h"

namespace nextlabs
{
	typedef boost::unordered_multimap<std::wstring, std::wstring> ATTRS;

	enum comm_type{type_cesdk = 0, type_nlcc};

	enum comm_error{error_success = 0, error_load_failed, error_pc_notrunning};

	struct cesdk_context
	{
	public:
		cesdk_loader m_sdk;
		std::wstring m_strUserID;
		std::wstring m_strUserName;
		CEApplication* m_pApp;
		bool bEnableReverseDNS;

		cesdk_context()
		{
			m_pApp = NULL;
			bEnableReverseDNS = false;
		}
	};

	class parms 
	{
	protected:
		LPCWSTR action;
		LPCWSTR src;
		LPCWSTR src_type;
		const ATTRS* src_attrs;
		LPCWSTR target;
		LPCWSTR target_type;
		const ATTRS* target_attrs;
		size_t timeout;
		CENoiseLevel_t noise_level;
		LPCWSTR username;
		LPCWSTR userid;
		unsigned int ip; 
		bool performob;
		LPCWSTR apppath;//use this member for nlcc
		CEApplication* app;//use this member for cesdk
		const ATTRS* app_attrs;
		std::vector<std::wstring>* recipients;

		parms ()
		{
			action = NULL;
			src = NULL;
			src_type = L"fso";
			src_attrs = NULL;
			target = NULL;
			target_type = L"fso";
			target_attrs = NULL;
			timeout = 30000;
			noise_level = CE_NOISE_LEVEL_USER_ACTION;
			username = NULL;
			userid = NULL;
			ip = 0;
			performob = true;
			apppath = NULL;
			app = NULL;
			app_attrs = NULL;
			recipients = NULL;
		}
	public:
		_Ret_z_ inline virtual LPCWSTR GetAction(){ return action; }
		_Ret_z_ inline virtual LPCWSTR GetSrcPath(){ return src; }
		_Ret_z_ inline virtual LPCWSTR GetSrcType(){ return src_type; }
		_Ret_ inline virtual const ATTRS* GetSrcAttrs(){ return src_attrs; }
		_Ret_z_ inline virtual LPCWSTR GetTargetPath(){ return target; }
		_Ret_z_ inline virtual LPCWSTR GetTargetType(){ return target_type; }
		_Ret_ inline virtual const ATTRS* GetTargetAttrs(){ return target_attrs; }
		_Ret_ inline virtual size_t GetTimeout(){ return timeout; }
		_Ret_ inline virtual CENoiseLevel_t GetNoiseLevel() { return noise_level; }
		_Ret_z_ inline virtual LPCWSTR GetUserName(){ return username; }
		_Ret_z_ inline virtual LPCWSTR GetUserID(){ return userid; }
		_Ret_ inline virtual unsigned int GetIp(){ return ip; }
		_Ret_ inline virtual bool GetOBPerformFlag(){ return performob; }
		_Ret_z_ inline virtual LPCWSTR GetAppPath(){ return apppath;}
		_Ret_ inline virtual CEApplication* GetApp(){ return app; }
		_Ret_ inline virtual const ATTRS* GetAppAttrs(){return app_attrs;}
		_Ret_ inline virtual std::vector<std::wstring>* GetRecipients(){ return recipients; }
	};

	class eval_parms: public parms
	{
	public:
		inline void SetAction(_In_z_ LPCWSTR act){ action = act; }
		inline void SetSrc(_In_z_ LPCWSTR pSrc, _In_z_ LPCWSTR pType = L"fso", _In_opt_ const ATTRS* pAttrs = NULL)
		{ 
			src = pSrc;
			if(pType != NULL)
				src_type = pType;
			
			src_attrs = pAttrs;
		}
		inline void SetTarget(_In_z_ LPCWSTR pTarget, _In_z_ LPCWSTR pType = L"fso", _In_opt_ const ATTRS* pAttrs = NULL)
		{
			target = pTarget;
			if(pType != NULL)
				target_type = pType;
			target_attrs = pAttrs;
		}
		inline void SetTimeout(_In_ size_t t){ timeout = t;}
		inline void SetNoiseLevel(_In_ CENoiseLevel_t level){ noise_level = level; }
		inline void SetUserInfo(_In_z_ LPCWSTR pUserName, _In_z_ LPCWSTR pUserID)
		{
			username = pUserName;
			userid = pUserID;
		}
		inline void SetIp(_In_ unsigned int ip)
		{
			this->ip = ip;
		}
		inline void SetApplicationInfo(_In_z_ LPCWSTR pAppPath/*used by nlcc*/, _In_ CEApplication* pApp/*used by cesdk*/)
		{
			apppath = pAppPath;
			app = pApp;
		}
		inline void SetApplicationAttrs(_In_ const ATTRS* attrs)
		{
			app_attrs = attrs;
		}

		inline void SetObligationFlag(_In_ bool bOB)
		{
			performob = bOB;
		}

		inline void SetRecipients(_In_ std::vector<std::wstring>* pRecipients)
		{
			recipients = pRecipients;
		}
	};		


	class comm_base
	{
	public:
		comm_base()
		{
			feat.open();
		}
		virtual bool Connect(_Deref_out_ void** pCon, _In_ int nTimeOut = 5000) = 0;
		virtual bool Query(_In_ parms* param, _In_opt_ void* con = NULL, _In_opt_ void* reserved = NULL) = 0;
		virtual bool IsDenied() = 0;
		_Ret_ virtual nextlabs::Obligations& GetObligations() = 0;

		_Ret_ virtual int GetLastError(){return m_nErrorCode;}

		virtual bool ConfiguedInLicense(_In_ parms* pParms)
		{
			if (!pParms)
			{
				return false;
			}

			LPCWSTR action = pParms->GetAction();
			if (!action)
			{
				return false;
			}
			
			bool is_feature_enabled = false;
			if (_wcsicmp(action, L"OPEN") == 0)
			{
				is_feature_enabled = feat.is_enabled(NEXTLABS_FEATURE_OPEN);
			}
			else if (_wcsicmp(action, L"COPY") == 0)
			{
				is_feature_enabled = feat.is_enabled(NEXTLABS_FEATURE_COPY);
			}
            else if (_wcsicmp(action, L"MOVE") == 0)
            {
                is_feature_enabled = feat.is_enabled(NEXTLABS_FEATURE_COPY);  // Actually, we should add MOVE in license file, but for now, just use COPY.
            }
			else if (_wcsicmp(action, L"EDIT") == 0)
			{
				is_feature_enabled = feat.is_enabled(NEXTLABS_FEATURE_EDIT);
			}
			else if (_wcsicmp(action, L"PRINT") == 0)
			{
				is_feature_enabled = feat.is_enabled(NEXTLABS_FEATURE_PRINT);
			}
			else if (_wcsicmp(action, L"DELETE") == 0)
			{
				is_feature_enabled = feat.is_enabled(NEXTLABS_FEATURE_DELETE);
			}
			else if (_wcsicmp(action, L"PASTE") == 0)
			{
				is_feature_enabled = feat.is_enabled(NEXTLABS_FEATURE_COPY_CONTENT);
			}
			else if (_wcsicmp(action, L"RENAME") == 0)
			{
				is_feature_enabled = feat.is_enabled(NEXTLABS_FEATURE_RENAME);
			}
			else if (_wcsicmp(action, L"RUN") == 0)
			{
				is_feature_enabled = feat.is_enabled(NEXTLABS_FEATURE_RUN);
			}
			else if (_wcsicmp(action, L"CHANGE_ATTRIBUTES") == 0)
			{
				is_feature_enabled = feat.is_enabled(NEXTLABS_FEATURE_CHANGE_ATTRIBUTE);
			}
			else if (_wcsicmp(action, L"CHANGE_SECURITY") == 0)
			{
				is_feature_enabled = feat.is_enabled(NEXTLABS_FEATURE_CHANGE_SECURITY_ATTRIBUTE);
			}
			else if (_wcsicmp(action, L"EFS") == 0)
			{
				is_feature_enabled = feat.is_enabled(NEXTLABS_FEATURE_EFS);
			}
			else if (_wcsicmp(action, L"CONVERT") == 0)
			{
				is_feature_enabled = feat.is_enabled(NEXTLABS_FEATURE_CONVERT);
			}
			else if (_wcsicmp(action, L"SEND") == 0)
			{
				is_feature_enabled = feat.is_enabled(NEXTLABS_FEATURE_SEND);
			}
			else if (_wcsicmp(action, L"DECRYPT") == 0)
			{
				is_feature_enabled = feat.is_enabled(NEXTLABS_FEATURE_DECRYPT);
			}
			else if (_wcsicmp(action, L"SCREEN_CAPTURE") == 0)
			{
				is_feature_enabled = feat.is_enabled(NEXTLABS_FEATURE_SCREEN_CAPTURE);
			}

			return is_feature_enabled;
		}
	protected:
		nextlabs::feature_manager feat;
		int m_nErrorCode;
	};
}

#endif