#pragma once
#include <string>
#include "utilities.h"
class CSend
{
public:
	static CSend* GetInstance()
	{
		static CSend ins;
		return &ins;
	}
	void ShowSendAndCollaborateLive(bool bshow);
	bool IsSendCollaborateLiveShowed();
private:
	bool m_bShowed;
private:
	CSend(void);
	~CSend(void);
};
