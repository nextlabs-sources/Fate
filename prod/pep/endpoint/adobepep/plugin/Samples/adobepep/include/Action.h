#pragma once

#include <string>
#include <vector>
using namespace std;


#define INSERT_PAGES "insert_pages"
#define ACTION_TBD "to_be_determine"




class CAction
{
private:
	CAction(void)
	{

	}
	virtual ~CAction(void)
	{

	}

public:
	static CAction* GetInstance()
	{
		static CAction ins;
		return &ins;
	}

public:
	void reset()
	{
		//≥ı ºªØ
		m_action=ACTION_TBD;
	}
	void SetAction(const string& action)
	{
		m_action=action;
	}
	void GetAction(string& action)
	{
		action=m_action;
	}

private:
	string m_action;
};
