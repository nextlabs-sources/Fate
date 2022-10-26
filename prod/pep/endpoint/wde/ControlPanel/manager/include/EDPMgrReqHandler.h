#pragma once

#include <string>
#include <map>

#pragma warning(push)
#pragma warning(disable: 4100)
#include "brain.h"
#pragma warning(pop)

#include "iipcrequesthandler.h"
#include "dsipc.h"

class CEDPMgrReqHandler: public IIPCRequestHandler
{
public:
	CEDPMgrReqHandler(void);
	~CEDPMgrReqHandler(void);

	
	

	virtual bool Invoke(IPCREQUEST& request, IPCREQUEST* pResponse);

	/**
	* @return name of request handler to use for generating 
	* unique names for OS objects
	*/
	virtual TCHAR* GetName ();

private:

	BOOL GetUserSID(TCHAR **pszUserName);

	/*
	
	
	check if we need to show the last notification through notification window to end user.




	return value:

	true	--		yes, need to show
	false	--		no, we don't need to show
	
	
	*/
	BOOL NeedShowNotify();

	/*
	
	we rely on last notify' action and time tick to determine if we need to show current notify
	
	*/
	typedef struct _Notifys_
	{
		DWORD dwTick;
		wstring strAction;

		_Notifys_()
		{
			dwTick = 0;
		}
	}Notifys;
	Notifys m_sLastNotify;
	//	500 ms, we don't show the same action less than * ms in notification window.
	const static DWORD c_dwDiffMax = 1500;

	TCHAR m_szName [256];
};
