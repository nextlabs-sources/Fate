#pragma once

#include <windows.h>
#include <string>
#include <vector>
using namespace std;

class CInheritance
{
public:
	CInheritance(void);
	virtual ~CInheritance(void);

	//����Ҫ��inheritance������Ҫ�Ȱ�source��tag��dest tag obligation�ϲ�
	//���������ͬ����tag�����ߵ�ֵҪ ��ǰ�ߵ�ֵ���ǵ�
	static void mergeSourceTagToDestObligationTags(_In_ const vector<pair<wstring,wstring>>& wtags_src, 
													_Inout_ vector<pair<wstring,wstring>>& destObligationTags)
	{
		for (DWORD i=0;i<wtags_src.size();i++)
		{
			//�Ƚ�source�ļ������tag��������destObligationTags������û��
			bool bFound=false;
			for (DWORD j=0;j<destObligationTags.size();j++)
			{
				if (wtags_src[i].first==destObligationTags[j].first)
				{
					bFound=true;
					break;
				}
			}
			if (true==bFound)
			{
				//source�ļ������tag��������destObligationTags���ڣ�����Ҫʹ�õ���destObligationTags�����tag��ֵ���������ǲ����κ����飬
				//ֱ�ӿ�source�ļ�����һ��tag
				continue;
			}
			//source�ļ������tag��������destObligationTags�����ڣ������tag�ϲ���destObligationTags����
			destObligationTags.push_back(wtags_src[i]);
		}
	}

	//����Ҫ��inheritance��
	//bDestObligationEncrypted��ʾpolicy����Ҫ��Ҫ���dest�ļ���encryption obligation�����������������ʱ�����source�ļ��Ǽ��ܵģ��Ǽ�ʹ
	//bDestObligationEncrypted��������Ҫ����ܣ�bDestObligationEncrypted���Ҳ���Ҫ����--��Ϊ����Ҫ��inheritance
	static void mergeSourceEncryptionToDest(_In_ const bool bSrcEncrypted, _Out_ bool& bDestObligationEncrypted)
	{
		bDestObligationEncrypted |= bSrcEncrypted;
	}
};
