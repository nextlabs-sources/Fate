/*
 * Created on Apr 22, 2010
 *
 * All sources, binaries and HTML pages (C) copyright 2010 by NextLabs Inc.,
 * San Mateo CA, Ownership remains with NextLabs Inc, All rights reserved
 * worldwide.
 */

#ifndef __CE_PDPSEC_H
#define __CE_PDPSEC_H

#include <vector>
#include "CEsdk.h"

CEResult_t PDP_CESEC_MakeTrusted(JavaVM *PDP_jvm,
                                 jobject cmStub,
                                 jclass cmStubClass,
                                 std::vector<void *>& inputArgs,
                                 std::vector<void *>& outputArgs);
#endif
