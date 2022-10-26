#ifndef __WDE_IECONTEXT_H__
#define  __WDE_IECONTEXT_H__

#include "baseeventprovidercontext.h"
#include "shellexplorercontext.h"
#include "eventhander.h"

namespace nextlabs
{  
    class CIEContext : public CShellExploerContext
    {
    private:
        virtual void OnResponseInitHookTable(bool(&table)[kHM_MaxItems]);
        virtual	EventResult EventAfterGetOpenFileName(LPOPENFILENAMEW lpofn);
    };
}  // namespace nextlabs


#endif