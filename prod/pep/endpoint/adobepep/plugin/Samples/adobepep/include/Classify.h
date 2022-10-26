#pragma once
#include <string>
#include "utilities.h"

class CClassify
{
public:
	static CClassify* GetInstance()
	{
		static CClassify ins;
		return &ins;
	}

	void ShowDocPropDlg(bool bShow);
	bool IsDocPropDlgShowed();
private:
	CClassify(void);
	~CClassify(void);

	bool m_bShowed;
};
