#ifndef __WDE_CBCONTEXTCONTEXT_H__
#define  __WDE_CBCONTEXTCONTEXT_H__

#include "baseeventprovidercontext.h"
#include "shellexplorercontext.h"

#include <map>
#include <vector>

/***********************************************************************
// Special for Common Browser
***********************************************************************/
namespace nextlabs
{

class CCBContext : public CShellExploerContext
{

public:
    CCBContext();
private:
    virtual void OnResponseInitHookTable(bool(&table)[kHM_MaxItems]) ;

	virtual BOOL MySetFileAttributesW(LPCWSTR lpFileName, DWORD dwFileAttributes);

private:
	virtual EventResult OnBeforeSetFileAttributesW(LPCWSTR lpFileName, DWORD dwFileAttributes);

private:
	virtual EventResult EventBeforeSetAttributeAction(const std::wstring& deviceName);

private:

};

}  // ns nextlabs

#endif
