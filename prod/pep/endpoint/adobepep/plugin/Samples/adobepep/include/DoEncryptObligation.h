#pragma once
#include <string>
#include <vector>
using namespace std;

#include "celog.h"

#define CELOG_CUR_MODULE L"ADOBEPEP"
#undef CELOG_CUR_FILE
#define CELOG_CUR_FILE   CELOG_FILEPATH_PROD_PEP_ENDPOINT_ADOBEPEP_PLUGIN_SAMPLESADOBEPEP_INCLUDE_DOENCRYPTOBLIGATION_H

class CDoEncryObligation
{
private:
	static vector<wstring> s_files;
public:
	static void DoPostEncrypt()
	{
		for (DWORD i=0;i<s_files.size();i++)
		{
			CELOG_LOG(CELOG_DEBUG, L"CDoEncryObligation::DoPostEncrypt, for multiple files\n");
			CEncrypt::Encrypt(s_files[i],false);
		}

		s_files.clear();
	}
	static void DoMarkEncrypt(DWORD dwPageNum, const wstring& wstrDest)
	{
		//现在就可以做encryption obligation了，因为这个时候copy的dest file还没有生成，我们可以mark它为encryption
		
		
		//jpg例外，jpg的最终文件名是加了后缀的，是每个页面一个jpg文件，
		//不光是文件名的问题，而且就算是jpg文件被生成以后，他还会被覆盖，所以用mark as encryption的方式根本行不通-- 即使mark成功了，但他会被覆盖掉，最后又不是encryption的状态了
		//所以只能事后做encryption，这里只能记录下来文件名字

		wstring::size_type pos=wstrDest.rfind(L'.');
		if (wstring::npos!=pos)
		{
			wstring postfix=wstrDest.substr(pos,wstrDest.length()-pos);
			wstring wstrName=wstrDest.substr(0,pos);

			string strPostfix(postfix.begin(),postfix.end());
			if (IsMulti_BasedOnPages_Format(strPostfix))
			{
				CELOG_LOG(CELOG_DEBUG, L"CDoEncryObligation::DoMarkEncrypt, for multiple files, store it first\n");


				for (DWORD i=1;i<=dwPageNum;i++)
				{
					wchar_t temp[32]={0};

					if (".eps"==strPostfix)
					{
						_snwprintf_s(temp,32, _TRUNCATE, L"_%d",i);
					}
					else
					{
						_snwprintf_s(temp,32, _TRUNCATE, L"_Page_%d",i);	
					}
					
					wstring wstrFullPath=wstrName+temp;
					wstrFullPath+=postfix;
					
					s_files.push_back(wstrFullPath);
				}

				return;
			}
		}



		//被生产的文件是单个文件
		CELOG_LOG(CELOG_DEBUG, L"CDoEncryObligation::DoMarkEncrypt, for single file\n");

		CEncrypt::Encrypt(wstrDest,true);
	}
};

vector<wstring> CDoEncryObligation::s_files;