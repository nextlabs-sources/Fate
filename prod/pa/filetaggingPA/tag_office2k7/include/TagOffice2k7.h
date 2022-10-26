#ifndef __TAGOFFICE2K7_H
#define __TAGOFFICE2K7_H
////////////////////////////////////////////////////////////////////////// 
#include "stdafx.h"
#include <unzip.h>
#include <KtmW32.h>
#include "celog.h"
#include "eframework\platform\windows_file_server_enforcer.hpp"

#include "nlofficerep_only_debug.h"


enum OfficePropertiesType
{
	CustomProperties,
	SummaryProperties
};
typedef HANDLE
(APIENTRY* _CreateTransaction) (
				   IN LPSECURITY_ATTRIBUTES lpTransactionAttributes OPTIONAL,
				   IN LPGUID UOW OPTIONAL,
				   IN DWORD CreateOptions OPTIONAL,
				   IN DWORD IsolationLevel OPTIONAL,
				   IN DWORD IsolationFlags OPTIONAL,
				   IN DWORD Timeout OPTIONAL,
				   _In_opt_ LPWSTR Description
				   );


typedef WINBASEAPI
BOOL
(WINAPI* _CopyFileTransactedW)(
					_In_     LPCWSTR lpExistingFileName,
					_In_     LPCWSTR lpNewFileName,
					_In_opt_ LPPROGRESS_ROUTINE lpProgressRoutine,
					_In_opt_ LPVOID lpData,
					_In_opt_ LPBOOL pbCancel,
					_In_     DWORD dwCopyFlags,
					_In_     HANDLE hTransaction
					);


typedef BOOL
(APIENTRY* _CommitTransaction) (
				   IN HANDLE TransactionHandle
				   );

//typedef std::pair<std::wstring,std::wstring> FilePair;

typedef HRESULT (WINAPI* pfSHGetKnownFolderPath)(REFKNOWNFOLDERID, DWORD, HANDLE, PWSTR *);



enum PropType
{
	APP_PATH_TYPE,
	CORE_PATH_TYPE
};

//////////////////////////////////////////////////////////////////////////
extern HINSTANCE g_hInstance;
#define TONNY_DEBUG 1

class CTagOffice2k7
{
public:
	CTagOffice2k7::CTagOffice2k7(void):m_dwAttribute(0)
	{
		CoInitialize(NULL);
		wchar_t strPath[MAX_PATH]={0};
		DWORD dwLen = ::GetModuleFileNameW(g_hInstance,strPath,MAX_PATH);
		while(dwLen > 0)
		{
			if(strPath[dwLen--] == L'\\')	
			{
				strPath[dwLen+1]=L'\0';
				break;
			}
		}

#ifdef _WIN64
		wcsncat_s(strPath,MAX_PATH,L"\\zip_adapter.dll", _TRUNCATE);
#else
		wcsncat_s(strPath,MAX_PATH,L"\\zip_adapter32.dll", _TRUNCATE);
#endif

		

		//	init member data
		m_MyCommitTransaction = NULL;
		m_MyCopyFileTransactedW = NULL;
		m_MyCreateTransaction = NULL;
		m_hKtmW32 = NULL;
		m_hKernel32 = NULL;

		//	init transaction functions if we are in wfse pc env
		if (true == nextlabs::windows_fse::is_wfse_installed())
		{
			m_hKtmW32 = LoadLibraryW(L"KtmW32.dll");
			if (m_hKtmW32)
			{
				m_MyCreateTransaction = (_CreateTransaction)GetProcAddress(m_hKtmW32, "CreateTransaction");	
				m_MyCommitTransaction = (_CommitTransaction)GetProcAddress(m_hKtmW32, "CommitTransaction");	
			}
			m_hKernel32 = LoadLibraryW(L"Kernel32.dll");
			if (m_hKernel32)
			{
				m_MyCopyFileTransactedW = (_CopyFileTransactedW)GetProcAddress(m_hKernel32, "CopyFileTransactedW");	
			}
			if (!m_MyCreateTransaction || !m_MyCommitTransaction || !m_MyCopyFileTransactedW)
			{
				//	when we fail to load transaction function, we do nothing here, 
				//	but we'll make sure we check if those function pointer are not null before use it.
				TRACE(CELOG_WARNING, L"Missing Windows Transaction functions\n");
			}

			TRACE(CELOG_DEBUG, "load transaction function pointers successfully in tagoffice2k7");
		}
	}

	CTagOffice2k7::~CTagOffice2k7(void)
	{
		//Set file intrinsic attributes.	
		if (0 != m_dwAttribute)
		{
			SetFileAttributesW(m_strOrigFile.c_str(), m_dwAttribute);
		}

		if (m_hKtmW32)
		{
			FreeLibrary(m_hKtmW32);
			m_hKtmW32 = NULL;
		}
		if (m_hKernel32)
		{
			FreeLibrary(m_hKernel32);
			m_hKernel32 = NULL;			
		}
		m_MyCreateTransaction = NULL;
		m_MyCommitTransaction = NULL;
		m_MyCopyFileTransactedW = NULL;

		CoUninitialize();
	}

	std::wstring newGUID()
	{
		wchar_t wszGuid[65] = { 0 };
		::GUID guid = { 0 };
		HRESULT hr = ::CoCreateGuid(&guid);
		if (SUCCEEDED(hr))
		{
			swprintf_s(wszGuid, 64, L"{%08X-%04X-%04x-%02X%02X-%02X%02X%02X%02X%02X%02X}"
				, guid.Data1
				, guid.Data2
				, guid.Data3
				, guid.Data4[0], guid.Data4[1]
				, guid.Data4[2], guid.Data4[3], guid.Data4[4], guid.Data4[5]
				, guid.Data4[6], guid.Data4[7]
				);
			return wszGuid;
		}
		return L"";
	}

	std::wstring NLGetLongFilePath(const std::wstring& kwstrFileShortPath)
	{
		/** check parameters */
		if (kwstrFileShortPath.empty())
		{
			return L"";
		}

		/** Get long file path */
		std::wstring wstrFileLongPath(L"");

		wchar_t* pwchLongTempPath = new wchar_t[1024];
		DWORD dwBufferLength = GetLongPathNameW(kwstrFileShortPath.c_str(), pwchLongTempPath, 1023);
		if (1023 < dwBufferLength)
		{
			/** the buffer is too small and get long path again */
			delete[] pwchLongTempPath;

			pwchLongTempPath = new wchar_t[dwBufferLength + 1];
			GetLongPathNameW(kwstrFileShortPath.c_str(), pwchLongTempPath, dwBufferLength);
		}

		if (NULL != pwchLongTempPath)
		{
			wstrFileLongPath = pwchLongTempPath;
			delete[] pwchLongTempPath;
		}

		return wstrFileLongPath;
	}

	std::wstring NLGetSysTempFilePath(const BOOL kbGetLongPath)
	{
		std::wstring wstrTempFilePath(L"");

		wchar_t wszTempFilePath[1024] = { 0 };
		DWORD dwRet = GetTempPath(1023, wszTempFilePath);
		if (0 != dwRet)
		{
			/** Get long temp path */
			if (kbGetLongPath)
			{
				wstrTempFilePath = NLGetLongFilePath(wszTempFilePath);
				if (wstrTempFilePath.empty())
				{
					wstrTempFilePath = wszTempFilePath;
				}
			}
			else
			{
				wstrTempFilePath = wszTempFilePath;
			}
		}
		return wstrTempFilePath;
	}

	std::wstring CreateTempFile(const wchar_t* wszExtentsion, BOOL bGetLongPath)
	{
		std::wstring wstrFilePath = NLGetSysTempFilePath(bGetLongPath);
		if (!wstrFilePath.empty())
		{
			wstrFilePath += L"nlTag";
			wstrFilePath += newGUID();
			wstrFilePath += wszExtentsion;
		}
		return wstrFilePath;
	}

	
	// add by tonny for compress folder
	static bool CheckOffice2k7file(const wchar_t* strOffice2k7FilePath)
	{
		
		if(!PathFileExistsW(strOffice2k7FilePath))		return false;

		
		WIN32_FIND_DATA FindFileData;
		HANDLE hFind = INVALID_HANDLE_VALUE;
		hFind = FindFirstFileW( strOffice2k7FilePath,&FindFileData );
		if(hFind == INVALID_HANDLE_VALUE)	return false;
		FindClose(hFind);
		if(FindFileData.nFileSizeLow < 1)	return false;

		//1. check it should not be NULL
		if(strOffice2k7FilePath==NULL)	return false;
		//2. check whether it is a office file
		const wchar_t* pSuffix = wcsrchr(strOffice2k7FilePath,L'.');
		if(pSuffix == NULL)	return false;
		bool bFind = false;
		const wchar_t* table[] = { L".docx",L".docm",L".dotx",L".dotm",L".xlsm",L".xlsb",L".xlsx",L".xltm",\
			L".xltx",L".xlam",L".potx",L".ppsm",L".ppsx",L".pptm",L".pptx",L".potm",L".ppam"};
		unsigned int nsize = _countof(table);
		for(unsigned int i=0;i<nsize;i++)
		{
			if(_wcsicmp(pSuffix,table[i]) == 0)
			{
				bFind = true;
				break;
			}
		}
		
		if(!bFind)	return bFind;
		
		return bFind;
	}
private:

#if 1   // add by Tonny at 4/29/2016, we need to remove summery tag before add them in custom properties.
    void RemoveSummeryTagFromPairBeforeAdd(std::vector<TAGPAIR>& vecTag, const std::vector<TAGPAIR>& vecSummeryTag)
    {
        for (std::vector<TAGPAIR>::const_iterator iter = vecSummeryTag.begin();
            iter != vecSummeryTag.end(); iter++)
        {
            const wstring& strSName = iter->strTagName;
            const wstring& strSValue = iter->strTagValue;
            for (std::vector<TAGPAIR>::iterator init = vecTag.begin(); init != vecTag.end(); ++init)
            {
                if (_wcsicmp(init->strTagName.c_str(), strSName.c_str()) == 0 &&
                    _wcsicmp(init->strTagValue.c_str(), strSValue.c_str()) == 0)
                {
                    vecTag.erase(init);
                    break;
                }
            }
        }
    }

#endif

    //Add XML custom property for office2k7
	int	AddTag(const wchar_t* wstrFilePath,std::vector<TAGPAIR>& vecTag,bool bOverWrite)
	{
		NLONLY_DEBUG;
        if (wstrFilePath == NULL || vecTag.empty())   return E_FAIL;
		NLPRINT_DEBUGVIEWLOGEX(true, L"Enter AddTag, file:%s", wstrFilePath);
		HRESULT hr = E_FAIL;
		ATL::CComPtr<IOpcFactory>    pFactory = NULL;
		hr = CoCreateInstance(__uuidof(OpcFactory), NULL, CLSCTX_INPROC_SERVER,__uuidof(IOpcFactory), (LPVOID*)&pFactory);
        if (FAILED(hr)) return hr;

		if (SUCCEEDED(hr) && pFactory != NULL)
		{
			ATL::CComPtr<IOpcPackage>    pPackage;
			hr = opclib::LoadPackage(pFactory, wstrFilePath, &pPackage);
			if (SUCCEEDED(hr) && pPackage!=NULL)
			{
#if 1  // add by Tonny at 4/29/2016, we need to remove summery tag before add them in custom properties.
                std::vector<TAGPAIR> vecSummeryTag;
                hr = opclib::GetCoreProperties(pPackage, vecSummeryTag);
                if (FAILED(hr))
                {
                    NLPRINT_DEBUGVIEWLOGEX(true, L"GetCoreProperties Faild %x", hr);
                }
                hr = opclib::GetExtendedProperties(pPackage, vecSummeryTag);
                if (FAILED(hr))
                {
                    NLPRINT_DEBUGVIEWLOGEX(true, L"GetExtendedProperties Faild %x", hr);
                }
                // merge to vecTag.
                RemoveSummeryTagFromPairBeforeAdd(vecTag, vecSummeryTag);
#endif 

				ATL::CComPtr<IOpcPart>    pCustomPropertiesPart;
				hr =opclib::FindCustomPropertiesPart(pPackage, &pCustomPropertiesPart);
				if (FAILED(hr))
				{
					hr= opclib::CreateCustomPropertiesPart(pFactory, pPackage, &pCustomPropertiesPart);
					if (FAILED(hr) || pCustomPropertiesPart == NULL)
					{
						NLPRINT_DEBUGVIEWLOGEX(true,L"CreateCustomPropertiesPart Faild %x",hr);
                        return hr;
					}
				}

				hr = opclib::AddCustomProperties(pPackage, pCustomPropertiesPart,vecTag, bOverWrite);
			
				if (SUCCEEDED(hr))
				{
					hr = opclib::SavePackage(pFactory, pPackage, m_strResultFileName.c_str());
					if (SUCCEEDED(hr) )
					{
						pCustomPropertiesPart.Release();
						pPackage.Release();
						BOOL bCopy = CopyFileEx(m_strResultFileName.c_str(), wstrFilePath, NULL, NULL, NULL, 0);  
					    DeleteFileW(m_strResultFileName.c_str());
						NLPRINT_DEBUGVIEWLOGEX(true, (bCopy ? L"Move File on Addtag Success" : L"Move file on Addtag Failed.") );
					}
					else
					{
						NLPRINT_DEBUGVIEWLOGEX(true, L"Move File %s Faild", wstrFilePath);
					}

				}
				else
				{
					NLPRINT_DEBUGVIEWLOGEX(true,L"AddCustomProperties Faild %x",hr);
				}
			}
			else
			{
				NLPRINT_DEBUGVIEWLOGEX(true,L"CoCreateInstance Faild %x",hr);
			}
		}
		else
		{
			NLPRINT_DEBUGVIEWLOGEX(true,L"LoadPackage Faild %x",hr);	
		}
		
		
		
		return hr;

	}
	int DeleteCustomProperty(const wchar_t* wstrFilePath,std::vector<TAGPAIR>&vecTag)
	{
  		NLONLY_DEBUG;
		HRESULT hr;
		ATL::CComPtr<IOpcFactory>    pFactory;
		
		hr = CoCreateInstance(__uuidof(OpcFactory), NULL, CLSCTX_INPROC_SERVER, __uuidof(IOpcFactory), (LPVOID*)&pFactory);
		if (SUCCEEDED(hr))
		{
			ATL::CComPtr<IOpcPackage>    pPackage;
			hr = opclib::LoadPackage(pFactory, wstrFilePath, &pPackage);
			if (SUCCEEDED(hr))
			{
				ATL::CComPtr<IOpcPart>    pCustomPropertiesPart;
				hr = opclib::FindCustomPropertiesPart(pPackage, &pCustomPropertiesPart);
				if (SUCCEEDED(hr)&&pCustomPropertiesPart!=NULL)
				{
					hr = opclib::RemoveCustomProperties(pPackage, pCustomPropertiesPart, vecTag);
					if (SUCCEEDED(hr))
					{
						hr = opclib::SavePackage(pFactory, pPackage, m_strResultFileName.c_str());
						if (SUCCEEDED(hr))
						{
							pCustomPropertiesPart.Release();
							pPackage.Release();
							BOOL bCopy = CopyFileEx(m_strResultFileName.c_str(), wstrFilePath, NULL, NULL, NULL, 0);
							DeleteFileW(m_strResultFileName.c_str());
							NLPRINT_DEBUGVIEWLOGEX(true, (bCopy ? L"Move File on DeleteCustomProperty Success" : L"Move file on DeleteCustomProperty Failed.") );
						}
						else
						{
							NLPRINT_DEBUGVIEWLOGEX(true, L"SavePackage on  DeleteCustomProperty Faild");
						}
					}
					else
					{
						NLPRINT_DEBUGVIEWLOGEX(true,L"LoadPackage Faild %x",hr);
					}
				}
				else
				{
					NLPRINT_DEBUGVIEWLOGEX(true,L"FindCustomPropertiesPart, this file did't had any custom properties Faild %x",hr);
				}
			}
			else
			{
				NLPRINT_DEBUGVIEWLOGEX(true,L"LoadPackage Faild %x",hr);
			}
		}
		else
		{
			NLPRINT_DEBUGVIEWLOGEX(true,L"CoCreateInstance Faild %x",hr);
		}
	
		return hr;
		
	}

	int GetSummaryProperty(const wchar_t* wstrFilePath,std::vector<TAGPAIR>&vecTag)
	{
		NLONLY_DEBUG;
		HRESULT hr;
		ATL::CComPtr<IOpcFactory>    factory;
		hr = CoCreateInstance(__uuidof(OpcFactory), NULL, CLSCTX_INPROC_SERVER, __uuidof(IOpcFactory), (LPVOID*)&factory);
		if (SUCCEEDED(hr))
		{
			ATL::CComPtr<IOpcPackage>    package;
			hr = opclib::LoadPackage(factory, wstrFilePath, &package);
			if (SUCCEEDED(hr)&&package!=NULL)
			{
				hr = opclib::GetCoreProperties(package,vecTag);
				if (FAILED(hr))
				{
					NLPRINT_DEBUGVIEWLOGEX(true,L"GetCoreProperties Faild %x",hr);
				}
				hr=opclib::GetExtendedProperties(package, vecTag);
				if (FAILED(hr))
				{
					NLPRINT_DEBUGVIEWLOGEX(true,L"GetExtendedProperties Faild %x",hr);
				}
				
			}
			else
			{
				NLPRINT_DEBUGVIEWLOGEX(true,L"LoadPackage Faild %x",hr);
			}
		}
		else
		{
			NLPRINT_DEBUGVIEWLOGEX(true,L"CoCreateInstance Faild %x",hr);
		}
		return SUCCEEDED(hr)?ERROR_NO_ERROR:GENEROR_ERROR;
	}
#if 1   // add by Tonny at 4/29/2016, Add summery tags in vector.
    void MergeSummeryTagsIn(std::vector<TAGPAIR>&TargetTag, const std::vector<TAGPAIR>&vecSummeryTag)
    {
        std::vector<TAGPAIR>::const_iterator it = vecSummeryTag.begin();
        for (; it != vecSummeryTag.end(); ++it)
        {
            if (!it->strTagValue.empty())
            {
                TargetTag.push_back(*it);
            }
        }
    }
#endif
	int GetCustomProperty(const wchar_t* wstrFilePath,std::vector<TAGPAIR>&vecTag)
	{
		NLONLY_DEBUG;
		HRESULT hr;
		ATL::CComPtr<IOpcFactory>    factory;
		
		hr = CoCreateInstance(__uuidof(OpcFactory), NULL, CLSCTX_INPROC_SERVER, __uuidof(IOpcFactory), (LPVOID*)&factory);
		if (SUCCEEDED(hr))
		{
			ATL::CComPtr<IOpcPackage>    package;
			// Load the package file.
			hr = opclib::LoadPackage(factory, wstrFilePath, &package);
			if (SUCCEEDED(hr)&&package!=NULL)
			{
				hr = opclib::GetCustomProperties(package, vecTag);
				if(FAILED(hr))
				{
					NLPRINT_DEBUGVIEWLOGEX(true,L"GetCustomProperties  Faild , this file did had custom properties %x",hr);
					hr=S_OK;
				}
#if 1   // add by Tonny at 4/29/2016, after that, we only have pc read all tags instead of read tag by pep and pc both.
                std::vector<TAGPAIR> vecSummeryTag;
                hr = opclib::GetCoreProperties(package, vecSummeryTag);
                if (FAILED(hr))
                {
                    NLPRINT_DEBUGVIEWLOGEX(true, L"GetCoreProperties Faild %x", hr);
					hr = S_OK;  //if get core prop failed, may be it is not exist, so we set the result to OK. so the caller can get custom prop
                }
                hr = opclib::GetExtendedProperties(package, vecSummeryTag);
                if (FAILED(hr))
                {
                    NLPRINT_DEBUGVIEWLOGEX(true, L"GetExtendedProperties Faild %x", hr);
					hr = S_OK;
                }
                // merge to vecTag.
          		MergeSummeryTagsIn(vecTag,vecSummeryTag);
#endif
			}
			else
			{
				NLPRINT_DEBUGVIEWLOGEX(true,L"LoadPackage Faild %x",hr);
			}
		}
		else
		{
			NLPRINT_DEBUGVIEWLOGEX(true,L"CoCreateInstance Faild %x",hr);
		}	
		return SUCCEEDED(hr) ? ERROR_NO_ERROR : GENEROR_ERROR;
	}
	
public:
////////////////////////////////////////////////////////////////////
    //type 0 mean custom tag ,type 1 mean summary tag
	// return 1 is tag ok. 0 is fail. -1 is invalid office file, -2 File is using by another application
	int DoOffice2k7Tag(const wchar_t* strOffice2k7FilePath,std::vector<TAGPAIR>& vecTag,tagOperate tagMethod,bool bRewriteIfExists=true)
	{
        wchar_t buf[1024] = { 0 };

		m_strOrigFile = strOffice2k7FilePath;
		m_strResultFileName = CreateTempFile(L".tmp", TRUE);
		m_tagMethod = tagMethod;
		m_bRewriteIfExists = bRewriteIfExists;



		if(!PathFileExistsW(m_strOrigFile.c_str()))
		{
			TRACE(CELOG_DEBUG, L"FILE_NOT_EXIST");
			return FILE_NOT_EXIST;
		}

		if (tagMethod != enumReadTag)
		{
			// if read-only, drop this attribute and save
			DWORD dwAttribute = GetFileAttributesW(m_strOrigFile.c_str());
			m_dwAttribute = dwAttribute;
			if(dwAttribute & FILE_ATTRIBUTE_READONLY)
			{
				 m_bReadOnly=true;
				 dwAttribute &= ~FILE_ATTRIBUTE_READONLY;
				 if(!SetFileAttributesW(m_strOrigFile.c_str(),dwAttribute))
				 {
					 TRACE(CELOG_DEBUG, L"READ_ONLY_FILE");
					 return READ_ONLY_FILE;
				 }
			}
			else 
			{
				m_bReadOnly=false;
			}
			if (dwAttribute & FILE_ATTRIBUTE_HIDDEN)
			{
				TRACE(CELOG_DEBUG, L"The file has hidden Attributes");
				m_bHidden = true;
				dwAttribute &= ~FILE_ATTRIBUTE_HIDDEN;
				if (!SetFileAttributesW(m_strOrigFile.c_str(), dwAttribute)) 
				{
					TRACE(CELOG_DEBUG, L"READ_ONLY_FILE");
					return HIDDEN_FILE;
				}
			}
			else 
			{
				m_bHidden = false;
			}		
		}

		int nRegFlag = ERROR_NO_ERROR;
		//////////////////////////////////////////////////////////////////////////
		switch(tagMethod)
		{
		case enumAddTag:
			{
				nRegFlag = AddTag(m_strOrigFile.c_str(),vecTag,bRewriteIfExists);
			}
			break;
		case enumReadTag:
			{
				nRegFlag=GetCustomProperty(m_strOrigFile.c_str(),vecTag);
			}
			break;
		case enumReadSummaryTag:
			{
				nRegFlag=GetSummaryProperty(m_strOrigFile.c_str(),vecTag);	
			}
			break;
		case enumDeleteTag:
			{
				nRegFlag=DeleteCustomProperty(m_strOrigFile.c_str(),vecTag);
			}
			break;
		default:
			{
				
			}
			break;
		}
		
		
		if(SUCCEEDED(nRegFlag))
		{
			return ERROR_NO_ERROR;
		}
		else
		{
			return GENEROR_ERROR;
		}
		
	}
private:
	std::wstring m_strOrigFile;	// pass in the office file path
	std::wstring m_strResultFileName;//pass in office tag out file
	std::wstring m_strUnzipFolder;	// Unzip to here
	std::wstring m_strZipFile;	//temp zip file
	tagOperate m_tagMethod;		// add,read,delete tag information
	bool m_bRewriteIfExists;	// just for add tag
	
	bool m_bReadOnly;
	bool m_bHidden;
	DWORD m_dwAttribute;

	_CommitTransaction m_MyCommitTransaction;
	_CopyFileTransactedW m_MyCopyFileTransactedW;
	_CreateTransaction m_MyCreateTransaction;

	HMODULE m_hKtmW32;
	HMODULE m_hKernel32;
};

#endif //__TAGOFFICE2K7_H
