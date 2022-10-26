#pragma once
#include "smart_ptr.h"

enum MapMgrCacheEvalState
{
	MMCE_DEFAULT,
	MMCE_ALLOW,
	MMCE_DENY
};

typedef struct _tagFileInfo
{
	std::list<wstring>  skCheckList ;
	HANDLE hFile ;
	string strFile ;
	DWORD dStartTime ;
}HANDLE_NAME_TIME,*PHANDLE_NAME_TIME ;


class CMapperMgr
{
public:
	static const unsigned long MAX_TIME_OUT	= 300000 ;
	static const unsigned long MAX_TIME_OUT_EXPLORER = 5000 ;
	static const unsigned long MAX_TIME_OUT_IE6	= 5000 ;
	static CMapperMgr& Instance();

	void AddHandleName(HANDLE hFile, const string& sName);
	string GetHandleName(HANDLE hFile);
	void RemoveHandleName(HANDLE hFile);
	void RemoveItemByFileName(const string &sName) ;
	void CheckExpiredItem();

	/*	if return true:
	*	skip all the files which has already got an evaluation result with the current sk.
	*	put the left into \param list. user check all the file in the list on network_access
	*
	*	if return false:
	*	user can directly deny the network_access on this \param sk
	*/
	BOOL GetAllOpenFiles(wstring& sk, std::list<YLIB::smart_ptr<HANDLE_NAME_TIME>>& list);


private:
	CMapperMgr(void);
	CMapperMgr(const CMapperMgr&);
	void operator = (const CMapperMgr&);
	~CMapperMgr(void);

	BOOL CheckHasEvaluation( wstring& sk ,std::list<wstring>& skCheckList) const ;

private:
	std::list<YLIB::smart_ptr<HANDLE_NAME_TIME>>  m_listHandleName;    
};
