#pragma once
#include <string>
#include <vector>
using namespace std;

#include "nltag.h"
#include "MenuItem.h"
#include "Encrypt.h"
#include "celog.h"

#pragma warning(push)
#pragma warning(disable:6334 6011 4996 4189 4100 4819)
#include "boost\format.hpp"
#pragma warning(pop)

#define CELOG_CUR_MODULE L"ADOBEPEP"
#undef CELOG_CUR_FILE
#define CELOG_CUR_FILE   CELOG_FILEPATH_PROD_PEP_ENDPOINT_ADOBEPEP_PLUGIN_SAMPLESADOBEPEP_INCLUDE_SAVEASOBLIGATION_H

class CDoTag_SaveAs
{
public:

	static CDoTag_SaveAs* GetInstance()
	{
		static CDoTag_SaveAs ins;
		return &ins;
	}


	void set_closed_flag()
	{
		//������Ҫ����tag���ļ����Ѿ���close��
		m_b_closed=true;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//����Ҫ��tag�ı�־��ͬʱ����tag
	void setFlagAndObligationTags(const vector<pair<wstring,wstring>>& tags);
	//�ж�Ҫ��Ҫ��tag
	bool getFlag();




	


	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//������ʱ���ܴ�tag�������е�COPY��ʱ����Ϊ��ʱ�ļ���handle��adobe own�ˣ�����ֻ�ܵȵ�adobe release�����handle��ʱ����ܴ�tag
	//���������ṩ����ӿڣ����û��������ļ���handle���Ժ��û�����ȡ���handle������ƥ�䷢�ָ�handle�Ƿ�adobe release��
	void setFileHandle(ASFile handle);

	//ȡfile handle
	ASFile getFileHandle();

	//ȡ�ļ�·��
	const string& GetFilePath();


	//�����ļ�·��
	void SetFilePath(const string& file_path);

	//��tag lib��tag
	void DoTag_TagLib();

	void SetType(const string& strType)
	{
		CELOG_LOGA(CELOG_DEBUG, "CDoTag_SaveAs::SetType: %s\n", strType.c_str());
		m_strType=strType;
	}

	void execute_pddocdidsave(PDDoc doc);


	void execute_avappsavedialog(DWORD dwPageNum);
	
	void execute_avappendsave();

	void SetSEFlag()
	{
		m_SE_Flag=true;
	}

	//��tag lib����tag�������溯����֮ͬ�������ṩ��һ�����������Ҳ�������
	void DoTag_TagLib_2(const string& strPath);

	//�������еĳ�Ա
	virtual void reset();
	
protected://������Է���
	CDoTag_SaveAs::CDoTag_SaveAs()
	{
		reset();
	}
	CDoTag_SaveAs::~CDoTag_SaveAs()
	{

	}
	


private://ֻ���Լ����ܷ���





	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////


	//���Ѿ����˵�pdf��tag��acrobat��reader��ʵ�ֲ�ͬ
	void DoTag_On_Opened_PDF(PDDoc doc);



	//�����ļ�·��--��һ����--����jpg���Ƶ�ר��
	void AddFiles_WithPostFix(const string& file_path)
	{
		CELOG_LOGA(CELOG_DEBUG, "CDoTagBase::AddFilePath_with_PostFix,%s\n", file_path.c_str());


		m_files_with_postfix.push_back(file_path);
	}
	void DoTag_On_Files_WithPostFix()
	{
		CELOG_LOG(CELOG_DEBUG, L"CDoTagBase::DoTag_On_FilesWithPostFix\n");

		for (DWORD i=0;i<m_files_with_postfix.size();i++)
		{
			DoTag_TagLib_2(m_files_with_postfix[i]);
		}
		reset();
	}


	void tag_opened_pdf_with_resattrmgr(PDDoc doc);

	//Ҫ��Ҫ��tag�ı�־
	bool m_bNeedDoTag;
	//Ҫ���tag
	vector<pair<wstring,wstring>> m_tags;

	ASFile m_handle;//file handle of file to be tagged
	string m_file;//file path of file to be tagged

	vector<string> m_files_with_postfix;

	
	string m_strType;

	bool m_SE_Flag;

	bool m_b_closed;
};

