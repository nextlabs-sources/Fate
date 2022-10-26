#pragma once


#include <string>

#include "cesdk_loader.hpp"
using nextlabs::cesdk_loader;

#include "PCStatusMgr.h"

struct _CEHandle;
class cesdk_loader;
class CPrivateLoader;


class CPCStatusMgrImp : public CPCStatusMgr
{
public:
	CPCStatusMgrImp(void);
	virtual ~CPCStatusMgrImp(void);


	/*
	
	implementation:

	we check if pc is running before stop pc, if pc is already stopped, 
	we return directly. 

	otherwise, we stop pc and if we stop succeed, we disconnect pc, otherwise, we keep pc
	connected, pc connected equal to pc is still running.
	
	*/
	virtual _CEResult_t StopPC(wchar_t* pszPwd);

	/*
	
	implementation:

	we check if pc is running before start pc, if pc is already running, 
	we return directly. 

	otherwise, we start pc.
	
	*/
	virtual BOOL StartPC();

	/*
	
	implementation:

	using eframework api.

	
	*/
	virtual BOOL IsPCRunning();


	virtual void ResetUAC();
private:
	//	load sdk libraries and establish a connection with PC
	BOOL ConnectPC();

	//	disconnect to pc, and unload sdk libraries
	BOOL DisconnectPC();

	/*

	start pc service

	return value:
	
	false --	means error happened, 
	true	--	means service have started successfully.
	
	*/
	BOOL StartPCService();

private:

	cesdk_loader* m_psdkLoader;
	CPrivateLoader* m_pPrivateLoader;
	_CEHandle* m_ceHandle;
};
