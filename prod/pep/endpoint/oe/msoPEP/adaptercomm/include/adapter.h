#ifndef __ADAPTER_ADAPTERCOMMON_H__
#define __ADAPTER_ADAPTERCOMMON_H__
#include <atlcomcli.h>

namespace AdapterCommon
{
	namespace{
	class Adapter
	{
	public:
		static WCHAR ADAPTER_INI_SECTION_ADAPTER[];
		static WCHAR ADAPTER_INI_KEY_IMAGE[];
		static WCHAR ADAPTER_INI_KEY_OBLIGATIONNAME[];
		static WCHAR ADAPTER_INI_KEY_NAME[];
		static WCHAR ADAPTER_INI_KEY_ENFORCER[];

		static WCHAR KEYNAME_COMPLIANT_ENTERPRISE[];
		static WCHAR KEYNAME_WOW32COMPLIANT_ENTERPRISE[];
		static WCHAR KEYNAME_REPOSITORY_ADAPTER[];
		static WCHAR KEYVALUE_IMAGE[];
		static WCHAR KEYVALUE_OBLIGATIONNAME[];
	public:
		typedef HRESULT (WINAPI *FPRepositoryUpload)(CComPtr<IDispatch> pItem,Attachments* pAtts);
		typedef HRESULT (WINAPI *FPRepositoryUploadEx)(IDispatch*pItem,AdapterCommon::Attachments* pAtts,wchar_t** pwchObligationTopMessage,int &iTopMsgLength,wchar_t** pwchObligationBottomMessage,int &iBottomMsgLength);
		typedef HRESULT (WINAPI *FPRepositoryReleasePWCH)(wchar_t* pwch,bool bIsArray);
	public:
		Adapter(const WCHAR* wzImage,const WCHAR* wzObligationName):wstrImage(wzImage),wstrObligationName(wzObligationName),hModule(NULL),fpUpload(NULL){};
		Adapter(const WCHAR*wzIniFile,const WCHAR* wzImage,const WCHAR*wzDefaultObligationName)
		{
			WCHAR wzEnforcer[MAX_PATH+1];memset(wzEnforcer,0,sizeof(wzEnforcer));
			WCHAR wzAdapterName[MAX_PATH+1];memset(wzAdapterName,0,sizeof(wzAdapterName));
			WCHAR wzObligationName[MAX_PATH+1];memset(wzObligationName,0,sizeof(wzObligationName));
			
			DWORD dwRet=GetPrivateProfileString(ADAPTER_INI_SECTION_ADAPTER,ADAPTER_INI_KEY_OBLIGATIONNAME,wzDefaultObligationName,wzObligationName,MAX_PATH,wzIniFile);
			if(dwRet==0)
				return;

			dwRet=GetPrivateProfileString(ADAPTER_INI_SECTION_ADAPTER,ADAPTER_INI_KEY_NAME,NULL,wzAdapterName,MAX_PATH,wzIniFile);
			if(dwRet==0)
				return;
			
			dwRet=GetPrivateProfileString(ADAPTER_INI_SECTION_ADAPTER,ADAPTER_INI_KEY_ENFORCER,NULL,wzEnforcer,MAX_PATH,wzIniFile);
			if(dwRet==0)
				return;

			wstrAdapterName=wzAdapterName;
			wstrImage=wzImage;
			wstrObligationName=wzObligationName;
			wstrEnforcer=wzEnforcer;
			hModule=NULL;
			fpUpload=NULL;
		}
		~Adapter(){if(hModule)FreeLibrary(hModule); hModule=NULL;};
		void SetName(const WCHAR* wzAdapterName){wstrAdapterName=wzAdapterName;}
		std::wstring GetName(){return wstrAdapterName;};
		std::wstring GetObligationName(){return wstrObligationName;};
		BOOL Register()
		{
			if(wstrAdapterName.length()==0||wstrImage.length()==0||wstrObligationName.length()==0||wstrEnforcer.length()==0)
				return FALSE;
			HRESULT hr=RegisterAdapter(wstrEnforcer.c_str(),wstrAdapterName.c_str(),wstrImage.c_str(),wstrObligationName.c_str());
			if(SUCCEEDED(hr))
				return TRUE;
			return FALSE;

		};
		BOOL UnRegister()
		{
			if(wstrAdapterName.length()==0||wstrEnforcer.length()==0)
				return FALSE;
			HRESULT hr=UnRegisterAdapter(wstrEnforcer.c_str(),wstrAdapterName.c_str());
			if(SUCCEEDED(hr))
				return TRUE;
			return FALSE;

		}
		BOOL Upload(CComPtr<IDispatch> pItem,Attachments* pAtts)
		{
			if(hModule ==NULL)
			{
				if(wstrImage.length()==0)
				{
					return FALSE;
				}

				hModule=LoadLibrary(wstrImage.c_str());
			}
			if(hModule == NULL)
			{
				return FALSE;
			}
			if(fpUpload==NULL)
				fpUpload=(FPRepositoryUpload)GetProcAddress(hModule,"RepositoryUpload");
			if(fpUpload==NULL)
			{
				return FALSE;
			}
			HRESULT hr=fpUpload(pItem,pAtts);
			if(hr==S_OK)
				return TRUE;

			return FALSE;

		}
		BOOL UploadEx(CComPtr<IDispatch> pItem,Attachments* pAtts,wchar_t** pwchTopMessage,int &iTopMsgLen,wchar_t** pwchBottomMessage,int &iBottomMsgLength)
		{
			if(hModule ==NULL)
			{
				if(wstrImage.length()==0)
				{
					return FALSE;
				}

				hModule=LoadLibrary(wstrImage.c_str());
			}
			if(hModule == NULL)
			{
				return FALSE;
			}
			fpUploadEx=(FPRepositoryUploadEx)GetProcAddress(hModule,"RepositoryUploadEx");
			
			if(fpUploadEx==NULL)
			{
				return FALSE;
			}
			
			HRESULT hr=fpUploadEx(pItem,pAtts,pwchTopMessage,iTopMsgLen,pwchBottomMessage,iBottomMsgLength);			
			if(hr==S_OK)
			{
				return TRUE;
			}
			return FALSE;
				
		};

		BOOL RleaseUploadObligationPWCH(wchar_t* pwch,bool bIsArray)
		{
			fpRelease=(FPRepositoryReleasePWCH)GetProcAddress(hModule,"ReleaseRepositoryUploadExPWCH");
			if(fpRelease==NULL)
			{
				return FALSE;
			}
			return fpRelease(pwch,bIsArray);
		}
	private:
		HRESULT RegisterAdapter(LPCWSTR wzEnforcerName,LPCWSTR wzAdapterName,LPCWSTR wzImage,LPCWSTR wzObligationName)
		{
			DWORD   dwDisposition = 0;
			HKEY    hKeyEnforcer   = NULL;
			HKEY    hKeyReposAdapters     = NULL;
			HKEY	hKeyAdapter = NULL;
			LONG    lResult       = 0;
			//DWORD   dwVal         = 0;
			LONG   lRet = S_OK;
			WCHAR   wzKeyName[MAX_PATH];

			__try
			{
				_snwprintf_s(wzKeyName, MAX_PATH, _TRUNCATE, L"%s\\%s",KEYNAME_COMPLIANT_ENTERPRISE, wzEnforcerName);
				lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, wzKeyName, 0, KEY_ALL_ACCESS, &hKeyEnforcer);
				if(ERROR_SUCCESS != lResult)
				{
					_snwprintf_s(wzKeyName, MAX_PATH, _TRUNCATE, L"%s\\%s",KEYNAME_WOW32COMPLIANT_ENTERPRISE, wzEnforcerName);
					lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, wzKeyName, 0, KEY_ALL_ACCESS, &hKeyEnforcer);
				}
				if(ERROR_SUCCESS != lResult)     // get office/outlook key
				{
					lRet = E_UNEXPECTED;
					__leave;
			//		return E_UNEXPECTED;
				}
				_snwprintf_s(wzKeyName,MAX_PATH, _TRUNCATE, L"%s\\%s\\%s",KEYNAME_COMPLIANT_ENTERPRISE, wzEnforcerName,KEYNAME_REPOSITORY_ADAPTER);
				lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, wzKeyName,0, KEY_ALL_ACCESS,&hKeyReposAdapters);
				if(ERROR_SUCCESS != lResult)
				{
					_snwprintf_s(wzKeyName,MAX_PATH, _TRUNCATE, L"%s\\%s\\%s",KEYNAME_WOW32COMPLIANT_ENTERPRISE, wzEnforcerName,KEYNAME_REPOSITORY_ADAPTER);
					lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, wzKeyName,0, KEY_ALL_ACCESS,&hKeyReposAdapters);
				}
				if(ERROR_SUCCESS != lResult)
				{
					
					lResult = RegCreateKeyEx(hKeyEnforcer,KEYNAME_REPOSITORY_ADAPTER,0,NULL,REG_OPTION_NON_VOLATILE,KEY_WRITE,NULL,&hKeyReposAdapters,&dwDisposition);
					if(ERROR_SUCCESS != lResult)
					{
						lRet = E_UNEXPECTED;
						__leave;
						//return E_UNEXPECTED;
					}
				}

				lResult = RegCreateKeyEx( hKeyReposAdapters, wzAdapterName,0,NULL,REG_OPTION_NON_VOLATILE,KEY_WRITE,NULL,&hKeyAdapter,&dwDisposition);
				if(ERROR_SUCCESS != lResult)
				{
					lRet = E_UNEXPECTED;
					__leave;
					//return E_UNEXPECTED;
				}
				//dwVal = 0;
				//RegSetValueEx(hKeyAdapter, L"CommandLineSafe", 0, REG_SZ, (const BYTE*)&dwVal, sizeof(DWORD));
				//dwVal = 3;
				//RegSetValueEx(hKeyAddin, L"LoadBehavior", 0, REG_DWORD, (const BYTE*)&dwVal, sizeof(DWORD));
				//sprintf_s(szVal, MAX_PATH, "Enterprise DLP Office PEP"/*"Compliant Enterprise Office PEP"*/);
				RegSetValueExW(hKeyAdapter, KEYVALUE_OBLIGATIONNAME, 0, REG_SZ, (const BYTE*)wzObligationName, 1+(DWORD)wcslen(wzObligationName)*sizeof(WCHAR));
				RegSetValueExW(hKeyAdapter, KEYVALUE_IMAGE, 0, REG_SZ, (const BYTE*)wzImage, 1+(DWORD)wcslen(wzImage)*sizeof(WCHAR));
			}
			__finally
			{
				if(NULL != hKeyEnforcer) RegCloseKey(hKeyEnforcer);
				if(NULL != hKeyReposAdapters) RegCloseKey(hKeyReposAdapters);
				if(NULL != hKeyAdapter) RegCloseKey(hKeyAdapter);


				hKeyEnforcer   = NULL;
				hKeyReposAdapters = NULL;
				hKeyAdapter = NULL;
			}

			return lRet;

		}
		HRESULT UnRegisterAdapter(LPCWSTR wzEnforcerName,LPCWSTR wzAdapterName)
		{
			WCHAR   wzKeyName[MAX_PATH];
			_snwprintf_s(wzKeyName,MAX_PATH, _TRUNCATE, L"%s\\%s\\%s\\%s",KEYNAME_COMPLIANT_ENTERPRISE, wzEnforcerName,KEYNAME_REPOSITORY_ADAPTER,wzAdapterName);
			long lResult=RegDeleteKey(HKEY_LOCAL_MACHINE, wzKeyName);
			if(lResult != ERROR_SUCCESS)
			{
				_snwprintf_s(wzKeyName,MAX_PATH, _TRUNCATE, L"%s\\%s\\%s\\%s",KEYNAME_WOW32COMPLIANT_ENTERPRISE, wzEnforcerName,KEYNAME_REPOSITORY_ADAPTER,wzAdapterName);
				lResult=RegDeleteKey(HKEY_LOCAL_MACHINE, wzKeyName);
			}
			if(lResult !=ERROR_SUCCESS)
			{
				return E_UNEXPECTED;
			}
			_snwprintf_s(wzKeyName,MAX_PATH, _TRUNCATE, L"%s\\%s\\%s",KEYNAME_COMPLIANT_ENTERPRISE, wzEnforcerName,KEYNAME_REPOSITORY_ADAPTER);
			HKEY    hKeyReposAdapters = NULL ;
			lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, wzKeyName, 0, KEY_READ, &hKeyReposAdapters);
			if( lResult !=ERROR_SUCCESS)
			{
				_snwprintf_s(wzKeyName,MAX_PATH, _TRUNCATE, L"%s\\%s\\%s",KEYNAME_WOW32COMPLIANT_ENTERPRISE, wzEnforcerName,KEYNAME_REPOSITORY_ADAPTER);
				lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, wzKeyName, 0, KEY_READ, &hKeyReposAdapters);
			}
			if( lResult !=ERROR_SUCCESS)
			{
				return S_OK ;
			}
			
			wchar_t    achClass[MAX_PATH] = L"";  // buffer for class name 
			DWORD    cchClassName = MAX_PATH;  // size of class string 
			DWORD    cSubKeys=0;               // number of subkeys 
			DWORD    cbMaxSubKey;              // longest subkey size 
			DWORD    cchMaxClass;              // longest class string 
			DWORD    cValues;              // number of values for key 
			DWORD    cchMaxValue;          // longest value name 
			DWORD    cbMaxValueData;       // longest value data 
			DWORD    cbSecurityDescriptor; // size of security descriptor 
			FILETIME ftLastWriteTime;      // last write time 


			

			// Get the class name and the value count. 
			RegQueryInfoKeyW(
				hKeyReposAdapters,                    // key handle 
				achClass,                // buffer for class name 
				&cchClassName,           // size of class string 
				NULL,                    // reserved 
				&cSubKeys,               // number of subkeys 
				&cbMaxSubKey,            // longest subkey size 
				&cchMaxClass,            // longest class string 
				&cValues,                // number of values for this key 
				&cchMaxValue,            // longest value name 
				&cbMaxValueData,         // longest value data 
				&cbSecurityDescriptor,   // security descriptor 
				&ftLastWriteTime);       // last write time 
			if(NULL != hKeyReposAdapters) RegCloseKey(hKeyReposAdapters);
			if( cSubKeys == 0 )
			{
				lResult=RegDeleteKey(HKEY_LOCAL_MACHINE, wzKeyName);
				if(lResult !=ERROR_SUCCESS)
				{
					return E_UNEXPECTED;
				}
			}
			return S_OK;
		}
	private:
		std::wstring		wstrAdapterName;
		std::wstring		wstrImage;
		std::wstring		wstrObligationName;
		std::wstring		wstrEnforcer; //only works for registering
		HMODULE				hModule;
		FPRepositoryUpload	fpUpload;
		FPRepositoryUploadEx fpUploadEx;
		FPRepositoryReleasePWCH fpRelease;
	};
	

	#define KEYVALUE_IMAGE_STRING			L"Image"
	#define KEYVALUE_OBLIGATIONNAME_STRING	L"ObligationName"

	WCHAR Adapter::KEYNAME_COMPLIANT_ENTERPRISE[]=L"SOFTWARE\\NextLabs\\Compliant Enterprise";
	WCHAR Adapter::KEYNAME_WOW32COMPLIANT_ENTERPRISE[]=L"SOFTWARE\\Wow6432Node\\NextLabs\\Compliant Enterprise";
	WCHAR Adapter::KEYNAME_REPOSITORY_ADAPTER[]=L"Repository Adapters";
	WCHAR Adapter::KEYVALUE_IMAGE[]=KEYVALUE_IMAGE_STRING;
	WCHAR Adapter::KEYVALUE_OBLIGATIONNAME[]=KEYVALUE_OBLIGATIONNAME_STRING;

	WCHAR Adapter::ADAPTER_INI_SECTION_ADAPTER[]=L"Adapter";
	WCHAR Adapter::ADAPTER_INI_KEY_IMAGE[]=KEYVALUE_IMAGE_STRING;
	WCHAR Adapter::ADAPTER_INI_KEY_OBLIGATIONNAME[]=KEYVALUE_OBLIGATIONNAME_STRING;
	WCHAR Adapter::ADAPTER_INI_KEY_NAME[]=L"Name";
	WCHAR Adapter::ADAPTER_INI_KEY_ENFORCER[]=L"Enforcer";
	}
}


#endif
