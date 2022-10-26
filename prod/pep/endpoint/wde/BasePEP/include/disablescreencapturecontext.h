#ifndef __WDE_SCREENCAPTURECONTEXT_H__
#define  __WDE_SCREENCAPTURECONTEXT_H__

#include "genericcontext.h"

namespace nextlabs
{  
    class CDisableScreenCaptureContext : public CBaseEventProviderContext
    {
    private:
        virtual void OnResponseInitHookTable(bool(&table)[kHM_MaxItems]);
    };
}  // namespace nextlabs


#endif