#pragma once
#include <string>
#include <vector>
#include <map>
#include "utilities.h"
#include "nltag.h"
#include "obMgr.h"
#include "comm_helper.hpp"


using namespace std;
class CEdit
{
public:
	static CEdit* GetInstance()
	{
		static CEdit ins;
		ins.init();
		return &ins;
	}
	void do_tag_obligation(const PDDoc doc,const string& file, const vector<pair<wstring,wstring>>& tags);
	void do_se_obligation(const string& file);
	void execute_asfileclose(const string& file);

	//Ϊreader���ӵģ�
	//reader��edit��interactive tagging�Ի���ҲҪ���ļ����ص���ʱ����
	//��������Ҫ����obligation
	//Ȼ�����ļ��رյ�ʱ�����obligation������obligationҲ������interactive tagging�Ի�����Ϊ���obligation����interactive tagging���ͻ�����
	void save_obligaiton(const string& file,const nextlabs::Obligations& obs);

	void invalid_obligaiton(const string& file);
	
	bool isinvalid_obligaiton(const string& file);

	bool get_obligation(const string& file);
private:
	void init()
	{

	}
	CEdit(void);
	~CEdit(void);

private:
	map<wstring,vector<pair<wstring,wstring>>> m_file_tags;	//need to add these tags to these files when file is closed

	map<wstring,wstring> m_file_se;// need to SE these files when file is closed������ĵڶ���wstring��ʵ���ò����ģ���map��ԭ������Ϊ����Ч�ʸ�

	//Ϊreader���ӵģ�
	//reader��edit��interactive tagging�Ի���ҲҪ���ļ����ص���ʱ����
	//��������Ҫ����obligation
	//Ȼ�����ļ��رյ�ʱ�����obligation������obligationҲ������interactive tagging�Ի�����Ϊ���obligation����interactive tagging���ͻ�����
	map<wstring,std::pair<nextlabs::Obligations, bool>> m_file_obs;

	CRITICAL_SECTION m_cs;
};
