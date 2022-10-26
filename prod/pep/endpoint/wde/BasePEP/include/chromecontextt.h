#ifndef __WDE_CHROMECONTEXT_H__
#define  __WDE_CHROMECONTEXT_H__

#include "genericcontext.h"

/***********************************************************************
// Special for chrome
***********************************************************************/
namespace nextlabs
{

class CChromeContext : public CGenericContext
{

private:
	virtual void OnResponseInitHookTable(bool(&table)[kHM_MaxItems]);
};

}  // ns nextlabs

#endif
