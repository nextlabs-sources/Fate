#include "commonutils.hpp"

#include <Shlwapi.h>
#include <Shellapi.h>


#pragma warning(push)
#pragma warning(disable: 6387 6011) 
#include <atlbase.h>
#include <atlcom.h>
#include <atlctl.h>
#pragma warning(pop)

#pragma warning(push)
#pragma warning(disable: 4512 4244) 
#include <boost/smart_ptr.hpp>
#include <boost/thread.hpp>  
#pragma warning(pop)

#include "cetype.h"
#include "PALoad.h"
#include "cesdk.h"
#include "eframework/platform/cesdk_loader.hpp"
#include "eframework/platform/cesdk_attributes.hpp"

namespace
{
    const std::wstring kCopyAppendName = L"-Copy";

    typedef boost::shared_lock<boost::shared_mutex> boost_share_lock;  
    typedef boost::unique_lock<boost::shared_mutex> boost_unique_lock; 
    boost::shared_mutex gMutex;

	boost::shared_mutex gShowBubbleMutex;
	HWND gBubbleWnd = NULL;
	nextlabs::hyperlinkbubble::type_notify2 gNotify2 = NULL;
} // ns anonymous

namespace nextlabs
{
    namespace policyengine
    {
		nextlabs::cesdk_context gCesdkContext;

        PolicyResult DoEvaluation(WdeAction action, const std::wstring& strSourcePath)
        {
			nextlabs::Obligations obligations;
			return DoEvaluation(action, strSourcePath, L"", obligations, NULL, NULL);
        }

        PolicyResult DoEvaluation(WdeAction action, const std::wstring& strSourcePath, nextlabs::Obligations& obligations)
        {
			return DoEvaluation(action, strSourcePath, L"", obligations, NULL, NULL);
        }

        PolicyResult DoEvaluation(WdeAction action, const std::wstring& strSourcePath, const std::wstring& strDestPath)
        {
			nextlabs::Obligations obligations;
			return DoEvaluation(action, strSourcePath, strDestPath, obligations, NULL, NULL);
        }

        PolicyResult DoEvaluation(WdeAction action, const std::wstring& strSourcePath, const std::wstring& strDestPath, nextlabs::Obligations& obligations)
        {            
			return DoEvaluation(action, strSourcePath, strDestPath, obligations, NULL, NULL);
        }

		PolicyResult DoEvaluation(WdeAction action, const std::wstring& strSourcePath, const std::wstring& strDestPath, nextlabs::Obligations& obligations, strCEApplication* pceapp)
		{
			return DoEvaluation(action, strSourcePath, strDestPath, obligations, pceapp, NULL);
		}

		PolicyResult DoEvaluation(WdeAction action, const std::wstring& strSourcePath, const std::wstring& strDestPath, nextlabs::Obligations& obligations, strCEApplication* pceapp, ATTRS* psourceAttributes)
		{
            if (strSourcePath.empty())
            {
                return PolicyResultAllow;
            }
            
            {
                boost_unique_lock myLock(gMutex);
                static bool inited = false;
                if (!inited)
                {
                    nextlabs::comm_helper::Init_Cesdk(&nextlabs::policyengine::gCesdkContext);
                    inited = true;
                }
            }

			boost::shared_ptr<nextlabs::comm_base> evaluationPtr = nextlabs::comm_helper::CreateComm(nextlabs::type_cesdk, &gCesdkContext);
			nextlabs::eval_parms parm;

			switch (action)
			{
			case WdeActionRead:
				parm.SetAction(L"OPEN");
				break;

			case WdeActionWrite:
				parm.SetAction(L"EDIT");
				break;

			case WdeActionDelete:
				parm.SetAction(L"DELETE");
				break;

			case WdeActionPrint:
				parm.SetAction(L"PRINT");
				break;

			case WdeActionRun:
				parm.SetAction(L"RUN");
				break;

			case WdeActionChangeAttribute:
				parm.SetAction(L"CHANGE_ATTRIBUTES");
				break;

			case WdeActionChangeSecurityAttribute:
				parm.SetAction(L"CHANGE_SECURITY");
				break;

			case WdeActionEMail:
				parm.SetAction(L"EMAIL");
				break;

			case WdeActionIM:
				parm.SetAction(L"IM");
				break;

			case WdeActionCopyContent:
				parm.SetAction(L"PASTE");
				break;

			case WdeActionCopy:
				parm.SetAction(L"COPY");
				break;

			case WdeActionMove:
				parm.SetAction(L"MOVE");
				break;

			case WdeActionRename:
				parm.SetAction(L"RENAME");
				break;

			case WdeActionUpload:
				parm.SetAction(L"UPLOAD");
				break;

			case WdeActionDownload:
				parm.SetAction(L"DOWNLOAD");
				break;

			case WdeActionConvert:
				parm.SetAction(L"CONVERT");
				break;
            case WdeActionSend:
                parm.SetAction(L"SEND");
                break;
			default:
				return PolicyResultAllow;
			}

			std::wstring src = nextlabs::utils::FormatPath(strSourcePath);
			std::wstring dest = nextlabs::utils::FormatPath(strDestPath);

			if (psourceAttributes != NULL)
			{
				parm.SetSrc(src.c_str(), L"fso", psourceAttributes);	
			}
			else
			{
				parm.SetSrc(src.c_str());	
			}
			
			parm.SetTarget(dest.c_str());	

			CEApplication ceapp = { 0 };
			if (pceapp != NULL)
			{
				ceapp.appName = gCesdkContext.m_sdk.fns.CEM_AllocateString(pceapp->appName.c_str());
				ceapp.appPath = gCesdkContext.m_sdk.fns.CEM_AllocateString(pceapp->appPath.c_str());
				ceapp.appURL = gCesdkContext.m_sdk.fns.CEM_AllocateString(pceapp->appUrl.c_str());

				#pragma warning( push )
				#pragma warning( disable : 6309 6387 )
				parm.SetApplicationInfo(NULL, &ceapp);
				#pragma warning( pop )
			}

			PolicyResult pr = PolicyResultAllow;

			if(QueryPolicy(evaluationPtr, parm))
			{
				obligations = evaluationPtr->GetObligations();

				if (evaluationPtr->IsDenied())
				{
					pr = PolicyResultDeny;
				}

				if (parm.GetNoiseLevel() == CE_NOISE_LEVEL_USER_ACTION)
				{
					if(obligations.IsObligationSet(WDE_OBLIGATION_RICH_USER_MESSAGE))
					{
						const std::list<nextlabs::Obligation>& ob_list = obligations.GetObligations();

						for( std::list<nextlabs::Obligation>::const_iterator it = ob_list.begin() ; it != ob_list.end() ; ++it ) 
						{
							if( it->name == WDE_OBLIGATION_RICH_USER_MESSAGE )
							{
								nextlabs::ObligationOptions::const_iterator options_it = it->options.begin();

								std::wstring FirstValue = options_it->second.c_str();
								++options_it;
								std::wstring SecondValue = options_it->second.c_str();

								hyperlinkbubble::ShowBubble(FirstValue.c_str(), 1000 * _wtoi(SecondValue.c_str()), WdeActionMap(action));

								break;
							}
						}   
					}
				}
			}	

			if (pceapp != NULL)
			{
				gCesdkContext.m_sdk.fns.CEM_FreeString(ceapp.appName);
				gCesdkContext.m_sdk.fns.CEM_FreeString(ceapp.appPath);
				gCesdkContext.m_sdk.fns.CEM_FreeString(ceapp.appURL);
			}

            if (evaluationPtr->IsDenied())
            {
                //OutputDebugStringW(L"deny");
            }
            else
            {
                //OutputDebugStringW(L"allow");
            }
            
			return pr;
		}

		BOOL QueryPolicy(boost::shared_ptr<nextlabs::comm_base>& evaluationPtr, nextlabs::eval_parms& parm)
		{
			BOOL bRet = FALSE;

			__try
			{
				bRet = evaluationPtr->Query(&parm);
			}
			__except (EXCEPTION_EXECUTE_HANDLER)
			{
				bRet = TRUE;
			}

			return bRet;
		}

    }  // ns policyengine


	namespace policyassistant{
		class CPA
		{
		public:
			CPA(){ _sInit(); }

			inline bool perform(PABase::ACTION action,
				nextlabs::Obligations& obs,
				std::wstring& src,
				std::wstring& dst,
				bool bAllow)
			{
				return perform(action, obs, src, dst, bAllow, NULL, false, false);
			}
			
			inline bool perform(PABase::ACTION pam_action,
				nextlabs::Obligations& obs,
				std::wstring& src, 
				std::wstring& dst,
				bool bAllow, 
				const HWND _hParentWnd, 
				bool doWrapforSe, 
				bool bEncryptBeforeTag)
			{
				PA_Mngr::CPAMngr pam(policyengine::gCesdkContext.m_sdk.fns.CEM_GetString);
				return perform(pam, pam_action, obs,src, dst, bAllow, _hParentWnd, doWrapforSe, bEncryptBeforeTag);
			}
		private:
			bool perform(
				PA_Mngr::CPAMngr& pam,
				PABase::ACTION pam_action,
				nextlabs::Obligations& obs,
				std::wstring& src,
				std::wstring& dst,
				bool bAllow,
				const HWND _hParentWnd,
				bool doWrapforSe,
				bool bEncryptBeforeTag)
			{
				//	-set obs , obs should be returned by policy-engine
				pam.SetObligations(src.c_str(), src.c_str(), dst.c_str(), obs);
				

				// - as requirement-doc defined , this version support 2 pa
				//		- tagging obligation, i.e. DoFileTagging
				//		- zip obligation	, i.e. DoEcnryption

				LONG lRt = 0;
				lRt = pam.DoFileTagging(pa_tag_mod, NULL, pam_action, TRUE, _T("OK"), NULL, bEncryptBeforeTag);    // perform tagging
				if (0 != lRt)
				{
				 	return false;
				}
				
				WCHAR newFileName[MAX_PATH + 1] = { 0 };
				BOOL changed = FALSE;
				if (bAllow)
				{
					lRt = pam.DoEcnryption(pa_enc_mod, NULL, pam_action, TRUE);
					if (0 != lRt)
					{
						return false;
					}
					else
					{	
						pam.QueryRetName_bySrc(NULL, changed, newFileName);
						if (changed == TRUE)
						{
							src = newFileName;
							if (boost::algorithm::iends_with(src, L".zip") == true)
							{
								dst += L".zip";
							}
						}
					}
				}
				return true;
			}
		private:
			static void  _sInit();
		private:
			static	HMODULE pa_enc_mod;
			static	HMODULE pa_tag_mod;
			static	HMODULE paf_mod;
			static	HMODULE pa_pe_mod;
			static	HMODULE pdf_vl_mod;  // not used by current version
		};

		HMODULE CPA::pa_enc_mod = NULL;
		HMODULE CPA::pa_tag_mod = NULL;
		HMODULE CPA::paf_mod = NULL;
		HMODULE CPA::pa_pe_mod = NULL;
		HMODULE CPA::pdf_vl_mod = NULL;

		void CPA::_sInit(){
			if (pa_enc_mod != NULL &&
				pa_tag_mod != NULL &&
				paf_mod != NULL &&
				pa_pe_mod != NULL &&
				pdf_vl_mod != NULL){
				return;
			}
			// get Nextlabs install path , like C:\Program Files\NextLabs\Common\bin64
			wchar_t szDir[MAX_PATH] = { 0 };
			if (NLConfig::ReadKey(L"SOFTWARE\\NextLabs\\CommonLibraries\\InstallDir", szDir, MAX_PATH) && L'\0' != szDir[0])
			{
				if (szDir[wcslen(szDir) - 1] != L'\\')
				{
					wcsncat_s(szDir, MAX_PATH, L"\\", _TRUNCATE);
				}

#ifdef _WIN64
				wcsncat_s(szDir, MAX_PATH, L"bin64\\", _TRUNCATE);
#else
				wcsncat_s(szDir, MAX_PATH, L"bin32\\", _TRUNCATE);
#endif
			}

			PA_LOAD::LoadModuleByName(PA_LOAD::PA_MODULE_NAME_ENC, NULL, pa_enc_mod, szDir);
			PA_LOAD::LoadModuleByName(PA_LOAD::PA_MODULE_NAME_TAG, NULL, pa_tag_mod, szDir);
			PA_LOAD::LoadModuleByName(PA_LOAD::PAF_MODULE_NAME, NULL, paf_mod, szDir);
			PA_LOAD::LoadModuleByName(PA_LOAD::PA_MODULE_NAME_PE, NULL, pa_pe_mod, szDir);
			PA_LOAD::LoadModuleByName(PA_LOAD::PA_MODULE_NAME_NLVISUALABELING, NULL, pdf_vl_mod, szDir);
		}
	
		bool PAHelper(PABase::ACTION action, nextlabs::Obligations& obs, std::wstring& src, std::wstring& dst, 
			DWORD dwLastError, bool bAllow){

            TCHAR szPath[MAX_PATH] = {0};
            ::SHGetSpecialFolderPathW(NULL, szPath, CSIDL_APPDATA, FALSE);  
            std::wstring wstrTmpPath(szPath);
            size_t nPos = wstrTmpPath.rfind('\\');
            if (nPos != std::wstring::npos)
            {
                wstrTmpPath = wstrTmpPath.substr(0,nPos);  
            }

			if (obs.GetObligations().empty() || boost::istarts_with(dst.c_str(),wstrTmpPath + L"\\Local\\Microsoft\\Windows\\INetCache\\Content.MSO")){
				return true;
			}
			// oye: fileTagging will internally copy target file into [TMP]\Local\PA\** 
			//		it must be filtered out
			if (obs.GetObligations().empty() || boost::icontains(dst.c_str(),L"\\Local\\Temp\\PA\\")){
				return true;
			}
			// oye: for CBContext ,filter **\Local\PA\** 
			//		it must be filtered out
			if (obs.GetObligations().empty() || boost::icontains(dst.c_str(), L"\\Local\\Temp\\")) {
				return true;
			}
			CPA pa;
			bool rt = pa.perform(action, obs, src, dst, bAllow);
			::SetLastError(dwLastError);
			return rt;
		}

	}

    namespace comhelper
    {
        HRESULT NL_SHCreateShellItemArrayFromDataObject(_In_ IDataObject *pdo, _In_ REFIID riid,  void **ppv)
        {
            // vs 2008, not supprot shell32!SHCreateShellItemArrayFromDataObject , Windows Vista [desktop apps only]
            // so load it in runtime
            HMODULE hMod = LoadLibrary(L"shell32.dll");
            if (hMod == NULL){
                return E_FAIL;
            }
            f_SHCreateShellItemArrayFromDataObject f_shFunc = NULL;
            f_shFunc = (f_SHCreateShellItemArrayFromDataObject)GetProcAddress(hMod, "SHCreateShellItemArrayFromDataObject");
            HRESULT rt=E_FAIL;
            if (f_shFunc != NULL)
            {
                rt = f_shFunc(pdo, riid, ppv);
            }	
            FreeLibrary(hMod);
            return rt;
        }

        HRESULT GetPathByShellItem(std::wstring& path, IShellItem * pItem)
        {
            if (pItem == NULL){
                return E_FAIL;
            }

            wchar_t* pOutStr = NULL;
            HRESULT hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pOutStr);
            if (SUCCEEDED(hr) && pOutStr != NULL){
                path.assign(pOutStr);
                CoTaskMemFree(pOutStr);
            }
            return hr;
        }

        HRESULT CreateShellItemArrayFromPaths(UINT ct, std::vector<std::pair<std::wstring, nextlabs::Obligations>>& rgt, IShellItemArray **ppsia)
        {
            *ppsia = NULL;

            PIDLIST_ABSOLUTE *rgpidl = new(std::nothrow) PIDLIST_ABSOLUTE[ct];
            HRESULT hr = rgpidl ? S_OK : E_OUTOFMEMORY;

            UINT cpidl;
            for (cpidl = 0; SUCCEEDED(hr) && cpidl < ct; cpidl++)
            {
                hr = SHParseDisplayName(rgt.at(cpidl).first.c_str(), NULL, &rgpidl[cpidl], 0, NULL);
            }

            if (SUCCEEDED(hr))
            {
                hr = SHCreateShellItemArrayFromIDLists(cpidl, (PCIDLIST_ABSOLUTE_ARRAY)rgpidl, ppsia);
            }

            for (UINT i = 0; i < cpidl; i++)
            {
                CoTaskMemFree(rgpidl[i]);
            }

            delete[] rgpidl;
            return hr;
        }


        BOOL GetPathByShellArray(IShellItemArray* pShArray, std::vector<std::pair<std::wstring, nextlabs::Obligations>>& vecFiles)
        {
            if (pShArray == NULL) {
                return FALSE;
            }

            DWORD dwSize = 0;
            HRESULT hr = pShArray->GetCount(&dwSize);
            if (FAILED(hr) && dwSize == 0)
            {
                return FALSE;
            }

            std::wstring fullpath;
            IShellItem* pItem = NULL;
            for (DWORD i = 0; i < dwSize; i++){

                if (FAILED(pShArray->GetItemAt(i, &pItem))){
                    continue;
                }
                if (FAILED(GetPathByShellItem(fullpath, pItem))){
                    continue;
                }

                if (::PathIsDirectoryW(fullpath.c_str())){
                    // if is a directory, loop added the subdirectory files.

                    nextlabs::comhelper::ParseFolder(fullpath,vecFiles);
                }
                else{
					nextlabs::Obligations ob;
                    vecFiles.push_back(std::make_pair(fullpath, ob)); 
                }

            }
            return TRUE;
        }


        BOOL GetPathByShellArray(IShellItemArray* pShArray, std::vector<std::pair<std::wstring, nextlabs::Obligations>>& vecFiles, std::wstring& strSrcRoot)
        {
            strSrcRoot = L"";
           
            if (pShArray == NULL) {
                return FALSE; 
            }

            DWORD dwSize = 0;
            HRESULT hr = pShArray->GetCount(&dwSize);
            if (FAILED(hr) && dwSize == 0)
            {
                return FALSE;
            }

            std::wstring fullpath;
            IShellItem* pItem = NULL;
            for (DWORD i = 0; i < dwSize; i++){

                if (FAILED(pShArray->GetItemAt(i, &pItem))){ 
                    continue;
                }
                if (FAILED(GetPathByShellItem(fullpath, pItem))){
                    continue;
                }

                if (strSrcRoot.empty())
                {
                    strSrcRoot = fullpath;
                    size_t nPos = strSrcRoot.rfind('\\');
                    if (nPos == std::wstring::npos) nPos = strSrcRoot.rfind('/');
                    strSrcRoot.erase(nPos);
                }

                if (::PathIsDirectoryW(fullpath.c_str())){
                    // if is a directory, loop added the subdirectory files.
                    
                    nextlabs::comhelper::ParseFolder(fullpath,vecFiles);
                }
                else{
					nextlabs::Obligations ob;
                    vecFiles.push_back(std::make_pair(fullpath, ob)); 
                }

            }
            return TRUE;
        }


        BOOL GetPathByDateObject(IDataObject* pDataObject,  std::vector<std::pair<std::wstring, nextlabs::Obligations>>& vecFiles)
        {
            vecFiles.clear();

            // use a array
            CComPtr<IShellItemArray> pSArray = NULL;
            HRESULT hr = NL_SHCreateShellItemArrayFromDataObject(pDataObject, IID_IShellItemArray, (void**)&pSArray);
            if (FAILED(hr))
            {
                return FALSE;
            }

            // 	DWORD dwSize = 0;
            // 	hr = pSArray->GetCount(&dwSize);
            // 	if (FAILED(hr) && dwSize == 0)
            // 	{
            // 		return FALSE;
            // 	}
            // 
            // 	std::wstring fullpath;
            // 	IShellItem* pItem = NULL;
            // 	for (DWORD i = 0; i < dwSize; i++){
            // 		
            // 		if (FAILED(pSArray->GetItemAt(i, &pItem))){
            // 			continue;
            // 		}
            // 		if (FAILED(GetPathByShellItem(fullpath, pItem))){
            // 			continue;
            // 		}
            // 		
            // 		if (::PathIsDirectoryW(fullpath.c_str())){
            // 			// shit , maybe user want to copy/move/del a whole folder
            // 			::OutputDebugStringW(L"--oye---,tbd,for demo code ,ingore that user want to copy/move/del a whole folder\n");
            // 		}
            // 		else{
            // 			vecFiles.push_back(fullpath);
            // 		}		
            // 	}
            return GetPathByShellArray(pSArray,vecFiles);
        }

        std::vector<std::pair<std::wstring, nextlabs::Obligations>> GetFilePathsFromObject(IUnknown* pObj)
        {
            CComPtr<IDataObject> pData = NULL;
            std::vector<std::pair<std::wstring, nextlabs::Obligations>> vecFiles;

            HRESULT hr = pObj->QueryInterface(IID_IDataObject, (void**)&pData);
            if (SUCCEEDED(hr) && pData != NULL)
            { 
                
                GetPathByDateObject(pData, vecFiles);   
            }
            else if (E_NOINTERFACE == hr) // try IID_IShellItem
            {  

                CComPtr<IShellItem> pItem = NULL;
                hr = pObj->QueryInterface(IID_IShellItem, (void**)&pItem);  
                if (SUCCEEDED(hr) && pItem != NULL)
                {			
                    std::wstring path;
                    GetPathByShellItem(path, pItem);  
                    
					nextlabs::Obligations ob;
                    vecFiles.push_back(std::make_pair(path, ob));
                }
                else 
                {
                    //::OutputDebugStringW(L"not support IID_IShellItem, try IShellItemArray");
                    CComPtr<IShellItemArray> pSArray = NULL;
                    hr = pObj->QueryInterface(IID_IShellItemArray, (void**)&pSArray);  
                    if (SUCCEEDED(hr) && pSArray != NULL)
                    {			
                        
                        GetPathByShellArray(pSArray, vecFiles);
                    }
                }
            }

            return vecFiles; 
        }

        std::vector<std::pair<std::wstring, nextlabs::Obligations>> GetFilePathsFromObject(IUnknown* pObj, std::wstring& strSrcRoot)
        {
            CComPtr<IDataObject> pData = NULL;
            std::vector<std::pair<std::wstring, nextlabs::Obligations>> vecFiles;

            HRESULT hr = pObj->QueryInterface(IID_IDataObject, (void**)&pData);
            if (SUCCEEDED(hr) && pData != NULL)
            {   
                vecFiles.clear();
                CComPtr<IShellItemArray> pSArray = NULL;
                hr = NL_SHCreateShellItemArrayFromDataObject(pData, IID_IShellItemArray, (void**)&pSArray);
                if (SUCCEEDED(hr))
                {
                    GetPathByShellArray(pSArray, vecFiles, strSrcRoot);
                }

            }
            else if (E_NOINTERFACE == hr) // try IID_IShellItem
            {  
                //::OutputDebugStringW(L"not support IID_IDataObject, try IID_IShellItem\n");

                strSrcRoot = L"";
                CComPtr<IShellItem> pItem = NULL;
                hr = pObj->QueryInterface(IID_IShellItem, (void**)&pItem);  
                if (SUCCEEDED(hr) && pItem != NULL)
                {			
                    std::wstring path;
                    GetPathByShellItem(path, pItem); 
                    if (path.empty())
                    {
                        return vecFiles;
                    }

                    if (strSrcRoot.empty())
                    {
                        strSrcRoot = path;
                        size_t nPos = strSrcRoot.rfind('\\');
                        if (nPos == std::wstring::npos) nPos = strSrcRoot.rfind('/');
                        strSrcRoot.erase(nPos);
                    }

                    // if path is a directory, need recursively get subdirectory files.
                    if (::PathIsDirectoryW(path.c_str())){
                        // if is a directory, loop added the subdirectory files.
                       
                        nextlabs::comhelper::ParseFolder(path,vecFiles);  
                    }
                    else
                    {
						nextlabs::Obligations ob;
                        vecFiles.push_back(std::make_pair(path, ob));
                    }
                }
                else if (E_NOINTERFACE == hr)   
                {
                    //::OutputDebugStringW(L"not support IID_IShellItem, try IShellItemArray");
                    CComPtr<IShellItemArray> pSArray = NULL;
                    hr = pObj->QueryInterface(IID_IShellItemArray, (void**)&pItem);  
                    if (SUCCEEDED(hr) && pItem != NULL)
                    {	
                        
                        GetPathByShellArray(pSArray, vecFiles, strSrcRoot);
                    }
                } 

            }

            return vecFiles; 
        }


		void GetNewCreatedFileName(const std::wstring& strOrigPath, std::wstring& strRealPath)
		{
			std::wstring strDefName = strOrigPath;
			size_t nPos = strOrigPath.rfind(L'.');
			std::wstring strSuffix = L"";
			if(nPos != std::wstring::npos)
			{
				strDefName = strOrigPath.substr(0,nPos);
				strSuffix = strOrigPath.substr(nPos);
			}

			wchar_t szNum[32]={0};
			UINT nNum = 0;
			while(true)	
			{
				strRealPath = strDefName;
				if(nNum != 0)	
				{
					ZeroMemory(szNum,sizeof(szNum));
					strRealPath += L" (";
					_snwprintf_s(szNum,32, _TRUNCATE, L"%d",nNum);
					strRealPath += szNum;
					strRealPath += L")";
					nNum++;
				}
				else
				{
					nNum = 2;
				}
				strRealPath += strSuffix;
				if(!::PathFileExists(strRealPath.c_str()))	break;
			}
		}
   
        // the parameter strPath should be like C:\test
        VOID ParseFolder(const std::wstring& strPath, std::vector<std::pair<std::wstring, nextlabs::Obligations>>& vecFile)
        {

            bool bUnixPath = false;
            std::wstring strTmpPath(strPath);
            if (wcschr(strPath.c_str(),L'/') != NULL) bUnixPath = true;
            if (bUnixPath) strTmpPath.append(L"/*.*");
            else strTmpPath.append(L"\\*.*");
            WIN32_FIND_DATA FindFileData = {0};
            HANDLE hFile = ::FindFirstFileW(strTmpPath.c_str(),&FindFileData);
            if (hFile == INVALID_HANDLE_VALUE) return;


            do 
            {
                strTmpPath = strPath;
                if (StrCmpW(FindFileData.cFileName, L".") == 0 || StrCmpW(FindFileData.cFileName, L"..") == 0) continue;
                if (bUnixPath) strTmpPath.append(L"/");
                else strTmpPath.append(L"\\");
                strTmpPath.append(FindFileData.cFileName);
                 
                if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                {
                   
                    ParseFolder(strTmpPath,vecFile);
                } 
                else
                {
					nextlabs::Obligations ob;
                    vecFile.push_back(std::make_pair(strTmpPath, ob));
                }
            } while (FindNextFileW(hFile, &FindFileData)); 
			FindClose(hFile);
        } 

        VOID GenerateDestPath(const std::vector<std::wstring>& vecFiles, 
			const std::wstring& strDestFolder,
			const std::wstring& strSrcRoot, 
			std::vector<nextlabs::comhelper::FILEOP>& vecFileOP, bool bIsCopy)
        {

            for (std::vector<std::wstring>::const_iterator itr = vecFiles.begin(); itr != vecFiles.end(); ++itr)
            {
               
                const std::wstring& strSrc = *itr;
                std::wstring strRelativePath = L"";
                nextlabs::comhelper::GetRelativeFilePath(strSrc, strSrcRoot, strRelativePath);
                if (strRelativePath.empty()) continue; 

                std::wstring strDstFile = strDestFolder;
                boost::replace_all(strDstFile, L"/", L"\\");
                if (strDstFile[strDstFile.length() - 1] != '\\')
                {
                    strDstFile.append(L"\\", 1);
                }

                strDstFile += strRelativePath;

                // if copy a file (a.txt) and paste to the same directory, and the file name should append -Copy (a-Copy.txt).
                if (_wcsicmp(strSrc.c_str(),strDstFile.c_str()) == 0 && bIsCopy)
                {
                    size_t nPos = strDstFile.rfind(L'.');
                    if (nPos != std::wstring::npos)
                    {
                        std::wstring strTmpDst = strDstFile.substr(0,nPos);
                        strTmpDst += kCopyAppendName;
                        strTmpDst += strDstFile.substr(nPos);
                        strDstFile = strTmpDst;
                    } 
                    else
                    {
                        strDstFile += kCopyAppendName;
                    }

                }

                nextlabs::comhelper::FILEOP fileOp;
                fileOp.strSrc = strSrc;
                fileOp.strDst = strDstFile;
                fileOp.strConstSrc = strSrc;
                fileOp.strConstDst = strDstFile;
                vecFileOP.push_back(fileOp);
            }
        }

        VOID WrapMoveFileOperation(const std::wstring& strExistingFileName, const std::wstring& strNewFileName, std::vector<nextlabs::comhelper::FILEOP>& vecFileOP)
        {

            if (::PathIsDirectoryW(strExistingFileName.c_str()))
            {

                // if is a directory, loop added the subdirectory files.
                std::vector<std::pair<std::wstring, nextlabs::Obligations>> vecFiles;
                nextlabs::comhelper::ParseFolder(strExistingFileName,vecFiles);
                for (std::vector<std::pair<std::wstring, nextlabs::Obligations>>::const_iterator itr = vecFiles.begin(); itr != vecFiles.end(); ++itr)
                {
                    nextlabs::comhelper::FILEOP fileOp;
                    fileOp.strSrc = (*itr).first;
                    std::wstring strTmp = fileOp.strSrc.substr(strExistingFileName.length());
                    fileOp.strDst = strNewFileName + strTmp;
                    
                    vecFileOP.push_back(fileOp);
                }

            }
            else 
            {
                nextlabs::comhelper::FILEOP fileOp;
                fileOp.strSrc = strExistingFileName;
                fileOp.strDst = strNewFileName; 


                vecFileOP.push_back(fileOp);
            }
        }

        VOID ParseDeleteFolder(const std::wstring& strExistingFileName, std::vector<std::pair<std::wstring, nextlabs::Obligations>>& vecFile)
        {

            if (::PathIsDirectoryW(strExistingFileName.c_str()))
            {

                // if is a directory, loop added the subdirectory files.
                std::vector<std::pair<std::wstring, nextlabs::Obligations>> vecFiles;
                nextlabs::comhelper::ParseFolder(strExistingFileName, vecFiles);
                for (std::vector<std::pair<std::wstring, nextlabs::Obligations>>::const_iterator itr = vecFiles.begin(); itr != vecFiles.end(); ++itr)
                {
                    nextlabs::Obligations ob;
                    vecFile.push_back(std::make_pair((*itr).first.c_str(), ob));
                }
            }
            else 
            {
                nextlabs::Obligations ob;
                vecFile.push_back(std::make_pair(strExistingFileName, ob));
            }
        }

		VOID GetNameMapping(std::map<std::wstring, std::wstring>& NameMapping, const HANDLETOMAPPINGSW* pHandleToMappings)
		{
			if ( NULL == pHandleToMappings )
			{
				return;
			}

			for ( UINT i = 0; i < pHandleToMappings->uNumberOfMappings; i++ )
			{
				std::wstring OldPath = pHandleToMappings->lpSHNameMapping[i].pszOldPath;

				std::transform(OldPath.begin(), OldPath.end(), OldPath.begin(), toupper);

				NameMapping[OldPath] = pHandleToMappings->lpSHNameMapping[i].pszNewPath;
			}
		}

		BOOL HandleEFSObligation(LPCWSTR lpFileName)
		{
			BOOL bRet = TRUE;

			DWORD le = GetLastError();
			bRet = EncryptFileW(lpFileName);
			SetLastError(le);

			return bRet;
		}

        BOOL SHFileOperation(std::vector<nextlabs::comhelper::FILEOP>& vecFileOp, bool bIsCopy)
        {
            BOOL bSuccess = TRUE;
            SHFILEOPSTRUCTW theStruct;
            ZeroMemory(&theStruct,sizeof(theStruct));
            theStruct.fFlags = FOF_MULTIDESTFILES | FOF_NOCONFIRMMKDIR | FOF_WANTMAPPINGHANDLE; 
            if (bIsCopy)  theStruct.wFunc = FO_COPY;
            else theStruct.wFunc = FO_MOVE;          
            std::vector<nextlabs::comhelper::FILEOP>::iterator viter = vecFileOp.begin();
            size_t dwSrcLen = 0;
            size_t dwDstLen = 0;
            for(;viter != vecFileOp.end(); ++viter)
            {
                dwSrcLen += (*viter).strSrc.length();
                dwSrcLen += 1; // add end \0
                dwDstLen += (*viter).strDst.length();
                dwDstLen += 1;
            }
            dwSrcLen += 1; // end by \0\0
            dwDstLen += 1;
            wchar_t* szSrc = new wchar_t[dwSrcLen];
            memset(szSrc,0,dwSrcLen*sizeof(wchar_t));
            wchar_t* szDst = new wchar_t[dwDstLen];
            memset(szDst,0,dwDstLen*sizeof(wchar_t));

            size_t nSrcIndex = 0, nDstIndex = 0;
            for (viter = vecFileOp.begin(); viter != vecFileOp.end(); ++viter)
            {
                nextlabs::comhelper::FILEOP theOp = (*viter);
                memcpy_s(szSrc+nSrcIndex,(dwSrcLen-nSrcIndex)*2,theOp.strSrc.c_str(),theOp.strSrc.length()*2);
                nSrcIndex += theOp.strSrc.length() + 1; // 1 -- ??/0?? 
                memcpy_s(szDst+nDstIndex,(dwDstLen-nDstIndex)*2,theOp.strDst.c_str(),theOp.strDst.length()*2);
                nDstIndex += theOp.strDst.length() + 1;
            }
            theStruct.pFrom = szSrc;
            theStruct.pTo = szDst;
            bSuccess = SHFileOperationW(&theStruct) == 0? TRUE: FALSE;
            
            delete[] szSrc;
            delete[] szDst;

			std::map<std::wstring, std::wstring> NameMapping;

			if(bSuccess)
			{
				GetNameMapping(NameMapping, (const HANDLETOMAPPINGSW*)theStruct.hNameMappings);
			}

			SHFreeNameMappings(theStruct.hNameMappings);

			if (bSuccess)
			{
				for(viter = vecFileOp.begin(); viter != vecFileOp.end(); ++viter)
				{
					FILEOP& theFOP = (*viter);
					if(theFOP.obligations.IsObligationSet(WDE_OBLIGATION_ENCRYPT_NAME))
					{
						std::wstring DestinationFile = theFOP.strDst;
						std::transform(DestinationFile.begin(), DestinationFile.end(), DestinationFile.begin(), toupper);

						std::map<std::wstring, std::wstring>::const_iterator ci = NameMapping.find(DestinationFile);

						BOOL bRet = TRUE;

						if (ci != NameMapping.end())
						{
							bRet = HandleEFSObligation(ci->second.c_str());
						}
						else
						{
							bRet = HandleEFSObligation(theFOP.strDst.c_str());
						}

						if(!bRet && bIsCopy)
						{
							DeleteFileW(theFOP.strDst.c_str());
						}
					}

                    // Delete the original file if "Move" action succeeds and PA changes target file 
                    if(!bIsCopy && theFOP.strConstDst.compare(theFOP.strDst) != 0)
                    {
                        DeleteFileW(theFOP.strConstSrc.c_str());   
                    }
				}
			}

            return bSuccess;
        }

        BOOL DoesFileExist(const std::wstring& file) 
        {
            if (!file.empty())
            {
                WIN32_FIND_DATA data;
                HANDLE hFile = FindFirstFileW(file.c_str(),&data);
                if (hFile != INVALID_HANDLE_VALUE) return TRUE;
                else return FALSE;
            } 
            else
            {
                return FALSE;
            }
        }

        BOOL GetExplorerPath(std::wstring& destPath)
        {
            POINT point = {0, 0};
            ::GetCursorPos(&point);
            HWND hwndFind = ::WindowFromPoint(point);
            if (NULL == hwndFind)
            {
                return FALSE;
            }
            while(true) 
            {
                HWND hParent = ::GetParent(hwndFind);
                if(NULL == hParent)	
                {
                    break;
                }
                hwndFind = hParent;
            }
           
            CComPtr<IShellWindows> psw = NULL;
            HRESULT hr = CoCreateInstance(CLSID_ShellWindows, NULL, CLSCTX_ALL, IID_IShellWindows, (void**)&psw);
            if (FAILED(hr))
            {
                return FALSE;
            }
            
            VARIANT v;
            V_VT(&v) = VT_I4;
            CComPtr<IDispatch> pdisp = NULL;
            BOOL fFound = FALSE;
            for (V_I4(&v) = 0; !fFound && psw->Item(v, &pdisp) == S_OK; V_I4(&v)++) 
            {
                CComPtr<IWebBrowserApp> pwba = NULL;
                if (FAILED(pdisp->QueryInterface(IID_IWebBrowserApp, (void**)&pwba)))
                {
                    continue;
                }

                HWND hwndWBA = NULL;
                if (FAILED(pwba->get_HWND((LONG_PTR*)&hwndWBA)) || hwndWBA != hwndFind)
                {
                    continue;
                }

                fFound = TRUE;
                CComPtr<IServiceProvider> psp = NULL;
                if (FAILED(pwba->QueryInterface(IID_IServiceProvider, (void**)&psp))) 
                {
                    continue;
                }

                CComPtr<IShellBrowser> psb = NULL;
                if (FAILED(psp->QueryService(SID_STopLevelBrowser, IID_IShellBrowser, (void**)&psb)))
                {
                    continue;
                }

                CComPtr<IShellView> psv = NULL;
                if (FAILED(psb->QueryActiveShellView(&psv))) 
                {
                    continue;
                }

                CComPtr<IFolderView> pfv = NULL;
                if (FAILED(psv->QueryInterface(IID_IFolderView, (void**)&pfv))) 
                {
                    continue;
                }

                CComPtr<IPersistFolder2> ppf2 = NULL;
                if (FAILED(pfv->GetFolder(IID_IPersistFolder2, (void**)&ppf2))) 
                {
                    continue;
                }
                
                LPITEMIDLIST pidlFolder = NULL;
                if (FAILED(ppf2->GetCurFolder(&pidlFolder)))
                {
                    continue;
                }

                WCHAR tempPath[MAX_PATH] = { 0 };
                if (SHGetPathFromIDList(pidlFolder, tempPath)) 
                {
                    destPath = tempPath;
                }

                pdisp.Release();
            }

            return TRUE;
        }

        BOOL DataObj_CanGoAsync(IDataObject *pdtobj)
        {
            BOOL fDoOpAsynch = FALSE;
            IAsyncOperation* pao = NULL;
            if (SUCCEEDED(pdtobj->QueryInterface(IID_IAsyncOperation, (void **)&pao)))
            {
                BOOL fIsOpAsync = FALSE;
                if (SUCCEEDED(pao->GetAsyncMode(&fIsOpAsync)))
                {
                    if (fIsOpAsync)
                    {
                        fDoOpAsynch = TRUE;
                    }
                }
                pao->Release();
            }
            return fDoOpAsynch;
        }

        BOOL DataObj_GoAsyncForCompat(IDataObject *pdtobj)
        {
            BOOL bRet = FALSE;
            STGMEDIUM medium = { 0 };
            FORMATETC fmte = {CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
            if (SUCCEEDED(pdtobj->GetData(&fmte, &medium)))
            {
                // is there only one path in the hdrop?
                if (DragQueryFileW((HDROP)medium.hGlobal, (UINT)-1, NULL, 0) == 1)
                {
                    WCHAR szPath[MAX_PATH] = { 0 };
                    // is it the magical WS_FTP path ("%temp%WS_FTPENotify") that WS_FTP sniffs
                    // for in their copy hook?
                    if (DragQueryFileW((HDROP)medium.hGlobal, 0, szPath, ARRAYSIZE(szPath)) && StrStrIW(szPath, L"WS_FTPE\Notify"))
                    {
                        // yes, we have to do an async operation for app compat
                        bRet = TRUE;
                    }
                }
                ReleaseStgMedium(&medium);
            }
            return bRet;
        }

    }  // ns comhelper  

    namespace utils
    {
        // class CUri start
        void CUri::Normalize()
        {
            {
				if (uri_.empty())
				{
					return;
				}

                using namespace boost::algorithm;

                if(istarts_with(uri_, L"\\\\.\\") && !istarts_with(uri_, L"\\\\.\\UNC\\"))
                {
                    // it is a device ,not UNC
                    type_ = kUri_Device;
                }
				else if(istarts_with(uri_, L"\\\\?\\UNC\\"))
				{
					std::wstring wstr = L"\\";
					wstr += uri_.substr(7);
					uri_ = wstr;
					type_ = kUri_UNC;
				}
                else if(istarts_with(uri_, L"\\\\?\\") || istarts_with(uri_, L"\\??\\"))
                {
                    uri_ = uri_.substr(4);
                    type_ = kUri_FsPath;
                }
                else if(istarts_with(uri_, L"UNC\\"))
                {
                    std::wstring wstr = L"\\";
                    wstr += uri_.substr(3);
                    uri_ = wstr;
                    type_ = kUri_UNC;
                }
                else if(istarts_with(uri_,L"\\Device\\"))
                {
                    type_ = kUri_Device;
                }

                if( uri_.length() >= 3 && 
                    L'\\'  == uri_.c_str()[0] && 
                    L'\\' == uri_.c_str()[1] && 
                    L'\\' != uri_.c_str()[2])
                {
                    type_ =kUri_UNC;
                }

                if (uri_.length() >= 2 && 
                    L':' == uri_.c_str()[1] && 
                    L'a' <= tolower(uri_.c_str()[0]) && L'z' >= tolower(uri_.c_str()[0]))
                {
                    type_ = kUri_FsPath;
                }

                if (std::wstring::npos != uri_.find_first_of(L"://")) 
                {
                    type_ = kUri_URL;
                }

                // need parse the file path when use command line relative path to operate
                ParseCmdFilePath();

                // convert 8.3 path to long path.
                uri_ = MyGetLongPath(uri_);
            }
        }

        std::wstring CUri::MyGetLongPath(const std::wstring& path)
        {
            DWORD dwLen = GetLongPathName(path.c_str(), NULL, 0);
            if (dwLen == 0)
            {
                return path;
            }

            WCHAR* buffer = new WCHAR[dwLen + 1];
            memset(buffer, 0, (dwLen + 1) * sizeof(WCHAR));
            GetLongPathName(path.c_str(), buffer, dwLen);

            std::wstring longPath = std::wstring(buffer, dwLen);

            delete [] buffer;
            buffer = NULL;

            return longPath;
        }

        void CUri::ParseCmdFilePath()
        {
            std::wstring strTmp = uri_;

            DWORD dwPathSize =  GetCurrentDirectory(0, NULL);
            if (dwPathSize > 0)
            {
                WCHAR* buf = new WCHAR[dwPathSize + 1];
                memset(buf, 0, sizeof(WCHAR) * (dwPathSize + 1));
                GetCurrentDirectory(dwPathSize, buf);
                strTmp = std::wstring(buf);  // like: -- C:\kaka
                
                delete [] buf;
                buf = NULL;
            }

            boost::replace_all(uri_, L"/", L"\\");
            if (!boost::algorithm::icontains(uri_.c_str(),L"\\")) 
            {
                uri_ = strTmp + L"\\" + uri_;
            }
            else if (boost::algorithm::istarts_with(uri_.c_str(),L".\\"))
            {
                uri_ = strTmp + L"\\" + uri_.substr(2);
            }
            else if (boost::algorithm::istarts_with(uri_.c_str(),L"..\\"))
            {
                unsigned int nCount = 0;
                std::wstring wstr = uri_;
                do 
                {
                    nCount ++;
                    wstr = wstr.substr(3);

                    size_t nPos = strTmp.rfind('\\');
                    if (nPos != std::wstring::npos)
                    {
                        strTmp.erase(nPos);
                    }

                } while (boost::algorithm::istarts_with(wstr.c_str(),L"..\\"));

                uri_ = strTmp + L"\\" + wstr;
            }
        }
        // class CUri end

        std::wstring GetFilePathFromName(const std::wstring& fullPath)
        {
            if(fullPath.empty())
            {
                return fullPath;
            }
            size_t pos;
            wchar_t cslash = L'\\';
            pos = fullPath.find(L'/');
            if(pos != std::wstring::npos)
            {
                cslash = L'/';
            }
            pos = fullPath.rfind(cslash);
            if(pos == std::wstring::npos)
            {
                return fullPath;
            }
            else
            {
                return fullPath.substr(0, pos + 1);
            }
        }

        BOOL IsSameDirectory(const std::wstring& path1, const std::wstring& path2)
        {
            std::wstring dir1;
            std::wstring dir2;

            dir1 = path1.substr(0, path1.find_last_of(L"\\"));
            dir2 = path2.substr(0, path2.find_last_of(L"\\"));

            if(0 == _wcsicmp(dir1.c_str(), dir2.c_str()))
            {
                return TRUE;
            }

            return FALSE;
        }

        // Check to see if the destination is the recycle bin
        BOOL IsDestinationRecycleBin(const std::wstring& lpNewFileName)
        {
            if(lpNewFileName.empty())
            {
                return FALSE;
            }

            if(lpNewFileName.size() < 2 ) // must contain at least drive letter - i.e. 'C:'
            {
                return FALSE;
            }

            std::wstring in_lpNewFileName(lpNewFileName.c_str() + 1); // skip drive letter
            const wchar_t* recycle_bin_path[] = { L":\\RECYCLER\\" ,
                L":\\RECYCLED\\" ,
                L":\\$RECYCLE.BIN\\" ,
                L":\\$RECYCLED.BIN\\" };

            // search for recycle bin prefix
            for( std::size_t i = 0 ; i < _countof(recycle_bin_path) ; i++ )
            {
                if( boost::algorithm::istarts_with(in_lpNewFileName,recycle_bin_path[i]) == true )
                {
                    return TRUE;
                }
            }
            return FALSE;
        }

        BOOL GetFullFilePathFromTitle(const std::wstring& title, __out std::set<std::wstring>& outFiles, const std::set<std::wstring> files)
        {
            if (title.length() == 0)
            {
                return FALSE;
            }
             
            std::size_t pos = title.rfind(L"\\");
            std::wstring titlename = title;
            if (std::wstring::npos != pos && std::wstring::npos != pos + 1)
            {
                titlename = title.substr(pos + 1);
            }
            
            BOOL bFound = FALSE;
            for (std::set<std::wstring>::const_iterator itor = files.begin(); itor != files.end(); ++itor)
            {
                std::wstring curFilePath = itor->c_str();
                std::size_t index = curFilePath.rfind(L"\\");
                std::wstring filename = curFilePath;
                if (std::wstring::npos != index && std::wstring::npos != (index + 1))
                {
                    filename = curFilePath.substr(index + 1);
                }
                std::size_t length = filename.length();

				BOOL bHasFound = FALSE;

                if (titlename.length() > length)
                {
                    if (titlename.substr(0, length) == filename)
                    {
                        std::wstring specSymBol = titlename.substr(length, 1);
                        if (specSymBol == L" " || specSymBol == L"[" || specSymBol == L"]")
                        {
							bHasFound = TRUE;
                            outFiles.insert(curFilePath);
                            bFound = TRUE;
                        }
                    }
                }

				if (!bHasFound)
				{
					size_t DotPosition = filename.rfind(L'.');

					if (std::wstring::npos != DotPosition)
					{
						filename = filename.substr(0, DotPosition);
						length = filename.length();

						if (titlename.length() > length)
						{
							if (titlename.substr(0, length) == filename)
							{
								std::wstring specSymBol = titlename.substr(length, 1);
								if (specSymBol == L" " || specSymBol == L"[" || specSymBol == L"]")
								{
									outFiles.insert(curFilePath);
									bFound = TRUE;
								}
							}
						}
					}
				}
            }

            return bFound;
        }
        
        BOOL isContentData(IDataObject *pDataObject)
        {
            CComPtr<IEnumFORMATETC> pIEnumFORMATETC = NULL;
            
            //Enumerate clipboard data
            HRESULT hr = pDataObject->EnumFormatEtc(DATADIR_GET, &pIEnumFORMATETC);
            BOOL bContent = FALSE;
            if(FAILED(hr) || NULL == pIEnumFORMATETC) 
            {
                return bContent;
            }

            FORMATETC etc;
            while (1) 
            {
                memset(&etc, 0, sizeof(etc));
                if(S_FALSE == pIEnumFORMATETC->Next(1, &etc, NULL))
                {
                    break;
                }

                // Standard
                if(CF_TEXT == etc.cfFormat) 
                {
                    bContent = TRUE;
                } 
                else if(CF_BITMAP == etc.cfFormat) 
                {
                    bContent = TRUE;
                } 
                else if(CF_OEMTEXT == etc.cfFormat) 
                {
                    bContent = TRUE;
                } 
                else if(CF_UNICODETEXT == etc.cfFormat) 
                {
                    bContent = TRUE;
                } 
                else 
                {
                    WCHAR szFormat[256] = {0};
                    GetClipboardFormatNameW(etc.cfFormat, szFormat, 256);
                    if(wcscmp(szFormat, L"FileGroupDescriptor" ) == 0) 
                    {
                        //this email attachment
                        bContent = FALSE;
                        break;
                    }
                } 
            }

            pIEnumFORMATETC.Release();

            return bContent;
        }

        BOOL GetClipboardContentDataSource(IDataObject *pDataObject, const std::wstring& clipboardInfo, __out std::wstring& srcFilePath)
        {
            BOOL bRet = FALSE;
            if(pDataObject != NULL) 
            {
                bRet = GetOleContentClipboardDataSource(pDataObject, clipboardInfo, srcFilePath);
            } else {
                bRet = EnumClipboardToGetContentResource(srcFilePath);
            }

            if(bRet == FALSE) 
            {
                //Get the source from global shared memory
                if(clipboardInfo.size()) 
                {
                    srcFilePath = clipboardInfo;
                    bRet = TRUE;
                }
            }
            return bRet;
        }

        BOOL EnumClipboardToGetContentResource(std::wstring& srcFilePath)
        {
			return FALSE;
        }

        BOOL GetOleContentClipboardDataSource(IDataObject *pDataObject, const std::wstring& clipboardInfo, __out std::wstring& srcFilePath)
        {
            HRESULT hr;
            STGMEDIUM medium;
            OBJECTDESCRIPTOR  *od = NULL;
            BOOL bRet = FALSE;

            if(pDataObject == NULL) 
            {
                return FALSE;
            }
            FORMATETC fe = {(CLIPFORMAT)RegisterClipboardFormat(L"Object Descriptor"), NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};

            hr = pDataObject->QueryGetData(&fe);
            if(SUCCEEDED(hr)) 
            {
                hr = pDataObject->GetData(&fe, &medium);
                if(SUCCEEDED(hr)) 
                {
                    od = (OBJECTDESCRIPTOR*)GlobalLock(medium.hGlobal);
                    if(od)
                    {
                        srcFilePath = ((WCHAR*)((char*)od + od->dwSrcOfCopy));
                        if(std::wstring::npos != srcFilePath.find(L"Temporary Internet Files"))
                        {
                            //This is the temporary name for a file downloaded from sharepoint
                            srcFilePath = L"";
                        } 
                        else if(std::wstring::npos != srcFilePath.find(L"Microsoft Office Word Document"))
                        {
                            OutputDebugString(L"here is Microsoft Office Word Document from clipboard\n");
                            if(clipboardInfo.length())
                            {
                                srcFilePath = clipboardInfo;
                            }
                            else
                            {
                                srcFilePath = L"";
                            }
                        }
                        else 
                        {
                            size_t len = srcFilePath.size();
                            if(len > 2)
                            {
                                if(srcFilePath[len-1] == L'1' && srcFilePath[len-2] == L'!')
                                {
                                    //There is strange ending with !1; remove them
                                    srcFilePath.erase(srcFilePath.begin() + len - 2, srcFilePath.end());
                                }
                            }

                            if (std::wstring::npos != srcFilePath.find(L"\\") || std::wstring::npos != srcFilePath.find(L"/"))
                            {
                                bRet = TRUE;	
                                //for non-ole application to get the source 
                                //clipboardInfo = srcFilePath;
                            }
                        }
                        GlobalUnlock(medium.hGlobal);
                    }
                    ReleaseStgMedium(&medium);
                } 
                else
                { 
                    OutputDebugString(L"GetOleContentClipboardDataSource: failed ");
                }
            }
            return bRet;
        }

        BOOL GetFilesFromHDROP(HDROP hDrop, __out std::list<std::wstring>& files)
        {
            if (NULL == hDrop)
            {
                return FALSE;
            }
            UINT uCount = 0;

            //Get the count of files dropped.
            BOOL bSucceed = FALSE;
            uCount = DragQueryFile(hDrop, (UINT)-1, NULL, 0);
            for (UINT i = 0; i < uCount; ++i) 
            {
                WCHAR filePath[MAX_PATH] = {0};
                std::wstring wstrFile = L"";
                DragQueryFile(hDrop, i, filePath, MAX_PATH);
                wstrFile = filePath;
                boost::algorithm::to_lower(wstrFile);
                files.push_back(wstrFile);
                bSucceed = TRUE;
            }

            return bSucceed;
        }

		std::wstring GetCommonComponentsDir()
		{
			wchar_t szDir[MAX_PATH] = {0};
			if(NLConfig::ReadKey(L"SOFTWARE\\NextLabs\\CommonLibraries\\InstallDir", szDir, MAX_PATH) && L'\0'!=szDir[0])
			{
				if(szDir[wcslen(szDir) - 1] != L'\\')
				{
					wcsncat_s(szDir, MAX_PATH, L"\\", _TRUNCATE);
				}

			#ifdef _WIN64
				wcsncat_s(szDir, MAX_PATH, L"bin64\\", _TRUNCATE);
			#else
				wcsncat_s(szDir, MAX_PATH, L"bin32\\", _TRUNCATE);
			#endif
				return szDir;
			}

			return L"";
		}

		BOOL BJAnsiToUnicode (LPCSTR  pszAnsiBuff, LPWSTR lpWideCharStr, int cchDest)
		{
			// Call MadCodeHook
			// Avoids problems with calling MultiByteToWideChar from hook

			assert(pszAnsiBuff != NULL);
			assert(lpWideCharStr != NULL);
			assert(cchDest > 0);
			if ( (NULL == pszAnsiBuff) ||
				(NULL == lpWideCharStr) ||
				cchDest <= 0 )		// no room to shorten input string by null terminating it
			{
				return FALSE;	/* Fail */
			}

			int l = lstrlenA(pszAnsiBuff);
			CHAR ch = 0;
			if ( l >= cchDest )	// won't fit in dest buffer
			{
				// Copy as much of the string as possible
				ch = pszAnsiBuff[cchDest-1];	// save char
				((LPSTR) pszAnsiBuff)[cchDest-1] = 0;	// replace with null terminator
			}

			AnsiToWide (pszAnsiBuff, lpWideCharStr);

			if ( ch )	// restore character replaced by 0
				((LPSTR) pszAnsiBuff)[cchDest-1] = ch;

			return TRUE;
		}

        std::wstring FormatPath(const std::wstring& path)
        {
            CUri uri(path.c_str());
            return uri.GetUri();
        }

        BOOL CanIgnoreFile(const std::wstring& path)
        {
            if( boost::algorithm::iequals(path, L"CONIN$") ||
                boost::algorithm::iequals(path, L"CONOUT$") ||
                boost::algorithm::istarts_with(path, L"\\\\.\\PIPE\\") ||
                boost::algorithm::istarts_with(path, L"\\\\.\\TAPE") ||
                boost::algorithm::icontains(path, L":$") ||
                boost::algorithm::iends_with(path, L"*.*") || 
                boost::algorithm::istarts_with(path, L"C:\\Windows") || 
                boost::algorithm::icontains(path, L"\\AppData\\Local\\"))
            {
                return TRUE;
            }
            else if (boost::algorithm::icontains(path, L"\\AppData\\Roaming\\"))
            {
                if (!boost::algorithm::icontains(path, L"\\AppData\\Roaming\\GoodSync"))
                {
                    return TRUE;
                }
            }

            return FALSE;
        }

        BOOL GetDenyImageFilePath(std::wstring& imagefile)
        {
            wchar_t szDir[MAX_PATH + 1] = {0};
            if(!NLConfig::ReadKey(L"SOFTWARE\\NextLabs\\Compliant Enterprise\\Destop Enforcer\\InstallDir", szDir, MAX_PATH))
            {
                OutputDebugString(L"get deny image path failed");
                /*imagefile = L"";
                return FALSE;*/ 
                //TBD ReadKey always return failed, i can not fix this.
                wcsncpy_s(szDir, _countof(szDir), L"C:\\Program Files\\NextLabs\\Desktop Enforcer\\", _TRUNCATE);
            }
            wcsncat_s(szDir, MAX_PATH, L"bin\\ce_deny.gif", _TRUNCATE);
            std::wstring s(szDir);
            imagefile = s;
            return TRUE;
        }        
    }  // ns utils

	namespace hyperlinkbubble
	{
		void ShowBubble(const wchar_t* pHyperLink, int Timeout, const wchar_t* pAction)
		{
			static bool bFirst = false;

			if (!bFirst)
			{
				boost_unique_lock myLock(gShowBubbleMutex);
				if (!bFirst)
				{
					HANDLE hEvent = CreateEventW(NULL, TRUE, FALSE, NULL);

					if (hEvent != NULL)
					{
						CreateThread(NULL, 0, BubbleThreadProc, hEvent, 0, NULL);

						WaitForSingleObject(hEvent, INFINITE);
						CloseHandle(hEvent);

						bFirst = true;
					}
				}    
			}

			BubbleStruct ThisBubble = {pHyperLink, pAction};

			SendMessage(gBubbleWnd, WM_USER + 1, (WPARAM)&ThisBubble, Timeout);
		}

		DWORD WINAPI BubbleThreadProc(LPVOID lpParameter)
		{
			WCHAR CurrentDir[MAX_PATH] = { 0 };

			HMODULE hCurrentModule = NULL;

			if (GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, reinterpret_cast<LPCWSTR>(BubbleThreadProc), &hCurrentModule))
			{
				GetModuleFileNameW(hCurrentModule, CurrentDir, MAX_PATH);

				WCHAR* pCDirEnd = wcsrchr(CurrentDir, L'\\');
				if(NULL != pCDirEnd)
				{
					*pCDirEnd = NULL;
				}

#ifdef _WIN64
				wcsncat_s(CurrentDir, MAX_PATH, L"\\notification.dll", _TRUNCATE);
#else
				wcsncat_s(CurrentDir, MAX_PATH, L"\\notification32.dll", _TRUNCATE);
#endif

				HMODULE hmodule = LoadLibraryW(CurrentDir);

				if (hmodule == NULL)
				{
					SetEvent((HANDLE)lpParameter);
					return 0;
				}

				gNotify2 = (type_notify2)GetProcAddress(hmodule, "notify2");

				if (gNotify2 == NULL)
				{   
					SetEvent((HANDLE)lpParameter);
					return 0;
				}

				WNDCLASSEX wcex = { 0 };

				wcex.cbSize = sizeof(WNDCLASSEX);

				wcex.style			= 0;
				wcex.lpfnWndProc	= BubbleWndProc;
				wcex.cbClsExtra		= 0;
				wcex.cbWndExtra		= 0;
				wcex.hInstance		= hCurrentModule;
				wcex.hIcon			= NULL;
				wcex.hCursor		= NULL;
				wcex.hbrBackground	= NULL;
				wcex.lpszMenuName	= NULL;
				wcex.lpszClassName	= L"BubbleClass";
				wcex.hIconSm		= NULL;

				RegisterClassExW(&wcex);

				gBubbleWnd = CreateWindowW(L"BubbleClass", NULL, 0, 0, 0, 0, 0, NULL, NULL, hCurrentModule, NULL);;

				SetEvent((HANDLE)lpParameter);

				MSG msg = { 0 };

				while (GetMessage(&msg, NULL, 0, 0))
				{
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
			}

			return 0;
		}

		LRESULT CALLBACK BubbleWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
		{
			switch (message)
			{
			case WM_USER + 1:
				{   
					BubbleStruct* ThisBubble = (BubbleStruct*)wParam;
					NOTIFY_INFO Info = { 0 };
					wcsncpy_s(Info.methodName, 64, L"ShowNotification", _TRUNCATE);
					wcsncpy_s(Info.params[0], 256, L"Fri Mar 06 11:36:47 CST 2015", _TRUNCATE);
					wcsncpy_s(Info.params[1], 256, ThisBubble->pAction, _TRUNCATE);
					wcsncpy_s(Info.params[2], 256, L"file:///c:/fake", _TRUNCATE);
					wcsncpy_s(Info.params[3], 256, L"fake", _TRUNCATE);

					gNotify2(&Info, (int)lParam, 0, 0, (const WCHAR*)ThisBubble->pHyperLink);
				}

			case WM_PAINT:
				{
					PAINTSTRUCT ps = { 0 };
					BeginPaint(hWnd, &ps);
					EndPaint(hWnd, &ps);
				}
				break;

			case WM_DESTROY:
				PostQuitMessage(0);
				break;

			default:
				return DefWindowProc(hWnd, message, wParam, lParam);
			}

			return 0;
		}
	} //ns hyperlinkbubble

    namespace priorityHelper
    {
        bool SetObjectToLowIntegrity(HANDLE hObject, SE_OBJECT_TYPE type)
        {
            LPCWSTR LOW_INTEGRITY_SDDL_SACL_W = L"S:(ML;;NW;;;LW)";
            bool bRet = false;
            PSECURITY_DESCRIPTOR pSD = NULL;
            if (ConvertStringSecurityDescriptorToSecurityDescriptorW(LOW_INTEGRITY_SDDL_SACL_W, SDDL_REVISION_1, &pSD, NULL))
            {
                PACL pSacl = NULL;
                BOOL fSaclPresent = FALSE;
                BOOL fSaclDefaulted = FALSE;
                if (GetSecurityDescriptorSacl(pSD, &fSaclPresent, &pSacl, &fSaclDefaulted))
                {
                    DWORD dwErr = ERROR_SUCCESS;
                    dwErr = SetSecurityInfo (
                        hObject, type, LABEL_SECURITY_INFORMATION,
                        NULL, NULL, NULL, pSacl );
                    bRet = (ERROR_SUCCESS == dwErr);
                }

                LocalFree ( pSD );
            }

            return bRet;
        }

    } //ns priorityHelper

}  // ns nextlabs
