#ifndef __WDE_BASEPEP_H__
#define __WDE_BASEPEP_H__

#ifdef _MSC_VER
#pragma once
#else
#error "commonutils.hpp only supports windows-compile"
#endif // _MSC_VER

#include <commonutils.hpp>
#include <runtimecontext.h>

namespace nextlabs
{

class CBasePep : public nextlabs::CDllModule
{
public:
	CBasePep(HMODULE h) : CDllModule(h), pContext_(NULL){

	}
private:
	virtual BOOL OnProcessAttach();
	virtual BOOL OnProcessDetach();
private:
	BOOL InitContext();
	void DeinitContext();
private:

	//************************************
	// Returns: 
	//			TRUE  : DO NOT INSTANIATE
	//			FALSE : ALLOW INSNTANIATE
	//************************************
	BOOL _FilterPolicyCheck();
private:
	CRuntimeContext			*pContext_;
	nextlabs::CEXEModule	exe_;	// holds all fields that related with EXE module
};

}  // ns nextlabs

#endif //__WDE_BASEPEP_H__



