#include "..\include\Edit.h"
#include "Encrypt.h"
#pragma warning(push)
#pragma warning(disable:6334 6011 4996)
#include "boost\unordered_map.hpp"
#include "boost\shared_ptr.hpp"
#include "boost\weak_ptr.hpp"
#include <boost/algorithm/string.hpp>
#pragma warning(pop)


CEdit::CEdit(void)
{
	InitializeCriticalSection(&m_cs);
}

CEdit::~CEdit(void)
{
	DeleteCriticalSection(&m_cs);
}

void CEdit::do_tag_obligation(const PDDoc doc,const string& file, const vector<pair<wstring,wstring>>& tags)
{
	
	if (!file.length() || !tags.size())
	{
		return;
	}

	//�����acrobat�����Ҳ���SE�ļ�������native SDK�ķ�����tag
	//��Ȼ����RESATTRMGR����Ҫ�ȵ��ļ����رգ����ұ����Ǳ��صĻ���UNC��������ǾͲ���tag��
	wstring wfile(file.begin(),file.end());
	
#if ACRO_SDK_LEVEL==0x000A0000	//for acrobat
	bool bSrcEncrypted= (emIsEncrytFile==CEncrypt::Encrypt(wfile,false,true)) ? true:false;
	if (false==bSrcEncrypted)
	{
		//��������ֱ����native SDK�ķ�����tag
		CTag* ins_tag=CTag::GetInstance();
		ins_tag->add_tag_using_native_sdk(tags,doc);
	}
	else
	{
		CELOG_LOG(CELOG_DEBUG, L"AcrobatPEP will tag SE file using RESATTRMGR\n");

		//�����ļ��ǲ��Ǳ��ص��ļ�����UNC���ļ�
		if (false==IsURLPath(file))
		{
			//�Ǳ��ػ���UNC���ļ����������ļ����رյ�ʱ����RESATTRMGR��tag
			//����Ҫ���ľ��ǰ��ļ���tag�ȱ�������
			EnterCriticalSection(&m_cs);
			m_file_tags[wfile]=tags;
			LeaveCriticalSection(&m_cs);
		}
		else
		{
			//���ܴ�tag��ʲôҲ����
			CELOG_LOG(CELOG_DEBUG, L"the file path is URL, we can't tag it\n");
		}
	}
#else
	//�����ļ��ǲ��Ǳ��ص��ļ�����UNC���ļ�
	if (false==IsURLPath(file))
	{
		//�Ǳ��ػ���UNC���ļ����������ļ����رյ�ʱ����RESATTRMGR��tag
		//����Ҫ���ľ��ǰ��ļ���tag�ȱ�������
		EnterCriticalSection(&m_cs);
		m_file_tags[wfile]=tags;
		LeaveCriticalSection(&m_cs);
		CELOG_LOG(CELOG_DEBUG, L"we save tag for CEdit\n");
	}
	else
	{
		//���ܴ�tag��ʲôҲ����
	}
#endif
	
}

void CEdit::do_se_obligation(const string& file)
{

	//Ҫ�ȵ��ļ����رգ�������SE����Ϊ������EDIT�������ļ��϶��Ǳ��򿪵�

	//ֻ�Ա����ļ�SE,UNC��URL�ļ����޷�SE��ֱ�ӷ���
	
	//�����ļ��ǲ��Ǳ��ص��ļ�����UNC���ļ�
	std::wstring wfile=MyMultipleByteToWideChar(file);
	if (false==IsLocalPath(wfile.c_str()))
	{
		//��URL����UNC���ļ�������SE
		//ֱ�ӷ���
		return;
	}

	//�Ǳ��ص��ļ�
	wstring unused;
	EnterCriticalSection(&m_cs);
	m_file_se[wfile]=unused;
	LeaveCriticalSection(&m_cs);
}

//����PRD������Ҫ���ļ����ر��Ժ���SE
void CEdit::execute_asfileclose(const string& file)
{
	if (!file.length())
	{
		return;
	}

	EnterCriticalSection(&m_cs);


	wstring wfile(file.begin(),file.end());

	CELOG_LOG(CELOG_DEBUG, L"in CEdit::execute_asfileclose\n");

	//Ϊreader��edit�����ӵĴ���
	//reader��edit��interactive tagging�Ի���ҲҪ���ļ����ص���ʱ����
	//Ҳ������������
	vector<pair<wstring,wstring>> srcTag;
	map<wstring,std::pair<nextlabs::Obligations, bool>>::iterator itr_obs=m_file_obs.find(wfile);
	if (m_file_obs.end()!=itr_obs)
	{

		CELOG_LOG(CELOG_DEBUG, L"we get obs\n");

#if ACRO_SDK_LEVEL==0x000A0000	//hooks only for acrobat
		//�����acrobat��ֻҪ�Ƴ��Ϳ����ˣ�����reader��acrobat����obsֻ��Ϊ�˲���δ�tag
		//reader��Ϊ����close��ʱ���tag��acrobatֻ��Ϊ�˱���obs��ֹ��δ�tag����ʹ��SE�ļ���acrobatҲҪ�������tag
		//��others����Ĵ������������������
		m_file_obs.erase(itr_obs);
#else

		//����ļ���obs
		//����obs
		if (itr_obs->second.second)
		{
			vector<pair<wstring,wstring>> dstTag;
			CObMgr::GetObTags(itr_obs->second.first, srcTag,dstTag, wfile.c_str(), NULL);
		}
		

		//��map���Ƴ�
		m_file_obs.erase(itr_obs);

		//���Դ�tag��
		//Ҫ���tag��srcTag����
		CTag* ins_tag=CTag::GetInstance();
		ins_tag->add_tag_using_resattrmgr(wfile,srcTag);
#endif
	}


	////////////////////////////////////////////////////////
	bool bTag=false;
	vector<pair<wstring,wstring>> tags;//��Ҫ���tag
	map<wstring,vector<pair<wstring,wstring>>>::iterator itr_tag=m_file_tags.find(wfile);
	if (m_file_tags.end()!=itr_tag)
	{
		CELOG_LOG(CELOG_DEBUG, L"we tag EDIT file here\n");

		//����ļ���Ҫ��tag
		bTag=true;
		tags=itr_tag->second;

		//��map���Ƴ�
		m_file_tags.erase(itr_tag);
	}

	bool bSE=false;
	map<wstring,wstring>::iterator itr_se=m_file_se.find(wfile);
	if (m_file_se.end()!=itr_se)
	{
		CELOG_LOG(CELOG_DEBUG, L"we SE EDIT file here\n");

		//����ļ���ҪSE
		bSE=true;
		m_file_se.erase(itr_se);
	}
	
	//�������������ʱ���ļ�HANDLE�Ѿ���close��
	//��ʱ����resattrmgr���ļ���tag����SE���ļ���SE

	if (true==bTag)
	{
		CTag* ins_tag=CTag::GetInstance();
		ins_tag->add_tag_using_resattrmgr(wfile,tags);
	}

	if (true==bSE)
	{
		CEncrypt::Encrypt(wfile,false);

		//need to sync
		TAGS oldtags;
		CTag::GetInstance()->get_cachednativetag(wfile, oldtags);
		CTag::GetInstance()->add_tag_using_resattrmgr(wfile, oldtags);
		CTag::GetInstance()->add_tag_using_resattrmgr(wfile, tags);//synce newly created tags
		CTag::GetInstance()->add_tag_using_resattrmgr(wfile, srcTag);
		CELOG_LOG(CELOG_DEBUG, L"sync tags for EDIT, path: %s, existing tag count: %d, newly created tags: %d, srcTags count: %d\n", wfile.c_str(), oldtags.size(), tags.size(), srcTag.size());
	}

	LeaveCriticalSection(&m_cs);

	CELOG_LOG(CELOG_DEBUG, L"out CEdit::execute_asfileclose\n");

	return;
}


//Ϊreader���ӵģ�
//reader��edit��interactive tagging�Ի���ҲҪ���ļ����ص���ʱ����
//��������Ҫ����obligation
//Ȼ�����ļ��رյ�ʱ�����obligation������obligationҲ������interactive tagging�Ի�����Ϊ���obligation����interactive tagging���ͻ�����
void CEdit::save_obligaiton(const string& file,const nextlabs::Obligations& obs)
{
	

	//�����ļ��ǲ��Ǳ��ص��ļ�����UNC���ļ�
	std::wstring wfile=MyMultipleByteToWideChar(file);
	if (false==IsURLPath(file))
	{
		//�Ǳ��ػ���UNC���ļ����������ļ����رյ�ʱ����RESATTRMGR��tag
		//����Ҫ���ľ��ǰ��ļ���obs�ȱ�������
		CELOG_LOG(CELOG_DEBUG, L"we save obs for %s\n",wfile.c_str());
		EnterCriticalSection(&m_cs);
		if (m_file_obs.end() == m_file_obs.find(wfile))
		{
			m_file_obs[wfile]=std::make_pair(obs, true);
		}
		LeaveCriticalSection(&m_cs);
	}
	else
	{
		//���ܴ�tag����Ϊ���ܴ�tag������obsҲû��Ҫ�����ˣ�ʲôҲ����
	}

}

void CEdit::invalid_obligaiton(const string& file)
{
	std::wstring wfile=MyMultipleByteToWideChar(file);

	EnterCriticalSection(&m_cs);

	map<wstring,std::pair<nextlabs::Obligations, bool>>::iterator itr_obs=m_file_obs.find(wfile);
	if (m_file_obs.end()!=itr_obs)
	{
		itr_obs->second.second = false;
		CELOG_LOG(CELOG_DEBUG, L"invalid obs for %s\n",wfile.c_str());
	}
	else
	{
		const nextlabs::Obligations obs;
		m_file_obs[wfile]=std::make_pair(obs, false);
	}

	LeaveCriticalSection(&m_cs);
}

bool CEdit::isinvalid_obligaiton(const string& file)
{
	bool bisinvalid=false;

	std::wstring wfile=MyMultipleByteToWideChar(file);

	EnterCriticalSection(&m_cs);

	map<wstring,std::pair<nextlabs::Obligations, bool>>::iterator itr_obs=m_file_obs.find(wfile);
	if (m_file_obs.end()!=itr_obs)
	{
		if (!itr_obs->second.second)
		{
			bisinvalid = true;
		}
	}

	LeaveCriticalSection(&m_cs);
	
	return bisinvalid;
}

bool CEdit::get_obligation(const string& file)
{
	bool bfind=false;

	EnterCriticalSection(&m_cs);

	std::wstring wfile=MyMultipleByteToWideChar(file);
	map<wstring,std::pair<nextlabs::Obligations, bool>>::iterator itr_obs=m_file_obs.find(wfile);
	if (m_file_obs.end()!=itr_obs)
	{
		if (itr_obs->second.second)
		{
			bfind=true;
			CELOG_LOG(CELOG_DEBUG, L"we find obs for %s\n",wfile.c_str());
		}
	}

	LeaveCriticalSection(&m_cs);

	return bfind;
}
