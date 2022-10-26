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

	//为reader增加的，
	//reader的edit的interactive tagging对话框也要在文件被关掉的时候做
	//所以我们要保存obligation
	//然后在文件关闭的时候解析obligation，解析obligation也就是跳interactive tagging对话框，因为如果obligation里有interactive tagging，就会跳框
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

	map<wstring,wstring> m_file_se;// need to SE these files when file is closed，这里的第二个wstring其实是用不到的，用map的原因是因为查找效率高

	//为reader增加的，
	//reader的edit的interactive tagging对话框也要在文件被关掉的时候做
	//所以我们要保存obligation
	//然后在文件关闭的时候解析obligation，解析obligation也就是跳interactive tagging对话框，因为如果obligation里有interactive tagging，就会跳框
	map<wstring,std::pair<nextlabs::Obligations, bool>> m_file_obs;

	CRITICAL_SECTION m_cs;
};
