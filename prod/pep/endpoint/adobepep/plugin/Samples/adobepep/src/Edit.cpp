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

	//如果是acrobat，并且不是SE文件，就用native SDK的方法打tag
	//不然就用RESATTRMGR，这要等到文件被关闭，并且必须是本地的或者UNC，如果不是就不打tag了
	wstring wfile(file.begin(),file.end());
	
#if ACRO_SDK_LEVEL==0x000A0000	//for acrobat
	bool bSrcEncrypted= (emIsEncrytFile==CEncrypt::Encrypt(wfile,false,true)) ? true:false;
	if (false==bSrcEncrypted)
	{
		//可以现在直接用native SDK的方法打tag
		CTag* ins_tag=CTag::GetInstance();
		ins_tag->add_tag_using_native_sdk(tags,doc);
	}
	else
	{
		CELOG_LOG(CELOG_DEBUG, L"AcrobatPEP will tag SE file using RESATTRMGR\n");

		//看看文件是不是本地的文件或者UNC的文件
		if (false==IsURLPath(file))
		{
			//是本地或者UNC的文件，可以在文件被关闭的时候用RESATTRMGR打tag
			//现在要做的就是把文件和tag先保存起来
			EnterCriticalSection(&m_cs);
			m_file_tags[wfile]=tags;
			LeaveCriticalSection(&m_cs);
		}
		else
		{
			//不能打tag，什么也不做
			CELOG_LOG(CELOG_DEBUG, L"the file path is URL, we can't tag it\n");
		}
	}
#else
	//看看文件是不是本地的文件或者UNC的文件
	if (false==IsURLPath(file))
	{
		//是本地或者UNC的文件，可以在文件被关闭的时候用RESATTRMGR打tag
		//现在要做的就是把文件和tag先保存起来
		EnterCriticalSection(&m_cs);
		m_file_tags[wfile]=tags;
		LeaveCriticalSection(&m_cs);
		CELOG_LOG(CELOG_DEBUG, L"we save tag for CEdit\n");
	}
	else
	{
		//不能打tag，什么也不做
	}
#endif
	
}

void CEdit::do_se_obligation(const string& file)
{

	//要等到文件被关闭，才能做SE，因为现在是EDIT，所以文件肯定是被打开的

	//只对本地文件SE,UNC和URL文件都无法SE，直接返回
	
	//看看文件是不是本地的文件或者UNC的文件
	std::wstring wfile=MyMultipleByteToWideChar(file);
	if (false==IsLocalPath(wfile.c_str()))
	{
		//是URL或者UNC的文件，不能SE
		//直接返回
		return;
	}

	//是本地的文件
	wstring unused;
	EnterCriticalSection(&m_cs);
	m_file_se[wfile]=unused;
	LeaveCriticalSection(&m_cs);
}

//根据PRD，我们要等文件被关闭以后做SE
void CEdit::execute_asfileclose(const string& file)
{
	if (!file.length())
	{
		return;
	}

	EnterCriticalSection(&m_cs);


	wstring wfile(file.begin(),file.end());

	CELOG_LOG(CELOG_DEBUG, L"in CEdit::execute_asfileclose\n");

	//为reader的edit新增加的代码
	//reader的edit的interactive tagging对话框也要在文件被关掉的时候做
	//也就是在这里做
	vector<pair<wstring,wstring>> srcTag;
	map<wstring,std::pair<nextlabs::Obligations, bool>>::iterator itr_obs=m_file_obs.find(wfile);
	if (m_file_obs.end()!=itr_obs)
	{

		CELOG_LOG(CELOG_DEBUG, L"we get obs\n");

#if ACRO_SDK_LEVEL==0x000A0000	//hooks only for acrobat
		//如果是acrobat，只要移除就可以了，不像reader，acrobat保存obs只是为了不多次打tag
		//reader是为了在close的时候打tag，acrobat只是为了保存obs防止多次打tag，即使像SE文件，acrobat也要在这里打tag
		//但others下面的代码能完成这样的任务
		m_file_obs.erase(itr_obs);
#else

		//这个文件有obs
		//解析obs
		if (itr_obs->second.second)
		{
			vector<pair<wstring,wstring>> dstTag;
			CObMgr::GetObTags(itr_obs->second.first, srcTag,dstTag, wfile.c_str(), NULL);
		}
		

		//从map里移除
		m_file_obs.erase(itr_obs);

		//可以打tag了
		//要打的tag在srcTag里面
		CTag* ins_tag=CTag::GetInstance();
		ins_tag->add_tag_using_resattrmgr(wfile,srcTag);
#endif
	}


	////////////////////////////////////////////////////////
	bool bTag=false;
	vector<pair<wstring,wstring>> tags;//需要打的tag
	map<wstring,vector<pair<wstring,wstring>>>::iterator itr_tag=m_file_tags.find(wfile);
	if (m_file_tags.end()!=itr_tag)
	{
		CELOG_LOG(CELOG_DEBUG, L"we tag EDIT file here\n");

		//这个文件需要打tag
		bTag=true;
		tags=itr_tag->second;

		//从map里移除
		m_file_tags.erase(itr_tag);
	}

	bool bSE=false;
	map<wstring,wstring>::iterator itr_se=m_file_se.find(wfile);
	if (m_file_se.end()!=itr_se)
	{
		CELOG_LOG(CELOG_DEBUG, L"we SE EDIT file here\n");

		//这个文件需要SE
		bSE=true;
		m_file_se.erase(itr_se);
	}
	
	//调用这个函数的时候，文件HANDLE已经被close了
	//这时候用resattrmgr对文件打tag，用SE对文件做SE

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


//为reader增加的，
//reader的edit的interactive tagging对话框也要在文件被关掉的时候做
//所以我们要保存obligation
//然后在文件关闭的时候解析obligation，解析obligation也就是跳interactive tagging对话框，因为如果obligation里有interactive tagging，就会跳框
void CEdit::save_obligaiton(const string& file,const nextlabs::Obligations& obs)
{
	

	//看看文件是不是本地的文件或者UNC的文件
	std::wstring wfile=MyMultipleByteToWideChar(file);
	if (false==IsURLPath(file))
	{
		//是本地或者UNC的文件，可以在文件被关闭的时候用RESATTRMGR打tag
		//现在要做的就是把文件和obs先保存起来
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
		//不能打tag，因为不能打tag，所以obs也没必要保存了，什么也不做
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
