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
	//���Ǽ��COPY, e.g. extract page OR split document are all COPY actions,
	//�ж�extract page--acrobat x��extract page
	//��ΪĿǰ�����������ж�������extract page������extract page�����֣�һ����extract֮���ļ�ֱ�ӱ����浽ָ�����ļ�����
	//��һ����extract֮����ļ�������ʱ�ļ������ұ��򿪣��û�����save as��֮��ָ�����ļ����¡�
	//������Ȼ�����ж�ʲô��extract page���������extract����COPY�Ļ���ȴ���й��ɣ�
	//�Ҽ��裨��д�Ĵ�������������裩--���е�myPDDocInsertPages�������Ǵ�һ������ʱ��pdf��insert��һ����ʱ���ļ���--��ϵͳ��ʱĿ¼������.tmp��ʽ��β����֮�󶼻���didsave�����ʱ�ļ���β����������
	//���ǣ�������myPDDocInsertPages֮������myAVDocOpenFromPDDocWithParams�����ʱ��pdf tmp�ļ���β������
	//�����ҵĲ��ԣ��ҷ���split document��ȷҲ�Ǿ������������߼���������
	//��������߼��ĵ�����������������������������������������Ϊdidsave�ﲻ����deny��������asfileopen���������didsave����
	CPolicy* ins_policy=CPolicy::GetInstance();
	vector<pair<wstring,wstring>> dest_tags;
	bool bDest_Encrypt=false;


	//dest��������ʱ��.tmp�ļ�
	if(m_dest.length()==0 || false==istempfile(m_dest.c_str()) || false==boost::algorithm::iends_with(m_dest,".tmp"))
	{
		return;
	}

	//src�����Ƿ���ʱ��.pdf�ļ�
	if (m_src.length()==0 || true==istempfile(m_src.c_str()) || false==boost::algorithm::iends_with(m_src,".pdf"))
	{
		return;
	}

	//asopen_file�����Ƿ���ʱ��pdf�ļ�
	if (true==istempfile(asopen_file.c_str()) || false==boost::algorithm::iends_with(asopen_file,".pdf"))
	{
		return;
	}

	//��������������֮��
	//this is COPY, from m_src to asopen_file
	CELOG_LOG(CELOG_DEBUG, L"CPDDocInsertPages::execute_asopenfile satisfy condition\n");

	ins_policy->QueryCopy_Get_Obligation_Inheritance(m_src.c_str(),asopen_file.c_str(),bdeny,dest_tags,bDest_Encrypt);

	if (true==bdeny)
	{
		return;
	}

	//û�б�deny��Ҫ��tag����encryption�����ʱ��dest�ļ���û�б�����
	//������encryption�����ܴ�tag
	if (bDest_Encrypt)
	{
		wstring wasopen_file = MyMultipleByteToWideChar(asopen_file);
		CEncrypt::Encrypt(wasopen_file,true);
	}

	//��didsave���ﻹ������tag����Ϊ��ʱ��û��asfileclose�����Թ��ˣ���ʱ���tag���ϵģ�ֻ����asfileclose�ĵط���tag���У�������reset
	m_tags=dest_tags;

	m_finaldest=asopen_file;
}

void CPDDocInsertPages::execute_asfileclose(const string& file)
{
	//֮ǰʱ���Ǵ��ϵġ���Ϊ��ʱ�����ˡ�
	//�����ҿ�����Ҫ��asfileclose�������а�



	if (file!=m_finaldest)
	{
		return;
	}

	//����Ҫ��tag
	if (!m_tags.size())
	{
		return;
	}
	
	CELOG_LOG(CELOG_DEBUG, L"CPDDocInsertPages::execute_asfileclose satisfy condition\n");

	//��������������֮��
	//this is COPY, from m_src to file
	//������asopenfile����Ҳ�������жϵģ�������д�����ΨһĿ���Ǵ�tag

	CTag* ins_tag=CTag::GetInstance();
	wstring path = MyMultipleByteToWideChar(file);
	ins_tag->add_tag_using_resattrmgr(path,m_tags);

	//����֮��reset
	reset();
}
void CPDDocInsertPages::execute_AVDocOpenFromPDDocWithParams(const string& file)
{
	//dest��������ʱ��.tmp�ļ�
	if(m_dest.length()==0 || false==istempfile(m_dest.c_str()) || false==boost::algorithm::iends_with(m_dest,".tmp"))
	{
		return;
	}

	//src�����Ƿ���ʱ��.pdf�ļ�
	if (m_src.length()==0 || true==istempfile(m_src.c_str()) || false==boost::algorithm::iends_with(m_src,".pdf"))
	{
		return;
	}

	//�������������֮��
	//������ڱ��򿪵�file����m_dest���Ǿ�˵��m_dest�����ʱ�ļ��Ǹ�pdf�ļ������±������ģ������Ѿ����򿪸��û��ˣ��û�������save���ͻ�����save as�ĶԻ���
	//����Ҫ��һЩ����������Ҫ�����������ǻ���֪������֪��Ҫ��Ҫ��COPY��evaluation���Լ���ô��������ȴ�PM�ľ�����
	//��������ʲôҲ�����ȣ���
	CELOG_LOG(CELOG_DEBUG, L"CPDDocInsertPages::execute_AVDocOpenFromPDDocWithParams satisfy condition, but we do nothing for now\n");

	//������Ҫ�������飬reset
	reset();
}
