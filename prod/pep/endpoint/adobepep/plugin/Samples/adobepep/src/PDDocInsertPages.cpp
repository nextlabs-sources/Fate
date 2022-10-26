#include <winsock2.h>
#include <windows.h>
#include "..\include\PDDocInsertPages.h"
#include "utilities.h"
#include "policy.h"
#include "Encrypt.h"

#pragma warning(push)
#pragma warning(disable:6334 6011 4996)
#include <boost/algorithm/string.hpp>
#pragma warning(pop)

#define CELOG_CUR_MODULE L"ADOBEPEP"
#undef CELOG_CUR_FILE
#define CELOG_CUR_FILE CELOG_FILEPATH_PROD_PEP_ENDPOINT_ADOBEPEP_PLUGIN_SAMPLES_ADOBEPEP_SRC_PDDOCINSERTPAGES_CPP

CPDDocInsertPages::CPDDocInsertPages(void)
{
}

CPDDocInsertPages::~CPDDocInsertPages(void)
{
}

void CPDDocInsertPages::set_src_dest(const string& src, const string& dest)
{
	m_src=src;
	m_dest=dest;
}

void CPDDocInsertPages::execute_asopenfile(const string& asopen_file,bool& bdeny)
{
	//这是检查COPY, e.g. extract page OR split document are all COPY actions,
	//判断extract page--acrobat x有extract page
	//因为目前来看，很难判断怎样是extract page，而且extract page有两种，一种是extract之后文件直接被保存到指定的文件夹下
	//另一种是extract之后的文件还是临时文件，并且被打开，用户可以save as到之后指定的文件夹下。
	//但是虽然不能判断什么是extract page，但如果把extract当成COPY的话，却很有规律：
	//我假设（我写的代码依赖这个假设）--所有的myPDDocInsertPages（并且是从一个非临时的pdf被insert到一个临时的文件里--在系统临时目录，且以.tmp格式结尾），之后都会有didsave这个临时文件结尾！！！！！
	//除非！！！在myPDDocInsertPages之后，是以myAVDocOpenFromPDDocWithParams这个临时的pdf tmp文件结尾！！！
	//经过我的测试，我发现split document的确也是经历的这样的逻辑！！！！
	//对上面的逻辑的调整！！！！！！！！！！！！！！！！！！因为didsave里不能做deny，所以在asfileopen里面代替在didsave里检查
	CPolicy* ins_policy=CPolicy::GetInstance();
	vector<pair<wstring,wstring>> dest_tags;
	bool bDest_Encrypt=false;


	//dest必须是临时的.tmp文件
	if(m_dest.length()==0 || false==istempfile(m_dest.c_str()) || false==boost::algorithm::iends_with(m_dest,".tmp"))
	{
		return;
	}

	//src必须是非临时的.pdf文件
	if (m_src.length()==0 || true==istempfile(m_src.c_str()) || false==boost::algorithm::iends_with(m_src,".pdf"))
	{
		return;
	}

	//asopen_file必须是非临时的pdf文件
	if (true==istempfile(asopen_file.c_str()) || false==boost::algorithm::iends_with(asopen_file,".pdf"))
	{
		return;
	}

	//满足了以上条件之后，
	//this is COPY, from m_src to asopen_file
	CELOG_LOG(CELOG_DEBUG, L"CPDDocInsertPages::execute_asopenfile satisfy condition\n");

	ins_policy->QueryCopy_Get_Obligation_Inheritance(m_src.c_str(),asopen_file.c_str(),bdeny,dest_tags,bDest_Encrypt);

	if (true==bdeny)
	{
		return;
	}

	//没有被deny，要打tag和做encryption。这个时候dest文件还没有被生成
	//可以做encryption，不能打tag
	if (bDest_Encrypt)
	{
		wstring wasopen_file = MyMultipleByteToWideChar(asopen_file);
		CEncrypt::Encrypt(wasopen_file,true);
	}

	//在didsave那里还不能做tag，因为那时候还没有asfileclose，我试过了，那时候打tag打不上的，只能在asfileclose的地方打tag才行，并且做reset
	m_tags=dest_tags;

	m_finaldest=asopen_file;
}

void CPDDocInsertPages::execute_asfileclose(const string& file)
{
	//之前时候还是打不上的。因为那时候被锁了。
	//所以我看还是要在asfileclose里面打才行啊



	if (file!=m_finaldest)
	{
		return;
	}

	//必须要有tag
	if (!m_tags.size())
	{
		return;
	}
	
	CELOG_LOG(CELOG_DEBUG, L"CPDDocInsertPages::execute_asfileclose satisfy condition\n");

	//满足了以上条件之后，
	//this is COPY, from m_src to file
	//我们在asopenfile那里也是这样判断的，在这里写代码的唯一目的是打tag

	CTag* ins_tag=CTag::GetInstance();
	wstring path = MyMultipleByteToWideChar(file);
	ins_tag->add_tag_using_resattrmgr(path,m_tags);

	//做完之后，reset
	reset();
}
void CPDDocInsertPages::execute_AVDocOpenFromPDDocWithParams(const string& file)
{
	//dest必须是临时的.tmp文件
	if(m_dest.length()==0 || false==istempfile(m_dest.c_str()) || false==boost::algorithm::iends_with(m_dest,".tmp"))
	{
		return;
	}

	//src必须是非临时的.pdf文件
	if (m_src.length()==0 || true==istempfile(m_src.c_str()) || false==boost::algorithm::iends_with(m_src,".pdf"))
	{
		return;
	}

	//满足上面的条件之后，
	//如果现在被打开的file就是m_dest，那就说明m_dest这个临时文件是个pdf文件，是新被创建的，而且已经被打开给用户了，用户如果点击save，就会跳出save as的对话框
	//我们要做一些处理，但现在要怎样处理，我们还不知道，不知道要不要做COPY的evaluation，以及怎么做，这个等待PM的决定。
	//反正现在什么也不做先！！
	CELOG_LOG(CELOG_DEBUG, L"CPDDocInsertPages::execute_AVDocOpenFromPDDocWithParams satisfy condition, but we do nothing for now\n");

	//做完了要做的事情，reset
	reset();
}
