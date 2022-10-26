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

	//我们要做inheritance，所以要先把source的tag和dest tag obligation合并
	//后者如果有同名的tag，后者的值要 把前者的值覆盖掉
	static void mergeSourceTagToDestObligationTags(_In_ const vector<pair<wstring,wstring>>& wtags_src, 
													_Inout_ vector<pair<wstring,wstring>>& destObligationTags)
	{
		for (DWORD i=0;i<wtags_src.size();i++)
		{
			//比较source文件的这个tag的名字在destObligationTags里面有没有
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
				//source文件的这个tag的名字在destObligationTags存在，我们要使用的是destObligationTags的这个tag的值，所以我们不做任何事情，
				//直接看source文件的下一个tag
				continue;
			}
			//source文件的这个tag的名字在destObligationTags不存在，把这个tag合并到destObligationTags里面
			destObligationTags.push_back(wtags_src[i]);
		}
	}

	//我们要做inheritance，
	//bDestObligationEncrypted表示policy里面要不要求对dest文件做encryption obligation，在这个函数结束的时候，如果source文件是加密的，那即使
	//bDestObligationEncrypted本来不是要求加密，bDestObligationEncrypted最后也变成要加密--因为我们要做inheritance
	static void mergeSourceEncryptionToDest(_In_ const bool bSrcEncrypted, _Out_ bool& bDestObligationEncrypted)
	{
		bDestObligationEncrypted |= bSrcEncrypted;
	}
};
