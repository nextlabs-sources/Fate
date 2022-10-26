#pragma once

#include <string>
#include <vector>
using namespace std;

#include "celog.h"

#define CELOG_CUR_MODULE L"ADOBEPEP"
#undef CELOG_CUR_FILE
#define CELOG_CUR_FILE   CELOG_FILEPATH_PROD_PEP_ENDPOINT_ADOBEPEP_PLUGIN_SAMPLESADOBEPEP_INCLUDE_PDDOCLNSERTPAGES_H


class CPDDocInsertPages
{
private:
	CPDDocInsertPages(void);
	~CPDDocInsertPages(void);
	void reset()
	{
		CELOG_LOG(CELOG_DEBUG, L"CPDDocInsertPages::reset\n");
		m_src.clear();
		m_dest.clear();
		m_finaldest.clear();
		m_tags.clear();
	}
public:
	static CPDDocInsertPages* GetInstance()
	{
		static CPDDocInsertPages ins;
		return &ins;
	}
	void set_src_dest(const string& src, const string& dest);
	void execute_asopenfile(const string& asopen_file,bool& bdeny);
	void execute_AVDocOpenFromPDDocWithParams(const string& file);
	void execute_asfileclose(const string& file);
private:
	string m_src;
	string m_dest;
	string m_finaldest;
	vector<pair<wstring,wstring>> m_tags;
};
