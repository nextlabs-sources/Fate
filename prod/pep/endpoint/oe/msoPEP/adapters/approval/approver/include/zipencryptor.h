#ifndef __ZIPENCRYPTOR_H__
#define __ZIPENCRYPTOR_H__
#include "../include/AdapterBase.h"
#include "rdPasswd.h"
#include <shlobj.h>
#include <string>

class CZipEncryptor
{
private:
	typedef EA_Error (WINAPI *FTEncrypt)(EncryptionAdapterData *lpData);
	static const WCHAR MODULENAME_ZIPADAPTER[];
public:
	CZipEncryptor(HMODULE hModule)
	{
		if(m_hZipAdapterModule==NULL)
		{
			WCHAR wzPath[MAX_PATH]={0};
			/*DWORD dwLen = ::GetCurrentDirectory(MAX_PATH,wzPath);
			if(dwLen)
				m_strCurrDir=wzPath;*/
			::GetModuleFileName(hModule,wzPath,MAX_PATH);
			WCHAR*pSlash=wcsrchr(wzPath,L'\\');
			if(pSlash==NULL)
			{
				pSlash=wcsrchr(wzPath,L'/');
				if(pSlash)
					pSlash[0]=0;
			}
			else
				pSlash[0]=0;
			std::wstring strZipAdapterPath=wzPath;
			strZipAdapterPath+=L"\\";
			strZipAdapterPath+=MODULENAME_ZIPADAPTER;
			m_hZipAdapterModule=LoadLibraryW(strZipAdapterPath.c_str());
			if(m_hZipAdapterModule == NULL)	
			{
				std::wstring strErr=L"Load library failed! the Path is:";
				strErr+=strZipAdapterPath;
				DP((L"%s",strErr.c_str()));
				m_fpEncrypt=NULL;
			}
			else
			{
				m_fpEncrypt = (FTEncrypt)GetProcAddress(m_hZipAdapterModule,"Encrypt");
			}

		}
	};

	HRESULT Encrypt(const WCHAR* wzFile,WCHAR*wzRetFileName,const WCHAR*wzPasswd=NULL)
	{
		if(_waccess(wzFile,0)!=0)
			return S_FALSE;

		EncryptionAdapterData adapterData;
		adapterData.wstrSrcFile=wzFile;

		std::wstring::size_type posSlash=adapterData.wstrSrcFile.rfind(L'\\');
		if(posSlash==std::wstring::npos)
		{
			posSlash=adapterData.wstrSrcFile.rfind(L'/');
		}

		if(posSlash==std::wstring::npos)
			adapterData.wstrBaseFileName=adapterData.wstrSrcFile;
		else
			adapterData.wstrBaseFileName=adapterData.wstrSrcFile.substr(posSlash+1);

		adapterData.wstrDstFolder=adapterData.wstrSrcFile.substr(0,posSlash);
		WCHAR wzTempPath[MAX_PATH]=L"";
		//::SHGetSpecialFolderPath(NULL,wzTempPath,CSIDL_INTERNET_CACHE,FALSE);
		if(::SHGetSpecialFolderPath(NULL,wzTempPath,CSIDL_PERSONAL,FALSE)==TRUE)
		{
			adapterData.wstrDstFolder=wzTempPath;
		}
		
		adapterData.encryptContext.bSymm=TRUE;

		std::wstring strPasswd;
		if(wzPasswd==NULL)
		{
			CPasswdGenerator passGen;
			strPasswd=passGen.Generator();
		}
		else
			strPasswd=wzPasswd;
		adapterData.encryptContext.wstrPassword=strPasswd;
		if(m_fpEncrypt)
		{
			EA_Error ec=m_fpEncrypt(&adapterData);
			if(ec)
			{
				if(adapterData.wstrErrorInfo.length())
				{
					DP((L"Zip Encrypt error:%s",adapterData.wstrErrorInfo.c_str()));
				}
				else
				{
					std::wstring strErr=L"Encrypt error. The error number from Zip_Adapter is "+Int2String(ec);
					DP((L"%s",strErr.c_str()));
				}
				return S_FALSE;
			}
			else
			{
				wcsncpy_s(wzRetFileName,MAX_PATH*2,adapterData.wstrActualDstFile.c_str(), _TRUNCATE);
			}
		}
		else
		{
			return S_FALSE;
		}

		return S_OK;
	};
private:
	std::wstring Int2String(int i)
	{
		std::wstring strOut;
		WCHAR wzTemp[32]=L"";
		_snwprintf_s(wzTemp,32, _TRUNCATE,L"%d",i);
		strOut=wzTemp;
		return strOut;
	}
	static HMODULE	m_hZipAdapterModule;
	static FTEncrypt		m_fpEncrypt;
};



#endif //__ZIPENCRYPTOR_H__
