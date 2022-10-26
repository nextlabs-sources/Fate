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
		//���ھͿ�����encryption obligation�ˣ���Ϊ���ʱ��copy��dest file��û�����ɣ����ǿ���mark��Ϊencryption
		
		
		//jpg���⣬jpg�������ļ����Ǽ��˺�׺�ģ���ÿ��ҳ��һ��jpg�ļ���
		//�������ļ��������⣬���Ҿ�����jpg�ļ��������Ժ������ᱻ���ǣ�������mark as encryption�ķ�ʽ�����в�ͨ-- ��ʹmark�ɹ��ˣ������ᱻ���ǵ�������ֲ���encryption��״̬��
		//����ֻ���º���encryption������ֻ�ܼ�¼�����ļ�����

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



		//���������ļ��ǵ����ļ�
		CELOG_LOG(CELOG_DEBUG, L"CDoEncryObligation::DoMarkEncrypt, for single file\n");

		CEncrypt::Encrypt(wstrDest,true);
	}
};

vector<wstring> CDoEncryObligation::s_files;